/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "cpi.hh"
#include "fdbuilti.hh"

// ---------------------------------------------------------------------
// misc stuff

OZ_C_proc_begin(BIisFdVar, 1)
{
  return isGenFDVar(deref(OZ_getCArg(0))) || isGenBoolVar(deref(OZ_getCArg(0))) ? PROCEED : FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIisFdVarB, 2)
{
  return (OZ_unify
          (OZ_getCArg (1),
           (isGenFDVar(deref(OZ_getCArg(0))) ||
            isGenBoolVar(deref(OZ_getCArg(0)))) ? NameTrue : NameFalse));
}
OZ_C_proc_end

OZ_C_proc_begin(BIgetFDLimits, 2)
{
  return (OZ_unify(OZ_int(0), OZ_getCArg(0)) &&
    OZ_unify(OZ_int(fd_sup), OZ_getCArg(1))) ? PROCEED : FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdIs, 2)
{
  OZ_getCArgDeref(0, fd, fdptr, fdtag);

  if (isNotCVar(fdtag))
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, fdptr);

  return OZ_unify(OZ_getCArg(1),
                  (isPosSmallFDInt(fd) ||
                   isGenFDVar(fd, fdtag) ||
                   isGenBoolVar(fd, fdtag)) ? NameTrue : NameFalse);
}
OZ_C_proc_end

//-----------------------------------------------------------------------------
// reflective stuff

OZ_C_proc_begin(BIfdMin, 2)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT);

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    int minVal = tagged2GenFDVar(var)->getDom().getMinElem();
    return OZ_unify(OZ_int(minVal), OZ_getCArg(1));
  } else if (isGenBoolVar(var,vartag)) {
    return OZ_unify(OZ_int(0), OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdMax, 2)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT);

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    int maxVal = tagged2GenFDVar(var)->getDom().getMaxElem();
    return OZ_unify(OZ_int(maxVal), OZ_getCArg(1));
  } else if (isGenBoolVar(var,vartag)) {
    return OZ_unify(OZ_int(1), OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdMid, 2)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT);

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    OZ_FiniteDomain &fdomain = tagged2GenFDVar(var)->getDom();
    return OZ_unify(OZ_int(fdomain.getMidElem()), OZ_getCArg(1));
  } else if (isGenBoolVar(var,vartag)) {
    return OZ_unify(OZ_int(0), OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
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
  if (isAnyVar(valtag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, valptr);
  } else if (isSmallInt(valtag)) {
    value = OZ_intToC(val);
  } else {
    TypeError(1, "");
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    if (value > OZ_intToC(var))
      return OZ_unify(var, OZ_getCArg(2));;
  } else if (isGenFDVar(var,vartag)) {
    int nextSmaller = tagged2GenFDVar(var)->getDom().getNextSmallerElem(value);
    if (nextSmaller != -1)
      return OZ_unify(OZ_int(nextSmaller), OZ_getCArg(2));
  } else if (isGenBoolVar(var,vartag)) {
    if (value > 1)
      return OZ_unify(OZ_int(1), OZ_getCArg(2));
    else if (value > 0)
      return OZ_unify(OZ_int(0), OZ_getCArg(2));
  } else if (isNotCVar(vartag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
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
  if (isAnyVar(valtag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, valptr);
  } else if (isSmallInt(valtag)) {
    value = OZ_intToC(val);
  } else {
    TypeError(1, "");
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    if (value < OZ_intToC(var))
      return OZ_unify(var, OZ_getCArg(2));;
  } else if (isGenFDVar(var,vartag)) {
    int nextLarger = tagged2GenFDVar(var)->getDom().getNextLargerElem(value);
    if (nextLarger != -1)
      return OZ_unify(OZ_int(nextLarger), OZ_getCArg(2));
  } else if (isGenBoolVar(var,vartag)) {
    if (value < 0)
      return OZ_unify(OZ_int(0), OZ_getCArg(2));
    else if (value < 1)
      return OZ_unify(OZ_int(1), OZ_getCArg(2));
  } else if (isNotCVar(vartag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
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

  if(isSmallInt(vartag)) {
    LTuple * ltuple = new LTuple(var, AtomNil);
    return OZ_unify(makeTaggedLTuple(ltuple), OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    OZ_FiniteDomain &fdomain = tagged2GenFDVar(var)->getDom();
    return OZ_unify(fdomain.getDescr(), OZ_getCArg(1));
  } else if (isGenBoolVar(var,vartag)) {
    return OZ_unify(makeTaggedLTuple(new LTuple(mkTuple(0, 1), AtomNil)),
                    OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdGetCardinality, 2)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT);

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    return OZ_unify(OZ_int(1), OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    OZ_FiniteDomain &fdomain = tagged2GenFDVar(var)->getDom();
    return OZ_unify(OZ_int(fdomain.getSize()), OZ_getCArg(1));
  } else if (isGenBoolVar(var,vartag)) {
    return OZ_unify(OZ_int(2), OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end

//-----------------------------------------------------------------------------
// tell finite domain constraint

OZ_C_proc_begin(BIfdTellConstraint, 2)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_FDDESCR);

  ExpectOnly pe;
  OZ_expect_t r = pe.expectDomDescr(OZ_getCArg(1));
  if (pe.isFailing(r)) {
    TypeError(1, "The syntax of a " OZ_EM_FDDESCR " is:\n"
              "dom_descr   ::= simpl_descr | compl(simpl_descr)\n"
              "simpl_descr ::= range_descr | [range_descr+]\n"
              "range_descr ::= integer | integer#integer\n"
              "integer     ::= {" _OZ_EM_FDINF ",...," _OZ_EM_FDSUP "}");
  } else if (pe.isSuspending(r)) {
    for (OZ_Term * v = pe.getSuspVar(); v != NULL; v = pe.getSuspVar())
      am.addSuspendVarList(v);
    return SUSPEND;
  }

  OZ_FiniteDomain aux(OZ_getCArg(1));

  return tellBasicConstraint(OZ_getCArg(0), &aux);
}
OZ_C_proc_end

//-----------------------------------------------------------------------------
// watches

OZ_C_proc_begin(BIfdWatchSize, 3)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_TNAME);

  OZ_getCArgDeref(0, v, vptr, vtag);
  int vsize = 0;

// get the current size of the domain
  if(isSmallInt(vtag)) {
    vsize = 1;
  } else if (isGenFDVar(v,vtag)) {
    vsize = tagged2GenFDVar(v)->getDom().getSize();
  } else if (isGenBoolVar(v, vtag)) {
    vsize = 2;
  } else if (isNotCVar(vtag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } else {
    TypeError(0, "");
  }

// get the value to compare with
  OZ_getCArgDeref(1, vs, vsptr, vstag);
  int size = 0;

  if (isAnyVar(vstag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, vsptr);
  } else if (isSmallInt(vstag)) {
    size = OZ_intToC(vs);
  } else {
    TypeError(1, "");
  }

// compute return value
  if (vsize < size) return OZ_unify (OZ_getCArg(2), NameTrue);
  if (size < 1) return (OZ_unify (OZ_getCArg(2), NameFalse));

  if (isAnyVar(vtag)){
    //  must return SUSPEND;
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }

  return (OZ_unify (OZ_getCArg(2), NameFalse));
} OZ_C_proc_end

OZ_C_proc_begin(BIfdWatchMin, 3)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_TNAME);

  OZ_getCArgDeref(0, v, vptr, vtag);
  int vmin = -1, vmax = -1;

// get the current lower bound of the domain
  if(isSmallInt(vtag)) {
    vmin = vmax = OZ_intToC(v);
  } else if (isGenFDVar(v,vtag)) {
    vmin = tagged2GenFDVar(v)->getDom().getMinElem();
    vmax = tagged2GenFDVar(v)->getDom().getMaxElem();
  } else if (isGenBoolVar(v, vtag)) {
    vmin = 0;
    vmax = 1;
  } else if (isNotCVar(vtag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } else {
    TypeError(0, "");
  }

// get the value to compare with
  OZ_getCArgDeref(1, vm, vmptr, vmtag);
  int min = -1;

  if (isAnyVar(vmtag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, vmptr);
  } else if (isSmallInt(vmtag)) {
    min = OZ_intToC(vm);
  } else {
    TypeError(1, "");
  }

  if (min < 0) return (OZ_unify (OZ_getCArg(2), NameFalse));
  if (vmin > min) return OZ_unify (OZ_getCArg(2), NameTrue);

  if (isAnyVar(vtag) && min < vmax){
    //  must return SUSPEND;
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }

  return (OZ_unify (OZ_getCArg(2), NameFalse));
} OZ_C_proc_end

OZ_C_proc_begin(BIfdWatchMax, 3)
{
  ExpectedTypes(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_TNAME);

  OZ_getCArgDeref(0, v, vptr, vtag);
  int vmin = -1, vmax = -1;

// get the current lower bound of the domain
  if(isSmallInt(vtag)) {
    vmin = vmax = OZ_intToC(v);
  } else if (isGenFDVar(v,vtag)) {
    vmin = tagged2GenFDVar(v)->getDom().getMinElem();
    vmax = tagged2GenFDVar(v)->getDom().getMaxElem();
  } else if (isGenBoolVar(v, vtag)) {
    vmin = 0;
    vmax = 1;
  } else if (isNotCVar(vtag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } else {
    TypeError(0, "");
  }

// get the value to compare with
  OZ_getCArgDeref(1, vm, vmptr, vmtag);
  int max = -1;

  if (isAnyVar(vmtag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, vmptr);
  } else if (isSmallInt(vmtag)) {
    max = OZ_intToC(vm);
  } else {
    TypeError(1, "");
  }

  if (vmax < max) return OZ_unify (OZ_getCArg(2), NameTrue);
  if (max < 0) return (OZ_unify (OZ_getCArg(2), NameFalse));

  if (isAnyVar(vtag) && vmin < max){
    //  must return SUSPEND;
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }

  return (OZ_unify (OZ_getCArg(2), NameFalse));
} OZ_C_proc_end

// end of file
//-----------------------------------------------------------------------------
