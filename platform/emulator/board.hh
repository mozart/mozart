/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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

#ifndef __BOARDH
#define __BOARDH

#ifdef INTERFACE
#pragma interface
#endif

#include "distributor.hh"
#include "thr_class.hh"

#define GETBOARD(v) ((v)->getBoardInternal()->derefBoard())
#define GETBOARDOBJ(v) ((v).getBoardInternal()->derefBoard())

struct Equation {
friend class Script;
private:
  TaggedRef left;
  TaggedRef right;
public:
  void setLeft(TaggedRef *l) { left = makeTaggedRef(l); }
  void setRight(TaggedRef r) { right = r; }
  TaggedRef getLeft() { return left; }
  TaggedRef getRight() { return right; }
  OZPRINT;
};

class Script {
public:
  void gc();

  Script() { numbOfCons = 0; first = (Equation *)NULL; }
  Script(int sizeInit);
  ~Script();
  OZPRINT;
  void allocate(int sizeInit);
  void dealloc();

  int getSize() { return numbOfCons; }
  Equation* getRef()  { return (first); }

  Equation &operator[] (int elem)  { return ( *(first + elem) ); }
  /* no bounds checking;    */

private:
  int numbOfCons;
  Equation* first;
};




enum BoardFlags {
  Bo_Root       = 0x0001,
  Bo_Installed  = 0x0002,
  Bo_GlobalMark = 0x0004,
  Bo_Failed     = 0x0008,
  Bo_Committed  = 0x0010,
  Bo_Clone      = 0x0020,
};


#ifdef DEBUG_THREADCOUNT
extern int existingLTQs;
#endif

class Board {
friend int engine(Bool init);
private:
  int flags;
  int suspCount;
  Board * parent;
  Script script;
  Board * gcField; // Will go away, when new propagator model is active
  TaggedRef solveVar;
  TaggedRef result;
  SuspList  *suspList;
  int threads;

public:
  NO_DEFAULT_CONSTRUCTORS(Board);
  Board(Board *b);

  USEHEAPMEMORY;

  Bool isCommitted()    { return flags & Bo_Committed;  }
  Bool isFailed()       { return flags & Bo_Failed;     }
  Bool isInstalled()    { return flags & Bo_Installed;  }
  Bool isMarkedGlobal() { return flags & Bo_GlobalMark; }
  Bool isRoot()         { return flags & Bo_Root;       }
  Bool isCloneBoard()   { return flags & Bo_Clone;      }

  void setRoot()       { flags |= Bo_Root;       }
  void setInstalled()  { flags |= Bo_Installed;  }
  void setFailed()     { flags |= Bo_Failed;     }
  void setGlobalMark() { flags |= Bo_GlobalMark; }
  void setCloneBoard() { flags |= Bo_Clone;      }

  void unsetInstalled()  { flags &= ~Bo_Installed;  }
  void unsetGlobalMark() { flags &= ~Bo_GlobalMark; }
  void unsetCloneBoard() { flags &= ~Bo_Clone;      }

  Bool gcIsMarked(void);
  void gcMark(Board *);
  Board * gcGetFwd(void);
  Board *gcBoard();
  void gcRecurse(void);
  Bool gcIsAlive();
  Bool checkAlive();
  Board *gcGetNotificationBoard ();

  OZPRINTLONG;

  void printTree();

  //
  // Suspension counter
  //

  void incSuspCount(int n=1) {
    Assert(!isCommitted() && !isFailed());
    suspCount += n;
    Assert(suspCount >= 0);
  }
  void decSuspCount() {
    Assert(!isCommitted() && !isFailed());
    Assert(suspCount > 0);
    suspCount--;
  }

  int getSuspCount(void) {
    Assert(!isFailed());
    Assert(suspCount >= 0);
    return suspCount;
  }

  Bool hasSuspension(void) {
    Assert(!isFailed());
    Assert(suspCount >= 0);
    return suspCount != 0;
  }


  //
  // Thread counter
  //

  void incThreads(int n = 1) {
    threads += n;
  }
  int decThreads()  {
    Assert (threads > 0);
    return (--threads);
  }

  int getThreads(void) {
    return threads;
  }

  //
  // Home and parent access
  //

  void setCommittedBoard(Board *s) {
    Assert(!isInstalled() && !isCommitted());
    flags |= Bo_Committed;
    parent = s;
  }

  Board *derefBoard() {
    Board *bb;
    for (bb=this; bb->isCommitted(); bb=bb->parent) {}
    return bb;
  }

  Board *getParent() {
    Assert(!isCommitted());
    return parent->derefBoard();
  }

  Board *getParentAndTest() {
    Assert(!isCommitted());
    if (isFailed() || isRoot())
      return 0;
    return getParent();
  }

  //
  // Script
  //

  Script &getScriptRef() { return script; }
  void newScript(int size) {
    script.allocate(size);
  }

  void setScript(int i,TaggedRef *v,TaggedRef r) {
    script[i].setLeft(v);
    script[i].setRight(r);
  }

  //
  // Suspension list
  //
  void addSuspension(Suspension);
  Bool isEmptySuspList() { return suspList==0; }
  void setSuspList(SuspList *sl) { suspList=sl; }
  SuspList *unlinkSuspList() {
    SuspList *sl = suspList;
    suspList=0;
    return sl;
  }

  //
  // Copying marks
  //

  Bool isInTree(void);
  void unsetGlobalMarks(void);
  void setGlobalMarks(void);

  //
  // local thread queue
  //

private:
  LocalPropagatorQueue * localPropagatorQueue;

public:
  LocalPropagatorQueue * getLocalPropagatorQueue(void) {
    return localPropagatorQueue;
  }
  void setLocalPropagatorQueue(LocalPropagatorQueue * lpq) {
    localPropagatorQueue = lpq;
  }

  void resetLocalPropagatorQueue(void);

  //
  // nonmonotonic propagators
  //

private:
  OrderedSuspList * nonMonoSuspList;

public:
  void setNonMonoSuspList(OrderedSuspList * l) {
    nonMonoSuspList = l;
  }
  OrderedSuspList *getNonMonoSuspList() {
    return nonMonoSuspList;
  }

  void addToNonMonoSuspList(Propagator *);
  void mergeNonMonoSuspListWith(OrderedSuspList *);

  void mergeNonMono(Board *bb);

  //
  // distributors
  //
private:
  DistBag   *bag;

public:
  void addDistributor(Distributor * d) {
    bag = bag->add(d);
  }
  void mergeDistributors(DistBag * db) {
    bag = bag->merge(db);
  }
  DistBag * getBag() {
    return bag;
  }

  void cleanDistributors(void);
  Distributor * getDistributor(void);

  int commit(int left, int right);


  //
  // Status variable
  //

  void clearResult();
  void patchChoiceResult(int i);
  TaggedRef getResult() { return result; }
  void setResult(TaggedRef v) { result = v; }

  TaggedRef genSolved();
  TaggedRef genStuck();
  TaggedRef genChoice(int noOfClauses);
  TaggedRef genFailed();
  TaggedRef genUnstable(TaggedRef arg);


  //
  // Root variable
  //

  TaggedRef getSolveVar() {
    return makeTaggedRef(&solveVar);
  }


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

void oz_solve_scheduleNonMonoSuspList(Board *);

#ifndef OUTLINE
#include "board.icc"
#endif

#endif
