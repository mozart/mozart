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
 *     http://www.mozart-oz.org/
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "taskoverlap.hh"
#include "rel.hh"
#include "rel_filter.hh"

//-----------------------------------------------------------------------------
// constructor

TasksOverlapPropagator::TasksOverlapPropagator(OZ_Term x, OZ_Term xd,
                                               OZ_Term y, OZ_Term yd,
                                               OZ_Term o)
  : Propagator_D_I_D_I_D(x, xd, y, yd, o)
{
  _first = 0;
  // clause 1
  {
    _engine_cl1.init();
    make_PEL_GreaterOffset(_engine_cl1, _cl1_t1, reg_xd, _cl1_t2);
    make_PEL_GreaterOffset(_engine_cl1, _cl1_t2, reg_yd, _cl1_t1);

    (*_cl1_t1).initFull();
    (*_cl1_t2).initFull();
    (*_cl1_o).initSingleton(1);
  }
  // clause 2
  {
    _engine_cl2.init();
    make_PEL_LessEqOffset(_engine_cl2, _cl2_t1, reg_xd, _cl2_t2);
    (*_cl2_t1).initFull();
    (*_cl2_t2).initFull();
    (*_cl2_o).initSingleton(0);
  }
  // clause 3
  {
    _engine_cl3.init();
    make_PEL_LessEqOffset(_engine_cl3, _cl3_t2, reg_yd, _cl3_t1);
    //
    (*_cl3_t1).initFull();
    (*_cl3_t2).initFull();
    (*_cl3_o).initSingleton(0);
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

  PEL_FDIntVar cl1_t1, cl1_t2, cl1_o;
  PEL_FDIntVar cl2_t1, cl2_t2, cl2_o;
  PEL_FDIntVar cl3_t1, cl3_t2, cl3_o;
  //
  PEL_Engine engine_cl1(_engine_cl1, "DDD",
                        &_cl1_t1, &cl1_t1,
                        &_cl1_t2, &cl1_t2,
                        &_cl1_o, &cl1_o);
  CMD(engine_cl1.getPropTable().print(engine_cl1));

  PEL_Engine engine_cl2(_engine_cl2, "DDD",
                        &_cl2_t1, &cl2_t1,
                        &_cl2_t2, &cl2_t2,
                        &_cl2_o, &cl2_o);
  CMD(engine_cl2.getPropTable().print(engine_cl2));

  PEL_Engine engine_cl3(_engine_cl3, "DDD",
                        &_cl3_t1, &cl3_t1,
                        &_cl3_t2, &cl3_t2,
                        &_cl3_o, &cl3_o);
  CMD(engine_cl3.getPropTable().print(engine_cl3));
  //
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
      if (!engine_cl1.isFailed()) {
        u_t1 = u_t1 | *cl1_t1;
        u_t2 = u_t2 | *cl1_t2;
        u_o  = u_o  | *cl1_o;
      }
      if (!engine_cl2.isFailed()) {
        u_t1 = u_t1 | *cl2_t1;
        u_t2 = u_t2 | *cl2_t2;
        u_o  = u_o  | *cl2_o;
      }
      if (!engine_cl3.isFailed()) {
        u_t1 = u_t1 | *cl3_t1;
        u_t2 = u_t2 | *cl3_t2;
        u_o  = u_o  | *cl3_o;
      }
      FailOnEmpty(*_t1 &= u_t1);
      FailOnEmpty(*_t2 &= u_t2);
      FailOnEmpty(*_o  &= u_o);
    }
    not_first_iteration = 1;
    //--------------------------------------------------
    // 2. step
    if (!engine_cl1.isFailed()) {
      CDM(("cl1 propagating to\n"));
      if (!(cl1_t1.propagate_to(*_t1, _first) &
            cl1_t2.propagate_to(*_t2, _first) &
            cl1_o.propagate_to(*_o, _first))) {
        CDM(("cl1 failed while lifting\n"));
        engine_cl1.setFailed();
      }
    }
    if (!engine_cl2.isFailed()) {
      CDM(("cl2 propagating to\n"));
      if (!(cl2_t1.propagate_to(*_t1, _first) &
            cl2_t2.propagate_to(*_t2, _first) &
            cl2_o.propagate_to(*_o, _first))) {
        CDM(("cl2 failed while lifting\n"));
        engine_cl2.setFailed();
      }
    }
    if (!engine_cl3.isFailed()) {
      CDM(("cl3 propagating to\n"));
      if (!(cl3_t1.propagate_to(*_t1, _first) &
            cl3_t2.propagate_to(*_t2, _first) &
            cl3_o.propagate_to(*_o, _first))) {
        CDM(("cl3 failed\n"));
        engine_cl3.setFailed();
      }
    }
    _first = 0;
    // 3. step // was soll das?, wir sind och eager, oder?
    if (engine_cl1.hasReachedFixPoint() &&
        engine_cl2.hasReachedFixPoint() &&
        engine_cl3.hasReachedFixPoint()) {
      CDM(("all propagation queues are empty\n"));
      int nb_failed_clauses = (engine_cl1.isFailed() +
                               engine_cl2.isFailed() +
                               engine_cl3.isFailed());
      // 3.a step
      if (nb_failed_clauses == 3) {
        goto failure;
      }
      // 3.b step
      if (nb_failed_clauses == 2) {
        if (!engine_cl1.isFailed()) {
          CDM(("cl1 unit committed\n"));
          // t1 + d1 > t2
          {
            int r;
            make_lessEqOffset(r, *this, reg_y, reg_x, OZ_int(d1-1));
          }
          // t2 + d2 > t1
          {
            int r;
            make_lessEqOffset(r, *this, reg_x, reg_y, OZ_int(d2-1));
          }
          // o = 1
          FailOnEmpty(*_o &= 1);
          goto vanish;
        }
        if (!engine_cl2.isFailed()) {
          CDM(("cl2 unit committed\n"));
          // t1 + d1 <= t2
          {
            int r;
            make_lessEqOffset(r, *this, reg_x, reg_y, OZ_int(-d1));
          }
          // o = 1
          FailOnEmpty(*_o &= 0);
          goto vanish;
        }
        if (!engine_cl3.isFailed()) {
          CDM(("cl3 unit committed\n"));
          // t2 + d2 <= t1
          {
            int r;
            make_lessEqOffset(r, *this, reg_y, reg_x, OZ_int(-d2));
          }
          // o = 1
          FailOnEmpty(*_o &= 0);
          goto vanish;
        }
        CDM(("oops 1\n"));
      } // step 3.b
      // step 3.c
      //   a clause is entailed if no prop fncts are left and
      //   the basic constraints are subsumed
      if (engine_cl1.isBasic() &&
          _t1->getSize() <= cl1_t1->getSize() &&
          _t2->getSize() <= cl1_t2->getSize() &&
          _o->getSize()  <= cl1_o->getSize()) {
            goto vanish;
      }
      if (engine_cl2.isBasic() &&
          _t1->getSize() <= cl2_t1->getSize() &&
          _t2->getSize() <= cl2_t2->getSize() &&
          _o->getSize()  <= cl2_o->getSize()) {
        goto vanish;
      }
      if (engine_cl3.isBasic() &&
          _t1->getSize() <= cl3_t1->getSize() &&
          _t2->getSize() <= cl3_t2->getSize() &&
          _o->getSize()  <= cl3_o->getSize()) {
        goto vanish;
      }
      break;
    } // step 3.
    // 4.step
    CDM(("cl1 running propagation queue\n"));
    engine_cl1.propagate();
    CDM(("cl2 running propagation queue\n"));
    engine_cl2.propagate();
    CDM(("cl3 running propagation queue\n"));
    engine_cl3.propagate();
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

OZ_BI_define(fdp_tasksOverlap, 5, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_INT ","
                   OZ_EM_FD "," OZ_EM_INT ","
                   OZ_EM_FDBOOL);

  PropagatorExpect pe;
  int dummy;

  OZ_EXPECT(pe, 0, expectIntVarMinMax);
  OZ_EXPECT(pe, 1, expectInt);
  OZ_EXPECT(pe, 2, expectIntVarMinMax);
  OZ_EXPECT(pe, 3, expectInt);
  OZ_EXPECT_SUSPEND(pe, 4, expectBoolVar, dummy);

  return pe.impose(new TasksOverlapPropagator(OZ_in(0),
                                              OZ_in(1),
                                              OZ_in(2),
                                              OZ_in(3),
                                              OZ_in(4)));
}
OZ_BI_end

OZ_PropagatorProfile TasksOverlapPropagator::profile;
