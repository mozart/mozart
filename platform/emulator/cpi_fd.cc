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

void * OZ_FDIntVar::operator new(size_t s)
{
  return CpiHeap.alloc(s);
}

void OZ_FDIntVar::operator delete(void * p, size_t s)
{
  // deliberately left empty
}

#ifdef __GNUC__
void * OZ_FDIntVar::operator new[](size_t s)
{
  return CpiHeap.alloc(s);
}

void OZ_FDIntVar::operator delete[](void * p, size_t s)
{
  // deliberately left empty
}
#endif

void OZ_FDIntVar::ask(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));

  DEREF(v, _vptr, vtag);
  var = v;
  varPtr = _vptr;
  //
  if (isSmallIntTag(vtag)) {
    initial_size = CAST_FD_OBJ(dom).initSingleton(tagged2SmallInt(v));
    domPtr = &dom;
    setSort(sgl_e);
  } else {
    OzVariable * cvar = tagged2CVar(v);

    if (cvar->getType() == OZ_VAR_BOOL) {
      initial_size = CAST_FD_OBJ(dom).initBool();
      domPtr = &dom;
      setSort(bool_e);
    } else {
      Assert(cvar->getType() == OZ_VAR_FD);

      domPtr = &((OzFDVariable *) cvar)->getDom();
      initial_size = CAST_FD_PTR(domPtr)->getSize();
      setSort(int_e);

      Assert(initial_size > 1 && *domPtr != fd_bool);
    }
  }
}

int OZ_FDIntVar::read(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));

  DEREF(v, _vptr, vtag);
  var = v;
  varPtr = _vptr;
  //
  if (isSmallIntTag(vtag)) {

    initial_size = CAST_FD_OBJ(dom).initSingleton(tagged2SmallInt(v));
    initial_width = 0;
    domPtr = &dom;
    setSort(sgl_e);

  } else {
    Assert(oz_isCVar(v));

    if (Propagator::getRunningPropagator()->isLocal()) {
    // local variable per definition

      setState(loc_e);
      OzVariable * cvar = tagged2CVar(v);

      if (cvar->testReifiedFlag()) {
      // may already be a reified var; then the type is incorrect
	if (cvar->isBoolPatched()) goto local_bool; else goto local_fd;
      } else if (cvar->getType() == OZ_VAR_BOOL) {
      local_bool:

	setSort(bool_e);

	if (cvar->testStoreFlag()) {
	// may already be entered as store var
	  initial_size = 2;
	  domPtr = ((OzBoolVariable *) cvar)->getStorePatchBool();
	} else {
	// entered first time as store var
	  initial_size = CAST_FD_OBJ(dom).initBool();
	  domPtr = &dom;
	  ((OzBoolVariable *) cvar)->patchStoreBool(domPtr);
	}
	initial_width = 1;
      } else {
      local_fd:

	setSort(int_e);

	if (oz_onToplevel())
	  dom = ((OzFDVariable *) cvar)->getDom();
	domPtr = &((OzFDVariable *) cvar)->getDom();
	initial_size = CAST_FD_PTR(domPtr)->getSize();
	initial_width = CAST_FD_PTR(domPtr)->getWidth();

	Assert(initial_size > 1 && *domPtr != fd_bool);
      }

    } else {
      // don't know before hand if local or global

      OzVariable * cvar = tagged2CVar(v);
      setState(oz_isLocalVar(cvar) ? loc_e : glob_e);

      if (cvar->testReifiedFlag()) {
      // may already be a reified var; then the type is incorrect
	if (cvar->isBoolPatched()) {
	  goto global_bool;
	} else {
	  goto global_fd;
	}
      } else if (cvar->getType() == OZ_VAR_BOOL) {
      global_bool:
	setSort(bool_e);

	if (cvar->testStoreFlag()) {
	// may already be entered as store var
	  domPtr = ((OzBoolVariable *) cvar)->getStorePatchBool();
	  initial_size = 2;
	} else {
	// entered first time as store var
	  domPtr = &dom;
	  ((OzBoolVariable *) cvar)->patchStoreBool(domPtr);
	  initial_size = CAST_FD_OBJ(dom).initBool();
	}
	initial_width = 1;

      } else {
      global_fd:

	if (isState(glob_e)) {
	  dom = ((OzFDVariable *) cvar)->getDom();
	  domPtr = &dom;
	} else {
	  if (oz_onToplevel()) {
	    dom = ((OzFDVariable *) cvar)->getDom();
	  }
	  domPtr = &((OzFDVariable *) cvar)->getDom();
	}

	initial_size = CAST_FD_PTR(domPtr)->getSize();
	initial_width = CAST_FD_PTR(domPtr)->getWidth();
	setSort(int_e);

	Assert(initial_size > 1 && *domPtr != fd_bool);
      }
    }

    setStoreFlag(v);
  }
  return CAST_FD_PTR(domPtr)->getSize();
}

int OZ_FDIntVar::readEncap(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));

  DEREF(v, _vptr, vtag);
  var = v;
  varPtr= _vptr;
  //
  if (isSmallIntTag(vtag)) {
    initial_size = CAST_FD_OBJ(dom).initSingleton(tagged2SmallInt(v));
    initial_width = 0;
    domPtr = &dom;
    setState(loc_e);
    setSort(sgl_e);
  } else {
    setState(encap_e);
    OzVariable * cvar = tagged2CVar(v);

    if (cvar->testReifiedFlag()) {
    // var is already entered somewhere else
      setSort(cvar->isBoolPatched() ? bool_e : int_e);
      domPtr = cvar->getReifiedPatch();
      initial_size = CAST_FD_PTR(domPtr)->getSize();
      initial_width = CAST_FD_PTR(domPtr)->getWidth();
    } else if (cvar->getType() == OZ_VAR_BOOL) {
    // bool var entered first time
      initial_size = CAST_FD_OBJ(dom).initBool();
      initial_width = 1;
      domPtr = &dom;
      setSort(bool_e);
      cvar->patchReified(domPtr, TRUE);
    } else {
    // fd var entered first time
      Assert(cvar->getType() == OZ_VAR_FD);

      setSort(int_e);
      CAST_FD_OBJ(dom).initEmpty();
      dom = ((OzFDVariable *) cvar)->getDom();
      domPtr = &dom;
      initial_size = CAST_FD_PTR(domPtr)->getSize();
      initial_width = CAST_FD_PTR(domPtr)->getWidth();

      cvar->patchReified(domPtr, FALSE);

      Assert(initial_size > 1 && *domPtr != fd_bool);
    }
    setReifiedFlag(v);
  }
  return CAST_FD_PTR(domPtr)->getSize();
}

#define CHECK_BOUNDS					\
   (initial_width > CAST_FD_PTR(domPtr)->getWidth())	\
     ? fd_prop_bounds : fd_prop_any

OZ_Boolean OZ_FDIntVar::tell(void)
{
  DEBUG_CONSTRAIN_CVAR(("OZ_FDIntVar::tell "));

  // someone else has already determined it
  if (!oz_isVariable(*varPtr)) {
    goto oz_false;
  }
  //
  if (testReifiedFlag(var)) {
    unpatchReifiedFD(var, isSort(bool_e));
  }
  //
  if (!testResetStoreFlag(var)) {
    // the constraint has already been told, i.e., there were at least
    // two OZ_FDIntVar connected to the same store variable
    goto oz_false;
  } else if(!isTouched()) {
    // no constraints have been imposed by the current propagator run.
    // note the cases catches integers too.
    goto oz_true;
  } else if (isSort(int_e)) {
    //
    // there is a finite domain variable in the store
    //
    if (*CAST_FD_PTR(domPtr) == fd_singl) {
      //
      // propagation produced singleton
      //
      if (isState(loc_e)) {
	// local variable
	tagged2GenFDVar(var)->becomesSmallIntAndPropagate(varPtr);
      } else {
	// global variable
	int int_val = CAST_FD_PTR(domPtr)->getSingleElem();
	// wake up
	tagged2GenFDVar(var)->propagate(fd_prop_singl);
	bindGlobalVarToValue(varPtr, makeTaggedSmallInt(int_val));
      }
      goto oz_false;
    } else if (*CAST_FD_PTR(domPtr) == fd_bool) {
      //
      // propagation produced boolean domain
      //
      if (isState(loc_e)) {
	// local variable
	tagged2GenFDVar(var)->becomesBoolVarAndPropagate(varPtr);
      } else {
	// global variable
	// wake up
	tagged2GenFDVar(var)->propagate(CHECK_BOUNDS);
	// cast store variable to boolean varaible
	Board * fdvarhome = tagged2GenFDVar(var)->getBoardInternal();
	OZ_Term * newboolvar = newTaggedCVar(new OzBoolVariable(fdvarhome));
	castGlobalVar(varPtr, newboolvar);
      }
      goto oz_true;
    } else {
      //
      // propagation produced proper domain
      //
      tagged2GenFDVar(var)->propagate(CHECK_BOUNDS);
      if (isState(glob_e)) {
	// constrain global variable
	constrainGlobalVar(varPtr, dom);
      }
      goto oz_true;
    }
  } else {
    //
    // there is a boolean variable in the store which becomes a singleton
    // (otherwise it would not been recognized as touched)
    //
    Assert(isSort(bool_e) && *CAST_FD_PTR(domPtr) == fd_singl); 
    // boolean variable

    if (isState(loc_e)) {
      // local variable
      tagged2GenBoolVar(var)->becomesSmallIntAndPropagate(varPtr, *domPtr);
    } else {
      // global variable
      tagged2GenBoolVar(var)->propagate();
      int int_val = CAST_FD_PTR(domPtr)->getSingleElem();
      bindGlobalVarToValue(varPtr, makeTaggedSmallInt(int_val));
    }
    goto oz_false;
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

void OZ_FDIntVar::fail(void)
{
  if (isSort(sgl_e))
    return;
  if (isState(encap_e)) {
    unpatchReifiedFD(var, isSort(bool_e));
    return;
  }

  // dont't change the order of the calls (side effects!)
  if (testResetStoreFlag(var) && isState(glob_e) && isSort(int_e)) {
    *domPtr = dom;
  } else if (oz_onToplevel()) {
    *domPtr = dom;
  }
}

// End of File
//-----------------------------------------------------------------------------


