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
  Assert(oz_isRef(v) || !oz_isVariable(v));
  //
  DEREF(v, _vptr, vtag);
  var = v;
  varPtr = _vptr;
  //
  if (isSmallIntTag(vtag)) {
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
    Assert(oz_isCVar(v));
    //
    OzVariable * cvar = tagged2CVar(v);
    //
    if (cvar->getType() == OZ_VAR_BOOL) {
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
      Assert(cvar->getType() == OZ_VAR_FD);
      //
      _domain = &((OzFDVariable *) cvar)->getDom();
      initial_size = CAST_FD_PTR(_domain)->getSize();
      setSort(int_e);
      //
      Assert((CAST_FD_PTR(_domain)->getSize() > 1) && (*_domain != fd_bool));
    }
  }
}

int OZ_FDIntVar::read(OZ_Term v)
{
  Assert(oz_isRef(v) || !oz_isVariable(v));
  //
  DEREF(v, _vptr, vtag);
  var    = v;
  varPtr = _vptr;
  //
  if (isSmallIntTag(vtag)) {
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
    Assert(oz_isCVar(v));
    //
    OzVariable * cvar = tagged2CVar(v);
    //
    int is_bool = (cvar->getTypeMasked() == OZ_VAR_BOOL);
    //
    // check if this variable has already been read as encapsulated
    // parameter and if so, initilize forward reference appropriately
    //
    OZ_FDIntVar * forward = (cvar->isParamEncapTagged()
                             ? (is_bool
                                ? ((OzBoolVariable *) cvar)->getTag()
                                : ((OzFDVariable *) cvar)->getTag()
                                )
                                : this);
    //
    if (Propagator::getRunningPropagator()->isLocal()
        || oz_isLocalVar(cvar)) {
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
        if (cvar->isParamNonEncapTagged()) {
          //
          // has already been read
          //
          // get previous
          //
          OZ_FDIntVar * prev = ((OzBoolVariable *) cvar)->getTag();
          _domain = &(prev->_copy);
          prev->_nb_refs += 1;
          //
        } else {
          //
          // is being read the first time
          //
          CAST_FD_OBJ(forward->_copy).initBool();
          _domain = &(forward->_copy);
          ((OzBoolVariable *) cvar)->tagNonEncapParam(forward);
          forward->_nb_refs += 1;
          //
        }
      } else {
        //
        // local finite domain variable
        //
        setSort(int_e);
        //
        if (cvar->isParamNonEncapTagged()) {
          //
          // has already been read
          //
          // get previous
          //
          OZ_FDIntVar * prev = ((OzFDVariable *) cvar)->getTag();
          _domain = prev->_domain;
          prev->_nb_refs += 1;
          //
        } else {
          //
          // is being read the first time
          //
          _domain = &((OzFDVariable *) cvar)->getDom();
          // special treatment of top-level variables
          if (oz_onToplevel()) {
            forward->_copy = *_domain;
          }
          ((OzFDVariable *) cvar)->tagNonEncapParam(forward);
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
        if (cvar->isParamNonEncapTagged()) {
          //
          // has already been read
          //
          OZ_FDIntVar * prev = ((OzBoolVariable *) cvar)->getTag();
          _domain = &(prev->_copy);
          prev->_nb_refs += 1;
          //
        } else {
          //
          // is being read the first time
          //
          CAST_FD_OBJ(forward->_copy).initBool();
          _domain = &(forward->_copy);
          ((OzBoolVariable *) cvar)->tagNonEncapParam(forward);
          forward->_nb_refs += 1;
        }
      } else {
        //
        // global finite domain variable
        //
        setSort(int_e);
        //
        //
        if (cvar->isParamNonEncapTagged()) {
          //
          // has already been read
          //
          // get previous
          //
          OZ_FDIntVar * prev = ((OzFDVariable *) cvar)->getTag();
          _domain = &(prev->_copy);
          prev->_nb_refs += 1;
          //
        } else {
          //
          // is being read the first time
          //
          forward->_copy = ((OzFDVariable *) cvar)->getDom();
          _domain = &(forward->_copy);
          ((OzFDVariable *) cvar)->tagNonEncapParam(forward);
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
  Assert(oz_isRef(v) || !oz_isVariable(v));
  //
  DEREF(v, _vptr, vtag);
  var    = v;
  varPtr = _vptr;
  //
  if (isSmallIntTag(vtag)) {
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
    Assert(oz_isCVar(v));
    //
    setState(encap_e);
    //
    OzVariable * cvar = tagged2CVar(v);
    //
    int is_bool = (cvar->getTypeMasked() == OZ_VAR_BOOL);
    //
    // check if this variable has already been read as non-encapsulated
    // parameter and if so, initilize forward reference appropriately
    //
    OZ_FDIntVar * forward = (cvar->isParamNonEncapTagged()
                             ? (is_bool
                                ? ((OzBoolVariable *) cvar)->getTag()
                                : ((OzFDVariable *) cvar)->getTag()
                                )
                             : this);
    //
    if (cvar->isParamEncapTagged()) {
      //
      // has already been read
      //
      setSort(is_bool ? bool_e : int_e);
      OZ_FDIntVar * prev = (is_bool
                            ? ((OzBoolVariable *) cvar)->getTag()
                            : ((OzFDVariable *) cvar)->getTag()
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
      cvar->tagEncapParam(forward);
      forward->_nb_refs += 1;
      //
    } else {
      //
      // found finite domain
      //
      Assert(cvar->getTypeMasked() == OZ_VAR_FD);
      //
      setSort(int_e);
      forward->_encap = ((OzFDVariable *) cvar)->getDom();
      _domain = &(forward->_encap);
      cvar->tagEncapParam(forward);
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

#define CHECK_BOUNDS                                    \
   (initial_width > CAST_FD_PTR(_domain)->getWidth())   \
     ? fd_prop_bounds : fd_prop_any

OZ_Boolean OZ_FDIntVar::tell(void)
{
  //
  // if the parameter is a variable it returns 1 else 0
  //
  DEBUG_CONSTRAIN_CVAR(("OZ_FDIntVar::tell "));
  //
  // this parameter has become an integer by a previous tell
  //
  if (!oz_isVariable(*varPtr)) {
    //
    goto oz_false;
    //
  } else {
    //
    OzVariable * cvar = tagged2CVar(var);
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
          ((OzFDVariable *) cvar)->becomesSmallIntAndPropagate(varPtr);
          //
        } else {
          //
          // global variable
          //
          int int_val = CAST_FD_PTR(_domain)->getSingleElem();
          //
          // wake up
          //
          ((OzFDVariable *) cvar)->propagate(fd_prop_singl);
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
          ((OzFDVariable *) cvar)->becomesBoolVarAndPropagate(varPtr);
          //
        } else {
          //
          // global variable
          //
          OzFDVariable * fdvar = (OzFDVariable *) cvar;
          //
          // wake up
          fdvar->propagate(CHECK_BOUNDS);
          // cast store variable to 0/1-varaible
          Board * fdvarhome = fdvar->getBoardInternal();
          OZ_Term * newboolvar = newTaggedCVar(new OzBoolVariable(fdvarhome));
          castGlobalVar(varPtr, newboolvar);
          //
        }
      } else {
        //
        // a finite domain variable stays a finite domain variable
        //
        ((OzFDVariable *) cvar)->propagate(CHECK_BOUNDS);
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
        ((OzBoolVariable *) cvar)->propagate();
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
  if (isSort(sgl_e)) {
    return;
  } else {
    //
    OzVariable * cvar = tagged2CVar(var);
    //
    int is_non_encap = cvar->isParamNonEncapTagged();
    //
    cvar->untagParam();
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
