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
#pragma implementation "var_ct.hh"
#endif

#include "var_ct.hh"
#include "am.hh"
#include "builtins.hh"
#include "unify.hh"

OZ_Return OzCtVariable::bind(OZ_Term * vptr, OZ_Term term, ByteCode * scp)
{
  // bind temporarily to avoid looping in unification on cyclic terms
  OZ_Term trail = *vptr;
  *vptr = term;

  OZ_Boolean result_unify = _constraint->unify(term);

  // undo binding
  *vptr = trail;

  if (! result_unify)
    return FAILED;

  {
    Bool isLocalVar = oz_isLocalVar(this);
    Bool isNotInstallingScript = !am.isInstallingScript();

    if (scp == 0 && (isNotInstallingScript || isLocalVar)) {
      propagateUnify();
    }
    if (isLocalVar) {
      DoBind(vptr, term);
      dispose();
    } else {
      DoBindAndTrail(vptr, term);
    }
  }
  return PROCEED;
}

// `var' and `vptr' belongs to `this', i.e., is a `OzCtVariable
// `term' and `tptr' are either values or other constrained variable
// (preferably `OzCtVariable' of the right kind. Unification is
// implemented such, that `OzCtVariable's of the sam ekind are
// compatible with each other.
OZ_Return OzCtVariable::unify(OZ_Term * vptr, OZ_Term * tptr, ByteCode * scp)
{
  Assert(vptr);

  OZ_Term term = *tptr;
  OzVariable *cv = tagged2CVar(term);
  if (cv->getType() != OZ_VAR_CT) return FAILED;

  // `t' and `tptr' refer to another `OzCtVariable'

  OzCtVariable * term_var = (OzCtVariable *)cv;
  OZ_Ct * t_constr = term_var->getConstraint();
  OZ_Ct * constr = getConstraint();

  // bind temporarily to avoid looping in unification on cyclic terms
  OZ_Term trail = *vptr;
  *vptr = term;
  OZ_Ct * new_constr = constr->unify(t_constr);

  // undo binding
  *vptr = trail;

  if (! new_constr->isValid()) return FAILED;

  Bool var_is_local  = oz_isLocalVar(this);
  Bool term_is_local = oz_isLocalVar(term_var);
  Bool is_not_installing_script = !am.isInstallingScript();
  Bool var_is_constrained = (is_not_installing_script ||
                             (constr->isWeakerThan(new_constr)));
  Bool term_is_constrained = (is_not_installing_script ||
                              (t_constr->isWeakerThan(new_constr)));

  switch (var_is_local + 2 * term_is_local) {
  case TRUE + 2 * TRUE: // var and term are local
    {
      if (new_constr->isValue()) {
        // `new_constr' designates a value
        OZ_Term new_value = new_constr->toValue();
        term_var->propagateUnify();
        propagateUnify();
        DoBind(vptr, new_value);
        DoBind(tptr, new_value);
        dispose();
        term_var->dispose();
      } else if (heapNewer(vptr, tptr)) { // bind var to term
        term_var->copyConstraint(new_constr);
        propagateUnify();
        term_var->propagateUnify();
        relinkSuspListTo(term_var);
        DoBind(vptr, makeTaggedRef(tptr));
        dispose();
      } else { // bind term to var
        copyConstraint(new_constr);
        term_var->propagateUnify();
        propagateUnify();
        term_var->relinkSuspListTo(this);
        DoBind(tptr, makeTaggedRef(vptr));
        term_var->dispose();
      }
    } // TRUE + 2 * TRUE:
    break;

  case TRUE + 2 * FALSE: // var is local and term is global
    {
      if (t_constr->isWeakerThan(new_constr)) {
        if (new_constr->isValue()) {
          // `new_constr' designates a value
          OZ_Term new_value = new_constr->toValue();
          if (is_not_installing_script)
            term_var->propagateUnify();
          if (var_is_constrained)
            propagateUnify();
          DoBind(vptr, new_value);
          DoBindAndTrail(tptr, new_value);
          dispose();
        } else {
          copyConstraint(new_constr);
          if (is_not_installing_script)
            term_var->propagateUnify();
          if (var_is_constrained)
            propagateUnify();
          DoBindAndTrailAndIP(tptr, makeTaggedRef(vptr),
                              this, term_var);
        }
      } else {
        if (is_not_installing_script)
          term_var->propagateUnify();
        if (var_is_constrained)
          propagateUnify();
        relinkSuspListTo(term_var, TRUE);
        DoBind(vptr, makeTaggedRef(tptr));
        dispose();
      }
    } // TRUE + 2 * FALSE:
    break;

  case FALSE + 2 * TRUE: // var is global and term is local
    {
      if (constr->isWeakerThan(new_constr)) {
        if(new_constr->isValue()) {
          // `new_constr' designates a value
          OZ_Term new_value = new_constr->toValue();
          if (is_not_installing_script)
            propagateUnify();
          if (term_is_constrained)
            term_var->propagateUnify();
          DoBind(tptr, new_value);
          DoBindAndTrail(vptr, new_value);
          term_var->dispose();
        } else {
          term_var->copyConstraint(new_constr);
          if (is_not_installing_script)
            propagateUnify();
          if (term_is_constrained)
            term_var->propagateUnify();
          DoBindAndTrailAndIP(vptr, makeTaggedRef(tptr),
                              term_var, this);
        }
      } else {
        if (term_is_constrained)
          term_var->propagateUnify();
        if (is_not_installing_script)
          propagateUnify();
        term_var->relinkSuspListTo(this, TRUE);
        DoBind(tptr, makeTaggedRef(vptr));
        term_var->dispose();
      }
    } // FALSE + 2 * TRUE:
    break;

  case FALSE + 2 * FALSE: // var and term is global
    {
      if (new_constr->isValue()){
        // `new_constr' designates a value
        OZ_Term new_value = new_constr->toValue();
        if (scp == 0) {
          if (var_is_constrained)
            propagateUnify();
          if (term_is_constrained)
            term_var->propagateUnify();
        }
        DoBindAndTrail(vptr, new_value);
        DoBindAndTrail(tptr, new_value);
      } else {
        OzCtVariable * cv = new OzCtVariable(new_constr, getDefinition(),
                                             oz_currentBoard());
        OZ_Term * cvar = newTaggedCVar(cv);
        if (scp == 0) {
          if (var_is_constrained)
            propagateUnify();
          if (term_is_constrained)
            term_var->propagateUnify();
        }
        DoBindAndTrailAndIP(vptr, makeTaggedRef(cvar),
                            cv, this);
        DoBindAndTrailAndIP(tptr, makeTaggedRef(cvar),
                            cv, term_var);
      }
    } // FALSE + 2 * FALSE:
    break;

  default:
    OZ_error("unexpected case in unifyCt");
    break;
  } // switch (varIsLocal + 2 * termIsLocal)
  return PROCEED;
}

OZ_Return tellBasicConstraint(OZ_Term v, OZ_Ct * constr, OZ_CtDefinition * def)
{
  DEREF(v, vptr, vtag);


  // a constraint has to be valid
  if (constr && ! constr->isValid())
    goto failed;

  // tell constraint to unconstrained variable
  if (oz_isFree(v)) {
    if (! constr) goto ctvariable;

    // constr denotes a value --> v becomes value
    if (constr->isValue()) {
      if (oz_isLocalVariable(vptr)) {
        if (!isUVar(vtag))
          oz_checkSuspensionListProp(tagged2SVarPlus(v));
        DoBind(vptr, constr->toValue());
      } else {
        DoBindAndTrail(vptr, constr->toValue());
      }
      goto proceed;
    } else {
    ctvariable:
      OzCtVariable * ctv =
        constr
        ? new OzCtVariable(constr, def, oz_currentBoard())
        :  new OzCtVariable(def->leastConstraint(), def, oz_currentBoard());

      OZ_Term *  tctv = newTaggedCVar(ctv);

      if (oz_isLocalVariable(vptr)) {
        if (!isUVar(vtag)) {
          oz_checkSuspensionListProp(tagged2SVarPlus(v));
          ctv->setSuspList(tagged2SVarPlus(v)->unlinkSuspList());
        }
        DoBind(vptr, makeTaggedRef(tctv));
      } else {
        DoBindAndTrail(vptr, makeTaggedRef(tctv));
      }
      goto proceed;
    }
  } else if (isGenCtVar(v, vtag)) {
    // tell constraint to constrained variable
    if (! constr) goto proceed;

    OzCtVariable * ctvar = tagged2GenCtVar(v);
    OZ_Ct * old_constr = ctvar->getConstraint();
    OZ_CtProfile * old_constr_prof = old_constr->getProfile();
    OZ_Ct * new_constr = old_constr->unify(constr);

    if (! new_constr->isValid())
      goto failed;

    if (! ctvar->getConstraint()->isWeakerThan(new_constr))
      goto proceed;

    if (new_constr->isValue()) {
      // `new_constr' designates a value

      ctvar->propagate(OZ_WAKEUP_ALL, pc_propagator);

      if (oz_isLocalVar(ctvar)) {
        DoBind(vptr, new_constr->toValue());
      } else {
        DoBindAndTrail(vptr, new_constr->toValue());
      }
    } else {
      // `new_constr' does not designate a value
      ctvar->propagate(new_constr->getWakeUpDescriptor(old_constr_prof),
                       pc_propagator);

      if (oz_isLocalVar(ctvar)) {
        ctvar->copyConstraint(new_constr);
      } else {
        OzCtVariable * locctvar = new OzCtVariable(new_constr, def,
                                                     oz_currentBoard());
        OZ_Term * loctaggedctvar = newTaggedCVar(locctvar);
        DoBindAndTrailAndIP(vptr, makeTaggedRef(loctaggedctvar),
                            locctvar, ctvar);
      }
    }
    goto proceed;
  } else if (! oz_isVariable(vtag)) {
    // trying to constrain a value, i.e., check if constraint is
    // consistent with value
    if (! constr) goto proceed;

    if (constr->unify(v))
      goto proceed;
    goto failed;
  } else {
    Assert(oz_isVariable(v));
    TaggedRef newVar = oz_newVariable();
    OZ_Return ret = tellBasicConstraint(newVar, constr, def);
    Assert(ret == PROCEED);
    return oz_unify(makeTaggedRef(vptr), newVar);
  }

failed:
  return FAILED;

proceed:
  return PROCEED;
}


void OzCtVariable::propagate(OZ_CtWakeUp descr, PropCaller caller)
{
  int no_of_wakup_lists = _definition->getNoOfWakeUpLists();

  if (caller == pc_propagator) {
    // called by propagator
    for (int i = no_of_wakup_lists; i--; )
      if (descr.isWakeUp(i) && _susp_lists[i])
        OzVariable::propagate(_susp_lists[i], caller);
  } else {
    // called by unification
    for (int i = no_of_wakup_lists; i--; )
      if (_susp_lists[i])
        OzVariable::propagate(_susp_lists[i], caller);
  }
  if (suspList)
    OzVariable::propagate(suspList, caller);
}

void OzCtVariable::relinkSuspListTo(OzCtVariable * lv, Bool reset_local)
{
  OzVariable::relinkSuspListTo(lv, reset_local); // any

  for (int i = _definition->getNoOfWakeUpLists(); i--; )
    _susp_lists[i] =
      _susp_lists[i]->appendToAndUnlink(lv->_susp_lists[i], reset_local);
}

void OzCtVariable::installPropagators(OzCtVariable * glob_var)
{
  installPropagatorsG(glob_var);
  for (int i = _definition->getNoOfWakeUpLists(); i--; )
    _susp_lists[i] = oz_installPropagators(_susp_lists[i],
                                          glob_var->_susp_lists[i],
                                          GETBOARD(glob_var));
}

//-----------------------------------------------------------------------------
// built-ins

OZ_BI_define(BIIsGenCtVarB, 1,1)
{
  OZ_getINDeref(0, v, vptr, vtag);

  OZ_RETURN(oz_bool(isGenCtVar(v, vtag)));
} OZ_BI_end

OZ_C_proc_begin(BIGetCtVarConstraintAsAtom, 2)
{
  ExpectedTypes("OzCtVariable<ConstraintData>,Atom");

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(! oz_isVariable(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenCtVar(var, vartag)) {
    return OZ_unify(oz_atom(((OzCtVariable *) tagged2CVar(var))->getConstraint()->toString(ozconf.printDepth)),
                    OZ_getCArg(1));
  } else if (oz_isNonKinded(var)) {
    OZ_addThread(makeTaggedRef(varptr),
                 OZ_makeSuspendedThread(OZ_self, OZ_args, OZ_arity));
    return PROCEED;
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end

OZ_C_proc_begin(BIGetCtVarNameAsAtom, 2)
{
  ExpectedTypes("OzCtVariable<ConstraintData>,Atom");

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(! oz_isVariable(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenCtVar(var, vartag)) {
    return
      OZ_unify(oz_atom(((OzCtVariable*)tagged2CVar(var))->getDefinition()->getName()),
               OZ_getCArg(1));
  } else if (oz_isNonKinded(var)) {
    OZ_addThread(makeTaggedRef(varptr),
                 OZ_makeSuspendedThread(OZ_self, OZ_args, OZ_arity));
    return PROCEED;
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end



#ifdef OUTLINE
#define inline
#include "var_ct.icc"
#undef inline
#endif
