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
#include "pointer-marks.hh"

/*
 * A suspendable provides the interface that is common to
 * threads and propagators.
 *
 */

enum SuspendableFlags {

  // Thread priorities reserve the lower two bits
  SF_PriMask  = 0x000003,
  // Encoding is as follows:
  // 0:  this is a propagator
  // >0: this is a thread
  
  // Flags common to both threads and propagators
  SF_Dead     = 0x000004,
  SF_Tagged   = 0x000008,
  SF_Runnable = 0x000010,
  SF_External = 0x000020,

  // Flags for propagators
  SF_NMO      = 0x000040,
  SF_Local    = 0x000080,
  SF_OFS      = 0x000100,
  SF_Unify    = 0x000200,
  SF_Failed   = 0x000400,

  // Flags for threads
  SF_Catch    = 0x000800,
  SF_Trace    = 0x001000,
  SF_Step     = 0x002000,
  SF_Stop     = 0x004000,
  SF_NoBlock  = 0x008000,

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
  int    flags;
  void * board;

public:
  NO_DEFAULT_CONSTRUCTORS(Suspendable);
  USEFREELISTMEMORY;

  Suspendable(int f, Board * b) : flags(f), board((Board *) b) {}


  /*
   * Generic garbage collection part
   */
  int isGcMarked(void) {
    return IsMarkedPointer(board,1);
  }
  void gcMark(Suspendable * fwd) {
    Assert(!isGcMarked());
    board = MarkPointer(fwd,1); 
  }
  Suspendable * gcGetFwd() {
    Assert(isGcMarked());
    return (Suspendable *) UnMarkPointer(board,1);
  }
  void ** gcGetMarkField() { 
    return &board; 
  };


  /*
   * Board handling
   */
  Board * getBoardInternal(void) {
    Assert(!isGcMarked());
    return (Board *) board;
  }
  void setBoardInternal(Board * b) {
    Assert(!isGcMarked());
    board = (void *) b;
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
    return flags & SF_PriMask;
  }
  void setPriority(int p) {
    Assert(isThread());
    flags = (flags & ~SF_PriMask) | p;
  }


};

#undef FLAGTESTS

#endif
