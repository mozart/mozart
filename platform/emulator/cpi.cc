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

static OZ_Term atom_var     = OZ_atom("var");  
static OZ_Term atom_any     = OZ_atom("any");  
static OZ_Term atom_type    = OZ_atom("type");  
static OZ_Term atom_fd      = OZ_atom("fd");  
static OZ_Term atom_fs      = OZ_atom("fs");  
static OZ_Term atom_bool    = OZ_atom("bool");  
static OZ_Term atom_bounds  = OZ_atom("bounds");  
static OZ_Term atom_val     = OZ_atom("val");  
static OZ_Term atom_glb     = OZ_atom("glb");  
static OZ_Term atom_lub     = OZ_atom("lub");  
static OZ_Term atom_flat    = OZ_atom("flat actor");  
static OZ_Term atom_local   = OZ_atom("home");  
static OZ_Term atom_ask     = OZ_atom("ask actor");  
static OZ_Term atom_wait    = OZ_atom("wait actor");  
static OZ_Term atom_waittop = OZ_atom("waittop actor");  
static OZ_Term atom_oops    = OZ_atom("oops");  
static OZ_Term atom_prop    = OZ_atom("propagator");  
static OZ_Term atom_params  = OZ_atom("params");  
static OZ_Term atom_name    = OZ_atom("name");  
static OZ_Term atom_space   = OZ_atom("space");  
static OZ_Term atom_susp    = OZ_atom("suspension");  
static OZ_Term atom_thread  = OZ_atom("thread");  
  
#define MKARITY(Arity, ArityDef)			\
OZ_Term Arity = OZ_nil();				\
for (int i = 0; ArityDef[i] != (OZ_Term) 0; i += 1)	\
  Arity = OZ_cons(ArityDef[i], Arity);


OZ_Term getConstraintsList(SuspList * sl) 
{
  OZ_Term cl = OZ_nil();

  for (SuspList * p = sl; p != NULL; p = p->getNext()) {
    Suspension susp = p->getSuspension();
    
    OZ_Term r = atom_oops;

    if (susp.isPropagator()) {
      OZ_Propagator * p = susp.getPropagator()->getPropagator();
      Board * b = GETBOARD(susp.getPropagator());

      OZ_Term space = atom_oops;
      if (b == am.currentBoard()) {
	space = atom_local;
      } else if (b->isAsk()) {
	space = atom_ask;
      } else if (b->isWait()) {
	space = atom_wait;
      } else if (b->isWaitTop()) {
	space = atom_waittop;
      }

      OZ_Term arity_def[] = {
	{OZ_pair2(atom_type, atom_prop)},
	{OZ_pair2(atom_params, p->getParameters())},
	{OZ_pair2(atom_name, OZ_atom(p->getProfile()->getPropagatorName()))},
	{OZ_pair2(atom_space, space)},
	{(OZ_Term) 0}
      };
      
      MKARITY(arity, arity_def);
      
      r = OZ_recordInit(atom_susp, arity);
    } else {
      Board * b = GETBOARD(susp.getThread());
      OZ_Term space = atom_oops;
      
      if (b == am.currentBoard()) {
	space = atom_flat;
      } else if (b->isAsk()) {
	space = atom_ask;
      } else if (b->isWait()) {
	space = atom_wait;
      } else if (b->isWaitTop()) {
	space = atom_waittop;
      }
 
      OZ_Term arity_def[] = {
	{OZ_pair2(atom_type, atom_thread)},
	{OZ_pair2(atom_space, space)},
	{(OZ_Term) 0}
      };
      
      MKARITY(arity, arity_def);
      
      r = OZ_recordInit(atom_susp, arity);

    }

    cl = OZ_cons(r, cl);
  }

  return cl;
}

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
      {OZ_pair2(atom_var,  var_itself)},
      {OZ_pair2(atom_type, atom_any)},      
      {OZ_pair2(atom_any, (isSVar(vartag) ? 
			   getConstraintsList(tagged2SVar(var)->getSuspList()) 
			   : OZ_nil()))},      
      {(OZ_Term) 0}
    };
    
    MKARITY(arity, arity_def);

    r = OZ_recordInit(atom_var, arity);
  } else if (isGenFDVar(var,vartag)) {
    OZ_Term arity_def[] = {
      {OZ_pair2(atom_var,  var_itself)},
      {OZ_pair2(atom_type, atom_fd)},      
      {OZ_pair2(atom_any, 
		getConstraintsList(tagged2CVar(var)->getSuspList()))},
      {OZ_pair2(atom_bounds, 
		getConstraintsList(tagged2GenFDVar(var)->getSuspList(fd_prop_bounds)))},
      {OZ_pair2(atom_val, 
		getConstraintsList(tagged2GenFDVar(var)->getSuspList(fd_prop_singl)))},
      {(OZ_Term) 0}
    };
    
    MKARITY(arity, arity_def);

    r = OZ_recordInit(atom_var, arity);
  } else if (isGenBoolVar(var,vartag)) {
    OZ_Term arity_def[] = {
      {OZ_pair2(atom_var,  var_itself)},
      {OZ_pair2(atom_type, atom_bool)},      
      {OZ_pair2(atom_any, 
		getConstraintsList(tagged2CVar(var)->getSuspList()))},
      {(OZ_Term) 0}
    };
    
    MKARITY(arity, arity_def);

    r = OZ_recordInit(atom_var, arity);
  } else if (isGenFSetVar(var,vartag)) {
    OZ_Term arity_def[] = {
      {OZ_pair2(atom_var,  var_itself)},
      {OZ_pair2(atom_type, atom_fs)},     
      {OZ_pair2(atom_any, 
		getConstraintsList(tagged2CVar(var)->getSuspList()))},
      {OZ_pair2(atom_glb, 
		getConstraintsList(tagged2GenFSetVar(var)->getSuspList(fs_prop_glb)))},
      {OZ_pair2(atom_lub, 
		getConstraintsList(tagged2GenFSetVar(var)->getSuspList(fs_prop_lub)))},
      {OZ_pair2(atom_val, 
		getConstraintsList(tagged2GenFSetVar(var)->getSuspList(fs_prop_val)))},
      {(OZ_Term) 0}
    };
    
    MKARITY(arity, arity_def);

    r = OZ_recordInit(atom_var, arity);
  } 

#ifdef DEBUG_CHECK
  printf("out\n"); fflush(stdout);
#endif

  return OZ_unify(r, OZ_getCArg(1));
}
OZ_C_proc_end     

// End of File
//-----------------------------------------------------------------------------
