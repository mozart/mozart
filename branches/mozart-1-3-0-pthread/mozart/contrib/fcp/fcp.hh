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
extern int is_active;
inline
OZ_Return __first_class_prop(OZ_Return ret, OZ_Term fcp) {
  if (ret == OZ_FALSE) {
    return OZ_FALSE;
  } else {
    Assert(ret == OZ_SLEEP || ret == OZ_ENTAILED);
    OZ_Return r = OZ_unify(fcp, propagator2Term(imposed_propagator));
    if (r == FAILED) {
      return FAILED;
    }
  }
  return ret;
}

#define FIRST_CLASS_PROPAGATOR_OF(PROP, FC_PROP, ARITY_IN, ARITY_OUT)   \
OZ_BI_define (FC_PROP, ARITY_IN, ARITY_OUT)                             \
{                                                                       \
  return __first_class_prop(PROP(_OZ_LOC), OZ_in(ARITY_IN-1));          \
}                                                                       \
OZ_BI_end                                                               \
									\
OZ_BI_define (FC_PROP##_inactive, ARITY_IN, ARITY_OUT)                  \
{                                                                       \
  is_active = 0;                                                        \
  return __first_class_prop(PROP(_OZ_LOC), OZ_in(ARITY_IN-1));          \
}                                                                       \
OZ_BI_end

#endif /* __FCP_HH__ */
