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

// specific for objects
void ObjectMediator::marshal(ByteBuffer *bs) {
  OzObject *ozo = static_cast<OzObject*>(getConst());
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer*>(bs);
  absEntity->getCoordinatorAssistant()->marshal(gwb, PMF_ORDINARY);
  bs->put(getType());
  // We also marshal the global name of the class
  OzClass *oc = ozo->getClass();
  GName *gnclass = oc->globalize();
  Assert(gnclass);
  marshalGName(bs, gnclass);
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
      unmarshalNumber(bs); 
      unmarshalNumber(bs); 
      break;
    case GLUE_OBJECT:
      TaggedRef gnclass;
      unmarshalGName(&gnclass, bs);
      break;
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
      TaggedRef ref = makeTaggedConst(port);
      port->setMediator(new PortMediator(ref, ae));
      return ref;
    }
    case GLUE_CELL: {
      OzCell *cell = new OzCell(oz_currentBoard(), makeTaggedNULL());
      TaggedRef ref = makeTaggedConst(cell);
      cell->setMediator(new CellMediator(ref, ae));
      return ref;
    }
    case GLUE_LOCK: {
      OzLock *lock = new OzLock(oz_currentBoard());
      TaggedRef ref = makeTaggedConst(lock);
      lock->setMediator(new LockMediator(ref, ae));
      return ref;
    }
    case GLUE_OBJECT: {
      //OZ_error("Glue: unmarshaling object not implemented yet");
      // First unmarshal the class
      OZ_Term clas;
      GName *gnclass = unmarshalGName(&clas, bs);
      printf("Hasta aqui vamos bien dijo el chancho entrando en la puerta del horno\n"); //bmc
      OzObject *obj = new OzObject(oz_currentBoard(), 
                                   gnclass, makeTaggedNULL(), makeTaggedNULL(), makeTaggedNULL());
      printf("Object created, with only a gname as a class\n"); //bmc
      obj->setMediator(new ObjectMediator(makeTaggedConst(obj), ae));
      printf("Ready to Return the object\n"); //bmc      
      return makeTaggedConst(obj);
    }
    case GLUE_ARRAY: {
      int low  = unmarshalNumber(bs);
      int high = unmarshalNumber(bs);
      OzArray *oza = new OzArray(oz_currentBoard(), low, high,
				 makeTaggedNULL());
      TaggedRef ref = makeTaggedConst(oza);
      oza->setMediator(new ArrayMediator(ref, ae));
      return ref;
    }
    case GLUE_DICTIONARY: {
      OzDictionary *ozd = new OzDictionary(oz_currentBoard());
      TaggedRef ref = makeTaggedConst(ozd);
      ozd->setMediator(new DictionaryMediator(ref, ae));
      return ref;
    }
    case GLUE_THREAD: {
      TaggedRef ref = oz_thread(oz_newThreadSuspended(1));
      ConstTermWithHome* ctwh =
        static_cast<ConstTermWithHome*>(tagged2Const(ref));
      ctwh->setMediator(new OzThreadMediator(ref, ae));
      return ref;
    }
    default:
      OZ_error("Glue: unmarshaling unknown entity type");
      return makeTaggedNULL();
    }
  }
}
