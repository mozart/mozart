#ifndef __FILTER_HH_
#define __FILTER_HH_

//#define FILTER_DEBUG

#ifdef FILTER_DEBUG

#define DSP(A) printf A

#else

#define DSP(A)

#endif

//-----------------------------------------------------------------------------

#define FailOnInvalidTouched(R, V, E)           \
{                                               \
  FSetTouched __t = V;                          \
  if(!(E)) goto failure;                        \
  R |= (__t <= V);                              \
}

//-----------------------------------------------------------------------------

template <class T>
class CPIVector {
private:
  T * _vector;
  int _size;
  OZ_Term ** _ot_vector;
public:
  CPIVector(int size, T * vector, OZ_Term ** ot_vector)
    : _size(size), _vector(vector), _ot_vector(ot_vector) { }
  T & operator [] (int i) { return _vector[i]; }
  int getHigh(void) { return _size; }
  OZ_Term getOzTermVector(void) {
    OZ_Term r = OZ_nil();
    for (int i  = _size; i--; ) {
      if (! _vector[i].is_dropped()) {
        r = OZ_cons((*_ot_vector)[i], r);
      }
    }
    return r;
  }
  int * find_equals(void) {
    return OZ_findEqualVars(_size, *_ot_vector);
  }
};

typedef CPIVector<OZ_FSetVar> OZ_FSetVarVector;
typedef CPIVector<OZ_FDIntVar> OZ_FDIntVarVector;
typedef CPIVector<OZ_CtVar> OZ_CtVarVector;

//-----------------------------------------------------------------------------
// OZ_Filter

class OZ_Filter {
public:
  virtual int hasState(void) = 0;
};


//-----------------------------------------------------------------------------
// OZ_Service

#include <stdarg.h>

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
    } else {
      OZ_ASSERT(0);
    }
  }
public:  //
  OZ_Service(OZ_Propagator * prop, OZ_ParamIterator * iter)
    : _closed(0), _prop(prop), _iter(iter), _nb_actions(0) {}
  //
  // sleep is default, after one of these operations, the object is
  // closed
  OZ_Service &leave(int vars_left = 0) {
    DSP(("request leave\n"));
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_leave;
      _actions[_nb_actions]._action_params._vars_left = vars_left;
      _nb_actions += 1;
      OZ_ASSERT(_nb_actions =< _max_actions);
    }
    _closed = 1;
    return *this;
  }
  OZ_Service &entail(void) {
    DSP(("request entail\n"));
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_entailed;
      _nb_actions += 1;
      OZ_ASSERT(_nb_actions =< _max_actions);
    }
    _closed = 1;
    return *this;
  }
  OZ_Service &fail(void) {
    DSP(("request fail\n"));
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_failed;
      _nb_actions += 1;
      OZ_ASSERT(_nb_actions =< _max_actions);
    }
    _closed = 1;
    return *this;
  }
  OZ_Service &equate(OZ_Term x, OZ_Term y) {
    DSP(("request equate %x %x\n", x, y));
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_equate;
      _actions[_nb_actions]._action_params._equat._x = x;
      _actions[_nb_actions]._action_params._equat._y = y;
      _nb_actions += 1;
      OZ_ASSERT(_nb_actions =< _max_actions);
    }
    _closed = 1;
    return *this;
  }
  OZ_Service &add_parameter(OZ_CPIVar &, int event) {
    OZ_ASSERT(0);
    return *this;
  }
  OZ_Service &drop_parameter(OZ_CPIVar &) {
    OZ_ASSERT(0);
    return *this;
  }
  // propagator is set `scheduled'
  OZ_Service &impose_propagator(make_prop_fn_2,
                                OZ_Term, OZ_Term);
  OZ_Service &impose_propagator(make_prop_fn_3,
                                OZ_Term, OZ_Term, OZ_Term);
  OZ_Service &impose_propagator(make_prop_fn_4,
                                OZ_Term, OZ_Term, OZ_Term, OZ_Term);
  // replacing a propagator by another one happens frequently, hence a
  // dedicated fucntion is introduced, not that ununsed parameters
  // have to be passed as arguments and the replacment react on the
  // same events at the respective parameters
  OZ_Service &replace_propagator(OZ_Propagator * prop,
                                 int vars_drop = 0, /* (OZ_CPIVar *) */ ...)
  {
    DSP(("request replace\n"));
    if (!_closed) {
      _actions[_nb_actions]._what = _actions_t::_serv_replace;
      _actions[_nb_actions]._action_params._replacement = prop;
      _nb_actions += 1;
      OZ_ASSERT(_nb_actions =< _max_actions);
    }
    //
    va_list ap;
    va_start(ap, vars_drop);
    for (int i = vars_drop; i--; ) {
      OZ_CPIVar * cpivar = va_arg(ap, OZ_CPIVar *);
      cpivar->dropParameter();
    }
    //
    _closed = 1;
    return *this;
  }
  // changes state of propagator, propagator shall
  // not be set `scheduled' (hence, `impose_propagator' does not work)
  void condens_vector(OZ_FDIntVarVector &);
  void condens_vector(OZ_FSetVarVector &);
  void condens_vector(OZ_CtVarVector &);
  OZ_Return operator ()() {
    DSP(("OZ_Service %d\n", _nb_actions));
    OZ_Return r = OZ_ENTAILED;
    int do_leave = 1;
    //
    for (int i = 0; i < _nb_actions; i += 1) {
      _actions_t::_action_params_t &a = _actions[i]._action_params;
      //
      if (r = OZ_FAILED) {
        break;
      }
      switch (_actions[i]._what) {
      case _actions_t::_serv_failed:
        DSP(("\tfailed\n"));
        do_leave = 0;
        r = _iter->fail();
        break;
      case _actions_t::_serv_leave:
        DSP(("\tleave\n"));
        do_leave = 0;
        r = _iter->leave(a._vars_left);
        break;
      case _actions_t::_serv_entailed:
        DSP(("\tentailed\n"));
        do_leave = 0;
        r = _iter->vanish();
        break;
      case _actions_t::_serv_replace:
        DSP(("\tleave\n"));
        do_leave = 0;
        _iter->vanish();
        r = _prop->replaceBy(a._replacement);
        break;
      case _actions_t::_serv_equate:
        DSP(("\tequate\n"));
        do_leave = 0;
        _iter->vanish();
        DSP(("request equate %x %x\n", a._equat._x, a._equat._y));
        r = _prop->replaceBy(a._equat._x, a._equat._y);
        break;
      default:
        OZ_ASSERT(0);
      }
    }
    if (do_leave) {
      r = _iter->leave();
    }
    return r;
  }
};

//-----------------------------------------------------------------------------

#endif
