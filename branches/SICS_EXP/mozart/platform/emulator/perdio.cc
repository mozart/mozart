#line 1 "perdio.m4cc"
/* -----------------------------------------------------------------------
 *  (c) Perdio Project, DFKI & SICS
 *  Universit"at des Saarlandes
 *    Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
 *  SICS
 *    Box 1263, S-16428 Sweden, Phone (+46) 8 7521500
 *  Author: brand, scheidhr, mehl
 *
 *  protocol and message layer
 * -----------------------------------------------------------------------*/

/* -----------------------------------------------------------------------
 * TODO

 * --- high priority ---
 *   perdio prelude
 *     Site.init: setMethHdl, setSendHdl, ....
 *   dictionary, array
 *   builtin
 *     classify secure/insecure
 *   ip
 *     cache does not work together with close detection
 *     errors
 *   save
 *     saving proxies for resources raises exception
 *     url=unit: save without support
 *     isGround: save and check that no variables saved
 *     getUrls:  save and look at the urls
 *     isFixed: save and check that resources empty
 *     IDEA: implement {Flatten X ?Xs} Xs is a list of a reachable nodes
 *   Code
 *     must have global name
 *   Unification
 *     must stop thread immediately
 *   URL
 *     implementation for HTTP and FTP protocol
 *   Failure handling

 * --- medium priority ---
 *   threads
 *   port close
 *   spaces
 *     Space.ask[Verbose] may suspend?
 *     Space.merge may fail.
 *     Space.clone may suspend
 *     Space.clone should return a local clone
 *     Space.choose may fail, suspend
 *     Space.inject may fail, suspend
 *   gname
 *     use ip class Site
 *   marshal/unmarshal
 *     ref technique for Site*

 * --- low priority ---
 *   pendlinkHandle simplify
 *   ip
 *     fairness for IO
 *     flow control
 *     timer
 *     unregisterAccept
 *        The reason is that one site may have a full cache of connections
 *        and does not want to accept any more until it can close some other
 *        connections. 
 *     void OZ_registerTimer(int ms,OZ_TimeHandler,void *);
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
 *   how to interface I/O - emulator?
 *     IDEA: create OS threads
 *           communicate via shared memory

 * -----------------------------------------------------------------------*/

/* *****************************************************************************
   *****************************************************************************
                       ORGANIZATION

	    1  forward declarations
	    2  utility routines
	    3  message 
	    4  ProtocolObject
            5  GNames 
            6  Owner/Borrower common
	    7  NetAddress & NetHashTable
	    8  OwnerEntry
	    9  OwnerTable
	    10 BorrowEntry
	    11 BorrowTable
	    12 Thread control utility routines
	    13 garbage collection
	    14 globalizing
	    15 localizing
	    16 marshaling/unmarshaling by protocol-layer
	    17 main receive msg
	    18 protocol utility routines
	    19 remote-send protocol
	    20 port protocol
	    21 variable protocol
	    22 object protocol
	    23 credit protocol
	    24 cell prototocol
	    25 lock protocol
	    26 builtins
	    27 initialization

   *****************************************************************************
   ***************************************************************************** */

/* ATTENTION pragma */

#include "wsock.hh"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include "runtime.hh"
#include "codearea.hh"
#include "indexing.hh"

#include "perdio_debug.hh"  
#include "perdio_debug.cc"  
#include "genvar.hh"
#include "perdiovar.hh"
#include "gc.hh"
#include "dictionary.hh"
#include "urlc.hh"
#include "marshaler.hh"
#include "comm.hh"
#include "msgbuffer.hh"
#include "perdio.hh"

#line 1 "marshal.m4"

#line 3


#line 6



            
#line 149 "perdio.m4cc"


/* *********************************************************************/
/*   SECTION 1: forward declarations                                  */
/* *********************************************************************/

class BorrowTable;
class OwnerTable;
class MsgBuffer;
class FatInt;

void sendSurrender(BorrowEntry *be,OZ_Term val);
void sendRedirect(Site* sd,int OTI,TaggedRef val);
void sendAcknowledge(Site* sd,int OTI);
void sendRedirect(ProxyList *pl,OZ_Term val, Site* ackSite,int OTI);
void bindPerdioVar(PerdioVar *pv,TaggedRef *lPtr,TaggedRef v);
void sendCreditBack(Site* sd,int OTI,Credit c);
inline void reliableSendFail(Site*,ByteStream *,Bool,int);

Bool cellReceiveContentsManager(CellManager*,TaggedRef,int);
Bool cellReceiveContentsFrame(CellFrame*,TaggedRef,Site*,int);
Bool cellReceiveGet(CellManager*,int,Site*);
Bool cellReceiveRead(CellManager*,int,TaggedRef);
Bool cellReceiveForward(CellFrame*,int,Site*,Site*,int);
void cellReceiveRemoteRead(CellFrame*,TaggedRef);
void cellReceiveDump(CellManager*,Site *);

void cellSendDump(BorrowEntry*,CellFrame*);

Bool lockReceiveGet(LockManager*,int,Site*);
Bool lockReceiveForward(LockFrame*,Site *,Site*,int);
void lockReceiveDump(LockManager*,Site*);
void lockReceiveLock(LockFrame*);

void lockSendDump(BorrowEntry*,LockFrame*);

void sendRegister(BorrowEntry *);

OZ_C_proc_proto(BIapply);
extern TaggedRef BI_Unify;
extern TaggedRef BI_Show;

void sendObject(Site* sd, Object *o, Bool);

PERDIO_DEBUG_DO(void printTables());

#define OT ownerTable
#define BT borrowTable
#define BTOS(A) BT->getOriginSite(A)
#define BTOI(A) BT->getOriginIndex(A)

Bool withinBorrowTable(int i); // for assertion

/* *********************************************************************/
/*   SECTION 1: global variables                                       */
/* *********************************************************************/

// global variables
TaggedRef currentURL;      // ATTENTION
BorrowTable *borrowTable;
OwnerTable *ownerTable;
FatInt *idCounter;
MsgBufferManager* msgBufferManager= new MsgBufferManager();
Site* mySite;  // known to network-layer also 
Site* creditSite;

SendRecvCounter mess_counter[M_LAST];

char *mess_names[M_LAST] = {
  "port_send",
  "remote_send",
  "ask_for_credit",
  "owner_credit",
  "borrow_credit",
  "register",
  "redirect",
  "acknowledge",
  "surrender",
  "cell_get",
  "cell_contents",
  "cell_read",
  "cell_remoteread",
  "cell_forward",
  "cell_dump",
  "lock_get",
  "lock_sent",
  "lock_forward",
  "lock_dump",
  "get_object",
  "get_objectandclass",
  "send_object",
  "send_objectandclass",
  "file",
  "register virtual site",
  "init virtual site"
};

/* *********************************************************************/
/*   SECTION 2:: Utility routines                                      */
/* *********************************************************************/

void pushUnify(Thread *t, TaggedRef t1, TaggedRef t2)
{
  RefsArray args = allocateRefsArray(2,NO); // with default priority
  args[0]=t1;
  args[1]=t2;
  t->pushCall(BI_Unify,args,2);
}

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
  pushUnify(th,val1,val2);
  am.scheduleThread(th);
}

void SiteUnifyCannotFail(TaggedRef val1,TaggedRef val2){
  SiteUnify(val1,val2); // ATTENTION
}

/* *********************************************************************/
/*   SECTION 3::  class PendThread                                     */
/* *********************************************************************/

class PendThread{
public:
  PendThread *next;
  Thread *thread;
  PendThread(Thread *th,PendThread *pt):next(pt), thread(th) {}
  USEFREELISTMEMORY;
  void dispose(){freeListDispose(this,sizeof(PendThread));}
};

/**********************************************************************/
/*   SECTION 4:: class ProtocolObject                                 */
/**********************************************************************/


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
  ProtocolObject()            { DebugCode(type=(PO_TYPE)4711; u.ref=0x5b5b5b5b;)}
  Bool isTertiary()           { return type==PO_Tert; }
  Bool isRef()                { return type==PO_Ref; }
  Bool isVar()                { return type==PO_Var; }
  Bool isFree()               { return type==PO_Free; }
  void setFree()              { type = PO_Free; }
  void unsetFree()            { DebugCode(type=(PO_TYPE)4712); }
  void mkTertiary(Tertiary *t){ type = PO_Tert; u.tert=t; }
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
      u.tert=(Tertiary *)(u.tert->gcConstTerm()); }
    else {
      Assert(isRef() || isVar());
      PD((GC,"OT var/ref"));
      gcTagged(u.ref,u.ref);}
  }

  ProtocolObject &operator =(ProtocolObject &n); // not used

  TaggedRef getValue() {
    if (isTertiary()) {return makeTaggedConst(getTertiary());}
    else {return getRef();}
  }
};

/* ********************************************************************** */
/*   SECTION 5::  GNames                                                  */
/* ********************************************************************** */


#define GNAME_HASH_TABLE_DEFAULT_SIZE 500

class GNameTable: public GenHashTable{
  friend TaggedRef findGName(GName *gn);
private:
  int hash(GName *);
  TaggedRef find(GName *name);
public:
  void add(GName *name);
  GNameTable():GenHashTable(GNAME_HASH_TABLE_DEFAULT_SIZE) {}
  
  void gcGNameTable();
} theGNameTable;

int GNameTable::hash(GName *gname)
{
  int ret = gname->site->hashSecondary();
  for(int i=0; i<fatIntDigits; i++) {
    ret += gname->id.number[i];
  }
  return ret<0?-ret:ret;
}

inline void GNameTable::add(GName *name)
{
  int hvalue=hash(name);
  GenHashTable::htAdd(hvalue,(GenHashBaseKey*)name,0);
}

TaggedRef GNameTable::find(GName *name)
{
  int hvalue = hash(name);
  GenHashNode *aux = htFindFirst(hvalue);
  while(aux) {
    GName *gn = (GName*)aux->getBaseKey();
    if (name->same(gn)) {
      return gn->getValue();
    }
    aux = htFindNext(aux,hvalue); }
  return makeTaggedNULL();
}


inline TaggedRef findGName(GName *gn) {
  return theGNameTable.find(gn);
}

inline void addGName(GName *gn) {
  Assert(!findGName(gn));
  theGNameTable.add(gn);
}

void addGName(GName *gn, TaggedRef t) {
  gn->setValue(t);
  addGName(gn);
}

GName *newGName(TaggedRef t, GNameType gt)
{
  GName* ret = new GName(mySite,gt,t);
  addGName(ret);
  return ret;
}

void deleteGName(GName *gn)
{
  delete gn;
}

GName *newGName(PrTabEntry *pr)
{
  GName *ret = newGName(ToInt32(pr),GNT_CODE);
  return ret;
}


PrTabEntry *findCodeGName(GName *gn)
{
  TaggedRef aux = findGName(gn);
  if (aux) {
    return (PrTabEntry*) ToPointer(aux);
  }
  return NULL;
}

/* ********************************************************************** */
/*   SECTION 6:: Owner/Borrower common                                    */
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
#define BORROW_MIN               (2)
#define BORROW_GIVE_CREDIT_SIZE  (4)
#define BORROW_LOW_THRESHOLD     (8)
#define BORROW_HIGH_THRESHOLD    (64)

#else

#define START_CREDIT_SIZE        ((1<<31)-1)
#define OWNER_GIVE_CREDIT_SIZE   ((1<<15))
#define BORROW_MIN               (2)
#define BORROW_GIVE_CREDIT_SIZE  ((1<<7))
#define BORROW_LOW_THRESHOLD     ((1<<12))
#define BORROW_HIGH_THRESHOLD    ((1<<24))
#endif

#define BTRESIZE_CRITICAL

#ifdef BTRESIZE_CRITICAL
#define DEFAULT_OWNER_TABLE_SIZE   5000
#define DEFAULT_BORROW_TABLE_SIZE  5000
#else
#define DEFAULT_OWNER_TABLE_SIZE   100
#define DEFAULT_BORROW_TABLE_SIZE  100
#endif

static double TABLE_LOW_LIMIT=0.20;
static double TABLE_EXPAND_FACTOR=2.00;

#define TABLE_BUFFER 50
#define TABLE_WORTHWHILE_REALLOC 200

class OB_Entry : public ProtocolObject {
protected:
  union {
    Credit credit;
    int nextfree;
  } uOB;

  void makeFree(int next) {setFree(); uOB.nextfree=next;}

  int getNextFree(){
    Assert(isFree());
    return uOB.nextfree;  }

  void setCredit(Credit c) {uOB.credit=c;}

  void addToCredit(Credit c) {
    Assert(c!=INFINITE_CREDIT);
    Assert(!isPersistent());
    uOB.credit +=c;
  }

  void subFromCredit(Credit c) {
    Assert(c!=INFINITE_CREDIT);
    Assert(!isPersistent());
    uOB.credit -=c;
  }

  void makePersistent() {setCredit(INFINITE_CREDIT);} 

public:


  void print();
  Credit getCredit(){Assert(!isFree());return uOB.credit;} 
  Bool isPersistent()   { return uOB.credit==INFINITE_CREDIT; }
};

/* ********************************************************************** */
/*   SECTION 7: NetAddress & NetHashTable                                 */
/* ********************************************************************** */

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

#define NET_HASH_TABLE_DEFAULT_SIZE 100

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

inline NetAddress * GenHashNode2NetAddr(GenHashNode *ghn){
  NetAddress *na;
  GenCast(ghn->getBaseKey(),GenHashBaseKey*,na,NetAddress*);
  return na;}

inline int GenHashNode2BorrowIndex(GenHashNode *ghn){
  int i;
  GenCast(ghn->getEntry(),GenHashEntry*,i,int);
  Assert(i>=0);
  Assert(withinBorrowTable(i));
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
      printf("<%d> - s%s o:%d\n",i,na->site->stringrep(),na->index);
      ghn=ghn->getNext();
      while(ghn!=NULL){
	na=GenHashNode2NetAddr(ghn);
	printf("<coll> - s:%s o:%d\n",na->site->stringrep(),na->index);
	ghn=ghn->getNext();}}}
  printf("-----------------------------------\n");
}
#endif
  
/* ********************************************************************** */
/*   SECTION 8:: OwnerEntry                                               */  
/* ********************************************************************** */

class OwnerEntry: public OB_Entry {
friend class OwnerTable;
private:

  Credit upcredit;

  Credit requestCredit(Credit req){
    if (isPersistent()) 
      {return INFINITE_CREDIT;}
    Credit c=getCredit();
    if(c <= req) {
      if(upcredit==0){
	Assert(0);  // check this
	makePersistent();
	return req;}
      upcredit--;
      setCredit(c+START_CREDIT_SIZE);
      requestCredit(req);}
    setCredit(c-req);
    return req;}

  void addToCredit(Credit back){
    Credit c=getCredit();
    if(c+back>START_CREDIT_SIZE){
      upcredit++;
      Assert(upcredit<=START_CREDIT_SIZE);
      setCredit(c+back-START_CREDIT_SIZE);
      return;}
    setCredit(c+back);}

public:

  void makePersistentOwner() { makePersistent();}

  void returnCredit(Credit c) {
    if (isPersistent()) return;
    addToCredit(c);}

  Bool hasFullCredit()     { 
    Assert(getCredit()<=START_CREDIT_SIZE);
    Credit c=getCredit();
    if(c<START_CREDIT_SIZE) return NO;
    if(upcredit<START_CREDIT_SIZE) return NO;
    return OK;}
    
  Credit getSendCredit() { 
    return requestCredit(OWNER_GIVE_CREDIT_SIZE);}

  void getOneCreditOwner() { 
    requestCredit(1);
    return;}

  Credit giveMoreCredit() { 
    return requestCredit(OWNER_GIVE_CREDIT_SIZE); }
}; 

/* ********************************************************************** */
/*   SECTION 9:: OwnerTable                                               */  
/* ********************************************************************** */

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

  OwnerEntry *getOwner(int i)  { Assert(i>=0 && i<size); return &array[i];}
  
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
      base=&array[i].uOB.nextfree;}
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
  nextfree = array[index].uOB.nextfree;
  oe = (OwnerEntry *)&(array[index]);
  oe->setCredit(START_CREDIT_SIZE);
  PD((TABLE,"owner insert: o:%d",index));
  no_used++;
  return index;}

void OwnerTable::freeOwnerEntry(int i){
  array[i].setFree();
  array[i].uOB.nextfree=nextfree;
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
      printf("FREE: next:%d\n",array[i].uOB.nextfree);}}
  printf("-----------------------------------------------\n");  
}
#endif

/* ********************************************************************** */
/*   SECTION 10:: BorrowEntry                                              */
/* ********************************************************************** */

#define BORROW_GC_MARK       1
#define ASK_MARK             2

// START_CREDIT_SIZE
// BORROW_HIGH_THRESHOLD
// BORROW_LOW_THRESHOLD               
// BORROW_GIVE_CREDIT_SIZE
// BORROW_MIN

class BorrowEntry: public OB_Entry {
friend class BorrowTable;
private:
  NetAddress netaddr;
  unsigned int extension;
  
  Bool hasExtension(){return extension & (~(BORROW_GC_MARK | ASK_MARK));}

public:

  void print();
  void makeMark(){ extension |= BORROW_GC_MARK;}
  Bool isMarked(){ return (extension & BORROW_GC_MARK); }
  void removeMark(){extension = extension & (~BORROW_GC_MARK);}

  void makeMark(int i){ extension |= i;}
  Bool isMarked(int i){ return (extension & i);}
  void removeMark(int i){extension = extension & (~i);}

  
  void gcBorrow1(int);
  void gcBorrow2(int);

  inline void copyBorrow(BorrowEntry* from,int i){
    setCredit(from->getCredit());
    if (from->isTertiary()) {
      mkTertiary(from->getTertiary());
      from->getTertiary()->setIndex(i);
    } else if (from->isVar()) {
      mkVar(from->getRef());
      from->getVar()->setIndex(i);
    } else {
      Assert(from->isRef());
      mkRef(from->getRef());
    }
    extension=from->extension;
    netaddr.set(from->netaddr.site,from->netaddr.index);
  }

  void initBorrow(Credit c,Site* s,int i){
    Assert(isFree());
    setCredit(c);
    unsetFree();
    extension=0;
    netaddr.set(s,i);
    return;}

  NetAddress* getNetAddress() {
    Assert(!isFree());
    return &netaddr;}

  Site *getSite(){return netaddr.site;}
  int getOTI(){return netaddr.index;}

  void addCredit(Credit c){
    if (isPersistent()) return;
    if (c==INFINITE_CREDIT) {
      makePersistentBorrow();
      return;}
    Credit cur=getCredit();
    PD((CREDIT,"borrow add s:%s o:%d add:%d to:%d",getNetAddress()->site->stringrep(),
	getNetAddress()->index,c,cur));
    if(cur>BORROW_HIGH_THRESHOLD){
      giveBackCredit(cur-BORROW_HIGH_THRESHOLD);
      setCredit(BORROW_HIGH_THRESHOLD);
      return;}
    setCredit(cur+c);}
    
  Credit getOnePrimaryCredit() { 
    if(hasExtension()) return 0;
    if (isPersistent()) {return INFINITE_CREDIT;}
    Credit c=getCredit();
    Assert(c>0);
    if(c-1< BORROW_MIN) {
      PD((CREDIT,"getOnePrimaryCredit failed"));      
      return 0;}
    if(c-1 < BORROW_LOW_THRESHOLD){
      needMoreCredit();}
    PD((CREDIT,"getOnePrimaryCredit OK"));
    subFromCredit(1);
    return 1;}

  Site* getOneSecondaryCredit(){   
    Assert(0);
    error("not implemented");
    return NULL;}

  Credit getSmallPrimaryCredit(){
    if(hasExtension()) return 0;
    if(isPersistent()) {return INFINITE_CREDIT;}
    Credit cur=getCredit();
    if(cur-BORROW_GIVE_CREDIT_SIZE < BORROW_MIN) {
      if(cur < 2* BORROW_MIN) {return 0;}
      needMoreCredit();
      subFromCredit(BORROW_MIN);
      return BORROW_MIN;}
    if(cur-BORROW_GIVE_CREDIT_SIZE < BORROW_LOW_THRESHOLD) {
      needMoreCredit();
      subFromCredit(BORROW_GIVE_CREDIT_SIZE);
      return BORROW_GIVE_CREDIT_SIZE;}
    subFromCredit(BORROW_GIVE_CREDIT_SIZE);
    return BORROW_GIVE_CREDIT_SIZE;}

  Credit getSmallSecondaryCredit(Site* &s){
    Assert(0);
    return 0;}

  void explicitReceivedCredit(){ removeMark(ASK_MARK);}

  void freeBorrowEntry();    
  void giveBackCredit(Credit c);
  void needMoreCredit();
  void moreCredit();

  void makePersistentBorrow() {
    if(hasExtension()) {Assert(0);}
    makePersistent();}

  void makePersistentBorrowXX() {Assert(0);error("dont understand");}
};


void BorrowEntry::moreCredit(){
  if(!(isMarked(ASK_MARK))) return;
  removeMark(ASK_MARK);
  if(isPersistent()) return;
  NetAddress *na = getNetAddress();
  
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  
#line 1034
	marshal_M_ASK_FOR_CREDIT(bs,na->index ,mySite   );
  int ret=na->site->sendTo(bs,M_ASK_FOR_CREDIT,NULL,0); 
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemented");
}

void BorrowEntry::giveBackCredit(Credit c){
  Assert(!isPersistent());
  NetAddress *na = getNetAddress();
  Site* site = na->site;
  int index = na->index;
  sendCreditBack(site,index,c);
}

void BorrowEntry::freeBorrowEntry(){
  if (!isPersistent()) {giveBackCredit(getCredit());}
  getNetAddress()->site->dec();}

void BorrowEntry::needMoreCredit(){
  Assert(0);
  error("not implemented");
  return;}

/* ********************************************************************** */
/*   SECTION 11:: BorrowTable                                              */
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

  BorrowEntry *getBorrow(int i)  { Assert(i>=0 && i<size); return &array[i];}

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

  void gcBorrowTable1();
  void gcBorrowTable2();
  void gcBorrowTable3();
  void gcFrameToProxy();

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
    array[i].uOB.nextfree=i+1;
    array[i].setFree();
    i++;}
  i--;
  array[i].uOB.nextfree=nextfree;
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
  sd->inc();
  if(nextfree == END_FREE) resize();
  int index=nextfree;
  nextfree= array[index].uOB.nextfree;
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
  b->freeBorrowEntry();  
  hshtbl->sub(getBorrow(index)->getNetAddress());
  b->setFree();
  b->uOB.nextfree=nextfree;
  nextfree=index;
  no_used--;
  PD((TABLE,"borrow delete: b:%d",index));
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


void OB_Entry::print() {
  printf("Credit:%ld\n",getCredit());
}

void BorrowEntry::print() {
  OB_Entry::print();
  NetAddress *na=getNetAddress();
  printf("NA: s:%s o:%d\n",na->site->stringrep(),na->index);
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
      printf("<%d> FREE: next:%d\n",i,array[i].uOB.nextfree);}}
  printf("-----------------------------------------------\n");  
}

#endif

#ifdef DEBUG_PERDIO

void printTables(){
  ownerTable->print();
  borrowTable->print();
  borrowTable->hshtbl->print();}

void resize_hash(){
  borrowTable->hshtbl->print();}

#endif

/* ******************************************************************* */
/*   SECTION 12 :: pending thread utility routines                     */
/* ******************************************************************* */

#define DummyThread ((Thread*)0x1)
#define MoveThread  ((Thread*)NULL)

inline Bool isRealThread(Thread* t){
  if((t==MoveThread) || (t==DummyThread)) return FALSE;
  return TRUE;}

inline void pendThreadResumeAll(PendThread *pt){
  PendThread *tmp;
  while(pt!=NULL){
    Thread *t=pt->thread;
    Assert(t!=MoveThread);
    if(isRealThread(t)){
      PD((THREAD_D,"start thread ResumeAll %x",t));
      oz_resume(t);}
    tmp=pt;
    pt=pt->next;
    tmp->dispose();}}

inline Thread* pendThreadResumeFirst(PendThread **pt){
  PendThread *tmp=*pt;
  Assert(tmp!=NULL);
  Thread *t=tmp->thread;
  Assert(isRealThread(t));
  PD((THREAD_D,"start thread ResumeFirst %x",t));
  oz_resume(t);
  *pt=tmp->next;
  tmp->dispose();
  return t;}

inline void pendThreadRemoveFirst(PendThread **pt){
  PendThread *tmp=*pt;
  Assert(tmp!=NULL);
  Assert(!isRealThread(tmp->thread));
  *pt=tmp->next;  
  tmp->dispose();}

inline void pendThreadAddToEnd(PendThread **pt,Thread *t){
  if(isRealThread(t)){
    oz_stop(t);
    PD((THREAD_D,"stop thread addToEnd %x",t));}
  while(*pt!=NULL){pt= &((*pt)->next);}
  *pt=new PendThread(t,NULL);
  return;}

inline void pendThreadAddToNonFirst(PendThread **pt,Thread *t){
  if(isRealThread(t)){
    oz_stop(t);
    PD((THREAD_D,"stop thread addToNonFirst %x",t));}
  if(*pt!=NULL){pt= &((*pt)->next);}
  *pt=new PendThread(t,NULL);
  return;}

void sendHelpX(MessageType mt,BorrowEntry *be){
  NetAddress* na=be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  if(mt==M_GET_OBJECT){
    
#line 1336
	marshal_M_GET_OBJECT(bs,na->index ,mySite   );}
  else{
    Assert(mt==M_GET_OBJECTANDCLASS);
    
#line 1339
	marshal_M_GET_OBJECTANDCLASS(bs,na->index ,mySite   );}
  int ret=na->site->sendTo(bs,mt,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implmented");
  return;}

void PerdioVar::addSuspPerdioVar(Thread *el, int unstable)
{
  if (suspList!=NULL) {
    addSuspSVar(el,unstable);
    return;
  }

  addSuspSVar(el,unstable);

  if (isObjectGName()) {
    MessageType mt; 
    if(findGName(getGNameClass())==0) {mt=M_GET_OBJECTANDCLASS;}
    else {mt=M_GET_OBJECT;}
    BorrowEntry *be=BT->getBorrow(getObject()->getIndex());
    sendHelpX(mt,be);
    return;}
  
  if (isObjectURL()) {
    TaggedRef cl=deref(getClass());
    if (isPerdioVar(cl)) {
      PerdioVar *pv=tagged2PerdioVar(cl);
      Assert(pv->isURL());
      pv->addSuspPerdioVar(el,unstable);
    } else {
      Assert(isClass(cl));
      BorrowEntry *be=BT->getBorrow(getObject()->getIndex());      
      sendHelpX(M_GET_OBJECT,be);
    }
    return;
  }

  if (isURL()) {
    OZ_Return ret = loadURL(getURL(),oz_newVariable());
    // BI_PREEMPT is returned when  we fall through to the default
    // method of starting an independent loader process
    if (ret != PROCEED && ret != BI_PREEMPT) {
      warning("mm2: load URL %s failed not impl",toC(getURL()));
    }
 }
}

/* ******************************************************************* */
/*   SECTION 13::  garbage-collection                                  */
/* ******************************************************************* */

/* OBS: ---------- interface to gc.cc ----------*/

void gcOwnerTable()       { ownerTable->gcOwnerTable();}
void gcBorrowTable3()     { borrowTable->gcBorrowTable3();}
void gcBorrowTable2()     { borrowTable->gcBorrowTable2();}
void gcBorrowTable1()     { borrowTable->gcBorrowTable1();}
void gcGNameTable()       { theGNameTable.gcGNameTable();}
void gcGName(GName* name) { if (name) name->gcGName(); }
void gcFrameToProxy()     {borrowTable->gcFrameToProxy();}



void Tertiary::gcProxy(){
  int i=getIndex();
  BorrowEntry *be=BT->getBorrow(i);
  if(be->isMarked()){
    PD((GC,"borrow already marked:%d",i));
    return;}
  be->makeMark();
  PD((GC,"relocate borrow :%d old:%x new:%x",i,be,this));
  if (be->isTertiary())  /* might be avariable for an object */
    be->mkTertiary(this);
  return;}

void Tertiary::gcManager(){
  Assert(getTertType()!=Te_Frame);
  int i=getIndex();
  PD((GC,"relocate owner:%d old%x new %x",
     i,ownerTable->getOwner(i),this));
  ownerTable->getOwner(i)->mkTertiary(this);}

/* pendThread inlines */

void gcPendThread(PendThread **pt){
  PendThread *tmp;
  while(*pt!=NULL){
    if(((*pt)->thread == NULL) || ((*pt)->thread== (Thread*) 0x1)){
      tmp=new PendThread((*pt)->thread,(*pt)->next);}
    else{
      tmp=new PendThread((*pt)->thread->gcThread(),(*pt)->next);}
    *pt=tmp;
    pt=&(tmp->next);}}
  
void CellFrame::gcCellFrameSec(){
  int state=getState();
  PD((GC,"relocate Cell in state %d",state));
  if(state & Cell_Requested){
    gcTagged(sec->contents,sec->contents);
    gcTagged(sec->head,sec->head);
    gcPendThread(&sec->pending);
    return;}
  else{
    if(state & Cell_Valid){gcTagged(sec->contents,sec->contents);}}
  return;}

void CellFrame::gcCellFrame(){
  Tertiary *t=(Tertiary*)this;
  t->gcProxy();
  PD((GC,"relocate cellFrame:%d",t->getIndex()));
  gcCellFrameSec();}
  
void CellManager::gcCellManager(){
  int i=getIndex();
  PD((GC,"relocate cellManager:%d",i));
  OT->getOwner(i)->mkTertiary(this);
  CellFrame *cf=(CellFrame*)this;
  cf->gcCellFrameSec();}

void LockFrame::gcLockFrameSec(){
  int state=getState();
  PD((GC,"relocate Lock in state %d",state));
  if(sec->pending!=NULL){
    gcPendThread(&sec->pending);
    return;}
  else{
    if(state & Cell_Valid){
      setLocker(getLocker()->gcThread());}
    return;}}

void LockFrame::gcLockFrame(){
  Tertiary *t=(Tertiary*)this;
  t->gcProxy();
  PD((GC,"relocate lockFrame:%d",t->getIndex()));
  gcLockFrameSec();}

void LockManager::gcLockManager(){
  int i=getIndex();
  PD((GC,"relocate lockManager:%d",i));
  OT->getOwner(i)->mkTertiary(this);
  LockFrame *lf=(LockFrame*)this;
  lf->gcLockFrameSec();}


void Tertiary::gcTertiary() 
{
  switch (getTertType()) {

  case Te_Local:
    return;

  case Te_Proxy:
    {
      int i=getIndex();
      BorrowEntry *be = borrowTable->getBorrow(i);
      be->makeMark();
      PD((GC,"relocate borrow:%d old %x new %x",i,be,this));
      be->mkTertiary(this);
      return;
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

void BorrowEntry::gcBorrow1(int i) {
  if (isVar()) {
    PD((GC,"BT1 b:%d variable found",i));
    PerdioVar *pv=getVar();
    if (pv->getSuspList() || (pv->isProxy() && pv->hasVal())) {
      PD((WEIRD,"BT1 b:%d pending unmarked var found",i));
      makeMark();
      gcPO();
    }
    return;
  }
}

void BorrowTable::gcBorrowTable1()
{
  PD((GC,"borrowTable1 gc"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if (!b->isFree()){
      if (!b->isMarked()) {b->gcBorrow1(i);}}
  }
}

void BorrowEntry::gcBorrow2(int i) {
  if(isTertiary() && getTertiary()->getTertType()==Te_Frame)
    {u.tert= (Tertiary*) u.tert->gcConstTermSpec();}
}

void BorrowTable::gcBorrowTable2()
{
  PD((GC,"borrow gc roots"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if(!b->isFree()){
      Assert((b->isVar()) || (b->getTertiary()->getTertType()==Te_Frame) 
	     || (b->getTertiary()->getTertType()==Te_Proxy));
      if(!(b->isMarked())) {b->gcBorrow2(i);}}
  }
}

void BorrowTable::gcFrameToProxy()
{
  PD((GC,"borrow frame to proxy"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if((!b->isFree()) && (!b->isVar())){
      Tertiary *t=b->getTertiary();
      if(t->getTertType()==Te_Frame){
	if((t->getType()==Co_Cell) && ((CellFrame*)t)->getState()==Cell_Invalid){
	  ((CellFrame*)t)->convertToProxy();}
	else{
	  if((t->getType()==Co_Lock) && ((LockFrame*)t)->getState()==Lock_Invalid){
	    ((LockFrame*)t)->convertToProxy();}}}}
  }
}

extern TaggedRef gcTagged1(TaggedRef in);

/* OBSERVE - this must done at the end of other gc */
void BorrowTable::gcBorrowTable3()
{
  PD((GC,"borrow gc"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if (!b->isFree()) {
      if(b->isVar()){
	if(b->isMarked()) {
	  b->removeMark();
	  PD((GC,"BT b:%d mark variable found",i));}
	else{
	  PD((GC,"BT b:%d unmarked variable found",i));
	  borrowTable->maybeFreeBorrowEntry(i);}}
      else{
	if(b->isMarked()){
	  b->removeMark();
	  PD((GC,"BT b:%d mark tertiary found",i));
	  Assert(b->isTertiary());
	  Tertiary *t=b->getTertiary();
	  if(t->getTertType()==Te_Frame){
	    switch(t->getType()){
	    case Co_Cell:{
	      CellFrame *cf=(CellFrame *)t;
	      if(cf->isAccessBit()){
		short state=cf->getState();
		cf->resetAccessBit();
		if((state & Cell_Valid) && (!(state & Cell_Dump_Asked))){cellSendDump(b,cf);}}
	      break;}
	    case Co_Lock:{
	      LockFrame *lf=(LockFrame *)t;
	      if(lf->isAccessBit()){
		int state=lf->getState();
		lf->resetAccessBit();
		if((state & Lock_Valid) && (!(state & Lock_Dump_Asked))){lockSendDump(b,lf);}}
	      break;}
	    default:{
	      Assert(0);
	      break;}}}}
	else{
	    Assert(b->getTertiary()->getTertType()==Te_Proxy);	    
	    borrowTable->maybeFreeBorrowEntry(i);}}}}
  compactify();
  hshtbl->compactify();
}

/* OBSERVE - this must be done at the end of other gc */
void GNameTable::gcGNameTable()
{
  PD((GC,"gname gc"));
  int index;
  GenHashNode *aux = getFirst(index);
  DebugCode(int used = getUsed());
  while (aux!=NULL) {
    GName *gn = (GName*) aux->getBaseKey();

    DebugCode(used--);

    /* code is never garbage collected */
    if (gn->getGNameType()==GNT_CODE)
      goto next_one;

    if (gn->getGCMark()) {
      gn->resetGCMark();
    } else {
      if (gn->getGNameType()==GNT_NAME &&
	  tagged2Literal(gn->getValue())->isNamedName()) {
	goto next_one;
      }
      gn->site->dec();
      delete gn;
      if (!htSub(index,aux)) 
	continue;
    }
  next_one:
    aux = getNext(aux,index);
  }

  Assert(used==0);
  compactify();
}

extern void dogcGName(GName *gn);

void gcBorrowNow(int i)
{
  BorrowEntry *be=borrowTable->getBorrow(i);
  if (!be->isMarked()) { // this condition is necessary gcBorrow1
    be->makeMark();
    be->gcPO();
  }
}

void PerdioVar::gcPerdioVar(void)
{ 
  if (isProxy()) {
    PD((GC,"PerdioVar b:%d",getIndex()));
    gcBorrowNow(getIndex());
    PendBinding **last = &u.bindings;
    for (PendBinding *bl = u.bindings; bl; bl = bl->next) {
      PendBinding *newBL = new PendBinding();
      gcTagged(bl->val,newBL->val);
      newBL->thread = bl->thread->gcThread();
      *last = newBL;
      last = &newBL->next;
    }
    *last=0;
  } else if (isManager()) {
    PD((GC,"PerdioVar o:%d",getIndex()));
    ProxyList **last=&u.proxies;
    for (ProxyList *pl = u.proxies; pl; pl = pl->next) {
      ProxyList *newPL = new ProxyList(pl->sd,0);
      *last = newPL;
      last = &newPL->next;
    }
    *last = 0;
  } else if (isURL()) {
    // u.url is atom (no gc necessary)
    dogcGName(getGName());
  } else {
    Assert(isObjectURL() || isObjectGName());
    gcBorrowNow(getObject()->getIndex());
    ptr = getObject()->gcObject();
    if (isObjectURL()) {
      gcTagged(u.aclass,u.aclass);
    }
  }
}

/**********************************************************************/
/*   SECTION 14 :: Globalizing                                        */
/**********************************************************************/

GName *Name::globalize()
{
  if (!hasGName()) {
    Assert(GETBOARD(this)==am.rootBoard);
    homeOrGName = ToInt32(newGName(makeTaggedLiteral(this),GNT_NAME));
    setFlag(Lit_hasGName);
  }
  return getGName();
}

GName *Abstraction::globalize()
{
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_PROC));
  }
  return getGName();
}

GName *SChunk::globalize() {
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CHUNK));
  }
  return getGName();
}

void ObjectClass::globalize() {
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CLASS));
  }
}

void CellLocal::globalize(int myIndex){
  DebugIndexCheck(myIndex);

  CellManager *cm=(CellManager*)this;
  cm->setTertType(Te_Manager);
  cm->setOwnCurrent();
  cm->setIndex(myIndex);
    
  CellFrame *cf=(CellFrame*)this;
  TaggedRef v=val;
  PD((CELL,"globalize index:%d",myIndex));
  cf->initFromGlobalize(v);}

void LockLocal::globalize(int myIndex){
  DebugIndexCheck(myIndex);

  LockManager *lm=(LockManager*)this;
  lm->setTertType(Te_Manager);
  lm->setOwnCurrent();
  lm->setIndex(myIndex);
  LockFrame *lf=(LockFrame*)this;
  Thread *t=getLocker();
  PendThread *pt=getPending();
  PD((LOCK,"globalize index:%d",myIndex));
  lf->initFromGlobalize(t,pt);}


void Tertiary::globalizeTert()
{ 
  Assert(isLocal());

  switch(getType()) {
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
  case Co_Lock:
    {
      PD((GLOBALIZING,"GLOBALIZING lock"));
      OwnerEntry *oe_manager;
      int manI=ownerTable->newOwner(oe_manager);
      oe_manager->mkTertiary(this);
      LockLocal *ll=(LockLocal*)this;
      ll->globalize(manI);
      return;
    }

  case Co_Object:
    {
      Object *o = (Object*) this;
      RecOrCell state = o->getState();
      if (!stateIsCell(state)) {
	SRecord *r = getRecord(state);
	Assert(r!=NULL);
	Tertiary *cell = tagged2Tert(OZ_newCell(makeTaggedSRecord(r)));
	cell->globalizeTert();
	o->setState(cell);
      }
      break;
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

void CellProxy::convertToFrame(){
  CellFrame *cf=(CellFrame*)this;
  setTertType(Te_Frame);
  cf->initFromProxy();}

void LockProxy::convertToFrame(){
  LockFrame *lf=(LockFrame*)this;
  setTertType(Te_Frame);
  lf->initFromProxy();}


PerdioVar *var2PerdioVar(TaggedRef *tPtr)
{
  TypeOfTerm tag = tagTypeOf(*tPtr);
  if (tag==CVAR) {
    switch (tagged2CVar(*tPtr)->getType()) {
    case PerdioVariable:
      return tagged2PerdioVar(*tPtr);
    default:
      return NULL;
    }
  }

  OwnerEntry *oe;
  int i = ownerTable->newOwner(oe);
  oe->mkVar(makeTaggedRef(tPtr));

  PerdioVar *pvar=new PerdioVar();
  if (tag==SVAR) {
    pvar->setSuspList(tagged2SVar(*tPtr)->getSuspList());
  }
  pvar->setIndex(i);
  doBindCVar(tPtr,pvar);
  return pvar;
}

/**********************************************************************/
/*   SECTION 15 :: Localizing                                        */
/**********************************************************************/

void CellManager::localize(){
  CellFrame *cf=(CellFrame *)this;
  Assert(cf->getState()==Cell_Valid);
  TaggedRef tr=cf->getContents();    
  setTertType(Te_Local);
  setBoard(am.rootBoard);
  CellLocal *cl=(CellLocal*) this;
  cl->setValue(tr);}

void LockManager::localize(){
  LockFrame *lf=(LockFrame *)this;
  Assert(lf->getState()==Lock_Valid);
  Thread *t=lf->getLocker();
  setTertType(Te_Local);
  setBoard(am.rootBoard);
  LockLocal *ll=(LockLocal*) this;
  ll->convertToLocal(t,lf->getPending());}

void Tertiary::localize()
{ 
  switch(getType()) {
  case Co_Port:
    {
      Assert(getTertType()==Te_Manager);
      PD((GLOBALIZING,"GLOBALIZING: localizing tertiary manager"));
      setTertType(Te_Local);
      setBoard(am.rootBoard);
      return;
    }
  case Co_Cell:{
    Assert(getTertType()==Te_Manager);
    PD((GLOBALIZING,"localizing cell manager"));
    CellManager *cm=(CellManager*)this;
    cm->localize();
    return;}
  case Co_Lock:{
    Assert(getTertType()==Te_Manager);
    PD((GLOBALIZING,"localizing lock manager"));
    LockManager *lm=(LockManager*)this;
    lm->localize();
    return;}
  case Co_Thread:
  case Co_Space:
  case Co_Object:{
    Assert(getTertType()==Te_Manager);
    PD((GLOBALIZING,"localizing object/space/thread manager"));    
    setTertType(Te_Local);
    setBoard(am.rootBoard);
    return;}
  default:
    Assert(0);
    printf("cannot localize %d\n",getType());
    error("cannot localize\n");
  }
}

/**********************************************************************/
/*   SECTION 16 :: marshaling/unmarshaling by protocol-layer          */
/**********************************************************************/

/* for now credit is a 32-bit word */

inline void marshalCredit(Credit credit,MsgBuffer *bs){  
  Assert(sizeof(Credit)==sizeof(long));
  Assert(sizeof(Credit)==sizeof(unsigned int));
  PD((MARSHAL,"credit c:%d",credit));
  marshalNumber(credit,bs);}

inline Credit unmarshalCredit(MsgBuffer *bs){
  Assert(sizeof(Credit)==sizeof(long));
  Credit c=unmarshalNumber(bs);
  PD((UNMARSHAL,"credit c:%d",c));
  return c;}

inline void marshalOwnHead(int tag,int i,MsgBuffer *bs){
  bs->put(tag);
  mySite->marshalSite(bs);
  marshalNumber(i,bs);
  bs->put(DIF_PRIMARY);
  marshalNumber(ownerTable->getOwner(i)->getSendCredit(),bs);
  PD((MARSHAL,"ownHead o:%d rest-c:%d ",i,ownerTable->getOwner(i)->getCredit()));
  return;}

void marshalToOwner(int bi,MsgBuffer *bs){
  BorrowEntry *b=BT->getBorrow(bi); 
  int OTI=b->getOTI();
  if(b->getOnePrimaryCredit()){
    bs->put((BYTE) DIF_OWNER);
    marshalNumber(OTI,bs);
    PD((MARSHAL,"toOwner Borrow b:%d Owner o:%d",bi,OTI));
    return;}
  Assert(0);// ATTENTION
  bs->put((BYTE) DIF_OWNER_SEC);
  Site* xcs = b->getOneSecondaryCredit();
  marshalNumber(OTI,bs);
  xcs->marshalSite(bs);
  return;}

void marshalBorrowHead(MarshalTag tag, int bi,MsgBuffer *bs){
  bs->put((BYTE)tag);
  BorrowEntry *b=borrowTable->getBorrow(bi);
  NetAddress *na=b->getNetAddress();
  na->site->marshalSite(bs);
  marshalNumber(na->index,bs);
  Credit cred=b->getSmallPrimaryCredit();
  if(cred) {
    PD((MARSHAL,"borrowed b:%d remCredit c:%d give c:%d",bi,b->getCredit(),cred));
    bs->put(DIF_PRIMARY);
    marshalCredit(cred,bs);
    return;}
  Assert(0); // SECONDARY
  return;
}

OZ_Term unmarshalBorrow(MsgBuffer *bs,OB_Entry *&ob,int &bi){
  Site * sd=unmarshalSite(bs);
  int si=unmarshalNumber(bs);
  Credit cred;
  MarshalTag mt=(MarshalTag) bs->get();
  PD((UNMARSHAL,"borrow o:%d",si));
  if(sd==mySite){
    if(mt==DIF_PRIMARY){
      cred = unmarshalCredit(bs);      
      PD((UNMARSHAL,"mySite is owner"));
      OZ_Term ret = ownerTable->getOwner(si)->getValue();
      ownerTable->returnCreditAndCheck(si,cred);
      return ret;}
    Assert(0);}
  NetAddress na = NetAddress(sd,si); 
  BorrowEntry *b = borrowTable->find(&na);
    if (b!=NULL) {
      PD((UNMARSHAL,"borrow found"));
      if(mt==DIF_PRIMARY){
	cred = unmarshalCredit(bs);    
	b->addCredit(cred);
	ob = b;
	return b->getValue();}
      Assert(0);}
  if(mt==DIF_PRIMARY){
    bi=borrowTable->newBorrow(cred,sd,si);
    b=borrowTable->getBorrow(bi);
    PD((UNMARSHAL,"borrowed miss"));
    ob=b;
    return 0;}
  Assert(0); // SECONDARY
  return 0;
}

char *tagToComment(MarshalTag tag){
  switch(tag){
  case DIF_PORT:
    return "port";
  case DIF_THREAD:
    return "thread";
  case DIF_SPACE:
    return "space";
  case DIF_CELL:
    return "cell";
  case DIF_LOCK:
    return "lock";
  case DIF_OBJECT:
    return "object";
  default:
    Assert(0);
    return "";
}}

/* ******************  interface *********************************** */

void marshalVar(PerdioVar *pvar,MsgBuffer *bs){
  Site *sd=bs->getSite();
  if (pvar->isProxy()) {
    int i=pvar->getIndex();
    PD((MARSHAL,"var proxy o:%d",i));
    if(sd && borrowTable->getOriginSite(i)==sd) {
      marshalToOwner(i,bs);
      return;}
    marshalBorrowHead(DIF_VAR,i,bs);
    if (!sd) {
      warning("mm2: make persistent of proxy not fully impl.");
      BT->getBorrow(i)->makePersistentBorrowXX();}
    return;
  } 

  Assert(pvar->isManager());
  int i=pvar->getIndex();
  PD((MARSHAL,"var manager o:%d",i));
  Assert(pvar->isManager());
  marshalOwnHead(DIF_VAR,i,bs);
  if (!sd) {
    OT->getOwner(i)->makePersistentOwner();}
}

Bool marshalTertiary(Tertiary *t, MarshalTag tag, MsgBuffer *bs)
{
  Site *sd=bs->getSite();
  switch(t->getTertType()){
  case Te_Local:
    t->globalizeTert();
    // no break here!
  case Te_Manager:
    {
      PD((MARSHAL,"manager"));
      int OTI=t->getIndex();
      marshalOwnHead(tag,OTI,bs);
      if (!sd) {OT->getOwner(OTI)->makePersistentOwner();}
      break;
    }
  case Te_Frame:
  case Te_Proxy:
    {
      PD((MARSHAL,"proxy"));
      int BTI=t->getIndex();
      if (bs->getSite() && borrowTable->getOriginSite(BTI)==sd) {
	marshalToOwner(BTI,bs);
	return OK;}
      marshalBorrowHead(tag,BTI,bs);
      if(!sd) {BT->getBorrow(BTI)->makePersistentBorrow();}
      break;
    }
  default:
    Assert(0);
  }
  return NO;
}

OZ_Term unmarshalTertiary(MsgBuffer *bs, MarshalTag tag)
{
  OB_Entry* ob;
  int bi;
  OZ_Term val = unmarshalBorrow(bs,ob,bi);
  if(val){
    PD((UNMARSHAL,"%s hit b:%d",tagToComment(tag),bi));
    switch (tag) {
    case DIF_PORT:
    case DIF_THREAD:
    case DIF_SPACE:
      break;
    case DIF_CELL:{
      Tertiary *t=ob->getTertiary(); // mm2: bug: ob is 0 if I am the owner
	if((t->getType()==Co_Cell) && (t->getTertType()==Te_Frame)){
	  CellFrame *cf=(CellFrame *)t;
	  if(cf->getState() & Cell_Dump_Asked){
	    cf->setState(cf->getState() & ~Cell_Dump_Asked);}}
      break;}
    case DIF_LOCK:{
      Tertiary *t=ob->getTertiary();
      if((t->getType()==Co_Lock) && (t->getTertType()==Te_Frame)){
	LockFrame *lf=(LockFrame *)t;
	if(lf->getState() & Lock_Dump_Asked){
	  lf->setState(lf->getState() & ~Lock_Dump_Asked);}}
      break;}
    case DIF_OBJECT:
      TaggedRef clas;
      (void) unmarshalGName(&clas,bs);
      break;
    default:         
      Assert(0);
    }
    return val;
  }

  PD((UNMARSHAL,"%s miss b:%d",tagToComment(tag),bi));  
  Tertiary *tert;

  switch (tag) {
  case DIF_PORT:
    tert = new PortProxy(bi);        
    break;
  case DIF_THREAD:
    tert = new Thread(bi,Te_Proxy);  
    break;
  case DIF_SPACE:
    tert = new Space(bi,Te_Proxy);   
    break;
  case DIF_CELL:
    tert = new CellProxy(bi); 
    break;
  case DIF_LOCK:
    tert = new LockProxy(bi); 
    break;
  case DIF_OBJECT:
    {
      Object *o = new Object(bi);
      PerdioVar *pvar = new PerdioVar(o);
      val = makeTaggedRef(newTaggedCVar(pvar));
      TaggedRef clas;
      GName *gnclass = unmarshalGName(&clas,bs);
      if (gnclass) {
	pvar->setGNameClass(gnclass);
      } else {
	pvar->setClass(clas);
      }
      ob->mkVar(val);
      return val;}
  default:         
    Assert(0);
  }
  val=makeTaggedConst(tert);
  ob->mkTertiary(tert);
  return val;
}

OZ_Term unmarshalOwner(MsgBuffer *bs,MarshalTag mt){
  if(mt==DIF_OWNER){
    int OTI=unmarshalNumber(bs);
    PD((UNMARSHAL,"OWNER o:%d",OTI));
    OT->returnCreditAndCheck(OTI,1);
    return OT->getOwner(OTI)->getValue();}
  Assert(0);
  return 0;
}

OZ_Term unmarshalVar(MsgBuffer* bs){
  OB_Entry *ob;
  int bi;
  OZ_Term val1 = unmarshalBorrow(bs,ob,bi);
  
  if (val1) {
    PD((UNMARSHAL,"var/chunk hit: b:%d",bi));
    return val1;
  }
  
  PD((UNMARSHAL,"var miss: b:%d",bi));
  PerdioVar *pvar = new PerdioVar(bi);
  TaggedRef val = makeTaggedRef(newTaggedCVar(pvar));
  ob->mkVar(val);
  sendRegister((BorrowEntry *)ob);
  return val;
}

/**********************************************************************/
/*   SECTION 17:: Main Receive                                       */
/**********************************************************************/

void Site::msgReceived(MsgBuffer* bs)
{
  Assert(am.currentBoard==am.rootBoard);
  MessageType mt = (MessageType) unmarshalHeader(bs);
  
  
// ATTENTION handle secondary credit 
  switch (mt) {
  case M_PORT_SEND:   
    {
      int portIndex;
      OZ_Term t;
      
#line 2226
	unmarshal_M_PORT_SEND(bs,portIndex ,t   );
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
      int i;
      char *biName;
      OZ_Term t;
      
#line 2246
	unmarshal_M_REMOTE_SEND(bs,i ,biName ,t  );
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
      int na_index;
      Site* rsite;
      
#line 2274
	unmarshal_M_ASK_FOR_CREDIT(bs,na_index ,rsite   );
      OwnerEntry *o=OT->getOwner(na_index);
      o->returnCredit(1); // don't delete entry
      Credit c= o->giveMoreCredit();

      MsgBuffer *bs=msgBufferManager->getMsgBuffer(rsite);  
      
#line 2280
	marshal_M_BORROW_CREDIT(bs,mySite ,na_index ,c  );
      int rc=rsite->sendTo(bs,M_BORROW_CREDIT,NULL,0);
      if(rc==ACCEPTED) break;
      Assert(0);
      error("not implemented");
      break;
    }
  case M_OWNER_CREDIT:  
    {
      int index;
      Credit c;
      
#line 2291
	unmarshal_M_OWNER_CREDIT(bs,index ,c   );
      ownerTable->returnCreditAndCheck(index,c);
      break;
    }
  case M_BORROW_CREDIT:  
    {
      int si;
      Credit c;
      Site *sd;
      
#line 2300
	unmarshal_M_BORROW_CREDIT(bs,sd ,si ,c  );
      NetAddress na=NetAddress(sd,si);
      BorrowEntry *b=borrowTable->find(&na);
      Assert(b!=NULL);
      b->explicitReceivedCredit();
      b->addCredit(c);
      break;
    }

  case M_REGISTER:
    {
      int OTI;
      Site *rsite;
      
#line 2313
	unmarshal_M_REGISTER(bs,OTI ,rsite   );
      if (OT->getOwner(OTI)->isVar()) {
	PerdioVar *pv=OT->getOwner(OTI)->getVar();
	if (!pv->isRegistered(rsite)) {
	  pv->registerSite(rsite);}
	else {
	  PD((WEIRD,"REGISTER o:%d s:%s already registered",OTI,rsite->stringrep()));}}
      else {
	sendRedirect(rsite,OTI,OT->getOwner(OTI)->getRef());}
      ownerTable->returnCreditAndCheck(OTI,1);
      break;
    }

  case M_GET_OBJECT:
  case M_GET_OBJECTANDCLASS:
    {
      int OTI;
      Site *rsite;
      
#line 2331
	unmarshal_M_GET_OBJECT(bs,OTI ,rsite   );
      Tertiary *t = OT->getOwner(OTI)->getTertiary();
      Assert(isObject(t));
      sendObject(rsite,(Object *)t, mt==M_GET_OBJECTANDCLASS);
      OT->returnCreditAndCheck(OTI,1);
      break;
    }

  case M_SEND_OBJECT:
    {
      ObjectFields of;
      Site *sd;
      int si;
      
#line 2344
	unmarshal_M_SEND_OBJECT(bs,sd ,si ,&of  );

      NetAddress na=NetAddress(sd,si);
      BorrowEntry *be=BT->find(&na);
      be->addCredit(1);

      PerdioVar *pv = be->getVar();
      Object *o = pv->getObject();
      if(o==NULL) {Assert(0); error("M_SEND_OBJECT - don't understand");}
      fillInObject(&of,o);
      TaggedRef cl;
      if (pv->isObjectURL()) {cl=pv->getClass();}
      else {cl=findGName(pv->getGNameClass());}
      o->setClass(tagged2ObjectClass(deref(cl)));

      pv->primBind(be->getPtr(),makeTaggedConst(o));
      be->mkTertiary(o);
      break;
    }

  case M_SEND_OBJECTANDCLASS:
    {
      ObjectFields of;
      Site *sd;
      int si;
      
#line 2369
	unmarshal_M_SEND_OBJECTANDCLASS(bs,sd ,si ,&of  );

      NetAddress na=NetAddress(sd,si);
      BorrowEntry *be=BT->find(&na);
      be->addCredit(1);

      PerdioVar *pv = be->getVar();
      Object *o = pv->getObject();
      fillInObjectAndClass(&of,o);

      pv->primBind(be->getPtr(),makeTaggedConst(o));
      be->mkTertiary(o);
      break;
    }

  case M_REDIRECT:
    {
      Site *sd;
      int si;
      TaggedRef val;
      
#line 2389
	unmarshal_M_REDIRECT(bs,sd ,si ,val  );
      NetAddress na=NetAddress(sd,si);
      BorrowEntry *be=BT->find(&na);

      if (!be) { // if not found, then forget the redirect message
	PD((WEIRD,"REDIRECT: no borrow entry found"));
	sendCreditBack(na.site,na.index,1);
	break;
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
      int OTI;
      Site* rsite;
      TaggedRef v;
      
#line 2423
	unmarshal_M_SURRENDER(bs,OTI ,rsite ,v  );

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
      
      Site *sd;
      int si;
      
#line 2452
	unmarshal_M_ACKNOWLEDGE(bs,sd ,si   );

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
      int OTI;
      Site* rsite;
      
#line 2474
	unmarshal_M_CELL_GET(bs,OTI ,rsite   );
      OwnerEntry *oe=ownerTable->getOwner(OTI);
      if(cellReceiveGet((CellManager*)(oe->getTertiary()),OTI,rsite)){
	oe->returnCredit(1);}
      rsite->refCheck();
      break;
    }
   case M_CELL_CONTENTS:
    {
      Site *rsite;
      int OTI;
      TaggedRef val;
      
#line 2486
	unmarshal_M_CELL_CONTENTS(bs,rsite ,OTI ,val  );
      if(rsite==mySite){
	OwnerEntry *oe=OT->getOwner(OTI);
	if(cellReceiveContentsManager((CellManager*)(oe->getTertiary()),
				      val,OTI)) {
	  oe->returnCredit(1);}	
	break;
      }
      NetAddress na=NetAddress(rsite,OTI);
      BorrowEntry *be=borrowTable->find(&na);
      Assert(be!=NULL);
      if(cellReceiveContentsFrame((CellFrame*)(be->getTertiary()),
				  val,rsite,OTI)) {
	be->addCredit(1);}
      break;
    }
  case M_CELL_READ:
    {
      Assert(0);
      int OTI;
      Site *rsite;
      
#line 2507
	unmarshal_M_CELL_READ(bs,OTI ,rsite   );
      OwnerEntry *oe=ownerTable->getOwner(OTI);  
      CellManager *cm=(CellManager*)oe->getTertiary();
      PD((MSG_RECEIVED,"CELL_READ id:%d read_ctr:%d",OTI,((CellFrame*)oe->getTertiary())->getCtr()));      
/*
      if(cellReceiveRead((CellManager*)(oe->getTertiary()),OTI,val)){
      oe->returnCredit(1);}*/
      break;
    }
  case M_CELL_REMOTEREAD:      
    {
      Assert(0);
      int OTI;
      Site *cellSite;
      Site *toSite;
      
#line 2522
	unmarshal_M_CELL_REMOTEREAD(bs,cellSite ,OTI ,toSite  );
      break;
    }
  case M_CELL_FORWARD:
    {
      
      Site *site,*rsite;
      int OTI,ctr;
      
#line 2530
	unmarshal_M_CELL_FORWARD(bs,site ,OTI ,ctr ,rsite );
      NetAddress na=NetAddress(site,OTI);
      BorrowEntry *be=borrowTable->find(&na);
      Assert(be!=NULL);
      CellFrame *cf= (CellFrame*) be->getTertiary();
      if(cellReceiveForward(cf,ctr,rsite,site,OTI)){be->addCredit(1);}
      break;
    }
  case M_CELL_DUMP:
    {
      int OTI;
      Site* rsite;
      
#line 2542
	unmarshal_M_CELL_DUMP(bs,OTI ,rsite   );
      OwnerEntry *oe=ownerTable->getOwner(OTI);
      oe->returnCredit(1);
      cellReceiveDump((CellManager*)(oe->getTertiary()),rsite);
      rsite->refCheck();
      break;
    }
  case M_LOCK_GET:
    {
      int OTI;
      Site* rsite;
      
#line 2553
	unmarshal_M_LOCK_GET(bs,OTI ,rsite   );
      OwnerEntry *oe=OT->getOwner(OTI);
      if(lockReceiveGet((LockManager*)(oe->getTertiary()),OTI,rsite)){
	oe->returnCredit(1);
      }
      break;
    }
  case M_LOCK_TOKEN:
    {
      Site *rsite;
      int OTI;
      
#line 2564
	unmarshal_M_LOCK_TOKEN(bs,rsite ,OTI   );
      if(rsite==mySite){
	OwnerEntry *oe=OT->getOwner(OTI);
	oe->returnCredit(1);
	lockReceiveLock((LockFrame*)(oe->getTertiary()));
	break;
      }
      NetAddress na=NetAddress(rsite,OTI);
      BorrowEntry *be=borrowTable->find(&na);
      Assert(be!=NULL);
      be->addCredit(1);
      lockReceiveLock((LockFrame*)be->getTertiary());
      break;
    }
  case M_LOCK_FORWARD:
    {
      Site *site,*rsite;
      int OTI;
      
#line 2582
	unmarshal_M_LOCK_FORWARD(bs,site ,OTI ,rsite  );
      NetAddress na=NetAddress(site,OTI);
      BorrowEntry *be=borrowTable->find(&na);
      Assert(be!=NULL);
      LockFrame *lf= (LockFrame*) be->getTertiary();
      if(lockReceiveForward(lf,rsite,site,OTI)){be->addCredit(1);}
      break;
    }
  case M_LOCK_DUMP:
    {
      int OTI;
      Site* rsite;
      
#line 2594
	unmarshal_M_LOCK_DUMP(bs,OTI ,rsite   );
      OwnerEntry *oe=ownerTable->getOwner(OTI);
      oe->returnCredit(1);
      lockReceiveDump((LockManager*)(oe->getTertiary()),rsite);
      break;
    }
  default:
    error("siteReceive: unknown message %d\n",mt);
    break;
  }
}


/**********************************************************************/
/*   SECTION 18:: protocol utility routines                           */
/**********************************************************************/

inline void handleMsgCredit(BorrowEntry* be){
  Assert(creditSite==NULL);
  if(!be->getOnePrimaryCredit()){
    creditSite=be->getOneSecondaryCredit();}}

inline void handleMsgCredit(OwnerEntry* oe){oe->getOneCreditOwner();}

/**********************************************************************/
/*   SECTION 19:: remote send protocol                                */
/**********************************************************************/

/* engine-interface */
OZ_Return remoteSend(Tertiary *p, char *biName, TaggedRef msg) {
  BorrowEntry *b= borrowTable->getBorrow(p->getIndex());
  NetAddress *na = b->getNetAddress();
  Site* site = na->site;
  int index = na->index;

  MsgBuffer *bs = msgBufferManager->getMsgBuffer(site);
  handleMsgCredit(b);
  
#line 2631
	marshal_M_REMOTE_SEND(bs,index ,biName ,msg  );
  int ret=site->sendTo(bs,M_REMOTE_SEND,NULL,0);
  if(ret==ACCEPTED) return PROCEED;
  Assert(0);
  error("not implemnted");
  return PROCEED;}
  
/**********************************************************************/
/*   SECTION 20:: Port protocol                                       */
/**********************************************************************/

void portSend(Tertiary *p, TaggedRef msg) {
  int pi = p->getIndex();
  BorrowEntry* b=BT->getBorrow(pi);
  NetAddress *na = b->getNetAddress();
  Site* site = na->site;
  int index = na->index;
  
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(site);
  handleMsgCredit(b);
  
#line 2651
	marshal_M_PORT_SEND(bs,index ,msg   );
  int ret=site->sendTo(bs,M_PORT_SEND,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

/**********************************************************************/
/*   SECTION 21:: Variable protocol                                       */
/**********************************************************************/

// compare NAs
#define GET_ADDR(var,SD,OTI)						\
Site* SD;int OTI;							\
if (var->isProxy()) {							\
  NetAddress *na=BT->getBorrow(var->getIndex())->getNetAddress();	\
  SD=na->site;								\
  OTI=na->index;							\
} else {								\
  SD=mySite;                                                            \
  OTI=var->getIndex();							\
}

int compareNetAddress(PerdioVar *lVar,PerdioVar *rVar)
{
  GET_ADDR(lVar,lSD,lOTI);
  GET_ADDR(rVar,rSD,rOTI);
  int ret = lSD->compareSites(rSD);
  if (ret != 0) return ret;
  return lOTI<rOTI ? -1 : 1;
}

void sendRegister(BorrowEntry *be) {
  handleMsgCredit(be);
  NetAddress *na = be->getNetAddress();  
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  
#line 2687
	marshal_M_REGISTER(bs,na->index ,mySite   );
  int ret=na->site->sendTo(bs,M_REGISTER,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

void sendSurrender(BorrowEntry *be,OZ_Term val)
{
  handleMsgCredit(be);
  NetAddress *na = be->getNetAddress();  
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  
#line 2699
	marshal_M_SURRENDER(bs,na->index ,mySite ,val  );
  int ret=na->site->sendTo(bs,M_SURRENDER,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

void sendRedirect(Site* sd,int OTI,TaggedRef val)
{
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(sd);
  OT->getOwner(OTI)->getOneCreditOwner();
  
#line 2710
	marshal_M_REDIRECT(bs,mySite ,OTI ,val  );
  int ret=sd->sendTo(bs,M_REDIRECT,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

void sendAcknowledge(Site* sd,int OTI)
{
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(sd);  
  OT->getOwner(OTI)->getOneCreditOwner();
  
#line 2721
	marshal_M_ACKNOWLEDGE(bs,mySite ,OTI   );
  int ret=sd->sendTo(bs,M_ACKNOWLEDGE,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

void PerdioVar::acknowledge(OZ_Term *p)
{
  PD((PD_VAR,"acknowledge"));
  OZ_Term val=u.bindings->val;
  primBind(p,val);
  if (u.bindings->thread->isDeadThread()) {
    PD((WEIRD,"dead thread acknowledge %x",u.bindings->thread));
  } else {
    PD((THREAD_D,"start thread ackowledge %x",u.bindings->thread));
    oz_resume(u.bindings->thread);
  }

  PendBinding *tmp=u.bindings->next;
  u.bindings->dispose();
  u.bindings=tmp;
  redirect(val);
}
    
void PerdioVar::redirect(OZ_Term val) {
  PD((PD_VAR,"redirect v:%s",toC(val)));
  while (u.bindings) {

    if (u.bindings->thread->isDeadThread()) {
      PD((WEIRD,"dead thread redirect %x",u.bindings->thread));
      PD((THREAD_D,"dead thread redirect %x",u.bindings->thread));
      PD((PD_VAR,"redirect pending unify =%s",toC(u.bindings->val)));
      SiteUnify(val,u.bindings->val);
    } else {
      RefsArray args = allocateRefsArray(2,NO);
      args[0]=val;
      args[1]=u.bindings->val;
      u.bindings->thread->pushCall(BI_Unify,args,2);
      PD((PD_VAR,"redirect pending unify =%s",toC(u.bindings->val)));
      PD((THREAD_D,"start thread redirect %x",u.bindings->thread));
      oz_resume(u.bindings->thread);
    }

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
  } else if (pv->isObjectURL() || pv->isObjectGName()) {
    PD((PD_VAR,"bind object u:%s",toC(makeTaggedConst(pv->getObject()))));
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

/**********************************************************************/
/*   SECTION 22:: Object protocol                                     */
/**********************************************************************/

void sendObject(Site* sd, Object *o, Bool sendClass)
{
  int OTI = o->getIndex();
  MsgBuffer *bs= msgBufferManager->getMsgBuffer(sd);
  OT->getOwner(OTI)->getOneCreditOwner();
  int ret;
  if(sendClass){
    
#line 2826
	marshal_M_SEND_OBJECTANDCLASS(bs,mySite ,OTI ,o  );
    ret=sd->sendTo(bs,M_SEND_OBJECTANDCLASS,NULL,0);}
  else{
    
#line 2829
	marshal_M_SEND_OBJECT(bs,mySite ,OTI ,o  );
    ret=sd->sendTo(bs,M_SEND_OBJECT,NULL,0);}
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
}

/**********************************************************************/
/*   SECTION 23:: Credit protocol                                     */
/**********************************************************************/

void sendCreditBack(Site* sd,int OTI,Credit c)
{
  MsgBuffer *bs= msgBufferManager->getMsgBuffer(sd);
  
#line 2843
	marshal_M_OWNER_CREDIT(bs,OTI ,c   );
  int ret=sd->sendTo(bs,M_OWNER_CREDIT,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

/**********************************************************************/
/*   SECTION 24:: Cell protocol                                       */
/**********************************************************************/

/* ---------------------    send  primitives  TYPE 1 -------------------- */
/* -------------            OBS  holding one credit  ---------------------*/
                   
void cellSendForward(Site *toS,Site *rS,int ctr,int mI){  
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  
#line 2859
	marshal_M_CELL_FORWARD(bs,mySite ,mI ,ctr ,rS );
  int ret=toS->sendTo(bs,M_CELL_FORWARD,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

void cellSendRemoteRead(int mI,Site *toS,TaggedRef val){  
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  Assert(0);
  
#line 2869
	marshal_M_CELL_REMOTEREAD(bs,mySite ,mI ,NULL  );
  int ret=toS->sendTo(bs,M_CELL_REMOTEREAD,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

void cellSendContents(TaggedRef tr,Site* toS,Site *mS,int mI){ 
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  
#line 2878
	marshal_M_CELL_CONTENTS(bs,mS ,mI ,tr  );  
  int ret=toS->sendTo(bs,M_CELL_CONTENTS,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

/* ---------------------    send  primitives  TYPE 2 -------------------- */
/* -------------            OBS  not holding any credit ------------------*/

void cellSendGet(BorrowEntry *be){      
  NetAddress *na=be->getNetAddress();
  Site *toS=na->site;
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  handleMsgCredit(be);
  
#line 2893
	marshal_M_CELL_GET(bs,na->index ,mySite   );  
  int ret=toS->sendTo(bs,M_CELL_GET,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

void cellSendRead(BorrowEntry *be,TaggedRef val){
  NetAddress *na=be->getNetAddress();
  Site *toS=na->site;
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  handleMsgCredit(be);
  
#line 2905
	marshal_M_CELL_READ(bs,na->index ,mySite   );
  int ret=toS->sendTo(bs,M_CELL_READ,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

void cellSendDump(BorrowEntry *be,CellFrame *cf){
  Assert(cf->getState()==Cell_Valid);
  NetAddress *na=be->getNetAddress();
  Site *toS=na->site;
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  handleMsgCredit(be);
  
#line 2918
	marshal_M_CELL_DUMP(bs,na->index ,mySite   );
  cf->setState(Cell_Valid | Cell_Dump_Asked);
  int ret=toS->sendTo(bs,M_CELL_DUMP,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

/* --------------------- at Manager main  -------------------------------- */
/*                     holding one credit                                  */

Bool  cellReceiveGet(CellManager*cm,int mI,Site* toS){  
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);

  Site* current=cm->getCurrent();
  cm->setCurrent(toS);            
  toS->inc();

  if(current==NULL){                             // shortcut
    PD((CELL,"CELL - shortcut in cellReceiveGet"));
    CellFrame *cf=(CellFrame*)cm;
    if(cf->getState()==Cell_Requested){
      cf->setState(Cell_Requested | Cell_Next);
      toS->inc();
      cf->setNext(toS);
      return TRUE;}
    Assert(cf->getState()==Cell_Valid);
    cellSendContents(cf->getContents(),toS,mySite,mI);
    cf->setState(Cell_Invalid);
    return FALSE;}
  current->dec();
  cellSendForward(current,toS,cm->getAndInitManCtr(),mI);
  return FALSE;
}

Bool cellReceiveRead(CellManager *cm,int mI,TaggedRef val){ 
  Assert(cm->getType()==Co_Cell);                         
  Assert(cm->getTertType()==Te_Manager);
  Assert(cm->getIndex()==mI);
  Site* current=cm->getCurrent();
  if(current==NULL){                     // shortcut
    CellFrame *cf=(CellFrame*)cm;
    PD((CELL,"shortcut cellReceiveRead to cellReceiveRemoteRead"));
    SiteUnify(val,cf->getContents());    // ATTENTION
    return TRUE;}
  cm->incManCtr();
  cellSendRemoteRead(mI,current,val);
  return FALSE;
}

Bool cellReceiveContentsManager(CellManager *cm,TaggedRef val,int mI){ 
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);
  CellFrame *cf=(CellFrame*)cm;
  short  state=cf->getState();
  Assert(state & Cell_Requested);
  pendThreadResumeAll(cf->getPending());
  cf->setPending(NULL);
  TaggedRef head=cf->getHead();
  SiteUnify(head,val);
  if(state & Cell_Next){
    Site *next=cf->getNext();
    cellSendContents(cf->getContents(),next,mySite,mI);
    next->dec();
    cf->setState(Cell_Invalid);
    return FALSE;}
  cf->setState(Cell_Valid);
  return TRUE;
}

/* --------------------- at Manager main  -------------------------------- */
/*                    NOT holding one credit                               */

void cellReceiveDump(CellManager *cm,Site *fromS){
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);
  Site *current=cm->getCurrent();
  if(current!=fromS){
    PD((WEIRD,"WEIRD- CELL dump out of synch"));
    return;}
  CellFrame* cf=(CellFrame *)cm;
  if(cf->getState()!=Cell_Invalid){
    PD((WEIRD,"WEIRD- CELL dump not needed"));
    return;}
  Assert(cf->getState()==Cell_Invalid);
  fromS->refCheck();
  TaggedRef tr=oz_newVariable();
  cellDoExchange((Tertiary *)cf,tr,tr,DummyThread);
  return;
}

/* --------------------- at Frame ------------------------------------- */
/*              holding one credit                                      */

Bool cellReceiveForward(CellFrame *cf,int ctr,Site *toS,Site* mS,int mI){
  Assert(cf->getTertType()==Te_Frame);
  Assert(cf->getType()==Co_Cell);
  cf->setState(cf->getState() & (~Cell_Dump_Asked));
  if(cf->getState() & Cell_Requested){
    Assert(!(cf->getState() & Cell_Next));
    cf->setState(Cell_Requested | Cell_Next);
    toS->inc();
    cf->setNext(toS);
    return TRUE;}
  Assert(cf->getState() & Cell_Valid);
  cf->decCtr(ctr);
  if(cf->getCtr()!=0){
    cf->setNext(toS);
    toS->inc();
    cf->setState(Cell_Valid | Cell_Next);
    PD((CELL,"mismatch read/exchange"));
    return TRUE;}
  TaggedRef tr=cf->getContents();
  cellSendContents(tr,toS,mS,mI);
  cf->setState(Cell_Invalid);
  toS->refCheck();
  return FALSE;}

Bool cellReceiveContentsFrame(CellFrame *cf,TaggedRef val,Site *mS,int mI){ 
  Assert(cf->getType()==Co_Cell);
  Assert(cf->getTertType()==Te_Frame);
  short  state=cf->getState();
  Assert(state & Cell_Requested);

  pendThreadResumeAll(cf->getPending());
  cf->setPending(NULL);
  SiteUnify(cf->getHead(),val);
  if(state & Cell_Next){
    if(cf->getCtr()==0){
      Site *toSite=cf->getNext();
      cellSendContents(cf->getContents(),toSite,mS,mI);
      toSite->refCheck();
      cf->setState(Cell_Invalid);
      return FALSE;}
    else{
      cf->setState(Cell_Valid | Cell_Next);
      return TRUE;
    }}
  cf->setState(Cell_Valid);
  return TRUE;
}

/* --------------------- not holding creidt ------------------------------------- */

void cellReceiveRemoteRead(CellFrame *cf,TaggedRef val){
  Assert(cf->getTertType()==Te_Frame);
  Assert(cf->getType()==Co_Cell);
  cf->incCtr();
  Assert(cf->getState() & (Cell_Valid | Cell_Requested));
  SiteUnify(val,cf->getContents());
  return;}


/* --------------------- builtin ------------------------------------- */


TaggedRef cellGetContentsFast(Tertiary *c)
{
  switch (c->getTertType()) {
  case Te_Manager:
    {
      if(!((CellManager*)c)->isOwnCurrent())
	return makeTaggedNULL();
    }  // no break here
  case Te_Frame:
    {
      CellFrame *cf=(CellFrame *)c; 
      if (cf->getState() & Cell_Valid) {
	return cf->getContents();
      }
      break;
    }
  default:
    break;
  }
  return makeTaggedNULL();
}


/* --------------------- initiate ------------------------------------- */

inline void e_invalid(CellFrame *cf,TaggedRef old,TaggedRef nw,Thread* th){
  {
    PendThread *head=cf->getPending();
    PendThread *pt=head;
    while(pt!=NULL) pt=pt->next;}
    
  cf->setState(Cell_Requested);
  cf->setHead(old);
  cf->setContents(nw);
  Assert(isRealThread(th) || th==DummyThread);
  pendThreadAddToEnd(cf->getPendBase(),th);}

void cellDoExchange(Tertiary *c,TaggedRef old,TaggedRef nw,Thread* th){
  Assert(th!=MoveThread);
  TertType tt=c->getTertType();
  CellFrame *cf=(CellFrame *)c; 

  if(tt==Te_Proxy){              // convert Proxy to Frame
    CellProxy *cp=(CellProxy *)c;
    PD((CELL,"CELL: convertToFrame %s-%d",BTOS(cp->getIndex())->stringrep(),BTOI(cp->getIndex())));
    cp->convertToFrame();}
  else{
    if(tt==Te_Manager){             // Manager
      PD((CELL,"CELL: exchange on manager handled as frame %s-%d",mySite->stringrep(),cf->getIndex()));
      CellManager *cm=(CellManager*)c;
      if(cf->getState()==Cell_Invalid){
	PD((CELL,"CELL: exchange on INVALID manager %s-%d",mySite->stringrep(),cf->getIndex()));    
	Site* current=cm->getCurrent();
	cm->setOwnCurrent();
	e_invalid(cf,old,nw,th);  
	int myI=cm->getIndex();
	OwnerEntry *oe=OT->getOwner(myI);
	oe->getOneCreditOwner();
	cellSendForward(current,mySite,cm->getAndInitManCtr(),myI);
	return;}}
    else{
      Assert(tt=Te_Frame);
      PD((CELL,"CELL: exchange on frame %s-%d",BTOS(cf->getIndex())->stringrep(),BTOI(cf->getIndex())));}}
  int mI=cf->getIndex();
  short state=cf->getState();
  if(state & Cell_Valid){
    PD((CELL,"exchange on VALID"));
    TaggedRef tr=cf->getContents();
    cf->setContents(nw);
    SiteUnify(tr,old);
    return;}
  if(state & Cell_Requested){
    PD((CELL,"exchange on REQUESTED"));
    TaggedRef tr=cf->getContents();
    cf->setContents(nw);
    Assert(th!=DummyThread);
    pendThreadAddToEnd(cf->getPendBase(),th);
    SiteUnify(tr,old);
    return;}
  
  Assert(c->getTertType()==Te_Frame);
  Assert(state==Cell_Invalid);
  PD((CELL,"exchange on INVALID"));
  e_invalid(cf,old,nw,th);
  cellSendGet(BT->getBorrow(mI));
  return;
}

void cellDoAccess(Tertiary *c,TaggedRef val){
  TertType tt=c->getTertType();

  if(tt==Te_Proxy){
    PD((CELL,"CELL: access on PROXY index"));
    CellProxy *cp=(CellProxy*)c;
    cellSendRead(BT->getBorrow(cp->getIndex()),val);
    return;}

  CellFrame *cf=(CellFrame *)c;
  if(tt==Te_Manager){
    CellManager *cm=(CellManager*)c;
    if(cf->getState()==Cell_Invalid){
      int mI=cm->getIndex();
      OwnerEntry *oe=OT->getOwner(mI);
      oe->getOneCreditOwner();
      PD((CELL,"access on INVALID manager %d",cf->getIndex()));      
      cellSendRemoteRead(mI,cm->getCurrent(),val);
      cm->incManCtr();
      return;}
    PD((CELL,"access on INVALID manager %d",cf->getIndex()));}
  else{
    PD((CELL,"access on frame %s-%d",BT->getOriginSite(cf->getIndex())->stringrep(),
	BT->getOriginIndex(cf->getIndex())));}
  short state=cf->getState();
  if(state & Cell_Valid){
    PD((CELL,"access on valid"));
    SiteUnify(val,cf->getContents());
    return;}
  if(state & Cell_Requested){
    PD((CELL,"access on requested"));
    SiteUnify(val,cf->getContents());
    return;}
  Assert(state == Cell_Invalid);
  Assert(tt=Te_Frame);
  cellSendRead(BT->getBorrow(cf->getIndex()),val);
  return;
}

/**********************************************************************/
/*   SECTION 25:: Lock protocol                                       */
/**********************************************************************/

/* basic sends - type 1 - holding one credit  */

void lockSendForward(Site *toS,Site *fS,int mI){ 
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(toS);
  
#line 3210
	marshal_M_LOCK_FORWARD(bs,mySite ,mI ,fS  );
  int ret=toS->sendTo(bs,M_LOCK_FORWARD,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

void lockSendLock(Site *mS,int mI,Site* toS){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(toS);
  
#line 3219
	marshal_M_LOCK_TOKEN(bs,mS ,mI   );
  int ret=toS->sendTo(bs,M_LOCK_TOKEN,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

/* basic sends - type 2 - NOT holding one credit  */

void lockSendDump(BorrowEntry *be,LockFrame *lf){       
  Assert(lf->getState()==Lock_Valid);
  Assert(lf->getTertType()==Te_Frame);
  NetAddress *na=be->getNetAddress();
  Site *toS=na->site;
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(toS);  
  handleMsgCredit(be);
  
#line 3235
	marshal_M_LOCK_DUMP(bs,na->index ,mySite   );  
  lf->setState(Cell_Valid | Cell_Dump_Asked);
  int ret=toS->sendTo(bs,M_LOCK_DUMP,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

void lockSendLockBorrow(BorrowEntry *be,Site* toS){   
  NetAddress *na=be->getNetAddress();
  handleMsgCredit(be);
  lockSendLock(na->site,na->index,toS);}

void lockSendGet(BorrowEntry *be){
  NetAddress *na=be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);  
  handleMsgCredit(be);
  
#line 3252
	marshal_M_LOCK_GET(bs,na->index ,mySite   );  
  int ret=na->site->sendTo(bs,M_LOCK_GET,NULL,0);
  if(ret==ACCEPTED) return;
  Assert(0);
  error("not implemnted");
  return;}

/* -------------- at Manager - holding credit  ---------------------------- */	

Bool lockReceiveGet(LockManager* lm,int mI,Site* toS){  // holding one credit
  Assert(lm->getType()==Co_Lock);
  Assert(lm->getTertType()==Te_Manager);

  Site* current=lm->getCurrent();
  lm->setCurrent(toS);            
  toS->inc();
  if(current==NULL){                             // shortcut
    PD((LOCK," shortcut in lockReceiveGet"));
    LockFrame *lf=(LockFrame*)lm;
    if(lf->getState()==Lock_Requested){
      lf->setState(Lock_Requested | Lock_Next);
      toS->inc();
      lf->setNext(toS);
      return TRUE;}
    Assert(lf->getState()==Lock_Valid);
    if(lf->getLocker()==NULL){
      lockSendLock(mySite,mI,toS);
      lf->setState(Lock_Invalid);
      return FALSE;}
    lf->setState(Lock_Valid | Lock_Next);
    lf->setNext(toS);
    toS->inc();
    return TRUE;}
  lockSendForward(current,toS,mI);
  current->dec();
  return FALSE;
}


void lockReceiveDump(LockManager* lm,Site *fromS){
  Assert(lm->getType()==Co_Lock);
  Assert(lm->getTertType()==Te_Manager);
  Site *current=lm->getCurrent();
  if(current!=fromS){
    PD((WEIRD,"WEIRD- LOCK dump out of synch"));
    return;}
  LockFrame* lf=(LockFrame *)lm;
  if(lf->getState()!=Lock_Invalid){
    PD((WEIRD,"WEIRD- LOCK dump not needed"));
    return;}
  fromS->refCheck();
  lm->lockComplex(DummyThread);
  return;
}

void lockReceiveLock(LockFrame* lf){
  Assert((lf->getTertType()==Te_Manager) || (lf->getTertType()==Te_Frame));
  Assert(lf->getType()==Co_Lock);
  int state=lf->getState();
  Assert((state== Lock_Requested) || (state == (Lock_Requested | Lock_Next)));
  if(state & Lock_Next) {lf->setState(Lock_Valid | Lock_Next);}
  else {lf->setState(Lock_Valid);}
  Assert(lf->getLocker()==NULL);
  if(lf->getPending()->thread!=DummyThread){
    lf->setLocker(pendThreadResumeFirst(lf->getPendBase()));
    return;}
  pendThreadRemoveFirst(lf->getPendBase());
  Assert(lf->getTertType()==Te_Manager);
  if(lf->getPending()==NULL) {
    lf->setLocker(NULL);
    return;}
  ((LockManager*)lf)->unlockComplex();}

Bool lockReceiveForward(LockFrame *lf,Site *toS,Site* mS,int mI){
  Assert(lf->getTertType()==Te_Frame);
  Assert(lf->getType()==Co_Lock);
  lf->setState(lf->getState() & (~Lock_Dump_Asked));
  if(lf->getState() & Lock_Requested){
    Assert(!(lf->getState() & Lock_Next));
    lf->setState(Lock_Requested | Lock_Next);
    toS->inc();
    lf->setNext(toS);
    return TRUE;}
  Assert(lf->getState()==Lock_Valid);
  if((lf->getPending()==NULL) && lf->getLocker()==NULL){
    lockSendLock(mS,mI,toS);
    lf->setState(Lock_Invalid);
    toS->refCheck();
    return FALSE;}
  lf->setNext(toS);
  lf->setState(Lock_Valid | Lock_Next);
  toS->inc();
  pendThreadAddToEnd(lf->getPendBase(),MoveThread);
  return TRUE;}

void LockProxy::lock(Thread *t){
    PD((LOCK,"convertToFrame %s-%d",BTOS(getIndex())->stringrep(),BTOI(getIndex())));
    convertToFrame();
    ((LockFrame*)this)->lock(t);}

inline void basicLock(LockFrame *lf,Thread *t){
  Assert(isRealThread(t));
  int state=lf->getState();
  if(state & Lock_Valid){
    Thread *ct=lf->getLocker();
    if(ct==t) {
      PD((LOCK,"lock VALID already has locck"));
      return;}
    if(ct==NULL){
      Assert(lf->getPending()==NULL);
      PD((LOCK,"lock VALID gets lock"));
      lf->setLocker(t);
      return;
    }
    PD((LOCK,"lock VALID lock held by other"));
    if(state & Lock_Next){
      if(lf->getPending()==NULL){
	pendThreadAddToEnd(lf->getPendBase(),MoveThread);
	PD((LOCK,"lock VALID due to be shipped out"));}
    }
    pendThreadAddToEnd(lf->getPendBase(),t);
    return;}
  
  pendThreadAddToEnd(lf->getPendBase(),t);  
  if(state & Lock_Requested){
    PD((LOCK,"lock REQUESTED "));    
    return;}
  Assert(lf->getTertType()==Te_Frame);
  Assert(state == Lock_Invalid);
  lf->setState(Lock_Requested);
  lockSendGet(BT->getBorrow(lf->getIndex()));
  return;}
  
void LockManager::lockComplex(Thread *t){
  Assert(t!=MoveThread);
  LockFrame *lf=(LockFrame*)this;
  if(lf->getState()==Lock_Invalid){
    PD((LOCK,"lock on INVALID manager  %s-%d",mySite->stringrep(),getIndex()));
    Site* current=getCurrent();
    Assert(current!=NULL);
    lf->setState(Lock_Requested);
    setOwnCurrent();
    pendThreadAddToEnd(lf->getPendBase(),t);
    OwnerEntry *oe=OT->getOwner(getIndex());
    oe->getOneCreditOwner();
    lockSendForward(current,mySite,getIndex());
    current->dec();
    return;}
  PD((LOCK,"lock on manager treated as frame %d",getIndex()));
  basicLock(lf,t);}

void LockFrame::lockComplex(Thread *t){
  PD((LOCK,"lock on frame  %s-%d",BTOS(getIndex())->stringrep(),BTOI(getIndex())));
  basicLock(this,t);}


void LockLocal::unlockComplex(){
  setLocker(pendThreadResumeFirst(getPendBase()));
  Assert(getLocker());
  return;}

void LockLocal::lockComplex(Thread *t){
  pendThreadAddToEnd(getPendBase(),t);}
  
void LockFrame::unlockComplex(){
  Assert(getState() & Lock_Valid);
  if(getState() & Lock_Next){
    Assert(getState()==(Lock_Next | Lock_Valid));
    if(getPending()==NULL){
      BorrowEntry *be=BT->getBorrow(getIndex());
      setLocker(NULL);
      Site *toS=getNext();
      lockSendLockBorrow(be,toS);
      toS->dec();
      setState(Lock_Invalid);
      return;}
    Assert(getPending()->thread != DummyThread);
               // indicates where request to forward came 
    if(getPending()->thread==MoveThread){      
      BorrowEntry *be=BT->getBorrow(getIndex());
      pendThreadRemoveFirst(getPendBase());
      setLocker((Thread*) NULL);
      Site *toS=getNext();
      lockSendLockBorrow(be,toS);
      toS->dec();
      if(getPending()!=NULL){
	setState(Lock_Requested);
	lockSendGet(be);
	return;}
      setState(Lock_Invalid);
      return;}
    setLocker(pendThreadResumeFirst(getPendBase()));
    return;}
  if(getPending()==NULL){
    setLocker((Thread*) NULL);
    return;}
  setLocker(pendThreadResumeFirst(getPendBase()));
  return;}

void LockManager::unlockComplex(){
  LockFrame *lf=(LockFrame*)this;
  Assert(lf->getState() & Lock_Valid);
  if(lf->getState() & Lock_Next){
    Assert(lf->getState()==(Lock_Next | Lock_Valid));
    if(lf->getPending()==NULL){
      OwnerEntry *oe=OT->getOwner(getIndex());
      Site *toS=lf->getNext();
      oe->getOneCreditOwner();
      lockSendLock(mySite,getIndex(),toS);
      toS->dec();
      lf->setLocker(NULL);
      lf->setState(Lock_Invalid);
      return;}
    if(lf->getPending()->thread==DummyThread)  { // dummy lock
      pendThreadRemoveFirst(lf->getPendBase());
      if(lf->getPending()==NULL){
	lf->setLocker(NULL);
	return;}
      if(lf->getPending()->thread!=NULL){
	Assert(lf->getPending()->thread != DummyThread);
	lf->setLocker(pendThreadResumeFirst(lf->getPendBase()));
	return;}} 
    if(lf->getPending()->thread==MoveThread){ 
      OwnerEntry *oe=OT->getOwner(getIndex());
      pendThreadRemoveFirst(lf->getPendBase());
      lf->setLocker((Thread*) NULL);
      Site *toS=lf->getNext();      
      oe->getOneCreditOwner();
      lockSendLock(mySite,getIndex(),toS);
      toS->dec();
      lf->setState(Lock_Requested);
      Site *current=getCurrent();
      setCurrent(NULL);
      oe->getOneCreditOwner();
      lockSendForward(current,mySite,getIndex());
      current->dec();
      return;}
    Assert(lf->getPending()->thread != DummyThread);
    lf->setLocker(pendThreadResumeFirst(lf->getPendBase()));
    return;}
  if(lf->getPending()==NULL){
    lf->setLocker((Thread*) NULL);
    return;}
  lf->setLocker(pendThreadResumeFirst(lf->getPendBase()));
  return;}

/**********************************************************************/
/*   SECTION 26:: Builtins                                            */
/**********************************************************************/

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

/**********************************************************************/
/*   SECTION 27:: Initialization                                      */
/**********************************************************************/


BIspec perdioSpec[] = {

#ifdef DEBUG_PERDIO
  {"dvset",    2, BIdvset, 0},
#endif
  {0,0,0,0}
};

Bool perdioInit(){
  initNetwork();
  if(mySite==NULL) return NO;
  return OK;}

void BIinitPerdio()
{
#ifdef DEBUG_PERDIO
  DV = new DebugVector();
#endif

  initMarshaler();
  initComponents();

  creditSite=NULL;

  BIaddSpec(perdioSpec);

  ownerTable = new OwnerTable(DEFAULT_OWNER_TABLE_SIZE);
  borrowTable = new BorrowTable(DEFAULT_BORROW_TABLE_SIZE);
  msgBufferManager= new MsgBufferManager();
  idCounter  = new FatInt();

  Assert(sizeof(CellProxy)==sizeof(CellFrame));
  Assert(sizeof(CellManager)==sizeof(CellFrame));
  Assert(sizeof(CellManager)==sizeof(CellLocal));
  Assert(sizeof(LockProxy)==sizeof(LockFrame));
  Assert(sizeof(LockManager)==sizeof(LockLocal));
  Assert(sizeof(LockManager)==sizeof(LockFrame));
  Assert(sizeof(PortManager)==sizeof(PortLocal));
}

/**********************************************************************/
/*   SECTION 28:: assert stuff                                       */
/**********************************************************************/

Bool withinBorrowTable(int i){
  if(i<borrowTable->getSize()) return OK;
  return NO;}


/**********************************************************************/
/*   SECTION 29:: MISC                                                */
/**********************************************************************/

void marshalSite(Site *s,MsgBuffer *buf){
	s->marshalSite(buf);}


/**********************************************************************/
/*   SECTION 30:: NETWORK                                             */
/**********************************************************************/

