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

#include "disjoint.hh"
#include "rel.hh"
#include "auxcomp.hh"
#include "prop_engine.hh"

#include <stdlib.h>

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_disjoint, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD "," OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);
  OZ_EXPECT(pe, 3, expectInt);

  return pe.impose(new SchedCDPropagator(OZ_args[0], OZ_args[1],
					OZ_args[2], OZ_args[3]));
}
OZ_C_proc_end

OZ_Return SchedCDPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  int &xd = reg_xd, &yd = reg_yd;

  OZ_FDIntVar x(reg_x), y(reg_y);
  PropagatorController_V_V P(x, y);

  int xl = x->getMinElem(), xu = x->getMaxElem();
  int yl = y->getMinElem(), yu = y->getMaxElem();

  if (xu + xd <= yl) return P.vanish();
  if (yu + yd <= xl) return P.vanish();

  if (xl + xd > yu) {
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_y, reg_x, -yd));
  }

  if (yl + yd > xu) {
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_x, reg_y, -xd));
  }

  int lowx = yu-xd+1, lowy = xu-yd+1;
  int upx = yl+yd-1, upy = xl+xd-1;
  if (lowx <= upx) {
    OZ_FiniteDomain la;
    la.initRange(lowx,upx);
    FailOnEmpty(*x -= la);
  }
  if (lowy <= upy) {
    OZ_FiniteDomain la;
    la.initRange(lowy,upy);
    FailOnEmpty(*y -= la);
  }

  OZ_DEBUGPRINTTHIS("out: ");

  return P.leave();

failure:
  OZ_DEBUGPRINTTHIS("fail");
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_disjointC, 5)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FDBOOL);

  PropagatorExpect pe;
  int dummy;

  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);
  OZ_EXPECT(pe, 3, expectInt);
  OZ_EXPECT_SUSPEND(pe, 4, expectBoolVar, dummy);

  return pe.impose(new SchedCDBPropagator(OZ_args[0],
					  OZ_args[1],
					  OZ_args[2],
					  OZ_args[3],
					  OZ_args[4]));
}
OZ_C_proc_end

OZ_Return SchedCDBPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  int &xd = reg_xd, &yd = reg_yd;

  OZ_FDIntVar x(reg_x), y(reg_y), b(reg_b);
  PropagatorController_V_V_V P(x, y, b);
  OZ_FiniteDomain la, lb, lc, ld, l1, l2;

  int xl = x->getMinElem(), xu = x->getMaxElem();
  int yl = y->getMinElem(), yu = y->getMaxElem();
  int lowx, lowy, upx, upy;

  if (xu + xd <= yl) {
    FailOnEmpty(*b &= 0);
    return P.vanish();
  }
  if (yu + yd <= xl) {
    FailOnEmpty(*b &= 1);
    return P.vanish();
  }

  if (xl + xd > yu){
    FailOnEmpty(*b &= 1);
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_y, reg_x, -yd));
  }

  if (yl + yd > xu){
    FailOnEmpty(*b &= 0);
    P.vanish();
    return replaceBy(new LessEqOffPropagator(reg_x, reg_y, -xd));
  }

  if (*b == fd_singl) {
    P.vanish();
    if (b->getSingleElem() == 0) {
      return replaceBy(new LessEqOffPropagator(reg_x, reg_y, -xd));
    } else {
      return replaceBy(new LessEqOffPropagator(reg_y, reg_x, -yd));
    }
  }
  lowx = yu-xd+1; lowy = xu-yd+1;
  upx = yl+yd-1; upy = xl+xd-1;
  if (lowx <= upx) {
    OZ_FiniteDomain la;
    la.initRange(lowx,upx);
    FailOnEmpty(*x -= la);
  }
  if (lowy <= upy) {
    OZ_FiniteDomain la;
    la.initRange(lowy,upy);
    FailOnEmpty(*y -= la);
  }

  OZ_DEBUGPRINTTHIS("out: ");
  return P.leave();
failure:
  OZ_DEBUGPRINT(("fail"));

  return P.fail();
}

//-----------------------------------------------------------------------------
// TasksOverlapPropagator

class TasksOverlapPropagator : public Propagator_D_I_D_I_D {
  friend INIT_FUNC(fdp_nit);

private:
  static OZ_PropagatorProfile profile;

  int _first;
  OZ_FDProfile _x_profile, _y_profile;
  PropFnctTable _prop_fnct_table;
  ParamTable _param_table;

  // clause 1: t1 + d1 >: t2 /\ t2 + d2 >: t1 /\ o =: 1
  enum _var_ix1 {_cl1_t1 = 0, _cl1_t2, _cl1_o,

  // clause 2: t1 + d1 =<: t2 /\ o =: 0
	_cl2_t1, _cl2_t2, _cl2_o,

  // clause 3: t2 + d2 =<: t1 /\ o =: 0
	_cl3_t1, _cl3_t2, _cl3_o, nb_lvars };

  enum _var_ix2 {_d1 = nb_lvars, _d2, nb_consts};

  OZ_FiniteDomain _ld[nb_lvars];
  FDEventLists    _el[nb_lvars];
  PropQueue       _prop_queue_cl1, _prop_queue_cl2, _prop_queue_cl3;

public:
  TasksOverlapPropagator(OZ_Term x, OZ_Term xd, OZ_Term y, OZ_Term yd,
			 OZ_Term o);

  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
  virtual void updateHeapRefs(OZ_Boolean duplicate = OZ_FALSE) {
    Propagator_D_I_D_I_D::updateHeapRefs(duplicate);

    // here goes the additional stuff:
    _prop_fnct_table.gc();
    _param_table.gc();
    for (int i = nb_lvars; i--; ) {
      _ld[i].copyExtension();
      _el[i].gc();
    }
  }
  virtual size_t sizeOf(void) { return sizeof(TasksOverlapPropagator); }
};

//-----------------------------------------------------------------------------
// propagation functions

// X + C <= Y
pf_return_t lessEqOff(int * map, SuspVar * regs[])
{
  CDM(("lessEqOff function %p ", map));

  SuspFDIntVar &x = *(SuspFDIntVar *) regs[map[0]];
  int c = (int) regs[map[1]];
  SuspFDIntVar &y = *(SuspFDIntVar *) regs[map[2]];

  FailOnEmpty(*x <= (y->getMaxElem() - c));
  FailOnEmpty(*y >= (x->getMinElem() + c));

  if (x->getMaxElem() + c <= y->getMinElem()) {
    x.wakeUp();
    y.wakeUp();
    CDM(("\t-> entailed\n"));
    return pf_entailed;
  }

  if (x->getMinElem() + c > y->getMaxElem()) {
    CDM(("\t-> failed\n"));
    goto failure;
  }
  {
    pf_return_t r = (x.wakeUp() | y.wakeUp()) ? pf_sleep : pf_entailed;
    CDM(("\t-> %s\n", r == pf_sleep ? "sleep" : "entailed"));
    return r;
  }
 failure:
  CDM(("\t-> failed\n"));
  return pf_failed;
}

// X + C > Y
pf_return_t greaterOff(int * map, SuspVar * regs[])
{
  CDM(("greaterOff function %p ", map));

  SuspFDIntVar &x = *(SuspFDIntVar *) regs[map[0]];
  int c = (int) regs[map[1]];
  SuspFDIntVar &y = *(SuspFDIntVar *) regs[map[2]];

  FailOnEmpty(*x >= (y->getMinElem() - c + 1));
  FailOnEmpty(*y <= (x->getMaxElem() + c - 1));

  if (x->getMinElem() + c > y->getMaxElem()) {
    x.wakeUp();
    y.wakeUp();
    CDM(("\t-> entailed\n"));
    return pf_entailed;
  }

  if (x->getMaxElem() + c <= y->getMinElem()) {
    CDM(("\t-> failed\n"));
    goto failure;
  }
  {
    pf_return_t r = (x.wakeUp() | y.wakeUp()) ? pf_sleep : pf_entailed;
    CDM(("\t-> %s\n", r == pf_sleep ? "sleep" : "entailed"));
    return r;
  }
 failure:
  CDM(("\t-> failed\n"));
  return pf_failed;
}

//-----------------------------------------------------------------------------
// constructor

TasksOverlapPropagator::TasksOverlapPropagator(OZ_Term x,
					       OZ_Term xd,
					       OZ_Term y,
					       OZ_Term yd,
					       OZ_Term o)
  : Propagator_D_I_D_I_D(x, xd, y, yd, o)
{
  _first = 0;
  // clause 1
  {
    int fnct_idx;

    fnct_idx = _prop_fnct_table.add(_param_table, _prop_queue_cl1, greaterOff,
				    _cl1_t1, _d1, _cl1_t2);
    _el[_cl1_t1].getBounds().add(fnct_idx);
    _el[_cl1_t2].getBounds().add(fnct_idx);

    fnct_idx = _prop_fnct_table.add(_param_table, _prop_queue_cl1, greaterOff,
				    _cl1_t2, _d2, _cl1_t1);
    _el[_cl1_t1].getBounds().add(fnct_idx);
    _el[_cl1_t2].getBounds().add(fnct_idx);

    _ld[_cl1_t1].initFull();
    _ld[_cl1_t2].initFull();
    _ld[_cl1_o].initSingleton(1);
  }
  // clause 2
  {
    int fnct_idx;

    fnct_idx = _prop_fnct_table.add(_param_table, _prop_queue_cl2, lessEqOff,
				    _cl2_t1, _d1, _cl2_t2);
    _el[_cl2_t1].getBounds().add(fnct_idx);
    _el[_cl2_t2].getBounds().add(fnct_idx);

    _ld[_cl2_t1].initFull();
    _ld[_cl2_t2].initFull();
    _ld[_cl2_o].initSingleton(0);
  }

  // clause 3
  {
    int fnct_idx;

    fnct_idx = _prop_fnct_table.add(_param_table, _prop_queue_cl3, lessEqOff,
				    _cl3_t2, _d2, _cl3_t1);
    _el[_cl3_t1].getBounds().add(fnct_idx);
    _el[_cl3_t2].getBounds().add(fnct_idx);

    _ld[_cl3_t1].initFull();
    _ld[_cl3_t2].initFull();
    _ld[_cl3_o].initSingleton(0);
  }
}

//-----------------------------------------------------------------------------
// propagation member function

OZ_Return TasksOverlapPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in: ");

  int &d1 = reg_xd, &d2 = reg_yd;

  OZ_FDIntVar _t1(reg_x), _t2(reg_y), _o(reg_b);
  PropagatorController_V_V_V P(_t1, _t2, _o);

  _prop_queue_cl1.reset();
  _prop_queue_cl2.reset();
  _prop_queue_cl3.reset();

  SuspFDIntVar _x[nb_lvars];
  SuspVar * x[nb_lvars+nb_consts];

  x[_d1] = (SuspVar *) d1;
  x[_d2] = (SuspVar *) d2;

  for (int var_idx = _cl1_t1; var_idx <= _cl1_o; var_idx += 1)
    x[var_idx]=_x[var_idx].init(_ld[var_idx], _el[var_idx],
				_prop_queue_cl1, _prop_fnct_table);
  for (int var_idx = _cl2_t1; var_idx <= _cl2_o; var_idx += 1)
    x[var_idx]=_x[var_idx].init(_ld[var_idx], _el[var_idx],
				_prop_queue_cl2, _prop_fnct_table);
  for (int var_idx = _cl3_t1; var_idx <= _cl3_o; var_idx += 1)
    x[var_idx]=_x[var_idx].init(_ld[var_idx], _el[var_idx],
				_prop_queue_cl3, _prop_fnct_table);

  int nb_failed_clauses = 0, not_first_iteration = 0;

  /* 
     1. lift common information of unfailed clauses
     2. propagate basic constraints into unfailed spaces
     3. if all propagation queues are empty then:
     3.a check for failure
     3.b check for unit commit
     3.c check entailment of disjunction
     3.d otherwise leave propagator
     4. otherwise run propagation queues
     5. loop
  */
  while (1) {
    //--------------------------------------------------
    // 1. step
    if (not_first_iteration) {
      OZ_FiniteDomain u_t1(fd_empty), u_t2(fd_empty), u_o(fd_empty);
      if (!_prop_queue_cl1.isFailed()) {
	u_t1 = u_t1 | *_x[_cl1_t1];
	u_t2 = u_t2 | *_x[_cl1_t2];
	u_o  = u_o  | *_x[_cl1_o];
      }
      if (!_prop_queue_cl2.isFailed()) {
	u_t1 = u_t1 | *_x[_cl2_t1];
	u_t2 = u_t2 | *_x[_cl2_t2];
	u_o  = u_o  | *_x[_cl2_o];
      }
      if (!_prop_queue_cl3.isFailed()) {
	u_t1 = u_t1 | *_x[_cl3_t1];
	u_t2 = u_t2 | *_x[_cl3_t2];
	u_o  = u_o  | *_x[_cl3_o];
      }
      FailOnEmpty(*_t1 &= u_t1);
      FailOnEmpty(*_t2 &= u_t2);
      FailOnEmpty(*_o  &= u_o);
    }
    not_first_iteration = 1;
    //--------------------------------------------------
    // 2. step
    if (!_prop_queue_cl1.isFailed()) {
      CDM(("cl1 propagating to\n"));
      if (!(_x[_cl1_t1].propagate_to(*_t1, _first) &
	    _x[_cl1_t2].propagate_to(*_t2, _first) &
	    _x[_cl1_o].propagate_to(*_o, _first))) {
	CDM(("cl1 failed while lifting\n"));
	_prop_queue_cl1.setFailed();
      }
    }
    if (!_prop_queue_cl2.isFailed()) {
      CDM(("cl2 propagating to\n"));
      if (!(_x[_cl2_t1].propagate_to(*_t1, _first) &
	    _x[_cl2_t2].propagate_to(*_t2, _first) &
	    _x[_cl2_o].propagate_to(*_o, _first))) {
	CDM(("cl2 failed while lifting\n"));
	_prop_queue_cl2.setFailed();
      }
    }
    if (!_prop_queue_cl3.isFailed()) {
      CDM(("cl3 propagating to\n"));
      if (!(_x[_cl3_t1].propagate_to(*_t1, _first) &
	    _x[_cl3_t2].propagate_to(*_t2, _first) &
	    _x[_cl3_o].propagate_to(*_o, _first))) {
	CDM(("cl3 failed\n"));
	_prop_queue_cl3.setFailed();
      }
    }
    _first = 0;
    // 3. step
    if (_prop_queue_cl1.isEmpty() &&
	_prop_queue_cl2.isEmpty() &&
	_prop_queue_cl3.isEmpty()) {
      CDM(("all propagation queues are empty\n"));
      int nb_failed_clauses = (_prop_queue_cl1.isFailed() +
			       _prop_queue_cl2.isFailed() +
			       _prop_queue_cl3.isFailed());
      // 3.a step 
      if (nb_failed_clauses == 3) {
	goto failure;
      }
      // 3.b step
      if (nb_failed_clauses == 2) {
	if (!_prop_queue_cl1.isFailed()) {
	  CDM(("cl1 unit committed\n"));
	  // t1 + d1 > t2
	  {
	    addImpose(fd_prop_bounds, reg_x);
	    addImpose(fd_prop_bounds, reg_y);
	    impose(new LessEqOffPropagator(reg_y, reg_x, d1-1));
	  }
	  // t2 + d2 > t1
	  {
	    addImpose(fd_prop_bounds, reg_x);
	    addImpose(fd_prop_bounds, reg_y);
	    impose(new LessEqOffPropagator(reg_x, reg_y, d2-1));
	  }
	  // o = 1
	  FailOnEmpty(*_o &= 1);
	  goto vanish;
	}
	if (!_prop_queue_cl2.isFailed()) {
	  CDM(("cl2 unit committed\n"));
	  // t1 + d1 <= t2
	  {
	    addImpose(fd_prop_bounds, reg_x);
	    addImpose(fd_prop_bounds, reg_y);
	    impose(new LessEqOffPropagator(reg_x, reg_y, -d1));
	  }
	// o = 1
	  FailOnEmpty(*_o &= 0);
	  goto vanish;
	}
	if (!_prop_queue_cl3.isFailed()) {
	  CDM(("cl3 unit committed\n"));
	  // t2 + d2 <= t1
	  {
	    addImpose(fd_prop_bounds, reg_x);
	    addImpose(fd_prop_bounds, reg_y);
	    impose(new LessEqOffPropagator(reg_y, reg_x, -d2));
	  }
	  // o = 1
	  FailOnEmpty(*_o &= 0);
	  goto vanish;
	}
	CDM(("oops 1\n"));
      } // step 3.b
      // step 3.c
      //   a clause is entailed if no prop fncts are left and 
      //   the basic constraints subsumed 
      if (_prop_queue_cl1.isBasic() &&
	  _t1->getSize() <= _x[_cl1_t1]->getSize() &&
	  _t2->getSize() <= _x[_cl1_t2]->getSize() &&
	  _o->getSize()  <= _x[_cl1_o]->getSize()) {
	goto vanish;
      }
      if (_prop_queue_cl2.isBasic() &&
	  _t1->getSize() <= _x[_cl2_t1]->getSize() &&
	  _t2->getSize() <= _x[_cl2_t2]->getSize() &&
	  _o->getSize()  <= _x[_cl2_o]->getSize()) {
	goto vanish;
      }
      if (_prop_queue_cl3.isBasic() &&
	  _t1->getSize() <= _x[_cl3_t1]->getSize() &&
	  _t2->getSize() <= _x[_cl3_t2]->getSize() &&
	  _o->getSize()  <= _x[_cl3_o]->getSize()) {
	goto vanish;
      }
      break;
    } // step 3.
    // 4.step
    CDM(("cl1 running propagation queue\n"));
    while (!_prop_queue_cl1.isEmpty()) {
      pf_return_t r = _prop_queue_cl1.apply(_prop_fnct_table, _param_table, x);
    }
    CDM(("cl2 running propagation queue\n"));
    while (!_prop_queue_cl2.isEmpty()) {
      pf_return_t r = _prop_queue_cl2.apply(_prop_fnct_table, _param_table, x);
    }
    CDM(("cl3 running propagation queue\n"));
    while (!_prop_queue_cl3.isEmpty()) {
      pf_return_t r = _prop_queue_cl3.apply(_prop_fnct_table, _param_table, x);
    }
  } // while(1)
  //

  CDM(("leaving\n"));
  OZ_DEBUGPRINTTHIS("out: ");
  return P.leave();
  //
 vanish:
  OZ_DEBUGPRINTTHIS("out: ");
  CDM(("vanishing\n"));
  return P.vanish();
  //
 failure:
  OZ_DEBUGPRINT(("fail"));
  CDM(("failing\n"));
  return P.fail();
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_tasksOverlap, 5)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT "," OZ_EM_FD ","
		   OZ_EM_INT "," OZ_EM_FDBOOL);

  PropagatorExpect pe;
  int dummy;

  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);
  OZ_EXPECT(pe, 3, expectInt);
  OZ_EXPECT_SUSPEND(pe, 4, expectBoolVar, dummy);

  return pe.impose(new TasksOverlapPropagator(OZ_args[0],
					      OZ_args[1],
					      OZ_args[2],
					      OZ_args[3],
					      OZ_args[4]));
}
OZ_C_proc_end

//-----------------------------------------------------------------------------
// static member

OZ_PropagatorProfile SchedCDPropagator::profile;
OZ_PropagatorProfile SchedCDBPropagator::profile;
OZ_PropagatorProfile TasksOverlapPropagator::profile;

