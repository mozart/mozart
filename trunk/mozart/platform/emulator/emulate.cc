/* This may look like C code, but it is really -*- C++ -*-

  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow,mehl,scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  The main engine
  ------------------------------------------------------------------------
*/

#ifdef INTERFACE
#pragma implementation "emulate.hh"
#endif

#include "am.hh"

#include "indexing.hh"

#include "genvar.hh"
#include "fdhook.hh"


#define KOSTJA_MAGIC 0x6b6f7374


/*
 * Inline caching
 */

class RecordCache {
  Arity *ar;
  int index;

public:
  int lookup(SRecord *rec, TaggedRef feature)
  {
    if (ar!=rec->getArity()) {
      ar = rec->getArity();
      index = rec->getIndex(feature); // is ok even if index==-1 !
    }
    return index;
  }
};

int xCalled = 0;
int xMiss = 0;

class MethodCache {
  ObjectClass *cl;
  Abstraction *abstr;

public:
  Abstraction *lookup(Object *obj, TaggedRef meth, SRecordArity arity)
  {
    ObjectClass *cla = obj->getClass();
    if (cla!=cl) {
      cl    = cla;
      abstr = obj->getMethod(meth,arity);  /* is ok even if we find no method */
    }
    return abstr;
  }
};


/*
 * Object stuff
 */

extern TaggedRef methApplHdl;

#define StateLocked ((Abstraction*) -1l)

inline
Abstraction *getSendMethod(Object *obj, TaggedRef label, SRecordArity arity, 
			   MethodCache *cache)
{
  Assert(isFeature(label));

  if (obj->isClosedOrClassOrDeepOrLocked()) {
    /* send to object in guard */
    if (obj->getDeepness()!=0 || obj->isClosed() || obj->isClass() ||
	am.currentBoard != obj->getBoardFast())
      return NULL;
  }

  Assert(obj->getDeepness()==0 && !obj->isClosed() && !obj->isClass());

  return cache->lookup(obj,label,arity);
}

inline
Abstraction *getApplyMethod(Object *obj, TaggedRef label, SRecordArity arity)
{
  Assert(isFeature(label));
  return obj->getMethod(label,arity);
}

// -----------------------------------------------------------------------
// TOPLEVEL FAILURE (HF = Handle Failure)

/*
 * make an STuple
 * INPUT: X register, number of arguments
 */
OZ_Term mkSTupleX(char *label,OZ_Term fun, RefsArray A,int n)
{
  OZ_Term tmp=OZ_tupleC(label,n+1);
  OZ_putArg(tmp,0,fun);
  for (int i=0; i < n; i++) OZ_putArg(tmp,i+1,A[i]);
  return tmp;
}

OZ_Term mkSTupleX(char *label,char *fun, RefsArray A,int n)
{
  return mkSTupleX(label,OZ_atom(fun),A,n);
}

OZ_Term adjoinT(TaggedRef tuple,TaggedRef arg)
{
  if (!isSTuple(tuple)) {
    Assert(OZ_isAtom(tuple));
    OZ_Term tmp=OZ_tuple(tuple,1);
    OZ_putArg(tmp,0,arg);
    return tmp;
  } else {
    SRecord *st=tagged2SRecord(tuple);
    int len=st->getWidth();
    OZ_Term tmp=OZ_tuple(st->getLabel(),len+1);
    OZ_putArg(tmp,0,arg);
    for (int i=0; i < len; i++) OZ_putArg(tmp,i+1,st->getArg(i));
    return tmp;
  }
}


#define DORAISE(T) { X[0] = (T); goto LBLraise; }


#define HF_FAIL(R)							\
   if (!e->isToplevel()) { goto LBLfailure; }			\
   DORAISE(OZ_mkTupleC("toplevelFailure",1,(R)));		


#define INFO_BI					\
 mkSTupleX("proc",				\
	  builtinTab.getName((void *) biFun), 	\
	  X,predArity)

#define RAISE_BI(val) { DORAISE(adjoinT(am.exception,val)); }

#define NOFLATGUARD   (shallowCP==NULL)

#define SHALLOWFAIL if (!NOFLATGUARD) { goto LBLshallowFail; }

#define CheckArity(arityExp,proc)					   \
if (predArity != arityExp && VarArity != arityExp) {			   \
  DORAISE(OZ_mkTupleC("arityCheck",1,mkSTupleX("proc",proc,X,predArity))); \
}




#ifdef DEBUG_CHECK
#define IMPOSSIBLE(INSTR) error("%s: impossible instruction",INSTR)
#else
#define IMPOSSIBLE(INSTR) 
#endif






// TOPLEVEL END
// -----------------------------------------------------------------------


#define DoSwitchOnTerm(indexTerm,table)					      \
      TaggedRef term = indexTerm;					      \
      DEREF(term,_1,_2);						      \
									      \
      if (!isLTuple(term)) {						      \
	TaggedRef *sp = sPointer;					      \
	ProgramCounter offset = switchOnTermOutline(term,table,sp);	      \
	sPointer = sp;							      \
	JUMP(offset);							      \
      }									      \
									      \
      ProgramCounter offset = table->listLabel;				      \
      sPointer = tagged2LTuple(term)->getRef();				      \
      JUMP(offset);							      \



static
ProgramCounter switchOnTermOutline(TaggedRef term, IHashTable *table,
				   TaggedRef *&sP)
{
  ProgramCounter offset = table->getElse();
  if (isSRecord(term)) {
    if (table->functorTable) {
      SRecord *rec = tagged2SRecord(term);
      Literal *lname = rec->getLabelLiteral();
      Assert(lname!=NULL);
      int hsh = table->hash(lname->hash());
      offset = table->functorTable[hsh]->lookup(lname,rec->getSRecordArity(),offset);
      sP = rec->getRef();
    }
    return offset;
  }

  if (isLiteral(term)) {
    if (table->literalTable) {
      int hsh = table->hash(tagged2Literal(term)->hash());
      offset = table->literalTable[hsh]->lookup(tagged2Literal(term),offset);
    }
    return offset;
  }

  if (isNotCVar(term)) {
    return table->varLabel;
  }

  if (isSmallInt(term)) {
    if (table->numberTable) {
      int hsh = table->hash(smallIntHash(term));
      offset = table->numberTable[hsh]->lookup(term,offset);
    }
    return offset;
  }

  if (isFloat(term)) {
    if (table->numberTable) {
      int hsh = table->hash(tagged2Float(term)->hash());
      offset = table->numberTable[hsh]->lookup(term,offset);
    }
    return offset;
  }

  if (isBigInt(term)) {
    if (table->numberTable) {
      int hsh = table->hash(tagged2BigInt(term)->hash());
      offset =table->numberTable[hsh]->lookup(term,offset);
    }
    return offset;
  }

  if (isCVar(term) && !table->disentailed(tagged2CVar(term))) {
    return table->varLabel;
  }

  return offset;
}

#ifdef OUTLINE
#define inline
#endif

// -----------------------------------------------------------------------
// genCallInfo: self modifying code!

static
void genCallInfo(GenCallInfoClass *gci, TaggedRef pred, ProgramCounter PC)
{
  static int gencall = 0;
  static int gencallmethfailed = 0;
  static int gencallfailed = 0;
  gencall++;
  
  Assert(!isRef(pred));

  Abstraction *abstr = NULL;
  if (gci->isMethAppl) {
    if (!isObject(pred) ||
	NULL == (abstr = getApplyMethod((Object *) tagged2Const(pred),
					gci->lit,gci->arity))) {
      ApplMethInfoClass *ami = new ApplMethInfoClass(gci->lit,gci->arity);
      CodeArea::writeOpcode(gci->isTailCall ? TAILAPPLMETHG : APPLMETHG, PC);
      CodeArea::writeAddress(ami, PC+1);
      CodeArea::writeRegIndex(gci->regIndex, PC+2);
      gencallmethfailed++;
      return;
    }
  } else {
    if(!isAbstraction(pred)) goto bombGenCall;

    abstr = tagged2Abstraction(pred);
    if (abstr->getArity() != getWidth(gci->arity)) 
      goto bombGenCall;
  }
  
  {
    /* ok abstr points to an abstraction */
    AbstractionEntry *entry = AbstractionTable::add(abstr);
    CodeArea::writeAddress(entry, PC+1);
    CodeArea::writeOpcode(gci->isTailCall ? FASTTAILCALL : FASTCALL, PC);
    return;
  }
  
bombGenCall:
  gencallfailed++;
  CodeArea::writeRegIndex(gci->regIndex,PC+1);
  CodeArea::writeArity(getWidth(gci->arity), PC+2);
  CodeArea::writeOpcode(gci->isTailCall ? TAILCALLG : CALLG,PC);
  return;
}

// -----------------------------------------------------------------------
// CALL HOOK


/* the hook functions return:
     TRUE: must reschedule
     FALSE: can continue
   */

Bool AM::emulateHookOutline(Abstraction *def, int arity, TaggedRef *arguments)
{
  // without signal blocking;
  if (isSetSFlag(ThreadSwitch)) {
    if (threadQueueIsEmpty()
	|| getNextThPri () < currentThread->getPriority()) {
      restartThread();
    } else {
      return TRUE;
    }
  }
  if (isSetSFlag((StatusBit)(StartGC|UserAlarm|IOReady))) {
    return TRUE;
  }
  
  if (def && isSetSFlag(DebugMode)) {
    enterCall(currentBoard,makeTaggedConst(def),arity,arguments);
  }

  return FALSE;
}

inline
Bool AM::hookCheckNeeded()
{
#ifdef DEBUG_DET
  static int counter = 100;
  if (--counter < 0) {
    handleAlarm();   // simulate an alarm
    counter = 100;
  }
#endif
  
  return (isSetSFlag());
}


/* macros are faster ! */
#define emulateHookCall(e,def,arity,arguments,Code) 		\
    if (e->hookCheckNeeded()) {					\
      if (e->emulateHookOutline(def, arity, arguments)) {	\
	Code;							\
      }								\
    }

#define emulateHookPopTask(e,Code) emulateHookCall(e,0,0,0,Code)


#define CallPushCont(ContAdr) e->pushTaskInline(ContAdr,Y,G,NULL,0)

#define SaveSelf(e,obj,pushOntoStack)		\
  {						\
    Object *auxo = e->getSelf();		\
    if (auxo!=obj) {				\
      if (pushOntoStack)			\
	e->currentThread->pushSelf(auxo);	\
      else					\
	e->currentThread->setSelf(auxo);	\
      e->setSelf(obj);				\
    }						\
  }


/* NOTE:
 * in case we have call(x-N) and we have to switch process or do GC
 * we have to save as cont address Pred->getPC() and NOT PC
 */
#define CallDoChecks(Pred,gRegs,Arity)					\
     Y = NULL;								\
     G = gRegs;								\
     emulateHookCall(e,Pred,Arity,X,					\
		     e->pushTask(Pred->getPC(),NULL,G,X,Arity);	\
		     goto LBLpreemption;);


// load a continuation into the machine registers PC,Y,G,X
#define LOADCONT(cont)				\
  {						\
      Continuation *tmpCont = cont;		\
      PC = tmpCont->getPC();			\
      Y = tmpCont->getY();			\
      G = tmpCont->getG();			\
      predArity = tmpCont->getXSize();		\
      tmpCont->getX(X);				\
  }

// -----------------------------------------------------------------------
// THREADED CODE

#if defined(RECINSTRFETCH) && defined(THREADED)
 Error: RECINSTRFETCH requires THREADED == 0;
#endif

#define INCFPC(N) PC += N

//#define WANT_INSTRPROFILE
#if defined(WANT_INSTRPROFILE) && defined(__GNUC__)
#define asmLbl(INSTR) asm(" " #INSTR ":");
#else
#define asmLbl(INSTR)
#endif


/* threaded code broken on linux, leads to memory leek,
 * this is a workaround
 * seems to work under 2.7.x
 */

#ifdef THREADED
#define Case(INSTR) INSTR##LBL : asmLbl(INSTR); 

#ifdef XXLINUX
#define DISPATCH(INC) INCFPC(INC); goto LBLdispatcher
#else

// let gcc fill in the delay slot of the "jmp" instruction:
#define DISPATCH(INC) {							      \
  intlong aux = *(PC+INC);						      \
  INCFPC(INC);								      \
  goto* (void*) (aux|textBase);					   	      \
}
#endif /* LINUX */

#else /* THREADED */

#define Case(INSTR)   case INSTR :
#define DISPATCH(INC) INCFPC(INC); goto LBLdispatcher

#endif

#define JUMP(absAdr) Assert(absAdr!=0); PC=absAdr; DISPATCH(0)

#define ONREG(Label,R)      HelpReg = (R); goto Label
#define ONREG2(Label,R1,R2) HelpReg1 = (R1); HelpReg2 = (R2); goto Label


#ifdef FASTREGACCESS
#define RegAccess(Reg,Index) (*(RefsArray)((intlong) Reg + Index))
#define LessIndex(Index,CodedIndex) (Index <= CodedIndex/sizeof(TaggedRef))
#else
#define RegAccess(Reg,Index) (Reg[Index])
#define LessIndex(I1,I2) (I1<=I2)
#endif

#define Xreg(N) RegAccess(X,N)
#define Yreg(N) RegAccess(Y,N)
#define Greg(N) RegAccess(G,N)

#define XPC(N) Xreg(getRegArg(PC+N))

 
/* define REGOPT if you want the into register optimization for GCC */
#if defined(REGOPT) &&__GNUC__ >= 2 && (defined(LINUX_I486) || defined(MIPS) || defined(OSF1_ALPHA) || defined(SPARC)) && !defined(DEBUG_CHECK)
#define Into(Reg) asm(#Reg)

#ifdef LINUX_I486
/* This does NOT pay off */
/*   #define Reg1 asm("%esi") */
#define Reg1 
#define Reg2
#define Reg3
#define Reg4
#define Reg5
#define Reg6
#define Reg7
#endif

#ifdef SPARC
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

#define SetReadMode
#define SetWriteMode (sPointer = (TaggedRef *)((long)sPointer+1));

#define InWriteMode (((long)sPointer)&1)

#define GetSPointerWrite(ptr) (TaggedRef*)(((long)ptr)-1)

// ------------------------------------------------------------------------
// outlined auxiliary functions
// ------------------------------------------------------------------------

#define CHECK_CURRENT_THREAD			\
    if (e->currentThread->isSuspended()) {	\
      goto LBLsuspendThread;			\
    }						\
    goto LBLpopTask;


#define SUSP_PC(TermPtr,RegsToSave,PC)		\
   e->pushTask(PC,Y,G,X,RegsToSave);	\
   addSusp(TermPtr,e->mkSuspThread ());		\
   CHECK_CURRENT_THREAD;

void addSusp(TaggedRef var, Thread *thr)
{
  DEREF(var,varPtr,_1);
  Assert(isAnyVar(var));

  taggedBecomesSuspVar(varPtr)->addSuspension (thr);
}

void addSusp(TaggedRef *varPtr, Thread *thr)
{
  taggedBecomesSuspVar(varPtr)->addSuspension (thr);
}

/*
 * create the suspension for builtins returning SUSPEND
 *
 * PRE: no reference chains !!
 */
void AM::suspendOnVarList(Thread *thr)
{
  Assert(suspendVarList!=makeTaggedNULL());

  TaggedRef varList=suspendVarList;
  while (!isRef(varList)) {
    Assert(isCons(varList));
    
    addSusp(head(varList),thr);
    varList=tail(varList);
  }
  addSusp(varList,thr);
  suspendVarList=makeTaggedNULL();
}

// 
//  Not inlined, but 'Thread::getJob ()' is!
//  Note that 'Thread::getJob ()' yields a *suspended* thread,
// so the current thread is marked as suspended when 'LBLsuspendThread'
// is entered;
Thread *AM::mkSuspThread ()
{
  /* save special registers */
  SaveSelf(this,NULL,OK);
  return currentThread->getJob();
}

void AM::suspendCond(AskActor *aa)
{
  // Thread *th = mkSuspThread(); /* 'self' is saved directly in thread! */
  Thread *th = currentThread->getJob();

  Assert (th->isSuspended ());
  //  we put in either the same thread, or a part of it;
  Assert (currentThread == aa->getThread ());
  aa->setThread (th);
}

void AM::suspendInline(int n, OZ_Term A,OZ_Term B,OZ_Term C)
{
  Thread *thr = mkSuspThread();
  switch(n) { /* no break's used!! */
  case 3: { DEREF (C, ptr, _1); if (isAnyVar(C)) addSusp(ptr, thr); }
  case 2: { DEREF (B, ptr, _1); if (isAnyVar(B)) addSusp(ptr, thr); }
  case 1: { DEREF (A, ptr, _1); if (isAnyVar(A)) addSusp(ptr, thr); }
    break;
  default:
    Assert(0);
  }
    
  suspendVarList = makeTaggedNULL();   // mm2 please check
}


static
TaggedRef makeMessage(SRecordArity srecArity, TaggedRef label, TaggedRef *X)
{
  int width = getWidth(srecArity);
  if (width == 0) {
    return label;
  }

  if (width == 2 && literalEq(label,AtomCons))
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

TaggedRef AM::createNamedVariable(int regIndex, TaggedRef name)
{
  Assert(isLiteral(name));
  int size = getRefsArraySize(toplevelVars);
  if (LessIndex(size,regIndex)) {
    int newSize = int(size*1.5);
    message("resizing store for toplevel vars from %d to %d\n",size,newSize);
    toplevelVars = resize(toplevelVars,newSize);
    // no deletion of old array --> GC does it
  }
  SVariable *svar = new SVariable(currentBoard);
  TaggedRef ret = makeTaggedRef(newTaggedSVar(svar));
  VariableNamer::addName(ret,tagged2Literal(name)->getPrintName());
  return ret;
}


static
void printArg(OZ_Term arg)
{
  arg=deref(arg);
  if (isSTuple(arg)) {
    SRecord *st=tagged2SRecord(arg);
    int len=st->getWidth();
    TaggedRef lab=st->getLabel();
    if (lab==OZ_atom("proc")) {
      message("Application { %s ",toC(st->getArg(0)));
      for (int i=1; i<len; i++) {
	printf("%s ",toC(st->getArg(i)));
      }
      printf("} failed.\n");
    } else if (lab==OZ_atom("instr")) {
      message("Instruction %s(",toC(st->getArg(0)));
      for (int i=1; i<len; i++) {
	printf("%s ",toC(st->getArg(i)));
      }
      printf(") failed.\n");
    } else if (lab==OZ_atom("vs")) {
      for (int i=0; i<len; i++) {
	message("");
	OZ_printVirtualString(st->getArg(i));
	printf("\n");
      }
    } else if (lab==OZ_atom("pos")) {
      message("For Argument #%s.\n",toC(st->getArg(0)));
    } else if (lab==OZ_atom("type")) {
      message("Expected type %s.\n",toC(st->getArg(0)));
    } else {
      message("%s.\n",toC(arg));
    }
  } else {
    if (arg != OZ_atom("noInfo")) {
      message("%s.\n",toC(arg));
    }
  }
}


void AM::defaultExceptionHandler(OZ_Term val, ProgramCounter PC,
				 TaskStackEntry *oldTos)
{
  if (ozconf.errorVerbosity > 0) {
    errorHeader();
    if (OZ_isVariable(val) || !OZ_isRecord(val)) {
      message("Exception '%s' caught.\n",toC(val));
    } else {
      OZ_Term lab=OZ_label(val);
      if (literalEq(lab,OZ_atom("noElse"))) {
	message("Conditional without else failed.\n");
      } else if (literalEq(lab,OZ_atom("toplevelThreadBlocked"))) {
	message("The toplevel is blocked.\n");
      } else if (ozconf.errorVerbosity > 1) {
	val=deref(val);
	if (isSTuple(val)) {
	  SRecord *st=tagged2SRecord(val);
	  int len=st->getWidth();
	  for (int i=0; i<len; i++) {
	    printArg(st->getArg(i));
	  }
	}
      }
    }
    if (currentThread && ozconf.errorVerbosity > 1) {
      TaskStackEntry *saveTos = currentThread->getTop();
      currentThread->setTop(oldTos);
      currentThread->printTaskStack(PC);
      currentThread->setTop(saveTos);
    }
    errorTrailer();
  }
}

/*
 * Entailment handling for emulator
 *
 * check entailment and stability
 *  after thread is finished or top commit
 */
enum CE_RET {
  CE_CONT,
  CE_SOLVE_CONT,
  CE_NOTHING,
  CE_FAIL,
  CE_SUSPEND
};

int AM::checkEntailment(Continuation *&contAfter, Actor *&aa)
{

loop:
  DebugTrace(trace("check entailment",currentBoard));

  currentBoard->unsetNervous();

  if (currentBoard->isAsk()) {
    if ( entailment() ) {
      aa = currentBoard->getActor();
      contAfter=currentBoard->getBodyPtr();

      trail.popMark();

      Board *tmpBB = currentBoard;

      setCurrent(currentBoard->getParentFast());
      tmpBB->unsetInstalled();
      tmpBB->setCommitted(currentBoard);
      currentBoard->decSuspCount();

      return CE_CONT;
    }
    return CE_NOTHING;
  }

  if (currentBoard->isWait()) {
// WAITTTOP
    if (currentBoard->isWaitTop()) {

// WAITTOP: top commit
      if (entailment()) {
	trail.popMark();

	Board *tmpBB = currentBoard;

	setCurrent(currentBoard->getParentFast());
	tmpBB->unsetInstalled();
	tmpBB->setCommitted(currentBoard);
	currentBoard->decSuspCount();

	goto loop;
      }

      Assert(!WaitActor::Cast(currentBoard->getActor())->hasOneChild());

      return CE_NOTHING;
    }

    Assert(!currentBoard->isWaiting () ||
	   !WaitActor::Cast(currentBoard->getActor())->hasOneChild());
    return CE_NOTHING;

    // WAIT: no rule
  }

  if (currentBoard->isSolve()) {
    // try to reduce a solve board;
    SolveActor *solveAA = SolveActor::Cast(currentBoard->getActor());
    Board      *solveBB = currentBoard; 
 
    if (isStableSolve(solveAA)) {
      DebugCheck ((trail.isEmptyChunk() == NO),
		  error ("non-empty trail chunk for solve board"));
      // all possible reduction steps require this; 

      if (solveBB->hasSuspension() == NO) {
	// 'solved';
	// don't unlink the subtree from the computation tree;
	trail.popMark();
	currentBoard->unsetInstalled();
	setCurrent(currentBoard->getParentFast());
	// don't decrement counter of parent board!

	// statistic
	ozstat.incSolveSolved();
	if (!fastUnifyOutline(solveAA->getResult(), 
			      solveAA->genSolved(), OK)) {
	  return CE_FAIL;
	}
	return CE_NOTHING;
      } else {
 
	// wake up propagators to be woken up on stablity
	if (solveAA->stable_wake()) {
	  LOCAL_PROPAGATION(if (! localPropStore.do_propagation())
			    return CE_FAIL;);
	  if (!isStableSolve(solveAA)) {
	    deinstallCurrent();
	    return CE_NOTHING;
	  }
	  goto loop;
	}
 
	WaitActor *wa = solveAA->topChoice();
	
	if (wa == NULL) {
	  // "stuck" (stable without distributing waitActors);
	  // don't unlink the subtree from the computation tree; 
	  trail.popMark();
	  currentBoard->unsetInstalled();
	  setCurrent (currentBoard->getParentFast());

	  // don't decrement counter of parent board!

	  if ( !fastUnifyOutline(solveAA->getResult(), 
				 solveAA->genStuck(), OK) ) {
	    return CE_FAIL;
	  }
	  return CE_NOTHING;

	} else {
	  // to enumerate;

	  if (wa->getChildCount()==1 && wa->isChoice()) {
	    solveAA->popChoice();
	    Board *waitBoard = wa->getChildRef();

	    waitBoard->setCommitted(solveBB);
	    Assert(!solveBB->isCommitted());
	    solveBB->incSuspCount(waitBoard->getSuspCount()-1);

	    if (!installScript(waitBoard->getScriptRef())) {
	      return CE_FAIL;
	    }

	    if (waitBoard->isWaitTop()) {
	      goto loop;
	    }

	    aa        = wa;
	    contAfter = waitBoard->getBodyPtr();
	    return CE_SOLVE_CONT;
	  }

	  // give back number of clauses
	  trail.popMark();
	  currentBoard->unsetInstalled();
	  setCurrent(currentBoard->getParentFast());
	  
	  // don't decrement counter of parent board!

	  if (!fastUnifyOutline(solveAA->getResult(),
				solveAA->genChoice(wa->getChildCount()),
				OK)) {
	    return CE_FAIL;
	  }
	  return CE_NOTHING;
	}
      }
    } else if (solveAA->isDebug() && solveAA->getThreads() == 0) {
      // There are some external suspensions to this solver!

      deinstallCurrent();

      TaggedRef newVar = makeTaggedRef(newTaggedUVar(currentBoard));
      TaggedRef result = solveAA->getResult();

      solveAA->setResult(newVar);

      if ( !fastUnifyOutline(result,
			     solveAA->genUnstable(newVar),
			     OK)) {
	return CE_FAIL;
      }
      return CE_NOTHING;
    } 
    deinstallCurrent();
    return CE_NOTHING;
  } 

  Assert(currentBoard->isRoot());
  return CE_NOTHING;
}

// aux debugging;
#define VERBMSG(S,A1,A2)                                                   \
    fprintf(verbOut,"(em) %s (arg#1 0x%x, arg#2 0x%x) :%d\n",              \
	    S,A1,A2,__LINE__);                                             \
    fflush(verbOut);


/* &Var prevent Var to be allocated to a register,
 * increases chances that variables declared as "register"
 * will be really allocated to registers
 */

#define NoReg(Var) { void *p = &Var; }


void engine() 
{  
// ------------------------------------------------------------------------
// *** Global Variables
// ------------------------------------------------------------------------
  /* ordered by iomportance: first variables will go into machine registers
   * if -DREGOPT is set
   */
  register ProgramCounter PC   Reg1 = 0;
  register RefsArray X         Reg2 = am.xRegs;
  register RefsArray Y         Reg3 = NULL;
  register TaggedRef *sPointer Reg4 = NULL;
  register AM *e               Reg5 = &am;
  register RefsArray G         Reg6 = NULL;

  Bool isTailCall              = NO;                NoReg(isTailCall);
  AWActor *CAA                 = NULL;
  Board *tmpBB                 = NULL;              NoReg(tmpBB);
  DebugCheckT(Board *currentDebugBoard=0);

# define CBB (e->currentBoard)
# define CPP (e->currentThread->getPriority())

  RefsArray HelpReg1 = NULL, HelpReg2 = NULL;
  #define HelpReg sPointer  /* more efficient */

  /* shallow choice pointer */
  ByteCode *shallowCP = NULL;

  OZ_CFun biFun = NULL;     NoReg(biFun);
  ConstTerm *predicate;	    NoReg(predicate);
  int predArity;    	    NoReg(predArity);

#ifdef CATCH_SEGV
  //
  //  In the case of an error, the control is passed either to 
  // LBLkillToplevelThread or LBLfailure; Note that both require
  // the presence of a runnable thread;
  switch (e->catchError()) {
  case 0:
    //  the environment is setted;
    break;

  case SEGVIO:
    DORAISE(OZ_atom("segv"));
  case BUSERROR:
    DORAISE(OZ_atom("bus"));
  }
#endif
  

#ifdef THREADED
# include "instrtab.hh"
  CodeArea::globalInstrTable = instrTable;
# define op (Opcode) -1
#else
  Opcode op = (Opcode) -1;
#endif

  goto LBLstart;

// ------------------------------------------------------------------------
// *** RUN: Main Loop
// ------------------------------------------------------------------------
 LBLpreemption:

  SaveSelf(e,NULL,NO);
  e->currentThread->setBoard (CBB);
  e->scheduleThread(e->currentThread);
  e->currentThread=(Thread *) NULL;

 LBLerror:
 LBLstart:

  Assert (e->currentThread == (Thread *) NULL);

  if (e->isSetSFlag()) {  

    e->deinstallPath(e->rootBoard);

    if (e->isSetSFlag(StartGC)) {
      e->doGC();
    }


    if (e->isSetSFlag(UserAlarm)) {
      osBlockSignals();
      e->handleUser();
      osUnblockSignals();
    }
    if (e->isSetSFlag(IOReady)) {
      osBlockSignals();
      e->handleIO();
      osUnblockSignals();
    }
  }

  /* process switch */
  if (e->threadQueueIsEmpty()) {
    e->suspendEngine();
  }

  e->currentThread = e->getFirstThread ();
  DebugTrace (trace("new thread"));
  //  now, we have *globally* am.currentThread;

  //
  //  No dead threads here;
  // (while, of course, their *tasks* may be dead already;)
  //  So, every runnable thread must be terminated through 
  // 'LBL{discard,kill}Thread', and it should not appear 
  // more than once in the threads pool;
  Assert (!(e->currentThread->isDeadThread ()));
  //
  //  So, every thread in the threads pool *must* be runnable;
  Assert (e->currentThread->isRunnable ());

  /*
   *  Lazy stack allocation;
   *
   *  Note that we cover the case for 'propagator' threads specially, 
   *  since they don't differ in 'suspended' and 'runnable' states;
   */
  if (e->currentThread->isNewPropagator ()) {
    //
    //  First, get the home board of that propagator, 
    // and try to install it;
    tmpBB = e->currentThread->getBoardFast ();
    DebugTrace (trace ("propagator thread", tmpBB));

    // 
    //  HERE: a special version of INSTALLPATH for propagators;
    //  This is because we have to go for the 'LBLdiscardThread'
    // if the installation is rejected;
    if (CBB != tmpBB) {
      switch (e->installPath (tmpBB)) {
      case INST_OK:
	break;

      case INST_REJECTED:
	//
	//  Note that once a propagator is failed, its thread is 
	// scheduled for execution again (!), and at this place it 
	// should be catched and forwarded to the 
	// 'LBLdiscardThread';
	DebugCode (e->currentThread->removeNewPropagator ());
	goto LBLdiscardThread;

      case INST_FAILED:
	//
	//  The thread must be killed;
	//  So, first go to the 'LBLfailure', and there it must be 
	// scheduled once again - and then the 'INST_REJECTED' case
	// hits ...
	DebugCode (e->currentThread->removeNewPropagator ());
	goto LBLfailure;
      }
    }

    CBB->unsetNervous();

    LOCAL_PROPAGATION(Assert(localPropStore.isEmpty ()););

    switch (e->currentThread->runNewPropagator()) {
    case SLEEP: 
      e->currentThread->suspendPropagator ();
      if (e->currentSolveBoard != (Board *) NULL) {
	e->decSolveThreads (e->currentSolveBoard);
	//  but it's still "in solve";
      }
      e->currentThread = (Thread *) NULL;
      LOCAL_PROPAGATION(if (!(localPropStore.do_propagation ()))
			 goto localhack7;);
      goto LBLstart;

    case SCHEDULED:
      e->currentThread->scheduledPropagator ();
      if (e->currentSolveBoard != (Board *) NULL) {
	e->decSolveThreads (e->currentSolveBoard);
	//  but it's still "in solve";
      }
      e->currentThread = (Thread *) NULL;
      LOCAL_PROPAGATION(if (!(localPropStore.do_propagation ()))
			 goto localhack7;);
      goto LBLstart;

    case PROCEED:
      // Note: e->currentThread must be reset in 'LBLkillXXX';
      LOCAL_PROPAGATION(if (!(localPropStore.do_propagation ()))
			 goto localhack7;);
      if (e->isToplevel ()) {
	goto LBLkillToplevelThread;
      } else {
	goto LBLkillThread;
      }

      //  Note that *propagators* never yield 'SUSPEND';
    case FAILED:
      LOCAL_PROPAGATION(localPropStore.reset());

    localhack7:
      HF_FAIL(INFO_BI);

    default:
      error ("Unexpected value returned from a propagator.");
      goto LBLerror;
    }
  } else if (e->currentThread->isPropagator ()) {
    CFuncContinuation *ccont;

    //
    //  First, get the home board of that propagator, 
    // and try to install it;
    tmpBB = e->currentThread->getBoardFast ();
    DebugTrace (trace ("propagator thread", tmpBB));

    // 
    //  HERE: a special version of INSTALLPATH for propagators;
    //  This is because we have to go for the 'LBLdiscardThread'
    // if the installation is rejected;
    if (CBB != tmpBB) {
      switch (e->installPath (tmpBB)) {
      case INST_OK:
	break;

      case INST_REJECTED:
	//
	//  Note that once a propagator is failed, its thread is 
	// scheduled for execution again (!), and at this place it 
	// should be catched and forwarded to the 
	// 'LBLdiscardThread';
	DebugCode (e->currentThread->removePropagator ());
	goto LBLdiscardThread;

      case INST_FAILED:
	//
	//  The thread must be killed;
	//  So, first go to the 'LBLfailure', and there it must be 
	// scheduled once again - and then the 'INST_REJECTED' case
	// hits ...
	DebugCode (e->currentThread->removePropagator ());
	goto LBLfailure;
      }
    }

    ccont = e->currentThread->getCCont ();
    biFun = ccont->getCFunc ();
    RefsArray tmpX = ccont->getX ();
    Assert (tmpX != (RefsArray) NULL);
    predArity = getRefsArraySize (tmpX);

    CBB->unsetNervous();

    LOCAL_PROPAGATION(Assert(localPropStore.isEmpty ()););

    Assert (biFun);
    switch (biFun (predArity, tmpX)) {
    case SLEEP: 
      e->currentThread->suspendPropagator ();
      if (e->currentSolveBoard != (Board *) NULL) {
	e->decSolveThreads (e->currentSolveBoard);
	//  but it's still "in solve";
      }
      e->currentThread = (Thread *) NULL;
      LOCAL_PROPAGATION(if (!(localPropStore.do_propagation ()))
			 goto localhack0;);
      goto LBLstart;

    case PROCEED:
      // Note: e->currentThread must be reset in 'LBLkillXXX';
      LOCAL_PROPAGATION(if (!(localPropStore.do_propagation ()))
			 goto localhack0;);
      if (e->isToplevel ()) {
	goto LBLkillToplevelThread;
      } else {
	goto LBLkillThread;
      }

      //  Note that *propagators* never yield 'SUSPEND';
    case FAILED:
      LOCAL_PROPAGATION(localPropStore.reset());

    localhack0:
      HF_FAIL(INFO_BI);

    default:
      error ("Unexpected value returned from a propagator.");
      goto LBLerror;
    }
  } else { 
    /*
     * install board
     */
    Board *bb=e->currentThread->getBoardFast();

    if (CBB != bb) {
    LBLinstallLoop:
      switch (e->installPath(bb)) {
      case INST_OK:
	break;

      case INST_REJECTED:
	if (!(e->currentThread->hasStack ()) || 
	    !e->currentThread->discardLocalTasks ()) {
	  goto LBLdiscardThread;
	}
	bb=bb->getParentFast();
	goto LBLinstallLoop;

      case INST_FAILED:
	if (e->currentThread->hasStack ()) {
	  while (bb != CBB) {
	    if (!e->currentThread->discardLocalTasks ()) {
	      break;
	    }
	    bb=bb->getParentFast();
	  }
	}
	DebugCheckT (e->currentThread->setBoard ((Board *) NULL));
	goto LBLfailure;
      }
    }

    CBB->unsetNervous();
    DebugCheckT(currentDebugBoard=CBB);

    //  I.e. it's not a propagator - then just convert it 
    // to a full-fledged thread (with a task stack);
    e->currentThread->makeRunning ();

    //
    e->restartThread();

    DebugCheckT (e->currentThread->setBoard ((Board *) NULL));
  }

  //  INVARIANT:
  //  current thread always has a stack, and it might not 
  // be marked as dead;
  Assert(e->currentThread->hasStack ());

// ------------------------------------------------------------------------
// *** pop a task
// ------------------------------------------------------------------------
LBLpopTask:
  {
    Assert(!e->currentThread->isSuspended());
    Assert(CBB==currentDebugBoard);
    asmLbl(popTask);

    emulateHookPopTask(e,
		       goto LBLpreemption);

    DebugCheckT(CAA = NULL);

    TaskStack *taskstack = e->currentThread->getTaskStackRef ();
    TaskStackEntry *topCache = taskstack->getTop();
    TaskStackEntry topElem;
    ContFlag cFlag;

  next_task:
    topElem = TaskStackPop (topCache-1);
    cFlag   = getContFlag (ToInt32 (topElem));

    /* RS: Optimize most probable case:
     *  - do not handle C_CONT in switch --> faster
     *  - assume cFlag == C_CONT implies stack does not contain empty mark
     *  - topCache maintained more efficiently
     */
    if (cFlag == C_CONT) {  
      Assert(!taskstack->isEmpty(topElem));
      PC = getPC(C_CONT,ToInt32(topElem));
      Y  = (RefsArray) TaskStackPop(topCache-2);
      G  = (RefsArray) TaskStackPop(topCache-3);
      taskstack->setTop(topCache-3);
      goto LBLemulate;
    }

    if (taskstack->isEmpty(topElem)) {
      taskstack->setTop(topCache);
      if (e->isToplevel ()) {
	goto LBLkillToplevelThread;
      } else {
	goto LBLkillThread;
      }
    }

    topCache--;
    switch (cFlag){
    case C_JOB:
      {
	taskstack->setTop(topCache);
	Bool hasJobs = TaskStack::getJobFlagFromEntry(topElem);
	if (!hasJobs){
	  e->currentThread->unsetHasJobs();
	}
	goto LBLpopTask;
      }

    case C_XCONT:
      PC = getPC(C_CONT,ToInt32(topElem));
      Y  = (RefsArray) TaskStackPop(--topCache);
      G  = (RefsArray) TaskStackPop(--topCache);
      {
	RefsArray tmpX = (RefsArray) TaskStackPop(--topCache);
	predArity = getRefsArraySize(tmpX);
	int i = predArity;
	while (--i >= 0) {
	  X[i] = tmpX[i];
	}
	disposeRefsArray(tmpX);
      }
      taskstack->setTop(topCache);
      goto LBLemulate;

    case C_DEBUG_CONT:
      {
	OzDebug *ozdeb = (OzDebug *) TaskStackPop(--topCache);
	exitCall(PROCEED,ozdeb);
	goto next_task;
      }

    /* unraised exception */
    case C_EXCEPT_HANDLER:
      topCache--;
      goto next_task;

    case C_CALL_CONT:
      {
	predicate = tagged2Const((TaggedRef)ToInt32(TaskStackPop(--topCache)));
	RefsArray tmpX = (RefsArray) TaskStackPop(--topCache);
	predArity = tmpX ? getRefsArraySize(tmpX) : 0;
	int i = predArity;
	while (--i >= 0) {
	  X[i] = tmpX[i];
	}
	disposeRefsArray(tmpX);
	taskstack->setTop(topCache);
	DebugTrace(trace("call cont task",CBB));
	isTailCall = OK;
	goto LBLcall;
      }

    case C_LOCAL:
      {
	error("C_LOCAL task detected");
      }

    case C_CFUNC_CONT:
      {
	Assert (!(e->currentThread->isPropagator ()));
	// 
	// by kost@ : 'solve actors' are represented via a c-function; 
        biFun = (OZ_CFun) TaskStackPop(--topCache);
	RefsArray tmpX = (RefsArray) TaskStackPop(--topCache);
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

	taskstack->setTop(topCache);

	DebugTrace(trace("cfunc cont task",CBB));

	LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()););
  
	switch (biFun(predArity, X)) {
	default:
	case FAILED:
	  LOCAL_PROPAGATION(localPropStore.reset());
	localhack1:
	  HF_FAIL(INFO_BI);

	case SLEEP:
	  error ("popTask: CFunCont returns 'SLEEP'!");

	case PROCEED:
	  LOCAL_PROPAGATION(if (! localPropStore.do_propagation())
			    goto localhack1;);
	  goto LBLpopTask;

	case SUSPEND:
	  {
	    LOCAL_PROPAGATION(if (! localPropStore.do_propagation())
			      goto localhack1;);
	    e->pushCFun(biFun,X,predArity);
	    Thread *thr = e->mkSuspThread ();
	    e->suspendOnVarList (thr);
	    CHECK_CURRENT_THREAD;
	  }

	case RAISE:
	  RAISE_BI(INFO_BI);
	} // switch
      }

    case C_SET_CAA:
      CAA = (AWActor *) TaskStackPop (--topCache);
      goto next_task;

    case C_SET_SELF:
      e->setSelf((Object *) TaskStackPop(--topCache));
      goto next_task;

    default:
      error("invalid task type");
      goto LBLerror;
    }  // switch
  }
//
// ----------------- end popTask -----------------------------------------

  /*
   *  Kill a thread at the toplevel - i.e. just dispose it;
   *
   */
LBLkillToplevelThread:
  {
    Assert (e->currentThread);
    Assert (e->isToplevel ());
    asmLbl(killToplevelThread);

    CBB->decSuspCount ();

    if (e->currentThread == e->rootThread) {
      // 
      //  A special case: the "root" thread;
      e->rootThread->reInit (e->rootThread->getPriority (), e->rootBoard);
      CBB->incSuspCount ();

      //  Anything else?
      e->checkToplevel ();

      if (e->rootThread->isEmpty ()) {
	//  still empty:
	e->currentThread = (Thread *) NULL;
	goto LBLstart;
      } else {
	//  ... no, we have fetched something - go ahead;
	goto LBLpreemption;
      }
    } else {
      e->currentThread->disposeRunnableThread ();
      e->currentThread = (Thread *) NULL;

      goto LBLstart;
    }
  }

  error ("never here");
  goto LBLerror;


  /*
   *  Kill the thread - decrement 'suspCounter'"s and counters of 
   * runnable threads in solve actors if any;
   *
   *  This label should be entered from the only place: when 
   * the task stack is detected to be empty in 'LBLpopTask';
   *
   *  Note: this code should ensure the presence of a (non-dead) 
   * runnable non-propagator thread with a stack when the control 
   * is passed to the 'LBLfailure';
   *
   *  Invariants: 
   *  - The thread must be yet alive;
   *  - It might be entered only if the task stack is empty
   *    OR it's a propagator;
   *  - It might not be entered in the toplevel;
   *  - 'CBB' must be alive (and not committed, of course);
   * 
   */
LBLkillThread:
  {
    Thread *tmpThread = e->currentThread;
    Board *nb = 0;
    Continuation *cont;
    Actor *aa;

    DebugTrace (trace ("kill thread", CBB));
    Assert (tmpThread);
    Assert (!(tmpThread->isDeadThread ()));
    Assert (tmpThread->isRunnable ());

    Assert (!(e->isToplevel ()));
    Assert (tmpThread->isPropagator () || tmpThread->isNewPropagator () || 
	    tmpThread->isEmpty ());
    //  Note that during debugging the thread does not carry 
    // the board pointer (== NULL) wenn it's running;
    // Assert (CBB == tmpThread->getBoardFast ());
    Assert (CBB != (Board *) NULL);
    Assert (!(CBB->isFailed ()));
    // 
    //  Note that if we are in a solve problem, then 
    // the thread should be marked correspondingly, and vice versa.
    //  In other words, we will test for the presence/absence 
    // of a 'currentSolveBoard' instead of 'isInSolve'!
    Assert (tmpThread->isInSolve () || !e->currentSolveBoard); 
    Assert (e->currentSolveBoard || !(tmpThread->isInSolve ()));
    asmLbl(killThread);

    e->currentThread = (Thread *) NULL;
    tmpThread->disposeRunnableThread ();

    CBB->decSuspCount ();

    /*
     *  General comment - about checking for stability:
     *  In the case when the thread was originated in a solve board, 
     * we have to update the (runnable) threads counter there manually, 
     * check stability there ('AM::checkEntailment ()'), and proceed 
     * with further solve actors upstairs by means of 
     * 'AM::decSolveThreads ()' as usually.
     *  This is because the 'AM::decSolveThreads ()' just generates 
     * wakeups for solve boards where stability is suspected. But 
     * finally the stability check for them should be performed, 
     * and this (and 'LBLsuspendThread') labels are exactly the 
     * right places where it should be done!
     *  Note also that the order of decrementing (runnable) threads 
     * counters in solve actors is also essential: if some solve actor 
     * can be reduced, solve actors above it are getting *instable*
     * because of a new thread!
     *
     *  This code has been originally written by mm2 with some 
     * assistance from me (i.e. kost@). After that, it was fixed 
     * and simplified (in many iterations :-)) as follows.
     *
     */ 

    // 
    //  First, look at the current board, and if it's a solve one, 
    // decrement the (runnable) threads counter manually _and_
    // skip the 'AM::decSolveThreads ()' for it; 
    if (e->currentSolveBoard) {
      if (CBB->isSolve ()) {
	SolveActor *sa;

	//
	sa = SolveActor::Cast (CBB->getActor ());
	//  'nb' points to some board above the current one,
	// so, 'decSolveThreads' will start there!
	nb = sa->getBoardFast ();

	//
	//  kost@ : optimize the most probable case!
	if (sa->decThreads () != 0) {
	  e->decSolveThreads (nb);
	  goto LBLstart;
	}
      } else {
	nb = CBB;
      }
    }

    // 
    //  ... and now, check the entailment here!
    //  Note again that 'decSolveThreads' should be done from 
    // the 'nb' board which is probably modified above!
    // 
    DebugCode (e->currentThread = (Thread *) KOSTJA_MAGIC);
    switch (e->checkEntailment (cont, aa)) {
    case CE_CONT:
      DebugCode (e->currentThread = (Thread *) NULL);
      {
	Thread *tt=0;

	if (aa->isAsk()) {
	  tt = AskActor::Cast(aa)->getThread ();
	  Assert (tt);

	  //
	  if (tt->isSuspended ()) {
	    Assert (tt->getBoardFast () == CBB);
	    tt->suspThreadToRunnable ();

	    //
	    tt->pushCont (cont);
	    e->scheduleThread (tt);
	  } else {
	    //  The following assertion holds because 
	    // there must be a continuation (possibly empty) 
	    // after each conditional;
	    Assert (tt != e->currentThread);
	    Assert (e->isScheduled (tt));

	    //
	    tt->cleanUp (CBB);
	    tt->pushCont (cont);
	  }
	} else {
	  //  any other actor;
	  tt = new Thread (aa->getPriority (), CBB);
	  tt->pushCont (cont);
	  e->scheduleThread (tt);
	}

	if (nb) e->decSolveThreads(nb->getBoardFast());
	Assert (tt->isInSolve () || !e->currentSolveBoard);
	Assert (!(tt->isInSolve ()) || e->currentSolveBoard);

	goto LBLstart;
      }

    case CE_NOTHING:
      DebugCode (e->currentThread = (Thread *) NULL);
      // 
      //  deref nb, because it maybe just committed!
      if (nb) e->decSolveThreads (nb->getBoardFast ());
      goto LBLstart;

    case CE_SOLVE_CONT:
      DebugCode (e->currentThread = (Thread *) NULL);
      {
	Assert(aa->isWait());
	Thread *tt = new Thread (aa->getPriority (), CBB);
	((WaitActor*) aa)->dispose();
	tt->pushCont(cont);
	e->scheduleThread (tt);

	if (nb) e->decSolveThreads(nb->getBoardFast());
	goto LBLstart;
      }

    case CE_FAIL:
      DebugCode (e->currentThread = (Thread *) NULL);
      if (nb) e->decSolveThreads (nb->getBoardFast ());
      //
      //  Note: there is no thread!
      HF_FAIL(OZ_atom("noInfo"));
    }

    error("never here");
    goto LBLerror;
  }


  /*
   *  Discard the thread, i.e. just decrement solve thread counters 
   * everywhere it is needed, and dispose the body; 
   *  The main difference to the 'LBLkillThread' is that no 
   * entailment can be reached here, because it's tested already 
   * when the failure was processed;
   * 
   *  Invariants:
   *  - a runnable thread must be there;
   *  - the task stack must be empty (for proper ones), or
   *    it must be already marked as not having the propagator
   *    (in dedug mode, for propagators);
   *  - the home board of the thread must be failed;
   *
   */
LBLdiscardThread:
  {
    Thread *tmpThread = e->currentThread;

    Assert (tmpThread);
    Assert (!(tmpThread->isDeadThread ()));
    Assert (tmpThread->isRunnable ());
    Assert (!tmpThread->isPropagator () || !tmpThread->isNewPropagator () || 
	    tmpThread->getPropagator () == (OZ_CFun) NULL);
    Assert (tmpThread->isPropagator () || tmpThread->isNewPropagator () || 
	    !(tmpThread->hasStack ()) || 
	    tmpThread->isEmpty ());
    // 
    //  Note: actually, 'Board::getBoardFast ()' yields the first 
    // non-committed board upstairs (i.e. it might be failed);
    //  This is non-complaint with the contemporary comment just
    // above its definition (in board.icc), and THIS FACT IS USED 
    // BY INSTALLING THE BOARD IN THE EMULATOR!
    //
    Assert (!((tmpThread->getBoardFast ())->checkAlive ()));
    asmLbl(discardThread);

    //
    //  Note that we may not use the 'currentSolveBoard' test here,
    // because it may point to an irrelevant board!
    if (tmpThread->isInSolve ()) {
      Board *tmpBB = tmpThread->getBoardFast ();

      if (tmpBB->isSolve ()) {
	//
	//  The same technique as by 'LBLkillThread';
	SolveActor *sa = SolveActor::Cast (tmpBB->getActor ());
	Assert (sa);
	Assert (sa->getSolveBoard () == tmpBB);

	e->decSolveThreads (sa->getBoardFast ());
      } else {
	e->decSolveThreads (tmpBB);
      }
    }
    e->currentThread = (Thread *) NULL;
    tmpThread->disposeRunnableThread ();

    goto LBLstart;
  }


  /*
   *  Suspend the thread in a generic way. It's used when something 
   * suspends in the sequential mode;
   *  Note that the thread stack should already contain the 
   * "suspended" task; 
   *
   *  Note that this code might be entered not only from within 
   * the toplevel, so, it has to handle everything correctly ...
   *
   *  Invariants:
   *  - there must be a thread marked as suspended;
   *  - CBB must be alive;
   *
   */
LBLsuspendThread:
  {
    Thread *tmpThread = e->currentThread;
    Board *nb = 0;
    Continuation *cont;
    Actor *aa;

    DebugTrace (trace("suspend runnable thread", CBB));
    Assert (tmpThread);
    Assert (tmpThread->isSuspended ());
    Assert (CBB);
    Assert (!(CBB->isFailed ()));
    //  see the note for the 'LBLkillThread';
    Assert (tmpThread->isInSolve () || !e->currentSolveBoard); 
    Assert (e->currentSolveBoard || !(tmpThread->isInSolve ()));

    asmLbl(suspendThread);

    //
    //  First, set the board and self, and perform special action for 
    // the case of blocking the root thread;
    e->currentThread->setBoard (CBB);
    SaveSelf(e,NULL,NO);
    if (e->currentThread == e->rootThread) {
      e->rootThread = 
	new Thread (e->currentThread->getPriority (), e->rootBoard);
      e->checkToplevel ();
      e->currentThread->markPropagated();
      DORAISE(OZ_atom("toplevelThreadBlocked"));
    }

    e->currentThread = (Thread *) NULL;

    //  No counter decrement 'cause the thread is still alive!

    if (CBB->isRoot ()) {
      //
      //  Note that in this case no (runnable) thread counters 
      // in solve actors can be affected, just because this is 
      // a top-level thread;
      goto LBLstart;
    }

    // 
    //  So, from now it's somewhere in a deep guard;
    Assert (!(e->isToplevel ()));

    if (e->currentSolveBoard) {
      if (CBB->isSolve ()) {
	SolveActor *sa;

	//
	sa = SolveActor::Cast (CBB->getActor ());
	//  'nb' points to some board above the current one,
	// so, 'decSolveThreads' will start there!
	nb = sa->getBoardFast ();

	//
	//  kost@ : optimize the most probable case!
	if (sa->decThreads () != 0) {
	  e->decSolveThreads (nb);
	  goto LBLstart;
	}
      } else {
	nb = CBB;
      }
    }

    // 
    DebugCode (e->currentThread = (Thread *) KOSTJA_MAGIC);
    switch (e->checkEntailment (cont, aa)) {
    case CE_NOTHING:
      DebugCode (e->currentThread = (Thread *) NULL);
      // 
      if (nb) e->decSolveThreads (nb->getBoardFast ());
      goto LBLstart;

    case CE_SOLVE_CONT:
      DebugCode (e->currentThread = (Thread *) NULL);
      {
	Assert(aa->isWait());
	Thread *tt = new Thread (aa->getPriority (), CBB);
	((WaitActor*) aa)->dispose();
	tt->pushCont(cont);
	e->scheduleThread (tt);

	if (nb) e->decSolveThreads(nb->getBoardFast());
	goto LBLstart;
      }

    case CE_FAIL:
      DebugCode (e->currentThread = (Thread *) NULL);
      if (nb) e->decSolveThreads (nb->getBoardFast ());
      //
      //  The thread does not exist any longer, because threads
      // can contain the local tasks *only*!
      tmpThread->disposeSuspendedThread (); 
      // ... and not 'disposeRunnableThread', because it is
      // marked already as non-propagated;
      //
      //  Note: there is no thread!
      HF_FAIL(OZ_atom("noInfo"));

    case CE_CONT:
      error ("Entailment of some guard during suspending a thread???");
      goto LBLerror;
    }

    error("never here");
    goto LBLerror;
  }

//
// ----------------- end killThread----------------------------------------
  
// ------------------------------------------------------------------------
// *** Emulate: execute continuation
// ------------------------------------------------------------------------
 LBLemulate:
  Assert(CBB==currentDebugBoard);

  JUMP( PC );

 LBLdispatcher:

#if defined(THREADED) && defined(XXLINUX)
  /* threaded code broken under linux */
  goto* (void*) (*PC);
#endif

#ifdef SLOW_DEBUG_CHECK
  /* These tests make the emulator really sloooooww */
  osBlockSignals(OK);
  DebugCheckT(osUnblockSignals());
  DebugCheck ((e->currentSolveBoard != CBB->getSolveBoard ()),
	      error ("am.currentSolveBoard and real solve board mismatch"));

  Assert(!isFreedRefsArray(Y));
#endif

#ifndef THREADED
  op = CodeArea::getOP(PC);
#endif

#ifdef RECINSTRFETCH
  CodeArea::recordInstr(PC);
#endif

  DebugTrace( if (!trace("emulate",CBB,CAA,PC,Y,G)) {
		goto LBLfailure;
	      });

#ifndef THREADED
  switch (op) {
#endif
    
// the instructions are classified into groups
// to find a certain class of instructions search for the String "CLASS:"
// -------------------------------------------------------------------------
// CLASS: TERM: MOVE/UNIFY/CREATEVAR/...
// -------------------------------------------------------------------------

#include "register.hh"

// ------------------------------------------------------------------------
// CLASS: (Fast-) Call/Execute Inline Funs/Rels
// ------------------------------------------------------------------------

  Case(FASTCALL)
    {
      CallPushCont(PC+3);
      goto LBLFastTailCall;
    }


  Case(FASTTAILCALL)
  LBLFastTailCall:
    {
      AbstractionEntry *entry = (AbstractionEntry *) getAdressArg(PC+1);

      CallDoChecks(entry->getAbstr(),entry->getGRegs(), entry->getAbstr()->getArity());

      IHashTable *table = entry->indexTable;
      if (table) {
	DoSwitchOnTerm(X[0],table);
      } else {
	JUMP(entry->getPC());
      }
    }

  Case(CALLBUILTIN)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      biFun = entry->getFun();
      Assert(biFun); // NOTE: special builtin need suspension handler
      predArity = getPosIntArg(PC+2);

      CheckArity(entry->getArity(),entry->getPrintName());

      LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()););

      switch (biFun(predArity, X)){
      case SUSPEND:
	e->pushTask(PC,Y,G,X,predArity);
	e->suspendOnVarList(e->mkSuspThread ());
	CHECK_CURRENT_THREAD;

      default:
      case FAILED:
	LOCAL_PROPAGATION(localPropStore.reset());
      localhack2:
	HF_FAIL(INFO_BI);

      case SLEEP: // no break
	error ("'CALLBUILTIN' has got 'SLEEP' back!\n");
      case PROCEED:
	LOCAL_PROPAGATION(if (! localPropStore.do_propagation())
			  goto localhack2;);
	DISPATCH(3);

      case RAISE:
	RAISE_BI(INFO_BI);
      }
    }


  Case(INLINEREL1)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel1 rel         = (InlineRel1)entry->getInlineFun();

  
      switch(rel(XPC(2))) {
      case PROCEED:
	DISPATCH(4);

      case SUSPEND:
	if (shallowCP) {
	  e->emptySuspendVarList();
	  e->trail.pushIfVar(XPC(2));
	  DISPATCH(4);
	}
	e->pushTask(PC,Y,G,X,getPosIntArg(PC+3));
	e->suspendInline(1,XPC(2));
	CHECK_CURRENT_THREAD;
      case FAILED:
	SHALLOWFAIL;
	HF_FAIL(OZ_mkTupleC("proc",2,
			OZ_atom(entry->getPrintName()),
			XPC(2)));

      case RAISE:
	RAISE_BI(OZ_mkTupleC("proc",2,
			 OZ_atom(entry->getPrintName()),
			 XPC(2)));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEREL2)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel2 rel         = (InlineRel2)entry->getInlineFun();

      switch(rel(XPC(2),XPC(3))) {
      case PROCEED:
	DISPATCH(5);

      case SUSPEND:
	{
	  if (shallowCP) {
	    e->emptySuspendVarList();
	    e->trail.pushIfVar(XPC(2));
	    e->trail.pushIfVar(XPC(3));
	    DISPATCH(5);
	  }

	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+4));
	  e->suspendInline(2,XPC(2),XPC(3));
	  CHECK_CURRENT_THREAD;
	}
      case FAILED:
	SHALLOWFAIL;
	HF_FAIL(OZ_mkTupleC("proc",3,
			OZ_atom(entry->getPrintName()),
			XPC(2),XPC(3)));

      case RAISE:
	RAISE_BI(OZ_mkTupleC("proc",3,
			 OZ_atom(entry->getPrintName()),
			 XPC(2),XPC(3)));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEREL3)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel3 rel         = (InlineRel3)entry->getInlineFun();

      switch(rel(XPC(2),XPC(3),XPC(4))) {
      case PROCEED:
	DISPATCH(6);

      case SUSPEND:
	{
	  if (shallowCP) {
	    e->emptySuspendVarList();
	    e->trail.pushIfVar(XPC(2));
	    e->trail.pushIfVar(XPC(3));
	    e->trail.pushIfVar(XPC(4));
	    DISPATCH(6);
	  }

	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+5));
	  e->suspendInline(3,XPC(2),XPC(3),XPC(4));
	  CHECK_CURRENT_THREAD;
	}
      case FAILED:
	SHALLOWFAIL;
	HF_FAIL(OZ_mkTupleC("proc",4,
			OZ_atom(entry->getPrintName()),
			XPC(2),XPC(3),XPC(4)));

      case RAISE:
	RAISE_BI(OZ_mkTupleC("proc",4,
			 OZ_atom(entry->getPrintName()),
			 XPC(2),XPC(3),XPC(4)));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEFUN1)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun1 fun         = (InlineFun1)entry->getInlineFun();

      // XPC(3) maybe the same register as XPC(2)
      switch(fun(XPC(2),XPC(3))) {
      case PROCEED:
	DISPATCH(5);

      case SUSPEND:
	{
	  TaggedRef A=XPC(2);
	  if (shallowCP) {
	    XPC(3) = makeTaggedRef(newTaggedUVar(CBB));
	    e->emptySuspendVarList();
	    e->trail.pushIfVar(A);
	    DISPATCH(5);
	  }
	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+4));
	  e->suspendInline(1,A);
	  CHECK_CURRENT_THREAD;
	}

      case FAILED:
	SHALLOWFAIL;
	HF_FAIL(OZ_mkTupleC("proc",3,
			OZ_atom(entry->getPrintName()),
			XPC(2),AtomVoid));

      case RAISE:
	RAISE_BI(OZ_mkTupleC("proc",3,
			 OZ_atom(entry->getPrintName()),
			 XPC(2),AtomVoid));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEFUN2)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun2 fun = (InlineFun2)entry->getInlineFun();

      
      // note: XPC(4) is maybe the same as XPC(2) or XPC(3) !!
      OZ_Return ret = fun(XPC(2),XPC(3),XPC(4));
      LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()););
      switch(ret) {
      case PROCEED:
	DISPATCH(6);

      case SUSPEND:
	{
	  TaggedRef A=XPC(2);
	  TaggedRef B=XPC(3);
	  if (shallowCP) {
	    XPC(4) = makeTaggedRef(newTaggedUVar(CBB));
	    e->emptySuspendVarList();
	    e->trail.pushIfVar(A);
	    e->trail.pushIfVar(B);
	    DISPATCH(6);
	  }
	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+5));
	  e->suspendInline(2,A,B);
	  CHECK_CURRENT_THREAD;
	}

      case FAILED:
	SHALLOWFAIL;
	HF_FAIL(OZ_mkTupleC("proc",4,
			OZ_atom(entry->getPrintName()),
			XPC(2),XPC(3),AtomVoid));

      case RAISE:
	RAISE_BI(OZ_mkTupleC("proc",4,
			 OZ_atom(entry->getPrintName()),
			 XPC(2),XPC(3),AtomVoid));

      case SLEEP:
      default:
	Assert(0);
      }
     }

  Case(INLINEDOT)
    {
      TaggedRef feature = getLiteralArg(PC+2);
      TaggedRef rec = XPC(1);
      DEREF(rec,_1,_2);
      if (isSRecord(rec)) {
	SRecord *srec = tagged2SRecord(rec);
	int index = ((RecordCache*)(PC+5))->lookup(srec,feature);
	if (index<0) {
	  goto dotFailed;
	}
	XPC(3) = srec->getArg(index);
	DISPATCH(7);	
      }
      {
	OZ_Return ret = dotInline(XPC(1),feature,XPC(3));
	LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()););
	switch(ret) {
	case PROCEED: DISPATCH(7);
	case FAILED:  goto dotFailed;

	case SUSPEND:
	  {
	    TaggedRef A=XPC(1);
	    if (shallowCP) {
	      XPC(3) = makeTaggedRef(newTaggedUVar(CBB));
	      e->emptySuspendVarList();
	      e->trail.pushIfVar(A);
	      DISPATCH(7);
	    }
	    e->pushTask(PC,Y,G,X,getPosIntArg(PC+4));
	    e->suspendInline(1,A);
	    CHECK_CURRENT_THREAD;
	  }

	case RAISE:
	  RAISE_BI(OZ_mkTupleC("proc",4,OZ_atom("`.`"),XPC(1),feature,AtomVoid));

	case SLEEP:
	default:
	  Assert(0);
	}
      }
    dotFailed:
      SHALLOWFAIL;
      DORAISE(OZ_mkTupleC("typeError",1,
		      OZ_mkTupleC("proc",4,
			      OZ_atom("`.`"),XPC(1),feature,AtomVoid)));
    }

  Case(INLINEAT)
    {
      TaggedRef fea = getLiteralArg(PC+1);

      Assert(e->getSelf()!=NULL);
      SRecord *rec = e->getSelf()->getState();
      if (rec) {
	int index = ((RecordCache*)(PC+4))->lookup(rec,fea);
	if (index>=0) {
	  XPC(2) = rec->getArg(index);
	  DISPATCH(6);
	}
      }
      DORAISE(OZ_mkTupleC("accessFailure",1,
			  OZ_mkTupleC("proc",4,
				      OZ_atom("`@`"),
				      rec?makeTaggedSRecord(rec):OZ_atom("noattributes"),
				      fea,
				      AtomVoid)));
    }

  Case(INLINEASSIGN)
    {      
      TaggedRef fea = getLiteralArg(PC+1);

      SRecord *rec = e->getSelf()->getState();
      if (rec) {
	int index = ((RecordCache*)(PC+4))->lookup(rec,fea);
	if (index>=0) {
	  Assert(isRef(*rec->getRef(index)) || !isAnyVar(*rec->getRef(index)));
	  rec->setArg(index,XPC(2));
	  DISPATCH(6);
	}
      }
      
      DORAISE(OZ_mkTupleC("assignFailure",1,
			  OZ_mkTupleC("proc",4,
				      OZ_atom("`<-`"),
				      rec?makeTaggedSRecord(rec):OZ_atom("noattributes"),
				      fea,
				      XPC(2))));
    }

  Case(INLINEUPARROW)
    {
      LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()););

      OZ_Return ret = uparrowInline(XPC(1),XPC(2),XPC(3));

      switch(ret) {
      case PROCEED:
	LOCAL_PROPAGATION(if (! localPropStore.do_propagation())
			  goto localhack4;);
	DISPATCH(5);

      case SUSPEND:
	  Assert(!shallowCP);
	  OZ_suspendOnInternal2(XPC(1),XPC(2));
	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+4));
	  e->suspendOnVarList(e->mkSuspThread());
	  CHECK_CURRENT_THREAD;

      case FAILED:
        LOCAL_PROPAGATION(localPropStore.reset());
      localhack4:
	HF_FAIL(OZ_mkTupleC("proc",4,OZ_atom("`^`"),XPC(1),XPC(2),AtomVoid));

      case RAISE:
	RAISE_BI(OZ_mkTupleC("proc",4,OZ_atom("`^`"),XPC(1),XPC(2),AtomVoid));

      case SLEEP:
      default:
	Assert(0);
      }
     }


  Case(INLINEFUN3)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun3 fun = (InlineFun3)entry->getInlineFun();

      // note XPC(5) is maybe the same as XPC(2) or XPC(3) or XPC(4) !!
      switch(fun(XPC(2),XPC(3),XPC(4),XPC(5))) {
      case PROCEED:
	DISPATCH(7);

      case SUSPEND:
	{
	  TaggedRef A=XPC(2);
	  TaggedRef B=XPC(3);
	  TaggedRef C=XPC(4);
	  if (shallowCP) {
	    XPC(5) = makeTaggedRef(newTaggedUVar(CBB));
	    e->emptySuspendVarList();
	    e->trail.pushIfVar(A);
	    e->trail.pushIfVar(B);
	    e->trail.pushIfVar(C);
	    DISPATCH(7);
	  }
	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+6));
	  e->suspendInline(3,A,B,C);
	  CHECK_CURRENT_THREAD;
	}

      case FAILED:
	SHALLOWFAIL;
	HF_FAIL(OZ_mkTupleC("proc",5,
			OZ_atom(entry->getPrintName()),
			XPC(2),XPC(3),XPC(4),AtomVoid));

      case RAISE:
	RAISE_BI(OZ_mkTupleC("proc",5,
			 OZ_atom(entry->getPrintName()),
			 XPC(2),XPC(3),XPC(4),AtomVoid));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(INLINEEQEQ)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineFun2 fun = (InlineFun2)entry->getInlineFun();

      // note XPC(4) is maybe the same as XPC(2) or XPC(3) !!
      switch (fun(XPC(2),XPC(3),XPC(4))) {
      case PROCEED:
	DISPATCH(6);
      case SUSPEND:
	{
	  TaggedRef A=XPC(2);
	  TaggedRef B=XPC(3);
	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+5));
	  e->suspendInline(2,A,B);
	  CHECK_CURRENT_THREAD;
	}

      case RAISE:
      case FAILED:
      case SLEEP:
      default:
	Assert(0);
      }
    }

#undef SHALLOWFAIL

// ------------------------------------------------------------------------
// CLASS: Shallow guards stuff
// ------------------------------------------------------------------------

  Case(SHALLOWGUARD)
    {
      shallowCP = PC;
      e->trail.pushMark();
      DISPATCH(3);
    }

  Case(SHALLOWTEST1)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel1 rel         = (InlineRel1)entry->getInlineFun();

      switch(rel(XPC(2))) {

      case PROCEED: DISPATCH(5);
      case FAILED:  JUMP( getLabelArg(PC+3) );
      case SUSPEND:
	e->pushTask(PC,Y,G,X,getPosIntArg(PC+4));
	addSusp (XPC(2), e->mkSuspThread ());
	CHECK_CURRENT_THREAD;

      case RAISE:
	RAISE_BI(OZ_mkTupleC("proc",2,
			 OZ_atom(entry->getPrintName()),
			 XPC(1)));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(SHALLOWTEST2)
    {
      BuiltinTabEntry* entry = (BuiltinTabEntry*) getAdressArg(PC+1);
      InlineRel2 rel         = (InlineRel2)entry->getInlineFun();

      switch(rel(XPC(2),XPC(3))) {

      case PROCEED: DISPATCH(6);
      case FAILED:  JUMP( getLabelArg(PC+4) );

      case SUSPEND:
	{
	  e->pushTask(PC,Y,G,X,getPosIntArg(PC+5));
	  Thread *thr = e->mkSuspThread ();
	  OZ_Term A=XPC(2);
	  OZ_Term B=XPC(3);
	  DEREF(A,APtr,ATag); DEREF(B,BPtr,BTag);
	  Assert(isAnyVar(ATag) || isAnyVar(BTag));
	  if (isAnyVar (A)) addSusp (APtr, thr);
	  if (isAnyVar (B)) addSusp (BPtr, thr);
	  CHECK_CURRENT_THREAD;
	}

      case RAISE:
	RAISE_BI(OZ_mkTupleC("proc",3,OZ_atom(entry->getPrintName()),
			 XPC(1),XPC(2)));

      case SLEEP:
      default:
	Assert(0);
      }
    }

  Case(SHALLOWTHEN)
    {
      if (e->trail.isEmptyChunk()) {
	shallowCP = NULL;
	e->trail.popMark();
	DISPATCH(1);
      }

      int argsToSave = getPosIntArg(shallowCP+2);
      e->pushTask(shallowCP,Y,G,X,argsToSave);
      Thread *thr = e->mkSuspThread ();
      shallowCP = NULL;
      e->reduceTrailOnShallow ();
      e->suspendOnVarList (thr);
      CHECK_CURRENT_THREAD;
    }


// -------------------------------------------------------------------------
// CLASS: Environment
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

  Case(DEALLOCATEL)
    {
      Assert(!isFreedRefsArray(Y));
      if (!isDirtyRefsArray(Y)) {
	deallocateY(Y);
      }
      Y=NULL;
      DISPATCH(1);
    }
// -------------------------------------------------------------------------
// CLASS: CONTROL: FAIL/SUCCESS/RETURN/SAVECONT
// -------------------------------------------------------------------------

  Case(FAILURE)
    {
      HF_FAIL(OZ_mkTupleC("instr",1,OZ_atom("failure")));
    }


  Case(SUCCEED)
    DISPATCH(1);

  Case(SAVECONT)
    e->pushTask(getLabelArg(PC+1),Y,G);
    DISPATCH(2);

  Case(RELEASEOBJECT)
    e->getSelf()->release();
    DISPATCH(1);

  Case(SETMODETODEEP)
    e->getSelf()->incDeepness();
    DISPATCH(2);

  Case(RETURN)
    goto LBLpopTask;


// ------------------------------------------------------------------------
// CLASS: Definition
// ------------------------------------------------------------------------

  Case(DEFINITION)
    {
      Reg reg                     = getRegArg(PC+1);
      ProgramCounter nxt          = getLabelArg(PC+2);
      PrTabEntry *predd           = getPredArg(PC+5);
      AbstractionEntry *predEntry = (AbstractionEntry*) getAdressArg(PC+6);
      
      AssRegArray &list = predd->gRegs;
      int size = list.getSize();
      RefsArray gRegs = (size == 0) ? (RefsArray) NULL : allocateRefsArray(size);

      Abstraction *p = new Abstraction (predd, gRegs, CBB);

      if (predEntry) {
	predEntry->setPred(p);
      }

      for (int i = 0; i < size; i++) {
	switch (list[i].kind) {
	case XReg: gRegs[i] = X[list[i].number]; break;
	case YReg: gRegs[i] = Y[list[i].number]; break;
	case GReg: gRegs[i] = G[list[i].number]; break;
	}
      }
      Xreg(reg) = makeTaggedConst(p);
      JUMP(nxt);
    }

// -------------------------------------------------------------------------
// CLASS: CONTROL: FENCE/CALL/EXECUTE/SWITCH/BRANCH
// -------------------------------------------------------------------------
  
  Case(BRANCH)
    JUMP( getLabelArg(PC+1) );

  
  /* weakDet(X) woken up, WHENEVER something happens to X 
   *            (important if X is a FD var) */
  Case(WEAKDETX) ONREG(WeakDet,X);
  Case(WEAKDETY) ONREG(WeakDet,Y);
  Case(WEAKDETG) ONREG(WeakDet,G);
  WeakDet:
  {
    TaggedRef term = RegAccess(HelpReg,getRegArg(PC+1));
    DEREF(term,termPtr,tag);

    if (!isAnyVar(tag)) {
      DISPATCH(3);
    }
    /* INCFPC(3); do NOT suspend on next instructions: DET suspensions are
                  woken up always, even if variable is bound to another var */

    int argsToSave = getPosIntArg(PC+2);
    SUSP_PC(termPtr,argsToSave,PC);
  }


  /* det(X) wait until X will be ground */
  Case(DETX) ONREG(Det,X);
  Case(DETY) ONREG(Det,Y);
  Case(DETG) ONREG(Det,G);
  Det:
  {
    TaggedRef term = RegAccess(HelpReg,getRegArg(PC+1));
    DEREF(term,termPtr,tag);

    if (!isAnyVar(tag)) {
      DISPATCH(3);
    }
    /* INCFPC(3): dont do it */
    int argsToSave = getPosIntArg(PC+2);
    e->pushTask(PC,Y,G,X,argsToSave);
    Thread *thr = e->mkSuspThread ();
    if (isCVar (tag)) {
      (tagged2CVar (term))->addDetSusp (thr);
    } else {
      addSusp (termPtr, thr);
    }
    CHECK_CURRENT_THREAD;
  }

  Case(TAILSENDMSGX) isTailCall = OK; ONREG(SendMethod,X);
  Case(TAILSENDMSGY) isTailCall = OK; ONREG(SendMethod,Y);
  Case(TAILSENDMSGG) isTailCall = OK; ONREG(SendMethod,G);

  Case(SENDMSGX) isTailCall = NO; ONREG(SendMethod,X);
  Case(SENDMSGY) isTailCall = NO; ONREG(SendMethod,Y);
  Case(SENDMSGG) isTailCall = NO; ONREG(SendMethod,G);

 SendMethod:
  {
    TaggedRef label    = getLiteralArg(PC+1);
    TaggedRef origObj  = RegAccess(HelpReg,getRegArg(PC+2));
    TaggedRef object   = origObj;
    SRecordArity arity = (SRecordArity) getAdressArg(PC+3);

    /* compiler ensures: if object is in X[n], then n == arity+1,
     * so in case we have to suspend we save one additional argument */
    Assert(HelpReg!=X || getWidth(arity)==regToInt(getRegArg(PC+2)));

    DEREF(object,objectPtr,_2);
    if (isObject(object)) {
      Object *obj      = (Object *) tagged2Const(object);
      Abstraction *def = getSendMethod(obj,label,arity,(MethodCache*)(PC+4));
      if (def == NULL) {
	goto bombSend;
      }

      if (!isTailCall) { 
	CallPushCont(PC+6);
      }
      SaveSelf(e,obj,OK);
      Assert(obj->getDeepness()==0);
      obj->incDeepness();
      CallDoChecks(def,def->getGRegs(),getWidth(arity));
      JUMP(def->getPC());
    }

    if (isAnyVar(object)) {
      SUSP_PC(objectPtr,getWidth(arity)+1,PC);
    }

    if (isProcedure(object)) 
      goto bombSend;

    DORAISE(OZ_mkTupleC("applyFailure",1,
			OZ_mkTupleC("proc",2,object,
				    makeMessage(arity,label,X))));

  bombSend:
    PC = isTailCall ? PC : PC+6;
    X[0] = makeMessage(arity,label,X);
    predArity = 1;
    predicate = tagged2Const(object);
    goto LBLcall;
  }


  Case(TAILAPPLMETHX) isTailCall = OK; ONREG(ApplyMethod,X);
  Case(TAILAPPLMETHY) isTailCall = OK; ONREG(ApplyMethod,Y);
  Case(TAILAPPLMETHG) isTailCall = OK; ONREG(ApplyMethod,G);

  Case(APPLMETHX) isTailCall = NO; ONREG(ApplyMethod,X);
  Case(APPLMETHY) isTailCall = NO; ONREG(ApplyMethod,Y);
  Case(APPLMETHG) isTailCall = NO; ONREG(ApplyMethod,G);

 ApplyMethod:
  {
    ApplMethInfoClass *ami = (ApplMethInfoClass*) getAdressArg(PC+1);
    TaggedRef label        = ami->methName;
    SRecordArity arity     = ami->arity;
    TaggedRef origObject   = RegAccess(HelpReg,getRegArg(PC+2));
    TaggedRef object       = origObject;
    Abstraction *def       = NULL;

    PC = isTailCall ? PC : PC+3;

    DEREF(object,objectPtr,objectTag);
    if (!isObject(object)) {
      goto bombApply;
    }
    def = getApplyMethod((Object *) tagged2Const(object),label,arity);
    if (def==NULL) {
      goto bombApply;
    }
    
    if (!isTailCall) { CallPushCont(PC); }
    CallDoChecks(def,def->getGRegs(),getWidth(arity));
    JUMP(def->getPC());


  bombApply:
    if (methApplHdl == makeTaggedNULL()) {
      error("no apply handler");
    }

    X[1] = makeMessage(arity,label,X);
    X[0] = origObject;

    predArity = 2;
    predicate = tagged2Const(methApplHdl);
    goto LBLcall;
  }


  Case(CALLX) isTailCall = NO; ONREG(Call,X);
  Case(CALLY) isTailCall = NO; ONREG(Call,Y);
  Case(CALLG) isTailCall = NO; ONREG(Call,G);

  Case(TAILCALLX) isTailCall = OK; ONREG(Call,X);
  Case(TAILCALLY) isTailCall = OK; ONREG(Call,Y);
  Case(TAILCALLG) isTailCall = OK; ONREG(Call,G);

 Call:
   {
     {
       TaggedRef taggedPredicate = RegAccess(HelpReg,getRegArg(PC+1));
       predArity = getPosIntArg(PC+2);

       DEREF(taggedPredicate,predPtr,predTag);
       if (!isProcedure(taggedPredicate) && !isObject(taggedPredicate)) {
	 if (isAnyVar(predTag)) {
	   /* compiler ensures: if pred is in X[n], then n == arity+1,
	    * so we save one additional argument */
	   Assert(HelpReg!=X || predArity==regToInt(getRegArg(PC+1)));
	   SUSP_PC(predPtr,predArity+1,PC);
	 }
	 DORAISE(OZ_mkTupleC("applyFailure", 1,
			 mkSTupleX("proc",taggedPredicate, X,predArity)));
       }

       PC = isTailCall ? PC : PC+3;
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

       switch (typ) {
       case Co_Abstraction:
       case Co_Object:
	 {
	   Abstraction *def;
	   if (typ==Co_Object) {
	     /* {Obj Msg} --> {Obj Msg Methods} */
	     Object *o = (Object*) predicate;
	     if (o->isClass()) {
	       DORAISE(OZ_mkTupleC("applyFailure",1,
			       mkSTupleX("proc",makeTaggedConst(predicate),
					X,predArity)));
	     }
	     CheckArity(1,makeTaggedConst(o));
	     def = o->getAbstraction();
	     X[predArity++] = o->getSlowMethods();
	     X[predArity++] = makeTaggedConst(o);
	     if (!isTailCall) { 
	       CallPushCont(PC);
	     }
	     SaveSelf(e,o,OK); 
	     // o->deepness handelt by 'objectIsFree'
	   } else {
	     def = (Abstraction *) predicate;
	     CheckArity(def->getArity(), makeTaggedConst(def));
	     if (!isTailCall) { CallPushCont(PC); }
	   }
	   CallDoChecks(def,def->getGRegs(),def->getArity());
	   JUMP(def->getPC());
	 }


// -----------------------------------------------------------------------
// --- Call: Builtin
// -----------------------------------------------------------------------
       case Co_Builtin:
	 {
	   bi = (Builtin *) predicate;
	   
	   CheckArity(bi->getArity(),makeTaggedConst(bi));
	   
	   switch (bi->getType()) {
	     
	   case BIraise:
	     goto LBLraise;
	     
	   case BIDefault:
	     {
	       LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()););

	       if (e->isSetSFlag(DebugMode)) {
		 enterCall(CBB,makeTaggedConst(bi),predArity,X);
	       }

	       biFun=bi->getFun();
	       OZ_Return res = biFun(predArity, X);
	       if (e->isSetSFlag(DebugMode)) {
		 exitBuiltin(res,makeTaggedConst(bi),predArity,X);
	       }

	       switch (res) {
	    
	       case SUSPEND:
		 LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()););

		 {
		   TaggedRef sh = bi->getSuspHandler();
		   if (sh==makeTaggedNULL()) {
		     if (!isTailCall) e->pushTask(PC,Y,G);
		     e->pushCFun(biFun,X,predArity);
		     Thread *thr=e->mkSuspThread();
		     e->suspendOnVarList(thr);
		     CHECK_CURRENT_THREAD;
		   }
		   predicate = tagged2Const(sh);
		 }
		 goto LBLcall; 
	       case FAILED:
		 LOCAL_PROPAGATION(Assert(localPropStore.isEmpty()););
		 LOCAL_PROPAGATION(localPropStore.reset());
	       localHack3:
		 HF_FAIL(INFO_BI);

	       case SLEEP: // no break
		 error ("'xxxCALLxxx' has got 'SLEEP' back!\n");
	       case PROCEED:
		 LOCAL_PROPAGATION(if (! localPropStore.do_propagation())
				   goto localHack3;);
		 if (isTailCall) {
		   goto LBLpopTask;
		 }
		 JUMP(PC);

	       case RAISE:
		 RAISE_BI(INFO_BI);

	       default:
		 error("builtin: bad return value");
		 goto LBLerror;
	       }
	     }
	   default:
	     break;
	   }
	   error("emulate: call: builtin %s not correctly specified",
		 bi->getPrintName());
	   goto LBLerror;
	 } // end builtin
       default:
	 error("unknown procedure type");
       } // end switch on type of predicate
     }

// ------------------------------------------------------------------------
// --- Call: Builtin: raise
// ------------------------------------------------------------------------
    
   LBLraise:
     // exception is in X[0];
     {
       DebugCheck(ozconf.stopOnToplevelFailure, tracerOn();trace("raise"));

       TaskStackEntry *oldTos;
       int spaceCount;
       TaggedRef pred;
       if (!e->currentThread) {
	 e->defaultExceptionHandler(X[0],NOCODE,0);
	 goto LBLpopTask;
       }

       spaceCount = e->currentThread->findExceptionHandler(pred,oldTos);
       while (spaceCount-- > 0) e->deinstallCurrent();
       Assert(currentDebugBoard=CBB);


       /* exception is already in X[0], but should somehow be reflected !!! */
       if (pred!=makeTaggedNULL()) {
	 isTailCall = OK;
	 predicate=tagged2Const(pred);
	 predArity=1;
	 goto LBLcall;
       }

       e->defaultExceptionHandler(X[0],PC,oldTos);
       goto LBLpopTask;
     }

   }
// --------------------------------------------------------------------------
// --- end call/execute -----------------------------------------------------
// --------------------------------------------------------------------------

  
  Case(WAIT)
    {
      Assert(CBB->isWait() && !CBB->isCommitted());

      CBB->decSuspCount();
      {
	TaskStackEntry topElem = e->currentThread->pop ();
	Assert((ContFlag) (ToInt32(topElem) & 0xf) == C_LOCAL);
      }
      /* unit commit */
      WaitActor *aa = WaitActor::Cast(CBB->getActor());
      if (aa->hasOneChild()) {
	Board *waitBoard = CBB;
	e->reduceTrailOnUnitCommit();
        waitBoard->unsetInstalled();
	e->setCurrent(aa->getBoardFast());
	DebugCheckT(currentDebugBoard=CBB);

	waitBoard->setCommitted(CBB);   // by kost@ 4.10.94
	CBB->incSuspCount(waitBoard->getSuspCount()-1);
	if (aa->hasChoices()) {
	  if (CBB->isWait()) {
	    WaitActor::Cast(CBB->getActor())->pushChoices(aa->getCps());
	  } else if (CBB->isSolve()) {
	    SolveActor::Cast(CBB->getActor())->pushChoices(aa->getCps());
	  }
	}

	Bool ret = e->installScript(waitBoard->getScriptRef());
	if (!ret) {
	  HF_FAIL(OZ_atom("commitClause"));
	}
	Assert(ret!=NO);
	DISPATCH(1);
      }

      // not commitable, suspend
      CBB->setWaiting();
      goto LBLsuspendBoard;
    }


  Case(WAITTOP)
    {
      Assert(CBB->isWait() && !CBB->isCommitted());

      /* top commit */
      CBB->decSuspCount();
      {
	TaskStackEntry topElem = e->currentThread->pop ();
	Assert((ContFlag) (ToInt32(topElem) & 0xf) == C_LOCAL);
      }
      if ( e->entailment() ) {
	e->trail.popMark();

	tmpBB = CBB;

	e->setCurrent(CBB->getParentFast());
	DebugCheckT(currentDebugBoard=CBB);
	tmpBB->unsetInstalled();
	tmpBB->setCommitted(CBB);
	CBB->decSuspCount();

	goto LBLpopTask;
      }

      /* unit commit for WAITTOP */
      WaitActor *aa = WaitActor::Cast(CBB->getActor());
      Board *bb = CBB;
      if (aa->hasOneChild()) {
	e->reduceTrailOnUnitCommit();
        bb->unsetInstalled();
	e->setCurrent(aa->getBoardFast());
	DebugCheckT(currentDebugBoard=CBB);

	bb->setCommitted(CBB);    // by kost@ 4.10.94
	CBB->incSuspCount(bb->getSuspCount()-1);

	if (aa->hasChoices()) {
	  if (CBB->isWait()) {
	    WaitActor::Cast(CBB->getActor())->pushChoices(aa->getCps());
	  } else if (CBB->isSolve()) {
	    SolveActor::Cast(CBB->getActor())->pushChoices(aa->getCps());
	  }
	}

	Bool ret = e->installScript(bb->getScriptRef());
	if (!ret) {
	  HF_FAIL(OZ_atom("commitClause"));
	}

	Assert(ret != NO);
	goto LBLpopTask;
      }

      /* suspend WAITTOP */
      CBB->setWaitTop();
      CBB->setWaiting();
      goto LBLsuspendBoardWaitTop;
    }

  Case(ASK)
    {
      Assert(CBB->isAsk() && !CBB->isCommitted());

      CBB->decSuspCount();
      {
	TaskStackEntry topElem = e->currentThread->pop ();
	Assert((ContFlag) (ToInt32(topElem) & 0xf) == C_LOCAL);
      }
      // entailment ?
      if (e->entailment()) {
	e->trail.popMark();
	tmpBB = CBB;
	e->setCurrent(CBB->getParentFast());
	DebugCheckT(currentDebugBoard=CBB);
	tmpBB->unsetInstalled();
	tmpBB->setCommitted(CBB);
	CBB->decSuspCount();
	DISPATCH(1);
      }

    LBLsuspendBoard:

      CBB->setBody(PC+1, Y, G,NULL,0);

    LBLsuspendBoardWaitTop:
      markDirtyRefsArray(Y);
      DebugTrace(trace("suspend clause",CBB,CAA));

      CAA = AWActor::Cast (CBB->getActor());

      e->deinstallCurrent();
      DebugCheckT(currentDebugBoard=CBB);
      if (CAA->hasNext()) {

	DebugTrace(trace("next clause",CBB,CAA));

	LOADCONT(CAA->getNext());

	goto LBLemulate; // no thread switch allowed here (CAA)
      }

      if (CAA->isAsk()) {
	e->suspendCond(AskActor::Cast(CAA));
	CHECK_CURRENT_THREAD;
      }
      DebugTrace(trace("suspend actor",CBB,CAA));

      goto LBLpopTask;
    }


// -------------------------------------------------------------------------
// CLASS: NODES: CREATE/END
// -------------------------------------------------------------------------

  Case(CREATECOND)
    {
      ProgramCounter elsePC = getLabelArg(PC+1);
      int argsToSave = getPosIntArg(PC+2);

      CAA = new AskActor(CBB,CPP,
			 elsePC ? elsePC : NOCODE,
			 NOCODE, Y, G, X, argsToSave,
			 e->currentThread);
      DISPATCH(3);
    }

  Case(CREATEOR)
    {
      ProgramCounter elsePC = getLabelArg(PC+1);
      int argsToSave = getPosIntArg(PC+2);

      CAA = new WaitActor(CBB, CPP, NOCODE, Y, G, X, argsToSave, NO);

      DISPATCH(3);
    }

  Case(CREATEENUMOR)
    {
      ProgramCounter elsePC = getLabelArg(PC+1);
      int argsToSave = getPosIntArg(PC+2);
      Board *bb = CBB;

      CAA = new WaitActor(bb, CPP, NOCODE, Y, G, X, argsToSave, NO);

      if (bb->isWait()) {
	WaitActor::Cast(bb->getActor())->pushChoice((WaitActor *) CAA);
      } else if (bb->isSolve()) {
	SolveActor::Cast(bb->getActor())->pushChoice((WaitActor *) CAA);
      }

      DISPATCH(3);
    }

  Case(CREATECHOICE)
    {
      ProgramCounter elsePC = getLabelArg (PC+1);
      int argsToSave = getPosIntArg (PC+2);
      Board *bb = CBB;

      CAA = new WaitActor(bb, CPP, NOCODE, Y, G, X, argsToSave, OK);

      if (bb->isWait()) {
	WaitActor::Cast(bb->getActor())->pushChoice((WaitActor *) CAA);
      } else if (bb->isSolve()) {
	SolveActor::Cast(bb->getActor())->pushChoice((WaitActor *) CAA);
      }

      DISPATCH(3);
    }

  Case(WAITCLAUSE)
    {
      // create a node
      e->setCurrent(new Board(CAA,Bo_Wait),OK);
      DebugCheckT(currentDebugBoard=CBB);
      e->pushLocal();
      CBB->setInstalled();
      e->trail.pushMark();
      DebugCheckT(CAA=NULL);
      IncfProfCounter(waitCounter,sizeof(Board));
      DISPATCH(1);
    }

  Case(ASKCLAUSE)
    {
      e->setCurrent(new Board(CAA,Bo_Ask),OK);
      DebugCheckT(currentDebugBoard=CBB);
      e->pushLocal();
      CBB->setInstalled();
      e->trail.pushMark();
      DebugCheckT(CAA=NULL);
      IncfProfCounter(askCounter,sizeof(Board));
      DISPATCH(1);
    }


  Case(ELSECLAUSE)
    DISPATCH(1);

  Case(THREAD)
    {
      markDirtyRefsArray(Y);
      ProgramCounter newPC = PC+2;
      ProgramCounter contPC = getLabelArg(PC+1);

      int prio = CPP;
      int defPrio = ozconf.defaultPriority;

      if (prio > defPrio) {
	prio = defPrio;
      }

      Thread *tt = new Thread (prio, CBB);
      tt->pushCont(newPC,Y,G,NULL,0);
      e->scheduleThread (tt);

      JUMP(contPC);
    }


  Case(NEXTCLAUSE)
      CAA->nextClause(getLabelArg(PC+1));
      DISPATCH(2);

  Case(LASTCLAUSE)
      CAA->lastClause();
      DISPATCH(1);

// -------------------------------------------------------------------------
// CLASS: MISC: ERROR/NOOP/default
// -------------------------------------------------------------------------

  Case(OZERROR)
      error("Emulate: OZERROR command executed");
      goto LBLerror;


  Case(DEBUGINFO)
    {
      TaggedRef filename = getLiteralArg(PC+1);
      int line           = smallIntValue(getNumberArg(PC+2));
      int absPos         = smallIntValue(getNumberArg(PC+3));
      TaggedRef comment  = getLiteralArg(PC+4);
      int noArgs         = smallIntValue(getNumberArg(PC+5));
      /*
      printf("%s in line %d in file: %s\n",
	     toC(comment),line,toC(filename));
      if (noArgs != -1) {
	printf("\t");
	for (int i=0; i<noArgs; i++) {
	  printf("%s ",toC(X[i]));
        }
      }
      printf("\n");
      */

      TaggedRef dbgVar = e->currentThread->getDebugVar();
      if (! OZ_isVariable(dbgVar)) {
	DISPATCH(6);
      }

      Board *b = e->currentBoard;
      e->currentBoard = e->rootBoard;
      
      OZ_Term waitFor   = OZ_newVariable();

      OZ_Term newDbgVar = OZ_newVariable();
      e->currentThread->setDebugVar(newDbgVar);

      OZ_Term dbgTuple = OZ_mkTupleC("dbgInfo",
				     5,
				     makeInt(line),
				     makeInt(absPos),
				     waitFor,
				     newDbgVar,
				     comment
				     );
      if (!OZ_unify(dbgVar,dbgTuple)) {
	warning("returnDebugVar(%s,%s) failed",toC(dbgVar),toC(dbgTuple));
	e->currentBoard = b;
	DISPATCH(6);
      }

      e->currentBoard = b;
      SUSP_PC(waitFor,noArgs,PC+6);
    }

  Case(JOB)
    {
      ProgramCounter contPC = getLabelArg(PC+1);

      markDirtyRefsArray(Y);
      e->currentThread->pushCont(contPC,Y,G,NULL,0);
      e->currentThread->pushJob();
      DISPATCH(2);
    }

  Case(GENCALL) 
    {
      GenCallInfoClass *gci = (GenCallInfoClass*)getAdressArg(PC+1);
      int arity = getPosIntArg(PC+2);
      Assert(arity==0); /* is filled in by procedure genCallInfo */
      TaggedRef pred = G[gci->regIndex];
      DEREF(pred,predPtr,_1);
      if (isAnyVar(pred)) {
	SUSP_PC(predPtr,getWidth(gci->arity),PC);
      }
      genCallInfo(gci,pred,PC);
      gci->dispose();
      DISPATCH(0);
    }


  Case(TESTLABEL1)
  Case(TESTLABEL2)
  Case(TESTLABEL3)
  Case(TESTLABEL4)

  Case(TEST1)
  Case(TEST2)
  Case(TEST3)
  Case(TEST4)

  Case(ENDOFFILE)
  Case(ENDDEFINITION)


#ifndef THREADED
  default:
     error("emulate instruction: default should never happen");
     break;
   } /* switch*/
#endif


  // ----------------- end emulate ------------------------------------------

// ------------------------------------------------------------------------
// *** FAILURE
// ------------------------------------------------------------------------
 LBLshallowFail:
  {
    if (e->trail.isEmptyChunk()) {
      e->trail.popMark();
    } else {
      e->reduceTrailOnFail();
    }
    ProgramCounter nxt = getLabelArg(shallowCP+1);
    shallowCP = NULL;
    JUMP(nxt);
  }

  /*
   *  kost@ : There are now the following invariants:
   *  - Can be entered only in a deep guard;
   *  - A thread must be runnable, or there is no one at all (when came 
   *    from 'LBLkillThread' or from the 'LBLfailure' itself);
   *
   */
 LBLfailure:
  Assert (!(e->isToplevel ()));
  Assert (!e->currentThread || e->currentThread->isRunnable ());
  {
    AWActor *aa;
    Continuation *cont;
    switch (e->handleFailure(cont,aa)) {
    case CE_CONT:
      DebugCheckT(currentDebugBoard=e->currentBoard);

      //
      if (!e->currentThread) {
	Thread *thr = new Thread (aa->getPriority (), CBB);
	thr->pushCont (cont);
	thr->pushSetCaa ((AskActor *) aa);;
	e->scheduleThread (thr);

	// 
	goto LBLstart;
      } else {
	CAA=aa;
	LOADCONT(cont);

	//
	goto LBLemulate; // no thread switch allowed here (CAA)
      }

    case CE_NOTHING:
      if (e->currentThread) {
	DebugCheckT(currentDebugBoard=e->currentBoard);
	goto LBLpopTask;
      } else {
	goto LBLstart;
      }

    case CE_FAIL:
      HF_FAIL(OZ_atom("noInfo"));

    case CE_SUSPEND:
      Assert (e->currentThread);
      Assert (aa->isAsk ());
      DebugCheckT(currentDebugBoard=e->currentBoard);

      //
      e->suspendCond(AskActor::Cast(aa));
      CHECK_CURRENT_THREAD;
    }
  }
} // end engine

int AM::handleFailure(Continuation *&cont, AWActor *&aaout)
{
  aaout=0;
  DebugTrace(trace("fail",currentBoard));
  Assert(currentBoard->isInstalled());
  Actor *aa=currentBoard->getActor();
  if (aa->isAskWait()) {
    (AWActor::Cast(aa))->failChild(currentBoard);
  }
  currentBoard->setFailed();
  reduceTrailOnFail();
  currentBoard->unsetInstalled();

  if (currentThread) {
    //
    if (!(currentThread->hasStack ()) || 
	//  i.e. that's a runnable thread, yet without a task stack,
	// which installation has failed;
	//  Note that propagators are also covered by this case!
	!(currentThread->discardLocalTasks ())
	//  i.e. that's a (proper) runnable thread which doesn't have
	// any tasks (but in the failed board) more;
	) {
      Assert (!(currentThread->hasStack ()) || 
	      currentThread->isEmpty ());
      //
      currentThread->setBoard (currentBoard);
      scheduleThread (currentThread);
      currentThread = (Thread *) NULL;
      /*
       *  kost@ 22.12.95 bug fix:
       *  In the case of a deep (and parallel) guard, it might happen 
       * that the thread in the actor is overwritten (by a wrong 
       * thread, which doesn't contain a continuation after the conditional
       * itself!). So, if a thread seems to be pretty local, just 
       * schedule it again - it will be discarded later;
       *
       *  to mm2: Michael, i told you aboout this before!
       */
    }
  }
  //  kost@ moved from ahead - we need a pointer to the board being killed;
  setCurrent(aa->getBoardFast());

  DebugTrace(trace("reduce actor",currentBoard,aa));

  if (aa->isAsk()) {
    AskActor *aaa = AskActor::Cast(aa);
    Thread *tt = aaa->getThread ();

    //
    Assert (tt);
    aaout = aaa;

    // 
    //  The first case: there is(are) still clause(s) to build up;
    if (aaa->hasNext()) {
      // 
      if (tt != currentThread) {
	//  ... it means actually that the currentThread was a local one, 
	// and it was handled just above, so here:
	Assert (currentThread == (Thread *) NULL);
	//  The following holds because the thread in the actor 
	// might be suspended only if all the clauses are built up
	// (that is NOT the case here);
	Assert (!(tt->isSuspended ()));
	// ... because it's not the current one;
	Assert (isScheduled (tt)); 

	//
	if (tt->isBelowFailed(currentBoard)) { // slow test
	  //  ... and this means that 'tt' is still trying to 
	  // build up the same (failed) board - we discard 
	  // everything in there, and continue with the next guard;
	  tt->cleanUp (currentBoard);

	  //
	  cont = aaa->getNext ();  // 'nextClause';
	  tt->pushCont(cont);
	  tt->pushSetCaa (aaa);
	  //  don't schedule it again;
	}

	//
	// tt is executing another clause: don't disturb him
	DebugCode (cont = (Continuation *) NULL);
	return (CE_NOTHING);
      } else {
	//  ... it means that the 'main' thread has failed 
	// inside a guard - then just continue with the next one;
	cont = aaa->getNext ();	   // 'nextClause';

	//
	return (CE_CONT);
      }
    }

    // 
    //  The second case: should we activate the 'else' clause?
    if (aaa->isLeaf()) {
      aaa->setCommitted();
      currentBoard->decSuspCount();

      cont = aaa->getNext();
      cont->setPC(aaa->getElsePC());

      /* rule: if fi --> false */
      if (cont->getPC() == NOCODE) {
	return CE_FAIL;
      }

      // 
      if (tt != currentThread) {
	//  ... the same as above: it wasn't the 'main' thread;
	Assert (currentThread == (Thread *) NULL);

	// 
	//  The 'tt' thread can be suspended or not: the last case 
	// is possible if the 'main' thread is still building up
	// the same (last) guard;
	if (tt->isSuspended ()) {
	  Assert (!(isScheduled (tt)));
	  //  The following must hold because 'tt' can suspend 
	  // only in the board where the actor itself is located;
	  Assert (tt->getBoardFast () == currentBoard);

	  //
	  tt->suspThreadToRunnable ();
	  tt->pushCont (cont);	// no 'SetCaa' is ever needed;

	  //
	  scheduleThread (tt);
	} else {
	  Assert (tt->isBelowFailed (currentBoard));
	  //  ... it means that the 'main' thread is still building 
	  // the same failed guard;
	  tt->cleanUp (currentBoard);
	  tt->pushCont (cont);	// no 'SetCaa' is ever needed;

	  //  don't schedule it again;
	}

	//
	DebugCode (cont = (Continuation *) NULL);
	return (CE_NOTHING);
      } else {
	//  ... the 'main' thread;
	return (CE_CONT);
      }
    }

    //
    //  The third case: all the guards are built up, 
    // not all of them are failed, and none of them are entailed;
    if (tt != currentThread) {
      //  ... not the 'main' thread;
      Assert (currentThread == (Thread *) NULL);
      //  The assertions below are the same as for the 'leaf' case above;
#ifdef DEBUG_CHECK
      if (tt->isSuspended ()) {
	Assert (!(isScheduled (tt)));
	Assert (tt->getBoardFast () == currentBoard);
      } else {
	Assert (isScheduled (tt));
	// the 'tt' thread might or might not be below the failed
	// board - so, let clean up it lazily ...
      }
#endif

      // 
      DebugCode (cont = (Continuation *) NULL);
      return (CE_NOTHING);
    } else {
      //  ... the 'main' thread;
      //  Note that it's cleaned up already;
      Assert (currentThread);

      //
      DebugCode (cont = (Continuation *) NULL);
      return (CE_SUSPEND);
    }
  }

  // 
  if (aa->isWait()) {
    WaitActor *waa = WaitActor::Cast(aa);
    aaout=waa;
    if (waa->hasNext()) {
      cont=waa->getNext();
      return CE_CONT;
    }

    /* rule: or ro (bottom commit) */
    if (waa->hasNoChilds()) {
      waa->setCommitted();
      return CE_FAIL;
    }

    /* rule: or <sigma> ro (unit commit rule) */
    if (waa->hasOneChild()) {
      Board *waitBoard = waa->getLastChild();
      DebugTrace(trace("reduce actor unit commit",waitBoard,waa));
      if (waitBoard->isWaiting()) {
	waitBoard->setCommitted(currentBoard); // do this first !!!

	// kost@: 'incSuspCount' moved from behind, because
	// otherwise the current board can be found to be 
	// entailed - that may be not true;
	/* add the suspension from the committed board
	   remove the suspension for the board itself */
	currentBoard->incSuspCount(waitBoard->getSuspCount()-1);

	if (waa->hasChoices()) {
	  if (currentBoard->isWait()) {
	    WaitActor::Cast(currentBoard->getActor())->pushChoices(waa->getCps());
	  } else if (currentBoard->isSolve()) {
	    SolveActor::Cast(currentBoard->getActor())->pushChoices(waa->getCps());
	  }
	}

	DebugCode (if (!currentThread) 
		   currentThread = (Thread *) KOSTJA_MAGIC;);

	if (!installScript(waitBoard->getScriptRef())) {
	  DebugCode (if (currentThread == (Thread *) KOSTJA_MAGIC)
		     currentThread = (Thread *) NULL;);
	  return CE_FAIL;
	}
	DebugCode (if (currentThread == (Thread *) KOSTJA_MAGIC)
		   currentThread = (Thread *) NULL;);

	/* unit commit & WAITTOP */
	if (waitBoard->isWaitTop()) {
	  if (!currentThread && currentBoard != rootBoard) {
	    // mm2: entailment check ???
	    // kost@:  Yes! This can happen;
	    Thread *thr = new Thread (currentBoard);
	    thr->wakeupToRunnable ();

	    //  fairness: don't go there directly, but:
	    scheduleThread (thr);
	  }
	  return CE_NOTHING;
	}

	cont=waitBoard->getBodyPtr();
	return CE_CONT;
      }
    }
    return CE_NOTHING;
  }

  Assert(aa->isSolve());

  //  Reduce (i.e. with failure in this case) the solve actor;
  //  The solve actor goes simply away, and the 'failed' atom is bound to
  // the result variable; 
  aa->setCommitted();
  SolveActor *saa=SolveActor::Cast(aa);
  // don't decrement parent counter

  // for statistic purposes
  ozstat.incSolveFailed();
  if (!fastUnifyOutline(saa->getResult(),saa->genFailed(),OK)) {
    return CE_FAIL;
  }
  if (!currentThread && currentBoard != rootBoard) {
    Thread *thr = new Thread (currentBoard);
    thr->wakeupToRunnable ();

    //
    scheduleThread (thr);
  }
  return CE_NOTHING;
}

#ifdef OUTLINE
#undef inline
#endif

