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

// returns an integer (ident of propagator)
OZ_Term reflect_space_prop(ReflectStack &rec_stack,
                           OZ_Term      &prop_list,
                           VarTable     &vtable,
                           PropTable    &ptable,
                           Propagator   * prop)
{
  Bool is_reflected;
  int id = ptable.add(prop, is_reflected);
  if (is_reflected)
    return OZ_int(id);
  ptable.reflected(prop);


  OZ_Propagator * p = prop->getPropagator();

  OZ_Term arity_def[] = {
    {OZ_pair2(atom_ref,    propagator2Term(prop))},
    {OZ_pair2(atom_params, p->getParameters())},
    {OZ_pair2(atom_name,   prop_name(p->getProfile()->getPropagatorName()))},
    {OZ_pair2(atom_loc,    oz_propGetName(prop))},
    {(OZ_Term) 0}
  };

  MKARITY(arity, arity_def);

  ADD_TO_LIST(prop_list, OZ_recordInit(atom_prop, arity));

  return oz_int(id);
}

//-----------------------------------------------------------------------------

// returns a list of integers (idents of propagators)
OZ_Term reflect_space_susplist(ReflectStack &rec_stack,
                               PropTable    &ptable,
                               SuspList     * susplist)
{
  OZ_Term term_props = OZ_nil();

  for (SuspList * p = susplist; p != NULL; p = p->getNext()) {
    Suspension susp = p->getSuspension();

    if (susp.isPropagator()) {
      Propagator * prop = susp.getPropagator();

      Bool is_reflected;
      int id = ptable.add(prop, is_reflected);

      ADD_TO_LIST(term_props, OZ_int(id));
      if (!is_reflected) {
        rec_stack.push(prop);
      }
    }
  }

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
  OZ_Term var_itself = var;
  DEREF(var, varptr, vartag);

  OZ_Term term_id       = (OZ_Term) 0;
  OZ_Term term_susplist = (OZ_Term) 0;
  OZ_Term term_type     = OZ_nil();

  if (oz_isFree(var)) {

    Bool is_reflected;
    int id = vtable.add(varptr, is_reflected);
    if (is_reflected)
      return OZ_int(id);
    vtable.reflected(varptr);

    term_type = atom_any;

    OZ_Term susp_arity_def[] = {
      {OZ_pair2(atom_any,
                (isCVar(vartag) ?
                 reflect_space_susplist(rec_stack, ptable,
                                        tagged2CVar(var)->getSuspList())
                 : OZ_nil()))},
      {(OZ_Term) 0}
    };

    MKARITY(susp_arity, susp_arity_def);

    term_susplist = OZ_recordInit(atom_susplists, susp_arity);
  } else if (isGenFDVar(var,vartag)) {

    Bool is_reflected;
    int id = vtable.add(varptr, is_reflected);
    if (is_reflected)
      return OZ_int(id);
    vtable.reflected(varptr);

    term_type = atom_fd;

    OZ_Term susp_arity_def[] = {
      {OZ_pair2(atom_any,
                reflect_space_susplist(rec_stack, ptable,
                                       tagged2CVar(var)->
                                       getSuspList()))},
      {OZ_pair2(atom_bounds,
                reflect_space_susplist(rec_stack, ptable,
                                       tagged2GenFDVar(var)->
                                       getSuspList(fd_prop_bounds)))},
      {OZ_pair2(atom_val,
                reflect_space_susplist(rec_stack, ptable,
                                       tagged2GenFDVar(var)->
                                       getSuspList(fd_prop_singl)))},
      {(OZ_Term) 0}
    };

    MKARITY(susp_arity, susp_arity_def);

    term_susplist = OZ_recordInit(atom_susplists, susp_arity);
  } else if (isGenBoolVar(var,vartag)) {

    Bool is_reflected;
    int id = vtable.add(varptr, is_reflected);
    if (is_reflected)
      return OZ_int(id);
    vtable.reflected(varptr);

    term_type = atom_bool;

    OZ_Term susp_arity_def[] = {
      {OZ_pair2(atom_any,
                reflect_space_susplist(rec_stack, ptable,
                                       tagged2CVar(var)->getSuspList()))},
      {(OZ_Term) 0}
    };

    MKARITY(susp_arity, susp_arity_def);

    term_susplist = OZ_recordInit(atom_susplists, susp_arity);
  } else if (isGenFSetVar(var,vartag)) {

    Bool is_reflected;
    int id = vtable.add(varptr, is_reflected);
    if (is_reflected)
      return OZ_int(id);
    vtable.reflected(varptr);

    term_type = atom_fs;

    OZ_Term susp_arity_def[] = {
      {OZ_pair2(atom_any,
                reflect_space_susplist(rec_stack, ptable,
                                       tagged2CVar(var)->getSuspList()))},
      {OZ_pair2(atom_glb,
                reflect_space_susplist(rec_stack, ptable,
                                       tagged2GenFSetVar(var)->
                                       getSuspList(fs_prop_glb)))},
      {OZ_pair2(atom_lub,
                reflect_space_susplist(rec_stack, ptable,
                                       tagged2GenFSetVar(var)->
                                       getSuspList(fs_prop_lub)))},
      {OZ_pair2(atom_val,
                reflect_space_susplist(rec_stack, ptable,
                                       tagged2GenFSetVar(var)->
                                       getSuspList(fs_prop_val)))},
      {(OZ_Term) 0}
    };

    MKARITY(susp_arity, susp_arity_def);

    term_susplist = OZ_recordInit(atom_susplists, susp_arity);
  } else if (isGenCtVar(var, vartag)) {

    Bool is_reflected;
    int id = vtable.add(varptr, is_reflected);
    if (is_reflected)
      return OZ_int(id);
    vtable.reflected(varptr);

    term_type = atom_ct;

    OzCtVariable * v         = tagged2GenCtVar(var);
    OZ_CtDefinition * def    = v->getDefinition();
    int numOfSuspLists       = def->getNoOfWakeUpLists();
    char ** namesOfSuspLists = def->getNamesOfWakeUpLists();
    const int ind_offset     = 2;
    OZ_Term susp_arity_def[ind_offset + numOfSuspLists];

    susp_arity_def[0]
      = OZ_pair2(atom_any,
                 reflect_space_susplist(rec_stack, ptable,
                                        tagged2CVar(var)->getSuspList()));

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
      {OZ_pair2(atom_var,       var_itself)},
      {OZ_pair2(atom_type,      term_type)},
      {OZ_pair2(atom_id,        term_id)},
      {OZ_pair2(atom_susplists, term_susplist)},
      {OZ_pair2(atom_name,      OZ_atom(oz_varGetName(var_itself)))},
      {(OZ_Term) 0}
    };

    MKARITY(arity, arity_def);

    ADD_TO_LIST(var_list, OZ_recordInit(atom_var, arity));

    DEBUGPRINT(("reflect_variable out (adding reflected var)\n"));
  }


  DEBUGPRINT(("reflect_variable out (not adding reflected var)\n"));

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

  (void) reflect_space_variable(rec_stack, var_list, vtable, ptable, v);

  while (!rec_stack.isEmpty()) {
    OZ_Term se = (OZ_Term) rec_stack.pop();
    void * ptr = tagValueOf(se);
    TypeOfReflStackEntry what = (TypeOfReflStackEntry) tagTypeOf(se);

    switch (what) {
    case Entry_Propagator:
      {
        Propagator * prop = (Propagator *) ptr;
        (void) reflect_space_prop(rec_stack,
                                  var_list,
                                  vtable,
                                  ptable,
                                  prop);

      }
      break;
    case Entry_Variable:
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

}
