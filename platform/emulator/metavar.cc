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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "metavar.hh"
#endif

#include <string.h>

#include "am.hh"

#include "genvar.hh"
#include "metavar.hh"

/*
char * OZ_printMetaDefault(OZ_Term d)
{
  return toC(d);
}
*/

GenMetaVariable::GenMetaVariable(MetaTag * t, TaggedRef tr)
: data(tr), tag(t), GenCVariable(MetaVariable) { }


// The idea of this routine is to provide a universal unification
// service which just requires as constraint system dependent part
// two functions unify_meta_det and unify_meta_meta. These functions
// must be provided by the implementor of the constraint system and that's it.
// A meta variable is only successively unifyable with another meta variable
// or a determined term. Unification with other GenCVariables will fail.
OZ_Return GenMetaVariable::unifyV(TaggedRef * vptr, TaggedRef t,
                                  ByteCode *scp)
{
  if (oz_isRef(t)) {
    TaggedRef *tptr=tagged2Ref(t);
    t = *tptr;
    GenCVariable *cv=tagged2CVar(t);
    if (cv->getType() != MetaVariable) return FALSE;

    GenMetaVariable * term = (GenMetaVariable *) cv;
    TaggedRef result, trail = *vptr;

    *vptr = makeTaggedRef(tptr);
    mur_t ret_value = tag->unify_meta_meta(makeTaggedRef(vptr), getData(),
                                           makeTaggedRef(tptr), term->getData(),
                                           term->getTag(), &result);
    *vptr = trail;

#ifdef DEBUG_META
    DebugCode(printf("0x%x\n", ret_value));
#endif

    if (ret_value & meta_fail) return FALSE;

    // start unification for meta-var and meta-var

    Bool v_is_local = am.isLocalSVar(this);
    Bool t_is_local = am.isLocalSVar(term);
    switch (v_is_local + 2 * t_is_local) {
    case TRUE + 2 * TRUE: // v and t are local
      if (heapNewer(vptr, tptr)) { // bind v to t
        if (ret_value & meta_det) {
          propagate(suspList, pc_cv_unif);
          term->propagate(term->suspList, pc_cv_unif);
          doBind(tptr, result);
          doBind(vptr, result);
        } else {
          term->setData(result);
          propagate(suspList, pc_cv_unif);
          term->propagate(term->suspList, pc_cv_unif);
          relinkSuspListTo(term);
          doBind(vptr, makeTaggedRef(tptr));
        }
      } else { // bind t to v
        if (ret_value & meta_det) {
          propagate(suspList, pc_cv_unif);
          term->propagate(term->suspList, pc_cv_unif);
          doBind(vptr, result);
          doBind(tptr, result);
        } else {
          setData(result);
          propagate(suspList, pc_cv_unif);
          term->propagate(term->suspList, pc_cv_unif);
          term->relinkSuspListTo(this);
          doBind(tptr, makeTaggedRef(vptr));
        }
      }
      break;

    case TRUE + 2 * FALSE: // v is local and t is global
      if (ret_value & meta_right_constr) {
        if (ret_value & meta_det) {
          propagate(suspList, pc_cv_unif);
          term->propagate(term->suspList, pc_cv_unif);
          doBind(vptr, result);
          am.doBindAndTrail(tptr, result);
        } else {
          setData(result);
          propagate(suspList, pc_cv_unif);
          term->propagate(term->suspList, pc_cv_unif);
          DoBindAndTrailAndIP(tptr, makeTaggedRef(vptr), this, term);
        }
      } else {
        propagate(suspList, pc_cv_unif);
        term->propagate(term->suspList, pc_cv_unif);

        relinkSuspListTo(term, TRUE);
        doBind(vptr, makeTaggedRef(tptr));
      }
      break;

    case FALSE + 2 * TRUE: // v is global and t is local
      if (ret_value & meta_left_constr) {
        if(ret_value & meta_det) {
          propagate(suspList, pc_cv_unif);
          term->propagate(term->suspList, pc_cv_unif);
          doBind(tptr, result);
          am.doBindAndTrail(vptr, result);
        } else {
          term->setData(result);
          propagate(suspList, pc_cv_unif);
          term->propagate(term->suspList, pc_cv_unif);
          DoBindAndTrailAndIP(vptr, makeTaggedRef(tptr), term, this);
        }
      } else {
        propagate(suspList, pc_cv_unif);
        term->propagate(term->suspList, pc_cv_unif);

        term->relinkSuspListTo(this, TRUE);
        doBind(tptr, makeTaggedRef(vptr));
      }
      break;

    case FALSE + 2 * FALSE: // v and t is global
      if (ret_value & meta_det){
        if (scp==0) {
          propagate(suspList, pc_cv_unif);
          term->propagate(term->suspList, pc_cv_unif);
        }
        am.doBindAndTrail(vptr, result);
        am.doBindAndTrail(tptr, result);
      } else {
        GenMetaVariable * meta_var = new GenMetaVariable(tag, result);
        TaggedRef * var_val = newTaggedCVar(meta_var);
        if (scp==0) {
          propagate(suspList, pc_cv_unif);
          term->propagate(term->suspList, pc_cv_unif);
        }
        DoBindAndTrailAndIP(vptr, makeTaggedRef(var_val), meta_var, this);
        DoBindAndTrailAndIP(tptr, makeTaggedRef(var_val), meta_var, term);
      }
      break;

    default:
      error("unexpected case in unifyMeta");
      break;
    }
  } else {
    TaggedRef result, trail = *vptr;

    // bind temporarily to catch cycles
    *vptr = t;
    mur_t ret_value = tag->unify_meta_det(makeTaggedRef(vptr), getData(),
                                          t, OZ_typeOf(t),
                                          &result);
    *vptr = trail;

#ifdef DEBUG_META
    DebugCode(printf("meta-det 0x%x\n", ret_value));
#endif

    if (ret_value == meta_fail) return FALSE;

    if (scp==0) propagate(suspList, pc_propagator);

    if (am.isLocalSVar(this)) {
      doBind(vptr, result);
    } else {
      am.doBindAndTrail(vptr, result);
    }
  }
  return TRUE;
}

Bool GenMetaVariable::valid(TaggedRef v)
{
  Assert(!oz_isRef(v));

  return meta_fail != tag->unify_meta_det(0, getData(),
                                            v, OZ_typeOf(v), NULL);
}


Bool GenMetaVariable::isStrongerThan(TaggedRef var, TaggedRef vdata)
{
  mur_t ret_value = tag->unify_meta_meta(makeTaggedCVar(this), getData(),
                                         var, vdata, getTag(), NULL);

  if (ret_value == meta_fail) {
    warning("GenMetaVariable::isStrongerThan found inconsistency.");
    return FALSE;
  }

  return  (ret_value & meta_right_constr) ? TRUE : FALSE;
}

//-----------------------------------------------------------------------------
// Implementation of interface functions

OZ_MetaType OZ_introMetaTerm(OZ_UnifyMetaDet unify_md,
                            OZ_UnifyMetaMeta unify_mm,
                            OZ_PrintMeta print_m,
                            OZ_IsSingleValue sgl_val_m,
                            char * name_m)
{
  return OZ_MetaType(::new MetaTag(unify_md, unify_mm,
                                   print_m, sgl_val_m, ozstrdup(name_m)));
}

OZ_Term OZ_makeMetaTerm(OZ_MetaType t, OZ_Term d)
{
  return makeTaggedRef(newTaggedCVar(new GenMetaVariable((MetaTag *) t, d)));
}

OZ_Return OZ_constrainMetaTerm(OZ_Term v, OZ_MetaType t, OZ_Term d)
{
  TaggedRef v_deref = oz_deref(v), metaterm = OZ_makeMetaTerm(t, d);

  if (!oz_isVariable(v_deref) ||
      (OZ_isMetaTerm(v_deref) &&
       ((GenMetaVariable *) tagged2CVar(v_deref))->check(metaterm, t, d)
       == meta_unconstr)) {
    return PROCEED;
  }

  return OZ_unify(v, metaterm); // mm_u
}

OZ_Return OZ_suspendMetaProp(OZ_CFun OZ_self, OZ_Term * OZ_args, int OZ_arity)
{
  OZ_Thread thr = OZ_makeSuspendedThread(OZ_self, OZ_args, OZ_arity);
  Bool suspNotAdded = TRUE;

  for (int i = OZ_arity; i--; )
    if (!OZ_isSingleValue(OZ_args[i])) {
      OZ_addThread(OZ_args[i], thr);
      suspNotAdded = FALSE;
    }

  if (suspNotAdded)
    OZ_warning("No suspension added in OZ_suspendMetaProp.");

  return PROCEED;
}

OZ_MetaType OZ_getMetaTermType(OZ_Term v)
{
  v = oz_deref(v);
  if (isCVar(v) && tagged2CVar(v)->getType() == MetaVariable)
    return ((GenMetaVariable *) tagged2CVar(v))->getTag();
  return NULL;
}

void OZ_putMetaTermType(OZ_Term v, OZ_MetaType t)
{
  v = oz_deref(v);
  if (isCVar(v) && tagged2CVar(v)->getType() == MetaVariable)
    ((GenMetaVariable *) tagged2CVar(v))->putTag((MetaTag *)t);
}

OZ_Term OZ_getMetaTermAttr(OZ_Term v)
{
  v = oz_deref(v);
  if (isCVar(v) && tagged2CVar(v)->getType() == MetaVariable)
    return ((GenMetaVariable *) tagged2CVar(v))->getData();
  return makeTaggedNULL();
}

int OZ_isSingleValue(OZ_Term v)
{
  v = oz_deref(v);

  if (!oz_isVariable(v)) {
    return TRUE;
  } else if (OZ_isMetaTerm(v)) {
    return ((GenMetaVariable *) tagged2CVar(oz_deref(v)))->isSingleValue();
  } else {
    return FALSE;
  }
}


int OZ_areIdentVars(OZ_Term v1, OZ_Term v2) // replace by OZ_isEqualVars
{
  DEREF(v1, vptr1, vtag1);
  DEREF(v2, vptr2, vtag2);
  return oz_isVariable(vtag1) && (vptr1 == vptr2) ? PROCEED : FAILED;
}


OZ_Term OZ_makeHeapChunk(int s)
{
  HeapChunk * hc = new HeapChunk(s);
  return makeTaggedConst(hc);
}

#define NotHeapChunkWarning(T, F, R)                                        \
if (! OZ_isHeapChunk(T)) {                                                  \
  OZ_warning("Heap chunk expected in %s. Got 0x%x. Result undetermined.\n", \
             #F, T);                                                        \
  return R;                                                                 \
}

int OZ_getHeapChunkSize(TaggedRef t)
{
  NotHeapChunkWarning(t, OZ_getHeapChunkSize, 0);

  return ((HeapChunk *) tagged2Const(oz_deref(t)))->getChunkSize();
}

void * OZ_getHeapChunkData(TaggedRef t)
{
  NotHeapChunkWarning(t, OZ_getHeapChunk, NULL);

  return ((HeapChunk *) tagged2Const(oz_deref(t)))->getChunkData();
}

int OZ_isHeapChunk(OZ_Term t)
{
  return oz_isHeapChunk(oz_deref(t));
}


int OZ_isMetaTerm(OZ_Term t)
{
  t = oz_deref(t);
  return isCVar(t) ? tagged2CVar(t)->getType()==MetaVariable : FALSE;
}


OZ_TermType OZ_typeOf(OZ_Term t)
{
  t = oz_deref(t);
  if (oz_isCell(t)) return OZ_Type_Cell;
  if (oz_isCons(t)) return OZ_Type_Cons;
  if (oz_isHeapChunk(t)) return OZ_Type_HeapChunk;
  if (isCVar(t)) return OZ_Type_CVar;
  if (oz_isFloat(t)) return OZ_Type_Float;
  if (oz_isInt(t)) return OZ_Type_Int;
  if (oz_isLiteral(t)) return OZ_Type_Literal;
  if (oz_isProcedure(t)) return OZ_Type_Procedure;
  if (oz_isSRecord(t)) return OZ_Type_Record;
  if (oz_isSTuple(t)) return OZ_Type_Tuple;
  if (oz_isVariable(t)) return OZ_Type_Var;
  return OZ_Type_Unknown;
}

//-----------------------------------------------------------------------------
// Built-ins


OZ_BI_define(BImetaIsVar, 1,0)
{
  return isGenMetaVar(oz_deref(OZ_in(0))) ? PROCEED : FAILED;
} OZ_BI_end

OZ_BI_define(BImetaIsVarB, 1,1)
{
  OZ_RETURN(isGenMetaVar(oz_deref(OZ_in(0))) ? NameTrue : NameFalse);
} OZ_BI_end

#define OZ_getINDeref(N, V, VPTR, VTAG) \
  OZ_Term V = OZ_in(N); \
  DEREF(V, VPTR, VTAG);

OZ_C_proc_begin(BImetaGetDataAsAtom, 2)
{
  ExpectedTypes("GenMetaVariable<ConstraintData>,Atom");

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(! oz_isVariable(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenMetaVar(var, vartag)) {
    return OZ_unify(makeTaggedAtom(((GenMetaVariable *) tagged2CVar(var))->toString(ozconf.printDepth)),
                    OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    OZ_addThread(makeTaggedRef(varptr),
                 OZ_makeSuspendedThread(OZ_self, OZ_args, OZ_arity));
    return PROCEED;
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BImetaGetStrength, 2)
{
  ExpectedTypes("GenMetaVariable<ConstraintData>,ConstraintData");

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(! oz_isVariable(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenMetaVar(var, vartag)) {
    return OZ_unify(((GenMetaVariable *) tagged2CVar(var))->getData(),
                    OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    OZ_addThread(makeTaggedRef(varptr),
                 OZ_makeSuspendedThread(OZ_self, OZ_args, OZ_arity));
    return PROCEED;
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BImetaGetNameAsAtom, 2)
{
  ExpectedTypes("GenMetaVariable<ConstraintData>,Atom");

  OZ_getCArgDeref(0, var, varptr, vartag);

  if(! oz_isVariable(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));
  } else if (isGenMetaVar(var, vartag)) {
    return
      OZ_unify(makeTaggedAtom(((GenMetaVariable*)tagged2CVar(var))->getName()),
               OZ_getCArg(1));
  } else if (isNotCVar(vartag)) {
    OZ_addThread(makeTaggedRef(varptr),
                 OZ_makeSuspendedThread(OZ_self, OZ_args, OZ_arity));
    return PROCEED;
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BImetaWatchVar, 2)
{
  ExpectedTypes("GenMetaVariable<ConstraintData>,ConstraintData");

  OZ_getCArgDeref(0, v, vptr, vtag);

  if(! oz_isVariable(vtag)) {
    return PROCEED;
  } else if (isGenMetaVar(v, vtag)) {
    if (((GenMetaVariable*)tagged2CVar(v))->isStrongerThan(makeTaggedRef(vptr),
                                                           oz_deref(OZ_args[1])))
      return PROCEED;

    OZ_addThread(makeTaggedRef(vptr),
                 OZ_makeSuspendedThread(OZ_self, OZ_args, OZ_arity));
    return PROCEED;
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end

OZ_C_proc_begin(BImetaWatchVarB, 3)
{
  ExpectedTypes("GenMetaVariable<ConstraintData>,ConstraintData");

  OZ_getCArgDeref(0, v, vptr, vtag);

  if(! oz_isVariable(vtag)) {
    return (OZ_unify (OZ_getCArg (2), NameTrue));
  } else if (isGenMetaVar(v, vtag)) {
    if (((GenMetaVariable*)tagged2CVar(v))->isStrongerThan(makeTaggedRef(vptr),
                                                           oz_deref(OZ_args[1])))
      return (OZ_unify (OZ_getCArg (2), NameTrue));

    OZ_addThread(makeTaggedRef(vptr),
                 OZ_makeSuspendedThread(OZ_self, OZ_args, OZ_arity));
    return (OZ_unify (OZ_getCArg (2), NameTrue));
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end


#if defined(OUTLINE)
#define inline
#include "metavar.icc"
#undef inline
#endif
