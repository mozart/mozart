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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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
#include "mozart_cpi.hh"

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
    Propagator * prop;
  } * queue;

public:
  void resize(void);

  void enqueue (Propagator * prop) {
    if (size == maxsize) resize ();
    tail = (tail + 1) & (maxsize - 1); // reason for maxsize to be power of 2
    queue[tail].prop = prop;
    size += 1;
  }

  Propagator * dequeue () {
    if (size == 0)
      OZ_error ( "Cannot dequeue from empty queue.");
    Propagator * prop = queue[head].prop;
    head = (head + 1) & (maxsize - 1);
    size -= 1;
    return prop;
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

  OZPRINT;
};

/*
 *  Local propagation queue of *propagators*;
 *
 */
class LocalPropagationStore : protected LocalPropagationQueue {
private:
  Bool useit;

public:
  LocalPropagationStore () {}

  Bool reset () {
    LocalPropagationQueue::reset ();
    return FALSE;
  }

  Bool isEmpty () {
    return (LocalPropagationQueue::isEmpty ());
  }

  void push (Propagator * prop) {
    enqueue (prop);
  }

  Propagator * pop () {
    return (dequeue ());
  }

  void setUseIt (void) { useit = TRUE; }
  void unsetUseIt (void) { useit = FALSE; }
  Bool isUseIt (void) { return (useit); }
  int getSize (void) { return LocalPropagationQueue::getSize(); }
};

extern LocalPropagationStore localPropStore;

#endif //__LPS_H__
