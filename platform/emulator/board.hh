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

class Board : public ConstTerm {
friend void engine();
private:
  static Board *Root;
  static Board *Current;
public:
  static void Init();
  static void Print();
  static Board *GetCurrent();
  static Board *GetRoot();
  static void SetCurrent(Board *c, Bool checkNotGC=OK);
  static void NewCurrentAsk(Actor *a);
  static void NewCurrentWait(Actor *a);
  static Actor *FailCurrent();
  static void GC();

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

  void addSuspension();
  Actor *getActor();
  Continuation *getBodyPtr();
  Board *getParentBoard();
  ConsList &getScriptRef();
  Board *getBoard();
  Board *getBoardDeref();
  Board *gcGetBoardDeref();
  int getSuspCount(void);
  Bool hasSuspension();
  Bool isAsk();
  Bool isCommitted();
  Bool isDiscarded();
  Bool isFailed();
  Bool isInstalled();
  Bool isNervous();
  Bool isPathMark();
  Bool isWaitTop();
  Bool isWait();
  Bool isWaiting();
  Bool isRoot();
  void newScript(int size);
  void removeSuspension();
  void setBody(ProgramCounter p,RefsArray y,
                       RefsArray g,RefsArray x,int i);
  void setInstalled();
  void setNervous();
  void setPathMark();
  void setScript(int i,TaggedRef *v,TaggedRef r);
  void setCommitted(Board *s);
  void setWaitTop();
  void setWaiting();
  void unsetInstalled();
  void unsetNervous();
  void unsetPathMark();
};

#ifndef OUTLINE
#include "board.icc"
#endif

#endif
