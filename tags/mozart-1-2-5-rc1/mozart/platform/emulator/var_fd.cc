/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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

#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_fd.hh"
#endif

#include "var_fd.hh"
#include "var_bool.hh"
#include "fdomn.hh"
#include "am.hh"
#include "unify.hh"

OZ_Return OzFDVariable::bind(OZ_Term * vPtr, OZ_Term term)
{
  DEBUG_CONSTRAIN_VAR(("bindFD "));

  Assert(!oz_isRef(term));

  if (!oz_isSmallInt(term)){
    DEBUG_CONSTRAIN_VAR(("FAILED\n"));
    return FAILED;
  }
  if (! finiteDomain.isIn(tagged2SmallInt(term))) {
    DEBUG_CONSTRAIN_VAR(("FAILED\n"));
    return FAILED;
  }

  Bool isLocalVar = oz_isLocalVar(this);

  propagate(fd_prop_singl);

  if (isLocalVar) {
    bindLocalVarToValue(vPtr, term);
    dispose();
  } else {
    bindGlobalVarToValue(vPtr, term);
  }

  DEBUG_CONSTRAIN_VAR(("PROCEED\n"));
  return PROCEED;
}

// unify expects either two OzFDVariables or at least one
// OzFDVariable and one non-variable
// invariant: left term (ie var)  == *this
// Only if a local variable is bound relink its suspension list, since
// global variables are trailed.(ie. their suspension lists are
// implicitely relinked.)
OZ_Return OzFDVariable::unify(OZ_Term * left_varptr, OZ_Term * right_varptr)
{
  DEBUG_CONSTRAIN_VAR(("unifyFD "));
  //
  OZ_Term right_var       = *right_varptr;
  OzVariable *right_ov = tagged2Var(right_var);
  //
  if (right_ov->getType() == OZ_VAR_BOOL) {
    DEBUG_CONSTRAIN_VAR(("branch to OzBoolVariable::unify\n"));
    return ((OzBoolVariable *) right_ov)->unify(right_varptr, left_varptr);
  }
  OzFDVariable * right_fdvar = (OzFDVariable *) right_ov;
  Bool left_var_is_local     = oz_isLocalVar(this);
  Bool right_var_is_local    = oz_isLocalVar(right_fdvar);
  //
  if (!left_var_is_local && right_var_is_local) {
    DEBUG_CONSTRAIN_VAR(("global-local (swapping)"));
    //
    // left variable is global and right variable is local
    //
    // swap variables to be unified and recurse
    return unify(right_varptr, left_varptr);
  }
  //
  if (right_ov->getType() != OZ_VAR_FD) {
    goto failed;
  }
  //
  {
    // compute intersection of domains ...
    OZ_FiniteDomain &right_dom   = right_fdvar->finiteDomain;
    OZ_FiniteDomain intersection = finiteDomain & right_dom;
    //
    if (intersection == fd_empty) {
      goto failed;
    }
    // 
    if (left_var_is_local && right_var_is_local) {
      DEBUG_CONSTRAIN_VAR(("local-local"));
      //
      // left and right variable are local
      //
      if (intersection == fd_singl) {
	// intersection is singleton
	OZ_Term int_var = 
	  makeTaggedSmallInt(CAST_FD_OBJ(intersection).getSingleElem());
	// wake up 
	right_fdvar->propagateUnify();
	propagateUnify();
	// bind variables to integer value
	bindLocalVarToValue(left_varptr, int_var);
	bindLocalVarToValue(right_varptr, int_var);
	// dispose variables
	dispose();
	right_fdvar->dispose();
      } else if (heapNewer(left_varptr, right_varptr)) {
	// bind left variable to right variable
	if (intersection == fd_bool) {
	  // intersection has boolean domain
	  OzBoolVariable * right_boolvar = right_fdvar->becomesBool();
	  propagateUnify();
	  right_boolvar->propagateUnify();
	  relinkSuspListTo(right_boolvar);
	  bindLocalVar(left_varptr, right_varptr);
	} else {
	  // intersection has proper domain
	  right_fdvar->setDom(intersection);
	  propagateUnify();
	  right_fdvar->propagateUnify();
	  relinkSuspListTo(right_fdvar);
	  bindLocalVar(left_varptr, right_varptr);
	}
	// dispose left_var
	dispose();
      } else {
	// bind right variable to left variable
	if (intersection == fd_bool) {
	  // intersection has boolean domain
	  OzBoolVariable * left_boolvar = becomesBool();
	  right_fdvar->propagateUnify();
	  left_boolvar->propagateUnify();
	  right_fdvar->relinkSuspListTo(left_boolvar);
	  bindLocalVar(right_varptr, left_varptr);
	} else {
	  // intersection has proper domain
	  setDom(intersection);
	  right_fdvar->propagateUnify();
	  propagateUnify();
	  right_fdvar->relinkSuspListTo(this);
	  bindLocalVar(right_varptr, left_varptr);
	}
	// dispose right_var
	right_fdvar->dispose();
      }
    } else if (left_var_is_local && !right_var_is_local) {
      DEBUG_CONSTRAIN_VAR(("local-global"));
      //
      // left variable is local and right variable is global
      //
      if (intersection == fd_singl) {
	// intersection is singleton
	OZ_Term int_var = 
	  makeTaggedSmallInt(CAST_FD_OBJ(intersection).getSingleElem());
	right_fdvar->propagateUnify();
	propagateUnify();
	bindLocalVarToValue(left_varptr, int_var);
	bindGlobalVarToValue(right_varptr, int_var);
	// dispose left_var
	dispose();
      } else if (intersection== fd_bool) {
	// intersection has boolean domain
	Board * right_fdvar_home = right_fdvar->getBoardInternal();
	OzBoolVariable * right_boolvar = new OzBoolVariable(right_fdvar_home);
	OZ_Term * right_varptr_bool =
	  newTaggedVar(new OzBoolVariable(right_fdvar_home));
	// wake up
	right_fdvar->propagateUnify();
	propagateUnify();
	// cast and bind
	castGlobalVar(right_varptr, right_varptr_bool);
	bindLocalVar(left_varptr, right_varptr_bool);
      } else {
	// intersection has proper domain
	right_fdvar->propagateUnify();
	if (intersection.getSize() < right_dom.getSize()) {
	  // intersection constrains domain of global variable
	  constrainGlobalVar(right_varptr, intersection);
	}
	propagateUnify();
	bindLocalVar(left_varptr, right_varptr);
	// dispose local variable
	dispose();
      }
    } else {
      DEBUG_CONSTRAIN_VAR(("global-global"));
      //
      // left and right variable are global
      //
      Assert(!left_var_is_local && !right_var_is_local);
      //
      // note bind from left to right since left var is more local than
      // right one (important for stablity/space merging)
      //
      if (intersection == fd_singl){
	// intersection is singleton
	OZ_Term int_val = 
	  makeTaggedSmallInt(CAST_FD_OBJ(intersection).getSingleElem());
	propagateUnify();
	right_fdvar->propagateUnify();
	bindGlobalVarToValue(left_varptr, int_val);
	bindGlobalVarToValue(right_varptr, int_val);
      } else if (intersection == fd_bool) {
	// intersection has boolean domain
	Board * right_fdvar_home = right_fdvar->getBoardInternal();
	OzBoolVariable * right_boolvar =
	  new OzBoolVariable(right_fdvar_home);
	OZ_Term * right_varptr_bool =
	  newTaggedVar(new OzBoolVariable(right_fdvar_home));
	//
	propagateUnify();
	right_fdvar->propagateUnify();
	// bind left variable to right variable ..
	bindGlobalVar(left_varptr, right_varptr);
	// .. and cast right variable to boolean variable
	castGlobalVar(right_varptr, right_varptr_bool);
      } else {
	// intersection has proper domain
	propagateUnify();
	right_fdvar->propagateUnify();
	// bind left variable to right variable ..
	bindGlobalVar(left_varptr, right_varptr);
	//
	if (intersection.getSize() < right_dom.getSize()) {
	  constrainGlobalVar(right_varptr, intersection);
	}
      }
    }
    DEBUG_CONSTRAIN_VAR(("SUCCEEDED\n"));
    return TRUE;
  }
 failed:
  DEBUG_CONSTRAIN_VAR(("FAILED\n"));
  return FAILED;
} // OzFDVariable::unify

Bool OzFDVariable::valid(TaggedRef val)
{
  Assert(!oz_isRef(val));
  return (oz_isSmallInt(val) && finiteDomain.isIn(tagged2SmallInt(val)));
}

void OzFDVariable::relinkSuspListTo(OzBoolVariable * lv, Bool reset_local)
{
  OzVariable::relinkSuspListTo(lv, reset_local); // any

  for (int i = 0; i < fd_prop_any; i += 1)
    fdSuspList[i] =
      fdSuspList[i]->appendToAndUnlink(lv->suspList, reset_local);

}


void OzFDVariable::relinkSuspListToItself(Bool reset_local)
{
  for (int i = 0; i < fd_prop_any; i += 1)
    fdSuspList[i]->appendToAndUnlink(suspList, reset_local);
}


void OzFDVariable::becomesBoolVarAndPropagate(TaggedRef * trPtr)
{
  if (isGenBoolVar(*trPtr)) return;

  propagate(fd_prop_bounds);
  becomesBool();
}

int OzFDVariable::intersectWithBool(void)
{
  return ((OZ_FiniteDomainImpl *) &finiteDomain)->intersectWithBool();
}

OZ_Return tellBasicConstraint(OZ_Term v, OZ_FiniteDomain * fd)
{
  DEBUG_CONSTRAIN_VAR(("tellBasicConstraintFD "));

  DEREF(v, vptr);

  if (fd && (*fd == fd_empty)) {
    goto failed;
  }
  Assert(!oz_isRef(v));
  if (oz_isFree(v)) {

#ifdef COUNT_PROP_INVOCS
    extern int count_prop_invocs_fdvars_created;
    count_prop_invocs_fdvars_created += 1;
#endif

    //
    // tell finite domain constraint to an unconstrained variable
    //
    if (! fd) {
      goto fdvariable;
    }
    // fd is singleton domain and hence v becomes integer. otherwise ..
    if (fd->getSize() == 1) {
      if (oz_isLocalVariable(v)) {
	if (!oz_isOptVar(v))
	  oz_checkSuspensionListProp(tagged2Var(v));
	bindLocalVarToValue(vptr, 
			    makeTaggedSmallInt(CAST_FD_PTR(fd)->getSingleElem()));
      } else {
	bindGlobalVarToValue(vptr, 
			     makeTaggedSmallInt(CAST_FD_PTR(fd)->getSingleElem()));
      }
      goto proceed;
    }

    // .. create a finite domain variable
  fdvariable:
    OzVariable * cv =
      (fd
       ? (*fd == fd_bool
	  ? (OzVariable *) new OzBoolVariable(oz_currentBoard())
	  : new OzFDVariable(*fd, oz_currentBoard())
	  )
       : new OzFDVariable(oz_currentBoard()));
    OZ_Term *  tcv = newTaggedVar(cv);

    if (oz_isLocalVariable(v)) {
      if (!oz_isOptVar(v)) {
	oz_checkSuspensionListProp(tagged2Var(v));
      }
      bindLocalVar(vptr, tcv);
    } else {
      bindGlobalVar(vptr, tcv);
    }

    goto proceed;
  } else if (isGenFDVar(v)) {
    //
    // tell finite domain constraint to a finite domain variable
    //
    if (! fd) {
      goto proceed;
    }
    OzFDVariable * fdvar = tagged2GenFDVar(v);
    OZ_FiniteDomain dom  = (fdvar->getDom() & *fd);
    Board * fdvarhome    = fdvar->getBoardInternal();

    if (dom == fd_empty) {
      goto failed;
    }
    if (dom.getSize() == fdvar->getDom().getSize()) {
      goto proceed;
    }
    if (dom == fd_singl) {
      //
      // singleton domain
      //
      if (oz_isLocalVar(fdvar)) {
	fdvar->getDom() = dom;
	fdvar->becomesSmallIntAndPropagate(vptr);
      } else {
	int singl = CAST_FD_OBJ(dom).getSingleElem();
	fdvar->propagate(fd_prop_singl);
	bindGlobalVarToValue(vptr, makeTaggedSmallInt(singl));
      }
    } else if (dom == fd_bool) {
      // 
      // boolean domain
      //
      if (oz_isLocalVar(fdvar)) {
	fdvar->becomesBoolVarAndPropagate(vptr);
      } else {
	fdvar->propagate(fd_prop_bounds);
	castGlobalVar(vptr, newTaggedVar(new OzBoolVariable(fdvarhome)));
      }
    } else {
      // 
      // proper finite domain
      //
      fdvar->propagate(fd_prop_bounds);
      if (oz_isLocalVar(fdvar)) {
	fdvar->getDom() = dom;
      } else {
	constrainGlobalVar(vptr, dom);
      }
    }
    goto proceed;
  } else if (isGenBoolVar(v)) {
    //
    // tell finite domain constraint to a boolean variable
    //
    if (! fd) goto proceed;

    int dom = fd->intersectWithBool();

    if (dom == -2) goto failed;
    if (dom == -1) goto proceed;

    OzBoolVariable * boolvar = tagged2GenBoolVar(v);
    if (oz_isLocalVar(boolvar)) {
      boolvar->becomesSmallIntAndPropagate(vptr, dom);
    } else {
      boolvar->propagate();
      bindGlobalVarToValue(vptr, makeTaggedSmallInt(dom));
    }
    goto proceed;
  } else if (oz_isSmallInt(v)) {
    //
    // tell finite domain constraint to a integer
    //
    if (! fd) goto proceed;

    if (fd->isIn(tagged2SmallInt(v)))
      goto proceed;
  } else if (oz_isVarOrRef(v)) {
    // 
    // future stuff, no idea what is going on here 
    TaggedRef newVar = oz_newVariable();
    OZ_Return ret = tellBasicConstraint(newVar, fd);
    Assert(ret == PROCEED);
    return oz_unify(makeTaggedRef(vptr), newVar);
  }

failed:
  DEBUG_CONSTRAIN_VAR(("FAILED\n"));
  return FAILED;

proceed:
  DEBUG_CONSTRAIN_VAR(("PROCEED\n"));
  return PROCEED;
}

// inline DISABLED CS
void OzFDVariable::propagate(OZ_FDPropState state,
			      PropCaller prop_eq)
{
  if (prop_eq == pc_propagator) {
    switch (state) {
    case fd_prop_singl: // no break
      if (fdSuspList[fd_prop_singl])
	OzVariable::propagateLocal(fdSuspList[fd_prop_singl], prop_eq);
    case fd_prop_bounds: // no break
      if (fdSuspList[fd_prop_bounds])
	OzVariable::propagateLocal(fdSuspList[fd_prop_bounds], prop_eq);
    default:
      break;
    }
  } else {
    OzVariable::propagateLocal(fdSuspList[fd_prop_singl], prop_eq);
    OzVariable::propagateLocal(fdSuspList[fd_prop_bounds], prop_eq);
  }
  if (suspList)
    OzVariable::propagate(suspList, prop_eq);
}

/*
 * Trailing support
 *
 */

OzVariable * OzFDVariable::copyForTrail(void) {
  return new OzFDVariable(getDom(), oz_currentBoard());
}

void OzFDVariable::restoreFromCopy(OzFDVariable * c) {
  OZ_FiniteDomain tmp = getDom();
  setDom(c->getDom());
  c->setDom(tmp);
  tmp.disposeExtension();
}


#ifdef OUTLINE
#define inline
#include "var_fd.icc"
#undef inline
#endif

