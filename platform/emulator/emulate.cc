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
#include "space.hh"
#include "thr_int.hh"
#include "debug.hh"
#include "prop_int.hh"
#include "space.hh"
#include "codearea.hh"
#include "builtins.hh"
#include "indexing.hh"

#include "var_all.hh"
#include "dictionary.hh"
#include "marshaler.hh"
#include "copycode.hh"
#include "trace.hh"
#include "os.hh"

#ifdef OUTLINE
#define inline
#endif

// -----------------------------------------------------------------------
// Object stuff
// -----------------------------------------------------------------------

inline
Abstraction *getSendMethod(Object *obj, TaggedRef label, SRecordArity arity,
                           InlineCache *cache, RefsArray X)
{
  Assert(oz_isFeature(label));
  return cache->lookup(obj->getClass(),label,arity,X);
}

// -----------------------------------------------------------------------
// *** EXCEPTION stuff
// -----------------------------------------------------------------------

#define RAISE_APPLY(fun,args)                           \
  (void) oz_raise(E_ERROR,E_KERNEL,"apply",2,fun,args); \
  RAISE_THREAD;


static
void enrichTypeException(TaggedRef value,const char *fun, OZ_Term args)
{
  OZ_Term e = OZ_subtree(value,OZ_int(1));
  OZ_putArg(e,1,OZ_atom((OZ_CONST char*)fun));
  OZ_putArg(e,2,args);
}

#define RAISE_TYPE1(fun,args)                           \
  enrichTypeException(e->exception.value,fun,args);     \
  RAISE_THREAD;

#define RAISE_TYPE1_FUN(fun,args) \
  RAISE_TYPE1(fun, appendI(args,oz_cons(oz_newVariable(),oz_nil())));

#define RAISE_TYPE_NEW(bi,loc) \
  RAISE_TYPE1(bi->getPrintName(), biArgs(loc,X));

#define RAISE_TYPE(bi) \
  RAISE_TYPE1(bi->getPrintName(), OZ_toList(bi->getArity(),X));

/*
 * Handle Failure macros (HF)
 */

Bool AM::hf_raise_failure()
{
  if (!oz_onToplevel() &&
      (!oz_currentThread()->hasCatchFlag() ||
       !oz_isCurrentBoard(GETBOARD(oz_currentThread())))) {
    return OK;
  }
  exception.info  = NameUnit;
  exception.value = RecordFailure;
  exception.debug = ozconf.errorDebug;
  return NO;
}

// This macro is optimized such that the term T is only created
// when needed, so don't pass it as argument to functions.
#define HF_RAISE_FAILURE(T)                             \
   if (e->hf_raise_failure())                           \
     return T_FAILURE;                                  \
   if (ozconf.errorDebug) e->setExceptionInfo(T);       \
   RAISE_THREAD;


#define HF_EQ(X,Y)    HF_RAISE_FAILURE(OZ_mkTupleC("eq",2,X,Y))
#define HF_TELL(X,Y)  HF_RAISE_FAILURE(OZ_mkTupleC("tell",2,X,Y))
#define HF_APPLY(N,A) HF_RAISE_FAILURE(OZ_mkTupleC("apply",2,N,A))
#define HF_BI(bi)     HF_APPLY(bi->getName(),OZ_toList(bi->getArity(),X));
#define HF_BI_NEW(bi,loc)   HF_APPLY(bi->getName(),biArgs(loc,X));

#define CheckArity(arityExp,proc)                                          \
if (predArity != arityExp) {                                               \
  (void) oz_raise(E_ERROR,E_KERNEL,"arity",2,proc,OZ_toList(predArity,X)); \
  RAISE_THREAD;                                                    \
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

// optimized RefsArray allocation
inline
RefsArray allocateY(int n)
{
  COUNT(numEnvAllocs);

  int sz = (n+1) * sizeof(TaggedRef);
  COUNT1(sizeEnvs,sz);
  CountMax(maxEnvSize,sz);
  RefsArray a = (RefsArray) freeListMalloc(sz);
  a += 1;
  initRefsArray(a,n,OK);
  return a;
}

inline
void deallocateY(RefsArray a, int sz)
{
  Assert(getRefsArraySize(a)==sz);
  Assert(!isFreedRefsArray(a));
#ifdef DEBUG_CHECK
  markFreedRefsArray(a);
#else
  freeListDispose(a-1,(sz+1) * sizeof(TaggedRef));
#endif
}

inline
void deallocateY(RefsArray a)
{
  deallocateY(a,getRefsArraySize(a));
}

// fix disabled
// #define DIST_UNIFY_FIX 0

// X=f(Y,b) Y=g(a): stop after X=f(Y,b) (create new var for Y)
//#define DIST_UNIFY_FIX 1

// X=f(Y,b) Y=g(a): ask manager of X for binding to f(g(a),b)
#define DIST_UNIFY_FIX 2

void buildRecord(ProgramCounter PC, RefsArray X, RefsArray Y, Abstraction *CAP);

// -----------------------------------------------------------------------
// *** ???
// -----------------------------------------------------------------------

#define DoSwitchOnTerm(indexTerm,table)                                 \
      TaggedRef term = indexTerm;                                       \
      DEREF(term,termPtr,_2);                                           \
                                                                        \
      if (oz_isLTuple(term)) {                                          \
        int offset = table->listLabel;                                  \
        if (!offset) offset = table->elseLabel;                         \
        sPointer = tagged2LTuple(term)->getRef();                       \
        JUMPRELATIVE(offset);                                           \
      } else {                                                          \
        TaggedRef *sp = sPointer;                                       \
        int offset = switchOnTermOutline(term,termPtr,table,sp);        \
        sPointer = sp;                                                  \
        if (offset) {                                                   \
          JUMPRELATIVE(offset);                                         \
        } else {                                                        \
          SUSP_PC(termPtr,PC);                                          \
        }                                                               \
      }


// most probable case first: local UVar
// if (isUVar(var) && isCurrentBoard(tagged2VarHome(var))) {
// more efficient:
inline
void bindOPT(OZ_Term *varPtr, OZ_Term term)
{
  Assert(isUVar(*varPtr));
  if (!am.currentUVarPrototypeEq(*varPtr)) {
    if (!oz_isLocalUVar(varPtr)) {
      am.trail.pushRef(varPtr,*varPtr);
    }
  }
  COUNT(varOptUnify);
  doBind(varPtr,term);
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
  COUNT(varOptUnify);
  return PROCEED;
}

/*
 * new builtins support
 */

static OZ_Term *savedX = NULL;

OZ_Return oz_bi_wrapper(Builtin *bi,OZ_Term *X)
{
  Assert(am.isEmptySuspendVarList());
  Assert(am.isEmptyPreparedCalls());

  const int inAr = bi->getInArity();
  const int outAr = bi->getOutArity();

  if (savedX)
    delete [] savedX;
  savedX = new OZ_Term[outAr];

  int i;
  for (i=outAr; i--; ) savedX[i]=X[inAr+i];

  OZ_Return ret1 = bi->getFun()(X,OZ_ID_MAP);
  if (ret1!=PROCEED) {
    switch (ret1) {
    case FAILED:
    case RAISE:
    case BI_TYPE_ERROR:
    case SUSPEND:
      {
        // restore X
        for (int j=outAr; j--; ) {
          X[inAr+j]=savedX[j];
        }
        return ret1;
      }
    case PROCEED:
    case BI_PREEMPT:
    case BI_REPLACEBICALL:
      break;
    default:
      OZ_error("oz_bi_wrapper: return not handled: %d",ret1);
      return FAILED;
    }
  }
  for (i=outAr;i--;) {
    OZ_Return ret2 = fastUnify(X[inAr+i],savedX[i]);
    if (ret2!=PROCEED) {
      switch (ret2) {
      case FAILED:
      case RAISE:
      case BI_TYPE_ERROR:
        {
          // restore X in case of error
          for (int j=outAr; j--; ) {
            X[inAr+j]=savedX[j];
          }
          return ret2;
        }
      case SUSPEND:
        DebugCheckT(printf("oz_bi_wrapper: unify suspend\n"));
        am.emptySuspendVarList();
        am.prepareCall(BI_Unify,X[inAr+i],savedX[i]);
        ret1=BI_REPLACEBICALL;
        break;
      case BI_REPLACEBICALL:
        DebugCheckT(printf("oz_bi_wrapper: unify replcall\n"));
        ret1=BI_REPLACEBICALL;
        break;
      default:
        Assert(0);
      }
    }
  }
  return ret1;
}

static
void set_exception_info_call(Builtin *bi,OZ_Term *X, int *map=OZ_ID_MAP)
{
  if (bi==bi_raise||bi==bi_raiseError) return;

  int iarity = bi->getInArity();
  int oarity = bi->getOutArity();

  OZ_Term args=oz_nil();
  for (int j = iarity; j--;) {
    args=oz_cons(X[map == OZ_ID_MAP? j : map[j]],args);
  }
  am.setExceptionInfo(OZ_mkTupleC("fapply",3,
                                  makeTaggedConst(bi),
                                  args,
                                  OZ_int(oarity)));
}

static
OZ_Term biArgs(OZ_Location *loc, OZ_Term *X) {
  OZ_Term out=oz_nil();
  int i;
  for (i=loc->getOutArity(); i--; ) {
    out=oz_cons(oz_newVariable(),out);
  }
  for (i=loc->getInArity(); i--; ) {
    out=oz_cons(X[loc->in(i)],out);
  }
  return out;
}

// -----------------------------------------------------------------------
// *** genCallInfo: self modifying code!
// -----------------------------------------------------------------------

static
Bool genCallInfo(GenCallInfoClass *gci, TaggedRef pred, ProgramCounter PC)
{
  Assert(!oz_isRef(pred));

  Abstraction *abstr = NULL;
  if (gci->isMethAppl) {
    Bool defaultsUsed;
    abstr = tagged2ObjectClass(pred)->getMethod(gci->mn,gci->arity,
                                                0,defaultsUsed);
    /* fill cache and try again later */
    if (abstr==NULL || defaultsUsed) return NO;
  } else {
    if(!oz_isAbstraction(pred)) goto bombGenCall;

    abstr = tagged2Abstraction(pred);
    if (abstr->getArity() != getWidth(gci->arity))
      goto bombGenCall;
  }

  {
    /* ok abstr points to an abstraction */
    AbstractionEntry *entry = new AbstractionEntry(NO);
    entry->setPred(abstr);
    CodeArea *code = CodeArea::findBlock(PC);
    code->writeAbstractionEntry(entry, PC+1);
    CodeArea::writeOpcode(gci->isTailCall ? FASTTAILCALL : FASTCALL, PC);
    return OK;
  }

bombGenCall:
  CodeArea::writeRegIndex(gci->regIndex,PC+1);
  CodeArea::writeArity(getWidth(gci->arity), PC+2);
  CodeArea::writeOpcode(gci->isTailCall ? TAILCALLG : CALLG,PC);
  return OK;
}




// -----------------------------------------------------------------------
// *** CALL HOOK
// -----------------------------------------------------------------------


/* the hook functions return:
     TRUE: must reschedule
     FALSE: can continue
   */

inline
Bool isNotPreemptiveScheduling(void)
{
  //  return TRUE;
  if (am.isSetSFlag()) {
    if (am.isSetSFlag(ThreadSwitch)) {
      if (am.threadsPool.threadQueuesAreEmpty())
        am.restartThread();
      else
        return FALSE;
    }
    return !am.isSetSFlag();
  } else {
    return TRUE;
  }
}

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

#define RAISE_THREAD_NO_PC                      \
  e->exception.pc=NOCODE;                       \
  return T_RAISE;

#define RAISE_THREAD                            \
  e->exception.pc=PC;                           \
  e->exception.y=Y;                             \
  e->exception.cap=CAP;                         \
  return T_RAISE;


/* macros are faster ! */
#define emulateHookCall(e,Code)                 \
   if (hookCheckNeeded()) {                     \
       Code;                                    \
       return T_PREEMPT;                        \
   }

#define emulateHookPopTask(e) emulateHookCall(e,;)


#define ChangeSelf(obj)                         \
      e->changeSelf(obj);

#define PushCont(_PC)  CTS->pushCont(_PC,Y,CAP);
#define PushContX(_PC) pushContX(CTS,_PC,Y,CAP,X);

void pushContX(TaskStack *stk,
               ProgramCounter pc,RefsArray y,Abstraction *cap,
               RefsArray x);

/* NOTE:
 * in case we have call(x-N) and we have to switch process or do GC
 * we have to save as cont address Pred->getPC() and NOT PC
 */
#define CallDoChecks(Pred)                                                  \
     Y = NULL;                                                              \
     CAP = Pred;                                                            \
     emulateHookCall(e,PushContX(Pred->getPC()));

/* Must save output register too !!! (RS) */
#define MaxToSave(OutReg,LivingRegs) \
        max(getRegArg(PC+OutReg)+1,getPosIntArg(PC+LivingRegs));

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

#ifdef THREADED
#define Case(INSTR) INSTR##LBL : asmLbl(INSTR);

#ifdef DELAY_SLOT
// let gcc fill in the delay slot of the "jmp" instruction:
#define DISPATCH(INC) {                         \
  intlong aux = *(PC+INC);                      \
  INCFPC(INC);                                  \
  goto* (void*) (aux|textBase);                 \
}
#elif defined(_MSC_VER)
#define DISPATCH(INC) {                         \
   PC += INC;                                   \
   int aux = *PC;                               \
   __asm jmp aux                                \
}

#else
#define DISPATCH(INC) {                         \
  INCFPC(INC);                                  \
  goto* (void*) ((*PC)|textBase);               \
}
#endif

#else /* THREADED */

#define Case(INSTR)   case INSTR :  INSTR##LBL : asmLbl(INSTR);
#define DISPATCH(INC) INCFPC(INC); goto LBLdispatcher

#endif

#define JUMPRELATIVE(offset) Assert(offset!=0); DISPATCH(offset)
#define JUMPABSOLUTE(absaddr) PC=absaddr; DISPATCH(0)

#define ONREG(Label,R)      HelpReg = (R); goto Label
#define ONREG2(Label,R1,R2) HelpReg1 = (R1); HelpReg2 = (R2); goto Label


#ifdef FASTREGACCESS
#define RegAccess(Reg,Index) (*(RefsArray)((intlong) Reg + Index))
#else
#define RegAccess(Reg,Index) (Reg[Index])
#endif

#define GREF (CAP->getGRef())
#define Xreg(N) RegAccess(X,N)
#define Yreg(N) RegAccess(Y,N)
#define Greg(N) RegAccess(GREF,N)

#define XPC(N) Xreg(getRegArg(PC+N))

/* define REGOPT if you want the into register optimization for GCC */
#if defined(REGOPT) &&__GNUC__ >= 2 && (defined(ARCH_I486) || defined(ARCH_MIPS) || defined(OSF1_ALPHA) || defined(ARCH_SPARC)) && !defined(DEBUG_CHECK)
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
#define CheckLiveness(PC) checkLiveness(PC,X,CAP->getPred()->getMaxX())
#else
#define CheckLiveness(PC)
#endif

// ------------------------------------------------------------------------
// ???
// ------------------------------------------------------------------------

#define MAGIC_RET goto LBLMagicRet;

#define SUSP_PC(TermPtr,PC) {                   \
   CheckLiveness(PC);                           \
   PushContX(PC);                               \
   tmpRet = oz_var_addSusp(TermPtr,CTT);        \
   MAGIC_RET;                                   \
}

/*
 * create the suspension for builtins returning SUSPEND
 *
 * PRE: no reference chains !!
 */


#define SUSPENDONVARLIST                        \
{                                               \
  tmpRet = e->suspendOnVarList(CTT);            \
  MAGIC_RET;                                    \
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
TaggedRef makeMessage(SRecordArity srecArity, TaggedRef label, TaggedRef *X)
{
  int width = getWidth(srecArity);
  if (width == 0) {
    return label;
  }

  if (width == 2 && oz_eq(label,AtomCons))
    return makeTaggedLTuple(new LTuple(X[0],X[1]));

  SRecord *tt;
  if(sraIsTuple(srecArity)) {
    tt = SRecord::newSRecord(label,width);
  } else {
    tt = SRecord::newSRecord(label,getRecordArity(srecArity));
  }
  for (int i = width-1;i >= 0; i--) {
    tt->setArg(i,X[i]);
  }
  TaggedRef ret = makeTaggedSRecord(tt);
  //  message("makeMessage: %s\n",toC(ret));
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
# define CPP (CTT->getPriority())
# define CTS (e->cachedStack)

int engine(Bool init)
{
// ------------------------------------------------------------------------
// *** Global Variables
// ------------------------------------------------------------------------
  /* ordered by importance: first variables will go into machine registers
   * if -DREGOPT is set
   */
  register ProgramCounter PC   Reg1 = 0;
#ifdef MANY_REGISTERS
  register TaggedRef *X        Reg2 = am.xRegs;
#else
  register TaggedRef * const X Reg2 = am.xRegs;
#endif
  register RefsArray Y         Reg3 = NULL;
  register TaggedRef *sPointer Reg4 = NULL;
  register AM * const e        Reg5 = &am;
  register Abstraction * CAP   Reg6 = NULL;

  Bool isTailCall              = NO;                NoReg(isTailCall);
  DebugCheckT(Board *currentDebugBoard=CBB);

  // handling perdio unification
  ProgramCounter lastGetRecord;                     NoReg(lastGetRecord);

  RefsArray HelpReg1 = NULL, HelpReg2 = NULL;
  #define HelpReg sPointer  /* more efficient */

  ConstTerm *predicate;     NoReg(predicate);
  int predArity;            NoReg(predArity);

  // optimized arithmetic and special cases for unification
  OZ_Return tmpRet;  NoReg(tmpRet);

  TaggedRef auxTaggedA, auxTaggedB;
  int auxInt;
  char *auxString;

#ifdef THREADED
  if (init) {
#include "instrtab.hh"
    CodeArea::init(instrTable);
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
  Assert(CBB==currentDebugBoard);

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
  // displayCode(PC,1);

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
    {
      Xreg(getRegArg(PC+2)) = Xreg(getRegArg(PC+1));
      DISPATCH(3);
    }
  Case(MOVEXY)
    {
      Yreg(getRegArg(PC+2)) = Xreg(getRegArg(PC+1));
      DISPATCH(3);
    }

  Case(MOVEYX)
    {
      Xreg(getRegArg(PC+2)) = Yreg(getRegArg(PC+1));
      DISPATCH(3);
    }
  Case(MOVEYY)
    {
      Yreg(getRegArg(PC+2)) = Yreg(getRegArg(PC+1));
      DISPATCH(3);
    }

  Case(MOVEGX)
    {
      Xreg(getRegArg(PC+2)) = Greg(getRegArg(PC+1));
      DISPATCH(3);
    }
  Case(MOVEGY)
    {
      Yreg(getRegArg(PC+2)) = Greg(getRegArg(PC+1));
      DISPATCH(3);
    }


  // move X[i] --> Y[j], X[k] --> Y[l]
  Case(MOVEMOVEXYXY)
    {
      Yreg(getRegArg(PC+2)) = Xreg(getRegArg(PC+1));
      Yreg(getRegArg(PC+4)) = Xreg(getRegArg(PC+3));
      DISPATCH(5);
    }

  // move Y[i] --> X[j], Y[k] --> X[l]
  Case(MOVEMOVEYXYX)
    {
      Xreg(getRegArg(PC+2)) = Yreg(getRegArg(PC+1));
      Xreg(getRegArg(PC+4)) = Yreg(getRegArg(PC+3));
      DISPATCH(5);
    }

  // move Y[i] --> X[j], X[k] --> Y[l]
  Case(MOVEMOVEYXXY)
    {
      Xreg(getRegArg(PC+2)) = Yreg(getRegArg(PC+1));
      Yreg(getRegArg(PC+4)) = Xreg(getRegArg(PC+3));
      DISPATCH(5);
    }

  // move X[i] --> Y[j], Y[k] --> X[l]
  Case(MOVEMOVEXYYX)
    {
      Yreg(getRegArg(PC+2)) = Xreg(getRegArg(PC+1));
      Xreg(getRegArg(PC+4)) = Yreg(getRegArg(PC+3));
      DISPATCH(5);
    }

  Case(CLEARY)
    {
      Yreg(getRegArg(PC+1)) = makeTaggedNULL();
      DISPATCH(2);
    }


  Case(GETSELF)
    {
      XPC(1) = makeTaggedConst(e->getSelf());
      DISPATCH(2);
    }

  Case(SETSELF)
    {
      TaggedRef term = XPC(1);
      if (oz_isRef(term)) {
        DEREF(term,termPtr,tag);
        if (oz_isVariable(term)) {
          SUSP_PC(termPtr,PC);
        }
      }
      ChangeSelf(tagged2Object(term));
      DISPATCH(2);
    }


  Case(GETRETURNX) { Xreg(getRegArg(PC+1)) = GetFunReturn(); DISPATCH(2); }
  Case(GETRETURNY) { Yreg(getRegArg(PC+1)) = GetFunReturn(); DISPATCH(2); }
  Case(GETRETURNG) { Greg(getRegArg(PC+1)) = GetFunReturn(); DISPATCH(2); }

  Case(FUNRETURNX) {
    SetFunReturn(Xreg(getRegArg(PC+1)));
    if (Y) { deallocateY(Y); Y = NULL; }
    goto LBLpopTaskNoPreempt;
  }
  Case(FUNRETURNY) {
    SetFunReturn(Yreg(getRegArg(PC+1)));
    if (Y) { deallocateY(Y); Y = NULL; }
    goto LBLpopTaskNoPreempt;
  }
  Case(FUNRETURNG) {
    SetFunReturn(Greg(getRegArg(PC+1)));
    if (Y) { deallocateY(Y); Y = NULL; }
    goto LBLpopTaskNoPreempt;
  }


  Case(CREATEVARIABLEX)
    {
      Xreg(getRegArg(PC+1)) = oz_newVariableOPT();
      DISPATCH(2);
    }
  Case(CREATEVARIABLEY)
    {
      ProfileCode(if (CAP) CAP->getPred()->szVars += sizeof(TaggedRef));
      ProfileCode(COUNT1(sizeStackVars,sizeof(TaggedRef)));
      Yreg(getRegArg(PC+1)) = oz_newVariableOPT();
      DISPATCH(2);
    }

  Case(CREATEVARIABLEMOVEX)
    {
      Xreg(getRegArg(PC+1)) = Xreg(getRegArg(PC+2)) = oz_newVariableOPT();
      DISPATCH(3);
    }
  Case(CREATEVARIABLEMOVEY)
    {
      ProfileCode(if (CAP) CAP->getPred()->szVars += sizeof(TaggedRef));
      ProfileCode(COUNT1(sizeStackVars,sizeof(TaggedRef)));
      Yreg(getRegArg(PC+1)) = Xreg(getRegArg(PC+2)) = oz_newVariableOPT();
      DISPATCH(3);
    }


  Case(UNIFYXX) ONREG(Unify,X);
  Case(UNIFYXY) ONREG(Unify,Y);
  Case(UNIFYXG) ONREG(Unify,GREF);
  {
  Unify:
    const TaggedRef A = Xreg(getRegArg(PC+1));
    const TaggedRef B = RegAccess(HelpReg,getRegArg(PC+2));
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

    Xreg(getRegArg(PC+3)) = makeTaggedSRecord(srecord);
    sPointer = srecord->getRef();

    DISPATCH(4);
  }

  Case(PUTRECORDY)
  {
    TaggedRef label = getLiteralArg(PC+1);
    SRecordArity ff = (SRecordArity) getAdressArg(PC+2);
    SRecord *srecord = SRecord::newSRecord(label,ff,getWidth(ff));

    Yreg(getRegArg(PC+3)) = makeTaggedSRecord(srecord);
    sPointer = srecord->getRef();

    DISPATCH(4);
  }

  Case(PUTCONSTANTX)
    Xreg(getRegArg(PC+2)) = getTaggedArg(PC+1); DISPATCH(3);

  Case(PUTCONSTANTY)
    Yreg(getRegArg(PC+2)) = getTaggedArg(PC+1); DISPATCH(3);

  Case(PUTLISTX)
    {
      LTuple *term = new LTuple();
      Xreg(getRegArg(PC+1)) = makeTaggedLTuple(term);
      sPointer = term->getRef();
      DISPATCH(2);
    }

  Case(PUTLISTY)
    {
      LTuple *term = new LTuple();
      Yreg(getRegArg(PC+1)) = makeTaggedLTuple(term);
      sPointer = term->getRef();
      DISPATCH(2);
    }


  Case(SETVARIABLEX)
    {
      Reg reg = getRegArg(PC+1);
      *sPointer = e->currentUVarPrototype();
      Xreg(reg) = makeTaggedRef(sPointer++);
      DISPATCH(2);
    }

  Case(SETVARIABLEY)
    {
      Reg reg = getRegArg(PC+1);
      *sPointer = e->currentUVarPrototype();
      Yreg(reg) = makeTaggedRef(sPointer++);
      DISPATCH(2);
    }

  Case(SETVALUEX)
    {
      *sPointer++ = Xreg(getRegArg(PC+1));
      DISPATCH(2);
    }

  Case(SETVALUEY)
    {
      *sPointer++ = Yreg(getRegArg(PC+1));
      DISPATCH(2);
    }

  Case(SETVALUEG)
    {
      *sPointer++ = Greg(getRegArg(PC+1));
      DISPATCH(2);
    }


  Case(SETCONSTANT)
    {
      *sPointer++ = getTaggedArg(PC+1);
      DISPATCH(2);
    }

  Case(SETPREDICATEREF)
    {
      *sPointer++ = OZ_makeForeignPointer(getAdressArg(PC+1));
      DISPATCH(2);
    }

  Case(SETVOID)
    {
      int n = getPosIntArg(PC+1);
      for (int i = 0; i < n; i++ ) {
        *sPointer++ = e->currentUVarPrototype();
      }
      DISPATCH(2);
    }


  Case(GETRECORDX) ONREG(GetRecord,X);
  Case(GETRECORDY) ONREG(GetRecord,Y);
  Case(GETRECORDG) ONREG(GetRecord,GREF);
  {
  GetRecord:
    TaggedRef label = getLiteralArg(PC+1);
    SRecordArity ff = (SRecordArity) getAdressArg(PC+2);

    TaggedRef term = RegAccess(HelpReg,getRegArg(PC+3));
    DEREF(term,termPtr,tag);

    if (isUVar(term)) {
      SRecord *srecord = SRecord::newSRecord(label,ff,getWidth(ff));
      bindOPT(termPtr,makeTaggedSRecord(srecord));
      sPointer = srecord->getRef();
      SetWriteMode;
      DISPATCH(4);
    } else if (oz_isVariable(term)) {
      Assert(isCVar(term));
      TaggedRef record;
      if (DIST_UNIFY_FIX && oz_onToplevel()) {
        RegAccess(HelpReg,getRegArg(PC+3)) = record = OZ_newVariable();
        buildRecord(PC,X,Y,CAP);
        record = oz_deref(record);
        //message("buildRecord: %s\n",toC(record));
        RegAccess(HelpReg,getRegArg(PC+3)) = makeTaggedRef(termPtr);
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

    HF_TELL(RegAccess(HelpReg,getRegArg(PC+3)),mkRecord(label,ff));
  }

#define ONREG3(lbl,Reg)                         \
      auxTaggedA = Reg(getRegArg(PC+1));        \
      goto lbl;

  Case(TESTLITERALG) ONREG3(testLiteral,Greg);
  Case(TESTLITERALY) ONREG3(testLiteral,Yreg);
  Case(TESTLITERALX) ONREG3(testLiteral,Xreg);
  {
  testLiteral:
    TaggedRef term = auxTaggedA;
    TaggedRef atm  = getLiteralArg(PC+2);

    DEREF(term,termPtr,tag);
    if (oz_isVariable(term)) {
      if (isCVar(term) &&
          !oz_var_valid(tagged2CVar(term),termPtr,atm)) {
        // fail
        JUMPRELATIVE( getLabelArg(PC+3) );
      }
      SUSP_PC(termPtr,PC);
    }
    if (oz_eq(term,atm)) {
      DISPATCH(4);
    }
    // fail
    JUMPRELATIVE( getLabelArg(PC+3) );
  }

  Case(TESTBOOLG) ONREG3(testBool,Greg);
  Case(TESTBOOLY) ONREG3(testBool,Yreg);
  Case(TESTBOOLX) ONREG3(testBool,Xreg);
  {
  testBool:
    TaggedRef term = auxTaggedA;
    DEREF(term,termPtr,tag);

    if (oz_eq(term,oz_true())) {
      DISPATCH(4);
    }

    if (oz_eq(term,oz_false())) {
      JUMPRELATIVE(getLabelArg(PC+2));
    }

    // mm2: kinded and ofs handling missing
    if (oz_isVariable(term)) {
      SUSP_PC(termPtr, PC);
    }

    JUMPRELATIVE( getLabelArg(PC+3) );
  }

  Case(TESTNUMBERG) ONREG3(testNumber,Greg);
  Case(TESTNUMBERY) ONREG3(testNumber,Yreg);
  Case(TESTNUMBERX) ONREG3(testNumber,Xreg);
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
      JUMPRELATIVE(getLabelArg(PC+3));
    }

    if (oz_numberEq(i,term)) {
      DISPATCH(4);
    }

    if (oz_isVariable(term)) {
      if (oz_isKinded(term) &&
          !oz_var_valid(tagged2CVar(term),termPtr,i)) {
        // fail
        JUMPRELATIVE( getLabelArg(PC+3) );
      }
      SUSP_PC(termPtr,PC);
    }
    // fail
    JUMPRELATIVE( getLabelArg(PC+3) );
  }


  Case(TESTRECORDG) ONREG3(testRecord,Greg);
  Case(TESTRECORDY) ONREG3(testRecord,Yreg);
  Case(TESTRECORDX) ONREG3(testRecord,Xreg);
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
      } else if (isCVar(term)) {
        OzVariable *cvar = tagged2CVar(term);
        if (cvar->getType() == OZ_VAR_OF) {
          OzOFVariable *ofsvar = (OzOFVariable *) cvar;
          Literal *lit = tagged2Literal(label);
          if (sraIsTuple(sra) && ofsvar->disentailed(lit,getTupleWidth(sra)) ||
              ofsvar->disentailed(lit,getRecordArity(sra))) {
            JUMPRELATIVE(getLabelArg(PC+4));
          }
          SUSP_PC(termPtr,PC);
        }
      }
      // fall through
    }
    // fail
    JUMPRELATIVE(getLabelArg(PC+4));
  }


  Case(TESTLISTG) ONREG3(testList,Greg);
  Case(TESTLISTY) ONREG3(testList,Yreg);
  Case(TESTLISTX) ONREG3(testList,Xreg);
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
      } else if (isCVar(term)) {
        OzVariable *cvar = tagged2CVar(term);
        if (cvar->getType() == OZ_VAR_OF) {
          OzOFVariable *ofsvar = (OzOFVariable *) cvar;
          if (ofsvar->disentailed(tagged2Literal(AtomCons),2)) {
            JUMPRELATIVE(getLabelArg(PC+2));
          }
          SUSP_PC(termPtr,PC);
        }
      }
      // fall through
    }
    // fail
    JUMPRELATIVE(getLabelArg(PC+2));
  }


  Case(GETLITERALX)
    {
      TaggedRef atm = getLiteralArg(PC+1);

      TaggedRef term = Xreg(getRegArg(PC+2));
      DEREF(term,termPtr,tag);

      if (isUVar(tag)) {
        bindOPT(termPtr, atm);
        DISPATCH(3);
      }

      if (oz_eq(term,atm)) {
        DISPATCH(3);
      }

      auxTaggedA = Xreg(getRegArg(PC+2));
      goto getLiteralComplicated;
    }
  Case(GETLITERALY)
    {
      TaggedRef atm = getLiteralArg(PC+1);

      TaggedRef term = Yreg(getRegArg(PC+2));
      DEREF(term,termPtr,tag);

      if (isUVar(tag)) {
        bindOPT(termPtr, atm);
        DISPATCH(3);
      }

      if (oz_eq(term,atm)) {
        DISPATCH(3);
      }

      auxTaggedA = Yreg(getRegArg(PC+2));
      goto getLiteralComplicated;
    }
  Case(GETLITERALG)
    {
      TaggedRef atm = getLiteralArg(PC+1);

      TaggedRef term = Greg(getRegArg(PC+2));
      DEREF(term,termPtr,tag);

      if (isUVar(tag)) {
        bindOPT(termPtr, atm);
        DISPATCH(3);
      }

      if (oz_eq(term,atm)) {
        DISPATCH(3);
      }

      auxTaggedA = Greg(getRegArg(PC+2));
      goto getLiteralComplicated;
    }


    {
    getLiteralComplicated:
      TaggedRef term = auxTaggedA;
      TaggedRef atm = getLiteralArg(PC+1);
      DEREF(term,termPtr,tag);
      if (isCVar(term)) {
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
      TaggedRef term = Xreg(getRegArg(PC+2));
      DEREF(term,termPtr,tag);

      if (isUVar(tag)) {
        bindOPT(termPtr, i);
        DISPATCH(3);
      }

      if (oz_numberEq(term,i)) {
        DISPATCH(3);
      }

      if (isCVar(term)) {
        tmpRet=oz_var_bind(tagged2CVar(term),termPtr,i);
        if (tmpRet==PROCEED) { DISPATCH(3); }
        if (tmpRet!=FAILED)  { goto LBLunifySpecial; }
        // fall through to fail
      }

      HF_TELL(Xreg(getRegArg(PC+2)), getNumberArg(PC+1));
    }
  Case(GETNUMBERY)
    {
      TaggedRef i = getNumberArg(PC+1);
      TaggedRef term = Yreg(getRegArg(PC+2));
      DEREF(term,termPtr,tag);

      if (isUVar(tag)) {
        bindOPT(termPtr, i);
        DISPATCH(3);
      }

      if (oz_numberEq(term,i)) {
        DISPATCH(3);
      }

      if (isCVar(term)) {
        tmpRet=oz_var_bind(tagged2CVar(term),termPtr,i);
        if (tmpRet==PROCEED) { DISPATCH(3); }
        if (tmpRet!=FAILED)  { goto LBLunifySpecial; }
        // fall through to fail
      }

      HF_TELL(Yreg(getRegArg(PC+2)), getNumberArg(PC+1));
    }
  Case(GETNUMBERG)
    {
      TaggedRef i = getNumberArg(PC+1);
      TaggedRef term = Greg(getRegArg(PC+2));
      DEREF(term,termPtr,tag);

      if (isUVar(tag)) {
        bindOPT(termPtr, i);
        DISPATCH(3);
      }

      if (oz_numberEq(term,i)) {
        DISPATCH(3);
      }

      if (isCVar(term)) {
        tmpRet=oz_var_bind(tagged2CVar(term),termPtr,i);
        if (tmpRet==PROCEED) { DISPATCH(3); }
        if (tmpRet!=FAILED)  { goto LBLunifySpecial; }
        // fall through to fail
      }

      HF_TELL(Greg(getRegArg(PC+2)), getNumberArg(PC+1));
    }

/* getListValVar(N,R,M) == getList(X[N]) unifyVal(R) unifyVar(X[M]) */
  Case(GETLISTVALVARX)
    {
      TaggedRef term = Xreg(getRegArg(PC+1));
      DEREF(term,termPtr,tag);

      if (isUVar(term)) {
        register LTuple *ltuple = new LTuple();
        ltuple->setHead(Xreg(getRegArg(PC+2)));
        ltuple->setTail(e->currentUVarPrototype());
        bindOPT(termPtr,makeTaggedLTuple(ltuple));
        Xreg(getRegArg(PC+3)) = makeTaggedRef(ltuple->getRef()+1);
        DISPATCH(4);
      }
      if (oz_isLTuple(term)) {
        TaggedRef *argg = tagged2LTuple(term)->getRef();
        OZ_Return aux = fastUnify(Xreg(getRegArg(PC+2)),
                                  makeTaggedRef(argg));
        if (aux==PROCEED) {
          Xreg(getRegArg(PC+3)) = tagged2NonVariable(argg+1);
          DISPATCH(4);
        }
        if (aux!=FAILED) {
          tmpRet = aux;
          goto LBLunifySpecial;
        }
        HF_TELL(Xreg(getRegArg(PC+2)), makeTaggedRef(argg));
      }

      if (oz_isVariable(term)) {
        Assert(isCVar(term));
        // mm2: why not new LTuple(h,t)? OPT?
        register LTuple *ltuple = new LTuple();
        ltuple->setHead(Xreg(getRegArg(PC+2)));
        ltuple->setTail(e->currentUVarPrototype());
        tmpRet=oz_var_bind(tagged2CVar(term),termPtr,makeTaggedLTuple(ltuple));
        if (tmpRet==PROCEED) {
          Xreg(getRegArg(PC+3)) = makeTaggedRef(ltuple->getRef()+1);
          DISPATCH(4);
        }
        if (tmpRet!=FAILED)  { goto LBLunifySpecial; }

        HF_TELL(Xreg(getRegArg(PC+1)), makeTaggedLTuple(ltuple));
      }

      HF_TELL(Xreg(getRegArg(PC+1)), makeTaggedLTuple(new LTuple(Xreg(getRegArg(PC+2)),e->currentUVarPrototype())));
    }


  Case(GETLISTG) ONREG(getList,GREF);
  Case(GETLISTY) ONREG(getList,Y);
  Case(GETLISTX) ONREG(getList,X);
    {
    getList:
      TaggedRef term = RegAccess(HelpReg,getRegArg(PC+1));
      DEREF(term,termPtr,tag);

      if (isUVar(term)) {
        LTuple *ltuple = new LTuple();
        sPointer = ltuple->getRef();
        SetWriteMode;
        bindOPT(termPtr,makeTaggedLTuple(ltuple));
        DISPATCH(2);
      } else if (oz_isVariable(term)) {
        Assert(isCVar(term));
        TaggedRef record;
        if (DIST_UNIFY_FIX && oz_onToplevel()) {
          TaggedRef *regPtr = &(RegAccess(HelpReg,getRegArg(PC+1)));
          TaggedRef savedTerm = *regPtr;
          *regPtr = record = OZ_newVariable();
          buildRecord(PC,X,Y,CAP);
          record = oz_deref(record);
          sPointer = tagged2LTuple(record)->getRef();
          //message("buildRecord: %s\n",toC(record));
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

      HF_TELL(RegAccess(HelpReg,getRegArg(PC+1)), makeTaggedLTuple(new LTuple(e->currentUVarPrototype(),e->currentUVarPrototype())));
    }


  /* a unifyVariable in read mode */
  Case(GETVARIABLEX)
    {
      Xreg(getRegArg(PC+1)) = tagged2NonVariable(sPointer);
      sPointer++;
      DISPATCH(2);
    }
  Case(GETVARIABLEY)
    {
      Yreg(getRegArg(PC+1)) = tagged2NonVariable(sPointer);
      sPointer++;
      DISPATCH(2);
    }

  Case(GETVARVARXX)
    {
      Xreg(getRegArg(PC+1)) = tagged2NonVariable(sPointer);
      Xreg(getRegArg(PC+2)) = tagged2NonVariable(sPointer+1);
      sPointer += 2;
      DISPATCH(3);
    }
  Case(GETVARVARXY)
    {
      Xreg(getRegArg(PC+1)) = tagged2NonVariable(sPointer);
      Yreg(getRegArg(PC+2)) = tagged2NonVariable(sPointer+1);
      sPointer += 2;
      DISPATCH(3);
    }
  Case(GETVARVARYX)
    {
      Yreg(getRegArg(PC+1)) = tagged2NonVariable(sPointer);
      Xreg(getRegArg(PC+2)) = tagged2NonVariable(sPointer+1);
      sPointer += 2;
      DISPATCH(3);
    }
  Case(GETVARVARYY)
    {
      Yreg(getRegArg(PC+1)) = tagged2NonVariable(sPointer);
      Yreg(getRegArg(PC+2)) = tagged2NonVariable(sPointer+1);
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
      if(InWriteMode) {
        Reg reg = getRegArg(PC+1);
        TaggedRef *sp = GetSPointerWrite(sPointer);
        *sp = e->currentUVarPrototype();
        Xreg(reg) = makeTaggedRef(sp);
      } else {
        Xreg(getRegArg(PC+1)) = tagged2NonVariable(sPointer);
      }
      sPointer++;
      DISPATCH(2);
    }
  Case(UNIFYVARIABLEY)
    {
      if(InWriteMode) {
        Reg reg = getRegArg(PC+1);
        TaggedRef *sp = GetSPointerWrite(sPointer);
        *sp = e->currentUVarPrototype();
        Yreg(reg) = makeTaggedRef(sp);
      } else {
        Yreg(getRegArg(PC+1)) = tagged2NonVariable(sPointer);
      }
      sPointer++;
      DISPATCH(2);
    }


  Case(UNIFYVALUEX)
    {
      TaggedRef term = Xreg(getRegArg(PC+1));

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
      TaggedRef term = Yreg(getRegArg(PC+1));

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
      TaggedRef term = Greg(getRegArg(PC+1));

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

  Case(UNIFYVALVARXX) ONREG2(UnifyValVar,X,X);
  Case(UNIFYVALVARXY) ONREG2(UnifyValVar,X,Y);
  Case(UNIFYVALVARYX) ONREG2(UnifyValVar,Y,X);
  Case(UNIFYVALVARYY) ONREG2(UnifyValVar,Y,Y);
  Case(UNIFYVALVARGX) ONREG2(UnifyValVar,GREF,X);
  Case(UNIFYVALVARGY) ONREG2(UnifyValVar,GREF,Y);
  {
  UnifyValVar:

    Reg reg = getRegArg(PC+1);

    if (InWriteMode) {
      TaggedRef *sp = GetSPointerWrite(sPointer);
      *sp = RegAccess(HelpReg1,reg);

      *(sp+1) = e->currentUVarPrototype();
      RegAccess(HelpReg2,getRegArg(PC+2)) = makeTaggedRef(sp+1);

      sPointer += 2;
      DISPATCH(3);
    }

    tmpRet=fastUnify(RegAccess(HelpReg1,reg),makeTaggedRef(sPointer));
    if (tmpRet==PROCEED) {
      RegAccess(HelpReg2,getRegArg(PC+2)) = tagged2NonVariable(sPointer+1);
      sPointer += 2;
      DISPATCH(3);
    }
    if (tmpRet == FAILED) {
      HF_TELL(*sPointer, RegAccess(HelpReg1,reg));
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

       if (isUVar(tag)) {
         bindOPT(termPtr, atm);
         DISPATCH(2);
       }

       if ( oz_eq(term,atm) ) {
         DISPATCH(2);
       }

       if (isCVar(term)) {
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

      if (isUVar(tag)) {
        bindOPT(termPtr, i);
        DISPATCH(2);
      }

      if (oz_numberEq(term,i)) {
        DISPATCH(2);
      }

      if (isCVar(term)) {
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

  Case(MATCHX)
    {
      TaggedRef val     = Xreg(getRegArg(PC+1));
      IHashTable *table = (IHashTable *) getAdressArg(PC+2);
      DoSwitchOnTerm(val,table);
    }
  Case(MATCHY)
    {
      TaggedRef val     = Yreg(getRegArg(PC+1));
      IHashTable *table = (IHashTable *) getAdressArg(PC+2);
      DoSwitchOnTerm(val,table);
    }
  Case(MATCHG)
    {
      TaggedRef val     = Greg(getRegArg(PC+1));
      IHashTable *table = (IHashTable *) getAdressArg(PC+2);
      DoSwitchOnTerm(val,table);
    }


// ------------------------------------------------------------------------
// INSTRUCTIONS: (Fast-) Call/Execute Inline Funs/Rels
// ------------------------------------------------------------------------

  Case(FASTCALL)
    {
      PushCont(PC+3);   // PC+3 goes into a temp var (mm2)
      // goto LBLFastTailCall; // is not optimized away (mm2)
    }


  Case(FASTTAILCALL)
    //  LBLFastTailCall:
    {
      AbstractionEntry *entry = (AbstractionEntry *)getAdressArg(PC+1);

      COUNT(fastcalls);
      CallDoChecks(entry->getAbstr());

      IHashTable *table = entry->indexTable;
      if (table) {
        PC = entry->getPC();
        DoSwitchOnTerm(X[0],table);
      } else {
        JUMPABSOLUTE(entry->getPC());
      }
    }

  Case(CALLBI)
    {
      COUNT(optbicalls);
      Builtin* bi = GetBI(PC+1);
      OZ_Location* loc = GetLoc(PC+2);

      Assert(loc->getOutArity()==bi->getOutArity());
      Assert(loc->getInArity()==bi->getInArity());

#ifdef PROFILE_BI
      bi->incCounter();
#endif

      int res = bi->getFun()(X,loc->mapping());
      if (res == PROCEED) { DISPATCH(3); }
      switch (res) {
      case FAILED:        HF_BI_NEW(bi,loc);
      case RAISE:
        if (e->exception.debug) set_exception_info_call(bi,X,loc->mapping());
        RAISE_THREAD;

      case BI_TYPE_ERROR: RAISE_TYPE_NEW(bi,loc);

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
      COUNT(optbicalls);
      Builtin* bi = GetBI(PC+1);
      OZ_Location* loc = GetLoc(PC+2);

      Assert(loc->getInArity()==bi->getInArity());
      Assert(bi->getOutArity()>=1);
      Assert(loc->getOutArity()==bi->getOutArity());

#ifdef PROFILE_BI
      bi->incCounter();
#endif
      int ret = bi->getFun()(X,loc->mapping());
      if (ret==PROCEED) {
        if (oz_isTrue(X[loc->out(0)])) {
          DISPATCH(4);
        } else {
          Assert(oz_isFalse(X[loc->out(0)]));
          JUMPRELATIVE(getLabelArg(PC+3));
        }
      }

      switch (ret) {
      case RAISE:
        if (e->exception.debug) set_exception_info_call(bi,X,loc->mapping());
        RAISE_THREAD;
      case BI_TYPE_ERROR:
        RAISE_TYPE_NEW(bi,loc);

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
        OZ_error("unexcpeted return value in TESTBI: %d",ret);
      }
    }


  Case(INLINEMINUS)
    {
      COUNT(inlinecalls);

      TaggedRef A = XPC(1); DEREF0(A,_1,tagA);

      if (isSmallIntTag(tagA)) {
        TaggedRef B = XPC(2); DEREF0(B,_2,tagB);
        if (isSmallIntTag(tagB) ) {
          XPC(3) = oz_int(smallIntValue(A) - smallIntValue(B));
          DISPATCH(4);
        }
      }

      if (isFloatTag(tagA)) {
        TaggedRef B = XPC(2); DEREF0(B,_2,tagB);
        if (isFloatTag(tagB)) {
          XPC(3) = oz_float(floatValue(A) - floatValue(B));
          DISPATCH(4);
        }
      }

      auxTaggedA = XPC(1);
      auxTaggedB = XPC(2);
      auxInt     = 4;
      auxString = "-";

      tmpRet = BIminusOrPlus(NO,auxTaggedA,auxTaggedB,XPC(3));
      goto LBLhandlePlusMinus;
    }

  Case(INLINEPLUS)
    {
      COUNT(inlinecalls);

      TaggedRef A = XPC(1); DEREF0(A,_1,tagA);

      if ( isSmallIntTag(tagA)) {
        TaggedRef B = XPC(2); DEREF0(B,_2,tagB);
        if (isSmallIntTag(tagB) ) {
          XPC(3) = oz_int(smallIntValue(A) + smallIntValue(B));
          DISPATCH(4);
        }
      }

      if (isFloatTag(tagA)) {
        TaggedRef B = XPC(2); DEREF0(B,_2,tagB);
        if (isFloatTag(tagB)) {
          XPC(3) = oz_float(floatValue(A) + floatValue(B));
          DISPATCH(4);
        }
      }

      auxTaggedA = XPC(1);
      auxTaggedB = XPC(2);
      auxInt     = 4;
      auxString = "+";

      tmpRet = BIminusOrPlus(OK,auxTaggedA,auxTaggedB,XPC(3));
      goto LBLhandlePlusMinus;
    }

  Case(INLINEMINUS1)
    {
      COUNT(inlinecalls);

      TaggedRef A = XPC(1); DEREF0(A,_1,tagA);

      if (isSmallIntTag(tagA)) {
        /* INTDEP */
        int res = (int)A - (1<<tagSize);
        if ((int)A > res) {
          XPC(2) = res;
          DISPATCH(3);
        }
      }

      auxTaggedA = XPC(1);
      auxTaggedB = makeTaggedSmallInt(1);
      auxInt     = 3;
      auxString = "-1";

      tmpRet = BIminusOrPlus(NO,auxTaggedA,auxTaggedB,XPC(2));
      goto LBLhandlePlusMinus;
    }

  Case(INLINEPLUS1)
    {
      COUNT(inlinecalls);

      TaggedRef A = XPC(1); DEREF0(A,_1,tagA);

      if (isSmallIntTag(tagA)) {
        /* INTDEP */
        int res = (int)A + (1<<tagSize);
        if ((int)A < res) {
          XPC(2) = res;
          DISPATCH(3);
        }
      }

      auxTaggedA = XPC(1);
      auxTaggedB = makeTaggedSmallInt(1);
      auxInt     = 3;
      auxString = "+1";

      tmpRet = BIminusOrPlus(OK,auxTaggedA,auxTaggedB,XPC(2));
      goto LBLhandlePlusMinus;
    }


  LBLhandlePlusMinus:
  {
      switch(tmpRet) {
      case PROCEED:       DISPATCH(auxInt);
      case BI_TYPE_ERROR: RAISE_TYPE1_FUN(auxString,
                                          oz_cons(auxTaggedA,oz_cons(auxTaggedB,oz_nil())));

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
      COUNT(inlinedots);
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
          RAISE_TYPE1_FUN(".", oz_cons(XPC(1), oz_cons(feature, oz_nil())));

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
        (void) oz_raise(E_ERROR,E_KERNEL,"globalState",1,OZ_atom("object"));
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

  Case(INLINEUPARROW)
    {
      switch(uparrowInlineBlocking(XPC(1),XPC(2),XPC(3))) {
      case PROCEED:
        DISPATCH(4);

      case SUSPEND:
          OZ_suspendOnInternal2(XPC(1),XPC(2));
          CheckLiveness(PC);
          PushContX(PC);
          SUSPENDONVARLIST;

      case FAILED:
        HF_APPLY(OZ_atom("Record.'^'"),
                 oz_cons(XPC(1),oz_cons(XPC(2),oz_nil())));

      case RAISE:
        RAISE_THREAD;

      case BI_TYPE_ERROR:
        RAISE_TYPE1_FUN("^",oz_cons(XPC(1),oz_cons(XPC(2),oz_nil())));

      case SLEEP:
      default:
        Assert(0);
      }
    }


// ------------------------------------------------------------------------
// INSTRUCTIONS: Testing
// ------------------------------------------------------------------------

#define LT_IF(T) if (T) THEN_CASE else ELSE_CASE
#define THEN_CASE { XPC(3)=oz_true(); DISPATCH(5); }
#define ELSE_CASE { XPC(3)=oz_false(); JUMPRELATIVE(getLabelArg(PC+4)); }

  Case(TESTLT)
    {
      COUNT(inlinecalls);

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
        RAISE_TYPE1(auxString,oz_cons(XPC(1),oz_cons(XPC(2),oz_nil())));

      default:
        Assert(0);
      }
    }

  Case(TESTLE)
    {
      COUNT(inlinecalls);

      TaggedRef A = XPC(1); DEREF0(A,_1,tagA);
      TaggedRef B = XPC(2); DEREF0(B,_2,tagB);

      if (tagA == tagB) {
        if (tagA == SMALLINT) {
          LT_IF(smallIntLE(A,B));
        }

        if (isFloatTag(tagA)) {
          LT_IF(floatValue(A) <= floatValue(B));
        }

        if (tagA == LITERAL) {
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
      Y = allocateY(posInt);
      DISPATCH(2);
    }

  Case(ALLOCATEL1)  { Y =  allocateY(1); DISPATCH(1); }
  Case(ALLOCATEL2)  { Y =  allocateY(2); DISPATCH(1); }
  Case(ALLOCATEL3)  { Y =  allocateY(3); DISPATCH(1); }
  Case(ALLOCATEL4)  { Y =  allocateY(4); DISPATCH(1); }
  Case(ALLOCATEL5)  { Y =  allocateY(5); DISPATCH(1); }
  Case(ALLOCATEL6)  { Y =  allocateY(6); DISPATCH(1); }
  Case(ALLOCATEL7)  { Y =  allocateY(7); DISPATCH(1); }
  Case(ALLOCATEL8)  { Y =  allocateY(8); DISPATCH(1); }
  Case(ALLOCATEL9)  { Y =  allocateY(9); DISPATCH(1); }
  Case(ALLOCATEL10) { Y = allocateY(10); DISPATCH(1); }

  Case(DEALLOCATEL1)  { deallocateY(Y,1);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL2)  { deallocateY(Y,2);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL3)  { deallocateY(Y,3);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL4)  { deallocateY(Y,4);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL5)  { deallocateY(Y,5);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL6)  { deallocateY(Y,6);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL7)  { deallocateY(Y,7);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL8)  { deallocateY(Y,8);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL9)  { deallocateY(Y,9);  Y=0; DISPATCH(1); }
  Case(DEALLOCATEL10) { deallocateY(Y,10); Y=0; DISPATCH(1); }

  Case(DEALLOCATEL)
    {
      if (Y) {
        deallocateY(Y);
        Y = NULL;
      }
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
    JUMPRELATIVE(getLabelArg(PC+1));

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
    SUSP_PC(auxPtr,PC);}

  if (!oz_isLock(aux)) {
    /* arghhhhhhhhhh! fucking exceptions (RS) */
    (void) oz_raise(E_ERROR,E_KERNEL,"type",5,
                    NameUnit,
                    NameUnit,
                    OZ_atom("Lock"),
                    OZ_int(1),
                    OZ_string(""));
    RAISE_TYPE1("lock",oz_cons(aux,oz_nil()));
  }

  OzLock *t = (OzLock*)tagged2Tert(aux);
  Thread *th=oz_currentThread();

  if(t->isLocal()){
    if(!oz_onToplevel()){
      if (!oz_isCurrentBoard(GETBOARD((LockLocal*)t))) {
        (void) oz_raise(E_ERROR,E_KERNEL,"globalState",1,OZ_atom("lock"));
        RAISE_THREAD;}}
    if(((LockLocal*)t)->hasLock(th)) {goto has_lock;}
    if(((LockLocal*)t)->lockB(th)) {goto got_lock;}
    goto no_lock;}

  if(!oz_onToplevel()){
    (void) oz_raise(E_ERROR,E_KERNEL,"globalState",1,OZ_atom("lock"));
    RAISE_THREAD;
  }

  LockRet ret;

  switch(t->getTertType()){
  case Te_Frame:{
    if(((LockFrameEmul *)t)->hasLock(th)) {goto has_lock;}
    ret = ((LockFrameEmul *)t)->lockB(th);
    break;}
  case Te_Proxy:{
    (*lockLockProxy)(t, th);
    goto no_lock;}
  case Te_Manager:{
    if(((LockManagerEmul *)t)->hasLock(th)) {goto has_lock;}
    ret=((LockManagerEmul *)t)->lockB(th);
    break;}
  default:
    Assert(0);}

  if(ret==LOCK_GOT) goto got_lock;
  if(ret==LOCK_WAIT) goto no_lock;

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
        Assert(CBB==currentDebugBoard);

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
      Reg reg                     = getRegArg(PC+1);
      int nxt                     = getLabelArg(PC+2);
      PrTabEntry *predd           = getPredArg(PC+3);
      AbstractionEntry *predEntry = (AbstractionEntry*) getAdressArg(PC+4);
      AssRegArray *list           = (AssRegArray*) getAdressArg(PC+5);
      int size = list->getSize();

      if (predd->getPC()==NOCODE) {
        predd->PC = PC+sizeOf(DEFINITION);
        predd->setGSize(size);
      }

      predd->numClosures++;

      if (isTailCall) { // was DEFINITIONCOPY?
        TaggedRef list = oz_deref(Xreg(reg));
        ProgramCounter preddPC = predd->PC;
        predd = new PrTabEntry(predd->getName(), predd->getMethodArity(),
                               predd->getFile(), predd->getLine(), predd->getColumn(),
                               oz_nil(), // mm2: inherit sited?
                               predd->getMaxX());
        predd->PC = copyCode(preddPC,list);
        predd->setGSize(size);
      }

      Abstraction *p = Abstraction::newAbstraction(predd, CBB);

      COUNT(numClosures);
      COUNT1(sizeGs,size);

      if (predEntry) {
        predEntry->setPred(p);
      }

      for (int i = 0; i < size; i++) {
        switch ((*list)[i].kind) {
        case XReg: p->initG(i, X[(*list)[i].number]); break;
        case YReg: p->initG(i, Y[(*list)[i].number]); break;
        case GReg: p->initG(i, CAP->getG((*list)[i].number)); break;
        }
      }
      Xreg(reg) = makeTaggedConst(p);
      JUMPRELATIVE(nxt);
    }

// -------------------------------------------------------------------------
// INSTRUCTIONS: CONTROL: FENCE/CALL/EXECUTE/SWITCH/BRANCH
// -------------------------------------------------------------------------

  Case(BRANCH)
    JUMPRELATIVE( getLabelArg(PC+1) );


  Case(TAILSENDMSGX) isTailCall = OK; ONREG(SendMethod,X);
  Case(TAILSENDMSGY) isTailCall = OK; ONREG(SendMethod,Y);
  Case(TAILSENDMSGG) isTailCall = OK; ONREG(SendMethod,GREF);

  Case(SENDMSGX) isTailCall = NO; ONREG(SendMethod,X);
  Case(SENDMSGY) isTailCall = NO; ONREG(SendMethod,Y);
  Case(SENDMSGG) isTailCall = NO; ONREG(SendMethod,GREF);

 SendMethod:
  {
    TaggedRef label    = getLiteralArg(PC+1);
    TaggedRef origObj  = RegAccess(HelpReg,getRegArg(PC+2)); // mm2
    TaggedRef object   = origObj;
    SRecordArity arity = (SRecordArity) getAdressArg(PC+3);

    DEREF(object,objectPtr,_2);
    if (oz_isObject(object)) {
      Object *obj      = tagged2Object(object);
      Abstraction *def = getSendMethod(obj,label,arity,(InlineCache*)(PC+4),X);
      if (def == NULL) {
        goto bombSend;
      }

      if (!isTailCall) PushCont(PC+6);
      ChangeSelf(obj);
      CallDoChecks(def);
      COUNT(sendmsg);
      JUMPABSOLUTE(def->getPC());
    }

    if (oz_isVariable(object)) {
      SUSP_PC(objectPtr,PC);
    }

    if (oz_isProcedure(object))
      goto bombSend;

    RAISE_APPLY(object, oz_cons(makeMessage(arity,label,X),oz_nil()));

  bombSend:
    if (!isTailCall) PC = PC+6;
    X[0] = makeMessage(arity,label,X);
    predArity = 1;
    predicate = tagged2Const(object);
    goto LBLcall;
  }





  Case(CALLX) isTailCall = NO; ONREG(Call,X);
  Case(CALLY) isTailCall = NO; ONREG(Call,Y);
  Case(CALLG) isTailCall = NO; ONREG(Call,GREF);

  Case(TAILCALLX) isTailCall = OK; ONREG(Call,X);
  Case(TAILCALLG) isTailCall = OK; ONREG(Call,GREF);

 Call:
  asmLbl(TAILCALL);
   {
     {
       TaggedRef taggedPredicate = RegAccess(HelpReg,getRegArg(PC+1));
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
         RAISE_APPLY(taggedPredicate,OZ_toList(predArity,X));
       }

       if (!isTailCall) PC = PC+3;
       predicate = tagged2Const(taggedPredicate);
     }

// -----------------------------------------------------------------------
// --- Call: entry point
// -----------------------------------------------------------------------

  LBLcall:
     COUNT(nonoptcalls);
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
         COUNT(nonoptsendmsg);
         CheckArity(1, makeTaggedConst(predicate));
         Object *o = (Object*) predicate;
         Assert(o->getClass()->getFallbackApply());
         Abstraction *def =
           tagged2Abstraction(o->getClass()->getFallbackApply());
         /* {Obj Msg} --> {SetSelf Obj} {FallbackApply Class Msg} */
         X[1] = X[0];
         X[0] = makeTaggedConst(o->getClass());
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
       COUNT(nonoptbicalls);

       bi = (Builtin *) predicate;

       CheckArity(bi->getArity(),makeTaggedConst(bi));

#ifdef PROFILE_BI
       bi->incCounter();
#endif
       OZ_Return res = oz_bi_wrapper(bi,X);

       switch (res) {

       case SUSPEND:
         {
           if (!isTailCall) PushCont(PC);

           CTT->pushCall(makeTaggedConst(bi),X,predArity);
           SUSPENDONVARLIST;
         }

       case PROCEED:
         if (isTailCall) {
           goto LBLpopTask;
         }
         JUMPABSOLUTE(PC);

       case SLEEP:         Assert(0);
       case RAISE:
         if (e->exception.debug) set_exception_info_call(bi,X);
         RAISE_THREAD;
       case BI_TYPE_ERROR: RAISE_TYPE(bi);
       case FAILED:        HF_BI(bi);

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
     OZ_error("impossible");


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

// KOSTJA: THIS IS DEAD CS

  Case(FAILURE)
  Case(UNIFYYX)
  Case(UNIFYYY)
  Case(UNIFYYG)
  Case(UNIFYGX)
  Case(UNIFYGY)
  Case(UNIFYGG)

  Case(GETLISTVALVARY)
  Case(GETLISTVALVARG)
  Case(GETVARVARGX)
  Case(GETVARVARGY)
  Case(GETVARVARGG)
  Case(GETVARVARXG)
  Case(GETVARVARYG)
  Case(MOVEGG)
  Case(MOVEXG)
  Case(MOVEYG)
  Case(PUTRECORDG)
  Case(PUTCONSTANTG)
  Case(PUTLISTG)
  Case(SETVARIABLEG)
  Case(GETVARIABLEG)
  Case(UNIFYVARIABLEG)
  Case(UNIFYVALVARXG)
  Case(UNIFYVALVARYG)
  Case(UNIFYVALVARGG)


  Case(TAILCALLY)
  Case(TAILAPPLMETHX)
  Case(TAILAPPLMETHY)
  Case(TAILAPPLMETHG)
  Case(APPLMETHX)
  Case(APPLMETHY)
  Case(APPLMETHG)

  Case(SHALLOWGUARD)
  Case(SHALLOWTHEN)
  Case(WAIT)
  Case(WAITTOP)
  Case(ASK)
  Case(CREATECOND)
  Case(CREATEOR)
  Case(CREATEENUMOR)
  Case(CREATECHOICE)
  Case(CLAUSE)
  Case(EMPTYCLAUSE)
  Case(NEXTCLAUSE)
  Case(LASTCLAUSE)
  Case(THREAD)

  Case(TESTLABEL1)
  Case(TESTLABEL2)
  Case(TESTLABEL3)
  Case(TESTLABEL4)

  Case(TEST1)
  Case(TEST2)
  Case(TEST3)
  Case(TEST4)
  Case(CREATEVARIABLEG)
  Case(CREATEVARIABLEMOVEG)
    {
      OZ_error("DEPRECATED DEEP GUARD INSTRUCTION (%s) ENCOUNTERED.\n PLEASE RECOMPILE YOUR PROGRAMS!!!",opcodeToString(CodeArea::getOpcode(PC)));
      return T_ERROR;
    }

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
      ozstat.leaveCall((PrTabEntry*)Y);
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
        X[i] = args[i];
      }
      disposeRefsArray(args);
      isTailCall = OK;

      predicate=tagged2Const(taggedPredicate);
      goto LBLcall;
    }

  Case(TASKLOCK)
    {
      OzLock *lck = (OzLock *) Y;
      Y = NULL;
      switch(lck->getTertType()){
      case Te_Local:
        ((LockLocal*)lck)->unlock();
        break;
      case Te_Frame:
        ((LockFrameEmul *)lck)->unlock(oz_currentThread());
        break;
      case Te_Proxy:
        oz_raise(E_ERROR,E_KERNEL,"globalState",1,OZ_atom("lock"));
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
        X[i] = tmpX[i];
      }
      disposeRefsArray(tmpX);
      goto LBLpopTaskNoPreempt;
    }

  Case(TASKSETSELF)
    {
      e->setSelf((Object*)Y);
      Y = NULL;
      goto LBLpopTaskNoPreempt;
    }

  Case(TASKDEBUGCONT)
    {
      OZ_warning("\n      TASKDEBUGCONT instruction executed -- "
                 "this should not happen.\n      "
                 "Please send a bug report to <lorenz@dfki.de>.");
      if (e->debugmode() && CTT->getTrace())
        debugStreamUpdate(CTT);
      ((OzDebug *) Y)->dispose();
      goto LBLpopTaskNoPreempt;
    }

  Case(TASKCFUNCONT)
     {
       OZ_CFun biFun = (OZ_CFun) (void*) Y;
       RefsArray tmpX = (RefsArray) CAP;
       CAP = 0;
       Y = 0;
       if (tmpX != NULL) {
         predArity = getRefsArraySize(tmpX);
         int i = predArity;
         while (--i >= 0) {
           X[i] = tmpX[i];
         }
       } else {
         predArity = 0;
       }
       disposeRefsArray(tmpX);

       DebugTrace(ozd_trace(cfunc2Builtin((void *) biFun)->getPrintName()));

       switch (biFun(X,OZ_ID_MAP)) {
       case PROCEED:       goto LBLpopTask;
       case FAILED:        HF_BI(cfunc2Builtin((void *) biFun));
       case RAISE:
         if (e->exception.debug)
           set_exception_info_call(cfunc2Builtin((void *) biFun),X);
         RAISE_THREAD_NO_PC;
       case BI_TYPE_ERROR: RAISE_TYPE(cfunc2Builtin((void *) biFun));

       case BI_REPLACEBICALL:
         PC = NOCODE;
         Assert(!e->isEmptyPreparedCalls());
         goto LBLreplaceBICall;

       case SUSPEND:
         CTT->pushCFun(biFun,X,predArity);
         SUSPENDONVARLIST;

      case BI_PREEMPT:
        return T_PREEMPT;

       case SLEEP:
       default:
         Assert(0);
       }
     }

  Case(OZERROR)
    {
      OZ_error("Emulate: OZERROR instruction executed");
      return T_ERROR;
    }

  Case(DEBUGENTRY)
    {
      if ((e->debugmode() || CTT->getTrace()) && oz_onToplevel()) {
        int line = smallIntValue(getNumberArg(PC+2));
        if (line < 0) {
          execBreakpoint(oz_currentThread());
        }

        OzDebug *dbg = new OzDebug(PC,Y,CAP);

        TaggedRef kind = getTaggedArg(PC+4);
        if (oz_eq(kind,AtomDebugCallC) ||
            oz_eq(kind,AtomDebugCallF)) {
          // save abstraction and arguments:
          int arity = -1;
          switch (CodeArea::getOpcode(PC+5)) {
          case CALLBI:
            {
              Builtin *bi = GetBI(PC+6);
              dbg->data = makeTaggedConst(bi);
              int iarity = bi->getInArity(), oarity = bi->getOutArity();
              int *map = GetLoc(PC+7)->mapping();
              dbg->arguments = allocateRefsArray(iarity+oarity+1,NO);
              int i;
              for (i = 0; i < iarity; i++)
                dbg->arguments[i] = X[map == OZ_ID_MAP? i: map[i]];
              for (i = 0; i < oarity; i++)
                dbg->arguments[iarity + i] =
                  CTT->getStep()? OZ_newVariable(): makeTaggedNULL();
              dbg->arguments[iarity + oarity] = makeTaggedNULL();
            }
            break;
          case CALLX:
            dbg->data = Xreg(getRegArg(PC+6));
            arity = getPosIntArg(PC+7);
            break;
          case CALLY:
            dbg->data = Yreg(getRegArg(PC+6));
            arity = getPosIntArg(PC+7);
            break;
          case CALLG:
            dbg->data = Greg(getRegArg(PC+6));
            arity = getPosIntArg(PC+7);
            break;
          case GENFASTCALL:
          case FASTCALL:
            {
              Abstraction *abstr =
                ((AbstractionEntry *) getAdressArg(PC+6))->getAbstr();
              dbg->data = makeTaggedConst(abstr);
              arity = abstr->getArity();
            }
            break;
          case MARSHALLEDFASTCALL:
            dbg->data = getTaggedArg(PC+6);
            arity = getPosIntArg(PC+7) >> 1;
            break;
          default:
            break;
          }
          if (arity >= 0) {
            dbg->arguments = allocateRefsArray(arity+1,NO);
            for (int i = 0; i < arity; i++) {
              dbg->arguments[i] = Xreg(intToReg(i));
            }
            dbg->arguments[arity] = makeTaggedNULL();
          }
        } else if (oz_eq(kind,AtomDebugLockC) ||
                   oz_eq(kind,AtomDebugLockF)) {
          // save the lock:
          switch (CodeArea::getOpcode(PC+5)) {
          case LOCKTHREAD:
            dbg->setSingleArgument(Xreg(getRegArg(PC+7)));
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
            dbg->setSingleArgument(Xreg(getRegArg(PC+6)));
            break;
          case TESTLITERALY:
          case TESTNUMBERY:
          case TESTRECORDY:
          case TESTLISTY:
          case TESTBOOLY:
          case MATCHY:
            dbg->setSingleArgument(Yreg(getRegArg(PC+6)));
            break;
          case TESTLITERALG:
          case TESTNUMBERG:
          case TESTRECORDG:
          case TESTLISTG:
          case TESTBOOLG:
          case MATCHG:
            dbg->setSingleArgument(Greg(getRegArg(PC+6)));
            break;
          default:
            break;
          }
        } else if (oz_eq(kind,AtomDebugNameC) ||
                   oz_eq(kind,AtomDebugNameF)) {
          switch (CodeArea::getOpcode(PC+5)) {
          case PUTCONSTANTX:
          case PUTCONSTANTY:
          case PUTCONSTANTG:
          case GETLITERALX:
          case GETLITERALY:
          case GETLITERALG:
            dbg->setSingleArgument(getTaggedArg(PC+6));
            break;
          default:
            break;
          }
        }

        if (CTT->getStep()) {
          CTT->pushDebug(dbg,DBG_STEP);
          debugStreamEntry(dbg,CTT->getTaskStackRef()->getFrameId());
          INCFPC(5);
          PushContX(PC);
          return T_PREEMPT;
        } else {
          CTT->pushDebug(dbg,DBG_NOSTEP);
        }
      } else if (e->isPropagatorLocation()) {
        OzDebug *dbg = new OzDebug(PC,NULL,CAP);
        CTT->pushDebug(dbg,DBG_EXIT);
      }

      DISPATCH(5);
    }

  Case(DEBUGEXIT)
    {
      OzDebug *dbg;
      OzDebugDoit dothis;
      CTT->popDebug(dbg, dothis);

      if (dbg != (OzDebug *) NULL) {
        Assert(oz_eq(getLiteralArg(dbg->PC+4),getLiteralArg(PC+4)));
        Assert(e->isPropagatorLocation() || (dbg->Y == Y && dbg->CAP == CAP));

        if (dothis != DBG_EXIT
            && (oz_eq(getLiteralArg(PC+4),AtomDebugCallC) ||
                oz_eq(getLiteralArg(PC+4),AtomDebugCallF))
            && CodeArea::getOpcode(dbg->PC+5) == CALLBI) {
          Builtin *bi = GetBI(dbg->PC+6);
          int iarity = bi->getInArity(), oarity = bi->getOutArity();
          int *map = GetLoc(dbg->PC+7)->mapping();
          if (oarity)
            if (dbg->arguments[iarity] != makeTaggedNULL())
              for (int i = 0; i < oarity; i++) {
                TaggedRef x = X[map == OZ_ID_MAP? iarity + i: map[iarity + i]];
                if (OZ_unify(dbg->arguments[iarity + i], x) == FAILED)
                  return T_FAILURE;
              }
            else
              for (int i = 0; i < oarity; i++)
                dbg->arguments[iarity + i] =
                  X[map == OZ_ID_MAP? iarity + i: map[iarity + i]];
        }

        if (dothis == DBG_STEP && CTT->getTrace()) {
          dbg->PC = PC;
          CTT->pushDebug(dbg,DBG_EXIT);
          debugStreamExit(dbg,CTT->getTaskStackRef()->getFrameId());
          PushContX(PC);
          return T_PREEMPT;
        }

        dbg->dispose();
      }

      DISPATCH(5);
    }

  Case(GENFASTCALL)
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

  Case(MARSHALLEDFASTCALL)
    {
      TaggedRef pred = getTaggedArg(PC+1);
      int tailcallAndArity  = getPosIntArg(PC+2);

      DEREF(pred,predPtr,_1);
      if (oz_isVariable(pred)) {
        SUSP_PC(predPtr,PC);
      }

      if (oz_isAbstraction(pred)) {
        CodeArea *code = CodeArea::findBlock(PC);
        code->unprotect((TaggedRef*)(PC+1));
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
      RAISE_APPLY(pred,oz_cons(OZ_atom("proc or builtin expected."),oz_nil()));
    }

  Case(GENCALL)
    {
      GenCallInfoClass *gci = (GenCallInfoClass*)getAdressArg(PC+1);
      int arity = getPosIntArg(PC+2);
      Assert(arity==0); /* is filled in by procedure genCallInfo */
      TaggedRef pred = CAP->getG(gci->regIndex);
      DEREF(pred,predPtr,_1);
      if (oz_isVariable(pred)) {
        SUSP_PC(predPtr,PC);
      }

      if (genCallInfo(gci,pred,PC)) {
        gci->dispose();
        DISPATCH(0);
      }

      isTailCall = gci->isTailCall;
      if (!isTailCall) PC = PC+3;

      /* the following adapted from bombApply */
      Assert(tagged2ObjectClass(pred)->getFallbackApply());

      X[1] = makeMessage(gci->arity,gci->mn,X);
      X[0] = pred;

      predArity = 2;
      predicate = tagged2Const(tagged2ObjectClass(pred)->getFallbackApply());
      goto LBLcall;
    }


  /* The following must be different from the following,
   * otherwise definitionEnd breaks under threaded code
   */

  Case(GLOBALVARNAME)
    {
      OZ_error("under threaded code this must be different from LOCALVARNAME,");
      OZ_error("otherwise CodeArea::adressToOpcode will not work.");
    }

  Case(LOCALVARNAME)
    {
      OZ_error("under threaded code this must be different from GLOBALVARNAME,");
      OZ_error("otherwise CodeArea::adressToOpcode will not work.");
    }

  Case(PROFILEPROC)
    {
      static int sizeOfDef = -1;
      if (sizeOfDef==-1) sizeOfDef = sizeOf(DEFINITION);

      Assert(CodeArea::getOpcode(PC-sizeOfDef) == DEFINITION);
      PrTabEntry *pred = getPredArg(PC-sizeOfDef+3); /* this is faster */

      pred->numCalled++;
      if (pred!=ozstat.currAbstr) {
        CTS->pushAbstr(ozstat.currAbstr);
        ozstat.leaveCall(pred);
      }

      DISPATCH(1);
    }

  Case(TASKCATCH)
    {
      OZ_error("impossible");
      /* remove unused continuation for handler */
      CTS->discardFrame(NOCODE);
      DISPATCH(1);
    }

  Case(ENDOFFILE)
    {
      OZ_error("Emulate: ENDOFFILE instruction executed");
      return T_ERROR;
    }

  Case(ENDDEFINITION)
    {
      OZ_error("Emulate: ENDDEFINITION instruction executed");
      return T_ERROR;
    }

#ifndef THREADED
  default:
    OZ_error("emulate instruction: default should never happen");
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

  Assert(0);
  return T_ERROR;

} // end engine


// ------------------------------------------------------------------------
// *** FAILURE
// ------------------------------------------------------------------------


#undef ONREG
#undef ONREG2
#undef DISPATCH
#define ONREG(Label,R)      auxReg = (R); goto Label
#define ONREG2(Label,R1,R2) auxReg = (R1); auxReg1 = (R2); goto Label
#define DISPATCH(incPC,incArgs)                 \
   PC += incPC;                                 \
   argsToHandle += incArgs;                     \
   break;

Bool isGetRecAndFriends(Opcode op)
{
#if DIST_UNIFY_FIX > 1
  return NO; // don't build nested structures
#else
  switch(op) {

  case GETRECORDX:
  case GETRECORDY:
  case GETRECORDG:
  case GETLISTVALVARX:
  case GETLISTVALVARY:
  case GETLISTVALVARG:
  case GETLISTX:
  case GETLISTY:
  case GETLISTG:
    return OK;

  default:
    return NO;
  }
#endif
}

void buildRecord(ProgramCounter PC, RefsArray X, RefsArray Y,Abstraction *CAP)
{
#if DIST_UNIFY_FIX > 0
  Assert(oz_onToplevel());
  TaggedRef *sPointer;
  RefsArray auxReg, auxReg1;
  int argsToHandle = 0;

  int maxX = CAP->getPred()->getMaxX();
  RefsArray savedX = maxX ? allocateRefsArray(maxX,NO) : 0;
  {
    for (int i = 0; i < maxX; i++)
      savedX[i] = X[i];
  }
  int maxY = Y ? getRefsArraySize(Y) : 0;
  RefsArray savedY = Y ? allocateRefsArray(maxY,NO) : 0;
  {
    for (int i = 0; i < maxY; i++)
      savedY[i] = Y[i];
  }

  Bool firstCall = OK;
  while(1) {
    Opcode op = CodeArea::getOpcode(PC);
    if (!firstCall && argsToHandle==0 && !isGetRecAndFriends(op))
      goto exit;
    firstCall = NO;
    switch(op) {

    case GETRECORDX: ONREG(getRecord,X);
    case GETRECORDY: ONREG(getRecord,Y);
    case GETRECORDG: ONREG(getRecord,GREF);
      {
      getRecord:
        TaggedRef label = getLiteralArg(PC+1);
        SRecordArity ff = (SRecordArity) getAdressArg(PC+2);
        TaggedRef term = RegAccess(auxReg,getRegArg(PC+3));
        DEREF(term,termPtr,tag);

        Assert(isUVar(term));
        int numArgs = getWidth(ff);
        SRecord *srecord = SRecord::newSRecord(label,ff, numArgs);
        bindOPT(termPtr,makeTaggedSRecord(srecord));
        sPointer = srecord->getRef();
        DISPATCH(4,numArgs);
      }

    case GETLITERALX: ONREG(getNumber,X);
    case GETLITERALY: ONREG(getNumber,Y);
    case GETLITERALG: ONREG(getNumber,GREF);
    case GETNUMBERX:  ONREG(getNumber,X);
    case GETNUMBERY:  ONREG(getNumber,Y);
    case GETNUMBERG:  ONREG(getNumber,GREF);
    getNumber:
    {
      TaggedRef i = getNumberArg(PC+1);
      TaggedRef term = RegAccess(auxReg,getRegArg(PC+2));
      DEREF(term,termPtr,tag);
      Assert(isUVar(tag));
      bindOPT(termPtr, i);
      DISPATCH(3,-1);
    }


    case GETLISTVALVARX: ONREG(getListValVar,X);
    case GETLISTVALVARY: ONREG(getListValVar,Y);
    case GETLISTVALVARG: ONREG(getListValVar,GREF);
    getListValVar:
      {
        TaggedRef term = RegAccess(X,getRegArg(PC+1));
        DEREF(term,termPtr,tag);

        Assert(isUVar(term));
        LTuple *ltuple = new LTuple();
        ltuple->setHead(RegAccess(auxReg,getRegArg(PC+2)));
        ltuple->setTail(am.currentUVarPrototype());
        bindOPT(termPtr,makeTaggedLTuple(ltuple));
        RegAccess(X,getRegArg(PC+3)) = makeTaggedRef(ltuple->getRef()+1);
        DISPATCH(4,0);
      }

    case GETLISTX: ONREG(getList,X);
    case GETLISTY: ONREG(getList,Y);
    case GETLISTG: ONREG(getList,GREF);
    getList:
      {
        TaggedRef aux = RegAccess(auxReg,getRegArg(PC+1));
        DEREF(aux,auxPtr,tag);

        Assert(isUVar(aux));
        LTuple *ltuple = new LTuple();
        sPointer = ltuple->getRef();
        bindOPT(auxPtr,makeTaggedLTuple(ltuple));
        DISPATCH(2,2);
      }

    case UNIFYVARIABLEX: ONREG(unifyVariable,X);
    case UNIFYVARIABLEY: ONREG(unifyVariable,Y);
    case UNIFYVARIABLEG: ONREG(unifyVariable,GREF);
    unifyVariable:
    {
        *sPointer = am.currentUVarPrototype();
        RegAccess(auxReg,getRegArg(PC+1)) = makeTaggedRef(sPointer++);
        DISPATCH(2,-1);
      }

    case UNIFYVALUEX: ONREG(unifyValue,X);
    case UNIFYVALUEY: ONREG(unifyValue,Y);
    case UNIFYVALUEG: ONREG(unifyValue,GREF);
    unifyValue:
    {
        *sPointer++ = RegAccess(auxReg,getRegArg(PC+1));
        DISPATCH(2,-1);
      }

    case UNIFYVALVARXX: ONREG2(UnifyValVar,X,X);
    case UNIFYVALVARXY: ONREG2(UnifyValVar,X,Y);
    case UNIFYVALVARXG: ONREG2(UnifyValVar,X,GREF);
    case UNIFYVALVARYX: ONREG2(UnifyValVar,Y,X);
    case UNIFYVALVARYY: ONREG2(UnifyValVar,Y,Y);
    case UNIFYVALVARYG: ONREG2(UnifyValVar,Y,GREF);
    case UNIFYVALVARGX: ONREG2(UnifyValVar,GREF,X);
    case UNIFYVALVARGY: ONREG2(UnifyValVar,GREF,Y);
    case UNIFYVALVARGG: ONREG2(UnifyValVar,GREF,GREF);
      {
      UnifyValVar:
        *sPointer++ = RegAccess(auxReg,getRegArg(PC+1));
        *sPointer++ = am.currentUVarPrototype();
        RegAccess(auxReg1,getRegArg(PC+2)) = makeTaggedRef(sPointer);
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

    default:
      displayCode(PC,1);
      displayDef(PC,1);
      OZ_error("buildRecord: unhandled opcode: %d\n",op);
      goto exit;
    }
  }
 exit:
  {
    for (int i = 0; i < maxX; i++)
      X[i] = savedX[i];
  }
  {
    for (int i = 0; i < maxY; i++)
      Y[i] = savedY[i];
  }
#endif
}


// outlined:
void pushContX(TaskStack *stk,
               ProgramCounter pc,RefsArray y,Abstraction *cap,
               RefsArray x)
{
  stk->pushCont(pc,y,cap);
  stk->pushX(x,cap->getPred()->getMaxX());
}


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

#ifdef OUTLINE
#undef inline
#endif
