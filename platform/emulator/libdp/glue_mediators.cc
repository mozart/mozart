/*
 *  Authors:
 *    Erik Klintskog, 2002
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 *    Boriss Mejias (bmc@info.ucl.ac.be)
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
#include "glue_interface.hh"
#include "pstContainer.hh"
#include "value.hh"
#include "controlvar.hh"
#include "thr_int.hh"
#include "glue_faults.hh"
#include "unify.hh"
#include "var_readonly.hh"
#include "cac.hh"

// The identity of the mediators, used for... I dont remeber
// Check it out, Erik. 
static int medIdCntr = 1; 

void doPortSend(OzPort *port, TaggedRef val, Board * home);



/************************* glue_getMediator *************************/

// generic functions to retrieve mediators

template <class M>
inline Mediator* lookupMediator(TaggedRef entity) {
  Mediator* med = mediatorTable->lookup(entity);
  return (med ? med : new M(entity));
}

template <class M>
inline Mediator* getCTWHMediator(ConstTerm* ct) {
  ConstTermWithHome* ctwh = static_cast<ConstTermWithHome*>(ct);
  return (ctwh->isDistributed() ?
	  static_cast<Mediator*>(ctwh->getMediator()) :
	  lookupMediator<M>(makeTaggedConst(ct)));
}



Mediator *glue_getMediator(TaggedRef entity) {
  // assumption: entity is properly deref'd

  if (oz_isRef(entity)) { // entity is a variable
    TaggedRef *vPtr = tagged2Ref(entity);
    OzVariable *var = oz_getNonOptVar(vPtr);
    if (!var->hasMediator()) var->setMediator(new OzVariableMediator(entity));
    return static_cast<Mediator*>(var->getMediator());

  } else { // entity is a const term
    ConstTerm *ct = tagged2Const(entity);

    switch (ct->getType()) {
    case Co_Cell:       return getCTWHMediator<CellMediator>(ct);
    case Co_Object:     return getCTWHMediator<ObjectMediator>(ct);
    case Co_Port:       return getCTWHMediator<PortMediator>(ct);
    case Co_Array:      return getCTWHMediator<ArrayMediator>(ct);
    case Co_Dictionary: return getCTWHMediator<DictionaryMediator>(ct);
    case Co_Lock:       return getCTWHMediator<LockMediator>(ct);

    case Co_Builtin:
      if (static_cast<Builtin*>(ct)->isSited())
	return lookupMediator<UnusableMediator>(entity);
      break;   // other builtins don't have a mediator

    case Co_Extension:
      if (oz_isThread(entity))
	return getCTWHMediator<OzThreadMediator>(ct);

      // fall through, other extensions are unusables
    case Co_Resource:
      return lookupMediator<UnusableMediator>(entity);

    default:
      break;
    }
    OZ_error("No mediator for entity");
    return NULL;
  }
}



/************************* common stuff *************************/

char* ConstMediator::getPrintType(){ return "const";}
char* LazyVarMediator::getPrintType(){ return "lazyVar";}
char* PortMediator::getPrintType(){ return "port";}
char* CellMediator::getPrintType(){ return "cell";}
char* LockMediator::getPrintType(){ return "lock";}
char* ArrayMediator::getPrintType(){ return "array";}
char* DictionaryMediator::getPrintType(){ return "dictionary";}
char* OzThreadMediator::getPrintType() {return "thread";}
char* UnusableMediator::getPrintType(){return "Unusable!!!";}
char* OzVariableMediator::getPrintType(){ return "var";}



//************************** Mediator ***************************//

Mediator::Mediator(TaggedRef ref, GlueTag etype, bool attach) :
  active(TRUE), attached(attach), collected(FALSE),
  dss_gc_status(DSS_GC_NONE), type(etype), faultState(GLUE_FAULT_NONE),
  annotation(emptyAnnotation),
  entity(ref), absEntity(NULL), faultStream(0), faultCtlVar(0), next(NULL)
{
  id = medIdCntr++; 
  mediatorTable->insert(this);
  printf("--- raph: new mediator %p (type=%d)\n", this, type);
}

Mediator::~Mediator(){
  printf("--- raph: remove mediator %p\n", this);
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

// set both links between mediator and abstract entity
void            
Mediator::setAbstractEntity(AbstractEntity *ae) {
  absEntity = ae;
  if (ae) {
    ae->assignMediator(dynamic_cast<MediatorInterface*>(this));
    ae->getCoordinatorAssistant()->setRegisteredFS(FS_PROT_MASK);
  }
}

CoordinatorAssistantInterface* 
Mediator::getCoordinatorAssistant() {
  return absEntity->getCoordinatorAssistant();
}

void
Mediator::completeAnnotation() {
  if (!annotation.pn || !annotation.aa || !annotation.rc) {
    Annotation def = getDefaultAnnotation(getType());
    if (!annotation.pn) annotation.pn = def.pn;
    if (!annotation.aa) annotation.aa = def.aa;
    if (!annotation.rc) annotation.rc = def.rc;
  }
}

void Mediator::gCollect(){
  if (!collected) {
    printf("--- raph: gc %s mediator %p\n", getPrintType(), this);
    collected = TRUE;
    // collect the entity, its fault stream, and control var (if present)
    oz_gCollectTerm(entity, entity);
    oz_gCollectTerm(faultStream, faultStream);
    oz_gCollectTerm(faultCtlVar, faultCtlVar);
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

void
Mediator::setFaultState(GlueFaultState fs) {
  if (faultState != fs) {
    faultState = fs;

    if (fs == GLUE_FAULT_NONE && faultCtlVar) { // wakeup blocked threads
      ControlVarResume(faultCtlVar);
      faultCtlVar = 0;
    }

    if (faultStream) {   // extend fault stream if present
      TaggedRef tail = oz_newReadOnly(oz_rootBoard());
      oz_bindReadOnly(tagged2Ref(faultStream), oz_cons(fsToAtom(fs), tail));
      faultStream = tail;
    }
  }
}

TaggedRef
Mediator::getFaultStream() {
  // create the fault stream if necessary
  if (faultStream == 0) faultStream = oz_newReadOnly(oz_rootBoard());
  return oz_cons(fsToAtom(faultState), faultStream);
}

OZ_Return
Mediator::suspendOnFault() {
  Assert(faultState);
  if (oz_currentThread() != NULL) {
    if (faultCtlVar == 0) faultCtlVar = oz_newVariable(oz_rootBoard());
    return oz_var_addSusp(tagged2Ref(faultCtlVar), oz_currentThread());
  }
  return SUSPEND;
}

void
Mediator::reportFS(const FaultState& fs) {
  if (faultState != GLUE_FAULT_PERM) {
    // determine new fault state
    GlueFaultState s = GLUE_FAULT_NONE;
    if (fs & FS_PROT_STATE_TMP_UNAVAIL) s = GLUE_FAULT_TEMP;
    if (fs & FS_PROT_STATE_PRM_UNAVAIL) s = GLUE_FAULT_PERM;

    setFaultState(s);
  }
}

void
Mediator::print(){
  printf("%s mediator, id %d, ae %x, ref %x, gc(eng:%d dss:%d), con %d\n",
	 getPrintType(), id, absEntity, entity,
	 (int) collected, (int) dss_gc_status, (int) attached);
}



/************************* ConstMediator *************************/

ConstMediator::ConstMediator(TaggedRef t, GlueTag type, bool attach) :
  Mediator(t, type, attach)
{}

ConstTerm* ConstMediator::getConst(){
  return tagged2Const(getEntity()); 
}

void ConstMediator::globalize() {
  Assert(getAbstractEntity() == NULL);

  completeAnnotation();
  setAbstractEntity(dss->m_createMutableAbstractEntity(annotation.pn,
						       annotation.aa,
						       annotation.rc));

  static_cast<ConstTermWithHome*>(getConst())->setMediator((void *)this);
  setAttached(ATTACHED);
}

void ConstMediator::localize(){
  printf("--- raph: localize mediator %p\n", this);
  // We always to keep the mediator, so we remove the abstract entity,
  // detach the mediator, and reinsert the mediator in the table.
  delete absEntity;
  absEntity = NULL;

  static_cast<ConstTermWithHome*>(getConst())->setBoard(oz_currentBoard());
  setAttached(DETACHED);

  mediatorTable->insert(this);
}


/************************* PortMediator *************************/

PortMediator::PortMediator(TaggedRef t) :
  ConstMediator(t, GLUE_PORT, DETACHED)
{}

PortMediator::PortMediator(TaggedRef t, AbstractEntity *ae) :
  ConstMediator(t, GLUE_PORT, ATTACHED)
{
  setAbstractEntity(ae);
}

AOcallback PortMediator::callback_Write(DssThreadId *id, 
					DssOperationId* operation_id,
					PstInContainerInterface* pstin)
{
  PstInContainer *pst = static_cast<PstInContainer*>(pstin); 
  TaggedRef arg =  pst->a_term;
  OzPort *p = tagged2Port(entity);
  doPortSend(p, arg, NULL);
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

void PortMediator::globalize() {
  Assert(getAbstractEntity() == NULL);

  completeAnnotation();
  setAbstractEntity(dss->m_createRelaxedMutableAbstractEntity(annotation.pn,
							      annotation.aa,
							      annotation.rc));

  tagged2Port(entity)->setMediator((void *)this);
  setAttached(ATTACHED);
}



/************************* LazyVarMediator *************************/

LazyVarMediator::LazyVarMediator(TaggedRef t, AbstractEntity *ae) :
  Mediator(t, GLUE_LAZYCOPY, ATTACHED)
{
  setAbstractEntity(ae);
}

void LazyVarMediator::globalize() { Assert(0); }
void LazyVarMediator::localize() { Assert(0); }

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



/************************* UnusableMediator *************************/

UnusableMediator::UnusableMediator(TaggedRef t) :
  Mediator(t, GLUE_UNUSABLE, DETACHED)
{}

UnusableMediator::UnusableMediator(TaggedRef t, AbstractEntity *ae) :
  Mediator(t, GLUE_UNUSABLE, DETACHED)
{
  setAbstractEntity(ae);
}

void UnusableMediator::globalize() {
  Assert(getAbstractEntity() == NULL);

  completeAnnotation();
  setAbstractEntity(dss->m_createImmutableAbstractEntity(annotation.pn,
							 annotation.aa,
							 annotation.rc));
}

void UnusableMediator::localize() {
  // We always to keep the mediator, so we remove the abstract entity,
  // and reinsert the mediator in the table.
  delete absEntity;
  absEntity = NULL;
  mediatorTable->insert(this);
}

AOcallback
UnusableMediator::callback_Read(DssThreadId* id_of_calling_thread,
				DssOperationId* operation_id,
				PstInContainerInterface* operation,
				PstOutContainerInterface*& possible_answer)
{ return AOCB_FINISH;}



/************************* OzThreadMediator *************************/

OzThreadMediator::OzThreadMediator(TaggedRef t) :
  ConstMediator(t, GLUE_THREAD, DETACHED)
{}

OzThreadMediator::OzThreadMediator(TaggedRef t, AbstractEntity *ae) :
  ConstMediator(t, GLUE_THREAD, ATTACHED)
{
  setAbstractEntity(ae);
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

LockMediator::LockMediator(TaggedRef t) :
  ConstMediator(t, GLUE_LOCK, DETACHED)
{}

LockMediator::LockMediator(TaggedRef t, AbstractEntity *ae) :
  ConstMediator(t, GLUE_LOCK, ATTACHED)
{
  setAbstractEntity(ae);
}

AOcallback 
LockMediator::callback_Write(DssThreadId* id_of_calling_thread,
			     DssOperationId* operation_id,
			     PstInContainerInterface* operation,
			     PstOutContainerInterface*& possible_answer)
{
  // Two perations can be done, Lock and Unlock. If a reference to a
  // thread is passed as contents, it is a lock, otherwise an unlock
  OzLock *lock = static_cast<OzLock*>(getConst());
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
  OzLock *lock = static_cast<OzLock*>(getConst());
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
  OzLock *lock = static_cast<OzLock*>(getConst());
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

// use this constructor for annotations, etc.
CellMediator::CellMediator(TaggedRef t) :
  ConstMediator(t, GLUE_CELL, DETACHED)
{}

CellMediator::CellMediator(TaggedRef t, AbstractEntity *ae) :
  ConstMediator(t, GLUE_CELL, ATTACHED)
{
  setAbstractEntity(ae);
}

PstOutContainerInterface *
CellMediator::retrieveEntityRepresentation() {
  OzCell *cell = tagged2Cell(entity);
  TaggedRef out =cell->getValue();
  return new PstOutContainer(out);
}

void 
CellMediator::installEntityRepresentation(PstInContainerInterface* pstIn){
  PstInContainer *pst = static_cast<PstInContainer*>(pstIn); 
  TaggedRef state =  pst->a_term;
  OzCell *cell = tagged2Cell(entity);
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
  OzCell *cell = tagged2Cell(entity);
  TaggedRef out = cell->exchangeValue(arg);
  possible_answer =  new PstOutContainer(out);
  return AOCB_FINISH;
}

AOcallback
CellMediator::callback_Read(DssThreadId *id,
			    DssOperationId* operation_id,
			    PstInContainerInterface* pstin,
			    PstOutContainerInterface*& possible_answer){
  OzCell *cell = tagged2Cell(entity);
  TaggedRef out =cell->getValue();
  possible_answer =  new PstOutContainer(out);
  return AOCB_FINISH;
}



/************************* ObjectMediator *************************/

ObjectMediator::ObjectMediator(TaggedRef t) :
  ConstMediator(t, GLUE_OBJECT, DETACHED)
{}

ObjectMediator::ObjectMediator(TaggedRef t, AbstractEntity *ae) :
  ConstMediator(t, GLUE_OBJECT, ATTACHED)
{
  setAbstractEntity(ae);
}

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

void ObjectMediator::globalize() {
  Assert(getAbstractEntity() == NULL);

  completeAnnotation();
  setAbstractEntity(dss->m_createMutableAbstractEntity(annotation.pn,
						       annotation.aa,
						       annotation.rc));
  //bmc: Maybe two possibilities here. Create a LazyVarMediator first
  //continuing with the approach of marshaling only the stub in the 
  //beginning, or just go eagerly for the object. We are going to try
  //the eager approach first, and then the optimization.
  
  //bmc: Cellify state
  OzObject *o = tagged2Object(entity);
  RecOrCell state = o->getState();
  if (!stateIsCell(state)) {
    SRecord *r = getRecord(state);
    Assert(r != NULL);
    OzCell *cell = tagged2Cell(OZ_newCell(makeTaggedSRecord(r)));
    o->setState(cell);
  }
  
  o->setMediator((void *)this);
  setAttached(ATTACHED);
}

char*
ObjectMediator::getPrintType(){ return NULL; }

PstOutContainerInterface*
ObjectMediator::retrieveEntityRepresentation(){ return NULL; }

void
ObjectMediator::installEntityRepresentation(PstInContainerInterface*){;}



/************************* ArrayMediator *************************/

ArrayMediator::ArrayMediator(TaggedRef t) :
  ConstMediator(t, GLUE_ARRAY, DETACHED)
{}

ArrayMediator::ArrayMediator(TaggedRef t, AbstractEntity *ae) :
  ConstMediator(t, GLUE_ARRAY, ATTACHED)
{
  setAbstractEntity(ae);
}

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
  if (oza->setArg(index,val))
    possible_answer = NULL;
  else
    possible_answer = new PstOutContainer(
          OZ_makeException(E_ERROR, E_KERNEL, "array", 
                           2, makeTaggedConst(oza), index));
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
  int index = tagged2SmallInt(arg);
  TaggedRef out = oza->getArg(index);
  if (out) 
    possible_answer = new PstOutContainer(out);
  else 
    possible_answer = new PstOutContainer(
          OZ_makeException(E_ERROR, E_KERNEL, "array", 
                           2, makeTaggedConst(oza), index));
  return AOCB_FINISH;

}

PstOutContainerInterface*
ArrayMediator::retrieveEntityRepresentation(){
  // raph: the elements are sent in a list (in order)
  OzArray *oza = static_cast<OzArray*>(getConst()); 
  TaggedRef *ar = oza->getRef();
  TaggedRef list = oz_nil();
  for (int i = oza->getWidth()-1; i >= 0; i--) {
    list = oz_cons(ar[i], list);
    ar[i] = makeTaggedSmallInt(0);   // not a great idea...
  }
  return new PstOutContainer(list);
}

void
ArrayMediator::installEntityRepresentation(PstInContainerInterface* pstin){
  // raph: the elements are taken from a list (in order)
  OzArray *oza = static_cast<OzArray*>(getConst()); 
  TaggedRef *ar = oza->getRef();
  int width = oza->getWidth();
  TaggedRef list = static_cast<PstInContainer*>(pstin)->a_term;
  for (int i = 0; i < width; i++) {
    ar[i] = oz_head(list);
    list = oz_tail(list);
  }
  Assert(oz_isNil(list));
}


/************************* DictionaryMediator *************************/

DictionaryMediator::DictionaryMediator(TaggedRef t) :
  ConstMediator(t, GLUE_DICTIONARY, DETACHED)
{}

DictionaryMediator::DictionaryMediator(TaggedRef t, AbstractEntity *ae) :
  ConstMediator(t, GLUE_DICTIONARY, ATTACHED)
{
  setAbstractEntity(ae);
}

AOcallback 
DictionaryMediator::callback_Write(DssThreadId *id,
			      DssOperationId* operation_id,
			      PstInContainerInterface* pstin,
			      PstOutContainerInterface*& possible_answer)
{
  OzDictionary *ozd = tagged2Dictionary(entity);
  TaggedRef arg = static_cast<PstInContainer*>(pstin)->a_term;
  TaggedRef key = OZ_head(arg);
  TaggedRef val = OZ_tail(arg);
  ozd->setArg(key,val);
  possible_answer = NULL;
  return AOCB_FINISH;
}

AOcallback
DictionaryMediator::callback_Read(DssThreadId *id,
			     DssOperationId* operation_id,
			     PstInContainerInterface* pstin,
			     PstOutContainerInterface*& possible_answer)
{
  OzDictionary *ozd = tagged2Dictionary(entity); 
  TaggedRef key = static_cast<PstInContainer*>(pstin)->a_term;
  TaggedRef out = ozd->getArg(key);
  TaggedRef recOut;
  if (out) {
    recOut = OZ_record(OZ_atom("ok"), oz_mklist(makeTaggedSmallInt(1)));
    OZ_putSubtree(recOut, makeTaggedSmallInt(1), out);
  } else {
    recOut = OZ_makeException(E_ERROR, E_KERNEL, "dict",
                              2, makeTaggedConst(ozd), key);
  }
  Assert(oz_isSRecord(recOut));
  possible_answer = new PstOutContainer(recOut);
  return AOCB_FINISH;
}

PstOutContainerInterface*
DictionaryMediator::retrieveEntityRepresentation(){
  // sent the list of entries
  OzDictionary *ozd = tagged2Dictionary(entity);
  return new PstOutContainer(ozd->pairs());
}

void
DictionaryMediator::installEntityRepresentation(PstInContainerInterface* pstin){
  // make sure the dictionary is empty
  OzDictionary *ozd = tagged2Dictionary(entity); 
  ozd->removeAll();
  // insert all entries (not pretty efficient)
  TaggedRef entries = static_cast<PstInContainer*>(pstin)->a_term;
  while (!oz_isNil(entries)) {
    TaggedRef keyval = oz_head(entries);
    ozd->setArg(oz_left(keyval), oz_right(keyval));
    entries = oz_tail(entries);
  }
}


/************************* OzVariableMediator *************************/

// assumption: t is a tagged REF to a tagged VAR.
OzVariableMediator::OzVariableMediator(TaggedRef t) :
  Mediator(t, oz_isFree(*tagged2Ref(t)) ? GLUE_VARIABLE : GLUE_READONLY,
	   ATTACHED)
{}

// assumption: t is a tagged REF to a tagged VAR.
OzVariableMediator::OzVariableMediator(TaggedRef t, AbstractEntity *ae) :
  Mediator(t, oz_isFree(*tagged2Ref(t)) ? GLUE_VARIABLE : GLUE_READONLY,
	   ATTACHED)
{
  setAbstractEntity(ae);
}

void OzVariableMediator::globalize() {
  if (absEntity) return;   // don't globalize twice

  // create abstract entity
  completeAnnotation();
  setAbstractEntity(dss->m_createMonotonicAbstractEntity(annotation.pn,
							 annotation.aa,
							 annotation.rc));
}

void OzVariableMediator::localize() {
  // In any case, we keep the mediator.  So remove the abstract
  // entity, and keep the mediator in the table
  delete absEntity;
  absEntity = NULL;
  mediatorTable->insert(this);
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
