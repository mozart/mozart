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
  OZ_getCArgDeref(0, term, tptr, tag);

  return isGenFDVar(term) == OK ? PROCEED : FAILED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIgetFDLimits,2)
{ 
  return (OZ_Bool)(OZ_unify(newSmallInt(fdMinBA),OZ_getCArg(0)) &&
		   OZ_unify(newSmallInt(fdMaxBA),OZ_getCArg(1)));
}
OZ_C_proc_end


// Non-suspending version of min and max
// unconstrained variable is going to be constrained to an FD-variable
OZ_C_proc_begin(BIfdMin,2)
{
  OZ_getCArgDeref(0, fdarg, fdptr, fdtag);

  if(isInt(fdtag) == OK)
    return OZ_unify(fdarg, OZ_getCArg(1));   

  if(isAnyVar(fdarg) == OK)
    if(isGenFDVar(fdarg) == OK){
      int minVal = tagged2GenFDVar(fdarg)->getDom().minElem();
      return OZ_unify(newSmallInt(minVal), OZ_getCArg(1));   
    } else {
      Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
      CondSuspList * csl = new CondSuspList(susp, NULL, isConstrained);
      addVirtualConstr(taggedBecomesSuspVar(fdptr), csl);
      return PROCEED;
    }

  warning("Expected either small integer or (fd-) variable in BIfdMin.");
  return FAILED;
}
OZ_C_proc_end     


OZ_C_proc_begin(BIfdMax,2)
{
  OZ_getCArgDeref(0, fdarg, fdptr, fdtag);

  if(isInt(fdtag) == OK)
    return OZ_unify(fdarg, OZ_getCArg(1));   
  

  if(isAnyVar(fdarg) == OK)
    if(isGenFDVar(fdarg) == OK){
      int maxVal = tagged2GenFDVar(fdarg)->getDom().maxElem();
      return OZ_unify(newSmallInt(maxVal), OZ_getCArg(1));   
    } else {
      Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
      CondSuspList * csl = new CondSuspList(susp, NULL, isConstrained);
      addVirtualConstr(taggedBecomesSuspVar(fdptr), csl);
      return PROCEED;
    }
  
  warning("Expected either small integer or (fd-) variable in BIfdMax.");
  return FAILED;
}
OZ_C_proc_end     


OZ_C_proc_begin(BIfdGetAsList, 2)
{
  OZ_getCArgDeref(0, fdvar, fdptr, fdtag);
  
  if(isInt(fdtag) == OK) {
    LTuple * ltuple = new LTuple(fdvar, AtomNil);
    return OZ_unify(makeTaggedLTuple(ltuple), OZ_getCArg(1));
  } else {
    if(isAnyVar(fdvar) == OK) {
      if(isGenFDVar(fdvar) == OK) {
	FiniteDomain &fdomain = tagged2GenFDVar(fdvar)->getDom();
	return OZ_unify(fdomain.getAsList(), OZ_getCArg(1));
      } else {
	Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
	CondSuspList * csl = new CondSuspList(susp, NULL, isConstrained);
	addVirtualConstr(taggedBecomesSuspVar(fdptr), csl);
	return PROCEED;
      }
    }
  }
  warning("Expected either small integer or (fd-) variable in BIfdGetAsList.");
  return FAILED;
}
OZ_C_proc_end     


OZ_C_proc_begin(BIfdGetCardinality,2)
{
  OZ_getCArgDeref(0, fdvar, fdptr, fdtag);

  if(isInt(fdtag) == OK){
    return OZ_unify(newSmallInt(1), OZ_getCArg(1));
  } else {
    if(isAnyVar(fdvar) == OK) {
      if(isGenFDVar(fdvar) == OK) {
	FiniteDomain &fdomain = tagged2GenFDVar(fdvar)->getDom();
	int card = fdomain.getSize();
	return OZ_unify(newSmallInt(card), OZ_getCArg(1));
      } else {
	Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
	CondSuspList * csl = new CondSuspList(susp, NULL, isConstrained);
	addVirtualConstr(taggedBecomesSuspVar(fdptr), csl);
	return PROCEED;
      }
    }
  }
  warning("Expected either small integer or fd-variable.");
  return FAILED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdPutLe, 2)
{
  OZ_getCArgDeref(0, var, varPtr, varTag);
  OZ_getCArgDeref(1, n, nPtr, nTag);

  if (isAnyVar(nTag) == OK){
    Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
    CondSuspList * ncsl = new CondSuspList(susp, NULL, isDet);
    addVirtualConstr(taggedBecomesSuspVar(nPtr), ncsl);
    return PROCEED;
  }    
  if (isSmallInt(nTag) == NO){
    warning("Small integer expected at %s:%d.", __FILE__, __LINE__);
    return FAILED;
  }

  if (isNotCVar(varTag) == OK){
    Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
    CondSuspList * csl = new CondSuspList(susp, NULL, isConstrained);
    addVirtualConstr(taggedBecomesSuspVar(varPtr), csl);
    return PROCEED;
  }

  return fdLessEqual(var, varPtr, smallIntValue(n));
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdPutGe, 2)
{
  OZ_getCArgDeref(0, var, varPtr, varTag);
  OZ_getCArgDeref(1, n, nPtr, nTag);

  if (isAnyVar(nTag) == OK){
    Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
    CondSuspList * ncsl = new CondSuspList(susp, NULL, isDet);
    addVirtualConstr(taggedBecomesSuspVar(nPtr), ncsl);
    return PROCEED;
  }
  if (isSmallInt(nTag) == NO){
    warning("Small integer expected at %s:%d.", __FILE__, __LINE__);
    return FAILED;
  }

  if (isNotCVar(varTag) == OK){
    Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
    CondSuspList * csl = new CondSuspList(susp, NULL, isConstrained);
    addVirtualConstr(taggedBecomesSuspVar(varPtr), csl);
    return PROCEED;
  }

  return fdGreaterEqual(var, varPtr, smallIntValue(n));
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdPutList, 3)
{
  OZ_getCArgDeref(0, var, varPtr, varTag);
  OZ_getCArgDeref(1, list, listPtr, listTag);
  OZ_getCArgDeref(2, sign, signPtr, signTag);

  if (isSmallInt(signTag) == NO){
    warning("sign expected to be small integer");
    return FAILED;
  }
  
  if (isAnyVar(listTag) == OK) {
    Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
    CondSuspList * csl = new CondSuspList(susp, NULL, isConstrained);
    addVirtualConstr(taggedBecomesSuspVar(listPtr), csl);
    return PROCEED;
  }

  if (isLTuple(listTag) == NO){
    warning("LTuple expected at %s:%d.", __FILE__, __LINE__);
    return FAILED;
  }

  int elemListLen = 0;
  static elemList[fdMaxBA - fdMinBA +1];
  BitArray bitArray;
  bitArray.empty();
  
  while(isLTuple(list) == OK) {
    TaggedRef ival = tagged2LTuple(list)->getHead();
    
    // find small int pointed by current list node
    while(isRef(ival))
      ival = * (TaggedRef *)ival;
    if(!isSmallInt(ival)){
      warning("fdPutList: list of small ints expected at %s:%d.",
	      __FILE__, __LINE__);
      return FAILED;
    } else {
      int i = smallIntValue(ival);
      if (bitArray.contains(i) == NO && i >= fdMinBA && i <= fdMaxBA) {
	elemList[elemListLen++] = i;
	bitArray.setBit(i);
      }
    }
    // find next list node
    list = tagged2LTuple(list)->getTail();
    while(isRef(list) == OK)
      list = * (TaggedRef *) list;
  } // while
  
  return smallIntValue(sign) == 0
    ? fdList(var, varPtr, elemListLen, elemList)
    : fdNotList(var, varPtr, elemListLen, elemList);
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdPutNot, 2)
{
  OZ_getCArgDeref(0, var, varPtr, varTag);
  OZ_getCArgDeref(1, n, nPtr, nTag);

  if (isAnyVar(nTag) == OK){
    Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
    CondSuspList * ncsl = new CondSuspList(susp, NULL, isDet);
    addVirtualConstr(taggedBecomesSuspVar(nPtr), ncsl);
    return PROCEED;
  }
  if (isSmallInt(nTag) == NO){
    warning("Small integer expected at %s:%d.", __FILE__, __LINE__);
    return FAILED;
  }

  if (isNotCVar(varTag) == OK){
    Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
    CondSuspList * csl = new CondSuspList(susp, NULL, isConstrained);
    addVirtualConstr(taggedBecomesSuspVar(varPtr), csl);
    return PROCEED;
  }

  return fdNot(var, varPtr, smallIntValue(n));
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdPutFromTo, 3){
  OZ_getCArgDeref(0, var, varPtr, varTag);
  OZ_getCArgDeref(1, from, fromPtr, fromTag);
  OZ_getCArgDeref(2, to, toPtr, toTag);

  if (isAnyVar(fromTag) == OK){
    Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
    CondSuspList * fromcsl = new CondSuspList(susp, NULL, isDet);
    addVirtualConstr(taggedBecomesSuspVar(fromPtr),fromcsl);
    return PROCEED;
  }
  if (isSmallInt(fromTag) == NO){
    warning("Small integer expected at %s:%d.", __FILE__, __LINE__);
    return FAILED;
  }

  if (isAnyVar(toTag) == OK){
    Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
    CondSuspList * tocsl = new CondSuspList(susp, NULL, isDet);
    addVirtualConstr(taggedBecomesSuspVar(toPtr), tocsl);
    return PROCEED;
  }
  if (isSmallInt(toTag) == NO){
    warning("Small integer expected at %s:%d.", __FILE__, __LINE__);
    return FAILED;
  }

  return fdFromTo(var, varPtr, smallIntValue(from), smallIntValue(to));
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdNextTo, 3)
{
  OZ_getCArgDeref(0, var, varPtr, varTag);
  OZ_getCArgDeref(1, n, nPtr, nTag);
  OZ_Term next = OZ_getCArg(2);

  if (isNotCVar(varTag) == OK){
    Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
    CondSuspList * csl = new CondSuspList(susp, NULL, isConstrained);
    addVirtualConstr(taggedBecomesSuspVar(varPtr), csl);
    return PROCEED;
  }

  if (isGenFDVar(var) == NO){
    warning("Expected fd-variable at this point of BINextTo.");
    return FAILED;
  }

  if (isAnyVar(nTag) == OK){
    Suspension * susp = createNonResidentSusp(OZ_self, OZ_args, OZ_arity);
    CondSuspList * csl = new CondSuspList(susp, NULL, isDet);
    addVirtualConstr(taggedBecomesSuspVar(nPtr), csl);
    return PROCEED;
  }

  if (isSmallInt(nTag) == NO){
    warning("Expected small integer at this point of BINextTo.");
    return FAILED;
  }
      
  int nextVal, nVal = smallIntValue(n);
  if (tagged2GenFDVar(var)->getDom().next(nVal, nextVal))
    return OZ_unify(next,makeRangeTuple(nextVal, 2 * nVal - nextVal));
  else
    return OZ_unify(next, newSmallInt(nextVal));
}
OZ_C_proc_end
