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

int oz_raise(OZ_Term cat, OZ_Term key, char *label, int arity, ...);

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

OZ_Return oz_var_addSusp(TaggedRef *v, Suspendable * susp, int unstable)
{
  OzVariable *ov=oz_getVar(v);
  switch(ov->getType()) {
  case OZ_VAR_FUTURE:
    return ((Future *) ov)->addSusp(v, susp, unstable);
  case OZ_VAR_EXT:
    return ((ExtVar *) ov)->addSuspV(v, susp, unstable);
  case OZ_VAR_SIMPLE:
    if (ozconf.useFutures || susp->isNoBlock()) {
      return oz_raise(E_ERROR, E_KERNEL, "block", 1, makeTaggedRef(v));
    }
    // fall through
  default:
    ov->addSuspSVar(susp,unstable);
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

#define VAR_CAST_DEBUG

#ifdef VAR_CAST_DEBUG

static char * VCTN[] = {
  "EXT", "SIMPLE", "FUTURE", "BOOL", "FD", "OF", "FS", "CT"
};

#endif


OZ_Return oz_var_cast(TaggedRef * &fp, Board * fb, TypeOfVariable tt) {
  OzVariable * fv = tagged2CVar(*fp);

  TypeOfVariable ft = fv->getType();

#ifdef VAR_CAST_DEBUG

  if (ft != tt)
    printf("Variable casting: %s ---> %s\n", VCTN[(int) ft], VCTN[(int) tt]);

#endif

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
  
  if (oz_currentBoard() != fb)
    // The variable is not local, so trail the cast operation
    am.trail.pushCast(fp);
  
#ifdef VAR_CAST_DEBUG

  if (ft != tt && oz_currentBoard() != fb)
    printf("Variable casting: TRAILING\n");

#endif
  /*
   *  The new variable inherits all suspensions:
   *   see above to understand that no variable with multiple
   *   suspensionlist is casted!
   *
   */  

  tv->setSuspList(fv->unlinkSuspList());

  // Bind original variable to casted variable
  TaggedRef * tvt = newTaggedCVar(tv);

  *fp = makeTaggedRef(tvt);

  fp = tvt;

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

#ifdef TMUELLER
// dealing with global variables
// add assertions that right sides of bin and cast are global or values!
void bindGlobalVarToValue(OZ_Term *, OZ_Term)
{
}

void bindGlobalVar(OZ_Term *, OZ_Term *)
{
}

void castGlobalVar(OZ_Term *, OZ_Term *)
{
}

void constrainGlobalVar(OZ_Term *, OZ_FiniteDomain &)
{
}

void constrainGlobalVar(OZ_Term *, OZ_FSetConstraint &)
{
}

void constrainGlobalVar(OZ_Term *, OZ_Ct *, OZ_CtDefinition *)
{
}

// dealing with local variables
void bindLocalVarToValue(OZ_Term *, OZ_Term)
{
}

void bindLocalVar(OZ_Term *, OZ_Term *)
{
}

void castLocalVar(OZ_Term *, OZ_Term *)
{
}

void constrainLocalVar(OZ_Term *, OZ_FiniteDomain &)
{
}

void constrainLocalVar(OZ_Term *, OZ_FSetConstraint &)
{
}

void constrainLocalVar(OZ_Term *, OZ_Ct *, OZ_CtDefinition *)
{
}
#endif
