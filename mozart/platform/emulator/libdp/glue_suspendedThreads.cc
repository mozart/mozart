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



/************************* SuspendedCellOp *************************/

SuspendedCellOp::SuspendedCellOp(Mediator* med, OperationTag _op,
				 TaggedRef* a, TaggedRef* res) :
  SuspendedOperation(med), op(_op)
{
  // cell operations have at most one argument
  args[0] = OperationIn[_op]-1 > 0 ? a[0] : makeTaggedNULL();
  result = res ? (*res = oz_newVariable()) : makeTaggedNULL();
  suspend();
}

WakeRetVal SuspendedCellOp::resumeDoLocal(DssOperationId*) {
  CellMediator* med = static_cast<CellMediator*>(getMediator());
  OzCell* cell = static_cast<OzCell*>(med->getConst());
  TaggedRef out;
  int ret = cellOperation(op, cell, args, &out);
  if (ret == PROCEED) {
    if (result) resumeUnify(result, out); else resume();
  } else {
    Assert(ret == RAISE);
    resumeRaise(am.getExceptionValue());
  }
  return WRV_DONE;
}

WakeRetVal SuspendedCellOp::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer* pst = static_cast<PstInContainer*>(pstin);
  if (pst == NULL) {
    resume();
  } else if (glue_isReturn(pst->a_term)) {
    resumeUnify(result, glue_getData(pst->a_term));
  } else {
    resumeRaise(glue_getData(pst->a_term));
  }
  return WRV_DONE;
}

bool SuspendedCellOp::gCollect(){
  if (gc()) {
    oz_gCollectTerm(args[0], args[0]);
    oz_gCollectTerm(result, result);
    return true;
  }
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



/************************* SuspendedArrayOp *************************/

SuspendedArrayOp::SuspendedArrayOp(Mediator* med, OperationTag _op,
				   TaggedRef* a, TaggedRef* res) :
  SuspendedOperation(med), op(_op)
{
  for (int i = 0; i < 2; i++) {
    args[i] = i < OperationIn[_op] ? a[i] : makeTaggedNULL();
  }
  result = res ? (*res = oz_newVariable()) : makeTaggedNULL();
  suspend();
}

WakeRetVal SuspendedArrayOp::resumeDoLocal(DssOperationId*) {
  ArrayMediator* med = static_cast<ArrayMediator*>(getMediator());
  OzArray* arr = static_cast<OzArray*>(med->getConst());
  TaggedRef out;
  int ret = arrayOperation(op, arr, args, &out);
  if (ret == PROCEED) {
    if (result) resumeUnify(result, out); else resume();
  } else {
    Assert(ret == RAISE);
    resumeRaise(am.getExceptionValue());
  }
  return WRV_DONE;
}

WakeRetVal SuspendedArrayOp::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer* pst = static_cast<PstInContainer*>(pstin);
  if (pst == NULL) {
    resume();
  } else if (glue_isReturn(pst->a_term)) {
    resumeUnify(result, glue_getData(pst->a_term));
  } else {
    resumeRaise(glue_getData(pst->a_term));
  }
  return WRV_DONE;
}

bool SuspendedArrayOp::gCollect(){
  if (gc()) {
    OZ_gCollectBlock(args, args, 2);
    oz_gCollectTerm(result, result);
    return true;
  }
  return false;
}



/************************* SuspendedDictionaryOp *************************/

SuspendedDictionaryOp::SuspendedDictionaryOp(Mediator* med, OperationTag _op,
					     TaggedRef* a, TaggedRef* res) :
  SuspendedOperation(med), op(_op)
{
  for (int i = 0; i < 3; i++) {
    args[i] = i < OperationIn[_op] ? a[i] : makeTaggedNULL();
  }
  result = res ? (*res = oz_newVariable()) : makeTaggedNULL();
  suspend();
}

WakeRetVal SuspendedDictionaryOp::resumeDoLocal(DssOperationId*) {
  DictionaryMediator* med = static_cast<DictionaryMediator*>(getMediator());
  OzDictionary* dict = static_cast<OzDictionary*>(med->getConst());
  TaggedRef out;
  int ret = dictionaryOperation(op, dict, args, &out);
  if (ret == PROCEED) {
    if (result) resumeUnify(result, out); else resume();
  } else {
    Assert(ret == RAISE);
    resumeRaise(am.getExceptionValue());
  }
  return WRV_DONE;
}

WakeRetVal SuspendedDictionaryOp::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer* pst = static_cast<PstInContainer*>(pstin);
  if (pst == NULL) {
    resume();
  } else if (glue_isReturn(pst->a_term)) {
    resumeUnify(result, glue_getData(pst->a_term));
  } else {
    resumeRaise(glue_getData(pst->a_term));
  }
  return WRV_DONE;
}

bool SuspendedDictionaryOp::gCollect(){
  if (gc()) {
    OZ_gCollectBlock(args, args, 3);
    oz_gCollectTerm(result, result);
    return true;
  }
  return false;
}



/************************* SuspendedObjectOp *************************/

SuspendedObjectOp::SuspendedObjectOp(Mediator* med, OperationTag _op,
				     TaggedRef* a, TaggedRef* res) :
  SuspendedOperation(med), op(_op)
{
  for (int i = 0; i < 2; i++) {
    args[i] = i < OperationIn[_op] ? a[i] : makeTaggedNULL();
  }
  result = res ? (*res = oz_newVariable()) : makeTaggedNULL();
  suspend();
}

WakeRetVal SuspendedObjectOp::resumeDoLocal(DssOperationId*) {
  ObjectMediator* med = static_cast<ObjectMediator*>(getMediator());
  OzObject* obj = static_cast<OzObject*>(med->getConst());
  TaggedRef out;
  int ret = objectOperation(op, obj, args, &out);
  if (ret == PROCEED) {
    if (result) resumeUnify(result, out); else resume();
  } else {
    Assert(ret == RAISE);
    resumeRaise(am.getExceptionValue());
  }
  return WRV_DONE;
}

WakeRetVal SuspendedObjectOp::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer* pst = static_cast<PstInContainer*>(pstin);
  if (pst == NULL) {
    resume();
  } else if (glue_isReturn(pst->a_term)) {
    resumeUnify(result, glue_getData(pst->a_term));
  } else {
    resumeRaise(glue_getData(pst->a_term));
  }
  return WRV_DONE;
}

bool SuspendedObjectOp::gCollect(){
  if (gc()) {
    OZ_gCollectBlock(args, args, 2);
    oz_gCollectTerm(result, result);
    return true;
  }
  return false;
}



/************************* SuspendedObjectStateOp *************************/

SuspendedObjectStateOp::SuspendedObjectStateOp(Mediator* med, OperationTag _op,
					       TaggedRef* a, TaggedRef* res) :
  SuspendedOperation(med), op(_op)
{
  for (int i = 0; i < 2; i++) {
    args[i] = i < OperationIn[_op] ? a[i] : makeTaggedNULL();
  }
  result = res ? (*res = oz_newVariable()) : makeTaggedNULL();
  suspend();
}

WakeRetVal SuspendedObjectStateOp::resumeDoLocal(DssOperationId*) {
  ObjectMediator* med = static_cast<ObjectMediator*>(getMediator());
  ObjectState* state = static_cast<ObjectState*>(med->getConst());
  TaggedRef out;
  int ret = ostateOperation(op, state, args, &out);
  if (ret == PROCEED) {
    if (result) resumeUnify(result, out); else resume();
  } else {
    Assert(ret == RAISE);
    resumeRaise(am.getExceptionValue());
  }
  return WRV_DONE;
}

WakeRetVal
SuspendedObjectStateOp::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer* pst = static_cast<PstInContainer*>(pstin);
  if (pst == NULL) {
    resume();
  } else if (glue_isReturn(pst->a_term)) {
    resumeUnify(result, glue_getData(pst->a_term));
  } else {
    resumeRaise(glue_getData(pst->a_term));
  }
  return WRV_DONE;
}

bool SuspendedObjectStateOp::gCollect(){
  if (gc()) {
    OZ_gCollectBlock(args, args, 2);
    oz_gCollectTerm(result, result);
    return true;
  }
  return false;
}



/************************* SuspendedChunkOp *************************/

SuspendedChunkOp::SuspendedChunkOp(Mediator* med, OperationTag _op,
				   TaggedRef* a, TaggedRef* res) :
  SuspendedOperation(med), op(_op)
{
  for (int i = 0; i < 2; i++) {
    args[i] = i < OperationIn[_op] ? a[i] : makeTaggedNULL();
  }
  result = res ? (*res = oz_newVariable()) : makeTaggedNULL();
  suspend();
}

WakeRetVal SuspendedChunkOp::resumeDoLocal(DssOperationId*) {
  ChunkMediator* med = static_cast<ChunkMediator*>(getMediator());
  SChunk* chunk = static_cast<SChunk*>(med->getConst());
  TaggedRef out;
  int ret = chunkOperation(op, chunk, args, &out);
  if (ret == PROCEED) {
    if (result) resumeUnify(result, out); else resume();
  } else {
    Assert(ret == RAISE);
    resumeRaise(am.getExceptionValue());
  }
  return WRV_DONE;
}

WakeRetVal SuspendedChunkOp::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer* pst = static_cast<PstInContainer*>(pstin);
  if (pst == NULL) {
    resume();
  } else if (glue_isReturn(pst->a_term)) {
    resumeUnify(result, glue_getData(pst->a_term));
  } else {
    resumeRaise(glue_getData(pst->a_term));
  }
  return WRV_DONE;
}

bool SuspendedChunkOp::gCollect(){
  if (gc()) {
    OZ_gCollectBlock(args, args, 2);
    oz_gCollectTerm(result, result);
    return true;
  }
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
  case SUSPEND:
    // entity is an object, and its class is incomplete
    Assert(oz_isObject(entity));
    Assert(! tagged2Object(entity)->getClass()->isComplete());
    // cancel suspensions, and re-apply 'dot'.  The latter will wait
    // for the class, and then resume locally.
    am.emptySuspendVarList();
    resumeApply(BI_dot, oz_mklist(entity, key, result));
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



/************************* SuspendedCall *************************/

SuspendedCall::SuspendedCall(Mediator* med, OZ_Term list) :
  SuspendedOperation(med), args(list)
{
  Assert(OZ_isList(list, NULL));
  suspend();
}

WakeRetVal SuspendedCall::resumeDoLocal(DssOperationId*) {
  // resume by a local call
  TaggedRef entity = getMediator()->getEntity();
  resumeApply(entity, args);
  return WRV_DONE;
}

WakeRetVal
SuspendedCall::resumeRemoteDone(PstInContainerInterface* pstin) {
  // the calling thread synchronizes on the result
  PstInContainer* pst = static_cast<PstInContainer*>(pstin);
  resumeApply(BI_wait, oz_mklist(glue_getData(pst->a_term)));
  return WRV_DONE;
}

bool SuspendedCall::gCollect(){
  if (gc()) {
    oz_gCollectTerm(args, args);
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
