 /*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(__GNUC__) && !defined(NOPRAGMA)
#pragma implementation "metavar.hh"
#endif


#include "genvar.hh"
#include "cell.hh"
#include "builtins.hh"


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
				Bool prop)
{
  Assert(! isNotCVar(t));
  
  if (isCVar(t)) {
    if (tagged2CVar(t)->getType() != MetaVariable) return FALSE;

    GenMetaVariable * term = (GenMetaVariable *) tagged2CVar(t);
    TaggedRef result, trail = v;

    *vptr = makeTaggedRef(tptr);
    mur_t ret_value = tag->unify_meta_meta(getData(),
					   term->getData(), term->getTag(),
					   &result);
    *vptr = trail;
    
#ifdef DEBUG_META
    DebugCode(printf("0x%x\n", ret_value));
#endif
    
    if (ret_value == meta_failed) return FALSE;

    // start unification for meta-var and meta-var
    
    Bool v_is_local = (prop && isLocalVariable());
    Bool t_is_local = (prop && term->isLocalVariable());
    switch (v_is_local + 2 * t_is_local) {
    case TRUE + 2 * TRUE: // v and t are local
      if (heapNewer(vptr, tptr)) { // bind v to t
	if (ret_value & meta_determined) {
	  propagate(v, suspList, result, pc_cv_unif);
	  term->propagate(t, term->suspList, result, pc_cv_unif);
	  doBind(tptr, result);
	  doBind(vptr, result);
	} else {
	  term->setData(result);
	  propagate(v, suspList, makeTaggedRef(tptr), pc_cv_unif);
	  term->propagate(t, term->suspList, makeTaggedRef(vptr), pc_cv_unif);
	  relinkSuspListTo(term);
	  doBind(vptr, makeTaggedRef(tptr));
	}
      } else { // bind t to v
	if (ret_value & meta_determined) {
	  propagate(v, suspList, result, pc_cv_unif);
	  term->propagate(t, term->suspList, result, pc_cv_unif);
	  doBind(vptr, result);
	  doBind(tptr, result);
	} else {
	  setData(result);
	  propagate(v, suspList, makeTaggedRef(tptr), pc_cv_unif);
	  term->propagate(t, term->suspList, makeTaggedRef(vptr), pc_cv_unif);
	  term->relinkSuspListTo(this);
	  doBind(tptr, makeTaggedRef(vptr));
	}
      }
      break;
      
    case TRUE + 2 * FALSE: // v is local and t is global
      if (ret_value & meta_right_constrained) {
	if (ret_value & meta_determined) {
	  propagate(v, suspList, result, pc_cv_unif);
	  term->propagate(t, term->suspList, result, pc_cv_unif);
	  term->addSuspension(new Suspension(am.currentBoard));
	  doBind(vptr, result);
	  doBindAndTrail(t, tptr, result);
	} else {
	  setData(result);
	  propagate(v, suspList, makeTaggedRef(tptr), pc_cv_unif);
	  term->propagate(t, term->suspList, makeTaggedRef(vptr), pc_cv_unif);
	  term->addSuspension(new Suspension(am.currentBoard));
	  doBindAndTrailAndIP(t, tptr, makeTaggedRef(vptr), this, term, prop);
	}
      } else {
	propagate(v, suspList, makeTaggedRef(tptr), pc_cv_unif);
	term->propagate(t, term->suspList, makeTaggedRef(vptr), pc_cv_unif);

	relinkSuspListTo(term, TRUE);
	doBind(vptr, makeTaggedRef(tptr));
      }
      break;
      
    case FALSE + 2 * TRUE: // v is global and t is local
      if (ret_value & meta_left_constrained) {
	if(ret_value & meta_determined) {
	  propagate(v, suspList, result, pc_cv_unif);
	  term->propagate(t, term->suspList, result, pc_cv_unif);
	  addSuspension(new Suspension(am.currentBoard));
	  doBind(tptr, result);
	  doBindAndTrail(v, vptr, result);
	} else {
	  term->setData(result);
	  propagate(v, suspList, makeTaggedRef(tptr), pc_cv_unif);
	  term->propagate(t, term->suspList, makeTaggedRef(vptr), pc_cv_unif);
	  addSuspension(new Suspension(am.currentBoard));
	  doBindAndTrailAndIP(v, vptr, makeTaggedRef(tptr), term, this, prop);
	}
      } else {
	propagate(v, suspList, makeTaggedRef(tptr), pc_cv_unif);
	term->propagate(t, term->suspList, makeTaggedRef(vptr), pc_cv_unif);

	term->relinkSuspListTo(this, TRUE);
	doBind(tptr, makeTaggedRef(vptr));
      }
      break;

    case FALSE + 2 * FALSE: // v and t is global
      if (ret_value & meta_determined){
	if (prop) {
	  propagate(v, suspList, result, pc_cv_unif);
	  term->propagate(t, term->suspList, result, pc_cv_unif);
	}
	doBindAndTrail(v, vptr, result);
	doBindAndTrail(t, tptr, result);
      } else {
	GenMetaVariable * meta_var = new GenMetaVariable(tag, result);
	TaggedRef * var_val = newTaggedCVar(meta_var);
	if (prop) {
	  propagate(v, suspList, makeTaggedRef(var_val), pc_cv_unif);
	  term->propagate(t, term->suspList, makeTaggedRef(var_val),
			  pc_cv_unif);
	}
	doBindAndTrailAndIP(v, vptr, makeTaggedRef(var_val), meta_var, this, prop);
	doBindAndTrailAndIP(t, tptr, makeTaggedRef(var_val), meta_var, term, prop);
      }
      if (prop) {
	Suspension * susp = new Suspension(am.currentBoard);
	term->addSuspension(susp);
	addSuspension(susp);
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
    mur_t ret_value = tag->unify_meta_det(getData(),
					  t, OZ_typeOf(t),
					  &result);
    if (vptr && tptr) *vptr = trail;
    
#ifdef DEBUG_META
    DebugCode(printf("meta-det 0x%x\n", ret_value));
#endif
    
    if (ret_value == meta_failed) return FALSE;
    
    if (prop) propagate(v, suspList, result, pc_propagator);

    if (prop && isLocalVariable()) {
      doBind(vptr, result);
    } else {
      addSuspension(new Suspension(am.currentBoard));
      doBindAndTrail(v, vptr, result);
    }
  }
  return TRUE;
}

Bool GenMetaVariable::valid(TaggedRef v)
{
  Assert(!isRef(v));
  
  TaggedRef d;
  
  return meta_failed != tag->unify_meta_det(getData(), v, OZ_typeOf(v), &d);
}


Bool GenMetaVariable::isStrongerThan(TaggedRef d)
{
  TaggedRef result;
  
  mur_t ret_value = tag->unify_meta_meta(getData(), d, getTag(), &result);
  
  if (ret_value == meta_failed) {
    warning("GenMetaVariable::isStrongerThan found inconsistency.");
    return FALSE;
  }

  return  (ret_value & meta_right_constrained) ? TRUE : FALSE;
}

//-----------------------------------------------------------------------------
// Implementation of interface functions

OZ_MetaType OZ_introMetaVar(OZ_UnifyMetaDet unify_md,
			    OZ_UnifyMetaMeta unify_mm,
			    OZ_PrintMeta print_m,
			    OZ_IsUnique unique_m,
			    char * name_m)
{
  return OZ_MetaType(::new MetaTag(unify_md, unify_mm,
				   print_m, unique_m, strdup(name_m)));
}

OZ_Term OZ_makeMetaVar(OZ_MetaType t, OZ_Term d)
{
  return makeTaggedRef(newTaggedCVar(new GenMetaVariable((MetaTag *) t, d)));
}

OZ_Bool OZ_constrainMetaVar(OZ_Term v, OZ_MetaType t, OZ_Term d)
{
  TaggedRef v_deref = deref(v);
  
  if (!isAnyVar(v_deref) ||
      (OZ_isMetaVar(v_deref) &&
       ((GenMetaVariable *) tagged2CVar(v_deref))->check(t, d)
       == meta_unconstrained)) {
    return PROCEED;
  }

  return OZ_unify(v, OZ_makeMetaVar(t, d));
}  

OZ_Bool OZ_suspendMetaProp(OZ_CFun OZ_self, OZ_Term * OZ_args, int OZ_arity)
{
  OZ_Suspension susp = OZ_makeSuspension(OZ_self, OZ_args, OZ_arity);
  Bool suspNotAdded = TRUE;
  
  for (int i = OZ_arity; i--; )
    if (!OZ_isUnique(OZ_getCArg(i))) {
      OZ_addSuspension(OZ_args[i], susp);
      suspNotAdded = FALSE;
    }

  if (suspNotAdded)
    OZ_warning("No suspension added in OZ_suspendMetaProp.");
  
  return PROCEED;
}

OZ_MetaType OZ_getMetaVarType(OZ_Term v)
{
  v = deref(v);
  if (isCVar(v) && tagged2CVar(v)->getType() == MetaVariable)
    return ((GenMetaVariable *) tagged2CVar(v))->getTag();
  return NULL;
}

void OZ_putMetaVarType(OZ_Term v, OZ_MetaType t)
{
  v = deref(v);
  if (isCVar(v) && tagged2CVar(v)->getType() == MetaVariable)
    ((GenMetaVariable *) tagged2CVar(v))->putTag((MetaTag *)t);
}

OZ_Term OZ_getMetaVarData(OZ_Term v)
{
  v = deref(v);
  if (isCVar(v) && tagged2CVar(v)->getType() == MetaVariable)
    return ((GenMetaVariable *) tagged2CVar(v))->getData();
  return NULL;
}

int OZ_isUnique(OZ_Term v)
{
  v = deref(v);

  if (!isAnyVar(v)) {
    return TRUE;
  } else if (OZ_isMetaVar(v)) {
    return ((GenMetaVariable *) tagged2CVar(deref(v)))->isUnique();
  } else {
    return FALSE;
  }
}


int OZ_areIdentVars(OZ_Term v1, OZ_Term v2)
{
  DEREF(v1, vptr1, vtag1);
  DEREF(v2, vptr2, vtag2);
  return (vptr1 == vptr2) ? PROCEED : FAILED;
}


OZ_Term OZ_makeHeapChunk(int s)
{
  HeapChunk * hc = new HeapChunk(s);
  return makeTaggedConst(hc);
}

#define NotHeapChunkWarning(T, F, R)					      \
if (! OZ_isHeapChunk(T)) {						      \
  OZ_warning("Heap chunk expected in %s. Got 0x%x. Result undetermined.\n",   \
             #F, T);	                                                      \
  return R;								      \
}

int OZ_getHeapChunkSize(TaggedRef t)
{
  NotHeapChunkWarning(t, OZ_getHeapChunkSize, 0);
  
  return ((HeapChunk *) tagged2Const(t))->getChunkSize();
}

char * OZ_getHeapChunk(TaggedRef t, char * buf)
{
  NotHeapChunkWarning(t, OZ_getHeapChunk, NULL);

  HeapChunk * hc = (HeapChunk *) tagged2Const(t);
  char * hc_data = hc->getChunkData();
  int hc_size = hc->getChunkSize();
  
  if (! buf) buf = ::new char[hc_size];
  
  for (int i = hc_size; i--; )
    buf[i] = hc_data[i];
  
  return buf;
}

void OZ_putHeapChunk(OZ_Term t, char * buf)
{
  NotHeapChunkWarning(t, OZ_putHeapChunk, );
  
  HeapChunk * hc = (HeapChunk *) tagged2Const(t);
  char * hc_data = hc->getChunkData();
  
  for (int i = hc->getChunkSize(); i--; )
    hc_data[i] = buf[i];
}

int OZ_isHeapChunk(OZ_Term t)
{
  return isHeapChunk(t);
}


int OZ_isMetaVar(OZ_Term t)
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


OZ_C_proc_begin(BImetaGetDataAsAtom, 2)
{ 
  ExpectedTypes("GenMetaVariable<ConstraintData>,Atom");
  
  OZ_getCArgDeref(0, var, varptr, vartag);

  if(! isAnyVar(vartag)) {
    return OZ_unify(var, OZ_getCArg(1));   
  } else if (isGenMetaVar(var, vartag)) {
    return OZ_unify(makeTaggedAtom(((GenMetaVariable *) tagged2CVar(var))->toString()),
		    OZ_getCArg(1));   
  } else if (isNotCVar(vartag)) {
    OZ_addSuspension(makeTaggedRef(varptr),
		     OZ_makeSuspension(OZ_self, OZ_args, OZ_arity));
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
    OZ_addSuspension(makeTaggedRef(varptr),
		     OZ_makeSuspension(OZ_self, OZ_args, OZ_arity));
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
    OZ_addSuspension(makeTaggedRef(varptr),
		     OZ_makeSuspension(OZ_self, OZ_args, OZ_arity));
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
    if (((GenMetaVariable*)tagged2CVar(v))->isStrongerThan(deref(OZ_args[1])))
      return PROCEED;
    
    OZ_addSuspension(makeTaggedRef(vptr),
		     OZ_makeSuspension(OZ_self, OZ_args, OZ_arity));
    return PROCEED;
  } else {
    TypeError(0, "");
  }
}
OZ_C_proc_end


void BIinitMeta(void)
{
  BIadd("metaIsVar", 1, BImetaIsVar);
  BIadd("metaWatchVar", 2, BImetaWatchVar);
  BIadd("metaGetDataAsAtom", 2, BImetaGetDataAsAtom);
  BIadd("metaGetNameAsAtom", 2, BImetaGetNameAsAtom);
  BIadd("metaGetStrength", 2, BImetaGetStrength);
}

#if defined(OUTLINE)
#define inline
#include "metavar.icc"
#undef inline
#endif
