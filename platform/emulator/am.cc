/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow, mehl

      (main) AM::procedure;
      another things.
  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "am.hh"
#endif

#include "am.hh"

#include "genvar.hh"
#include "fdbuilti.hh"
#include "builtins.hh"

AM am;

/* -------------------------------------------------------------------------
 * Init and exit AM
 * -------------------------------------------------------------------------*/

static
void usage(int /* argc */,char **argv) {
  fprintf(stderr,
          "usage: %s [-E] [-S file | -f file] [-d] [-c compiler]\n",
          argv[0]);
  osExit(1);
}

static
char *getOptArg(int &i, int argc, char **argv)
{
  i++;
  if (i == argc) {
    usage(argc,argv);
    return NULL;
  }

  return argv[i];
}


static
void printBanner()
{
  version();

#if defined(DEBUG_DET) && !defined(WINDOWS)
  // windows dumps if I activate this
  warning("DEBUG_DET implies eager waking of sleep.");
#endif

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

#ifdef THREADED
  // printf("Using threaded code (abs jumps).\n");
#else
  printf("Not using threaded code.\n");
#endif

#ifdef PROFILE_FD
  printf("Compiled to support fd-profiling.\n");
#endif
}


extern void bigIntInit(); /* from value.cc */

void AM::init(int argc,char **argv)
{
  Assert(makeTaggedNULL() == 0);
  ozconf.init();
  osInit();
  bigIntInit();

  installingScript = FALSE;

  suspendVarList   = makeTaggedNULL();
  aVarUnifyHandler = makeTaggedNULL();
  aVarBindHandler  = makeTaggedNULL();
  methApplHdl      = makeTaggedNULL();

  char *compilerName = OzCompiler;

  char *tmp;
  if ((tmp = getenv("OZPATH"))) {
    ozconf.ozPath = tmp;
    ozconf.linkPath = tmp;
  }

  if ((tmp = getenv("OZLINKPATH"))) {
    ozconf.linkPath = tmp;
  }

  char *compilerFIFO = NULL;  // path name where to connect to
  char *precompiledFile = NULL;
  Bool quiet = FALSE;

  /* process command line arguments */
  ozconf.argV = NULL;
  ozconf.argC = 0;
  for (int i=1; i<argc; i++) {
    if (strcmp(argv[i],"-E")==0) {
      ozconf.runningUnderEmacs = 1;
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
      compilerName = getOptArg(i,argc,argv);
      continue;
    }
    if (strcmp(argv[i],"-S")==0) {
      compilerFIFO = getOptArg(i,argc,argv);
      continue;
    }
    if (strcmp(argv[i],"-f")==0) {
      precompiledFile = getOptArg(i,argc,argv);
      continue;
    }

    if (strcmp(argv[i],"-a")==0) {
      ozconf.argC = argc-i-1;
      ozconf.argV = argv+i+1;
      break;
    }

    usage(argc,argv);
  }

  int moreThanOne = 0;
  moreThanOne += (compilerFIFO != NULL);
  moreThanOne += (precompiledFile != NULL);
  if (moreThanOne > 1) {
     fprintf(stderr,"Specify only one of '-s' and '-f' and '-S'.\n");
     usage(argc,argv);
   }

  if (quiet == FALSE) {
    printBanner();
  }

  isStandaloneF=NO;
  if (compilerFIFO != NULL) {
    compStream = connectCompiler(compilerFIFO);
  } else if (precompiledFile != NULL) {
    compStream = useFile(precompiledFile);
    isStandaloneF=OK;
  } else {
    compStream = execCompiler(compilerName);
  }

  if (compStream == NULL) {
    fprintf(stderr,"Cannot open code input\n");
    ossleep(5);
    osExit(1);
  }

  checkVersion();

  engine(OK);

  initFDs();

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
  cachedStack  = NULL;
  cachedSelf   = NULL;
  setCurrent(rootBoard,OK);
  currentSolveBoard = (Board *) NULL;
  wasSolveSet = NO;

  lastThreadID     = 0L;
  threadStream     = OZ_newVariable();
  threadStreamTail = threadStream;

  breakflag = NO;

  initThreads();
  toplevelQueue = (Toplevel *) NULL;

  // builtins
  BuiltinTabEntry *entry = BIinit();
  if (!entry) {
    error("BIinit failed");
    osExit(1);
  }
  initLiterals();

  extern void initTagged();
  initTagged();

  toplevelVars      = allocateRefsArray(ozconf.numToplevelVars);
  toplevelVarsCount = 0;

  toplevelVars[0] = makeTaggedConst(entry);

  BuiltinTabEntry *biTabEntry = builtinTab.find("biExceptionHandler");
  defaultExceptionHandler = makeTaggedConst(biTabEntry);

  ioNodes = new IONode[osOpenMax()];

  if (!isStandalone()) {
    osWatchFD(compStream->csfileno(),SEL_READ);
  }

  osInitSignals();
  osSetAlarmTimer(CLOCK_TICK/1000);

  // --> make sure that we check for input from compiler
  setSFlag(IOReady);

#ifdef DEBUG_CHECK
  dontPropagate = NO;
#endif
}

void AM::checkVersion()
{
  char s[100];
  char *ss = compStream->csgets(s,100);
  if (ss && ss[strlen(ss)-1] == '\n')
    ss[strlen(ss)-1] = '\0';
  if (ss != NULL && strcmp(ss,OZVERSION) != 0) {
    fprintf(stderr,"*** Wrong version from compiler\n");
    fprintf(stderr,"*** Expected: '%s', got: '%s'\n",OZVERSION,ss);
    ossleep(3);
    osExit(1);
  }
}

void AM::exitOz(int status)
{
  compStream->csclose();
  osExit(status);
}

/* -------------------------------------------------------------------------
 * Unification
 * -------------------------------------------------------------------------*/


Bool AM::fastUnifyOutline(TaggedRef ref1, TaggedRef ref2, Bool prop)
{
  return fastUnify(ref1, ref2, prop);
}

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

  return result;
}


Bool AM::isLocalUVarOutline(TaggedRef var, TaggedRef *varPtr)
{
  Board *bb=tagged2VarHome(var);
  if (bb->isCommitted()) {
    bb=bb->derefBoard();
    *varPtr=makeTaggedUVar(bb);
  }
  return  bb == currentBoard;
}

Bool AM::isLocalSVarOutline(SVariable *var)
{
  Board *home = var->getHomeUpdate();
  return home == currentBoard;
}


inline
Bool AM::installScript(Script &script)
{
  Bool ret = OK;
  installingScript = TRUE;
  for (int index = 0; index < script.getSize(); index++) {
    if (!unify(script[index].getLeft(),script[index].getRight())) {
      ret = NO;
      if (!isToplevel()) {
        break;
      }
    }
  }
  installingScript = FALSE;
#ifndef DEBUG_CHECK
  script.dealloc();
#else
  if (ret == OK)
    script.dealloc ();
#endif
  return ret;
}

Bool AM::installScriptOutline(Script &script)
{
  return installScript(script);
}


inline
Board *getVarBoard(TaggedRef var)
{
  CHECK_ISVAR(var);

  if (isUVar(var))
    return tagged2VarHome(var);
  if (isSVar(var))
    return tagged2SVar(var)->getHome1();

  return taggedCVar2SVar(var)->getHome1();
}


inline
Bool AM::isMoreLocal(TaggedRef var1, TaggedRef var2)
{
  Board *board1 = getVarBoard(var1)->derefBoard();
  Board *board2 = getVarBoard(var2)->derefBoard();
  return isBelow(board1,board2);
}


#define Swap(A,B,Type) { Type help=A; A=B; B=help; }

Bool AM::performUnify(TaggedRef *termPtr1, TaggedRef *termPtr2, Bool prop)
{
  int argSize;
  RefsArray args1, args2;

start:
  DEREFPTR(term1,termPtr1,tag1);
  DEREFPTR(term2,termPtr2,tag2);

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

  /*
   * The implemented partial order for binding variables to variables is:
   *   local -> global
   *   UVAR/SVAR -> CVAR (prefer binding nonCVars to CVars)
   *   UVAR      -> SVAR
   *   local newer -> local older
   */
  if (isNotCVar(tag1)) {
    if ( isNotCVar(tag2) &&
         isMoreLocal(term2,term1) &&
         (!isLocalVariable(term1,termPtr1) ||
          (isUVar(term2) && !isUVar(term1)) ||
           heapNewer(termPtr2,termPtr1))) {
      genericBind(termPtr2, term2, termPtr1, *termPtr1, prop);
    } else {
      genericBind(termPtr1, term1, termPtr2, *termPtr2, prop);
    }
    return OK;
  }

  if (isNotCVar(tag2)) {
    genericBind(termPtr2, term2, termPtr1, *termPtr1, prop);
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

  case FSETVALUE:
    return ((FSetValue *) tagged2FSetValue(term1))->unify(term2);

  case OZCONST:
    return tagged2Const(term1)->unify(term2,prop);

  case LTUPLE:
    {
      args1 = tagged2LTuple(term1)->getRef();
      args2 = tagged2LTuple(term2)->getRef();
      argSize = 2;
      goto unify_args;
    }

  case SRECORD:
    {
      SRecord *sr1 = tagged2SRecord(term1);
      SRecord *sr2 = tagged2SRecord(term2);

      if (! sr1->compareFunctor(sr2)) {
        return NO;
      }

      argSize = sr1->getWidth();
      args1 = sr1->getRef();
      args2 = sr2->getRef();

      goto unify_args;
    }

  case LITERAL:
    /* literals unify if their pointers are equal */
    return NO;

  case OZFLOAT:
  case BIGINT:
  case SMALLINT:
    return numberEq(term1,term2);

  default:
    return NO;
  }


 /*************/
 unify_args:

  rebind(termPtr2,term1);
  for (int i = 0; i < argSize-1; i++ ) {
    if ( !performUnify(args1+i,args2+i, prop)) {
      return NO;
    }
  }

  /* tail recursion optimization */
  termPtr1 = args1+argSize-1;
  termPtr2 = args2+argSize-1;
  goto start;
}

/*
  This function checks if the current board is between "varHome" and "to"
  resp. equal to "to".
  */

BFlag AM::isBetween(Board *to, Board *varHome)
{
  while (1) {
    if (to == currentBoard) return B_BETWEEN;
    if (to == varHome) return B_NOT_BETWEEN;
    to = to->getParentAndTest();
    if (!to) return B_DEAD;
  }
}

Bool AM::isBelow(Board *below, Board *above)
{
  while (1) {
    if (below == above) return OK;
    if (below == rootBoard) return NO;
    below = below->getParent();
  }
}

// val is used because it may be a variable which must suspend.
//  if det X then ... fi
//  X = Y
// --> if det Y then ... fi

SuspList * AM::checkSuspensionList(SVariable * var,
                                   SuspList * suspList,
                                   PropCaller calledBy)
{
  SuspList * retSuspList = NULL;

  // see the reduction of solve actor by the enumeration;
  DebugCheck(dontPropagate == OK, return (suspList));

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
   if (var->getBoard() == am.currentBoard) {
     if (thr->getBoard() == am.currentBoard)
       FDProfiles.inc_item(from_home_to_home_hits);
     else
       FDProfiles.inc_item(from_home_to_deep_hits);
   } else {
     Board * b = thr->getBoard();
     if (b == var->getBoard())
       FDProfiles.inc_item(from_deep_to_home_misses);
     else if (am.isBetween(b, var->getBoard())==B_BETWEEN)
       FDProfiles.inc_item(from_deep_to_deep_hits);
     else
       FDProfiles.inc_item(from_deep_to_deep_misses);
   }
   )

 // already propagated susps remain in suspList
    if (thr->isPropagated ()) {
      if (thr->isPropagator ()) {
        if (calledBy && !(thr->isUnifyThread ())) {
          switch (isBetween(thr->getBoard(), var->getBoard())) {
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
      if (thr->wakeUp(var->getBoard(), calledBy)) {
        Assert (thr->isDeadThread () || thr->isPropagated ());
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

// exception from general rule that arguments are never variables!
//  term may be an
void AM::genericBind(TaggedRef *varPtr, TaggedRef var,
                     TaggedRef *termPtr, TaggedRef term,
                     Bool prop)
     /* bind var to term;         */
{
  Assert(!isCVar(var) && !isRef(term));

  /* first step: do suspensions */
  if (prop) {
    if (isSVar(var)) {
      checkSuspensionList(var, pc_std_unif);

      if (isSVar(term)) {
        checkSuspensionList(term, pc_std_unif);
      }
    }
  }

  /* second step: mark binding for non-local variable in trail;     */
  /* also mark such (i.e. this) variable in suspention list;        */
  if ( !isLocalVariable(var,varPtr) || !prop ) {
    trail.pushRef(varPtr,var);
  } else  { // isLocalVariable(var)
    if (isSVar(var)) {
      tagged2SVar(var)->dispose();
    }
  }

  doBind(varPtr,isAnyVar(term) ? makeTaggedRef(termPtr) : term);
}


void AM::doBindAndTrail(TaggedRef v, TaggedRef * vp, TaggedRef t)
{
  trail.pushRef(vp, v);

  CHECK_NONVAR(t);
  *vp = t;

  Assert(isRef(*vp) || !isAnyVar(*vp));
}

/*
 * ... and install propagators
 */
void AM::doBindAndTrailAndIP(TaggedRef v, TaggedRef * vp, TaggedRef t,
                             GenCVariable * lv, GenCVariable * gv,
                             Bool prop)
{
  lv->installPropagators(gv,prop);
  trail.pushRef(vp, v);

  CHECK_NONVAR(t);
  *vp = t;

  Assert(isRef(*vp) || !isAnyVar(*vp));
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

  Assert(to != rootBoard);

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

void AM::reduceTrailOnUnitCommit()
{
  int numbOfCons = trail.chunkSize();

  Board *bb = currentBoard;

  bb->newScript(numbOfCons);

  for (int index = 0; index < numbOfCons; index++) {
    TaggedRef *refPtr;
    TaggedRef value;
    trail.popRef(refPtr,value);

    Assert(isRef(*refPtr) || !isAnyVar(*refPtr));
    Assert(isAnyVar(value));

    bb->setScript(index,refPtr,*refPtr);

    unBind(refPtr,value);
  }
  trail.popMark();
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
    Board * bb = currentBoard;
    bb->newScript(numbOfCons);

    //
    // one single suspended thread for all;
    Thread *thr = mkWakeupThread(bb);

    for (int index = 0; index < numbOfCons; index++) {
      TaggedRef * refPtr, value, old_value;

      trail.popRef(refPtr, value);

      Assert(isRef(*refPtr) || !isAnyVar(*refPtr));
      Assert(isAnyVar(value));

      bb->setScript(index,refPtr,*refPtr);

      old_value = makeTaggedRef(refPtr);
      DEREF(old_value, old_value_ptr, old_value_tag);

      unBind(refPtr, value);

      // value is always global variable, so add always a thread;
      taggedBecomesSuspVar(refPtr)->addSuspension (thr);

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

inline void mkSusp(TaggedRef *ptr, Thread *t)
{
  if (t) {
    taggedBecomesSuspVar(ptr)->addSuspension(t);
  } else {
    am.addSuspendVarList(ptr);
  }
}


/*
 * shallow guards sometimes do not bind variables but only push them
 * if thread is NULL then we are called from '==':
 * return the list of variable in am.suspendVarList otherwise
 * add "thread" to susplist
 */
void AM::reduceTrailOnShallow(Thread *thread)
{
  am.suspendVarList = makeTaggedNULL();

  while(!trail.isEmptyChunk()) {
    TaggedRef *refPtr;
    TaggedRef value;
    trail.popRef(refPtr,value);

    Assert(isAnyVar(value));

    TaggedRef oldVal = makeTaggedRef(refPtr);
    DEREF(oldVal,ptrOldVal,_1);

    unBind(refPtr,value);

    /* test if only trailed to create thread and not bound ? */
    if (refPtr!=ptrOldVal) {
      if (isAnyVar(oldVal)) {
        mkSusp(ptrOldVal,thread);
      }
    }

    mkSusp(refPtr,thread);
  }
  trail.popMark();
}

/* -------------------------------------------------------------------------
 * OFS
 * -------------------------------------------------------------------------*/
// Check if there exists an S_ofs (Open Feature Structure) suspension in the suspList
// (Used only for monitorArity)
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


// Add list of features to each OFS-marked suspension list
// 'flist' has three possible values: a single feature (literal or integer), a
// nonempty list of features, or NULL (no extra features).  'determined'==TRUE iff
// the unify makes the OFS determined.  'var' (which must be deref'ed) is used to
// make sure that features are added only to variables that are indeed waiting for
// features. This routine is inspired by am.checkSuspensionList, and must track all
// changes to it.
void AM::addFeatOFSSuspensionList(TaggedRef var,
                                  SuspList* suspList,
                                  TaggedRef flist,
                                  Bool determ)
{
    while (suspList) {
        Thread *thr = suspList->getElem ();

        // The added condition ' || thr->isPropagated () ' is incorrect
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
            if (!isAnyVar(killTag)) {
                suspList=suspList->getNext();
                continue;
            }

            // Add the feature or list to the diff. list in FH and FT:
            if (flist) {
                if (isFeature(flist))
                    prop->setFH(cons(flist,prop->getFH()));
                else {
                    // flist must be a list
                    Assert(isLTuple(flist));
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

int AM::awakeIO(int fd, void *var) {
  am.awakeIOVar((TaggedRef) var);
  return 1;
}

void AM::awakeIOVar(TaggedRef var)
{
  Assert(isToplevel());
  Assert(isCons(var));

  if (OZ_unify(OZ_head(var),OZ_tail(var)) != PROCEED) {
    warning("select or sleep failed");
  }
}

#ifdef DEBUG_CHECK
static Board *oldBoard = (Board *) NULL;
static Board *oldSolveBoard = (Board *) NULL;
#endif

void AM::setCurrent(Board *c, Bool checkNotGC)
{
  Assert(!c->isCommitted() && !c->isFailed());
  Assert(!checkNotGC || oldBoard == currentBoard);

  currentBoard = c;
  currentUVarPrototype = makeTaggedUVar(c);
  DebugCheckT(oldBoard=c);

  if (c->isSolve ()) {
    Assert(!checkNotGC || oldSolveBoard == currentSolveBoard);

    currentSolveBoard = c;
    wasSolveSet = OK;
    DebugCode (oldSolveBoard = c);
  } else if (wasSolveSet == OK) {
    Assert(!checkNotGC || oldSolveBoard == currentSolveBoard);

    currentSolveBoard = c->getSolveBoard();
    wasSolveSet = NO;
    DebugCode (oldSolveBoard = currentSolveBoard);
  }
}

Bool AM::loadQuery(CompStream *fd)
{
  unsigned int starttime = osUserTime();

  ProgramCounter pc;

  Bool ret = CodeArea::load(fd,pc);

  if (ret == OK && pc != NOCODE) {
    addToplevel(pc);
  }

  ozstat.timeForLoading.incf(osUserTime()-starttime);

  return ret;
}


void AM::pushTask(ProgramCounter pc,RefsArray y,RefsArray g,RefsArray x,int i)
{
  x = (i>0) ? copyRefsArray(x,i) : 0;
  cachedStack->pushCont(pc,y,g,x);
}

void AM::select(int fd, int mode, OZ_IOHandler fun, void *val)
{
  if (!isToplevel()) {
    warning("select only on toplevel");
    return;
  }
  ioNodes[fd].readwritepair[mode]=val;
  ioNodes[fd].handler[mode]=fun;
  osWatchFD(fd,mode);
}


void AM::acceptSelect(int fd, OZ_IOHandler fun, void *val)
{
  if (!isToplevel()) {
    warning("select only on toplevel");
    return;
  }

  ioNodes[fd].readwritepair[SEL_READ]=val;
  ioNodes[fd].handler[SEL_READ]=fun;
  osWatchAccept(fd);
}

int AM::select(int fd, int mode,TaggedRef l,TaggedRef r)
{
  if (!isToplevel()) {
    warning("select only on toplevel");
    return OK;
  }
  if (osTestSelect(fd,mode)==1) return OZ_unify(l,r);
  ioNodes[fd].readwritepair[mode]=(void *) cons(l,r);
  gcProtect((TaggedRef *) &ioNodes[fd].readwritepair[mode]);

  ioNodes[fd].handler[mode]=awakeIO;
  osWatchFD(fd,mode);
  return OK;
}

void AM::acceptSelect(int fd,TaggedRef l,TaggedRef r)
{
  if (!isToplevel()) {
    warning("acceptSelect only on toplevel");
    return;
  }

  ioNodes[fd].readwritepair[SEL_READ]=(void *) cons(l,r);
  gcProtect((TaggedRef *) &ioNodes[fd].readwritepair[SEL_READ]);

  ioNodes[fd].handler[SEL_READ]=awakeIO;
  osWatchAccept(fd);
}

void AM::deSelect(int fd)
{
  osClrWatchedFD(fd,SEL_READ);
  osClrWatchedFD(fd,SEL_WRITE);
  ioNodes[fd].readwritepair[SEL_READ]  = 0;
  ioNodes[fd].readwritepair[SEL_WRITE] = 0;
  (void) gcUnprotect((TaggedRef *) &ioNodes[fd].readwritepair[SEL_READ]);
  (void) gcUnprotect((TaggedRef *) &ioNodes[fd].readwritepair[SEL_WRITE]);
  ioNodes[fd].handler[SEL_READ]  = 0;
  ioNodes[fd].handler[SEL_WRITE]  = 0;
}

void AM::deSelect(int fd,int mode)
{
  osClrWatchedFD(fd,mode);
  ioNodes[fd].readwritepair[mode]  = 0;
  (void) gcUnprotect((TaggedRef *) &ioNodes[fd].readwritepair[mode]);
  ioNodes[fd].handler[mode]  = 0;
}

// called if IOReady (signals are blocked)
void AM::handleIO()
{
  unsetSFlag(IOReady);
  int numbOfFDs = osFirstSelect();

  /* check input from compiler */
  if (osNextSelect(compStream->csfileno(),SEL_READ) || /* do this FIRST, sideeffect! */
      !compStream->bufEmpty()) {
    do {
      loadQuery(compStream);
    } while(!compStream->bufEmpty());
    numbOfFDs--;
  }

  // find the nodes to awake
  for (int index = 0; numbOfFDs > 0; index++) {
    for(int mode=SEL_READ; mode <= SEL_WRITE; mode++) {

      if (osNextSelect(index, mode) ) {

        numbOfFDs--;

        Assert(ioNodes[index].handler[mode]);
        if ((ioNodes[index].handler[mode])
            (index, ioNodes[index].readwritepair[mode])) {
          ioNodes[index].readwritepair[mode] = 0;
          (void) gcUnprotect((TaggedRef *)&ioNodes[index].readwritepair[mode]);
          ioNodes[index].handler[mode] = 0;
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
  if (!isCritical() && (numbOfFDs > 0 || !compStream->bufEmpty())) {
    setSFlag(IOReady);
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
 */
void AM::incSolveThreads(Board *bb)
{
  while (!bb->isRoot()) {
    Assert(!bb->isCommitted());
    if (bb->isSolve()) {
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
}

void AM::decSolveThreads(Board *bb)
{
  while (!bb->isRoot()) {
    Assert(!bb->isCommitted());
    if (bb->isSolve()) {
      SolveActor *sa = SolveActor::Cast(bb->getActor());

      //
      // local optimization - check for threads first;
      if (sa->decThreads () == 0) {
        //
        // ... first - notification board below the failed solve board;
        if (!(sa->isCommitted ()) && isStableSolve (sa)) {
          scheduleThread (mkRunnableThread(sa->getPriority(),bb,OK));
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
  while (!bb->isRoot()) {
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
  if (sa->getSolveBoard() == currentBoard &&
      !trail.isEmptyChunk())
    return NO;
  // simply "don't worry" if in all other cases it is too weak;
  return sa->areNoExtSuspensions();
}

/* ------------------------------------------------------------------------
 * Threads
 *
 * running thread: currentThread
 * toplevel: rootThread and toplevelQueue
 * ------------------------------------------------------------------------ */


/*
 * Toplevel is a queue of toplevel queries, which must be executed
 * sequentially.
 *   see checkToplevel()  called when disposing a thread
 *       addToplevel()    called when a new query arrives
 */
class Toplevel {
public:
  ProgramCounter pc;
  Toplevel *next;
  Toplevel(ProgramCounter p, Toplevel *nxt) : pc(p), next(nxt) {}
};

/*
 * push a new toplevel continuation to the rootThread
 * PRE: the rootThread is empty
 * called from checkToplevel and addToplevel
 */
void AM::pushToplevel(ProgramCounter pc)
{
  Assert(rootThread->isEmpty());
  // kost@ : MOD!!! TODO?
  // rootBoard->incSuspCount();
  rootThread->pushCont(pc,toplevelVars,NULL,NULL);
  if (rootThread!=currentThread && !isScheduled(rootThread)) {
    scheduleThread(rootThread);
  }
}

/*
 * in dispose: check if there are more toplevel tasks
 */
void AM::checkToplevel()
{
  if (toplevelQueue) {
    Verbose((VERB_THREAD,"checkToplevel: pushNext: 0x%x\n",this));
    Toplevel **last;
    for (last = &toplevelQueue;
         ((*last)->next) != NULL;
         last = &((*last)->next)) {
    }
    pushToplevel((*last)->pc);
    *last = (Toplevel *) NULL;
  }
}

void AM::addToplevel(ProgramCounter pc)
{
  if (rootThread->isEmpty()) {
    Verbose((VERB_THREAD,"addToplevel: push\n"));
    pushToplevel(pc);
  } else {
    Verbose((VERB_THREAD,"addToplevel: delay\n"));
    toplevelQueue = new Toplevel(pc,toplevelQueue);
  }
}

/* -------------------------------------------------------------------------
 * Signals
 * -------------------------------------------------------------------------*/

void handlerUSR1()
{
  message("Error handler entered ****\n");

  CodeArea::writeInstr();

  tracerOn(); trace("halt");
  message("Error handler exit ****\n");
}

void handlerINT()
{
  message("SIG INT ****\n");
  am.exitOz(1);
}

void handlerTERM()
{
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
  message("**** segmentation violation ****\n");
  longjmp(am.engineEnvironment,SEGVIO);
}

void handlerBUS()
{
  CodeArea::writeInstr();
  message("**** bus error ****\n");
  longjmp(am.engineEnvironment,BUSERROR);
}

void handlerPIPE()
{
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
  if (isCritical()) {  /* wait for next ALRM signal */
    return;
  }

  if (threadSwitchCounter > 0) {
    if (--threadSwitchCounter == 0) {
      setSFlag(ThreadSwitch);
    }
  }

  /* load userCounter into a local variable: under win32 this
   * code is executed concurrently with other code manipulating
   * the userCounter
   */
  int uc = userCounter;
  Assert(uc>=0);
  if (uc > 0) {
    if (--uc == 0) {
      setSFlag(UserAlarm);
    }
    setUserAlarmTimer(uc);
  }

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
  int nextMS = wakeUser();
  setUserAlarmTimer(nextMS);
}

class OzSleep {
public:
  OzSleep *next;
  int time;    // in clock ticks
  TaggedRef node;
public:
  OzSleep(int t, TaggedRef n,OzSleep *a)
  : time(t), node(n), next(a)
  {
    OZ_protect(&node);
    Assert(t>=0);
  }
  ~OzSleep() {
    OZ_unprotect(&node);
  }
};

void AM::insertUser(int ms, TaggedRef node)
{
  int t = osMsToClockTick(ms);

  osBlockSignals();

  if (sleepQueue) {
    int rest = getUserAlarmTimer();
    Assert(isSetSFlag(UserAlarm) || rest > 0);

    if (rest <= t) {
      setUserAlarmTimer(rest);
      t = t-rest;
    } else {
      setUserAlarmTimer(t);
      sleepQueue->time = rest - t;
      t = 0;
    }
  } else {
    setUserAlarmTimer(t);
    t = 0;
  }

  osUnblockSignals();

  OzSleep **prev = &sleepQueue;
  for (OzSleep *a = *prev; a; prev = &a->next, a=a->next) {
    if (t <= a->time) {
      a->time = a->time - t;
      *prev = new OzSleep(t,node,a);
      return;
    }
  }
  *prev = new OzSleep(t,node,NULL);
}

int AM::wakeUser()
{
  if (!sleepQueue) {
    OZ_warning("wake: nobody to wake up");
    return 0;
  }
  while (sleepQueue && sleepQueue->time==0) {
    awakeIOVar(sleepQueue->node);
    OzSleep *tmp = sleepQueue->next;
    delete sleepQueue;
    sleepQueue = tmp;
  }
  if (sleepQueue) {
    int ret = sleepQueue->time;
    sleepQueue->time = 0;
    return ret;
  }
  return 0;
}



OZ_Term AM::dbgGetLoc(Board *bb) {
  OZ_Term out = nil();
  while (!bb->isRoot()) {
    if (bb->isSolve()) {
      out = cons(OZ_atom("space"),out);
    } else if (bb->isAsk()) {
      out = cons(OZ_atom("cond"),out);
    } else if (bb->isWait()) {
      out = cons(OZ_atom("dis"),out);
    } else {
      out = cons(OZ_atom("???"),out);
    }
    bb=bb->getParent();
  }
  return out;
}


//
//  Make a runnable thread with a single task stack entry <local trhread queue>
Thread *AM::mkLTQ(Board *bb, int prio, SolveActor * sa)
{
  Thread *th = new Thread(S_RTHREAD | T_prop | T_ltq,prio,bb);
  th->setBody(allocateBody());

  Assert(bb == currentBoard);
  Assert (isInSolveDebug(bb));

  incSolveThreads(bb);
  th->setInSolve();

  th->pushTask(sa);

  return th;
}

#ifdef PERDIO
void AM::stopThread(Thread *th) {
  if (th->pStop()==0) {
    if (th==am.currentThread) {
      setSFlag(StopThread);
    }
    th->stop();
  }
}

void AM::resumeThread(Thread *th) {
  if (th->pCont()==0) {
    th->cont();
    if (!th->isDeadThread() && th->isRunnable() && !am.isScheduled(th)) {
      am.scheduleThread(th);
    }
  }
}
#endif

#ifdef OUTLINE
#define inline
#include "am.icc"
#undef inline
#endif



// ---------------------------------------------------------------------
