/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

//  protocol and message layer

/* ***************************************************************************
   ***************************************************************************
                       ORGANIZATION

            1  forward declarations
            2  global variables
            3  utility routines
            4  class PendThread
            5  class ProtocolObject
            6  class GNameTable & gname routines
            7  class OB_Entry Owner/Borrower common
            8  class NetAddress & class NetHashTable
            9  class OwnerCreditExtension
            10  class OwnerEntry
            11 class OwnerTable
            12 class BorrowCreditExtension
            13 creditExtension methods
            14 class BorrowEntry
            15 class BorrowTable
            16 div small routines
            17 Pending Thread control utility routines
            18 garbage collection
            19 globalizing
            20 localizing
            21 marshaling/unmarshaling by protocol-layer
            22 main receive msg
            23 remote send protocol
            24 port protocol
            25 variable protocol
            26 object protocol
            27 credit protocol
            28 cell protocol - receive
            29 cell protocol - send
            30 cell protocol - basics
            31 chain routines
            32 chain protocol
            33 lock protocol - receive
            34 lock protocol - send
            35 lock protocol - basics
            36 error msgs
            37 handlers/watchers
            38 error
            39 probes
            40 commincation problem
            41 builtins
            42 initialization
            43 misc

   **************************************************************************
   **************************************************************************/

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
#include "chain.hh"

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
void sendPrimaryCredit(Site *sd,int OTI,Credit c);
void sendSecondaryCredit(Site* s,Site* site,int index,Credit c);

void cellLockSendForward(Site *toS,Site *rS,int mI);
void cellLockSendDump(BorrowEntry*);
void cellLockReceiveForward(BorrowEntry*,Site*,Site*,int);
void cellLockReceiveDump(Tertiary*,Site *);
void cellLockReceiveGet(OwnerEntry*,int,Site*,int);

void cellReceiveDump(CellManager*,Site *);
void cellReceiveContentsManager(OwnerEntry*,TaggedRef,int);
void cellReceiveContentsFrame(BorrowEntry*,TaggedRef,Site*,int);
void cellReceiveGet(OwnerEntry*,Tertiary*,int,Site*,int);
void cellReceiveForward(BorrowEntry*,Tertiary *,Site*,Site*,int);
void cellReceiveRead(OwnerEntry*,int,Site*);
void cellReceiveRemoteRead(CellFrame*,Site*,int,Site*);
void cellReceiveReadAns(Tertiary*,TaggedRef);
void cellReceiveCantPut(CellManager*,TaggedRef,int,Site*,Site*);
void cellSendReadAns(Site*,Site*,int,TaggedRef);
void cellSendRemoteRead(Site*,Site*,int,Site*);
void cellSendRead(BorrowEntry*);
void cellSendContents(TaggedRef,Site*,Site*,int);

void lockReceiveGet(OwnerEntry*,Tertiary*,int,Site*,int);
void lockReceiveForward(BorrowEntry*,Tertiary*,Site *,Site*,int);
void lockSendLock(Site*,int,Site*);
void mainLockReceiveLock(Site*,int);
void cellReceiveContentsManager(OwnerEntry*,TaggedRef,int);
void cellSendContents(TaggedRef,Site*,Site*,int,int);

Tertiary* getTertiaryFromOTI(int);
OwnerEntry* getOwnerEntryFromOTI(int);
Site* getSiteFromBTI(int);

void cellSendForwardFailure(Site*, int);
void cellSendContentsFailure(TaggedRef,Site*,Site*,int);
void lockReceiveDump(LockManager*,Site*);
void lockReceiveLock(LockFrame*);
void lockReceiveCantPut(LockManager *cm,int mI,Site* rsite, Site* dS);

void receiveTellError(Tertiary*,Site*,int,EntityCond);
void chainReceiveAck(OwnerEntry*, Site*, int);
void chainReceiveAnswer(OwnerEntry*,Site*,int,int);
void chainReceiveQuestion(BorrowEntry*,Site*,int,int);
void chainSendAnswer(Site*,int,int,int);
void chainSendQuestion(Site*,int,int,int);
void chainSendAck(Site*,int,int);

void lockSendForward(Site *toS,Site *fS,int mI);
void lockSendForwardFailure(Site*, int);
void lockSendTokenFailure(Site*,Site*,int);
void lockSendDump(BorrowEntry*,LockFrame*);

void sendRegister(BorrowEntry *);

OZ_C_proc_proto(BIapply);

void sendObject(Site* sd, Object *o, Bool);

PERDIO_DEBUG_DO(void printTables());



#define OT ownerTable
#define BT borrowTable
#define BTOS(A) BT->getOriginSite(A)
#define BTOI(A) BT->getOriginIndex(A)


Bool withinBorrowTable(int i); // for assertion

void tokenLost(Chain*, Tertiary *t);
void tokenRecovery(Chain*, Tertiary *);

/* *********************************************************************/
/*   SECTION 2: global variables                                       */
/* *********************************************************************/

#define OWNER_ACCESS_NR (0-1)

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
  "owner_sec_credit",
  "borrow_credit",
  "register",
  "redirect",
  "acknowledge",
  "surrender",

  "cell_lock_get",
  "cell_lock_forward",
  "cell_lock_dump",

  "cell_contents",
  "cell_read",
  "cell_remoteread",
  "cell_readans",
  "cell_cantput",

  "lock_token",
  "lock_cantput",

  "file",
  "chain_ack",
  "chain_question",
  "chain_answer",
  "ask_error"
  "tell_error",
  "get_object",
  "get_objectandclass",

  "send_object",
  "send_objectandclass",
  "register virtual site",
  "init virtual site"
};

/* *********************************************************************/
/*   SECTION 3:: Utility routines                                      */
/* *********************************************************************/

inline void SendTo(Site *toS,MsgBuffer *bs,MessageType mt,Site *sS,int sI){
  int ret=toS->sendTo(bs,mt,sS,sI);
  if(ret==ACCEPTED) return;
  if(ret==PERM_NOT_SENT){
    toS->communicationProblem(mt,sS,sI,COMM_FAULT_PERM_NOT_SENT,(FaultInfo) bs);}
  Assert(ret!=ACCEPTED);
  NOT_IMPLEMENTED;}

inline Bool SEND_SHORT(Site* s){
  if(s->siteStatus()==PERM_SITE) {return OK;}
  return NO;}

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
  Assert(am.onToplevel());
  Thread *th=am.mkRunnableThread(DEFAULT_PRIORITY,am.currentBoard());
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

Chain * tertiaryGetChain(Tertiary*t){
  if(t->getType()==Co_Cell){
    return ((CellManager*)t)->getChain();}
  Assert(t->getType()==Co_Lock);
  return ((LockManager*)t)->getChain();}



/* *********************************************************************/
/*   SECTION 4::  class PendThread                                     */
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
/*   SECTION 5:: class ProtocolObject                                 */
/**********************************************************************/

enum PO_TYPE {
  PO_Var,
  PO_Tert,
  PO_Ref,
  PO_Free
};

/* possibilities     PO_NONE
                     PO_EXTENDED | PO_BIGCREDIT                              (owner)
                     PO_EXTENDED | PO_MASTER                                 (borrow)
                     PO_EXTENDED | PO_MASTER | PO_BIGCREDIT                  (borrow)
                     PO_EXTENDED | PO_SLAVE  | PO_MASTER | PO_BIGCREDIT      (borrow) */

enum PO_FLAGS{
  PO_NONE=0,
  PO_EXTENDED=1,
  PO_BIGCREDIT=2,
  PO_MASTER=4,
  PO_SLAVE=8,
  PO_PERSISTENT=16,
  PO_BORROW_GC_MARK=32
};

class ProtocolObject {
  short type;
  unsigned short flags;
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

  void mkTertiary(Tertiary *t){
    type = PO_Tert; u.tert=t; flags=PO_NONE; }
  void mkTertiary(Tertiary *t,unsigned short f){
    type = PO_Tert; u.tert=t; flags=f; }

  void mkRef(TaggedRef v){
    type=PO_Ref; u.ref=v; flags=PO_NONE; }

  void mkVar(TaggedRef v){
    type=PO_Var; u.ref=v; flags=PO_NONE; }
  void mkVar(TaggedRef v,unsigned short f){
    type=PO_Var; u.ref=v; flags=f;}

  void mkRef(){
    Assert(isVar()); type=PO_Ref;flags=PO_NONE; }

  unsigned short getFlags()         {return flags;}
  void setFlags(unsigned short f)   {flags=f;}
  void removeFlags(unsigned short f) {flags = flags & (~f);}
  void addFlags(unsigned short f)    {flags = flags | f;}

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
/*   SECTION 6::  GNames                                                  */
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

TaggedRef findGNameOutline(GName *gn)
{
  return findGName(gn);
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
/*   SECTION 7:: Owner/Borrower common                                    */
/* ********************************************************************** */

#define INFINITE_CREDIT            (0-1)

#ifdef DEBUG_CREDIT
#define START_CREDIT_SIZE        (256)
#define OWNER_GIVE_CREDIT_SIZE   (16)
#define BORROW_MIN               (2)
#define BORROW_GIVE_CREDIT_SIZE  (4)
#define BORROW_LOW_THRESHOLD     (8)
#define BORROW_HIGH_THRESHOLD    (64)


#else

#define START_CREDIT_SIZE        ((1<<30)-1)
#define OWNER_GIVE_CREDIT_SIZE   ((1<<14))
#define BORROW_MIN               (2)
#define BORROW_GIVE_CREDIT_SIZE  ((1<<7))
#define BORROW_LOW_THRESHOLD     ((1<<4))
#define BORROW_HIGH_THRESHOLD    ((1<<19))
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

class OwnerCreditExtension;
class BorrowCreditExtension;

class OB_Entry : public ProtocolObject {
protected:
  union {
    Credit credit;
    int nextfree;
    OwnerCreditExtension *oExt;
    BorrowCreditExtension *bExt;
  } uOB;

  void makeFree(int next) {
    setFree();
    uOB.nextfree=next;}

  int getNextFree(){
    Assert(isFree());
    return uOB.nextfree;}

  Bool isExtended(){
    if(getFlags() & PO_EXTENDED) return OK;
    return NO;}

  void initCreditOB(Credit c) {
    uOB.credit=START_CREDIT_SIZE;}

  void addCreditOB(Credit c) {
    Assert(!isExtended());
    uOB.credit += c;
    Assert(uOB.credit<=(START_CREDIT_SIZE));}

  void subCreditOB(Credit c) {
    Assert(uOB.credit>c);
    Assert(!isExtended());
    uOB.credit -=c;}

  void setCreditOB(Credit c){
    uOB.credit=c;}

public:

  Credit getCreditOB(){
    Assert(!isExtended());
    Assert(!isFree());
    return uOB.credit;}

  void print();
  Bool isPersistent(){return getFlags()& PO_PERSISTENT;}
  void makePersisent(){addFlags(PO_PERSISTENT);}
};

/* ********************************************************************** */
/*   SECTION 8: NetAddress & NetHashTable                                 */
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
/*   SECTION 9:: OwnerCreditExtension                                    */
/* ********************************************************************** */


enum ReduceCode{
 CANNOT_REDUCE,
   CAN_REDUCE_LAST,
   CAN_REDUCE_SINGLE};

class OwnerCreditExtension{
friend class OB_Entry;
  Credit credit[2];
  OwnerCreditExtension *next;

protected:

public:
  OwnerCreditExtension(){}
  void init(Credit);
  void requestCreditE(Credit);
  ReduceCode addCreditE(Credit);
  ReduceCode isReducible();
  OwnerCreditExtension* getNext(){return next;}
  void reduceLast();
  Credit reduceSingle(){
    Assert(credit[1]==START_CREDIT_SIZE);
    Assert(next==NULL);
    return credit[0];}
  void expand();

};

void OwnerCreditExtension::init(Credit c){
  PD((CREDIT,"owner credit extension init %d",c));
  credit[0]=c;
  credit[1]=START_CREDIT_SIZE;
  next=NULL;}

void OwnerCreditExtension::requestCreditE(Credit req){
  if(credit[0]>=req) {
    PD((CREDIT,"request from owner credit extension credit[0]"));
    credit[0] -= req;
    return;}
  if(credit[1]!=0){
    credit[1]--;
    PD((CREDIT,"request from owner credit extension credit[1]"));
    credit[0] += START_CREDIT_SIZE - req;
    return;}
  if(next==NULL){
    expand();
    requestCreditE(req);
    return;}
  next->requestCreditE(1);
  credit[1]=START_CREDIT_SIZE;
  requestCreditE(req);}

ReduceCode OwnerCreditExtension::isReducible(){ // called from first
  OwnerCreditExtension* oce=next;
  if(oce==NULL){
    if(credit[1]!=START_CREDIT_SIZE) {return CANNOT_REDUCE;}
    return CAN_REDUCE_SINGLE;}
  while(oce->next!=NULL) {oce=oce->next;}
  if(credit[0]!=START_CREDIT_SIZE) {return CANNOT_REDUCE;}
  if(credit[1]!=START_CREDIT_SIZE) {return CANNOT_REDUCE;}
  return CAN_REDUCE_LAST;}

ReduceCode OwnerCreditExtension::addCreditE(Credit ret){
  if(credit[0]+ret>START_CREDIT_SIZE){
    credit[0]= credit[0]-START_CREDIT_SIZE+ret;
    if(credit[1]==START_CREDIT_SIZE){
      ReduceCode rc=next->addCreditE(1);
      if(rc==CANNOT_REDUCE) {return CANNOT_REDUCE;}
      return CAN_REDUCE_LAST;}
    credit[1]++;
    if(credit[1]<START_CREDIT_SIZE){return CANNOT_REDUCE;}
    if(next==NULL) {return CAN_REDUCE_SINGLE;}
    return CANNOT_REDUCE;}
  credit[0]+=ret;
  return CANNOT_REDUCE;}

/* ********************************************************************** */
/*   SECTION 10:: OwnerEntry                                               */
/* ********************************************************************** */

class OwnerEntry: public OB_Entry {
friend class OwnerTable;
private:

  OwnerCreditExtension* getOwnerCreditExtension(){
    Assert(isExtended());
    return uOB.oExt;}

  void setOwnerCreditExtension(OwnerCreditExtension* oce){
    Assert(isExtended());
    uOB.oExt=oce;}

  void extend();

  void requestCredit(Credit req){
    Assert(!isPersistent());
    if(isExtended()){
      getOwnerCreditExtension()->requestCreditE(req);
      return;}
    Credit credit=getCreditOB();
    if(credit<=req){
      extend();
      requestCredit(req);
      return;}
    setCreditOB(credit-req);
    return;}

  void addCreditExtended(Credit);

  void addCredit(Credit back){
    if(isPersistent()) return;
    if(isExtended()){
      addCreditExtended(back);
      return;}
    addCreditOB(back);
    return;}

public:

  void returnCreditOwner(Credit c) {
    addCredit(c);}

  Bool hasFullCredit(){
    if(isExtended()) return NO;
    Credit c=getCreditOB();
    Assert(c<=START_CREDIT_SIZE);
    if(c<START_CREDIT_SIZE) return NO;
    return OK;}

  Credit getSendCredit() {
    if(isPersistent()) {return INFINITE_CREDIT;}
    requestCredit(OWNER_GIVE_CREDIT_SIZE);
    return OWNER_GIVE_CREDIT_SIZE;}

  void getOneCreditOwner() {
    if(isPersistent()) return;
    requestCredit(1);
    return;}

  Credit giveMoreCredit() {
    requestCredit(OWNER_GIVE_CREDIT_SIZE);
    return OWNER_GIVE_CREDIT_SIZE;}

  void removeExtension();
  void makePersistentOwner();
  void receiveCredit(int i){
    if(creditSite==NULL){
      addCredit(1);
      return;}
    sendSecondaryCredit(creditSite,mySite,i,1);
    creditSite=NULL;}
};


void OwnerEntry::addCreditExtended(Credit back){
  ReduceCode rc=getOwnerCreditExtension()->addCreditE(back);
  if(rc==CANNOT_REDUCE) return;
  if(rc==CAN_REDUCE_LAST){
    getOwnerCreditExtension()->reduceLast();
    return;}
  Assert(rc==CAN_REDUCE_SINGLE);
  removeExtension();
  removeFlags(PO_EXTENDED);}

void OwnerEntry::makePersistentOwner(){
  if(isExtended()){
    removeExtension();
    removeFlags(PO_EXTENDED);}
  addFlags(PO_PERSISTENT);
}

/* ********************************************************************** */
/*   SECTION 11:: OwnerTable                                               */
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

  OwnerEntry* getEntry(int i){
    Assert(i<=size);
    if(array[i].isFree()) return NULL;
    return &array[i];}

  void gcOwnerTable();

  void resize();

  int newOwner(OwnerEntry *&);

  void freeOwnerEntry(int);

  void ownerCheck(OwnerEntry*,int);
};

void OwnerTable::ownerCheck(OwnerEntry *oe,int OTI)
{
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
        SVariable *svar = new SVariable(GETBOARD(pvar));
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
  oe->setCreditOB(START_CREDIT_SIZE);
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
/*   SECTION 12:: BorrowCreditExtension                                   */
/* ********************************************************************** */

class BorrowCreditExtension{
friend class BorrowEntry;
  union{
  Credit secCredit;          // SecSlave || (SecMaster && ~BigCredit)
  OwnerCreditExtension *oce; // SecMaster && BigCredit
  BorrowCreditExtension *bce; // SecSlave
}uSOB;
  Credit primCredit;
  Site* site;                // non-NULL SecSlave SecMaster



protected:

  BorrowCreditExtension* getMaster(){return uSOB.bce;}
  OwnerCreditExtension* getBig(){return uSOB.oce;}
  Site *getSite(){return site;}

  void initMaster(Credit cred){
    uSOB.secCredit=START_CREDIT_SIZE;
    primCredit=cred;
    site=NULL;}

  void initSlave(Credit pc,Credit sc,Site *s){
    uSOB.secCredit=sc;
    primCredit=pc;
    site=s;}

  Bool getSecCredit_Master(Credit); // called from Master - can expand to Big
  Bool getSmall_Slave(Credit &);    // called from Slave - can expand to Master
  Bool getOne_Slave();              // called from Slave - can expand to Master

  Credit reduceSlave(Credit,Site* &,Credit &); // called from Slave - before removal
  Credit addPrimaryCredit_Master(Credit); // called from Master
  Credit addPrimaryCredit_SlaveMaster(Credit); // called from Master

  Credit msGetPrimCredit(){return primCredit;}
  Credit slaveGetSecCredit(){return uSOB.secCredit;}
  Credit masterGetSecCredit(){return uSOB.secCredit;}

  void msSetPrimCredit(Credit c){primCredit=c;}
  void slaveSetSecCredit(Credit c){uSOB.secCredit=c;}
  void masterSetSecCredit(Credit c){uSOB.secCredit=c;}

  Bool isReducibleSlave() {return (primCredit>BORROW_MIN)||(primCredit==0);}
  Bool isReducibleMaster(){return uSOB.secCredit==START_CREDIT_SIZE;}
  Bool isReducibleBig(){return uSOB.oce->isReducible();}

  void expandSlave();    // called from Slave
  void expandMaster();   // called from Master

public:
  BorrowCreditExtension(){}
};

/* ********************************************************************** */
/*   SECTION 13:: BorrowCreditExtension methods                           */
/* ********************************************************************** */

inline OwnerCreditExtension* newOwnerCreditExtension(){
  return (OwnerCreditExtension*) genFreeListManager->getOne_3();}

inline BorrowCreditExtension* newBorrowCreditExtension(){
  return (BorrowCreditExtension*) genFreeListManager->getOne_3();}

inline void freeOwnerCreditExtension(OwnerCreditExtension* oce){
  genFreeListManager->putOne_3((FreeListEntry*)oce);}

inline void freeBorrowCreditExtension(BorrowCreditExtension* bce){
  genFreeListManager->putOne_3((FreeListEntry*)bce);}

/* expand */

void BorrowCreditExtension::expandSlave(){
  BorrowCreditExtension* master=newBorrowCreditExtension();
  master->initMaster(slaveGetSecCredit());
  uSOB.bce=master;}

void BorrowCreditExtension::expandMaster(){
  OwnerCreditExtension* oce=newOwnerCreditExtension();
  oce->init(uSOB.secCredit);
  uSOB.oce=oce;
  return;}

/* get secondary credit */

Bool BorrowCreditExtension::getSecCredit_Master(Credit cred){
  Credit c=masterGetSecCredit();
  if(c>cred){
    masterSetSecCredit(c-cred);
    return NO;}
  expandMaster();
  return OK;}

Bool BorrowCreditExtension::getSmall_Slave(Credit &cred){
  Credit c=slaveGetSecCredit();
  if(c>BORROW_GIVE_CREDIT_SIZE){
    cred=BORROW_GIVE_CREDIT_SIZE;
    slaveSetSecCredit(c-BORROW_GIVE_CREDIT_SIZE);
    return NO;}
  expandSlave();
  return OK;}

Bool BorrowCreditExtension::getOne_Slave(){
  Credit c=slaveGetSecCredit();
  if(c>1){
    slaveSetSecCredit(c-1);
    return NO;}
  expandSlave();
  return OK;
}

/* reduce */

Credit BorrowCreditExtension::reduceSlave(Credit more,Site* &s,Credit &secCredit){
  Assert(msGetPrimCredit()+more < BORROW_HIGH_THRESHOLD);
  s=site;
  secCredit=slaveGetSecCredit();
  return msGetPrimCredit()+more;}

/* add PrimaryCredit */

Credit BorrowCreditExtension::addPrimaryCredit_Master(Credit more){
  Assert(more!=INFINITE_CREDIT);
  Credit c=msGetPrimCredit()+more;
  if(c>BORROW_HIGH_THRESHOLD) {
    msSetPrimCredit(BORROW_HIGH_THRESHOLD);
    return c-BORROW_HIGH_THRESHOLD;}
  msSetPrimCredit(c);
  return 0;}

void OwnerCreditExtension::expand(){
  OwnerCreditExtension *oce=newOwnerCreditExtension();
  oce->init(0);
  next=oce;}

void OwnerCreditExtension::reduceLast(){
  OwnerCreditExtension *oce=getNext();
  OwnerCreditExtension *tmp;
  while(oce->next->next!=NULL){
    oce=oce->next;}
  tmp=oce->next;
  Assert(tmp->credit[0]==START_CREDIT_SIZE);
  Assert(tmp->credit[1]==START_CREDIT_SIZE);
  Assert(tmp->next==NULL);
  freeOwnerCreditExtension(tmp);
  oce->next=NULL;}

void OwnerEntry::extend(){
  OwnerCreditExtension* oce=newOwnerCreditExtension();
  oce->init(getCreditOB());
  addFlags(PO_EXTENDED|PO_BIGCREDIT);
  setOwnerCreditExtension(oce);}

void OwnerEntry::removeExtension(){
  OwnerCreditExtension *oce=getOwnerCreditExtension();
  setCreditOB(oce->reduceSingle());
  freeOwnerCreditExtension(oce);
  return;}


/* **********************************************************************  */
/*   SECTION 14:: BorrowEntry                                              */
/* **********************************************************************  */

// START_CREDIT_SIZE
// BORROW_HIGH_THRESHOLD
// BORROW_GIVE_CREDIT_SIZE
// BORROW_LOW_THRESHOLD

class BorrowEntry: public OB_Entry {
friend class BorrowTable;
friend class BorrowCreditExtension;
private:
  NetAddress netaddr;

  BorrowCreditExtension* getSlave(){
    Assert(getFlags() & PO_SLAVE);
    return uOB.bExt;}

  BorrowCreditExtension* getMaster(){
    Assert(getFlags() & PO_MASTER);
    Assert(!(getFlags() & PO_SLAVE));
    return uOB.bExt;}


  Bool getOnePrimaryCredit_E();
  Credit getSmallPrimaryCredit_E();

  void thresholdCheck(Credit c){
    if((getCreditOB()+c>BORROW_LOW_THRESHOLD) &&
       (getCreditOB()<=BORROW_LOW_THRESHOLD)){
      moreCredit();}}

  void removeSoleExtension(Credit);

  void createSecMaster();
  void removeSlave();
  void createSecSlave(Credit,Site *);

  Credit extendGetPrimCredit(){
    Assert(getSlave()==getMaster());
    return getSlave()->msGetPrimCredit();}

  void extendSetPrimCredit(Credit c){
    Assert(getSlave()==getMaster());
    getSlave()->msSetPrimCredit(c);}

  void generalTryToReduce();
  void giveBackSecCredit(Site *,Credit);
  void removeBig(BorrowCreditExtension*);
  void removeMaster_SM(BorrowCreditExtension*);
  void removeMaster_M(BorrowCreditExtension*);

  void addSecCredit_MasterBig(Credit,BorrowCreditExtension *);
  void addSecCredit_Master(Credit,BorrowCreditExtension *);
  Bool addSecCredit_Slave(Credit,BorrowCreditExtension *);

public:

  int getExtendFlags(){
    return getFlags() & (~PO_BORROW_GC_MARK|PO_EXTENDED);}

  void print();
  void makeGCMark(){addFlags(PO_BORROW_GC_MARK);}
  Bool isGCMarked(){ return (getFlags() & PO_BORROW_GC_MARK); }
  void removeGCMark(){removeFlags(PO_BORROW_GC_MARK);}

  void gcBorrow1(int);
  void gcBorrow2(int);

  void copyBorrow(BorrowEntry* from,int i);

  void initBorrow(Credit c,Site* s,int i){
    Assert(isFree());
    if(c==INFINITE_CREDIT){
      addFlags(PO_PERSISTENT);
      setCreditOB(0);}
    else{
      setCreditOB(c);}
    unsetFree();
    netaddr.set(s,i);
    return;}

  void initSecBorrow(Site*,Credit,Site*,int);

  NetAddress* getNetAddress() {
    Assert(!isFree());
    return &netaddr;}

  Site *getSite(){return netaddr.site;}

  int getOTI(){return netaddr.index;}

  void addPrimaryCreditExtended(Credit c);
  void addSecondaryCredit(Credit c,Site *s);

  void addPrimaryCredit(Credit c){
    if(isExtended()) {
      addPrimaryCreditExtended(c);
      return;}
    if (isPersistent()) return;
    Credit cur=getCreditOB();
    PD((CREDIT,"borrow add s:%s o:%d add:%d to:%d",getNetAddress()->site->stringrep(),
        getNetAddress()->index,c,cur));
    if(cur>BORROW_HIGH_THRESHOLD){
      giveBackCredit(cur-BORROW_HIGH_THRESHOLD);
      setCreditOB(BORROW_HIGH_THRESHOLD);
      return;}
    setCreditOB(cur+c);}

  Bool getOnePrimaryCredit(){
    if(isExtended()) {
      return getOnePrimaryCredit_E();}
    if(isPersistent()) {return OK;}
    Credit tmp=getCreditOB();
    Assert(tmp>0);
    if(tmp-1<BORROW_MIN) {
      PD((SPECIAL,"low credit %d",tmp));
      return NO;}
    setCreditOB(tmp-1);
    thresholdCheck(1);
    return OK;}

  Credit getSmallPrimaryCredit(){
    if(isExtended()){
      return getSmallPrimaryCredit_E();}
    if(isPersistent()) {
      return INFINITE_CREDIT;}
    Credit tmp=getCreditOB();
    Assert(tmp>0);
    if(tmp-BORROW_GIVE_CREDIT_SIZE >= BORROW_MIN){
      setCreditOB(tmp-BORROW_GIVE_CREDIT_SIZE);
      thresholdCheck(BORROW_GIVE_CREDIT_SIZE);
      return BORROW_GIVE_CREDIT_SIZE;}
    PD((SPECIAL,"low credit %d",tmp));
    if(tmp-2>=BORROW_MIN){
      setCreditOB(tmp-2);
      thresholdCheck(2);
      return 2;}
    return 0;}

  Site *getOneSecondaryCredit();
  Site *getSmallSecondaryCredit(Credit &);

  void freeBorrowEntry();
  void giveBackCredit(Credit c);
  void moreCredit();

  void makePersistentBorrow(){
    if(isPersistent()) {return;}
    if(isExtended()) {
      makePersistent_E();
      return;}
    addFlags(PO_PERSISTENT);}

  void makePersistent_E();

  void makePersistentBorrowXX() {Assert(0);error("dont understand");}

  void receiveCredit(){
    if(creditSite==NULL){
      addPrimaryCredit(1);
      return;}
    addSecondaryCredit(1,creditSite);
    creditSite=NULL;}

void getOneMsgCredit(){
  if(getOnePrimaryCredit()){
    Assert(creditSite==NULL);
    return;}
  creditSite=getOneSecondaryCredit();
  return;}
};

/* ********************** private **************************** */

void BorrowEntry::initSecBorrow(Site *cs,Credit c,Site *s,int i){
  Assert(isFree());
  Assert(c!=INFINITE_CREDIT);
  unsetFree();
  netaddr.set(s,i);
  createSecSlave(c,cs);
}

void BorrowEntry::makePersistent_E(){
  switch(getExtendFlags()){
  case PO_MASTER:
  case PO_MASTER|PO_BIGCREDIT:
  case PO_SLAVE|PO_MASTER|PO_BIGCREDIT:
  case PO_SLAVE|PO_MASTER:{
    addFlags(PO_PERSISTENT);
    return;}
  case PO_SLAVE:{
    addFlags(PO_PERSISTENT);
    removeSlave();
    return;}
  default:
  Assert(0);}
}

void BorrowEntry::removeSoleExtension(Credit c){
  BorrowCreditExtension* bce=getSlave();
  freeBorrowCreditExtension(bce);
  removeFlags(PO_MASTER|PO_SLAVE|PO_EXTENDED);
  setCreditOB(c);}

void BorrowEntry::createSecMaster(){
  BorrowCreditExtension *bce=newBorrowCreditExtension();
  bce->initMaster(getCreditOB());
  setFlags(PO_MASTER|PO_EXTENDED);}

void BorrowEntry::createSecSlave(Credit cred,Site *s){
  BorrowCreditExtension *bce=newBorrowCreditExtension();
  bce->initSlave(getCreditOB(),cred,s);
  setFlags(PO_SLAVE|PO_EXTENDED);}

Bool BorrowEntry::getOnePrimaryCredit_E(){
  Credit c=extendGetPrimCredit();
  if(c>BORROW_MIN){
    extendSetPrimCredit(c-1);
    return OK;}
  return NO;}

Credit BorrowEntry::getSmallPrimaryCredit_E(){
  Credit c=extendGetPrimCredit();
  if(c>BORROW_MIN+BORROW_GIVE_CREDIT_SIZE){
    extendSetPrimCredit(c-BORROW_GIVE_CREDIT_SIZE);
    return BORROW_GIVE_CREDIT_SIZE;}
  if(c>BORROW_MIN+2){
    extendSetPrimCredit(c-2);
    return 2;}
  return 0;}

/* ********************** public **************************** */


Site* BorrowEntry::getSmallSecondaryCredit(Credit &cred){
  while(TRUE){
    switch(getExtendFlags()){
    case PO_SLAVE|PO_MASTER|PO_BIGCREDIT:{
      cred=OWNER_GIVE_CREDIT_SIZE;
      getSlave()->getMaster()->getBig()->requestCreditE(OWNER_GIVE_CREDIT_SIZE);
      return mySite;}
    case PO_SLAVE|PO_MASTER:{
      cred=OWNER_GIVE_CREDIT_SIZE;
      if(getSlave()->getMaster()->getSecCredit_Master(OWNER_GIVE_CREDIT_SIZE)){
        addFlags(PO_BIGCREDIT);}
      return mySite;}
    case PO_MASTER:{
      cred=OWNER_GIVE_CREDIT_SIZE;
      if(getMaster()->getSecCredit_Master(OWNER_GIVE_CREDIT_SIZE)){
        addFlags(PO_BIGCREDIT);}
      return mySite;}
    case PO_MASTER|PO_BIGCREDIT:{
      cred=OWNER_GIVE_CREDIT_SIZE;
      getMaster()->getBig()->requestCreditE(OWNER_GIVE_CREDIT_SIZE);
      return mySite;}
    case PO_SLAVE:{
      if(getSlave()->getSmall_Slave(cred)){
        addFlags(PO_MASTER);}
      return getSlave()->getSite();}
    case PO_NONE:{
      createSecMaster();
      break;}
    default:{
      Assert(0);
      return NULL;}
    }
  }
}

Site* BorrowEntry::getOneSecondaryCredit(){
  while(TRUE){
    switch(getExtendFlags()){
    case PO_SLAVE|PO_MASTER|PO_BIGCREDIT:{
      getSlave()->getMaster()->getBig()->requestCreditE(1);
      return mySite;}
    case PO_SLAVE|PO_MASTER:{
      if(getSlave()->getMaster()->getSecCredit_Master(1)){
        addFlags(PO_BIGCREDIT);
        break;}
      return mySite;}
    case PO_MASTER:{
      if(getMaster()->getSecCredit_Master(1)){
        addFlags(PO_BIGCREDIT);
        break;}
      return mySite;}
    case PO_MASTER|PO_BIGCREDIT:{
      getMaster()->getBig()->requestCreditE(1);
      return mySite;}
    case PO_SLAVE:{
      if(getSlave()->getOne_Slave()){
        addFlags(PO_MASTER);
        break;}
      return getSlave()->getSite();}
    case PO_NONE:{
      createSecMaster();
      break;}
    default:{
      Assert(0);
      return NULL;}
    }
  }
  return NULL; // stupid compiler
}

void BorrowEntry::addPrimaryCreditExtended(Credit c){
  if(c==INFINITE_CREDIT){
    makePersistentBorrow();
    return;}
  Credit overflow;
  switch(getExtendFlags()){
  case PO_SLAVE|PO_MASTER|PO_BIGCREDIT:
  case PO_SLAVE|PO_MASTER:{
    overflow=getSlave()->getMaster()->addPrimaryCredit_Master(c);
    break;}
  case PO_SLAVE:{
    Site *s;
    Credit sec;
    overflow=getSlave()->reduceSlave(c,s,sec);
    removeSoleExtension(overflow);
    giveBackSecCredit(s,sec);
    break;}
  case PO_MASTER|PO_BIGCREDIT:
  case PO_MASTER:{
    overflow=getMaster()->addPrimaryCredit_Master(c);
    break;
  default:
    Assert(0);}}
  if(overflow>0){
    giveBackCredit(overflow);}
}

/************* reduction  all private *****************/

void BorrowEntry::removeBig(BorrowCreditExtension* master){
  OwnerCreditExtension *oce=master->getBig();
  master->masterSetSecCredit(oce->reduceSingle());
  freeOwnerCreditExtension(oce);
  removeFlags(PO_BIGCREDIT);
  generalTryToReduce();}

void BorrowEntry::removeMaster_SM(BorrowCreditExtension* master){
  Assert(!(getExtendFlags() & PO_BIGCREDIT));
  Assert(master->masterGetSecCredit()==START_CREDIT_SIZE);
  BorrowCreditExtension *slave=getSlave();
  Credit c=master->msGetPrimCredit();
  slave->slaveSetSecCredit(c);
  removeFlags(PO_MASTER);
  freeBorrowCreditExtension(master);
  generalTryToReduce();}

void BorrowEntry::removeMaster_M(BorrowCreditExtension* master){
  Assert(!(getExtendFlags() & PO_BIGCREDIT));
  Assert(!(getExtendFlags() & PO_SLAVE));
  Assert(master->masterGetSecCredit()==START_CREDIT_SIZE);
  Credit c=master->msGetPrimCredit();
  removeFlags(PO_MASTER);
  freeBorrowCreditExtension(master);
  setCreditOB(c);}

void BorrowEntry::removeSlave(){
  BorrowCreditExtension* slave=getSlave();
  Credit c=slave->msGetPrimCredit();
  Assert((c>=BORROW_MIN)||(c==0));
  giveBackSecCredit(slave->getSite(),slave->slaveGetSecCredit());
  removeFlags(PO_SLAVE|PO_EXTENDED);
  freeBorrowCreditExtension(slave);
  setCreditOB(c);}

void BorrowEntry::generalTryToReduce(){
  while(TRUE){
    switch(getExtendFlags()){
    case PO_SLAVE|PO_MASTER|PO_BIGCREDIT:{
      if(getSlave()->getMaster()->isReducibleBig()){
        removeBig(getSlave()->getMaster());
        break;}
      return;}

    case PO_SLAVE|PO_MASTER:{
      if(getSlave()->getMaster()->isReducibleMaster()){
        removeMaster_SM(getSlave()->getMaster());
        break;}
      return;}

    case PO_SLAVE:{
      if(getSlave()->isReducibleSlave()){
        removeSlave();}
      return;}

    case PO_MASTER|PO_BIGCREDIT:{
      if(getMaster()->isReducibleBig()){
        removeBig(getMaster());
        break;}
      return;}

    case PO_MASTER:{
      if(getMaster()->isReducibleMaster()){
        removeMaster_M(getMaster());
        break;}
      return;}
    default:{
      Assert(0);}
    }
  }
}

/************* add secondary *****************/

/* private */

void BorrowEntry::addSecCredit_MasterBig(Credit c,BorrowCreditExtension *master){
  ReduceCode rc=master->getBig()->addCreditE(c);
  if(rc==CANNOT_REDUCE) return;
  if(rc==CAN_REDUCE_LAST){
    master->getBig()->reduceLast();
    return;}
  Assert(rc==CAN_REDUCE_SINGLE);
  generalTryToReduce();}

void BorrowEntry::addSecCredit_Master(Credit c,BorrowCreditExtension *master){
  Credit cx=master->masterGetSecCredit()+c;
  master->masterSetSecCredit(cx);
  if(cx==START_CREDIT_SIZE){
    generalTryToReduce();}}

Bool BorrowEntry::addSecCredit_Slave(Credit c,BorrowCreditExtension *slave){
  Credit cx=slave->slaveGetSecCredit()+c;
  if(cx>START_CREDIT_SIZE){
    return NO;}
  slave->slaveSetSecCredit(cx);
  return OK;}

/* public */

void BorrowEntry::addSecondaryCredit(Credit c,Site *s){
  switch(getExtendFlags()){
  case PO_SLAVE|PO_MASTER|PO_BIGCREDIT:{
    if(s==mySite){
      addSecCredit_MasterBig(c,getSlave()->getMaster());
      return;}
    if(s==getSlave()->getSite()){
      if(addSecCredit_Slave(c,getSlave())) {return;}}
    break;}

  case PO_SLAVE|PO_MASTER:{
    if(s==mySite){
      addSecCredit_Master(c,getSlave()->getMaster());
      return;}
    if(s==getSlave()->getSite()){
      if(addSecCredit_Slave(c,getSlave())) {return;}}
    break;}

  case PO_SLAVE:{
    if(s==getSlave()->getSite()){
      if(addSecCredit_Slave(c,getSlave())){return;}}
    break;}

  case PO_MASTER|PO_BIGCREDIT:{
    if(s==mySite){
      addSecCredit_MasterBig(c,getMaster());
      return;}
    break;}

  case PO_MASTER:{
    if(s==mySite){
      addSecCredit_Master(c,getMaster());
      return;}
    break;}
  default:
    Assert(0);
  }
  giveBackSecCredit(s,c);
}

void BorrowEntry::copyBorrow(BorrowEntry* from,int i){
  if (from->isTertiary()) {
    mkTertiary(from->getTertiary(),from->getFlags());
    from->getTertiary()->setIndex(i);
  } else if (from->isVar()) {
    mkVar(from->getRef(),from->getFlags());
    from->getVar()->setIndex(i);
  } else {
    Assert(from->isRef());
    mkRef(from->getRef());
  }
  netaddr.set(from->netaddr.site,from->netaddr.index);
}

void BorrowEntry::moreCredit(){
  Assert(!isExtended());
  Assert(!isPersistent());
  NetAddress *na = getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_ASK_FOR_CREDIT(bs,na->index,mySite);
  SendTo(na->site,bs,M_ASK_FOR_CREDIT,netaddr.site,netaddr.index);
}

void BorrowEntry::giveBackCredit(Credit c){
  Assert(!isPersistent());
  NetAddress *na = getNetAddress();
  Site* site = na->site;
  int index = na->index;
  sendPrimaryCredit(site,index,c);
}

void BorrowEntry::giveBackSecCredit(Site *s,Credit c){
  Assert(!isPersistent());
  NetAddress *na = getNetAddress();
  Site* site = na->site;
  int index = na->index;
  sendSecondaryCredit(s,site,index,c);
}

void BorrowEntry::freeBorrowEntry(){
  Assert(!isExtended());
  if (!isPersistent()) {giveBackCredit(getCreditOB());}}

/* ********************************************************************** */
/*   SECTION 15:: BorrowTable                                              */
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

  BorrowEntry* getEntry(int i){
    Assert(i<=size);
    if(array[i].isFree()) return NULL;
    return &array[i];}

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
  int newSecBorrow(Site*,Credit,Site*,int);

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

int BorrowTable::newSecBorrow(Site *creditSite,Credit c,Site * sd,int off){
  if(nextfree == END_FREE) resize();
  int index=nextfree;
  nextfree= array[index].uOB.nextfree;
  BorrowEntry* oe = &(array[index]);
  oe->initSecBorrow(creditSite,c,sd,off);
  PD((HASH2,"<SPECIAL>:net=%x borrow=%x owner=%x hash=%x",
                oe->getNetAddress(),array,ownerTable->array,
                hshtbl->table));
  hshtbl->add(oe->getNetAddress(),index);
  no_used++;
  PD((TABLE,"borrow insert: b:%d",index));
  return index;}

int BorrowTable::newBorrow(Credit c,Site * sd,int off){
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
  if(b->isExtended()){
    if(b->getExtendFlags() & PO_MASTER) return;
    Assert(b->getExtendFlags()==PO_SLAVE);
    b->removeSlave();}
  Assert(!b->isExtended());
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
  printf("Credit:%ld\n",getCreditOB());
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

Bool withinBorrowTable(int i){
  if(i<borrowTable->getSize()) return OK;
  return NO;}

/* ******************************************************************* */
/*   SECTION 16 :: div small routines                                  */
/* ******************************************************************* */

Chain *newChain(){
  return (Chain*) genFreeListManager->getOne_3();}

ChainElem *newChainElem(){
  return (ChainElem*) genFreeListManager->getOne_3();}

void freeChainElem(ChainElem* e){
  genFreeListManager->putOne_3((FreeListEntry*) e);}

void freeChain(Chain* e){
  genFreeListManager->putOne_3((FreeListEntry*) e);}

InformElem* newInformElem(){
  return (InformElem*) genFreeListManager->getOne_3();}

void freeInformElem(InformElem* e){
  genFreeListManager->putOne_3((FreeListEntry*) e);}

void Chain::init(Site *s, int nr){
  ChainElem *e=newChainElem();
  e->init(s,nr);
  first=last=e;}

void Chain::setCurrent(Site* s, int nr){
  ChainElem *e=newChainElem();
  e->init(s, nr);
  last->setNext(e);
  last= e;}

void CellFrame::addPendBinding(Thread* th,TaggedRef val){
  PendBinding* pb=new PendBinding(val,th,sec->pendBinding);
  sec->pendBinding=pb;}

void CellManager::setCurrent(Site *s, int nr){
  Assert(getChain()->siteListCheck());
  getChain()->setCurrent(s,nr);
  getChain()->installProbes();} /*ATTENTION*/

Site* CellManager::getCurrent(){
  return getChain()->getCurrent();}

void CellManager::setOwnCurrent(){
  setCurrent(mySite, OWNER_ACCESS_NR);}

Bool CellManager::isOwnCurrent(){
  if(getCurrent() == mySite) return TRUE;
  return FALSE;}

void CellManager::init(){
  Chain *cl = newChain();
  cl->init(mySite, OWNER_ACCESS_NR);
  setPtr(cl);}

void LockManager::setOwnCurrent(){
  setCurrent(mySite, OWNER_ACCESS_NR);}

Bool LockManager::isOwnCurrent(){
  if(getCurrent() == mySite) return TRUE;
  return FALSE;}

void LockManager::setCurrent(Site *s, int nr){
  Assert(getChain()->siteListCheck());
  getChain()->setCurrent(s,nr);
  getChain()->installProbes();} /* ATTENTION */

Site* LockManager::getCurrent(){
  return getChain()->getCurrent();}

void LockManager::init(){
  Chain *cl = newChain();
  cl->init(mySite, OWNER_ACCESS_NR);
  sec->chain=cl;}

/* ******************************************************************* */
/*   SECTION 17 :: pending thread utility routines                     */
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
      oz_resumeFromNet(t);}
    tmp=pt;
    pt=pt->next;
    tmp->dispose();}}

inline Thread* pendThreadResumeFirst(PendThread **pt){
  PendThread *tmp=*pt;
  Assert(tmp!=NULL);
  Thread *t=tmp->thread;
  Assert(isRealThread(t));
  PD((THREAD_D,"start thread ResumeFirst %x",t));
  oz_resumeFromNet(t);
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
    oz_suspendOnNet(t);
    PD((THREAD_D,"stop thread addToEnd %x",t));}
  while(*pt!=NULL){pt= &((*pt)->next);}
  *pt=new PendThread(t,NULL);
  return;}

inline void pendThreadAddToNonFirst(PendThread **pt,Thread *t){
  if(isRealThread(t)){
    oz_suspendOnNet(t);
    PD((THREAD_D,"stop thread addToNonFirst %x",t));}
  if(*pt!=NULL){pt= &((*pt)->next);}
  *pt=new PendThread(t,NULL);
  return;}

Bool threadIsPending(PendThread *pt,Thread *t){
  while(pt!=NULL){
    if(pt->thread==t) return OK;
    pt=pt->next;}
  return NO;}

void sendHelpX(MessageType mt,BorrowEntry *be){
  NetAddress* na=be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  if(mt==M_GET_OBJECT){
    marshal_M_GET_OBJECT(bs,na->index,mySite);}
  else{
    Assert(mt==M_GET_OBJECTANDCLASS);
    marshal_M_GET_OBJECTANDCLASS(bs,na->index,mySite);}
  SendTo(na->site,bs,mt,na->site,na->index);}

void PerdioVar::addSuspPerdioVar(TaggedRef *v,Thread *el, int unstable){
  if (suspList!=NULL) {
    addSuspSVar(el,unstable);
    return;  }

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
      pv->addSuspPerdioVar(v,el,unstable);}
    else {
      Assert(isClass(cl));
      BorrowEntry *be=BT->getBorrow(getObject()->getIndex());
      sendHelpX(M_GET_OBJECT,be);}
    return;}

  if (isURL()) {
    OZ_Return ret = loadURL(getURL(),oz_newVariable(),makeTaggedRef(v),am.currentThread()); // ATTENTION
    // BI_PREEMPT is returned when  we fall through to the default
    // method of starting an independent loader process
    switch (ret) {
    case RAISE: {
      prefixError();
      warning("load URL %s raised exception:",toC(getURL()));
      warning("%s",toC(am.getExceptionValue()));
      break;
    }
    case PROCEED:
    case BI_PREEMPT:
      break;
    default:
      warning("mm2: load URL %s failed not impl",toC(getURL()));
    }
 }
}

Site* getSiteFromTertiary(Tertiary* t){
  Assert(t->getTertType()!=Te_Manager);
  return BT->getBorrow(t->getIndex())->getSite();}

/* ******************************************************************* */
/*   SECTION 18::  garbage-collection                                  */
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
  if(be->isGCMarked()){
    PD((GC,"borrow already marked:%d",i));
    return;}
  be->makeGCMark();
  PD((GC,"relocate borrow :%d old:%x new:%x",i,be,this));
  if (be->isTertiary())  /* might be avariable for an object */
    be->mkTertiary(this,be->getFlags());
  return;}

void Tertiary::gcManager(){
  Assert(getTertType()!=Te_Frame);
  int i=getIndex();
  OwnerEntry *oe=OT->getOwner(i);
  PD((GC,"relocate owner:%d old%x new %x",
     i,oe,this));
  oe->mkTertiary(this,oe->getFlags());}

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

void gcPendBindingList(PendBinding **last){
  PendBinding *bl = *last;
  PendBinding *newBL;
  for (; bl; bl = bl->next) {
    newBL = new PendBinding();
    gcTagged(bl->val,newBL->val);
    newBL->thread = bl->thread->gcThread();
    *last = newBL;
    last = &newBL->next;}
  *last=NULL;}

void CellFrame::gcCellFrameSec(){
  int state=getState();
  if(state & Lock_Next){
    getNext()->makeGCMarkSite();}
  PD((GC,"relocate Cell in state %d",state));
  gcPendBindingList(&(sec->pendBinding));
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


void Chain::gcChainSites(){
  ChainElem *ce=first;
  while(ce!=NULL){
    ce->getSite()->makeGCMarkSite();
    ce=ce->getNext();}
}

void CellManager::gcCellManager(){
  getChain()->gcChainSites();
  int i=getIndex();
  PD((GC,"relocate cellManager:%d",i));
  OwnerEntry* oe=OT->getOwner(i);
  oe->mkTertiary(this,oe->getFlags());
  CellFrame *cf=(CellFrame*)this;
  cf->gcCellFrameSec();}

void LockFrame::gcLockFrameSec(){
  int state=getState();
  if(state & Lock_Next){
    getNext()->makeGCMarkSite();}
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
  getChain()->gcChainSites();
  int i=getIndex();
  PD((GC,"relocate lockManager:%d",i));
  OwnerEntry* oe=OT->getOwner(i);
  oe->mkTertiary(this,oe->getFlags());
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
      be->makeGCMark();
      PD((GC,"relocate borrow:%d old %x new %x",i,be,this));
      be->mkTertiary(this,be->getFlags());
      return;
    }
  case Te_Manager:
    {
      int i=getIndex();
      OwnerEntry *oe=OT->getOwner(i);
      PD((GC,"relocate owner:%d old%x new %x",i,oe,this));
      oe->mkTertiary(this,oe->getFlags());
      return;
    }
  case Te_Frame:
    Assert(0);
  }
}

/*--------------------*/

void GName::gcMarkSite(){
  site->makeGCMarkSite();}

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
      makeGCMark();
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
      if (!b->isGCMarked()) {b->gcBorrow1(i);}}
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
      if(!(b->isGCMarked())) {b->gcBorrow2(i);}}
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
        if(b->isGCMarked()) {
          b->removeGCMark(); // marks owner site too
          PD((GC,"BT b:%d mark variable found",i));}
        else{
          PD((GC,"BT b:%d unmarked variable found",i));
          borrowTable->maybeFreeBorrowEntry(i);}}
      else{
        if(b->isGCMarked()){
          b->removeGCMark(); // marks owner site too
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
                if((state & Cell_Valid) && (!(state & Cell_Dump_Asked))){
                  cf->setState(Cell_Valid | Cell_Dump_Asked);
                  cellLockSendDump(b);}}
              break;}
            case Co_Lock:{
              LockFrame *lf=(LockFrame *)t;
              if(lf->isAccessBit()){
                int state=lf->getState();
                lf->resetAccessBit();
                if((state & Lock_Valid) && (!(state & Lock_Dump_Asked))){
                  lf->setState(Lock_Valid | Lock_Dump_Asked);
                  cellLockSendDump(b);}}
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
      gn->site->makeGCMarkSite();
    } else {
      if (gn->getGNameType()==GNT_NAME &&
          tagged2Literal(gn->getValue())->isNamedName()) {
        goto next_one;
      }
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
  if (!be->isGCMarked()) { // this condition is necessary gcBorrow1
    be->makeGCMark();
    be->gcPO();
  }
}


void PerdioVar::gcPerdioVar(void)
{
  if (isProxy()) {
    PD((GC,"PerdioVar b:%d",getIndex()));
    gcBorrowNow(getIndex());
    gcPendBindingList(&u.bindings);}
  else if (isManager()) {
    PD((GC,"PerdioVar o:%d",getIndex()));
    ProxyList **last=&u.proxies;
    for (ProxyList *pl = u.proxies; pl; pl = pl->next) {
      pl->sd->makeGCMarkSite();
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
/*   SECTION 19 :: Globalizing                                        */
/**********************************************************************/

GName *Name::globalize()
{
  if (!hasGName()) {
    Assert(am.isRootBoard(GETBOARD(this)));
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
  cm->init();
  cm->setTertType(Te_Manager);
  cm->setIndex(myIndex);

  CellFrame *cf=(CellFrame*)this;
  TaggedRef v=val;
  PD((CELL,"globalize cell index:%d",myIndex));
  cf->initFromGlobalize(v);


}

void LockLocal::globalize(int myIndex){
  DebugIndexCheck(myIndex);

  LockManager *lm=(LockManager*)this;
  lm->setTertType(Te_Manager);
  lm->setIndex(myIndex);
  LockFrame *lf=(LockFrame*)this;
  Thread *t=getLocker();
  PendThread *pt=getPending();
  PD((LOCK,"globalize lock index:%d",myIndex));
  lf->initFromGlobalize(t,pt);
  lm->init();
}


void Tertiary::globalizeTert()
{
  Assert(isLocal());

  switch(getType()) {
  case Co_Cell:
    {
      OwnerEntry *oe_manager;
      int manI=ownerTable->newOwner(oe_manager);
      PD((GLOBALIZING,"GLOBALIZING cell index:%d",manI));
      oe_manager->mkTertiary(this);
      CellLocal *cl=(CellLocal*)this;
      cl->globalize(manI);
      return;
    }
  case Co_Lock:
    {
      OwnerEntry *oe_manager;
      int manI=ownerTable->newOwner(oe_manager);
      PD((GLOBALIZING,"GLOBALIZING lock index:%d",manI));
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

  setTertType(Te_Manager);
  OwnerEntry *oe;
  int i = ownerTable->newOwner(oe);
  PD((GLOBALIZING,"GLOBALIZING port/object/space/thread index:%d",i));
  if(getType()==Co_Object)
    {PD((SPECIAL,"object:%x class%x",this,((Object *)this)->getClass()));}
  oe->mkTertiary(this);
  setIndex(i);
  if(getType()==Co_Object)
    {PD((SPECIAL,"object:%x class%x",this,((Object *)this)->getClass()));}
}

inline void convertCellProxyToFrame(Tertiary *t){
  Assert(t->getTertType()==Te_Proxy);
  CellFrame *lf=(CellFrame*) t;
  t->setTertType(Te_Frame);
  lf->initFromProxy();}

inline void convertLockProxyToFrame(Tertiary *t){
  Assert(t->getTertType()==Te_Proxy);
  LockFrame *lf=(LockFrame*) t;
  t->setTertType(Te_Frame);
  lf->initFromProxy();}

inline void maybeConvertLockProxyToFrame(Tertiary *t){
  if(t->getTertType()==Te_Proxy){
    convertLockProxyToFrame(t);}}

inline void maybeConvertCellProxyToFrame(Tertiary *t){
  if(t->getTertType()==Te_Proxy){
    convertCellProxyToFrame(t);}}

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
  PD((GLOBALIZING,"globalize var index:%d",i));

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
/*   SECTION 20 :: Localizing                                        */
/**********************************************************************/

void CellManager::localize(){
  CellFrame *cf=(CellFrame *)this;
  Assert(cf->getState()==Cell_Valid);
  TaggedRef tr=cf->getContents();
  setTertType(Te_Local);
  Assert(am.onToplevel());
  setBoard(am.currentBoard());
  CellLocal *cl=(CellLocal*) this;
  cl->setValue(tr);}

void LockManager::localize(){
  LockFrame *lf=(LockFrame *)this;
  Assert(lf->getState()==Lock_Valid);
  Thread *t=lf->getLocker();
  setTertType(Te_Local);
  Assert(am.onToplevel());
  setBoard(am.currentBoard());
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
      Assert(am.onToplevel());
      setBoard(am.currentBoard());
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
    Assert(am.onToplevel());
    setBoard(am.currentBoard());
    return;}
  default:
    Assert(0);
    printf("cannot localize %d\n",getType());
    error("cannot localize\n");
  }
}

/**********************************************************************/
/*   SECTION 21 :: marshaling/unmarshaling by protocol-layer          */
/**********************************************************************/

/* for now credit is a 32-bit word */

inline void marshalCredit(Credit credit,MsgBuffer *bs){
  Assert(sizeof(Credit)==sizeof(long));
  Assert(sizeof(Credit)==sizeof(unsigned int));
  PD((MARSHAL,"credit c:%d",credit));
  marshalNumber(credit,bs);}

void marshalCreditOutline(Credit credit,MsgBuffer *bs)
{
  marshalCredit(credit,bs);
}


inline Credit unmarshalCredit(MsgBuffer *bs){
  Assert(sizeof(Credit)==sizeof(long));
  Credit c=unmarshalNumber(bs);
  PD((UNMARSHAL,"credit c:%d",c));
  return c;}

Credit unmarshalCreditOutline(MsgBuffer *bs)
{
  return unmarshalCredit(bs);
}


inline void marshalOwnHead(int tag,int i,MsgBuffer *bs){
  PD((MARSHAL_CT,"OwnHead"));
  bs->put(tag);
  mySite->marshalSite(bs);
  marshalNumber(i,bs);
  bs->put(DIF_PRIMARY);
  Credit c=ownerTable->getOwner(i)->getSendCredit();
  marshalNumber(c,bs);
  PD((MARSHAL,"ownHead o:%d rest-c:%d ",i,ownerTable->getOwner(i)->getCreditOB()));
  return;}

void marshalToOwner(int bi,MsgBuffer *bs){
  PD((MARSHAL,"toOwner"));
  BorrowEntry *b=BT->getBorrow(bi);
  int OTI=b->getOTI();
  if(b->getOnePrimaryCredit()){
    bs->put((BYTE) DIF_OWNER);
    marshalNumber(OTI,bs);
    PD((MARSHAL,"toOwner Borrow b:%d Owner o:%d",bi,OTI));
    return;}
  bs->put((BYTE) DIF_OWNER_SEC);
  Site* xcs = b->getOneSecondaryCredit();
  marshalNumber(OTI,bs);
  xcs->marshalSite(bs);
  return;}

void marshalBorrowHead(MarshalTag tag, int bi,MsgBuffer *bs){
  PD((MARSHAL,"BorrowHead"));
  bs->put((BYTE)tag);
  BorrowEntry *b=borrowTable->getBorrow(bi);
  NetAddress *na=b->getNetAddress();
  na->site->marshalSite(bs);
  marshalNumber(na->index,bs);
  Credit cred=b->getSmallPrimaryCredit();
  if(cred) {
    PD((MARSHAL,"borrowed b:%d remCredit c:%d give c:%d",bi,b->getCreditOB(),cred));
    bs->put(DIF_PRIMARY);
    marshalCredit(cred,bs);
    return;}
  Site *ss=b->getSmallSecondaryCredit(cred);
  bs->put(DIF_SECONDARY);
  marshalCredit(cred,bs);
  marshalSite(ss,bs);
  return;
}

OZ_Term unmarshalBorrow(MsgBuffer *bs,OB_Entry *&ob,int &bi){
  PD((UNMARSHAL,"Borrow"));
  Site * sd=unmarshalSite(bs);
  int si=unmarshalNumber(bs);
  Credit cred;
  MarshalTag mt=(MarshalTag) bs->get();
  PD((UNMARSHAL,"borrow o:%d",si));
  if(sd==mySite){
    if(mt==DIF_PRIMARY){
      cred = unmarshalCredit(bs);
      PD((UNMARSHAL,"mySite is owner"));
      OwnerEntry* oe=OT->getOwner(si);
      oe->returnCreditOwner(cred);
      OZ_Term ret = oe->getValue();
      OT->ownerCheck(oe,si);
      return ret;}
    Assert(mt==DIF_SECONDARY);
    cred = unmarshalCredit(bs);
    Site* cs=unmarshalSite(bs);
    sendSecondaryCredit(cs,mySite,si,cred);
    PD((UNMARSHAL,"mySite is owner"));
    OwnerEntry* oe=OT->getOwner(si);
    OZ_Term ret = oe->getValue();
    return ret;}
  NetAddress na = NetAddress(sd,si);
  BorrowEntry *b = borrowTable->find(&na);
    if (b!=NULL) {
      PD((UNMARSHAL,"borrow found"));
      cred = unmarshalCredit(bs);
      if(mt==DIF_PRIMARY){
        b->addPrimaryCredit(cred);}
      else{
        Assert(mt==DIF_SECONDARY);
        Site *s=unmarshalSite(bs);
        b->addSecondaryCredit(cred,s);}
      ob = b;
      return b->getValue();}
  cred = unmarshalCredit(bs);
  if(mt==DIF_PRIMARY){
    bi=borrowTable->newBorrow(cred,sd,si);
    b=borrowTable->getBorrow(bi);
    PD((UNMARSHAL,"borrowed miss"));
    ob=b;
    return 0;}
  Assert(mt==DIF_SECONDARY);
  Site* site = unmarshalSite(bs);
  bi=borrowTable->newSecBorrow(site,cred,sd,si);
  b=borrowTable->getBorrow(bi);
  PD((UNMARSHAL,"borrowed miss"));
  ob=b;
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
  PD((MARSHAL,"Tert"));
  Site *sd=bs->getSite();
  switch(t->getTertType()){
  case Te_Local:
    t->globalizeTert();
    // no break here!
  case Te_Manager:
    {
      PD((MARSHAL_CT,"manager"));
      int OTI=t->getIndex();
      marshalOwnHead(tag,OTI,bs);
      if (!sd) {OT->getOwner(OTI)->makePersistentOwner();} // ATTENTION
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
    OwnerEntry* oe=OT->getOwner(OTI);
    oe->returnCreditOwner(1);
    OZ_Term oz=oe->getValue();
    OT->ownerCheck(oe,OTI);
    return oz;}
  Assert(mt==DIF_OWNER_SEC);
  int OTI=unmarshalNumber(bs);
  Site *cs=unmarshalSite(bs);
  sendSecondaryCredit(cs,mySite,OTI,1);
  return OT->getOwner(OTI)->getValue();
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
/*   SECTION 22:: Main Receive                                       */
/**********************************************************************/

void Site::msgReceived(MsgBuffer* bs)
{
  Assert(am.onToplevel());
  MessageType mt = (MessageType) unmarshalHeader(bs);
  PD((MSG_RECEIVED,"msg type %d",mt));

  switch (mt) {
  case M_PORT_SEND:
    {
      int portIndex;
      OZ_Term t;
      unmarshal_M_PORT_SEND(bs,portIndex,t);
      PD((MSG_RECEIVED,"PORTSEND: o:%d v:%s",portIndex,toC(t)));

      OwnerEntry *oe=OT->getOwner(portIndex);
      oe->receiveCredit(portIndex);
      PortManager *pm=(PortManager*)(oe->getTertiary());
      Assert(pm->checkTertiary(Co_Port,Te_Manager));

      LTuple *lt = new LTuple(t,am.currentUVarPrototype());
      OZ_Term old = pm->exchangeStream(lt->getTail());
      PD((SPECIAL,"just after send port"));
      SiteUnify(makeTaggedLTuple(lt),old); // ATTENTION
      OT->ownerCheck(oe,portIndex);
      break;
      }

  case M_REMOTE_SEND:    /* index string term */
    {
      int i;
      char *biName;
      OZ_Term t;
      unmarshal_M_REMOTE_SEND(bs,i,biName,t);
      PD((MSG_RECEIVED,"REMOTE_SEND: o:%d bi:%s v:%s",i,biName,toC(t)));

      OwnerEntry *oe=OT->getOwner(i);
      oe->receiveCredit(i);

      Tertiary *tert= oe->getTertiary();


      BuiltinTabEntry *found = builtinTab.find(biName);

      OT->ownerCheck(oe,i);
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
      unmarshal_M_ASK_FOR_CREDIT(bs,na_index,rsite);
      PD((MSG_RECEIVED,"ASK_FOR_CREDIT index:%d site:%s",na_index,rsite->stringrep()));
      OwnerEntry *oe=OT->getOwner(na_index);
      oe->receiveCredit(na_index);
      Credit c= oe->giveMoreCredit();
      MsgBuffer *bs=msgBufferManager->getMsgBuffer(rsite);
      marshal_M_BORROW_CREDIT(bs,mySite,na_index,c);
      SendTo(rsite,bs,M_BORROW_CREDIT,mySite,na_index);
      break;
    }

  case M_OWNER_CREDIT:
    {
      int index;
      Credit c;
      unmarshal_M_OWNER_CREDIT(bs,index,c);
      PD((MSG_RECEIVED,"OWNER_CREDIT index:%d credit:%d",index,c));
      OwnerEntry *oe=OT->getOwner(index);
      oe->returnCreditOwner(c);
      OT->ownerCheck(oe,index);
      break;
    }

  case M_OWNER_SEC_CREDIT:
    {
    int index;
    Credit c;
    Site *s;
    unmarshal_M_OWNER_SEC_CREDIT(bs,s,index,c);
    PD((MSG_RECEIVED,"OWNER_SEC_CREDIT site:%s index:%d credit:%d",s->stringrep(),index,c));
    NetAddress na=NetAddress(s,index);
    BorrowEntry *b=BT->find(&na);
    Assert(b!=NULL);
    b->addSecondaryCredit(c,mySite);
    }

  case M_BORROW_CREDIT:
    {
      int si;
      Credit c;
      Site *sd;
      unmarshal_M_BORROW_CREDIT(bs,sd,si,c);
      PD((MSG_RECEIVED,"BORROW_CREDIT site:%s index:%d credit:%d",sd->stringrep(),si,c));
      NetAddress na=NetAddress(sd,si);
      BorrowEntry *b=borrowTable->find(&na);
      Assert(b!=NULL);
      b->addPrimaryCredit(c);
      break;
    }

  case M_REGISTER:
    {
      int OTI;
      Site *rsite;
      unmarshal_M_REGISTER(bs,OTI,rsite);
      PD((MSG_RECEIVED,"REGISTER index:%d site:%s",OTI,rsite->stringrep()));
      OwnerEntry *oe=OT->getOwner(OTI);
      oe->receiveCredit(OTI);
      if (oe->isVar()) {
        PerdioVar *pv=oe->getVar();
        if (!pv->isRegistered(rsite)) {
          pv->registerSite(rsite);}
        else {
          PD((WEIRD,"REGISTER o:%d s:%s already registered",OTI,rsite->stringrep()));}}
      else {
        sendRedirect(rsite,OTI,OT->getOwner(OTI)->getRef());}
      OT->ownerCheck(oe,OTI);
      break;
    }

  case M_GET_OBJECT:
  case M_GET_OBJECTANDCLASS:
    {
      int OTI;
      Site *rsite;
      unmarshal_M_GET_OBJECT(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_GET_OBJECT(ANDCLASS) index:%d site:%s",OTI,rsite->stringrep()));
      OwnerEntry *oe=OT->getOwner(OTI);
      oe->receiveCredit(OTI);
      Tertiary *t = oe->getTertiary();
      Assert(isObject(t));
      PD((SPECIAL,"object get %x %x",t,((Object *)t)->getClass()));
      sendObject(rsite,(Object *)t, mt==M_GET_OBJECTANDCLASS);
      OT->ownerCheck(oe,OTI);
      break;
    }
  case M_SEND_OBJECT:
    {
      ObjectFields of;
      Site *sd;
      int si;
      unmarshal_M_SEND_OBJECT(bs,sd,si,&of);
      PD((MSG_RECEIVED,"M_SEND_OBJECT site:%s index:%d",sd->stringrep(),si));
      NetAddress na=NetAddress(sd,si);
      BorrowEntry *be=BT->find(&na);
      Assert(be);
      be->receiveCredit();

      PerdioVar *pv = be->getVar();
      Object *o = pv->getObject();
      if(o==NULL) {
        Assert(0);
        error("M_SEND_OBJECT - don't understand");}
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
      unmarshal_M_SEND_OBJECTANDCLASS(bs,sd,si,&of);
      PD((MSG_RECEIVED,"M_SEND_OBJECTANDCLASS site:%s index:%d",sd->stringrep(),si));

      NetAddress na=NetAddress(sd,si);
      BorrowEntry *be=BT->find(&na);
      Assert(be);
      be->receiveCredit();

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
      unmarshal_M_REDIRECT(bs,sd,si,val);
      PD((MSG_RECEIVED,"M_REDIRECT site:%s index:%d val%s",sd->stringrep(),si,toC(val)));

      NetAddress na=NetAddress(sd,si);
      BorrowEntry *be=BT->find(&na);

      if (!be) { // if not found, then forget the redirect message
        PD((WEIRD,"REDIRECT: no borrow entry found"));
        sendCreditBack(na.site,na.index,1); // ATTENTION
        break;
      }
      be->receiveCredit();

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
      unmarshal_M_SURRENDER(bs,OTI,rsite,v);
      PD((MSG_RECEIVED,"M_SURRENDER index:%d site:%s val%s",OTI,rsite->stringrep(),toC(v)));
      OwnerEntry *oe = ownerTable->getOwner(OTI);
      oe->receiveCredit(OTI);

      if (oe->isVar()) {
        PD((PD_VAR,"SURRENDER do it"));
        PerdioVar *pv = oe->getVar();
        // bug fixed: may be bound to a different perdio var
        pv->primBind(oe->getPtr(),v);
        oe->mkRef();
        if (oe->hasFullCredit()) {
          PD((WEIRD,"SURRENDER: full credit"));
        }
        sendRedirect(pv->getProxies(),v,rsite,OTI);
      } else {
        PD((PD_VAR,"SURRENDER discard"));
        PD((WEIRD,"SURRENDER discard"));
        // ignore redirect: NOTE: v is handled by the usual garbage collection
      }
      OT->ownerCheck(oe,OTI);
      break;
    }

  case M_ACKNOWLEDGE:
    {

      Site *sd;
      int si;
      unmarshal_M_ACKNOWLEDGE(bs,sd,si);
      PD((MSG_RECEIVED,"M_ACKNOWLEDGE site:%s index:%d",sd->stringrep(),si));

      NetAddress na=NetAddress(sd,si);
      BorrowEntry *be=BT->find(&na);
      Assert(be);
      be->receiveCredit();

      Assert(be->isVar());
      PerdioVar *pv = be->getVar();
      pv->acknowledge(be->getPtr());
      be->mkRef();

      // pv->dispose();
      BT->maybeFreeBorrowEntry(pv->getIndex());
      break;
    }
  case M_CELL_LOCK_GET:
    {
      int OTI;
      int accessNr;
      Site* rsite;
      unmarshal_M_CELL_LOCK_GET(bs,OTI,accessNr,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_GET index:%d site:%s access:%d",OTI,rsite->stringrep(),accessNr));
      OwnerEntry *oe=ownerTable->getOwner(OTI);
      oe->receiveCredit(OTI);
      cellLockReceiveGet(oe,OTI,rsite,accessNr);
      break;
    }
   case M_CELL_CONTENTS:
    {
      Site *rsite;
      int OTI;
      TaggedRef val;
      unmarshal_M_CELL_CONTENTS(bs,rsite,OTI,val);
      PD((MSG_RECEIVED,"M_CELL_CONTENTS index:%d site:%s val:%s",OTI,rsite->stringrep(),toC(val)));

      if(rsite==mySite){
        OwnerEntry *oe=OT->getOwner(OTI);
        oe->receiveCredit(OTI);
        cellReceiveContentsManager(oe,val,OTI);
        OT->ownerCheck(oe,OTI);
        break;
      }
      NetAddress na=NetAddress(rsite,OTI);
      BorrowEntry *be=borrowTable->find(&na);
      Assert(be!=NULL);
      be->receiveCredit();
      cellReceiveContentsFrame(be,val,rsite,OTI);
      break;
    }
  case M_CELL_READ:
    {
      int OTI;
      Site *fS;
      unmarshal_M_CELL_READ(bs,OTI,fS);
      PD((MSG_RECEIVED,"M_CELL_READ"));
      cellReceiveRead(OT->getOwner(OTI),OTI,fS); // NOTE: holding credit
      break;
    }
  case M_CELL_REMOTEREAD:
    {
      int BTI;
      Site *fS,*mS;
      unmarshal_M_CELL_REMOTEREAD(bs,mS,BTI,fS);
      NetAddress na=NetAddress(mS,BTI);
      BorrowEntry *be=BT->find(&na);
      Assert(be!=NULL);
      PD((MSG_RECEIVED,"M_CELL_REMOTEREAD"));
      Assert(be->getTertiary()->getType()==Co_Cell);
      Assert(be->getTertiary()->getTertType()==Te_Frame);
      cellReceiveRemoteRead((CellFrame*)be->getTertiary(),mS,BTI,fS); // NOTE: holding credit
      break;
    }
  case M_CELL_READANS:
    {
      int index;
      Site*mS;
      TaggedRef val;
      unmarshal_M_CELL_READANS(bs,mS,index,val);
      if(mS!=mySite){
        NetAddress na=NetAddress(mS,index);
        BorrowEntry *be=BT->find(&na);
        Assert(be!=NULL);
        be->receiveCredit();
        PD((MSG_RECEIVED,"M_CELL_READANS"));
        Assert(be->getTertiary()->getType()==Co_Cell);
        Assert(be->getTertiary()->getTertType()==Te_Frame);
        cellReceiveReadAns((CellFrame*)be->getTertiary(),val);
        break;}
      OwnerEntry *oe=OT->getOwner(index);
      oe->receiveCredit(index);
      cellReceiveReadAns((CellManager*)oe->getTertiary(),val);
      OT->ownerCheck(oe,index);
      break;
   }
  case M_CELL_LOCK_FORWARD:
    {
      Site *site,*rsite;
      int OTI;
      unmarshal_M_CELL_LOCK_FORWARD(bs,site,OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_FORWARD index:%d site:%s rsite:%s",OTI,site->stringrep(),rsite->stringrep()));

      NetAddress na=NetAddress(site,OTI);
      BorrowEntry *be=borrowTable->find(&na);
      Assert(be!=NULL);
      be->receiveCredit();
      cellLockReceiveForward(be,rsite,site,OTI);
      break;
    }
  case M_CELL_LOCK_DUMP:
    {
      int OTI;
      Site* rsite;
      unmarshal_M_CELL_LOCK_DUMP(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_DUMP index:%d site:%s",OTI,rsite->stringrep()));
      OwnerEntry *oe=ownerTable->getOwner(OTI);
      oe->receiveCredit(OTI);
      cellLockReceiveDump(oe->getTertiary(),rsite);
      OT->ownerCheck(oe,OTI);
      break;
    }
  case M_CELL_CANTPUT:
    {
      Site *rsite;
      int OTI;
      TaggedRef val;
      unmarshal_M_CELL_CANTPUT(bs,OTI,rsite,val);
      PD((MSG_RECEIVED,"M_CELL_CANTPUT index:%d site:%s val:%s",OTI,rsite->stringrep(),toC(val)));
      OwnerEntry *oe=OT->getOwner(OTI);
      oe->receiveCredit(OTI);
      cellReceiveCantPut((CellManager*)(oe->getTertiary()),val,OTI,this,rsite);
      OT->ownerCheck(oe,OTI);
      break;
    }
  case M_LOCK_TOKEN:
    {
      Site *rsite;
      int OTI;
      unmarshal_M_LOCK_TOKEN(bs,rsite,OTI);
      PD((MSG_RECEIVED,"M_LOCK_TOKEN index:%d site:%s",OTI,rsite->stringrep()));
      mainLockReceiveLock(rsite,OTI); // NOTE: holding credit
      break;
    }
  case M_CHAIN_ACK:
    {
      int OTI, accessNr;
      Site* rsite;
      unmarshal_M_CHAIN_ACK(bs,OTI,rsite, accessNr);
      PD((MSG_RECEIVED,"M_CHAIN_ACK index:%d site:%s ack:%d",OTI,rsite->stringrep(),accessNr));

      OwnerEntry *oe=ownerTable->getOwner(OTI);
      oe->receiveCredit(OTI);
      chainReceiveAck(oe,rsite,accessNr);
      break;
    }
  case M_LOCK_CANTPUT:
    {
      Site *rsite;
      int OTI;
      TaggedRef val;
      unmarshal_M_LOCK_CANTPUT(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_CANTPUT index:%d site:%s val:%s",OTI,rsite->stringrep()));
      OwnerEntry *oe=OT->getOwner(OTI);
      oe->receiveCredit(OTI);
      lockReceiveCantPut((LockManager*)(oe->getTertiary()),OTI,this,rsite);
      break;
    }
  case M_CHAIN_QUESTION:
   {
      Site *site,*rsite;
      int OTI,ctr;
      unmarshal_M_CHAIN_QUESTION(bs,OTI,site,ctr);
      PD((MSG_RECEIVED,"M_CHAIN_QUESTION index:%d site:%s ctr:%d",OTI,site->stringrep(),ctr));
      NetAddress na=NetAddress(site,OTI);
      BorrowEntry *be=borrowTable->find(&na);
      if(be==NULL){
        sendCreditBack(na.site,na.index,1);}
      else{
        be->receiveCredit();}
      chainReceiveQuestion(be,site,OTI,ctr);
      break;
   }
  case M_CHAIN_ANSWER:
    {
      Site *rsite;
      int OTI;
      int accessNr;
      int ans;
      unmarshal_M_CHAIN_ANSWER(bs,OTI,rsite,accessNr,ans);
      PD((MSG_RECEIVED,"M_CHAIN_ANSWER index:%d site:%s val:%d accNr:%d",
          OTI,rsite->stringrep(),ans,accessNr));
      OwnerEntry *oe=OT->getOwner(OTI);
      oe->receiveCredit(OTI);
      chainReceiveAnswer(oe,rsite,accessNr,ans);
      break;
    }

  case M_TELL_ERROR:
    {
      Site *site;
      int OTI;
      int ec;
      unmarshal_M_TELL_ERROR(bs,site,OTI,ec);
      PD((MSG_RECEIVED,"M_TELL_ERROR index:%d site:%s ec:%d",OTI,site->stringrep(),ec));

      NetAddress na=NetAddress(site,OTI);
      BorrowEntry *be=borrowTable->find(&na);
      if(be==NULL){
        sendCreditBack(na.site,na.index,1);
        return;
      }
      be->receiveCredit();
      receiveTellError(be->getTertiary(),site,OTI,ec);
      break;
    }

  case M_ASK_ERROR:
    {
      int OTI;
      int ec;
      unmarshal_M_ASK_ERROR(bs,OTI,ec);
      PD((MSG_RECEIVED,"M_ASK_ERROR index:%d ec:%d",OTI,ec));

      NOT_IMPLEMENTED;
    }
  default:
    error("siteReceive: unknown message %d\n",mt);
    break;
  }
}


/**********************************************************************/
/*   SECTION 23:: remote send protocol                                */
/**********************************************************************/

/* engine-interface */
OZ_Return remoteSend(Tertiary *p, char *biName, TaggedRef msg) {
  BorrowEntry *b= borrowTable->getBorrow(p->getIndex());
  NetAddress *na = b->getNetAddress();
  Site* site = na->site;
  int index = na->index;

  MsgBuffer *bs = msgBufferManager->getMsgBuffer(site);
  b->getOneMsgCredit();
  marshal_M_REMOTE_SEND(bs,index,biName,msg);
  SendTo(site,bs,M_REMOTE_SEND,site,index);
  return PROCEED;}

/**********************************************************************/
/*   SECTION 24:: Port protocol                                       */
/**********************************************************************/

void portSend(Tertiary *p, TaggedRef msg) {
  int pi = p->getIndex();
  BorrowEntry* b=BT->getBorrow(pi);
  NetAddress *na = b->getNetAddress();
  Site* site = na->site;
  int index = na->index;

  MsgBuffer *bs=msgBufferManager->getMsgBuffer(site);
  b->getOneMsgCredit();
  marshal_M_PORT_SEND(bs,index,msg);
  SendTo(site,bs,M_PORT_SEND,site,index);}

/**********************************************************************/
/*   SECTION 25:: Variable protocol                                       */
/**********************************************************************/

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
  int ret = lSD->compareSites(rSD);
  if (ret != 0) return ret;
  return lOTI<rOTI ? -1 : 1;
}

void sendRegister(BorrowEntry *be) {
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_REGISTER(bs,na->index,mySite);
  SendTo(na->site,bs,M_REGISTER,na->site,na->index);}

void sendSurrender(BorrowEntry *be,OZ_Term val){
  be->getOneMsgCredit();
  NetAddress *na = be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  marshal_M_SURRENDER(bs,na->index,mySite,val);
  SendTo(na->site,bs,M_SURRENDER,na->site,na->index);}

void sendRedirect(Site* sd,int OTI,TaggedRef val){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(sd);
  OT->getOwner(OTI)->getOneCreditOwner();
  marshal_M_REDIRECT(bs,mySite,OTI,val);
  SendTo(sd,bs,M_REDIRECT,mySite,OTI);}

void sendAcknowledge(Site* sd,int OTI){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(sd);
  OT->getOwner(OTI)->getOneCreditOwner();
  marshal_M_ACKNOWLEDGE(bs,mySite,OTI);
  SendTo(sd,bs,M_ACKNOWLEDGE,mySite,OTI);}

void PerdioVar::acknowledge(OZ_Term *p)
{
  PD((PD_VAR,"acknowledge"));
  OZ_Term val=u.bindings->val;
  primBind(p,val);
  if (u.bindings->thread->isDeadThread()) {
    PD((WEIRD,"dead thread acknowledge %x",u.bindings->thread));
  } else {
    PD((THREAD_D,"start thread ackowledge %x",u.bindings->thread));
    oz_resumeFromNet(u.bindings->thread);
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
      oz_resumeFromNet(u.bindings->thread);
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
  PD((PD_VAR,"bindPerdioVar by thread: %x",am.currentThread()));
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
/*   SECTION 26:: Object protocol                                     */
/**********************************************************************/

void sendObject(Site* sd, Object *o, Bool sendClass){
  int OTI = o->getIndex();
  MsgBuffer *bs= msgBufferManager->getMsgBuffer(sd);
  OT->getOwner(OTI)->getOneCreditOwner();
  int ret;
  if(sendClass){
    marshal_M_SEND_OBJECTANDCLASS(bs,mySite,OTI,o);
    SendTo(sd,bs,M_SEND_OBJECTANDCLASS,mySite,OTI);}
  else{
    marshal_M_SEND_OBJECT(bs,mySite,OTI,o);
    SendTo(sd,bs,M_SEND_OBJECT,mySite,OTI);}}

/**********************************************************************/
/*   SECTION 27:: Credit protocol                                     */
/**********************************************************************/

void sendCreditBack(Site* sd,int OTI,Credit c)
{ int ret;
  if(creditSite==NULL){
    sendPrimaryCredit(sd,OTI,c);
    return;}
  sendSecondaryCredit(creditSite,sd,OTI,c);
  return;}

void sendPrimaryCredit(Site *sd,int OTI,Credit c){
  MsgBuffer *bs= msgBufferManager->getMsgBuffer(sd);
  marshal_M_OWNER_CREDIT(bs,OTI,c);
  SendTo(sd,bs,M_OWNER_CREDIT,sd,OTI);}

void sendSecondaryCredit(Site *cs,Site *sd,int OTI,Credit c){
  MsgBuffer *bs= msgBufferManager->getMsgBuffer(cs);
  marshal_M_OWNER_SEC_CREDIT(bs,sd,OTI,c);
  SendTo(sd,bs,M_OWNER_SEC_CREDIT,sd,OTI);}

/**********************************************************************/
/*   SECTION 28:: Cell lock protocol common                           */
/**********************************************************************/

void cellLockReceiveGet(OwnerEntry* oe,int mI,Site* toS,int accessNr){
  Tertiary* t=oe->getTertiary();
  if(t->getType()==Co_Cell){
    cellReceiveGet(oe,t,mI,toS,accessNr);}
  lockReceiveGet(oe,t,mI,toS,accessNr);}

void cellLockReceiveForward(BorrowEntry *be,Site* toS,Site* mS,int mI){
  Tertiary* t=be->getTertiary();
  if(t->getType()==Co_Cell){
    cellReceiveForward(be,t,toS,mS,mI);}
  lockReceiveForward(be,t,toS,mS,mI);}

void cellLockSendGet(BorrowEntry *be, int accessNr){
  NetAddress *na=be->getNetAddress();
  Site *toS=na->site;
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  be->getOneMsgCredit();
  PD((CELL,"M_CELL_LOCK_GET indx:%d site:%s accessNr:%d",na->index,toS->stringrep(),accessNr));
  marshal_M_CELL_LOCK_GET(bs,na->index,accessNr,mySite);
  SendTo(toS,bs,M_CELL_LOCK_GET,toS,na->index);}

void cellLockSendForward(Site *toS,Site *fS,int mI){ // holding one credit
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_LOCK_FORWARD(bs,mySite,mI,fS);
  SendTo(toS,bs,M_CELL_LOCK_FORWARD,mySite,mI);}

void cellLockReceiveDump(Tertiary *t,Site* fromS){
  if(t->getType()==Co_Cell){cellReceiveDump((CellManager*) t,fromS);}
  else{lockReceiveDump((LockManager*) t,fromS);}}

void cellLockSendDump(BorrowEntry *be){
  NetAddress *na=be->getNetAddress();
  Site *toS=na->site;
  if(SEND_SHORT(toS)){return;}
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  be->getOneMsgCredit();
  marshal_M_CELL_LOCK_DUMP(bs,na->index,mySite);
  SendTo(toS,bs,M_CELL_LOCK_DUMP,toS,na->index);}

/**********************************************************************/
/*   SECTION 28:: Cell protocol - receive                            */
/**********************************************************************/

void  cellReceiveGet(OwnerEntry* oe,Tertiary* t,int mI,Site* toS,int accessNr){
  CellManager* cm=(CellManager*)(oe->getTertiary());
  if(t->getEntityCond() & PERM_BLOCKED){return;}
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);

  Site* current=cm->getCurrent();
  PD((CELL,"CellMgr Received get from %s num: %d",toS->stringrep(),
      accessNr));
  cm->setCurrent(toS, accessNr);
  if(current==mySite){                             // shortcut
    PD((CELL,"CELL - shortcut in cellReceiveGet"));
    CellFrame *cf=(CellFrame*)cm;
    if(cf->getState()==Cell_Requested){
      cf->setState(Cell_Requested | Cell_Next);
      cf->setNext(toS);
      return;}
    Assert(cf->getState()==Cell_Valid);
    oe->getOneCreditOwner();
    cellSendContents(cf->getContents(),toS,mySite,mI);
    cf->setState(Cell_Invalid);
    return;}
  oe->getOneCreditOwner();
  cellLockSendForward(current,toS,mI);
}

void cellReceiveContentsManager(OwnerEntry *oe,TaggedRef val,int mI){
  CellManager *cm=(CellManager*)oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);
  CellFrame *cf=(CellFrame*)cm;
  chainReceiveAck(oe,mySite,OWNER_ACCESS_NR);
  short  state=cf->getState();
  Assert(state & Cell_Requested);
  pendThreadResumeAll(cf->getPending());
  cf->setPending(NULL);
  TaggedRef head=cf->getHead();
  SiteUnify(head,val);
  if(state & Cell_Next){
    Site *next=cf->getNext();
    oe->getOneCreditOwner();
    cellSendContents(cf->getContents(),next,mySite,mI);
    cf->setState(Cell_Invalid);
    return;}
  cf->setState(Cell_Valid);
  return;
}

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
  TaggedRef tr=oz_newVariable();
  cellDoExchange((Tertiary *)cf,tr,tr,DummyThread);
  return;
}

void cellReceiveForward(BorrowEntry *be,Tertiary *t,Site *toS,Site* mS,int mI){
  CellFrame *cf= (CellFrame*) t;
  Assert(cf->getTertType()==Te_Frame);
  Assert(cf->getType()==Co_Cell);
  cf->setState(cf->getState() & (~Cell_Dump_Asked));
  if(cf->getState() & Cell_Requested){
    Assert(!(cf->getState() & Cell_Next));
    cf->setState(Cell_Requested | Cell_Next);
    cf->setNext(toS);
    return;}
  Assert(cf->getState() & Cell_Valid);
  TaggedRef tr=cf->getContents();
  be->getOneMsgCredit();
  cellSendContents(tr,toS,mS,mI);
  cf->setState(Cell_Invalid);
  return;}

void cellReceiveContentsFrame(BorrowEntry *be,TaggedRef val,Site *mS,int mI){
  CellFrame*cf=(CellFrame*)(be->getTertiary());
  Assert(cf->getType()==Co_Cell);
  Assert(cf->getTertType()==Te_Frame);
  short  state=cf->getState();
  Assert(state & Cell_Requested);

  chainSendAck(mS,mI,cf->readAccessNr());
  pendThreadResumeAll(cf->getPending());
  cf->setPending(NULL);
  SiteUnify(cf->getHead(),val);
  if(state & Cell_Next){
    Site *toSite=cf->getNext();
    be->getOneMsgCredit();
    cellSendContents(cf->getContents(),toSite,mS,mI);
    cf->setState(Cell_Invalid);
    return;}
  cf->setState(Cell_Valid);
  return;}

void cellReceiveRemoteRead(CellFrame* cf,Site* mS,int OTI,Site* fS){ // NOTE:holding credit
  Assert(cf->getTertType()==Te_Frame);
  Assert(cf->getType()==Co_Cell);
  Assert(cf->getState() & (Cell_Valid | Cell_Requested));
  cellSendReadAns(fS,mS,OTI,cf->getContents());
  return;}

void cellReceiveRead(OwnerEntry *oe,int OTI,Site* fS){ // NOTE: holding credit
  CellManager *cm=(CellManager*)oe->getTertiary();
  if(cm->getCurrent()==mySite){
    cellSendReadAns(fS,mySite,OTI,((CellFrame*) cm)->getContents());
    return;}
  cellSendRemoteRead(cm->getCurrent(),mySite,OTI,fS);}

void cellReceiveReadAns(Tertiary* t,TaggedRef val){ // hol
  Assert((t->getTertType()==Te_Manager)|| (t->getTertType()==Te_Frame));
  NOT_IMPLEMENTED;
}

/**********************************************************************/
/*   SECTION 29:: Cell protocol - send                            */
/**********************************************************************/

void cellSendReadAns(Site* toS,Site* mS,int mI,TaggedRef val){ // NOTE: holding credit
  NOT_IMPLEMENTED;}

void cellSendRemoteRead(Site* toS,Site* mS,int mI,Site* fS){ // NOTE:holding credit
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_REMOTEREAD(bs,mS,mI,fS);
  SendTo(toS,bs,M_CELL_REMOTEREAD,mS,mI);}

void cellSendContents(TaggedRef tr,Site* toS,Site *mS,int mI){ // holding CREDIT
  PD((CELL,"Cell Send Contents to:%s",toS->stringrep()));
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_CONTENTS(bs,mS,mI,tr);
  PD((SPECIAL,"CellContents %s",toC(tr)));
  SendTo(toS,bs,M_CELL_CONTENTS,mS,mI);}


void cellSendRead(BorrowEntry *be){
  NetAddress *na=be->getNetAddress();
  Site *toS=na->site;
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  be->getOneMsgCredit();
  marshal_M_CELL_READ(bs,na->index,mySite);
  SendTo(toS,bs,M_CELL_READ,na->site,na->index);}

/**********************************************************************/
/*   SECTION 30:: Cell protocol - basics                              */
/**********************************************************************/

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
  PD((SPECIAL,"exchange old:%d new:%s",toC(old),toC(nw)));
  maybeConvertCellProxyToFrame(c);
  Assert(th!=MoveThread);
  TertType tt=c->getTertType();
  CellFrame *cf=(CellFrame *)c;
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
      cellLockSendForward(current,mySite,myI);
      return;}}
  else{
    Assert(tt=Te_Frame);
    PD((CELL,"CELL: exchange on frame %s-%d",BTOS(cf->getIndex())->stringrep(),BTOI(cf->getIndex())));}

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
  cellLockSendGet(BT->getBorrow(mI),cf->incAccessNr());
  return;
}

void cellDoAccess(Tertiary *c,TaggedRef val){
  maybeConvertCellProxyToFrame(c);
  TertType tt=c->getTertType();
  CellFrame *cf=(CellFrame *)c;
  if(cf->getState()==Cell_Valid){
    pushUnify(am.currentThread(),val,cf->getContents());
    return;}
  if(cf->getState()==Cell_Requested){
    cf->addPendBinding(am.currentThread(),val);
    oz_suspendOnNet(am.currentThread());
    return;}
  cf->addPendBinding(am.currentThread(),val);
  if(tt==Te_Manager){
    CellManager *cm=(CellManager*)c;
    int mI=cm->getIndex();
    Assert(cm->getCurrent()!=mySite);
    OwnerEntry *oe=OT->getOwner(mI);
    oe->getOneCreditOwner();
    PD((CELL,"access on INVALID manager %d",cf->getIndex()));
    cellSendRemoteRead(cm->getCurrent(),mySite,mI,mySite);
    return;}
  cellSendRead(BT->getBorrow(cf->getIndex()));
  return;
}

/**********************************************************************/
/*   SECTION 31:: chain routines                                      */
/**********************************************************************/

Bool Chain::siteNrExists(Site *s, int nr){
  ChainElem *e = first;
  while(e!=NULL && (e->getNum()!= nr || e->getSite()!=s)){
    e = e->getNext();}
  return e!=NULL?true:false;}

Bool Chain::siteListCheck(){
  /* Just for Debugging the system */
  ChainElem *e = first;
  while(e!=NULL){
    PD((CELL_MGR,"site:%s QueueNr:%d Probe:%d",e->getSite()->stringrep(),e->getNum(),e->isProbing()));
    e = e->getNext();}
  return true;}

void Chain::siteNrRemove(Site* s, int nr){
  Assert(first!=NULL && last!=NULL);
  Assert(siteNrExists(s,nr));
  ChainElem *e = first, *tmp;
  while(e!=NULL && (e->getNum()!=nr || e->getSite()!=s)){
    if(e->isProbing())
      e->probeStop();
    tmp = e; e = e->getNext();
    freeChainElem(tmp);}
  first = e;
  Assert(e!=NULL && e->getSite() == s && e->getNum()==nr);}

Site* Chain::removeSiteNext(Site* s, Site* bad){
  ChainElem *e;

  Assert(first->getSite()==s);
  Assert(first->next->getSite()==bad);
  e=first->next;
  first->next=e->next;
  e->probeStop();
  freeChainElem(e);
  if(first->next!=NULL){installProbes();}
  return first->next->getSite();}

void Chain::installProbes(){
  Assert(first!=NULL);
  ChainElem *e = first->getNext();
  if(e!=NULL && (!e->isProbing()) && e->getSite()!=mySite)
    e->probeStarted();
  if( (!first->isProbing()) && first->getSite()!=mySite)
    first->probeStarted();}

/**********************************************************************/
/*   SECTION 32:: chain protocol                                      */
/**********************************************************************/

enum ChainAnswer{
  PAST_ME,
    AT_ME,
    BEFORE_ME};

Chain* getChainFromTertiary(Tertiary *t){
  Assert(t->getTertType()==Te_Manager);
  if(t->getType()==Co_Cell){
    return ((CellManager *)t)->getChain();}
  Assert(t->getType()==Co_Lock);
  return ((LockManager *)t)->getChain();}

void chainSendAck(Site* toS, int mI, int accessNr){
  if(SEND_SHORT(toS)) {return;}
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  PD((CELL,"M_CHAIN_ACK indx:%d site:%s accessNr:%d",mI,toS->stringrep(),accessNr));
  marshal_M_CHAIN_ACK(bs,mI,mySite, accessNr);
  SendTo(toS,bs,M_CHAIN_ACK,toS,mI);}

void chainReceiveAck(OwnerEntry* oe,Site* rsite,int nr){
  Tertiary *t=oe->getTertiary();
  Chain* chain=tertiaryGetChain(t);
  if(chain->siteNrExists(rsite,nr)){
    chain->siteNrRemove(rsite,nr);
    return;}}

void chainReceiveQuestion(BorrowEntry *be,Site* site,int OTI,int accessNr){
  if(be==NULL){
    chainSendAnswer(site,OTI,accessNr,PAST_ME);}
  Tertiary *t = be->getTertiary();
  if(t->getType()==Co_Cell){
    if(t->getTertType()==Te_Frame){
      CellFrame *cf= (CellFrame*)t;
      if(cf->readAccessNr()>accessNr){
        chainSendAnswer(site,OTI,accessNr,PAST_ME);
        return;}
      Assert(cf->readAccessNr()==accessNr);
      if(cf->getState()== Cell_Valid){
        chainSendAnswer(site,OTI,accessNr,AT_ME);
        return;}
      if(cf->getState()==Cell_Invalid){
        chainSendAnswer(site,OTI,accessNr,PAST_ME);
        return;}
      chainSendAnswer(site,OTI,accessNr,BEFORE_ME);
      return;}
    chainSendAnswer(site,OTI,accessNr,PAST_ME);
    return;}
  Assert(t->getType()==Co_Lock);
  if(t->getTertType()==Te_Frame){
    LockFrame *cf= (LockFrame*)t;
    if(cf->readAccessNr()>accessNr){
      chainSendAnswer(site,OTI,accessNr,PAST_ME);
      return;}
    Assert(cf->readAccessNr()==accessNr);
    if(cf->getState()== Lock_Valid){
      chainSendAnswer(site,OTI,accessNr,AT_ME);
      return;}
    if(cf->getState()==Lock_Invalid){
      chainSendAnswer(site,OTI,accessNr,PAST_ME);
      return;}
    chainSendAnswer(site,OTI,accessNr,BEFORE_ME);
    return;}
  chainSendAnswer(site,OTI,accessNr,PAST_ME);
  return;}

void chainReceiveAnswer(OwnerEntry* oe,Site* site,int accessNr,int ans){
  Tertiary* t=oe->getTertiary();
  Chain* chain=tertiaryGetChain(t);
  if(!chain->siteNrExists(site,accessNr)) {return;}
  if(ans==PAST_ME){
    if(chain->siteIsFirst(site,accessNr)){
      chain->siteNrRemove(site,accessNr);
      int tn;
      Site *tS=chain->getFirstSite();
      getChainFromTertiary(t)->managerSeesSiteCrash(t,tS);
      return;}
    Assert(chain->getFirstSite()->siteStatus()==PERM_SITE);
    chain->removeFirstSite();
    chain->installProbes();
    return;}
  if(ans==AT_ME){
    if(chain->siteIsFirst(site,accessNr)){
      return;}
    Assert(chain->getFirstSite()->siteStatus()==PERM_SITE);
    chain->removeFirstSite();
    chain->installProbes();
    return;}
  Assert(ans==BEFORE_ME);
  if(t->getType()==Co_Cell){
    tokenLost(chain,t);
    NOT_IMPLEMENTED;
    t->entityProblem((EntityCond) PERM_BLOCKED|PERM_SOME|PERM_ALL);
    chain->informHandle(t,(EntityCond) PERM_BLOCKED|PERM_SOME|PERM_ALL);
    return;}
  Assert(t->getType()==Co_Lock);

                 // ATTENTION not optimal - lock can be recovered
  t->entityProblem((EntityCond) PERM_BLOCKED|PERM_SOME|PERM_ALL);
  chain->informHandle(t,(EntityCond) PERM_BLOCKED|PERM_SOME|PERM_ALL);
  tokenRecovery(chain,t);
  return;}

void chainSendQuestion(Site* toS,int mI,int accessNr){
  OT->getOwner(mI)->getOneCreditOwner();
  PD((ERROR_DET,"chainSendQuestion  %s nr:%d",toS->stringrep(),accessNr));
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CHAIN_QUESTION(bs,mI,mySite,accessNr);
  SendTo(toS,bs,M_CHAIN_QUESTION,toS,mI);}

void chainSendAnswer(Site* toS, int mI, int accessNr, int ans){
  /* OT->getOwner(mI)->getOneCreditOwner(); */
  BT->getBorrow(mI)->getOneMsgCredit();
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CHAIN_ANSWER(bs,mI,mySite, accessNr,ans);
  SendTo(toS,bs,M_CHAIN_ANSWER,toS,mI);}

/**********************************************************************/
/*   SECTION 33:: Lock protocol - receive                             */
/**********************************************************************/

void mainLockReceiveLock(Site *rsite,int OTI){
  if(rsite==mySite){
    OwnerEntry *oe=OT->getOwner(OTI);
    oe->receiveCredit(OTI);
    Tertiary *t=oe->getTertiary();
    chainReceiveAck(oe,rsite,OWNER_ACCESS_NR);
    lockReceiveLock((LockFrame*)t);
    OT->ownerCheck(oe,OTI);
    return;}

  NetAddress na=NetAddress(rsite,OTI);
  BorrowEntry *be=borrowTable->find(&na);
  Assert(be!=NULL);
  be->receiveCredit();
  chainSendAck(rsite,OTI,((LockFrame*)be->getTertiary())->readAccessNr());
  lockReceiveLock((LockFrame*)be->getTertiary());}

void lockReceiveGet(OwnerEntry* oe,Tertiary *t,int mI,Site* toS,int accessNr){
  LockManager* lm=(LockManager*)t;
  Assert(lm->getType()==Co_Lock);
  Assert(lm->getTertType()==Te_Manager);

  if(t->getEntityCond() & PERM_BLOCKED){// ATTENTION;
    return;}
  Site* current=lm->getCurrent();
  PD((LOCK,"LockMgr Received get from %s num: %d",toS->stringrep(),accessNr));
  lm->setCurrent(toS,accessNr);
  if(current==mySite){                             // shortcut
    PD((LOCK," shortcut in lockReceiveGet"));
    LockFrame *lf=(LockFrame*)lm;
    if(lf->getState()==Lock_Requested){
      lf->setState(Lock_Requested | Lock_Next);
      lf->setNext(toS);
      return;}
    Assert(lf->getState()==Lock_Valid);
    if(lf->getLocker()==NULL){
      oe->getOneCreditOwner();
      lockSendLock(mySite,mI,toS);
      lf->setState(Lock_Invalid);
      return;}
    lf->setState(Lock_Valid | Lock_Next);
    lf->setNext(toS);
    return;}
  oe->getOneCreditOwner();
  cellLockSendForward(current,toS,mI);
  return;}

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
  lm->lockComplex(DummyThread);
  return;}

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

void lockReceiveForward(BorrowEntry *be,Tertiary *t,Site *toS,Site* mS,int mI){
  LockFrame *lf= (LockFrame*) t;
  Assert(lf->getTertType()==Te_Frame);
  Assert(lf->getType()==Co_Lock);
  lf->setState(lf->getState() & (~Lock_Dump_Asked));
  if(lf->getState() & Lock_Requested){
    Assert(!(lf->getState() & Lock_Next));
    lf->setState(Lock_Requested | Lock_Next);
    lf->setNext(toS);
    return;}
  Assert(lf->getState()==Lock_Valid);
  if((lf->getPending()==NULL) && lf->getLocker()==NULL){
    be->getOneMsgCredit();
    lockSendLock(mS,mI,toS);
    lf->setState(Lock_Invalid);
    return;}
  lf->setNext(toS);
  lf->setState(Lock_Valid | Lock_Next);
  pendThreadAddToEnd(lf->getPendBase(),MoveThread);
  return;}

/**********************************************************************/
/*   SECTION 34:: Lock protocol - send                                */
/**********************************************************************/

void lockSendLock(Site *mS,int mI,Site* toS){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_LOCK_TOKEN(bs,mS,mI);
  SendTo(toS,bs,M_LOCK_TOKEN,mS,mI);}

void lockSendLockBorrow(BorrowEntry *be,Site* toS){
  NetAddress *na=be->getNetAddress();
  be->getOneMsgCredit();
  lockSendLock(na->site,na->index,toS);}

/**********************************************************************/
/*   SECTION 35:: Lock protocol - basics                             */
/**********************************************************************/

Bool LockFrame:: isPending(Thread *th){
    PendThread *pt = getPending();
    while(pt!=NULL){
      if(pt->thread == th)
        return true;
      pt = pt->next;}
    return false;}

void LockProxy::lock(Thread *t){
  PD((LOCK,"convertToFrame %s-%d",BTOS(getIndex())->stringrep(),BTOI(getIndex())));
  convertLockProxyToFrame(this);
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
      return;}
    PD((LOCK,"lock VALID lock held by other"));
    if(state & Lock_Next){
      if(lf->getPending()==NULL){
        pendThreadAddToEnd(lf->getPendBase(),MoveThread);
        PD((LOCK,"lock VALID due to be shipped out"));}}
    pendThreadAddToEnd(lf->getPendBase(),t);
    return;}

  pendThreadAddToEnd(lf->getPendBase(),t);
  if(state & Lock_Requested){
    PD((LOCK,"lock REQUESTED "));
    return;}
  Assert(lf->getTertType()==Te_Frame);
  Assert(state == Lock_Invalid);
  lf->setState(Lock_Requested);
  if(lf->getEntityCond() & PERM_BLOCKED){return;}
  cellLockSendGet(BT->getBorrow(lf->getIndex()),lf->incAccessNr());
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
    if(getEntityCond() & PERM_BLOCKED){ return;}
    OwnerEntry *oe=OT->getOwner(getIndex());
    oe->getOneCreditOwner();
    cellLockSendForward(current,mySite,getIndex());
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

void LockFrame::unlockComplexB(Thread *t){
  PendThread **pt=getPendBase();
  while((*pt)->thread!=t) {
    pt=&((*pt)->next);}
  *pt=(*pt)->next;}

void LockManager::unlockComplexB(Thread *t){
  ((LockFrame *)this)->unlockComplexB(t);}

void LockFrame::unlockComplex(){
  Assert(getState() & Lock_Valid);
  if(getState() & Lock_Next){
    Assert(getState()==(Lock_Next | Lock_Valid));
    if(getPending()==NULL){
      BorrowEntry *be=BT->getBorrow(getIndex());
      setLocker(NULL);
      Site *toS=getNext();
      lockSendLockBorrow(be,toS);
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
      if(getPending()!=NULL){
        setState(Lock_Requested);
        cellLockSendGet(be,incAccessNr());
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
      lf->setState(Lock_Requested);
      Site *current=getCurrent();
      setOwnCurrent();
      oe->getOneCreditOwner();
      cellLockSendForward(current,mySite,getIndex());
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
/*   SECTION 36:: error msgs                                         */
/**********************************************************************/

void cellManagerIsDown(TaggedRef tr,Site* mS,int mI){
  NetAddress na=NetAddress(mS,mI);
  BorrowEntry *be=BT->find(&na);
  if(be==NULL){ return ;}
  Tertiary* t=be->getTertiary();
  maybeConvertCellProxyToFrame(t);
  CellFrame *cf = (CellFrame*)t;
  if(cf->getState()==Cell_Invalid){
    cf->setState(Cell_Valid);
    cf->setContents(tr);
    return;}
  Assert(cf->getState()==Cell_Requested);
  cellReceiveContentsFrame(be,tr,mS,mI);}

void cellSendCantPut(TaggedRef tr,Site* toS, Site *mS, int mI){
  PD((ERROR_DET,"Proxy cant put to %s site: %s:%d",toS->stringrep(),mS->stringrep(),mI));
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(mS);
  marshal_M_CELL_CANTPUT(bs,mI, toS, tr);
  SendTo(mS,bs,M_CELL_CANTPUT,mS,mI);
  return;}

void cellSendContentsFailure(TaggedRef tr,Site* toS,Site *mS, int mI){
  if(toS==mS) {// ManagerSite is down
    cellManagerIsDown(tr,toS,mI);
    return;}
  if(mS==mySite){// At managerSite
    Tertiary *tmp=getTertiaryFromOTI(mI);
    cellReceiveCantPut((CellManager*)tmp,tr,mI,mS,toS);
    return;}
  cellSendCantPut(tr,toS,mS,mI);
  return;}

void cellSendForwardFailure(Site* toS, int OTI){
  Tertiary *t=getTertiaryFromOTI(OTI);
  getChainFromTertiary(t)->managerSeesSiteCrash(t,toS);} // ATTENTION at the end

void lockManagerIsDown(Site* mS,int mI){
  NetAddress na=NetAddress(mS,mI);
  BorrowEntry *be=BT->find(&na);
  if(be==NULL){ // has been gc'ed
    return;}
  Tertiary* t=be->getTertiary();
  maybeConvertLockProxyToFrame(t);
  Assert(t->getTertType()==Te_Frame);
  LockFrame *cf = (LockFrame*)t;
  if(cf->getState()==Lock_Invalid){
    cf->setState(Lock_Valid);
    return;}
  Assert(cf->getState()==Lock_Requested);
  lockReceiveLock(cf);}

void lockSendCantPut(Site* toS, Site *mS, int mI){
  PD((ERROR_DET,"Proxy cant put - to %s site: %s:%d Nr %d",toS->stringrep(),mS->stringrep(),mI));
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(mS);
  marshal_M_LOCK_CANTPUT(bs,mI, toS);
  SendTo(mS,bs,M_LOCK_CANTPUT,mS,mI);
  return;}

void lockSendTokenFailure(Site* toS,Site *mS, int mI){
  if(toS==mS) {// ManagerSite is down
    lockManagerIsDown(mS,mI);
    return;}
  if(mS==mySite){// At managerSite
    lockReceiveCantPut((LockManager*)getTertiaryFromOTI(mI),mI,mS,toS);
    return;}
  lockSendCantPut(toS,mS,mI);
  return;}

void lockSendForwardFailure(Site* toS, int OTI){
  Tertiary *t=getTertiaryFromOTI(OTI);
  getChainFromTertiary(t)->managerSeesSiteCrash(t,toS);}

void lockReceiveCantPut(LockManager *cm,int mI,Site* rsite, Site* dS){
  Assert(cm->getType()==Co_Lock);
  Assert(cm->getTertType()==Te_Manager);
  PD((ERROR_DET,"Proxy cant Put"));
  Site* nextSite = cm->getChain()->proxySeesSiteCrash((Tertiary*)cm,rsite, dS);
  if(nextSite==NULL){
   LockFrame *cf=(LockFrame*)cm;
   Assert(~(Lock_Requested & cf->getState()) & ~(cf->getState() &  Lock_Next));
   cf->setState(Lock_Valid);
   return;}
  if(nextSite == mySite){
    mainLockReceiveLock(rsite,mI);
    return;}
  lockSendLock(nextSite,mI,mySite);
  return;}

void receiveAskError(Tertiary *t,Site *mS,int mI,EntityCond ec){
  NOT_IMPLEMENTED;}

void receivePermBlocked(Tertiary *t, Site* site, int OTI, int accessNr){
  t->entityProblem((EntityCond) PERM_BLOCKED|PERM_SOME);}

void cellReceiveCantPut(CellManager *cm,TaggedRef val,int mI,Site* rsite, Site* badS){
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);
  PD((ERROR_DET,"Proxy cant Put"));

  Site* nextSite = cm->getChain()->proxySeesSiteCrash((Tertiary*) cm,rsite, badS);

  if(nextSite==NULL){
   CellFrame *cf=(CellFrame*)cm;
   Assert(cf->getState() & Cell_Invalid);
   cf->setHead(val);
   cf->setState(Cell_Valid);
   return;}
  if(nextSite == mySite){
    cellReceiveContentsManager(getOwnerEntryFromOTI(mI),val,mI);
    return;}
  cellSendContents(val,nextSite,mySite,mI);
  return;
}

/**********************************************************************/
/*   SECTION 37:: handlers/watchers                                   */
/**********************************************************************/

Bool Tertiary::handlerExists(Thread *t){
  if(info==NULL) return NO;
  Watcher *w=info->watchers;
  while(w!=NULL){
    if(w->getThread()==t) {return OK;}
    w=w->next;}
  return NO;}

void Tertiary::insertWatcher(Watcher *w){
  if(info==NULL){
    info=new EntityInfo(w);
    return;}
  w->next=info->watchers;
  info->watchers=w;}

Bool Tertiary::installHandler(EntityCond wc,TaggedRef proc,Thread* th){
  if(handlerExists(th)){return FALSE;} // duplicate
  PD((NET_HANDLER,"Handler installed on tertiary %x",this));
  Watcher *w=new Watcher(proc,th,wc);
  insertWatcher(w);
  Watcher *old=getWatchers();
  if(wc & TEMP_BLOCKED){
    while(old!=NULL){
      if(old->isHandler() && old->watchcond & TEMP_BLOCKED){break;}
      old=old->getNext();}
    if(old==NULL){
      NOT_IMPLEMENTED;}}
  if(getEntityCond()!=PERM_BLOCKED && (getTertType()!=Te_Manager)) { // ATTENTION
    getSiteFromTertiary(this)->installProbe(PROBE_TYPE_ALL,0);}
  return TRUE;}

void Tertiary::installWatcher(EntityCond wc,TaggedRef proc){
  NOT_IMPLEMENTED;}

void Watcher::invokeHandler(EntityCond ec,Tertiary* entity){
  Assert(isHandler());
  thread->pushCall(BI_restop,0,0);
  RefsArray args = allocateRefsArray(2, NO);
  args[0]= makeTaggedTert(entity);
  args[1]= (ec==PERM_BLOCKED)?AtomPermBlocked:AtomTempBlocked;
  thread->pushCall(proc,args,2);
  oz_resumeFromNet(thread);}

void Watcher::invokeWatcher(EntityCond ec,Tertiary* entity){
  NOT_IMPLEMENTED;}

Bool threadIsPending(Tertiary* t,Thread *th){
  if(t->getTertType()==Te_Proxy) {return NO;}
  if(t->getType()==Co_Cell){
    return threadIsPending(((CellFrame *)t)->getPending(),th);}
  Assert(t->getType()==Co_Lock);
  return threadIsPending(((LockFrame *)t)->getPending(),th);}

/**********************************************************************/
/*   SECTION 38:: error                                               */
/**********************************************************************/

void Tertiary::entityProblem(EntityCond ec){
  setEntityCond(ec);
  Watcher** base=getWatcherBase();
  if(base==NULL) {return;}
  Watcher* w=*base;
  while(w!=NULL){
    if(w->isHandler()){
      if(w->isTriggered(ec) && threadIsPending(this,w->getThread())){
        w->invokeHandler(ec,this);
        *base=w->next;
        w=*base;
        getSiteFromTertiary(this)->deinstallProbe(PROBE_TYPE_ALL);} // ATTENTION
      else{
        base= &(w->next);
        w=*base;}}
    else{
      NOT_IMPLEMENTED; // watcher
    }}
}

void Tertiary::entityOK(EntityCond ec){
  NOT_IMPLEMENTED;
}

void sendTellError(OwnerEntry *oe,Site* toS,Site *mS,int mI,int ec){
  if(SEND_SHORT(toS)) {return;}
  oe->getOneCreditOwner();
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_TELL_ERROR(bs,mS,mI,ec);
  SendTo(toS,bs,M_TELL_ERROR,mS,mI);}

void receiveTellError(Tertiary *t,Site* mS,int mI,EntityCond ec){
  NOT_IMPLEMENTED;}

void Chain::informHandle(Tertiary* t,EntityCond ec){
  InformElem **base=&inform;
  InformElem *cur=*base;
  while(cur!=NULL){
    if(SEND_SHORT(cur->site)){
      *base=cur->next;
      freeInformElem(cur);
      cur=*base;}
    else{
      if(cur->watchcond & ec){
        Assert(t->getTertType()==Te_Manager);
        int OTI=t->getIndex();
        sendTellError(OT->getOwner(OTI),cur->site,mySite,OTI,cur->watchcond & ec);
        *base=cur->next;
        freeInformElem(cur);
        cur=*base;}
      else{
        base=&(cur->next);
        cur=*base;}}}}

/* incorrect lock can recover somewhat */


void tokenLost(Chain *ch,Tertiary *t){
  t->entityProblem(PERM_BLOCKED|PERM_SOME|PERM_ALL);
  ch->informHandle(t,PERM_BLOCKED|PERM_SOME|PERM_ALL);
  return;}

void tokenRecovery(Chain *ch,Tertiary *t){
  t->entityProblem(PERM_SOME);
  ch->informHandle(t,PERM_SOME);
  return;}

void Chain::managerSeesSiteCrash(Tertiary *t,Site *s){
  PD((ERROR_DET,"managerSeesSiteCrash site:%s nr:%d",s->stringrep(),t->getIndex()));
  ChainElem *ce=first->next;
  if(first->getSite()==s){
    if(ce==NULL){
      freeChainElem(first);
      first=ce;
      if(t->getType()==Co_Cell){
        tokenLost(this,t);
        return;}
      Assert(t->getType()==Co_Lock);
      init(mySite,OWNER_ACCESS_NR);
      tokenRecovery(this,t);
      return;}
    if(ce->getSite()->siteStatus()==PERM_SITE){
      freeChainElem(first);
      first=ce;
      tokenRecovery(this,t);
      return;}
    chainSendQuestion(ce->getSite(),t->getIndex(),ce->numId);
    return;}
  if(ce->getSite()!=s){return;} // only interesting on first or second
  chainSendQuestion(first->getSite(),t->getIndex(),first->numId);
  return;}

Site* Chain::proxySeesSiteCrash(Tertiary *t,Site *s,Site *bad){
  PD((ERROR_DET,"proxySeesSiteCrash site:%s dead: %s",s->stringrep(),bad->stringrep()));
  Assert(siteListCheck());
  informHandle(t,PERM_SOME);
  t->entityProblem(PERM_SOME); // informing this site
  return removeSiteNext(s,bad);}

/**********************************************************************/
/*   SECTION 39:: probes                                            */
/**********************************************************************/

void Tertiary::managerProbeFault(Site *s, int pr){
  PD((ERROR_DET,"Mgr probe invoked %d",pr));
  if(pr == PROBE_PERM){
    getChainFromTertiary(this)->managerSeesSiteCrash(this,s);
    return;}
  Assert(pr==PROBE_TEMP);
  NOT_IMPLEMENTED;}

void Tertiary::proxyProbeFault(Site *s,int pr){
  PD((ERROR_DET,"proxy probe invoked %d",pr));
  if(pr == PROBE_PERM){
    entityProblem(PERM_BLOCKED|PERM_SOME);
    return;}
  Assert(pr==PROBE_TEMP);
  NOT_IMPLEMENTED;}

void Site::probeFault(ProbeReturn pr){
  // EKS
  // ATTENTION
  // iterating through size steps and not accsssing the
  // actual getEntry(size), comprende??
  PD((PROBES,"PROBEfAULT  site:%s",stringrep()));
  int limit=OT->getSize();
  for(int ctr = 0; ctr<limit;ctr++){
    OwnerEntry *oe = OT->getEntry(ctr);
    if(oe==NULL){continue;}
    Assert(oe!=NULL);
    Tertiary *tr=oe->getTertiary();
    PD((PROBES,"Informing Manager"));
    Assert(tr->getTertType()==Te_Manager);
    tr->managerProbeFault(this,pr);}

  limit=BT->getSize();
  for(int ctr = 0; ctr<limit;ctr++){
    BorrowEntry *be = BT->getEntry(ctr);
    if(be==NULL){continue;}
    Assert(be!=NULL);
    Tertiary *tr=be->getTertiary();
    tr->proxyProbeFault(this,pr);}
  return;}

/**********************************************************************/
/*   SECTION 40:: communication problem                               */
/**********************************************************************/

inline void returnSendCredit(Site* s,int OTI){
  if(s==mySite){
    OT->getOwner(OTI)->receiveCredit(OTI);
    return;}
  sendCreditBack(s,OTI,1);}

void Site::communicationProblem(MessageType mt,Site*
                                storeSite,int storeIndex
                                ,FaultCode fc,FaultInfo fi){
  int OTI,Index;
  Site *s1,*s2;
  TaggedRef tr;

  PD((SITE,"CommProb type:%d site:%x indx:%d faultCode:%d",
      mt,storeSite, storeIndex, fc));
  switch(mt){
  case M_PORT_SEND:{
      NOT_IMPLEMENTED;}

    case M_REMOTE_SEND:{
      NOT_IMPLEMENTED;}

    case M_ASK_FOR_CREDIT:{
      NOT_IMPLEMENTED;}

    case M_OWNER_CREDIT:{
      NOT_IMPLEMENTED;}

    case M_OWNER_SEC_CREDIT:{
      NOT_IMPLEMENTED;}

    case M_BORROW_CREDIT:{
      NOT_IMPLEMENTED;}

    case M_REGISTER:{
      NOT_IMPLEMENTED;}

    case M_REDIRECT:{
      NOT_IMPLEMENTED;}

    case M_ACKNOWLEDGE:{
      NOT_IMPLEMENTED;}

    case M_SURRENDER:{
      NOT_IMPLEMENTED;}

    case M_CELL_LOCK_GET:{
      if(fc == COMM_FAULT_PERM_NOT_SENT){
        int accNr;
        unmarshal_M_CELL_LOCK_GET((MsgBuffer*)fi,OTI,accNr,s1);
        Assert(OTI==storeIndex);
        returnSendCredit(storeSite,OTI);
        return;}
      if(fc==COMM_FAULT_PERM_MAYBE_SENT){
        return;}
      NOT_IMPLEMENTED;
      break;}

    case  M_CELL_LOCK_FORWARD:{
      if(fc == COMM_FAULT_PERM_NOT_SENT){
        unmarshal_M_CELL_LOCK_FORWARD((MsgBuffer*)fi,s1,OTI,s2);
        Assert(s1==storeSite);
        Assert(OTI=storeIndex);
        returnSendCredit(s1,OTI);
        return;}
      if(fc==COMM_FAULT_PERM_MAYBE_SENT){
        return;}
      NOT_IMPLEMENTED;
      break;}

    case M_CELL_LOCK_DUMP:{
      if(fc == COMM_FAULT_PERM_NOT_SENT){
        unmarshal_M_CELL_LOCK_DUMP((MsgBuffer*)fi,OTI,s1);
        Assert(OTI==storeIndex);
        Assert(s1==mySite);
        returnSendCredit(storeSite,OTI);
        return;}
      if(fc==COMM_FAULT_PERM_MAYBE_SENT){
        return;}
      NOT_IMPLEMENTED;
      break;}

    case M_CELL_CONTENTS:{
      if(fc == COMM_FAULT_PERM_NOT_SENT){
        unmarshal_M_CELL_CONTENTS((MsgBuffer*)fi,s1,OTI,tr);
        Assert(s1==storeSite);
        Assert(OTI=storeIndex);
        returnSendCredit(s1,OTI);
        cellSendContentsFailure(tr,this,storeSite,OTI);
        return;}
      NOT_IMPLEMENTED;
      break;}

    case M_CELL_READ:{
      NOT_IMPLEMENTED;}

    case M_CELL_REMOTEREAD:{
      NOT_IMPLEMENTED;}

    case M_CELL_READANS:{
      NOT_IMPLEMENTED;}

    case M_CELL_CANTPUT:{
      NOT_IMPLEMENTED;}

    case M_LOCK_TOKEN:{
      if(fc == COMM_FAULT_PERM_NOT_SENT){
        unmarshal_M_LOCK_TOKEN((MsgBuffer*)fi,s1,OTI);
        Assert(s1==storeSite);
        Assert(OTI=storeIndex);
        returnSendCredit(s1,OTI);
        lockSendTokenFailure(this,storeSite,OTI);
        return;}
      NOT_IMPLEMENTED;
      break;}

    case M_LOCK_CANTPUT:{
      NOT_IMPLEMENTED;}

    case M_FILE:{
      Assert(0);
      warning("impossible");}

    case M_CHAIN_ACK:{
      NOT_IMPLEMENTED;}

    case M_CHAIN_QUESTION:{
      NOT_IMPLEMENTED;}

    case M_CHAIN_ANSWER:{
      NOT_IMPLEMENTED;}

    case M_ASK_ERROR:{
      NOT_IMPLEMENTED;}

    case M_TELL_ERROR:{
      NOT_IMPLEMENTED;}

    case M_GET_OBJECT:{
      NOT_IMPLEMENTED;}

    case M_GET_OBJECTANDCLASS:{
      NOT_IMPLEMENTED;}

    case M_SEND_OBJECT:{
      NOT_IMPLEMENTED;}

    case M_SEND_OBJECTANDCLASS:{
      NOT_IMPLEMENTED;}

    default:{
      warning("communication problem - impossible");
      Assert(0);}
    }
  Assert(0);
  return;}

/**********************************************************************/
/*   SECTION 41:: Builtins                                            */
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

OZ_C_proc_begin(BItimer,0)
{
  networkTimer();
  return PROCEED;
}
OZ_C_proc_end
#endif

/**********************************************************************/
/*   SECTION 42:: Initialization                                      */
/**********************************************************************/

BIspec perdioSpec[] = {

#ifdef DEBUG_PERDIO
  {"dvset",    2, BIdvset, 0},
  {"timer",    0, BItimer, 0},
#endif
  {0,0,0,0}
};

Bool perdioInit(){
  if(mySite!=NULL) return OK;
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

  genFreeListManager=new GenFreeListManager();
  ownerTable = new OwnerTable(DEFAULT_OWNER_TABLE_SIZE);
  borrowTable = new BorrowTable(DEFAULT_BORROW_TABLE_SIZE);
  msgBufferManager= new MsgBufferManager();
  idCounter  = new FatInt();

  Assert(sizeof(BorrowCreditExtension)==sizeof(Construct_3));
  Assert(sizeof(OwnerCreditExtension)==sizeof(Construct_3));
  Assert(sizeof(Chain)==sizeof(Construct_3));
  Assert(sizeof(ChainElem)==sizeof(Construct_3));
  Assert(sizeof(InformElem)==sizeof(Construct_3));
  Assert(sizeof(CellProxy)==sizeof(CellFrame));
  Assert(sizeof(CellManager)==sizeof(CellFrame));
  Assert(sizeof(CellManager)==sizeof(CellLocal));
  Assert(sizeof(LockProxy)==sizeof(LockFrame));
  Assert(sizeof(LockManager)==sizeof(LockLocal));
  Assert(sizeof(LockManager)==sizeof(LockFrame));
  Assert(sizeof(PortManager)==sizeof(PortLocal));
}

/**********************************************************************/
/*   SECTION 43:: MISC                                                */
/**********************************************************************/


void marshalSite(Site *s,MsgBuffer *buf){
        s->marshalSite(buf);}

Site* getSiteFromBTI(int i){
  return BT->getBorrow(i)->getNetAddress()->site;}

OwnerEntry *getOwnerEntryFromOTI(int i){
  return OT->getOwner(i);}

Tertiary* getTertiaryFromOTI(int i){
  return OT->getOwner(i)->getTertiary();}
