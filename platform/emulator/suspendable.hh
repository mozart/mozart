/*
 *  Authors:
 *    Konstantin Popov (popow@ps.uni-sb.de)
 *    Tobias Müller (tmueller@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Copyright:
 *    Konstantin Popov, 1999
 *    Tobias Müller, 1999
 *    Christian Schulte, 1999
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

#ifndef __SUSPENDABLE_HH__
#define __SUSPENDABLE_HH__

#include "base.hh"
#include "mem.hh"

/*
 * A suspendable provides the interface that is common to
 * threads and propagators.
 *
 */

/* thread priorities */

#define HI_PRIORITY             3
#define MID_PRIORITY            2
#define LOW_PRIORITY            1

#define DEFAULT_PRIORITY        MID_PRIORITY

#define PRIORITY_SHIFT 15

enum SuspendableFlags {
  // THIS MUST BE IN THE LOWEST TWO BITS!
  SF_GcMark   = 1 << 0,

  // Flags common to both threads and propagators
  SF_Dead     = 1 << 1,
  SF_Tagged   = 1 << 2,
  SF_Runnable = 1 << 3,
  SF_External = 1 << 4,

  // Flags for propagators
  SF_NMO      = 1 << 5,
  SF_Local    = 1 << 6,
  SF_OFS      = 1 << 7,
  SF_Unify    = 1 << 8,
  SF_Failed   = 1 << 9,

  // Flags for threads
  SF_Catch    = 1 << 10,
  SF_Trace    = 1 << 11,
  SF_Step     = 1 << 12,
  SF_Stop     = 1 << 13,
  SF_NoBlock  = 1 << 14,

  // Thread priorities reserve two bits
  SF_PriMask  = 3 << PRIORITY_SHIFT,
  // Encoding is as follows:
  // 0:  this is a propagator
  // >0: this is a thread

};

#define FLAGTESTS(FLAG) \
  int is ## FLAG(void) {         \
    return flags & SF_ ## FLAG;  \
  }                              \
  void set ## FLAG(void) {       \
    flags |= SF_ ## FLAG;        \
  }                              \
  void unset ## FLAG(void) {     \
    flags &= ~SF_ ## FLAG;       \
  }

class Suspendable {
protected:
  int     flags;
  Board * board;

public:
  NO_DEFAULT_CONSTRUCTORS(Suspendable);
  USEFREELISTMEMORY;

  Suspendable(int f, Board * b) : flags(f), board(b) {}


  /*
   * Generic garbage collection part
   */
  int isGcMarked(void) {
    return flags & SF_GcMark;
  }
  void gcMark(Suspendable * fwd) {
    Assert(!isGcMarked());
    flags = ((int32) fwd) | SF_GcMark ;
  }
  Suspendable * gcGetFwd() {
    Assert(isGcMarked());
    return (Suspendable *) (flags & ~SF_GcMark);
  }
  void ** gcGetMarkField() {
    return (void **) (void *) &flags;
  };

  Suspendable * gcSuspendableInline(void);
  Suspendable * gcSuspendable(void);

  /*
   * Board handling
   */
  Board * getBoardInternal(void) {
    return board;
  }
  void setBoardInternal(Board * b) {
    board = b;
  }


  /*
   * Come in and find out...
   */
  int getFlags(void) {
    return flags;
  }
  int isThread(void) {
    return flags & SF_PriMask;
  }
  int isPropagator(void) {
    return !isThread();
  }

  /*
   * Common to threads and propagators
   */
  FLAGTESTS(Dead)
  FLAGTESTS(Tagged)
  FLAGTESTS(Runnable)
  FLAGTESTS(External)

  Bool _wakeup(Board *, PropCaller);
  Bool wakeup(Board *, PropCaller);

  Bool _wakeupLocal(PropCaller);

  /*
   * Threads
   */
  FLAGTESTS(Catch)
  FLAGTESTS(Trace)
  FLAGTESTS(Step)
  FLAGTESTS(Stop)
  FLAGTESTS(NoBlock)

  /*
   * Propagators
   */
  FLAGTESTS(NMO)
  FLAGTESTS(Local)
  FLAGTESTS(OFS)
  FLAGTESTS(Failed)
  FLAGTESTS(Unify)

  /*
   * Threads
   */
  int getPriority(void) {
    Assert(isThread());
    return flags >> PRIORITY_SHIFT;
  }
  void setPriority(int p) {
    Assert(isThread());
    flags = (flags & ~SF_PriMask) | (p << PRIORITY_SHIFT);
  }


  /*
   * Misc nonsense
   */

  OZPRINTLONG;

};

#undef FLAGTESTS

#ifdef DEBUG_CHECK

inline
Propagator * SuspToPropagator(Suspendable * s) {
  Assert(!s || s->isGcMarked() || s->isPropagator());
  return (Propagator *) s;
}

inline
Thread * SuspToThread(Suspendable * s) {
  Assert(!s || s->isGcMarked() || s->isThread());
  return (Thread *) s;
}

#else

#define SuspToPropagator(s) ((Propagator *) s)
#define SuspToThread(s)     ((Thread *) s)

#endif

#endif
