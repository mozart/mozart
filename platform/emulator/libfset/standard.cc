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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "standard.hh"

OZ_Return make_intersect_3(OZ_Term x, OZ_Term y, OZ_Term z)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  _OZ_EXPECT_SUSPEND(pe, x, 0, expectFSetVarBounds, susp_count);
  _OZ_EXPECT_SUSPEND(pe, y, 1, expectFSetVarBounds, susp_count);
  _OZ_EXPECT_SUSPEND(pe, z, 2, expectFSetVarBounds, susp_count);

  if (susp_count > 1)
    return pe.suspend();

  return pe.impose(new FSetIntersectionPropagator(x, y, z));
}

OZ_BI_define(fsp_intersection, 3, 0)
{
  return make_intersect_3(OZ_in(0), OZ_in(1), OZ_in(2));
}
OZ_BI_end

OZ_BI_define(fsp_union, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectFSetVarBounds, susp_count);

  if (susp_count > 1)
    return pe.suspend();

  return pe.impose(new FSetUnionPropagator(OZ_in(0),
                                           OZ_in(1),
                                           OZ_in(2)));
}
OZ_BI_end


OZ_BI_define(fsp_subsume, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, susp_count);

  if (susp_count > 1)
    return pe.suspend();

  return pe.impose(new FSetSubsumePropagator(OZ_in(0),
                                             OZ_in(1)));
}
OZ_BI_end

OZ_Return make_disjoint_2(OZ_Term x, OZ_Term y)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  _OZ_EXPECT_SUSPEND(pe, x, 0, expectFSetVarGlb, susp_count);
  _OZ_EXPECT_SUSPEND(pe, y, 1, expectFSetVarGlb, susp_count);

  if (susp_count > 1)
    return pe.suspend();

  return pe.impose(new FSetDisjointPropagator(x, y));
}

OZ_BI_define(fsp_disjoint, 2, 0)
{
  return make_disjoint_2(OZ_in(0), OZ_in(1));
}
OZ_BI_end


OZ_BI_define(fsp_distinct, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, susp_count);

  if (susp_count > 1)
    return pe.suspend();

  return pe.impose(new FSetDistinctPropagator(OZ_in(0),
                                              OZ_in(1)));
}
OZ_BI_end

OZ_BI_define(fsp_diff, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET","OZ_EM_FSET","OZ_EM_FSET);

  PropagatorExpect pe;
   int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectFSetVarBounds, susp_count);

  if (susp_count > 1)
    return pe.suspend();

  return pe.impose(new FSetDiffPropagator(OZ_in(0),
                                          OZ_in(1),
                                          OZ_in(2)));
}
OZ_BI_end


//*****************************************************************************

//-----------------------------------------------------------------------------
// OZ_Filter

class OZ_Filter {
public:
  virtual int hasState(void) = 0;
};


//-----------------------------------------------------------------------------
// OZ_Service

class OZ_CPIVarVector {
private:
  int _size;
  OZ_Term ** _vector;
public:
  OZ_CPIVarVector(int sz, OZ_Term * &vector) : _size(sz), _vector(&vector) {}
  void condens(void);
};


class OZ_FSetVarVector : public OZ_CPIVarVector {
  OZ_FSetVar * _vector;
public:
  OZ_FSetVarVector(int size, OZ_Term * &vector);
};

typedef OZ_Return (*make_prop_fn_2)(OZ_Term, OZ_Term);
typedef OZ_Return (*make_prop_fn_3)(OZ_Term, OZ_Term, OZ_Term);
typedef OZ_Return (*make_prop_fn_4)(OZ_Term, OZ_Term, OZ_Term, OZ_Term);


class OZ_Service {
private:
  int _closed;
  OZ_Propagator * _prop;
  OZ_ParamIterator * _iter;
  static const int _max_actions = 10;
  struct _actions_t {
    enum {
      _serv_failed = 0,
      _serv_entailed,
      _serv_leave,
      _serv_replace,
      _serv_equate} _what;
    union _action_params_t {
      int _vars_left;
      OZ_Propagator * _replacement;
      struct { OZ_Term _x, _y; } _equat;
    } _action_params;
  } _actions[_max_actions];
  int _nb_actions;
  OZ_Return update_return(OZ_Return  o, OZ_Return n) {
    if (o == OZ_ENTAILED) {
      return n;
    } else if (o == OZ_SLEEP) {
      return (n == OZ_FAILED ? n : o);
    } else if (o = OZ_FAILED) {
      return o;
    }
  }
public:  //
  OZ_Service(OZ_Propagator * prop, OZ_ParamIterator * iter)
    : _closed(0), _prop(prop), _iter(iter), _nb_actions(0) {}
  //
  // sleep is default, after one of these operations, the object is
  // closed
  void leave(int vars_left = 0) {
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_entailed;
      _actions[_nb_actions]._action_params._vars_left = vars_left;
      _nb_actions += 1;
      OZ_ASSERT(_nb_actions =< _max_actions);
    }
  }
  void entailed(void) {
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_entailed;
      _nb_actions += 1;
      OZ_ASSERT(_nb_actions =< _max_actions);
    }
  }
  void failed(void) {
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_failed;
      _nb_actions += 1;
      OZ_ASSERT(_nb_actions =< _max_actions);
    }
  }
  void equate(OZ_Term x, OZ_Term y) {
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_equate;
      _actions[_nb_actions]._action_params._equat._x = x;
      _actions[_nb_actions]._action_params._equat._y = y;
      _nb_actions += 1;
      OZ_ASSERT(_nb_actions =< _max_actions);
    }
  }
  void add_parameter(OZ_CPIVar &, int event) {
    OZ_ASSERT(0);
  }
  void drop_parameter(OZ_CPIVar &) {
    OZ_ASSERT(0);
  }
  // propagator is set `scheduled'
  void impose_propagator(make_prop_fn_2, OZ_Term, OZ_Term);
  void impose_propagator(make_prop_fn_3, OZ_Term, OZ_Term, OZ_Term);
  void impose_propagator(make_prop_fn_4, OZ_Term, OZ_Term, OZ_Term, OZ_Term);
  // replacing a propagator by another one happens frequently, hence a
  // dedicated fucntion is introduced, not that ununsed parameters
  // have to be passed as arguments and the replacment react on the
  // same events at the respective parameters
  void replace_propagator(OZ_Propagator * prop, int vars_drop,/*OZ_Term*/ ...)
  {
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_replace;
      _actions[_nb_actions]._action_params._replacement = prop;
      _nb_actions += 1;
      OZ_ASSERT(_nb_actions =< _max_actions);
    }
    // add drop parameter code here
  }
  // changes state of propagator, propagator shall
  // not be set `scheduled' (hence, `impose_propagator' does not work)
  void condens_vector(OZ_CPIVarVector &);
  OZ_Return operator ()() {
    OZ_Return r = OZ_ENTAILED;
    int do_leave = 1;
    //
    for (int i = 0; i < _nb_actions; i += 1) {
      _actions_t::_action_params_t &a = _actions[_nb_actions]._action_params;
      //
      if (r = OZ_FAILED) {
        break;
      }
      switch (_actions[_nb_actions]._what) {
      case _actions_t::_serv_failed:
        do_leave = 0;
        r = update_return(r, _iter->fail());
        break;
      case _actions_t::_serv_leave:
        do_leave = 0;
        r = update_return(r, _iter->leave(a._vars_left));
        break;
      case _actions_t::_serv_entailed:
        do_leave = 0;
        r = update_return(r, _iter->vanish());
        break;
      case _actions_t::_serv_replace:
        do_leave = 0;
        _iter->vanish();
        r = update_return(r, _prop->replaceBy(a._replacement));
        break;
      case _actions_t::_serv_equate:
        do_leave = 0;
        _iter->vanish();
        r = update_return(r, _prop->replaceBy(a._equat._x, a._equat._y));
        break;
      default:
        OZ_ASSERT(0);
      }
    }
    if (do_leave) {
      r = update_return(r, _iter->leave());
    }
    return r;
  }
};


//-----------------------------------------------------------------------------

OZ_Return filter_intersection(OZ_Propagator * Prop,
                              OZ_ParamIterator &P,
                              OZ_FSetVar &x, OZ_FSetVar &y, OZ_FSetVar &z)
{
  FSetTouched xt, yt, zt;
  //
  do {
    xt = x;  yt = y;  zt = z;
    //
    if (z->isEmpty()) {
      OZ_DEBUGPRINTTHIS("replace: (z empty)");
      P.vanish();
      return Prop->replaceBy(new FSetDisjointPropagator(x, y));
    }
    if (x->isSubsumedBy(*y)) {
      OZ_DEBUGPRINTTHIS("replace: (x subsumbed by y)");
      P.vanish();
      return OZ_DEBUGRETURNPRINT(Prop->replaceBy(x, z));
    }
    if (y->isSubsumedBy(*x)) {
      OZ_DEBUGPRINTTHIS("replace: (y subsumbed by x)");
      P.vanish();
      return OZ_DEBUGRETURNPRINT(Prop->replaceBy(y, z));
    }
    if (z->isFull()) {
      FailOnInvalid(x->putCard(fs_max_card, fs_max_card));
      FailOnInvalid(y->putCard(fs_max_card, fs_max_card));
      OZ_DEBUGPRINTTHIS("replace: (z = U)");
      return P.vanish();
    }
    //
    FailOnInvalid(*x <= -(- *z & *y)); // lub
    OZ_DEBUGPRINT(("x=%s",x->toString()));
    FailOnInvalid(*y <= -(- *z & *x)); // lub
    OZ_DEBUGPRINT(("y=%s",y->toString()));
    //
    FailOnInvalid(*z <<= (*x & *y)); // glb
    OZ_DEBUGPRINT(("z=%s",z->toString()));
    FailOnInvalid(*x >= *z); // glb
    OZ_DEBUGPRINT(("x=%s",x->toString()));
    FailOnInvalid(*y >= *z); // glb
    OZ_DEBUGPRINT(("y=%s",y->toString()));
    //
  } while (xt <= x || yt <= y || zt <= z);
  //
  return P.leave(1);
  //
 failure:
  return P.fail();
}

#define TMUELLER

#ifdef TMUELLER
OZ_Return FSetIntersectionPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");
  //
  OZ_FSetVar x(_x), y(_y), z(_z);
  PropagatorController_S_S_S P(x, y, z);
  //
  return filter_intersection(this, P, x, y, z);
}
#else
OZ_Return FSetIntersectionPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  OZ_FSetVar x(_x), y(_y), z(_z);
  PropagatorController_S_S_S P(x, y, z);
  FSetTouched xt, yt, zt;

  do {
    xt = x;  yt = y;  zt = z;

    if (z->isEmpty()) {
      OZ_DEBUGPRINTTHIS("replace: (z empty)");
      P.vanish();
      return replaceBy(new FSetDisjointPropagator(_x, _y));
    }
    if (x->isSubsumedBy(*y)) {
      OZ_DEBUGPRINTTHIS("replace: (x subsumbed by y)");
      P.vanish();
      return OZ_DEBUGRETURNPRINT(replaceBy(_x, _z));
    }
    if (y->isSubsumedBy(*x)) {
      OZ_DEBUGPRINTTHIS("replace: (y subsumbed by xy)");
      P.vanish();
      return OZ_DEBUGRETURNPRINT(replaceBy(_y, _z));
    }

    FailOnInvalid(*x <= -(- *z & *y)); // lub
    OZ_DEBUGPRINT(("x=%s",x->toString()));
    FailOnInvalid(*y <= -(- *z & *x)); // lub
    OZ_DEBUGPRINT(("y=%s",y->toString()));

    FailOnInvalid(*z <<= (*x & *y)); // glb
    OZ_DEBUGPRINT(("z=%s",z->toString()));
    FailOnInvalid(*x >= *z); // glb
    OZ_DEBUGPRINT(("x=%s",x->toString()));
    FailOnInvalid(*y >= *z); // glb
    OZ_DEBUGPRINT(("y=%s",y->toString()));

  } while (xt <= x || yt <= y || zt <= z);

  OZ_DEBUGPRINTTHIS("out ");

  return OZ_DEBUGRETURNPRINT(P.leave(1));

failure:
  OZ_DEBUGPRINTTHIS("failed ");
  return P.fail();
}
#endif

OZ_Return FSetUnionPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  OZ_FSetVar x(_x), y(_y), z(_z);
  PropagatorController_S_S_S P(x, y, z);
  FSetTouched xt, yt, zt;

  do {
    xt = x;  yt = y;  zt = z;

    if (z->isEmpty()) {
      OZ_DEBUGPRINT(("z->isEmpty()"));
      OZ_FSetConstraint aux(fs_empty);
      FailOnInvalid(*x <<= aux);
      FailOnInvalid(*y <<= aux);
      P.vanish();
      return OZ_ENTAILED;
    }
    if (x->isSubsumedBy(*y)) {
      OZ_DEBUGPRINT(("x->isSubsumedBy(*y)"));
      P.vanish();
      return replaceBy(_y, _z);
    }
    if (y->isSubsumedBy(*x)) {
      OZ_DEBUGPRINT(("y->isSubsumedBy(*x)"));
      P.vanish();
      return replaceBy(_x, _z);
    }

    OZ_DEBUGPRINT(("before"));
    FailOnInvalid(*x >= (*z & - *y)); // glb
    OZ_DEBUGPRINT(("x=%s",x->toString()));
    FailOnInvalid(*y >= (*z & - *x)); // glb
    OZ_DEBUGPRINT(("y=%s",y->toString()));

    FailOnInvalid(*z <<= (*x | *y)); // lub
    OZ_DEBUGPRINT(("z=%s",z->toString()));
    FailOnInvalid(*x <= *z); // lub
    OZ_DEBUGPRINT(("x=%s",x->toString()));
    FailOnInvalid(*y <= *z); // lub
    OZ_DEBUGPRINT(("y=%s",y->toString()));

  } while (xt <= x || yt <= y || zt <= z);

  OZ_DEBUGPRINTTHIS("out ");
  return P.leave(1);

failure:
  OZ_DEBUGPRINTTHIS("failed ");
  return P.fail();
}

OZ_Return FSetSubsumePropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("int ");
  OZ_FSetVar x(_x), y(_y);
  PropagatorController_S_S P(x, y);

  FailOnInvalid(*x <= *y);
  FailOnInvalid(*y >= *x);

  OZ_DEBUGPRINTTHIS("out ");
  return P.leave(1); /* is entailed if only
                        one var is left */

failure:
  OZ_DEBUGPRINTTHIS("failed ");
  return P.fail();
}

OZ_Return FSetDisjointPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  OZ_FSetVar x(_x), y(_y);
  PropagatorController_S_S P(x, y);

  FSetTouched xt, yt;

  do {
    xt = x;  yt = y;

    FailOnInvalid(*x != *y);
    FailOnInvalid(*y != *x);

  } while (xt <= x || yt <= y);

  OZ_DEBUGPRINTTHIS("out ");
  return  OZ_DEBUGRETURNPRINT(P.leave(1));
  /* is entailed if only one var is left */

failure:
  OZ_DEBUGPRINTTHIS("failed ");
  return P.fail();
}

OZ_Return FSetDistinctPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");
  OZ_FSetVar x(_x), y(_y);
  PropagatorController_S_S P(x, y);

  // cardinality differs
  if (x->getCardMax() < y->getCardMin() ||
      y->getCardMax() < x->getCardMin()) {
    return P.vanish();
  }

  // glb(x) is_not_subseteq lub(y) -> entailed
  {
    OZ_FSetValue glb_x = x->getGlbSet();
    OZ_FSetValue lub_y = y->getLubSet();

    if(!(glb_x <= lub_y))
      return P.vanish();
  }

  // glb(y) is_not_subseteq lub(x) -> entailed
  {
    OZ_FSetValue glb_y = y->getGlbSet();
    OZ_FSetValue lub_x = x->getLubSet();

    if(!(glb_y <= lub_x))
      return P.vanish();
  }

  // x is value and x = glb(y) and glb(y) == union({e}, lub(y))
  if (x->isValue()) {
    OZ_FSetValue x_val = x->getGlbSet();
    OZ_FSetValue glb_y = y->getGlbSet();
    int card_glb_y = y->getGlbCard();
    int card_lub_y = y->getLubCard();

    if ((card_glb_y + 1) == card_lub_y) {
      if (x_val.getCard() == 1 && card_lub_y == 1) {
        FailOnInvalid(y->putCard(0, 0));
        return P.vanish();
      } else if ((x_val == glb_y) && (card_glb_y + 1 == card_lub_y)) {
        FailOnInvalid(y->putCard(card_lub_y, card_lub_y));
        return P.vanish();
      }
    }
  }

  // y is value and y = glb(x) and glb(x) == union({e}, lub(x))
  if (y->isValue()) {
    OZ_FSetValue y_val = y->getGlbSet();
    OZ_FSetValue glb_x = x->getGlbSet();
    int card_glb_x = x->getGlbCard();
    int card_lub_x = x->getLubCard();

    if ((card_glb_x + 1) == card_lub_x) {
      if (y_val.getCard() == 1 && card_lub_x == 1) {
        FailOnInvalid(x->putCard(0, 0));
        return P.vanish();
      } else if (y_val == glb_x) {
        FailOnInvalid(x->putCard(card_lub_x, card_lub_x));
        return P.vanish();
      }
    }
  }

  if (x->isValue() && y->isValue() && *x == *y) {
    goto failure;
  }

  return P.leave();

 failure:
  return P.fail();
}

//--------------------------------------------------------------------
// Set Difference Propagator (DENYS)

OZ_Return FSetDiffPropagator::propagate(void)
{
  OZ_FSetVar x(_x),y(_y),z(_z);
  PropagatorController_S_S_S P(x,y,z);
  FSetTouched xt,yt,zt;

  do {
    xt=x; yt=y; zt=z;
    // x-y=z
    //  disjoint(y,z)
    //  union(x,z)=union(y,z)
    //
    // disjoint(y,z)
    //
    FailOnInvalid(*y != *z);
    FailOnInvalid(*z != *y);
    //
    // union(x,y)=union(y,z)
    //  x subset union(y,z)
    //  z = x-y
    //  x supset z
    //  y supset x-z
    //
    FailOnInvalid(*x <= (*y | *z));
    FailOnInvalid(*z <<= (*x & - *y));
    FailOnInvalid(*x >= *z);
    FailOnInvalid(*y >= (*x & - *z));
  } while (xt <= x || yt <= y || zt <= z);

  return P.leave(1);
failure:
  return P.fail();
}

OZ_PropagatorProfile FSetIntersectionPropagator::profile;
OZ_PropagatorProfile FSetUnionPropagator::profile;
OZ_PropagatorProfile FSetSubsumePropagator::profile;
OZ_PropagatorProfile FSetDisjointPropagator::profile;
OZ_PropagatorProfile FSetDistinctPropagator::profile;
OZ_PropagatorProfile FSetDiffPropagator::profile;
