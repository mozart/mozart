/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
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

#ifndef __FCP_HH__
#define __FCP_HH__

#include <assert.h>

#include "reflect.hh"
#include "mozart_cpi.hh"

extern Propagator * imposed_propagator;

#define FIRST_CLASS_PROPAGATOR_OF(PROP, FC_PROP, ARITY_IN, ARITY_OUT)	\
OZ_BI_define (FC_PROP, ARITY_IN, ARITY_OUT)				\
{									\
  OZ_Return ret = PROP(_OZ_LOC);				        \
  if (ret == OZ_FALSE) {						\
    return OZ_FALSE;							\
  } else {								\
    Assert(ret == OZ_SLEEP || ret == OZ_ENTAILED);			\
    OZ_Return r = OZ_unify(OZ_in(ARITY_IN-1),				\
			   propagator2Term(imposed_propagator));	\
    if (r == FAILED) {							\
      return FAILED;							\
    }									\
  }									\
  return ret;								\
}									\
OZ_BI_end

#endif /* __FCP_HH__ */
