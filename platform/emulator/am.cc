/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "am.hh"

#ifdef OUTLINE
#define inline
#include "am.icc"
#undef inline
#endif

#include "os.hh"
#include "genvar.hh"
#include "fdbuilti.hh"
#include "builtins.hh"

AM am;

/* -------------------------------------------------------------------------
 * IONodes
 * -------------------------------------------------------------------------*/


class IONode {
public:
  int fd;
  OZ_IOHandler handler[2];
  void *readwritepair[2];
  IONode *next;
  IONode(int f, IONode *nxt): fd(f), next(nxt) {
    handler[0] = handler[1] = 0;
    readwritepair[0] = readwritepair[1] = 0;
  }
};

static
IONode *ioNodes = NULL;

static
IONode *findIONode(int fd)
{

  IONode *aux = ioNodes;
  while(aux) {
    if (aux->fd == fd) return aux;
    aux = aux->next;
  }
  ioNodes = new IONode(fd,ioNodes);
  return ioNodes;
}


static
int hasPendingSelect()
{
  IONode *aux = ioNodes;
  while(aux) {
    if (aux->handler[SEL_READ] || aux->handler[SEL_WRITE]) return OK;
    aux = aux->next;
  }
  return NO;
}



/* -------------------------------------------------------------------------
 * Init and exit AM
 * -------------------------------------------------------------------------*/

static
void usage(int /* argc */,char **argv) {
  fprintf(stderr,
          "usage: %s <options>\n",
          argv[0]);
  fprintf(stderr, " -d           : debugging on\n");
  fprintf(stderr, " -init <file> : load and execute init procedure\n");
  fprintf(stderr, " -u <url>     : start a compute server\n");
  fprintf(stderr, " -b <file>    : boot from assembly code\n");
  fprintf(stderr, " -B <dynlib>  : load ozma\n");
  fprintf(stderr, " -browser     : undocumented\n");
  fprintf(stderr, " -a <args> ...: application arguments\n");
  fprintf(stderr, " -- <args> ...: same as above\n");
  osExit(1);
}

static
char *getOptArg(int &i, int argc, char **argv)
{
  i++;
  if (i == argc) {
    fprintf(stderr,"Option '%s' requires argument.\n",argv[i-1]);
    usage(argc,argv);
    return NULL;
  }

  return argv[i];
}


static
void printBanner(char*initFile)
{
#ifdef NO_LTQ
  warning("LTQ is turned off.");
#endif

#ifdef DEBUG_CHECK
  fprintf(stderr,
          "Compile Flags:"
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
#ifdef DEBUG_FSET
          " DEBUG_FSET"
#endif
#ifdef PROFILE_FD
          " PROFILE_FD"
#endif
#ifdef RECINSTRFETCH
          " RECINSTRFETCH=%d", RECINSTRFETCH
#endif
         );
  fprintf(stderr, "\n");
#endif

#ifdef PROFILE_EMULATOR
  fprintf(stderr, "Compiled to support gprof-profiling.\n");
#ifdef DEBUG_DET
  fprintf(stderr, "Deterministic scheduling.\n");
#else
  fprintf(stderr, "Time-slice scheduling.\n");
#endif
#endif

#ifdef THREADED
  // fprintf(stderr, "Using threaded code (abs jumps).\n");
#else
  fprintf(stderr, "Not using threaded code.\n");
#endif

#ifdef PROFILE_FD
  fprintf(stderr, "Compiled to support fd-profiling.\n");
#endif
}


extern void bigIntInit(); /* from value.cc */
extern void initffuns();  /* from initffuns.cc */

void AM::init(int argc,char **argv)
{
  Assert(makeTaggedNULL() == 0);
  Assert(PROCEED && !FAILED);

  ozconf.init();
  ProfileCode(ozstat.initCount());
  osInit();
  bigIntInit();
  initffuns();

  installingScript = FALSE;

  defaultExceptionHdl     = makeTaggedNULL();
  opiCompiler             = makeTaggedNULL();

  preparedCalls = NULL;

  char *tmp;
  if ((tmp = getenv("OZPATH"))) {
    ozconf.ozPath = tmp;
    ozconf.linkPath = tmp;
  }

  if ((tmp = getenv("OZLINKPATH"))) {
    ozconf.linkPath = tmp;
  }

  char *url = NULL;
  char *initFile = getenv("OZINIT");
  char *assemblyCodeFile = NULL;
  char *ozmaLib = NULL;
  int moreThanOne = 0;

  /* process command line arguments */
  ozconf.argV = NULL;
  ozconf.argC = 0;

  for (int i=url?2:1; i<argc; i++) {
    if (strcmp(argv[i],"-d")==0) {
#ifdef DEBUG_TRACE
      ozd_tracerOn();
#endif
      continue;
    }
    if (strcmp(argv[i],"-browser")==0) {
      ozconf.browser = 1;
      continue;
    }

    if (strcmp(argv[i],"-u")==0) {
      moreThanOne++;
      url = getOptArg(i,argc,argv);
      continue;
    }
    if (strcmp(argv[i],"-b")==0) {
      moreThanOne++;
      assemblyCodeFile = getOptArg(i,argc,argv);
      continue;
    }
    if (strcmp(argv[i],"-B")==0) {
      ozmaLib = getOptArg(i,argc,argv);
#ifdef OZMA
      fprintf(stderr,"ozma linked statically. Ignoring -B option\n");
#endif
      continue;
    }
    if (strcmp(argv[i],"-init")==0) {
      initFile = getOptArg(i,argc,argv);
      continue;
    }
    if (strcmp(argv[i],"-a")==0 ||
        strcmp(argv[i],"--")==0) {
      ozconf.argC = argc-i-1;
      ozconf.argV = argv+i+1;
      break;
    }

    fprintf(stderr,"Unknown option '%s'.\n",argv[i]);
    usage(argc,argv);
  }

  if (moreThanOne > 1) {
    fprintf(stderr,"Atmost one of '-u', '-b' allowed.\n");
    usage(argc,argv);
   }

#ifdef DEBUG_CHECK
  ozconf.showIdleMessage=1;
#endif

  if (!initFile) {
    char* ini = "/lib/Init.ozc";
    int m = strlen(ozconf.ozHome);
    int n = m+strlen(ini)+1;
    char*s = new char[n];
    strcpy(s,ozconf.ozHome);
    strcpy(s+m,ini);
    if (access(s,F_OK)==0) initFile = s;
    else delete[] s;
  }
  if (initFile && *initFile=='\0') initFile=0;

  printBanner(initFile);
  if (getenv("OZ_TRACE_LOAD"))
    if (initFile)
      fprintf(stderr,"Init file: %s\n",initFile);
    else
      fprintf(stderr,"No init file\n");

  if (url==0 && assemblyCodeFile==0) {
    fprintf(stderr,"Exactly one of '-b' or '-u' must be given\n");
    ossleep(5);
    osExit(1);
  }

  (void) engine(OK);

  initFDs();

  initMemoryManagement();

// not changeable
  // SizeOfWorkingArea,NumberOfXRegisters,NumberOfYRegisters


// internal registers
  statusReg    = (StatusBit)0;
  criticalFlag = NO;

  _rootBoard = new Board(NULL,Bo_Root);
  _rootBoard->setInstalled();
  _currentBoard = NULL;
  cachedStack  = NULL;
  cachedSelf   = NULL;
  shallowHeapTop = NULL;
  setCurrent(_rootBoard,OK);
  _currentSolveBoard = (Board *) NULL;
  wasSolveSet = NO;

  lastThreadID    = 0;
  debugStreamTail = OZ_newVariable();

  initThreads();
  ozstat.createdThreads.incf();

  // builtins
  initLiterals();
  Builtin *entry = BIinit();
  if (!entry) {
    error("BIinit failed");
    osExit(1);
  }

  extern void initTagged();
  initTagged();

  emptySuspendVarList(); // must be after initLiterals

  osInitSignals();
  osSetAlarmTimer(CLOCK_TICK/1000);

  if (!perdioInit()) {
    fprintf(stderr,"Perdio initialization failed\n");
  }

  {
    Thread *tt = (initFile||url
                  ||assemblyCodeFile
                  )?
      mkRunnableThread(DEFAULT_PRIORITY, _rootBoard):0;

    if (url) {
      TaggedRef proc = oz_newVariable();
      tt->pushCall(proc);
      tt->pushCall(BI_load,oz_atom(url),proc);
    }

    OZ_Term (*ozmaFunc)(const char *);
#ifdef OZMA
    ozmaFunc = ozma_readProc;
#else
    if (ozmaLib) {
      OZ_Term out;
      int ret = osDlopen(ozmaLib, out);
      if (ret != PROCEED) {
        fprintf(stderr, "can not open ozma library %s\n",ozmaLib);
        osExit(1);
      }
      ozmaFunc = (OZ_Term (*)(const char *))
        osDlsym(OZ_getForeignPointer(out),"ozma_readProc");
      if (!ozmaFunc) {
        fprintf(stderr, "can't find ozma_readProc\n");
        osExit(1);
      }
    }
#endif
    if (assemblyCodeFile) {
      OZ_Term v=(*ozmaFunc)(assemblyCodeFile);
      if (v!=makeTaggedNULL()) tt->pushCall(v, 0, 0);
    }

    if (initFile) {
      TaggedRef proc = oz_newVariable();
      tt->pushCall(proc);
      tt->pushCall(BI_load,oz_atom(initFile),proc);
    }

    if (tt) scheduleThread(tt);
  }


  profileMode = NO;

}

void AM::exitOz(int status)
{
  osExit(status);
}

/* -------------------------------------------------------------------------
 * Unification
 * -------------------------------------------------------------------------*/


Bool AM::isLocalUVarOutline(TaggedRef var, TaggedRef *varPtr)
{
  Board *bb=tagged2VarHome(var);
  if (bb->isCommitted()) {
    bb=bb->derefBoard();
    *varPtr=makeTaggedUVar(bb);
  }
  return isCurrentBoard(bb);
}

Bool AM::isLocalSVarOutline(SVariable *var)
{
  Board *home = var->getHomeUpdate();
  return isCurrentBoard(home);
}


inline
// static
Bool AM::installScript(Script &script)
{
  Bool ret = OK;
  installingScript = TRUE; // mm2: special hack ???
  for (int index = 0; index < script.getSize(); index++) {
    int res = oz_unify(script[index].getLeft(),script[index].getRight());
    if (res == PROCEED) continue;
    if (res == FAILED) {
      ret = NO;
      if (!onToplevel()) {
        break;
      }
    } else {
      // mm2:
      error("installScript: unify suspended: %d",res);
    }
  }
  installingScript = FALSE;
#ifndef DEBUG_CHECK
  script.dealloc();
#else
  if (ret == OK)
    script.dealloc();
#endif
  return ret;
}

Bool AM::installScriptOutline(Script &script)
{
  return installScript(script);
}

Bool oz_isBelow(Board *below, Board *above)
{
  while (1) {
    if (below == above) return OK;
    if (oz_isRootBoard(below)) return NO;
    below = below->getParent();
  }
}

inline
static
Board *getVarBoard(TaggedRef var)
{
  CHECK_ISVAR(var);

  if (isUVar(var))
    return tagged2VarHome(var);
  return tagged2SVarPlus(var)->getHome1();
}

inline
static
Bool isMoreLocal(TaggedRef var1, TaggedRef var2)
{
  Board *board1 = getVarBoard(var1)->derefBoard();
  Board *board2 = getVarBoard(var2)->derefBoard();
  return oz_isBelow(board1,board2);
}


/* Define a partial order on CVARs:
 *
 *              Future
 *                |
 *                |
 *               Lazy
 *                |
 *                |
 *              Perdio
 *                |
 *           +----------+
 *           |    |     |
 *             any other
*/


/* return -1 (v1=<v2), +1 (v1>=v2), 0 (dont care) */

int isFuture(GenCVariable *t, TypeOfGenCVariable tag)
{
  return (tag==PerdioVariable) && ((PerdioVar*)t)->isFuture();
}

static
int cmpCVar(GenCVariable *v1, GenCVariable *v2)
{
  TypeOfGenCVariable t1 = v1->getType();
  TypeOfGenCVariable t2 = v2->getType();
  if (isFuture(v1,t1))    return  1;
  if (isFuture(v2,t2))    return -1;
  if (t1==LazyVariable)   return  1;
  if (t2==LazyVariable)   return -1;
  if (t1==PerdioVariable) return  1;
  if (t2==PerdioVariable) return -1;
  return 0;
}


// global vars!!!
static Stack unifyStack(100,Stack_WithMalloc);
static Stack rebindTrail(100,Stack_WithMalloc);

inline
static
void rebind(TaggedRef *refPtr, TaggedRef *ptr2)
{
  rebindTrail.ensureFree(2);
  rebindTrail.push(refPtr,NO);
  rebindTrail.push(ToPointer(*refPtr),NO);
  doBind(refPtr,makeTaggedRef(ptr2));
}

#define PopRebindTrail(value,refPtr)                    \
    TaggedRef value   = ToInt32(rebindTrail.pop());     \
    TaggedRef *refPtr = (TaggedRef*) rebindTrail.pop();


OZ_Return oz_unify(TaggedRef t1, TaggedRef t2, ByteCode *scp)
{
  Assert(am.checkShallow(scp));
  Assert(unifyStack.isEmpty()); /* unify is not reentrant */
  CHECK_NONVAR(t1); CHECK_NONVAR(t2);

  Bool result = FAILED;

  TaggedRef *termPtr1 = &t1;
  TaggedRef *termPtr2 = &t2;

loop:
  int argSize;

  COUNT(totalUnify);

  DEREFPTR(term1,termPtr1,tag1);
  DEREFPTR(term2,termPtr2,tag2);

  // identical terms ?
  if (term1 == term2 &&
      (!isUVar(term1) || termPtr1 == termPtr2)) {
    goto next;
  }

  if (oz_isVariable(term1)) {
    if (oz_isVariable(term2)) {
      goto var_var;
    } else {
      goto var_nonvar;
    }
  } else {
    if (oz_isVariable(term2)) {
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

  COUNT(varNonvarUnify);

  if (isCVar(tag1))
    goto cvar;

  oz_bindToNonvar(termPtr1, term1, term2, scp);
  goto next;



 /*************/
 var_var:

  /*
   * The implemented partial order for binding variables to variables is:
   *   local -> global
   *   UVAR/SVAR -> CVAR (prefer binding nonCVars to CVars)
   *   UVAR      -> SVAR
   *   local newer -> local older
   */
  COUNT(varVarUnify);
  if (isNotCVar(tag1)) {
    if (isNotCVar(tag2) &&
        isMoreLocal(term2,term1) &&
        (!am.isLocalVariable(term1,termPtr1) ||
         (isUVar(term2) && !isUVar(term1)) ||
         heapNewer(termPtr2,termPtr1))) {
      oz_bind(termPtr2, term2, makeTaggedRef(termPtr1));
    } else {
      oz_bind(termPtr1, term1, makeTaggedRef(termPtr2));
    }
    goto next;
  }

  if (isNotCVar(tag2)) {
    oz_bind(termPtr2, term2, makeTaggedRef(termPtr1));
    goto next;
  }

  Assert(isCVar(tag1) && isCVar(tag2));
  /* prefered binding of perdio vars */
  if (cmpCVar(tagged2CVar(term2),tagged2CVar(term1))==1) {
    Swap(term1,term2,TaggedRef);
    Swap(termPtr1,termPtr2,TaggedRef*);
  }


cvar:
  result = tagged2CVar(term1)->unify(termPtr1, termPtr2, term2, scp);
  if (result== PROCEED)
    goto next;
  goto fail;


 /*************/
 nonvar_nonvar:

  COUNT(nonvarNonvarUnify);

  if (tag1 != tag2)
    goto fail;

  switch ( tag1 ) {

  case FSETVALUE:
    if (((FSetValue *) tagged2FSetValue(term1))->unify(term2))
      goto next;
    goto fail;

  case LTUPLE:
    {
      COUNT(recRecUnify);
      LTuple *lt1 = tagged2LTuple(term1);
      LTuple *lt2 = tagged2LTuple(term2);

      rebind(termPtr2,termPtr1);
      argSize = 2;
      termPtr1 = lt1->getRef();
      termPtr2 = lt2->getRef();
      goto push;
    }

  case SRECORD:
    {
      COUNT(recRecUnify);
      SRecord *sr1 = tagged2SRecord(term1);
      SRecord *sr2 = tagged2SRecord(term2);

      if (! sr1->compareFunctor(sr2))
        goto fail;

      rebind(termPtr2,termPtr1);
      argSize  = sr1->getWidth();
      termPtr1 = sr1->getRef();
      termPtr2 = sr2->getRef();
      goto push;
    }

  case OZFLOAT:
    if (floatEq(term1,term2))
      goto next;
    else
      goto fail;

  case SMALLINT:
    if (smallIntEq(term1,term2))
      goto next;
    goto fail;

  case OZCONST:
    if (bigIntEq(term1,term2))
      goto next;
    goto fail;

  case LITERAL:
  case PROMISE:
    /* literals and constants unify if their pointers are equal */
  default:
    goto fail;
  }


 /*************/

next:
  if (unifyStack.isEmpty()) {
    result = PROCEED;
    goto exit;
  }

  termPtr2 = (TaggedRef*) unifyStack.pop();
  termPtr1 = (TaggedRef*) unifyStack.pop();
  argSize  = ToInt32(unifyStack.pop());
  // fall through

push:
  if (argSize>1) {
    unifyStack.ensureFree(3);
    unifyStack.push(ToPointer(argSize-1),NO);
    unifyStack.push(termPtr1+1,NO);
    unifyStack.push(termPtr2+1,NO);
  }
  goto loop;

fail:
  Assert(result!=PROCEED);
  unifyStack.mkEmpty();
  // fall through

exit:
  while (!rebindTrail.isEmpty ()) {
    PopRebindTrail(value,refPtr);
    doBind(refPtr,value);
  }

  return result;
}


/*
  This function checks if the current board is between "varHome" and "to"
  resp. equal to "to".
  */

oz_BFlag oz_isBetween(Board *to, Board *varHome)
{
  while (1) {
    if (am.isCurrentBoard(to)) return B_BETWEEN;
    if (to == varHome) return B_NOT_BETWEEN;
    Assert(!oz_isRootBoard(to));
    to = to->getParentAndTest();
    if (!to) return B_DEAD;
  }
}

inline
// static
Bool AM::wakeUpThread(Thread *tt, Board *home)
{
  Assert (tt->isSuspended());
  Assert (tt->isRThread());

  switch (oz_isBetween(GETBOARD(tt), home)) {
  case B_BETWEEN:
    suspThreadToRunnableOPT(tt);
    scheduleThread(tt);
    return TRUE;

  case B_NOT_BETWEEN:
    return FALSE;

  case B_DEAD:
    //
    //  The whole thread is eliminated - because of the invariant
    // stated just before 'disposeSuspendedThread ()' in thread.hh;
    tt->markDeadThread();
    checkExtThread(tt);
    freeThreadBody(tt);
    return TRUE;

  default:
    Assert(0);
    return FALSE;
  }
}

inline
// static
void AM::wakeupToRunnable(Thread *tt)
{
  Assert(tt->isSuspended());
  tt->markRunnable();

  if (isBelowSolveBoard() || tt->isExtThread()) {
    Assert (isInSolveDebug (GETBOARD(tt)));
    incSolveThreads(GETBOARD(tt));
    tt->setInSolve();
  } else {
    Assert(!isInSolveDebug(GETBOARD(tt)));
  }
}

inline
// static
Bool AM::wakeUpBoard(Thread *tt, Board *home)
{
  Assert(tt->isSuspended());
  Assert(tt->getThrType() == S_WAKEUP);

  //
  //  Note:
  //  We use here the dereferenced board pointer, because:
  // - normally, there should be a *single* "wakeup" suspension
  //   per guard (TODO);
  // - when "unit commit" takes place, the rest of (suspended?) threads
  //   from that guard belong to the guard just above
  //   (or toplevel, of course) - we have to update
  //   the threads counter there;
  // - garbage collector moves the pointer anyway.
  //
  //  It's relevant (should be) for unit commits *only*;
  //  Implicitly move the thread upstairs - the threads counter
  // should be already updated before (during unit committing);
  Board *bb=GETBOARD(tt);

  //
  //  Do not propagate to the current board, but discard it;
  //  Do not propagate to the board which has a runnable
  // "wakeup" thread;
  //
  // Note that we don't need to schedule the wakeup for the board
  // because in both cases there is a thread which will check
  // entailment for us;
  if (isCurrentBoard(bb) || bb->isNervous ()) {
#ifdef DEBUG_CHECK
    // because of assertions in decSuspCount and getSuspCount
    if (bb->isFailed()) {
      tt->markDeadThread();
      checkExtThread(tt);
      return OK;
    }
#endif
    bb->decSuspCount();

    Assert(bb->getSuspCount() > 0);
    tt->markDeadThread();
    // checkExtThread(); // don't check here !
    return OK;
  }

  //
  //  Don't propagate to the variable's home board (again,
  // this can happen only in the case of unit commit), but we have
  // to schedule a wakeup for the new thread's home board,
  // because it could be the last thread in it - check entailment!
  if (bb == home && bb->getSuspCount() == 1) {
    wakeupToRunnable(tt);
    scheduleThread(tt);
    return OK;
  }

  //
  //  General case;
  switch (oz_isBetween(bb, home)) {
  case B_BETWEEN:
    Assert(!am.currentBoard()->isSolve() || am.isBelowSolveBoard());
    am.wakeupToRunnable(tt);
    am.scheduleThread(tt);
    return OK;

  case B_NOT_BETWEEN:
    return NO;

  case B_DEAD:
    tt->markDeadThread();
    checkExtThread(tt);
    return OK;

  default:
    Assert(0);
    return NO;
  }
}


//
//  Generic 'wakeUp';
//  Since this method is used at the only one place, it's inlined;
inline
// static
Bool AM::wakeUp(Thread *tt, Board *home, PropCaller calledBy) {
  switch (tt->getThrType()) {
  case S_RTHREAD:
    return wakeUpThread(tt,home);
  case S_WAKEUP:
    return wakeUpBoard(tt,home);
  case S_PR_THR:
    return wakeUpPropagator(tt, home, calledBy);
  default:
    Assert(0);
    return FALSE;
  }
  return FALSE;         // just to keep gcc happy;
}


void AM::wakeupAny(Thread *tt,Board *bb)
{
  switch (tt->getThrType()) {
  case S_RTHREAD:
    Assert (tt->isSuspended());
    Assert (tt->isRThread());
    suspThreadToRunnable(tt);
    scheduleThread(tt);
    break;
  case S_WAKEUP:
    Assert(tt->isSuspended());
    wakeupToRunnable(tt);
    scheduleThread(tt);
    break;
  case S_PR_THR:
    {
      //mm2
      int ret = wakeUpPropagator(tt,bb,pc_std_unif);
      Assert(ret);
    }
  break;
  }
}

// val is used because it may be a variable which must suspend.
//  if det X then ... fi
//  X = Y
// --> if det Y then ... fi

SuspList *AM::checkSuspensionList(SVariable * var,
                                  SuspList * suspList,
                                  PropCaller calledBy)
{
  if (inShallowGuard())
    return suspList;

  SuspList * retSuspList = NULL;

  PROFILE_CODE1(FDProfiles.inc_item(no_calls_checksusplist);)

  while (suspList) {
    PROFILE_CODE1(FDProfiles.inc_item(susps_per_checksusplist);)

    Thread *thr = suspList->getElem();

    if (thr->isDeadThread ()) {
      suspList = suspList->dispose();
      continue;
    }

PROFILE_CODE1
  (
   if (isCurrentBoard(GETBOARD(var))) {
     if (isCurrentBoard(GETBOARD(thr)))
       FDProfiles.inc_item(from_home_to_home_hits);
     else
       FDProfiles.inc_item(from_home_to_deep_hits);
   } else {
     Board * b = GETBOARD(thr);
     if (b == GETBOARD(var))
       FDProfiles.inc_item(from_deep_to_home_misses);
     else if (oz_isBetween(b, GETBOARD(var))==B_BETWEEN)
       FDProfiles.inc_item(from_deep_to_deep_hits);
     else
       FDProfiles.inc_item(from_deep_to_deep_misses);
   }
   )

 // already runnable susps remain in suspList
    if (thr->isRunnable()) {
      if (thr->isPropagator()) {
        if (calledBy && !thr->isUnifyThread()) {
          switch (oz_isBetween(GETBOARD(thr), GETBOARD(var))) {
          case B_BETWEEN:
            thr->markUnifyThread ();
            break;
          case B_DEAD:
            //  keep the thread itself alive - it will be discarded
            // *properly* in the emulator;
            suspList = suspList->dispose ();
            continue;
          case B_NOT_BETWEEN:
            break;
          }
        }
      } else {
        //  non-propagator, i.e. it just goes away;
        suspList = suspList->dispose();
        continue;
      }
    } else {
      if (wakeUp(thr, GETBOARD(var), calledBy)) {
        Assert (thr->isDeadThread () || thr->isRunnable ());
        suspList = suspList->dispose ();
        continue;
      }
    }

    // susp cannot be woken up therefore relink it
    SuspList * first = suspList;
    suspList = suspList->getNext();
    first->setNext(retSuspList);
    retSuspList = first;
  } // while

  return retSuspList;
}


#ifdef DEBUG_CHECK
static
Board *varHome(TaggedRef val) {
  if (isUVar(val)) {
    return tagged2VarHome(val);
  } else {
    return GETBOARD(tagged2SVarPlus(val));
  }
}
#endif

#ifdef DEBUG_CHECK
static
Bool checkHome(TaggedRef *vPtr) {
  TaggedRef val = oz_deref(*vPtr);

  return !oz_isVariable(val) ||
    oz_isBelow(am.currentBoard(),varHome(val));
}
#endif

/*
 * oz_bind: bind var to term
 * Note: does not handle CVARs specifically
 */

void oz_bind(TaggedRef *varPtr, TaggedRef var, TaggedRef term)
{
  /* first step: do suspension */
  if (isSVar(var) || isCVar(var)) {
    am.checkSuspensionList(var, pc_std_unif);
  }

  /* second step: push binding for non-local variable on trail;     */
  if ( !am.isLocalVariable(var,varPtr)) {
    Assert(am.inShallowGuard() || checkHome(varPtr));
    am.doTrail(varPtr,var);
  } else  {
    if (isSVar(var)) {
      tagged2SVar(var)->dispose();
    } else if (isCVar(var)) {
      tagged2CVar(var)->dispose();
    }
  }

  /* third step: bind */
  doBind(varPtr,term);
}

void oz_bind_global(TaggedRef var, TaggedRef term)
{
  DEREF(var,varPtr,varTag);

  Assert(oz_isVariable(var));

  /* first step: do suspension */
  if (isSVar(var) || isCVar(var)) {
    tagged2SVarPlus(var)->wakeupAll();
  }

  /* free memory */
  if (isSVar(var)) {
    tagged2SVar(var)->dispose();
  } else if (isCVar(var)) {
    tagged2CVar(var)->dispose();
  }

  /* second step: bind */
  doBind(varPtr,term);
}


void AM::doBindAndTrail(TaggedRef v, TaggedRef * vp, TaggedRef t)
{
  Assert(shallowHeapTop || checkHome(vp));
  trail.pushRef(vp, v);

  CHECK_NONVAR(t);
  *vp = t;

  Assert(oz_isRef(*vp) || !oz_isVariable(*vp));
}

/*
 * ... and install propagators
 */
void AM::doBindAndTrailAndIP(TaggedRef v, TaggedRef * vp, TaggedRef t,
                             GenCVariable * lv, GenCVariable * gv)
{
  lv->installPropagators(gv);
  Assert(shallowHeapTop || checkHome(vp));
  trail.pushRef(vp, v);

  CHECK_NONVAR(t);
  *vp = t;

  Assert(oz_isRef(*vp) || !oz_isVariable(*vp));
}

/*
 *
 *  Install every board from the currentBoard to 'n'
 * and move cursor to 'n'
 *
 *  Algorithm:
 *   find common parent board of 'to' and 'currentBoard'
 *   deinstall until common parent (go upward)
 *   install (go downward)
 *
 *  Pre-conditions:
 *  - 'to' ist not deref'd;
 *  - 'to' may be committed, failed or discarded;
 *
 *  Return values and post-conditions:
 *  - INST_OK:
 *      installation successful, currentBoard == 'to';
 *  - INST_FAILED:
 *      installation of *some* board on the "down" path has failed,
 *      'am.currentBoard' points to that board;
 *  - INST_REJECTED:
 *      *some* board on the "down" path is already failed or discarded,
 *      'am.currentBoard' stays unchanged;
 *
 */
InstType AM::installPath(Board *to)
{
  if (to->isInstalled()) {
    deinstallPath(to);
    return INST_OK;
  }

  Assert(!oz_isRootBoard(to));

  Board *par=to->getParentAndTest();
  if (!par) {
    return INST_REJECTED;
  }

  InstType ret = installPath(par);
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

//
//  Constructors for 'suspended' cases:
//    deep 'unify' suspension;
//    suspension with continuation;
//    suspension with a 'C' function;
//    suspended sequential thread (with a task stack);
//
inline
// static
Thread *AM::mkWakeupThread(Board *bb)
{
  Thread *th = new Thread(S_WAKEUP,DEFAULT_PRIORITY,bb,newId());
  bb->incSuspCount();
  checkDebug(th,bb);
  return th;
}


// only used in deinstall
// Three cases may occur:
// any global var G -> ground ==> add susp to G
// any global var G -> constrained local var ==> add susp to G
// unconstrained global var G1 -> unconstrained global var G2
//    ==> add susp to G1 and G2

void AM::reduceTrailOnSuspend()
{
  if (!trail.isEmptyChunk()) {
    int numbOfCons = trail.chunkSize();
    Board * bb = currentBoard();
    bb->newScript(numbOfCons);

    //
    // one single suspended thread for all;
    Thread *thr = mkWakeupThread(bb);

    for (int index = 0; index < numbOfCons; index++) {
      TaggedRef * refPtr, value;

      trail.popRef(refPtr, value);

      Assert(oz_isRef(*refPtr) || !oz_isVariable(*refPtr));
      Assert(oz_isVariable(value));

      bb->setScript(index,refPtr,*refPtr);

      TaggedRef vv= *refPtr;
      DEREF(vv,vvPtr,_vvTag);
      if (oz_isVariable(vv)) {
        addSuspAnyVar(vvPtr,thr,NO);  // !!! Makes space *not* unstable !!!
      }

      unBind(refPtr, value);

      // value is always global variable, so add always a thread;
      addSuspAnyVar(refPtr,thr);

    } // for
  } // if
  trail.popMark();
}

void AM::reduceTrailOnFail()
{
  while(!trail.isEmptyChunk()) {
    TaggedRef *refPtr;
    TaggedRef value;
    trail.popRef(refPtr,value);
    unBind(refPtr,value);
  }
  trail.popMark();
}

void AM::reduceTrailOnShallow()
{
  emptySuspendVarList();

  while(!trail.isEmptyChunk()) {
    TaggedRef *refPtr;
    TaggedRef value;
    trail.popRef(refPtr,value);

    Assert(oz_isVariable(value));

    TaggedRef oldVal = makeTaggedRef(refPtr);
    DEREF(oldVal,ptrOldVal,_1);

    unBind(refPtr,value);

    /*
     * shallow guards don't bind variables always.
     *  in INLINEREL/FUNs they are only pushed (pushIfVar) onto the trail
     */
    if (refPtr!=ptrOldVal) {
      if (oz_isVariable(oldVal)) {
        addSuspAnyVar(ptrOldVal,currentThread());
      }
    }

    addSuspAnyVar(refPtr,currentThread());
  }
  trail.popMark();
}

void AM::reduceTrailOnEqEq()
{
  emptySuspendVarList();

  while(!trail.isEmptyChunk()) {
    TaggedRef *refPtr;
    TaggedRef value;
    trail.popRef(refPtr,value);

    Assert(oz_isVariable(value));

    TaggedRef oldVal = makeTaggedRef(refPtr);
    DEREF(oldVal,ptrOldVal,_1);

    unBind(refPtr,value);

    if (oz_isVariable(oldVal)) {
      addSuspendVarList(ptrOldVal);
    }

    addSuspendVarList(refPtr);
  }
  trail.popMark();
}

/* -------------------------------------------------------------------------
 * OFS
 * -------------------------------------------------------------------------*/
// Check if there exists an S_ofs (Open Feature Structure) suspension
// in the suspList (Used only for monitorArity)
Bool AM::hasOFSSuspension(SuspList *suspList)
{
  while (suspList) {
    Thread *thr = suspList->getElem ();
    if (!thr->isDeadThread () &&
        thr->isPropagator() && thr->isOFSThread ()) return TRUE;
    suspList = suspList->getNext();
  }
  return FALSE;
}


/* Add list of features to each OFS-marked suspension list 'flist' has
 * three possible values: a single feature (literal or integer), a
 * nonempty list of features, or NULL (no extra features).
 * 'determined'==TRUE iff the unify makes the OFS determined.  'var'
 * (which must be deref'ed) is used to make sure that features are
 * added only to variables that are indeed waiting for features. This
 * routine is inspired by am.checkSuspensionList, and must track all
 * changes to it.  */
void AM::addFeatOFSSuspensionList(TaggedRef var,
                                  SuspList* suspList,
                                  TaggedRef flist,
                                  Bool determ)
{
  while (suspList) {
    Thread *thr = suspList->getElem ();

    // The added condition ' || thr->isRunnable () ' is incorrect
    // since isPropagated means only that the thread is runnable
    if (thr->isDeadThread ()) {
      suspList=suspList->getNext();
      continue;
    }

    if (thr->isPropagator() && thr->isOFSThread ()) {
      MonitorArityPropagator *prop =
        (MonitorArityPropagator *) thr->getPropagator();

      Assert(sizeof(MonitorArityPropagator)==prop->sizeOf());

      // Only add features if var and fvar are the same:
      TaggedRef fvar=prop->getX();
      DEREF(fvar,_1,_2);
      if (var!=fvar) {
        suspList=suspList->getNext();
        continue;
      }
      // Only add features if the 'kill' variable is undetermined:
      TaggedRef killl=prop->getK();
      DEREF(killl,_,killTag);
      if (!isVariableTag(killTag)) {
        suspList=suspList->getNext();
        continue;
      }

      // Add the feature or list to the diff. list in FH and FT:
      if (flist) {
        if (oz_isFeature(flist))
          prop->setFH(cons(flist,prop->getFH()));
        else {
          // flist must be a list
          Assert(oz_isCons(flist));
          TaggedRef tmplist=flist;
          while (tmplist!=AtomNil) {
            prop->setFH(cons(head(tmplist),prop->getFH()));
            tmplist=tail(tmplist);
          }
        }
      }
      if (determ) {
        // FS is det.: tail of list must be bound to nil: (always succeeds)
        // Do *not* use unification to do this binding!
        TaggedRef tl=prop->getFT();
        DEREF(tl,tailPtr,tailTag);
        switch (tailTag) {
        case LITERAL:
          Assert(tl==AtomNil);
          break;
        case UVAR:
          doBind(tailPtr, AtomNil);
          break;
        default:
          Assert(FALSE);
        }
      }
    }

    suspList = suspList->getNext();
  }
}

/* -------------------------------------------------------------------------
 * MISC
 * -------------------------------------------------------------------------*/

int AM::awakeIO(int, void *var) {
  am.awakeIOVar((TaggedRef) var);
  return 1;
}

void AM::awakeIOVar(TaggedRef var)
{
  Assert(onToplevel());
  Assert(oz_isCons(var));

  OZ_unifyInThread(OZ_head(var),OZ_tail(var));
}

#ifdef DEBUG_CHECK
static Board *oldBoard = (Board *) NULL;
static Board *oldSolveBoard = (Board *) NULL;
#endif

void AM::setCurrent(Board *c, Bool checkNotGC)
{
  Assert(!c->isCommitted() && !c->isFailed());
  Assert(!checkNotGC || isCurrentBoard(oldBoard));

  _currentBoard = c;
  _currentUVarPrototype = makeTaggedUVar(c);
  DebugCheckT(oldBoard=c);

  if (c->isSolve ()) {
    Assert(!checkNotGC || oldSolveBoard == _currentSolveBoard);

    _currentSolveBoard = c;
    wasSolveSet = OK;
    DebugCode (oldSolveBoard = c);
  } else if (wasSolveSet == OK) {
    Assert(!checkNotGC || oldSolveBoard == _currentSolveBoard);

    _currentSolveBoard = c->getSolveBoard();
    wasSolveSet = NO;
    DebugCode (oldSolveBoard = _currentSolveBoard);
  }
}


void AM::select(int fd, int mode, OZ_IOHandler fun, void *val)
{
  if (!onToplevel()) {
    warning("select only on toplevel");
    return;
  }
  IONode *ion = findIONode(fd);
  ion->readwritepair[mode]=val;
  ion->handler[mode]=fun;
  osWatchFD(fd,mode);
}


void AM::acceptSelect(int fd, OZ_IOHandler fun, void *val)
{
  if (!onToplevel()) {
    warning("select only on toplevel");
    return;
  }

  IONode *ion = findIONode(fd);
  ion->readwritepair[SEL_READ]=val;
  ion->handler[SEL_READ]=fun;
  osWatchAccept(fd);
}

int AM::select(int fd, int mode,TaggedRef l,TaggedRef r)
{
  if (!onToplevel()) {
    warning("select only on toplevel");
    return OK;
  }
  if (osTestSelect(fd,mode)==1) {
    OZ_unifyInThread(l,r);
    return OK;
  }
  IONode *ion = findIONode(fd);
  ion->readwritepair[mode]=(void *) cons(l,r);
  gcProtect((TaggedRef *) &(ion->readwritepair[mode]));

  ion->handler[mode]=awakeIO;
  osWatchFD(fd,mode);
  return OK;
}

void AM::acceptSelect(int fd,TaggedRef l,TaggedRef r)
{
  if (!onToplevel()) {
    warning("acceptSelect only on toplevel");
    return;
  }

  IONode *ion = findIONode(fd);
  ion->readwritepair[SEL_READ]=(void *) cons(l,r);
  gcProtect((TaggedRef *) &(ion->readwritepair[SEL_READ]));

  ion->handler[SEL_READ]=awakeIO;
  osWatchAccept(fd);
}

void AM::deSelect(int fd)
{
  deSelect(fd,SEL_READ);
  deSelect(fd,SEL_WRITE);
}

void AM::deSelect(int fd,int mode)
{
  osClrWatchedFD(fd,mode);
  IONode *ion = findIONode(fd);
  ion->readwritepair[mode]  = 0;
  (void) gcUnprotect((TaggedRef *) &(ion->readwritepair[mode]));
  ion->handler[mode]  = 0;
}

// called if IOReady (signals are blocked)
void AM::handleIO()
{
  unsetSFlag(IOReady);
  int numbOfFDs = osFirstSelect();


  // find the nodes to awake
  for (int index = 0; numbOfFDs > 0; index++) {
    for(int mode=SEL_READ; mode <= SEL_WRITE; mode++) {

      if (osNextSelect(index, mode) ) {

        numbOfFDs--;

        IONode *ion = findIONode(index);
        if ((ion->handler[mode]) &&  /* Perdio: handlers may do a deselect for other fds*/
            (ion->handler[mode])(index, ion->readwritepair[mode])) {
          ion->readwritepair[mode] = 0;
          (void) gcUnprotect((TaggedRef *)&(ion->readwritepair[mode]));
          ion->handler[mode] = 0;
          osClrWatchedFD(index,mode);
        }
      }
    }
  }
}

void checkIO(){
  AM &e=am;
  if(e.isSetSFlag(IOReady)){
    osBlockSignals();
    e.handleIO();
    osUnblockSignals();}
}


// called from signal handler
void AM::checkIO()
{
  int numbOfFDs = osCheckIO();
  if (!isCritical() && (numbOfFDs > 0)) {
    setSFlag(IOReady);
  }
}

void AM::suspendEngine()
{
  deinstallPath(_rootBoard);

  ozstat.printIdle(stdout);

  osBlockSignals(OK);

  while (1) {

    if (isSetSFlag(UserAlarm)) {
      handleUser();
    }

    if (isSetSFlag(IOReady)) {
      handleIO();
    }

    if (!threadQueuesAreEmpty()) {
      break;
    }

    // mm2: test if system is idle (not yet working: perdio test is missing)
#ifdef TEST_IDLE
    if (!nextUser() && !hasPendingSelect()) {
      fprintf(stderr,"System is idle: terminating\n");
      exitOz(-1);
    }
#endif

    unsigned long idle_start = osTotalTime();

    int msleft = osBlockSelect(nextUser());
    setSFlag(IOReady);

    ozstat.timeIdle += (osTotalTime() - idle_start);

    wakeUser();
  }

  ozstat.printRunning(stdout);

  // restart alarm
  osSetAlarmTimer(CLOCK_TICK/1000);

  osUnblockSignals();
}

void AM::checkStatus()
{
  if (isSetSFlag(StartGC)) {
      deinstallPath(_rootBoard);
      doGC();
  }
  if (isSetSFlag(UserAlarm)) {
    deinstallPath(_rootBoard);
    osBlockSignals();
    handleUser();
    osUnblockSignals();
  }
  if (isSetSFlag(IOReady)) {
    deinstallPath(_rootBoard);
    osBlockSignals();
    handleIO();
    osUnblockSignals();
  }
}


/* -------------------------------------------------------------------------
 * Search
 * -------------------------------------------------------------------------*/

/*
 * increment/decrement the thread counter
 * in every solve board above
 * if "stable" generate a new thread "solve waker"
 * NOTE:
 *   there may be failed board in between which must be simply ignored
 *
 * kost@ to mm2:
 *   Michael, i don't think that it was a good idea to merge
 * these things together - this leads to efficiency penalties,
 * and less possibilities to make an assertion.
 *
 * RETURNS: OK if solveSpace found, else NO
 */

int AM::incSolveThreads(Board *bb)
{
  int ret = NO;
  while (!oz_isRootBoard(bb)) {
    Assert(!bb->isCommitted());
    if (bb->isSolve()) {
      ret = OK;
      SolveActor *sa = SolveActor::Cast(bb->getActor());
      //
      Assert (!sa->isCommitted());

      //
      sa->incThreads ();

      //
      Assert (!(isStableSolve (sa)));
    }
    bb = bb->getParent();
  }
  return ret;
}

void AM::decSolveThreads(Board *bb)
{
  while (!oz_isRootBoard(bb)) {
    Assert(!bb->isCommitted());
    if (bb->isSolve()) {
      SolveActor *sa = SolveActor::Cast(bb->getActor());

      //
      // local optimization - check for threads first;
      if (sa->decThreads () == 0) {
        //
        // ... first - notification board below the failed solve board;
        if (!(sa->isCommitted ()) && isStableSolve (sa)) {
          scheduleThread(mkRunnableThread(DEFAULT_PRIORITY,bb));
        }
      } else {
        Assert (sa->getThreads () > 0);
      }
    }
    bb = bb->getParent();
  }
}

#ifdef DEBUG_CHECK
/*
 *  Just check whether the 'bb' is located beneath some (possibly dead)
 * solve board;
 */
Bool AM::isInSolveDebug (Board *bb)
{
  while (!oz_isRootBoard(bb)) {
    Assert(!bb->isCommitted());
    if (bb->isSolve()) {
      SolveActor *sa = SolveActor::Cast(bb->getActor());
      if (!sa->isCommitted()) {
        return (OK);
      }
    }
    bb = bb->getParent();
  }

  return (NO);
}
#endif

Bool AM::isStableSolve(SolveActor *sa)
{
  if (sa->getThreads() != 0)
    return NO;
  if (isCurrentBoard(sa->getSolveBoard()) &&
      !trail.isEmptyChunk())
    return NO;
  // simply "don't worry" if in all other cases it is too weak;
  return sa->areNoExtSuspensions();
}


/* -------------------------------------------------------------------------
 * Signals
 * -------------------------------------------------------------------------*/

void handlerUSR1()
{
  message("Error handler entered ****\n");

  CodeArea::writeInstr();

#ifdef DEBUG_TRACE
  ozd_tracerOn(); ozd_trace("halt");
#endif
  message("Error handler exit ****\n");
}

void handlerINT()
{
  prefixError();
  message("SIG INT ****\n");
  am.exitOz(1);
}

void handlerTERM()
{
  prefixError();
  message("SIG TERM ****\n");
  am.exitOz(0);
}

void handlerMessage()
{
  message("SIGNAL inside signal handler ***\n");
}

void handlerSEGV()
{
  CodeArea::writeInstr();
  error("**** segmentation violation ****\n");
  am.exitOz(1);
}

void handlerBUS()
{
  CodeArea::writeInstr();
  error("**** bus error ****\n");
  am.exitOz(1);
}

void handlerPIPE()
{
  prefixError();
  message("write on a pipe or other socket with no one to read it ****\n");
}

void handlerCHLD()
{
  DebugCheckT(message("a child process' state changed ****\n"));
}

void handlerFPE()
{
  OZ_warning("signal: floating point exception");
}

// signal handler
void handlerALRM()
{
  am.handleAlarm();
}

/* -------------------------------------------------------------------------
 * Alarm handling
 * -------------------------------------------------------------------------*/

void AM::handleAlarm()
{
  if (ozstat.currPropagator) {
    ozstat.currPropagator->incSamples();
  } else if (ozstat.currAbstr) {
    ozstat.currAbstr->samples++;
  }

  if (isCritical()) {  /* wait for next ALRM signal */
    return;
  }

  if (threadSwitchCounter > 0) {
    if (--threadSwitchCounter == 0) {
      setSFlag(ThreadSwitch);
    }
  }

  if (checkUser())
    setSFlag(UserAlarm);

  checkGC();

  checkIO();
}

/* handleUserAlarm:
    if UserAlarm-SFLAG is set this method is called
    interrupts should already be disabled by the parent procedure
    */
void AM::handleUser()
{
  unsetSFlag(UserAlarm);
  wakeUser();
}

class OzSleep {
public:
  OzSleep *next;
  unsigned int time;    // absolut time in ms, when this guy must be woken up
  TaggedRef node;
public:
  OzSleep(int t, TaggedRef n,OzSleep *a)
    : next(a), time(t), node(n)
  {
    OZ_protect(&node);
    Assert(t>=0);
  }
  ~OzSleep() { OZ_unprotect(&node); }
};

void AM::insertUser(int ms, TaggedRef node)
{
  osBlockSignals();

  unsigned int wakeupAt = osTotalTime() + ms;

  OzSleep **prev = &sleepQueue;
  for (OzSleep *aux = *prev; aux; prev = &aux->next, aux=aux->next) {
    if (wakeupAt <= aux->time) {
      *prev = new OzSleep(wakeupAt,node,aux);
      goto exit;
    }
  }
  *prev = new OzSleep(wakeupAt,node,NULL);

exit:
  osUnblockSignals();
}


int AM::nextUser()
{
  return (sleepQueue==NULL) ? 0 : max(1,sleepQueue->time - osTotalTime());
}


Bool AM::checkUser()
{
  return (sleepQueue!=NULL && sleepQueue->time <= osTotalTime());
}

void AM::wakeUser()
{
  unsigned int now = osTotalTime();

  while (sleepQueue && sleepQueue->time<=now) {
    awakeIOVar(sleepQueue->node);
    OzSleep *aux = sleepQueue->next;
    delete sleepQueue;
    sleepQueue = aux;
  }
}



void AM::checkDebugOutline(Thread *tt)
{
  Assert(debugmode());
  if (currentThread() && tt->getThrType() == S_RTHREAD)
    if (currentThread()->getTrace()) {
      tt->setTrace(OK);
      tt->setStep(OK);
    }
}

//  Make a runnable thread with a single task stack entry <local thread queue>
Thread *AM::mkLTQ(Board *bb, int prio)
{
  Thread *th = new Thread(S_RTHREAD | T_runnable | T_ltq,prio,bb,newId());
  th->setBody(allocateBody());
  bb->incSuspCount();
  checkDebug(th,bb);
  Assert(isCurrentBoard(bb));
  Assert(isInSolveDebug(bb));

  incSolveThreads(bb);
  th->setInSolve();

  th->pushLTQ(bb);

  return th;
}

int AM::commit(Board *bb, Thread *tt)
{
  Assert(!currentBoard()->isCommitted());
  Assert(isCurrentBoard(bb->getParent()));

  AWActor *aw = AWActor::Cast(bb->getActor());

  Assert(!tt || tt==aw->getThread());

  Continuation *cont=bb->getBodyPtr();

  bb->setCommitted(currentBoard());
  currentBoard()->incSuspCount(bb->getSuspCount()-1);

  if (bb->isWait()) {
    Assert(bb->isWaiting());

    WaitActor *wa = WaitActor::Cast(aw);

    if (currentBoard()->isWait()) {
      WaitActor::Cast(currentBoard()->getActor())->mergeChoices(wa->getCpb());
    } else if (currentBoard()->isSolve()) {
      SolveActor::Cast(currentBoard()->getActor())->mergeChoices(wa->getCpb());
    } else {
      // forget the choice stack when committing to a conditional
    }

    if (!installScriptOutline(bb->getScriptRef())) {
      return 0;
    }
  }

  if (!tt) {
    tt=aw->getThread();
    Assert(tt->isSuspended());
    suspThreadToRunnableOPT(tt);
    scheduleThread(tt);
    DebugCheckT(aw->setThread(0));
  }

  TaskStack *ts = tt->getTaskStackRef();
  ts->discardActor();
  if (aw->isAsk()) {
    AskActor::Cast(aw)->disposeAsk();
  } else {
    WaitActor::Cast(aw)->disposeWait();
  }

  ts->pushCont(cont->getPC(),cont->getY(),cont->getG());
  if (cont->getX()) ts->pushX(cont->getX());

  return 1;
}

// see variable.hh
void checkExtThread(Thread *elem, Board *home)
{
  if (am.isBelowSolveBoard()) {
    am.setExtThreadOutlined(elem,home->derefBoard());
  }
}

void AM::setExtThreadOutlined(Thread *tt, Board *varHome)
{
  Board *bb = currentBoard();
  Bool wasFound = NO;
  Assert (!varHome->isCommitted());

  while (bb != varHome) {
    Assert (!oz_isRootBoard(bb));
    Assert (!bb->isCommitted() && !bb->isFailed());
    if (bb->isSolve()) {
      SolveActor *sa = SolveActor::Cast(bb->getActor());
      sa->addSuspension(tt);
      wasFound = OK;
    }
    bb = bb->getParent();
  }

  if (wasFound) tt->setExtThread();
}

void AM::checkExtThreadOutlined(Thread *tt)
{
  Assert(tt->wasExtThread());

  Board *sb = GETBOARD(tt)->getSolveBoard();

  while (sb) {
    Assert(sb->isSolve());

    SolveActor *sa = SolveActor::Cast(sb->getActor());
    if (isStableSolve(sa)) {
      scheduleThread(mkRunnableThreadOPT(DEFAULT_PRIORITY,sb));
    }
    sb = GETBOARD(sa)->getSolveBoard();
  }
}

void AM::removeExtThreadOutlined(Thread *tt)
{
  Assert(tt->wasExtThread());

  Board *sb = GETBOARD(tt)->getSolveBoard ();

  while (sb) {
    Assert (sb->isSolve());

    SolveActor *sa = SolveActor::Cast(sb->getActor());
    sa->clearSuspList(tt);
    sb = GETBOARD(sa)->getSolveBoard();
  }
}


SuspList *AM::installPropagators(SuspList * local_list, SuspList * glob_list,
                                 Board * glob_home)
{
  Assert((local_list && glob_list && (local_list != glob_list)) ||
         !local_list || !glob_list);

  SuspList * aux = local_list, * ret_list = local_list;


  // mark up local suspensions to avoid copying them
  while (aux) {
    aux->getElem()->markTagged();
    aux = aux->getNext();
  }

  // create references to suspensions of global variable
  aux = glob_list;
  while (aux) {
    Thread *thr = aux->getElem();

    if (!thr->isDeadThread() &&
        thr->isPropagator() &&
        !thr->isTagged() && /* TMUELLER possible optimization
                                  isTaggedAndUntag */
        oz_isBetween(GETBOARD(thr), glob_home) == B_BETWEEN) {
      ret_list = new SuspList (thr, ret_list);
    }

    aux = aux->getNext();
  }

  // unmark local suspensions
  aux = local_list;
  while (aux) {
    aux->getElem()->unmarkTagged();
    aux = aux->getNext();
  }

  return ret_list;
}

#ifdef DEBUG_STATUS
  /*
   * Print capital letter, when flag is set and
   * lower case letter when unset.
   */
char flagChar(StatusBit flag)
{
  switch (flag) {
  case ThreadSwitch: return 'T';
  case IOReady:      return 'I';
  case UserAlarm:    return 'U';
  case StartGC:      return 'G';
  case DebugMode:    return 'D';
  default:           return 'X';
  }
}
#endif

/*
 * when failure occurs
 *  mark the actor
 *  clean the trail
 *  update the current board
 */
void AM::failBoard()
{
  Assert(!onToplevel());
  Board *bb=currentBoard();
  Assert(bb->isInstalled());

  Actor *aa=bb->getActor();
  if (aa->isAsk()) {
    (AskActor::Cast(aa))->failAskChild();
  } else if (aa->isWait()) {
    (WaitActor::Cast(aa))->failWaitChild(bb);
  }

  Assert(!bb->isFailed());
  bb->setFailed();

  reduceTrailOnFail();
  bb->unsetInstalled();
  setCurrent(GETBOARD(aa));
}



void AM::prepareCall(TaggedRef pred, RefsArray args)
{
  CallList **aux = &preparedCalls;
  while(*aux) {
    aux = &(*aux)->next;
  }
  *aux = new CallList(pred,args);
}


void AM::prepareCall(TaggedRef pred, TaggedRef arg0, TaggedRef arg1,
                     TaggedRef arg2, TaggedRef arg3, TaggedRef arg4)
{
  int argno = 0;
  if (arg0) argno++;
  if (arg1) argno++;
  if (arg2) argno++;
  if (arg3) argno++;
  if (arg4) argno++;

  RefsArray a = allocateRefsArray(argno);
  if (arg0) a[0]=arg0;
  if (arg1) a[1]=arg1;
  if (arg2) a[2]=arg2;
  if (arg3) a[3]=arg3;
  if (arg4) a[4]=arg4;
  prepareCall(pred,a);
}


void AM::pushPreparedCalls(Thread *thr)
{
  Assert(preparedCalls != NULL);
  while(preparedCalls) {
    CallList *aux = preparedCalls;
    if (thr) {
      thr->pushCallNoCopy(aux->proc,aux->args);
    } else {
      cachedStack->pushCallNoCopy(aux->proc,aux->args);
    }
    preparedCalls = aux->next;
    aux->dispose();
  }
}

void AM::suspendOnVarList(Thread *thr)
{
  while (oz_isCons(_suspendVarList)) {
    OZ_Term v=head(_suspendVarList);
    Assert(oz_isVariable(*tagged2Ref(v)));

    addSuspAnyVar(tagged2Ref(v),thr);
    _suspendVarList=tail(_suspendVarList);
  }
}
