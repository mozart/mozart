/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
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

#ifndef __GLUE_MARSHAL_HH
#define __GLUE_MARSHAL_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "value.hh"
#include "gentraverser.hh"     // for GTAbstractEntity

// globalize an entity
void glue_globalizeEntity(TaggedRef entity);

// marshal/unmarshal an entity.  Those functions return true if the
// protocol is immediate, i.e., the entity's state is marshaled
// together with the glue reference.
bool glue_marshalEntity(TaggedRef entity, ByteBuffer *bs);
bool glue_unmarshalEntity(ByteBuffer *bs, TaggedRef &entity);

// hook for the marshaler: return true if the protocol is immediate
bool glue_isImmediate(TaggedRef entity);

#endif
