#if defined(__GNUC__) && !defined(NOPRAGMA)
#pragma implementation "metavar.hh"
#endif


#include "genvar.hh"
#include "cell.hh"

GenMetaVariable::meta_vars_t GenMetaVariable::meta_vars[maxmetavars];
unsigned GenMetaVariable::last_tag = 0;

ucr_t inconsistent_unifyMeta(int, OZ_TermType, OZ_Term,
                             int, OZ_TermType, OZ_Term,
                             OZ_Term *)
{
  return failed;
}

char * printMetaDefault(OZ_Term d)
{
  return tagged2String(d, 10);
}

unsigned metadummy = GenMetaVariable::introduceMetaVar("Inconsistent meta var",
                                                       inconsistent_unifyMeta);

GenMetaVariable::GenMetaVariable(unsigned ty, TaggedRef tr)
: data(tr), tag(ty), GenCVariable(MetaVariable)
{
  if (tag >= maxmetavars) {
    tag = 0;
    cout << "Maximum number of meta variables exceeded."
           << "Got tag: " << tag
           << " Introducing inconsistent meta variable." << endl;
  }
}

unsigned GenMetaVariable::introduceMetaVar(char * name,
                                           unifyMeta_t unify_data,
                                           printMeta_t print_data)
{
  if (last_tag >= maxmetavars)
    return 0;

  meta_vars[last_tag].name = name;
  meta_vars[last_tag].unify_data = unify_data;
  meta_vars[last_tag].print_data = print_data;
  return last_tag ++;
}


// The idea of routine is to provided a universal unification
// service which just requires as constraint system dependent part
// a function unify_data. This function must be provided by the
// implementor of the constraint system and that's it.
// A meta variable is only successively unifyable with a meta variable
// or a determined term. Conversion from one constraint variable
// one of another type needs explicite conversion.
Bool GenMetaVariable::unifyMeta(TaggedRef * vptr, TaggedRef v,
                                TaggedRef * tptr, TaggedRef t,
                                Bool prop)
{
  Assert(! isNotCVar(t));

  TypeOfTerm ttag = tagTypeOf(t), rt;
  TaggedRef result;

  if (isCVar(t)) {
    if (tagged2CVar(t)->getType() != MetaVariable) return FALSE;
    if (((GenMetaVariable *) tagged2CVar(t))->getMetaType() != getMetaType())
      return FALSE;
  }

  ucr_t ret_value =
    meta_vars[tag].unify_data(isLocalVariable(), OZ_typeOf(v), v,
                               isCVar(t) ? am.isLocalCVar(t) : FALSE,
                               OZ_typeOf(t), t,
                               &result);

#ifdef DEBUG_META
  DebugCode(printf("0x%x\n", ret_value));
#endif

  if (ret_value == failed) return FALSE;

  if (isCVar(t)) {
    GenMetaVariable * term = (GenMetaVariable *) tagged2CVar(t);

    Bool v_is_local = (prop && isLocalVariable());
    Bool t_is_local = (prop && term->isLocalVariable());
    switch (v_is_local + 2 * t_is_local) {
    case TRUE + 2 * TRUE: // v and t are local
      if (heapNewer(vptr, tptr)) { // bind v to t
        if (ret_value & determined) {
          propagate(v, suspList, result, pc_cv_unif);
          term->propagate(t, term->suspList, result, pc_cv_unif);
          doBind(tptr, result);
          doBind(vptr, result);
        } else {
          term->setData(result);
          propagate(v, suspList, TaggedRef(tptr), pc_cv_unif);
          term->propagate(t, term->suspList, TaggedRef(vptr), pc_cv_unif);
          relinkSuspListTo(term);
          doBind(vptr, TaggedRef(tptr));
        }
      } else { // bind t to v
        if (ret_value & determined) {
          propagate(v, suspList, result, pc_cv_unif);
          term->propagate(t, term->suspList, result, pc_cv_unif);
          doBind(vptr, result);
          doBind(tptr, result);
        } else {
          setData(result);
          propagate(v, suspList, TaggedRef(tptr), pc_cv_unif);
          term->propagate(t, term->suspList, TaggedRef(vptr), pc_cv_unif);
          term->relinkSuspListTo(this);
          doBind(tptr, TaggedRef(vptr));
        }
      }
      break;

    case TRUE + 2 * FALSE: // v is local and t is global
      if (ret_value & right_constrained) {
        if (ret_value & determined) {
          propagate(v, suspList, result, pc_cv_unif);
          term->propagate(t, term->suspList, result, pc_cv_unif);
          term->addSuspension(new Suspension(am.currentBoard));
          doBind(vptr, result);
          doBindAndTrail(t, tptr, result);
        } else {
          setData(result);
          propagate(v, suspList, TaggedRef(tptr), pc_cv_unif);
          term->propagate(t, term->suspList, TaggedRef(vptr), pc_cv_unif);
          term->addSuspension(new Suspension(am.currentBoard));
          doBindAndTrail(t, tptr, TaggedRef(vptr));
        }
      } else {
        propagate(v, suspList, TaggedRef(tptr), pc_cv_unif);
        term->propagate(t, term->suspList, TaggedRef(vptr), pc_cv_unif);

        relinkSuspListTo(term, TRUE);
        doBind(vptr, TaggedRef(tptr));
      }
      break;

    case FALSE + 2 * TRUE: // v is global and t is local
      if (ret_value & left_constrained) {
        if(ret_value & determined) {
          propagate(v, suspList, result, pc_cv_unif);
          term->propagate(t, term->suspList, result, pc_cv_unif);
          addSuspension(new Suspension(am.currentBoard));
          doBind(tptr, result);
          doBindAndTrail(v, vptr, result);
        } else {
          term->setData(result);
          propagate(v, suspList, TaggedRef(tptr), pc_cv_unif);
          term->propagate(t, term->suspList, TaggedRef(vptr), pc_cv_unif);
          addSuspension(new Suspension(am.currentBoard));
          doBindAndTrail(v, vptr, TaggedRef(tptr));
        }
      } else {
        propagate(v, suspList, TaggedRef(tptr), pc_cv_unif);
        term->propagate(t, term->suspList, TaggedRef(vptr), pc_cv_unif);

        term->relinkSuspListTo(this, TRUE);
        doBind(tptr, TaggedRef(vptr));
      }
      break;

    case FALSE + 2 * FALSE: // v and t is global
      if (ret_value & determined){
        if (prop) {
          propagate(v, suspList, result, pc_cv_unif);
          term->propagate(t, term->suspList, result, pc_cv_unif);
        }
        doBindAndTrail(v, vptr, result);
        doBindAndTrail(t, tptr, result);
      } else {
        TaggedRef * var_val = newTaggedCVar(new GenMetaVariable(tag, result));
        if (prop) {
          propagate(v, suspList, TaggedRef(var_val), pc_cv_unif);
          term->propagate(t, term->suspList, TaggedRef(var_val),
                          pc_cv_unif);
        }
        doBindAndTrail(v, vptr, TaggedRef(var_val));
        doBindAndTrail(t, tptr, TaggedRef(var_val));
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
  TaggedRef dummy;
  return failed == meta_vars[tag].unify_data(FALSE, OZ_Type_CVar,
                                             makeTaggedCVar(this),
                                             FALSE, OZ_typeOf(v), v,
                                             &dummy) ? FALSE : TRUE;
}
//-----------------------------------------------------------------------------
// implementation of interface functions

unsigned introduceMetaVar(char * n, unifyMeta_t u, printMeta_t p)
{
  return GenMetaVariable::introduceMetaVar(n, u, p);
}

OZ_Bool makeMetaVar(OZ_Term v, unsigned t, OZ_Term d)
{
  return OZ_unify(v, TaggedRef(newTaggedCVar(new GenMetaVariable(t, d))));
}


void constrainMetaVar(int d, OZ_Term v, OZ_Term c)
{
  TaggedRef v_deref = deref(v);

  if (d) {
    if (OZ_unify(c, v) == FAILED)
      warning("Found inconsistency when constraining meta variable.");
  } else if (am.isLocalCVar(v_deref)) {
    ((GenMetaVariable *) tagged2CVar(v_deref))->constrainVar(v_deref, c);
  } else {
    if (makeMetaVar(v, getTypeMetaVar(v_deref), c) == FAILED)
      warning("Found inconsistency when constraining meta variable.");
  }
}


unsigned getTypeMetaVar(OZ_Term v)
{
  v = deref(v);
  if (isCVar(v) && tagged2CVar(v)->getType() == MetaVariable)
    return ((GenMetaVariable *) tagged2CVar(v))->getMetaType();
  return 0;
}

OZ_Term getDataMetaVar(OZ_Term v)
{
  v = deref(v);
  if (isCVar(v) && tagged2CVar(v)->getType() == MetaVariable)
    return ((GenMetaVariable *) tagged2CVar(v))->getData();
  return 0;
}


int areIdentVars(OZ_Term v1, OZ_Term v2)
{
  DEREF(v1, vptr1, vtag1);
  DEREF(v2, vptr2, vtag2);
  return (vptr1 == vptr2) ? PROCEED : FAILED;
}


TaggedRef makeChunk(int s)
{
  HeapChunk * hc = new HeapChunk(s);
  return makeTaggedConst(hc);
}

int getChunkSize(TaggedRef t)
{
  Assert(isChunk(t));
  return ((HeapChunk *) tagged2Const(t))->getChunkSize();
}

void readChunkDataFromHeap(TaggedRef t, char * buf)
{
  Assert(isChunk(t));

  HeapChunk * hc = (HeapChunk *) tagged2Const(t);
  char * hc_data = hc->getChunkData();

  for (int i = hc->getChunkSize(); i--; )
    buf[i] = hc_data[i];
}

void writeChunkDataToHeap(TaggedRef t, char * buf)
{
  Assert(isChunk(t));

  HeapChunk * hc = (HeapChunk *) tagged2Const(t);
  char * hc_data = hc->getChunkData();

  for (int i = hc->getChunkSize(); i--; )
    hc_data[i] = buf[i];
}

int isChunk(TaggedRef t)
{
  t = deref(t);
  return tagTypeOf(t)==CONST ? tagged2Const(t)->getType()==Co_Chunk : FALSE;
}

int isMetaVar(TaggedRef t)
{
  t = deref(t);
  return isCVar(t) ? tagged2CVar(t)->getType()==MetaVariable : FALSE;
}

OZ_TermType OZ_typeOf(OZ_Term t)
{
  t = deref(t);
  if (isCell(t)) return OZ_Type_Cell;
  if (isCons(t)) return OZ_Type_Cons;
  if (isConst(t)) return OZ_Type_Chunk;
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
