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
#include "unify.hh"
#include "fdprofil.hh"

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
TypeOfTerm static_vartag[MAXFDBIARGS];
float static_coeff_float[MAXFDBIARGS];
int static_coeff_int[MAXFDBIARGS];
Bool static_sign_bit[MAXFDBIARGS];

Bool static_bool_a[MAXFDBIARGS];
Bool static_bool_b[MAXFDBIARGS];
int static_int_a[MAXFDBIARGS];
int static_int_b[MAXFDBIARGS];
float static_float_a[MAXFDBIARGS];
float static_float_b[MAXFDBIARGS];

int static_index_offset[MAXFDBIARGS];
int static_index_size[MAXFDBIARGS];

FiniteDomain __CDVoidFiniteDomain(fd_full);

//-----------------------------------------------------------------------------
// Member functions

TaggedRef * BIfdHeadManager::bifdhm_var;
TaggedRefPtr * BIfdHeadManager::bifdhm_varptr;
TypeOfTerm * BIfdHeadManager::bifdhm_vartag;
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
                                   OZ_CFun func, RefsArray xregs, int arity)
{
  DebugCheck(i < 0 || i >= curr_num_of_items, error("index overflow"));

  if (! isSTuple(deref(tagged_xtc, bifdhm_varptr[i], bifdhm_vartag[i]))) {
    bifdhm_var[i] = tagged_xtc;
    return TRUE;
  }

  STuple &xtc = *tagged2STuple(tagged_xtc);
  const int ts = xtc.getSize();
  TaggedRef prev_fdvar, last_fdvar;
  TaggedRefPtr last_fdvarptr = NULL;
  TaggedRef var;
  TaggedRefPtr varptr;
  TypeOfTerm vartag;
  long prod = 1;
  Suspension * susp;

  int j, fds_found;
  for (j = ts, fds_found = 0; j-- && (fds_found < 2); ) {
    var = makeTaggedRef(&xtc[j]);
    deref(var, varptr, vartag);

    if (vartag == SMALLINT) {
      prod *= smallIntValue(var);
      if (prod < OzMinInt || OzMaxInt < prod) return FALSE;
    } else if (isGenFDVar(var,vartag)) {
      fds_found += 1;
      if (last_fdvarptr != NULL)
        prev_fdvar = last_fdvar;
      last_fdvar = var;
      last_fdvarptr = varptr;
    } else if (isUVar(vartag)) {
      fds_found = 3;
    } else if (isSVar(vartag)) {
      fds_found = 4;
    } else {
      fds_found = 5;
    }
  }

  switch (fds_found) {
  case 0: // no variables left
    bifdhm_vartag[i] = SMALLINT;
    xt[i] = bifdhm_var[i] = newSmallInt(1);
    bifdhm_coeff[i] *= prod;
    if (bifdhm_coeff[i] < OzMinInt || OzMaxInt < bifdhm_coeff[i]) return FALSE;
    at[i] = newSmallInt(bifdhm_coeff[i]);
    return TRUE;
  case 1: // exactly one variable left
    bifdhm_vartag[i] = CVAR;
    bifdhm_var[i] = last_fdvar;
    xt[i] = makeTaggedRef(bifdhm_varptr[i] = last_fdvarptr);
    bifdhm_coeff[i] *= prod;
    if (bifdhm_coeff[i] < OzMinInt || OzMaxInt < bifdhm_coeff[i]) return FALSE;
    at[i] = newSmallInt(bifdhm_coeff[i]);
    return TRUE;
  case 2:
    s += 1;
    susp = createNonResSusp(func, xregs, arity);
    addSuspFDVar(prev_fdvar, new SuspList(susp, NULL), fd_det);
    addSuspFDVar(last_fdvar, new SuspList(susp, NULL), fd_det);
    return TRUE;
  case 3:
    s += 1;
    susp = createNonResSusp(func, xregs, arity);
    addSuspOnlyToUVar(varptr, new CondSuspList(susp, NULL, isConstrained));
    return TRUE;
  case 4:
    s += 1;
    susp = createNonResSusp(func, xregs, arity);
    addSuspSVar(var, new CondSuspList(susp, NULL, isConstrained));
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

  TypeOfTerm tag = bifdhm_vartag[i];

  if (tag == SMALLINT) {
    return;
  } else if (tag == CVAR) {
    addSuspFDVar(bifdhm_var[i], new SuspList(susp, NULL), target);
    if (! am.isLocalCVar(bifdhm_var[i])) global_vars += 1;
  } else if (tag == UVAR) {
    if (bifdhm_var[i] != *bifdhm_varptr[i]) return;
    if (am.isLocalUVar(bifdhm_var[i])) {
      TaggedRef * taggedfdvar = newTaggedCVar(new GenFDVariable());
      addSuspFDVar(*taggedfdvar, new SuspList(susp, NULL), target);
      doBind(bifdhm_varptr[i], makeTaggedRef(taggedfdvar));
    } else {
      global_vars += 1;
      addSuspUVar(bifdhm_varptr[i], new SuspList(susp, NULL));
    }
  } else if (tag == SVAR) {
    if (bifdhm_var[i] != *bifdhm_varptr[i]) return;
    if (am.isLocalSVar(bifdhm_var[i])) {
      GenFDVariable * fdvar = new GenFDVariable();
      TaggedRef * taggedfdvar = newTaggedCVar(fdvar);
      am.checkSuspensionList(bifdhm_var[i], makeTaggedRef(taggedfdvar));
      fdvar->setSuspList(tagged2SVar(bifdhm_var[i])->getSuspList());
      addSuspFDVar(*taggedfdvar, new SuspList(susp, NULL), target);
      doBind(bifdhm_varptr[i], makeTaggedRef(taggedfdvar));
    } else {
      global_vars += 1;
      addSuspSVar(bifdhm_var[i], new SuspList(susp, NULL));
    }
  } else {
    error("Unexpected tag in addResSusp, got 0x%x.", bifdhm_vartag[i]);
  }
}


void BIfdHeadManager::addForIntSusp(int i, Suspension * susp)
{
  DebugCheck(i < 0 || i >= curr_num_of_items, error("index overflow"));

  TypeOfTerm vtag = bifdhm_vartag[i];

  if (vtag != SMALLINT)
    if (vtag == CVAR) {
      addSuspFDVar(bifdhm_var[i], new SuspList(susp, NULL), fd_det);
    } else if (vtag == UVAR) {
      addSuspOnlyToUVar(bifdhm_varptr[i], new CondSuspList(susp, NULL, isDet));
    } else if (vtag == SVAR) {
      addSuspSVar(bifdhm_var[i], new CondSuspList(susp, NULL, isDet));
    } else {
      error("Unexpected tag in addSuspForInt, got 0x%x.", vtag);
    }
}


void BIfdHeadManager::addForFDishSusp(int i, Suspension * susp)
{
  DebugCheck(i < 0 || i >= curr_num_of_items, error("index overflow"));

  TypeOfTerm vtag = bifdhm_vartag[i];

  if (!(vtag == SMALLINT || vtag == CVAR))
    if (vtag == UVAR) {
      addSuspOnlyToUVar(bifdhm_varptr[i],
                        new CondSuspList(susp, NULL, isConstrained));
    } else if (vtag == SVAR) {
      addSuspSVar(bifdhm_var[i], new CondSuspList(susp, NULL, isConstrained));
    } else {
      error("Unexpected tag in addSuspForInt, got 0x%x.", vtag);
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
  // 1st pass: mark first occ of a var and sum up coeffs of further occs
  for (int i = 0; i < ts; i++) {
    Assert(isCVar(bifdhm_vartag[i]) || isSmallInt(bifdhm_vartag[i]));
    if (isAnyVar(bifdhm_var[i]))
      if (! isTaggedIndex(*bifdhm_varptr[i])) {
        *bifdhm_varptr[i] = makeTaggedIndex(i);
      } else {
        int ind = getIndex(*bifdhm_varptr[i]);
        a[ind] = newSmallInt(bifdhm_coeff[ind] += bifdhm_coeff[i]);
        bifdhm_var[i] = 0;
      }
  }

  // 2nd pass: undo marks and compress vector
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


//-----------------------------------------------------------------------------
//                              class BIfdBodyManager
//-----------------------------------------------------------------------------
// Global data

TaggedRef * BIfdBodyManager::bifdbm_var;
TaggedRefPtr * BIfdBodyManager::bifdbm_varptr;
TypeOfTerm * BIfdBodyManager::bifdbm_vartag;
FiniteDomainPtr * BIfdBodyManager::bifdbm_dom;
FiniteDomain * BIfdBodyManager::bifdbm_domain;
int * BIfdBodyManager::bifdbm_init_dom_size;
Bool * BIfdBodyManager::bifdbm_is_local;
int * BIfdBodyManager::cache_from;
int * BIfdBodyManager::cache_to;

int BIfdBodyManager::curr_num_of_vars;
Bool BIfdBodyManager::vars_left;
Bool BIfdBodyManager::glob_vars_touched;
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
  bifdbm_is_local = new Bool[MAXFDBIARGS];
  cache_from = new int[MAXFDBIARGS];
  cache_to = new int[MAXFDBIARGS];
}

//-----------------------------------------------------------------------------
// Member functions

void BIfdBodyManager::_introduce(int i, TaggedRef v)
{
  DebugCheck(i < 0 || i >= curr_num_of_vars, error("index overflow"));

  TypeOfTerm vtag;
  TaggedRef *vptr;

  deref(v, vptr, vtag);

  if (vtag == SMALLINT) {
    int i_val = smallIntValue(v);
    if (i_val >= 0) {
      bifdbm_domain[i].setSingleton(i_val);
      bifdbm_dom[i] = &bifdbm_domain[i];
      bifdbm_is_local[i] = TRUE;
      bifdbm_init_dom_size[i] = 1;
    } else {
      error("Expected positive small integer.");;
    }
  } else if (isGenFDVar(v,vtag)) {
    GenFDVariable * fdvar = tagged2GenFDVar(v);
    Bool is_local = bifdbm_is_local[i] = am.isLocalCVar(v);
    bifdbm_domain[i].FiniteDomainInit();
    if (is_local) {
      bifdbm_dom[i] = &fdvar->getDom();
    } else {
      bifdbm_domain[i] = fdvar->getDom();
      bifdbm_dom[i] = &bifdbm_domain[i];
    }
    bifdbm_init_dom_size[i] = bifdbm_dom[i]->getSize();
  } else if (vtag == SVAR) {
    bifdbm_domain[i].setFull();
    bifdbm_dom[i] = &bifdbm_domain[i];
    bifdbm_is_local[i] = am.isLocalSVar(v);
  } else if (vtag == LITERAL) {
    bifdbm_dom[i] = &__CDVoidFiniteDomain;
    bifdbm_init_dom_size[i] = bifdbm_dom[i]->getSize();
  } else {
    error("Found unexpected term tag.");
  }
  bifdbm_var[i] = v;
  bifdbm_varptr[i] = vptr;
  bifdbm_vartag[i] = vtag;
} // BIfdBodyManager::_introduce

inline
void BIfdBodyManager::processFromTo(int from, int to)
{
  vars_left = glob_vars_touched = FALSE;

  for (int i = from; i < to; i += 1) {
    TypeOfTerm vartag = bifdbm_vartag[i];

    if (vartag == SMALLINT || isSmallInt(*bifdbm_varptr[i])) {
      continue;
    } else if (vartag == CVAR) {
      if (! isTouched(i)) {
        vars_left = TRUE;
      } else {
        if (*bifdbm_dom[i] == fd_singleton) {
          if (bifdbm_is_local[i]) {
            glob_vars_touched |= tagged2GenFDVar(bifdbm_var[i])->isTagged();
            tagged2GenFDVar(bifdbm_var[i])->
              becomesSmallIntAndPropagate(bifdbm_varptr[i]);
          } else {
            tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_det,
                                                      makeTaggedRef(bifdbm_varptr[i]));
            doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i],
                           newSmallInt(bifdbm_dom[i]->singl()));
            glob_vars_touched = TRUE;
          }
        } else {
          tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_bounds,
                                                    makeTaggedRef(bifdbm_varptr[i]));
          if (! bifdbm_is_local[i]) {
            GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[i]);
            TaggedRef * newtaggedfdvar = newTaggedCVar(newfdvar);
            doBindAndTrailAndIP(bifdbm_var[i], bifdbm_varptr[i],
                                makeTaggedRef(newtaggedfdvar),
                                newfdvar, tagged2GenFDVar(bifdbm_var[i]), NO);
            newfdvar->setTag();
            glob_vars_touched = TRUE;
          } else {
            glob_vars_touched |= tagged2GenFDVar(bifdbm_var[i])->isTagged();
          }

          vars_left = TRUE;
        }
        PROFILE_CODE1(
                      if (FDVarsTouched.add(bifdbm_var[i]))
                         FDProfiles.inc_item(no_touched_vars);
                      )
      }
    } else if (vartag == SVAR) {
      DebugCheck(bifdbm_is_local[i], error("Global SVAR expected."));

      glob_vars_touched = TRUE;

      if (*bifdbm_dom[i] == fd_singleton) {
        TaggedRef newsmallint = newSmallInt(bifdbm_dom[i]->singl());
        am.checkSuspensionList(bifdbm_var[i], newsmallint);
        doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i], newsmallint);
      } else {
        GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[i]);
        TaggedRef * newtaggedfdvar = newTaggedCVar(newfdvar);
        am.checkSuspensionList(bifdbm_var[i], makeTaggedRef(newtaggedfdvar));
        doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i],
                       makeTaggedRef(newtaggedfdvar));
        vars_left = TRUE;
      }
      PROFILE_CODE1(
                    if (FDVarsTouched.add(bifdbm_var[i]))
                       FDProfiles.inc_item(no_touched_vars);
                    )

    } else  {
      error("Unexpected vartag (0x%x) found.", vartag);
    }
  } // for
} // BIfdBodyManager::process

void BIfdBodyManager::process(void) {
  processFromTo(0, curr_num_of_vars);
}

void BIfdBodyManager::processNonRes(void)
{
  Suspension * susp = NULL;

  TypeOfTerm vartag = bifdbm_vartag[0];

  if (vartag == CVAR && isTouched(0)) {
    if (*bifdbm_dom[0] == fd_singleton) {
      if (bifdbm_is_local[0]) {
        glob_vars_touched |= tagged2GenFDVar(bifdbm_var[0])->isTagged();
        tagged2GenFDVar(bifdbm_var[0])->
          becomesSmallIntAndPropagate(bifdbm_varptr[0]);
      } else {
        tagged2GenFDVar(bifdbm_var[0])->propagate(bifdbm_var[0], fd_det,
                                                  makeTaggedRef(bifdbm_varptr[0]));

        if (susp == NULL) susp = new Suspension(am.currentBoard);
        addSuspFDVar(bifdbm_var[0], new SuspList(susp, NULL));

        doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0],
                       newSmallInt(bifdbm_dom[0]->singl()));
        glob_vars_touched = TRUE;
      }
    } else {
      tagged2GenFDVar(bifdbm_var[0])->propagate(bifdbm_var[0], fd_bounds,
                                                makeTaggedRef(bifdbm_varptr[0]));
      if (! bifdbm_is_local[0]) {
        if (susp == NULL) susp = new Suspension(am.currentBoard);
        addSuspFDVar(bifdbm_var[0], new SuspList(susp, NULL));

        GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[0]);
        TaggedRef * newtaggedfdvar = newTaggedCVar(newfdvar);
        doBindAndTrailAndIP(bifdbm_var[0], bifdbm_varptr[0],
                            makeTaggedRef(newtaggedfdvar),
                            newfdvar, tagged2GenFDVar(bifdbm_var[0]), NO);
        newfdvar->setTag();
        glob_vars_touched = TRUE;
      } else {
        glob_vars_touched |= tagged2GenFDVar(bifdbm_var[0])->isTagged();
      }
    }
    PROFILE_CODE1(
                  if (FDVarsTouched.add(bifdbm_var[0]))
                     FDProfiles.inc_item(no_touched_vars);
                  )
  } else if (vartag == SVAR) {
    DebugCheck(bifdbm_is_local[0], error("Global SVAR expected."));

    if (susp == NULL) susp = new Suspension(am.currentBoard);

    glob_vars_touched = TRUE;

    if (*bifdbm_dom[0] == fd_singleton) {
      TaggedRef newsmallint = newSmallInt(bifdbm_dom[0]->singl());
      am.checkSuspensionList(bifdbm_var[0], newsmallint);
      addSuspSVar(bifdbm_var[0], new CondSuspList(susp, NULL, isConstrained));
      doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0], newsmallint);
    } else {
      GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[0]);
      TaggedRef * newtaggedfdvar = newTaggedCVar(newfdvar);
      am.checkSuspensionList(bifdbm_var[0], makeTaggedRef(newtaggedfdvar));
      addSuspSVar(bifdbm_var[0], new CondSuspList(susp, NULL, isConstrained));
      doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0],
                     makeTaggedRef(newtaggedfdvar));
      vars_left = TRUE;
    }
    PROFILE_CODE1(
                  if (FDVarsTouched.add(bifdbm_var[0]))
                     FDProfiles.inc_item(no_touched_vars);
                  )

  }
} // BIfdBodyManager::processNonRes

Bool BIfdBodyManager::introduce(TaggedRef v)
{
  TypeOfTerm vtag;
  TaggedRef *vptr;

  deref(v, vptr, vtag);

  if (vtag == SMALLINT) {
    int i_val = smallIntValue(v);
    if (i_val >= 0) {
      bifdbm_domain[0].setSingleton(i_val);
      bifdbm_dom[0] = &bifdbm_domain[0];
      bifdbm_is_local[0] = TRUE;
      bifdbm_init_dom_size[0] = 1;
      bifdbm_var[0] = v;
      bifdbm_varptr[0] = vptr;
      bifdbm_vartag[0] = vtag;
   } else {
      return FALSE;
    }
  } else if (isGenFDVar(v,vtag)) {
    GenFDVariable * fdvar = tagged2GenFDVar(v);
    Bool is_local = bifdbm_is_local[0] = am.isLocalCVar(v);
    if (is_local) {
      bifdbm_dom[0] = &fdvar->getDom();
    } else {
      bifdbm_domain[0] = fdvar->getDom();
      bifdbm_dom[0] = &bifdbm_domain[0];
    }
    bifdbm_init_dom_size[0] = bifdbm_dom[0]->getSize();
    bifdbm_var[0] = v;
    bifdbm_varptr[0] = vptr;
    bifdbm_vartag[0] = vtag;
  } else if (vtag == SVAR) {
    if (bifdbm_is_local[0] = am.isLocalSVar(v)) {
      GenFDVariable * fdvar = new GenFDVariable();
      TaggedRef * taggedfdvar = newTaggedCVar(fdvar);
      bifdbm_dom[0] = &fdvar->getDom();
      bifdbm_init_dom_size[0] = bifdbm_dom[0]->getSize();
      am.checkSuspensionList(v, makeTaggedRef(taggedfdvar));
      fdvar->setSuspList(tagged2SVar(v)->getSuspList());
      doBind(vptr, makeTaggedRef(taggedfdvar));
      bifdbm_var[0] = *(bifdbm_varptr[0] = taggedfdvar);
      bifdbm_vartag[0] = CVAR;
    } else {
      bifdbm_domain[0].setFull();
      bifdbm_dom[0] = &bifdbm_domain[0];
      bifdbm_var[0] = v;
      bifdbm_varptr[0] = vptr;
      bifdbm_vartag[0] = vtag;
     }
  } else if (vtag == UVAR) {
    if (bifdbm_is_local[0] = am.isLocalUVar(v)) {
      GenFDVariable * fdvar = new GenFDVariable();
      TaggedRef * taggedfdvar = newTaggedCVar(fdvar);
      bifdbm_dom[0] = &fdvar->getDom();
      bifdbm_init_dom_size[0] = bifdbm_dom[0]->getSize();
      doBind(vptr, makeTaggedRef(taggedfdvar));
      bifdbm_var[0] = *(bifdbm_varptr[0] = taggedfdvar);
      bifdbm_vartag[0] = CVAR;
    } else {
      bifdbm_domain[0].setFull();
      bifdbm_dom[0] = &bifdbm_domain[0];
      *vptr = bifdbm_var[0] = makeTaggedSVar(new SVariable(tagged2VarHome(v)));
      bifdbm_varptr[0] = vptr;
      bifdbm_vartag[0] = SVAR;
    }

  } else {
    return FALSE;
  }

  return TRUE;
} // BIfdBodyManager::introduce


int BIfdBodyManager::simplifyBody(int ts, STuple &a, STuple &x,
                                  Bool sign_bits[], float coeffs[])
{
  // 1st pass: mark first occ of a var and sum up coeffs of further occs
  for (int i = 0; i < ts; i++) {
    Assert(isCVar(bifdbm_vartag[i]) || isSmallInt(bifdbm_vartag[i]));
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

    float coeffs_from = coeffs[from];

    if (coeffs_from == 0.0 || var_from == newSmallInt(0)) continue;

    if (from != to) {
      coeffs[to] = coeffs_from;
      bifdbm_var[to] = var_from;
      bifdbm_varptr[to] = varptr_from;
      bifdbm_vartag[to] = bifdbm_vartag[from];
      bifdbm_dom[to] = bifdbm_dom[from];
      sign_bits[to] = sign_bits[from];
      bifdbm_init_dom_size[to] = bifdbm_init_dom_size[from];
      bifdbm_is_local[to] = bifdbm_is_local[from];
      a[to] = a[from];
      x[to] = x[from];
    }
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
  int v;
  for (v = 0; v < variables; v++) {
    Assert(isCVar(bifdbm_vartag[idx_v(v)]) ||
           isSmallInt(bifdbm_vartag[idx_v(v)]));
    if (isAnyVar(bifdbm_var[idx_v(v)]))
      if (! isTaggedIndex(*bifdbm_varptr[idx_v(v)])) {
        *bifdbm_varptr[idx_v(v)] = makeTaggedIndex(v);
      } else {
        int fs = getIndex(*bifdbm_varptr[idx_v(v)]);

        // unify corresponding local variables
        for (int c = 0; c < clauses; c++) {
          // check void variables
          TaggedRef l_var, r_var;

          if (isLiteral(bifdbm_vartag[idx_vp(c, fs)])) {
            // convert void variable to heap variable
            GenFDVariable * fv = new GenFDVariable(*bifdbm_dom[idx_vp(c, fs)]);
            l_var = makeTaggedRef(newTaggedCVar(fv));
            // update arguments
            STuple &vp_c = *tagged2STuple(deref(vp[c]));
            vp_c[fs] = l_var;
          } else if (isSmallInt(bifdbm_vartag[idx_vp(c, fs)])) {
            l_var = bifdbm_var[idx_vp(c, fs)];
          } else if (isCVar(bifdbm_vartag[idx_vp(c, fs)])) {
            l_var = makeTaggedRef(bifdbm_varptr[idx_vp(c, fs)]);
          } else {
            error("Unexpected type found for l_var variable.");
          }

          if (isLiteral(bifdbm_vartag[idx_vp(c, v)])) {
            // convert void variable to heap variable
            GenFDVariable * fv = new GenFDVariable(*bifdbm_dom[idx_vp(c, v)]);
            r_var = makeTaggedRef(newTaggedCVar(fv));
            // update arguments
            STuple &vp_c = *tagged2STuple(deref(vp[c]));
            vp_c[v] = r_var;
          } else if (isSmallInt(bifdbm_vartag[idx_vp(c, v)])) {
            r_var = bifdbm_var[idx_vp(c, v)];
          } else if (isCVar(bifdbm_vartag[idx_vp(c, v)])) {
            r_var  = makeTaggedRef(bifdbm_varptr[idx_vp(c, v)]);
          } else {
            error("Unexpected type found for r_var variable.");
          }
          if (OZ_unify(l_var, r_var) == FALSE) {
            *bifdbm_dom[idx_b(c)] &= 0;
            goto escape;
          }
          introduceLocal(idx_vp(c, fs), l_var);
          introduceLocal(idx_vp(c, v), r_var);
        }
      }
  }

 escape:
  // 2nd pass: undo marks
  for (v = 0; v < variables; v++) {
    if (isAnyVar(bifdbm_vartag[idx_v(v)]))
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

//-----------------------------------------------------------------------------
// Introduce FD Built-ins to the Emulator

void BIinitFD()
{
// fdprofil.cc
  BIadd("fdReset", 0, BIfdReset);
  BIadd("fdDiscard", 0, BIfdDiscard);
  BIadd("fdGetNext", 1, BIfdGetNext);
  BIadd("fdPrint", 0, BIfdPrint);
  BIadd("fdTotalAverage", 0, BIfdTotalAverage);

// fdcore.cc
  BIadd("fdIs", 1, BIfdIs, FALSE, (InlineFunOrRel) BIfdIsInline);
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
  BIadd("fdAllDifferent_body", 2, BIfdAllDifferent_body);

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
  BIadd("fdMod", 3, BIfdMod);
  BIadd("fdMod_body", 3, BIfdMod_body);
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
  BIadd("fdCDSched", 4, BIfdCDSched);
  BIadd("fdCDSched_body", 4, BIfdCDSched_body);
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
}
