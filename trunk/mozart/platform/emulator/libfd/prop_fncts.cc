/*
 *  Authors:
 *    Joerg Wuertz (wuertz@dfki.de)
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
 *     http://www.mozart-oz.org/
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "prop_fncts.hh"
#include "std.hh"

//-----------------------------------------------------------------------------

// X + C <= Y
pf_return_t lessEqOff(int * map, PEL_SuspVar * regs[])
{
  CDM(("lessEqOff function %p ", map));

  PEL_SuspFDIntVar &x = *(PEL_SuspFDIntVar *) regs[map[0]];
  int c = (int) regs[map[1]];
  PEL_SuspFDIntVar &y = *(PEL_SuspFDIntVar *) regs[map[2]];

  FailOnEmpty(*x <= (y->getMaxElem() - c));
  FailOnEmpty(*y >= (x->getMinElem() + c));

  if (x->getMaxElem() + c <= y->getMinElem()) {
    x.wakeup();
    y.wakeup();
    CDM(("\t-> entailed\n"));
    return pf_entailed;
  }

  if (x->getMinElem() + c > y->getMaxElem()) {
    CDM(("\t-> failed\n"));
    goto failure;
  }
  {
    pf_return_t r = (x.wakeup() | y.wakeup()) ? pf_sleep : pf_entailed;
    CDM(("\t-> %s\n", r == pf_sleep ? "sleep" : "entailed"));
    return r;
  }
 failure:
  CDM(("\t-> failed\n"));
  return pf_failed;
}

// X + C > Y
pf_return_t greaterOff(int * map, PEL_SuspVar * regs[])
{
  CDM(("greaterOff function %p ", map));

  PEL_SuspFDIntVar &x = *(PEL_SuspFDIntVar *) regs[map[0]];
  int c = (int) regs[map[1]];
  PEL_SuspFDIntVar &y = *(PEL_SuspFDIntVar *) regs[map[2]];

  FailOnEmpty(*x >= (y->getMinElem() - c + 1));
  FailOnEmpty(*y <= (x->getMaxElem() + c - 1));

  if (x->getMinElem() + c > y->getMaxElem()) {
    x.wakeup();
    y.wakeup();
    CDM(("\t-> entailed\n"));
    return pf_entailed;
  }

  if (x->getMaxElem() + c <= y->getMinElem()) {
    CDM(("\t-> failed\n"));
    goto failure;
  }
  {
    pf_return_t r = (x.wakeup() | y.wakeup()) ? pf_sleep : pf_entailed;
    CDM(("\t-> %s\n", r == pf_sleep ? "sleep" : "entailed"));
    return r;
  }
 failure:
  CDM(("\t-> failed\n"));
  return pf_failed;
}

