/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(__GNUC__)
#pragma implementation "fdbuilti.hh"
#endif

#include "fdbuilti.hh"
#include "unify.hh"

#if defined(OUTLINE) || defined(FDOUTLINE)
#define inline
#include "fdbuilti.icc"
#undef inline
#endif


//-----------------------------------------------------------------------------
// class BIfdHeadManager
//-----------------------------------------------------------------------------
// Global data

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
  TaggedRef prev_fdvar;
  TaggedRefPtr prev_fdvarptr = NULL;
  TaggedRef var;
  TaggedRefPtr varptr;
  TypeOfTerm vartag;
  long prod = 1;
  Suspension * susp;

  for (int j = ts, fds_found = 0; j-- && (fds_found < 2); ) {
    var = TaggedRef(&xtc[j]);
    deref(var, varptr, vartag);

    if (vartag == SMALLINT) {
      prod *= smallIntValue(var);
      if (prod < OzMinInt || OzMaxInt < prod) return FALSE;
    } else if (isGenFDVar(var,vartag)) {
      fds_found += 1;
      prev_fdvar = var;
      prev_fdvarptr = varptr;
    } else if (isUVar(vartag)) {
      fds_found = 3;
    } else if (isSVar(vartag)) {
      fds_found = 4;
    } else {
      fds_found = 5;
    }
  }

  switch (fds_found) {
  case 0:
    bifdhm_vartag[i] = SMALLINT;
    xt[i] = bifdhm_var[i] = newSmallInt(1);
    bifdhm_coeff[i] *= prod;
    if (bifdhm_coeff[i] < OzMinInt || OzMaxInt < bifdhm_coeff[i]) return FALSE;
    at[i] = newSmallInt(bifdhm_coeff[i]);
    return TRUE;
  case 1:
    bifdhm_vartag[i] = CVAR;
    bifdhm_var[i] = prev_fdvar;
    xt[i] = TaggedRef(bifdhm_varptr[i] = prev_fdvarptr);
    bifdhm_coeff[i] *= prod;
    if (bifdhm_coeff[i] < OzMinInt || OzMaxInt < bifdhm_coeff[i]) return FALSE;
    at[i] = newSmallInt(bifdhm_coeff[i]);
    return TRUE;
  case 2:
    s += 1;
    susp = createNonResSusp(func, xregs, arity);
    addSuspFDVar(var, new SuspList(susp, NULL), fd_det);
    addSuspFDVar(prev_fdvar, new SuspList(susp, NULL), fd_det);
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
  } else if (tag == UVAR) {
    if (bifdhm_var[i] != *bifdhm_varptr[i]) return;
    if (am.isLocalUVar(bifdhm_var[i])) {
      TaggedRef * taggedfdvar = newTaggedCVar(new GenFDVariable());
      addSuspFDVar(*taggedfdvar, new SuspList(susp, NULL), target);
      doBind(bifdhm_varptr[i], TaggedRef(taggedfdvar));
    } else {
      addSuspUVar(bifdhm_varptr[i], new SuspList(susp, NULL));
    }
  } else if (tag == SVAR) {
    if (bifdhm_var[i] != *bifdhm_varptr[i]) return;
    if (am.isLocalSVar(bifdhm_var[i])) {
      GenFDVariable * fdvar = new GenFDVariable();
      TaggedRef * taggedfdvar = newTaggedCVar(fdvar);
      am.checkSuspensionList(bifdhm_var[i], makeTaggedRef(taggedfdvar), NULL);
      fdvar->setSuspList(tagged2SVar(bifdhm_var[i])->getSuspList());
      addSuspFDVar(*taggedfdvar, new SuspList(susp, NULL), target);
      doBind(bifdhm_varptr[i], TaggedRef(taggedfdvar));
    } else {
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
const int taggedIndex = 0x80000000;
inline TaggedRef makeTaggedIndex(TaggedRef t) {return (t | taggedIndex);}
inline TaggedRef getIndex(TaggedRef t) {return (t & ~taggedIndex);}
inline Bool isTaggedIndex(TaggedRef t) {return (t & taggedIndex);}

int BIfdHeadManager::simplifyHead(int ts, STuple &a, STuple &x)
{
  // 1st pass: mark first occ of a var and sum up coeffs of further occs
  for (int i = 0; i < ts; i++)
    if (isAnyVar(bifdhm_var[i]))
      if (! isTaggedIndex(*bifdhm_varptr[i])) {
        *bifdhm_varptr[i] = makeTaggedIndex(i);
      } else {
        int ind = getIndex(*bifdhm_varptr[i]);
        a[ind] = newSmallInt(bifdhm_coeff[ind] += bifdhm_coeff[i]);
        bifdhm_var[i] = 0;
      }

  // 2nd pass: undo marks and compress vector
  for (int from = 0, to = 0; from < ts; from += 1) {
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
FiniteDomainPtr BIfdBodyManager::bifdbm_dom[MAXFDBIARGS];
FiniteDomain BIfdBodyManager::bifdbm_domain[MAXFDBIARGS];
int BIfdBodyManager::bifdbm_init_dom_size[MAXFDBIARGS];
Bool BIfdBodyManager::bifdbm_is_local[MAXFDBIARGS];
int BIfdBodyManager::curr_num_of_vars;
Bool BIfdBodyManager::vars_left;
Bool BIfdBodyManager::glob_vars_touched;

void BIfdBodyManager::initStaticData(void) {
  bifdbm_var = static_var;
  bifdbm_varptr = static_varptr;
  bifdbm_vartag = static_vartag;
  curr_num_of_vars = 0;
}

//-----------------------------------------------------------------------------
// Member functions

void BIfdBodyManager::introduce(int i, TaggedRef v)
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
  } else {
    error("Found unexpected term tag.");;
  }
  bifdbm_var[i] = v;
  bifdbm_varptr[i] = vptr;
  bifdbm_vartag[i] = vtag;
}


void BIfdBodyManager::process(void)
{
  vars_left = glob_vars_touched = FALSE;

  for (int i = curr_num_of_vars; i--; ) {
    TypeOfTerm vartag = bifdbm_vartag[i];

    if (vartag == SMALLINT) {
      continue;
    } else if (vartag == CVAR) {
      if (! isTouched(i)) {
        vars_left = TRUE;
      } else if (*bifdbm_dom[i] == fd_singleton) {
        if (bifdbm_is_local[i]) {
          tagged2GenFDVar(bifdbm_var[i])->
            becomesSmallIntAndPropagate(bifdbm_varptr[i]);
        } else {
          tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_det,
                                                    TaggedRef(bifdbm_varptr[i]));
          doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i],
                         newSmallInt(bifdbm_dom[i]->singl()));
          glob_vars_touched = TRUE;
        }
      } else {
        tagged2GenFDVar(bifdbm_var[i])->propagate(bifdbm_var[i], fd_bounds,
                                                  TaggedRef(bifdbm_varptr[i]));
        if (! bifdbm_is_local[i]) {
          GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[i]);
          TaggedRef * newtaggedfdvar = newTaggedCVar(newfdvar);
          doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i], TaggedRef(newtaggedfdvar));
          glob_vars_touched = TRUE;
        }
        vars_left = TRUE;
      }
    } else if (vartag == SVAR) {
      DebugCheck(bifdbm_is_local[i], error("Global SVAR expected."));

      glob_vars_touched = TRUE;

      if (*bifdbm_dom[i] == fd_singleton) {
        TaggedRef newsmallint = newSmallInt(bifdbm_dom[i]->singl());
        am.checkSuspensionList(bifdbm_var[i], newsmallint, NULL);
        doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i], newsmallint);
      } else {
        GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[i]);
        TaggedRef * newtaggedfdvar = newTaggedCVar(newfdvar);
        am.checkSuspensionList(bifdbm_var[i], makeTaggedRef(newtaggedfdvar), NULL);
        doBindAndTrail(bifdbm_var[i], bifdbm_varptr[i], TaggedRef(newtaggedfdvar));
        vars_left = TRUE;
      }
    } else  {
      error("Unexpected vartag (0x%x) found.", vartag);
    }
  } // for
} // BIfdBodyManager::process

void BIfdBodyManager::processNonRes(void)
{
  Suspension * susp = NULL;

  TypeOfTerm vartag = bifdbm_vartag[0];

  if (vartag == CVAR && isTouched(0)) {
    if (*bifdbm_dom[0] == fd_singleton) {
      if (bifdbm_is_local[0]) {
        tagged2GenFDVar(bifdbm_var[0])->
          becomesSmallIntAndPropagate(bifdbm_varptr[0]);
      } else {
        tagged2GenFDVar(bifdbm_var[0])->propagate(bifdbm_var[0], fd_det,
                                                  TaggedRef(bifdbm_varptr[0]));

        if (susp == NULL) susp = new Suspension(am.currentBoard);
        addSuspFDVar(bifdbm_var[0], new SuspList(susp, NULL), fd_size);

        doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0],
                       newSmallInt(bifdbm_dom[0]->singl()));
      }
    } else {
      tagged2GenFDVar(bifdbm_var[0])->propagate(bifdbm_var[0], fd_bounds,
                                                TaggedRef(bifdbm_varptr[0]));
      if (! bifdbm_is_local[0]) {
        if (susp == NULL) susp = new Suspension(am.currentBoard);
        addSuspFDVar(bifdbm_var[0], new SuspList(susp, NULL), fd_size);

        GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[0]);
        TaggedRef * newtaggedfdvar = newTaggedCVar(newfdvar);
        doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0], TaggedRef(newtaggedfdvar));
        glob_vars_touched = TRUE;
      }
    }
  } else if (vartag == SVAR) {
    DebugCheck(bifdbm_is_local[0], error("Global SVAR expected."));

    if (susp == NULL) susp = new Suspension(am.currentBoard);

    if (*bifdbm_dom[0] == fd_singleton) {
      TaggedRef newsmallint = newSmallInt(bifdbm_dom[0]->singl());
      am.checkSuspensionList(bifdbm_var[0], newsmallint, NULL);
      addSuspSVar(bifdbm_var[0], new CondSuspList(susp, NULL, isConstrained));
      doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0], newsmallint);
    } else {
      GenFDVariable * newfdvar = new GenFDVariable(*bifdbm_dom[0]);
      TaggedRef * newtaggedfdvar = newTaggedCVar(newfdvar);
      am.checkSuspensionList(bifdbm_var[0], makeTaggedRef(newtaggedfdvar), NULL);
      addSuspSVar(bifdbm_var[0], new CondSuspList(susp, NULL, isConstrained));
      doBindAndTrail(bifdbm_var[0], bifdbm_varptr[0], TaggedRef(newtaggedfdvar));
      vars_left = TRUE;
    }
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
      am.checkSuspensionList(v, TaggedRef(taggedfdvar), NULL);
      fdvar->setSuspList(tagged2SVar(v)->getSuspList());
      doBind(vptr, TaggedRef(taggedfdvar));
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
      doBind(vptr, TaggedRef(taggedfdvar));
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
  for (int i = 0; i < ts; i++)
    if (isAnyVar(bifdbm_var[i]))
      if (! isTaggedIndex(*bifdbm_varptr[i])) {
        *bifdbm_varptr[i] = makeTaggedIndex(i);
      } else {
        int ind = getIndex(*bifdbm_varptr[i]);
        a[ind] = newSmallInt(int(coeffs[ind] += coeffs[i]));
        bifdbm_var[i] = 0;
      }

  // 2nd pass: undo marks and compress vector
  for (int from = 0, to = 0; from < ts; from += 1) {
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

Bool BIfdBodyManager::_unifiedVars(void)
{
  Bool ret = FALSE;

  // 1st pass: mark occ of var and break if you find already touched var
  for (int i = 0; i < curr_num_of_vars; i += 1)
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
// fdcore.C
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
  BIadd("fdPutNot", 2, BIfdPutNot);

// fdrel.C
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
  BIadd("fdAllDifferent_body", 1, BIfdAllDifferent_body);

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

// fdarith.C
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
  BIadd("fdMinus_rel", 3, BIfdMinus_rel);
  BIadd("fdMult_rel", 3, BIfdMult_rel);

// fdgeneric.C
  BIadd("fdGenLinEq", 3, BIfdGenLinEq);
  BIadd("fdGenLinEq_body", 3, BIfdGenLinEq_body);
  BIadd("fdGenNonLinEq", 3, BIfdGenNonLinEq);
  BIadd("fdGenLinNotEq", 3, BIfdGenLinNotEq);
  BIadd("fdGenLinNotEq_body", 3, BIfdGenLinNotEq_body);
  BIadd("fdGenNonLinNotEq", 3, BIfdGenNonLinNotEq);
  BIadd("fdGenLinLessEq", 3, BIfdGenLinLessEq);
  BIadd("fdGenLinLessEq_body", 3, BIfdGenLinLessEq_body);
  BIadd("fdGenNonLinLessEq", 3, BIfdGenNonLinLessEq);
  BIadd("fdGenLinAbs", 4, BIfdGenLinAbs);
  BIadd("fdGenLinAbs_body", 4, BIfdGenLinAbs_body);

// fdcard.C
  BIadd("fdElement", 3, BIfdElement);
  BIadd("fdElement_body", 3, BIfdElement_body);
  BIadd("fdAtMost", 3, BIfdAtMost);
  BIadd("fdAtMost_body", 3, BIfdAtMost_body);

// fdwatch.C
  BIadd("fdWatchDom1", 2, BIfdWatchDom1);
  BIadd("fdWatchDom2", 4, BIfdWatchDom2);
  BIadd("fdWatchDom3", 6, BIfdWatchDom3);

  BIadd("fdWatchBounds1", 3, BIfdWatchBounds1);
  BIadd("fdWatchBounds2", 6, BIfdWatchBounds2);
  BIadd("fdWatchBounds3", 9, BIfdWatchBounds3);

// fdmisc.C
  BIadd("fdCardSched", 4, BIfdCardSched);
  BIadd("fdCardSched_body", 4, BIfdCardSched_body);
  BIadd("fdCDSched", 4, BIfdCDSched);
  BIadd("fdCDSched_body", 4, BIfdCDSched_body);
  BIadd("fdCapacity1", 4, BIfdCapacity1);
  BIadd("fdCapacity1_body", 4, BIfdCapacity1_body);
  BIadd("fdCapacity2", 4, BIfdCapacity2);
  BIadd("fdCapacity2_body", 4, BIfdCapacity2_body);
  BIadd("fdCapacity3", 4, BIfdCapacity3);
  BIadd("fdCapacity3_body", 4, BIfdCapacity3_body);
  BIadd("fdNoOverlap", 6, BIfdNoOverlap);
  BIadd("fdNoOverlap_body", 6, BIfdNoOverlap_body);
  BIadd("fdNoOverlap1", 6, BIfdNoOverlap1);
  BIadd("fdNoOverlap1_body", 6, BIfdNoOverlap1_body);
  BIadd("fdGreaterBool", 3, BIfdGreaterBool);
  BIadd("fdGreaterBool_body", 3, BIfdGreaterBool_body);
  BIadd("fdLessBool", 3, BIfdLessBool);
  BIadd("fdLessBool_body", 3, BIfdLessBool_body);
  BIadd("fdEqualBool", 4, BIfdEqualBool);
  BIadd("fdEqualBool_body", 4, BIfdEqualBool_body);
  BIadd("fdLepcBool", 4, BIfdLepcBool);
  BIadd("fdLepcBool_body", 4, BIfdLepcBool_body);
  BIadd("fdLepcBool1", 4, BIfdLepcBool1);
  BIadd("fdLepcBool1_body", 4, BIfdLepcBool1_body);
  BIadd("fdJoergCard", 4, BIfdJoergCard);
  BIadd("fdJoergCard_body", 4, BIfdJoergCard_body);
  BIadd("fdGenLinEqB", 4, BIfdGenLinEqB);
  BIadd("fdGenLinEqB_body", 4, BIfdGenLinEqB_body);
  BIadd("fdGenLinNotEqB", 4, BIfdGenLinNotEqB);
  BIadd("fdGenLinNotEqB_body", 4, BIfdGenLinNotEqB_body);
  BIadd("fdGenLessEqB", 4, BIfdGenLessEqB);
  BIadd("fdGenLessEqB_body", 4, BIfdGenLessEqB_body);
  BIadd("fdCardBI", 3, BIfdCardBI);
  BIadd("fdCardBI_body", 3, BIfdCardBI_body);
  BIadd("fdInB", 3, BIfdInB);
  BIadd("fdInB_body", 3, BIfdInB_body);
  BIadd("fdIsIntB", 2, BIfdIsIntB);
  BIadd("fdIsIntB_body", 2, BIfdIsIntB_body);
  BIadd("fdCardBIBin", 2, BIfdCardBIBin);
  BIadd("fdCardBIBin_body", 2, BIfdCardBIBin_body);
}
