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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "cpi.hh"
#endif

#include <math.h>

#include "cpi.hh"

CpiHeapClass CpiHeap;

EnlargeableArray<_spawnVars_t> staticSpawnVars(CPIINITSIZE);
EnlargeableArray<_spawnVars_t> staticSpawnVarsProp(CPIINITSIZE);
EnlargeableArray<_spawnVars_t> staticSuspendVars(CPIINITSIZE);

int staticSpawnVarsNumber = 0;
int staticSpawnVarsNumberProp = 0;
int staticSuspendVarsNumber = 0;


#ifdef FDBISTUCK

OZ_Return constraintsSuspendOnVar(OZ_CFun, int, OZ_Term *,
                                  OZ_Term * t)
{
  OZ_suspendOn(makeTaggedRef(t));
}

#else

OZ_Return constraintsSuspendOnVar(OZ_CFun f, int a, OZ_Term * x,
                                  OZ_Term * t)
{
  OZ_addThread(makeTaggedRef(t), OZ_makeSuspendedThread(f, x, a));
  return PROCEED;
}

#endif

#if defined(OUTLINE)
#define inline
#include "cpi.icc"
#undef inline
#endif

static OZ_Term var_atom     = OZ_atom("var");
static OZ_Term any_atom     = OZ_atom("any");
static OZ_Term type_atom    = OZ_atom("type");
static OZ_Term fd_atom      = OZ_atom("fd");
static OZ_Term fs_atom      = OZ_atom("fs");
static OZ_Term bool_atom    = OZ_atom("bool");
static OZ_Term bounds_atom  = OZ_atom("bounds");
static OZ_Term val_atom     = OZ_atom("val");
static OZ_Term glb_atom     = OZ_atom("glb");
static OZ_Term lub_atom     = OZ_atom("lub");
static OZ_Term ask_atom     = OZ_atom("ask actor");
static OZ_Term wait_atom    = OZ_atom("wait actor");
static OZ_Term waittop_atom = OZ_atom("waittop actor");
static OZ_Term oops_atom    = OZ_atom("oops");

OZ_Term getConstraintsList(SuspList * sl)
{
  OZ_Term cl = OZ_nil();

  for (SuspList * p = sl; p != NULL; p = p->getNext()) {
    Suspension susp = p->getSuspension();

    OZ_Term r = oops_atom;

    if (susp.isPropagator()) {
      OZ_Propagator * p = susp.getPropagator()->getPropagator();

      const char * func_name = p->getProfile()->getPropagatorName();
      OZ_Term params = p->getParameters();

      r = OZ_pair2(OZ_atom(func_name), params);

    } else {
      Board * b = GETBOARD(susp.getThread());

      if (b->isAsk()) {
        r = ask_atom;
      } else if (b->isWait()) {
        r = wait_atom;
      } else if (b->isWaitTop()) {
        r = waittop_atom;
      }

    }

    cl = OZ_cons(r, cl);
  }

  return cl;
}

#define mkArity(Arity, ArityDef)                        \
OZ_Term Arity = OZ_nil();                               \
for (int i = 0; ArityDef[i] != (OZ_Term) 0; i += 1)     \
  Arity = OZ_cons(ArityDef[i], Arity);


OZ_C_proc_begin(BIgetConstraints, 2)
{
#ifdef DEBUG_CHECK
  printf("in\n"); fflush(stdout);
#endif

  OZ_getCArgDeref(0, var, varptr, vartag);

  OZ_Term var_itself  = OZ_getCArg(0);

  OZ_Term r = OZ_nil();

  if (isNotCVar(vartag)) {
    OZ_Term arity_def[] = {
      {OZ_pair2(var_atom,  var_itself)},
      {OZ_pair2(type_atom, any_atom)},
      {(OZ_Term) 0}
    };

    mkArity(arity, arity_def);

    if (isSVar(vartag)) {
      arity = OZ_cons(OZ_pair2(any_atom,
                               getConstraintsList(tagged2SVar(var)->getSuspList())), arity);
    }

    r = OZ_recordInit(var_atom, arity);
  } else if (isGenFDVar(var,vartag)) {
    OZ_Term arity_def[] = {
      {OZ_pair2(var_atom,  var_itself)},
      {OZ_pair2(type_atom, fd_atom)},
      {OZ_pair2(any_atom,
                getConstraintsList(tagged2CVar(var)->getSuspList()))},
      {OZ_pair2(bounds_atom,
                getConstraintsList(tagged2GenFDVar(var)->getSuspList(fd_prop_bounds)))},
      {OZ_pair2(val_atom,
                getConstraintsList(tagged2GenFDVar(var)->getSuspList(fd_prop_singl)))},
      {(OZ_Term) 0}
    };

    mkArity(arity, arity_def);

    r = OZ_recordInit(var_atom, arity);
  } else if (isGenBoolVar(var,vartag)) {
    OZ_Term arity_def[] = {
      {OZ_pair2(var_atom,  var_itself)},
      {OZ_pair2(type_atom, bool_atom)},
      {OZ_pair2(any_atom,
                getConstraintsList(tagged2CVar(var)->getSuspList()))},
      {(OZ_Term) 0}
    };

    mkArity(arity, arity_def);

    r = OZ_recordInit(var_atom, arity);
  } else if (isGenFSetVar(var,vartag)) {
    OZ_Term arity_def[] = {
      {OZ_pair2(var_atom,  var_itself)},
      {OZ_pair2(type_atom, fs_atom)},
      {OZ_pair2(any_atom,
                getConstraintsList(tagged2CVar(var)->getSuspList()))},
      {OZ_pair2(glb_atom,
                getConstraintsList(tagged2GenFSetVar(var)->getSuspList(fs_prop_glb)))},
      {OZ_pair2(lub_atom,
                getConstraintsList(tagged2GenFSetVar(var)->getSuspList(fs_prop_lub)))},
      {OZ_pair2(val_atom,
                getConstraintsList(tagged2GenFSetVar(var)->getSuspList(fs_prop_val)))},
      {(OZ_Term) 0}
    };

    mkArity(arity, arity_def);

    r = OZ_recordInit(var_atom, arity);
  }

#ifdef DEBUG_CHECK
  printf("out\n"); fflush(stdout);
#endif

  return OZ_unify(r, OZ_getCArg(1));
}
OZ_C_proc_end

// End of File
//-----------------------------------------------------------------------------
