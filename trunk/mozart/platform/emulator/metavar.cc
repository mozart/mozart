/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
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
  return tagged2String(d, 10);
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
Bool GenMetaVariable::unifyMeta(TaggedRef * vptr, TaggedRef v,
				TaggedRef * tptr, TaggedRef t,
				ByteCode *scp)
{
  Assert(! isNotCVar(t));
  
  if (isCVar(t)) {
    if (tagged2CVar(t)->getType() != MetaVariable) return FALSE;

    GenMetaVariable * term = (GenMetaVariable *) tagged2CVar(t);
    TaggedRef result, trail = v;

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
    
    Bool v_is_local = (scp==0 && am.isLocalSVar(this));
    Bool t_is_local = (scp==0 && am.isLocalSVar(term));
    switch (v_is_local + 2 * t_is_local) {
    case TRUE + 2 * TRUE: // v and t are local
      if (heapNewer(vptr, tptr)) { // bind v to t
	if (ret_value & meta_det) {
	  propagate(v, suspList, pc_cv_unif);
	  term->propagate(t, term->suspList, pc_cv_unif);
	  doBind(tptr, result);
	  doBind(vptr, result);
	} else {
	  term->setData(result);
	  propagate(v, suspList, pc_cv_unif);
	  term->propagate(t, term->suspList, pc_cv_unif);
	  relinkSuspListTo(term);
	  doBind(vptr, makeTaggedRef(tptr));
	}
      } else { // bind t to v
	if (ret_value & meta_det) {
	  propagate(v, suspList, pc_cv_unif);
	  term->propagate(t, term->suspList, pc_cv_unif);
	  doBind(vptr, result);
	  doBind(tptr, result);
	} else {
	  setData(result);
	  propagate(v, suspList, pc_cv_unif);
	  term->propagate(t, term->suspList, pc_cv_unif);
	  term->relinkSuspListTo(this);
	  doBind(tptr, makeTaggedRef(vptr));
	}
      }
      break;
      
    case TRUE + 2 * FALSE: // v is local and t is global
      if (ret_value & meta_right_constr) {
	if (ret_value & meta_det) {
	  propagate(v, suspList, pc_cv_unif);
	  term->propagate(t, term->suspList, pc_cv_unif);
	  doBind(vptr, result);
	  am.doBindAndTrail(t, tptr, result);
	} else {
	  setData(result);
	  propagate(v, suspList, pc_cv_unif);
	  term->propagate(t, term->suspList, pc_cv_unif);
	  am.doBindAndTrailAndIP(t, tptr, makeTaggedRef(vptr), this, term, scp);
	}
      } else {
	propagate(v, suspList, pc_cv_unif);
	term->propagate(t, term->suspList, pc_cv_unif);

	relinkSuspListTo(term, TRUE);
	doBind(vptr, makeTaggedRef(tptr));
      }
      break;
      
    case FALSE + 2 * TRUE: // v is global and t is local
      if (ret_value & meta_left_constr) {
	if(ret_value & meta_det) {
	  propagate(v, suspList, pc_cv_unif);
	  term->propagate(t, term->suspList, pc_cv_unif);
	  doBind(tptr, result);
	  am.doBindAndTrail(v, vptr, result);
	} else {
	  term->setData(result);
	  propagate(v, suspList, pc_cv_unif);
	  term->propagate(t, term->suspList, pc_cv_unif);
	  am.doBindAndTrailAndIP(v, vptr, makeTaggedRef(tptr), term, this, scp);
	}
      } else {
	propagate(v, suspList, pc_cv_unif);
	term->propagate(t, term->suspList, pc_cv_unif);

	term->relinkSuspListTo(this, TRUE);
	doBind(tptr, makeTaggedRef(vptr));
      }
      break;

    case FALSE + 2 * FALSE: // v and t is global
      if (ret_value & meta_det){
	if (scp==0) {
	  propagate(v, suspList, pc_cv_unif);
	  term->propagate(t, term->suspList, pc_cv_unif);
	}
	am.doBindAndTrail(v, vptr, result);
	am.doBindAndTrail(t, tptr, result);
      } else {
	GenMetaVariable * meta_var = new GenMetaVariable(tag, result);
	TaggedRef * var_val = newTaggedCVar(meta_var);
	if (scp==0) {
	  propagate(v, suspList, pc_cv_unif);
	  term->propagate(t, term->suspList, pc_cv_unif);
	}
	am.doBindAndTrailAndIP(v, vptr, makeTaggedRef(var_val), meta_var, this, scp);
	am.doBindAndTrailAndIP(t, tptr, makeTaggedRef(var_val), meta_var, term, scp);
      }
      break;
      
    default:
      error("unexpected case in unifyMeta");
      break;
    }
  } else {
    TaggedRef result, trail = v;

    // bind temporarily to catch cycles
    if (vptr && tptr) *vptr = makeTaggedRef(tptr);
    mur_t ret_value = tag->unify_meta_det(makeTaggedRef(vptr), getData(),
					  t, OZ_typeOf(t),
					  &result);
    if (vptr && tptr) *vptr = trail;
    
#ifdef DEBUG_META
    DebugCode(printf("meta-det 0x%x\n", ret_value));
#endif
    
    if (ret_value == meta_fail) return FALSE;
    
    if (scp==0) propagate(v, suspList, pc_propagator);

    if (scp==0 && am.isLocalSVar(this)) {
      doBind(vptr, result);
    } else {
      am.doBindAndTrail(v, vptr, result);
    }
  }
  return TRUE;
}

Bool GenMetaVariable::valid(TaggedRef v)
{
  Assert(!isRef(v));
  
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
  TaggedRef v_deref = deref(v), metaterm = OZ_makeMetaTerm(t, d);
  
  if (!isAnyVar(v_deref) ||
      (OZ_isMetaTerm(v_deref) &&
       ((GenMetaVariable *) tagged2CVar(v_deref))->check(metaterm, t, d)
       == meta_unconstr)) {
    return PROCEED;
  }

  return OZ_unify(v, metaterm);
}  

OZ_Return OZ_suspendMetaProp(OZ_CFun OZ_self, OZ_Term * OZ_args, int OZ_arity)
{
  OZ_Thread thr = OZ_makeSuspendedThread(OZ_self, OZ_args, OZ_arity);
  Bool suspNotAdded = TRUE;
  
  for (int i = OZ_arity; i--; )
    if (!OZ_isSingleValue(OZ_getCArg(i))) {
      OZ_addThread(OZ_args[i], thr);
      suspNotAdded = FALSE;
    }

  if (suspNotAdded)
    OZ_warning("No suspension added in OZ_suspendMetaProp.");
  
  return PROCEED;
}

OZ_MetaType OZ_getMetaTermType(OZ_Term v)
{
  v = deref(v);
  if (isCVar(v) && tagged2CVar(v)->getType() == MetaVariable)
    return ((GenMetaVariable *) tagged2CVar(v))->getTag();
  return NULL;
}

void OZ_putMetaTermType(OZ_Term v, OZ_MetaType t)
{
  v = deref(v);
  if (isCVar(v) && tagged2CVar(v)->getType() == MetaVariable)
    ((GenMetaVariable *) tagged2CVar(v))->putTag((MetaTag *)t);
}

OZ_Term OZ_getMetaTermAttr(OZ_Term v)
{
  v = deref(v);
  if (isCVar(v) && tagged2CVar(v)->getType() == MetaVariable)
    return ((GenMetaVariable *) tagged2CVar(v))->getData();
  return makeTaggedNULL();
}

int OZ_isSingleValue(OZ_Term v)
{
  v = deref(v);

  if (!isAnyVar(v)) {
    return TRUE;
  } else if (OZ_isMetaTerm(v)) {
    return ((GenMetaVariable *) tagged2CVar(deref(v)))->isSingleValue();
  } else {
    return FALSE;
  }
}


int OZ_areIdentVars(OZ_Term v1, OZ_Term v2) // replace by OZ_isEqualVars
{
  DEREF(v1, vptr1, vtag1);
  DEREF(v2, vptr2, vtag2);
  return isAnyVar(vtag1) && (vptr1 == vptr2) ? PROCEED : FAILED;
}


OZ_Term OZ_makeHeapChunk(int s)
{
  HeapChunk * hc = new HeapChunk(s);
  return makeTaggedConst(hc);
}

#define NotHeapChunkWarning(T, F, R)					    \
if (! OZ_isHeapChunk(T)) {						    \
  OZ_warning("Heap chunk expected in %s. Got 0x%x. Result undetermined.\n", \
             #F, T);							    \
  return R;								    \
}

int OZ_getHeapChunkSize(TaggedRef t)
{
  NotHeapChunkWarning(t, OZ_getHeapChunkSize, 0);
  
  return ((HeapChunk *) tagged2Const(deref(t)))->getChunkSize();
}

char * OZ_getHeapChunkData(TaggedRef t)
{
  NotHeapChunkWarning(t, OZ_getHeapChunk, NULL);
  
  return ((HeapChunk *) tagged2Const(deref(t)))->getChunkData();
}

int OZ_isHeapChunk(OZ_Term t)
{
  return isHeapChunk(deref(t));
}


int OZ_isMetaTerm(OZ_Term t)
{
  t = deref(t);
  return isCVar(t) ? tagged2CVar(t)->getType()==MetaVariable : FALSE;
}


OZ_TermType OZ_typeOf(OZ_Term t)
{
  t = deref(t);
  if (isCell(t)) return OZ_Type_Cell;
  if (isCons(t)) return OZ_Type_Cons;
  if (isHeapChunk(t)) return OZ_Type_HeapChunk;
  if (isCVar(t)) return OZ_Type_CVar;
  if (isFloat(t)) return OZ_Type_Float;
  if (isSmallInt(t) || isBigInt(t)) return OZ_Type_Int;
  if (isLiteral(t)) return OZ_Type_Literal;
  if (isProcedure(t)) return OZ_Type_Procedure;
  if (isSRecord(t)) return OZ_Type_Record;
  if (isSTuple(t)) return OZ_Type_Tuple;
  if (isAnyVar(t)) return OZ_Type_Var;
  return OZ_Type_Unknown;
}

//-----------------------------------------------------------------------------
// Built-ins


OZ_C_proc_begin(BImetaIsVar, 1)
{ 
  return isGenMetaVar(deref(OZ_getCArg(0))) ? PROCEED : FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BImetaIsVarB, 2)
{ 
  return (OZ_unify 
	  (OZ_getCArg(1), 
	   isGenMetaVar(deref(OZ_getCArg(0))) ? NameTrue : NameFalse));
}
OZ_C_proc_end


OZ_C_proc_begin(BImetaGetDataAsAtom, 2)
{ 
  ExpectedTypes("GenMetaVariable<ConstraintData>,Atom");
  
  OZ_getCArgDeref(0, var, varptr, vartag);

  if(! isAnyVar(vartag)) {
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

  if(! isAnyVar(vartag)) {
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

  if(! isAnyVar(vartag)) {
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

  if(! isAnyVar(vtag)) {
    return PROCEED;
  } else if (isGenMetaVar(v, vtag)) {
    if (((GenMetaVariable*)tagged2CVar(v))->isStrongerThan(makeTaggedRef(vptr), 
							   deref(OZ_args[1])))
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

  if(! isAnyVar(vtag)) {
    return (OZ_unify (OZ_getCArg (2), NameTrue));
  } else if (isGenMetaVar(v, vtag)) {
    if (((GenMetaVariable*)tagged2CVar(v))->isStrongerThan(makeTaggedRef(vptr), 
							   deref(OZ_args[1])))
      return (OZ_unify (OZ_getCArg (2), NameTrue));
    
    OZ_addThread(makeTaggedRef(vptr),
		 OZ_makeSuspendedThread(OZ_self, OZ_args, OZ_arity));
    return (OZ_unify (OZ_getCArg (2), NameTrue));
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end


static
BIspec metaSpec[] = {
  {"metaIsVar", 1, BImetaIsVar},
  {"metaIsVarB", 2, BImetaIsVarB},
  {"metaWatchVar", 2, BImetaWatchVar},
  {"metaWatchVarB", 3, BImetaWatchVarB},
  {"metaGetDataAsAtom", 2, BImetaGetDataAsAtom},
  {"metaGetNameAsAtom", 2, BImetaGetNameAsAtom},
  {"metaGetStrength", 2, BImetaGetStrength},
  {0,0,0,0}
};


void BIinitMeta(void)
{
  BIaddSpec(metaSpec);
}

#if defined(OUTLINE)
#define inline
#include "metavar.icc"
#undef inline
#endif
