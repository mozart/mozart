/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Contributors:
 *
 *  Copyright:
 *    Kostja Popow, 1997-1999
 *    Michael Mehl, 1997-1999
 *    Christian Schulte, 1997-1999
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

#ifndef __BOARDH
#define __BOARDH

#ifdef INTERFACE
#pragma interface
#endif

#include "thr_class.hh"
#include "distributor.hh"
#include "suspendable.hh"
#include "susp_queue.hh"

#define GETBOARD(v) ((v)->getBoardInternal()->derefBoard())


enum BoardTags {
  BoTag_Root       = 1,
  BoTag_Failed     = 2,
  BoTag_Committed  = 3,
  BoTag_MarkOne    = 4,
  BoTag_MarkTwo    = 8
};

#define BoTag_StatusMask 3
#define BoTag_AllMask    15

class Board {
  friend int engine(Bool init);
public:
  NO_DEFAULT_CONSTRUCTORS(Board);
  Board(void);
  Board(Board *b);

  USEHEAPMEMORY;

  //
  // Home and parent access
  //
private:
  Tagged4 parentAndFlags;

public:
  //
  // State of the space
  //
  int isRoot(void) {
    return (parentAndFlags.getTag() & BoTag_StatusMask) == BoTag_Root;
  }
  int isFailed(void) {
    return (parentAndFlags.getTag() & BoTag_StatusMask) == BoTag_Failed;
  }
  int isCommitted(void) {
    return (parentAndFlags.getTag() & BoTag_StatusMask) == BoTag_Committed;
  }

  void setCommitted(Board * b) {
    Assert(parentAndFlags.getTag() == 0);
    parentAndFlags.set((void *) b, BoTag_Committed);
  }
  void setFailed(void) {
    Assert(parentAndFlags.getTag() == 0);
    parentAndFlags.setTag(BoTag_Failed);
  }
  int getTag(void) {
    return parentAndFlags.getTag();
  }
  void setParent(Board * p) {
    Assert(!parentAndFlags.getTag())
    parentAndFlags.set(p,0);
  }
  //
  // Various marks needed during installation, cloning and garbage
  // collection
  //

  int hasMarkOne(void) {
    return parentAndFlags.getTag() & BoTag_MarkOne;
  }
  int hasMarkTwo(void) {
    return parentAndFlags.getTag() & BoTag_MarkTwo;
  }

  void setMarkOne(void) {
    parentAndFlags.borTag(BoTag_MarkOne);
  }
  void setMarkTwo(void) {
    parentAndFlags.borTag(BoTag_MarkTwo);
  }

  void unsetMarkOne(void) {
    parentAndFlags.bandTag((~BoTag_MarkOne) & BoTag_AllMask);
  }
  void unsetMarkTwo(void) {
    parentAndFlags.bandTag((~BoTag_MarkTwo) & BoTag_AllMask);
  }


  //
  // Parent access
  //
  Board * getParentInternal(void) {
    return (Board *) parentAndFlags.getPtr();
  }
  Board *derefBoard(void) {
    Board *bb;
    for (bb=this; bb->isCommitted(); bb=bb->getParentInternal()) {}
    return bb;
  }
  Board *getParent(void) {
    Assert(!isCommitted());
    return getParentInternal()->derefBoard();
  }
  int isAlive(void) {
    for (Board * s = this; !s->isRoot() ; s=s->getParent())
      if (s->isFailed())
        return NO;
    return OK;
  }
  int isAdmissible(void);

  //
  // Garbage collection and copying
  //
public:
  Bool cacIsMarked(void) {
    return hasMarkTwo();
  }
  Board * cacGetFwd(void) {
    Assert(cacIsMarked());
    return getParentInternal();
  }
  int32 ** cacGetMarkField(void) {
    return (int32 **) &parentAndFlags;
  }
  void cacMark(Board * fwd) {
    Assert(!cacIsMarked());
    parentAndFlags.set((void *) fwd, BoTag_MarkTwo);
  }

  Bool    cacIsAlive(void);

  Board * sCloneBoard(void);
  Board * sCloneBoardDo(void);
  void    sCloneRecurse(void);
  void    sCloneMark(Board *);

  Board * gCollectBoard(void);
  Board * gCollectBoardDo(void);
  void    gCollectRecurse(void);
  void    gCollectMark(Board *);

  void unsetGlobalMarks(void);
  void setGlobalMarks(void);

  //
  // Local suspendable counter
  //
private:
  int suspCount;

public:
  void incSuspCount(int n=1) {
    Assert(!isCommitted() && !isFailed());
    suspCount += n;
    Assert(suspCount >= 0);
  }
  void decSuspCount(void) {
    Assert(!isCommitted() && !isFailed());
    Assert(suspCount > 0);
    suspCount--;
  }
  int getSuspCount(void) {
    Assert(!isFailed() && suspCount >= 0);
    return suspCount;
  }
  Bool isStable(void);
  Bool isBlocked(void);
  void checkStability(void);
  void checkExtSuspension(Suspendable *);

  //
  // Cascaded runnable thread counter
  //
private:
  int crt;
public:
  int hasRunnableThreads(void) {
    return (crt > 0);
  }
  void incRunnableThreads(void);
  void decRunnableThreads(void);

  //
  // All taggedrefs in a row for garbage collection
  //
private:
  TaggedRef script, status, rootVar;

  //
  // Script and script installation
  //
private:
  OZ_Return installScript(Bool isMerging);

  Bool installDown(Board *);

  void setScript(TaggedRef s) {
    script = s;
  }

public:
  Bool install(void);

  //
  // Suspension list
  //
private:
  SuspList  *suspList;

public:
  void addSuspension(Suspendable *);
  SuspList * getSuspList(void) {
    return suspList;
  }
  void setSuspList(SuspList *sl) {
    suspList=sl;
  }
  Bool isEmptySuspList() {
    return suspList==0;
  }
  void clearSuspList(Suspendable *);

  //
  // Propagation queue
  //

private:
  SuspQueue lpq;

  static Board * board_served;

  void wakeServeLPQ(void);
  void killServeLPQ(void);

public:
  void addToLPQ(Propagator * p) {
    Assert(!isCommitted() && p->getBoardInternal()->derefBoard() == this);
    if (lpq.isEmpty())
      wakeServeLPQ();
    lpq.enqueue(p);
  }
  OZ_Return scheduleLPQ(void);


  //
  // nonmonotonic propagators
  //

private:
  OrderedSuspList * nonMonoSuspList;

public:
  void setNonMono(OrderedSuspList * l) {
    nonMonoSuspList = l;
  }
  OrderedSuspList *getNonMono() {
    return nonMonoSuspList;
  }
  void addToNonMono(Propagator *);
  void scheduleNonMono(void);

  //
  // distributors
  //
private:
  Distributor * dist;

public:
  Distributor * getDistributor(void) {
    return dist;
  }
  void setDistributor(Distributor * d) {
    dist = d;
  }


  //
  // Operations
  //
  Board * clone(void);
  void fail(void);
  int commit(int, int);
  void inject(TaggedRef, int arity = 1);
  OZ_Return merge(Board *);

  //
  // Status variable
  //
public:
  TaggedRef getStatus() {
    return status;
  }
  void setStatus(TaggedRef v) {
    status = v;
  }
  void bindStatus(TaggedRef t);

  void clearStatus();
  void patchAltStatus(int i);

  TaggedRef genSucceeded(Bool);
  TaggedRef genAlt(int);
  TaggedRef genFailed();
  TaggedRef genBlocked(TaggedRef arg);


  //
  // Root variable
  //
public:
  TaggedRef getRootVar() {
    return makeTaggedRef(&rootVar);
  }

  //
  // Installation control
  //
private:
  static Bool _ignoreWakeUp;

public:
  static Bool mustIgnoreWakeUp(void) {
    return Board::_ignoreWakeUp;
  }
  static void ignoreWakeUp(Bool iwu) {
    Board::_ignoreWakeUp = iwu;
  }

  //
  // Checks
  //
  void checkSituatedness(TaggedRef*,TaggedRef*,TaggedRef*);

  //
  // Misc
  //

  OZPRINTLONG;

#ifdef DEBUG_CHECK
  void printTree();
#endif

  //
  // Trailing comparison
  //

#ifdef CS_PROFILE
public:
  int32 * orig_start;
  int32 * copy_start;
  int     copy_size;

  TaggedRef getCloneDiff(void);
#endif


};


#ifndef OUTLINE
#include "board.icc"
#endif

#endif
