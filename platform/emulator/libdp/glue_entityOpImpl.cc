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

void oz_thread_setDistVal(TaggedRef tr, int i, void* v); 
void* oz_thread_getDistVal(TaggedRef tr, int i); 

DssThreadId* getThreadId(){
  // Warning, this one will leak. We have to associate each thread
  // with a DssThreadId.  By some means a thread can be queried for
  // its DssId, preferably by the use of a hastable. Unfortunately
  // Mozart only supports those darn open-hashtables, not really
  // suitable for our need.  Consequently, due to laziness I have
  // chosen to do this suboptimal hack.
  TaggedRef thr = oz_thread(oz_currentThread());
  DssThreadId *id = reinterpret_cast<DssThreadId*>(oz_thread_getDistVal(thr, 1));
  if(id == NULL) {
    id = dss->m_createDssThreadId();
    oz_thread_setDistVal(thr, 1,reinterpret_cast<void*>(id));
  }
  return id; 
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


bool cellDoAccessImpl(Tertiary *p, TaggedRef &ans )
{
  CellMediator *me = static_cast<CellMediator*>(index2Me(p->getTertIndex())); 
  AbstractEntity *ae = me->getAbstractEntity();
  MutableAbstractEntity *mae = static_cast<MutableAbstractEntity*>(ae);
  
  DssThreadId *thrId = getThreadId();

  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Read(thrId,pstout);
  if (pstout != NULL){
    *(pstout) = new PstOutContainer(oz_nil()); 
  }

  switch(cont)
    {
    case DSS_PROCEED:
      {
	return true; 
      }
    case DSS_SKIP:
      Assert(0); 
      return true;   
    case DSS_SUSPEND:
      {
	OZ_Term var = oz_newVariable();
	ans = var; 
	thrId->setThreadMediator(new SuspendedCellAccess(me, ans)); 
	return false;
      }
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
  return false;
}


 bool cellDoExchangeImpl(Tertiary *p, TaggedRef &oldVal,TaggedRef newVal )
{
  // This is used for both assign and exchange! 
  CellMediator *me = static_cast<CellMediator*>(index2Me(p->getTertIndex())); 
  AbstractEntity *ae = me->getAbstractEntity();
  MutableAbstractEntity *mae = static_cast<MutableAbstractEntity*>(ae);
  DssThreadId *thrId = getThreadId();


  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Write(thrId, pstout);
  if (pstout != NULL){
    *(pstout) = new PstOutContainer(newVal);
  }
  switch(cont)
    {
    case DSS_PROCEED:
      {
	return true; 
      }
    case DSS_SKIP:
      Assert(0); 
      return true;   
    case DSS_SUSPEND:
      {
	OZ_Term var = oz_newVariable();
	oldVal = var; 
	thrId->setThreadMediator(new SuspendedCellExchange(me, newVal, var)); 
	return false;
      }
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
  return false;
}




bool portSendImpl(Tertiary *p, TaggedRef msg){
  PortMediator *me = static_cast<PortMediator*>(index2Me(p->getTertIndex())); 
  AbstractEntity *ae = me->getAbstractEntity();
  RelaxedMutableAbstractEntity *mae = static_cast<RelaxedMutableAbstractEntity*>(ae);

  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Write(pstout);
  if (pstout != NULL){
    *(pstout) = new PstOutContainer(msg);
  }
  switch(cont)
    {
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
  return false;
}

void cellOperationDoneReadImpl(Tertiary* tert, TaggedRef ans, int thid){
  //  DssThreadId *thrId = reinterpret_cast<DssThreadId*>(thid); 
//    PortMediator *me = static_cast<PortMediator*>(index2Me(tert->getIndex())); 
//    ProxyInterface *pi = me->getProxy();
//    PstOutContainer *load = new PstOutContainer(ans);
//    if (pi->doAbstractOperation(AO_EP_W_DONE,thrId,load) != DSS_SKIP) 
    OZ_error("Unexpected return value from AO_RE_W_DONE"); 
}


void cellOperationDoneWriteImpl(Tertiary* tert )
{
  ; 
}

bool distArrayGetImpl(OzArray *oza, TaggedRef indx, TaggedRef &ans){
  ArrayMediator *me = static_cast<ArrayMediator*>(index2Me(oza->getDist())); 
  AbstractEntity *ae = me->getAbstractEntity();
  MutableAbstractEntity *mae = static_cast<MutableAbstractEntity*>(ae);
  
  DssThreadId *thrId = getThreadId();
    
    PstOutContainerInterface** pstout;
    OpRetVal cont = mae->abstractOperation_Read(thrId,pstout);
    if (pstout != NULL){
    *(pstout) = new PstOutContainer(indx); 
    }
    if (cont == DSS_PROCEED) {
    return false; 
    }
    if (cont == DSS_SUSPEND){
	OZ_Term var = oz_newVariable();
	ans = var; 
	thrId->setThreadMediator(new SuspendedArrayGet(me, ans, tagged2SmallInt(indx))); 
	return true;}
    OZ_error("Shit, something vent wrong when doing an array op");
    return false;
}

bool distArrayPutImpl(OzArray *oza, TaggedRef indx, TaggedRef val){
  ArrayMediator *me = static_cast<ArrayMediator*>(index2Me(oza->getDist())); 
  AbstractEntity *ae = me->getAbstractEntity();
  MutableAbstractEntity *mae = static_cast<MutableAbstractEntity*>(ae);
  
  DssThreadId *thrId = getThreadId();
    
  PstOutContainerInterface** pstout;
  OpRetVal cont = mae->abstractOperation_Write(thrId,pstout);
  if (pstout != NULL){
    *(pstout) = new PstOutContainer(oz_cons(indx,val)); 
  }
  if (cont == DSS_PROCEED) {
    return false; 
  }
  if (cont == DSS_SUSPEND){
    thrId->setThreadMediator(new SuspendedArrayPut(me, val, tagged2SmallInt(indx))); 
    return true;}
  OZ_error("Shit, something vent wrong when doing an array op");
  return false;
}


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



OZ_Return cellAtExchangeImpl(Tertiary* o, TaggedRef fea, TaggedRef val){
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


OZ_Return cellAtAccessImpl(Tertiary* o, TaggedRef fea, TaggedRef val){
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

OZ_Return objectExchangeImpl(Tertiary* o, TaggedRef fea, TaggedRef oVal, TaggedRef nVal){
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
