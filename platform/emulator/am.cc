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

#include "sunproto.h"

#include "actor.hh"
#include "am.hh"
#include "bignum.hh"
#include "builtins.hh"
#include "debug.hh"
#include "genvar.hh"
#include "io.hh"
#include "misc.hh"
#include "records.hh"
#include "thread.hh"
#include "unify.hh"
#include "fdbuilti.hh"

AM am;

int AM::ProcessCounter;

int getenvDefault(char *envvar, int def)
{
  char *s = getenv(envvar);
  if (s) {
    int ret = atoi(s);
    message("Using %s=%d\n", envvar, ret);
    return ret;
  }
  return def;
}

void ConfigData::init() {
  ozPath		= OZ_PATH;
  linkPath		= OZ_PATH;
  printDepth		= PRINT_DEPTH;
  showFastLoad		= SHOW_FAST_LOAD;
  showForeignLoad	= SHOW_FOREIGN_LOAD;
  showIdleMessage	= SHOW_IDLE_MESSAGE;
  showSuspension	= SHOW_SUSPENSION;

  stopOnToplevelFailure = STOP_ON_TOPLEVEL_FAILURE;

  gcFlag		= GC_FLAG;
  gcVerbosity		= GC_VERBOSITY;
  heapMaxSize           = getenvDefault("OZHEAPMAXSIZE",HEAPMAXSIZE);
  heapMargin            = HEAPMARGIN;
  heapIncrement         = HEAPINCREMENT;
  heapIdleMargin        = HEAPIDLEMARGIN;


  timeSlice		= TIME_SLICE;
  defaultPriority	= DEFAULT_PRIORITY;
  systemPriority	= SYSTEM_PRIORITY;
  taskStackSize		= TASK_STACK_SIZE;
  errorVerbosity        = ERROR_VERBOSITY;
  dumpCore		= 0;
}

extern "C" int runningUnderEmacs; // mm2
extern void version(); // mm2


void usage(int /* argc */,char **argv) {
  fprintf(stderr,
	  "usage: %s [-E] [-S file | -f file] [-d] [-c compiler]\n",
	  argv[0]);
  exit(1);
}


char *getOptArg(int &i, int argc, char **argv)
{
  i++;
  if (i == argc) {
    usage(argc,argv);
    return NULL;
  }

  return argv[i];
}


static void printBanner()
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
#ifdef DEBUG_DET
  printf("Deterministic scheduling.\n");
#else
  printf("Time-slice scheduling.\n");
#endif  
#endif

#if THREADED == 1
  // printf("Using threaded code (rel jumps).\n");
#else
#if THREADED == 2
  // printf("Using threaded code (abs jumps).\n");
#else
  printf("Not using threaded code.\n");
#endif
#endif
}

void AM::init(int argc,char **argv)
{  
  conf.init();

#ifdef MAKEANEWPGRP
  // create a new process group, so that we can
  // easily terminate all our children
  if (setpgrp(getpid(),getpid()) < 0) {
    ozperror("setpgid");
  }
#endif

  int c;

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
  Bool quiet = FALSE;
  
  /* process command line arguments */
  conf.argV = NULL;
  conf.argC = 0;
  for (int i=1; i<argc; i++) {
    if (strcmp(argv[i],"-E")==0) {
      runningUnderEmacs = 1;
      continue;
    }
    if (strcmp(argv[i],"-d")==0) {
      tracerOn();
      continue;
    }
    if (strcmp(argv[i],"-quiet")==0) {
      quiet = TRUE;
      continue;
    }
    if (strcmp(argv[i],"-c")==0) {
      compilerFile = getOptArg(i,argc,argv);
      continue;
    }
    if (strcmp(argv[i],"-S")==0) {
      comPath = getOptArg(i,argc,argv);
      continue;
    }
    if (strcmp(argv[i],"-f")==0) {
      queryFileName = getOptArg(i,argc,argv);
      continue;
    }

    if (strcmp(argv[i],"-a")==0) {
      conf.argC = argc-i-1;
      conf.argV = argv+i+1;
      break;
    }

    usage(argc,argv);
  }

  int moreThanOne = 0;
  moreThanOne += (comPath != NULL);
  moreThanOne += (queryFileName != NULL);
  if (moreThanOne > 1) {
     fprintf(stderr,"Specify only one of '-s' and '-f' and '-S'.\n");
     usage(argc,argv);
   }

  if (quiet == FALSE) {
    printBanner();
  }
  
  IO::initQuery(comPath,queryFileName,compilerFile);

  extern void DLinit(char *name);
  DLinit(argv[0]);

  BIfdHeadManager::initStaticData();
  BIfdBodyManager::initStaticData();
  
  initMemoryManagement();

// not changeable
  // SizeOfWorkingArea,NumberOfXRegisters,NumberOfYRegisters


// internal registers
  statusReg    = (StatusBit)0;
  criticalFlag = NO;
  xRegs        = allocateStaticRefsArray(NumberOfXRegisters);

  rootBoard = new Board(NULL,Bo_Root);
  rootBoard->setInstalled();
  currentBoard = NULL;
  setCurrent(rootBoard,OK);
  currentSolveBoard = (Board *) NULL; 
  wasSolveSet = NO; 

  Thread::Init();

  // builtins
  BuiltinTabEntry *entry = BIinit();
  if (!entry) {
    error("BIinit failed");
    exit(1);
  }

  initLiterals();
  initTagged();
  SolveActor::Init();

  int numToplevelVars = getenvDefault("OZTOPLEVELVARS",NUM_TOPLEVEL_VARS);
  toplevelVars = allocateRefsArray(numToplevelVars);

  Builtin *bi = new Builtin(entry,makeTaggedNULL());
  toplevelVars[0] = makeTaggedSRecord(bi);
  
  IO::init();
#ifdef DEBUG_CHECK
  dontPropagate = NO;
#endif
}

// ----------------------- unification

// unify and manage rebindTrail
Bool AM::unify(TaggedRef t1, TaggedRef t2, Bool prop)
{
  CHECK_NONVAR(t1); CHECK_NONVAR(t2);
  Bool result = performUnify(&t1, &t2, prop);

  // unwindRebindTrail
  TaggedRef *refPtr;
  TaggedRef value;
  
  while (!rebindTrail.isEmpty ()) {
    rebindTrail.popCouple(refPtr, value);
    doBind(refPtr,value);
  }

  LOCAL_PROPAGATION(Assert(localPropStore.isEmpty() ||
			   localPropStore.isInLocalPropagation()));

  return result;
}


/* we prefer binding of newer variables to older ones */
inline Bool heapNewer(TaggedRef *termPtr1, TaggedRef *termPtr2)
{
  return termPtr1 > termPtr2;
}

#define Swap(A,B,Type) { Type help=A; A=B; B=help; }

Bool AM::performUnify(TaggedRef *termPtr1, TaggedRef *termPtr2, Bool prop)
{
  int argSize;
  RefsArray args1, args2;

  LOCAL_PROPAGATION(Assert(localPropStore.isEmpty() ||
			   localPropStore.isInLocalPropagation()));

start:

  DEREFPTR(termPtr1,term1,tag1);
  DEREFPTR(termPtr2,term2,tag2);

  // identical terms ?
  if (term1 == term2 &&
      (!isUVar(term1) || termPtr1 == termPtr2)) {
    return OK;
  }

  if (isAnyVar(term1)) {
    if (isAnyVar(term2)) {
      goto var_var;
    } else {
      goto var_nonvar; 
    }
  } else {
    if (isAnyVar(term2)) {
      Swap(term1,term2,TaggedRef);
      Swap(termPtr1,termPtr2,TaggedRef*);
      Swap(tag1,tag2,TypeOfTerm);
      goto var_nonvar;
    } else {
      goto nonvar_nonvar;
    }
  }
    

 /*************/
 var_nonvar:

  if (isCVar(tag1)) {
    return tagged2CVar(term1)->unify(termPtr1, term1, termPtr2, term2, prop);
  }
  
  bindToNonvar(termPtr1, term1, term2, prop);
  return OK;


  
 /*************/
 var_var:

  /* prefer binding nonCVars to CVars */
  if (isNotCVar(tag1)) {
    if ( isNotCVar(tag2) && isLocalVariable(term2) &&
	 (!isLocalVariable(term1) || heapNewer(termPtr2,termPtr1))) {
      bind(termPtr2, term2, termPtr1, prop);
    } else {
      bind(termPtr1, term1, termPtr2, prop);
    }
    return OK;
  }
  
  if (isNotCVar(tag2)) {
    bind(termPtr2, term2, termPtr1, prop);
    return OK;
  }

  Assert(isCVar(tag1) && isCVar(tag2));
  return tagged2CVar(term1)->unify(termPtr1,term1,termPtr2,term2,prop);



 /*************/
 nonvar_nonvar:

  if (tag1 != tag2) {
    return NO;
  }

  switch ( tag1 ) {

  case CONST:
    return tagged2Const(term1)->unify(term2);

  case LTUPLE:
    {
      args1 = tagged2LTuple(term1)->getRef();
      args2 = tagged2LTuple(term2)->getRef();
      argSize = 2;
      goto unify_args;
    }

  case STUPLE:
    {
      STuple *st1 = tagged2STuple(term1);
      STuple *st2 = tagged2STuple(term2);

      if ( ! st1->compareFunctor(st2) ) {
	return NO;
      }

      argSize = st1->getSize();
      args1 = st1->getRef();
      args2 = st2->getRef();

      goto unify_args;
    }
      
  case SRECORD:
    {
      SRecord *sr1 = tagged2SRecord(term1);
      SRecord *sr2 = tagged2SRecord(term2);

      if (! sr1->compareFunctor(sr2)) {
	return NO;
      }

      argSize = sr1->getArgsSize();
      args1 = sr1->getRef();
      args2 = sr2->getRef();

      goto unify_args;
    }

  case LITERAL:
    /* literals unify if their pointers are equal */
    return NO;

  case FLOAT:
  case BIGINT:
  case SMALLINT:
    return numberEq(term1,term2);
    
  default:
    return NO;
  }


 /*************/
 unify_args:

  rebind (termPtr2, term1);

  for (int i = 0; i < argSize-1; i++ ) {
    if (!performUnify(args1+i,args2+i, prop)) {
      return NO;
    }
  }

  /* tail recursion optimization */
  termPtr1 = args1+argSize-1;
  termPtr2 = args2+argSize-1;
  goto start;
}





// mm2: has to be optimzed: ask rs
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

void AM::setExtSuspension (Board *varHome, Suspension *susp)
{
  Board *bb = currentBoard;
  Bool wasFound = NO;
  DebugCheck ((varHome->isCommitted () == OK),
	      error ("committed board as the varHome in AM::setExtSuspension"));
  while (bb != varHome) {
    DebugCheck ((bb == rootBoard),
		error ("the root board is reached in AM::setExtSuspensions"));
    if (bb->isSolve () == OK) {
      bb->addSuspension (susp);
      wasFound = OK;
    }
    bb = (bb->getParentBoard ())->getBoardDeref ();
  }
  if (wasFound == OK)
    susp->setExtSusp ();
}

// expects susp to be external
Bool AM::_checkExtSuspension (Suspension *susp)
{
  Assert(susp->isExtSusp());
  
  Board *sb = susp->getBoard ();
  DebugCheck ((sb == (Board *) NULL),
	      error ("no board is found in AM::checkExtSuspension"));
  sb = sb->getSolveBoard ();
  
  Bool wasFound = (sb == (Board *) NULL) ? NO : OK; 
  while (sb != (Board *) NULL) {
    DebugCheck ((sb->isSolve () == NO),
		error ("no solve board is found in AM::checkExtSuspension"));
    
    SolveActor *sa = SolveActor::Cast (sb->getActor ());
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
}

void AM::incSolveThreads (Board *bb)
{
  // get the next "cluster"; 
  // no getBoardDeref () !!!
  while (bb != (Board *) NULL && bb->isCommitted () == OK)
    bb = bb->getBoard ();
  while (bb != (Board *) NULL && bb != rootBoard) {
    if (bb->isSolve () == OK) {
      DebugCheck ((bb->isReflected () == OK),
		  error ("reflected board is found in AM::incSolveThreads ()"));
      SolveActor *sa = SolveActor::Cast (bb->getActor ());
      DebugCheck ((sa->getBoard () == (Board *) NULL),
		  error ("solve actor in abstraction (AM::incSolveThreads ())"));
      sa->incThreads ();
    }
    bb = bb->getParentBoard ();
    while (bb != (Board *) NULL && bb->isCommitted () == OK)
      bb = bb->getBoard ();
  }
}

void AM::decSolveThreads (Board *bb)
{
  while (bb != (Board *) NULL && bb->isCommitted () == OK)
    bb = bb->getBoard ();
  while (bb != (Board *) NULL && bb != rootBoard) {
    if (bb->isSolve () == OK) {
      DebugCheck ((bb->isReflected () == OK),
		  error ("reflected board is found in AM::decSolveThreads ()"));
      SolveActor *sa = SolveActor::Cast (bb->getActor ());
      sa->decThreads ();
      if (sa->isStable () == OK) {
	Thread::ScheduleSolve (bb);
      }
    }
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

SuspList * AM::checkSuspensionList(SVariable * var, TaggedRef taggedvar,
				   SuspList * suspList,
				   TaggedRef term,
				   PropCaller calledBy)
{
  SuspList * retSuspList = NULL;

  // see the reduction of solve actor by the enumeration; 
  DebugCheck(dontPropagate == OK, return (suspList));

  
  while (suspList) {
    Suspension * susp = suspList->getElem();

    if (susp->isDead()) {
      suspList = suspList->dispose();
      continue;
    }

    // suspension points to an already reduced branch of the computation tree
    if (! susp->getBoard()->getBoardDeref()) {
      susp->markDead();
      checkExtSuspension (susp);
      suspList = suspList->dispose();
      continue;
    }
    
PROFILE_CODE1
  (
   if (var->getHome() == am.currentBoard) {
     if (susp->getBoard()->getBoardDeref() == am.currentBoard)
       FDProfiles.inc_item(from_home_to_home_hits); 
     else
       FDProfiles.inc_item(from_home_to_deep_hits);
   } else {
     Board * b = susp->getBoard()->getBoardDeref();
     if (b == var->getHome())
       FDProfiles.inc_item(from_deep_to_home_misses);
     else if (am.isBetween(b, var->getHome()))
       FDProfiles.inc_item(from_deep_to_deep_hits);
     else
       FDProfiles.inc_item(from_deep_to_deep_misses);
   }
   )
  
  // already propagated susps remain in suspList
    if (! susp->isPropagated()) {      
      if ((suspList->checkCondition(taggedvar, term)) &&
	  (susp->wakeUp(var->getHome(), calledBy))) {
        // dispose only non-resistant susps
	if (! susp->isResistant()) {
	  suspList = suspList->dispose();
	  continue;
	} else if (calledBy) {
	  susp->markUnifySusp();
	}
      }
    } else if (calledBy && susp->isResistant() && ! susp->isUnifySusp())
      if (isBetween(susp->getBoard(), var->getHome()))
	susp->markUnifySusp();
    
    // susp cannot be woken up therefore relink it
    SuspList * first = suspList;
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
		     TaggedRef *termPtr, TaggedRef term,
		     Bool prop)
     /* bind var to term;         */  
{
  /* termPtr == NULL means term is not a variable */

  /* first step: do suspensions */

  Assert(isCVar(var) == NO);
  
  if (prop && isSVar(var)) {

    DEREF(term,zzz,tag);
   // special case if second arg is a variable !!!!
    SVariable *svar = (termPtr && isNotCVar(tag)) ? 
      (taggedBecomesSuspVar(termPtr)) : NULL;
    // variables are passed as references
    checkSuspensionList(var, svar ? makeTaggedRef(termPtr) : term, pc_std_unif);

    LOCAL_PROPAGATION(Assert(localPropStore.isEmpty() ||
			     localPropStore.isInLocalPropagation()));

#ifdef DEBUG_CHECK
    Board *hb = (tagged2SuspVar(var)->getHome ())->getBoardDeref ();
    if (hb->isReflected () == OK)
      error ("the variable from reflected board is bound");
    Board *sb = hb->getSolveBoard ();
    if (sb != (Board *) NULL && sb->isReflected () == OK)
      error ("the variable in reflected search problem is bound");
#endif
  }
#ifdef DEBUG_CHECK
  if (isUVar (var)) {
    Board *hb = (tagged2VarHome (var))->getBoardDeref ();
    if (hb->isReflected () == OK)
      error ("UVar from reflected board is bound???");
  }
#endif
  
  /* second step: mark binding for non-local variable in trail;     */
  /* also mark such (i.e. this) variable in suspention list;        */
  if ( !isLocalVariable(var) || prop==NO ) {

    (void) taggedBecomesSuspVar(varPtr);

    /* trail old value */
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
    if (isSVar(var)) {
      tagged2SVar(var)->dispose();
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

  setCurrent(to);
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
#ifdef DEBUG_CHECK
    TaggedRef aux = value;
    DEREF(aux, ptr, tag);
    if (isAnyVar (tag) == NO)
      error ("non-variable is found in AM::reduceTrailOnUnitCommit ()");
    if (isUVar (tag) == OK)
      error ("UVar is found as value in trail;");
#endif
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

  susp = (numbOfCons > 0) ? new Suspension(bb) : NULL;
  
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
      // add susps to global non-cvars
      if(!isLocalVariable(oldVal) && isNotCVar(tagOldVal)) 
	taggedBecomesSuspVar(ptrOldVal)->addSuspension (susp);
    }
#ifdef DEBUG_CHECK
    TaggedRef aux = value;
    DEREF(aux, ptr, tag);
    if (isAnyVar (tag) == NO)
      error ("non-variable is found in AM::reduceTrailOnSuspend ()");
    if (isUVar (tag) == OK)
      error ("UVar is found as value in trail;");
#endif
    if (isNotCVar(value))
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
/* 
 *  Don't works??? Sometimes the machine gets UVars ???
#ifdef DEBUG_CHECK
    TaggedRef aux = value;
    DEREF(aux, ptr, tag);
    if (isAnyVar (tag) == NO)
      error ("non-variable is found in AM::reduceTrailOnFail ()");
    if (isUVar (tag) == OK)
      error ("UVar is found as value in trail;");
#endif
 */
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

    TaggedRef oldVal = makeTaggedRef(refPtr);
    DEREF(oldVal,ptrOldVal,_1);
    *refPtr = value;

    taggedBecomesSuspVar(refPtr)->addSuspension(susp);
    if (isAnyVar(oldVal)) {
      taggedBecomesSuspVar(ptrOldVal)->addSuspension(susp);
    }
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

#ifdef DEBUG_CHECK
static Board *oldBoard = (Board *) NULL;
static Board *oldSolveBoard = (Board *) NULL; 
#endif

void AM::setCurrent(Board *c, Bool checkNotGC)
{
  Assert(c!=NULL);
  DebugCheck ((c->isCommitted () == OK),
	      error ("committed board in AM::setCurrent ()"));
  DebugCheck(checkNotGC && oldBoard != currentBoard,
	     error("someone has changed 'currentBoard'"));
  currentBoard = c;
  currentUVarPrototype = makeTaggedUVar(c);
  DebugCheckT(oldBoard=c);

  if (c->isSolve () == OK) {
    DebugCheck ((checkNotGC && oldSolveBoard != currentSolveBoard),
		error ("somebody has changed 'currentSolveBoard'"));
    currentSolveBoard = c;
    wasSolveSet = OK; 
    DebugCheckT (oldSolveBoard = c); 
  } else if (wasSolveSet == OK) {
    DebugCheck ((checkNotGC && oldSolveBoard != currentSolveBoard),
		error ("somebody has changed 'currentSolveBoard'"));
    currentSolveBoard = c->getSolveBoard ();
    wasSolveSet = NO;
    DebugCheckT (oldSolveBoard = currentSolveBoard); 
  }
}



#ifdef FASTSS
Bool AM::fastUnifyOutline(TaggedRef term1, TaggedRef *term1Ptr, TaggedRef term2)
{
  SVariable *svar;
  TaggedRef term;
  TaggedRef *varPtr;

  if (isSVar(term1) && isLocalVariable(term1)) {
    svar = tagged2SVar(term1);
    varPtr = term1Ptr;
    term = term2;
  } else {
    DEREF(term2,term2Ptr,_1);
    if (isSVar(term2) && isLocalVariable(term2)) {
      svar = tagged2SVar(term2);
      varPtr = term2Ptr;
      term = makeTaggedRef(term1Ptr?term1Ptr:term1);
    } else {
      if (term1Ptr)
	return unify(term1Ptr,term2);
      else
	return unify(term1,term2);
    }
  }

  for (SuspList* sl = svar->getSuspList(); sl; 	sl = sl->dispose()) {

    Suspension* susp = sl->getElem();
    
    if (susp->isDead()) {	
      continue;
    }
    
    if (susp->getNode()->getBoardDeref() == NULL) {
      susp->markDead();
      continue;
    }
    
    switch (susp->getFlag() & (S_cont|S_cfun)) {
    case S_null:
      susp->wakeUpNode(svar);
      break;
    case S_cont:
      {
	susp->markDead();
	Thread::ScheduleSuspCont(susp->getCont(), NO);
	break;
      }
    case S_cont|S_cfun:
      susp->wakeUpCCont(svar);
      break;
    }
  }
    
  svar->dispose();
  
  doBind(varPtr,term);
}
#endif


#ifdef OUTLINE
#define inline
#include "am.icc"
#undef inline
#endif



// ---------------------------------------------------------------------
