/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __FDAUX_HH__
#define __FDAUX_HH__

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "../oz_cpi.hh" // TMUELLER

//-----------------------------------------------------------------------------
// debug macros

#ifdef DEBUG_FD
#define OZ_DEBUG
#endif

#ifdef OZ_DEBUG
#define OZ_DEBUGCODE(C) C

extern "C" void oz_debugprint(char *format ...);

#define _OZ_DEBUGPRINT(C) oz_debugprint C
#define OZ_DEBUGPRINT(C) /* _OZ_DEBUGPRINT(C) */
#define OZ_ASSERT(C)					\
  if (! (C)) {						\
    fprintf(stderr,"OZ_ASSERT %s failed (%s:%d).\n",	\
	    #C,__FILE__, __LINE__);			\
    fflush(stderr);					\
  }

#else
#define OZ_DEBUGCODE(C)
#define _OZ_DEBUGPRINT(C)
#define OZ_DEBUGPRINT(C)
#define OZ_ASSERT(C)
#endif

#define _OZ_DEBUGPRINTTHIS(string) 		\
   _OZ_DEBUGPRINT(("%s%s",string,this->toString()))

#define OZ_DEBUGPRINTTHIS(string) /* _OZ_DEBUGPRINTTHIS(string) */

//-----------------------------------------------------------------------------
// misc macros

#define SAMELENGTH_VECTORS(I, J)					\
  { 									\
    int i_size = OZ_vectorSize(OZ_args[I]);				\
    int j_size = OZ_vectorSize(OZ_args[J]);				\
    if ((i_size >= 0) && (j_size >= 0) && (i_size != j_size))		\
      return OZ_typeError(expectedType, J,	                        \
	  		  "Vectors must have same size.");		\
  }

#define NUMBERCAST double

#define SUM_OP_EQ  "=:"
#define SUM_OP_NEQ "\\=:"
#define SUM_OP_LEQ "=<:"
#define SUM_OP_LT  "<:"
#define SUM_OP_GEQ ">=:"
#define SUM_OP_GT  ">:"

#define ERROR_UNEXPECTED_OPERATOR(P)					\
return OZ_typeError(expectedType, P, 	                                \
		    "Expected one of the following: "			\
		    SUM_OP_EQ " , " SUM_OP_NEQ " , " SUM_OP_LEQ " , "	\
		    SUM_OP_LT " , " SUM_OP_GEQ " or " SUM_OP_GT ".")

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

#define _TERMVECTOR2LIST(V, S, L, C)		\
OZ_Term L = OZ_nil();				\
{for (int i = S; i--; )				\
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
  OZ_expect_t expectIntVarMinMax(OZ_Term t) {
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
  
  OZ_expect_t expectVectorInt(OZ_Term t) {
    return expectVector(t, (PropagatorExpectMeth) &OZ_Expect::expectInt);
  }
  OZ_expect_t expectVectorLiteral(OZ_Term t) {
    return expectVector(t, (PropagatorExpectMeth) &OZ_Expect::expectLiteral);
  }
  OZ_expect_t expectVectorIntVarMinMax(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectIntVarMinMax);
  }
  OZ_expect_t expectVectorIntVarSingl(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectIntVarSingl);
  }
  OZ_expect_t expectVectorIntVarAny(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectIntVarAny);
  }
  OZ_expect_t expectVectorVectorIntVarMinMax(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectVectorIntVarMinMax);
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
    return expectProperRecord(t, &PropagatorExpect::expectInt);
  }
};

//-----------------------------------------------------------------------------

class PropagatorController_VV {
protected:
  OZ_FDIntVar * vv;
  int size;
public:
  PropagatorController_VV(int s, OZ_FDIntVar i[]) : size(s), vv(i) {}

  OZ_Return leave(void) {
    OZ_Boolean vars_left = OZ_FALSE;
    for (int i = size; i--; vars_left |= vv[i].leave());
    return vars_left ? SLEEP : PROCEED;
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

class PropagatorController_V_V {
protected:
  OZ_FDIntVar &v1, &v2;
public:
  PropagatorController_V_V(OZ_FDIntVar &i1, OZ_FDIntVar &i2) 
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

class PropagatorController_V_V_V {
protected:
  OZ_FDIntVar &v1, &v2, &v3;
public:
  PropagatorController_V_V_V(OZ_FDIntVar &i1, OZ_FDIntVar &i2, OZ_FDIntVar &i3)
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

//-----------------------------------------------------------------------------
// Dynamically sized arrays for compilers which do not provide alloca


#ifdef __GNUC__
#define USE_GCCALLOCA
#else
#ifdef __XXWATCOMC__
#define USE_INTVAR_NEW
#else
#ifdef _MSC_VER
#define USE_TEMPLATE_ARRAY
#else
#define USE_TEMPLATE_ARRAY
#endif
#endif
#endif


#ifdef DEBUG_INDICES
template <class T>
class IndexCheckArray {
private:
  int _size;
  T * _array;
public:
  IndexCheckArray(int s)  { 
    _size = s; 
    _array = (T *) malloc(s * sizeof(T));
  }
  
  inline
  T &operator [](int i) { 
    OZ_ASSERT(0 <= i && i < _size);
    return _array[i]; 
  }
  
  inline 

  inline
  operator T*() { return _array; } // conversion operator
};

#define _DECL_DYN_ARRAY(Type,Var,Size) IndexCheckArray<Type> Var(Size)
#else

/* gcc supports dynamic sized arrays */
#ifdef USE_GCCALLOCA
#define _DECL_DYN_ARRAY(Type,Var,Size) Type Var[Size]
#endif


#ifdef USE_INTVAR_NEW
#define _DECL_DYN_ARRAY(Type,Var,Size) \
     Type *Var = (Type *) OZ_FDIntVar::operator new(sizeof(Type) * Size)
#endif

#ifdef USE_ALLOCA
#define _DECL_DYN_ARRAY(Type,Var,Size) \
     Type *Var = (Type *) alloca(sizeof(Type) * Size)
#endif


/* this one is really slooow */
#ifdef USE_TEMPLATE_ARRAY

#define _DECL_DYN_ARRAY(Type,Var,Size) _DynArray<Type> Var(Size)

void * freeListMallocOutline(size_t chunk_size);
void freeListDisposeOutline(void *addr, size_t chunk_size);

template <class T>
class _DynArray {
private:
  T * array;
  int size;
public:
  _DynArray(int sz) { size = sz; array = (T*) freeListMallocOutline(sz*sizeof(T)); };
  ~_DynArray() { freeListDisposeOutline(array,size*sizeof(T)); }
  T &operator[] (int i) { return array[i]; }
  operator T * () { return array; }
};

#endif
#endif /* DEBUG_INDICES */

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
  
//-----------------------------------------------------------------------------
#endif

