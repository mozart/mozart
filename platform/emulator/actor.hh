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

#include "constter.hh"
#include "suspensi.hh"
#include "dllstack.hh"

// ------------------------------------------------------------------------
//  all 'proper' actors; 

enum ActorFlags {
  Ac_None	= 0,
  Ac_Ask	= 0x01,
  Ac_Wait	= 0x02,
  Ac_Solve      = 0x04,
  Ac_Committed	= 0x08,
  Ac_EatWaits   = 0x10, // in disjunction with Ac_Solve
};

class Actor : public ConstTerm {
public:
protected:
  int flags;
  Board *board;
  int priority;
public:
  Actor(int typ,Board *bb,int prio);

  ~Actor() { flags=Ac_None; board=(Board *) NULL; }

  USEHEAPMEMORY;
  Actor *gc();
  void gcRecurse(void);
  OZPRINT;
  OZPRINTLONG;

  int getPriority() { return (priority); }
  Board *getBoard() { return (board); }
  Bool isCommitted() { return ((flags & Ac_Committed) ? OK : NO); }
  Bool isAsk() { return ((flags & Ac_Ask) ? OK : NO); }
  Bool isWait() { return ((flags & Ac_Wait) ? OK : NO); }
  Bool isAskWait () { return ((flags & (Ac_Ask|Ac_Wait)) ? OK : NO); } 
  Bool isSolve () { return ((flags & Ac_Solve) ? OK : NO); }
  Bool isEatWaits() { return ((flags & Ac_EatWaits) ? OK : NO); }
  void setCommitted() { flags |= Ac_Committed; }
  void setEatWaits() { flags |= Ac_EatWaits; }
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
  AWActor(int type,Board *s,int prio,
	  ProgramCounter p,RefsArray y,RefsArray g,
	  RefsArray x,int i);
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
  AskActor(Board *s,int prio,
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
{ DebugCheck(!a->isWait(),error("WaitActor::Cast")); return (WaitActor *) a; }
private:
  Board **childs;
public:
  WaitActor(Board *s,int prio,
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
  void decChilds () { childCount--; }    // for search; 
  /* see also: isLeaf() */
  Bool hasOneChild() { return ((childCount == 1 && !hasNext()) ? OK : NO); }
  Bool hasNoChilds() { return ((childCount == 0 && !hasNext()) ? OK : NO); }
  void unsetBoard () { board = (Board *) NULL; }
  void setBoard (Board *bb) {  board = bb; }
};

// ------------------------------------------------------------------------
//  'solve' actors; 

class SolveActor : public Actor {
public:
  OZPRINT;
  OZPRINTLONG;

  static SolveActor *Cast(Actor *a)
  {
    Assert(a->isSolve());
    return ((SolveActor *) a);
  }
  static void Init();
//  This is 'de facto' the "solve actor";
//  If BIsolve is applied, CFuncCont is generated containing request to call
// this procedure (type OZ_CFun!);
  static OZ_Bool Waker (int n, TaggedRef *args);
//  Very special thing: 
// The procedure that converts the DLLStackEntry to the Actor* (in our case),
// collects it and returns the DLLStackEntry again (for gc);
  static DLLStackEntry StackEntryGC (DLLStackEntry entry);
private:
  Board *solveBoard;
  DLLStack orActors;
  TaggedRef solveVar;
  TaggedRef result;
  Board *boardToInstall;
  SuspList *suspList;
  int threads;
public:
  SolveActor (Board *bb, int prio, TaggedRef resTR);
  void setSolveBoard(Board *bb);
  ~SolveActor ();

  void gcRecurse();

  void incThreads (int n=1) {
    Assert(threads+n >= 0);
    threads+=n;
  }
  void decThreads () { incThreads(-1); }
  Bool isStable ();  // so simple!
  void setUnStable() { threads=-1; }
  void addSuspension (Suspension *susp); 
  void addSuspension (SuspList *l);
  Bool areNoExtSuspensions ();
  TaggedRef* getSolveVarRef () { return (&solveVar); }
  TaggedRef getSolveVar () { return (solveVar); }
  TaggedRef getResult () { return (makeTaggedRef (&result)); }
  void pushWaitActor (WaitActor *a);
  void pushWaitActorsStackOf (SolveActor *sa);
  WaitActor *getDisWaitActor ();
  void unsetBoard () { board = (Board *) NULL; }
  void setBoard (Board *bb) { board = bb; }
  void setBoardToInstall (Board *bb) { boardToInstall = bb; }
  Board* getBoardToInstall () { return (boardToInstall); }
  TaggedRef genSolved ();
  TaggedRef genStuck ();
  TaggedRef genEnumed (Board *newSolveBB);
  TaggedRef genEnumedFail ();
  TaggedRef genFailed ();
  void printDebugKP();

private:
  WaitActor* getTopWaitActor ();
  WaitActor* getNextWaitActor ();
  void unlinkLastWaitActor (); 
  Bool checkExtSuspList ();
};

// ------------------------------------------------------------------------

#ifndef OUTLINE
#include "actor.icc"
#endif

#endif
