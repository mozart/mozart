/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller, wuertz
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __SCHED_HH__
#define __SCHED_HH__

#include "std.hh"


//-----------------------------------------------------------------------------

class TaskIntervalsPropagator : public Propagator_VD_VI {
private:
  static OZ_CFun spawner;
public:
 TaskIntervalsPropagator (OZ_Term x, OZ_Term durations)
    : Propagator_VD_VI(x, durations) {}

  virtual OZ_Return propagate(void);
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
};

#endif // __SCHED_HH__
