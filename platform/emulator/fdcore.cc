/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "fdbuiltin.hh"


// ---------------------------------------------------------------------
//                  Finite Domains Core Built-ins
// ---------------------------------------------------------------------


OZ_C_proc_begin(BIisFdVar,1)
{ 
  return isGenFDVar(deref(OZ_getCArg(0))) ? PROCEED : FAILED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIgetFDLimits,2)
{ 
  return (OZ_unify(newSmallInt(fdMinBA), OZ_getCArg(0)) &&
    OZ_unify(newSmallInt(fdMaxBA), OZ_getCArg(1))) ? PROCEED : FAILED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdMin, 2)
{
  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));   
#ifdef CVAR_ONLY_FDVAR
  } else if (isCVar(vartag)) {
#else
  } else if (isGenFDVar(var)) {
#endif
    int minVal = tagged2GenFDVar(var)->getDom().minElem();
    return OZ_unify(newSmallInt(minVal), OZ_getCArg(1));   
  } else if (isNotCVar(vartag)) {
    return addNonResSuspForCon(var, varptr, vartag,
			       createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else {
    warning("BIfdMin: Expected fdish value, got 0x%x.", vartag);
    return FAILED;
  }
}
OZ_C_proc_end     


OZ_C_proc_begin(BIfdMax,2)
{
  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));   
#ifdef CVAR_ONLY_FDVAR
  } else if (isCVar(vartag)) {
#else
  } else if (isGenFDVar(var)) {
#endif
    int maxVal = tagged2GenFDVar(var)->getDom().maxElem();
    return OZ_unify(newSmallInt(maxVal), OZ_getCArg(1));   
  } else if (isNotCVar(vartag)) {
    return addNonResSuspForCon(var, varptr, vartag,
			       createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else {
    warning("BIfdMax: Expected fdish value, got 0x%x.", vartag);
    return FAILED;
  }
}
OZ_C_proc_end     


OZ_C_proc_begin(BIfdGetAsList, 2)
{
  OZ_getCArgDeref(0, var, varptr, vartag);
  
  if(isSmallInt(vartag)) {
    LTuple * ltuple = new LTuple(var, AtomNil);
    return OZ_unify(makeTaggedLTuple(ltuple), OZ_getCArg(1));
#ifdef CVAR_ONLY_FDVAR
  } else if (isCVar(vartag)) {
#else
  } else if (isGenFDVar(var)) {
#endif
    FiniteDomain &fdomain = tagged2GenFDVar(var)->getDom();
    return OZ_unify(fdomain.getAsList(), OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    return addNonResSuspForCon(var, varptr, vartag,
			       createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else {
    warning("BIfdGetAsList: Expected fdish value, got 0x%x.", vartag);
    return FAILED;
  }
}
OZ_C_proc_end     


OZ_C_proc_begin(BIfdGetCardinality,2)
{
  OZ_getCArgDeref(0, var, varptr, vartag);

  if(isSmallInt(vartag)) {
    return OZ_unify(newSmallInt(1), OZ_getCArg(1));
#ifdef CVAR_ONLY_FDVAR
  } else if (isCVar(vartag)) {
#else
  } else if (isGenFDVar(var)) {
#endif
    FiniteDomain &fdomain = tagged2GenFDVar(var)->getDom();
    return OZ_unify(newSmallInt(fdomain.getSize()), OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    return addNonResSuspForCon(var, varptr, vartag,
			       createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else { 
    warning("BIfdGetCardinality: Expected fdish value, got 0x%x.", vartag);
    return FAILED;
  }
}
OZ_C_proc_end



OZ_C_proc_begin(BIfdNextTo, 3)
{
  OZ_getCArgDeref(1, n, nptr, ntag);

  if (isAnyVar(ntag)) {
    return addNonResSuspForDet(n, nptr, ntag,
			       createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(ntag)){
    warning("BIfdNextTo: Expected small integer, got 0x%x.", ntag);
    return FAILED;
  }

  OZ_getCArgDeref(0, var, varptr, vartag);

  if (isPosSmallInt(var)) {
    return OZ_unify(OZ_getCArg(2), var);
#ifdef CVAR_ONLY_FDVAR
  } else if (isCVar(vartag)) {
#else
  } else if (isGenFDVar(var)) {
#endif
    int next_val, n_val = smallIntValue(n);
    return (tagged2GenFDVar(var)->getDom().next(n_val, next_val))
      ? OZ_unify(OZ_getCArg(2), makeRangeTuple(next_val, 2 * n_val - next_val))
      : OZ_unify(OZ_getCArg(2), newSmallInt(next_val));
  } else if (isNotCVar(vartag)) {
    return addNonResSuspForCon(var, varptr, vartag,
			       createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else {
    warning("BIfdNextTo: Expected fdish value, got got 0x%x.", vartag);
    return FAILED;
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdPutLe, 2)
{
  OZ_getCArgDeref(1, n, nptr, ntag);

  if (isAnyVar(ntag)) {
    return addNonResSuspForDet(n, nptr, ntag,
			       createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(ntag)) {
    warning("BIfdPutLe: Expected small integer, got 0x%x.", ntag);
    return FAILED;
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) return FAILED;
  
  if ((*x <= smallIntValue(n)) == 0) return FAILED;
  
  return x.releaseNonRes();
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdPutGe, 2)
{
  OZ_getCArgDeref(1, n, nptr, ntag);

  if (isAnyVar(ntag)) {
    return addNonResSuspForDet(n, nptr, ntag,
			       createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(ntag)) {
    warning("BIfdPutLe: Expected small integer, got 0x%x.", ntag);
    return FAILED;
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) return FAILED;
  
  if ((*x >= smallIntValue(n)) == 0) return FAILED;
  
  return x.releaseNonRes();
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdPutList, 3)
{
  OZ_getCArgDeref(2, s, sptr, stag); // sign

  if (isAnyVar(stag)) {
    return addNonResSuspForDet(s, sptr, stag,
			       createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(stag)) {
    warning("BIfdPutLe: Expected small integer, got 0x%x.", stag);
    return FAILED;
  }

  OZ_getCArgDeref(1, list, listptr, listtag);
  
  if (isNotCVar(listtag)) {
    return addNonResSuspForCon(list, listptr, listtag,
			       createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isLTuple(listtag)) {
    warning("BIfdPutLe: Expected list tuple, got 0x%x.", listtag);
    return FAILED;
  }

  
  int list_array[fdMaxBA - fdMinBA + 1];
  BitArray bitArray;
  bitArray.empty();
  
  for (int list_len = 0; isLTuple(list); list_len += 1) {
    TaggedRef ival = tagged2LTuple(list)->getHead();
    
    while(isRef(ival)) ival = * TaggedRefPtr(ival);
    
    if(! isSmallInt(ival)){
      warning("BIfdPutList: Expected list of small ints, got 0x%x.",
	      tagTypeOf(ival));
      return FAILED;
    } else {
      int v = smallIntValue(ival);
      if (! bitArray.contains(v)) {
	list_array[list_len] = v;
	bitArray.setBit(v);
      }
    }

    list = tagged2LTuple(list)->getTail();
    while(isRef(list)) list = *TaggedRefPtr(list);
  } 
  
  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) return FAILED;

  LocalFD aux; aux.init(list_len, list_array);

  if (smallIntValue(s) != 0) aux = ~aux;
  
  if ((*x &= aux) == 0) return FAILED;
  
  return x.releaseNonRes();
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdPutNot, 2)
{
  OZ_getCArgDeref(1, n, nptr, ntag);

  if (isAnyVar(ntag)) {
    return addNonResSuspForDet(n, nptr, ntag,
			       createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(ntag)) {
    warning("BIfdPutLe: Expected small integer, got 0x%x.", ntag);
    return FAILED;
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) return FAILED;

  if ((*x -= smallIntValue(n)) == 0) return FAILED;
  
  return x.releaseNonRes();
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdPutFromTo, 3){
  OZ_getCArgDeref(1, f, fptr, ftag); // from

  if (isAnyVar(ftag)) {
    return addNonResSuspForDet(f, fptr, ftag,
			       createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(ftag)) {
    warning("BIfdPutLe: Expected small integer, got 0x%x.", ftag);
    return FAILED;
  }

  OZ_getCArgDeref(2, t, tptr, ttag); // to

  if (isAnyVar(ttag)) {
    return addNonResSuspForDet(t, tptr, ttag,
			       createNonResSusp(OZ_self, OZ_args, OZ_arity));
  } else if (! isSmallInt(ttag)) {
    warning("BIfdPutLe: Expected small integer, got 0x%x.", ttag);
    return FAILED;
  }

  BIfdBodyManager x;

  if (! x.introduce(OZ_getCArg(0))) return FAILED;
  
  if ((*x >= smallIntValue(f)) == 0) return FAILED;
  if ((*x <= smallIntValue(t)) == 0) return FAILED;
  
  return x.releaseNonRes();
}
OZ_C_proc_end
