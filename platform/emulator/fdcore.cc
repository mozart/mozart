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
//                  Finite Domains Core Built-ins
// ---------------------------------------------------------------------


OZ_C_proc_begin(BIisFdVar, 1)
{
  return isGenFDVar(deref(OZ_getCArg(0))) ? PROCEED : FAILED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIgetFDLimits,2)
{
  return (OZ_unify(newSmallInt(0), OZ_getCArg(0)) &&
    OZ_unify(newSmallInt(fd_iv_max_elem), OZ_getCArg(1))) ? PROCEED : FAILED;
}
OZ_C_proc_end

State BIfdIsInline(TaggedRef fd) {
  DEREF(fd, fdptr, fdtag);

  if (isNotCVar(fdtag)) return SUSPEND;

  return (isPosSmallInt(fd) || isGenFDVar(fd, fdtag)) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIfdIs, BIfdIsInline);

OZ_C_proc_begin(BIfdMin, 2)
{
  ExpectedTypes("FiniteDomain,SmallInt");

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    int minVal = tagged2GenFDVar(var)->getDom().minElem();
    return OZ_unify(newSmallInt(minVal), OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    return addNonResSuspForCon(var, varptr, vartag,
                               createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdMax,2)
{
  ExpectedTypes("FiniteDomain,SmallInt");

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    int maxVal = tagged2GenFDVar(var)->getDom().maxElem();
    return OZ_unify(newSmallInt(maxVal), OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    return addNonResSuspForCon(var, varptr, vartag,
                               createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdGetAsList, 2)
{
  ExpectedTypes("FiniteDomain,List");

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    LTuple * ltuple = new LTuple(var, AtomNil);
    return OZ_unify(makeTaggedLTuple(ltuple), OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    FiniteDomain &fdomain = tagged2GenFDVar(var)->getDom();
    return OZ_unify(fdomain.getAsList(), OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    return addNonResSuspForCon(var, varptr, vartag,
                               createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdGetCardinality,2)
{
  ExpectedTypes("FiniteDomain,SmallInt");

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    return OZ_unify(newSmallInt(1), OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    FiniteDomain &fdomain = tagged2GenFDVar(var)->getDom();
    return OZ_unify(newSmallInt(fdomain.getSize()), OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    return addNonResSuspForCon(var, varptr, vartag,
                               createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end



OZ_C_proc_begin(BIfdNextTo, 3)
{
  ExpectedTypes("FiniteDomain,SmallInt,SmallInt or Tuple");

  OZ_getCArgDeref(1, n, nptr, ntag);

  if (isAnyVar(ntag)) {
    return addNonResSuspForDet(n, nptr, ntag,
                               createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(ntag)) {
    TypeError(1, "");
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if (isPosSmallInt(var)) {
    return OZ_unify(OZ_getCArg(2), var);
  } else if (isGenFDVar(var,vartag)) {
    int next_val, n_val = smallIntValue(n);
    return (tagged2GenFDVar(var)->getDom().next(n_val, next_val))
      ? OZ_unify(OZ_getCArg(2), mkTuple(next_val, 2 * n_val - next_val))
      : OZ_unify(OZ_getCArg(2), newSmallInt(next_val));
  } else if (isNotCVar(vartag)) {
    return addNonResSuspForCon(var, varptr, vartag,
                               createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdPutLe, 2)
{
  Assert(!FDcurrentTaskSusp);

  ExpectedTypes("FiniteDomain,SmallInt");

  OZ_getCArgDeref(1, n, nptr, ntag);

  if (isAnyVar(ntag)) {
    return addNonResSuspForDet(n, nptr, ntag,
                               createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(ntag)) {
    TypeError(1, "");
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if (! (isGenFDVar(var,vartag) || isSmallInt(vartag))) {
    if (isNotCVar(vartag)) {
      return addNonResSuspForCon(var, varptr, vartag,
                                 createNonResSusp(OZ_self, OZ_args, OZ_arity));
    } else {
      TypeError(0, "");
    }
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) {
    error("Should never happen.");
    return FAILED;
  }

  FailOnEmpty(*x <= smallIntValue(n));

  return x.releaseNonRes();
}
OZ_C_proc_end // BIfdPutLe


OZ_C_proc_begin(BIfdPutGe, 2)
{
  Assert(!FDcurrentTaskSusp);

  ExpectedTypes("FiniteDomain,SmallInt");

  OZ_getCArgDeref(1, n, nptr, ntag);

  if (isAnyVar(ntag)) {
    return addNonResSuspForDet(n, nptr, ntag,
                               createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(ntag)) {
    TypeError(1, "");
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if (! (isGenFDVar(var,vartag) || isSmallInt(vartag))) {
    if (isNotCVar(vartag)) {
      return addNonResSuspForCon(var, varptr, vartag,
                                 createNonResSusp(OZ_self, OZ_args, OZ_arity));
    } else {
      TypeError(0, "");
    }
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) {
    error("Should never happen.");
    return FAILED;
  }

  FailOnEmpty(*x >= smallIntValue(n));

  return x.releaseNonRes();
}
OZ_C_proc_end // BIfdPutGe


OZ_C_proc_begin(BIfdPutList, 3)
{
  Assert(!FDcurrentTaskSusp);

  ExpectedTypes("FiniteDomain,List of SmallInts or Tuples,SmallInt");

  OZ_getCArgDeref(2, s, sptr, stag); // sign

  if (isAnyVar(stag)) {
    return addNonResSuspForDet(s, sptr, stag,
                               createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(stag)) {
    TypeError(2, "");
  }

  OZ_getCArgDeref(1, list, listptr, listtag);

  if (isNotCVar(listtag)) {
    return addNonResSuspForDet(list, listptr, listtag,
                               createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (isNil(list)) { // empty list represents empty domain, ie failure
    return FAILED;
  } else if (! isLTuple(listtag)) {
    TypeError(1, "");
  }

  int * left_arr = static_int_a, * right_arr = static_int_b;
  int min_arr = fd_iv_max_elem, max_arr = 0;

  for (int len_arr = 0; isLTuple(list) && len_arr < MAXFDBIARGS;) {
    TaggedRef val = tagged2LTuple(list)->getHead();

    DEREF(val, valptr, valtag);

    if (isSmallInt(valtag)) {
      int v = smallIntValue(val);
      if (0 <= v && v <= fd_iv_max_elem) {
        left_arr[len_arr] = right_arr[len_arr] = v;
        min_arr = min(min_arr, left_arr[len_arr]);
        max_arr = max(max_arr, right_arr[len_arr]);
        len_arr += 1;
      }
    } else if (isSTuple(valtag)) {
      STuple * t = tagged2STuple(val);

      if (t->getSize() != 2) {
        TypeError(1, "Expected 2-tuple as tuple.");
      }
      TaggedRef t_l = (*t)[0];
      DEREF(t_l, t_lptr, t_ltag);
      if (isSmallInt(t_ltag)) {
        left_arr[len_arr] = smallIntValue(t_l);
      } else if (isAnyVar(t_ltag)) {
        return addNonResSuspForDet(t_l, t_lptr, t_ltag,
                                   createNonResSusp(OZ_self,OZ_args,OZ_arity));
      } else {
        TypeError(1, "Expected SmallInt in 2-tuple.");
      }

      TaggedRef t_r = (*t)[1];
      DEREF(t_r, t_rptr, t_rtag);
      if (isSmallInt(t_rtag)) {
        right_arr[len_arr] = smallIntValue(t_r);
      } else if (isAnyVar(t_rtag)) {
        return addNonResSuspForDet(t_r, t_rptr, t_rtag,
                                   createNonResSusp(OZ_self,OZ_args,OZ_arity));
      } else {
        TypeError(1, "Expected SmallInt in 2-tuple.");
      }

      if (left_arr[len_arr] <= right_arr[len_arr] &&
          right_arr[len_arr] <= fd_iv_max_elem) {
        if (left_arr[len_arr] < 0) left_arr[len_arr] = 0;
        min_arr = min(min_arr, left_arr[len_arr]);
        max_arr = max(max_arr, right_arr[len_arr]);
        len_arr += 1;
      }
    } else if (isNotCVar(valtag)) {
      return addNonResSuspForDet(val, valptr, valtag,
                                 createNonResSusp(OZ_self, OZ_args, OZ_arity));
    } else {
      TypeError(1, "Expected list of SmallInts or SmallInt 2-tuple.");
    }

    list = tagged2LTuple(list)->getTail();
    while(isRef(list)) list = *tagged2Ref(list);
  }

  if (len_arr >= MAXFDBIARGS)
    warning("BIfdPutList: Probably elements of description are ignored");

  OZ_getCArgDeref(0, var, varptr, vartag);

  if (! (isGenFDVar(var,vartag) || isNotCVar(vartag) || isSmallInt(vartag))) {
    TypeError(0, "");
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) return FAILED;

  LocalFD aux; aux.initList(len_arr, left_arr, right_arr, min_arr, max_arr);

  if (smallIntValue(s) != 0) aux = ~aux;

  FailOnEmpty(*x &= aux);

  return x.releaseNonRes();
}
OZ_C_proc_end // BIfdPutList


OZ_C_proc_begin(BIfdPutInterval, 3)
{
  Assert(!FDcurrentTaskSusp);

  ExpectedTypes("FiniteDomain,SmallInt,SmallInt");

  OZ_getCArgDeref(1, l, lptr, ltag); // lower bound

  if (isAnyVar(ltag)) {
    return addNonResSuspForDet(l, lptr, ltag,
                               createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(ltag)) {
    TypeError(1, "");
  }

  OZ_getCArgDeref(1, u, uptr, utag); // upper bound

  if (isAnyVar(utag)) {
    return addNonResSuspForDet(u, uptr, utag,
                               createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(utag)) {
    TypeError(2, "");
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if (! (isGenFDVar(var,vartag) || isNotCVar(vartag) || isSmallInt(vartag))) {
    TypeError(0, "");
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) return FAILED;

  LocalFD aux;

  FailOnEmpty(aux.init(smallIntValue(l), smallIntValue(u)));
  FailOnEmpty(*x &= aux);

  return x.releaseNonRes();
}
OZ_C_proc_end // BIfdPutInterval


OZ_C_proc_begin(BIfdPutNot, 2)
{
  Assert(!FDcurrentTaskSusp);

  ExpectedTypes("FiniteDomain,SmallInt");

  OZ_getCArgDeref(1, n, nptr, ntag);

  if (isAnyVar(ntag)) {
    return addNonResSuspForDet(n, nptr, ntag,
                               createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(ntag)) {
    TypeError(1, "");
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if (! (isGenFDVar(var, vartag) || isSmallInt(vartag))) {
    if (isNotCVar(vartag)) {
      return addNonResSuspForCon(var, varptr, vartag,
                                 createNonResSusp(OZ_self, OZ_args, OZ_arity));
    } else {
      TypeError(0, "");
    }
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) {
    error("Should never happen.");
    return FAILED;
  }

  FailOnEmpty(*x -= smallIntValue(n));

  return x.releaseNonRes();
}
OZ_C_proc_end // BIfdPutNot
