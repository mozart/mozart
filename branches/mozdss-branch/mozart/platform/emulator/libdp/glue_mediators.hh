/*
 *  Authors:
 *    Erik Klintskog, 2002
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

enum AO_TYPE {
  AO_TYPE_UNINIT,
  AO_TYPE_VAR,
  AO_TYPE_CONST,
  AO_TYPE_REF
};

enum AO_CONNECT{
  AO_CONNECT_HASH,
  AO_CONNECT_LIST
};

enum ENGINE_GC{
  ENGINE_GC_DEAD,
  ENGINE_GC_WEAK,
  ENGINE_GC_PRIMARY
};


// Mediator is the abstract class for all mediators
class Mediator{
  friend class EngineTable;
  friend class Proxy;
protected:
  AO_TYPE    type:2;
  AO_CONNECT connect:2;
  ENGINE_GC  engine_gc:2;
  DSS_GC     dss_gc:2;
  int        id; 
protected:
  Mediator *prev,*next;  // For Engine Table
  int patchCnt; //ERIK, document why we had this
  Watcher *watchers;
  AbstractEntity *a_abstractEntityInstance; 
private:
  void triggerWatchers(FaultState);

public:

  Mediator(AbstractEntity*);
  virtual ~Mediator();

  //******************* GC and flags ************************
  bool hasGCStatus(){ return (engine_gc != ENGINE_GC_DEAD); }
  void engGC(ENGINE_GC status); // For engineTable
  virtual void dssGC() = 0; // This can be invoked ONLY ONCE!
  void gCollect();
  //********************** ASSORTED ************************
  void incPatchCnt();
  void decPatchCnt();

  AbstractEntity *getAbstractEntity();
  void            setAbstractEntity(AbstractEntity *a);

  CoordinatorAssistantInterface* getCoordAssInterface();
  
  void addWatcher(TaggedRef proc, FaultState fs);
  void removeWatcher(TaggedRef proc, FaultState fs);

  virtual TaggedRef getEntity() = 0; 
  virtual char* getPrintType() = 0;
  
  void mkPassiveRef(); //Convert Var to Ref and remove from further hash-listing

  //********************** DEBUG ***************************
  void print();
  //#ifdef INTERFACE
  bool check();
  //#endif

  //***************** MediatorInterface ********************
  virtual void reportFaultState(const FaultState& fs);
};


// ConstMediator is an abstract class for mediators referring to a
// ConstTerm in the emulator
class ConstMediator:public Mediator{
protected: 
  ConstTerm* a_const;
public: 
  ConstMediator(AbstractEntity *ae, ConstTerm *t);
  
  void setConst(ConstTerm *t);
  ConstTerm* getConst();
  virtual TaggedRef getEntity();
  virtual void dssGC();
  virtual char *getPrintType();
};


// RefMediator is an abstract class for mediators referring to an
// emulator entity via a tagged reference (variables, threads)
class RefMediator:public Mediator{
  TaggedRef a_ref; 
public: 
  RefMediator(AbstractEntity *ae, TaggedRef t);
  
  void setRef(TaggedRef tr) {  a_ref = tr; }
  TaggedRef getRef() { return a_ref;}
  void derefPtr();
  virtual TaggedRef getEntity(){return a_ref;}
  virtual void dssGC();
  virtual char *getPrintType();
};


// mediators for Oz threads
class OzThreadMediator:public RefMediator, public MutableMediatorInterface{
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
class UnusableMediator:public RefMediator, public ImmutableMediatorInterface {
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
class VarMediator: public RefMediator, public MonotonicMediatorInterface{
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
class LazyVarMediator: public RefMediator{
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
  public RefMediator, public MonotonicMediatorInterface {
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
