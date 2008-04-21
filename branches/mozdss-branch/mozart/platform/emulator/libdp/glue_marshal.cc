/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 *    Boris Mejias (bmc@info.ucl.ac.be)
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
#pragma implementation "glue_marshal.hh"
#endif

#include "glue_marshal.hh"
#include "glue_mediators.hh"
#include "glue_buffer.hh"
#include "glue_interface.hh"
#include "marshalerBase.hh"
#include "glue_tables.hh"
#include "thr_int.hh"
#include "value.hh"
#include "var_readonly.hh"
#include "dss_enums.hh"



/************************* Globalization *************************/

// the most generic globalize function
void glue_globalizeEntity(TaggedRef entity) {
  Mediator *med = glue_getMediator(entity);
  if (!med->getCoordinatorAssistant()) med->globalize();
}



/************************* Marshaling *************************/

/*
  The format of marshaled glue entities is the following:

  +----------+ - - - +----------+------------+-------------+
  | DIF_GLUE | index | Dss data | entity tag | entity data |
  +----------+ - - - +----------+------------+-------------+

  The first tag and optional index are handled outside of the glue.
  The tag is either DIF_GLUE (with no index), or DIF_GLUE_DEF (with
  index).

  The remaining part is glue-specific, and handled here.  The Dss data
  is a marshaled representation of the entity's coordination proxy,
  and the entity tag (a GlueTag) describes the mediator type.  Those
  two elements are handled inside the functions glue_marshalEntity()
  and glue_unmarshalEntity().

  The last part is entity-specific data, which is necessary to build a
  stub for an entity.  This part must be small in size, and should not
  contain the state of the entity.  This data is handled by the
  methods marshalData()/unmarshalData() of the entity's mediator.  The
  method unmarshalData() is in charge of building an entity stub, if
  it does not exist yet.  In case the entity exists, it simply
  disposes the data from the buffer.


  Note.  Sited entities cheat a bit with that scheme.  A sited entity
  marshals itself as an unusable (with the GlueTag GLUE_UNUSABLE),
  such that it is unmarshaled as an unusable resource on remote sites.
  Note also that sited entities do not serialize entity-specific data.
  Therefore one must be careful when unmarshaling a sited entity on
  its home site: the mediator should not try to unmarshal entity-
  specific data, because it is not present!

*/

// marshal an entity
bool glue_marshalEntity(TaggedRef entity, ByteBuffer *bs) {
  GlueWriteBuffer* buf = static_cast<GlueWriteBuffer*>(bs);
  Mediator* med = glue_getMediator(entity);
  if (!med->isDistributed()) med->globalize();

  // marshal coordination proxy
  bool immediate = med->getCoordinatorAssistant()->marshal(buf, PMF_ORDINARY);

  // marshal mediator type and entity-specific data
  if (med->getAnnotation().pn == PN_SITED) {
    // special case: sited entities marshal themselves as unusables
    bs->put(GLUE_UNUSABLE);
  } else {
    bs->put(med->getType());
    med->marshalData(bs);
  }

  return immediate;
}


// unmarshal an entity
bool glue_unmarshalEntity(ByteBuffer *bs, TaggedRef &entity) {
  GlueReadBuffer* buf = static_cast<GlueReadBuffer*>(bs);
  AbstractEntityName aen;
  bool immediate;
  CoordinatorAssistant* proxy;

  // unmarshal coordination proxy and mediator type
  proxy = dss->unmarshalProxy(buf, PUF_ORDINARY, aen, immediate);
  GlueTag tag = static_cast<GlueTag>(bs->get());

  // unmarshal entity-specific data
  Mediator* med = dynamic_cast<Mediator*>(proxy->getAbstractEntity());
  if (!med) { // create mediator
    med = glue_newMediator(tag);
    med->setProxy(proxy);
    med->unmarshalData(bs);
  } else {
    // Sited entities have nothing to unmarshal, so we do not call
    // unmarshalData() if the entity was marshaled as an unusable.
    // This avoids a sited procedure to unmarshal a GName, etc.
    if (tag != GLUE_UNUSABLE) med->unmarshalData(bs);
  }
  entity = med->getEntity();

  return immediate;
}


// marshaler hook
int glue_getMarshaledSize(TaggedRef entity) {
  Mediator* med = glue_getMediator(entity);
  if (!med->isDistributed()) med->globalize();

  int psize = med->getCoordinatorAssistant()->getMarshaledSize(PMF_ORDINARY);
  int esize = (med->getAnnotation().pn == PN_SITED ? 1 :
	       med->getMarshaledDataSize() + 1);
  return (psize + esize);
}


// marshaler hook
bool glue_isImmediate(TaggedRef entity) {
  return glue_getMediator(entity)->isImmediate();
}



/************************* Entity-specific stuff *************************/

// generic marshaling; by default there is no entity-specific data
void Mediator::marshalData(ByteBuffer *bs) {}
int Mediator::getMarshaledDataSize() const { return 0; }


// ports
void PortMediator::unmarshalData(ByteBuffer* bs) {
  if (!hasEntity()) setConst(new OzPort(oz_currentBoard(), makeTaggedNULL()));
}


// cells
void CellMediator::unmarshalData(ByteBuffer* bs) {
  if (!hasEntity()) setConst(new OzCell(oz_currentBoard(), makeTaggedNULL()));
}


// locks
void LockMediator::unmarshalData(ByteBuffer* bs) {
  if (!hasEntity()) setConst(new OzLock(oz_currentBoard()));
}


// arrays
void ArrayMediator::marshalData(ByteBuffer *bs) {
  OzArray *oza = static_cast<OzArray*>(getConst());
  marshalNumber(bs, oza->getLow());
  marshalNumber(bs, oza->getHigh());
}

void ArrayMediator::unmarshalData(ByteBuffer* bs) {
  int low  = unmarshalNumber(bs);
  int high = unmarshalNumber(bs);
  if (!hasEntity()) {
    setConst(new OzArray(oz_currentBoard(), low, high, makeTaggedNULL()));
  }
}

int ArrayMediator::getMarshaledDataSize() const {
  return 2 * MNumberMaxSize;
}


// dictionaries
void DictionaryMediator::unmarshalData(ByteBuffer* bs) {
  if (!hasEntity()) setConst(new OzDictionary(oz_currentBoard()));
}


// objects
void ObjectMediator::unmarshalData(ByteBuffer* bs) {
  if (!hasEntity()) setConst(new OzObject(oz_currentBoard()));
}

// object states
void ObjectStateMediator::unmarshalData(ByteBuffer* bs) {
  if (!hasEntity())
    setConst(new ObjectState(oz_currentBoard(), makeTaggedNULL()));
}


// thread ids
void OzThreadMediator::unmarshalData(ByteBuffer* bs) {
  if (!hasEntity()) setEntity(oz_thread(oz_newThreadSuspended()));
}


// variables
void OzVariableMediator::unmarshalData(ByteBuffer* bs) {
  if (!hasEntity()) {
    switch (getType()) {
    case GLUE_VARIABLE:
      setEntity(oz_newSimpleVar(oz_currentBoard())); attach();
      break;
    case GLUE_READONLY:
      setEntity(oz_newReadOnly(oz_currentBoard())); attach();
      break;
    default:
      Assert(0);
    }
  }
}


// unusables
void UnusableMediator::unmarshalData(ByteBuffer* bs) {
  if (!hasEntity()) setConst(new UnusableResource());
}


// chunks
void ChunkMediator::marshalData(ByteBuffer *bs) {
  SChunk* chunk = static_cast<SChunk*>(getConst());
  GName* gname = chunk->globalize();
  Assert(gname);
  marshalGName(bs, gname);
}

void ChunkMediator::unmarshalData(ByteBuffer* bs) {
  TaggedRef value;
  GName* gname = unmarshalGName(&value, bs);
  if (!hasEntity()) {
    if (gname) {   // entity does not exist on this site
      SChunk* chunk = new SChunk(oz_currentBoard(), makeTaggedNULL());
      chunk->setGName(gname);
      value = makeTaggedConst(chunk);
      addGName(gname, value);
    }
    Assert(oz_isSChunk(value));
    setEntity(value);
  }
}

int ChunkMediator::getMarshaledDataSize() const {
  return MGNameMaxSize;
}


// classes
void ClassMediator::marshalData(ByteBuffer *bs) {
  OzClass* cls = static_cast<OzClass*>(getConst());
  GName* gname = cls->globalize();
  Assert(gname);
  marshalGName(bs, gname);
}

void ClassMediator::unmarshalData(ByteBuffer* bs) {
  TaggedRef value;
  GName* gname = unmarshalGName(&value, bs);
  if (!hasEntity()) {
    if (gname) {   // entity does not exist on this site
      OzClass* cls = new OzClass(makeTaggedNULL(), 
				 makeTaggedNULL(),
				 makeTaggedNULL(), 
				 makeTaggedNULL(), NO, NO,
				 oz_currentBoard());
      cls->setGName(gname);
      value = makeTaggedConst(cls);
      addGName(gname, value);
    }
    Assert(oz_isClass(value));
    setEntity(value);
  }
}

int ClassMediator::getMarshaledDataSize() const {
  return MGNameMaxSize;
}


// procedures
void ProcedureMediator::marshalData(ByteBuffer *bs) {
  Abstraction* a = tagged2Abstraction(getEntity());
  GName* gname = a->globalize();
  Assert(gname);
  marshalGName(bs, gname);
  marshalNumber(bs, a->getArity());
}

void ProcedureMediator::unmarshalData(ByteBuffer* bs) {
  TaggedRef value;
  GName* gname = unmarshalGName(&value, bs);
  int arity = unmarshalNumber(bs);
  if (!hasEntity()) {
    if (gname) {   // entity does not exist on this site
      Abstraction* a = new Abstraction(oz_currentBoard(), arity);
      a->setGName(gname);
      value = makeTaggedConst(a);
      addGName(gname, value);
    }
    Assert(oz_isAbstraction(value));
    setEntity(value);
  }
}

int ProcedureMediator::getMarshaledDataSize() const {
  return MGNameMaxSize + MNumberMaxSize;
}
