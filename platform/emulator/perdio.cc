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

#include "perdio.hh"
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
void cellLockSendGet(BorrowEntry*);
void cellLockSendDump(BorrowEntry*);

void cellLockReceiveForward(BorrowEntry*,Site*,Site*,int);
void cellLockReceiveDump(OwnerEntry*,Site *);
void cellLockReceiveGet(OwnerEntry*,Site *);

void cellReceiveGet(OwnerEntry* oe,CellManager*,Site*);
void cellReceiveDump(CellManager*,Site*);
void cellReceiveForward(BorrowEntry*,Site*,Site*,int);
void cellReceiveContentsManager(OwnerEntry*,TaggedRef,int);
void cellReceiveContentsFrame(BorrowEntry*,TaggedRef,Site*,int);
void cellReceiveRemoteRead(BorrowEntry*,Site*,int,Site*);
void cellReceiveRead(OwnerEntry*,Site*);
void cellReceiveReadAns(Tertiary*,TaggedRef);
void cellReceiveCantPut(OwnerEntry*,TaggedRef,int,Site*,Site*);
void cellSendReadAns(Site*,Site*,int,TaggedRef);
void cellSendRemoteRead(Site* toS,Site* mS,int mI,Site* fS);
void cellSendContents(TaggedRef tr,Site* toS,Site *mS,int mI);
void cellSendRead(BorrowEntry *be,Site *dS);

void lockReceiveGet(OwnerEntry* oe,LockManager*,Site*);
void lockReceiveDump(LockManager*,Site*);
void lockReceiveTokenManager(OwnerEntry*,int);
void lockReceiveTokenFrame(BorrowEntry*,Site*,int);
void lockReceiveForward(BorrowEntry*,Site*,Site*,int);
void lockReceiveCantPut(OwnerEntry*,int,Site*,Site*);
void lockSendToken(Site*,int,Site*);

void cellSendContentsFailure(TaggedRef,Site*,Site*,int);
void lockReceiveCantPut(LockManager *cm,int mI,Site* rsite, Site* dS);

void receiveAskError(OwnerEntry*,Site*,EntityCond);
void sendAskError(Tertiary*, EntityCond);
void receiveTellError(Tertiary*, Site*, int, EntityCond, Bool);

void chainReceiveAck(OwnerEntry*, Site*);
void chainReceiveAnswer(OwnerEntry*,Site*,int,Site*);
void chainReceiveQuestion(BorrowEntry*,Site*,int,Site*);
void chainSendAnswer(BorrowEntry*,Site*,int,int,Site*);
void chainSendQuestion(Site*,int,Site*);
void chainSendAck(Site*,int);
void receiveAskError(OwnerEntry *,Site*,EntityCond);
void receiveUnAskError(OwnerEntry *,Site*,EntityCond);
void sendTellError(OwnerEntry *,Site*,int,EntityCond,Bool);
void lockSendForward(Site *toS,Site *fS,int mI);
void lockSendTokenFailure(Site*,Site*,int);
void lockSendDump(BorrowEntry*,LockFrame*);
void sendUnAskError(Tertiary*,EntityCond);
void sendRegister(BorrowEntry *);

void printChain(Chain*);

OZ_C_proc_proto(BIapply);

void sendObject(Site* sd, Object *o, Bool);

TaggedRef listifyWatcherCond(EntityCond);
PERDIO_DEBUG_DO(void printTables());



#define OT ownerTable
#define BT borrowTable
#define BTOS(A) BT->getOriginSite(A)
#define BTOI(A) BT->getOriginIndex(A)


Bool withinBorrowTable(int i); // for assertion


/* *********************************************************************/
/*   SECTION 2: global variables                                       */
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

  "chain_ack",
  "chain_question",
  "chain_answer",
  "ask_error",
  "tell_error",
  "get_object",
  "get_objectandclass",
  "send_object",
  "send_objectandclass",
  "register virtual site",
  "file",
  "init virtual site",
  "unask_error",
  "send_gate",
};

/* *********************************************************************/
/*   SECTION 3:: Utility routines                                      */
/* *********************************************************************/

inline void SendTo(Site *toS,MsgBuffer *bs,MessageType mt,Site *sS,int sI){
  int ret=toS->sendTo(bs,mt,sS,sI);
  if(ret==ACCEPTED) return;
  if(ret==PERM_NOT_SENT){
    toS->communicationProblem(mt,sS,sI,COMM_FAULT_PERM_NOT_SENT,(FaultInfo) bs);}
  toS->communicationProblem(mt,sS,sI,COMM_FAULT_TEMP_NOT_SENT,ret);}

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

inline CellSec* getCellSecFromFrameOrManager(Tertiary *t){
  if(t->getTertType()==Te_Frame){
    return ((CellFrame*)t)->getSec();}
  return ((CellManager*)t)->getSec();}


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
      if (!inToSpace(u.tert))
        u.tert=(Tertiary *)(u.tert->gcConstTerm());
    } else {
      Assert(isRef() || isVar());
      PD((GC,"OT var/ref"));
      OZ_collectHeapTerm(u.ref,u.ref);}
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
  void maybeLocalize();
};

void OwnerTable::ownerCheck(OwnerEntry *oe,int OTI){
  Assert(oe->hasFullCredit());
  if (oe->isTertiary()) {
    Tertiary *te=oe->getTertiary();
    Assert(te->getTertType()==Te_Manager);
    te->localize();}
  else{
    PD((PD_VAR,"localize var o:%d",OTI));
    // localize a variable
    if (oe->isVar()) {
      PerdioVar *pvar = oe->getVar();
      SVariable *svar = new SVariable(GETBOARD(pvar));
      svar->setSuspList(pvar->getSuspList());
      doBindSVar(oe->getPtr(),svar);}}
  freeOwnerEntry(OTI);}

void OwnerTable::init(int beg,int end){
  int i=beg;
  while(i<end){
    array[i].makeFree(i+1);
    i++;}
  i--;
  array[i].makeFree(END_FREE);
  nextfree=beg;}

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
/*   SECTION 16a :: div small routines                                  */
/* ******************************************************************* */

Site* getNASiteFromTertiary(Tertiary* t){
  if(t->getTertType()==Te_Manager){
    return mySite;}
  Assert(!(t->getTertType()==Te_Proxy));
  return BT->getOriginSite(t->getIndex());}

int getNAIndexFromTertiary(Tertiary* t){
  if(t->getTertType()==Te_Manager){
    return t->getIndex();}
  Assert(!(t->getTertType()==Te_Proxy));
  return BT->getOriginIndex(t->getIndex());}

inline int getStateFromLockOrCell(Tertiary*t){
  if(t->getType()==Co_Cell){
    if(t->getTertType()==Te_Manager){
      return ((CellManager*)t)->getSec()->getState();}
    Assert(t->getTertType()==Te_Frame);
    return ((CellFrame*)t)->getSec()->getState();}
  Assert(t->getType()==Co_Lock);
  if(t->getTertType()==Te_Manager){
    return ((LockManager*)t)->getSec()->getState();}
  Assert(t->getTertType()==Te_Frame);
  return ((LockFrame*)t)->getSec()->getState();}

/* ******************************************************************* */
/*   SECTION 16b :: div small routines                                  */
/* ******************************************************************* */

inline Bool someTempCondition(EntityCond ec){
  return ec & (TEMP_SOME|TEMP_BLOCKED|TEMP_ME|TEMP_ALL);}

inline Bool somePermCondition(EntityCond ec){
  return ec & (PERM_SOME|PERM_BLOCKED|PERM_ME|PERM_ALL);}

inline EntityCond managerPart(EntityCond ec){
  return ec & (PERM_SOME|PERM_BLOCKED|PERM_ME|TEMP_SOME|TEMP_BLOCKED|TEMP_ME);}

ChainElem *newChainElem(){
  return (ChainElem*) genFreeListManager->getOne_3();}

void freeChainElem(ChainElem* e){
  genFreeListManager->putOne_3((FreeListEntry*) e);}

Chain *newChain(){
  return (Chain*) genFreeListManager->getOne_4();}

void freeChain(Chain* e){
  genFreeListManager->putOne_4((FreeListEntry*) e);}

InformElem* newInformElem(){
  return (InformElem*) genFreeListManager->getOne_3();}

void freeInformElem(InformElem* e){
  genFreeListManager->putOne_3((FreeListEntry*) e);}

inline void installProbe(Site *s,ProbeType pt){
  if(s==mySite) return;
  s->installProbe(pt,0);}

inline void deinstallProbe(Site *s,ProbeType pt){
  if(s==mySite) return;
  s->deinstallProbe(pt);}

Chain* getChainFromTertiary(Tertiary *t){
  Assert(t->getTertType()==Te_Manager);
  if(t->getType()==Co_Cell){
    return ((CellManager *)t)->getChain();}
  Assert(t->getType()==Co_Lock);
  return ((LockManager *)t)->getChain();}

/* ******************************************************************* */
/*   SECTION 16c :: div small routines                                  */
/* ******************************************************************* */

void Chain::newInform(Site* toS,EntityCond ec){
  InformElem *ie=newInformElem();
  if(someTempCondition(ec)){
    probeTemp();}
  ie->init(toS,ec);
  ie->next=inform;
  inform=ie;}

void Chain::releaseChainElem(ChainElem *ce){
  if((!ce->flagIsSet(CHAIN_GHOST))){
    if(hasFlag(INTERESTED_IN_TEMP)){
      deinstallProbe(ce->getSite(),PROBE_TYPE_ALL);}
    else{
      deinstallProbe(ce->getSite(),PROBE_TYPE_PERM);}}
  freeChainElem(ce);}


void Chain::removePerm(ChainElem** base){
  ChainElem *ce=*base;
  *base=ce->next;
  freeChainElem(ce);}

void Chain::removeNextChainElem(ChainElem** base){
  ChainElem *ce=*base;
  *base=ce->next;
  releaseChainElem(ce);}

void Chain::releaseInformElem(InformElem *ie){
  EntityCond ec=ie->watchcond;
  if(someTempCondition(ie->watchcond)){
    InformElem *tmp=inform;
    int interestedInTemp=NO;
    while(tmp!=NULL){
      if(someTempCondition(tmp->watchcond)) {
        interestedInTemp=OK;
        break;}
      tmp=tmp->next;}
    if(!interestedInTemp){
      deProbeTemp();}
    deinstallProbe(ie->site,PROBE_TYPE_ALL);}
  else{
    deinstallProbe(ie->site,PROBE_TYPE_PERM);}
  freeInformElem(ie);}

void Chain::init(Site *s){
  ChainElem *e=newChainElem();
  e->init(s);
  first=last=e;}

Site* Chain::setCurrent(Site* s){
  ChainElem *e=newChainElem();
  e->init(s);
  Site *toS=last->site;
  last->next=e;
  last= e;
  if(s==mySite){
    return toS;}
  ChainElem *de = getFirstNonGhost();
  if(de->site==s){
    de->setFlagAndCheck(CHAIN_DUPLICATE);}
  if(hasFlag(INTERESTED_IN_TEMP)){
    installProbe(s,PROBE_TYPE_ALL);}
  else{
    installProbe(s,PROBE_TYPE_PERM);}
  return toS;}

inline Bool tokenLostCheckProxy(Tertiary*t){
  if(t->getEntityCond() & PERM_ME){
    PD((WEIRD,"lost token found BUT cannot recover"));
    return OK;}
  return NO;}

inline Bool tokenLostCheckManager(Tertiary *t){
  if(getChainFromTertiary(t)->hasFlag(TOKEN_LOST)) {
    PD((WEIRD,"lost token found BUT cannot recover"));
    return OK;}
  return NO;}

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

inline Bool basicThreadIsPending(PendThread *pt,Thread*t){
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

  if (isObject()) {
    BorrowEntry *be=BT->getBorrow(getObject()->getIndex());
    sendHelpX(M_GET_OBJECT,be);
    return;
  }
}


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
void gcFrameToProxy()     {
  borrowTable->gcFrameToProxy();
  OT->maybeLocalize();}

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
    OZ_collectHeapTerm(bl->val,newBL->val);
    newBL->thread = bl->thread->gcThread();
    *last = newBL;
    last = &newBL->next;}
  *last=NULL;}


void CellSec::gcCellSec(){
  gcPendBindingList(&pendBinding);
  switch(stateWithoutAccessBit()){
  case Cell_Lock_Next|Cell_Lock_Requested:{
    next->makeGCMarkSite();}
  case Cell_Lock_Requested:{
    OZ_collectHeapTerm(contents,contents);
    OZ_collectHeapTerm(head,head);
    gcPendThread(&pending);
    return;}
  case Cell_Lock_Next:{
    next->makeGCMarkSite();}
  case Cell_Lock_Invalid:{
    return;}
  case Cell_Lock_Valid:{
    OZ_collectHeapTerm(contents,contents);
  default: Assert(0);}}}

void CellFrame::gcCellFrame(){
  Tertiary *t=(Tertiary*)this;
  t->gcProxy();
  PD((GC,"relocate cellFrame:%d",t->getIndex()));
  sec->gcCellSec();}

void Chain::gcChainSites(){
  ChainElem *ce=first;
  while(ce!=NULL){
    ce->site->makeGCMarkSite();
    ce=ce->next;}
  InformElem *ie=inform;
  while(ie!=NULL){
    ie->site->makeGCMarkSite();
    ie=ie->next;}}

void CellManager::gcCellManager(){
  getChain()->gcChainSites();
  int i=getIndex();
  PD((GC,"relocate cellManager:%d",i));
  OwnerEntry* oe=OT->getOwner(i);
  oe->mkTertiary(this,oe->getFlags());
  CellFrame *cf=(CellFrame*)this;
  sec->gcCellSec();}

void LockSec::gcLockSec(){
  if(state & Cell_Lock_Next){
    getNext()->makeGCMarkSite();}
  PD((GC,"relocate Lock in state %d",state));
  if(pending!=NULL){
    gcPendThread(&pending);
    return;}
  else{
    if(state & Cell_Lock_Valid){
      locker=locker->gcThread();}
    return;}}

void LockFrame::gcLockFrame(){
  Tertiary *t=(Tertiary*)this;
  t->gcProxy();
  PD((GC,"relocate lockFrame:%d",t->getIndex()));
  sec->gcLockSec();}

void LockManager::gcLockManager(){
  getChain()->gcChainSites();
  int i=getIndex();
  PD((GC,"relocate lockManager:%d",i));
  OwnerEntry* oe=OT->getOwner(i);
  oe->mkTertiary(this,oe->getFlags());
  sec->gcLockSec();}

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

void OwnerTable::maybeLocalize()
{
  PD((GC,"owner gc"));
  int i;
  for(i=0;i<size;i++){
    OwnerEntry* o = ownerTable->getOwner(i);
    if(!o->isFree()) {
      PD((GC,"OT o:%d",i));
      if(o->hasFullCredit()){
        ownerCheck(o,i);}}}
  compactify();
  return;
}

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
      if (!b->isGCMarked()) {b->gcBorrow1(i);}}}}

void BorrowEntry::gcBorrow2(int i) {
  if(isTertiary() && getTertiary()->getTertType()==Te_Frame)
    {u.tert= (Tertiary*) u.tert->gcConstTermSpec();}}

void BorrowTable::gcBorrowTable2()
{
  PD((GC,"borrow gc roots"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if(!b->isFree()){
      Assert((b->isVar()) || (b->getTertiary()->getTertType()==Te_Frame)
             || (b->getTertiary()->getTertType()==Te_Proxy));
      if(!(b->isGCMarked())) {b->gcBorrow2(i);}}}
}

void BorrowTable::gcFrameToProxy(){
  PD((GC,"borrow frame to proxy"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if((!b->isFree()) && (!b->isVar())){
      Tertiary *t=b->getTertiary();
      if(t->getTertType()==Te_Frame){
        if((t->getType()==Co_Cell) && ((CellFrame*)t)->getState()==Cell_Lock_Invalid){
          ((CellFrame*)t)->convertToProxy();}
        else{
          if((t->getType()==Co_Lock) && ((LockFrame*)t)->getState()==Cell_Lock_Invalid){
            ((LockFrame*)t)->convertToProxy();}}}}}
}

void maybeUnask(BorrowEntry *be){
  Tertiary *t=be->getTertiary();
  Watcher *w=t->getWatchersIfExist();
  EntityCond ec;
  while(w!=NULL){
    ec=managerPart(w->getWatchCond());
    if(ec!=ENTITY_NORMAL){
      sendUnAskError(t,ec);}
    w=w->getNext();}}

extern TaggedRef gcTagged1(TaggedRef in);

/* OBSERVE - this must done at the end of other gc */
void BorrowTable::gcBorrowTable3(){
  Tertiary *t;
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
          if(b->getTertiary()->maybeHasInform()){
            maybeUnask(b);}
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
                if(cf->dumpCandidate()){
                  cellLockSendDump(b);}}
              break;}
            case Co_Lock:{
              LockFrame *lf=(LockFrame *)t;
              if(lf->isAccessBit()){
                int state=lf->getState();
                lf->resetAccessBit();
                if(lf->dumpCandidate()){
                  cellLockSendDump(b);}}
              break;}
            default:{
              Assert(0);
              break;}}}}
        else{
            Assert(b->getTertiary()->getTertType()==Te_Proxy);
            borrowTable->maybeFreeBorrowEntry(i);}}}}
  compactify();
  hshtbl->compactify();}

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
        continue;}
  next_one:
    aux = getNext(aux,index);
  }

  Assert(used==0);
  compactify();
}

extern void dogcGName(GName *gn);

void gcBorrowNow(int i){
  BorrowEntry *be=borrowTable->getBorrow(i);
  if (!be->isGCMarked()) { // this condition is necessary gcBorrow1
    be->makeGCMark();
    be->gcPO();}}


void PerdioVar::gc(void)
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
      last = &newPL->next;}
    *last = 0;}
  else {
    Assert(isObject() || isObjectGName());
    gcBorrowNow(getObject()->getIndex());
    ptr = getObject()->gcObject();
    if (isObject()) {
      u.aclass = u.aclass->gcClass();}}
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

GName *Abstraction::globalize(){
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_PROC));}
  return getGName();
}

GName *SChunk::globalize() {
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CHUNK));}
  return getGName();
}

void ObjectClass::globalize() {
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CLASS));}
}

void CellLocal::globalize(int myIndex){
  PD((CELL,"globalize cell index:%d",myIndex));
  TaggedRef val1=val;
  CellManager* cm=(CellManager*) this;
  CellSec* sec=new CellSec(val1);
  Chain* ch=newChain();
  ch->init(mySite);
  cm->initOnGlobalize(myIndex,ch,sec);}


void LockLocal::globalize(int myIndex){
  PD((LOCK,"globalize lock index:%d",myIndex));
  Thread* th=getLocker();
  PendThread* pt=getPending();
  LockManager* lm=(LockManager*) this;
  LockSec* sec=new LockSec(th,pt);
  Chain* ch=newChain();
  ch->init(mySite);
  lm->initOnGlobalize(myIndex,ch,sec);}

void CellManager::initOnGlobalize(int index,Chain* ch,CellSec *secX){
  Watcher *w = getWatchersIfExist();
  setTertType(Te_Manager);
  setIndex(index);
  setPtr(ch);
  sec=secX;
  while(w!=NULL){
    if(managerPart(w->getWatchCond()) != ENTITY_NORMAL){
      getChain()->newInform(mySite,w->getWatchCond());}
    w = w->getNext();}}

void LockManager::initOnGlobalize(int index,Chain* ch,LockSec *secX){
  Watcher *w = getWatchersIfExist();
  setTertType(Te_Manager);
  setIndex(index);
  setPtr(ch);
  sec=secX;
  while(w!=NULL){
    if(managerPart(w->getWatchCond()) != ENTITY_NORMAL){
      getChain()->newInform(mySite,w->getWatchCond());}
    w = w->getNext();}}

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
  CellFrame *cf=(CellFrame*) t;
  cf->convertFromProxy();}

inline void convertLockProxyToFrame(Tertiary *t){
  Assert(t->getTertType()==Te_Proxy);
  LockFrame *lf=(LockFrame*) t;
  lf->convertFromProxy();}

inline void maybeConvertLockProxyToFrame(Tertiary *t){
  if(t->getTertType()==Te_Proxy){
    convertLockProxyToFrame(t);}}

inline void maybeConvertCellProxyToFrame(Tertiary *t){
  if(t->getTertType()==Te_Proxy){
    convertCellProxyToFrame(t);}}

PerdioVar *var2PerdioVar(TaggedRef *tPtr){
  TypeOfTerm tag = tagTypeOf(*tPtr);
  if (tag==CVAR) {
    switch (tagged2CVar(*tPtr)->getType()) {
    case PerdioVariable:
      return tagged2PerdioVar(*tPtr);
    default:
      return NULL;}}

  OwnerEntry *oe;
  int i = ownerTable->newOwner(oe);
  PD((GLOBALIZING,"globalize var index:%d",i));

  oe->mkVar(makeTaggedRef(tPtr));

  PerdioVar *pvar=new PerdioVar();
  if (tag==SVAR) {
    pvar->setSuspList(tagged2SVar(*tPtr)->getSuspList());}
  pvar->setIndex(i);
  doBindCVar(tPtr,pvar);
  return pvar;}

/**********************************************************************/
/*   SECTION 20 :: Localizing                                        */
/**********************************************************************/

void CellManager::localize(){
  Assert(getPendBinding()==NULL);
  CellSec *sec=getSec();
  Assert(sec->state==Cell_Lock_Valid);
  TaggedRef tr=sec->getContents();
  CellLocal* cl=(CellLocal*) this;
  cl->setTertType(Te_Local);
  Assert(am.onToplevel());
  cl->setBoard(am.currentBoard());
  cl->setValue(tr);}

void LockManager::localize(){
  LockSec* sec=getSec();
  Assert(sec->state==Cell_Lock_Valid);
  Thread* t=sec->locker;
  PendThread* pt=sec->pending;
  LockLocal* ll=(LockLocal*)this;
  ll->setTertType(Te_Local);
  Assert(am.onToplevel());
  ll->setBoard(am.currentBoard());
  ll->convertToLocal(t,pt);}

void Tertiary::localize()
{
  switch(getType()) {
  case Co_Port:{
    Assert(getTertType()==Te_Manager);
    PD((GLOBALIZING,"GLOBALIZING: localizing tertiary manager"));
    setTertType(Te_Local);
    Assert(am.onToplevel());
    setBoard(am.currentBoard());
    return;}

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

void marshalCreditOutline(Credit credit,MsgBuffer *bs){
  marshalCredit(credit,bs);}


inline Credit unmarshalCredit(MsgBuffer *bs){
  Assert(sizeof(Credit)==sizeof(long));
  Credit c=unmarshalNumber(bs);
  PD((UNMARSHAL,"credit c:%d",c));
  return c;}

Credit unmarshalCreditOutline(MsgBuffer *bs){
  return unmarshalCredit(bs);}

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
  return;}

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
      // mm2
      warning("make persistent of proxy not fully impl.");
      BT->getBorrow(i)->makePersistentBorrowXX();}
    return;}

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
        ((CellFrame *)t)->resetDumpBit();}
      break;}
    case DIF_LOCK:{
      Tertiary *t=ob->getTertiary();
      if((t->getType()==Co_Lock) && (t->getTertType()==Te_Frame)){
        ((LockFrame *)t)->resetDumpBit();}
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
        pvar->setClass(tagged2ObjectClass(deref(clas)));
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
    return val1;}

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

inline OwnerEntry* maybeReceiveAtOwner(Site *mS,int OTI){
  if(mS==mySite){
    OwnerEntry *oe=OT->getOwner(OTI);
    Assert(!oe->isFree());
    oe->receiveCredit(OTI);
    return oe;}
  return NULL;}

inline OwnerEntry* receiveAtOwner(int OTI){
  OwnerEntry *oe=OT->getOwner(OTI);
  Assert(!oe->isFree());
  oe->receiveCredit(OTI);
  return oe;}

inline OwnerEntry* receiveAtOwnerNoCredit(int OTI){
  OwnerEntry *oe=OT->getOwner(OTI);
  Assert(!oe->isFree());
  return oe;}

inline BorrowEntry* receiveAtBorrow(Site* mS,int OTI){
  NetAddress na=NetAddress(mS,OTI);
  BorrowEntry* be=BT->find(&na);
  Assert(be!=NULL);
  be->receiveCredit();
  return be;}

inline BorrowEntry* receiveAtBorrowNoCredit(Site* mS,int OTI){
  NetAddress na=NetAddress(mS,OTI);
  BorrowEntry* be=BT->find(&na);
  Assert(be!=NULL);
  return be;}

inline BorrowEntry* maybeReceiveAtBorrow(Site *mS,int OTI){
  NetAddress na=NetAddress(mS,OTI);
  BorrowEntry* be=BT->find(&na);
  if(be==NULL){sendCreditBack(na.site,na.index,1);}
  else {be->receiveCredit();}
  return be;}

inline void sendPrepOwner(int index){
  OwnerEntry *oe=OT->getOwner(index);
  oe->getOneCreditOwner();}

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

      OwnerEntry *oe=receiveAtOwner(portIndex);
      PortManager *pm=(PortManager*)(oe->getTertiary());
      Assert(pm->checkTertiary(Co_Port,Te_Manager));

      LTuple *lt = new LTuple(t,am.currentUVarPrototype());
      OZ_Term old = pm->exchangeStream(lt->getTail());
      PD((SPECIAL,"just after send port"));
      SiteUnify(makeTaggedLTuple(lt),old); // ATTENTION
      break;
      }

  case M_REMOTE_SEND:    /* index string term */
    {
      int i;
      char *biName;
      OZ_Term t;
      unmarshal_M_REMOTE_SEND(bs,i,biName,t);
      PD((MSG_RECEIVED,"REMOTE_SEND: o:%d bi:%s v:%s",i,biName,toC(t)));

      OwnerEntry *oe=receiveAtOwner(i);
      Tertiary *tert= oe->getTertiary();
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

  case M_SEND_GATE:    /* term */
    {
      OZ_Term t;
      unmarshal_M_SEND_GATE(bs,t);
      PD((MSG_RECEIVED,"SEND_GATE: v:%s",toC(t)));
      sendGate(t);
      break;
    }

  case M_ASK_FOR_CREDIT:
    {
      int na_index;
      Site* rsite;
      unmarshal_M_ASK_FOR_CREDIT(bs,na_index,rsite);
      PD((MSG_RECEIVED,"ASK_FOR_CREDIT index:%d site:%s",na_index,rsite->stringrep()));
      OwnerEntry *oe=receiveAtOwner(na_index);
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
      receiveAtOwnerNoCredit(index)->returnCreditOwner(c);
      break;
    }

  case M_OWNER_SEC_CREDIT:
    {
      int index;
      Credit c;
      Site *s;
      unmarshal_M_OWNER_SEC_CREDIT(bs,s,index,c);
      PD((MSG_RECEIVED,"OWNER_SEC_CREDIT site:%s index:%d credit:%d",s->stringrep(),index,c));
      receiveAtBorrowNoCredit(s,index)->addSecondaryCredit(c,mySite);
      break;
    }

  case M_BORROW_CREDIT:
    {
      int si;
      Credit c;
      Site *sd;
      unmarshal_M_BORROW_CREDIT(bs,sd,si,c);
      PD((MSG_RECEIVED,"BORROW_CREDIT site:%s index:%d credit:%d",sd->stringrep(),si,c));
      receiveAtBorrowNoCredit(sd,si)->addPrimaryCredit(c);
      break;
    }

  case M_REGISTER:
    {
      int OTI;
      Site *rsite;
      unmarshal_M_REGISTER(bs,OTI,rsite);
      PD((MSG_RECEIVED,"REGISTER index:%d site:%s",OTI,rsite->stringrep()));
      OwnerEntry *oe=receiveAtOwner(OTI);
      if (oe->isVar()) {
        PerdioVar *pv=oe->getVar();
        if (!pv->isRegistered(rsite)) {
          pv->registerSite(rsite);}
        else {
          PD((WEIRD,"REGISTER o:%d s:%s already registered",OTI,rsite->stringrep()));}}
      else {
        sendRedirect(rsite,OTI,OT->getOwner(OTI)->getRef());}
      break;
    }

  case M_GET_OBJECT:
  case M_GET_OBJECTANDCLASS:
    {
      int OTI;
      Site *rsite;
      unmarshal_M_GET_OBJECT(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_GET_OBJECT(ANDCLASS) index:%d site:%s",OTI,rsite->stringrep()));
      OwnerEntry *oe=receiveAtOwner(OTI);
      Tertiary *t = oe->getTertiary();
      Assert(isObject(t));
      PD((SPECIAL,"object get %x %x",t,((Object *)t)->getClass()));
      sendObject(rsite,(Object *)t, mt==M_GET_OBJECTANDCLASS);
      break;
    }
  case M_SEND_OBJECT:
    {
      ObjectFields of;
      Site *sd;
      int si;
      unmarshal_M_SEND_OBJECT(bs,sd,si,&of);
      PD((MSG_RECEIVED,"M_SEND_OBJECT site:%s index:%d",sd->stringrep(),si));
      BorrowEntry *be=receiveAtBorrow(sd,si);

      PerdioVar *pv = be->getVar();
      Object *o = pv->getObject();
      if(o==NULL) {
        Assert(0);
        error("M_SEND_OBJECT - don't understand");}
      fillInObject(&of,o);
      ObjectClass *cl;
      if (pv->isObject()) {cl=pv->getClass();}
      else {cl=tagged2ObjectClass(deref(findGName(pv->getGNameClass())));}
      o->setClass(cl);

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
      BorrowEntry *be=receiveAtBorrow(sd,si);
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
      BorrowEntry* be=maybeReceiveAtBorrow(sd,si);
      if (!be) { // if not found, then forget the redirect message
        PD((WEIRD,"REDIRECT: no borrow entry found"));
        break;
      }
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
      OwnerEntry *oe = receiveAtOwner(OTI);

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
      Site* rsite;
      unmarshal_M_CELL_LOCK_GET(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_GET index:%d site:%s",OTI,rsite->stringrep()));
      cellLockReceiveGet(receiveAtOwner(OTI),rsite);
      break;
    }
   case M_CELL_CONTENTS:
    {
      Site *rsite;
      int OTI;
      TaggedRef val;
      unmarshal_M_CELL_CONTENTS(bs,rsite,OTI,val);
      PD((MSG_RECEIVED,"M_CELL_CONTENTS index:%d site:%s val:%s",OTI,rsite->stringrep(),toC(val)));

      OwnerEntry* oe=maybeReceiveAtOwner(rsite,OTI);
      if(oe!=NULL){
        cellReceiveContentsManager(oe,val,OTI);
        break;}

      cellReceiveContentsFrame(receiveAtBorrow(rsite,OTI),val,rsite,OTI);
      break;
    }
  case M_CELL_READ:
    {
      int OTI;
      Site *fS;
      unmarshal_M_CELL_READ(bs,OTI,fS);
      PD((MSG_RECEIVED,"M_CELL_READ"));
      cellReceiveRead(receiveAtOwner(OTI),fS);
      break;
    }
  case M_CELL_REMOTEREAD:
    {
      int OTI;
      Site *fS,*mS;
      unmarshal_M_CELL_REMOTEREAD(bs,mS,OTI,fS);
      cellReceiveRemoteRead(receiveAtBorrow(mS,OTI),mS,OTI,fS);
      break;
    }
  case M_CELL_READANS:
    {
      int index;
      Site*mS;
      TaggedRef val;
      unmarshal_M_CELL_READANS(bs,mS,index,val);
      OwnerEntry *oe=maybeReceiveAtOwner(mS,index);
      if(oe==NULL){
        cellReceiveReadAns(receiveAtBorrow(mS,index)->getTertiary(),val);
        break;}
      cellReceiveReadAns(oe->getTertiary(),val);
      break;
   }
  case M_CELL_LOCK_FORWARD:
    {
      Site *site,*rsite;
      int OTI;
      unmarshal_M_CELL_LOCK_FORWARD(bs,site,OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_FORWARD index:%d site:%s rsite:%s",OTI,site->stringrep(),rsite->stringrep()));

      cellLockReceiveForward(receiveAtBorrow(site,OTI),rsite,site,OTI);
      break;
    }
  case M_CELL_LOCK_DUMP:
    {
      int OTI;
      Site* rsite;
      unmarshal_M_CELL_LOCK_DUMP(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_CELL_LOCK_DUMP index:%d site:%s",OTI,rsite->stringrep()));
      cellLockReceiveDump(receiveAtOwner(OTI),rsite);
      break;
    }
  case M_CELL_CANTPUT:
    {
      Site *rsite;
      int OTI;
      TaggedRef val;
      unmarshal_M_CELL_CANTPUT(bs,OTI,rsite,val);
      PD((MSG_RECEIVED,"M_CELL_CANTPUT index:%d site:%s val:%s",OTI,rsite->stringrep(),toC(val)));
      cellReceiveCantPut(receiveAtOwner(OTI),val,OTI,this,rsite);
      break;
    }
  case M_LOCK_TOKEN:
    {
      Site *rsite;
      int OTI;
      unmarshal_M_LOCK_TOKEN(bs,rsite,OTI);
      PD((MSG_RECEIVED,"M_LOCK_TOKEN index:%d site:%s",OTI,rsite->stringrep()));
      OwnerEntry *oe=maybeReceiveAtOwner(rsite,OTI);
      if(oe!=NULL){
        lockReceiveTokenManager(oe,OTI);
        break;}
      lockReceiveTokenFrame(receiveAtBorrow(rsite,OTI),rsite,OTI);
      break;
    }
  case M_CHAIN_ACK:
    {
      int OTI;
      Site* rsite;
      unmarshal_M_CHAIN_ACK(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_CHAIN_ACK index:%d site:%s",OTI,rsite->stringrep()));
      chainReceiveAck(receiveAtOwner(OTI),rsite);
      break;
    }
  case M_LOCK_CANTPUT:
    {
      Site *rsite;
      int OTI;
      TaggedRef val;
      unmarshal_M_LOCK_CANTPUT(bs,OTI,rsite);
      PD((MSG_RECEIVED,"M_LOCK_CANTPUT index:%d site:%s val:%s",OTI,rsite->stringrep()));
      lockReceiveCantPut(receiveAtOwner(OTI),OTI,this,rsite);
      break;
    }
  case M_CHAIN_QUESTION:
   {
      Site *site,*rsite,*deadS;
      int OTI;
      unmarshal_M_CHAIN_QUESTION(bs,OTI,site,deadS);
      PD((MSG_RECEIVED,"M_CHAIN_QUESTION index:%d site:%s",OTI,site->stringrep()));
      BorrowEntry *be=maybeReceiveAtBorrow(site,OTI);
      if(be==NULL) break;
      chainReceiveQuestion(be,site,OTI,deadS);
      break;
   }
  case M_CHAIN_ANSWER:
    {
      Site *rsite,*deadS;
      int OTI;
      int ans;
      unmarshal_M_CHAIN_ANSWER(bs,OTI,rsite,ans,deadS);
      PD((MSG_RECEIVED,"M_CHAIN_ANSWER index:%d site:%s val:%d",OTI,rsite->stringrep(),ans));
      chainReceiveAnswer(receiveAtOwner(OTI),rsite,ans,deadS);
      break;
    }

  case M_TELL_ERROR:
    {
      Site *site;
      int OTI;
      int ec,flag;
      unmarshal_M_TELL_ERROR(bs,site,OTI,ec,flag);
      PD((MSG_RECEIVED,"M_TELL_ERROR index:%d site:%s ec:%d",OTI,site->stringrep(),ec));
      BorrowEntry *be=maybeReceiveAtBorrow(site,OTI);
      if(be==NULL) break;
      receiveTellError(be->getTertiary(),site,OTI,ec,flag);
      break;
    }

  case M_ASK_ERROR:
    {
      int OTI;
      int ec;
      Site *toS;
      unmarshal_M_ASK_ERROR(bs,OTI,toS,ec);
      PD((MSG_RECEIVED,"M_ASK_ERROR index:%d ec:%d toS:%s",OTI,ec,toS->stringrep()));
      receiveAskError(receiveAtOwner(OTI),toS,ec);
      break;
    }
  case M_UNASK_ERROR:
    {
      int OTI;
      int ec;
      Site *toS;
      unmarshal_M_UNASK_ERROR(bs,OTI,toS,ec);
      PD((MSG_RECEIVED,"M_UNASK_ERROR index:%d ec:%d toS:%s",OTI,ec,toS->stringrep()));
      receiveUnAskError(receiveAtOwner(OTI),toS,ec);
      break;
    }
  /*  case M_SITEDWN_DBG:
    {
      PD((MSG_RECEIVED,"M_SITEDWN_DBG %s",this->stringrep()));
      receiveSiteDwnDbg();
      break;
    }
    */
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

void PerdioVar::acknowledge(OZ_Term *p){
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
      SiteUnify(val,u.bindings->val);}
    else {
      PD((PD_VAR,"redirect pending unify =%s",toC(u.bindings->val)));
      PD((THREAD_D,"start thread redirect %x",u.bindings->thread));
      pushUnify(u.bindings->thread,val,u.bindings->val);
      oz_resumeFromNet(u.bindings->thread);}
    PendBinding *tmp=u.bindings->next;
    u.bindings->dispose();
    u.bindings=tmp;}
}

void sendRedirect(ProxyList *pl,OZ_Term val, Site* ackSite, int OTI){
  while (pl) {
    Site* sd=pl->sd;
    ProxyList *tmp=pl->next;
    pl->dispose();
    pl = tmp;

    if (sd==ackSite) {
      sendAcknowledge(sd,OTI);}
    else {
      sendRedirect(sd,OTI,val);}}
}

void bindPerdioVar(PerdioVar *pv,TaggedRef *lPtr,TaggedRef v)
{
  PD((PD_VAR,"bindPerdioVar by thread: %x",am.currentThread()));
  if (pv->isManager()) {
    PD((PD_VAR,"bind manager o:%d v:%s",pv->getIndex(),toC(v)));
    pv->primBind(lPtr,v);
    OT->getOwner(pv->getIndex())->mkRef();
    sendRedirect(pv->getProxies(),v,mySite,pv->getIndex());
  } else if (pv->isObject() || pv->isObjectGName()) {
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

void sendObject(Site* sd, Object *o, Bool sendClass){ // holding one credit
  int OTI = o->getIndex();
  OT->getOwner(OTI)->getOneCreditOwner();
  MsgBuffer *bs= msgBufferManager->getMsgBuffer(sd);
  if(sendClass){
    marshal_M_SEND_OBJECTANDCLASS(bs,mySite,OTI,o);
    SendTo(sd,bs,M_SEND_OBJECTANDCLASS,mySite,OTI);}
  else{
    marshal_M_SEND_OBJECT(bs,mySite,OTI,o);
    SendTo(sd,bs,M_SEND_OBJECT,mySite,OTI);}}

/**********************************************************************/
/*   SECTION 27:: Credit protocol                                     */
/**********************************************************************/

void sendCreditBack(Site* sd,int OTI,Credit c){
  int ret;
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

void Chain::informHandleTempOnAdd(OwnerEntry* oe,Tertiary *t,Site *s){
  InformElem *ie=getInform();
  while(ie!=NULL){
    if(ie->site==s){
      EntityCond ec=ie->wouldTrigger(TEMP_BLOCKED|TEMP_SOME|TEMP_ME);
      if(ec!=ENTITY_NORMAL){
        sendTellError(oe,s,t->getIndex(),ec,TRUE);}}
    ie=ie->next;}}

void cellLockReceiveGet(OwnerEntry* oe,Site* toS){
  Tertiary* t=oe->getTertiary();
  Chain *ch=getChainFromTertiary(t);
  if(ch->hasFlag(TOKEN_LOST)){
    PD((ERROR_DET,"TOKEN_LOST message bouncing"));
    sendTellError(oe,toS,t->getIndex(),PERM_BLOCKED|PERM_SOME|PERM_ME,true);
    return;}
  if(t->getType()==Co_Cell){
    cellReceiveGet(oe,(CellManager*) t,toS);}
  else{
    lockReceiveGet(oe,(LockManager*) t,toS);}
  if(ch->hasFlag(INTERESTED_IN_OK)){
    Assert(ch->tempConnectionInChain());
    ch->informHandleTempOnAdd(oe,t,toS);}}

void cellLockReceiveForward(BorrowEntry *be,Site* toS,Site* mS,int mI){
  if(be->getTertiary()->getType()==Co_Cell){
    cellReceiveForward(be,toS,mS,mI);
    return;}
  lockReceiveForward(be,toS,mS,mI);}

void cellLockSendGet(BorrowEntry *be){
  NetAddress *na=be->getNetAddress();
  Site *toS=na->site;
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  PD((CELL,"M_CELL_LOCK_GET indx:%d site:%s",na->index,toS->stringrep()));
  marshal_M_CELL_LOCK_GET(bs,na->index,mySite);
  SendTo(toS,bs,M_CELL_LOCK_GET,toS,na->index);}

void cellLockSendForward(Site *toS,Site *fS,int mI){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_LOCK_FORWARD(bs,mySite,mI,fS);
  SendTo(toS,bs,M_CELL_LOCK_FORWARD,mySite,mI);}

void cellLockReceiveDump(OwnerEntry *oe,Site* fromS){
  Tertiary *t=oe->getTertiary();
  if(t->getType()==Co_Cell){
    cellReceiveDump((CellManager*) t,fromS);}
  else{
    lockReceiveDump((LockManager*) t,fromS);}}

void cellLockSendDump(BorrowEntry *be){
  NetAddress *na=be->getNetAddress();
  Site *toS=na->site;
  if(SEND_SHORT(toS)){return;}
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_LOCK_DUMP(bs,na->index,mySite);
  SendTo(toS,bs,M_CELL_LOCK_DUMP,toS,na->index);}

/**********************************************************************/
/*   SECTION 28:: Cell protocol - receive                            */
/**********************************************************************/

Bool CellSec::secForward(Site* toS,TaggedRef &val){
  if(state==Cell_Lock_Valid){
    state=Cell_Lock_Invalid;
    val=contents;
    return OK;}
  Assert(state==Cell_Lock_Requested);
  state=Cell_Lock_Requested|Cell_Lock_Next;
  next=toS;
  return NO;}

Bool CellSec::secReceiveContents(TaggedRef val,Site* &toS,TaggedRef &outval){
  Thread *th=pending->thread;
  pushUnify(th,head,val);
  pendThreadResumeAll(pending);
  pending=NULL;
  outval=contents;
  if(state & Cell_Lock_Next){
    state = Cell_Lock_Invalid;
    toS=next;
    return OK;}
  state = Cell_Lock_Valid;
  return NO;}

void CellSec::secReceiveReadAns(TaggedRef val){
  PendBinding* pb=pendBinding;
  while(pb!=NULL){
    pushUnify(pb->thread,pb->val,val);
    oz_resumeFromNet(pb->thread);
    pb=pb->next;}
  pendBinding=NULL;}

Bool CellSec::secReceiveRemoteRead(TaggedRef &val){
  if(state==Cell_Lock_Invalid) return NO;
  val=contents;
  return TRUE;}

void cellReceiveGet(OwnerEntry* oe,CellManager* cm,Site* toS){
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);
  Chain *ch=cm->getChain();
  Site* current=ch->setCurrent(toS);
  PD((CELL,"CellMgr Received get from %s",toS->stringrep()));
  //DebugCode(printChain(ch);)
  if(current==mySite){
    PD((CELL,"CELL - shortcut in cellReceiveGet"));
    TaggedRef val;
    if(cm->getSec()->secForward(toS,val)){
      oe->getOneCreditOwner();
      cellSendContents(val,toS,mySite,cm->getIndex());}
    return;}
  oe->getOneCreditOwner();
  cellLockSendForward(current,toS,cm->getIndex());}

void cellReceiveDump(CellManager *cm,Site *fromS){
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);
  if((cm->getChain()->getCurrent()!=fromS) || (cm->getState()!=Cell_Lock_Invalid)){
    PD((WEIRD,"CELL dump not needed"));
    return;}
  TaggedRef tr=oz_newVariable();
  cellDoExchange((Tertiary *)cm,tr,tr,DummyThread);
  return;}

void cellReceiveForward(BorrowEntry *be,Site *toS,Site* mS,int mI){
  CellFrame *cf=(CellFrame*) be->getTertiary();
  Assert(cf->getTertType()==Te_Frame);
  Assert(cf->getType()==Co_Cell);
  CellSec *sec=cf->getSec();
  TaggedRef val;
  cf->resetDumpBit();
  if(!sec->secForward(toS,val)) return;
  be->getOneMsgCredit();
  cellSendContents(val,toS,mS,mI);
  return;}

void cellReceiveContentsManager(OwnerEntry *oe,TaggedRef val,int mI){
  CellManager *cm=(CellManager*)oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);
  if(tokenLostCheckManager(cm)) return; // ERROR-HOOK ATTENTION
  chainReceiveAck(oe,mySite);
  CellSec *sec=cm->getSec();
  Site *toS;
  TaggedRef outval;
  if(!sec->secReceiveContents(val,toS,outval)) return;
  oe->getOneCreditOwner();
  cellSendContents(outval,toS,mySite,mI);
  return;}

void cellReceiveContentsFrame(BorrowEntry *be,TaggedRef val,Site *mS,int mI){
  CellFrame *cf=(CellFrame*) be->getTertiary();
  Assert(cf->getType()==Co_Cell);
  Assert(cf->getTertType()==Te_Frame);
  if(tokenLostCheckProxy(cf)) return; // ERROR-HOOK ATTENTION
  be->getOneMsgCredit();
  chainSendAck(mS,mI);
  CellSec *sec=cf->getSec();
  TaggedRef outval;
  Site *toS;
  if(!sec->secReceiveContents(val,toS,outval)) return;
  be->getOneMsgCredit();
  cellSendContents(outval,toS,mS,mI);}

void cellReceiveRemoteRead(BorrowEntry *be,Site* mS,int mI,Site* fS){
  Tertiary* t=be->getTertiary();
  Assert(t->getTertType()==Te_Frame);
  Assert(t->getType()==Co_Cell);
  CellSec *sec=((CellFrame*)t)->getSec();
  be->getOneMsgCredit();
  TaggedRef val;
  if(sec->secReceiveRemoteRead(val)) {
    cellSendReadAns(fS,mS,mI,val);
    return;}
  PD((WEIRD,"miss on read"));
  be->getOneMsgCredit();
  cellSendRead(be,fS);}

void cellReceiveRead(OwnerEntry *oe,Site* fS){
  CellManager* cm=(CellManager*) oe->getTertiary();
  Assert(cm->getTertType()==Te_Manager);
  Assert(cm->getType()==Co_Cell);
  CellSec *sec=cm->getSec();
  oe->getOneCreditOwner();
  Chain* ch=cm->getChain();
  if(ch->getCurrent()==mySite){
    cellSendReadAns(fS,mySite,cm->getIndex(),sec->getContents());
    return;}
  cellSendRemoteRead(ch->getCurrent(),mySite,cm->getIndex(),fS);}

void cellReceiveReadAns(Tertiary* t,TaggedRef val){
  Assert((t->getTertType()==Te_Manager)|| (t->getTertType()==Te_Frame));
  getCellSecFromFrameOrManager(t)->secReceiveReadAns(val);}

/**********************************************************************/
/*   SECTION 29:: Cell protocol - holding credit                      */
/**********************************************************************/

void cellSendReadAns(Site* toS,Site* mS,int mI,TaggedRef val){
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_READANS(bs,mS,mI,val);
  SendTo(toS,bs,M_CELL_READANS,mS,mI);}

void cellSendRemoteRead(Site* toS,Site* mS,int mI,Site* fS){
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_REMOTEREAD(bs,mS,mI,fS);
  SendTo(toS,bs,M_CELL_REMOTEREAD,mS,mI);}

void cellSendContents(TaggedRef tr,Site* toS,Site *mS,int mI){
  PD((CELL,"Cell Send Contents to:%s",toS->stringrep()));
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_CONTENTS(bs,mS,mI,tr);
  PD((SPECIAL,"CellContents %s",toC(tr)));
  SendTo(toS,bs,M_CELL_CONTENTS,mS,mI);}

void cellSendRead(BorrowEntry *be,Site *dS){
  NetAddress *na=be->getNetAddress();
  Site *toS=na->site;
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CELL_READ(bs,na->index,dS);
  SendTo(toS,bs,M_CELL_READ,na->site,na->index);}

/**********************************************************************/
/*   SECTION 30:: Cell protocol - basics                              */
/**********************************************************************/

inline CellSec *getCellSecFromTert(Tertiary *c){
  if(c->getTertType()==Te_Manager){
    return ((CellManager*)c)->getSec();}
  Assert(c->getTertType()!=Te_Proxy);
  return ((CellFrame*)c)->getSec();}

inline Bool maybeInvokeHandler(Tertiary* t,Thread* th){
  if(t->getEntityCond()==ENTITY_NORMAL) return NO;
  if(!t->handlerExists(th)) return NO;
  return OK;}

void genInvokeHandlerLockOrCell(Tertiary* t,Thread* th){
  if(!t->handlerExists(th)) return;
  Watcher** base=t->findWatcherBase(th,t->getEntityCond());
  if(base==NULL) return;
  Watcher *hit=*base;
  hit->invokeHandler(t->getEntityCond(),t);
  t->releaseWatcher(hit);
  *base= hit->getNext();}

void CellSec::exchange(Tertiary* c,TaggedRef old,TaggedRef nw,Thread* th){
  switch(state){
  case Cell_Lock_Valid:{
    PD((CELL,"CELL: exchange on valid"));
    TaggedRef tr=contents;
    contents=nw;
    SiteUnify(tr,old);
    return;}
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:{
    PD((CELL,"CELL: exchange on requested"));
    pendThreadAddToEnd(&pending,th);
    SiteUnify(contents,old);
    contents=nw;
    if(c->errorIgnore()) return;
    break;}
  case Cell_Lock_Invalid:{
    PD((CELL,"CELL: exchange on invalid"));
    state=Cell_Lock_Requested;
    head=old;
    contents=nw;
    Assert(isRealThread(th) || th==DummyThread);
    pendThreadAddToEnd(&pending,th);
    int index=c->getIndex();
    if(c->getTertType()==Te_Frame){
      BorrowEntry* be=BT->getBorrow(index);
      be->getOneMsgCredit();
      cellLockSendGet(be);
      if(c->errorIgnore()) return;
      return;}
    Assert(c->getTertType()==Te_Manager);
    Site *toS=((CellManager*)c)->getChain()->setCurrent(mySite);
    sendPrepOwner(index);
    cellLockSendForward(toS,mySite,index);
    if(c->errorIgnore()) return;
    break;}
  default: Assert(0);}
  if(maybeInvokeHandler(c,th)){ // ERROR-HOOK
    genInvokeHandlerLockOrCell(c,th);}
}

void cellDoExchange(Tertiary *c,TaggedRef old,TaggedRef nw,Thread* th){
  PD((SPECIAL,"exchange old:%d new:%s",toC(old),toC(nw)));
  if(c->getTertType()==Te_Proxy){
    convertCellProxyToFrame(c);}
  PD((CELL,"CELL: exchange on %s-%d",getNASiteFromTertiary(c)->stringrep(),getNAIndexFromTertiary(c)));
  getCellSecFromTert(c)->exchange(c,old,nw,th);}

void CellSec::access(Tertiary* c,TaggedRef val){
  switch(state){
  case Cell_Lock_Valid:{
    PD((CELL,"CELL: access on valid"));
    pushUnify(am.currentThread(),val,contents);
    return;}
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:{
    PD((CELL,"CELL: access on requested"));
    break;}
  case Cell_Lock_Invalid:{
    PD((CELL,"CELL: access on invalid"));
    break;}
  default: Assert(0);}

  int index=c->getIndex();
  Thread* th=am.currentThread();
  Bool ask;
  if(pendBinding!=NULL) ask=NO;
  else ask=OK;
  PendBinding *pb=new PendBinding(val,th,pendBinding);
  oz_suspendOnNet(th);
  if(!ask) return;
  if(c->getTertType()==Te_Frame){
    BorrowEntry *be=BT->getBorrow(index);
    be->getOneMsgCredit();
    cellSendRead(be,mySite);
    return;}
  sendPrepOwner(index);
  cellSendRemoteRead(((CellManager*)c)->getChain()->getCurrent(),mySite,index,mySite);
  if(c->errorIgnore()) return;
  if(maybeInvokeHandler(c,th)){// ERROR-HOOK
    genInvokeHandlerLockOrCell(c,th);
  }}

void cellDoAccess(Tertiary *c,TaggedRef val){
  if(c->getTertType()==Te_Proxy){
    convertCellProxyToFrame(c);}
  getCellSecFromTert(c)->access(c,val);}

/**********************************************************************/
/*   SECTION 31a:: ChainElem routines                                  */
/**********************************************************************/

void ChainElem::init(Site *s){
  next=NULL;
  flags=0;
  site=s;}

/**********************************************************************/
/*   SECTION 31b:: InformElem routines                                  */
/**********************************************************************/

void InformElem::init(Site*s,EntityCond c){// ATTENTION
  site=s;
  next=NULL;
  watchcond=c;
  foundcond=0;
  if(someTempCondition(c)){
    s->installProbe(PROBE_TYPE_ALL,0);}}

/**********************************************************************/
/*   SECTION 31c:: Chain routines working on ChainElem                */
/**********************************************************************/

Bool Chain::basicSiteExists(ChainElem *ce,Site* s){
  while(ce!=NULL){
    if(ce->site==s) {return OK;}
    ce=ce->next;}
  return NO;}

ChainElem** Chain::getFirstNonGhostBase(){
  if(first==last) {return &first;}
  ChainElem **ce=&first;
  while((*ce)->next->flagIsSet(CHAIN_GHOST)){
    ce= &((*ce)->next);}
  return ce;}

ChainElem* Chain::getFirstNonGhost(){
  return *getFirstNonGhostBase();}

Bool Chain::siteExists(Site *s){
  return basicSiteExists(getFirstNonGhost(),s);}

Bool Chain::siteOrGhostExists(Site *s){
  return basicSiteExists(first,s);}

void Chain::makeGhost(ChainElem* ce){
  ce->setFlagAndCheck(CHAIN_GHOST);
  ce->resetFlagAndCheck(CHAIN_QUESTION_ASKED);
  if(hasFlag(INTERESTED_IN_TEMP)){
    deinstallProbe(ce->site,PROBE_TYPE_ALL);}
  else{
    deinstallProbe(ce->site,PROBE_TYPE_PERM);}}

void Chain::removeBefore(Site* s){
  ChainElem **base,*ce;
  base=getFirstNonGhostBase();
  Assert(siteExists(s));
  while((*base)->site!=s){
    if((*base)->flagIsSet(CHAIN_QUESTION_ASKED)){
      makeGhost(*base);
      base=&((*base)->next);}
    else{
      removeNextChainElem(base);}}}

ChainElem *Chain::findAfter(Site *s){
  Assert(siteExists(s));
  if(first->next==NULL){
    return NULL;}
  ChainElem *ce=getFirstNonGhost();
  while(ce->site!=s){
    ce=ce->next;}
  return ce->next;}

ChainElem *Chain::findChainElemFrom(ChainElem *ce,Site*s){// ATTENTION
  while(TRUE){
    if(ce->site==s) return ce;
    ce=ce->next;
    if(ce==NULL) return NULL;}}

Bool Chain::removeGhost(Site* s){
  ChainElem **ce=&first;
  while(TRUE){
    if((*ce)==NULL) return NO;
    if(!(*ce)->flagIsSet(CHAIN_GHOST)) return NO;
    if((*ce)->site==s) {
      removeNextChainElem(ce);
      return OK;}
    ce = &((*ce)->next);}}

void Chain::probeTemp(){
  ChainElem *ce=getFirstNonGhost();
  while(ce!=NULL){
    installProbe(ce->site,PROBE_TYPE_ALL);
    ce=ce->next;}}

void Chain::deProbeTemp(){
  ChainElem *ce=getFirstNonGhost();
  while(ce!=NULL){
    deinstallProbe(ce->site,PROBE_TYPE_ALL);
    ce=ce->next;}}

Bool Chain::tempConnectionInChain(){
  ChainElem *ce=first;
  while(ce!=NULL){
    if(ce->site->siteStatus()==SITE_TEMP) return OK;
    ce=ce->next;}
  return NO;}

/**********************************************************************/
/*   SECTION 31c:: Chain routines working on InformElem                */
/**********************************************************************/

void Chain::removeInformOnPerm(Site *s){
  InformElem **ce= &inform;
  InformElem *tmp;
  while(*ce!=NULL){
    if((*ce)->site==s){
      tmp=*ce;
      releaseInformElem(tmp);
      *ce=tmp->next;
      return;}
    ce= &((*ce)->next);}}

/**********************************************************************/
/*   SECTION 32:: chain protocol                                      */
/**********************************************************************/

void chainSendAck(Site* toS, int mI){
  if(SEND_SHORT(toS)) {return;}
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  PD((CHAIN,"M_CHAIN_ACK indx:%d site:%s",mI,toS->stringrep()));
  marshal_M_CHAIN_ACK(bs,mI,mySite);
  SendTo(toS,bs,M_CHAIN_ACK,toS,mI);}

void chainReceiveAck(OwnerEntry* oe,Site* rsite){
  Tertiary *t=oe->getTertiary();
  Chain* chain=tertiaryGetChain(t);
  if(!(chain->siteExists(rsite))) {
    return;}
  chain->removeBefore(rsite);
  //DebugCode(printChain(chain);)
}

  ChainAnswer answerChainQuestion(Tertiary *t){
  if(t->getTertType()==Te_Proxy){
    return PAST_ME;}
  switch(getStateFromLockOrCell(t)){
  case Cell_Lock_Invalid:
    return PAST_ME;
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:
    return BEFORE_ME;
  case Cell_Lock_Valid|Cell_Lock_Next:
  case Cell_Lock_Valid:
    return AT_ME;
  default:
    Assert(0);}
  Assert(0);
  return BEFORE_ME;}

void chainReceiveQuestion(BorrowEntry *be,Site* site,int OTI,Site* deadS){
  if(be==NULL){
    chainSendAnswer(be,site,OTI,PAST_ME,deadS);}
  chainSendAnswer(be,site,OTI,answerChainQuestion(be->getTertiary()),deadS);}

void chainReceiveAnswer(OwnerEntry* oe,Site* fS,int ans,Site* deadS){
  Tertiary* t=oe->getTertiary();
  getChainFromTertiary(t)->receiveAnswer(t,fS,ans,deadS);
  //DebugCode(printChain(getChainFromTertiary(t));)
}

inline void maybeChainSendQuestion(ChainElem *ce,Tertiary *t,Site* deadS){
  if(ce->getSite()!=mySite){
    if(!(ce->flagIsSet(CHAIN_QUESTION_ASKED))){
      ce->setFlagAndCheck(CHAIN_QUESTION_ASKED);
      chainSendQuestion(ce->getSite(),t->getIndex(),deadS);}
    return;}
  getChainFromTertiary(t)->receiveAnswer(t,mySite,answerChainQuestion(t),deadS);}

void Chain::receiveAnswer(Tertiary* t,Site* site,int ans,Site* deadS){
  PD((ERROR_DET,"chain receive answer %d",ans));
  if(hasFlag(TOKEN_LOST)) return;
  if(removeGhost(site)) return;
  Assert(siteExists(site));
  ChainElem **base=getFirstNonGhostBase();
  ChainElem *dead,*answer;

  while(((*base)->site!=deadS) && ((*base)->site!=site)){
    base= &((*base)->next);}
  if((*base)->site==site){ //      order Answer-Dead
    PD((ERROR_DET,"chain receive answer - order answer-dead"));
    answer=*base;
    answer->resetFlagAndCheck(CHAIN_QUESTION_ASKED);
    dead=answer->next;
    if(dead->site!=deadS) {
      PD((ERROR_DET,"dead->site!=deadS"));
      Assert(answer==getFirstNonGhost());
      return;}
    if(answer->flagIsSet(CHAIN_DUPLICATE)){
      PD((ERROR_DET,"answer->flagIsSet(CHAIN_DUPLICATE)"));
      dead->setFlagAndCheck(CHAIN_PAST);
      managerSeesSitePerm(t,deadS);
      return;}
    if(ans==PAST_ME){
      PD((ERROR_DET,"ans==PAST_ME"));
      dead->setFlagAndCheck(CHAIN_PAST);
      managerSeesSitePerm(t,deadS);
      return;}
    PD((ERROR_DET,"Manager will receive CANT_PUT from %s",site->stringrep()));
    dead->setFlagAndCheck(CHAIN_CANT_PUT);
    dead->resetFlag(CHAIN_BEFORE);
    dead->resetFlag(CHAIN_PAST);
    //DebugCode(printChain(this);)
    return;}
  PD((ERROR_DET,"chain receive answer - order dead-answer"));
  dead= *base;                      // order Dead-Answer
  answer=dead->next;
  Assert(answer->site=site);
  Assert(ans==BEFORE_ME);
  answer->resetFlagAndCheck(CHAIN_QUESTION_ASKED);
  dead->setFlagAndCheck(CHAIN_BEFORE);
  managerSeesSitePerm(t,deadS);
  return;}

void chainSendQuestion(Site* toS,int mI,Site *deadS){
  OT->getOwner(mI)->getOneCreditOwner();
  PD((ERROR_DET,"chainSendQuestion  %s",toS->stringrep()));
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CHAIN_QUESTION(bs,mI,mySite,deadS);
  SendTo(toS,bs,M_CHAIN_QUESTION,mySite,mI);}

void chainSendAnswer(BorrowEntry* be,Site* toS, int mI, int ans, Site *deadS){
  be->getOneMsgCredit();
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_CHAIN_ANSWER(bs,mI,mySite,ans,deadS);
  SendTo(toS,bs,M_CHAIN_ANSWER,toS,mI);}

/**********************************************************************/
/*   SECTION 33:: Lock protocol - receive                             */
/**********************************************************************/

Bool LockSec::secReceiveToken(Tertiary* t,Site* &toS){
  if(state & Cell_Lock_Next) state = Cell_Lock_Next|Cell_Lock_Valid;
  else state=Cell_Lock_Valid;
  if(pending->thread!=DummyThread){
    locker=pendThreadResumeFirst(&pending);
    return OK;}
  pendThreadRemoveFirst(getPendBase());
  if(pending==NULL){
    locker=NULL;
    if(state!=Cell_Lock_Valid|Cell_Lock_Next) return OK;
    toS=next;
    return OK;}
  unlockComplex(t);
  return OK;}

Bool LockSec::secForward(Site* toS){
  if(state==Cell_Lock_Valid){
    if(locker==NULL){
      state=Cell_Lock_Invalid;
      return OK;}
    state=Cell_Lock_Valid|Cell_Lock_Next;
    next=toS;
    return NO;}
  Assert(state==Cell_Lock_Requested);
  state= Cell_Lock_Requested|Cell_Lock_Next;
  next=toS;
  return NO;}

void lockReceiveGet(OwnerEntry* oe,LockManager* lm,Site* toS){
  Assert(lm->getType()==Co_Lock);
  Assert(lm->getTertType()==Te_Manager);
  Chain *ch=lm->getChain();
  PD((LOCK,"LockMgr Received get from %s",toS->stringrep()));
  Site* current=ch->setCurrent(toS);
  //DebugCode(printChain(ch);)
  if(current==mySite){                             // shortcut
    PD((LOCK," shortcut in lockReceiveGet"));
    TaggedRef val;
    if(lm->getSec()->secForward(toS)){
      oe->getOneCreditOwner();
      lockSendToken(mySite,lm->getIndex(),toS);}
    return;}
  oe->getOneCreditOwner();
  cellLockSendForward(current,toS,lm->getIndex());}

void lockReceiveDump(LockManager* lm,Site *fromS){
  Assert(lm->getType()==Co_Lock);
  Assert(lm->getTertType()==Te_Manager);
  LockSec* sec=lm->getSec();
  if((lm->getChain()->getCurrent()!=fromS) || (sec->getState()!=Cell_Lock_Invalid)){
    PD((WEIRD,"WEIRD- LOCK dump not needed"));
    return;}
  Assert(sec->getState()==Cell_Lock_Invalid);
  sec->lockComplex(DummyThread,lm);
  return;}

void lockReceiveTokenManager(OwnerEntry* oe,int mI){
  Tertiary *t=oe->getTertiary();
  Assert(t->getType()==Co_Lock);
  Assert(t->getTertType()==Te_Manager);
  if(tokenLostCheckManager(t)) return; // ERROR-HOOK ATTENTION
  LockManager*lm=(LockManager*)t;
  chainReceiveAck(oe,mySite);
  LockSec *sec=lm->getSec();
  Site* toS;
  if(sec->secReceiveToken(t,toS)) return;
   //DebugCode(printChain(lm->getChain());)
  oe->getOneCreditOwner();
  lockSendToken(mySite,mI,toS);}

void lockReceiveTokenFrame(BorrowEntry* be, Site *mS,int mI){
  LockFrame *lf=(LockFrame*) be->getTertiary();
  Assert(lf->getType()==Co_Lock);
  Assert(lf->getTertType()==Te_Frame);
  if(tokenLostCheckProxy(lf)) return; // ERROR-HOOK ATTENTION
  be->getOneMsgCredit();
  chainSendAck(mS,mI);
  LockSec *sec=lf->getSec();
  Site* toS;
  if(sec->secReceiveToken(lf,toS)) return;
  be->getOneMsgCredit();
  lockSendToken(mS,mI,toS);}

void lockReceiveForward(BorrowEntry *be,Site *toS,Site* mS,int mI){
  LockFrame *lf= (LockFrame*) be->getTertiary();
  lf->resetDumpBit();
  Assert(lf->getTertType()==Te_Frame);
  Assert(lf->getType()==Co_Lock);
  LockSec* sec=lf->getSec();
  if(!sec->secForward(toS)) return;
  be->getOneMsgCredit();
  lockSendToken(mS,mI,toS);}

/**********************************************************************/
/*   SECTION 34:: Lock protocol - send                                */
/**********************************************************************/

void lockSendToken(Site *mS,int mI,Site* toS){
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_LOCK_TOKEN(bs,mS,mI);
  SendTo(toS,bs,M_LOCK_TOKEN,mS,mI);}

/**********************************************************************/
/*   SECTION 35:: Lock protocol - basics                             */
/**********************************************************************/

void LockProxy::lock(Thread *t){
  PD((LOCK,"convertToFrame %s-%d",BTOS(getIndex())->stringrep(),BTOI(getIndex())));
  convertLockProxyToFrame(this);
  ((LockFrame*)this)->lock(t);}

void secLockToNext(LockSec* sec,Tertiary* t,Site* toS){
  int index=t->getIndex();
  if(t->getTertType()==Te_Frame){
    BorrowEntry *be=BT->getBorrow(index);
    be->getOneMsgCredit();
    NetAddress *na=be->getNetAddress();
    lockSendToken(na->site,na->index,toS);
    return;}
  Assert(t->getTertType()==Te_Manager);
  OwnerEntry *oe=OT->getOwner(index);
  oe->getOneCreditOwner();
  lockSendToken(mySite,index,toS);}

void secLockGet(LockSec* sec,Tertiary* t,Thread* th){
  int index=t->getIndex();
  sec->makeRequested();
  if(t->getTertType()==Te_Frame){
    BorrowEntry *be=BT->getBorrow(index);
    be->getOneMsgCredit();
    cellLockSendGet(be);
    return;}
  Assert(t->getTertType()==Te_Manager);
  OwnerEntry *oe=OT->getOwner(index);
  Chain* ch=((LockManager*) t)->getChain();
  Site* current=ch->setCurrent(mySite);
  oe->getOneCreditOwner();
  cellLockSendForward(current,mySite,index);
  return;}

void LockSec::lockComplex(Thread *th,Tertiary* t){
  PD((LOCK,"lockComplex in state:%d",state));
  switch(state){
  case Cell_Lock_Valid|Cell_Lock_Next:{
    Assert(getLocker()!=th);
    Assert(getLocker()!=NULL);
    if(pending==NULL){
      pendThreadAddToEnd(getPendBase(),MoveThread);}}
  case Cell_Lock_Valid:{
    Assert(getLocker()!=th);
    Assert(getLocker()!=NULL);
    pendThreadAddToEnd(getPendBase(),th);
    if(t->errorIgnore()) return;
    break;}
  case Cell_Lock_Next|Cell_Lock_Requested:
  case Cell_Lock_Requested:{
    pendThreadAddToEnd(getPendBase(),th);
    if(t->errorIgnore()) return;
    break;}
  case Cell_Lock_Invalid:{
    pendThreadAddToEnd(getPendBase(),th);
    secLockGet(this,t,th);
    if(t->errorIgnore()) return;
    break;}
  default: Assert(0);}
  if(maybeInvokeHandler(t,th)){// ERROR-HOOK
    genInvokeHandlerLockOrCell(t,th);
  }}

void LockLocal::unlockComplex(){
  setLocker(pendThreadResumeFirst(&pending));
  return;}

void LockLocal::lockComplex(Thread *t){
  pendThreadAddToEnd(getPendBase(),t);}

void LockSec::unlockPending(Thread *t){
  PendThread **pt=&pending;
  while((*pt)->thread!=t) {
    pt=&((*pt)->next);}
  *pt=(*pt)->next;}

void LockSec::unlockComplex(Tertiary* tert){
  PD((LOCK,"unlock complex in state:%d",getState()));
  Assert(getState() & Cell_Lock_Valid);
  if(getState() & Cell_Lock_Next){
    Assert(getState()==(Cell_Lock_Next | Cell_Lock_Valid));
    if(pending==NULL){
      secLockToNext(this,tert,next);
      state=Cell_Lock_Invalid;
      return;}
    Thread *th=pending->thread;
    if(th==DummyThread){
      Assert(tert->getTertType()==Te_Manager);
      pendThreadRemoveFirst(getPendBase());
      unlockComplex(tert);
      return;}
    if(th==MoveThread){
      pendThreadRemoveFirst(getPendBase());
      secLockToNext(this,tert,next);
      state=Cell_Lock_Invalid;
      if(pending==NULL) return;
      secLockGet(this,tert,NULL);
      return;}
    locker=pendThreadResumeFirst(getPendBase());
    return;}
  if(pending!=NULL){
    locker=pendThreadResumeFirst(getPendBase());
    return;}
  return;}

/**********************************************************************/
/*   SECTION 36:: error msgs                                         */
/**********************************************************************/

Bool CellSec::cellRecovery(TaggedRef tr){
  if(state==Cell_Lock_Invalid){
    state=Cell_Lock_Valid;
    contents=tr;
    return NO;}
  Assert(state==Cell_Lock_Requested);
  return OK;}

Bool LockSec::lockRecovery(){
  if(state==Cell_Lock_Invalid){
    state=Cell_Lock_Valid;
    locker=NULL;
    return NO;}
  state &= ~Cell_Lock_Next;
  Assert(state==Cell_Lock_Requested);
  return OK;}

void cellManagerIsDown(TaggedRef tr,Site* mS,int mI){
  NetAddress na=NetAddress(mS,mI);
  BorrowEntry *be=BT->find(&na);
  if(be==NULL){return;}
  Tertiary* t=be->getTertiary();
  maybeConvertCellProxyToFrame(t);
  if(((CellFrame*)t)->getSec()->cellRecovery(tr)){
    cellReceiveContentsFrame(be,tr,mS,mI);}}

void lockManagerIsDown(Site* mS,int mI){
  NetAddress na=NetAddress(mS,mI);
  BorrowEntry *be=BT->find(&na);
  if(be==NULL) {return;} // has been gced
  Tertiary* t=be->getTertiary();
  maybeConvertLockProxyToFrame(t);
  if(((LockFrame*)t)->getSec()->lockRecovery()){
    lockReceiveTokenFrame(be,mS,mI);}}

void cellSendCantPut(TaggedRef tr,Site* toS, Site *mS, int mI){
  PD((ERROR_DET,"Proxy cant put to %s site: %s:%d",toS->stringrep(),mS->stringrep(),mI));
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(mS);
  marshal_M_CELL_CANTPUT(bs,mI, toS, tr);
  SendTo(mS,bs,M_CELL_CANTPUT,mS,mI);}

void cellSendContentsFailure(TaggedRef tr,Site* toS,Site *mS, int mI){
  if(toS==mS) {// ManagerSite is down
    cellManagerIsDown(tr,toS,mI);
    return;}
  if(mS==mySite){// At managerSite
    cellReceiveCantPut(OT->getOwner(mI),tr,mI,mS,toS);
    return;}
  cellSendCantPut(tr,toS,mS,mI);
  return;}

void lockSendCantPut(Site* toS, Site *mS, int mI){
  PD((ERROR_DET,"Proxy cant put - to %s site: %s:%d Nr %d",toS->stringrep(),mS->stringrep(),mI));
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(mS);
  marshal_M_LOCK_CANTPUT(bs,mI, toS);
  SendTo(mS,bs,M_LOCK_CANTPUT,mS,mI);
  return;}

void lockSendTokenFailure(Site* toS,Site *mS, int mI){
  PD((ERROR_DET,"LockTokenFailure"));
  if(toS==mS) {// ManagerSite is down
    lockManagerIsDown(mS,mI);
    return;}
  if(mS==mySite){// At managerSite
    lockReceiveCantPut(OT->getOwner(mI),mI,mS,toS);
    return;}
  lockSendCantPut(toS,mS,mI);
  return;}

void lockReceiveCantPut(OwnerEntry *oe,int mI,Site* rsite, Site* bad){
  LockManager* lm=(LockManager*)oe->getTertiary();
  Assert(lm->getType()==Co_Lock);
  Assert(lm->getTertType()==Te_Manager);
  PD((ERROR_DET,"Proxy cant Put"));
  Chain *ch=lm->getChain();
  ch->removeBefore(bad);
  ch->shortcutCrashLock(lm);
  //DebugCode(printChain(ch);)
}

void cellReceiveCantPut(OwnerEntry* oe,TaggedRef val,int mI,Site* rsite, Site* badS){
  CellManager* cm=(CellManager*)oe->getTertiary();
  Assert(cm->getType()==Co_Cell);
  Assert(cm->getTertType()==Te_Manager);
  PD((ERROR_DET,"Proxy cant Put"));
  Chain *ch=cm->getChain();
  ch->removeBefore(badS);
  ch->shortcutCrashCell(cm,val);
  //DebugCode(printChain(ch);)
}

void sendAskError(Tertiary *t,EntityCond ec){ // caused by installing handler/watcher
  BorrowEntry *be=BT->getBorrow(t->getIndex());
  NetAddress* na=be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  be->getOneMsgCredit();
  marshal_M_ASK_ERROR(bs,t->getIndex(),mySite,ec);
  SendTo(na->site,bs,M_ASK_ERROR,na->site,na->index);}

void sendUnAskError(Tertiary *t,EntityCond ec){ // caused by deinstalling handler/watcher
  BorrowEntry *be=BT->getBorrow(t->getIndex());
  NetAddress* na=be->getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  be->getOneMsgCredit();
  marshal_M_UNASK_ERROR(bs,na->index,mySite,ec);
  SendTo(na->site,bs,M_UNASK_ERROR,na->site,na->index);}

                                   // caused by installing remote watcher/handler
void receiveAskError(OwnerEntry *oe,Site *toS,EntityCond ec){
  Tertiary *t=oe->getTertiary();
  switch(t->getType()){
  case Co_Cell: break;
  case Co_Lock: break;
  default: NOT_IMPLEMENTED;}
  Chain *ch=getChainFromTertiary(t);
  if(ch->hasFlag(TOKEN_LOST)){
    EntityCond tmp=ec & (PERM_SOME|PERM_ME);
    if(tmp != ENTITY_NORMAL){
      sendTellError(oe,toS,t->getIndex(),tmp,true);
      return;}
    ch->newInform(toS,ec);
    ch->dealWithTokenLostBySite(oe,t->getIndex(),toS);
    return;}
  Assert(!(ec & TEMP_ALL));
  if((ch->hasFlag(TOKEN_PERM_SOME)) && (ec & PERM_SOME)){
    sendTellError(oe,toS,t->getIndex(),PERM_SOME,TRUE);
    return;}
  ch->newInform(toS,ec);
  if(someTempCondition(ec)){
    if(!ch->hasFlag(INTERESTED_IN_OK)){
      ch->setFlagAndCheck(INTERESTED_IN_TEMP);
      ch->probeTemp();
      return;}
    ChainElem *ce=ch->getFirstNonGhost();
    while(ce!=NULL){
      if(ce->getSite()->siteStatus()==SITE_TEMP) break;
      ce=ce->getNext();}
    if(ce!=NULL){
      ch->managerSeesSiteTemp(t,ce->getSite());}}}

void Chain::receiveUnAsk(Site* s,EntityCond ec){
  InformElem **ie=&inform;
  InformElem *tmp;
  while(*ie!=NULL){
    if(((*ie)->site==s) && ((*ie)->watchcond==ec)){
      tmp=*ie;
      *ie=tmp->next;
      releaseInformElem(tmp);}
    ie=&((*ie)->next);}
  PD((WEIRD,"unaskerror with no error"));
  return;}

void receiveUnAskError(OwnerEntry *oe,Site *toS,EntityCond ec){
  Tertiary* t=oe->getTertiary();
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock: break;
  default: NOT_IMPLEMENTED;}
  getChainFromTertiary(t)->receiveUnAsk(toS,ec);}

/**********************************************************************/
/*   SECTION 37:: handlers/watchers                                   */
/**********************************************************************/

void Tertiary::restop(){
  Thread *th=am.currentThread();
  if(!maybeInvokeHandler(this,th)) return;
  genInvokeHandlerLockOrCell(this,th);}

Watcher** Tertiary::findWatcherBase(Thread* th,EntityCond ec){
  Watcher** base=getWatcherBase();
  while(*base!=NULL){
    if(((*base)->isHandler()) && ((*base)->thread==th) &&
      ((*base)->isTriggered(ec))) return base;
    base= &((*base)->next);}
  return NULL;}

Bool Tertiary::handlerExists(Thread *t){
  if(info==NULL) return NO;
  Watcher *w=info->watchers;
  while(w!=NULL){
    if(w->isHandler() && w->getThread()==t) {return OK;}
    w=w->next;}
  return NO;}

void Tertiary::insertWatcher(Watcher *w){
  if(info==NULL){
    info=new EntityInfo(w);
    return;}
  w->next=info->watchers;
  info->watchers=w;}

inline Site* getSiteFromTertiaryProxy(Tertiary* t){
  BorrowEntry *be=BT->getBorrow(t->getIndex());
  Assert(be!=NULL);
  return be->getNetAddress()->site;}

void informInstallHandler(Tertiary* t,EntityCond ec){
  switch(t->getType()){
  case Co_Cell:
  case Co_Lock: break;
  default: NOT_IMPLEMENTED;}
  if(t->getTertType()==Te_Manager){
    Chain *ch=getChainFromTertiary(t);
    ch->newInform(mySite,ec);
    return;}
  sendAskError(t,managerPart(ec));
  if(someTempCondition(ec))
    getSiteFromTertiaryProxy(t)->installProbe(PROBE_TYPE_ALL,0);
  else
    getSiteFromTertiaryProxy(t)->installProbe(PROBE_TYPE_PERM,0);}

Bool Tertiary::installHandler(EntityCond wc,TaggedRef proc,Thread* th){
  if(handlerExists(th)){return FALSE;} // duplicate
  PD((NET_HANDLER,"Handler installed on tertiary %x",this));
  Watcher *w=new Watcher(proc,th,wc);
  insertWatcher(w);
  if(this->getTertType() == Te_Local){
    return TRUE;}
  if(wc & TEMP_BLOCKED){
    Assert(wc==TEMP_BLOCKED|PERM_BLOCKED);
    informInstallHandler(this,wc);
    return TRUE;}
  if(getTertType()==Te_Manager) return TRUE;
  getSiteFromTertiaryProxy(this)->installProbe(PROBE_TYPE_PERM,0);
  return TRUE;}


Bool Tertiary::deinstallHandler(Thread *th){
  if(!handlerExists(th)){return FALSE;} // nonexistent
  PD((NET_HANDLER,"Handler deinstalled on tertiary %x",this));
  NOT_IMPLEMENTED;
  return NO;}



void Tertiary::installWatcher(EntityCond wc,TaggedRef proc){
  PD((NET_HANDLER,"Watcher installed on tertiary %x",this));
  Watcher *w=new Watcher(proc,wc);
  insertWatcher(w);
   if(getTertType() == Te_Local){
    return;}
  if(managerPart(wc) != ENTITY_NORMAL){
    informInstallHandler(this,managerPart(wc));
    return;}
  if(this->getTertType() == Te_Manager){
    return;}
  if(someTempCondition(wc))
    getSiteFromTertiaryProxy(this)->installProbe(PROBE_TYPE_ALL,0);
  else
    getSiteFromTertiaryProxy(this)->installProbe(PROBE_TYPE_PERM,0);}

Bool Tertiary::deinstallWatcher(EntityCond wc, TaggedRef proc){
  Watcher** base=getWatcherBase();
  while(*base!=NULL){
    if((!((*base)->isHandler())) &&
       ((*base)->proc==proc) &&
       ((*base)->getWatchCond() == wc)){
      *base = (*base)->next;
      return OK;}
    base= &((*base)->next);}
  return NO;}

void Watcher::invokeHandler(EntityCond ec,Tertiary* entity){
  Assert(isHandler());
  RefsArray argsX = allocateRefsArray(1, NO);
  argsX[0]= makeTaggedTert(entity);
  thread->pushCall(BI_restop,argsX,1);
  RefsArray args = allocateRefsArray(2, NO);
  args[0]= makeTaggedTert(entity);
  args[1]= (ec & PERM_BLOCKED)?AtomPermBlocked:AtomTempBlocked;
  thread->pushCall(proc,args,2);
  oz_resumeFromNet(thread);}

void Watcher::invokeWatcher(EntityCond ec,Tertiary* entity){
  Assert(!isHandler());
  Thread *tt = am.mkRunnableThread(DEFAULT_PRIORITY, ozx_rootBoard());
  RefsArray args = allocateRefsArray(2,NO);
  args[0]=makeTaggedTert(entity);
  args[1]=listifyWatcherCond(ec);
  tt->pushCall(proc, args, 2);
  am.scheduleThread(tt);}

Bool CellSec::threadIsPending(Thread *t){
  return basicThreadIsPending(pending,t);}

Bool LockSec::threadIsPending(Thread *t){
  return basicThreadIsPending(pending,t);}

Bool Tertiary::threadIsPending(Thread* th){
  switch(getType()){
  case Co_Cell: {
    if(getTertType()==Te_Proxy) {return NO;}
    if(getTertType()==Te_Frame){
      return ((CellFrame*)this)->getSec()->threadIsPending(th);}
    Assert(getTertType()==Te_Manager);
    return ((CellManager*)this)->getSec()->threadIsPending(th);}
  case Co_Lock:{
  if(getTertType()==Te_Proxy) {return NO;}
    if(getTertType()==Te_Frame){
      return ((LockFrame*)this)->getSec()->threadIsPending(th);}
    Assert(getTertType()==Te_Manager);
  return ((LockManager*)this)->getSec()->threadIsPending(th);}
  default:
    NOT_IMPLEMENTED;}
  return NO;}

/**********************************************************************/
/*   SECTION 38:: error                                               */
/**********************************************************************/

void sendTellError(OwnerEntry *oe,Site* toS,int mI,EntityCond ec,Bool set){
  if(toS==mySite){
    receiveTellError(oe->getTertiary(),mySite,mI,ec,set);
    return;}
  if(SEND_SHORT(toS)) {return;}
  oe->getOneCreditOwner();
  MsgBuffer* bs=msgBufferManager->getMsgBuffer(toS);
  marshal_M_TELL_ERROR(bs,mySite,mI,ec,set);
  SendTo(toS,bs,M_TELL_ERROR,mySite,mI);}

void receiveTellError(Tertiary *t,Site* mS,int mI,EntityCond ec,Bool set){

  if(set){
    if(t->setEntityCondManager(ec)){
      t->entityProblem();}
    return;}
  t->resetEntityCondManager(ec);}

void Tertiary::releaseWatcher(Watcher* w){
  if(getTertType()!=Te_Manager){
    EntityCond ec=managerPart(w->watchcond);
    switch(getType()){
    case Co_Cell:
    case Co_Lock: {
      ec &= (~PERM_BLOCKED|PERM_SOME|PERM_ME); // Automatic
      break;}
    default: NOT_IMPLEMENTED;}
    if(ec!=ENTITY_NORMAL) {
      sendUnAskError(this,managerPart(w->watchcond));}}}

void Tertiary::entityProblem(){
  PD((ERROR_DET,"entityProblem invoked"));
  if(errorIgnore()) return;
  Watcher** base=getWatcherBase();
  if(*base==NULL) return;
  EntityCond ec=getEntityCond();
  Watcher* w=*base;
  while(w!=NULL){
    if((!w->isTriggered(ec)) ||
       ((w->isHandler()) && (!threadIsPending(w->getThread())))){
        base= &(w->next);
        w=*base;}
    else{
      if(w->isHandler()){
        w->invokeHandler(ec,this);}
      else{
        w->invokeWatcher(ec,this);}
      releaseWatcher(w);
      *base=w->next;
      w=*base;}}}

void Chain::informHandle(OwnerEntry* oe,int OTI,EntityCond ec){
  Assert(somePermCondition(ec));
  InformElem **base=&inform;
  InformElem *cur=*base;
  while(cur!=NULL){
    if(cur->watchcond & ec){
      sendTellError(oe,cur->site,OTI,cur->watchcond & ec,TRUE);
      *base=cur->next;
      freeInformElem(cur);
      cur=*base;
      continue;}
    base=&(cur->next);
    cur=*base;}}

void Chain::dealWithTokenLostBySite(OwnerEntry*oe,int OTI,Site *s){ // ATTENTION
  InformElem **base=&inform;
  InformElem *cur=*base;
  while(cur!=NULL){
    if((cur->site==s) & (cur->watchcond & PERM_BLOCKED)){
      Assert((cur->watchcond == PERM_BLOCKED) || (cur->watchcond == TEMP_BLOCKED|PERM_BLOCKED));
      sendTellError(oe,cur->site,OTI,PERM_BLOCKED,TRUE);
      *base=cur->next;
      freeInformElem(cur);
      cur=*base;}
    else{
      base=&(cur->next);
      cur=*base;}}}

void Chain::shortcutCrashLock(LockManager* lm){
  setFlag(TOKEN_PERM_SOME);
  int OTI=lm->getIndex();
  informHandle(OT->getOwner(OTI),OTI,PERM_SOME);
  lm->setEntityCondManager(PERM_SOME);
  lm->entityProblem();
  ChainElem** base=getFirstNonGhostBase();
  ChainElem *ce;
  LockSec* sec=lm->getSec();
  if((*base)->next==NULL){
    LockSec *sec=lm->getSec();
    ChainElem *ce=*base;
    ce->init(mySite);
    Assert(sec->state==Cell_Lock_Invalid);
    sec->state=Cell_Lock_Valid;
    return;}
  removePerm(base);
  ce=getFirstNonGhost();
  if(ce->site==mySite){
    lockReceiveTokenManager(OT->getOwner(OTI),OTI);
    return;}
  lockSendToken(mySite,OTI,ce->site);}

void Chain::shortcutCrashCell(CellManager* cm,TaggedRef val){
  setFlag(TOKEN_PERM_SOME);
  int OTI=cm->getIndex();
  informHandle(OT->getOwner(OTI),OTI,PERM_SOME);
  cm->setEntityCondManager(PERM_SOME);
  cm->entityProblem();
  ChainElem** base=getFirstNonGhostBase();
  ChainElem *ce;
  CellSec* sec=cm->getSec();
  if((*base)->next==NULL){
    CellSec *sec=cm->getSec();
    ChainElem *ce=*base;
    ce->init(mySite);
    Assert(sec->state=Cell_Lock_Invalid);
    sec->state=Cell_Lock_Valid;
    sec->contents=val;
    return;}
  removePerm(base);
  ce=getFirstNonGhost();
  int index=cm->getIndex();
  if(ce->site==mySite){
    cellReceiveContentsManager(OT->getOwner(index),val,index);
    return;}
  OT->getOwner(index)->getOneCreditOwner();
  cellSendContents(val,ce->site,mySite,index);}

void Chain::handleTokenLost(OwnerEntry *oe,int OTI){
  ChainElem *ce=first->next;
  ChainElem *back;
  Assert(first->site->siteStatus()==SITE_PERM);
  releaseChainElem(first);
  while(ce){
    if(!ce->flagIsSet(CHAIN_GHOST)){
      sendTellError(oe,ce->site,OTI,PERM_SOME|PERM_BLOCKED|PERM_ME,TRUE);}
    back=ce;
    ce=ce->next;
    releaseChainElem(back);}
  first=NULL;
  last=NULL;}

void Chain::managerSeesSitePerm(Tertiary *t,Site *s){
  PD((ERROR_DET,"managerSeesSitePerm site:%s nr:%d",s->stringrep(),t->getIndex()));
  //DebugCode(printChain(this);)
  removeGhost(s); // remove ghost if any
  if(!siteExists(s)) return;
  ChainElem **base=getFirstNonGhostBase();
  ChainElem *after,*dead,*before;
  if((*base)->site==s){
    PD((ERROR_DET,"managerSeesSitePerm - perm is first site"));
    dead=*base;
    after=dead->next;
    before=NULL;}
  else{
    PD((ERROR_DET,"managerSeesSitePerm - perm is not first site"));
    while((*base)->next->site!=s){base=&((*base)->next);}
    before=*base;
    dead=before->next;
    after=dead->next;}
  if(before==NULL){
    dead->setFlag(CHAIN_PAST);}
  else{
    if(before->site->siteStatus()==SITE_PERM){
      if(dead->flagIsSet(CHAIN_BEFORE)){
        before->setFlagAndCheck(CHAIN_BEFORE);}
      removePerm(&(before->next));
      managerSeesSitePerm(t,before->site);
      return;}}
  if(after==NULL){
    PD((ERROR_DET,"managerSeesSitePerm - perm is last site"));
    dead->setFlag(CHAIN_BEFORE);}
  else{
    PD((ERROR_DET,"managerSeesSitePerm - perm is not last site"));
    if(after->site->siteStatus()==SITE_PERM){
      removePerm(&(dead->next));
      managerSeesSitePerm(t,s);
      return;}}
  if(dead->flagIsSet(CHAIN_CANT_PUT)) return;
  if(!dead->flagIsSet(CHAIN_PAST)){
    maybeChainSendQuestion(before,t,s);
    return;}
  if(!dead->flagIsSet(CHAIN_BEFORE)){
    maybeChainSendQuestion(after,t,s);
    return;}
  PD((ERROR_DET,"managerSeesSitePerm - token lost (lock can recover"));
  if(before!=NULL) {
    removeBefore(dead->site);}
  if(t->getType()==Co_Lock){
    PD((ERROR_DET,"LockToken lost, now recreated"));
    ((LockManager*)t)->getChain()->shortcutCrashLock((LockManager*) t);
    return;}
  PD((ERROR_DET,"Token lost"));
  setFlagAndCheck(TOKEN_LOST);
  int OTI=t->getIndex();
  OwnerEntry *oe=OT->getOwner(OTI);
  informHandle(oe,OTI,PERM_SOME|PERM_ME);
  t->setEntityCondManager(PERM_SOME|PERM_ME);
  t->entityProblem();
  handleTokenLost(oe,OTI);
  return;}

void Chain::managerSeesSiteTemp(Tertiary *t,Site *s){
  Assert(siteExists(s));
  Assert(hasFlag(INTERESTED_IN_TEMP));
  EntityCond ec;
  Bool change=NO;
  PD((ERROR_DET,"managerSeesSiteTemp site:%s nr:%d",s->stringrep(),t->getIndex()));
  int index;
  OwnerEntry *oe;
  InformElem *cur=inform;  // deal with TEMP_SOME|TEMP_ME watchers
  while(cur!=NULL){
    ec=cur->wouldTrigger(TEMP_SOME|TEMP_ME);
    if(ec != ENTITY_NORMAL){
      change=OK;
      index=t->getIndex();
      sendTellError(OT->getOwner(index),cur->site,index,ec,TRUE);}
    cur=cur->next;}
  ChainElem *ce=findAfter(s); // deal with TEMP_BLOCKED handlers
  while(ce!=NULL){
    cur=inform;
    while(cur!=NULL){
      if(cur->site==s){
        ec=cur->wouldTrigger(TEMP_BLOCKED);
        if(ec!= ENTITY_NORMAL){
          change=OK;
          index=t->getIndex();
          sendTellError(OT->getOwner(index),cur->site,index,ec,TRUE);}}
      cur=cur->next;}
    ce=ce->next;}
  if(change){
    setFlag(INTERESTED_IN_OK);}}

void Chain::managerSeesSiteOK(Tertiary *t,Site *s){
  Assert(siteExists(s));
  Assert(hasFlag(INTERESTED_IN_OK));
  PD((ERROR_DET,"managerSeesSiteOK site:%s nr:%d",s->stringrep(),t->getIndex()));
  int index;
  Bool change=NO;
  OwnerEntry *oe;
  EntityCond ec;
  InformElem *cur=inform;  // deal with TEMP_SOME|TEMP_ME watchers
  while(cur!=NULL){
    ec=cur->wouldTriggerOK(TEMP_SOME|TEMP_ME);
    if(ec!=ENTITY_NORMAL){
      change=OK;
      index=t->getIndex();
      sendTellError(OT->getOwner(index),cur->site,index,ec,FALSE);}
    cur=cur->next;}

  ChainElem *ce=findAfter(s); // deal with TEMP_BLOCKED handlers
  while(ce!=NULL){
    cur=inform;
    while(cur!=NULL){
      if(cur->site==s){
        ec=cur->wouldTriggerOK(TEMP_BLOCKED);
        if(ec!= ENTITY_NORMAL){
          change=OK;
          index=t->getIndex();
          sendTellError(OT->getOwner(index),cur->site,index,ec,FALSE);}}
      cur=cur->next;}
    ce=ce->next;}

  if((change) && (!tempConnectionInChain())){
    resetFlagAndCheck(INTERESTED_IN_OK);
    deProbeTemp();}}

/**********************************************************************/
/*   SECTION 39:: probes                                            */
/**********************************************************************/

void cellLock_Perm(int state,Tertiary* t){
  switch(state){
  case Cell_Lock_Invalid:{
    if(t->setEntityCondOwn(PERM_SOME|PERM_ME)) break;
    return;}
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:{
    if(t->setEntityCondOwn(PERM_SOME|PERM_BLOCKED|PERM_ME)) break;
               // ATTENTION note that we don't know if we'll be blocked maybe TEMP?
    return;}
  case Cell_Lock_Valid|Cell_Lock_Next:
  case Cell_Lock_Valid:{
    if(t->setEntityCondOwn(PERM_ALL|PERM_SOME)) break;
    return;}
  default: {
    Assert(0);}}
  t->entityProblem();}

void cellLock_Temp(int state,Tertiary* t){
  switch(state){
  case Cell_Lock_Invalid:{
    if(t->setEntityCondOwn(TEMP_SOME|TEMP_ME)) break;
    return;}
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:{
    if(t->setEntityCondOwn(TEMP_SOME|TEMP_ME|TEMP_BLOCKED)) break;
                   // ATTENTION: note that we don't know if we'll be blocked
    return;}
  case Cell_Lock_Valid|Cell_Lock_Next:
  case Cell_Lock_Valid:{
    if(t->setEntityCondOwn(TEMP_SOME|TEMP_ALL)) break;
    return;}
  default: {
    Assert(0);}}
  t->entityProblem();}

void cellLock_OK(int state,Tertiary* t){
  switch(state){
  case Cell_Lock_Invalid:{
    if(t->resetEntityCondProxy(TEMP_SOME|TEMP_ME)) break;
    return;}
  case Cell_Lock_Requested|Cell_Lock_Next:
  case Cell_Lock_Requested:{
    if(t->resetEntityCondProxy(TEMP_SOME|TEMP_ME|TEMP_BLOCKED)) break;
    return;}
  case Cell_Lock_Valid|Cell_Lock_Next:
  case Cell_Lock_Valid:{
    if(t->resetEntityCondProxy(TEMP_SOME|TEMP_ALL)) break;
    return;}
  default: {
    Assert(0);}}}

void Tertiary::managerProbeFault(Site *s, int pr){
  PD((ERROR_DET,"Mgr probe invoked %d",pr));
  switch(getType()){
  case Co_Cell:
  case Co_Lock:{
    Chain *ch=getChainFromTertiary(this);
    if(pr==PROBE_OK){
      if(!ch->hasFlag(INTERESTED_IN_OK)) return;
      if(!ch->siteOfInterest(s)) return;
      ch->managerSeesSiteOK(this,s);
      return;}
    if(pr==PROBE_TEMP){
      if(!ch->hasFlag(INTERESTED_IN_TEMP)) return;
      if(!ch->siteOfInterest(s)) return;
      ch->managerSeesSiteTemp(this,s);
      return;}
    if(ch->hasInform()){
      ch->removeInformOnPerm(s);}
    if(!ch->siteOfInterest(s)) return;
    ch->managerSeesSitePerm(this,s);}
  default: return;  // TO_BE_IMPLEMENTED
    return;}}

void Tertiary::proxyProbeFault(int pr){
  PD((ERROR_DET,"proxy probe invoked %d",pr));
  switch(getType()){
  case Co_Cell:
  case Co_Lock:{
    int state;
    if(getTertType()==Te_Proxy){
      state=Cell_Lock_Invalid;}
    else{
      state=getStateFromLockOrCell(this);}
    if(pr==PROBE_PERM){
      cellLock_Perm(state,this);
      return;}
    if(pr == PROBE_OK){
      cellLock_OK(state,this);
      return;}
    Assert(pr==PROBE_TEMP);
    cellLock_Temp(state,this);
    return;}
  default: return;}}     // TO_BE_IMPLEMENTED

void Site::probeFault(ProbeReturn pr){
  PD((PROBES,"PROBEfAULT  site:%s",stringrep()));
  int limit=OT->getSize();
  for(int ctr = 0; ctr<limit;ctr++){
    OwnerEntry *oe = OT->getEntry(ctr);
    if(oe==NULL){continue;}
    Assert(oe!=NULL);
    if(oe->isTertiary()){
      Tertiary *tr=oe->getTertiary();
      PD((PROBES,"Informing Manager"));
      Assert(tr->getTertType()==Te_Manager);
      tr->managerProbeFault(this,pr);}} // TO_BE_IMPLEMENTED vars
  limit=BT->getSize();
  for(int ctr1 = 0; ctr1<limit;ctr1++){
    BorrowEntry *be = BT->getEntry(ctr1);
    if(be==NULL){continue;}
    Assert(be!=NULL);
    if(be->isTertiary()){
      Tertiary *tr=be->getTertiary();
      if(tr->hasWatchers()){
        tr->proxyProbeFault(pr);}}}
  return;}

/**********************************************************************/
/*   SECTION 40:: communication problem                               */
/**********************************************************************/

inline void returnSendCredit(Site* s,int OTI){
  if(s==mySite){
    OT->getOwner(OTI)->receiveCredit(OTI);
    return;}
  sendCreditBack(s,OTI,1);}

enum CommCase{
    USUAL_OWNER_CASE,
    USUAL_BORROW_CASE
  };

#define ResetCP(buf,mt) {\
  buf->unmarshalReset();\
  MessageType mt1=unmarshalHeader(buf);\
  Assert(mt1==mt);}

void Site::communicationProblem(MessageType mt,Site*
                                storeSite,int storeIndex
                                ,FaultCode fc,FaultInfo fi){
  int OTI,Index;
  Site *s1,*s2;
  TaggedRef tr;
  CommCase flag;

  PD((SITE,"CommProb type:%d site:%s\n storeSite: %s \n indx:%d faultCode:%d",
      mt,this->stringrep(),storeSite->stringrep(), storeIndex, fc));
  switch(mt){

  case M_PORT_SEND:{
    flag=USUAL_BORROW_CASE;
    break;}

  case M_REMOTE_SEND:{
    NOT_IMPLEMENTED;}

  case M_ASK_FOR_CREDIT:{
    flag=USUAL_BORROW_CASE;
    break;}

  case M_OWNER_CREDIT:{
      flag=USUAL_BORROW_CASE;
      break;}

  case M_OWNER_SEC_CREDIT:{
    flag=USUAL_BORROW_CASE;
    break;}

  case M_BORROW_CREDIT:{
    flag=USUAL_OWNER_CASE;
    break;}

    case M_REGISTER:{
      flag=USUAL_BORROW_CASE;
      break;}

    case M_REDIRECT:{
      if(fc==COMM_FAULT_PERM_NOT_SENT){
        ResetCP(((MsgBuffer*)fi),M_REDIRECT);
        unmarshal_M_REDIRECT((MsgBuffer*)fi,s1,OTI,tr);
        returnSendCredit(s1,OTI);
        return;}
      flag=USUAL_OWNER_CASE;
      break;}

    case M_ACKNOWLEDGE:{
      flag=USUAL_OWNER_CASE;
      break;}

    case M_SURRENDER:{
      if(fc==COMM_FAULT_PERM_NOT_SENT){
        ResetCP(((MsgBuffer*)fi),M_SURRENDER);
        unmarshal_M_SURRENDER((MsgBuffer*)fi,OTI,s1,tr);
        returnSendCredit(mySite,OTI);
        return;}
      flag=USUAL_OWNER_CASE;
      break;}

    case M_CELL_LOCK_GET:{
      flag=USUAL_BORROW_CASE;
      break;}

    case  M_CELL_LOCK_FORWARD:{
      flag=USUAL_OWNER_CASE;
      break;}

    case M_CELL_LOCK_DUMP:{
      flag=USUAL_BORROW_CASE;
      break;}

    case M_CELL_CONTENTS:{
      if(fc == COMM_FAULT_PERM_NOT_SENT){
        ResetCP(((MsgBuffer*)fi),M_CELL_CONTENTS);
        unmarshal_M_CELL_CONTENTS((MsgBuffer*)fi,s1,OTI,tr);
        Assert(s1==storeSite);
        Assert(OTI=storeIndex);
        returnSendCredit(s1,OTI);
        cellSendContentsFailure(tr,this,storeSite,OTI);
        return;}
      /*
        if(fc==COMM_FAULT_PERM_MAYBE_SENT){
        NOT_IMPLEMENTED;}
        */
      return;}

    case M_CELL_READ:{
      flag=USUAL_BORROW_CASE;
      break;}

    case M_CELL_REMOTEREAD:{
      NOT_IMPLEMENTED;}

    case M_CELL_READANS:{
      NOT_IMPLEMENTED;}

    case M_CELL_CANTPUT:{
      NOT_IMPLEMENTED;}

    case M_LOCK_TOKEN:{
      if(fc == COMM_FAULT_PERM_NOT_SENT){
        ResetCP(((MsgBuffer*)fi),M_LOCK_TOKEN);
        unmarshal_M_LOCK_TOKEN((MsgBuffer*)fi,s1,OTI);
        Assert(s1==storeSite);
        Assert(OTI=storeIndex);
        returnSendCredit(s1,OTI);
        lockSendTokenFailure(this,storeSite,OTI);
        return;}
      return;}

    case M_LOCK_CANTPUT:{
      return;}

    case M_FILE:{
      Assert(0);
      warning("impossible\n");
      return;}

    case M_CHAIN_ACK:{
      flag=USUAL_BORROW_CASE;
      break;}


    case M_CHAIN_QUESTION:{
      flag=USUAL_OWNER_CASE;
      break;}

    case M_CHAIN_ANSWER:{
      flag=USUAL_BORROW_CASE;
      break;}

    case M_ASK_ERROR:{
      flag=USUAL_OWNER_CASE;
      break;}

    case M_UNASK_ERROR:{
      flag=USUAL_OWNER_CASE;
      break;}

    case M_TELL_ERROR:{
      flag=USUAL_BORROW_CASE;
      break;}

    case M_GET_OBJECT:{
      NOT_IMPLEMENTED;}

    case M_GET_OBJECTANDCLASS:{
      NOT_IMPLEMENTED;}

    case M_SEND_OBJECT:{
      NOT_IMPLEMENTED;}

    case M_SEND_OBJECTANDCLASS:{
      NOT_IMPLEMENTED;}

    case M_SEND_GATE:{
      NOT_IMPLEMENTED;}

    default:{
      warning("communication problem - impossible");
      Assert(0);}
    }

  switch(flag){
  case USUAL_OWNER_CASE:{
    switch(fc){
    case COMM_FAULT_TEMP_NOT_SENT:
    case COMM_FAULT_TEMP_MAYBE_SENT: {
      PD((SITE,"Owner:CommProb temp ignored"));
      return;}
    case COMM_FAULT_PERM_NOT_SENT:{
      PD((SITE,"Owner:CommProb perm not sent extract send credit and ignore"));
      returnSendCredit(storeSite,storeIndex);
      return;}
    case COMM_FAULT_PERM_MAYBE_SENT:{
      PD((SITE,"Owner:CommProb perm maybe sent lose send credit and ignore"));
      return;}}}
  case USUAL_BORROW_CASE:{
    switch(fc){
    case COMM_FAULT_TEMP_NOT_SENT:
    case COMM_FAULT_TEMP_MAYBE_SENT: {
      PD((SITE,"Borrow:CommProb temp ignored"));
      return;}
    case COMM_FAULT_PERM_NOT_SENT:
    case COMM_FAULT_PERM_MAYBE_SENT:{
      PD((SITE,"Borrow:CommProb perm maybe sent lose send credit and ignore"));
      NetAddress na=NetAddress(storeSite,storeIndex);
      BorrowEntry *be=BT->find(&na);
      if(be==NULL) return;
      be->makePersistentBorrow();
      return;}}}}}


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
  Assert(sizeof(Chain)==sizeof(Construct_4));
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

TaggedRef listifyWatcherCond(EntityCond ec){
  TaggedRef list = nil();

  while(ec != 0){
    if(ec & PERM_BLOCKED)
      {list = cons(AtomPermBlocked, list);
      ec = ec & ~(PERM_BLOCKED|TEMP_BLOCKED);
      continue;}
    if(ec & TEMP_BLOCKED){
      list = cons(AtomTempBlocked, list);
      ec = ec & ~(TEMP_BLOCKED);
      continue;}
    if(ec & PERM_ME)
      {list = cons(AtomPermMe, list);
      ec = ec & ~(PERM_ME);
      continue;}
    if(ec & TEMP_ME){
      list = cons(AtomTempMe, list);
      ec = ec & ~(TEMP_ME);
      continue;}
    if(ec & PERM_ALL)
      {list = cons(AtomPermAllOthers, list);
      ec = ec & ~(TEMP_ALL|PERM_ALL);
      continue;}
    if(ec & TEMP_ALL){
      list = cons(AtomTempAllOthers, list);
      ec = ec & ~(TEMP_ALL);
      continue;}
    if(ec & PERM_SOME){
      list = cons(AtomPermSomeOther, list);
      ec = ec & ~(TEMP_SOME|PERM_SOME);
      continue;}
    if(ec & TEMP_SOME){
      list = cons(AtomTempSomeOther, list);
      ec = ec & ~(TEMP_SOME);
      continue;}
    OZ_warning("Error in ec");
    Assert(0);
  }
  return list;
}

/**********************************************************************/
/*   SECTION 44::Debug                                                */
/**********************************************************************/

ChainElem* Chain::getFirst(){return first;}
ChainElem* Chain::getLast(){return last;}

void printChain(Chain* chain){
  printf("Chain ### Flags: [");
  if(chain->hasFlag(INTERESTED_IN_OK))
    printf(" INTERESTED_IN_OK");
  if(chain->hasFlag(INTERESTED_IN_TEMP ))
    printf(" INTERESTED_IN_TEMP");
  if(chain->hasFlag( TOKEN_PERM_SOME))
    printf(" TOKEN_PERM_SOME");
  if(chain->hasFlag(TOKEN_LOST))
    printf(" TOKEN_LOST");
  printf("]\n");
  ChainElem* cp = chain->getFirst();
  while(cp!= chain->getLast()){
    Assert(cp!=NULL);
    printf("Elem  Flags: [ ");
    if(cp->flagIsSet(CHAIN_GHOST ))
       printf("CHAIN_GHOST ");
    if(cp->flagIsSet( CHAIN_QUESTION_ASKED))
       printf("CHAIN_QUESTION_ASKED ");
    if(cp->flagIsSet( CHAIN_BEFORE))
       printf("CHAIN_BEFORE ");
    if(cp->flagIsSet( CHAIN_PAST))
       printf("CHAIN_PAST ");
    if(cp->flagIsSet( CHAIN_CANT_PUT))
       printf("CHAIN_CANT_PUT ");
    if(cp->flagIsSet( CHAIN_DUPLICATE))
       printf("CHAIN_DUPLICATE ");
    printf("] %s\n",cp->getSite()->stringrep());
    cp = cp->getNext();}
  Assert(cp!=NULL);
  printf("Elem  Flags: [ ");
  if(cp->flagIsSet(CHAIN_GHOST ))
    printf(" CHAIN_GHOST");
  if(cp->flagIsSet( CHAIN_QUESTION_ASKED))
    printf(" CHAIN_QUESTION_ASKED");
  if(cp->flagIsSet( CHAIN_BEFORE))
    printf(" CHAIN_BEFORE");
  if(cp->flagIsSet( CHAIN_PAST))
    printf(" CHAIN_PAST");
  if(cp->flagIsSet( CHAIN_CANT_PUT))
    printf(" CHAIN_CANT_PUT");
  if(cp->flagIsSet( CHAIN_DUPLICATE))
    printf(" CHAIN_DUPLICATE");
  printf("] %s\n",cp->getSite()->stringrep());}

void receiveSiteDwnDbg(){

}
