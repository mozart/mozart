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


OZ_C_proc_begin(BIgetFDLimits,2)
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
                  (OZ_isPosSmallInt(fd) ||
                   isGenFDVar(fd, fdtag) ||
                   isGenBoolVar(fd, fdtag)) ? NameTrue : NameFalse);
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdMin, 2)
{
  ExpectedTypes("FiniteDomain,SmallInt");

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    int minVal = tagged2GenFDVar(var)->getDom().minElem();
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


OZ_C_proc_begin(BIfdMax,2)
{
  ExpectedTypes("FiniteDomain,SmallInt");

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    int maxVal = tagged2GenFDVar(var)->getDom().maxElem();
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

OZ_C_proc_begin(BIfdMid,2)
{
  ExpectedTypes("FiniteDomain,SmallInt");

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenFDVar(var,vartag)) {
    OZ_FiniteDomain &fdomain = tagged2GenFDVar(var)->getDom();
    return OZ_unify(OZ_int(fdomain.next((fdomain.minElem() +
                                         fdomain.maxElem()) / 2)),
                    OZ_getCArg(1));
  } else if (isGenBoolVar(var,vartag)) {
    return OZ_unify(OZ_int(0), OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
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
    OZ_FiniteDomain &fdomain = tagged2GenFDVar(var)->getDom();
    return OZ_unify(fdomain.getAsList(), OZ_getCArg(1));
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


OZ_C_proc_begin(BIfdGetCardinality,2)
{
  ExpectedTypes("FiniteDomain,SmallInt");

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



OZ_C_proc_begin(BIfdPutLe, 2)
{
  Assert (!(am.currentThread->isPropagator () || am.currentThread->isNewPropagator ()));

  ExpectedTypes("FiniteDomain,SmallInt");

  OZ_getCArgDeref(1, n, nptr, ntag);

  if (isAnyVar(ntag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, nptr);
  } else if (! isSmallInt(ntag)) {
    TypeError(1, "");
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if (! (isGenFDVar(var,vartag) || isGenBoolVar(var,vartag) ||
         OZ_isPosSmallInt(var))) {
    if (isNotCVar(vartag)) {
      return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
    } else {
      TypeError(0, "");
    }
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) {
    error("Should never happen.");
    return FAILED;
  }

  FailOnEmpty(*x <= OZ_intToC(n));

  return x.releaseNonRes();
}
OZ_C_proc_end // BIfdPutLe


OZ_C_proc_begin(BIfdPutGe, 2)
{
  Assert (!(am.currentThread->isPropagator () || am.currentThread->isNewPropagator ()));

  ExpectedTypes("FiniteDomain,SmallInt");

  OZ_getCArgDeref(1, n, nptr, ntag);

  if (isAnyVar(ntag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, nptr);
  } else if (! isSmallInt(ntag)) {
    TypeError(1, "");
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if (! (isGenFDVar(var,vartag) || isGenBoolVar(var,vartag) ||
         OZ_isPosSmallInt(var))) {
    if (isNotCVar(vartag)) {
      return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
    } else {
      TypeError(0, "");
    }
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) {
    error("Should never happen.");
    return FAILED;
  }

  FailOnEmpty(*x >= OZ_intToC(n));

  return x.releaseNonRes();
}
OZ_C_proc_end // BIfdPutGe


OZ_C_proc_begin(BIfdPutList, 3) // TMUELLER; 3rd arg is redundant soon
{
  Assert (!(am.currentThread->isPropagator () || am.currentThread->isNewPropagator ()));

  ExpectedTypes("FiniteDomain,List of SmallInts or Tuples,SmallInt");

  OZ_getCArgDeref(2, s, sptr, stag); // sign

  if (isAnyVar(stag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, sptr);
  } else if (! isSmallInt(stag)) {
    TypeError(2, "");
  }

  OZ_PropagatorExpect pe;
  OZ_expect_t r = pe.expectDomainDescription(OZ_getCArg(1));
  if (pe.isFailing(r)) {
    TypeError(1, "");
  } else if (pe.isSuspending(r)) {
    return pe.suspend(OZ_makeSuspendedThread(OZ_self,OZ_args,OZ_arity));
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) return FAILED;

  OZ_FiniteDomain aux(OZ_getCArg(1));

  if (OZ_intToC(s) != 0) aux = ~aux;

  FailOnEmpty(*x &= aux);

  return x.releaseNonRes();
}
OZ_C_proc_end // BIfdPutList


OZ_C_proc_begin(BIfdPutInterval, 3)
{
  Assert (!(am.currentThread->isPropagator () || am.currentThread->isNewPropagator ()));

  ExpectedTypes("FiniteDomain,SmallInt,SmallInt");

  OZ_getCArgDeref(1, l, lptr, ltag); // lower bound

  if (isAnyVar(ltag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, lptr);
  } else if (! isSmallInt(ltag)) {
    TypeError(1, "");
  }

  OZ_getCArgDeref(2, u, uptr, utag); // upper bound

  if (isAnyVar(utag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, uptr);
  } else if (! isSmallInt(utag)) {
    TypeError(2, "");
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if (! (isGenFDVar(var, vartag) || isGenBoolVar(var, vartag) ||
         isNotCVar(vartag) || isSmallInt(vartag))) {
    TypeError(0, "");
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) return FAILED;

  OZ_FiniteDomain aux;

  FailOnEmpty(aux.init(OZ_intToC(l), OZ_intToC(u)));
  FailOnEmpty(*x &= aux);

  return x.releaseNonRes();
}
OZ_C_proc_end // BIfdPutInterval


OZ_C_proc_begin(BIfdPutNot, 2)
{
  Assert (!(am.currentThread->isPropagator () || am.currentThread->isNewPropagator ()));

  ExpectedTypes("FiniteDomain,SmallInt");

  OZ_getCArgDeref(1, n, nptr, ntag);

  if (isAnyVar(ntag)) {
    return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, nptr);
  } else if (! isSmallInt(ntag)) {
    TypeError(1, "");
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if (! (isGenFDVar(var, vartag) || isGenBoolVar(var, vartag) ||
         OZ_isPosSmallInt(var))) {
    if (isNotCVar(vartag)) {
      return BIfdHeadManager::suspendOnVar(OZ_self, OZ_arity, OZ_args, varptr);
    } else {
      TypeError(0, "");
    }
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) {
    error("Should never happen.");
    return FAILED;
  }

  FailOnEmpty(*x -= OZ_intToC(n));

  return x.releaseNonRes();
}
OZ_C_proc_end // BIfdPutNot
