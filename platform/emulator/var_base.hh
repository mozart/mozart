/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    Christian Schulte (schulte@ps.uni-sb.de)
 *    Raphael Collet (raph@info.ucl.ac.be)
 *    Alfred Spiessens (fsp@info.ucl.ac.be)
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

#define DEBUG_CONSTRAIN_VAR(ARGS) printf ARGS; fflush(stdout);

#else

#define DEBUG_CONSTRAIN_VAR(ARGS)

#endif

#include "tagged.hh"
#include "susplist.hh"
#include "board.hh"
#include "value.hh"
#include "pointer-marks.hh"
#include "am.hh"

//#define DEBUG_TELLCONSTRAINTS

// NOTE:
//   this order is used in the case of VAR=VAR unification
//
// kost@ : the following description (who compiled it??!), the
// 'CMPVAR(...)'  with its usage, and the table used before (just
// below) are contradictory!!
// -----
//   e.g. SimpleVariable are bound prefered
// partial order required:
//  Simple<<Future<<Distributed<<everything
//  Bool<<FD
// see int cmpVar(OzVariable *, OzVariable *)
//
//  enum TypeOfVariable {
//    OZ_VAR_FD      = 0,
//    OZ_VAR_BOOL    = 1,
//    OZ_VAR_FS      = 2,
//    OZ_VAR_CT      = 3,
//    OZ_VAR_EXT     = 4,
//    OZ_VAR_SIMPLE  = 5,
//    OZ_VAR_FUTURE  = 6,
//    OZ_VAR_OF      = 7
//  };
// -----

//
// kost@ : Now, we keep the 'cmpVar(...)'  definition, so: variables
// with GREATER types are bound to variables with SMALLER types. Here
// we go:

enum TypeOfVariable {
  // group 0:  constraint stuff, both built-in and extensible.
  //           Cannot be moved from this position since Tobias (ab)uses
  //           it also for additional tagging.
  OZ_VAR_FD             = 0,
  OZ_VAR_BOOL           = 1,
  OZ_VAR_FS             = 2,
  OZ_VAR_CT             = 3,
  // group 0a: constraints, but without a need for additional tagging;
  OZ_VAR_OF             = 4,
  // group 1: read-onlys and failed values: anything but constrained
  //          variables should be bound to them.
  //          Note that a constrained variable cannot be bound to a
  //          read-only or a failed value, since that means that the
  //          latter has to be converted to an FD variable, which is not
  //          possible.
  OZ_VAR_FAILED         = 5,
  OZ_VAR_READONLY       = 6,
  OZ_VAR_READONLY_QUIET = 7,
  // group 2:  extensions, notably the distributed variables;
  OZ_VAR_EXT            = 8,
  // group 3:  simple variables;
  OZ_VAR_SIMPLE         = 9,
  OZ_VAR_SIMPLE_QUIET   = 10,
  // group 4:  optimized variables are bound to anything else anyway
  //           whenever possible (since they are optimized);
  OZ_VAR_OPT            = 11
};


#ifdef DEBUG_CHECK
#define OZ_VAR_INVALID ((TypeOfVariable) -1)
#endif

#define STORE_FLAG 1
#define REIFIED_FLAG 2

#define SVAR_UNUSED    0x1
#define VAR_TRAILED   0x2
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
    TypeOfVariable var_type;
    OZ_FDIntVar  * cpi_fd_var;
    OZ_FSetVar   * cpi_fs_var;
    OZ_CtVar     * cpi_ct_var;
    void         * cpi_raw;
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

  void initAsExtension(Board*bb) {
    homeAndFlags=(unsigned int)bb;
    suspList = 0;
    setType(OZ_VAR_EXT);
  }

  USEFREELISTMEMORY;

  Board *getBoardInternal() {
    return (Board *)(homeAndFlags&~SVAR_FLAGSMASK);
  }
  void setHome(Board *h) {
    homeAndFlags = (homeAndFlags&SVAR_FLAGSMASK)|((unsigned)h);
  }

  Bool isTrailed(void) {
    return homeAndFlags&VAR_TRAILED;
  }
  void setTrailed(void) {
    homeAndFlags |= VAR_TRAILED;
  }
  void unsetTrailed(void) {
    homeAndFlags &= ~VAR_TRAILED;
  }

  void disposeS(void) {
    for (SuspList * l = suspList; l; l = l->dispose());
    DebugCode(suspList=0);
  }

  Bool isEmptySuspList() { return suspList==0; }
  int getSuspListLengthS() { return suspList->length(); }
  Bool isInSuspList(Suspendable * sl) {
    return suspList->isIn(sl);
  }

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
  void setReifiedFlag(void) {
    suspList = (SuspList *) (((long) suspList) | REIFIED_FLAG);
  }
  void resetReifiedFlag(void) {
    suspList = (SuspList *) (((long) suspList) & ~REIFIED_FLAG);
  }
  OZ_Boolean testReifiedFlag(void) {
    return ((long)suspList) & REIFIED_FLAG;
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

  void * getRaw(void) {
    return (void *)(u.var_type & ~u_mask);
  }
  void * getRawAndUntag(void) {
    void * raw = getRaw();
    untagParam();
    return raw;
  }
  void putRawTag(void * raw_tag) {
    u.cpi_raw = (void *) ToPointer(ToInt32(raw_tag) | getTypeMasked());
  }
  //
  // end of tagging ...
  //

  //
  void addSuspSVar(Suspendable * susp) {
    Assert(getType() != OZ_VAR_OPT);
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

};

/* ---------------------------------------------------------------------- */

//
OzVariable *oz_getNonOptVar(TaggedRef *v);

Bool oz_var_valid(OzVariable*,TaggedRef);
// getBoard(lvp) != getBoard(rvp) || getType(lvp) >= getType(rvp)
OZ_Return oz_var_unify(OzVariable *v, TaggedRef *lvp, TaggedRef *rvp);
OZ_Return oz_var_bind(OzVariable*,TaggedRef*,TaggedRef);
OZ_Return oz_var_forceBind(OzVariable*,TaggedRef*,TaggedRef);
OZ_Return oz_var_addSusp(TaggedRef*, Suspendable *);
OZ_Return oz_var_addQuietSusp(TaggedRef*, Suspendable *);
OZ_Return oz_var_makeNeeded(TaggedRef *);
void oz_var_dispose(OzVariable*);
void oz_var_printStream(ostream&, const char*, OzVariable*, int = 10);
int oz_var_getSuspListLength(OzVariable*);

OzVariable * oz_var_copyForTrail(OzVariable *);
void oz_var_restoreFromCopy(OzVariable *, OzVariable *);

OZ_Return oz_var_cast(TaggedRef *&, Board *, TypeOfVariable);

inline
Bool oz_var_hasSuspAt(TaggedRef v, Board * b) {
  Assert(oz_isVar(v) && !oz_isRef(v));
  return oz_isOptVar(v) ? NO : tagged2Var(v)->getSuspList()->hasSuspAt(b);
}

inline
SimpleVar *tagged2SimpleVar(TaggedRef t)
{
  Assert(oz_isVar(t) && ((tagged2Var(t)->getType() == OZ_VAR_SIMPLE) ||
                         (tagged2Var(t)->getType() == OZ_VAR_SIMPLE_QUIET)));
  return ((SimpleVar *) tagged2Var(t));
}

/* -------------------------------------------------------------------------
 * Kinded/Free
 * ------------------------------------------------------------------------- */


enum VarStatus {
  EVAR_STATUS_KINDED,
  EVAR_STATUS_FREE,
  EVAR_STATUS_READONLY,
  EVAR_STATUS_FAILED,
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
  case OZ_VAR_FS:
  case OZ_VAR_CT:
  case OZ_VAR_OF:
    return EVAR_STATUS_KINDED;
  case OZ_VAR_EXT:
    return _var_check_status(cv);
  case OZ_VAR_SIMPLE_QUIET:
  case OZ_VAR_SIMPLE:
    return EVAR_STATUS_FREE;
  case OZ_VAR_READONLY_QUIET:
  case OZ_VAR_READONLY:
    return EVAR_STATUS_READONLY;
  case OZ_VAR_FAILED:
    return EVAR_STATUS_FAILED;
  case OZ_VAR_OPT:
    return EVAR_STATUS_FREE;
  ExhaustiveSwitch();
  }
  return EVAR_STATUS_UNKNOWN;
}

#if defined(DEBUG_CHECK)

// isKinded || isFree || isReadOnly
inline
int oz_isFree(TaggedRef r)
{
  return (oz_isVar(r) &&
          oz_check_var_status(tagged2Var(r)) == EVAR_STATUS_FREE);
}

inline
int oz_isKinded(TaggedRef r)
{
  return (oz_isVar(r) &&
          oz_check_var_status(tagged2Var(r))==EVAR_STATUS_KINDED);
}

inline
int oz_isKindedVar(TaggedRef r)
{
  Assert(oz_isVar(r));
  return (oz_check_var_status(tagged2Var(r)) == EVAR_STATUS_KINDED);
}

inline
int oz_isReadOnly(TaggedRef r)
{
  return (oz_isVar(r) &&
          oz_check_var_status(tagged2Var(r))==EVAR_STATUS_READONLY);
}

// returns TRUE iff the entity is a failed value
inline
int oz_isFailed(TaggedRef r) {
  return (oz_isVar(r) &&
          oz_check_var_status(tagged2Var(r))==EVAR_STATUS_FAILED);
}

inline
int oz_isNonKinded(TaggedRef r)
{
  return (oz_isVar(r) && !oz_isKindedVar(r));
}

#else

#define oz_isFree(r)                                                    \
((oz_isVar(r) && oz_check_var_status(tagged2Var(r)) == EVAR_STATUS_FREE))

#define oz_isKinded(r)                                                  \
((oz_isVar(r) && oz_check_var_status(tagged2Var(r))==EVAR_STATUS_KINDED))

#define oz_isKindedVar(r)                                               \
((oz_check_var_status(tagged2Var(r)) == EVAR_STATUS_KINDED))

#define oz_isReadOnly(r)                                                \
((oz_isVar(r) && oz_check_var_status(tagged2Var(r))==EVAR_STATUS_READONLY))

#define oz_isFailed(r)                                                  \
((oz_isVar(r) && oz_check_var_status(tagged2Var(r))==EVAR_STATUS_FAILED))

#define oz_isNonKinded(r)                                               \
((oz_isVar(r) && !oz_isKindedVar(r)))

#endif


// raph: to know whether an entity is needed
inline
int oz_isNeeded(TaggedRef r) {
  if (oz_isVar(r)) {
    switch (tagged2Var(r)->getType()) {
    case OZ_VAR_OPT:
    case OZ_VAR_SIMPLE_QUIET:
    case OZ_VAR_READONLY_QUIET:
      return FALSE;
    default:
      // every other type of variable is needed
      return TRUE;
    }
  }
  // values are needed
  return TRUE;
}


/* -------------------------------------------------------------------------
 *
 * ------------------------------------------------------------------------- */

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
