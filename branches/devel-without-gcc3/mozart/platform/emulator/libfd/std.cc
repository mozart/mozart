/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *    Joerg Wuertz (wuertz@dfki.de)
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

#include "conf.h"
#include "std.hh"
#include <stdlib.h>
#include <math.h>

#if __GLIBC__ == 2 || defined(WINDOWS) || defined(NETBSD) || defined(RHAPSODY) || defined(OPENBSD)

/* the following have been copied from linux's values.h */
#define _DEXPLEN    11
#define _HIDDENBIT  1
#define DSIGNIF     (DOUBLEBITS - _DEXPLEN + _HIDDENBIT - 1)
#define BITSPERBYTE 8
#define BITS(type)  (BITSPERBYTE * (int)sizeof(type))
#define LONGBITS    BITS(long)
#define DOUBLEBITS  BITS(double)
#define DMAXPOWTWO  ((double)(1L << LONGBITS -2)*(1L << DSIGNIF - LONGBITS +1))

#else

#include <values.h>

#endif


//-----------------------------------------------------------------------------

inline
void warn_inexact(double v)
{
  if (DMAXPOWTWO < v) {
    printf("System warning (finite domain module): \n");
    printf("\tThe produced result might be incorrect due to\n");
    printf("\tlimited precision of internal computation.");
  }
}

//-----------------------------------------------------------------------------

void Propagator_D_D::gCollect(void) {
  OZ_gCollectBlock(&reg_x, &reg_x, 2);
}

void Propagator_D_D::sClone(void) {
  OZ_sCloneBlock(&reg_x, &reg_x, 2);
}

OZ_Term Propagator_D_D::getParameters(void) const {
  RETURN_LIST2(reg_x, reg_y);
}

//-----------------------------------------------------------------------------

void Propagator_D_D_D::gCollect(void) {
  OZ_gCollectBlock(&reg_x, &reg_x, 3);
}

void Propagator_D_D_D::sClone(void) {
  OZ_sCloneBlock(&reg_x, &reg_x, 3);
}

OZ_Term Propagator_D_D_D::getParameters(void) const
{
  RETURN_LIST3(reg_x, reg_y, reg_z);
}


//-----------------------------------------------------------------------------

void Propagator_D_D_D_I::gCollect(void) {
  OZ_gCollectBlock(&reg_x, &reg_x, 3);
}

void Propagator_D_D_D_I::sClone(void) {
  OZ_sCloneBlock(&reg_x, &reg_x, 3);
}

OZ_Term Propagator_D_D_D_I::getParameters(void) const
{
  RETURN_LIST4(reg_x, reg_y, reg_z, OZ_int(reg_c));
}

OZ_Term Propagator_D_D_D_I::getParametersC(char * lit) const
{
  RETURN_LIST5(reg_x, reg_y, OZ_atom(lit), reg_z, OZ_int(reg_c));
}

//-----------------------------------------------------------------------------

void Propagator_D_I_D::gCollect(void) {
  OZ_gCollectBlock(&reg_x, &reg_x, 2);
}

void Propagator_D_I_D::sClone(void) {
  OZ_sCloneBlock(&reg_x, &reg_x, 2);
}

OZ_Term Propagator_D_I_D::getParameters(void) const
{
  RETURN_LIST3(reg_x, OZ_int(reg_y), reg_z);
}

//-----------------------------------------------------------------------------

void Propagator_D_D_I::gCollect(void) {
  OZ_gCollectBlock(&reg_x, &reg_x, 2);
}

void Propagator_D_D_I::sClone(void) {
  OZ_sCloneBlock(&reg_x, &reg_x, 2);
}

OZ_Term Propagator_D_D_I::getParameters(void) const
{
  RETURN_LIST3(reg_x, reg_y, OZ_int(reg_c));
}

OZ_Term Propagator_D_D_I::getParametersC(char * lit) const
{
  RETURN_LIST4(reg_x, reg_y, OZ_atom(lit), OZ_int(reg_c));
}

//-----------------------------------------------------------------------------

Propagator_D_VD_I::Propagator_D_VD_I(OZ_Term n, OZ_Term l, OZ_Term v) 
{
  reg_n = n;
  reg_v = OZ_intToC(v);
  reg_l = vectorToOzTerms(l, reg_l_sz);
}

Propagator_D_VD_I::~Propagator_D_VD_I(void) 
{
  OZ_hfreeOzTerms(reg_l, reg_l_sz);
}

void Propagator_D_VD_I::gCollect(void) {
  OZ_gCollectTerm(reg_n);
  reg_l = OZ_gCollectAllocBlock(reg_l_sz, reg_l);
}

void Propagator_D_VD_I::sClone(void) {
  OZ_sCloneTerm(reg_n);
  reg_l = OZ_sCloneAllocBlock(reg_l_sz, reg_l);
}

OZ_Term Propagator_D_VD_I::getParameters(void) const
{
  TERMVECTOR2LIST(reg_l, reg_l_sz, l);
  RETURN_LIST3(reg_n, l, OZ_int(reg_v));
}

//-----------------------------------------------------------------------------

Propagator_D_VI_D::Propagator_D_VI_D(OZ_Term n, OZ_Term l, OZ_Term v) 
{
  reg_n = n;
  reg_v = v;
  reg_l = vectorToInts(l, reg_l_sz);
}

Propagator_D_VI_D::~Propagator_D_VI_D(void) 
{
  OZ_hfreeCInts(reg_l, reg_l_sz);
}

void Propagator_D_VI_D::gCollect(void) {
  OZ_gCollectBlock(&reg_n, &reg_n, 2);
  reg_l = OZ_copyCInts(reg_l_sz, reg_l);
}

void Propagator_D_VI_D::sClone(void) {
  OZ_sCloneBlock(&reg_n, &reg_n, 2);
  reg_l = OZ_copyCInts(reg_l_sz, reg_l);
}

OZ_Term Propagator_D_VI_D::getParameters(void) const
{
  INTVECTOR2LIST(reg_l, reg_l_sz, l);
  RETURN_LIST3(reg_n, l, reg_v);
}

//-----------------------------------------------------------------------------

Propagator_VI_VD_I::Propagator_VI_VD_I(OZ_Term x, OZ_Term d)
{
  reg_c = 0;

  NUMBERCAST check_inexact = reg_c;

  reg_x = vectorToOzTerms(x, d, reg_sz);
  reg_a = OZ_hallocCInts(reg_sz);
    
  for (int i = 0; i < reg_sz; i += 1) {
    reg_a[i] = (i == reg_sz-1) ? -1 : 1;
    OZ_FDIntVar xv; // no initialising constructor 'cause it uses method ask
    xv.ask(reg_x[i]);
    check_inexact += NUMBERCAST(abs(reg_a[i])) * xv->getMaxElem();
  }
  
  warn_inexact(check_inexact);
}


Propagator_VI_VD_I::Propagator_VI_VD_I(int ax, OZ_Term x, 
				       int ay, OZ_Term y, 
				       int az, OZ_Term z, int c)
{
  reg_sz = 3;
  reg_c = c;
  reg_a = OZ_hallocCInts(reg_sz);
  reg_a[0] = ax;
  reg_a[1] = ay;
  reg_a[2] = az;
  reg_x = OZ_hallocOzTerms(reg_sz);
  reg_x[0] = x;
  reg_x[1] = y;
  reg_x[2] = z;
}

Propagator_VI_VD_I::Propagator_VI_VD_I(OZ_Term a, OZ_Term x, OZ_Term d,
                                      OZ_Boolean is_lin) 
{
  reg_c = 0;

  NUMBERCAST check_inexact = reg_c;

  if (is_lin) {
    reg_x = vectorToOzTerms(x, d, reg_sz);
    reg_a = vectorToInts1(a, reg_sz);
    
    for (int i = 0; i < reg_sz; i += 1) {
      OZ_FDIntVar xv; // no initialising constructor 'cause it uses method ask
      xv.ask(reg_x[i]);
      check_inexact += NUMBERCAST(abs(reg_a[i])) * xv->getMaxElem();
    }
  } else {
    reg_a = vectorToInts1(a, reg_sz);
    reg_x = OZ_hallocOzTerms(reg_sz);
    
    if (OZ_isCons(x)) {      
      for (int i = 0; OZ_isCons(x); x = OZ_tail(x), i += 1) {
	vectorToLinear(OZ_head(x), reg_a[i], reg_x[i]);	
	OZ_FDIntVar xv; // no initialising constructor 'cause it uses method ask
	xv.ask(reg_x[i]);
	check_inexact += NUMBERCAST(abs(reg_a[i])) * xv->getMaxElem();
      }
      reg_x[reg_sz-1] = d;
      OZ_FDIntVar xv; // no initialising constructor 'cause it uses method ask
      xv.ask(reg_x[reg_sz-1]);
      check_inexact += NUMBERCAST(abs(reg_a[reg_sz-1])) * xv->getMaxElem();
    } else if (OZ_isTuple(x)) {
      for (int i = 0; i < reg_sz; i += 1) {
	if (i == reg_sz-1) 
	  reg_x[i] = d;
	else 
	  vectorToLinear(OZ_getArg(x, i), reg_a[i], reg_x[i]); 	
	OZ_FDIntVar xv; // no initialising constructor 'cause it uses method ask
	xv.ask(reg_x[i]);
	check_inexact += NUMBERCAST(abs(reg_a[i])) * xv->getMaxElem();
      }
    } else {
      OZ_ASSERT(OZ_isRecord(x));

      OZ_Term al = OZ_arityList(x);
      for (int i = 0; OZ_isCons(al); al = OZ_tail(al), i += 1) {
	vectorToLinear(OZ_subtree(x, OZ_head(al)), reg_a[i], reg_x[i]);	
	OZ_FDIntVar xv; // no initialising constructor 'cause it uses method ask
	xv.ask(reg_x[i]);
	check_inexact += NUMBERCAST(abs(reg_a[i])) * xv->getMaxElem();
      }
      reg_x[reg_sz-1] = d;
      OZ_FDIntVar xv; // no initialising constructor 'cause it uses method ask
      xv.ask(reg_x[reg_sz-1]);
      check_inexact += NUMBERCAST(abs(reg_a[reg_sz-1])) * xv->getMaxElem();
      
    } 
  }
  
  warn_inexact(check_inexact);
}

Propagator_VI_VD_I::Propagator_VI_VD_I(const Propagator_VI_VD_I_D  &o) {
  reg_c = o.reg_c;
  reg_sz = o.reg_sz;
  reg_a = OZ_hallocCInts(reg_sz);
  reg_x = OZ_hallocOzTerms(reg_sz);
  for (int i = reg_sz; i--; ) {
    reg_a[i] = o.reg_a[i];
    reg_x[i] = o.reg_x[i];
  }
}

Propagator_VI_VD_I::Propagator_VI_VD_I(int sz, int sizes[], int single_var[], 
				       int a[], OZ_Term x[], int c)
{
  reg_sz = sz;
  reg_a = OZ_hallocCInts(reg_sz);
  reg_x = OZ_hallocOzTerms(reg_sz);
  reg_c = c;

  for (int i = 0, k = 0, l = 0; i < sz; i += 1) {
    if (single_var[i] == -1) {  // determined summand
      int prod = a[i];
      for (int j = 0; j < sizes[i]; j += 1, k += 1) 
	prod *= OZ_intToC(x[k]);
      reg_c += prod; 
      reg_sz -= 1;
    } else {                   // linear summand
      reg_a[l] = a[i];
      for (int j = 0; j < sizes[i]; j += 1, k += 1) {
	if (j == single_var[i]) { // found variable
	  reg_x[l] = x[k];
	} else {
	  reg_a[l] *= OZ_intToC(x[k]);
	}
      }
      l += 1;
    }
  }
  
  simplify();
}

Propagator_VI_VD_I::~Propagator_VI_VD_I(void) 
{
  OZ_hfreeCInts(reg_a, reg_sz);
  OZ_hfreeOzTerms(reg_x, reg_sz);
}

void Propagator_VI_VD_I::gCollect(void) {
  reg_a = OZ_copyCInts(reg_sz, reg_a);
  reg_x = OZ_gCollectAllocBlock(reg_sz, reg_x);
}

void Propagator_VI_VD_I::sClone(void) {
  reg_a = OZ_copyCInts(reg_sz, reg_a);
  reg_x = OZ_sCloneAllocBlock(reg_sz, reg_x);
}

OZ_Term Propagator_VI_VD_I::getParametersC(char * lit) const
{
  INTVECTOR2LIST(reg_a, reg_sz, a);
  TERMVECTOR2LIST(reg_x, reg_sz, x);

  RETURN_LIST4(a, x, OZ_atom(lit), OZ_int(-reg_c));
}

//-----------------------------------------------------------------------------

void Propagator_VI_VD_I_D::gCollect(void) {
  Propagator_VI_VD_I::gCollect();
  OZ_gCollectTerm(reg_b);
}

void Propagator_VI_VD_I_D::sClone(void) {
  Propagator_VI_VD_I::sClone();
  OZ_sCloneTerm(reg_b);
}

OZ_Term Propagator_VI_VD_I_D::getParametersC(char * lit) const
{
  INTVECTOR2LIST(reg_a, reg_sz, a);
  TERMVECTOR2LIST(reg_x, reg_sz, x);

  RETURN_LIST5(a, x, OZ_atom(lit), OZ_int(-reg_c), reg_b);
}

//-----------------------------------------------------------------------------

Propagator_VD::Propagator_VD(OZ_Term l) 
{
  reg_l = vectorToOzTerms(l, reg_l_sz);
}

Propagator_VD::~Propagator_VD(void) 
{
  OZ_hfreeOzTerms(reg_l, reg_l_sz);
}

void Propagator_VD::gCollect(void) {
  reg_l = OZ_gCollectAllocBlock(reg_l_sz, reg_l);
}

void Propagator_VD::sClone(void) {
  reg_l = OZ_sCloneAllocBlock(reg_l_sz, reg_l);
}

OZ_Term Propagator_VD::getParameters(void) const
{
  TERMVECTOR2LIST(reg_l, reg_l_sz, l);
  RETURN_LIST1(l);
}

//-----------------------------------------------------------------------------

Propagator_VD_VI::Propagator_VD_VI(OZ_Term l, OZ_Term offset)
{
  reg_offset = vectorToInts(offset, reg_sz);
  reg_l = vectorToOzTerms(l, reg_sz);
}

Propagator_VD_VI::Propagator_VD_VI(int size) : reg_sz(size)
{
  reg_offset = OZ_hallocCInts(size);
  reg_l = OZ_hallocOzTerms(size);
}

Propagator_VD_VI::~Propagator_VD_VI(void) 
{
  OZ_hfreeCInts(reg_offset, reg_sz);
  OZ_hfreeOzTerms(reg_l, reg_sz);
}

void Propagator_VD_VI::gCollect(void) {
  reg_l      = OZ_gCollectAllocBlock(reg_sz, reg_l);
  reg_offset = OZ_copyCInts(reg_sz, reg_offset);
}

void Propagator_VD_VI::sClone(void) {
  reg_l      = OZ_sCloneAllocBlock(reg_sz, reg_l);
  reg_offset = OZ_copyCInts(reg_sz, reg_offset);
}

OZ_Term Propagator_VD_VI::getParameters(void) const
{
  TERMVECTOR2LIST(reg_l, reg_sz, l);
  INTVECTOR2LIST(reg_offset, reg_sz, offset);
  RETURN_LIST2(l, offset);
}

//-----------------------------------------------------------------------------
// cumulative

Propagator_VD_VI_VI_I::Propagator_VD_VI_VI_I(OZ_Term l, OZ_Term offset, OZ_Term use, OZ_Term cap)
{
  reg_offset   = vectorToInts(offset, reg_sz);
  reg_l        = vectorToOzTerms(l, reg_sz);
  reg_use      = vectorToInts(use, reg_sz);
  reg_capacity = OZ_intToC(cap);
}

Propagator_VD_VI_VI_I::Propagator_VD_VI_VI_I(int size) : reg_sz(size)
{
  reg_offset = OZ_hallocCInts(size);
  reg_use    = OZ_hallocCInts(size);
  reg_l      = OZ_hallocOzTerms(size);
}

Propagator_VD_VI_VI_I::~Propagator_VD_VI_VI_I(void) 
{
  OZ_hfreeCInts(reg_offset, reg_sz);
  OZ_hfreeCInts(reg_use, reg_sz);
  OZ_hfreeOzTerms(reg_l, reg_sz);

}

void Propagator_VD_VI_VI_I::gCollect(void) {
  reg_l      = OZ_gCollectAllocBlock(reg_sz, reg_l);
  reg_offset = OZ_copyCInts(reg_sz, reg_offset);
  reg_use    = OZ_copyCInts(reg_sz, reg_use);
}

void Propagator_VD_VI_VI_I::sClone(void) {
  reg_l      = OZ_sCloneAllocBlock(reg_sz, reg_l);
  reg_offset = OZ_copyCInts(reg_sz, reg_offset);
  reg_use    = OZ_copyCInts(reg_sz, reg_use);
}

OZ_Term Propagator_VD_VI_VI_I::getParameters(void) const
{
  TERMVECTOR2LIST(reg_l, reg_sz, l);
  INTVECTOR2LIST(reg_offset, reg_sz, offset);
  INTVECTOR2LIST(reg_use, reg_sz, use);
  RETURN_LIST4(l, offset, use, OZ_int(reg_capacity));
}

//-----------------------------------------------------------------------------

Propagator_VI_VVD_I::Propagator_VI_VVD_I(OZ_Term a, OZ_Term x, OZ_Term d)
{
  reg_c = 0;

  NUMBERCAST check_inexact = reg_c;

  reg_a = vectorToInts1(a, reg_sz);
  reg_smd_sz = OZ_hallocCInts(reg_sz);
  reg_x_sz = 0;

  if (OZ_isCons(x)) {
    int i, k;
    
    OZ_Term t = x;
    for (i = 0; OZ_isCons(t); t = OZ_tail(t), i += 1) 
      reg_x_sz += (reg_smd_sz[i] = OZ_vectorSize(OZ_head(t)));
    reg_x_sz += (reg_smd_sz[i] = 1); // for d

    reg_x = OZ_hallocOzTerms(reg_x_sz);
    t = x;
    for (i = 0, k = 0; OZ_isCons(t); i += 1, t = OZ_tail(t)) {
      vectorToOzTerms(OZ_head(t), reg_x + k);

      NUMBERCAST prod = abs(reg_a[i]);

      for (int j = 0; j < reg_smd_sz[i]; j++, k++) {
	OZ_FDIntVar xv; // no initialising constructor 'cause it uses method ask
	xv.ask(reg_x[k]);
	prod *= xv->getMaxElem();
      }
      check_inexact += prod;
    }

  } else if (OZ_isTuple(x)) {
    int i, k;
    for (i = 0; i < reg_sz; i += 1) {
      reg_x_sz += (reg_smd_sz[i] = (i==reg_sz-1) ? 1 : OZ_vectorSize(OZ_getArg(x, i)));
    }

    reg_x = OZ_hallocOzTerms(reg_x_sz);
    for (i = 0, k = 0; i < reg_sz-1; i += 1) {
      vectorToOzTerms(OZ_getArg(x, i), reg_x + k);

      NUMBERCAST prod = abs(reg_a[i]);

      for (int j = 0; j < reg_smd_sz[i]; j++, k++) {
	OZ_FDIntVar xv; // no initialising constructor 'cause it uses method ask
	xv.ask(reg_x[k]);
	prod *= xv->getMaxElem();
      }
      check_inexact += prod;
    }

  } else {
    OZ_ASSERT(OZ_isRecord(x));

    int i, k;
    OZ_Term al = OZ_arityList(x);

    for (i = 0; OZ_isCons(al); al = OZ_tail(al), i += 1) 
      reg_x_sz += (reg_smd_sz[i] = OZ_vectorSize(OZ_subtree(x, OZ_head(al))));
    reg_x_sz += (reg_smd_sz[i] = 1); // for d

    al = OZ_arityList(x);
    reg_x = OZ_hallocOzTerms(reg_x_sz);
    for (i = 0, k = 0; OZ_isCons(al); i += 1, al = OZ_tail(al)) {
      vectorToOzTerms(OZ_subtree(x, OZ_head(al)), reg_x + k);

      NUMBERCAST prod = abs(reg_a[i]);

      for (int j = 0; j < reg_smd_sz[i]; j++, k++) {
	OZ_FDIntVar xv; // no initialising constructor 'cause it uses method ask
	xv.ask(reg_x[k]);
	prod *= xv->getMaxElem();
      }
      check_inexact += prod;
    }

  } 

  reg_x[reg_x_sz-1] = d;
  OZ_FDIntVar xv; // no initialising constructor 'cause it uses method ask
  xv.ask(reg_x[reg_x_sz-1]);
  check_inexact += abs(reg_a[reg_sz-1]) * xv->getMaxElem();

  
  warn_inexact(check_inexact);
}

Propagator_VI_VVD_I::~Propagator_VI_VVD_I(void) 
{
  OZ_hfreeCInts(reg_a, reg_sz);
  OZ_hfreeCInts(reg_smd_sz, reg_sz);
  OZ_hfreeOzTerms(reg_x, reg_x_sz);
}

void Propagator_VI_VVD_I::gCollect(void) {
  reg_x      = OZ_gCollectAllocBlock(reg_x_sz, reg_x);
  reg_a      = OZ_copyCInts(reg_sz, reg_a);
  reg_smd_sz = OZ_copyCInts(reg_sz, reg_smd_sz);
}

void Propagator_VI_VVD_I::sClone(void) {
  reg_x      = OZ_sCloneAllocBlock(reg_x_sz, reg_x);
  reg_a      = OZ_copyCInts(reg_sz, reg_a);
  reg_smd_sz = OZ_copyCInts(reg_sz, reg_smd_sz);
}

OZ_Term Propagator_VI_VVD_I::getParametersC(char * lit) const
{
  INTVECTOR2LIST(reg_a, reg_sz, a);

  OZ_Term x = OZ_nil();
  for (int k = reg_x_sz, i = reg_sz; i--; ) {
    OZ_Term s = OZ_nil();
    for (int j = reg_smd_sz[i]; j--; )
      s = OZ_cons(reg_x[--k], s);
    
    x = OZ_cons(s, x);
  }

  RETURN_LIST4(a, x, OZ_atom(lit), OZ_int(-reg_c));
}

//-----------------------------------------------------------------------------

Propagator_D_FD_D::Propagator_D_FD_D(OZ_Term v, OZ_Term d, OZ_Term b)
  : reg_v(v), reg_b(b)
{
  reg_domain.initDescr(d);
}

void Propagator_D_FD_D::gCollect(void) {
  OZ_gCollectTerm(reg_v);
  OZ_gCollectTerm(reg_b);
  reg_domain.copyExtension();
}

void Propagator_D_FD_D::sClone(void) {
  OZ_sCloneTerm(reg_v);
  OZ_sCloneTerm(reg_b);
  reg_domain.copyExtension();
}

OZ_Term Propagator_D_FD_D::getParameters(void) const
{
  OZ_Term d = OZ_fsetValue(new OZ_FSetValue(reg_domain));

  RETURN_LIST3(reg_v, d, reg_b);
}

//-----------------------------------------------------------------------------

Propagator_VD_D_D_D::Propagator_VD_D_D_D(OZ_Term v, OZ_Term l, 
					 OZ_Term u, OZ_Term b) 
  : reg_low(l), reg_up(u), reg_b(b)
{
  reg_v = vectorToOzTerms(v, reg_v_sz);
}

void Propagator_VD_D_D_D::gCollect(void) {
  OZ_gCollectBlock(&reg_low, &reg_low, 3);
  reg_v = OZ_gCollectAllocBlock(reg_v_sz, reg_v);
}

void Propagator_VD_D_D_D::sClone(void) {
  OZ_sCloneBlock(&reg_low, &reg_low, 3);
  reg_v = OZ_sCloneAllocBlock(reg_v_sz, reg_v);
}

Propagator_VD_D_D_D::~Propagator_VD_D_D_D(void) 
{
  OZ_hfreeOzTerms(reg_v, reg_v_sz);
}

OZ_Term Propagator_VD_D_D_D::getParameters(void) const
{
  TERMVECTOR2LIST(reg_v, reg_v_sz, v);
  RETURN_LIST4(v, reg_low, reg_up, reg_b);
}

//-----------------------------------------------------------------------------

void Propagator_D_I_D_I::gCollect(void) {
  OZ_gCollectBlock(&reg_x, &reg_x, 2);
}

void Propagator_D_I_D_I::sClone(void) {
  OZ_sCloneBlock(&reg_x, &reg_x, 2);
}

OZ_Term Propagator_D_I_D_I::getParameters(void) const
{
  RETURN_LIST4(reg_x, OZ_int(reg_xd), reg_y, OZ_int(reg_yd));
}

//-----------------------------------------------------------------------------

void Propagator_D_I_D_I_D::gCollect(void)  {
  Propagator_D_I_D_I::gCollect();
  OZ_gCollectTerm(reg_b);
}

void Propagator_D_I_D_I_D::sClone(void)  {
  Propagator_D_I_D_I::sClone();
  OZ_sCloneTerm(reg_b);
}

OZ_Term Propagator_D_I_D_I_D::getParameters(void) const
{
  RETURN_LIST5(reg_x, OZ_int(reg_xd), reg_y, OZ_int(reg_yd), reg_b);
}

//-----------------------------------------------------------------------------

Propagator_VI_VD_D::Propagator_VI_VD_D(OZ_Term a, OZ_Term x, OZ_Term d)
  : reg_sz(OZ_vectorSize(x)), reg_d(d)
{
  reg_c = 0;
  dpos = -1; 
  NUMBERCAST check_inexact = reg_c;
  reg_x = vectorToOzTerms(x, reg_sz);
  reg_a = vectorToInts(a, reg_sz);
  
  for (int i = 0; i < reg_sz; i += 1) {
    OZ_FDIntVar xv; // no initialising constructor 'cause it uses method ask
    xv.ask(reg_x[i]);
    check_inexact+= double(reg_a[i]<0 ? -reg_a[i]:reg_a[i]) * xv->getMaxElem();
  }
  warn_inexact(check_inexact);
}

Propagator_VI_VD_D::~Propagator_VI_VD_D(void) 
{
  OZ_hfreeCInts(reg_a, reg_sz);
  OZ_hfreeOzTerms(reg_x, reg_sz);
}

void Propagator_VI_VD_D::gCollect(void) {
  OZ_gCollectTerm(reg_d);
  reg_x = OZ_gCollectAllocBlock(reg_sz, reg_x);
  reg_a = OZ_copyCInts(reg_sz, reg_a);
}

void Propagator_VI_VD_D::sClone(void) {
  OZ_sCloneTerm(reg_d);
  reg_x = OZ_sCloneAllocBlock(reg_sz, reg_x);
  reg_a = OZ_copyCInts(reg_sz, reg_a);
}

OZ_Term Propagator_VI_VD_D::getParametersC(char *lit) const
{
  INTVECTOR2LIST(reg_a, reg_sz, a);
  TERMVECTOR2LIST(reg_x, reg_sz, x);
  RETURN_LIST4(a,x,OZ_atom(lit),reg_d);
}

OZ_Boolean Propagator_VI_VD_D::simplify(void)
{ 
  if (reg_sz) {
    DECL_DYN_ARRAY(OZ_Term,xd,reg_sz+1);
    int j;
    for(j = reg_sz; j--; ) 
      xd[j] = reg_x[j];
    xd[reg_sz] = reg_d;
    int *is = OZ_findEqualVars(reg_sz+1,xd);
    dpos = is[reg_sz];
    for (j = reg_sz; j--; ) {
      if (is[j] == -1) {      // singleton in x
	reg_c += int(OZ_intToC(reg_x[j]) * NUMBERCAST(reg_a[j]));
	reg_x[j] = 0; 
      } else if (j != is[j]) {   // multiple appearing var in x 
	reg_a[is[j]] += reg_a[j];
	reg_x[j] = 0;
      } 
    }
    int from = 0, to = 0;
    for (; from < reg_sz; from++) {
     if (reg_x[from] != 0 && reg_a[from] != 0) {
       if (from != to) {
         reg_a[to] = reg_a[from];
         reg_x[to] = reg_x[from];
       }
       to++;
     } else if (dpos != reg_sz && from < dpos) 
       dpos--;
    }
    reg_sz = to;
  }
  return (dpos > -1 && dpos < reg_sz);
}

//-----------------------------------------------------------------------------
// eof

