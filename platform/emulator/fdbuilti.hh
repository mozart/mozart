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
#include "builtins.hh"
#include "fdhook.hh"
#include "genvar.hh"
#include "fdprofil.hh"

enum Recalc_e {lower, upper};

//-----------------------------------------------------------------------------
// Debug Macros
//-----------------------------------------------------------------------------

#if defined(DEBUG_CHECK) || 1
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
      FDcurrentTaskSusp->markDead(); \
      FDcurrentTaskSusp = NULL; \
      return (EQ01); \
    } \
    OZ_getCArgDeref(2, z, zPtr, zTag); \
    if (xPtr == zPtr && isAnyVar(xTag)) { \
      FDcurrentTaskSusp->markDead(); \
      FDcurrentTaskSusp = NULL; \
      return (EQ02); \
    } \
    if (yPtr == zPtr && isAnyVar(yTag)) { \
      FDcurrentTaskSusp->markDead(); \
      FDcurrentTaskSusp = NULL; \
      return (EQ12); \
    } \
  }

//-----------------------------------------------------------------------------
// Global Variables relenvant for FD Built-ins

extern float static_coeff_float[MAXFDBIARGS];
extern int static_coeff_int[MAXFDBIARGS];
extern Bool static_sign_bit[MAXFDBIARGS];
extern TaggedRef static_var[MAXFDBIARGS];
extern TaggedRefPtr static_varptr[MAXFDBIARGS];
extern TypeOfTerm static_vartag[MAXFDBIARGS];
extern Bool static_bool_a[MAXFDBIARGS];
extern Bool static_bool_b[MAXFDBIARGS];
extern int static_int_a[MAXFDBIARGS];
extern int static_int_b[MAXFDBIARGS];
extern float static_float_a[MAXFDBIARGS];
extern float static_float_b[MAXFDBIARGS];
extern int static_index_offset[MAXFDBIARGS];
extern int static_index_size[MAXFDBIARGS];


//-----------------------------------------------------------------------------
//                         Prototypes of FD-built-ins
//-----------------------------------------------------------------------------

// fdprof.cc
OZ_C_proc_proto(BIfdReset)
OZ_C_proc_proto(BIfdDiscard)
OZ_C_proc_proto(BIfdGetNext)
OZ_C_proc_proto(BIfdPrint)
OZ_C_proc_proto(BIfdTotalAverage)

// fdrel.cc
OZ_C_proc_proto(BIfdMinimum)
OZ_C_proc_proto(BIfdMaximum)
OZ_C_proc_proto(BIfdUnion)
OZ_C_proc_proto(BIfdLessEqOff)
OZ_C_proc_proto(BIfdNotEqEnt)
OZ_C_proc_proto(BIfdNotEq)
OZ_C_proc_proto(BIfdNotEqOff)
OZ_C_proc_proto(BIfdNotEqOffEnt)
OZ_C_proc_proto(BIfdAllDifferent)

// fdarith.cc
OZ_C_proc_proto(BIfdPlus)
OZ_C_proc_proto(BIfdMinus)
OZ_C_proc_proto(BIfdMult)
OZ_C_proc_proto(BIfdDiv)
OZ_C_proc_proto(BIfdMod)
OZ_C_proc_proto(BIfdPlus_rel)
OZ_C_proc_proto(BIfdMult_rel)

// fdbool.cc
OZ_C_proc_proto(BIfdAnd)
OZ_C_proc_proto(BIfdOr)
OZ_C_proc_proto(BIfdNot)
OZ_C_proc_proto(BIfdXor)
OZ_C_proc_proto(BIfdEquiv)
OZ_C_proc_proto(BIfdImpl)

// fdgeneric.cc
OZ_C_proc_proto(BIfdGenLinEq)
OZ_C_proc_proto(BIfdGenNonLinEq)
OZ_C_proc_proto(BIfdGenNonLinEq1)
OZ_C_proc_proto(BIfdGenLinNotEq)
OZ_C_proc_proto(BIfdGenNonLinNotEq)
OZ_C_proc_proto(BIfdGenLinLessEq)
OZ_C_proc_proto(BIfdGenNonLinLessEq)
OZ_C_proc_proto(BIfdGenNonLinLessEq1)

// fdcount.cc
OZ_C_proc_proto(BIfdElement)
OZ_C_proc_proto(BIfdElement_body)
OZ_C_proc_proto(BIfdAtMost)
OZ_C_proc_proto(BIfdAtMost_body)
OZ_C_proc_proto(BIfdAtLeast)
OZ_C_proc_proto(BIfdAtLeast_body)
OZ_C_proc_proto(BIfdCount)
OZ_C_proc_proto(BIfdCount_body)

// fdcard.cc
OZ_C_proc_proto(BIfdGenLinEqB)
OZ_C_proc_proto(BIfdGenNonLinEqB)
OZ_C_proc_proto(BIfdGenLinNotEqB)
OZ_C_proc_proto(BIfdGenNonLinNotEqB)
OZ_C_proc_proto(BIfdGenLinLessEqB)
OZ_C_proc_proto(BIfdGenNonLinLessEqB)
OZ_C_proc_proto(BIfdInB)
OZ_C_proc_proto(BIfdNotInB)
OZ_C_proc_proto(BIfdCardBIBin)
OZ_C_proc_proto(BIfdCardNestableBI)
OZ_C_proc_proto(BIfdCardNestableBIBin)

// fdcore.cc
OZ_C_proc_proto(BIisFdVar)
OZ_C_proc_proto(BIfdIs)
State BIfdIsInline(TaggedRef);
OZ_C_proc_proto(BIgetFDLimits)
OZ_C_proc_proto(BIfdMin)
OZ_C_proc_proto(BIfdMax)
OZ_C_proc_proto(BIfdGetAsList)
OZ_C_proc_proto(BIfdGetCardinality)
OZ_C_proc_proto(BIfdNextTo)
OZ_C_proc_proto(BIfdCopyDomain)
OZ_C_proc_proto(BIfdPutLe)
OZ_C_proc_proto(BIfdPutGe)
OZ_C_proc_proto(BIfdPutList)
OZ_C_proc_proto(BIfdPutInterval)
OZ_C_proc_proto(BIfdPutNot)

// fdcd.cc
OZ_C_proc_proto(BIfdConstrDisjSetUp)
OZ_C_proc_proto(BIfdConstrDisj)
OZ_C_proc_proto(BIfdConstrDisj_body)

OZ_C_proc_proto(BIfdGenLinEqCD)
OZ_C_proc_proto(BIfdGenNonLinEqCD)
OZ_C_proc_proto(BIfdGenLinEqCD_body)
OZ_C_proc_proto(BIfdGenLinLessEqCD)
OZ_C_proc_proto(BIfdGenNonLinLessEqCD)
OZ_C_proc_proto(BIfdGenLinLessEqCD_body)
OZ_C_proc_proto(BIfdGenLinNotEqCD)
OZ_C_proc_proto(BIfdGenNonLinNotEqCD)
OZ_C_proc_proto(BIfdGenLinNotEqCD_body)

OZ_C_proc_proto(BIfdPlusCD_rel)
OZ_C_proc_proto(BIfdPlusCD_rel_body)
OZ_C_proc_proto(BIfdMultCD_rel)
OZ_C_proc_proto(BIfdMultCD_rel_body)

OZ_C_proc_proto(BIfdLessEqOffCD)
OZ_C_proc_proto(BIfdLessEqOffCD_body)
OZ_C_proc_proto(BIfdNotEqCD)
OZ_C_proc_proto(BIfdNotEqCD_body)
OZ_C_proc_proto(BIfdNotEqOffCD)
OZ_C_proc_proto(BIfdNotEqOffCD_body)

OZ_C_proc_proto(BIfdPutLeCD)
OZ_C_proc_proto(BIfdPutGeCD)
OZ_C_proc_proto(BIfdPutListCD)
OZ_C_proc_proto(BIfdPutIntervalCD)
OZ_C_proc_proto(BIfdPutNotCD)

// fdmisc.cc
OZ_C_proc_proto(BIfdCardSched)
OZ_C_proc_proto(BIfdCDSched)
OZ_C_proc_proto(BIfdNoOverlap)
OZ_C_proc_proto(BIfdCardBIKill)
OZ_C_proc_proto(BIfdInKillB)
OZ_C_proc_proto(BIfdCardBIKill)
OZ_C_proc_proto(BIfdInKillB)
OZ_C_proc_proto(BIfdNotInKillB)
OZ_C_proc_proto(BIfdGenLinEqKillB)
OZ_C_proc_proto(BIfdGenLinLessEqKillB)

// fdwatch.cc
OZ_C_proc_proto(BIfdWatchDom1)
OZ_C_proc_proto(BIfdWatchDom2)
OZ_C_proc_proto(BIfdWatchDom3)
OZ_C_proc_proto(BIfdWatchBounds1)
OZ_C_proc_proto(BIfdWatchBounds2)
OZ_C_proc_proto(BIfdWatchBounds3)

// body prototypes
OZ_C_proc_proto(BIfdPlus_body)
OZ_C_proc_proto(BIfdTwice_body)
OZ_C_proc_proto(BIfdMinus_body)
OZ_C_proc_proto(BIfdMult_body)
OZ_C_proc_proto(BIfdSquare_body)
OZ_C_proc_proto(BIfdDiv_body)
OZ_C_proc_proto(BIfdMod_body)
OZ_C_proc_proto(BIfdAnd_body)
OZ_C_proc_proto(BIfdOr_body)
OZ_C_proc_proto(BIfdNot_body)
OZ_C_proc_proto(BIfdXor_body)
OZ_C_proc_proto(BIfdEquiv_body)
OZ_C_proc_proto(BIfdImpl_body)
OZ_C_proc_proto(BIfdGenLinEq_body)
OZ_C_proc_proto(BIfdGenNonLinEq_body)
OZ_C_proc_proto(BIfdGenLinNotEq_body)
OZ_C_proc_proto(BIfdGenNonLinNotEq_body)
OZ_C_proc_proto(BIfdGenLinLessEq_body)
OZ_C_proc_proto(BIfdGenNonLinLessEq_body)
OZ_C_proc_proto(BIfdGenLinNotEq_body)
OZ_C_proc_proto(BIfdGenLinLessEq_body)
OZ_C_proc_proto(BIfdLessEqOff_body)
OZ_C_proc_proto(BIfdNotEqOffEnt_body)
OZ_C_proc_proto(BIfdCardSched_body)
OZ_C_proc_proto(BIfdCDSched_body)
OZ_C_proc_proto(BIfdDisjunction)
OZ_C_proc_proto(BIfdDisjunction_body)
OZ_C_proc_proto(BIfdNoOverlap_body)
OZ_C_proc_proto(BIfdAllDifferent_body)
OZ_C_proc_proto(BIfdLessEqOff_body)
OZ_C_proc_proto(BIfdNotEqEnt_body)
OZ_C_proc_proto(BIfdNotEq_body)
OZ_C_proc_proto(BIfdNotEqOffEnt_body)
OZ_C_proc_proto(BIfdNotEqOff_body)
OZ_C_proc_proto(BIfdMaximum_body)
OZ_C_proc_proto(BIfdMinimum_body)
OZ_C_proc_proto(BIfdSubsume_body)
OZ_C_proc_proto(BIfdUnion_body)
OZ_C_proc_proto(BIfdGenLinEqB_body)
OZ_C_proc_proto(BIfdGenNonLinEqB_body)
OZ_C_proc_proto(BIfdGenLinNotEqB_body)
OZ_C_proc_proto(BIfdGenNonLinNotEqB_body)
OZ_C_proc_proto(BIfdGenLinLessEqB_body)
OZ_C_proc_proto(BIfdGenLinLessEqCD_body)
OZ_C_proc_proto(BIfdGenNonLinLessEqB_body)
OZ_C_proc_proto(BIfdGenNonLinLessEqCD_body)
OZ_C_proc_proto(BIfdCardBIKill_body)
OZ_C_proc_proto(BIfdCardBIBin_body)
OZ_C_proc_proto(BIfdInB_body)
OZ_C_proc_proto(BIfdInKillB_body)
OZ_C_proc_proto(BIfdNotInB_body)
OZ_C_proc_proto(BIfdCardNestableBI_body)
OZ_C_proc_proto(BIfdCardBIKill_body)
OZ_C_proc_proto(BIfdCardNestableBIBin_body)
OZ_C_proc_proto(BIfdInKillB_body)
OZ_C_proc_proto(BIfdNotInKillB_body)
OZ_C_proc_proto(BIfdGenLinEqKillB_body)
OZ_C_proc_proto(BIfdGenLinLessEqKillB_body)

//-----------------------------------------------------------------------------
// Prototypes for Heads of Built-ins
//-----------------------------------------------------------------------------

OZ_Bool genericHead_a_x_c(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			  OZ_CFun BI_body, FDPropState target_list);

OZ_Bool genericHead_a_x_c_y(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			    OZ_CFun BI_body, FDPropState target_list);

OZ_Bool genericHead_a_x_c_nl(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			     OZ_CFun BI_body, FDPropState target_list);
OZ_Bool genericHead_a_x_c_nl1(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			      OZ_CFun BI_body, FDPropState target_list);

OZ_Bool genericHead_x_y_z(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			  OZ_CFun BI_body, Bool nestable,
			  FDPropState target_list  = fd_bounds);

OZ_Bool genericHead_x_y_z_det_x_or_y(int OZ_arity, OZ_Term OZ_args[],
				     OZ_CFun OZ_self, OZ_CFun BI_body);

OZ_Bool genericHead_x_y_c(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			  OZ_CFun BI_body, FDPropState target_list);

OZ_Bool genericHead_x_y(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			OZ_CFun BI_body, FDPropState target_list,
			Bool nestable = FALSE);


OZ_Bool genericHead_a_x_c_b(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			    OZ_CFun BI_body, FDPropState target_list);

OZ_Bool genericHead_a_x_c_b_nl(int OZ_arity, OZ_Term OZ_args[],
			       OZ_CFun OZ_self, OZ_CFun BI_body,
			       FDPropState target_list);
OZ_Bool genericHead_x_y_z_b(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			    OZ_CFun BI_body,
			    FDPropState target_list  = fd_bounds);
OZ_Bool genericHead_x_y_z_det_x_or_y_b(int OZ_arity, OZ_Term OZ_args[],
				       OZ_CFun OZ_self, OZ_CFun BI_body);
OZ_Bool genericHead_x_y_c_b(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			    OZ_CFun BI_body, FDPropState target_list);
OZ_Bool genericHead_x_y_b(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			  OZ_CFun BI_body, FDPropState target_list);


OZ_Bool genericHead_x_c_d(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			  OZ_CFun BI_body, FDPropState target_list);

OZ_Bool genericHead_x_D_d(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			  OZ_CFun BI_body, FDPropState target_list);

OZ_Bool genericHead_x_c_d_e(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
			    OZ_CFun BI_body, FDPropState target_list);

OZ_Bool genericHead_cd(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
		       OZ_CFun BI_body, FDPropState target_list);

OZ_Bool genericHead_cdIntersect(int OZ_arity, OZ_Term OZ_args[], OZ_CFun OZ_self,
				OZ_CFun BI_body, FDPropState target_list);



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
void getSignbit(int i, int n) {
  static_sign_bit[i] = getSign(n);
}

inline
void getFloatCoeff(int i, TaggedRef v) {
  int n =  smallIntValue(deref(v));
  static_coeff_float[i] = (float) n;
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
  return (Suspension *) OZ_makeSuspension(func, xregs, arity);
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
//                          class BIfdHeadManager
//-----------------------------------------------------------------------------

class BIfdHeadManager {
private:
  static TaggedRef * bifdhm_var;
  static TaggedRefPtr * bifdhm_varptr;
  static TypeOfTerm * bifdhm_vartag;
  static int * bifdhm_coeff;
  static int curr_num_of_items;
  
  int simplifyHead(int ts, STuple &a, STuple &x);

  int global_vars;
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
  
  void addResSusp(int i, Suspension * susp, FDPropState target);
  void addForIntSusp(int i, Suspension * susp);
  void addForFDishSusp(int i, Suspension * susp);
  Bool addForXorYdet(OZ_CFun func, RefsArray xregs, int arity);
  
  void addResSusps(Suspension * susp, FDPropState target) {
    for (int i = curr_num_of_items; i--; )
      addResSusp(i, susp, target);
    
    if (global_vars == 0)
      FDcurrentTaskSusp->markLocalSusp();
  }
  void addForIntSusps(Suspension * susp) {
    for (int i = curr_num_of_items; i--; )
      addForIntSusp(i, susp);
  }
  void addForFDishSusps(Suspension * susp) {
    for (int i = curr_num_of_items; i--; )
      addForFDishSusp(i, susp);
  }
  
  int simplify(STuple &a, STuple &x) {
    return curr_num_of_items = simplifyHead(curr_num_of_items, a, x);
  }
  int simplify(int ts, STuple &a, STuple &x) {
    DebugCheck(ts > curr_num_of_items,
	       error("ts must be less or equal to curr_num_of_items."));
    return simplifyHead(ts, a, x);
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
  TypeOfTerm getTag(int i) {
    DebugCheck(i < 0 || i >= curr_num_of_items, error("index overflow"));
    return bifdhm_vartag[i];
  }
  int getCurrNumOfItems(void) {return curr_num_of_items;}
  
  TaggedRef * makeArgs3(int x, int y, int c) {
    DebugCheck((x < 0 || x >= curr_num_of_items ||
		y < 0 || y >= curr_num_of_items ||
		c < 0 || c >= curr_num_of_items),
	       error("index overflow"));
    int c_val = -bifdhm_coeff[c] * smallIntValue(bifdhm_var[c]);
    return allocateRegs(makeTaggedRef(bifdhm_varptr[x]),
			makeTaggedRef(bifdhm_varptr[y]),
			newSmallInt(c_val));
  }
  
  Bool areIdentVar(int a, int b) {
    DebugCheck((a < 0 || a >= curr_num_of_items) ||
	       (b < 0 || b >= curr_num_of_items),
	       error("index overflow."));
    return (bifdhm_varptr[a] == bifdhm_varptr[b] &&
	    isAnyVar(bifdhm_vartag[a]));
  }

  void printDebug(void) {
    for (int i = 0; i < curr_num_of_items; i += 1)
      printDebug(i);
  }
  void printDebug(int i) {
    cerr << '[' << i << "]: var=" << (void *) bifdhm_var[i]
	 << ", varptr=" << (void *) bifdhm_varptr[i]
	 << ", vartag=" << (void *) bifdhm_vartag[i]
	 << ", coeff=" << bifdhm_coeff[i] << endl;
    cerr.flush();
  }
};


//-----------------------------------------------------------------------------
//                           class BIfdBodyManager
//-----------------------------------------------------------------------------

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
  static TypeOfTerm * bifdbm_vartag;

  static FiniteDomainPtr * bifdbm_dom;
  static FiniteDomain * bifdbm_domain;

  static int curr_num_of_vars;
  static int * bifdbm_init_dom_size;
  static int * cache_from;
  static int * cache_to;
  static int * index_offset;
  static int * index_size;
  
  static Bool vars_left;
  static Bool glob_vars_touched;
  static Bool only_local_vars;
  static Bool * bifdbm_is_local;

// backup data slots
  int backup_curr_num_of_vars1;
  Bool backup_vars_left1;
  Bool backup_glob_vars_touched1;
  Bool backup_only_local_vars1;
  Suspension * backup_FDcurrentTaskSusp1;
  int backup_curr_num_of_vars2;
  Bool backup_vars_left2;
  Bool backup_glob_vars_touched2;
  Bool backup_only_local_vars2;
  Suspension * backup_FDcurrentTaskSusp2;
  
// private methods
  Bool isTouched(int i) {
    return bifdbm_init_dom_size[i] > bifdbm_dom[i]->getSize();
  }
  
  void process(void);
  void processFromTo(int, int);
  void processLocal(void);
  void processLocalFromTo(int, int);
  void processNonRes(void);
  
  void _introduce(int i, TaggedRef v);
  void introduceLocal(int i, TaggedRef v);
  void saveDomainOnTopLevel(int i) {
    if (am.currentBoard->isRoot()) {
      if (bifdbm_vartag[i] == CVAR)
	bifdbm_domain[i] = tagged2GenFDVar(bifdbm_var[i])->getDom();
    }
  }
  Bool addAnySuspToTouchedGlobalVars(void);
  int simplifyBody(int ts, STuple &a, STuple &x,
		   Bool sign_bits[], float coeffs[]);
  void _propagate_unify_cd(int clauses, int variables, STuple &st);

  enum {cache_slot_size = 4};
public:
  BIfdBodyManager(int s) {
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
    cerr << '[' << i << "]: var=" << (void *) bifdbm_var[i]
	 << ", varptr=" << (void *) bifdbm_varptr[i]
	 << ", dom=" << *bifdbm_dom[i]
	 << ", @dom=" << bifdbm_dom[i]
	 << ", init_dom_size=" << bifdbm_init_dom_size[i]
	 << ", is_local=" << bifdbm_is_local[i] << endl;
    cerr.flush();
  }

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

  OZ_Bool entailment(void) {
    if (only_local_vars) {
      processLocal();
    } else {
      process();
      if (glob_vars_touched) dismissCurrentTaskSusp();
    }
    return EntailFD;
  }

  OZ_Bool entailmentAndSuspOnAny(void) {
    Assert(FDcurrentTaskSusp);
    killPropagatedCurrentTaskSusp();

    Bool a = FALSE;
    
    if (only_local_vars) {
      processLocal();
    } else {
      a = addAnySuspToTouchedGlobalVars();
      process();
    }

    if (a) 
      activateCurrentTaskSusp();

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
      else if (glob_vars_touched)
	dismissCurrentTaskSusp(); 
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

  void propagateIfTouched(int);

  OZ_Bool release1(void) { // used by square and twice
    process();
    return EntailFD;
  }

  OZ_Bool releaseNonRes(void) { // used by putList, putNot, putLe, putGe
    processNonRes();
    return EntailFD;
  }

  int simplifyOnUnify(STuple &a, Bool sign_bits[], float coeffs[], STuple &x) {
    if (isUnifyCurrentTaskSusp())
      curr_num_of_vars =
	simplifyBody(curr_num_of_vars, a, x, sign_bits, coeffs);
    return curr_num_of_vars;
  }

  int simplifyOnUnify(int ts, STuple &a, Bool sign_bits[], float coeffs[],
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
	bifdbm_is_local[to] = bifdbm_is_local[from];
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

  OZ_Bool unify_cd(int var, int prime);

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
    return bifdbm_varptr[a] == bifdbm_varptr[b] && isAnyVar(bifdbm_vartag[a]);
  }
  
  FiniteDomainPtr * getDoms(void) {return bifdbm_dom;}

// exactly one variable is regarded
  BIfdBodyManager(void) {curr_num_of_vars = 1;}

  Bool introduce(TaggedRef v);

  FiniteDomain &operator *(void) {return *bifdbm_dom[0];}

  static void restoreDomainOnToplevel(void) {
    if (am.currentBoard->isRoot()) {
      for (int i = curr_num_of_vars; i--; )
	if (bifdbm_vartag[i] == CVAR)
	  tagged2GenFDVar(bifdbm_var[i])->getDom() = bifdbm_domain[i];
    }
  }
}; // BIfdBodyManager


//-----------------------------------------------------------------------------


#if !defined(OUTLINE) && !defined(FDOUTLINE)
#include "fdbuilti.icc"
#else
OZ_Bool addNonResSuspForDet(TaggedRef v, TaggedRefPtr vp, TypeOfTerm vt,
			    Suspension * s);
OZ_Bool addNonResSuspForCon(TaggedRef v, TaggedRefPtr vp, TypeOfTerm vt,
			    Suspension * s);
#endif


#endif
