/*
 *  Authors:
 *    Tobias Müller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *
 *  Copyright:
 *    Tobias Müller, 2000
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

#ifndef __TASKOVERLAP_HH__
#define __TASKOVERLAP_HH__

#include "std.hh"
#include "rel.hh"
#include "pel_fncts.hh"

//-----------------------------------------------------------------------------

#include "tasksoverlap_filterclass.hh"

class TasksOverlapPropagator : public Propagator_D_I_D_I_D {
  friend INIT_FUNC(fdp_init);
  //
private:
  //
  static OZ_PropagatorProfile profile;
  //
  FilterTasksOverlap<OZ_Filter<OZ_Propagator>, OZ_FDIntVar, OZ_FiniteDomain, PEL_PersistentFDIntVar, PEL_FDIntVar, PEL_PersistentEngine> tasksoverlap;
  //
public:
  //
  TasksOverlapPropagator(OZ_Term x, OZ_Term xd,
			 OZ_Term y, OZ_Term yd,
			 OZ_Term o)
    : Propagator_D_I_D_I_D(x, xd, y, yd, o),
    tasksoverlap(OZ_intToC(xd), OZ_intToC(yd)) { }
  //
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual void gCollect(void) {
    Propagator_D_I_D_I_D::gCollect();
    //
    tasksoverlap.gCollect();
  }
  virtual void sClone(void) {
    Propagator_D_I_D_I_D::sClone();
    //
    tasksoverlap.sClone();
  }
  virtual size_t sizeOf(void) { return sizeof(TasksOverlapPropagator); }
};

typedef TasksOverlapPropagator TasksOverlap;

//-----------------------------------------------------------------------------

#endif // __TASKOVERLAP_HH__

