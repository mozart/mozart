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

#include "cpi.hh"
#include "var_fd.hh"
#include "var_bool.hh"

//-----------------------------------------------------------------------------
// fd built-ins don't cause stuck threads, but may cause their minds, therefore

#undef FDBISTUCK

#ifdef FDBISTUCK

#define SUSPEND_PROCEED SUSPEND

#else

#define SUSPEND_PROCEED PROCEED

#endif


#ifdef FDBISTUCK

inline
OZ_Return BI_FD_suspendOnVar(OZ_CFun, int, OZ_Term *,
                             OZ_Term * t)
{
  OZ_suspendOn(makeTaggedRef(t));
}

OZ_Return BI_FD_suspendOnVar(OZ_CFun, int, OZ_Term *,
                             OZ_Term * t1, OZ_Term * t2)
{
  OZ_suspendOn2(makeTaggedRef(t1),
                makeTaggedRef(t2));
}

#else

inline
OZ_Return BI_FD_suspendOnVar(OZ_CFun f, int a, OZ_Term * x,
                             OZ_Term * t)
{
  OZ_addThread(makeTaggedRef(t), OZ_makeSuspendedThread(f, x, a));
  return PROCEED;
}

inline
OZ_Return BI_FD_suspendOnVar(OZ_CFun f, int a, OZ_Term * x,
                             OZ_Term * t1, OZ_Term * t2)
{
  OZ_Thread th = OZ_makeSuspendedThread(f, x, a);
  OZ_addThread(makeTaggedRef(t1), th);
  OZ_addThread(makeTaggedRef(t2), th);
  return PROCEED;
}

#endif

// ---------------------------------------------------------------------
// misc stuff

OZ_BI_define(BIisFdVar, 1,0)
{
  return isGenFDVar(oz_deref(OZ_in(0)))
    || isGenBoolVar(oz_deref(OZ_in(0))) ? PROCEED : FAILED;
} OZ_BI_end

OZ_BI_define(BIisFdVarB, 1,1)
{
  OZ_RETURN(oz_bool(isGenFDVar(oz_deref(OZ_in(0))) ||
                    isGenBoolVar(oz_deref(OZ_in(0)))));
} OZ_BI_end

OZ_BI_define(BIgetFDLimits, 0,2)
{
  OZ_out(0) = oz_int(0);
  OZ_out(1) = oz_int(fd_sup);
  return PROCEED;
} OZ_BI_end

OZ_C_proc_begin(BIfdIs, 2)
{
  OZ_getCArgDeref(0, fd, fdptr, fdtag);

  if (oz_isNonKinded(fd))
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, fdptr);

  return oz_unify(OZ_getCArg(1),
                  oz_bool(isPosSmallFDInt(fd) ||
                          isGenFDVar(fd, fdtag) ||
                          isGenBoolVar(fd, fdtag)));
}
OZ_C_proc_end

//-----------------------------------------------------------------------------
// reflective stuff

OZ_C_proc_begin(BIfdMin, 2)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT);

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallIntTag(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    int minVal = tagged2GenFDVar(var)->getDom().getMinElem();
    return OZ_unify(oz_int(minVal), OZ_getCArg(1));
  } else if (isGenBoolVar(var,vartag)) {
    return OZ_unify(oz_int(0), OZ_getCArg(1));
  } else if (oz_isNonKinded(var)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdMax, 2)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT);

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallIntTag(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    int maxVal = tagged2GenFDVar(var)->getDom().getMaxElem();
    return OZ_unify(oz_int(maxVal), OZ_getCArg(1));
  } else if (isGenBoolVar(var,vartag)) {
    return OZ_unify(oz_int(1), OZ_getCArg(1));
  } else if (oz_isNonKinded(var)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdMid, 2)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT);

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallIntTag(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    OZ_FiniteDomain &fdomain = tagged2GenFDVar(var)->getDom();
    return OZ_unify(oz_int(fdomain.getMidElem()), OZ_getCArg(1));
  } else if (isGenBoolVar(var,vartag)) {
    return OZ_unify(oz_int(0), OZ_getCArg(1));
  } else if (oz_isNonKinded(var)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdNextSmaller, 3)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_INT);

  OZ_getCArgDeref(1, val, valptr, valtag);

  int value = -1;
  if (isVariableTag(valtag)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, valptr);
  } else if (isSmallIntTag(valtag)) {
    value = smallIntValue(val);
  } else {
    TypeError(1, "");
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallIntTag(vartag)) {
    if (value > smallIntValue(var))
      return OZ_unify(var, OZ_getCArg(2));;
  } else if (isGenFDVar(var,vartag)) {
    int nextSmaller = tagged2GenFDVar(var)->getDom().getNextSmallerElem(value);
    if (nextSmaller != -1)
      return OZ_unify(oz_int(nextSmaller), OZ_getCArg(2));
  } else if (isGenBoolVar(var,vartag)) {
    if (value > 1)
      return OZ_unify(oz_int(1), OZ_getCArg(2));
    else if (value > 0)
      return OZ_unify(oz_int(0), OZ_getCArg(2));
  } else if (oz_isNonKinded(var)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
  } else {
    TypeError(0, "");
  }
  return FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdNextLarger, 3)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_INT);

  OZ_getCArgDeref(1, val, valptr, valtag);

  int value = -1;
  if (oz_isVariable(valtag)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, valptr);
  } else if (isSmallIntTag(valtag)) {
    value = smallIntValue(val);
  } else {
    TypeError(1, "");
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallIntTag(vartag)) {
    if (value < smallIntValue(var))
      return OZ_unify(var, OZ_getCArg(2));;
  } else if (isGenFDVar(var,vartag)) {
    int nextLarger = tagged2GenFDVar(var)->getDom().getNextLargerElem(value);
    if (nextLarger != -1)
      return OZ_unify(oz_int(nextLarger), OZ_getCArg(2));
  } else if (isGenBoolVar(var,vartag)) {
    if (value < 0)
      return OZ_unify(oz_int(0), OZ_getCArg(2));
    else if (value < 1)
      return OZ_unify(oz_int(1), OZ_getCArg(2));
  } else if (oz_isNonKinded(var)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
  } else {
    TypeError(0, "");
  }
  return FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdGetAsList, 2)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_FDDESCR);

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallIntTag(vartag)) {
    LTuple * ltuple = new LTuple(var, AtomNil);
    return OZ_unify(makeTaggedLTuple(ltuple), OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    OZ_FiniteDomain &fdomain = tagged2GenFDVar(var)->getDom();
    return OZ_unify(fdomain.getDescr(), OZ_getCArg(1));
  } else if (isGenBoolVar(var,vartag)) {
    return OZ_unify(makeTaggedLTuple(new LTuple(oz_pairII(0, 1), AtomNil)),
                    OZ_getCArg(1));
  } else if (oz_isNonKinded(var)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdGetCardinality, 2)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT);

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallIntTag(vartag)) {
    return OZ_unify(oz_int(1), OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    OZ_FiniteDomain &fdomain = tagged2GenFDVar(var)->getDom();
    return OZ_unify(oz_int(fdomain.getSize()), OZ_getCArg(1));
  } else if (isGenBoolVar(var,vartag)) {
    return OZ_unify(oz_int(2), OZ_getCArg(1));
  } else if (oz_isNonKinded(var)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end

//-----------------------------------------------------------------------------
// tell finite domain constraint

OZ_C_proc_begin(BIfdTellConstraint, 2)
{
  ExpectedTypes(OZ_EM_FDDESCR "," OZ_EM_FD);

  ExpectOnly pe;
  EXPECT_BLOCK(pe, 0, expectDomDescr,
               "The syntax of a " OZ_EM_FDDESCR " is:\n"
               "   dom_descr   ::= simpl_descr | compl(simpl_descr)\n"
               "   simpl_descr ::= range_descr | [range_descr+]\n"
               "   range_descr ::= integer | integer#integer\n"
               "   integer     ::= {" _OZ_EM_FDINF ",...," _OZ_EM_FDSUP "}");

  OZ_FiniteDomain aux(OZ_getCArg(0));

  return tellBasicConstraint(OZ_getCArg(1), &aux);
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdBoolTellConstraint, 1)
{
  return tellBasicBoolConstraint(OZ_getCArg(0));
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdDeclTellConstraint, 1)
{
  return tellBasicConstraint(OZ_getCArg(0), NULL);
}
OZ_C_proc_end

//-----------------------------------------------------------------------------
// watches

OZ_C_proc_begin(BIfdWatchSize, 3)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_TNAME);

  OZ_getCArgDeref(2, t, tptr, ttag);
  if (!isVariableTag(ttag)) {
    if (oz_isBool(t))
      return PROCEED;
    return FAILED;
  }

  OZ_getCArgDeref(0, v, vptr, vtag);
  int vsize = 0;

// get the current size of the domain
  if(isSmallIntTag(vtag)) {
    vsize = 1;
  } else if (isGenFDVar(v,vtag)) {
    vsize = tagged2GenFDVar(v)->getDom().getSize();
  } else if (isGenBoolVar(v, vtag)) {
    vsize = 2;
  } else if (oz_isNonKinded(v)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } else {
    TypeError(0, "");
  }

// get the value to compare with
  OZ_getCArgDeref(1, vs, vsptr, vstag);
  int size = 0;

  if (isVariableTag(vstag)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, vsptr);
  } else if (isSmallIntTag(vstag)) {
    size = smallIntValue(vs);
  } else {
    TypeError(1, "");
  }

// compute return value
  if (vsize < size) return OZ_unify (OZ_getCArg(2), oz_true());
  if (size < 1) return (OZ_unify (OZ_getCArg(2), oz_false()));

  if (isVariableTag(vtag)){
    //  must return SUSPEND;
    if (isVariableTag(ttag))
      return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args,
                                           vptr, tptr);
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }

  return (OZ_unify (OZ_getCArg(2), oz_false()));
} OZ_C_proc_end

OZ_C_proc_begin(BIfdWatchMin, 3)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_TNAME);

  OZ_getCArgDeref(2, t, tptr, ttag);
  if (!isVariableTag(ttag)) {
    if (oz_isBool(t))
      return PROCEED;
    return FAILED;
  }

  OZ_getCArgDeref(0, v, vptr, vtag);
  int vmin = -1, vmax = -1;

// get the current lower bound of the domain
  if(isSmallIntTag(vtag)) {
    vmin = vmax = smallIntValue(v);
  } else if (isGenFDVar(v,vtag)) {
    vmin = tagged2GenFDVar(v)->getDom().getMinElem();
    vmax = tagged2GenFDVar(v)->getDom().getMaxElem();
  } else if (isGenBoolVar(v, vtag)) {
    vmin = 0;
    vmax = 1;
  } else if (oz_isNonKinded(v)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } else {
    TypeError(0, "");
  }

// get the value to compare with
  OZ_getCArgDeref(1, vm, vmptr, vmtag);
  int min = -1;

  if (isVariableTag(vmtag)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, vmptr);
  } else if (isSmallIntTag(vmtag)) {
    min = smallIntValue(vm);
  } else {
    TypeError(1, "");
  }

  if (min < 0) return (OZ_unify (OZ_getCArg(2), oz_false()));
  if (vmin > min) return OZ_unify (OZ_getCArg(2), oz_true());

  if (isVariableTag(vtag) && min < vmax){
    //  must return SUSPEND;
    if (isVariableTag(ttag))
      return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args,
                                           vptr, tptr);
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }

  return (OZ_unify (OZ_getCArg(2), oz_false()));
} OZ_C_proc_end

OZ_C_proc_begin(BIfdWatchMax, 3)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_TNAME);

  OZ_getCArgDeref(2, t, tptr, ttag);
  if (!isVariableTag(ttag)) {
    if (oz_isBool(t))
      return PROCEED;
    return FAILED;
  }

  OZ_getCArgDeref(0, v, vptr, vtag);
  int vmin = -1, vmax = -1;

// get the current lower bound of the domain
  if(isSmallIntTag(vtag)) {
    vmin = vmax = smallIntValue(v);
  } else if (isGenFDVar(v,vtag)) {
    vmin = tagged2GenFDVar(v)->getDom().getMinElem();
    vmax = tagged2GenFDVar(v)->getDom().getMaxElem();
  } else if (isGenBoolVar(v, vtag)) {
    vmin = 0;
    vmax = 1;
  } else if (oz_isNonKinded(v)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } else {
    TypeError(0, "");
  }

// get the value to compare with
  OZ_getCArgDeref(1, vm, vmptr, vmtag);
  int max = -1;

  if (isVariableTag(vmtag)) {
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, vmptr);
  } else if (isSmallIntTag(vmtag)) {
    max = smallIntValue(vm);
  } else {
    TypeError(1, "");
  }

  if (vmax < max) return OZ_unify (OZ_getCArg(2), oz_true());
  if (max < 0) return (OZ_unify (OZ_getCArg(2), oz_false()));

  if (isVariableTag(vtag) && vmin < max){
    //  must return SUSPEND;
    if (isVariableTag(ttag))
      return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args,
                                           vptr, tptr);
    return BI_FD_suspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }

  return (OZ_unify (OZ_getCArg(2), oz_false()));
} OZ_C_proc_end
