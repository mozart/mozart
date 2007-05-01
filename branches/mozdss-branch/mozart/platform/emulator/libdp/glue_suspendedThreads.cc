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

void SuspendedOperation::resumeApply(TaggedRef p, TaggedRef args) {
  Assert(ctlVar);
  ControlVarApply(ctlVar, p, args);
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

WakeRetVal SuspendedOperation::resumeFailed() {
  // simply forget about the control var, and release the thread id
  releaseThreadId(threadId);
  threadId = NULL;
  return WRV_DONE;
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
  ConstMediator* med = static_cast<ConstMediator*>(getMediator());
  OzLock* lock = static_cast<OzLock*>(med->getConst());
  if (!lock->take(ozthread)) { // the thread must subscribe
    // the thread already waits on ctlVar, so let's reuse it!
    lock->subscribe(ozthread, ctlVar);
    ctlVar = 0;
  }
  resume();
  return WRV_DONE;
}

WakeRetVal 
SuspendedLockTake::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer* pst = static_cast<PstInContainer*>(pstin);
  // The result is either unit, or a control variable, on which the
  // thread should synchronize.  Binding ctlVar to res does the job!
  OZ_Return ret = oz_unify(ctlVar, pst->a_term);
  // It succeed immediately, since ctlVar is never distributed.
  Assert(ret == PROCEED);
  ctlVar = 0;
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

SuspendedLockRelease::SuspendedLockRelease(Mediator* med, TaggedRef thr) :
  SuspendedOperation(med), ozthread(thr)
{
  // Remember: the release operation is asynchronuous for the thread,
  // so we do not suspend it.
}

WakeRetVal SuspendedLockRelease::resumeDoLocal(DssOperationId*) {
  ConstMediator* med = static_cast<ConstMediator*>(getMediator());
  OzLock *lock = static_cast<OzLock*>(med->getConst());
  lock->release(ozthread);
  resume();
  return WRV_DONE;
}

WakeRetVal
SuspendedLockRelease::resumeRemoteDone(PstInContainerInterface* pstin){
  resume();
  return WRV_DONE;
}

bool SuspendedLockRelease::gCollect() {
  if (gc()) {
    oz_gCollectTerm(ozthread, ozthread);
    return true;
  } else
    return false;
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
  OZ_Term answer = pst->a_term;
  // Check if it's an exception
  if (oz_isSRecord(answer) && tagged2SRecord(answer)->getLabel() == E_ERROR)
      resumeRaise(answer);
  else
    resumeUnify(result, answer);
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
  PstInContainer *pst = static_cast<PstInContainer*>(pstin);
  
  // If the result sent back is NULL, means everything went fine
  if (pst == NULL) {
    resume();
  } else {
    // Check if it's an exception
    OZ_Term answer = pst->a_term;
    if (oz_isSRecord(answer) && tagged2SRecord(answer)->getLabel() == E_ERROR)
      resumeRaise(answer);
  }
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
  OZ_Term answer = pst->a_term;
  Assert(oz_isSRecord(answer));
  SRecord* recAns = tagged2SRecord(answer);
  if (recAns->getLabel() == OZ_atom("ok"))
    resumeUnify(result, OZ_subtree(answer, makeTaggedSmallInt(1)));  
  else if (recAns->getLabel() == E_ERROR)
    resumeRaise(answer);
  return WRV_DONE;
}

bool SuspendedDictionaryGet::gCollect(){
  if (gc()) {
    oz_gCollectTerm(key, key);
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
    oz_gCollectTerm(key, key);
    oz_gCollectTerm(value, value);
    return true;
  } else
    return false;
}



/************************* SuspendedObjectInvoke *************************/

SuspendedObjectInvoke::SuspendedObjectInvoke(Mediator* med, OZ_Term m) :
  SuspendedOperation(med), method(m)
{
  suspend();
}

WakeRetVal SuspendedObjectInvoke::resumeDoLocal(DssOperationId*) {
  // resume by a local call
  TaggedRef entity = getMediator()->getEntity();
  resumeApply(entity, oz_mklist(method));
  return WRV_DONE;
}

WakeRetVal
SuspendedObjectInvoke::resumeRemoteDone(PstInContainerInterface* pstin) {
  PstInContainer* pst = static_cast<PstInContainer*>(pstin);
  // The result is a control variable, we simply bind ctlVar to result!
  OZ_Return ret = oz_unify(ctlVar, pst->a_term);
  // It succeed immediately, since ctlVar is never distributed.
  Assert(ret == PROCEED);
  ctlVar = 0;
  resume();
  return WRV_DONE;
}

bool SuspendedObjectInvoke::gCollect(){
  if (gc()) {
    oz_gCollectTerm(method, method);
    return true;
  } else
    return false;
}



/************************* SuspendedObjectAccess *************************/

SuspendedObjectAccess::SuspendedObjectAccess(Mediator* med,
					     OZ_Term k, OZ_Term r) :
  SuspendedOperation(med), key(k), result(r)
{
  suspend();
}

WakeRetVal SuspendedObjectAccess::resumeDoLocal(DssOperationId*) {
  OzObject* obj = tagged2Object(getMediator()->getEntity());
  TaggedRef out = obj->getState()->getFeature(key);
  if (out)
    resumeUnify(result, out);
  else
    resumeRaise(OZ_makeException(E_SYSTEM, E_KERNEL, "object", 2, obj, key));
  return WRV_DONE;
}

WakeRetVal
SuspendedObjectAccess::resumeRemoteDone(PstInContainerInterface* pstin) {
  if (pstin) {
    PstInContainer *pst = static_cast<PstInContainer*>(pstin);
    resumeUnify(result, pst->a_term);
  } else {
    TaggedRef entity = getMediator()->getEntity();
    resumeRaise(OZ_makeException(E_ERROR, E_KERNEL, ".", 2, entity, key));
  }
  return WRV_DONE;
}

bool SuspendedObjectAccess::gCollect(){
  if (gc()) {
    oz_gCollectTerm(key, key);
    oz_gCollectTerm(result, result);
    return true;
  } else
    return false;
}



/************************* SuspendedObjectAssign *************************/

SuspendedObjectAssign::SuspendedObjectAssign(Mediator* med,
					     OZ_Term k, OZ_Term v) :
  SuspendedOperation(med), key(k), value(v)
{
  suspend();
}

WakeRetVal SuspendedObjectAssign::resumeDoLocal(DssOperationId*) {
  OzObject* obj = tagged2Object(getMediator()->getEntity());
  bool out = obj->getState()->setFeature(key, value);
  if (out)
    resume();
  else
    resumeRaise(OZ_makeException(E_SYSTEM, E_KERNEL, "object", 2, obj, key));
  return WRV_DONE;
}

WakeRetVal
SuspendedObjectAssign::resumeRemoteDone(PstInContainerInterface* pstin) {
  if (pstin)
    resume();
  else
    resumeRaise(OZ_makeException(E_SYSTEM, E_KERNEL, "object", 2,
				 getMediator()->getEntity(), key));
  return WRV_DONE;
}

bool SuspendedObjectAssign::gCollect(){
  if (gc()) {
    oz_gCollectTerm(key, key);
    oz_gCollectTerm(value, value);
    return true;
  } else
    return false;
}



/************************* SuspendedObjectExchange *************************/

SuspendedObjectExchange::SuspendedObjectExchange(Mediator* med, OZ_Term k,
						 OZ_Term newV, OZ_Term oldV) :
  SuspendedOperation(med), key(k), newVal(newV), oldVal(oldV)
{
  suspend();
}

WakeRetVal SuspendedObjectExchange::resumeDoLocal(DssOperationId*) {
  OzObject* obj = tagged2Object(getMediator()->getEntity());
  TaggedRef out = obj->getState()->getFeature(key);
  if (out) {
    obj->getState()->setFeature(key, newVal);
    resumeUnify(oldVal, out);
  } else {
    resumeRaise(OZ_makeException(E_SYSTEM, E_KERNEL, "object", 2, obj, key));
  }
  return WRV_DONE;
}

WakeRetVal
SuspendedObjectExchange::resumeRemoteDone(PstInContainerInterface* pstin) {
  if (pstin) {
    PstInContainer *pst = static_cast<PstInContainer*>(pstin);
    resumeUnify(oldVal, pst->a_term);
  } else {
    TaggedRef entity = getMediator()->getEntity();
    resumeRaise(OZ_makeException(E_ERROR, E_KERNEL, ".", 2, entity, key));
  }
  return WRV_DONE;
}

bool SuspendedObjectExchange::gCollect(){
  if (gc()) {
    oz_gCollectTerm(key, key);
    oz_gCollectTerm(newVal, newVal);
    oz_gCollectTerm(oldVal, oldVal);
    return true;
  } else
    return false;
}



/************************* SuspendedGenericDot *************************/

SuspendedGenericDot::SuspendedGenericDot(Mediator* med,
					 OZ_Term k,  OZ_Term var) :
  SuspendedOperation(med), key(k), result(var)
{
  suspend();
}

WakeRetVal SuspendedGenericDot::resumeDoLocal(DssOperationId*) {
  TaggedRef entity = getMediator()->getEntity();
  TaggedRef out;
  switch (dotInline(entity, key, out)) {
  case PROCEED:
    resumeUnify(result, out);
    break;
  case RAISE:
    resumeRaise(OZ_makeException(E_ERROR, E_KERNEL, ".", 2, entity, key));
    break;
  default:
    // should never happen, because the state is local
    Assert(0);
  }
  return WRV_DONE;
}

WakeRetVal
SuspendedGenericDot::resumeRemoteDone(PstInContainerInterface* pstin) {
  if (pstin) {
    PstInContainer *pst = static_cast<PstInContainer*>(pstin);
    resumeUnify(result, pst->a_term);
  } else {
    TaggedRef entity = getMediator()->getEntity();
    resumeRaise(OZ_makeException(E_ERROR, E_KERNEL, ".", 2, entity, key));
  }
  return WRV_DONE;
}

bool SuspendedGenericDot::gCollect(){
  if (gc()) {
    oz_gCollectTerm(key, key);
    oz_gCollectTerm(result, result);
    return true;
  } else
    return false;
}



/************************* SuspendedClassGet *************************/

SuspendedClassGet::SuspendedClassGet(Mediator* med) : SuspendedOperation(med) {
  // this operation is only a Wait, and does not replace the current
  // operation.  So we do not call suspendOnControlVar() here...
  ControlVarNew(cv, oz_rootBoard());
  ctlVar = cv;
}

WakeRetVal SuspendedClassGet::resumeDoLocal(DssOperationId*) {
  resume(); return WRV_DONE;
}

WakeRetVal SuspendedClassGet::resumeRemoteDone(PstInContainerInterface*) {
  Assert(0); return WRV_DONE;
}

bool SuspendedClassGet::gCollect() {
  return gc();
}
