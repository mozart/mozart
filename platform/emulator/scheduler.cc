/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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
#include "space.hh"

# define CBB (oz_currentBoard())

inline
TaggedRef formatError(TaggedRef info, TaggedRef val, TaggedRef traceBack) {
  OZ_Term d = OZ_record(AtomD,
                        oz_cons(AtomInfo,
                                oz_cons(AtomStack, oz_nil())));
  OZ_putSubtree(d, AtomStack, traceBack);
  OZ_putSubtree(d, AtomInfo,  info);

  return OZ_adjoinAt(val, AtomDebug, d);
}


void scheduler() {
  register AM * const e = &am;

  register Thread * ct;
  register Board  * cb;

  do {

    /*
     * Do everything that is expensive and ugly: garbage collection...
     *
     */

    e->checkStatus(OK);


    /*
     * Get thread from queue
     *
     */

    do {

      ct = e->threadsPool.getNext();

      if (ct == (Thread *) NULL) {
        e->suspendEngine();
        continue;
      }

      if (!ct->getStop())
        break;

    } while (1);

    Assert(ct);

    // The thread must be alive and kicking, ready to rumble!
    Assert(!ct->isDeadThread() && ct->isRunnable());


    /*
     * Install space in which thread is situated
     *
     */

    cb = ct->getBoardInternal()->derefBoard();

    if (!cb->isAlive()) {
      // Some space superordinated to ct's home is dead

      if (!cb->isRoot())
        cb->getParent()->decSolveThreads();

      oz_disposeThread(ct);
      continue;

    }

    if (!oz_installPath(cb))
      goto LBLfailure;

    e->restartThread(); // start a new time slice
    // fall through


    /*
     * Run thread
     *
     */

  LBLrunThread:
    {

      am.threadsPool.setCurrentThread(ct);
      ozstat.leaveCall(ct->abstr);
      ct->abstr = 0;
      e->cachedStack = ct->getTaskStackRef();
      e->cachedSelf  = (Object *) 0;

      int ret = engine(NO);

      am.threadsPool.unsetCurrentThread();

      ct->setAbstr(ozstat.currAbstr);
      ozstat.leaveCall(NULL);

      if (e->getSelf()) {
        ct->pushSelf(e->getSelf());
        e->cachedSelf = (Object *) NULL;
      }

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


    /*
     * preemption
     *
     */

  LBLpreemption:
    {
      am.threadsPool.scheduleThread(ct);

      continue;
    }


    /*
     * An error occured
     *
     */

  LBLerror:
    {
      Assert(0);
      continue;
    }



    /*
     * Thread is terminated
     *
     */

  LBLterminate:
    {
      Assert(!ct->isDeadThread() && ct->isRunnable() && ct->isEmpty());

      CBB->decSuspCount();

      oz_disposeThread(ct);

      // fall through to checkEntailmentAndStability
    }


    /*
     * Check stability
     *
     */

  LBLcheckEntailmentAndStability:
    {

      if (oz_onToplevel())
        continue;

      Assert(!CBB->isRoot() && !CBB->isFailed() && !CBB->isCommitted());

      //  'nb' points to some board above the current one,
      Board * nb = CBB->getParent();

      //  kost@ : optimize the most probable case!

      if (CBB->decThreads () != 0) {
        nb->decSolveThreads();
        continue;
      }

      Assert(!CBB->isRoot());

      CBB->checkStability();

      //  deref nb, because it maybe just committed!

      Assert(nb);

      if (nb)
        nb->derefBoard()->decSolveThreads();

      continue;
    }

    /*
     * Suspend Thread
     *
     */

  LBLsuspend:
    {

      if (e->debugmode() && ct->getTrace()) {
        debugStreamBlocked(ct);
      }

      ct->unmarkRunnable();

      Assert(CBB);
      Assert(!CBB->isFailed());

      //  First, set the board and self, and perform special action for
      // the case of blocking the root thread;
      Assert(GETBOARD(ct)==CBB);

      if (oz_onToplevel())
        continue;
      else
        goto LBLcheckEntailmentAndStability;

    }

    /*
     * Fail Thread
     *
     *  - can be entered only in subordinated space
     *  - current thread must be runnable
     */

  LBLfailure:
    {
      Assert(ct->isRunnable());

      Board * b = CBB;
      Board * p = b->getParent();

      Assert(!b->isRoot());

      b->setFailed();

      oz_reduceTrailOnFail();

      am.setCurrent(p);

      if (!oz_unify(b->getStatus(),b->genFailed())) {
        // this should never happen?
        Assert(0);
      }

      p->decSolveThreads();

      // tmueller: this experimental
#ifdef NAME_PROPAGATORS
      if (!e->isPropagatorLocation())
        oz_disposeThread(ct);
#endif

      continue;
    }


    /*
     * Raise exception
     *
     */

  LBLraise:
    {

      Bool foundHdl;

      if (e->exception.debug) {
        OZ_Term traceBack;
        foundHdl =
          ct->getTaskStackRef()->findCatch(ct,
                                           e->exception.pc,
                                           e->exception.y,
                                           e->exception.cap,
                                           &traceBack,
                                           e->debugmode());

        e->exception.value = formatError(e->exception.info,e->exception.value,
                                         traceBack);

      } else {
        foundHdl = ct->getTaskStackRef()->findCatch(ct);
      }

      if (foundHdl) {
        if (e->debugmode() && ct->getTrace())
          debugStreamUpdate(ct);
        e->xRegs[0] = e->exception.value; // mm2: use pushX
        goto LBLrunThread;  // execute task with no preemption!
      }

      if (!oz_onToplevel() &&
          OZ_eq(OZ_label(e->exception.value),AtomFailure)) {
        goto LBLfailure;
      }

      if (e->debugmode()) {
        OZ_Term exc = e->exception.value;
        // ignore system(kernel(terminate)) exception:
        if (OZ_isRecord(exc) &&
            OZ_eq(OZ_label(exc),AtomSystem) &&
            OZ_subtree(exc,OZ_int(1)) != makeTaggedNULL() &&
            OZ_eq(OZ_label(OZ_subtree(exc,OZ_int(1))),E_KERNEL) &&
            OZ_eq(OZ_subtree(OZ_subtree(exc,OZ_int(1)),OZ_int(1)),
                  OZ_atom("terminate")))
          ;
        else {
          ct->setTrace(OK);
          ct->setStep(OK);
          debugStreamException(ct,e->exception.value);
          goto LBLpreemption;
        }
      }

      // else
      if (e->defaultExceptionHdl) {
        ct->pushCall(e->defaultExceptionHdl,e->exception.value);
      } else {
        prefixError();
        fprintf(stderr,
                "Exception raised:\n   %s\n",
                OZ_toC(e->exception.value,100,100));
        fflush(stderr);
      }

      goto LBLrunThread;

    }

  } while(1);

}
