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

void oz_thread_setDistVal(TaggedRef tr, int i, void* v); 
void* oz_thread_getDistVal(TaggedRef tr, int i); 

// return a DssThreadId corresponding to the current Oz thread, or a
// new DssThreadId if the operation is internal (in which case there
// is no current thread)
DssThreadId* getThreadId(){
  // Warning, this one will leak. We have to associate each thread
  // with a DssThreadId.  By some means a thread can be queried for
  // its DssId, preferably by the use of a hastable. Unfortunately
  // Mozart only supports those darn open-hashtables, not really
  // suitable for our need.  Consequently, due to laziness I have
  // chosen to do this suboptimal hack.
  Thread *t = oz_currentThread();
  if (t) {
    // get a DssThreadId corresponding to t
    TaggedRef thr = oz_thread(t);
    DssThreadId *id =
      reinterpret_cast<DssThreadId*>(oz_thread_getDistVal(thr, 1));
    if(id == NULL) {
      id = dss->m_createDssThreadId();
      oz_thread_setDistVal(thr, 1,reinterpret_cast<void*>(id));
    }
    return id; 
  } else {
    // there is no current thread, create a new one
    return dss->m_createDssThreadId();
  }
}



bool unlockDistLockImpl(Tertiary *l){
  // Here we have to do a minor violation to the sequential
  // consistency.  I don't undestand how a thread should be suspended
  // on an unlock operation.  This is a necessity, in order to
  // preserve consistency.  However, the show must go on, so I create
  // a dummy thread that acts as a goaly for the resume operation. No
  // actual thread exists.

  LockMediator *me = static_cast<LockMediator*>(index2Me(l->getTertIndex())); 
  AbstractEntity *ae = me->getAbstractEntity();
  MutableAbstractEntity *mae = static_cast<MutableAbstractEntity*>(ae);
  DssThreadId *thrId = getThreadId();
  printf("LockDistLockRelease\n");
  PstOutContainerInterface** pstout;
  // Should we assert this one will not request a pstout?
  switch(mae->abstractOperation_Write(thrId,pstout))
    {
    case DSS_PROCEED:
      return false; 

    case DSS_SUSPEND:
      thrId->setThreadMediator(new SuspendedLockRelease(me )); 
      return true;
    case DSS_RAISE:
      OZ_error("Not Implemented yet, raise");
    case DSS_INTERNAL_ERROR_NO_OP:
    case DSS_INTERNAL_ERROR_NO_PROXY:
    default: 
      OZ_error("Should not happened, skip, lock");
    }
  OZ_error("Should not happened, no true switch statement");
  
  return true;
}

bool lockDistLockImpl(Tertiary *l, Thread *thr){
  LockMediator *me = static_cast<LockMediator*>(index2Me(l->getTertIndex())); 
  AbstractEntity *ae = me->getAbstractEntity();
  MutableAbstractEntity *mae = static_cast<MutableAbstractEntity*>(ae);
  DssThreadId *thrId = getThreadId();

  printf("LockDistLockTake\n");
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Write(thrId,pstout);
  if (pstout != NULL){
    *(pstout) = new PstOutContainer(oz_thread(thr)); 
  }
  switch(cont){
    case DSS_PROCEED:
      return false; 
    case DSS_SKIP:
      OZ_error("Should not happened, skip, lock");
    case DSS_SUSPEND:
      thrId->setThreadMediator(new SuspendedLockTake(me, oz_thread(thr))); 
      return true; ;
    case DSS_RAISE:
      OZ_error("Not Implemented yet, raise");
      return PROCEED; 
    case DSS_INTERNAL_ERROR_NO_OP:
    case DSS_INTERNAL_ERROR_NO_PROXY:
    default: 
      OZ_error("Should not happened, skip, lock");
    }
  OZ_error("Should not happened, no true switch statement");
  return true; 
}



bool cellDoAccessImpl(OzCell *p, TaggedRef &ans) {
  CellMediator *me = static_cast<CellMediator*>(p->getMediator()); 
  AbstractEntity *ae = me->getAbstractEntity();
  MutableAbstractEntity *mae = static_cast<MutableAbstractEntity*>(ae);
  
  DssThreadId *thrId = getThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Read(thrId,pstout);
  if (pstout != NULL) *(pstout) = new PstOutContainer(oz_nil()); 

  switch(cont) {
  case DSS_PROCEED:
    return true; 
  case DSS_SKIP:
    Assert(0); 
    return true;   
  case DSS_SUSPEND:
    ans = static_cast<TaggedRef>(oz_newVariable());
    thrId->setThreadMediator(new SuspendedCellAccess(me, ans)); 
    return false;
  case DSS_RAISE:
    OZ_error("Not Implemented yet, raise");
    return false; 
  case DSS_INTERNAL_ERROR_NO_OP:
    OZ_error("Not Handled cellDoAccess,  DSS_INTERNAL_ERROR_NO_OP");
    return false; 
  case DSS_INTERNAL_ERROR_NO_PROXY:
  default: 
    OZ_error("Not Handled cellDoAccess, DSS_INTERNAL_ERROR_NO_PROXY ");
    return false; 
  }
}


bool cellDoExchangeImpl(OzCell *p, TaggedRef &oldVal, TaggedRef newVal) {
  // This is used for both assign and exchange! 
  CellMediator *me = static_cast<CellMediator*>(p->getMediator()); 
  AbstractEntity *ae = me->getAbstractEntity();
  MutableAbstractEntity *mae = static_cast<MutableAbstractEntity*>(ae);
  DssThreadId *thrId = getThreadId();

  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Write(thrId, pstout);
  if (pstout != NULL) *(pstout) = new PstOutContainer(newVal);

  switch(cont) {
  case DSS_PROCEED:
    return true; 
  case DSS_SKIP:
    Assert(0); 
    return true;   
  case DSS_SUSPEND:
    oldVal = static_cast<TaggedRef>(oz_newVariable());
    thrId->setThreadMediator(new SuspendedCellExchange(me, newVal, oldVal)); 
    return false;
  case DSS_RAISE:
    OZ_error("Not Implemented yet, raise");
    return false; 
  case DSS_INTERNAL_ERROR_NO_OP:
    OZ_error("Not Handled Cell Exchange  DSS_INTERNAL_ERROR_NO_OP");
    return false; 
  case DSS_INTERNAL_ERROR_NO_PROXY:
  default: 
    OZ_error("Not Handled  Cell Exchange DSS_INTERNAL_ERROR_NO_PROXY");
    return false; 
  }
}




bool portSendImpl(OzPort *p, TaggedRef msg){
  Assert(p->isDistributed());
  PortMediator *me = static_cast<PortMediator*>(p->getMediator()); 
  RelaxedMutableAbstractEntity *mae =
    static_cast<RelaxedMutableAbstractEntity*>(me->getAbstractEntity());

  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Write(pstout);
  if (pstout != NULL) *(pstout) = new PstOutContainer(msg);

  switch(cont) {
  case DSS_PROCEED:
    return false;
  case DSS_SKIP:
    return true;   
  case DSS_SUSPEND:
    Assert(0); 
    return true; 
  case DSS_RAISE:
    OZ_error("Not Implemented yet, raise");
    return false; 
  case DSS_INTERNAL_ERROR_NO_OP:
    OZ_error("Not Handled, portSend DSS_INTERNAL_ERROR_NO_OP");
    return false; 
  case DSS_INTERNAL_ERROR_NO_PROXY:
  default: 
    OZ_error("Not Handled, portSend DSS_INTERNAL_ERROR_NO_PROXY");
    return false; 
  }
}

void cellOperationDoneReadImpl(OzCell* cell, TaggedRef ans, int thid){
    OZ_error("Unexpected return value from AO_RE_W_DONE"); 
}


void cellOperationDoneWriteImpl(OzCell* cell)
{
  ; 
}



bool distArrayGetImpl(OzArray *oza, TaggedRef indx, TaggedRef &ans){
  ArrayMediator *me = static_cast<ArrayMediator*>(oza->getMediator()); 
  AbstractEntity *ae = me->getAbstractEntity();
  MutableAbstractEntity *mae = static_cast<MutableAbstractEntity*>(ae);
  
  DssThreadId *thrId = getThreadId();
    
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Read(thrId,pstout);
  if (pstout != NULL) *(pstout) = new PstOutContainer(indx); 

  if (cont == DSS_PROCEED) return false; 
  if (cont == DSS_SUSPEND) {
    ans = oz_newVariable();
    thrId->setThreadMediator(new SuspendedArrayGet(me, ans, tagged2SmallInt(indx))); 
    return true;
  }
  OZ_error("Shit, something vent wrong when doing an array op");
  return false;
}

bool distArrayPutImpl(OzArray *oza, TaggedRef indx, TaggedRef val){
  ArrayMediator *me = static_cast<ArrayMediator*>(oza->getMediator()); 
  AbstractEntity *ae = me->getAbstractEntity();
  MutableAbstractEntity *mae = static_cast<MutableAbstractEntity*>(ae);
  
  DssThreadId *thrId = getThreadId();
    
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Write(thrId,pstout);
  if (pstout != NULL) *(pstout) = new PstOutContainer(oz_cons(indx,val)); 

  if (cont == DSS_PROCEED) return false; 
  if (cont == DSS_SUSPEND) {
    thrId->setThreadMediator(new SuspendedArrayPut(me, val, tagged2SmallInt(indx))); 
    return true;
  }
  OZ_error("Shit, something vent wrong when doing an array op");
  return false;
}



/************************* Variables *************************/

// Variable are somewhat different from the other entities. 
// The variable in itself is used to cntrl the execution of
// the threads. Thus, no extra operations has to be done to 
// suspend the thread. 

// The DOE of the MEdiator is solely used to cntrl the variable, 
// and not the thread-resumes. 

bool distVarDoBind(ProxyVar* pv, TaggedRef *lPtr, TaggedRef r){
  Mediator *me = pv->getMediator();
  AbstractEntity *ae = me->getAbstractEntity();
  MonotonicAbstractEntity *tae = static_cast<MonotonicAbstractEntity *>(ae);
  DssThreadId *thrId = getThreadId();

  PstOutContainerInterface** pstout;
  OpRetVal cont = tae->abstractOperation_Bind(thrId,pstout);
  if (pstout != NULL){
    *(pstout) = new PstOutContainer(r);
  }
  switch(cont)
    {
    case DSS_PROCEED:
      delete thrId; 
      return true; 
    case DSS_SKIP:
      Assert(0);
      return true; // I dont know about this one    
    case DSS_SUSPEND:
      thrId->setThreadMediator(new SuspendedVarBind(r,me));
      return false; 
    case DSS_RAISE:
      OZ_error("Not Implemented yet, raise");
      return false; 
    case DSS_INTERNAL_ERROR_NO_OP:
      OZ_error("Not Handled, varBind DSS_INTERNAL_ERROR_NO_OP ");
      return false; 
    case DSS_INTERNAL_ERROR_NO_PROXY:
    default: 
      OZ_error("Not Handled, varBind,  DSS_INTERNAL_ERROR_NO_PROXY");
      return false; 
    }
  Assert(0); 
  return false; 
}

OZ_Return distVarDoUnify(ProxyVar* lpv,ProxyVar* rpv, TaggedRef *lPtr, TaggedRef r){
  TaggedRef bind;
  Mediator* me;
  MonotonicAbstractEntity *tae;
  MonotonicAbstractEntity *ltae = static_cast<MonotonicAbstractEntity *>(lpv->getMediator()->getAbstractEntity());
  MonotonicAbstractEntity *rtae = static_cast<MonotonicAbstractEntity *>(rpv->getMediator()->getAbstractEntity());

  DssThreadId *thrId = getThreadId();
  
  bool order = dss->m_orderEntities(ltae,rtae);
  if(order){
    tae  = ltae;
    bind = rpv->getTaggedRef();
    me   = lpv->getMediator();
  }else{
    tae  = rtae;
    bind = lpv->getTaggedRef();
    me   = rpv->getMediator();
  }

  PstOutContainerInterface** pstout;
  OpRetVal cont = tae->abstractOperation_Bind(thrId,pstout);
  if (pstout != NULL){
    *(pstout) = new PstOutContainer(bind);
  }
  
  // Same as above
  switch(cont){
  case DSS_PROCEED:
    return PROCEED;
  case DSS_SUSPEND:
    thrId->setThreadMediator(new SuspendedVarBind(r,me));
    return SUSPEND;
  case DSS_SKIP:
    Assert(0);
    return PROCEED; // I dont know about this one    
  case DSS_RAISE:
  case DSS_INTERNAL_ERROR_NO_OP:
  case DSS_INTERNAL_ERROR_NO_PROXY:
  default: 
    OZ_error("Not Handled, varUnify");
    return PROCEED; 
  }
  Assert(0);
  return PROCEED;
}


// bind a distributed variable
OZ_Return distVarBindImpl(OzVariable *ov, TaggedRef *varPtr, TaggedRef val) {
  printf("--- raph: bind distributed variable %x\n", makeTaggedRef(varPtr));
  Assert(ov == tagged2Var(*varPtr));
  Mediator *med = static_cast<Mediator*>(ov->getMediator());
  MonotonicAbstractEntity *ae =
    static_cast<MonotonicAbstractEntity*>(med->getAbstractEntity());

  // if not distributed, simply bind locally
  if (ae == NULL) {
    oz_bindLocalVar(ov, varPtr, val);
    return PROCEED;
  }

  // otherwise ask the abstract entity
  DssThreadId *thrId = getThreadId();
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
  case DSS_SUSPEND: { // suspend operation
    thrId->setThreadMediator(new SuspendedVarBind(val, med));
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
  DssThreadId *thrId = getThreadId();
  PstOutContainerInterface** pstout;
  OpRetVal cont = ae->abstractOperation_Append(thrId, pstout);

  // allocate PstOutContainer if required
  if (pstout != NULL) *(pstout) = new PstOutContainer(oz_atom("needed"));

  switch(cont) {
  case DSS_PROCEED:
  case DSS_SKIP:
    return PROCEED;
  case DSS_SUSPEND: // raph: this is a dirty solution...
    thrId->setThreadMediator(new SuspendedVarBind(0, med));
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

OZ_Return lazyVarFetch(LazyVar* lv, TaggedRef *vptr, OZ_Term Term)
{
  /*
  //printf("Fetching Lazy Var\n");
  ProxyInterface *pi = lv->getMediator()->getProxy();
  SuspLazyObject *stoc = new SuspLazyObject(vptr);
  PstOutContainer *load = new PstOutContainer(Term);
  // Prepare for full marshaling. A lazy node is a stop node
  // for the marshaler; the stopnode property must be bypassed.
  OpRetVal ret = pi->doAbstractOperation(AO_LZ_FETCH,stoc->thId,load);
  
  return decodeOpRetVal(ret,stoc);
  */
  return PROCEED;
}


void initEntityOperations(){
  cellDoExchange = &cellDoExchangeImpl;
  cellDoAccess = &cellDoAccessImpl;
  
  // Experimental

  // variables
  distVarBind = &distVarBindImpl;
  distVarUnify = &distVarUnifyImpl;
  distVarMakeNeeded = &distVarMakeNeededImpl;
  
  // The objects 
  cellAtExchange = &cellAtExchangeImpl;
  cellAtAccess   = &cellAtAccessImpl; 
  objectExchange = &objectExchangeImpl;
  
  // Ports 
  portSend = &portSendImpl;
  
  // Lock
  lockDistLock = &lockDistLockImpl; 
  unlockDistLock = &unlockDistLockImpl; 
  
  // Arrays
  distArrayPut = &distArrayPutImpl; 
  distArrayGet = &distArrayGetImpl; 
  
}
