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
  Ac_None       = 0,
  Ac_Ask        = 0x01,
  Ac_Wait       = 0x02,
  Ac_Solve      = 0x04,
  Ac_Committed  = 0x08,
  Ac_WaitTop    = 0x10,
  Ac_DisWait    = 0x20,
  Ac_SolveDet   = 0x40          // it means, that there is more seq. work in solve;
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

AWActor *CastAWActor(Actor *a);

// ------------------------------------------------------------------------

class AskActor : public AWActor {
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

AskActor *CastAskActor(Actor *a);

// ------------------------------------------------------------------------

class WaitActor : public AWActor {
friend class SolveActor;
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
  void unsetBoard ();
  void setBoard (Board *bb);
};

WaitActor *CastWaitActor(Actor *a);

// ------------------------------------------------------------------------
//  'solve' actors;

class SolveActor : public Actor {
public:
  static void Init();
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
  ~SolveActor ();

  void gcRecurse();

  void incThreads ();
  void decThreads ();
  Bool isSolveDet () { return ((flags & Ac_SolveDet) ? OK : NO); }
  void unsetSolveDet () { flags &= ~Ac_SolveDet; }
  Bool isStable ();  // so simple!
  void addSuspension (Suspension *susp);
  void addSuspension (SuspList *l);
  Bool areNoExtSuspensions ();
  TaggedRef* getSolveVarRef () { return (&solveVar); }
  TaggedRef getSolveVar () { return (solveVar); }
  TaggedRef getResult () { return (result); }
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

SolveActor *CastSolveActor (Actor *a);

//  Very special thing:
// The procedure that converts the DLLStackEntry to the Actor* (in our case),
// collects it and returns the DLLStackEntry again (for gc);
DLLStackEntry actorStackEntryGC (DLLStackEntry entry);

//  This is 'de facto' the "solve actor";
//  If BIsolve is applied, CFuncCont is generated containing request to call
// this procedure (type OZ_CFun!);
OZ_Bool solveActorWaker (int n, TaggedRef *args);

// ------------------------------------------------------------------------

#ifndef OUTLINE
#include "actor.icc"
#endif

#endif
