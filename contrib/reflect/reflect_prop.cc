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

OZ_Term reflect_propagator(Suspendable * susp)
{
  Propagator * prop = SuspToPropagator(susp);
  OZ_Propagator * p = prop->getPropagator();
  Board * b = GETBOARD(susp);

  OZ_Term arity_def[] = {
    OZ_pair2(atom_type, atom_prop),
    OZ_pair2(atom_params, p->getParameters()),
    OZ_pair2(atom_name, prop_name(p->getProfile()->getPropagatorName())),
    OZ_pair2(atom_ref, propagator2Term(prop)),
    (OZ_Term) 0
  };

  MKARITY(arity, arity_def);

  return OZ_recordInit(atom_susp, arity);
}

//-----------------------------------------------------------------------------

OZ_Term reflect_thread(Suspendable * susp)
{
  Board * b = GETBOARD(susp);

  OZ_Term arity_def[] = {
    OZ_pair2(atom_type, atom_thread),
    (OZ_Term) 0
  };

  MKARITY(arity, arity_def);

  return OZ_recordInit(atom_susp, arity);
}

//-----------------------------------------------------------------------------

OZ_Term reflect_susplist(SuspList * sl)
{
  OZ_Term cl = OZ_nil();

  for (SuspList * p = sl; p != NULL; p = p->getNext()) {
    Suspendable * susp = p->getSuspendable();

    cl = OZ_cons((susp->isPropagator()
                  ? reflect_propagator(susp)
                  : reflect_thread(susp)), cl);
  }

  return cl;
}

//-----------------------------------------------------------------------------
