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

#ifdef CORRECT_UNIFY
//-----------------------------------------------------------------------------
OZ_Return OzFDVariable::bind(OZ_Term * vPtr, OZ_Term term)
{
  DEBUG_CONSTRAIN_CVAR(("bindFD "));

  Assert(!oz_isRef(term));

  if (!oz_isSmallInt(term)){
    DEBUG_CONSTRAIN_CVAR(("FAILED\n"));
    return FAILED;
  }
  if (! finiteDomain.isIn(smallIntValue(term))) {
    DEBUG_CONSTRAIN_CVAR(("FAILED\n"));
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

  DEBUG_CONSTRAIN_CVAR(("PROCEED\n"));
  return PROCEED;
}
//-----------------------------------------------------------------------------
#else

OZ_Return OzFDVariable::bind(TaggedRef * vPtr, TaggedRef term)
{
  Assert(!oz_isRef(term));
  if (!oz_isSmallInt(term)) return FAILED;
  if (! finiteDomain.isIn(smallIntValue(term))) {
    return FAILED;
  }

  Bool isLocalVar = oz_isLocalVar(this);
  Bool isNotInstallingScript = !am.isInstallingScript();

#ifdef SCRIPTDEBUG
  printf("fd-int %s\n", isLocalVar ? "local" : "global"); fflush(stdout);
#endif

  if (!am.inEqEq() && (isNotInstallingScript || isLocalVar))
    propagate(fd_prop_singl);

  if (isLocalVar) {
    DoBind(vPtr, term);
    dispose();
  } else {
    DoBindAndTrail(vPtr, term);
  }

  return PROCEED;
}

#endif /* CORRECT_UNIFY */

// unify expects either two OzFDVariables or at least one
// OzFDVariable and one non-variable
// invariant: left term (ie var)  == *this
// Only if a local variable is bound relink its suspension list, since
// global variables are trailed.(ie. their suspension lists are
// implicitely relinked.)
#ifdef CORRECT_UNIFY
//-----------------------------------------------------------------------------
OZ_Return OzFDVariable::unify(OZ_Term * left_varptr, OZ_Term * right_varptr)
{
  DEBUG_CONSTRAIN_CVAR(("unifyFD "));
  //
  OZ_Term right_var       = *right_varptr;
  OzVariable * right_cvar = tagged2CVar(right_var);
  //
  if (right_cvar->getType() == OZ_VAR_BOOL) {
    DEBUG_CONSTRAIN_CVAR(("branch to OzBoolVariable::unify\n"));
    return ((OzBoolVariable *)right_cvar)->unify(right_varptr, left_varptr);
  }
  OzFDVariable * right_fdvar = (OzFDVariable *) right_cvar;
  Bool left_var_is_local     = oz_isLocalVar(this);
  Bool right_var_is_local    = oz_isLocalVar(right_fdvar);
  //
  if (!left_var_is_local && right_var_is_local) {
    DEBUG_CONSTRAIN_CVAR(("global-local (swapping)"));
    //
    // left variable is global and right variable is local
    //
    // swap variables to be unified and recurse
    return unify(right_varptr, left_varptr);
  }
  //
  if (right_cvar->getType() != OZ_VAR_FD) {
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
      DEBUG_CONSTRAIN_CVAR(("local-local"));
      //
      // left and right variable are local
      //
      if (intersection == fd_singl) {
        // intersection is singleton
        OZ_Term int_var = newSmallInt(intersection.getSingleElem());
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
      DEBUG_CONSTRAIN_CVAR(("local-global"));
      //
      // left variable is local and right variable is global
      //
      if (intersection == fd_singl) {
        // intersection is singleton
        OZ_Term int_var = newSmallInt(intersection.getSingleElem());
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
          newTaggedCVar(new OzBoolVariable(right_fdvar_home));
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
      DEBUG_CONSTRAIN_CVAR(("global-global"));
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
        OZ_Term int_val = newSmallInt(intersection.getSingleElem());
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
          newTaggedCVar(new OzBoolVariable(right_fdvar_home));
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
    DEBUG_CONSTRAIN_CVAR(("SUCCEEDED\n"));
    return TRUE;
  }
 failed:
  DEBUG_CONSTRAIN_CVAR(("FAILED\n"));
  return FAILED;
} // OzFDVariable::unify
//-----------------------------------------------------------------------------
#else
OZ_Return OzFDVariable::unify(TaggedRef * vPtr, TaggedRef *tPtr)
{
#ifdef SCRIPTDEBUG
  printf(am.isInstallingScript()
         ? "fd installing script\n"
         : "fd NOT installing script\n");
  fflush(stdout);
#endif

  TaggedRef term = *tPtr;
  OzVariable *cv=tagged2CVar(term);
  if (cv->getType()!=OZ_VAR_FD) return FAILED;

  // compute intersection of domains ...
  OzFDVariable * termVar = (OzFDVariable *)cv;
  OZ_FiniteDomain &termDom = termVar->finiteDomain;
  OZ_FiniteDomain intsct;

  if ((intsct = finiteDomain & termDom) == fd_empty) {
    return FALSE;
  }

  // bind - trail - propagate
  Bool varIsLocal =  oz_isLocalVar(this);
  Bool termIsLocal = oz_isLocalVar(termVar);

  Bool isNotInstallingScript = !am.isInstallingScript();
  Bool varIsConstrained = isNotInstallingScript ||
    (intsct.getSize() < finiteDomain.getSize());
  Bool termIsConstrained = isNotInstallingScript ||
    (intsct.getSize() < termDom.getSize());

  switch (varIsLocal + 2 * termIsLocal) {
  case TRUE + 2 * TRUE: // var and term are local
    {
#ifdef SCRIPTDEBUG
      printf("fd-fd local local\n"); fflush(stdout);
#endif
      if (intsct == fd_singl) {
        TaggedRef int_var = newSmallInt(intsct.getSingleElem());
        termVar->propagateUnify();
        propagateUnify();
        DoBind(vPtr, int_var);
        DoBind(tPtr, int_var);
        dispose();
        termVar->dispose();
      } else if (heapNewer(vPtr, tPtr)) { // bind var to term
        if (intsct == fd_bool) {
          OzBoolVariable * tbvar = termVar->becomesBool();
          propagateUnify();
          tbvar->propagateUnify();
          relinkSuspListTo(tbvar);
          DoBind(vPtr, makeTaggedRef(tPtr));
        } else {
          termVar->setDom(intsct);
          propagateUnify();
          termVar->propagateUnify();
          relinkSuspListTo(termVar);
          DoBind(vPtr, makeTaggedRef(tPtr));
        }
        dispose();
      } else { // bind term to var
        if (intsct == fd_bool) {
          OzBoolVariable * bvar = becomesBool();
          termVar->propagateUnify();
          bvar->propagateUnify();
          termVar->relinkSuspListTo(bvar);
          DoBind(tPtr, makeTaggedRef(vPtr));
        } else {
          setDom(intsct);
          termVar->propagateUnify();
          propagateUnify();
          termVar->relinkSuspListTo(this);
          DoBind(tPtr, makeTaggedRef(vPtr));
        }
        termVar->dispose();
      }
      break;
    }
  case TRUE + 2 * FALSE: // var is local and term is global
    {
#ifdef SCRIPTDEBUG
      printf("fd-fd local global\n"); fflush(stdout);
#endif
      if (intsct.getSize() != termDom.getSize()){
        if (intsct == fd_singl) {
          TaggedRef int_var = newSmallInt(intsct.getSingleElem());
          if (isNotInstallingScript) termVar->propagateUnify();
          if (varIsConstrained) propagateUnify();
          DoBind(vPtr, int_var);
          DoBindAndTrail(tPtr, int_var);
          dispose();
        } else {
          if (intsct == fd_bool) {
            OzBoolVariable * bvar = becomesBool();
            if (isNotInstallingScript) termVar->propagateUnify();
            if (varIsConstrained) bvar->propagateUnify();
            DoBindAndTrailAndIP(tPtr, makeTaggedRef(vPtr),
                                bvar, termVar);
          } else {
            setDom(intsct);
            if (isNotInstallingScript) termVar->propagateUnify();
            if (varIsConstrained) propagateUnify();
            DoBindAndTrailAndIP(tPtr, makeTaggedRef(vPtr),
                                this, termVar);
          }
        }
      } else {
        if (isNotInstallingScript) termVar->propagateUnify();
        if (varIsConstrained) propagateUnify();
        relinkSuspListTo(termVar, TRUE);
        DoBind(vPtr, makeTaggedRef(tPtr));
        dispose();
      }
      break;
    }
  case FALSE + 2 * TRUE: // var is global and term is local
    {
#ifdef SCRIPTDEBUG
      printf("fd-fd global local\n"); fflush(stdout);
#endif
      if (intsct.getSize() != finiteDomain.getSize()){
        if(intsct == fd_singl) {
          TaggedRef int_term = newSmallInt(intsct.getSingleElem());
          if (isNotInstallingScript) propagateUnify();
          if (termIsConstrained) termVar->propagateUnify();
          DoBind(tPtr, int_term);
          DoBindAndTrail(vPtr, int_term);
          termVar->dispose();
        } else {
          if (intsct == fd_bool) {
            OzBoolVariable * tbvar = termVar->becomesBool();
            if (isNotInstallingScript) propagateUnify();
            if (termIsConstrained) tbvar->propagateUnify();
            DoBindAndTrailAndIP(vPtr, makeTaggedRef(tPtr),
                                tbvar, this);
          } else {
            termVar->setDom(intsct);
            if (isNotInstallingScript) propagateUnify();
            if (termIsConstrained) termVar->propagateUnify();
            DoBindAndTrailAndIP(vPtr, makeTaggedRef(tPtr),
                                termVar, this);
          }
        }
      } else {
        if (termIsConstrained) termVar->propagateUnify();
        if (isNotInstallingScript) propagateUnify();
        termVar->relinkSuspListTo(this, TRUE);
        DoBind(tPtr, makeTaggedRef(vPtr));
        termVar->dispose();
      }
      break;
    }
  case FALSE + 2 * FALSE: // var and term is global
    {
#ifdef SCRIPTDEBUG
      printf("fd-fd global global\n"); fflush(stdout);
#endif
      if (intsct == fd_singl){
        TaggedRef int_val = newSmallInt(intsct.getSingleElem());
        if (!am.inEqEq()) {
          if (varIsConstrained) propagateUnify();
          if (termIsConstrained) termVar->propagateUnify();
        }
        DoBindAndTrail(vPtr, int_val);
        DoBindAndTrail(tPtr, int_val);
      } else {
        if (intsct == fd_bool) {
          OzBoolVariable * c_var
            = new OzBoolVariable(oz_currentBoard());
          TaggedRef * var_val = newTaggedCVar(c_var);
          if (!am.inEqEq()) {
            if (varIsConstrained) propagateUnify();
            if (termIsConstrained) termVar->propagateUnify();
          }
          DoBindAndTrailAndIP(vPtr, makeTaggedRef(var_val),
                              c_var, this);
          DoBindAndTrailAndIP(tPtr, makeTaggedRef(var_val),
                              c_var, termVar);
        } else {
          OzFDVariable * c_var
            = new OzFDVariable(intsct,oz_currentBoard());
          TaggedRef * var_val = newTaggedCVar(c_var);
          if (!am.inEqEq()) {
            if (varIsConstrained) propagateUnify();
            if (termIsConstrained) termVar->propagateUnify();
          }
          DoBindAndTrailAndIP(vPtr, makeTaggedRef(var_val),
                              c_var, this);
          DoBindAndTrailAndIP(tPtr, makeTaggedRef(var_val),
                              c_var, termVar);
        }
      }
      break;
    }
    ExhaustiveSwitch();
  } // switch (varIsLocal + 2 * termIsLocal) {
  return TRUE;
} // OzFDVariable::unify
#endif /* CORRECT_UNIFY */

Bool OzFDVariable::valid(TaggedRef val)
{
  Assert(!oz_isRef(val));
  return (oz_isSmallInt(val) && finiteDomain.isIn(smallIntValue(val)));
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
//-----------------------------------------------------------------------------
#ifdef CORRECT_UNIFY

  DEBUG_CONSTRAIN_CVAR(("tellBasicConstraintFD "));

  DEREF(v, vptr, vtag);

  if (fd && (*fd == fd_empty)) {
    goto failed;
  }
  if (oz_isFree(v)) {
    //
    // tell finite domain constraint to an unconstrained variable
    //
    if (! fd) {
      goto fdvariable;
    }
    // fd is singleton domain and hence v becomes integer. otherwise ..
    if (fd->getSize() == 1) {
      if (oz_isLocalVariable(vptr)) {
        if (!isUVar(vtag))
          oz_checkSuspensionListProp(tagged2CVar(v));
        bindLocalVarToValue(vptr, newSmallInt(fd->getSingleElem()));
      } else {
        bindGlobalVarToValue(vptr, newSmallInt(fd->getSingleElem()));
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
    OZ_Term *  tcv = newTaggedCVar(cv);

    if (oz_isLocalVariable(vptr)) {
      if (!isUVar(vtag)) {
        oz_checkSuspensionListProp(tagged2CVar(v));
        cv->setSuspList(tagged2CVar(v)->unlinkSuspList());
      }
      bindLocalVar(vptr, tcv);
    } else {
      bindGlobalVar(vptr, tcv);
    }

    goto proceed;
  } else if (isGenFDVar(v, vtag)) {
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
        int singl = dom.getSingleElem();
        fdvar->propagate(fd_prop_singl);
        bindGlobalVarToValue(vptr, newSmallInt(singl));
      }
    } else if (dom == fd_bool) {
      //
      // boolean domain
      //
      if (oz_isLocalVar(fdvar)) {
        fdvar->becomesBoolVarAndPropagate(vptr);
      } else {
        fdvar->propagate(fd_prop_bounds);
        castGlobalVar(vptr, newTaggedCVar(new OzBoolVariable(fdvarhome)));
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
  } else if (isGenBoolVar(v, vtag)) {
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
      bindGlobalVarToValue(vptr, newSmallInt(dom));
    }
    goto proceed;
  } else if (isSmallIntTag(vtag)) {
    //
    // tell finite domain constraint to a integer
    //
    if (! fd) goto proceed;

    if (fd->isIn(smallIntValue(v)))
      goto proceed;
  } else if (oz_isVariable(v)) {
    //
    // future stuff, no idea what is going on here
    TaggedRef newVar = oz_newVariable();
    OZ_Return ret = tellBasicConstraint(newVar, fd);
    Assert(ret == PROCEED);
    return oz_unify(makeTaggedRef(vptr), newVar);
  }

failed:
  DEBUG_CONSTRAIN_CVAR(("FAILED\n"));
  return FAILED;

proceed:
  DEBUG_CONSTRAIN_CVAR(("PROCEED\n"));
  return PROCEED;

#else
//-----------------------------------------------------------------------------
#ifdef DEBUG_TELLCONSTRAINTS
  cout << "tellBasicConstraint - in - : ";
  oz_print(v);
  if (fd) cout << " , " << *fd;
  cout << endl <<flush;
#endif

  DEREF(v, vptr, vtag);

  if (fd && (*fd == fd_empty))
    goto failed;


// tell finite domain constraint to unconstrained variable
  if (oz_isFree(v)) {
    if (! fd) goto fdvariable;

    // fd is singleton domain --> v becomes integer
    if (fd->getSize() == 1) {
      if (oz_isLocalVariable(vptr)) {
        if (!isUVar(vtag))
          oz_checkSuspensionListProp(tagged2SVarPlus(v));
        DoBind(vptr, newSmallInt(fd->getSingleElem()));
      } else {
        DoBindAndTrail(vptr, newSmallInt(fd->getSingleElem()));
      }
      goto proceed;
    }

    OzVariable * cv;

    // create appropriate constrained variable
    if (*fd == fd_bool) {
      cv = (OzVariable *) new OzBoolVariable(oz_currentBoard());
    } else {
    fdvariable:
      cv = (OzVariable *) fd ? new OzFDVariable(*fd,oz_currentBoard())
        : new OzFDVariable(oz_currentBoard());
    }
    OZ_Term *  tcv = newTaggedCVar(cv);

    if (oz_isLocalVariable(vptr)) {
      if (!isUVar(vtag)) {
        oz_checkSuspensionListProp(tagged2SVarPlus(v));
        cv->setSuspList(tagged2SVarPlus(v)->unlinkSuspList());
      }
      DoBind(vptr, makeTaggedRef(tcv));
    } else {
      DoBindAndTrail(vptr, makeTaggedRef(tcv));
    }

    goto proceed;
// tell finite domain constraint to finite domain variable
  } else if (isGenFDVar(v, vtag)) {
    if (! fd) goto proceed;

    OzFDVariable * fdvar = tagged2GenFDVar(v);
    OZ_FiniteDomain dom = (fdvar->getDom() & *fd);

    if (dom == fd_empty)
      goto failed;

    if (dom.getSize() == fdvar->getDom().getSize())
      goto proceed;

    if (dom == fd_singl) {
      if (oz_isLocalVar(fdvar)) {
        fdvar->getDom() = dom;
        fdvar->becomesSmallIntAndPropagate(vptr);
      } else {
        int singl = dom.getSingleElem();
        fdvar->propagate(fd_prop_singl);
        DoBindAndTrail(vptr, newSmallInt(singl));
      }
    } else if (dom == fd_bool) {
      if (oz_isLocalVar(fdvar)) {
        fdvar->becomesBoolVarAndPropagate(vptr);
      } else {
        fdvar->propagate(fd_prop_bounds);
        OzBoolVariable * newboolvar = new OzBoolVariable(oz_currentBoard());
        OZ_Term * newtaggedboolvar = newTaggedCVar(newboolvar);
        DoBindAndTrailAndIP(vptr, makeTaggedRef(newtaggedboolvar),
                            newboolvar, tagged2GenBoolVar(v));
      }
    } else {
      fdvar->propagate(fd_prop_bounds);
      if (oz_isLocalVar(fdvar)) {
        fdvar->getDom() = dom;
      } else {
        OzFDVariable * locfdvar = new OzFDVariable(dom,oz_currentBoard());
        OZ_Term * loctaggedfdvar = newTaggedCVar(locfdvar);
        DoBindAndTrailAndIP(vptr, makeTaggedRef(loctaggedfdvar),
                            locfdvar, tagged2GenFDVar(v));
      }
    }
    goto proceed;
// tell finite domain constraint to boolean finite domain variable
  } else if (isGenBoolVar(v, vtag)) {
    if (! fd) goto proceed;

    int dom = fd->intersectWithBool();

    if (dom == -2) goto failed;
    if (dom == -1) goto proceed;

    OzBoolVariable * boolvar = tagged2GenBoolVar(v);
    if (oz_isLocalVar(boolvar)) {
      boolvar->becomesSmallIntAndPropagate(vptr, dom);
    } else {
      boolvar->propagate();
      DoBindAndTrail(vptr, newSmallInt(dom));
    }
    goto proceed;
// tell finite domain constraint to integer, i.e. check for compatibility
  } else if (isSmallIntTag(vtag)) {
    if (! fd) goto proceed;

    if (fd->isIn(smallIntValue(v)))
      goto proceed;
  } else if (oz_isVariable(v)) {
    TaggedRef newVar = oz_newVariable();
    OZ_Return ret = tellBasicConstraint(newVar, fd);
    Assert(ret == PROCEED);
    return oz_unify(makeTaggedRef(vptr), newVar);
  }

failed:

  return FAILED;

proceed:

#ifdef DEBUG_TELLCONSTRAINTS
  cout << "tellBasicConstraint - out - : ";
  if (vptr) oz_print(*vptr); else oz_print(v);
  if (fd) cout << " , " << *fd;
  cout << endl <<flush;
#endif

  return PROCEED;

#endif /* CORRECT_UNIFY */
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
