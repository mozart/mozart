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

#ifndef __LPS_H__
#define __LPS_H__

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "oz_cpi.hh"

/*
 *  Local propagation store;
 *
 */ 

class LocalPropagationQueue {
private:
  int head, tail, size, maxsize;
#ifdef DEBUG_CHECK  
  enum {initmaxsize = 0x10}; // expected to be power of 2
#else
  enum {initmaxsize = 0x1000}; // expected to be power of 2
#endif
  struct queue_t {
    Thread *thr;
  } *queue;

public:
  void resize(void);
  
  void enqueue (Thread *thr) {
    if (size == maxsize) resize ();
    tail = (tail + 1) & (maxsize - 1); // reason for maxsize to be power of 2
    queue[tail].thr = thr;
    size += 1;
  }

  Thread *dequeue () {
    if (size == 0) 
      error ( "Cannot dequeue from empty queue.");
    Thread *thr = queue[head].thr;
    head = (head + 1) & (maxsize - 1);
    size -= 1;
    return (thr);
  }

  LocalPropagationQueue ()
  : head (0), tail (initmaxsize - 1), size (0), maxsize (initmaxsize)
  {
    queue = new queue_t[maxsize];
  }
  
  Bool isEmpty () { return (size == 0); }
  void reset () {
    head = 0; 
    tail = maxsize - 1; 
    size = 0;
  }
  int getSize () { return (size); }
  void printDebug ();
  void printDebugLong ();
};

/*
 *  Local propagation queue of *propagators*;
 *
 */
class LocalPropagationStore : protected LocalPropagationQueue {
private:
  Bool propagate_locally ();
  Bool useit;

  DebugCode (Bool checkIsPropagator (Thread *thr);)
public:
  LocalPropagationStore () {}

  Bool reset () {
    LocalPropagationQueue::reset ();
    return FALSE;
  }

  Bool isEmpty () {
    return (LocalPropagationQueue::isEmpty ());
  }

  Bool do_propagation () {
    if (!isEmpty ()) return (propagate_locally ());
    return (TRUE);
  }

  void push (Thread *thr) {
    Assert (checkIsPropagator (thr));
    enqueue (thr);
  }

  Thread *pop () {
    return (dequeue ());
  }

  void setUseIt () { useit = TRUE; }
  void unsetUseIt () { useit = FALSE; }
  Bool isUseIt () { return (useit); }
};

extern LocalPropagationStore localPropStore;

#endif //__LPS_H__
