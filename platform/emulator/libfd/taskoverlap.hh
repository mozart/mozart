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
#include "pel_fncts.hh"

//-----------------------------------------------------------------------------

class TasksOverlapPropagator : public Propagator_D_I_D_I_D {
  friend INIT_FUNC(fdp_init);

private:
  static OZ_PropagatorProfile profile;
  PEL_PersistentFDIntVar _cl1_t1, _cl1_t2, _cl1_o;
  PEL_PersistentFDIntVar _cl2_t1, _cl2_t2, _cl2_o;
  PEL_PersistentFDIntVar _cl3_t1, _cl3_t2, _cl3_o;
  //
  // persistent part of propagation engine
  //
  PEL_PersistentEngine _engine_cl1, _engine_cl2, _engine_cl3;
  //
  int _first;
  //
public:
  TasksOverlapPropagator(OZ_Term x, OZ_Term xd, OZ_Term y, OZ_Term yd,
                         OZ_Term o);
  //
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual void gCollect(void) {
    Propagator_D_I_D_I_D::gCollect();
    //
    // here goes the additional stuff:
    _engine_cl1.gCollect();
    _engine_cl2.gCollect();
    _engine_cl3.gCollect();
    _cl1_t1.gCollect();
    _cl1_t2.gCollect();
    _cl1_o.gCollect();
    _cl2_t1.gCollect();
    _cl2_t2.gCollect();
    _cl2_o.gCollect();
    _cl3_t1.gCollect();
    _cl3_t2.gCollect();
    _cl3_o.gCollect();
  }
  virtual void sClone(void) {
    Propagator_D_I_D_I_D::sClone();
    //
    // here goes the additional stuff:
    _engine_cl1.sClone();
    _engine_cl2.sClone();
    _engine_cl3.sClone();
    _cl1_t1.sClone();
    _cl1_t2.sClone();
    _cl1_o.sClone();
    _cl2_t1.sClone();
    _cl2_t2.sClone();
    _cl2_o.sClone();
    _cl3_t1.sClone();
    _cl3_t2.sClone();
    _cl3_o.sClone();
  }
  virtual size_t sizeOf(void) { return sizeof(TasksOverlapPropagator); }
};

//-----------------------------------------------------------------------------

#endif // __TASKOVERLAP_HH__
