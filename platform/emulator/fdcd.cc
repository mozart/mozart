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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "fdcd.hh"

#ifdef FDCD
//-----------------------------------------------------------------------------

void initFDCD(void)
{
  BIfdHeadManager::initStaticData();
  BIfdBodyManager::initStaticData();

  __CDVoidFiniteDomain.initFull();
}

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
    case OZ_VAR_FD: tag = pm_fd; break;
    case OZ_VAR_BOOL: tag = pm_bool; break;
    default:
      tag = oz_isNonKinded(tr1) ? pm_svar : pm_none; break;
    }
    break;
  // mm2 FUT
  case UVAR: tag = pm_uvar; break;
  case SRECORD: tag = oz_isSTuple(tr1) ? pm_tuple : pm_none; break;
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

OZ_Term * static_var = new OZ_Term[MAXFDBIARGS];
OZ_Term ** static_varptr = new OZ_Term *[MAXFDBIARGS];
pm_term_type * static_vartag = new pm_term_type[MAXFDBIARGS];
double * static_coeff_double = new double[MAXFDBIARGS];
int * static_coeff_int = new int[MAXFDBIARGS];
OZ_Boolean * static_sign_bit = new OZ_Boolean[MAXFDBIARGS];

OZ_Boolean * static_bool_a = new OZ_Boolean[MAXFDBIARGS];
OZ_Boolean * static_bool_b = new OZ_Boolean[MAXFDBIARGS];
int * static_int_a = new int[MAXFDBIARGS];
int * static_int_b = new int[MAXFDBIARGS];
double * static_double_a = new double[MAXFDBIARGS];
double * static_double_b = new double[MAXFDBIARGS];

int * static_index_offset = new int[MAXFDBIARGS];
int * static_index_size = new int[MAXFDBIARGS];

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

  return (bifdhm_varptr[a] == bifdhm_varptr[b] && oz_isVariable(bifdhm_var[a]));
}

#ifdef FDBISTUCK

OZ_Return BIfdHeadManager::addSuspFDish(OZ_CFun, OZ_Term *, int) {
  am.emptySuspendVarList();

  for (int i = curr_num_of_items; i--; )
    if (pm_is_noncvar(bifdhm_vartag[i])) {
      AssertFD(oz_isVariable(*bifdhm_varptr[i]));
      am.addSuspendVarList(bifdhm_varptr[i]);
    }

  AssertFD(!am.isEmptySuspendVarList());

  return SUSPEND;
}

OZ_Return BIfdHeadManager::addSuspSingl(OZ_CFun, OZ_Term *, int) {
  am.emptySuspendVarList();

  for (int i = curr_num_of_items; i--; )
    if (pm_is_var(bifdhm_vartag[i])) {
      AssertFD(oz_isVariable(*bifdhm_varptr[i]));
      am.addSuspendVarList(bifdhm_varptr[i]);
    }

  AssertFD(!am.isEmptySuspendVarList());

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
      AssertFD(oz_isVariable(*bifdhm_varptr[i]));
      OZ_addThread(makeTaggedRef(bifdhm_varptr[i]), th);
    }

  return PROCEED;
}

OZ_Return BIfdHeadManager::addSuspSingl(OZ_CFun f, OZ_Term * x, int a) {
  OZ_Thread th = OZ_makeSuspendedThread(f, x, a);

  for (int i = curr_num_of_items; i--; )
    if (pm_is_var(bifdhm_vartag[i])) {
      AssertFD(oz_isVariable(*bifdhm_varptr[i]));
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

#ifdef DEBUG_PRINT
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
#endif

BIfdBodyManager::BIfdBodyManager(int s) {
  DebugCodeFD(backup_count = 0);
  AssertFD(0 <= s && s < MAXFDBIARGS);
  curr_num_of_vars = s;
  AssertFD(Propagator::getRunningPropagator());
  only_local_vars = Propagator::getRunningPropagator()->isLocalPropagator();
}

void BIfdBodyManager::saveDomainOnTopLevel(int i) {
  if (oz_onToplevel()) {
    if (bifdbm_vartag[i] == pm_fd)
      bifdbm_domain[i] = tagged2GenFDVar(bifdbm_var[i])->getDom();
  }
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
        DoBind(bifdbm_varptr[j], makeTaggedRef(bifdbm_varptr[i]));
      } else if (bifdbm_vartag[i] == pm_bool) {
        if (bifdbm_vartag[j] == pm_fd) {
          tagged2GenFDVar(bifdbm_var[j])->
            relinkSuspListTo(tagged2GenBoolVar(bifdbm_var[i]));
        } else {
          tagged2GenBoolVar(bifdbm_var[j])->
            relinkSuspListTo(tagged2GenBoolVar(bifdbm_var[i]));
        }
        DoBind(bifdbm_varptr[j], makeTaggedRef(bifdbm_varptr[i]));
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
  return bifdbm_varptr[a] == bifdbm_varptr[b] && oz_isVariable(bifdbm_var[a]);
}

void BIfdBodyManager::restoreDomainOnToplevel(void) {
  if (oz_onToplevel()) {
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
  DebugCheck(i < 0 || i >= curr_num_of_vars, OZ_error("index overflow"));

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

    if (bifdbm_vartag[i] == pm_singl || oz_isSmallInt(*bifdbm_varptr[i])) {
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
        tagged2GenFDVar(bifdbm_var[i])->propagate(fd_prop_bounds);
        vars_left = OZ_TRUE;
      }
    }
  }
}


void BIfdBodyManager::_introduce(int i, OZ_Term v)
{
  DebugCheck(i < 0 || i >= curr_num_of_vars, OZ_error("index overflow"));

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
    bifdbm_var_state[i]
      = (oz_isLocalVar(tagged2CVar(v)) ? fdbm_local : fdbm_global);
  } else if (vtag == pm_fd) {
    OzFDVariable * fdvar = tagged2GenFDVar(v);
    OZ_Boolean var_state = bifdbm_var_state[i]
      = (oz_isLocalVar(fdvar) ? fdbm_local : fdbm_global);
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
    bifdbm_var_state[i] = (oz_isLocalVar(tagged2CVar(v)) ? fdbm_local : fdbm_global);
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

    if (vartag == pm_singl || oz_isSmallInt(*bifdbm_varptr[i])) {
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
          tagged2GenFDVar(bifdbm_var[i])->propagate(fd_prop_singl);
          DoBindAndTrail(bifdbm_varptr[i],
                         OZ_int(bifdbm_dom[i]->getSingleElem()));
        }
      } else if (*bifdbm_dom[i] == fd_bool) {

        if (bifdbm_var_state[i] == fdbm_local) {
          tagged2GenFDVar(bifdbm_var[i])->
            becomesBoolVarAndPropagate(bifdbm_varptr[i]);
        } else {
          tagged2GenFDVar(bifdbm_var[i])->propagate(fd_prop_bounds);
          OzBoolVariable * newboolvar
            = new OzBoolVariable(oz_currentBoard());
          OZ_Term * newtaggedboolvar = newTaggedCVar(newboolvar);
          DoBindAndTrailAndIP(bifdbm_varptr[i],
                              makeTaggedRef(newtaggedboolvar),
                              newboolvar, tagged2GenBoolVar(bifdbm_var[i]));
        }

      } else {

        tagged2GenFDVar(bifdbm_var[i])->propagate(fd_prop_bounds);
        if (bifdbm_var_state[i] == fdbm_global) {
          OzFDVariable * newfdvar
            = new OzFDVariable(*bifdbm_dom[i],oz_currentBoard());
          OZ_Term * newtaggedfdvar = newTaggedCVar(newfdvar);
          DoBindAndTrailAndIP(bifdbm_varptr[i],
                              makeTaggedRef(newtaggedfdvar),
                              newfdvar, tagged2GenFDVar(bifdbm_var[i]));
        }

        vars_left = OZ_TRUE;
      }

    } else if (vartag == pm_bool) {
      Assert(*bifdbm_dom[i] == fd_singl);

      if (bifdbm_var_state[i] == fdbm_local) {
        tagged2GenBoolVar(bifdbm_var[i])->
          becomesSmallIntAndPropagate(bifdbm_varptr[i], *bifdbm_dom[i]);
      } else {
        tagged2GenBoolVar(bifdbm_var[i])->propagate();
        DoBindAndTrail(bifdbm_varptr[i],
                       OZ_int(bifdbm_dom[i]->getSingleElem()));
      }
    } else {
      Assert(vartag == pm_svar && bifdbm_var_state[i] == fdbm_global);

      ozstat.fdvarsCreated.incf();

      if (*bifdbm_dom[i] == fd_singl) {
        OZ_Term smallInt = OZ_int(bifdbm_dom[i]->getSingleElem());
        oz_checkSuspensionListProp(tagged2SVarPlus(bifdbm_var[i]));
        DoBindAndTrail(bifdbm_varptr[i], smallInt);
      } else if (*bifdbm_dom[i] == fd_bool) {
        OzBoolVariable * newboolvar = new OzBoolVariable(oz_currentBoard());
        OZ_Term * newtaggedboolvar = newTaggedCVar(newboolvar);
        oz_checkSuspensionListProp(tagged2SVarPlus(bifdbm_var[i]));
        DoBindAndTrail(bifdbm_varptr[i],
                       makeTaggedRef(newtaggedboolvar));
        vars_left = OZ_TRUE;
      } else {
        OzFDVariable * newfdvar
          = new OzFDVariable(*bifdbm_dom[i],oz_currentBoard());
        OZ_Term * newtaggedfdvar = newTaggedCVar(newfdvar);
        oz_checkSuspensionListProp(tagged2SVarPlus(bifdbm_var[i]));
        DoBindAndTrail(bifdbm_varptr[i],
                       makeTaggedRef(newtaggedfdvar));
        vars_left = OZ_TRUE;
      }
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
        tagged2GenFDVar(bifdbm_var[0])->propagate(fd_prop_singl);
        DoBindAndTrail(bifdbm_varptr[0],
                       OZ_int(bifdbm_dom[0]->getSingleElem()));
      }
    } else if (*bifdbm_dom[0] == fd_bool) {
      tagged2GenFDVar(bifdbm_var[0])->propagate(fd_prop_bounds);
      if (bifdbm_var_state[0] == fdbm_global) {
        OzBoolVariable * newboolvar = new OzBoolVariable(oz_currentBoard());
        OZ_Term * newtaggedboolvar = newTaggedCVar(newboolvar);
        DoBindAndTrailAndIP(bifdbm_varptr[0],
                            makeTaggedRef(newtaggedboolvar),
                            newboolvar, tagged2GenFDVar(bifdbm_var[0]));
      } else {
        tagged2GenFDVar(bifdbm_var[0])->
          becomesBoolVarAndPropagate(bifdbm_varptr[0]);
      }
    } else {
      tagged2GenFDVar(bifdbm_var[0])->propagate(fd_prop_bounds);
      if (bifdbm_var_state[0] == fdbm_global) {
        OzFDVariable * newfdvar
          = new OzFDVariable(*bifdbm_dom[0],oz_currentBoard());
        OZ_Term * newtaggedfdvar = newTaggedCVar(newfdvar);
        DoBindAndTrailAndIP(bifdbm_varptr[0],
                            makeTaggedRef(newtaggedfdvar),
                            newfdvar, tagged2GenFDVar(bifdbm_var[0]));
      }
    }

  } else if (vartag == pm_bool) {
    Assert(*bifdbm_dom[0] == fd_singl);

    if (bifdbm_var_state[0] == fdbm_local) {
      tagged2GenBoolVar(bifdbm_var[0])->
        becomesSmallIntAndPropagate(bifdbm_varptr[0], *bifdbm_dom[0]);
    } else {
      tagged2GenBoolVar(bifdbm_var[0])->propagate();
      DoBindAndTrail(bifdbm_varptr[0],
                     OZ_int(bifdbm_dom[0]->getSingleElem()));
    }
  } else {
    Assert(bifdbm_var_state[0] == fdbm_global && vartag == pm_svar);

    ozstat.fdvarsCreated.incf();

    if (*bifdbm_dom[0] == fd_singl) {
      OZ_Term smallInt = OZ_int(bifdbm_dom[0]->getSingleElem());
      oz_checkSuspensionListProp(tagged2SVarPlus(bifdbm_var[0]));
      DoBindAndTrail(bifdbm_varptr[0], smallInt);
    } else if (*bifdbm_dom[0] == fd_bool) {
      OzBoolVariable * newboolvar = new OzBoolVariable(oz_currentBoard());
      OZ_Term * newtaggedboolvar = newTaggedCVar(newboolvar);
      oz_checkSuspensionListProp(tagged2SVarPlus(bifdbm_var[0]));
      DoBindAndTrail(bifdbm_varptr[0],
                     makeTaggedRef(newtaggedboolvar));
      vars_left = OZ_TRUE;
    } else {
      OzFDVariable * newfdvar
        = new OzFDVariable(*bifdbm_dom[0],oz_currentBoard());
      OZ_Term * newtaggedfdvar = newTaggedCVar(newfdvar);
      oz_checkSuspensionListProp(tagged2SVarPlus(bifdbm_var[0]));
      DoBindAndTrail(bifdbm_varptr[0],
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
    bifdbm_var_state[0]
      = (oz_isLocalVar(tagged2CVar(v)) ? fdbm_local : fdbm_global);
    bifdbm_var[0] = v;
    bifdbm_varptr[0] = vptr;
    bifdbm_vartag[0] = vtag;
  } else if (vtag == pm_fd) {
    OzFDVariable * fdvar = tagged2GenFDVar(v);
    fdbm_var_state var_state = bifdbm_var_state[0]
      = (oz_isLocalVar(fdvar) ? fdbm_local : fdbm_global);
    if (var_state == fdbm_local) {
      bifdbm_dom[0] = &fdvar->getDom();
      if (oz_onToplevel())
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

    bifdbm_var_state[0]
      = (oz_isLocalVar(tagged2CVar(v)) ? fdbm_local : fdbm_global);
    if (bifdbm_var_state[0] == fdbm_local) {
      OzFDVariable * fdvar = new OzFDVariable(oz_currentBoard());
      OZ_Term * taggedfdvar = newTaggedCVar(fdvar);
      bifdbm_dom[0] = &fdvar->getDom();
      bifdbm_init_dom_size[0] = bifdbm_dom[0]->getSize();
      oz_checkSuspensionListProp(tagged2SVarPlus(v));
      fdvar->setSuspList(tagged2SVarPlus(v)->unlinkSuspList()); // mm2
      DoBind(vptr, makeTaggedRef(taggedfdvar));
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

    bifdbm_var_state[0]
      = (oz_isLocalUVar(vptr)? fdbm_local : fdbm_global);
    if (bifdbm_var_state[0] == fdbm_local) {
      OzFDVariable * fdvar = new OzFDVariable(oz_currentBoard());
      OZ_Term * taggedfdvar = newTaggedCVar(fdvar);
      bifdbm_dom[0] = &fdvar->getDom();
      bifdbm_init_dom_size[0] = bifdbm_dom[0]->getSize();
      DoBind(vptr, makeTaggedRef(taggedfdvar));
      bifdbm_var[0] = *(bifdbm_varptr[0] = taggedfdvar);
      bifdbm_vartag[0] = pm_fd;
    } else {
      bifdbm_domain[0].initFull();
      bifdbm_dom[0] = &bifdbm_domain[0];
      *vptr = bifdbm_var[0] = oz_newVar(tagged2VarHome(v));
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

    if (oz_isVariable(bifdbm_var[idx_v(v)]))
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
            OzFDVariable * fv = new OzFDVariable(*bifdbm_dom[idx_vp(c, fs)],
                                                   oz_currentBoard());
            bifdbm_varptr[idx_vp(c, fs)] = newTaggedCVar(fv);
            l_var = makeTaggedRef(bifdbm_varptr[idx_vp(c, fs)]);
            bifdbm_vartag[idx_vp(c, fs)] = pm_fd;
            // update arguments
            SRecord &vp_c = *tagged2SRecord(oz_deref(vp[c]));
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
            OzFDVariable * fv = new OzFDVariable(*bifdbm_dom[idx_vp(c, v)],
                                                   oz_currentBoard());
            bifdbm_varptr[idx_vp(c, v)] = newTaggedCVar(fv);
            r_var = makeTaggedRef(bifdbm_varptr[idx_vp(c, v)]);
            bifdbm_vartag[idx_vp(c, v)] = pm_fd;
            // update arguments
            SRecord &vp_c = *tagged2SRecord(oz_deref(vp[c]));
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

          if (oz_unify(l_var, r_var) == FAILED) { //mm_u
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
    if (oz_isVariable(bifdbm_var[idx_v(v)]))
      *bifdbm_varptr[idx_v(v)] = bifdbm_var[idx_v(v)];
  }
}


OZ_Boolean BIfdBodyManager::_unifiedVars(void)
{
  OZ_Boolean ret = OZ_FALSE;

  // 1st pass: mark occ of var and break if you find already touched var
  int i;
  for (i = 0; i < curr_num_of_vars; i += 1)
    if (oz_isVariable(bifdbm_var[i]))
      if (isTaggedIndex(*bifdbm_varptr[i])) {
        ret = OZ_TRUE;
        break;
      } else {
        *bifdbm_varptr[i] = makeTaggedIndex(i);
      }

  // 2nd pass: undo marks
  for (i = 0; i < curr_num_of_vars; i += 1)
    if (oz_isVariable(bifdbm_var[i]) && isTaggedIndex(*bifdbm_varptr[i]))
      *bifdbm_varptr[i] = bifdbm_var[i];

  return ret;

}

#ifdef DEBUG_PRINT
void BIfdBodyManager::printDebug(void) {
  for (int i = 0; i < curr_num_of_vars; i += 1)
    printDebug(i);
}

void BIfdBodyManager::printDebug(int i) {
  if (bifdbm_dom[i]) {
    cout << '[' << i << "]: v=" << (void *) bifdbm_var[i]
         << ", vptr=" << (void *) bifdbm_varptr[i]
         << ", vtag=" << pm_term_type2string(bifdbm_vartag[i])
         << ", dom=" << bifdbm_dom[i]->toString()
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
    ozd_print(makeTaggedRef(bifdbm_varptr[i]));
    cout << endl << flush;
  } else {
    cout << "ATTENTION *bifdbm_varptr[i]!=bifdbm_var[i]. index="
         << i << endl;
    cout << "bifdbm_varptr";
    ozd_print(makeTaggedRef(bifdbm_varptr[i]));
    cout << endl << flush;
    cout << "bifdbm_var";
    ozd_print(bifdbm_var[i]);
    cout << endl << flush;
  }
}
#endif


//-----------------------------------------------------------------------------

//#define DEBUG_FDCD

Bool do_cd_propagation (LocalPropagationStore &lps) {
  Board *currentBoard = oz_currentBoard();
  RefsArray args;

  // kost@ : --> let's try ...
  Assert (currentBoard->getSuspCount () >= lps.getSize ());

  while (!(lps.isEmpty ())) {
    Propagator * prop = lps.pop();
    Propagator::setRunningPropagator(prop);
    Assert(oz_isCurrentBoard(GETBOARD(prop)));
    //
    //  No 'runnable' threads are allowed here,
    // because only true propagators are in the LPS;
    Assert (! prop->isDeadPropagator() );

    OZ_Return ret_val;

    ret_val = oz_runPropagator(prop);

    switch (ret_val) {
    case FAILED:
      if (oz_onToplevel()) {
        errorHeader();

        ostrstream buf;
        buf << prop->getPropagator()->toString() << '\0';
        char *str = buf.str();
        message("Propagator %s failed\n", str);
        delete str;
      }
      oz_closeDonePropagator(prop);
      return lps.reset();

    case RAISE:
      OZ_error("propagators can't raise exceptions");

    case SUSPEND:
      OZ_error ("propagate_locally: 'SUSPEND' is returned?\n");

    case SLEEP:
      oz_sleepPropagator(prop);
      break;

      case SCHEDULED:
        oz_preemptedPropagator(prop);
        break;

    case PROCEED:
      oz_closeDonePropagator(prop);
      break;
    } // switch
  } // while

  return (TRUE);
}



//-----------------------------------------------------------------------------
// BIfdConstrDisj
// Argument structure:
// [P_1..P_n] [B_1..B_n] [V_1..V_m] [|~~~~~~| .. |~~~~~~|]
//                                    Vp_1_1      Vp_n_1
//                                       .           .
//                                       .           .
//                                    Vp_1_m      Vp_n_m
//                                  [|______| .. |______|]
// P: number of propagators in clause i (expected to be small int)
// V: global variable relevant for disjunction (expected to be finite domain)
// B: variable which reifies true value of clause (expected to be uvar)
// Vp: local variables in clauses (expected to be uvar)
//-----------------------------------------------------------------------------

OZ_C_proc_begin(BIfdConstrDisjSetUp, 4)
{
  ExpectedTypes("tuple of small int,tuple of finite domain,"
                "tuple of tuple var,tuple of var");

  OZ_getCArgDeref(0, p_tuple, p_tupleptr, p_tupletag);
  OZ_getCArgDeref(1, b_tuple, b_tupleptr, b_tupletag);
  OZ_getCArgDeref(2, v_tuple, v_tupleptr, v_tupletag);
  OZ_getCArgDeref(3, vp_tuple, vp_tupleptr, vp_tupletag);

  if (! oz_isSTuple(p_tuple)) TypeError(0, "");
  if (! oz_isSTuple(b_tuple)) TypeError(1, "");

  SRecord &p = *tagged2SRecord(p_tuple);
  SRecord &b = *tagged2SRecord(b_tuple);

  const int clauses = p.getWidth();
  if (clauses != b.getWidth()) {
    OZ_warning("Tuples clauses and b differ in size.");
    return FAILED;
  }

  // constrain Bi to {0..Pi+2} if Bi is an uvar
  int i;
  for (i = clauses; i--; ) {
    TaggedRef bi = makeTaggedRef(&b[i]);
    DEREF(bi, bi_ptr, bi_tag);
    Assert(isUVar(bi));
    OzFDVariable * fdvar = new OzFDVariable(oz_currentBoard());
    fdvar->getDom().initRange(0, OZ_intToC(p[i]) + 2);
    DoBind(bi_ptr, makeTaggedRef(newTaggedCVar(fdvar)));
  }

  // Has already been reduced to sum(b) >= 1
  if (isLiteralTag(v_tupletag)) return PROCEED;

  if (! oz_isSTuple(v_tuple)) TypeError(2, "");
  if (! oz_isSTuple(vp_tuple)) TypeError(3, "");

  SRecord &v = *tagged2SRecord(v_tuple);
  SRecord &vp = *tagged2SRecord(vp_tuple);

  const int variables = v.getWidth();

  // constrain Vpij to {fd_inf..fd_sup} if Vpij is an uvar
  for (i = clauses; i--; ) {
    DEREF(vp[i], vp_i_ptr, vp_i_tag);
    if (! oz_isSTuple(vp[i])) TypeError(3, "2-dim-array expected");
    SRecord &vp_i = *tagged2SRecord(vp[i]);

    if (vp_i.getWidth() != variables) {
      OZ_warning("2-dim-array index incorrect in BIfdConstrDisjSetUp");
      return FAILED;
    }

    for (int j = variables; j--; ) {
      TaggedRef vp_i_j = makeTaggedRef(&vp_i[j]);
      DEREF(vp_i_j, vp_i_j_ptr, vp_i_j_tag);
      if (oz_isFree(vp_i_j)) {
        OZ_Term vj = v[j], vp_i_j_val;
        DEREF(vj, vjptr, vjtag);

        if (isSmallIntTag(vjtag)) {
          vp_i_j_val = vj;
        } else if (isGenBoolVar(vj,vjtag)) {
          vp_i_j_val = makeTaggedRef(newTaggedCVar(new OzBoolVariable(oz_currentBoard())));
        } else if (isGenFDVar(vj,vjtag)) {

          OzFDVariable * fdvar
            = new OzFDVariable(tagged2GenFDVar(vj)->getDom(),
                                oz_currentBoard());
          vp_i_j_val = makeTaggedRef(newTaggedCVar(fdvar));
        } else {
          OzFDVariable * fdvar = new OzFDVariable(oz_currentBoard());
          vp_i_j_val = makeTaggedRef(newTaggedCVar(fdvar));
        }
        DoBind(vp_i_j_ptr, vp_i_j_val);
      }
    }
  }
  return PROCEED;
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

class CDPropagator : public OZ_Propagator {
private:
  static OZ_PropagatorProfile spawner;
protected:
  OZ_Term b_tuple, v_tuple, vp_tuple;
public:
  CDPropagator(OZ_Term b, OZ_Term v, OZ_Term vp)
    : b_tuple(b), v_tuple(v), vp_tuple(vp) {}

  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_collectHeapTerm(b_tuple,b_tuple);
    OZ_collectHeapTerm(v_tuple,v_tuple);
    OZ_collectHeapTerm(vp_tuple,vp_tuple);
  }
  virtual size_t sizeOf(void) { return sizeof(CDPropagator); }
  virtual OZ_Return propagate(void);
  virtual OZ_Term getParameters(void) const { return OZ_nil(); }
  virtual OZ_PropagatorProfile * getProfile(void) const { return &spawner; }
};

int unifiedVars(int sz, OZ_Term * v)
{
  int * is = OZ_findEqualVars(sz, v), nb = 0;
  for (int i = sz; i--; )
    if (is[i] >= 0 && is[i] != i) nb += 1;
  return nb;
}

//-----------------------------------------------------------------------------
// BIfdConstrDisj
// Argument structure:    clause 1    clause n
// [B_1..B_n] [V_1..V_m] [|~~~~~~| .. |~~~~~~|]
//                         Vp_1_1      Vp_n_1
//                           .           .
//                           .           .
//                         Vp_1_m      Vp_n_m
//                       [|______| .. |______|]
// V:global variable relevant for disjunction (expected to be finite domain)
// B:variable which reifies true value of clause (expected to be finite domain)
// Vp: local variables in clauses (expected to be finite domain)
// m: number of variables involved; n: number of clauses
//
// Invariants:
//   o Bs carry suspensions propagators in clauses
//   o Vps carry exclusively suspensions to propagators in the clauses
//   o Vs must not carry suspensions to propagators in the clauses
// Structure of the built-in:
//   o check if variables got unified and if so propagate equality to
//     local variables
//   o constrain local variables with corresponding global ones
//   o check if unit commit, top commit, failure etc. occured and take
//     appropriate action
//   o propagate till you reach a fixpoint
//   o check if unit commit, top commit, failure etc. occured and take
//     appropriate action
//   o constrain global variables with union of corresponding local vars
//-----------------------------------------------------------------------------

OZ_Return CDPropagator::propagate(void)
{
  DEREF(b_tuple, b_tupleptr, b_tupletag);
  DEREF(v_tuple, v_tupleptr, v_tupletag);
  DEREF(vp_tuple, vp_tupleptr, vp_tupletag);

  SRecord &_b = *tagged2SRecord(b_tuple);
  SRecord &_v = *tagged2SRecord(v_tuple);
  SRecord &_vp = *tagged2SRecord(vp_tuple);

  const int clauses = _b.getWidth(); // number of clauses
  const int variables = _v.getWidth(); // number ob variables
  int failed_clauses = 0, not_failed_clause = -1, entailed_clause = -1;

  BIfdBodyManager x(0);

// introduction of variables

  // introduce Bs
  x.add(0, clauses);
  int c;
  for (c = clauses; c--; ) {
    x.introduce(idx_b(c), makeTaggedRef(&_b[c]));
  }

  // introduce Vs
  x.add(1, variables);
  int v;
  for (v = variables; v--; ) {
    x.introduce(idx_v(v), makeTaggedRef(&_v[v]));
  }

  // introduce Vps
  for (c = 0; c < clauses; c += 1) {  // acendingly counting ('cause of x.add)
    x.add(2 + c, variables);

    if (x[idx_b(c)] == 0) {
      for (v = variables; v--; )
        x.introduceDummy(idx_vp(c, v));
    } else {
      SRecord &vp_c = *tagged2SRecord(oz_deref(_vp[c]));
      for (v = variables; v--; )
        x.introduce(idx_vp(c, v), makeTaggedRef(&vp_c[v]));
    }
  }

// check if vars got unified and if so propagate equality to local vars
  // void variables become heap variables
  Assert(localPropStore.isEmpty());

  // propagating equality
  localPropStore.setUseIt();
  x.propagate_unify_cd(clauses, variables, _vp);
  localPropStore.unsetUseIt();

  int unified_vars = unifiedVars(variables, &_v[0]);

  for (c = clauses; c--; ) {
    if (x[idx_b(c)] == 0) {
      continue;
    } else for (v = variables; v--; )
      if (x.isNotCDVoid(idx_vp(c, v)))
        if ((x[idx_vp(c, v)] &= x[idx_v(v)]) == 0) {
          x[idx_b(c)] &= 0;  // intersection is empty --> clause failed
          break;
        }
  }

  localPropStore.setUseIt();

  for (c = clauses; c--; ) {
    if (x[idx_b(c)] == 0) {
      x.process(idx_b(c));

      x.backup();
      if (!do_cd_propagation(localPropStore))
        OZ_error("local propagation must be empty");
      x.restore();
      continue;
    }

    x.process(idx_b(c));

    Assert(x[idx_b(c)] != 0);

    for (v = variables; v--; )
      x.process(idx_vp(c, v));

    x.backup();
    if (!do_cd_propagation(localPropStore))
      OZ_error("local propagation must be empty");
    x.restore();

    Assert(localPropStore.isEmpty());

  }

  localPropStore.unsetUseIt();

// Note: since Bs and Vps are local, reintroduction is actual superfluous,
// since domains can get singletons and the according variable get disposed,
// we need to reintroduce Bs and Vps
  // reintroduce Bs
  for (c = clauses; c--; ) {
    x.reintroduce(idx_b(c), makeTaggedRef(&_b[c]));
  }

  // reintroduce Vps
  for (c = 0; c < clauses; c += 1) {  // acendingly counting ('cause of x.add)
    if (x[idx_b(c)] != 0) {
      SRecord &vp_c = *tagged2SRecord(oz_deref(_vp[c]));
      for (v = variables; v--; ) {
        x.reintroduce(idx_vp(c, v), makeTaggedRef(&vp_c[v]));
      }
    }
  }

// check if unit commit, top commit, failure occurs
  failed_clauses = 0;
  not_failed_clause = -1;
  entailed_clause = -1;

  for (c = clauses; c--; ) {
    if (x[idx_b(c)] == 0) {
      failed_clauses += 1;
    } else {
      not_failed_clause = c;

      if (x[idx_b(c)].getMaxElem() == 2) {
        Bool top_commit = TRUE;
        for (v = variables; v-- && top_commit; )
          if (x.isNotCDVoid(idx_vp(c, v)))
            top_commit &= (x[idx_vp(c, v)].getSize() == x[idx_v(v)].getSize());
        if (top_commit) {
          entailed_clause = c;
          break;
        }
      }
    }
  }
  if (failed_clauses == clauses) {                            // failure
#ifdef DEBUG_FDCD
    cout << "failure" << endl << flush;
#endif
    return FailFD;
  } else if ((clauses - failed_clauses) == 1) {               // unit commit

    for (v = variables; v--; )
      x[idx_v(v)] &= x[idx_vp(not_failed_clause, v)];
    x[idx_b(not_failed_clause)] &= 1;

    if (unified_vars < unifiedVars(variables, &(*tagged2SRecord(oz_deref(_vp[not_failed_clause])))[0])) {
      // imposed equality to global variables
      int * is = OZ_findEqualVars(variables, &_v[0]);
      for (int i = 0; i < variables; i += 1)
        if (is[i] >= 0 && is[i] != i)
          if (FAILED == OZ_unify(_v[i], _v[is[i]])) // mm_u
            OZ_error("Failure occured while commiting clause in constr disj.");
    }

#ifdef DEBUG_FDCD
    cout << "unit commit" << endl << flush;
#endif

    return x.entailmentClause(idx_b(0), idx_b(clauses - 1),
                              idx_v(0), idx_v(variables - 1),
                              idx_vp(not_failed_clause, 0),
                              idx_vp(not_failed_clause, variables - 1));
  } else if (entailed_clause != -1 &&
             unified_vars == unifiedVars(variables,
                                         &(*tagged2SRecord(oz_deref(_vp[entailed_clause])))[0])) {                         // top commit
    for (c = clauses; c--; )
      if (c != entailed_clause)
        x[idx_b(c)] &= 0;
    x[idx_b(entailed_clause)] &= 1;

#ifdef DEBUG_FDCD
    cout << "top commit" << endl << flush;
#endif

    return x.entailmentClause(idx_b(0), idx_b(clauses - 1));
  }


#ifdef DEBUG_FDCD
  x.printDebug();
#endif

// constrain global variables with union of corresponding local vars
  for (v = variables; v--; ) {
    int maxsize = 0, v_v_size = x[idx_v(v)].getSize();
    for (c = clauses; c-- && v_v_size < maxsize; )
      if (x[idx_b(c)] != 0)
        maxsize = max(maxsize, x[idx_vp(c, v)].getSize());

    if (maxsize < v_v_size) {
      OZ_FiniteDomain u(fd_empty);

      for (c = clauses; c--; ) {
        if (x[idx_b(c)] == 0) continue;
        u = u | x[idx_vp(c, v)];
      }
      x[idx_v(v)] &= u;
    }
  }

  DebugCode(for (c = clauses; c--; ) Assert(x[idx_b(c)] != 1);)

#ifdef DEBUG_FDCD
  x.printDebug();
#endif

  return x.releaseReify(idx_b(0), idx_b(clauses - 1),
                        idx_v(0), idx_v(variables - 1));
}

//-----------------------------------------------------------------------------
// Built-ins

OZ_Return cd_wrapper_b(int OZ_arity, OZ_Term OZ_args[],
                       OZ_CFun, OZ_CFun BI_body)
{
  int last_index = OZ_arity - 1;

  BIfdBodyManager x;

  x.introduce(OZ_args[last_index]);

  Assert(x[0] != 1); // clause cannot already be entailed

  if (x[0] == 0) {
    return EntailFD;
  }

  x.backup();
  OZ_Return ret_val = BI_body(OZ_args,OZ_ID_MAP);
  x.restore();

  Assert(x[0].getMaxElem() >= 2);
  if (ret_val == PROCEED) {
    x[0] <= (x[0].getMaxElem() - 1);
    Assert(x[0].getMaxElem() >= 2);
  } else {
    x[0] &= 0;
  }

  x.process(0);

  return EntailFD;
}

OZ_C_proc_proto(BIfdTellConstraint);

OZ_C_proc_begin(BIfdTellConstraintCD, 3)
{
  return cd_wrapper_b(OZ_arity, OZ_args, OZ_self, BIfdTellConstraint);
}
OZ_C_proc_end

//-----------------------------------------------------------------------------
// Propagators

#undef FailOnEmpty
#undef SimplifyOnUnify

#include "libfd/cd.hh"

OZ_C_proc_begin(BIfdConstrDisj, 3)
{
  OZ_EXPECTED_TYPE("tuple of finite domain,tuple of finite domain,"
                   "tuple of of tuple of finite domain");

  OZ_getCArgDeref(0, b_tuple, b_tupleptr, b_tupletag);
  OZ_getCArgDeref(1, v_tuple, v_tupleptr, v_tupletag);
  OZ_getCArgDeref(2, vp_tuple, vp_tupleptr, vp_tupletag);

  if (isLiteralTag(v_tupletag)) {
    SRecord &b = *tagged2SRecord(b_tuple);
    int b_size = b.getWidth();
    int ones = 0;

    for (int i = 0; i < b_size; i++) {
      OZ_Term b_i = b[i];
      DEREF(b_i, b_i_ptr, b_i_tag);
      int max_elem = isSmallIntTag(b_i_tag) ? smallIntValue(b_i) : tagged2GenFDVar(b_i)->getDom().getMaxElem();

      switch (max_elem) {
      case 2:
        ones += 1;
        break;
      case 0:
        break;
      default:
        DebugCode(OZ_warning("Unexpected value for controller variable, "
                             "in case no variable occured in clause."));
        break;
      }
    }
    return ones > 0 ? PROCEED : FAILED;
  }


  if (! oz_isSTuple(b_tuple) || ! oz_isSTuple(v_tuple) ||
      ! oz_isSTuple(vp_tuple)) {
    OZ_warning("Unexpected type in cd manager");
    return FAILED;
  }

  PropagatorExpect pe;
  OZ_EXPECT(pe, 1, expectVectorIntVarAny);

  return pe.impose(new CDPropagator(OZ_args[0], OZ_args[1], OZ_args[2]),
                   OZMAX_PRIORITY - 1);
}
OZ_C_proc_end

OZ_PropagatorProfile CDPropagator::spawner = "BIfdConstrDisj";

CDSuppl::CDSuppl(OZ_Propagator * p, OZ_Term b) : reg_b(b)
{
  prop = (void *) oz_newPropagator(OZ_getHighPrio(), p);
  // cd threads,  are expected to be suspended
  ((Propagator *) prop)->unmarkRunnable();
}

void CDSuppl::updateHeapRefs(OZ_Boolean) {
  prop = (void *) ((Propagator *) prop)->gcPropagatorOutlined();
  OZ_collectHeapTerm(reg_b, reg_b);
}

OZ_Return CDSuppl::propagate(void)
{
  OZ_FDIntVar b(reg_b);
  PropagatorController_V P(b);

  OZ_DEBUGPRINT(("cdsuppl.in: b=%s", b->toString()));

  if (*b == 0) {
    oz_closeDonePropagatorCD((Propagator *) prop);
    return PROCEED;
  }

  if (*b == 1) {
    OZ_Propagator * p = ((Propagator *) prop)->swapPropagator(this);
    oz_closeDonePropagatorThreadCD((Propagator *) prop);
    return replaceBy(p);
  }

  Propagator * backup_runningPropagator = Propagator::getRunningPropagator();
  Propagator::setRunningPropagator((Propagator *) prop);

  // propagate unify flag to actual propagator
  if (backup_runningPropagator->isUnifyPropagator()) {
    backup_runningPropagator->unmarkUnifyPropagator();
    ((Propagator *) prop)->markUnifyPropagator();
  }

  OZ_Return ret_val = oz_runPropagator((Propagator *) prop);

  Propagator::setRunningPropagator(backup_runningPropagator);

  OZ_ASSERT(b->getMaxElem() >= 2);

  if (ret_val == PROCEED) {
    oz_closeDonePropagatorCD((Propagator *) prop);
    *b <= (b->getMaxElem() - 1);
    OZ_ASSERT(b->getMaxElem() >= 2);
  } else if (ret_val == FAILED) {
    oz_closeDonePropagatorCD((Propagator *) prop);
    *b &= 0;
  }

  OZ_DEBUGPRINT(("cdsuppl.out: b=", b->toString()));

  P.vanish();
  return ret_val == FAILED ? PROCEED : ret_val;
}

// end of file
//-----------------------------------------------------------------------------

#endif
