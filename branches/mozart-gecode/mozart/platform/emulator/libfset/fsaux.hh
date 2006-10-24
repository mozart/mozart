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

#include "mozart_cpi.hh" // TMUELLER

#ifndef __FSAUX_HH__
#define __FSAUX_HH__

//#define DEBUG_INDICES

//-----------------------------------------------------------------------------
// debug macros

//#define OZ_DEBUG

#ifdef OZ_DEBUG
#define OZ_DEBUGCODE(C) C
#define OZ_NONDEBUGCODE(C)
extern "C" void oz_fsetdebugprint(char *format ...);
#define _OZ_DEBUGPRINT(C) printf C; printf("\n"); fflush(stdout)
#define OZ_DEBUGPRINT(C) /*_OZ_DEBUGPRINT(C)*/
#define OZ_ASSERT(C)					\
  if (! (C)) {						\
    fprintf(stderr,"OZ_ASSERT %s failed (%s:%d).\n",	\
	    #C,__FILE__, __LINE__);			\
    fflush(stderr);					\
    abort();						\
  }
#define _OZ_DEBUGRETURNPRINT(X) 		\
{						\
  OZ_Return ret = X;				\
  						\
  switch ret {					\
    case OZ_ENTAILED;				\
    printf("OZ_ENTAILED\n"); fflush(stdout);	\
    break;					\
    case OZ_SCHEDULED;				\
    printf("OZ_SCHEDULED\n"); fflush(stdout);	\
    break;					\
    case OZ_FAILED;				\
    printf("OZ_FAILED\n"); fflush(stdout);	\
    break;					\
    case OZ_SLEEP;				\
    printf("OZ_SLEEP\n"); fflush(stdout);	\
    break;					\
  }						\
  return ret;					\
}
#define OZ_DEBUGRETURNPRINT(X)  X /*_OZ_DEBUGRETURNPRINT(X) */
inline
OZ_Return __debugReturnPrint(OZ_Return r) 
{
  char *aux;
  switch (r) {
  case OZ_FAILED:   aux = "FAILED";   break;
  case OZ_ENTAILED: aux = "ENTAILED"; break;
  case OZ_SLEEP:    aux = "SLEEP";    break;
  default:
    oz_fsetdebugprint("returning: ??? (%d)",r);
    return r;
  }
  oz_fsetdebugprint("returning: %s",aux);
  return r;
}
#define _OZ_DEBUGPRINTTHIS(string) 		\
   _OZ_DEBUGPRINT(("%s%s",string,this->toString()))

#define OZ_DEBUGPRINTTHIS(string) /* _OZ_DEBUGPRINTTHIS(string) */
#else
#define OZ_DEBUGCODE(C)
#define OZ_NONDEBUGCODE(C) C
#define _OZ_DEBUGPRINT(C)
#define OZ_DEBUGPRINT(C)
#define OZ_ASSERT(C)
#define OZ_DEBUGRETURNPRINT(X) X
#define _OZ_DEBUGRETURNPRINT(X) X
#define _OZ_DEBUGPRINTTHIS(string)
#define OZ_DEBUGPRINTTHIS(string)
#endif


//-----------------------------------------------------------------------------

#define FailOnEmpty(X) if((X) == 0) goto failure;
#define FailOnInvalid(X) if(!(X)) goto failure;
#define FailOnInvalid3(L,OP,R)			\
{						\
  int __aux = (L).getLubCard();			\
  if(!(L OP (R))) goto failure;			\
  if((L).getLubCard() < __aux) 			\
    if (! (L).putCard(0, 0))			\
      goto failure;				\
  if((L).getGlbCard() > 0)			\
    if (! (L).putCard(__aux, __aux))		\
      goto failure;				\
}

#define SAMELENGTH_VECTORS(I, J)					\
  { 									\
    int i_size = OZ_vectorSize(OZ_in(I));				\
    int j_size = OZ_vectorSize(OZ_in(J));				\
    if ((i_size >= 0) && (j_size >= 0) && (i_size != j_size))		\
      return OZ_typeErrorCPI(expectedType, J,	                        \
	  		  "Vectors must have same size.");		\
  }

class PropagatorExpect;

typedef OZ_expect_t (PropagatorExpect::*PropagatorExpectMeth) (OZ_Term);

class PropagatorExpect : public OZ_Expect {
public:
  OZ_expect_t expectIntVarAny(OZ_Term t) { 
    return expectIntVar(t); 
  }
  OZ_expect_t expectIntVarMinMax(OZ_Term t) {
    return expectIntVar(t, fd_prop_bounds); 
  }
  OZ_expect_t expectFSetVarAny(OZ_Term t) { 
    return expectFSetVar(t, fs_prop_any); 
  }
  OZ_expect_t expectFSetVarGlb(OZ_Term t) { 
    return expectFSetVar(t, fs_prop_glb); 
  }
  OZ_expect_t expectFSetVarLub(OZ_Term t) { 
    return expectFSetVar(t, fs_prop_lub); 
  }
  OZ_expect_t expectFSetVarBounds(OZ_Term t) { 
    return expectFSetVar(t, fs_prop_bounds); 
  }
  OZ_expect_t expectVector(OZ_Term t, PropagatorExpectMeth expectf) {
    return OZ_Expect::expectVector(t, (OZ_ExpectMeth) expectf);
  }
  OZ_expect_t expectVectorIntVarMinMax(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectIntVarMinMax);
  }
  OZ_expect_t expectVectorFSetVarBounds(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectFSetVarBounds);
  }
  OZ_expect_t expectVectorFSetVarGlb(OZ_Term t) {
    return expectVector(t, &PropagatorExpect::expectFSetVarGlb);
  }
  OZ_expect_t expectVectorFSetValue(OZ_Term t) {
    return expectVector(t, (PropagatorExpectMeth) &PropagatorExpect::expectFSetValue);
  }
  OZ_expect_t expectVectorInt(OZ_Term t) {
    return expectVector(t, (PropagatorExpectMeth) &PropagatorExpect::expectInt);
  }
  OZ_expect_t expectVectorBoolVar(OZ_Term t) {
    return expectVector(t, (PropagatorExpectMeth) &PropagatorExpect::expectBoolVar);
  }
};

//-----------------------------------------------------------------------------

class PropagatorController_S_S : public OZ_ParamIterator {
protected:
  OZ_FSetVar &v1, &v2;
public:
  PropagatorController_S_S(OZ_FSetVar &i1, OZ_FSetVar &i2) 
    : v1(i1), v2(i2) {}

  OZ_Return leave(int vars_left = 0) {
    int r1 = v1.leave() ? 1 : 0;
    int r2 = v2.leave() ? 1 : 0;
    return (r1 + r2 <= vars_left) ? OZ_ENTAILED : OZ_SLEEP; 
  }
  OZ_Return vanish(void) {
    v1.leave();
    v2.leave();
    return OZ_ENTAILED;
  }
  OZ_Return fail(void) {
    v1.fail();
    v2.fail();
    return OZ_FAILED;
  }
};

class PropagatorController_S_S_S : public OZ_ParamIterator {
protected:
  OZ_FSetVar &v1, &v2, &v3;
public:
  PropagatorController_S_S_S(OZ_FSetVar &i1, OZ_FSetVar &i2, OZ_FSetVar &i3)
    : v1(i1), v2(i2), v3(i3) {}

  OZ_Return leave(int vars_left = 0) {
    int r1 = v1.leave() ? 1 : 0;
    int r2 = v2.leave() ? 1 : 0;
    int r3 = v3.leave() ? 1 : 0;
    return (r1 + r2 + r3 <= vars_left) ? OZ_ENTAILED : OZ_SLEEP;
  }
  OZ_Return vanish(void) {
    v1.leave();
    v2.leave();
    v3.leave();
    return OZ_ENTAILED;
  }
  OZ_Return fail(void) {
    v1.fail();
    v2.fail();
    v3.fail();
    return OZ_FAILED;
  }
};

class PropagatorController_S_D {
protected:
  OZ_FSetVar  &v1;
  OZ_FDIntVar &v2;
public:
  PropagatorController_S_D(OZ_FSetVar &i1, OZ_FDIntVar &i2) 
    : v1(i1), v2(i2) {}

  OZ_Return leave(int vars_left = 0) {
    int r1 = v1.leave() ? 1 : 0;
    int r2 = v2.leave() ? 1 : 0;
    return (r1 + r2 <= vars_left) ? OZ_ENTAILED : OZ_SLEEP;
  }
  OZ_Return vanish(void) {
    v1.leave();
    v2.leave();
    return OZ_ENTAILED;
  }
  OZ_Return fail(void) {
    v1.fail();
    v2.fail();
    return OZ_FAILED;
  }
};

class PropagatorController_S_D_D {
protected:
  OZ_FSetVar  &v1;
  OZ_FDIntVar &v2;
  OZ_FDIntVar &v3;
public:
  PropagatorController_S_D_D(OZ_FSetVar &i1, OZ_FDIntVar &i2, OZ_FDIntVar &i3) 
    : v1(i1), v2(i2), v3(i3) {}

  OZ_Return leave(void) {
    return (v1.leave() | v2.leave() | v3.leave()) ? OZ_SLEEP : OZ_ENTAILED;
  }
  OZ_Return vanish(void) {
    v1.leave();
    v2.leave();
    v3.leave();
    return OZ_ENTAILED;
  }
  OZ_Return fail(void) {
    v1.fail();
    v2.fail();
    v3.fail();
    return OZ_FAILED;
  }
};

class PropagatorController_VS_VD_VD {
protected:
  OZ_FSetVar  * _v1;
  OZ_FDIntVar * _v2;
  OZ_FDIntVar * _v3;
  int _vs_size;
public:
  PropagatorController_VS_VD_VD(int vs_size, OZ_FSetVar i1[], 
				OZ_FDIntVar i2[], OZ_FDIntVar i3[]) 
    : _vs_size(vs_size), _v1(i1), _v2(i2), _v3(i3) {}

  OZ_Return leave(void) {
    OZ_Boolean vars_left = OZ_FALSE;
    for (int i = _vs_size; i--; vars_left |= _v1[i].leave());
    for (int i1 = _vs_size; i1--; vars_left |= _v2[i1].leave());
    for (int i2 = _vs_size; i2--; vars_left |= _v3[i2].leave());

    return vars_left ? OZ_SLEEP : OZ_ENTAILED;
  }
  OZ_Return vanish(void) {
    for (int i3 = _vs_size; i3--; _v1[i3].leave());
    for (int i4 = _vs_size; i4--; _v2[i4].leave());
    for (int i5 = _vs_size; i5--; _v3[i5].leave());
    return OZ_ENTAILED;
  }
  OZ_Return fail(void) {
    for (int i6 = _vs_size; i6--; _v1[i6].fail());
    for (int i7 = _vs_size; i7--; _v2[i7].fail());
    for (int i8 = _vs_size; i8--; _v3[i8].fail());
    return OZ_FAILED;
  }
};

class PropagatorController_S_VD {
protected:
  OZ_FSetVar  & _s;
  OZ_FDIntVar * _vd;
  int _vd_size;
public:
  PropagatorController_S_VD(OZ_FSetVar &s, int vd_size, OZ_FDIntVar vd[]) 
    : _s(s), _vd_size(vd_size), _vd(vd){}

  OZ_Return leave(void) {
    OZ_Boolean vars_left = _s.leave();
    for (int i = _vd_size; i--; vars_left |= _vd[i].leave());
    return vars_left ? OZ_SLEEP : OZ_ENTAILED;
  }
  OZ_Return vanish(void) {
    _s.leave();
    for (int i = _vd_size; i--; _vd[i].leave());
    return OZ_ENTAILED;
  }
   OZ_Return fail(void) {
    _s.fail();
    for (int i = _vd_size; i--; _vd[i].fail());
    return OZ_FAILED;
  }
};

class PropagatorController_VS {
protected:
  OZ_FSetVar * _vs;
  int _vs_size;
public:
  PropagatorController_VS(int vs_size, OZ_FSetVar vs[]) 
    : _vs_size(vs_size), _vs(vs){}

  OZ_Return leave(void) {
    OZ_Boolean vars_left = OZ_FALSE;
    for (int i = _vs_size; i--; vars_left |= _vs[i].leave());
    return vars_left ? OZ_SLEEP : OZ_ENTAILED;
  }
  OZ_Return vanish(void) {
    for (int i = _vs_size; i--; _vs[i].leave());
    return OZ_ENTAILED;
  }
   OZ_Return fail(void) {
    for (int i = _vs_size; i--; _vs[i].fail());
    return OZ_FAILED;
  }
};

class PropagatorController_VS_S : public OZ_ParamIterator {
protected:
  OZ_FSetVar * _vs, _s;
  int _vs_size;
public:
  PropagatorController_VS_S(int vs_size, OZ_FSetVar vs[], OZ_FSetVar &s) 
    : _vs_size(vs_size), _vs(vs), _s(s){}

  OZ_Return leave(int j = 0) {
    int vars_left = _s.leave() ? 1 : 0;
    for (int i = _vs_size; i--; vars_left += _vs[i].leave() ? 1 : 0);
    return (vars_left <= j) ? OZ_ENTAILED : OZ_SLEEP;
  }
  OZ_Return vanish(void) {
    _s.leave();
    for (int i = _vs_size; i--; _vs[i].leave());
    return OZ_ENTAILED;
  }
   OZ_Return fail(void) {
    _s.fail();
    for (int i = _vs_size; i--; _vs[i].fail());
    return OZ_FAILED;
  }
};

class PropagatorController_VD {
protected:
  OZ_FDIntVar * _vd;
  int _vd_size;
public:
  PropagatorController_VD(int vd_size, OZ_FDIntVar vd[]) 
    : _vd_size(vd_size), _vd(vd){}

  OZ_Return leave(void) {
    OZ_Boolean vars_left = OZ_FALSE;
    for (int i = _vd_size; i--; vars_left |= _vd[i].leave());
    return vars_left ? OZ_SLEEP : OZ_ENTAILED;
  }
  OZ_Return vanish(void) {
    for (int i = _vd_size; i--; _vd[i].leave());
    return OZ_ENTAILED;
  }
   OZ_Return fail(void) {
    for (int i = _vd_size; i--; _vd[i].fail());
    return OZ_FAILED;
  }
};

class PropagatorController_VD_D {
protected:
  OZ_FDIntVar &v;
  OZ_FDIntVar * vv;
  int size;
public:
  PropagatorController_VD_D(int s, OZ_FDIntVar i1[], OZ_FDIntVar &i2)
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

//*****************************************************************************

class FSetTouched {
private:
  int _known_in, _known_not_in, _card_size;

public:
  FSetTouched(void) {};

  void operator = (OZ_FSetVar &sv) 
  {
    _known_in = sv->getKnownIn();
    _known_not_in = sv->getKnownNotIn();
    _card_size = sv->getCardSize();
  }

  FSetTouched(OZ_FSetVar &sv) { *this = sv; };

  OZ_Boolean operator <= (OZ_FSetVar &sv) 
  {
    return ((_known_in < sv->getKnownIn()) ||
	    (_known_not_in < sv->getKnownNotIn()) ||
	    (_card_size > sv->getCardSize()))
      ? OZ_TRUE : OZ_FALSE;
  }
  void operator = (OZ_FSetConstraint &sc) 
  {
    _known_in = sc.getKnownIn();
    _known_not_in = sc.getKnownNotIn();
    _card_size = sc.getCardSize();
  }

  OZ_Boolean operator <= (OZ_FSetConstraint &sc) 
  {
    return ((_known_in < sc.getKnownIn()) ||
	    (_known_not_in < sc.getKnownNotIn()) ||
	    (_card_size > sc.getCardSize()))
      ? OZ_TRUE : OZ_FALSE;
  }
};

class FSetTouchedGlb {
private:
  int _known_in;

public:
  FSetTouchedGlb(void) {};

  void operator = (OZ_FSetVar &sv) 
  {
    _known_in = sv->getKnownIn();
  }

  OZ_Boolean operator <= (OZ_FSetVar &sv) 
  {
    return _known_in < sv->getKnownIn() ? OZ_TRUE : OZ_FALSE;
  }
};

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


class FSetIterator {
private:
  OZ_FSetValue * _fset;
  int _elem;

public:
  FSetIterator(OZ_FSetValue * fs) : _fset(fs) {}
  FSetIterator(OZ_FSetValue * fs, int elem) : _elem(elem), _fset(fs) {}

  void init(int elem) { _elem = elem; }

  int resetToMin(void) {
    return _elem = _fset->getMinElem();
  }
  int resetToMax(void) {
    return _elem = _fset->getMaxElem();
  }

  int getNextLarger(void) {
    return _elem = _fset->getNextLargerElem(_elem);
  }
  int getNextSmaller(void) {
    return _elem = _fset->getNextSmallerElem(_elem);
  }
};

//-----------------------------------------------------------------------------

inline 
int min(int a, int b) { return a < b ? a : b; }

inline 
int max(int a, int b) { return a > b ? a : b; }

#define INIT_FUNC(F_NAME) void F_NAME(void)

INIT_FUNC(fsp_init);

#endif /* __FSAUX_HH__ */

//-----------------------------------------------------------------------------
// eof
