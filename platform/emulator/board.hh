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


#include "constter.hh"

#include "constr.hh"
#include "suspensi.hh"

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
  Bo_Discarded  = 0x0400,
  Bo_Waiting    = 0x0800,
  Bo_Reflected  = 0x1000        // for debugging of solve combinator;
};

class Board : public ConstTerm {
friend void engine();
public:
  static void Print();
private:
  int flags;
  int suspCount;
  Continuation body;
  union {
    Actor *actor;
    Board *board;
  } u;
  ConsList script;
public:
  Board(Actor *a,int type);
  ~Board();

  USEHEAPMEMORY;
  Board *gcBoard1();
  Board *gcBoard();
  void gcRecurse(void);
  OZPRINT;
  OZPRINTLONG;

  void printTree();

  void incSuspCount(int n=1);
  void addSuspension (Suspension *susp);
  // should be applied only for 'solve' boards;
  Board *gcGetBoardDeref();
  Board *gcGetNotificationBoard ();
  Actor *getActor();
  Board *getBoard() { return u.board; }
  Board *getBoardDeref();
  Continuation *getBodyPtr() { return &body; }
  Board *getParentBoard();
  Board* getSolveBoard ();
  ConsList &getScriptRef() { return script; }
  int getSuspCount(void);
  Bool hasSuspension(void);
  Bool isAsk() { return flags & Bo_Ask ? OK : NO; }
  Bool isCommitted() { return flags & Bo_Committed ? OK : NO; }
  Bool isDiscarded();
  Bool isFailed() { return flags & Bo_Failed ? OK : NO; }
  Bool isInstalled() { return flags & Bo_Installed ? OK : NO; }
  Bool isNervous() { return flags & Bo_Nervous ? OK : NO; }
  Bool isPathMark() { return flags & Bo_PathMark ? OK : NO; }
  Bool isWaitTop() { return flags & Bo_WaitTop ? OK : NO; }
  Bool isWait() { return flags & Bo_Wait ? OK : NO; }
  Bool isWaiting() { return flags & Bo_Waiting ? OK : NO; }
  Bool isRoot() { return flags & Bo_Root ? OK : NO; }
  Bool isSolve () { return ((flags & Bo_Solve) ? OK : NO); }
  void setReflected () { flags |= Bo_Reflected; }
  Bool isReflected () { return ((flags & Bo_Reflected) ? OK : NO); }
  void newScript(int size);
  void removeSuspension();
  void setBody(ProgramCounter p,RefsArray y,
                       RefsArray g,RefsArray x,int i);
  void setInstalled() { flags |= Bo_Installed; }
  void setNervous() { flags |= Bo_Nervous; }
  void setPathMark() { flags |= Bo_PathMark; }

  void setScript(int i,TaggedRef *v,TaggedRef r);
  void setCommitted(Board *s);
  void setWaitTop() { flags |= Bo_WaitTop; }
  void setWaiting() { flags |= Bo_Waiting; }
  void setActor (Actor *aa) { u.actor = aa; }   // needed for the solve combinator;
  void unsetInstalled() { flags &= ~Bo_Installed; }
  void unsetNervous() { flags &= ~Bo_Nervous; }
  void unsetPathMark() { flags &= ~Bo_PathMark; }
  // special for solve combinator: checks whether 'false' stays in body;
  Bool isFailureInBody ();
};

#ifndef OUTLINE
#include "board.icc"
#endif

#endif
