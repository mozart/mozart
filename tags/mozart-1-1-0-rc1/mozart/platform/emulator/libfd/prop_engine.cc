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

//-----------------------------------------------------------------------------

void PEL_EventList::wakeup(PEL_PropQueue * pq, PEL_PropFnctTable * pft) {
  for (int i = _high; i--; ) {
    int idx = operator[](i);
    PEL_PropFnctTableEntry &pft_idx = (*pft)[idx];
    if (! pft_idx.isScheduled()) {
      CDM((" <waking up>"));
      pq->enqueue(idx);
      pft_idx.setScheduled();
    }
  }
}

//-----------------------------------------------------------------------------

int PEL_PropFnctTable::add(PEL_ParamTable &pt,
			   PEL_PropQueue &pq,
			   pf_fnct_t fnct, ...)
{
  int pt_high = pt.getHigh();
  PEL_PropFnctTableEntry fnct_entry(fnct, pt_high);
  int r = push(fnct_entry);
  pq.incAPF();

  CDM(("fnct=%p pt_high=%d\n", fnct, pt_high));

  va_list ap;

  va_start(ap, fnct);

  for (int param = va_arg(ap, int); param != -1; param = va_arg(ap, int)) {
#ifdef DEBUG_COMPOUND
    int i =
#endif
    pt.add(param);

    CDM(("arg=%d idx=%d\n", param, i));
  }

  CDM(("=============================\n"));

  return r;
}

//-----------------------------------------------------------------------------

pf_return_t PEL_PropQueue::apply(PEL_PropFnctTable &pft,
				 PEL_ParamTable &pt,
				 PEL_SuspVar * x[])
{
  int idx = dequeue();
  PEL_PropFnctTableEntry &fnct_entry = pft[idx]; // this must be a reference

  pf_fnct_t fnct = fnct_entry.getFnct();
  int paramIdx = fnct_entry.getParamIdx();

  pf_return_t r = fnct(&pt[paramIdx], x);

  if (r == pf_entailed) {
    decAPF();
    fnct_entry.setDead();
    return r;
  }
  if (r == pf_failed) {
    setFailed();
    return r;
  }

  CDM(("apply sleep\n"));
  fnct_entry.unsetScheduled();
  return r;
}
