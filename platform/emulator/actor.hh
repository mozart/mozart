/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  implementation of conditional and disjunctive actors
  ------------------------------------------------------------------------
*/

#ifndef __ACTORH
#define __ACTORH

#include "types.hh"
#include "suspension.hh"

// ------------------------------------------------------------------------

class Actor : public ConstTerm {
protected:
  int flags;
  Continuation next;
  Board *board;
  int childCount;
  int priority;
public:
  Actor(int type,Board *s,int prio,
	       ProgramCounter p,RefsArray y,RefsArray g,
	       RefsArray x,int i);
  ~Actor();

  USEHEAPMEMORY;
  Actor *gc();
  void gcRecurse(void);
  OZPRINT;
  OZPRINTLONG;

  void addChild(Board *n);
  void failChild(Board *n);
  Continuation *getNext();
  int getPriority();
  Board *getBoard();
  Bool hasNext();
  Bool isCommitted();
  Bool isAsk();
  Bool isLeaf();
  Bool isWait();
  Bool isDisWait();
  void lastClause();
  void nextClause(ProgramCounter pc);
  void setCommitted();
  void setDisWait();
};

// ------------------------------------------------------------------------

class AskActor : public Actor {
private:
  ProgramCounter elsePC;
public:
  AskActor(Board *s,int prio,
	   ProgramCounter elsepc,
	   ProgramCounter p, RefsArray y,RefsArray g, RefsArray x, int i);
  ~AskActor();

  ProgramCounter getElsePC();
};

AskActor *CastAskActor(Actor *a);

// ------------------------------------------------------------------------

class WaitActor : public Actor {
private:
  Board **childs;
public:
  WaitActor(Board *s,int prio,
	    ProgramCounter p,RefsArray y,RefsArray g,RefsArray x,int i);
  ~WaitActor();

  void gcRecurse();

  void addChildInternal(Board *n);
  void failChildInternal(Board *n);
  Board *getChild();
  Bool hasOneChild();
};

WaitActor *CastWaitActor(Actor *a);

#ifndef OUTLINE
#include "actor.icc"
#endif

#endif
