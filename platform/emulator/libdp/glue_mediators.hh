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

  - A mediator is ATTACHED when its corresponding entity has a direct
  pointer to it.  Otherwise is is DETACHED, and should be looked up in
  the mediator table (using the entity's taggedref as a key).
  Attached mediators are not found by looking up the mediator table.

  Note 1. Upon creation, the mediator may or may not know its entity
  (for instance, when unmarshaling).  Insertion of the mediator in the
  mediator table is automatic once the entity is known.  Connection to
  the coordination proxy is also automatic upon globalization.

  Note 2. Mediators of failed entities should be attached, otherwise
  the emulator might not notice the failure.  This is because we only
  check for failure if the entity has an attached mediator.

 */



// the types of entities handled by the glue.  Those tags are also
// used as marshaling tags.
enum GlueTag {
  GLUE_NONE = 0,       // 
  GLUE_PORT,           // mutables
  GLUE_CELL,
  GLUE_LOCK,
  GLUE_OBJECTSTATE,
  GLUE_ARRAY,
  GLUE_DICTIONARY,
  GLUE_THREAD,
  GLUE_VARIABLE,       // transients
  GLUE_READONLY,
  GLUE_UNUSABLE,       // immutables
  GLUE_CHUNK,
  GLUE_CLASS,
  GLUE_OBJECT,
  GLUE_PROCEDURE,
  GLUE_LAST            // must be last
};

// attached or detached
#define DETACHED 0
#define ATTACHED 1



// Mediator is the abstract class for all mediators.  Mediator extends
// class AbstractEntity, and each concrete mediator class extends one
// of Mutable/Monotonic/Immutable AbstractEntity.

class Mediator : public virtual AbstractEntity {
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
  Mediator(GlueTag type);
  virtual ~Mediator();

  /*************** active/passive, attached/detached ***************/
  bool isActive() const { return active; }
  void makePassive();

  bool isAttached() const { return attached; }
  virtual void attach() {}     // attach mediator if possible
  virtual void detach() {}     // detach mediator if possible

  /*************** entity/coordinator ***************/
  bool      hasEntity() const { return entity != makeTaggedNULL(); }
  TaggedRef getEntity() const { return entity; }
  void      setEntity(TaggedRef e);

  void setProxy(CoordinatorAssistant*);
  bool isDistributed() const { return getCoordinatorAssistant() != NULL; }

  // Note. Use setProxy() instead of setCoordinatorAssistant().

  /*************** annotate/globalize/localize ***************/
  GlueTag getType() const { return type; }

  Annotation getAnnotation() const { return annotation; }
  void setAnnotation(const Annotation& a) { annotation = a; }
  void completeAnnotation();
  virtual bool annotate(Annotation);     // return true if successful

  // hook for the marshaler: complete the annotation if necessary, and
  // return true if the immediate protocol is used.
  bool isImmediate();

  void globalize();     // create coordination proxy
  void localize();      // localize entity

  /*************** garbage collection ***************/
  virtual void gCollectPrepare();     // preliminary stuff...
  bool isCollected() const { return collected; }
  void gCollect();            // collect mediator (idempotent)
  void checkGCollect();       // collect if entity marked
  void resetGCStatus();       // reset the gc status
  DSS_GC getDssGCStatus();    // ask and return dss gc status

  // Note: In order to specialize gCollect(), make it a virtual method.

  /*************** fault handling ***************/
  GlueFaultState getFaultState() const { return faultState; }
  void           setFaultState(GlueFaultState fs);
  TaggedRef      getFaultStreamTail();
  TaggedRef      getFaultStream();
  OZ_Return      suspendOnFault();     // suspend on control var

  virtual void reportFaultState(const FaultState&);

  /*************** marshaling, see glue_marshal.hh ***************/
  virtual void marshalData(ByteBuffer*);
  virtual void unmarshalData(ByteBuffer*) = 0;
  virtual int  getMarshaledDataSize() const;

  /*************** debugging ***************/
  virtual char* getPrintType() { return "unknown"; }
  void print();
};



// create a new mediator for a given entity type (set entity later)
Mediator *glue_newMediator(GlueTag);

// return the mediator of an entity (create one if necessary)
Mediator *glue_getMediator(TaggedRef entity);

// check the validity of a protocol for a given type
bool glue_validProtocol(const ProtocolName pn, const GlueTag type);



// ConstMediator is an abstract class for mediators referring to a
// ConstTermWithHome in the emulator.
class ConstMediator: public Mediator {
public: 
  ConstMediator(GlueTag type);
  ConstTermWithHome* getConst() const;
  void setConst(ConstTermWithHome* c) { setEntity(makeTaggedConst(c)); }
  virtual void attach();
  virtual void detach();
  virtual char *getPrintType() { return "const"; }
};



// mediators for Oz ports
class PortMediator: public ConstMediator, public RelaxedMutableAbstractEntity {
public:
  PortMediator();
  PortMediator(TaggedRef);

  virtual void callback_Write(DssThreadId*,
				    PstInContainerInterface*);
  virtual void callback_Read(DssThreadId*,
				   PstInContainerInterface*,
				   PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation() {
    Assert(0); return NULL; }
  virtual void installEntityRepresentation(PstInContainerInterface*) {
    Assert(0); }
  virtual void unmarshalData(ByteBuffer*);
  virtual char *getPrintType() { return "port"; }
}; 


// mediators for Oz cells
class CellMediator: public ConstMediator, public MutableAbstractEntity {
public:
  CellMediator();
  CellMediator(TaggedRef);

  void callback(DssThreadId*,
		PstInContainerInterface*, PstOutContainerInterface*&);
  virtual void callback_Write(DssThreadId*,
			      PstInContainerInterface*,
			      PstOutContainerInterface*&);
  virtual void callback_Read(DssThreadId*,
			     PstInContainerInterface*,
			     PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual PstOutContainerInterface *deinstallEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
  virtual void unmarshalData(ByteBuffer*);
  virtual char *getPrintType() { return "cell"; }
}; 


// mediators for Oz locks
class LockMediator: public ConstMediator, public MutableAbstractEntity {
public:
  LockMediator();
  LockMediator(TaggedRef);

  virtual void callback_Write(DssThreadId*,
			      PstInContainerInterface*,
			      PstOutContainerInterface*&);
  virtual void callback_Read(DssThreadId*,
			     PstInContainerInterface*,
			     PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual PstOutContainerInterface *deinstallEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
  virtual void unmarshalData(ByteBuffer*);
  virtual char *getPrintType() { return "lock"; }
}; 


// mediators for Oz arrays
class ArrayMediator: public ConstMediator, public MutableAbstractEntity {
public:
  ArrayMediator();
  ArrayMediator(TaggedRef);

  void callback(DssThreadId*,
		PstInContainerInterface*, PstOutContainerInterface*&);
  virtual void callback_Write(DssThreadId*,
			      PstInContainerInterface*,
			      PstOutContainerInterface*&);
  virtual void callback_Read(DssThreadId*,
				   PstInContainerInterface*,
				   PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual PstOutContainerInterface *deinstallEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
  virtual void marshalData(ByteBuffer*);
  virtual void unmarshalData(ByteBuffer*);
  virtual int  getMarshaledDataSize() const;
  virtual char *getPrintType() { return "array"; }
}; 


// mediators for Oz dictionaries
class DictionaryMediator: public ConstMediator, public MutableAbstractEntity {
public:
  DictionaryMediator();
  DictionaryMediator(TaggedRef);

  void callback(DssThreadId*,
		PstInContainerInterface*, PstOutContainerInterface*&);
  virtual void callback_Write(DssThreadId*,
			      PstInContainerInterface*,
			      PstOutContainerInterface*&);
  virtual void callback_Read(DssThreadId*,
			     PstInContainerInterface*,
			     PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual PstOutContainerInterface *deinstallEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
  virtual void unmarshalData(ByteBuffer*);
  virtual char *getPrintType() { return "dictionary"; }
}; 


// mediators for Oz objects
class ObjectMediator: public ConstMediator, public ImmutableAbstractEntity {
public:
  ObjectMediator();
  ObjectMediator(TaggedRef);

  virtual bool annotate(Annotation);
  virtual void callback_Read(DssThreadId*,
			     PstInContainerInterface*,
			     PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
  virtual void unmarshalData(ByteBuffer*);
  virtual char *getPrintType() { return "object"; }
};


// mediators for objects' state
class ObjectStateMediator: public ConstMediator, public MutableAbstractEntity {
public:
  ObjectStateMediator();
  ObjectStateMediator(TaggedRef);

  void callback(DssThreadId*,
		PstInContainerInterface*, PstOutContainerInterface*&);
  virtual void callback_Write(DssThreadId*,
			      PstInContainerInterface*,
			      PstOutContainerInterface*&);
  virtual void callback_Read(DssThreadId*,
			     PstInContainerInterface*,
			     PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual PstOutContainerInterface *deinstallEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
  virtual void unmarshalData(ByteBuffer*);
  virtual char *getPrintType() { return "object state"; }
};


// mediators for Oz threads
class OzThreadMediator: public ConstMediator, public MutableAbstractEntity {
public:
  OzThreadMediator();
  OzThreadMediator(TaggedRef);

  virtual void callback_Write(DssThreadId*,
			      PstInContainerInterface*,
			      PstOutContainerInterface*&);
  virtual void callback_Read(DssThreadId*,
			     PstInContainerInterface*,
			     PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*); 
  virtual void unmarshalData(ByteBuffer*);
  virtual char *getPrintType() { return "thread"; }
};



// mediators for Oz variables
class OzVariableMediator: public Mediator, public MonotonicAbstractEntity {
public:
  OzVariableMediator(GlueTag);
  OzVariableMediator(TaggedRef);
  
  virtual void attach();
  virtual void gCollectPrepare();   // overrides Mediator::gCollectPrepare()
  virtual char *getPrintType() { return "var"; }

  void bind(TaggedRef);     // bind the variable and its fault stream

  virtual void callback_Bind(PstInContainerInterface* operation); 
  virtual void callback_Append(PstInContainerInterface* operation);
  virtual void callback_Changes(PstOutContainerInterface*& answer);
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*);
  virtual void unmarshalData(ByteBuffer*);
}; 

// Note.  Patched variables keep their distribution support alive,
// even when the variable is bound.  The DistributedVarPatches do
// collect their mediators, which forces them to keep the coordination
// proxy alive.



// mediators for "unusables".  An unusable is a remote representation
// of a site-specific entity.
class UnusableMediator: public ConstMediator, public ImmutableAbstractEntity {
public:
  UnusableMediator();
  UnusableMediator(TaggedRef);

  virtual void callback_Read(DssThreadId*,
			     PstInContainerInterface*,
			     PstOutContainerInterface*&);
  virtual PstOutContainerInterface *retrieveEntityRepresentation() {
    Assert(0); return NULL; }
  virtual void installEntityRepresentation(PstInContainerInterface*) {
    Assert(0); }
  virtual void unmarshalData(ByteBuffer*);
  virtual char *getPrintType() { return "unusable"; }
};



// The following mediators are for immutables with a GName.  For those
// values, we have to keep the GName in order to maintain reference
// integrity.  This is because those values are not dependent on any
// site (unless they are explicitly sited, or stationary); they can be
// pickled to a file, for instance.
//
// A consequence of this fact is that those mediators must remain
// detached.  Indeed, the value already has a reference to its GName.
// We cannot remove this reference, because otherwise the pickler may
// create a second GName for that same entity: the pickler simply
// ignores the existence of mediators!
//
//                          Entity <---> GName
//                            ^
//                            |
//                         Mediator

// abstract base class for immutables with a gname ("tokens")
class TokenMediator : public ConstMediator, public ImmutableAbstractEntity {
public:
  TokenMediator(GlueTag);
  virtual void attach() {}     // always detached (because of gname)
  virtual void detach() {}
  virtual PstOutContainerInterface *retrieveEntityRepresentation();
  virtual void installEntityRepresentation(PstInContainerInterface*);
};

// mediators for chunks
class ChunkMediator : public TokenMediator {
public:
  ChunkMediator();
  ChunkMediator(TaggedRef);

  virtual void callback_Read(DssThreadId*,
			     PstInContainerInterface*,
			     PstOutContainerInterface*&);
  virtual void marshalData(ByteBuffer*);
  virtual void unmarshalData(ByteBuffer*);
  virtual int  getMarshaledDataSize() const;
  virtual char *getPrintType() { return "chunk"; }
};

// mediators for classes
class ClassMediator : public TokenMediator {
public:
  ClassMediator();
  ClassMediator(TaggedRef);

  virtual void callback_Read(DssThreadId*,
			     PstInContainerInterface*,
			     PstOutContainerInterface*&);
  virtual void marshalData(ByteBuffer*);
  virtual void unmarshalData(ByteBuffer*);
  virtual int  getMarshaledDataSize() const;
  virtual char *getPrintType() { return "class"; }
};

// mediators for procedures (Abstraction)
class ProcedureMediator : public TokenMediator {
public:
  ProcedureMediator();
  ProcedureMediator(TaggedRef);

  virtual void callback_Read(DssThreadId*,
			     PstInContainerInterface*,
			     PstOutContainerInterface*&);
  virtual void marshalData(ByteBuffer*);
  virtual void unmarshalData(ByteBuffer*);
  virtual int  getMarshaledDataSize() const;
  virtual char *getPrintType() { return "procedure"; }
};



// utils: generic wrapping of operation parameters for the DSS
inline
TaggedRef glue_wrap(int op, int n, TaggedRef* arg) {
  // build the record: '#'(op arg1 ... argn)
  SRecord* rec = SRecord::newSRecord(AtomPair, n + 1);
  rec->setArg(0, oz_int(op));
  while (n--) rec->setArg(n + 1, arg[n]);
  return makeTaggedSRecord(rec);
}

inline
int glue_getOp(TaggedRef msg) {
  return oz_intToC(tagged2SRecord(msg)->getArg(0));
}

inline
TaggedRef* glue_getArgs(TaggedRef msg) {
  return tagged2SRecord(msg)->getRef(1);
}

// generic wrapping of operation results for the DSS
inline
TaggedRef glue_return(TaggedRef res) {
  return OZ_mkTuple(AtomOk, 1, res);
}

inline
bool glue_isReturn(TaggedRef msg) {
  return tagged2SRecord(msg)->getLabel() == AtomOk;
}

inline
TaggedRef glue_raise(TaggedRef exc) {
  return OZ_mkTuple(AtomFailed, 1, exc);
}

inline
bool glue_isRaise(TaggedRef msg) {
  return tagged2SRecord(msg)->getLabel() == AtomFailed;
}

inline
TaggedRef glue_getData(TaggedRef msg) {   // result or exception
  return tagged2SRecord(msg)->getArg(0);
}

#endif
