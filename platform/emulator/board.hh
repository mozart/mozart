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

#include "types.hh"

#include "constr.hh"
#include "suspension.hh"

class Board : public ConstTerm {
public:
  static Board *Root;
  static Board *Current;
  static void SetCurrent(Board *c, Bool checkNotGC=OK);

private:
  int flags;
  int suspCount;
  Continuation body;
  union {
    Actor *actor;
    Board *board;
  };
  ConsList script;
public:
  Board(Actor *a,int type);
  ~Board();

  USEHEAPMEMORY;
  void gc();
  Bool isPathMark();
  Bool setPathMark();
  Bool unsetPathMark();
  Board *getParentDebug();
  OZPRINT;
  OZPRINTLONG;

  void addSuspension();
  Actor *getActor();
  Continuation *getBodyPtr();
  Board *getParentBoardDeref();
  ConsList &getScriptRef();
  Board *getBoardDeref();
  int getSuspCount(void);
  Bool hasSuspension();
  Bool isAsk();
  Bool isCommitted();
  Bool isDead();
  Bool isInstalled();
  Bool isNervous();
  Bool isWaitTop();
  Bool isWait();
  Bool isRoot();
  void newScript(int size);
  void removeSuspension();
  void setBody(ProgramCounter p,RefsArray y,
		       RefsArray g,RefsArray x,int i);
  void setInstalled();
  void setNervous();
  void setScript(int i,TaggedRef *v,TaggedRef r);
  void setBoard(Board *s);
  void setWaitTop();
  void unsetInstalled();
  void unsetNervous();
};


class AskBoard: public Board {
public:
  AskBoard(Actor *a);
};

class WaitBoard: public Board {
public:
  WaitBoard(Actor *a);
};

class RootBoard: public Board {
public:
  RootBoard();
};

#ifndef OUTLINE
#include board.icc
#endif

#endif

