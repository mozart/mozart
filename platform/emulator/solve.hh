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

#include "cpstack.hh"

// ------------------------------------------------------------------------

class SolveActor : public Actor {
public:
  OZPRINT;
  OZPRINTLONG;

  static SolveActor *Cast(Actor *a) {
    Assert(a->isSolve());
    return ((SolveActor *) a);
  }
private:
  Board     *solveBoard;
  CpStack   *cps;
  TaggedRef solveVar;
  TaggedRef result;
  SuspList  *suspList;
  LocalThreadQueue * localThreadQueue;
  int threads;
public:
  SolveActor();
  ~SolveActor();
  SolveActor(SolveActor&);
  SolveActor(Board *bb, int prio);

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

  Bool isBlocked();

  TaggedRef getResult() { return result; }
  void setResult(TaggedRef v) { result = v; }

  void pushChoice(WaitActor *wa) {
    if (cps) { cps->push(wa); } else { cps = new CpStack(wa); }
  }
  void pushChoices(CpStack *pcps) {
    if (cps) { cps->push(pcps); } else { cps = pcps; }
  }
  Bool hasChoices() {
    return (cps ? !cps->isEmpty() : NO);
  }
  CpStack *getCps() { return cps; }

  void popChoice() { Assert(cps); cps->pop(); }
  WaitActor *topChoice() {
    return (cps ? cps->getTop() : (WaitActor *) 0);
  }

  void setBoard(Board *bb) { board = bb; }

  TaggedRef genSolved();
  TaggedRef genStuck();
  TaggedRef genChoice(int noOfClauses);
  TaggedRef genFailed();
  TaggedRef genUnstable(TaggedRef arg);

  void pushToLTQ(Thread * thr, Board * b);

  void resetLocalThreadQueue(void) {
    Assert(localThreadQueue);
    localThreadQueue->dispose ();
    localThreadQueue = NULL;
  }
  LocalThreadQueue * getLocalThreadQueue(void) {
    Assert(localThreadQueue);
    return localThreadQueue;
  }
  void setLocalThreadQueue(LocalThreadQueue * ltq) {
    localThreadQueue = ltq;
  }

  void clearSuspList(Thread *killThr = NULL);
private:
  Bool checkExtSuspList () {
    clearSuspList();            // Christian's; (no spaces!);
    return (suspList == NULL);
  }
};

#ifndef OUTLINE
#include "solve.icc"
#endif

#endif
