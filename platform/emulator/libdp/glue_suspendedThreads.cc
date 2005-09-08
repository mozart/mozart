#if defined(INTERFACE)
#pragma implementation "glue_suspendedThreads.hh"
#endif


#include "glue_suspendedThreads.hh"
#include "value.hh"
#include "controlvar.hh"
#include "pstContainer.hh"

static SuspendedThread* g_suspenedThreadList = NULL; 

// FORWARDERS....
void doPortSend(PortWithStream *port,TaggedRef val,Board * home);
OZ_Return accessCell(OZ_Term cell,OZ_Term &out);


void gCollectSuspThreads(){
  SuspendedThread* tmp, **ptr = &g_suspenedThreadList; 
  while(*ptr){
    if ((*ptr)->gCollect()){
      ptr = &(*ptr)->a_next; 
      
    }
    else
      {
	tmp = *ptr;
	*ptr = tmp->a_next; 
	delete tmp; 
      }
  }
  
}

SuspendedThread::SuspendedThread(Mediator *m){
  a_next = g_suspenedThreadList;
  g_suspenedThreadList = this; 
  a_mediator = m; 
}



OZ_Return SuspendedThread::suspend()
{
  ControlVarNew(cv,oz_rootBoard());  
  a_cntrlVar = cv; 
  return suspendOnControlVar();
}


OZ_Return SuspendedThread::resume(){
  ControlVarResume(a_cntrlVar);
  // I assume that this is for debug purposes, Erik
  a_cntrlVar = static_cast<OZ_Term>(0);
  return PROCEED; 
}

bool SuspendedThread::gc(){
  if (a_cntrlVar == static_cast<OZ_Term>(0))
    return false; 
  else{
    oz_gCollectTerm(a_cntrlVar, a_cntrlVar);
    return true; 
  }
}

SuspendedVarBind::SuspendedVarBind(OZ_Term val, Mediator *med):SuspendedThread(med), a_val(val)
{ 
  // Erik, this is a suboptimal solution, we'll have to fixit right
  // later. The variable has its own suspension and does not relay 
  // on the the cntrl-vars. However, If things where to be done 
  // correctly, we should _not_ suspend on the variable body, but on
  // the cntrl strucure. 
}

WakeRetVal SuspendedVarBind::resumeDoLocal(DssOperationId*){
  a_val = (TaggedRef)0;  
  return WRV_DONE;
}
WakeRetVal SuspendedVarBind::resumeRemoteDone(PstInContainerInterface* pstin){
  a_val = (TaggedRef)0;  
  return WRV_DONE;
  
}

SuspendedCellAccess::SuspendedCellAccess(Mediator* med, OZ_Term var):
  SuspendedThread(med),
  a_var(var)
{
  suspend();
}

WakeRetVal SuspendedCellAccess::resumeDoLocal(DssOperationId*){
  CellMediator *pM = static_cast<CellMediator*>(a_mediator);
  OzCell *cell     = static_cast<OzCell*>(pM->getConst());
  OZ_Term contents = cell->getValue();
  oz_unify(a_var,contents);
  resume();
  return WRV_DONE; 
}
WakeRetVal SuspendedCellAccess::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer *pst = static_cast<PstInContainer*>(pstin);
  oz_unify(a_var, pst->a_term); 
  resume();
  return WRV_DONE; 
}

SuspendedCellExchange::SuspendedCellExchange(Mediator* med, OZ_Term newVal, OZ_Term ans):
  SuspendedThread(med), 
  a_newVal(newVal), 
  a_var(ans)
{
  suspend();
}

WakeRetVal SuspendedCellExchange::resumeDoLocal(DssOperationId*){
  CellMediator *pM = static_cast<CellMediator*>(a_mediator);
  OzCell *cell  = static_cast<OzCell*>(pM->getConst());
  OZ_Term contents = cell->exchangeValue(a_newVal); 
  oz_unify(a_var,contents);
  resume();
  return WRV_DONE; 
}
WakeRetVal SuspendedCellExchange::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer *pst = static_cast<PstInContainer*>(pstin);
  oz_unify(a_var, pst->a_term); 
  resume(); 
  return WRV_DONE; 
}



bool SuspendedCellExchange::gCollect(){
  if (gc())
    {
      oz_gCollectTerm(a_var,a_var); 
      oz_gCollectTerm(a_newVal,a_newVal); 
      return true; 
    }
  else
    return false; 
}




bool SuspendedCellAccess::gCollect(){
  if (gc())
    {
      oz_gCollectTerm(a_var,a_var); 
      return true; 
    }
  else
    return false; 
}



bool SuspendedVarBind::gCollect(){
  if (a_val != (TaggedRef) 0)
    {
      oz_gCollectTerm(a_val,a_val); 
      return true; 
    }
  else
    return false; 
  
}

SuspendedLockTake::SuspendedLockTake(Mediator* med, TaggedRef tr):SuspendedThread(med), a_ozThread(tr)
{
  suspend();
}

WakeRetVal 
SuspendedLockTake::resumeDoLocal(DssOperationId*){
  LockLocal *lck = static_cast<LockLocal*>(static_cast<ConstMediator*>(getMediator())->getConst());
  Thread *theThread = oz_ThreadToC( a_ozThread);
  if (lck->getLocker() == NULL || lck->getLocker() == theThread){
    lck->lockB(theThread);
    resume();
  }
  else
    {
      PendThread **pt = lck->getPendBase(); 
      while(*pt!=NULL){pt= &((*pt)->next);}
      *pt = new PendThread(a_ozThread, NULL,  a_cntrlVar);
      a_cntrlVar = 0; 
      
    }
  return WRV_DONE; 
}
WakeRetVal 
SuspendedLockTake::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer *pst = static_cast<PstInContainer*>(pstin);
  TaggedRef lst = pst->a_term;
  
  OzLock *lck = static_cast<OzLock*>(static_cast<ConstMediator*>(getMediator())->getConst());
  switch(oz_intToC(oz_head(lst))){
  case 1:
      // We had it, remove the unlock stackframe! 
      // Remove the darn 
    // Fall through!
  case 2:
    {
      resume(); 
      return WRV_DONE; 
    }
  case 3:
    oz_unify(oz_head(oz_tail(lst)), a_cntrlVar);
    break; 
  }
  
}
bool 
SuspendedLockTake::gCollect(){
  oz_gCollectTerm(a_ozThread, a_ozThread); 
  return gc();
}

SuspendedLockRelease::SuspendedLockRelease(Mediator* med):SuspendedThread(med), used(true){;}

WakeRetVal 
SuspendedLockRelease::resumeDoLocal(DssOperationId*){
  LockLocal *lck = static_cast<LockLocal*>(static_cast<ConstMediator*>(getMediator())->getConst());
  lck->unlock();
  used = false; 
  return WRV_DONE;
}


WakeRetVal 
SuspendedLockRelease::resumeRemoteDone(PstInContainerInterface* pstin){
  used = false; 
  return WRV_DONE;
}
bool 
SuspendedLockRelease::gCollect(){
  return used; 
}

SuspendedArrayPut::SuspendedArrayPut(Mediator* med, OZ_Term val, int indx):
  SuspendedThread(med), 
  a_val(val), a_indx(indx)
{
  suspend();
}

WakeRetVal SuspendedArrayPut::resumeDoLocal(DssOperationId*){
  ArrayMediator *pM = static_cast<ArrayMediator*>(a_mediator);
  OzArray*oza     = static_cast<OzArray*>(pM->getConst());  
  TaggedRef *ar = oza->getRef();
  ar[a_indx] = a_val; 
  resume(); 
  return WRV_DONE; 
}
WakeRetVal SuspendedArrayPut::resumeRemoteDone(PstInContainerInterface* pstin){
  resume(); 
  return WRV_DONE; 
}
bool SuspendedArrayPut::gCollect(){
  oz_gCollectTerm(a_val, a_val); 
  return gc();
}



SuspendedArrayGet::SuspendedArrayGet(Mediator* med, OZ_Term var, int indx):
  SuspendedThread(med),
  a_var(var), a_indx(indx)
{
  suspend();
}

WakeRetVal SuspendedArrayGet::resumeDoLocal(DssOperationId*){
  ArrayMediator *pM = static_cast<ArrayMediator*>(a_mediator);
  OzArray*oza     = static_cast<OzArray*>(pM->getConst()); 
  TaggedRef *ar = oza->getRef();
  oz_unify(a_var,ar[a_indx]);
  resume(); 
  return WRV_DONE; 
}
WakeRetVal SuspendedArrayGet::resumeRemoteDone(PstInContainerInterface* pstin){
  PstInContainer *pst = static_cast<PstInContainer*>(pstin);
  oz_unify(a_var, pst->a_term); 
  resume(); 
  return WRV_DONE; 
}
bool SuspendedArrayGet::gCollect(){
  oz_gCollectTerm(a_var, a_var); 
  return gc();
}

/*

ReplaceFrame(frame,pc,y,cap)      

pushFrame(C_LOCK_Ptr, 0, lck); 


*/




