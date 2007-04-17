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



/************************* glue_newMediator *************************/

Mediator *glue_newMediator(GlueTag tag) {
  // to be fixed
  switch (tag) {
  case GLUE_PORT:       return new PortMediator();
  case GLUE_CELL:       return new CellMediator();
  case GLUE_LOCK:       return new LockMediator();
  case GLUE_OBJECT:     return new ObjectMediator();
  case GLUE_ARRAY:      return new ArrayMediator();
  case GLUE_DICTIONARY: return new DictionaryMediator();
  case GLUE_THREAD:     return new OzThreadMediator();
  case GLUE_VARIABLE:   return new OzVariableMediator(GLUE_VARIABLE);
  case GLUE_READONLY:   return new OzVariableMediator(GLUE_READONLY);
  case GLUE_UNUSABLE:   return new UnusableMediator();
  case GLUE_CHUNK:      return new ChunkMediator();
  default:
    Assert(0); return NULL;
  }
}



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
  return (ctwh->isDistributed() ? ctwh->getMediator() :
	  lookupMediator<M>(makeTaggedConst(ct)));
}



Mediator *glue_getMediator(TaggedRef entity) {
  // assumption: entity is properly deref'd

  if (oz_isRef(entity)) { // entity is a variable
    TaggedRef *vPtr = tagged2Ref(entity);
    OzVariable *var = oz_getNonOptVar(vPtr);
    if (!var->hasMediator()) var->setMediator(new OzVariableMediator(entity));
    return var->getMediator();

  } else { // entity is a const term
    ConstTerm *ct = tagged2Const(entity);

    switch (ct->getType()) {
    case Co_Cell:       return getCTWHMediator<CellMediator>(ct);
    case Co_Object:     return getCTWHMediator<ObjectMediator>(ct);
    case Co_Port:       return getCTWHMediator<PortMediator>(ct);
    case Co_Chunk:      return getCTWHMediator<ChunkMediator>(ct);
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



/************************** Mediator ***************************/

Mediator::Mediator(GlueTag t) :
  active(TRUE), attached(FALSE), collected(FALSE),
  dss_gc_status(DSS_GC_NONE), type(t), faultState(GLUE_FAULT_NONE),
  annotation(emptyAnnotation),
  entity(makeTaggedNULL()),
  faultStream(makeTaggedNULL()),
  faultCtlVar(makeTaggedNULL()),
  next(NULL)
{
  id = medIdCntr++;
}

Mediator::~Mediator(){
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

void Mediator::setEntity(TaggedRef e) {
  Assert(!hasEntity() && e != makeTaggedNULL());
  entity = e;
  if (isDistributed() || faultState != GLUE_FAULT_NONE) attach();
  mediatorTable->insert(this);     // now we can put it in the table
}

void
Mediator::setProxy(CoordinatorAssistant* p) {
  setCoordinatorAssistant(p);
  if (p) {
    // first check annotation; set annotation from proxy if necessary
    if (!annotation.pn || !annotation.aa || !annotation.rc) {
      ProtocolName pn;
      AccessArchitecture aa;
      RCalg rc;
      p->getParameters(pn, aa, rc);
      annotation = makeAnnotation(pn, aa, rc);
    }
    // attach mediator to entity if required
    if (hasEntity() && !attached) attach();
    // then check fault state
    if (faultState == GLUE_FAULT_PERM) {
      abstractOperation_Kill();     // globalizing a failed entity...
    } else {
      p->setRegisteredFS(FS_PROT_MASK);   // register fault reporting
      if (faultStream) abstractOperation_Monitor();
    }
  } else {
    // detach the mediator (unless the entity has failed)
    if (attached && faultState == GLUE_FAULT_NONE) detach();
  }
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

bool Mediator::isImmediate() {
  completeAnnotation();
  return annotation.pn == PN_IMMEDIATE;
}

void Mediator::globalize() {
  Assert(!isDistributed());
  // Determine full annotation, create a coordination proxy with it,
  // and attach the mediator.
  completeAnnotation();
  setProxy(dss->createProxy(annotation.pn, annotation.aa, annotation.rc));
}

void Mediator::localize() {
  Assert(isDistributed());
  // We have to keep the mediator for future globalizations, so simply
  // remove the coordination proxy.
  setProxy(NULL);
}

void Mediator::gCollect(){
  if (!collected) {
    collected = TRUE;
    // collect the entity, its fault stream, ...
    oz_gCollectTerm(entity, entity);
    oz_gCollectTerm(faultStream, faultStream);
    // ... and mark the control var only if the failure can go away
    if (faultState > GLUE_FAULT_TEMP) faultCtlVar = makeTaggedNULL();
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
  if (isDistributed())
    dss_gc_status = getCoordinatorAssistant()->getDssDGCStatus();
  return dss_gc_status;
}

// used below to extend the fault stream
inline
void addToTail(TaggedRef& tail, const TaggedRef val) {
  TaggedRef t = oz_newReadOnly(oz_rootBoard());
  oz_bindReadOnly(tagged2Ref(tail), oz_cons(val, t));
  tail = t;
}

void
Mediator::setFaultState(GlueFaultState fs) {
  // this method is robust: it has no effect if state fs cannot be
  // reached from the current fault state.  This simplifies cases like
  // the DSS reporting 'ok' while we are in state 'localFail'.
  if (faultState == fs) return;
  if (!validFSTransition(faultState, fs)) return;

  faultState = fs;
  if (faultStream) addToTail(faultStream, fsToAtom(faultState));
  if (faultState == GLUE_FAULT_NONE) {
    if (faultCtlVar) {   // wake up blocked threads
      ControlVarResume(faultCtlVar);
      faultCtlVar = makeTaggedNULL();
    }
  } else {
    // The entity failed.  The failure will not be noticed by the
    // emulator if the mediator is detached, so try to attach it.
    if (hasEntity() && !attached) attach();
  }
}

TaggedRef
Mediator::getFaultStream() {
  // create the fault stream if necessary
  if (faultStream == 0) {
    faultStream = oz_newReadOnly(oz_rootBoard());
    abstractOperation_Monitor();
  }
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
Mediator::reportFaultState(const FaultState& fs) {
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

ConstMediator::ConstMediator(GlueTag t) : Mediator(t) {}

ConstTermWithHome* ConstMediator::getConst() const {
  return static_cast<ConstTermWithHome*>(tagged2Const(getEntity()));
}

void ConstMediator::attach() {
  getConst()->setMediator(this);
  attached = TRUE;
}

void ConstMediator::detach() {
  getConst()->setBoard(oz_currentBoard());
  attached = FALSE;
}



/************************* PortMediator *************************/

PortMediator::PortMediator() : ConstMediator(GLUE_PORT) {}

PortMediator::PortMediator(TaggedRef e) : ConstMediator(GLUE_PORT) {
  setEntity(e);
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
CellMediator::CellMediator() : ConstMediator(GLUE_CELL) {}

CellMediator::CellMediator(TaggedRef e) : ConstMediator(GLUE_CELL) {
  setEntity(e);
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

LockMediator::LockMediator() : ConstMediator(GLUE_LOCK) {}

LockMediator::LockMediator(TaggedRef e) : ConstMediator(GLUE_LOCK) {
  setEntity(e);
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

ArrayMediator::ArrayMediator() : ConstMediator(GLUE_ARRAY) {}

ArrayMediator::ArrayMediator(TaggedRef e) : ConstMediator(GLUE_ARRAY) {
  setEntity(e);
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

DictionaryMediator::DictionaryMediator() : ConstMediator(GLUE_DICTIONARY) {}

DictionaryMediator::DictionaryMediator(TaggedRef e) :
  ConstMediator(GLUE_DICTIONARY) {
  setEntity(e);
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

ObjectMediator::ObjectMediator() : ConstMediator(GLUE_OBJECT) {}

ObjectMediator::ObjectMediator(TaggedRef e) : ConstMediator(GLUE_OBJECT) {
  setEntity(e);
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



/************************* OzThreadMediator *************************/

OzThreadMediator::OzThreadMediator() : ConstMediator(GLUE_THREAD) {}

OzThreadMediator::OzThreadMediator(TaggedRef e) : ConstMediator(GLUE_THREAD) {
  setEntity(e);
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



/************************* OzVariableMediator *************************/

OzVariableMediator::OzVariableMediator(GlueTag t) : Mediator(t) {}

// assumption: e is a tagged REF to a tagged VAR.
OzVariableMediator::OzVariableMediator(TaggedRef e) :
  Mediator(oz_isFree(*tagged2Ref(e)) ? GLUE_VARIABLE : GLUE_READONLY) {
  setEntity(e);
  attach();
}

void OzVariableMediator::attach() {
  OzVariable* var = tagged2Var(*tagged2Ref(getEntity()));
  var->setMediator(this);
  attached = true;
}

PstOutContainerInterface *OzVariableMediator::retrieveEntityRepresentation(){
  return new PstOutContainer(getEntity());
}

void
OzVariableMediator::installEntityRepresentation(PstInContainerInterface* pstin){
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
  // simply check whether the variable is needed
  answer = (active && oz_isNeeded(oz_deref(entity)) ?
	    new PstOutContainer(oz_atom("needed")) : NULL);
  return AOCB_FINISH; 
}



/************************* UnusableMediator *************************/

UnusableMediator::UnusableMediator() : ConstMediator(GLUE_UNUSABLE) {}

UnusableMediator::UnusableMediator(TaggedRef e) : ConstMediator(GLUE_UNUSABLE) {
  setEntity(e);
}

AOcallback
UnusableMediator::callback_Read(DssThreadId*, DssOperationId*,
				PstInContainerInterface*,
				PstOutContainerInterface*&)
{
  return AOCB_FINISH;
}



/************************* ChunkMediator *************************/

ChunkMediator::ChunkMediator() : ConstMediator(GLUE_CHUNK) {}

ChunkMediator::ChunkMediator(TaggedRef e) : ConstMediator(GLUE_CHUNK) {
  setEntity(e);
}

AOcallback
ChunkMediator::callback_Read(DssThreadId*, DssOperationId*,
			     PstInContainerInterface* pstin,
			     PstOutContainerInterface*& answer) {
  // the only operation is '.'
  SChunk* chunk = static_cast<SChunk*>(getConst());
  TaggedRef key = static_cast<PstInContainer*>(pstin)->a_term;
  TaggedRef out = chunk->getFeature(key);
  answer = out ? new PstOutContainer(out) : NULL;
  return AOCB_FINISH;
}

PstOutContainerInterface*
ChunkMediator::retrieveEntityRepresentation() {
  PstOutContainer* pst = new PstOutContainer(getEntity());
  pst->setImmediate();
  return pst;
}

void
ChunkMediator::installEntityRepresentation(PstInContainerInterface* pst) {
  // we have nothing to do here, the unmarshaling of the value has
  // already completed the entity (via its gname)
}
