/* -----------------------------------------------------------------------
 *  (c) Perdio Project, DFKI & SICS
 *  Universit"at des Saarlandes
 *    Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
 *  SICS
 *    Box 1263, S-16428 Sweden, Phone (+46) 8 7521500
 *  Author: brand,scheidhr, mehl
 *  Last modified: $Date$ from $Author$
 *  Version: $Revision$
 *  State: $State$
 *
 *  protocol and message layer
 * -----------------------------------------------------------------------*/

/* -----------------------------------------------------------------------
 * TODO
 *
 *   variable protocol:
 *     failure/exception handling
 *     more than one bind request
 *     testing
 *   cell protocol
 *     all
 *   object protocol
 *     all
 *   chunks
 *     all
 *   builtin
 *     classify secure/insecure
 *   names
 *     true, false, unit and others (o-o)
 *     don't work at all?
 *   ip
 *     cache testing
 *     fairness for IO
 *     errors
 *     flow control
 *   port
 *     close: must fail client?
 *   gen hashtable: if hash value is negative: crash!
 * -----------------------------------------------------------------------*/

#ifdef PERDIO

#ifdef INTERFACE
#pragma implementation "perdio.hh"
#endif

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "oz.h"
#include "am.hh"
#include "ip.hh"
#include "codearea.hh"
#include "indexing.hh"

#include "perdio_debug.hh"
#include "perdio_debug.cc"

#include "genvar.hh"
#include "perdiovar.hh"

typedef long Credit;  /* TODO: full credit,long credit? */

class BorrowTable;
class OwnerTable;
class ByteStream;
class DebtRec;
DebtRec* debtRec;

void marshallTerm(Site* sd,OZ_Term t, ByteStream *bs, DebtRec *dr);
int unmarshallWithDest(BYTE *buf, int len, OZ_Term *t);
void domarshallTerm(Site* sd,OZ_Term t, ByteStream *bs);
void unmarshallTerm(ByteStream*,OZ_Term*);
OZ_Term unmarshallTerm(ByteStream *bs);
int sendSurrender(BorrowEntry *be,OZ_Term val);
int sendRedirect(Site* sd,int OTI,TaggedRef val);
int sendAcknowledge(Site* sd,int OTI);
int sendRedirect(ProxyList *pl,OZ_Term val, Site* ackSite,int OTI);
int bindPerdioVar(PerdioVar *pv,TaggedRef *lPtr,TaggedRef v);
int sendCreditBack(Site* sd,int OTI,Credit c);
inline void marshallNumber(unsigned int,ByteStream *);
inline void marshallMySite(ByteStream* );
inline void marshallCredit(Credit,ByteStream *);
inline void reliableSendFail(Site*,ByteStream *,Bool,int);

BorrowTable *borrowTable;
OwnerTable *ownerTable;

#define OT ownerTable
#define BT borrowTable

/*
 * Message formats
 */
enum MessageType {
  M_SITESEND,           // DIF
  M_PORTSEND,           // OTI DIF (implicit 1 credit)
  M_ASK_FOR_CREDIT,     // OTI SITE (implicit 1 credit)
  M_OWNER_CREDIT,       // OTI CREDIT
  M_BORROW_CREDIT,      // NA  CREDIT
  M_GET_CLOSUREANDCODE, // OTI SITE (implicit 1 credit)
  M_GET_CLOSURE,        // same as above
  M_SEND_CLOSUREANDCODE,// NA  N DIFs CODE
  M_SEND_CLOSURE,       // same as above
  M_REGISTER,           // OTI SITE (implicit 1 credit)
  M_REDIRECT,           // NA  DIF
  M_ACKNOWLEDGE,        // NA (implicit 1 credit)
  M_SURRENDER,          // OTI SITE DIF (implicit 1 credit)
  M_PORTCLOSE,          // OTI (implicit 1 credit)
};

/*
 *    NA      :=   SITE OTI
 *    OTI     :=   index
 *    SITE    :=   host port timestamp
 */


/*
 * the DIFs
 */
typedef enum {
  M_SMALLINT,           // int
  M_BIGINT,             // string
  M_FLOAT,              // string
  M_ATOM,               // string
  M_NAME,               // ???
  M_NAMETRUE,           // -
  M_NAMEFALSE,          // -
  M_RECORD,             //
  M_TUPLE,
  M_LIST,
  M_REF,
  M_OWNER,
  M_PORT,               // NA CREDIT
  M_PROC,               // NA CREDIT NAME ARITY
  M_VAR,
  M_BUILTIN
} MarshallTag;


/**********************************************************************/
/**********************************************************************/
/*                        INITIAL                                     */
/**********************************************************************/
/**********************************************************************/

#define PENDLINK_CUTOFF 100
#define PENDENTRY_CUTOFF 100

enum PO_TYPE {
  PO_Ref=0,
  PO_Tert,
  PO_Free
};

class ProtocolObject {
  int type;
  union {
    TaggedRef ref;
    Tertiary *tert;
  } u;
public:
  ProtocolObject()                {}
  Bool isTertiary()               { return type==PO_Tert; }
  Bool isRef()                    { return type==PO_Ref; }
  Bool isFree()                   { return type==PO_Free; }
  void setFree()                  { type = PO_Free; }
  void unsetFree()                { DebugCode(type=PO_Tert); }
  void mkTertiary(Tertiary *t)    { type = PO_Tert; u.tert=t; }
  void mkRef(TaggedRef v)         { type=PO_Ref; u.ref=v; }
  Tertiary *getTertiary()         { Assert(isTertiary()); return u.tert; }
  TaggedRef getRef()             { Assert(isRef()); return u.ref; }
  void gcPO() {
    if (isTertiary()) {
      u.tert=(Tertiary *)(u.tert->gcConstTerm());
    } else {
      Assert(isRef());
      gcTagged(u.ref,u.ref);
    }
  }

  ProtocolObject &operator =(ProtocolObject &n);

  void setIndex(int i) {
    if (isTertiary()) {
      getTertiary()->setIndex(i);
    } else {
      TaggedRef tPtr = getRef();
      TaggedRef val = *tagged2Ref(tPtr);
      PerdioVar *pv = tagged2PerdioVar(val);
      pv->setIndex(i);
    }
  }
  TaggedRef getValue() {
    if (isTertiary()) {
      return makeTaggedConst(getTertiary());
    } else {
      return getRef();
    }
  }
};

typedef Tertiary Proxy;

class ByteStream;

PERDIO_DEBUG_DO(void printTables());

/**********************************************************************/
/*                        NetAddress (not allocate only cast)         */
/**********************************************************************/

class NetAddress {
public:
  /*  DummyClassConstruction(NetAddress)*/

  Site* site;
  int index;

  NetAddress(Site* s, int i) : site(s), index(i) {}

  void set(Site *s,int i) {site=s,index=i;}

  Bool same(NetAddress *na) { return na->site==site && na->index==index; }

  Bool isLocal() { return site==mySite; }
};

/* ********************************************************************** */
/*                  BYTE STREAM
/* ********************************************************************** */

/* ByteBufferManger */

inline ByteBuffer* ByteBufferManager::newByteBuffer(){
  FreeListEntry *f=getOne();
  ByteBuffer *bb;
  if(f==NULL) {bb=new ByteBuffer();}
  else{GenCast(f,FreeListEntry*,bb,ByteBuffer*);}
  return bb;}

inline  void ByteBufferManager::deleteByteBuffer(ByteBuffer* bb){
  FreeListEntry *f;
  GenCast(bb,ByteBuffer*,f,FreeListEntry*);
  if(putOne(f)) return;
  delete bb;
  return;}


/* ByteStreamManager */

inline ByteStream* ByteStreamManager::newByteStream(){
  FreeListEntry *f=getOne();
  ByteStream *bs;
  if(f==NULL) { return new ByteStream();}
  GenCast(f,FreeListEntry*,bs,ByteStream*);
  return bs;}

inline  void ByteStreamManager::deleteByteStream(ByteStream* bs){
  FreeListEntry *f;
  GenCast(bs,ByteStream*,f,FreeListEntry*);
  if(putOne(f)) return;
  delete bs;
  return;}

/* ByteStream */

void ByteStream::removeFirst(){
  Assert(first!=last);
  ByteBuffer *bb=first;
  first=bb->next;
  bufferManager->freeByteBuffer(bb);}

void ByteStream::removeSingle(){
  Assert(first==last);
  bufferManager->freeByteBuffer(first);
  first=last=NULL;}

ByteBuffer *ByteStream::getAnother(){
  return(bufferManager->getByteBuffer());}

void ByteStream::marshalBegin(){
  PERDIO_DEBUG(MARSHALL_BE,"MARSHAL_BE marshal begin");
  Assert(first==NULL);
  Assert(last==NULL);
  first=getAnother();
  last=first;
  totlen= 0;
  pos=first->head()+tcpHeaderSize;}

/* BufferManager */

ByteStream* BufferManager::getByteStream(){
  ByteStream *bs=byteStreamM->newByteStream();
  bs->init();
  return bs;}

void BufferManager::freeByteStream(ByteStream *bs){
  if(bs->first!=NULL){
    Assert(bs->first==bs->last);
    byteBufM->deleteByteBuffer(bs->last);}
  byteStreamM->deleteByteStream(bs);}

ByteBuffer* BufferManager::getByteBuffer(){
  ByteBuffer *bb=byteBufM->newByteBuffer();
  bb->init();
  return bb;}

void BufferManager::freeByteBuffer(ByteBuffer* bb){
  byteBufM->deleteByteBuffer(bb);}

BufferManager *bufferManager;

/* ********************************************************************** */
/*                  PENDING MESSAGES STUFF
/* ********************************************************************** */


class PendEntry {
  int refCount;
  ByteStream *bs;
  BorrowEntry *back;
  Site * site;
public:
  void send();

  void initialize(ByteStream *bs1,Site * sd,BorrowEntry *b=NULL){
    bs=bs1;
    refCount=0;
    back=NULL;
    site=sd;
    if (b)
      back=b;}

  void inc() {refCount++;}
  void dec() {refCount--;}
  BorrowEntry *getBack() {return back;}
  int getrefCount() {return refCount;}
  Bool isFIFO() {return back!=NULL;}
};

void PendEntry::send(){
  reliableSendFail(site,bs,FALSE,1);
  return;
}

class PendEntryManager: public FreeListManager{
public:
  PendEntryManager():FreeListManager(PENDENTRY_CUTOFF){}

  PendEntry *newPendEntry(ByteStream *bs1,Site * sd,BorrowEntry *b=NULL) {
    FreeListEntry *f=getOne();
    if(f==NULL) {return new PendEntry();}
    PendEntry *pe;
    GenCast(f,FreeListEntry*,pe,PendEntry*);
    pe->initialize(bs1,sd,b);
    return pe;}

  void deletePendEntry(PendEntry* p){
    FreeListEntry *f;
    GenCast(p,PendEntry*,f,FreeListEntry*);
    if(putOne(f)) {return;}
    delete p;
    return;}
};

PendEntryManager *pendEntryManager;

/* ******************************************************************** */

class PendLink {
  Credit debt;
  PendEntry *pend;
public:
  PendLink *next;

  void initialize(Credit c,PendEntry* p){
    debt=c;
    pend=p;
    next=NULL;  }

  void setTag(){
    pend= (PendEntry *)(((unsigned int) pend) | 1);}

  Bool isTagged(){
    return ((unsigned int) pend & 1);}

  PendEntry *getPend(){
    return (PendEntry *)((unsigned int) pend & ~1);}

  Credit getDebt() {return debt;}
  void setDebt(Credit c) {debt=c;}
};

class PendLinkManager: public FreeListManager{
public:
  PendLinkManager():FreeListManager(PENDLINK_CUTOFF){}

  PendLink *newPendLink(){
    FreeListEntry *f=getOne();
    if(f==NULL) {return new PendLink();}
    PendLink *pl;
    GenCast(f,FreeListEntry*,pl,PendLink*);
    return pl;}

  void deletePendLink(PendLink* p){
    FreeListEntry *f;
    GenCast(p,PendLink*,f,FreeListEntry*);
    if(putOne(f)) {return;}
    delete p;
    return;}
};

PendLinkManager *pendLinkManager;

/* ********************************************************************** */
/* ********************************************************************** */
/*                  OWNER TABLE STUFF                                     */
/* ********************************************************************** */
/* ********************************************************************** */

/* credit           infinity=  1?----?
                    max=       01----1
                               .....
                    min=       0----01
                    dead=      0-----0   Invariant: credit never 0 for live owner
*/



#define INFINITE_CREDIT          -1

/*
#define START_CREDIT_SIZE        ((1<<31) - 1)
#define OWNER_GIVE_CREDIT_SIZE   ((1<<15))
#define BORROW_GIVE_CREDIT_SIZE  ((1<<7))
#define MIN_BORROW_CREDIT_SIZE   2
#define MAX_BORROW_CREDIT_SIZE   8 * OWNER_GIVE_CREDIT_SIZE
#define ASK_CREDIT_SIZE          OWNER_GIVE_CREDIT_SIZE

*/

#define START_CREDIT_SIZE        (256)
#define OWNER_GIVE_CREDIT_SIZE   (16)
#define BORROW_GIVE_CREDIT_SIZE  (4)
#define MIN_BORROW_CREDIT_SIZE   2
#define MAX_BORROW_CREDIT_SIZE   2 * OWNER_GIVE_CREDIT_SIZE
#define ASK_CREDIT_SIZE          OWNER_GIVE_CREDIT_SIZE


#define DEFAULT_OWNER_TABLE_SIZE   100
#define DEFAULT_BORROW_TABLE_SIZE  100
#define NET_HASH_TABLE_DEFAULT_SIZE 100
#define GNAME_HASH_TABLE_DEFAULT_SIZE 100

static double TABLE_LOW_LIMIT=0.20;
static double TABLE_EXPAND_FACTOR=2.00;

#define TABLE_BUFFER 50
#define TABLE_WORTHWHILE_REALLOC 200

/* ********************************************************************** */
/*                  HASH TABLE def
/* ********************************************************************** */

class NetHashTable: public GenHashTable{
  int hashFunc(NetAddress *);
  Bool findPlace(int ,NetAddress *, GenHashNode *&);
public:
  NetHashTable():GenHashTable(NET_HASH_TABLE_DEFAULT_SIZE){}
  BorrowEntry* findNA(NetAddress *);
  void add(NetAddress *,int);
  void sub(NetAddress *);
#ifdef DEBUG_PERDIO
  void print();
#endif
};


/* ********************************************************************** */
/*                  GNAME TABLE
/* ********************************************************************** */


const int fatIntDigits = 2;
const unsigned int maxDigit = 0xffffffff;

class FatInt {
public:
  unsigned int number[fatIntDigits];

  FatInt() { for(int i=0; i<fatIntDigits; i++) number[i]=0; }
  void inc()
  {
    int i=0;
    while(number[i]==maxDigit) {
      number[i]=0;
      i++;
    }
    Assert(i<fatIntDigits);
    number[i]++;
  }

  Bool same(FatInt &other)
  {
    for (int i=0; i<fatIntDigits; i++) {
      if(number[i]!=other.number[i])
        return NO;
    }
    return OK;
  }
};

static FatInt *idCounter = NULL;

class GNameSite {
public:
  ip_address ip;
  port_t port;
  time_t timestamp;
  Bool same(GNameSite &other)
  {
    return (port==other.port &&
            timestamp==other.timestamp &&
            ip==other.ip);
  }
};



#define GNAME_GC_MARK   1
#define GNAME_PRED_MARK 2

class GName {
public:
  int32 flags;
  GNameSite site;
  FatInt id;

  Bool same(GName *other) { return site.same(other->site) && id.same(other->id); }
  GName() { flags = 0; }
  GName(ip_address ip, port_t port, time_t timestamp)
  {
    flags = 0;

    site.ip        = ip;
    site.port      = port;
    site.timestamp = timestamp;

    idCounter->inc();
    id = *idCounter;
  }

  void setPredMark() { flags |= GNAME_PRED_MARK; }
  Bool getPredMark() { return (flags&GNAME_PRED_MARK); }

  void setGCMark()   { flags |= GNAME_GC_MARK; }
  Bool getGCMark()   { return (flags&GNAME_GC_MARK); }
  void resetGCMark() { flags &= ~GNAME_GC_MARK; }

};

class GNameTable: public GenHashTable{
  int hashFunc(GName *);
public:
  GNameTable():GenHashTable(GNAME_HASH_TABLE_DEFAULT_SIZE) {}
  void gnameAdd(GName *name, TaggedRef t);
  void gnameAdd(GName *name, PrTabEntry *pr);
  TaggedRef gnameFind(GName *name);

  void gcGNameTable();
};

static GNameTable *gnameTable = NULL;

int GNameTable::hashFunc(GName *gname)
{
  int ret = gname->site.ip + gname->site.port + gname->site.timestamp;
  for(int i=0; i<fatIntDigits; i++) {
    ret += gname->id.number[i];
  }
  return ret<0?-ret:ret;
}


inline
void GNameTable::gnameAdd(GName *name, TaggedRef t)
{
  int hvalue=hashFunc(name);
  GenHashTable::htAdd(hvalue,(GenHashBaseKey*)name, (GenHashEntry *) ToPointer(t));
}


inline
void GNameTable::gnameAdd(GName *name, PrTabEntry *pr)
{
  gnameAdd(name,ToInt32(pr));
  name->setPredMark();
}

void addGName(GName *name, PrTabEntry *pr)
{
  gnameTable->gnameAdd(name,pr);
}

TaggedRef GNameTable::gnameFind(GName *name)
{
  int hvalue = hashFunc(name);
  GenHashNode *aux = htFindFirst(hvalue);
  while(aux) {
    if (name->same((GName*)aux->getBaseKey())) {
      return (TaggedRef) ToInt32(aux->getEntry());
    }
    aux = htFindNext(aux,hvalue); }
  return makeTaggedNULL();
}



GName *newGName(TaggedRef t)
{
  ip_address ip;
  port_t port;
  time_t ts;

  getSiteFields(mySite,ip,port,ts);
  GName* ret = new GName(ip,port,ts);
  gnameTable->gnameAdd(ret,t);
  return ret;
}

GName *newGName(PrTabEntry *pr)
{
  GName *ret = newGName(ToInt32(pr));
  ret->setPredMark();
  return ret;
}


GName *copyGName(GName *gn)
{
  return new GName(*gn);
}

PrTabEntry *findCodeGName(GName *gn)
{
  TaggedRef aux = gnameTable->gnameFind(gn);
  if (aux) {
    return (PrTabEntry*) ToPointer(aux);
  }
  return NULL;
}


/* ********************************************************************** */
/*    OB_Entry - common to borrow and owner tables                        */
/* ********************************************************************** */

class OB_Entry : public ProtocolObject {
protected:
  union {
    Credit credit;
    int nextfree;
  } u;

  void makeFree(int next) {setFree(); u.nextfree=next;}

  int getNextFree(){
    Assert(isFree());
    return u.nextfree;  }

  void setCredit(Credit c) {u.credit=c;}

  void addToCredit(Credit c) {u.credit +=c;}

  void subFromCredit(Credit c) {u.credit -=c;}



public:
  Credit getCredit(){Assert(!isFree());return u.credit;}
};

/* ********************************************************************** */

class OwnerEntry: public OB_Entry {
friend class OwnerTable;

private:
  Credit requestCredit(Credit req){
    Credit c=getCredit();

    if(c == INFINITE_CREDIT) return(req);
    if(c < 2*req) {
      setCredit(INFINITE_CREDIT);
      return req;
    }
    subFromCredit(req);
    return req;
  }
public:
  void returnCredit(Credit c) {
    if (getCredit() == INFINITE_CREDIT) return;
    addToCredit(c);
  }
  int hasFullCredit()     { return getCredit()==START_CREDIT_SIZE; }
  Credit getSendCredit()  { return requestCredit(OWNER_GIVE_CREDIT_SIZE); }
  void getOneCredit()     { (void) requestCredit(1); }   // for redirect
  Credit giveMoreCredit() { return requestCredit(ASK_CREDIT_SIZE); }
};


/* ********************************************************** */
/*                    OWNER TABLE                             */
/* ********************************************************** */

#define END_FREE -1

class OwnerTable {
  int size;
  int no_used;
  int nextfree;

  void init(int,int);
  void compactify();

public:
  OwnerEntry* array;  /* TODO move to private */

#ifdef DEBUG_PERDIO
  void print();

#endif

  OwnerEntry *getOwner(int i)  {return &array[i];}

  int getSize() {return size;}

  OwnerTable(int sz) {
    size = sz;
    array = (OwnerEntry*) malloc(size*sizeof(OwnerEntry));
    Assert(array!=NULL);
    nextfree = END_FREE;
    no_used=0;
    init(0,sz);
  }

  void gcOwnerTable();

  void resize();

  int newOwner(OwnerEntry *&);

  void freeOwnerEntry(int);

  void newOZPort(Tertiary *);

  void returnCreditAndCheck(int,Credit);
};

void OwnerTable::returnCreditAndCheck(int OTI,Credit c)
{
  OwnerEntry *oe = getOwner(OTI);
  oe->returnCredit(c);
  if (oe->hasFullCredit()) {
    if (oe->isTertiary()) {
      Tertiary *te=oe->getTertiary();
      Assert(te->getTertType()==Te_Manager);
      te->localize();
    } else {
      // localize a variable
      TaggedRef tPtr=oe->getRef();
      TaggedRef val = deref(tPtr);
      if (isPerdioVar(val)) {
        PerdioVar *pvar = tagged2PerdioVar(val);
        SVariable *svar = new SVariable(am.rootBoard);
        svar->setSuspList(pvar->getSuspList());
        doBindSVar(tagged2Ref(tPtr),svar);
      }
    }
    freeOwnerEntry(OTI);
  }
}

void OwnerTable::init(int beg,int end)
{
  int i=beg;
  while(i<end){
    array[i].makeFree(i+1);
    i++;}
  i--;
  array[i].makeFree(END_FREE);
  nextfree=beg;
}

void OwnerTable::compactify()  /* TODO - not tested */
{
  Assert(size>=DEFAULT_OWNER_TABLE_SIZE);
  if(size==DEFAULT_OWNER_TABLE_SIZE) return;
  if(no_used/size< TABLE_LOW_LIMIT) return;
  PERDIO_DEBUG2(TABLE,"TABLE:owner compactify enter: size:%d no_used:%d",
               size,no_used);
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  int i=0;
  int used_slot= -1;
  int* base = &nextfree;
  while(TRUE){
    if(i>=size) break;
    if(array[i].isFree()){
      *base=i;
      base=&array[i].u.nextfree;}
    else used_slot=i;
    i++;}
  *base=END_FREE;
  int first_free=used_slot+1;
  int newsize= first_free-no_used < TABLE_BUFFER ?
    first_free-no_used+TABLE_BUFFER : used_slot+1;
  if(first_free < size - TABLE_WORTHWHILE_REALLOC){
    PERDIO_DEBUG1(TABLE,"TABLE:owner compactify free slots: new%d",newsize);
    OwnerEntry *oldarray=array;
    array = (OwnerEntry*) realloc(array,newsize*sizeof(OwnerEntry));
    Assert(array!=NULL);
    if(array!=NULL){
      size=newsize;
      init(first_free,size);
      return;}
    array=oldarray;}
  init(first_free,size);
  PERDIO_DEBUG(TABLE,"TABLE:owner compactify no realloc");
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

void OwnerTable::resize(){
  int newsize = ((int) (TABLE_EXPAND_FACTOR *size));
  PERDIO_DEBUG3(TABLE,"TABLE:resize owner old:%d no_used:%d new:%d",
                size,no_used,newsize);
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  array = (OwnerEntry*) realloc(array,newsize*sizeof(OwnerEntry));
  Assert(array!=NULL);
  if(array==NULL){
    error("Memory allocation: Owner Table growth not possible");}
  init(size, newsize);
  size=newsize;
  PERDIO_DEBUG(TABLE2,"TABLE:resize owner complete");
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

int OwnerTable::newOwner(OwnerEntry *&oe){
  if(nextfree == END_FREE) resize();
  int index = nextfree;
  nextfree = array[index].u.nextfree;
  oe = (OwnerEntry *)&(array[index]);
  oe->setCredit(START_CREDIT_SIZE);
  PERDIO_DEBUG1(TABLE,"TABLE:owner insert: %d",index);
  no_used++;
  return index;}

void OwnerTable::newOZPort(Tertiary* tert){
  Assert(nextfree==0);
  nextfree = array[0].u.nextfree;
  OwnerEntry* oe= (OwnerEntry *)&(array[0]);
  oe->mkTertiary(tert);
  tert->setIndex(0);
  oe->setCredit(INFINITE_CREDIT);}


void OwnerTable::freeOwnerEntry(int i){
  array[i].setFree();
  array[i].u.nextfree=nextfree;
  nextfree=i;
  no_used--;
  PERDIO_DEBUG1(TABLE,"TABLE:owner delete %d",i);
  return;}

#ifdef DEBUG_PERDIO
void OwnerTable::print(){
  printf("***********************************************\n");
  printf("********* OWNER TABLE *************************\n");
  printf("***********************************************\n");
  printf("Size:%d No_used:%d \n",size,no_used);
  int i;
  OwnerEntry *oe;
  for(i=0;i<size;i++){
    if(!(array[i].isFree())){
      oe=getOwner(i);
      printf("<%d> OWNER: Credit:%d\n",i,oe->getCredit());}
    else{
      printf("<%d> FREE: next:%d\n",i,array[i].u.nextfree);}}
  printf("-----------------------------------------------\n");
}
#endif

/* ********************************************************************** */
/* ********************************************************************** */
/*                  BORROW TABLE STUFF                                    */
/* ********************************************************************** */
/* ********************************************************************** */

#define BORROW_GC_MARK 1

int borrowEntryToIndex(BorrowEntry *b);

class BorrowEntry: public OB_Entry {
friend class BorrowTable;
private:
  NetAddress netaddr;
  PendLink *pendLink;
  void inDebt(PendLink *);
  Credit pendLinkCredit(Credit c);
  void pendLinkHandle();

public:

  void makeMark(){
    pendLink = (PendLink*)((unsigned int) pendLink | BORROW_GC_MARK);}

  Bool isMarked(){
    return ((unsigned int) pendLink & BORROW_GC_MARK);}

  void removeMark(){
    pendLink = (PendLink*)((unsigned int) pendLink & (~BORROW_GC_MARK));}

  Bool isPending(){
    Assert(!isFree());
    return(pendLink!=NULL);}

  Bool gcCheck() {
    Assert(!isFree());
    if(isMarked()){
      removeMark();
      PERDIO_DEBUG(GC,"GC:mark found");
      return FALSE;}
    PERDIO_DEBUG(GC,"GC:no mark found");
    return TRUE;}           // maybe garbage (only if pendLink==NULL);

  inline void copyBorrow(BorrowEntry* from,int i){
    setCredit(from->getCredit());
    if (from->isTertiary()) {
      mkTertiary(from->getTertiary());
    } else {
      mkRef(from->getRef());
    }
    pendLink=from->pendLink;
    netaddr.set(from->netaddr.site,from->netaddr.index);
    from->setIndex(i);}

  void initBorrow(Credit c,Site* s,int i){
    Assert(isFree());
    setCredit(c);
    unsetFree();
    pendLink=NULL;
    netaddr.set(s,i);
    return;}

  NetAddress* getNetAddress() {
    Assert(!isFree());
    return &netaddr;}

  void addPendLink(PendLink*);

  void freeBorrowEntry();

  void addAskCredit(Credit c){
    addToCredit(1);
    addCredit(c-1);
  }

#ifdef DEBUG_PERDIO
  void DEBUG_pendLink(PendLink *pl){
    while(pl!=NULL) {pl=pl->next;}
    return;}
#endif

  void addCredit(Credit cin){
    PERDIO_DEBUG4(CREDIT,"CREDIT:borrow add <%d:%d> add:%d to:%d",
                  getNetAddress()->site,
                  getNetAddress()->index,
                  cin,getCredit());
    if(pendLink!=NULL){
      cin=pendLinkCredit(cin);
      pendLinkHandle();
      PERDIO_DEBUG_DO(DEBUG_pendLink(pendLink));}
    addToCredit(cin);
    Credit cur=getCredit();
    if(cur>MAX_BORROW_CREDIT_SIZE){
      giveBackCredit(cur-MAX_BORROW_CREDIT_SIZE);
      setCredit(MAX_BORROW_CREDIT_SIZE);}
  }

  Bool getOneAskCredit() {
    Credit c=getCredit();
    if(c==1) {
      PERDIO_DEBUG(CREDIT,"CREDIT:getOneAskCredit failed");
      return FALSE;
    }
    PERDIO_DEBUG(CREDIT,"CREDIT:getOneAskCredit OK");
    subFromCredit(1);
    return TRUE;}

  Bool getOneCredit() {
    Credit c=getCredit();
    Assert(c>0);
    if(c <= MIN_BORROW_CREDIT_SIZE) {
      PERDIO_DEBUG(CREDIT,"CREDIT:getOneCredit failed");
      return FALSE;}
    PERDIO_DEBUG(CREDIT,"CREDIT:getOneCredit OK");
    subFromCredit(1);
    return TRUE; }

  Bool getSmallCredit(Credit &c){
    Credit cur=getCredit();
    if(cur < 2 * MIN_BORROW_CREDIT_SIZE) return FALSE;
    if(cur >  2 * BORROW_GIVE_CREDIT_SIZE) c=BORROW_GIVE_CREDIT_SIZE;
    else{
      if(cur >= 2 * MIN_BORROW_CREDIT_SIZE) c=MIN_BORROW_CREDIT_SIZE;}
    PERDIO_DEBUG1(CREDIT,"CREDIT:give small credit:%d",c);
    subFromCredit(c);
    return TRUE;}

  void inDebtMain(PendEntry *);
  void inDebtSec(Credit,PendEntry *);
  void moreCredit();

  void giveBackCredit(Credit c);
  Bool fifoCanSend(PendLink *,PendEntry *pe,Bool flag);
};

void BorrowEntry::inDebtMain(PendEntry *pe){
  PendLink *pl=pendLinkManager->newPendLink();
  pl->initialize(1,pe);
  pl->setTag();
  pe->inc();
  inDebt(pl);}

void BorrowEntry::inDebtSec(Credit c,PendEntry *pe){
  PendLink *pl=pendLinkManager->newPendLink();
  pl->initialize(c,pe);
  pe->inc();
  inDebt(pl);}

void BorrowEntry::inDebt(PendLink *pl){
  if(pendLink==NULL) {
    PERDIO_DEBUG(PENDLINK,"PENDLINK:new- none so far");
    moreCredit();
    pendLink=pl;
    return;}
  PERDIO_DEBUG(PENDLINK,"PENDLINK:new- others around far");
  PendLink* aux=pendLink;
  while(aux->next!=NULL) aux=aux->next;
  aux->next=pl;
  return;
}

Credit BorrowEntry::pendLinkCredit(Credit c){
  PendLink *pl= pendLink;
  while(c>0){
    Credit d= pl->getDebt();
    if(d>c) {
      pl->setDebt(d-c);
      return 0;}
    PERDIO_DEBUG(PENDLINK,"PENDLINK one entry = 0");
    pl->setDebt(0);
    c = c-d;
    (pl->getPend())->dec();
    pl=pl->next;
    if(pl==NULL) return c;
  }
  return c;}

Bool BorrowEntry::fifoCanSend(PendLink *cur,PendEntry *pe,Bool flag){
  if(!pe->isFIFO()) return TRUE;
  BorrowEntry *bb=pe->getBack();
  if(bb==this){
    if((cur->isTagged())&& (!flag)) {
      PERDIO_DEBUG(PENDLINK,"PENDLINK:cannot send due to FIFO");
      return FALSE;}
    return TRUE;}
  PendLink *pl=pendLink;
  while(TRUE){
    Assert(pl!=NULL);
    if(pl->getPend()==pe) return TRUE;
    if(pl->isTagged()) return FALSE;
    pl=pl->next; }}

void BorrowEntry::pendLinkHandle(){
  PendLink *cur=pendLink;
  PendLink **base=&pendLink;
  PendLink *aux;
  PendEntry *pe;
  Bool flag=TRUE; /* can send */
  Bool msgsent;

  PERDIO_DEBUG(PENDLINK,"PENDLINK:entering debt handler");
  while(TRUE){
    if(cur==NULL) {
      *base=cur;
      return;}
    if(cur->getDebt()!=0){
      PERDIO_DEBUG(PENDLINK,"PENDLINK - ran into non-zero debt");
      *base=cur;
      moreCredit();
      return;}
    pe=cur->getPend();
    if((pe->getrefCount()==0)){
      if(fifoCanSend(cur,pe,flag)){
        PERDIO_DEBUG(DELAYED_MSG_SENT,"DELAYED_MSG_SENT:pendLinkHandle");
        pe->send();
        msgsent=TRUE;
        pendEntryManager->deletePendEntry(pe);}
      else{
        PERDIO_DEBUG(PENDLINK,"PENDLINK - ran into fifo cannot send");
        msgsent=FALSE;}}
    else{
      PERDIO_DEBUG(PENDLINK,"PENDLINK - ran into non-zero ref ct");
      msgsent=FALSE;}
    if(cur->isTagged() && ((!flag) || (!msgsent))){
      PERDIO_DEBUG(PENDLINK,"PENDLINK fifo restriction cannot remove");
      flag=FALSE;
      base= &(cur->next);
      cur=cur->next;}
    else{
      aux=cur->next;
      PERDIO_DEBUG(PENDLINK,"PENDLINK removal");
      pendLinkManager->deletePendLink(cur);
      cur=aux;}}
}


void BorrowEntry::moreCredit(){
  if(!getOneAskCredit()) {
    // already required moreCredit!
    return;
  }
  ByteStream *bs= bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_ASK_FOR_CREDIT);
  NetAddress *na = getNetAddress();
  Site* site = na->site;
  int index = na->index;
  marshallNumber(index,bs);
  marshallMySite(bs);
  PERDIO_DEBUG2(MSG_SENT,"MSG_SENT:ASK_FOR_CREDIT sd:%x,index:%d",
                site,index);
  bs->marshalEnd();
  reliableSendFail(site,bs,TRUE,2);
  return;}

int sendRegister(BorrowEntry *be) {
  ByteStream *bs= bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_REGISTER);
  NetAddress *na = be->getNetAddress();
  Site* site = na->site;
  int index = na->index;
  marshallNumber(index,bs);
  marshallMySite(bs);
  bs->marshalEnd();
  PERDIO_DEBUG2(MSG_SENT,"MSG_SENT:REGISTER sd:%d,index:%d",
                site,index);

  if (be->getOneCredit()) {  /* priority */
    reliableSendFail(site,bs,TRUE,3);
    return PROCEED;
  }

  PERDIO_DEBUG(DEBT_MAIN,"DEBT_MAIN:register");
  PendEntry *pe= pendEntryManager->newPendEntry(bs,site,be);
  be->inDebtMain(pe);
  return PROCEED;
}


void BorrowEntry::giveBackCredit(Credit c){
  NetAddress *na = getNetAddress();
  Site* site = na->site;
  int index = na->index;
  sendCreditBack(site,index,c);
}

void BorrowEntry::freeBorrowEntry(){
  giveBackCredit(u.credit);
  networkSiteDec((getNetAddress())->site);}

/* ********************************************************************** */
/*         BorrowTable                                                    */
/* ********************************************************************** */

class BorrowTable {
private:
  int no_used;
  BorrowEntry* array;
  int size;
  int nextfree;

  void init(int,int);
  void compactify();

public:
  NetHashTable *hshtbl;

  BorrowEntry *getBorrow(int i)  {return &array[i];}

  int ptr2Index(BorrowEntry *a) { return(a-array);}

  int getSize() {return size;}

  BorrowTable(int sz)  {
    size= sz;
    array = (BorrowEntry*) malloc(size *sizeof(BorrowEntry));
    Assert(array!=NULL);
    nextfree = END_FREE;
    init(0,sz);
    no_used=0;
    hshtbl = new NetHashTable();  }

  void gcBorrowTable();

  BorrowEntry *find(NetAddress *na)  {
    BorrowEntry *b=hshtbl->findNA(na);
    if(b==NULL) {
      PERDIO_DEBUG(LOOKUP,"LOOKUP:borrow NO");}
    else {
      PERDIO_DEBUG(LOOKUP,"LOOKUP:borrow yes");}
    return b;
  }

  void resize();

  int newBorrow(Credit,Site*,int);

  void maybeFreeBorrowEntry(int);

  Site* getOriginSite(int bi){
    return getBorrow(bi)->getNetAddress()->site;}

  int getOriginIndex(int bi){
    return getBorrow(bi)->getNetAddress()->index;}

  void copyBorrowTable(BorrowEntry *,int);

#ifdef DEBUG_PERDIO
  void print();
#endif
};

void BorrowTable::init(int beg,int end)
{
  int i=beg;
  while(i<end){
    array[i].u.nextfree=i+1;
    array[i].setFree();
    i++;}
  i--;
  array[i].u.nextfree=nextfree;
  nextfree=beg;
}

void BorrowTable::compactify(){
  if(no_used / size >= TABLE_LOW_LIMIT) return;
  Assert(size>=DEFAULT_BORROW_TABLE_SIZE);
  if(size==DEFAULT_BORROW_TABLE_SIZE) return;
  int newsize= no_used+TABLE_BUFFER;
  if(newsize<DEFAULT_BORROW_TABLE_SIZE) newsize=DEFAULT_BORROW_TABLE_SIZE;
  PERDIO_DEBUG3(TABLE,"TABLE:compactify borrow old:%d no_used:%d new:%d",
                size,no_used,newsize);
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  BorrowEntry *oldarray=array;
  array = (BorrowEntry*) malloc(newsize*sizeof(BorrowEntry));
  if(array==NULL){
    PERDIO_DEBUG(TABLE,"TABLE: compactify borrow NOT POSSIBLE");
    array=oldarray;
    return;}
  int oldsize=size;
  size=newsize;
  copyBorrowTable(oldarray,oldsize);
  PERDIO_DEBUG(TABLE,"TABLE:compactify borrow complete");
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

void BorrowTable::resize(){
  Assert(no_used==size);
  int newsize = int (TABLE_EXPAND_FACTOR*size);
  PERDIO_DEBUG3(TABLE,"TABLE:resize borrow old:%d no_used:%d new:%d",
                size,no_used,newsize);
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  BorrowEntry *oldarray=array;
  array = (BorrowEntry*) malloc(newsize*sizeof(BorrowEntry));
  Assert(array!=NULL);
  if(array==NULL){
    error("Memory allocation: Borrow Table growth not possible");}
  int oldsize=size;
  size=newsize;
  copyBorrowTable(oldarray,oldsize);
  PERDIO_DEBUG(TABLE,"TABLE:resize borrow complete");
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

int BorrowTable::newBorrow(Credit c,Site * sd,int off){
  if(nextfree == END_FREE) resize();
  int index=nextfree;
  nextfree= array[index].u.nextfree;
  BorrowEntry* oe = &(array[index]);
  oe->initBorrow(c,sd,off);
  PERDIO_DEBUG4(HASH2,"HASH2:<SPECIAL>:net=%x borrow=%x owner=%x hash=%x",
                oe->getNetAddress(),array,ownerTable->array,
                hshtbl->table);
  hshtbl->add(oe->getNetAddress(),index);
  no_used++;
  PERDIO_DEBUG1(TABLE,"TABLE:borrow insert: %d",index);
  return index;}

void BorrowTable::maybeFreeBorrowEntry(int index){
  BorrowEntry *b = &(array[index]);
  if(b->isPending()) return; /* cannot remove as msgs pending on credit */
  b->freeBorrowEntry();
  hshtbl->sub(getBorrow(index)->getNetAddress());
  array[index].setFree();
  array[index].u.nextfree=nextfree;
  nextfree=index;
  no_used--;
  PERDIO_DEBUG1(TABLE,"TABLE:borrow delete: %d",index);
  return;}

void BorrowTable::copyBorrowTable(BorrowEntry *oarray,int osize){
  hshtbl->clear();
  int oindex=0;
  int nindex= 0;
  BorrowEntry *ob,*nb;
  while(TRUE){
    nb=&(array[nindex]);
    ob= &(oarray[oindex]);
    while(ob->isFree()) {
      oindex++;
      if(oindex>=osize) goto fin;
      ob= &(oarray[oindex]);}
    nb->copyBorrow(ob,nindex);
    hshtbl->add(nb->getNetAddress(),nindex);
    nindex++;
    oindex++;
    if(oindex>=osize) goto fin;}
fin:
  nextfree=END_FREE;
  init(nindex,size);
  free(oarray);
}

#ifdef DEBUG_PERDIO
void BorrowTable::print(){
  printf("***********************************************\n");
  printf("********* BORROW TABLE *************************\n");
  printf("***********************************************\n");
  printf("Size:%d No_used:%d \n",size,no_used);
  int i;
  BorrowEntry *b;
  for(i=0;i<size;i++){
    if(!(array[i].isFree())){
      b=getBorrow(i);
      printf("<%d> BORROW: Credit:%d net=%d:%d pend%d\n",i,b->getCredit(),
             b->getNetAddress()->site,b->getNetAddress()->index,
             b->pendLink);}
    else{
      printf("<%d> FREE: next:%d\n",i,array[i].u.nextfree);}}
  printf("-----------------------------------------------\n");
}

#endif

int borrowEntryToIndex(BorrowEntry *b){return borrowTable->ptr2Index(b);}

#ifdef DEBUG_PERDIO

void printTables(){
  ownerTable->print();
  borrowTable->print();
  borrowTable->hshtbl->print();}

void resize_hash(){
  borrowTable->hshtbl->print();}

#endif

/* ********************************************************************** */
/*                  HASH TABLE methods
/* ********************************************************************** */

inline NetAddress * GenHashNode2NetAddr(GenHashNode *ghn){
  NetAddress *na;
  GenCast(ghn->getBaseKey(),GenHashBaseKey*,na,NetAddress*);
  return na;}

inline int GenHashNode2BorrowIndex(GenHashNode *ghn){
  int i;
  GenCast(ghn->getEntry(),GenHashEntry*,i,int);
  Assert(i>=0);
  Assert(i<borrowTable->getSize());
  return i;}

inline Bool NetHashTable::findPlace(int hvalue,NetAddress *na,GenHashNode *&ghn){
  PERDIO_DEBUG3(HASH,"HASH:find Place hvalue=%d, net%d:%d",hvalue,
               na->site,na->index);
  ghn=htFindFirst(hvalue);
  NetAddress *na2;
  while(ghn!=NULL){
    na2=GenHashNode2NetAddr(ghn);
    if(na->same(na2)){
      PERDIO_DEBUG4(HASH,"HASH:compare success hvalue=%d bk=%x net%d:%d",
                    ghn->getKey(),ghn->getBaseKey,na2->site,na2->index);
      return TRUE;}
    PERDIO_DEBUG4(HASH,"HASH:compare fail hvalue=%d bk=%x net%d:%d",
                  ghn->getKey(),ghn->getBaseKey,na2->site,na2->index);
    ghn=htFindNext(ghn,hvalue);}
  return FALSE;}

int NetHashTable::hashFunc(NetAddress *na){
  unsigned char *p = (unsigned char*) na;
  int i;
  unsigned h = 0, g;

  for(i=0;i<8; i++,p++) {
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;}}
  return (int) h;}

BorrowEntry *NetHashTable::findNA(NetAddress *na){
  GenHashNode *ghn;
  int bindex;
  int hvalue=hashFunc(na);
  if(findPlace(hvalue,na,ghn)){
    int bindex= GenHashNode2BorrowIndex(ghn);
    PERDIO_DEBUG1(HASH,"HASH:borrow index %d",bindex);
    return borrowTable->getBorrow(bindex);}
  return NULL;}

void NetHashTable::add(NetAddress *na,int bindex){
  int hvalue=hashFunc(na);
  GenHashNode *ghn;
  Assert(!findPlace(hvalue,na,ghn));
  PERDIO_DEBUG4(HASH,"HASH:adding hvalue=%d net=%d:%d bindex=%d",
               hvalue,na->site,na->index,bindex);
  GenHashBaseKey* ghn_bk;
  GenHashEntry* ghn_e;
  GenCast(na,NetAddress*,ghn_bk,GenHashBaseKey*);
  GenCast(bindex,int,ghn_e,GenHashEntry*);
  htAdd(hvalue,ghn_bk,ghn_e);}

void NetHashTable::sub(NetAddress *na){
  int hvalue=hashFunc(na);
  GenHashNode *ghn;
  findPlace(hvalue,na,ghn);
  PERDIO_DEBUG3(HASH,"HASH:deleting hvalue=%d net=%d:%d bindex=%d",
               hvalue,na->site,na->index);
  htSub(hvalue,ghn);}

#ifdef DEBUG_PERDIO
void NetHashTable::print(){
  int limit=getSize();
  int used=getUsed();
  int i;
  printf("******************************************************\n");
  printf("************* NetAddress Hash Table *****************\n");
  printf("******************************************************\n");
  printf("Size:%d Used:%d\n",limit,used);
  GenHashNode *ghn;
  NetAddress *na;
  for(i=0;i<limit;i++){
    ghn=getElem(i);
    if(ghn!=NULL){
      na=GenHashNode2NetAddr(ghn);
      printf("<%d> - net%d:%d\n",i,na->site,na->index);
      ghn=ghn->getNext();
      while(ghn!=NULL){
        na=GenHashNode2NetAddr(ghn);
        printf("<coll> - net%d:%d\n",na->site,na->index);
        ghn=ghn->getNext();}}}
  printf("-----------------------------------\n");
}
#endif

/**********************************************************************/
/**********************************************************************/
/*                      GARBAGE COLLECTION
/**********************************************************************/
/**********************************************************************/

/* OBS: ---------- interface to gc.cc ----------*/

void gcOwnerTable()  { ownerTable->gcOwnerTable();}
void gcBorrowTable() { borrowTable->gcBorrowTable();}
void gcGNameTable()  { gnameTable->gcGNameTable();}
void gcGName(GName* name) { if (name) name->setGCMark(); }

void Tertiary::gcTertiary()
{
  switch (getTertType()) {

  case Te_Local:
    {
      setBoard(getBoard()->gcBoard());
      break;
    }

  case Te_Proxy:
    {
      int i=getIndex();
      borrowTable->getBorrow(i)->makeMark();
      PERDIO_DEBUG3(GC,"GC-relocate borrow:%d old%x new %x",
                    i,borrowTable->getBorrow(i),this);
      borrowTable->getBorrow(i)->mkTertiary(this);
      break;
    }

  case Te_Manager:
    {
      int i=getIndex();
      PERDIO_DEBUG3(GC,"GC-relocate owner:%d old%x new %x",
                    i,ownerTable->getOwner(i),this);
      ownerTable->getOwner(i)->mkTertiary(this);
    }
  }
}

/*--------------------*/

void OwnerTable::gcOwnerTable()
{
  PERDIO_DEBUG(GC,"GC:owner gc");
  int i;
  for(i=1;i<size;i++){
      OwnerEntry* o = ownerTable->getOwner(i);
      if(!(o->isFree())){
        o->gcPO();
      }
  }
  compactify();
  return;
}

/* OBSERVE - this must done at the end of other gc */
void BorrowTable::gcBorrowTable()
{
  PERDIO_DEBUG(GC,"GC:borrow gc");
  int i;
  for(i=0;i<size;i++){
    BorrowEntry *b=borrowTable->getBorrow(i);
    if(!(b->isFree())){
      if(b->gcCheck()) borrowTable->maybeFreeBorrowEntry(i);}}
  compactify();
  hshtbl->compactify();
}

/* OBSERVE - this must be done at the end of other gc */
void GNameTable::gcGNameTable()
{
  PERDIO_DEBUG(GC,"GC:gname gc");
  int index;
  GenHashNode *aux = getFirst(index);
  while(aux) {
    GName *gn = (GName*) aux->getBaseKey();
    GenHashNode *aux1 = aux;
    int oldindex = index;
    aux = getNext(aux,index);
    /* code is never garbage collected */
    if (gn->getPredMark()) {
      continue;
    }
    if (gn->getGCMark()) {
      TaggedRef t = (TaggedRef) ToInt32(aux1->getEntry());
      gcTagged(t,t);
      aux1->setEntry((GenHashEntry*)ToPointer(t));
      gn->resetGCMark();
    } else {
      delete gn;
      htSub(oldindex,aux1);
    }
  }
  compactify();
}


/**********************************************************************/
/**********************************************************************/
/*                      GLOBALIZING                                   */
/**********************************************************************/
/**********************************************************************/

void Tertiary::globalize()
{
  PERDIO_DEBUG(GLOBALIZING,"GLOBALIZING");
  Assert(sizeof(PortManager)==sizeof(PortLocal));
  setTertType(Te_Manager);
  OwnerEntry *oe;
  int i = ownerTable->newOwner(oe);
  oe->mkTertiary(this);
  setIndex(i);
}

/**********************************************************************/
/**********************************************************************/
/*                      LOCALIZING
/**********************************************************************/
/**********************************************************************/

void Tertiary::localize()
{
  PERDIO_DEBUG(GLOBALIZING,"GLOBALIZING: localizing proxy");
  Assert(sizeof(PortManager)==sizeof(PortLocal));
  setTertType(Te_Local);
  setBoard(am.rootBoard);
}

void ProcProxy::localize(RefsArray g, ProgramCounter pc)
{
  Tertiary::localize();
  gRegs = g;
  if (pc!=NOCODE) {
    pred->PC = pc;
  }
  if (OZ_unify(suspVar,NameUnit) != PROCEED) {
    warning("ProcProxy::localize: unify failed");
  }
}

/**********************************************************************/
/**********************************************************************/
/*                            MARSHALLING STUFF                       */
/**********************************************************************/
/**********************************************************************/

/**********************************************************************/
/*                Help-Classes for Marshalling                        */
/**********************************************************************/



class RefTable {
  OZ_Term *array;
  int size;
public:
  RefTable()
  {
    size = 100;
    array = new OZ_Term[size];
  }
  OZ_Term get(int i)
  {
    Assert(i<size);
    return array[i];
  }
  void set(int pos, OZ_Term val)
  {
    if (pos>=size)
      resize(pos);
    array[pos] = val;
  }
  void resize(int newsize)
  {
    int oldsize = size;
    OZ_Term  *oldarray = array;
    while(size <= newsize) {
      size = (size*3)/2;
    }
    array = new OZ_Term[size];
    for (int i=0; i<oldsize; i++) {
      array[i] = oldarray[i];
    }
    delete oldarray;
  }
};

RefTable *refTable;

class RefTrail: public Stack {
public:
  RefTrail() : Stack(200,Stack_WithMalloc) { }
  void trail(OZ_Term *t)
  {
    push(t);
    push(ToPointer(*t));
  }
  void unwind()
  {
    while(!isEmpty()) {
      OZ_Term oldval = ToInt32(pop());
      OZ_Term *loc = (OZ_Term*) pop();
      *loc = oldval;
    }
  }
};

RefTrail *refTrail;

class DebtRec:public Stack {
public:
  DebtRec():Stack(200,Stack_WithMalloc) {}

  void handler(PendEntry *pe){
    BorrowEntry *b;
    int debt,bi;
    Assert(!isEmpty());
    while(!isEmpty()) {
      bi = (int) pop();
      Assert(!isEmpty());
      debt = (int) pop();
      b=borrowTable->getBorrow(bi);
      b->inDebtSec(debt,pe);}
    return;}

  void debtPush(int debt,int i){
    PERDIO_DEBUG1(DEBT,"DEBT:push %d",i);
    push((void *)debt);
    push((void *)i);
  }
};


/**********************************************************************/
/*                 MARSHALLING/UNMARSHALLING  GROUND STRUCTURES       */
/**********************************************************************/


#define COMPRESSEDNUMBERS
#ifdef COMPRESSEDNUMBERS

#define SBit (1<<7)

inline
void marshallNumber(unsigned int i, ByteStream *bs)
{
  while(i >= SBit) {
    bs->put((i%SBit)|SBit);
    i /= SBit;
  }
  bs->put(i);
}



inline
int unmarshallNumber(ByteStream *bs)
{
  unsigned int ret = 0, shft = 0;
  unsigned int c = bs->get();
  while (c >= SBit) {
    ret += ((c-SBit) << shft);
    c = bs->get();
    shft += 7;
  }
  ret |= (c<<shft);
  return (int) ret;
}

#undef SBit

#else

const int intSize = sizeof(int32);


inline
void marshallNumber(unsigned int i, ByteStream *bs){
  PERDIO_DEBUG1(MARSHALL_CT,"MARSHALL_CT Number %d BYTES:4",i);
  for (int k=0; k<intSize; k++) {
    bs->put(i&0xFF);
    i = i>>8;}}

inline int unmarshallNumber(ByteStream *bs){
  int i;
  unsigned int i1 = bs->get();
  unsigned int i2 = bs->get();
  unsigned int i3 = bs->get();
  unsigned int i4 = bs->get();
  i=(int) (i1 + (i2<<8) + (i3<<16) + (i4<<24));
  PERDIO_DEBUG1(UNMARSHALL_CT,"UNMARSHALL_CT Number %d BYTES:4",i);
  return i;}

#endif

const int shortSize = 2;    /* TODO */

void marshallShort(unsigned short i, ByteStream *bs){
  PERDIO_DEBUG1(MARSHALL_CT,"MARSHALL_CT Short %d BYTES:2",i);
  for (int k=0; k<shortSize; k++) {
    bs->put(i&0xFF);
    i = i>>8;}}


inline int unmarshallShort(ByteStream *bs){
  unsigned short sh;
  unsigned int i1 = bs->get();
  unsigned int i2 = bs->get();
  sh= (i1 + (i2<<8));
  PERDIO_DEBUG1(UNMARSHALL_CT,"UNMARSHALL_CT Short %d BYTES:2",sh);
  return sh;}

class DoubleConv {
public:
  union {
    int32 i[2];
    double d;
  } u;
};

inline
void marshallFloat(double d, ByteStream *bs)
{
  static DoubleConv dc;
  dc.u.d = d;
  marshallNumber(dc.u.i[0],bs);
  marshallNumber(dc.u.i[1],bs);
}

inline
double unmarshallFloat(ByteStream *bs)
{
  static DoubleConv dc;
  dc.u.i[0] = unmarshallNumber(bs);
  dc.u.i[1] = unmarshallNumber(bs);
  return dc.u.d;
}

inline
char *unmarshallString(ByteStream *bs)
{
  int i = unmarshallNumber(bs);
  if(i>100){ // mm2
    int dummy=0;
    dummy=1;}

  char *ret = new char[i+1];  /* TODO: ask Ralph */
  int k=0;
  for (; k<i; k++) {
    ret[k] = bs->get();
  }
  PERDIO_DEBUG1(UNMARSHALL_CT,"UNMARSHALL_CT String BYTES:%d",k);
  ret[i] = '\0';
  return ret;
}

inline
void marshallString(char *s, ByteStream *bs)
{
  marshallNumber(strlen(s),bs);
  // mm2 \/?
  if(strlen(s)>100) {PERDIO_DEBUG1(SPECIAL,"SPECIAL string:%d",strlen(s));}
  PERDIO_DEBUG1(MARSHALL_CT,"MARSHALL_CT String BYTES:%d",strlen(s));
  while(*s) {
    bs->put(*s);
    s++;  }
}

void marshallGName(GName *gname, ByteStream *bs)
{
  marshallNumber(gname->site.ip,bs);
  marshallShort(gname->site.port,bs);
  marshallNumber(gname->site.timestamp,bs);
  for (int i=0; i<fatIntDigits; i++) {
    marshallNumber(gname->id.number[i],bs);
  }
}

void unmarshallGName(GName *gname, ByteStream *bs)
{
  gname->site.ip        = unmarshallNumber(bs);
  gname->site.port      = unmarshallShort(bs);
  gname->site.timestamp = unmarshallNumber(bs);
  for (int i=0; i<fatIntDigits; i++) {
    gname->id.number[i] = unmarshallNumber(bs);
  }
}

/**********************************************************************/
/*            MARSHALLING/UNMARSHALLING NETWORK ADDRESSES             */
/**********************************************************************/

inline
void marshallCredit(Credit credit,ByteStream *bs){
  Assert(sizeof(Credit)==sizeof(long));
  PERDIO_DEBUG1(MARSHALL,"MARSHALL-credit c=%d",credit);
  marshallNumber(credit,bs);}

inline
Credit unmarshallCredit(ByteStream *bs){
  Assert(sizeof(Credit)==sizeof(long));
  Credit c=unmarshallNumber(bs);
  PERDIO_DEBUG1(UNMARSHALL,"UNMARSHALL-credit c=%d",c);
  return c;}

inline
void marshallSite(Site *sd,ByteStream *bs){
  ip_address ip;
  port_t port;
  time_t timestamp;
  getSiteFields(sd,ip,port,timestamp);
  PERDIO_DEBUG4(MARSHALL,"MARSHALL-site (10) s:%x ip:%u p:%u t:%u",
                sd,ip,port,timestamp);
  marshallNumber(ip,bs);
  marshallShort(port,bs);
  marshallNumber(timestamp,bs);}

inline
void marshallMySite(ByteStream *bs){
  marshallSite(mySite,bs);}

inline
void marshallNetAddress2(Site* site,int index,ByteStream *bs){
  marshallSite(site,bs);
  PERDIO_DEBUG1(MARSHALL,"MARSHALL-index (4) i:%d",index);
  marshallNumber(index,bs);}

inline
void marshallNetAddress(NetAddress *a, ByteStream *bs){
  marshallNetAddress2(a->site,a->index,bs);}

inline
Site * unmarshallSiteId(ByteStream *bs){
  ip_address ip=unmarshallNumber(bs);
  port_t port = unmarshallShort(bs);
  time_t timestamp = unmarshallNumber(bs);
  Site *sd;
  if(importSite(ip,port,timestamp,sd)==NET_OK){
    PERDIO_DEBUG4(UNMARSHALL,"UNMARSHALL-site (10) id=%x ip:%d p:%u t:%u",
                  sd,ip,port,timestamp);
    return sd;}
  OZ_fail("timeStamp exception");
  return sd;}


/*
 * marshall a OT entry (i)
 */
void marshallOwnHead(int tag,int i,ByteStream *bs){
  bs->put(tag);

  OwnerEntry *o=ownerTable->getOwner(i);
  marshallNetAddress2(mySite,i,bs);
  marshallNumber(o->getSendCredit(),bs);
  PERDIO_DEBUG2(MARSHALL,"MARSHALL:owned=%d remCredit=%d ",i,o->getCredit());
}

/*
 * marshall a BT entry (bi) which is send to its owner
 */
void marshallToOwner(int bi,ByteStream *bs,DebtRec *dr){
  bs->put(M_OWNER);
  marshallNumber(borrowTable->getOriginIndex(bi),bs);
  BorrowEntry *b=borrowTable->getBorrow(bi); /* implicit 1 credit */
  if(b->getOneCredit()) {
    PERDIO_DEBUG2(MARSHALL,"MARSHALL:toOwner Borrow:%d Owner:%d",
                  bi,borrowTable->getOriginIndex(bi));
    return;}
  dr->debtPush(1,bi);
  PERDIO_DEBUG2(MARSHALL,"MARSHALL:toOwner Borrow:%d Owner:%d debt=1",
                bi,borrowTable->getOriginIndex(bi));
  return;}

/*
 * marshall a BT entry (bi) into a message
 */
void marshallBorrowHead(int tag, int bi,ByteStream *bs,DebtRec *dr){
  bs->put(tag);

  BorrowEntry *b=borrowTable->getBorrow(bi);
  marshallNetAddress(b->getNetAddress(),bs);
  Credit cred;
  if(b->getSmallCredit(cred)) {
    PERDIO_DEBUG3(MARSHALL,"MARSHALL:borrowed %d remCredit=%d give=%d",
                bi,b->getCredit(),cred);
    marshallCredit(cred,bs);
    return;  }
  PERDIO_DEBUG3(MARSHALL,"MARSHALL:borrowed %d remCredit=%d debt=",
                bi,b->getCredit(),MIN_BORROW_CREDIT_SIZE);
  marshallCredit(MIN_BORROW_CREDIT_SIZE,bs);
  dr->debtPush(MIN_BORROW_CREDIT_SIZE,bi);
  return;}

NetAddress *unmarshallNetAddress(ByteStream *bs)
{
  Site *sd=unmarshallSiteId(bs);
  int si=unmarshallNumber(bs);

  return new NetAddress(sd,si);
}

OZ_Term unmarshallBorrow(ByteStream *bs,OB_Entry *&ob,int &bi){
  Site * sd=unmarshallSiteId(bs);
  int si=unmarshallNumber(bs);
  PERDIO_DEBUG1(UNMARSHALL,"UNMARSHALL borrow index %d",si);
  Credit cred = unmarshallCredit(bs);
  if (sd==mySite){
    OZ_Term ret = ownerTable->getOwner(si)->getValue();;
    ownerTable->returnCreditAndCheck(si,cred);
    DebugCode(ob=0;bi=-4711);
    return ret;}

  NetAddress na = NetAddress(sd,si);
  int hindex;
  BorrowEntry *b = borrowTable->find(&na);
  if (b!=NULL) {
    PERDIO_DEBUG(UNMARSHALL,"UNMARSHALL:borrowed hit");
    b->addCredit(cred);
    return b->getValue();
  }
  bi=borrowTable->newBorrow(cred,sd,si);
  b=borrowTable->getBorrow(bi);
  PERDIO_DEBUG(UNMARSHALL,"UNMARSHALL:borrowed miss");
  ob=b;
  return 0;}



/**********************************************************************/
/*                 MARSHALLING terms                                  */
/**********************************************************************/


inline
Bool checkCycle(OZ_Term t, ByteStream *bs)
{
  if (!isRef(t) && tagTypeOf(t)==GCTAG) {
    PERDIO_DEBUG(MARSHALL,"MARSHALL:circular");
    bs->put(M_REF);
    marshallNumber(t>>tagSize,bs);
    return OK;
  }
  return NO;
}

inline
void trailCycle(OZ_Term *t,int r)
{
  refTrail->trail(t);
  *t = (r<<tagSize)|GCTAG;
}

void marshallTertiary(Site* sd,Tertiary *t, ByteStream *bs, DebtRec *dr)
{
  if (t->isProxy()) {
    PERDIO_DEBUG(MARSHALL,"MARSHALL: proxy");
    if (borrowTable->getOriginSite(t->getIndex())==sd) {
      marshallToOwner(t->getIndex(),bs,dr);
      return;
    }
  }

  int tag;
  switch (t->getType()) {
  case Co_Port:        tag = M_PORT;    break;
  case Co_Abstraction: tag = M_PROC;    break;
  default: Assert(0);  tag=0;
  }

  if (t->isProxy()) {
    PERDIO_DEBUG(MARSHALL,"MARSHALL: proxy");
    marshallBorrowHead(tag,t->getIndex(),bs,dr);
  } else {
    if(t->isLocal()){
      PERDIO_DEBUG(MARSHALL,"MARSHALL:proxy local");
      t->globalize();
    } else {
      PERDIO_DEBUG(MARSHALL,"MARSHALL:proxy manager");
    }
    marshallOwnHead(tag,t->getIndex(),bs);
  }

  if (t->getType() == Co_Abstraction) {
    Abstraction *a = (Abstraction *) t;
    GName *gname = a->globalize();
    marshallGName(gname,bs);
    GName *gnamecode = a->getPred()->globalize();
    marshallGName(gnamecode,bs);
    marshallTerm(sd,a->getName(),bs,dr);
    marshallNumber(a->getArity(),bs);
  }

  trailCycle(t->getRef(),bs->refCounter++);
}

void marshallVariable(Site * sd, PerdioVar *pvar, ByteStream *bs,DebtRec *dr)
{
  int i=pvar->getIndex();
  if (pvar->isProxy()) {
    PERDIO_DEBUG(MARSHALL,"MARSHALL: var proxy");
    if(borrowTable->getOriginSite(i)==sd){
      marshallToOwner(i,bs,dr);
      return;}
    marshallBorrowHead(M_VAR,i,bs,dr);
  } else {  // owner
    Assert(pvar->isManager());
    marshallOwnHead(M_VAR,i,bs);
  }
}

#include "marshallcode.cc"

void marshallTerm(Site * sd, OZ_Term t, ByteStream *bs, DebtRec *dr)
{
  OZ_Term *args;
  int argno;

loop:
  DEREF(t,tPtr,tTag);
  switch(tTag) {

  case SMALLINT:
    bs->put(M_SMALLINT);
    marshallNumber(smallIntValue(t),bs);
    PERDIO_DEBUG(MARSHALL,"MARSHALL:small int");
    break;

  case OZFLOAT:
    bs->put(M_FLOAT);
    marshallFloat(tagged2Float(t)->getValue(),bs);
    PERDIO_DEBUG(MARSHALL,"MARSHALL:float");
    break;

  case BIGINT:
    bs->put(M_BIGINT);
    marshallString(toC(t),bs);
    PERDIO_DEBUG(MARSHALL,"MARSHALL:big int");
    break;

  case LITERAL:
    {
      Literal *lit = tagged2Literal(t);
      if (lit->isAtom()) {
        bs->put(M_ATOM);
        PERDIO_DEBUG(MARSHALL_CT,"MARSHALL_CT tag M_ATOM  BYTES:1");
        marshallString(lit->getPrintName(),bs);
        PERDIO_DEBUG(MARSHALL,"MARSHALL:atom");
        break;
      }
      if (literalEq(NameTrue,t)) {
        bs->put(M_NAMETRUE);
        PERDIO_DEBUG(MARSHALL_CT,"MARSHALL_CT tag M_NAMETRUE  BYTES:1");
        break;
      }
      if (literalEq(NameFalse,t)) {
        PERDIO_DEBUG(MARSHALL_CT,"MARSHALL_CT tag M_NAMEFALSE  BYTES:1");
        bs->put(M_NAMEFALSE);
        break;
      }

      bs->put(M_NAME);
      GName *gname = ((Name*)lit)->globalize();
      marshallGName(gname,bs);
      marshallString(lit->getPrintName(),bs);
      break;
    }

  case LTUPLE:
    {
      LTuple *l = tagged2LTuple(t);
      if (checkCycle(*l->getRef(),bs)) return;
      bs->put(M_LIST);
      PERDIO_DEBUG(MARSHALL_CT,"MARSHALL_CT tag M_LIST BYTES:1");
      argno = 2;
      args  = l->getRef();
      PERDIO_DEBUG(MARSHALL,"MARSHALL:list");
      goto processArgs;
    }

  case SRECORD:
    {
      SRecord *rec = tagged2SRecord(t);
      if (checkCycle(*rec->getRef(),bs)) return; /* TODO mark instead of getRef ??*/
      if (rec->isTuple()) {
        bs->put(M_TUPLE);
        PERDIO_DEBUG(MARSHALL_CT,"MARSHALL_CT tag M_TUPLE BYTES:1");
        marshallNumber(rec->getTupleWidth(),bs);
      } else {
        bs->put(M_RECORD);
        PERDIO_DEBUG(MARSHALL_CT,"MARSHALL_CT tag M_RECORD BYTES:1");
        marshallTerm(sd,rec->getArityList(),bs,dr);
      }
      marshallTerm(sd,rec->getLabel(),bs,dr);
      argno = rec->getWidth();
      args  = rec->getRef();
      PERDIO_DEBUG1(MARSHALL,"MARSHALL:record-tuple no:%d",argno);
      goto processArgs;
    }

  case OZCONST:
    {
      PERDIO_DEBUG(MARSHALL,"MARSHALL:constterm");
      if (checkCycle(*(tagged2Const(t)->getRef()),bs))
        break;

      if (isBuiltin(t)) {
        bs->put(M_BUILTIN);
        PERDIO_DEBUG(MARSHALL_CT,"MARSHALL_CT tag M_BUILTIN BYTES:1");
        marshallTerm(sd,tagged2Builtin(t)->getName(),bs,dr);
        break;
      }
      if (!isProcedure(t) && !isPort(t))
        goto bomb;

      marshallTertiary(sd,tagged2Tert(t),bs,dr);
      break;
    }

  case UVAR:
    {
      OwnerEntry *oe;
      int i = ownerTable->newOwner(oe);
      oe->mkRef(makeTaggedRef(tPtr));

      PerdioVar *pvar=new PerdioVar();
      pvar->setIndex(i);
      doBindCVar(tPtr,pvar);

      marshallVariable(sd,pvar,bs,dr);
      break;
    }
  case SVAR:
    {
      OwnerEntry *oe;
      int i = ownerTable->newOwner(oe);
      oe->mkRef(makeTaggedRef(tPtr));

      SVariable *svar = tagged2SVar(t);
      PerdioVar *pvar=new PerdioVar();
      pvar->setSuspList(svar->getSuspList());
      pvar->setIndex(i);
      doBindCVar(tPtr,pvar);

      marshallVariable(sd,pvar,bs,dr);
      break;
    }
  case CVAR:
    switch (tagged2CVar(t)->getType()) {
    case PerdioVariable:
      {
        PerdioVar *pvar = (PerdioVar *) tagged2CVar(t);
        marshallVariable(sd,pvar,bs,dr);
        break;
      }
    default:
      t=makeTaggedRef(tPtr);
      goto bomb;
    }
    break;
  default:
  bomb:
    warning("Cannot marshall %s",toC(t));
    marshallTerm(sd,nil(),bs,dr);
    break;
  }

  return;

processArgs:
  OZ_Term arg0 = tagged2NonVariable(args);
  if (!isRef(*args) && isAnyVar(*args)) {
    int r=bs->refCounter++;
    marshallTerm(sd,arg0,bs,dr);
    trailCycle(args,r);
  } else {
    trailCycle(args,bs->refCounter++);
    marshallTerm(sd,arg0,bs,dr);
  }
  args++;
  if (argno == 1) return;
  for(int i=1; i<argno-1; i++) {
    marshallTerm(sd,tagged2NonVariable(args),bs,dr);
    args++;
  }
  // tail recursion optimization
  t = tagged2NonVariable(args);
  goto loop;
}



/**********************************************************************/
/*                 UNMARSHALLING terms                                */
/**********************************************************************/

OZ_Term unmarshallTerm(ByteStream *bs)
{
  OZ_Term ret;
  unmarshallTerm(bs,&ret);
  return ret;
}

void unmarshallTerm(ByteStream *bs, OZ_Term *ret)
{
  int argno;
loop:
  MarshallTag tag = (MarshallTag) bs->get();
  PERDIO_DEBUG1(UNMARSHALL_CT,"UNMARSHALL_CT tag %c BYTES:1",tag);

  switch(tag) {

  case M_SMALLINT:
    *ret = OZ_int(unmarshallNumber(bs));
    PERDIO_DEBUG(UNMARSHALL,"UNMARSHALL:small int");
    return;

  case M_FLOAT:
    *ret = OZ_float(unmarshallFloat(bs));
    PERDIO_DEBUG(UNMARSHALL,"UNMARSHALL:float");
    return;

  case M_NAMETRUE:  *ret=NameTrue; return;
  case M_NAMEFALSE: *ret=NameFalse; return;

  case M_NAME:
    {
      GName gname;
      unmarshallGName(&gname,bs);
      char *printname = unmarshallString(bs);

      TaggedRef aux = gnameTable->gnameFind(&gname);
      if (aux) {
        *ret = aux;
      } else {
        GName *copy = copyGName(&gname);
        if (strcmp("",printname)==0) {
          aux = OZ_newName();
        } else {
          aux = makeTaggedLiteral(NamedName::newNamedName(printname));
        }
        ((Name*)tagged2Literal(aux))->import(copy);
        gnameTable->gnameAdd(copy,aux);
        *ret = aux;
      }
      return;
    }

  case M_ATOM:
    {
      char *aux = unmarshallString(bs);
      *ret = OZ_atom(aux);
      delete aux;
      PERDIO_DEBUG1(UNMARSHALL,"UNMARSHALL:atom %s",aux);
      return;
    }

  case M_BIGINT:
    {
      char *aux = unmarshallString(bs);
      *ret = OZ_CStringToNumber(aux);
      delete aux;
      PERDIO_DEBUG(UNMARSHALL,"UNMARSHALL:big int");
      return;
    }

  case M_LIST:
    {
      OZ_Term head, tail;
      argno = 2;
      LTuple *l = new LTuple();
      *ret = makeTaggedLTuple(l);
      refTable->set(bs->refCounter++,*ret);
      ret = l->getRef();
      PERDIO_DEBUG(UNMARSHALL,"UNMARSHALL:list");
      goto processArgs;
    }
  case M_TUPLE:
    {
      argno = unmarshallNumber(bs);
      TaggedRef label = unmarshallTerm(bs);
      SRecord *rec = SRecord::newSRecord(label,argno);
      *ret = makeTaggedSRecord(rec);
      refTable->set(bs->refCounter++,*ret);
      ret = rec->getRef();
      PERDIO_DEBUG1(UNMARSHALL,"UNMARSHALL:tuple no_args:%d",argno)
      goto processArgs;
    }

  case M_RECORD:
    {
      TaggedRef arity = unmarshallTerm(bs);
      argno = length(arity);
      TaggedRef label = unmarshallTerm(bs);
      SRecord *rec = SRecord::newSRecord(label,mkArity(arity));
      *ret = makeTaggedSRecord(rec);
      refTable->set(bs->refCounter++,*ret);
      ret = rec->getRef();
      PERDIO_DEBUG1(UNMARSHALL,"UNMARSHALL:record no:%d",argno)
      goto processArgs;
    }

  case M_REF:
    {
      PERDIO_DEBUG(UNMARSHALL,"UNMARSHALL:ref");
      int i = unmarshallNumber(bs);
      *ret = refTable->get(i);
      return;
    }

  case M_OWNER:
    {
      int OTI=unmarshallNumber(bs);
      PERDIO_DEBUG1(UNMARSHALL,"UNMARSHALL:owner %d",OTI);
      *ret = OT->getOwner(OTI)->getValue();
      OT->returnCreditAndCheck(OTI,1);
      return;
    }

  case M_PORT:
    {
      OB_Entry *ob;
      int bi;
      OZ_Term val = unmarshallBorrow(bs,ob,bi);
      if (val) {
        PERDIO_DEBUG(UNMARSHALL,"UNMARSHALL:port hit");
        refTable->set(bs->refCounter++,val);
        *ret=val;
        return;
      }
      PERDIO_DEBUG(UNMARSHALL,"UNMARSHALL:port miss");
      Tertiary *tert = new PortProxy(bi);
      *ret= makeTaggedConst(tert);
      refTable->set(bs->refCounter++,*ret);
      ob->mkTertiary(tert);
      return;
    }
  case M_VAR:
    {
      OB_Entry *ob;
      int bi;
      OZ_Term val1 = unmarshallBorrow(bs,ob,bi);
      if (val1) {
        PERDIO_DEBUG(UNMARSHALL,"UNMARSHALL:var hit");
        *ret=val1;
        return;
      }
      PERDIO_DEBUG(UNMARSHALL,"UNMARSHALL:var miss");
      PerdioVar *pvar = new PerdioVar(bi);
      TaggedRef *cvar = newTaggedCVar(pvar);
      TaggedRef val = makeTaggedRef(cvar);
      *ret = val;
      ob->mkRef(val);

      if (sendRegister((BorrowEntry *)ob) != PROCEED) {
        printf("mm2: register failed"); //TODO
      }
      return;
    }
  case M_PROC:
    {
      OB_Entry *ob;
      int bi;
      OZ_Term val=unmarshallBorrow(bs,ob,bi);

      GName gname;     unmarshallGName(&gname,bs);
      GName gnamecode; unmarshallGName(&gnamecode,bs);
      OZ_Term name = unmarshallTerm(bs);
      int arity    = unmarshallNumber(bs);

      if (val) {
        *ret=val;
        refTable->set(bs->refCounter++,val);
        return;
      }

      TaggedRef aux = gnameTable->gnameFind(&gname);
      if (aux) {
        *ret = aux;
        refTable->set(bs->refCounter++,aux);
        return;
      }

      ProcProxy *pp = new ProcProxy(bi,name,arity,&gnamecode);
      TaggedRef taggedPP = makeTaggedConst(pp);
      *ret = taggedPP;

      GName *copy = copyGName(&gname);
      gnameTable->gnameAdd(copy,taggedPP);
      pp->setGName(copy);

      refTable->set(bs->refCounter++,taggedPP);
      ob->mkTertiary(pp);
      return;
    }

  case M_BUILTIN:
    {
      char *name = tagged2Literal(unmarshallTerm(bs))->getPrintName();
      BuiltinTabEntry *found = builtinTab.find(name);

      if (found == htEmpty) {
        warning("Builtin '%s' not in table.", name);
        *ret = nil();
        return;
      }

      *ret = makeTaggedConst(found);
      return;
    }

  default:
    printf("unmarshall: unexpected tag: %d\n",tag);
    Assert(0);
    *ret = nil();
    return;
  }

processArgs:
  for(int i=0; i<argno-1; i++) {
    unmarshallTerm(bs,ret++);
  }
  // tail recursion optimization
  goto loop;
}


/**********************************************************************
**********************************************************************
                            MAIN RECEIVE

**********************************************************************
**********************************************************************/

static
OZ_Term ozport=0;

void siteReceive(ByteStream* bs)
{
  bs->unmarshalBegin();

  MessageType mt= (MessageType) bs->get();
  switch (mt) {
  case M_PORTSEND:    /* M_PORTSEND index term */
    {
      PERDIO_DEBUG(MSG_RECEIVED,"MSG_RECEIVED:PORTSEND");

      int portIndex = unmarshallNumber(bs);
      OZ_Term t;
      unmarshallTerm(bs,&t);
      bs->unmarshalEnd();

      if (!t) {
        if (ozconf.debugPerdio) {
          printf("siteReceive: message PORTSEND:");
          /*      printBytes(msg,len); MERGING doesn't work*/
        }
        error("siteReceive: PORTSEND unmarshall failed\n");
      }
      if (ozconf.debugPerdio) {
        printf("siteReceive: PORTSEND '%s'\n",OZ_toC(t,10,10));
      }
      Tertiary *tert= ownerTable->getOwner(portIndex)->getTertiary();
      ownerTable->returnCreditAndCheck(portIndex,1);
      Assert(tert->checkTertiary(Co_Port,Te_Manager) ||
             tert->checkTertiary(Co_Port,Te_Local));
      sendPort(makeTaggedConst(tert),t);
      PERDIO_DEBUG(SPECIAL,"SPECIAL just after send port");
      break;
      }
  case M_PORTCLOSE:    /* M_PORTCLOSE index */
    {
      PERDIO_DEBUG(MSG_RECEIVED,"MSG_RECEIVED:PORTCLOSE");

      int portIndex = unmarshallNumber(bs);
      bs->unmarshalEnd();

      Tertiary *tert= ownerTable->getOwner(portIndex)->getTertiary();
      ownerTable->returnCreditAndCheck(portIndex,1);
      Assert(tert->checkTertiary(Co_Port,Te_Manager) ||
             tert->checkTertiary(Co_Port,Te_Local));
      closePort(makeTaggedConst(tert));
      PERDIO_DEBUG(SPECIAL,"SPECIAL just after send port");
      break;
      }
  case M_ASK_FOR_CREDIT:
    {
      PERDIO_DEBUG(MSG_RECEIVED,"MSG_RECEIVED:ASK_FOR_CREDIT");
      int na_index=unmarshallNumber(bs);
      Site* rsite=unmarshallSiteId(bs);
      bs->unmarshalEnd();
      OwnerEntry *o=ownerTable->getOwner(na_index);
      o->returnCredit(1); // don't delete entry
      Credit c= o->giveMoreCredit();
      ByteStream *bs1=bufferManager->getByteStream();
      bs1->marshalBegin();
      bs1->put(M_BORROW_CREDIT);
      NetAddress na = NetAddress(mySite,na_index);
      marshallNetAddress(&na,bs1);
      marshallCredit(c,bs1);
      PERDIO_DEBUG1(MSG_SENT,"MSG_SENT:BORROW_CREDIT %d",c);
      bs1->marshalEnd();
      reliableSendFail(rsite,bs1,TRUE,4);
      break;
    }
  case M_OWNER_CREDIT:
    {
      int index=unmarshallNumber(bs);
      Credit c=unmarshallCredit(bs);
      PERDIO_DEBUG1(MSG_RECEIVED,"MSG_RECEIVED:OWNER_CREDIT %d",c);
      bs->unmarshalEnd();

      ownerTable->returnCreditAndCheck(index,c);
      break;
    }
  case M_BORROW_CREDIT:
    {
      Site * sd=unmarshallSiteId(bs);
      Assert(sd!=mySite);
      int si=unmarshallNumber(bs);
      Credit c=unmarshallCredit(bs);
      NetAddress na=NetAddress(sd,si);
      PERDIO_DEBUG1(MSG_RECEIVED,"MSG_RECEIVED:BORROW_CREDIT %d",c);
      bs->unmarshalEnd();

      BorrowEntry *b=borrowTable->find(&na);
      Assert(b!=NULL);
      b->addAskCredit(c);
      break;
    }
  case M_GET_CLOSURE:
  case M_GET_CLOSUREANDCODE:
    {
      Bool sendCode = (mt==M_GET_CLOSUREANDCODE);
      PERDIO_DEBUG(MSG_RECEIVED,"MSG_RECEIVED:GET_CLOSUREANDCODE");
      int na_index=unmarshallNumber(bs);
      Site* rsite=unmarshallSiteId(bs);
      bs->unmarshalEnd();

      Tertiary *tert=ownerTable->getOwner(na_index)->getTertiary();
      Assert (isAbstraction(tert) && tert->isManager());
      ProcProxy *pp = (ProcProxy*) tert;

      ByteStream *bs1= bufferManager->getByteStream();
      bs1->marshalBegin();
      bs1->put(sendCode ? M_SEND_CLOSUREANDCODE : M_SEND_CLOSURE);
      NetAddress na = NetAddress(mySite,na_index);
      marshallNetAddress(&na,bs1);

      /* send globals */
      RefsArray globals = pp->getGRegs();
      int gs = globals ? pp->getGSize() : 0;
      marshallNumber(gs,bs1);
      for (int i=0; i<gs; i++) {
        marshallTerm(rsite,globals[i],bs1,debtRec);
      }
      if (sendCode) {
        marshallCode(rsite,pp->getPC(),bs1,debtRec);
      }
      bs1->marshalEnd();
      refTrail->unwind();
      if(debtRec->isEmpty()) {
        reliableSendFail(rsite,bs1,FALSE,5);
      } else {
        PERDIO_DEBUG(DEBT_SEC,"DEBT_SEC:remoteSend");
        PendEntry * pe = pendEntryManager->newPendEntry(bs1,rsite);
        debtRec->handler(pe);
      }
      break;
    }

  case M_SEND_CLOSURE:
  case M_SEND_CLOSUREANDCODE:
    {
      Site * sd=unmarshallSiteId(bs);
      Assert(sd!=mySite);
      int si=unmarshallNumber(bs);
      NetAddress na=NetAddress(sd,si);
      PERDIO_DEBUG(MSG_RECEIVED,"MSG_RECEIVED:SEND_CLOSUREANDCODE");
      Bool sendCode = (mt==M_SEND_CLOSUREANDCODE);

      BorrowEntry *b=borrowTable->find(&na);
      Assert(b!=NULL);
      Tertiary *tert = b->getTertiary();
      Assert (isAbstraction(tert) && tert->isProxy());
      ProcProxy *pp = (ProcProxy*) tert;

      int gsize = unmarshallNumber(bs);
      RefsArray globals = gsize==0 ? 0 : allocateRefsArray(gsize);

      for (int i=0; i<gsize; i++) {
        globals[i] = unmarshallTerm(bs);
      }

      ProgramCounter PC = sendCode ? unmarshallCode(bs) : NOCODE;
      bs->unmarshalEnd();
      pp->localize(globals,PC);
      break;
    }

  case M_REGISTER:
    {
      int OTI=unmarshallNumber(bs);
      Site* rsite=unmarshallSiteId(bs);
      PERDIO_DEBUG(MSG_RECEIVED,"MSG_RECEIVED:REGISTER");
      bs->unmarshalEnd();


      TaggedRef ptr = OT->getOwner(OTI)->getRef();
      OT->getOwner(OTI)->returnCredit(1);
      Assert(!OT->getOwner(OTI)->hasFullCredit());
      TaggedRef val=deref(ptr);
      if (isPerdioVar(val)) {
        tagged2PerdioVar(val)->registerSite(rsite);
      } else {
        if (sendRedirect(rsite,OTI,ptr) != PROCEED) {
          printf("mm2: redirect failed");}
      }
      break;
    }

  case M_REDIRECT:
    {
      NetAddress *na = unmarshallNetAddress(bs);
      TaggedRef val = unmarshallTerm(bs);
      bs->unmarshalEnd();
      PERDIO_DEBUG(MSG_RECEIVED,"MSG_RECEIVED:REDIRECT");

      BorrowEntry *be=BT->find(na);

      if (!be) { // if not found, then forget the redirect message
        sendCreditBack(na->site,na->index,1);
        delete na;
        return;
      }
      delete na;          /* TODO */

      TaggedRef ptr=be->getRef();
      TaggedRef v1=*tagged2Ref(ptr);
      Assert(isPerdioVar(v1));
      PerdioVar *pv = tagged2PerdioVar(v1);
      Assert(pv->isProxy());
      if (pv->hasVal()) {
        // mm2: TODO
        printf("mm2: bind twice not impl.");
      }
      pv->primBind(tagged2Ref(ptr),val);

      be->freeBorrowEntry();

      break;
    }

  case M_SURRENDER:
    {
      PERDIO_DEBUG(MSG_RECEIVED,"MSG_RECEIVED:SURRENDER");

      int OTI=unmarshallNumber(bs);
      Site* rsite=unmarshallSiteId(bs);
      TaggedRef v = unmarshallTerm(bs);
      bs->unmarshalEnd();

      TaggedRef ptr = ownerTable->getOwner(OTI)->getRef();

      TaggedRef val=deref(ptr);
      if (isPerdioVar(val)) {
        PerdioVar *pv = tagged2PerdioVar(val);
        pv->primBind(tagged2Ref(ptr),v);
        ownerTable->getOwner(OTI)->returnCredit(1); // don't delete!
        sendRedirect(pv->getProxies(),v,rsite,OTI);
      } else {
        ownerTable->returnCreditAndCheck(OTI,1);
        // ignore redirect: NOTE: v is handled by the usual garbage collection
      }
      break;
    }

  case M_ACKNOWLEDGE:
    {
      NetAddress *na = unmarshallNetAddress(bs);
      bs->unmarshalEnd();
      PERDIO_DEBUG(MSG_RECEIVED,"MSG_RECEIVED:ACKNOWLEDGE");

      BorrowEntry *be=BT->find(na);

      if (!be) { // if not found, then forget the ACK message
        sendCreditBack(na->site,na->index,1);
        delete na;
        return;
      }
      delete na; /* TODO */

      TaggedRef ptr=be->getRef();

      Assert(isPerdioVar(*tagged2Ref(ptr)));
      PerdioVar *pv = tagged2PerdioVar(*tagged2Ref(ptr));
      pv->primBind(tagged2Ref(ptr),pv->getVal());

      be->freeBorrowEntry();

      break;
    }
  default:
    error("siteReceive: unknown message %d\n",mt);
    /*    printf("\n--\n%s\n--\n",msg); MERGING not possible */
    break;
  }
}


/* ********************************************************************** */
/* ********************************************************************** */
/*              BUILTINS                                                  */
/* ********************************************************************** */
/* ********************************************************************** */

void domarshallTerm(Site * sd,OZ_Term t, ByteStream *bs)
{
  marshallTerm(sd,t,bs,debtRec);
  refTrail->unwind();
}

inline void reliableSendFail(Site * sd, ByteStream *bs,Bool p,int i){
  InterfaceCode ret=reliableSend(sd,bs,p);
  if(ret!=NET_OK){OZ_fail("reliableSend %d",i);return;}
  return;}

inline int reliableSend0(Site * sd, ByteStream *bs,Bool p){
  InterfaceCode ret=reliableSend(sd,bs,p);
  if(ret!=NET_OK){
    return OZ_raiseC("reliableSend0",1,OZ_string("ran out of tries"));}
  return PROCEED;
}

/* engine-interface */
int remoteSend(PortProxy *p, TaggedRef msg) {
  BorrowEntry *b= borrowTable->getBorrow(p->getIndex());
  ByteStream *bs = bufferManager->getByteStream();
  bs->marshalBegin();
  NetAddress *na = b->getNetAddress();
  PendEntry *pe;
  Site* site = na->site;
  int index = na->index;

  if(!(b->getOneCredit())){
      PERDIO_DEBUG(DEBT_MAIN,"DEBT_MAIN:remoteSend");
      pe= pendEntryManager->newPendEntry(bs,site,b);
      b->inDebtMain(pe);}
  else pe=NULL;

  bs->put(M_PORTSEND);
  marshallNumber(index,bs);
  domarshallTerm(site,msg,bs);
  bs->marshalEnd();
  if(pe==NULL){
    if(debtRec->isEmpty()){
      return reliableSend0(site,bs,FALSE);}
    PERDIO_DEBUG(DEBT_SEC,"DEBT_SEC:remoteSend");
    pe=pendEntryManager->newPendEntry(bs,site);
    debtRec->handler(pe);
    return PROCEED;}
  if(debtRec->isEmpty()){
    return PROCEED;}
  debtRec->handler(pe);
  return PROCEED;
}

int remoteClose(PortProxy *p) {
  BorrowEntry *b= borrowTable->getBorrow(p->getIndex());
  ByteStream *bs = bufferManager->getByteStream();
  bs->marshalBegin();
  NetAddress *na = b->getNetAddress();
  Site* site = na->site;
  int index = na->index;

  bs->put(M_PORTCLOSE);
  marshallNumber(index,bs);
  bs->marshalEnd();
  if(b->getOneCredit()) {
    return reliableSend0(site,bs,FALSE);
  }
  PERDIO_DEBUG(DEBT_MAIN,"DEBT_MAIN:remoteClose");
  PendEntry *pe = pendEntryManager->newPendEntry(bs,site,b);
  b->inDebtMain(pe);
  return PROCEED;
}

void getClosure(ProcProxy *pp, Bool getCode)
{
  ByteStream *bs= bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(getCode ? M_GET_CLOSUREANDCODE : M_GET_CLOSURE);
  int bi = pp->getIndex();
  Site* site  =  borrowTable->getOriginSite(bi);
  int index =  borrowTable->getOriginIndex(bi);
  marshallNumber(index,bs);
  marshallMySite(bs);
  bs->marshalEnd();
  PERDIO_DEBUG2(MSG_SENT,"MSG_SENT: GET_CLOSUREANDCODE sd:%d,index:%d",
                site,index);
  reliableSendFail(site,bs,FALSE,6);
  return;
}

int sendSurrender(BorrowEntry *be,OZ_Term val)
{
  ByteStream *bs= bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_SURRENDER);
  NetAddress *na = be->getNetAddress();
  Site* site = na->site;
  int index = na->index;
  marshallNumber(index,bs);
  marshallMySite(bs);
  domarshallTerm(site,val,bs);

  PERDIO_DEBUG2(MSG_SENT,"MSG_SENT:SURRENDER sd:%d,index:%d",
                site,index);
  bs->marshalEnd();

  if (be->getOneCredit()) {
    if (debtRec->isEmpty()) {
      reliableSendFail(site,bs,FALSE,7);
      return PROCEED;
    }
    PERDIO_DEBUG(DEBT_SEC,"DEBT_SEC:surrender");
    PendEntry *pe=pendEntryManager->newPendEntry(bs,site);
    debtRec->handler(pe);
    return PROCEED;
  }
  PERDIO_DEBUG(DEBT_MAIN,"DEBT_MAIN:surrender");
  PendEntry *pe= pendEntryManager->newPendEntry(bs,site,be);
  be->inDebtMain(pe);
  if(!debtRec->isEmpty()){
    debtRec->handler(pe);
  }
  return PROCEED;
}

int sendRedirect(Site* sd,int OTI,TaggedRef val)
{
  ByteStream *bs= bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_REDIRECT);
  marshallNetAddress2(mySite,OTI,bs);
  domarshallTerm(sd,val,bs);
  bs->marshalEnd();
  PERDIO_DEBUG2(MSG_SENT,"MSG_SENT:REDIRECT sd:%d,index:%d",
                sd,OTI);
  OwnerEntry *oe = OT->getOwner(OTI);
  oe->getOneCredit();

  if (debtRec->isEmpty()) {
    reliableSendFail(sd,bs,FALSE,8);
    return PROCEED;
  }

  PERDIO_DEBUG(DEBT_SEC,"DEBT_SEC:sendRedirect");
  PendEntry *pe=pendEntryManager->newPendEntry(bs,sd);
  debtRec->handler(pe);
  return PROCEED;
}

int sendAcknowledge(Site* sd,int OTI)
{
  ByteStream *bs= bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_ACKNOWLEDGE);
  marshallNetAddress2(mySite,OTI,bs);
  bs->marshalEnd();
  PERDIO_DEBUG2(MSG_SENT,"MSG_SENT:ACKNOWLEDGE sd:%d,index:%d",
                sd,OTI);

  OwnerEntry *oe = OT->getOwner(OTI);
  oe->getOneCredit();

  reliableSendFail(sd,bs,FALSE,9);
  return PROCEED;
}

int sendRedirect(ProxyList *pl,OZ_Term val, Site* ackSite, int OTI)
{

  while (pl) {
    Site* sd=pl->sd;
    ProxyList *tmp=pl->next;
    delete pl;
    pl = tmp;

    if (sd==ackSite) {
      int ret = sendAcknowledge(sd,OTI);
      Assert(ret==PROCEED);
    } else {
      int ret = sendRedirect(sd,OTI,val);
      Assert(ret==PROCEED); // TODO
    }
  }

  return PROCEED;
}

int bindPerdioVar(PerdioVar *pv,TaggedRef *lPtr,TaggedRef v)
{

  if (pv->isManager()) {
    pv->primBind(lPtr,v);
    return sendRedirect(pv->getProxies(),v,mySite,pv->getIndex());
  } else {
    Assert(pv->isProxy());
    if (pv->hasVal()) {
      // mm2: TODO
      printf("mm2: bind twice not implemented");
    }
    pv->setVal(v); // save binding for ack message
    BorrowEntry *be=BT->getBorrow(pv->getIndex());
    return sendSurrender(be,v);
  }
}

int sendCreditBack(Site* sd,int OTI,Credit c)
{
  PERDIO_DEBUG1(CREDIT,"CREDIT:give back - %d",c);
  ByteStream *bs= bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_OWNER_CREDIT);
  marshallNumber(OTI,bs);
  marshallCredit(c,bs);
  bs->marshalEnd();
  PERDIO_DEBUG3(MSG_SENT,"MSG_SENT:OWNER_CREDIT sd:%d,index:%d,credit:%d",
                sd,OTI,c);
  reliableSendFail(sd,bs,TRUE,10);
  return PROCEED;
}


// compare NAs
#define GET_ADDR(var,SD,OTI)                                            \
Site* SD;int OTI;                                                       \
if (var->isProxy()) {                                                   \
  NetAddress *na=BT->getBorrow(var->getIndex())->getNetAddress();       \
  SD=na->site;                                                          \
  OTI=na->index;                                                        \
} else {                                                                \
  SD=mySite;                                                            \
  OTI=var->getIndex();                                                  \
}

int compareNetAddress(PerdioVar *lVar,PerdioVar *rVar)
{
  GET_ADDR(lVar,lSD,lOTI);
  GET_ADDR(rVar,rSD,rOTI);
  int ret = compareSites(lSD,rSD);
  if (ret != 0) return ret;
  return lOTI<rOTI ? -1 : 1;
}

/* ********************************************************************** */
/*              BUILTINS themselves                                       */
/* ********************************************************************** */

#define CHECK_INIT                                              \
  if (!ipIsInit()) {                                            \
    return am.raise(E_ERROR,OZ_atom("ip"),"uninitialized",0);   \
                                                                  }


OZ_C_proc_begin(BIStartSite,2)
{
  OZ_declareIntArg(0,vport);
  OZ_declareArg(1,stream);
  PERDIO_DEBUG1(USER,"USER:startSite called vp:%d",vport);
  if (ozport!=0) {
    return OZ_raise(OZ_mkTupleC("perdio",1,OZ_atom("site already started")));
  }
  InterfaceCode ret=ipInitV(vport,siteReceive);
  if(ret==INVALID_VIRTUAL_PORT){
    ozport=0;
    return OZ_raiseC("startSite",1,OZ_string("invalid virtual port"));}
   if(ret==NET_RAN_OUT_OF_TRIES){
    ozport=0;
    return OZ_raiseC("startSite",1,OZ_string("ran out of tries"));}
  PERDIO_DEBUG(USER,"USER:startSite succeeded");
  Tertiary *tert;
  ozport = makeTaggedConst(new PortWithStream(am.rootBoard, stream));
  tert=tagged2Tert(ozport);
  tert->setTertType(Te_Manager);
  ownerTable->newOZPort(tert);
  return PROCEED;
}
OZ_C_proc_end

#define INIT_IP(port)                                                     \
  if (!ipIsInit()) {                                                      \
    InterfaceCode ret=ipInit(port,siteReceive);                   \
    if(ret==INVALID_VIRTUAL_PORT){                                        \
      return OZ_raiseC("startSite",1,OZ_string("invalid virtual port"));} \
    if(ret==NET_RAN_OUT_OF_TRIES){                                        \
      return OZ_raiseC("startSite",1,OZ_string("ran out of tries"));}     \
    PERDIO_DEBUG(USER,"USER:startSite succeeded");                        \
  }

OZ_C_proc_begin(BIstartServer,2)
{
  OZ_declareIntArg(0,port);
  OZ_declareNonvarArg(1,prt);

  prt=deref(prt);
  if (!isPort(prt)) {
    TypeErrorT(0,"Port");
  }

  PERDIO_DEBUG1(USER,"USER:startServer called p:%d",port);

  INIT_IP(port);

  ozport = prt;
  Tertiary *tert=tagged2Port(prt);
  tert->setTertType(Te_Manager);
  ownerTable->newOZPort(tert);

  return PROCEED;
}
OZ_C_proc_end


inline OZ_Term connect_site_aux(Site * sd){
  int bi=borrowTable->newBorrow(OWNER_GIVE_CREDIT_SIZE,sd,0);
  BorrowEntry *b=borrowTable->getBorrow(bi);
  Tertiary *tert=new PortProxy(bi);
  b->mkTertiary(tert);
  return makeTaggedConst(tert);
  }

OZ_C_proc_begin(BIConnectSite,3){
  // CHECK_INIT;
  OZ_declareVirtualStringArg(0,host);
  OZ_declareIntArg(1,vport);
  OZ_declareArg(2,out);

  PERDIO_DEBUG2(USER,"USER connectSite started vp:%d ho:%s",vport,host);
  if(ozport==0){
    return OZ_raiseC("connectSite",1,OZ_string("startSite first"));}

  Site * sd;
  InterfaceCode ret=connectSiteV(host,vport,sd,FALSE);
  if(ret==NET_OK){
    PERDIO_DEBUG(USER,"USER connectSite success");
    OZ_Term x=connect_site_aux(sd);
    return OZ_unify(out,x);}
  if(ret==NET_RAN_OUT_OF_TRIES){
    return OZ_raiseC("connectSite",1,OZ_string("ran out of tries"));}
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIstartClient,3){
  // CHECK_INIT;
  OZ_declareVirtualStringArg(0,host);
  OZ_declareIntArg(1,port);
  OZ_declareArg(2,out);

  INIT_IP(0);

  Site * sd;
  InterfaceCode ret=connectSite(host,port,sd,FALSE);
  if(ret==NET_OK){
    PERDIO_DEBUG(USER,"USER connectSite success");
    OZ_Term x=connect_site_aux(sd);
    return OZ_unify(out,x);}
  if(ret==NET_RAN_OUT_OF_TRIES){
    return OZ_raiseC("connectSite",1,OZ_string("ran out of tries"));}
  error("never here");
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIConnectSiteWait,3){
  //  CHECK_INIT;
  OZ_declareVirtualStringArg(0,host);
  OZ_declareIntArg(1,vport);
  OZ_declareArg(2,out);
  PERDIO_DEBUG2(USER,"USER connectSiteWait started vp:%d h:%s",vport,host);
  if(ozport==0){
    return OZ_raiseC("connectSiteWait",1,OZ_string("startSite first"));}

  Site * sd;
  InterfaceCode ret=connectSiteV(host,vport,sd,TRUE);
  if(ret==NET_OK){
    PERDIO_DEBUG(USER,"USER connectSiteWait success");
    OZ_Term x=connect_site_aux(sd);
    return OZ_unify(out,x);}
  if(ret==NET_RAN_OUT_OF_TRIES){
    return OZ_raiseC("connectSiteWait",1,OZ_string("ran out of tries"));}
  return PROCEED;
}
OZ_C_proc_end


BIspec perdioSpec[] = {
  {"startSite",      2, BIStartSite, 0},
  {"connectSite",    3, BIConnectSite, 0},
  {"connectSiteWait",3, BIConnectSiteWait, 0},

  {"startServer",    2, BIstartServer, 0},
  {"startClient",    3, BIstartClient, 0},
  {0,0,0,0}
};

void BIinitPerdio()
{
  BIaddSpec(perdioSpec);

  OZ_protect(&ozport);

  refTable = new RefTable();
  refTrail = new RefTrail();
  ownerTable = new OwnerTable(DEFAULT_OWNER_TABLE_SIZE);
  borrowTable = new BorrowTable(DEFAULT_BORROW_TABLE_SIZE);
  debtRec= new DebtRec();
  pendLinkManager = new PendLinkManager();
  pendEntryManager = new PendEntryManager();
  bufferManager= new BufferManager();
  mySite=NULL;
  idCounter  = new FatInt();
  gnameTable = new GNameTable();
#ifdef DEBUG_PERDIO
  printf("dvset \n");
  dvset(3);
#endif
}


#endif
