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

#ifndef __TASKSOVERLAP_FILTER_HH__
#define __TASKSOVERLAP_FILTER_HH__

#include "rel_filter.hh"

#ifndef CMD
#define CMD
#endif

#ifndef OZ_DEBUGPRINTTHIS
#define OZ_DEBUGPRINTTHIS
#endif

#ifndef OZ_DEBUGPRINT
#define OZ_DEBUGPRINT
#endif

template  <class SERVICE, class FDVAR, class FDM, class P_PFDVAR, class PFDVAR, class ENGINE>
SERVICE &FilterTasksOverlap<SERVICE, FDVAR, FDM, P_PFDVAR, PFDVAR, ENGINE>::filter(SERVICE & s,
							FDVAR &x,
							int xd,
							FDVAR &y,
							int yd,
							FDVAR &o)
{
  PFDVAR cl1_t1, cl1_t2, cl1_o;
  PFDVAR cl2_t1, cl2_t2, cl2_o;
  PFDVAR cl3_t1, cl3_t2, cl3_o;
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
  //--------------------------------------------------
  int x_card, y_card, o_card;
  do {
    x_card = x->getSize();
    y_card = y->getSize();
    o_card = o->getSize();
    //
    if (!engine_cl1.isFailed()) {
      CDM(("cl1 propagating to\n"));
      if (!(cl1_t1.propagate_to(*x, _first) &
	    cl1_t2.propagate_to(*y, _first) &
	    cl1_o.propagate_to(*o, _first))) {
	CDM(("cl1 failed while propagating to clause\n"));
	engine_cl1.setFailed();
      }
    }
    if (!engine_cl2.isFailed()) {
      CDM(("cl2 propagating to\n"));
      if (!(cl2_t1.propagate_to(*x, _first) &
	    cl2_t2.propagate_to(*y, _first) &
	    cl2_o.propagate_to(*o, _first))) {
	CDM(("cl2 failed while propagating to clause\n"));
	engine_cl2.setFailed();
      }
    }
    if (!engine_cl3.isFailed()) {
      CDM(("cl3 propagating to\n"));
      if (!(cl3_t1.propagate_to(*x, _first) &
	    cl3_t2.propagate_to(*y, _first) &
	    cl3_o.propagate_to(*o, _first))) {
	CDM(("cl3 failed propagating to clause\n"));
	engine_cl3.setFailed();
      }
    }
    _first = 0;
    CDM(("cl1 running propagation queue\n"));
    engine_cl1.propagate();
    CDM(("cl2 running propagation queue\n"));
    engine_cl2.propagate();
    CDM(("cl3 running propagation queue\n"));
    engine_cl3.propagate();
    //--------------------------------------------------
    FDM u_t1, u_t2, u_o;
    u_t1.initEmpty(); u_t2.initEmpty(); u_o.initEmpty();
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
    FailOnEmpty(*x &= u_t1);
    FailOnEmpty(*y &= u_t2);
    FailOnEmpty(*o  &= u_o);
  } while (x_card > x->getSize() || 
	   y_card > y->getSize() ||
	   o_card > o->getSize());
  
  //--------------------------------------------------
  {
    _first = 0;
    // 3. step 
    CDM(("all propagation queues are empty\n"));
    int nb_failed_clauses = (engine_cl1.isFailed() +
			     engine_cl2.isFailed() +
			     engine_cl3.isFailed());
    // 3.a step
    if (nb_failed_clauses == 3) {
      goto failure;
    }
    // step 3.c
    //   a clause is entailed if no prop fncts are left and
    //   the basic constraints are subsumed
    if (engine_cl1.hasNoPropsLeft() &&
	x->getSize() == cl1_t1->getSize() &&
	y->getSize() == cl1_t2->getSize() &&
	o->getSize() == cl1_o->getSize()) {
      CDM(("cl1 entailed\n"));
      goto vanish;
    }
    if (engine_cl2.hasNoPropsLeft() &&
	x->getSize() == cl2_t1->getSize() &&
	y->getSize() == cl2_t2->getSize() &&
	o->getSize() == cl2_o->getSize()) {
      CDM(("cl2 entailed\n"));
      goto vanish;
    }
    if (engine_cl3.hasNoPropsLeft() &&
	x->getSize() == cl3_t1->getSize() &&
	y->getSize() == cl3_t2->getSize() &&
	o->getSize() == cl3_o->getSize()) {
      CDM(("cl3 entailed\n"));
      goto vanish;
    }
    // 3.b step
    if (nb_failed_clauses == 2) {
      if (!engine_cl1.isFailed()) {
	CDM(("cl1 unit committed\n"));
	// t1 + d1 > t2
	int r;
	make_lessEqOffset(r, *s, y, x, OZ_int(xd-1));
	// t2 + d2 > t1
	make_lessEqOffset(r, *s, x, y, OZ_int(yd-1));
	// o = 1
	FailOnEmpty(*o &= 1);
	goto vanish;
      }
      if (!engine_cl2.isFailed()) {
	CDM(("cl2 unit committed\n"));
	// t1 + d1 <= t2
	int r;
	make_lessEqOffset(r, *s, x, y, OZ_int(-xd));
	// o = 1
	FailOnEmpty(*o &= 0);
	goto vanish;
      }
      if (!engine_cl3.isFailed()) {
	CDM(("cl3 unit committed\n"));
	// t2 + d2 <= t1
	int r;
	make_lessEqOffset(r, *s, y, x, OZ_int(-yd));
	// o = 1
	FailOnEmpty(*o &= 0);
	goto vanish;
      }
      CDM(("oops 1\n"));
    } // step 3.b
  }
  CDM(("propagation fix-point reached\n"));
  //  printf("gaga %s",(*s).toString());
  CDM(("leaving\n"));
  OZ_DEBUGPRINTTHIS("out: ");
  return s.leave();
  //
 vanish:
  OZ_DEBUGPRINTTHIS("out: ");
  CDM(("vanishing\n"));
  return s.entail();
  //
 failure:
  OZ_DEBUGPRINT(("fail"));
  CDM(("failing\n"));
  return s.fail();
}

//-----------------------------------------------------------------------------

#define EXPECT(O, F, V, R) if (O.F(V, R)) return R;

#define EXPECT_SUSPEND(O, F, V, R, C) 		\
{						\
  int __r = O.F(V, R);				\
  if (__r == 1) return R;			\
  if (__r == 2) { C += 1; return R; }		\
}

template <class RETURN, class EXPECT, class FDVAR>
RETURN make_tasksoverlap(RETURN r, EXPECT &pe,
			 FDVAR x, FDVAR xd, FDVAR y, FDVAR yd, FDVAR o)
{
  int _d;
  EXPECT(pe, expectIntVarBounds, x, r);
  EXPECT(pe, expectInt, xd, r);
  EXPECT(pe, expectIntVarBounds, y, r);
  EXPECT(pe, expectInt, yd, r);
  EXPECT_SUSPEND(pe, expectBoolVar, o, r, _d);
  //
  return pe.impose(new TasksOverlap(x, xd, y, yd, o));
}

#endif
