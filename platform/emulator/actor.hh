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

#ifdef __GNUC__
#pragma interface
#endif

#include "types.hh"
#include "suspension.hh"

// ------------------------------------------------------------------------

enum ActorFlags {
  Ac_None       = 0,
  Ac_Ask        = 1<<0,
  Ac_Wait       = 1<<1,
  Ac_Committed  = 1<<2,
  Ac_WaitTop    = 1<<3,
  Ac_DisWait    = 1<<4,
};

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
  Continuation *getNext() { return &next;}
  int getPriority() { return priority; }
  Board *getBoard() { return board; }
  Bool hasNext() { return next.getPC() == NOCODE ? NO : OK; }
  Bool isCommitted() { return flags & Ac_Committed ? OK : NO; }
  Bool isAsk() { return flags & Ac_Ask ? OK : NO; }
  Bool isLeaf() { return childCount == 0 && next.getPC() == NOCODE ? OK : NO; }
  Bool isWait() { return flags & Ac_Wait ? OK : NO; }
  Bool isDisWait() { return flags & Ac_DisWait ? OK : NO; }
  void lastClause() { next.setPC(NOCODE); }
  void nextClause(ProgramCounter pc) { next.setPC(pc); }
  void setCommitted() { flags |= Ac_Committed; }
  void setDisWait() { flags |= Ac_DisWait; }

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

  ProgramCounter getElsePC() { return elsePC; }

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
