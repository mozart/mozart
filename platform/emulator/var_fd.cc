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

#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_fd.hh"
#endif

#include "var_fd.hh"
#include "var_bool.hh"
#include "fdomn.hh"
#include "am.hh"
#include "unify.hh"

OZ_Return OzFDVariable::bind(TaggedRef * vPtr, TaggedRef term, ByteCode *scp)
{
  Assert(!oz_isRef(term));
  if (!oz_isSmallInt(term)) return FAILED;
  if (! finiteDomain.isIn(OZ_intToC(term))) {
    return FAILED;
  }

  Bool isLocalVar = oz_isLocalVar(this);
  Bool isNotInstallingScript = !am.isInstallingScript();

#ifdef SCRIPTDEBUG
  printf("fd-int %s\n", isLocalVar ? "local" : "global"); fflush(stdout);
#endif

  if (scp==0 && (isNotInstallingScript || isLocalVar))
    propagate(fd_prop_singl);

  if (isLocalVar) {
    DoBind(vPtr, term);
    dispose();
  } else {
    DoBindAndTrail(vPtr, term);
  }

  return PROCEED;
}

// unify expects either two OzFDVariables or at least one
// OzFDVariable and one non-variable
// invariant: left term (ie var)  == *this
// Only if a local variable is bound relink its suspension list, since
// global variables are trailed.(ie. their suspension lists are
// implicitely relinked.)
OZ_Return OzFDVariable::unify(TaggedRef * vPtr, TaggedRef *tPtr, ByteCode *scp)
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
        TaggedRef int_var = OZ_int(intsct.getSingleElem());
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
          TaggedRef int_var = OZ_int(intsct.getSingleElem());
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
          TaggedRef int_term = OZ_int(intsct.getSingleElem());
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
        TaggedRef int_val = OZ_int(intsct.getSingleElem());
        if (scp==0) {
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
          if (scp==0) {
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
          if (scp==0) {
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
  default:
    OZ_error("unexpected case in FD::unify");
    break;
  } // switch (varIsLocal + 2 * termIsLocal) {
  return TRUE;
} // OzFDVariable::unify

Bool OzFDVariable::valid(TaggedRef val)
{
  Assert(!oz_isRef(val));
  return (oz_isSmallInt(val) && finiteDomain.isIn(OZ_intToC(val)));
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
        DoBind(vptr, OZ_int(fd->getSingleElem()));
      } else {
        DoBindAndTrail(vptr, OZ_int(fd->getSingleElem()));
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
        DoBindAndTrail(vptr, OZ_int(singl));
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
      DoBindAndTrail(vptr, OZ_int(dom));
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
}

// inline DISABLED CS
void OzFDVariable::propagate(OZ_FDPropState state,
                              PropCaller prop_eq)
{
  if (prop_eq == pc_propagator) {
    switch (state) {
    case fd_prop_singl: // no break
      if (fdSuspList[fd_prop_singl])
        OzVariable::propagate(fdSuspList[fd_prop_singl], prop_eq);
    case fd_prop_bounds: // no break
      if (fdSuspList[fd_prop_bounds])
        OzVariable::propagate(fdSuspList[fd_prop_bounds], prop_eq);
    default:
      break;
    }
  } else {
    OzVariable::propagate(fdSuspList[fd_prop_singl], prop_eq);
    OzVariable::propagate(fdSuspList[fd_prop_bounds], prop_eq);
  }
  if (suspList)
    OzVariable::propagate(suspList, prop_eq);
}


#ifdef OUTLINE
#define inline
#include "var_fd.icc"
#undef inline
#endif
