/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
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

#ifndef __GLUE_MARSHAL_HH
#define __GLUE_MARSHAL_HH

#ifdef INTERFACE
#pragma interface
#endif


#include "base.hh"
#include "value.hh"
class ProxyVar; 
OZ_Term glue_unmarshalDistTerm(ByteBuffer *bs);
OZ_Term glue_newUnmarshalVar(ByteBuffer* bs, Bool isFuture);
void glue_marshalTertiary(ByteBuffer *bs, Tertiary *t, Bool push);
OZ_Term glue_unmarshalObjectStub(ByteBuffer *bs);
void  glue_marshalObjectStubInternal(Object*,ByteBuffer *bs);
void glue_marshalArray(ByteBuffer *bs, ConstTermWithHome *arrayConst);
void glue_marshalDictionary(ByteBuffer *bs, ConstTermWithHome *dictConst);
void glue_marshalCell(ByteBuffer *bs, ConstTermWithHome *cellConst);
void glue_marshalUnusable(ByteBuffer *bs, TaggedRef tr); 
void  glue_marshalOzThread(ByteBuffer *bs, TaggedRef tr); 
/* GLobalization of the tertiaries and variables. */
/* Localization should in here as well. */ 

void glue_marshalOzVariable(ByteBuffer *bs, TaggedRef *vPtr,
			    Bool hasIndex, Bool push);
OZ_Term glue_unmarshalOzVariable(ByteBuffer* bs, Bool isReadOnly);

ProxyVar* glue_newGlobalizeFreeVariable(TaggedRef *tPtr); 
OzVariable *glue_globalizeOzVariable(TaggedRef *vPtr);
void globalizeTertiary(Tertiary *t);

#endif
