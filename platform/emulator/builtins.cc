/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */

#include "wsock.hh"

#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "dldwrap.h"
#if DLOPEN

#ifndef SUNOS_SPARC
#include <dlfcn.h>
#else

#define RTLD_NOW 1
extern "C" void * dlopen(char *, int);
extern "C" char * dlerror(void);
extern "C" void * dlsym(void *, char *);
extern "C" int dlclose(void *);

#endif
#endif

#ifdef IRIX5_MIPS
#include <bstring.h>
#include <sys/time.h>
#endif


#ifdef HPUX_700
#include <dl.h>
#endif

#include "am.hh"
#include "builtins.hh"

#include "genvar.hh"
#include "ofgenvar.hh"
#include "fdbuilti.hh"
#include "fdhook.hh"
#include "solve.hh"
#include "oz_cpi.hh"
#include "dictionary.hh"

/*===================================================================
 * Macros
 *=================================================================== */

#define NONVAR(X,term,tag)                      \
TaggedRef term = X;                             \
TypeOfTerm tag;                                 \
{ DEREF(term,_myTermPtr,myTag);                 \
  tag = myTag;                                  \
  if (isAnyVar(tag)) return SUSPEND;            \
}

// mm2
// Suspend on UVAR and SVAR:
#define NONSUVAR(X,term,tag)                    \
TaggedRef term = X;                             \
TypeOfTerm tag;                                 \
{ DEREF(term,_myTermPtr,myTag);                 \
  tag = myTag;                                  \
  if (isNotCVar(tag)) return SUSPEND;           \
}


#define DECLAREBI_USEINLINEREL1(Name,InlineName)        \
OZ_C_proc_begin(Name,1)                                 \
{                                                       \
  OZ_Term arg1 = OZ_getCArg(0);                         \
  OZ_Return state = InlineName(arg1);                   \
  if (state == SUSPEND) {                               \
    OZ_suspendOn(arg1);                                 \
  } else {                                              \
    return state;                                       \
  }                                                     \
}                                                       \
OZ_C_proc_end


#define DECLAREBI_USEINLINEREL2(Name,InlineName)        \
OZ_C_proc_begin(Name,2)                                 \
{                                                       \
  OZ_Term arg0 = OZ_getCArg(0);                         \
  OZ_Term arg1 = OZ_getCArg(1);                         \
  OZ_Return state = InlineName(arg0,arg1);              \
  if (state == SUSPEND) {                               \
    OZ_suspendOn2(arg0,arg1);                           \
  } else {                                              \
    return state;                                       \
  }                                                     \
}                                                       \
OZ_C_proc_end


#define DECLAREBI_USEINLINEREL3(Name,InlineName)        \
OZ_C_proc_begin(Name,3)                                 \
{                                                       \
  OZ_Term arg0 = OZ_getCArg(0);                         \
  OZ_Term arg1 = OZ_getCArg(1);                         \
  OZ_Term arg2 = OZ_getCArg(2);                         \
  OZ_Return state = InlineName(arg0,arg1,arg2);         \
  if (state == SUSPEND) {                               \
    OZ_suspendOn3(arg0,arg1,arg2);                      \
  } else {                                              \
    return state;                                       \
  }                                                     \
}                                                       \
OZ_C_proc_end


#define DECLAREBI_USEINLINEFUN1(Name,InlineName)        \
OZ_C_proc_begin(Name,2)                                 \
{                                                       \
  OZ_Term help;                                         \
                                                        \
  OZ_Term arg1 = OZ_getCArg(0);                         \
  OZ_Return state = InlineName(arg1,help);              \
  switch (state) {                                      \
  case SUSPEND:                                         \
    OZ_suspendOn(arg1);                                 \
  case PROCEED:                                         \
    return(OZ_unify(help,OZ_getCArg(1)));               \
  default:                                              \
    return state;                                       \
  }                                                     \
                                                        \
}                                                       \
OZ_C_proc_end



#define DECLAREBI_USEINLINEFUN2(Name,InlineName)        \
OZ_C_proc_begin(Name,3)                                 \
{                                                       \
  OZ_Term help;                                         \
                                                        \
  OZ_Term arg0 = OZ_getCArg(0);                         \
  OZ_Term arg1 = OZ_getCArg(1);                         \
  OZ_Return state=InlineName(arg0,arg1,help);           \
  switch (state) {                                      \
  case SUSPEND:                                         \
    OZ_suspendOn2(arg0,arg1);                           \
  case PROCEED:                                         \
    return(OZ_unify(help,OZ_getCArg(2)));               \
  default:                                              \
    return state;                                       \
  }                                                     \
                                                        \
}                                                       \
OZ_C_proc_end


// When InlineName suspends, then Name creates thread containing BlockName
#define DECLAREBINOBLOCK_USEINLINEFUN2(Name,BlockName,InlineName) \
OZ_C_proc_begin(Name,3)                                           \
{                                                                 \
  OZ_Term help;                                                   \
                                                                  \
  OZ_Term arg0 = OZ_getCArg(0);                                   \
  OZ_Term arg1 = OZ_getCArg(1);                                   \
  OZ_Return state=InlineName(arg0,arg1,help);                     \
  switch (state) {                                                \
  case SUSPEND:                                                   \
    {                                                             \
      RefsArray x=allocateRefsArray(3, NO);                       \
      x[0]=OZ_getCArg(0);                                         \
      x[1]=OZ_getCArg(1);                                         \
      x[2]=OZ_getCArg(2);                                         \
      OZ_Thread thr=OZ_makeSuspendedThread(BlockName,x,3);        \
      OZ_addThread(arg0,thr);                                     \
      OZ_addThread(arg1,thr);                                     \
      return PROCEED;                                             \
    }                                                             \
  case PROCEED:                                                   \
    return(OZ_unify(help,OZ_getCArg(2)));                         \
  default:                                                        \
    return state;                                                 \
  }                                                               \
                                                                  \
}                                                                 \
OZ_C_proc_end


// When InlineName suspends, then create thread on second argument
#define DECLARETHR2BI_USEINLINEFUN2(Name,InlineName) \
OZ_C_proc_begin(Name,3)                                           \
{                                                                 \
  OZ_Term help;                                                   \
                                                                  \
  OZ_Term arg0 = OZ_getCArg(0);                                   \
  OZ_Term arg1 = OZ_getCArg(1);                                   \
  OZ_Return state=InlineName(arg0,arg1,help);                     \
  switch (state) {                                                \
  case SUSPEND:                                                   \
    {                                                             \
      RefsArray x=allocateRefsArray(3, NO);                       \
      x[0]=OZ_getCArg(0);                                         \
      x[1]=OZ_getCArg(1);                                         \
      x[2]=OZ_getCArg(2);                                         \
      OZ_Thread thr=OZ_makeSuspendedThread(Name,x,3);             \
      OZ_addThread(arg1,thr);                                     \
      return PROCEED;                                             \
    }                                                             \
  case PROCEED:                                                   \
    return(OZ_unify(help,OZ_getCArg(2)));                         \
  default:                                                        \
    return state;                                                 \
  }                                                               \
                                                                  \
}                                                                 \
OZ_C_proc_end


#define DECLAREBI_USEINLINEFUN3(Name,InlineName)        \
OZ_C_proc_begin(Name,4)                                 \
{                                                       \
  OZ_Term help;                                         \
                                                        \
  OZ_Term arg0 = OZ_getCArg(0);                         \
  OZ_Term arg1 = OZ_getCArg(1);                         \
  OZ_Term arg2 = OZ_getCArg(2);                         \
  OZ_Return state=InlineName(arg0,arg1,arg2,help);      \
  switch (state) {                                      \
  case SUSPEND:                                         \
    OZ_suspendOn3(arg0,arg1,arg2);                      \
  case PROCEED:                                         \
    return(OZ_unify(help,OZ_getCArg(3)));               \
  default:                                              \
    return state;                                       \
  }                                                     \
                                                        \
}                                                       \
OZ_C_proc_end

#define DECLAREBOOLFUN1(BIfun,BIifun,BIirel)            \
OZ_Return BIifun(TaggedRef val, TaggedRef &out)         \
{                                                       \
  OZ_Return state = BIirel(val);                        \
  switch(state) {                                       \
  case PROCEED: out = NameTrue;  return PROCEED;        \
  case FAILED:  out = NameFalse; return PROCEED;        \
  default: return state;                                \
  }                                                     \
}                                                       \
DECLAREBI_USEINLINEFUN1(BIfun,BIifun)

#define DECLAREBOOLFUN2(BIfun,BIifun,BIirel)                            \
OZ_Return BIifun(TaggedRef val1, TaggedRef val2, TaggedRef &out)        \
{                                                                       \
  OZ_Return state = BIirel(val1,val2);                                  \
  switch(state) {                                                       \
  case PROCEED: out = NameTrue;  return PROCEED;                        \
  case FAILED:  out = NameFalse; return PROCEED;                        \
  default: return state;                                                \
  }                                                                     \
}                                                                       \
DECLAREBI_USEINLINEFUN2(BIfun,BIifun)

/*===================================================================
 * BuiltinTab
 *=================================================================== */

#ifdef BUILTINS2

BuiltinTab builtinTab(750);


BuiltinTabEntry *BIadd(char *name,int arity, OZ_CFun funn, IFOR infun)
{
  BuiltinTabEntry *builtin = new BuiltinTabEntry(name,arity,funn,infun);

  if (builtinTab.htAdd(name,builtin) == NO) {
    warning("BIadd: failed to add %s/%d\n",name,arity);
    delete builtin;
    return((BuiltinTabEntry *) NULL);
  }
  return(builtin);
}

// add specification to builtin table
void BIaddSpec(BIspec *spec)
{
  for (int i=0; spec[i].name; i++) {
    BIadd(spec[i].name,spec[i].arity,spec[i].fun,spec[i].ifun);
  }
}

BuiltinTabEntry *BIaddSpecial(char *name,int arity,BIType t)
{
  BuiltinTabEntry *builtin = new BuiltinTabEntry(name,arity,t);

  if (builtinTab.htAdd(name,builtin) == NO) {
    warning("BIadd: failed to add %s/%d\n",name,arity);
    delete builtin;
    return((BuiltinTabEntry *) NULL);
  }
  return(builtin);
}

/*===================================================================
 * `builtin`
 *=================================================================== */

OZ_Term OZ_findBuiltin(char *name, OZ_Term handler)
{
  if (!OZ_isProcedure(handler)) {
    if (!OZ_isAtom(handler) || !OZ_eq(handler,OZ_atom("noHandler"))) {
      warning("builtin: '%s' illegal handler", name,toC(handler));
    }
    handler = 0;
  }

  BuiltinTabEntry *found = (BuiltinTabEntry *) builtinTab.htFind(name);

  if (found == htEmpty) {
    warning("builtin: '%s' not in table", name);
    return 0;
  }

  if (!handler && !found->getFun()) {
    warning("builtin '%s' is special: needs suspension handler",name);
    return 0;
  }
  if (handler && found->getInlineFun()) {
    warning("builtin '%s' is compiled inline: suspension handler ignored",name);
    handler = 0;
  }

  Builtin *bi = new Builtin(found,handler);
  return makeTaggedConst(bi);
}

OZ_C_proc_begin(BIbuiltin,3)
{
  OZ_declareAtomArg(0,str);
  OZ_nonvarArg(1);
  OZ_Term hdl = OZ_deref(OZ_getCArg(1));
  OZ_Term ret = OZ_getCArg(2);

  OZ_Term bi = OZ_findBuiltin(str, hdl);

  if (!bi) return PROCEED;

  return OZ_unify(ret,bi);
}
OZ_C_proc_end


/*===================================================================
 * All builtins
 *=================================================================== */


OZ_Return isValueInline(TaggedRef val)
{
  NONVAR( val, _1, tag );
  return PROCEED;
}

DECLAREBI_USEINLINEREL1(BIisValue,isValueInline)
DECLAREBOOLFUN1(BIisValueB,isValueBInline,isValueInline)

OZ_Return isLiteralInline(TaggedRef t)
{
  NONSUVAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case OFSVariable:
        {
          GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
          if (ofsvar->getWidth()>0) return FAILED;
          return SUSPEND;
        }
      case FDVariable:
      case BoolVariable:
          return FAILED;
      default:
          return SUSPEND;
      }
  }
  return isLiteral(tag) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisLiteral,isLiteralInline)
DECLAREBOOLFUN1(BIisLiteralB,isLiteralBInline,isLiteralInline)


OZ_Return isAtomInline(TaggedRef t)
{
  NONSUVAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case OFSVariable:
        {
          GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
          TaggedRef lbl=ofsvar->getLabel();
          DEREF(lbl,_1,lblTag);
          if (isLiteral(lblTag) && !isAtom(lbl)) return FAILED;
          if (ofsvar->getWidth()>0) return FAILED;
          return SUSPEND;
        }
      case FDVariable:
      case BoolVariable:
          return FAILED;
      default:
          return SUSPEND;
      }
  }
  return isAtom(term) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisAtom,isAtomInline)
DECLAREBOOLFUN1(BIisAtomB,isAtomBInline,isAtomInline)


OZ_Return isVarInline(TaggedRef term)
{
  DEREF(term, _1, _2);
  return isAnyVar(term) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisVar,isVarInline)
DECLAREBOOLFUN1(BIisVarB,isVarBInline,isVarInline)

OZ_Return isNonvarInline(TaggedRef term)
{
  DEREF(term, _1, _2);
  return isAnyVar(term) ? FAILED : PROCEED;
}

DECLAREBI_USEINLINEREL1(BIisNonvar,isNonvarInline)
DECLAREBOOLFUN1(BIisNonvarB,isNonvarBInline,isNonvarInline)


OZ_Return isNameInline(TaggedRef t)
{
  NONSUVAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case OFSVariable:
        {
          GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
          TaggedRef lbl=ofsvar->getLabel();
          Assert(lbl!=makeTaggedNULL());
          DEREF(lbl,_1,lblTag);
          if (isAtom(lbl)) return FAILED;
          if (ofsvar->getWidth()>0) return FAILED;
          return SUSPEND;
        }
      case FDVariable:
      case BoolVariable:
          return FAILED;
      default:
          return SUSPEND;
      }
  }
  if (!isLiteral(tag)) return FAILED;
  return isAtom(term) ? FAILED: PROCEED;
}

DECLAREBI_USEINLINEREL1(BIisName,isNameInline)
DECLAREBOOLFUN1(BIisNameB,isNameBInline,isNameInline)


OZ_Return isTupleInline(TaggedRef t)
{
  NONSUVAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case OFSVariable:
        {
          GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
          if (ofsvar->getWidth()>0) return FAILED;
          return SUSPEND;
        }
      case FDVariable:
      case BoolVariable:
          return FAILED;
      default:
          return SUSPEND;
      }
  }
  return isTuple(term) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisTuple,isTupleInline)
DECLAREBOOLFUN1(BIisTupleB,isTupleBInline,isTupleInline)


OZ_Return isRecordInline(TaggedRef t)
{
  NONSUVAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case OFSVariable:
          return SUSPEND;
      case FDVariable:
      case BoolVariable:
          return FAILED;
      default:
          return SUSPEND;
      }
  }
  return isRecord(term) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisRecord,isRecordInline)
DECLAREBOOLFUN1(BIisRecordB,isRecordBInline,isRecordInline)


// Constrain term to a record.  Never suspend.
OZ_C_proc_begin(BIrecordC,1)
{
  TaggedRef t = OZ_getCArg(0);
  DEREF(t, tPtr, tag);

  switch (tag) {
  case LTUPLE:
  case LITERAL:
  case SRECORD:
    return PROCEED;
  case CVAR:
    switch (tagged2CVar(t)->getType()) {
    case OFSVariable:
      return PROCEED;
    case FDVariable:
    case BoolVariable:
      return FAILED;
    default:
      return SUSPEND;
    }
  case UVAR:
  case SVAR:
    {
      // Create newofsvar with unbound variable as label:
      GenOFSVariable *newofsvar=new GenOFSVariable();
      // Unify newofsvar and term:
      Bool ok=am.unify(makeTaggedRef(newTaggedCVar(newofsvar)),
                       makeTaggedRef(tPtr));
      Assert(ok);
      return PROCEED;
    }
  default:
    return FAILED;
  }
}
OZ_C_proc_end


// Constrain term to a record, with an initial size sufficient for at least
// tNumFeats features.  Never suspend.  If term is already a record, do nothing.
OZ_C_proc_begin(BIrecordCSize,2)
{
  TaggedRef tNumFeats = OZ_getCArg(0);
  TaggedRef t = OZ_getCArg(1);

  DEREF(tNumFeats, nPtr, ntag);
  DEREF(t, tPtr, tag);

  /* Calculate initial size of hash table */
  if (!isSmallInt(tNumFeats)) TypeErrorT(0,"Int");
  dt_index numFeats=smallIntValue(tNumFeats);
  dt_index size=ceilPwrTwo((numFeats<=FILLLIMIT) ? numFeats
                                                 : (int)ceil((double)numFeats/FILLFACTOR));
  switch (tag) {
  case LTUPLE:
  case LITERAL:
  case SRECORD:
    return PROCEED;
  case CVAR:
    if (tagged2CVar(t)->getType()!=OFSVariable) return FAILED;
    return PROCEED;
  case UVAR:
  case SVAR:
    {
      // Create newofsvar with unbound variable as label & given initial size:
      GenOFSVariable *newofsvar=new GenOFSVariable(size);
      // Unify newofsvar and term:
      Bool ok=am.unify(makeTaggedRef(newTaggedCVar(newofsvar)),makeTaggedRef(tPtr));
      Assert(ok);
      return PROCEED;
    }
  default:
    return FAILED;
  }
}
OZ_C_proc_end


// Suspend until can determine whether term is a record or not.
// This routine extends isRecordInline to accept undetermined records.
OZ_Return isRecordCInline(TaggedRef t)
{
  DEREF(t, tPtr, tag);
  switch (tag) {
  case LTUPLE:
  case LITERAL:
  case SRECORD:
    break;
  case CVAR:
    if (tagged2CVar(t)->getType()!=OFSVariable) return FAILED;
    break;
  case UVAR:
  case SVAR:
    return SUSPEND;
  default:
    return FAILED;
  }
  return PROCEED;
}

DECLAREBI_USEINLINEREL1(BIisRecordC,isRecordCInline)
DECLAREBOOLFUN1(BIisRecordCB,isRecordCBInline,isRecordCInline)


// Immediate test whether term is a (possibly undetermined) record or not.
// Non-monotonic test; never suspends.
OZ_C_proc_begin(BIisRecordCVar,1)
{
  TaggedRef t = OZ_getCArg(0);
  DEREF(t, tPtr, tag);
  switch (tag) {
  case LTUPLE:
  case LITERAL:
  case SRECORD:
    break;
  case CVAR:
    if (tagged2CVar(t)->getType()!=OFSVariable) return FAILED;
    break;
  case UVAR:
  case SVAR:
    return FAILED;
  default:
    return FAILED;
  }
  return PROCEED;
}
OZ_C_proc_end

//
OZ_C_proc_begin(BIisRecordCVarB,2)
{
  TaggedRef t = OZ_getCArg(0);
  DEREF(t, tPtr, tag);
  switch (tag) {
  case LTUPLE:
  case LITERAL:
  case SRECORD:
    break;
  case CVAR:
    if (tagged2CVar(t)->getType()!=OFSVariable)
      return (OZ_unify (OZ_getCArg (1), NameFalse));
    break;
  case UVAR:
  case SVAR:
    return (OZ_unify (OZ_getCArg (1), NameFalse));
  default:
    return (OZ_unify (OZ_getCArg (1), NameFalse));
  }
  return (OZ_unify (OZ_getCArg (1), NameTrue));
}
OZ_C_proc_end



OZ_Return isProcedureInline(TaggedRef t)
{
  NONVAR( t, term, tag );
  return isProcedure(term) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisProcedure,isProcedureInline)
DECLAREBOOLFUN1(BIisProcedureB,isProcedureBInline,isProcedureInline)

OZ_Return isChunkInline(TaggedRef t)
{
  NONSUVAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case OFSVariable:
          return FAILED;
      case FDVariable:
      case BoolVariable:
          return FAILED;
      default:
          return SUSPEND;
      }
  }
  return isChunk(term) ? PROCEED : FAILED;
}
DECLAREBI_USEINLINEREL1(BIisChunk,isChunkInline)
DECLAREBOOLFUN1(BIisChunkB,isChunkBInline,isChunkInline)

OZ_Return procedureArityInline(TaggedRef procedure, TaggedRef &out)
{
  NONVAR( procedure, pterm, ptag );

  if (isProcedure(pterm)) {
    int arity;
    ConstTerm *rec = tagged2Const(pterm);

    switch (rec->getType()) {
    case Co_Abstraction:
      arity = ((Abstraction *) rec)->getArity();
      break;
    case Co_Builtin:
      arity = ((Builtin *) rec)->getArity();
      break;
    default:
      goto typeError;
    }
    out = newSmallInt(arity);
    return PROCEED;
  }
  goto typeError;

typeError:
  out = nil();
  TypeErrorT(0,"Procedure");
}

DECLAREBI_USEINLINEFUN1(BIprocedureArity,procedureArityInline)




OZ_Return isCellInline(TaggedRef cell)
{
  NONVAR( cell, term, _ );
  return isCell(term) ? PROCEED : FAILED;
}
DECLAREBI_USEINLINEREL1(BIisCell,isCellInline)
DECLAREBOOLFUN1(BIisCellB,isCellBInline,isCellInline)

// ---------------------------------------------------------------------
// Spaces
// ---------------------------------------------------------------------


#define declareSpace()                                  \
  OZ_Term tagged_space = OZ_getCArg(0);                 \
  DEREF(tagged_space, space_ptr, space_tag);            \
  if (isAnyVar(space_tag))                              \
    OZ_suspendOn(makeTaggedRef(space_ptr));             \
  if (!isSpace(tagged_space))                           \
    TypeErrorT(0, "Space");                             \
  Space *space = (Space *) tagged2Const(tagged_space);

#define declareUnmergedSpace()                  \
  declareSpace()                                \
  if (space->isMerged())                        \
    TypeErrorT(0, "Space already merged");

#define declareStableSpace()                                            \
  declareUnmergedSpace();                                               \
  {                                                                     \
    TaggedRef result = space->getSolveActor()->getResult();             \
    DEREF(result, result_ptr, result_tag);                              \
    if (isAnyVar(result_tag)) OZ_suspendOn(makeTaggedRef(result_ptr));  \
  }


OZ_C_proc_begin(BInewSpace, 2) {
  OZ_Term proc = OZ_getCArg(0);

  DEREF(proc, proc_ptr, proc_tag);
  if (isAnyVar(proc_tag))
    OZ_suspendOn(makeTaggedRef(proc_ptr));

  if (!isProcedure(proc))
    TypeErrorT(0, "Procedure");

  Board* CBB = am.currentBoard;
  int    CPP = am.currentThread->getPriority();

  // creation of solve actor and solve board
  SolveActor *sa = new SolveActor(CBB, CPP);

  // thread creation for {proc root}
  sa->inject(CPP, proc);

  // create space
  return OZ_unify(OZ_getCArg(1), makeTaggedConst(new Space(CBB,sa->getSolveBoard())));
} OZ_C_proc_end


OZ_C_proc_begin(BIisSpace, 2) {
  OZ_Term tagged_space = OZ_getCArg(0);

  DEREF(tagged_space, space_ptr, space_tag);

  if (isAnyVar(space_tag))
    OZ_suspendOn(makeTaggedRef(space_ptr));

  return OZ_unify(OZ_getCArg(1), isSpace(tagged_space) ? NameTrue : NameFalse);
} OZ_C_proc_end


OZ_C_proc_begin(BIaskSpace, 2) {
  declareSpace();

  if (space->isFailed())
    return OZ_unify(OZ_args[1], AtomFailed);

  if (space->isMerged())
    return OZ_unify(OZ_args[1], AtomMerged);

  TaggedRef answer = space->getSolveActor()->getResult();

  DEREF(answer, answer_ptr, answer_tag);

  if (isAnyVar(answer_tag))
    OZ_suspendOn(makeTaggedRef(answer_ptr));

  return OZ_unify(OZ_args[1],
                  (isSTuple(answer) &&
                   literalEq(tagged2SRecord(answer)->getLabel(),
                             AtomSucceeded))
                    ? AtomSucceeded : answer);
} OZ_C_proc_end


OZ_C_proc_begin(BIaskVerboseSpace, 2) {
  declareSpace();

  if (space->isFailed())
    return OZ_unify(OZ_args[1], AtomFailed);

  if (space->isMerged())
    return OZ_unify(OZ_args[1], AtomMerged);

  if (space->getSolveActor()->isBlocked()) {
    SRecord *stuple = SRecord::newSRecord(AtomBlocked, 1);
    stuple->setArg(0, am.currentUVarPrototype);

    if (OZ_unify(OZ_args[1], makeTaggedSRecord(stuple)) == FAILED)
      return FAILED;

    OZ_args[1] = stuple->getArg(0);
  }

  TaggedRef answer = space->getSolveActor()->getResult();

  DEREF(answer, answer_ptr, answer_tag);

  if (isAnyVar(answer_tag))
    OZ_suspendOn(makeTaggedRef(answer_ptr));

  return OZ_unify(OZ_args[1], answer);
} OZ_C_proc_end


OZ_C_proc_begin(BImergeSpace, 2) {
  declareUnmergedSpace();

  if (am.isBelow(am.currentBoard,space->getSolveBoard()->getBoardFast()))
    TypeErrorM("current space is subordinated");

  if (space->isFailed())
    return FAILED;

  Board *CBB = am.currentBoard;

  // Check board

  TaggedRef result = space->getSolveActor()->getResult();

  if (OZ_isVariable(result) && OZ_unify(result, AtomMerged) == FAILED)
    return FAILED;

  TaggedRef root = space->getSolveActor()->merge(CBB);
  space->merge();

  return OZ_unify(root, OZ_getCArg(1));
} OZ_C_proc_end


OZ_C_proc_begin(BIcloneSpace, 2) {
  declareStableSpace();

  Board* CBB = am.currentBoard;

  if (space->isFailed())
    return OZ_unify(OZ_getCArg(1),
                    makeTaggedConst(new Space(CBB, (Board *) 0)));


  return OZ_unify(OZ_getCArg(1),
                  makeTaggedConst(new Space(CBB,
                                            space->getSolveActor()->clone(CBB))));

} OZ_C_proc_end


OZ_C_proc_begin(contChooseInternal, 2) {
  int left  = smallIntValue(OZ_getCArg(0)) - 1;
  int right = smallIntValue(OZ_getCArg(1)) - 1;

  int status =
    SolveActor::Cast(am.currentBoard->getActor())->choose(left,right);

  if (status==-1) {
    TypeErrorT(0, "Space: choice point stack is empty");
  } else if (status==0) {
    return FAILED;
  }

  return PROCEED;
} OZ_C_proc_end



OZ_C_proc_begin(BIchooseSpace, 2) {
  declareStableSpace();
  TaggedRef choice = OZ_getCArg(1);

  DEREF(choice, choice_ptr, choice_tag);

  if (isAnyVar(choice_tag))
    OZ_suspendOn(makeTaggedRef(choice_ptr));

  TaggedRef left, right;

  if (isSmallInt(choice_tag)) {
    left  = choice;
    right = choice;
  } else if (isSTuple(choice) &&
             literalEq(AtomPair,
                       tagged2SRecord(choice)->getLabel()) &&
             tagged2SRecord(choice)->getWidth() == 2) {
    left  = tagged2SRecord(choice)->getArg(0);
    DEREF(left, left_ptr, left_tag);

    if (isAnyVar(left_tag))
      OZ_suspendOn(makeTaggedRef(left_ptr));

    right = tagged2SRecord(choice)->getArg(1);

    DEREF(right, right_ptr, right_tag);

    if (isAnyVar(right_tag))
      OZ_suspendOn(makeTaggedRef(right_ptr));
  } else {
    TypeErrorT(1, "Integer or pair of integers");
  }

  if (am.isBelow(am.currentBoard,space->getSolveBoard()->getBoardFast()))
    TypeErrorT(0, "current space is subordinated");

  space->getSolveActor()->clearResult(space->getBoardFast());

  RefsArray args = allocateRefsArray(2, NO);
  args[0] = left;
  args[1] = right;

  Thread *it = new Thread(am.currentThread->getPriority(),
                          space->getSolveBoard(), OK);
  it->pushCFunCont(contChooseInternal, args, 2, NO);
  am.scheduleThread(it);

  return PROCEED;
} OZ_C_proc_end


OZ_C_proc_begin(BIinjectSpace, 2) {
  declareUnmergedSpace();

  // Check whether space is failed!
  if (space->isFailed())
    return PROCEED;

  if (am.isBelow(am.currentBoard,space->getSolveBoard()->getBoardFast()))
    TypeErrorT(0, "current space is subordinated");

  OZ_Term proc = OZ_getCArg(1);

  DEREF(proc, proc_ptr, proc_tag);

  if (isAnyVar(proc_tag))
    OZ_suspendOn(makeTaggedRef(proc_ptr));

  if (!isProcedure(proc))
    TypeErrorT(1, "Procedure");

  Board      *sb = space->getSolveBoard();
  SolveActor *sa = space->getSolveActor();

  // clear status
  sa->clearResult(space->getBoardFast());

  // inject
  sa->inject(sa->getPriority(), proc);

  return PROCEED;
} OZ_C_proc_end



// ---------------------------------------------------------------------
// Tuple
// ---------------------------------------------------------------------

OZ_Return tupleInline(TaggedRef label, TaggedRef argno, TaggedRef &out)
{
  DEREF(argno,_1,argnoTag);
  DEREF(label,_2,labelTag);

  if (isSmallInt(argnoTag)) {
    if (isLiteral(labelTag)) {
      int i = smallIntValue(argno);

      if (i < 0) {
        goto typeError1;
      }

      // literals
      if (i == 0) {
        out = label;
        return PROCEED;
      }

      {
        SRecord *s = SRecord::newSRecord(label,i);

        TaggedRef newVar = am.currentUVarPrototype;
        for (int j = 0; j < i; j++) {
          s->setArg(j,newVar);
        }

        out = s->normalize();
        return PROCEED;
      }
    }
    if (isAnyVar(labelTag)) {
      return SUSPEND;
    }
    goto typeError0;
  }
  if (isAnyVar(argnoTag)) {
    if (isAnyVar(labelTag) || isLiteral(labelTag)) {
      return SUSPEND;
    }
    goto typeError0;
  }
  goto typeError1;

 typeError0:
  TypeErrorT(0,"Literal");
 typeError1:
  TypeErrorT(1,"(non-negative small) Int");
}

DECLAREBI_USEINLINEFUN2(BItuple,tupleInline)


// ---------------------------------------------------------------------
// Tuple & Record
// ---------------------------------------------------------------------


OZ_Return labelInline(TaggedRef term, TaggedRef &out)
{
  DEREF(term,_,tag);

  switch (tag) {
  case LTUPLE:
    out = AtomCons;
    return PROCEED;
  case LITERAL:
    out = term;
    return PROCEED;
  case SRECORD:
  record:
    out = tagged2SRecord(term)->getLabel();
    return PROCEED;
  case UVAR:
  case SVAR:
    return SUSPEND;
  case CVAR:
    if (tagged2CVar(term)->getType() == OFSVariable) {
        GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
        TaggedRef lbl = ofsvar->getLabel();
        TaggedRef tmplbl=lbl;
        DEREF(tmplbl,_,tmptag);
        if (isAnyVar(tmptag))
            return SUSPEND;
        else {
            out = lbl;
            return PROCEED;
        }
    }
    break;
  default:
    break;
  }
  TypeErrorT(0,"Record");
}

DECLAREBI_USEINLINEFUN1(BIlabel,labelInline)



/*
 * {RecordC.widthC X W} -- builtin that constrains number of features
 * of X to be equal to finite domain variable W.  Will constrain X to
 * a record and W to a finite domain.  This built-in installs a
 * WidthPropagator.
 */
OZ_C_proc_begin(BIwidthC, 2)
{
    EXPECTED_TYPE("record,finite domain");

    TaggedRef rawrec=OZ_getCArg(0);
    TaggedRef rawwid=OZ_getCArg(1);
    TaggedRef rec=OZ_getCArg(0);
    TaggedRef wid=OZ_getCArg(1);
    DEREF(rec, recPtr, recTag);
    DEREF(wid, widPtr, widTag);

    // Ensure that first argument rec is an OFS, SRECORD, or LITERAL:
    switch (recTag) {
    case UVAR:
    case SVAR:
      {
        // Create new ofsvar:
        GenOFSVariable *ofsvar=new GenOFSVariable();
        // Unify ofsvar and rec:
        Bool ok=am.unify(makeTaggedRef(newTaggedCVar(ofsvar)),rawrec);
        Assert(ok);
        break;
      }
    case CVAR:
      if (tagged2CVar(rec)->getType()!=OFSVariable) return FAILED;
      break;
    case SRECORD:
    case LITERAL:
    case LTUPLE:
      break;
    default:
      return FAILED;
    }

    // Ensure that second argument wid is a FD or integer:
    switch (widTag) {
    case UVAR:
    case SVAR:
    {
        // Create new fdvar:
        GenFDVariable *fdvar=new GenFDVariable(); // Variable with maximal domain
        // Unify fdvar and wid:
        Bool ok=am.unify(makeTaggedRef(newTaggedCVar(fdvar)),rawwid);
        Assert(ok);
        break;
    }
    case CVAR:
        if (tagged2CVar(wid)->getType()!=FDVariable) return FAILED;
        break;
    case BIGINT:
    case SMALLINT:
        break;
    default:
        return FAILED;
    }

    // This completes the propagation abilities of widthC.  However, entailment is
    // still hard, so this rule will not be added now--we'll wait until people need it.
    //   // If propagator exists already on the variable, just unify the widths
    //   // Implements rule: width(x,w1)/\width(x,w2) -> width(x,w1)/\(w1=w2)
    //   if (recTag==CVAR) {
    //       TaggedRef otherwid=am.getWidthSuspension((void*)BIpropWidth,rec);
    //       if (otherwid!=makeTaggedNULL()) {
    //           return (am.unify(otherwid,rawwid) ? PROCEED : FAILED);
    //       }
    //   }

    OZ_PropagatorExpect pe;
    EXPECT(pe, 0, expectRecordVar);
    EXPECT(pe, 1, expectIntVarAny);

    return pe.spawn(new WidthPropagator(rawrec, rawwid)); // OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end



// Used by BIwidthC built-in
// {PropWidth X W} where X is OFS and W is FD width.
// Assume: rec is OFS or SRECORD or LITERAL.
// Assume: wid is FD or SMALLINT or BIGINT.
// This is the simplest most straightforward possible
// implementation and it can be optimized in many ways.
OZ_Return WidthPropagator::run(void)
{
    int res;
    int recwidth;
    OZ_Return result = SLEEP;

    TaggedRef rec=rawrec;
    TaggedRef wid=rawwid;
    DEREF(rec, recptr, recTag);
    DEREF(wid, widptr, widTag);

    switch (recTag) {
    case SRECORD:
    record:
    case LITERAL:
    case LTUPLE:
    {
        // Impose width constraint
        recwidth=(recTag==SRECORD) ? tagged2SRecord(rec)->getWidth() :
                 ((recTag==LTUPLE) ? 2 : 0);
        if (isGenFDVar(wid)) {
            // GenFDVariable *fdwid=tagged2GenFDVar(wid);
            // res=fdwid->setSingleton(recwidth);
            res=am.unify(makeTaggedSmallInt(recwidth),rawwid);
            if (res==FAILED) { result = FAILED; break; }
        } else if (isSmallInt(widTag)) {
            int intwid=smallIntValue(wid);
            if (recwidth!=intwid) { result = FAILED; break; }
        } else if (isBigInt(widTag)) {
            // BIGINT case: fail
            result = FAILED; break;
        } else {
            error("unexpected wrong type for width in determined widthC");
        }
        result = PROCEED;
        break;
    }
    case CVAR:
    {
        if (tagged2CVar(rec)->getType() != OFSVariable)
            error("widthC on wrong CVAR type");
        // 1. Impose width constraint
        GenOFSVariable *recvar=tagged2GenOFSVar(rec);
        recwidth=recvar->getWidth(); // current actual width of record
        if (isGenFDVar(wid)) {
            // Build fd with domain recwidth..fd_sup:
            OZ_FiniteDomain slice=new OZ_FiniteDomain();
            slice.init(recwidth,fd_sup);
            OZ_FiniteDomain &dom = tagged2GenFDVar(wid)->getDom();
            if (dom.getSize() > (dom & slice).getSize()) {
                GenFDVariable *fdcon=new GenFDVariable(slice);
                res=am.unify(makeTaggedRef(newTaggedCVar(fdcon)),rawwid);
                // No loc/glob handling: res=(fdwid>=recwidth);
                if (res==FAILED) { result = FAILED; break; }
            }
        } else if (isSmallInt(widTag)) {
            int intwid=smallIntValue(wid);
            if (recwidth>intwid) { result = FAILED; break; }
        } else if (isBigInt(widTag)) {
            // BIGINT case: fail
            result = FAILED; break;
        } else {
            error("unexpected wrong type for width in undetermined widthC");
        }
        // 2. Convert representation if necessary
        // 2a. Find size and value (latter is valid only if goodsize==TRUE):
        int goodsize,value;
        DEREF(wid,_3,newwidTag);
        if (isGenFDVar(wid)) {
            GenFDVariable *newfdwid=tagged2GenFDVar(wid);
            goodsize=(newfdwid->getDom().getSize())==1;
            value=newfdwid->getDom().minElem();
        } else if (isSmallInt(newwidTag)) {
            goodsize=TRUE;
            value=smallIntValue(wid);
        } else {
            goodsize=FALSE;
            value = 0; // make gcc quiet
        }
        // 2b. If size==1 and all features and label present,
        //     then convert to SRECORD or LITERAL:
        if (goodsize && value==recwidth) {
            TaggedRef lbl=tagged2GenOFSVar(rec)->getLabel();
            DEREF(lbl,_4,lblTag);
            if (isLiteral(lblTag)) {
                result = PROCEED;
                if (recwidth==0) {
                    // Convert to LITERAL:
                    res=am.unify(rawrec,lbl);
                    if (res==FAILED) error("unexpected failure of Literal conversion");
                } else {
                    // Convert to SRECORD or LTUPLE:
                    // (Two efficiency problems: 1. Creates record & then unifies,
                    // instead of creating & only binding.  2. Rec->normalize()
                    // wastes the space of the original record.)
                    TaggedRef alist=tagged2GenOFSVar(rec)->getTable()->getArityList();
                    Arity *arity=aritytable.find(alist);
                    SRecord *newrec = SRecord::newSRecord(lbl,arity);
                    newrec->initArgs(am.currentUVarPrototype);
                    res=am.unify(rawrec,newrec->normalize());
                    Assert(res!=FAILED);
                }
            }
        }
        break;
    }
    default:
      // mm2: type error ?
      warning("unexpected bad first argument to widthC");
      result=FAILED;
    }

    return (result);
}



OZ_C_proc_begin(BIlabelC,2)
{
  OZ_Term term = OZ_getCArg(0);
  OZ_Term lbl  = OZ_getCArg(1);

  DEREF(term,termPtr,tag);
  TaggedRef lbldrf=lbl;
  DEREF(lbldrf,_,lbltag);

  // Constrain term to a record:
  // Get the term's label, if it exists
  TaggedRef thelabel=makeTaggedNULL();
  switch (tag) {
  case LTUPLE:
    thelabel=AtomCons;
    break;
  case LITERAL:
    thelabel=term;
    break;
  case SRECORD:
  record:
    thelabel=tagged2SRecord(term)->getLabel();
    break;
  case UVAR:
  case SVAR:
    {
      // Create newofsvar with unbound variable as label:
      GenOFSVariable *newofsvar=new GenOFSVariable();
      // Unify newofsvar and term:
      Bool ok=am.unify(makeTaggedRef(newTaggedCVar(newofsvar)),makeTaggedRef(termPtr));
      Assert(ok);
      term=makeTaggedRef(termPtr);
      DEREF(term, termPtr2, tag2);
      termPtr=termPtr2;
      tag=tag2;
      thelabel=newofsvar->getLabel();
      break;
    }
  case CVAR:
    if (tagged2CVar(term)->getType()!=OFSVariable)
        TypeErrorT(0,"Record");
    thelabel=tagged2GenOFSVar(term)->getLabel();
    break;
  default:
    TypeErrorT(0,"Record");
  }

  // At this point, thelabel is term's label
  // Constrain the term's label to be lbl:
  Assert(thelabel!=makeTaggedNULL());
  TaggedRef thelabeldrf=thelabel;
  DEREF(thelabeldrf,_1,_2);
  // One of the two must be a literal:
  if (!isLiteral(thelabeldrf) && !isLiteral(lbldrf)) {
      // If neither are literals, then both must be variables:
      if (isAnyVar(thelabeldrf) && isAnyVar(lbldrf)) {
          // Create a thread with this task:
          RefsArray x=allocateRefsArray(2, NO);
          x[0]=OZ_getCArg(0);
          x[1]=OZ_getCArg(1);
          OZ_Thread thr=OZ_makeSuspendedThread(BIlabelC,x,2);
          OZ_addThread(lbl,thr);
          OZ_addThread(thelabel,thr);
          return PROCEED;
          // OZ_suspendOn2(thelabel,lbl);
      } else {
          // Label argument is not a literal:
          TypeErrorT(1,"Literal");
          // TypeErrorMessage2("labelC","Labels must be literals",thelabel,lbl);
          return FAILED;
      }
      // return OZ_suspendOnVar2(thelabel,lbl);
  }
  if (!isAnyVar(lbltag) && !isLiteral(lbldrf)) return FAILED;
  return (am.unify(thelabel,lbl)? PROCEED : FAILED);
}
OZ_C_proc_end

// DECLAREBI_USEINLINEREL2(BIlabelC,labelCInline);


// {RecordC.monitorArity X K L} -- builtin that tracks features added to OFS X
// in the list L.  Goes away if K is determined (if K is determined on first call,
// L is list of current features).  monitorArity imposes that X is a record (like
// RecordC) and hence fails if X is not a record.
OZ_C_proc_begin(BImonitorArity, 3)
{
    EXPECTED_TYPE("any(record),any,any(list)");

    OZ_Term rec = OZ_getCArg(0);
    OZ_Term kill = OZ_getCArg(1);
    OZ_Term arity = OZ_getCArg(2);

    OZ_Term tmpkill=OZ_getCArg(1);
    DEREF(tmpkill,_1,killTag);
    Bool isKilled = !isAnyVar(killTag);

    OZ_Term tmprec=OZ_getCArg(0);
    DEREF(tmprec,_2,recTag);
    switch (recTag) {
    case LTUPLE:
      return am.unify(arity,makeTupleArityList(2)) ? PROCEED : FAILED;
    case LITERAL:
      // *** arity is nil
      return (am.unify(arity,AtomNil)? PROCEED : FAILED);
    case SRECORD:
    record:
      // *** arity is known set of features of the SRecord
      return am.unify(arity,tagged2SRecord(tmprec)->getArityList())
        ? PROCEED : FAILED;
    case UVAR:
    case SVAR:
        {
          // Create newofsvar with no features and unbound variable as label:
          GenOFSVariable *newofsvar=new GenOFSVariable();
          // Unify newofsvar and rec:
          Bool ok=am.unify(makeTaggedRef(newTaggedCVar(newofsvar)),rec);
          Assert(ok);
          break;
        }
    case CVAR:
        if (tagged2CVar(tmprec)->getType()!=OFSVariable) {
            TypeErrorT(0,"Record");
        }
        // *** arity is calculated from the OFS; see below
        break;
    default:
        TypeErrorT(0,"Record");
    }
    tmprec=OZ_getCArg(0);
    DEREF(tmprec,_3,_4);

    // At this point, rec is OFS and tmprec is dereferenced and unbound

    if (isKilled) {
        TaggedRef featlist;
        featlist=tagged2GenOFSVar(tmprec)->getArityList();

        return (am.unify(arity,featlist)? PROCEED : FAILED);
    } else {
        TaggedRef featlist;
        TaggedRef feattail;
        Board *home=am.currentBoard;
        featlist=tagged2GenOFSVar(tmprec)->getOpenArityList(&feattail,home);

        if (!am.unify(featlist,arity)) return FAILED;

        OZ_PropagatorExpect pe;
        EXPECT(pe, 0, expectRecordVar);
        EXPECT(pe, 1, expectVar);

        TaggedRef uvar=makeTaggedRef(newTaggedUVar(home));
        return pe.spawn(
            new MonitorArityPropagator(rec,kill,feattail,uvar,uvar),
            OFS_flag);
    }

    return PROCEED;
}
OZ_C_proc_end // BImonitorArity


// The propagator for the built-in RecordC.monitorArity
// {PropFeat X K L FH FT} -- propagate features from X to the list L, and go
// away when K is determined.  L is closed when K is determined.  X is used to
// check in addFeatOFSSuspList that the suspension is waiting for the right
// variable.  FH and FT are a difference list that holds the features that
// have been added.
OZ_Return MonitorArityPropagator::run(void)
{
    // Check if killed:
    TaggedRef kill=K;
    TaggedRef tmpkill=kill;
    DEREF(tmpkill,_2,killTag);
    Bool isKilled = !isAnyVar(killTag);

    TaggedRef tmptail=FT;
    DEREF(tmptail,_3,_4);

    // Get featlist (a difference list stored in the arguments):
    TaggedRef fhead = FH;
    TaggedRef ftail = FT;

    if (tmptail!=AtomNil) {
        // The record is not determined, so reinitialize the featlist:
        // The home board of uvar must be taken from outside propFeat!
        // Get the home board for any new variables:
        Board* home=tagged2VarHome(tmptail);
        TaggedRef uvar=makeTaggedRef(newTaggedUVar(home));
        FH=uvar;
        FT=uvar;
    } else {
        // Precaution for the GC?
        FH=makeTaggedNULL();
        FT=makeTaggedNULL();
    }

    // Add the features to L (the tail of the output list)
    TaggedRef arity=L;
    if (!am.unify(fhead,arity)) return FAILED; // No further updating of the suspension
    L=ftail; // 'ftail' is the new L in the suspension

    if (tmptail!=AtomNil) {
        // The record is not determined, so the suspension is revived:
        if (!isKilled) return (SLEEP);
        else return (am.unify(ftail,AtomNil)? PROCEED : FAILED);
    }
    return PROCEED;
}



/*
 * NOTE: similar functions are dot, genericSet, uparrow
 */
OZ_Return genericDot(TaggedRef term, TaggedRef fea, TaggedRef *out, Bool dot)
{
  DEREF(fea, _1,feaTag);
LBLagain:
  DEREF(term, _2, termTag);

  if (isAnyVar(feaTag)) {
    switch (termTag) {
    case LTUPLE:
    case SRECORD:
    case SVAR:
    case UVAR:
      return SUSPEND;
    case CVAR:
      if (tagged2CVar(term)->getType() == OFSVariable) return SUSPEND;
      if (tagged2CVar(term)->getType() == AVAR) return SUSPEND;
      goto typeError0;
    case LITERAL:
      goto typeError0;
    default:
      if (isChunk(term)) return SUSPEND;
      goto typeError0;
    }
  }

  switch (termTag) {
  case LTUPLE:
    {
      if (!isSmallInt(fea)) {
        if (!dot && isBigInt(fea)) return FAILED;
        goto typeError1t;
      }
      int i2 = smallIntValue(fea);

      switch (i2) {
      case 1:
        if (out) *out = tagged2LTuple(term)->getHead();
        return PROCEED;
      case 2:
        if (out) *out = tagged2LTuple(term)->getTail();
        return PROCEED;
      }
      if (dot) goto typeError1t;
      return FAILED;
    }

  case SRECORD:
    {
      if ( ! isFeature(feaTag) ) goto typeError1r;

      TaggedRef t = tagged2SRecord(term)->getFeature(fea);
      if (t == makeTaggedNULL()) {
        if (dot) { goto typeError1r; }
        return FAILED;
      }
      if (out) *out = t;
      return PROCEED;
    }

  case UVAR:
  case SVAR:
    if (!isFeature(feaTag)) {
      TypeErrorT(1,"Feature");
    }
    return SUSPEND;

  case CVAR:
    {
      if (tagged2CVar(term)->getType() != OFSVariable) goto typeError0;
      if (!isFeature(feaTag)) goto typeError1r;
      GenOFSVariable *ofs=(GenOFSVariable *)tagged2CVar(term);
      TaggedRef t = ofs->getFeatureValue(fea);
      if (t == makeTaggedNULL()) return SUSPEND;
      if (out) *out = t;
      return PROCEED;
    }

  case LITERAL:
    if (dot) goto typeError0;
    return FAILED;

  default:
    if (isChunk(term)) {
      if (! isFeature(feaTag)) { goto typeError1r; }
      TaggedRef t;
      switch (tagged2Const(term)->getType()) {
      case Co_Chunk:
        t = tagged2SChunk(term)->getFeature(fea);
        break;
      case Co_Thread:
        t = tagged2Thread(term)->getFeature(fea);
        break;
      case Co_Object:
        t = tagged2Object(term)->getFeature(fea);
        break;
      case Co_Array:
      case Co_Dictionary:
      default:
        // no public known features
        t = 0;
        break;
      }
      if (t == makeTaggedNULL()) {
        if (dot) { goto typeError1r; }
        return FAILED;
      }
      if (out) *out = t;
      return PROCEED;
    }
    /* special case for cells (mm 17.3.95) */
    if (ozconf.cellHack && isCell(term)) {
      Cell *cell= tagged2Cell(term);
      term = cell->getValue();
      goto LBLagain;
    }

    goto typeError0;
  }
typeError0:
  if (dot)   TypeErrorT(0,"Record or Chunk and no Literal");
  TypeErrorT(0,"Record or Chunk");
typeError1t:
  if (dot) TypeErrorT(1,"(valid) Int)");
  TypeErrorT(1,"Int");
typeError1r:
  if (dot) TypeErrorT(1,"(valid) Feature");
  TypeErrorT(1,"Feature");
}


OZ_Return dotInline(TaggedRef term, TaggedRef fea, TaggedRef &out)
{
  return genericDot(term,fea,&out,TRUE);
}
DECLAREBI_USEINLINEFUN2(BIdot,dotInline)



OZ_Return subtreeInline(TaggedRef term, TaggedRef fea, TaggedRef &out)
{
  return genericDot(term,fea,&out,FALSE);
}
DECLAREBI_USEINLINEFUN2(BIsubtree,subtreeInline)



// {SetC X F Y}: destructively update feature F of X with new value Y.
// X must be undetermined record, F must be literal, feature is added if it doesn't exist.
// Non-monotonic built-in.
OZ_C_proc_begin(BIsetC, 3)
{
    OZ_Term term = OZ_getCArg(0);
    OZ_Term fea = OZ_getCArg(1);
    OZ_Term val = OZ_getCArg(2);

    DEREF(term, termPtr, termTag);
    DEREF(fea,  feaPtr,  feaTag);

    // Error unless X is OFS:
    if (termTag!=CVAR ||
        tagged2CVar(term)->getType()!=OFSVariable) {
        TypeErrorT(0,"undetermined record");
    }

    // Error unless F is a literal:
    if (!isFeature(feaTag))
        TypeErrorT(1,"Feature");

    // At this point, X is OFS and F is feature.
    GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
    // Destructively update the feature F:
    Bool updated=ofsvar->setFeatureValue(fea,val);
    if (!updated) {
        // Feature doesn't exist so add it:
        // Add feature by (1) creating new ofsvar with one feature,
        // (2) unifying the new ofsvar with the old.
        if (am.currentBoard == ofsvar->getBoardFast()) {
            // Optimization:
            // If current board is same as ofsvar board then can add feature directly
            Bool ok=ofsvar->addFeatureValue(fea,val);
            Assert(ok);
            ofsvar->propagateFeature();

            return (PROCEED);
        } else {
            // Create newofsvar:
            GenOFSVariable *newofsvar=new GenOFSVariable();
            // Add feature to newofsvar:
            Bool ok1=newofsvar->addFeatureValue(fea,val);
            Assert(ok1);
            // Unify newofsvar and term (which is also an ofsvar):
            Bool ok2=am.unify(makeTaggedRef(newTaggedCVar(newofsvar)),makeTaggedRef(termPtr));
            Assert(ok2);
            return PROCEED;
        }
    }
    return PROCEED;
}
OZ_C_proc_end


// {RemoveC X F}: destructively remove feature F of X
// X must be undetermined record and F must be feature.
// Non-monotonic built-in.
OZ_C_proc_begin(BIremoveC, 2)
{
    OZ_Term term = OZ_getCArg(0);
    OZ_Term fea = OZ_getCArg(1);

    DEREF(term, termPtr, termTag);
    DEREF(fea,  feaPtr,  feaTag);

    // Error unless X is OFS:
    if (termTag!=CVAR || tagged2CVar(term)->getType()!=OFSVariable)
        TypeErrorT(0,"undetermined record");

    // Error unless F is a Feature:
    if (!isFeature(feaTag))
        TypeErrorT(1,"Feature");

    // At this point, X is OFS and F is feature.
    GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
    // Destructively remove the feature F:
    ofsvar->removeFeature(fea);
    return PROCEED;
}
OZ_C_proc_end


// {TestCB X F ?B}: B is boolean with truth value "X has feature F".
// X must be undetermined record and F must be feature.
// Non-monotonic built-in.
OZ_C_proc_begin(BItestCB, 3)
{
    OZ_Term term = OZ_getCArg(0);
    OZ_Term fea = OZ_getCArg(1);
    OZ_Term out = OZ_getCArg(2);

    DEREF(term, termPtr, termTag);
    DEREF(fea,  feaPtr,  feaTag);

    // Error unless X is OFS:
    if (termTag!=CVAR || tagged2CVar(term)->getType()!=OFSVariable)
        TypeErrorT(0,"undetermined record");

    // Error unless F is a feature:
    if (!isFeature(feaTag))
        TypeErrorT(1,"Feature");

    // At this point, X is OFS and F is feature.
    GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
    // Test presence of feature F:
    if (ofsvar->getFeatureValue(fea)!=makeTaggedNULL())
        return OZ_unify(out,NameTrue);
    else
        return OZ_unify(out,NameFalse);
}
OZ_C_proc_end


// Create new thread on suspension:
OZ_Return uparrowInline(TaggedRef, TaggedRef, TaggedRef&);
DECLAREBI_USEINLINEFUN2(BIuparrow,uparrowInline)

// Block current thread on suspension:
OZ_Return uparrowInlineBlocking(TaggedRef, TaggedRef, TaggedRef&);
DECLAREBI_USEINLINEFUN2(BIuparrowBlocking,uparrowInlineBlocking)


/*
 * NOTE: similar functions are dot, genericSet, uparrow
 */
// X^Y=Z: add feature Y to open feature structure X (Tell operation).
OZ_Return genericUparrowInline(TaggedRef term, TaggedRef fea, TaggedRef &out, Bool blocking)
{
    TaggedRef termOrig=term;
    TaggedRef feaOrig=fea;
    DEREF(term, termPtr, termTag);
    DEREF(fea,  feaPtr,  feaTag);
    int suspFlag=FALSE;

    // Constrain term to a record:
    switch (termTag) {
    case LTUPLE:
    case LITERAL:
    case SRECORD:
        break;
    case UVAR:
    case SVAR:
        if (!isFeature(feaTag)) {
            // Create newofsvar with unbound variable as label:
            GenOFSVariable *newofsvar=new GenOFSVariable();
            // Unify newofsvar and term:
            Bool ok=am.unify(makeTaggedRef(newTaggedCVar(newofsvar)),makeTaggedRef(termPtr));
            Assert(ok);
            term=makeTaggedRef(termPtr);
            DEREF(term, termPtr2, tag2);
            termPtr=termPtr2;
            termTag=tag2;
        }
        break;
    case CVAR:
        if (tagged2CVar(term)->getType()!=OFSVariable) goto typeError1;
        break;
    default:
      if (isChunk(term)) {
        break;
      }
        goto typeError1;
    }

    // Wait until Y is a literal:
    if (isNotCVar(feaTag)) suspFlag=TRUE;
    if (isCVar(feaTag)) {
        switch (tagged2CVar(fea)->getType()) {
        case OFSVariable:
          {
            GenOFSVariable *ofsvar=tagged2GenOFSVar(fea);
            if (ofsvar->getWidth()>0) return FAILED;
            suspFlag=TRUE;
          }
        default:
            return FAILED;
      }
    }
    if (suspFlag) {
        if (blocking) {
            return SUSPEND;
        } else {
            // Create thread containing relational blocking version of uparrow:
            RefsArray x=allocateRefsArray(3, NO);
            out=makeTaggedRef(newTaggedUVar(am.currentBoard));
            x[0]=termOrig;
            x[1]=feaOrig;
            x[2]=out;
            OZ_Thread thr=OZ_makeSuspendedThread(BIuparrowBlocking,x,3);
            OZ_addThread(feaOrig,thr);
            return PROCEED;
        }
    }
    if (!isFeature(feaTag)) goto typeError2;

    // Add feature and return:
    Assert(term!=makeTaggedNULL());
    switch (termTag) {
    case CVAR:
      {
        Assert(term!=makeTaggedNULL());
        Assert(tagged2CVar(term)->getType()==FDVariable ||
               tagged2CVar(term)->getType()==OFSVariable);
        if (tagged2CVar(term)->getType() != OFSVariable) return FAILED;
        GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
        TaggedRef t=ofsvar->getFeatureValue(fea);
        if (t!=makeTaggedNULL()) {
            // Feature exists
            out=t;
            return PROCEED;
        } else {
            // Feature does not yet exist
            // Add feature by (1) creating new ofsvar with one feature,
            // (2) unifying the new ofsvar with the old.
            if (am.currentBoard == ofsvar->getBoardFast()) {
                // Optimization:
                // If current board is same as ofsvar board then can add feature directly
                TaggedRef uvar=makeTaggedRef(newTaggedUVar(am.currentBoard));
                Bool ok=ofsvar->addFeatureValue(fea,uvar);
                Assert(ok);
                ofsvar->propagateFeature();
                out=uvar;

                return (PROCEED);
            } else {
                // Create newofsvar:
                GenOFSVariable *newofsvar=new GenOFSVariable();
                // Add feature to newofsvar:
                TaggedRef uvar=makeTaggedRef(newTaggedUVar(am.currentBoard));
                Bool ok1=newofsvar->addFeatureValue(fea,uvar);
                Assert(ok1);
                out=uvar;
                // Unify newofsvar and term (which is also an ofsvar):
                Bool ok2=am.unify(makeTaggedRef(newTaggedCVar(newofsvar)),makeTaggedRef(termPtr));
                Assert(ok2);
                // newofsvar->propagateFeature();
                return PROCEED;
            }
            break;
        }
        Assert(FALSE);
        break;
      }

    case UVAR:
    case SVAR:
      {
        // Create newofsvar:
        GenOFSVariable *newofsvar=new GenOFSVariable();
        // Add feature to newofsvar:
        TaggedRef uvar=makeTaggedRef(newTaggedUVar(am.currentBoard));
        Bool ok1=newofsvar->addFeatureValue(fea,uvar);
        Assert(ok1);
        out=uvar;
        // Unify newofsvar (CVAR) and term (SVAR or UVAR):
        Bool ok2=am.unify(makeTaggedRef(newTaggedCVar(newofsvar)),makeTaggedRef(termPtr));
        Assert(ok2);
        return PROCEED;
        break;
      }

    case SRECORD:
    case LTUPLE:
    record:
      {
        // Get the SRecord corresponding to term:
        SRecord* termSRec=makeRecord(term);

        TaggedRef t=termSRec->getFeature(fea);
        if (t!=makeTaggedNULL()) {
            out=t;
            return PROCEED;
        }
        return FAILED;
        break;
      }

    case LITERAL:
        return FAILED;

    default:
      return dotInline(term,fea,out);
    }
typeError1:
    TypeErrorT(0,"Record");
typeError2:
    TypeErrorT(1,"Feature");
}


OZ_Return uparrowInline(TaggedRef term, TaggedRef fea, TaggedRef &out)
{
    return genericUparrowInline(term, fea, out, FALSE);
}

OZ_Return uparrowInlineBlocking(TaggedRef term, TaggedRef fea, TaggedRef &out)
{
    return genericUparrowInline(term, fea, out, TRUE);
}


OZ_Return hasSubtreeAtInline(TaggedRef term, TaggedRef fea)
{
  return genericDot(term,fea,0,FALSE);
}
DECLAREBI_USEINLINEREL2(BIhasSubtreeAt,hasSubtreeAtInline)
DECLAREBOOLFUN2(BIhasSubtreeAtB,hasSubtreeAtBInline,hasSubtreeAtInline)


OZ_Return widthInline(TaggedRef term, TaggedRef &out)
{
  DEREF(term,_,tag);

  switch (tag) {
  case LTUPLE:
    out = OZ_int(2);
    return PROCEED;
  case SRECORD:
  record:
    out = OZ_int(tagged2SRecord(term)->getWidth());
    return PROCEED;
  case LITERAL:
    out = OZ_int(0);
    return PROCEED;
  case UVAR:
  case SVAR:
    return SUSPEND;
  case CVAR:
    if (tagged2CVar(term)->getType() == OFSVariable) return SUSPEND;
    break;
  default:
    break;
  }

  TypeErrorT(0,"Record");
}

DECLAREBI_USEINLINEFUN1(BIwidth,widthInline)



// ---------------------------------------------------------------------
// Bool things
// ---------------------------------------------------------------------

OZ_C_proc_begin(BIgetTrue,1)
{
  return OZ_unify(NameTrue,OZ_getCArg(0));
}
OZ_C_proc_end


OZ_C_proc_begin(BIgetFalse,1)
{
  return OZ_unify(NameFalse,OZ_getCArg(0));
}
OZ_C_proc_end


OZ_Return isBoolInline(TaggedRef t)
{
  NONSUVAR( t, term, tag);
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case OFSVariable:
        {
          GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
          if (ofsvar->getWidth()>0) return FAILED;
          TaggedRef lbl=ofsvar->getLabel();
          DEREF(lbl,_1,lblTag);
          if (isLiteral(lblTag)) {
              if (isAtom(lbl)) {
                  if (literalEq(term,NameTrue) ||
                      literalEq(term,NameFalse))
                      return SUSPEND;
                  else
                      return FAILED;
              } else { // isName
                  return FAILED;
              }
          }
          return SUSPEND;
        }
      case FDVariable:
      case BoolVariable:
          return FAILED;
      default:
          return SUSPEND;
      }
  }
  if (literalEq(term,NameTrue) || literalEq(term,NameFalse))
    return PROCEED;
  else
    return FAILED;
}

DECLAREBI_USEINLINEREL1(BIisBool,isBoolInline)
DECLAREBOOLFUN1(BIisBoolB,isBoolBInline,isBoolInline)


OZ_Return notInline(TaggedRef A, TaggedRef &out)
{
  NONVAR(A,term,tag);

  if (literalEq(term,NameTrue)) {
    out = NameFalse;
    return PROCEED;
  } else {
    if (literalEq(term,NameFalse)) {
      out = NameTrue;
      return PROCEED;
    }
  }

  TypeErrorT(0,"Bool");
}

DECLAREBI_USEINLINEFUN1(BInot,notInline)

OZ_Return andInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  TaggedRef nt = NameTrue;

  if (literalEq(A,nt)) {
    if (isAnyVar(B)) {
      return SUSPEND;
    }

    out = literalEq(B,nt) ? nt : NameFalse;
    return PROCEED;
  }

  if (isAnyVar(A)) {
    if (isAnyVar(B) || literalEq(B,nt)) {
      return SUSPEND;
    }
    out = NameFalse;
    return PROCEED;
  }

  out = NameFalse;
  return PROCEED;
}

DECLAREBI_USEINLINEFUN2(BIand,andInline)


OZ_Return orInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  TaggedRef nt = NameTrue;

  if (literalEq(A,nt)) {
    out = nt;
    return PROCEED;
  }

  if (literalEq(B,nt)) {
    out = nt;
    return PROCEED;
  }

  if (isAnyVar(tagA) || isAnyVar(tagB)) {
    return SUSPEND;
  }

  out = NameFalse;
  return PROCEED;
}

DECLAREBI_USEINLINEFUN2(BIor,orInline)



// ---------------------------------------------------------------------
// Atom
// ---------------------------------------------------------------------

OZ_C_proc_begin(BIatomToString,2)
{
  OZ_declareAtomArg(0,str);
  OZ_Term out = OZ_getCArg(1);

  return OZ_unify(out,OZ_string(str));
}
OZ_C_proc_end

OZ_C_proc_begin(BIstringToAtom,2)
{
  OZ_declareStringArg(0,str);
  OZ_Term out = OZ_getCArg(1);

  if (!str) return OZ_typeError(0,"String as Atom");

  OZ_Return ret = OZ_unifyAtom(out,str);

  return ret;
}
OZ_C_proc_end

// ---------------------------------------------------------------------
// Virtual Strings
// ---------------------------------------------------------------------


inline
TaggedRef vs_suspend(SRecord *vs, int i, TaggedRef arg_rest) {
  if (i == vs->getWidth()-1) {
    return arg_rest;
  } else {
    SRecord *stuple = SRecord::newSRecord(AtomPair, vs->getWidth() - i);
    stuple->setArg(0, arg_rest);
    i++;
    for (int j=1 ; i < vs->getWidth() ; (j++, i++))
      stuple->setArg(j, vs->getArg(i));
    return makeTaggedSRecord(stuple);
  }
}

static OZ_Return vs_check(OZ_Term vs, OZ_Term *rest) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isAnyVar(vs_tag)) {
    *rest = makeTaggedRef(vs_ptr);
    OZ_suspendOn(*rest);
  } else if (isInt(vs_tag)) {
    return PROCEED;
  } else if (isFloat(vs_tag)) {
    return PROCEED;
  } else if (isLiteral(vs_tag) && tagged2Literal(vs)->isAtom()) {
    return PROCEED;
  } else if (isCons(vs_tag)) {
    TaggedRef cdr  = vs;
    TaggedRef prev = vs;

    while (1) {
      DEREF(cdr, cdr_ptr, cdr_tag);

      if (isNil(cdr))
        return PROCEED;

      if (isAnyVar(cdr_tag)) {
        *rest = prev;
        OZ_suspendOn(makeTaggedRef(cdr_ptr));
      }

      if (!isCons(cdr_tag))
        return FAILED;

      TaggedRef car = tagged2LTuple(cdr)->getHead();
      DEREF(car, car_ptr, car_tag);

      if (isAnyVar(car_tag)) {
        *rest = cdr;
        OZ_suspendOn(makeTaggedRef(car_ptr));
      } else if (!isSmallInt(car_tag) ||
                 (smallIntValue(car) < 0) ||
                 (smallIntValue(car) > 255)) {
        return FAILED;
      } else {
        prev = cdr;
        cdr  = tagged2LTuple(cdr)->getTail();
      }
    };

    return FAILED;

  } else if (isSTuple(vs) &&
             literalEq(tagged2SRecord(vs)->getLabel(),AtomPair)) {
    for (int i=0; i < tagged2SRecord(vs)->getWidth(); i++) {
      TaggedRef arg_rest;
      OZ_Return status = vs_check(tagged2SRecord(vs)->getArg(i), &arg_rest);

      if (status == SUSPEND) {
        *rest = vs_suspend(tagged2SRecord(vs), i, arg_rest);
        return SUSPEND;
      } else if (status==FAILED) {
        return FAILED;
      }
    }
    return PROCEED;
  } else {
    return FAILED;
  }
}


static OZ_Return vs_length(OZ_Term vs, OZ_Term *rest, int *len) {
  DEREF(vs, vs_ptr, vs_tag);

  if (isAnyVar(vs_tag)) {
    *rest = makeTaggedRef(vs_ptr);
    OZ_suspendOn(*rest);
  } else if (isInt(vs_tag)) {
    *len = *len + strlen(toC(vs));
    return PROCEED;
  } else if (isFloat(vs_tag)) {
    *len = *len + strlen(toC(vs));
    return PROCEED;
  } else if (isLiteral(vs_tag) && tagged2Literal(vs)->isAtom()) {
    if (literalEq(vs,AtomPair) ||
        literalEq(vs,AtomNil))
      return PROCEED;
    *len = *len + strlen(tagged2Literal(vs)->getPrintName());
    return PROCEED;
  } else if (isCons(vs_tag)) {
    TaggedRef cdr  = vs;
    TaggedRef prev = vs;

    while (1) {
      DEREF(cdr, cdr_ptr, cdr_tag);

      if (isNil(cdr))
        return PROCEED;

      if (isAnyVar(cdr_tag)) {
        *rest = prev;
        Assert((*len)>0);
        *len = *len - 1;
        OZ_suspendOn(makeTaggedRef(cdr_ptr));
      }

      if (!isCons(cdr_tag))
        return FAILED;

      TaggedRef car = tagged2LTuple(cdr)->getHead();
      DEREF(car, car_ptr, car_tag);

      if (isAnyVar(car_tag)) {
        *rest = cdr;
        OZ_suspendOn(makeTaggedRef(car_ptr));
      } else if (!isSmallInt(car_tag) ||
                 (smallIntValue(car) < 0) ||
                 (smallIntValue(car) > 255)) {
        return FAILED;
      } else {
        prev = cdr;
        cdr  = tagged2LTuple(cdr)->getTail();
        *len = *len + 1;
      }
    };

    return FAILED;

  } else if (isSTuple(vs) &&
             literalEq(tagged2SRecord(vs)->getLabel(),AtomPair)) {
    for (int i=0; i < tagged2SRecord(vs)->getWidth(); i++) {
      TaggedRef arg_rest;
      OZ_Return status =
        vs_length(tagged2SRecord(vs)->getArg(i), &arg_rest, len);

      if (status == SUSPEND) {
        *rest = vs_suspend(tagged2SRecord(vs), i, arg_rest);
        return SUSPEND;
      } else if (status==FAILED) {
        return FAILED;
      }
    }
    return PROCEED;
  } else {
    return FAILED;
  }
}


OZ_C_proc_begin(BIvsLength,3) {
  TaggedRef rest = makeTaggedNULL();
  int len = smallIntValue(deref(OZ_args[1]));
  OZ_Return status = vs_length(OZ_args[0], &rest, &len);
  if (status == SUSPEND) {
    OZ_args[0] = rest;
    OZ_args[1] = makeTaggedSmallInt(len);
    return SUSPEND;
  } else if (status == FAILED) {
    TypeErrorT(0, "Virtual String");
  } else {
    return OZ_unify(OZ_args[2], makeTaggedSmallInt(len));
  }
} OZ_C_proc_end

OZ_C_proc_begin(BIvsIs,2) {
  TaggedRef rest = makeTaggedNULL();
  OZ_Return status = vs_check(OZ_args[0], &rest);
  if (status == SUSPEND) {
    OZ_args[0] = rest;
    return SUSPEND;
  }
  return OZ_unify(OZ_args[1], (status == PROCEED) ? NameTrue : NameFalse);
} OZ_C_proc_end


// ---------------------------------------------------------------------
// Chunk
// ---------------------------------------------------------------------

OZ_C_proc_begin(BInewChunk,2)
{
  OZ_Term val = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  if (OZ_isVariable(val)) OZ_suspendOn(val);
  val=deref(val);
  if (!isRecord(val)) TypeErrorT(0,"Record");

  return OZ_unify(out,OZ_newChunk(val));
}
OZ_C_proc_end

OZ_C_proc_begin(BIchunkArity,2)
{
  OZ_Term ch =  OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  if (OZ_isVariable(ch)) OZ_suspendOn(ch);
  ch=deref(ch);
  if (!isChunk(ch)) TypeErrorT(0,"Chunk");

  switch (tagged2Const(ch)->getType()) {
  case Co_Object:
    return OZ_unify(out,tagged2Object(ch)->getArityList());
  case Co_Chunk:
    return OZ_unify(out,tagged2SChunk(ch)->getArityList());
  case Co_Thread:
    return OZ_unify(out,tagged2Thread(ch)->getArityList());
  case Co_Dictionary:
  case Co_Array:
  default:
    // no features
    return OZ_unify(out,nil());
  }
}
OZ_C_proc_end


// ---------------------------------------------------------------------

OZ_C_proc_begin(BIsetThreadPriority,1)
{
  OZ_declareIntArg(0,prio);

  if (prio > OZMAX_PRIORITY || prio < OZMIN_PRIORITY) {
    TypeErrorT(0,"Int [0 ... 100]");
  }
  Thread *tt = am.currentThread;
  int oldPrio = tt->getPriority();
  tt->setPriority(prio);

  if (prio <= oldPrio) {
    am.setSFlag(ThreadSwitch);
  }

  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIgetThreadPriority,1)
{
  OZ_Term out = OZ_getCArg(0);

  return OZ_unifyInt(out,am.currentThread->getPriority());
}
OZ_C_proc_end

// ---------------------------------------------------------------------
// NAMES
// ---------------------------------------------------------------------

OZ_C_proc_begin(BInewName,1)
{
  OZ_Term out = OZ_getCArg(0);
  return OZ_unify(out,OZ_newName());
}
OZ_C_proc_end

// ---------------------------------------------------------------------
// term type
// ---------------------------------------------------------------------

OZ_Return BItermTypeInline(TaggedRef term, TaggedRef &out)
{
  out = OZ_termType(term);
  if (OZ_eq(out,OZ_atom("variable"))) {
    return SUSPEND;
  }
  return PROCEED;
}

DECLAREBI_USEINLINEFUN1(BItermType,BItermTypeInline)


// ---------------------------------------------------------------------
// Builtins ==, \=, ==B and \=B
// ---------------------------------------------------------------------

inline OZ_Return eqeqWrapper(TaggedRef Ain, TaggedRef Bin)
{
  TaggedRef A = Ain, B = Bin;
  DEREF(A,aPtr,tagA); DEREF(B,bPtr,tagB);

  /* Really fast test for equality */
  if (tagA != tagB) {
    if (isAnyVar(A) || isAnyVar(B)) goto dontknow;
    return FAILED;
  }

  if (isSmallInt(tagA)) return smallIntEq(A,B) ? PROCEED : FAILED;
  if (isBigInt(tagA))   return bigIntEq(A,B)   ? PROCEED : FAILED;
  if (isFloat(tagA))    return floatEq(A,B)    ? PROCEED : FAILED;

  if (isLiteral(tagA))  return literalEq(A,B)  ? PROCEED : FAILED;

  if (A == B && !isAnyVar(A)) return PROCEED;

 dontknow:
  am.trail.pushMark();
  Bool ret = am.unify(Ain,Bin,NO);
  if (ret == NO) {
    am.reduceTrailOnFail();
    return FAILED;
  }

  if (am.trail.isEmptyChunk()) {
    am.trail.popMark();
    return PROCEED;
  }

  am.reduceTrailOnShallow(NULL);

  return SUSPEND;
}


OZ_C_proc_begin(BIeq,2)
{
  TaggedRef A = OZ_getCArg(0);
  TaggedRef B = OZ_getCArg(1);
  return eqeqWrapper(A,B);
}
OZ_C_proc_end


OZ_C_proc_begin(BIneq,2)
{
  TaggedRef A = OZ_getCArg(0);
  TaggedRef B = OZ_getCArg(1);
  return eqeqWrapper(A,B);
}
OZ_C_proc_end



OZ_Return neqInline(TaggedRef A, TaggedRef B, TaggedRef &out);
OZ_Return eqeqInline(TaggedRef A, TaggedRef B, TaggedRef &out);


OZ_C_proc_begin(BIneqB,3)
{
  OZ_Term help;
  OZ_Return ret=neqInline(OZ_getCArg(0),OZ_getCArg(1),help);
  return ret==PROCEED ? OZ_unify(help,OZ_getCArg(2)) : ret;
}
OZ_C_proc_end

OZ_C_proc_begin(BIeqB,3)
{
  OZ_Term help;
  OZ_Return ret=eqeqInline(OZ_getCArg(0),OZ_getCArg(1),help);
  return ret==PROCEED ? OZ_unify(help,OZ_getCArg(2)): ret;
}
OZ_C_proc_end


OZ_Return eqeqInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  switch(eqeqWrapper(A,B)) {
  case PROCEED:
    out = NameTrue;
    return PROCEED;
  case FAILED:
    out = NameFalse;
    return PROCEED;
  case SUSPEND:
  default:
    return SUSPEND;
  }
}


OZ_Return neqInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  switch(eqeqWrapper(A,B)) {
  case PROCEED:
    out = NameFalse;
    return PROCEED;
  case FAILED:
    out = NameTrue;
    return PROCEED;
  case SUSPEND:
  default:
    return SUSPEND;
  }
}

OZ_C_proc_begin(BIisString,2)
{
  OZ_Term in=OZ_getCArg(0);
  OZ_Term out=OZ_getCArg(1);

  OZ_Term var;
  if (!OZ_isString(in,&var)) {
    if (var == 0) return OZ_unify(out,NameFalse);
    OZ_suspendOn(var);
  }
  return OZ_unify(out,NameTrue);
}
OZ_C_proc_end

#endif /* BUILTINS2 */

#ifdef BUILTINS1


#define FirstCharArg(NAME)                      \
 int i;                                         \
 OZ_nonvarArg(0);                               \
 if (!OZ_isInt(OZ_getCArg(0))) {                \
   return OZ_typeError(1,"Char");               \
 } else {                                       \
   i = OZ_intToC(OZ_getCArg(0));                \
   if ((i < 0) || (i > 255)) {                  \
     return OZ_typeError(1,"Char");             \
   }                                            \
 }

OZ_C_proc_begin(BIcharIs,2) {
 OZ_declareNonvarArg(0,c);
 OZ_declareArg(1,out);
 c = deref(c);
 if (!isSmallInt(c)) return OZ_unify(out,NameFalse);
 int i = smallIntValue(c);
 return OZ_unify(out,(i >=0 && i <= 255) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsAlNum,2) {
  FirstCharArg("Char.isAlNum");
  return OZ_unify(OZ_getCArg(1), isalnum(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsAlpha,2) {
  FirstCharArg("Char.isAlpha");
  return OZ_unify(OZ_getCArg(1), isalpha(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsCntrl,2) {
  FirstCharArg("Char.isCntrl");
  return OZ_unify(OZ_getCArg(1), iscntrl(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsDigit,2) {
  FirstCharArg("Char.isDigit");
  return OZ_unify(OZ_getCArg(1), isdigit(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsGraph,2) {
  FirstCharArg("Char.isGraph");
  return OZ_unify(OZ_getCArg(1), isgraph(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsLower,2) {
  FirstCharArg("Char.isLower");
  return OZ_unify(OZ_getCArg(1), islower(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsPrint,2) {
  FirstCharArg("Char.isPrint");
  return OZ_unify(OZ_getCArg(1), isprint(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsPunct,2) {
  FirstCharArg("Char.isPunct");
  return OZ_unify(OZ_getCArg(1), ispunct(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsSpace,2) {
  FirstCharArg("Char.isSpace");
  return OZ_unify(OZ_getCArg(1), isspace(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsUpper,2) {
  FirstCharArg("Char.isUpper");
  return OZ_unify(OZ_getCArg(1), isupper(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsXDigit,2) {
  FirstCharArg("Char.isXDigit");
  return OZ_unify(OZ_getCArg(1), isxdigit(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharToLower,2) {
  FirstCharArg("Char.toLower");
  return OZ_unifyInt(OZ_getCArg(1), tolower(i));
} OZ_C_proc_end

OZ_C_proc_begin(BIcharToUpper,2) {
  FirstCharArg("Char.toUpper");
  return OZ_unifyInt(OZ_getCArg(1), toupper(i));
} OZ_C_proc_end

OZ_C_proc_begin(BIcharToAtom,2) {
  FirstCharArg("Char.toAtom");
  if (i) {
     char s[2]; s[0]= (char) i; s[1]='\0';
     return OZ_unify(OZ_getCArg(1), makeTaggedAtom(s));
  }
  return OZ_unify(OZ_getCArg(1), AtomEmpty);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharType,2) {
  FirstCharArg("Char.type");
  TaggedRef type;
  if (isupper(i))      type = AtomUpper;
  else if (islower(i)) type = AtomLower;
  else if (isdigit(i)) type = AtomDigit;
  else if (isspace(i)) type = AtomCharSpace;
  else if (ispunct(i)) type = AtomPunct;
  else                 type = AtomOther;
  return OZ_unify(OZ_getCArg(1), type);
} OZ_C_proc_end


/*
 *      Construct a new SRecord to be a copy of old.
 *      This is the functionality of adjoin(old,newlabel).
 */
OZ_C_proc_begin(BIcopyRecord,2)
{
  OZ_Term in = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);
  NONVAR(in,rec,tag);

  switch (tag) {
  case SRECORD:
    {
      SRecord *rec0 = tagged2SRecord(rec);
      SRecord *rec1 = SRecord::newSRecord(rec0);
      return OZ_unify(out,makeTaggedSRecord(rec1));
    }
  case LITERAL:
    return OZ_unify(out,rec);

  default:
    TypeErrorT(0,"Determined Record");
  }
}
OZ_C_proc_end



/*===================================================================
 * Records
 *=================================================================== */

OZ_Return BIadjoinInline(TaggedRef t0, TaggedRef t1, TaggedRef &out)
{
  DEREF(t0,_0,tag0);
  DEREF(t1,_1,tag1);

  switch (tag0) {
  case LITERAL:
    switch (tag1) {
    case SRECORD:
    case LITERAL:
      out = t1;
      return PROCEED;
    case UVAR:
    case SVAR:
      return SUSPEND;
    case CVAR:
      if (tagged2CVar(t1)->getType()!=OFSVariable)
          TypeErrorT(1,"Record");
      return SUSPEND;
    default:
      TypeErrorT(1,"Record");
    }
  case LTUPLE:
  case SRECORD:
    {
      SRecord *rec= makeRecord(t0);
      switch (tag1) {
      case LITERAL:
        {
          SRecord *newrec = rec->replaceLabel(t1);
          out = newrec->normalize();
          return PROCEED;
        }
      case SRECORD:
      case LTUPLE:
        {
          out = rec->adjoin(makeRecord(t1));
          return PROCEED;
        }
      case UVAR:
      case SVAR:
        return SUSPEND;
      case CVAR:
        if (tagged2CVar(t1)->getType()!=OFSVariable)
            TypeErrorT(1,"Record");
        return SUSPEND;
      default:
        TypeErrorT(1,"Record");
      }
    }
  case UVAR:
  case SVAR:
  case CVAR:
    if (tag0==CVAR && tagged2CVar(t0)->getType()!=OFSVariable)
        TypeErrorT(0,"Record");
    switch (tag1) {
    case UVAR:
    case SVAR:
    case SRECORD:
    case LTUPLE:
    case LITERAL:
      return SUSPEND;
    case CVAR:
      if (tagged2CVar(t1)->getType()!=OFSVariable)
        TypeErrorT(1,"Record");
      return SUSPEND;
    default:
      TypeErrorT(1,"Record");
    }
  default:
    TypeErrorT(0,"Record");
  }
}

DECLAREBI_USEINLINEFUN2(BIadjoin,BIadjoinInline)

OZ_C_proc_begin(BIadjoinAt,4)
{
  OZ_Term rec = OZ_getCArg(0);
  OZ_Term fea = OZ_getCArg(1);
  OZ_Term value = OZ_getCArg(2);
  OZ_Term out = OZ_getCArg(3);

  DEREF(rec,recPtr,tag0);
  DEREF(fea,feaPtr,tag1);

  switch (tag0) {
  case LITERAL:
    if (isFeature(fea)) {
      SRecord *newrec = SRecord::newSRecord(rec,aritytable.find(cons(fea,nil())));
      newrec->setArg(0,value);
      return OZ_unify(out, makeTaggedSRecord(newrec));
    }
    if (isNotCVar(fea)) {
      OZ_suspendOn(makeTaggedRef(feaPtr));
    }
    if (isCVar(fea)) {
      if (tagged2CVar(fea)->getType()!=OFSVariable ||
          tagged2GenOFSVar(fea)->getWidth()>0)
        TypeErrorT(1,"Feature");
      OZ_suspendOn(makeTaggedRef(feaPtr));;
    }
    TypeErrorT(1,"Feature");

  case SRECORD:
    {
      SRecord *rec1 = tagged2SRecord(rec);
      if (isAnyVar(tag1)) {
        OZ_suspendOn(makeTaggedRef(feaPtr));
      }
      if (!isFeature(tag1)) {
        TypeErrorT(1,"Feature");
      }
      return OZ_unify(out,rec1->adjoinAt(fea,value));
    }

  case UVAR:
  case SVAR:
  case CVAR:
    if (tag0==CVAR && tagged2CVar(rec)->getType()!=OFSVariable)
        TypeErrorT(0,"Record");
    if (isFeature(fea) || isNotCVar(fea)) {
      OZ_suspendOn(makeTaggedRef(recPtr));
    }
    if (isCVar(fea)) {
      if (tagged2CVar(fea)->getType()!=OFSVariable ||
          tagged2GenOFSVar(fea)->getWidth()>0)
        TypeErrorT(1,"Feature");
      OZ_suspendOn(makeTaggedRef(recPtr));
    }
    TypeErrorT(1,"Feature");

  default:
    TypeErrorT(0,"Record");
  }
}
OZ_C_proc_end

TaggedRef getArity(TaggedRef list)
{
  TaggedRef arity;
  TaggedRef *next=&arity;
loop:
  DEREF(list,listPtr,_l1);
  if (isLTuple(list)) {
    TaggedRef pair = head(list);
    DEREF(pair,pairPtr,_e1);
    if (isAnyVar(pair)) return makeTaggedRef(pairPtr);
    if (!OZ_isPair2(pair)) goto bomb;

    TaggedRef fea = tagged2SRecord(pair)->getArg(0);
    DEREF(fea,feaPtr,_f1);

    if (isAnyVar(fea)) return makeTaggedRef(feaPtr);
    if (!isFeature(fea)) goto bomb;

    LTuple *lt=new LTuple();
    *next=makeTaggedLTuple(lt);
    lt->setHead(fea);
    next=lt->getRefTail();

    list = tail(list);
    goto loop;
  }

  if (isAnyVar(list)) return makeTaggedRef(listPtr);
  if (isNil(list)) {
    *next=nil();
    return arity;
  }

bomb:
  return makeTaggedNULL(); // FAIL
}

/* common subroutine for builtins adjoinList and record:
   recordFlag=OK: 'adjoinList' allows records as first arg
              N0: 'makeRecord' only allows literals */

OZ_Return adjoinPropListInline(TaggedRef t0, TaggedRef list, TaggedRef &out,
                           Bool recordFlag)
{
  TaggedRef arity=getArity(list);
  if (arity == makeTaggedNULL()) {
    TypeErrorM("incorrect pairlist found");
  }
  DEREF(t0,t0Ptr,tag0);
  if (isRef(arity)) { // must suspend
    out=arity;
    switch (tag0) {
    case UVAR:
    case SVAR:
    case LITERAL:
      return SUSPEND;
    case SRECORD:
    case LTUPLE:
      if (recordFlag) {
        return SUSPEND;
      }
      goto typeError0;
    case CVAR:
      if (tagged2CVar(t0)->getType()!=OFSVariable)
          goto typeError0;
      if (recordFlag) {
        return SUSPEND;
      }
      goto typeError0;
    default:
      goto typeError0;
    }
  }

  if (isNil(arity)) { // adjoin nothing
    switch (tag0) {
    case SRECORD:
    case LTUPLE:
      if (recordFlag) {
        out = t0;
        return PROCEED;
      }
      goto typeError0;
    case LITERAL:
      out = t0;
      return PROCEED;
    case UVAR:
    case SVAR:
      out=makeTaggedRef(t0Ptr);
      return SUSPEND;
    case CVAR:
      if (tagged2CVar(t0)->getType()!=OFSVariable)
          goto typeError0;
      out=makeTaggedRef(t0Ptr);
      return SUSPEND;
    default:
      goto typeError0;
    }
  }

  switch (tag0) {
  case LITERAL:
    {
      int len=length(arity);
      arity = sortlist(arity,len);
      len=length(arity); // NOTE: duplicates may be removed
      SRecord *newrec = SRecord::newSRecord(t0,aritytable.find(arity));
      newrec->setFeatures(list);
      out = makeTaggedSRecord(newrec);
      return PROCEED;
    }
  case SRECORD:
  case LTUPLE:
    if (recordFlag) {
      SRecord *rec1 = makeRecord(t0);
      out = rec1->adjoinList(arity,list);
      return PROCEED;
    }
    goto typeError0;
  case UVAR:
  case SVAR:
    out=makeTaggedRef(t0Ptr);
    return SUSPEND;
  case CVAR:
    if (tagged2CVar(t0)->getType()!=OFSVariable)
        goto typeError0;
    out=makeTaggedRef(t0Ptr);
    return SUSPEND;
  default:
    goto typeError0;
  }

 typeError0:
  if (recordFlag) {
    TypeErrorT(0,"Record");
  } else {
    TypeErrorT(0,"Literal");
  }
}

OZ_Return adjoinPropList(TaggedRef t0, TaggedRef list, TaggedRef &out,
                     Bool recordFlag)
{
  return adjoinPropListInline(t0,list,out,recordFlag);
}


OZ_C_proc_begin(BIadjoinList,3)
{
  OZ_Term help;

  OZ_Return state = adjoinPropListInline(OZ_getCArg(0),OZ_getCArg(1),help,OK);
  switch (state) {
  case SUSPEND:
    OZ_suspendOn(help);
  case PROCEED:
    return(OZ_unify(help,OZ_getCArg(2)));
  default:
    return state;
  }
}
OZ_C_proc_end


OZ_C_proc_begin(BImakeRecord,3)
{
  OZ_Term help;

  OZ_Return state = adjoinPropListInline(OZ_getCArg(0),OZ_getCArg(1),help,NO);
  switch (state) {
  case SUSPEND:
    OZ_suspendOn(help);
    return PROCEED;
  case PROCEED:
    return(OZ_unify(help,OZ_getCArg(2)));
  default:
    return state;
  }
}
OZ_C_proc_end


OZ_Return BIarityInline(TaggedRef term, TaggedRef &out)
{
  DEREF(term,termPtr,tag);

  out = getArityList(term);
  if (out) return PROCEED;
  if (isNotCVar(tag)) return SUSPEND;
  if (isCVar(tag)) {
    if (tagged2CVar(term)->getType()!=OFSVariable)
      TypeErrorT(0,"Record");
    return SUSPEND;
  }
  TypeErrorT(0,"Record");
}

DECLAREBI_USEINLINEFUN1(BIarity,BIarityInline)

void assignError(TaggedRef rec, TaggedRef fea, char *name)
{
  prefixError();
  message("Object Error\n");
  message("Assignment (%s) failed: bad attribute or state\n",name);
  message("attribute found  : %s\n", toC(fea));
  message("state found      : %s\n", toC(rec));
}

#define CheckSelf                               \
     Assert(am.getSelf() != NULL);              \
     { Object *o = am.getSelf();                \
       Assert(o->getDeepness()>=1);             \
     }


OZ_Return atInline(TaggedRef fea, TaggedRef &out)
{
  DEREF(fea, _1, feaTag);

  SRecord *rec = am.getSelf()->getState();
  if (rec) {
    if (!isFeature(fea)) {
      if (isAnyVar(fea)) {
        return SUSPEND;
      }
      goto bomb;
    }
    CheckSelf;
    TaggedRef t = rec->getFeature(fea);
    if (t) {
      out = t;
      return PROCEED;
    }
  }

bomb:
  TypeErrorT(1,"(valid) Feature");
}
DECLAREBI_USEINLINEFUN1(BIat,atInline)

OZ_Return assignInline(TaggedRef fea, TaggedRef value)
{
  DEREF(fea, _2, feaTag);

  SRecord *r = am.getSelf()->getState();
  if (r) {
    CheckSelf;
    if (!isFeature(fea)) {
      if (isAnyVar(fea)) {
        return SUSPEND;
      }
      goto bomb;
    }
    if (r->replaceFeature(fea,value) == makeTaggedNULL()) {
      goto bomb;
    }
    return PROCEED;
  }

 bomb:
  assignError(r?makeTaggedSRecord(r):OZ_atom("noattributes"),
              fea,"<-");
  return PROCEED;
}

DECLAREBI_USEINLINEREL2(BIassign,assignInline)

#endif /* BUILTINS1 */


/* -----------------------------------------------------------------------
   suspending
   ----------------------------------------------------------------------- */

#ifdef BUILTINS1

static OZ_Return bombBuiltin(char *type)
{
  return OZ_raise(OZ_mkTupleC("typeError",1,
                              OZ_mkTupleC("type",1,OZ_atom(type))));
}

#define suspendTest(A,B,test,type)                      \
  if (isAnyVar(A)) {                                    \
    if (isAnyVar(B) || test(B)) { return SUSPEND; }     \
    return bombBuiltin(type);                           \
  }                                                     \
  if (isAnyVar(B)) {                                    \
    if (isNumber(A)) { return SUSPEND; }                \
  }                                                     \
  return bombBuiltin(type);


static OZ_Return suspendOnNumbers(TaggedRef A, TaggedRef B)
{
  suspendTest(A,B,isNumber,"Number");
}

inline Bool isNumOrAtom(TaggedRef t)
{
  return isNumber(t) || isAtom(t);
}

static OZ_Return suspendOnNumbersAndAtoms(TaggedRef A, TaggedRef B)
{
  suspendTest(A,B,isNumOrAtom,"Number or Atom");
}

static OZ_Return suspendOnFloats(TaggedRef A, TaggedRef B)
{
  suspendTest(A,B,isFloat,"Float");
}


static OZ_Return suspendOnInts(TaggedRef A, TaggedRef B)
{
  suspendTest(A,B,isInt,"integer");
}

#undef suspendTest





/* -----------------------------------
   Z = X op Y
   ----------------------------------- */

// Float x Float -> Float
OZ_Return BIfdivInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);
  if (isFloat(tagA) && isFloat(tagB)) {
    out = makeTaggedFloat(floatValue(A) / floatValue(B));
    return PROCEED;
  }
  return suspendOnFloats(A,B);
}


// Int x Int -> Int
#define BIGOP(op)                                                             \
  if (tagA == BIGINT) {                                                       \
    if (tagB == BIGINT) {                                                     \
      out = tagged2BigInt(A)->op(tagged2BigInt(B));                           \
      return PROCEED;                                                         \
    }                                                                         \
    if (tagB == SMALLINT) {                                                   \
      BigInt *b = new BigInt(smallIntValue(B));                               \
      out = tagged2BigInt(A)->op(b);                                          \
      b->dispose();                                                           \
      return PROCEED;                                                         \
    }                                                                         \
  }                                                                           \
  if (tagB == BIGINT) {                                                       \
    if (tagA == SMALLINT) {                                                   \
      BigInt *a = new BigInt(smallIntValue(A));                               \
      out = a->op(tagged2BigInt(B));                                          \
      a->dispose();                                                           \
      return PROCEED;                                                         \
    }                                                                         \
  }

// Integer x Integer -> Integer
OZ_Return BIdivInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if ((tagB == SMALLINT && smallIntValue(B) == 0)) {
    OZ_warning("div(%s,%s): division by zero",
               toC(A),toC(B));
    return FAILED;

  }

  if ( (tagA == SMALLINT) && (tagB == SMALLINT)) {
    out = newSmallInt(smallIntValue(A) / smallIntValue(B));
    return PROCEED;
  }
  BIGOP(div);
  return suspendOnInts(A,B);
}

// Integer x Integer -> Integer
OZ_Return BImodInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if ((tagB == SMALLINT && smallIntValue(B) == 0)) {
    OZ_warning("mod(%s,0): division by zero",toC(A));
    return FAILED;
  }

  if ( (tagA == SMALLINT) && (tagB == SMALLINT)) {
    out = newSmallInt(smallIntValue(A) % smallIntValue(B));
    return PROCEED;
  }

  BIGOP(mod);
  return suspendOnInts(A,B);
}


/* Division is slow on RISC (at least SPARC)
 *  --> first make a simpler test for no overflow
 */

inline
int multOverflow(int a, int b)
{
  int absa = ozabs(a);
  int absb = ozabs(b);
  const int bits = (sizeof(TaggedRef)*8-tagSize)/2 - 1;

  if (!((absa|absb)>>bits)) /* if none of the 13 MSB in neither a nor b are set */
    return NO;
  return ((b!=0) && (absa >= OzMaxInt / absb));
}


OZ_Return BImultInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == SMALLINT && tagB == SMALLINT) {
    int valA = smallIntValue(A);
    int valB = smallIntValue(B);
    if ( multOverflow(valA,valB) ) {
      BigInt *a = new BigInt(valA);
      BigInt *b = new BigInt(valB);
      out = a->mul(b);
      a->dispose();
      b->dispose();
      return PROCEED;
    } else {
      out = newSmallInt(valA*valB);
      return PROCEED;
    }
  }

  if (isFloat(tagA) && isFloat(tagB)) {
    out = makeTaggedFloat(floatValue(A) * floatValue(B));
    return PROCEED;
  }

  BIGOP(mul);
  return suspendOnNumbers(A,B);
}


OZ_Return BIminusInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if ( (tagA == SMALLINT) && (tagB == SMALLINT) ) {
    out = makeInt(smallIntValue(A) - smallIntValue(B));
    return PROCEED;
  }

  if (isFloat(tagA) && isFloat(tagB)) {
    out = makeTaggedFloat(floatValue(A) - floatValue(B));
    return PROCEED;
  }

  BIGOP(sub);
  return suspendOnNumbers(A,B);
}

OZ_Return BIplusInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if ( (tagA == SMALLINT) && (tagB == SMALLINT) ) {
    out = makeInt(smallIntValue(A) + smallIntValue(B));
    return PROCEED;
  }

  if (isFloat(tagA) && isFloat(tagB)) {
    out = makeTaggedFloat(floatValue(A) + floatValue(B));
    return PROCEED;
  }

  BIGOP(add);
  return suspendOnNumbers(A,B);
}
#undef BIGOP

/* -----------------------------------
   Z = op X
   ----------------------------------- */

// unary minus: Number -> Number
OZ_Return BIuminusInline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1,tagA);

  switch(tagA) {

  case OZFLOAT:
    out = makeTaggedFloat(-floatValue(A));
    return PROCEED;

  case SMALLINT:
    out = newSmallInt(-smallIntValue(A));
    return PROCEED;

  case BIGINT:
    out = tagged2BigInt(A)->neg();
    return PROCEED;

  case UVAR:
  case SVAR:
    return SUSPEND;

  default:
    return FAILED;
  }
}

OZ_Return BIabsInline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1,tagA);

  if (isSmallInt(tagA)) {
    int i = smallIntValue(A);
    out = (i >= 0) ? A : newSmallInt(-i);
    return PROCEED;
  }

  if (isFloat(tagA)) {
    double f = floatValue(A);
    out = (f >= 0.0) ? A : makeTaggedFloat(fabs(f));
    return PROCEED;
  }

  if (isBigInt(tagA)) {
    BigInt *b = tagged2BigInt(A);
    out = (b->cmp(0l) >= 0) ? A : b->neg();
    return PROCEED;
  }

  if (isAnyVar(tagA)){
    return SUSPEND;
  }

  return FAILED;
}

// add1(X) --> X+1
OZ_Return BIadd1Inline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1,tagA);

  if (isSmallInt(tagA)) {
    /* INTDEP */
    int res = (int)A + (1<<tagSize);
    out = (int)A >= res ? makeInt(smallIntValue(A)+1) : res;
    return PROCEED;
  }

  if (isBigInt(tagA)) {
    return BIplusInline(A,newSmallInt(1),out);
  }

  if (isAnyVar(tagA)) {
    return SUSPEND;
  }

  return FAILED;
}

// sub1(X) --> X-1
OZ_Return BIsub1Inline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1,tagA);

  if (isSmallInt(tagA)) {
    /* INTDEP */
    int res = (int)A - (1<<tagSize);
    out = (int)A <= res ? makeInt(smallIntValue(A)-1) : res;
    return PROCEED;
  }

  if (isBigInt(tagA)) {
    return BIminusInline(A,newSmallInt(1),out);
  }

  if (isAnyVar(tagA)) {
    return SUSPEND;
  }

  return FAILED;
}


/* -----------------------------------
   X test Y
   ----------------------------------- */

OZ_Return bigintLess(BigInt *A, BigInt *B)
{
  return (A->cmp(B) < 0 ? PROCEED : FAILED);
}


OZ_Return bigintLe(BigInt *A, BigInt *B)
{
  return (A->cmp(B) <= 0 ? PROCEED : FAILED);
}


OZ_Return bigtest(TaggedRef A, TaggedRef B,
                  OZ_Return (*test)(BigInt*, BigInt*))
{
  if (isBigInt(A)) {
    if (isBigInt(B)) {
      return test(tagged2BigInt(A),tagged2BigInt(B));
    }
    if (isSmallInt(B)) {
      BigInt *b = new BigInt(smallIntValue(B));
      OZ_Return res = test(tagged2BigInt(A),b);
      b->dispose();
      return res;
    }
  }
  if (isBigInt(B)) {
    if (isSmallInt(A)) {
      BigInt *a = new BigInt(smallIntValue(A));
      OZ_Return res = test(a,tagged2BigInt(B));
      a->dispose();
      return res;
    }
  }
  if (isAnyVar(A) || isAnyVar(B))
    return SUSPEND;

  TypeErrorM("Not comparable");
}




OZ_Return BIminInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == tagB) {
    switch(tagA) {
    case SMALLINT: out = (smallIntLess(A,B) ? A : B);             return PROCEED;
    case OZFLOAT:  out = (floatValue(A) < floatValue(B)) ? A : B; return PROCEED;
    case LITERAL:
      if (isAtom(A) && isAtom(B)) {
        out = (strcmp(tagged2Literal(A)->getPrintName(),
                      tagged2Literal(B)->getPrintName()) < 0)
          ? A : B;
        return PROCEED;
      }
      TypeErrorM("Number or Atom");
      return FAILED;

    default: break;
    }
  }

  OZ_Return ret = bigtest(A,B,bigintLess);
  switch (ret) {
  case PROCEED: out = A; return PROCEED;
  case FAILED:  out = B; return PROCEED;
  case RAISE:   return RAISE;
  default:      break;
  }

  return suspendOnNumbersAndAtoms(A,B);
}


/* code adapted from min */
OZ_Return BImaxInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == tagB) {
    switch(tagA) {
    case SMALLINT: out = (smallIntLess(A,B) ? B : A);             return PROCEED;
    case OZFLOAT:  out = (floatValue(A) < floatValue(B)) ? B : A; return PROCEED;
    case LITERAL:
      if (isAtom(A) && isAtom(B)) {
        out = (strcmp(tagged2Literal(A)->getPrintName(),
                      tagged2Literal(B)->getPrintName()) < 0)
          ? B : A;
        return PROCEED;
      }
      TypeErrorM("Names are not ordered");
      return FAILED;

    default: break;
    }
  }

  OZ_Return ret = bigtest(A,B,bigintLess);
  switch (ret) {
  case PROCEED: out = B; return PROCEED;
  case FAILED:  out = A; return PROCEED;
  case RAISE:   return RAISE;
  default:      break;
  }

  return suspendOnNumbersAndAtoms(A,B);
}


OZ_Return BIlessInline(TaggedRef A, TaggedRef B)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == tagB) {
    if (tagA == SMALLINT) {
      return (smallIntLess(A,B) ? PROCEED : FAILED);
    }

    if (isFloat(tagA)) {
      return (floatValue(A) < floatValue(B)) ? PROCEED : FAILED;
    }

    if (tagA == LITERAL) {
      if (isAtom(A) && isAtom(B)) {
        return (strcmp(tagged2Literal(A)->getPrintName(),
                       tagged2Literal(B)->getPrintName()) < 0)
          ? PROCEED : FAILED;
      }
      TypeErrorM("Names are not ordered");
      return FAILED;
    }
  }

  OZ_Return ret = bigtest(A,B,bigintLess);
  if (ret!=SUSPEND)
    return ret;

  return suspendOnNumbersAndAtoms(A,B);
}



OZ_Return BInumeqInline(TaggedRef A, TaggedRef B)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == tagB) {
    if (isSmallInt(tagA)) if (smallIntEq(A,B)) goto proceed; goto failed;
    if (isFloat(tagA))    if (floatEq(A,B))    goto proceed; goto failed;
    if (isBigInt(tagA))   if (bigIntEq(A,B))   goto proceed; goto failed;
  }

  return suspendOnNumbers(A,B);

 failed:
  return FAILED;

 proceed:
  return PROCEED;
}


OZ_Return BInumeqInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  switch (BInumeqInline(A,B)) {
  case PROCEED:
    out = NameTrue;
    return PROCEED;
  case FAILED:
    out = NameFalse;
    return PROCEED;
  case SUSPEND:
  default:
    return SUSPEND;
  }
}

OZ_Return BInumneqInline(TaggedRef A, TaggedRef B)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == tagB) {
    if (isSmallInt(tagA)) if(smallIntEq(A,B)) goto failed; goto proceed;
    if (isFloat(tagA))    if(floatEq(A,B))    goto failed; goto proceed;
    if (isBigInt(tagA))   if(bigIntEq(A,B))   goto failed; goto proceed;
  }

  return suspendOnNumbers(A,B);

 failed:
  return FAILED;

 proceed:
  return PROCEED;
}


OZ_Return BInumneqInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  OZ_Return ret = BInumneqInline(A,B);
  switch (ret) {
  case PROCEED: out = NameTrue;  return PROCEED;
  case FAILED:  out = NameFalse; return PROCEED;
  default:      return ret;
  }
}

OZ_Return BIlessInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  OZ_Return ret = BIlessInline(A,B);
  switch (ret) {
  case PROCEED: out = NameTrue;  return PROCEED;
  case FAILED:  out = NameFalse; return PROCEED;
  default:      return ret;
  }
}

OZ_Return BIgreatInline(TaggedRef A, TaggedRef B)
{
  return BIlessInline(B,A);
}

OZ_Return BIgreatInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  return BIlessInlineFun(B,A,out);
}


OZ_Return BIleInline(TaggedRef A, TaggedRef B)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == tagB) {

    if (tagA == SMALLINT) {
      return (smallIntLE(A,B) ? PROCEED : FAILED);
    }

    if (isFloat(tagA)) {
      return (floatValue(A) <= floatValue(B)) ? PROCEED : FAILED;
    }

    if (tagA == LITERAL) {
      if (isAtom(A) && isAtom(B)) {
        return (strcmp(tagged2Literal(A)->getPrintName(),
                       tagged2Literal(B)->getPrintName()) <= 0)
          ? PROCEED : FAILED;
      }
      TypeErrorM("Names are not ordered");
      return FAILED;
    }

  }

  OZ_Return ret = bigtest(A,B,bigintLe);
  if (ret!=SUSPEND)
    return ret;

  return suspendOnNumbersAndAtoms(A,B);
}


OZ_Return BIleInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  switch (BIleInline(A,B)) {
  case PROCEED:
    out = NameTrue;
    return PROCEED;
  case FAILED:
    out = NameFalse;
    return PROCEED;
  case SUSPEND:
  default:
    return SUSPEND;
  }
}

OZ_Return BIgeInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  return BIleInlineFun(B,A,out);
}


OZ_Return BIgeInline(TaggedRef A, TaggedRef B)
{
  return BIleInline(B,A);
}

/* -----------------------------------
   X = conv(Y)
   ----------------------------------- */


OZ_Return BIintToFloatInline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1,_2);
  if (isSmallInt(A)) {
    out = makeTaggedFloat((double)smallIntValue(A));
    return PROCEED;
  }
  if (isBigInt(A)) {
    char *s = toC(A);
    out = OZ_CStringToFloat(s);
    return PROCEED;
  }

  if (OZ_isVariable(A)) {
    return SUSPEND;
  }

  return FAILED;
}

/* mm2: I don't know if this is efficient, but it's only called,
   when rounding "xx.5" */
inline
Bool ozisodd(double ff)
{
  double m = ff/2;
  return m != floor(m);
}

/* ozround -- round float to int as required by IEEE Standard */
inline
double ozround(double in) {
  double ff = floor(in);
  double diff = in-ff;
  if (diff > 0.5 || (diff == 0.5 && ozisodd(ff))) {
    ff += 1;
  }
  return ff;
}

OZ_Return BIfloatToIntInline(TaggedRef A, TaggedRef &out)
{
  A=OZ_deref(A);
  if (OZ_isFloat(A)) {
    double ff = ozround(OZ_floatToC(A));
    if (ff > INT_MAX || ff < INT_MIN) {
      OZ_warning("float to int: truncated to signed 32 Bit\n");
    }
    out = makeInt((int) ff);
    return PROCEED;
  }

  if (OZ_isVariable(A)) {
    return SUSPEND;
  }

  return FAILED;
}

OZ_C_proc_begin(BIfloatToString, 2)
{
  OZ_nonvarArg(0);

  OZ_Term in = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  if (OZ_isFloat(in)) {
    char *s = toC(in);
    OZ_Return ret = OZ_unify(out,OZ_string(s));
    return ret;
  }
  TypeErrorT(0,"Float");
  return FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIstringToFloat, 2)
{
  OZ_declareStringArg(0,str);
  OZ_declareArg(1,out);

  if (!str) return OZ_typeError(0,"String as Float");

  char *end = OZ_parseFloat(str);
  if (!end || *end != 0) {
    return FAILED;
  }
  OZ_Return ret = OZ_unify(out,OZ_CStringToFloat(str));
  return ret;
}
OZ_C_proc_end

OZ_C_proc_begin(BIstringIsFloat, 2)
{
  OZ_declareStringArg(0,str);
  OZ_declareArg(1,out);

  if (!str) return OZ_unify(out,NameFalse);

  char *end = OZ_parseFloat(str);

  if (!end || *end != 0) {
    return OZ_unify(out,NameFalse);
  }

  return OZ_unify(out,NameTrue);
}
OZ_C_proc_end

OZ_C_proc_begin(BIstringToInt, 2)
{
  OZ_declareStringArg(0,str);
  OZ_declareArg(1,out);

  if (!str) return OZ_typeError(0,"String as Int");

  char *end = OZ_parseInt(str);
  if (!end || *end != 0) {
    return FAILED;
  }
  OZ_Return ret = OZ_unify(out,OZ_CStringToInt(str));
  return ret;
}
OZ_C_proc_end

OZ_C_proc_begin(BIstringIsInt, 2)
{
  OZ_declareStringArg(0,str);
  OZ_declareArg(1,out);

  if (!str) return OZ_unify(out,NameFalse);

  char *end = OZ_parseInt(str);

  if (!end || *end != 0) {
    return OZ_unify(out,NameFalse);
  }

  return OZ_unify(out,NameTrue);
}
OZ_C_proc_end

OZ_C_proc_begin(BIintToString, 2)
{
  OZ_nonvarArg(0);

  OZ_Term in = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  if (OZ_isInt(in)) {
    char *str = toC(in);
    OZ_Return ret = OZ_unify(out,OZ_string(str));
    return ret;
  }
  TypeErrorT(0,"Int");
  return FAILED;
}
OZ_C_proc_end
#endif

/* -----------------------------------
   type X
   ----------------------------------- */
#ifdef BUILTINS2

OZ_Return BIisFloatInline(TaggedRef num)
{
  DEREF(num,_,tag);

  if (isAnyVar(tag)) {
    return SUSPEND;
  }

  return isFloat(tag) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisFloat,BIisFloatInline)
DECLAREBOOLFUN1(BIisFloatB,BIisFloatBInline,BIisFloatInline)

OZ_Return BIisIntInline(TaggedRef num)
{
  DEREF(num,_,tag);

  if (isAnyVar(tag)) {
    return SUSPEND;
  }

  return isInt(tag) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisInt,BIisIntInline)
DECLAREBOOLFUN1(BIisIntB,BIisIntBInline,BIisIntInline)



OZ_Return BIisNumberInline(TaggedRef num)
{
  DEREF(num,_,tag);

  if (isAnyVar(tag)) {
    return SUSPEND;
  }

  return isNumber(tag) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisNumber,BIisNumberInline)
DECLAREBOOLFUN1(BIisNumberB,BIisNumberBInline,BIisNumberInline)


#endif


#ifdef BUILTINS1

/* -----------------------------------------------------------------------
   misc. floating point functions
   ----------------------------------------------------------------------- */


#define FLOATFUN(Fun,BIName,InlineName)                                       \
OZ_Return InlineName(TaggedRef AA, TaggedRef &out)                                    \
{                                                                             \
  DEREF(AA,_,tag);                                                            \
                                                                              \
  if (isAnyVar(tag)) {                                                        \
    return SUSPEND;                                                           \
  }                                                                           \
                                                                              \
  if (isFloat(tag)) {                                                         \
    out = makeTaggedFloat(Fun(floatValue(AA)));                       \
    return PROCEED;                                                           \
  }                                                                           \
  return FAILED;                                                              \
}                                                                             \
DECLAREBI_USEINLINEFUN1(BIName,InlineName)


FLOATFUN(exp, BIexp, BIinlineExp)
FLOATFUN(log, BIlog, BIinlineLog)
FLOATFUN(sqrt,BIsqrt,BIinlineSqrt)
FLOATFUN(sin, BIsin, BIinlineSin)
FLOATFUN(asin,BIasin,BIinlineAsin)
FLOATFUN(cos, BIcos, BIinlineCos)
FLOATFUN(acos,BIacos,BIinlineAcos)
FLOATFUN(tan, BItan, BIinlineTan)
FLOATFUN(atan,BIatan,BIinlineAtan)
FLOATFUN(ceil,BIceil,BIinlineCeil)
FLOATFUN(floor,BIfloor,BIinlineFloor)
FLOATFUN(fabs, BIfabs, BIinlineFabs)
FLOATFUN(ozround, BIround, BIinlineRound)
#undef FLOATFUN


OZ_Return BIfPowInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (isFloat(tagA) && isFloat(tagB)) {
    out = makeTaggedFloat(pow(floatValue(A),floatValue(B)));
    return PROCEED;
  }
  return suspendOnFloats(A,B);
}

OZ_Return BIatan2Inline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (isFloat(tagA) && isFloat(tagB)) {
    out = makeTaggedFloat(atan2(floatValue(A),floatValue(B)));
    return PROCEED;
  }
  return suspendOnFloats(A,B);
}


/* -----------------------------------
   EXCEPTION HANDLERS
   ----------------------------------- */

static void invalid_handler(int /* sig */,
                     int /* code */,
                     struct sigcontext* /* scp */,
                     char* /* addr */) {
  OZ_warning("signal: arithmethic exception: invalid argument");
}

static void overflow_handler(int /* sig */,
                      int /* code */,
                      struct sigcontext* /* scp */,
                      char* /* addr */) {
  OZ_warning("signal: arithmethic exception: overflow");
}

static void underflow_handler(int /* sig */,
                       int /* code */,
                       struct sigcontext* /* scp */,
                       char* /* addr */) {
  OZ_warning("signal: arithmethic exception: underflow");
}

static void divison_handler(int /* sig */,
                     int /* code */,
                     struct sigcontext* /* scp */,
                     char* /* addr */) {
  OZ_warning("signal: arithmethic exception: divison by zero");
}


/* ----------------------------------------------------------------------- *
 * limits
 * ----------------------------------------------------------------------- */

OZ_C_proc_begin(BIsmallIntLimits,2)
{
  OZ_Term minV = OZ_getCArg(0);
  OZ_Term maxV = OZ_getCArg(1);

  OZ_Return ret;
  if ((ret=OZ_unifyInt(minV,OzMinInt)) != PROCEED) {
    return ret;
  }
  if ((ret=OZ_unifyInt(maxV,OzMaxInt)) != PROCEED) {
    return ret;
  }
  return PROCEED;
}
OZ_C_proc_end

/* -----------------------------------
   make non inline versions
   ----------------------------------- */

DECLAREBI_USEINLINEREL2(BIless,BIlessInline)
DECLAREBI_USEINLINEREL2(BIle,BIleInline)
DECLAREBI_USEINLINEREL2(BIgreat,BIgreatInline)
DECLAREBI_USEINLINEREL2(BIge,BIgeInline)
DECLAREBI_USEINLINEREL2(BInumeq,BInumeqInline)
DECLAREBI_USEINLINEREL2(BInumneq,BInumneqInline)

DECLAREBI_USEINLINEFUN2(BIplus,BIplusInline)
DECLAREBI_USEINLINEFUN2(BIminus,BIminusInline)

DECLAREBI_USEINLINEFUN2(BImult,BImultInline)
DECLAREBI_USEINLINEFUN2(BIdiv,BIdivInline)
DECLAREBI_USEINLINEFUN2(BIfdiv,BIfdivInline)
DECLAREBI_USEINLINEFUN2(BImod,BImodInline)

DECLAREBI_USEINLINEFUN2(BIfPow,BIfPowInline)
DECLAREBI_USEINLINEFUN2(BIatan2,BIatan2Inline)

DECLAREBI_USEINLINEFUN2(BImax,BImaxInline)
DECLAREBI_USEINLINEFUN2(BImin,BIminInline)

DECLAREBI_USEINLINEFUN2(BIlessFun,BIlessInlineFun)
DECLAREBI_USEINLINEFUN2(BIleFun,BIleInlineFun)
DECLAREBI_USEINLINEFUN2(BIgreatFun,BIgreatInlineFun)
DECLAREBI_USEINLINEFUN2(BIgeFun,BIgeInlineFun)
DECLAREBI_USEINLINEFUN2(BInumeqFun,BInumeqInlineFun)
DECLAREBI_USEINLINEFUN2(BInumneqFun,BInumneqInlineFun)

DECLAREBI_USEINLINEFUN1(BIintToFloat,BIintToFloatInline)
DECLAREBI_USEINLINEFUN1(BIfloatToInt,BIfloatToIntInline)
DECLAREBI_USEINLINEFUN1(BIuminus,BIuminusInline)
DECLAREBI_USEINLINEFUN1(BIabs,BIabsInline)
DECLAREBI_USEINLINEFUN1(BIadd1,BIadd1Inline)
DECLAREBI_USEINLINEFUN1(BIsub1,BIsub1Inline)

#define CheckFuckingBoard(Object);                                                              \
  if (am.currentBoard != Object->getBoardFast()) {                                              \
    if (am.currentBoard->isWait()) {                                                            \
      warning("nonlocal access in computation space of disjunction not impl.");                 \
    } else {                                                                                    \
      warning("nonlocal access in computation space of %s",                                     \
              am.currentBoard->isSolve() ? "solve" : "conditional");                            \
    }                                                                                           \
    message("Built-in exchangeCell: message sending to a non locally declared object?\n");      \
    am.currentBoard->incSuspCount();                                                            \
    return PROCEED;                                                                             \
  }

/*
 * Groups
 */

OZ_C_proc_begin(BInewGroup,2)
{
  OZ_Term proc = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  DEREF(proc,procPtr,_1);
  if (isAnyVar(proc)) OZ_suspendOn(makeTaggedRef(procPtr));
  if (!isProcedure(proc)) {
    TypeErrorT(0,"Procedure");
  }

  Group *gr = new Group(am.currentThread->getGroup());
  Thread *tt = new Thread (am.currentThread->getPriority(),am.currentBoard);
  tt->setGroup(gr);
  tt->pushCall(proc,0,0);
  am.scheduleThread(tt);
  return OZ_unify(out,makeTaggedConst(gr));
}
OZ_C_proc_end

OZ_C_proc_begin(BInewGroupHdl,3)
{
  OZ_Term proc = OZ_getCArg(0);
  OZ_Term hdl = OZ_getCArg(1);
  OZ_Term out = OZ_getCArg(2);

  DEREF(proc,procPtr,_1);
  if (isAnyVar(proc)) OZ_suspendOn(makeTaggedRef(procPtr));
  if (!isProcedure(proc)) {
    TypeErrorT(0,"Procedure");
  }

  DEREF(hdl,hdlPtr,_2);
  if (isAnyVar(hdl)) OZ_suspendOn(makeTaggedRef(hdlPtr));
  if (!isProcedure(hdl)) {
    TypeErrorT(1,"Procedure");
  }

  Group *gr = new Group(am.currentThread->getGroup());
  gr->setExceptionHandler(hdl);
  Thread *tt = new Thread (am.currentThread->getPriority(),am.currentBoard);
  tt->setGroup(gr);
  tt->pushCall(proc,0,0);
  am.scheduleThread(tt);
  return OZ_unify(out,makeTaggedConst(gr));
}
OZ_C_proc_end

// ---------------------------------------------------------------------
// Cell
// ---------------------------------------------------------------------

OZ_C_proc_begin(BInewCell,2)
{
  OZ_Term val = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  return OZ_unify(out,OZ_newCell(val));
}
OZ_C_proc_end


OZ_Return BIexchangeCellInline(TaggedRef c, TaggedRef inState, TaggedRef &outState)
{
  NONVAR(c,rec,_1);
  outState = OZ_newVariable();

  if (!isCell(rec)) {
    TypeErrorT(0,"Cell");
  }
  Cell *cell = tagged2Cell(rec);

  CheckFuckingBoard(cell);

  TaggedRef old = cell->exchangeValue(outState);
  return OZ_unify(old,inState);
}

/* we do not use here DECLAREBI_USEINLINEFUN2,
 * since we only want to suspend on the first argument
 */
OZ_C_proc_begin(BIexchangeCell,3)
{
  OZ_Term help;
  OZ_Term cell = OZ_getCArg(0);
  OZ_Term inState = OZ_getCArg(1);
  OZ_Term outState = OZ_getCArg(2);
  OZ_Return state=BIexchangeCellInline(cell,inState,help);
  switch (state) {
  case SUSPEND:
    OZ_suspendOn(cell);
  case FAILED:
    return FAILED;
  case PROCEED:
  default:
    return OZ_unify(help,outState);
  }
}
OZ_C_proc_end


/*===================================================================
 * Arrays
 *=================================================================== */

OZ_C_proc_begin(BIarrayNew,4)
{
  OZ_declareIntArg(0,low);
  OZ_declareIntArg(1,high);
  OZ_Term initvalue = OZ_getCArg(2);
  OZ_Term out       = OZ_getCArg(3);

  low = deref(OZ_getCArg(0));
  high = deref(OZ_getCArg(1));

  if (!isSmallInt(low)) {
    TypeErrorT(0,"smallInteger");
  }
  if (!isSmallInt(high)) {
    TypeErrorT(1,"smallInteger");
  }

  int ilow  = smallIntValue(low);
  int ihigh = smallIntValue(high);

  return OZ_unify(makeTaggedConst(new OzArray(am.currentBoard,ilow,ihigh,initvalue)),out);
}
OZ_C_proc_end


OZ_Return isArrayInline(TaggedRef t, TaggedRef &out)
{
  NONVAR( t, term, tag );
  out = isArray(term) ? NameTrue : NameFalse;
  return PROCEED;
}

DECLAREBI_USEINLINEFUN1(BIisArray,isArrayInline)

OZ_Return arrayLowInline(TaggedRef t, TaggedRef &out)
{
  NONVAR( t, term, tag );
  if (!isArray(term)) {
    TypeErrorT(0,"Array");
  }
  out = makeInt(tagged2Array(term)->getLow());
  return PROCEED;
}
DECLAREBI_USEINLINEFUN1(BIarrayLow,arrayLowInline)

OZ_Return arrayHighInline(TaggedRef t, TaggedRef &out)
{
  NONVAR( t, term, tag );
  if (!isArray(term)) {
    TypeErrorT(0,"Array");
  }
  out = makeInt(tagged2Array(term)->getHigh());
  return PROCEED;
}

DECLAREBI_USEINLINEFUN1(BIarrayHigh,arrayHighInline)

OZ_Return arrayGetInline(TaggedRef t, TaggedRef i, TaggedRef &out)
{
  NONVAR( t, array, _1 );
  NONVAR( i, index, _2 );

  if (!isArray(array)) {
    TypeErrorT(0,"Array");
  }

  if (!isSmallInt(index)) {
    TypeErrorT(1,"smallInteger");
  }

  OzArray *ar = tagged2Array(array);
  CheckFuckingBoard(ar);
  return ar->getArg(smallIntValue(index),out);
}
DECLAREBI_USEINLINEFUN2(BIarrayGet,arrayGetInline)

OZ_Return arrayPutInline(TaggedRef t, TaggedRef i, TaggedRef value)
{
  NONVAR( t, array, _1 );
  NONVAR( i, index, _2 );

  if (!isArray(array)) {
    TypeErrorT(0,"Array");
  }

  if (!isSmallInt(index)) {
    TypeErrorT(1,"smallInteger");
  }

  OzArray *ar = tagged2Array(array);
  CheckFuckingBoard(ar);
  return ar->setArg(smallIntValue(index),value);
}

DECLAREBI_USEINLINEREL3(BIarrayPut,arrayPutInline)


/*===================================================================
 *   Dictionaries
 *=================================================================== */

OZ_C_proc_begin(BIdictionaryNew,1)
{
  OZ_Term out       = OZ_getCArg(0);

  return OZ_unify(makeTaggedConst(new OzDictionary(am.currentBoard)),out);
}
OZ_C_proc_end


OZ_C_proc_begin(BIdictionaryKeys,2)
{
  OZ_Term d   = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  NONVAR( d, dict, _1 );
  if (!isDictionary(dict)) {
    TypeErrorT(0,"Dictionary");
  }

  CheckFuckingBoard(tagged2Dictionary(dict));
  return OZ_unify(tagged2Dictionary(dict)->keys(),out);
}
OZ_C_proc_end


OZ_Return isDictionaryInline(TaggedRef t, TaggedRef &out)
{
  NONVAR( t, term, tag );
  out = isDictionary(term) ? NameTrue : NameFalse;
  return PROCEED;
}
DECLAREBI_USEINLINEFUN1(BIisDictionary,isDictionaryInline)


#define GetDictAndKey(d,k,dict,key)                             \
  NONVAR(d,dictaux,_1);                                         \
  NONVAR(k,key, _2);                                            \
  if (!isDictionary(dictaux)) { TypeErrorT(0,"Dictionary"); }   \
  if (!isFeature(key))        { TypeErrorT(1,"feature"); }      \
  OzDictionary *dict = tagged2Dictionary(dictaux);              \
  CheckFuckingBoard(dict);


OZ_Return dictionaryMemberInline(TaggedRef d, TaggedRef k, TaggedRef &out)
{
  GetDictAndKey(d,k,dict,key);
  out = dict->member(key);
  return PROCEED;
}
DECLAREBI_USEINLINEFUN2(BIdictionaryMember,dictionaryMemberInline)


OZ_Return dictionaryGetInline(TaggedRef d, TaggedRef k, TaggedRef &out)
{
  GetDictAndKey(d,k,dict,key);
  return dict->getArg(key,out);
}
DECLAREBI_USEINLINEFUN2(BIdictionaryGet,dictionaryGetInline)


OZ_Return dictionaryGetIfInline(TaggedRef d, TaggedRef k, TaggedRef deflt, TaggedRef &out)
{
  GetDictAndKey(d,k,dict,key);
  if (dict->getArg(key,out)!=PROCEED) {
    out = deflt;
  }
  return PROCEED;
}
DECLAREBI_USEINLINEFUN3(BIdictionaryGetIf,dictionaryGetIfInline)


OZ_Return dictionaryPutInline(TaggedRef d, TaggedRef k, TaggedRef value)
{
  GetDictAndKey(d,k,dict,key);
  dict->setArg(key,value);
  return PROCEED;
}

DECLAREBI_USEINLINEREL3(BIdictionaryPut,dictionaryPutInline)


OZ_Return dictionaryRemoveInline(TaggedRef d, TaggedRef k)
{
  GetDictAndKey(d,k,dict,key);
  dict->remove(key);
  return PROCEED;
}
DECLAREBI_USEINLINEREL2(BIdictionaryRemove,dictionaryRemoveInline)


#endif /* BUILTINS1 */

/* -----------------------------------------------------------------
   dynamic link objects files
   ----------------------------------------------------------------- */

#ifdef BUILTINS2

#ifdef WINDOWS
#define PATHSEP ';'
#else
#define PATHSEP ':'
#endif

char *expandFileName(char *fileName,char *path) {

  char *ret;

  if (*fileName == '/') {
    if (access(fileName,F_OK) == 0) {
      ret = new char[strlen(fileName)+1];
      strcpy(ret,fileName);
      return ret;
    }
    return NULL;
  }

  if (!path) {
    return NULL;
  }

  char *tmp = new char[strlen(path)+strlen(fileName)+2];
  char *last = tmp;
  while (1) {
    if (*path == PATHSEP || *path == 0) {

      if (last == tmp && *path == 0) { // path empty ??
        return NULL;
      }

      *last++ = '/';
      strcpy(last,fileName);
      if (access(tmp,F_OK) == 0) {
        ret = new char[strlen(tmp)+1];
        strcpy(ret,tmp);
        delete [] tmp;
        return ret;
      }
      last = tmp;
      if (*path == 0) {
        return NULL;
      }
      path++;
    } else {
      *last++ = *path++;
    }
  }

}


static void strCat(char toStr[], int &toStringSize, char *fromString)
{
  int i = toStringSize;
  while (*fromString)  {
    toStr[i++] = *fromString;
    fromString++;
  }
  toStr[i] = '\0';
  toStringSize = i;
}


/* arrayFromList(list,array,size):
 *       "list" is a Oz list of atoms.
 *     "array" is an array containing those atoms of max. size "size"
 *     returns NULL on failure
 */

char **arrayFromList(OZ_Term list, char **array, int size)
{
  int i=0;

  while(OZ_isCons(list)) {

    if (i >= size-1) {
      OZ_warning("linkObjectFiles: too many arguments");
      goto bomb;
    }

    OZ_Term hh = OZ_head(list);
    if (!OZ_isAtom(hh)) {
      OZ_warning("linkObjectFiles: List with atoms expected");
      goto bomb;
    }
    char *fileName = OZ_atomToC(hh);

    char *f = expandFileName(fileName,ozconf.linkPath);

    if (!f) {
      OZ_warning("linkObjectFiles(%s): expand filename failed",fileName);
      goto bomb;
    }

    array[i++] = f;
    list = OZ_tail(list);
  }

  array[i] = NULL;
  return array;

 bomb:
  while (i>=0) {
    char *f = array[i--];
    delete [] f;
  }
  return NULL;
}

#ifdef DLD
/* dld initialization eats up approx 2MB of main memory, so
 * we dealy it, until it is realy neede
 */
static char *executable = NULL;

void dld_doinit()
{
  static Bool inited = NO;
  if (!inited) {
    if (dld_init(dld_find_executable(executable)) != 0) {
      dld_perror("Failed to initialize DLD");
      return;
    }
    inited = OK;
  }
}
#endif

void DLinit(char *nm) {
#ifdef DLD
  executable = nm;
#endif
}


/* linkObjectFiles(+filelist,-handle)
 *    filelist: list of atoms (== object files)
 *    handle:   for future calls of findFunction
 * we do not check for well-typedness, do it in Oz !!
 */

OZ_C_proc_begin(BIlinkObjectFiles,2)
{
  OZ_Term list = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

#ifdef DLD
  dld_doinit();
  while(OZ_isCons(list)) {

    OZ_Term hh = OZ_head(list);
    if (!OZ_isAtom(hh)) {
      TypeErrorT(0,"List of Atoms");
    }
    char *fileName = OZ_atomToC(hh);
    char *f = expandFileName(fileName,ozconf.linkPath);

    if (!f) {
      OZ_warning("linkObjectFiles(%s): expand filename failed",fileName);
      return FAILED;
    }

    if (ozconf.showForeignLoad) {
      message("Linking file '%s'\n",f);
    }
    if (dld_link(f) != 0) {
      OZ_warning("linkObjectFiles(%s): failed for %s",fileName,f);
      dld_perror("linkObjectFiles");
      delete [] f;
      return FAILED;
    }
    delete [] f;
    list = OZ_tail(list);
  }
  dld_link("/usr/lib/libc.a");   /* check for unresolved references */
  return OZ_unifyInt(out,0);  /* no handle needed */
#elif defined(DLOPEN) || defined(HPUX_700)
  const int commandSize = 1000;
  int commandUsed = 0;
  char command[commandSize];
  char *tempfile = tmpnam(NULL);
#ifdef SOLARIS_SPARC
  strCat(command, commandUsed, "ld -G -lsocket -lnsl -lintl -o ");
#endif
#ifdef SUNOS_SPARC
  strCat(command, commandUsed, "ld -o ");
#endif
#ifdef IRIX5_MIPS
  strCat(command, commandUsed, "ld -shared -check_registry /dev/null -o ");
#endif
#ifdef OSF1_ALPHA
  strCat(command, commandUsed, "ld -shared -expect_unresolved \\* -o ");
#endif
#ifdef HPUX_700
  strCat(command, commandUsed, "ld -b -o ");
#endif
#ifdef LINUX_I486
  strCat(command, commandUsed, "ld -shared -o ");
#endif
  strCat(command, commandUsed, tempfile);

  const numOfiles = 100;
  char *ofiles[numOfiles];
  if (arrayFromList(list,ofiles,numOfiles) == NULL) {
    unlink(tempfile);
    return FAILED;
  }

  for (int i=0; ofiles[i] != NULL; i++) {
    char *f = ofiles[i];
    if (commandUsed + strlen(f) >= commandSize-1) {
      OZ_warning("linkObjectFiles: too many arguments");
      unlink(tempfile);
      delete [] f;
      return FAILED;
    }
    strCat(command, commandUsed, " ");
    strCat(command, commandUsed, f);
    delete [] f;
  }

  if (ozconf.showForeignLoad) {
    message("Linking files\n %s\n",command);
  }

  if (osSystem(command) < 0) {
    char buf[1000];
    sprintf(buf,"'%s' failed in linkObjectFiles",command);
    ozpwarning(buf);
    unlink(tempfile);
    return FAILED;
  }

#ifdef HPUX_700
  shl_t handle;
  handle = shl_load(tempfile,
                    BIND_IMMEDIATE | BIND_NONFATAL | BIND_NOSTART | BIND_VERBOSE, 0L);

  if (handle == NULL) {
    return FAILED;
  }
#else
  void *handle;

  if (!(handle = (void *)dlopen(tempfile, RTLD_NOW ))) {
    OZ_warning("dlopen failed in linkObjectFiles: %s",dlerror());
    unlink(tempfile);
    return FAILED;
  }
#endif

  unlink(tempfile);
  return OZ_unifyInt(out,ToInt32(handle));

#elif defined(WINDOWS)

  const numOfiles = 100;
  char *ofiles[numOfiles];
  if (arrayFromList(list,ofiles,numOfiles) == NULL ||
      ofiles[0]==NULL ||
      ofiles[1]!=NULL) {
    OZ_warning("linkObjectFiles(%s): can only accept one DLL\n",toC(list));
    return FAILED;
  }

  if (ozconf.showForeignLoad) {
    message("Linking files\n %s\n",ofiles[0]);
  }

  void *handle = (void *)LoadLibrary(ofiles[0]);
  if (handle==NULL) {
    OZ_warning("failed in linkObjectFiles: %d",GetLastError());
    return FAILED;
  }
  return OZ_unifyInt(out,ToInt32(handle));

#endif
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIunlinkObjectFile,1)
{
  OZ_declareAtomArg(0,fileName);

  char *f = expandFileName(fileName,ozconf.linkPath);

#ifdef DLD
  dld_doinit();

  if (!f) {
    OZ_warning("unlinkObjectFile(%s): expand filename failed",fileName);
    return FAILED;
  }
  if (dld_unlink_by_file(f,0) != 0) {
    delete [] f;
    OZ_warning("unlinkObjectFile(%s): failed for %s",fileName,f);
    return FAILED;
  }
  delete [] f;
#endif

#ifdef WINDOWS
  FreeLibrary(GetModuleHandle(f));
#endif

  return PROCEED;
}
OZ_C_proc_end

#ifdef DLD
#define Link(handle,name) dld_get_func(name)
#elif defined(DLOPEN)
#define Link(handle,name) dlsym(handle,name)
#elif defined(HPUX_700)
void *ozdlsym(void *handle,char *name)
{
  void *symaddr;

  int symbol = shl_findsym((shl_t*)&handle, name, TYPE_PROCEDURE, &symaddr);
  if (symbol != 0) {
    return NULL;
  }

  return symaddr;
}

#define Link(handle,name) ozdlsym(handle,name)

#elif defined(WINDOWS)

FARPROC winLink(HMODULE handle, char *name)
{
  FARPROC ret = GetProcAddress(handle,name);
  if (ret == NULL) {
    // try prepending a "_"
    char buf[1000];
    sprintf(buf,"_%s",name);
    ret = GetProcAddress(handle,buf);
  }
  return ret;
}

#define Link(handle,name) winLink(handle,name)

#endif

/* findFunction(+fname,+farity,+handle) */
OZ_C_proc_begin(BIfindFunction,3)
{
#ifdef DLD
    dld_doinit();
#endif
#if defined(DLD) || defined(DLOPEN) || defined(HPUX_700) || defined(WINDOWS)
  OZ_declareAtomArg(0,functionName);
  OZ_declareIntArg(1,functionArity);
  OZ_declareIntArg(2,handle);

  // get the function
  OZ_CFun func;
  if ((func = (OZ_CFun) Link((void *) handle,functionName)) == 0) {
    OZ_warning("findFunction(%s,%s,%s): not found",
               toC(OZ_getCArg(0)),
               toC(OZ_getCArg(1)),
               toC(OZ_getCArg(2))
               );
#ifdef DLD
    dld_perror("findFunction");
#endif
#ifdef WINDOWS
    OZ_warning("error=%d\n",GetLastError());
#endif
    return FAILED;
  }

#ifdef DLD
  // check whether it contains unresolved references:
  if (dld_function_executable_p(functionName) == 0) {
    OZ_warning("findFunction(%s,%s): unresolved references:",
               toC(OZ_getCArg(0)),
               toC(OZ_getCArg(1))
               );
    char ** unresolved = dld_list_undefined_sym();
    for (int i=0; i< dld_undefined_sym_count; i++)
      OZ_warning("findFunction: %s",unresolved[i]);
    return FAILED;
  }
#endif

  OZ_addBuiltin(functionName,functionArity,*func);

#endif

  return PROCEED;
}
OZ_C_proc_end


// ---------------------------------------------------------------------------

OZ_C_proc_begin(BIshutdown,0)
{
  am.exitOz(0);
  return(PROCEED); /* not reached but anyway */
}
OZ_C_proc_end

// -------------- SLEEP -------------------------------

OZ_C_proc_begin(BIsleep,3)
{
  OZ_declareIntArg(0,t);
  OZ_Term l=OZ_getCArg(1);
  OZ_Term r=OZ_getCArg(2);
  if (t <= 0) return OZ_unify(l,r);

  if (!am.isToplevel()) {
    OZ_warning("Built-in sleep: only on toplevel");
    return FAILED;
  }

  am.insertUser(t,cons(l,r));
  return PROCEED;
}
OZ_C_proc_end


// ---------------------------------------------------------------------------

OZ_C_proc_begin(BIgarbageCollection,0)
{
  am.setSFlag(StartGC);
  return PROCEED;
}
OZ_C_proc_end

// ---------------------------------------------------------------------------

OZ_C_proc_begin(BIsystemEq,2)
{
  return termEq(OZ_getCArg(0),OZ_getCArg(1)) ? PROCEED : FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIsystemEqB,3)
{
  OZ_Term ret = termEq(OZ_getCArg(0),OZ_getCArg(1)) ? NameTrue : NameFalse;
  return OZ_unify(OZ_getCArg(2),ret);
}
OZ_C_proc_end


OZ_C_proc_begin(BIunify,2)
{
  OZ_Term t0 = OZ_getCArg(0);
  OZ_Term t1 = OZ_getCArg(1);

  return (OZ_unify(t0,t1));
}
OZ_C_proc_end

OZ_C_proc_begin(BIfail,VarArity)
{
  return FAILED;
}
OZ_C_proc_end

// ------------------------------------------------------------------------
// --- Call: Builtin: Load File
// ------------------------------------------------------------------------

OZ_C_proc_begin(BIloadFile,1)
{
  TaggedRef term0 = OZ_getCArg(0);
  DEREF(term0,_0,tag0);

  if (!isAtom(term0)) {
    TypeErrorT(0,"Atom");
  }

  char *file = tagged2Literal(term0)->getPrintName();

  CompStream *fd = CompStream::csopen(file);

  if (fd == NULL) {
    OZ_warning("call: loadFile: cannot open file '%s'",file);
    return FAILED;
  }

  if (ozconf.showFastLoad) {
    printf("Fast loading file '%s'\n",file);
  }

  // begin critical region
  osBlockSignals();

  am.loadQuery(fd);

  fd->csclose();

  osUnblockSignals();
  // end critical region

  return PROCEED;
}
OZ_C_proc_end


// ------------------------------------------------------------------------
// --- Apply
// ------------------------------------------------------------------------

OZ_C_proc_begin(BIapply,2)
{
  OZ_Term proc = OZ_getCArg(0);
  OZ_Term args = OZ_getCArg(1);

  OZ_Term var;
  if (!OZ_isList(args,&var)) {
    if (var == 0) TypeErrorT(1,"List");
    OZ_suspendOn(var);
  }

  int len = OZ_length(args);
  RefsArray argsArray = allocateY(len);
  for (int i=0; i < len; i++) {
    argsArray[i] = OZ_head(args);
    args=OZ_tail(args);
  }
  Assert(OZ_isNil(args));

  DEREF(proc,procPtr,_2);
  if (isAnyVar(proc)) OZ_suspendOn(makeTaggedRef(procPtr));
  if (!isProcedure(proc) && !isObject(proc)) {
    TypeErrorT(0,"Procedure or Object");
  }

  am.pushCall(proc,len,argsArray);
  deallocateY(argsArray);
  return PROCEED;
}
OZ_C_proc_end

// ------------------------------------------------------------------------
// --- special Cell access
// ------------------------------------------------------------------------

OZ_C_proc_begin(BIdeepFeed,2)
{
  OZ_Term c = OZ_getCArg(0);
  OZ_Term val = OZ_getCArg(1);

  NONVAR(c,rec,_1);
  if (!isCell(rec)) {
    TypeErrorT(0,"Cell");
  }

  Cell *cell1 = tagged2Cell(rec);

  Board *savedNode = am.currentBoard;
  Board *home1 = cell1->getBoardFast();

  switch (am.installPath(home1)) {
  case INST_FAILED:
  case INST_REJECTED:
    error("deep: install");
  case INST_OK:
    break;
  }

  TaggedRef newVar = OZ_newVariable();
  TaggedRef old = cell1->exchangeValue(newVar);
  OZ_Return ret = OZ_unify(old,cons(val,newVar));

  switch (am.installPath(savedNode)) {
  case INST_FAILED:
  case INST_REJECTED:
    error("deep: install back");
  case INST_OK:
    break;
  }

  return ret;
}
OZ_C_proc_end


OZ_C_proc_begin(BIdeepReadCell,2)
{
  OZ_Term c = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);
  NONVAR(c,rec,_1);
  if (!isCell(rec)) {
    TypeErrorT(0,"Cell");
  }
  Cell *cell1 = tagged2Cell(rec);
  return OZ_unify(out,cell1->getValue());
}
OZ_C_proc_end


/* ---------------------------------------------------------------------
 * Destructive dot
 *   this is used in the array/dictionary objects
 *
 * NOTE: similar functions are dot, genericSet, uparrow, subtree, hasSubtreeAt
 * ---------------------------------------------------------------------*/
OZ_C_proc_begin(BIgenericSet, 3)
{
  OZ_Term term = OZ_getCArg(0);
  OZ_Term fea = OZ_getCArg(1);
  OZ_Term val = OZ_getCArg(2);

  DEREF(term, _2, termTag);
  DEREF(fea, _1,feaTag);

  if (isAnyVar(feaTag)) {
    switch (termTag) {
    case LTUPLE:
    case SRECORD:
    case SVAR:
    case UVAR:
      return SUSPEND;
    case CVAR:
      if (tagged2CVar(term)->getType()!=OFSVariable) return FAILED;
      return SUSPEND;
    default:
      return FAILED;
    }
  }

  switch (termTag) {
  case LTUPLE:
    {
      if (!isSmallInt(fea)) {
        return FAILED;
      }
      int i2 = smallIntValue(fea);

      switch (i2) {
      case 1:
        tagged2LTuple(term)->setHead(val);
        return PROCEED;
      case 2:
        tagged2LTuple(term)->setTail(val);
        return PROCEED;
      }
      return FAILED;
    }

  case SRECORD:
    {
      if ( ! isFeature(feaTag) ) {
        return FAILED;
      }
      SRecord *rec = tagged2SRecord(term);
      if (rec->setFeature(fea, val) ) {
        return PROCEED;
      }
      return FAILED;
    }

  case UVAR:
  case SVAR:
    if (!isFeature(feaTag)) {
      return FAILED;
    }
    return SUSPEND;

  case CVAR:
    {
      if (!isFeature(feaTag)) {
        return FAILED;
      }
      if (tagged2CVar(term)->getType()!=OFSVariable) return FAILED;
      return SUSPEND;
    }

  default:
    return FAILED;
  }
}
OZ_C_proc_end

// ---------------------------------------------------------------------------

/* fun {matchDefault Term Attr Defau}
 *    if X in Term.Attr = X then X else Defau fi
 * end
 */

extern OZ_Return subtreeInline(TaggedRef term, TaggedRef fea, TaggedRef &out);

OZ_Return matchDefaultInline(TaggedRef term, TaggedRef attr, TaggedRef defau,
                         TaggedRef &out)
{
  switch(subtreeInline(term,attr,out)) {
  case PROCEED:
    return PROCEED;
  case SUSPEND:
    return SUSPEND;
  case FAILED:
  default:
    out = defau;
    return PROCEED;
  }
}

DECLAREBI_USEINLINEFUN3(BImatchDefault,matchDefaultInline)

// ---------------------------------------------------------------------------

OZ_C_proc_begin(BIatomHash, 3)
{
  OZ_Term atom = OZ_getCArg(0);
  if (OZ_isVariable(atom)) {
      OZ_suspendOn(atom);
  }

  OZ_Term modulo = OZ_getCArg(1);
  if (OZ_isVariable(modulo)) {
    OZ_suspendOn(modulo);
  }

  OZ_Term ret = OZ_getCArg(2);

  if (!OZ_isAtom(atom) || !OZ_isInt(modulo)) {
    TypeErrorM("Arguments must be Atom and (small) Int");
  }
  char *nm = OZ_atomToC(atom);
  int mod = OZ_intToC(modulo);

  // 'hashfunc' is taken from 'Aho,Sethi,Ullman: Compilers ...', page 436
  unsigned h = 0;
  {
    unsigned g;
    for(; *nm; nm++) {
      h = (h << 4) + (*nm);
      if ((g = h & 0xf0000000)) {
        h = h ^ (g >> 24);
        h = h ^ g;
      }
    }
  }
  return OZ_unifyInt(ret,h % mod + 1);
}
OZ_C_proc_end

// ---------------------------------------------------------------------------

static int genCount = 0;

OZ_C_proc_begin(BIgensym,2)
{
  OZ_declareAtomArg(0,str);
  OZ_Term out = OZ_getCArg(1);
  char *s = new char[strlen(str) + 20];
  sprintf(s,"%s%04d",str,genCount++);

  OZ_Return ret = OZ_unifyAtom(out,s);
  delete [] s;
  return ret;
}
OZ_C_proc_end


/*
 * some special builtins for browser:
 *  getsBound
 *  intToAtom
 */

OZ_C_proc_begin(_getsBound_dummy, 0)
{
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIgetsBound, 1)
{
  OZ_Term v = OZ_getCArg(0);

  DEREF(v, vPtr, vTag);

  if (isAnyVar(vTag)){
    Thread *thr =
      (Thread *) OZ_makeSuspendedThread (_getsBound_dummy, NULL, 0);
    SuspList *vcsl = new SuspList(thr, NULL);
    addSuspAnyVar(vPtr, vcsl);
  }

  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(_getsBound_dummyB, 2)
{
  return (OZ_unify (OZ_getCArg (1), NameTrue));
}
OZ_C_proc_end


OZ_C_proc_begin(BIgetsBoundB, 2)
{
  OZ_Term v = OZ_getCArg(0);

  DEREF(v, vPtr, vTag);

  if (isAnyVar(vTag)){
    Thread *thr =
      (Thread *) OZ_makeSuspendedThread (_getsBound_dummyB, OZ_args, OZ_arity);
    SuspList *vcsl = new SuspList (thr, NULL);
    addSuspAnyVar(vPtr, vcsl);
  }

  return (PROCEED);             // no result yet;
}
OZ_C_proc_end

OZ_C_proc_begin(BIintToAtom, 2)
{
  OZ_Term inP = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  NONVAR(inP,in,_1);

  if (OZ_isInt(in)) {
    char *str = toC(in);
    OZ_Return ret = OZ_unifyAtom(out,str);
    return ret;
  }
  TypeErrorT(0,"Int");
}
OZ_C_proc_end

OZ_C_proc_begin(BIconstraints,2)
{
  OZ_Term in = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  DEREF(in,inPtr,inTag);
  int len = 0;
  if (isCVar(inTag)) {
    len=tagged2CVar(in)->getSuspListLength();
  } else if (isSVar(inTag)) {
    len = tagged2SVar(in)->getSuspList()->length();
  }
  return OZ_unifyInt(out,len);
}
OZ_C_proc_end

// ---------------------------------------------------------------------------
// linguistic reflection
// ---------------------------------------------------------------------------

OZ_C_proc_begin(BIconnectLingRef,1)
{
  OZ_warning("connectLingRef no longer needed");
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIgetLingRefFd,1)
{
  return OZ_unifyInt(OZ_getCArg(0),am.compStream->getLingRefFd());
}
OZ_C_proc_end


OZ_C_proc_begin(BIgetLingEof,1)
{
  return OZ_unifyInt(OZ_getCArg(0),am.compStream->getLingEOF());
}
OZ_C_proc_end


// ---------------------------------------------------------------------------
// abstraction table
// ---------------------------------------------------------------------------

OZ_C_proc_begin(BIsetAbstractionTabDefaultEntry,1)
{
  TaggedRef in = OZ_getCArg(0); DEREF(in,_1,_2);
  if (!isAbstraction(in)) {
    warning("setAbstractionTabDefaultEntry: abstraction expected: %s",toC(in));
    return FAILED;
  }

  AbstractionEntry::setDefaultEntry(tagged2Abstraction(in));
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIprintError,1)
{
  prefixError();
  OZ_Term args0 = OZ_getCArg(0);
  taggedPrint(args0,ozconf.printDepth);
  fflush(stdout);

  return (PROCEED);
}
OZ_C_proc_end

/* print and show are inline,
   because it prevents the compiler from generating different code
   */
OZ_Return printInline(TaggedRef term)
{
  char *s=OZ_toC(term,ozconf.printDepth,ozconf.printWidth);
  fprintf(stdout,"%s",s);
  fflush(stdout);
  return (PROCEED);
}

DECLAREBI_USEINLINEREL1(BIprint,printInline)

OZ_C_proc_begin(BIprintVS,1)
{
  OZ_Term t=OZ_getCArg(0);
  OZ_printVirtualString(t);
  fflush(stdout);
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BItermToVS,2)
{
  OZ_Term t=OZ_getCArg(0);
  OZ_Term out=OZ_getCArg(1);
  return OZ_unify(out,
                  OZ_string(OZ_toC(t,ozconf.printDepth,ozconf.printWidth)));
}
OZ_C_proc_end

OZ_C_proc_begin(BIgetTermSize,4)
{
  int depth, width;
  OZ_Term t = OZ_getCArg (0);
  OZ_Term dt = OZ_getCArg (1);
  OZ_Term wt = OZ_getCArg (2);
  OZ_Term out = OZ_getCArg (3);

  DEREF(dt, _1, dtTag);
  DEREF(wt, _2, wtTag);

  if (isAnyVar(dtTag)) {
    return (SUSPEND);
  } else if (!isSmallInt(dt)) {
    TypeErrorT(1,"(valid) Int)");
    TypeErrorT(1,"Int");
  } else {
    depth = smallIntValue(dt);
  }

  if (isAnyVar(wtTag)) {
    return (SUSPEND);
  } else if (!isSmallInt(wt)) {
    TypeErrorT(2,"(valid) Int)");
    TypeErrorT(2,"Int");
  } else {
    width = smallIntValue(wt);
  }

  return OZ_unify(out, OZ_int(OZ_termGetSize(t, depth, width)));
}
OZ_C_proc_end

OZ_Return showInline(TaggedRef term)
{
  printInline(term);
  printf("\n");
  return (PROCEED);
}

DECLAREBI_USEINLINEREL1(BIshow,showInline)

// ---------------------------------------------------------------------------
// GETVALUE
// ---------------------------------------------------------------------------

#define STRCASE(STR,VAL,UNIFYPROC) \
     if (strcmp(feature,STR) == 0) { \
       return UNIFYPROC (out,(VAL)); \
     }

TaggedRef Abstraction::DBGgetGlobals() {
  int n = getGSize();
  OZ_Term t = OZ_tuple(OZ_atom("globals"),n);
  for (int i = 0; i < n; i++) {
    OZ_putArg(t,i,gRegs[i]);
  }
  return t;
}

inline
OZ_Term NULL2NIL(OZ_Term t)
{
  return t==makeTaggedNULL() ? OZ_nil() : t;
}

int taggedGetValue(TaggedRef term,TaggedRef feat,TaggedRef out)
{
  DEREF(term,termPtr,tag);
  DEREF(feat,_1,fTag);
  if (!OZ_isAtom(feat)) {
    return NO;
  }

  char *feature = tagged2Literal(feat)->getPrintName();

  STRCASE("addr",
          isAnyVar(tag) ? ToInt32(termPtr) : ToInt32(tagValueOf(tag,term)),
          OZ_unifyInt);
  STRCASE("isvar",
          isAnyVar(tag) ? "ok" : "no",
          OZ_unifyAtom);
  STRCASE("lengthSuspList",
          isCVar(tag) ? tagged2CVar(term)->getSuspListLength() :
          (isSVar(tag) ? tagged2SVar(term)->getSuspList()->length()
           : (isUVar(tag) ? 0 : -1)),
          OZ_unifyInt);

  STRCASE("name",
          tagged2String(term,ozconf.printDepth),
          OZ_unifyAtom);

  switch (tag) {
  case SRECORD:
  case LITERAL:
    break;
  case UVAR:
    STRCASE("home",ToInt32(tagged2VarHome(term)->getBoardFast()),OZ_unifyInt);
    STRCASE("suspensions",nil(),OZ_unify);
    break;
  case SVAR:
  case CVAR:
    STRCASE("home",ToInt32(tagged2SuspVar(term)->getBoardFast()),OZ_unifyInt);
    STRCASE("suspensions",tagged2SuspVar(term)->DBGmakeSuspList(),OZ_unify);
    break;

  case OZCONST:
  default:
    if (isConst(term)) {
      ConstTerm *rec = tagged2Const(term);
      switch(rec->getType()) {
      case Co_Builtin:
        STRCASE("arity",((Builtin *) rec)->getArity(),OZ_unifyInt);
        STRCASE("handler",
                NULL2NIL(((Builtin *) rec)->getSuspHandler()),
                OZ_unify);
        break;

      case Co_Object:
        rec = ((Object *) rec)->getAbstraction();
        // no break here!
      case Co_Abstraction:
        STRCASE("arity",((Abstraction *) rec)->getArity(),OZ_unifyInt);
        STRCASE("globals",
                ((Abstraction *) rec)->DBGgetGlobals(),
                OZ_unify);
        break;
      case Co_Cell:
        STRCASE("home",ToInt32(((Cell *) rec)->getBoardFast()),OZ_unifyInt);
        break;
      default:
        break;
      }
    }
    break;
  }

  warning("getValue(%s,%s): bad feature",
          toC(term),toC(feat));
  return NO;
}

OZ_C_proc_begin(BIgetArgv,1)
{
  TaggedRef out = nil();
  for(int i=ozconf.argC-1; i>=0; i--) {
    out = cons(OZ_atom(ozconf.argV[i]),out);
  }

  return OZ_unify(OZ_getCArg(0),out);
}
OZ_C_proc_end


OZ_C_proc_begin(BIgetPrintName,2)
{
  OZ_Term term = OZ_getCArg(0);
  OZ_Term out  = OZ_getCArg(1);

  DEREF(term,termPtr,tag);

  switch (tag) {
  case OZCONST:
    if (isConst(term)) {
      ConstTerm *rec = tagged2Const(term);
      switch (rec->getType()) {
      case Co_Builtin:      return OZ_unify(out, ((Builtin *) rec)->getName());
      case Co_Abstraction:  return OZ_unify(out, ((Abstraction *) rec)->getName());
      case Co_Object:       return OZ_unifyAtom(out, ((Object*) rec)->getPrintName());

      case Co_Cell:
      case Co_Dictionary:
      case Co_Array:
      default:             return OZ_unifyAtom(out,"_");
      }
    }
    break;

  case UVAR:    return OZ_unifyAtom(out, "_");
  case SVAR:
  case CVAR:    return OZ_unifyAtom(out, VariableNamer::getName(OZ_getCArg(0)));
  case LITERAL: return OZ_unifyAtom(out, tagged2Literal(term)->getPrintName());

  default:      break;
  }

  return OZ_unifyAtom(out, tagged2String(term,ozconf.printDepth));
}
OZ_C_proc_end

TaggedRef SVariable::DBGmakeSuspList()
{
  return suspList->DBGmakeList();
}

TaggedRef SuspList::DBGmakeList() {
  if (this == NULL) {
    return nil();
  }

  Thread *t = getElem();
  Board *b = t->getBoardFast ();
  return cons(makeTaggedConst(b),getNext()->DBGmakeList());
}

int AM::getValue(TaggedRef feat, TaggedRef out)
{
  DEREF(feat,_1,fTag);
  if (!OZ_isAtom(feat)) {
    return NO;
  }

  char *feature = tagged2Literal(feat)->getPrintName();

  STRCASE("userTime",       (int) osUserTime(),               OZ_unifyInt);
  STRCASE("currentHeapSize",getUsedMemory()*KB,               OZ_unifyInt);
  STRCASE("freeListSize",   getMemoryInFreeList(),            OZ_unifyInt);
  STRCASE("debugmode",      isSetSFlag(DebugMode) ? 1 : 0,    OZ_unifyInt);
  STRCASE("fastload",       ozconf.showFastLoad,                OZ_unifyInt);
  STRCASE("foreignload",    ozconf.showForeignLoad,             OZ_unifyInt);
  STRCASE("idleMessage",    ozconf.showIdleMessage,             OZ_unifyInt);
  STRCASE("showSuspension", ozconf.showSuspension,              OZ_unifyInt);
  STRCASE("stopOnToplevelFailure",ozconf.stopOnToplevelFailure, OZ_unifyInt);
  STRCASE("isvar",          "no",OZ_unifyAtom);

  STRCASE("cellHack",          ozconf.cellHack,                 OZ_unifyInt);
  STRCASE("gcFlag",            ozconf.gcFlag,                   OZ_unifyInt);
  STRCASE("gcVerbosity",       ozconf.gcVerbosity,              OZ_unifyInt);
  STRCASE("heapMaxSize",       ozconf.heapMaxSize,              OZ_unifyInt);
  STRCASE("heapThreshold",     ozconf.heapThreshold,            OZ_unifyInt);
  STRCASE("heapMargin",        ozconf.heapMargin,               OZ_unifyInt);
  STRCASE("heapIncrement",     ozconf.heapIncrement,            OZ_unifyInt);
  STRCASE("heapIdleMargin",    ozconf.heapIdleMargin,           OZ_unifyInt);

  STRCASE("clockTick",         CLOCK_TICK,                    OZ_unifyInt);
  STRCASE("printDepth",        ozconf.printDepth,               OZ_unifyInt);
  STRCASE("statusReg",         (int)statusReg,                OZ_unifyInt);
  STRCASE("root",              makeTaggedConst(rootBoard),    OZ_unify);
  STRCASE("currentBlackboard", makeTaggedConst(currentBoard), OZ_unify);
  STRCASE("queryFILE",         compStream->csfileno(),     OZ_unifyInt);
  STRCASE("errorVerbosity",    ozconf.errorVerbosity,           OZ_unifyInt);

  warning("getValue(0,%s): bad feature", toC(feat));
  return NO;
}


OZ_C_proc_begin(BIgetValue,3)
{
  OZ_Term term = OZ_getCArg(0);
  OZ_Term fea = OZ_getCArg(1);
  OZ_Term out = OZ_getCArg(2);

  OZ_nonvarArg(1);
  NONVAR(fea,feat,ftag);

  if (!isLiteral(ftag)) {
    TypeErrorT(1,"Feature");
  }

  char *feature = tagged2Literal(feat)->getPrintName();

  if (OZ_isInt(term)) {  // AM
    DEREF(term,_1,_2);
    STRCASE("addr",ToInt32(tagValueOf(term)),OZ_unifyInt);
    STRCASE("name", tagged2String(term,ozconf.printDepth),
            OZ_unifyAtom);
    STRCASE("printname", tagged2String(term,ozconf.printDepth),
            OZ_unifyAtom);
    return am.getValue(feat,out) ? PROCEED : FAILED;
  }

  return taggedGetValue(term,feat,out) ? PROCEED: FAILED;
}
OZ_C_proc_end

#undef STRCASE

// --------------------------------------------------------------------------
// SETVALUE
// --------------------------------------------------------------------------

#define DOIF(str,body) \
     if (feature == OZ_atom(str)) { \
       body \
       return OK; \
     }

int AM::setValue(TaggedRef feature, TaggedRef value)
{
  if (!OZ_isInt(value)) {
    return NO;
  }
  int val = OZ_intToC(value);

  DOIF("debugmode",
       val ? setSFlag(DebugMode) : unsetSFlag(DebugMode);
       );
  DOIF("foreignload",
       ozconf.showForeignLoad = val ? OK : NO;
       );
  DOIF("fastload",
       ozconf.showFastLoad = val ? OK : NO;
       );
  DOIF("idleMessage",
       ozconf.showIdleMessage = val ? OK : NO;
       );
  DOIF("showSuspension",
       ozconf.showSuspension = val ? OK : NO;
       );
  DOIF("stopOnToplevelFailure",
       ozconf.stopOnToplevelFailure = val ? OK : NO;);
  DOIF("cellHack",
       ozconf.cellHack = val;
       );
  DOIF("gcFlag",
       ozconf.gcFlag = val;
       );
  DOIF("gcVerbosity",
       ozconf.gcVerbosity = val;
       );
  DOIF("heapThreshold",
       ozconf.heapThreshold = val;
       );
  DOIF("heapMaxSize",
       ozconf.heapMaxSize = val;
       );
  DOIF("heapMargin",
       ozconf.heapMargin = val;
       );
  DOIF("heapIncrement",
       ozconf.heapIncrement = val;
       );
  DOIF("heapIdleMargin",
       ozconf.heapIdleMargin = val;
       );
  DOIF("printDepth",
       ozconf.printDepth = val;
       );
  DOIF("errorVerbosity",
       ozconf.errorVerbosity = val;
       );
  warning("setValue(0,%s): bad feature",toC(feature));
  return NO;
}

OZ_C_proc_begin(BIsetValue,3)
{
  OZ_nonvarArg(0);
  OZ_nonvarArg(1);

  OZ_Term tmpTerm = OZ_getCArg(0);
  OZ_Term fea = OZ_getCArg(1);
  OZ_Term val = OZ_getCArg(2);

  NONVAR(tmpTerm,term,ttag);
  NONVAR(val,value,vtag);
  NONVAR(fea,feature,ftag);
  if (!isLiteral(ftag)) {
    TypeErrorT(1,"Feature");
  }

  if (OZ_isInt(term)) {  // AM
    return am.setValue(feature,value) ? PROCEED : FAILED;
  }

  TypeErrorT(0,"Const or 0");
}
OZ_C_proc_end

#undef DOIF

/*
 * System parameters
 */

#define BICONF(gname,sname,aname)               \
OZ_C_proc_begin(gname,2)                        \
{                                               \
  OZ_declareArg(1,val);                         \
                                                \
  return OZ_unifyInt(val,ozconf.aname);         \
}                                               \
OZ_C_proc_end                                   \
OZ_C_proc_begin(sname,2)                        \
{                                               \
  OZ_declareIntArg(0,val);                      \
  ozconf.aname=val;                             \
  return PROCEED;                               \
}                                               \
OZ_C_proc_end


BICONF(BIgetPrintDepth,BIsetPrintDepth,printDepth)
BICONF(BIgetPrintWidth,BIsetPrintWidth,printWidth)
BICONF(BIgetErrorDepth,BIsetErrorDepth,errorPrintDepth)
BICONF(BIgetErrorWidth,BIsetErrorWidth,errorPrintWidth)

#undef BICONF


// ---------------------------------------------------------------------------
// Debugging: set a break
// ---------------------------------------------------------------------------

OZ_C_proc_begin(BIhalt,0)
{
  tracerOn();
  return PROCEED;
}
OZ_C_proc_end


// ---------------------------------------------------------------------------
// Debugging: special builtins for Benni
// ---------------------------------------------------------------------------

OZ_C_proc_begin(BIsetStreamVar,1)
{
  OZ_Term var = OZ_getCArg(0);
  am.currentThread->setStreamVar(var);
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIsetStepMode,2)
{
  OZ_Term chunk = OZ_deref(OZ_getCArg(0));
  char   *onoff = toC(OZ_getCArg(1));

  ConstTerm *rec = tagged2Const(chunk);
  Thread *thread = ((OzThread*) rec)->th();

  if (!strcmp(onoff, "on"))
    thread->startStepMode();
  else if (!strcmp(onoff, "off"))
    thread->stopStepMode();
  else warning("setStepMode: invalid second argument: must be 'on' or 'off'");
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIstartTraceMode,2)
{
  OZ_Term chunk = OZ_deref(OZ_getCArg(0));
  OZ_Term out   = OZ_getCArg(1);

  ConstTerm *rec = tagged2Const(chunk);
  Thread *thread = ((OzThread*) rec)->th();

  TaggedRef var = OZ_newVariable();
  thread->setStreamVar(var);
  thread->startTraceMode();
  return OZ_unify(out, var);
}
OZ_C_proc_end

OZ_C_proc_begin(BIstopTraceMode,1)
{
  OZ_Term chunk = OZ_deref(OZ_getCArg(0));

  ConstTerm *rec = tagged2Const(chunk);
  Thread *thread = ((OzThread*) rec)->th();

  thread->setStreamVar(OZ_atom("noStream"));
  thread->stopTraceMode();
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIrunPermission,2)
{
  OZ_Term chunk = OZ_deref(OZ_getCArg(0));
  char   *yesno = toC(OZ_getCArg(1));

  ConstTerm *rec = tagged2Const(chunk);
  Thread *thread = ((OzThread*) rec)->th();

  if (!strcmp(yesno, "yes"))
    thread->runPermission();
  else if (!strcmp(yesno, "no"))
    thread->noRunPermission();
  else
    warning("runPermission: invalid second argument: must be 'yes' or 'no'");
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIqueryDebugState,2)
{
  OZ_Term chunk = OZ_deref(OZ_getCArg(0));
  OZ_Term out   = OZ_getCArg(1);

  ConstTerm *rec = tagged2Const(chunk);
  Thread *thread = ((OzThread*) rec)->th();

  return OZ_unify(out,
                  OZ_mkTupleC("debugState",
                              3,
                              thread->traceMode() ? OZ_true() : OZ_false(),
                              thread->stepMode() ? OZ_true() : OZ_false(),
                              thread->stopped() ? OZ_atom("stopped")
                                                : OZ_atom("running")
                              ));

}
OZ_C_proc_end

// --------------------- ... -----------------------------

OZ_C_proc_begin(BIprintLong,1)
{
  OZ_Term args0 = OZ_getCArg(0);
  taggedPrintLong(args0);

  return PROCEED;
}
OZ_C_proc_end



OZ_C_proc_begin(BIidToTerm,2)
{
  OZ_Term term = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  if (!OZ_isInt(term)) {
    TypeErrorT(0,"Int");
  }
  int no = OZ_intToC(term);
  return OZ_unify(out,(TaggedRef) no);
}
OZ_C_proc_end

OZ_C_proc_begin(BItermToId,2)
{
  OZ_Term in = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);
  return OZ_unifyInt(out,(int) in);
}
OZ_C_proc_end

// ----------------------------------------------------------

OZ_C_proc_begin(BIusertime, 1)
{
  OZ_Term out = OZ_getCArg(0);
  return OZ_unifyInt(out, (int) osUserTime());
}
OZ_C_proc_end

OZ_C_proc_begin(BImemory, 1)
   {
    OZ_Term out = OZ_getCArg(0);
    int memory = getUsedMemory()*KB - getMemoryInFreeList();
    return OZ_unifyInt(out,memory);
    }
OZ_C_proc_end


OZ_C_proc_begin(BIshowStatistics,0)
{
  ozstat.print(stderr);
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIgetStatistics,1)
{
  OZ_Term out=OZ_getCArg(0);
  return OZ_unify(out,ozstat.getStatistics());
}
OZ_C_proc_end

OZ_C_proc_begin(BIresetStatistics,0)
{
  ozstat.reset();
  return PROCEED;
}
OZ_C_proc_end



OZ_C_proc_begin(BIisStandalone,1)
{
  return OZ_unify(OZ_getCArg(0),am.isStandalone() ? NameTrue : NameFalse);
}
OZ_C_proc_end

OZ_C_proc_begin(BIshowBuiltins,0)
{
  builtinTab.print();
  return(PROCEED);
}
OZ_C_proc_end

OZ_C_proc_begin(BItraceBack, 0)
{
  am.currentThread->printTaskStack(NOCODE);
  return PROCEED;
}
OZ_C_proc_end




OZ_C_proc_begin(BIplatform, 1)
{
  OZ_Term ret = OZ_pair2(OZ_atom(ozconf.osname),OZ_atom(ozconf.cpu));
  return OZ_unify(ret,OZ_getCArg(0));
}
OZ_C_proc_end


OZ_C_proc_begin(BIozhome, 1)
{
  return OZ_unifyAtom(OZ_getCArg(0),ozconf.ozHome);
}
OZ_C_proc_end

// ---------------------------------------------------------------------------

OZ_C_proc_begin(BIforeignFDProps, 1)
{
  return OZ_unify(OZ_args[0],
#ifdef FOREIGNFDPROPS
                                          NameTrue
#else
                                          NameFalse
#endif
                                          );
}
OZ_C_proc_end

// ---------------------------------------------------------------------------

#ifdef PARSER
OZ_C_proc_proto(ozparser_parse)
OZ_C_proc_proto(ozparser_init)
OZ_C_proc_proto(ozparser_exit)
#else
OZ_C_proc_begin(ozparser_parse,2)
return PROCEED;
OZ_C_proc_end
OZ_C_proc_begin(ozparser_init,0)
return PROCEED;
OZ_C_proc_end
OZ_C_proc_begin(ozparser_exit,0)
return PROCEED;
OZ_C_proc_end
#endif

Object *newObject(SRecord *feat, SRecord *st, ObjectClass *cla,
                  Bool iscl, Board *b)
{
  Bool deep = (b!=am.rootBoard);
  Object *ret = deep
    ? new DeepObject(st,cla,feat,iscl,b)
    : new Object(st,cla,feat,iscl);
  return ret;
}


OZ_C_proc_begin(BImakeClass,8)
{
  OZ_Term fastmeth  = OZ_getCArg(0); { DEREF(fastmeth,_1,_2); }
  OZ_Term printname = OZ_getCArg(1); { DEREF(printname,_1,_2); }
  OZ_Term slowmeth  = OZ_getCArg(2); { DEREF(slowmeth,_1,_2); }
  OZ_Term hfb       = OZ_getCArg(3); { DEREF(hfb,_1,_2); }
  OZ_Term send      = OZ_getCArg(4); { DEREF(send,_1,_2); }
  OZ_Term features  = OZ_getCArg(5); { DEREF(features,_1,_2); }
  OZ_Term ufeatures = OZ_getCArg(6); { DEREF(ufeatures,_1,_2); }
  OZ_Term out       = OZ_getCArg(7);

  /* only isRecord, not isSRecord: fast methods may be empty */
  if (!isRecord(fastmeth)) {
    warning("makeClass: record expected: %s", toC(fastmeth));
    return FAILED;
  }

  SRecord *methods = NULL;

  /* fastmethods may be empty! */
  if (isSRecord(fastmeth)) {
    methods = tagged2SRecord(fastmeth);
    int width = methods->getWidth();
    for (int i=0; i < width; i++) {
      TaggedRef abstr = methods->getArg(i);
      DEREF(abstr,_11,_12);
      if (!isAbstraction(abstr)) {
        warning("makeClass: %s should be an abstraction",
                toC(abstr));
        return FAILED;
      }
      methods->setArg(i,abstr);
    }
  }

  if (!isLiteral(printname)) {
    warning("makeClass: literal expected: %s", toC(printname));
    return FAILED;
  }

  if (!isAbstraction(send)) {
    warning("makeClass: abstraction expected: %s", toC(send));
    return FAILED;
  }

  if (!isRecord(features)) {
    warning("makeClass: record expected: %s", toC(features));
    return FAILED;
  }

  if (!isRecord(ufeatures)) {
    warning("makeClass: record expected: %s", toC(ufeatures));
    return FAILED;
  }

  SRecord *uf = isSRecord(ufeatures) ? tagged2SRecord(ufeatures) : (SRecord*)NULL;

  ObjectClass *cl = new ObjectClass(methods,
                                    tagged2Literal(printname),
                                    slowmeth,
                                    tagged2Abstraction(send),
                                    literalEq(hfb,NameTrue),
                                    uf);

  Object *reto = newObject(tagged2SRecord(features),
                           NULL, // initState
                           cl,
                           OK,
                           am.currentBoard);
  TaggedRef ret   = makeTaggedConst(reto);
  cl->setOzClass(ret);
  return OZ_unify(out,ret);
}
OZ_C_proc_end




TaggedRef methApplHdl = makeTaggedNULL();

OZ_C_proc_begin(BIsetMethApplHdl,1)
{
  OZ_Term preed = OZ_getCArg(0); DEREF(preed,_1,_2);
  if (! isAbstraction(preed)) {
    warning("setMethApplHdl: Abstraction expected");
    return FAILED;
  }

  if (methApplHdl == makeTaggedNULL()) {
    methApplHdl = preed;
    OZ_protect(&methApplHdl);
  } else {
    methApplHdl = preed;
    // warning("setMethApplHdl called twice (hint: prelude may not be fed twice)");
  }
  return PROCEED;
}
OZ_C_proc_end


OZ_Return hasFastBatchInline(TaggedRef t)
{
  DEREF(t,_,tag);
  if (isNotCVar(tag)) return SUSPEND;
  if (!isObject(t)) {
    return FAILED;
  }
  Object *obj = (Object *) tagged2Const(t);
  if (obj->getFastBatch() &&
      am.currentBoard == obj->getBoardFast()) {
    return PROCEED;
  }

  return FAILED;
}

DECLAREBI_USEINLINEREL1(BIhasFastBatch,hasFastBatchInline)


OZ_Return BIisObjectInline(TaggedRef t)
{
  DEREF(t,_1,_2);
  if (isAnyVar(t)) return SUSPEND;
  if (!isObject(t)) {
    return FAILED;
  }
  Object *obj = (Object *) tagged2Const(t);
  return obj->isClass() ? FAILED : PROCEED;
}

DECLAREBI_USEINLINEREL1(BIisObject,BIisObjectInline)
DECLAREBOOLFUN1(BIisObjectB,BIisObjectBInline,BIisObjectInline)

OZ_Return BIisClassInline(TaggedRef t)
{
  DEREF(t,_1,_2);
  if (isAnyVar(t)) return SUSPEND;
  if (!isObject(t)) {
    return FAILED;
  }
  Object *obj = (Object *) tagged2Const(t);
  return obj->isClass() ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisClass,BIisClassInline)
DECLAREBOOLFUN1(BIisClassB,BIisClassBInline,BIisClassInline)


/* getClass(t) returns class of t, if t is an object
 * otherwise return t!
 */
OZ_Return getClassInline(TaggedRef t, TaggedRef &out)
{
  DEREF(t,_,tag);
  if (isAnyVar(tag)) return SUSPEND;
  if (!isObject(t)) {
    out = t;
    return PROCEED;
  }
  out = ((Object *)tagged2Const(t))->getOzClass();
  return PROCEED;
}

DECLAREBI_USEINLINEFUN1(BIgetClass,getClassInline)




inline
TaggedRef cloneObjectRecord(TaggedRef record, Bool cloneAll)
{
  if (isLiteral(record))
    return record;

  Assert(isSRecord(record));

  SRecord *in  = tagged2SRecord(record);
  SRecord *rec = SRecord::newSRecord(in);

  OZ_Term proto = am.currentUVarPrototype;

  for(int i=0; i < in->getWidth(); i++) {
    OZ_Term arg = in->getArg(i);
    if (cloneAll || literalEq(NameOoFreeFlag,deref(arg))) {
      arg = makeTaggedRef(newTaggedUVar(proto));
    }
    rec->setArg(i,arg);
  }

  return makeTaggedSRecord(rec);
}

inline
OZ_Term makeObject(OZ_Term initState, OZ_Term ffeatures, ObjectClass *clas)
{
  Assert(isRecord(initState) && isRecord(ffeatures));

  Object *out =
    newObject(isSRecord(ffeatures) ? tagged2SRecord(ffeatures) : (SRecord*) NULL,
              isSRecord(initState) ? tagged2SRecord(initState) : (SRecord*) NULL,
              clas,
              NO,
              am.currentBoard);

  return makeTaggedConst(out);
}


OZ_Return newObjectInline(TaggedRef cla, TaggedRef &out)
{
  { DEREF(cla,_1,_2); }
  if (isAnyVar(cla)) return SUSPEND;
  if (!isObject(cla)) {
    TypeErrorT(0,"Class");
  }

  Object *obj = (Object *)tagged2Const(cla);
  TaggedRef realclass = obj->getOzClass();
  { DEREF(realclass,_1,_2); }
  Assert(isObject(realclass));

  Object *realcl = (Object *)tagged2Const(realclass);
  TaggedRef attr = realcl->getFeature(NameOoAttr);
  { DEREF(attr,_1,_2); }
  if (isAnyVar(attr)) return SUSPEND;

  TaggedRef attrclone = cloneObjectRecord(attr,NO);

  TaggedRef freefeat = realcl->getFeature(NameOoFreeFeatR);
  { DEREF(freefeat,_1,_2); }
  Assert(!isAnyVar(freefeat));
  TaggedRef freefeatclone = cloneObjectRecord(freefeat,OK);

  out = makeObject(attrclone, freefeatclone, obj->getClass());

  return PROCEED;
}

DECLAREBI_USEINLINEFUN1(BInewObject,newObjectInline)


OZ_C_proc_begin(BInew,3)
{
  /* the suspension handler does the real work */
  return SUSPEND;
}
OZ_C_proc_end



OZ_C_proc_begin(BIsetClosed,1)
{
  OZ_Term obj=OZ_getCArg(0);
  OZ_Term out=OZ_getCArg(1);

  DEREF(obj,objPtr,_2);
  if (isAnyVar(obj)) OZ_suspendOn(makeTaggedRef(objPtr));
  if (!isObject(obj)) return FAILED;
  Object *oo = (Object *)tagged2Const(obj);
  oo->close();
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIgetOONames,3)
{
  if (OZ_unify(OZ_getCArg(0),NameOoAttr) &&
      OZ_unify(OZ_getCArg(1),NameOoFreeFeatR) &&
      OZ_unify(OZ_getCArg(2),NameOoFreeFlag))
    return PROCEED;
  return FAILED;
}
OZ_C_proc_end


OZ_Return objectIsFreeInline(TaggedRef tobj, TaggedRef &out)
{
  DEREF(tobj,_1,_2);
  Assert(isObject(tobj));

  Object *obj = (Object *) tagged2Const(tobj);

  if (am.currentBoard != obj->getBoardFast()) {
    TypeErrorM("object application in local computation space not allowed");
  }

  if (obj->isClosed()) {
    out = NameTrue;
  } else if (obj->getDeepness()>=1) {
    out = obj->attachThread();
  } else {
    obj->incDeepness();
    out = NameFalse;
  }
  return PROCEED;
}

DECLAREBI_USEINLINEFUN1(BIobjectIsFree,objectIsFreeInline)



/* is sometimes explicitely called within Object.oz */
OZ_C_proc_begin(BIreleaseObject,0)
{
  am.getSelf()->release();
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIgetSelf,1)
{
  return OZ_unify(makeTaggedConst(am.getSelf()),
                  OZ_getCArg(0));
}
OZ_C_proc_end


OZ_C_proc_begin(BIsetSelf,1)
{
  TaggedRef o = OZ_getCArg(0);
  DEREF(o,_1,_2);
  if (!isObject(o)) {
    OZ_warning("setSelf(%s): object expected",toC(OZ_getCArg(0)));
    return FAILED;
  }

  Object *obj = (Object *) tagged2Const(o);
  /* same code as in emulate.cc !!!!! */
  if (am.getSelf()!=obj) {
    am.currentThread->pushSelf(am.getSelf());
    am.setSelf(obj);
  }

  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIsetModeToDeep,0)
{
  Assert(0);
  return PROCEED;
}
OZ_C_proc_end


/*===================================================================
 * Exceptions
 *=================================================================== */

OZ_C_proc_begin(BIsetDefaultExceptionHandler,1)
{
  OZ_declareNonvarArg(0,hdl);
  if (!OZ_isProcedure(hdl)) TypeErrorT(0,"Procedure");

  am.defaultExceptionHandler = deref(hdl);
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIbiExceptionHandler,3)
{
  OZ_Term val=OZ_getCArg(0);
  OZ_Term list=OZ_getCArg(1);
  OZ_Term traceBack=OZ_getCArg(2);

  if (ozconf.errorVerbosity > 0) {
    errorHeader();
    if (OZ_isVariable(val) || !OZ_isRecord(val)) {
      message("Exception '%s' caught\n",toC(val));
    } else {
      OZ_Term lab=OZ_label(val);
      if (literalEq(lab,OZ_atom("noElse"))) {
        message("ERROR: Conditional without else failed\n");
        if (ozconf.errorVerbosity > 1) {
          switch (OZ_width(val)) {
          case 2:
            message("Store: %s\n",toC(OZ_getArg(val,1)));
            // fall through
          case 1:
            message("In line: %s\n",toC(OZ_getArg(val,0)));
          default:
            break;
          }
        }
      } else if (literalEq(lab,OZ_atom("toplevelBlocked"))) {
        message("The toplevel is blocked\n");
      } else if (literalEq(lab,OZ_atom("apply"))) {
        message("ERROR: Illtyped application\n");
        if (OZ_width(val) < 2) {
          message("Ups: %s\n",toC(val));
        } else {
          message("In Expression: {%s",toC(OZ_getArg(val,0)));
          OZ_Term args = OZ_getArg(val,1);
          while (OZ_isCons(args)) {
            printf(" %s",toC(OZ_head(args)));
            args = OZ_tail(args);
          }
          printf("}\n");
        }
      } else if (literalEq(lab,OZ_atom("tell"))) {
        message("ERROR: Failure\n");
        message("Tell:  %s\n",toC(OZ_getArg(val,1)));
        message("Store: %s\n",toC(OZ_getArg(val,0)));
      } else if (literalEq(lab,OZ_atom("eq"))) {
        message("ERROR: Failure\n");
        message("Constraint:  %s",toC(OZ_getArg(val,0)));
        printf(" = %s\n",toC(OZ_getArg(val,1)));
      } else if (literalEq(lab,OZ_atom("fail"))) {
        message("ERROR: Failure\n");
        if (ozconf.errorVerbosity > 1) {
          if (OZ_width(val)>0) {
            for (int i=0; i < OZ_width(val); i++) {
              message("[ Hint: %s ]\n",toC(OZ_getArg(val,i)));
            }
          }
        }
      } else if (literalEq(lab,OZ_atom("hf"))) {
        message("ERROR: Failure\n");
        OZ_Term arg0 = OZ_getArg(val,0);
        message("In Expression: {%s",
                OZ_isAtom(arg0) ?
                tagged2Literal(deref(arg0))->getPrintName():toC(arg0));
        OZ_Term args = OZ_getArg(val,1);
        while (OZ_isCons(args)) {
          printf(" %s",toC(OZ_head(args)));
          args = OZ_tail(args);
        }
        printf("}\n");
      } else if (literalEq(lab,OZ_atom("type")) ||
                 literalEq(lab,OZ_atom("ftype"))) {
        message("ERROR: Illtyped application\n");
        if (OZ_width(val) < 2) {
          message("Ups: %s\n",toC(val));
        } else {
          OZ_Term arg0 = OZ_getArg(val,0);
          message("In Expression: %s{%s",
                  literalEq(lab,OZ_atom("ftype")) ? "_ = " : "",
                  OZ_isAtom(arg0) ?
                  tagged2Literal(deref(arg0))->getPrintName():toC(arg0));
          OZ_Term args = OZ_getArg(val,1);
          while (OZ_isCons(args)) {
            printf(" %s",toC(OZ_head(args)));
            args = OZ_tail(args);
          }
          printf("}\n");

          if (ozconf.errorVerbosity > 1) {
            if (!OZ_isNil(args)) message("[ UPS: %s ]\n",toC(args));
            if (OZ_width(val)>2) {
              for (int i=2; i < OZ_width(val); i++) {
                message("[ Hint: %s ]\n",toC(OZ_getArg(val,i)));
              }
            }
          }
        }
      } else if (ozconf.errorVerbosity > 1) {
        message("???: %s\n",toC(val));
      }
    }
    if (ozconf.errorVerbosity > 1) {
      message("\n");
      message("Stack dump:\n");
      message("\n");
      while (OZ_isCons(traceBack)) {
        OZ_Term tt=OZ_head(traceBack);
        OZ_Term lab = OZ_label(tt);
        if (OZ_eq(lab,OZ_atom("proc"))) {
          message(" In procedure %s",toC(OZ_getArg(tt,0)));
          printf(" (File %s",toC(OZ_getArg(tt,1)));
          printf(", Line %s",toC(OZ_getArg(tt,2)));
          printf(", PC = %s)\n",toC(OZ_getArg(tt,3)));
        } else {
          message("[  %s ]\n",toC(tt));
        }
        traceBack=OZ_tail(traceBack);
      }
    }
    errorTrailer();
  }
  return PROCEED;
}
OZ_C_proc_end


#endif /* BUILTINS2 */


/*===================================================================
 * Table of builtins
 *=================================================================== */


#ifdef BUILTINS2
extern BIspec allSpec1[];
#endif

#ifdef BUILTINS1

BIspec allSpec1[] = {
  {"/",  3,BIfdiv,    (IFOR) BIfdivInline},
  {"*",  3,BImult,    (IFOR) BImultInline},
  {"div",3,BIdiv,     (IFOR) BIdivInline},
  {"mod",3,BImod,     (IFOR) BImodInline},
  {"-",  3,BIminus,   (IFOR) BIminusInline},
  {"+",  3,BIplus,    (IFOR) BIplusInline},

  {"Max", 3,BImax,      (IFOR) BImaxInline},
  {"Min", 3,BImin,      (IFOR) BIminInline},

  {"<", 3,BIlessFun,      (IFOR) BIlessInlineFun},
  {"=<",3,BIleFun,        (IFOR) BIleInlineFun},
  {">", 3,BIgreatFun,     (IFOR) BIgreatInlineFun},
  {">=",3,BIgeFun,        (IFOR) BIgeInlineFun},
  {"=:=",2,BInumeqFun,    (IFOR) BInumeqInlineFun},
  {"=\\=",2,BInumneqFun,  (IFOR) BInumneqInlineFun},

  {"=<Rel",2,BIle,       (IFOR) BIleInline},
  {"<Rel",2,BIless,      (IFOR) BIlessInline},
  {">=Rel",2,BIge,       (IFOR) BIgeInline},
  {">Rel",2,BIgreat,     (IFOR) BIgreatInline},
  {"=:=Rel",2,BInumeq,   (IFOR) BInumeqInline},
  {"=\\=Rel",2,BInumneq,  (IFOR) BInumneqInline},

  {"~",2,BIuminus,    (IFOR) BIuminusInline},
  {"+1",2,BIadd1,     (IFOR) BIadd1Inline},
  {"-1",2,BIsub1,     (IFOR) BIsub1Inline},

  {"Exp",  2, BIexp,   (IFOR) BIinlineExp},
  {"Log",  2, BIlog,   (IFOR) BIinlineLog},
  {"Sqrt", 2, BIsqrt,  (IFOR) BIinlineSqrt},
  {"Sin",  2, BIsin,   (IFOR) BIinlineSin},
  {"Asin", 2, BIasin,  (IFOR) BIinlineAsin},
  {"Cos",  2, BIcos,   (IFOR) BIinlineCos},
  {"Acos", 2, BIacos,  (IFOR) BIinlineAcos},
  {"Tan",  2, BItan,   (IFOR) BIinlineTan},
  {"Atan", 2, BIatan,  (IFOR) BIinlineAtan},
  {"Ceil", 2, BIceil,  (IFOR) BIinlineCeil},
  {"Floor",2, BIfloor, (IFOR) BIinlineFloor},
  {"Abs",  2, BIabs,   (IFOR) BIabsInline},

  {"fabs", 2, BIfabs,  (IFOR) BIinlineFabs},
  {"Float.round",2, BIround, (IFOR) BIinlineRound},

  {"fPow",3,BIfPow, (IFOR) BIfPowInline},
  {"Float.atan2",3,BIatan2, (IFOR) BIatan2Inline},

  /* what is a small int ? */
  {"smallIntLimits", 2, BIsmallIntLimits, 0},

  /* conversion: float <-> int <-> virtualStrings */
  {"IntToFloat",2,BIintToFloat,  (IFOR) BIintToFloatInline},
  {"FloatToInt",2,BIfloatToInt,  (IFOR) BIfloatToIntInline},

  {"Int.toString",    2, BIintToString,         0},
  {"Float.toString",  2, BIfloatToString,       0},
  {"String.toInt",    2, BIstringToInt,         0},
  {"String.toFloat",  2, BIstringToFloat,       0},
  {"String.isInt",    2, BIstringIsInt,         0},
  {"String.isFloat",  2, BIstringIsFloat,       0},

  {"IsArray",   2, BIisArray,   (IFOR) isArrayInline},
  {"NewArray",  4, BIarrayNew,  0},
  {"Array.high", 2, BIarrayHigh, (IFOR) arrayHighInline},
  {"Array.low",  2, BIarrayLow,  (IFOR) arrayLowInline},
  {"Get",  3, BIarrayGet,  (IFOR) arrayGetInline},
  {"Put",  3, BIarrayPut,  (IFOR) arrayPutInline},

  {"NewDictionary",     1, BIdictionaryNew,     0},
  {"IsDictionary",      2, BIisDictionary,     (IFOR) isDictionaryInline},
  {"Dictionary.get",    3, BIdictionaryGet,    (IFOR) dictionaryGetInline},
  {"Dictionary.getIf",  4, BIdictionaryGetIf,  (IFOR) dictionaryGetIfInline},
  {"Dictionary.put",    3, BIdictionaryPut,    (IFOR) dictionaryPutInline},
  {"Dictionary.remove", 2, BIdictionaryRemove, (IFOR) dictionaryRemoveInline},
  {"Dictionary.member", 3, BIdictionaryMember, (IFOR) dictionaryMemberInline},
  {"Dictionary.keys",   2, BIdictionaryKeys,    0},

  {"NewGroup",          2,BInewGroup,    0},
  {"NewGroupHdl",       3,BInewGroupHdl, 0},

  {"NewCell",         2,BInewCell,       0},
  {"Exchange",        3,BIexchangeCell, (IFOR) BIexchangeCellInline},

  {"IsChar",        2, BIcharIs,        0},
  {"Char.isAlNum",  2, BIcharIsAlNum,   0},
  {"Char.isAlpha",  2, BIcharIsAlpha,   0},
  {"Char.isCntrl",  2, BIcharIsCntrl,   0},
  {"Char.isDigit",  2, BIcharIsDigit,   0},
  {"Char.isGraph",  2, BIcharIsGraph,   0},
  {"Char.isLower",  2, BIcharIsLower,   0},
  {"Char.isPrint",  2, BIcharIsPrint,   0},
  {"Char.isPunct",  2, BIcharIsPunct,   0},
  {"Char.isSpace",  2, BIcharIsSpace,   0},
  {"Char.isUpper",  2, BIcharIsUpper,   0},
  {"Char.isXDigit", 2, BIcharIsXDigit,  0},
  {"Char.toLower",  2, BIcharToLower,   0},
  {"Char.toUpper",  2, BIcharToUpper,   0},
  {"Char.toAtom",   2, BIcharToAtom,    0},
  {"Char.type",     2, BIcharType,      0},

  {"Adjoin",          3,BIadjoin,           (IFOR) BIadjoinInline},
  {"AdjoinList",      3,BIadjoinList,       0},
  {"record",      3,BImakeRecord,       0},
  {"Arity",           2,BIarity,            (IFOR) BIarityInline},
  {"AdjoinAt",        4,BIadjoinAt,         0},
  {"@",               2,BIat,               (IFOR) atInline},
  {"<-",              2,BIassign,           (IFOR) assignInline},
  {"copyRecord",      2,BIcopyRecord,       0},

  {0,0,0,0}
};
#endif

#ifdef BUILTINS2

BIspec allSpec2[] = {
  {"IsValue",2,BIisValueB,        (IFOR) isValueBInline},
  {"IsNumber",2,BIisNumberB,      (IFOR) BIisNumberBInline},
  {"IsInt"   ,2,BIisIntB,         (IFOR) BIisIntBInline},
  {"IsFloat" ,2,BIisFloatB,       (IFOR) BIisFloatBInline},
  {"IsRecord",2,BIisRecordB,      (IFOR) isRecordBInline},
  {"IsTuple",2,BIisTupleB,        (IFOR) isTupleBInline},
  {"IsLiteral",2,BIisLiteralB,    (IFOR) isLiteralBInline},
  {"IsCell",2,BIisCellB,          (IFOR) isCellBInline},
  {"IsProcedure",2,BIisProcedureB,(IFOR) isProcedureBInline},
  {"IsName",2,BIisNameB,          (IFOR) isNameBInline},
  {"IsAtom",2,BIisAtomB,          (IFOR) isAtomBInline},
  {"IsBool", 2,BIisBoolB,         (IFOR) isBoolBInline},
  {"IsChunk",2,BIisChunkB,        (IFOR) isChunkBInline},
  {"IsRecordC",2,BIisRecordCB,    (IFOR) isRecordCBInline},
  {"IsObject", 2,BIisObjectB,     (IFOR) BIisObjectBInline},
  {"IsClass", 2,BIisClassB,       (IFOR) BIisClassBInline},
  {"IsString", 2,BIisString,      0},

  {"Wait",1,BIisValue,            (IFOR) isValueInline},

  {"isNumberRel",1,BIisNumber,       (IFOR) BIisNumberInline},
  {"isIntRel"   ,1,BIisInt,          (IFOR) BIisIntInline},
  {"isFloatRel" ,1,BIisFloat,        (IFOR) BIisFloatInline},
  {"isRecordRel",1,BIisRecord,       (IFOR) isRecordInline},
  {"isTupleRel",1,BIisTuple,         (IFOR) isTupleInline},
  {"isLiteralRel",1,BIisLiteral,     (IFOR) isLiteralInline},
  {"isCellRel",1,BIisCell,           (IFOR) isCellInline},
  {"isProcedureRel",1,BIisProcedure, (IFOR) isProcedureInline},
  {"isNameRel",1,BIisName,           (IFOR) isNameInline},
  {"isAtomRel",1,BIisAtom,           (IFOR) isAtomInline},
  {"isBoolRel",  1,BIisBool,         (IFOR) isBoolInline},
  {"isChunkRel",  1,BIisChunk,       (IFOR) isChunkInline},
  {"isRecordCRel",1,BIisRecordC,     (IFOR) isRecordCInline},
  {"isClassRel", 1,BIisClass,        (IFOR) BIisClassInline},
  {"isObjectRel", 1,BIisObject,      (IFOR) BIisObjectInline},

  {"isVar",2,BIisVarB,           (IFOR) isVarBInline},
  {"isNonvar",2,BIisNonvarB,     (IFOR) isNonvarBInline},
  {"isVarRel",1,BIisVar,             (IFOR) isVarInline},
  {"isNonvarRel",1,BIisNonvar,       (IFOR) isNonvarInline},


  {"IsVirtualString",      2, BIvsIs,    0},
  {"VirtualString.length", 3, BIvsLength, 0},

  {"getTrue", 1,BIgetTrue,         0},
  {"getFalse",1,BIgetFalse,        0},
  {"Not",     2,BInot,             (IFOR) notInline},
  {"And",     3,BIand,             (IFOR) andInline},
  {"Or",      3,BIor,              (IFOR) orInline},

  {"Value.type",2,BItermType,            (IFOR) BItermTypeInline},

  {"ProcedureArity",2,BIprocedureArity,  (IFOR)procedureArityInline},
  {"MakeTuple",3,BItuple,              (IFOR) tupleInline},
  {"Label",2,BIlabel,              (IFOR) labelInline},

  {"MakeRecordC",  1, BIrecordC,        0},
  {"LabelC",       2, BIlabelC,         0},
  {"WidthC",       2, BIwidthC,         0},
  {"SubtreeC",     3, BIuparrow,        (IFOR) uparrowInline},

  {"recordCIsVar", 1, BIisRecordCVar,   0},
  {"recordCIsVarB",2, BIisRecordCVarB,  0},
  {"setC",         3, BIsetC,           0},
  {"removeC",      2, BIremoveC,        0},
  {"testCB",       3, BItestCB,         0},
  {"monitorArity", 3, BImonitorArity,   0},
  {"recordCSize",  2, BIrecordCSize,    0},

  {".",            3,BIdot,              (IFOR) dotInline},
  {"Subtree",      3,BIsubtree,          (IFOR) subtreeInline},
  {"^",            3,BIuparrow,          (IFOR) uparrowInline},

  {"HasSubtreeAt",    3,BIhasSubtreeAtB,   (IFOR)hasSubtreeAtBInline},
  {"hasSubtreeAtRel", 2,BIhasSubtreeAt,    (IFOR)hasSubtreeAtInline},
  {"Width",           2,BIwidth,            (IFOR) widthInline},

  {"AtomToString",    2, BIatomToString,        0},
  {"StringToAtom",    2, BIstringToAtom,        0},

  {"NewChunk",        2,BInewChunk,     0},
  {"chunkArity",      2,BIchunkArity,   0},

  {"NewName",         1,BInewName,      0},

  {"setThreadPriority", 1, BIsetThreadPriority, 0},
  {"getThreadPriority", 1, BIgetThreadPriority, 0},

  {"==",      3,BIeqB,    (IFOR) eqeqInline},
  {"\\=",     3,BIneqB,   (IFOR) neqInline},
  {"==Rel",   2,BIeq,     0},
  {"\\=Rel",  2,BIneq,    0},

  {"loadFile",       1, BIloadFile,             0},
  {"linkObjectFiles",2, BIlinkObjectFiles,      0},
  {"unlinkObjectFile",1,BIunlinkObjectFile,     0},
  {"findFunction",   3, BIfindFunction,         0},
  {"shutdown",       0, BIshutdown,             0},

  {"Sleep",          3, BIsleep,                0},

  {"garbageCollection",0,BIgarbageCollection,   0},

  {"apply",          2, BIapply,                0},

  {"eq",             2, BIsystemEq,             0},
  {"eqB",            3, BIsystemEqB,            0},

  {"=",              2, BIunify,                0},
  {"fail",           VarArity,BIfail,           0},

  {"deepReadCell",   2, BIdeepReadCell,         0},
  {"deepFeed",       2, BIdeepFeed,             0},

  {"genericSet",     3, BIgenericSet,           0},

  {"atomHash",       3, BIatomHash,             0},

  {"SubtreeIf",   4, BImatchDefault,         (IFOR) matchDefaultInline},

  {"gensym",         2, BIgensym,               0},

  {"getsBound",      1, BIgetsBound,            0},
  {"getsBoundB",     2, BIgetsBoundB,           0},
  {"intToAtom",      2, BIintToAtom,            0},

  {"connectLingRef", 1, BIconnectLingRef,       0},
  {"getLingRefFd",   1, BIgetLingRefFd,         0},
  {"getLingEof",     1, BIgetLingEof,           0},
  {"getOzEof",       1, BIgetLingEof,           0},
  {"constraints",    2, BIconstraints,          0},

  {"setAbstractionTabDefaultEntry", 1, BIsetAbstractionTabDefaultEntry, 0},

  {"usertime",1,BIusertime},
  {"memory",1,BImemory},
  {"isStandalone",1,BIisStandalone},
  {"showBuiltins",0,BIshowBuiltins},
  {"Print",1,BIprint,  (IFOR) printInline},
  {"printError",1,BIprintError},
  {"Show",1,BIshow,  (IFOR) showInline},

  {"getValue",3,BIgetValue},
  {"setValue",3,BIsetValue},
  {"getArgv", 1,BIgetArgv},

  {"halt",0,BIhalt},

  // Debugging ---
  {"setStreamVar",1,BIsetStreamVar},

  {"startTraceMode",2,BIstartTraceMode},
  {"stopTraceMode",1,BIstopTraceMode},

  {"setStepMode",2,BIsetStepMode},

  {"runPermission",2,BIrunPermission},
  // End Debugging ---

  {"queryDebugState",2,BIqueryDebugState},

  {"printLong",1,BIprintLong},

  {"termToId",2,BItermToId},
  {"idToTerm",2,BIidToTerm},

  {"spy",         1, BIspy},
  {"nospy",       1, BInospy},
  {"traceOn",     0, BItraceOn},
  {"traceOff",    0, BItraceOff},
  {"displayCode", 2, BIdisplayCode},

  {"showStatistics",0,BIshowStatistics},
  {"getStatistics",1,BIgetStatistics},
  {"resetStatistics",0,BIresetStatistics},
  {"System_getPrintName",2,BIgetPrintName},
  {"traceBack",0,BItraceBack},

  {"ozparser_parse",2,ozparser_parse},
  {"ozparser_init",0,ozparser_init},
  {"ozparser_exit",0,ozparser_exit},

  {"printVS",1,BIprintVS},
  {"termToVS",2,BItermToVS},
  {"getTermSize",4,BIgetTermSize},

  {"dumpThreads",0,BIdumpThreads},
  {"listThreads",1,BIlistThreads},

  {"foreignFDProps", 1, BIforeignFDProps},
  {"platform",       1, BIplatform},
  {"ozhome",         1, BIozhome},

  {"makeClass",        8,BImakeClass,          0},
  {"setModeToDeep",    0,BIsetModeToDeep,  0},
  {"setMethApplHdl",   1,BIsetMethApplHdl,     0},
  {"getClass",         2,BIgetClass,           (IFOR) getClassInline},
  {"new",              3,BInew,                0},
  {"newObject",        2,BInewObject,          (IFOR) newObjectInline},
  {"hasFastBatch",     1,BIhasFastBatch,       (IFOR) hasFastBatchInline},
  {"objectIsFree",     2,BIobjectIsFree,       (IFOR) objectIsFreeInline},
  {"getOONames",       3,BIgetOONames,         0},
  {"releaseObject",    0,BIreleaseObject,      0},
  {"getSelf",          1,BIgetSelf,            0},
  {"setSelf",          1,BIsetSelf,            0},
  {"setClosed",        1,BIsetClosed,          0},

  {"Space.new",           2, BInewSpace,        0},
  {"IsSpace",             2, BIisSpace,         0},
  {"Space.ask",           2, BIaskSpace,        0},
  {"Space.askVerbose",    2, BIaskVerboseSpace, 0},
  {"Space.merge",         2, BImergeSpace,      0},
  {"Space.clone",         2, BIcloneSpace,      0},
  {"Space.choose",        2, BIchooseSpace,     0},
  {"Space.inject",        2, BIinjectSpace,     0},

  {"System.getPrintDepth", 2, BIgetPrintDepth, 0},
  {"System.getPrintWidth", 2, BIgetPrintWidth, 0},
  {"System.getErrorDepth", 2, BIgetErrorDepth, 0},
  {"System.getErrorWidth", 2, BIgetErrorWidth, 0},
  {"System.setPrintDepth", 2, BIsetPrintDepth, 0},
  {"System.setPrintWidth", 2, BIsetPrintWidth, 0},
  {"System.setErrorDepth", 2, BIsetErrorDepth, 0},
  {"System.setErrorWidth", 2, BIsetErrorWidth, 0},

  {"biExceptionHandler",         3, BIbiExceptionHandler,         0},
  {"setDefaultExceptionHandler", 1, BIsetDefaultExceptionHandler, 0},
  {0,0,0,0}
};


extern void BIinitFD(void);
extern void BIinitMeta(void);
extern void BIinitAVar(void);
extern void BIinitDVar(void);
extern void BIinitUnix();
extern void BIinitAssembler();
extern void BIinitTclTk();

BuiltinTabEntry *BIinit()
{
  BuiltinTabEntry *bi = BIadd("builtin",3,BIbuiltin);

  if (!bi)
    return bi;

  BIaddSpec(allSpec1);
  BIaddSpec(allSpec2);

  /* see emulate.cc */
  BIaddSpecial("raise",             1, BIraise);

#ifdef ASSEMBLER
  BIinitAssembler();
#endif

  BIinitFD();
  BIinitMeta();

  BIinitAVar();
  BIinitDVar();
  BIinitUnix();
  BIinitTclTk();

  return bi;
}

#endif /* BUILTINS2 */
