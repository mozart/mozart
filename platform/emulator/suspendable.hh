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

#define DEFAULT_PRIORITY	MID_PRIORITY

#define PRIORITY_SHIFT 17

enum SuspendableFlags {
  // THIS MUST BE IN THE LOWEST TWO BITS!
  SF_GcMark   = 1 << 0,
  SF_MultiMark= 1 << 1,

  // Flags common to both threads and propagators
  SF_Dead     = 1 << 2,
  SF_Tagged   = 1 << 3,
  SF_Runnable = 1 << 4,
  SF_External = 1 << 5,

  // Flags for propagators
  SF_NMO      = 1 << 6,
  SF_Local    = 1 << 7,
  SF_OFS      = 1 << 8,
  SF_Unify    = 1 << 9,
  SF_Failed   = 1 << 10,
  SF_Active   = 1 << 11,

  // Flags for threads
  SF_Catch    = 1 << 12,
  SF_Trace    = 1 << 13,
  SF_Step     = 1 << 14,
  SF_Stop     = 1 << 15,
  SF_NoBlock  = 1 << 16,
  
  // Thread priorities reserve two bits
  SF_PriMask  = 3 << PRIORITY_SHIFT,
  // Encoding is as follows:
  // 0:  this is a propagator
  // >0: this is a thread
  
};

#define FLAGTESTS(FLAG)				\
  int is ## FLAG(void) {			\
    return flags & SF_ ## FLAG;			\
  }						\
  void set ## FLAG(void) {			\
    flags |= SF_ ## FLAG;			\
  }						\
  void unset ## FLAG(void) {			\
    flags &= ~SF_ ## FLAG;			\
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
  int isCacMarked(void) {
    return flags & SF_GcMark;
  }
  void cacMark(Suspendable * fwd) {
    Assert(!isCacMarked());
    flags = ((int32) fwd) | SF_GcMark ; 
  }
  Suspendable * cacGetFwd(void) {
    Assert(isCacMarked());
    return (Suspendable *) (flags & ~SF_GcMark);
  }
  void ** cacGetMarkField(void) { 
    return (void **) (void *) &flags; 
  };

  Suspendable * gCollectSuspendableInline(Bool);
  Suspendable * gCollectSuspendable(void);
  Suspendable * sCloneSuspendableInline(Bool);
  Suspendable * sCloneSuspendable(void);

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

  Bool _wakeup(Board *, PropCaller);
  Bool _wakeup_outline(Board *, PropCaller);
  Bool _wakeupLocal(Board *, PropCaller);
  Bool _wakeupAll(void);

  /*
   * Common to threads and propagators
   */
  FLAGTESTS(MultiMark)
  FLAGTESTS(Dead)
  FLAGTESTS(Tagged)
  FLAGTESTS(Runnable)
  FLAGTESTS(External)

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
  FLAGTESTS(Active)
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
  Assert(!s || s->isCacMarked() || s->isPropagator());
  return (Propagator *) s;
}

inline
Thread * SuspToThread(Suspendable * s) {
  Assert(!s || s->isCacMarked() || s->isThread());
  return (Thread *) s;
}

#else

#define SuspToPropagator(s) ((Propagator *) s)
#define SuspToThread(s)     ((Thread *) s)

#endif

#endif
