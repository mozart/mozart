/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __FDBUILTIN_H__
#define __FDBUILTIN_H__

#ifdef __GNUC__
#pragma interface
#endif

#include "oz.h"

#include "am.hh"

#include "fdhook.hh"
#include "genvar.hh"
#include "fdprofil.hh"
#include "fdproto.hh"
#include "fdheads.hh"

enum Recalc_e {lower, upper};

//-----------------------------------------------------------------------------
// Debug Macros
//-----------------------------------------------------------------------------

#if defined(DEBUG_CHECK)
#define FORCE_ALL 0

#define FD_DEBUG_T(TEXT, SIZE, T, COND) \
if (FORCE_ALL || COND) { \
  cout << TEXT << endl; \
  for (int _i = 0; _i < SIZE; _i += 1) \
    cout << "x[" << _i << "]=", taggedPrint(T[_i]), cout << endl; \
  cout.flush(); \
}
#define FD_DEBUG_TTI(TEXT, SIZE, T1, T2, I, COND) \
if (FORCE_ALL || COND) { \
  cout << TEXT << endl; \
  int _i; \
  for (_i = 0; _i < SIZE; _i += 1) \
    cout << "a[" << _i << "]=", taggedPrint(T1[_i]), cout << endl; \
  for (_i = 0; _i < SIZE; _i += 1) \
    cout << "x[" << _i << "]=", taggedPrint(T2[_i]), cout << endl; \
  cout << "c=" << I  << endl; \
  cout.flush(); \
}
#define FD_DEBUG_ITI(TEXT, SIZE, I1, T, I2, COND) \
if (FORCE_ALL || COND) { \
  cout << TEXT << endl; \
  cout << "n=" << I1  << endl; \
  for (int _i = 0; _i < SIZE; _i += 1) \
    cout << "l[" << _i << "]=", taggedPrint(T[_i]), cout << endl; \
  cout << "v=" << I2  << endl; \
  cout.flush(); \
}
#define FD_DEBUG_XYZ(TEXT, X, Y, Z, COND) \
if (FORCE_ALL || COND) { \
  cout << TEXT << endl; \
  cout << "x="; taggedPrint(X); cout << endl; \
  cout << "y="; taggedPrint(Y); cout << endl; \
  cout << "z="; taggedPrint(Z); cout << endl; \
  cout.flush(); \
}
#define FD_DEBUG_XYC(TEXT, X, Y, C, COND) \
if (FORCE_ALL || COND) { \
  cout << TEXT << endl; \
  cout << "x="; taggedPrint(X); cout << endl; \
  cout << "y="; taggedPrint(Y); cout << endl; \
  cout << "c=" << C << endl; \
  cout.flush(); \
}
#define FD_DEBUG_XY(TEXT, X, Y, COND) \
if (FORCE_ALL || COND) { \
  cout << TEXT << endl; \
  cout << "x="; taggedPrint(X); cout << endl; \
  cout << "y="; taggedPrint(Y); cout << endl; \
  cout.flush(); \
}
#else
#define FD_DEBUG_T(TEXT, SIZE, T1, COND)
#define FD_DEBUG_TTI(TEXT, SIZE, T1, T2, I, COND)
#define FD_DEBUG_ITI(TEXT, SIZE, I1, T, I2, COND)
#define FD_DEBUG_XYZ(TEXT, X, Y, Z, COND)
#define FD_DEBUG_XYC(TEXT, X, Y, C, COND)
#define FD_DEBUG_XY(TEXT, X, Y, COND)
#endif

//-----------------------------------------------------------------------------
// Macros

#if PROFILE_FD == 1
#define FailFD (_PROFILE_CODE1(FDProfiles.inc_item(no_failed_props)), FAILED)
#define SuspendFD (_PROFILE_CODE1(FDProfiles.inc_item(no_susp_props)), PROCEED)
#define EntailFD (_PROFILE_CODE1(FDProfiles.inc_item(no_ent_props)), PROCEED)
#else
#define FailFD FAILED
#define SuspendFD PROCEED
#define EntailFD PROCEED
#endif

#define FailOnEmpty(X) \
{ \
  if((X) == 0) { \
    BIfdBodyManager::restoreDomainOnToplevel(); \
    PROFILE_CODE1(FDProfiles.inc_item(no_failed_props);) \
    return FailFD; \
  } \
}


#define SimplifyOnUnify(EQ01, EQ02, EQ12) \
  if (isUnifyCurrentTaskSusp()) { \
    OZ_getCArgDeref(0, x, xPtr, xTag); \
    OZ_getCArgDeref(1, y, yPtr, yTag); \
    if (xPtr == yPtr && isAnyVar(xTag)) { \
      killPropagatedCurrentTaskSusp(); \
      return (EQ01); \
    } \
    OZ_getCArgDeref(2, z, zPtr, zTag); \
    if (xPtr == zPtr && isAnyVar(xTag)) { \
      killPropagatedCurrentTaskSusp(); \
      return (EQ02); \
    } \
    if (yPtr == zPtr && isAnyVar(yTag)) { \
      killPropagatedCurrentTaskSusp(); \
      return (EQ12); \
    } \
  }


enum pm_term_type {pm_none = 0x0, pm_singl = 0x1,
                   pm_bool = 0x2, pm_fd = 0x4,
                   pm_svar = 0x8, pm_uvar = 0x10,
                   pm_tuple = 0x20, pm_literal = 0x40};

inline
Bool pm_is_var(pm_term_type t) {
  return t & (pm_bool | pm_fd | pm_svar | pm_uvar);
}

inline
Bool pm_is_noncvar(pm_term_type t) {
  return t & (pm_svar | pm_uvar);
}

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
TaggedRef deref(TaggedRef &tr, TaggedRef * &ptr, pm_term_type &tag)
{
  TaggedRef tr1=tr;
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
  case STUPLE: tag = pm_tuple; break;
  case LITERAL: tag = pm_literal; break;
  default: tag = pm_none; break;
  }
  return tr1;
}

//-----------------------------------------------------------------------------
// Global Variables relenvant for FD Built-ins

extern double static_coeff_double[MAXFDBIARGS];
extern int static_coeff_int[MAXFDBIARGS];
extern Bool static_sign_bit[MAXFDBIARGS];
extern TaggedRef static_var[MAXFDBIARGS];
extern TaggedRefPtr static_varptr[MAXFDBIARGS];
extern pm_term_type static_vartag[MAXFDBIARGS];
extern Bool static_bool_a[MAXFDBIARGS];
extern Bool static_bool_b[MAXFDBIARGS];
extern int static_int_a[MAXFDBIARGS];
extern int static_int_b[MAXFDBIARGS];
extern double static_double_a[MAXFDBIARGS];
extern double static_double_b[MAXFDBIARGS];
extern int static_index_offset[MAXFDBIARGS];
extern int static_index_size[MAXFDBIARGS];


//-----------------------------------------------------------------------------
// Auxiliary stuff
//-----------------------------------------------------------------------------

inline
Bool isPosSmallInt(TaggedRef val)
{
  if (isSmallInt(val))
    return smallIntValue(val) >= 0;

  return FALSE;
}

inline
Bool isBoolSmallInt(TaggedRef val)
{
  if (isSmallInt(val)) {
    int ival = smallIntValue(val);
    return (ival == 0 || ival == 1);
  }
  return NO;
}

inline
TaggedRef * allocateRegs(TaggedRef t1, TaggedRef t2)
{
  TaggedRef * a = (TaggedRef *) heapMalloc(2 * sizeof(TaggedRef));
  a[0] = t1;
  a[1] = t2;
  return a;
}

inline
TaggedRef * allocateRegs(TaggedRef t1, TaggedRef t2, TaggedRef t3)
{
  TaggedRef * a = (TaggedRef *) heapMalloc(3 * sizeof(TaggedRef));
  a[0] = t1;
  a[1] = t2;
  a[2] = t3;
  return a;
}

inline
TaggedRef * allocateRegs(TaggedRef t1, TaggedRef t2, TaggedRef t3,
                         TaggedRef t4)
{
  TaggedRef * a = (TaggedRef *) heapMalloc(4 * sizeof(TaggedRef));
  a[0] = t1;
  a[1] = t2;
  a[2] = t3;
  a[3] = t4;
  return a;
}

inline
TaggedRef * allocateRegs(TaggedRef t1, TaggedRef t2, TaggedRef t3,
                         TaggedRef t4, TaggedRef t5)
{
  TaggedRef * a = (TaggedRef *) heapMalloc(5 * sizeof(TaggedRef));
  a[0] = t1;
  a[1] = t2;
  a[2] = t3;
  a[3] = t4;
  a[4] = t5;
  return a;
}

inline
TaggedRef * allocateRegs(TaggedRef t1, TaggedRef t2, TaggedRef t3,
                         TaggedRef t4, TaggedRef t5, TaggedRef t6)
{
  TaggedRef * a = (TaggedRef *) heapMalloc(6 * sizeof(TaggedRef));
  a[0] = t1;
  a[1] = t2;
  a[2] = t3;
  a[3] = t4;
  a[4] = t5;
  a[5] = t6;
  return a;
}

inline
TaggedRef * allocateRegs(TaggedRef t1, TaggedRef t2, TaggedRef t3,
                         TaggedRef t4, TaggedRef t5, TaggedRef t6,
                         TaggedRef t7, TaggedRef t8, TaggedRef t9,
                         TaggedRef t10, TaggedRef t11)
{
  TaggedRef * a = (TaggedRef *) heapMalloc(11 * sizeof(TaggedRef));
  a[0] = t1;
  a[1] = t2;
  a[2] = t3;
  a[3] = t4;
  a[4] = t5;
  a[5] = t6;
  a[6] = t7;
  a[7] = t8;
  a[8] = t9;
  a[9] = t10;
  a[10] = t11;
  return a;
}

// return TRUE if i is not negative
inline
Bool getSign(int i) {
  return i >= 0;
}

inline
Bool getSign(double  i) {
  return i >= 0.0;
}

inline
void getSignbit(int i, int n) {
  static_sign_bit[i] = getSign(n);
}

inline
void getDoubleCoeff(int i, TaggedRef v) {
  int n =  smallIntValue(deref(v));
  static_coeff_double[i] = (double) n;
  getSignbit(i, n);
}

inline
void getIntCoeff(int i, TaggedRef v) {
  int n =  smallIntValue(deref(v));
  static_coeff_int[i] = n;
  getSignbit(i, n);
}

//-----------------------------------------------------------------------------
// Suspension Creation Functions

inline
Suspension * createNonResSusp(OZ_CFun func, RefsArray xregs, int arity)
{
  return (Suspension *) OZ_makeThread(func, xregs, arity);
}


inline
Suspension * createResSusp(OZ_CFun func, int arity, RefsArray xregs)
{
  Suspension * s = makeHeadSuspension(func, xregs, arity);

  s->headInit();

  Assert(FDcurrentTaskSusp == NULL);

  FDcurrentTaskSusp = s;
  return s;
}

//-----------------------------------------------------------------------------
// fd built-ins don't cause stuck threads, but may cause their minds, therefore

#undef FDBISTUCK

#ifdef FDBISTUCK

#define SUSPEND_PROCEED SUSPEND

#else

#define SUSPEND_PROCEED PROCEED

#endif

//-----------------------------------------------------------------------------
//                          class BIfdHeadManager
//-----------------------------------------------------------------------------

class BIfdHeadManager {
private:
  static TaggedRef * bifdhm_var;
  static TaggedRefPtr * bifdhm_varptr;
  static pm_term_type * bifdhm_vartag;
  static int * bifdhm_coeff;
  static int curr_num_of_items;

  int simplifyHead(int ts, STuple &a, STuple &x);

  int global_vars;

  void addResSusp(int i, Suspension * susp, FDPropState target);
  void addResSusps(Suspension * susp, FDPropState target);

public:
  BIfdHeadManager(int s) : global_vars(0) {
    DebugCheck(s < 0 || s > MAXFDBIARGS, error("too many items"));
    curr_num_of_items = s;
  }

  void increaseSizeBy(int s) {
    curr_num_of_items += s;
    DebugCheck(curr_num_of_items < 0 || curr_num_of_items > MAXFDBIARGS,
               error("too many items"));
  }

  static
  void initStaticData(void);

  Bool expectFDish(int i, TaggedRef v, int &s);
  Bool expectInt(int i, TaggedRef v, int &s);
  Bool expectNonLin(int i, STuple &at, STuple &xt, TaggedRef tagged_xtc,
                    int &s, OZ_CFun func, RefsArray xregs, int arity);

  OZ_Bool addSuspFDish(OZ_CFun, RefsArray, int);
  OZ_Bool addSuspSingl(OZ_CFun, RefsArray, int);
  Bool addSuspXorYdet(OZ_CFun, RefsArray, int);

  int simplify(STuple &a, STuple &x) {
    return curr_num_of_items = simplifyHead(curr_num_of_items, a, x);
  }
  int allOne(void) {
    for (int i = curr_num_of_items; i--; )
      if (bifdhm_coeff[i] != 1 && bifdhm_coeff[i] != -1)
        return FALSE;
    return TRUE;
  }

  int getCoeff(int i) {
    DebugCheck(i < 0 || i >= curr_num_of_items, error("index overflow"));
    return bifdhm_coeff[i];
  }
  pm_term_type getTag(int i) {
    DebugCheck(i < 0 || i >= curr_num_of_items, error("index overflow"));
    return bifdhm_vartag[i];
  }
  int getCurrNumOfItems(void) {return curr_num_of_items;}

  OZ_Bool spawnPropagator(FDPropState, OZ_CFun, int, OZ_Term *);
  OZ_Bool spawnPropagator(FDPropState, FDPropState, OZ_CFun, int, OZ_Term *);
  OZ_Bool spawnPropagator(FDPropState, OZ_CFun, int, OZ_Term, ...);

  Bool areIdentVar(int a, int b) {
    DebugCheck((a < 0 || a >= curr_num_of_items) ||
               (b < 0 || b >= curr_num_of_items),
               error("index overflow."));
    return (bifdhm_varptr[a] == bifdhm_varptr[b] &&
            isAnyVar(bifdhm_var[a]));
  }

  void printDebug(void) {
    for (int i = 0; i < curr_num_of_items; i += 1)
      printDebug(i);
  }
  void printDebug(int i) {
    cerr << '[' << i << "]: var=" << (void *) bifdhm_var[i]
         << ", varptr=" << (void *) bifdhm_varptr[i]
         << ", vartag=" << pm_term_type2string(bifdhm_vartag[i])
         << ", coeff=" << bifdhm_coeff[i] << endl;
    cerr.flush();
  }
};


//-----------------------------------------------------------------------------
//                           class BIfdBodyManager
//-----------------------------------------------------------------------------

enum fdbm_var_state {fdbm_local, fdbm_global, fdbm_speculative};
inline
char * fdbm_var_stat2char(fdbm_var_state s) {
  static char * fdbm_var_state_names[3] = {"local", "global", "speculative"};
  return fdbm_var_state_names[s];
}

inline
int idx(int i, int j) {
  Assert(0 <= j && j < static_index_size[i]);
  Assert(0 <= static_index_offset[i] + j &&
         static_index_offset[i] + j < MAXFDBIARGS);
  return static_index_offset[i] + j;
}

inline int idx_b(int i) { return idx(0, i); }
inline int idx_v(int i) { return idx(1, i); }
inline int idx_vp(int c, int v) { return idx(2+c, v); }

extern FiniteDomain __CDVoidFiniteDomain;

class BIfdBodyManager {
private:
// data slots in charge
  static TaggedRef * bifdbm_var;
  static TaggedRefPtr * bifdbm_varptr;
  static pm_term_type * bifdbm_vartag;

  static FiniteDomainPtr * bifdbm_dom;
  static FiniteDomain * bifdbm_domain;

  static int curr_num_of_vars;
  static int * bifdbm_init_dom_size;
  static int * cache_from;
  static int * cache_to;
  static int * index_offset;
  static int * index_size;

  static Bool vars_left;
  static Bool only_local_vars;
  static fdbm_var_state * bifdbm_var_state;

// backup data slots
  int backup_count;
  int backup_curr_num_of_vars1;
  Bool backup_vars_left1;
  Bool backup_only_local_vars1;
  Suspension * backup_FDcurrentTaskSusp1;

// private methods
  Bool isTouched(int i) {
    return bifdbm_init_dom_size[i] > bifdbm_dom[i]->getSize() ||
      bifdbm_vartag[i] == pm_svar;
  }

  void process(void);
  void processFromTo(int, int);
  void processLocal(void) {
    processLocalFromTo(0, curr_num_of_vars);
  }

  void processLocalFromTo(int, int);
  void processNonRes(void);

  void _introduce(int i, TaggedRef v);
  void introduceLocal(int i, TaggedRef v);
  void saveDomainOnTopLevel(int i) {
    if (am.currentBoard->isRoot()) {
      if (bifdbm_vartag[i] == pm_fd)
        bifdbm_domain[i] = tagged2GenFDVar(bifdbm_var[i])->getDom();
    }
  }
  int simplifyBody(int ts, STuple &a, STuple &x,
                   Bool sign_bits[], double coeffs[]);
  void _propagate_unify_cd(int clauses, int variables, STuple &st);

  enum {cache_slot_size = 4};

  void setSpeculative(int i);
public:
  BIfdBodyManager(int s) {
    DebugCode(backup_count = 0;)
    if (s == -1) {
      curr_num_of_vars = 0;
      only_local_vars = FALSE;
    } else {
      DebugCheck(s < 0 || s > MAXFDBIARGS, error("too many variables."));
      curr_num_of_vars = s;
      Assert(FDcurrentTaskSusp);
      only_local_vars = FDcurrentTaskSusp->isLocalSusp();
    }
  }

  DebugCode(~BIfdBodyManager() {if (backup_count) error("backup_count!=0");})

  Bool setCurr_num_of_vars(int i) {
    if (i < 0 || i > MAXFDBIARGS)
      return TRUE;
    curr_num_of_vars = i;
   return FALSE;
  }

  Bool indexIsInvalid(int i) {return (i < 0) || (i >= curr_num_of_vars);}

  void add(int i, int size) {
    curr_num_of_vars += size;
    DebugCheck(curr_num_of_vars < 0 || curr_num_of_vars > MAXFDBIARGS,
               error("too many variables."));
    if (i == 0) index_offset[0] = 0;
    index_offset[i + 1] = index_offset[i] + size;
    index_size[i] = size;
  }

  int getCurrNumOfVars(void) {return curr_num_of_vars;}

  int initCache(void);
  int getCacheSlotFrom(int i) {return cache_from[i];}
  int getCacheSlotTo(int i) {return cache_to[i];}

  Bool allVarsAreLocal(void) {return only_local_vars;}

  static
  void initStaticData(void);

  void backup(void);
  void restore(void);

  FiniteDomain &operator [](int i) {
    DebugCheck(i < 0 || i >= curr_num_of_vars, error("index overflow."));
    return *bifdbm_dom[i];
  }

  FiniteDomain &operator ()(int i, int j) {
    return operator [](idx(i, j));
  }

  void printDebug(void) {
    for (int i = 0; i < curr_num_of_vars; i += 1)
      printDebug(i);
  }

  void printDebug(int i) {
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

  void printTerm(void) {
    for (int i = 0; i < curr_num_of_vars; i += 1)
      printTerm(i);
  }

  void printTerm(int i) {
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

  void introduceDummy(int i) { bifdbm_dom[i] = NULL; }

  void introduce(int i, TaggedRef v) {
    if (only_local_vars) {
      introduceLocal(i, v);
    } else {
      _introduce(i, v);
    }
    // if current board is the top-level then save domains for
    // restoration on failure
    saveDomainOnTopLevel(i);
  }

  void introduceSpeculative(int i, TaggedRef v);

  OZ_Bool checkAndIntroduce(int i, TaggedRef v);

  void reintroduce(int i, TaggedRef v) {
    int aux = bifdbm_init_dom_size[i];
    introduce(i, v);
    bifdbm_init_dom_size[i] = aux;
  }

  void introduce(int i, int j, TaggedRef v) {
    int index = idx(i, j);
    Assert(index_offset[i] <= index && index < index_offset[i + 1]);
    if (only_local_vars) {
      introduceLocal(index, v);
    } else {
      _introduce(index, v);
    }
    saveDomainOnTopLevel(index);
  }

  void process(int i) {processFromTo(i, i+1);}

  OZ_Bool entailment(void) {
    if (only_local_vars) {
      processLocal();
    } else {
      process();
    }
    return EntailFD;
  }

  OZ_Bool entailmentClause(int from_b, int to_b,
                           int from, int to,
                           int from_p, int to_p);

  OZ_Bool entailmentClause(int from_b, int to_b) {
    processLocalFromTo(from_b, to_b+1);

    return EntailFD;
  }

  OZ_Bool release(int from, int to) {

    if (only_local_vars) {
      processLocalFromTo(from, to+1);
      if (vars_left) reviveCurrentTaskSusp();
    } else {
      processFromTo(from, to+1);
      if (vars_left)
        reviveCurrentTaskSusp();
    }

    return vars_left ? SuspendFD : EntailFD;
  }

  OZ_Bool releaseReify(int from_b, int to_b, int from, int to) {

    processLocalFromTo(from_b, to_b+1);

    return release(from, to);
  }

  OZ_Bool release(void) {
    return release(0, curr_num_of_vars - 1);
  }

  OZ_Bool release1(void) { // used by square and twice
    process();
    return EntailFD;
  }

  OZ_Bool releaseNonRes(void) { // used by putList, putNot, putLe, putGe
    processNonRes();
    return EntailFD;
  }

  int simplifyOnUnify(STuple &a, Bool sign_bits[], double coeffs[], STuple &x) {
    if (isUnifyCurrentTaskSusp())
      curr_num_of_vars =
        simplifyBody(curr_num_of_vars, a, x, sign_bits, coeffs);
    return curr_num_of_vars;
  }

  int simplifyOnUnify(int ts, STuple &a, Bool sign_bits[], double coeffs[],
                      STuple &x) {
    if (isUnifyCurrentTaskSusp()) {
      Assert(curr_num_of_vars >= ts);
      int new_ts = simplifyBody(ts, a, x, sign_bits, coeffs);
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

  Bool _unifiedVars(void);
  Bool unifiedVars(void) {
    if (! isUnifyCurrentTaskSusp()) return FALSE;
    return _unifiedVars();
  }

  void propagate_unify_cd(int cl, int vars, STuple &st) {
    if (isUnifyCurrentTaskSusp())
      _propagate_unify_cd(cl, vars, st);
  }

  Bool isNotCDVoid(int i) {return bifdbm_dom[i] != &__CDVoidFiniteDomain;}

  Bool areIdentVar(int a, int b) {
    DebugCheck((a < 0 || a >= curr_num_of_vars) ||
               (b < 0 || b >= curr_num_of_vars),
               error("index overflow."));
    if (! isUnifyCurrentTaskSusp()) return FALSE;
    return bifdbm_varptr[a] == bifdbm_varptr[b] && isAnyVar(bifdbm_var[a]);
  }

  FiniteDomainPtr * getDoms(void) {return bifdbm_dom;}

// exactly one variable is regarded
  BIfdBodyManager(void) {backup_count = 0; curr_num_of_vars = 1;}

  Bool introduce(TaggedRef v);

  FiniteDomain &operator *(void) {return *bifdbm_dom[0];}

  static void restoreDomainOnToplevel(void) {
    if (am.currentBoard->isRoot()) {
      for (int i = curr_num_of_vars; i--; )
        if (bifdbm_vartag[i] == pm_fd)
          tagged2GenFDVar(bifdbm_var[i])->getDom() = *bifdbm_dom[i];
    }
  }
}; // BIfdBodyManager


//-----------------------------------------------------------------------------

OZ_Bool checkDomDescr(OZ_Term descr,
                      OZ_CFun cfun, OZ_Term * args, int arity,
                      int expect = 3);

#if !defined(OUTLINE) && !defined(FDOUTLINE)
#include "fdbuilti.icc"
#endif


#endif
