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

#include "value.hh"
#include "trace.hh"
#include "thr_int.hh"
#include "prop_int.hh"
#include "builtins.hh"
#include "debug.hh"
#include "solve.hh"
#include "space.hh"

# define CBB (oz_currentBoard())
# define CTT (oz_currentThread())


/*
 * check stability after thread is finished
 */
static
void oz_checkStability()
{
  // try to reduce a solve board;
  SolveActor *solveAA = SolveActor::Cast(oz_currentBoard()->getActor());
  Board      *solveBB = oz_currentBoard();
 
  if (oz_isStableSolve(solveAA)) {
    Assert(am.trail.isEmptyChunk());
    // all possible reduction steps require this; 

    if (!solveBB->hasSuspension()) {
      // 'solved';
      // don't unlink the subtree from the computation tree;
      am.trail.popMark();
      oz_currentBoard()->unsetInstalled();
      am.setCurrent(oz_currentBoard()->getParent());
      // don't decrement counter of parent board!

      int ret = oz_unify(solveAA->getResult(), solveAA->genSolved());
      Assert(ret==PROCEED);
      return;
    }

    // check for nonmonotonic propagators
    oz_solve_scheduleNonMonoSuspList(solveAA);
    if (! oz_isStableSolve(solveAA))
      return;

    WaitActor *wa = solveAA->getChoice();

    if (wa == NULL) {
      // "stuck" (stable without distributing waitActors);
      // don't unlink the subtree from the computation tree; 
      am.trail.popMark();
      oz_currentBoard()->unsetInstalled();
      am.setCurrent(oz_currentBoard()->getParent());

      // don't decrement counter of parent board!

      int ret = oz_unify(solveAA->getResult(), solveAA->genStuck());
      Assert(ret==PROCEED);
      return;
    }

    // to enumerate;

    if (wa->getChildCount()==1) {
      Assert(wa->isChoice());

      Board *waitBoard = wa->getChildRef();

      int ret=oz_commit(waitBoard);
      Assert(ret);
      DebugCode(am.threadsPool.unsetCurrentThread());
      return;
    }

    // give back number of clauses
    am.trail.popMark();
    oz_currentBoard()->unsetInstalled();
    am.setCurrent(oz_currentBoard()->getParent());

    // don't decrement counter of parent board!

    int ret = oz_unify(solveAA->getResult(), 
		       solveAA->genChoice(wa->getChildCount()));
    Assert(ret==PROCEED);
    return;
  }

  if (solveAA->getThreads() == 0) {
    // There are some external suspensions to this solver!

    oz_deinstallCurrent();

    TaggedRef newVar = oz_newVariable();
    TaggedRef result = solveAA->getResult();

    solveAA->setResult(newVar);

    int ret = oz_unify(result, solveAA->genUnstable(newVar));
    Assert(ret==PROCEED);
    return;
  }

  oz_deinstallCurrent();
  return;
} 

static
TaggedRef formatError(TaggedRef info,TaggedRef val,
		      OZ_Term traceBack,OZ_Term loc)
{
  OZ_Term d = OZ_record(OZ_atom("d"),
			oz_cons(OZ_atom("info"),
			     oz_cons(OZ_atom("stack"),
				  oz_cons(OZ_atom("loc"),
				       oz_nil()))));
  OZ_putSubtree(d,OZ_atom("stack"),traceBack);
  OZ_putSubtree(d,OZ_atom("loc"),loc);
  OZ_putSubtree(d,OZ_atom("info"),info);

  return OZ_adjoinAt(val,OZ_atom("debug"),d);
}

// check if failure has to be raised as exception on thread
static
int canOptimizeFailure(AM *e, Thread *tt)
{
  if (tt->hasCatchFlag() || oz_onToplevel()) { // catch failure
    if (tt->isSuspended()) {
      tt->pushCall(BI_fail,0,0);
      oz_wakeupThreadOPT(tt);
    } else {
      printf("WEIRD: failure detected twice");
#ifdef DEBUG_CHECK
      PopFrame(tt->getTaskStackRef(),PC,Y,G);
      Assert(PC==C_CALL_CONT_Ptr);
      Assert((TaggedRef)ToInt32(Y)==BI_fail);
      tt->pushCall(BI_fail,0,0);
#endif
    }
    return NO;
  } else {
    return OK;
  }
}

void scheduler() {
  register AM * const e	= &am;
  goto LBLstart;

  /* -----------------------------------------------------------------------
   * Get thread from queue
   * ----------------------------------------------------------------------- */
LBLstart:
  //  Assert(CTT==0); // TMUELLER

  // check status register
  e->checkStatus(OK);

  if (am.threadsPool.threadQueuesAreEmpty()) {
    e->suspendEngine();
  }
  am.threadsPool.setCurrentThread(am.threadsPool.getFirstThread());
  Assert(CTT);

  DebugTrace(ozd_trace("run thread"));

  // source level debugger & Thread.suspend
  if (CTT->getStop()) {
    am.threadsPool.unsetCurrentThread();  // byebye...
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
      switch (oz_installPath(bb)) {
      case INST_OK:
	break;
      case INST_REJECTED:
	goto LBLdiscardThread;
      case INST_FAILED:
	goto LBLfailure;
      }
    }
  }

  // Constraints are implicitly propagated
  CBB->unsetNervous();

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
    case T_SUSPEND_ACTOR:
      goto LBLsuspendActor;
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
  DebugTrace(ozd_trace("thread preempted"));
  Assert(GETBOARD(CTT)==CBB);
  /*  Assert(CTT->isRunnable()|| (CTT->isStopped())); ATTENTION */
  am.threadsPool.scheduleThreadInline(CTT, CTT->getPriority());
  am.threadsPool.unsetCurrentThread();
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
    DebugTrace(ozd_trace("thread terminated"));
    Assert(CTT);
    Assert(!CTT->isDeadThread());
    Assert(CTT->isRunnable());
    Assert(CTT->isWakeup() || CTT->isEmpty());

    //  Note that during debugging the thread does not carry 
    // the board pointer (== NULL) wenn it's running;
    // Assert (CBB == CTT->getBoard());

    Assert(CBB != (Board *) NULL);
    Assert(!CBB->isFailed());

    Assert(oz_onToplevel() ||
	   ((CTT->isInSolve() || !am.isBelowSolveBoard()) &&
	    (am.isBelowSolveBoard() || !CTT->isInSolve())));


    CBB->decSuspCount();

    oz_disposeThread(CTT);
    //am.threadsPool.unsetCurrentThread(); // TMUELLER

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
     * check stability there ('oz_checkStability ()'), and proceed 
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
    if (oz_onToplevel()) goto LBLstart;

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
	  DECSOLVETHREADS (nb, "a");
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
    DebugCode(am.threadsPool.unsetCurrentThread());

#ifdef DEBUG_NONMONOTONIC
    cout << "checkEntailment" << endl << flush;
#endif

    CBB->unsetNervous();

  // check for entailment of ASK and WAITTOP
    if ((CBB->isAsk() || CBB->isWaitTop()) && oz_entailment()) {
      Board *bb = CBB;
      oz_deinstallCurrent();
      int ret=oz_commit(bb);
      Assert(ret);
    } else if (CBB->isSolve()) {
      oz_checkStability();
    }

    // 
    //  deref nb, because it maybe just committed!
    if (nb) DECSOLVETHREADS (nb->derefBoard(), "b");
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

#ifdef DEBUG_THREADCOUNT
    GETBOARD(CTT)->resetLocalPropagatorQueue();
#endif

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

	DECSOLVETHREADS(GETBOARD(sa), "c");
      } else {
	DECSOLVETHREADS (tmpBB, "d");
      }
    }
    oz_disposeThread(CTT);
    am.threadsPool.unsetCurrentThread();

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
LBLsuspendActor:
  DebugTrace(ozd_trace("thread suspend on actor"));
  if (e->debugmode() && CTT->getTrace()) {
    debugStreamBlocked(CTT);
  }
  goto LBLsuspend1;

LBLsuspend:
  DebugTrace(ozd_trace("thread suspended"));
  if (e->debugmode() && CTT->getTrace()) {
    debugStreamBlocked(CTT);
  }
  // fall through

LBLsuspend1:
  {
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

    am.threadsPool.unsetCurrentThread();

    //  No counter decrement 'cause the thread is still alive!

    if (oz_onToplevel()) {
      //
      //  Note that in this case no (runnable) thread counters 
      // in solve actors can be affected, just because this is 
      // a top-level thread;
      goto LBLstart;
    }

    // 
    //  So, from now it's somewhere in a deep guard;
    Assert (!oz_onToplevel());

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
     DebugTrace(ozd_trace("thread failed"));

     Assert(CTT);
     Assert(CTT->isRunnable());

#ifdef DEBUG_THREADCOUNT
     GETBOARD(CTT)->resetLocalPropagatorQueue();
#endif

     Actor *aa=CBB->getActor();

     oz_failBoard();

     // currentThread is a thread forked in a local space or a propagator
     if (aa->isSolve()) {

       //  Reduce (i.e. with failure in this case) the solve actor;
       //  The solve actor goes simply away, and the 'failed' atom is bound to
       // the result variable; 
       aa->setCommittedActor();
       SolveActor *saa=SolveActor::Cast(aa);
       // don't decrement parent counter

       if (!oz_unify(saa->getResult(),saa->genFailed())) { // mm_u
	 // this should never happen?
	 Assert(0);
       }

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
	   Assert(!am.threadsPool.isScheduledSlow(tt));
	   /* test unit commit */
	   if (wa->hasOneChildNoChoice()) {
	     Board *waitBoard = wa->getLastChild();
	     int succeeded = oz_commit(waitBoard);
	     if (!succeeded) {
	       if (canOptimizeFailure(e,tt)) goto LBLfailure;
	     }
	   }
	 }
       } else {
	 Assert(!am.threadsPool.isScheduledSlow(tt));
	 Assert(aw->isAsk());

	 AskActor *aa = AskActor::Cast(aw);

	 //  should we activate the 'else' clause?
	 if (aa->isLeaf()) {  // OPT commit()
	   aa->setCommittedActor();
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
			  tmpCont->getY(), tmpCont->getCAP());
	     if (tmpCont->getX()) ts->pushX(tmpCont->getX());
	     aa->disposeAsk();
	     oz_wakeupThreadOPT(tt);
	   }
	 }
       }
     }
     
     DECSOLVETHREADS(CBB, "e");
     oz_disposeThread(CTT);
     am.threadsPool.unsetCurrentThread();

     goto LBLstart;
   }

  /* -----------------------------------------------------------------------
   * Raise exception on thread
   * ----------------------------------------------------------------------- */

LBLraise:
  {
    DebugCode(if (ozconf.stopOnToplevelFailure) {DebugTrace(ozd_tracerOn());});
    DebugTrace(ozd_trace("exception raised"));

    Assert(CTT);

    Bool foundHdl;

    if (e->exception.debug) {
      OZ_Term traceBack;
      foundHdl =
	CTT->getTaskStackRef()->findCatch(CTT,e->exception.pc,
					  e->exception.y, e->exception.cap,
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
      e->xRegs[0] = e->exception.value; // mm2: use pushX
      goto LBLrunThread;  // execute task with no preemption!
    }

    if (!oz_onToplevel() &&
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
    if (e->defaultExceptionHdl) {
      CTT->pushCall(e->defaultExceptionHdl,e->exception.value);
    } else {
      prefixError();
      fprintf(stderr,"Exception raise:\n   %s\n",toC(e->exception.value));
      fflush(stderr);
    }
    goto LBLrunThread; // changed from LBLpopTaskNoPreempt; -BL 26.3.97
  }
}
