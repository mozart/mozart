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
  return oz_heapMalloc(s);
}

void OZ_Ct::operator delete(void * p, size_t s)
{
  oz_freeListDisposeUnsafe(p, s);
}

void OZ_CtVar::ask(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVar(v));
  //
  DEREF(v, _vptr);
  var = v;
  varPtr = _vptr;
  //
  Assert(!oz_isRef(v));
  if (oz_isVarOrRef(v)) {
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
  Assert(oz_isRef(v) || !oz_isVar(v));
  //
  DEREF(v, _vptr);
  var = v;
  varPtr = _vptr;
  //
  Assert(!oz_isRef(v));
  if (oz_isVarOrRef(v)) {
    //
    // found constrained variable
    //
    Assert(oz_isVar(v));
    //
    setSort(var_e);
    //
    OzCtVariable * var = tagged2GenCtVar(v);
    //
    //
    // check if this variable has already been read as encapsulated
    // parameter and if so, initilize forward reference appropriately
    //
    OZ_CtVar * forward = (var->isParamEncapTagged() 
			  ? ((OzCtVariable *) var)->getTag()
			  : this);
    //
    if (Propagator::getRunningPropagator()->isLocal()
	|| oz_isLocalVar(var)) {
      //
      // local variable
      //
      setState(loc_e);
      //
      if (var->isParamNonEncapTagged()) {
	//
	// has already been read
	//
	// get previous
	//
	OZ_CtVar * prev = var->getTag();
	//
	ctRefConstraint(prev->ctGetConstraint());
	prev->_nb_refs += 1;
	//
      } else {
	//
	// is being read the first time
	//
	OZ_Ct * constr = ctRefConstraint(var->getConstraint());
	// special treament for top-level variables
	if (oz_onToplevel()) {
	  forward->ctSaveConstraint(constr);
	}
	var->tagNonEncapParam(forward);
	forward->_nb_refs += 1;
	//
      }
    } else {
      // 
      // global variable
      //
      setState(glob_e);
      //
      if (var->isParamNonEncapTagged()) {
	//
	// has already been read
	//
	// get previous
	//
	OZ_CtVar * prev = var->getTag();
	ctRefConstraint(prev->ctGetConstraint());
	prev->_nb_refs += 1;
	//
      } else {
	//
	// is being read the first time
	//
	OZ_Ct * constr = var->getConstraint();
	ctRefConstraint(forward->ctSaveConstraint(constr));
	var->tagNonEncapParam(forward);
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
  Assert(oz_isRef(v) || !oz_isVar(v));
  //
  DEREF(v, _vptr);
  var = v;
  varPtr = _vptr;
  //
  Assert(!oz_isRef(v));
  if (oz_isVarOrRef(v)) {
    //
    // found variable
    //
    Assert(oz_isVar(v));
    // 
    setState(encap_e);
    setSort(var_e);
    //
    OzCtVariable * var = tagged2GenCtVar(v);
    //
    // check if this variable has already been read as non-encapsulated
    // parameter and if so, initilize forward reference appropriately
    //
    OZ_CtVar * forward = (var->isParamNonEncapTagged() 
			  ? ((OzCtVariable *) var)->getTag()
			  : this);
    //
    if (var->isParamEncapTagged()) {
      //
      // has already been read
      //
      OZ_CtVar * prev = var->getTag();
      //
      ctRefConstraint(prev->ctGetConstraint());
      //
      prev->_nb_refs += 1;
      //
    } else {
      //
      // is being read the first time
      //
      ctRefConstraint(forward->ctSaveEncapConstraint(var->getConstraint()));
      var->tagEncapParam(forward);
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
  DEBUG_CONSTRAIN_VAR(("OZ_CtVar::tell "));
  //
  // this parameter has become an integer by a previous tell
  //
  if (!oz_isVar(*varPtr)) {
    //
    goto oz_false;
    //
  } else {
    //
    OzCtVariable *ov = tagged2GenCtVar(var);
    //
    int is_non_encap = ov->isParamNonEncapTagged();
    //
    ov->untagParam();
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
	  ov->propagate(OZ_WAKEUP_ALL, pc_propagator);
	  bindLocalVarToValue(varPtr, constr->toValue());
	} else {
	  //
	  // global variable
	  //
	  // wake up
	  //
	  ov->propagate(OZ_WAKEUP_ALL, pc_propagator);
	  bindGlobalVarToValue(varPtr, constr->toValue());
	}
	goto oz_false;
      } else {
	// 
	// propagation produced a set constraint
	//
	// wake up ...
	OZ_CtWakeUp wakeup_descr = ctGetWakeUpDescriptor();
	ov->propagate(wakeup_descr, pc_propagator);
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
  DEBUG_CONSTRAIN_VAR(("FALSE\n"));
  return OZ_FALSE;
  //
 oz_true:
  //
  // variable is still undetermined
  //
  DEBUG_CONSTRAIN_VAR(("TRUE\n"));
  return OZ_TRUE;
}

void OZ_CtVar::fail(void)
{
  if (isSort(val_e)) {
    return;
  } else {
    //
    OzCtVariable *ov = tagged2GenCtVar(var);
    //
    int is_non_encap = ov->isParamNonEncapTagged();
    //
    ov->untagParam();
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
