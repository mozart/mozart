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

#if defined(INTERFACE)
#pragma implementation "cpi.hh"
#endif

#include <math.h>

#include "cpi.hh"

CpiHeapClass CpiHeap;

EnlargeableArray<_spawnVars_t> staticSpawnVars(CPIINITSIZE);
EnlargeableArray<_spawnVars_t> staticSpawnVarsProp(CPIINITSIZE);
EnlargeableArray<_spawnVars_t> staticSuspendVars(CPIINITSIZE);

int staticSpawnVarsNumber = 0;
int staticSpawnVarsNumberProp = 0;
int staticSuspendVarsNumber = 0;


#ifdef FDBISTUCK

OZ_Return constraintsSuspendOnVar(OZ_CFun, int, OZ_Term *,
				  OZ_Term * t)
{
  OZ_suspendOn(makeTaggedRef(t));
}

#else

OZ_Return constraintsSuspendOnVar(OZ_CFun f, int a, OZ_Term * x,
				  OZ_Term * t)
{
  OZ_addThread(makeTaggedRef(t), OZ_makeSuspendedThread(f, x, a));
  return PROCEED;
}

#endif

#if defined(OUTLINE)
#define inline
#include "cpi.icc"
#undef inline
#endif

//-----------------------------------------------------------------------------
// temporary stuff

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

#ifdef SOLARIS_IEEE
#include <sunmath.h>
#endif

#include "oz_cpi.hh"

#ifdef DEBUG_CHECK
#define RI_DEBUG_PRINT(ARGS) printf ARGS; fflush(stdout);
#define RI_DEBUG_PRINT_THIS(STR) 		\
   RI_DEBUG_PRINT(("%s%s\n", STR, this->toString()))
#else
#define RI_DEBUG_PRINT(ARGS) 
#define RI_DEBUG_PRINT_THIS(STR) 	
#endif

#define USE_RI_DOUBLE

#ifdef USE_RI_DOUBLE

typedef double ri_float;
#define RI_FLOAT_MIN -DBL_MAX
#define RI_FLOAT_MAX DBL_MAX
#define RI_EPSILON 0.0001
#define RI_FLOAT_FORMAT "%g"

#else

typedef float ri_float;
#define RI_FLOAT_MIN FLT_MIN
#define RI_FLOAT_MAX FLT_MAX
#define RI_EPSILON 0.0001
#define RI_FLOAT_FORMAT "%g"

#endif

#define EM_RI "real interval"

ri_float ri_precision = 1e-6;

ri_float ri_nextSmaller(ri_float f) 
{
  ri_float r = f - ri_precision;
  if (r < f) 
    return r;

  error("ri_nextSmaller: unable to produce result.");
  return 0.0;
}

ri_float ri_nextLarger(ri_float f)
{
  ri_float r = f + ri_precision;
  if (r > f) 
    return r;

  error("ri_nextSmaller: unable to produce result.");
  return 0.0;
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

class RealInterval;

//-----------------------------------------------------------------------------
//

class RealIntervalWakeUpDescriptor : public OZ_GenWakeUpDescriptor {
public:
  static OZ_GenWakeUpDescriptor wakeupMin(void) {
    OZ_GenWakeUpDescriptor ri_wakeup_min;
    ri_wakeup_min.init();
    ri_wakeup_min.setWakeUp(0);
    return ri_wakeup_min;
  }

  static OZ_GenWakeUpDescriptor wakeupMax(void) {
    OZ_GenWakeUpDescriptor ri_wakeup_max;
    ri_wakeup_max.init();
    ri_wakeup_max.setWakeUp(1);
    return ri_wakeup_max;
  }

  static OZ_GenWakeUpDescriptor wakeupMinMax(void) {
    OZ_GenWakeUpDescriptor ri_wakeup_minmax;
    ri_wakeup_minmax.init();
    ri_wakeup_minmax.setWakeUp(0);
    ri_wakeup_minmax.setWakeUp(1);
    return ri_wakeup_minmax;
  }
};

//-----------------------------------------------------------------------------

class RealIntervalProfile : public OZ_GenConstraintProfile {

  friend class RealInterval;

private:
  ri_float _l, _u;

public:
  
  RealIntervalProfile(void) {}
  virtual void init(OZ_GenConstraint *);
  
};


//-----------------------------------------------------------------------------
// class RealInterval

class RealInterval : public OZ_GenConstraint {

friend class RealIntervalProfile;

private:
  ri_float _l, _u;

public:
  RealInterval(ri_float l, ri_float u) : _l(l), _u(u) {}
  RealInterval(ri_float f) : _l(f), _u(f) {}
  RealInterval(void) {}
  
  void init(OZ_Term t)
  {
    double d = OZ_floatToC(t);

    _l = _u = d;
  }

  virtual OZ_GenConstraint * copy(void) {
    RealInterval * ri = new RealInterval(_l, _u);
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

  OZ_Boolean isWeakerThan(OZ_GenConstraint * r) {
    RealInterval * ri = (RealInterval *) r;
    return (ri->_u - ri->_l) < (_u - _l);
  }

  OZ_GenConstraint * unify(OZ_GenConstraint * r) {
    RealInterval * x = this;
    RealInterval * y = (RealInterval *) r;
    static RealInterval z; 

    z._l = max_ri(x->_l, y->_l);
    z._u = min_ri(x->_u, y->_u);

    return &z;
  }

  virtual OZ_Boolean unify(OZ_Term rvt) {
    if (OZ_isFloat(rvt)) {
      double rv = OZ_floatToC(rvt);
      
      return (_l <= rv) && (rv <= _u);
    } 
    return 0;
  }

  virtual size_t sizeOf(void) {
    return sizeof(RealInterval);
  }

  virtual RealIntervalProfile * getProfile(void) {
    static RealIntervalProfile rip;
    rip.init(this);
    return &rip;
  }

  virtual 
  OZ_GenWakeUpDescriptor getWakeUpDescriptor(OZ_GenConstraintProfile * p) 
  {
    OZ_GenWakeUpDescriptor d;
    d.init();
    
    RealIntervalProfile * rip = (RealIntervalProfile *) p;

    if (_l > rip->_l) d.setWakeUp(0);

    if (_u < rip->_u) d.setWakeUp(1);

    return d;
  }

  virtual char * toString(int) 
  {
    const int buf_size = 30;
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
    Assert(strlen(buffer) < buf_size);

    return buffer;
  }

  static OZ_GenConstraint * leastConstraint(void) 
  {
    static RealInterval ri(RI_FLOAT_MIN, RI_FLOAT_MAX);
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

  OZ_Boolean isTouched(RealIntervalProfile rip)
  {
    return (rip._l < _l) || (rip._u > _u);
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
// class RealIntervalDefinition

class RealIntervalDefinition : public OZ_GenDefinition {
public: 
  virtual int getKind(void) { return 1; }
  virtual char * getName(void) { return "real interval"; }
  virtual int getNoOfWakeUpLists(void) { return 2; }
  
  virtual OZ_GenConstraint * toConstraint(OZ_Term f) { 
    static RealInterval r(f); 
    return &r;
  }
  
  virtual OZ_GenConstraint * leastConstraint(void) {
    return RealInterval::leastConstraint();
  }

  virtual OZ_Boolean isValidValue(OZ_Term f) {
    return RealInterval::isValidValue(f);
  }
				  
};

RealIntervalDefinition ri_definition;

//-----------------------------------------------------------------------------
// class RealIntervalVar

class RealIntervalVar : public OZ_GenCtVar {
private:

  RealInterval * _ri_ref;
  RealInterval _ri;

  RealIntervalProfile _rip;

protected:

  virtual void ctSetValue(OZ_Term t)
  {
    _ri.init(t);
    _ri_ref = &_ri;
  }

  virtual OZ_GenConstraint * ctRefConstraint(OZ_GenConstraint * c)
  {
    return _ri_ref = (RealInterval *) c;
  }

  virtual OZ_GenConstraint * ctSaveConstraint(OZ_GenConstraint * c)
  {
    _ri = *(RealInterval *) c;
    return c;
  }

  virtual void ctRestoreConstraint(void)
  {
    *_ri_ref = _ri;
  }
 
  virtual void ctSetConstraintProfile(void) 
  {
    _rip = *_ri_ref->getProfile();
  }

  virtual OZ_GenConstraintProfile * ctGetConstraintProfile(void) 
  {
    return &_rip;
  }

  virtual OZ_GenConstraint * ctGetConstraint(void)
  {
    return _ri_ref;
  }

public:

  RealIntervalVar(void) : OZ_GenCtVar() { }

  RealIntervalVar(OZ_Term t) : OZ_GenCtVar() { read(t); }

  virtual OZ_Boolean isTouched(void) const
  {
    return _ri_ref->isTouched(_rip);
  }

  RealInterval &operator * (void) { return *_ri_ref; }
  RealInterval * operator -> (void) { return _ri_ref; }

};


//-----------------------------------------------------------------------------
// implementation

void RealIntervalProfile::init(OZ_GenConstraint * c)
{
  RealInterval * ri = (RealInterval *) c;
  _l = ri->_l;
  _u = ri->_u;
}

//-----------------------------------------------------------------------------
// built-ins 

OZ_BI_define(ri_mkVar, 3, 0)
{
  OZ_declareFloatIN(0, l);
  OZ_declareFloatIN(1, u);

  if (l > u) 
    return OZ_FAILED;

  RealInterval ri(l, u);
  
  static RealIntervalDefinition def;

  return OZ_mkCtVariable(OZ_in(2), &ri, &def);
}
OZ_BI_end

OZ_BI_define(ri_setPrecision, 1, 0)
{
  OZ_declareFloatIN(0, p);

  ri_precision = p;

  return PROCEED;
}
OZ_BI_end

//
class RealIntervalExpect : public OZ_Expect {
public:
  OZ_expect_t expectRealIntervalVarMin(OZ_Term t) { 
    return expectGenCtVar(t, &ri_definition, 
			  RealIntervalWakeUpDescriptor::wakeupMin()); 
  }
  OZ_expect_t expectRealIntervalVarMax(OZ_Term t) { 
    return expectGenCtVar(t, &ri_definition, 
			  RealIntervalWakeUpDescriptor::wakeupMax()); 
  }
  OZ_expect_t expectRealIntervalVarMinMax(OZ_Term t) { 
    return expectGenCtVar(t, &ri_definition, 
			  RealIntervalWakeUpDescriptor::wakeupMinMax()); 
  }
};

OZ_BI_define(ri_getBounds, 3, 0)
{
  ExpectedTypes(EM_RI ", FLOAT, FLOAT" );

  RealIntervalExpect pe;
  OZ_expect_t r = pe.expectRealIntervalVarMinMax(OZ_in(0));
  if (pe.isFailing(r)) {
    TypeError(0, "");
  } else if (pe.isSuspending(r)) {
    return pe.suspend(NULL);
  }
  
  RealIntervalVar ri;
  
  ri.ask(OZ_in(0));
  
  double l = ri->lowerBound(), u = ri->upperBound();

  return OZ_unifyFloat(OZ_in(1), l) && OZ_unifyFloat(OZ_in(2), u);
}
OZ_BI_end


//-----------------------------------------------------------------------------
// common stuff for propagators


#define FailOnInvalid(X) if((X) < 0.0) goto failure;

#define FailOnInvalidTouched(X, W, F) 		\
{						\
  ri_float _w = (X);				\
  if(_w < 0.0) 					\
    goto failure;				\
  if (_w < W) {					\
    F = 1;					\
    W = _w;					\
  }						\
}

class PropagatorController_RI_RI {
protected:
  RealIntervalVar &v1, &v2;
public:
  PropagatorController_RI_RI(RealIntervalVar &i1, RealIntervalVar &i2) 
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
  RealIntervalVar &v1, &v2, &v3;
public:
  PropagatorController_RI_RI_RI(RealIntervalVar &i1, 
				RealIntervalVar &i2, 
				RealIntervalVar &i3) 
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

class Propagator_RI_RI : public OZ_Propagator {
protected:

  OZ_Term _x, _y;

public:
  Propagator_RI_RI(OZ_Term x, OZ_Term y) : _x(x), _y(y) {}

  virtual void updateHeapRefs(OZ_Boolean)
  {
    OZ_updateHeapTerm(_x);
    OZ_updateHeapTerm(_y);
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

  virtual void updateHeapRefs(OZ_Boolean)
  {
    OZ_updateHeapTerm(_x);
    OZ_updateHeapTerm(_y);
    OZ_updateHeapTerm(_z);
  }

  virtual size_t sizeOf(void) { return sizeof(Propagator_RI_RI_RI); }

  virtual OZ_Term getParameters(void) const 
  {
    return OZ_cons(_x, OZ_cons(_y, OZ_cons(_z, OZ_nil())));
  }

};

//-----------------------------------------------------------------------------
// the actual propagators

class RealIntervalLessEq : public Propagator_RI_RI {
private:

  static OZ_CFunHeader header;

public:
  RealIntervalLessEq(OZ_Term x, OZ_Term y) : Propagator_RI_RI(x, y) {}

  virtual OZ_CFunHeader * getHeader(void) const { return &header; }

  virtual OZ_Return propagate(void);

};

OZ_C_proc_begin(ri_lessEq, 2)
{
  OZ_EXPECTED_TYPE(EM_RI "," EM_RI);

  RealIntervalExpect pe;

  OZ_EXPECT(pe, 0, expectRealIntervalVarMinMax);
  OZ_EXPECT(pe, 1, expectRealIntervalVarMinMax);

  return pe.impose(new RealIntervalLessEq(OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end

OZ_CFunHeader RealIntervalLessEq::header = ri_lessEq;

OZ_Return RealIntervalLessEq::propagate(void)
{
  RI_DEBUG_PRINT_THIS(("IN "));

  // x <= y

  RealIntervalVar x(_x), y(_y);
  PropagatorController_RI_RI P(x, y);
  
  if (x->upperBound() <= y->lowerBound())
    return P.vanish();

  FailOnInvalid(*x <= y->upperBound());

  FailOnInvalid(*y >= x->lowerBound());

  RI_DEBUG_PRINT_THIS(("OUT "));

  return P.leave();

failure:
  RI_DEBUG_PRINT(("FAIL\n"));

  return P.fail();
}

//-----------------------------------------------------------------------------

class RealIntervalGreater : public Propagator_RI_RI {
private:

  static OZ_CFunHeader header;

public:
  RealIntervalGreater(OZ_Term x, OZ_Term y) : Propagator_RI_RI(x, y) {}

  virtual OZ_CFunHeader * getHeader(void) const { return &header; }

  virtual OZ_Return propagate(void);

};

OZ_C_proc_begin(ri_greater, 2)
{
  OZ_EXPECTED_TYPE(EM_RI "," EM_RI);

  RealIntervalExpect pe;

  OZ_EXPECT(pe, 0, expectRealIntervalVarMinMax);
  OZ_EXPECT(pe, 1, expectRealIntervalVarMinMax);

  return pe.impose(new RealIntervalGreater(OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end

OZ_CFunHeader RealIntervalGreater::header = ri_greater;

OZ_Return RealIntervalGreater::propagate(void)
{
  RI_DEBUG_PRINT_THIS(("IN "));

  // x > y

  RealIntervalVar x(_x), y(_y);
  PropagatorController_RI_RI P(x, y);

  if (x->lowerBound() > y->upperBound())
    return P.vanish();

  FailOnInvalid(*x > y->lowerBound());

  FailOnInvalid(*y < x->upperBound());

  RI_DEBUG_PRINT_THIS(("OUT "));

  return P.leave();

failure:
  RI_DEBUG_PRINT(("FAIL\n"));

  return P.fail();
}

//-----------------------------------------------------------------------------

class RealIntervalPlus : public Propagator_RI_RI_RI {
private:

  static OZ_CFunHeader header;

public:
  RealIntervalPlus(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_RI_RI_RI(x, y, z) {}

  virtual OZ_CFunHeader * getHeader(void) const { return &header; }

  virtual OZ_Return propagate(void);

};

OZ_C_proc_begin(ri_plus, 3)
{
  OZ_EXPECTED_TYPE(EM_RI "," EM_RI);

  RealIntervalExpect pe;

  int susp_count = 0;
  
  OZ_EXPECT_SUSPEND(pe, 0, expectRealIntervalVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectRealIntervalVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectRealIntervalVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new RealIntervalPlus(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_CFunHeader RealIntervalPlus::header = ri_plus;

OZ_Return RealIntervalPlus::propagate(void)
{
  RI_DEBUG_PRINT_THIS(("IN "));

  // x + y = z

  RealIntervalVar x(_x), y(_y), z(_z);
  ri_float x_w = x->getWidth(), y_w = y->getWidth(), z_w = z->getWidth();
  int redo;
  PropagatorController_RI_RI_RI P(x, y, z);

  do {
    redo = 0;

    // downwards rounding
#ifdef SOLARIS_IEEE
    ieee_flags("set", "direction", "negative");
#endif
    
    FailOnInvalidTouched(*z >= x->lowerBound() + y->lowerBound(), z_w, redo);
    FailOnInvalidTouched(*x >= z->lowerBound() - y->upperBound(), x_w, redo);
    FailOnInvalidTouched(*y >= z->lowerBound() - x->upperBound(), y_w, redo);
    
    // upwards rounding
#ifdef SOLARIS_IEEE
    ieee_flags("set", "direction", "negative");
#endif
    FailOnInvalidTouched(*z <= x->upperBound() + y->upperBound(), z_w, redo);
    FailOnInvalidTouched(*x <= z->upperBound() - y->lowerBound(), x_w, redo);
    FailOnInvalidTouched(*y <= z->upperBound() - x->lowerBound(), y_w, redo);
  } while (redo);

  RI_DEBUG_PRINT_THIS(("OUT "));

  return P.leave();

failure:
  RI_DEBUG_PRINT(("FAIL\n"));

  return P.fail();
}

//-----------------------------------------------------------------------------

class RealIntervalTimes : public Propagator_RI_RI_RI {
private:

  static OZ_CFunHeader header;

public:
  RealIntervalTimes(OZ_Term x, OZ_Term y, OZ_Term z) 
    : Propagator_RI_RI_RI(x, y, z) {}

  virtual OZ_CFunHeader * getHeader(void) const { return &header; }

  virtual OZ_Return propagate(void);

};

OZ_C_proc_begin(ri_times, 3)
{
  OZ_EXPECTED_TYPE(EM_RI "," EM_RI);

  RealIntervalExpect pe;

  int susp_count = 0;
  
  OZ_EXPECT_SUSPEND(pe, 0, expectRealIntervalVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectRealIntervalVarMinMax, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectRealIntervalVarMinMax, susp_count);

  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new RealIntervalTimes(OZ_args[0], OZ_args[1], OZ_args[2]));
}
OZ_C_proc_end

OZ_CFunHeader RealIntervalTimes::header = ri_times;

OZ_Return RealIntervalTimes::propagate(void)
{
  RI_DEBUG_PRINT_THIS(("IN "));

  // x * y = z

  RealIntervalVar x(_x), y(_y), z(_z);
  ri_float x_w = x->getWidth(), y_w = y->getWidth(), z_w = z->getWidth();
  int redo;
  PropagatorController_RI_RI_RI P(x, y, z);

  do {
    redo = 0;

    // constrain `z'
#ifdef SOLARIS_IEEE
    ieee_flags("set", "direction", "positive");
#endif
        
    ri_float z_min_ri = min_ri(x->lowerBound() * y->lowerBound(),
			       x->lowerBound() * y->upperBound(),
			       x->upperBound() * y->lowerBound(),
			       x->upperBound() * y->upperBound());
    RI_DEBUG_PRINT(("z_min_ri=%f\n", z_min_ri));

    FailOnInvalidTouched(*z >= z_min_ri, z_w, redo);
    
#ifdef SOLARIS_IEEE
    ieee_flags("set", "direction", "negative");
#endif

    ri_float z_max_ri = max_ri(x->lowerBound() * y->lowerBound(),
			       x->lowerBound() * y->upperBound(),
			       x->upperBound() * y->lowerBound(),
			       x->upperBound() * y->upperBound());
    RI_DEBUG_PRINT(("z_max_ri=%f\n", z_max_ri));

    FailOnInvalidTouched(*z <= z_max_ri, z_w, redo);

    // constraint `x' (if possible)
    if (! y->containsZero()) {
      
#ifdef SOLARIS_IEEE
      ieee_flags("set", "direction", "positive");
#endif
      ri_float y_ub_reciprocal = 1.0 / y->upperBound();
      
#ifdef SOLARIS_IEEE
      ieee_flags("set", "direction", "negative");
#endif
      ri_float y_lb_reciprocal = 1.0 / y->lowerBound();
      
#ifdef SOLARIS_IEEE
      ieee_flags("set", "direction", "positive");
#endif
      ri_float x_min_ri = min_ri(z->lowerBound() * y_lb_reciprocal,
				 z->lowerBound() * y_ub_reciprocal,
				 z->upperBound() * y_lb_reciprocal,
				 z->upperBound() * y_ub_reciprocal);
      RI_DEBUG_PRINT(("x_min_ri=%f\n", x_min_ri));
      
      FailOnInvalidTouched(*x >= x_min_ri, x_w, redo);
#ifdef SOLARIS_IEEE
      ieee_flags("set", "direction", "negative");
#endif
      ri_float x_max_ri = max_ri(z->lowerBound() * y_lb_reciprocal,
				 z->lowerBound() * y_ub_reciprocal,
				 z->upperBound() * y_lb_reciprocal,
				 z->upperBound() * y_ub_reciprocal);
      RI_DEBUG_PRINT(("x_max_ri=%f\n", x_max_ri));
	
      FailOnInvalidTouched(*x <= x_max_ri, z_w, redo);
    }

    // constraint `y' (if possible)
    if (! x->containsZero()) {
      
#ifdef SOLARIS_IEEE
      ieee_flags("set", "direction", "positive");
#endif
      ri_float x_ub_reciprocal = 1.0 / x->upperBound();
      
#ifdef SOLARIS_IEEE
      ieee_flags("set", "direction", "negative");
#endif
      ri_float x_lb_reciprocal = 1.0 / x->lowerBound();
      
#ifdef SOLARIS_IEEE
      ieee_flags("set", "direction", "positive");
#endif
      ri_float y_min_ri = min_ri(z->lowerBound() * x_lb_reciprocal,
				 z->lowerBound() * x_ub_reciprocal,
				 z->upperBound() * x_lb_reciprocal,
				 z->upperBound() * x_ub_reciprocal);
      RI_DEBUG_PRINT(("y_min_ri=%f\n", y_min_ri));

      FailOnInvalidTouched(*y >= y_min_ri, y_w, redo);
#ifdef SOLARIS_IEEE
      ieee_flags("set", "direction", "negative");
#endif
      ri_float y_max_ri = max_ri(z->lowerBound() * x_lb_reciprocal,
				 z->lowerBound() * x_ub_reciprocal,
				 z->upperBound() * x_lb_reciprocal,
				 z->upperBound() * x_ub_reciprocal);
      RI_DEBUG_PRINT(("y_max_ri=%f\n", y_max_ri));

      FailOnInvalidTouched(*y <= y_max_ri, z_w, redo);
    }

  } while (redo);

  RI_DEBUG_PRINT_THIS(("OUT "));

  return P.leave();

failure:
  RI_DEBUG_PRINT(("FAIL\n"));

  return P.fail();
}

// End of File
//-----------------------------------------------------------------------------
