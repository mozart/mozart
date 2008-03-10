/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 *    Boriss Mejias (bmc@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Erik Klintskog, 2003
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
#pragma implementation "glue_entityOpImpl.hh"
#endif

#include "value.hh"
#include "glue_mediators.hh"
#include "glue_suspendedThreads.hh"
#include "glue_tables.hh"
#include "glue_interface.hh"
#include "pstContainer.hh"
#include "unify.hh"
#include "builtins.hh"
#include "controlvar.hh"



/******************************* Ports ********************************/

OZ_Return distPortSendImpl(OzPort *p, TaggedRef msg) {
  Assert(p->isDistributed());
  PortMediator *me = static_cast<PortMediator*>(p->getMediator());

  // A port never blocks.  In case of permanent failure, just skip.
  if (me->getFaultState() > GLUE_FAULT_TEMP) return PROCEED;

  PstOutContainerInterface** pstout;
  OpRetVal cont = me->abstractOperation_Write(pstout);
  if (pstout != NULL) *(pstout) = new PstOutContainer(msg);

  switch(cont) {
  case DSS_PROCEED:
    doPortSend(p, msg, NULL);
    return PROCEED;
  case DSS_SKIP:
    return PROCEED;
  case DSS_SUSPEND:
    Assert(0); 
    return PROCEED;
  case DSS_RAISE:
    OZ_error("Not Implemented yet, raise");
    return PROCEED;
  case DSS_INTERNAL_ERROR_NO_OP:
    OZ_error("Not Handled, distPortSend DSS_INTERNAL_ERROR_NO_OP");
    return PROCEED;
  case DSS_INTERNAL_ERROR_NO_PROXY:
  default: 
    OZ_error("Not Handled, distPortSend DSS_INTERNAL_ERROR_NO_PROXY");
    return PROCEED;
  }
}



/******************************* Cells ********************************/

OZ_Return distCellAccessImpl(OzCell *c, TaggedRef &ans) {
  CellMediator *me = static_cast<CellMediator*>(c->getMediator());

  // suspend if fault state not ok
  if (me->getFaultState()) return me->suspendOnFault();

  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = me->abstractOperation_Read(thrId,pstout);
  if (pstout != NULL) *(pstout) = new PstOutContainer(oz_nil()); 

  switch(cont) {
  case DSS_PROCEED:
    ans = c->getValue();
    return PROCEED;
  case DSS_SKIP:
    Assert(0); 
    return PROCEED;
  case DSS_SUSPEND:
    ans = static_cast<TaggedRef>(oz_newVariable());
    new SuspendedCellAccess(me, ans);
    return BI_REPLACEBICALL;
  case DSS_RAISE:
    OZ_error("Not Implemented yet, raise");
    return PROCEED;
  case DSS_INTERNAL_ERROR_NO_OP:
    OZ_error("Not Handled distCellAccess, DSS_INTERNAL_ERROR_NO_OP");
    return PROCEED;
  case DSS_INTERNAL_ERROR_NO_PROXY:
  default: 
    OZ_error("Not Handled distCellAccess, DSS_INTERNAL_ERROR_NO_PROXY ");
    return PROCEED;
  }
}


OZ_Return distCellExchangeImpl(OzCell *c,
			       TaggedRef &oldVal, TaggedRef newVal) {
  // This is used for both assign and exchange! 
  CellMediator *me = static_cast<CellMediator*>(c->getMediator()); 

  // suspend if fault state not ok
  if (me->getFaultState()) return me->suspendOnFault();

  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = me->abstractOperation_Write(thrId, pstout);
  if (pstout != NULL) *(pstout) = new PstOutContainer(newVal);

  switch(cont) {
  case DSS_PROCEED:
    oldVal = c->exchangeValue(newVal);
    return PROCEED;
  case DSS_SKIP:
    Assert(0); 
    return PROCEED;
  case DSS_SUSPEND:
    oldVal = static_cast<TaggedRef>(oz_newVariable());
    new SuspendedCellExchange(me, newVal, oldVal);
    return BI_REPLACEBICALL;
  case DSS_RAISE:
    OZ_error("Not Implemented yet, raise");
    return PROCEED;
  case DSS_INTERNAL_ERROR_NO_OP:
    OZ_error("Not Handled Cell Exchange DSS_INTERNAL_ERROR_NO_OP");
    return PROCEED;
  case DSS_INTERNAL_ERROR_NO_PROXY:
  default: 
    OZ_error("Not Handled Cell Exchange DSS_INTERNAL_ERROR_NO_PROXY");
    return PROCEED;
  }
}



/****************************** Locks *********************************/

OZ_Return distLockTakeImpl(OzLock* lock, TaggedRef thr) {
  LockMediator *me = static_cast<LockMediator*>(lock->getMediator()); 

  // suspend if fault state not ok
  if (me->getFaultState()) return me->suspendOnFault();

  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = me->abstractOperation_Write(thrId, pstout);
  if (pstout != NULL)
    *(pstout) = new PstOutContainer(oz_cons(oz_atom("take"), thr));

  switch(cont) {
  case DSS_PROCEED: {
    if (lock->take(thr)) return PROCEED;
    ControlVarNew(controlvar, oz_currentBoard());
    lock->subscribe(thr, controlvar);
    SuspendOnControlVar;
  }
  case DSS_SKIP:
    Assert(0); 
    return PROCEED;
  case DSS_SUSPEND:
    new SuspendedLockTake(me, thr);
    return BI_REPLACEBICALL;
  case DSS_RAISE:
    OZ_error("Not Implemented yet, raise");
    return PROCEED;
  case DSS_INTERNAL_ERROR_NO_OP:
    OZ_error("Not Handled Cell Exchange DSS_INTERNAL_ERROR_NO_OP");
    return PROCEED;
  case DSS_INTERNAL_ERROR_NO_PROXY:
  default: 
    OZ_error("Not Handled Cell Exchange DSS_INTERNAL_ERROR_NO_PROXY");
    return PROCEED;
  }
}

OZ_Return distLockReleaseImpl(OzLock* lock, TaggedRef thr) {
  LockMediator *me = static_cast<LockMediator*>(lock->getMediator()); 

  // ignore if fault state is permFail
  if (me->getFaultState() == GLUE_FAULT_PERM) return PROCEED;

  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = me->abstractOperation_Write(thrId, pstout);
  if (pstout != NULL)
    *(pstout) = new PstOutContainer(oz_cons(oz_atom("release"), thr));

  switch(cont) {
  case DSS_PROCEED:
    lock->release(thr);
    return PROCEED;
  case DSS_SKIP:
    Assert(0); 
    return PROCEED;
  case DSS_SUSPEND:
    new SuspendedLockRelease(me, thr);
    return PROCEED;     // we do not block the current thread!
  case DSS_RAISE:
    OZ_error("Not Implemented yet, raise");
    return PROCEED;
  case DSS_INTERNAL_ERROR_NO_OP:
    OZ_error("Not Handled Cell Exchange DSS_INTERNAL_ERROR_NO_OP");
    return PROCEED;
  case DSS_INTERNAL_ERROR_NO_PROXY:
  default: 
    OZ_error("Not Handled Cell Exchange DSS_INTERNAL_ERROR_NO_PROXY");
    return PROCEED;
  }
}



/******************************* Arrays *******************************/

OZ_Return distArrayOpImpl(OperationTag op, OzArray *arr,
			  TaggedRef* arg, TaggedRef* result) {
  ArrayMediator *med = static_cast<ArrayMediator*>(arr->getMediator()); 

  // suspend if fault state not ok
  if (med->getFaultState()) return med->suspendOnFault();

  // perform DSS operation
  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal ret = (OperationWrite[op] ?
		  med->abstractOperation_Write(thrId, pstout) :
		  med->abstractOperation_Read(thrId, pstout));
  if (pstout)
    *pstout = new PstOutContainer(glue_wrap(op, OperationIn[op], arg));

  switch (ret) {
  case DSS_PROCEED:
    // perform locally
    return arrayOperation(op, arr, arg, result);
  case DSS_SUSPEND:
    // create suspended operation
    new SuspendedArrayOp(med, op, arg, result);
    return BI_REPLACEBICALL;
  default:
    OZ_error("Unhandled error in distArrayOp");
    return PROCEED;
  }
}



/**************************** Dictionaries ****************************/

OZ_Return distDictionaryOpImpl(OperationTag op, OzDictionary *dict,
			       TaggedRef* arg, TaggedRef* result) {
  DictionaryMediator *med =
    static_cast<DictionaryMediator*>(dict->getMediator()); 

  // suspend if fault state not ok
  if (med->getFaultState()) return med->suspendOnFault();

  // perform DSS operation
  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal ret = (OperationWrite[op] ?
		  med->abstractOperation_Write(thrId, pstout) :
		  med->abstractOperation_Read(thrId, pstout));
  if (pstout)
    *pstout = new PstOutContainer(glue_wrap(op, OperationIn[op], arg));

  switch (ret) {
  case DSS_PROCEED:
    // perform locally
    return dictionaryOperation(op, dict, arg, result);
  case DSS_SUSPEND:
    // create suspended operation
    new SuspendedDictionaryOp(med, op, arg, result);
    return BI_REPLACEBICALL;
  default:
    OZ_error("Unhandled error in distDictionaryOp");
    return PROCEED;
  }
}



/************************* Objects *************************/

// invoke a distributed object with a method meth
OZ_Return distObjectInvokeImpl(OzObject* obj, TaggedRef meth) {
  ObjectMediator* med = static_cast<ObjectMediator*>(obj->getMediator());

  // suspend if fault state not ok
  if (med->getFaultState()) return med->suspendOnFault();

  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = med->abstractOperation_Read(thrId, pstout);
  if (pstout != NULL) {
    Thread* thr = oz_currentThread();
    TaggedRef t = oz_thread(thr);
    *pstout = new PstOutContainer(OZ_mkTupleC("invoke", 2, meth, t));
  }

  switch (cont) {
  case DSS_PROCEED:   // local case not handled here!
    Assert(0);
    return PROCEED;
  case DSS_SUSPEND:
    new SuspendedCall(med, oz_mklist(meth));
    return BI_REPLACEBICALL;
  default:
    OZ_error("Unhandled error in distObjectInvoke");
    return PROCEED;
  }
}

// select an object feature
OZ_Return
distObjectGetFeatureImpl(OzObject* obj, TaggedRef key, TaggedRef &res) {
  ObjectMediator* med = static_cast<ObjectMediator*>(obj->getMediator());

  // suspend if fault state not ok
  if (med->getFaultState()) return med->suspendOnFault();

  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = med->abstractOperation_Read(thrId, pstout);
  if (pstout != NULL)
    *pstout = new PstOutContainer(OZ_mkTupleC("get", 1, key));

  switch (cont) {
  case DSS_PROCEED:   // local case not handled here!
    Assert(0);
    return PROCEED;
  case DSS_SUSPEND:
    res = oz_newVariable();
    new SuspendedGenericDot(med, key, res);
    return BI_REPLACEBICALL;
  default:
    OZ_error("Unhandled error in distObjectGetFeature");
    return PROCEED;
  }
}

// read an object attribute
OZ_Return distObjectAccessImpl(ObjectState* state,
			       TaggedRef key, TaggedRef &res) {
  ObjectStateMediator* med =
    static_cast<ObjectStateMediator*>(state->getMediator());

  // suspend if fault state not ok
  if (med->getFaultState()) return med->suspendOnFault();

  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = med->abstractOperation_Read(thrId, pstout);
  if (pstout != NULL)
    *pstout = new PstOutContainer(OZ_mkTupleC("access", 1, key));

  switch (cont) {
  case DSS_PROCEED:
    res = state->getFeature(key);
    if (res)
      return PROCEED;
    oz_typeError(0,"(valid) Feature");
  case DSS_SUSPEND:
    res = oz_newVariable();
    new SuspendedObjectAccess(med, key, res);
    return BI_REPLACEBICALL;
  default:
    OZ_error("Unhandled error in distObjectAccess");
    return PROCEED;
  }
}

// write an object attribute
OZ_Return distObjectAssignImpl(ObjectState* state,
			       TaggedRef key, TaggedRef val) {
  ObjectStateMediator* med =
    static_cast<ObjectStateMediator*>(state->getMediator());

  // suspend if fault state not ok
  if (med->getFaultState()) return med->suspendOnFault();

  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = med->abstractOperation_Write(thrId, pstout);
  if (pstout != NULL)
    *pstout = new PstOutContainer(OZ_mkTupleC("assign", 2, key, val));

  switch (cont) {
  case DSS_PROCEED: {
    bool ret = state->setFeature(key, val);
    if (ret) return PROCEED;
    oz_typeError(0,"(valid) Feature");
  }
  case DSS_SUSPEND:
    new SuspendedObjectAssign(med, key, val);
    return BI_REPLACEBICALL;
  default:
    OZ_error("Unhandled error in distObjectAssign");
    return PROCEED;
  }
}

// exchange an object attribute
OZ_Return distObjectExchangeImpl(ObjectState* state, TaggedRef key,
				 TaggedRef newVal, TaggedRef &oldVal) {
  ObjectStateMediator* med =
    static_cast<ObjectStateMediator*>(state->getMediator());

  // suspend if fault state not ok
  if (med->getFaultState()) return med->suspendOnFault();

  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = med->abstractOperation_Write(thrId, pstout);
  if (pstout != NULL)
    *pstout = new PstOutContainer(OZ_mkTupleC("exchange", 2, key, newVal));

  switch (cont) {
  case DSS_PROCEED:
    oldVal = state->getFeature(key);
    if (oldVal) {
      state->setFeature(key, newVal);
      return PROCEED;
    }
    oz_typeError(0,"(valid) Feature");
  case DSS_SUSPEND:
    oldVal = oz_newVariable();
    new SuspendedObjectExchange(med, key, newVal, oldVal);
    return BI_REPLACEBICALL;
  default:
    OZ_error("Unhandled error in distObjectAssign");
    return PROCEED;
  }
}



/************************* Variables *************************/

// Variable are somewhat different from the other entities. 
// The variable in itself is used to control the execution of
// the threads. Thus, no extra operations has to be done to 
// suspend the thread. 

// The DOE of the MEdiator is solely used to cntrl the variable, 
// and not the thread-resumes. 


// bind a distributed variable
OZ_Return distVarBindImpl(OzVariable *ov, TaggedRef *varPtr, TaggedRef val) {
  Assert(ov == tagged2Var(*varPtr));
  OzVariableMediator *med =
    static_cast<OzVariableMediator*>(ov->getMediator());

  // suspend if fault state not ok
  if (med->getFaultState()) return med->suspendOnFault();

  // if not distributed, simply bind locally
  if (!med->isDistributed()) {
    med->bind(val);
    return PROCEED;
  }

  // otherwise ask the abstract entity
  PstOutContainerInterface** pstout;
  OpRetVal cont = med->abstractOperation_Bind(NULL, pstout);

  // allocate PstOutContainer if required
  if (pstout != NULL) *(pstout) = new PstOutContainer(val);

  switch(cont) {
  case DSS_PROCEED: // bind the local entity
    med->bind(val);
    return PROCEED; 
  case DSS_SKIP: // the binding is done
    return PROCEED;
  case DSS_SUSPEND: { // suspend operation (no explicit resume)
    // use quiet suspension here (avoid extra distributed operations),
    // and don't add a suspension if there is no current thread
    Thread *thr = oz_currentThread();
    return (thr ? oz_var_addQuietSusp(varPtr, thr) : PROCEED);
  }
  case DSS_RAISE: // error
    OZ_error("Not Implemented yet, raise");
    return PROCEED; 
  case DSS_INTERNAL_ERROR_NO_OP:
    OZ_error("Not Handled, varBind DSS_INTERNAL_ERROR_NO_OP ");
    return PROCEED; 
  case DSS_INTERNAL_ERROR_NO_PROXY:
    OZ_error("Not Handled, varBind DSS_INTERNAL_ERROR_NO_PROXY");
    return PROCEED; 
  default:
    OZ_error("Unknown error");
    return PROCEED; 
  }
}


// unify two distributed variables (left hand side must be free)
OZ_Return distVarUnifyImpl(OzVariable *lvar, TaggedRef *lptr,
			   OzVariable *rvar, TaggedRef *rptr) {
  Assert(lptr != rptr);
  Assert(lvar == tagged2Var(*lptr) && rvar == tagged2Var(*rptr));
  Assert(lvar->hasMediator() && rvar->hasMediator());
  Assert(oz_isFree(*lptr));

  OzVariableMediator *lmed, *rmed;
  lmed = static_cast<OzVariableMediator*>(lvar->getMediator());
  rmed = static_cast<OzVariableMediator*>(rvar->getMediator());

  // first propagate need
  if (oz_isNeeded(*lptr)) oz_var_makeNeeded(rptr);
  else { if (oz_isNeeded(*rptr)) oz_var_makeNeeded(lptr); }

  // then determine which binds to which
  //  (1) bind free to read-only;
  //  (2) if both free, bind local to distributed;
  //  (3) if both free and distributed, use dss ordering.
  if (!oz_isFree(*rptr) || !lmed->isDistributed() ||
      (rmed->isDistributed() && dss->m_orderEntities(lmed, rmed))) {
    return distVarBindImpl(lvar, lptr, rmed->getEntity()); // left-to-right
  } else {
    return distVarBindImpl(rvar, rptr, lmed->getEntity()); // right-to-left
  }
}


// make a distributed variable needed
OZ_Return distVarMakeNeededImpl(TaggedRef *varPtr) {
  // anyway make it needed locally
  oz_var_makeNeededLocal(varPtr);

  OzVariable *ov = tagged2Var(*varPtr);
  OzVariableMediator *med =
    static_cast<OzVariableMediator*>(ov->getMediator());

  // if not distributed, return
  if (!med->isDistributed()) return PROCEED;

  // otherwise ask the abstract entity
  PstOutContainerInterface** pstout;
  OpRetVal cont = med->abstractOperation_Append(NULL, pstout);

  // allocate PstOutContainer if required
  if (pstout != NULL) *(pstout) = new PstOutContainer(oz_atom("needed"));

  switch(cont) {
  case DSS_PROCEED:
  case DSS_SKIP:
  case DSS_SUSPEND:
    return PROCEED;
  case DSS_RAISE:
  case DSS_INTERNAL_ERROR_NO_OP:
  case DSS_INTERNAL_ERROR_NO_PROXY:
  default:
    OZ_error("error in distVarMakeNeeded");
    return PROCEED; 
  }
}



/**************************** Chunks ****************************/

OZ_Return distChunkOpImpl(OperationTag op, SChunk *chunk,
			  TaggedRef* arg, TaggedRef* result) {
  // precondition: the chunk is only a stub
  Assert(chunk->getValue() == makeTaggedNULL());
  ChunkMediator* med =
    static_cast<ChunkMediator*>(glue_getMediator(makeTaggedConst(chunk)));

  // suspend if fault state not ok
  if (med->getFaultState()) return med->suspendOnFault();

  // perform DSS operation
  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal ret = med->abstractOperation_Read(thrId, pstout);
  if (pstout)
    *pstout = new PstOutContainer(glue_wrap(op, OperationIn[op], arg));

  switch (ret) {
  case DSS_SUSPEND:   // we cannot have DSS_PROCEED here!
    new SuspendedChunkOp(med, op, arg, result);
    return BI_REPLACEBICALL;
  default:
    OZ_error("Unhandled error in distChunkOp");
    return PROCEED;
  }
}



/**************************** Classes ****************************/

OZ_Return
distClassGetImpl(OzClass *cls) {
  // precondition: the class is only a stub
  Assert(!cls->isComplete());
  ClassMediator* med =
    static_cast<ClassMediator*>(glue_getMediator(makeTaggedConst(cls)));

  // suspend if fault state not ok
  if (med->getFaultState()) return med->suspendOnFault();

  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = med->abstractOperation_Read(thrId, pstout);

  switch (cont) {
  case DSS_SUSPEND:   // we cannot have DSS_PROCEED here!
    new SuspendedClassGet(med);
    return SUSPEND;
  default:
    OZ_error("Unhandled error in distClassGet");
    return PROCEED;
  }
}



/************************* Procedures *************************/

// invoke a distributed procedure with a list of arguments
OZ_Return distProcedureCallImpl(Abstraction* a, TaggedRef args) {
  // precondition: the procedure is not complete
  Assert(!a->isComplete());
  ProcedureMediator* med =
    static_cast<ProcedureMediator*>(glue_getMediator(makeTaggedConst(a)));

  // suspend if fault state not ok
  if (med->getFaultState()) return med->suspendOnFault();

  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = med->abstractOperation_Read(thrId, pstout);
  if (pstout != NULL) {
    Thread* thr = oz_currentThread();
    TaggedRef t = oz_thread(thr);
    *pstout = new PstOutContainer(OZ_mkTupleC("call", 2, args, t));
  }

  switch (cont) {
  case DSS_PROCEED:   // local case not handled here!
    Assert(0);
    return PROCEED;
  case DSS_SUSPEND:
    new SuspendedCall(med, args);
    return BI_REPLACEBICALL;
  default:
    OZ_error("Unhandled error in distObjectInvoke");
    return PROCEED;
  }
}



void initEntityOperations(){
  // ports
  distPortSend = &distPortSendImpl;

  // cells
  distCellAccess = &distCellAccessImpl;
  distCellExchange = &distCellExchangeImpl;
  
  // locks
  distLockTake = &distLockTakeImpl;
  distLockRelease = &distLockReleaseImpl;

  // arrays
  distArrayOp = &distArrayOpImpl;

  // dictionaries
  distDictionaryOp = &distDictionaryOpImpl;

  // objects
  distObjectInvoke = &distObjectInvokeImpl;
  distObjectGetFeature = &distObjectGetFeatureImpl;
  distObjectAccess = &distObjectAccessImpl;
  distObjectAssign = &distObjectAssignImpl;
  distObjectExchange = &distObjectExchangeImpl;

  // variables
  distVarBind = &distVarBindImpl;
  distVarUnify = &distVarUnifyImpl;
  distVarMakeNeeded = &distVarMakeNeededImpl;

  // chunks
  distChunkOp = &distChunkOpImpl;

  // classes
  distClassGet = &distClassGetImpl;

  // procedures
  distProcedureCall = &distProcedureCallImpl;
}
