/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Copyright:
 *    Christian Schulte, 1998
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

#include "../builtins.hh"
#include "../var_base.hh"
#include "../var_fd.hh"
#include "../var_bool.hh"

// ---------------------------------------------------------------------
//                  Finite Domains Distribution Built-ins
// ---------------------------------------------------------------------

inline
static int getSize(TaggedRef var) {
  return isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().getSize() : 2;
}

inline
static int getMin(TaggedRef var) {
  return isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().getMinElem() : 0;
}

inline
static int getMax(TaggedRef var) {
  return isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().getMaxElem() : 1;
}

inline
static int getMid(TaggedRef var) {
  if (isGenFDVar(var)) {
    OZ_FiniteDomain &dom = tagged2GenFDVar(var)->getDom();
    return dom.getMidElem();
  } else {
    return 0;
  }
}

inline
static int getConstraints(TaggedRef var) {
  return oz_var_getSuspListLength(tagged2CVar(var));
}


inline
static int discard_determined(SRecord * vec) {
  // Discard all elements which are small ints already
  register int j = 0;
  register int w = vec->getWidth();

  for (int i = 0; i<w; i++) {
    TaggedRef var   = vec->getArg(i);
    TaggedRef d_var = oz_deref(var);

    if (!oz_isSmallInt(d_var)) {
      Assert(isGenFDVar(d_var) || isGenBoolVar(d_var));
      vec->setArg(j++, var);
    }

  }

  if (j)
    vec->downSize(j);

  return j;
}


OZ_BI_define(BIfdd_selVarNaive, 1, 1) {
  oz_declareSTupleIN(0, vec);

  int w = discard_determined(vec);

  // No elements left
  if (!w)
    return OZ_raise(makeTaggedSmallInt(-1));

  OZ_RETURN(vec->getArg(0));
} OZ_BI_end


OZ_BI_define(BIfdd_selVarSize, 1, 1) {
  oz_declareSTupleIN(0, vec);

  int w = discard_determined(vec);

  // No elements left
  if (!w)
    return OZ_raise(makeTaggedSmallInt(-1));


  TaggedRef var = vec->getArg(0);

  int minsize = getSize(oz_deref(var));

  for (int i=1; i<w; i++) {
    TaggedRef arg   = vec->getArg(i);
    TaggedRef d_arg = oz_deref(arg);

    int cursize = getSize(d_arg);

    if (cursize < minsize) {
      minsize = cursize;
      var     = arg;
    }

  }

  OZ_RETURN(var);
} OZ_BI_end


OZ_BI_define(BIfdd_selVarMax, 1, 1) {
  oz_declareSTupleIN(0, vec);

  int w = discard_determined(vec);

  // No elements left
  if (!w)
    return OZ_raise(makeTaggedSmallInt(-1));

  TaggedRef var = vec->getArg(0);
  int maxmax    = getMax(oz_deref(var));

  for (int i=1; i<w; i++) {
    TaggedRef arg   = vec->getArg(i);
    TaggedRef d_arg = oz_deref(arg);

    int curmax = getMax(d_arg);

    if (curmax > maxmax) {
      maxmax = curmax;
      var    = arg;
    }

  }

  OZ_RETURN(var);
} OZ_BI_end


OZ_BI_define(BIfdd_selVarMin, 1, 1) {
  oz_declareSTupleIN(0, vec);

  int w = discard_determined(vec);

  // No elements left
  if (!w)
    return OZ_raise(makeTaggedSmallInt(-1));

  TaggedRef var = vec->getArg(0);
  int minmin    = getMin(oz_deref(var));

  for (int i=1; i<w; i++) {
    TaggedRef arg   = vec->getArg(i);
    TaggedRef d_arg = oz_deref(arg);

    int curmin = getMin(d_arg);

    if (curmin < minmin) {
      minmin = curmin;
      var    = arg;
    }

  }

  OZ_RETURN(var);
} OZ_BI_end


OZ_BI_define(BIfdd_selVarNbSusps, 1, 1) {
  oz_declareSTupleIN(0, vec);

  int w = discard_determined(vec);

  // No elements left
  if (!w)
    return OZ_raise(makeTaggedSmallInt(-1));

  TaggedRef var   = vec->getArg(0);
  TaggedRef d_var = oz_deref(var);

  int minsize = getSize(d_var);
  int maxcon  = getConstraints(d_var);

  for (int i=1; i<w; i++) {
    TaggedRef arg   = vec->getArg(i);
    TaggedRef d_arg = oz_deref(arg);

    int curcon  = getConstraints(d_arg);

    if (curcon < maxcon)
      continue;

    if (curcon==maxcon) {
      int cursize = getSize(d_arg);

      if (cursize < minsize) {
        minsize = cursize;
      } else {
        continue;
      }
    }

    maxcon = curcon;
    var    = arg;

  }

  OZ_RETURN(var);
} OZ_BI_end
