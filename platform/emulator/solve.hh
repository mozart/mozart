/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  Solver
  ------------------------------------------------------------------------
*/

#ifndef __SOLVEH
#define __SOLVEH

#ifdef INTERFACE
#pragma interface
#endif

// ------------------------------------------------------------------------
//  'solve' actors;

extern BuiltinTabEntry *solveContBITabEntry;
extern BuiltinTabEntry *solvedBITabEntry;

class SolveActor : public Actor {
public:
  OZPRINT;
  OZPRINTLONG;

  static SolveActor *Cast(Actor *a)
  {
    Assert(a->isSolve());
    return ((SolveActor *) a);
  }
//  Very special thing:
// The procedure that converts the DLLStackEntry to the Actor* (in our case),
// collects it and returns the DLLStackEntry again (for gc);
  static DLLStackEntry StackEntryGC (DLLStackEntry entry);
private:
  Board *solveBoard;
  DLLStack orActors;
  TaggedRef solveVar;
  TaggedRef result;
  SuspList *suspList;
  SuspList *stable_sl;
  int threads;
public:
  SolveActor(Board *bb, int prio, Bool debug);

  Board *getSolveBoard() { return solveBoard; }

  void gcRecurse();

  void incThreads() { threads++; }
  int  decThreads() { Assert (threads > 0); return (--threads); }
  int  getThreads() { return threads; }

  void addSuspension(Thread *thr);
  void addSuspension(SuspList *l);
  Bool areNoExtSuspensions();

  void inject(int prio, TaggedRef proc);
  int choose(int left, int right);
  TaggedRef merge(Board* bb);
  Board *clone(Board *bb);
  void clearResult(Board *bb);

  Bool isDebugBlocked();

  TaggedRef getResult() { return result; }
  void setResult(TaggedRef v) { result = v; }

  void pushWaitActor (WaitActor *a);
  void pushWaitActorsStackOf (SolveActor *sa);
  WaitActor *getDisWaitActor ();

  Bool stable_wake(void);
  void add_stable_susp (Thread *thr);
  void setBoard(Board *bb) { board = bb; }

  TaggedRef genSolved();
  TaggedRef genStuck();
  TaggedRef genChoice(int noOfClauses);
  TaggedRef genFailed();
  TaggedRef genUnstable(TaggedRef arg);

private:
  WaitActor* getTopWaitActor ();
  WaitActor* getNextWaitActor ();
  void unlinkLastWaitActor ();
  Bool checkExtSuspList ();
};

#ifndef OUTLINE
#include "solve.icc"
#endif

#endif
