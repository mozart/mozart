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


#include "types.hh"

#include "constr.hh"
#include "suspension.hh"

enum BoardFlags {
  Bo_Ask	= 1<<0,
  Bo_Wait	= 1<<1,
  Bo_Solve      = 1<<2,
  Bo_Root	= 1<<3,
  Bo_Installed	= 1<<4,
  Bo_Nervous	= 1<<5,
  Bo_WaitTop	= 1<<6,
  Bo_PathMark	= 1<<7,
  Bo_Failed	= 1<<8,
  Bo_Committed	= 1<<9,
  Bo_Discarded	= 1<<10,
  Bo_Waiting    = 1<<11
};

class Board : public ConstTerm {
friend void engine();
public:
  static void Init();
  static Board* GetSolveBoard (); 
  static void SetCurrent(Board *c, Bool checkNotGC=OK);
  static void NewCurrentAsk(Actor *a);
  static void NewCurrentWait(Actor *a);
  static void NewCurrentSolve (Actor *a);
  static Board *NewRoot();
  static Actor *FailCurrent();
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
  Board(Actor *a,int type);
public:
  ~Board();

  USEHEAPMEMORY;
  Board *gcBoard1();
  Board *gcBoard();
  void gcRecurse(void);
  OZPRINT;
  OZPRINTLONG;
  MPRINT
  void printTree();

  void addSuspension(int n=1) { suspCount += n; }
  Board *gcGetBoardDeref();
  Actor *getActor();
  Board *getBoard() { return u.board; }
  Board *getBoardDeref();
  Continuation *getBodyPtr() { return &body; }
  Board *getParentBoard();
  ConsList &getScriptRef() { return script; }
  int getSuspCount() { return suspCount; }
  Bool hasSuspension() { return suspCount == 0 ? NO : OK; }
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
  void newScript(int size);
  inline void removeSuspension();
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
};

#ifndef OUTLINE
#include "board.icc"
#endif

#endif

