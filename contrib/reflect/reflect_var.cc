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

//-----------------------------------------------------------------------------

OZ_Term reflect_variable(OZ_Term var)
{
  DEBUGPRINT(("reflect_variable (in)\n"));

  OZ_Term var_itself = var;
  DEREF(var, varptr);

  OZ_Term sl   = (OZ_Term) 0;
  OZ_Term type = OZ_nil();

  if (oz_isFree(var)) {
    type = atom_any;

    OZ_Term susp_arity_def[] = {
      OZ_pair2(atom_any, (oz_isVar(var) ?
                          reflect_susplist(tagged2Var(var)->getSuspList())
                          : OZ_nil())),
      (OZ_Term) 0
    };

    MKARITY(susp_arity, susp_arity_def);

    sl = OZ_recordInit(atom_susplists, susp_arity);
  } else if (isGenFDVar(var)) {
    type = atom_fd;

    OZ_Term susp_arity_def[] = {
      OZ_pair2(atom_any,
               reflect_susplist(tagged2Var(var)->
                                getSuspList())),
      OZ_pair2(atom_bounds,
               reflect_susplist(tagged2GenFDVar(var)->
                                getSuspList(fd_prop_bounds))),
      OZ_pair2(atom_val,
               reflect_susplist(tagged2GenFDVar(var)->
                                getSuspList(fd_prop_singl))),
      (OZ_Term) 0
    };

    MKARITY(susp_arity, susp_arity_def);

    sl = OZ_recordInit(atom_susplists, susp_arity);
  } else if (isGenBoolVar(var)) {
    type = atom_bool;

    OZ_Term susp_arity_def[] = {
      OZ_pair2(atom_any,
               reflect_susplist(tagged2Var(var)->getSuspList())),
      (OZ_Term) 0
    };

    MKARITY(susp_arity, susp_arity_def);

    sl = OZ_recordInit(atom_susplists, susp_arity);
  } else if (isGenFSetVar(var)) {
    type = atom_fs;

    OZ_Term susp_arity_def[] = {
      OZ_pair2(atom_any,
               reflect_susplist(tagged2Var(var)->getSuspList())),
      OZ_pair2(atom_glb,
               reflect_susplist(tagged2GenFSetVar(var)->
                                getSuspList(fs_prop_glb))),
      OZ_pair2(atom_lub,
               reflect_susplist(tagged2GenFSetVar(var)->
                                getSuspList(fs_prop_lub))),
      OZ_pair2(atom_val,
               reflect_susplist(tagged2GenFSetVar(var)->
                                getSuspList(fs_prop_val))),
      (OZ_Term) 0
    };

    MKARITY(susp_arity, susp_arity_def);

    sl = OZ_recordInit(atom_susplists, susp_arity);
  } else if (isGenCtVar(var)) {
    type = atom_ct;

    OzCtVariable * v = tagged2GenCtVar(var);
    OZ_CtDefinition * def = v->getDefinition();
    int numOfSuspLists = def->getNoEvents();
    char ** namesOfSuspLists = def->getEventNames();
    const int ind_offset = 2;
    OZ_Term susp_arity_def[ind_offset + numOfSuspLists];

    susp_arity_def[0] = OZ_pair2(atom_any,
                                 reflect_susplist(tagged2Var(var)->
                                                  getSuspList()));
    susp_arity_def[numOfSuspLists + ind_offset - 1] = (OZ_Term) 0;

    for (int i = numOfSuspLists; i--; )
      susp_arity_def[i + ind_offset - 1] =
        OZ_pair2(OZ_atom(namesOfSuspLists[i]),
                 reflect_susplist(v->getSuspList(i)));

    MKARITY(susp_arity, susp_arity_def);

    sl = OZ_recordInit(atom_susplists, susp_arity);
  }

  if (type != OZ_nil()) {
    OZ_Term arity_def[] = {
      OZ_pair2(atom_var,  var_itself),
      OZ_pair2(atom_type, type),
      OZ_pair2(atom_susplists, sl),
      OZ_pair2(atom_name, OZ_atom(oz_varGetName(var_itself))),
      (OZ_Term) 0
    };

    MKARITY(arity, arity_def);

    DEBUGPRINT(("reflect_variable out\n"));

    return OZ_recordInit(atom_var, arity);
  }


  DEBUGPRINT(("reflect_variable out\n"));

  return OZ_nil();
}
