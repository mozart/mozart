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

OZ_Term reflect_susplist(SuspList * sl) 
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

//-----------------------------------------------------------------------------
