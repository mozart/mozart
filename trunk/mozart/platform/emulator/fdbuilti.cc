/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(__GNUC__) && !defined(NOPRAGMA)
#pragma implementation "fdbuilti.hh"
#endif

#include "fdbuilti.hh"
#include "fdprofil.hh"

#include <stdarg.h>

#if defined(OUTLINE) || defined(FDOUTLINE)
#define inline 
#include "fdbuilti.icc"
#undef inline 
#endif


//-----------------------------------------------------------------------------
// class BIfdHeadManager
//-----------------------------------------------------------------------------
// Global data which are shared between different pieces of code

TaggedRef static_var[MAXFDBIARGS];
TaggedRefPtr static_varptr[MAXFDBIARGS];
pm_term_type static_vartag[MAXFDBIARGS];
double static_coeff_double[MAXFDBIARGS];
int static_coeff_int[MAXFDBIARGS];
Bool static_sign_bit[MAXFDBIARGS];

Bool static_bool_a[MAXFDBIARGS];
Bool static_bool_b[MAXFDBIARGS];
int static_int_a[MAXFDBIARGS];
int static_int_b[MAXFDBIARGS];
double static_double_a[MAXFDBIARGS];
double static_double_b[MAXFDBIARGS];

int static_index_offset[MAXFDBIARGS];
int static_index_size[MAXFDBIARGS];

FiniteDomain __CDVoidFiniteDomain(fd_full);

//-----------------------------------------------------------------------------
// Member functions

TaggedRef * BIfdHeadManager::bifdhm_var;
TaggedRefPtr * BIfdHeadManager::bifdhm_varptr;
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

Bool BIfdHeadManager::expectNonLin(int i, STuple &at, STuple &xt,
				   TaggedRef tagged_xtc, int &s,
				   OZ_CFun f, RefsArray x, int a)
{
  DebugCheck(i < 0 || i >= curr_num_of_items, error("index overflow"));

  deref(tagged_xtc, bifdhm_varptr[i], bifdhm_vartag[i]);
  
  if (bifdhm_vartag[i] == pm_literal) {
    if (tagged_xtc == AtomPair) {
      bifdhm_var[i] = newSmallInt(0);
      bifdhm_vartag[i] = pm_singl;
      return TRUE;
    }
    return FALSE;
  }
  
  if (! (bifdhm_vartag[i] == pm_tuple)) {
    bifdhm_var[i] = tagged_xtc;
    return TRUE;
  }

  STuple &xtc = *tagged2STuple(tagged_xtc);
  const int ts = xtc.getSize();
  TaggedRef last_fdvar;
  TaggedRefPtr last_fdvarptr = NULL, prev_fdvarptr;
  TaggedRef var;
  TaggedRefPtr varptr;
  pm_term_type vartag;
  long prod = 1;
  pm_term_type last_tag = pm_none;

  int j, fds_found;
  for (j = ts, fds_found = 0; j-- && (fds_found < 2); ) {
    var = makeTaggedRef(&xtc[j]);
    deref(var, varptr, vartag);
 
    if (vartag == pm_singl) {
      prod *= smallIntValue(var);
      if (prod < OzMinInt || OzMaxInt < prod) return FALSE;
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
    xt[i] = bifdhm_var[i] = newSmallInt(1);
    bifdhm_coeff[i] *= prod;
    if (bifdhm_coeff[i] < OzMinInt || OzMaxInt < bifdhm_coeff[i]) return FALSE;
    at[i] = newSmallInt(bifdhm_coeff[i]);
    return TRUE;
  case 1: // exactly one variable left
    bifdhm_vartag[i] = last_tag;
    bifdhm_var[i] = last_fdvar;
    xt[i] = makeTaggedRef(bifdhm_varptr[i] = last_fdvarptr);
    bifdhm_coeff[i] *= prod;
    if (bifdhm_coeff[i] < OzMinInt || OzMaxInt < bifdhm_coeff[i]) return FALSE;
    at[i] = newSmallInt(bifdhm_coeff[i]);
    return TRUE;
  case 2:
    s += 1;
#ifdef FDBISTUCK
    am.addSuspendVarList(prev_fdvarptr);
    am.addSuspendVarList(last_fdvarptr);
#else
    {
      OZ_Thread th = OZ_makeSuspension(f, x, a);
      OZ_addSuspension(makeTaggedRef(prev_fdvarptr), th);
      OZ_addSuspension(makeTaggedRef(last_fdvarptr), th);
    }
#endif
    return TRUE;
  case 3:
    s += 1;
#ifdef FDBISTUCK
    am.addSuspendVarList(varptr);
#else 
    OZ_addSuspension(makeTaggedRef(varptr), OZ_makeSuspension(f, x, a));
#endif
    return TRUE;
  case 4:
    s += 1;
#ifdef FDBISTUCK
    am.addSuspendVarList(varptr);
#else 
    OZ_addSuspension(makeTaggedRef(varptr), OZ_makeSuspension(f, x, a));
#endif
    return TRUE;
  case 5:
    return FALSE;
  default:
    error("Unexpected fds_found (0x%x).", fds_found);
    return FALSE;
  }
}


void BIfdHeadManager::addResSusp(int i, Suspension * susp, FDPropState target)
{
  DebugCheck(i < 0 || i >= curr_num_of_items, error("index overflow"));

  pm_term_type tag = bifdhm_vartag[i];

  if (tag == pm_singl) {
    return;
  } else if (tag == pm_fd) {
    addSuspFDVar(bifdhm_var[i], new SuspList(susp), target);
    if (! am.isLocalCVar(bifdhm_var[i])) global_vars += 1;
  } else if (tag == pm_bool) {
    addSuspBoolVar(bifdhm_var[i], new SuspList(susp));
    if (! am.isLocalCVar(bifdhm_var[i])) global_vars += 1;
  } else if (tag == pm_uvar) {
    if (bifdhm_var[i] != *bifdhm_varptr[i]) return;
    if (am.isLocalUVar(bifdhm_var[i])) {
      TaggedRef * taggedfdvar = newTaggedCVar(new GenFDVariable());
      addSuspFDVar(*taggedfdvar, new SuspList(susp), target);
      doBind(bifdhm_varptr[i], makeTaggedRef(taggedfdvar));
    } else {
      global_vars += 1;
      addSuspUVar(bifdhm_varptr[i], new SuspList(susp));
    } 
  } else {
    Assert(tag == pm_svar);
    
    if (bifdhm_var[i] != *bifdhm_varptr[i]) return;
    if (am.isLocalSVar(bifdhm_var[i])) {
      GenFDVariable * fdvar = new GenFDVariable();
      TaggedRef * taggedfdvar = newTaggedCVar(fdvar);
      am.checkSuspensionList(bifdhm_var[i]);
      fdvar->setSuspList(tagged2SVar(bifdhm_var[i])->getSuspList());
      addSuspFDVar(*taggedfdvar, new SuspList(susp), target);
      doBind(bifdhm_varptr[i], makeTaggedRef(taggedfdvar));
    } else {
      global_vars += 1;
      addSuspSVar(bifdhm_var[i], new SuspList(susp));
    } 
  } 
}


//-----------------------------------------------------------------------------
// Simplification Function
// - Simplifies a equation, so that are variables are pairwise distinct
//   and no coefficient nor variables are zero. Returns 1 if all
//   coefficients are either  1 or -1 otherwise 0.

// functions used by BIfdHeadManager::simplifyHead and
// BIfdBodyManager::simplifyBody
const int taggedIndex = 0x00000008; // produces GCTAG 8 + 5 = 13
inline TaggedRef makeTaggedIndex(TaggedRef t) {return (t | taggedIndex);}
inline TaggedRef getIndex(TaggedRef t) {return (t & ~taggedIndex);}
inline Bool isTaggedIndex(TaggedRef t) {return (t & taggedIndex);}

int BIfdHeadManager::simplifyHead(int ts, STuple &a, STuple &x)
{
  int first_int_index = -1;
  int int_sum = 0;

  // 1st pass: mark first occ of a var and sum up coeffs of further occs 
  for (int i = 0; i < ts; i++) {
    Assert(bifdhm_vartag[i] == pm_fd || 
	   bifdhm_vartag[i] == pm_bool || 
	   bifdhm_vartag[i] == pm_singl);

    if (isAnyVar(bifdhm_var[i])) {
      if (! isTaggedIndex(*bifdhm_varptr[i])) {
	*bifdhm_varptr[i] = makeTaggedIndex(i);
      } else {
	int ind = getIndex(*bifdhm_varptr[i]);
	a[ind] = newSmallInt(bifdhm_coeff[ind] += bifdhm_coeff[i]);
	bifdhm_var[i] = 0;
      }
    } else {
      Assert(bifdhm_vartag[i] == pm_singl);

      if (first_int_index == -1) 
	first_int_index = i;
      int_sum += (smallIntValue(bifdhm_var[i]) * bifdhm_coeff[i]);
      bifdhm_var[i] = 0;
    }
  }
  
  // 2nd pass: collect integer values into one field, whereby
  // a[first_int_index] is preferably 1 or -1
  if (first_int_index != -1) {
    if (int_sum < 0) {
      x[first_int_index] = bifdhm_var[first_int_index] = newSmallInt(-int_sum);
      a[first_int_index] = newSmallInt(bifdhm_coeff[first_int_index] = -1);
    } else {
      x[first_int_index] = bifdhm_var[first_int_index] = newSmallInt(int_sum);
      a[first_int_index] = newSmallInt(bifdhm_coeff[first_int_index] = 1);
    }
    bifdhm_varptr[first_int_index] = NULL;
    bifdhm_vartag[first_int_index] = pm_singl;
  }
  
  // 3rd pass: undo marks and compress vector
  int from, to;
  for (from = 0, to = 0; from < ts; from += 1) {
    TaggedRef var_from = bifdhm_var[from];
    
    if (var_from == 0) continue;

    TaggedRef * varptr_from = bifdhm_varptr[from];

    if (isAnyVar(var_from) && isTaggedIndex(*varptr_from))
      *varptr_from = var_from;

    int coeffs_from = bifdhm_coeff[from];

    if (coeffs_from == 0.0 || var_from == newSmallInt(0)) continue;

    if (from != to) {
      bifdhm_coeff[to] = coeffs_from;
      bifdhm_var[to] = var_from;
      bifdhm_varptr[to] = varptr_from;
      bifdhm_vartag[to] = bifdhm_vartag[from];
      a[to] = a[from];
      x[to] = x[from];
    }
    to += 1;
  } 
  
  // adjust ts and size of tuples
  a.downSize(to);
  x.downSize(to);
  return to;
} // simplifyHead

OZ_Bool BIfdHeadManager::spawnPropagator(FDPropState t, 
					 OZ_CFun f, int a, OZ_Term * x) 
{
  addResSusps(createResSusp(f, a, x), t);

  return f(a, x);
}

OZ_Bool BIfdHeadManager::spawnPropagator(FDPropState t1, FDPropState t2, 
					 OZ_CFun f, int a, OZ_Term * x) 
{
  Suspension * s = createResSusp(f, a, x);
  addResSusp(0, s, t1);
  addResSusp(1, s, t2);

  return f(a, x);
}

OZ_Bool BIfdHeadManager::spawnPropagator(FDPropState t, 
					 OZ_CFun f, int a, OZ_Term t1, ...)
{
  OZ_Term * x = (OZ_Term *) heapMalloc(a * sizeof(OZ_Term));

  x[0] = t1;

  va_list ap;
  va_start(ap, t1);

  for (int i = 1; i < a; i += 1)
    x[i] = va_arg(ap, OZ_Term);

  va_end(ap);

  addResSusps(createResSusp(f, a, x), t);

  return f(a, x);
}

#ifdef FDBISTUCK

OZ_Bool BIfdHeadManager::suspendOnVar(OZ_CFun, int, OZ_Term *,
				      OZ_Term * t)
{
  return OZ_suspendOnVar(makeTaggedRef(t));
}

OZ_Bool BIfdHeadManager::suspendOnVar(OZ_CFun, int, OZ_Term *,
				      OZ_Term * t1, OZ_Term * t2)
{
  return OZ_suspendOnVar2(makeTaggedRef(t1), 
			  makeTaggedRef(t2));
}

OZ_Bool BIfdHeadManager::suspendOnVar(OZ_CFun, int, OZ_Term *,
				      OZ_Term * t1, OZ_Term * t2, OZ_Term * t3)
{
  return OZ_suspendOnVar3(makeTaggedRef(t1), 
			  makeTaggedRef(t2), 
			  makeTaggedRef(t3));
}

#else

OZ_Bool BIfdHeadManager::suspendOnVar(OZ_CFun f, int a, OZ_Term * x,
				      OZ_Term * t)
{
  OZ_addSuspension(makeTaggedRef(t), OZ_makeSuspension(f, x, a));
  return PROCEED;
}

OZ_Bool BIfdHeadManager::suspendOnVar(OZ_CFun f, int a, OZ_Term * x,
				      OZ_Term * t1, OZ_Term * t2)
{
  OZ_Thread th = OZ_makeSuspension(f, x, a);
  OZ_addSuspension(makeTaggedRef(t1), th);
  OZ_addSuspension(makeTaggedRef(t2), th);
  return PROCEED;
}

OZ_Bool BIfdHeadManager::suspendOnVar(OZ_CFun f, int a, OZ_Term * x,
				      OZ_Term * t1, OZ_Term * t2, OZ_Term * t3)
{
  OZ_Thread th = OZ_makeSuspension(f, x, a);
  OZ_addSuspension(makeTaggedRef(t1), th);
  OZ_addSuspension(makeTaggedRef(t2), th);
  OZ_addSuspension(makeTaggedRef(t3), th);
  return PROCEED;
}
#endif

//-----------------------------------------------------------------------------
// An OZ term describing a finite domain is either:
// (1) a positive small integer <= fd_sup
// (2) a 2 tuple of (1)
// (3) a list of (1) and/or (2)

OZ_Bool checkDomDescr(OZ_Term descr,
		      OZ_CFun cfun, OZ_Term * args, int arity,
		      int expect)
{
  TaggedRef * descr_ptr;
  TypeOfTerm descr_tag;
  
  deref(descr, descr_ptr, descr_tag);
  
  if (isNotCVar(descr_tag)) {
#ifdef FDBISTUCK
    am.addSuspendVarList(descr_ptr);
#else 
    OZ_addSuspension(makeTaggedRef(descr_ptr), 
		     OZ_makeSuspension(cfun, args, arity));
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
    OZ_addSuspension(makeTaggedRef(descr_ptr), 
		     OZ_makeSuspension(cfun, args, arity));
#endif
    return SUSPEND; // checkDomDescr
  } else if (AtomBool == descr && (expect >= 2)) { // (1)
    return PROCEED;
  } else if (isSTuple(descr_tag) && (expect >= 2)) {
    STuple &tuple = *tagged2STuple(descr);
    if (tuple.getSize() != 2) {
      return FAILED;
    }
    for (int i = 0; i < 2; i++) {
      OZ_Bool r = checkDomDescr(makeTaggedRef(&tuple[i]), cfun, args, arity, 1);
      if (r != PROCEED)
	return r;
    }
    return PROCEED;
  } else if (isNil(descr) && (expect == 3)) {
    return PROCEED;
  } else if (isLTuple(descr_tag) && (expect == 3)) {
    
    do {
      LTuple &list = *tagged2LTuple(descr);
      OZ_Bool r = checkDomDescr(makeTaggedRef(list.getRefHead()),
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

TaggedRef * BIfdBodyManager::bifdbm_var;
TaggedRefPtr * BIfdBodyManager::bifdbm_varptr;
pm_term_type * BIfdBodyManager::bifdbm_vartag;
FiniteDomainPtr * BIfdBodyManager::bifdbm_dom;
FiniteDomain * BIfdBodyManager::bifdbm_domain;
int * BIfdBodyManager::bifdbm_init_dom_size;
fdbm_var_state * BIfdBodyManager::bifdbm_var_state;
int * BIfdBodyManager::cache_from;
int * BIfdBodyManager::cache_to;

int BIfdBodyManager::curr_num_of_vars;
Bool BIfdBodyManager::vars_left;
int * BIfdBodyManager::index_offset;
int * BIfdBodyManager::index_size;
Bool BIfdBodyManager::only_local_vars;

void BIfdBodyManager::initStaticData(void) {
  bifdbm_var = static_var;
  bifdbm_varptr = static_varptr;
  bifdbm_vartag = static_vartag;
  index_offset = static_index_offset;
  index_size = static_index_size;

  bifdbm_dom = new FiniteDomainPtr[MAXFDBIARGS];
  bifdbm_domain = new FiniteDomain[MAXFDBIARGS];
  bifdbm_init_dom_size = new int[MAXFDBIARGS];
  bifdbm_var_state = new fdbm_var_state[MAXFDBIARGS];
  cache_from = new int[MAXFDBIARGS];
  cache_to = new int[MAXFDBIARGS];
}

//-----------------------------------------------------------------------------
// Member functions

void BIfdBodyManager::introduceLocal(int i, TaggedRef v)
{
  DebugCheck(i < 0 || i >= curr_num_of_vars, error("index overflow"));

  pm_term_type &vartag = bifdbm_vartag[i];
  TaggedRef  &var = bifdbm_var[i] = v;
  
  deref(var, bifdbm_varptr[i], vartag);
  Assert(vartag == pm_fd || vartag == pm_bool || 
	 vartag == pm_singl || vartag == pm_literal);

  bifdbm_var_state[i] = fdbm_local; 
  if (vartag == pm_singl) {
    bifdbm_init_dom_size[i]=bifdbm_domain[i].setSingleton(smallIntValue(var));
    bifdbm_dom[i] = &bifdbm_domain[i];
  } else if (vartag == pm_bool) {
    bifdbm_init_dom_size[i] = bifdbm_domain[i].setBool();
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
  vars_left = FALSE;

  for (int i = from; i < to; i += 1) {
    Assert(bifdbm_vartag[i] == pm_fd || 
	   bifdbm_vartag[i] == pm_bool || 
	   bifdbm_vartag[i] == pm_singl);

    if (bifdbm_var_state[i] == fdbm_speculative)
      continue;

    if (bifdbm_vartag[i] == pm_singl || isSmallInt(*bifdbm_varptr[i])) {
      continue;
    } else if (!isTouched(i)) {
      vars_left = TRUE;
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
	vars_left = TRUE;
      } else {
	tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_bounds);
	vars_left = TRUE;
      }
    }
  }
} 


void BIfdBodyManager::_introduce(int i, TaggedRef v)
{
  DebugCheck(i < 0 || i >= curr_num_of_vars, error("index overflow"));

  pm_term_type vtag;
  TaggedRef *vptr;

  deref(v, vptr, vtag);

  if (vtag == pm_singl) {
    bifdbm_init_dom_size[i] = bifdbm_domain[i].setSingleton(smallIntValue(v));
    bifdbm_dom[i] = &bifdbm_domain[i];
    bifdbm_var_state[i] = fdbm_local;
  } else if (vtag == pm_bool) {
    bifdbm_init_dom_size[i] = bifdbm_domain[i].setBool();
    bifdbm_dom[i] = &bifdbm_domain[i];
    bifdbm_var_state[i] = (am.isLocalCVar(v) ? fdbm_local : fdbm_global);
  } else if (vtag == pm_fd) {
    GenFDVariable * fdvar = tagged2GenFDVar(v);
    Bool var_state = bifdbm_var_state[i] = (am.isLocalCVar(v) ? fdbm_local : fdbm_global);
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
    bifdbm_domain[i].setFull();
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

// returns FAILED for incompatible values
// returns SUSPEND for UVAR and SVAR
OZ_Bool BIfdBodyManager::checkAndIntroduce(int i, TaggedRef v)
{
  DebugCheck(i < 0 || i >= curr_num_of_vars, error("index overflow"));

  pm_term_type vtag;
  TaggedRef *vptr;
  
  deref(v, vptr, vtag);
  
  if (vtag == pm_singl) {
    int i_val = smallIntValue(v);
    if (i_val >= 0) {
      bifdbm_init_dom_size[i] = bifdbm_domain[i].setSingleton(i_val);
      bifdbm_dom[i] = &bifdbm_domain[i];
      bifdbm_var_state[i] = fdbm_local;
    } else {
      warning("Expected positive small integer.");;
      return FAILED;
    }
  } else if (vtag == pm_bool) {
    bifdbm_init_dom_size[i] = bifdbm_domain[i].setBool();
    bifdbm_dom[i] = &bifdbm_domain[i];
    bifdbm_var_state[i] = (am.isLocalCVar(v) ? fdbm_local : fdbm_global);
  } else if (vtag == pm_fd) {
    GenFDVariable * fdvar = tagged2GenFDVar(v);
    fdbm_var_state var_state = bifdbm_var_state[i] = (am.isLocalCVar(v) ? fdbm_local : fdbm_global);
    bifdbm_domain[i].FiniteDomainInit(NULL);
    if (var_state == fdbm_local) {
      bifdbm_dom[i] = &fdvar->getDom();
    } else {
      bifdbm_domain[i] = fdvar->getDom();
      bifdbm_dom[i] = &bifdbm_domain[i];
    }
    bifdbm_init_dom_size[i] = bifdbm_dom[i]->getSize();
    saveDomainOnTopLevel(i);
    Assert(bifdbm_init_dom_size[i] > 1 && *bifdbm_dom[i] != fd_bool);
  } else if (vtag == pm_uvar || vtag == pm_svar) {
    return SUSPEND; // checkAndIntroduce 
  } else {
    return FAILED;
  }
  bifdbm_var[i] = v;
  bifdbm_varptr[i] = vptr;
  bifdbm_vartag[i] = vtag;
  return PROCEED;
}  


void BIfdBodyManager::processFromTo(int from, int to)
{
  vars_left = FALSE;
  
  for (int i = from; i < to; i += 1) {

    if (bifdbm_var_state[i] == fdbm_speculative)
      continue;
    
    pm_term_type vartag = bifdbm_vartag[i];

    if (vartag == pm_singl || isSmallInt(*bifdbm_varptr[i])) {
      continue;
    } else if (! isTouched(i)) {
      vars_left = TRUE;
    } else if (vartag == pm_fd) {
      if (*bifdbm_dom[i] == fd_singleton) {
	if (bifdbm_var_state[i] == fdbm_local) {
	  tagged2GenFDVar(bifdbm_var[i])->
	    becomesSmallIntAndPropagate(bifdbm_varptr[i]);
	} else {
	  tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_det);
	  am.doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i],
			    newSmallInt(bifdbm_dom[i]->singl()));
	}
      } else if (*bifdbm_dom[i] == fd_bool) {

	if (bifdbm_var_state[i] == fdbm_local) {
	  tagged2GenFDVar(bifdbm_var[i])->
	    becomesBoolVarAndPropagate(bifdbm_varptr[i]);
	} else {
	  tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_bounds);
	  GenBoolVariable * newboolvar = new GenBoolVariable();
	  TaggedRef * newtaggedboolvar = newTaggedCVar(newboolvar);
	  am.doBindAndTrailAndIP(bifdbm_var[i], bifdbm_varptr[i],
				 makeTaggedRef(newtaggedboolvar),
				 newboolvar, tagged2GenBoolVar(bifdbm_var[i]),
				 NO);
	}

      } else {

	tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_bounds);
	if (bifdbm_var_state[i] == fdbm_global) {
	  GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[i]);
	  TaggedRef * newtaggedfdvar = newTaggedCVar(newfdvar);
	  am.doBindAndTrailAndIP(bifdbm_var[i], bifdbm_varptr[i],
				 makeTaggedRef(newtaggedfdvar),
				 newfdvar, tagged2GenFDVar(bifdbm_var[i]), NO);
	} 
	
	vars_left = TRUE;
      }

    } else if (vartag == pm_bool) {
      Assert(*bifdbm_dom[i] == fd_singleton); 

      if (bifdbm_var_state[i] == fdbm_local) {
	tagged2GenBoolVar(bifdbm_var[i])->
	  becomesSmallIntAndPropagate(bifdbm_varptr[i], *bifdbm_dom[i]);
      } else {
	tagged2GenBoolVar(bifdbm_var[i])->propagate(bifdbm_var[i]);
	am.doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i],
			  newSmallInt(bifdbm_dom[i]->singl()));
      }
    } else {
      Assert(vartag == pm_svar && bifdbm_var_state[i] == fdbm_global);

      if (*bifdbm_dom[i] == fd_singleton) {
	TaggedRef newsmallint = newSmallInt(bifdbm_dom[i]->singl());
	am.checkSuspensionList(bifdbm_var[i]);
	am.doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i], newsmallint);
      } else if (*bifdbm_dom[i] == fd_bool) {
	GenBoolVariable * newboolvar = new GenBoolVariable();
	TaggedRef * newtaggedboolvar = newTaggedCVar(newboolvar);
	am.checkSuspensionList(bifdbm_var[i]);
	am.doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i],
			  makeTaggedRef(newtaggedboolvar));
	vars_left = TRUE;
      } else {
	GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[i]);
	TaggedRef * newtaggedfdvar = newTaggedCVar(newfdvar);
	am.checkSuspensionList(bifdbm_var[i]);
	am.doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i],
			  makeTaggedRef(newtaggedfdvar));
	vars_left = TRUE;
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

  Suspension * susp = NULL;

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
			  newSmallInt(bifdbm_dom[0]->singl()));
      }
    } else if (*bifdbm_dom[0] == fd_bool) {
      tagged2GenFDVar(bifdbm_var[0])->propagate(bifdbm_var[0], fd_bounds);
      if (bifdbm_var_state[0] == fdbm_global) {
	GenBoolVariable * newboolvar = new GenBoolVariable();
	TaggedRef * newtaggedboolvar = newTaggedCVar(newboolvar);
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
	TaggedRef * newtaggedfdvar = newTaggedCVar(newfdvar);
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
			newSmallInt(bifdbm_dom[0]->singl()));
    }
  } else {
    Assert(bifdbm_var_state[0] == fdbm_global && vartag == pm_svar);
    
    if (*bifdbm_dom[0] == fd_singleton) {
      TaggedRef newsmallint = newSmallInt(bifdbm_dom[0]->singl());
      am.checkSuspensionList(bifdbm_var[0]);
      am.doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0], newsmallint);
    } else if (*bifdbm_dom[0] == fd_bool) {
      GenBoolVariable * newboolvar = new GenBoolVariable();
      TaggedRef * newtaggedboolvar = newTaggedCVar(newboolvar);
      am.checkSuspensionList(bifdbm_var[0]);
      am.doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0],
			makeTaggedRef(newtaggedboolvar));
      vars_left = TRUE;
    } else {
      GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[0]);
      TaggedRef * newtaggedfdvar = newTaggedCVar(newfdvar);
      am.checkSuspensionList(bifdbm_var[0]);
      am.doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0], 
			makeTaggedRef(newtaggedfdvar));
      vars_left = TRUE;
    }
    
  }
} // BIfdBodyManager::processNonRes

Bool BIfdBodyManager::introduce(TaggedRef v)
{
  pm_term_type vtag;
  TaggedRef *vptr;

  deref(v, vptr, vtag);

  if (vtag == pm_singl) {
    int i_val = smallIntValue(v);
    if (i_val >= 0) {
      bifdbm_init_dom_size[0] = bifdbm_domain[0].setSingleton(i_val);
      bifdbm_dom[0] = &bifdbm_domain[0];
      bifdbm_var_state[0] = fdbm_local;
      bifdbm_var[0] = v;
      bifdbm_varptr[0] = vptr;
      bifdbm_vartag[0] = vtag;
   } else {
      return FALSE;
    }
  } else if (vtag == pm_bool) {
    bifdbm_init_dom_size[0] = bifdbm_domain[0].setBool();
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
    bifdbm_var_state[0] = (am.isLocalSVar(v) ? fdbm_local : fdbm_global);
    if (bifdbm_var_state[0] == fdbm_local) {
      GenFDVariable * fdvar = new GenFDVariable();
      TaggedRef * taggedfdvar = newTaggedCVar(fdvar);
      bifdbm_dom[0] = &fdvar->getDom();
      bifdbm_init_dom_size[0] = bifdbm_dom[0]->getSize();
      am.checkSuspensionList(v);
      fdvar->setSuspList(tagged2SVar(v)->getSuspList());
      doBind(vptr, makeTaggedRef(taggedfdvar));
      bifdbm_var[0] = *(bifdbm_varptr[0] = taggedfdvar);
      bifdbm_vartag[0] = pm_fd;
    } else {
      bifdbm_domain[0].setFull();
      bifdbm_dom[0] = &bifdbm_domain[0];
      bifdbm_var[0] = v;
      bifdbm_varptr[0] = vptr;
      bifdbm_vartag[0] = vtag;
     }
  } else if (vtag == pm_uvar) {
    bifdbm_var_state[0] = (am.isLocalUVar(v)? fdbm_local : fdbm_global);
    if (bifdbm_var_state[0] == fdbm_local) {
      GenFDVariable * fdvar = new GenFDVariable();
      TaggedRef * taggedfdvar = newTaggedCVar(fdvar);
      bifdbm_dom[0] = &fdvar->getDom();
      bifdbm_init_dom_size[0] = bifdbm_dom[0]->getSize();
      doBind(vptr, makeTaggedRef(taggedfdvar));
      bifdbm_var[0] = *(bifdbm_varptr[0] = taggedfdvar);
      bifdbm_vartag[0] = pm_fd;
    } else {
      bifdbm_domain[0].setFull();
      bifdbm_dom[0] = &bifdbm_domain[0];
      *vptr = bifdbm_var[0] = makeTaggedSVar(new SVariable(tagged2VarHome(v)));
      bifdbm_varptr[0] = vptr;
      bifdbm_vartag[0] = pm_svar;
    }

  } else {
    return FALSE;
  }

  return TRUE;
} // BIfdBodyManager::introduce


int BIfdBodyManager::simplifyBody(int ts, STuple &a, STuple &x,
				  Bool sign_bits[], double coeffs[])
{
  // 1st pass: mark first occ of a var and sum up coeffs of further occs 
  for (int i = 0; i < ts; i++) {
    Assert(bifdbm_vartag[i] == pm_fd || 
	   bifdbm_vartag[i] == pm_bool || 
	   bifdbm_vartag[i] == pm_singl);

    if (isAnyVar(bifdbm_var[i]))
      if (! isTaggedIndex(*bifdbm_varptr[i])) {
	*bifdbm_varptr[i] = makeTaggedIndex(i);
      } else {
	int ind = getIndex(*bifdbm_varptr[i]);
	a[ind] = newSmallInt(int(coeffs[ind] += coeffs[i]));
	bifdbm_var[i] = 0;
      }
  }
  
  // 2nd pass: undo marks and compress vector
  int from, to;
  for (from = 0, to = 0; from < ts; from += 1) {
    TaggedRef var_from = bifdbm_var[from];
    
    if (var_from == 0) continue;

    TaggedRef * varptr_from = bifdbm_varptr[from];

    if (isAnyVar(var_from) && isTaggedIndex(*varptr_from))
      *varptr_from = var_from;

    double coeffs_from = coeffs[from];

    if (coeffs_from == 0.0 || var_from == newSmallInt(0)) continue;

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


void BIfdBodyManager::_propagate_unify_cd(int clauses, int variables,
					  STuple &vp)
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
	  // check void variables
	  TaggedRef l_var, r_var;
	  
	  if (bifdbm_vartag[idx_vp(c, fs)] == pm_literal) {
	    // convert void variable to heap variable
	    GenFDVariable * fv = new GenFDVariable(*bifdbm_dom[idx_vp(c, fs)]);
	    bifdbm_varptr[idx_vp(c, fs)] = newTaggedCVar(fv);
	    l_var = makeTaggedRef(bifdbm_varptr[idx_vp(c, v)]);
	    bifdbm_vartag[idx_vp(c, fs)] = pm_fd;
	    // update arguments
	    STuple &vp_c = *tagged2STuple(deref(vp[c]));
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
	    STuple &vp_c = *tagged2STuple(deref(vp[c]));
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

	  if (OZ_unify(l_var, r_var) == FALSE) {
	    restore();
	    *bifdbm_dom[idx_b(c)] &= 0;
	    goto escape;
	  }

	  restore();
	}
      }
  }
  
 escape:

  // (re)introduce renamed variable (propagation done by unify) after
  // propagation of equalities has finished
  for (c = clauses; c--; )
    for (v = variables; v--; )
      if (bifdbm_varptr[idx_vp(c, v)])
	introduceLocal(idx_vp(c, v), makeTaggedRef(bifdbm_varptr[idx_vp(c, v)]));
  
  // 2nd pass: undo marks
  for (v = 0; v < variables; v++) {
    if (isAnyVar(bifdbm_var[idx_v(v)]))
      *bifdbm_varptr[idx_v(v)] = bifdbm_var[idx_v(v)];
  }
}


Bool BIfdBodyManager::_unifiedVars(void)
{
  Bool ret = FALSE;

  // 1st pass: mark occ of var and break if you find already touched var
  int i;
  for (i = 0; i < curr_num_of_vars; i += 1)
    if (isAnyVar(bifdbm_var[i]))
      if (isTaggedIndex(*bifdbm_varptr[i])) {
	ret = TRUE;
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

OZ_Bool BIfdBodyManager::replacePropagator(OZ_CFun f, int a, OZ_Term * x) 
{
  RefsArray xregs = allocateRefsArray(a, FALSE);
  
  for (int i = 0; i < a; i += 1)
    xregs[i] = x[i];

  FDcurrentTaskSusp->getCCont()->setCFuncRegs(f, xregs);
  return f(a, xregs);
}

OZ_Bool BIfdBodyManager::replacePropagator(OZ_CFun f, int a, OZ_Term t1, ...)
{
  RefsArray xregs = allocateRefsArray(a, FALSE);

  xregs[0] = t1;

  va_list ap;
  va_start(ap, t1);

  for (int i = 1; i < a; i += 1)
    xregs[i] = va_arg(ap, OZ_Term);

  va_end(ap);

  FDcurrentTaskSusp->getCCont()->setCFuncRegs(f, xregs);
  return f(a, xregs);
}

OZ_Bool BIfdBodyManager::replacePropagator(OZ_Term a, OZ_Term b)
{
  killPropagatedCurrentTaskSusp();
  return OZ_unify(a, b);
}

//-----------------------------------------------------------------------------
// Introduce FD Built-ins to the Emulator

void BIinitFD(void)
{
// fdprofil.cc
  BIadd("fdReset", 0, BIfdReset);
  BIadd("fdDiscard", 0, BIfdDiscard);
  BIadd("fdGetNext", 1, BIfdGetNext);
  BIadd("fdPrint", 0, BIfdPrint);
  BIadd("fdTotalAverage", 0, BIfdTotalAverage);

// fdcore.cc
  BIadd("fdIs", 1, BIfdIs);
  BIadd("fdIsVar", 1, BIisFdVar);
  BIadd("fdGetLimits", 2, BIgetFDLimits);
  BIadd("fdGetMin", 2, BIfdMin);
  BIadd("fdGetMax", 2, BIfdMax);
  BIadd("fdGetDom", 2, BIfdGetAsList);
  BIadd("fdGetCard", 2, BIfdGetCardinality);
  BIadd("fdNextTo", 3, BIfdNextTo);

  BIadd("fdPutLe", 2, BIfdPutLe);
  BIadd("fdPutGe", 2, BIfdPutGe);
  BIadd("fdPutList", 3, BIfdPutList);
  BIadd("fdPutInterval", 3, BIfdPutInterval);
  BIadd("fdPutNot", 2, BIfdPutNot);
  
// fdrel.cc
  BIadd("fdMinimum", 3, BIfdMinimum);
  BIadd("fdMinimum_body", 3, BIfdMinimum_body);
  BIadd("fdMaximum", 3, BIfdMaximum);
  BIadd("fdMaximum_body", 3, BIfdMaximum_body);
  BIadd("fdUnion", 3, BIfdUnion);
  BIadd("fdUnion_body", 3, BIfdUnion_body);
  BIadd("fdIntersection", 3, BIfdIntersection);
  BIadd("fdIntersection_body", 3, BIfdIntersection_body);
  BIadd("fdSubsume_body", 3, BIfdSubsume_body);
  BIadd("fdLessEqOff", 3, BIfdLessEqOff);
  BIadd("fdLessEqOff_body", 3, BIfdLessEqOff_body);
  BIadd("fdNotEqEnt", 2, BIfdNotEqEnt);
  BIadd("fdNotEqEnt_body", 2, BIfdNotEqEnt_body);
  BIadd("fdNotEq", 2, BIfdNotEq);
  BIadd("fdNotEq_body", 2, BIfdNotEq_body);
  BIadd("fdNotEqOffEnt", 3, BIfdNotEqOffEnt);
  BIadd("fdNotEqOffEnt_body", 3, BIfdNotEqOffEnt_body);
  BIadd("fdNotEqOff", 3, BIfdNotEqOff);
  BIadd("fdNotEqOff_body", 3, BIfdNotEqOff_body);
  BIadd("fdAllDifferent", 1, BIfdAllDifferent);
  BIadd("fdAllDifferent_body", 1, BIfdAllDifferent_body);
  BIadd("fdDistinctOffset", 2, BIfdDistinctOffset);
  BIadd("fdDistinctOffset_body", 2, BIfdDistinctOffset_body);
  
// fdbool.cc
  BIadd("fdAnd", 3, BIfdAnd);
  BIadd("fdAnd_body", 3, BIfdAnd_body);
  BIadd("fdOr", 3, BIfdOr);
  BIadd("fdOr_body", 3, BIfdOr_body);
  BIadd("fdNot", 2, BIfdNot);
  BIadd("fdNot_body", 2, BIfdNot_body);
  BIadd("fdXor", 3, BIfdXor);
  BIadd("fdXor_body", 3, BIfdXor_body);
  BIadd("fdEquiv", 3, BIfdEquiv);
  BIadd("fdEquiv_body", 3, BIfdEquiv_body);
  BIadd("fdImpl", 3, BIfdImpl);
  BIadd("fdImpl_body", 3, BIfdImpl_body);
  
// fdarith.cc
  BIadd("fdPlus", 3, BIfdPlus);
  BIadd("fdPlus_body", 3, BIfdPlus_body);
  BIadd("fdTwice_body", 2, BIfdTwice_body);
  BIadd("fdMinus", 3, BIfdMinus);
  BIadd("fdMinus_body", 3, BIfdMinus_body);
  BIadd("fdMult", 3, BIfdMult);
  BIadd("fdMult_body", 3, BIfdMult_body);
  BIadd("fdSquare_body", 2, BIfdSquare_body);
  BIadd("fdDiv", 3, BIfdDiv);
  BIadd("fdDiv_body", 3, BIfdDiv_body);
  BIadd("fdDivInterval", 3, BIfdDivInterval);
  BIadd("fdDivInterval_body", 3, BIfdDivInterval_body);
  BIadd("fdMod", 3, BIfdMod);
  BIadd("fdMod_body", 3, BIfdMod_body);
  BIadd("fdModInterval", 3, BIfdModInterval);
  BIadd("fdModInterval_body", 3, BIfdModInterval_body);
  BIadd("fdPlus_rel", 3, BIfdPlus_rel);
  BIadd("fdMult_rel", 3, BIfdMult_rel);
  
// fdgeneric.cc
  BIadd("fdGenLinEq", 3, BIfdGenLinEq);
  BIadd("fdGenLinEq_body", 3, BIfdGenLinEq_body);
  BIadd("fdGenNonLinEq", 3, BIfdGenNonLinEq);
  BIadd("fdGenNonLinEq1", 3, BIfdGenNonLinEq1);
  BIadd("fdGenNonLinEq_body", 3, BIfdGenNonLinEq_body);
  BIadd("fdGenLinNotEq", 3, BIfdGenLinNotEq);
  BIadd("fdGenLinNotEq_body", 3, BIfdGenLinNotEq_body);
  BIadd("fdGenNonLinNotEq", 3, BIfdGenNonLinNotEq);
  BIadd("fdGenNonLinNotEq_body", 3, BIfdGenNonLinNotEq_body);
  BIadd("fdGenLinLessEq", 3, BIfdGenLinLessEq);
  BIadd("fdGenLinLessEq_body", 3, BIfdGenLinLessEq_body);
  BIadd("fdGenNonLinLessEq", 3, BIfdGenNonLinLessEq);
  BIadd("fdGenNonLinLessEq1", 3, BIfdGenNonLinLessEq1);
  BIadd("fdGenNonLinLessEq_body", 3, BIfdGenNonLinLessEq_body);
  
// fdcount.cc
  BIadd("fdElement", 3, BIfdElement);
  BIadd("fdElement_body", 3, BIfdElement_body);
  BIadd("fdAtMost", 3, BIfdAtMost);
  BIadd("fdAtMost_body", 3, BIfdAtMost_body);
  BIadd("fdAtLeast", 3, BIfdAtLeast);
  BIadd("fdAtLeast_body", 3, BIfdAtLeast_body);
  BIadd("fdCount", 3, BIfdCount);
  BIadd("fdCount_body", 3, BIfdCount_body);

// fdcard.cc
  BIadd("fdCardBIBin", 2, BIfdCardBIBin); 
  BIadd("fdCardBIBin_body", 2, BIfdCardBIBin_body);
  BIadd("fdCardNestableBI", 4, BIfdCardNestableBI); 
  BIadd("fdCardNestableBI_body", 4, BIfdCardNestableBI_body);
  BIadd("fdCardNestableBIBin", 3, BIfdCardNestableBIBin); 
  BIadd("fdCardNestableBIBin_body", 3, BIfdCardNestableBIBin_body);
  BIadd("fdInB", 3, BIfdInB); 
  BIadd("fdInB_body", 3, BIfdInB_body);
  BIadd("fdNotInB", 3, BIfdNotInB); 
  BIadd("fdNotInB_body", 3, BIfdNotInB_body);
  BIadd("fdGenLinEqB", 4, BIfdGenLinEqB);
  BIadd("fdGenNonLinEqB", 4, BIfdGenNonLinEqB); 
  BIadd("fdGenLinEqB_body", 4, BIfdGenLinEqB_body);
  BIadd("fdGenLinNotEqB", 4, BIfdGenLinNotEqB); 
  BIadd("fdGenNonLinNotEqB", 4, BIfdGenNonLinNotEqB); 
  BIadd("fdGenLinNotEqB_body", 4, BIfdGenLinNotEqB_body);
  BIadd("fdGenLinLessEqB", 4, BIfdGenLinLessEqB);
  BIadd("fdGenLinLessEqB_body", 4, BIfdGenLinLessEqB_body);
  BIadd("fdGenNonLinLessEqB", 4, BIfdGenNonLinLessEqB); 

// fdcd.cc
  BIadd("fdConstrDisjSetUp", 4, BIfdConstrDisjSetUp);
  BIadd("fdConstrDisj", 3, BIfdConstrDisj);
  BIadd("fdConstrDisj_body", 3, BIfdConstrDisj_body);
  
  BIadd("fdGenLinEqCD", 4, BIfdGenLinEqCD);
  BIadd("fdGenLinEqCD_body", 4, BIfdGenLinEqCD_body);
  BIadd("fdGenNonLinEqCD", 4, BIfdGenNonLinEqCD);
  BIadd("fdGenLinNotEqCD", 4, BIfdGenLinNotEqCD);
  BIadd("fdGenLinNotEqCD_body", 4, BIfdGenLinNotEqCD_body);
  BIadd("fdGenNonLinNotEqCD", 4, BIfdGenNonLinNotEqCD);
  BIadd("fdGenLinLessEqCD", 4, BIfdGenLinLessEqCD);
  BIadd("fdGenLinLessEqCD_body", 4, BIfdGenLinLessEqCD_body);
  BIadd("fdGenNonLinLessEqCD", 4, BIfdGenNonLinLessEqCD);

  BIadd("fdPlusCD", 4, BIfdPlusCD_rel);
  BIadd("fdPlusCD_body", 4, BIfdPlusCD_rel_body);
  BIadd("fdMultCD", 4, BIfdMultCD_rel);
  BIadd("fdMultCD_body", 4, BIfdMultCD_rel_body);

  BIadd("fdLessEqOffCD", 4, BIfdLessEqOffCD);
  BIadd("fdLessEqOffCD_body", 4, BIfdLessEqOffCD_body);
  BIadd("fdNotEqCD", 3, BIfdNotEqCD);
  BIadd("fdNotEqCD_body", 3, BIfdNotEqCD_body);
  BIadd("fdNotEqOffCD", 4, BIfdNotEqOffCD);
  BIadd("fdNotEqOffCD_body", 4, BIfdNotEqOffCD_body);

  BIadd("fdPutLeCD", 3, BIfdPutLeCD);
  BIadd("fdPutGeCD", 3, BIfdPutGeCD);
  BIadd("fdPutListCD", 4, BIfdPutListCD);
  BIadd("fdPutIntervalCD", 4, BIfdPutIntervalCD);
  BIadd("fdPutNotCD", 3, BIfdPutNotCD);

// fdwatch.cc
  BIadd("fdWatchDom1", 2, BIfdWatchDom1);
  BIadd("fdWatchDom2", 4, BIfdWatchDom2);
  BIadd("fdWatchDom3", 6, BIfdWatchDom3);

  BIadd("fdWatchBounds1", 3, BIfdWatchBounds1);
  BIadd("fdWatchBounds2", 6, BIfdWatchBounds2);
  BIadd("fdWatchBounds3", 9, BIfdWatchBounds3);

// fdmisc.cc
  BIadd("fdCardSched", 4, BIfdCardSched);
  BIadd("fdCardSched_body", 4, BIfdCardSched_body);
  BIadd("fdCardSchedControl", 5, BIfdCardSchedControl);
  BIadd("fdCardSchedControl_body", 5, BIfdCardSchedControl_body);
  BIadd("fdCDSched", 4, BIfdCDSched);
  BIadd("fdCDSched_body", 4, BIfdCDSched_body);
  BIadd("fdCDSchedControl", 5, BIfdCDSchedControl);
  BIadd("fdCDSchedControl_body", 5, BIfdCDSchedControl_body);
  BIadd("fdNoOverlap", 6, BIfdNoOverlap);
  BIadd("fdNoOverlap_body", 6, BIfdNoOverlap_body);
  BIadd("fdGenLinEqKillB", 4, BIfdGenLinEqKillB); 
  BIadd("fdGenLinEqKillB_body", 4, BIfdGenLinEqKillB_body);
  BIadd("fdGenLinLessEqKillB", 4, BIfdGenLinLessEqKillB); 
  BIadd("fdGenLinLessEqKillB_body", 4, BIfdGenLinLessEqKillB_body);
  BIadd("fdCardBIKill", 4, BIfdCardBIKill); 
  BIadd("fdCardBIKill_body", 4, BIfdCardBIKill_body);
  BIadd("fdInKillB", 3, BIfdInKillB); 
  BIadd("fdInKillB_body", 3, BIfdInKillB_body);
  BIadd("fdNotInKillB", 3, BIfdNotInKillB); 
  BIadd("fdNotInKillB_body", 3, BIfdNotInKillB_body);
  BIadd("fdCopyDomain", 2, BIfdCopyDomain);
  BIadd("fdDivDomCons", 3, BIfdDivIntervalCons);
  BIadd("getCopyStat", 1, BIgetCopyStat);
}


