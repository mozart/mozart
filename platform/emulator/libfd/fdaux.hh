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

#ifndef __FDAUX_HH__
#define __FDAUX_HH__

#include <stdio.h>

#include "mozart_cpi.hh"

//-----------------------------------------------------------------------------
// debug macros

#ifdef DEBUG_FD
#define OZ_DEBUG
#endif

//#define OZ_DEBUG

#ifdef OZ_DEBUG
#define OZ_DEBUGCODE(C) C

extern "C" void oz_debugprint(char *format ...);

#define _OZ_DEBUGPRINTTHIS(string)              \
   _OZ_DEBUGPRINT(("%s%s",string,this->toString()))

#define OZ_DEBUGPRINTTHIS(string) _OZ_DEBUGPRINTTHIS(string)

#define _OZ_DEBUGPRINT(C) oz_debugprint C
#define OZ_DEBUGPRINT(C) _OZ_DEBUGPRINT(C)
#define OZ_ASSERT(C)                                    \
  if (! (C)) {                                          \
    fprintf(stderr,"OZ_ASSERT %s failed (%s:%d).\n",    \
            #C,__FILE__, __LINE__);                     \
    fflush(stderr);                                     \
  }

#else
#define OZ_DEBUGCODE(C)
#define _OZ_DEBUGPRINT(C)
#define OZ_DEBUGPRINT(C)
#define OZ_ASSERT(C)
#define OZ_DEBUGPRINTTHIS(C)
#endif


//-----------------------------------------------------------------------------
// misc macros

#define SAMELENGTH_VECTORS(I, J)                                        \
  {                                                                     \
    int i_size = OZ_vectorSize(OZ_in(I));                               \
    int j_size = OZ_vectorSize(OZ_in(J));                               \
    if ((i_size >= 0) && (j_size >= 0) && (i_size != j_size))           \
      return OZ_typeErrorCPI(expectedType, J,                           \
                          "Vectors must have same size.");              \
  }

#define NUMBERCAST double

#define SUM_OP_EQ  "=:"
#define SUM_OP_NEQ "\\=:"
#define SUM_OP_LEQ "=<:"
#define SUM_OP_LT  "<:"
#define SUM_OP_GEQ ">=:"
#define SUM_OP_GT  ">:"

enum sum_ops {
  sum_ops_eq,
  sum_ops_neq,
  sum_ops_leq,
  sum_ops_geq,
  sum_ops_lt,
  sum_ops_gt,
  sum_ops_unknown
};

sum_ops getSumOps(OZ_Term);

#define ERROR_UNEXPECTED_OPERATOR(P) \
return OZ_typeErrorCPI(expectedType, P,                                 \
                    "Expected one of the following: "                   \
                    SUM_OP_EQ " , " SUM_OP_NEQ " , " SUM_OP_LEQ " , "   \
                    SUM_OP_LT " , " SUM_OP_GEQ " or " SUM_OP_GT ".")

#define ERROR_UNEXPECTED_OPERATOR_NOIN(P) \
return OZ_typeErrorCPI(expectedType, P,                \
                    "Expected one of the following: "  \
                    SUM_OP_EQ " or " SUM_OP_NEQ ".")

#define RETURN_LIST1(X) \
return OZ_cons(X, OZ_nil())

#define RETURN_LIST2(X, Y) \
return OZ_cons(X, OZ_cons(Y, OZ_nil()))

#define RETURN_LIST3(X, Y, Z) \
return OZ_cons(X, OZ_cons(Y, OZ_cons(Z, OZ_nil())))

#define RETURN_LIST4(U, V, W, X) \
return OZ_cons(U, OZ_cons(V, OZ_cons(W, OZ_cons(X, OZ_nil()))))

#define RETURN_LIST5(U, V, W, X, Y) \
return OZ_cons(U, OZ_cons(V, OZ_cons(W, OZ_cons(X, OZ_cons(Y, OZ_nil())))))

#define _TERMVECTOR2LIST(V, S, L, C)            \
OZ_Term L = OZ_nil();                           \
{for (int i = S; i--; )                         \
  L = OZ_cons(C(V[i]), L); }


#define TERMVECTOR2LIST(V, S, L)  _TERMVECTOR2LIST(V, S, L, )
#define INTVECTOR2LIST(V, S, L)  _TERMVECTOR2LIST(V, S, L, OZ_int)

//-----------------------------------------------------------------------------

class PropagatorExpect;

typedef OZ_expect_t (PropagatorExpect::*PropagatorExpectMeth) (OZ_Term);

class PropagatorExpect : public OZ_Expect {
public:
  OZ_expect_t expectIntVarAny(OZ_Term t) {
    return expectIntVar(t);
  }
  OZ_expect_t expectIntVarMin(OZ_Term t) {
    return expectIntVar(t, fd_prop_bounds);
  }
  OZ_expect_t expectIntVarMax(OZ_Term t) {
    return expectIntVar(t, fd_prop_bounds);
  }
  OZ_expect_t expectIntVarSingl(OZ_Term t) {
    return expectIntVar(t, fd_prop_singl);
  }
  OZ_expect_t expectVector(OZ_Term t, PropagatorExpectMeth expectf) {
    return OZ_Expect::expectVector(t, (OZ_ExpectMeth) expectf);
  }
  OZ_expect_t expectProperRecord(OZ_Term t, PropagatorExpectMeth expectf) {
    return OZ_Expect::expectProperRecord(t, (OZ_ExpectMeth) expectf);
  }
  OZ_expect_t expectVectorLiteral(OZ_Term t) {
    return expectVector(t, (PropagatorExpectMeth) &OZ_Expect::expectLiteral);
  }
  OZ_expect_t expectVectorIntVarSingl(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectIntVarSingl);
  }
  OZ_expect_t expectVectorIntVarAny(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectIntVarAny);
  }
  OZ_expect_t expectVectorBoolVar(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectBoolVar);
  }
  OZ_expect_t expectVectorVectorIntVarMinMax(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectVectorIntVarMinMax);
  }
  OZ_expect_t expectVectorVectorIntVarAny(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectVectorIntVarAny);
  }
  OZ_expect_t expectVectorVectorLiteral(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectVectorLiteral);
  }
  OZ_expect_t expectVectorVectorIntVarSingl(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectVectorIntVarSingl);
  }
  OZ_expect_t expectVectorLinearVector(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectLinearVector);
  }
  OZ_expect_t expectLinearVector(OZ_Term t) {
    OZ_expect_t r = expectVector(t, (PropagatorExpectMeth) &OZ_Expect::expectInt);

    if ((r.size - r.accepted) == 1)
      r.accepted = r.size;

    return r;
  }
  OZ_expect_t expectProperRecordIntVarMinMax(OZ_Term t) {
    return expectProperRecord(t, &PropagatorExpect::expectIntVarMinMax);
  }
  OZ_expect_t expectProperRecordInt(OZ_Term t) {
    return expectProperRecord(t, (PropagatorExpectMeth) &PropagatorExpect::expectInt);
  }
  OZ_expect_t expectLinearVectorIntVarSingl(OZ_Term t) {
    collectVarsOff();
    OZ_expect_t r =
      expectVector(t, (PropagatorExpectMeth) &OZ_Expect::expectInt);
    collectVarsOn();
    if ((r.size - r.accepted) <= 1) {
      return  expectVectorIntVarSingl(t);
    }
    return expectVector(t, (PropagatorExpectMeth) &OZ_Expect::expectInt);
  }
  OZ_expect_t expectVectorLinearVectorIntVarSingl(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectLinearVectorIntVarSingl);
  }
};

//-----------------------------------------------------------------------------

class PropagatorController_VV : public OZ_ParamIterator {
protected:
  OZ_FDIntVar * vv;
  int size;
public:
  PropagatorController_VV(int s, OZ_FDIntVar i[]) : size(s), vv(i) {}

  OZ_Return leave(int j = 0) {
    int vars_left = 0;
    for (int i = size; i--; vars_left += vv[i].leave() ? 1 : 0);
    return (vars_left <= j)  ? OZ_ENTAILED : OZ_SLEEP;
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

class PropagatorController_VV_V {
protected:
  OZ_FDIntVar &v;
  OZ_FDIntVar * vv;
  int size;
public:
  PropagatorController_VV_V(int s, OZ_FDIntVar i1[], OZ_FDIntVar &i2)
    : size(s), vv(i1), v(i2) {}

  OZ_Return leave(void) {
    OZ_Boolean vars_left = v.leave();
    for (int i = size; i--; vars_left |= vv[i].leave());
    return vars_left ? SLEEP : PROCEED;
  }
  OZ_Return vanish(void) {
    v.leave();
    for (int i = size; i--; vv[i].leave());
    return PROCEED;
  }
  OZ_Return fail(void) {
    v.fail();
    for (int i = size; i--; vv[i].fail());
    return FAILED;
  }
};

class PropagatorController_V {
protected:
  OZ_FDIntVar &v1;
public:
  PropagatorController_V(OZ_FDIntVar &i1) : v1(i1){}

  OZ_Return leave(void) {
    return v1.leave()? SLEEP : PROCEED;
  }
  OZ_Return vanish(void) {
    v1.leave();
    return PROCEED;
  }
  OZ_Return fail(void) {
    v1.fail();
    return FAILED;
  }
};

template<class RTYPE, class VAR, int P, int F, int S>
class _PropagatorController_V_V : public _OZ_ParamIterator<RTYPE> {
protected:
  VAR &v1, &v2;
public:
  _PropagatorController_V_V(VAR &i1, VAR &i2)
    : v1(i1), v2(i2) {}

  RTYPE leave(int vars_left = 0) {
    return ((v1.leave()?1:0) + (v2.leave()?1:0) <= vars_left) ? P : S;
  }
  RTYPE vanish(void) {
    v1.leave();
    v2.leave();
    return P;
  }
  RTYPE fail(void) {
    v1.fail();
    v2.fail();
    return F;
  }
};

typedef _PropagatorController_V_V<OZ_Return,OZ_FDIntVar,PROCEED,FAILED,SLEEP>
PropagatorController_V_V;

template<class RTYPE, class VAR, int P, int F, int S>
class _PropagatorController_V_V_V : public _OZ_ParamIterator<RTYPE> {
protected:
  VAR &v1, &v2, &v3;
public:
  _PropagatorController_V_V_V(VAR &i1, VAR &i2, VAR &i3)
    : v1(i1), v2(i2), v3(i3) {}

  RTYPE leave(int vars_left = 0) {
    return ((v1.leave()?1:0) + (v2.leave()?1:0) + (v3.leave()?1:0) <= vars_left) ? P : S;
  }
  RTYPE vanish(void) {
    v1.leave();
    v2.leave();
    v3.leave();
    return P;
  }
  RTYPE fail(void) {
    v1.fail();
    v2.fail();
    v3.fail();
    return F;
  }
};

typedef _PropagatorController_V_V_V<OZ_Return,OZ_FDIntVar,PROCEED,FAILED,SLEEP> PropagatorController_V_V_V;

class PropagatorController_VV_V_V_V {
protected:
  OZ_FDIntVar * vv1, &v2, &v3, &v4;
  int size;
public:
  PropagatorController_VV_V_V_V(int s, OZ_FDIntVar i1[], OZ_FDIntVar &i2,
                                OZ_FDIntVar &i3, OZ_FDIntVar &i4)
    : size(s), vv1(i1), v2(i2), v3(i3), v4(i4) {}

  OZ_Return leave(void) {
    OZ_Boolean vars_left = v2.leave();
    vars_left |= v3.leave();
    vars_left |= v4.leave();
    for (int i = size; i--; vars_left |= vv1[i].leave());
    return vars_left ? SLEEP : PROCEED;
  }
  OZ_Return vanish(void) {
    for (int i = size; i--; vv1[i].leave());
    v2.leave();
    v3.leave();
    v4.leave();
    return PROCEED;
  }
  OZ_Return fail(void) {
    for (int i = size; i--; vv1[i].fail());
    v2.fail();
    v3.fail();
    v4.fail();
    return FAILED;
 }
};

//-----------------------------------------------------------------------------


class FiniteDomainIterator {
private:
  OZ_FiniteDomain * finiteDomain;
  int current;
  int size;
public:
  FiniteDomainIterator(OZ_FiniteDomain * fd) : finiteDomain(fd) {}

  int resetToMin(void) {
    size = finiteDomain->getSize() - 1;
    return current = finiteDomain->getMinElem();
  }

  int nextLarger(void) {
    if (size > 0) {
      size -= 1;
      return current = finiteDomain->getNextLargerElem(current);
    } else {
      return -1;
    }
  }
};

//-----------------------------------------------------------------------------

class FDIntVarArr2 {
protected:
  OZ_FDIntVar ** vars;
  int size, * sizes;
public:
  FDIntVarArr2(int s, OZ_FDIntVar *vfs[], int ss[])
    : size(s), vars(vfs), sizes(ss) {
    for (int i = size; i--; )
      vars[i] = new OZ_FDIntVar[sizes[i]];
  }
  OZ_FDIntVar * operator [](int i) {return vars[i];}

  // single_var[i] = -2 : nonlin
  //               = -1 : determined
  //               > -1 : linear
  OZ_Return leave(int single_var[], OZ_Boolean &is_lin) {
    OZ_Boolean vars_left = OZ_FALSE;
    is_lin = OZ_TRUE;

    for (int i = size; i--; ) {
      single_var[i] = -1;
      for (int j = sizes[i]; j--; ) {
        vars_left |= vars[i][j].leave();
        if (vars[i][j]->getSize() > 1 && single_var[i] > -2)
          single_var[i] = (single_var[i] == -1 ? j : -2);
      }
      is_lin &= (single_var[i] != -2);
    }
    return vars_left ? SLEEP : PROCEED;
  }
  OZ_Return vanish(void) {
    for (int i = size; i--; )
      for (int j = sizes[i]; j--; )
        vars[i][j].leave();
    return PROCEED;
  }
  OZ_Return fail(void) {
    for (int i = size; i--; )
      for (int j = sizes[i]; j--; )
        vars[i][j].fail();
    return FAILED;
  }
};

//-----------------------------------------------------------------------------

int * vectorToInts(OZ_Term, int &);
int * vectorToInts1(OZ_Term, int &);
OZ_Term * vectorToOzTerms(OZ_Term, int &);
OZ_Term * vectorToOzTerms(OZ_Term, OZ_Term, int &);
void vectorToOzTerms(OZ_Term, OZ_Term *);
void vectorToLinear(OZ_Term, int &, OZ_Term &);

class VectorIterator {
private:
  OZ_Term * _vector;
  int _size, _counter;
public:
  VectorIterator(OZ_Term v) : _counter(0) {
    _size = OZ_vectorSize(v);
    _vector = new OZ_Term[_size];
    OZ_DEBUGCODE(OZ_Term * __a =) OZ_getOzTermVector(v, _vector);
    OZ_ASSERT((__a - _vector) == _size);
  }
  ~VectorIterator(void) { delete [] _vector; }
  void reset(void) { _counter = 0; }
  int anyLeft(void) { return _counter < _size; }
  OZ_Term getNext(void) { return _counter < _size ? _vector[_counter++] : 0; }
};


/* cannot handle sometimes arrays of size 0 correctly */
#define DECL_DYN_ARRAY(Type,Var,Size) \
_DECL_DYN_ARRAY(Type,Var,Size==0?1:Size)

#define INIT_FUNC(F_NAME) void F_NAME(void)

INIT_FUNC(fdp_init);
INIT_FUNC(sched_init);

//-----------------------------------------------------------------------------
#endif
