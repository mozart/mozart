/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "../oz_cpi.hh" // TMUELLER

#ifndef __FSAUX_HH__
#define __FSAUX_HH__

//#define OZ_DEBUG

//-----------------------------------------------------------------------------
// debug macros

#ifdef DEBUG_FSET
#define OZ_DEBUG
#endif

#ifdef OZ_DEBUG
#define OZ_DEBUGCODE(C) C
#define _OZ_DEBUGPRINT(C) (*cpi_cout) << C << endl << flush 
#define OZ_DEBUGPRINT(C) _OZ_DEBUGPRINT(C) 
#define OZ_ASSERT(C)						\
  if (! (C)) {							\
    cerr << "OZ_ASSERT " << #C << " failed (" __FILE__ << ':'   \
	 << __LINE__ << ")." << endl << flush;	                \
  }
#define _OZ_DEBUGRETURNPRINT(X) __debugReturnPrint(X)
#define OZ_DEBUGRETURNPRINT(X) X /* _OZ_DEBUGRETURNPRINT(X) */
#else
#define OZ_DEBUGCODE(C)
#define _OZ_DEBUGPRINT(C)
#define OZ_DEBUGPRINT(C)
#define OZ_ASSERT(C)
#define OZ_DEBUGRETURNPRINT(X) X
#define _OZ_DEBUGRETURNPRINT(X) X
#endif

inline
OZ_Return __debugReturnPrint(OZ_Return r) 
{
  *cpi_cout << "returning: ";
  switch (r) {
  case FAILED: 
    *cpi_cout << "FAILED";
    break;
  case OZ_ENTAILED:
    *cpi_cout << "ENTAILED";
    break;
  case SLEEP:
    *cpi_cout <<"SLEEP";
    break;
  default:
    *cpi_cout << "??? (" << r << ")";
    break;
  }
  *cpi_cout << endl << flush;
  return r;
}
//-----------------------------------------------------------------------------

#define FailOnEmpty(X) if((X) == 0) goto failure;
#define FailOnInvalid(X) if(!(X)) goto failure;

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
};

//-----------------------------------------------------------------------------

class PropagatorController_S_S {
protected:
  OZ_FSetVar &v1, &v2;
public:
  PropagatorController_S_S(OZ_FSetVar &i1, OZ_FSetVar &i2) 
    : v1(i1), v2(i2) {}

  OZ_Return leave(void) {
    return (v1.leave() | v2.leave()) ? SLEEP : OZ_ENTAILED;
  }
  OZ_Return leave1(void) {
    int r1 = v1.leave() ? 1 : 0;
    int r2 = v2.leave() ? 1 : 0;
    return (r1 + r2 <= 0) ? OZ_ENTAILED : SLEEP; // TMUELLER
  }
  OZ_Return vanish(void) {
    v1.leave();
    v2.leave();
    return OZ_ENTAILED;
  }
  OZ_Return fail(void) {
    v1.fail();
    v2.fail();
    return FAILED;
  }
};

class PropagatorController_S_S_S {
protected:
  OZ_FSetVar &v1, &v2, &v3;
public:
  PropagatorController_S_S_S(OZ_FSetVar &i1, OZ_FSetVar &i2, OZ_FSetVar &i3)
    : v1(i1), v2(i2), v3(i3) {}

  OZ_Return leave(void) {
    return (v1.leave()|v2.leave()|v3.leave())  ? SLEEP : OZ_ENTAILED;
  }
  OZ_Return leave1(void) {
    int r1 = v1.leave() ? 1 : 0;
    int r2 = v2.leave() ? 1 : 0;
    int r3 = v3.leave() ? 1 : 0;
    return (r1 + r2 + r3 <= 0) ? OZ_ENTAILED : SLEEP; // TMUELLER
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
    return FAILED;
  }
};

class PropagatorController_S_D {
protected:
  OZ_FSetVar  &v1;
  OZ_FDIntVar &v2;
public:
  PropagatorController_S_D(OZ_FSetVar &i1, OZ_FDIntVar &i2) 
    : v1(i1), v2(i2) {}

  OZ_Return leave(void) {
    return (v1.leave() | v2.leave()) ? SLEEP : OZ_ENTAILED;
  }
  OZ_Return leave1(void) {
    int r1 = v1.leave() ? 1 : 0;
    int r2 = v2.leave() ? 1 : 0;
    return (r1 + r2 <= 1) ? OZ_ENTAILED : SLEEP;
  }
  OZ_Return vanish(void) {
    v1.leave();
    v2.leave();
    return OZ_ENTAILED;
  }
  OZ_Return fail(void) {
    v1.fail();
    v2.fail();
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

  OZ_Boolean operator <= (OZ_FSetVar &sv) 
  {
    if ((_known_in < sv->getKnownIn()) ||
	(_known_not_in < sv->getKnownNotIn()) ||
	(_card_size > sv->getCardSize()))
      return OZ_TRUE;

    return OZ_FALSE;
  }
};

#endif /* __FSAUX_HH__ */

//-----------------------------------------------------------------------------
// eof
