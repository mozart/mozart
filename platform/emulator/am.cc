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

#include "dldwrapper.h"
#include "sun-proto.h"

#include "am.hh"
#include "bignum.hh"
#include "builtins.hh"
#include "genvar.hh"
#include "misc.hh"
#include "records.hh"
#include "thread.hh"
#include "tracer.hh"

#ifdef OUTLINE
#define inline
#endif

FILE *openCompiler(int port, char *compiler);
FILE *connectCompiler(char *path);

extern "C" int runningUnderEmacs;
extern void version();


void usage(int /* argc */,char **argv) {
  fprintf(stderr,
	  "usage: %s [-E] [-s port | -S file | -f file] [-d] [-c compiler]\n",
	  argv[0]);
  exit(1);
}

AM am;


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

  if (!(compilerFile = getenv("OZCOMPILER"))) {
    compilerFile = OzCompiler;
  }

  if (!(ozPath = getenv("OZPATH"))) {
    ozPath = OzPath;
  }

  if (!(linkPath = getenv("OZLINKPATH"))) {
    linkPath = ozPath;
  }


  char *comPath = NULL;  // path name where to create AF_UNIX socket

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
      QueryFileName = optarg;
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
  moreThanOne += (QueryFileName != NULL);
  if (moreThanOne > 1) {
     fprintf(stderr,"Specify only one of '-s' and '-f' and '-S'.\n");
     usage(argc,argv);
   }

  if (comPath != NULL) {
    QueryFILE = connectCompiler(comPath);
  } else if (QueryFileName != NULL) {
    QueryFILE = fopen(QueryFileName,"r");
    if (QueryFILE == NULL) {
      char buf [1000];
      sprintf(buf, "Cannot open query file '%s'",QueryFileName);
      perror(buf);
      exit(1);
    }
    int skipLines = 2;
    int c;
    while ((c = getc(QueryFILE)) != EOF) {
      if ((c == '\n') && --skipLines == 0)
	break;
    }
  } else {
    QueryFILE = openCompiler(port,compilerFile);
  }

  if (QueryFILE == NULL) {
    fprintf(stderr,"Cannot open code input\n");	
    exit(1);
  }

#ifdef DLD
  if (dld_init (dld_find_executable (argv[0])) != 0) {
    dld_perror("Failed to initialize DLD");
    exit(1);
  }
#endif

#ifdef XDL
  /* argv[0] should be a full pathname */
  if (xdl_init(argv[0]) != 0) {
    error("Failed to initialize XDL");
    exit(1);
  }
#endif


  initMemoryManagement();

// user configurable parameters
  printDepthVal = 10;
  fastLoad      = NO;
  foreignLoad   = NO;
  idleMessage   = NO;
  
  gcFlag = OK;
  gcVerbosity = 1;

  timeSinceLastIdle = usertime();
  timeForGC = timeForCopy = timeForLoading = 0;
  heapAllocated = 0;
  
  clockTick = InitialClockTick;
  DebugCheck(clockTick < 1000, error("clockTick must be greater than 1 ms"));
// not changeable
  // SizeOfWorkingArea,NumberOfXRegisters,NumberOfYRegisters


// internal registers
  statusReg   = (StatusBit)0;
  xRegs       = allocateStaticRefsArray(NumberOfXRegisters);
  globalStore = allocateStaticRefsArray(NumberOfYRegisters);

  Board::Init();
  Thread::Init();

  currentTaskSusp = NULL;
  currentTaskStack = NULL;

  // builtins
  BuiltinTabEntry *entry = BIinit();
  if (!entry) {
    error("BIinit failed");
    exit(1);
  }
  Builtin *bi = new Builtin(entry,makeTaggedNULL());
  globalStore[0] = makeTaggedSRecord(bi);
  SVariable *svar = new SVariable(Board::GetRoot(),OZ_stringToTerm("TopLevelFailed"));
  globalStore[1] =  makeTaggedRef(newTaggedSVar(svar));


  initAtoms();
  
  initSignal();

  initIO();

  acknowledgeCompiler(OK);

  // --> make sure that we check input from compiler
  setSFlag(IOReady);  
}



void AM::exitOz(int status) {

  fclose(QueryFILE);

  waitCompiler();

  statusMessage("halted");

  // terminate all our children
  ozSignal(SIGTERM,(Sigfunc*)SIG_IGN);
  if (kill(-getpid(),SIGTERM) < 0) {
    perror("kill");
  }

  exit(status);
}


// ----------------------- unification

// optimize: inline ....
inline void AM::rebind(TaggedRef *refPtr, TaggedRef newRef)
/* (re)bind Ref *ref to Term *ptr;                     */
/* We need this procedure for the rational unification */
/* algorithm;                                          */
{
  rebindTrail.pushCouple(refPtr, *refPtr);   /* always;   */
  doBind(refPtr,newRef);
}


// unify and manage rebindTrail
Bool AM::unify(TaggedRef *ref1, TaggedRef *ref2)
{
  Bool result;
  result = performUnify(ref1, ref2);

  // unwindRebindTrail
  TaggedRef *refPtr;
  TaggedRef value;
  
  while (!rebindTrail.isEmpty ()) {
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
       tmp != Board::GetCurrent();
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
    retSuspList = addSuspension(retSuspList, first);
  } // while
  
  return retSuspList;
}


void AM::awakeNode(Board *node)
{
  node = node->getBoardDeref();

  if (!node)
    return;

  node->removeSuspension();
  Thread::Schedule(node);
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
    // we must add Board::Current to suspension list of second variable if
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
  install every board from the Board::Current to 'n'
  and move cursor to 'n'

  algm
    find common parent board of 'to' and 'Board::Current'
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

  DebugCheck(to == Board::GetRoot(),
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


inline void AM::deinstallPath(Board *top)
{
  DebugCheck(top->isCommitted(),
	     error("AM::deinstallPath: top already commited");
	     return;);
  
  while (Board::GetCurrent() != top) {
    deinstallCurrent();
    DebugCheck(Board::GetCurrent() == Board::GetRoot()
	       && top != Board::GetRoot(),
	       error("AM::deinstallPath: root node reached"));
  }
}

inline void AM::deinstallCurrent()
{
  reduceTrailOnSuspend();
  Board::GetCurrent()->unsetInstalled();
  Board::SetCurrent(Board::GetCurrent()->getParentBoard()->getBoardDeref());
}

void AM::reduceTrailOnUnitCommit()
{
  int numbOfCons = trail.chunkSize();

  Board *bb = Board::GetCurrent();

  bb->newScript(numbOfCons);

  for (int index = 0; index < numbOfCons; index++) {
    TaggedRef *refPtr;
    TaggedRef value;
    {
      TrailEntry *eq = trail.popRef();
      refPtr = eq->getRefPtr();
      value = eq->getValue ();
    }

    bb->setScript(index,refPtr,*refPtr);
    *refPtr = value;
  }
  trail.popMark();
}


void AM::reduceTrailOnSuspend()
{
  int numbOfCons = trail.chunkSize();

  Suspension *susp;
  Board *bb = Board::GetCurrent();

  bb->newScript(numbOfCons);
  if (numbOfCons > 0) {
    susp = new Suspension(bb);
  }
  
  for (int index = 0; index < numbOfCons; index++) {
    TaggedRef *refPtr;
    TaggedRef value;
    {
      TrailEntry *eq = trail.popRef();
      refPtr = eq->getRefPtr();
      value = eq->getValue ();
    }
    
    TaggedRef oldVal = makeTaggedRef(refPtr);
    DEREF(oldVal,ptrOldVal,tagOldVal);
    
    bb->setScript(index,refPtr,*refPtr);

    if (isAnyVar(oldVal)) {
      // local generic variables are allowed to occure here
      DebugCheck (isLocalVariable (oldVal) && isNotCVar(oldVal),
		  error ("the right var is local  and unconstrained");
		  return;);
      if(!isLocalVariable(oldVal)) // add susps to global vars
	taggedBecomesSuspVar(ptrOldVal)->addSuspension(susp);
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
    {
      TrailEntry *eq = trail.popRef();
      refPtr = eq->getRefPtr();
      value = eq->getValue ();
    }
    *refPtr = value;
  }
  trail.popMark();
}

void AM::reduceTrailOnShallow(Suspension *susp,int numbOfCons)
{
  Bool isOuterSuspMaint = NO;

  // one single suspension for all
  
  for (int i = 0; i < numbOfCons; i++) {
    TrailEntry *eq = trail.popRef();
    TaggedRef *refPtr = eq->getRefPtr();
    // unbind
    *refPtr = eq->getValue();

    DebugCheck(!isAnyVar(*refPtr),
	       error("Non-variable on trail"));
    
    SVariable *svar = taggedBecomesSuspVar(refPtr);
    svar->addSuspension(susp);
  }

  trail.popMark();
}



/* specially optimized unify: test two most probable cases first:
 *
 *     1. bind a variable
 *     2. test two non-variables
 *     3. but don't forget to check identical variables
 */

inline Bool AM::fastUnify(TaggedRef A, TaggedRef B)
{

  TaggedRef term1 = A;
  TaggedRef term2 = B;

  DEREF(term1,term1Ptr,_1);
  DEREF(term2,term2Ptr,_2);

  if (term1Ptr == term2Ptr && term1Ptr != NULL)
    return OK;

  TaggedRef proto = currentUVarPrototype;
  if (proto == term1) {
    doBind(term1Ptr,B);
    return OK;
  }
  

  if (proto == term2) {
    doBind(term2Ptr,A);
    return OK;
  }

  return unify(A,B);
}


// unify two non derefed tagged refs
inline Bool AM::unify(TaggedRef ref1, TaggedRef ref2)
{
  CHECK_NONVAR(ref1);
  CHECK_NONVAR(ref2);

  return unify(&ref1, &ref2);
}

inline Bool AM::unify(TaggedRef *ref1, TaggedRef ref2)
{
  CHECK_NONVAR(ref2);

  return unify(ref1, &ref2);
}


inline Bool AM::installScript(ConsList &script)
{
  Bool ret = OK;
  for (int index = 0; index < script.getSize(); index++) {
    if (!unify(script[index].getLeft(),script[index].getRight())) {
      ret = NO;
      if (!isToplevel()) {
	break;
      }
    }
  }
  script.dealloc();
  return ret;
}

inline void AM::pushTask(Board *n,ProgramCounter pc,
			 RefsArray y,RefsArray g,
			 RefsArray x,int i)
{
  n->addSuspension();
  if (!currentTaskStack) {
    Thread::MakeTaskStack();
  }
  currentTaskStack->pushCont(n,pc,y,g,x,i);
}


/*
 *   Procedure what checks whether one node is in subtree of another;
 *
 */
inline Bool AM::isInScope (Board *above, Board* node) {
  while (node != Board::GetRoot()) {
    if (node == above)
      return (OK);
    node = node->getParentBoard()->getBoardDeref();
  }
  return (NO);
}


inline Bool AM::isLocalUVar(TaggedRef var)
{
  return (var == currentUVarPrototype ||
	  // variables are usually bound 
	  // in the node where they are created
	  tagged2VarHome(var)->getBoardDeref() == Board::GetCurrent() )
    ? OK : NO;
}

inline Bool AM::isLocalSVar(TaggedRef var) {
  Board *home = tagged2SVar(var)->getHome1();

  return (home == Board::GetCurrent() ||
	  home->getBoardDeref() == Board::GetCurrent() )
    ? OK : NO;
}

inline Bool AM::isLocalCVar(TaggedRef var) {
  Board *home = tagged2CVar(var)->getHome1();

  return (home == Board::GetCurrent() ||
          home->getBoardDeref() == Board::GetCurrent() )
    ? OK : NO;
}

inline Bool AM::isLocalVariable(TaggedRef var)
{
  CHECK_ISVAR(var);

  if (isUVar(var))
    return isLocalUVar(var);
  if (isSVar(var))
    return isLocalSVar(var);

  return isLocalCVar(var);
}

inline void AM::checkSuspensionList(TaggedRef taggedvar, TaggedRef term,
				    SVariable *rightVar)
{
  SVariable* var = tagged2SuspVar(taggedvar);
  var->concSuspList(checkSuspensionList(var, taggedvar,
					var->unlinkSuspension(),
					term, rightVar));
}


inline void AM::reviveCurrentTaskSusp(void)
{
  DebugCheck(currentTaskSusp == NULL,
	     error("currentTaskSusp is NULL in AM::reviveCurrentTaskSusp."));
  DebugCheck(currentTaskSusp->isResistant() == NO,
	     error("Cannot revive non-resistant suspension."));
  DebugCheck(currentTaskSusp->isDead() == OK,
	     error("Cannot revive non-resistant suspension."));
  currentTaskSusp->unmarkPropagated();
  currentTaskSusp->setNode(Board::GetCurrent());
  Board::GetCurrent()->addSuspension();
}

inline void AM::killPropagatedCurrentTaskSusp(void) {
  if (currentTaskSusp == NULL) return;
  if (currentTaskSusp->isPropagated() == NO) return;
  
  DebugCheck(currentTaskSusp->isResistant() == NO,
	     error("Cannot kill non-resistant suspension."));
  currentTaskSusp->markDead();
}

void AM::dismissCurrentTaskSusp(void) {
  currentTaskSusp->cContToNode(Board::GetCurrent());
  currentTaskSusp = NULL;
}

inline Bool AM::entailment ()
{
  return (!Board::GetCurrent()->hasSuspension()
	  // First test: no subtrees;
	  && trail.isEmptyChunk()
	  // second test: is this node stable?
	  ? OK : NO);
}

inline Bool AM::isEmptyTrailChunk ()
{
  return (trail.isEmptyChunk ());
}

inline Bool AM::isToplevel() {
  return Board::GetCurrent() == Board::GetRoot() ? OK : NO;
}

inline void AM::bind(TaggedRef *varPtr, TaggedRef var, TaggedRef *termPtr)
{
  genericBind(varPtr,var,termPtr, *termPtr);
}

inline void AM::bindToNonvar(TaggedRef *varPtr, TaggedRef var, TaggedRef a) 
{
  // most probable case first: local UVar
  // if (isUVar(var) && Board::GetCurrent() == tagged2VarHome(var)) {
  // more efficient:
  if (var == currentUVarPrototype) {
    doBind(varPtr,a);
  } else {
    genericBind(varPtr,var,NULL,a);
  }
}

void AM::undoTrailing(int n) {
  while(n--) {
    TrailEntry *eq = trail.popRef();
    TaggedRef *refPtr = eq->getRefPtr();
    TaggedRef value = eq->getValue();
    *refPtr = value;
  }
}

#ifdef OUTLINE
#include "am.icc"
#undef inline
#endif

// ---------------------------------------------------------------------
