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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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
private:
  static OZ_CFunHeader header;
public:
  LinEqAbsPropagator(OZ_Term a, OZ_Term x, OZ_Term d)
    : Propagator_VI_VD_D(a, x, d) {}
  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const {return &header;}
  virtual OZ_Term getParameters(void) const {
    return Propagator_VI_VD_D::getParameters(SUM_OP_EQ);
  }
};

class LinLessEqAbsPropagator : public Propagator_VI_VD_D {
private:
  static OZ_CFunHeader header;
public:
  LinLessEqAbsPropagator(OZ_Term a, OZ_Term x, OZ_Term d)
    : Propagator_VI_VD_D(a, x, d) {}
  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const {return &header;}
  virtual OZ_Term getParameters(void) const {
    return Propagator_VI_VD_D::getParameters(SUM_OP_LEQ);
  }
};

class LinGreaterEqAbsPropagator : public Propagator_VI_VD_D {
private:
  static OZ_CFunHeader header;
public:
  LinGreaterEqAbsPropagator(OZ_Term a, OZ_Term x, OZ_Term d)
    : Propagator_VI_VD_D(a, x, d) {}
  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const {return &header;}
  virtual OZ_Term getParameters(void) const {
    return Propagator_VI_VD_D::getParameters(SUM_OP_EQ);
  }
};

class LinNotEqAbsPropagator : public Propagator_VI_VD_D {
private:
  static OZ_CFunHeader header;
public:
  LinNotEqAbsPropagator(OZ_Term a, OZ_Term x, OZ_Term d)
    : Propagator_VI_VD_D(a, x, d) {}
  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const {return &header;}
  virtual OZ_Term getParameters(void) const {
    return Propagator_VI_VD_D::getParameters(SUM_OP_NEQ);
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
