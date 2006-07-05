/*
 *  Authors:
 *    Erik Klintskog, 2002
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Contributors:
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

#ifndef __GLUE_MEDIATORS_HH
#define __GLUE_MEDIATORS_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "dss_object.hh"
#include "tagged.hh"
#include "engine_interface.hh"
#include "glue_buffer.hh"
#include "glue_faults.hh"

/*
  Mediators interface Oz entities to the DSS by extending abstract
  entities.  Several aspects of distribution are implemented by
  mediators:

  - Operations: An operation on a distributed Oz entity may use an
  abstract operation.  The mediator also implements callbacks for
  remote operations, and the needs of the entity's distribution
  protocol.

  - Globalization/Localization: An Oz entity is distributed iff its
  mediator has a coordination proxy.  The mediator is responsible for
  creating/removing its coordination proxy.  It takes part in garbage
  collection, and makes the local and distributed GC cooperate.  It
  also stores the entity's annotation, which parameterizes its
  globalization.

  - Faults: Abstract operations report distribution failures to their
  mediator.  The mediator adapts the Oz entity's semantics to the
  given failure, and maintains a stream describing the entity's fault
  state.

  A mediator always has references to its Oz entity, and to its
  corresponding coordination proxy (when distributed).  An Oz entity
  normally has a pointer to its mediator.  But this might not be the
  case, for instance when the mediator only stores the entity's
  annotation (see detached mediators below).

				Entity
				 :  ^
				 :  |				   
				 v  |
		             XxxMediator (AbstractEntity)
				  ^
				  |				   
				  v
			 CoordinatorAssistant

  Now a bit of terminology:

  - A mediator is ACTIVE when it refers to a valid language entity.
  It becomes PASSIVE whenever this entity disappears.  In our case
  this only happens when variables are bound.  For the sake of
  consistency, passive mediators are considered attached (see below).

  - A mediator is ATTACHED when the emulator has a direct pointer to
  it.  Otherwise is is DETACHED, and should be looked up in the
  mediator table (using the entity's taggedref as a key).  Attached
  mediators are not found by looking up the mediator table.

  Note. Insertion of the mediator in the mediator table is automatic
  upon creation.  Connection to the coordination proxy is also
  automatic upon globalization.

 */



// the types of entities handled by the glue.  Those tags are also
// used as marshaling tags.
enum GlueTag {
  GLUE_NONE = 0,       // 
  GLUE_LAZYCOPY,       // immutables
  GLUE_UNUSABLE,
  GLUE_VARIABLE,       // transients
  GLUE_READONLY,
  GLUE_PORT,           // asynchronuous mutables
  GLUE_CELL,           // mutables
  GLUE_LOCK,
  GLUE_OBJECT,
  GLUE_ARRAY,
  GLUE_DICTIONARY,
  GLUE_THREAD,
  GLUE_LAST            // must be last
};

// attached or detached
#define DETACHED 0
#define ATTACHED 1



// Mediator is the abstract class for all mediators.  Mediator itself
// does not extend class AbstractEntity, but each concrete mediator
// class extends one of Mutable/Monotonic/Immutable AbstractEntity.

class Mediator {
  friend class MediatorTable;

protected:
  bool            active:1;          // TRUE if it is active
  bool            attached:1;        // TRUE if it is attached
  bool            collected:1;       // TRUE if it has been collected
  DSS_GC          dss_gc_status:2;   // status of dss gc
  GlueTag         type:5;            // type of entity
  GlueFaultState  faultState:2;      // current fault state

  Annotation      annotation;        // the entity's annotation

  int             id;

  TaggedRef       entity;            // references to engine
  TaggedRef       faultStream;
  TaggedRef       faultCtlVar;

  Mediator*       next;              // for the mediator table

public:
  /*************** constructor/destructor ***************/
  Mediator(TaggedRef entity, GlueTag type, bool attached);
  virtual ~Mediator();

  /*************** active/passive, attached/detached ***************/
  bool isActive() const { return active; }
  void makePassive();

  bool isAttached() const { return attached; }
  void setAttached(bool a) { attached = a; }

  /*************** get entity/coordination proxy ***************/
  TaggedRef getEntity() const { return entity; }
  CoordinatorAssistant* getCoordinatorAssistant();
  void setCoordinatorAssistant(CoordinatorAssistant*);

  /*************** annotate/globalize/localize ***************/
  GlueTag getType() const { return type; }

  Annotation getAnnotation() const { return annotation; }
  void setAnnotation(const Annotation& a) { annotation = a; }
  void completeAnnotation();

  virtual void globalize() = 0;     // create coordination proxy
  virtual void localize() = 0;      // localize entity

  /*************** garbage collection ***************/
  bool isCollected() const { return collected; }
  void gCollect();            // collect mediator (idempotent)
  void checkGCollect();       // collect if entity marked
  void resetGCStatus();       // reset the gc status
  DSS_GC getDssGCStatus();    // ask and return dss gc status

  // Note: In order to specialize gCollect(), make it a virtual method.

  /*************** fault handling ***************/
  GlueFaultState getFaultState() const { return faultState; }
  void           setFaultState(GlueFaultState fs);
  TaggedRef      getFaultStream();
  OZ_Return      suspendOnFault();     // suspend on control var

  void reportFS(const FaultState& fs);

  /*************** marshaling ***************/
  virtual void marshal(ByteBuffer *bs);

  /*************** debugging ***************/
  virtual char* getPrintType() { return "unknown"; }
  void print();
};



// return the mediator of an entity (create one if necessary)
Mediator *glue_getMediator(TaggedRef entity);



// ConstMediator is an abstract class for mediators referring to a
// ConstTermWithHome in the emulator.
class ConstMediator: public Mediator {
public: 
  ConstMediator(TaggedRef t, GlueTag type, bool attached);
  ConstTermWithHome* getConst() const;
  virtual void globalize();
  virtual void localize();
  virtual char *getPrintType() { return "const"; }
};



// mediators for Oz ports
class PortMediator: public ConstMediator, public RelaxedMutableAbstractEntity {
public:
  PortMediator(TaggedRef);
  PortMediator(TaggedRef, CoordinatorAssistant*);

  virtual AOcallback callback_Write(DssThreadId*, DssOperationId*,
				    PstInContainerInterface*);
  virtual AOcallback callback_Read(DssThreadId*, DssOperationId*,
				   PstInContainerInterface*,
				   PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation() {
    Assert(0); return NULL; }
  virtual void installEntityRepresentation(PstInContainerInterface*) {
    Assert(0); }
  virtual void reportFaultState(const FaultState& fs) { reportFS(fs); }
  virtual char *getPrintType() { return "port"; }
}; 


// mediators for Oz cells
class CellMediator: public ConstMediator, public MutableAbstractEntity {
public:
  CellMediator(TaggedRef);
  CellMediator(TaggedRef, CoordinatorAssistant*);

  virtual AOcallback callback_Write(DssThreadId*, DssOperationId*,
				    PstInContainerInterface*,
				    PstOutContainerInterface*&);
  virtual AOcallback callback_Read(DssThreadId*, DssOperationId*,
				   PstInContainerInterface*,
				   PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual PstOutContainerInterface *deinstallEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
  virtual void reportFaultState(const FaultState& fs) { reportFS(fs); }
  virtual char *getPrintType() { return "cell"; }
}; 


// mediators for Oz locks
class LockMediator: public ConstMediator, public MutableAbstractEntity {
public:
  LockMediator(TaggedRef);
  LockMediator(TaggedRef, CoordinatorAssistant*);

  virtual AOcallback callback_Write(DssThreadId*, DssOperationId*,
				    PstInContainerInterface*,
				    PstOutContainerInterface*&);
  virtual AOcallback callback_Read(DssThreadId*, DssOperationId*,
				   PstInContainerInterface*,
				   PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual PstOutContainerInterface *deinstallEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
  virtual void reportFaultState(const FaultState& fs) { reportFS(fs); }
  virtual char *getPrintType() { return "lock"; }
}; 


// mediators for Oz arrays
class ArrayMediator: public ConstMediator, public MutableAbstractEntity {
public:
  ArrayMediator(TaggedRef);
  ArrayMediator(TaggedRef, CoordinatorAssistant*);

  virtual AOcallback callback_Write(DssThreadId*, DssOperationId*,
				    PstInContainerInterface*,
				    PstOutContainerInterface*&);
  virtual AOcallback callback_Read(DssThreadId*, DssOperationId*,
				   PstInContainerInterface*,
				   PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual PstOutContainerInterface *deinstallEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
  virtual void reportFaultState(const FaultState& fs) { reportFS(fs); }
  virtual void marshal(ByteBuffer*);
  virtual char *getPrintType() { return "array"; }
}; 


// mediators for Oz dictionaries
class DictionaryMediator: public ConstMediator, public MutableAbstractEntity {
public:
  DictionaryMediator(TaggedRef);
  DictionaryMediator(TaggedRef, CoordinatorAssistant*);

  virtual AOcallback callback_Write(DssThreadId*, DssOperationId*,
				    PstInContainerInterface*,
				    PstOutContainerInterface*&);
  virtual AOcallback callback_Read(DssThreadId*, DssOperationId*,
				   PstInContainerInterface*,
				   PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual PstOutContainerInterface *deinstallEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
  virtual void reportFaultState(const FaultState& fs) { reportFS(fs); }
  virtual char *getPrintType() { return "dictionary"; }
}; 


// mediators for Oz objects
class ObjectMediator: public ConstMediator, public MutableAbstractEntity {
public:
  ObjectMediator(TaggedRef);
  ObjectMediator(TaggedRef, CoordinatorAssistant*);

  virtual AOcallback callback_Write(DssThreadId*, DssOperationId*,
				    PstInContainerInterface*,
				    PstOutContainerInterface*&);
  virtual AOcallback callback_Read(DssThreadId*, DssOperationId*,
				   PstInContainerInterface*,
				   PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual PstOutContainerInterface *deinstallEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
  virtual void reportFaultState(const FaultState& fs) { reportFS(fs); }
  virtual void globalize();
  virtual void marshal(ByteBuffer*);
  virtual char *getPrintType() { return "object"; }
};


// mediators for Oz threads
class OzThreadMediator: public ConstMediator, public MutableAbstractEntity {
public:
  OzThreadMediator(TaggedRef);
  OzThreadMediator(TaggedRef, CoordinatorAssistant*);

  virtual AOcallback callback_Write(DssThreadId*, DssOperationId*,
				    PstInContainerInterface*,
				    PstOutContainerInterface*&);
  virtual AOcallback callback_Read(DssThreadId*, DssOperationId*,
				   PstInContainerInterface*,
				   PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
  virtual void reportFaultState(const FaultState& fs) { reportFS(fs); }
  virtual char *getPrintType() { return "thread"; }
};



// mediators for "unusables".  An unusable is a remote representation
// of a site-specific entity.
class UnusableMediator: public Mediator, public ImmutableAbstractEntity {
public:
  UnusableMediator(TaggedRef);
  UnusableMediator(TaggedRef, CoordinatorAssistant*);

  virtual AOcallback callback_Read(DssThreadId*, DssOperationId*,
				   PstInContainerInterface*,
				   PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation() {
    Assert(0); return NULL; }
  virtual void installEntityRepresentation(PstInContainerInterface*) {
    Assert(0); }
  virtual void reportFaultState(const FaultState& fs) { reportFS(fs); }
  virtual void globalize();
  virtual void localize();
  virtual char *getPrintType() { return "unusable"; }
};



// mediators for Oz variables
class OzVariableMediator: public Mediator, public MonotonicAbstractEntity {
public:
  OzVariableMediator(TaggedRef);
  OzVariableMediator(TaggedRef, CoordinatorAssistant*);
  bool isDistributed() const;
  
  virtual void globalize();
  virtual void localize();
  virtual char *getPrintType() { return "var"; }

  virtual AOcallback callback_Bind(DssOperationId *id,
				   PstInContainerInterface* operation); 
  virtual AOcallback callback_Append(DssOperationId *id,
				     PstInContainerInterface* operation);
  virtual AOcallback callback_Changes(DssOperationId* id,
				      PstOutContainerInterface*& answer);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*);
  virtual void reportFaultState(const FaultState& fs) { reportFS(fs); }
}; 

// Note.  Patched variables keep their distribution support alive,
// even when the variable is bound.  The DistributedVarPatches do
// collect their mediators, which forces them to keep the coordination
// proxy alive.

#endif
