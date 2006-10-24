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

void OZ_FDIntVar::ask(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVar(v));
  //
  DEREF(v, _vptr);
  var = v;
  varPtr = _vptr;
  //
  if (oz_isSmallInt(v)) {
    //
    // found integer
    //
    initial_size = CAST_FD_OBJ(_copy).initSingleton(tagged2SmallInt(v));
    _domain = &_copy;
    setSort(sgl_e);
  } else {
    // 
    // found variable
    //
    Assert(oz_isVar(v));
    //
    OzVariable * var = tagged2Var(v);
    //
    if (var->getType() == OZ_VAR_BOOL) {
      //
      // 0/1-variable
      //
      initial_size = CAST_FD_OBJ(_copy).initBool();
      _domain = &_copy;
      setSort(bool_e);
    } else {
      //
      // finite domain variable
      //
      Assert(var->getType() == OZ_VAR_FD);
      //
      _domain = &((OzFDVariable *) var)->getDom();
      initial_size = CAST_FD_PTR(_domain)->getSize();
      setSort(int_e);
      //
      Assert((CAST_FD_PTR(_domain)->getSize() > 1) && (*_domain != fd_bool));
    }
  }
}

int OZ_FDIntVar::read(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVar(v));
  //
  DEREF(v, _vptr);
  var    = v;
  varPtr = _vptr;
  //
  if (oz_isSmallInt(v)) {
    //
    // found integer
    //
    setSort(sgl_e);
    setState(loc_e);
    CAST_FD_OBJ(_copy).initSingleton(tagged2SmallInt(v));
    _domain = &_copy;
    //
  } else {
    //
    // found variable
    //
    Assert(oz_isVar(v));
    //
    OzVariable * var = tagged2Var(v);
    //
    int is_bool = (var->getTypeMasked() == OZ_VAR_BOOL);
    //
    // check if this variable has already been read as encapsulated
    // parameter and if so, initilize forward reference appropriately
    //
    OZ_FDIntVar * forward = (var->isParamEncapTagged() 
			     ? (is_bool 
				? ((OzBoolVariable *) var)->getTag() 
				: ((OzFDVariable *) var)->getTag()
				) 
				: this);
    //
    if (Propagator::getRunningPropagator()->isLocal()
	|| oz_isLocalVar(var)) {
      //
      // local variable
      //
      setState(loc_e);
      //
      if (is_bool) {
	// 
	// local 0/1-variable
	//
	setSort(bool_e);
	//
	if (var->isParamNonEncapTagged()) {
	  //
	  // has already been read
	  //
	  // get previous 
	  //
	  OZ_FDIntVar * prev = ((OzBoolVariable *) var)->getTag();
	  _domain = &(prev->_copy);
	  prev->_nb_refs += 1;
	  //
	} else {
	  //
	  // is being read the first time
	  //
	  CAST_FD_OBJ(forward->_copy).initBool();
	  _domain = &(forward->_copy);
	  ((OzBoolVariable *) var)->tagNonEncapParam(forward);
	  forward->_nb_refs += 1;
	  //
	}
      } else {
	//
	// local finite domain variable
	//
	setSort(int_e);
	//
	if (var->isParamNonEncapTagged()) {
	  //
	  // has already been read
	  //
	  // get previous 
	  //
	  OZ_FDIntVar * prev = ((OzFDVariable *) var)->getTag();
	  _domain = prev->_domain;
	  prev->_nb_refs += 1;
	  //
	} else {
	  //
	  // is being read the first time
	  //
	  _domain = &((OzFDVariable *) var)->getDom();
	  // special treatment of top-level variables
	  if (oz_onToplevel()) {
	    forward->_copy = *_domain;
	  }
	  ((OzFDVariable *) var)->tagNonEncapParam(forward);
	  forward->_nb_refs += 1;
	  //
	}
	//
	Assert((CAST_FD_PTR(_domain)->getSize() > 1) && (*_domain != fd_bool));
      }
    } else {
      // 
      // global variable
      //
      setState(glob_e);
      //
      if (is_bool) {
	// 
	// global 0/1-variable
	//
	setSort(bool_e);
	//
	if (var->isParamNonEncapTagged()) {
	  //
	  // has already been read
	  //
	  OZ_FDIntVar * prev = ((OzBoolVariable *) var)->getTag();
	  _domain = &(prev->_copy);
	  prev->_nb_refs += 1;
	  //
	} else {
	  //
	  // is being read the first time
	  //
	  CAST_FD_OBJ(forward->_copy).initBool();
	  _domain = &(forward->_copy);
	  ((OzBoolVariable *) var)->tagNonEncapParam(forward);
	  forward->_nb_refs += 1;
	}
      } else {
	//
	// global finite domain variable
	//
	setSort(int_e);
	//
	//
	if (var->isParamNonEncapTagged()) {
	  //
	  // has already been read
	  //
	  // get previous 
	  //
	  OZ_FDIntVar * prev = ((OzFDVariable *) var)->getTag();
	  _domain = &(prev->_copy);
	  prev->_nb_refs += 1;
	  //
	} else {
	  //
	  // is being read the first time
	  //
	  forward->_copy = ((OzFDVariable *) var)->getDom();
	  _domain = &(forward->_copy);
	  ((OzFDVariable *) var)->tagNonEncapParam(forward);
	  forward->_nb_refs += 1;
	  //
	}
	//
	Assert((CAST_FD_PTR(_domain)->getSize() > 1) && (*_domain != fd_bool));
      }
    }
  }
  //
  initial_size  = CAST_FD_PTR(_domain)->getSize();
  initial_width = CAST_FD_PTR(_domain)->getWidth();
  //
  return initial_size;
}

int OZ_FDIntVar::readEncap(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVar(v));
  //
  DEREF(v, _vptr);
  var    = v;
  varPtr = _vptr;
  //
  if (oz_isSmallInt(v)) {
    //
    // found integer
    //
    setSort(sgl_e);
    CAST_FD_OBJ(_encap).initSingleton(tagged2SmallInt(v));
    _domain = &_encap;
  } else {
    //
    // found variable
    //
    Assert(oz_isVar(v));
    //
    setState(encap_e);
    //
    OzVariable * var = tagged2Var(v);
    //
    int is_bool = (var->getTypeMasked() == OZ_VAR_BOOL);
    //
    // check if this variable has already been read as non-encapsulated
    // parameter and if so, initilize forward reference appropriately
    //
    OZ_FDIntVar * forward = (var->isParamNonEncapTagged() 
			     ? (is_bool 
				? ((OzBoolVariable *) var)->getTag() 
				: ((OzFDVariable *) var)->getTag()
				) 
			     : this);
    //
    if (var->isParamEncapTagged()) {
      //
      // has already been read
      //
      setSort(is_bool ? bool_e : int_e);
      OZ_FDIntVar * prev = (is_bool 
			    ? ((OzBoolVariable *) var)->getTag() 
			    : ((OzFDVariable *) var)->getTag() 
			    );
      _domain = &(prev->_encap);
      prev->_nb_refs += 1;
      //
    } else if (is_bool) {
      //
      // 0/1-variable is being read the first time
      //
      setSort(bool_e);
      CAST_FD_OBJ(forward->_encap).initBool();
      _domain = &(forward->_encap);
      var->tagEncapParam(forward);
      forward->_nb_refs += 1;
      //
    } else {
      // 
      // found finite domain
      //
      Assert(var->getTypeMasked() == OZ_VAR_FD);
      //
      setSort(int_e);
      forward->_encap = ((OzFDVariable *) var)->getDom();
      _domain = &(forward->_encap);
      var->tagEncapParam(forward);
      forward->_nb_refs += 1;
      //
      Assert((CAST_FD_PTR(_domain)->getSize() > 1) && (*_domain != fd_bool));
      //
    }
  }
  //
  initial_size  = CAST_FD_PTR(_domain)->getSize();
  initial_width = CAST_FD_PTR(_domain)->getWidth();
  //
  return initial_size;
}

#define CHECK_BOUNDS					\
   (initial_width > CAST_FD_PTR(_domain)->getWidth())	\
     ? fd_prop_bounds : fd_prop_any

OZ_Boolean OZ_FDIntVar::tell(void)
{
  //
  // if the parameter is a variable it returns 1 else 0
  //
  DEBUG_CONSTRAIN_VAR(("OZ_FDIntVar::tell "));
  //
  // this parameter has become an integer by a previous tell
  //
  if (!oz_isVar(*varPtr)) {
    //
    goto oz_false;
    //
  } else {
    //
    OzVariable *ov = tagged2Var(var);
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
    } else if (isSort(int_e)) {
      //
      // values have been removed from a finite domain variable in the store
      //
      if (*CAST_FD_PTR(_domain) == fd_singl) {
	//
	// a finite domain variable becomes an integer (singleton domain)
	//
	if (isState(loc_e)) {
	  //
	  // local variable
	  //
	  ((OzFDVariable *) ov)->becomesSmallIntAndPropagate(varPtr);
	  //
	} else {
	  //
	  // global variable
	  //
	  int int_val = CAST_FD_PTR(_domain)->getSingleElem();
	  //	
	  // wake up
	  //
	  ((OzFDVariable *) ov)->propagate(fd_prop_singl);
	  bindGlobalVarToValue(varPtr, makeTaggedSmallInt(int_val));
	  //
	}
	//
	goto oz_false;
	//
      } else if (*CAST_FD_PTR(_domain) == fd_bool) {
	//
	// a finite domain variable becomes a 0/1-variable (0/1-domain)
	//
	if (isState(loc_e)) {
	  //
	  // local variable
	  //
	  ((OzFDVariable *) ov)->becomesBoolVarAndPropagate(varPtr);
	  //
	} else {
	  //
	  // global variable
	  //
	  OzFDVariable * fdvar = (OzFDVariable *) ov;
	  //
	  // wake up
	  fdvar->propagate(CHECK_BOUNDS);
	  // cast store variable to 0/1-varaible
	  Board * fdvarhome = fdvar->getBoardInternal();
	  OZ_Term * newboolvar = newTaggedVar(new OzBoolVariable(fdvarhome));
	  castGlobalVar(varPtr, newboolvar);
	  //
	}
      } else {
	//
	// a finite domain variable stays a finite domain variable
	//
	((OzFDVariable *) ov)->propagate(CHECK_BOUNDS);
	if (isState(glob_e)) {
	  //
	  // tell basic constraint to global variable
	  //
	  constrainGlobalVar(varPtr, *_domain);
	  //
	}
      }
      //
      goto oz_true;
      //
    } else {
      //
      // there is a boolean variable in the store 
      //
      // it becomes an integer since it has been detected as touched.
      //
      Assert(isSort(bool_e) && *CAST_FD_PTR(_domain) == fd_singl); 
      //
      // 0/1-variable
      //
      if (isState(loc_e)) {
	//
	// local variable
	//
	tagged2GenBoolVar(var)->becomesSmallIntAndPropagate(varPtr, *_domain);
	//
      } else {
	//
	// global variable
	//
	((OzBoolVariable *) ov)->propagate();
	int int_val = CAST_FD_PTR(_domain)->getSingleElem();
	bindGlobalVarToValue(varPtr, makeTaggedSmallInt(int_val));
      }
      goto oz_false;
    }
  }
  //
  // this line must never be executed
  //
  Assert(0);
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

void OZ_FDIntVar::fail(void)
{
  if (isSort(sgl_e)) {
    return; 
  } else {
    //
    OzVariable *ov = tagged2Var(var);
    //
    int is_non_encap = ov->isParamNonEncapTagged();
    //
    ov->untagParam();
    //
    if (! is_non_encap) {
      //
      // this parameter has already been untagged or is an
      // encapsulated parameter which needs no special care (in contrast
      // to global and top-level variables).
      //
      return;
    } else if ((isState(glob_e) && isSort(int_e)) 
	       || oz_onToplevel()) {
      *_domain = _copy;
    }
  }
}

// End of File
//-----------------------------------------------------------------------------
