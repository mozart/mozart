/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    Christian Schulte (schulte@dfki.de)
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

#ifndef __BOARDH
#define __BOARDH

#ifdef INTERFACE
#pragma interface
#endif

#include "actor.hh"

#ifdef PROP_MERGING
#include "thrqueue.hh"
#endif


#define GETBOARD(v) ((v)->getBoardInternal()->derefBoard())


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
  Bo_Ask        = 0x0001,
  Bo_Wait       = 0x0002,
  Bo_Solve      = 0x0004,
  Bo_Root       = 0x0008,
  Bo_Installed  = 0x0010,
  Bo_Nervous    = 0x0020,
  Bo_WaitTop    = 0x0040,
  Bo_GlobalMark = 0x0080,
  Bo_Failed     = 0x0100,
  Bo_Committed  = 0x0200,
  Bo_Waiting    = 0x0800
};


class Board {
friend int engine(Bool init);
private:
  int flags;
  int suspCount;
  Continuation body;
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
  NO_DEFAULT_CONSTRUCTORS(Board);
  Board(Actor *a,int type);

  USEHEAPMEMORY;

  Bool isAsk()          { return flags & Bo_Ask;        }
  Bool isCommitted()    { return flags & Bo_Committed;  }
  Bool isFailed()       { return flags & Bo_Failed;     }
  Bool isInstalled()    { return flags & Bo_Installed;  }
  Bool isNervous()      { return flags & Bo_Nervous;    }
  Bool isMarkedGlobal() { return flags & Bo_GlobalMark; }
  Bool isWaitTop()      { return flags & Bo_WaitTop;    }
  Bool isWait()         { return flags & Bo_Wait;       }
  Bool isWaiting()      { return flags & Bo_Waiting;    }
  Bool _isRoot()        { return flags & Bo_Root;       }
  Bool isSolve ()       { return flags & Bo_Solve;      }

  void setInstalled()  { flags |= Bo_Installed; }
  void setNervous()    { flags |= Bo_Nervous; }
  void setFailed()     { flags |= Bo_Failed; }
  void setGlobalMark() { flags |= Bo_GlobalMark; }
  void setWaitTop()    { flags |= Bo_WaitTop; }
  void setWaiting()    { flags |= Bo_Waiting; }

  void unsetInstalled()  { flags &= ~Bo_Installed;  }
  void unsetNervous()    { flags &= ~Bo_Nervous;    }
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

//  Board *getRef() { return u.ref; }
  Continuation *getBodyPtr() { return &body; }
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

  void setBody(ProgramCounter p,RefsArray y,
               RefsArray g,RefsArray x,int i){
    body.setPC(p);
    body.setY(y);
    body.setG(g);
    body.setX(x,i);
  }

  void setCommitted(Board *s) {
    Assert(!isInstalled() && !isCommitted());
#ifdef PROP_MERGING
    pq->merge(s->getPropQueue());
#endif
    flags |= Bo_Committed;
    u.actor->setCommitted();
    u.ref = s;
  }
  void setActor (Actor *aa) {
    u.actor = aa;
  }
  void setScript(int i,TaggedRef *v,TaggedRef r) {
    script[i].setLeft(v);
    script[i].setRight(r);
  }

  Bool isInTree(void);
  void unsetGlobalMarks(void);
  void setGlobalMarks(void);
};

#endif
