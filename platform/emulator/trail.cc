/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "types.hh"

#include "trail.hh"


/* We define constructor&destructor here (not in memory.C)   */
/* since we need to allocate this structure only once.       */
RebindTrail::RebindTrail (int sizeInit)
{
  lowBound = (TrailEntry *) new char[sizeInit * sizeof (TrailEntry)];
  if ( lowBound == NULL )
    error ("RebindTrail::RebindTrail: failed");
  cursor = lowBound;
  upperBound = lowBound + sizeInit;
  size = sizeInit;
}

RebindTrail::~RebindTrail ()
{
  delete [] (char *)lowBound;
}

void RebindTrail::pushCouple(TaggedRef *reference, TaggedRef oldValue)
{
  if (cursor >= upperBound)
    error ("RebindTrail::pushCouple: space in rebindTrail exhausted");
  cursor->setRefPtr(reference);
  cursor->setValue(oldValue);
  cursor++;
}

void RebindTrail::popCouple(TaggedRef * &reference, TaggedRef &value)
{
  cursor--;
  if (cursor < lowBound)
    error ("RebindTrail::popCouple: bottom in rebindTrail is reached");
  reference = cursor->getRefPtr();
  value = cursor->getValue();
}
