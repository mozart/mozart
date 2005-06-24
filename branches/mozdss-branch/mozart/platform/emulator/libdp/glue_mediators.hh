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

// forward declarations of class definitions
class Watcher;

// the different types of mediators
enum MEDIATOR_TYPE {
  MEDIATOR_TYPE_UNINIT,     // unused
  MEDIATOR_TYPE_VAR,        // mediator for an OzVariable
  MEDIATOR_TYPE_CONST,      // mediator for a ConstTerm
  MEDIATOR_TYPE_REF         // only a reference (no entity)
};

// whether a mediator should be kept in the mediator table
enum MEDIATOR_CONNECT {
  MEDIATOR_CONNECT_HASH,    // should be kept in the hash table
  MEDIATOR_CONNECT_LIST     // no longer kept in the hash table
};

// status of engine garbage collection
enum ENGINE_GC {
  ENGINE_GC_NONE,           // not collected
  ENGINE_GC_WEAK,           // collected as a weak reference
  ENGINE_GC_PRIMARY         // collected as a primary reference
};



// Mediator is the abstract class for all mediators
class Mediator {
  friend class EngineTable;
protected:
  MEDIATOR_TYPE    type:2;
  MEDIATOR_CONNECT connect:2;
  ENGINE_GC        engine_gc:2;   // status of engine gc
  DSS_GC           dss_gc:2;      // status of dss gc
  int              id;

  TaggedRef entity;            // references to engine and dss entities
  AbstractEntity *absEntity;

  Mediator *prev,*next;   // For Engine Table
  int patchCount;         // number of patches (for marshaler)
  Watcher *watchers;

  // raph: for debugging (to be removed)
  bool collected;

private:
  void triggerWatchers(FaultState);

public:
  /*************** constructor/destructor ***************/
  Mediator(AbstractEntity*, TaggedRef);
  virtual ~Mediator();

  /*************** set/get entity/abstract entity ***************/
  TaggedRef getEntity();
  void setEntity(TaggedRef ref);

  // the entity (variable) disappears and becomes a passive reference
  void mkPassiveRef();

  AbstractEntity *getAbstractEntity();
  void setAbstractEntity(AbstractEntity *a);
  CoordinatorAssistantInterface* getCoordinatorAssistant();

  /*************** garbage collection ***************/
  bool hasBeenGC() { return (engine_gc != ENGINE_GC_NONE); }
  void resetGC();                  // reset the gc status
  void engineGC(ENGINE_GC status); // collect mediator (from engine)
  void dssGC();                    // collect mediator (from dss)
  void gCollect();                 // collect the mediator's references
  // Note: In order to specialize gCollect(), make it a virtual method.

  /*************** marshaling ***************/
  void incPatchCount();
  void decPatchCount();

  /*************** fault handling ***************/
  void addWatcher(TaggedRef proc, FaultState fs);
  void removeWatcher(TaggedRef proc, FaultState fs);

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
public:
  OzVariableMediator(AbstractEntity *ae, TaggedRef t);
  
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
