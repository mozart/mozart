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

// gc.cc: OZ_Propagator * OZ_Propagator::gc(void)

// propagators use free list heap memory
void * OZ_Propagator::operator new(size_t s)
{
  return freeListMalloc(s);
}

void OZ_Propagator::operator delete(void * p, size_t s)
{
  freeListDispose(p, s);
}

static void outputArgsList(ostream& o, OZ_Term args, Bool not_top)
{
  Bool not_first = FALSE;
  if (not_top) o << '[';

  for (; OZ_isCons(args); args = OZ_tail(args)) {
    OZ_Term h = OZ_head(args);
    if (not_first) o << ' ';

    DEREF(h, hptr, htag);
    switch (htag) {

    case LITERAL:
      o << tagged2Literal(h)->getPrintName();
      break;

    case LTUPLE:
      outputArgsList(o, h, TRUE);
      break;

    case SRECORD:
      {
        SRecord * st = tagged2SRecord(h);
        if (st->isTuple()) {
          int width = st->getWidth();
          o << tagged2Literal(st->getLabel())->getPrintName() << '/' << width;
        }
      }
      break;
    case UVAR: case SVAR:
      o << '_';
      break;

    case SMALLINT:
      o << smallIntValue(h);
      break;

    case FSETVALUE:
      o << *tagged2FSetValue(h);
      break;

    case CVAR:
      {
        char * n = getVarName(makeTaggedRef(hptr));
        o << (!n ? "_" : n);

        GenCVariable * cv = tagged2CVar(h);

        if (cv->testReifiedFlag()) {
          if (cv->isBoolPatched()) goto bool_lbl;
          if (cv->isFDPatched()) goto fd_lbl;
          if (cv->isFSetPatched()) goto fs_lbl;
          /*Assert(cv->isFDPatched()); goto ri_lbl;*/
        } else if (cv->getType() == FDVariable) {
        fd_lbl:
          o << ((GenFDVariable *) cv)->getDom();
        } else if (cv->getType() == BoolVariable) {
        bool_lbl:
          o << "{0#1}";
        } else if (cv->getType() == FSetVariable) {
        fs_lbl:
          o << ((GenFSetVariable *) cv)->getSet();
        } else {
          goto problem;
        }
      }
      break;

    default:
      goto problem;
    }

    not_first = TRUE;
  }

  if (!OZ_isNil(args)) goto problem;
  if (not_top) o << ']' << flush;
  return;

problem:
  OZ_warning("Unexpected term found in argument list "
             "of propagator while printing.");
}

ostream& operator << (ostream& o, const OZ_Propagator &p)
{
  char * func_name = builtinTab.getName((void *) p.getSpawner());
  OZ_Term args = p.getArguments();

#ifdef DEBUG_CHECK
  o << "cb(" << (void *) am.currentBoard << "), p(" << (void *) &p << ") ";
#endif

  o << '{' << func_name << ' ';
  outputArgsList(o, args, FALSE);
  o << '}' << flush;

  return o;
}

OZ_Boolean OZ_Propagator::mayBeEqualVars(void)
{
  return am.currentThread->isUnifyThread();
}


OZ_Return OZ_Propagator::replaceBy(OZ_Propagator * p)
{
  am.currentThread->setPropagator(p);
  return am.currentThread->runPropagator();
}

OZ_Return OZ_Propagator::replaceBy(OZ_Term a, OZ_Term b)
{
  return OZ_unify(a, b);
}

OZ_Return OZ_Propagator::replaceByInt(OZ_Term v, int i)
{
  return OZ_unify(v, newSmallInt(i));
}

OZ_Return OZ_Propagator::postpone(void)
{
  return SCHEDULED;
}

OZ_Boolean OZ_Propagator::postOn(OZ_Term t)
{
  DEREF(t, tptr, ttag);
  if (isAnyVar(ttag)) {
    addSuspAnyVar(tptr, new SuspList(am.currentThread));
    return OZ_TRUE;
  }
  return OZ_FALSE;
}

OZ_Boolean OZ_Propagator::addSpawn(OZ_FDPropState ps, OZ_Term v)
{
  DEREF(v, vptr, vtag);
  if (!isAnyVar(vtag))
    return FALSE;
  Assert(vptr);

  staticAddSpawnProp(ps, vptr);
  return TRUE;
}

void OZ_Propagator::spawn(OZ_Propagator * p, int prio)
{
  Thread * thr = am.mkPropagator(am.currentBoard, prio, p);
  thr->headInitPropagator();

  ozstat.propagatorsCreated.incf();

  thr->suspendPropagator();
  thr->propagatorToRunnable ();
  am.scheduleThreadInline(thr, thr->getPriority());

  OZ_Boolean all_local = OZ_TRUE;

  for (int i = staticSpawnVarsNumberProp; i--; ) {
    OZ_Term v = makeTaggedRef(staticSpawnVarsProp[i].var);
    DEREF(v, vptr, vtag);

    Assert(isAnyVar(vtag));

    Bool isStorePatched, isReifiedPatched, isBoolPatched;
    OZ_FiniteDomain * tmp_fd;

    if (isCVar(vtag)) {
      isStorePatched = testResetStoreFlag(v);
      isReifiedPatched = testResetReifiedFlag(v);
      if (isReifiedPatched) {
        isBoolPatched = testBoolPatched(v);
        tmp_fd = unpatchReified(v, isBoolPatched);
      }
    }

    if (isGenFDVar(v, vtag)) {
      addSuspFDVar(v, thr, staticSpawnVars[i].state.fd);
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

    if (isCVar(vtag)) {
      if (isStorePatched)
        setStoreFlag(v);
      if (isReifiedPatched)
        patchReified(tmp_fd, v, isBoolPatched);
    }
  }

  if (all_local)
    thr->markLocalThread();

  staticSpawnVarsNumberProp = 0;
}

// End of File
//-----------------------------------------------------------------------------
