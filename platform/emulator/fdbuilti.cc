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

OZ_FiniteDomain __CDVoidFiniteDomain;

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
  OZ_Thread th = OZ_makeSuspendedThread(f, x, a);

  for (int i = curr_num_of_items; i--; )
    if (pm_is_noncvar(bifdhm_vartag[i])) {
      AssertFD(isAnyVar(*bifdhm_varptr[i]));
      OZ_addThread(makeTaggedRef(bifdhm_varptr[i]), th);
    }

  return PROCEED;
}

OZ_Return BIfdHeadManager::addSuspSingl(OZ_CFun f, OZ_Term * x, int a) {
  OZ_Thread th = OZ_makeSuspendedThread(f, x, a);

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
    OZ_Thread th = OZ_makeSuspendedThread(f, x, a);
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
    addSuspFDVar(bifdhm_var[i], thr, target);
    if (! am.isLocalCVar(bifdhm_var[i])) global_vars += 1;
  } else if (tag == pm_bool) {
    addSuspBoolVar(bifdhm_var[i], thr);
    if (! am.isLocalCVar(bifdhm_var[i])) global_vars += 1;
  } else if (tag == pm_uvar) {
    if (bifdhm_var[i] != *bifdhm_varptr[i]) return;
    if (am.isLocalUVar(bifdhm_var[i])) {
      OZ_Term * taggedfdvar = newTaggedCVar(new GenFDVariable());
      addSuspFDVar(*taggedfdvar, thr, target);
      doBind(bifdhm_varptr[i], makeTaggedRef(taggedfdvar));
    } else {
      global_vars += 1;
      addSuspUVar(bifdhm_varptr[i], thr);
    } 
  } else {
    Assert(tag == pm_svar);
    
    if (bifdhm_var[i] != *bifdhm_varptr[i]) return;
    if (am.isLocalSVar(bifdhm_var[i])) {
      GenFDVariable * fdvar = new GenFDVariable();
      OZ_Term * taggedfdvar = newTaggedCVar(fdvar);
      am.checkSuspensionList(bifdhm_var[i]);
      fdvar->setSuspList(tagged2SVar(bifdhm_var[i])->getSuspList());
      addSuspFDVar(*taggedfdvar, thr, target);
      doBind(bifdhm_varptr[i], makeTaggedRef(taggedfdvar));
    } else {
      global_vars += 1;
      addSuspSVar(bifdhm_var[i],thr);
    } 
  } 
}

void BIfdHeadManager::printDebug(void) {
  for (int i = 0; i < curr_num_of_items; i += 1)
    printDebug(i);
}

void BIfdHeadManager::printDebug(int i) {
  cout << '[' << i << "]: var=" << (void *) bifdhm_var[i]
       << ", varptr=" << (void *) bifdhm_varptr[i]
       << ", vartag=" << pm_term_type2string(bifdhm_vartag[i])
       << ", coeff=" << bifdhm_coeff[i] << endl << flush;
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
  OZ_addThread(makeTaggedRef(t), OZ_makeSuspendedThread(f, x, a));
  return PROCEED;
}

OZ_Return BIfdHeadManager::suspendOnVar(OZ_CFun f, int a, OZ_Term * x,
				      OZ_Term * t1, OZ_Term * t2)
{
  OZ_Thread th = OZ_makeSuspendedThread(f, x, a);
  OZ_addThread(makeTaggedRef(t1), th);
  OZ_addThread(makeTaggedRef(t2), th);
  return PROCEED;
}

OZ_Return BIfdHeadManager::suspendOnVar(OZ_CFun f, int a, OZ_Term * x,
				      OZ_Term * t1, OZ_Term * t2, OZ_Term * t3)
{
  OZ_Thread th = OZ_makeSuspendedThread(f, x, a);
  OZ_addThread(makeTaggedRef(t1), th);
  OZ_addThread(makeTaggedRef(t2), th);
  OZ_addThread(makeTaggedRef(t3), th);
  return PROCEED;
}
#endif

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
	tagged2GenFDVar(bifdbm_var[i])->getDom() = bifdbm_domain[i];
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

// produces GCTAG 8 + 5 = 13
const int taggedIndex = 0x00000008; // is REF, cannot happen for derefed term
inline OZ_Term makeTaggedIndex(int i) {return ((i << 4) | taggedIndex);}
inline int getIndex(OZ_Term t) {return (t & ~taggedIndex) >> 4;}
inline OZ_Boolean isTaggedIndex(OZ_Term t) {return (t & taggedIndex);}

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
      Assert(*bifdbm_dom[i] == fd_singl);
      
      tagged2GenBoolVar(bifdbm_var[i])->
	becomesSmallIntAndPropagate(bifdbm_varptr[i], *bifdbm_dom[i]);
    } else {
      Assert(bifdbm_vartag[i] == pm_fd);
      
      if (*bifdbm_dom[i] == fd_singl) {
	tagged2GenFDVar(bifdbm_var[i])->
	  becomesSmallIntAndPropagate(bifdbm_varptr[i]);
      } else if (*bifdbm_dom[i] == fd_bool) {
	tagged2GenFDVar(bifdbm_var[i])->
	  becomesBoolVarAndPropagate(bifdbm_varptr[i]);
	vars_left = OZ_TRUE;
      } else {
	tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_prop_bounds);
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
    bifdbm_domain[i].initEmpty();
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
    } else if (bifdbm_var[i] != *bifdbm_varptr[i]) { // avoid multiple telling
      continue;                                      // of global vars
    } else if (! isTouched(i)) {
      vars_left = OZ_TRUE;
    } else if (vartag == pm_fd) {
      if (*bifdbm_dom[i] == fd_singl) {
	if (bifdbm_var_state[i] == fdbm_local) {
	  tagged2GenFDVar(bifdbm_var[i])->
	    becomesSmallIntAndPropagate(bifdbm_varptr[i]);
	} else {
	  tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_prop_singl);
	  am.doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i],
			    OZ_int(bifdbm_dom[i]->getSingleElem()));
	}
      } else if (*bifdbm_dom[i] == fd_bool) {

	if (bifdbm_var_state[i] == fdbm_local) {
	  tagged2GenFDVar(bifdbm_var[i])->
	    becomesBoolVarAndPropagate(bifdbm_varptr[i]);
	} else {
	  tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_prop_bounds);
	  GenBoolVariable * newboolvar = new GenBoolVariable();
	  OZ_Term * newtaggedboolvar = newTaggedCVar(newboolvar);
	  am.doBindAndTrailAndIP(bifdbm_var[i], bifdbm_varptr[i],
				 makeTaggedRef(newtaggedboolvar),
				 newboolvar, tagged2GenBoolVar(bifdbm_var[i]),
				 NO);
	}

      } else {

	tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_prop_bounds);
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
      Assert(*bifdbm_dom[i] == fd_singl); 

      if (bifdbm_var_state[i] == fdbm_local) {
	tagged2GenBoolVar(bifdbm_var[i])->
	  becomesSmallIntAndPropagate(bifdbm_varptr[i], *bifdbm_dom[i]);
      } else {
	tagged2GenBoolVar(bifdbm_var[i])->propagate(bifdbm_var[i]);
	am.doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i],
			  OZ_int(bifdbm_dom[i]->getSingleElem()));
      }
    } else {
      Assert(vartag == pm_svar && bifdbm_var_state[i] == fdbm_global);

      ozstat.fdvarsCreated.incf();

      if (*bifdbm_dom[i] == fd_singl) {
	OZ_Term smallInt = OZ_int(bifdbm_dom[i]->getSingleElem());
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

    if (*bifdbm_dom[0] == fd_singl) {
      if (bifdbm_var_state[0] == fdbm_local) {
	tagged2GenFDVar(bifdbm_var[0])->
	  becomesSmallIntAndPropagate(bifdbm_varptr[0]);
      } else {
	tagged2GenFDVar(bifdbm_var[0])->propagate(bifdbm_var[0], fd_prop_singl);
	am.doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0],
			  OZ_int(bifdbm_dom[0]->getSingleElem()));
      }
    } else if (*bifdbm_dom[0] == fd_bool) {
      tagged2GenFDVar(bifdbm_var[0])->propagate(bifdbm_var[0], fd_prop_bounds);
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
      tagged2GenFDVar(bifdbm_var[0])->propagate(bifdbm_var[0], fd_prop_bounds);
      if (bifdbm_var_state[0] == fdbm_global) {
	GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[0]);
	OZ_Term * newtaggedfdvar = newTaggedCVar(newfdvar);
	am.doBindAndTrailAndIP(bifdbm_var[0], bifdbm_varptr[0],
			       makeTaggedRef(newtaggedfdvar),
			       newfdvar, tagged2GenFDVar(bifdbm_var[0]), NO);
      } 
    }

  } else if (vartag == pm_bool) {
    Assert(*bifdbm_dom[0] == fd_singl);

    if (bifdbm_var_state[0] == fdbm_local) {
      tagged2GenBoolVar(bifdbm_var[0])->
	becomesSmallIntAndPropagate(bifdbm_varptr[0], *bifdbm_dom[0]);
    } else {
      tagged2GenBoolVar(bifdbm_var[0])->propagate(bifdbm_var[0]);
      am.doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0],
			OZ_int(bifdbm_dom[0]->getSingleElem()));
    }
  } else {
    Assert(bifdbm_var_state[0] == fdbm_global && vartag == pm_svar);
    
    ozstat.fdvarsCreated.incf();

    if (*bifdbm_dom[0] == fd_singl) {
      OZ_Term smallInt = OZ_int(bifdbm_dom[0]->getSingleElem());
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
      if (am.currentBoard->isRoot())
	bifdbm_domain[0] = fdvar->getDom();
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
	    l_var = makeTaggedRef(bifdbm_varptr[idx_vp(c, fs)]);
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

void BIfdBodyManager::printDebug(void) {
  for (int i = 0; i < curr_num_of_vars; i += 1)
    printDebug(i);
}

void BIfdBodyManager::printDebug(int i) {
  if (bifdbm_dom[i]) {
    cout << '[' << i << "]: v=" << (void *) bifdbm_var[i]
	 << ", vptr=" << (void *) bifdbm_varptr[i]
	 << ", vtag=" << pm_term_type2string(bifdbm_vartag[i])
	 << ", dom=" << *bifdbm_dom[i]
	 << ", ids=" << bifdbm_init_dom_size[i]
	 << ", var_state=" << fdbm_var_stat2char(bifdbm_var_state[i]) 
	 << endl << flush;
  } else {
    cout << "unvalid field" << endl << flush;
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
  {"fdIs", 2, BIfdIs},
  {"fdIsVar", 1, BIisFdVar},
  {"fdIsVarB", 2, BIisFdVarB},
  {"fdGetLimits", 2, BIgetFDLimits},
  {"fdGetMin", 2, BIfdMin},
  {"fdGetMid", 2, BIfdMid},
  {"fdGetMax", 2, BIfdMax},
  {"fdGetDom", 2, BIfdGetAsList},
  {"fdGetCard", 2, BIfdGetCardinality},
  {"fdGetNextSmaller", 3, BIfdNextSmaller},
  {"fdGetNextLarger", 3, BIfdNextLarger},

  {"fdTellConstraint", 2, BIfdTellConstraint},

  {"fdWatchSize", 3, BIfdWatchSize},
  {"fdWatchMin", 3, BIfdWatchMin},
  {"fdWatchMax", 3, BIfdWatchMax},
  
// fdcd.cc
  {"fdConstrDisjSetUp", 4, BIfdConstrDisjSetUp},
  {"fdConstrDisj", 3, BIfdConstrDisj},
  {"fdTellConstraintCD", 3, BIfdTellConstraintCD},

#ifndef FOREIGNFDPROPS
  {"fdp_init", 0, fdp_init},
  {"fdp_sum", 3, fdp_sum},
  {"fdp_sumC", 4, fdp_sumC},
  {"fdp_sumCN", 4, fdp_sumCN},
  {"fdp_sumR", 4, fdp_sumR},
  {"fdp_sumCR", 5, fdp_sumCR},
  {"fdp_sumCNR", 5, fdp_sumCNR},
  {"fdp_sumCD", 4, fdp_sumCD},
  {"fdp_sumCCD", 5, fdp_sumCCD},
  {"fdp_sumCNCD", 5, fdp_sumCNCD},
  {"fdp_plus_rel", 3, fdp_plus_rel},
  {"fdp_plus", 3, fdp_plus},
  {"fdp_minus", 3, fdp_minus},
  {"fdp_times", 3, fdp_times},
  {"fdp_times_rel", 3, fdp_times_rel},
  {"fdp_power", 3, fdp_power},
  {"fdp_divD", 3, fdp_divD},
  {"fdp_divI", 3, fdp_divI},
  {"fdp_modD", 3, fdp_modD},
  {"fdp_modI", 3, fdp_modI},
  {"fdp_conj", 3, fdp_conj},
  {"fdp_disj", 3, fdp_disj},
  {"fdp_exor", 3, fdp_exor},
  {"fdp_impl", 3, fdp_impl},
  {"fdp_equi", 3, fdp_equi},
  {"fdp_nega", 2, fdp_nega},
  {"fdp_intR", 3, fdp_intR},
  {"fdp_card", 4, fdp_card},
  {"fdp_exactly", 3, fdp_exactly},
  {"fdp_atLeast", 3, fdp_atLeast},
  {"fdp_atMost", 3, fdp_atMost},
  {"fdp_element", 3, fdp_element},
  {"fdp_notEqOff", 3, fdp_notEqOff},
  {"fdp_lessEqOff", 3, fdp_lessEqOff},
  {"fdp_minimum", 3, fdp_minimum},
  {"fdp_maximum", 3, fdp_maximum},
  {"fdp_inter", 3, fdp_inter},
  {"fdp_union", 3, fdp_union},
  {"fdp_distinct", 1, fdp_distinct},
  {"fdp_distinctStream", 2, fdp_distinctStream},
  {"fdp_distinctOffset", 2, fdp_distinctOffset},
  {"fdp_disjoint", 4, fdp_disjoint},
  {"sched_disjoint_card", 4, sched_disjoint_card},
  {"fdp_disjointC", 5, fdp_disjointC},
  {"fdp_distance", 4, fdp_distance},
  {"sched_cpIterate", 2, sched_cpIterate},
  {"sched_cpIterateCap", 4, sched_cpIterateCap},
  {"sched_cpIterateCapUp", 4, sched_cpIterateCapUp},
  {"sched_taskIntervals", 2, sched_taskIntervals},
  {"sched_disjunctive", 2, sched_disjunctive},
  {"sched_disjunctiveStream", 3, sched_disjunctiveStream},
  // dummies
  {"fdp_twice", 2, fdp_twice},
  {"fdp_square", 2, fdp_square},
  {"fdp_subset", 2, fdp_subset},

  //
  {"fdp_dsum", 3, fdp_dsum},
  {"fdp_dsumC", 4, fdp_dsumC},  
  {"fdp_sumAC", 4, fdp_sumAC},


  {"counter", 2, fdtest_counter},
  {"firstFail", 2, fdtest_firstFail},
  {"sched_taskIntervalsProof", 5, sched_taskIntervalsProof},
  {"sched_firstsLasts", 5, sched_firstsLasts},
  {"spawnLess", 2, fdtest_spawnLess},
  {"dplus", 3, fdtest_plus},
  {"sumac", 3, fdtest_sumac},

#endif /* FOREIGNFDPROPS */

#ifdef DEBUG_STABLE
  {"debugStable", 0, debugStable},
  {"resetStable", 0, resetStable},
#endif

  // Distribution builtins: fddist.cc
  {"fddistribute", 5, BIfdDistribute},
  {"fdgetCandidates", 5, BIfdGetCandidates},
  {"fddistributeMinPairs", 5, BIfdDistributeMinPairs},
  {"fddistributeTaskIntervals", 7, BIfdDistributeTaskIntervals},
  {"fddistributeTaskIntervalsOpt", 7, BIfdDistributeTaskIntervalsOpt},


  {0,0,0,0}
};

void BIinitFD(void)
{
  BIaddSpec(fdSpec);
}


