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

#ifndef __GLUE_MEDIATORS_HH
#define __GLUE_MEDIATORS_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "dss_object.hh"
#include "tagged.hh"


/*
  Mediators interface Oz entities to DSS abstract entities.  Several
  aspects of distribution are implemented by mediators:

  - Operations: An operation on a distributed Oz entity may require an
  operation on its abstract entity.  The entity's mediator gives
  access to its abstract entity.  The mediator also implements
  callbacks for remote operations, and the needs of the entity's
  distribution protocol.

  - Globalization/Localization: The mediator is responsible for
  creating the abstract entity, and for localizing the Oz entity.  It
  takes part in garbage collection, and makes the local and
  distributed GC cooperate.  It also stores the entity's annotation,
  which parameterizes its globalization.

  - Faults: Abstract operations report distribution failures to their
  mediator.  The mediator adapts the Oz entity's semantics to the
  given failure, and maintains a stream describing the entity's fault
  state.

  A mediator always has references to its Oz entity, and to its
  corresponding abstract entity (when distributed).  The abstract
  entity also refers to the mediator.  An Oz entity normally has a
  pointer to its mediator.  But this might not be the case, for
  instance when the mediator only stores the entity's annotation (see
  detached mediators below).

  Now a bit of terminology:

  - A mediator is ACTIVE when it refers to a valid language entity.
  It becomes PASSIVE whenever this entity disappears.  In our case
  this only happens when variables are bound.  For the sake of
  consistency, passive mediators must be attached (see below).

  - A mediator is ATTACHED when the emulator has a direct pointer to
  it.  Otherwise is is DETACHED, and should be looked up in the
  mediator table (using the entity's taggedref as a key).  Attached
  mediators are not found by looking up the mediator table.

  Note. Insertion of the mediator in the mediator table is automatic
  upon creation.  Connection to the abstract entity is also automatic
  upon globalization (not yet, still buggy).

 */



// Mediator is the abstract class for all mediators
class Mediator {
  friend class EngineTable;
protected:
  bool   active:1;          // TRUE if it is active
  bool   attached:1;        // TRUE if it is attached
  bool   collected:1;       // TRUE if it has been collected
  DSS_GC dss_gc_status:2;   // status of dss gc
  int    annotation:24;     // the entity's annotation (24 bits)

  int              id;

  TaggedRef       entity;      // references to engine and dss entities
  TaggedRef       faultStream;
  AbstractEntity* absEntity;

  Mediator *prev;
  Mediator *next;           // for mediator table

public:
  /*************** constructor/destructor ***************/
  Mediator(AbstractEntity*, TaggedRef, bool);
  virtual ~Mediator();

  /*************** active/passive, attached/detached ***************/
  bool isActive() { return active; }
  void makePassive();

  bool isAttached() { return attached; }
  void setAttached(bool a) { attached = a; }

  /*************** set/get entity/abstract entity ***************/
  TaggedRef getEntity();
  void setEntity(TaggedRef ref);

  AbstractEntity *getAbstractEntity();
  void setAbstractEntity(AbstractEntity *a);
  CoordinatorAssistantInterface* getCoordinatorAssistant();

  /*************** annotate/globalize/localize ***************/
  int getAnnotation() { return annotation; }
  void annotate(int a) { annotation = a; } // warning: unchecked
  //virtual void globalize() = 0;     // create abstract entity
  virtual void localize() = 0;      // try to localize entity

  /*************** garbage collection ***************/
  bool isCollected();         // test (may also collect if detached)
  void gCollect();            // collect mediator (idempotent)
  void resetGCStatus();       // reset the gc status
  DSS_GC getDssDGCStatus();   // ask and return dss gc status

  // Note: In order to specialize gCollect(), make it a virtual method.

  /*************** fault handling ***************/
  TaggedRef getFaultStream();
  // int getFaultState();        // return current fault state
  // void setFaultState(int);    // force fault state

  virtual void reportFaultState(const FaultState& fs);

  /*************** debugging ***************/
  virtual char* getPrintType() = 0;
  void print();
  bool check();
};



// ConstMediator is an abstract class for mediators referring to a
// ConstTerm in the emulator.
class ConstMediator: public Mediator {
public: 
  ConstMediator(AbstractEntity *ae, ConstTerm *t);
  ConstTerm* getConst();
  virtual char *getPrintType();
};



// mediators for Oz threads
class OzThreadMediator: public Mediator, public MutableMediatorInterface {
public:
  OzThreadMediator(AbstractEntity *ae, TaggedRef t);
  
  virtual AOcallback callback_Write(DssThreadId* id_of_calling_thread,
				    DssOperationId* operation_id,
				    PstInContainerInterface* operation,
				    PstOutContainerInterface*& possible_answer);
  virtual AOcallback callback_Read(DssThreadId* id_of_calling_thread,
				   DssOperationId* operation_id,
				   PstInContainerInterface* operation,
				   PstOutContainerInterface*& possible_answer);
  virtual void localize();
  virtual char *getPrintType();
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*);  
};


// mediators for "unusables".  An unusable is a remote representation
// of a site-specific entity.
class UnusableMediator: public Mediator, public ImmutableMediatorInterface {
public:
  UnusableMediator(AbstractEntity* ae, TaggedRef t);
  ~UnusableMediator();
  
  virtual AOcallback callback_Read(DssThreadId* id_of_calling_thread,
				   DssOperationId* operation_id,
				   PstInContainerInterface* operation,
				   PstOutContainerInterface*& possible_answer);
  virtual void localize();
  virtual char *getPrintType();
  virtual PstOutContainerInterface *retrieveEntityRepresentation() { Assert(0); return NULL;}
  virtual void installEntityRepresentation(PstInContainerInterface*) { Assert(0);}
};


// mediators for Oz ports
class PortMediator:
  public ConstMediator, public RelaxedMutableMediatorInterface {
public:
  PortMediator(AbstractEntity *p, Tertiary *t);
  
  virtual AOcallback callback_Write(DssThreadId* id_of_calling_thread,
				    DssOperationId* operation_id,
				    PstInContainerInterface* operation);
  virtual AOcallback callback_Read(DssThreadId* id_of_calling_thread,
				   DssOperationId* operation_id,
				   PstInContainerInterface* operation,
				   PstOutContainerInterface*& possible_answer);
  virtual void localize();
  virtual char *getPrintType();
  virtual PstOutContainerInterface *retrieveEntityRepresentation() { Assert(0); return NULL;}
  virtual void installEntityRepresentation(PstInContainerInterface*) { Assert(0);} 
}; 


// mediators for Oz cells
class CellMediator: public ConstMediator, public MutableMediatorInterface{
public:
  CellMediator(AbstractEntity *p, Tertiary *t);
  
  virtual AOcallback callback_Write(DssThreadId* id_of_calling_thread,
				    DssOperationId* operation_id,
				    PstInContainerInterface* operation,
				    PstOutContainerInterface*& possible_answer);
  virtual AOcallback callback_Read(DssThreadId* id_of_calling_thread,
				   DssOperationId* operation_id,
				   PstInContainerInterface* operation,
				   PstOutContainerInterface*& possible_answer);
  virtual void localize();
  virtual char *getPrintType();
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
}; 


// mediators for Oz locks
class LockMediator: public ConstMediator, public MutableMediatorInterface{
public:
  LockMediator(AbstractEntity *p, Tertiary *t);
  
  virtual AOcallback callback_Write(DssThreadId* id_of_calling_thread,
				    DssOperationId* operation_id,
				    PstInContainerInterface* operation,
				    PstOutContainerInterface*& possible_answer);
  virtual AOcallback callback_Read(DssThreadId* id_of_calling_thread,
				   DssOperationId* operation_id,
				   PstInContainerInterface* operation,
				   PstOutContainerInterface*& possible_answer);
  virtual void localize();
  virtual char *getPrintType();
  virtual PstOutContainerInterface *retrieveEntityRepresentation();  
  virtual void installEntityRepresentation(PstInContainerInterface*);  
}; 


// mediators for Oz variables
class VarMediator: public Mediator, public MonotonicMediatorInterface {
public:
  VarMediator(AbstractEntity *p, TaggedRef t);
  
  virtual AOcallback callback_Bind(DssOperationId *id,
				   PstInContainerInterface* operation); 
  virtual AOcallback callback_Append(DssOperationId *id,
				     PstInContainerInterface* operation);
  virtual void localize();
  virtual char *getPrintType();
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*);
}; 


// mediators for immutable values that are replicated lazily
class LazyVarMediator: public Mediator {
public:
  LazyVarMediator(AbstractEntity *p, TaggedRef t);
  virtual void localize();
  virtual char *getPrintType();
}; 


// mediators for Oz arrays
class ArrayMediator: public ConstMediator, public MutableMediatorInterface{
public:
  ArrayMediator(AbstractEntity *p, ConstTerm *t);
  
  virtual AOcallback callback_Write(DssThreadId* id_of_calling_thread,
				    DssOperationId* operation_id,
				    PstInContainerInterface* operation,
				    PstOutContainerInterface*& possible_answer);
  virtual AOcallback callback_Read(DssThreadId* id_of_calling_thread,
				   DssOperationId* operation_id,
				   PstInContainerInterface* operation,
				   PstOutContainerInterface*& possible_answer);
  virtual void localize();
  virtual char *getPrintType();

  virtual PstOutContainerInterface *retrieveEntityRepresentation() ;
  virtual void installEntityRepresentation(PstInContainerInterface*) ;
}; 


// mediators for Oz objects
class ObjectMediator: public ConstMediator, public MutableMediatorInterface{
public:
  ObjectMediator(AbstractEntity *p, Tertiary *t);
  
  virtual AOcallback callback_Write(DssThreadId* id_of_calling_thread,
				    DssOperationId* operation_id,
				    PstInContainerInterface* operation,
				    PstOutContainerInterface*& possible_answer);
  virtual AOcallback callback_Read(DssThreadId* id_of_calling_thread,
				   DssOperationId* operation_id,
				   PstInContainerInterface* operation,
				   PstOutContainerInterface*& possible_answer);
  virtual void localize();
  virtual char *getPrintType();
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*);
};


// mediators for Oz variables
class OzVariableMediator:
  public Mediator, public MonotonicMediatorInterface {
private:
  int patchCount;     // number of patches (for marshaler)

public:
  OzVariableMediator(AbstractEntity *ae, TaggedRef t);
  void incPatchCount();
  void decPatchCount();
  
  virtual AOcallback callback_Bind(DssOperationId *id,
				   PstInContainerInterface* operation); 
  virtual AOcallback callback_Append(DssOperationId *id,
				     PstInContainerInterface* operation);
  virtual void localize();
  virtual char *getPrintType();
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*);
}; 


#endif
