/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __BOARDH
#define __BOARDH

#ifdef __GNUC__
#pragma interface
#endif

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

  inline int getSize() { return numbOfCons; }
  inline Equation* getRef()  { return (first); }

  inline Equation &operator[] (int elem)  { return ( *(first + elem) ); }
  /* no bounds checking;    */

private:
  int numbOfCons;
  Equation* first;
};




enum BoardFlags {
  Bo_Ask	= 0x0001,
  Bo_Wait	= 0x0002,
  Bo_Solve      = 0x0004,
  Bo_Root	= 0x0008,
  Bo_Installed	= 0x0010,
  Bo_Nervous	= 0x0020,
  Bo_WaitTop	= 0x0040,
  Bo_PathMark	= 0x0080,
  Bo_Failed	= 0x0100,
  Bo_Committed	= 0x0200,
  Bo_Waiting    = 0x0800,
  Bo_Reflected  = 0x1000,	// for debugging of solve combinator;
  Bo_NervousSolve= 0x2000
};


class Board : public ConstTerm {
friend void engine();
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
  Board(Actor *a,int type);
  ~Board();

  USEHEAPMEMORY;
  Board *gcBoard();
  void gcRecurse(void);
  Bool gcIsAlive();
  Board *gcGetNotificationBoard ();

  OZPRINT;
  OZPRINTLONG;

  void printTree();

  void setCompModeHackForBoardToInstall(int compMode) {
    flags |= compMode<<16;
  }
  int getCompModeHackForBoardToInstall() {
    return flags>>16;
  }
  inline void incSuspCount(int n=1);
  void decSuspCount();
  inline Board *getBoardFast();
  Board *getParentAndTest();
  Board *getParentFast();

  Actor *getActor();
//  Board *getRef() { return u.ref; }
  Continuation *getBodyPtr() { return &body; }
  Board* getSolveBoard (); 
  Bool underReflected();
  Script &getScriptRef() { return script; }
  int getSuspCount(void);
  Bool hasSuspension(void);
  Bool isAsk() { return flags & Bo_Ask ? OK : NO; }
  Bool isCommitted() { return flags & Bo_Committed ? OK : NO; }
  Bool isFailed() { return flags & Bo_Failed ? OK : NO; }
  Bool isInstalled() { return flags & Bo_Installed ? OK : NO; }
  Bool isNervous() { return flags & Bo_Nervous ? OK : NO; }
  Bool isNervousSolve() { return flags & Bo_NervousSolve ? OK : NO; }
  Bool isPathMark() { return flags & Bo_PathMark ? OK : NO; }
  Bool isWaitTop() { return flags & Bo_WaitTop ? OK : NO; }
  Bool isWait() { return flags & Bo_Wait ? OK : NO; }
  Bool isWaiting() { return flags & Bo_Waiting ? OK : NO; }
  Bool isRoot() { return flags & Bo_Root ? OK : NO; }
  Bool isSolve () { return ((flags & Bo_Solve) ? OK : NO); }
  void setReflected () { flags |= Bo_Reflected; }
  Bool isReflected () { return ((flags & Bo_Reflected) ? OK : NO); }
  void newScript(int size);
  void setBody(ProgramCounter p,RefsArray y,
		       RefsArray g,RefsArray x,int i);
  void setInstalled() { flags |= Bo_Installed; }
  void setNervous() { flags |= Bo_Nervous; }
  void setFailed() { flags |= Bo_Failed; }
  void setNervousSolve() { flags |= Bo_NervousSolve; }
  void setPathMark() { flags |= Bo_PathMark; }

  void setScript(int i,TaggedRef *v,TaggedRef r);
  void setCommitted(Board *s);
  void setWaitTop() { flags |= Bo_WaitTop; }
  void setWaiting() { flags |= Bo_Waiting; }
  void setActor (Actor *aa) { u.actor = aa; }   // needed for the solve combinator; 
  void unsetInstalled() { flags &= ~Bo_Installed; }
  void unsetNervous() { flags &= ~Bo_Nervous; }
  void unsetNervousSolve() { flags &= ~Bo_NervousSolve; }
  void unsetPathMark() { flags &= ~Bo_PathMark; }
  // special for solve combinator: checks whether 'false' stays in body;
  Bool isFailureInBody ();
};

#ifndef OUTLINE
#include "actor.hh"
#include "board.icc"
#endif

#endif

