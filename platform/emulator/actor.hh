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

#ifdef __GNUC__
#pragma interface
#endif

// ------------------------------------------------------------------------
//  all 'proper' actors; 

enum ActorFlags {
  Ac_None	= 0,
  Ac_Ask	= 0x01,
  Ac_Wait	= 0x02,
  Ac_Solve      = 0x04,
  Ac_Committed	= 0x08,
  Ac_EatWaits   = 0x10, // in disjunction with Ac_Solve
  Ac_Guided     = 0x20, // in disjunction with Ac_Solve
  Ac_Debug      = 0x40  // in disjunction with Ac_Solve
};

class Actor : public ConstTerm {
public:
protected:
  int flags;
  Board *board;
  short priority;
  short compMode;
public:
  Actor(int typ,Board *bb,int prio,int compMode);

  ~Actor() { flags=Ac_None; board=0; }

  USEHEAPMEMORY;
  Actor *gcActor();
  void gcRecurse(void);
  OZPRINT;
  OZPRINTLONG;

  int getPriority() { return (priority); }
  int getCompMode() { return (compMode); }
  Board *getBoardFast();
  Board *getBoardAndTest();
  Bool isCommitted() { return flags & Ac_Committed; }
  Bool isAsk() { return ((flags & Ac_Ask) ? OK : NO); }
  Bool isWait() { return ((flags & Ac_Wait) ? OK : NO); }
  Bool isAskWait () { return ((flags & (Ac_Ask|Ac_Wait)) ? OK : NO); } 
  Bool isSolve () { return ((flags & Ac_Solve) ? OK : NO); }
  Bool isEatWaits() { return ((flags & Ac_EatWaits) ? OK : NO); }
  Bool isGuided() { return ((flags & Ac_Guided) ? OK : NO); }
  Bool isDebug() { return ((flags & Ac_Debug) ? OK : NO); }
  void setCommitted() { flags |= Ac_Committed; }
  void setEatWaits() { flags |= Ac_EatWaits; }
  void setGuided() { flags |= Ac_Guided; }
  void setDebug() { flags |= Ac_Debug; }
};

// ------------------------------------------------------------------------
//  'ask' or 'wait' actors; 

class AWActor : public Actor {
public:
  static AWActor *Cast(Actor *a)
{
  DebugCheck((a->isAskWait () == NO), error ("AWActor::Cast"));
  return ((AWActor *) a);
}
protected:
  Continuation next;
  int childCount;
public:
  AWActor(int type,Board *s,int prio,int compMode,
	  ProgramCounter p=NOCODE,RefsArray y=0,RefsArray g=0,
	  RefsArray x=0,int i=0);
  ~AWActor();

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
public:
  static AskActor *Cast(Actor *a)
  { DebugCheck(!a->isAsk(),error("AskActor::Cast")); return (AskActor *) a; }
private:
  ProgramCounter elsePC;
public:
  AskActor(Board *s,int prio,int compMode,
	   ProgramCounter elsepc,
	   ProgramCounter p, RefsArray y,RefsArray g, RefsArray x, int i);
  ~AskActor();

  void gcRecurse();

  ProgramCounter getElsePC() { return elsePC; }
};

// ------------------------------------------------------------------------

class WaitActor : public AWActor {
public:
  static WaitActor *Cast(Actor *a)
  {
    Assert(a->isWait()); return (WaitActor *) a;
  }
private:
  Board **childs;
public:
  WaitActor(Board *s,int prio,int compMode,
	    ProgramCounter p,RefsArray y,RefsArray g,RefsArray x,int i);
  WaitActor (WaitActor *wa);  // without childs; 
  ~WaitActor();

  void gcRecurse();

  void addChildInternal(Board *n);
  void failChildInternal(Board *n);
  Board *getChild();
  // returns the first created child; this child is unlinked from the actor;
  Board *getChildRef ();
  // the same, but a child is not unlinked from the actor; 
  Board *getChildRefAt(int i) { return childs[i]; }
  void decChilds () { childCount--; }    // for search; 
  /* see also: isLeaf() */
  int getChildCount() { return childCount; };
  Bool hasOneChild() { return ((childCount == 1 && !hasNext()) ? OK : NO); }
  Bool hasNoChilds() { return ((childCount == 0 && !hasNext()) ? OK : NO); }
  int selectChildren(int l, int r);

  // during copying: unlink the distributed actor
  void unsetBoard() { board = 0; }
  Bool hasUnsetBoard() { return board == 0; }
  void setBoard(Board *bb) {  board = bb; }
};

// ------------------------------------------------------------------------

#ifndef OUTLINE
#include "board.hh"
#include "actor.icc"
#endif

#endif
