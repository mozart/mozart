/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(INTERFACE)
#pragma implementation "fdbuilti.hh"
#endif

#include <stdarg.h>

#include "fdbuilti.hh"

#include "fdhook.hh"
#include "genvar.hh"

#if defined(OUTLINE) || defined(FDOUTLINE)
#define inline 
#include "fdbuilti.icc"
#undef inline 
#endif

//-----------------------------------------------------------------------------
// pm_

inline
char * pm_term_type2string(int t) {
  switch (t) {
  case pm_none: return "pm_none";
  case pm_singl: return "pm_singl";
  case pm_bool: return "pm_bool";
  case pm_fd:  return "pm_fd";
  case pm_svar: return "pm_svar";
  case pm_uvar: return "pm_uvar";
  case pm_tuple: return "pm_tuple";
  case pm_literal: return "pm_literal";
  default:  return "unexpected";
  }
}

inline
OZ_Term deref(OZ_Term &tr, OZ_Term * &ptr, pm_term_type &tag)
{
  OZ_Term tr1=tr;
  DEREF(tr1,ptr1,tag1);
  tr=tr1;
  ptr=ptr1;
  switch (tag1) {
  case SMALLINT: tag = pm_singl; break;
  case CVAR: 
    switch (tagged2CVar(tr1)->getType()) {
    case FDVariable: tag = pm_fd; break;
    case BoolVariable: tag = pm_bool; break;
    default: tag = pm_none; break;
    }
    break;
  case SVAR: tag = pm_svar; break;
  case UVAR: tag = pm_uvar; break;
  case SRECORD: tag = isSTuple(tr1) ? pm_tuple : pm_none; break;
  case LITERAL: tag = pm_literal; break;
  default: tag = pm_none; break;
  }
  return tr1;
}

//-----------------------------------------------------------------------------
// Auxiliary stuff
//-----------------------------------------------------------------------------

// return OZ_TRUE if i is not negative
OZ_Boolean getSign(int i) {
  return i >= 0;
}

OZ_Boolean getSign(double  i) {
  return i >= 0.0;
}

void getSignbit(int i, int n) {
  static_sign_bit[i] = getSign(n);
}

void getDoubleCoeff(int i, OZ_Term v) {
  int n =  OZ_intToC(v);
  static_coeff_double[i] = (double) n;
  getSignbit(i, n);
}

void getIntCoeff(int i, OZ_Term v) {
  int n =  OZ_intToC(v);
  static_coeff_int[i] = n;
  getSignbit(i, n);
}

//-----------------------------------------------------------------------------
// class BIfdHeadManager
//-----------------------------------------------------------------------------
// Global data which are shared between different pieces of code

OZ_Term static_var[MAXFDBIARGS];
OZ_Term * static_varptr[MAXFDBIARGS];
pm_term_type static_vartag[MAXFDBIARGS];
double static_coeff_double[MAXFDBIARGS];
int static_coeff_int[MAXFDBIARGS];
OZ_Boolean static_sign_bit[MAXFDBIARGS];

OZ_Boolean static_bool_a[MAXFDBIARGS];
OZ_Boolean static_bool_b[MAXFDBIARGS];
int static_int_a[MAXFDBIARGS];
int static_int_b[MAXFDBIARGS];
double static_double_a[MAXFDBIARGS];
double static_double_b[MAXFDBIARGS];

int static_index_offset[MAXFDBIARGS];
int static_index_size[MAXFDBIARGS];

OZ_FiniteDomain __CDVoidFiniteDomain(fd_full);

//-----------------------------------------------------------------------------
// Member functions

OZ_Term * BIfdHeadManager::bifdhm_var;
OZ_Term ** BIfdHeadManager::bifdhm_varptr;
pm_term_type * BIfdHeadManager::bifdhm_vartag;
int * BIfdHeadManager::bifdhm_coeff;
int BIfdHeadManager::curr_num_of_items;

void BIfdHeadManager::initStaticData(void) {
  bifdhm_var = static_var;
  bifdhm_varptr = static_varptr;
  bifdhm_vartag = static_vartag;
  bifdhm_coeff = static_coeff_int;
  curr_num_of_items = 0;
}

OZ_Boolean BIfdHeadManager::areIdentVar(int a, int b) {
  AssertFD(0 <= a && a < curr_num_of_items || 0 <= b && b < curr_num_of_items);
  
  return (bifdhm_varptr[a] == bifdhm_varptr[b] && isAnyVar(bifdhm_var[a]));
}

OZ_Boolean BIfdHeadManager::expectFDish(int i, OZ_Term v, int &s)
{
  DebugCheck(i < 0 || i >= curr_num_of_items, error("index violation"));

  bifdhm_var[i] = deref(v, bifdhm_varptr[i], bifdhm_vartag[i]);
  pm_term_type vtag = bifdhm_vartag[i];
  
  if (vtag == pm_fd || vtag == pm_bool) {
    return OZ_TRUE;
  } else if (vtag == pm_singl) {
    return OZ_intToC(v) >= 0;
  } else if (vtag == pm_svar || vtag == pm_uvar) {
    s += 1;
    return OZ_TRUE;
  } 
  
  return OZ_FALSE;
}
  
OZ_Boolean BIfdHeadManager::expectInt(int i, OZ_Term v, int &s)
{
  DebugCheck(i < 0 || i >= curr_num_of_items, error("index violation"));

  bifdhm_var[i] = deref(v, bifdhm_varptr[i], bifdhm_vartag[i]);
  pm_term_type vtag = bifdhm_vartag[i];
  
  if (vtag == pm_singl) {
    bifdhm_coeff[i] = OZ_intToC(bifdhm_var[i]);
    return OZ_TRUE;
  } else if (vtag == pm_fd || vtag == pm_bool) {
    s += 1;
    return OZ_TRUE;
  } else if (vtag == pm_svar || vtag == pm_uvar) {
    s += 1;
    return OZ_TRUE;
  }
  return OZ_FALSE;
}

OZ_Boolean BIfdHeadManager::expectNonLin(int i, SRecord &at, SRecord &xt,
				   OZ_Term tagged_xtc, int &s,
				   OZ_CFun f, RefsArray x, int a)
{
  DebugCheck(i < 0 || i >= curr_num_of_items, error("index overflow"));

  deref(tagged_xtc, bifdhm_varptr[i], bifdhm_vartag[i]);
  
  if (bifdhm_vartag[i] == pm_literal) {
    if (tagged_xtc == AtomPair) {
      bifdhm_var[i] = OZ_int(0);
      bifdhm_vartag[i] = pm_singl;
      return OZ_TRUE;
    }
    return OZ_FALSE;
  }
  
  if (! (bifdhm_vartag[i] == pm_tuple)) {
    bifdhm_var[i] = tagged_xtc;
    return OZ_TRUE;
  }

  SRecord &xtc = *tagged2SRecord(tagged_xtc);
  const int ts = xtc.getWidth();
  OZ_Term last_fdvar;
  OZ_Term * last_fdvarptr = NULL, * prev_fdvarptr;
  OZ_Term var;
  OZ_Term * varptr;
  pm_term_type vartag;
  long prod = 1;
  pm_term_type last_tag = pm_none;

  int j, fds_found;
  for (j = ts, fds_found = 0; j-- && (fds_found < 2); ) {
    var = makeTaggedRef(&xtc[j]);
    deref(var, varptr, vartag);
 
    if (vartag == pm_singl) {
      prod *= OZ_intToC(var);
      if (prod < OzMinInt || OzMaxInt < prod) return OZ_FALSE;
    } else if (vartag == pm_fd || vartag == pm_bool) {
      fds_found += 1;
      if (last_fdvarptr != NULL)
	prev_fdvarptr = last_fdvarptr;
      last_fdvar = var;
      last_tag = vartag;
      last_fdvarptr = varptr;
    } else if (vartag == pm_uvar) {
      fds_found = 3;
    } else if (vartag == pm_svar) {
      fds_found = 4;      
    } else {
      fds_found = 5;
    }
  }

  switch (fds_found) {
  case 0: // no variables left
    bifdhm_vartag[i] = pm_singl;
    xt[i] = bifdhm_var[i] = OZ_int(1);
    bifdhm_coeff[i] *= prod;
    if (bifdhm_coeff[i] < OzMinInt || OzMaxInt < bifdhm_coeff[i]) return OZ_FALSE;
    at[i] = OZ_int(bifdhm_coeff[i]);
    return OZ_TRUE;
  case 1: // exactly one variable left
    bifdhm_vartag[i] = last_tag;
    bifdhm_var[i] = last_fdvar;
    xt[i] = makeTaggedRef(bifdhm_varptr[i] = last_fdvarptr);
    bifdhm_coeff[i] *= prod;
    if (bifdhm_coeff[i] < OzMinInt || OzMaxInt < bifdhm_coeff[i]) return OZ_FALSE;
    at[i] = OZ_int(bifdhm_coeff[i]);
    return OZ_TRUE;
  case 2:
    s += 1;
#ifdef FDBISTUCK
    am.addSuspendVarList(prev_fdvarptr);
    am.addSuspendVarList(last_fdvarptr);
#else
    {
      OZ_Thread th = OZ_makeThread(f, x, a);
      OZ_addThread(makeTaggedRef(prev_fdvarptr), th);
      OZ_addThread(makeTaggedRef(last_fdvarptr), th);
    }
#endif
    return OZ_TRUE;
  case 3:
    s += 1;
#ifdef FDBISTUCK
    am.addSuspendVarList(varptr);
#else 
    OZ_addThread(makeTaggedRef(varptr), OZ_makeThread(f, x, a));
#endif
    return OZ_TRUE;
  case 4:
    s += 1;
#ifdef FDBISTUCK
    am.addSuspendVarList(varptr);
#else 
    OZ_addThread(makeTaggedRef(varptr), OZ_makeThread(f, x, a));
#endif
    return OZ_TRUE;
  case 5:
    return OZ_FALSE;
  default:
    error("Unexpected fds_found (0x%x).", fds_found);
    return OZ_FALSE;
  }
}

void BIfdHeadManager::addPropagators (Thread *thr, OZ_FDPropState target) {
  for (int i = curr_num_of_items; i--; )
    addPropagator (i, thr, target);
  
  if (global_vars == 0)
    thr->markLocalThread ();
}


#ifdef FDBISTUCK

OZ_Return BIfdHeadManager::addSuspFDish(OZ_CFun, OZ_Term *, int) {
  am.emptySuspendVarList();

  for (int i = curr_num_of_items; i--; )
    if (pm_is_noncvar(bifdhm_vartag[i])) {
      AssertFD(isAnyVar(*bifdhm_varptr[i]));
      am.addSuspendVarList(bifdhm_varptr[i]);
    }

  AssertFD(am.suspendVarList != makeTaggedNULL());

  return SUSPEND;
}

OZ_Return BIfdHeadManager::addSuspSingl(OZ_CFun, OZ_Term *, int) {
  am.emptySuspendVarList();

  for (int i = curr_num_of_items; i--; )
    if (pm_is_var(bifdhm_vartag[i])) {
      AssertFD(isAnyVar(*bifdhm_varptr[i]));
      am.addSuspendVarList(bifdhm_varptr[i]);
    }

  AssertFD(am.suspendVarList != makeTaggedNULL());

  return SUSPEND;
}

Bool BIfdHeadManager::addSuspXorYdet(OZ_CFun, OZ_Term *, int)
{
  if (pm_is_var(bifdhm_vartag[0]) && pm_is_var(bifdhm_vartag[1])) {
    am.addSuspendVarList(bifdhm_varptr[0]);
    am.addSuspendVarList(bifdhm_varptr[1]);
    return OZ_FALSE;
  }
  return OZ_TRUE;
}

#else

OZ_Return BIfdHeadManager::addSuspFDish(OZ_CFun f, OZ_Term * x, int a) {
  OZ_Thread th = OZ_makeThread(f, x, a);

  for (int i = curr_num_of_items; i--; )
    if (pm_is_noncvar(bifdhm_vartag[i])) {
      AssertFD(isAnyVar(*bifdhm_varptr[i]));
      OZ_addThread(makeTaggedRef(bifdhm_varptr[i]), th);
    }

  return PROCEED;
}

OZ_Return BIfdHeadManager::addSuspSingl(OZ_CFun f, OZ_Term * x, int a) {
  OZ_Thread th = OZ_makeThread(f, x, a);

  for (int i = curr_num_of_items; i--; )
    if (pm_is_var(bifdhm_vartag[i])) {
      AssertFD(isAnyVar(*bifdhm_varptr[i]));
      OZ_addThread(makeTaggedRef(bifdhm_varptr[i]), th);
    }

  return PROCEED;
}

Bool BIfdHeadManager::addSuspXorYdet(OZ_CFun f, OZ_Term * x, int a)
{
  if (pm_is_var(bifdhm_vartag[0]) && pm_is_var(bifdhm_vartag[1])) {
    OZ_Thread th = OZ_makeThread(f, x, a);
    OZ_addThread(makeTaggedRef(bifdhm_varptr[0]), th);
    OZ_addThread(makeTaggedRef(bifdhm_varptr[1]), th);
    return OZ_FALSE;
  }
  return OZ_TRUE;
}

#endif


void BIfdHeadManager::addPropagator (int i, Thread *thr, OZ_FDPropState target)
{
  DebugCheck(i < 0 || i >= curr_num_of_items, error("index overflow"));

  pm_term_type tag = bifdhm_vartag[i];

  if (tag == pm_singl) {
    return;
  } else if (tag == pm_fd) {
    addSuspFDVar(bifdhm_var[i], new SuspList (thr), target);
    if (! am.isLocalCVar(bifdhm_var[i])) global_vars += 1;
  } else if (tag == pm_bool) {
    addSuspBoolVar(bifdhm_var[i], new SuspList (thr));
    if (! am.isLocalCVar(bifdhm_var[i])) global_vars += 1;
  } else if (tag == pm_uvar) {
    if (bifdhm_var[i] != *bifdhm_varptr[i]) return;
    if (am.isLocalUVar(bifdhm_var[i])) {
      OZ_Term * taggedfdvar = newTaggedCVar(new GenFDVariable());
      addSuspFDVar(*taggedfdvar, new SuspList (thr), target);
      doBind(bifdhm_varptr[i], makeTaggedRef(taggedfdvar));
    } else {
      global_vars += 1;
      addSuspUVar(bifdhm_varptr[i], new SuspList (thr));
    } 
  } else {
    Assert(tag == pm_svar);
    
    if (bifdhm_var[i] != *bifdhm_varptr[i]) return;
    if (am.isLocalSVar(bifdhm_var[i])) {
      GenFDVariable * fdvar = new GenFDVariable();
      OZ_Term * taggedfdvar = newTaggedCVar(fdvar);
      am.checkSuspensionList(bifdhm_var[i]);
      fdvar->setSuspList(tagged2SVar(bifdhm_var[i])->getSuspList());
      addSuspFDVar(*taggedfdvar, new SuspList (thr), target);
      doBind(bifdhm_varptr[i], makeTaggedRef(taggedfdvar));
    } else {
      global_vars += 1;
      addSuspSVar(bifdhm_var[i], new SuspList (thr));
    } 
  } 
}

void BIfdHeadManager::printDebug(void) {
  for (int i = 0; i < curr_num_of_items; i += 1)
    printDebug(i);
}

void BIfdHeadManager::printDebug(int i) {
  cerr << '[' << i << "]: var=" << (void *) bifdhm_var[i]
       << ", varptr=" << (void *) bifdhm_varptr[i]
       << ", vartag=" << pm_term_type2string(bifdhm_vartag[i])
       << ", coeff=" << bifdhm_coeff[i] << endl;
  cerr.flush();
}

BIfdBodyManager::BIfdBodyManager(int s) { 
  DebugCodeFD(backup_count = 0);
  AssertFD(0 <= s && s < MAXFDBIARGS);
  curr_num_of_vars = s;
  AssertFD(am.currentThread);
  only_local_vars = am.currentThread->isLocalThread ();
}

void BIfdBodyManager::saveDomainOnTopLevel(int i) {
  if (am.currentBoard->isRoot()) {
    if (bifdbm_vartag[i] == pm_fd)
      bifdbm_domain[i] = tagged2GenFDVar(bifdbm_var[i])->getDom();
  }
}

OZ_Return BIfdHeadManager::spawnPropagator
(OZ_FDPropState t, OZ_CFun f, int a, OZ_Term * x) 
{
#ifndef TM_LP
  if (localPropStore.isUseIt ()) {
    Thread *prop;

    //
    prop = createPropagator (f, a, x);
    addPropagators (prop, t);
    localPropStore.push (prop);

    return (PROCEED);
  } else {
    AM *e = &am;
    Thread *savedCT;

    //
    savedCT = e->currentThread;
    e->currentThread = createPropagator (f, a, x); // propagated!
    addPropagators (e->currentThread, t);

    // 
    switch ((f)(a, x)) {
    case SLEEP:
      e->currentThread->unmarkPropagated (); // kost@ TODO!!! temporary;
      e->currentThread = savedCT;
      return (PROCEED);

    case PROCEED:
      e->currentThread->closeDonePropagator ();
      e->currentThread = savedCT;
      return (PROCEED);

    case FAILED: 
      e->currentThread->closeDonePropagator ();
      e->currentThread = savedCT;
      return FAILED;

    case RAISE:
      error ("propagators can't raise exceptions.");

    case SUSPEND:
      error ("propagators can't return 'SUSPEND'.\n");
      
    default:
      error ("propagator returned an unknown value.\n");
    }
  }
#else
  Thread *prop;

  //
  //  New idea: don't execute the propagator directly, but rather
  // push it into the local propagation store - and proceed further 
  // immediately (so, propagator's head should return 'PROCEED' 
  // normally);
  prop = createPropagator (f, a, x);
  addPropagators (prop, t);
  localPropStore.push (prop);

  return (PROCEED);
#endif
}

OZ_Return BIfdHeadManager::spawnPropagator
(OZ_FDPropState t1, OZ_FDPropState t2, OZ_CFun f, int a, OZ_Term * x) 
{
#ifndef TM_LP
  if (localPropStore.isUseIt ()) {
    Thread *prop;

    prop = createPropagator (f, a, x);
    addPropagator (0, prop, t1);
    addPropagator (1, prop, t2);
    localPropStore.push (prop);

    return (PROCEED);
  } else {
    AM *e = &am;
    Thread *savedCT;

    //
    savedCT = e->currentThread;
    e->currentThread = createPropagator (f, a, x); // propagated!
    addPropagator (0, e->currentThread, t1);
    addPropagator (1, e->currentThread, t2);

    // 
    switch ((f)(a, x)) {
    case SLEEP:
      e->currentThread->unmarkPropagated (); // kost@ TODO!!! temporary;
      e->currentThread = savedCT;
      return (PROCEED);

    case PROCEED:
      e->currentThread->closeDonePropagator ();
      e->currentThread = savedCT;
      return (PROCEED);

    case FAILED: 
      e->currentThread->closeDonePropagator ();
      e->currentThread = savedCT;
      return FAILED;

    case RAISE:
      error ("propagators can't raise exceptions.");

    case SUSPEND:
      error ("propagators can't return 'SUSPEND'.\n");
      
    default:
      error ("propagator returned an unknown value.\n");
    }
  }
#else
  Thread *prop;

  prop = createPropagator (f, a, x);
  addPropagator (0, prop, t1);
  addPropagator (1, prop, t2);
  localPropStore.push (prop);

  return (PROCEED);
#endif
}

OZ_Return BIfdHeadManager::spawnPropagator
(OZ_FDPropState t, OZ_CFun f, int a, OZ_Term t1, ...)
{
  OZ_Term * x = (OZ_Term *) heapMalloc (a * sizeof (OZ_Term));

  x[0] = t1;

  va_list ap;
  va_start (ap, t1);

  for (int i = 1; i < a; i += 1)
    x[i] = va_arg (ap, OZ_Term);

  va_end (ap);

#ifndef TM_LP
  if (localPropStore.isUseIt ()) {
    Thread *prop;

    prop = createPropagator (f, a, x);
    addPropagators (prop, t);
    localPropStore.push (prop);

    return (PROCEED);
  } else {
    AM *e = &am;
    Thread *savedCT;

    //
    savedCT = e->currentThread;
    e->currentThread = createPropagator (f, a, x); // propagated!
    addPropagators (e->currentThread, t);

    // 
    switch ((f)(a, x)) {
    case SLEEP:
      e->currentThread->unmarkPropagated (); // kost@ TODO!!! temporary;
      e->currentThread = savedCT;
      return (PROCEED);

    case PROCEED:
      e->currentThread->closeDonePropagator ();
      e->currentThread = savedCT;
      return (PROCEED);

    case FAILED: 
      e->currentThread->closeDonePropagator ();
      e->currentThread = savedCT;
      return FAILED;

    case RAISE:
      error ("propagators can't raise exceptions.");

    case SUSPEND:
      error ("propagators can't return 'SUSPEND'.\n");
      
    default:
      error ("propagator returned an unknown value.\n");
    }
  }
#else
  Thread *prop;

  prop = createPropagator (f, a, x);
  addPropagators (prop, t);
  localPropStore.push (prop);

  return (PROCEED);
#endif
}

OZ_Return BIfdHeadManager::spawnPropagatorStabil
(OZ_FDPropState t, OZ_CFun f, int a, OZ_Term t1, ...)
{
  OZ_Term * x = (OZ_Term *) heapMalloc (a * sizeof (OZ_Term));

  x[0] = t1;

  va_list ap;
  va_start (ap, t1);

  for (int i = 1; i < a; i += 1)
    x[i] = va_arg (ap, OZ_Term);

  va_end (ap);

#ifndef TM_LP
  if (localPropStore.isUseIt ()) {
    Thread *prop;

    prop = createPropagator (f, a, x);
    addPropagators (prop, t);
    prop->markStable ();
    localPropStore.push (prop);

    return (PROCEED);
  } else {
    AM *e = &am;
    Thread *savedCT;

    //
    savedCT = e->currentThread;
    e->currentThread = createPropagator (f, a, x); // propagated!
    addPropagators (e->currentThread, t);

    // 
    switch ((f)(a, x)) {
    case SLEEP:
      e->currentThread->markStable ();
      e->currentThread->unmarkPropagated (); // kost@ TODO!!! temporary;
      e->currentThread = savedCT;
      return (PROCEED);

    case PROCEED:
      e->currentThread->closeDonePropagator ();
      e->currentThread = savedCT;
      return (PROCEED);

    case FAILED: 
      e->currentThread->closeDonePropagator ();
      e->currentThread = savedCT;
      return FAILED;

    case RAISE:
      error ("propagators can't raise exceptions.");

    case SUSPEND:
      error ("propagators can't return 'SUSPEND'.\n");
      
    default:
      error ("propagator returned an unknown value.\n");
    }
  }
#else
  Thread *prop;

  prop = createPropagator (f, a, x);
  addPropagators (prop, t);
  prop->markStable ();
  localPropStore.push (prop);

  return (PROCEED);
#endif
}

#ifdef FDBISTUCK

OZ_Return BIfdHeadManager::suspendOnVar(OZ_CFun, int, OZ_Term *,
				      OZ_Term * t)
{
  OZ_suspendOn(makeTaggedRef(t));
}

OZ_Return BIfdHeadManager::suspendOnVar(OZ_CFun, int, OZ_Term *,
				      OZ_Term * t1, OZ_Term * t2)
{
  OZ_suspendOn2(makeTaggedRef(t1), 
		makeTaggedRef(t2));
}

OZ_Return BIfdHeadManager::suspendOnVar(OZ_CFun, int, OZ_Term *,
				      OZ_Term * t1, OZ_Term * t2, OZ_Term * t3)
{
  OZ_suspendOn3(makeTaggedRef(t1), 
		makeTaggedRef(t2), 
		makeTaggedRef(t3));
}

#else

OZ_Return BIfdHeadManager::suspendOnVar(OZ_CFun f, int a, OZ_Term * x,
				      OZ_Term * t)
{
  OZ_addThread(makeTaggedRef(t), OZ_makeThread(f, x, a));
  return PROCEED;
}

OZ_Return BIfdHeadManager::suspendOnVar(OZ_CFun f, int a, OZ_Term * x,
				      OZ_Term * t1, OZ_Term * t2)
{
  OZ_Thread th = OZ_makeThread(f, x, a);
  OZ_addThread(makeTaggedRef(t1), th);
  OZ_addThread(makeTaggedRef(t2), th);
  return PROCEED;
}

OZ_Return BIfdHeadManager::suspendOnVar(OZ_CFun f, int a, OZ_Term * x,
				      OZ_Term * t1, OZ_Term * t2, OZ_Term * t3)
{
  OZ_Thread th = OZ_makeThread(f, x, a);
  OZ_addThread(makeTaggedRef(t1), th);
  OZ_addThread(makeTaggedRef(t2), th);
  OZ_addThread(makeTaggedRef(t3), th);
  return PROCEED;
}
#endif

//-----------------------------------------------------------------------------
// An OZ term describing a finite domain is either:
// (1) a positive small integer <= fd_sup
// (2) a 2 tuple of (1)
// (3) a list of (1) and/or (2)

OZ_Return checkDomDescr(OZ_Term descr,
		      OZ_CFun cfun, OZ_Term * args, int arity,
		      int expect)
{
  OZ_Term * descr_ptr;
  TypeOfTerm descr_tag;
  
  deref(descr, descr_ptr, descr_tag);
  
  if (isNotCVar(descr_tag)) {
#ifdef FDBISTUCK
    am.addSuspendVarList(descr_ptr);
#else 
    OZ_addThread(makeTaggedRef(descr_ptr), 
		     OZ_makeThread(cfun, args, arity));
#endif
    return SUSPEND; // checkDomDescr
  } else if (isSmallInt(descr_tag) && (expect >= 1)) { // (1)
    return PROCEED;
  } else if (AtomSup == descr && (expect >= 1)) { // (1)
    return PROCEED;
  } else if (isGenFDVar(descr, descr_tag) && (expect >= 1)) {
#ifdef FDBISTUCK
    am.addSuspendVarList(descr_ptr);
#else 
    OZ_addThread(makeTaggedRef(descr_ptr), 
		     OZ_makeThread(cfun, args, arity));
#endif
    return SUSPEND; // checkDomDescr
  } else if (AtomBool == descr && (expect >= 2)) { // (1)
    return PROCEED;
  } else if (isSRecord(descr_tag) && (expect >= 2)) {
    SRecord &tuple = *tagged2SRecord(descr);
    if (tuple.getWidth() != 2) {
      return FAILED;
    }
    for (int i = 0; i < 2; i++) {
      OZ_Return r = checkDomDescr(makeTaggedRef(&tuple[i]), cfun, args, arity, 1);
      if (r != PROCEED)
	return r;
    }
    return PROCEED;
  } else if (isNil(descr) && (expect == 3)) {
    return PROCEED;
  } else if (isLTuple(descr_tag) && (expect == 3)) {
    
    do {
      LTuple &list = *tagged2LTuple(descr);
      OZ_Return r = checkDomDescr(makeTaggedRef(list.getRefHead()),
				cfun, args, arity, 2);
      if (r != PROCEED)
	return r;
      descr = makeTaggedRef(list.getRefTail());
      
      deref(descr, descr_ptr, descr_tag);
    } while (isLTuple(descr_tag));
    
    if (isNil(descr)) return PROCEED;
    return checkDomDescr(makeTaggedRef(descr_ptr), cfun, args, arity, 0);
  } 
  return FAILED;
}

//-----------------------------------------------------------------------------
//                              class BIfdBodyManager
//-----------------------------------------------------------------------------
// Global data

OZ_Term * BIfdBodyManager::bifdbm_var;
OZ_Term ** BIfdBodyManager::bifdbm_varptr;
pm_term_type * BIfdBodyManager::bifdbm_vartag;
OZ_FiniteDomain ** BIfdBodyManager::bifdbm_dom;
OZ_FiniteDomain * BIfdBodyManager::bifdbm_domain;
int * BIfdBodyManager::bifdbm_init_dom_size;
fdbm_var_state * BIfdBodyManager::bifdbm_var_state;
int * BIfdBodyManager::cache_from;
int * BIfdBodyManager::cache_to;

int BIfdBodyManager::curr_num_of_vars;
OZ_Boolean BIfdBodyManager::vars_left;
int * BIfdBodyManager::index_offset;
int * BIfdBodyManager::index_size;
OZ_Boolean BIfdBodyManager::only_local_vars;

void BIfdBodyManager::initStaticData(void) {
  bifdbm_var = static_var;
  bifdbm_varptr = static_varptr;
  bifdbm_vartag = static_vartag;
  index_offset = static_index_offset;
  index_size = static_index_size;

  bifdbm_dom = new OZ_FiniteDomain *[MAXFDBIARGS];
  bifdbm_domain = new OZ_FiniteDomain[MAXFDBIARGS];
  bifdbm_init_dom_size = new int[MAXFDBIARGS];
  bifdbm_var_state = new fdbm_var_state[MAXFDBIARGS];
  cache_from = new int[MAXFDBIARGS];
  cache_to = new int[MAXFDBIARGS];
}

//-----------------------------------------------------------------------------
// Member functions

void BIfdBodyManager::backup(void)
{
  backup_count += 1;

  backup_curr_num_of_vars1 = curr_num_of_vars;
  backup_vars_left1 = vars_left;
  backup_only_local_vars1 = only_local_vars;
  backup_FDcurrentThread = am.currentThread;
  
  bifdbm_var += backup_curr_num_of_vars1;
  bifdbm_varptr += backup_curr_num_of_vars1;
  bifdbm_vartag += backup_curr_num_of_vars1;
  bifdbm_dom += backup_curr_num_of_vars1;
  bifdbm_domain += backup_curr_num_of_vars1;
  bifdbm_init_dom_size += backup_curr_num_of_vars1;
  cache_from += backup_curr_num_of_vars1;
  cache_to += backup_curr_num_of_vars1;
  index_offset += backup_curr_num_of_vars1;
  index_size += backup_curr_num_of_vars1;
  bifdbm_var_state += backup_curr_num_of_vars1;
}

void BIfdBodyManager::restore(void)
{
  backup_count -= 1;

  bifdbm_var -= backup_curr_num_of_vars1;
  bifdbm_varptr -= backup_curr_num_of_vars1;
  bifdbm_vartag -= backup_curr_num_of_vars1;
  bifdbm_dom -= backup_curr_num_of_vars1;
  bifdbm_domain -= backup_curr_num_of_vars1;
  bifdbm_init_dom_size -= backup_curr_num_of_vars1;
  cache_from -= backup_curr_num_of_vars1;
  cache_to -= backup_curr_num_of_vars1;
  index_offset -= backup_curr_num_of_vars1;
  index_size -= backup_curr_num_of_vars1;
  bifdbm_var_state -= backup_curr_num_of_vars1;

  curr_num_of_vars = backup_curr_num_of_vars1;
  vars_left = backup_vars_left1;
  only_local_vars = backup_only_local_vars1;
  am.currentThread = backup_FDcurrentThread;
}

int BIfdBodyManager::initCache(void) {
  int num_of_slots = int(ceil(double(curr_num_of_vars)/double(cache_slot_size)));
  cache_from[0] = 0;

  int i;
  for (i = 0; i < num_of_slots; i += 1) {
    cache_from[i + 1] = cache_to[i] = cache_slot_size + cache_from[i];
    AssertFD(cache_from[i] < curr_num_of_vars);
  }
  cache_to[i - 1] = min(curr_num_of_vars, cache_to[i - 1]);
  AssertFD(cache_to[i - 1] <= curr_num_of_vars);
  
  return num_of_slots;
  
}

OZ_Return BIfdBodyManager::entailmentClause(int from_b, int to_b,
					  int from,   int to,
					  int from_p, int to_p)
{
  AssertFD((from <= to) && (from_p <= to_p) && (to - from) == (to_p - from_p));

  for (int i = from, j = from_p; i < to+1; i += 1, j += 1) {

    *bifdbm_dom[i] &= *bifdbm_dom[j];
    if(bifdbm_vartag[j] == pm_fd || bifdbm_vartag[j] == pm_bool) {
      if (bifdbm_vartag[i] == pm_fd) {
	if (bifdbm_vartag[j] == pm_fd) {
	  tagged2GenFDVar(bifdbm_var[j])->
	    relinkSuspListTo(tagged2GenFDVar(bifdbm_var[i]));
	} else {
	  tagged2GenBoolVar(bifdbm_var[j])->
	    relinkSuspListTo(tagged2GenFDVar(bifdbm_var[i]));
	}
	doBind(bifdbm_varptr[j], makeTaggedRef(bifdbm_varptr[i]));
      } else if (bifdbm_vartag[i] == pm_bool) {
	if (bifdbm_vartag[j] == pm_fd) {
	  tagged2GenFDVar(bifdbm_var[j])->
	    relinkSuspListTo(tagged2GenBoolVar(bifdbm_var[i]));
	} else {
	  tagged2GenBoolVar(bifdbm_var[j])->
	    relinkSuspListTo(tagged2GenBoolVar(bifdbm_var[i]));
	}
	doBind(bifdbm_varptr[j], makeTaggedRef(bifdbm_varptr[i]));
      } else {
	process(j);
      }
    }
  }
  
  if (only_local_vars) {
    processLocalFromTo(from, to+1);
  } else {
    processFromTo(from, to+1);
  }

  processLocalFromTo(from_b, to_b+1);
  
  return EntailFD;
}

OZ_Boolean BIfdBodyManager::areIdentVar(int a, int b) {
  AssertFD(0 <= a && a < curr_num_of_vars || 0 <= b && b < curr_num_of_vars);

  if (! isUnifyCurrentPropagator ()) return OZ_FALSE;
  return bifdbm_varptr[a] == bifdbm_varptr[b] && isAnyVar(bifdbm_var[a]);
}

void BIfdBodyManager::restoreDomainOnToplevel(void) {
  if (am.currentBoard->isRoot()) {
    for (int i = curr_num_of_vars; i--; )
      if (bifdbm_vartag[i] == pm_fd)
	tagged2GenFDVar(bifdbm_var[i])->getDom() = *bifdbm_dom[i];
  }
}

OZ_Boolean BIfdBodyManager::unifiedVars(void) {
  if (! isUnifyCurrentPropagator ()) return OZ_FALSE;
  return _unifiedVars();
}

void BIfdBodyManager::propagate_unify_cd(int cl, int vars, SRecord &st) {
  if (isUnifyCurrentPropagator ())
    _propagate_unify_cd(cl, vars, st);
}

// functions used by BIfdBodyManager::simplifyBody
// produces GCTAG 8 + 5 = 13
const int taggedIndex = 0x00000008; // is REF, cannot happen for derefed term
inline OZ_Term makeTaggedIndex(int i) {return ((i << 4) | taggedIndex);}
inline int getIndex(OZ_Term t) {return (t & ~taggedIndex) >> 4;}
inline OZ_Boolean isTaggedIndex(OZ_Term t) {return (t & taggedIndex);}
inline
int BIfdBodyManager::simplifyBody(int ts, SRecord &a, SRecord &x,
				  OZ_Boolean sign_bits[], double coeffs[],
				  OZ_Term ct, int &c)
{
  int int_sum = 0;

  // 1st pass: mark first occ of a var and sum up coeffs of further occs 
  for (int i = 0; i < ts; i++) {
    Assert(bifdbm_vartag[i] == pm_fd || 
	   bifdbm_vartag[i] == pm_bool || 
	   bifdbm_vartag[i] == pm_singl);

    if (isAnyVar(bifdbm_var[i])) {
      if (! isTaggedIndex(*bifdbm_varptr[i])) {
	*bifdbm_varptr[i] = makeTaggedIndex(i);
      } else {
	int ind = getIndex(*bifdbm_varptr[i]);
	a[ind] = OZ_int(int(coeffs[ind] += coeffs[i]));
	bifdbm_var[i] = 0;
      } 
    } else {
      Assert(bifdbm_vartag[i] == pm_singl);
      
      int_sum += (OZ_intToC(bifdbm_var[i]) * int(coeffs[i]));
      bifdbm_var[i] = 0;
    }
  }

  c += int_sum;
  //  DEREF(ct, ctptr, cttag);
  *(OZ_Term *) ct = newSmallInt(c);

  // 3rd pass: undo marks and compress vector
  int from, to;
  for (from = 0, to = 0; from < ts; from += 1) {
    OZ_Term var_from = bifdbm_var[from];
    
    if (var_from == 0) continue;

    OZ_Term * varptr_from = bifdbm_varptr[from];

    if (isAnyVar(var_from) && isTaggedIndex(*varptr_from))
      *varptr_from = var_from;

    double coeffs_from = coeffs[from];

    if (coeffs_from == 0.0 || var_from == OZ_int(0)) continue;

    if (from != to) {
      coeffs[to] = coeffs_from;
      bifdbm_var[to] = var_from;
      bifdbm_varptr[to] = varptr_from;
      bifdbm_vartag[to] = bifdbm_vartag[from];
      bifdbm_dom[to] = bifdbm_dom[from];
      bifdbm_init_dom_size[to] = bifdbm_init_dom_size[from];
      bifdbm_var_state[to] = bifdbm_var_state[from];
      a[to] = a[from];
      x[to] = x[from];
    }
    sign_bits[to] = getSign(coeffs_from);
    to += 1;
  } 
  
  // adjust ts and size of tuples
  a.downSize(to);
  x.downSize(to);

  return to;
} // simplifyBody

int BIfdBodyManager::simplifyOnUnify(int ts, SRecord &a, 
				     OZ_Boolean sign_bits[], 
				     double coeffs[], SRecord &x,
				     OZ_Term * ct, int &c) {
  if (isUnifyCurrentPropagator ()) {
    AssertFD(curr_num_of_vars >= ts);
    int new_ts = simplifyBody(ts, a, x, sign_bits, coeffs, 
			      makeTaggedRef(ct), c);
    for (int to = new_ts, from = ts; from < curr_num_of_vars; to++, from++) {
      coeffs[to] = coeffs[from];
      sign_bits[to] = sign_bits[from];
      bifdbm_var[to] = bifdbm_var[from];
      bifdbm_varptr[to] = bifdbm_varptr[from];
      bifdbm_vartag[to] = bifdbm_vartag[from];
      bifdbm_dom[to] = bifdbm_dom[from];
      bifdbm_init_dom_size[to] = bifdbm_init_dom_size[from];
      bifdbm_var_state[to] = bifdbm_var_state[from];
    }
    curr_num_of_vars -= (ts - new_ts);
  } 
  return curr_num_of_vars;
}

int BIfdBodyManager::simplifyOnUnify(SRecord &a, OZ_Boolean sign_bits[], 
				     double coeffs[], SRecord &x,
				     OZ_Term * ct, int &c) 
{
  if (isUnifyCurrentPropagator ())
    curr_num_of_vars = simplifyBody(curr_num_of_vars, a, x, sign_bits, coeffs,
				    makeTaggedRef(ct), c);
  return curr_num_of_vars;
}

OZ_Return BIfdBodyManager::release(int from, int to) {
  
  if (only_local_vars) {
    processLocalFromTo(from, to+1);
    if (vars_left) return SuspendFD ;
  } else {
    processFromTo(from, to+1);
    if (vars_left)
      return SuspendFD ;
  }
  
  return EntailFD;
}

void BIfdBodyManager::introduceDummy(int i) { 
  bifdbm_dom[i] = NULL; 
}

void BIfdBodyManager::introduceLocal(int i, OZ_Term v)
{
  DebugCheck(i < 0 || i >= curr_num_of_vars, error("index overflow"));

  pm_term_type &vartag = bifdbm_vartag[i];
  OZ_Term  &var = bifdbm_var[i] = v;
  
  deref(var, bifdbm_varptr[i], vartag);
  Assert(vartag == pm_fd || vartag == pm_bool || 
	 vartag == pm_singl || vartag == pm_literal);

  bifdbm_var_state[i] = fdbm_local; 
  if (vartag == pm_singl) {
    bifdbm_init_dom_size[i]=bifdbm_domain[i].initSingleton(OZ_intToC(var));
    bifdbm_dom[i] = &bifdbm_domain[i];
  } else if (vartag == pm_bool) {
    bifdbm_init_dom_size[i] = bifdbm_domain[i].initBool();
    bifdbm_dom[i] = &bifdbm_domain[i];
  } else if (vartag == pm_fd) {
    bifdbm_dom[i] = &tagged2GenFDVar(var)->getDom();
    bifdbm_init_dom_size[i] = bifdbm_dom[i]->getSize();
    Assert(bifdbm_init_dom_size[i] > 1 && *bifdbm_dom[i] != fd_bool);
  } else {
    Assert(vartag == pm_literal);

    bifdbm_dom[i] = &__CDVoidFiniteDomain;
    bifdbm_init_dom_size[i] = bifdbm_dom[i]->getSize();
  }
}


void BIfdBodyManager::processLocalFromTo(int from, int to)
{
  vars_left = OZ_FALSE;

  for (int i = from; i < to; i += 1) {
    Assert(bifdbm_vartag[i] == pm_fd || 
	   bifdbm_vartag[i] == pm_bool || 
	   bifdbm_vartag[i] == pm_singl);

    if (bifdbm_var_state[i] == fdbm_speculative)
      continue;

    if (bifdbm_vartag[i] == pm_singl || isSmallInt(*bifdbm_varptr[i])) {
      continue;
    } else if (!isTouched(i)) {
      vars_left = OZ_TRUE;
    } else if (bifdbm_vartag[i] == pm_bool) {
      Assert(*bifdbm_dom[i] == fd_singleton);
      
      tagged2GenBoolVar(bifdbm_var[i])->
	becomesSmallIntAndPropagate(bifdbm_varptr[i], *bifdbm_dom[i]);
    } else {
      Assert(bifdbm_vartag[i] == pm_fd);
      
      if (*bifdbm_dom[i] == fd_singleton) {
	tagged2GenFDVar(bifdbm_var[i])->
	  becomesSmallIntAndPropagate(bifdbm_varptr[i]);
      } else if (*bifdbm_dom[i] == fd_bool) {
	tagged2GenFDVar(bifdbm_var[i])->
	  becomesBoolVarAndPropagate(bifdbm_varptr[i]);
	vars_left = OZ_TRUE;
      } else {
	tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_bounds);
	vars_left = OZ_TRUE;
      }
    }
  }
} 


void BIfdBodyManager::_introduce(int i, OZ_Term v)
{
  DebugCheck(i < 0 || i >= curr_num_of_vars, error("index overflow"));

  pm_term_type vtag;
  OZ_Term *vptr;

  deref(v, vptr, vtag);

  if (vtag == pm_singl) {
    bifdbm_init_dom_size[i] = bifdbm_domain[i].initSingleton(OZ_intToC(v));
    bifdbm_dom[i] = &bifdbm_domain[i];
    bifdbm_var_state[i] = fdbm_local;
  } else if (vtag == pm_bool) {
    bifdbm_init_dom_size[i] = bifdbm_domain[i].initBool();
    bifdbm_dom[i] = &bifdbm_domain[i];
    bifdbm_var_state[i] = (am.isLocalCVar(v) ? fdbm_local : fdbm_global);
  } else if (vtag == pm_fd) {
    GenFDVariable * fdvar = tagged2GenFDVar(v);
    OZ_Boolean var_state = bifdbm_var_state[i] = (am.isLocalCVar(v) ? fdbm_local : fdbm_global);
    bifdbm_domain[i].FiniteDomainInit(NULL);
    if (var_state == fdbm_local) {
      bifdbm_dom[i] = &fdvar->getDom();
    } else {
      bifdbm_domain[i] = fdvar->getDom();
      bifdbm_dom[i] = &bifdbm_domain[i];
    }
    bifdbm_init_dom_size[i] = bifdbm_dom[i]->getSize();
    Assert(bifdbm_init_dom_size[i] > 1 && *bifdbm_dom[i] != fd_bool);
  } else if (vtag == pm_svar) {
    bifdbm_domain[i].initFull();
    bifdbm_dom[i] = &bifdbm_domain[i];
    bifdbm_var_state[i] = (am.isLocalSVar(v) ? fdbm_local : fdbm_global);
  } else {
    Assert(vtag == pm_literal);

    bifdbm_dom[i] = &__CDVoidFiniteDomain;
    bifdbm_init_dom_size[i] = bifdbm_dom[i]->getSize();
  }

  bifdbm_var[i] = v;
  bifdbm_varptr[i] = vptr;
  bifdbm_vartag[i] = vtag;
} // BIfdBodyManager::_introduce

void BIfdBodyManager::processFromTo(int from, int to)
{
  vars_left = OZ_FALSE;
  
  for (int i = from; i < to; i += 1) {

    if (bifdbm_var_state[i] == fdbm_speculative)
      continue;
    
    pm_term_type vartag = bifdbm_vartag[i];

    if (vartag == pm_singl || isSmallInt(*bifdbm_varptr[i])) {
      continue;
    } else if (! isTouched(i)) {
      vars_left = OZ_TRUE;
    } else if (vartag == pm_fd) {
      if (*bifdbm_dom[i] == fd_singleton) {
	if (bifdbm_var_state[i] == fdbm_local) {
	  tagged2GenFDVar(bifdbm_var[i])->
	    becomesSmallIntAndPropagate(bifdbm_varptr[i]);
	} else {
	  tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_det);
	  am.doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i],
			    OZ_int(bifdbm_dom[i]->singl()));
	}
      } else if (*bifdbm_dom[i] == fd_bool) {

	if (bifdbm_var_state[i] == fdbm_local) {
	  tagged2GenFDVar(bifdbm_var[i])->
	    becomesBoolVarAndPropagate(bifdbm_varptr[i]);
	} else {
	  tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_bounds);
	  GenBoolVariable * newboolvar = new GenBoolVariable();
	  OZ_Term * newtaggedboolvar = newTaggedCVar(newboolvar);
	  am.doBindAndTrailAndIP(bifdbm_var[i], bifdbm_varptr[i],
				 makeTaggedRef(newtaggedboolvar),
				 newboolvar, tagged2GenBoolVar(bifdbm_var[i]),
				 NO);
	}

      } else {

	tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_bounds);
	if (bifdbm_var_state[i] == fdbm_global) {
	  GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[i]);
	  OZ_Term * newtaggedfdvar = newTaggedCVar(newfdvar);
	  am.doBindAndTrailAndIP(bifdbm_var[i], bifdbm_varptr[i],
				 makeTaggedRef(newtaggedfdvar),
				 newfdvar, tagged2GenFDVar(bifdbm_var[i]), NO);
	} 
	
	vars_left = OZ_TRUE;
      }

    } else if (vartag == pm_bool) {
      Assert(*bifdbm_dom[i] == fd_singleton); 

      if (bifdbm_var_state[i] == fdbm_local) {
	tagged2GenBoolVar(bifdbm_var[i])->
	  becomesSmallIntAndPropagate(bifdbm_varptr[i], *bifdbm_dom[i]);
      } else {
	tagged2GenBoolVar(bifdbm_var[i])->propagate(bifdbm_var[i]);
	am.doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i],
			  OZ_int(bifdbm_dom[i]->singl()));
      }
    } else {
      Assert(vartag == pm_svar && bifdbm_var_state[i] == fdbm_global);

      ozstat.fdvarsCreated.incf();

      if (*bifdbm_dom[i] == fd_singleton) {
	OZ_Term smallInt = OZ_int(bifdbm_dom[i]->singl());
	am.checkSuspensionList(bifdbm_var[i]);
	am.doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i], smallInt);
      } else if (*bifdbm_dom[i] == fd_bool) {
	GenBoolVariable * newboolvar = new GenBoolVariable();
	OZ_Term * newtaggedboolvar = newTaggedCVar(newboolvar);
	am.checkSuspensionList(bifdbm_var[i]);
	am.doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i],
			  makeTaggedRef(newtaggedboolvar));
	vars_left = OZ_TRUE;
      } else {
	GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[i]);
	OZ_Term * newtaggedfdvar = newTaggedCVar(newfdvar);
	am.checkSuspensionList(bifdbm_var[i]);
	am.doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i],
			  makeTaggedRef(newtaggedfdvar));
	vars_left = OZ_TRUE;
      }
      PROFILE_CODE1(
		    if (FDVarsTouched.add(bifdbm_var[i]))
		       FDProfiles.inc_item(no_touched_vars);
		    )
      
    }
  } // for
} // BIfdBodyManager::process

void BIfdBodyManager::process(void) {
  processFromTo(0, curr_num_of_vars);
}

void BIfdBodyManager::processNonRes(void)
{
  if (bifdbm_var_state[0] == fdbm_speculative)
    return;

  pm_term_type vartag = bifdbm_vartag[0];
  
  if (!isTouched(0)) {
    return;
  } else if (vartag == pm_fd) {

    if (*bifdbm_dom[0] == fd_singleton) {
      if (bifdbm_var_state[0] == fdbm_local) {
	tagged2GenFDVar(bifdbm_var[0])->
	  becomesSmallIntAndPropagate(bifdbm_varptr[0]);
      } else {
	tagged2GenFDVar(bifdbm_var[0])->propagate(bifdbm_var[0], fd_det);
	am.doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0],
			  OZ_int(bifdbm_dom[0]->singl()));
      }
    } else if (*bifdbm_dom[0] == fd_bool) {
      tagged2GenFDVar(bifdbm_var[0])->propagate(bifdbm_var[0], fd_bounds);
      if (bifdbm_var_state[0] == fdbm_global) {
	GenBoolVariable * newboolvar = new GenBoolVariable();
	OZ_Term * newtaggedboolvar = newTaggedCVar(newboolvar);
	am.doBindAndTrailAndIP(bifdbm_var[0], bifdbm_varptr[0],
			       makeTaggedRef(newtaggedboolvar),
			       newboolvar, tagged2GenFDVar(bifdbm_var[0]), NO);
      } else {
	tagged2GenFDVar(bifdbm_var[0])->
	  becomesBoolVarAndPropagate(bifdbm_varptr[0]);
      }
    } else {
      tagged2GenFDVar(bifdbm_var[0])->propagate(bifdbm_var[0], fd_bounds);
      if (bifdbm_var_state[0] == fdbm_global) {
	GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[0]);
	OZ_Term * newtaggedfdvar = newTaggedCVar(newfdvar);
	am.doBindAndTrailAndIP(bifdbm_var[0], bifdbm_varptr[0],
			       makeTaggedRef(newtaggedfdvar),
			       newfdvar, tagged2GenFDVar(bifdbm_var[0]), NO);
      } 
    }

  } else if (vartag == pm_bool) {
    Assert(*bifdbm_dom[0] == fd_singleton);

    if (bifdbm_var_state[0] == fdbm_local) {
      tagged2GenBoolVar(bifdbm_var[0])->
	becomesSmallIntAndPropagate(bifdbm_varptr[0], *bifdbm_dom[0]);
    } else {
      tagged2GenBoolVar(bifdbm_var[0])->propagate(bifdbm_var[0]);
      am.doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0],
			OZ_int(bifdbm_dom[0]->singl()));
    }
  } else {
    Assert(bifdbm_var_state[0] == fdbm_global && vartag == pm_svar);
    
    ozstat.fdvarsCreated.incf();

    if (*bifdbm_dom[0] == fd_singleton) {
      OZ_Term smallInt = OZ_int(bifdbm_dom[0]->singl());
      am.checkSuspensionList(bifdbm_var[0]);
      am.doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0], smallInt);
    } else if (*bifdbm_dom[0] == fd_bool) {
      GenBoolVariable * newboolvar = new GenBoolVariable();
      OZ_Term * newtaggedboolvar = newTaggedCVar(newboolvar);
      am.checkSuspensionList(bifdbm_var[0]);
      am.doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0],
			makeTaggedRef(newtaggedboolvar));
      vars_left = OZ_TRUE;
    } else {
      GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[0]);
      OZ_Term * newtaggedfdvar = newTaggedCVar(newfdvar);
      am.checkSuspensionList(bifdbm_var[0]);
      am.doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0], 
			makeTaggedRef(newtaggedfdvar));
      vars_left = OZ_TRUE;
    }
    
  }
} // BIfdBodyManager::processNonRes

OZ_Boolean BIfdBodyManager::introduce(OZ_Term v)
{
  pm_term_type vtag;
  OZ_Term *vptr;

  deref(v, vptr, vtag);

  if (vtag == pm_singl) {
    int i_val = OZ_intToC(v);
    if (i_val >= 0) {
      bifdbm_init_dom_size[0] = bifdbm_domain[0].initSingleton(i_val);
      bifdbm_dom[0] = &bifdbm_domain[0];
      bifdbm_var_state[0] = fdbm_local;
      bifdbm_var[0] = v;
      bifdbm_varptr[0] = vptr;
      bifdbm_vartag[0] = vtag;
   } else {
      return OZ_FALSE;
    }
  } else if (vtag == pm_bool) {
    bifdbm_init_dom_size[0] = bifdbm_domain[0].initBool();
    bifdbm_dom[0] = &bifdbm_domain[0];
    bifdbm_var_state[0] = (am.isLocalCVar(v) ? fdbm_local : fdbm_global);
    bifdbm_var[0] = v;
    bifdbm_varptr[0] = vptr;
    bifdbm_vartag[0] = vtag;
  } else if (vtag == pm_fd) {
    GenFDVariable * fdvar = tagged2GenFDVar(v);
    fdbm_var_state var_state = bifdbm_var_state[0] = (am.isLocalCVar(v) ? fdbm_local : fdbm_global);
    if (var_state == fdbm_local) {
      bifdbm_dom[0] = &fdvar->getDom();
    } else {
      bifdbm_domain[0] = fdvar->getDom();
      bifdbm_dom[0] = &bifdbm_domain[0];
    }
    bifdbm_init_dom_size[0] = bifdbm_dom[0]->getSize();
    bifdbm_var[0] = v;
    bifdbm_varptr[0] = vptr;
    bifdbm_vartag[0] = vtag;
    Assert(bifdbm_init_dom_size[0] > 1 && *bifdbm_dom[0] != fd_bool);
  } else if (vtag == pm_svar) {

    ozstat.fdvarsCreated.incf();

    bifdbm_var_state[0] = (am.isLocalSVar(v) ? fdbm_local : fdbm_global);
    if (bifdbm_var_state[0] == fdbm_local) {
      GenFDVariable * fdvar = new GenFDVariable();
      OZ_Term * taggedfdvar = newTaggedCVar(fdvar);
      bifdbm_dom[0] = &fdvar->getDom();
      bifdbm_init_dom_size[0] = bifdbm_dom[0]->getSize();
      am.checkSuspensionList(v);
      fdvar->setSuspList(tagged2SVar(v)->getSuspList());
      doBind(vptr, makeTaggedRef(taggedfdvar));
      bifdbm_var[0] = *(bifdbm_varptr[0] = taggedfdvar);
      bifdbm_vartag[0] = pm_fd;
    } else {
      bifdbm_domain[0].initFull();
      bifdbm_dom[0] = &bifdbm_domain[0];
      bifdbm_var[0] = v;
      bifdbm_varptr[0] = vptr;
      bifdbm_vartag[0] = vtag;
     }
  } else if (vtag == pm_uvar) {

    ozstat.fdvarsCreated.incf();

    bifdbm_var_state[0] = (am.isLocalUVar(v)? fdbm_local : fdbm_global);
    if (bifdbm_var_state[0] == fdbm_local) {
      GenFDVariable * fdvar = new GenFDVariable();
      OZ_Term * taggedfdvar = newTaggedCVar(fdvar);
      bifdbm_dom[0] = &fdvar->getDom();
      bifdbm_init_dom_size[0] = bifdbm_dom[0]->getSize();
      doBind(vptr, makeTaggedRef(taggedfdvar));
      bifdbm_var[0] = *(bifdbm_varptr[0] = taggedfdvar);
      bifdbm_vartag[0] = pm_fd;
    } else {
      bifdbm_domain[0].initFull();
      bifdbm_dom[0] = &bifdbm_domain[0];
      *vptr = bifdbm_var[0] = makeTaggedSVar(new SVariable(tagged2VarHome(v)));
      bifdbm_varptr[0] = vptr;
      bifdbm_vartag[0] = pm_svar;
    }

  } else {
    return OZ_FALSE;
  }

  return OZ_TRUE;
} // BIfdBodyManager::introduce

void BIfdBodyManager::_propagate_unify_cd(int clauses, int variables,
					  SRecord &vp)
{
  // 1st pass: mark first occ of a var
  int v, c;
  for (v = 0; v < variables; v++) {
    Assert(bifdbm_vartag[idx_v(v)] == pm_fd ||
	   bifdbm_vartag[idx_v(v)] == pm_bool ||
	   bifdbm_vartag[idx_v(v)] == pm_singl);

    if (isAnyVar(bifdbm_var[idx_v(v)]))
      if (! isTaggedIndex(*bifdbm_varptr[idx_v(v)])) {
	*bifdbm_varptr[idx_v(v)] = makeTaggedIndex(v);
      } else {
	int fs = getIndex(*bifdbm_varptr[idx_v(v)]);
	
	// unify corresponding local variables
	for (c = 0; c < clauses; c++) {
	
	  if (*bifdbm_dom[idx_b(c)] == 0) continue;

	  // check void variables
	  OZ_Term l_var, r_var;
	  
	  if (bifdbm_vartag[idx_vp(c, fs)] == pm_literal) {
	    // convert void variable to heap variable
	    GenFDVariable * fv = new GenFDVariable(*bifdbm_dom[idx_vp(c, fs)]);
	    bifdbm_varptr[idx_vp(c, fs)] = newTaggedCVar(fv);
	    l_var = makeTaggedRef(bifdbm_varptr[idx_vp(c, v)]);
	    bifdbm_vartag[idx_vp(c, fs)] = pm_fd;
	    // update arguments
	    SRecord &vp_c = *tagged2SRecord(deref(vp[c]));
	    vp_c[fs] = l_var;
	  } else if (bifdbm_vartag[idx_vp(c, fs)] == pm_singl) {
	    l_var = bifdbm_var[idx_vp(c, fs)];
	  } else if (bifdbm_vartag[idx_vp(c, fs)] == pm_fd) {
	    l_var = makeTaggedRef(bifdbm_varptr[idx_vp(c, fs)]);
	  } else {
	    Assert(bifdbm_vartag[idx_vp(c, fs)] == pm_bool)

	    l_var = makeTaggedRef(bifdbm_varptr[idx_vp(c, fs)]);
	  }
	  
	  if (bifdbm_vartag[idx_vp(c, v)] == pm_literal) {
	    // convert void variable to heap variable
	    GenFDVariable * fv = new GenFDVariable(*bifdbm_dom[idx_vp(c, v)]);
	    bifdbm_varptr[idx_vp(c, v)] = newTaggedCVar(fv);
	    r_var = makeTaggedRef(bifdbm_varptr[idx_vp(c, v)]);
	    bifdbm_vartag[idx_vp(c, v)] = pm_fd;
	    // update arguments
	    SRecord &vp_c = *tagged2SRecord(deref(vp[c]));
	    vp_c[v] = r_var;
	  } else if (bifdbm_vartag[idx_vp(c, v)] == pm_singl) {
	    r_var = bifdbm_var[idx_vp(c, v)];
	  } else if (bifdbm_vartag[idx_vp(c, v)] == pm_fd) {
	    r_var  = makeTaggedRef(bifdbm_varptr[idx_vp(c, v)]);
	  } else {
	    Assert(bifdbm_vartag[idx_vp(c, v)] == pm_bool);

	    r_var  = makeTaggedRef(bifdbm_varptr[idx_vp(c, v)]);
	  } 

	  backup();

	  if (OZ_unify(l_var, r_var) == FAILED) {
	    restore();
	    *bifdbm_dom[idx_b(c)] &= 0;
	    continue;
	  }

	  restore();
	}
      }
  }
  
  // (re)introduce renamed variable (propagation done by unify) after
  // propagation of equalities has finished
  for (c = clauses; c--; )
    for (v = variables; v--; )
      if ((*bifdbm_dom[idx_b(c)] != 0) && bifdbm_varptr[idx_vp(c, v)])
	introduceLocal(idx_vp(c, v), makeTaggedRef(bifdbm_varptr[idx_vp(c, v)]));
  
  // 2nd pass: undo marks
  for (v = 0; v < variables; v++) {
    if (isAnyVar(bifdbm_var[idx_v(v)]))
      *bifdbm_varptr[idx_v(v)] = bifdbm_var[idx_v(v)];
  }
}


OZ_Boolean BIfdBodyManager::_unifiedVars(void)
{
  OZ_Boolean ret = OZ_FALSE;

  // 1st pass: mark occ of var and break if you find already touched var
  int i;
  for (i = 0; i < curr_num_of_vars; i += 1)
    if (isAnyVar(bifdbm_var[i]))
      if (isTaggedIndex(*bifdbm_varptr[i])) {
	ret = OZ_TRUE;
	break;
      } else {
	*bifdbm_varptr[i] = makeTaggedIndex(i);
      }
  
  // 2nd pass: undo marks
  for (i = 0; i < curr_num_of_vars; i += 1)
    if (isAnyVar(bifdbm_var[i]) && isTaggedIndex(*bifdbm_varptr[i]))
      *bifdbm_varptr[i] = bifdbm_var[i];

  return ret;
  
}

OZ_Return BIfdBodyManager::replacePropagator(OZ_CFun f, int a, OZ_Term * x) 
{
  RefsArray xregs = allocateRefsArray(a, OZ_FALSE);
  
  for (int i = 0; i < a; i += 1)
    xregs[i] = x[i];

  am.currentThread->getCCont()->setCFuncRegs(f, xregs);
  return f(a, xregs);
}

OZ_Return BIfdBodyManager::replacePropagator(OZ_CFun f, int a, OZ_Term t1, ...)
{
  RefsArray xregs = allocateRefsArray(a, OZ_FALSE);

  xregs[0] = t1;

  va_list ap;
  va_start(ap, t1);

  for (int i = 1; i < a; i += 1)
    xregs[i] = va_arg(ap, OZ_Term);

  va_end(ap);

  am.currentThread->getCCont()->setCFuncRegs(f, xregs);
  return f(a, xregs);
}

OZ_Return BIfdBodyManager::replacePropagator(OZ_Term a, OZ_Term b)
{
  //  kost@: 
  //  'OZ_unify ()' returns only 'PROCEED' or 'FAILED';
  //  In both cases the propagator will be closed by the 
  // caller routine;
  // 
  // am.currentThread->closeDonePropagator ();

  //
  //  Tobias: should this method be inlined?    // TODO?
  return OZ_unify(a, b);
}

void BIfdBodyManager::printDebug(void) {
  for (int i = 0; i < curr_num_of_vars; i += 1)
    printDebug(i);
}

void BIfdBodyManager::printDebug(int i) {
  if (bifdbm_dom[i]) {
    cerr << '[' << i << "]: v=" << (void *) bifdbm_var[i]
	 << ", vptr=" << (void *) bifdbm_varptr[i]
	 << ", vtag=" << pm_term_type2string(bifdbm_vartag[i])
	 << ", dom=" << *bifdbm_dom[i]
	 << ", ids=" << bifdbm_init_dom_size[i]
	 << ", var_state=" << fdbm_var_stat2char(bifdbm_var_state[i]) 
	 << endl << flush;
  } else {
    cerr << "unvalid field" << endl << flush;
  }
}

void BIfdBodyManager::printTerm(void) {
  for (int i = 0; i < curr_num_of_vars; i += 1)
    printTerm(i);
}

void BIfdBodyManager::printTerm(int i) {
  if (*bifdbm_varptr[i] == bifdbm_var[i]) {
    cout << "index=" << i << endl;
    taggedPrintLong(makeTaggedRef(bifdbm_varptr[i]));
    cout << endl << flush;
  } else {
    cout << "ATTENTION *bifdbm_varptr[i]!=bifdbm_var[i]. index="
	 << i << endl;
    cout << "bifdbm_varptr";
    taggedPrintLong(makeTaggedRef(bifdbm_varptr[i]));
    cout << endl << flush;
    cout << "bifdbm_var";
    taggedPrintLong(bifdbm_var[i]);
    cout << endl << flush;
  }
}


#ifdef DEBUG_STABLE

OZ_C_proc_begin(debugStable,0)
{ 
  printBCDebug();
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(resetStable,0)
{ 
  board_constraints = NULL;
  return PROCEED;
}
OZ_C_proc_end

#endif
//-----------------------------------------------------------------------------
// Introduce FD Built-ins to the Emulator

static
BIspec fdSpec[] = {
// fdprofil.cc
#ifdef PROFILE_FD
  {"fdReset", 0, BIfdReset},
  {"fdDiscard", 0, BIfdDiscard},
  {"fdGetNext", 1, BIfdGetNext},
  {"fdPrint", 0, BIfdPrint},
  {"fdTotalAverage", 0, BIfdTotalAverage},
#endif

// fdcore.cc
  {"fdIs", 1, BIfdIs},
  {"fdIsVar", 1, BIisFdVar},
  {"fdIsVarB", 2, BIisFdVarB},
  {"fdGetLimits", 2, BIgetFDLimits},
  {"fdGetMin", 2, BIfdMin},
  {"fdGetMax", 2, BIfdMax},
  {"fdGetDom", 2, BIfdGetAsList},
  {"fdGetCard", 2, BIfdGetCardinality},
  {"fdNextTo", 3, BIfdNextTo},

  {"fdPutLe", 2, BIfdPutLe},
  {"fdPutGe", 2, BIfdPutGe},
  {"fdPutList", 3, BIfdPutList},
  {"fdPutInterval", 3, BIfdPutInterval},
  {"fdPutNot", 2, BIfdPutNot},
  
// fdrel.cc
  {"fdMinimum", 3, BIfdMinimum},
  {"fdMinimum_body", 3, BIfdMinimum_body},
  {"fdMaximum", 3, BIfdMaximum},
  {"fdMaximum_body", 3, BIfdMaximum_body},
  {"fdUnion", 3, BIfdUnion},
  {"fdUnion_body", 3, BIfdUnion_body},
  {"fdIntersection", 3, BIfdIntersection},
  {"fdIntersection_body", 3, BIfdIntersection_body},
  {"fdSubsume_body", 3, BIfdSubsume_body},
  {"fdLessEqOff", 3, BIfdLessEqOff},
  {"fdLessEqOff_body", 3, BIfdLessEqOff_body},
  {"fdNotEqEnt", 2, BIfdNotEqEnt},
  {"fdNotEqEnt_body", 2, BIfdNotEqEnt_body},
  {"fdNotEq", 2, BIfdNotEq},
  {"fdNotEq_body", 2, BIfdNotEq_body},
  {"fdNotEqOffEnt", 3, BIfdNotEqOffEnt},
  {"fdNotEqOffEnt_body", 3, BIfdNotEqOffEnt_body},
  {"fdNotEqOff", 3, BIfdNotEqOff},
  {"fdNotEqOff_body", 3, BIfdNotEqOff_body},
  {"fdAllDifferent", 1, BIfdAllDifferent},
  {"fdAllDifferent_body", 1, BIfdAllDifferent_body},
  {"fdDistinctOffset", 2, BIfdDistinctOffset},
  {"fdDistinctOffset_body", 2, BIfdDistinctOffset_body},
  {"fdCPIterate", 2, BIfdCPIterate},
  {"fdCPIterate_body", 2, BIfdCPIterate_body},
  {"fdCPIterate2", 2, BIfdCPIterate2},
  {"fdCPIterate2_body", 2, BIfdCPIterate2_body},
  
// fdbool.cc
  {"fdAnd", 3, BIfdAnd},
  {"fdAnd_body", 3, BIfdAnd_body},
  {"fdOr", 3, BIfdOr},
  {"fdOr_body", 3, BIfdOr_body},
  {"fdNot", 2, BIfdNot},
  {"fdNot_body", 2, BIfdNot_body},
  {"fdXor", 3, BIfdXor},
  {"fdXor_body", 3, BIfdXor_body},
  {"fdEquiv", 3, BIfdEquiv},
  {"fdEquiv_body", 3, BIfdEquiv_body},
  {"fdImpl", 3, BIfdImpl},
  {"fdImpl_body", 3, BIfdImpl_body},
  
// fdarith.cc
  {"fdPlus", 3, BIfdPlus},
  {"fdPlus_body", 3, BIfdPlus_body},
  {"fdTwice_body", 2, BIfdTwice_body},
  {"fdMinus", 3, BIfdMinus},
  {"fdMinus_body", 3, BIfdMinus_body},
  {"fdMult", 3, BIfdMult},
  {"fdMult_body", 3, BIfdMult_body},
  {"fdSquare_body", 2, BIfdSquare_body},
  {"fdDiv", 3, BIfdDiv},
  {"fdDiv_body", 3, BIfdDiv_body},
  {"fdDivInterval", 3, BIfdDivInterval},
  {"fdDivInterval_body", 3, BIfdDivInterval_body},
  {"fdMod", 3, BIfdMod},
  {"fdMod_body", 3, BIfdMod_body},
  {"fdModInterval", 3, BIfdModInterval},
  {"fdModInterval_body", 3, BIfdModInterval_body},
  {"fdPlus_rel", 3, BIfdPlus_rel},
  {"fdMult_rel", 3, BIfdMult_rel},
  
// fdgeneric.cc
  {"fdGenLinEq", 3, BIfdGenLinEq},
  {"fdGenLinEq_body", 3, BIfdGenLinEq_body},
  {"fdGenNonLinEq", 3, BIfdGenNonLinEq},
  {"fdGenNonLinEq1", 3, BIfdGenNonLinEq1},
  {"fdGenNonLinEq_body", 3, BIfdGenNonLinEq_body},
  {"fdGenLinNotEq", 3, BIfdGenLinNotEq},
  {"fdGenLinNotEq_body", 3, BIfdGenLinNotEq_body},
  {"fdGenNonLinNotEq", 3, BIfdGenNonLinNotEq},
  {"fdGenNonLinNotEq_body", 3, BIfdGenNonLinNotEq_body},
  {"fdGenLinLessEq", 3, BIfdGenLinLessEq},
  {"fdGenLinLessEq_body", 3, BIfdGenLinLessEq_body},
  {"fdGenNonLinLessEq", 3, BIfdGenNonLinLessEq},
  {"fdGenNonLinLessEq1", 3, BIfdGenNonLinLessEq1},
  {"fdGenNonLinLessEq_body", 3, BIfdGenNonLinLessEq_body},
  
// fdcount.cc
  {"fdElement", 3, BIfdElement},
  {"fdElement_body", 3, BIfdElement_body},
  {"fdAtMost", 3, BIfdAtMost},
  {"fdAtMost_body", 3, BIfdAtMost_body},
  {"fdAtLeast", 3, BIfdAtLeast},
  {"fdAtLeast_body", 3, BIfdAtLeast_body},
  {"fdCount", 3, BIfdCount},
  {"fdCount_body", 3, BIfdCount_body},

// fdcard.cc
  {"fdCardBIBin", 2, BIfdCardBIBin},
  {"fdCardBIBin_body", 2, BIfdCardBIBin_body},
  {"fdCardNestableBI", 4, BIfdCardNestableBI},
  {"fdCardNestableBI_body", 4, BIfdCardNestableBI_body},
  {"fdCardNestableBIBin", 3, BIfdCardNestableBIBin},
  {"fdCardNestableBIBin_body", 3, BIfdCardNestableBIBin_body},
  {"fdInB", 3, BIfdInB},
  {"fdInB_body", 3, BIfdInB_body},
  {"fdNotInB", 3, BIfdNotInB},
  {"fdNotInB_body", 3, BIfdNotInB_body},
  {"fdGenLinEqB", 4, BIfdGenLinEqB},
  {"fdGenNonLinEqB", 4, BIfdGenNonLinEqB},
  {"fdGenLinEqB_body", 4, BIfdGenLinEqB_body},
  {"fdGenLinNotEqB", 4, BIfdGenLinNotEqB},
  {"fdGenNonLinNotEqB", 4, BIfdGenNonLinNotEqB},
  {"fdGenLinNotEqB_body", 4, BIfdGenLinNotEqB_body},
  {"fdGenLinLessEqB", 4, BIfdGenLinLessEqB},
  {"fdGenLinLessEqB_body", 4, BIfdGenLinLessEqB_body},
  {"fdGenNonLinLessEqB", 4, BIfdGenNonLinLessEqB},

// fdcd.cc
  {"fdConstrDisjSetUp", 4, BIfdConstrDisjSetUp},
  {"fdConstrDisj", 3, BIfdConstrDisj},
#ifndef PROPAGATOR_CD
  {"fdConstrDisj_body", 3, BIfdConstrDisj_body},
#endif 

  {"fdGenLinEqCD", 4, BIfdGenLinEqCD},
  {"fdGenLinEqCD_body", 4, BIfdGenLinEqCD_body},
  {"fdGenNonLinEqCD", 4, BIfdGenNonLinEqCD},
  {"fdGenLinNotEqCD", 4, BIfdGenLinNotEqCD},
  {"fdGenLinNotEqCD_body", 4, BIfdGenLinNotEqCD_body},
  {"fdGenNonLinNotEqCD", 4, BIfdGenNonLinNotEqCD},
  {"fdGenLinLessEqCD", 4, BIfdGenLinLessEqCD},
  {"fdGenLinLessEqCD_body", 4, BIfdGenLinLessEqCD_body},
  {"fdGenNonLinLessEqCD", 4, BIfdGenNonLinLessEqCD},
  {"fdPlusCD", 4, BIfdPlusCD_rel},
  {"fdPlusCD_body", 4, BIfdPlusCD_rel_body},
  {"fdMultCD", 4, BIfdMultCD_rel},
  {"fdMultCD_body", 4, BIfdMultCD_rel_body},

  {"fdLessEqOffCD", 4, BIfdLessEqOffCD},
  {"fdLessEqOffCD_body", 4, BIfdLessEqOffCD_body},
  {"fdNotEqCD", 3, BIfdNotEqCD},
  {"fdNotEqCD_body", 3, BIfdNotEqCD_body},
  {"fdNotEqOffCD", 4, BIfdNotEqOffCD},
  {"fdNotEqOffCD_body", 4, BIfdNotEqOffCD_body},

  {"fdPutLeCD", 3, BIfdPutLeCD},
  {"fdPutGeCD", 3, BIfdPutGeCD},
  {"fdPutListCD", 4, BIfdPutListCD},
  {"fdPutIntervalCD", 4, BIfdPutIntervalCD},
  {"fdPutNotCD", 3, BIfdPutNotCD},

// fdwatch.cc
  {"fdWatchDomB", 3, BIfdWatchDomB},
  {"fdWatchDom1", 2, BIfdWatchDom1},
  {"fdWatchDom2", 4, BIfdWatchDom2},
  {"fdWatchDom3", 6, BIfdWatchDom3},

  {"fdWatchBounds1", 3, BIfdWatchBounds1},
  {"fdWatchBounds2", 6, BIfdWatchBounds2},
  {"fdWatchBounds3", 9, BIfdWatchBounds3},

// fdmisc.cc
  {"fdCardSched", 4, BIfdCardSched},
  {"fdCardSched_body", 4, BIfdCardSched_body},
  {"fdCardSchedControl", 5, BIfdCardSchedControl},
  {"fdCardSchedControl_body", 5, BIfdCardSchedControl_body},
  {"fdCDSched", 4, BIfdCDSched},
  {"fdCDSched_body", 4, BIfdCDSched_body},
  {"fdCDSchedControl", 5, BIfdCDSchedControl},
  {"fdCDSchedControl_body", 5, BIfdCDSchedControl_body},
  {"fdNoOverlap", 6, BIfdNoOverlap},
  {"fdNoOverlap_body", 6, BIfdNoOverlap_body},
  {"fdGenLinEqKillB", 4, BIfdGenLinEqKillB},
  {"fdGenLinEqKillB_body", 4, BIfdGenLinEqKillB_body},
  {"fdGenLinLessEqKillB", 4, BIfdGenLinLessEqKillB},
  {"fdGenLinLessEqKillB_body", 4, BIfdGenLinLessEqKillB_body},
  {"fdCardBIKill", 4, BIfdCardBIKill},
  {"fdCardBIKill_body", 4, BIfdCardBIKill_body},
  {"fdInKillB", 3, BIfdInKillB},
  {"fdInKillB_body", 3, BIfdInKillB_body},
  {"fdNotInKillB", 3, BIfdNotInKillB},
  {"fdNotInKillB_body", 3, BIfdNotInKillB_body},
  {"fdCopyDomain", 2, BIfdCopyDomain},
  {"fdDivDomCons", 3, BIfdDivIntervalCons},
  {"getCopyStat", 1, BIgetCopyStat},
#ifndef FOREIGNFDPROPS
  {"fdp_init", 0, fdp_init},
  {"fdp_plus_rel", 3, fdp_plus_rel},
  {"fdp_plus", 3, fdp_plus},
  {"fdp_minus", 3, fdp_minus},
  {"fdp_times", 3, fdp_times},
  {"fdp_times_rel", 3, fdp_times_rel},
  {"fdp_divD", 3, fdp_divD},
  {"fdp_divI", 3, fdp_divI},
  {"fdp_modD", 3, fdp_modD},
  {"fdp_modI", 3, fdp_modI},
  {"fdp_con", 3, fdp_con},
  {"fdp_dis", 3, fdp_dis},
  {"fdp_xor", 3, fdp_xor},
  {"fdp_imp", 3, fdp_imp},
  {"fdp_equ", 3, fdp_equ},
  {"fdp_neg", 2, fdp_neg},
  {"fdp_sprodEqR", 4, fdp_sprodEqR},
  {"fdp_sprodEqNLR", 4, fdp_sprodEqNLR},
  {"fdp_sprodLessEqR", 4, fdp_sprodLessEqR},
  {"fdp_sprodLessEqNLR", 4, fdp_sprodLessEqNLR},
  {"fdp_sprodNotEqR", 4, fdp_sprodNotEqR},
  {"fdp_sprodNotEqNLR", 4, fdp_sprodNotEqNLR},
  {"fdp_intR", 3, fdp_intR},
  {"fdp_card", 4, fdp_card},
  {"fdp_sprodEqCD", 4, fdp_sprodEqCD},
  {"fdp_sprodEqNLCD", 4, fdp_sprodEqNLCD},
  {"fdp_sprodLessEqCD", 4, fdp_sprodLessEqCD},
  {"fdp_sprodLessEqNLCD", 4, fdp_sprodLessEqNLCD},
  {"fdp_sprodNotEqCD", 4, fdp_sprodNotEqCD},
  {"fdp_sprodNotEqNLCD", 4, fdp_sprodNotEqNLCD},
  {"fdp_exactly", 3, fdp_exactly},
  {"fdp_atLeast", 3, fdp_atLeast},
  {"fdp_atMost", 3, fdp_atMost},
  {"fdp_element", 3, fdp_element},
  {"fdp_sprodEq", 3, fdp_sprodEq},
  {"fdp_sprodEqNL", 3, fdp_sprodEqNL},
  {"fdp_sprodNotEq", 3, fdp_sprodNotEq},
  {"fdp_sprodNotEqNL", 3, fdp_sprodNotEqNL},
  {"fdp_sprodLessEq", 3, fdp_sprodLessEq},
  {"fdp_sprodLessEqNL", 3, fdp_sprodLessEqNL},
  {"fdp_sprodEqNLP", 3, fdp_sprodEqNLP},
  {"fdp_sprodLessEqNLP", 3, fdp_sprodLessEqNLP},
  {"fdp_notEqOff", 3, fdp_notEqOff},
  {"fdp_lessEqOff", 3, fdp_lessEqOff},
  {"fdp_minimum", 3, fdp_minimum},
  {"fdp_maximum", 3, fdp_maximum},
  {"fdp_inter", 3, fdp_inter},
  {"fdp_union", 3, fdp_union},
  {"fdp_distinct", 1, fdp_distinct},
  {"fdp_distinctOffset", 2, fdp_distinctOffset},
  {"fdp_disjoint", 4, fdp_disjoint},
  {"fdp_disjointC", 5, fdp_disjointC},
#endif
#ifdef DEBUG_STABLE
  {"debugStable", 0, debugStable},
  {"resetStable", 0, resetStable},
#endif
  {0,0,0,0}
};

void BIinitFD(void)
{
  BIaddSpec(fdSpec);
}


