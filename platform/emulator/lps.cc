/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller,popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "lps.hh"
#endif

#include "am.hh"

#include "genvar.hh"
#include "fdprofil.hh"


#ifdef OUTLINE
#define inline
#endif

//-----------------------------------------------------------------------------

LocalPropagationStore localPropStore;

void LocalPropagationQueue::resize () {
  int new_maxsize = maxsize * 2;
  queue_t * new_queue = ::new queue_t[new_maxsize];
  int index = size - 1;
  int old_size = size;

  DebugCode(message("Resizing local propagation queue 0x%x --> 0x%x.\n",
		    maxsize, new_maxsize));
  
  while (size) {
    Thread *thr = dequeue ();
    new_queue[index].thr = thr;
    index -= 1;
  }

  Assert(index == -1);
  delete queue;
  queue = new_queue;
  head = 0;
  tail = (size = old_size) - 1;
  maxsize = new_maxsize;
}

void LocalPropagationQueue::printDebug () {
  int psize = size, phead = head;

  for (; psize; psize --) {
    cout << "lpqueue[" << phead << "]="
	 << "@" << queue[phead].thr << endl;

    phead = (phead + 1) & (maxsize - 1);
  }
}

void LocalPropagationQueue::printDebugLong () {
  int psize = size, phead = head;

  for (; psize; psize --) {
    cout << "lpqueue[" << phead << "]="
	 << "@" << queue[phead].thr << "(";
    queue[phead].thr->print(cout);
    cout << ")" << endl;

    phead = (phead + 1) & (maxsize - 1);
  }
}

#ifdef DEBUG_CHECK
Bool LocalPropagationStore::checkIsPropagator (Thread *thr)
{
  return (thr->isPropagator());
}
#endif

Bool LocalPropagationStore::propagate_locally () {
  Board *currentBoard = am.currentBoard;
  Thread *savedCurrentThread = am.currentThread;
  RefsArray args;

  /*
   *  We save the actual 'am.currentThread' pointer and restore
   * it before returning. This is because propagators must have 
   * an access to themselves; this is provided by setting the 
   * (global) 'am.currentThread' to an actual propagator (thread);
   * 
   */
  // kost@ : --> let's try ...
  Assert (currentBoard->getSuspCount () >= getSize ());
	 
  while (!(isEmpty ())) {
    Thread *thr = am.currentThread = pop ();

    Assert (am.currentThread != (Thread *) NULL);
    Assert (am.currentBoard == GETBOARD(thr));
    //
    //  No 'runnable' threads are allowed here, 
    // because only true propagators are in the LPS;
    Assert (!(am.currentThread->isDeadThread ()));
    Assert (am.currentThread->isPropagator ());

    OZ_Return ret_val;
    
    ret_val = am.runPropagator(thr);

    switch (ret_val) {
    case FAILED:
      if (am.currentBoard->isRoot ()) {
	errorHeader();

	ostrstream buf; 
	buf << thr->getPropagator()->toString() << '\0';
	char *str = buf.str();
	message("Propagator %s failed\n", str);
	delete str;
      }
      am.closeDonePropagator(am.currentThread);
      am.currentThread = savedCurrentThread;
      return reset();

    case RAISE:
      error("propagators can't raise exceptions");

    case SUSPEND:
      error ("propagate_locally: 'SUSPEND' is returned?\n");

    case SLEEP:
      am.suspendPropagator(am.currentThread);
      break;

    case SCHEDULED:
      am.scheduledPropagator(am.currentThread);
      break;

    case PROCEED:
      am.closeDonePropagator(am.currentThread);
      break;
    }
    
  }
  am.currentThread = savedCurrentThread;
  return (TRUE);
}


#ifdef OUTLINE
#define inline
#include "lps.icc"
#undef inline
#endif
