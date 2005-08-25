/*
 *  Authors:
 *    Erik Klintskog, 2002
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
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
#pragma implementation "glue_mediators.hh"
#endif

#include "glue_mediators.hh"
#include "glue_tables.hh"
//#include "dss_interface.hh"
#include "pstContainer.hh"
#include "value.hh"
#include "thr_int.hh"
#include "glue_faults.hh"
#include "unify.hh"
#include "cac.hh"


// The identity of the mediators, used for... I dont remeber
// Check it out, Erik. 
static int medIdCntr = 1; 

void doPortSend(PortWithStream *port,TaggedRef val,Board * home);


char* ConstMediator::getPrintType(){ return "const";}
char* LazyVarMediator::getPrintType(){ return "lazyVar";}
char* PortMediator::getPrintType(){ return "port";}
char* CellMediator::getPrintType(){ return "cell";}
char* LockMediator::getPrintType(){ return "lock";}
char* VarMediator::getPrintType(){ return "var";}
char* ArrayMediator::getPrintType(){ return "array";}
char* OzThreadMediator::getPrintType() {return "thread";}
char* UnusableMediator::getPrintType(){return "Unusable!!!";}
char* OzVariableMediator::getPrintType(){ return "var";}



//************************** Mediator ***************************//

Mediator::Mediator(AbstractEntity *ae, TaggedRef ref, bool attach) :
  active(TRUE), attached(attach), collected(FALSE),
  dss_gc_status(DSS_GC_NONE), annotation(0),
  entity(ref), absEntity(ae), faultStream(0), next(NULL)
{
  id = medIdCntr++; 
  mediatorTable->insert(this);
  // apparently this does not work...
  // if (ae) ae->assignMediator(reinterpret_cast<MediatorInterface*>(this));
}

Mediator::~Mediator(){
  // ERIK, removed delete during reconstruction
  // We keep the tight 1:1 mapping and remove the abstract entity
  // along with the Mediator.
  if (absEntity) delete absEntity;
#ifdef INTERFACE
  // Nullify the pointers in debug mode
  absEntity = NULL;
  next = NULL;
#endif
}

void
Mediator::makePassive() {
  Assert(attached);
  active = FALSE;
}

void
Mediator::setEntity(TaggedRef ref) {
  entity = ref;
}

TaggedRef
Mediator::getEntity() {
  return entity;
}

AbstractEntity *
Mediator::getAbstractEntity() { 
  return absEntity;
}

void            
Mediator::setAbstractEntity(AbstractEntity *ae) {
  absEntity = ae;
}

CoordinatorAssistantInterface* 
Mediator::getCoordinatorAssistant() {
  return absEntity->getCoordinatorAssistant();
}

void Mediator::gCollect(){
  if (!collected) {
    printf("--- raph: Mediator::gCollect %d\n", id);
    collected = TRUE;
    
    // collect the entity and its fault stream (if present)
    oz_gCollectTerm(entity, entity);
    if (faultStream) oz_gCollectTerm(faultStream, faultStream);
  }
}

void
Mediator::checkGCollect() {
  // if mediator is detached, check its term
  if (!collected && !attached && isGCMarkedTerm(entity)) gCollect();
}

void
Mediator::resetGCStatus() {
  collected = FALSE;
  dss_gc_status = DSS_GC_NONE;
}

DSS_GC
Mediator::getDssGCStatus() {
  Assert(dss_gc_status == DSS_GC_NONE);
  if (absEntity)
    dss_gc_status = absEntity->getCoordinatorAssistant()->getDssDGCStatus();
  return dss_gc_status;
}

TaggedRef
Mediator::getFaultStream() {
  // create the fault stream, if necessary
  if (faultStream == 0) reportFaultState(FS_NO_FAULT);
  return faultStream;
}

void Mediator::reportFaultState(const FaultState& fs) {
  // determine fault description
  TaggedRef state;
  switch (fs) {
  case FS_NO_FAULT:               state = AtomNormal;
  case FS_AA_HOME_TMP_UNAVAIL:
  case FS_AA_HOME_PRM_UNAVAIL:
  case FS_AA_HOME_REMOVED:
    OZ_error("Dunno what to do with these FS_AA_HOME_*");
  case FS_PROT_STATE_TMP_UNAVAIL: state = AtomTempFail;
  case FS_PROT_STATE_PRM_UNAVAIL: state = AtomPermFail;
  default: Assert(0);
  }

  // add state to the stream
  TaggedRef newStream = oz_cons(state, oz_newReadOnly(oz_rootBoard()));
  if (faultStream) {
    TaggedRef *vptr = tagged2Ref(oz_tail(faultStream));
    Assert(oz_isVar(*vptr));
    oz_bindReadOnly(vptr, newStream);
  }
  faultStream = newStream;
}

void
Mediator::print(){
  printf("%s mediator, id %d, ae %x, ref %x, gc(eng:%d dss:%d), con %d\n",
	 getPrintType(), id, absEntity, entity,
	 (int) collected, (int) dss_gc_status, (int) attached);
}



/************************* ConstMediator *************************/

ConstMediator::ConstMediator(AbstractEntity *ae, ConstTerm *t) :
  Mediator(ae, makeTaggedConst(t), TRUE)
{}

ConstTerm* ConstMediator::getConst(){
  return tagged2Const(getEntity()); 
}



/************************* PortMediator *************************/

PortMediator::PortMediator(AbstractEntity *ae, Tertiary *t) :
  ConstMediator(ae, t) {}

AOcallback PortMediator::callback_Write(DssThreadId *id, 
					DssOperationId* operation_id,
					PstInContainerInterface* pstin)
{
  PstInContainer *pst = static_cast<PstInContainer*>(pstin); 
  TaggedRef arg =  pst->a_term;
  PortWithStream *p = static_cast<PortWithStream*>(getConst());
  doPortSend(p,arg,NULL);
  return AOCB_FINISH; 
}

AOcallback
PortMediator::callback_Read(DssThreadId *id,
			    DssOperationId* operation_id,
			    PstInContainerInterface* pstin,
			    PstOutContainerInterface*& possible_answer)
{
  Assert(0); 
  return AOCB_FINISH; 
}

// This is code for extracting and installing the state of a port...
// 
// We'll put a fresh variable in the stream. This stream is not to be 
// used any more, its just a placeholder. 
//{
//  TaggedRef out = p->exchangeStream(oz_newFuture(oz_currentBoard()));
//    PstOutContainer *c = new PstOutContainer(out);
//    c->a_pushContents = TRUE;
//    return c; 
//  }
//
//{
//    (void)  p->exchangeStream(arg); 
// }

void PortMediator::localize(){
  Tertiary *t = static_cast<Tertiary*>(getConst());
  t->setTertType(Te_Local);
  t->setBoard(am.currentBoard());
}



/************************* LazyVarMediator *************************/

LazyVarMediator::LazyVarMediator(AbstractEntity *ae, TaggedRef t) :
  Mediator(ae, t, TRUE) {}

void LazyVarMediator::localize(){
  ((Object*)tagged2Const(getEntity()))->localize();
}

/*
PstOutContainerInterface* LazyVarMediator::DOE(const AbsOp& aop, DssThreadId * id, PstInContainerInterface* trav){ 
  PstInContainer *pst = static_cast<PstInContainer*>(trav); 
  TaggedRef arg =  pst->a_term;
  switch (aop){ 
  case AO_LZ_FETCH:
    {
      OZ_Term t = getEntity();
      TaggedRef out; 
      if(OZ_boolToC(arg))
	out=OZ_pair2( oz_nil(),t);
      else
	{
	  Object *o = (Object *) tagged2Const(t);
	  out = OZ_pair2(o->getClassTerm(),t);
	}
      PstOutContainer *pst = new PstOutContainer(out);
      pst->a_fullTopTerm = TRUE;
      return pst; 
    }
  default:
    {
      OZ_error("Unknown AOP:%d for CM_LAZY_VAR",aop); 
    }
  }
  return NULL; 
}
*/

/*

WakeRetVal LazyVarMediator::resumeFunctionalThread(DssThreadId * id, PstInContainerInterface* trav){ 
  PstInContainer *tc = static_cast<PstInContainer*>(trav);
  TaggedRef t = tc->a_term; 
  Assert(oz_isTuple(t));
  TaggedRef to = oz_deref(OZ_getArg(t,1));
  Object *o = (Object *) tagged2Const(to);
  
  OZ_Term cl = oz_deref(getEntity());
  ObjectVar *var = (ObjectVar *)tagged2Var(cl);
  
  // Second, fix the variable binding
  var->transfer(to,  &cl);
  // ERIK, I think we are lacking a bind here...
  return WRV_OK;
}

*/



/************************* VarMediator *************************/

VarMediator::VarMediator(AbstractEntity *ae, TaggedRef t) :
  Mediator(ae, t, TRUE)
{}

void VarMediator::localize(){
  OZ_warning("Localizing of var is disabled, %d\n", id);
}

PstOutContainerInterface *VarMediator::retrieveEntityRepresentation(){
  return new PstOutContainer(getEntity());
}

void VarMediator::installEntityRepresentation(PstInContainerInterface* pstin){
  PstInContainer *pst = static_cast<PstInContainer*>(pstin); 
  TaggedRef var = getEntity();
  TaggedRef arg =  pst->a_term;
  // raph: don't do this at home, kids!
  ExtVar* ev = oz_getExtVar(*(TaggedRef *)var);
  oz_bindLocalVar( extVar2Var(ev), (TaggedRef *)var,arg);
  makePassive();
}

AOcallback
VarMediator::callback_Bind(DssOperationId *id,
			   PstInContainerInterface* pstin)
{
  PstInContainer *pst = static_cast<PstInContainer*>(pstin); 
  TaggedRef var = getEntity();
  TaggedRef arg = pst->a_term;
  // raph: don't do this at home, kids!
  ExtVar *ev = oz_getExtVar(*(TaggedRef *)var);
  oz_bindLocalVar( extVar2Var(ev), (TaggedRef *)var,arg);
  makePassive();
  return AOCB_FINISH;
}

AOcallback
VarMediator::callback_Append(DssOperationId *id,
			     PstInContainerInterface* pstin)
{
  Assert(0);
  return AOCB_FINISH; 
}



/************************* UnusableMediator *************************/
UnusableMediator::UnusableMediator(AbstractEntity *ae, TaggedRef t) :
  Mediator(ae, t, FALSE) {}

UnusableMediator::~UnusableMediator() {
  printf("--- raph: delete UnusableMediator\n");
}

void UnusableMediator::localize() {}

AOcallback
UnusableMediator::callback_Read(DssThreadId* id_of_calling_thread,
				DssOperationId* operation_id,
				PstInContainerInterface* operation,
				PstOutContainerInterface*& possible_answer)
{ return AOCB_FINISH;}



/************************* OzThreadMediator *************************/

OzThreadMediator::OzThreadMediator(AbstractEntity *ae, TaggedRef t) :
  Mediator(ae, t, TRUE) {}

void OzThreadMediator::localize(){
  // check that we are a proper version...
  ;
}

AOcallback
OzThreadMediator::callback_Write(DssThreadId *id, 
				 DssOperationId* operation_id,
				 PstInContainerInterface* pstin,
				 PstOutContainerInterface*& possible_answer)
{
  return AOCB_FINISH;
}

AOcallback
OzThreadMediator::callback_Read(DssThreadId *id,
				DssOperationId* operation_id,
				PstInContainerInterface* pstin,
				PstOutContainerInterface*& possible_answer)
{
  return AOCB_FINISH;
}

PstOutContainerInterface *
OzThreadMediator::retrieveEntityRepresentation(){
  Assert(0); 
  return NULL;
}

void 
OzThreadMediator::installEntityRepresentation(PstInContainerInterface*){
  Assert(0); 
}



/************************* LockMediator *************************/

LockMediator::LockMediator(AbstractEntity *ae, Tertiary *t) :
  ConstMediator(ae, t) {}

void LockMediator::localize(){
  OZ_warning("localizing lock");
}

AOcallback 
LockMediator::callback_Write(DssThreadId* id_of_calling_thread,
			     DssOperationId* operation_id,
			     PstInContainerInterface* operation,
			     PstOutContainerInterface*& possible_answer)
{
  // Two perations can be done, Lock and Unlock. If a reference to a
  // thread is passed as contents, it is a lock, otherwise an unlock
  LockLocal *lock = static_cast<LockLocal*>(getConst());
  if(operation !=  NULL) {
    /////// LOCK /////////////
    PstInContainer *pst = static_cast<PstInContainer*>(operation); 
    TaggedRef ozThread =  pst->a_term;
    
    Thread *dummyThread = oz_ThreadToC(ozThread);
    if (lock->getLocker() == NULL){
      lock->lockB(dummyThread); 
      possible_answer = new PstOutContainer(oz_cons(oz_int(2),oz_nil())); 
      return AOCB_FINISH;
  }
    if( lock->getLocker() == dummyThread){
      lock->lockB(dummyThread); 
      possible_answer = new PstOutContainer(oz_cons(oz_int(1),oz_nil())); 
      return AOCB_FINISH;
    }
    // Hua, breaking all the abstractions... But if things are implemented
    // so non-general as the locks, what can we do? 
    TaggedRef var = oz_newVariable(oz_currentBoard());	
    // add pendthread to pending list. 
    
    PendThread **pt = lock->getPendBase(); 
    while(*pt!=NULL){pt= &((*pt)->next);}
    *pt = new PendThread(ozThread, NULL,  var); 
    
    possible_answer = new PstOutContainer(oz_cons(oz_int(3),oz_cons(var, oz_nil()))); 
    return AOCB_FINISH;
  }
  else{
    /////// UNLOCK /////////////
    lock->unlock(); 
    possible_answer = NULL; 
    return AOCB_FINISH;
  }
}

AOcallback 
LockMediator::callback_Read(DssThreadId* id_of_calling_thread,
			    DssOperationId* operation_id,
			    PstInContainerInterface* operation,
			    PstOutContainerInterface*& possible_answer){
  Assert(0); 
  return AOCB_FINISH;
}

PstOutContainerInterface *
LockMediator::retrieveEntityRepresentation(){
  LockLocal *lock = static_cast<LockLocal*>(getConst());
  if (lock->getLocker() == NULL)
    return NULL; 
  
  TaggedRef pList = oz_nil(); 
  TaggedRef locker =  oz_pair2(oz_thread(lock->getLocker()), oz_int(lock->getRelocks()));
  PendThread *pt = lock->getPending();
  
  while (pt){
    PendThread *tmp = pt->next;
    pList = oz_cons(oz_pair2(pt->oThread, pt->controlvar), pList); 
    pt->dispose(); 
    pt = tmp; 
  }
  // ERIK, this should actually NOT be done. It is first
  // when the proxy is defined to be a skeleton that those 
  // can be removed. 
  TaggedRef strct = oz_pair2(locker, pList);
  lock->setPending(NULL); 
  lock->setLocker(NULL); 
  lock->setRelocks(0); 
  return (new PstOutContainer(strct));
}

void 
LockMediator::installEntityRepresentation(PstInContainerInterface* pstIn){
  LockLocal *lock = static_cast<LockLocal*>(getConst());
  if (pstIn == NULL)
    {
      lock->setPending(NULL); 
      lock->setLocker(NULL); 
      lock->setRelocks(0); 
    }
  else
    {
      PstInContainer *pst = static_cast<PstInContainer*>(pstIn); 
      TaggedRef strckt = pst->a_term; 
      lock->setLocker(oz_ThreadToC(oz_left(oz_left(strckt))));
      lock->setRelocks(OZ_intToC(oz_right(oz_left(strckt))));
      TaggedRef lst = oz_right(strckt); 
      PendThread *pending = NULL; 
      while(!oz_eq(lst, oz_nil()))
	{
	  TaggedRef ele = oz_head(lst); 
	  pending = new PendThread(oz_left(ele), pending, oz_right(ele)); 
	  lst = oz_tail(lst); 
	}
      lock->setPending(pending); 
    }
}


 
/************************* CellMediator *************************/

extern TaggedRef BI_remoteExecDone; 

CellMediator::CellMediator(AbstractEntity *ae, Tertiary *t) :
 ConstMediator(ae, t) {}

void CellMediator::localize(){
  OZ_warning("localizing cell");
}

PstOutContainerInterface *
CellMediator::retrieveEntityRepresentation(){
  CellLocal *cell = static_cast<CellLocal*>(getConst());
  TaggedRef out =cell->getValue();
  return new PstOutContainer(out);
}

void 
CellMediator::installEntityRepresentation(PstInContainerInterface* pstIn){
  PstInContainer *pst = static_cast<PstInContainer*>(pstIn); 
  TaggedRef state =  pst->a_term;
  CellLocal *cell = static_cast<CellLocal*>(getConst());
  cell->setValue(state); 
}

AOcallback
CellMediator::callback_Write(DssThreadId *id,
			     DssOperationId* operation_id,
			     PstInContainerInterface* pstin,
			     PstOutContainerInterface*& possible_answer){
  TaggedRef arg;
  if(pstin !=  NULL) {
    PstInContainer *pst = static_cast<PstInContainer*>(pstin); 
    arg =  pst->a_term;
  }
  CellLocal *cell = static_cast<CellLocal*>(getConst());
  TaggedRef out = cell->exchangeValue(arg);
  possible_answer =  new PstOutContainer(out);
  return AOCB_FINISH;
}

AOcallback
CellMediator::callback_Read(DssThreadId *id,
			    DssOperationId* operation_id,
			    PstInContainerInterface* pstin,
			    PstOutContainerInterface*& possible_answer){
  CellLocal *cell = static_cast<CellLocal*>(getConst());
  TaggedRef out =cell->getValue();
  possible_answer =  new PstOutContainer(out);
  return AOCB_FINISH;
}



/************************* ObjectMediator *************************/

ObjectMediator::ObjectMediator(AbstractEntity *ae, Tertiary *t) :
  ConstMediator(ae, t) {}

AOcallback
ObjectMediator::callback_Write(DssThreadId* id_of_calling_thread,
			       DssOperationId* operation_id,
			       PstInContainerInterface* operation,
			       PstOutContainerInterface*& possible_answer){
  return AOCB_FINISH;
}

AOcallback
ObjectMediator::callback_Read(DssThreadId* id_of_calling_thread,
       DssOperationId* operation_id,
       PstInContainerInterface* operation,
       PstOutContainerInterface*& possible_answer){
  return AOCB_FINISH;
}

void
ObjectMediator::localize() {}

char*
ObjectMediator::getPrintType(){ return NULL; }

PstOutContainerInterface*
ObjectMediator::retrieveEntityRepresentation(){ return NULL; }

void
ObjectMediator::installEntityRepresentation(PstInContainerInterface*){;}



/************************* ArrayMediator *************************/

ArrayMediator::ArrayMediator(AbstractEntity *ae, ConstTerm *t) :
  ConstMediator(ae, t) {}
  
void
ArrayMediator::globalize() {
///////////////////////////////////////////////////////77
/*  OzArray *ozA = static_cast<OzArray*>(arrayConst);
  AbstractEntity *ae;
  if(!ozA->isDistributed()) {
    //bmc: Shouldn't we call a globalize function here??
    printf( "new array "); 
    ProtocolName prot; 
    AccessArchitecture aa; 
    RCalg gc;
    getAnnotations(makeTaggedConst(arrayConst), PN_SIMPLE_CHANNEL, AA_STATIONARY_MANAGER ,RC_ALG_WRC, prot, aa, gc);
    
    ae = dss->m_createMutableAbstractEntity(prot,aa,gc);

    ArrayMediator *am = new ArrayMediator(ae,arrayConst);
    Mediator *me = am; 
    ae->assignMediator(am); 
    
    arrayConst->setDist(reinterpret_cast<int>(me));
    Assert(ozA->isDistributed());
    Assert(me = index2Me(ozA->getDist())); 
  }
  else
    ae=index2AE(ozA->getDist());
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs); 
  ae->getCoordinatorAssistant()->marshal(gwb, PMF_ORDINARY);
  bs->put(DSS_DIF_ARRAY);
  marshalNumber(bs, ozA->getLow());;
  marshalNumber(bs, ozA->getHigh());;
///////////////////////////////////////////////////////77
*/
  Assert(getAbstractEntity() == NULL);
/*  
  ProtocolName pn = annotation;
  AccessArchitecture aa;
  RCalg gc_alg;
  int annotation = 0;
  getAnnotation(tr, annotation); 
  g = ((annotation & RC_ALG_MASK)) ? annotation & RC_ALG_MASK : g_def;
  p = static_cast<ProtocolName>(((annotation & ANOT_PROT_MASK)) ? annotation & ANOT_PROT_MASK : p_def);
  a = static_cast<AccessArchitecture>((annotation & ANOT_AA_MASK) ? annotation & ANOT_AA_MASK : a_def);
  */
}

void
ArrayMediator::localize() {;}

AOcallback 
ArrayMediator::callback_Write(DssThreadId *id,
			      DssOperationId* operation_id,
			      PstInContainerInterface* pstin,
			      PstOutContainerInterface*& possible_answer)
{
  OzArray *oza = static_cast<OzArray*>(getConst());
  TaggedRef arg = static_cast<PstInContainer*>(pstin)->a_term;
  int index = tagged2SmallInt(OZ_head(arg));
  TaggedRef val = OZ_tail(arg);
  oza->setArg(index,val);
  possible_answer = NULL;
  return AOCB_FINISH;
}

AOcallback
ArrayMediator::callback_Read(DssThreadId *id,
			     DssOperationId* operation_id,
			     PstInContainerInterface* pstin,
			     PstOutContainerInterface*& possible_answer)
{
  OzArray *oza = static_cast<OzArray*>(getConst()); 
  TaggedRef arg = static_cast<PstInContainer*>(pstin)->a_term;
  int indx = tagged2SmallInt(arg);
  possible_answer =  new PstOutContainer(oza->getArg(indx));
  return AOCB_FINISH;
}

void
ArrayMediator::installEntityRepresentation(PstInContainerInterface* pstin){
  OzArray *oza = static_cast<OzArray*>(getConst()); 
  TaggedRef arg = static_cast<PstInContainer*>(pstin)->a_term;
  int width = oza->getWidth();
  TaggedRef *ar = oza->getRef();
  for(int i=width - 1; i>=0; i--){
    TaggedRef el = oz_head(arg); 
    arg = oz_tail(arg); 
    ar[i] = el; 
  }
}

PstOutContainerInterface*
ArrayMediator::retrieveEntityRepresentation(){
  OzArray *oza = static_cast<OzArray*>(getConst()); 
  TaggedRef *ar = oza->getRef();
  int width = oza->getWidth();
  TaggedRef list = oz_nil();
  TaggedRef int_0 = makeTaggedSmallInt(0); 
  for(int i=0; i<width; i++){
    list = oz_cons(ar[i],list); 
    ar[i] = int_0; 
  }
  return new PstOutContainer(list);
}



/************************* OzVariableMediator *************************/

// assumption: t is a tagged REF to a tagged VAR.
OzVariableMediator::OzVariableMediator(AbstractEntity *ae, TaggedRef t) :
  Mediator(ae, t, TRUE), patchCount(0)
{}

void OzVariableMediator::incPatchCount() { patchCount++; }
void OzVariableMediator::decPatchCount() { patchCount--; }

void OzVariableMediator::localize() {
  OZ_warning("Localizing of var is disabled, %d\n", id);
}

PstOutContainerInterface *OzVariableMediator::retrieveEntityRepresentation(){
  printf("--- raph: retrieveEntityRepresentation %x\n", getEntity());
  return new PstOutContainer(getEntity());
}

void OzVariableMediator::installEntityRepresentation(PstInContainerInterface* pstin){
  printf("--- raph: installEntityRepresentation %x\n", getEntity());
  Assert(active);
  PstInContainer *pst = static_cast<PstInContainer*>(pstin); 
  TaggedRef* ref = tagged2Ref(getEntity()); // points to the var's tagged ref
  OzVariable* ov = tagged2Var(*ref);
  TaggedRef  arg = pst->a_term;
  
  oz_bindLocalVar(ov, ref, arg);
  makePassive();
}

AOcallback
OzVariableMediator::callback_Bind(DssOperationId *id,
				  PstInContainerInterface* pstin) {
  printf("--- raph: callback_Bind %x\n", getEntity());
  Assert(active);
  PstInContainer *pst = static_cast<PstInContainer*>(pstin); 
  TaggedRef* ref = tagged2Ref(getEntity()); // points to the var's tagged ref
  OzVariable* ov = tagged2Var(*ref);
  TaggedRef  arg = pst->a_term;
  
  oz_bindLocalVar(ov, ref, arg);
  makePassive();
  return AOCB_FINISH;
}

AOcallback
OzVariableMediator::callback_Append(DssOperationId *id,
				    PstInContainerInterface* pstin) {
  printf("--- raph: callback_Append %x\n", getEntity());

  // raph: The variable may have been bound at this point.  This can
  // happen when two operations Bind and Append are done concurrently
  // (a "feature" of the dss).  Therefore we check the type first.
  if (active) {
    // check pstin
    if (pstin !=  NULL) {
      PstInContainer *pst = static_cast<PstInContainer*>(pstin);
      TaggedRef msg = pst->a_term;
      Assert(msg == oz_atom("needed"));
    }
    TaggedRef* ref = tagged2Ref(getEntity()); // points to the tagged ref
    oz_var_makeNeededLocal(ref);
  }
  return AOCB_FINISH; 
}
