#include "runtime.hh"

# define CBB (e->currentBoard())
# define CTT (e->currentThread())


static
TaggedRef formatError(TaggedRef info,TaggedRef val,
                      OZ_Term traceBack,OZ_Term loc)
{
  OZ_Term d = OZ_record(OZ_atom("d"),
                        cons(OZ_atom("info"),
                             cons(OZ_atom("stack"),
                                  cons(OZ_atom("loc"),
                                       nil()))));
  OZ_putSubtree(d,OZ_atom("stack"),traceBack);
  OZ_putSubtree(d,OZ_atom("loc"),loc);
  OZ_putSubtree(d,OZ_atom("info"),info);

  return OZ_adjoinAt(val,OZ_atom("debug"),d);
}

OZ_C_proc_proto(BIfail);     // builtins.cc

// check if failure has to be raised as exception on thread
static
int canOptimizeFailure(AM *e, Thread *tt)
{
  if (tt->hasCatchFlag() || e->onToplevel()) { // catch failure
    if (tt->isSuspended()) {
      tt->pushCFun(BIfail,0,0,NO);
      e->suspThreadToRunnableOPT(tt);
      e->scheduleThread(tt);
    } else {
      printf("WEIRD: failure detected twice");
#ifdef DEBUG_CHECK
      PopFrame(tt->getTaskStackRef(),PC,Y,G);
      Assert(PC==C_CFUNC_CONT_Ptr);
      Assert(((OZ_CFun)(void*)Y)==BIfail);
      tt->pushCFun(BIfail,0,0,NO);
#endif
    }
    return NO;
  } else {
    return OK;
  }
}

void scheduler() {
  register AM * const e = &am;
  goto LBLstart;

  /* -----------------------------------------------------------------------
   * Get thread from queue
   * ----------------------------------------------------------------------- */
LBLstart:
  Assert(CTT==0);

  // check status register
  if (e->isSetSFlag()) {
    e->checkStatus();
  }

  if (e->threadQueuesAreEmpty()) {
    e->suspendEngine();
  }
  e->setCurrentThread(e->getFirstThread());
  Assert(CTT);

  DebugTrace(trace("runnable thread->running"));

  // source level debugger & Thread.suspend
  if (CTT->getStop() || CTT->getPStop()) {
    e->unsetCurrentThread();  // byebye...
    goto LBLstart;
  }

  //  Every runnable thread must be terminated through
  // 'LBL{discard,kill}Thread', and it should not appear
  // more than once in the threads pool;
  Assert(!CTT->isDeadThread() && CTT->isRunnable());

  // Install board
  {
    Board *bb=GETBOARD(CTT);
    if (CBB != bb) {
      switch (e->installPath(bb)) {
      case INST_OK:
        break;
      case INST_REJECTED:
        DebugCode (if (CTT->isPropagator()) CTT->removePropagator());
        goto LBLdiscardThread;
      case INST_FAILED:
        DebugCode (if (CTT->isPropagator()) CTT->removePropagator());
        goto LBLfailure;
      }
    }
  }

  // Constraints are implicitly propagated
  CBB->unsetNervous();

  if (CTT->isPropagator()) {
    // Propagator
    //    unsigned int starttime = osUserTime();
    switch (e->runPropagator(CTT)) {
    case SLEEP:
      e->suspendPropagator(CTT);
      if (e->isBelowSolveBoard()) {
        e->decSolveThreads(e->currentSolveBoard());
        //  but it's still "in solve";
      }
      e->unsetCurrentThread();

      goto LBLstart;

    case SCHEDULED:
      e->scheduledPropagator(CTT);
      if (e->isBelowSolveBoard()) {
        e->decSolveThreads (e->currentSolveBoard());
        //  but it's still "in solve";
      }
      e->unsetCurrentThread();

      goto LBLstart;

    case PROCEED:
      // Note: CTT must be reset in 'LBLkillXXX';

      goto LBLterminate;

      //  Note that *propagators* never yield 'SUSPEND';
    case FAILED:
      // propagator failure never catched
      if (!e->onToplevel()) goto LBLfailure;

      // simulate the case that the propagator runs on a usual stack
      {
        OZ_Propagator *prop = CTT->getPropagator();
        e->setCurrentThread(e->mkRunnableThreadOPT(PROPAGATOR_PRIORITY, CBB));
        e->cachedStack = CTT->getTaskStackRef();
        e->restartThread();
        e->exception.info  = NameUnit;
        e->exception.value = RecordFailure;
        e->exception.debug = ozconf.errorDebug;
        e->exception.info  = OZ_mkTupleC("apply",2,
                                         OZ_atom(builtinTab.getName((void *)(prop->getHeader()->getHeaderFunc()))),
                                         prop->getParameters());
        e->exception.pc = NOCODE;
        goto LBLraise;
      }
    }
    error("propagator failure");
    goto LBLerror;
  }

  Assert(CTT->isRunnable());

  //
  //  Note that this test covers also the case when a runnable thread
  // was suspended in a sequential mode: it had already a stack,
  // so we don't have to do anything now;
  if (CTT->isWakeup()) goto LBLterminate;

  Assert(CTT->getThrType() == S_RTHREAD);

  e->restartThread(); // start a new time slice
  // fall through

  /* -----------------------------------------------------------------------
   * Running a thread
   * ----------------------------------------------------------------------- */
LBLrunThread:
  {

    ozstat.leaveCall(CTT->abstr);
    CTT->abstr = 0;
    e->cachedStack = CTT->getTaskStackRef();
    e->cachedSelf  = CTT->getSelf();
    CTT->setSelf(0);

    int ret=engine(NO);

    CTT->setAbstr(ozstat.currAbstr);
    ozstat.leaveCall(NULL);
    e->saveSelf();
    Assert(!e->cachedSelf);

    switch (ret) {
    case T_PREEMPT:
      goto LBLpreemption;
    case T_SUSPEND:
      goto LBLsuspend;
    case T_RAISE:
      goto LBLraise;
    case T_TERMINATE:
      goto LBLterminate;
    case T_FAILURE:
      goto LBLfailure;
    case T_ERROR:
    default:
      goto LBLerror;
    }
  }


  /* -----------------------------------------------------------------------
   * preemption
   * ----------------------------------------------------------------------- */
LBLpreemption:
  Assert(GETBOARD(CTT)==CBB);
  /*  Assert(CTT->isRunnable()|| (CTT->isStopped())); ATTENTION */
  e->scheduleThreadInline(CTT, CTT->getPriority());
  e->unsetCurrentThread();
  goto LBLstart;

  /* -----------------------------------------------------------------------
   * An error occured
   * ----------------------------------------------------------------------- */

LBLerror:
  fprintf(stderr,"scheduler: An error has occurred.\n");
  goto LBLstart;


  /* -----------------------------------------------------------------------
   * Thread is terminated
   * ----------------------------------------------------------------------- */
  /*
   *  Kill the thread - decrement 'suspCounter'"s and counters of
   * runnable threads in solve actors if any
   */
LBLterminate:
  {
    DebugTrace(trace("kill thread", CBB));
    Assert(CTT);
    Assert(!CTT->isDeadThread());
    Assert(CTT->isRunnable());
    Assert(CTT->isWakeup() || CTT->isPropagator() || CTT->isEmpty());

    //  Note that during debugging the thread does not carry
    // the board pointer (== NULL) wenn it's running;
    // Assert (CBB == CTT->getBoard());

    Assert(CBB != (Board *) NULL);
    Assert(!CBB->isFailed());

    Assert(e->onToplevel() ||
           ((CTT->isInSolve() || !e->isBelowSolveBoard()) &&
            (e->isBelowSolveBoard() || !CTT->isInSolve())));

    if (CTT == e->rootThread()) {
      e->rootThread()->reInit();
      e->checkToplevel();
      if (e->rootThread()->isEmpty()) {
        e->unsetCurrentThread();
        goto LBLstart;
      } else {
        goto LBLpreemption;
      }
    }

    CBB->decSuspCount();

    e->disposeRunnableThread(CTT);
    e->unsetCurrentThread();

    // fall through to checkEntailmentAndStability
  }

  /* -----------------------------------------------------------------------
   * Check entailment and stability
   * ----------------------------------------------------------------------- */

LBLcheckEntailmentAndStability:
  {
    /*     *  General comment - about checking for stability:
     *  In the case when the thread was originated in a solve board,
     * we have to update the (runnable) threads counter there manually,
     * check stability there ('AM::checkStability ()'), and proceed
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
     */

    // maybe optimize?
    if (e->onToplevel()) goto LBLstart;

    Board *nb = 0; // notification board

    //
    //  First, look at the current board, and if it's a solve one,
    // decrement the (runnable) threads counter manually _and_
    // skip the 'AM::decSolveThreads ()' for it;
    if (e->isBelowSolveBoard()) {
      if (CBB->isSolve ()) {
        SolveActor *sa;

        //
        sa = SolveActor::Cast (CBB->getActor ());
        //  'nb' points to some board above the current one,
        // so, 'decSolveThreads' will start there!
        nb = GETBOARD(sa);

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
    DebugCode(e->unsetCurrentThread());

    DebugTrace(trace("check entailment",CBB));

#ifdef DEBUG_NONMONOTONIC
    cout << "checkEntailment" << endl << flush;
#endif

    CBB->unsetNervous();

  // check for entailment of ASK and WAITTOP
    if ((CBB->isAsk() || CBB->isWaitTop()) && e->entailment()) {
      Board *bb = CBB;
      e->deinstallCurrent();
      int ret=e->commit(bb);
      Assert(ret);
    } else if (CBB->isSolve()) {
      e->checkStability();
    }

    //
    //  deref nb, because it maybe just committed!
    if (nb) e->decSolveThreads (nb->derefBoard());
    goto LBLstart;
  }

  /* -----------------------------------------------------------------------
   * Discard Thread
   * ----------------------------------------------------------------------- */

  /*
   *  Discard the thread, i.e. just decrement solve thread counters
   * everywhere it is needed, and dispose the body;
   *  The main difference to the 'LBLterminateThread' is that no
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
    Assert(CTT);
    Assert(!CTT->isDeadThread());
    Assert(CTT->isRunnable());

    //
    //  Note that we may not use the 'currentSolveBoard' test here,
    // because it may point to an irrelevant board!
    if (CTT->isInSolve()) {
      Board *tmpBB = GETBOARD(CTT);

      if (tmpBB->isSolve()) {
        //
        //  The same technique as by 'LBLterminateThread';
        SolveActor *sa = SolveActor::Cast (tmpBB->getActor ());
        Assert (sa);
        Assert (sa->getSolveBoard () == tmpBB);

        e->decSolveThreads(GETBOARD(sa));
      } else {
        e->decSolveThreads (tmpBB);
      }
    }
    e->disposeRunnableThread(CTT);
    e->unsetCurrentThread();

    goto LBLstart;
  }

  /* -----------------------------------------------------------------------
   * Suspend Thread
   * ----------------------------------------------------------------------- */

  /*
   *  Suspend the thread in a generic way. It's used when something
   * suspends in the sequential mode;
   *  Note that the thread stack should already contain the
   * "suspended" task;
   *
   *  Note that this code might be entered not only from within
   * the toplevel, so, it has to handle everything correctly ...
   *   *  Invariants:
   *  - CBB must be alive;
   *
   */
LBLsuspend:
  {
    DebugTrace(trace("suspend runnable thread", CBB));

    Assert(CTT);
    CTT->unmarkRunnable();

    Assert(CBB);
    Assert(!CBB->isFailed());
    //  see the note for the 'LBLterminateThread';
    Assert(CTT->isInSolve() || !e->isBelowSolveBoard());
    Assert(e->isBelowSolveBoard() || !CTT->isInSolve());

    //
    //  First, set the board and self, and perform special action for
    // the case of blocking the root thread;
    Assert(GETBOARD(CTT)==CBB);

#ifdef DEBUG_ROOT_THREAD
    // this can happen if \sw -threadedqueries,
    // or in non-threaded \feeds, e.g. suspend for I/O
    if (CTT==e->rootThread()) {
      printf("root blocked\n");
    }
#endif

    if (e->debugmode() && CTT->getTrace()) {
      debugStreamBlocked(CTT);
    } else if (CTT->getNoBlock()) {
      // ### mm2: && CAA == NULL) {
      (void) oz_raise(E_ERROR,E_KERNEL,"block",1,makeTaggedConst(CTT));
      CTT->markRunnable();
      e->exception.pc = NOCODE;
      goto LBLraise;
    }

    e->unsetCurrentThread();

    //  No counter decrement 'cause the thread is still alive!

    if (e->onToplevel()) {
      //
      //  Note that in this case no (runnable) thread counters
      // in solve actors can be affected, just because this is
      // a top-level thread;
      goto LBLstart;
    }

    //
    //  So, from now it's somewhere in a deep guard;
    Assert (!(e->onToplevel ()));

    goto LBLcheckEntailmentAndStability;
  }

  /* -----------------------------------------------------------------------
   * Fail Thread
   * ----------------------------------------------------------------------- */
  /*
   *  kost@ : There are now the following invariants:
   *  - Can be entered only in a deep guard;
   *  - current thread must be runnable.
   */
LBLfailure:
   {
     DebugTrace(trace("fail",CBB));

     Assert(CTT);
     Assert(CTT->isRunnable());

     Actor *aa=CBB->getActor();

     e->failBoard();

     // currentThread is a thread forked in a local space or a propagator
     if (aa->isSolve()) {

       //  Reduce (i.e. with failure in this case) the solve actor;
       //  The solve actor goes simply away, and the 'failed' atom is bound to
       // the result variable;
       aa->setCommitted();
       SolveActor *saa=SolveActor::Cast(aa);
       // don't decrement parent counter

       if (!e->fastUnifyOutline(saa->getResult(),saa->genFailed(),0)) {
         // this should never happen?
         Assert(0);
       }
       //       saa->dealloc();
     } else {
       AWActor *aw = AWActor::Cast(aa);
       Thread *tt = aw->getThread();

       Assert(CTT != tt && GETBOARD(tt) == CBB);
       Assert(!aw->isCommitted() && !aw->hasNext());

       if (aw->isWait()) {
         WaitActor *wa = WaitActor::Cast(aw);
         /* test bottom commit */
         if (wa->hasNoChildren()) {
           if (canOptimizeFailure(e,tt)) goto LBLfailure;
         } else {
           Assert(!e->isScheduledSlow(tt));
           /* test unit commit */
           if (wa->hasOneChildNoChoice()) {
             Board *waitBoard = wa->getLastChild();
             int succeeded = e->commit(waitBoard);
             if (!succeeded) {
               if (canOptimizeFailure(e,tt)) goto LBLfailure;
             }
           }
         }
       } else {
         Assert(!e->isScheduledSlow(tt));
         Assert(aw->isAsk());

         AskActor *aa = AskActor::Cast(aw);

         //  should we activate the 'else' clause?
         if (aa->isLeaf()) {  // OPT commit()
           aa->setCommitted();
           CBB->decSuspCount();
           TaskStack *ts = tt->getTaskStackRef();
           ts->discardActor();

           /* rule: if fi --> false */
           if (aa->getElsePC() == NOCODE) {
             aa->disposeAsk();
             if (canOptimizeFailure(e,tt)) goto LBLfailure;
           } else {
             Continuation *tmpCont = aa->getNext();
             ts->pushCont(aa->getElsePC(),
                          tmpCont->getY(), tmpCont->getG());
             if (tmpCont->getX()) ts->pushX(tmpCont->getX());
             aa->disposeAsk();
             e->suspThreadToRunnableOPT(tt);
             e->scheduleThread(tt);
           }
         }
       }
     }

#ifdef DEBUG_CHECK
     if (CTT==e->rootThread()) {
       printf("fail root thread\n");
     }
#endif

     e->decSolveThreads(CBB);
     e->disposeRunnableThread(CTT);
     e->unsetCurrentThread();

     goto LBLstart;
   }

  /* -----------------------------------------------------------------------
   * Raise exception on thread
   * ----------------------------------------------------------------------- */

LBLraise:
  {
    DebugCheck(ozconf.stopOnToplevelFailure,
               DebugTrace(tracerOn();trace("raise")));

    Assert(CTT && !CTT->isPropagator());

    Bool foundHdl;

    if (e->exception.debug) {
      OZ_Term traceBack;
      foundHdl =
        CTT->getTaskStackRef()->findCatch(CTT,e->exception.pc,
                                          &traceBack,e->debugmode());

      OZ_Term loc = oz_getLocation(CBB);
      e->exception.value = formatError(e->exception.info,e->exception.value,
                                       traceBack,loc);

    } else {
      foundHdl = CTT->getTaskStackRef()->findCatch(CTT);
    }

    if (foundHdl) {
      if (e->debugmode() && CTT->getTrace())
        debugStreamUpdate(CTT);
      e->xRegs[0] = e->exception.value;
      goto LBLrunThread;  // execute task with no preemption!
    }

    if (!e->onToplevel() &&
        OZ_eq(OZ_label(e->exception.value),OZ_atom("failure"))) {
      goto LBLfailure;
    }

    if (e->debugmode()) {
      OZ_Term exc = e->exception.value;
      // ignore system(kernel(terminate)) exception:
      if (OZ_isRecord(exc) &&
          OZ_eq(OZ_label(exc),OZ_atom("system")) &&
          OZ_subtree(exc,OZ_int(1)) != makeTaggedNULL() &&
          OZ_eq(OZ_label(OZ_subtree(exc,OZ_int(1))),OZ_atom("kernel")) &&
          OZ_eq(OZ_subtree(OZ_subtree(exc,OZ_int(1)),OZ_int(1)),
                OZ_atom("terminate")))
        ;
      else {
        CTT->setTrace(OK);
        CTT->setStep(OK);
        debugStreamException(CTT,e->exception.value);
        goto LBLpreemption;
      }
    }
    // else
    RefsArray argsArray = allocateRefsArray(1,NO);
    argsArray[0] = e->exception.value;
    if (e->defaultExceptionHdl) {
      CTT->pushCall(e->defaultExceptionHdl,argsArray,1);
    } else {
      if (!am.isStandalone())
        printf("\021");
      printf("Exception raise:\n   %s\n",toC(argsArray[0]));
      fflush(stdout);
    }
    goto LBLrunThread; // changed from LBLpopTaskNoPreempt; -BL 26.3.97
  }
}
