/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *    Raphael Collet <raph@info.ucl.ac.be>
 *    Alfred Spiessens <fsp@info.ucl.ac.be>
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

#if defined(INTERFACE)
#pragma implementation "var_base.hh"
#endif

#include "trail.hh"
#include "var_base.hh"
#include "var_fs.hh"
#include "var_fd.hh"
#include "var_bool.hh"
#include "var_of.hh"
#include "var_ct.hh"
#include "var_simple.hh"
#include "var_quiet.hh"
#include "var_opt.hh"
#include "var_readonly_quiet.hh"
#include "var_readonly.hh"
#include "var_future.hh"
#include "var_ext.hh"
#include "dpInterface.hh"
#include "ozconfig.hh"

int oz_raise(OZ_Term cat, OZ_Term key, const char *label, int arity, ...);

void OzVariable::dropPropagator(Propagator * prop)
{
  switch (getType()){
  case OZ_VAR_BOOL:    
    ((OzBoolVariable*) this)->dropPropagator(prop);
    return;
  case OZ_VAR_FD:      
    ((OzFDVariable*) this)->dropPropagator(prop);
    return;
  case OZ_VAR_FS:      
    ((OzFSVariable*) this)->dropPropagator(prop);
    return;
  case OZ_VAR_CT:      
    ((OzCtVariable*) this)->dropPropagator(prop);
    return;
  ExhaustiveSwitch();
  }
}

Bool oz_var_valid(OzVariable *ov,TaggedRef val) {
  switch (ov->getType()){
  case OZ_VAR_OPT:            return ((OptVar *) ov)->valid(val);
  case OZ_VAR_QUIET:          return ((QuietVar *) ov)->valid(val);
  case OZ_VAR_SIMPLE:         return ((SimpleVar *) ov)->valid(val);
  case OZ_VAR_READONLY_QUIET: return ((QuietReadOnly *) ov)->valid(val);
  case OZ_VAR_READONLY:       return ((ReadOnly *) ov)->valid(val);
  case OZ_VAR_FUTURE:         return ((Future *) ov)->valid(val);
  case OZ_VAR_BOOL:           return ((OzBoolVariable*) ov)->valid(val);
  case OZ_VAR_FD:             return ((OzFDVariable*) ov)->valid(val);
  case OZ_VAR_OF:             return ((OzOFVariable*) ov)->valid(val);
  case OZ_VAR_FS:             return ((OzFSVariable*) ov)->valid(val);
  case OZ_VAR_CT:             return ((OzCtVariable*) ov)->valid(val);
  case OZ_VAR_EXT:            return var2ExtVar(ov)->validV(val);
  ExhaustiveSwitch();
  }
  return NO;
}

OZ_Return oz_var_unify(OzVariable *ov, TaggedRef *ptr, TaggedRef *val) {
  Assert(oz_isVar(*val));
  Assert(ov == tagged2Var(*ptr));
  DebugCode(OzVariable *rv = tagged2Var(*val));
  DebugCode(Board* tb1 = ov->getBoardInternal()->derefBoard());
  DebugCode(Board* tb2 = rv->getBoardInternal()->derefBoard());
  Assert(oz_isBelow(tb1, tb2) || ov->getType() >= rv->getType());
  //
  switch (ov->getType()){
  case OZ_VAR_OPT:            return ((OptVar *) ov)->unify(ptr,val);
  case OZ_VAR_QUIET:          return ((QuietVar *) ov)->unify(ptr,val);
  case OZ_VAR_SIMPLE:         return ((SimpleVar *) ov)->unify(ptr,val);
  case OZ_VAR_READONLY_QUIET: return ((QuietReadOnly *) ov)->unify(ptr,val);
  case OZ_VAR_READONLY:       return ((ReadOnly *) ov)->unify(ptr,val);
  case OZ_VAR_FUTURE:         return ((Future *) ov)->unify(ptr,val);
  case OZ_VAR_BOOL:           return ((OzBoolVariable*) ov)->unify(ptr,val);
  case OZ_VAR_FD:             return ((OzFDVariable*) ov)->unify(ptr,val);
  case OZ_VAR_OF:             return ((OzOFVariable*) ov)->unify(ptr,val);
  case OZ_VAR_FS:             return ((OzFSVariable*) ov)->unify(ptr,val);
  case OZ_VAR_CT:             return ((OzCtVariable*) ov)->unify(ptr,val);
  case OZ_VAR_EXT:            return var2ExtVar(ov)->unifyV(ptr,val);
  ExhaustiveSwitch();
  }
  return FAILED;
}

OZ_Return oz_var_bind(OzVariable *ov,TaggedRef *ptr,TaggedRef val) {
  switch (ov->getType()){
  case OZ_VAR_OPT:            return ((OptVar *) ov)->bind(ptr,val);
  case OZ_VAR_QUIET:          return ((QuietVar *) ov)->bind(ptr,val);
  case OZ_VAR_SIMPLE:         return ((SimpleVar *) ov)->bind(ptr,val);
  case OZ_VAR_READONLY_QUIET: return ((QuietReadOnly *) ov)->bind(ptr,val);
  case OZ_VAR_READONLY:       return ((ReadOnly *) ov)->bind(ptr,val);
  case OZ_VAR_FUTURE:         return ((Future *) ov)->bind(ptr,val);
  case OZ_VAR_BOOL:           return ((OzBoolVariable*) ov)->bind(ptr,val);
  case OZ_VAR_FD:             return ((OzFDVariable*) ov)->bind(ptr,val);
  case OZ_VAR_OF:             return ((OzOFVariable*) ov)->bind(ptr,val);
  case OZ_VAR_FS:             return ((OzFSVariable*) ov)->bind(ptr,val);
  case OZ_VAR_CT:             return ((OzCtVariable*) ov)->bind(ptr,val);
  case OZ_VAR_EXT:            return var2ExtVar(ov)->bindV(ptr,val);
  ExhaustiveSwitch();
  }
  return FAILED;
}

OZ_Return oz_var_forceBind(OzVariable *ov,TaggedRef *ptr,TaggedRef val) {
  switch (ov->getType()){
  case OZ_VAR_OPT:      return ((OptVar *) ov)->bind(ptr,val);
  case OZ_VAR_QUIET:    return ((QuietVar *) ov)->bind(ptr,val);
  case OZ_VAR_SIMPLE:   return ((SimpleVar *) ov)->bind(ptr,val);
  case OZ_VAR_READONLY_QUIET:
    return ((QuietReadOnly *) ov)->forceBind(ptr,val);
  case OZ_VAR_READONLY: return ((ReadOnly *) ov)->forceBind(ptr,val);
  case OZ_VAR_FUTURE:   return ((Future *) ov)->forceBind(ptr,val);
  case OZ_VAR_BOOL:     return ((OzBoolVariable*) ov)->bind(ptr,val);
  case OZ_VAR_FD:       return ((OzFDVariable*) ov)->bind(ptr,val);
  case OZ_VAR_OF:       return ((OzOFVariable*) ov)->bind(ptr,val);
  case OZ_VAR_FS:       return ((OzFSVariable*) ov)->bind(ptr,val);
  case OZ_VAR_CT:       return ((OzCtVariable*) ov)->bind(ptr,val);
  case OZ_VAR_EXT:      return var2ExtVar(ov)->forceBindV(ptr,val);
  ExhaustiveSwitch();
  }
  return FAILED;
}

//
// Convert an OptVar to a simple var;
OzVariable* oz_getNonOptVar(TaggedRef *v)
{
  OzVariable *ov = tagged2Var(*v);
  if (ov->getType() == OZ_VAR_OPT) {
    ov = new SimpleVar(ov->getBoardInternal());
    *v = makeTaggedVar(ov);
  }
  return (ov);
}

OZ_Return oz_var_addSusp(TaggedRef *v, Suspendable * susp) {
  Assert(oz_isVar(*v));
  OzVariable *ov = tagged2Var(*v);
  switch (ov->getType()) {
  case OZ_VAR_FUTURE:
    return ((Future *) ov)->addSusp(v, susp);
  case OZ_VAR_READONLY_QUIET:
    return ((QuietReadOnly *) ov)->addSusp(v, susp);
  case OZ_VAR_QUIET:
    return ((QuietVar *) ov)->addSusp(v, susp);
  case OZ_VAR_EXT:
    return var2ExtVar(ov)->addSuspV(v, susp);
  case OZ_VAR_OPT:
    ov = new SimpleVar(ov->getBoardInternal());
    *v = makeTaggedVar(ov);
    // fall through;
  case OZ_VAR_READONLY:
  case OZ_VAR_SIMPLE:
    if (ozconf.useFutures || susp->isNoBlock()) {
      return oz_raise(E_ERROR, E_KERNEL, "block", 1, makeTaggedRef(v));
    }
    // fall through
  default:
    ov->addSuspSVar(susp);
    return (SUSPEND);
  }
}

// fred+raph: Add a "quiet" suspension to v
OZ_Return oz_var_addQuietSusp(TaggedRef *v, Suspendable * susp) {
  Assert(oz_isVar(*v));
  OzVariable *ov = tagged2Var(*v);
  switch (ov->getType()) {
  case OZ_VAR_FUTURE:
    // not correct, to be fixed later
    return ((Future *) ov)->addSusp(v, susp);
  case OZ_VAR_EXT:
    return var2ExtVar(ov)->addSuspV(v, susp);
  case OZ_VAR_OPT:
    ov = new QuietVar(ov->getBoardInternal());
    *v = makeTaggedVar(ov);
    // fall through;
  case OZ_VAR_READONLY_QUIET:
  case OZ_VAR_READONLY:
  case OZ_VAR_QUIET:
  case OZ_VAR_SIMPLE:
    if (ozconf.useFutures || susp->isNoBlock()) {
      return oz_raise(E_ERROR, E_KERNEL, "block", 1, makeTaggedRef(v));
    }
    // fall through
  default:
    ov->addSuspSVar(susp);
    return (SUSPEND);
  }
}

void oz_var_dispose(OzVariable *ov) {
  switch (ov->getType()) {
  case OZ_VAR_OPT:     ((OptVar *) ov)->dispose(); break; // debugging, if any;
  case OZ_VAR_QUIET:          ((QuietVar *) ov)->dispose(); break;
  case OZ_VAR_SIMPLE:         ((SimpleVar *) ov)->dispose(); break;
  case OZ_VAR_READONLY_QUIET: ((QuietReadOnly *) ov)->dispose(); break;
  case OZ_VAR_READONLY:       ((ReadOnly *) ov)->dispose(); break;
  case OZ_VAR_FUTURE:         ((Future *) ov)->dispose(); break;
  case OZ_VAR_BOOL:           ((OzBoolVariable*) ov)->dispose(); break;
  case OZ_VAR_FD:             ((OzFDVariable*) ov)->dispose(); break;
  case OZ_VAR_OF:             ((OzOFVariable*) ov)->dispose(); break;
  case OZ_VAR_FS:             ((OzFSVariable*) ov)->dispose(); break;
  case OZ_VAR_CT:             ((OzCtVariable*) ov)->dispose(); break;
  case OZ_VAR_EXT:            var2ExtVar(ov)->disposeV(); break;
  ExhaustiveSwitch();
  }
}

void oz_var_printStream(ostream &out, const char *s, OzVariable *cv, int depth)
{
  if (ozconf.printVerbose)
    switch (cv->getType()) {
    case OZ_VAR_OPT:
      out << s;
      ((OptVar *) cv)->printStream(out, depth); return;
    case OZ_VAR_QUIET:
      out << s;
      ((QuietVar *)cv)->printStream(out,depth); return;
    case OZ_VAR_SIMPLE:
      out << s;
      ((SimpleVar *)cv)->printStream(out,depth); return;
    case OZ_VAR_READONLY_QUIET:
      out << s;
      ((QuietReadOnly *)cv)->printStream(out,depth); return;
    case OZ_VAR_READONLY:
      out << s;
      ((ReadOnly *)cv)->printStream(out,depth); return;
    case OZ_VAR_FUTURE:
      out << s;
      ((Future *)cv)->printStream(out,depth); return;
    case OZ_VAR_BOOL:
      out << s;
      ((OzBoolVariable*)cv)->printStream(out,depth); return;
    case OZ_VAR_FD:
      out << s;
      ((OzFDVariable*)cv)->printStream(out,depth); return;
    case OZ_VAR_OF:
      ((OzOFVariable*)cv)->printStream(out,depth); return;
    case OZ_VAR_FS:
      out << s;
      ((OzFSVariable*)cv)->printStream(out,depth); return;
    case OZ_VAR_CT:
      out << s;
      ((OzCtVariable*)cv)->printStream(out,depth); return;
    case OZ_VAR_EXT:
      out << s;
      var2ExtVar(cv)->printStreamV(out,depth); return;
      ExhaustiveSwitch();
    }
  else
    out << s;
}

int oz_var_getSuspListLength(OzVariable *cv)
{
  Assert(cv->getType()!=OZ_VAR_INVALID);

  switch (cv->getType()){
  case OZ_VAR_BOOL:   return ((OzBoolVariable*)cv)->getSuspListLength();
  case OZ_VAR_FD:     return ((OzFDVariable*)cv)->getSuspListLength();
  case OZ_VAR_OF:     return ((OzOFVariable*)cv)->getSuspListLength();
  case OZ_VAR_FS:     return ((OzFSVariable*)cv)->getSuspListLength();
  case OZ_VAR_CT:     return ((OzCtVariable*)cv)->getSuspListLength();
  case OZ_VAR_EXT:    return var2ExtVar(cv)->getSuspListLengthV();
  case OZ_VAR_OPT:    return (0); // per definition;
  default:            return cv->getSuspListLengthS();
  }
}

OzVariable * oz_var_copyForTrail(OzVariable * v) {
  switch (v->getType()){
  case OZ_VAR_FD: 
    return ((OzFDVariable *) v)->copyForTrail();
  case OZ_VAR_OF: 
    return ((OzOFVariable *) v)->copyForTrail();
  case OZ_VAR_FS: 
    return ((OzFSVariable *) v)->copyForTrail();
  case OZ_VAR_CT: 
    return ((OzCtVariable *) v)->copyForTrail();
  default:
    Assert(0);
    return NULL;
  }
}

void oz_var_restoreFromCopy(OzVariable * o, OzVariable * c) {
  Assert(c->getType() == o->getType());

  switch (o->getType()) {
  case OZ_VAR_FD: 
    ((OzFDVariable *) o)->restoreFromCopy((OzFDVariable *) c);
    break;
  case OZ_VAR_OF: 
    ((OzOFVariable *) o)->restoreFromCopy((OzOFVariable *) c);
    break;
  case OZ_VAR_FS: 
    ((OzFSVariable *) o)->restoreFromCopy((OzFSVariable *) c);
    break;
  case OZ_VAR_CT: 
    ((OzCtVariable *) o)->restoreFromCopy((OzCtVariable *) c);
    break;
  default:
    Assert(0);
    break;
  }  
}

extern void oz_forceWakeUp(SuspList **);

/*
 * This is the definitive casting table
 *
 *  L=    R=| OPT   | SI    | FU    | EX    | OF    | CT    | FS    | BOOL  | FD    |
 *  ---------------------------------------------------------------------------------
 *  OPT     | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  |
 *  ---------------------------------------------------------------------------------
 *  SI      | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  |
 *  ---------------------------------------------------------------------------------
 *  FU      | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  |
 *  ---------------------------------------------------------------------------------
 *  EX      | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  |
 *  ---------------------------------------------------------------------------------
 *  OF      | R->OF | R->OF | SUSP  | SUSP  | NOOP  | CLASH | CLASH | CLASH | CLASH |
 *  ---------------------------------------------------------------------------------
 *  CT      | R->CT | R->CT | SUSP  | SUSP  | CLASH | NOOP  | CLASH | CLASH | CLASH |
 *  ---------------------------------------------------------------------------------
 *  FS      | R->FS | R->FS | SUSP  | SUSP  | CLASH | CLASH | NOOP  | CLASH | CLASH |
 *  ---------------------------------------------------------------------------------
 *  BOOL    | R->BO | R->BO | SUSP  | SUSP  | CLASH | CLASH | CLASH | NOOP  | NOOP  |
 *  ---------------------------------------------------------------------------------
 *  FD      | R->FD | R->FD | SUSP  | SUSP  | CLASH | CLASH | CLASH | NOOP  | NOOP  |
 *  ---------------------------------------------------------------------------------
 *
 * Comments (kost@):
 * -  Ext -> *      is ignored here: that's the business of that ext var itself!
 *                  Notably, the distribution one;
 * -  Future -> *   is, and has to be, handled by the Future::bind. Don't return 
 *                  'SUSPEND' here!
 * -  Ct -> Future  cannot proceed: unification of Ct"s require a Ct or a non-variable
 *                  at the right hand side, which is not possible due to the nature.
 * -  Ct -> Ext     cannot proceed either: don't know how to make a Ct out of it!
 *
 *
 * raph+fsp: We added new cases, namely quiet variables and read-onlys
 *
 *  L=    R=| OPT   | SI    | FU    | EX    | OF    | CT    | FS    | BOOL  | FD    |
 *  ---------------------------------------------------------------------------------
 *  QU      | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  |
 *  ---------------------------------------------------------------------------------
 *  QURO    | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  |
 *  ---------------------------------------------------------------------------------
 *  RO      | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  | NOOP  |
 *  ---------------------------------------------------------------------------------
 *
 *  L=    R=| QU    | QURO  | RO    |
 *  ---------------------------------
 *  OPT     | NOOP  | NOOP  | NOOP  |
 *  ---------------------------------
 *  QU      | NOOP  | NOOP  | NOOP  |
 *  ---------------------------------
 *  SI      | R->SI | R->RO | NOOP  |
 *  ---------------------------------
 *  QURO    | NOOP  | NOOP  | NOOP  |
 *  ---------------------------------
 *  RO      | NOOP  | NOOP  | NOOP  |
 *  ---------------------------------
 *  FU      | NOOP  | NOOP  | NOOP  |
 *  ---------------------------------
 *  EX      | NOOP  | NOOP  | NOOP  |
 *  ---------------------------------
 *  OF      | R->OF | SUSP  | SUSP  |
 *  ---------------------------------
 *  CT      | R->CT | SUSP  | SUSP  |
 *  ---------------------------------
 *  FS      | R->FS | SUSP  | SUSP  |
 *  ---------------------------------
 *  BOOL    | R->BO | SUSP  | SUSP  |
 *  ---------------------------------
 *  FD      | R->FD | SUSP  | SUSP  |
 *  ---------------------------------
 */

#define VARTP(T1,T2) ((T1<<4)|T2)
#define VTP(T1,T2)   VARTP(OZ_VAR_ ## T1, OZ_VAR_ ## T2)


OZ_Return oz_var_cast(TaggedRef * & fp, Board * fb, TypeOfVariable tt) {
  OzVariable * fv = tagged2Var(*fp);

  TypeOfVariable ft = fv->getType();

  OzVariable * tv;
  
  switch (VARTP(tt,ft)) {

  case VTP(FD,FS):   case VTP(FD,OF):   case VTP(FD,CT):
  case VTP(BOOL,FS): case VTP(BOOL,OF): case VTP(BOOL,CT):
  case VTP(FS,FD): case VTP(FS,BOOL): case VTP(FS,OF): case VTP(FS,CT):
  case VTP(CT,FD): case VTP(CT,BOOL): case VTP(CT,FS): case VTP(CT,OF):
  case VTP(OF,FD): case VTP(OF,BOOL): case VTP(OF,FS): case VTP(OF,CT):
    return FAILED;

  case VTP(FD,OPT): case VTP(FD,QUIET): case VTP(FD,SIMPLE):
  case VTP(FD,READONLY_QUIET): case VTP(FD,READONLY):
  case VTP(FD,FUTURE): case VTP(FD,EXT):
    tv = new OzFDVariable(fb);
    break;

  case VTP(BOOL,OPT): case VTP(BOOL,QUIET): case VTP(BOOL,SIMPLE):
  case VTP(BOOL,READONLY_QUIET): case VTP(BOOL,READONLY):
  case VTP(BOOL,FUTURE): case VTP(BOOL,EXT):
    tv = new OzBoolVariable(fb);
    break;

  case VTP(FS,OPT): case VTP(FS,QUIET): case VTP(FS,SIMPLE):
  case VTP(FS,READONLY_QUIET): case VTP(FS,READONLY):
  case VTP(FS,FUTURE): case VTP(FS,EXT):
    tv = new OzFSVariable(fb);
    break;

  case VTP(CT,OPT): case VTP(CT,QUIET): case VTP(CT,SIMPLE):
  case VTP(CT,READONLY_QUIET): case VTP(CT,READONLY):
  case VTP(CT,FUTURE): case VTP(CT,EXT):
    tv = new OzCtVariable(((OzCtVariable *) fv)->getConstraint(), 
			  ((OzCtVariable *) fv)->getDefinition(), 
			  fb);
    break;

  case VTP(OF,OPT): case VTP(OF,QUIET): case VTP(OF,SIMPLE):
  case VTP(OF,READONLY_QUIET): case VTP(OF,READONLY):
  case VTP(OF,FUTURE): case VTP(OF,EXT):
    tv = new OzOFVariable(fb);
    break;

  case VTP(SIMPLE,QUIET):
    ((QuietVar *) fv)->becomeNeeded();
    return PROCEED;

  case VTP(SIMPLE,READONLY_QUIET):
    ((QuietReadOnly *) fv)->becomeNeeded();
    return PROCEED;

  default:
    return PROCEED;
  }

  if (am.inEqEq()) {
    (void) oz_var_bind(fv, fp, makeTaggedRef(newTaggedVar(tv)));
  } else {
    oz_forceWakeUp(fv->getSuspListRef());
    *fp = makeTaggedRef(newTaggedVar(tv));

  }

  Assert(oz_isRef(*fp));

  fp = tagged2Ref(*fp);

  return PROCEED;
}

#undef VTP
#undef VARTP
  

OZ_Term _var_status(OzVariable *cv) {
  Assert(cv->getType()==OZ_VAR_EXT);
  return var2ExtVar(cv)->statusV();
}


VarStatus _var_check_status(OzVariable *cv) {
  Assert(cv->getType()==OZ_VAR_EXT);
  return var2ExtVar(cv)->checkStatusV();
}

// dealing with global variables
// add assertions that right sides of bin and cast are global or values!
void bindGlobalVarToValue(OZ_Term * varptr, OZ_Term value)
{
  DEBUG_CONSTRAIN_VAR(("bindGlobalVarToValue\n"));
  DoBindAndTrail(varptr, value);
}

void bindGlobalVar(OZ_Term * varptr_left, OZ_Term * varptr_right)
{
  DEBUG_CONSTRAIN_VAR(("bindGlobalVar\n"));
  DoBindAndTrail(varptr_left, makeTaggedRef(varptr_right));
}

void castGlobalVar(OZ_Term * varptr_left, OZ_Term * varptr_right)
{
  DEBUG_CONSTRAIN_VAR(("castGlobalVar\n"));
  DoBindAndTrail(varptr_left, makeTaggedRef(varptr_right));
}

void constrainGlobalVar(OZ_Term * varptr, OZ_FiniteDomain & fd)
{
  DEBUG_CONSTRAIN_VAR(("constrainGlobalVar(fd)\n"));
  trail.pushVariable(varptr);
  OzFDVariable * fdvar = (OzFDVariable *) tagged2Var(*varptr);
  fdvar->setDom(fd);
}

void constrainGlobalVar(OZ_Term * varptr, OZ_FSetConstraint &fs)
{
  DEBUG_CONSTRAIN_VAR(("constrainGlobalVar(fs)\n"));
  trail.pushVariable(varptr);
  OzFSVariable * fsvar = (OzFSVariable *) tagged2Var(*varptr);
  fsvar->setSet(fs);
}

void constrainGlobalVar(OZ_Term * varptr, OZ_Ct * ct)
{
  DEBUG_CONSTRAIN_VAR(("constrainGlobalVar(ct)\n"));
  trail.pushVariable(varptr);
  OzCtVariable * ctvar = (OzCtVariable *) tagged2Var(*varptr);
  ctvar->copyConstraint(ct); 
}

void constrainGlobalVar(OZ_Term * varptr, DynamicTable * dt)
{
  DEBUG_CONSTRAIN_VAR(("constrainGlobalVar(of)\n"));
  trail.pushVariable(varptr);
  OzOFVariable * ofvar = (OzOFVariable *) tagged2Var(*varptr);
  ofvar->dynamictable = dt->copyDynamicTable();
}

// dealing with local variables
void bindLocalVarToValue(OZ_Term * varptr, OZ_Term value)
{
  DEBUG_CONSTRAIN_VAR(("bindLocalVarToValue\n"));
  DoBind(varptr, value);
}

void bindLocalVar(OZ_Term * varptr_left, OZ_Term * varptr_right)
{
  DEBUG_CONSTRAIN_VAR(("bindLocalVar\n"));
  DoBind(varptr_left, makeTaggedRef(varptr_right));
}
