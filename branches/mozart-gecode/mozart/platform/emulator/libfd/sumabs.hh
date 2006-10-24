/*
 *  Authors:
 *    Thorsten Oelgart (oelgart@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

#ifndef __SUMABS_HH__
#define __SUMABS_HH__

#include "std.hh"
#include "auxcomp.hh"

class LinEqAbsPropagator : public Propagator_VI_VD_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  LinEqAbsPropagator(OZ_Term a, OZ_Term x, OZ_Term d)
    : Propagator_VI_VD_D(a, x, d) {}
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {return &profile;}
  virtual OZ_Term getParameters(void) const {
    return Propagator_VI_VD_D::getParametersC(SUM_OP_EQ);
  }
};

class LinLessEqAbsPropagator : public Propagator_VI_VD_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  LinLessEqAbsPropagator(OZ_Term a, OZ_Term x, OZ_Term d)
    : Propagator_VI_VD_D(a, x, d) {}
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {return &profile;}
  virtual OZ_Term getParameters(void) const { 
    return Propagator_VI_VD_D::getParametersC(SUM_OP_LEQ);
  }
};

class LinGreaterEqAbsPropagator : public Propagator_VI_VD_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  LinGreaterEqAbsPropagator(OZ_Term a, OZ_Term x, OZ_Term d)
    : Propagator_VI_VD_D(a, x, d) {}
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {return &profile;}
  virtual OZ_Term getParameters(void) const { 
    return Propagator_VI_VD_D::getParametersC(SUM_OP_EQ);
  }
};

class LinNotEqAbsPropagator : public Propagator_VI_VD_D {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  LinNotEqAbsPropagator(OZ_Term a, OZ_Term x, OZ_Term d)
    : Propagator_VI_VD_D(a, x, d) {}
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {return &profile;}
  virtual OZ_Term getParameters(void) const { 
    return Propagator_VI_VD_D::getParametersC(SUM_OP_NEQ);
  }
};

//-----------------------------------------------------------------------------

class SumACEqPropagator : public LinEqAbsPropagator {
public:
  SumACEqPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : LinEqAbsPropagator(a, x, d) {}
};


class SumACLessEqPropagator : public LinLessEqAbsPropagator {
public:
  SumACLessEqPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : LinLessEqAbsPropagator(a, x, d) {}
};

class SumACLessPropagator : public LinLessEqAbsPropagator {
public:
  SumACLessPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : LinLessEqAbsPropagator(a, x, d) { reg_c += 1; }
};

class SumACNotEqPropagator : public LinNotEqAbsPropagator {
public:
  SumACNotEqPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : LinNotEqAbsPropagator(a, x, d) {}
};

class SumACGreaterEqPropagator : public LinGreaterEqAbsPropagator {
public:
  SumACGreaterEqPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : LinGreaterEqAbsPropagator(a, x, d) {}
};

class SumACGreaterPropagator : public LinGreaterEqAbsPropagator {
public:
  SumACGreaterPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : LinGreaterEqAbsPropagator(a, x, d) { reg_c += 1; }
};

#endif

