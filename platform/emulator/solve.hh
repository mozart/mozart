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

#include "cpbag.hh"

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
  CpBag     *cpb;
  TaggedRef solveVar;
  TaggedRef result;
  SuspList  *suspList;
  LocalThreadQueue * localThreadQueue;
  int threads;
public:
  SolveActor();
  ~SolveActor();
  SolveActor(SolveActor&);
  SolveActor(Board *bb);

  Board *getSolveBoard() { return solveBoard; }

  void gcRecurse();

  void incThreads() { threads++; }
  void incThreads(int n) { threads += n; }
  int  decThreads() { Assert (threads > 0); return (--threads); }
  int  getThreads() { return threads; }

  void addSuspension(Thread *thr); 
  void addSuspension(SuspList *l);
  Bool areNoExtSuspensions();

  void inject(int prio, TaggedRef proc);
  int choose(int left, int right);
  TaggedRef merge(Board* bb, int isSibling);
  Board *clone(Board *bb);
  void clearResult(Board *bb);

  Bool isBlocked();
  
  TaggedRef getResult() { return result; }
  void setResult(TaggedRef v) { result = v; }

  void addChoice(WaitActor *wa) {
    cpb = cpb->add(wa);
  }
  void mergeChoices(CpBag *pcpb) {
    cpb = cpb->merge(pcpb);
  }
  CpBag *getCpb() { return cpb; } 

  WaitActor *getChoice() {
    WaitActor * wa;
    cpb = cpb->get(&wa);
    return wa;
  } 
  void removeChoice() {
    cpb = cpb->remove();
  } 

  void setBoard(Board *bb) { board = bb; }

  TaggedRef genSolved();
  TaggedRef genStuck();
  TaggedRef genChoice(int noOfClauses);
  TaggedRef genFailed();
  TaggedRef genUnstable(TaggedRef arg);

  void pushToLTQ(Thread * thr, Board * b);

  void resetLocalThreadQueue(void) {
    localThreadQueue->getLTQThread()->getTaskStackRef()->makeEmpty();
    Assert(localThreadQueue);
    localThreadQueue->dispose ();
    localThreadQueue = NULL;
  }
  LocalThreadQueue * getLocalThreadQueue(void) {
    return localThreadQueue;
  }
  void setLocalThreadQueue(LocalThreadQueue * ltq) {
    localThreadQueue = ltq;
  }

  void clearSuspList(Thread *killThr = NULL);
private:
  Bool checkExtSuspList () {
    clearSuspList();		// Christian's; (no spaces!);
    return (suspList == NULL);
  }
};

#ifndef OUTLINE
#include "solve.icc"
#endif

#endif
