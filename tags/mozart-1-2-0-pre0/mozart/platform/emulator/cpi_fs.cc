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
#include "var_fs.hh"

//-----------------------------------------------------------------------------

void OZ_FSetVar::ask(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVar(v));
  //
  DEREF(v, _vptr);
  var = v;
  varPtr = _vptr;
  //
  if (oz_isFSetValue(v)) {
    // 
    // found finite set value
    //
    _copy = *tagged2FSetValue(v);
    _set = &_copy;
    //
    setSort(val_e);
  } else {
    //
    // found finite set variable
    //
    Assert(isGenFSetVar(v));
    //
    OzFSVariable * fsvar = tagged2GenFSetVar(v);
    //
    _set = &fsvar->getSet();
    //
    setSort(var_e);
  }
}

void OZ_FSetVar::read(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVar(v));
  //
  DEREF(v, _vptr);
  var    = v;
  varPtr = _vptr;
  //
  if (oz_isFSetValue(v)) {
    //
    // finite set value
    //
    setSort(val_e);
    _copy = *tagged2FSetValue(v);
    _set = &_copy;
    //
  } else {
    //
    // found variable
    //
    Assert(oz_isVar(v));
    //
    setSort(var_e);
    //
    OzFSVariable * var = tagged2GenFSetVar(v);
    //
    // check if this variable has already been read as encapsulated
    // parameter and if so, initilize forward reference appropriately
    //
    OZ_FSetVar * forward = (var->isParamEncapTagged() 
			    ? ((OzFSVariable *) var)->getTag()
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
	OZ_FSetVar * prev = var->getTag();
	_set = prev->_set;
	prev->_nb_refs += 1;
	//
      } else {
	//
	// is being read the first time
	//
	_set = &((OzFSVariable *) var)->getSet();
	// special treatment of top-level variables
	if (oz_onToplevel()) {
	  forward->_copy = *_set;
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
	OZ_FSetVar * prev = var->getTag();
	_set = &(prev->_copy);
	prev->_nb_refs += 1;
	//
      } else {
	//
	// is being read the first time
	//
	forward->_copy = var->getSet();
	_set = &(forward->_copy);
	var->tagNonEncapParam(forward);
	forward->_nb_refs += 1;
	//
      }
    }
  }
  known_in     = _set->getKnownIn();
  known_not_in = _set->getKnownNotIn();
  card_size    = _set->getCardSize();
}

void OZ_FSetVar::readEncap(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVar(v));
  //
  DEREF(v, _vptr);
  var    = v;
  varPtr = _vptr;
  //
  if (oz_isFSetValue(v)) {
    //
    // found finite set value
    //
    setSort(val_e);
    setState(loc_e);
    _encap = *tagged2FSetValue(v);
    _set = &_encap;
  } else {
    //
    // found variable
    //
    setSort(var_e);
    setState(encap_e);
    //
    OzFSVariable * var = tagged2GenFSetVar(v);
    //
    // check if this variable has already been read as non-encapsulated
    // parameter and if so, initilize forward reference appropriately
    //
    OZ_FSetVar * forward = (var->isParamNonEncapTagged()
			    ? ((OzFSVariable *) var)->getTag()
			    : this);
    //
    if (var->isParamEncapTagged()) {
      //
      // has already been read
      //
      OZ_FSetVar * prev = var->getTag();
      //
      _set = &(prev->_encap);
      //
      prev->_nb_refs += 1;
      //
    } else {
      //
      // is being read the first time
      //
      forward->_encap = var->getSet();
      _set = &(forward->_encap);
      var->tagEncapParam(forward);
      forward->_nb_refs += 1;
      //
    }
  }
  known_in     = _set->getKnownIn();
  known_not_in = _set->getKnownNotIn();
  card_size    = _set->getCardSize();
}

OZ_Boolean OZ_FSetVar::tell(void)
{
  //
  // if the parameter is a variable it returns 1 else 0
  //
  DEBUG_CONSTRAIN_VAR(("OZ_FSetVar::tell "));
  //
  // this parameter has become an integer by a previous tell
  //
  if (!oz_isVar(*varPtr)) {
    //
    goto oz_false;
    //
  } else {
    //
    OzFSVariable *ov = tagged2GenFSetVar(var);
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
      // there is a finite set variable in the store
      //
      Assert(isSort(var_e));
      //
      if (_set->isValue()) {
	//
	// propagation produced a set value
	//
	if (isState(loc_e)) {
	  //
	  // local variable
	  //
	  tagged2GenFSetVar(var)->becomesFSetValueAndPropagate(varPtr);
	  //
	} else {
	  //
	  // global variable
	  //
	  OZ_FSetValue * set_value = new OZ_FSetValue(*_set);
	  //
	  // wake up
	  //
	  tagged2GenFSetVar(var)->propagate(fs_prop_val);
	  bindGlobalVarToValue(varPtr, makeTaggedFSetValue(set_value));
	  //
	}
	//
	goto oz_false;
	//
      } else {
	// 
	// propagation produced a set constraint
	//
	// wake up ...
	// ... lower bounds
	if (known_in < _set->getKnownIn()) {
	  tagged2GenFSetVar(var)->propagate(fs_prop_glb);
	}
	// ... upper bounds
	if (known_not_in < _set->getKnownNotIn()) {
	  tagged2GenFSetVar(var)->propagate(fs_prop_lub);
	}
	// ... cardinality
	if (card_size > _set->getCardSize()) {
	  tagged2GenFSetVar(var)->propagate(fs_prop_val);
	}
	//
	if (isState(glob_e)) {
	  //
	  // tell basic constraint to global variable
	  //
	  constrainGlobalVar(varPtr, *_set);
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

void OZ_FSetVar::fail(void)
{
  if (isSort(val_e)) {
    return;
  } else {
    //
    OzFSVariable *ov = tagged2GenFSetVar(var);
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
      *_set = _copy;
    }
  }
}

OZ_Boolean OZ_FSetVar::isTouched(void) const
{
  return ((known_in     < _set->getKnownIn()) ||
	  (known_not_in < _set->getKnownNotIn()) ||
	  (card_size    > _set->getCardSize()));
}

int OZ_getFSetInf(void)
{
  return fset_inf;
}

int OZ_getFSetSup(void)
{
  return fset_sup;
}


// End of File
//-----------------------------------------------------------------------------


