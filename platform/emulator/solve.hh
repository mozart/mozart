/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Christian Schulte (schulte@dfki.de)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

// Solver

#ifndef __SOLVEH
#define __SOLVEH

#ifdef INTERFACE
#pragma interface
#endif

#include "cpbag.hh"
#include "gc.hh"

// ------------------------------------------------------------------------

class SolveActor : public Actor {
public:

  void printLongStreamSolve(ostream &stream, int depth, int offset);

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
  int threads;

#ifdef CS_PROFILE
  int32 * orig_start;
  int32 * copy_start;
  int     copy_size;
#endif

public:
  NO_DEFAULT_CONSTRUCTORS(SolveActor);
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
  WaitActor * select(int left, int right);
  TaggedRef merge(Board* bb, int isSibling);
  Board *clone(Board *bb);
  void clearResult(Board *bb);
  void patchChoiceResult(int i) {
    SRecord *stuple = SRecord::newSRecord(AtomAlt, 1);

    stuple->setArg(0, makeTaggedSmallInt(i));

    result = makeTaggedSRecord(stuple);
  }

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

#ifdef CS_PROFILE
  TaggedRef getCloneDiff(void);
#endif

//-----------------------------------------------------------------------------
// local thread queue
private:
  LocalThreadQueue * localThreadQueue;
public:
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

//-----------------------------------------------------------------------------
// support for nonmonotonic propagators
private:
  OrderedSuspList * nonMonoSuspList;
public:
  void addToNonMonoSuspList(Thread *);
  void scheduleNonMonoSuspList(void);
  void mergeNonMonoSuspListWith(OrderedSuspList *);
  void setNonMonoSuspList(OrderedSuspList * l) {
    nonMonoSuspList = l;
  }

//-----------------------------------------------------------------------------

public:
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
