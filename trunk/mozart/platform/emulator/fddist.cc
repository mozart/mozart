/*
 *  Authors:
 *    Christian Schulte (schulte@dfki.de)
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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "fdbuilti.hh"

// ---------------------------------------------------------------------
//                  Finite Domains Distribution Built-ins
// ---------------------------------------------------------------------

#define getSize(var) \
  (isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().getSize() : 2)

#define getMin(var) \
  (isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().getMinElem() : 0)

#define getMax(var) \
  (isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().getMaxElem() : 1)

inline
static int getMid(TaggedRef var) {
  if (isGenFDVar(var)) {
    OZ_FiniteDomain &dom = tagged2GenFDVar(var)->getDom();
    return dom.getMidElem();
  } else {
    return 0;
  }
}

#define getConstraints(var) \
  (tagged2CVar(var)->getSuspListLength())

#define CheckVectorArg(i) \
  TaggedRef arg, d_arg;                             \
  arg   = vector->getArg(i);                        \
  d_arg = arg;                                      \
  DEREF(d_arg, p_arg, t_arg);                       \
  if (isSmallInt(t_arg)) continue;                  \
  vector->setArg(new_cur++, arg);                   \
  Assert(isGenFDVar(d_arg) || isGenBoolVar(d_arg));

OZ_C_proc_begin(BIfdDistribute, 5) { 
  TaggedRef tagged_vector = deref(OZ_getCArg(0));
  TaggedRef order_spec    = deref(OZ_getCArg(1));
  TaggedRef value_spec    = deref(OZ_getCArg(2));
  TaggedRef out_variable  = OZ_getCArg(3); 
  TaggedRef out_value     = OZ_getCArg(4); 
  TaggedRef variable, d_variable, value;

  if (!isSTuple(tagged_vector))
    return OZ_unify(out_value, makeTaggedSmallInt(-1));

  SRecord *vector = tagged2SRecord(tagged_vector);
  int      width  = vector->getWidth();
  int      cur    = 0;

  // Skip all elements which are small ints already
  do {
    variable   = vector->getArg(cur);
    d_variable = deref(variable);
  } while (isSmallInt(d_variable) && (++cur < width));

  // No elements left
  if (cur==width)
    return OZ_unify(out_value, makeTaggedSmallInt(-1));

  Assert(isGenFDVar(d_variable) || isGenBoolVar(d_variable));

  vector->setArg(0,variable);

  int new_cur = 1;

  if (literalEq(order_spec, AtomNaive)) {

    for (int i=cur+1; i<width; i++) {
      CheckVectorArg(i);
    }

  } else  if (literalEq(order_spec, AtomSize)) {
    int minsize = getSize(d_variable);

    for (int i=cur+1; i<width; i++) {
      CheckVectorArg(i);

      int cursize = getSize(d_arg);

      if (cursize < minsize) {
	minsize    = cursize;
	variable   = arg; 
	d_variable = d_arg;
      }
      
    }
  } else  if (literalEq(order_spec, AtomMax)) {
    int maxmax  = getMax(d_variable);

    for (int i=cur+1; i<width; i++) {
      CheckVectorArg(i);

      int curmax = getMax(d_arg);

      if (curmax > maxmax) {
	maxmax     = curmax;
	variable   = arg; 
	d_variable = d_arg;
      }
      
    }
  } else  if (literalEq(order_spec, AtomMin)) {
    int minmin  = getMin(d_variable);

    for (int i=cur+1; i<width; i++) {
      CheckVectorArg(i);

      int curmin = getMin(d_arg);

      if (curmin < minmin) {
	minmin     = curmin;
	variable   = arg; 
	d_variable = d_arg;
      }
      
    }
  } else  if (literalEq(order_spec, AtomNbSusps)) {
    int minsize = getSize(d_variable);
    int mincon  = getConstraints(d_variable);

    for (int i=cur+1; i<width; i++) {
      CheckVectorArg(i);
      
      int curcon  = getConstraints(d_arg);

      if (curcon < mincon)
	continue;

      if (curcon==mincon) {
	int cursize = getSize(d_arg);

	if (cursize < minsize) {
	  minsize = cursize;
	} else {
	  continue;
	}
      }

      mincon     = curcon;
      variable   = arg; 
      d_variable = d_arg;
      
    }

  }

  vector->downSize(new_cur);

  if (literalEq(value_spec, AtomMin)) {
    value = makeTaggedSmallInt(getMin(d_variable));
  } else if (literalEq(value_spec, AtomMax)) {
    value = makeTaggedSmallInt(getMax(d_variable));
  } else {
    Assert(literalEq(value_spec, AtomMid));
    value = makeTaggedSmallInt(getMid(d_variable));
  } 
 
  return (OZ_unify(out_variable, variable) &&
	  OZ_unify(out_value,    value)) ? PROCEED : FAILED;
} OZ_C_proc_end


#undef CheckVectorArg


