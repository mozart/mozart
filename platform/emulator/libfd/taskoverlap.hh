/*
 *  Authors:
 *    Jörg Würtz (wuertz@dfki.de)
 *
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Copyright:
 *    Jörg Würtz, 1997
 *    Christian Schulte, 1999
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
#include "prop_fncts.hh"

//-----------------------------------------------------------------------------

class TasksOverlapPropagator : public Propagator_D_I_D_I_D {
  friend INIT_FUNC(fdp_init);

private:
  static OZ_PropagatorProfile profile;
  //
                // clause 1: t1 + d1 >: t2 /\ t2 + d2 >: t1 /\ o =: 1
  enum _var_ix1 {_cl1_t1 = 0, _cl1_t2, _cl1_o,
                 // clause 2: t1 + d1 =<: t2 /\ o =: 0
		 _cl2_t1, _cl2_t2, _cl2_o,
                 // clause 3: t2 + d2 =<: t1 /\ o =: 0
		 _cl3_t1, _cl3_t2, _cl3_o, nb_lvars };
                 // constant values
  enum _var_ix2 {_d1 = nb_lvars, _d2, nb_consts};
  //
  OZ_FiniteDomain _ld[nb_lvars];
  //
  int _first;
  PEL_FDProfile     _x_profile, _y_profile;
  PEL_PropFnctTable _prop_fnct_table;
  PEL_ParamTable    _param_table;
  PEL_FDEventLists  _el[nb_lvars];
  PEL_PropQueue     _prop_queue_cl1, _prop_queue_cl2, _prop_queue_cl3;
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
    _prop_fnct_table.gCollect();
    _param_table.gCollect();
    for (int i = nb_lvars; i--; ) {
      _ld[i].copyExtension();
      _el[i].gCollect();
    }
  }
  virtual void sClone(void) {
    Propagator_D_I_D_I_D::sClone();
    //
    // here goes the additional stuff:
    _prop_fnct_table.sClone();
    _param_table.sClone();
    for (int i = nb_lvars; i--; ) {
      _ld[i].copyExtension();
      _el[i].sClone();
    }
  }
  virtual size_t sizeOf(void) { return sizeof(TasksOverlapPropagator); }
};

//-----------------------------------------------------------------------------

#endif // __TASKOVERLAP_HH__
