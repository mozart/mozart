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
  static void Init();
//  This is 'de facto' the "solve actor";
//  If BIsolve is applied, CFuncCont is generated containing request to call
// this procedure (type OZ_CFun!);
//  Very special thing:
// The procedure that converts the DLLStackEntry to the Actor* (in our case),
// collects it and returns the DLLStackEntry again (for gc);
  static DLLStackEntry StackEntryGC (DLLStackEntry entry);
private:
  Board *solveBoard;
  DLLStack orActors;
  TaggedRef solveVar;
  TaggedRef result;
  TaggedRef guidance;
  SuspList *suspList;
  SuspList * stable_sl;
  int threads;
public:
  SolveActor (Board *bb, int prio,
              TaggedRef resTR, TaggedRef guiTR=0);

  void setSolveBoard(Board *bb);
  Board *getSolveBoard() { return solveBoard; }

  void gcRecurse();

  void incThreads (int n=1) {
    Assert(threads+n >= 0);
    threads+=n;
  }
  int getThreads() { return threads; }
  void decThreads () { incThreads(-1); }
  void setUnStable() { threads=-1; }
  void addSuspension (Suspension *susp);
  void addSuspension (SuspList *l);
  Bool areNoExtSuspensions();

  TaggedRef* getSolveVarRef() { return (&solveVar); }
  TaggedRef getSolveVar() { return (solveVar); }

  TaggedRef getResult() { return result; }
  void setResult(TaggedRef v) { result = v; }

  TaggedRef getGuidance() { return (guidance); }
  void setGuidance(TaggedRef guiTR) { guidance = guiTR; }

  void pushWaitActor (WaitActor *a);
  void pushWaitActorsStackOf (SolveActor *sa);
  WaitActor *getDisWaitActor ();

  Bool stable_wake(void);
  void add_stable_susp(Suspension *);

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
