/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  implementation of proper actors
  ------------------------------------------------------------------------
*/

#ifndef __ACTORH
#define __ACTORH

#ifdef INTERFACE
#pragma interface
#endif

#ifdef OUTLINE
#define inline
#endif

#include "cpstack.hh"

// ------------------------------------------------------------------------
//  all 'proper' actors;

enum ActorFlags {
  Ac_None       = 0,
  Ac_Ask        = 0x01,
  Ac_Wait       = 0x02,
  Ac_Solve      = 0x04,
  Ac_Committed  = 0x08,
  Ac_Debug      = 0x10, // in disjunction with Ac_Solve
  Ac_Choice     = 0x20, // in disjunction with Ac_Wait
};

class Actor : public ConstTerm {
public:
protected:
  int flags;
  Board *board;
  int priority;
public:
  Actor(int typ,Board *bb,int prio);

  ~Actor() {}

  USEHEAPMEMORY;
  Actor *gcActor();
  void gcRecurse(void);
  OZPRINT;
  OZPRINTLONG;

  int getPriority() { return (priority); }
  inline Board *getBoardFast();
  inline Board *getBoardAndTest();
  Bool isCommitted() { return flags & Ac_Committed; }
  Bool isAsk() { return ((flags & Ac_Ask) ? OK : NO); }
  Bool isWait() { return ((flags & Ac_Wait) ? OK : NO); }
  Bool isAskWait () { return ((flags & (Ac_Ask|Ac_Wait)) ? OK : NO); }
  Bool isSolve () { return ((flags & Ac_Solve) ? OK : NO); }
  Bool isDebug() { return ((flags & Ac_Debug) ? OK : NO); }
  Bool isChoice() { return ((flags & Ac_Choice) ? OK : NO); }
  void setCommitted() { flags |= Ac_Committed; }
  void setDebug() { flags |= Ac_Debug; }
};

// ------------------------------------------------------------------------
//  'ask' or 'wait' actors;

class AWActor : public Actor {
public:
  static AWActor *Cast(Actor *a) {
    DebugCheck((a->isAskWait () == NO), error ("AWActor::Cast"));
    return ((AWActor *) a);
  }
protected:
  Continuation next;
  int childCount;
public:
  AWActor(int type,Board *s,int prio,
          ProgramCounter p=NOCODE,RefsArray y=0,RefsArray g=0,
          RefsArray x=0,int i=0);
  USEHEAPMEMORY;

  void addChild(Board *n);
  void failChild(Board *n);
  Continuation *getNext() { return &next; }
  Bool hasNext() { return ((next.getPC() == NOCODE) ? NO : OK); }
  /* see also: hasOneChild() */
  Bool isLeaf() { return ((childCount == 0 && next.getPC() == NOCODE) ? OK : NO); }
  void lastClause() { next.setPC(NOCODE); }
  void nextClause(ProgramCounter pc) { next.setPC(pc); }
};

// ------------------------------------------------------------------------

class AskActor : public AWActor {
private:
  Thread *thread;
public:
  static AskActor *Cast(Actor *a)
  { DebugCheck(!a->isAsk(),error("AskActor::Cast")); return (AskActor *) a; }
private:
  ProgramCounter elsePC;
public:
  AskActor(Board *s,int prio,
           ProgramCounter elsepc,
           ProgramCounter p, RefsArray y,RefsArray g, RefsArray x, int i,
           Thread *tt);

  void gcRecurse();

  ProgramCounter getElsePC() { return elsePC; }
  void setThread(Thread *th) { thread = th; }
  Thread *getThread() { return thread; }
};

// ------------------------------------------------------------------------

class WaitActor : public AWActor {
public:
  static WaitActor *Cast(Actor *a)
  {
    Assert(a->isWait()); return (WaitActor *) a;
  }
private:
  Board   **childs;
  CpStack *cps;
public:
  WaitActor(Board *s,int prio,
            ProgramCounter p,RefsArray y,RefsArray g,RefsArray x, int i, Bool d);

  USEFREELISTMEMORY;

  void gcRecurse();

  void addChildInternal(Board *n);
  void failChildInternal(Board *n);
  Board *getLastChild() { Board* b=childs[0]; childs[0] = NULL; return b; }
  // returns the first created child; this child is unlinked from the actor;
  Board *getChildRef() { return childs[0]; }
  // the same, but a child is not unlinked from the actor;
  int getChildCount() { return childCount; };
  Bool hasOneChild() { return ((childCount == 1 &&
                                !isChoice() &&
                                !hasNext()) ? OK : NO); }
  Bool hasNoChilds() { return ((childCount == 0 && !hasNext()) ? OK : NO); }
  int selectOrFailChildren(int l, int r);

  void dispose(void);

  void pushChoice(WaitActor *wa);
  void pushChoices(CpStack *pcps);
  Bool hasChoices();

  CpStack *getCps() { return cps; }
};

// ------------------------------------------------------------------------

#ifdef OUTLINE
#undef inline
#else
#include "board.hh"
#include "actor.icc"
#endif

#endif
