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
#include "var_ct.hh"

//-----------------------------------------------------------------------------

void * OZ_Ct::operator new(size_t s, int)
{
  return doubleMalloc(s);
}

void OZ_Ct::operator delete(void * p, size_t s)
{
  freeListDispose(p, s);
}

void OZ_CtVar::ask(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));
  //
  DEREF(v, _vptr, vtag);
  var = v;
  varPtr = _vptr;
  //
  if (oz_isVariable(vtag)) {
    //
    // found constrained variable
    //
    setSort(var_e);
    //
    ctRefConstraint(tagged2GenCtVar(v)->getConstraint());
    //
  } else {
    //
    // found value
    //
    setSort(val_e);
    //
    ctSetValue(v);
    //
  }
}

void OZ_CtVar::read(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));
  //
  DEREF(v, _vptr, vtag);
  var = v;
  varPtr = _vptr;
  //
  if (oz_isVariable(vtag)) {
    //
    // found constrained variable
    //
    Assert(isCVarTag(vtag));
    //
    setSort(var_e);
    //
    OzCtVariable * cvar = tagged2GenCtVar(v);
    //
    //
    // check if this variable has already been read as encapsulated
    // parameter and if so, initilize forward reference appropriately
    //
    OZ_CtVar * forward = (cvar->isParamEncapTagged()
                          ? ((OzCtVariable *) cvar)->getTag()
                          : this);
    //
    if (Propagator::getRunningPropagator()->isLocal()
        || oz_isLocalVar(cvar)) {
      //
      // local variable
      //
      setState(loc_e);
      //
      if (cvar->isParamNonEncapTagged()) {
        //
        // has already been read
        //
        // get previous
        //
        OZ_CtVar * prev = cvar->getTag();
        //
        ctRefConstraint(prev->ctGetConstraint());
        prev->_nb_refs += 1;
        //
      } else {
        //
        // is being read the first time
        //
        OZ_Ct * constr = ctRefConstraint(cvar->getConstraint());
        // special treament for top-level variables
        if (oz_onToplevel()) {
          forward->ctSaveConstraint(constr);
        }
        cvar->tagNonEncapParam(forward);
        forward->_nb_refs += 1;
        //
      }
    } else {
      //
      // global variable
      //
      setState(glob_e);
      //
      if (cvar->isParamNonEncapTagged()) {
        //
        // has already been read
        //
        // get previous
        //
        OZ_CtVar * prev = cvar->getTag();
        ctRefConstraint(prev->ctGetConstraint());
        prev->_nb_refs += 1;
        //
      } else {
        //
        // is being read the first time
        //
        OZ_Ct * constr = cvar->getConstraint();
        ctRefConstraint(forward->ctSaveConstraint(constr));
        cvar->tagNonEncapParam(forward);
        forward->_nb_refs += 1;
        //
      }
    }
  } else {
    //
    // found value
    //
    setSort(val_e);
    //
    ctSetValue(v);
    //
  }
  ctSetConstraintProfile();
}


void OZ_CtVar::readEncap(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));
  //
  DEREF(v, _vptr, vtag);
  var = v;
  varPtr = _vptr;
  //
  if (oz_isVariable(v)) {
    //
    // found variable
    //
    Assert(isCVarTag(vtag));
    //
    setState(encap_e);
    setSort(var_e);
    //
    OzCtVariable * cvar = tagged2GenCtVar(v);
    //
    // check if this variable has already been read as non-encapsulated
    // parameter and if so, initilize forward reference appropriately
    //
    OZ_CtVar * forward = (cvar->isParamNonEncapTagged()
                          ? ((OzCtVariable *) cvar)->getTag()
                          : this);
    //
    if (cvar->isParamEncapTagged()) {
      //
      // has already been read
      //
      OZ_CtVar * prev = cvar->getTag();
      //
      ctRefConstraint(prev->ctGetConstraint());
      //
      prev->_nb_refs += 1;
      //
    } else {
      //
      // is being read the first time
      //
      ctRefConstraint(forward->ctSaveEncapConstraint(cvar->getConstraint()));
      cvar->tagEncapParam(forward);
      forward->_nb_refs += 1;
      //
    }
  } else {
    //
    // found value
    //
    setSort(val_e);
    //
    ctSetValue(v);
    //
  }
  ctSetConstraintProfile();
}

OZ_Boolean OZ_CtVar::tell(void)
{
  //
  // if the parameter is a variable it returns 1 else 0
  //
  DEBUG_CONSTRAIN_CVAR(("OZ_CtVar::tell "));
  //
  // this parameter has become an integer by a previous tell
  //
  if (!oz_isVariable(*varPtr)) {
    //
    goto oz_false;
    //
  } else {
    //
    OzCtVariable * cvar = tagged2GenCtVar(var);
    //
    int is_non_encap = cvar->isParamNonEncapTagged();
    //
    cvar->untagParam();
    //
    if (! is_non_encap) {
      //
      // the basic constraint of this parameter has already been told,
      // i.e., there are at least two parameter refering to the same
      // variable in the constraint store
      //
      goto oz_false;
      //
    } else if(!isTouched()) {
      //
      // no values have been removed from the constraint connected to
      // this parameter. note this cases catches integers too.
      //
      goto oz_true;
      //
    } else {
      //
      // there is a constrained variable in the store
      //
      Assert(isSort(var_e));
      //
      //
      OZ_Ct * constr       = ctGetConstraint();
      //
      if (constr->isValue()) {
        //
        // propagation produced a value
        //
        if (isState(loc_e)) {
          //
          // local variable
          //
          // wake up
          //
          cvar->propagate(OZ_WAKEUP_ALL, pc_propagator);
          bindLocalVarToValue(varPtr, constr->toValue());
        } else {
          //
          // global variable
          //
          // wake up
          //
          cvar->propagate(OZ_WAKEUP_ALL, pc_propagator);
          bindGlobalVarToValue(varPtr, constr->toValue());
        }
        goto oz_false;
      } else {
        //
        // propagation produced a set constraint
        //
        // wake up ...
        OZ_CtWakeUp wakeup_descr = ctGetWakeUpDescriptor();
        cvar->propagate(wakeup_descr, pc_propagator);
        //
        if (isState(glob_e)) {
          //
          // tell basic constraint to global variable
          //
          constrainGlobalVar(varPtr, constr);
          //
        }
        goto oz_true;
      }
    }
  }
  //
 oz_false:
  //
  // variable is determined
  //
  DEBUG_CONSTRAIN_CVAR(("FALSE\n"));
  return OZ_FALSE;
  //
 oz_true:
  //
  // variable is still undetermined
  //
  DEBUG_CONSTRAIN_CVAR(("TRUE\n"));
  return OZ_TRUE;
}

void OZ_CtVar::fail(void)
{
  if (isSort(val_e)) {
    return;
  } else {
    //
    OzCtVariable * cvar = tagged2GenCtVar(var);
    //
    int is_non_encap = cvar->isParamNonEncapTagged();
    //
    cvar->untagParam();
    //
    if (! is_non_encap) {
      //
      // this parameter has already been untagged or is an
      // encapsulated parameter which needs no special care (in
      // contrast to global and top-level variables).
      //
      return;
    } else if ((isState(glob_e) && isSort(var_e))
               || oz_onToplevel()) {
      ctRestoreConstraint();
    }
  }
}

OZ_Return OZ_mkCtVar(OZ_Term v, OZ_Ct * c, OZ_CtDefinition * d)
{
  return tellBasicConstraint(v, c, d);
}

// eof
//-----------------------------------------------------------------------------
