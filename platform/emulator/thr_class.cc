/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  implementation of threads and queues of threads with priorities
  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "thread.hh"
#endif

/* some random comments on threads:

   threads are 'switch'ed, if
     - 'timed out' by alarm clock
     - 'finish'ed
     - 'displace'd by higher priority
     - 'freeze'd

   need for thread switching is signaled by 'ThreadSwitch' S-Flag

   "current thread"
     - the currently running thread

   */

#include "am.hh"

#ifdef OUTLINE
#define inline
#include "thread.icc"
#undef inline
#endif

// --------------------------------------------------------------------------

/*
 *  Threads;
 *
 */
void Thread::setExtThreadOutlined (Board *varHome)
{
  Board *bb = am.currentBoard;
  Bool wasFound = NO;
  Assert (!(varHome->isCommitted ()));

  while (bb != varHome) {
    Assert (!(bb->isRoot ()));
    Assert (!(bb->isCommitted ()) && !(bb->isFailed ()));
    if (bb->isSolve ()) {
      SolveActor *sa = SolveActor::Cast (bb->getActor ());
      sa->addSuspension (this);
      wasFound = OK;
    }
    bb = bb->getParentFast ();
  }

  if (wasFound) {
    setExtThread ();

    //  ... and again to the 'SolveActor::checkExtSuspList':
    // we should not produce 'external' threads for propagators,
    // because stability can be achieved only when the last such
    // a propagator dissapears;
    if (isPropagator ()) {
      warning ("'External' propagator thread: stability check is implemented *partially*!");
    }
  }
}

//
void Thread::checkExtThreadOutlined ()
{
  Assert (wasExtThread ());

  Board *sb = (getBoardFast ())->getSolveBoard ();
  AM *e = &am;

  while (sb) {
    Assert (sb->isSolve());

    SolveActor *sa = SolveActor::Cast (sb->getActor ());
    if (e->isStableSolve (sa)) {
#ifdef NEWCOUNTER
      e->scheduleThread (new Thread (sa->getPriority (), sb));
#else
      does not work;
      e->scheduleSolve (sb);
#endif
    }
    sb = (sa->getBoardFast ())->getSolveBoard ();
  }
}

//
void Thread::addStableThreadOutlined ()
{
  (SolveActor::Cast (am.currentSolveBoard->getActor ()))
    ->add_stable_susp (this);
}

/*
 * check if a thread's board is below a failed board
 */
Bool Thread::isBelowFailed (Board *top)
{
  Assert (isRunnable ());

  Board *bb=getBoardFast();
  while (bb!=top) {
    if (bb->isFailed()) {
      return TRUE;
    }
    bb=bb->getParentFast();
  }
  return FALSE;
}

/*
 * remove local tasks
 * return OK, if done
 * return NO, if no C_LOCAL/C_SOLVE found
 */
Bool Thread::discardLocalTasks()
{
  TaskStack *ts;
  TaskStackEntry *tos;
  Assert (hasStack ());

  ts = &(item.threadBody->taskStack);
  tos = ts->getTop ();
  while (TRUE) {
    TaskStackEntry entry=*(--tos);
    if (ts->isEmpty (entry)) {
      ts->setTop (tos+1);
      return (NO);
    }

    ContFlag cFlag = getContFlag(ToInt32(entry));

    switch (cFlag) {
    case C_LOCAL:
    case C_SOLVE:
      ts->setTop (tos);
      return (OK);

    case C_JOB:
      {
        DebugCode (ts->setTop (tos));
        Bool hasJobs = ts->getJobFlagFromEntry(entry);
        if (!hasJobs) unsetHasJobs();
      }
      break;

    default:
      tos = tos - ts->frameSize(cFlag) + 1;
      break;
    }
  }
}

//
//
#ifdef DEBUG_CHECK
Bool Thread::hasJobDebug ()
{
  Assert (hasStack ());

  return (item.threadBody->taskStack.hasJobDebug ());
}
#endif

//
int Thread::findExceptionHandler(Chunk *&chunk, TaskStackEntry *&oldTos)
{
  int spaceCount=0;
  TaskStack *ts = &(item.threadBody->taskStack);
  TaskStackEntry *tos = ts->getTop ();

  oldTos=tos;
  while (1) {
    TaskStackEntry entry=*(tos-1);
    if (ts->isEmpty(entry)) {
      ts->setTop (tos);
      chunk=0;
      return spaceCount;
    }

    ContFlag cFlag = getContFlag(ToInt32(entry));

    switch (cFlag) {
    case C_EXCEPT_HANDLER:
      chunk = (Chunk*) *(tos-2);
      ts->setTop (tos-2);
      return spaceCount;
    case C_LOCAL:
    case C_SOLVE:
      spaceCount++;
      break;
    case C_JOB:
      {
        DebugCode (ts->setTop (tos-1));
        Bool hasJobs = ts->getJobFlagFromEntry (entry);
        if (!hasJobs) unsetHasJobs();
      }
      break;
    default:
      break;
    }
    tos = tos - ts->frameSize (cFlag);
  }
}
