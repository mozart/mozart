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

OZ_Term atom_var, atom_any, atom_type, atom_fd, atom_fs, atom_bool,  
  atom_bounds, atom_val, atom_glb, atom_lub, atom_flat, atom_local, atom_ask, 
  atom_wait, atom_waittop, atom_oops, atom_prop, atom_params, atom_name,
  atom_space, atom_susp, atom_thread, atom_ct, atom_susplists, atom_ref,
  atom_id, atom_loc, atom_vars, atom_props, atom_reflect;
  
//=============================================================================
// interface to the Oz loader

extern "C"
{
  OZ_C_proc_interface * oz_init_module(void)
  {
    static OZ_C_proc_interface i_table[] = {
      {"spaceReflect",          1, 1, BIReflectSpace},
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
    atom_id        = OZ_atom("id");  
    atom_loc       = OZ_atom("location");  
    atom_vars      = OZ_atom("vars");  
    atom_props     = OZ_atom("props");  
    atom_reflect   = OZ_atom("reflect");  

    PropagatorReference::_id = oz_newUniqueId();

    return i_table;
  } /* oz_init_module */
} /* extern "C" */

// End of File
//-----------------------------------------------------------------------------
