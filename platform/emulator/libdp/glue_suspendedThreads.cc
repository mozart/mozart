/*
 *  Authors:
 *    Zacharias El Banna, 2002
 *    Erik Klintskog, 2002
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Zacharias El Banna, 2002
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

#if defined(INTERFACE)
#pragma implementation "glue_suspendedThreads.hh"
#endif

#include "glue_suspendedThreads.hh"
#include "glue_interface.hh"
#include "value.hh"
#include "controlvar.hh"
#include "pstContainer.hh"


/************************* DssThreadIds *************************/

// a small pool of available DssThreadIds (circular buffer)
#define POOL_SIZE 16
static DssThreadId* threadId[POOL_SIZE];
static unsigned int first = 0;     // first available DssThreadId
static unsigned int count = 0;     // how many DssThreadIds in the pool

inline void putThreadId(DssThreadId* tid) {
  threadId[(first+count) % POOL_SIZE] = tid; count++;
}
inline DssThreadId* popThreadId() {
  int f = first; first = (f+1) % POOL_SIZE; count--; return threadId[f];
}

DssThreadId* currentThreadId() {
  if (count == 0) putThreadId(dss->m_createDssThreadId());
  return threadId[first];
}

void setCurrentThreadMediator(ThreadMediator* tm) {
  popThreadId()->setThreadMediator(tm);
}

void releaseThreadId(DssThreadId* tid) {
  if (count < POOL_SIZE) putThreadId(tid); else tid->dispose();
}



/************************* SuspendedOperation *************************/

static SuspendedOperation* suspendedOpList = NULL;

void gCollectSuspendedOperations() {
  SuspendedOperation** curPtr = &suspendedOpList;
  SuspendedOperation* cur = *curPtr;
  
  while (cur) {
    if (cur->gCollect()) {
      curPtr = &(cur->next);
    } else {
      *curPtr = cur->next;   // removed from list
      delete cur;
    }
    cur = *curPtr;
  }
}

SuspendedOperation::SuspendedOperation(Mediator* med) :
  mediator(med), ctlVar(makeTaggedNULL())
{
  threadId = currentThreadId();
  setCurrentThreadMediator(this);
  next = suspendedOpList;
  suspendedOpList = this;
}

void SuspendedOperation::suspend() {
  ControlVarNew(cv, oz_rootBoard());
  ctlVar = cv;
  suspendOnControlVar();
}

void SuspendedOperation::resume() {
  if (ctlVar) { ControlVarResume(ctlVar); }
  releaseThreadId(threadId);
  threadId = NULL;
}

void SuspendedOperation::resumeRaise(TaggedRef exc) {
  Assert(ctlVar);
  ControlVarRaise(ctlVar, exc);
  releaseThreadId(threadId);
  threadId = NULL;
}

void SuspendedOperation::resumeUnify(TaggedRef a, TaggedRef b) {
  Assert(ctlVar);
  ControlVarUnify(ctlVar, a, b);
  releaseThreadId(threadId);
  threadId = NULL;
}

bool SuspendedOperation::gc() {
  if (threadId) {
    oz_gCollectTerm(ctlVar, ctlVar); return true;
  } else {
    return false;
  }
}



/************************* SuspendedDummy *************************/

SuspendedDummy::SuspendedDummy() : SuspendedOperation(NULL)
{}

WakeRetVal SuspendedDummy::resumeDoLocal(DssOperationId*) {
  resume(); return WRV_DONE;
}

WakeRetVal SuspendedDummy::resumeRemoteDone(PstInContainerInterface*) {
  resume(); return WRV_DONE;
}

bool SuspendedDummy::gCollect() {
  return gc();
}



/************************* SuspendedCellAccess *************************/

SuspendedCellAccess::SuspendedCellAccess(Mediator* med, OZ_Term var) :
  SuspendedOperation(med), result(var)
{
  suspend();
}

WakeRetVal SuspendedCellAccess::resumeDoLocal(DssOperationId*) {
  CellMediator *pM = static_cast<CellMediator*>(getMediator());
  OzCell *cell     = static_cast<OzCell*>(pM->getConst());
  OZ_Term contents = cell->getValue();
  resumeUnify(result,contents); // no exceptions here.
  return WRV_DONE; 
}

WakeRetVal SuspendedCellAccess::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer *pst = static_cast<PstInContainer*>(pstin);
  resumeUnify(result, pst->a_term); 
  return WRV_DONE; 
}

bool SuspendedCellAccess::gCollect(){
  if (gc()) {
    oz_gCollectTerm(result, result);
    return true; 
  } else
    return false; 
}



/************************* SuspendedCellExchange *************************/

SuspendedCellExchange::SuspendedCellExchange(Mediator* med,
					     OZ_Term newVal, OZ_Term ans) :
  SuspendedOperation(med), newValue(newVal), result(ans)
{
  suspend();
}

WakeRetVal SuspendedCellExchange::resumeDoLocal(DssOperationId*) {
  CellMediator *pM = static_cast<CellMediator*>(getMediator());
  OzCell *cell     = static_cast<OzCell*>(pM->getConst());
  OZ_Term contents = cell->exchangeValue(newValue); 
  resumeUnify(result, contents); // no exceptions here
  return WRV_DONE; 
}

WakeRetVal SuspendedCellExchange::resumeRemoteDone(PstInContainerInterface* pstin) {
  PstInContainer *pst = static_cast<PstInContainer*>(pstin);
  resumeUnify(result, pst->a_term);
  return WRV_DONE;
}

bool SuspendedCellExchange::gCollect(){
  if (gc()) {
    oz_gCollectTerm(newValue, newValue);
    oz_gCollectTerm(result, result);
    return true; 
  } else
    return false; 
}



/************************* SuspendedLockTake *************************/

SuspendedLockTake::SuspendedLockTake(Mediator* med, TaggedRef thr) :
  SuspendedOperation(med), ozthread(thr)
{
  suspend();
}

WakeRetVal SuspendedLockTake::resumeDoLocal(DssOperationId*) {
  ConstMediator *med = static_cast<ConstMediator*>(getMediator());
  OzLock *lock = static_cast<OzLock*>(med->getConst());
  Thread *theThread = oz_ThreadToC(ozthread);
  if (lock->getLocker() == NULL || lock->getLocker() == theThread) {
    lock->lockB(theThread);
  } else {
    PendThread **pt = lock->getPendBase(); 
    while(*pt!=NULL) { pt= &((*pt)->next); }
    *pt = new PendThread(ozthread, NULL,  ctlVar);
    ctlVar = 0;     // resume() should not trigger control var...
  }
  resume();
  return WRV_DONE; 
}

WakeRetVal 
SuspendedLockTake::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer *pst = static_cast<PstInContainer*>(pstin);
  TaggedRef lst = pst->a_term;

  ConstMediator *med = static_cast<ConstMediator*>(getMediator());
  OzLock *lck     = static_cast<OzLock*>(med->getConst());
  switch(oz_intToC(oz_head(lst))){
  case 1:
    // We had it, remove the unlock stackframe! 
    // Remove the darn 
    // Fall through!
  case 2:
    break;
  case 3:
    oz_unify(oz_head(oz_tail(lst)), ctlVar);
    ctlVar = 0;
    break; 
  }
  resume();
  return WRV_DONE;
}

bool SuspendedLockTake::gCollect() {
  if (gc()) {
    oz_gCollectTerm(ozthread, ozthread);
    return true;
  } else
    return false;
}



/************************* SuspendedLockRelease *************************/

SuspendedLockRelease::SuspendedLockRelease(Mediator* med) :
  SuspendedOperation(med)
{}

WakeRetVal 
SuspendedLockRelease::resumeDoLocal(DssOperationId*) {
  ConstMediator *med = static_cast<ConstMediator*>(getMediator());
  OzLock *lock = static_cast<OzLock*>(med->getConst());
  lock->unlock();
  resume();
  return WRV_DONE;
}

WakeRetVal 
SuspendedLockRelease::resumeRemoteDone(PstInContainerInterface* pstin){
  resume();
  return WRV_DONE;
}

bool SuspendedLockRelease::gCollect() {
  return gc();
}



/************************* SuspendedArrayGet *************************/

SuspendedArrayGet::SuspendedArrayGet(Mediator* med, int idx,  OZ_Term var) :
  SuspendedOperation(med), index(idx), result(var)
{
  suspend();
}

WakeRetVal SuspendedArrayGet::resumeDoLocal(DssOperationId*) {
  ArrayMediator *pM = static_cast<ArrayMediator*>(getMediator());
  OzArray* oza      = static_cast<OzArray*>(pM->getConst());
  TaggedRef out = oza->getArg(index);
  if (out) 
    resumeUnify(result, out);
  else 
    resumeRaise(OZ_makeException(E_ERROR, E_KERNEL, "array", 2, oza, index));
  return WRV_DONE;
}

WakeRetVal SuspendedArrayGet::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer *pst = static_cast<PstInContainer*>(pstin);
  resumeUnify(result, pst->a_term);
  return WRV_DONE;
}

bool SuspendedArrayGet::gCollect(){
  if (gc()) {
    oz_gCollectTerm(result, result);
    return true;
  } else
    return false;
}



/************************* SuspendedArrayPut *************************/

SuspendedArrayPut::SuspendedArrayPut(Mediator* med, int idx, OZ_Term val) :
  SuspendedOperation(med), index(idx), value(val)
{
  suspend();
}

WakeRetVal SuspendedArrayPut::resumeDoLocal(DssOperationId*) {
  ArrayMediator *pM = static_cast<ArrayMediator*>(getMediator());
  OzArray*oza       = static_cast<OzArray*>(pM->getConst());
  TaggedRef *ar = oza->getRef();
  if (oza->setArg(index,value))
    resume();
  else
    resumeRaise(OZ_makeException(E_ERROR, E_KERNEL, "array", 2, oza, index));
  return WRV_DONE;
}

WakeRetVal SuspendedArrayPut::resumeRemoteDone(PstInContainerInterface* pstin){
  resume();
  return WRV_DONE;
}

bool SuspendedArrayPut::gCollect() {
  if (gc()) {
    oz_gCollectTerm(value, value);
    return true;
  } else
    return false;
}


/************************* SuspendedDictionaryGet *************************/

SuspendedDictionaryGet::SuspendedDictionaryGet(Mediator* med, OZ_Term k,  OZ_Term var) :
  SuspendedOperation(med), key(k), result(var)
{
  suspend();
}

WakeRetVal SuspendedDictionaryGet::resumeDoLocal(DssOperationId*) {
  DictionaryMediator *dm = static_cast<DictionaryMediator*>(getMediator());
  OzDictionary* ozD  = static_cast<OzDictionary*>(dm->getConst());
  TaggedRef out = ozD->getArg(key);
  if (out)
    resumeUnify(result, out);
  else
    resumeRaise(OZ_makeException(E_SYSTEM, E_KERNEL, "dict", 2, ozD, key));
  return WRV_DONE;
}

WakeRetVal SuspendedDictionaryGet::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer *pst = static_cast<PstInContainer*>(pstin);
  resumeUnify(result, pst->a_term);
  return WRV_DONE;
}

bool SuspendedDictionaryGet::gCollect(){
  if (gc()) {
    oz_gCollectTerm(result, result);
    return true;
  } else
    return false;
}



/************************* SuspendedDictionaryPut *************************/

SuspendedDictionaryPut::SuspendedDictionaryPut(Mediator* med, OZ_Term k, OZ_Term val) :
  SuspendedOperation(med), key(k), value(val)
{
  suspend();
}

WakeRetVal SuspendedDictionaryPut::resumeDoLocal(DssOperationId*) {
  DictionaryMediator *dm = static_cast<DictionaryMediator*>(getMediator());
  OzDictionary* ozD = static_cast<OzDictionary*>(dm->getConst());
  ozD->setArg(key, value);
  resume();
  return WRV_DONE;
}

WakeRetVal SuspendedDictionaryPut::resumeRemoteDone(PstInContainerInterface* pstin){
  resume();
  return WRV_DONE;
}

bool SuspendedDictionaryPut::gCollect() {
  if (gc()) {
    oz_gCollectTerm(value, value);
    return true;
  } else
    return false;
}

