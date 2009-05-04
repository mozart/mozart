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

#ifndef __STD_HH__
#define __STD_HH__

#include "fdaux.hh"

//-----------------------------------------------------------------------------

// to be used inside the method 'run'
#define SimplifyOnUnify(EQ01, EQ02, EQ12) 	\
  if (mayBeEqualVars()) { 			\
    if (OZ_isEqualVars(reg_x, reg_y)) { 	\
      return (EQ01); 				\
    } 						\
    if (OZ_isEqualVars(reg_x, reg_z)) { 	\
      return (EQ02); 				\
    } 						\
    if (OZ_isEqualVars(reg_y, reg_z)) { 	\
      return (EQ12); 				\
    }                                           \
  }

#define FailOnEmpty(X) if((X) == 0) goto failure;

//-----------------------------------------------------------------------------

class Propagator_D_D : public OZ_Propagator {
protected:
  OZ_Term reg_x;
  OZ_Term reg_y;
public:
  Propagator_D_D(OZ_Term x, OZ_Term y) : reg_x(x), reg_y(y) {}

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_D_D); }
  virtual OZ_Term getParameters(void) const;
};


//-----------------------------------------------------------------------------

class Propagator_D_D_D : public OZ_Propagator {
protected:
  OZ_Term reg_x, reg_y, reg_z;
public:
  Propagator_D_D_D(OZ_Term x, OZ_Term y, OZ_Term z)
    : reg_x(x), reg_y(y), reg_z(z) {}

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_D_D_D); }
  virtual OZ_Term getParameters(void) const;
};

//-----------------------------------------------------------------------------

class Propagator_D_D_D_I : public Propagator_D_D_D {
protected:
  int reg_c;
public:
  Propagator_D_D_D_I(OZ_Term x, OZ_Term y, OZ_Term z, int c)
    : reg_c(c), Propagator_D_D_D(x, y, z) {}

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_D_D_D_I); }
  virtual OZ_Term getParameters(void) const;
  virtual OZ_Term getParametersC(const char *) const;
};

//-----------------------------------------------------------------------------

class Propagator_D_I_D : public OZ_Propagator {
protected:
  OZ_Term reg_x, reg_z; // Keep this order!
  int reg_y;
public:
  Propagator_D_I_D(OZ_Term x, OZ_Term y, OZ_Term z)
    : reg_x(x), reg_y(OZ_intToC(y)), reg_z(z) {}
  
  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_D_I_D); }
  virtual OZ_Term getParameters(void) const;
};

//-----------------------------------------------------------------------------

class Propagator_D_D_I : public OZ_Propagator {
protected:
  OZ_Term reg_x, reg_y; // Keep this order!
  int reg_c;
public:
  Propagator_D_D_I(OZ_Term x, OZ_Term y, int c)
    : reg_x(x), reg_y(y), reg_c(c) {}

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_D_D_I); }
  virtual OZ_Term getParameters(void) const;
  virtual OZ_Term getParametersC(const char *) const;
};

//-----------------------------------------------------------------------------

class Propagator_D_VD_I : public OZ_Propagator {
protected:
  OZ_Term   reg_n; 
  int       reg_v; 
  int       reg_l_sz;    
  OZ_Term * reg_l;
public:
  Propagator_D_VD_I(OZ_Term, OZ_Term, OZ_Term);
  virtual ~Propagator_D_VD_I(void);
  
  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_D_VD_I); }
  virtual OZ_Term getParameters(void) const;
};

//-----------------------------------------------------------------------------

class Propagator_D_VI_D : public OZ_Propagator {
protected:
  OZ_Term reg_n, reg_v;  // Keep this order!
  int     reg_l_sz;
  int *   reg_l;
public:
  Propagator_D_VI_D(OZ_Term, OZ_Term, OZ_Term);
  virtual ~Propagator_D_VI_D(void);

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_D_VI_D); }
  virtual OZ_Term getParameters(void) const;
};

//-----------------------------------------------------------------------------

class Propagator_VI_VD_I_D;

class Propagator_VI_VD_I : public OZ_Propagator {
protected:
  int reg_c;
  OZ_Term * reg_x;
  int * reg_a;
  int reg_sz;
public:
  Propagator_VI_VD_I(OZ_Term, OZ_Term, OZ_Term, OZ_Boolean);
  Propagator_VI_VD_I(int, OZ_Term, int, OZ_Term, int, OZ_Term, int);
  Propagator_VI_VD_I(OZ_Term, OZ_Term);
  Propagator_VI_VD_I(const Propagator_VI_VD_I_D &);
  Propagator_VI_VD_I(int sz, int sizes[], int single_var[], 
		     int a[], OZ_Term x[], int c);
  virtual ~Propagator_VI_VD_I(void);

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_VI_VD_I); }
  virtual OZ_Term getParametersC(const char *) const;
  
  void simplify_on_equality(void);
  void simplify(void);
  void simplifySingletons(void);
};

inline 
void Propagator_VI_VD_I::simplify(void)
{
  if (reg_sz == 0) return;
  int * is = OZ_findEqualVars(reg_sz, reg_x);
  
  for (int i = 0; i < reg_sz; i += 1) {
    if (is[i] == -1) {        // singleton in reg_x
      reg_c += int(NUMBERCAST((OZ_intToC(reg_x[i])) * reg_a[i]));
      reg_x[i] = 0; 
    } else if (i != is[i]) {  // multiply apprearing var in reg_x 
      reg_a[is[i]] += reg_a[i];
      reg_x[i] = 0;
    } 
  }
  int from = 0, to = 0;
  for (; from < reg_sz; from += 1) {
    if (reg_x[from] == 0 || reg_a[from] == 0) continue;
    if (from != to) {
      reg_a[to] = reg_a[from];
      reg_x[to] = reg_x[from];
    }
    to += 1;
  }
  reg_sz = to;
}

inline 
void Propagator_VI_VD_I::simplifySingletons(void)
{
  if (reg_sz == 0) return;
  int * is = OZ_findSingletons(reg_sz, reg_x);
  
  for (int i = 0; i < reg_sz; i += 1) {
    if (is[i] > 0) {        // singleton in x
      reg_c += int(double(is[i]) * reg_a[i]);
      reg_x[i] = 0; 
    } 
  }
  int from = 0, to = 0;
  for (; from < reg_sz; from += 1) {
    if (reg_x[from] == 0 || reg_a[from] == 0) continue;
    if (from != to) {
      reg_a[to] = reg_a[from];
      reg_x[to] = reg_x[from];
    }
    to += 1;
  }
  reg_sz = to;
}

inline 
void Propagator_VI_VD_I::simplify_on_equality(void)
{
  if (mayBeEqualVars()) {
    simplify();
  } else {
    simplifySingletons();
  }
  OZ_DEBUGCODE(int * is = OZ_findEqualVars(reg_sz, reg_x);
	       for (int i = reg_sz; i--; ) {
	         if (is[i] != i && is[i] != -1) 
	           OZ_warning("Unexpected equal variables");
		 if (reg_a[i] == 0)
		   OZ_warning("Unexpected 0 coefficient.");
	       }
	       );
}

//-----------------------------------------------------------------------------

class Propagator_VI_VD_I_D : public Propagator_VI_VD_I {
protected:
  OZ_Term reg_b;
public:
  Propagator_VI_VD_I_D(OZ_Term a, OZ_Term x, OZ_Term c, OZ_Term b, 
		       OZ_Boolean is_lin) 
    : reg_b(b), Propagator_VI_VD_I(a, x, c, is_lin) {}
  Propagator_VI_VD_I_D(OZ_Term x, OZ_Term c, OZ_Term b) 
    : reg_b(b), Propagator_VI_VD_I(x, c) {}
  
  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_VI_VD_I_D); }
  virtual OZ_Term getParametersC(const char *) const;
};

//-----------------------------------------------------------------------------

class Propagator_VD : public OZ_Propagator {
protected:
  int       reg_l_sz;    
  OZ_Term * reg_l;
public:
  Propagator_VD(OZ_Term);
  virtual ~Propagator_VD(void);

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_VD); }
  virtual OZ_Term getParameters(void) const;

  OZ_Boolean hasEqualVars(void);
};

inline
OZ_Boolean Propagator_VD::hasEqualVars(void) 
{
  return (mayBeEqualVars() && OZ_hasEqualVars(reg_l_sz, reg_l));
}

//-----------------------------------------------------------------------------

class Propagator_VD_VI : public OZ_Propagator {
protected:
  OZ_Term * reg_l;
  int * reg_offset;
  int reg_sz;
public:
  Propagator_VD_VI(OZ_Term, OZ_Term);
  Propagator_VD_VI(int);
  virtual ~Propagator_VD_VI(void);

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_VD_VI); }
  virtual OZ_Term getParameters(void) const;
};

//-----------------------------------------------------------------------------
// cumulative
class Propagator_VD_VI_VI_I : public OZ_Propagator {
protected:
  OZ_Term * reg_l;
  int * reg_offset;
  int reg_sz;
  int * reg_use;
  int reg_capacity;
public:
  Propagator_VD_VI_VI_I(OZ_Term l, OZ_Term offset, OZ_Term use, OZ_Term cap);
  Propagator_VD_VI_VI_I(int);
  virtual ~Propagator_VD_VI_VI_I(void);

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_VD_VI_VI_I); }
  virtual OZ_Term getParameters(void) const;
};

//-----------------------------------------------------------------------------

class Propagator_VI_VVD_I : public OZ_Propagator {
protected:
  int reg_sz;
  int * reg_a;
  int * reg_smd_sz;
  int reg_x_sz;
  OZ_Term * reg_x;
  int reg_c;
public:
  Propagator_VI_VVD_I(OZ_Term, OZ_Term, OZ_Term);
  virtual ~Propagator_VI_VVD_I(void);

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_VI_VVD_I); }
  virtual OZ_Term getParametersC(const char *) const;
};

//-----------------------------------------------------------------------------

class Propagator_D_FD_D : public OZ_Propagator {
protected:
  OZ_Term reg_v, reg_b;
  OZ_FiniteDomain reg_domain;
public:
  Propagator_D_FD_D(OZ_Term, OZ_Term, OZ_Term);
  virtual ~Propagator_D_FD_D(void) {reg_domain.disposeExtension();}

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_D_FD_D); }
  virtual OZ_Term getParameters(void) const;
};

//-----------------------------------------------------------------------------

class Propagator_VD_D_D_D : public OZ_Propagator {
protected:
  int reg_v_sz;
  OZ_Term * reg_v;
  OZ_Term reg_low, reg_up, reg_b; // Keep this order
public:
  Propagator_VD_D_D_D(OZ_Term, OZ_Term, OZ_Term, OZ_Term);
  virtual ~Propagator_VD_D_D_D(void);

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_VD_D_D_D); }
  virtual OZ_Term getParameters(void) const;
};

//-----------------------------------------------------------------------------

class Propagator_D_I_D_I : public OZ_Propagator {
protected:
  OZ_Term reg_x, reg_y; // Keep this order
  int reg_xd, reg_yd;
public:
  Propagator_D_I_D_I(OZ_Term x, OZ_Term xd, OZ_Term y, OZ_Term yd)
    : reg_x(x), reg_xd(OZ_intToC(xd)), reg_y(y), reg_yd(OZ_intToC(yd)) {}

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_D_I_D_I); }
  virtual OZ_Term getParameters(void) const;
};

//-----------------------------------------------------------------------------

class Propagator_D_I_D_I_D : public Propagator_D_I_D_I {
protected:
  OZ_Term reg_b;
public:
  Propagator_D_I_D_I_D(OZ_Term x, OZ_Term xd, OZ_Term y, OZ_Term yd, OZ_Term b)
    : reg_b(b),  Propagator_D_I_D_I(x, xd, y, yd) {}
  
  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_D_I_D_I_D); }
  virtual OZ_Term getParameters(void) const;
};

//-----------------------------------------------------------------------------

class Propagator_VI_VD_D : public OZ_Propagator {
protected:
  int *reg_a,reg_sz,reg_c,dpos;
  OZ_Term reg_d;
  OZ_Term *_a,*reg_x;
public:
  Propagator_VI_VD_D(OZ_Term, OZ_Term, OZ_Term);
  virtual ~Propagator_VI_VD_D(void);

  virtual void gCollect(void);
  virtual void sClone(void);

  virtual size_t sizeOf(void) { return sizeof(Propagator_VI_VD_D); }
  virtual OZ_Term getParametersC(const char *) const;
  OZ_Boolean simplify(void);
};


#endif // __STD_HH__

