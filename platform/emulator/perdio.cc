/* -----------------------------------------------------------------------
 *  (c) Perdio Project, DFKI & SICS
 *  Universit"at des Saarlandes
 *    Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
 *  SICS
 *    Box 1263, S-16428 Sweden, Phone (+46) 8 7521500
 *  Author: brand,scheidhr, mehl
 *
 *  protocol and message layer
 * -----------------------------------------------------------------------*/

/* -----------------------------------------------------------------------
 * TODO
 *
 *   cell protocol
 *     test
 *     use proxy addressing for frames
 *   object protocol
 *     all
 *   stationary object, dictionary, array, lock
 *     all
 *   pendlinkHandle simplify
 *   chunks
 *     all
 *   builtin
 *     classify secure/insecure
 *   names
 *     true, false, unit and others (o-o)
 *       new procedure makeGlobalName(Atom)
 *     test (don't work at all?)
 *   ip
 *     cache does not work together with close detection
 *     fairness for IO
 *     errors
 *     flow control
 *     timer
 *   port
 *     close: must fail client?
 *   gen hashtable
 *     if hash value is negative: crash!
 *   owner table
 *     compactify
 *     maybe hash values: sparse tables?
 *   emulator
 *     builtin can return SUSPEND_EXTERNAL
 *        immediately stop the current thread (suspends on external event)
 *        idea: use return SUSPEND, and if am.suspVarList=0 do nothing.
 *        add to oz_stop am.suspVarList = 0!
 *        don't forget to save X register!
 *   ConstTerm/Tertiary
 *     unify and rename to Entity
 *   malloc bug for refTrail if sending Lists>100000 elements
 *   spaces
 *     Space.ask[Verbose] may suspend?
 *     Space.merge may fail.
 *     Space.clone may suspend
 *     Space.clone should return a local clone
 *     Space.choose may fail, suspend
 *     Space.inject may fail, suspend
 * -----------------------------------------------------------------------*/

#ifdef PERDIO

#ifdef INTERFACE
#pragma implementation "perdio.hh"
#endif

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "runtime.hh"
#include "ip.hh"
#include "codearea.hh"
#include "indexing.hh"

#include "perdio_debug.hh"
#include "perdio_debug.cc"
#include "genvar.hh"
#include "perdiovar.hh"
#include "gc.hh"

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
void sendSurrender(BorrowEntry *be,OZ_Term val);
void sendRedirect(Site* sd,int OTI,TaggedRef val);
void sendAcknowledge(Site* sd,int OTI);
void sendRedirect(ProxyList *pl,OZ_Term val, Site* ackSite,int OTI);
void bindPerdioVar(PerdioVar *pv,TaggedRef *lPtr,TaggedRef v);
void sendCreditBack(Site* sd,int OTI,Credit c);
inline void marshallNumber(unsigned int,ByteStream *);
inline void marshallMySite(ByteStream* );
inline void marshallCredit(Credit,ByteStream *);
inline void reliableSendFail(Site*,ByteStream *,Bool,int);

void cellReceiveContents(OwnerEntry*,TaggedRef);
void cellReceiveGet(OwnerEntry*,Site*,int);
void cellReceiveRead(OwnerEntry*,TaggedRef);
void cellReceiveRemoteRead(OwnerEntry*,TaggedRef);
void cellReceiveForward(OwnerEntry*,int,Site*,int);
void cellReceiveDump(OwnerEntry*,Site *,int);
void cellSendRemoteRead(BorrowEntry*,int,Site*,TaggedRef);
void cellSendDump(CellFrame *);
void cellDoExchange(Tertiary *,TaggedRef,TaggedRef,Thread*);
OZ_C_proc_proto(BIapply);


BorrowTable *borrowTable;
OwnerTable *ownerTable;

#define OT ownerTable
#define BT borrowTable

extern TaggedRef BI_Unify;
extern TaggedRef BI_Show;

void SiteUnify(TaggedRef val1,TaggedRef val2){
Thread *th=am.mkRunnableThread(DEFAULT_PRIORITY,am.rootBoard);
#ifdef PERDIO_DEBUG
  PD(SITE_OP,"SITE_OP: site unify called");
  if(DV->on(SITE_OP)){
    RefsArray args0=allocateRefsArray(1,NO);
    TaggedRef tr=oz_atom("SITE_OP: site unify complete";
    args[0]=tr;
    th->pushCall(BI_Show,args0,1);}
#endif
  RefsArray args = allocateRefsArray(2,NO); // with default priority
  args[0]=val1;
  args[1]=val2;
  th->pushCall(BI_Unify,args,2);
  am.scheduleThread(th);}

/*
 * Message formats
 */
enum MessageType {
  M_REMOTE_SEND,        // OTI STRING DIF (implicit 1 credit)
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
  M_CELL_GET,           // OTI* SITE-OTI (implicit CELLCREDITSIZE credits)
  M_CELL_CONTENTS,      // OTI* DIF
  M_CELL_READ,          // OTI* VAR-DIF
  M_CELL_REMOTEREAD,    // OTI* VAR-DIF
  M_CELL_FORWARD,       // OTI* INTEGER SITE-OTI*
  M_CELL_DUMP,          // OTI* SITE-OTI*
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
  M_CELL,               // NA CREDIT
  M_PROC,               // NA CREDIT NAME ARITY
  M_VAR,
  M_BUILTIN,
  DIF_CHUNK,
  DIF_THREAD,           // NA CREDIT
  DIF_SPACE,            // NA CREDIT
} MarshallTag;


void sendMessage(Tertiary *, MessageType);


/**********************************************************************/
/**********************************************************************/
/*                        INITIAL                                     */
/**********************************************************************/
/**********************************************************************/

#define PENDLINK_CUTOFF 100
#define PENDENTRY_CUTOFF 100

enum PO_TYPE {
  PO_Var,
  PO_Tert,
  PO_Ref,
  PO_Free
};

class ProtocolObject {
  int type;
protected:
  union {
    TaggedRef ref;
    Tertiary *tert;
  } u;
public:
  ProtocolObject()            { DebugCode(type=4711; u.ref=0x5a5a5a5a;)}
  Bool isTertiary()           { return type==PO_Tert; }
  Bool isDummyTertiary()      { Assert(type==PO_Tert);return u.tert==NULL;}
  Bool isRef()                { return type==PO_Ref; }
  Bool isVar()                { return type==PO_Var; }
  Bool isFree()               { return type==PO_Free; }
  void setFree()              { type = PO_Free; }
  void unsetFree()            { DebugCode(type=PO_Tert); }
  void mkTertiary(Tertiary *t){ type = PO_Tert; u.tert=t; }
  void mkDummyTert()          { type = PO_Tert; u.tert=NULL;}
  void mkRef(TaggedRef v)     { type=PO_Ref; u.ref=v; }
  void mkVar(TaggedRef v)     { type=PO_Var; u.ref=v; }
  void mkRef()                { Assert(isVar()); type=PO_Ref; }
  Tertiary *getTertiary()     { Assert(isTertiary()); return u.tert; }
  TaggedRef getRef()          { Assert(isRef()||isVar()); return u.ref; }
  TaggedRef *getPtr()         { Assert(isVar()); return tagged2Ref(getRef()); }
  PerdioVar *getVar() {
    Assert(isVar());
    TaggedRef tPtr = getRef();
    TaggedRef val = *tagged2Ref(tPtr);
    return tagged2PerdioVar(val);
  }
  void gcPO() {
    if (isTertiary()) {
      PD(GC,"OT tertiary found");
      Assert(!isDummyTertiary());
      u.tert=(Tertiary *)(u.tert->gcConstTermSpec()); /* ATTENTION */
    } else {
      Assert(isRef() || isVar());
      PD(GC,"OT var/ref");
      gcTagged(u.ref,u.ref);
    }
  }

  ProtocolObject &operator =(ProtocolObject &n); // not used

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
  PD(MARSHALL_BE,"marshal begin");
  Assert(type==BS_None);
  Assert(first==NULL);
  Assert(last==NULL);
  first=getAnother();
  last=first;
  totlen= 0;
  type=BS_Marshal;
  pos=first->head()+tcpHeaderSize;}

void ByteStream::dumpByteBuffers(){
  ByteBuffer *bb=first;
  while(bb!=NULL){
    ByteBuffer *bb1=bb;
    bb=bb->next;
    bufferManager->freeByteBuffer(bb1);
  }}

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

void BufferManager::dumpByteStream(ByteStream *bs){
  bs->dumpByteBuffers();
  freeByteStream(bs);}

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
  void print();
  void send();

  PendEntry(ByteStream *bs1,Site * sd,BorrowEntry *b=NULL){
    initialize(bs1,sd,b);
  }

  void initialize(ByteStream *bs1,Site * sd,BorrowEntry *b=NULL){
    bs=bs1;
    refCount=0;
    back=b;
    site=sd;}
  void inc() {refCount++;}
  void dec() {refCount--;}
  BorrowEntry *getBack() {return back;}
  int getrefCount() {return refCount;}
  Bool isFIFO() {return back!=NULL;}
};

void PendEntry::send(){
  reliableSendFail(site,bs,FALSE,1);
}

class PendEntryManager: public FreeListManager{
public:
  PendEntryManager():FreeListManager(PENDENTRY_CUTOFF){}

  PendEntry *newPendEntry(ByteStream *bs1,Site * sd,BorrowEntry *b=NULL) {
    FreeListEntry *f=getOne();
    if(f==NULL) {return new PendEntry(bs1,sd,b);}
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

  void print() {
    printf("Credit debt: %c\n",debt);
    pend->print();
  }
  void initialize(Credit c,PendEntry* p){
    debt=c;
    pend=p;
    next=NULL;  }

  void setTag(){ // FIFO
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

#ifdef DEBUG_CREDIT
#define START_CREDIT_SIZE        (256)
#define OWNER_GIVE_CREDIT_SIZE   (16)
#define BORROW_GIVE_CREDIT_SIZE  (4)
#define MIN_BORROW_CREDIT_SIZE   2
#define MAX_BORROW_CREDIT_SIZE   2 * OWNER_GIVE_CREDIT_SIZE
#define ASK_CREDIT_SIZE          OWNER_GIVE_CREDIT_SIZE

#define CELL_CREDIT_SIZE        (8)

#else

#define START_CREDIT_SIZE        ((1<<31) - 1)
#define OWNER_GIVE_CREDIT_SIZE   ((1<<15))
#define BORROW_GIVE_CREDIT_SIZE  ((1<<7))
#define MIN_BORROW_CREDIT_SIZE   2
#define MAX_BORROW_CREDIT_SIZE   8 * OWNER_GIVE_CREDIT_SIZE
#define ASK_CREDIT_SIZE          OWNER_GIVE_CREDIT_SIZE

#define CELL_CREDIT_SIZE        (1<<5)

#endif

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
  int findNA(NetAddress *);
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
  void print();
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
  void getCellCredit()    { requestCredit(CELL_CREDIT_SIZE);}
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

  void gcPostOwnerTable();

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
      PD(PD_VAR,"localize var o:%d",OTI);
      // localize a variable
      if (oe->isVar()) {
        PerdioVar *pvar = oe->getVar();
        SVariable *svar = new SVariable(am.rootBoard);
        svar->setSuspList(pvar->getSuspList());
        doBindSVar(oe->getPtr(),svar);
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
  return; // mm2

  Assert(size>=DEFAULT_OWNER_TABLE_SIZE);
  if(size==DEFAULT_OWNER_TABLE_SIZE) return;
  if(no_used/size< TABLE_LOW_LIMIT) return;
  PD(TABLE,"TABLE:owner compactify enter: size:%d no_used:%d",
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
    PD(TABLE,"TABLE:owner compactify free slots: new%d",newsize);
    OwnerEntry *oldarray=array;
    array = (OwnerEntry*) realloc(array,newsize*sizeof(OwnerEntry));
    Assert(array!=NULL);
    if(array!=NULL){
      size=newsize;
      init(first_free,size);
      return;}
    array=oldarray;}
  init(first_free,size);
  PD(TABLE,"TABLE:owner compactify no realloc");
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

void OwnerTable::resize(){
  int newsize = ((int) (TABLE_EXPAND_FACTOR *size));
  PD(TABLE,"TABLE:resize owner old:%d no_used:%d new:%d",
                size,no_used,newsize);
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  array = (OwnerEntry*) realloc(array,newsize*sizeof(OwnerEntry));
  Assert(array!=NULL);
  if(array==NULL){
    error("Memory allocation: Owner Table growth not possible");}
  init(size, newsize);
  size=newsize;
  PD(TABLE2,"TABLE:resize owner complete");
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

int OwnerTable::newOwner(OwnerEntry *&oe){
  if(nextfree == END_FREE) resize();
  int index = nextfree;
  nextfree = array[index].u.nextfree;
  oe = (OwnerEntry *)&(array[index]);
  oe->setCredit(START_CREDIT_SIZE);
  PD(TABLE,"owner insert: o:%d",index);
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
  PD(TABLE,"owner delete o:%d",i);
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
    printf("<%d> ",i);
    if(!(array[i].isFree())){
      oe=getOwner(i);
      printf("OWNER\n");
      oe->print();
    } else{
      printf("FREE: next:%d\n",array[i].u.nextfree);}}
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
  void inDebtInternal(PendLink *);
  Credit pendLinkCredit(Credit c);
  void pendLinkHandle();

public:

  void print();
  void makeMark(){
    pendLink = (PendLink*)((unsigned int) pendLink | BORROW_GC_MARK);}

  Bool isMarked(){
    return ((unsigned int) pendLink & BORROW_GC_MARK);}

  void removeMark(){
    pendLink = (PendLink*)((unsigned int) pendLink & (~BORROW_GC_MARK));}

  Bool isPending(){
    Assert(!isFree());
    return(pendLink!=NULL);}

  void gcBorrow(int i);
  inline void copyBorrow(BorrowEntry* from,int i){
    setCredit(from->getCredit());
    if (from->isTertiary()) {
      mkTertiary(from->getTertiary());
      if(!(from->isDummyTertiary())){
        from->getTertiary()->setIndex(i);}
    } else if (from->isVar()) {
      mkVar(from->getRef());
      from->getVar()->setIndex(i);
    } else {
      Assert(from->isRef());
      mkRef(from->getRef());
    }
    pendLink=from->pendLink;
    netaddr.set(from->netaddr.site,from->netaddr.index);
  }

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

  Site *getSite(){return netaddr.site;}
  int getOTI(){return netaddr.index;}

  void addPendLink(PendLink*);

  void freeBorrowEntry();

  void addAskCredit(Credit c){
    addToCredit(1); // at least one credit to request more
    addCredit(c-1); // rest may be use for pending messages
  }

#ifdef DEBUG_PERDIO
  void DEBUG_pendLink(PendLink *pl){
    while(pl!=NULL) {pl=pl->next;}
    return;}
#endif

  void addCredit(Credit cin){
    PD(CREDIT,"borrow add s:%s o:%d add:%d to:%d",
       pSite(getNetAddress()->site),
       getNetAddress()->index,
       cin,
       getCredit());
    if(pendLink!=NULL){
      cin=pendLinkCredit(cin);
      pendLinkHandle();
      PERDIO_DEBUG_DO(DEBUG_pendLink(pendLink));
      } // fall through with updated cin
    addToCredit(cin);
    Credit cur=getCredit();
    if(cur>MAX_BORROW_CREDIT_SIZE){
      giveBackCredit(cur-MAX_BORROW_CREDIT_SIZE);
      setCredit(MAX_BORROW_CREDIT_SIZE);}
  }

  Bool getOneAskCredit() {
    Credit c=getCredit();
    if(c==1) {
      PD(CREDIT,"getOneAskCredit failed");
      return FALSE;
    }
    PD(CREDIT,"getOneAskCredit OK");
    subFromCredit(1);
    return TRUE;}

  Bool getOneCredit() {
    Credit c=getCredit();
    Assert(c>0);
    if(c <= MIN_BORROW_CREDIT_SIZE) {
      PD(CREDIT,"getOneCredit failed");
      return FALSE;}
    PD(CREDIT,"getOneCredit OK");
    subFromCredit(1);
    return TRUE; }

  Bool getSmallCredit(Credit &c){
    Credit cur=getCredit();
    if(cur < 2 * MIN_BORROW_CREDIT_SIZE) return FALSE;
    if(cur >  2 * BORROW_GIVE_CREDIT_SIZE) c=BORROW_GIVE_CREDIT_SIZE;
    else{
      if(cur >= 2 * MIN_BORROW_CREDIT_SIZE) c=MIN_BORROW_CREDIT_SIZE;}
    PD(CREDIT,"give small credit c:%d",c);
    subFromCredit(c);
    return TRUE;}

  void inDebtMain(PendEntry *);
  void inDebtSec(Credit,PendEntry *);
  void inDebtFIFO(Credit,PendEntry *);
  void moreCredit();

  void giveBackCredit(Credit c);
  Bool fifoCanSend(PendLink *,PendEntry *pe,Bool flag);
};

void BorrowEntry::inDebtFIFO(Credit c,PendEntry *pe){
  PendLink *pl=pendLinkManager->newPendLink();
  pl->initialize(c,pe);
  pl->setTag();
  pe->inc();
  inDebtInternal(pl);}

void BorrowEntry::inDebtMain(PendEntry *pe){
  PendLink *pl=pendLinkManager->newPendLink();
  pl->initialize(1,pe);
  pe->inc();
  inDebtInternal(pl);}

void BorrowEntry::inDebtSec(Credit c,PendEntry *pe){
  PendLink *pl=pendLinkManager->newPendLink();
  pl->initialize(c,pe);
  pe->inc();
  inDebtInternal(pl);}

void BorrowEntry::inDebtInternal(PendLink *pl){
  moreCredit();
  if(pendLink==NULL) {
    PD(PENDLINK,"new- none so far");
    pendLink=pl;
    return;}
  PD(PENDLINK,"new- others around far");
  PendLink* aux=pendLink;
  while(aux->next!=NULL) aux=aux->next;
  aux->next=pl;
  return;}

Credit BorrowEntry::pendLinkCredit(Credit c){
  PendLink *pl= pendLink;
  while(c>0){
    Credit d= pl->getDebt();
    if(d>c) {
      pl->setDebt(d-c);
      return 0;}
    PD(PENDLINK,"one entry = 0");
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
      PD(PENDLINK,"cannot send due to FIFO");
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

  PD(PENDLINK,"entering debt handler");
  while(TRUE){
    if(cur==NULL) {
      *base=cur;
      return;}
    if(cur->getDebt()!=0){
      PD(PENDLINK,"ran into non-zero debt");
      *base=cur;
      moreCredit();
      return;}
    pe=cur->getPend();
    if((pe->getrefCount()==0)){
      if(fifoCanSend(cur,pe,flag)){
        PD(DELAYED_MSG_SENT,"pendLinkHandle");
        pe->send();
        msgsent=TRUE;
        pendEntryManager->deletePendEntry(pe);}
      else{
        PD(PENDLINK,"ran into fifo cannot send");
        msgsent=FALSE;}}
    else{
      PD(PENDLINK,"ran into non-zero ref ct");
      msgsent=FALSE;}
    if(cur->isTagged() && ((!flag) || (!msgsent))){
      PD(PENDLINK,"fifo restriction cannot remove");
      flag=FALSE;
      base= &(cur->next);
      cur=cur->next;}
    else{
      aux=cur->next;
      PD(PENDLINK,"removal");
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
  bs->marshalEnd();
  PD(MSG_SENT,"ASK_FOR_CREDIT s:%s o:%d",pSite(site),index);
  reliableSendFail(site,bs,TRUE,2);
}

void sendRegister(BorrowEntry *be) {
  ByteStream *bs= bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_REGISTER);
  NetAddress *na = be->getNetAddress();
  Site* site = na->site;
  int index = na->index;
  marshallNumber(index,bs);
  marshallMySite(bs);
  bs->marshalEnd();
  PD(MSG_SENT,"REGISTER s:%s o:%d",pSite(site),index);

  if (be->getOneCredit()) {  /* priority */
    reliableSendFail(site,bs,FALSE,30);
    return;
  }

  PD(DEBT_MAIN,"register");
  PendEntry *pe= pendEntryManager->newPendEntry(bs,site);
  be->inDebtMain(pe);
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

  void gcBorrowTableRoots();
  void gcBorrowTable();

  BorrowEntry* find(NetAddress *na)  {
    int i = hshtbl->findNA(na);
    if(i<0) {
      PD(LOOKUP,"borrow not found");
      return 0;
    } else {
      PD(LOOKUP,"borrow found b:%d",i);
      return borrowTable->getBorrow(i);
    }
  }

  void resize();

  int newBorrow(Credit,Site*,int);

  void maybeFreeBorrowEntry(int);

  void freeSecBorrow(int);

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
  PD(TABLE,"compactify borrow old:%d no_used:%d new:%d",
                size,no_used,newsize);
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  BorrowEntry *oldarray=array;
  array = (BorrowEntry*) malloc(newsize*sizeof(BorrowEntry));
  if(array==NULL){
    PD(TABLE,"compactify borrow NOT POSSIBLE");
    array=oldarray;
    return;}
  int oldsize=size;
  size=newsize;
  copyBorrowTable(oldarray,oldsize);
  PD(TABLE,"compactify borrow complete");
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

void BorrowTable::resize(){
  Assert(no_used==size);
  int newsize = int (TABLE_EXPAND_FACTOR*size);
  PD(TABLE,"resize borrow old:%d no_used:%d new:%d",
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
  PD(TABLE,"resize borrow complete");
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

int BorrowTable::newBorrow(Credit c,Site * sd,int off){
  if(nextfree == END_FREE) resize();
  int index=nextfree;
  nextfree= array[index].u.nextfree;
  BorrowEntry* oe = &(array[index]);
  oe->initBorrow(c,sd,off);
  PD(HASH2,"<SPECIAL>:net=%x borrow=%x owner=%x hash=%x",
                oe->getNetAddress(),array,ownerTable->array,
                hshtbl->table);
  hshtbl->add(oe->getNetAddress(),index);
  no_used++;
  PD(TABLE,"borrow insert: b:%d",index);
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
  PD(TABLE,"borrow delete: b:%d",index);
  return;}

void BorrowTable::freeSecBorrow(int index){}  /* just a reminder not used */

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

void PendEntry::print() {
  printf("refCount: %d\n",refCount);
  if (back) { printf("back:\n"); back->print(); }
  printf("bs:\n"); bs->print();
  printf("Site: s:%s\n",pSite(site));
}

void OB_Entry::print() {
  printf("Credit:%d\n",getCredit());
}

void BorrowEntry::print() {
  OB_Entry::print();
  NetAddress *na=getNetAddress();
  printf("NA: s:%s o:%d\n",pSite(na->site),na->index);
  if (pendLink) pendLink->print();
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
      printf("<%d> BORROW\n",i);
      b->print();
    } else {
      printf("<%d> FREE: next:%d\n",i,array[i].u.nextfree);}}
  printf("-----------------------------------------------\n");
}

#endif

inline int borrowEntryToIndex(BorrowEntry *b){return borrowTable->ptr2Index(b);}

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
  PD(HASH,"find Place hvalue=%d, net%d:%d",hvalue,
               na->site,na->index);
  ghn=htFindFirst(hvalue);
  NetAddress *na2;
  while(ghn!=NULL){
    na2=GenHashNode2NetAddr(ghn);
    if(na->same(na2)){
      PD(HASH,"compare success hvalue=%d bk=%x net%d:%d",
                    ghn->getKey(),ghn->getBaseKey,na2->site,na2->index);
      return TRUE;}
    PD(HASH,"compare fail hvalue=%d bk=%x net%d:%d",
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

int NetHashTable::findNA(NetAddress *na){
  GenHashNode *ghn;
  int bindex;
  int hvalue=hashFunc(na);
  if(findPlace(hvalue,na,ghn)){
    int bindex= GenHashNode2BorrowIndex(ghn);
    PD(HASH,"borrow index b:%d",bindex);
    return bindex;}
  return -1;}

void NetHashTable::add(NetAddress *na,int bindex){
  int hvalue=hashFunc(na);
  GenHashNode *ghn;
  Assert(!findPlace(hvalue,na,ghn));
  PD(HASH,"adding hvalue=%d net=%d:%d bindex=%d",
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
  PD(HASH,"deleting hvalue=%d net=%d:%d bindex=%d",
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
      printf("<%d> - s%s o:%d\n",i,pSite(na->site),na->index);
      ghn=ghn->getNext();
      while(ghn!=NULL){
        na=GenHashNode2NetAddr(ghn);
        printf("<coll> - s:%s o:%d\n",pSite(na->site),na->index);
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

void gcOwnerTable()      { ownerTable->gcOwnerTable();}
void gcPostOwnerTable()  { ownerTable->gcPostOwnerTable();}
void gcBorrowTable() { borrowTable->gcBorrowTable();}
void gcBorrowTableRoots() { borrowTable->gcBorrowTableRoots();}
void gcGNameTable()  { gnameTable->gcGNameTable();}
void gcGName(GName* name) { if (name) name->setGCMark(); }

void Tertiary::gcBorrowSec(int i){
  PD(GC,"GC-relocate borrow sec:%d",i);
  Assert(BT->getBorrow(i)->isDummyTertiary());
  (BT->getBorrow(i))->makeMark();}

void Tertiary::gcProxy(){
  int i=getIndex();
  BorrowEntry *be=BT->getBorrow(i);
  if(be->isMarked()){
    PD(GC,"relocate borrow unnecessary:%d",i);
    return;}
  be->makeMark();
  PD(GC,"GC-relocate borrow:%d old%x new %x",i,be,this);
  Assert(be->isDummyTertiary()==FALSE);
  be->mkTertiary(this);}

void Tertiary::gcManager(){
  Assert(getTertType()!=Te_Frame);
  int i=getIndex();
  PD(GC,"relocate owner:%d old%x new %x",
     i,ownerTable->getOwner(i),this);
  ownerTable->getOwner(i)->mkTertiary(this);}

void CellFrame::gcCellFrameSec(){
  int state=getState();
  PD(GC,"relocate Cell in state %d",state);
  if(state & Cell_Next){gcBorrowSec(sec->nextIndex);}
  if(state & Cell_Requested){
    gcTagged(sec->contents,sec->contents);
    gcTagged(sec->head,sec->head);
    PendThread **last= &sec->pending;
    for(PendThread *pt=sec->pending;pt;pt=pt->next){
      PendThread *newPt=new PendThread();
      newPt->thread=pt->thread->gcThread();
      *last=newPt;
      last=&newPt->next;}
    *last=NULL;
    return;}
  if(state & Cell_Valid){gcTagged(sec->contents,sec->contents);}
  return;}

void CellFrame::gcCellFrame(){
  gcBorrowSec(getManagerIndex());
  int i=getOwnerIndex();
  PD(GC,"relocate cellFrame:%d",i);
  ownerTable->getOwner(i)->mkTertiary(this);
  gcCellFrameSec();}

void CellManager::gcCellManager(){
  int i=getOwnerIndex();
  PD(GC,"relocate cellManager",i);
  ownerTable->getOwner(i)->mkTertiary(this);
  if(!isOwnCurrent()){gcBorrowSec(getCurrent());}
  CellFrame *cf=(CellFrame*)this;
  cf->gcCellFrameSec();}

void Tertiary::gcTertiary() /* TODO -remove use */
{
  switch (getTertType()) {

  case Te_Local:
    {
      break;
    }

  case Te_Proxy:
    {
      int i=getIndex();
      borrowTable->getBorrow(i)->makeMark();
      PD(GC,"relocate borrow:%d old %x new %x",
                    i,borrowTable->getBorrow(i),this);
      borrowTable->getBorrow(i)->mkTertiary(this);
      break;
    }

  case Te_Manager:
    {
      int i=getIndex();
      PD(GC,"relocate owner:%d old%x new %x",
                    i,ownerTable->getOwner(i),this);
      ownerTable->getOwner(i)->mkTertiary(this);
      break;
    }
  case Te_Frame:
    Assert(0);
  }
}

/*--------------------*/

void OwnerTable::gcOwnerTable()
{
  PD(GC,"owner gc");
  int i;
  for(i=0;i<size;i++){
      OwnerEntry* o = ownerTable->getOwner(i);
      if(!o->isFree()) {
        PD(GC,"OT o:%d",i);
        o->gcPO();
      }
  }
  compactify();
  return;
}

void OwnerTable::gcPostOwnerTable()
{
  PD(GC,"owner post gc");
  int i;
  Tertiary *t;
  for(i=1;i<size;i++){
      OwnerEntry* o = ownerTable->getOwner(i);
      if(!o->isFree() && o->isTertiary()){
        t=o->getTertiary();
        if(t->getTertType()==Te_Frame && t->getType()==Co_Cell){
          CellFrame *cf=(CellFrame *)t;
          if(cf->isAccessBit()){
            cf->resetAccessBit();
            short state=cf->getState();
            if((state & Cell_Valid) && (!(state & Cell_Dump_Asked))){
              cellSendDump(cf);}}
        }
      }
  }
  return;
}

void BorrowEntry::gcBorrow(int i) {
  if (!isFree() &&
      !isMarked() &&
      isVar()) {
    PD(GC,"BT b:%d variable found",i);
    PerdioVar *pv=getVar();
    if (pv->hasVal()) {
      PD(WEIRD,"BT b:%d pending unmarked var found",i);
      gcPO();
    }
  }
}

void BorrowTable::gcBorrowTableRoots()
{
  PD(GC,"borrow gc roots");
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    b->gcBorrow(i);
  }
}

extern TaggedRef gcTagged1(TaggedRef in);

/* OBSERVE - this must done at the end of other gc */
void BorrowTable::gcBorrowTable()
{
  PD(GC,"borrow gc");
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if (!b->isFree()) {
      if(b->isMarked()) {
        b->removeMark();
        PD(GC,"BT b:%d mark found",i);
        if (b->isVar()) {
          PD(GC,"BT b:%d variable found",i);
        } else {
          PD(GC,"BT b:%d tertiary found",i);
          Assert(b->isTertiary());
        }
      } else {
        PD(GC,"BT b:%d no mark found",i);
        borrowTable->maybeFreeBorrowEntry(i);
      }
    }
  }
  compactify();
  hshtbl->compactify();
}

/* OBSERVE - this must be done at the end of other gc */
void GNameTable::gcGNameTable()
{
  PD(GC,"gname gc");
  int index;
  GenHashNode *aux = getFirst(index);
  while(aux) {
    GName *gn = (GName*) aux->getBaseKey();
    /* code is never garbage collected */
    if (!gn->getPredMark()) {
      if (gn->getGCMark()) {
        TaggedRef t = (TaggedRef) ToInt32(aux->getEntry());
        gcTagged(t,t);
        aux->setEntry((GenHashEntry*)ToPointer(t));
        gn->resetGCMark();
      } else {
        delete gn;
        if (!htSub(index,aux)) continue;
      }
    }
    aux = getNext(aux,index);
  }
  compactify();
}

void PerdioVar::gcPerdioVar(void)
{
  int i = getIndex();
  if (isProxy()) {
    PD(GC,"PerdioVar b:%d",i);
    BorrowEntry *be=borrowTable->getBorrow(i);
    be->gcPO();
    be->makeMark();

    PendBinding **last = &u.bindings;
    for (PendBinding *bl = u.bindings; bl; bl = bl->next) {
      PendBinding *newBL = new PendBinding();
      gcTagged(bl->val,newBL->val);
      newBL->thread = bl->thread->gcThread();
      *last = newBL;
      last = &newBL->next;
    }
    *last=0;
  } else {
    PD(GC,"PerdioVar o:%d",i);
    ProxyList **last=&u.proxies;
    for (ProxyList *pl = u.proxies; pl; pl = pl->next) {
      ProxyList *newPL = new ProxyList(pl->sd,0);
      *last = newPL;
      last = &newPL->next;
    }
    *last = 0;
  }
}


TaggedRef ProcProxy::getSuspvar()
{
  if (suspVar==makeTaggedNULL()) {
    suspVar = makeTaggedRef(newTaggedUVar(am.rootBoard));
    MessageType msg = ((getPC()==NOCODE) ? M_GET_CLOSUREANDCODE : M_GET_CLOSURE);
    sendMessage(this,msg);
  }
  return suspVar;
}


TaggedRef SChunk::fetchValue()
{
  if (value==makeTaggedNULL()) {
    value = makeTaggedRef(newTaggedUVar(am.rootBoard));
    //    sendMessage(this,M_GET_CHUNK);
  }
  return value;
}


/**********************************************************************/
/**********************************************************************/
/*                      GLOBALIZING                                   */
/**********************************************************************/
/**********************************************************************/

void CellLocal::globalize(int myIndex){
  DebugIndexCheck(myIndex);

  CellManager *cm=(CellManager*)this;
  cm->setTertType(Te_Manager);
  cm->setOwnCurrent();
    cm->setOwnerIndex(myIndex);

  CellFrame *cf=(CellFrame*)this;
  TaggedRef v=val;
  PD(CELL,"val:%x v:%x",val,v);
  cf->initFromGlobalize(v);}

void CellManager::localize(){
  CellFrame *cf=(CellFrame *)this;
  Assert(cf->getState()==Cell_Valid);
  TaggedRef tr=cf->getContents();
  setBoard(am.rootBoard);
  setTertType(Te_Local);
  CellLocal *cl=(CellLocal*) this;
  cl->setValue(tr);}

void Tertiary::globalize()
{
  switch(getType()) {
  case Co_Port:{
    PD(GLOBALIZING,"GLOBALIZING port");
    setTertType(Te_Manager);
    OwnerEntry *oe;
    int i = ownerTable->newOwner(oe);
    oe->mkTertiary(this);
    setIndex(i);
    return;}
  case Co_Cell:{
    PD(GLOBALIZING,"GLOBALIZING cell");
    OwnerEntry *oe_manager;
    int manI=ownerTable->newOwner(oe_manager);
    oe_manager->mkTertiary(this);
    CellLocal *cl=(CellLocal*)this;
    cl->globalize(manI);
    return;}
  default:
    error("cannot globalize\n");
  }
}

/**********************************************************************/
/**********************************************************************/
/*                      LOCALIZING
/**********************************************************************/
/**********************************************************************/

void Tertiary::localize()
{
  switch(getType()) {
  case Co_Port:{
    Assert(getTertType()==Te_Manager);
    PD(GLOBALIZING,"GLOBALIZING: localizing port manager");
    setTertType(Te_Local);
    setBoard(am.rootBoard);
    return;}
  case Co_Cell:{
    if(getTertType()==Te_Manager){
      PD(GLOBALIZING,"GLOBALIZING: localizing cell manager");
      CellManager *cm=(CellManager*)this;
      cm->localize();
      return;}
    Assert(getTertType()==Te_Frame);
    CellFrame *cf=(CellFrame *)this;
    Assert(cf->getState()==Cell_Invalid);
    cf->convertToProxy();
    return;}
  default:
    error("cannot localize\n");
  }
}

void ProcProxy::localize(RefsArray g, ProgramCounter pc)
{
  Tertiary::localize();
  gRegs = g;
  if (pc!=NOCODE) {
    pred->PC = pc;
  }
  if (OZ_unify(suspVar,NameUnit) != PROCEED) { /* TODO - */
    warning("ProcProxy::localize: unify failed");
  }
}

void SChunk::localize(TaggedRef val)
{
  Tertiary::localize();

  if (OZ_unify(value,NameUnit) != PROCEED) {
    warning("ProcProxy::localize: unify failed");
  }

  value = val;
}

void CellProxy::convertToFrame(int myIndex){
  DebugIndexCheck(myIndex);
  CellFrame *cf=(CellFrame*)this;
  setTertType(Te_Frame);
  BT->getBorrow(cf->getManagerIndex())->mkDummyTert();
  cf->initFromProxy(myIndex);}

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
    PD(DEBT,"push %d",i);
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
  PD(MARSHALL_CT,"Number %d BYTES:4",i);
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
  PD(UNMARSHALL_CT,"Number %d BYTES:4",i);
  return i;}

#endif

const int shortSize = 2;    /* TODO */

void marshallShort(unsigned short i, ByteStream *bs){
  PD(MARSHALL_CT,"Short %d BYTES:2",i);
  for (int k=0; k<shortSize; k++) {
    bs->put(i&0xFF);
    i = i>>8;}}


inline int unmarshallShort(ByteStream *bs){
  unsigned short sh;
  unsigned int i1 = bs->get();
  unsigned int i2 = bs->get();
  sh= (i1 + (i2<<8));
  PD(UNMARSHALL_CT,"Short %d BYTES:2",sh);
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

  char *ret = new char[i+1];  /* TODO: ask Ralph */
  int k=0;
  for (; k<i; k++) {
    ret[k] = bs->get();
  }
  PD(UNMARSHALL_CT,"String BYTES:%d",k);
  ret[i] = '\0';
  return ret;
}

inline
void marshallString(char *s, ByteStream *bs)
{
  marshallNumber(strlen(s),bs);
  PD(MARSHALL_CT,"String BYTES:%d",strlen(s));
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
  PD(MARSHALL,"credit c:%d",credit);
  marshallNumber(credit,bs);}

inline
Credit unmarshallCredit(ByteStream *bs){
  Assert(sizeof(Credit)==sizeof(long));
  Credit c=unmarshallNumber(bs);
  PD(UNMARSHALL,"credit c:%d",c);
  return c;}

inline
void marshallSite(Site *sd,ByteStream *bs){
  ip_address ip;
  port_t port;
  time_t timestamp;
  PD(MARSHALL,"site (10) s:%s", pSite(sd));
  getSiteFields(sd,ip,port,timestamp);
  marshallNumber(ip,bs);
  marshallShort(port,bs);
  marshallNumber(timestamp,bs);}

inline
void marshallMySite(ByteStream *bs){
  marshallSite(mySite,bs);}

inline
void marshallNetAddress2(Site* site,int index,ByteStream *bs){
  marshallSite(site,bs);
  PD(MARSHALL,"index (4) o:%d",index);
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
    PD(UNMARSHALL,"site (10) s:%s", pSite(sd));
    return sd;}
  OZ_warning("timeStamp exception\n");
  return sd;}


/*
 * marshall a OT entry (i)
 */
void marshallOwnHead(int tag,int i,ByteStream *bs){
  bs->put(tag);

  OwnerEntry *o=ownerTable->getOwner(i);
  marshallNetAddress2(mySite,i,bs);
  marshallNumber(o->getSendCredit(),bs);
  PD(MARSHALL,"ownHead o:%d rest-c:%d ",i,o->getCredit());
}

/*
 * marshall a BT entry (bi) which is send to its owner
 */
void marshallToOwner(int bi,ByteStream *bs,DebtRec *dr){
  bs->put(M_OWNER);
  marshallNumber(borrowTable->getOriginIndex(bi),bs);
  BorrowEntry *b=borrowTable->getBorrow(bi); /* implicit 1 credit */
  if(b->getOneCredit()) {
    PD(MARSHALL,"toOwner Borrow b:%d Owner o:%d",
                  bi,borrowTable->getOriginIndex(bi));
    return;}
  dr->debtPush(1,bi);
  PD(MARSHALL,"toOwner Borrow b:%d Owner o:%d debt=1",
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
    PD(MARSHALL,"borrowed b:%d remCredit c:%d give c:%d",
                bi,b->getCredit(),cred);
    marshallCredit(cred,bs);
    return;  }
  PD(MARSHALL,"borrowed b:%d remCredit c:%d debt c:%d",
                bi,b->getCredit(),MIN_BORROW_CREDIT_SIZE);
  marshallCredit(MIN_BORROW_CREDIT_SIZE,bs);
  dr->debtPush(MIN_BORROW_CREDIT_SIZE,bi);
  return;}

OZ_Term unmarshallBorrow(ByteStream *bs,OB_Entry *&ob,int &bi){
  Site * sd=unmarshallSiteId(bs);
  int si=unmarshallNumber(bs);
  PD(UNMARSHALL,"borrow o:%d",si);
  Credit cred = unmarshallCredit(bs);
  if (sd==mySite){
    OZ_Term ret = ownerTable->getOwner(si)->getValue();;
    ownerTable->returnCreditAndCheck(si,cred);
    DebugCode(ob=0;bi=-4711);
    return ret;}

  NetAddress na = NetAddress(sd,si);
  BorrowEntry *b = borrowTable->find(&na);
  if (b!=NULL) {
    PD(UNMARSHALL,"borrow found");
    b->addCredit(cred);
    return b->getValue();
  }
  bi=borrowTable->newBorrow(cred,sd,si);
  b=borrowTable->getBorrow(bi);
  PD(UNMARSHALL,"borrowed miss");
  ob=b;
  return 0;}



/**********************************************************************/
/*                 MARSHALLING terms                                  */
/**********************************************************************/


inline
Bool checkCycle(OZ_Term t, ByteStream *bs)
{
  if (!isRef(t) && tagTypeOf(t)==GCTAG) {
    PD(MARSHALL,"circular: %d",t>>tagSize);
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

Bool marshallTertiary(Site* sd,Tertiary *t, ByteStream *bs, DebtRec *dr)
{
  int tag;
  switch (t->getType()) {
  case Co_Port:        tag = M_PORT;     break;
  case Co_Abstraction: tag = M_PROC;     break;
  case Co_Cell:        tag = M_CELL;    break;
  case Co_Thread:      tag = DIF_THREAD; break;
  case Co_Space:       tag = DIF_SPACE;  break;
  case Co_Chunk:       tag = DIF_CHUNK;  break;
  default: return NO;}

  switch(t->getTertType()){
  case Te_Local:
    t->globalize();
  case Te_Manager:
    PD(MARSHALL,"manager");
    marshallOwnHead(tag,t->getIndex(),bs);
    break;
  case Te_Frame:{
    PD(MARSHALL,"frame");
    Assert(t->getType()==Co_Cell);
    CellFrame *cf=(CellFrame *)t;
    marshallOwnHead(tag,cf->getOwnerIndex(),bs);
    break;}
  case Te_Proxy:
    PD(MARSHALL,"proxy");
    if (sd && borrowTable->getOriginSite(t->getIndex())==sd) {
      marshallToOwner(t->getIndex(),bs,dr);
      return OK;}
    marshallBorrowHead(tag,t->getIndex(),bs,dr);
    break;
  default:
    Assert(0);}

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
  return OK;
}


void unmarshallTertiary(ByteStream *bs, TaggedRef *ret, MarshallTag tag, char *comment)
{
  OB_Entry *ob;
  int bi;
  OZ_Term val = unmarshallBorrow(bs,ob,bi);
  if (val) {
    PD(UNMARSHALL,"%s hit b:%d",bi,comment);
    refTable->set(bs->refCounter++,val);
    *ret=val;
    return;
  }
  PD(UNMARSHALL,"%s miss b:%d",bi,comment);

  Tertiary *tert;
  switch (tag) {
  case M_PORT:
    tert = new PortProxy(bi);
    break;
  case DIF_THREAD:
    tert = new Thread(bi,Te_Proxy);
    break;
  case DIF_SPACE:
    tert = new Space(bi,Te_Proxy);
    break;
  case DIF_CHUNK:
    tert = new SChunk(bi,Te_Proxy);
    break;
  default:
    Assert(0);
  }

  *ret= makeTaggedConst(tert);
  refTable->set(bs->refCounter++,*ret);
  ob->mkTertiary(tert);
}

void marshallVariable(Site * sd, PerdioVar *pvar, ByteStream *bs,DebtRec *dr)
{
  int i=pvar->getIndex();
  if (pvar->isProxy()) {
    PD(MARSHALL,"var proxy o:%d",i);
    if(sd && borrowTable->getOriginSite(i)==sd){
      marshallToOwner(i,bs,dr);
      return;}
    marshallBorrowHead(M_VAR,i,bs,dr);
  } else {  // owner
    PD(MARSHALL,"var manager o:%d",i);
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
    PD(MARSHALL,"small int: %d",smallIntValue(t));
    break;

  case OZFLOAT:
    bs->put(M_FLOAT);
    marshallFloat(tagged2Float(t)->getValue(),bs);
    PD(MARSHALL,"float");
    break;

  case BIGINT:
    bs->put(M_BIGINT);
    marshallString(toC(t),bs);
    PD(MARSHALL,"big int");
    break;

  case LITERAL:
    {
      Literal *lit = tagged2Literal(t);
      if (lit->isAtom()) {
        bs->put(M_ATOM);
        PD(MARSHALL_CT,"tag M_ATOM  BYTES:1");
        marshallString(lit->getPrintName(),bs);
        PD(MARSHALL,"atom: %s",lit->getPrintName());
        break;
      }
      if (literalEq(NameTrue,t)) {
        bs->put(M_NAMETRUE);
        PD(MARSHALL_CT,"tag M_NAMETRUE  BYTES:1");
        break;
      }
      if (literalEq(NameFalse,t)) {
        PD(MARSHALL_CT,"tag M_NAMEFALSE  BYTES:1");
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
      PD(MARSHALL_CT,"tag M_LIST BYTES:1");
      argno = 2;
      args  = l->getRef();
      PD(MARSHALL,"list");
      goto processArgs;
    }

  case SRECORD:
    {
      SRecord *rec = tagged2SRecord(t);
      if (checkCycle(*rec->getRef(),bs)) return; /* TODO mark instead of getRef ??*/
      if (rec->isTuple()) {
        bs->put(M_TUPLE);
        PD(MARSHALL_CT,"tag M_TUPLE BYTES:1");
        marshallNumber(rec->getTupleWidth(),bs);
      } else {
        bs->put(M_RECORD);
        PD(MARSHALL_CT,"tag M_RECORD BYTES:1");
        marshallTerm(sd,rec->getArityList(),bs,dr);
      }
      marshallTerm(sd,rec->getLabel(),bs,dr);
      argno = rec->getWidth();
      args  = rec->getRef();
      PD(MARSHALL,"record-tuple no:%d",argno);
      goto processArgs;
    }

  case OZCONST:
    {
      PD(MARSHALL,"constterm");
      if (checkCycle(*(tagged2Const(t)->getRef()),bs))
        break;

      if (isBuiltin(t)) {
        bs->put(M_BUILTIN);
        PD(MARSHALL_CT,"tag M_BUILTIN BYTES:1");
        marshallString(tagged2Builtin(t)->getPrintName(),bs);
        break;
      }
      if (!marshallTertiary(sd,tagged2Tert(t),bs,dr)) goto bomb;
      break;
    }

  case UVAR:
    {
      OwnerEntry *oe;
      int i = ownerTable->newOwner(oe);
      oe->mkVar(makeTaggedRef(tPtr));

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
      oe->mkVar(makeTaggedRef(tPtr));

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
  PD(UNMARSHALL_CT,"tag %c BYTES:1",tag);

  switch(tag) {

  case M_SMALLINT:
    *ret = OZ_int(unmarshallNumber(bs));
    PD(UNMARSHALL,"small int");
    return;

  case M_FLOAT:
    *ret = OZ_float(unmarshallFloat(bs));
    PD(UNMARSHALL,"float");
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
      PD(UNMARSHALL,"atom %s",aux);
      *ret = OZ_atom(aux);
      delete aux;
      return;
    }

  case M_BIGINT:
    {
      char *aux = unmarshallString(bs);
      PD(UNMARSHALL,"big int %s",aux);
      *ret = OZ_CStringToNumber(aux);
      delete aux;
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
      PD(UNMARSHALL,"list");
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
      PD(UNMARSHALL,"tuple no_args:%d",argno);
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
      PD(UNMARSHALL,"record no:%d",argno);
      goto processArgs;
    }

  case M_REF:
    {
      PD(UNMARSHALL,"ref");
      int i = unmarshallNumber(bs);
      *ret = refTable->get(i);
      return;
    }

  case M_OWNER:
    {
      int OTI=unmarshallNumber(bs);
      PD(UNMARSHALL,"OWNER o:%d",OTI);
      *ret = OT->getOwner(OTI)->getValue();
      OT->returnCreditAndCheck(OTI,1);
      return;
    }

  case M_PORT:     unmarshallTertiary(bs,ret,tag,"port");   return;
  case DIF_THREAD: unmarshallTertiary(bs,ret,tag,"thread"); return;
  case DIF_SPACE:  unmarshallTertiary(bs,ret,tag,"space");  return;
  case DIF_CHUNK:  unmarshallTertiary(bs,ret,tag,"chunk");  return;

  case M_CELL:
    {
      OB_Entry *ob;
      int bi;
      OZ_Term val = unmarshallBorrow(bs,ob,bi);
      if (val) {
        PD(UNMARSHALL,"cell hit b:%d",bi);
        refTable->set(bs->refCounter++,val);
        Tertiary *t=BT->getBorrow(bi)->getTertiary();
        if((t->getType()==Co_Cell) && (t->getTertType()==Te_Frame)){
          CellFrame *cf=(CellFrame *)t;
          if(cf->getState() & Cell_Dump_Asked){
            cf->setState(cf->getState() & ~Cell_Dump_Asked);}}
        *ret=val;
        return;
      }
      PD(UNMARSHALL,"cell miss b:%d",bi);
      Tertiary *tert = new CellProxy(bi);
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
        PD(UNMARSHALL,"var hit: b:%d",bi);
        *ret=val1;
        return;
      }
      PD(UNMARSHALL,"var miss: b:%d",bi);
      PerdioVar *pvar = new PerdioVar(bi);
      TaggedRef *cvar = newTaggedCVar(pvar);
      TaggedRef val = makeTaggedRef(cvar);
      *ret = val;
      ob->mkVar(val);

      sendRegister((BorrowEntry *)ob);
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
      char *name = unmarshallString(bs);
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
  Assert(am.currentBoard==am.rootBoard);

  bs->unmarshalBegin();

  MessageType mt= (MessageType) bs->get();
  switch (mt) {
  case M_REMOTE_SEND:    /* index string term */
    {
      int i = unmarshallNumber(bs);
      char *biName = unmarshallString(bs);
      OZ_Term t;
      unmarshallTerm(bs,&t);
      Assert(t);
      bs->unmarshalEnd();
      PD(MSG_RECEIVED,"REMOTE_SEND: o:%d bi:%s v:%s",i,biName,toC(t));

      Tertiary *tert= ownerTable->getOwner(i)->getTertiary();
      ownerTable->returnCreditAndCheck(i,1);

      BuiltinTabEntry *found = builtinTab.find(biName);

      if (!found) {
        PD(WEIRD,"builtin %s not found",biName);
        break;
      }

      RefsArray args=allocateRefsArray(2,NO);
      args[0]=makeTaggedConst(tert);
      args[1]=t;
      int arity=found->getArity();
      Assert(arity<=2);
      OZ_Return ret = found->getFun()(arity,args);
      if (ret != PROCEED) {
        PD(SPECIAL,"REMOTE_SEND failed: %d\n",ret);
      }
      break;
    }
  case M_ASK_FOR_CREDIT:
    {
      int na_index=unmarshallNumber(bs);
      Site* rsite=unmarshallSiteId(bs);
      bs->unmarshalEnd();
      PD(MSG_RECEIVED,"ASK_FOR_CREDIT o:%d s:%s",
         na_index,pSite(rsite));
      OwnerEntry *o=ownerTable->getOwner(na_index);
      o->returnCredit(1); // don't delete entry
      Credit c= o->giveMoreCredit();
      ByteStream *bs1=bufferManager->getByteStream();
      bs1->marshalBegin();
      bs1->put(M_BORROW_CREDIT);
      NetAddress na = NetAddress(mySite,na_index);
      marshallNetAddress(&na,bs1);
      marshallCredit(c,bs1);
      bs1->marshalEnd();
      PD(MSG_SENT,"BORROW_CREDIT s:%s o:%d c:%d", pSite(rsite),na_index,c);
      reliableSendFail(rsite,bs1,TRUE,4);
      break;
    }
  case M_OWNER_CREDIT:
    {
      int index=unmarshallNumber(bs);
      Credit c=unmarshallCredit(bs);
      bs->unmarshalEnd();
      PD(MSG_RECEIVED,"OWNER_CREDIT o:%d c:%d",index,c);

      ownerTable->returnCreditAndCheck(index,c);
      break;
    }
  case M_BORROW_CREDIT:
    {
      Site * sd=unmarshallSiteId(bs);
      Assert(sd!=mySite);
      int si=unmarshallNumber(bs);
      Credit c=unmarshallCredit(bs);
      bs->unmarshalEnd();
      PD(MSG_RECEIVED,"BORROW_CREDIT s:%s o:%d c:%d",pSite(sd),si,c);

      NetAddress na=NetAddress(sd,si);

      BorrowEntry *b=borrowTable->find(&na);
      Assert(b!=NULL);
      b->addAskCredit(c);
      break;
    }
  case M_GET_CLOSURE:
  case M_GET_CLOSUREANDCODE:
    {
      Bool sendCode = (mt==M_GET_CLOSUREANDCODE);
      int na_index=unmarshallNumber(bs);
      Site* rsite=unmarshallSiteId(bs);
      bs->unmarshalEnd();
      PD(MSG_RECEIVED,"GET_CLOSURE[ANDCODE] o:%d s:%s",na_index,pSite(rsite));

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
      PD(MSG_SENT,"SEND_CLOSURE[ANDCODE] s:%s o:%d",pSite(rsite),na_index);
      refTrail->unwind();
      if(debtRec->isEmpty()) {
        reliableSendFail(rsite,bs1,FALSE,5);
      } else {
        PD(DEBT_SEC,"remoteSend");
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
      PD(MSG_RECEIVED,"SEND_CLOSURE[ANDCODE] s:%s o:%d",pSite(sd),si);
      pp->localize(globals,PC);
      break;
    }

  case M_REGISTER:
    {
      int OTI=unmarshallNumber(bs);
      Site* rsite=unmarshallSiteId(bs);
      bs->unmarshalEnd();
      PD(MSG_RECEIVED,"REGISTER o:%d s:%s",OTI,pSite(rsite));


      OwnerEntry *oe = OT->getOwner(OTI);
      if (oe->isVar()) {
        PerdioVar *pv=oe->getVar();
        if (!pv->isRegistered(rsite)) {
          pv->registerSite(rsite);
        } else {
          PD(WEIRD,"REGISTER o:%d s:%s already registered",OTI,pSite(rsite));
        }

      } else {
        sendRedirect(rsite,OTI,oe->getRef());
      }
      ownerTable->returnCreditAndCheck(OTI,1);
      break;
    }

  case M_REDIRECT:
    {
      Site *sd=unmarshallSiteId(bs);
      int si=unmarshallNumber(bs);

      TaggedRef val = unmarshallTerm(bs);
      bs->unmarshalEnd();
      PD(MSG_RECEIVED,"REDIRECT s:%s o:%d v:%s",pSite(sd),si,toC(val));

      NetAddress na=NetAddress(sd,si);
      BorrowEntry *be=BT->find(&na);

      if (!be) { // if not found, then forget the redirect message
        PD(WEIRD,"REDIRECT: no borrow entry found");
        sendCreditBack(na.site,na.index,1);
        return;
      }
      be->addCredit(1);

      Assert(be->isVar());
      PerdioVar *pv = be->getVar();
      PD(TABLE,"REDIRECT - borrow entry hit b:%d",pv->getIndex());
      Assert(pv->isProxy());
      pv->primBind(be->getPtr(),val);
      be->mkRef();

      if (pv->hasVal()) {
        PD(PD_VAR,"REDIRECT while pending");
        pv->redirect(val);
      }

      // pv->dispose();
      BT->maybeFreeBorrowEntry(pv->getIndex());

      break;
    }

  case M_SURRENDER:
    {
      int OTI=unmarshallNumber(bs);
      Site* rsite=unmarshallSiteId(bs);
      TaggedRef v = unmarshallTerm(bs);
      bs->unmarshalEnd();
      PD(MSG_RECEIVED,"SURRENDER s:%s o:%d v:%s", pSite(rsite), OTI, toC(v));

      OwnerEntry *oe = ownerTable->getOwner(OTI);

      if (oe->isVar()) {
        PD(PD_VAR,"SURRENDER do it");
        PerdioVar *pv = oe->getVar();
        // bug fixed: may be bound to a different perdio var
        pv->primBind(oe->getPtr(),v);
        oe->mkRef();
        oe->returnCredit(1); // don't delete!
        if (oe->hasFullCredit()) {
          PD(WEIRD,"SURRENDER: full credit");
        }
        sendRedirect(pv->getProxies(),v,rsite,OTI);
      } else {
        PD(PD_VAR,"SURRENDER discard");
        PD(WEIRD,"SURRENDER discard");
        // ignore redirect: NOTE: v is handled by the usual garbage collection
        ownerTable->returnCreditAndCheck(OTI,1);
      }
      break;
    }

  case M_ACKNOWLEDGE:
    {
      Site *sd=unmarshallSiteId(bs);
      int si=unmarshallNumber(bs);
      bs->unmarshalEnd();
      PD(MSG_RECEIVED,"ACKNOWLEDGE s:%s o:%d",pSite(sd),si);

      NetAddress na=NetAddress(sd,si);
      BorrowEntry *be=BT->find(&na);
      Assert(be);
      be->addCredit(1);

      Assert(be->isVar());
      PerdioVar *pv = be->getVar();
      pv->acknowledge(be->getPtr());
      be->mkRef();

      // pv->dispose();
      BT->maybeFreeBorrowEntry(pv->getIndex());

      break;
    }

    case M_CELL_GET:
    {
      int OTI=unmarshallNumber(bs);
      Site* rsite=unmarshallSiteId(bs);
      int OTI2=unmarshallNumber(bs);
      bs->unmarshalEnd();
      OwnerEntry *oe=ownerTable->getOwner(OTI);
      oe->returnCredit(1);
      cellReceiveGet(oe,rsite,OTI2);
      break;
    }
  case M_CELL_CONTENTS:
    {
      int OTI=unmarshallNumber(bs);
      TaggedRef val=unmarshallTerm(bs);
      bs->unmarshalEnd();
      OwnerEntry *oe=ownerTable->getOwner(OTI);
      oe->returnCredit(1);
      cellReceiveContents(oe,val);
      break;
    }
  case M_CELL_READ:
    {
      int OTI=unmarshallNumber(bs);
      TaggedRef val=unmarshallTerm(bs); // perb-can be optimized
      bs->unmarshalEnd();
      OwnerEntry *oe=ownerTable->getOwner(OTI);
      oe->returnCredit(1);
      cellReceiveRead(oe,val);
      break;
    }
  case M_CELL_REMOTEREAD:
    {
      int OTI=unmarshallNumber(bs);
      TaggedRef val=unmarshallTerm(bs);
      bs->unmarshalEnd();
      OwnerEntry *oe=ownerTable->getOwner(OTI);
      oe->returnCredit(1);
      cellReceiveRemoteRead(oe,val);
      break;
    }
  case M_CELL_FORWARD:
    {
      int OTI=unmarshallNumber(bs);
      int ctr=unmarshallNumber(bs);
      Site* rsite=unmarshallSiteId(bs);
      int OTI2=unmarshallNumber(bs);
      bs->unmarshalEnd();
      OwnerEntry *oe=ownerTable->getOwner(OTI);
      oe->returnCredit(1);
      cellReceiveForward(oe,ctr,rsite,OTI2);
      break;
    }
  case M_CELL_DUMP:
    {
      int OTI=unmarshallNumber(bs);
      Site* rsite=unmarshallSiteId(bs);
      int OTI2=unmarshallNumber(bs);
      bs->unmarshalEnd();
      OwnerEntry *oe=ownerTable->getOwner(OTI);
      oe->returnCredit(1);
      cellReceiveDump(oe,rsite,OTI2);
      break;
    }
  default:
    error("siteReceive: unknown message %d\n",mt);
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
  if(ret!=NET_OK){OZ_warning("reliableSend %d failed\n",i);}}

/* engine-interface */
OZ_Return remoteSend(Tertiary *p, char *biName, TaggedRef msg) {
  BorrowEntry *b= borrowTable->getBorrow(p->getIndex());
  NetAddress *na = b->getNetAddress();
  Site* site = na->site;
  int index = na->index;

  ByteStream *bs = bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_REMOTE_SEND);
  marshallNumber(index,bs);
  marshallString(biName,bs);
  domarshallTerm(site,msg,bs);
  bs->marshalEnd();

  PD(MSG_SENT,"REMOTE_SEND s:%s o:%d bi:%s v:%s",
     pSite(site),index,biName,toC(msg));

  if (b->getOneCredit()) {
    if(debtRec->isEmpty()) {
      reliableSendFail(site,bs,FALSE,11);
    } else {
      PD(DEBT_SEC,"remoteSend");
      PendEntry *pe=pendEntryManager->newPendEntry(bs,site,b);
      b->inDebtFIFO(0,pe);
      debtRec->handler(pe);
    }
  } else {
    PD(DEBT_MAIN,"remoteSend");
    PendEntry *pe=pendEntryManager->newPendEntry(bs,site,b);
    pe= pendEntryManager->newPendEntry(bs,site,b);
    b->inDebtFIFO(1,pe);
    if(!debtRec->isEmpty()){
      PD(DEBT_SEC,"remoteSend");
      debtRec->handler(pe);
    }
  }
  return PROCEED;
}

void sendMessage(Tertiary *tert, MessageType msg)
{
  ByteStream *bs= bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(msg);
  int bi = tert->getIndex();
  Site* site  =  borrowTable->getOriginSite(bi);
  int index =  borrowTable->getOriginIndex(bi);
  marshallNumber(index,bs);
  marshallMySite(bs);
  bs->marshalEnd();
  PD(MSG_SENT,"GET_CLOSURE[ANDCODE] s:%s o:%d", pSite(site),index);
  reliableSendFail(site,bs,FALSE,6);
}

void sendSurrender(BorrowEntry *be,OZ_Term val)
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
  bs->marshalEnd();
  PD(MSG_SENT,"SURRENDER s:%s o:%d v:%s", pSite(site),index,toC(val));

  if (be->getOneCredit()) {
    if (debtRec->isEmpty()) {
      reliableSendFail(site,bs,FALSE,7);
      return;
    }
    PD(DEBT_SEC,"surrender");
    PendEntry *pe=pendEntryManager->newPendEntry(bs,site);
    debtRec->handler(pe);
    return;
  }
  PD(DEBT_MAIN,"surrender");
  PendEntry *pe= pendEntryManager->newPendEntry(bs,site);
  be->inDebtMain(pe);
  if(!debtRec->isEmpty()){
    debtRec->handler(pe);
  }
}

void sendRedirect(Site* sd,int OTI,TaggedRef val)
{
  ByteStream *bs= bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_REDIRECT);
  marshallNetAddress2(mySite,OTI,bs);
  domarshallTerm(sd,val,bs);
  bs->marshalEnd();
  PD(MSG_SENT,"REDIRECT s:%s o:%d v:%s",pSite(sd),OTI,toC(val));
  OwnerEntry *oe = OT->getOwner(OTI);
  oe->getOneCredit();

  if (debtRec->isEmpty()) {
    reliableSendFail(sd,bs,FALSE,8);
    return;
  }

  PD(DEBT_SEC,"sendRedirect");
  PendEntry *pe=pendEntryManager->newPendEntry(bs,sd);
  debtRec->handler(pe);
}

void sendAcknowledge(Site* sd,int OTI)
{
  ByteStream *bs= bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_ACKNOWLEDGE);
  marshallNetAddress2(mySite,OTI,bs);
  bs->marshalEnd();
  PD(MSG_SENT,"ACKNOWLEDGE s:%s o:%d",pSite(sd),OTI);

  OwnerEntry *oe = OT->getOwner(OTI);
  oe->getOneCredit();

  reliableSendFail(sd,bs,FALSE,9);
}


void PerdioVar::acknowledge(OZ_Term *ptr)
{
  PD(PD_VAR,"acknowledge");
  OZ_Term val=u.bindings->val;
  primBind(ptr,val);
  oz_resume(u.bindings->thread);

  PendBinding *tmp=u.bindings->next;
  u.bindings->dispose();
  u.bindings=tmp;
  redirect(val);
}



void PerdioVar::redirect(OZ_Term val) {
  PD(PD_VAR,"redirect v:%s",toC(val));
  while (u.bindings) {

    RefsArray args = allocateRefsArray(2,NO);
    args[0]=val;
    args[1]=u.bindings->val;
    u.bindings->thread->pushCall(BI_Unify,args,2);
    PD(PD_VAR,"redirect pending unify =%s",toC(u.bindings->val));
    oz_resume(u.bindings->thread);

    PendBinding *tmp=u.bindings->next;
    u.bindings->dispose();
    u.bindings=tmp;
  }
}


void sendRedirect(ProxyList *pl,OZ_Term val, Site* ackSite, int OTI)
{
  while (pl) {
    Site* sd=pl->sd;
    ProxyList *tmp=pl->next;
    pl->dispose();
    pl = tmp;

    if (sd==ackSite) {
      sendAcknowledge(sd,OTI);
    } else {
      sendRedirect(sd,OTI,val);
    }
  }
}

void bindPerdioVar(PerdioVar *pv,TaggedRef *lPtr,TaggedRef v)
{
  if (pv->isManager()) {
    PD(PD_VAR,"bind manager o:%d v:%s",pv->getIndex(),toC(v));
    pv->primBind(lPtr,v);
    OT->getOwner(pv->getIndex())->mkRef();
    sendRedirect(pv->getProxies(),v,mySite,pv->getIndex());
  } else {
    PD(PD_VAR,"bind proxy b:%d v:%s",pv->getIndex(),toC(v));
    Assert(pv->isProxy());
    if (pv->hasVal()) {
      pv->pushVal(v); // save binding for ack message, ...
    } else {
      pv->setVal(v); // save binding for ack message, ...
      BorrowEntry *be=BT->getBorrow(pv->getIndex());
      sendSurrender(be,v);
    }
  }
}

void sendCreditBack(Site* sd,int OTI,Credit c)
{
  PD(CREDIT,"give back - %d",c);
  ByteStream *bs= bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_OWNER_CREDIT);
  marshallNumber(OTI,bs);
  marshallCredit(c,bs);
  bs->marshalEnd();
  PD(MSG_SENT,"OWNER_CREDIT s:%s o:%d c:%d",pSite(sd),OTI,c);
  reliableSendFail(sd,bs,TRUE,10);
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
/*              CELL PROTOCOL - BEGIN                                     */
/* ********************************************************************** */

inline int secBorrowInit(Site *site,int index,Credit c){
  NetAddress na=NetAddress(site,index);
  BorrowEntry* be=BT->find(&na);
  int bi;
  if(be==NULL){
    bi=BT->newBorrow(c,site,index);
    (BT->getBorrow(bi))->mkDummyTert();
    return bi;}
  be->addCredit(c);
  return borrowEntryToIndex(be);}

/* --------------------- at Manager ------------------------------------- */

inline void cellSendForward(BorrowEntry *be,Site *dS,int dI,int ctr,
                            Site *mS,int mI){
  PD(MSG_SENT,"MSG_SENT:CELL_FORWARD to s:%s,o:%d ctr:%d msg s:%s o:%d",
     pSite(dS),dI,ctr,pSite(mS),mI);

  ByteStream* bs=bufferManager->getByteStream();  // marshal
  bs->marshalBegin();
  bs->put(M_CELL_FORWARD);
  marshallNumber(dI,bs);
  marshallNumber(ctr,bs);
  marshallSite(mS,bs);
  marshallNumber(mI,bs);
  bs->marshalEnd();

  if(be->getOneCredit()){                          // send
    reliableSendFail(dS,bs,FALSE,15);
    return;}
  PendEntry *pe=pendEntryManager->newPendEntry(bs,dS);   // delayed send
  be->inDebtMain(pe);
  return;}

inline void cellSendRemoteRead(BorrowEntry *be,Site *cs,int si,TaggedRef val){
  PD(MSG_SENT,"MSG_SENT:CELL_REMOTEREAD to s:%s o:%d",pSite(cs),si);

  ByteStream* bs=bufferManager->getByteStream();   // marshal
  bs->marshalBegin();
  bs->put(M_CELL_REMOTEREAD);
  marshallNumber(si,bs);
  domarshallTerm(cs,val,bs);
  bs->marshalEnd();

  if(be->getOneCredit()){                             //send
    reliableSendFail(cs,bs,FALSE,15);
    return;}
  PendEntry *pe=pendEntryManager->newPendEntry(bs,cs); // delayed send
  be->inDebtMain(pe);
  return;}

/* --------------------- at Manager  MAIN ------------------------------------- */

void cellReceiveGet(OwnerEntry* oe,Site* rsite,int OTI){
  PD(MSG_RECEIVED,"MSG_RECEIVED:CELL_GET from s:%s o:%d",pSite(rsite),OTI);

  CellManager* cm=(CellManager*) oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);

  int current=cm->getCurrent();
  int bi=secBorrowInit(rsite,OTI,CELL_CREDIT_SIZE);
  cm->setCurrent(bi);

  if(current==NO_INDEX){                             // shortcut
    PD(CELL,"CELL - shortcut in cellReceiveGet");
    cellReceiveForward(oe,0,rsite,OTI);
    return;}

  BorrowEntry* be=BT->getBorrow(current);
  NetAddress* na=be->getNetAddress();
  cellSendForward(be,na->site,na->index,cm->getAndInitCtr(),rsite,OTI);
  BT->freeSecBorrow(current);
}

void cellReceiveRead(OwnerEntry* oe,TaggedRef val){
  PD(MSG_RECEIVED,"MSG_RECEIVED:CELL_READ");
  CellManager *cm=(CellManager*) oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);

  int current=cm->getCurrent();
  if(current==NO_INDEX){            // shortcut
    cellReceiveRemoteRead(oe,val);
    PD(CELL,"CELL- shortcut cellReceiveRead");
    return;}
  cm->incCtr();
  BorrowEntry *be=borrowTable->getBorrow(current);
  NetAddress *na=be->getNetAddress();
  cellSendRemoteRead(be,na->site,na->index,val);
}

void cellReceiveDump(OwnerEntry *oe,Site *rsite,int OTI){
  PD(MSG_RECEIVED,"MSG_RECEIVED:CELL_DUMP");
  CellManager *cm=(CellManager*) oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);
  int current=cm->getCurrent();
  Assert(current!=NO_INDEX);

  NetAddress *na=borrowTable->getBorrow(current)->getNetAddress();
  if((na->site!=rsite)|| (na->index!=OTI)){
    PD(WEIRD,"WEIRD- CELL dump out of synch");
    sendCreditBack(rsite,OTI,1);
    return;}

  CellFrame* cf=(CellFrame *)cm;
  if(cf->getState()!=Cell_Invalid){
    PD(WEIRD,"WEIRD- CELL dump not needed");
    sendCreditBack(rsite,OTI,1);
    return;}
  Assert(cf->getState()==Cell_Invalid);
  TaggedRef tr=makeTaggedRef(newTaggedUVar(am.currentBoard));
  cellDoExchange((Tertiary *)cf,tr,tr,NULL);
  return;
}

/* --------------------- at Frame ------------------------------------- */

inline void cellSendContents(TaggedRef tr,Site* rsite,int OTI){
  PD(MSG_SENT,"CELL_CONTENTS to s:%s,o:%d from s:%d",pSite(rsite),
     OTI,pSite(mySite));

  ByteStream *bs=bufferManager->getByteStream();    // marshal
  bs->marshalBegin();
  bs->put(M_CELL_CONTENTS);
  marshallNumber(OTI,bs);
  domarshallTerm(rsite,tr,bs);
  bs->marshalEnd();

  reliableSendFail(rsite,bs,FALSE,16);
}

inline void placeNext(CellFrame *cf,Site *rsite,int OTI){
  int bi=secBorrowInit(rsite,OTI,1);
  cf->setNext(bi);}

/* --------------------- at Frame ------------------------------------- */

void cellReceiveForward(OwnerEntry *oe,int ctr,Site *rsite,int OTI){
  PD(MSG_RECEIVED,"CELL_FORWARD to ctr:%d s:%s o:%d",
     ctr,pSite(rsite),OTI);
  CellFrame *cf=(CellFrame*) oe->getTertiary();
  Assert((cf->getTertType()==Te_Frame) || (cf->getTertType()==Te_Manager));
  Assert(cf->getType()==Co_Cell);
  cf->setState(cf->getState() & (~Cell_Dump_Asked));
  if(cf->getState() & Cell_Requested){
    Assert(!(cf->getState() & Cell_Next));
    cf->setState(Cell_Requested | Cell_Next);
    placeNext(cf,rsite,OTI);
    return;}
  Assert(cf->getState() & Cell_Valid);
  cf->decCtr(ctr);
  if(cf->getCtr()!=0){
    placeNext(cf,rsite,OTI);
    cf->setState(Cell_Valid | Cell_Next);
    return;}
  TaggedRef tr=cf->getContents();
  cellSendContents(tr,rsite,OTI);
  cf->setState(Cell_Invalid);}

void cellReceiveRemoteRead(OwnerEntry *oe,TaggedRef val){
  CellFrame *cf=(CellFrame*) oe->getTertiary();
  PD(MSG_RECEIVED,"CELL_REMOTEREAD o:%d",cf->getOwnerIndex());
  Assert(cf->getTertType()==Te_Frame);
  Assert(cf->getType()==Co_Cell);
  cf->incCtr();
  Assert(cf->getState() & (Cell_Valid | Cell_Requested));
  SiteUnify(val,cf->getContents());
  return;}

void cellReceiveContents(OwnerEntry *oe,TaggedRef val){
  CellFrame *cf=(CellFrame*)oe->getTertiary();
  Assert(cf->getType()==Co_Cell);
  PD(MSG_RECEIVED,"CELL_CONTENTS o:%d",cf->getOwnerIndex());
  Assert((cf->getTertType()==Te_Frame) || (cf->getTertType()==Te_Manager));
  short  state=cf->getState();
  Assert(state & Cell_Requested);

  PendThread *pt=cf->getPending();
  PendThread *tmp;
  while(pt!=NULL){
    if(pt->thread!=NULL) {
      PD(CELL,"CELL thread activated");
      oz_resume(pt->thread);}
    tmp=pt;
    pt=pt->next;
    tmp->dispose();}
  DebugCode(cf->setPending(NULL));

  TaggedRef head=cf->getHead();
  if(state & Cell_Next){
    if(cf->getCtr()==0){
      int bI=cf->getNext();
      NetAddress *na=borrowTable->getBorrow(bI)->getNetAddress();
      cellSendContents(val,na->site,na->index);
      cf->setState(Cell_Invalid);
      BT->freeSecBorrow(bI);
      return;}
    else{
      cf->setState(Cell_Valid | Cell_Next);
      return;
    }}
  cf->setState(Cell_Valid);
  SiteUnify(head,val);
}

/* --------------------- initiate ------------------------------------- */

inline void cellSendGet(BorrowEntry *be,Site *manS,int manI,CellFrame *cf){
  int myI=cf->getOwnerIndex();
  OT->getOwner(myI)->getCellCredit();
  ByteStream *bs=bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_CELL_GET);
  marshallNumber(manI,bs);
  marshallMySite(bs);
  marshallNumber(myI,bs);
  bs->marshalEnd();

  if(be->getOneCredit()){
    PD(MSG_SENT,"CELL_GET to s:%s,o:%d,from o:%d",pSite(manS),manI,myI);
    reliableSendFail(manS,bs,FALSE,27);
    return;}
  PD(MSG_QUEUED,"CELL GET to s:%s,o:%d,from o:%d",pSite(manS),manI,myI);
  PendEntry *pe=pendEntryManager->newPendEntry(bs,manS);
  be->inDebtMain(pe);
  return;}

inline void cellSendRead(BorrowEntry *be,Site *manS,int manI,TaggedRef val){
  ByteStream* bs=bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_CELL_READ);
  marshallNumber(manI,bs);
  domarshallTerm(manS,val,bs);
  bs->marshalEnd();
  if(be->getOneCredit()){
    PD(MSG_SENT,"CELL_READ to site:%s,o:%d",pSite(manS),manI);
    reliableSendFail(manS,bs,FALSE,28);
    return;}
  PD(MSG_QUEUED,"CELL_READ to site:%s,o:%d",pSite(manS),manI);
  PendEntry *pe=pendEntryManager->newPendEntry(bs,manS);
  be->inDebtMain(pe);
  return;}

void cellSendDump(CellFrame *cf){
  Assert(cf->getState()==Cell_Valid);
  BorrowEntry* be=BT->getBorrow(cf->getManagerIndex());
  NetAddress *na=be->getNetAddress();
  ByteStream *bs=bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_CELL_DUMP);
  marshallNumber(na->index,bs);
  marshallMySite(bs);
  marshallNumber(cf->getOwnerIndex(),bs);
  bs->marshalEnd();
  cf->setState(Cell_Valid | Cell_Dump_Asked);
  OT->getOwner(cf->getOwnerIndex())->getOneCredit();
  if(be->getOneCredit()){
    PD(MSG_SENT,"CELL_DUMP to site:%s,o:%d",pSite(na->site),na->index);
    reliableSendFail(na->site,bs,FALSE,22);
    return;}
  PD(MSG_QUEUED,"CELL_DUMP to site:%s,o:%d",pSite(na->site),na->index);
  PendEntry *pe=pendEntryManager->newPendEntry(bs,na->site);
  be->inDebtMain(pe);
  return;
}

/* --------------------- builtin ------------------------------------- */

inline void e_valid(CellFrame *cf,TaggedRef old,TaggedRef nw,Thread* th){
  Assert(cf->getState() & Cell_Valid);
  TaggedRef c=cf->getContents();
  cf->setContents(nw);
  PD(CELL,"CELL: exchange on VALID o:%d",cf->getOwnerIndex());
  SiteUnify(c,old);}

inline void e_invalid(CellFrame *cf,TaggedRef old,TaggedRef nw,Thread* th){
  cf->setState(Cell_Requested);
  cf->setHead(old);
  cf->setContents(nw);
  oz_stop(th);
  PendThread* pt=new PendThread(th,(PendThread*)NULL);
  PD(CELL,"CELL: exchange on INVALID o:%d",cf->getOwnerIndex());
  cf->setPending(pt);}

inline void e_req(CellFrame *cf,TaggedRef old,TaggedRef nw,Thread* th){
  TaggedRef tr=cf->getContents();
  cf->setContents(nw);
  oz_stop(th);
  PendThread* pt=new PendThread(th,cf->getPending());
  PD(CELL,"CELL: exchange on REQUESTED o:%d",cf->getOwnerIndex());
  cf->setPending(pt);
  SiteUnify(tr,old);}

void cellDoExchange(Tertiary *c,TaggedRef old,TaggedRef nw,Thread* th){
  TertType tt=c->getTertType();

  if(tt==Te_Proxy){              // convert Proxy to Frame
    CellProxy *cp=(CellProxy *)c;
    OwnerEntry *oe;
    int index=ownerTable->newOwner(oe);
    oe->mkTertiary(c);
    PD(CELL,"CELL: convertToFrame o:%d",index);
    cp->convertToFrame(index);
    tt=Te_Frame;}

  if(tt==Te_Manager){             // Manager

    CellManager *cm=(CellManager*)c;
    CellFrame *cf=(CellFrame *)c;
    if(cm->isOwnCurrent()){
      if(cf->getState() & Cell_Valid){
        PD(CELL,"Exchange: manager own current- valid");
        Assert(cf->getState() & Cell_Valid);
        e_valid(cf,old,nw,th);
        return;}
      Assert(cf->getState() & Cell_Requested);
      PD(CELL,"Exchange: manager own current -requested");
      e_req(cf,old,nw,th);
      return;}

    short state=cf->getState();
    Assert(!(state & Cell_Valid));
    if(state & Cell_Requested){
      PD(CELL,"Exchange: manager has requested");
      e_req(cf,old,nw,th);
      return;}
    PD(CELL,"Exchange: manager invalid");
    Assert(state == Cell_Invalid);      // shortcut
    int current=cm->getCurrent();
    cm->setOwnCurrent();
    e_invalid(cf,old,nw,th);
    BorrowEntry* be=BT->getBorrow(current);
    NetAddress *na=be->getNetAddress();
    int myI=cm->getOwnerIndex();
    cellSendForward(be,na->site,na->index,cm->getAndInitCtr(),mySite,myI);
    return;}

  CellFrame *cf=(CellFrame*)c;
  Assert(cf->getTertType()==Te_Frame);
  short state=cf->getState();
  if(state & Cell_Valid){
    e_valid(cf,old,nw,th);
    return;}
  if(state & Cell_Requested){
    e_req(cf,old,nw,th);
    return;}
  Assert(state==Cell_Invalid);
  e_invalid(cf,old,nw,th);
  BorrowEntry *be=borrowTable->getBorrow(cf->getManagerIndex());
  NetAddress *na=be->getNetAddress();
  cellSendGet(be,na->site,na->index,cf);
  return;
}

inline void a_valid(CellFrame *cf,TaggedRef val){
  PD(CELL,"CELL: access on VALID o:%d",cf->getOwnerIndex());
  SiteUnify(val,cf->getContents());}

inline void a_req(CellFrame *cf,TaggedRef val){
  PD(CELL,"CELL: access on REQUESTED o:%d",cf->getOwnerIndex());
  SiteUnify(val,cf->getContents());}


void cellDoAccess(Tertiary *c,TaggedRef val){
  TertType tt=c->getTertType();

  if(tt==Te_Proxy){
    PD(CELL,"CELL: access on PROXY index");
    CellProxy *cp=(CellProxy*)c;
    BorrowEntry *be=borrowTable->getBorrow(cp->getManagerIndex());
    NetAddress *na=be->getNetAddress();
    cellSendRead(be,na->site,na->index,val);
    return;}

  CellFrame *cf=(CellFrame *)c;
  if(tt==Te_Manager){
    CellManager *cm=(CellManager*)c;
    if(cm->isOwnCurrent()){
      SiteUnify(cf->getContents(),val);
      return;}
    short state=cf->getState();
    if(state & Cell_Valid){
      a_valid(cf,val);return;}
    if(state & Cell_Requested){
      a_req(cf,val);return;}
    Assert(state == Cell_Invalid);
    BorrowEntry *be=BT->getBorrow(cm->getCurrent());
    NetAddress *na=be->getNetAddress();
    cellSendRemoteRead(be,na->site,na->index,val);
    return;}

  Assert(tt=Te_Frame);
  short state=cf->getState();
  if(state & Cell_Valid){a_valid(cf,val);return;}
  if(state & Cell_Requested){a_req(cf,val);return;}
  Assert(state == Cell_Invalid);

  PD(CELL,"CELL: access on INVALID o:%d",cf->getOwnerIndex());
  BorrowEntry *be=BT->getBorrow(cf->getManagerIndex());
  NetAddress *na=be->getNetAddress();
  cellSendRead(be,na->site,na->index,val);
  return;
}

/*              CELL PROTOCOL    - END                                    */
/* ********************************************************************** */



/* ********************************************************************** */
/*              BUILTINS themselves                                       */
/* ********************************************************************** */

#define CHECK_INIT                                              \
  if (!ipIsInit()) {                                            \
    return oz_raise(E_ERROR,OZ_atom("ip"),"uninitialized",0);   \
                                                                  }


OZ_C_proc_begin(BIStartSite,2)
{
  OZ_declareIntArg(0,vport);
  OZ_declareArg(1,stream);
  PD(USER,"startSite called vp:%d",vport);
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
  PD(USER,"startSite succeeded");
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
    InterfaceCode ret=ipInit(port,siteReceive);                           \
    if(ret==NET_RAN_OUT_OF_TRIES){                                        \
      return OZ_raiseC("startSite",1,OZ_string("ran out of tries"));}     \
    PD(USER,"startSite succeeded");                                       \
  }

OZ_C_proc_begin(BIstartServer,2)
{
  OZ_declareIntArg(0,port);
  OZ_declareNonvarArg(1,prt);

  prt=deref(prt);
  if (!isPort(prt)) {
    oz_typeError(0,"Port");
  }

  PD(USER,"startServer called p:%d",port);

  if (ipIsInit()) {
    return OZ_raiseC("startServer",1,OZ_string("already running"));
  }

  INIT_IP(port);

  ozport = prt;
  Tertiary *tert=tagged2Port(prt);
  tert->setTertType(Te_Manager);
  ownerTable->newOZPort(tert);

  return PROCEED;
}
OZ_C_proc_end


// builtin assumption: connect to a port with OTI=0 !!!
inline OZ_Term connect_site_aux(Site * sd)
{
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

  PD(USER,"connectSite started vp:%d ho:%s",vport,host);
  if(ozport==0){
    return OZ_raiseC("connectSite",1,OZ_string("startSite first"));}

  Site * sd;
  InterfaceCode ret=connectSiteV(host,vport,sd,FALSE);
  if(ret==NET_OK){
    PD(USER,"connectSite success");
    OZ_Term x=connect_site_aux(sd);
    return OZ_unify(out,x);} /* TODO */
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
    PD(USER,"connectSite success");
    OZ_Term x=connect_site_aux(sd);
    return OZ_unify(out,x);} /* TODO */
  return OZ_raiseC("connectSite",2,OZ_int(ret),
                   OZ_string(ret==NET_RAN_OUT_OF_TRIES?"ran out of tries":"unknown"));
}
OZ_C_proc_end

OZ_C_proc_begin(BIConnectSiteWait,3){
  //  CHECK_INIT;
  OZ_declareVirtualStringArg(0,host);
  OZ_declareIntArg(1,vport);
  OZ_declareArg(2,out);
  PD(USER,"connectSiteWait started vp:%d h:%s",vport,host);
  if(ozport==0){
    return OZ_raiseC("connectSiteWait",1,OZ_string("startSite first"));}

  Site * sd;
  InterfaceCode ret=connectSiteV(host,vport,sd,TRUE);
  if(ret==NET_OK){
    PD(USER,"connectSiteWait success");
    OZ_Term x=connect_site_aux(sd);
    return OZ_unify(out,x);} /* TODO */
  if(ret==NET_RAN_OUT_OF_TRIES){
    return OZ_raiseC("connectSiteWait",1,OZ_string("ran out of tries"));}
  return PROCEED;
}
OZ_C_proc_end

#ifdef DEBUG_PERDIO
OZ_C_proc_begin(BIdvset,2)
{
  OZ_declareIntArg(0,what);
  OZ_declareIntArg(1,val);

  if (val) {
    DV->set(what);
  } else {
    DV->unset(what);
  }
  return PROCEED;
}
OZ_C_proc_end
#endif


OZ_C_proc_begin(BImarshall,2)
{
  // CHECK_INIT;
  OZ_declareArg(0,in);
  OZ_declareArg(1,out);

  INIT_IP(0);

  ByteStream *bs = bufferManager->getByteStream();
  bs->marshalBegin();

  domarshallTerm(0,in,bs);

  bs->marshalEnd();

  bs->beginWrite();
  bs->incPosAfterWrite(tcpHeaderSize);
  OZ_Term l=nil();

  int total=bs->calcTotLen();
  while (total){
    Assert(total>0);
    int len=bs->getWriteLen();
    BYTE* pos=bs->getWritePos();
    total -=len;
    while (len) {
      l=cons(OZ_int(*pos),l);
      pos++;
      len--;
    }
    bs->sentFirst();
  }
  bs->writeCheck();

  bufferManager->freeByteStream(bs);

  return OZ_unify(out,reverseC(l));  /* TODO? SiteUnify*/
}
OZ_C_proc_end

OZ_C_proc_begin(BIunmarshall,2)
{
  OZ_declareArg(0,in);
  OZ_declareArg(1,out);

  INIT_IP(0);

  OZ_Term var;
  if (!OZ_isString(in,&var)) {
    if (var==0) {
       return OZ_typeError(1,"String");
     } else {
       OZ_suspendOn(var);
     }
  }

  ByteStream *bs=bufferManager->getByteStream();
  bs->getSingle();

  int max;
  BYTE *pos=bs->initForRead(max);
  int len=0;

  OZ_Term tmp = deref(in);
  for (; isCons(tmp); tmp=deref(tail(tmp))) {
    OZ_Term hh = deref(head(tmp));
    Assert(isSmallInt(hh));
    int i = smallIntValue(hh);
    Assert(i>=0 && i <= 255);
    *pos++ = (unsigned char) i;
    max--;
    len++;
    if (max<=0) {
      bs->afterRead(len);
      pos=bs->initForRead(max);
      len=0;
    }
  }
  Assert(isNil(tmp));

  bs->afterRead(len);
  bs->beforeInterpret(0);

  bs->unmarshalBegin();

  OZ_Term v;
  unmarshallTerm(bs,&v);

  bs->unmarshalEnd();

  bs->afterInterpret();

  bufferManager->freeByteStream(bs);

  return OZ_unify(out,v);  /* TODO SiteUnify?*/
}
OZ_C_proc_end

BIspec perdioSpec[] = {
  {"startSite",      2, BIStartSite, 0},
  {"connectSite",    3, BIConnectSite, 0},
  {"connectSiteWait",3, BIConnectSiteWait, 0},

  {"startServer",    2, BIstartServer, 0},
  {"startClient",    3, BIstartClient, 0},

  {"marshall",    2, BImarshall, 0},
  {"unmarshall",  2, BIunmarshall, 0},

#ifdef DEBUG_PERDIO
  {"dvset",    2, BIdvset, 0},
#endif
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

  Assert(sizeof(CellProxy)==sizeof(CellFrame));
  Assert(sizeof(CellManager)==sizeof(CellLocal));
  Assert(sizeof(PortManager)==sizeof(PortLocal));
}


#endif
