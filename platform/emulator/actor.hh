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

#include "constterm.hh"
#include "suspension.hh"
#include "dllstack.hh"

// ------------------------------------------------------------------------
//  all 'proper' actors; 

enum ActorFlags {
  Ac_None	= 0,
  Ac_Ask	= 0x01,
  Ac_Wait	= 0x02,
  Ac_Solve      = 0x04,
  Ac_Committed	= 0x08,
  Ac_WaitTop	= 0x10,
  Ac_DisWait	= 0x20
};

class Actor : public ConstTerm {
public:
protected:
  int flags;
  Board *board;
  int priority;
public:
  Actor(int type,Board *s,int prio);
  ~Actor();

  USEHEAPMEMORY;
  Actor *gc();
  void gcRecurse(void);
  OZPRINT;
  OZPRINTLONG;

  int getPriority();
  Board *getBoard();
  Bool isCommitted();
  Bool isAsk();
  Bool isWait();
  Bool isAskWait (); 
  Bool isSolve ();
  Bool isDisWait();
  void setCommitted();
  void setDisWait();
};

// ------------------------------------------------------------------------
//  'ask' or 'wait' actors; 

class AWActor : public Actor {
public:
  static AWActor *Cast(Actor *a);
protected:
  Continuation next;
  int childCount;
public:
  AWActor(int type,Board *s,int prio,
	  ProgramCounter p,RefsArray y,RefsArray g,
	  RefsArray x,int i);
  ~AWActor();

  USEHEAPMEMORY;
  OZPRINT;
  OZPRINTLONG;

  void addChild(Board *n);
  void failChild(Board *n);
  Continuation *getNext();
  Bool hasNext();
  Bool isLeaf();
  void lastClause();
  void nextClause(ProgramCounter pc);
};

// ------------------------------------------------------------------------

class AskActor : public AWActor {
public:
  static AskActor *Cast(Actor *a);
private:
  ProgramCounter elsePC;
public:
  AskActor(Board *s,int prio,
	   ProgramCounter elsepc,
	   ProgramCounter p, RefsArray y,RefsArray g, RefsArray x, int i);
  ~AskActor();

  void gcRecurse();

  ProgramCounter getElsePC();

};

// ------------------------------------------------------------------------

class WaitActor : public AWActor {
public:
  static WaitActor *Cast(Actor *a);
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
  void decChilds ();    // for search; 
  Bool hasOneChild();
  Bool hasNoChilds();
  void unsetBoard () { board = (Board *) NULL; }
  void setBoard (Board *bb) {  board = bb; }
};

// ------------------------------------------------------------------------
//  'solve' actors; 

class SolveActor : public Actor {
public:
  static SolveActor *Cast(Actor *a);
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

  void incThreads ();
  void decThreads ();
  Bool isStable ();  // so simple!
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
  TaggedRef genFailed ();

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
