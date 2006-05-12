/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

  // suspend if fault state not ok
  if (me->getFaultState()) return me->suspendOnFault();

  RelaxedMutableAbstractEntity *mae =
    static_cast<RelaxedMutableAbstractEntity*>(me->getAbstractEntity());
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Write(pstout);
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

  MutableAbstractEntity *mae =
    static_cast<MutableAbstractEntity*>(me->getAbstractEntity());
  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Read(thrId,pstout);
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

  MutableAbstractEntity *mae =
    static_cast<MutableAbstractEntity*>(me->getAbstractEntity());
  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Write(thrId, pstout);
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

  MutableAbstractEntity *mae =
    static_cast<MutableAbstractEntity*>(me->getAbstractEntity());
  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Write(thrId, pstout);
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

  MutableAbstractEntity *mae =
    static_cast<MutableAbstractEntity*>(me->getAbstractEntity());
  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Write(thrId, pstout);
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




void cellOperationDoneReadImpl(OzCell* cell, TaggedRef ans, int thid){
    OZ_error("Unexpected return value from AO_RE_W_DONE"); 
}


void cellOperationDoneWriteImpl(OzCell* cell)
{
  ; 
}



/******************************* Arrays *******************************/

OZ_Return distArrayGetImpl(OzArray *oza, TaggedRef indx, TaggedRef &ans) {
  ArrayMediator *me = static_cast<ArrayMediator*>(oza->getMediator());

  // suspend if fault state not ok
  if (me->getFaultState()) return me->suspendOnFault();

  // Before starting distributed operations, check the index
  if (!oza->checkIndex(tagged2SmallInt(indx)))
    return oz_raise(E_ERROR, E_KERNEL, "array", 2, makeTaggedConst(oza), indx);

  MutableAbstractEntity *mae =
    static_cast<MutableAbstractEntity*>(me->getAbstractEntity());
  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Read(thrId, pstout);
  if (pstout != NULL) *(pstout) = new PstOutContainer(indx);

  switch (cont) {
  case DSS_PROCEED:
    ans = oza->getArg(tagged2SmallInt(indx));
    if (ans) 
      return PROCEED;
    return oz_raise(E_ERROR, E_KERNEL, "array", 2, makeTaggedConst(oza), indx);
  case DSS_SUSPEND:
    ans = oz_newVariable();
    new SuspendedArrayGet(me, tagged2SmallInt(indx), ans);
    return BI_REPLACEBICALL;
  default:
    OZ_error("Unhandled error in distArrayGet");
    return PROCEED;
  }
}

OZ_Return distArrayPutImpl(OzArray *oza, TaggedRef indx, TaggedRef val) {
  ArrayMediator *me = static_cast<ArrayMediator*>(oza->getMediator()); 

  // suspend if fault state not ok
  if (me->getFaultState()) return me->suspendOnFault();

  // Before starting distributed operations, check the index
  if (!oza->checkIndex(tagged2SmallInt(indx)))
    return oz_raise(E_ERROR, E_KERNEL, "array", 2, makeTaggedConst(oza), indx);

  MutableAbstractEntity *mae =
    static_cast<MutableAbstractEntity*>(me->getAbstractEntity());
  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Write(thrId,pstout);
  if (pstout != NULL) *(pstout) = new PstOutContainer(oz_cons(indx,val));

  switch (cont) {
  case DSS_PROCEED:
    if (oza->setArg(tagged2SmallInt(indx),val)) 
      return PROCEED;
    return oz_raise(E_ERROR,E_KERNEL,"array",2,makeTaggedConst(oza),indx);
  case DSS_SUSPEND:
    new SuspendedArrayPut(me, tagged2SmallInt(indx), val);
    return BI_REPLACEBICALL;
  default:
    OZ_error("Unhandled error in distArrayPut");
    return PROCEED;
  }
}



/**************************** Dictionaries ****************************/

OZ_Return
distDictionaryGetImpl(OzDictionary *ozD, TaggedRef key, TaggedRef &ans) {
  DictionaryMediator *me =
    static_cast<DictionaryMediator*>(ozD->getMediator());

  // suspend if fault state not ok
  if (me->getFaultState()) return me->suspendOnFault();

  MutableAbstractEntity *mae = 
    static_cast<MutableAbstractEntity*>(me->getAbstractEntity());
  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Read(thrId, pstout);
  if (pstout != NULL) *(pstout) = new PstOutContainer(key);

  switch (cont) {
  case DSS_PROCEED:
    ans = ozD->getArg(key);
    if (ans) 
      return PROCEED;
    return oz_raise(E_ERROR, E_KERNEL, "dict", 2, makeTaggedConst(ozD), key);
  case DSS_SUSPEND:
    ans = oz_newVariable();
    new SuspendedDictionaryGet(me, key, ans);
    return BI_REPLACEBICALL;
  default:
    OZ_error("Unhandled error in distDictionaryGet");
    return PROCEED;
  }
}

OZ_Return
distDictionaryPutImpl(OzDictionary *ozD, TaggedRef key, TaggedRef val) {
  DictionaryMediator *me = 
    static_cast<DictionaryMediator*>(ozD->getMediator()); 

  // suspend if fault state not ok
  if (me->getFaultState()) return me->suspendOnFault();

  MutableAbstractEntity *mae =
    static_cast<MutableAbstractEntity*>(me->getAbstractEntity());
  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Write(thrId, pstout);
  if (pstout != NULL) *(pstout) = new PstOutContainer(oz_cons(key, val));

  switch (cont) {
  case DSS_PROCEED:
    ozD->setArg(key, val);
    return PROCEED;
  case DSS_SUSPEND:
    new SuspendedDictionaryPut(me, key, val);
    return BI_REPLACEBICALL;
  default:
    OZ_error("Unhandled error in distDictionaryPut");
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
  //  printf("--- raph: bind distributed variable %x\n", makeTaggedRef(varPtr));
  Assert(ov == tagged2Var(*varPtr));
  Mediator *med = static_cast<Mediator*>(ov->getMediator());

  // suspend if fault state not ok
  if (med->getFaultState()) return med->suspendOnFault();

  MonotonicAbstractEntity *ae =
    static_cast<MonotonicAbstractEntity*>(med->getAbstractEntity());

  // if not distributed, simply bind locally
  if (ae == NULL) {
    oz_bindLocalVar(ov, varPtr, val);
    return PROCEED;
  }

  // otherwise ask the abstract entity
  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = ae->abstractOperation_Bind(thrId, pstout);

  // allocate PstOutContainer if required
  if (pstout != NULL) *(pstout) = new PstOutContainer(val);

  switch(cont) {
  case DSS_PROCEED: // bind the local entity
    oz_bindLocalVar(ov, varPtr, val);
    return PROCEED; 
  case DSS_SKIP: // skip the operation: should not happen
    Assert(0);
    return PROCEED;
  case DSS_SUSPEND: { // suspend operation (no explicit resume)
    new SuspendedDummy();
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
  printf("--- raph: unify distributed variables %x and %x\n",
	 makeTaggedRef(lptr), makeTaggedRef(rptr));
  Assert(lptr != rptr);
  Assert(lvar == tagged2Var(*lptr) && rvar == tagged2Var(*rptr));
  Assert(lvar->hasMediator() && rvar->hasMediator());
  Assert(oz_isFree(*lptr));

  OzVariableMediator *lmed, *rmed;
  MonotonicAbstractEntity *lae, *rae;
  lmed = static_cast<OzVariableMediator*>(lvar->getMediator());
  rmed = static_cast<OzVariableMediator*>(rvar->getMediator());
  lae = static_cast<MonotonicAbstractEntity*>(lmed->getAbstractEntity());
  rae = static_cast<MonotonicAbstractEntity*>(rmed->getAbstractEntity());

  // first propagate need
  if (oz_isNeeded(*lptr)) oz_var_makeNeeded(rptr);
  else { if (oz_isNeeded(*rptr)) oz_var_makeNeeded(lptr); }

  // then determine which binds to which
  //  (1) bind free to read-only;
  //  (2) if both free, bind local to distributed;
  //  (3) if both free and distributed, use dss ordering.
  if (!oz_isFree(*rptr) || (lae == NULL) ||
      (rae && dss->m_orderEntities(lae, rae))) {
    return distVarBindImpl(lvar, lptr, rmed->getEntity()); // left-to-right
  } else {
    return distVarBindImpl(rvar, rptr, lmed->getEntity()); // right-to-left
  }
}


// make a distributed variable needed
OZ_Return distVarMakeNeededImpl(TaggedRef *varPtr) {
  printf("--- raph: need distributed var %x\n", makeTaggedRef(varPtr));
  // anyway make it needed locally
  oz_var_makeNeededLocal(varPtr);

  OzVariable *ov = tagged2Var(*varPtr);
  Mediator *med = static_cast<Mediator *> (ov->getMediator());
  MonotonicAbstractEntity *ae =
    static_cast<MonotonicAbstractEntity *> (med->getAbstractEntity());

  // if not distributed, return
  if (ae == NULL) return PROCEED;

  // otherwise ask the abstract entity
  DssThreadId *thrId = currentThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = ae->abstractOperation_Append(thrId, pstout);

  // allocate PstOutContainer if required
  if (pstout != NULL) *(pstout) = new PstOutContainer(oz_atom("needed"));

  switch(cont) {
  case DSS_PROCEED:
  case DSS_SKIP:
    return PROCEED;
  case DSS_SUSPEND:
    new SuspendedDummy();
    return PROCEED;
  case DSS_RAISE:
  case DSS_INTERNAL_ERROR_NO_OP:
  case DSS_INTERNAL_ERROR_NO_PROXY:
  default:
    OZ_error("error in distVarMakeNeeded");
    return PROCEED; 
  }
}



OZ_Return cellAtExchangeImpl(OzCell* cell, TaggedRef fea, TaggedRef val){
  /*
  printf("cellAtExchange feature:%s",toC(fea));
  printf(" val:%s\n", toC(val));
  ProxyInterface *pi = index2Pi(o->getIndex());
  SuspCellAtExchange *sps = new SuspCellAtExchange(o, fea, val);
  OZ_Term msg = OZ_mkTupleC("exchange",2,fea,val);
  PstOutContainer *Load = new PstOutContainer(msg); 
  OpRetVal ret = pi->doAbstractOperation(AO_STATE_WRITE, sps->thId,Load);
  return decodeOpRetVal(ret,sps);
  */
  return PROCEED;
}


OZ_Return cellAtAccessImpl(OzCell* cell, TaggedRef fea, TaggedRef val){
  /*
  printf("cellAtAccess feature:%s",toC(fea));
  printf(" val:%s\n",toC(val)); 
  ProxyInterface *pi = index2Pi(o->getIndex());
  SuspCellAtAccess *sps = new SuspCellAtAccess(o, fea, val);
  OZ_Term msg = OZ_mkTupleC("access",2,fea,val);
  PstOutContainer *Load = new PstOutContainer(msg);
  OpRetVal ret = pi->doAbstractOperation(AO_STATE_READ, sps->thId,Load);
  return  decodeOpRetVal(ret,sps);
  */
  return PROCEED;
}

OZ_Return objectExchangeImpl(OzCell* o, TaggedRef fea, TaggedRef oVal, TaggedRef nVal){
  return PROCEED;
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
  
  // objects 
  cellAtExchange = &cellAtExchangeImpl;
  cellAtAccess   = &cellAtAccessImpl; 
  objectExchange = &objectExchangeImpl;

  // arrays
  distArrayPut = &distArrayPutImpl;
  distArrayGet = &distArrayGetImpl;

  // dictionaries
  distDictionaryPut = &distDictionaryPutImpl;
  distDictionaryGet = &distDictionaryGetImpl;

  // variables
  distVarBind = &distVarBindImpl;
  distVarUnify = &distVarUnifyImpl;
  distVarMakeNeeded = &distVarMakeNeededImpl;
}
