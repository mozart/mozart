/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "lps.hh"
#endif

#include "lps.hh"
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

  while (size) {
    Propagator * prop = dequeue ();
    new_queue[index].prop = prop;
    index -= 1;
  }

  Assert(index == -1);
  delete queue;
  queue = new_queue;
  head = 0;
  tail = (size = old_size) - 1;
  maxsize = new_maxsize;
}

Bool LocalPropagationStore::propagate_locally () {
  Board *currentBoard = am.currentBoard();
  RefsArray args;

  // kost@ : --> let's try ...
  Assert (currentBoard->getSuspCount () >= getSize ());

  while (!(isEmpty ())) {
    Propagator * prop = pop ();
    Propagator::setRunningPropagator(prop);
    Assert(am.isCurrentBoard(GETBOARD(prop)));
    //
    //  No 'runnable' threads are allowed here,
    // because only true propagators are in the LPS;
    Assert (! prop->isDeadPropagator() );

    OZ_Return ret_val;

    ret_val = am.runPropagator(prop);

    switch (ret_val) {
    case FAILED:
      if (am.onToplevel()) {
        errorHeader();

        ostrstream buf;
        buf << prop->getPropagator()->toString() << '\0';
        char *str = buf.str();
        message("Propagator %s failed\n", str);
        delete str;
      }
      am.closeDonePropagator(prop);
      return reset();

    case RAISE:
      error("propagators can't raise exceptions");

    case SUSPEND:
      error ("propagate_locally: 'SUSPEND' is returned?\n");

    case SLEEP:
      am.suspendPropagator(prop);
      break;

    case SCHEDULED:
      am.scheduledPropagator(prop);
      break;

    case PROCEED:
      am.closeDonePropagator(prop);
      break;
    }

  }
  return (TRUE);
}
