/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __CARD_HH__
#define __CARD_HH__

#include "std.hh"

//-----------------------------------------------------------------------------

class LinEqBPropagator : public Propagator_VI_VD_I_D {
private:
  static OZ_CFunHeader spawner;
public:
  LinEqBPropagator(OZ_Term a, OZ_Term x, OZ_Term c, OZ_Term b,
		  OZ_Boolean is_lin = OZ_TRUE) 
    : Propagator_VI_VD_I_D(a, x, c, b, is_lin) { };
  LinEqBPropagator(OZ_Term x, OZ_Term c, OZ_Term b) 
    : Propagator_VI_VD_I_D(x, c, b) { };

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Term getParameters(void) const { return Propagator_VI_VD_I_D::getParameters(SUM_OP_EQ); }
};

class LinNotEqBPropagator : public Propagator_VI_VD_I_D {
private:
  static OZ_CFunHeader spawner;
public:
  LinNotEqBPropagator(OZ_Term a, OZ_Term x, OZ_Term c, OZ_Term b,
		     OZ_Boolean is_lin = OZ_TRUE) 
    : Propagator_VI_VD_I_D(a, x, c, b, is_lin) { };
  LinNotEqBPropagator(OZ_Term x, OZ_Term c, OZ_Term b) 
    : Propagator_VI_VD_I_D(x, c, b) { };

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Term getParameters(void) const { return Propagator_VI_VD_I_D::getParameters(SUM_OP_NEQ); }
};

class LinLessEqBPropagator : public Propagator_VI_VD_I_D {
private:
  static OZ_CFunHeader spawner;
public:
  LinLessEqBPropagator(OZ_Term a, OZ_Term x, OZ_Term c, OZ_Term b, 
		      OZ_Boolean is_lin = OZ_TRUE) 
    : Propagator_VI_VD_I_D(a, x, c, b, is_lin) { };
  LinLessEqBPropagator(OZ_Term x, OZ_Term c, OZ_Term b) 
    : Propagator_VI_VD_I_D(x, c, b) { };

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
  virtual OZ_Term getParameters(void) const { return Propagator_VI_VD_I_D::getParameters(SUM_OP_LEQ); }
};

//=============================================================================

class SumREqPropagator : public LinEqBPropagator {
public:
  SumREqPropagator(OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinEqBPropagator(x, d, r) {}
};

class SumRNeqPropagator : public LinNotEqBPropagator {
public:
  SumRNeqPropagator(OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinNotEqBPropagator(x, d, r) {}
};

class SumRLeqPropagator : public LinLessEqBPropagator {
public:
  SumRLeqPropagator(OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinLessEqBPropagator(x, d, r) {}
};

class SumRLtPropagator : public LinLessEqBPropagator {
public:
  SumRLtPropagator(OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinLessEqBPropagator(x, d, r) 
  {
    reg_c += 1;
  }
};

class SumRGeqPropagator : public LinLessEqBPropagator {
public:
  SumRGeqPropagator(OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinLessEqBPropagator(x, d, r) 
  {
    for (int i = reg_sz; i--; ) reg_a[i] = - reg_a[i];
  }
};

class SumRGtPropagator : public LinLessEqBPropagator {
public:
  SumRGtPropagator(OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinLessEqBPropagator(x, d, r) 
  {
    reg_c += 1;
    for (int i = reg_sz; i--; ) reg_a[i] = - reg_a[i];
  }
};

//-----------------------------------------------------------------------------

class SumCREqPropagator : public LinEqBPropagator {
public:
  SumCREqPropagator(OZ_Term a, OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinEqBPropagator(a, x, d, r) {}
};

class SumCRNeqPropagator : public LinNotEqBPropagator {
public:
  SumCRNeqPropagator(OZ_Term a, OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinNotEqBPropagator(a, x, d, r) {}
};

class SumCRLeqPropagator : public LinLessEqBPropagator {
public:
  SumCRLeqPropagator(OZ_Term a, OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinLessEqBPropagator(a, x, d, r) {}
};

class SumCRLtPropagator : public LinLessEqBPropagator {
public:
  SumCRLtPropagator(OZ_Term a, OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinLessEqBPropagator(a, x, d, r) 
  {
    reg_c += 1;
  }
};

class SumCRGeqPropagator : public LinLessEqBPropagator {
public:
  SumCRGeqPropagator(OZ_Term a, OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinLessEqBPropagator(a, x, d, r) 
  {
    for (int i = reg_sz; i--; ) reg_a[i] = - reg_a[i];
  }
};

class SumCRGtPropagator : public LinLessEqBPropagator {
public:
  SumCRGtPropagator(OZ_Term a, OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinLessEqBPropagator(a, x, d, r) 
  {
    reg_c += 1;
    for (int i = reg_sz; i--; ) reg_a[i] = - reg_a[i];
  }
};

//-----------------------------------------------------------------------------

class SumCNREqPropagator : public LinEqBPropagator {
public:
  SumCNREqPropagator(OZ_Term a, OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinEqBPropagator(a, x, d, r, OZ_FALSE) {}
};

class SumCNRNeqPropagator : public LinNotEqBPropagator {
public:
  SumCNRNeqPropagator(OZ_Term a, OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinNotEqBPropagator(a, x, d, r, OZ_FALSE) {}
};

class SumCNRLeqPropagator : public LinLessEqBPropagator {
public:
  SumCNRLeqPropagator(OZ_Term a, OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinLessEqBPropagator(a, x, d, r, OZ_FALSE) {}
};

class SumCNRLtPropagator : public LinLessEqBPropagator {
public:
  SumCNRLtPropagator(OZ_Term a, OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinLessEqBPropagator(a, x, d, r, OZ_FALSE) 
  {
    reg_c += 1;
  }
};

class SumCNRGeqPropagator : public LinLessEqBPropagator {
public:
  SumCNRGeqPropagator(OZ_Term a, OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinLessEqBPropagator(a, x, d, r, OZ_FALSE) 
  {
    for (int i = reg_sz; i--; ) reg_a[i] = - reg_a[i];
  }
};

class SumCNRGtPropagator : public LinLessEqBPropagator {
public:
  SumCNRGtPropagator(OZ_Term a, OZ_Term x, OZ_Term d, OZ_Term r) 
    : LinLessEqBPropagator(a, x, d, r, OZ_FALSE) 
  {
    reg_c += 1;
    for (int i = reg_sz; i--; ) reg_a[i] = - reg_a[i];
  }
};

//-----------------------------------------------------------------------------

class InBPropagator : public  Propagator_D_FD_D {
private:
  static OZ_CFunHeader spawner;
public:
  InBPropagator(OZ_Term v, OZ_Term d, OZ_Term b) 
    : Propagator_D_FD_D(v, d, b) {}

  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

//-----------------------------------------------------------------------------

class CardBPropagator : public Propagator_VD_D_D_D {
private:
  static OZ_CFunHeader spawner;
public: 
  CardBPropagator(OZ_Term v, OZ_Term l, OZ_Term u, OZ_Term b)
    : Propagator_VD_D_D_D(v, l, u,b) {}
  
  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};

//-----------------------------------------------------------------------------

#endif // __CARD_HH__

