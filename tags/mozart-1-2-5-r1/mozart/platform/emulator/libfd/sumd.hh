/*
 *  Authors:
 *    Markus Loeckelt (loeckelt@ps.uni-sb.de)
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

#include "std.hh"

//-----------------------------------------------------------------------------

class isumProp : public Propagator_VI_VD_I {
public: 
  isumProp(OZ_Term a, OZ_Term x, OZ_Term c, OZ_Boolean is_lin = OZ_TRUE) 
    : Propagator_VI_VD_I(a, x, c, is_lin) { };
  isumProp(const Propagator_VI_VD_I_D &o) 
    : Propagator_VI_VD_I(o) {}
  isumProp(int sz, int sizes[], int single_var[], int a[], OZ_Term x[], int c) 
    : Propagator_VI_VD_I(sz, sizes, single_var, a, x, c) {}
  isumProp(OZ_Term x, OZ_Term d)
    : Propagator_VI_VD_I(x, d) {}
  isumProp(int ax, OZ_Term x, int ay, OZ_Term y, int az, OZ_Term z, int c)
    : Propagator_VI_VD_I(ax, x, ay, y, az, z, c) {}

  int getNextInterval(OZ_FDIntVar F, int position, int &lower, int &upper);
  void trimRemainders(int factor, int &lo, int &hi);
  int sum (OZ_FiniteDomain &AuxDom, OZ_FDIntVar var[], int except,  
	    int pos, int losum, int hisum, int typ);
};

class iLinEqProp : public isumProp {
public:
  iLinEqProp(OZ_Term a, OZ_Term x, OZ_Term c, OZ_Boolean is_lin = OZ_TRUE) 
    : isumProp(a, x, c, is_lin) {};
  iLinEqProp(const Propagator_VI_VD_I_D &o) 
    : isumProp(o) {}
  iLinEqProp(int sz, int sizes[], int single_var[], int a[], 
	     OZ_Term x[], int c) 
    : isumProp(sz, sizes, single_var, a, x, c) {}
  iLinEqProp(OZ_Term x, OZ_Term d)
    : isumProp(x, d) {}

  virtual OZ_Return propagate(void);
  virtual OZ_Term getParameters(void) const 
    { return Propagator_VI_VD_I::getParametersC(SUM_OP_EQ); }
};


//-----------------------------------------------------------------------------

class iLinNEqProp : public isumProp {
public:
  iLinNEqProp(OZ_Term a, OZ_Term x, OZ_Term c, OZ_Boolean is_lin = OZ_TRUE) 
    : isumProp(a, x, c, is_lin) {};
  iLinNEqProp(const Propagator_VI_VD_I_D &o) 
    : isumProp(o) {}
  iLinNEqProp(int sz, int sizes[], int single_var[], int a[], 
	     OZ_Term x[], int c) 
    : isumProp(sz, sizes, single_var, a, x, c) {}
  iLinNEqProp(OZ_Term x, OZ_Term d)
    : isumProp(x, d) {}

  virtual OZ_Return propagate(void);
  virtual OZ_Term getParameters(void) const 
    { return Propagator_VI_VD_I::getParametersC(SUM_OP_NEQ); }
};



//=============================================================================

class isumEqProp : public iLinEqProp {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  isumEqProp(OZ_Term x, OZ_Term d) : iLinEqProp(x, d) {}
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

class isumNEqProp : public iLinNEqProp {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  isumNEqProp(OZ_Term x, OZ_Term d) : iLinNEqProp(x, d) {}
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

//-----------------------------------------------------------------------------

class isumcEqProp : public iLinEqProp {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  isumcEqProp(OZ_Term a, OZ_Term x, OZ_Term d) : iLinEqProp(a, x, d) {}
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

class isumcNEqProp : public iLinNEqProp {
  friend INIT_FUNC(fdp_init);
private:
  static OZ_PropagatorProfile profile;
public:
  isumcNEqProp(OZ_Term a, OZ_Term x, OZ_Term d) : iLinNEqProp(a, x, d) {}
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

//-----------------------------------------------------------------------------






