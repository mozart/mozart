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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __RI_HH__
#define __RI_HH__

#include "misc.hh"

//-----------------------------------------------------------------------------

#define INIT_FUNC_RI void module_init_ri(void)
#define INIT_FUNC_LP void module_init_lp(void)

INIT_FUNC_RI;
INIT_FUNC_LP;

OZ_BI_proto(ri_newVar);
OZ_BI_proto(ri_declVar);
OZ_BI_proto(ri_setPrecision);
OZ_BI_proto(ri_getLowerBound);
OZ_BI_proto(ri_getUpperBound);
OZ_BI_proto(ri_getWidth);
OZ_BI_proto(ri_getInf);
OZ_BI_proto(ri_getSup);
OZ_BI_proto(ri_lessEq);
OZ_BI_proto(ri_lessEqMeta);
OZ_BI_proto(ri_greater);
OZ_BI_proto(ri_intBounds);
OZ_BI_proto(ri_intBoundsSPP);
OZ_BI_proto(ri_plus);
OZ_BI_proto(ri_times);

//-----------------------------------------------------------------------------
// auxiliary functions and other stuff

extern ri_float ri_precision;

inline
ri_float ri_nextSmaller(ri_float f)
{
  return nextafter(f, f-1.0);
}

inline
ri_float ri_nextLarger(ri_float f)
{
  return nextafter(f, f+1.0);
}

inline
ri_float min_ri(ri_float a, ri_float b)
{
  return a < b ? a : b;
}

inline
ri_float max_ri(ri_float a, ri_float b)
{
  return a > b ? a : b;
}

inline
ri_float min_ri(ri_float a, ri_float b, ri_float c, ri_float d)
{
  return min_ri(min_ri(a, b), min_ri(c, d));
}

inline
ri_float max_ri(ri_float a, ri_float b, ri_float c, ri_float d)
{
  return max_ri(max_ri(a, b), max_ri(c, d));
}

class RI;

//-----------------------------------------------------------------------------
//

class RIWakeUp : public OZ_CtWakeUp {
public:
  static OZ_CtWakeUp wakeupMin(void) {
    OZ_CtWakeUp ri_wakeup_min;
    ri_wakeup_min.init();
    ri_wakeup_min.setWakeUp(0);
    return ri_wakeup_min;
  }

  static OZ_CtWakeUp wakeupMax(void) {
    OZ_CtWakeUp ri_wakeup_max;
    ri_wakeup_max.init();
    ri_wakeup_max.setWakeUp(1);
    return ri_wakeup_max;
  }

  static OZ_CtWakeUp wakeupMinMax(void) {
    OZ_CtWakeUp ri_wakeup_minmax;
    ri_wakeup_minmax.init();
    ri_wakeup_minmax.setWakeUp(0);
    ri_wakeup_minmax.setWakeUp(1);
    return ri_wakeup_minmax;
  }
};

//-----------------------------------------------------------------------------

class RIProfile : public OZ_CtProfile {

  friend class RI;

private:
  ri_float _l, _u;

public:

  RIProfile(void) {}
  virtual void init(OZ_Ct *);

};


//-----------------------------------------------------------------------------
// class RI

class RI : public OZ_Ct {

friend class RIProfile;

private:
  ri_float _l, _u;

public:
  RI(ri_float l, ri_float u) : _l(l), _u(u) {}

  RI(ri_float f) : _l(f), _u(f) {}

  RI(void) {}

  void init(ri_float l, ri_float u) {
    _l = l;
    _u = u;
  }

  void init(OZ_Term t)
  {
    double d = OZ_floatToC(t);

    _l = _u = d;
  }

  virtual OZ_Ct * copy(void) {
    RI * ri = new RI(_l, _u);
    return ri;
  }

  ri_float getWidth(void) {
    return _u - _l;
  }

  virtual OZ_Boolean isValue(void) {
    return (getWidth() < ri_precision);
  }

  virtual OZ_Term toValue(void) {
    double val = (_u + _l) / 2.0;
    return OZ_float(val);
  }

  virtual OZ_Boolean isValid(void) {
    return _l <= _u;
  }

  virtual OZ_Boolean isEmpty(void) {
    return _l > _u;
  }

  OZ_Boolean isWeakerThan(OZ_Ct * r) {
    RI * ri = (RI *) r;
    //    return (ri->_u - ri->_l) < (_u - _l);
    return (ri->getWidth() < getWidth());
  }

  OZ_Boolean operator ==(OZ_Ct * r) {
    RI * ri = (RI *) r;
    return (ri->lowerBound() == lowerBound() && 
	    ri->upperBound() == upperBound());
  }

  virtual OZ_Ct * intersectDomains(OZ_Ct * r) {
    RI * x = this;
    RI * y = (RI *) r;
    static RI z;

    z._l = max_ri(x->_l, y->_l);
    z._u = min_ri(x->_u, y->_u);

    return &z;
  }

  virtual OZ_Boolean isInDomain(OZ_Term rvt) {
    if (OZ_isFloat(rvt)) { // TMUELLER: isValidValue
      double rv = OZ_floatToC(rvt);

      return (_l <= rv) && (rv <= _u);
    }
    return 0;
  }

  virtual size_t sizeOf(void) {
    return sizeof(RI);
  }

  virtual RIProfile * getProfile(void) {
    static RIProfile rip;
    rip.init(this);
    return &rip;
  }

  virtual
  OZ_CtWakeUp computeEvents(OZ_Ct * c)
  {
    return computeEvents(((RI *) c)->getProfile());
  }

  virtual
  OZ_CtWakeUp computeEvents(OZ_CtProfile * p)
  {
    OZ_CtWakeUp d;
    d.init();

    RIProfile * rip = (RIProfile *) p;

    if (_l > rip->_l) d.setWakeUp(0);

    if (_u < rip->_u) d.setWakeUp(1);

    return d;
  }

  virtual char * toString(int)
  {
    const unsigned buf_size = 30;
    static char buffer[buf_size];

    //    snprintf(buffer, buf_size,
    sprintf(buffer,
	    "[" RI_FLOAT_FORMAT ", " RI_FLOAT_FORMAT "]", _l, _u);
    /*
    // replace '-' by '~'
    char * ch = buffer-1;
    while (*(++ch))
      if (*ch == '-')
	*ch = '~';
    */
    ASSERT(strlen(buffer) < buf_size);

    return buffer;
  }

  static OZ_Ct * leastConstraint(void)
  {
    static RI ri;
    ri.init(RI_FLOAT_MIN, RI_FLOAT_MAX);
    return &ri;
  }

  static OZ_Boolean isValidValue(OZ_Term f)
  {
#ifdef USE_RI_DOUBLE
    return OZ_isFloat(f);
#else
    if (OZ_isFloat(f)) {
      double d = OZ_floatToC(f);
      return (RI_FLOAT_MIN <= d && d <= RI_FLOAT_MAX);
    }
    return 0;
#endif
  }

  OZ_Boolean isTouched(RIProfile rip)
  {
    return (rip._l < _l) || (rip._u > _u);
  }

  ri_float operator = (ri_float f) {
    if (_l <= f && f <= _u) {
      _l = _u = f;
      return 0.0;
    }
    return -1.0;
  }

  ri_float operator <= (ri_float f) {
    _u = min_ri(_u, f);

    return getWidth();
  }

  ri_float operator < (ri_float f) {
    _u = min_ri(_u, ri_nextSmaller(f));

    return getWidth();
  }

  ri_float operator >= (ri_float f){
    _l = max_ri(_l, f);

    return getWidth();
  }

  ri_float operator > (ri_float f) {
    _l = max_ri(_l, ri_nextLarger(f));

    return getWidth();
  }

  ri_float lowerBound(void) { return _l; }

  ri_float upperBound(void) { return _u; }

  OZ_Boolean containsZero(void) { return ((_l <= 0) && (0 <= _u)); }
};

//-----------------------------------------------------------------------------
// class RIDefinition

class RIDefinition : public OZ_CtDefinition {

  friend INIT_FUNC_RI;
  friend INIT_FUNC_LP;

private:

  static int _kind;
public:
  virtual int getId(void) { return _kind; }
  virtual char * getName(void) { return "real interval"; }
  virtual int getNoEvents(void) { return 2; }
  virtual char ** getEventNames(void);

  virtual OZ_Ct * fullDomain(void) {
    return RI::leastConstraint();
  }

  virtual OZ_Boolean isValueOfDomain(OZ_Term f) {
    return RI::isValidValue(f);
  }

};

extern RIDefinition * ri_definition;

//-----------------------------------------------------------------------------
// class RIVar

class RIVar : public OZ_CtVar {
private:

  RI * _ref;
  RI _copy, _encap;

  RIProfile _rip;

protected:

  virtual void ctSetValue(OZ_Term t)
  {
    _copy.init(t);
    _ref = &_copy;
  }

  virtual OZ_Ct * ctRefConstraint(OZ_Ct * c)
  {
    return _ref = (RI *) c;
  }

  virtual OZ_Ct * ctSaveConstraint(OZ_Ct * c)
  {
    _copy = *(RI *) c;
    return &_copy;
  }

  virtual OZ_Ct * ctSaveEncapConstraint(OZ_Ct * c)
  {
    _encap = *(RI *) c;
    return &_encap;
  }

  virtual void ctRestoreConstraint(void)
  {
    *_ref = _copy;
  }

  virtual void ctSetConstraintProfile(void)
  {
    _rip = *_ref->getProfile();
  }

  virtual OZ_CtProfile * ctGetConstraintProfile(void)
  {
    return &_rip;
  }

  virtual OZ_Ct * ctGetConstraint(void)
  {
    return _ref;
  }

public:

  RIVar(void) : OZ_CtVar() { }

  RIVar(OZ_Term t) : OZ_CtVar() { read(t); }

  virtual OZ_Boolean isTouched(void) const
  {
    return _ref->isTouched(_rip);
  }

  RI &operator * (void) { return *_ref; }
  RI * operator -> (void) { return _ref; }

};

//-----------------------------------------------------------------------------
// type-checking parameters

class RIExpect;

typedef OZ_expect_t (RIExpect::*PropagatorExpectMeth) (OZ_Term);

class RIExpect : public OZ_Expect {
public:
  OZ_expect_t expectRIVarMin(OZ_Term t) {
    return expectGenCtVar(t, ri_definition,
			  RIWakeUp::wakeupMin());
  }
  OZ_expect_t expectRIVarMax(OZ_Term t) {
    return expectGenCtVar(t, ri_definition,
			  RIWakeUp::wakeupMax());
  }
  OZ_expect_t expectRIVarMinMax(OZ_Term t) {
    return expectGenCtVar(t, ri_definition,
			  RIWakeUp::wakeupMinMax());
  }

  OZ_expect_t expectVector(OZ_Term t, PropagatorExpectMeth expectf) {
    return OZ_Expect::expectVector(t, (OZ_ExpectMeth) expectf);
  }
  OZ_expect_t expectVectorFloat(OZ_Term t) {
    return expectVector(t, (PropagatorExpectMeth) &OZ_Expect::expectFloat);
  }
  OZ_expect_t expectVectorRIVarMinMax(OZ_Term t) {
    return expectVector(t,  &RIExpect::expectRIVarMinMax);
  }
  OZ_expect_t expectIntVarMinMax(OZ_Term t) {
    return expectIntVar(t, fd_prop_bounds);
  }
};

//-----------------------------------------------------------------------------
// propagator controller

class PropagatorController_RI_RI {
protected:
  RIVar &v1, &v2;
public:
  PropagatorController_RI_RI(RIVar &i1, RIVar &i2)
    : v1(i1), v2(i2) {}

  OZ_Return leave(void) {
    return (v1.leave() | v2.leave()) ? SLEEP : PROCEED;
  }
  OZ_Return vanish(void) {
    v1.leave();
    v2.leave();
    return PROCEED;
  }
  OZ_Return fail(void) {
    v1.fail();
    v2.fail();
    return FAILED;
  }
};

class PropagatorController_RI_RI_RI {
protected:
  RIVar &v1, &v2, &v3;
public:
  PropagatorController_RI_RI_RI(RIVar &i1,
				RIVar &i2,
				RIVar &i3)
    : v1(i1), v2(i2), v3(i3) {}

  OZ_Return leave(void) {
    return (v1.leave() | v2.leave() | v3.leave()) ? SLEEP : PROCEED;
  }
  OZ_Return vanish(void) {
    v1.leave();
    v2.leave();
    v3.leave();
    return PROCEED;
  }
  OZ_Return fail(void) {
    v1.fail();
    v2.fail();
    v3.fail();
    return FAILED;
  }
};

class PropagatorController_VRI {
protected:
  int size;
  RIVar * vv;
public:
  PropagatorController_VRI(int s, RIVar i[]) : size(s), vv(i) {}

  OZ_Return leave(void) {
    OZ_Boolean vars_left = OZ_FALSE;
    for (int i = size; i--; vars_left |= vv[i].leave());
    return vars_left ? SLEEP : PROCEED;
  }
  OZ_Return vanish(void) {
    for (int i = size; i--; vv[i].leave());
    return PROCEED;
  }
   OZ_Return fail(void) {
    for (int i = size; i--; vv[i].fail());
    return FAILED;
  }
};

class PropagatorController_RI_D {
protected:
  RIVar &_ri;
  OZ_FDIntVar &_d;
public:
  PropagatorController_RI_D(RIVar &ri, OZ_FDIntVar &d)
    : _ri(ri), _d(d) {}

  OZ_Return leave(void) {
    return (_ri.leave() | _d.leave()) ? SLEEP : PROCEED;
  }
  OZ_Return vanish(void) {
    _ri.leave();
    _d.leave();
    return PROCEED;
  }
  OZ_Return fail(void) {
    _ri.fail();
    _d.fail();
    return FAILED;
  }
};

//-----------------------------------------------------------------------------
// propagator skeletons

class Propagator_RI_RI : public OZ_Propagator {
protected:

  OZ_Term _x, _y;

public:
  Propagator_RI_RI(OZ_Term x, OZ_Term y) : _x(x), _y(y) {}

  virtual void gCollect(void)
  {
    OZ_gCollectBlock(&_x, &_x, 2);
  }

  virtual void sClone(void)
  {
    OZ_sCloneBlock(&_x, &_x, 2);
  }

  virtual size_t sizeOf(void) { return sizeof(Propagator_RI_RI); }

  virtual OZ_Term getParameters(void) const
  {
    return OZ_cons(_x, OZ_cons(_y, OZ_nil()));
  }
};

class Propagator_RI_RI_RI : public OZ_Propagator {
protected:

  OZ_Term _x, _y, _z;

public:
  Propagator_RI_RI_RI(OZ_Term x, OZ_Term y, OZ_Term z) : _x(x), _y(y), _z(z) {}

  virtual void gCollect(void)
  {
    OZ_gCollectBlock(&_x, &_x, 3);
  }

  virtual void sClone(void)
  {
    OZ_sCloneBlock(&_x, &_x, 3);
  }

  virtual size_t sizeOf(void) { return sizeof(Propagator_RI_RI_RI); }

  virtual OZ_Term getParameters(void) const
  {
    return OZ_cons(_x, OZ_cons(_y, OZ_cons(_z, OZ_nil())));
  }
};

class Propagator_RI_D : public OZ_Propagator {
protected:

  OZ_Term _ri, _d;

public:
  Propagator_RI_D(OZ_Term ri, OZ_Term d) : _ri(ri), _d(d) {}

  virtual void gCollect(void)
  {
    OZ_gCollectBlock(&_ri, &_ri, 2);
  }

  virtual void sClone(void)
  {
    OZ_sCloneBlock(&_ri, &_ri, 2);
  }

  virtual size_t sizeOf(void) { return sizeof(Propagator_RI_D); }

  virtual OZ_Term getParameters(void) const
  {
    return OZ_cons(_ri, OZ_cons(_d, OZ_nil()));
  }
};

#endif /* __RI_HH__ */
