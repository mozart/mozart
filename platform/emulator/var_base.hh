/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __VAR_BASE_HH
#define __VAR_BASE_HH

#include "base.hh"

#ifdef INTERFACE
#pragma interface
#endif

#if defined(DEBUG_CONSTRAINT_UNIFY)

#define DEBUG_CONSTRAIN_CVAR(ARGS) printf ARGS; fflush(stdout);

#else

#define DEBUG_CONSTRAIN_CVAR(ARGS)

#endif

#include "tagged.hh"
#include "susplist.hh"
#include "board.hh"
#include "value.hh"
#include "pointer-marks.hh"
#include "am.hh"

//#define DEBUG_TELLCONSTRAINTS

// NOTE:
//   this order is used in the case of CVAR=CVAR unification
//   e.g. SimpleVariable are bound prefered
// partial order required:
//  Simple<<Future<<Distributed<<everything
//  Bool<<FD
// see int cmpCVar(OzVariable *, OzVariable *)

enum TypeOfVariable {
  OZ_VAR_FD      = 0,
  OZ_VAR_BOOL    = 1,
  OZ_VAR_FS      = 2,
  OZ_VAR_CT      = 3,
  OZ_VAR_EXT     = 4,
  OZ_VAR_SIMPLE  = 5,
  OZ_VAR_FUTURE  = 6,
  OZ_VAR_OF      = 7
};

#ifdef DEBUG_CHECK
#define OZ_VAR_INVALID ((TypeOfVariable) -1)
#endif

#define STORE_FLAG 1
#define REIFIED_FLAG 2

#define SVAR_EXPORTED  0x1
#define CVAR_TRAILED   0x2
#define SVAR_FLAGSMASK 0x3

#define DISPOSE_SUSPLIST(SL)                    \
{                                               \
  SuspList * sl = SL;                           \
  while (sl) {                                  \
    sl=sl->dispose();                           \
  }                                             \
  DebugCode(SL = 0);                            \
}

class OzVariable {
  //
  friend class OzBoolVariable;
  friend class OzFDVariable;
  friend class OzFSVariable;
  friend class OzCtVariable;
  //
private:
  union {
    TypeOfVariable      var_type;
    OZ_FiniteDomain   * patchDomain;
    OZ_FSetConstraint * patchFSet;
    OZ_Ct             * patchCt;

    OZ_FDIntVar * cpi_fd_var;
    OZ_FSetVar  * cpi_fs_var;
    OZ_CtVar    * cpi_ct_var;
    void        * cpi_raw;
  } u;

  // these enumerables have to be comaptible!
  // tmueller: they have to be unified with TypeOfVariable
  enum u_mask_t {u_fd   = OZ_VAR_FD,
                 u_bool = OZ_VAR_BOOL,
                 u_fset = OZ_VAR_FS,
                 u_ct   = OZ_VAR_CT,
                 u_mask = 3};

  unsigned int homeAndFlags;
protected:
  SuspList * suspList;

public:

  TypeOfVariable getTypeMasked(void) {
    return TypeOfVariable(u.var_type & u_mask);
  }

  TypeOfVariable getType(void) {
    return u.var_type;
  }

  void setType(TypeOfVariable t){
    u.var_type = t;
  }

  OzVariable() { Assert(0); }
  OzVariable(TypeOfVariable t, DummyClass *) { setType(t); };
  OzVariable(TypeOfVariable t, Board *bb) : suspList(NULL) {
    homeAndFlags=(unsigned int)bb;
    setType(t);
  }

  USEFREELISTMEMORY;

  Board *getBoardInternal() {
    return (Board *)(homeAndFlags&~SVAR_FLAGSMASK);
  }
  void setHome(Board *h) {
    homeAndFlags = (homeAndFlags&SVAR_FLAGSMASK)|((unsigned)h);
  }

  Bool isExported(void) {
    return homeAndFlags&SVAR_EXPORTED;
  }
  void markExported() {
    homeAndFlags |= SVAR_EXPORTED;
  }

  Bool isTrailed(void) {
    return homeAndFlags&CVAR_TRAILED;
  }
  void setTrailed(void) {
    homeAndFlags |= CVAR_TRAILED;
  }
  void unsetTrailed(void) {
    homeAndFlags &= ~CVAR_TRAILED;
  }

  void disposeS(void) {
    for (SuspList * l = suspList; l; l = l->dispose());
    DebugCode(suspList=0);
  }

  Bool isEmptySuspList() { return suspList==0; }
  int getSuspListLengthS() { return suspList->length(); }

  void setSuspList(SuspList *inSuspList) { suspList = inSuspList; }
  SuspList *getSuspList() { return suspList; }
  SuspList *unlinkSuspList() {
    SuspList *sl=suspList;
    suspList= NULL;
    return sl;
  }
  SuspList ** getSuspListRef(void) {
    return &suspList;
  }

protected:

  void propagate(SuspList *& sl, PropCaller unifyVars) {
    oz_checkAnySuspensionList(&sl, this->getBoardInternal(), unifyVars);
  }
  void propagateLocal(SuspList *& sl, PropCaller unifyVars) {
    oz_checkLocalSuspensionList(&sl, unifyVars);
  }

public:
  // takes the suspensionlist of var and  appends it to the
  // suspensionlist of leftVar
  void relinkSuspListTo(OzVariable * lv, Bool reset_local = FALSE) {
    suspList = suspList->appendToAndUnlink(lv->suspList, reset_local);
  }

  Bool cacIsMarked(void) {
    return IsMarkedPointer(suspList,1);
  }
  TaggedRef * cacGetFwd(void) {
    Assert(cacIsMarked());
    return (TaggedRef *) UnMarkPointer(suspList,1);
  }

  void           gCollectMark(TaggedRef *);
  OzVariable *   gCollectVarInline();
  void           gCollectVarRecurse(void);

  void           sCloneMark(TaggedRef *);
  OzVariable *   sCloneVarInline();
  void           sCloneVarRecurse(void);
  //
  // tagging and untagging constrained variables
  //
  void setStoreFlag(void) {
    suspList = (SuspList *) (((long) suspList) | STORE_FLAG);
  }
  void resetStoreFlag(void) {
    suspList = (SuspList *) (((long) suspList) & ~STORE_FLAG);
  }
  OZ_Boolean testStoreFlag(void) {
    return ((long)suspList) & STORE_FLAG;
  }
  OZ_Boolean testResetStoreFlag(void) {
    OZ_Boolean r = testStoreFlag();
    resetStoreFlag();
    return r;
  }

  void setReifiedFlag(void) {
    suspList = (SuspList *) (((long) suspList) | REIFIED_FLAG);
  }
  void resetReifiedFlag(void) {
    suspList = (SuspList *) (((long) suspList) & ~REIFIED_FLAG);
  }
  OZ_Boolean testReifiedFlag(void) {
    return ((long)suspList) & REIFIED_FLAG;
  }
  OZ_Boolean testResetReifiedFlag(void) {
    OZ_Boolean r = testReifiedFlag();
    resetReifiedFlag();
    return r;
  }
  //
  void dropPropagator(Propagator *);
  //
  // new tagging scheme
  //
  void tagNonEncapParam(OZ_FDIntVar * fd) {
    setStoreFlag();
    u.cpi_fd_var = (OZ_FDIntVar *) ToPointer(ToInt32(fd) | getTypeMasked());
  }
  void tagNonEncapParam(OZ_FSetVar * fs) {
    setStoreFlag();
    u.cpi_fs_var = (OZ_FSetVar *) ToPointer(ToInt32(fs) | getTypeMasked());
  }
  void tagNonEncapParam(OZ_CtVar * ct) {
    setStoreFlag();
    u.cpi_ct_var = (OZ_CtVar *) ToPointer(ToInt32(ct) | getTypeMasked());
  }

  void tagEncapParam(OZ_FDIntVar * fd) {
    setReifiedFlag();
    u.cpi_fd_var = (OZ_FDIntVar *) ToPointer(ToInt32(fd) | getTypeMasked());
  }
  void tagEncapParam(OZ_FSetVar * fs) {
    setReifiedFlag();
    u.cpi_fs_var = (OZ_FSetVar *) ToPointer(ToInt32(fs) | getTypeMasked());
  }
  void tagEncapParam(OZ_CtVar * ct) {
    setReifiedFlag();
    u.cpi_ct_var = (OZ_CtVar *) ToPointer(ToInt32(ct) | getTypeMasked());
  }
  int isParamEncapTagged(void) {
    return testReifiedFlag();
  }
  int isParamNonEncapTagged(void) {
    return testStoreFlag();
  }
  int isParamTagged(void) {
    return isParamNonEncapTagged() || isParamEncapTagged();
  }
  void untagParam(void) {
    u.var_type = getTypeMasked();
    resetStoreFlag();
    resetReifiedFlag();
  }

  void * getRawAndUntag(void) {
    void * raw = u.cpi_raw;
    untagParam();
    return raw;
  }
  void putRawTag(void * raw_tag) {
    u.cpi_raw = raw_tag;
  }
  //
  // end of tagging ...
  //
  void addSuspSVar(Suspendable * susp) {
    suspList = new SuspList(susp, suspList);
    if (!oz_onToplevel())
      getBoardInternal()->checkExtSuspension(susp);
  }

  OZPRINTLONG;

  void installPropagatorsG(OzVariable *glob_var) {
    Assert(this->getType() == glob_var->getType() ||
           (this->getType() == OZ_VAR_BOOL &&
            glob_var->getType() == OZ_VAR_FD));
    suspList = oz_installPropagators(suspList,
                                     glob_var->getSuspList(),
                                     glob_var->getBoardInternal());
  }

  // needed to catch multiply occuring reified vars in propagators
  void patchReified(OZ_FiniteDomain * d, Bool isBool) {
    u.patchDomain = d;
    if (isBool) {
      u.patchDomain =
        (OZ_FiniteDomain*) ToPointer(ToInt32(u.patchDomain) | u_bool);
    }
    setReifiedFlag();
  }
  void unpatchReified(Bool isBool) {
    setType(isBool ? OZ_VAR_BOOL : OZ_VAR_FD);
    resetReifiedFlag();
  }
  OZ_Boolean isBoolPatched(void) {
    return (u.var_type & u_mask) == u_bool;
  }
  OZ_Boolean isFDPatched(void) {
    return (u.var_type & u_mask) == u_fd;
  }
  OZ_Boolean isFSetPatched(void) {
    return (u.var_type & u_mask) == u_fset;
  }
  OZ_Boolean isCtPatched(void) {
    return (u.var_type & u_mask) == u_ct;
  }

  OZ_FiniteDomain * getReifiedPatch(void) {
    return (OZ_FiniteDomain *)  (u.var_type & ~u_mask);
  }
};

/* ---------------------------------------------------------------------- */

// mm2: not inlined
OzVariable *oz_getVar(TaggedRef *v);

Bool oz_var_valid(OzVariable*,TaggedRef);
OZ_Return oz_var_unify(OzVariable*,TaggedRef*,TaggedRef*);
OZ_Return oz_var_bind(OzVariable*,TaggedRef*,TaggedRef);
OZ_Return oz_var_forceBind(OzVariable*,TaggedRef*,TaggedRef);
OZ_Return oz_var_addSusp(TaggedRef*, Suspendable *);
void oz_var_dispose(OzVariable*);
void oz_var_printStream(ostream&, const char*, OzVariable*, int = 10);
int oz_var_getSuspListLength(OzVariable*);

OzVariable * oz_var_copyForTrail(OzVariable *);
void oz_var_restoreFromCopy(OzVariable *, OzVariable *);

OZ_Return oz_var_cast(TaggedRef *&, Board *, TypeOfVariable);

inline
Bool oz_var_hasSuspAt(TaggedRef v, Board * b) {
  Assert(oz_isVariable(v) && !oz_isRef(v));
  return oz_isUVar(v) ? NO : tagged2CVar(v)->getSuspList()->hasSuspAt(b);
}

inline
Bool isFuture(TaggedRef term)
{
  GCDEBUG(term);
  return oz_isCVar(term) && (tagged2CVar(term)->getType() == OZ_VAR_FUTURE);
}

inline
Future *tagged2Future(TaggedRef t) {
  Assert(isFuture(t));
  return (Future *) tagged2CVar(t);
}

inline
Bool isSimpleVar(TaggedRef term)
{
  GCDEBUG(term);
  return oz_isCVar(term) && (tagged2CVar(term)->getType() == OZ_VAR_SIMPLE);
}

inline
SimpleVar *tagged2SimpleVar(TaggedRef t) {
  Assert(isSimpleVar(t));
  return (SimpleVar *) tagged2CVar(t);
}

/* -------------------------------------------------------------------------
 * Kinded/Free
 * ------------------------------------------------------------------------- */


enum VarStatus {
  EVAR_STATUS_KINDED,
  EVAR_STATUS_FREE,
  EVAR_STATUS_FUTURE,
  EVAR_STATUS_DET,
  EVAR_STATUS_UNKNOWN
};


OZ_Term oz_status(OZ_Term term);

/* just check without network ops */
VarStatus _var_check_status(OzVariable *cv);
/* really check status, asking manager if necessary */
OZ_Term   _var_status(OzVariable *cv);


#if defined(DEBUG_CHECK) && defined(__MINGW32__)
static
#else
inline
#endif
VarStatus oz_check_var_status(OzVariable *cv)
{
  switch (cv->getType()) {
  case OZ_VAR_FD:
  case OZ_VAR_BOOL:
  case OZ_VAR_OF:
  case OZ_VAR_FS:
  case OZ_VAR_CT:
    return EVAR_STATUS_KINDED;
  case OZ_VAR_SIMPLE:
    return EVAR_STATUS_FREE;
  case OZ_VAR_FUTURE:
    return EVAR_STATUS_FUTURE;
  case OZ_VAR_EXT:
    return _var_check_status(cv);
  ExhaustiveSwitch();
  }
  return EVAR_STATUS_UNKNOWN;
}

// isKinded || isFree || isFuture
inline
int oz_isFree(TaggedRef r)
{
  return oz_isUVar(r) ||
    (oz_isCVar(r) && oz_check_var_status(tagged2CVar(r))==EVAR_STATUS_FREE);
}

inline
int oz_isKinded(TaggedRef r)
{
  return oz_isCVar(r) && oz_check_var_status(tagged2CVar(r))==EVAR_STATUS_KINDED;
}

inline
int oz_isFuture(TaggedRef r)
{
  return oz_isCVar(r) && oz_check_var_status(tagged2CVar(r))==EVAR_STATUS_FUTURE;
}

inline
int oz_isNonKinded(TaggedRef r)
{
  return oz_isVariable(r) && !oz_isKinded(r);
}

/* -------------------------------------------------------------------------
 *
 * ------------------------------------------------------------------------- */

// only SVar and their descendants can be exclusive
inline
void setStoreFlag(OZ_Term t)
{
  tagged2CVar(t)->setStoreFlag();
}

inline
void setReifiedFlag(OZ_Term t)
{
  tagged2CVar(t)->setReifiedFlag();
}

inline
OZ_Boolean testReifiedFlag(OZ_Term t)
{
  return tagged2CVar(t)->testReifiedFlag();
}

inline
void patchReified(OZ_FiniteDomain * fd, OZ_Term t, Bool isBool)
{
  tagged2CVar(t)->patchReified(fd, isBool);
}

inline
OZ_Boolean testBoolPatched(OZ_Term t)
{
  return tagged2CVar(t)->isBoolPatched();
}

inline
OZ_Boolean testResetStoreFlag(OZ_Term t)
{
  return tagged2CVar(t)->testResetStoreFlag();
}

inline
OZ_Boolean testStoreFlag(OZ_Term t)
{
  return tagged2CVar(t)->testStoreFlag();
}

inline
OZ_Boolean testResetReifiedFlag(OZ_Term t)
{
  return tagged2CVar(t)->testResetReifiedFlag();
}

inline
OZ_FiniteDomain * unpatchReifiedFD(OZ_Term t, Bool isBool)
{
  OzVariable * v = tagged2CVar(t);

  v->unpatchReified(isBool);
  return v->getReifiedPatch();
}

// dealing with global variables
void bindGlobalVar(OZ_Term *, OZ_Term *);
void bindGlobalVarToValue(OZ_Term *, OZ_Term);
void castGlobalVar(OZ_Term *, OZ_Term *);
void constrainGlobalVar(OZ_Term *, OZ_FiniteDomain &);
void constrainGlobalVar(OZ_Term *, OZ_FSetConstraint &);
void constrainGlobalVar(OZ_Term *, OZ_Ct *);
void constrainGlobalVar(OZ_Term *, DynamicTable *);

// dealing with local variables
void bindLocalVar(OZ_Term *, OZ_Term *);
void bindLocalVarToValue(OZ_Term *, OZ_Term);
void castLocalVar(OZ_Term *, OZ_Term *);
void constrainLocalVar(OZ_Term *, OZ_FiniteDomain &);
void constrainLocalVar(OZ_Term *, OZ_FSetConstraint &);
void constrainLocalVar(OZ_Term *, OZ_Ct *, OZ_CtDefinition *);

#include "namer.hh"


#endif
