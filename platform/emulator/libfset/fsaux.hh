#include "../oz_cpi.hh" // TMUELLER

//-----------------------------------------------------------------------------
// debug macros

#ifdef DEBUG_FSET
#define OZ_DEBUG
#endif


#ifdef OZ_DEBUG
#define OZ_DEBUGCODE(C) C
#define _OZ_DEBUGPRINT(C) cout << C << endl << flush 
#define OZ_DEBUGPRINT(C) /* _OZ_DEBUGPRINT(C) */
#define OZ_ASSERT(C)						\
  if (! (C)) {							\
    cerr << "OZ_ASSERT " << #C << " failed (" __FILE__ << ':'   \
	 << __LINE__ << ")." << endl << flush;	                \
  }
#else
#define OZ_DEBUGCODE(C)
#define _OZ_DEBUGPRINT(C)
#define OZ_DEBUGPRINT(C)
#define OZ_ASSERT(C)
#endif

//-----------------------------------------------------------------------------

#define FailOnEmpty(X) if((X) == 0) goto failure;
#define FailOnInvalid(X) if(!(X)) goto failure;

class PropagatorExpect;

typedef OZ_expect_t (PropagatorExpect::*PropagatorExpectMeth) (OZ_Term);

class PropagatorExpect : public OZ_Expect {
public:
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
    return (v1.leave() | v2.leave()) ? SLEEP : ENTAILED;
  }
  OZ_Return leave1(void) {
    int r1 = v1.leave() ? 1 : 0;
    int r2 = v2.leave() ? 1 : 0;
    return (r1 + r2 <= 1) ? ENTAILED : SLEEP;
  }
  OZ_Return vanish(void) {
    v1.leave();
    v2.leave();
    return ENTAILED;
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
    return (v1.leave()|v2.leave()|v3.leave())  ? SLEEP : ENTAILED;
  }
  OZ_Return leave1(void) {
    int r1 = v1.leave() ? 1 : 0;
    int r2 = v2.leave() ? 1 : 0;
    int r3 = v3.leave() ? 1 : 0;
    return (r1 + r2 + r3 <= 1) ? ENTAILED : SLEEP;
  }
  OZ_Return vanish(void) {
    v1.leave();
    v2.leave();
    v3.leave();
    return ENTAILED;
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
    return (v1.leave() | v2.leave()) ? SLEEP : ENTAILED;
  }
  OZ_Return leave1(void) {
    int r1 = v1.leave() ? 1 : 0;
    int r2 = v2.leave() ? 1 : 0;
    return (r1 + r2 <= 1) ? ENTAILED : SLEEP;
  }
  OZ_Return vanish(void) {
    v1.leave();
    v2.leave();
    return ENTAILED;
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
