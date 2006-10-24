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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "reflect_space.hh"

//-----------------------------------------------------------------------------

void _reflect_space_params(ReflectStack &rec_stack, 
			   VarTable     &vtable, 
			   OZ_Term      params,
			   OZ_Term      &term_params)
{
  DEBUGPRINT(("_reflect_space_params (in)"));

  DEREF(params, paramsptr);
  
  if (oz_isVar(params)) {

    Bool is_reflected;
    int id = vtable.add(paramsptr, is_reflected);
    if (!is_reflected) 
      rec_stack.push(paramsptr);

    ADD_TO_LIST(term_params, OZ_int(id));

  } else if (OZ_isLiteral(params)) {
    
  } else if (OZ_isCons(params)) {

    int sz = OZ_length(params);

    for (int i = 0; OZ_isCons(params); params = OZ_tail(params)) 
      _reflect_space_params(rec_stack, 
			    vtable, 
			    OZ_head(params), 
			    term_params);

  } else if (OZ_isTuple(params)) {

    int sz = OZ_width(params);

    for (int i = 0; i < sz; i += 1) 
      _reflect_space_params(rec_stack, 
			    vtable, 
			    OZ_getArg(params, i), 
			    term_params);

  } else if (OZ_isRecord(params)) {

    OZ_Term al = OZ_arityList(params);
    int sz = OZ_width(params);

    for (int i = 0; OZ_isCons(al); al = OZ_tail(al)) 
      _reflect_space_params(rec_stack, 
			    vtable, 
			    OZ_subtree(params, OZ_head(al)), 
			    term_params);
  } 
  DEBUGPRINT(("_reflect_space_params (out)"));
}

OZ_Term reflect_space_params(ReflectStack &rec_stack, 
			     VarTable     &vtable, 
			     OZ_Term      params)
{
  DEBUGPRINT(("reflect_space_params (in)"));

  OZ_Term term_params = OZ_nil();

  _reflect_space_params(rec_stack, vtable, params, term_params);
  
  DEBUGPRINT(("reflect_space_params (out)"));

  return term_params;
}
    
//-----------------------------------------------------------------------------

// returns an integer (ident of propagator)
OZ_Term reflect_space_prop(ReflectStack &rec_stack, 
			   OZ_Term      &prop_list, 
			   VarTable     &vtable, 
			   PropTable    &ptable, 
			   Propagator   * prop)
{
  DEBUGPRINT(("reflect_space_prop (in)"));

  Bool is_reflected;
  DEBUGPRINT(("ptable.add(%p)", prop));
  int id = ptable.add(prop, is_reflected);
  OZ_Term term_id = OZ_int(id); 
  if (is_reflected) {
    DEBUGPRINT(("reflect_space_prop (out)"));
    return OZ_int(id);
  }
  ptable.reflected(prop);
    
  OZ_Propagator * p = prop->getPropagator();
  
  OZ_Term arity_def[] = {
    OZ_pair2(atom_id,     term_id),        
    OZ_pair2(atom_ref,    propagator2Term(prop)),
    OZ_pair2(atom_params, reflect_space_params(rec_stack,
					       vtable,
					       p->getParameters())),
    OZ_pair2(atom_name,   prop_name(p->getProfile()->getPropagatorName())),
    OZ_pair2(atom_loc,    oz_propGetName(prop)),
    (OZ_Term) 0
  };
  
  MKARITY(arity, arity_def);
  
  ADD_TO_LIST(prop_list, OZ_pair2(term_id, 
				  OZ_recordInit(atom_prop, arity)));

  DEBUGPRINT(("reflect_space_prop (out)"));

  return oz_int(id);
}

//-----------------------------------------------------------------------------

// returns a list of integers (idents of propagators)
OZ_Term reflect_space_susplist(ReflectStack &rec_stack,
			       PropTable    &ptable, 
			       SuspList     * susplist)
{
  DEBUGPRINT(("reflect_space_susplist (in)"));

  OZ_Term term_props = OZ_nil();
  
  for (SuspList * p = susplist; p != NULL; p = p->getNext()) {
    Suspendable * susp = p->getSuspendable();
    
    if (susp->isPropagator()) {
      Propagator * prop = SuspToPropagator(susp);
      
      if (!prop->isDead()) {
	Bool is_reflected;
	DEBUGPRINT(("ptable.add(%p)", prop));
	int id = ptable.add(prop, is_reflected);
	
	ADD_TO_LIST(term_props, OZ_int(id));
	
	if (!is_reflected) { 
	  rec_stack.push(prop);
	}
      }
    }
  }
  
  DEBUGPRINT(("reflect_space_susplist (out)"));

  return term_props;
}

//-----------------------------------------------------------------------------

// returns an integer (ident of variable)
OZ_Term reflect_space_variable(ReflectStack &rec_stack,
			       OZ_Term      &var_list, 
			       VarTable     &vtable, 
			       PropTable    &ptable, 
			       OZ_Term      var)
{
  DEBUGPRINT(("reflect_space_variable (in)"));

  OZ_Term var_itself = var;
  DEREF(var, varptr);
  
  OZ_Term term_id       = (OZ_Term) 0;
  OZ_Term term_susplist = (OZ_Term) 0;
  OZ_Term term_type     = OZ_nil();

  if (oz_isFree(var)) {
    DEBUGPRINT(("reflect_space_variable (free)"));
    
    Bool is_reflected;
    int id = vtable.add(varptr, is_reflected);
    term_id = OZ_int(id); 
    if (is_reflected) {
      DEBUGPRINT(("reflect_space_variable (out -1-)"));
      return term_id;
    }
    vtable.reflected(varptr);

    term_type = atom_any;

    OZ_Term susp_arity_def[] = {
      OZ_pair2(atom_any, 
	       (oz_isVar(var) ? 
		reflect_space_susplist(rec_stack, ptable, 
				       tagged2Var(var)->getSuspList()) 
		: OZ_nil())),      
      (OZ_Term) 0
    };
    
    MKARITY(susp_arity, susp_arity_def);

    term_susplist = OZ_recordInit(atom_susplists, susp_arity);

  } else if (isGenFDVar(var)) {
    DEBUGPRINT(("reflect_space_variable (a)"));

    Bool is_reflected;
    int id = vtable.add(varptr, is_reflected);
    term_id = OZ_int(id); 
    if (is_reflected) {
      DEBUGPRINT(("reflect_space_variable (out -2-)"));
      return term_id;
    }
    DEBUGPRINT(("reflect_space_variable (b)"));
    vtable.reflected(varptr);
    DEBUGPRINT(("reflect_space_variable (c)"));

    term_type = atom_fd;

    OZ_Term susp_arity_def[] = {
      OZ_pair2(atom_any, 
	       reflect_space_susplist(rec_stack, ptable, 
				      tagged2Var(var)->
				      getSuspList())),
      OZ_pair2(atom_bounds, 
	       reflect_space_susplist(rec_stack, ptable, 
				      tagged2GenFDVar(var)->
				      getSuspList(fd_prop_bounds))),
      OZ_pair2(atom_val, 
	       reflect_space_susplist(rec_stack, ptable, 
				      tagged2GenFDVar(var)->
				      getSuspList(fd_prop_singl))),
      (OZ_Term) 0
    };
    DEBUGPRINT(("reflect_space_variable (d)"));

    MKARITY(susp_arity, susp_arity_def);
    DEBUGPRINT(("reflect_space_variable (e)"));

    term_susplist = OZ_recordInit(atom_susplists, susp_arity);
    DEBUGPRINT(("reflect_space_variable (f)"));

  } else if (isGenBoolVar(var)) {
    DEBUGPRINT(("reflect_space_variable (bool)"));

    Bool is_reflected;
    int id = vtable.add(varptr, is_reflected);
    term_id = OZ_int(id); 
    if (is_reflected) {
      DEBUGPRINT(("reflect_space_variable (out -3-)"));
      return term_id;
    }
    vtable.reflected(varptr);

    term_type = atom_bool;
    
    OZ_Term susp_arity_def[] = {
      OZ_pair2(atom_any, 
	       reflect_space_susplist(rec_stack, ptable, 
				      tagged2Var(var)->getSuspList())),
      (OZ_Term) 0
    };
    
    MKARITY(susp_arity, susp_arity_def);

    term_susplist = OZ_recordInit(atom_susplists, susp_arity);

  } else if (isGenFSetVar(var)) {
    DEBUGPRINT(("reflect_space_variable (fs)"));

    Bool is_reflected;
    int id = vtable.add(varptr, is_reflected);
    term_id = OZ_int(id); 
    if (is_reflected) {
      DEBUGPRINT(("reflect_space_variable (out -4-)"));
      return term_id;
    }
    vtable.reflected(varptr);

    term_type = atom_fs;

    OZ_Term susp_arity_def[] = {
      OZ_pair2(atom_any, 
	       reflect_space_susplist(rec_stack, ptable, 
				      tagged2Var(var)->getSuspList())),
      OZ_pair2(atom_glb, 
	       reflect_space_susplist(rec_stack, ptable, 
				      tagged2GenFSetVar(var)->
				      getSuspList(fs_prop_glb))),
      OZ_pair2(atom_lub, 
	       reflect_space_susplist(rec_stack, ptable, 
				      tagged2GenFSetVar(var)->
				      getSuspList(fs_prop_lub))),
      OZ_pair2(atom_val, 
	       reflect_space_susplist(rec_stack, ptable, 
				      tagged2GenFSetVar(var)->
				      getSuspList(fs_prop_val))),
      (OZ_Term) 0
    };
    
    MKARITY(susp_arity, susp_arity_def);

    term_susplist = OZ_recordInit(atom_susplists, susp_arity);

  } else if (isGenCtVar(var)) {
    DEBUGPRINT(("reflect_space_variable (ct)"));

    Bool is_reflected;
    int id = vtable.add(varptr, is_reflected);
    term_id = OZ_int(id); 
    if (is_reflected) {
      DEBUGPRINT(("reflect_space_variable (out -5-)"));
      return term_id;
    }
    vtable.reflected(varptr);

    term_type = atom_ct;

    OzCtVariable * v         = tagged2GenCtVar(var);
    OZ_CtDefinition * def    = v->getDefinition();
    int numOfSuspLists       = def->getNoEvents();
    char ** namesOfSuspLists = def->getEventNames();
    const int ind_offset     = 2;
    OZ_Term susp_arity_def[ind_offset + numOfSuspLists];

    susp_arity_def[0] 
      = OZ_pair2(atom_any, 
		 reflect_space_susplist(rec_stack, ptable, 
					tagged2Var(var)->getSuspList()));

    susp_arity_def[numOfSuspLists + ind_offset - 1] = (OZ_Term) 0;
   
    for (int i = numOfSuspLists; i--; )
      susp_arity_def[i + ind_offset - 1] = 
	OZ_pair2(OZ_atom(namesOfSuspLists[i]), 
		 reflect_space_susplist(rec_stack, ptable, v->getSuspList(i)));
    
    MKARITY(susp_arity, susp_arity_def);
    
    term_susplist = OZ_recordInit(atom_susplists, susp_arity);
  } 
  
  if (term_type != OZ_nil()) { // it is a variable
    OZ_Term arity_def[] = {
      OZ_pair2(atom_ref,       var_itself),
      OZ_pair2(atom_type,      term_type),      
      OZ_pair2(atom_id,        term_id),      
      OZ_pair2(atom_susplists, term_susplist),
      OZ_pair2(atom_name,      OZ_atom(oz_varGetName(var_itself))),
      (OZ_Term) 0
    };
    
    MKARITY(arity, arity_def);

    ADD_TO_LIST(var_list, OZ_pair2(term_id,
				   OZ_recordInit(atom_var, arity)));

    DEBUGPRINT(("reflect_variable (out -- adding reflected var)\n"));  

    return term_id;
  }
  
  DEBUGPRINT(("reflect_variable (out -- not adding reflected var)\n"));

  return OZ_unit();
}

//-----------------------------------------------------------------------------

OZ_Term reflect_space(OZ_Term v)
{
  OZ_Term prop_list = OZ_nil();
  OZ_Term var_list  = OZ_nil();

  VarTable     vtable;
  PropTable    ptable;
  ReflectStack rec_stack;

  //  (void) reflect_space_variable(rec_stack, var_list, vtable, ptable, v);

  (void) reflect_space_params(rec_stack, vtable, v);

  while (!rec_stack.isEmpty()) {
    OZ_Term se = (OZ_Term) rec_stack.pop();
    void * ptr = __unstag_ptr(void*,se,__tagged2stag(se));
    TypeOfReflStackEntry what = (TypeOfReflStackEntry) __tagged2stag(se);
    
    switch (what) {
    case Entry_Propagator:
      DEBUGPRINT(("reflect_space -- switch entry propagator\n"));
      {
	Propagator * prop = (Propagator *) ptr; 

	DEBUG_ASSERT (!prop->isDead());

	(void) reflect_space_prop(rec_stack, 
				  prop_list, 
				  vtable, 
				  ptable, 
				  prop);
      }
      break;

    case Entry_Variable:
      DEBUGPRINT(("reflect_space -- switch entry variable\n"));
      {
	OZ_Term * varptr = (OZ_Term *) ptr;
	(void) reflect_space_variable(rec_stack, 
				      var_list, 
				      vtable, 
				      ptable, 
				      (OZ_Term) varptr);
      }
      break;

    default:
      break;
    }
  } // while

  _DEBUGPRINT(("Reflecting Space done.\n"));

  OZ_Term arity_def[] = {
    OZ_pair2(atom_vars, OZ_recordInit(atom_reflect_vartable, var_list)),
    OZ_pair2(atom_props, OZ_recordInit(atom_reflect_proptable, prop_list)),
    (OZ_Term) 0
  };

  MKARITY(term_arity, arity_def);

  return OZ_recordInit(atom_reflect, term_arity);
}
