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

#include "actor.hh"

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
  Bo_Solve      = 0x0004,
  Bo_Root       = 0x0008,
  Bo_Installed  = 0x0010,
  Bo_GlobalMark = 0x0080,
  Bo_Failed     = 0x0100,
  Bo_Committed  = 0x0200,
};


#ifdef DEBUG_THREADCOUNT
extern int existingLTQs;
#endif

class Board {
friend int engine(Bool init);
private:
  int flags;
  int suspCount;
  union {
    Actor *actor;
    Board *ref;
  } u;
  Script script;
  Board * gcField; // Will go away, when new propagator model is active
#ifdef PROP_MERGING
  PropagatorQueue * pq;
#endif

public:
  NO_DEFAULT_CONSTRUCTORS(Board)
  Board(Actor *a,int type);

  USEHEAPMEMORY;

  Bool isCommitted()    { return flags & Bo_Committed;  }
  Bool isFailed()       { return flags & Bo_Failed;     }
  Bool isInstalled()    { return flags & Bo_Installed;  }
  Bool isMarkedGlobal() { return flags & Bo_GlobalMark; }
  Bool _isRoot()        { return flags & Bo_Root;       }
  Bool isSolve ()       { return flags & Bo_Solve;      }

  void setInstalled()  { flags |= Bo_Installed; }
  void setFailed()     { flags |= Bo_Failed; }
  void setGlobalMark() { flags |= Bo_GlobalMark; }

  void unsetInstalled()  { flags &= ~Bo_Installed;  }
  void unsetGlobalMark() { flags &= ~Bo_GlobalMark; }

#ifdef PROP_MERGING
  PropagatorQueue * getPropQueue() {
    return pq;
  }
#endif

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

  Board *derefBoard() {
    Board *bb;
    for (bb=this; bb->isCommitted(); bb=bb->u.ref) {}
    return bb;
  }

  Board *getParent() {
    Assert(!isCommitted());
    return GETBOARD(u.actor);
  }

  Board *getParentAndTest() {
    Assert(!isCommitted());
    if (isFailed() || _isRoot() || u.actor->isCommitted()) return 0;
    return getParent();
  }

  Actor *getActor() {
    Assert(!isCommitted());
    return u.actor;
  }

  Board* getSolveBoard ();
  Script &getScriptRef() { return script; }
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

  void newScript(int size) {
    script.allocate(size);
  }

  void setCommittedBoard(Board *s) {
    Assert(!isInstalled() && !isCommitted());
    flags |= Bo_Committed;
    u.ref = s;
  }

  void setScript(int i,TaggedRef *v,TaggedRef r) {
    script[i].setLeft(v);
    script[i].setRight(r);
  }

  Bool isInTree(void);
  void unsetGlobalMarks(void);
  void setGlobalMarks(void);

//-----------------------------------------------------------------------------
// local thread queue
private:
  LocalPropagatorQueue * localPropagatorQueue;
public:
  void initLPQ(LocalPropagatorQueue *lpq) {
    Assert(!localPropagatorQueue);
    localPropagatorQueue=lpq;
  }
  void resetLocalPropagatorQueue(void);

  LocalPropagatorQueue * getLocalPropagatorQueue(void) {
    return localPropagatorQueue;
  }
  void setLocalPropagatorQueue(LocalPropagatorQueue * lpq) {
    localPropagatorQueue = lpq;
  }

};

#endif
