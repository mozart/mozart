/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl

  ------------------------------------------------------------------------
*/

#ifndef __BOARDH
#define __BOARDH

#ifdef INTERFACE
#pragma interface
#endif

#include "actor.hh"

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
  OZPRINTLONG;
};

class Script {
public:
  void gc();

  Script() { numbOfCons = 0; first = (Equation *)NULL; }
  Script(int sizeInit);
  ~Script();
  OZPRINT;
  OZPRINTLONG;
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
  Bo_PathMark   = 0x0080,
  Bo_Failed     = 0x0100,
  Bo_Committed  = 0x0200,
  Bo_Waiting    = 0x0800
};


class Board {
friend void engine(Bool init);
private:
  int flags;
  int suspCount;
  Continuation body;
  union {
    Actor *actor;
    Board *ref;
  } u;
  Script script;
public:
  Board(); // fake compiler
  ~Board(); // fake compiler
  Board(Board&); // fake compiler
  Board(Actor *a,int type);

  USEHEAPMEMORY;
  Board *gcBoard();
  void gcRecurse(void);
  Bool gcIsAlive();
  Bool checkAlive();
  Board *gcGetNotificationBoard ();
  int32 *getGCField() { return body.getGCField(); }

  OZPRINT;
  OZPRINTLONG;

  void printTree();
  Board * getHighestSolveDebug(void); // TMUELLER

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
    for (bb=this; bb->isCommitted(); bb=bb->u.ref)
      ;
    return bb;
  }

  Board *getParent() {
    Assert(!isCommitted());
    return GETBOARD(u.actor);
  }

  Board *getParentAndTest() {
    Assert(!isCommitted());
    if (isFailed() || isRoot() || u.actor->isCommitted()) return 0;
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
  Bool isAsk() { return flags & Bo_Ask ? OK : NO; }
  Bool isCommitted() { return flags & Bo_Committed ? OK : NO; }
  Bool isFailed() { return flags & Bo_Failed ? OK : NO; }
  Bool isInstalled() { return flags & Bo_Installed ? OK : NO; }
  Bool isNervous() { return flags & Bo_Nervous ? OK : NO; }
  Bool isPathMark() { return flags & Bo_PathMark ? OK : NO; }
  Bool isWaitTop() { return flags & Bo_WaitTop ? OK : NO; }
  Bool isWait() { return flags & Bo_Wait ? OK : NO; }
  Bool isWaiting() { return flags & Bo_Waiting ? OK : NO; }
  Bool isRoot() { return flags & Bo_Root ? OK : NO; }
  Bool isSolve () { return ((flags & Bo_Solve) ? OK : NO); }

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

  void setInstalled() { flags |= Bo_Installed; }
  void setNervous() { flags |= Bo_Nervous; }
  void setFailed() { flags |= Bo_Failed; }
  void setPathMark() { flags |= Bo_PathMark; }

  void setScript(int i,TaggedRef *v,TaggedRef r) {
    script[i].setLeft(v);
    script[i].setRight(r);
  }

  void setCommitted(Board *s) {
    Assert(!isInstalled() && !isCommitted());
    flags |= Bo_Committed;
    u.actor->setCommitted();
    u.ref = s;
  }

  void setWaitTop() { flags |= Bo_WaitTop; }
  void setWaiting() { flags |= Bo_Waiting; }
  void setActor (Actor *aa) { u.actor = aa; }   // needed for the solve combinator;
  void unsetInstalled() { flags &= ~Bo_Installed; }
  void unsetNervous() { flags &= ~Bo_Nervous; }
  void unsetPathMark() { flags &= ~Bo_PathMark; }
};

#endif
