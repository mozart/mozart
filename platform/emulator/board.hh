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
  Bo_Root       = 0x0001, // is root
  Bo_Installed  = 0x0002, // is installed
  Bo_GlobalMark = 0x0004, // is marked as global for cloning
  Bo_Failed     = 0x0008, // is failed
  Bo_Committed  = 0x0010, // is committed (merged)
  Bo_Clone      = 0x0020, // is the root for cloning
};


class Board {
friend int engine(Bool init);
public:
  NO_DEFAULT_CONSTRUCTORS(Board);
  Board(Board *b);

  USEHEAPMEMORY;

  //
  // Flags
  //
private:
  int flags;

  void setCommitted() { flags |= Bo_Committed;  }
  void setCloneBoard() { flags |= Bo_Clone;      }
  void setGlobalMark() { flags |= Bo_GlobalMark; }

  void unsetGlobalMark() { flags &= ~Bo_GlobalMark; }
  void unsetCloneBoard() { flags &= ~Bo_Clone;      }

public:
  Bool isCommitted()    { return flags & Bo_Committed;  }
  Bool isFailed()       { return flags & Bo_Failed;     }
  Bool isInstalled()    { return flags & Bo_Installed;  }
  Bool isMarkedGlobal() { return flags & Bo_GlobalMark; }
  Bool isRoot()         { return flags & Bo_Root;       }
  Bool isCloneBoard()   { return flags & Bo_Clone;      }

  void setRoot()       { flags |= Bo_Root;       }
  void setInstalled()  { flags |= Bo_Installed;  }
  void setFailed()     { flags |= Bo_Failed;     }

  void unsetInstalled()  { flags &= ~Bo_Installed;  }


  //
  // Garbage collection and copying
  //
public:
  Bool gcIsMarked(void);
  void gcMark(Board *);
  Board * gcGetFwd(void);
  Board *gcBoard();
  void gcRecurse(void);
  Bool gcIsAlive();
  Board *gcGetNotificationBoard ();

  void unsetGlobalMarks(void);
  void setGlobalMarks(void);
  Board * clone(void);

  //
  // Suspension counter
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
  void incSolveThreads(void);
  void decSolveThreads(void);
  void checkSolveThreads(void);

  //
  // Thread counter
  //
private:
  int threads;

public:
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
private:
  Board * parent;

public:
  Board *derefBoard() {
    Board *bb;
    for (bb=this; bb->isCommitted(); bb=bb->parent) {}
    return bb;
  }
  Board *getParent() {
    Assert(!isCommitted());
    return parent->derefBoard();
  }

  //
  // Script
  //
private:
  Script script;

public:
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
private:
  SuspList  *suspList;

public:
  void addSuspension(Suspension);
  SuspList * getSuspList(void) {
    return suspList;
  }
  void setSuspList(SuspList *sl) {
    suspList=sl;
  }
  Bool isEmptySuspList() {
    return suspList==0;
  }
  void clearSuspList(Suspension);

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
  DistBag   *bag;

public:
  DistBag * getDistBag(void) {
    return bag;
  }
  void setDistBag(DistBag * db) {
    bag = db;
  }
  void addToDistBag(Distributor * d);
  void cleanDistributors(void);
  Distributor * getDistributor(void);

  //
  // Operations
  //
  int commit(int, int);
  void inject(TaggedRef);
  TaggedRef merge(Board *, Bool);

  //
  // Status variable
  //
private:
  TaggedRef result;

public:
  TaggedRef getResult() {
    return result;
  }
  void setResult(TaggedRef v) {
    result = v;
  }

  void clearResult();
  void patchChoiceResult(int i);

  TaggedRef genSolved();
  TaggedRef genStuck();
  TaggedRef genChoice(int noOfClauses);
  TaggedRef genFailed();
  TaggedRef genUnstable(TaggedRef arg);


  //
  // Root variable
  //
private:
  TaggedRef rootVar;

public:
  TaggedRef getRootVar() {
    return makeTaggedRef(&rootVar);
  }


  //
  // Misc
  //

  OZPRINTLONG;

#ifdef DEBUG_CHECK
  void printTree();

  Bool isInTree(void);

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

void oz_checkExtSuspension(Suspension, Board *);

#define CheckExtSuspension(susp)               \
  if (((Suspension)susp).wasExtSuspension()) { \
    GETBOARDOBJ((Suspension) susp)->checkSolveThreads();   \
  }

#ifndef OUTLINE
#include "board.icc"
#endif

#endif
