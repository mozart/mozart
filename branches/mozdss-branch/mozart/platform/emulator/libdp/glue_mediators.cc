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



//************************** Mediator ***************************//

Mediator::Mediator(TaggedRef ref, GlueTag etype, bool attach) :
  active(TRUE), attached(attach), collected(FALSE),
  dss_gc_status(DSS_GC_NONE), type(etype), faultState(GLUE_FAULT_NONE),
  annotation(emptyAnnotation),
  entity(ref), faultStream(0), faultCtlVar(0), next(NULL)
{
  id = medIdCntr++; 
  mediatorTable->insert(this);
  printf("--- raph: new mediator %p (type=%d)\n", this, type);
}

Mediator::~Mediator(){
  printf("--- raph: remove mediator %p\n", this);
  // The coordination proxy is deleted by ~AbstractEntity().
#ifdef INTERFACE
  // Nullify the pointers in debug mode
  next = NULL;
#endif
}

void
Mediator::makePassive() {
  Assert(attached);
  active = FALSE;
}

CoordinatorAssistant*
Mediator::getCoordinatorAssistant() {
  return dynamic_cast<AbstractEntity*>(this)->getCoordinatorAssistant();
}

void
Mediator::setCoordinatorAssistant(CoordinatorAssistant* p) {
  dynamic_cast<AbstractEntity*>(this)->setCoordinatorAssistant(p);
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
  if (getCoordinatorAssistant())
    dss_gc_status = getCoordinatorAssistant()->getDssDGCStatus();
  return dss_gc_status;
}

void
Mediator::setFaultState(GlueFaultState fs) {
  if (faultState != fs) {
    faultState = fs;

    if (fs == GLUE_FAULT_NONE && faultCtlVar) { // wakeup blocked threads
      ControlVarResume(faultCtlVar);
      faultCtlVar = makeTaggedNULL();
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
  printf("%s mediator, id %d, proxy %x, ref %x, gc(eng:%d dss:%d), con %d\n",
	 getPrintType(), id, getCoordinatorAssistant(), entity,
	 (int) collected, (int) dss_gc_status, (int) attached);
}



/************************* ConstMediator *************************/

ConstMediator::ConstMediator(TaggedRef t, GlueTag type, bool attached) :
  Mediator(t, type, attached)
{}

ConstTermWithHome* ConstMediator::getConst() const {
  return static_cast<ConstTermWithHome*>(tagged2Const(getEntity()));
}

void ConstMediator::globalize() {
  Assert(getCoordinatorAssistant() == NULL);
  // Determine the full annotation, create a coordination proxy with
  // it, and attach the mediator.
  completeAnnotation();
  setCoordinatorAssistant(dss->createProxy(annotation.pn,
					   annotation.aa,
					   annotation.rc));
  getConst()->setMediator(this);
  setAttached(ATTACHED);
}

void ConstMediator::localize() {
  // We keep the mediator, so we remove the coordination proxy, detach
  // the mediator, and reinsert it in the mediator table.
  setCoordinatorAssistant(NULL);
  getConst()->setBoard(oz_currentBoard());
  setAttached(DETACHED);
}



/************************* PortMediator *************************/

PortMediator::PortMediator(TaggedRef t) :
  ConstMediator(t, GLUE_PORT, DETACHED)
{}

PortMediator::PortMediator(TaggedRef t, CoordinatorAssistant* p) :
  ConstMediator(t, GLUE_PORT, ATTACHED)
{
  AbstractEntity::setCoordinatorAssistant(p);
}

AOcallback
PortMediator::callback_Write(DssThreadId*, DssOperationId*,
			     PstInContainerInterface* pstin)
{
  TaggedRef arg = static_cast<PstInContainer*>(pstin)->a_term;
  doPortSend(static_cast<OzPort*>(getConst()), arg, NULL);
  return AOCB_FINISH;
}

AOcallback
PortMediator::callback_Read(DssThreadId*, DssOperationId*,
			    PstInContainerInterface*,
			    PstOutContainerInterface*&)
{
  Assert(0);
  return AOCB_FINISH;
}


 
/************************* CellMediator *************************/

// use this constructor for annotations, etc.
CellMediator::CellMediator(TaggedRef t) :
  ConstMediator(t, GLUE_CELL, DETACHED)
{}

CellMediator::CellMediator(TaggedRef t, CoordinatorAssistant* p) :
  ConstMediator(t, GLUE_CELL, ATTACHED)
{
  AbstractEntity::setCoordinatorAssistant(p);
}

PstOutContainerInterface *
CellMediator::retrieveEntityRepresentation() {
  OzCell *cell = tagged2Cell(getEntity());
  TaggedRef out = cell->getValue();
  return new PstOutContainer(out);
}

PstOutContainerInterface *
CellMediator::deinstallEntityRepresentation() {
  OzCell *cell = tagged2Cell(getEntity());
  TaggedRef out = cell->exchangeValue(makeTaggedNULL());
  return new PstOutContainer(out);
}

void 
CellMediator::installEntityRepresentation(PstInContainerInterface* pstIn){
  PstInContainer *pst = static_cast<PstInContainer*>(pstIn);
  OzCell *cell = tagged2Cell(getEntity());
  cell->setValue(pst->a_term);
}

AOcallback
CellMediator::callback_Write(DssThreadId*, DssOperationId*,
			     PstInContainerInterface* pstin,
			     PstOutContainerInterface*& answer){
  OzCell *cell = tagged2Cell(getEntity());
  TaggedRef arg = static_cast<PstInContainer*>(pstin)->a_term;
  TaggedRef out = cell->exchangeValue(arg);
  answer = new PstOutContainer(out);
  return AOCB_FINISH;
}

AOcallback
CellMediator::callback_Read(DssThreadId*, DssOperationId*,
			    PstInContainerInterface* pstin,
			    PstOutContainerInterface*& answer){
  OzCell *cell = tagged2Cell(entity);
  TaggedRef out =cell->getValue();
  answer = new PstOutContainer(out);
  return AOCB_FINISH;
}



/************************* LockMediator *************************/

LockMediator::LockMediator(TaggedRef t) :
  ConstMediator(t, GLUE_LOCK, DETACHED)
{}

LockMediator::LockMediator(TaggedRef t, CoordinatorAssistant* p) :
  ConstMediator(t, GLUE_LOCK, ATTACHED)
{
  AbstractEntity::setCoordinatorAssistant(p);
}

AOcallback 
LockMediator::callback_Write(DssThreadId*, DssOperationId*,
			     PstInContainerInterface* operation,
			     PstOutContainerInterface*& answer)
{
  // Locks provide two operations.  The argument 'operation' has the
  // form Op|ThreadId, where Op is either 'take', or 'release'.
  OzLock* lock = static_cast<OzLock*>(getConst());
  TaggedRef arg = static_cast<PstInContainer*>(operation)->a_term;
  if (oz_isCons(arg)) {
    TaggedRef op = oz_head(arg);
    TaggedRef thr = oz_tail(arg);

    if (oz_eq(op, oz_atom("take"))) {
      if (lock->take(thr)) { // granted, return unit
	answer = new PstOutContainer(oz_unit());
      } else { // subscribe, and return control variable
	TaggedRef controlvar = oz_newVariable(oz_rootBoard());
	lock->subscribe(thr, controlvar);
	answer = new PstOutContainer(controlvar);
      }
    } else {
      Assert(oz_eq(op, oz_atom("release")));
      lock->release(thr);
      answer = NULL;
    }
  }
  return AOCB_FINISH;
}

AOcallback 
LockMediator::callback_Read(DssThreadId*, DssOperationId*,
			    PstInContainerInterface*,
			    PstOutContainerInterface*&)
{
  Assert(0);
  return AOCB_FINISH;
}

PstOutContainerInterface *
LockMediator::retrieveEntityRepresentation() {
  OzLock* lock = static_cast<OzLock*>(getConst());
  TaggedRef locker = lock->getLocker();
  if (locker == 0) {
    return NULL;
  } else {
    TaggedRef depth = oz_int(lock->getLockingDepth());
    TaggedRef pending = pendingThreadList2List(lock->getPending());
    TaggedRef answer = oz_cons(oz_pair2(locker, depth), pending);
    return new PstOutContainer(answer);
  }
}

PstOutContainerInterface *
LockMediator::deinstallEntityRepresentation() {
  PstOutContainerInterface* tmp = retrieveEntityRepresentation();
  OzLock* lock = static_cast<OzLock*>(getConst());
  lock->setLocker(0);
  lock->setLockingDepth(0);
  lock->setPending(NULL);
  return tmp;
}

void 
LockMediator::installEntityRepresentation(PstInContainerInterface* pstIn){
  OzLock* lock = static_cast<OzLock*>(getConst());
  if (pstIn == NULL) {
    lock->setLocker(0);
    lock->setLockingDepth(0);
    lock->setPending(NULL);
  } else {
    TaggedRef arg = static_cast<PstInContainer*>(pstIn)->a_term;
    lock->setLocker(oz_left(oz_head(arg)));
    lock->setLockingDepth(OZ_intToC(oz_right(oz_head(arg))));
    lock->setPending(list2PendingThreadList(oz_tail(arg)));
  }
}



/************************* ArrayMediator *************************/

ArrayMediator::ArrayMediator(TaggedRef t) :
  ConstMediator(t, GLUE_ARRAY, DETACHED)
{}

ArrayMediator::ArrayMediator(TaggedRef t, CoordinatorAssistant* p) :
  ConstMediator(t, GLUE_ARRAY, ATTACHED)
{
  AbstractEntity::setCoordinatorAssistant(p);
}

AOcallback 
ArrayMediator::callback_Write(DssThreadId*, DssOperationId*,
			      PstInContainerInterface* pstin,
			      PstOutContainerInterface*& answer)
{
  OzArray *oza = static_cast<OzArray*>(getConst());
  TaggedRef arg = static_cast<PstInContainer*>(pstin)->a_term;
  int index = tagged2SmallInt(oz_head(arg));
  TaggedRef val = oz_tail(arg);
  if (oza->setArg(index,val))
    answer = NULL;
  else
    answer = new PstOutContainer(OZ_makeException(E_ERROR, E_KERNEL, "array",
						  2, getEntity(), index));
  return AOCB_FINISH;
}

AOcallback
ArrayMediator::callback_Read(DssThreadId*, DssOperationId*,
			     PstInContainerInterface* pstin,
			     PstOutContainerInterface*& answer)
{
  OzArray *oza = static_cast<OzArray*>(getConst());
  TaggedRef arg = static_cast<PstInContainer*>(pstin)->a_term;
  int index = tagged2SmallInt(arg);
  TaggedRef out = oza->getArg(index);
  if (out)
    answer = new PstOutContainer(out);
  else 
    answer = new PstOutContainer(OZ_makeException(E_ERROR, E_KERNEL, "array", 
						  2, getEntity(), index));
  return AOCB_FINISH;
}

PstOutContainerInterface*
ArrayMediator::retrieveEntityRepresentation(){
  // raph: the elements are sent in a list (in order)
  OzArray *oza = static_cast<OzArray*>(getConst()); 
  TaggedRef *ar = oza->getRef();
  TaggedRef list = oz_nil();
  for (int i = oza->getWidth()-1; i >= 0; i--)
    list = oz_cons(ar[i], list);
  return new PstOutContainer(list);
}

PstOutContainerInterface*
ArrayMediator::deinstallEntityRepresentation(){
  // raph: the elements are sent in a list (in order)
  OzArray *oza = static_cast<OzArray*>(getConst()); 
  TaggedRef *ar = oza->getRef();
  TaggedRef list = oz_nil();
  for (int i = oza->getWidth()-1; i >= 0; i--) {
    list = oz_cons(ar[i], list);
    ar[i] = makeTaggedNULL();
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

DictionaryMediator::DictionaryMediator(TaggedRef t, CoordinatorAssistant* p) :
  ConstMediator(t, GLUE_DICTIONARY, ATTACHED)
{
  AbstractEntity::setCoordinatorAssistant(p);
}

AOcallback 
DictionaryMediator::callback_Write(DssThreadId*, DssOperationId*,
				   PstInContainerInterface* pstin,
				   PstOutContainerInterface*& answer)
{
  OzDictionary *ozd = tagged2Dictionary(getEntity());
  TaggedRef arg = static_cast<PstInContainer*>(pstin)->a_term;
  TaggedRef key = oz_head(arg);
  TaggedRef val = oz_tail(arg);
  ozd->setArg(key,val);
  answer = NULL;
  return AOCB_FINISH;
}

AOcallback
DictionaryMediator::callback_Read(DssThreadId*, DssOperationId*,
				  PstInContainerInterface* pstin,
				  PstOutContainerInterface*& answer)
{
  OzDictionary *ozd = tagged2Dictionary(getEntity()); 
  TaggedRef key = static_cast<PstInContainer*>(pstin)->a_term;
  TaggedRef out = ozd->getArg(key);
  if (out) {
    answer = new PstOutContainer(OZ_mkTupleC("ok", 1, out));
  } else {
    answer = new PstOutContainer(OZ_makeException(E_ERROR, E_KERNEL, "dict",
						  2, getEntity(), key));
  }
  return AOCB_FINISH;
}

PstOutContainerInterface*
DictionaryMediator::retrieveEntityRepresentation(){
  // sent the list of entries
  OzDictionary *ozd = tagged2Dictionary(getEntity());
  return new PstOutContainer(ozd->pairs());
}

PstOutContainerInterface*
DictionaryMediator::deinstallEntityRepresentation(){
  // sent the list of entries, and clean up the dictionary
  OzDictionary *ozd = tagged2Dictionary(getEntity());
  TaggedRef entries = ozd->pairs();
  ozd->removeAll();
  return new PstOutContainer(entries);
}

void
DictionaryMediator::installEntityRepresentation(PstInContainerInterface* pstin){
  // make sure the dictionary is empty
  OzDictionary *ozd = tagged2Dictionary(getEntity()); 
  ozd->removeAll();
  // insert all entries (not pretty efficient)
  TaggedRef entries = static_cast<PstInContainer*>(pstin)->a_term;
  while (!oz_isNil(entries)) {
    TaggedRef keyval = oz_head(entries);
    ozd->setArg(oz_left(keyval), oz_right(keyval));
    entries = oz_tail(entries);
  }
}



/************************* ObjectMediator *************************/

ObjectMediator::ObjectMediator(TaggedRef t) :
  ConstMediator(t, GLUE_OBJECT, DETACHED)
{}

ObjectMediator::ObjectMediator(TaggedRef t, CoordinatorAssistant* p) :
  ConstMediator(t, GLUE_OBJECT, ATTACHED)
{
  AbstractEntity::setCoordinatorAssistant(p);
}

AOcallback
ObjectMediator::callback_Write(DssThreadId*, DssOperationId*,
			       PstInContainerInterface* operation,
			       PstOutContainerInterface*& answer) {
  return AOCB_FINISH;
}

AOcallback
ObjectMediator::callback_Read(DssThreadId*, DssOperationId*,
			      PstInContainerInterface* operation,
			      PstOutContainerInterface*& answer) {
  return AOCB_FINISH;
}

PstOutContainerInterface*
ObjectMediator::retrieveEntityRepresentation(){ return NULL; }

PstOutContainerInterface*
ObjectMediator::deinstallEntityRepresentation(){ return NULL; }

void
ObjectMediator::installEntityRepresentation(PstInContainerInterface*){;}

void ObjectMediator::globalize() {
  // We still have to figure out for this one.  See the old crap below.

  /*
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
  */
}



/************************* OzThreadMediator *************************/

OzThreadMediator::OzThreadMediator(TaggedRef t) :
  ConstMediator(t, GLUE_THREAD, DETACHED)
{}

OzThreadMediator::OzThreadMediator(TaggedRef t, CoordinatorAssistant* p) :
  ConstMediator(t, GLUE_THREAD, ATTACHED)
{
  AbstractEntity::setCoordinatorAssistant(p);
}

AOcallback
OzThreadMediator::callback_Write(DssThreadId*, DssOperationId*,
				 PstInContainerInterface*,
				 PstOutContainerInterface*&)
{
  return AOCB_FINISH;
}

AOcallback
OzThreadMediator::callback_Read(DssThreadId*, DssOperationId*,
				PstInContainerInterface*,
				PstOutContainerInterface*&)
{
  return AOCB_FINISH;
}

PstOutContainerInterface*
OzThreadMediator::retrieveEntityRepresentation(){
  Assert(0); return NULL;
}

void 
OzThreadMediator::installEntityRepresentation(PstInContainerInterface*){
  Assert(0); 
}



/************************* UnusableMediator *************************/

UnusableMediator::UnusableMediator(TaggedRef t) :
  Mediator(t, GLUE_UNUSABLE, DETACHED)
{}

UnusableMediator::UnusableMediator(TaggedRef t, CoordinatorAssistant* p) :
  Mediator(t, GLUE_UNUSABLE, DETACHED)
{
  AbstractEntity::setCoordinatorAssistant(p);
}

void UnusableMediator::globalize() {
  Assert(AbstractEntity::getCoordinatorAssistant() == NULL);
  completeAnnotation();
  AbstractEntity::setCoordinatorAssistant(dss->createProxy(annotation.pn,
							   annotation.aa,
							   annotation.rc));
}

void UnusableMediator::localize() {
  // We keep the mediator, so we remove the coordination proxy, and
  // reinsert it in the mediator table.
  AbstractEntity::setCoordinatorAssistant(NULL);
}

AOcallback
UnusableMediator::callback_Read(DssThreadId*, DssOperationId*,
				PstInContainerInterface*,
				PstOutContainerInterface*&)
{
  return AOCB_FINISH;
}



/************************* OzVariableMediator *************************/

// assumption: t is a tagged REF to a tagged VAR.
OzVariableMediator::OzVariableMediator(TaggedRef t) :
  Mediator(t, oz_isFree(*tagged2Ref(t)) ? GLUE_VARIABLE : GLUE_READONLY,
	   ATTACHED)
{}

// assumption: t is a tagged REF to a tagged VAR.
OzVariableMediator::OzVariableMediator(TaggedRef t, CoordinatorAssistant* p) :
  Mediator(t, oz_isFree(*tagged2Ref(t)) ? GLUE_VARIABLE : GLUE_READONLY,
	   ATTACHED)
{
  AbstractEntity::setCoordinatorAssistant(p);
}

bool OzVariableMediator::isDistributed() const {
  return AbstractEntity::getCoordinatorAssistant() != NULL;
}

void OzVariableMediator::globalize() {
  Assert(AbstractEntity::getCoordinatorAssistant() == NULL);
  completeAnnotation();
  AbstractEntity::setCoordinatorAssistant(dss->createProxy(annotation.pn,
							   annotation.aa,
							   annotation.rc));
}

void OzVariableMediator::localize() {
  printf("--- raph: localize var mediator %p\n", this);
  // So remove the coordination proxy, and keep the mediator in the
  // table
  AbstractEntity::setCoordinatorAssistant(NULL);
}

PstOutContainerInterface *OzVariableMediator::retrieveEntityRepresentation(){
  printf("--- raph: retrieveEntityRepresentation %x\n", getEntity());
  return new PstOutContainer(getEntity());
}

void OzVariableMediator::installEntityRepresentation(PstInContainerInterface* pstin){
  printf("--- raph: installEntityRepresentation %x\n", getEntity());
  Assert(active);
  TaggedRef arg = static_cast<PstInContainer*>(pstin)->a_term;
  TaggedRef* ref = tagged2Ref(getEntity()); // points to the var's tagged ref
  OzVariable* ov = tagged2Var(*ref);
  
  oz_bindLocalVar(ov, ref, arg);
  makePassive();
}

AOcallback
OzVariableMediator::callback_Bind(DssOperationId*,
				  PstInContainerInterface* pstin) {
  printf("--- raph: callback_Bind %x\n", getEntity());
  Assert(active);
  TaggedRef arg = static_cast<PstInContainer*>(pstin)->a_term;
  TaggedRef* ref = tagged2Ref(getEntity()); // points to the var's tagged ref
  OzVariable* ov = tagged2Var(*ref);
  
  oz_bindLocalVar(ov, ref, arg);
  makePassive();
  return AOCB_FINISH;
}

AOcallback
OzVariableMediator::callback_Append(DssOperationId*,
				    PstInContainerInterface* pstin) {
  printf("--- raph: callback_Append %x\n", getEntity());

  // raph: The variable may have been bound at this point.  This can
  // happen when two operations Bind and Append are done concurrently
  // (a "feature" of the dss).  Therefore we check the type first.
  if (active) {
    // check pstin
    if (pstin !=  NULL) {
      TaggedRef arg = static_cast<PstInContainer*>(pstin)->a_term;
      Assert(arg == oz_atom("needed"));
    }
    TaggedRef* ref = tagged2Ref(getEntity()); // points to the tagged ref
    oz_var_makeNeededLocal(ref);
  }
  return AOCB_FINISH; 
}

AOcallback
OzVariableMediator::callback_Changes(DssOperationId*,
				     PstOutContainerInterface*& answer) {
  printf("--- raph: callback_Changes\n");
  // simply check whether the variable is needed
  answer = (active && oz_isNeeded(oz_deref(entity)) ?
	    new PstOutContainer(oz_atom("needed")) : NULL);
  return AOCB_FINISH; 
}
