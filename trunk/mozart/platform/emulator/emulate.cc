/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Benjamin Lorenz (lorenz@ps.uni-sb.de)
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

// The main engine

#include <stdarg.h>

#include "am.hh"
#include "thr_int.hh"
#include "debug.hh"
#include "prop_int.hh"
#include "var_of.hh"
#include "codearea.hh"
#include "builtins.hh"
#include "indexing.hh"

#include "boot-manager.hh"
#include "dictionary.hh"
#include "copycode.hh"
#include "trace.hh"
#include "os.hh"

#ifdef OUTLINE
#define inline
#endif

/*
 * X registers. They are defined for proximity to the emulator!
 *
 */

TaggedRef XREGS[NumberOfXRegisters];
TaggedRef XREGS_SAVE[NumberOfXRegisters];


// -----------------------------------------------------------------------
// Object stuff
// -----------------------------------------------------------------------

inline
Abstraction *getSendMethod(Object *obj, TaggedRef label, SRecordArity arity, 
			   InlineCache *cache)
{
  Assert(oz_isFeature(label));
  return cache->lookup(obj->getClass(),label,arity);
}

// -----------------------------------------------------------------------
// *** EXCEPTION stuff
// -----------------------------------------------------------------------


#define RAISE_APPLY(fun,args)				\
  (void) oz_raise(E_ERROR,E_KERNEL,"apply",2,fun,args);	\
  RAISE_THREAD;

static
void enrichTypeException(TaggedRef value,const char *fun, OZ_Term args)
{
  OZ_Term e = OZ_subtree(value,makeTaggedSmallInt(1));
  OZ_putArg(e,1,OZ_atom((OZ_CONST char*)fun));
  OZ_putArg(e,2,args);
}

static
TaggedRef formatError(TaggedRef info, TaggedRef val, TaggedRef traceBack) {
  OZ_Term d = OZ_record(AtomD,oz_mklist(AtomInfo,AtomStack));
  OZ_putSubtree(d, AtomStack, traceBack);
  OZ_putSubtree(d, AtomInfo,  info);
  
  return OZ_adjoinAt(val, AtomDebug, d);
}


#define RAISE_TYPE1(fun,args)				\
  enrichTypeException(e->exception.value,fun,args);	\
  RAISE_THREAD;

#define RAISE_TYPE1_FUN(fun,args) \
  RAISE_TYPE1(fun, appendI(args,oz_mklist(oz_newVariable())));

#define RAISE_TYPE(bi,loc) \
  RAISE_TYPE1(bi->getPrintName(), loc->getArgs(bi));


/*
 * Handle Failure macros (HF)
 */

Bool AM::hf_raise_failure()
{
  if (!oz_onToplevel() && !oz_currentThread()->isCatch())
    return OK;

  exception.info  = NameUnit;
  exception.value = RecordFailure;
  exception.debug = ozconf.errorDebug;
  return NO;
}

// This macro is optimized such that the term T is only created
// when needed, so don't pass it as argument to functions.
#define HF_RAISE_FAILURE(T)				\
   if (e->hf_raise_failure())				\
     return T_FAILURE;				        \
   if (ozconf.errorDebug) e->setExceptionInfo(T);	\
   RAISE_THREAD;


#define HF_EQ(X,Y)     HF_RAISE_FAILURE(OZ_mkTupleC("eq",2,X,Y))
#define HF_TELL(X,Y)   HF_RAISE_FAILURE(OZ_mkTupleC("tell",2,X,Y))
#define HF_APPLY(N,A)  HF_RAISE_FAILURE(OZ_mkTupleC("apply",2,N,A))
#define HF_BI(bi,loc)  HF_APPLY(bi->getName(),loc->getArgs(bi));

#define CheckArity(arityExp,proc)				   \
if (predArity != arityExp) {					   \
  (void) oz_raise(E_ERROR,E_KERNEL,"arity",2,proc,                 \
                  OZ_toList(predArity,XREGS));                     \
  RAISE_THREAD;							   \
}

// -----------------------------------------------------------------------
// *** ???
// -----------------------------------------------------------------------

/*
 * make an record
 *  the subtrees are initialized with new variables
 */
static
TaggedRef mkRecord(TaggedRef label,SRecordArity ff)
{
  SRecord *srecord = SRecord::newSRecord(label,ff,getWidth(ff));
  srecord->initArgs();
  return makeTaggedSRecord(srecord);
}

#define allocateY(n) \
{                                      \
  int _sz = (n+1) * sizeof(TaggedRef); \
  Y = (RefsArray) freeListMalloc(_sz); \
  Y += 1;                              \
  initRefsArray(Y,n,OK);               \
}

#define deallocateYN(sz) \
{                                                   \
  Assert(getRefsArraySize(Y)==sz);                  \
  freeListDispose(Y-1,(sz+1) * sizeof(TaggedRef));  \
  Y=NULL;                                           \
}

#define deallocateY() deallocateYN(getRefsArraySize(Y))


static void buildRecord(ProgramCounter PC, RefsArray Y, Abstraction *CAP);
static ThreadReturn debugEntry(ProgramCounter PC, RefsArray Y, Abstraction *CAP);
static ThreadReturn debugExit(ProgramCounter PC, RefsArray Y, Abstraction *CAP);


// most probable case first: local UVar
// if (oz_isUVar(var) && isCurrentBoard(tagged2VarHome(var))) {
// more efficient:
inline
void bindOPT(OZ_Term *varPtr, OZ_Term term)
{
  Assert(oz_isUVar(*varPtr));
  if (!am.currentUVarPrototypeEq(*varPtr) && !oz_isLocalUVar(varPtr)) {
    DoBindAndTrail(varPtr,term);
  } else {
    doBind(varPtr,term);
  }
}

/* specially optimized unify: test two most probable cases first:
 *
 *     1. bind a unconstraint local variable to a non-var
 *     2. test two non-variables for equality
 */
inline
OZ_Return fastUnify(OZ_Term A, OZ_Term B) {
  OZ_Term term1 = A;
  DEREF0(term1,term1Ptr,_1);
  
  OZ_Term term2 = B;
  DEREF0(term2,term2Ptr,_2);

  if (!oz_isVariable(term2)) {
    if (am.currentUVarPrototypeEq(term1)) {
      doBind(term1Ptr,term2);
      goto exit;
    }
    if (term1==term2) {
      goto exit;
    }
  } else if (!oz_isVariable(term1) && am.currentUVarPrototypeEq(term2)) {
    doBind(term2Ptr,term1);
    goto exit;
  }
  
  return oz_unify(A,B);
  
 exit:
  return PROCEED;
}

/*
 * Executre builtins if now location is available:
 *  Normal procedure call
 *
 */

inline
OZ_Return oz_bi_wrapper(Builtin *bi) {
  Assert(am.isEmptySuspendVarList());
  Assert(am.isEmptyPreparedCalls());

  register const int outAr = bi->getOutArity();

  register TaggedRef * const XREGS_IN = XREGS + bi->getInArity();
  
  {
    for (int i=outAr; i--; ) 
      XREGS_SAVE[i]=XREGS_IN[i];
  }

  OZ_Return ret1 = bi->getFun()(OZ_ID_LOC->getMapping());
  
  if (ret1!=PROCEED) {
    switch (ret1) {
    case FAILED:
    case RAISE:
    case BI_TYPE_ERROR:
    case SUSPEND:
      {
	// restore X
	for (int j=outAr; j--; )
	  XREGS_IN[j]=XREGS_SAVE[j];
	return ret1;
      }
    case PROCEED:
    case BI_PREEMPT:
    case BI_REPLACEBICALL:
      break;
    default:
      OZ_error("Builtin: Unknown return value.\nDoes your builtin return a meaningful value?\nThis value is definitely unknown: %d",ret1);
      return FAILED;
    }
  }

  {
    for (int i=outAr;i--;) {
      OZ_Return ret2 = fastUnify(XREGS_IN[i],XREGS_SAVE[i]);
      if (ret2!=PROCEED) {
	switch (ret2) {
	case FAILED:
	case RAISE:
	case BI_TYPE_ERROR:
	  {
	    // restore X in case of error
	    for (int j=outAr; j--; ) {
	      XREGS_IN[j]=XREGS_SAVE[j];
	    }
	    return ret2;
	}
	case SUSPEND:
	  am.emptySuspendVarList();
	  am.prepareCall(BI_Unify,XREGS_IN[i],XREGS_SAVE[i]);
	  ret1=BI_REPLACEBICALL;
	  break;
	case BI_REPLACEBICALL:
	  ret1=BI_REPLACEBICALL;
	  break;
	default:
	  Assert(0);
	}
      }
    }
  }

  return ret1;
}

static
void set_exception_info_call(Builtin *bi, OZ_Location * loc) {
  if (bi==bi_raise||bi==bi_raiseError) 
    return;

  am.setExceptionInfo(OZ_mkTupleC("fapply",3,
				  makeTaggedConst(bi),
				  loc->getInArgs(bi),
				  oz_int(bi->getOutArity())));
}

// -----------------------------------------------------------------------
// *** patchToFastCall: self modifying code!
// -----------------------------------------------------------------------


void patchToFastCall(Abstraction *abstr, ProgramCounter PC, Bool isTailCall)
{
  AbstractionEntry *entry = new AbstractionEntry(NO);
  entry->setPred(abstr);
  CodeArea *code = CodeArea::findBlock(PC);
  code->writeAbstractionEntry(entry, PC+1);
  CodeArea::writeOpcode(isTailCall ? FASTTAILCALL : FASTCALL, PC);
}




// -----------------------------------------------------------------------
// *** CALL HOOK
// -----------------------------------------------------------------------


/* the hook functions return:
     TRUE: must reschedule
     FALSE: can continue
   */

#define DET_COUNTER 10000
inline
Bool hookCheckNeeded()
{
#if defined(DEBUG_DET)
  static int counter = DET_COUNTER;
  if (--counter < 0) {
    am.handleAlarm(CLOCK_TICK/1000);   // simulate an alarm
    counter = DET_COUNTER;
  }
#endif
  
  return am.isSetSFlag();
}


// -----------------------------------------------------------------------
// ??? <- Bob, Justus und Peter
// -----------------------------------------------------------------------

#define RAISE_THREAD_NO_PC			\
  e->exception.pc=NOCODE;			\
  goto LBLraise;

#define RAISE_THREAD				\
  e->exception.pc=PC;				\
  e->exception.y=Y;				\
  e->exception.cap=CAP;				\
  goto LBLraise;


/* macros are faster ! */
#define emulateHookCall(e,Code)			\
   if (hookCheckNeeded()) {			\
       Code;					\
       return T_PREEMPT;			\
   }

#define emulateHookPopTask(e) emulateHookCall(e,;)


#define ChangeSelf(obj)				\
      e->changeSelf(obj);

#define PushCont(_PC)  CTS->pushCont(_PC,Y,CAP);
#define PushContX(_PC) pushContX(CTS,_PC,Y,CAP);

void pushContX(TaskStack *stk, 
	       ProgramCounter pc,RefsArray y,Abstraction *cap);

/* NOTE:
 * in case we have call(x-N) and we have to switch process or do GC
 * we have to save as cont address Pred->getPC() and NOT PC
 */
#define CallDoChecks(Pred)						    \
     Y = NULL;								    \
     CAP = Pred;							    \
     emulateHookCall(e,PushContX(Pred->getPC()));


// -----------------------------------------------------------------------
// *** THREADED CODE
// -----------------------------------------------------------------------

#if defined(RECINSTRFETCH) && defined(THREADED)
 Error: RECINSTRFETCH requires THREADED == 0;
#endif

#define INCFPC(N) PC += N

#if !defined(DEBUG_EMULATOR) && !defined(DISABLE_INSTRPROFILE) && defined(__GNUC__)
#define asmLbl(INSTR) asm(" " #INSTR ":");
#else
#define asmLbl(INSTR)
#endif

#ifdef INLINEOPCODEMAP
#define INSERTOPCODE(INSTR) \
        INSTR##FAKE: \
        asm(OPCODEALIGNINSTR); \
        asm(OPM_##INSTR); \
        asm(OPCODEALIGNINSTR);
#else
#define INSERTOPCODE(INSTR)
#endif

#ifdef THREADED

#define Case(INSTR) INSERTOPCODE(INSTR); INSTR##LBL : asmLbl(INSTR); 

#ifdef DELAY_SLOT
// let gcc fill in the delay slot of the "jmp" instruction:
#define DISPATCH(INC) {				\
  intlong aux = *(PC+INC);			\
  INCFPC(INC);					\
  goto* (void*) (aux|textBase);			\
}
#elif defined(_MSC_VER)
#define DISPATCH(INC) {				\
   PC += INC;					\
   int aux = *PC;				\
   __asm jmp aux				\
}

#else
#define DISPATCH(INC) {				\
  INCFPC(INC);					\
  goto* (void*) ((*PC)|textBase);		\
}
#endif

#else /* THREADED */

#define Case(INSTR)   case INSTR :  INSTR##LBL : asmLbl(INSTR); 
#define DISPATCH(INC) INCFPC(INC); goto LBLdispatcher

#endif

#define JUMPRELATIVE(offset) Assert(offset!=0); DISPATCH(offset)
#define JUMPPC(N) JUMPRELATIVE(getLabelArg(PC+N))
#define JUMPABSOLUTE(absaddr) PC=absaddr; DISPATCH(0)


#define SETTMPA(V) { TMPA = &(V); }
#define SETTMPB(V) { TMPB = &(V); }
#define GETTMPA()  (*(TMPA))
#define GETTMPB()  (*(TMPB))

#define SETAUX(V)   { auxTaggedA = (V); };

#define XPC(N) (*(XRegToPtr(getXRegArg(PC+N))))
#define YPC(N) (*(YRegToPtr(Y,getYRegArg(PC+N))))
#define GPC(N) (*(GRegToPtr((CAP->getGRef()),getGRegArg(PC+N))))

/* define REGOPT if you want the into register optimization for GCC */
#if defined(REGOPT) && __GNUC__ >= 2 && (defined(ARCH_I486) || defined(ARCH_MIPS) || defined(OSF1_ALPHA) || defined(ARCH_SPARC)) && !defined(DEBUG_CHECK)
#define Into(Reg) asm(#Reg)

#ifdef ARCH_I486
#define Reg1 asm("%esi")
#define Reg2
#define Reg3
#define Reg4
#define Reg5
#define Reg6
#define Reg7
#endif

#ifdef ARCH_SPARC
#define Reg1 asm("i0")
#define Reg2 asm("i1")
#define Reg3 asm("i2")
#define Reg4 asm("i3")
#define Reg5 asm("i4")
#define Reg6 asm("i5")
#define Reg7 asm("l0")
#endif

#ifdef OSF1_ALPHA
#define Reg1 asm("$9")
#define Reg2 asm("$10")
#define Reg3
#define Reg4
#define Reg5
#define Reg6
#define Reg7
#endif

#ifdef ARCH_MIPS
#define Reg1 asm("$16")
#define Reg2 asm("$17")
#define Reg3 asm("$18")
#define Reg4 asm("$19")
#define Reg5 asm("$20")
#define Reg6
#define Reg7
#endif

#else

#define Reg1
#define Reg2
#define Reg3
#define Reg4
#define Reg5
#define Reg6
#define Reg7

#endif

/*
 * Handling of the READ/WRITE mode bit: 
 * last significant bit of sPointer set iff in WRITE mode
 */

#define SetReadMode  lastGetRecord = PC
#define SetWriteMode (sPointer = (TaggedRef *)((long)sPointer+1));

#define InWriteMode (((long)sPointer)&1)

#define GetSPointerWrite(ptr) (TaggedRef*)(((long)ptr)-1)


/* spointer also abused for return values of functions */
#define SetFunReturn(val) sPointer = (TaggedRef*) val
#define GetFunReturn()    (TaggedRef)sPointer


#ifdef DEBUG_LIVENESS
extern void checkLiveness(ProgramCounter PC,TaggedRef *X, int maxX);
#define CheckLiveness(PC) checkLiveness(PC,XREGS,CAP->getPred()->getMaxX())
#else
#define CheckLiveness(PC)
#endif

// ------------------------------------------------------------------------
// ???
// ------------------------------------------------------------------------

#define MAGIC_RET goto LBLMagicRet;

#define SUSP_PC(TermPtr,PC) {			\
   CheckLiveness(PC);				\
   PushContX(PC);				\
   tmpRet = oz_var_addSusp(TermPtr,CTT);	\
   MAGIC_RET;					\
}

/*
 * create the suspension for builtins returning SUSPEND
 *
 * PRE: no reference chains !!
 */


#define SUSPENDONVARLIST			\
{						\
  tmpRet = e->suspendOnVarList(CTT);		\
  MAGIC_RET;					\
}

static
OZ_Return suspendInline(Thread *th, OZ_Term A,OZ_Term B=0,OZ_Term C=0)
{
  if (C) {
    DEREF (C, ptr, _1);
    if (oz_isVariable(C)) {
      OZ_Return ret = oz_var_addSusp(ptr, th);
      if (ret != SUSPEND) return ret;
    }
  }
  if (B) {
    DEREF (B, ptr, _1);
    if (oz_isVariable(B)) {
      OZ_Return ret = oz_var_addSusp(ptr, th);
      if (ret != SUSPEND) return ret;
    }
  }
  {
    DEREF (A, ptr, _1);
    if (oz_isVariable(A)) {
      OZ_Return ret = oz_var_addSusp(ptr, th);
      if (ret != SUSPEND) return ret;
    }
  }
  return SUSPEND;
}

// -----------------------------------------------------------------------
// outlined auxiliary functions
// -----------------------------------------------------------------------

static
TaggedRef makeMessage(SRecordArity srecArity, TaggedRef label) {
  int width = getWidth(srecArity);
  if (width == 0) {
    return label;
  }

  if (width == 2 && oz_eq(label,AtomCons))
    return makeTaggedLTuple(new LTuple(XREGS[0],XREGS[1]));

  SRecord *tt;
  if(sraIsTuple(srecArity)) {
    tt = SRecord::newSRecord(label,width);
  } else {
    tt = SRecord::newSRecord(label,getRecordArity(srecArity));
  }
  for (int i = width-1;i >= 0; i--) {
    tt->setArg(i,XREGS[i]);
  }
  TaggedRef ret = makeTaggedSRecord(tt);

  return ret;
}


// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ************************************************************************
// ************************************************************************
// THE BIG EMULATE LOOP STARTS
// ************************************************************************
// ************************************************************************
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------


/* &Var prevent Var to be allocated to a register,
 * increases chances that variables declared as "register"
 * will be really allocated to registers
 */

#define NoReg(Var) { void *p = &Var; }

// short names
# define CBB (oz_currentBoard())
# define CTT (oz_currentThread())
# define CTS (e->getCachedStack())

int engine(Bool init) 
{  
// ------------------------------------------------------------------------
// *** Global Variables
// ------------------------------------------------------------------------
  /* ordered by importance: first variables will go into machine registers
   * if -DREGOPT is set
   */
  register ProgramCounter PC   Reg1 = 0;
  register RefsArray Y         Reg2 = NULL;
  register TaggedRef *sPointer Reg3 = NULL;
  register AM * const e	       Reg4 = &am;
  register Abstraction * CAP   Reg5 = NULL;

  Bool isTailCall              = NO;                NoReg(isTailCall);

  // handling perdio unification
  ProgramCounter lastGetRecord;                     NoReg(lastGetRecord);

  ConstTerm *predicate;	    NoReg(predicate);
  int predArity;    	    NoReg(predArity);

  TaggedRef * TMPA, * TMPB;

  // optimized arithmetic and special cases for unification
  OZ_Return tmpRet;  NoReg(tmpRet); 

  TaggedRef auxTaggedA, auxTaggedB;
  int auxInt;
  char *auxString;

#ifdef THREADED
  if (init) {
#include "instrtab.hh"
    CodeArea::init(instrTable);
#ifdef DEBUG_INLINEOPCODES
    for (int i=0; i<OZERROR; i++)
      if (CodeArea::adressToOpcode(CodeArea::opcodeToAdress((Opcode) i)) != i) {
	printf("Aaargh: %s\n", opcodeToString((Opcode) i));
      }
	
#endif
    return 0;
  }
#else
  if (init) {
    CodeArea::init(NULL);
    return 0;
  }
  Opcode op = (Opcode) -1;
#endif

  // no preemption required, because of exception handling impl.
  goto LBLpopTaskNoPreempt;

// ------------------------------------------------------------------------
// *** Emulator: execute instructions
// ------------------------------------------------------------------------

 LBLemulate:
  asmLbl(EMULATE);

  JUMPABSOLUTE( PC );

  asmLbl(END_EMULATE);
#ifndef THREADED
LBLdispatcher:
  asmLbl(DISPATCH);

#ifdef RECINSTRFETCH
  CodeArea::recordInstr(PC);
#endif

  DebugTrace( if (!ozd_trace("emulate",PC,Y,CAP)) {
    return T_FAILURE;
  });

  op = CodeArea::getOP(PC);

#ifdef PROFILE_INSTR
  {
    static Opcode lastop = OZERROR;
    if (op < PROFILE_INSTR_MAX) {
      ozstat.instr[op]++;
      ozstat.instrCollapsable[lastop][op]++;
      lastop = op;
    }
  }
#endif
 
  Assert(am.isEmptySuspendVarList());
  Assert(am.isEmptyPreparedCalls());
  switch (op) {
#endif

// -------------------------------------------------------------------------
// INSTRUCTIONS: TERM: MOVE/UNIFY/CREATEVAR/...
// -------------------------------------------------------------------------

  Case(MOVEXX)
    XPC(2) = XPC(1); DISPATCH(3);
  Case(MOVEXY)
    YPC(2) = XPC(1); DISPATCH(3);
  Case(MOVEYX)
    XPC(2) = YPC(1); DISPATCH(3);
  Case(MOVEYY)
    YPC(2) = YPC(1); DISPATCH(3);
  Case(MOVEGX)
    XPC(2) = GPC(1); DISPATCH(3);
  Case(MOVEGY)
    YPC(2) = GPC(1); DISPATCH(3);

  Case(MOVEMOVEXYXY)
    YPC(2) = XPC(1); YPC(4) = XPC(3); DISPATCH(5);
  Case(MOVEMOVEYXYX)
    XPC(2) = YPC(1); XPC(4) = YPC(3); DISPATCH(5);
  Case(MOVEMOVEYXXY)
    XPC(2) = YPC(1); YPC(4) = XPC(3); DISPATCH(5);
  Case(MOVEMOVEXYYX)
    YPC(2) = XPC(1); XPC(4) = YPC(3); DISPATCH(5);

  Case(CLEARY)
    {
      YPC(1) = NameVoidRegister;
      DISPATCH(2);
    }

  Case(GETSELF)
    {
      XPC(1) = makeTaggedConst(e->getSelf());
      DISPATCH(2);
    }

  Case(SETSELFG)
    {
      TaggedRef term = GPC(1);
      if (oz_isRef(term)) {
	DEREF(term,termPtr,tag);
	if (oz_isVariable(term)) {
	  SUSP_PC(termPtr,PC);
	}
      }
      ChangeSelf(tagged2Object(term));
      DISPATCH(2);
    }


  /*
   * Currently unused
   */
  Case(GETRETURNX) 
  Case(GETRETURNY) 
  Case(GETRETURNG) 
  Case(FUNRETURNX) 
  Case(FUNRETURNY) 
  Case(FUNRETURNG) 
    OZ_error("Impossible.");
  
  Case(CREATEVARIABLEX)
    {
      XPC(1) = oz_newVariableOPT();
      DISPATCH(2);
    }
  Case(CREATEVARIABLEY)
    {
      YPC(1) = oz_newVariableOPT();
      DISPATCH(2);
    }
  
  Case(CREATEVARIABLEMOVEX)
    {
      XPC(1) = XPC(2) = oz_newVariableOPT();
      DISPATCH(3);
    }
  Case(CREATEVARIABLEMOVEY)
    {
      YPC(1) = XPC(2) = oz_newVariableOPT();
      DISPATCH(3);
    }
  
  
  Case(UNIFYXY) SETAUX(YPC(2)); goto Unify;
  Case(UNIFYXG) SETAUX(GPC(2)); goto Unify;
  Case(UNIFYXX) SETAUX(XPC(2));
  {
  Unify:
    const TaggedRef A   = XPC(1);
    const TaggedRef B   = auxTaggedA;
    const OZ_Return ret = fastUnify(A,B);
    if (ret == PROCEED) {
      DISPATCH(3);
    }
    if (ret == FAILED) {
      HF_EQ(A,B);
    }
    
    tmpRet = ret;
    goto LBLunifySpecial;
  }
  
  
  Case(PUTRECORDX)
    {
      TaggedRef label = getLiteralArg(PC+1);
      SRecordArity ff = (SRecordArity) getAdressArg(PC+2);
      SRecord *srecord = SRecord::newSRecord(label,ff,getWidth(ff));
      
      XPC(3) = makeTaggedSRecord(srecord);
      sPointer = srecord->getRef();
      
      DISPATCH(4);
    }
  Case(PUTRECORDY)
    {
      TaggedRef label = getLiteralArg(PC+1);
      SRecordArity ff = (SRecordArity) getAdressArg(PC+2);
      SRecord *srecord = SRecord::newSRecord(label,ff,getWidth(ff));
      
      YPC(3) = makeTaggedSRecord(srecord);
      sPointer = srecord->getRef();
      
      DISPATCH(4);
    }

  
  Case(PUTCONSTANTX)
    {
      XPC(2) = getTaggedArg(PC+1); 
      DISPATCH(3);
    }
  Case(PUTCONSTANTY)
    {
      YPC(2) = getTaggedArg(PC+1); 
      DISPATCH(3);
    }

  
  Case(PUTLISTX)
    {
      LTuple *term = new LTuple();
      XPC(1)   = makeTaggedLTuple(term);
      sPointer = term->getRef();
      DISPATCH(2);
    }
  Case(PUTLISTY)
    {
      LTuple *term = new LTuple();
      YPC(1)   = makeTaggedLTuple(term);
      sPointer = term->getRef();
      DISPATCH(2);
    }
  

  Case(SETVARIABLEX)
    {
      *sPointer = e->currentUVarPrototype();
      XPC(1)    = makeTaggedRef(sPointer++);
      DISPATCH(2);
    } 
  Case(SETVARIABLEY)
    {
      *sPointer = e->currentUVarPrototype();
      YPC(1)    = makeTaggedRef(sPointer++);
      DISPATCH(2);
    } 


  Case(SETVALUEX)
    {
      *sPointer++ = XPC(1);
      DISPATCH(2);
    }
  Case(SETVALUEY)
    {
      *sPointer++ = YPC(1);
      DISPATCH(2);
    }
  Case(SETVALUEG)
    {
      *sPointer++ = GPC(1);
      DISPATCH(2);
    }
  
  
  Case(SETCONSTANT)
    {
      *sPointer++ = getTaggedArg(PC+1);
      DISPATCH(2);
    }
  
  Case(SETPROCEDUREREF)
    {
      *sPointer++ = OZ_makeForeignPointer(getAdressArg(PC+1));
      DISPATCH(2);
    }
  
  Case(SETVOID)
    {
      int n = getPosIntArg(PC+1);
      while (n > 0) {
	*sPointer++ = e->currentUVarPrototype();
	n--;
      }
      DISPATCH(2);
    }
  
  
  Case(GETRECORDY) SETTMPA(YPC(3)); goto GetRecord;
  Case(GETRECORDG) SETTMPA(GPC(3)); goto GetRecord;
  Case(GETRECORDX) SETTMPA(XPC(3)); /* fall through */
  {
  GetRecord:
    TaggedRef label = getLiteralArg(PC+1);
    SRecordArity ff = (SRecordArity) getAdressArg(PC+2);
    
    TaggedRef term = GETTMPA();
    DEREF(term,termPtr,tag);
    
    if (oz_isUVar(term)) {
      SRecord *srecord = SRecord::newSRecord(label,ff,getWidth(ff));
      bindOPT(termPtr,makeTaggedSRecord(srecord));
      sPointer = srecord->getRef();
      SetWriteMode;
      DISPATCH(4);
    } else if (oz_isVariable(term)) {
      Assert(oz_isCVar(term));
      TaggedRef record;
      if (oz_onToplevel()) {
	GETTMPA() = record = OZ_newVariable();
	buildRecord(PC,Y,CAP);
	record = oz_deref(record);
	GETTMPA() = makeTaggedRef(termPtr);
      } else {
	SRecord *srecord = SRecord::newSRecord(label,ff,getWidth(ff));
	// mm2: optimize simple variables: use write mode
	// fill w/unb. var.
	srecord->initArgs();
	record=makeTaggedSRecord(srecord);
      }
      tmpRet = oz_var_bind(tagged2CVar(term),termPtr,record);

      if (tmpRet==PROCEED) {
	sPointer = tagged2SRecord(oz_deref(record))->getRef();
	SetReadMode;
	DISPATCH(4);
      }
      if (tmpRet!=FAILED) goto LBLunifySpecial;
      // fall through to failed
    } else if (isSRecordTag(tag) &&
	       tagged2SRecord(term)->compareSortAndArity(label,ff)) {
      sPointer = tagged2SRecord(term)->getRef();
      SetReadMode;
      DISPATCH(4);
    }

    HF_TELL(GETTMPA(),mkRecord(label,ff));
  }


  Case(TESTLITERALG) SETAUX(GPC(1)); goto testLiteral;
  Case(TESTLITERALY) SETAUX(YPC(1)); goto testLiteral;
  Case(TESTLITERALX) SETAUX(XPC(1)); /* fall through */
  {
  testLiteral:
    TaggedRef term = auxTaggedA;
    TaggedRef atm  = getLiteralArg(PC+2);
    
    DEREF(term,termPtr,tag);
    if (oz_isVariable(term)) {
      if (oz_isCVar(term) &&
	  !oz_var_valid(tagged2CVar(term),atm)) {
	// fail
	JUMPPC(3);
      }
      SUSP_PC(termPtr,PC);
    }
    if (oz_eq(term,atm)) {
      DISPATCH(4);
    }
    // fail
    JUMPPC(3);
  }
  
  Case(TESTBOOLG) SETAUX(GPC(1)); goto testBool;
  Case(TESTBOOLY) SETAUX(YPC(1)); goto testBool;
  Case(TESTBOOLX) SETAUX(XPC(1)); /* fall through */
  {
  testBool:
    TaggedRef term = auxTaggedA;
    DEREF(term,termPtr,tag);
    
    if (oz_eq(term,oz_true())) {
      DISPATCH(4);
    }
    
    if (oz_eq(term,oz_false())) {
      JUMPPC(2);
    }
    
    // mm2: kinded and ofs handling missing
    if (oz_isVariable(term)) {
      SUSP_PC(termPtr, PC);
    }
    
    JUMPPC(3);
  }
  
  Case(TESTNUMBERG) SETAUX(GPC(1)); goto testNumber;
  Case(TESTNUMBERY) SETAUX(YPC(1)); goto testNumber;
  Case(TESTNUMBERX) SETAUX(XPC(1)); /* fall through */
  {
  testNumber:
    TaggedRef term = auxTaggedA;
    TaggedRef i = getNumberArg(PC+2);
    
    DEREF(term,termPtr,tag);
    
    /* optimized for integer case */
    if (isSmallIntTag(tag)) {
      if (smallIntEq(term,i)) {
	DISPATCH(4);
      }
      JUMPPC(3);
    }
    
    if (oz_numberEq(i,term)) {
      DISPATCH(4);
    }
    
    if (oz_isVariable(term)) {
      if (oz_isKinded(term) &&
	  !oz_var_valid(tagged2CVar(term),i)) {
	// fail
	JUMPPC(3);
      }
      SUSP_PC(termPtr,PC);
    }
    // fail
    JUMPPC(3);
  }
  
  
  Case(TESTRECORDG) SETAUX(GPC(1)); goto testRecord;
  Case(TESTRECORDY) SETAUX(YPC(1)); goto testRecord;
  Case(TESTRECORDX) SETAUX(XPC(1));  /* fall through */
  {
  testRecord:
    TaggedRef term = auxTaggedA;
    TaggedRef label = getLiteralArg(PC+2);
    SRecordArity sra = (SRecordArity) getAdressArg(PC+3);
    
    DEREF(term,termPtr,tag);
    if (isSRecordTag(tag)) {
      if (tagged2SRecord(term)->compareSortAndArity(label,sra)) {
	sPointer = tagged2SRecord(term)->getRef();
	DISPATCH(5);
      }
    } else if (oz_isVariable(term)) {
      if (!oz_isKinded(term)) {
	SUSP_PC(termPtr,PC);
      } else if (oz_isCVar(term)) {
	OzVariable *cvar = tagged2CVar(term);
	if (cvar->getType() == OZ_VAR_OF) {
	  OzOFVariable *ofsvar = (OzOFVariable *) cvar;
	  Literal *lit = tagged2Literal(label);
	  if (sraIsTuple(sra) && ofsvar->disentailed(lit,getTupleWidth(sra)) ||
	      !sraIsTuple(sra) && ofsvar->disentailed(lit,getRecordArity(sra))) {
	    JUMPPC(4);
	  }
	  SUSP_PC(termPtr,PC);
	}
      }
      // fall through
    }
    // fail
    JUMPPC(4);
  }
  
  
  Case(TESTLISTG) SETAUX(GPC(1)); goto testList;
  Case(TESTLISTY) SETAUX(YPC(1)); goto testList;
  Case(TESTLISTX) SETAUX(XPC(1)); /* fall through */
  {
  testList:
    TaggedRef term = auxTaggedA;
    
    DEREF(term,termPtr,tag);
    if (isLTupleTag(tag)) {
      sPointer = tagged2LTuple(term)->getRef();
      DISPATCH(3);
    } else if (oz_isVariable(term)) {
      if (!oz_isKinded(term)) {
	SUSP_PC(termPtr,PC);
      } else if (oz_isCVar(term)) {
	OzVariable *cvar = tagged2CVar(term);
	if (cvar->getType() == OZ_VAR_OF) {
	  OzOFVariable *ofsvar = (OzOFVariable *) cvar;
	  if (ofsvar->disentailed(tagged2Literal(AtomCons),2)) {
	    JUMPPC(2);
	  }
	  SUSP_PC(termPtr,PC);
	}
      }
      // fall through
    }
    // fail
    JUMPPC(2);
  }
  
  
  Case(GETLITERALX)
    {
      TaggedRef atm = getLiteralArg(PC+1);
      
      TaggedRef term = XPC(2);
      DEREF(term,termPtr,tag);
      
      if (oz_isUVar(term)) {
	bindOPT(termPtr, atm);
	DISPATCH(3);
      }
      
      if (oz_eq(term,atm)) {
	DISPATCH(3);
      }

      auxTaggedA = XPC(2);
      goto getLiteralComplicated;
    }
  Case(GETLITERALY)
    {
      TaggedRef atm = getLiteralArg(PC+1);
      
      TaggedRef term = YPC(2);
      DEREF(term,termPtr,tag);
      
      if (oz_isUVar(term)) {
	bindOPT(termPtr, atm);
	DISPATCH(3);
      }
      
      if (oz_eq(term,atm)) {
	DISPATCH(3);
      }

      auxTaggedA = YPC(2);
      goto getLiteralComplicated;
    }
  Case(GETLITERALG)
    {
      TaggedRef atm = getLiteralArg(PC+1);
      
      TaggedRef term = GPC(2);
      DEREF(term,termPtr,tag);
      
      if (oz_isUVar(term)) {
	bindOPT(termPtr, atm);
	DISPATCH(3);
      }
      
      if (oz_eq(term,atm)) {
	DISPATCH(3);
      }

      auxTaggedA = GPC(2);
      goto getLiteralComplicated;
    }


  {
  getLiteralComplicated:
    TaggedRef term = auxTaggedA;
    TaggedRef atm = getLiteralArg(PC+1);
    DEREF(term,termPtr,tag);
    if (oz_isCVar(term)) {
      tmpRet = oz_var_bind(tagged2CVar(term),termPtr,atm);
      if (tmpRet==PROCEED) { DISPATCH(3); }
      if (tmpRet!=FAILED)  { goto LBLunifySpecial; }
      // fall through to fail
    }
    
    HF_TELL(auxTaggedA, atm);
  }


  Case(GETNUMBERX)
    {
      TaggedRef i = getNumberArg(PC+1);
      TaggedRef term = XPC(2);
      DEREF(term,termPtr,tag);
      
      if (oz_isUVar(term)) {
	bindOPT(termPtr, i);
	DISPATCH(3);
      }
      
      if (oz_numberEq(term,i)) {
	DISPATCH(3);
      }
      
      if (oz_isCVar(term)) {
	tmpRet=oz_var_bind(tagged2CVar(term),termPtr,i);
	if (tmpRet==PROCEED) { DISPATCH(3); }
	if (tmpRet!=FAILED)  { goto LBLunifySpecial; }
	// fall through to fail
      }
      
      HF_TELL(XPC(2), getNumberArg(PC+1));
    }
  Case(GETNUMBERY)
    {
      TaggedRef i = getNumberArg(PC+1);
      TaggedRef term = YPC(2);
      DEREF(term,termPtr,tag);
      
      if (oz_isUVar(term)) {
	bindOPT(termPtr, i);
	DISPATCH(3);
      }
      
      if (oz_numberEq(term,i)) {
	DISPATCH(3);
      }
      
      if (oz_isCVar(term)) {
	tmpRet=oz_var_bind(tagged2CVar(term),termPtr,i);
	if (tmpRet==PROCEED) { DISPATCH(3); }
	if (tmpRet!=FAILED)  { goto LBLunifySpecial; }
	// fall through to fail
      }
      
      HF_TELL(YPC(2), getNumberArg(PC+1));
    }
  Case(GETNUMBERG)
    {
      TaggedRef i = getNumberArg(PC+1);
      TaggedRef term = GPC(2);
      DEREF(term,termPtr,tag);
      
      if (oz_isUVar(term)) {
	bindOPT(termPtr, i);
	DISPATCH(3);
      }
      
      if (oz_numberEq(term,i)) {
	DISPATCH(3);
      }
      
      if (oz_isCVar(term)) {
	tmpRet=oz_var_bind(tagged2CVar(term),termPtr,i);
	if (tmpRet==PROCEED) { DISPATCH(3); }
	if (tmpRet!=FAILED)  { goto LBLunifySpecial; }
	// fall through to fail
      }
      
      HF_TELL(GPC(2), getNumberArg(PC+1));
    }

/* getListValVar(N,R,M) == getList(X[N]) unifyVal(R) unifyVar(X[M]) */
  Case(GETLISTVALVARX)
    {
      TaggedRef term = XPC(1);
      DEREF(term,termPtr,tag);
      
      if (oz_isUVar(term)) {
	register LTuple *ltuple = new LTuple();
	ltuple->setHead(XPC(2));
	ltuple->setTail(e->currentUVarPrototype());
	bindOPT(termPtr,makeTaggedLTuple(ltuple));
	XPC(3) = makeTaggedRef(ltuple->getRef()+1);
	DISPATCH(4);
      }
      if (oz_isLTuple(term)) {
	TaggedRef *argg = tagged2LTuple(term)->getRef();
	OZ_Return aux = fastUnify(XPC(2),
				  makeTaggedRef(argg));
	if (aux==PROCEED) {
	  XPC(3) = tagged2NonVariable(argg+1);
	  DISPATCH(4);
	}
	if (aux!=FAILED) {
	  tmpRet = aux;
	  goto LBLunifySpecial;
	}
	HF_TELL(XPC(2), makeTaggedRef(argg));
      }
      
      if (oz_isVariable(term)) {
	Assert(oz_isCVar(term));
	// mm2: why not new LTuple(h,t)? OPT?
	register LTuple *ltuple = new LTuple();
	ltuple->setHead(XPC(2));
	ltuple->setTail(e->currentUVarPrototype());
	tmpRet=oz_var_bind(tagged2CVar(term),termPtr,makeTaggedLTuple(ltuple));
	if (tmpRet==PROCEED) { 
	  XPC(3) = makeTaggedRef(ltuple->getRef()+1);
	  DISPATCH(4);
	}
	if (tmpRet!=FAILED)  { goto LBLunifySpecial; }
	
	HF_TELL(XPC(1), makeTaggedLTuple(ltuple));
      }
      
      HF_TELL(XPC(1), makeTaggedLTuple(new LTuple(XPC(2),e->currentUVarPrototype())));
    }


  Case(GETLISTG) SETTMPA(GPC(1)); goto getList;
  Case(GETLISTY) SETTMPA(YPC(1)); goto getList;
  Case(GETLISTX) SETTMPA(XPC(1)); /* fall through */
    {
    getList:
      TaggedRef term = GETTMPA();
      DEREF(term,termPtr,tag);
      
      if (oz_isUVar(term)) {
	LTuple *ltuple = new LTuple();
	sPointer = ltuple->getRef();
	SetWriteMode;
	bindOPT(termPtr,makeTaggedLTuple(ltuple));
	DISPATCH(2);
      } else if (oz_isVariable(term)) {
	Assert(oz_isCVar(term));
	TaggedRef record;
	if (oz_onToplevel()) {
	  TaggedRef *regPtr = &(GETTMPA());
	  TaggedRef savedTerm = *regPtr;
	  *regPtr = record = OZ_newVariable();
	  buildRecord(PC,Y,CAP);
	  record = oz_deref(record);
	  sPointer = tagged2LTuple(record)->getRef();
	  *regPtr = savedTerm;
	} else {
	  LTuple *ltuple = new LTuple();
	  sPointer = ltuple->getRef();
	  ltuple->setHead(e->currentUVarPrototype());
	  ltuple->setTail(e->currentUVarPrototype());
	  record = makeTaggedLTuple(ltuple);
	}
	tmpRet=oz_var_bind(tagged2CVar(term),termPtr,record);
	if (tmpRet==PROCEED) {
	  SetReadMode;
	  DISPATCH(2);
	}
	if (tmpRet!=FAILED) {
	  goto LBLunifySpecial;
	}
	// fall through to fail
      } else if (oz_isLTuple(term)) {
	sPointer = tagged2LTuple(term)->getRef();
	SetReadMode;
	DISPATCH(2);
      }
      
      HF_TELL(GETTMPA(), 
	      makeTaggedLTuple(new LTuple(e->currentUVarPrototype(),
					  e->currentUVarPrototype())));
    }


  /* a unifyVariable in read mode */
  Case(GETVARIABLEX)
    {
      XPC(1) = tagged2NonVariable(sPointer);
      sPointer++;
      DISPATCH(2);
    }
  Case(GETVARIABLEY)
    {
      YPC(1) = tagged2NonVariable(sPointer);
      sPointer++;
      DISPATCH(2);
    }

  Case(GETVARVARXX)
    {
      XPC(1) = tagged2NonVariable(sPointer);
      XPC(2) = tagged2NonVariable(sPointer+1);
      sPointer += 2;
      DISPATCH(3);
    }
  Case(GETVARVARXY)
    {
      XPC(1) = tagged2NonVariable(sPointer);
      YPC(2) = tagged2NonVariable(sPointer+1);
      sPointer += 2;
      DISPATCH(3);
    }
  Case(GETVARVARYX)
    {
      YPC(1) = tagged2NonVariable(sPointer);
      XPC(2) = tagged2NonVariable(sPointer+1);
      sPointer += 2;
      DISPATCH(3);
    }
  Case(GETVARVARYY)
    {
      YPC(1) = tagged2NonVariable(sPointer);
      YPC(2) = tagged2NonVariable(sPointer+1);
      sPointer += 2;
      DISPATCH(3);
    }



  /* a unify void in read case mode */
Case(GETVOID)
    {
      sPointer += getPosIntArg(PC+1);
      DISPATCH(2);
    }


  Case(UNIFYVARIABLEX)
    {
      if (InWriteMode) {
	TaggedRef *sp = GetSPointerWrite(sPointer);
	*sp = e->currentUVarPrototype();
	XPC(1) = makeTaggedRef(sp);
      } else {
	XPC(1) = tagged2NonVariable(sPointer);
      }
      sPointer++;
      DISPATCH(2);
    }
  Case(UNIFYVARIABLEY)
    {
      if (InWriteMode) {
	TaggedRef *sp = GetSPointerWrite(sPointer);
	*sp = e->currentUVarPrototype();
	YPC(1) = makeTaggedRef(sp);
      } else {
	YPC(1) = tagged2NonVariable(sPointer);
      }
      sPointer++;
      DISPATCH(2);
    }


  Case(UNIFYVALUEX)
    {
      TaggedRef term = XPC(1);
      
      if(InWriteMode) {
	TaggedRef *sp = GetSPointerWrite(sPointer);
	*sp = term;
	sPointer++;
	DISPATCH(2);
      } else {
	tmpRet=fastUnify(tagged2NonVariable(sPointer),term);
	sPointer++;
	if (tmpRet==PROCEED) { DISPATCH(2); }
	if (tmpRet!=FAILED)  {
	  PC = lastGetRecord; goto LBLunifySpecial;
	} 
	
	HF_TELL(*(sPointer-1), term);
      }
    }
  Case(UNIFYVALUEY)
    {
      TaggedRef term = YPC(1);
      
      if(InWriteMode) {
	TaggedRef *sp = GetSPointerWrite(sPointer);
	*sp = term;
	sPointer++;
	DISPATCH(2);
      } else {
	tmpRet=fastUnify(tagged2NonVariable(sPointer),term);
	sPointer++;
	if (tmpRet==PROCEED) { DISPATCH(2); }
	if (tmpRet!=FAILED)  {
	  PC = lastGetRecord; goto LBLunifySpecial;
	} 
	
	HF_TELL(*(sPointer-1), term);
      }
    }
  Case(UNIFYVALUEG)
    {
      TaggedRef term = GPC(1);
      
      if(InWriteMode) {
	TaggedRef *sp = GetSPointerWrite(sPointer);
	*sp = term;
	sPointer++;
	DISPATCH(2);
      } else {
	tmpRet=fastUnify(tagged2NonVariable(sPointer),term);
	sPointer++;
	if (tmpRet==PROCEED) { DISPATCH(2); }
	if (tmpRet!=FAILED)  {
	  PC = lastGetRecord; goto LBLunifySpecial;
	} 
	
	HF_TELL(*(sPointer-1), term);
      }
    }

  Case(UNIFYVALVARXX) SETTMPA(XPC(1)); SETTMPB(XPC(2)); goto UnifyValVar;
  Case(UNIFYVALVARXY) SETTMPA(XPC(1)); SETTMPB(YPC(2)); goto UnifyValVar;
  Case(UNIFYVALVARYX) SETTMPA(YPC(1)); SETTMPB(XPC(2)); goto UnifyValVar;
  Case(UNIFYVALVARYY) SETTMPA(YPC(1)); SETTMPB(YPC(2)); goto UnifyValVar;
  Case(UNIFYVALVARGX) SETTMPA(GPC(1)); SETTMPB(XPC(2)); goto UnifyValVar;
  Case(UNIFYVALVARGY) SETTMPA(GPC(1)); SETTMPB(YPC(2)); goto UnifyValVar;
  {
  UnifyValVar:

    if (InWriteMode) {
      TaggedRef *sp = GetSPointerWrite(sPointer);
      *sp = GETTMPA();

      *(sp+1) = e->currentUVarPrototype();
      GETTMPB() = makeTaggedRef(sp+1);

      sPointer += 2;
      DISPATCH(3);
    }

    tmpRet = fastUnify(GETTMPA(),makeTaggedRef(sPointer));

    if (tmpRet==PROCEED) {
      GETTMPB() = tagged2NonVariable(sPointer+1);
      sPointer += 2;
      DISPATCH(3);
    }
    if (tmpRet == FAILED) {
      HF_TELL(*sPointer, GETTMPA());
    }
    
    PC = lastGetRecord;
    goto LBLunifySpecial;
  }
  

  Case(UNIFYLITERAL)
     {
       if (InWriteMode) {
	 TaggedRef *sp = GetSPointerWrite(sPointer);
	 *sp = getTaggedArg(PC+1);
	 sPointer++;
	 DISPATCH(2);
       }

       TaggedRef atm = getLiteralArg(PC+1);
	 
       /* code adapted from GETLITERAL */
       TaggedRef *termPtr = sPointer;
       sPointer++;
       DEREFPTR(term,termPtr,tag);

       if (oz_isUVar(term)) {
         bindOPT(termPtr, atm);
         DISPATCH(2);
       }
		 
       if ( oz_eq(term,atm) ) {
         DISPATCH(2);
       }

       if (oz_isCVar(term)) {
         tmpRet=oz_var_bind(tagged2CVar(term),termPtr,atm);
	 if (tmpRet==PROCEED) { DISPATCH(2); }
	 if (tmpRet!=FAILED)  { PC = lastGetRecord; goto LBLunifySpecial; }
         term = *termPtr;  // Note: 'term' may be disposed
       }

       HF_TELL(*(sPointer-1), getTaggedArg(PC+1));
     }

  Case(UNIFYNUMBER)
    {
      if (InWriteMode) {
	TaggedRef *sp = GetSPointerWrite(sPointer);
	*sp = getTaggedArg(PC+1);
	sPointer++;
	DISPATCH(2);
      } 

      TaggedRef i = getNumberArg(PC+1);
      /* code adapted from GETLITERAL */
      TaggedRef *termPtr = sPointer;
      sPointer++;
      DEREFPTR(term,termPtr,tag);

      if (oz_isUVar(term)) {
        bindOPT(termPtr, i);
        DISPATCH(2);
      }
		 
      if (oz_numberEq(term,i)) {
	DISPATCH(2);
      }

      if (oz_isCVar(term)) {
	tmpRet=oz_var_bind(tagged2CVar(term),termPtr,i);
	if (tmpRet==PROCEED) { DISPATCH(2); }
	if (tmpRet!=FAILED)  { PC = lastGetRecord; goto LBLunifySpecial; }
	term = *termPtr;  // Note: 'term' may be disposed
      }

      HF_TELL(*(sPointer-1), getTaggedArg(PC+1) );
    }


  Case(UNIFYVOID)
    {
      int n = getPosIntArg(PC+1);
      if (InWriteMode) {
	TaggedRef *sp = GetSPointerWrite(sPointer);
	for (int i = n-1; i >=0; i-- ) {
	  *sp++ = e->currentUVarPrototype();
	}
      }
      sPointer += n;
      DISPATCH(2);
    }

  Case(MATCHG) SETTMPA(GPC(1)); goto match;
  Case(MATCHY) SETTMPA(YPC(1)); goto match;
  Case(MATCHX) SETTMPA(XPC(1)); /* fall through */
  match:
    {
      TaggedRef term     = GETTMPA();
      IHashTable * table = (IHashTable *) getAdressArg(PC+2);

      TaggedRef * termPtr;
#ifdef DEBUG_CHECK
      termPtr = 0;
#endif

    retry:
      switch (tagTypeOf(term)) {
      case TAG_LTUPLE:
	Assert(table->lookupLTuple());
	sPointer = tagged2LTuple(term)->getRef();
	JUMPRELATIVE(table->lookupLTuple());
      case TAG_SRECORD:
	sPointer = tagged2SRecord(term)->getRef();
	JUMPRELATIVE(table->lookupSRecord(term));
      case TAG_LITERAL:
	JUMPRELATIVE(table->lookupLiteral(term));
      case TAG_SMALLINT:
	JUMPRELATIVE(table->lookupSmallInt(term));
      case TAG_FLOAT:
	JUMPRELATIVE(table->lookupFloat(term));
      case TAG_CONST:
	if (tagged2Const(term)->getType() == Co_BigInt) {
	  JUMPRELATIVE(table->lookupBigInt(term));
	} else {
	  JUMPRELATIVE(table->lookupElse());
	}
      case TAG_REF:
      case TAG_REF2:
      case TAG_REF3:
      case TAG_REF4:
	termPtr = tagged2Ref(term);
	term    = *termPtr;
	goto retry;
      case TAG_CVAR:
	if (oz_isKinded(term) && table->disentailed(tagged2CVar(term))) {
	  JUMPRELATIVE(table->lookupElse());
	};
	/* fall through */
      case TAG_UVAR:
	Assert(termPtr);
	SUSP_PC(termPtr,PC);
      default:
	JUMPRELATIVE(table->lookupElse());
      }
      
    }

// ------------------------------------------------------------------------
// INSTRUCTIONS: (Fast-) Call/Execute Inline Funs/Rels
// ------------------------------------------------------------------------

  Case(FASTCALL)
    {
      PushCont(PC+3);
      AbstractionEntry *entry = (AbstractionEntry *)getAdressArg(PC+1);
      
      CallDoChecks(entry->getAbstr());

      IHashTable *table = entry->getIndexTable();
      PC = entry->getPC();
      if (table && tagTypeOf(XREGS[0])==TAG_LTUPLE) {
	sPointer = tagged2LTuple(XREGS[0])->getRef();
	JUMPRELATIVE(table->lookupLTuple());
      } else {
	DISPATCH(0);
      }
    }


  Case(FASTTAILCALL)
    {
      AbstractionEntry *entry = (AbstractionEntry *)getAdressArg(PC+1);
      
      CallDoChecks(entry->getAbstr());

      IHashTable *table = entry->getIndexTable();
      PC = entry->getPC();
      if (table && tagTypeOf(XREGS[0])==TAG_LTUPLE) {
	sPointer = tagged2LTuple(XREGS[0])->getRef();
	JUMPRELATIVE(table->lookupLTuple());
      } else {
	DISPATCH(0);
      }
    }

  Case(CALLBI)
    {
      Builtin* bi = GetBI(PC+1);
      OZ_Location* loc = GetLoc(PC+2);

#ifdef PROFILE_BI
      bi->incCounter();
#endif

      int res = bi->getFun()(loc->getMapping());
      if (res == PROCEED) { DISPATCH(3); }
      switch (res) {
      case FAILED:	  HF_BI(bi,loc);
      case RAISE:
	if (e->exception.debug) 
	  set_exception_info_call(bi,loc);
	RAISE_THREAD;

      case BI_TYPE_ERROR: RAISE_TYPE(bi,loc);
	 
      case SUSPEND:
	PushContX(PC);
	SUSPENDONVARLIST;

      case BI_PREEMPT:
	PushContX(PC+3);
	return T_PREEMPT;

      case BI_REPLACEBICALL: 
	PC += 3;
	Assert(!e->isEmptyPreparedCalls());
	goto LBLreplaceBICall;

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(TESTBI)
    {
      Builtin* bi = GetBI(PC+1);
      OZ_Location* loc = GetLoc(PC+2);

      Assert(bi->getOutArity()>=1);

#ifdef PROFILE_BI
      bi->incCounter();
#endif
      int ret = bi->getFun()(loc->getMapping());
      if (ret==PROCEED) {
	if (oz_isTrue(loc->getOutValue(bi,0))) {
	  DISPATCH(4);
	} else {
	  Assert(oz_isFalse(loc->getOutValue(bi,0)));
	  JUMPPC(3);
	}
      }

      switch (ret) {
      case RAISE:
	if (e->exception.debug) 
	  set_exception_info_call(bi,loc);
	RAISE_THREAD;
      case BI_TYPE_ERROR:
	RAISE_TYPE(bi,loc);
	 
      case SUSPEND:
	PushContX(PC);
	SUSPENDONVARLIST;

	// kost@, PER - added handling 18.01.99
      case BI_REPLACEBICALL: 
	PC += 4;
	Assert(!e->isEmptyPreparedCalls());
	goto LBLreplaceBICall;

      case BI_PREEMPT:
      case SLEEP:
      case FAILED:
      default:
	Assert(0);
      }
    }


  Case(INLINEMINUS)
    {

#if defined(FASTARITH) && defined(__GNUC__) && defined(__i386__) && defined(REGOPT) && defined(FASTERREGACCESS)

      {
	register TaggedRef A, B;
	
        asm volatile("movl   4(%2),%0
                      movl   8(%2),%1
                      movl   (%0),%0
                      movl   (%1),%1
                           
                     "
                     : "=r" (A), "=r" (B) : "r" (PC));
	
      retryINLINEMINUSAF:
	A = A ^ TAG_SMALLINT;
	if (!(A & TAG_MASK)) {
	retryINLINEMINUSBF:
	  B = B ^ TAG_SMALLINT;
	  if (!(B & TAG_MASK)) {

	    asm volatile("   subl %2,%1
                             jo   0f
                             movl 12(%3),%2
                             addl $16,%3
                             orl  %0,%1
                             movl %1,(%2)
                             jmp *(%3)
                          0:
                       "
                       :  /* OUTPUT */
                       :  /* INPUT  */
                          "i" (TAG_SMALLINT),
                          "r" (A),
                          "r" (B),
		          "r" (PC)
                       );

	  } else {
	    B = B ^ TAG_SMALLINT;
	    if (oz_isRef(B)) {
	      B = oz_derefOne(B);
	      goto retryINLINEMINUSBF;
	    }
	  }
	} else {
	  A = A ^ TAG_SMALLINT;
	  if (oz_isRef(A)) {
	    A = oz_derefOne(A);
	    goto retryINLINEMINUSAF;
	  }
	}
      }
      
#endif

      TaggedRef A = XPC(1); 

    retryINLINEMINUSA:

      if (oz_isSmallInt(A)) {
	TaggedRef B = XPC(2);

      retryINLINEMINUSB1:

	if (oz_isSmallInt(B)) {
	  XPC(3)=oz_int(tagged2SmallInt(A) - tagged2SmallInt(B));
	  DISPATCH(4);
	}

	if (oz_isRef(B)) {
	  B = oz_derefOne(B);
	  goto retryINLINEMINUSB1;
	}

      }
      
      if (oz_isFloat(A)) {
	TaggedRef B = XPC(2);
	
      retryINLINEMINUSB2:

	if (oz_isFloat(B)) {
	  XPC(3) = oz_float(floatValue(A) - floatValue(B));
	  DISPATCH(4);
	}

	if (oz_isRef(B)) {
	  B = oz_derefOne(B);
	  goto retryINLINEMINUSB2;
	}

      }
            
      if (oz_isRef(A)) {
	A = oz_derefOne(A);
	goto retryINLINEMINUSA;
      }

      auxTaggedA = XPC(1);
      auxTaggedB = XPC(2);
      auxInt     = 4;
      auxString = "-";

      tmpRet = BIminusInline(auxTaggedA,auxTaggedB,XPC(3));
      goto LBLhandlePlusMinus;
    }

  Case(INLINEPLUS)
    {

#if defined(FASTARITH) && defined(__GNUC__) && defined(__i386__) && defined(REGOPT) && defined(FASTERREGACCESS)

      {
	register TaggedRef A, B;
	
        asm volatile("movl   4(%2),%0
                      movl   8(%2),%1
                      movl   (%0),%0
                      movl   (%1),%1
                           
                     "
                     : "=r" (A),
                       "=r" (B)
                     : "r" (PC));
	
      retryINLINEPLUSAF:
	A = A ^ TAG_SMALLINT;
	if (!(A & TAG_MASK)) {
	retryINLINEPLUSBF:
	  B = B ^ TAG_SMALLINT;
	  if (!(B & TAG_MASK)) {

	    asm volatile("   
                             addl %2,%1
                             jo   0f
                             movl 12(%3),%2
                             addl $16,%3
                             orl  %0,%1
                             movl %1,(%2)
                             jmp *(%3)
                          0:
                       "
                       :  /* OUTPUT */
                       :  /* INPUT  */
                          "i" (TAG_SMALLINT),
                          "r" (A),
                          "r" (B),
		          "r" (PC)
                       );

	  } else {
	    B = B ^ TAG_SMALLINT;
	    if (oz_isRef(B)) {
	      B = oz_derefOne(B);
	      goto retryINLINEPLUSBF;
	    }
	  }
	} else {
	  A = A ^ TAG_SMALLINT;
	  if (oz_isRef(A)) {
	    A = oz_derefOne(A);
	    goto retryINLINEPLUSAF;
	  }
	}
      }
      
#endif

      TaggedRef A = XPC(1); 

    retryINLINEPLUSA:

      if (oz_isSmallInt(A)) {
	TaggedRef B = XPC(2);

      retryINLINEPLUSB1:
	if (oz_isSmallInt(B)) {
	  XPC(3)=oz_int(tagged2SmallInt(A) + tagged2SmallInt(B));
	  DISPATCH(4);
	}

	if (oz_isRef(B)) {
	  B = oz_derefOne(B);
	  goto retryINLINEPLUSB1;
	}

      }
      
      if (oz_isFloat(A)) {
	TaggedRef B = XPC(2);
	
      retryINLINEPLUSB2:

	if (oz_isFloat(B)) {
	  XPC(3) = oz_float(floatValue(A) + floatValue(B));
	  DISPATCH(4);
	}

	if (oz_isRef(B)) {
	  B = oz_derefOne(B);
	  goto retryINLINEPLUSB2;
	}

      }
            
      if (oz_isRef(A)) {
	A = oz_derefOne(A);
	goto retryINLINEPLUSA;
      }

      auxTaggedA = XPC(1);
      auxTaggedB = XPC(2);
      auxInt     = 4;
      auxString = "+";

      tmpRet = BIplusInline(auxTaggedA,auxTaggedB,XPC(3));
      goto LBLhandlePlusMinus;
    }

  Case(INLINEMINUS1)
    {
#if defined(FASTARITH) && defined(__GNUC__) && defined(__i386__) && defined(REGOPT) && defined(FASTERREGACCESS)

      {
	register TaggedRef A, T;
	
        asm volatile("    movl   4(%1),%0
                          movl   (%0),%0
                     "
		     : "=r" (A) : "r" (PC)); 
	
      retryINLINEMINUS1:
	A = A ^ TAG_SMALLINT;
	if (!(A & TAG_MASK)) {
	  asm volatile("   addl $12,%3
                           movl -4(%3),%2
                           addl %0,%1
                           jo   0f
                           movl %1,(%2)
                           jmp *(%3)
                        0: movl %4,%1
                           movl %1,(%2)
                           jmp *(%3)
                       "
                       :  
                       : "i" (TAG_SMALLINT - (1 << TAG_SIZE)),
		         "r" (A),
                         "r" (T),
		         "r" (PC),
		         "m" (TaggedOzOverMinInt)
                       );
	} else {
	  A = A ^ TAG_SMALLINT;
	  if (oz_isRef(A)) {
	    A = oz_derefOne(A);
	    goto retryINLINEMINUS1;
	  }
	}
      }
      
#else

      TaggedRef A = XPC(1);

    retryINLINEMINUS1:

      if (oz_isSmallInt(A)) {
	/* INTDEP */
	if (A != TaggedOzMinInt) {
	  XPC(2) = (int) A - (1<<TAG_SIZE);
	  DISPATCH(3);
	} else {
	  XPC(2) = TaggedOzOverMinInt;
	  DISPATCH(3);
	} 
      }

      if (oz_isRef(A)) {
	A = oz_derefOne(A);
	goto retryINLINEMINUS1;
      }

#endif

      auxTaggedA = XPC(1);
      auxTaggedB = makeTaggedSmallInt(1);
      auxInt     = 3;
      auxString  = "-1";

      tmpRet = BIminusInline(auxTaggedA,auxTaggedB,XPC(2));
      goto LBLhandlePlusMinus;
    }

  Case(INLINEPLUS1)
    {

#if defined(FASTARITH) && defined(__GNUC__) && defined(__i386__) && defined(REGOPT) && defined(FASTERREGACCESS)

      {
	register TaggedRef A, T;
	
        asm volatile("    movl   4(%1),%0
                          movl   (%0),%0
                     "
		     : "=r" (A) : "r" (PC)); 
	
      retryINLINEPLUS1:
	A = A ^ TAG_SMALLINT;
	if (!(A & TAG_MASK)) {
	  asm volatile("   addl $12,%3
                           movl -4(%3),%2
                           addl %0,%1
                           jo   0f
                           movl %1,(%2)
                           jmp *(%3)
                        0: movl %4,%1
                           movl %1,(%2)
                           jmp *(%3)
                       "
                       :  
                       : "i" ((1 << TAG_SIZE) + TAG_SMALLINT),
		         "r" (A),
                         "r" (T),
		         "r" (PC),
		         "m" (TaggedOzOverMaxInt)
                       );
	} else {
	  A = A ^ TAG_SMALLINT;
	  if (oz_isRef(A)) {
	    A = oz_derefOne(A);
	    goto retryINLINEPLUS1;
	  }
	}
      }
      
#else

      TaggedRef A = XPC(1); 

    retryINLINEPLUS1:

      if (oz_isSmallInt(A)) {
	/* INTDEP */
	if (A != TaggedOzMaxInt) {
	  XPC(2) = (int) A + (1 << TAG_SIZE);
	  DISPATCH(3);
	} else {
	  XPC(2) = TaggedOzOverMaxInt;
	  DISPATCH(3);
	} 
      }

      if (oz_isRef(A)) {
	A = oz_derefOne(A);
	goto retryINLINEPLUS1;
      }

#endif

      auxTaggedA = XPC(1);
      auxTaggedB = makeTaggedSmallInt(1);
      auxInt     = 3;
      auxString = "+1";

      tmpRet = BIplusInline(auxTaggedA,auxTaggedB,XPC(2));
      goto LBLhandlePlusMinus;
    }


  LBLhandlePlusMinus:
  {
      switch(tmpRet) {
      case PROCEED:       DISPATCH(auxInt);
      case BI_TYPE_ERROR: RAISE_TYPE1_FUN(auxString, 
					  oz_mklist(auxTaggedA,auxTaggedB));

      case SUSPEND:
	{
	  CheckLiveness(PC);
	  PushContX(PC);
	  tmpRet = suspendInline(CTT,auxTaggedA,auxTaggedB);
	  MAGIC_RET;
	}
      default:    Assert(0);
      }
    }

  Case(INLINEDOT)
    {
      TaggedRef rec = XPC(1);
      DEREF(rec,_1,_2);
      if (oz_isSRecord(rec)) {
	SRecord *srec = tagged2SRecord(rec);
	TaggedRef feature = getLiteralArg(PC+2);
	int index = ((InlineCache*)(PC+4))->lookup(srec,feature);
	if (index<0) {
	  (void) oz_raise(E_ERROR,E_KERNEL,".", 2, XPC(1), feature);
	  RAISE_THREAD;
	}
	XPC(3) = srec->getArg(index);
	DISPATCH(6);	
      }
      {
	TaggedRef feature = getLiteralArg(PC+2);
	OZ_Return res = dotInline(XPC(1),feature,XPC(3));
	if (res==PROCEED) { DISPATCH(6); }

	switch(res) {
	case SUSPEND:
	  {
	    TaggedRef A=XPC(1);
	    CheckLiveness(PC);
	    PushContX(PC);
	    tmpRet = suspendInline(CTT,A);
	    MAGIC_RET;
	  }

	case RAISE:
	  RAISE_THREAD;

	case BI_TYPE_ERROR:
	  RAISE_TYPE1_FUN(".", oz_mklist(XPC(1),feature));

	case SLEEP:
	default:
	  Assert(0);
	}
      }
    }

  Case(INLINEAT)
    {
      TaggedRef fea = getLiteralArg(PC+1);
      Object *self = e->getSelf();

      Assert(e->getSelf()!=NULL);
      RecOrCell state = self->getState();
      SRecord *rec;
      if (stateIsCell(state)) {
	rec = getState(state,NO,fea,XPC(2));
	if (rec==NULL) {
	  PC += 5;
	  Assert(!e->isEmptyPreparedCalls());
	  goto LBLreplaceBICall;
	}
      } else {
	rec = getRecord(state);
      }
      Assert(rec!=NULL);
      int index = ((InlineCache*)(PC+3))->lookup(rec,fea);
      if (index>=0) {
	XPC(2) = rec->getArg(index);
	DISPATCH(5);
      }
      (void) oz_raise(E_ERROR,E_OBJECT,"@",2,makeTaggedConst(self),fea);
      RAISE_THREAD;
    }

  Case(INLINEASSIGN)
    {      
      TaggedRef fea = getLiteralArg(PC+1);

      Object *self = e->getSelf();

      if (!oz_onToplevel() && !oz_isCurrentBoard(GETBOARD(self))) {
	(void) oz_raise(E_ERROR,E_KERNEL,"globalState",1,AtomObject);
	RAISE_THREAD;
     }

      RecOrCell state = self->getState();
      SRecord *rec;
      
      if (stateIsCell(state)) {
	rec = getState(state,OK,fea,XPC(2));
	if (rec==NULL) {
	  PC += 5;
	  Assert(!e->isEmptyPreparedCalls());
	  goto LBLreplaceBICall;
	}
      } else {
	rec = getRecord(state);
      }
      Assert(rec!=NULL);
      int index = ((InlineCache*)(PC+3))->lookup(rec,fea);
      if (index>=0) {
	Assert(oz_isRef(*rec->getRef(index)) || !oz_isVariable(*rec->getRef(index)));
	rec->setArg(index,XPC(2));
	DISPATCH(5);
      }
      
      (void) oz_raise(E_ERROR,E_OBJECT,"<-",3,
		      makeTaggedConst(self), fea, XPC(2));
      RAISE_THREAD;
    }



// ------------------------------------------------------------------------
// INSTRUCTIONS: Testing
// ------------------------------------------------------------------------

#define LT_IF(T) if (T) THEN_CASE else ELSE_CASE
#define THEN_CASE { XPC(3)=oz_true();  DISPATCH(5); }
#define ELSE_CASE { XPC(3)=oz_false(); JUMPPC(4);   }

  Case(TESTLT)
    {
      TaggedRef A = XPC(1); DEREF0(A,_1,tagA);
      
      if (isSmallIntTag(tagA)) {
	TaggedRef B = XPC(2); DEREF0(B,_2,tagB);
	if (isSmallIntTag(tagB)) {
	  LT_IF(smallIntLess(A,B));
	}
      }
	
      if (isFloatTag(tagA)) {
	TaggedRef B = XPC(2); DEREF0(B,_2,tagB);
	if (isFloatTag(tagB)) {
	  LT_IF(floatValue(A) < floatValue(B));
	}
      }
	
      {
	TaggedRef B = XPC(2); DEREF0(B,_2,tagB);
	if (oz_isAtom(A) && oz_isAtom(B)) {
	  LT_IF(strcmp(tagged2Literal(A)->getPrintName(),
		       tagged2Literal(B)->getPrintName()) < 0);
	  }
      }
      tmpRet = BILessOrLessEq(OK,XPC(1),XPC(2));
      auxString = "<";
      goto LBLhandleLT;
    }

  LBLhandleLT:
    {
      switch(tmpRet) {
	
      case PROCEED: THEN_CASE;
      case FAILED:  ELSE_CASE;
	
      case SUSPEND:
	{
	  CheckLiveness(PC);
	  PushContX(PC);
	  tmpRet = suspendInline(CTT,XPC(1),XPC(2));
	  MAGIC_RET;
	}
      
      case BI_TYPE_ERROR:
	RAISE_TYPE1(auxString,oz_mklist(XPC(1),XPC(2)));
	
      default:
	Assert(0);
      }
    }

  Case(TESTLE)
    {
      TaggedRef A = XPC(1); DEREF0(A,_1,tagA);
      TaggedRef B = XPC(2); DEREF0(B,_2,tagB);
      
      if (tagA == tagB) {
	if (isSmallIntTag(tagA)) {
	  LT_IF(smallIntLE(A,B));
	}
	
	if (isFloatTag(tagA)) {
	  LT_IF(floatValue(A) <= floatValue(B));
	}
	
	if (isLiteralTag(tagA)) {
	  if (oz_isAtom(A) && oz_isAtom(B)) {
	    LT_IF(strcmp(tagged2Literal(A)->getPrintName(),
			 tagged2Literal(B)->getPrintName()) <= 0);
	  }
	}
      }
      tmpRet = BILessOrLessEq(NO,XPC(1),XPC(2));
      auxString = "=<";
      goto LBLhandleLT;
    }

#undef THEN_CASE
#undef ELSE_CASE

// -------------------------------------------------------------------------
// INSTRUCTIONS: Environment
// -------------------------------------------------------------------------

  Case(ALLOCATEL)
    {
      int posInt = getPosIntArg(PC+1);
      Assert(posInt > 0);
      allocateY(posInt);
      DISPATCH(2);
    }

  Case(ALLOCATEL1)  { allocateY(1);  DISPATCH(1); }
  Case(ALLOCATEL2)  { allocateY(2);  DISPATCH(1); }
  Case(ALLOCATEL3)  { allocateY(3);  DISPATCH(1); }
  Case(ALLOCATEL4)  { allocateY(4);  DISPATCH(1); }
  Case(ALLOCATEL5)  { allocateY(5);  DISPATCH(1); }
  Case(ALLOCATEL6)  { allocateY(6);  DISPATCH(1); }
  Case(ALLOCATEL7)  { allocateY(7);  DISPATCH(1); }
  Case(ALLOCATEL8)  { allocateY(8);  DISPATCH(1); }
  Case(ALLOCATEL9)  { allocateY(9);  DISPATCH(1); }
  Case(ALLOCATEL10) { allocateY(10); DISPATCH(1); }

  Case(DEALLOCATEL1)  { deallocateYN(1);  DISPATCH(1); }
  Case(DEALLOCATEL2)  { deallocateYN(2);  DISPATCH(1); }
  Case(DEALLOCATEL3)  { deallocateYN(3);  DISPATCH(1); }
  Case(DEALLOCATEL4)  { deallocateYN(4);  DISPATCH(1); }
  Case(DEALLOCATEL5)  { deallocateYN(5);  DISPATCH(1); }
  Case(DEALLOCATEL6)  { deallocateYN(6);  DISPATCH(1); }
  Case(DEALLOCATEL7)  { deallocateYN(7);  DISPATCH(1); }
  Case(DEALLOCATEL8)  { deallocateYN(8);  DISPATCH(1); }
  Case(DEALLOCATEL9)  { deallocateYN(9);  DISPATCH(1); }
  Case(DEALLOCATEL10) { deallocateYN(10); DISPATCH(1); }

  Case(DEALLOCATEL)
    {
      if (Y)
	deallocateY();
      DISPATCH(1);
    }
// -------------------------------------------------------------------------
// INSTRUCTIONS: CONTROL: FAIL/SUCCESS/RETURN
// -------------------------------------------------------------------------

  Case(SKIP)
    DISPATCH(1);

  Case(EXHANDLER)
    PushCont(PC+2);
    oz_currentThread()->pushCatch();
    JUMPPC(1);

  Case(POPEX)
    {
      TaskStack *taskstack = CTS;
      taskstack->discardCatch();
      /* remove unused continuation for handler */
      taskstack->discardFrame(NOCODE);
      DISPATCH(1);
    }

  Case(LOCKTHREAD)
    {
      int lbl = getLabelArg(PC+1);
      TaggedRef aux      = XPC(2);
  
      DEREF(aux,auxPtr,_1);
      if (oz_isVariable(aux)) {
	SUSP_PC(auxPtr,PC);
      }
  
      if (!oz_isLock(aux)) {
	(void) oz_raise(E_ERROR,E_KERNEL,"type",5,
			NameUnit,
			NameUnit,
			oz_atomNoDup("Lock"),
			makeTaggedSmallInt(1),
			oz_nil());
	RAISE_TYPE1("lock",oz_mklist(aux));
      }
  
      OzLock *t = (OzLock*) tagged2Const(aux);
      Thread *th=oz_currentThread();
  
      if (t->isLocal()) {
	if (!oz_onToplevel()) {
	  if (!oz_isCurrentBoard(GETBOARD((LockLocal*)t))) {
	    (void) oz_raise(E_ERROR,E_KERNEL,"globalState",1,AtomLock);
	    RAISE_THREAD;
	  }
	}
	if (((LockLocal*)t)->hasLock(th))
	  goto has_lock;
	if (((LockLocal*)t)->lockB(th))
	  goto got_lock;
	goto no_lock;
      }

      if (!oz_onToplevel()) {
	(void) oz_raise(E_ERROR,E_KERNEL,"globalState",1,AtomLock);
	RAISE_THREAD;
      }

      LockRet ret;
  
      switch (t->getTertType()) {
      case Te_Frame:
	{
	  if (((LockFrameEmul *)t)->hasLock(th))
	    goto has_lock;
	  ret = ((LockFrameEmul *)t)->lockB(th);
	  break;
	}
      case Te_Proxy:
	{
	  (*lockLockProxy)(t, th);
	  goto no_lock;
	}
      case Te_Manager:
	{
	  if (((LockManagerEmul *)t)->hasLock(th)) 
	    goto has_lock;
	  ret=((LockManagerEmul *)t)->lockB(th);
	  break;
	}
      default:
	Assert(0);
      }

      if (ret==LOCK_GOT) 
	goto got_lock;
      if (ret==LOCK_WAIT) 
	goto no_lock;

      PushCont(PC+lbl); // failure preepmtion 
      PC += 3;
      Assert(!e->isEmptyPreparedCalls());
      goto LBLreplaceBICall;
  
    got_lock:
      PushCont(PC+lbl);
      CTS->pushLock(t);
      DISPATCH(3);
  
    has_lock:
      PushCont(PC+lbl);
      DISPATCH(3);
  
    no_lock:
      PushCont(PC+lbl);
      CTS->pushLock(t);
      PC += 3;
      Assert(!e->isEmptyPreparedCalls());
      goto LBLreplaceBICall;

    }

  Case(RETURN)

    LBLpopTask:
      {
	emulateHookPopTask(e);

	Assert(!CTT->isSuspended());

      LBLpopTaskNoPreempt:
	Assert(CTS==CTT->getTaskStackRef());
	PopFrameNoDecl(CTS,PC,Y,CAP);
	JUMPABSOLUTE(PC);
      }

// ------------------------------------------------------------------------
// INSTRUCTIONS: Definition
// ------------------------------------------------------------------------

  Case(DEFINITIONCOPY)
    isTailCall = OK; // abuse for indicating that we have to copy
    goto LBLDefinition;

  Case(DEFINITION)
      isTailCall = NO;

    LBLDefinition:
    {
      PrTabEntry *predd           = getPredArg(PC+3);
      AbstractionEntry *predEntry = (AbstractionEntry*) getAdressArg(PC+4);
      AssRegArray *list           = (AssRegArray*) getAdressArg(PC+5);
      int size = list->getSize();

      if (predd->getPC()==NOCODE) {
        predd->PC = PC+sizeOf(DEFINITION);
	predd->setGSize(size);
      }
      
      if (am.profileMode())
	predd->getProfile()->numClosures++;
      
      if (isTailCall) { // was DEFINITIONCOPY?
	TaggedRef list = oz_deref(XPC(1));
	ProgramCounter preddPC = predd->PC;
	predd = new PrTabEntry(predd->getName(), predd->getMethodArity(),
			       predd->getFile(), predd->getLine(), predd->getColumn(),
			       oz_nil(), // mm2: inherit sited?
			       predd->getMaxX());
	predd->PC = copyCode(preddPC,list);
	predd->setGSize(size);
      }

      Abstraction *p = Abstraction::newAbstraction(predd, CBB);

      if (predEntry) {
	predEntry->setPred(p);
      }

      for (int i = 0; i < size; i++) {
	switch ((*list)[i].getKind()) {
	case K_XReg: p->initG(i, XREGS[(*list)[i].getIndex()]); break;
	case K_YReg: p->initG(i, Y[(*list)[i].getIndex()]); break;
	case K_GReg: p->initG(i, CAP->getG((*list)[i].getIndex())); break;
	}
      }
      XPC(1) = makeTaggedConst(p);
      JUMPPC(2);
    }

// -------------------------------------------------------------------------
// INSTRUCTIONS: CONTROL: FENCE/CALL/EXECUTE/SWITCH/BRANCH
// -------------------------------------------------------------------------
  
  Case(BRANCH)
    JUMPPC(1);

  
  Case(TAILSENDMSGX) 
    isTailCall = OK; SETAUX(XPC(2)); goto SendMethod;
  Case(TAILSENDMSGY) 
    isTailCall = OK; SETAUX(YPC(2)); goto SendMethod;
  Case(TAILSENDMSGG) 
    isTailCall = OK; SETAUX(GPC(2)); goto SendMethod;

  Case(SENDMSGY) 
    isTailCall = NO; SETAUX(YPC(2)); goto SendMethod;
  Case(SENDMSGG) 
    isTailCall = NO; SETAUX(GPC(2)); goto SendMethod;
  Case(SENDMSGX) 
    isTailCall = NO; SETAUX(XPC(2)); /* fall through */

 SendMethod:
  {
    TaggedRef label    = getLiteralArg(PC+1);
    TaggedRef origObj  = auxTaggedA;
    TaggedRef object   = origObj;
    SRecordArity arity = (SRecordArity) getAdressArg(PC+3);

    DEREF(object,objectPtr,_2);
    if (oz_isObject(object)) {
      Object *obj      = tagged2Object(object);
      Abstraction *def = getSendMethod(obj,label,arity,(InlineCache*)(PC+4));
      if (def == NULL) {
	goto bombSend;
      }

      if (!isTailCall) PushCont(PC+6);
      ChangeSelf(obj);
      CallDoChecks(def);
      JUMPABSOLUTE(def->getPC());
    }

    if (oz_isVariable(object)) {
      SUSP_PC(objectPtr,PC);
    }

    if (oz_isProcedure(object)) 
      goto bombSend;

    RAISE_APPLY(object, oz_mklist(makeMessage(arity,label)));

  bombSend:
    if (!isTailCall) PC = PC+6;
    XREGS[0] = makeMessage(arity,label);
    predArity = 1;
    predicate = tagged2Const(object);
    goto LBLcall;
  }


  


  Case(CALLX) 
    isTailCall = NO; SETAUX(XPC(1)); goto Call;
  Case(CALLY) 
    isTailCall = NO; SETAUX(YPC(1)); goto Call;
  Case(CALLG) 
    isTailCall = NO; SETAUX(GPC(1)); goto Call;

  Case(TAILCALLG) 
    isTailCall = OK; SETAUX(GPC(1)); goto Call;
  Case(TAILCALLX) 
    isTailCall = OK; SETAUX(XPC(1)); /* fall through */

 Call:
  asmLbl(TAILCALL);
  {
     {
       TaggedRef taggedPredicate = auxTaggedA;
       predArity = getPosIntArg(PC+2);

       DEREF(taggedPredicate,predPtr,_1);

       if (oz_isAbstraction(taggedPredicate)) {
         Abstraction *def = tagged2Abstraction(taggedPredicate);
	 PrTabEntry *pte = def->getPred();
         CheckArity(pte->getArity(), taggedPredicate);
         if (!isTailCall) { PushCont(PC+3); }
         CallDoChecks(def);
         JUMPABSOLUTE(pte->getPC());
       }

       if (!oz_isProcedure(taggedPredicate) && !oz_isObject(taggedPredicate)) {
	 if (oz_isVariable(taggedPredicate)) {
	   SUSP_PC(predPtr,PC);
	 }
	 RAISE_APPLY(taggedPredicate,OZ_toList(predArity,XREGS));
       }

       if (!isTailCall) PC = PC+3;
       predicate = tagged2Const(taggedPredicate);
     }

// -----------------------------------------------------------------------
// --- Call: entry point
// -----------------------------------------------------------------------

  LBLcall:
     Builtin *bi;

// -----------------------------------------------------------------------
// --- Call: Abstraction
// -----------------------------------------------------------------------

     {
       TypeOfConst typ = predicate->getType();

       if (typ==Co_Abstraction) {
	 Abstraction *def = (Abstraction *) predicate;
	 CheckArity(def->getArity(), makeTaggedConst(def));
	 if (!isTailCall) { PushCont(PC); }
	 CallDoChecks(def);
	 JUMPABSOLUTE(def->getPC());
       }

// -----------------------------------------------------------------------
// --- Call: Object
// -----------------------------------------------------------------------
       if (typ==Co_Object) {
	 CheckArity(1, makeTaggedConst(predicate));
	 Object *o = (Object*) predicate;
	 Assert(o->getClass()->getFallbackApply());
	 Abstraction *def =
	   tagged2Abstraction(o->getClass()->getFallbackApply());
	 /* {Obj Msg} --> {SetSelf Obj} {FallbackApply Class Msg} */
	 XREGS[1] = XREGS[0];
	 XREGS[0] = makeTaggedConst(o->getClass());
	 predArity = 2;
	 if (!isTailCall) { PushCont(PC); }
	 ChangeSelf(o);
	 CallDoChecks(def);
	 JUMPABSOLUTE(def->getPC());
       }

// -----------------------------------------------------------------------
// --- Call: Builtin
// -----------------------------------------------------------------------
       Assert(typ==Co_Builtin);
     
       bi = (Builtin *) predicate;
	
       CheckArity(bi->getArity(),makeTaggedConst(bi));
	   
#ifdef PROFILE_BI
       bi->incCounter();
#endif
       OZ_Return res = oz_bi_wrapper(bi);
	     
       switch (res) {
	    
       case SUSPEND:
	 {
	   if (!isTailCall) PushCont(PC);

	   CTT->pushCall(makeTaggedConst(bi),XREGS,predArity);
	   SUSPENDONVARLIST;
	 }

       case PROCEED:
	 if (isTailCall) {
	   goto LBLpopTask;
	 }
	 JUMPABSOLUTE(PC);
	 
       case SLEEP:         Assert(0);
       case RAISE:
	 if (e->exception.debug) 
	   set_exception_info_call(bi, OZ_ID_LOC);

         RAISE_THREAD;
       case BI_TYPE_ERROR: 
	 RAISE_TYPE(bi,OZ_ID_LOC);
       case FAILED:        
	 HF_BI(bi,OZ_ID_LOC);

       case BI_PREEMPT:
	 if (!isTailCall) {
	   PushCont(PC);
	 }
	 return T_PREEMPT;
	 
      case BI_REPLACEBICALL: 
	if (isTailCall) {
	  PC=NOCODE;
	}
	Assert(!e->isEmptyPreparedCalls());
	goto LBLreplaceBICall;

       default: Assert(0);
       }
     }
     Assert(0);

   LBLhandleRet:
     switch (tmpRet) {
     case RAISE: RAISE_THREAD;
     case BI_REPLACEBICALL: 
       PC=NOCODE; 
       Assert(!e->isEmptyPreparedCalls());
       goto LBLreplaceBICall;
     default: break;
     }
     Assert(0);

   LBLMagicRet:
     {
       if (tmpRet == SUSPEND) return T_SUSPEND;
       if (tmpRet == PROCEED) goto LBLpopTask;
       Assert(tmpRet == RAISE || tmpRet == BI_REPLACEBICALL);
       goto LBLhandleRet;
     }


// ------------------------------------------------------------------------
// --- Call: Builtin: replaceBICall
// ------------------------------------------------------------------------

   LBLreplaceBICall:
     {
       if (PC != NOCODE) {
	 PushContX(PC);
       }

       e->pushPreparedCalls();

       if (!e->isEmptySuspendVarList()) {
	 SUSPENDONVARLIST;
       }
       goto LBLpopTask;
     }
   }
// --------------------------------------------------------------------------
// --- end call/execute -----------------------------------------------------
// --------------------------------------------------------------------------


// -------------------------------------------------------------------------
// INSTRUCTIONS: MISC: ERROR/NOOP/default
// -------------------------------------------------------------------------

  Case(TASKEMPTYSTACK)  
    {
      Assert(Y==0 && CAP==0);
      CTS->pushEmpty();   // mm2: is this really needed?
      DebugCode(e->cachedSelf=0);
      return T_TERMINATE;
    }

  Case(TASKPROFILECALL)
    {
      ozstat.leaveCall((PrTabEntry*) Y);
      goto LBLpopTaskNoPreempt;
    }

  Case(TASKCALLCONT)
    {
      TaggedRef taggedPredicate = (TaggedRef)ToInt32(Y);
      RefsArray args = (RefsArray) CAP;
      Y = 0;
      CAP = 0;

      DebugTrace(ozd_trace(toC(taggedPredicate)));

      predArity = args ? getRefsArraySize(args) : 0;

      DEREF(taggedPredicate,predPtr,predTag);
      if (!oz_isProcedure(taggedPredicate) && !oz_isObject(taggedPredicate)) {
	if (isVariableTag(predTag)) {
	  CTS->pushCallNoCopy(makeTaggedRef(predPtr),args);
	  tmpRet = oz_var_addSusp(predPtr,CTT);
	  MAGIC_RET;
	}
	RAISE_APPLY(taggedPredicate,OZ_toList(predArity,args));
      }

      int i = predArity;
      while (--i >= 0) {
	XREGS[i] = args[i];
      }
      disposeRefsArray(args);
      isTailCall = OK;

      predicate=tagged2Const(taggedPredicate);
      goto LBLcall;
    }

  Case(TASKLOCK)
    {
      OzLock *lck = (OzLock *) CAP;
      CAP = (Abstraction *) NULL;
      switch(lck->getTertType()){
      case Te_Local:
	((LockLocal*)lck)->unlock();
	break;
      case Te_Frame:
	((LockFrameEmul *)lck)->unlock(oz_currentThread());
	break;
      case Te_Proxy:
	oz_raise(E_ERROR,E_KERNEL,"globalState",1,AtomLock);
	RAISE_THREAD_NO_PC;
      case Te_Manager:
	((LockManagerEmul *)lck)->unlock(oz_currentThread());
	break;}
      goto LBLpopTaskNoPreempt;
    }

  Case(TASKXCONT)
    {
      RefsArray tmpX = Y;
      Y = NULL;
      int i = getRefsArraySize(tmpX);
      while (--i >= 0) {
	XREGS[i] = tmpX[i];
      }
      disposeRefsArray(tmpX);
      goto LBLpopTaskNoPreempt;
    }

  Case(TASKSETSELF)
    {
      e->setSelf((Object *) CAP);
      CAP = NULL;
      goto LBLpopTaskNoPreempt;
    }

  Case(TASKDEBUGCONT)
    {
      Assert(0);
      goto LBLpopTaskNoPreempt;
    }

  Case(OZERROR)
    {
      return T_ERROR;
    }

  Case(DEBUGENTRY)
    {
      ThreadReturn ret = debugEntry(PC,Y,CAP);
      if (ret != T_OKOK)
	return ret;
      DISPATCH(5);
    }

  Case(DEBUGEXIT)
    {
      ThreadReturn ret = debugExit(PC,Y,CAP);
      if (ret != T_OKOK)
	return ret;
      DISPATCH(5);
    }

  Case(CALLPROCEDUREREF)
    {
      AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);
      Bool tailcall           =  getPosIntArg(PC+2) & 1;

      if (entry->getAbstr() == 0) {
	(void) oz_raise(E_ERROR,E_SYSTEM,"inconsistentFastcall",0);
	RAISE_THREAD;
      }
      CodeArea::writeOpcode(tailcall ? FASTTAILCALL : FASTCALL, PC);
      DISPATCH(0);
    }

  Case(CALLCONSTANT)
    {
      TaggedRef pred = getTaggedArg(PC+1);
      int tailcallAndArity  = getPosIntArg(PC+2);

      DEREF(pred,predPtr,_1);
      if (oz_isVariable(pred)) {
	SUSP_PC(predPtr,PC);
      }

      if (oz_isAbstraction(pred)) {
	CodeArea *code = CodeArea::findBlock(PC);
	AbstractionEntry *entry = new AbstractionEntry(NO);
	entry->setPred(tagged2Abstraction(pred));
	CodeArea::writeOpcode((tailcallAndArity&1)? FASTTAILCALL: FASTCALL,PC);
	code->writeAbstractionEntry(entry, PC+1);
	DISPATCH(0);
      }

      if (oz_isBuiltin(pred) || oz_isObject(pred)) {
	isTailCall = tailcallAndArity & 1;
	if (!isTailCall) PC += 3;
	predArity = tailcallAndArity >> 1;
	predicate = tagged2Const(pred);
	goto LBLcall;
      }
      RAISE_APPLY(pred,oz_mklist(OZ_atom("proc or builtin expected.")));
    }

  Case(CALLGLOBAL)
    {
      TaggedRef pred = GPC(1);
      int tailcallAndArity  = getPosIntArg(PC+2);
      Bool tailCall = tailcallAndArity & 1;
      int arity     = tailcallAndArity >> 1;

      DEREF(pred,predPtr,_1);
      if (oz_isVariable(pred)) {
	SUSP_PC(predPtr,PC);
      }

      if(oz_isAbstraction(pred)) {
	Abstraction *abstr = tagged2Abstraction(pred);
	if (abstr->getArity() == arity) { 
	  patchToFastCall(abstr,PC,tailCall);
	  DISPATCH(0);
	}
      }

      CodeArea::writeArity(arity, PC+2);
      CodeArea::writeOpcode(tailCall ? TAILCALLG : CALLG,PC);
      DISPATCH(0);
    }

  Case(CALLMETHOD)
    {
      CallMethodInfo *cmi = (CallMethodInfo*)getAdressArg(PC+1);
      TaggedRef cls = CAP->getG(cmi->regIndex);
      DEREF(cls,clsPtr,_1);
      if (oz_isVariable(cls)) {
	SUSP_PC(clsPtr,PC);
      }

      if (oz_isClass(cls)) {
	Bool defaultsUsed;
	Abstraction *abstr = tagged2ObjectClass(cls)->getMethod(cmi->mn,cmi->arity,
								NO,defaultsUsed);
	/* fill cache and try again later */
	if (abstr==NULL || defaultsUsed) {
	  isTailCall = cmi->isTailCall;
	  if (!isTailCall) PC = PC+3;
	  
	  Assert(tagged2ObjectClass(cls)->getFallbackApply());
	
	  XREGS[1] = makeMessage(cmi->arity,cmi->mn);
	  XREGS[0] = cls;
	
	  predArity = 2;
	  predicate = tagged2Const(tagged2ObjectClass(cls)->getFallbackApply());
	  goto LBLcall;
	}
	patchToFastCall(abstr,PC,cmi->isTailCall);
	cmi->dispose();
	DISPATCH(0);
      }

      (void) oz_raise(E_ERROR,E_KERNEL,"type",5,AtomComma,
		      oz_mklist(cls,makeMessage(cmi->arity,cmi->mn)),
		      AtomClass,oz_int(1),oz_nil());
      RAISE_THREAD;
      
    }



  /* The following must be different from the following,
   * otherwise definitionEnd breaks under threaded code
   */

  Case(GLOBALVARNAME)
    {
      // Trick: just do something weird
      DISPATCH(4711);
    }

  Case(LOCALVARNAME)
    {
      // Trick: just do something weird, but something different
      goto LBLcall;
    }

  Case(PROFILEPROC)
    {

      static int sizeOfDef = -1;
      if (sizeOfDef==-1) sizeOfDef = sizeOf(DEFINITION);
      
      Assert(CodeArea::getOpcode(PC-sizeOfDef) == DEFINITION);
      PrTabEntry *pred = getPredArg(PC-sizeOfDef+3); /* this is faster */

      pred->getProfile()->numCalled++;
      if (pred!=ozstat.currAbstr) {
	CTS->pushAbstr(ozstat.currAbstr);
	ozstat.leaveCall(pred);
      }
      
      DISPATCH(1);
    }

  Case(TASKCATCH)
    {
      Assert(0);
      DISPATCH(1);
    }

  Case(ENDOFFILE)
    {
      return T_ERROR;
    }

  Case(ENDDEFINITION)
    {
      return T_ERROR;
    }

#ifndef THREADED
  default:
    Assert(0);
     break;
   } /* switch*/
#endif


// ----------------- end instructions -------------------------------------


// ------------------------------------------------------------------------
// *** Special return values from unify: SUSPEND, EXCEPTION, etc.
// ------------------------------------------------------------------------

  LBLunifySpecial:
  {
    switch (tmpRet) {
    case BI_REPLACEBICALL:
      Assert(!e->isEmptyPreparedCalls());
      goto LBLreplaceBICall;
    case SUSPEND:
      PushContX(PC);
      SUSPENDONVARLIST;
    case RAISE:
      RAISE_THREAD;
    default:
      Assert(0);
    }
  }


  /*
   * Raise exception
   *
   */

 LBLraise:
  {
    Bool foundHdl;
    Thread *ct = CTT;

    if (e->exception.debug) {
      OZ_Term traceBack;
      foundHdl =
	ct->getTaskStackRef()->findCatch(ct,
					 e->exception.pc,
					 e->exception.y, 
					 e->exception.cap,
					 &traceBack,
					 e->debugmode());
	
      e->exception.value = formatError(e->exception.info,
				       e->exception.value,
				       traceBack);
    } else {
      foundHdl = ct->getTaskStackRef()->findCatch(ct);
    }
      
    if (foundHdl) {
      if (e->debugmode() && ct->isTrace())
	debugStreamUpdate(ct);
      XREGS[0] = e->exception.value;
      goto LBLpopTaskNoPreempt;
    }
      
    if (!CBB->isRoot() &&
	OZ_eq(OZ_label(e->exception.value),AtomFailure)) {
      return T_FAILURE;
    }

    if (e->debugmode()) {
      ct->setTrace();
      ct->setStep();
      debugStreamException(ct,e->exception.value);
      return T_PREEMPT;
    }

    if (e->defaultExceptionHdl) {
      ct->pushCall(e->defaultExceptionHdl,e->exception.value);
    } else {
      prefixError();
      fprintf(stderr,
	      "Exception raised:\n   %s\n",
	      OZ_toC(e->exception.value,100,100));
      fflush(stderr);
    }

    goto LBLpopTaskNoPreempt;
  }


  Assert(0);
  return T_ERROR;

} // end engine


#undef DISPATCH


static 
ThreadReturn debugEntry(ProgramCounter PC, RefsArray Y, Abstraction * CAP) {
  register AM * const e	       Reg4 = &am;

  if ((e->debugmode() || CTT->isTrace()) && oz_onToplevel()) {
    int line = tagged2SmallInt(getNumberArg(PC+2));
    if (line < 0) {
      execBreakpoint(oz_currentThread());
    }
    
    OzDebug * dbg = new OzDebug(PC,Y,CAP);
    
    TaggedRef kind = getTaggedArg(PC+4);
    if (oz_eq(kind,AtomDebugCallC) ||
	oz_eq(kind,AtomDebugCallF)) {
      // save abstraction and arguments:
      Bool copyArgs = NO;
      switch (CodeArea::getOpcode(PC+5)) {
      case CALLBI:
	{
	  Builtin *bi = GetBI(PC+6);
	  dbg->data = makeTaggedConst(bi);
	  int iarity = bi->getInArity(), oarity = bi->getOutArity();
	  int arity = iarity + oarity;
	  OZ_Location * loc = GetLoc(PC+7);
	  dbg->arity = arity;
	  if (arity > 0) {
	    dbg->arguments =
	      (TaggedRef *) freeListMalloc(sizeof(TaggedRef) * arity);
	    for (int i = iarity; i--; )
	      dbg->arguments[i] = loc->getInValue(i);
	    if (CTT->isStep())
	      for (int i = oarity; i--; )
		dbg->arguments[iarity + i] = OZ_newVariable();
	    else
	      for (int i = oarity; i--; )
		dbg->arguments[iarity + i] = NameVoidRegister;
	  }
	}
	break;
      case CALLX:
	dbg->data  = XPC(6);
	dbg->arity = getPosIntArg(PC+7);
	copyArgs = OK;
	break;
      case CALLY:
	dbg->data  = YPC(6);
	dbg->arity = getPosIntArg(PC+7);
	copyArgs = OK;
	break;
      case CALLG:
	dbg->data  = GPC(6);
	dbg->arity = getPosIntArg(PC+7);
	copyArgs = OK;
	break;
      case CALLPROCEDUREREF:
      case FASTCALL:
	{
	  Abstraction *abstr =
	    ((AbstractionEntry *) getAdressArg(PC+6))->getAbstr();
	  dbg->data = makeTaggedConst(abstr);
	  dbg->arity = abstr->getArity();
	  copyArgs = OK;
	}
	break;
      case CALLCONSTANT:
	dbg->data = getTaggedArg(PC+6);
	dbg->arity = getPosIntArg(PC+7) >> 1;
	copyArgs = OK;
	break;
      default:
	break;
      }
      if (copyArgs && dbg->arity > 0) {
	dbg->arguments =
	  (TaggedRef *) freeListMalloc(sizeof(TaggedRef) * dbg->arity);
	for (int i = dbg->arity; i--; )
	  dbg->arguments[i] = XREGS[i];
      }
    } else if (oz_eq(kind,AtomDebugLockC) ||
	       oz_eq(kind,AtomDebugLockF)) {
      // save the lock:
      switch (CodeArea::getOpcode(PC+5)) {
      case LOCKTHREAD:
	dbg->setSingleArgument(XPC(7));
	break;
      default:
	break;
      }
    } else if (oz_eq(kind,AtomDebugCondC) ||
	       oz_eq(kind,AtomDebugCondF)) {
      // look whether we can determine the arbiter:
      switch (CodeArea::getOpcode(PC+5)) {
      case TESTLITERALX:
      case TESTNUMBERX:
      case TESTRECORDX:
      case TESTLISTX:
      case TESTBOOLX:
      case MATCHX:
	dbg->setSingleArgument(XPC(6));
	break;
      case TESTLITERALY:
      case TESTNUMBERY:
      case TESTRECORDY:
      case TESTLISTY:
      case TESTBOOLY:
      case MATCHY:
	dbg->setSingleArgument(YPC(6));
	break;
      case TESTLITERALG:
      case TESTNUMBERG:
      case TESTRECORDG:
      case TESTLISTG:
      case TESTBOOLG:
      case MATCHG:
	dbg->setSingleArgument(GPC(6));
	break;
      default:
	break;
      }
    } else if (oz_eq(kind,AtomDebugNameC) ||
	       oz_eq(kind,AtomDebugNameF)) {
      switch (CodeArea::getOpcode(PC+5)) {
      case PUTCONSTANTX:
      case PUTCONSTANTY:
      case GETLITERALX:
      case GETLITERALY:
      case GETLITERALG:
	dbg->setSingleArgument(getTaggedArg(PC+6));
	break;
      default:
	break;
      }
    }
    
    if (CTT->isStep()) {
      CTT->pushDebug(dbg,DBG_STEP_ATOM);
      debugStreamEntry(dbg,CTT->getTaskStackRef()->getFrameId());
      INCFPC(5);
      PushContX(PC);
      return T_PREEMPT;
    } else {
      CTT->pushDebug(dbg,DBG_NOSTEP_ATOM);
    }
  } else if (e->isPropagatorLocation()) {
    OzDebug *dbg = new OzDebug(PC,NULL,CAP);
    CTT->pushDebug(dbg,DBG_EXIT_ATOM);
  }
  return T_OKOK;
}


static 
ThreadReturn debugExit(ProgramCounter PC, RefsArray Y, Abstraction * CAP) {
  register AM * const e	       Reg4 = &am;

  OzDebug *dbg;
  Atom * dothis;
  CTT->popDebug(dbg, dothis);

  if (dbg != (OzDebug *) NULL) {
    Assert(oz_eq(getLiteralArg(dbg->PC+4),getLiteralArg(PC+4)));
    Assert(e->isPropagatorLocation() || 
	   (dbg->Y == Y && 
	    ((Abstraction *) tagged2Const(dbg->CAP)) == CAP));
    
    if (dothis != DBG_EXIT_ATOM
	&& (oz_eq(getLiteralArg(PC+4),AtomDebugCallC) ||
	    oz_eq(getLiteralArg(PC+4),AtomDebugCallF))
	&& CodeArea::getOpcode(dbg->PC+5) == CALLBI) {
      Builtin *bi = GetBI(dbg->PC+6);
      int iarity        = bi->getInArity();
      int oarity        = bi->getOutArity();
      OZ_Location * loc = GetLoc(dbg->PC+7);
      if (oarity > 0)
	if (dbg->arguments[iarity] != NameVoidRegister)
	  for (int i = oarity; i--; ) {
	    TaggedRef x = loc->getOutValue(bi,i);
	    if (OZ_unify(dbg->arguments[iarity + i], x) == FAILED)
	      return T_FAILURE;
	  }
	else
	  for (int i = oarity; i--; )
	    dbg->arguments[iarity + i] = loc->getInValue(i);
    }
    
    if (dothis == DBG_STEP_ATOM && CTT->isTrace()) {
      dbg->PC = PC;
      CTT->pushDebug(dbg,DBG_EXIT_ATOM);
      debugStreamExit(dbg,CTT->getTaskStackRef()->getFrameId());
      PushContX(PC);
      return T_PREEMPT;
    }
    
    dbg->dispose();
  }
  return T_OKOK;
}



#define DISPATCH(incPC,incArgs) 		\
   PC += incPC;					\
   argsToHandle += incArgs;			\
   break;

static
void buildRecord(ProgramCounter PC, RefsArray Y, Abstraction *CAP) {
  Assert(oz_onToplevel());

  TaggedRef *sPointer, *TMPA, *TMPB;

  int argsToHandle = 0;
  
  int maxX = (CAP->getPred()->getMaxX()) * sizeof(TaggedRef);
  int maxY = (Y ? getRefsArraySize(Y) : 0) * sizeof(TaggedRef);

  void * savedY;

  if (maxX > 0)
    memcpy(XREGS_SAVE, XREGS, maxX);
  if (maxY > 0)
    savedY = memcpy(freeListMalloc(maxY), Y, maxY);
  
  Bool firstCall = OK;

  while (OK) {
    Opcode op = CodeArea::getOpcode(PC);
    
    if (!firstCall && argsToHandle==0)
      goto exit;
    
    firstCall = NO;
    switch(op) {

    case GETRECORDY: SETTMPA(YPC(3)); goto getRecord;
    case GETRECORDG: SETTMPA(GPC(3)); goto getRecord;
    case GETRECORDX: SETTMPA(XPC(3)); /* fall through */
      {
      getRecord:
	TaggedRef label = getLiteralArg(PC+1);
	SRecordArity ff = (SRecordArity) getAdressArg(PC+2);
	TaggedRef term  = GETTMPA();
	DEREF(term,termPtr,tag);
	
	Assert(oz_isUVar(term));
	int numArgs = getWidth(ff);
	SRecord *srecord = SRecord::newSRecord(label,ff, numArgs);
	bindOPT(termPtr,makeTaggedSRecord(srecord));
	sPointer = srecord->getRef();
	DISPATCH(4,numArgs);
      }

    case GETLITERALY: /* fall through */
    case GETNUMBERY:  SETTMPA(YPC(2)); goto getNumber;
    case GETLITERALG: /* fall through */
    case GETNUMBERG:  SETTMPA(GPC(2)); goto getNumber;
    case GETLITERALX: /* fall through */
    case GETNUMBERX:  SETTMPA(XPC(2)); /* fall through */
    getNumber:
    {
      TaggedRef i = getNumberArg(PC+1);
      TaggedRef term = GETTMPA();
      DEREF(term,termPtr,tag);
      Assert(oz_isUVar(term));
      bindOPT(termPtr, i);
      DISPATCH(3,-1);
    }


    case GETLISTVALVARX:
      {
	TaggedRef term = XPC(1);
	DEREF(term,termPtr,tag);
	
	Assert(oz_isUVar(term));
	LTuple *ltuple = new LTuple();
	ltuple->setHead(XPC(2));
	ltuple->setTail(am.currentUVarPrototype());
	bindOPT(termPtr,makeTaggedLTuple(ltuple));
	XPC(3) = makeTaggedRef(ltuple->getRef()+1);
	DISPATCH(4,0);
      }

    case GETLISTY: SETTMPA(YPC(1)); goto getList;
    case GETLISTG: SETTMPA(GPC(1)); goto getList;
    case GETLISTX: SETTMPA(XPC(1));
    getList:
      {
	TaggedRef aux = GETTMPA();
	DEREF(aux,auxPtr,tag);
	
	Assert(oz_isUVar(aux));
	LTuple *ltuple = new LTuple();
	sPointer = ltuple->getRef();
	bindOPT(auxPtr,makeTaggedLTuple(ltuple));
	DISPATCH(2,2);
      }

    case UNIFYVARIABLEY: SETTMPA(YPC(1)); goto unifyVariable;
    case UNIFYVARIABLEX: SETTMPA(XPC(1));
    unifyVariable:
    {
	*sPointer = am.currentUVarPrototype();
	GETTMPA() = makeTaggedRef(sPointer++);
	DISPATCH(2,-1);
      }

    case UNIFYVALUEY: SETTMPA(YPC(1)); goto unifyValue;
    case UNIFYVALUEG: SETTMPA(GPC(1)); goto unifyValue;
    case UNIFYVALUEX: SETTMPA(XPC(1)); goto unifyValue;
    unifyValue:
    {
	*sPointer++ = GETTMPA();
	DISPATCH(2,-1);
      }

    case UNIFYVALVARXY: SETTMPA(XPC(1)); SETTMPB(YPC(2)); goto unifyValVar;
    case UNIFYVALVARYX: SETTMPA(YPC(1)); SETTMPB(XPC(2)); goto unifyValVar;
    case UNIFYVALVARYY: SETTMPA(YPC(1)); SETTMPB(YPC(2)); goto unifyValVar;
    case UNIFYVALVARGX: SETTMPA(GPC(1)); SETTMPB(XPC(2)); goto unifyValVar;
    case UNIFYVALVARGY: SETTMPA(GPC(1)); SETTMPB(YPC(2)); goto unifyValVar;
    case UNIFYVALVARXX: SETTMPA(XPC(1)); SETTMPB(XPC(2)); goto unifyValVar;
      {
      unifyValVar:
	*sPointer++ = GETTMPA();
	*sPointer++ = am.currentUVarPrototype();
	GETTMPB() = makeTaggedRef(sPointer);
	DISPATCH(3,-2);
      }

    case UNIFYVOID:
      {
	int n = getPosIntArg(PC+1);
	for (int i = n-1; i >=0; i-- ) {
	  *sPointer++ = am.currentUVarPrototype();
	}
	DISPATCH(2,-n);
      }

    case UNIFYNUMBER:
    case UNIFYLITERAL:
      {
	*sPointer++ = getTaggedArg(PC+1);
	DISPATCH(2,-1);
      }

    case PUTCONSTANTX:
      {
	XPC(2) = getTaggedArg(PC+1);
	DISPATCH(3,0);
      }

    default:
      Assert(0);
      goto exit;
    }
  }

 exit:
  if (maxX > 0) {
    memcpy(XREGS, XREGS_SAVE, maxX);
  }
  if (maxY > 0) {
    memcpy(Y, savedY, maxY); freeListDispose(savedY, maxY);
  }
}


// outlined:
void pushContX(TaskStack *stk, 
	       ProgramCounter pc,RefsArray y,Abstraction *cap) {
  stk->pushCont(pc,y,cap); 
  stk->pushX(cap->getPred()->getMaxX());
}


#ifdef OUTLINE
#undef inline
#endif
