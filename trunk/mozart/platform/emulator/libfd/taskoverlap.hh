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

template <class SERVICE, class FDVAR>
class FilterTasksOverlap : public OZ_StatefulFilter {
private:
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
  FilterTasksOverlap(int xd, int yd) {
    _first = 0;
    // clause 1
    {
      _engine_cl1.init();
      make_PEL_GreaterOffset(_engine_cl1, _cl1_t1, xd, _cl1_t2);
      make_PEL_GreaterOffset(_engine_cl1, _cl1_t2, yd, _cl1_t1);
      
      (*_cl1_t1).initFull();
      (*_cl1_t2).initFull();
      (*_cl1_o).initSingleton(1);
    }
    // clause 2
    {
      _engine_cl2.init();
      make_PEL_LessEqOffset(_engine_cl2, _cl2_t1, xd, _cl2_t2);
      (*_cl2_t1).initFull();
      (*_cl2_t2).initFull();
      (*_cl2_o).initSingleton(0);
    }
    // clause 3
    {
      _engine_cl3.init();
      make_PEL_LessEqOffset(_engine_cl3, _cl3_t2, yd, _cl3_t1);
      //
      (*_cl3_t1).initFull();
      (*_cl3_t2).initFull();
      (*_cl3_o).initSingleton(0);
    }
  }
  virtual void sClone(void) {
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
  virtual void gCollect(void) {
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
  SERVICE &filter(SERVICE & s, FDVAR &x, int xd, FDVAR &y, int yd, FDVAR &o);
};

//-----------------------------------------------------------------------------

class TasksOverlapPropagator : public Propagator_D_I_D_I_D {
  friend INIT_FUNC(fdp_init);
  //
private:
  //
  static OZ_PropagatorProfile profile;
  //
  FilterTasksOverlap<OZ_Service, OZ_FDIntVar> tasksoverlap;
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

//-----------------------------------------------------------------------------

#endif // __TASKOVERLAP_HH__

