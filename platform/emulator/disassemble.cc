/*
 *  Author:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 2000
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

#include "base.hh"
#include "codearea.hh"
#include "indexing.hh"
#include "builtins.hh"

TaggedRef _DA_AtomTab[10];

const char * _DA_CharTab[] = {
  "x",			        // 0
  "y",				// 1
  "g",				// 2
  "l",			        // 3
  "cache",                      // 4
  "dummy",                      // 5
  "iht",                        // 6
  "t",				// 7
};

#define DA_x               			_DA_AtomTab[0]
#define DA_y    				_DA_AtomTab[1]
#define DA_g      				_DA_AtomTab[2]
#define DA_l             			_DA_AtomTab[3]
#define DA_cache                  		_DA_AtomTab[4]
#define DA_dummy                  		_DA_AtomTab[5]
#define DA_iht                  		_DA_AtomTab[6]
#define DA_t                  		        _DA_AtomTab[7]

void disassembler_init(void) {
  for (int i = 8; i--; )
    _DA_AtomTab[i] = oz_atomNoDup(_DA_CharTab[i]);
}

#define DISASS_SETUP(N) \
if (N==0) {                                  \
  *head = OPA;                               \
} else {                                     \
  SRecord * sr = SRecord::newSRecord(OPA,N); \
  *head = makeTaggedSRecord(sr);             \
  SP = sr->getRef();                         \
}

#define SETSP(ARG) *SP++ = (ARG)

#define SETSP_TUPLE1(lbl,arg) \
{ SRecord * sr = SRecord::newSRecord(lbl,1); \
  sr->setArg(0,arg);                         \
  SETSP(makeTaggedSRecord(sr));              \
}

#define DISASS_XREGISTERINDEX(N) \
  SETSP_TUPLE1(DA_x,makeTaggedSmallInt(XRegToInt((XReg) *(PC+N))))
#define DISASS_YREGISTERINDEX(N) \
  SETSP_TUPLE1(DA_y,makeTaggedSmallInt(YRegToInt((YReg) *(PC+N))))
#define DISASS_GREGISTERINDEX(N) \
  SETSP_TUPLE1(DA_g,makeTaggedSmallInt(GRegToInt((GReg) *(PC+N))))

#define DISASS_TAGGED(N) \
  SETSP_TUPLE1(DA_t,getTaggedArg(PC+N))
#define DISASS_DUMMY(N) \
  SETSP(DA_dummy)
#define DISASS_LABEL(N) \
  SETSP_TUPLE1(DA_l,makeTaggedSmallInt(getLabelArg(PC+N)))
#define DISASS_ARITY(N) \
  SETSP(makeTaggedSmallInt(getPosIntArg(PC+N)));
#define DISASS_COUNT(N) \
  DISASS_ARITY(N)
#define DISASS_CACHE(N) \
  SETSP(DA_cache); SETSP(DA_cache);
#define DISASS_HASHTABLEREF(N) \
  SETSP(ihtToTagged((IHashTable *) getAdressArg(PC+N)));
#define DISASS_RECORDARITY(N) \
  SETSP(sraToTagged(((SRecordArity) getAdressArg(PC+N))));
#define DISASS_LOCATION(N,M) \
  SETSP(locToTagged(GetBI(PC+N),GetLoc(PC+M)))
#define DISASS_BUILTINNAME(N) \
  SETSP(GetBI(PC+N)->getName())
#define DISASS_CALLMETHODINFO(N) \
  SETSP(((CallMethodInfo *) getAdressArg(PC+N))->mn)

#define DISASS_PREDID(N)         SETSP(oz_atom("pid"));
#define DISASS_PROCEDUREREF(N)   SETSP(oz_atom("pref"));
#define DISASS_GREGREF(N)        SETSP(oz_atom("gref"));

#define DISASS_ARITYANDISTAIL(N)    SETSP(oz_atom("arityt"));


inline 
OZ_Term oz_triple(OZ_Term t1,OZ_Term t2,OZ_Term t3) {
  SRecord *sr = SRecord::newSRecord(AtomPair,3);
  sr->setArg(0,t1);
  sr->setArg(1,t2);
  sr->setArg(2,t3);
  return makeTaggedSRecord(sr);
}

static TaggedRef sraToTagged(SRecordArity a) {
  return sraIsTuple(a) ?
    makeTaggedSmallInt(getTupleWidth(a)) :
    getRecordArity(a)->getList();
}

static TaggedRef ihtToTagged(IHashTable * iht) {
  SRecord * sr = SRecord::newSRecord(DA_iht,3);
  sr->setArg(0,makeTaggedSmallInt(iht->lookupElse()));
  sr->setArg(1,makeTaggedSmallInt(iht->lookupLTuple()));
  TaggedRef tbl = oz_nil();
  for (int i = iht->getSize(); i--; ) {
    if (iht->entries[i].val)
      if (iht->entries[i].sra == mkTupleWidth(0))
	tbl = oz_cons(oz_pair2(iht->entries[i].val,
			       makeTaggedSmallInt(iht->entries[i].lbl)),
		      tbl);
      else
	tbl = oz_cons(oz_triple(iht->entries[i].val,
				sraToTagged(iht->entries[i].sra),
				makeTaggedSmallInt(iht->entries[i].lbl)),
		      tbl);
				

    }
  sr->setArg(2,tbl);
  return makeTaggedSRecord(sr);
}

static TaggedRef locToTagged(Builtin * bi, OZ_Location * loc) {
  TaggedRef in = oz_nil();
  for (int j=bi->getInArity(); j--; )
    in=oz_cons(makeTaggedSmallInt(loc->getInIndex(j)),in);
  TaggedRef out = oz_nil();
  for (int i=bi->getOutArity(); i--; ) {
    out=oz_cons(makeTaggedSmallInt(loc->getOutIndex(bi,i)),out);
  }
  return oz_pair2(in,out);
}

TaggedRef CodeArea::disassemble(void){
  ProgramCounter PC = getStart();
  TaggedRef code;
  TaggedRef * tail = &code;
  while (OK) {
    TaggedRef   IN;
    TaggedRef * SP;
    Opcode OP     = getOpcode(PC);
    TaggedRef OPA = oz_atom(opcodeToString(OP));
    LTuple * lt = new LTuple();
    *tail = makeTaggedLTuple(lt);
    TaggedRef * head = lt->getRefHead();
    tail = lt->getRefTail();
    switch (OP) {
    case ENDOFFILE:
      DISASS_SETUP(0);
      *tail = oz_nil();
      return code;
    case PROFILEPROC:
    case RETURN:
    case POPEX:
    case DEALLOCATEL10:
    case DEALLOCATEL9:
    case DEALLOCATEL8:
    case DEALLOCATEL7:
    case DEALLOCATEL6:
    case DEALLOCATEL5:
    case DEALLOCATEL4:
    case DEALLOCATEL3:
    case DEALLOCATEL2:
    case DEALLOCATEL1:
    case DEALLOCATEL:
    case ALLOCATEL10:
    case ALLOCATEL9:
    case ALLOCATEL8:
    case ALLOCATEL7:
    case ALLOCATEL6:
    case ALLOCATEL5:
    case ALLOCATEL4:
    case ALLOCATEL3:
    case ALLOCATEL2:
    case ALLOCATEL1:
    case SKIP:
      DISASS_SETUP(0);
      PC += 1;
      break;
    case DEFINITIONCOPY:
    case DEFINITION:
      DISASS_SETUP(5);
      DISASS_XREGISTERINDEX(1);
      DISASS_LABEL(2);
      DISASS_PREDID(3);
      DISASS_PROCEDUREREF(4);
      DISASS_GREGREF(5);
      PC += 6;
      break;
    case EXHANDLER:
    case BRANCH:
    case ENDDEFINITION:
      DISASS_SETUP(1);
      DISASS_LABEL(1);
      PC += 2;
      break;
    case INLINEMINUS1:
    case INLINEPLUS1:
    case GETVARVARXX:
    case UNIFYVALVARXX:
    case UNIFYXX:
    case CREATEVARIABLEMOVEX:
    case MOVEXX:
      DISASS_SETUP(2);
      DISASS_XREGISTERINDEX(1);
      DISASS_XREGISTERINDEX(2);
      PC += 3;
      break;
    case GETVARVARXY:
    case UNIFYVALVARXY:
    case UNIFYXY:
    case MOVEXY:
      DISASS_SETUP(2);
      DISASS_XREGISTERINDEX(1);
      DISASS_YREGISTERINDEX(2);
      PC += 3;
      break;
    case GETVARVARYX:
    case UNIFYVALVARYX:
    case CREATEVARIABLEMOVEY:
    case MOVEYX:
      DISASS_SETUP(2);
      DISASS_YREGISTERINDEX(1);
      DISASS_XREGISTERINDEX(2);
      PC += 3;
      break;
    case GETVARVARYY:
    case UNIFYVALVARYY:
    case MOVEYY:
      DISASS_SETUP(2);
      DISASS_YREGISTERINDEX(1);
      DISASS_YREGISTERINDEX(2);
      PC += 3;
      break;
    case UNIFYVALVARGX:
    case MOVEGX:
      DISASS_SETUP(2);
      DISASS_GREGISTERINDEX(1);
      DISASS_XREGISTERINDEX(2);
      PC += 3;
      break;
    case UNIFYVALVARGY:
    case MOVEGY:
      DISASS_SETUP(2);
      DISASS_GREGISTERINDEX(1);
      DISASS_YREGISTERINDEX(2);
      PC += 3;
      break;
    case MOVEMOVEXYXY:
      DISASS_SETUP(4);
      DISASS_XREGISTERINDEX(1);
      DISASS_YREGISTERINDEX(2);
      DISASS_XREGISTERINDEX(3);
      DISASS_YREGISTERINDEX(4);
      PC += 5;
      break;
    case MOVEMOVEYXYX:
      DISASS_SETUP(4);
      DISASS_YREGISTERINDEX(1);
      DISASS_XREGISTERINDEX(2);
      DISASS_YREGISTERINDEX(3);
      DISASS_XREGISTERINDEX(4);
      PC += 5;
      break;
    case MOVEMOVEXYYX:
      DISASS_SETUP(4);
      DISASS_XREGISTERINDEX(1);
      DISASS_YREGISTERINDEX(2);
      DISASS_YREGISTERINDEX(3);
      DISASS_XREGISTERINDEX(4);
      PC += 5;
      break;
    case MOVEMOVEYXXY:
      DISASS_SETUP(4);
      DISASS_YREGISTERINDEX(1);
      DISASS_XREGISTERINDEX(2);
      DISASS_XREGISTERINDEX(3);
      DISASS_YREGISTERINDEX(4);
      PC += 5;
      break;
    case GETVARIABLEX:
    case FUNRETURNX:
    case GETRETURNX:
    case GETSELF:
    case UNIFYVALUEX:
    case UNIFYVARIABLEX:
    case GETLISTX:
    case SETVALUEX:
    case SETVARIABLEX:
    case PUTLISTX:
    case CREATEVARIABLEX:
      DISASS_SETUP(1);
      DISASS_XREGISTERINDEX(1);
      PC += 2;
      break;
    case CLEARY:
    case GETVARIABLEY:
    case FUNRETURNY:
    case GETRETURNY:
    case UNIFYVALUEY:
    case UNIFYVARIABLEY:
    case GETLISTY:
    case SETVALUEY:
    case SETVARIABLEY:
    case PUTLISTY:
    case CREATEVARIABLEY:
      DISASS_SETUP(1);
      DISASS_YREGISTERINDEX(1);
      PC += 2;
      break;
    case UNIFYXG:
      DISASS_SETUP(2);
      DISASS_XREGISTERINDEX(1);
      DISASS_GREGISTERINDEX(2);
      PC += 3;
      break;
    case GETRECORDX:
    case PUTRECORDX:
      DISASS_SETUP(3);
      DISASS_TAGGED(1);
      DISASS_RECORDARITY(2);
      DISASS_XREGISTERINDEX(3);
      PC += 4;
      break;
    case GETRECORDY:
    case PUTRECORDY:
      DISASS_SETUP(3);
      DISASS_TAGGED(1);
      DISASS_RECORDARITY(2);
      DISASS_YREGISTERINDEX(3);
      PC += 4;
      break;
    case GETNUMBERX:
    case GETLITERALX:
    case PUTCONSTANTX:
      DISASS_SETUP(2);
      DISASS_TAGGED(1);
      DISASS_XREGISTERINDEX(2);
      PC += 3;
      break;
    case GETNUMBERY:
    case GETLITERALY:
    case PUTCONSTANTY:
      DISASS_SETUP(2);
      DISASS_TAGGED(1);
      DISASS_YREGISTERINDEX(2);
      PC += 3;
      break;
    case FUNRETURNG:
    case GETRETURNG:
    case SETSELFG:
    case UNIFYVALUEG:
    case GETLISTG:
    case SETVALUEG:
      DISASS_SETUP(1);
      DISASS_GREGISTERINDEX(1);
      PC += 2;
      break;
    case LOCALVARNAME:
    case GLOBALVARNAME:
    case UNIFYLITERAL:
    case UNIFYNUMBER:
    case SETCONSTANT:
      DISASS_SETUP(1);
      DISASS_TAGGED(1);
      PC += 2;
      break;
    case SETPROCEDUREREF:
      DISASS_SETUP(1);
      DISASS_PROCEDUREREF(1);
      PC += 2;
      break;
    case GETVOID:
    case ALLOCATEL:
    case UNIFYVOID:
    case SETVOID:
      DISASS_SETUP(1);
      DISASS_COUNT(1);
      PC += 2;
      break;
    case GETRECORDG:
      DISASS_SETUP(3);
      DISASS_TAGGED(1);
      DISASS_RECORDARITY(2);
      DISASS_GREGISTERINDEX(3);
      PC += 4;
      break;
    case INLINEMINUS:
    case INLINEPLUS:
    case GETLISTVALVARX:
      DISASS_SETUP(3);
      DISASS_XREGISTERINDEX(1);
      DISASS_XREGISTERINDEX(2);
      DISASS_XREGISTERINDEX(3);
      PC += 4;
      break;
    case GETNUMBERG:
    case GETLITERALG:
      DISASS_SETUP(2);
      DISASS_TAGGED(1);
      DISASS_GREGISTERINDEX(2);
      PC += 3;
      break;
    case CALLMETHOD:
      DISASS_SETUP(2);
      DISASS_CALLMETHODINFO(1);
      DISASS_ARITY(2);
      PC += 3;
      break;
    case CALLGLOBAL:
      DISASS_SETUP(2);
      DISASS_GREGISTERINDEX(1);
      DISASS_ARITYANDISTAIL(2);
      PC += 3;
      break;
    case TAILCALLX:
    case CALLX:
      DISASS_SETUP(2);
      DISASS_XREGISTERINDEX(1);
      DISASS_ARITY(2);
      PC += 3;
      break;
    case CALLY:
      DISASS_SETUP(2);
      DISASS_YREGISTERINDEX(1);
      DISASS_ARITY(2);
      PC += 3;
      break;
    case TAILCALLG:
    case CALLG:
      DISASS_SETUP(2);
      DISASS_GREGISTERINDEX(1);
      DISASS_ARITY(2);
      PC += 3;
      break;
    case CALLCONSTANT:
      DISASS_SETUP(2);
      DISASS_TAGGED(1);
      DISASS_ARITYANDISTAIL(2);
      PC += 3;
      break;
    case CALLPROCEDUREREF:
      DISASS_SETUP(2);
      DISASS_PROCEDUREREF(1);
      DISASS_ARITYANDISTAIL(2);
      PC += 3;
      break;
    case FASTTAILCALL:
    case FASTCALL:
      DISASS_SETUP(2);
      DISASS_PROCEDUREREF(1);
      DISASS_DUMMY(2);
      PC += 3;
      break;
    case TAILSENDMSGX:
    case SENDMSGX:
      DISASS_SETUP(5);
      DISASS_TAGGED(1);
      DISASS_XREGISTERINDEX(2);
      DISASS_RECORDARITY(3);
      DISASS_CACHE(4);
      PC += 6;
      break;
    case TAILSENDMSGY:
    case SENDMSGY:
      DISASS_SETUP(5);
      DISASS_TAGGED(1);
      DISASS_YREGISTERINDEX(2);
      DISASS_RECORDARITY(3);
      DISASS_CACHE(4);
      PC += 6;
      break;
    case TAILSENDMSGG:
    case SENDMSGG:
      DISASS_SETUP(5);
      DISASS_TAGGED(1);
      DISASS_GREGISTERINDEX(2);
      DISASS_RECORDARITY(3);
      DISASS_CACHE(4);
      PC += 6;
      break;
    case LOCKTHREAD:
      DISASS_SETUP(2);
      DISASS_LABEL(1);
      DISASS_XREGISTERINDEX(2);
      PC += 3;
      break;
    case INLINEASSIGN:
    case INLINEAT:
      DISASS_SETUP(4);
      DISASS_TAGGED(1);
      DISASS_XREGISTERINDEX(2);
      DISASS_CACHE(3);
      PC += 5;
      break;
    case TESTNUMBERX:
    case TESTLITERALX:
      DISASS_SETUP(3);
      DISASS_XREGISTERINDEX(1);
      DISASS_TAGGED(2);
      DISASS_LABEL(3);
      PC += 4;
      break;
    case TESTNUMBERY:
    case TESTLITERALY:
      DISASS_SETUP(3);
      DISASS_YREGISTERINDEX(1);
      DISASS_TAGGED(2);
      DISASS_LABEL(3);
      PC += 4;
      break;
    case TESTNUMBERG:
    case TESTLITERALG:
      DISASS_SETUP(3);
      DISASS_GREGISTERINDEX(1);
      DISASS_TAGGED(2);
      DISASS_LABEL(3);
      PC += 4;
      break;
    case TESTRECORDX:
      DISASS_SETUP(4);
      DISASS_XREGISTERINDEX(1);
      DISASS_TAGGED(2);
      DISASS_RECORDARITY(3);
      DISASS_LABEL(4);
      PC += 5;
      break;
    case TESTRECORDY:
      DISASS_SETUP(4);
      DISASS_YREGISTERINDEX(1);
      DISASS_TAGGED(2);
      DISASS_RECORDARITY(3);
      DISASS_LABEL(4);
      PC += 5;
      break;
    case TESTRECORDG:
      DISASS_SETUP(4);
      DISASS_GREGISTERINDEX(1);
      DISASS_TAGGED(2);
      DISASS_RECORDARITY(3);
      DISASS_LABEL(4);
      PC += 5;
      break;
    case TESTLISTX:
      DISASS_SETUP(2);
      DISASS_XREGISTERINDEX(1);
      DISASS_LABEL(2);
      PC += 3;
      break;
    case TESTLISTY:
      DISASS_SETUP(2);
      DISASS_YREGISTERINDEX(1);
      DISASS_LABEL(2);
      PC += 3;
      break;
    case TESTLISTG:
      DISASS_SETUP(2);
      DISASS_GREGISTERINDEX(1);
      DISASS_LABEL(2);
      PC += 3;
      break;
    case TESTBOOLX:
      DISASS_SETUP(3);
      DISASS_XREGISTERINDEX(1);
      DISASS_LABEL(2);
      DISASS_LABEL(3);
      PC += 4;
      break;
    case TESTBOOLY:
      DISASS_SETUP(3);
      DISASS_YREGISTERINDEX(1);
      DISASS_LABEL(2);
      DISASS_LABEL(3);
      PC += 4;
      break;
    case TESTBOOLG:
      DISASS_SETUP(3);
      DISASS_GREGISTERINDEX(1);
      DISASS_LABEL(2);
      DISASS_LABEL(3);
      PC += 4;
      break;
    case MATCHX:
      DISASS_SETUP(2);
      DISASS_XREGISTERINDEX(1);
      DISASS_HASHTABLEREF(2);
      PC += 3;
      break;
    case MATCHY:
      DISASS_SETUP(2);
      DISASS_YREGISTERINDEX(1);
      DISASS_HASHTABLEREF(2);
      PC += 3;
      break;
    case MATCHG:
      DISASS_SETUP(2);
      DISASS_GREGISTERINDEX(1);
      DISASS_HASHTABLEREF(2);
      PC += 3;
      break;
    case DEBUGEXIT:
    case DEBUGENTRY:
      DISASS_SETUP(4);
      DISASS_TAGGED(1);
      DISASS_TAGGED(2);
      DISASS_TAGGED(3);
      DISASS_TAGGED(4);
      PC += 5;
      break;
    case CALLBI:
      DISASS_SETUP(2);
      DISASS_BUILTINNAME(1);
      DISASS_LOCATION(1,2);
      PC += 3;
      break;
    case INLINEDOT:
      DISASS_SETUP(5);
      DISASS_XREGISTERINDEX(1);
      DISASS_TAGGED(2);
      DISASS_XREGISTERINDEX(3);
      DISASS_CACHE(4);
      PC += 6;
      break;
    case TESTBI:
      DISASS_SETUP(3);
      DISASS_BUILTINNAME(1);
      DISASS_LOCATION(1,2);
      DISASS_LABEL(3);
      PC += 4;
      break;
    case TESTLE:
    case TESTLT:
      DISASS_SETUP(4);
      DISASS_XREGISTERINDEX(1);
      DISASS_XREGISTERINDEX(2);
      DISASS_XREGISTERINDEX(3);
      DISASS_LABEL(4);
      PC += 5;
      break;
    default:
      Assert(0);
      break;
    }
  }
}



OZ_BI_define(BIdisassemble, 1, 1) {
  oz_declareNonvarIN(0,p);
  
  if (oz_isConst(p)) {
    switch (tagged2Const(p)->getType()) {
    case Co_Abstraction:
      {
	PrTabEntry * pte = ((Abstraction *) tagged2Const(p))->getPred();
	OZ_RETURN(pte->getCodeBlock()->disassemble());
      }
    case Co_Builtin:
      OZ_RETURN(oz_mklist(oz_atom("builtin")));
    default:
      break;
    }
  }
  oz_typeError(0,"procedure");
} OZ_BI_end



#ifndef MODULES_LINK_STATIC

#include "modAssembler-if.cc"

#endif
