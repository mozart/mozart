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

#include "reflect.hh"

//=============================================================================
// definition of static data members

int PropagatorReference::_id;

//=============================================================================
// auxiliary functions

OZ_Term prop_name(char * name)
{
  struct prop_names_tab_t {
    char * internal_name;
    char * external_name;
  } prop_names_tab[] = {
#include "prop_names_tab.cc"
    {(char *) NULL, (char *) NULL}
  };

  for (int i = 0; prop_names_tab[i].internal_name != NULL; i += 1) {
    if (!strcmp(name, prop_names_tab[i].internal_name))
      return OZ_atom(prop_names_tab[i].external_name);
  }
  return OZ_atom(name);
}

//-----------------------------------------------------------------------------

static OZ_Term atom_var, atom_any, atom_type, atom_fd, atom_fs, atom_bool,
  atom_bounds, atom_val, atom_glb, atom_lub, atom_flat, atom_local, atom_ask,
  atom_wait, atom_waittop, atom_oops, atom_prop, atom_params, atom_name,
  atom_space, atom_susp, atom_thread, atom_ct, atom_susplists, atom_ref;

#define MKARITY(Arity, ArityDef)                        \
OZ_Term Arity = OZ_nil();                               \
for (int i = 0; ArityDef[i] != (OZ_Term) 0; i += 1)     \
  Arity = OZ_cons(ArityDef[i], Arity);

OZ_Term reflect_propagator(Suspension susp)
{
  Propagator * prop = susp.getPropagator();
  OZ_Propagator * p = prop->getPropagator();
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
    {OZ_pair2(atom_name, prop_name(p->getProfile()->getPropagatorName()))},
    {OZ_pair2(atom_ref, propagator2Term(prop))},
    {OZ_pair2(atom_space, space)},
    {(OZ_Term) 0}
  };

  MKARITY(arity, arity_def);

  return OZ_recordInit(atom_susp, arity);
}

//-----------------------------------------------------------------------------

OZ_Term reflect_thread(Suspension susp)
{
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

  return OZ_recordInit(atom_susp, arity);
}

//-----------------------------------------------------------------------------

OZ_Term getConstraintsList(SuspList * sl)
{
  OZ_Term cl = OZ_nil();

  for (SuspList * p = sl; p != NULL; p = p->getNext()) {
    Suspension susp = p->getSuspension();

    cl = OZ_cons((susp.isPropagator()
                  ? reflect_propagator(susp)
                  : reflect_thread(susp)), cl);
  }

  return cl;
}


//=============================================================================
// built-ins

#define EXPECT_PROPGATORREF \
OZ_atom("Expecting Situated Extension (Propagator Reference)")

#define EXCEPTION \
"visualize_constraints_error"

OZ_BI_define(BIPropagatorEq, 2, 1)
{
  DEBUGPRINT(("BIPropagatorEq in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (!oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = oz_tagged2Extension(v1);
  if (PropagatorReference::getId() != se1->getIdV()) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Term v2 = OZ_in(1);

  if (!oz_isExtension(v2)) {
    return OZ_raiseErrorC(EXCEPTION, 1, EXPECT_PROPGATORREF, v2);
  }

  OZ_Extension * se2 = oz_tagged2Extension(v2);
  if (PropagatorReference::getId() != se2->getIdV()) {
    return OZ_raiseErrorC(EXCEPTION, 1, EXPECT_PROPGATORREF, v2);
  }

  DEBUGPRINT(("BIPropagatorEq out\n"));

  OZ_RETURN(* ((PropagatorReference *) se1) == *((PropagatorReference *) se2)
            ? oz_true() : oz_false());
} OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(BIReflectPropagator, 1, 1)
{
  DEBUGPRINT(("BIReflectPropagator in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (! oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = oz_tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    OZ_RETURN(oz_false());
  }

  OZ_Term r =
    reflect_propagator(((PropagatorReference*) se1)->getPropagator());

  DEBUGPRINT(("BIReflectPropagator out\n"));

  OZ_RETURN(r);
} OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(BIReflectPropagatorName, 1, 1)
{
  DEBUGPRINT(("BIReflectPropagatorName in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (! oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = oz_tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    OZ_RETURN(oz_false());
  }

  OZ_Term r = prop_name(((PropagatorReference*) se1)
                        ->getPropagator()->getPropagator()
                        ->getProfile()->getPropagatorName());

  DEBUGPRINT(("BIReflectPropagatorName out\n"));

  OZ_RETURN(r);
} OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(BIIsPropagatorFailed, 1, 1)
{
  DEBUGPRINT(("BIIsPropagatorFailed in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (! oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = oz_tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    OZ_RETURN(oz_false());
  }

  Propagator * p = ((PropagatorReference*) se1)->getPropagator();

  DEBUGPRINT(("BIIsPropagatorFailed out\n"));

  OZ_RETURN(p->isFailed() ? oz_true() : oz_false());
} OZ_BI_end


//-----------------------------------------------------------------------------

OZ_BI_define(BIReflectPropagatorCoordinates, 1, 1)
{
  DEBUGPRINT(("BIReflectPropagatorCoordinates in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (! oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = oz_tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    OZ_RETURN(oz_false());
  }

  OZ_Term r = oz_propGetName(((PropagatorReference*) se1)
                             ->getPropagator());

  DEBUGPRINT(("BIReflectPropagatorCoordinates out\n"));

  OZ_RETURN(r);
} OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(BIReflectVariable, 1, 1)
{
  DEBUGPRINT(("BIReflectVariable in\n"));

  OZ_Term var_itself  = OZ_in(0), var = var_itself;
  DEREF(var, varptr, vartag);


  OZ_Term sl   = (OZ_Term) 0;
  OZ_Term type = OZ_nil();

  if (oz_isFree(var)) {
    type = atom_any;

    OZ_Term susp_arity_def[] = {
      {OZ_pair2(atom_any, (isCVar(vartag) ?
                           getConstraintsList(tagged2CVar(var)->getSuspList())
                           : OZ_nil()))},
      {(OZ_Term) 0}
    };

    MKARITY(susp_arity, susp_arity_def);

    sl = OZ_recordInit(atom_var, susp_arity);
  } else if (isGenFDVar(var,vartag)) {
    type = atom_fd;

    OZ_Term susp_arity_def[] = {
      {OZ_pair2(atom_any,
                getConstraintsList(tagged2CVar(var)->
                                   getSuspList()))},
      {OZ_pair2(atom_bounds,
                getConstraintsList(tagged2GenFDVar(var)->
                                   getSuspList(fd_prop_bounds)))},
      {OZ_pair2(atom_val,
                getConstraintsList(tagged2GenFDVar(var)->
                                   getSuspList(fd_prop_singl)))},
      {(OZ_Term) 0}
    };

    MKARITY(susp_arity, susp_arity_def);

    sl = OZ_recordInit(atom_susplists, susp_arity);
  } else if (isGenBoolVar(var,vartag)) {
    type = atom_bool;

    OZ_Term susp_arity_def[] = {
      {OZ_pair2(atom_any,
                getConstraintsList(tagged2CVar(var)->getSuspList()))},
      {(OZ_Term) 0}
    };

    MKARITY(susp_arity, susp_arity_def);

    sl = OZ_recordInit(atom_susplists, susp_arity);
  } else if (isGenFSetVar(var,vartag)) {
    type = atom_fs;

    OZ_Term susp_arity_def[] = {
      {OZ_pair2(atom_any,
                getConstraintsList(tagged2CVar(var)->getSuspList()))},
      {OZ_pair2(atom_glb,
                getConstraintsList(tagged2GenFSetVar(var)->
                                   getSuspList(fs_prop_glb)))},
      {OZ_pair2(atom_lub,
                getConstraintsList(tagged2GenFSetVar(var)->
                                   getSuspList(fs_prop_lub)))},
      {OZ_pair2(atom_val,
                getConstraintsList(tagged2GenFSetVar(var)->
                                   getSuspList(fs_prop_val)))},
      {(OZ_Term) 0}
    };

    MKARITY(susp_arity, susp_arity_def);

    sl = OZ_recordInit(atom_susplists, susp_arity);
  } else if (isGenCtVar(var, vartag)) {
    type = atom_ct;

    OzCtVariable * v = tagged2GenCtVar(var);
    OZ_CtDefinition * def = v->getDefinition();
    int numOfSuspLists = def->getNoOfWakeUpLists();
    char ** namesOfSuspLists = def->getNamesOfWakeUpLists();
    const int ind_offset = 2;
    OZ_Term susp_arity_def[ind_offset + numOfSuspLists];

    susp_arity_def[0] = OZ_pair2(atom_any,
                                 getConstraintsList(tagged2CVar(var)->
                                                    getSuspList()));
    susp_arity_def[numOfSuspLists + ind_offset - 1] = (OZ_Term) 0;

    for (int i = numOfSuspLists; i--; )
      susp_arity_def[i + ind_offset - 1] =
        OZ_pair2(OZ_atom(namesOfSuspLists[i]),
                 getConstraintsList(v->getSuspList(i)));

    MKARITY(susp_arity, susp_arity_def);

    sl = OZ_recordInit(atom_susplists, susp_arity);
  }

  if (type != OZ_nil()) {
    OZ_Term arity_def[] = {
      {OZ_pair2(atom_var,  var_itself)},
      {OZ_pair2(atom_type, type)},
      {OZ_pair2(atom_susplists, sl)},
      {OZ_pair2(atom_name, OZ_atom(oz_varGetName(var_itself)))},
      {(OZ_Term) 0}
    };

    MKARITY(arity, arity_def);

    DEBUGPRINT(("BIReflectVariable out\n"));

    OZ_RETURN(OZ_recordInit(atom_var, arity));
  }


  DEBUGPRINT(("BIReflectVariable out\n"));

  OZ_RETURN(OZ_nil());
}
OZ_BI_end

//=============================================================================
// interface to the Oz loader

extern "C"
{
  OZ_C_proc_interface * oz_init_module(void)
  {
    static OZ_C_proc_interface i_table[] = {
      {"variableReflect",       1, 1, BIReflectVariable},
      {"propagatorEq",          2, 1, BIPropagatorEq},
      {"propagatorReflect",     1, 1, BIReflectPropagator},
      {"propagatorName",        1, 1, BIReflectPropagatorName},
      {"propagatorIsFailed",    1, 1, BIIsPropagatorFailed},
      {"propagatorCoordinates", 1, 1, BIReflectPropagatorCoordinates},
      {0,0,0,0}
    };

    atom_var       = OZ_atom("var");
    atom_any       = OZ_atom("any");
    atom_type      = OZ_atom("type");
    atom_fd        = OZ_atom("fd");
    atom_fs        = OZ_atom("fs");
    atom_bool      = OZ_atom("bool");
    atom_bounds    = OZ_atom("bounds");
    atom_val       = OZ_atom("val");
    atom_glb       = OZ_atom("glb");
    atom_lub       = OZ_atom("lub");
    atom_flat      = OZ_atom("flat actor");
    atom_local     = OZ_atom("home");
    atom_ask       = OZ_atom("ask actor");
    atom_wait      = OZ_atom("wait actor");
    atom_waittop   = OZ_atom("waittop actor");
    atom_oops      = OZ_atom("oops");
    atom_prop      = OZ_atom("propagator");
    atom_params    = OZ_atom("params");
    atom_name      = OZ_atom("name");
    atom_space     = OZ_atom("space");
    atom_susp      = OZ_atom("suspension");
    atom_thread    = OZ_atom("thread");
    atom_ct        = OZ_atom("ct");
    atom_susplists = OZ_atom("susplists");
    atom_ref       = OZ_atom("reference");

    PropagatorReference::_id = oz_newUniqueId();

    return i_table;
  } /* oz_init_module */
} /* extern "C" */

// End of File
//-----------------------------------------------------------------------------
