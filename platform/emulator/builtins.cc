/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "builtins.hh"
#endif

#include <ctype.h>
#include <string.h>
#ifdef WINDOWS
#include "wsock.hh"
#endif

#include <errno.h>

#include "dldwrap.h"
#if DLOPEN
#include <dlfcn.h>
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

/*===================================================================
 * Macros
 *=================================================================== */

#define NONVAR(X,term,tag)  					              \
TaggedRef term = X;							      \
TypeOfTerm tag;								      \
{ DEREF(term,_myTermPtr,myTag);  	                                      \
  tag = myTag;		    		                                      \
  if (isAnyVar(tag)) return SUSPEND;					      \
}

// mm2
// Suspend on UVAR and SVAR:
#define NONSUVAR(X,term,tag)                                                  \
TaggedRef term = X;                                                           \
TypeOfTerm tag;                                                               \
{ DEREF(term,_myTermPtr,myTag);                                               \
  tag = myTag;                                                                \
  if (isNotCVar(tag)) return SUSPEND; 			                      \
}


#define DECLAREBI_USEINLINEREL1(Name,InlineName)			      \
OZ_C_proc_begin(Name,1)							      \
{									      \
  OZ_Term arg1 = OZ_getCArg(0);						      \
  State state = InlineName(arg1);				      	      \
  if (state == SUSPEND)	{						      \
    return OZ_suspendOnVar(arg1);					      \
  } else {								      \
    return state;							      \
  }									      \
}									      \
OZ_C_proc_end


#define DECLAREBI_USEINLINEREL2(Name,InlineName)			      \
OZ_C_proc_begin(Name,2)							      \
{									      \
  OZ_Term arg0 = OZ_getCArg(0);						      \
  OZ_Term arg1 = OZ_getCArg(1);						      \
  State state = InlineName(arg0,arg1);					      \
  if (state == SUSPEND) {						      \
    return OZ_suspendOnVar2(arg0,arg1);					      \
  } else {								      \
    return state;							      \
  }									      \
}									      \
OZ_C_proc_end


#define DECLAREBI_USEINLINEFUN1(Name,InlineName)			      \
OZ_C_proc_begin(Name,2)							      \
{									      \
  OZ_Term help;								      \
  									      \
  OZ_Term arg1 = OZ_getCArg(0);						      \
  State state = InlineName(arg1,help);					      \
  switch (state) {							      \
  case SUSPEND: 							      \
    return OZ_suspendOnVar(arg1);					      \
  case PROCEED:								      \
    return(OZ_unify(help,OZ_getCArg(1)));				      \
  default:								      \
    return state;							      \
  }									      \
									      \
}									      \
OZ_C_proc_end



#define DECLAREBI_USEINLINEFUN2(Name,InlineName)	\
OZ_C_proc_begin(Name,3)					\
{							\
  OZ_Term help;						\
							\
  OZ_Term arg0 = OZ_getCArg(0);				\
  OZ_Term arg1 = OZ_getCArg(1);				\
  State state=InlineName(arg0,arg1,help);		\
  switch (state) {					\
  case SUSPEND:						\
    return OZ_suspendOnVar2(arg0,arg1);			\
  case PROCEED:						\
    return(OZ_unify(help,OZ_getCArg(2)));		\
  default:						\
    return state;					\
  }							\
							\
}							\
OZ_C_proc_end


#define DECLAREBI_USEINLINEFUN3(Name,InlineName)			      \
OZ_C_proc_begin(Name,4)							      \
{									      \
  OZ_Term help;								      \
  									      \
  OZ_Term arg0 = OZ_getCArg(0);						      \
  OZ_Term arg1 = OZ_getCArg(1);						      \
  OZ_Term arg2 = OZ_getCArg(2);						      \
  State state=InlineName(arg0,arg1,arg2,help);		  		      \
  switch (state) {							      \
  case SUSPEND:								      \
    return OZ_suspendOnVar3(arg0,arg1,arg2);				      \
  case PROCEED:								      \
    return(OZ_unify(help,OZ_getCArg(3)));				      \
  default:								      \
    return state;							      \
  }									      \
									      \
}									      \
OZ_C_proc_end

#define DECLAREBOOLFUN1(BIfun,BIifun,BIirel) 				      \
State BIifun(TaggedRef val, TaggedRef &out)				      \
{									      \
  State state = BIirel(val);						      \
  switch(state) {							      \
  case PROCEED: out = NameTrue;  return PROCEED;			      \
  case FAILED:  out = NameFalse; return PROCEED;			      \
  default: return state;						      \
  }									      \
}									      \
DECLAREBI_USEINLINEFUN1(BIfun,BIifun)

#define DECLAREBOOLFUN2(BIfun,BIifun,BIirel) 				      \
State BIifun(TaggedRef val1, TaggedRef val2, TaggedRef &out)		      \
{									      \
  State state = BIirel(val1,val2);					      \
  switch(state) {							      \
  case PROCEED: out = NameTrue;  return PROCEED;			      \
  case FAILED:  out = NameFalse; return PROCEED;			      \
  default: return state;						      \
  }									      \
}									      \
DECLAREBI_USEINLINEFUN2(BIfun,BIifun)

/*===================================================================
 * BuiltinTab
 *=================================================================== */

BuiltinTab builtinTab(750);


BuiltinTabEntry *BIadd(char *name,int arity, OZ_CFun funn, Bool replace,
		       IFOR infun)
{
  BuiltinTabEntry *builtin = new BuiltinTabEntry(name,arity,funn,infun);

  if (builtinTab.aadd(builtin,name,replace) == NO) {
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
    BIadd(spec[i].name,spec[i].arity,spec[i].fun,
	  spec[i].yps,spec[i].ifun);
  }
}

BuiltinTabEntry *BIaddSpecial(char *name,int arity,BIType t, Bool replace)
{
  BuiltinTabEntry *builtin = new BuiltinTabEntry(name,arity,t);

  if (builtinTab.aadd(builtin,name,replace) == NO) {
    warning("BIadd: failed to add %s/%d\n",name,arity);
    delete builtin;
    return((BuiltinTabEntry *) NULL);
  }
  return(builtin);
}

/*===================================================================
 * `builtin`
 *=================================================================== */

OZ_C_proc_begin(BIbuiltin,3)
{
  OZ_declareAtomArg(0,str);
  OZ_Term hdl = OZ_getCArg(1);
  OZ_Term ret = OZ_getCArg(2);

  DEREF(hdl,_3,tag);
  if (!isProcedure(hdl)) {
    if (!isAtom(hdl) || !OZ_unifyString(hdl,"noHandler")) {
      TypeError2("builtin",1,"Procedure or Atom \"noHandler\"",
		 OZ_getCArg(0),hdl);
    }
    hdl = makeTaggedNULL();
  }

  BuiltinTabEntry *found = (BuiltinTabEntry *) builtinTab.ffind(str);

  if (found == htEmpty) {
    warning("builtin: '%s' not in table", str);
    return FAILED;
  }

  if (hdl == makeTaggedNULL() && !found->getFun()) {
    warning("builtin '%s' is special and needs suspension handler",str);
    return FAILED;
  }
  if (hdl != makeTaggedNULL() && found->getInlineFun()) {
    hdl = makeTaggedNULL();
    warning("builtin '%s' is compiled inline, suspension handler ignored",str);
  }

  Builtin *bi = new Builtin(found,hdl);

  return (OZ_unify(ret,makeTaggedConst(bi)));
}
OZ_C_proc_end


/*===================================================================
 * All builtins
 *=================================================================== */

State isValueInline(TaggedRef vall)
{ 
  NONVAR( vall, _1, tag );
  return PROCEED;
}

DECLAREBI_USEINLINEREL1(BIisValue,isValueInline)
DECLAREBOOLFUN1(BIisValueB,isValueBInline,isValueInline)

State isLiteralInline(TaggedRef t)
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
      default:
          return FAILED;
      }
  }
  return isLiteral(tag) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisLiteral,isLiteralInline)
DECLAREBOOLFUN1(BIisLiteralB,isLiteralBInline,isLiteralInline)


State isAtomInline(TaggedRef t)
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
      default:
          return FAILED;
      }
  }
  return isAtom(term) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisAtom,isAtomInline)
DECLAREBOOLFUN1(BIisAtomB,isAtomBInline,isAtomInline)


State isVarInline(TaggedRef term)
{
  DEREF(term, _1, _2);
  return isAnyVar(term) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisVar,isVarInline)
DECLAREBOOLFUN1(BIisVarB,isVarBInline,isVarInline)

State isNonvarInline(TaggedRef term)
{
  DEREF(term, _1, _2);
  return isAnyVar(term) ? FAILED : PROCEED;
}

DECLAREBI_USEINLINEREL1(BIisNonvar,isNonvarInline)
DECLAREBOOLFUN1(BIisNonvarB,isNonvarBInline,isNonvarInline)


State isNameInline(TaggedRef t)
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
      default:
	return FAILED;
      }
  }
  if (!isLiteral(tag)) return FAILED;
  return isAtom(term) ? FAILED: PROCEED;
}

DECLAREBI_USEINLINEREL1(BIisName,isNameInline)
DECLAREBOOLFUN1(BIisNameB,isNameBInline,isNameInline)


State isTupleInline(TaggedRef t)
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
      default:
	return FAILED;
      }
  }
  return isTuple(term) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisTuple,isTupleInline)
DECLAREBOOLFUN1(BIisTupleB,isTupleBInline,isTupleInline)


State isRecordInline(TaggedRef t)
{
  NONSUVAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case OFSVariable:
          return SUSPEND;
      default:
          return FAILED;
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
    if (tagged2CVar(t)->getType()!=OFSVariable) return FAILED;
    return PROCEED;
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


// Constrain term to a record, with a given initial size of hash table.
// Never suspend.  If term is already a record, do nothing.
OZ_C_proc_begin(BIrecordCSize,2)
{
  TaggedRef tSize = OZ_getCArg(0);
  TaggedRef t = OZ_getCArg(1); 

  DEREF(tSize, sPtr, stag);
  DEREF(t, tPtr, tag);

  /* Get initial size of hash table */
  if (!isSmallInt(stag)) TypeError2("recordCSize",0,"Int",tSize,t);
  dt_index size=smallIntValue(tSize);
  if (size!=0 && !isPwrTwo(size)) 
      TypeError2("recordCSize",0,"Zero or power of 2",tSize,t);

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
State isRecordCInline(TaggedRef t)
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



State isProcedureInline(TaggedRef t)
{
  NONVAR( t, term, tag );
  return isProcedure(term) ? PROCEED : FAILED;
}

DECLAREBI_USEINLINEREL1(BIisProcedure,isProcedureInline)
DECLAREBOOLFUN1(BIisProcedureB,isProcedureBInline,isProcedureInline)

State isChunkInline(TaggedRef t)
{
  NONSUVAR( t, term, tag );
  if (isCVar(tag)) {
      switch (tagged2CVar(term)->getType()) {
      case OFSVariable:
          return SUSPEND;
      default:
          return FAILED;
      }
  }
  return isChunk(term) ? PROCEED : FAILED;
}
DECLAREBOOLFUN1(BIisChunk,isChunkBInline,isChunkInline)

State isNaryInline(TaggedRef procedure, TaggedRef arity)
{
  DEREF( procedure, pterm, ptag );
  DEREF( arity, aterm, atag );

  if (isProcedure(procedure)) {
    if (isSmallInt(arity)) {
      int i = smallIntValue(arity);
      ConstTerm *rec = tagged2Const(procedure);

      switch (rec->getType()) {
      case Co_Abstraction:
	return ((Abstraction *) rec)->getArity()==i ? PROCEED : FAILED;
      case Co_Builtin:
	return ((Builtin *) rec)->getArity()==i ? PROCEED : FAILED;
      default:
	goto typeError0;
      }
    }
    if (isAnyVar(arity)) return SUSPEND;
    if (isBigInt(arity)) return FAILED;
    goto typeError1;
  }

  if (isAnyVar(procedure)) {
    if (isBigInt(arity)) return FAILED;
    if (isSmallInt(arity) && isAnyVar(arity)) return SUSPEND;
    goto typeError1;
  }
  goto typeError0;

typeError0:
  TypeError2("isNary",0,"Procedure",procedure,arity);
typeError1:
  TypeError2("isNary",1,"Int",procedure,arity);
}
DECLAREBI_USEINLINEREL2(BIisNary,isNaryInline)
DECLAREBOOLFUN2(BIisNaryB,isNaryBInline,isNaryInline)

State procedureArityInline(TaggedRef procedure, TaggedRef &out)
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
  TypeError1("procedureArity",0,"Procedure",pterm);
}

DECLAREBI_USEINLINEFUN1(BIprocedureArity,procedureArityInline)


OZ_C_proc_begin(BIcloneProcedure,2)
{
  TaggedRef master = OZ_getCArg(0);

  DEREF(master, _m, mtag);
  if (isAnyVar(mtag)) return SUSPEND;

  if (isProcedure(master)) {
    ConstTerm *rec = tagged2Const(master);
    
    switch (rec->getType()) {
    case Co_Abstraction:
      return OZ_unify(OZ_getCArg(1),master);
    case Co_Builtin:
      if (((Builtin *) rec)->getType() == BIsolveCont) {

	if (((OneCallBuiltin *) rec)->isSeen()) {
	  return OZ_unify(OZ_getCArg(1),master);
	} else {
	  RefsArray contGRegs;
      
	  Board *solveBB =
	    (Board *) tagValueOf ((((OneCallBuiltin *) rec)->getGRegs ())[0]);
      
	  SolveActor::Cast (solveBB->getActor ())->setBoard (am.currentBoard);
      
	  Board *newSolveBB = (Board *) am.copyTree(solveBB, (Bool *) NULL);
	  ozstat.incSolveClone();

	  contGRegs = allocateRefsArray(1);
	  contGRegs[0] = makeTaggedConst(newSolveBB);
      
	  return (OZ_unify(OZ_getCArg(1),
			   makeTaggedConst
			   (new OneCallBuiltin 
			    (((Builtin *) rec)->getBITabEntry(), 
			     contGRegs))));
	}
      } else if (((Builtin *) rec)->getType() == BIsolved) {
	return OZ_unify(OZ_getCArg(1),master);
      }
    default:
      goto typeError;
    }
  }
  goto typeError;

typeError:
  TypeError1("cloneProcedure",0,"Procedure",master);
}
OZ_C_proc_end


State isCellInline(TaggedRef cell)
{
  NONVAR( cell, term, _ );
  return isCell(term) ? PROCEED : FAILED;
}
DECLAREBI_USEINLINEREL1(BIisCell,isCellInline)
DECLAREBOOLFUN1(BIisCellB,isCellBInline,isCellInline)


// ---------------------------------------------------------------------
// Tuple
// ---------------------------------------------------------------------

State tupleInline(TaggedRef label, TaggedRef argno, TaggedRef &out)
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
	SRecord *s =
	  SRecord::newSRecord(label,i);

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
  TypeError2("tuple",0,"Literal",label,argno);
 typeError1:
  TypeError2("tuple",1,"(non-negative small) Int",label,argno);
}

DECLAREBI_USEINLINEFUN2(BItuple,tupleInline)


// ---------------------------------------------------------------------
// Tuple & Record
// ---------------------------------------------------------------------


State labelInline(TaggedRef term, TaggedRef &out)
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
    if (tagged2CVar(term)->getType() == OFSVariable) return SUSPEND;
    break;
  default:
    break;
  }
  TypeError1("label",0,"Record",term);
}

DECLAREBI_USEINLINEFUN1(BIlabel,labelInline)


// Used by BIwidthC and BIpropWidth built-ins
// {PropWidth X W} where X is OFS and W is FD width.
// Assume: X is OFS or SRECORD or LITERAL.
// Assume: W is FD or SMALLINT or BIGINT.
// This is the simplest most straightforward possible
// implementation and it can be optimized in many ways.
OZ_Bool internPropWidth(TaggedRef rawrec, TaggedRef rawwid)
{
    int res;
    int recwidth;
    TaggedRef rec=rawrec;
    TaggedRef wid=rawwid;
    DEREF(rec,_1,recTag);
    DEREF(wid,_2,widTag);
    OZ_Bool result = SLEEP;
    switch (recTag) {
    case SRECORD:
    record:
    case LITERAL:
    {
        // Impose width constraint
        recwidth=(recTag==SRECORD) ? tagged2SRecord(rec)->getWidth() : 0;
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
            GenFDVariable *fdcon=new GenFDVariable(slice);
            res=am.unify(makeTaggedRef(newTaggedCVar(fdcon)),rawwid);
            // No loc/glob handling: res=(fdwid>=recwidth);
            if (res==FAILED) { result = FAILED; break; }
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
                    // Convert to SRECORD:
                    TaggedRef arity=tagged2GenOFSVar(rec)->getTable()->getArityList();
                    Arity *atable=aritytable.find(arity);
                    SRecord *newrec = SRecord::newSRecord(lbl,atable);
		    newrec->initArgs(am.currentUVarPrototype); 
                    res=am.unify(rawrec,makeTaggedSRecord(newrec));
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



// This propagator is installed by the WidthC built-in (BIwidthC):
// This is a stub to internPropWidth which does the work.
OZ_C_proc_begin(BIpropWidth,2)
{
    TaggedRef rec=OZ_getCArg(0);
    TaggedRef wid=OZ_getCArg(1);
    return (internPropWidth(rec, wid));
}
OZ_C_proc_end



// {RecordC.widthC X W} -- builtin that constrains number of features of X to be
// equal to finite domain variable W.  Will constrain X to a record and W to a
// finite domain.  This built-in installs the propagator BIpropWidth.
OZ_C_proc_begin(BIwidthC, 2)
{
    OZ_Term rawrec = OZ_getCArg(0);
    OZ_Term rawwid = OZ_getCArg(1);
    OZ_Term rec=rawrec;
    OZ_Term wid=rawwid;

    // Ensure that first argument rec is an OFS, SRECORD, or LITERAL:
    DEREF(rec,_1,recTag);
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
      break;
    default:
      return FAILED;
    }

    // Ensure that second argument wid is a FD or integer:
    DEREF(wid,_2,widTag);
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

    // Execute propagator once and create suspension if necessary:
    OZ_Bool result;
    result = internPropWidth(rawrec, rawwid);
    switch (result) {
    case FAILED:
	return FAILED;

    case SLEEP:
    {
        // Create a 'BIpropWidth' propagator;
        Thread *thr = 
	    createPropagator (BIpropWidth, 2, allocateRegs (rawrec, rawwid));
	thr->suspendPropagator ();
        Assert(OZ_isVariable(rawrec));
        OZ_addThread (rawrec, thr);
        if (OZ_isVariable (rawwid)) OZ_addThread (rawwid, thr);
        // This could be optimized to add the lbl suspension only when the
        // conditions are satisfied (inside internPropWidth):
	rec=rawrec;
	DEREF(rec,_3,_4);
	if (isCVar(rec)) {
	    TaggedRef lbl=tagged2GenOFSVar(rec)->getLabel();
	    if (OZ_isVariable (lbl)) OZ_addThread (lbl, thr);
	}

	result = PROCEED;
	break;
    }

    case SUSPEND:
    case PROCEED:
	break;

    default:
	error ("unknown value returned from the 'internPropWidth'\n");
	break;
    }

    return result;
}
OZ_C_proc_end



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
        TypeError1("labelC",0,"Record",term);
    thelabel=tagged2GenOFSVar(term)->getLabel(); 
    break;
  default:
    TypeError1("labelC",0,"Record",term);
  }

  // At this point, thelabel is term's label
  // Constrain the term's label to be lbl:
  Assert(thelabel!=makeTaggedNULL());
  TaggedRef thelabeldrf=thelabel;
  DEREF(thelabeldrf,_1,_2);
  // one of the two must be a literal:
  if (!isAnyVar(lbltag) && !isLiteral(lbldrf)) return FAILED;
  if (!isLiteral(thelabeldrf) && !isLiteral(lbldrf)) {
      // Suspend if at least one of the two is a variable:
    return OZ_suspendOnVar2(thelabel,lbl);
  }
  return (am.unify(thelabel,lbl)? PROCEED : FAILED);
}
OZ_C_proc_end

// DECLAREBI_USEINLINEREL2(BIlabelC,labelCInline);


// The propagator for the built-in RecordC.monitorArity 
// {PropFeat X K L FH FT} -- propagate features from X to the list L, and go
// away when K is determined.  L is closed when K is determined.  X is used to
// check in addFeatOFSSuspList that the suspension is waiting for the right
// variable.  FH and FT are a difference list that holds the features that
// have been added.
OZ_C_proc_begin(BIpropFeat,5)
{
    // This test is unnecessary:
    // Check type of X: (must be OFS)
    // TaggedRef rec=OZ_getCArg(0);
    // DEREF(rec,_1,recTag);
    // if (!(recTag==CVAR && tagged2CVar(rec)->getType()==OFSVariable)) {
    //     TypeError1("RecordC.monitorArity",0,"Record",rec);
    //     return FAILED;
    // }

    // Check if killed:
    TaggedRef kill=OZ_getCArg(1);
    TaggedRef tmpkill=kill;
    DEREF(tmpkill,_2,killTag);
    Bool isKilled = !isAnyVar(killTag);

    TaggedRef tmptail=OZ_getCArg(4);
    DEREF(tmptail,_3,_4);

    // Get featlist (a difference list stored in the arguments):
    TaggedRef fhead = OZ_getCArg(3);
    TaggedRef ftail = OZ_getCArg(4);

    if (tmptail!=AtomNil) {
        // The record is not determined, so reinitialize the featlist:
        // The home board of uvar must be taken from outside propFeat!
        // Get the home board for any new variables:
        Board* home=tagged2VarHome(tmptail);
        TaggedRef uvar=makeTaggedRef(newTaggedUVar(home));
        OZ_args[3]=uvar;
        OZ_args[4]=uvar;
    } else {
        // Precaution for the GC?
        OZ_args[3]=makeTaggedNULL();
        OZ_args[4]=makeTaggedNULL();
    }

    // Add the features to L (the tail of the output list)
    TaggedRef arity=OZ_getCArg(2);
    if (!am.unify(fhead,arity)) return FAILED; // No further updating of the suspension
    OZ_args[2]=ftail; // 'ftail' is the new CArg(2) in the suspension

    if (tmptail!=AtomNil) {
        // The record is not determined, so the suspension is revived:
        if (!isKilled) return (SLEEP);
        else return (am.unify(ftail,AtomNil)? PROCEED : FAILED);
    }
    return PROCEED;
}
OZ_C_proc_end // BIpropFeat


// {RecordC.monitorArity X K L} -- builtin that tracks features added to OFS X
// in the list L.  Goes away if K is determined (if K is determined on first call,
// L is list of current features).  monitorArity imposes that X is a record (like
// RecordC) and hence fails if X is not a record.
OZ_C_proc_begin(BImonitorArity, 3)
{
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
	  // Previously:
	  // // *** suspend this call until rec becomes a record
	  // OZ_Suspension susp = OZ_makeSelfSuspension();
	  // OZ_addThread (rec,susp);
	  // return PROCEED;
	  break;
	}
    case CVAR:
        if (tagged2CVar(tmprec)->getType()!=OFSVariable) {
            TypeError1("RecordC.monitorArity",0,"Record",rec);
	    return FAILED;
	}
        // *** arity is calculated from the OFS; see below
        break;
    default:
        TypeError1("RecordC.monitorArity",0,"Record",rec);
        return FAILED;
    }
    tmprec=OZ_getCArg(0);
    DEREF(tmprec,_3,_4);

    // At this point, rec is OFS and tmprec is dereferenced

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

        // Create CFunc suspension BIpropFeat
        // Use two arguments of xRegs to hold the featlist as a dlist
        TaggedRef uvar=makeTaggedRef(newTaggedUVar(home));
        Thread *thr = 
	    createPropagator (BIpropFeat, 5,
			      allocateRegs(rec, kill, feattail, uvar, uvar));
        thr->setOFSThread ();
	thr->suspendPropagator ();
        OZ_addThread (rec, thr);
        OZ_addThread (kill, thr);
    }

    return PROCEED;
}
OZ_C_proc_end // BImonitorArity


/*
 * NOTE: similar functions are dot, genericSet, uparrow
 */
State genericDot(TaggedRef term, TaggedRef fea, TaggedRef *out, Bool dot)
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
      TypeError2("subtree",1,"Feature",term,fea);
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
      if (isSChunk(term)) {
	t = tagged2SChunk(term)->getFeature(fea);
      } else {
	Assert(isObject(term));
	t = tagged2Object(term)->getFeature(fea);
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
  if (dot)   TypeError2(".",0,"Record and no Literal",term,fea);
  TypeError2("subtree",0,"Record",term,fea);
typeError1t:
  if (dot) TypeError2(".",1,"Int (and in range [1 .. width])",term,fea);
  TypeError2("subtree",1,"Int",term,fea);
typeError1r:
  if (dot) TypeError2(".",1,"Feature (and the name of a field)",term,fea);
  TypeError2("subtree",1,"Feature",term,fea);
}


State dotInline(TaggedRef term, TaggedRef fea, TaggedRef &out)
{
  return genericDot(term,fea,&out,TRUE);
}
DECLAREBI_USEINLINEFUN2(BIdot,dotInline)


// !!! second assertion deactivated because of bug in state threading
#define CheckCurObj				\
     Assert(am.getCurrentObject() != NULL);	\
     { Object *o = am.getCurrentObject();	\
       Assert(1 || o->getDeepness()>=1);	\
     }


State atInline(TaggedRef fea, TaggedRef &out)
{
  DEREF(fea, _1, feaTag);

  SRecord *rec = am.getCurrentObject()->getState();
  if (rec) {
    if (!isFeature(fea)) {
      if (isAnyVar(fea)) {
	return SUSPEND;
      }
      goto bomb;
    }
    CheckCurObj;
    TaggedRef t = rec->getFeature(fea);
    if (t) {
      out = t;
      return PROCEED;
    }
  }

bomb:
  TypeError2("@",1,"Feature (and the name of a field)",
	     rec?makeTaggedSRecord(rec):OZ_CToAtom("noattributes"),
	     fea);
}
DECLAREBI_USEINLINEFUN1(BIat,atInline)


State subtreeInline(TaggedRef term, TaggedRef fea, TaggedRef &out)
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
        TypeError2("setC",0,"undetermined record",term,fea);
    }

    // Error unless F is a literal:
    if (!isFeature(feaTag))
        TypeError2("setC",1,"Feature",term,fea);

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
            LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()););
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
        TypeError2("removeC",0,"undetermined record",term,fea);

    // Error unless F is a Feature:
    if (!isFeature(feaTag))
        TypeError2("removeC",1,"Feature",term,fea);

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
        TypeError2("testC",0,"undetermined record",term,fea);

    // Error unless F is a feature:
    if (!isFeature(feaTag))
        TypeError2("testC",1,"Feature",term,fea);

    // At this point, X is OFS and F is feature.
    GenOFSVariable *ofsvar=tagged2GenOFSVar(term);
    // Test presence of feature F:
    if (ofsvar->getFeatureValue(fea)!=makeTaggedNULL())
        return OZ_unify(out,NameTrue);
    else
        return OZ_unify(out,NameFalse);
}
OZ_C_proc_end



/*
 * NOTE: similar functions are dot, genericSet, uparrow
 */
// X^Y=Z: add feature Y to open feature structure X (Tell operation).
State uparrowInline(TaggedRef term, TaggedRef fea, TaggedRef &out)
{
    DEREF(fea,  feaPtr,  feaTag);
    DEREF(term, termPtr, termTag);

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
    if (isNotCVar(feaTag)) return SUSPEND;
    if (isCVar(feaTag)) {
        switch (tagged2CVar(fea)->getType()) {
        case OFSVariable:
	  {
            GenOFSVariable *ofsvar=tagged2GenOFSVar(fea);
            if (ofsvar->getWidth()>0) return FAILED;
            return SUSPEND;
	  }
        default:
            return FAILED;
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
                LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()););
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
    TypeError1("subtreeC",0,"Record",term);
typeError1t:
    TypeError2("subtreeC",0,"Record",term,fea);
typeError2:
    TypeError2("subtreeC",1,"Feature",term,fea);
}
DECLAREBI_USEINLINEFUN2(BIuparrow,uparrowInline)


State hasSubtreeAtInline(TaggedRef term, TaggedRef fea)
{
  return genericDot(term,fea,0,FALSE);
}
DECLAREBI_USEINLINEREL2(BIhasSubtreeAt,hasSubtreeAtInline)
DECLAREBOOLFUN2(BIhasSubtreeAtB,hasSubtreeAtBInline,hasSubtreeAtInline)


State widthInline(TaggedRef term, TaggedRef &out)
{
  DEREF(term,_,tag);

  switch (tag) {
  case LTUPLE:
    out = OZ_CToInt(2);
    return PROCEED;
  case SRECORD:
  record:
    out = OZ_CToInt(tagged2SRecord(term)->getWidth());
    return PROCEED;
  case LITERAL:
    out = OZ_CToInt(0);
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

  TypeError1("width",0,"Record",term);
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


State isBoolInline(TaggedRef t)
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
                  if (sameLiteral(term,NameTrue) ||
		      sameLiteral(term,NameFalse))
                      return SUSPEND;
                  else
                      return FAILED;
	      } else { // isName
		  return FAILED;
	      }
          }
	  return SUSPEND;
	}
      default:
	return FAILED;
      }
  }
  if (sameLiteral(term,NameTrue) || sameLiteral(term,NameFalse))
    return PROCEED;
  else
    return FAILED;
}

DECLAREBI_USEINLINEREL1(BIisBool,isBoolInline)
DECLAREBOOLFUN1(BIisBoolB,isBoolBInline,isBoolInline)


State notInline(TaggedRef A, TaggedRef &out)
{
  NONVAR(A,term,tag);

  if (sameLiteral(term,NameTrue)) {
    out = NameFalse;
    return PROCEED;
  } else {
    if (sameLiteral(term,NameFalse)) {
      out = NameTrue;
      return PROCEED;
    }
  }

  return FAILED;
}

DECLAREBI_USEINLINEFUN1(BInot,notInline)

State andInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  TaggedRef nt = NameTrue;

  if (sameLiteral(A,nt)) {
    if (isAnyVar(B)) {
      return SUSPEND;
    }

    out = sameLiteral(B,nt) ? nt : NameFalse;
    return PROCEED;
  }

  if (isAnyVar(A)) {
    if (isAnyVar(B) || sameLiteral(B,nt)) {
      return SUSPEND;
    }
    out = NameFalse;
    return PROCEED;
  }

  out = NameFalse;
  return PROCEED;  
}

DECLAREBI_USEINLINEFUN2(BIand,andInline)


State orInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  TaggedRef nt = NameTrue;
  
  if (sameLiteral(A,nt)) {
    out = nt;
    return PROCEED;
  }

  if (sameLiteral(B,nt)) {
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

  return OZ_unify(out,OZ_CToString(str));
}
OZ_C_proc_end

OZ_C_proc_begin(BIstringToAtom,2)
{
  OZ_Term in = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  int len=isString(in);
  if (len == -2) {
    TypeError1("stringToAtom",0,"String",in);
    return FAILED;
  }
  if (len == -1) {
    return SUSPEND;
  }
  char *str=stringToC(in,len);

  OZ_Bool ret = OZ_unifyString(out,str);

  delete [] str;
  return ret;
}
OZ_C_proc_end

// ---------------------------------------------------------------------
// Chunk
// ---------------------------------------------------------------------

OZ_C_proc_begin(BInewChunk,2)
{
  OZ_Term vall = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  if (OZ_isVariable(vall)) return OZ_suspendOnVar(vall);
  vall=deref(vall);
  if (!isSRecord(vall)) TypeError1("Chunk.new",0,"Record and not Literal",vall);

  return OZ_unify(out,OZ_newChunk(vall));
}
OZ_C_proc_end

OZ_C_proc_begin(BIchunkArity,2)
{
  OZ_Term ch =  OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  if (OZ_isVariable(ch)) return OZ_suspendOnVar(ch);
  ch=deref(ch);
  if (!OZ_isChunk(ch)) TypeError1("Chunk.arity",0,"Chunk",ch);

  if (isObject(ch)) {
    return OZ_unify(out,tagged2Object(ch)->getArityList());
  }
  Assert(isSChunk(ch));
  return OZ_unify(out,tagged2SChunk(ch)->getRecord()->getArityList());
}
OZ_C_proc_end

// ---------------------------------------------------------------------
// Cell
// ---------------------------------------------------------------------

OZ_C_proc_begin(BInewCell,2)
{
  OZ_Term vall = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  return OZ_unify(out,OZ_newCell(vall));
}
OZ_C_proc_end

State BIexchangeCellInline(TaggedRef c, TaggedRef inState, TaggedRef &outState)
{
  NONVAR(c,rec,_1);
  outState = OZ_newVariable();

  if (!isCell(rec)) {
    TypeError2("exchangeCell",0,"Cell",c,inState);
  }
  Cell *cell = tagged2Cell(rec);

  if (am.currentBoard != cell->getBoardFast()) {
    if (am.currentBoard->isWait()) {
      warning("Built-in exchangeCell: in local computation space of disjunction not impl.");
    } else {
      warning("Built-in exchangeCell: in local computation space of %s",
	      am.currentBoard->isSolve() ? "solve" : "conditional");
    }
    message("Built-in exchangeCell: message sending to a non locally declared object?\n");
    am.currentBoard->incSuspCount();
    return PROCEED;
  }

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
  State state=BIexchangeCellInline(cell,inState,help);
  switch (state) {
  case SUSPEND:
    return OZ_suspendOnVar(cell);
  case FAILED:
    return FAILED;
  case PROCEED:
  default:
    return OZ_unify(help,outState);
  }
}
OZ_C_proc_end

// ---------------------------------------------------------------------

OZ_C_proc_begin(BIsetThreadPriority,1)
{
  OZ_declareIntArg(0,prio);

  if (prio > OZMAX_PRIORITY || prio < OZMIN_PRIORITY) {
    TypeError1("setThreadPriority",0,"Int (0 ... 100)",OZ_getCArg(0));
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

State BItermTypeInline(TaggedRef term, TaggedRef &out)
{
  out = OZ_termType(term);
  if (out == AtomVariable) {
    return SUSPEND;
  }
  return PROCEED;
}

DECLAREBI_USEINLINEFUN1(BItermType,BItermTypeInline)


// ---------------------------------------------------------------------
// Builtins ==, \=, ==B and \=B
// ---------------------------------------------------------------------

inline OZ_Bool eqeqWrapper(TaggedRef Ain, TaggedRef Bin)
{
  TaggedRef A = Ain, B = Bin;
  DEREF(A,aPtr,tagA); DEREF(B,bPtr,tagB);

  /* Really fast test for equality */
  if (tagA != tagB) {
    if (isAnyVar(A) || isAnyVar(B)) goto dontknow;
    return FAILED;
  }

  if (isSmallInt(tagA)) return sameSmallInt(A,B) ? PROCEED : FAILED;
  if (isBigInt(tagA))   return sameBigInt(A,B)   ? PROCEED : FAILED;
  if (isFloat(tagA))    return sameFloat(A,B)    ? PROCEED : FAILED;

  if (isLiteral(tagA))  return sameLiteral(A,B)  ? PROCEED : FAILED;

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

  am.reduceTrailOnShallow();

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



State neqInline(TaggedRef A, TaggedRef B, TaggedRef &out);
State eqeqInline(TaggedRef A, TaggedRef B, TaggedRef &out);


OZ_C_proc_begin(BIneqB,3)
{
  OZ_Term help;
  State ret=neqInline(OZ_getCArg(0),OZ_getCArg(1),help);
  return ret==PROCEED ? OZ_unify(help,OZ_getCArg(2)) : ret;
}
OZ_C_proc_end

OZ_C_proc_begin(BIeqB,3)
{
  OZ_Term help;
  State ret=eqeqInline(OZ_getCArg(0),OZ_getCArg(1),help);
  return ret==PROCEED ? OZ_unify(help,OZ_getCArg(2)): ret;
}
OZ_C_proc_end


State eqeqInline(TaggedRef A, TaggedRef B, TaggedRef &out)
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


State neqInline(TaggedRef A, TaggedRef B, TaggedRef &out)
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

  int len=isString(in);
  if (len==-1) {
    return SUSPEND;
  }
  return (len==-2) ? OZ_unify(out,NameFalse) : OZ_unify(out,NameTrue);
}
OZ_C_proc_end




#define FirstCharArg(NAME)                       \
 int i;                                          \
 OZ_nonvarArg(0);                                \
 if (!OZ_isInt(OZ_getCArg(0))) {                 \
   return OZ_typeError(1,"Int");     \
 } else {                                        \
   i = OZ_intToC(OZ_getCArg(0));                 \
   if ((i < 0) || (i > 255)) {                   \
     return OZ_typeError(1,"Char");  \
   }                                             \
 }

OZ_C_proc_begin(BIcharIs,2) {
  OZ_declareIntArg(0,i);
  return OZ_unify(OZ_getCArg(1),
		  (i>=0 && i <= 255) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsAlNum,2) {
  FirstCharArg("charIsAlNum");
  return OZ_unify(OZ_getCArg(1), isalnum(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsAlpha,2) {
  FirstCharArg("charIsAlpha");
  return OZ_unify(OZ_getCArg(1), isalpha(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsCntrl,2) {
  FirstCharArg("charIsCntrl");
  return OZ_unify(OZ_getCArg(1), iscntrl(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsDigit,2) {
  FirstCharArg("charIsDigit");
  return OZ_unify(OZ_getCArg(1), isdigit(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsGraph,2) {
  FirstCharArg("charIsGraph");
  return OZ_unify(OZ_getCArg(1), isgraph(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsLower,2) {
  FirstCharArg("charIsLower");
  return OZ_unify(OZ_getCArg(1), islower(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsPrint,2) {
  FirstCharArg("charIsPrint");
  return OZ_unify(OZ_getCArg(1), isprint(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsPunct,2) {
  FirstCharArg("charIsPunct");
  return OZ_unify(OZ_getCArg(1), ispunct(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsSpace,2) {
  FirstCharArg("charIsSpace");
  return OZ_unify(OZ_getCArg(1), isspace(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsUpper,2) {
  FirstCharArg("charIsUpper");
  return OZ_unify(OZ_getCArg(1), isupper(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharIsXDigit,2) {
  FirstCharArg("charIsXDigit");
  return OZ_unify(OZ_getCArg(1), isxdigit(i) ? NameTrue : NameFalse);
} OZ_C_proc_end

OZ_C_proc_begin(BIcharToLower,2) {
  FirstCharArg("charToLower");
  return OZ_unifyInt(OZ_getCArg(1), tolower(i));
} OZ_C_proc_end

OZ_C_proc_begin(BIcharToUpper,2) {
  FirstCharArg("charToUpper");
  return OZ_unifyInt(OZ_getCArg(1), toupper(i));
} OZ_C_proc_end


/*
 *	Construct a new SRecord to be a copy of old.
 *	This is the functionality of adjoin(old,newlabel).
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
    TypeError1("copyRecord",0,"Determined Record",rec);
  }
}
OZ_C_proc_end



/*===================================================================
 * Records
 *=================================================================== */

State BIadjoinInline(TaggedRef t0, TaggedRef t1, TaggedRef &out)
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
          TypeError2("adjoin",1,"Record",t0,t1);
      return SUSPEND;
    default:
      TypeError2("adjoin",1,"Record",t0,t1);
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
            TypeError2("adjoin",1,"Record",t0,t1);
        return SUSPEND;
      default:
	TypeError2("adjoin",1,"Record",t0,t1);
      }
    }
  case UVAR:
  case SVAR:
  case CVAR:
    if (tag0==CVAR && tagged2CVar(t0)->getType()!=OFSVariable)
        TypeError2("adjoin",0,"Record",t0,t1);
    switch (tag1) {
    case UVAR:
    case SVAR:
    case SRECORD:
    case LTUPLE:
    case LITERAL:
      return SUSPEND;
    case CVAR:
      if (tagged2CVar(t1)->getType()!=OFSVariable)
	TypeError2("adjoin",1,"Record",t0,t1);
      return SUSPEND;
    default:
      TypeError2("adjoin",1,"Record",t0,t1);
    }
  default:
    TypeError2("adjoin",0,"Record",t0,t1);
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
      return OZ_suspendOnVar(makeTaggedRef(feaPtr));
    }
    if (isCVar(fea)) {
      if (tagged2CVar(fea)->getType()!=OFSVariable ||
          tagged2GenOFSVar(fea)->getWidth()>0)
	TypeError3("adjoinAt",1,"Feature",rec,fea,value);
      return OZ_suspendOnVar(makeTaggedRef(feaPtr));;
    }
    TypeError3("adjoinAt",1,"Feature",rec,fea,value);

  case SRECORD:
    {
      SRecord *rec1 = tagged2SRecord(rec);
      if (isAnyVar(tag1)) {
	return OZ_suspendOnVar(makeTaggedRef(feaPtr));
      }
      if (!isFeature(tag1)) {
	TypeError3("adjoinAt",1,"Feature",rec,fea,value);
      }
      return OZ_unify(out,rec1->adjoinAt(fea,value));
    }

  case UVAR:
  case SVAR:
  case CVAR:
    if (tag0==CVAR && tagged2CVar(rec)->getType()!=OFSVariable)
        TypeError3("adjoinAt",0,"Record",rec,fea,value);
    if (isFeature(fea) || isNotCVar(fea)) {
      return OZ_suspendOnVar(makeTaggedRef(recPtr));
    }
    if (isCVar(fea)) {
      if (tagged2CVar(fea)->getType()!=OFSVariable ||
          tagged2GenOFSVar(fea)->getWidth()>0)
	TypeError3("adjoinAt",1,"Feature",rec,fea,value);
      return OZ_suspendOnVar(makeTaggedRef(recPtr));
    }
    TypeError3("adjoinAt",1,"Feature",rec,fea,value);

  default:
    TypeError3("adjoinAt",0,"Record",rec,fea,value);
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
    if (!isPair(pair)) goto bomb;

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

State adjoinPropListInline(TaggedRef t0, TaggedRef list, TaggedRef &out,
			   Bool recordFlag)
{
  TaggedRef arity=getArity(list);
  if (arity == makeTaggedNULL()) {
    TypeErrorMessage1("adjoinList","incorrect pairlist found",list);
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
      int len=lengthOfList(arity);
      arity = sortlist(arity,len);
      len=lengthOfList(arity); // NOTE: duplicates may be removed
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
    TypeError2("adjoinList",0,"Record",t0,list);
  } else {
    TypeError2("makeRecord",0,"Literal",t0,list);
  }
}

State adjoinPropList(TaggedRef t0, TaggedRef list, TaggedRef &out,
		     Bool recordFlag) 
{
  return adjoinPropListInline(t0,list,out,recordFlag);
}


OZ_C_proc_begin(BIadjoinList,3)
{
  OZ_Term help;

  State state = adjoinPropListInline(OZ_getCArg(0),OZ_getCArg(1),help,OK);
  switch (state) {
  case SUSPEND:
    return OZ_suspendOnVar(help);
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

  State state = adjoinPropListInline(OZ_getCArg(0),OZ_getCArg(1),help,NO);
  switch (state) {
  case SUSPEND:
    return OZ_suspendOnVar(help);
    return PROCEED;
  case PROCEED:
    return(OZ_unify(help,OZ_getCArg(2)));
  default:
    return state;
  }
}
OZ_C_proc_end


State BIarityInline(TaggedRef term, TaggedRef &out)
{
  DEREF(term,termPtr,tag);

  switch (tag) {
  case SRECORD:
    out = tagged2SRecord(term)->getArityList();
    return PROCEED;
  case LTUPLE:
    out = makeTupleArityList(2);
    return PROCEED;
  case LITERAL:
    out = AtomNil;
    return PROCEED;
  case UVAR:
  case SVAR:
    return SUSPEND;
  case CVAR:
    if (tagged2CVar(term)->getType()!=OFSVariable)
        TypeError1("arity",0,"Record",term);
    return SUSPEND;
  default:
    TypeError1("arity",0,"Record",term);
  }
}

DECLAREBI_USEINLINEFUN1(BIarity,BIarityInline)

void assignError(TaggedRef rec, TaggedRef fea, char *name)
{
  prefixError();
  message("Object Error\n");
  message("Assignment (%s) failed: bad attribute or state\n",name);
  message("attribute found  : %s\n", OZ_toC(fea));
  message("state found      : %s\n", OZ_toC(rec));
}


State assignInline(TaggedRef fea, TaggedRef value)
{
  DEREF(fea, _2, feaTag);

  SRecord *r = am.getCurrentObject()->getState();
  if (r) {
    CheckCurObj;
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
  assignError(r?makeTaggedSRecord(r):OZ_CToAtom("noattributes"),
	      fea,"<-");
  return PROCEED;
}

DECLAREBI_USEINLINEREL2(BIassign,assignInline)


/* -----------------------------------------------------------------------
   suspending
   ----------------------------------------------------------------------- */

static State bombBuiltin(char *type)
{
  return OZ_raise(OZ_mkTupleC("typeError",1,
			      OZ_mkTupleC("type",1,OZ_CToAtom(type))));
}

#define suspendTest(A,B,test,type)			\
  if (isAnyVar(A)) {					\
    if (isAnyVar(B) || test(B)) { return SUSPEND; }	\
    return bombBuiltin(type);				\
  } 							\
  if (isAnyVar(B)) {					\
    if (isNumber(A)) { return SUSPEND; }		\
  }							\
  return bombBuiltin(type);


static State suspendOnNumbers(TaggedRef A, TaggedRef B) 
{
  suspendTest(A,B,isNumber,"Number");
}

inline Bool isNumOrAtom(TaggedRef t)
{
  return isNumber(t) || isAtom(t);
}

static State suspendOnNumbersAndAtoms(TaggedRef A, TaggedRef B) 
{
  suspendTest(A,B,isNumOrAtom,"Number or Atom");
}

static State suspendOnFloats(TaggedRef A, TaggedRef B) 
{
  suspendTest(A,B,isFloat,"Float");
}


static State suspendOnInts(TaggedRef A, TaggedRef B) 
{
  suspendTest(A,B,isInt,"integer");
}

#undef suspendTest





/* -----------------------------------
   Z = X op Y
   ----------------------------------- */

// Float x Float -> Float
State BIfdivInline(TaggedRef A, TaggedRef B, TaggedRef &out)
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
#define BIGOP(op)							      \
  if (tagA == BIGINT) {							      \
    if (tagB == BIGINT) {						      \
      out = tagged2BigInt(A)->op(tagged2BigInt(B));			      \
      return PROCEED;							      \
    }									      \
    if (tagB == SMALLINT) {						      \
      BigInt *b = new BigInt(smallIntValue(B));				      \
      out = tagged2BigInt(A)->op(b);					      \
      b->dispose();							      \
      return PROCEED;							      \
    }									      \
  }									      \
  if (tagB == BIGINT) {							      \
    if (tagA == SMALLINT) {						      \
      BigInt *a = new BigInt(smallIntValue(A));				      \
      out = a->op(tagged2BigInt(B));					      \
      a->dispose();							      \
      return PROCEED;							      \
    }									      \
  }

// Integer x Integer -> Integer
State BIdivInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if ((tagB == SMALLINT && smallIntValue(B) == 0)) {
    OZ_warning("div(%s,%s): division by zero",
	       OZ_toC(A),OZ_toC(B));
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
State BImodInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if ((tagB == SMALLINT && smallIntValue(B) == 0)) {
    OZ_warning("mod(%s,0): division by zero",OZ_toC(A));
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


State BImultInline(TaggedRef A, TaggedRef B, TaggedRef &out)
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


State BIminusInline(TaggedRef A, TaggedRef B, TaggedRef &out)
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

State BIplusInline(TaggedRef A, TaggedRef B, TaggedRef &out)
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
State BIuminusInline(TaggedRef A, TaggedRef &out)
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

State BIabsInline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1,tagA);

  if (isSmallInt(tagA)) {
    int i = smallIntValue(A);
    out = (i >= 0) ? A : newSmallInt(-i);
    return PROCEED;
  }

  if (isFloat(tagA)) {
    OZ_Float f = floatValue(A);
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
State BIadd1Inline(TaggedRef A, TaggedRef &out)
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
State BIsub1Inline(TaggedRef A, TaggedRef &out)
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

State bigintLess(BigInt *A, BigInt *B)
{
  return (A->cmp(B) < 0 ? PROCEED : FAILED);
}


State bigintLe(BigInt *A, BigInt *B)
{
  return (A->cmp(B) <= 0 ? PROCEED : FAILED);
}


State bigtest(TaggedRef A, TaggedRef B, State (*test)(BigInt*, BigInt*))
{
  if (isBigInt(A)) {
    if (isBigInt(B)) {
      return test(tagged2BigInt(A),tagged2BigInt(B));
    }
    if (isSmallInt(B)) {
      BigInt *b = new BigInt(smallIntValue(B));
      State res = test(tagged2BigInt(A),b);
      b->dispose();
      return res;
    }
  }
  if (isBigInt(B)) {
    if (isSmallInt(A)) {
      BigInt *a = new BigInt(smallIntValue(A));
      State res = test(a,tagged2BigInt(B));
      a->dispose();
      return res;
    }
  }
  return (isAnyVar(A) || isAnyVar(B)) ? SUSPEND : FAILED;
}




State BIminInline(TaggedRef A, TaggedRef B, TaggedRef &out)
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
      TypeErrorMessage2("min","Names are not ordered",A,B);
      return FAILED;

    default: break;
    }
  }

  State ret = bigtest(A,B,bigintLess);
  if (ret != SUSPEND) {
    out = (ret == PROCEED ? A : B);
    return PROCEED;
  }

  return suspendOnNumbersAndAtoms(A,B);
}


/* code adapted from min */
State BImaxInline(TaggedRef A, TaggedRef B, TaggedRef &out)
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
      TypeErrorMessage2("max","Names are not ordered",A,B);
      return FAILED;

    default: break;
    }
  }

  State ret = bigtest(A,B,bigintLess);
  if (ret != SUSPEND) {
    out = (ret == PROCEED ? B : A);
    return PROCEED;
  }

  return suspendOnNumbersAndAtoms(A,B);
}


State BIlessInline(TaggedRef A, TaggedRef B)
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
      TypeErrorMessage2("less","Names are not ordered",A,B);
      return FAILED;
    }
  }

  State ret = bigtest(A,B,bigintLess); 
  if (ret!=SUSPEND) 
    return ret;

  return suspendOnNumbersAndAtoms(A,B);
}



State BInumeqInline(TaggedRef A, TaggedRef B)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == tagB) {
    if (isSmallInt(tagA)) if (sameSmallInt(A,B)) goto proceed; goto failed;
    if (isFloat(tagA))    if (sameFloat(A,B))    goto proceed; goto failed;
    if (isBigInt(tagA))   if (sameBigInt(A,B))   goto proceed; goto failed;
  }

  return suspendOnNumbers(A,B);

 failed:
  return FAILED;
  
 proceed:
  return PROCEED;
}


State BInumeqInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
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

State BInumneqInline(TaggedRef A, TaggedRef B)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (tagA == tagB) {
    if (isSmallInt(tagA)) if(sameSmallInt(A,B)) goto failed; goto proceed;
    if (isFloat(tagA))    if(sameFloat(A,B))    goto failed; goto proceed;
    if (isBigInt(tagA))   if(sameBigInt(A,B))   goto failed; goto proceed;
  }

  return suspendOnNumbers(A,B);

 failed:
  return FAILED;
  
 proceed:
  return PROCEED;
}


State BInumneqInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  switch (BInumneqInline(A,B)) {
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

State BIlessInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  switch (BIlessInline(A,B)) {
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

State BIgreatInline(TaggedRef A, TaggedRef B)
{
  return BIlessInline(B,A);
}

State BIgreatInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  return BIlessInlineFun(B,A,out);
}


State BIleInline(TaggedRef A, TaggedRef B)
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
      TypeErrorMessage2("le","Names are not ordered",A,B);
      return FAILED;
    }

  }

  State ret = bigtest(A,B,bigintLe); 
  if (ret!=SUSPEND) 
    return ret;

  return suspendOnNumbersAndAtoms(A,B);
}


State BIleInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
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

State BIgeInlineFun(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  return BIleInlineFun(B,A,out);
}


State BIgeInline(TaggedRef A, TaggedRef B)
{
  return BIleInline(B,A);
}

/* -----------------------------------
   X = conv(Y)
   ----------------------------------- */


State BIintToFloatInline(TaggedRef A, TaggedRef &out)
{
  DEREF(A,_1,_2);
  if (isSmallInt(A)) {
    out = makeTaggedFloat((OZ_Float)smallIntValue(A));
    return PROCEED;
  }
  if (isBigInt(A)) {
    char *s = OZ_intToCString(A);
    out = OZ_CStringToFloat(s);
    OZ_free(s);
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

State BIfloatToIntInline(TaggedRef A, TaggedRef &out)
{
  if (OZ_isFloat(A)) {
    double ff = ozround(OZ_floatToC(A));
    if (ff <= OzMaxInt && ff >= OzMinInt) {
      out = makeInt((int) ff);
    } else {
      char *s = OZ_floatToCStringInt(A);
      out = makeTaggedBigInt(new BigInt(s));
      OZ_free(s);
    }
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
    char *s = OZ_floatToCString(in);
    (void) OZ_normFloat(s);
    OZ_Bool ret = OZ_unify(out,OZ_CToString(s));
    OZ_free(s);
    return ret;
  }
  TypeError1("Float.toString",0,"Float",in);
  return FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIstringToFloat, 2)
{
  OZ_Term in = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  int len=isString(in);
  if (len == -2) {
    TypeError1("String.toFloat",0,"String",in);
    return FAILED;
  }
  if (len == -1) {
    return SUSPEND;
  }

  char *str=stringToC(in,len);

  char *end = OZ_parseFloat(str);
  if (!end || *end != 0) {
    OZ_free(str);
    return FAILED;
  }
  OZ_Bool ret = OZ_unify(out,OZ_CStringToFloat(str));
  OZ_free(str);
  return ret;
}
OZ_C_proc_end

OZ_C_proc_begin(BIstringIsFloat, 2)
{
  OZ_Term in = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  int len=isString(in);
  if (len == -2) {
    TypeError1("String.isFloat",0,"String",in);
    return FAILED;
  }
  if (len == -1) {
    return SUSPEND;
  }

  char *str=stringToC(in,len);

  char *end = OZ_parseFloat(str);

  if (!end || *end != 0) {
    OZ_free(str);
    return OZ_unify(out,NameFalse);
  }

  OZ_free(str);
  return OZ_unify(out,NameTrue);
}
OZ_C_proc_end

OZ_C_proc_begin(BIstringToInt, 2)
{
  OZ_Term in = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  int len=isString(in);
  if (len == -2) {
    TypeError1("String.toInt",0,"String",in);
    return FAILED;
  }
  if (len == -1) {
    return SUSPEND;
  }

  char *str=stringToC(in,len);

  char *end = OZ_parseInt(str);
  if (!end || *end != 0) {
    OZ_free(str);
    return FAILED;
  }
  OZ_Bool ret = OZ_unify(out,OZ_CStringToInt(str));
  OZ_free(str);
  return ret;
}
OZ_C_proc_end

OZ_C_proc_begin(BIstringIsInt, 2)
{
  OZ_Term in = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  int len=isString(in);
  if (len == -2) {
    TypeError1("String.isInt",0,"String",in);
    return OZ_unify(out,NameFalse);
  }
  if (len == -1) {
    return SUSPEND;
  }

  char *str=stringToC(in,len);

  char *end = OZ_parseInt(str);

  if (!end || *end != 0) {
    OZ_free(str);
    return OZ_unify(out,NameFalse);
  }

  OZ_free(str);
  return OZ_unify(out,NameTrue);
}
OZ_C_proc_end

OZ_C_proc_begin(BIintToString, 2)
{
  OZ_nonvarArg(0);

  OZ_Term in = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  if (OZ_isInt(in)) {
    char *str = OZ_intToCString(in);
    (void) OZ_normInt(str);
    OZ_Bool ret = OZ_unify(out,OZ_CToString(str));
    OZ_free(str);
    return ret;
  }
  TypeError1("Int.toString",0,"Int",in);
  return FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BInumStrLen, 2)
{
  OZ_nonvarArg(0);

  OZ_Term in = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  int lenn;
  
  if (OZ_isInt(in)) {
    char *str = OZ_intToCString(in);
    lenn = strlen(str);
    OZ_free(str);
  } else if (OZ_isFloat(in)) {
    char *s = OZ_floatToCString(in);
    delChar(s,'+');
    lenn = strlen(s);
    OZ_free(s);
  } else {
    TypeError1("numStrLen",0,"Number",in);
    return FAILED;
  }
  return OZ_unifyInt(out,lenn);
}
OZ_C_proc_end


/* -----------------------------------
   type X
   ----------------------------------- */

State BIisNumberInline(TaggedRef num)
{
  DEREF(num,_,tag);

  if (isAnyVar(tag)) {
    return SUSPEND;
  }

  return isNumber(tag) ? PROCEED : FAILED;
}


State BIisFloatInline(TaggedRef num)
{
  DEREF(num,_,tag);

  if (isAnyVar(tag)) {
    return SUSPEND;
  }

  return isFloat(tag) ? PROCEED : FAILED;
}


State BIisIntInline(TaggedRef num)
{
  DEREF(num,_,tag);

  if (isAnyVar(tag)) {
    return SUSPEND;
  }

  return isInt(tag) ? PROCEED : FAILED;
}



/* -----------------------------------------------------------------------
   misc. floating point functions
   ----------------------------------------------------------------------- */


#define FLOATFUN(Fun,BIName,InlineName)					      \
State InlineName(TaggedRef AA, TaggedRef &out)				      \
{									      \
  DEREF(AA,_,tag);							      \
									      \
  if (isAnyVar(tag)) {							      \
    return SUSPEND;							      \
  }									      \
									      \
  if (isFloat(tag)) {							      \
    out = makeTaggedFloat(Fun(floatValue(AA)));			      \
    return PROCEED;							      \
  }									      \
  return FAILED;							      \
}									      \
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


State BIfPowInline(TaggedRef A, TaggedRef B, TaggedRef &out)
{
  DEREF(A,_1,tagA);
  DEREF(B,_2,tagB);

  if (isFloat(tagA) && isFloat(tagB)) {
    out = makeTaggedFloat(pow(floatValue(A),floatValue(B)));
    return PROCEED;
  }
  return suspendOnFloats(A,B);
}

State BIatan2Inline(TaggedRef A, TaggedRef B, TaggedRef &out)
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

  OZ_Bool ret;
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

DECLAREBI_USEINLINEREL1(BIisNumber,BIisNumberInline)
DECLAREBOOLFUN1(BIisNumberB,BIisNumberBInline,BIisNumberInline)
DECLAREBI_USEINLINEREL1(BIisFloat,BIisFloatInline)
DECLAREBOOLFUN1(BIisFloatB,BIisFloatBInline,BIisFloatInline)
DECLAREBI_USEINLINEREL1(BIisInt,BIisIntInline)
DECLAREBOOLFUN1(BIisIntB,BIisIntBInline,BIisIntInline)

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


/* -----------------------------------------------------------------
   dynamic link objects files
   ----------------------------------------------------------------- */


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
    if (*path == ':' || *path == 0) {

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
      TypeError1("linkObjectFiles",0,"List of Atoms",list);
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

  /*
   * SunOS 4.1
   */
#ifndef RTLD_NOW
# define RTLD_NOW 1
#endif

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
    OZ_warning("linkObjectFiles(%s): can only accept one DLL\n",OZ_toC(list));
    return FAILED;
  }

  if (ozconf.showForeignLoad) {
    message("Linking files\n %s\n",ofiles[0]);
  }

  void *handle = (void *)LoadLibrary(ofiles[0]);
  if (handle==NULL) {
    OZ_warning("failed in linkObjectFiles: %s",GetLastError());
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
#define Link(handle,name) GetProcAddress(handle,name)

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
	       OZ_toC(OZ_getCArg(0)),
	       OZ_toC(OZ_getCArg(1)),
	       OZ_toC(OZ_getCArg(2))
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
	       OZ_toC(OZ_getCArg(0)),
	       OZ_toC(OZ_getCArg(1))
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
  if (t == 0) {
    // OZ_warning("Built-in sleep(0): wakeup immediately");
    return OZ_unify(l,r);
  }
  if (t < 0) {
    OZ_warning("Built-in sleep: negative time %d",t);
    return OZ_unify(l,r);
  }
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

OZ_C_proc_begin(BIsame,2)
{
  return sameTerm(OZ_getCArg(0),OZ_getCArg(1)) ? PROCEED : FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIsameB,3)
{
  OZ_Term ret = sameTerm(OZ_getCArg(0),OZ_getCArg(1)) ? NameTrue : NameFalse;
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
    TypeError1("loadFile",0,"Atom",term0);
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
  
  int len=isList(args,OK);
  if (len == -1) return SUSPEND;
  if (len == -2) {
    TypeError2("apply",1,"List",proc,args);
  }
  RefsArray argsArray = allocateY(len);
  for (int i=0; i < len; i++) {
    argsArray[i] = head(args);
    args=tail(args);
  }
  Assert(OZ_isNil(args));

  DEREF(proc,procPtr,_2);
  if (isAnyVar(proc)) return OZ_suspendOnVar(makeTaggedRef(procPtr));
  if (!isProcedure(proc) && !isObject(proc)) {
    TypeError2("apply",0,"Procedure",proc,args);
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
    TypeError2("deepFeed",0,"Cell",c,val);
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
  State ret = OZ_unify(old,cons(val,newVar));

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
    TypeError1("deepReadCell",0,"Cell",c);
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

extern State subtreeInline(TaggedRef term, TaggedRef fea, TaggedRef &out);

State matchDefaultInline(TaggedRef term, TaggedRef attr, TaggedRef defau,
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
      return OZ_suspendOnVar(atom);
  }    

  OZ_Term modulo = OZ_getCArg(1);
  if (OZ_isVariable(modulo)) {
    return OZ_suspendOnVar(modulo);
  }    

  OZ_Term ret = OZ_getCArg(2);

  if (!OZ_isAtom(atom) || !OZ_isInt(modulo)) {
    TypeErrorMessage2("atomHash","Arguments must be Atom and (small) Int",
		      atom,modulo);
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

OZ_C_proc_begin(BIatomConcat,3)
{
  OZ_declareAtomArg(0,s0);
  OZ_declareAtomArg(1,s1);
  OZ_Term out = OZ_getCArg(2);
  
  char *str = new char[strlen(s0)+strlen(s1)+1];
  sprintf(str,"%s%s",s0,s1);
  
  State ret = OZ_unifyString(out,str);
  
  delete [] str;
  
  return ret;
}
OZ_C_proc_end

OZ_C_proc_begin(BIatomLength,2)
{
  OZ_declareAtomArg(0,a);
  OZ_Term out = OZ_getCArg(1);
  
  return OZ_unifyInt(out,strlen(a));
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
  
  State ret = OZ_unifyString(out,s);
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
      (Thread *) OZ_makeThread (_getsBound_dummy, NULL, 0);
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
      (Thread *) OZ_makeThread (_getsBound_dummyB, OZ_args, OZ_arity);
    SuspList *vcsl = new SuspList (thr, NULL);
    addSuspAnyVar(vPtr, vcsl);
  }

  return (PROCEED);		// no result yet;
}
OZ_C_proc_end

OZ_C_proc_begin(BIintToAtom, 2)
{
  OZ_Term inP = OZ_getCArg(0);
  OZ_Term out = OZ_getCArg(1);

  NONVAR(inP,in,_1);

  if (OZ_isInt(in)) {
    char *str = OZ_intToCString(in);
    (void) OZ_normInt(str);
    OZ_Bool ret = OZ_unifyString(out,str);
    OZ_free(str);
    return ret;
  }
  TypeError1("intToAtom",0,"Int",inP);
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

OZ_C_proc_begin(BIpushExHdl,1)
{
  OZ_Term pred = OZ_getCArg(0);
  DEREF(pred,_1,_2);
  if (!isProcedure(pred)) {
    TypeErrorT(0,"Procedure");
  }
  am.currentThread->pushExceptionHandler(pred);
  return PROCEED;
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
    warning("setAbstractionTabDefaultEntry: abstraction expected: %s",OZ_toC(in));
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
State printInline(TaggedRef term)
{
  taggedPrint(term,ozconf.printDepth);
  fflush(stdout);
  return (PROCEED);
}

DECLAREBI_USEINLINEREL1(BIprint,printInline)

OZ_C_proc_begin(BIprintVS,1)
{
  OZ_Term t=OZ_getCArg(0);
  OZ_printVS(t);
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BItermToVS,2)
{
  OZ_Term t=OZ_getCArg(0);
  OZ_Term out=OZ_getCArg(1);
  return OZ_unify(out,OZ_termToVS(t));
}
OZ_C_proc_end

State showInline(TaggedRef term)
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
  OZ_Term t = OZ_tuple(OZ_CToAtom("globals"),n);
  for (int i = 0; i < n; i++) {
    OZ_putArg(t,i+1,gRegs[i]);
  }
  return t;
}

inline
OZ_Term NULL2NIL(OZ_Term t)
{
  return t==makeTaggedNULL() ? OZ_nil() : t;
}

State taggedGetValue(TaggedRef term,TaggedRef feat,TaggedRef out)
{
  DEREF(term,termPtr,tag);
  DEREF(feat,_1,fTag);
  if (!OZ_isAtom(feat)) {
    TypeError2("getValue",1,"Literal",term,feat);
  }

  char *feature = tagged2Literal(feat)->getPrintName();
  
  STRCASE("addr",
	  isAnyVar(tag) ? ToInt32(termPtr) : ToInt32(tagValueOf(tag,term)),
	  OZ_unifyInt);
  STRCASE("isvar",
	  isAnyVar(tag) ? "ok" : "no",
	  OZ_unifyString);
  STRCASE("lengthSuspList",
	  isCVar(tag) ? tagged2CVar(term)->getSuspListLength() :
	  (isSVar(tag) ? tagged2SVar(term)->getSuspList()->length()
	   : (isUVar(tag) ? 0 : -1)),
	  OZ_unifyInt);

  STRCASE("name",
	  tagged2String(term,ozconf.printDepth),
	  OZ_unifyString);

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
	  OZ_toC(term),OZ_toC(feat));
  return FAILED;
}

OZ_C_proc_begin(BIgetArgv,1)
{
  TaggedRef out = nil();
  for(int i=ozconf.argC-1; i>=0; i--) {
    out = cons(OZ_CToAtom(ozconf.argV[i]),out);
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
      case Co_Object:  	   return OZ_unifyString(out, ((Object*) rec)->getPrintName());
      case Co_Cell: 	   return OZ_unifyString(out,"_");	
      default:    	   break;
      }
    }
    break;

  case UVAR:    return OZ_unifyString(out, "_");
  case SVAR:
  case CVAR:    return OZ_unify(out, VariableNamer::getName(OZ_getCArg(0)));
  case LITERAL: return OZ_unifyString(out, tagged2Literal(term)->getPrintName());

  default:      break;
  }

  return OZ_unifyString(out, tagged2String(term,ozconf.printDepth));
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

  Thread *thr = getElem ();
  Board *b = thr->getBoardFast ();
  return cons(makeTaggedConst(b),getNext()->DBGmakeList());
}

State AM::getValue(TaggedRef feat, TaggedRef out)
{
  DEREF(feat,_1,fTag);
  if (!OZ_isAtom(feat)) {
    TypeError1("getValue",0,"Literal",feat);
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
  STRCASE("isvar",          "no",OZ_unifyString);

  STRCASE("cellHack",          ozconf.cellHack,                 OZ_unifyInt);
  STRCASE("showSolveFailure",  ozconf.showSolveFailure,         OZ_unifyInt);
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

  warning("getValue(0,%s): bad feature", OZ_toC(feat));
  return FAILED;
}


OZ_C_proc_begin(BIgetValue,3)
{
  OZ_Term term = OZ_getCArg(0);
  OZ_Term fea = OZ_getCArg(1);
  OZ_Term out = OZ_getCArg(2);

  OZ_nonvarArg(1);
  NONVAR(fea,feat,ftag);

  if (!isLiteral(ftag)) {
    TypeError2("getValue",1,"Literal",term,feat);
  }

  char *feature = tagged2Literal(feat)->getPrintName();

  if (OZ_isInt(term)) {  // AM
    DEREF(term,_1,_2);
    STRCASE("addr",ToInt32(tagValueOf(term)),OZ_unifyInt);
    STRCASE("name", tagged2String(term,ozconf.printDepth),
	    OZ_unifyString);
    STRCASE("printname", tagged2String(term,ozconf.printDepth),
	    OZ_unifyString);
    return am.getValue(feat,out);
  }

  return taggedGetValue(term,feat,out);
}
OZ_C_proc_end

#undef STRCASE

// --------------------------------------------------------------------------
// SETVALUE
// --------------------------------------------------------------------------

#define DOIF(str,body) \
     if (feature == OZ_CToAtom(str)) { \
       body \
       return PROCEED; \
     }

State AM::setValue(TaggedRef feature, TaggedRef value)
{
  if (!OZ_isInt(value)) {
    TypeError1("setValue",0,"Int",feature);
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
  DOIF("showSolveFailure",
       ozconf.showSolveFailure = val;
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
  warning("setValue(0,%s): bad feature",OZ_toC(feature));
  return FAILED;
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
    TypeError2("setValue",1,"Literal",tmpTerm,fea);
  }

  if (OZ_isInt(term)) {  // AM
    return am.setValue(feature,value);
  }

  TypeError2("setValue",0,"Const",term,feature);
}
OZ_C_proc_end

#undef DOIF

// ---------------------------------------------------------------------------
// Debugging: set a break
// ---------------------------------------------------------------------------

OZ_C_proc_begin(BIhalt,0)
{
  tracerOn();
  return PROCEED;
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
    TypeError1("idToTerm",0,"Int",term);
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

OZ_C_proc_begin(BItraceBack, 1)
{
  OZ_declareIntArg(0,depth)
  am.currentThread->printLong(cout,depth,0);
  return PROCEED;
}
OZ_C_proc_end




OZ_C_proc_begin(BIplatform, 1)
{
  OZ_Term ret = OZ_pair(OZ_CToAtom(ozconf.osname),OZ_CToAtom(ozconf.cpu));
  return OZ_unify(ret,OZ_getCArg(0));
}
OZ_C_proc_end


OZ_C_proc_begin(BIozhome, 1)
{
  return OZ_unifyString(OZ_getCArg(0),ozconf.ozHome);
}
OZ_C_proc_end

// ---------------------------------------------------------------------------

OZ_C_proc_proto(ozparser_parse)
OZ_C_proc_proto(ozparser_init)
OZ_C_proc_proto(ozparser_exit)


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
    warning("makeClass: record expected: %s", OZ_toC(fastmeth));
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
		OZ_toC(abstr));
	return FAILED;
      }
      methods->setArg(i,abstr);
    }
  }

  if (!isLiteral(printname)) {
    warning("makeClass: literal expected: %s", OZ_toC(printname));
    return FAILED;
  }

  if (!isAbstraction(send)) {
    warning("makeClass: abstraction expected: %s", OZ_toC(send));
    return FAILED;
  }

  if (!isRecord(features)) {
    warning("makeClass: record expected: %s", OZ_toC(features));
    return FAILED;
  }

  if (!isRecord(ufeatures)) {
    warning("makeClass: record expected: %s", OZ_toC(ufeatures));
    return FAILED;
  }

  SRecord *uf = isSRecord(ufeatures) ? tagged2SRecord(ufeatures) : (SRecord*)NULL;

  ObjectClass *cl = new ObjectClass(methods,
				    tagged2Literal(printname),
				    slowmeth,
				    tagged2Abstraction(send),
				    sameLiteral(hfb,NameTrue),
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


OZ_C_proc_begin(BImakeObject,4)
{
  OZ_Term initState = OZ_getCArg(0); { DEREF(initState,_1,_2); }
  OZ_Term ffeatures = OZ_getCArg(1); { DEREF(ffeatures,_1,_2); }
  OZ_Term clas      = OZ_getCArg(2); { DEREF(clas,_1,_2); }
  OZ_Term obj       = OZ_getCArg(3);

  if (!isRecord(initState)) {
    warning("makeObject: record expected: %s", OZ_toC(initState));
    return FAILED;
  }

  if (!isRecord(ffeatures)) {
    warning("makeObject: record expected: %s", OZ_toC(ffeatures));
    return FAILED;
  }

  if (!isObject(clas)) {
    warning("makeObject: class expected: %s", OZ_toC(clas));
    return FAILED;
  }

  Object *out = newObject(tagged2SRecord(ffeatures),
			  isSRecord(initState) ? tagged2SRecord(initState) : NULL,
			  ((Object*)tagged2Const(clas))->getClass(),
			  NO,
			  am.currentBoard);
  return OZ_unify(obj,makeTaggedConst(out));
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
    warning("setMethApplHdl called twice (hint: prelude may not be fed twice)");
  }
  return PROCEED;
}
OZ_C_proc_end


State hasFastBatchInline(TaggedRef t)
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


State BIisObjectInline(TaggedRef t)
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

State BIisClassInline(TaggedRef t)
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
State getClassInline(TaggedRef t, TaggedRef &out)
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


OZ_C_proc_begin(BIsetClosed,1)
{
  OZ_Term obj=OZ_getCArg(0);
  OZ_Term out=OZ_getCArg(1);

  DEREF(obj,objPtr,_2);
  if (isAnyVar(obj)) return OZ_suspendOnVar(makeTaggedRef(objPtr));
  if (!isObject(obj)) return FAILED;
  Object *oo = (Object *)tagged2Const(obj);
  oo->close();
  return PROCEED;
}
OZ_C_proc_end


State objectIsFreeInline(TaggedRef tobj, TaggedRef &out)
{
  DEREF(tobj,_1,_2);  
  Assert(isObject(tobj));

  Object *obj = (Object *) tagged2Const(tobj);

  if (am.currentBoard != obj->getBoardFast()) {
    warning("object application(%s): in local computation space not allowed",
	    OZ_toC(tobj));
    am.currentBoard->incSuspCount();
    am.currentThread->printTaskStack(NOCODE);
    return FAILED;
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
  am.getCurrentObject()->release();
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIgetSelf,1)
{
  return OZ_unify(makeTaggedConst(am.getCurrentObject()),
		  OZ_getCArg(0));
}
OZ_C_proc_end


OZ_C_proc_begin(BIsetSelf,1)
{
  TaggedRef o = OZ_getCArg(0);
  DEREF(o,_1,_2);
  if (!isObject(o)) {
    OZ_warning("setSelf(%s): object expected",OZ_toC(OZ_getCArg(0)));
    return FAILED;
  }

  Object *obj = (Object *) tagged2Const(o);
  /* same code as in emulate.cc !!!!! */
  if (am.getCurrentObject()!=obj) {
    am.currentThread->pushSetCurObject(am.getCurrentObject());
    am.setCurrentObject(obj);
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


/* cloneObjectRecord(in,nocopy,varOnHeap,out):
 *      "out" is a copy of record "in", except that for those arguments
 *     in "in" which are equal to literal "nocopy", a new Variable is created
 *     this new var is created on the heap iff varOnHeap==True
 */
OZ_C_proc_begin(BIcloneObjectRecord,4)
{
  OZ_Term record    = OZ_getCArg(0); DEREF(record,_1,_2);
  OZ_Term nocopy    = OZ_getCArg(1); DEREF(nocopy,_3,_4);
  OZ_Term varOnHeap = OZ_getCArg(2); DEREF(varOnHeap,_5,_6);
  OZ_Term out       = OZ_getCArg(3);

  if (isLiteral(record)) {
    return OZ_unify(record,out);
  }

  if (!isSRecord(record)) {
    warning("BIcloneObjectRecord: record expected: %s", OZ_toC(record));
    return FAILED;
  }
  if (!isLiteral(nocopy)) {
    warning("BIcloneObjectRecord: literal expected: %s", OZ_toC(nocopy));
    return FAILED;
  }

  SRecord *in  = tagged2SRecord(record);
  SRecord *rec = SRecord::newSRecord(in);

  OZ_Term proto = am.currentUVarPrototype;
  Assert(isLiteral(varOnHeap));
  TaggedRef newvar = proto;
  for(int i=0; i < in->getWidth(); i++) {
    OZ_Term arg = in->getArg(i);
    if (sameLiteral(nocopy,deref(arg))) {
      if (sameLiteral(NameTrue,varOnHeap)) {
	newvar = makeTaggedRef(newTaggedUVar(proto));
      }
      rec->setArg(i,newvar);
    } else {
      rec->setArg(i,arg);
    }
  }

  return OZ_unify(makeTaggedSRecord(rec),out);
}
OZ_C_proc_end

/*===================================================================
 * Table of builtins
 *=================================================================== */


static
BIspec allSpec[] = {
  {"isValue",1,BIisValue,         NO, (IFOR) isValueInline},
  {"isValueB",2,BIisValueB,       NO, (IFOR) isValueBInline},
  {"isLiteral",1,BIisLiteral,     NO, (IFOR) isLiteralInline},
  {"isLiteralB",2,BIisLiteralB,   NO, (IFOR) isLiteralBInline},
  {"isAtom",1,BIisAtom,           NO, (IFOR) isAtomInline},
  {"isAtomB",2,BIisAtomB,         NO, (IFOR) isAtomBInline},
  {"isName",1,BIisName,           NO, (IFOR) isNameInline},
  {"isNameB",2,BIisNameB,         NO, (IFOR) isNameBInline},
  {"isTuple",1,BIisTuple,         NO, (IFOR) isTupleInline},
  {"isTupleB",2,BIisTupleB,       NO, (IFOR) isTupleBInline},
  {"isRecord",1,BIisRecord,       NO, (IFOR) isRecordInline},
  {"isRecordB",2,BIisRecordB,     NO, (IFOR) isRecordBInline},
  {"isRecordC",1,BIisRecordC,     NO, (IFOR) isRecordCInline},
  {"isRecordCB",2,BIisRecordCB,   NO, (IFOR) isRecordCBInline},
  {"isProcedure",1,BIisProcedure, NO, (IFOR) isProcedureInline},
  {"isProcedureB",2,BIisProcedureB,NO,(IFOR) isProcedureBInline},
  {"Chunk.is",2,BIisChunk,        NO, (IFOR) isChunkInline},
  {"isNary",2,BIisNary,           NO, (IFOR) isNaryInline},
  {"isNaryB",3,BIisNaryB,         NO, (IFOR) isNaryBInline},
  {"isCell",1,BIisCell,           NO, (IFOR) isCellInline},
  {"isCellB",2,BIisCellB,         NO, (IFOR) isCellBInline},

  {"isVar",1,BIisVar,             NO, (IFOR) isVarInline},
  {"isVarB",2,BIisVarB,           NO, (IFOR) isVarBInline},
  {"isNonvar",1,BIisNonvar,       NO, (IFOR) isNonvarInline},
  {"isNonvarB",2,BIisNonvarB,     NO, (IFOR) isNonvarBInline},

  {"isString",2,BIisString,       NO, 0},

  {"getTrue", 1,BIgetTrue,  	  NO, 0},
  {"getFalse",1,BIgetFalse,  	  NO, 0},
  {"isBool",  1,BIisBool,         NO, (IFOR) isBoolInline},
  {"isBoolB", 2,BIisBoolB,        NO, (IFOR) isBoolBInline},
  {"not",     2,BInot,            NO, (IFOR) notInline},
  {"and",     3,BIand,            NO, (IFOR) andInline},
  {"or",      3,BIor,             	NO, (IFOR) orInline},

  {"termType",2,BItermType,       	NO, (IFOR) BItermTypeInline},

  {"procedureArity",2,BIprocedureArity,	NO, (IFOR)procedureArityInline},
  {"cloneProcedure",2,BIcloneProcedure, NO, 0},
  {"tuple",3,BItuple,             NO, (IFOR) tupleInline},
  {"label",2,BIlabel,             NO, (IFOR) labelInline},

  {"recordC",      1, BIrecordC,	NO,0},
  {"recordCSize",  2, BIrecordCSize,	NO,0},
  {"labelC",       2, BIlabelC,		NO,0},
  {"widthC",       2, BIwidthC,		NO,0},
  {"recordCIsVar", 1, BIisRecordCVar,	NO,0},
  {"recordCIsVarB",2, BIisRecordCVarB,	NO,0},
  {"setC",         3, BIsetC,		NO,0},
  {"removeC",      2, BIremoveC,	NO,0},
  {"testCB",       3, BItestCB,		NO,0},
  {"propWidth",    2, BIpropWidth,	NO,0},
  {"monitorArity", 3, BImonitorArity,	NO,0},
  {"propFeat",     5, BIpropFeat,	NO,0},
  {".",            3,BIdot,             NO, (IFOR) dotInline},
  {"subtree", 	   3,BIsubtree,        	NO, (IFOR) subtreeInline},
  {"^",            3,BIuparrow,        	NO, (IFOR) uparrowInline},
  {"subtreeC",     3,BIuparrow,        	NO, (IFOR) uparrowInline},

  {"hasSubtreeAt", 2,BIhasSubtreeAt,    NO,(IFOR)hasSubtreeAtInline},
  {"hasSubtreeAtB",3,BIhasSubtreeAtB,   NO,(IFOR)hasSubtreeAtBInline},
  {"width",        2,BIwidth,           NO, (IFOR) widthInline},

  {"atomToString",    2, BIatomToString,	NO,0},
  {"stringToAtom",    2, BIstringToAtom,	NO,0},

  {"Chunk.new",	      2,BInewChunk,	NO,0},
  {"`ChunkArity`",    2,BIchunkArity,	NO,0},

  {"newCell",	      2,BInewCell,	NO,0},
  {"exchangeCell",    3,BIexchangeCell, NO,(IFOR) BIexchangeCellInline},

  {"newName",         1,BInewName,	NO,0},

  {"setThreadPriority", 1, BIsetThreadPriority,	NO,0},
  {"getThreadPriority", 1, BIgetThreadPriority,	NO,0},

  {"==B",  3,BIeqB,   NO, (IFOR) eqeqInline},
  {"\\=B", 3,BIneqB,  NO, (IFOR) neqInline},
  {"==",   2,BIeq,    NO, 0},
  {"\\=",  2,BIneq,   NO, 0},

  {"charIs",      2, BIcharIs,		NO,0},
  {"charIsAlNum", 2, BIcharIsAlNum,	NO,0},
  {"charIsAlpha", 2, BIcharIsAlpha,	NO,0},
  {"charIsCntrl", 2, BIcharIsCntrl,	NO,0},
  {"charIsDigit", 2, BIcharIsDigit,	NO,0},
  {"charIsGraph", 2, BIcharIsGraph,	NO,0},
  {"charIsLower", 2, BIcharIsLower,	NO,0},
  {"charIsPrint", 2, BIcharIsPrint,	NO,0},
  {"charIsPunct", 2, BIcharIsPunct,	NO,0},
  {"charIsSpace", 2, BIcharIsSpace,	NO,0},
  {"charIsUpper", 2, BIcharIsUpper,	NO,0},
  {"charIsXDigit", 2, BIcharIsXDigit,	NO,0},
  {"charToLower", 2, BIcharToLower,	NO,0},
  {"charToUpper", 2, BIcharToUpper,	NO,0},
  {"adjoin",          3,BIadjoin,          NO, (IFOR) BIadjoinInline},
  {"adjoinList",      3,BIadjoinList,      NO, 0},
  {"record",          3,BImakeRecord,      NO, 0},
  {"makeRecord",      3,BImakeRecord,      NO, 0},
  {"arity",           2,BIarity,           NO, (IFOR) BIarityInline},
  {"adjoinAt",        4,BIadjoinAt,        NO, 0},
  {"@",               2,BIat,              NO, (IFOR) atInline},
  {"<-",              2,BIassign,          NO, (IFOR) assignInline},
  {"copyRecord",      2,BIcopyRecord,      NO, 0},
  {"/",  3,BIfdiv,   NO, (IFOR) BIfdivInline},
  {"*",  3,BImult,   NO, (IFOR) BImultInline},
  {"div",3,BIdiv,    NO, (IFOR) BIdivInline},
  {"mod",3,BImod,    NO, (IFOR) BImodInline},
  {"-",  3,BIminus,  NO, (IFOR) BIminusInline},
  {"+",  3,BIplus,   NO, (IFOR) BIplusInline},
  
  {"max", 3,BImax,     NO, (IFOR) BImaxInline},
  {"min", 3,BImin,     NO, (IFOR) BIminInline},
  
  {"<B", 3,BIlessFun,     NO, (IFOR) BIlessInlineFun},
  {"=<B",3,BIleFun,       NO, (IFOR) BIleInlineFun},
  {">B", 3,BIgreatFun,    NO, (IFOR) BIgreatInlineFun},
  {">=B",3,BIgeFun,       NO, (IFOR) BIgeInlineFun},
  {"=:=B",2,BInumeqFun,   NO, (IFOR) BInumeqInlineFun},
  {"=\\=B",2,BInumneqFun, NO, (IFOR) BInumneqInlineFun},
  
  {"=<",2,BIle,      NO, (IFOR) BIleInline},
  {"<",2,BIless,     NO, (IFOR) BIlessInline},
  {">=",2,BIge,      NO, (IFOR) BIgeInline},
  {">",2,BIgreat,    NO, (IFOR) BIgreatInline},
  {"=:=",2,BInumeq,  NO, (IFOR) BInumeqInline},
  {"=\\=",2,BInumneq, NO, (IFOR) BInumneqInline},
  
  {"~",2,BIuminus,   NO, (IFOR) BIuminusInline},
  {"+1",2,BIadd1,    NO, (IFOR) BIadd1Inline},
  {"-1",2,BIsub1,    NO, (IFOR) BIsub1Inline},
  
  {"isNumber",1,BIisNumber,   NO, (IFOR) BIisNumberInline},
  {"isInt"   ,1,BIisInt,      NO, (IFOR) BIisIntInline},
  {"isFloat" ,1,BIisFloat,    NO, (IFOR) BIisFloatInline},
  {"isNumberB",2,BIisNumberB, NO, (IFOR) BIisNumberBInline},
  {"isIntB"   ,2,BIisIntB,    NO, (IFOR) BIisIntBInline},
  {"isFloatB" ,2,BIisFloatB,  NO, (IFOR) BIisFloatBInline},
  
  {"exp",  2, BIexp,  NO, (IFOR) BIinlineExp},
  {"log",  2, BIlog,  NO, (IFOR) BIinlineLog},
  {"sqrt", 2, BIsqrt, NO, (IFOR) BIinlineSqrt},
  {"sin",  2, BIsin,  NO, (IFOR) BIinlineSin},
  {"asin", 2, BIasin, NO, (IFOR) BIinlineAsin},
  {"cos",  2, BIcos,  NO, (IFOR) BIinlineCos},
  {"acos", 2, BIacos, NO, (IFOR) BIinlineAcos},
  {"tan",  2, BItan,  NO, (IFOR) BIinlineTan},
  {"atan", 2, BIatan, NO, (IFOR) BIinlineAtan},
  {"ceil", 2, BIceil, NO, (IFOR) BIinlineCeil},
  {"floor",2, BIfloor,NO, (IFOR) BIinlineFloor},
  {"fabs", 2, BIfabs, NO, (IFOR) BIinlineFabs},
  {"round",2, BIround,NO, (IFOR) BIinlineRound},
  {"abs",  2, BIabs,  NO, (IFOR) BIabsInline},

  {"fPow",3,BIfPow,NO, (IFOR) BIfPowInline},
  {"atan2",3,BIatan2,NO, (IFOR) BIatan2Inline},

  /* what is a small int ? */
  {"smallIntLimits", 2, BIsmallIntLimits, NO,0},

  /* conversion: float <-> int <-> virtualStrings */
  {"intToFloat",2,BIintToFloat, NO, (IFOR) BIintToFloatInline},
  {"floatToInt",2,BIfloatToInt, NO, (IFOR) BIfloatToIntInline},

  {"numStrLen",   2, BInumStrLen,		NO,0},

  {"intToString",    2, BIintToString,		NO,0},
  {"floatToString",  2, BIfloatToString,	NO,0},
  {"stringToInt",    2, BIstringToInt,		NO,0},
  {"stringToFloat",  2, BIstringToFloat,	NO,0},
  {"stringIsInt",    2, BIstringIsInt,		NO,0},
  {"stringIsFloat",  2, BIstringIsFloat,	NO,0},
  {"loadFile",       1, BIloadFile,		NO,0},

  {"linkObjectFiles",2, BIlinkObjectFiles,	NO,0},
  {"unlinkObjectFile",1,BIunlinkObjectFile,	NO,0},
  {"findFunction",   3, BIfindFunction,		NO,0},
  {"shutdown",       0, BIshutdown,		NO,0},

  {"sleep",          3, BIsleep,		NO,0},

  {"garbageCollection",0,BIgarbageCollection,	NO,0},

  {"apply",          2, BIapply,		NO,0},

  {"eq",             2, BIsame,		        NO,0},
  {"eqB",            3, BIsameB,		NO,0},

  {"=",              2, BIunify,		NO,0},
  {"fail",           VarArity,BIfail,		NO,0},

  {"deepReadCell",   2, BIdeepReadCell,		NO,0},
  {"deepFeed",       2, BIdeepFeed,		NO,0},

  {"genericSet",     3, BIgenericSet,		NO,0},

  {"atomHash",       3, BIatomHash,		NO,0},

  {"matchDefault",   4, BImatchDefault,         NO,(IFOR) matchDefaultInline},

  {"atomConcat",     3, BIatomConcat,		NO,0},
  {"atomLength",     2, BIatomLength,		NO,0},

  {"gensym",         2, BIgensym,		NO,0},

  {"getsBound",      1, BIgetsBound,		NO,0},
  {"getsBoundB",     2, BIgetsBoundB,		NO,0},
  {"intToAtom",      2, BIintToAtom,		NO,0},

  {"connectLingRef", 1, BIconnectLingRef,	NO,0},
  {"getLingRefFd",   1, BIgetLingRefFd,		NO,0},
  {"getLingEof",     1, BIgetLingEof,		NO,0},
  {"getOzEof",       1, BIgetLingEof,		NO,0},
  {"constraints",    2, BIconstraints,		NO,0},

  {"pushExHdl",      1, BIpushExHdl,		NO,0},

  {"setAbstractionTabDefaultEntry", 1, BIsetAbstractionTabDefaultEntry, NO,0},

  {"usertime",1,BIusertime},
  {"memory",1,BImemory},
  {"isStandalone",1,BIisStandalone},
  {"showBuiltins",0,BIshowBuiltins},
  {"print",1,BIprint, NO, (IFOR) printInline},
  {"printError",1,BIprintError},
  {"show",1,BIshow, NO, (IFOR) showInline},

  {"getValue",3,BIgetValue},
  {"setValue",3,BIsetValue},
  {"getArgv", 1,BIgetArgv},

  {"halt",0,BIhalt},

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
  {"traceBack",1,BItraceBack},

  {"ozparser_parse",2,ozparser_parse},
  {"ozparser_init",0,ozparser_init},
  {"ozparser_exit",0,ozparser_exit},

  {"printVS",1,BIprintVS},
  {"termToVS",2,BItermToVS},

  {"dumpThreads",0,BIdumpThreads},

  {"platform",1,BIplatform},
  {"ozhome",1,BIozhome},

  {"makeClass",        8,BImakeClass,	       NO,0},
  {"makeObject",       4,BImakeObject,	       NO,0},
  {"cloneObjectRecord",4,BIcloneObjectRecord,  NO,0},
  {"setModeToDeep",    0,BIsetModeToDeep,  NO,0},
  {"setMethApplHdl",   1,BIsetMethApplHdl,     NO,0},
  {"getClass",         2,BIgetClass, 	       NO,(IFOR) getClassInline},
  {"hasFastBatch",     1,BIhasFastBatch,       NO,(IFOR) hasFastBatchInline},
  {"objectIsFree",     2,BIobjectIsFree,       NO,(IFOR) objectIsFreeInline},
  {"releaseObject",    0,BIreleaseObject,      NO,0},
  {"getSelf",          1,BIgetSelf,            NO,0},
  {"setSelf",          1,BIsetSelf,            NO,0},
  {"Object.is",        2,BIisObjectB, 	       NO,(IFOR) BIisObjectBInline},
  {"isClass",          1,BIisClass,   	       NO,(IFOR) BIisClassInline},
  {"isClassB",         2,BIisClassB,	       NO,(IFOR) BIisClassBInline},
  {"setClosed",        1,BIsetClosed,          NO,0},

  {0,0,0,0,0}
};



extern void BIinitFD(void);
extern void BIinitMeta(void);
extern void BIinitAVar(void);
extern void BIinitUnix();
extern void BIinitAssembler();
extern void BIinitTclTk();

BuiltinTabEntry *BIinit()
{
  BuiltinTabEntry *bi = BIadd("builtin",3,BIbuiltin);

  if (!bi)
    return bi;

  BIaddSpec(allSpec);

  /* see emulate.cc */
  BIaddSpecial("solve",             3, BIsolve);
  BIaddSpecial("solveEatWait",      3, BIsolveEatWait);
  BIaddSpecial("solveDebug",        3, BIsolveDebug);
  BIaddSpecial("solveDebugEatWait", 3, BIsolveDebugEatWait);
  BIaddSpecial("raise",             1, BIraise);

#ifdef ASSEMBLER
  BIinitAssembler();
#endif

  BIinitFD();
  BIinitMeta();

  BIinitAVar();
  BIinitUnix();
  BIinitTclTk();

  return bi;
}
