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

/*
 * clusters
 *   implement URLs
 *     a URL is an atom
 *     make a quick and dirty implementation of file: and http:
 *       load into ByteStream
 *   save bytestream into file
 *
 *   add URL to gname
 *
 *   proxies for URLs:
 *     have gname, if loaded from a cluster or send from other site
 *     have no gname, if linked with X={Link URL} (should this be impl?)
 *     unification of a URL proxies must load URL data.
 *     implemenation use Ralfs PerdioVar extension
 *
 *   creating a URL {SAVE X URL} ***1
 *     check if server supports upload (PUT)
 *     save also resources
 *       must set manager credit to infinity
 *       must write infinite credit to cluster
 *       must set borrow credit to infinity
 *     save proxies for already tagged gnames
 *     mark all saved procedures and chunks with URL
 *
 *   procedures and chunks must be eager if they are not tagged with a URL
 *     with a switch to turn it on/off (#ifdef LAZY)
 * BUGS:
 *   Saving proxies for resources must suspend until manager is made persistent
 *
 */

#ifdef PERDIO

#ifdef INTERFACE
#pragma implementation "perdio.hh"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "runtime.hh"
#include "ip.hh"
#include "codearea.hh"
#include "indexing.hh"

#include "perdio_debug.hh"
#include "perdio_debug.cc"
#include "genvar.hh"
#include "perdiovar.hh"
#include "gc.hh"
#include "dictionary.hh"

typedef long Credit;  /* TODO: full credit,long credit? */

class BorrowTable;
class OwnerTable;
class ByteStream;
class DebtRec;
DebtRec* debtRec;

TaggedRef currentURL;


void marshallTerm(Site* sd,OZ_Term t, ByteStream *bs);
int unmarshallWithDest(BYTE *buf, int len, OZ_Term *t);
void domarshallTerm(Site* sd,OZ_Term t, ByteStream *bs);
void unmarshallTerm(ByteStream*,OZ_Term*);
void marshallCode(Site*,ProgramCounter, ByteStream *);
void marshallObject(Site *sd, Object *o, ByteStream *bs);
void marshallClass(Site *sd, Object *o, ByteStream *bs);
OZ_Term unmarshallTerm(ByteStream *bs);
inline void marshallNumber(unsigned int,ByteStream *);
inline void marshallMySite(ByteStream* );
inline void marshallCredit(Credit,ByteStream *);

void sendSurrender(BorrowEntry *be,OZ_Term val);
void sendRedirect(Site* sd,int OTI,TaggedRef val);
void sendAcknowledge(Site* sd,int OTI);
void sendRedirect(ProxyList *pl,OZ_Term val, Site* ackSite,int OTI);
void bindPerdioVar(PerdioVar *pv,TaggedRef *lPtr,TaggedRef v);
void sendCreditBack(Site* sd,int OTI,Credit c);
inline void reliableSendFail(Site*,ByteStream *,Bool,int);

void cellReceiveContents(OwnerEntry*,TaggedRef);
void cellReceiveGet(OwnerEntry*,Site*,int);
void cellReceiveRead(OwnerEntry*,TaggedRef);
void cellReceiveRemoteRead(OwnerEntry*,TaggedRef);
void cellReceiveForward(OwnerEntry*,int,Site*,int);
void cellReceiveDump(OwnerEntry*,Site *,int);
void cellSendRemoteRead(BorrowEntry*,int,Site*,TaggedRef);
void cellSendDump(CellFrame *);

BorrowTable *borrowTable;
OwnerTable *ownerTable;

#define OT ownerTable
#define BT borrowTable

extern TaggedRef BI_Unify;
extern TaggedRef BI_Show;

void SiteUnify(TaggedRef val1,TaggedRef val2)
{
  TaggedRef aux1 = val1; DEREF(aux1,_1,_2);
  TaggedRef aux2 = val2; DEREF(aux2,_3,_4);

  if (isUVar(aux1) || isUVar(aux2)) {
    // cannot fail --> do it in current thread
    OZ_unify(val1,val2);
    return;
  }
  Thread *th=am.mkRunnableThread(DEFAULT_PRIORITY,am.rootBoard);
#ifdef PERDIO_DEBUG
  PD((SITE_OP,"SITE_OP: site unify called"));
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
  am.scheduleThread(th);
}

/*
 * Message formats
 */
enum MessageType {
  M_PORT_SEND,
  M_REMOTE_SEND,        // OTI STRING DIF (implicit 1 credit)
  M_ASK_FOR_CREDIT,     // OTI SITE (implicit 1 credit)
  M_OWNER_CREDIT,       // OTI CREDIT
  M_BORROW_CREDIT,      // NA  CREDIT
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
  DIF_SMALLINT,           // int
  DIF_BIGINT,             // string
  DIF_FLOAT,            // string
  DIF_ATOM,             // string
  DIF_NAME,             // ???
  DIF_UNIQUENAME,               // ???
  DIF_RECORD,           //
  DIF_TUPLE,
  DIF_LIST,
  DIF_REF,
  DIF_OWNER,
  DIF_PORT,             // NA CREDIT
  DIF_LOCK,             // NA CREDIT
  DIF_CELL,               // NA CREDIT
  DIF_VAR,
  DIF_BUILTIN,
  DIF_DICT,
  DIF_OBJECT,
  DIF_THREAD,           // NA CREDIT
  DIF_SPACE,            // NA CREDIT
  DIF_CHUNK,        // NA NAME value
  DIF_PROC,             // NA NAME ARITY globals code
  DIF_CLASS,        // NA NAME obj class
  DIF_URL,              // gname url
} MarshallTag;


void sendMessage(int bi, MessageType);


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
  PO_TYPE type;
protected:
  union {
    TaggedRef ref;
    Tertiary *tert;
  } u;
public:
  ProtocolObject()            { DebugCode(type=(PO_TYPE)4711; u.ref=0x5a5a5a5a;)}
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
      PD((GC,"OT tertiary found"));
      Assert(!isDummyTertiary());
      u.tert=(Tertiary *)(u.tert->gcConstTermSpec()); /* ATTENTION */
    } else {
      Assert(isRef() || isVar());
      PD((GC,"OT var/ref"));
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
  PD((MARSHALL_BE,"marshal begin"));
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

ByteStream* BufferManager::getByteStreamMarshal(){
  ByteStream *bs=getByteStream();
  bs->marshalBegin();
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

#define BTRESIZE_CRITICAL

#ifdef BTRESIZE_CRITICAL
#define DEFAULT_OWNER_TABLE_SIZE   5000
#define DEFAULT_BORROW_TABLE_SIZE  5000
#else
#define DEFAULT_OWNER_TABLE_SIZE   100
#define DEFAULT_BORROW_TABLE_SIZE  100
#endif
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



class GName {
public:
  char gcMark;
  char gnameType;
  GNameSite site;
  FatInt id;
  TaggedRef url;

  Bool hasURL() { return url!=0; }
  TaggedRef getURL() { return url; }
  TaggedRef markURL(TaggedRef u) { Assert(!url); url = u; }

  Bool same(GName *other) {
    return site.same(other->site) && id.same(other->id);
  }
  GName() { gcMark = 0; url=0; }
  GName(ip_address ip, port_t port, time_t timestamp, GNameType gt)
  {
    gcMark = 0;
    url = 0;

    site.ip        = ip;
    site.port      = port;
    site.timestamp = timestamp;

    idCounter->inc();
    id = *idCounter;
    gnameType = (char) gt;
  }

  GNameType getGNameType() { return (GNameType) gnameType; }

  void setGCMark()   { gcMark = 1; }
  Bool getGCMark()   { return gcMark; }
  void resetGCMark() { gcMark = 0; }

};

class GNameTable: public GenHashTable{
  int hashFunc(GName *);
public:
  GNameTable():GenHashTable(GNAME_HASH_TABLE_DEFAULT_SIZE) {}
  void gnameAdd(GName *name, TaggedRef t);
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
void addGName(GName *name, TaggedRef t)
{
  gnameTable->gnameAdd(name,t);
}

void addGName(GName *name, PrTabEntry *pr)
{
  Assert(name->getGNameType()==GNT_CODE);
  addGName(name,ToInt32(pr));
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



GName *newGName(TaggedRef t, GNameType gt)
{
  ip_address ip;
  port_t port;
  time_t ts;

  getSiteFields(mySite,ip,port,ts);
  GName* ret = new GName(ip,port,ts,gt);
  gnameTable->gnameAdd(ret,t);
  return ret;
}

GName *newGName(PrTabEntry *pr)
{
  GName *ret = newGName(ToInt32(pr),GNT_CODE);
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

  void setCredit(Credit c) {
    u.credit=c;
  }

  void addToCredit(Credit c) {
    Assert(c!=INFINITE_CREDIT);
    Assert(!isPersistent());
    u.credit +=c;
  }

  void subFromCredit(Credit c) {
    Assert(c!=INFINITE_CREDIT);
    Assert(!isPersistent());
    u.credit -=c;
  }

  void makePersistent1() { setCredit(INFINITE_CREDIT); }

public:
  void print();
  Credit getCredit()    { Assert(!isFree());return u.credit;}
  Bool isPersistent()   { return u.credit==INFINITE_CREDIT; }
};

/* ********************************************************************** */

class OwnerEntry: public OB_Entry {
friend class OwnerTable;

private:
  Credit requestCredit(Credit req){
    if (isPersistent()) return INFINITE_CREDIT;

    Credit c=getCredit();
    if(c < 2*req) {
      makePersistent();
      return req;
    }
    subFromCredit(req);
    return req;
  }
public:
  void makePersistent() { makePersistent1(); }
  void returnCredit(Credit c) {
    if (isPersistent()) return;
    if (c==INFINITE_CREDIT) { makePersistent(); return; }
    addToCredit(c);
  }
  int hasFullCredit()     { return getCredit()==START_CREDIT_SIZE; }
  Credit getSendCredit()  { return requestCredit(OWNER_GIVE_CREDIT_SIZE); }
  Credit getOneCredit()   { return requestCredit(1); }
  // for redirect, acknowledge
  Credit giveMoreCredit() { return requestCredit(ASK_CREDIT_SIZE); }
  void getCellCredit()    { (void) requestCredit(CELL_CREDIT_SIZE);}
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

  OwnerEntry *getOwner(int i)  { Assert(i<size); return &array[i];}

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
      PD((PD_VAR,"localize var o:%d",OTI));
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
  PD((TABLE,"TABLE:owner compactify enter: size:%d no_used:%d",
               size,no_used));
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
    PD((TABLE,"TABLE:owner compactify free slots: new%d",newsize));
    OwnerEntry *oldarray=array;
    array = (OwnerEntry*) realloc(array,newsize*sizeof(OwnerEntry));
    Assert(array!=NULL);
    if(array!=NULL){
      size=newsize;
      init(first_free,size);
      return;}
    array=oldarray;}
  init(first_free,size);
  PD((TABLE,"TABLE:owner compactify no realloc"));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

void OwnerTable::resize(){
#ifdef BTRESIZE_CRITICAL
  warning("OwnerTable::resize: maybe incorrect");
#endif
  int newsize = ((int) (TABLE_EXPAND_FACTOR *size));
  PD((TABLE,"TABLE:resize owner old:%d no_used:%d new:%d",
                size,no_used,newsize));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  array = (OwnerEntry*) realloc(array,newsize*sizeof(OwnerEntry));
  if(array==NULL){
    error("Memory allocation: Owner Table growth not possible");}
  init(size, newsize);
  size=newsize;
  PD((TABLE2,"TABLE:resize owner complete"));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

int OwnerTable::newOwner(OwnerEntry *&oe){
  if(nextfree == END_FREE) resize();
  int index = nextfree;
  nextfree = array[index].u.nextfree;
  oe = (OwnerEntry *)&(array[index]);
  oe->setCredit(START_CREDIT_SIZE);
  PD((TABLE,"owner insert: o:%d",index));
  no_used++;
  return index;}


void OwnerTable::newOZPort(Tertiary* tert){
  Assert(nextfree==0);
  nextfree = array[0].u.nextfree;
  OwnerEntry* oe= (OwnerEntry *)&(array[0]);
  oe->mkTertiary(tert);
  tert->setIndex(0);
  oe->makePersistent();}


void OwnerTable::freeOwnerEntry(int i){
  array[i].setFree();
  array[i].u.nextfree=nextfree;
  nextfree=i;
  no_used--;
  PD((TABLE,"owner delete o:%d",i));
  return;}

#ifdef DEBUG_PERDIO
void OwnerTable::print(){
  printf("***********************************************\n");
  printf("********* OWNER TABLE *************************\n");
  printf("***********************************************\n");
  printf("Size:%d No_used:%d \n",size,no_used);
  int i;
  for(i=0;i<size;i++){
    printf("<%d> ",i);
    if(!(array[i].isFree())){
      printf("OWNER\n");
      getOwner(i)->print();
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
    if (!isPersistent()) {
      if (c==INFINITE_CREDIT) {
        makePersistent();
      } else {
        addToCredit(1); // at least one credit to request more
        addCredit(c-1); // rest may be use for pending messages
      }
    }
  }

#ifdef DEBUG_PERDIO
  void DEBUG_pendLink(PendLink *pl){
    while(pl!=NULL) {pl=pl->next;}
    return;}
#endif

  void makePersistent();
  void addCredit(Credit cin){
    Assert(cin!=INFINITE_CREDIT);
    PD((CREDIT,"borrow add s:%s o:%d add:%d to:%d",
       pSite(getNetAddress()->site),
       getNetAddress()->index,
       cin,
       getCredit()));
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
    Assert(!isPersistent());
    Credit c=getCredit();
    if(c==1) {
      PD((CREDIT,"getOneAskCredit failed"));
      return FALSE;
    }
    PD((CREDIT,"getOneAskCredit OK"));
    subFromCredit(1);
    return TRUE;}

  Credit getOneCredit() {
    if (isPersistent()) return INFINITE_CREDIT;
    Credit c=getCredit();
    Assert(c>0);
    if(c <= MIN_BORROW_CREDIT_SIZE) {
      PD((CREDIT,"getOneCredit failed"));
      return 0;}
    PD((CREDIT,"getOneCredit OK"));
    subFromCredit(1);
    return 1; }

  Bool getSmallCredit(Credit &c){
    if (isPersistent()) {
      c=INFINITE_CREDIT;
      return TRUE;
    }
    Credit cur=getCredit();
    if(cur < 2 * MIN_BORROW_CREDIT_SIZE) return FALSE;
    if(cur >  2 * BORROW_GIVE_CREDIT_SIZE) c=BORROW_GIVE_CREDIT_SIZE;
    else{
      if(cur >= 2 * MIN_BORROW_CREDIT_SIZE) c=MIN_BORROW_CREDIT_SIZE;}
    PD((CREDIT,"give small credit c:%d",c));
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
    PD((PENDLINK,"new- none so far"));
    pendLink=pl;
    return;}
  PD((PENDLINK,"new- others around far"));
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
    PD((PENDLINK,"one entry = 0"));
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
      PD((PENDLINK,"cannot send due to FIFO"));
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

  PD((PENDLINK,"entering debt handler"));
  while(TRUE){
    if(cur==NULL) {
      *base=cur;
      return;}
    if(cur->getDebt()!=0){
      PD((PENDLINK,"ran into non-zero debt"));
      *base=cur;
      moreCredit();
      return;}
    pe=cur->getPend();
    if((pe->getrefCount()==0)){
      if(fifoCanSend(cur,pe,flag)){
        PD((DELAYED_MSG_SENT,"pendLinkHandle"));
        pe->send();
        msgsent=TRUE;
        pendEntryManager->deletePendEntry(pe);}
      else{
        PD((PENDLINK,"ran into fifo cannot send"));
        msgsent=FALSE;}}
    else{
      PD((PENDLINK,"ran into non-zero ref ct"));
      msgsent=FALSE;}
    if(cur->isTagged() && ((!flag) || (!msgsent))){
      PD((PENDLINK,"fifo restriction cannot remove"));
      flag=FALSE;
      base= &(cur->next);
      cur=cur->next;}
    else{
      aux=cur->next;
      PD((PENDLINK,"removal"));
      pendLinkManager->deletePendLink(cur);
      cur=aux;}}
}


void BorrowEntry::makePersistent(){
  while (pendLink!=NULL){
    pendLinkCredit(MAX_BORROW_CREDIT_SIZE);
    pendLinkHandle();
    PERDIO_DEBUG_DO(DEBUG_pendLink(pendLink));
  }

  makePersistent1();
}


void BorrowEntry::moreCredit(){
  if(!getOneAskCredit()) {
    // already required moreCredit!
    return;
  }
  ByteStream *bs= bufferManager->getByteStreamMarshal();
  bs->put(M_ASK_FOR_CREDIT);
  NetAddress *na = getNetAddress();
  Site* site = na->site;
  int index = na->index;
  marshallNumber(index,bs);
  marshallMySite(bs);
  bs->marshalEnd();
  PD((MSG_SENT,"ASK_FOR_CREDIT s:%s o:%d",pSite(site),index));
  reliableSendFail(site,bs,TRUE,2);
}

void sendRegister(BorrowEntry *be) {
  ByteStream *bs= bufferManager->getByteStreamMarshal();
  bs->put(M_REGISTER);
  NetAddress *na = be->getNetAddress();
  Site* site = na->site;
  int index = na->index;
  marshallNumber(index,bs);
  marshallMySite(bs);
  bs->marshalEnd();
  PD((MSG_SENT,"REGISTER s:%s o:%d",pSite(site),index));

  if (be->getOneCredit()) {  /* priority */
    reliableSendFail(site,bs,FALSE,30);
    return;
  }

  PD((DEBT_MAIN,"register"));
  PendEntry *pe= pendEntryManager->newPendEntry(bs,site);
  be->inDebtMain(pe);
}


void BorrowEntry::giveBackCredit(Credit c){
  Assert(!isPersistent());
  NetAddress *na = getNetAddress();
  Site* site = na->site;
  int index = na->index;
  sendCreditBack(site,index,c);
}

void BorrowEntry::freeBorrowEntry(){
  if (!isPersistent()) {
    giveBackCredit(getCredit());
  }
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
      PD((LOOKUP,"borrow not found"));
      return 0;
    } else {
      PD((LOOKUP,"borrow found b:%d",i));
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
  PD((TABLE,"compactify borrow old:%d no_used:%d new:%d",
                size,no_used,newsize));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  BorrowEntry *oldarray=array;
  array = (BorrowEntry*) malloc(newsize*sizeof(BorrowEntry));
  if(array==NULL){
    PD((TABLE,"compactify borrow NOT POSSIBLE"));
    array=oldarray;
    return;}
  int oldsize=size;
  size=newsize;
  copyBorrowTable(oldarray,oldsize);
  PD((TABLE,"compactify borrow complete"));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

void BorrowTable::resize()
{
#ifdef BTRESIZE_CRITICAL
  warning("BorrowTable::resize: maybe incorrect");
#endif
  Assert(no_used==size);
  int newsize = int (TABLE_EXPAND_FACTOR*size);
  PD((TABLE,"resize borrow old:%d no_used:%d new:%d", size,no_used,newsize));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  BorrowEntry *oldarray=array;
  array = (BorrowEntry*) malloc(newsize*sizeof(BorrowEntry));
  if(array==NULL){
    error("Memory allocation: Borrow Table growth not possible");}
  int oldsize=size;
  size=newsize;
  copyBorrowTable(oldarray,oldsize);
  PD((TABLE,"resize borrow complete"));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

int BorrowTable::newBorrow(Credit c,Site * sd,int off){
  if(nextfree == END_FREE) resize();
  int index=nextfree;
  nextfree= array[index].u.nextfree;
  BorrowEntry* oe = &(array[index]);
  oe->initBorrow(c,sd,off);
  PD((HASH2,"<SPECIAL>:net=%x borrow=%x owner=%x hash=%x",
                oe->getNetAddress(),array,ownerTable->array,
                hshtbl->table));
  hshtbl->add(oe->getNetAddress(),index);
  no_used++;
  PD((TABLE,"borrow insert: b:%d",index));
  return index;}

void BorrowTable::maybeFreeBorrowEntry(int index){
  BorrowEntry *b = &(array[index]);
  if(b->isPending()) return; /* cannot remove as msgs pending on credit */
  b->freeBorrowEntry();
  hshtbl->sub(getBorrow(index)->getNetAddress());
  b->setFree();
  b->u.nextfree=nextfree;
  nextfree=index;
  no_used--;
  PD((TABLE,"borrow delete: b:%d",index));
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
  PD((HASH,"find Place hvalue=%d, net%d:%d",hvalue,
               na->site,na->index));
  ghn=htFindFirst(hvalue);
  NetAddress *na2;
  while(ghn!=NULL){
    na2=GenHashNode2NetAddr(ghn);
    if(na->same(na2)){
      PD((HASH,"compare success hvalue=%d bk=%x net%d:%d",
                    ghn->getKey(),ghn->getBaseKey(),na2->site,na2->index));
      return TRUE;}
    PD((HASH,"compare fail hvalue=%d bk=%x net%d:%d",
                  ghn->getKey(),ghn->getBaseKey(),na2->site,na2->index));
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
    PD((HASH,"borrow index b:%d",bindex));
    return bindex;}
  return -1;}

void NetHashTable::add(NetAddress *na,int bindex){
  int hvalue=hashFunc(na);
  GenHashNode *ghn;
  Assert(!findPlace(hvalue,na,ghn));
  PD((HASH,"adding hvalue=%d net=%d:%d bindex=%d",
               hvalue,na->site,na->index,bindex));
  GenHashBaseKey* ghn_bk;
  GenHashEntry* ghn_e;
  GenCast(na,NetAddress*,ghn_bk,GenHashBaseKey*);
  GenCast(bindex,int,ghn_e,GenHashEntry*);
  htAdd(hvalue,ghn_bk,ghn_e);}

void NetHashTable::sub(NetAddress *na){
  int hvalue=hashFunc(na);
  GenHashNode *ghn;
  findPlace(hvalue,na,ghn);
  PD((HASH,"deleting hvalue=%d net=%d:%d bindex=%d",
               hvalue,na->site,na->index));
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
  PD((GC,"GC-relocate borrow sec:%d",i));
  Assert(BT->getBorrow(i)->isDummyTertiary());
  (BT->getBorrow(i))->makeMark();}

void Tertiary::gcProxy(){
  int i=getIndex();
  BorrowEntry *be=BT->getBorrow(i);
  if(be->isMarked()){
    PD((GC,"relocate borrow unnecessary:%d",i));
    return;}
  be->makeMark();
  PD((GC,"GC-relocate borrow:%d old%x new %x",i,be,this));
  Assert(be->isDummyTertiary()==FALSE);
  be->mkTertiary(this);}

void Tertiary::gcManager(){
  Assert(getTertType()!=Te_Frame);
  int i=getIndex();
  PD((GC,"relocate owner:%d old%x new %x",
     i,ownerTable->getOwner(i),this));
  ownerTable->getOwner(i)->mkTertiary(this);}

void CellFrame::gcCellFrameSec(){
  int state=getState();
  PD((GC,"relocate Cell in state %d",state));
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
  PD((GC,"relocate cellFrame:%d",i));
  ownerTable->getOwner(i)->mkTertiary(this);
  gcCellFrameSec();}

void CellManager::gcCellManager(){
  int i=getOwnerIndex();
  PD((GC,"relocate cellManager",i));
  ownerTable->getOwner(i)->mkTertiary(this);
  if(!isOwnCurrent()){gcBorrowSec(getCurrent());}
  CellFrame *cf=(CellFrame*)this;
  cf->gcCellFrameSec();}

void Tertiary::gcTertiary() /* TODO -remove use */
{
  switch (getTertType()) {

  case Te_Local:
    return;

  case Te_Proxy:
    {
      switch (getType()) {
      case Co_Chunk:
      case Co_Object:
        return;
      default:
        {
          int i=getIndex();
          BorrowEntry *be = borrowTable->getBorrow(i);
          be->makeMark();
          PD((GC,"relocate borrow:%d old %x new %x",i,be,this));
          be->mkTertiary(this);
          return;
        }
      }
    }

  case Te_Manager:
    {
      int i=getIndex();
      PD((GC,"relocate owner:%d old%x new %x",i,ownerTable->getOwner(i),this));
      ownerTable->getOwner(i)->mkTertiary(this);
      return;
    }
  case Te_Frame:
    Assert(0);
  }
}

/*--------------------*/

void OwnerTable::gcOwnerTable()
{
  PD((GC,"owner gc"));
  int i;
  for(i=0;i<size;i++){
      OwnerEntry* o = ownerTable->getOwner(i);
      if(!o->isFree()) {
        PD((GC,"OT o:%d",i));
        o->gcPO();
      }
  }
  compactify();
  return;
}

void OwnerTable::gcPostOwnerTable()
{
  PD((GC,"owner post gc"));
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
    PD((GC,"BT b:%d variable found",i));
    PerdioVar *pv=getVar();
    if (pv->isProxy() && pv->hasVal()) {
      PD((WEIRD,"BT b:%d pending unmarked var found",i));
      gcPO();
    }
  }
}

void BorrowTable::gcBorrowTableRoots()
{
  PD((GC,"borrow gc roots"));
  int i;
  for(i=0;i<size;i++) {
    getBorrow(i)->gcBorrow(i);
  }
}

extern TaggedRef gcTagged1(TaggedRef in);

/* OBSERVE - this must done at the end of other gc */
void BorrowTable::gcBorrowTable()
{
  PD((GC,"borrow gc"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if (!b->isFree()) {
      if(b->isMarked()) {
        b->removeMark();
        PD((GC,"BT b:%d mark found",i));
        if (b->isVar()) {
          PD((GC,"BT b:%d variable found",i));
        } else {
          PD((GC,"BT b:%d tertiary found",i));
          Assert(b->isTertiary());
        }
      } else {
        PD((GC,"BT b:%d no mark found",i));
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
  PD((GC,"gname gc"));
  int index;
  for (GenHashNode *aux=getFirst(index); aux!=NULL; aux=getNext(aux,index)) {
    GName *gn = (GName*) aux->getBaseKey();

    /* code is never garbage collected */
    if (gn->getGNameType()==GNT_CODE)
      continue;

    if (gn->getGCMark()) {
      TaggedRef t = (TaggedRef) ToInt32(aux->getEntry());
      gcTagged(t,t);
      aux->setEntry((GenHashEntry*)ToPointer(t));
      gn->resetGCMark();
    } else {
      if (gn->getGNameType()==GNT_NAME &&
          tagged2Literal(ToInt32(aux->getEntry()))->isNamedName()) {
        continue;
      }
      delete gn;
      if (!htSub(index,aux)) continue;
    }
  }
  compactify();
}

extern void dogcGName(GName *gn);

void PerdioVar::gcPerdioVar(void)
{
  int i = getIndex();
  if (isProxy() || isTertProxy()) {
    PD((GC,"PerdioVar b:%d",i));
    BorrowEntry *be=borrowTable->getBorrow(i);
    be->gcPO();
    be->makeMark();

    if (isProxy()) {
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
      u.tert = (Tertiary*) u.tert->gcConstTerm();
    }
  } else if (isManager()) {
    PD((GC,"PerdioVar o:%d",i));
    ProxyList **last=&u.proxies;
    for (ProxyList *pl = u.proxies; pl; pl = pl->next) {
      ProxyList *newPL = new ProxyList(pl->sd,0);
      *last = newPL;
      last = &newPL->next;
    }
    *last = 0;
  } else {
    Assert(isURL());
    // u.url is atom (no gc necessary)
    dogcGName(getGName());
  }
}


void PerdioVar::addSuspPerdioVar()
{
  if (isTertProxy() && suspList==NULL) {
    // mm2
  }
  if (isURL() && suspList==NULL) {
    OZ_Term val;
    int ret=loadURL(getURL(),val);
    if (ret != PROCEED) {
      warning("mm2: load URL failed not impl");
    } else {
      SiteUnify(val,gnameTable->gnameFind(getGName()));
    }
  }
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
  PD((CELL,"val:%x v:%x",val,v));
  cf->initFromGlobalize(v);}

void CellManager::localize(){
  CellFrame *cf=(CellFrame *)this;
  Assert(cf->getState()==Cell_Valid);
  TaggedRef tr=cf->getContents();
  setBoard(am.rootBoard);
  setTertType(Te_Local);
  CellLocal *cl=(CellLocal*) this;
  cl->setValue(tr);}

// globalize a tertiary
// and add it to the owner table
void Tertiary::globalizeTert()
{
  Assert(isLocal());

  switch(getType()) {
  case Co_Abstraction:
    ((Abstraction *)this)->globalize();
    break;
  case Co_Chunk:
    ((SChunk *) this)->globalize();
    break;
  case Co_Object:
    ((Object*) this)->globalize();
    break;
  case Co_Cell:
    {
      PD((GLOBALIZING,"GLOBALIZING cell"));
      OwnerEntry *oe_manager;
      int manI=ownerTable->newOwner(oe_manager);
      oe_manager->mkTertiary(this);
      CellLocal *cl=(CellLocal*)this;
      cl->globalize(manI);
      return;
    }
  case Co_Thread:
  case Co_Space:
  case Co_Port:
    break;
  default:
    Assert(0);
  }

  PD((GLOBALIZING,"GLOBALIZING port/chunk"));
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
  switch(getType()) {
  case Co_Object:{
  case Co_Chunk:
  case Co_Port:
  case Co_Abstraction:
    Assert(getTertType()==Te_Manager);
    PD((GLOBALIZING,"GLOBALIZING: localizing tertiary manager"));
    setTertType(Te_Local);
    setBoard(am.rootBoard);
    return;}
  case Co_Cell:{
    if(getTertType()==Te_Manager){
      PD((GLOBALIZING,"GLOBALIZING: localizing cell manager"));
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

void Tertiary::import()
{
  switch(getType()) {
  case Co_Object:
  case Co_Chunk:
  case Co_Abstraction:
    {
      Assert(getTertType()==Te_Proxy);
      PD((GLOBALIZING,"GLOBALIZING: localizing tertiary manager"));
      setTertType(Te_Local);
      setBoard(am.rootBoard);
      return;
    }
  default:
    Assert(0);
  }
}

void SChunk::import(TaggedRef val)
{
  Assert(getTertType()==Te_Proxy);
  Tertiary::import();
  PD((GLOBALIZING,"GLOBALIZING: importing chunk proxy"));
  value = val;
}

void Object::import()
{
  Assert(getTertType()==Te_Proxy);
  Tertiary::import();
  PD((GLOBALIZING,"GLOBALIZING: importing object"));
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

inline
void gotRef(ByteStream *bs, TaggedRef val)
{
  PD((REF_COUNTER,"got: %d",bs->refCounter));
  refTable->set(bs->refCounter++,val);
}

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
    PD((DEBT,"push %d",i));
    push((void *)debt);
    push((void *)i);
  }
};


/**********************************************************************/
/*                 MARSHALLING/UNMARSHALLING  GROUND STRUCTURES       */
/**********************************************************************/


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

const int shortSize = 2;    /* TODO */

void marshallShort(unsigned short i, ByteStream *bs){
  PD((MARSHALL_CT,"Short %d BYTES:2",i));
  for (int k=0; k<shortSize; k++) {
    bs->put(i&0xFF);
    i = i>>8;}}


inline int unmarshallShort(ByteStream *bs){
  unsigned short sh;
  unsigned int i1 = bs->get();
  unsigned int i2 = bs->get();
  sh= (i1 + (i2<<8));
  PD((UNMARSHALL_CT,"Short %d BYTES:2",sh));
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

  char *ret = new char[i+1];  /* TODO: ask Ralf */
  int k=0;
  for (; k<i; k++) {
    ret[k] = bs->get();
  }
  PD((UNMARSHALL_CT,"String BYTES:%d",k));
  ret[i] = '\0';
  return ret;
}

inline
void marshallString(char *s, ByteStream *bs)
{
  marshallNumber(strlen(s),bs);
  PD((MARSHALL_CT,"String BYTES:%d",strlen(s)));
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
  marshallNumber((int)gname->gnameType,bs);
}

void unmarshallGName(GName *gname, ByteStream *bs)
{
  gname->site.ip        = unmarshallNumber(bs);
  gname->site.port      = unmarshallShort(bs);
  gname->site.timestamp = unmarshallNumber(bs);
  for (int i=0; i<fatIntDigits; i++) {
    gname->id.number[i] = unmarshallNumber(bs);
  }
  gname->gnameType = (GNameType) unmarshallNumber(bs);
}

Bool unmarshallGName(GName **gn, TaggedRef *ret, ByteStream *bs)
{
  GName gname;
  unmarshallGName(&gname,bs);

  TaggedRef aux = gnameTable->gnameFind(&gname);
  if (aux) {
    if (ret) *ret = aux;
    return OK;
  }
  *gn = copyGName(&gname);
  return NO;
}

/**********************************************************************/
/*            MARSHALLING/UNMARSHALLING NETWORK ADDRESSES             */
/**********************************************************************/

inline
void marshallCredit(Credit credit,ByteStream *bs){
  Assert(sizeof(Credit)==sizeof(long));
  Assert(sizeof(Credit)==sizeof(unsigned int));
  PD((MARSHALL,"credit c:%d",credit));
  marshallNumber(credit,bs);}

inline
Credit unmarshallCredit(ByteStream *bs){
  Assert(sizeof(Credit)==sizeof(long));
  Credit c=unmarshallNumber(bs);
  PD((UNMARSHALL,"credit c:%d",c));
  return c;}

inline
void marshallSite(Site *sd,ByteStream *bs){
  ip_address ip;
  port_t port;
  time_t timestamp;
  PD((MARSHALL,"site (10) s:%s", pSite(sd)));
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
  PD((MARSHALL,"index (4) o:%d",index));
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
    PD((UNMARSHALL,"site (10) s:%s", pSite(sd)));
    return sd;}
  OZ_warning("timeStamp exception\n");
  return sd;}


/*
 * marshall a OT entry (i)
 */
void marshallOwnHead(int tag,int i,ByteStream *bs){
  bs->put(tag);

  marshallNetAddress2(mySite,i,bs);
  marshallNumber(ownerTable->getOwner(i)->getSendCredit(),bs);
  PD((MARSHALL,"ownHead o:%d rest-c:%d ",i,ownerTable->getOwner(i)->getCredit()));
}

/*
 * marshall a BT entry (bi) which is send to its owner
 */
void marshallToOwner(int bi,ByteStream *bs){
  bs->put(DIF_OWNER);
  marshallNumber(borrowTable->getOriginIndex(bi),bs);
  BorrowEntry *b=BT->getBorrow(bi); /* implicit 1 credit */
  if(b->getOneCredit()) {
    PD((MARSHALL,"toOwner Borrow b:%d Owner o:%d",
                  bi,borrowTable->getOriginIndex(bi)));
    return;}
  debtRec->debtPush(1,bi);
  PD((MARSHALL,"toOwner Borrow b:%d Owner o:%d debt=1",
                bi,borrowTable->getOriginIndex(bi)));
  return;}

/*
 * marshall a BT entry (bi) into a message
 */
void marshallBorrowHead(int tag, int bi,ByteStream *bs){
  bs->put(tag);

  BorrowEntry *b=borrowTable->getBorrow(bi);
  marshallNetAddress(b->getNetAddress(),bs);
  Credit cred;
  if(b->getSmallCredit(cred)) {
    PD((MARSHALL,"borrowed b:%d remCredit c:%d give c:%d",
                bi,b->getCredit(),cred));
    marshallCredit(cred,bs);
    return;  }
  PD((MARSHALL,"borrowed b:%d remCredit c:%d debt c:%d",
                bi,b->getCredit(),MIN_BORROW_CREDIT_SIZE));
  marshallCredit(MIN_BORROW_CREDIT_SIZE,bs);
  debtRec->debtPush(MIN_BORROW_CREDIT_SIZE,bi);
  return;}

OZ_Term unmarshallBorrow(ByteStream *bs,OB_Entry *&ob,int &bi){
  Site * sd=unmarshallSiteId(bs);
  int si=unmarshallNumber(bs);
  PD((UNMARSHALL,"borrow o:%d",si));
  Credit cred = unmarshallCredit(bs);
  if (sd==mySite){
    PD((UNMARSHALL,"mySite is owner"));
    OZ_Term ret = ownerTable->getOwner(si)->getValue();
    ownerTable->returnCreditAndCheck(si,cred);
    DebugCode(ob=0;bi=-4711);
    return ret;}

  NetAddress na = NetAddress(sd,si);
  BorrowEntry *b = borrowTable->find(&na);
  if (b!=NULL) {
    PD((UNMARSHALL,"borrow found"));
    b->addCredit(cred);
    return b->getValue();
  }
  bi=borrowTable->newBorrow(cred,sd,si);
  b=borrowTable->getBorrow(bi);
  PD((UNMARSHALL,"borrowed miss"));
  ob=b;
  return 0;}


BorrowEntry *unmarshallBorrowEntry(ByteStream *bs)
{
  Site *sd      = unmarshallSiteId(bs);
  int si        = unmarshallNumber(bs);
  NetAddress na = NetAddress(sd,si);

  BorrowEntry *ret = borrowTable->find(&na);
  Assert(ret!=NULL);
  return ret;
}


/**********************************************************************/
/*                 MARSHALLING terms                                  */
/**********************************************************************/


inline
Bool checkCycle(OZ_Term t, ByteStream *bs)
{
  if (!isRef(t) && tagTypeOf(t)==GCTAG) {
    PD((MARSHALL,"circular: %d",t>>tagSize));
    bs->put(DIF_REF);
    marshallNumber(t>>tagSize,bs);
    return OK;
  }
  return NO;
}

inline
void trailCycle(OZ_Term *t,int r)
{
  PD((REF_COUNTER,"trail: %d",r));
  refTrail->trail(t);
  *t = (r<<tagSize)|GCTAG;
}

void marshallAbstractionProxy(Site *sd,Abstraction *a,ByteStream *bs) {
  marshallGName(a->getGName(),bs);
  marshallGName(a->getPred()->getGName(),bs);
  marshallTerm(sd,a->getName(),bs);
  marshallNumber(a->getArity(),bs);
}

void marshallClosure(Site *sd,Abstraction *a,ByteStream *bs) {
  RefsArray globals = a->getGRegs();
  int gs = globals ? a->getGSize() : 0;
  marshallNumber(gs,bs);
  for (int i=0; i<gs; i++) {
    marshallTerm(sd,globals[i],bs);
  }
}

Bool marshallTertHead(Site *sd, Tertiary *t, int tag, ByteStream *bs)
{
  switch(t->getTertType()){
  case Te_Local:
    t->globalizeTert();
    // no break here!
  case Te_Manager:
    {
      PD((MARSHALL,"manager"));
      int OTI=t->getIndex();
      marshallOwnHead(tag,OTI,bs);
      if (!sd) {
        OT->getOwner(OTI)->makePersistent();
      }
      break;
    }
  case Te_Frame:
    {
      PD((MARSHALL,"frame"));
      Assert(t->getType()==Co_Cell);
      CellFrame *cf=(CellFrame *)t;
      int OTI=cf->getOwnerIndex();
      marshallOwnHead(tag,OTI,bs);
      if (!sd) {
        OT->getOwner(OTI)->makePersistent();
      }
      break;
    }
  case Te_Proxy:
    {
      PD((MARSHALL,"proxy"));
      int BTI=t->getIndex();
      if (sd && borrowTable->getOriginSite(BTI)==sd) {
        marshallToOwner(BTI,bs);
        return OK;}
      marshallBorrowHead(tag,BTI,bs);
      if (!sd) {
        BT->getBorrow(BTI)->makePersistent();
      }
      break;
    }
  default:
    Assert(0);
  }
  return NO;
}

void marshallURL(GName *gname, TaggedRef t, ByteStream *bs) {
  PD((MARSHALL,"URL %s",toC(t)));
  bs->put(DIF_URL);
  marshallGName(gname,bs);
  Assert(isAtom(t));
  marshallTerm(0,t,bs);
}

Bool checkURL(GName *gname, ByteStream *bs) {
  TaggedRef t = gname->getURL();
  if (t) {
    marshallURL(gname,t,bs);
    return OK;
  }
  if (currentURL) { gname->markURL(currentURL); }
  return NO;
}

void marshallTertiary(Site *sd, Tertiary *t, ByteStream *bs)
{
  switch (t->getType()) {
  case Co_Abstraction:
    {
      Abstraction *pp=(Abstraction *) t;
      GName *gname=pp->getGName();
      if (checkURL(gname,bs)) return;

      bs->put(DIF_PROC);
      marshallAbstractionProxy(sd,pp,bs);

      trailCycle(t->getRef(),bs->refCounter++);

      marshallClosure(sd,pp,bs);
      marshallCode(sd,pp->getPC(),bs);
      return;
    }

  case Co_Chunk:
    {
      SChunk *ch=(SChunk *) t;
      GName *gname=ch->getGName();
      if (checkURL(gname,bs)) return;

      bs->put(DIF_CHUNK);
      marshallGName(gname,bs);

      trailCycle(t->getRef(),bs->refCounter++);

      marshallTerm(sd,ch->getValue(),bs);
      return;
    }

  case Co_Object:
    {
      Object *o = (Object*) t;
      if (o->isClass()) {
        if (checkURL(o->getGName(),bs)) return;

        bs->put(DIF_CLASS);
        marshallGName(o->getGName(),bs);
        trailCycle(t->getRef(),bs->refCounter++);
        marshallClass(sd,o,bs);
      } else {
        if (marshallTertHead(sd,t,DIF_OBJECT,bs)) return;
        marshallGName(o->getGName(),bs);
        marshallTerm(sd,makeTaggedConst(o->getOzClass()),bs);
        trailCycle(t->getRef(),bs->refCounter++);
        marshallObject(sd,o,bs);
      }
      return;
    }

  case Co_Thread:
    if (marshallTertHead(sd,t,DIF_THREAD,bs)) return;
    break;
  case Co_Space:
    if (marshallTertHead(sd,t,DIF_SPACE,bs)) return;
    break;
  case Co_Cell:
    if (marshallTertHead(sd,t,DIF_CELL,bs)) return;
    break;
  case Co_Port:
    if (marshallTertHead(sd,t,DIF_PORT,bs)) return;
    break;
  default:
    error("marshallTertiary(%d) not impl",t->getType());
  }

  trailCycle(t->getRef(),bs->refCounter++);
}

void unmarshallTertiary(ByteStream *bs, TaggedRef *ret, MarshallTag tag,
                        char *comment)
{
  OB_Entry *ob;
  int bi;
  OZ_Term val = unmarshallBorrow(bs,ob,bi);
  if (val) {
    PD((UNMARSHALL,"%s hit b:%d",bi,comment));
    gotRef(bs,val);
    *ret=val;
    return;
  }
  PD((UNMARSHALL,"%s miss b:%d",bi,comment));

  Tertiary *tert = NULL;
  switch (tag) {
  case DIF_PORT:   tert = new PortProxy(bi);        break;
  case DIF_THREAD: tert = new Thread(bi,Te_Proxy);  break;
  case DIF_SPACE:  tert = new Space(bi,Te_Proxy);   break;
  default:         Assert(0);
  }

  *ret= makeTaggedConst(tert);
  gotRef(bs,*ret);
  ob->mkTertiary(tert);
}

void marshallVariable(Site *sd, PerdioVar *pvar, ByteStream *bs)
{
  if (pvar->isProxy()) {
    int i=pvar->getIndex();
    PD((MARSHALL,"var proxy o:%d",i));
    if(sd && borrowTable->getOriginSite(i)==sd) {
      marshallToOwner(i,bs);
      return;
    }
    marshallBorrowHead(DIF_VAR,i,bs);
    if (!sd) {
      warning("mm2: make persistent of proxy not fully impl.");
      BT->getBorrow(i)->makePersistent();
    }
    return;
  }

  if (pvar->isTertProxy()) {
    PD((MARSHALL,"var tertproxy"));
    marshallTertiary(sd,pvar->getTertiary(),bs);
    return;
  }

  if (pvar->isURL()) {
    PD((MARSHALL,"var url"));
    marshallURL(pvar->getGName(),pvar->getURL(),bs);
    return;
  }

  // owner
  int i=pvar->getIndex();
  PD((MARSHALL,"var manager o:%d",i));
  Assert(pvar->isManager());
  marshallOwnHead(DIF_VAR,i,bs);
  if (!sd) {
    OT->getOwner(i)->makePersistent();
  }
}

#include "marshallcode.cc"

void marshallSRecord(Site *sd, SRecord *sr, ByteStream *bs)
{
  if (!sr) {
    bs->put(0);
  } else {
    bs->put(1);
    marshallTerm(sd,makeTaggedSRecord(sr),bs);
  }
}

SRecord *unmarshallSRecord(ByteStream *bs)
{
  if (bs->get()==0)
    return NULL;

  return tagged2SRecord(unmarshallTerm(bs));
}


void marshallDict(Site *sd, OzDictionary *d, ByteStream *bs)
{
  int size = d->getSize();
  marshallNumber(size,bs);

  int i = d->getFirst();
  i = d->getNext(i);
  while(i>=0) {
    marshallTerm(sd,d->getKey(i),bs);
    marshallTerm(sd,d->getValue(i),bs);
    i = d->getNext(i);
    size--;
  }
  Assert(size==0);
}

OzDictionary *unmarshallDict(ByteStream *bs)
{
  int size = unmarshallNumber(bs);
  OzDictionary *ret = new OzDictionary(am.currentBoard,size);

  while(size-- > 0) {
    TaggedRef key = unmarshallTerm(bs);
    TaggedRef val = unmarshallTerm(bs);
    ret->setArg(key,val);
  }
  return ret;
}

void unmarshallObject(Object *o, ByteStream *bs)
{
  SRecord *feat = unmarshallSRecord(bs);
  OZ_Term t     = unmarshallTerm(bs);
  OZ_Term lock  = unmarshallTerm(bs);
  if (o==NULL)
    return;

  o->setFreeRecord(feat);
  if (o->getTertType()==Te_Proxy) o->import();
  o->setState(tagged2Tert(t));

  // TODO: make locks work
  o->setLock(isNil(lock) ? (OzLock*)NULL : new OzLock(am.rootBoard));
}

void unmarshallClass(Object *o, ByteStream *bs)
{
  SRecord *feat = unmarshallSRecord(bs);
  if (o==NULL)
    return;

  o->setFreeRecord(feat);
  if (o->getTertType()==Te_Proxy) o->import();

  TaggedRef ff = feat->getFeature(NameOoUnFreeFeat);
  Bool locking = literalEq(NameTrue,deref(feat->getFeature(NameOoLocking)));

  o->getClass()->import(tagged2Dictionary(feat->getFeature(NameOoFastMeth)),
                        isSRecord(ff) ? tagged2SRecord(ff) : (SRecord*)NULL,
                        tagged2Dictionary(feat->getFeature(NameOoDefaults)),
                        locking);
}

void marshallObject(Site *sd, Object *o, ByteStream *bs)
{
  marshallSRecord(sd,o->getFreeRecord(),bs);
  marshallTerm(sd,makeTaggedConst(getCell(o->getState())),bs);
  if (o->getLock()) {
    warning("marshallObject(%s): cannot handle locks correctly (yet)",
            toC(makeTaggedConst(o)));
    marshallTerm(sd,makeTaggedConst(o->getLock()),bs);
  } else {
    marshallTerm(sd,nil(),bs);
  }
}

void marshallClass(Site *sd, Object *o, ByteStream *bs)
{
  marshallSRecord(sd,o->getFreeRecord(),bs);
}

void marshallTerm(Site *sd, OZ_Term t, ByteStream *bs)
{
  OZ_Term *args;
  int argno;

loop:
  DEREF(t,tPtr,tTag);
  switch(tTag) {

  case SMALLINT:
    bs->put(DIF_SMALLINT);
    marshallNumber(smallIntValue(t),bs);
    PD((MARSHALL,"small int: %d",smallIntValue(t)));
    break;

  case OZFLOAT:
    bs->put(DIF_FLOAT);
    marshallFloat(tagged2Float(t)->getValue(),bs);
    PD((MARSHALL,"float"));
    break;

  case BIGINT:
    bs->put(DIF_BIGINT);
    marshallString(toC(t),bs);
    PD((MARSHALL,"big int"));
    break;

  case LITERAL:
    {
      Literal *lit = tagged2Literal(t);
      if (checkCycle(*lit->getRef(),bs)) return;

      if (lit->isAtom()) {
        bs->put(DIF_ATOM);
        PD((MARSHALL_CT,"tag DIF_ATOM  BYTES:1"));
        marshallString(lit->getPrintName(),bs);
        PD((MARSHALL,"atom: %s",lit->getPrintName()));
      } else if (lit->getFlags()&Lit_isUniqueName) {
        bs->put(DIF_UNIQUENAME);
        marshallString(lit->getPrintName(),bs);
      } else {
        bs->put(DIF_NAME);
        GName *gname = ((Name*)lit)->globalize();
        marshallGName(gname,bs);
        marshallString(lit->getPrintName(),bs);
      }
      trailCycle(lit->getRef(),bs->refCounter++);
      break;
    }

  case LTUPLE:
    {
      LTuple *l = tagged2LTuple(t);
      if (checkCycle(*l->getRef(),bs)) return;
      bs->put(DIF_LIST);
      PD((MARSHALL_CT,"tag DIF_LIST BYTES:1"));
      argno = 2;
      args  = l->getRef();
      PD((MARSHALL,"list"));
      goto processArgs;
    }

  case SRECORD:
    {
      SRecord *rec = tagged2SRecord(t);
      if (checkCycle(*rec->getRef(),bs)) return; /* TODO mark instead of getRef ??*/
      if (rec->isTuple()) {
        bs->put(DIF_TUPLE);
        PD((MARSHALL_CT,"tag DIF_TUPLE BYTES:1"));
        marshallNumber(rec->getTupleWidth(),bs);
      } else {
        bs->put(DIF_RECORD);
        PD((MARSHALL_CT,"tag DIF_RECORD BYTES:1"));
        marshallTerm(sd,rec->getArityList(),bs);
      }
      marshallTerm(sd,rec->getLabel(),bs);
      argno = rec->getWidth();
      args  = rec->getRef();
      PD((MARSHALL,"record-tuple no:%d",argno));
      goto processArgs;
    }

  case OZCONST:
    {
      PD((MARSHALL,"constterm"));
      if (checkCycle(*(tagged2Const(t)->getRef()),bs))
        break;

      if (isBuiltin(t)) {
        PD((MARSHALL,"builtin"));
        bs->put(DIF_BUILTIN);
        PD((MARSHALL_CT,"tag DIF_BUILTIN BYTES:1"));
        marshallString(tagged2Builtin(t)->getPrintName(),bs);
        break;
      }

      if (isLock(t)) {
        // TODO: make locks work
        PD((MARSHALL,"lock"));
        bs->put(DIF_LOCK);
        break;
      }

      if (isDictionary(t)) {
        PD((MARSHALL,"dictionary"));
        bs->put(DIF_DICT);
        marshallDict(sd,tagged2Dictionary(t),bs);
        break;
      }

      marshallTertiary(sd,tagged2Tert(t),bs);
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

      marshallVariable(sd,pvar,bs);
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

      marshallVariable(sd,pvar,bs);
      break;
    }
  case CVAR:
    switch (tagged2CVar(t)->getType()) {
    case PerdioVariable:
      {
        PerdioVar *pvar = (PerdioVar *) tagged2CVar(t);
        marshallVariable(sd,pvar,bs);
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
    marshallTerm(sd,nil(),bs);
    break;
  }

  return;

processArgs:
  OZ_Term arg0 = tagged2NonVariable(args);
  if (!isRef(*args) && isAnyVar(*args)) {
    int r=bs->refCounter++;
    marshallTerm(sd,arg0,bs);
    trailCycle(args,r);
  } else {
    trailCycle(args,bs->refCounter++);
    marshallTerm(sd,arg0,bs);
  }
  args++;
  if (argno == 1) return;
  for(int i=1; i<argno-1; i++) {
    marshallTerm(sd,tagged2NonVariable(args),bs);
    args++;
  }
  // tail recursion optimization
  t = tagged2NonVariable(args);
  goto loop;
}


RefsArray unmarshallClosure(ByteStream *bs) {
  int gsize = unmarshallNumber(bs);
  RefsArray globals = gsize==0 ? 0 : allocateRefsArray(gsize);

  for (int i=0; i<gsize; i++) {
    globals[i] = unmarshallTerm(bs);
  }
  return globals;
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
  PD((UNMARSHALL_CT,"tag %c BYTES:1",tag));

  switch(tag) {

  case DIF_SMALLINT:
    *ret = OZ_int(unmarshallNumber(bs));
    PD((UNMARSHALL,"small int"));
    return;

  case DIF_FLOAT:
    *ret = OZ_float(unmarshallFloat(bs));
    PD((UNMARSHALL,"float"));
    return;

  case DIF_NAME:
    {
      GName *gname;
      Bool foundit    = unmarshallGName(&gname,ret,bs);
      char *printname = unmarshallString(bs);

      PD((UNMARSHALL,"name %s",printname));

      if (!foundit) {
        Name *aux;
        if (strcmp("",printname)==0) {
          aux = Name::newName(am.currentBoard);
        } else {
          aux = NamedName::newNamedName(printname);
        }
        aux->import(gname);
        *ret = makeTaggedLiteral(aux);
        gnameTable->gnameAdd(gname,*ret);
      }
      delete printname;
      gotRef(bs,*ret);
      return;
    }

  case DIF_UNIQUENAME:
    {
      char *printname = unmarshallString(bs);

      PD((UNMARSHALL,"unique name %s",printname));

      *ret = getUniqueName(printname);
      delete printname;
      gotRef(bs,*ret);
      return;
    }

  case DIF_ATOM:
    {
      char *aux = unmarshallString(bs);
      PD((UNMARSHALL,"atom %s",aux));
      *ret = OZ_atom(aux);
      delete aux;
      gotRef(bs,*ret);
      return;
    }

  case DIF_BIGINT:
    {
      char *aux = unmarshallString(bs);
      PD((UNMARSHALL,"big int %s",aux));
      *ret = OZ_CStringToNumber(aux);
      delete aux;
      return;
    }

  case DIF_LIST:
    {
      OZ_Term head, tail;
      argno = 2;
      LTuple *l = new LTuple();
      *ret = makeTaggedLTuple(l);
      gotRef(bs,*ret);
      ret = l->getRef();
      PD((UNMARSHALL,"list"));
      goto processArgs;
    }
  case DIF_TUPLE:
    {
      argno = unmarshallNumber(bs);
      TaggedRef label = unmarshallTerm(bs);
      SRecord *rec = SRecord::newSRecord(label,argno);
      *ret = makeTaggedSRecord(rec);
      gotRef(bs,*ret);
      ret = rec->getRef();
      PD((UNMARSHALL,"tuple no_args:%d",argno));
      goto processArgs;
    }

  case DIF_RECORD:
    {
      TaggedRef arity = unmarshallTerm(bs);
      argno = length(arity);
      TaggedRef label = unmarshallTerm(bs);
      SRecord *rec = SRecord::newSRecord(label,mkArity(arity));
      *ret = makeTaggedSRecord(rec);
      gotRef(bs,*ret);
      ret = rec->getRef();
      PD((UNMARSHALL,"record no:%d",argno));
      goto processArgs;
    }

  case DIF_REF:
    {
      int i = unmarshallNumber(bs);
      PD((UNMARSHALL,"ref: %d",i));
      *ret = refTable->get(i);
      Assert(*ret);
      return;
    }

  case DIF_OWNER:
    {
      int OTI=unmarshallNumber(bs);
      PD((UNMARSHALL,"OWNER o:%d",OTI));
      *ret = OT->getOwner(OTI)->getValue();
      OT->returnCreditAndCheck(OTI,1);
      return;
    }

  case DIF_PORT:   unmarshallTertiary(bs,ret,tag,"port");   return;
  case DIF_THREAD: unmarshallTertiary(bs,ret,tag,"thread"); return;
  case DIF_SPACE:  unmarshallTertiary(bs,ret,tag,"space");  return;

  case DIF_CELL:
    {
      OB_Entry *ob;
      int bi;
      OZ_Term val = unmarshallBorrow(bs,ob,bi);
      if (val) {
        PD((UNMARSHALL,"cell hit b:%d",bi));
        gotRef(bs,val);
        Tertiary *t=BT->getBorrow(bi)->getTertiary();
        if((t->getType()==Co_Cell) && (t->getTertType()==Te_Frame)){
          CellFrame *cf=(CellFrame *)t;
          if(cf->getState() & Cell_Dump_Asked){
            cf->setState(cf->getState() & ~Cell_Dump_Asked);}}
        *ret=val;
        return;
      }
      PD((UNMARSHALL,"cell miss b:%d",bi));
      Tertiary *tert = new CellProxy(bi);
      *ret= makeTaggedConst(tert);
      gotRef(bs,*ret);
      ob->mkTertiary(tert);
      return;
    }

  case DIF_URL:
    {
      GName *gname;
      int found     = unmarshallGName(&gname,ret,bs);
      TaggedRef url = unmarshallTerm(bs);

      PD((UNMARSHALL,"url: %s",toC(url)));
      if (found) {
        PD((UNMARSHALL,"url found"));
        return;
      }

      PerdioVar *pvar = new PerdioVar(gname,url);

      TaggedRef val = makeTaggedRef(newTaggedCVar(pvar));
      addGName(gname,val);
      *ret = val;
      return;
    }

  case DIF_CHUNK:
    {
      GName *gname=new GName();  unmarshallGName(gname,bs);

      PD((UNMARSHALL,"chunk"));
      TaggedRef ch=gnameTable->gnameFind(gname);

      if (!ch || !isSChunk(ch)) {
        SChunk *sc=new SChunk(am.rootBoard,0);
        sc->setGName(gname);
        if (ch) {
          SiteUnify(ch,makeTaggedConst(sc));
        } else {
          ch = makeTaggedConst(sc);
          addGName(gname,ch);
        }
      } else {
        delete gname;
      }

      *ret = ch;
      gotRef(bs,ch);

      TaggedRef value = unmarshallTerm(bs);
      tagged2SChunk(ch)->install(value);

      return;
    }

  case DIF_OBJECT:
    {
      OB_Entry *ob;
      int bi;
      GName *gname;
      OZ_Term val1    = unmarshallBorrow(bs,ob,bi);
      Bool foundit    = unmarshallGName(&gname,ret,bs);
      TaggedRef clas  = unmarshallTerm(bs);

      PD((UNMARSHALL,"object"));

      if (val1) {
        Assert(foundit);
        PD((UNMARSHALL,"object hit: b:%d",bi));
        gotRef(bs,val1);
        *ret=val1;
        unmarshallObject(0,bs);
        return;
      }

      if (foundit) {
        BT->maybeFreeBorrowEntry(bi);
        gotRef(bs,*ret);
        unmarshallObject(0,bs);
        return;
      }

      PD((UNMARSHALL,"object miss: b:%d",bi));
      Object *obj = new Object(bi,gname);

      TaggedRef val;
      if (isClass(deref(clas))) {
        obj->setClass(tagged2Object(deref(clas))->getClass());
        val = makeTaggedConst(obj);
      } else {
        PerdioVar *pvar = new PerdioVar(obj,bi);
        val = makeTaggedRef(newTaggedCVar(pvar));
      }
      gnameTable->gnameAdd(gname,val);
      *ret = val;
      ob->mkVar(val);

      gotRef(bs,val);

      unmarshallObject(obj,bs);

      return;
    }


  case DIF_CLASS:
    {
      GName *gname=new GName();  unmarshallGName(gname,bs);

      PD((UNMARSHALL,"class all"));

      TaggedRef oo=gnameTable->gnameFind(gname);
      Object *obj;
      if (oo && isClass(oo)) {
        delete gname;
        obj = tagged2Object(oo);
      } else {
        ObjectClass *ocl = new ObjectClass(NULL,NULL,NULL,NO);
        obj = new Object(am.rootBoard,0,ocl,0,OK,0);
        obj->setClass();
        ocl->setOzClass(obj);
        obj->setGName(gname);
        if (oo) {
          SiteUnify(oo,makeTaggedConst(obj));
        } else {
          oo = makeTaggedConst(obj);
          addGName(gname,oo);
        }
      }

      gotRef(bs,oo);
      *ret=oo;

      unmarshallClass(obj,bs);

      return;
    }

  case DIF_VAR:
    {
      OB_Entry *ob;
      int bi;
      OZ_Term val1 = unmarshallBorrow(bs,ob,bi);

      if (val1) {
        PD((UNMARSHALL,"var/chunk hit: b:%d",bi));
        *ret=val1;
        return;
      }
      PD((UNMARSHALL,"var miss: b:%d",bi));
      PerdioVar *pvar = new PerdioVar(bi);
      TaggedRef val = makeTaggedRef(newTaggedCVar(pvar));
      *ret = val;
      ob->mkVar(val);

      sendRegister((BorrowEntry *)ob);
      return;
    }

  case DIF_PROC:
    {
      GName *gname = new GName();      unmarshallGName(gname,bs);
      GName *gnamecode = new GName();  unmarshallGName(gnamecode,bs);
      OZ_Term name                   = unmarshallTerm(bs);
      int arity                      = unmarshallNumber(bs);

      PD((UNMARSHALL,"proc all"));

      TaggedRef proc=gnameTable->gnameFind(gname);
      Abstraction *pp;
      if (proc && isAbstraction(proc)) {
        delete gname;
        delete gnamecode;
        pp = tagged2Abstraction(proc);
        pp = 0; // mm2
      } else {
        PrTabEntry *pr=findCodeGName(gnamecode);
        if (pr) {
          delete gnamecode;
        } else {
          pr=new PrTabEntry(name,mkTupleWidth(arity),AtomNil,0);
          pr->setGName(gnamecode);
          addGName(gnamecode,pr);
        }
        pp=new Abstraction(pr,0,am.rootBoard);
        pp->setGName(gname);
        if (proc) {
          Assert(isURL(deref(proc)));
          SiteUnify(proc,makeTaggedConst(pp));
          proc = makeTaggedConst(pp);
          delete gname;
        } else {
          proc = makeTaggedConst(pp);
          addGName(gname,proc);
        }
      }
      *ret = proc;
      gotRef(bs,proc);

      RefsArray globals = unmarshallClosure(bs);
      ProgramCounter PC = unmarshallCode(bs);

      if (pp) pp->import(globals,PC);

      return;
    }

  case DIF_DICT:
    {
      PD((UNMARSHALL,"dict"));
      *ret = makeTaggedConst(unmarshallDict(bs));
      return;
    }
  case DIF_LOCK:
    {
      // TODO: make locks work
      PD((UNMARSHALL,"lock"));
      *ret = makeTaggedConst(new OzLock(am.rootBoard));
      return;
    }
  case DIF_BUILTIN:
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

  Assert(0);
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
  case M_PORT_SEND:    /* M_PORT_SEND index term */
    {
      int portIndex = unmarshallNumber(bs);
      OZ_Term t = unmarshallTerm(bs);
      Assert(t);
      bs->unmarshalEnd();
      PD((MSG_RECEIVED,"PORTSEND: o:%d v:%s",portIndex,toC(t)));

      Tertiary *tert= ownerTable->getOwner(portIndex)->getTertiary();
      ownerTable->returnCreditAndCheck(portIndex,1);
      Assert(tert->checkTertiary(Co_Port,Te_Manager));
      PortManager *pm=(PortManager*)tert;

      LTuple *lt = new LTuple(t,am.currentUVarPrototype());
      OZ_Term old = pm->exchangeStream(lt->getTail());
      PD((SPECIAL,"just after send port"));
      SiteUnify(makeTaggedLTuple(lt),old);
      break;
      }

  case M_REMOTE_SEND:    /* index string term */
    {
      int i = unmarshallNumber(bs);
      char *biName = unmarshallString(bs);
      OZ_Term t = unmarshallTerm(bs);
      Assert(t);
      bs->unmarshalEnd();
      PD((MSG_RECEIVED,"REMOTE_SEND: o:%d bi:%s v:%s",i,biName,toC(t)));

      Tertiary *tert= ownerTable->getOwner(i)->getTertiary();
      ownerTable->returnCreditAndCheck(i,1);

      BuiltinTabEntry *found = builtinTab.find(biName);

      if (!found) {
        PD((WEIRD,"builtin %s not found",biName));
        break;
      }

      RefsArray args=allocateRefsArray(2,NO);
      args[0]=makeTaggedConst(tert);
      args[1]=t;
      int arity=found->getArity();
      Assert(arity<=2);
      OZ_Return ret = found->getFun()(arity,args);
      if (ret != PROCEED) {
        PD((SPECIAL,"REMOTE_SEND failed: %d\n",ret));
      }
      break;
    }
  case M_ASK_FOR_CREDIT:
    {
      int na_index=unmarshallNumber(bs);
      Site* rsite=unmarshallSiteId(bs);
      bs->unmarshalEnd();
      PD((MSG_RECEIVED,"ASK_FOR_CREDIT o:%d s:%s",
         na_index,pSite(rsite)));
      OwnerEntry *o=OT->getOwner(na_index);
      o->returnCredit(1); // don't delete entry
      Credit c= o->giveMoreCredit();
      ByteStream *bs1=bufferManager->getByteStreamMarshal();
      bs1->put(M_BORROW_CREDIT);
      NetAddress na = NetAddress(mySite,na_index);
      marshallNetAddress(&na,bs1);
      marshallCredit(c,bs1);
      bs1->marshalEnd();
      PD((MSG_SENT,"BORROW_CREDIT s:%s o:%d c:%d", pSite(rsite),na_index,c));
      reliableSendFail(rsite,bs1,TRUE,4);
      break;
    }
  case M_OWNER_CREDIT:
    {
      int index=unmarshallNumber(bs);
      Credit c=unmarshallCredit(bs);
      bs->unmarshalEnd();
      PD((MSG_RECEIVED,"OWNER_CREDIT o:%d c:%d",index,c));

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
      PD((MSG_RECEIVED,"BORROW_CREDIT s:%s o:%d c:%d",pSite(sd),si,c));

      NetAddress na=NetAddress(sd,si);

      BorrowEntry *b=borrowTable->find(&na);
      Assert(b!=NULL);
      b->addAskCredit(c);
      break;
    }

  case M_REGISTER:
    {
      int OTI=unmarshallNumber(bs);
      Site* rsite=unmarshallSiteId(bs);
      bs->unmarshalEnd();
      PD((MSG_RECEIVED,"REGISTER o:%d s:%s",OTI,pSite(rsite)));

      if (OT->getOwner(OTI)->isVar()) {
        PerdioVar *pv=OT->getOwner(OTI)->getVar();
        if (!pv->isRegistered(rsite)) {
          pv->registerSite(rsite);
        } else {
          PD((WEIRD,"REGISTER o:%d s:%s already registered",OTI,pSite(rsite)));
        }

      } else {
        sendRedirect(rsite,OTI,OT->getOwner(OTI)->getRef());
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
      PD((MSG_RECEIVED,"REDIRECT s:%s o:%d v:%s",pSite(sd),si,toC(val)));

      NetAddress na=NetAddress(sd,si);
      BorrowEntry *be=BT->find(&na);

      if (!be) { // if not found, then forget the redirect message
        PD((WEIRD,"REDIRECT: no borrow entry found"));
        sendCreditBack(na.site,na.index,1);
        return;
      }
      be->addCredit(1);

      Assert(be->isVar());
      PerdioVar *pv = be->getVar();
      PD((TABLE,"REDIRECT - borrow entry hit b:%d",pv->getIndex()));
      Assert(pv->isProxy());
      pv->primBind(be->getPtr(),val);
      be->mkRef();

      if (pv->hasVal()) {
        PD((PD_VAR,"REDIRECT while pending"));
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
      PD((MSG_RECEIVED,"SURRENDER s:%s o:%d v:%s", pSite(rsite), OTI, toC(v)));

      OwnerEntry *oe = ownerTable->getOwner(OTI);

      if (oe->isVar()) {
        PD((PD_VAR,"SURRENDER do it"));
        PerdioVar *pv = oe->getVar();
        // bug fixed: may be bound to a different perdio var
        pv->primBind(oe->getPtr(),v);
        oe->mkRef();
        oe->returnCredit(1); // don't delete!
        if (oe->hasFullCredit()) {
          PD((WEIRD,"SURRENDER: full credit"));
        }
        sendRedirect(pv->getProxies(),v,rsite,OTI);
      } else {
        PD((PD_VAR,"SURRENDER discard"));
        PD((WEIRD,"SURRENDER discard"));
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
      PD((MSG_RECEIVED,"ACKNOWLEDGE s:%s o:%d",pSite(sd),si));

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
  marshallTerm(sd,t,bs);
  refTrail->unwind();
}

void domarshallTerm(TaggedRef url,OZ_Term t, ByteStream *bs)
{
  currentURL=url;
  marshallTerm(0,t,bs);
  refTrail->unwind();
  currentURL=0;
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

  ByteStream *bs = bufferManager->getByteStreamMarshal();
  bs->put(M_REMOTE_SEND);
  marshallNumber(index,bs);
  marshallString(biName,bs);
  domarshallTerm(site,msg,bs);
  bs->marshalEnd();
  PD((MSG_SENT,"PORTSEND s:%s o:%d v:%s",pSite(site),index,toC(msg)));

  PD((MSG_SENT,"REMOTE_SEND s:%s o:%d bi:%s v:%s",
     pSite(site),index,biName,toC(msg)));

  if (b->getOneCredit()) {
    if(debtRec->isEmpty()) {
      reliableSendFail(site,bs,FALSE,11);
    } else {
      PD((DEBT_SEC,"remoteSend"));
      PendEntry *pe=pendEntryManager->newPendEntry(bs,site,b);
      b->inDebtFIFO(0,pe);
      debtRec->handler(pe);
    }
  } else {
    PD((DEBT_MAIN,"remoteSend"));
    PendEntry *pe=pendEntryManager->newPendEntry(bs,site,b);
    pe= pendEntryManager->newPendEntry(bs,site,b);
    b->inDebtFIFO(1,pe);
    if(!debtRec->isEmpty()){
      PD((DEBT_SEC,"remoteSend"));
      debtRec->handler(pe);
    }
  }
  return PROCEED;
}


void portSend(Tertiary *p, TaggedRef msg) {
  int pi = p->getIndex();
  NetAddress *na = BT->getBorrow(pi)->getNetAddress();
  Site* site = na->site;
  int index = na->index;

  ByteStream *bs = bufferManager->getByteStream();
  bs->marshalBegin();
  bs->put(M_PORT_SEND);
  marshallNumber(index,bs);
  domarshallTerm(site,msg,bs);
  bs->marshalEnd();

  PD((MSG_SENT,"PORT_SEND s:%s o:%d v:%s",
     pSite(site),index,toC(msg)));

  BorrowEntry *b = BT->getBorrow(pi);

  if (b->getOneCredit()) {
    if(debtRec->isEmpty()) {
      reliableSendFail(site,bs,FALSE,11);
    } else {
      PD((DEBT_SEC,"portSend"));
      PendEntry *pe=pendEntryManager->newPendEntry(bs,site,b);
      b->inDebtFIFO(0,pe);
      debtRec->handler(pe);
    }
  } else {
    PD((DEBT_MAIN,"portSend"));
    PendEntry *pe=pendEntryManager->newPendEntry(bs,site,b);
    pe= pendEntryManager->newPendEntry(bs,site,b);
    b->inDebtFIFO(1,pe);
    if(!debtRec->isEmpty()){
      PD((DEBT_SEC,"portSend"));
      debtRec->handler(pe);
    }
  }
  return;
}

void sendMessage(int bi, MessageType msg)
{
  Site* site     = borrowTable->getOriginSite(bi);
  int index      = borrowTable->getOriginIndex(bi);
  ByteStream *bs = bufferManager->getByteStreamMarshal();

  bs->put(msg);
  marshallNumber(index,bs);
  marshallMySite(bs);
  bs->marshalEnd();

  PD((MSG_SENT,"MSG_SENT m:%d s:%s o:%d", msg,pSite(site),index));

  BorrowEntry *b = borrowTable->getBorrow(bi);
  if (b->getOneCredit()) {
    reliableSendFail(site,bs,FALSE,4711);
  } else {
    b->inDebtMain(pendEntryManager->newPendEntry(bs,site,b));
  }
}

void sendSurrender(BorrowEntry *be,OZ_Term val)
{
  ByteStream *bs= bufferManager->getByteStreamMarshal();
  bs->put(M_SURRENDER);
  NetAddress *na = be->getNetAddress();
  Site* site = na->site;
  int index = na->index;
  marshallNumber(index,bs);
  marshallMySite(bs);
  domarshallTerm(site,val,bs);
  bs->marshalEnd();
  PD((MSG_SENT,"SURRENDER s:%s o:%d v:%s", pSite(site),index,toC(val)));

  if (be->getOneCredit()) {
    if (debtRec->isEmpty()) {
      reliableSendFail(site,bs,FALSE,7);
      return;
    }
    PD((DEBT_SEC,"surrender"));
    PendEntry *pe=pendEntryManager->newPendEntry(bs,site);
    debtRec->handler(pe);
    return;
  }
  PD((DEBT_MAIN,"surrender"));
  PendEntry *pe= pendEntryManager->newPendEntry(bs,site);
  be->inDebtMain(pe);
  if(!debtRec->isEmpty()){
    debtRec->handler(pe);
  }
}

void sendRedirect(Site* sd,int OTI,TaggedRef val)
{
  ByteStream *bs= bufferManager->getByteStreamMarshal();
  bs->put(M_REDIRECT);
  marshallNetAddress2(mySite,OTI,bs);
  domarshallTerm(sd,val,bs);
  bs->marshalEnd();
  PD((MSG_SENT,"REDIRECT s:%s o:%d v:%s",pSite(sd),OTI,toC(val)));
  OT->getOwner(OTI)->getOneCredit();

  if (debtRec->isEmpty()) {
    reliableSendFail(sd,bs,FALSE,8);
    return;
  }

  PD((DEBT_SEC,"sendRedirect"));
  PendEntry *pe=pendEntryManager->newPendEntry(bs,sd);
  debtRec->handler(pe);
}

void sendAcknowledge(Site* sd,int OTI)
{
  ByteStream *bs= bufferManager->getByteStreamMarshal();
  bs->put(M_ACKNOWLEDGE);
  marshallNetAddress2(mySite,OTI,bs);
  bs->marshalEnd();
  PD((MSG_SENT,"ACKNOWLEDGE s:%s o:%d",pSite(sd),OTI));

  OT->getOwner(OTI)->getOneCredit();

  reliableSendFail(sd,bs,FALSE,9);
}


void PerdioVar::acknowledge(OZ_Term *ptr)
{
  PD((PD_VAR,"acknowledge"));
  OZ_Term val=u.bindings->val;
  primBind(ptr,val);
  oz_resume(u.bindings->thread);

  PendBinding *tmp=u.bindings->next;
  u.bindings->dispose();
  u.bindings=tmp;
  redirect(val);
}



void PerdioVar::redirect(OZ_Term val) {
  PD((PD_VAR,"redirect v:%s",toC(val)));
  while (u.bindings) {

    RefsArray args = allocateRefsArray(2,NO);
    args[0]=val;
    args[1]=u.bindings->val;
    u.bindings->thread->pushCall(BI_Unify,args,2);
    PD((PD_VAR,"redirect pending unify =%s",toC(u.bindings->val)));
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
    PD((PD_VAR,"bind manager o:%d v:%s",pv->getIndex(),toC(v)));
    pv->primBind(lPtr,v);
    OT->getOwner(pv->getIndex())->mkRef();
    sendRedirect(pv->getProxies(),v,mySite,pv->getIndex());
  } else if (pv->isURL()) {
    PD((PD_VAR,"bind url u:%s",toC(pv->getURL())));
    PD((PD_VAR,"bind url v:%s",toC(v)));
    pv->primBind(lPtr,v);
  } else {
    PD((PD_VAR,"bind proxy b:%d v:%s",pv->getIndex(),toC(v)));
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
  PD((CREDIT,"give back - %d",c));
  ByteStream *bs= bufferManager->getByteStreamMarshal();
  bs->put(M_OWNER_CREDIT);
  marshallNumber(OTI,bs);
  marshallCredit(c,bs);
  bs->marshalEnd();
  PD((MSG_SENT,"OWNER_CREDIT s:%s o:%d c:%d",pSite(sd),OTI,c));
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
    BT->getBorrow(bi)->mkDummyTert();
    return bi;}
  be->addCredit(c);
  return borrowEntryToIndex(be);}

/* --------------------- at Manager ------------------------------------- */

inline void cellSendForward(BorrowEntry *be,Site *dS,int dI,int ctr,
                            Site *mS,int mI){
  PD((MSG_SENT,"MSG_SENT:CELL_FORWARD to s:%s,o:%d ctr:%d msg s:%s o:%d",
     pSite(dS),dI,ctr,pSite(mS),mI));

  ByteStream* bs=bufferManager->getByteStreamMarshal();  // marshal
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
  PD((MSG_SENT,"MSG_SENT:CELL_REMOTEREAD to s:%s o:%d",pSite(cs),si));

  ByteStream* bs=bufferManager->getByteStreamMarshal();   // marshal
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
  PD((MSG_RECEIVED,"MSG_RECEIVED:CELL_GET from s:%s o:%d",pSite(rsite),OTI));

  CellManager* cm=(CellManager*) oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);

  int current=cm->getCurrent();
  int bi=secBorrowInit(rsite,OTI,CELL_CREDIT_SIZE);
  cm->setCurrent(bi);

  if(current==NO_INDEX){                             // shortcut
    PD((CELL,"CELL - shortcut in cellReceiveGet"));
    cellReceiveForward(oe,0,rsite,OTI);
    return;}

  BorrowEntry* be=BT->getBorrow(current);
  NetAddress* na=be->getNetAddress();
  cellSendForward(be,na->site,na->index,cm->getAndInitCtr(),rsite,OTI);
  BT->freeSecBorrow(current);
}

void cellReceiveRead(OwnerEntry* oe,TaggedRef val){
  PD((MSG_RECEIVED,"MSG_RECEIVED:CELL_READ"));
  CellManager *cm=(CellManager*) oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);

  int current=cm->getCurrent();
  if(current==NO_INDEX){            // shortcut
    cellReceiveRemoteRead(oe,val);
    PD((CELL,"CELL- shortcut cellReceiveRead"));
    return;}
  cm->incCtr();
  BorrowEntry *be=borrowTable->getBorrow(current);
  NetAddress *na=be->getNetAddress();
  cellSendRemoteRead(be,na->site,na->index,val);
}

void cellReceiveDump(OwnerEntry *oe,Site *rsite,int OTI){
  PD((MSG_RECEIVED,"MSG_RECEIVED:CELL_DUMP"));
  CellManager *cm=(CellManager*) oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);
  int current=cm->getCurrent();
  Assert(current!=NO_INDEX);

  NetAddress *na=borrowTable->getBorrow(current)->getNetAddress();
  if((na->site!=rsite)|| (na->index!=OTI)){
    PD((WEIRD,"WEIRD- CELL dump out of synch"));
    sendCreditBack(rsite,OTI,1);
    return;}

  CellFrame* cf=(CellFrame *)cm;
  if(cf->getState()!=Cell_Invalid){
    PD((WEIRD,"WEIRD- CELL dump not needed"));
    sendCreditBack(rsite,OTI,1);
    return;}
  Assert(cf->getState()==Cell_Invalid);
  TaggedRef tr=makeTaggedRef(newTaggedUVar(am.currentBoard));
  cellDoExchange((Tertiary *)cf,tr,tr,NULL);
  return;
}

/* --------------------- at Frame ------------------------------------- */

inline void cellSendContents(TaggedRef tr,Site* rsite,int OTI){
  PD((MSG_SENT,"CELL_CONTENTS to s:%s,o:%d from s:%d",pSite(rsite),
     OTI,pSite(mySite)));

  ByteStream *bs=bufferManager->getByteStreamMarshal();    // marshal
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
  PD((MSG_RECEIVED,"CELL_FORWARD to ctr:%d s:%s o:%d",
     ctr,pSite(rsite),OTI));
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
  PD((MSG_RECEIVED,"CELL_REMOTEREAD o:%d",cf->getOwnerIndex()));
  //  Assert(cf->getTertType()==Te_Frame); RSTODO: ask pb
  Assert(cf->getType()==Co_Cell);
  cf->incCtr();
  Assert(cf->getState() & (Cell_Valid | Cell_Requested));
  SiteUnify(val,cf->getContents());
  return;}

void cellReceiveContents(OwnerEntry *oe,TaggedRef val){
  CellFrame *cf=(CellFrame*)oe->getTertiary();
  Assert(cf->getType()==Co_Cell);
  PD((MSG_RECEIVED,"CELL_CONTENTS o:%d",cf->getOwnerIndex()));
  Assert((cf->getTertType()==Te_Frame) || (cf->getTertType()==Te_Manager));
  short  state=cf->getState();
  Assert(state & Cell_Requested);

  PendThread *pt=cf->getPending();
  PendThread *tmp;
  while(pt!=NULL){
    if(pt->thread!=NULL) {
      PD((CELL,"CELL thread activated"));
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
  ByteStream *bs=bufferManager->getByteStreamMarshal();
  bs->put(M_CELL_GET);
  marshallNumber(manI,bs);
  marshallMySite(bs);
  marshallNumber(myI,bs);
  bs->marshalEnd();

  if(be->getOneCredit()){
    PD((MSG_SENT,"CELL_GET to s:%s,o:%d,from o:%d",pSite(manS),manI,myI));
    reliableSendFail(manS,bs,FALSE,27);
    return;}
  PD((MSG_QUEUED,"CELL GET to s:%s,o:%d,from o:%d",pSite(manS),manI,myI));
  PendEntry *pe=pendEntryManager->newPendEntry(bs,manS);
  be->inDebtMain(pe);
  return;}

inline void cellSendRead(BorrowEntry *be,Site *manS,int manI,TaggedRef val){
  ByteStream* bs=bufferManager->getByteStreamMarshal();
  bs->put(M_CELL_READ);
  marshallNumber(manI,bs);
  domarshallTerm(manS,val,bs);
  bs->marshalEnd();
  if(be->getOneCredit()){
    PD((MSG_SENT,"CELL_READ to site:%s,o:%d",pSite(manS),manI));
    reliableSendFail(manS,bs,FALSE,28);
    return;}
  PD((MSG_QUEUED,"CELL_READ to site:%s,o:%d",pSite(manS),manI));
  PendEntry *pe=pendEntryManager->newPendEntry(bs,manS);
  be->inDebtMain(pe);
  return;}

void cellSendDump(CellFrame *cf){
  Assert(cf->getState()==Cell_Valid);
  BorrowEntry* be=BT->getBorrow(cf->getManagerIndex());
  NetAddress *na=be->getNetAddress();
  ByteStream *bs=bufferManager->getByteStreamMarshal();
  bs->put(M_CELL_DUMP);
  marshallNumber(na->index,bs);
  marshallMySite(bs);
  marshallNumber(cf->getOwnerIndex(),bs);
  bs->marshalEnd();
  cf->setState(Cell_Valid | Cell_Dump_Asked);
  OT->getOwner(cf->getOwnerIndex())->getOneCredit();
  if(be->getOneCredit()){
    PD((MSG_SENT,"CELL_DUMP to site:%s,o:%d",pSite(na->site),na->index));
    reliableSendFail(na->site,bs,FALSE,22);
    return;}
  PD((MSG_QUEUED,"CELL_DUMP to site:%s,o:%d",pSite(na->site),na->index));
  PendEntry *pe=pendEntryManager->newPendEntry(bs,na->site);
  be->inDebtMain(pe);
  return;
}

/* --------------------- builtin ------------------------------------- */

inline void e_valid(CellFrame *cf,TaggedRef old,TaggedRef nw){
  Assert(cf->getState() & Cell_Valid);
  TaggedRef c=cf->getContents();
  cf->setContents(nw);
  PD((CELL,"CELL: exchange on VALID o:%d",cf->getOwnerIndex()));
  SiteUnify(c,old);}

inline void e_invalid(CellFrame *cf,TaggedRef old,TaggedRef nw,Thread* th){
  cf->setState(Cell_Requested);
  cf->setHead(old);
  cf->setContents(nw);
  PD((CELL,"CELL: exchange on INVALID o:%d",cf->getOwnerIndex()));
  if (th) {
    oz_stop(th);
    PendThread* pt=new PendThread(th,(PendThread*)NULL);
    cf->setPending(pt);
  }
}

inline void e_req(CellFrame *cf,TaggedRef old,TaggedRef nw,Thread* th){
  TaggedRef tr=cf->getContents();
  cf->setContents(nw);
  PD((CELL,"CELL: exchange on REQUESTED o:%d",cf->getOwnerIndex()));
  if (th) {
    oz_stop(th);
    PendThread* pt=new PendThread(th,cf->getPending());
    cf->setPending(pt);
    SiteUnify(tr,old);
  }
}

TaggedRef cellGetContentsFast(Tertiary *c)
{
  switch (c->getTertType()) {
  case Te_Manager:
    {
      if(!((CellManager*)c)->isOwnCurrent())
        return makeTaggedNULL();
    }
  // no break here
  case Te_Frame:
    {
      CellFrame *cf=(CellFrame *)c;
      if (cf->getState() & Cell_Valid) {
        return cf->getContents();
      }
      break;
    }
  }
  return makeTaggedNULL();
}

void cellDoExchange(Tertiary *c,TaggedRef old,TaggedRef nw,Thread* th){
  TertType tt=c->getTertType();

  if(tt==Te_Proxy){              // convert Proxy to Frame
    CellProxy *cp=(CellProxy *)c;
    OwnerEntry *oe;
    int index=ownerTable->newOwner(oe);
    oe->mkTertiary(c);
    PD((CELL,"CELL: convertToFrame o:%d",index));
    cp->convertToFrame(index);
    tt=Te_Frame;}

  if(tt==Te_Manager){             // Manager
    CellManager *cm=(CellManager*)c;
    CellFrame *cf=(CellFrame *)c;
    if(cm->isOwnCurrent()){
      if(cf->getState() & Cell_Valid){
        PD((CELL,"Exchange: manager own current- valid"));
        e_valid(cf,old,nw);
        return;}
      Assert(cf->getState() & Cell_Requested);
      PD((CELL,"Exchange: manager own current -requested"));
      e_req(cf,old,nw,th);
      return;}

    short state=cf->getState();
    Assert(!(state & Cell_Valid));
    if(state & Cell_Requested){
      PD((CELL,"Exchange: manager has requested"));
      e_req(cf,old,nw,th);
      return;}
    PD((CELL,"Exchange: manager invalid"));
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
    e_valid(cf,old,nw);
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
  PD((CELL,"CELL: access on VALID o:%d",cf->getOwnerIndex()));
  SiteUnify(val,cf->getContents());}

inline void a_req(CellFrame *cf,TaggedRef val){
  PD((CELL,"CELL: access on REQUESTED o:%d",cf->getOwnerIndex()));
  SiteUnify(val,cf->getContents());}


void cellDoAccess(Tertiary *c,TaggedRef val){
  TertType tt=c->getTertType();

  if(tt==Te_Proxy){
    PD((CELL,"CELL: access on PROXY index"));
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

  PD((CELL,"CELL: access on INVALID o:%d",cf->getOwnerIndex()));
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

OZ_C_proc_begin(BIStartSite,2)
{
  OZ_declareIntArg(0,vport);
  OZ_declareArg(1,stream);
  PD((USER,"startSite called vp:%d",vport));
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
  PD((USER,"startSite succeeded"));
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
    PD((USER,"startSite succeeded"));                                     \
  }

OZ_C_proc_begin(BIstartServer,2)
{
  OZ_declareIntArg(0,port);
  OZ_declareNonvarArg(1,prt);

  prt=deref(prt);
  if (!isPort(prt)) {
    oz_typeError(0,"Port");
  }

  PD((USER,"startServer called p:%d",port));

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
  Tertiary *tert=new PortProxy(bi);
  BT->getBorrow(bi)->mkTertiary(tert);
  return makeTaggedConst(tert);
}

OZ_C_proc_begin(BIConnectSite,3){
  OZ_declareVirtualStringArg(0,host);
  OZ_declareIntArg(1,vport);
  OZ_declareArg(2,out);

  PD((USER,"connectSite started vp:%d ho:%s",vport,host));
  if(ozport==0){
    return OZ_raiseC("connectSite",1,OZ_string("startSite first"));}

  Site * sd;
  InterfaceCode ret=connectSiteV(host,vport,sd,FALSE);
  if(ret==NET_OK){
    PD((USER,"connectSite success"));
    OZ_Term x=connect_site_aux(sd);
    return OZ_unify(out,x);} /* TODO */
  if(ret==NET_RAN_OUT_OF_TRIES){
    return OZ_raiseC("connectSite",1,OZ_string("ran out of tries"));}
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIstartClient,3){
  OZ_declareVirtualStringArg(0,host);
  OZ_declareIntArg(1,port);
  OZ_declareArg(2,out);

  INIT_IP(0);

  Site * sd;
  InterfaceCode ret=connectSite(host,port,sd,FALSE);
  if(ret==NET_OK){
    PD((USER,"connectSite success"));
    OZ_Term x=connect_site_aux(sd);
    return OZ_unify(out,x);} /* TODO */
  return OZ_raiseC("connectSite",2,OZ_int(ret),
                   OZ_string(ret==NET_RAN_OUT_OF_TRIES?"ran out of tries":"unknown"));
}
OZ_C_proc_end

OZ_C_proc_begin(BIConnectSiteWait,3){
  OZ_declareVirtualStringArg(0,host);
  OZ_declareIntArg(1,vport);
  OZ_declareArg(2,out);
  PD((USER,"connectSiteWait started vp:%d h:%s",vport,host));
  if(ozport==0){
    return OZ_raiseC("connectSiteWait",1,OZ_string("startSite first"));}

  Site * sd;
  InterfaceCode ret=connectSiteV(host,vport,sd,TRUE);
  if(ret==NET_OK){
    PD((USER,"connectSiteWait success"));
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


enum URL_Protocol {
  URL_UNKNOWN,
  URL_HTTP,
  URL_FTP,
  URL_FILE
};

class URL {
public:
  int protocol;
  char *host;
  int port;
  char *path;

  URL(char *url);
  URL(URL *base, char *url);
  URL(int protocol, char *host, int port, char *path)
    : protocol(protocol), host(host), port(port), path(path)
  {}
};

OZ_C_proc_begin(BIsave,2)
{
  OZ_declareArg(0,in);
  OZ_declareVirtualStringArg(1,url);

  if (strncmp(url,"file:",5)!=0) {
    return oz_raise(E_ERROR,OZ_atom("perdio"),"only \"file:\" allowed",0);
  }

  char *filename = url+5;

  INIT_IP(0);

  ByteStream *bs = bufferManager->getByteStreamMarshal();

  domarshallTerm(OZ_atom(url),in,bs);
  refTrail->unwind();

  bs->marshalEnd();

  int fd = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0666);
  if (fd < 0) {
    return oz_raise(E_ERROR,OZ_atom("perdio"),"open",1,
                    oz_atom(OZ_unixError(errno)));
  }
  bs->beginWrite();
  bs->incPosAfterWrite(tcpHeaderSize);

  int total=bs->calcTotLen();
  while (total){
    Assert(total>0);
    int len=bs->getWriteLen();
    BYTE* pos=bs->getWritePos();
    total -=len;
  again:
    int ret=oswrite(fd,pos,len);
    if (ret < 0) {
      if (errno==EINTR) goto again;
      return oz_raise(E_ERROR,OZ_atom("perdio"),"write",1,
                      oz_atom(OZ_unixError(errno)));
    }
    bs->sentFirst();
  }
  bs->writeCheck();
  bufferManager->freeByteStream(bs);

  close(fd);

  return PROCEED;
}
OZ_C_proc_end

int loadURL(TaggedRef url, OZ_Term &out)
{
  Literal *lit = tagged2Literal(url);
  Assert(lit->isAtom());
  char *s=lit->getPrintName();
  return loadURL(s,out);
}
int loadURL(char *url, OZ_Term &out)
{
  if (strncmp(url,"file:",5)!=0) {
    return oz_raise(E_ERROR,OZ_atom("perdio"),"only \"file:\" allowed",0);
  }

  char *filename = url+5;

  INIT_IP(0);

  int fd = open(filename,O_RDONLY);
  if (fd < 0) {
    return oz_raise(E_ERROR,OZ_atom("perdio"),"open",1,
                    oz_atom(OZ_unixError(errno)));
  }

  ByteStream *bs=bufferManager->getByteStream();
  bs->getSingle();

  int max;
  BYTE *pos=bs->initForRead(max);
  int len=0;

  while (TRUE) {
    int ret=osread(fd,pos,max);
    if (ret < 0) {
      if (errno==EINTR) continue;
      return oz_raise(E_ERROR,OZ_atom("perdio"),"read",1,
                      oz_atom(OZ_unixError(errno)));
    }
    if (ret < max) {
      bs->afterRead(ret);
      break;
    }
    bs->afterRead(max);
    pos=bs->beginRead(max);
  }

  bs->beforeInterpret(0);

  bs->unmarshalBegin();

  out = unmarshallTerm(bs);

  bs->unmarshalEnd();

  bs->afterInterpret();

  bufferManager->freeByteStream(bs);
  return PROCEED;
}

OZ_C_proc_begin(BIload,2)
{
  OZ_declareVirtualStringArg(0,url);
  OZ_declareArg(1,out);

  OZ_Term v;
  int ret=loadURL(url,v);
  if (ret != PROCEED) return ret;

  return OZ_unify(out,v);
}
OZ_C_proc_end

BIspec perdioSpec[] = {
  {"startSite",      2, BIStartSite, 0},
  {"connectSite",    3, BIConnectSite, 0},
  {"connectSiteWait",3, BIConnectSiteWait, 0},

  {"startServer",    2, BIstartServer, 0},
  {"startClient",    3, BIstartClient, 0},

  {"save",    2, BIsave, 0},
  {"load",    2, BIload, 0},

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

  currentURL=0;
}


#endif
