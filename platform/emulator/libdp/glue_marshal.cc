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
#include "glue_entities.hh"
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
  methods marshal()/unmarshal() of the entity's mediator.  The method
  unmarshal() is in charge of building an entity stub, if it does not
  exist yet.  In case the entity exists, it simply disposes the data
  from the buffer.

*/

// marshal an entity
void glue_marshalEntity(TaggedRef entity, ByteBuffer *bs) {
  GlueWriteBuffer* buf = static_cast<GlueWriteBuffer*>(bs);
  Mediator* med = glue_getMediator(entity);
  if (!med->isDistributed()) med->globalize();

  // marshal coordination proxy and mediator type
  bool immediate = med->getCoordinatorAssistant()->marshal(buf, PMF_ORDINARY);
  bs->put(med->getType());

  // marshal entity-specific data
  med->marshal(bs);
}


// unmarshal an entity
OZ_Term glue_unmarshalEntity(ByteBuffer *bs) {
  GlueReadBuffer* buf = static_cast<GlueReadBuffer*>(bs);
  AbstractEntityName aen;
  bool immediate;
  CoordinatorAssistant* proxy;

  // unmarshal coordination proxy and mediator type
  proxy = dss->unmarshalProxy(buf, PUF_ORDINARY, aen, immediate);
  GlueTag tag = static_cast<GlueTag>(bs->get());

  Mediator* med = dynamic_cast<Mediator*>(proxy->getAbstractEntity());
  if (!med) { // create mediator
    med = glue_newMediator(tag);
    med->setProxy(proxy);
  }
  med->unmarshal(bs);

  return med->getEntity();
}



/************************* Entity-specific stuff *************************/

// generic marshaling; by default there is no entity-specific data
void Mediator::marshal(ByteBuffer *bs) {}


// ports
void PortMediator::unmarshal(ByteBuffer* bs) {
  if (!hasEntity()) setConst(new OzPort(oz_currentBoard(), makeTaggedNULL()));
}


// cells
void CellMediator::unmarshal(ByteBuffer* bs) {
  if (!hasEntity()) setConst(new OzCell(oz_currentBoard(), makeTaggedNULL()));
}


// locks
void LockMediator::unmarshal(ByteBuffer* bs) {
  if (!hasEntity()) setConst(new OzLock(oz_currentBoard()));
}


// arrays
void ArrayMediator::marshal(ByteBuffer *bs) {
  OzArray *oza = static_cast<OzArray*>(getConst());
  marshalNumber(bs, oza->getLow());
  marshalNumber(bs, oza->getHigh());
}

void ArrayMediator::unmarshal(ByteBuffer* bs) {
  int low  = unmarshalNumber(bs);
  int high = unmarshalNumber(bs);
  if (!hasEntity()) {
    setConst(new OzArray(oz_currentBoard(), low, high, makeTaggedNULL()));
  }
}


// dictionaries
void DictionaryMediator::unmarshal(ByteBuffer* bs) {
  if (!hasEntity()) setConst(new OzDictionary(oz_currentBoard()));
}


// objects
void ObjectMediator::unmarshal(ByteBuffer* bs) {
  // not done yet
  if (!hasEntity()) { Assert(0); }
}


// thread ids
void OzThreadMediator::unmarshal(ByteBuffer* bs) {
  if (!hasEntity()) setEntity(oz_thread(oz_newThreadSuspended(1)));
}


// unusables
void UnusableMediator::unmarshal(ByteBuffer* bs) {
  if (!hasEntity()) setConst(new UnusableResource());
}


// variables
void OzVariableMediator::unmarshal(ByteBuffer* bs) {
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
