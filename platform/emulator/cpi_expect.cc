/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "cpi.hh"

inline
OZ_expect_t expectFail(void) {
  return OZ_expect_t(0, -1);
}

inline
OZ_expect_t expectProceed(int s, int a) {
  return OZ_expect_t(s, a);
}

inline
OZ_expect_t expectSuspend(int s, int a)
{
  Assert(s > a);
  return OZ_expect_t(s, a);
}

// expect_t.accepted:
//   -1 on failure, at least one term is inconsistent
//    0 on unsuffient constraints
// >= 1 number of terms sufficiently constrained
// args are sufficiently constrained if expect_t.size == expect_t.accepted

//-----------------------------------------------------------------------------

OZ_Expect::OZ_Expect(void)
{
  staticSpawnVarsNumber = staticSuspendVarsNumber = 0;
  collect = TRUE;
}

OZ_Expect::~OZ_Expect(void)
{
}

void OZ_Expect::collectVarsOn(void)
{
  collect = TRUE;
}

void OZ_Expect::collectVarsOff(void)
{
  collect = FALSE;
}

// variables are suffiently constrained
inline
void OZ_Expect::addSpawn(OZ_FDPropState ps, OZ_Term * v)
{
  if (collect)
    staticAddSpawn(ps, v);
}

inline
void OZ_Expect::addSpawn(OZ_FSetPropState ps, OZ_Term * v)
{
  if (collect)
    staticAddSpawn(ps, v);
}

// variables are insuffiently constrained

inline
void OZ_Expect::addSuspend(OZ_Term * v)
{
  if (collect) {
    staticSuspendVars[staticSuspendVarsNumber].var = v;
#ifdef Assert
    staticSuspendVars[staticSuspendVarsNumber++].expected_type = NonGenCVariable;
#else
    staticSuspendVarsNumber++;
#endif

#ifdef GAGA
    staticSuspendVars.request(staticSuspendVarsNumber);
#else
    Assert(staticSuspendVarsNumber < CPIINITSIZE);
#endif
  }
}


inline
void OZ_Expect::addSuspend(OZ_FDPropState ps, OZ_Term * v)
{
  if (collect) {
    staticSuspendVars[staticSuspendVarsNumber].var = v;
    staticSuspendVars[staticSuspendVarsNumber].expected_type = FDVariable;
    staticSuspendVars[staticSuspendVarsNumber++].state.fd = ps;

#ifdef GAGA
    staticSuspendVars.request(staticSuspendVarsNumber);
#else
    Assert(staticSuspendVarsNumber < CPIINITSIZE);
#endif
  }
}

inline
void OZ_Expect::addSuspend(OZ_FSetPropState ps, OZ_Term * v)
{
  if (collect) {
    staticSuspendVars[staticSuspendVarsNumber].var = v;
    staticSuspendVars[staticSuspendVarsNumber].expected_type = FSetVariable;
    staticSuspendVars[staticSuspendVarsNumber++].state.fs = ps;

#ifdef GAGA
    staticSuspendVars.request(staticSuspendVarsNumber);
#else
    Assert(staticSuspendVarsNumber < CPIINITSIZE);
#endif
  }
}

//*****************************************************************************
// member functions related to finite domain constraints
//*****************************************************************************

//-----------------------------------------------------------------------------
// An OZ term describing a finite domain is either:
// (1) a positive small integer <= FD.sup
// (2) a 2 tuple of (1)
// (3) a non-empty list of (1) and/or (2) or (1) or (2)
// (4) a tuple compl() of 4

OZ_expect_t OZ_Expect::expectDomDescr(OZ_Term descr, int level)
{
  DEREF(descr, descr_ptr, descr_tag);

  if (level >= 4) {
    if (isAnyVar(descr_tag)) {
      addSuspend(descr_ptr);
      return expectSuspend(1, 0);
    } else if (isSTuple(descr) && tagged2SRecord(descr)->getWidth() == 1 &&
               AtomCompl == tagged2SRecord(descr)->getLabel()) {
      return expectDomDescr(makeTaggedRef(&(*tagged2SRecord(descr))[0]), 3);
    }
    level = 3;
  }

  if (isNotCVar(descr_tag)) {
    addSuspend(descr_ptr);
    return expectSuspend(1, 0);
  } else if (isPosSmallFDInt(descr) && (level >= 1)) { // (1)
    return expectProceed(1, 1);
  } else if (isGenFDVar(descr, descr_tag) && (level >= 1)) {
    addSuspend(fd_prop_singl, descr_ptr);
    return expectSuspend(1, 0);
  } else if (isGenBoolVar(descr, descr_tag) && (level >= 1)) {
    addSuspend(descr_ptr);
    return expectSuspend(1, 0);
  } else if (isSTuple(descr) && (level >= 2)) {
    SRecord &tuple = *tagged2SRecord(descr);
    if (tuple.getWidth() != 2)
      return expectFail();
    for (int i = 0; i < 2; i++) {
      OZ_expect_t r = expectDomDescr(makeTaggedRef(&tuple[i]), 1);
      if (isSuspending(r) || isFailing(r))
        return r;
    }
    return expectProceed(1, 1);
  } else if (isLTuple(descr_tag) && (level == 3)) {

    do {
      LTuple &list = *tagged2LTuple(descr);
      OZ_expect_t r = expectDomDescr(makeTaggedRef(list.getRefHead()), 2);
      if (isSuspending(r) || isFailing(r))
        return r;
      descr = makeTaggedRef(list.getRefTail());

      __DEREF(descr, descr_ptr, descr_tag);
    } while (isLTuple(descr_tag));

    if (isNil(descr)) return expectProceed(1, 1);
    return expectDomDescr(makeTaggedRef(descr_ptr), 0);
  }
  return expectFail();

}

OZ_expect_t OZ_Expect::expectIntVar(OZ_Term t, OZ_FDPropState ps)
{
  DEREF(t, tptr, ttag);

  if (isPosSmallFDInt(t)) {
    return expectProceed(1, 1);
  } else if (isGenBoolVar(t, ttag) || isGenFDVar(t, ttag)) {
    addSpawn(ps, tptr);
    return expectProceed(1, 1);
  } else if (isNotCVar(ttag)) {
    addSuspend(ps, tptr);
    return expectSuspend(1, 0);
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectInt(OZ_Term t)
{
  Assert(isRef(t) || !isAnyVar(t));

  DEREF(t, tptr, ttag);

  if (isSmallInt(ttag)) {
    return expectProceed(1, 1);
  } else if (isAnyVar(ttag)) {
    addSuspend(fd_prop_singl, tptr);
    return expectSuspend(1, 0);
  }
  return expectFail();
}

//*****************************************************************************
// member functions related to finite set constraints
//*****************************************************************************

//-----------------------------------------------------------------------------
// An OZ term describing a finite set is either:
// (1) a positive small integer <= FS.sup
// (2) a 2 tuple of (1)
// (3) a possibly empty list of (1) and/or (2)

OZ_expect_t OZ_Expect::_expectFSetDescr(OZ_Term descr, int level)
{
  DEREF(descr, descr_ptr, descr_tag);

  if (isNotCVar(descr_tag)) {
    addSuspend(descr_ptr);
    return expectSuspend(1, 0);
  } else if (isPosSmallSetInt(descr) && (level == 1 || level == 2)) { // (1)
    return expectProceed(1, 1);
  } else if (isGenFDVar(descr, descr_tag) && (level == 1 || level == 2)) {
    addSuspend(fd_prop_singl, descr_ptr);
    return expectSuspend(1, 0);
  } else if (isGenBoolVar(descr, descr_tag) && (level == 1 || level == 2)) {
    addSuspend(descr_ptr);
    return expectSuspend(1, 0);
  } else if (isSTuple(descr) && (level == 2)) {
    SRecord &tuple = *tagged2SRecord(descr);
    if (tuple.getWidth() != 2)
      return expectFail();
    for (int i = 0; i < 2; i++) {
      OZ_expect_t r = expectDomDescr(makeTaggedRef(&tuple[i]), 1);
      if (isSuspending(r) || isFailing(r))
        return r;
    }
    return expectProceed(1, 1);
  } else if (isNil(descr) && (level == 3)) {
    return expectProceed(1, 1);
  } else if (isLTuple(descr_tag) && (level == 3)) {

    do {
      LTuple &list = *tagged2LTuple(descr);
      OZ_expect_t r = expectDomDescr(makeTaggedRef(list.getRefHead()), 2);
      if (isSuspending(r) || isFailing(r))
        return r;
      descr = makeTaggedRef(list.getRefTail());

      __DEREF(descr, descr_ptr, descr_tag);
    } while (isLTuple(descr_tag));

    if (isNil(descr)) return expectProceed(1, 1);
    return expectDomDescr(makeTaggedRef(descr_ptr), 0);
  }
  return expectFail();

}

OZ_expect_t OZ_Expect::expectFSetVar(OZ_Term t, OZ_FSetPropState ps)
{
  DEREF(t, tptr, ttag);

  if (isFSetValue(ttag)) {
    return expectProceed(1, 1);
  } else if (isGenFSetVar(t, ttag)) {
    addSpawn(ps, tptr);
    return expectProceed(1, 1);
  } else if (isNotCVar(ttag)) {
    addSuspend(ps, tptr);
    return expectSuspend(1, 0);
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectFSetValue(OZ_Term t)
{
  Assert(isRef(t) || !isAnyVar(t));

  DEREF(t, tptr, ttag);

  if (isFSetValue(ttag)) {
    return expectProceed(1, 1);
  } else if (isAnyVar(ttag)) {
    addSuspend(fs_prop_val, tptr);
    return expectSuspend(1, 0);
  }
  return expectFail();
}

//*****************************************************************************

OZ_expect_t OZ_Expect::expectVar(OZ_Term t)
{
  DEREF(t, tptr, ttag);

  if (isAnyVar(ttag)) {
    addSpawn(fd_prop_any, tptr);
    return expectProceed(1, 1);
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectRecordVar(OZ_Term t)
{
  DEREF(t, tptr, ttag);

  if (isRecord(t)) {
    return expectProceed(1, 1);
  } else if (isGenOFSVar(t, ttag)) {
    addSpawn(fd_prop_any, tptr);
    return expectProceed(1, 1);
  } else if (isNotCVar(ttag)) {
    addSuspend(tptr);
    return expectSuspend(1, 0);
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectLiteral(OZ_Term t)
{
  Assert(isRef(t) || !isAnyVar(t));

  DEREF(t, tptr, ttag);

  if (isLiteral(ttag)) {
    return expectProceed(1, 1);
  } else if (isAnyVar(ttag)) {
    addSuspend(tptr);
    return expectSuspend(1, 0);
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectVector(OZ_Term t,
                                    OZ_expect_t(OZ_Expect::* expectf)(OZ_Term))
{
  Assert(isRef(t) || !isAnyVar(t));

  DEREF(t, tptr, ttag);

  if (isLiteral(ttag)) { // subsumes nil
    return expectProceed(1, 1);
  } else if (isSTuple(t) || isSRecord(t)) {

    SRecord & tuple = *tagged2SRecord(t);
    int width = tuple.getWidth(), acc = 1;

    for (int i = width; i--; ) {
      OZ_expect_t r = (this->*expectf)(makeTaggedRef(&tuple[i]));
      if (r.accepted == -1) {
        return r;
      } else if (r.accepted == r.size) {
        acc += 1;
      }
    }
    return expectProceed(width + 1, acc);

  } else if (isCons(ttag)) {

    int len = 0, acc = 0;

    do {
      len += 1;
      OZ_expect_t r = (this->*expectf)(makeTaggedRef(headRef(t)));

      if (r.accepted == -1) {
        return r;
      } else if (r.accepted == r.size) {
        acc += 1;
      } /* else {
        Assert(r.accepted < r.size);
        return expectSuspend(len, acc);
      } */

      t = tail(t);
      __DEREF(t, tptr, ttag);
    } while (isCons(ttag));

    if (isNil(t)) {
      return expectProceed(len, acc);
    } else if (isAnyVar(ttag)) {
      addSuspend(tptr);
      return expectSuspend(len+1, acc);
    } else {
      return expectFail();
    }

  } else if (isAnyVar(ttag)) {
    addSuspend(tptr);
    return expectSuspend(1, 0);
  }
  return expectFail();
}


OZ_expect_t OZ_Expect::expectStream(OZ_Term st)
{
  Assert(isRef(st) || !isAnyVar(st));

  DEREF(st, stptr, sttag);

  if (isNotCVar(sttag)) {
    addSpawn(fd_prop_any, stptr);
    return expectProceed(1, 1);
  } else if (isNil(st)) {
    return expectProceed(1, 1);
  } else if (isCons(sttag)) {

    int len = 0;

    do {
      len += 1;
      st = tail(st);
      __DEREF(st, stptr, sttag);
    } while (isCons(sttag));

    if (isNil(st)) {
      return expectProceed(len, len);
    } else if (isNotCVar(sttag)) {
      addSpawn(fd_prop_any, stptr);
      return expectProceed(len, len);
    }
  }

  return expectFail();
}

//-----------------------------------------------------------------------------

OZ_Return OZ_Expect::suspend(OZ_Thread th)
{
  Assert(staticSuspendVarsNumber > 0);
#ifdef FDBISTUCK
  for (int i = staticSuspendVarsNumber; i--; )
    am.addSuspendVarList(staticSuspendVars[i].var);
  return SUSPEND;
#else
  for (int i = staticSuspendVarsNumber; i--; )
    OZ_addThread (makeTaggedRef(staticSuspendVars[i].var), th);
  return PROCEED;
#endif
}

OZ_Return OZ_Expect::fail(void)
{
  return FAILED;
}


//*****************************************************************************
// member function to spawn a propagator
//*****************************************************************************

OZ_Return OZ_Expect::impose(OZ_Propagator * p, int prio,
                            OZ_PropagatorFlags flags)
{
// do initial run with dummy thread

  // Constrain all SVARs and UVARs in staticSuspendVars to FDVARs before
  // OZ_Propagator::run is run.
  int i;
  for (i = staticSuspendVarsNumber; i--; ) {
    OZ_Term v = makeTaggedRef(staticSuspendVars[i].var);
    DEREF(v, vptr, vtag);
    TypeOfGenCVariable type = staticSuspendVars[i].expected_type;

    Assert(type == FDVariable || type == FSetVariable);

    if (isNotCVar(vtag)) {

      if (type == FDVariable) {
        tellBasicConstraint(makeTaggedRef(vptr), (OZ_FiniteDomain *) NULL);
      } else {
        Assert(type == FSetVariable);
        tellBasicConstraint(makeTaggedRef(vptr), (OZ_FSet *) NULL);
      }
    }
  }

  Thread * thr = am.mkPropagator(am.currentBoard, prio, p);

  ozstat.propagatorsCreated.incf();
  ozstat.propagatorsInvoked.incf();

  Thread * backup_currentThread = am.currentThread;
  am.currentThread = thr;
  switch (thr->runPropagator()) {
  case FAILED:
    thr->closeDonePropagator();
    am.currentThread = backup_currentThread;
    staticSpawnVarsNumber = staticSuspendVarsNumber = 0;
    return FAILED;
  case SLEEP:
    thr->suspendPropagator();
    break;
  case SCHEDULED:
    thr->scheduledPropagator();
    break;
  case PROCEED:
    thr->closeDonePropagator();
    am.currentThread = backup_currentThread;
    staticSpawnVarsNumber = staticSuspendVarsNumber = 0;
    return PROCEED;
  default:
    error("Unexpected return value.");
  }
  am.currentThread = backup_currentThread;

// only if a propagator survives its first run proper suspension are created
  OZ_Boolean all_local = OZ_TRUE;

  // caused by the first run of the run method variables may have
  // become determined
  for (i = staticSpawnVarsNumber; i--; ) {
    OZ_Term v = makeTaggedRef(staticSpawnVars[i].var);
    DEREF(v, vptr, vtag);

    if (isAnyVar(vtag)) {

      Assert(!isCVar(vtag) ||
             (!testResetStoreFlag(v) && !testResetReifiedFlag(v)));

      if (isGenFDVar(v, vtag)) {
        addSuspFDVar(v, thr, staticSpawnVars[i].state.fd);
        all_local &= am.isLocalCVar(v);
      } else if (isGenFSetVar(v, vtag)) {
        addSuspFSetVar(v, thr, staticSpawnVars[i].state.fs);
        all_local &= am.isLocalCVar(v);
      } else if (isGenOFSVar(v, vtag)) {
        addSuspOFSVar(v, thr);
        all_local &= am.isLocalCVar(v);
      } else if (isGenBoolVar(v, vtag)) {
        addSuspBoolVar(v, thr);
        all_local &= am.isLocalCVar(v);
      } else if (isSVar(vtag)) {
        addSuspSVar(v, thr);
        all_local &= am.isLocalSVar(v);
      } else {
        Assert(isUVar(vtag));
        addSuspUVar(vptr, thr);
        all_local &= am.isLocalUVar(v);
      }
    }
  }

  // Note all SVARs and UVARs in staticSuspendVars are constrained to FDVARs.
  for (i = staticSuspendVarsNumber; i--; ) {
    OZ_Term v = makeTaggedRef(staticSuspendVars[i].var);
    DEREF(v, vptr, vtag);

    if (isAnyVar(vtag)) {
      Assert(isCVar(vtag));

      addSuspCVar(v, thr);
      all_local &= am.isLocalCVar(v);
    }
  }

  if (all_local)
    thr->markLocalThread();

  switch (flags) {
  case OFS_flag:
    thr->setOFSThread();
    break;
  case NULL_flag:
    break;
  default:
    warning("Unrecognized flag found when spawning propagator");
  }

  staticSpawnVarsNumber = staticSuspendVarsNumber = 0;
  return PROCEED;
}

// End of File
//-----------------------------------------------------------------------------
