/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __SUM_HH__
#define __SUM_HH__

#include <iostream.h>

#include "std.hh"

//-----------------------------------------------------------------------------

class LinEqPropagator : public Propagator_VI_VD_I {
private:
  static OZ_CFun spawner;
public:
  LinEqPropagator(OZ_Term a, OZ_Term x, OZ_Term c,
		  OZ_Boolean is_lin = OZ_TRUE) 
    : Propagator_VI_VD_I(a, x, c, is_lin) { };
  LinEqPropagator(const Propagator_VI_VD_I_D &o) 
    : Propagator_VI_VD_I(o) {}
  LinEqPropagator(int sz, int sizes[], int single_var[], 
		  int a[], OZ_Term x[], int c) 
    : Propagator_VI_VD_I(sz, sizes, single_var, a, x, c) {}
  LinEqPropagator(OZ_Term x, OZ_Term d)
    : Propagator_VI_VD_I(x, d) {}
  LinEqPropagator(int sz, int * a, OZ_Term * x, int c)
  : Propagator_VI_VD_I(sz, a, x, c) {}

  virtual OZ_Return propagate(void);
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Term getParameters(void) const { return Propagator_VI_VD_I::getParameters(SUM_OP_EQ); }
};

class NonLinEqPropagator : public LinEqPropagator {
public:
  NonLinEqPropagator(OZ_Term a, OZ_Term x, OZ_Term c) 
    : LinEqPropagator(a, x, c, OZ_FALSE) {}
  NonLinEqPropagator(const Propagator_VI_VD_I_D &o) 
    : LinEqPropagator(o) {}
};

//-----------------------------------------------------------------------------

class LinNotEqPropagator : public Propagator_VI_VD_I {
private:
  static OZ_CFun spawner;
public:
  LinNotEqPropagator(OZ_Term a, OZ_Term x, OZ_Term c,
		     OZ_Boolean is_lin = OZ_TRUE) 
    : Propagator_VI_VD_I(a, x, c, is_lin) { };
  LinNotEqPropagator(const Propagator_VI_VD_I_D &o) 
    : Propagator_VI_VD_I(o) {}
  LinNotEqPropagator(OZ_Term x, OZ_Term d)
    : Propagator_VI_VD_I(x, d) {}
  LinNotEqPropagator(int sz, int * a, OZ_Term * x, int c)
  : Propagator_VI_VD_I(sz, a, x, c) {}

  virtual OZ_Return propagate(void);
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Term getParameters(void) const { return Propagator_VI_VD_I::getParameters(SUM_OP_NEQ); }
};

class NonLinNotEqPropagator : public LinNotEqPropagator {
public:
  NonLinNotEqPropagator(OZ_Term a, OZ_Term x, OZ_Term c)
    : LinNotEqPropagator(a, x, c, OZ_FALSE) {}
  NonLinNotEqPropagator(const Propagator_VI_VD_I_D &o) 
    : LinNotEqPropagator(o) {}
};

//-----------------------------------------------------------------------------

class LinLessEqPropagator : public Propagator_VI_VD_I {
private:
  static OZ_CFun spawner;
public:
  LinLessEqPropagator(OZ_Term a, OZ_Term x, OZ_Term c, 
		      OZ_Boolean is_lin = OZ_TRUE) 
    : Propagator_VI_VD_I(a, x, c, is_lin) { };
  LinLessEqPropagator(const Propagator_VI_VD_I_D &o) 
    : Propagator_VI_VD_I(o) {}
  LinLessEqPropagator(int sz, int sizes[], int single_var[], 
		      int a[], OZ_Term x[], int c) 
    : Propagator_VI_VD_I(sz, sizes, single_var, a, x, c) {}
  LinLessEqPropagator(int ax, OZ_Term x, int ay, OZ_Term y, int az,
		      OZ_Term z, int c)
    : Propagator_VI_VD_I(ax, x, ay, y, az, z, c) {}
  LinLessEqPropagator(OZ_Term x, OZ_Term d)
    : Propagator_VI_VD_I(x, d) {}
  LinLessEqPropagator(int sz, int * a, OZ_Term * x, int c)
  : Propagator_VI_VD_I(sz, a, x, c) {}

  virtual OZ_Return propagate(void);
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Term getParameters(void) const { return Propagator_VI_VD_I::getParameters(SUM_OP_LEQ); }
};

class NonLinLessEqPropagator : public LinLessEqPropagator {
public:
  NonLinLessEqPropagator(OZ_Term a, OZ_Term x, OZ_Term c) 
    : LinLessEqPropagator(a, x, c, OZ_FALSE) {}
  NonLinLessEqPropagator(const Propagator_VI_VD_I_D &o) 
    : LinLessEqPropagator(o) {}
};

//-----------------------------------------------------------------------------

class NonLinEqPropagatorP : public Propagator_VI_VVD_I {
private:
  static OZ_CFun spawner;
public:
  NonLinEqPropagatorP(OZ_Term a, OZ_Term x, OZ_Term d) 
    : Propagator_VI_VVD_I(a, x, d) {}

  virtual OZ_Return propagate(void);
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Term getParameters(void) const { return Propagator_VI_VVD_I::getParameters(SUM_OP_EQ); }
};

//-----------------------------------------------------------------------------

class NonLinLessEqPropagatorP : public Propagator_VI_VVD_I {
private:
  static OZ_CFun spawner;
public:
  NonLinLessEqPropagatorP(OZ_Term a, OZ_Term x, OZ_Term d) 
    : Propagator_VI_VVD_I(a, x, d) {}
  
  virtual OZ_Return propagate(void);
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
  virtual OZ_Term getParameters(void) const { return Propagator_VI_VVD_I::getParameters(SUM_OP_LEQ); }};

//=============================================================================

#define LEQ_TO_GT \
{ reg_c += 1; for (int i = reg_sz; i--; ) reg_a[i] = - reg_a[i]; }

#define LEQ_TO_GEQ \
{ for (int i = reg_sz; i--; ) reg_a[i] = - reg_a[i]; }

#define LEQ_TO_LT \
{ reg_c += 1; }
 
//-----------------------------------------------------------------------------

class SumEqPropagator : public LinEqPropagator {
public:
  SumEqPropagator(OZ_Term x, OZ_Term d) : LinEqPropagator(x, d) {}
};

class SumNeqPropagator : public LinNotEqPropagator {
public:
  SumNeqPropagator(OZ_Term x, OZ_Term d) : LinNotEqPropagator(x, d) {}
};

class SumLeqPropagator : public LinLessEqPropagator {
public:
  SumLeqPropagator(OZ_Term x, OZ_Term d) : LinLessEqPropagator(x, d) {}
};

class SumLtPropagator : public LinLessEqPropagator {
public:
  SumLtPropagator(OZ_Term x, OZ_Term d) 
    : LinLessEqPropagator(x, d) LEQ_TO_LT
};

class SumGeqPropagator : public LinLessEqPropagator {
public:
  SumGeqPropagator(OZ_Term x, OZ_Term d) 
    : LinLessEqPropagator(x, d) LEQ_TO_GEQ
};

class SumGtPropagator : public LinLessEqPropagator {
public:
  SumGtPropagator(OZ_Term x, OZ_Term d) 
    : LinLessEqPropagator(x, d) LEQ_TO_GT
};

//-----------------------------------------------------------------------------

class SumCEqPropagator : public LinEqPropagator {
public:
  SumCEqPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : LinEqPropagator(a, x, d) {}
};

class SumCNeqPropagator : public LinNotEqPropagator {
public:
  SumCNeqPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : LinNotEqPropagator(a, x, d) {}
};

class SumCLeqPropagator : public LinLessEqPropagator {
public:
  SumCLeqPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : LinLessEqPropagator(a, x, d) {}
};

class SumCLtPropagator : public LinLessEqPropagator {
public:
  SumCLtPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : LinLessEqPropagator(a, x, d) LEQ_TO_LT
};

class SumCGeqPropagator : public LinLessEqPropagator {
public:
  SumCGeqPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : LinLessEqPropagator(a, x, d) LEQ_TO_GEQ
};

class SumCGtPropagator : public LinLessEqPropagator {
public:
  SumCGtPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : LinLessEqPropagator(a, x, d) LEQ_TO_GT
};

//-----------------------------------------------------------------------------

class SumCN_EqPropagator : public NonLinEqPropagatorP {
public:
  SumCN_EqPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : NonLinEqPropagatorP(a, x, d) {}
};

// has to wait till it becomes linear
class SumCNNeqPropagator : public LinNotEqPropagator {
public:
  SumCNNeqPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : LinNotEqPropagator(a, x, d, OZ_FALSE) {}
};

class SumCNLeqPropagator : public NonLinLessEqPropagatorP {
public:
  SumCNLeqPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : NonLinLessEqPropagatorP(a, x, d) {}
};

class SumCNLtPropagator : public NonLinLessEqPropagatorP {
public:
  SumCNLtPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : NonLinLessEqPropagatorP(a, x, d) LEQ_TO_LT
};

class SumCNGeqPropagator : public NonLinLessEqPropagatorP {
public:
  SumCNGeqPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : NonLinLessEqPropagatorP(a, x, d) LEQ_TO_GEQ
};

class SumCNGtPropagator : public NonLinLessEqPropagatorP {
public:
  SumCNGtPropagator(OZ_Term a, OZ_Term x, OZ_Term d) 
    : NonLinLessEqPropagatorP(a, x, d) LEQ_TO_GT
};

#endif // __SUM_HH__
