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
#include "dss_enums.hh"

// for the threads
void oz_thread_setDistVal(TaggedRef tr, int i, void* v); 
void* oz_thread_getDistVal(TaggedRef tr, int i); 



/************************* Globalization *************************/

// the most generic globalize function
void glue_globalizeEntity(TaggedRef entity) {
  Mediator *med = glue_getMediator(entity);
  if (!med->getAbstractEntity()) med->globalize();
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
  is a marshaled representation of a Dss abstract entity.  The entity
  tag (a GlueTag) describes the type of entity, and is followed by
  entity-specific data.

  The reason for putting entity data after the Dss data is: if the
  proxy unmarshaled by the Dss already exists, we don't need to create
  an emulator entity.  In that case, we simply have to remove the data
  from the buffer.

*/

// marshal an entity
void glue_marshalEntity(TaggedRef entity, ByteBuffer *bs) {
  Mediator *med = glue_getMediator(entity);
  if (!med->getAbstractEntity()) med->globalize();
  med->marshal(bs);
}

// generic marshaling
void Mediator::marshal(ByteBuffer *bs) {
  // first marshal Dss-specific stuff
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer*>(bs);
  absEntity->getCoordinatorAssistant()->marshal(gwb, PMF_ORDINARY);
  // then the entity type
  bs->put(getType());
  // by default there is no extra data
}

// specific for arrays
void ArrayMediator::marshal(ByteBuffer *bs) {
  OzArray *oza = static_cast<OzArray*>(getConst());
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer*>(bs);
  absEntity->getCoordinatorAssistant()->marshal(gwb, PMF_ORDINARY);
  bs->put(getType());
  marshalNumber(bs, oza->getLow());
  marshalNumber(bs, oza->getHigh());
}



/************************* Unmarshaling *************************/

// unmarshal an entity
OZ_Term glue_unmarshalEntity(ByteBuffer *bs) {
  AbstractEntity *ae;
  AbstractEntityName aen;
  GlueReadBuffer *grb = static_cast<GlueReadBuffer*>(bs);

  DSS_unmarshal_status stat = dss->unmarshalProxy(ae, grb, PUF_ORDINARY, aen);
  GlueTag tag = static_cast<GlueTag>(bs->get());

  if (stat.exist) {
    // drop entity-specific data
    switch (tag) {
    case GLUE_ARRAY:
      unmarshalNumber(bs); unmarshalNumber(bs); break;
    default:
      break;
    }
    // access to entity
    Mediator *med = dynamic_cast<Mediator*>(ae->accessMediator());
    return med->getEntity();

  } else {
    // build the entity and its mediator
    switch (tag) {
    case GLUE_LAZYCOPY: {
      OZ_error("Glue: unmarshaling lazy immutable not implemented yet");
      return makeTaggedNULL();
    }
    case GLUE_UNUSABLE: {
      UnusableResource *unused = new UnusableResource();
      TaggedRef ref = makeTaggedConst(unused);
      unused->setMediator(new UnusableMediator(ref, ae));
      return ref;
    }
    case GLUE_VARIABLE: {
      TaggedRef ref = oz_newSimpleVar(oz_currentBoard());
      OzVariable *var = tagged2Var(*tagged2Ref(ref));
      var->setMediator(new OzVariableMediator(ref, ae));
      return ref;
    }
    case GLUE_READONLY: {
      TaggedRef ref = oz_newReadOnly(oz_currentBoard());
      OzVariable *var = tagged2Var(*tagged2Ref(ref));
      var->setMediator(new OzVariableMediator(ref, ae));
      return ref;
    }
    case GLUE_PORT: {
      // the stream of the port is unused
      OzPort *port = new OzPort(oz_currentBoard(), makeTaggedNULL());
      port->setMediator(new PortMediator(port, ae));
      return makeTaggedConst(port);
    }
    case GLUE_CELL: {
      OzCell *cell = new OzCell(oz_currentBoard(), makeTaggedNULL());
      cell->setMediator(new CellMediator(cell, ae));
      return makeTaggedConst(cell);
    }
    case GLUE_LOCK: {
      OzLock *lock = new OzLock(oz_currentBoard());
      lock->setMediator(new LockMediator(lock, ae));
      return makeTaggedConst(lock);
    }
    case GLUE_OBJECT: {
      OZ_error("Glue: unmarshaling object not implemented yet");
      return makeTaggedNULL();
    }
    case GLUE_ARRAY: {
      int low  = unmarshalNumber(bs);
      int high = unmarshalNumber(bs);
      OzArray *oza = new OzArray(oz_currentBoard(), low, high,
				 makeTaggedNULL());
      oza->setMediator(new ArrayMediator(oza, ae));
      return makeTaggedConst(oza);
    }
    case GLUE_DICTIONARY: {
      OzDictionary *dict = new OzDictionary(oz_currentBoard());
      dict->setMediator(new DictionaryMediator(dict, ae));
      return makeTaggedConst(dict);
    }
    case GLUE_THREAD: {
      TaggedRef ref = oz_thread(oz_newThreadSuspended(1));
      oz_thread_setDistVal(ref, 0, new OzThreadMediator(ref, ae));
      return ref;
    }
    default:
      OZ_error("Glue: unmarshaling unknown entity type");
      return makeTaggedNULL();
    }
  }
}



/**********************************************************************/

///////////////////////////////////////////////////////////////////////////
////  Marshal of a Variable

void ProxyVar::marshal(ByteBuffer *bs, Bool hasIndex, TaggedRef* vRef, Bool push)
{
  Assert(getIdV() == OZ_EVAR_PROXY);
  MarshalTag tag = (isReadOnly() ? 
		    (hasIndex ? DIF_READONLY_DEF : DIF_READONLY) :
		    (hasIndex ? DIF_VAR_DEF : DIF_VAR));
  bs->put(tag);
  //  ProxyName pn=tr2Pn(makeTaggedRef(vRef));

  CoordinatorAssistantInterface  *cai = getMediator()->getCoordinatorAssistant();
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs); 
  cai->marshal(gwb, (push)?PMF_PUSH:PMF_ORDINARY);

}
