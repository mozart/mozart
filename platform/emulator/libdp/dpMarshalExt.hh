/* -*- C++ -*-
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Konstantin Popov, 2000
 * 
 *  Last change:
 *    $Date$
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

#ifndef __DPMARSHALEXT_HH
#define __DPMARSHALEXT_HH

#include "base.hh"
#include "dpBase.hh"
#include "extension.hh"
#include "gentraverser.hh"

//
void dpmExtInit();

//
// Unfortunately, we cann't have a virtaul method defined in the
// OZ_Extension class and implemented here (since this module is
// loaded dynamically), so here we go:
int dpMinNeededSpaceExt(OZ_Extension *oe);
Bool dpMarshalExt(ByteBuffer *bs, GenTraverser *gt,
		  OZ_Term oet, OZ_Extension *oe);

//
typedef _FUNTYPEDECL(OZ_Term, oz_suspUnmarshalProcType, (void*, GTAbstractEntity* &));
typedef _FUNTYPEDECL(OZ_Term, oz_unmarshalContProcType, (void*, GTAbstractEntity*));
//
_FUNDECL(OZ_Term,oz_extension_unmarshal,(int, void*,
					 GTAbstractEntity*&));
_FUNDECL(OZ_Term,oz_extension_unmarshalCont,(int, void*,
					     GTAbstractEntity*));

//
OZ_Term oz_extension_unmarshal(int type, void *bs,
			       GTAbstractEntity* &arg);
OZ_Term oz_extension_unmarshalCont(int type, void *bs,
				   GTAbstractEntity *arg);

//
void dpAddExtensions();

//
#if defined(DEBUG_CHECK)
void dpMarshalByteArrayCont(GenTraverser *gt, GTAbstractEntity *cont);
// DPMExtDesc *cont
#endif

#endif
