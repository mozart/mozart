/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "fdbuilti.hh"


// ---------------------------------------------------------------------
//                  Finite Domains Distribution Built-ins
// ---------------------------------------------------------------------

inline
static int getSize(TaggedRef var) {
  return isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().getSize() : 2;
}

inline
static int getMin(TaggedRef var) {
  return isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().minElem() : 0;
}

inline
static int getMax(TaggedRef var) {
  return isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().maxElem() : 1;
}

inline
static int getMid(TaggedRef var) {
  if (isGenFDVar(var)) {
    OZ_FiniteDomain &dom = tagged2GenFDVar(var)->getDom();
    return dom.next((dom.minElem() + dom.maxElem()) / 2);
  } else {
    return 0;
  }
}

inline
static int getConstraints(TaggedRef var) {
  Assert(isCVar(var));
  return tagged2CVar(var)->getSuspListLength();
}


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

  if (cur!=0)
    vector->setArg(0,variable);

  if (literalEq(order_spec, AtomNaive) && (cur>0)) {
    for (int i=cur+1; i<width; i++)
      vector->setArg(i-cur,vector->getArg(i));

  } else  if (literalEq(order_spec, AtomSize)) {
    int minsize = getSize(d_variable);
    int new_cur = 1;

    for (int i=cur+1; i<width; i++) {
      TaggedRef   arg = vector->getArg(i);
      TaggedRef d_arg = deref(arg);

      if (isSmallInt(d_arg))
        continue;

      if (i != new_cur)
        vector->setArg(new_cur, arg);
      new_cur++;

      Assert(isGenFDVar(d_arg) || isGenBoolVar(d_arg));

      int cursize = getSize(d_arg);

      if (cursize < minsize) {
        minsize    = cursize;
        variable   = arg;
        d_variable = d_arg;
      }

    }
  } else  if (literalEq(order_spec, AtomMax)) {
    int maxmax  = getMax(d_variable);
    int new_cur = 1;

    for (int i=cur+1; i<width; i++) {
      TaggedRef   arg = vector->getArg(i);
      TaggedRef d_arg = deref(arg);

      if (isSmallInt(d_arg))
        continue;

      if (i != new_cur)
        vector->setArg(new_cur, arg);
      new_cur++;

      Assert(isGenFDVar(d_arg) || isGenBoolVar(d_arg));

      int curmax = getMax(d_arg);

      if (curmax < maxmax) {
        maxmax     = curmax;
        variable   = arg;
        d_variable = d_arg;
      }

    }
  } else  if (literalEq(order_spec, AtomMin)) {
    int minmin  = getMin(d_variable);
    int new_cur = 1;

    for (int i=cur+1; i<width; i++) {
      TaggedRef   arg = vector->getArg(i);
      TaggedRef d_arg = deref(arg);

      if (isSmallInt(d_arg))
        continue;

      if (i != new_cur)
        vector->setArg(new_cur, arg);
      new_cur++;

      Assert(isGenFDVar(d_arg) || isGenBoolVar(d_arg));

      int curmin = getSize(d_arg);

      if (curmin < minmin) {
        minmin     = curmin;
        variable   = arg;
        d_variable = d_arg;
      }

    }
  } else  if (literalEq(order_spec, AtomConstraints)) {
    int minsize = getSize(d_variable);
    int mincon  = getConstraints(d_variable);
    int new_cur = 1;

    for (int i=cur+1; i<width; i++) {
      TaggedRef   arg = vector->getArg(i);
      TaggedRef d_arg = deref(arg);

      if (isSmallInt(d_arg))
        continue;

      if (i != new_cur)
        vector->setArg(new_cur, arg);
      new_cur++;

      Assert(isGenFDVar(d_arg) || isGenBoolVar(d_arg));

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

    if (new_cur < width)
      vector->downSize(new_cur);
  }

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
