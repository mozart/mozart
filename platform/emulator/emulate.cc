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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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

inline
Abstraction *getApplyMethod(ObjectClass *cl, ApplMethInfoClass *ami,
                            SRecordArity arity, RefsArray X)
{
  Assert(oz_isFeature(ami->methName));
  return ami->methCache.lookup(cl,ami->methName,arity,X);
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
  OZ_putArg(e,1,OZ_atom(fun));
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

// mm: this macro is optimized such that the term T is only created
//  when needed, so don't pass it as argument to functions.
#define HF_RAISE_FAILURE(T)                             \
   if (e->hf_raise_failure())                           \
     goto LBLfailure;                                   \
   if (ozconf.errorDebug) e->exception.info  = (T);     \
   RAISE_THREAD;


#define HF_FAIL       HF_RAISE_FAILURE(OZ_atom("fail"))
#define HF_DIS        HF_RAISE_FAILURE(OZ_atom("fail"))
#define HF_COND       HF_RAISE_FAILURE(OZ_atom("fail"))
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

// -----------------------------------------------------------------------
// *** ???
// -----------------------------------------------------------------------

#define NOFLATGUARD   (shallowCP)

#define IMPOSSIBLE(INSTR) OZ_error("%s: impossible instruction",INSTR)

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
void bindOPT(OZ_Term *varPtr, OZ_Term term, ByteCode *scp)
{
  Assert(isUVar(*varPtr));
  if (!am.currentUVarPrototypeEq(*varPtr) || scp!=0) {
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
OZ_Return fastUnify(OZ_Term A, OZ_Term B, ByteCode *scp=0)
{
  if (scp) goto fallback;

  {
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
  }

fallback:
  return oz_unify(A,B,scp);

 exit:
  COUNT(varOptUnify);
  return PROCEED;
}

/*
 * new builtins support
 */

OZ_Return oz_bi_wrapper(Builtin *bi,OZ_Term *X)
{
  Assert(am.isEmptySuspendVarList());
  Assert(am.isEmptyPreparedCalls());

  const int inAr = bi->getInArity();
  const int outAr = bi->getOutArity();

  OZ_Term savedX[outAr];
  for (int i=outAr; i--; ) savedX[i]=X[inAr+i];

  OZ_Return ret1 = bi->getFun()(X,OZ_ID_MAP);
  if (ret1!=PROCEED) {
    switch (ret1) {
    case FAILED:
    case RAISE:
    case BI_TYPE_ERROR:
    case SUSPEND:
      // restore X
      for (int j=outAr; j--; ) {
        X[inAr+j]=savedX[j];
      }
      return ret1;
    case PROCEED:
    case BI_PREEMPT:
    case BI_REPLACEBICALL:
      break;
    default:
      OZ_error("oz_bi_wrapper: return not handled: %d",ret1);
      return FAILED;
    }
  }
  for (int i=outAr;i--;) {
    OZ_Return ret2 = fastUnify(X[inAr+i],savedX[i]);
    if (ret2!=PROCEED) {
      switch (ret2) {
      case FAILED:
      case RAISE:
      case BI_TYPE_ERROR:
        // restore X in case of error
        for (int j=outAr; j--; ) {
          X[inAr+j]=savedX[j];
        }
        return ret2;
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
  if (bi==bi_raise||bi==bi_raiseError||bi==bi_raiseDebug) return;

  int iarity = bi->getInArity();
  int oarity = bi->getOutArity();

  OZ_Term tt=OZ_tupleC("apply",iarity+oarity+1);

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
  for (int i=loc->getOutArity(); i--; ) {
    out=oz_cons(oz_newVariable(),out);
  }
  for (int i=loc->getInArity(); i--; ) {
    out=oz_cons(X[loc->in(i)],out);
  }
  return out;
}

// -----------------------------------------------------------------------
// *** genCallInfo: self modifying code!
// -----------------------------------------------------------------------

static
Bool genCallInfo(GenCallInfoClass *gci, TaggedRef pred, ProgramCounter PC,
                 TaggedRef *X)
{
  Assert(!oz_isRef(pred));

  Abstraction *abstr = NULL;
  if (gci->isMethAppl) {
    if (!oz_isClass(pred)) goto insertMethApply;

    Bool defaultsUsed;
    abstr = tagged2ObjectClass(pred)->getMethod(gci->mn,gci->arity,
                                                0,defaultsUsed);
    /* fill cache and try again later */
    if (abstr==NULL) return NO;
    if (defaultsUsed) goto insertMethApply;
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


insertMethApply:
  {
    CodeArea *code = CodeArea::findBlock(PC);
    ApplMethInfoClass *ami = new ApplMethInfoClass(gci->mn,gci->arity,code);
    CodeArea::writeOpcode(gci->isTailCall ? TAILAPPLMETHG : APPLMETHG, PC);
    CodeArea::writeAddress(ami, PC+1);
    CodeArea::writeRegIndex(gci->regIndex, PC+2);
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

// failure in shallow guard can never be handled
#define RAISE_THREAD_NO_PC                      \
  e->exception.pc=NOCODE;                       \
  e->setShallowHeapTop(0);                      \
  return T_RAISE;

#define RAISE_THREAD                            \
  e->exception.pc=PC;                           \
  e->exception.y=Y;                             \
  e->exception.cap=CAP;                         \
  e->setShallowHeapTop(0);                      \
  return T_RAISE;


Bool oz_emulateHookOutline()
{
  // without signal blocking;
  if (am.isSetSFlag(ThreadSwitch)) {
    if (am.threadsPool.threadQueuesAreEmpty()) {
      am.restartThread();
    } else {
      return TRUE;
    }
  }
  if (am.isSetSFlag((StatusBit)(StartGC|UserAlarm|IOReady|TasksReady))) {
    return TRUE;
  }

  return FALSE;
}

/* macros are faster ! */
#define emulateHookCall(e,Code)                 \
   if (hookCheckNeeded()) {                     \
     if (oz_emulateHookOutline()) {             \
       Code;                                    \
       return T_PREEMPT;                        \
     }                                          \
   }

#define emulateHookPopTask(e) emulateHookCall(e,)


#define ChangeSelf(obj)                         \
      e->changeSelf(obj);

#define PushCont(_PC)  CTS->pushCont(_PC,Y,CAP);
#define PushContX(_PC) pushContX(CTS,_PC,Y,CAP,X);

// outlined:
void pushContX(TaskStack *stk,
               ProgramCounter pc,RefsArray y,Abstraction *cap,
               RefsArray x)
{
  stk->pushCont(pc,y,cap);
  stk->pushX(x,cap->getPred()->getMaxX());
}


/* NOTE:
 * in case we have call(x-N) and we have to switch process or do GC
 * we have to save as cont address Pred->getPC() and NOT PC
 */
#define CallDoChecks(Pred)                                                  \
     Y = NULL;                                                              \
     CAP = Pred;                                                            \
     emulateHookCall(e,PushContX(Pred->getPC()));

// load a continuation into the machine registers PC,Y,CAP,X
#define LOADCONT(cont)                          \
  {                                             \
      Continuation *tmpCont = cont;             \
      PC = tmpCont->getPC();                    \
      Y = tmpCont->getY();                      \
      CAP = tmpCont->getCAP();                  \
      predArity = tmpCont->getXSize();          \
      tmpCont->getX(X);                         \
  }

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
#else
#define DISPATCH(INC) {                         \
  INCFPC(INC);                                  \
  goto* (void*) ((*PC)|textBase);               \
}
#endif

#else /* THREADED */

#define Case(INSTR)   case INSTR :  asmLbl(INSTR);
#define DISPATCH(INC) INCFPC(INC); goto LBLdispatcher

#endif

#define JUMPRELATIVE(offset) Assert(offset!=0); DISPATCH(offset)
#define JUMPABSOLUTE(absaddr) PC=absaddr; DISPATCH(0)

#define ONREG(Label,R)      HelpReg = (R); goto Label
#define ONREG2(Label,R1,R2) HelpReg1 = (R1); HelpReg2 = (R2); goto Label


#ifdef FASTREGACCESS
#define RegAccess(Reg,Index) (*(RefsArray)((intlong) Reg + Index))
#define LessIndex(Index,CodedIndex) \
                       (Index <= (int)(CodedIndex/sizeof(TaggedRef)))
#else
#define RegAccess(Reg,Index) (Reg[Index])
#define LessIndex(I1,I2) (I1<=I2)
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

#ifdef MIPS
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

#define MAGIC_RET                                               \
{                                                               \
   if (tmpRet == SUSPEND) return T_SUSPEND;                     \
   if (tmpRet == PROCEED) goto LBLpopTask;                      \
   Assert(tmpRet == RAISE || tmpRet == BI_REPLACEBICALL);       \
   goto LBLhandleRet;                                           \
}

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
  AWActor *CAA                 = NULL;
  DebugCheckT(Board *currentDebugBoard=CBB);

  // handling perdio unification
  ProgramCounter lastGetRecord;                     NoReg(lastGetRecord);

  RefsArray HelpReg1 = NULL, HelpReg2 = NULL;
  #define HelpReg sPointer  /* more efficient */

  /* shallow choice pointer */
  ByteCode *shallowCP = NULL;

  ConstTerm *predicate;     NoReg(predicate);
  int predArity;            NoReg(predArity);

  // optimized arithmetic and special cases for unification
  OZ_Return tmpRet;  NoReg(tmpRet);

  TaggedRef auxTaggedA, auxTaggedB;
  int auxInt;
  char *auxString;

#ifdef THREADED
# include "instrtab.hh"
  if (init) {
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
// *** leaving emulation mode returning to scheduler
// ------------------------------------------------------------------------

LBLfailure: // mm2: eventually merge with raise?
  {
    // check for 'pseudo flat guard'
    Actor *aa=CBB->getActor();
    if (aa->isAskWait() && CTT == AWActor::Cast(aa)->getThread()) {
      Assert(CAA==aa);
      oz_failBoard();
      DebugCheckT(currentDebugBoard=CBB);
      goto LBLcheckFlat2;
    }

    return T_FAILURE;
  }

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
                goto LBLfailure;
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

#include "register.hh"

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
          JUMPRELATIVE(getLabelArg(PC+3));
        }
      }

      switch (ret) {
      case RAISE:
        if (e->exception.debug) set_exception_info_call(bi,X,loc->mapping());
        RAISE_THREAD;
      case BI_TYPE_ERROR: RAISE_TYPE_NEW(bi,loc);

      case SUSPEND:
        PushContX(PC);
        SUSPENDONVARLIST;

      case BI_PREEMPT:
      case BI_REPLACEBICALL:
      case SLEEP:
      case FAILED:
      default:
        Assert(0);
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
          if (shallowCP) {
            e->trail.pushIfVar(auxTaggedA);
            e->trail.pushIfVar(auxTaggedB);
            goto LBLsuspendShallow;
          }
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
            if (shallowCP) {
              e->trail.pushIfVar(A);
              goto LBLsuspendShallow;
            }
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
      (void) oz_raise(E_ERROR,E_OBJECT,"@",2,makeTaggedSRecord(rec),fea);
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
                      makeTaggedSRecord(rec), fea, XPC(2));
      RAISE_THREAD;
    }

  Case(INLINEUPARROW)
    {
      switch(uparrowInlineBlocking(XPC(1),XPC(2),XPC(3))) {
      case PROCEED:
        DISPATCH(4);

      case SUSPEND:
          Assert(!shallowCP);
          OZ_suspendOnInternal2(XPC(1),XPC(2));
          CheckLiveness(PC);
          PushContX(PC);
          SUSPENDONVARLIST;

      case FAILED:
        HF_APPLY(OZ_atom("^"),
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
// INSTRUCTIONS: Shallow guards stuff
// ------------------------------------------------------------------------

  Case(SHALLOWGUARD)
    {
      shallowCP = PC;
      e->setShallowHeapTop(heapTop);
      e->trail.pushMark();
      DISPATCH(2);
    }

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

  Case(SHALLOWTHEN)
    {
      if (e->trail.isEmptyChunk()) {
        shallowCP = NULL;
        e->setShallowHeapTop(NULL);
        e->trail.popMark();
        DISPATCH(1);
      }

    LBLsuspendShallow:
      {
        e->emptySuspendVarList(); // mm2: done twice
        CheckLiveness(shallowCP);
        PushContX(shallowCP);
        shallowCP = NULL;
        e->setShallowHeapTop(NULL);
        oz_reduceTrailOnShallow();
        return T_SUSPEND;
      }
    }

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

  Case(FAILURE)
    {
      HF_FAIL;
    }


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
    RAISE_THREAD;
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

  switch(t->getTertType()){
  case Te_Frame:{
    if(((LockFrameEmul *)t)->hasLock(th)) {goto has_lock;}
    if(((LockFrameEmul *)t)->lockB(oz_currentThread())){goto got_lock;}
    goto no_lock;}
  case Te_Proxy:{
    (*lockLockProxy)(t, th);
    goto no_lock;}
  case Te_Manager:{
    if(((LockManagerEmul *)t)->hasLock(th)) {goto has_lock;}
    if(((LockManagerEmul *)t)->lockB(th)){goto got_lock;}
    goto no_lock;}
  default:
    Assert(0);}

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
    goto LBLreplaceBICall;
  }

  Case(RETURN)

    LBLpopTask:
      {
        emulateHookPopTask(e);

        Assert(!CTT->isSuspended());
        Assert(CBB==currentDebugBoard);
        DebugCheckT(CAA = NULL);

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
                               oz_nil(), // mm2: inherit native?
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


  Case(TAILAPPLMETHX) isTailCall = OK; ONREG(ApplyMethod,X);
  Case(TAILAPPLMETHY) isTailCall = OK; ONREG(ApplyMethod,Y);
  Case(TAILAPPLMETHG) isTailCall = OK; ONREG(ApplyMethod,GREF);

  Case(APPLMETHX) isTailCall = NO; ONREG(ApplyMethod,X);
  Case(APPLMETHY) isTailCall = NO; ONREG(ApplyMethod,Y);
  Case(APPLMETHG) isTailCall = NO; ONREG(ApplyMethod,GREF);

 ApplyMethod:
  {
    ApplMethInfoClass *ami = (ApplMethInfoClass*) getAdressArg(PC+1);
    SRecordArity arity     = ami->arity;
    TaggedRef origCls   = RegAccess(HelpReg,getRegArg(PC+2));
    TaggedRef cls       = origCls;
    Abstraction *def       = NULL;

    if (!isTailCall) PC = PC+3;

    DEREF(cls,clsPtr,clsTag);
    if (!oz_isClass(cls)) {
      oz_raise(E_ERROR,E_KERNEL,"type",5,
               oz_atom(","),
               oz_cons(origCls,oz_cons(makeMessage(arity,ami->methName,X),oz_nil())),
               oz_atom("class"),
               oz_int(1),
               oz_atom(""));
      RAISE_THREAD;
    }
    def = getApplyMethod(tagged2ObjectClass(cls),ami,arity,X);
    if (def==NULL) {
      goto bombApply;
    }

    if (!isTailCall) { PushCont(PC); }
    COUNT(applmeth);
    CallDoChecks(def);
    JUMPABSOLUTE(def->getPC());


  bombApply:
    Assert(tagged2ObjectClass(cls)->getFallbackApply());

    X[1] = makeMessage(arity,ami->methName,X);
    X[0] = origCls;

    predArity = 2;
    predicate = tagged2Const(tagged2ObjectClass(cls)->getFallbackApply());
    goto LBLcall;
  }


  Case(CALLX) isTailCall = NO; ONREG(Call,X);
  Case(CALLY) isTailCall = NO; ONREG(Call,Y);
  Case(CALLG) isTailCall = NO; ONREG(Call,GREF);

  Case(TAILCALLX) isTailCall = OK; ONREG(Call,X);
  Case(TAILCALLY) isTailCall = OK; ONREG(Call,Y);
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
        goto LBLreplaceBICall;

       default: Assert(0);
       }
     }
     Assert(0);

   LBLhandleRet:
     switch (tmpRet) {
     case RAISE: RAISE_THREAD;
     case BI_REPLACEBICALL: PC=NOCODE; goto LBLreplaceBICall;
     default: break;
     }
     Assert(0);
     error("impossible");

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
// INSTRUCTIONS: Actors/Deep Guards
// -------------------------------------------------------------------------

  Case(WAIT)
    {
      CBB->setWaiting();
      CBB->decSuspCount();
      goto LBLsuspendBoard;
    }

  Case(WAITTOP)
    {
      CBB->decSuspCount();

      if ( oz_entailment() ) { // OPT commit()
        e->trail.popMark();
        Board *tmpBB = CBB;
        e->setCurrent(CBB->getParent());
        DebugCheckT(currentDebugBoard=CBB);
        tmpBB->unsetInstalled();
        oz_merge(tmpBB,CBB,-1);
        CTS->discardActor();
        WaitActor::Cast(CAA)->disposeWait();
        CAA = NULL;
        DISPATCH(1);
      }
      CBB->setWaiting();
      CBB->setWaitTop();
      goto LBLsuspendBoard;
    }

  Case(ASK)
    {
      CBB->decSuspCount();

      // entailment ?
      if (oz_entailment()) { // OPT commit()
        e->trail.popMark();
        Board *tmpBB = CBB;
        e->setCurrent(CBB->getParent());
        DebugCheckT(currentDebugBoard=CBB);
        tmpBB->unsetInstalled();
        oz_merge(tmpBB,CBB,-1);
        CTS->discardActor();
        AskActor::Cast(CAA)->disposeAsk();
        CAA = NULL;
        DISPATCH(1);
      }

    LBLsuspendBoard:
      CBB->setBody(PC+1, Y, CAP,NULL,0);
      Assert(CAA == AWActor::Cast(CBB->getActor()));

      oz_deinstallCurrent();
      DebugCode(currentDebugBoard=CBB);

    LBLcheckFlat2:
      Assert(!CAA->isCommitted());
      Assert(CAA->getThread()==CTT);

      if (CAA->hasNext()) {
        LOADCONT(CAA->getNext());
        goto LBLemulate; // no thread switch allowed here (CAA)
      }

      if (CAA->isWait()) {
        WaitActor *wa = WaitActor::Cast(CAA);
        /* test bottom commit */
        if (wa->hasNoChildren()) {
          HF_DIS;
        }

        /* test unit commit */
        if (wa->hasOneChildNoChoice()) {
          Board *waitBoard = wa->getLastChild();
          if (!oz_commit(waitBoard,CTT)) {
            HF_DIS;
          }
          goto LBLpopTask;
        }

        // suspend wait actor
        return T_SUSPEND_ACTOR;
      }

      Assert(CAA->isAsk());
      {
        AskActor *aa = AskActor::Cast(CAA);

        //  should we activate the 'else' clause?
        if (aa->isLeaf()) { // OPT commit()
          CTS->discardActor();
          aa->setCommittedActor();
          CBB->decSuspCount();

          LOADCONT(aa->getNext());
          PC = aa->getElsePC();

          aa->disposeAsk();

          goto LBLemulate;
        }

        return T_SUSPEND_ACTOR;
      }
    }

  Case(CREATECOND)
    {
      ProgramCounter elsePC = PC+getLabelArg(PC+1);

      CAA = new AskActor(CBB,CTT,
                         elsePC ? elsePC : NOCODE,
                         NOCODE, Y, CAP, X, CAP->getPred()->getMaxX());
      CTS->pushActor(CAA,PC);
      CBB->incSuspCount();
      DISPATCH(2);
    }

  Case(CREATEOR)
    {
      CAA = new WaitActor(CBB, CTT, NOCODE, Y, CAP, NO);
      CTS->pushActor(CAA,PC);
      CBB->incSuspCount();

      DISPATCH(1);
    }

  Case(CREATEENUMOR)
    {
      Board *bb = CBB;

      CAA = new WaitActor(bb, CTT, NOCODE, Y, CAP, NO);
      CTS->pushActor(CAA,PC);
      CBB->incSuspCount();

      if (bb->isWait()) {
        WaitActor::Cast(bb->getActor())->addChoice((WaitActor *) CAA);
      } else if (bb->isSolve()) {
        SolveActor::Cast(bb->getActor())->addChoice((WaitActor *) CAA);
      }

      DISPATCH(1);
    }

  Case(CREATECHOICE)
    {
      Board *bb = CBB;

      CAA = new WaitActor(bb, CTT, NOCODE, Y, CAP, OK);
      CTS->pushActor(CAA,PC);
      CBB->incSuspCount();

      Assert(CAA->isChoice());

      if (bb->isWait()) {
        WaitActor::Cast(bb->getActor())->addChoice((WaitActor *) CAA);
      } else if (bb->isSolve()) {
        SolveActor::Cast(bb->getActor())->addChoice((WaitActor *) CAA);
      }

      DISPATCH(1);
    }

  Case(CLAUSE)
    {
      Board *bb;
      if (CAA->isAsk()) {
        bb = new Board(CAA,Bo_Ask);
        AskActor::Cast(CAA)->addAskChild();
      } else {
        bb = new Board(CAA,Bo_Wait);
        WaitActor::Cast(CAA)->addWaitChild(bb);
      }
      e->setCurrent(bb,OK);
      CBB->incSuspCount();
      DebugCode(currentDebugBoard=CBB);
      CBB->setInstalled();
      e->trail.pushMark();
      Assert(CAA->getThread()==CTT);
      DISPATCH(1);
    }

  // == CLAUSE, WAIT
  Case(EMPTYCLAUSE)
    {
      Assert(CAA->isWait());
      Board *bb = new Board(CAA, Bo_Wait | Bo_Waiting);
      WaitActor::Cast(CAA)->addWaitChild(bb);

      bb->setBody(PC+1, Y, CAP, NULL,0);

      goto LBLcheckFlat2;
    }

  Case(NEXTCLAUSE)
      CAA->nextClause(PC+getLabelArg(PC+1));
      DISPATCH(2);

  Case(LASTCLAUSE)
      CAA->lastClause();
      DISPATCH(1);

  Case(THREAD)
    {
      ProgramCounter newPC = PC+2;
      int contPC = getLabelArg(PC+1);

      int prio = CPP;

      if (prio > DEFAULT_PRIORITY) {
        prio = DEFAULT_PRIORITY;
      }

      Thread *tt = oz_newThread(prio);

      COUNT(numThreads);
      RefsArray newY = Y==NULL ? (RefsArray) NULL : copyRefsArray(Y);

      tt->getTaskStackRef()->pushCont(newPC,newY,CAP);
      tt->setSelf(e->getSelf());
      tt->setAbstr(ozstat.currAbstr);

      JUMPRELATIVE(contPC);
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

  Case(TASKACTOR)
    {
      // this is the second part of Space.choose (see builtin.cc)
      WaitActor *wa = WaitActor::Cast((Actor *) Y);
      CTS->restoreFrame();
      if (wa->getChildCount() != 1) {
        return T_SUSPEND;
      }
      Board *bb = wa->getChildRef();
      Assert(bb->isWait());

      if (!oz_commit(bb,CTT)) {
        goto LBLfailure; // ???
      }
      goto LBLpopTask;
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
       //
       // by kost@ : 'solve actors' are represented via a c-function;
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

  Case(TASKLPQ)
     {
#ifdef DEBUG_THREADCOUNT
       extern int existingLTQs;
       if (existingLTQs <= 0)
         Assert(0);
#endif
       Y = NULL;  // sa here unused
       //       Assert(e->currentBoard()->isSolve());
       //Assert(!oz_onToplevel());
       Assert(CTS->isEmpty()); // approximates one LTQ task

       // postpone poping task from taskstack until
       // local thread queue is empty
       Board * sb = e->currentBoard();
       LocalPropagatorQueue * lpq = sb->getLocalPropagatorQueue();

#ifdef DEBUG_LTQ
       cout << "sb=" << sb << " emu " << " thr="
            << oz_currentThread() << endl << flush;
#endif

       if (lpq == NULL) {
         goto LBLpopTask;
       }

       {
         unsigned int starttime = 0;

         if (ozconf.timeDetailed)
           starttime = osUserTime();

         Thread * backup_currentThread = CTT;

         while (!lpq->isEmpty() && isNotPreemptiveScheduling()) {
           Propagator * prop = lpq->dequeue();
           Propagator::setRunningPropagator(prop);
           Assert(!prop->isDeadPropagator());

           OZ_Return r = oz_runPropagator(prop);

           if (r == SLEEP) {
             oz_sleepPropagator(prop);
           } else if (r == PROCEED) {
             oz_closeDonePropagator(prop);
           } else if (r == FAILED) {

             if (ozconf.timeDetailed)
               ozstat.timeForPropagation.incf(osUserTime()-starttime);

             // check for top-level and if not, prepare raising of an
             // exception (`hf_raise_failure()')
             if (e->hf_raise_failure()) {
               oz_closeDonePropagator(prop);
               goto LBLfailure;
             }

             if (ozconf.errorDebug)
               e->setExceptionInfo(OZ_mkTupleC("apply",2,
                                               OZ_atom((prop->getPropagator()->getProfile()->getPropagatorName())),
                                               prop->getPropagator()->getParameters()));

             oz_closeDonePropagator(prop);

             oz_resetLocalPropagatorQueue(sb);

             RAISE_THREAD;
           } else {
             Assert(r == SCHEDULED);
             oz_preemptedPropagator(prop);
           }
           Assert(prop->isDeadPropagator() || !prop->isRunnable());
         }

         if (ozconf.timeDetailed)
           ozstat.timeForPropagation.incf(osUserTime()-starttime);

       }

       if (lpq->isEmpty()) {
         oz_resetLocalPropagatorQueue(sb);
#ifdef DEBUG_LTQ
         cout << "sb emu sb=" << sb << " EMPTY" << endl << flush;
#endif
         goto LBLpopTask;
       } else {
#ifdef DEBUG_LTQ
         cout << "sb emu sb=" << sb << " PREEMPTIVE" << endl << flush;
#endif
         CTS->pushLPQ(sb);
         Assert(sb->getLocalPropagatorQueue());
         return T_PREEMPT;
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
        Assert(dbg->Y == Y && dbg->CAP == CAP);

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
                  goto LBLfailure;
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

      if (genCallInfo(gci,pred,PC,X)) {
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
    if (shallowCP) {
      if (e->trail.isEmptyChunk()) {
        e->trail.popMark();
      } else {
        oz_reduceTrailOnFail();
      }
      PC=shallowCP;
      shallowCP=0;
      e->setShallowHeapTop(NULL);
    }

    // mm2: must also handle pseudo shallow guards ala 'or X=1 [] ... end',
    //  e.g. when X is a future.
    Actor *aa=CBB->getActor();
    if (aa && aa->isAskWait() && CTT == AWActor::Cast(aa)->getThread()) {
      OZ_warning("unifySpecial in pseudo shallow guard not impl. Failing.");
      switch (tmpRet) {
      case BI_REPLACEBICALL:
        e->emptyPreparedCalls();
        // fall through
      case SUSPEND:
        e->emptySuspendVarList();
        // fall through
      case RAISE:
        oz_failBoard();
        DebugCheckT(currentDebugBoard=CBB);
        goto LBLcheckFlat2;
      default:
        Assert(0);
      }
    }

    switch (tmpRet) {
    case BI_REPLACEBICALL:
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

// ------------------------------------------------------------------------
// *** FAILURE
// ------------------------------------------------------------------------

LBLshallowFail:
  {
    asmLbl(SHALLOW_FAIL);
    if (e->trail.isEmptyChunk()) {
      e->trail.popMark();
    } else {
      oz_reduceTrailOnFail();
    }
    PC                 = shallowCP;
    shallowCP          = NULL;
    e->setShallowHeapTop(NULL);
    JUMPRELATIVE(getLabelArg(PC+1));
  }
} // end engine


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
