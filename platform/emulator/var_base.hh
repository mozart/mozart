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

#ifdef INTERFACE
#pragma interface
#endif

#include "tagged.hh"
#include "susplist.hh"
// mm2
#include "board.hh"
#include "value.hh"

#ifdef DEBUG_CHECK
#include "am.hh"
#endif

//#define DEBUG_TELLCONSTRAINTS

// NOTE:
//   this order is used in the case of CVAR=CVAR unification
//   e.g. SimpleVariable are bound prefered
// partial order required:
//  Simple<<Future<<Distributed<<everything
//  Bool<<FD

enum TypeOfVariable {
  OZ_VAR_EXT,
  OZ_VAR_SIMPLE,
  OZ_VAR_FUTURE,
  OZ_VAR_BOOL,
  OZ_VAR_FD,
  OZ_VAR_OF,
  OZ_VAR_FS,
  OZ_VAR_CT
};

#ifdef DEBUG_CHECK
#define OZ_VAR_INVALID ((TypeOfVariable) -1)
#endif

#define AddSuspToList(List, Susp, Home)		\
{							\
  if ((List) && ((List)->getElem() == Susp)) {		\
  } else {						\
    List = new SuspList(Susp, List);			\
    if (Home && !oz_onToplevel())                       \
      oz_checkExtSuspension(Susp,Home);	                \
  }							\
}


#define STORE_FLAG 1
#define REIFIED_FLAG 2


#define SVAR_EXPORTED 1
#define SVAR_FLAGSMASK 0x3

class OzVariable {
friend class OzFDVariable;
friend class OzFSVariable;
friend class OzCtVariable;
private:
  union {
    TypeOfVariable      var_type;
    OZ_FiniteDomain   * patchDomain;
    OZ_FSetConstraint * patchFSet;
    OZ_Ct             * patchCt;
  } u;

  enum u_mask_t {u_fd = 0, u_bool = 1, u_fset = 2, u_ct = 3, u_mask = 3};
  unsigned int homeAndFlags;
protected:
  SuspList * suspList;

protected:
  
  void propagate(SuspList *& sl, PropCaller unifyVars) {
    sl=oz_checkAnySuspensionList(sl,GETBOARD(this), unifyVars);
  }

public:
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

  Board *getHome1()        { return (Board *)(homeAndFlags&~SVAR_FLAGSMASK); }
  void setHome(Board *h) { 
    homeAndFlags = (homeAndFlags&SVAR_FLAGSMASK)|((unsigned)h); }

  Bool isExported()   { return homeAndFlags&SVAR_EXPORTED; }
  void markExported() { homeAndFlags |= SVAR_EXPORTED; }
  
  void disposeS(void) {
    for (SuspList * l = suspList; l; l = l->dispose());
    DebugCode(suspList=0);
  }

  Board *getBoardInternal() { return getHome1(); }
  Bool isEmptySuspList() { return suspList==0; }
  int getSuspListLengthS() { return suspList->length(); }

  void setSuspList(SuspList *inSuspList) { suspList = inSuspList; }
  SuspList *getSuspList() { return suspList; }
  SuspList *unlinkSuspList() {
    SuspList *sl=suspList;
    suspList= NULL;
    return sl;
  }

  // takes the suspensionlist of var and  appends it to the
  // suspensionlist of leftVar
  void relinkSuspListTo(OzVariable * lv, Bool reset_local = FALSE) {
    suspList = suspList->appendToAndUnlink(lv->suspList, reset_local);
  }

  Bool           gcIsMarked(void);
  Bool           gcIsMarkedOutlined(void);
  void           gcMark(Bool, TaggedRef *);
  TaggedRef *    gcGetFwd(void);
  TaggedRef *    gcGetFwdOutlined(void);
  OzVariable *   gcVar();
  void           gcVarRecurse(void);

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

  void addSuspSVar(Suspension susp, int unstable)
  {
    AddSuspToList(suspList, susp, unstable ? getHome1() : 0);
  }

  OZPRINTLONG;

  void installPropagatorsG(OzVariable *glob_var) {
    Assert(this->getType() == glob_var->getType() || 
	   (this->getType() == OZ_VAR_BOOL &&
	    glob_var->getType() == OZ_VAR_FD));
    suspList = oz_installPropagators(suspList,
				     glob_var->getSuspList(),
				     GETBOARD(glob_var));
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
  OZ_Boolean isBoolPatched(void) { return (u.var_type & u_mask) == u_bool; }
  OZ_Boolean isFDPatched(void) { return (u.var_type & u_mask) == u_fd; }
  OZ_Boolean isFSetPatched(void) { return (u.var_type & u_mask) == u_fset; }
  OZ_Boolean isCtPatched(void) { return (u.var_type & u_mask) == u_ct; }

  OZ_FiniteDomain * getReifiedPatch(void) { 
    return (OZ_FiniteDomain *)  (u.var_type & ~u_mask); 
  }
};

/* ---------------------------------------------------------------------- */

// mm2: not inlined
OzVariable *oz_getVar(TaggedRef *v);

Bool oz_var_valid(OzVariable*,TaggedRef*,TaggedRef);
OZ_Return oz_var_unify(OzVariable*,TaggedRef*,TaggedRef*);
OZ_Return oz_var_bind(OzVariable*,TaggedRef*,TaggedRef);
OZ_Return oz_var_forceBind(OzVariable*,TaggedRef*,TaggedRef);
OZ_Return oz_var_addSusp(TaggedRef*, Suspension, int = TRUE);
void oz_var_dispose(OzVariable*);
void oz_var_printStream(ostream&, const char*, OzVariable*, int = 10);
int oz_var_getSuspListLength(OzVariable*);

inline
Bool isFuture(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == OZ_VAR_FUTURE);
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
  return isCVar(term) && (tagged2CVar(term)->getType() == OZ_VAR_SIMPLE);
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
  default:
    Assert(0);
    return EVAR_STATUS_UNKNOWN;
  }
}

// isKinded || isFree || isFuture
inline
int oz_isFree(TaggedRef r)
{
  return isUVar(r) ||
    (isCVar(r) && oz_check_var_status(tagged2CVar(r))==EVAR_STATUS_FREE);
}

inline
int oz_isKinded(TaggedRef r)
{
  return isCVar(r) && oz_check_var_status(tagged2CVar(r))==EVAR_STATUS_KINDED;
}

inline
int oz_isFuture(TaggedRef r)
{
  return isCVar(r) && oz_check_var_status(tagged2CVar(r))==EVAR_STATUS_FUTURE;
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
  tagged2SVarPlus(t)->setStoreFlag();
}

inline
void setReifiedFlag(OZ_Term t) 
{
  tagged2SVarPlus(t)->setReifiedFlag();
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
  return tagged2SVarPlus(t)->testResetStoreFlag();
}

inline
OZ_Boolean testStoreFlag(OZ_Term t) 
{
  return tagged2SVarPlus(t)->testStoreFlag();
}

inline
OZ_Boolean testResetReifiedFlag(OZ_Term t) 
{
  return tagged2SVarPlus(t)->testResetReifiedFlag();
}

inline
OZ_FiniteDomain * unpatchReifiedFD(OZ_Term t, Bool isBool) 
{
  OzVariable * v = tagged2CVar(t);
  
  v->unpatchReified(isBool);
  return v->getReifiedPatch();
}


#include "namer.hh"


#endif
