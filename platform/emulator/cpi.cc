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

#include "oz_cpi.hh"

typedef double ri_float;

class RealInterval;

//-----------------------------------------------------------------------------

class RealIntervalProfile : public OZ_GenConstraintProfile {

  friend class RealInterval;

private:
  ri_float _l, _u;
public:
  RealIntervalProfile(RealInterval * ri);
};


//-----------------------------------------------------------------------------
// class RealInterval

class RealInterval : public OZ_GenConstraint {

friend class RealIntervalProfile;

private:
  const ri_float _epsilon = 0.001;

  ri_float _l, _u;

public:
  RealInterval(ri_float l, ri_float u) : _l(l), _u(u) {}
  RealInterval(ri_float f) : _l(f), _u(f) {}

  virtual OZ_Boolean isValue(void) {
    return _u - _l < _epsilon;
  }

  virtual OZ_Term toValue(void) {
    double val = _u + _l / 2.0;
    return OZ_float(val);
  }

  virtual OZ_Boolean isValid(void) {
    return _l <= _u;
  }

  OZ_Boolean isWeakerThan(RealInterval * r) {
    return (r->_u - r->_l) < (_u - _l);
  }

  RealInterval * unify(RealInterval * r) {
    return (RealInterval *) NULL; // TMUELLER;
  }

  virtual OZ_Boolean unify(OZ_Term rvt) {
    double rv = OZ_floatToC(rvt);

    return (_l <= rv) && (rv <= _u);
  }

  virtual size_t sizeOf(void) {
    return sizeof(RealInterval);
  }

  RealIntervalProfile * getProfile(void) {
    static RealIntervalProfile rip(this);
    return &rip;
  }

  virtual
  OZ_GenWakeUpDescriptor getWakeUpDescriptor(OZ_GenConstraintProfile * p) {
    OZ_GenWakeUpDescriptor d;
    return d;
  }
};

//-----------------------------------------------------------------------------
// class RealIntervalDefinition

class RealIntervalDefinition : public OZ_GenDefinition {
public:
  virtual int getKind(void) { return 1; }
  virtual int getNoOfWakeUpLists(void) { return 2; }
  /*
  virtual OZ_GenConstraint * toConstraint(OZ_Term f) {
    static RealInterval r(f);
    return &r;
  }
  */
};

//-----------------------------------------------------------------------------
// Implementation

RealIntervalProfile::RealIntervalProfile(RealInterval * ri)
{
  _l = ri->_l;
  _u = ri->_u;
}



// End of File
//-----------------------------------------------------------------------------
