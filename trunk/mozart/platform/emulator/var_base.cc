/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
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
#include "var_future.hh"
#include "var_ext.hh"
#include "dpInterface.hh"

int oz_raise(OZ_Term cat, OZ_Term key, const char *label, int arity, ...);

Bool oz_var_valid(OzVariable *ov,TaggedRef *ptr,TaggedRef val) {
  switch (ov->getType()){
  case OZ_VAR_SIMPLE:  return ((SimpleVar *) ov)->valid(val);
  case OZ_VAR_FUTURE:  return ((Future *) ov)->valid(val);
  case OZ_VAR_BOOL:    return ((OzBoolVariable*) ov)->valid(val);
  case OZ_VAR_FD:      return ((OzFDVariable*) ov)->valid(val);
  case OZ_VAR_OF:      return ((OzOFVariable*) ov)->valid(val);
  case OZ_VAR_FS:      return ((OzFSVariable*) ov)->valid(val);
  case OZ_VAR_CT:      return ((OzCtVariable*) ov)->valid(val);
  case OZ_VAR_EXT:     return ((ExtVar *) ov)->validV(val);
  ExhaustiveSwitch();
  }
  return NO;
}

OZ_Return oz_var_unify(OzVariable *ov,TaggedRef *ptr,TaggedRef *val) {
  switch (ov->getType()){
  case OZ_VAR_SIMPLE:  return ((SimpleVar *) ov)->unify(ptr,val);
  case OZ_VAR_FUTURE:  return ((Future *) ov)->unify(ptr,val);
  case OZ_VAR_BOOL:    return ((OzBoolVariable*) ov)->unify(ptr,val);
  case OZ_VAR_FD:      return ((OzFDVariable*) ov)->unify(ptr,val);
  case OZ_VAR_OF:      return ((OzOFVariable*) ov)->unify(ptr,val);
  case OZ_VAR_FS:      return ((OzFSVariable*) ov)->unify(ptr,val);
  case OZ_VAR_CT:      return ((OzCtVariable*) ov)->unify(ptr,val);
  case OZ_VAR_EXT:     return ((ExtVar *) ov)->unifyV(ptr,val);
  ExhaustiveSwitch();
  }
  return FAILED;
}

OZ_Return oz_var_bind(OzVariable *ov,TaggedRef *ptr,TaggedRef val) {
  switch (ov->getType()){
  case OZ_VAR_SIMPLE:  return ((SimpleVar *) ov)->bind(ptr,val);
  case OZ_VAR_FUTURE:  return ((Future *) ov)->bind(ptr,val);
  case OZ_VAR_BOOL:    return ((OzBoolVariable*) ov)->bind(ptr,val);
  case OZ_VAR_FD:      return ((OzFDVariable*) ov)->bind(ptr,val);
  case OZ_VAR_OF:      return ((OzOFVariable*) ov)->bind(ptr,val);
  case OZ_VAR_FS:      return ((OzFSVariable*) ov)->bind(ptr,val);
  case OZ_VAR_CT:      return ((OzCtVariable*) ov)->bind(ptr,val);
  case OZ_VAR_EXT:     return ((ExtVar *) ov)->bindV(ptr,val);
  ExhaustiveSwitch();
  }
  return FAILED;
}

OZ_Return oz_var_forceBind(OzVariable *ov,TaggedRef *ptr,TaggedRef val) {
  switch (ov->getType()){
  case OZ_VAR_SIMPLE:  return ((SimpleVar *) ov)->bind(ptr,val);
  case OZ_VAR_FUTURE:  return ((Future *) ov)->forceBind(ptr,val);
  case OZ_VAR_BOOL:    return ((OzBoolVariable*) ov)->bind(ptr,val);
  case OZ_VAR_FD:      return ((OzFDVariable*) ov)->bind(ptr,val);
  case OZ_VAR_OF:      return ((OzOFVariable*) ov)->bind(ptr,val);
  case OZ_VAR_FS:      return ((OzFSVariable*) ov)->bind(ptr,val);
  case OZ_VAR_CT:      return ((OzCtVariable*) ov)->bind(ptr,val);
  case OZ_VAR_EXT:     return ((ExtVar *) ov)->forceBindV(ptr,val);
  ExhaustiveSwitch();
  }
  return FAILED;
}

OZ_Return oz_var_addSusp(TaggedRef *v, Suspendable * susp) {
  OzVariable *ov=oz_getVar(v);
  switch(ov->getType()) {
  case OZ_VAR_FUTURE:
    return ((Future *) ov)->addSusp(v, susp);
  case OZ_VAR_EXT:
    return ((ExtVar *) ov)->addSuspV(v, susp);
  case OZ_VAR_SIMPLE:
    if (ozconf.useFutures || susp->isNoBlock()) {
      return oz_raise(E_ERROR, E_KERNEL, "block", 1, makeTaggedRef(v));
    }
    // fall through
  default:
    ov->addSuspSVar(susp);
    return SUSPEND;
  }
}

void oz_var_dispose(OzVariable *ov) {
  switch (ov->getType()){
  case OZ_VAR_SIMPLE:  ((SimpleVar *) ov)->dispose(); break;
  case OZ_VAR_FUTURE:  ((Future *) ov)->dispose(); break;
  case OZ_VAR_BOOL:    ((OzBoolVariable*) ov)->dispose(); break;
  case OZ_VAR_FD:      ((OzFDVariable*) ov)->dispose(); break;
  case OZ_VAR_OF:      ((OzOFVariable*) ov)->dispose(); break;
  case OZ_VAR_FS:      ((OzFSVariable*) ov)->dispose(); break;
  case OZ_VAR_CT:      ((OzCtVariable*) ov)->dispose(); break;
  case OZ_VAR_EXT:     ((ExtVar *) ov)->disposeV(); break;
  ExhaustiveSwitch();
  }
}

void oz_var_printStream(ostream &out, const char *s, OzVariable *cv, int depth)
{
  switch (cv->getType()) {
  case OZ_VAR_SIMPLE:
    out << s;
    ((SimpleVar *)cv)->printStream(out,depth); return;
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
    ((ExtVar *)cv)->printStreamV(out,depth); return;
  ExhaustiveSwitch();
  }
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
  case OZ_VAR_EXT:    return ((ExtVar *)cv)->getSuspListLengthV();
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

  switch (o->getType()){
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
 *  L=    R=| SI    | FU    | EX    | FD    | BO    | FS    | OF    | CT    |
 *  -------------------------------------------------------------------------
 *  SI      | NOOP  | --    | --    | --    | --    | --    | --    | --    |
 *  -------------------------------------------------------------------------
 *  FU      | --    | NOOP  | --    | --    | --    | --    | --    | --    |
 *  -------------------------------------------------------------------------
 *  EX      | --    | --    | NOOP  | --    | --    | --    | --    | --    |
 *  -------------------------------------------------------------------------
 *  FD      | R->FD | R->FD | R->FD | NOOP  | NOOP  | CLASH | CLASH | CLASH |
 *  -------------------------------------------------------------------------
 *  BO      | R->BO | R->BO | R->BO | NOOP  | NOOP  | CLASH | CLASH | CLASH |
 *  -------------------------------------------------------------------------
 *  FS      | R->FS | R->FS | R->FS | CLASH | CLASH | NOOP  | CLASH | CLASH |
 *  -------------------------------------------------------------------------
 *  OF      | R->OF | R->OF | R->OF | CLASH | CLASH | CLASH | NOOP  | CLASH |
 *  -------------------------------------------------------------------------
 *  CT      | R->CT | R->CT | R->CT | CLASH | CLASH | CLASH | CLASH | NOOP  |
 *  -------------------------------------------------------------------------
 *
 */

#define VARTP(T1,T2) ((T1<<3)|T2)
#define VTP(T1,T2)   VARTP(OZ_VAR_ ## T1, OZ_VAR_ ## T2)


OZ_Return oz_var_cast(TaggedRef * & fp, Board * fb, TypeOfVariable tt) {
  OzVariable * fv = tagged2CVar(*fp);

  TypeOfVariable ft = fv->getType();

  OzVariable * tv;
  
  switch (VARTP(tt,ft)) {

  case VTP(FD,FS):   case VTP(FD,OF):   case VTP(FD,CT):
  case VTP(BOOL,FS): case VTP(BOOL,OF): case VTP(BOOL,CT):
    
  case VTP(FS,FD): case VTP(FS,BOOL): case VTP(FS,OF): case VTP(FS,CT):
  case VTP(OF,FD): case VTP(OF,BOOL): case VTP(OF,FS): case VTP(OF,CT):
  case VTP(CT,FD): case VTP(CT,BOOL): case VTP(CT,FS): case VTP(CT,OF):
    return FAILED;

  case VTP(FD,SIMPLE): case VTP(FD,FUTURE): case VTP(FD,EXT):
    tv = new OzFDVariable(fb);
    break;

  case VTP(BOOL,SIMPLE): case VTP(BOOL,FUTURE): case VTP(BOOL,EXT):
    tv = new OzBoolVariable(fb);
    break;

  case VTP(FS,SIMPLE): case VTP(FS,FUTURE): case VTP(FS,EXT):
    tv = new OzFSVariable(fb);
    break;

  case VTP(OF,SIMPLE): case VTP(OF,FUTURE): case VTP(OF,EXT):
    tv = new OzOFVariable(fb);
    break;

  case VTP(CT,SIMPLE): case VTP(CT,FUTURE): case VTP(CT,EXT):
    tv = new OzCtVariable(((OzCtVariable *) fv)->getConstraint(), 
			  ((OzCtVariable *) fv)->getDefinition(), 
			  fb);
    break;
    
  default:
    return PROCEED;
  }
  
  if (am.inEqEq()) {
    (void) oz_var_bind(fv, fp, makeTaggedRef(newTaggedCVar(tv)));
  } else {
    oz_forceWakeUp(fv->getSuspListRef());
    *fp = makeTaggedRef(newTaggedCVar(tv));

  }

  Assert(oz_isRef(*fp));

  fp = tagged2Ref(*fp);

  return PROCEED;
}

#undef VTP
#undef VARTP
  

OZ_Term _var_status(OzVariable *cv) {
  Assert(cv->getType()==OZ_VAR_EXT);
  return ((ExtVar*)cv)->statusV();
}


VarStatus _var_check_status(OzVariable *cv) {
  Assert(cv->getType()==OZ_VAR_EXT);
  return ((ExtVar*)cv)->checkStatusV();
}

// dealing with global variables
// add assertions that right sides of bin and cast are global or values!
void bindGlobalVarToValue(OZ_Term * varptr, OZ_Term value)
{
  DEBUG_CONSTRAIN_CVAR(("bindGlobalVarToValue\n"));
  DoBindAndTrail(varptr, value);
}

void bindGlobalVar(OZ_Term * varptr_left, OZ_Term * varptr_right)
{
  DEBUG_CONSTRAIN_CVAR(("bindGlobalVar\n"));
  DoBindAndTrail(varptr_left, makeTaggedRef(varptr_right));
}

void castGlobalVar(OZ_Term * varptr_left, OZ_Term * varptr_right)
{
  DEBUG_CONSTRAIN_CVAR(("castGlobalVar\n"));
  DoBindAndTrail(varptr_left, makeTaggedRef(varptr_right));
}

void constrainGlobalVar(OZ_Term * varptr, OZ_FiniteDomain & fd)
{
  DEBUG_CONSTRAIN_CVAR(("constrainGlobalVar(fd)\n"));
  trail.pushVariable(varptr);
  OzFDVariable * fdvar = (OzFDVariable *) tagged2CVar(*varptr);
  fdvar->setDom(fd);
}

void constrainGlobalVar(OZ_Term * varptr, OZ_FSetConstraint &fs)
{
  DEBUG_CONSTRAIN_CVAR(("constrainGlobalVar(fs)\n"));
  trail.pushVariable(varptr);
  OzFSVariable * fsvar = (OzFSVariable *) tagged2CVar(*varptr);
  fsvar->setSet(fs);
}

void constrainGlobalVar(OZ_Term * varptr, OZ_Ct * ct)
{
  DEBUG_CONSTRAIN_CVAR(("constrainGlobalVar(ct)\n"));
  trail.pushVariable(varptr);
  OzCtVariable * ctvar = (OzCtVariable *) tagged2CVar(*varptr);
  ctvar->copyConstraint(ct); 
}

void constrainGlobalVar(OZ_Term * varptr, DynamicTable * dt)
{
  DEBUG_CONSTRAIN_CVAR(("constrainGlobalVar(of)\n"));
  trail.pushVariable(varptr);
  OzOFVariable * ofvar = (OzOFVariable *) tagged2CVar(*varptr);
  ofvar->dynamictable = dt->copyDynamicTable();
}

// dealing with local variables
void bindLocalVarToValue(OZ_Term * varptr, OZ_Term value)
{
  DEBUG_CONSTRAIN_CVAR(("bindLocalVarToValue\n"));
  DoBind(varptr, value);
}

void bindLocalVar(OZ_Term * varptr_left, OZ_Term * varptr_right)
{
  DEBUG_CONSTRAIN_CVAR(("bindLocalVar\n"));
  DoBind(varptr_left, makeTaggedRef(varptr_right));
}
