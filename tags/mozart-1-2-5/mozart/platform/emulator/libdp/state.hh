/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Copyright:
 *    Per Brand, 1998
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __STATEHH
#define __STATEHH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"
#include "value.hh"


/**********************************************************************/
/*  SECTION: class CellSec, CellProxy, CellManager                    */
/**********************************************************************/

class CellSec:public CellSecEmul{
friend class CellFrame;
friend class CellManager;
friend class Chain;
public:
  USEHEAPMEMORY;
  NO_DEFAULT_CONSTRUCTORS2(CellSec);
  CellSec(TaggedRef val){ // on globalize
    Assert(sizeof(CellSecEmul) == sizeof(CellSec));
    state=Cell_Lock_Valid;
    pending=NULL;
    next=NULL;
    contents=val;
    pendBinding=NULL;}

  CellSec(){ // on Proxy becoming Frame
    Assert(sizeof(CellSecEmul) == sizeof(CellSec));
    state=Cell_Lock_Invalid;
    pending=NULL;
    pendBinding=NULL;
    next=NULL;}

  PendThread* getPending(){return pending;}
  DSite* getNext(){return next;}
  PendThread** getPendBase(){return &pending;}

  void gcCellSec();
  OZ_Return exchange(Tertiary*,TaggedRef,TaggedRef,ExKind);
  OZ_Return access(Tertiary*,TaggedRef,TaggedRef);
  OZ_Return exchangeVal(TaggedRef,TaggedRef,ExKind);

  Bool secReceiveRemoteRead(DSite*,DSite*,int);
  void secReceiveReadAns(TaggedRef);
  Bool secReceiveContents(TaggedRef,DSite* &,TaggedRef &);
  Bool secForward(DSite*,TaggedRef&);
  // failure
  Bool cellRecovery(TaggedRef);
  TaggedRef unpendCell(PendThread*,TaggedRef);
  void dummyExchange(CellManager*);

  // site shutdown;
  void markDumpAsk() {
    Assert(!(state & Cell_Lock_Dump_Asked));
    state |= Cell_Lock_Dump_Asked;
  }
};

class CellProxy : public Tertiary {
private:
  int holder; // mm2: on alpha sizeof(int) != sizeof(void *)
  void *dummy; // mm2
public:
  NO_DEFAULT_CONSTRUCTORS(CellProxy)

  CellProxy(int manager):Tertiary(manager,Co_Cell,Te_Proxy){  // on import
    holder = 0;}
};

//
// "Real" cell manager - handles cells' access structures;
class CellManager : public  CellManagerEmul {
public:
  CellSec* getCellSec(){return (CellSec*) getSec();}
  NO_DEFAULT_CONSTRUCTORS2(CellManager)
  CellManager() {
    Assert(sizeof(CellManagerEmul) == sizeof(CellManager));
    Assert(0);}

  Chain *getChain() {return chain;}
  void setChain(Chain *ch) { chain = ch; }

  void init(int index,Chain *ch, CellSec *secX){
    setTertType(Te_Manager);
    setIndex(index);
    setChain(ch);
    sec=secX;}

  void setOwnCurrent();
  Bool isOwnCurrent();
  DSite* getCurrent();
  void gcCellManager();

  PendThread* getPending(){return sec->pending;}
  PendThread *getPendBinding(){return sec->pendBinding;}

  // failure
  void tokenLost();
};


class CellFrame : public CellFrameEmul {
public:
  CellSec* getCellSec(){return (CellSec*) getSec();}
  void setCellSec(CellSec* cs){sec=(CellSecEmul*) cs;}
  NO_DEFAULT_CONSTRUCTORS2(CellFrame);
  CellFrame(){
    Assert(0);}

  Bool dumpCandidate(){
    if((getCellSec()->state & Cell_Lock_Valid)) return OK;
    return NO;}
  
  void myStoreForward(void* f) { forward = f; }
  void* getForward()           { return forward; }

  void convertToProxy(){
    setTertType(Te_Proxy);
    sec=NULL;}

  void convertFromProxy(){
    setTertType(Te_Frame);
    sec=new CellSec();}

  void gcCellFrame();
};

/**********************************************************************/
/*  SECTION: class LockSec, LockProxy, LockManager                    */
/**********************************************************************/

class LockSec : public LockSecEmul {
public:
  USEHEAPMEMORY;
  NO_DEFAULT_CONSTRUCTORS2(LockSec)
  LockSec(Thread *t,PendThread *pt){ // on globalize
    Assert(sizeof(LockSecEmul) == sizeof(LockSec));
    state=Cell_Lock_Valid;
    pending=pt;
    locker=t;
    next=NULL; }

  LockSec(){ // on Proxy becoming Frame
    Assert(sizeof(LockSecEmul) == sizeof(LockSec));
    state=Cell_Lock_Invalid;
    locker=NULL;
    pending=NULL;
    next=NULL;}

  Bool isPending(Thread *th);      
  PendThread* getPending(){return pending;}

  DSite* getNext(){return next;}

  void lockComplex(Thread* th,Tertiary*);
  void unlockComplex(Tertiary* );
  void unlockComplexB(Thread *);
  void unlockPending(Thread*);
  void gcLockSec();
  Bool secReceiveToken(Tertiary*,DSite* &);
  Bool secForward(DSite*);

  void makeRequested(){
    Assert(state==Cell_Lock_Invalid);
    state=Cell_Lock_Requested;}

  // failure;
  Bool lockRecovery();

  // site shutdown;
  void markDumpAsk() {
    Assert(!(state & Cell_Lock_Dump_Asked));
    state |= Cell_Lock_Dump_Asked;
  }
  void markInvalid() {
    Assert(!(state & Cell_Lock_Invalid));
    state = Cell_Lock_Invalid;
  }
};

class LockFrame : public LockFrameEmul {
public:
  LockSec* getLockSec(){return (LockSec*) getSec();}
  void setLockSec(LockSec* cs){sec=(LockSecEmul*)cs;}
  NO_DEFAULT_CONSTRUCTORS2(LockFrame);
  LockFrame(){Assert(0);}

  Bool dumpCandidate(){
    if(getLockSec()->state & Cell_Lock_Valid) return OK;
    return NO;}

  void myStoreForward(void* f) { forward=f; }
  void* getForward() { return forward; }
    
  void convertToProxy(){
    setTertType(Te_Proxy);
    sec=NULL;}

  void convertFromProxy(){
    setTertType(Te_Frame);
    sec=new LockSec();}

  void gcLockFrame();
};    

class LockManager : public LockManagerEmul {
public:
  LockSec* getLockSec(){return (LockSec*) getSec();}
  NO_DEFAULT_CONSTRUCTORS2(LockManager);
  LockManager() {Assert(0);}

  Chain *getChain() {return chain;}
  void setChain(Chain *ch) { chain = ch; }

  void init(int index,Chain *ch, LockSec *secX){
    setTertType(Te_Manager);
    setIndex(index);
    setChain(ch);
    sec=secX;}
  
  void gcLockManager();
  void setOwnCurrent();
  Bool isOwnCurrent();
  DSite* getCurrent();

  // failure
  void probeFault(DSite*, int);
  void tokenLost();
};

class LockProxy:public OzLock{
private:
  int holder; // mm2: on alpha sizeof(int) != sizeof(void *)
  void *dummy; // mm2
public:
  NO_DEFAULT_CONSTRUCTORS(LockProxy)
  LockProxy(int manager):OzLock(manager,Te_Proxy){  // on import
    holder = 0;}

  void lock(Thread *);
  void unlock();
  // failure
  void probeFault(DSite*, int);
};

/**********************************************************************/
/*  SECTION: provide routines                                         */
/**********************************************************************/

// may not be 'Local';
void gcDistCell(Tertiary *t);
void gcDistLock(Tertiary *t);

void convertCellProxyToFrame(Tertiary *t);
void convertLockProxyToFrame(Tertiary *t);

inline void maybeConvertLockProxyToFrame(Tertiary *t){
  if(t->isProxy())
    {convertLockProxyToFrame(t);}}

inline void maybeConvertCellProxyToFrame(Tertiary *t){
  if(t->isProxy()){
    convertCellProxyToFrame(t);}}

Chain* getChainFromTertiary(Tertiary*);
CellSec *getCellSecFromTert(Tertiary *c);
LockSec *getLockSecFromTert(Tertiary *c);
int getStateFromLockOrCell(Tertiary*);
PendThread* getPendThreadStartFromCellLock(Tertiary *);

//
OZ_Return cellDoExchangeInternal(Tertiary *, TaggedRef, TaggedRef,
				 Thread *, ExKind);
OZ_Return objectExchangeImpl(Tertiary *c,TaggedRef fea,TaggedRef old,TaggedRef nw);

//
OZ_Return cellDoExchangeImpl(Tertiary *c,TaggedRef old,TaggedRef nw);
OZ_Return cellDoAccessImpl(Tertiary *c, TaggedRef val);
OZ_Return cellAtAccessImpl(Tertiary *c, TaggedRef fea, TaggedRef val);
OZ_Return cellAtExchangeImpl(Tertiary *c,TaggedRef old,TaggedRef nw);
OZ_Return cellAssignExchangeImpl(Tertiary *c,TaggedRef fea,TaggedRef val);
//
void lockLockProxyImpl(Tertiary *t, Thread *thr);
LockRet lockLockManagerOutlineImpl(LockManagerEmul *lmu, Thread *thr);
void unlockLockManagerOutlineImpl(LockManagerEmul *lmu, Thread *thr);
LockRet lockLockFrameOutlineImpl(LockFrameEmul *lfu, Thread *thr);
void unlockLockFrameOutlineImpl(LockFrameEmul *lfu, Thread *thr);
void secLockGet(LockSec*,Tertiary*,Thread*);
//
void gcDistCellRecurseImpl(Tertiary *t);
void gcDistLockRecurseImpl(Tertiary *t);
ConstTerm* auxGcDistCellImpl(Tertiary *t);
ConstTerm* auxGcDistLockImpl(Tertiary *t);

void globalizeCell(CellLocal*, int);
void globalizeLock(LockLocal*, int);

void cellLock_Perm(int state,Tertiary* t);
void cellLock_Temp(int state,Tertiary* t);
void cellLock_OK(int state,Tertiary* t);

void ooExchGetFeaOld(TaggedRef,TaggedRef&,TaggedRef&);
Bool tertiaryFail(Tertiary*, EntityCond &,TaggedRef &);
OZ_Return tertiaryFailHandler(Tertiary*, TaggedRef,EntityCond,TaggedRef);

#endif


