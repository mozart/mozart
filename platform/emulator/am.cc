/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow, mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------

  exported variables/classes:

  exported procedures:

  ------------------------------------------------------------------------

  internal static variables:

  internal procedures:

  ------------------------------------------------------------------------
*/
/*
      (main) AM::procedure;
      another things.
*/


#ifdef __GNUC__
#pragma implementation "am.hh"
#endif

#include <signal.h>

#include "../include/config.h"

#include "sun-proto.h"

#include "actor.hh"
#include "am.hh"
#include "bignum.hh"
#include "builtins.hh"
#include "genvar.hh"
#include "io.hh"
#include "misc.hh"
#include "records.hh"
#include "thread.hh"
#include "tracer.hh"
#include "ozdebug.hh"

AM am;

ConfigData::ConfigData() {
  ozPath		= OZ_PATH;
  linkPath		= OZ_PATH;
  printDepth		= PRINT_DEPTH;
  showFastLoad		= SHOW_FAST_LOAD;
  showForeignLoad	= SHOW_FOREIGN_LOAD;
  showIdleMessage	= SHOW_IDLE_MESSAGE;
  gcFlag		= GC_FLAG;
  gcVerbosity		= GC_VERBOSITY;
  timeSlice		= TIME_SLICE;
  defaultPriority	= DEFAULT_PRIORITY;
  systemPriority	= SYSTEM_PRIORITY;
  clockTick		= CLOCK_TICK;
  taskStackSize		= TASK_STACK_SIZE;
}
extern "C" int runningUnderEmacs; // mm2
extern void version(); // mm2


void usage(int /* argc */,char **argv) {
  fprintf(stderr,
	  "usage: %s [-E] [-s port | -S file | -f file] [-d] [-c compiler]\n",
	  argv[0]);
  exit(1);
}

void AM::init(int argc,char **argv)
{  
  version();

#ifdef DEBUG_CHECK
  printf("Compile Flags:"
	 " DEBUG_CHECK"
#ifdef DEBUG_DET
	 " DEBUG_DET"
#endif
#ifdef DEBUG_TRACE
	 " DEBUG_TRACE"
#endif
#ifdef DEBUG_GC
         " DEBUG_GC"
#endif
#ifdef DEBUG_FD
         " DEBUG_FD"
#endif
#ifdef PROFILE_FD
         " PROFILE_FD"
#endif
#ifdef RECINSTRFETCH
         " RECINSTRFETCH=%d", RECINSTRFETCH
#endif
	 );
#endif

  printf("\n");
  
#ifdef PROFILE
  printf("Compiled to support gprof-profiling.\n");
#endif

#if THREADED == 1
  printf("Using threaded code (rel jumps).\n");
#else
#if THREADED == 2
  printf("Using threaded code (abs jumps).\n");
#else
  printf("Not using threaded code.\n");
#endif
#endif


#ifdef MAKEANEWPGRP
  // create a new process group, so that we can
  // easily terminate all our children
  if (setpgrp(getpid(),getpid()) < 0) {
    ozperror("setpgid");
  }
#endif

  int c;

  extern char *optarg;
  extern int optind, opterr;
  
  int port = -1; // read from Compiler

  char *compilerFile;
  if (!(compilerFile = getenv("OZCOMPILER"))) {
    compilerFile = OzCompiler;
  }

  char *tmp;
  if ((tmp = getenv("OZPATH"))) {
    conf.ozPath = tmp;
    conf.linkPath = tmp;
  }

  if ((tmp = getenv("OZLINKPATH"))) {
    conf.linkPath = tmp;
  }

  char *comPath = NULL;  // path name where to create AF_UNIX socket
  char *queryFileName = NULL;

  while ((c = getopt(argc, argv, "Eds:S:f:Pc:i:I:")) != -1)
    switch (c) {
    case 'E':
      runningUnderEmacs = 1;
      break;
    case 'd':
      tracerOn();
      break;
    case 's':
      port = atoi(optarg);
      compilerFile = (char *) NULL;
      break;
    case 'c':
      compilerFile = optarg;
      break;
    case 'S':
      comPath = optarg;
      break;
    case 'f':
      queryFileName = optarg;
      break;

    default:
      usage(argc,argv);
    }

  if (optind < argc) {
    usage(argc,argv);
  }

  int moreThanOne = 0;
  moreThanOne += (comPath != NULL);
  moreThanOne += (port != -1);
  moreThanOne += (queryFileName != NULL);
  if (moreThanOne > 1) {
     fprintf(stderr,"Specify only one of '-s' and '-f' and '-S'.\n");
     usage(argc,argv);
   }

  IO::initQuery(comPath,queryFileName,port,compilerFile);

  extern void DLinit(char *name);
  DLinit(argv[0]);

  initMemoryManagement();

// not changeable
  // SizeOfWorkingArea,NumberOfXRegisters,NumberOfYRegisters


// internal registers
  statusReg   = (StatusBit)0;
  xRegs       = allocateStaticRefsArray(NumberOfXRegisters);

  Board::Init();
  Thread::Init();

  // builtins
  BuiltinTabEntry *entry = BIinit();
  if (!entry) {
    error("BIinit failed");
    exit(1);
  }

  initAtoms();
  SolveActor::Init();

  toplevelVars = allocateRefsArray(GLOBAL_STORE_SIZE);

  Builtin *bi = new Builtin(entry,makeTaggedNULL());
  toplevelVars[0] = makeTaggedSRecord(bi);
  
  IO::init();
}

// ----------------------- unification

// unify and manage rebindTrail
Bool AM::unify(TaggedRef *ref1, TaggedRef *ref2)
{
  Bool result;
  result = performUnify(ref1, ref2);

  // unwindRebindTrail
  TaggedRef *refPtr;
  TaggedRef value;
  
  while (!rebindTrail.empty ()) {
    rebindTrail.popCouple(refPtr, value);
    doBind(refPtr,value);
  }

  return result;
}

Bool AM::performUnify(TaggedRef *termPtr1, TaggedRef *termPtr2)
{
  int currArg;

  DEREFPTR(termPtr1,term1,tag1);
  DEREFPTR(termPtr2,term2,tag2);

// detect identical variables or equal terms
  if ( isUVar(tag1) ) {  // distinct variables created in the
                         // same blackboard have the same image !!!!
                         // (the same home pointer), that's why  
    if (termPtr1 == termPtr2) // check the location.
      return OK;            
  } else  
    if ( term1 == term2 ) {   // CVars and other terms can be uniquely 
                              // identified by their value.
      return OK;
  }

// unify destinct variables
  if ( isNotCVar(tag1) ) {
    if ( isNotCVar(tag2) && isLocalVariable(term2) ) {
      /* if the second reference is local unbound VARiable,   */
      /* then bind this var to the first reference.           */

      /* if we want to bind two local variables to each other 
       * we bind the newer one to the older one. So variable names
       * look nicer: help variables are bound more likely to
       * pretty variables.
       * Note also, that this helps shorten reference chains !!!!
       *
       * (term2 < term1) since we allocate upwards in memory chunks
       */

// !!! check here mm2
      if ((termPtr2 < termPtr1) && isLocalVariable(term1)) {
	bind ( termPtr1, term1, termPtr2);
      } else {
	bind ( termPtr2, term2, termPtr1);
      }
    } else {
      /* ... the first ref is unbound ref; bind it.      */
      bind ( termPtr1, term1, termPtr2);
    } // ( isNotCVar(tag2) && isLocalVariable(term2) )
    return OK;
  } // ( isNotCVar(tag1) )
  
  if ( isNotCVar (tag2) ) {
    bind ( termPtr2, term2, termPtr1);
    return OK;
  }


// unification involving at least one generic variable
  if(tag2 == CVAR) {
    GenCVariable::unifyGenCVariables =
      (GenCVariable::unifyGenCVariables == OK && tag1 == CVAR) ? OK : NO; 
    Bool ret = tagged2CVar(term2)->unify(termPtr2, term2, tag2,
					 termPtr1, term1, tag1);
    GenCVariable::unifyGenCVariables = OK;
    return ret;
  }
  if(tag1 == CVAR) {
    GenCVariable::unifyGenCVariables =
      (GenCVariable::unifyGenCVariables == OK && tag2 == CVAR) ? OK : NO;
    Bool ret = tagged2CVar(term1)->unify(termPtr1, term1, tag1,
					 termPtr2, term2, tag2);
    GenCVariable::unifyGenCVariables = OK;
    return ret;    
  }

// unify non-variable terms
      /* At this point there are two refs which point to a non-ref    */
      /* structures. We try to unify them.                            */
      /* There is the following assumption: we exchange the structure */
      /* at the higher address to the one at the lower address.       */


  if (tag1 != tag2) {
// #define TESTGEN
#ifdef TESTGEN
    if (tag1 == CONST) {
      ConstTerm *gen = tagged2Const(term1);
      return gen->unify(term2);
    }

    if (tag2 == CONST) {
      ConstTerm *gen = tagged2Const(term2);
      return gen->unify(term1);
    }
#endif
    return NO;
  }

// rational unification needs rebinding
  rebind (termPtr2, term1);

  switch ( tag1 ) {
  case CONST:
    {
      ConstTerm *gen = tagged2Const(term1);
      return gen->unify(term2);
    } 
  case LTUPLE:
    {
      TaggedRef *arg1 = tagged2LTuple(term1)->getRef();
      TaggedRef *arg2 = tagged2LTuple(term2)->getRef();
      if (!performUnify(arg1,arg2)) {
	if (isToplevel()) {
	  performUnify(arg1+1,arg2+1);
	}
	return NO;
      }
      return performUnify(arg1+1,arg2+1);
    }
    
  case STUPLE:
    {
      if ( ! tagged2STuple(term1)->compareFunctor(tagged2STuple(term2))) {
	/* i.e. there are different functors.    */
	return NO;
      }

      /* ... now we must reset the ref and try to unify args.       */
      int end = tagged2STuple(term1)->getSize ();
      Bool ret = OK;
      for ( currArg = 0;
	    currArg < end;
	    currArg++ ) {
	if ( !performUnify (tagged2STuple(term1)->getRef(currArg),
			    tagged2STuple(term2)->getRef(currArg))) {
	  ret = NO;
	  if (!isToplevel()) {
	    break;
	  }
	}
      }
      return ret;
    }
  case SRECORD:
    {
      if ( ! tagged2SRecord(term1)->compareFunctor(tagged2SRecord(term2))) {
	/* i.e. there are different functors.    */
	return NO;
      }
      /* ... now we must reset the ref and try to unify args.       */
      int end = tagged2SRecord(term1)->getArgsSize();
      Bool ret = OK;
      for ( currArg = 0;
	    currArg < end;
	    currArg++ ) {
	if ( !performUnify ( tagged2SRecord(term1)->getRef(currArg),
			     tagged2SRecord(term2)->getRef(currArg))) {
	  ret = NO;
	  if (!isToplevel()) {
	    break;
	  }
	}
      }
      return ret;
    }
  case ATOM:
    // atom unify if there pointers are equal
    return NO;

  case FLOAT:
  case BIGINT:
  case SMALLINT:
    /* numberEq assumes, that both arguments have the same tag !! */
    return numberEq(term1,term2);
    
  default:
	// every unification of other data types fails
    return NO;
  }
}

Bool AM::isBetween(Board *to, Board *varHome)
{
  for (Board *tmp = to->getBoardDeref();
       tmp != currentBoard;
       tmp = tmp->getParentBoard()->getBoardDeref()) {
    if (tmp == varHome) {
      return NO;
    }
    if (!tmp) {
      // mm2: dead detected
      return OK;
    }
  }
  return OK;
}

void updateExtSuspension(Board *varHome, Suspension *susp)
{
  if (am.setExtSuspension (varHome, susp) == OK) {
    susp->setExtSusp ();
  }
}

Bool AM::setExtSuspension (Board *varHome, Suspension *susp)
{
  Board *bb = currentSolveBoard;
  Bool wasFound = NO;
  while (bb != (Board *) NULL && bb != varHome) {
    DebugCheck ((bb == rootBoard),
		error ("the root board is reached in AM::setExtSuspensions"));
    bb->addSuspension (susp);
    wasFound = OK;
    bb = (bb->getParentBoard ())->getSolveBoard ();
  }
  return (wasFound);
}

Bool AM::checkExtSuspension (Suspension *susp)
{
  if (susp->isExtSusp () == OK) {
    Board *sb = susp->getNode ();
    DebugCheck ((sb == (Board *) NULL),
		error ("no board is found in AM::checkExtSuspension"));
    sb = sb->getSolveBoard ();

    Bool wasFound = (sb == (Board *) NULL) ? NO : OK; 
    while (sb != (Board *) NULL) {
      DebugCheck ((sb->isSolve () == NO),
		  error ("no solve board is found in AM::checkExtSuspension"));

      SolveActor *sa = CastSolveActor (sb->getActor ());
      if (sa->isStable () == OK) {
	Thread::ScheduleSolve (sb);
	// Note:
	//  The observation is that some actors which have imposed instability
        // could be discarded by reduction of other such actors. It means,
	// that the stability condition can not be COMPLETELY controlled by the 
	// absence of active threads; 
	// Note too:
	//  If the note is not yet stable, it means that there are other
	// external suspension(s) and/or threads. Therefore it need not be waked.
      }
      sb = (sb->getParentBoard ())->getSolveBoard ();
    }
    return (wasFound);
  } else {
    return (NO);
  }
}

void AM::incSolveThreads (Board *bb)
{
  // get the next "cluster"; 
  // no getBoardDeref () !!!
  while (bb != (Board *) NULL && bb->isCommitted () == OK)
    bb = bb->getBoard ();
  while (bb != (Board *) NULL && bb != rootBoard) {
//     DebugCheck ((bb == (Board *) NULL),
// 		error ("NULL board is reached in AM::incSolveThreads"));
    if (bb->isSolve () == OK) {
      SolveActor *sa = CastSolveActor (bb->getActor ());
      DebugCheck ((sa->getBoard () == (Board *) NULL),
		  error ("solve actor in abstraction (AM::incSolveThreads ())"));
      sa->incThreads ();
    }
    DebugCheck ((bb->isCommitted () == OK),
		error ("committed board in loop in AM::decSolveThreads ()"));
    bb = bb->getParentBoard ();
    while (bb != (Board *) NULL && bb->isCommitted () == OK)
      bb = bb->getBoard ();
  }
}

void AM::decSolveThreads (Board *bb)
{
  // get the next "cluster"; 
  // no getBoardDeref () !!!
  while (bb != (Board *) NULL && bb->isCommitted () == OK)
    bb = bb->getBoard ();
  while (bb != (Board *) NULL && bb != rootBoard) {
//     DebugCheck ((bb == (Board *) NULL),
// 		error ("NULL board is reached in AM::decSolveThreads"));
    if (bb->isSolve () == OK) {
      SolveActor *sa = CastSolveActor (bb->getActor ());
      if (sa->getBoard () != (Board *) NULL) {     // i.e. no abstraction; 
	sa->decThreads ();
	if (sa->isStable () == OK) {
	  Thread::ScheduleSolve (bb);
	}
      }
    }
    DebugCheck ((bb->isCommitted () == OK),
		error ("committed board in loop in AM::decSolveThreads ()"));
    bb = bb->getParentBoard ();
    while (bb != (Board *) NULL && bb->isCommitted () == OK)
      bb = bb->getBoard ();
  }
}

// ------------------------------------------------------------------------

// val is used because it may be a variable which must suspend.
//  if det X then ... fi
//  X = Y 
// --> if det Y then ... fi

SuspList* AM::checkSuspensionList(SVariable* var, TaggedRef taggedvar,
				  SuspList* suspList,
				  TaggedRef term, SVariable* rightVar)
{
  SuspList* retSuspList = NULL;
  
  while (suspList) {
    Suspension* susp = suspList->getElem();
    Board* n = susp->getNode()->getBoardDeref();

    // suspension points to an already reduced branch of the computation tree
    if (!n) {
      susp->markDead();
      (void) checkExtSuspension (susp);
      suspList = suspList->dispose();
      continue;
    }
    
    // suspension has already been marked 'dead' 
    if (susp->isDead()) {
      suspList = suspList->dispose();
      continue;
    }

    // already propagated susps remain in suspList
    if (susp->isPropagated() == NO) {      
      if ((suspList->checkCondition(taggedvar, term) == OK) &&
	  (susp->wakeUp(var, rightVar) == OK)){
        // dispose only non-resistant susps
	if (susp->isResistant() == NO) {
	  suspList = suspList->dispose();
	  continue;
	}
      }
    }
#ifdef DEBUG_CHECK
    else
      if (susp->isResistant() == NO)
	error("Propagated susp has to be resistant.");

    if (susp->isDead() == OK)
      error("Unexpected dead suspension.");
#endif
    
    // susp cannot be woken up therefore relink it
    SuspList *first = suspList;
    suspList = suspList->getNext();
    first->setNext(retSuspList);
    retSuspList = first;
  } // while
  
  return retSuspList;
}


void AM::awakeNode(Board *node)
{
  node = node->getBoardDeref();

  if (!node)
    return;

  node->removeSuspension();
  Thread::ScheduleWakeup(node, NO);    // diese scheiss-'meta-logik' Dinge!!!!
}

// exception from general rule that arguments are never variables!
//  term may be an 
void AM::genericBind(TaggedRef *varPtr, TaggedRef var,
		     TaggedRef *termPtr, TaggedRef term)
     /* bind var to term;         */  
{
  Suspension* susp;

/* termPtr == NULL means term is not a variable */

  /* first step: do suspensions */

#ifdef DEBUG_FD
  if(isCVar(var) == OK)
    error("Constrained variable not allowed here at %s:%d.",
	  __FILE__, __LINE__);
#endif
  
  if (isSVar(var)) {

    DEREF(term,zzz,tag);
// special case if second arg is a variable !!!!
    SVariable *svar = (termPtr && isNotCVar(tag)) ? 
      (taggedBecomesSuspVar(termPtr)) : NULL;
    // variables are passed as references
    checkSuspensionList(var, svar ? makeTaggedRef(termPtr) : term, svar);
  }

  /* second step: mark binding for non-local variable in trail;     */
  /* also mark such (i.e. this) variable in suspention list;        */
  if ( ! isLocalVariable(var) ) {

    (void) taggedBecomesSuspVar(varPtr);

// trail old value
    trail.pushRef(varPtr,*varPtr);

// check if term is also a variable
    // bug fixed by mm 25.08.92
    // we must add currentBoard to suspension list of second variable if
    // both variables are global
    // term must be deref\'ed because isAnyVar() should really check
    // is unbound variable

    if (termPtr) {  // term may be a variable ?
      DEREFPTR(termPtr,term1,_2);

      if (isAnyVar(term1) ) {
	(void) taggedBecomesSuspVar(termPtr);
	term = makeTaggedRef(termPtr);
      }
    }
  } else  { // isLocalVariable(var)
    if ( termPtr && isAnyVar(term) ) {
      term = makeTaggedRef(termPtr);
    }
  }

// term is set now correctly to a non variable TaggedRef
  doBind(varPtr,term);
}


/*
  install every board from the currentBoard to 'n'
  and move cursor to 'n'

  algm
    find common parent board of 'to' and 'currentBoard'
    deinstall until common parent (go upward)
    install (go downward)
  pre:
     'to' ist not deref'd
     'to' may be committed, failed or discarded
   ret:
     INST_OK         installation successful
     INST_FAILED     installation of board failed
     INST_REJECTED   'to' is failed or discarded board
     */
InstType AM::installPath(Board *to)
{
  to = to->getBoardDeref();
  if (!to) {
    return INST_REJECTED;
  }

  if (to->isInstalled()) {
    deinstallPath(to);
    return INST_OK;
  }

  DebugCheck(to == rootBoard,
	     error("AM::installPath: root node reached");
	     );

  InstType ret = installPath(to->getParentBoard());
  if (ret != INST_OK) {
    return ret;
  }

  Board::SetCurrent(to);
  to->setInstalled();

  trail.pushMark();
  if (!installScript(to->getScriptRef())) {
    return INST_FAILED;
  }
  return INST_OK;
}

void AM::reduceTrailOnUnitCommit()
{
  int numbOfCons = trail.chunkSize();

  Board *bb = currentBoard;

  bb->newScript(numbOfCons);

  for (int index = 0; index < numbOfCons; index++) {
    TaggedRef *refPtr;
    TaggedRef value;
    trail.popRef(refPtr,value);

    bb->setScript(index,refPtr,*refPtr);
    *refPtr = value;
  }
  trail.popMark();
}


void AM::reduceTrailOnSuspend()
{
  int numbOfCons = trail.chunkSize();

  Suspension *susp;
  Board *bb = currentBoard;

  // one single suspension for all
  
  bb->newScript(numbOfCons);
  if (numbOfCons > 0) {
    susp = new Suspension(bb);
  }
  
  for (int index = 0; index < numbOfCons; index++) {
    TaggedRef *refPtr;
    TaggedRef value;
    trail.popRef(refPtr,value);
    
    TaggedRef oldVal = makeTaggedRef(refPtr);
    DEREF(oldVal,ptrOldVal,tagOldVal);
    
    bb->setScript(index,refPtr,*refPtr);

    if (isAnyVar(oldVal)) {
      // local generic variables are allowed to occure here
      DebugCheck (isLocalVariable (oldVal) && isNotCVar(oldVal),
		  error ("the right var is local  and unconstrained");
		  return;);
      if(!isLocalVariable(oldVal)) { // add susps to global vars
	SVariable *svar = taggedBecomesSuspVar (ptrOldVal);
	svar->addSuspension (susp);
      }
    }
    tagged2SuspVar(value)->addSuspension(susp);

    *refPtr = value;
  }
  trail.popMark();
}


void AM::reduceTrailOnFail()
{
  int numbOfCons = trail.chunkSize();
  for (int index = 0; index < numbOfCons; index++) {
    TaggedRef *refPtr;
    TaggedRef value;
    trail.popRef(refPtr,value);
    *refPtr = value;
  }
  trail.popMark();
}

void AM::reduceTrailOnShallow(Suspension *susp,int numbOfCons)
{
  // one single suspension for all
  
  for (int i = 0; i < numbOfCons; i++) {
    TaggedRef *refPtr;
    TaggedRef value;
    trail.popRef(refPtr,value);
    *refPtr = value;

    DebugCheck(!isAnyVar(*refPtr),
	       error("Non-variable on trail"));
    
    SVariable *svar = taggedBecomesSuspVar(refPtr);
    svar->addSuspension(susp);
  }

  trail.popMark();
}

void AM::pushCall(Board *n, SRecord *def, int arity, RefsArray args)
{
  n->incSuspCount();
  ensureTaskStack();
  currentTaskStack->pushCall(n,def,args,arity);
}

void AM::pushDebug(Board *n, SRecord *def, int arity, RefsArray args)
{
  n->incSuspCount();
  ensureTaskStack();
  currentTaskStack->pushDebug(n, new OzDebug(def,arity,args));
}

void AM::pushTaskOutline(Board *n,ProgramCounter pc,
			 RefsArray y,RefsArray g,RefsArray x,int i)
{
  pushTask(n, pc, y, g, x, i);
}

#ifdef OUTLINE
#define inline
#include "am.icc"
#undef inline
#endif

// ---------------------------------------------------------------------
