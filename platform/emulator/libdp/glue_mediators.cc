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
#include "thr_int.hh"
#include "glue_faults.hh"
#include "unify.hh"
#include "cac.hh"

// The identity of the mediators, used for... I dont remeber
// Check it out, Erik. 
static int medIdCntr = 1; 

void doPortSend(OzPort *port, TaggedRef val, Board * home);
void oz_thread_setDistVal(TaggedRef tr, int i, void* v); 
void* oz_thread_getDistVal(TaggedRef tr, int i);



/************************* glue_getMediator *************************/

// special cases for glue_getMediator
template <class EntityMediator>
inline Mediator *getCTWHMediator(ConstTerm* ct) {
  ConstTermWithHome *ctwh = static_cast<ConstTermWithHome*>(ct);
  Mediator *med = (ctwh->isDistributed() ?
		   static_cast<Mediator*>(ctwh->getMediator()) :
		   mediatorTable->lookup(makeTaggedConst(ct)));
  return (med ? med : new EntityMediator(ct));
}

inline Mediator *getUnusableMediator(TaggedRef entity) {
  Mediator *med = mediatorTable->lookup(entity);
  return (med ? med : new UnusableMediator(entity));
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
	return getUnusableMediator(entity);
      break;   // other builtins don't have a mediator

    case Co_Extension:
      if (oz_isThread(entity)) {
	Mediator *med =
	  reinterpret_cast<Mediator*>(oz_thread_getDistVal(entity, 0));
	if (med == NULL) med = mediatorTable->lookup(entity);
	return (med ? med : new OzThreadMediator(entity));
      }
      // fall through, other extensions are unusables
    case Co_Resource:
      return getUnusableMediator(entity);

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
char* VarMediator::getPrintType(){ return "var";}
char* ArrayMediator::getPrintType(){ return "array";}
char* DictionaryMediator::getPrintType(){ return "dictionary";}
char* OzThreadMediator::getPrintType() {return "thread";}
char* UnusableMediator::getPrintType(){return "Unusable!!!";}
char* OzVariableMediator::getPrintType(){ return "var";}



//************************** Mediator ***************************//

Mediator::Mediator(TaggedRef ref, GlueTag etype, bool attach) :
  active(TRUE), attached(attach), collected(FALSE),
  dss_gc_status(DSS_GC_NONE), type(etype), annotation(0),
  entity(ref), absEntity(NULL), faultStream(0), next(NULL)
{
  id = medIdCntr++; 
  mediatorTable->insert(this);
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

// set both links between mediator and abstract entity
void            
Mediator::setAbstractEntity(AbstractEntity *ae) {
  absEntity = ae;
  if (ae) ae->assignMediator(dynamic_cast<MediatorInterface*>(this));
}

CoordinatorAssistantInterface* 
Mediator::getCoordinatorAssistant() {
  return absEntity->getCoordinatorAssistant();
}

void
Mediator::getDssParameters(ProtocolName &pn, AccessArchitecture &aa,
			   RCalg &rc) {
  int def = getDefaultAnnotation(getType());
  pn = static_cast<ProtocolName>
    (annotation & PN_MASK ? annotation & PN_MASK : def & PN_MASK);
  aa = static_cast<AccessArchitecture>
    (annotation & AA_MASK ? annotation & AA_MASK : def & AA_MASK);
  rc = static_cast<RCalg>
    (annotation & RC_ALG_MASK ? annotation & RC_ALG_MASK : def & RC_ALG_MASK);
}

void Mediator::gCollect(){
  if (!collected) {
    collected = TRUE;
    // collect the entity and its fault stream (if present)
    oz_gCollectTerm(entity, entity);
    oz_gCollectTerm(faultStream, faultStream);
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

ConstMediator::ConstMediator(ConstTerm *t, GlueTag type, bool attach) :
  Mediator(makeTaggedConst(t), type, attach)
{}

ConstTerm* ConstMediator::getConst(){
  return tagged2Const(getEntity()); 
}

void ConstMediator::globalize() {
  Assert(getAbstractEntity() == NULL);

  ProtocolName pn;
  AccessArchitecture aa;
  RCalg rc;
  getDssParameters(pn, aa, rc);
  setAbstractEntity(dss->m_createMutableAbstractEntity( pn, aa, rc));

  static_cast<ConstTermWithHome*>(getConst())->setMediator((void *)this);
  setAttached(ATTACHED);
}

void ConstMediator::localize(){
  if (annotation || faultStream) {
    // we have to keep the mediator, so
    // 1. remove abstract entity
    delete absEntity;
    absEntity = NULL;
    // 2. localize the lock (detach mediator)
    static_cast<ConstTermWithHome*>(getConst())->setBoard(oz_currentBoard());
    setAttached(DETACHED);
    // 3. keep the mediator in the table
    mediatorTable->insert(this);
    
  } else {
    // remove completely mediator, so
    // 1. localize the lock
    static_cast<ConstTermWithHome*>(getConst())->setBoard(oz_currentBoard());
    // 2. delete mediator
    delete this;
  }
}


/************************* PortMediator *************************/

PortMediator::PortMediator(ConstTerm *p) :
  ConstMediator(p, GLUE_PORT, DETACHED)
{}

PortMediator::PortMediator(ConstTerm *p, AbstractEntity *ae) :
  ConstMediator(p, GLUE_PORT, ATTACHED)
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
  
  ProtocolName pn;
  AccessArchitecture aa;
  RCalg rc;
  getDssParameters(pn, aa, rc);
  setAbstractEntity(dss->m_createRelaxedMutableAbstractEntity( pn, aa, rc));

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



/************************* VarMediator *************************/

VarMediator::VarMediator(AbstractEntity *ae, TaggedRef t) :
  Mediator(t, GLUE_VARIABLE, ATTACHED)
{
  setAbstractEntity(ae);
}

void VarMediator::globalize() { Assert(0); }
void VarMediator::localize() { Assert(0); }

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

  // create abstract entity
  ProtocolName pn;
  AccessArchitecture aa;
  RCalg rc;
  getDssParameters(pn, aa, rc);
  setAbstractEntity(dss->m_createImmutableAbstractEntity(pn, aa, rc));
}

void UnusableMediator::localize() {
  if (annotation || faultStream) {
    // we have to keep the mediator, so remove the abstract entity,
    // and reinsert the mediator in the table
    delete absEntity;
    absEntity = NULL;
    mediatorTable->insert(this);
  } else {
    // remove completely mediator, so
    delete this;
  }
}

AOcallback
UnusableMediator::callback_Read(DssThreadId* id_of_calling_thread,
				DssOperationId* operation_id,
				PstInContainerInterface* operation,
				PstOutContainerInterface*& possible_answer)
{ return AOCB_FINISH;}



/************************* OzThreadMediator *************************/

OzThreadMediator::OzThreadMediator(TaggedRef t) :
  Mediator(t, GLUE_THREAD, DETACHED)
{}

OzThreadMediator::OzThreadMediator(TaggedRef t, AbstractEntity *ae) :
  Mediator(t, GLUE_THREAD, ATTACHED)
{
  setAbstractEntity(ae);
}

void OzThreadMediator::globalize(){
  Assert(getAbstractEntity() == NULL);

  // create abstract entity
  ProtocolName pn;
  AccessArchitecture aa;
  RCalg rc;
  getDssParameters(pn, aa, rc);
  setAbstractEntity(dss->m_createMutableAbstractEntity(pn, aa, rc));

  // attach to entity
  oz_thread_setDistVal(entity, 0, this);
  setAttached(ATTACHED);
}

void OzThreadMediator::localize(){
  // raph: current limitation: we don't keep thread mediators...
  oz_thread_setDistVal(entity, 0, NULL);
  delete this;
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

LockMediator::LockMediator(ConstTerm *t) :
  ConstMediator(t, GLUE_LOCK, DETACHED)
{}

LockMediator::LockMediator(ConstTerm *t, AbstractEntity *ae) :
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
CellMediator::CellMediator(ConstTerm *c) :
  ConstMediator(c, GLUE_CELL, DETACHED)
{}

CellMediator::CellMediator(ConstTerm *c, AbstractEntity *ae) :
  ConstMediator(c, GLUE_CELL, ATTACHED)
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

ObjectMediator::ObjectMediator(ConstTerm *obj) :
  ConstMediator(obj, GLUE_OBJECT, DETACHED)
{}

ObjectMediator::ObjectMediator(ConstTerm *obj, AbstractEntity *ae) :
  ConstMediator(obj, GLUE_OBJECT, ATTACHED)
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
  ProtocolName pn;
  AccessArchitecture aa;
  RCalg gc;
  getDssParameters( pn, aa, gc);
  setAbstractEntity(dss->m_createMutableAbstractEntity(pn, aa, gc));
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

ArrayMediator::ArrayMediator(ConstTerm *t) :
  ConstMediator(t, GLUE_ARRAY, DETACHED)
{}

ArrayMediator::ArrayMediator(ConstTerm *t, AbstractEntity *ae) :
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

DictionaryMediator::DictionaryMediator(ConstTerm *t) :
  ConstMediator(t, GLUE_DICTIONARY, DETACHED)
{}

DictionaryMediator::DictionaryMediator(ConstTerm *t, AbstractEntity *ae) :
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
  int index = tagged2SmallInt(OZ_head(arg));
  TaggedRef val = OZ_tail(arg);
  ozd->setArg(index,val);
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
  TaggedRef arg = static_cast<PstInContainer*>(pstin)->a_term;
  int indx = tagged2SmallInt(arg);
  possible_answer =  new PstOutContainer(ozd->getArg(indx));
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
	   ATTACHED),
  patchCount(0)
{}

// assumption: t is a tagged REF to a tagged VAR.
OzVariableMediator::OzVariableMediator(TaggedRef t, AbstractEntity *ae) :
  Mediator(t, oz_isFree(*tagged2Ref(t)) ? GLUE_VARIABLE : GLUE_READONLY,
	   ATTACHED),
  patchCount(0)
{
  setAbstractEntity(ae);
}

void OzVariableMediator::incPatchCount() { patchCount++; }
void OzVariableMediator::decPatchCount() { patchCount--; }

void OzVariableMediator::globalize() {
  if (absEntity) return;   // don't globalize twice

  // create abstract entity
  ProtocolName pn;
  AccessArchitecture aa;
  RCalg rc;
  getDssParameters(pn, aa, rc);
  setAbstractEntity(dss->m_createMonotonicAbstractEntity(pn, aa, rc));
}

void OzVariableMediator::localize() {
  if (patchCount > 0) {
    // in this case, we simply cannot localize
    mediatorTable->insert(this);

  } else if (annotation || faultStream) {
    // we have to keep the mediator, so remove the abstract entity,
    // and keep the mediator in the table
    delete absEntity;
    absEntity = NULL;
    mediatorTable->insert(this);
    
  } else {
    // remove completely mediator, so
    // 1. localize the variable
    if (active) tagged2Var(oz_deref(entity))->removeMediator();
    // 2. delete mediator
    delete this;
  }
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
