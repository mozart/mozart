/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Christian Schulte (schulte@dfki.de)
 * 
 *  Contributors:
 *    Michael Mehl (mehl@dfki.de)
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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

// Solver

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "solve.hh"
#endif

#include "solve.hh"
#include "board.hh"
#include "variable.hh"

#ifdef OUTLINE
#define inline
#include "solve.icc"
#undef inline
#endif

/*
 * class SolveActor:
 *    solve actor;
 *    member data:
 *      solveBoard: 'home' board of a search problem; 
 *      orActors: all actors that may be distributed; 
 *      solveVar: root variable of a search problem; 
 *      result: reference to the control variable 
 *         (recall that there is single control variable, but not three 
 *           as in kernel definition);
 *      suspList: list of external threads; 
 *      threads: the number of *runnable* threads!
 */

/* ------------------------------------------------------------------------
   class SolveActor
   ------------------------------------------------------------------------ */

TaggedRef SolveActor::genSolved() {
  ozstat.incSolveSolved();
  SRecord *stuple = SRecord::newSRecord(AtomSucceeded, 1);

  Assert(solveBoard->isSolve());
  stuple->setArg(0, AtomEntailed);
  
  return makeTaggedSRecord(stuple);
}

TaggedRef SolveActor::genStuck() {
  SRecord *stuple = SRecord::newSRecord(AtomSucceeded, 1);
  
  Assert(solveBoard->isSolve());
  stuple->setArg(0, AtomSuspended);
  return makeTaggedSRecord(stuple);
}

TaggedRef SolveActor::genChoice(int noOfClauses) {
  SRecord *stuple = SRecord::newSRecord(AtomAlt, 1);

  Assert(!solveBoard->isCommitted());
  stuple->setArg(0, makeTaggedSmallInt(noOfClauses));

  return makeTaggedSRecord(stuple);
}

TaggedRef SolveActor::genFailed() {
  ozstat.incSolveFailed();
  return AtomFailed;
}

TaggedRef SolveActor::genUnstable(TaggedRef arg) {
  SRecord *stuple = SRecord::newSRecord(AtomBlocked, 1);
  stuple->setArg(0, arg);
  return makeTaggedSRecord(stuple);
}

SolveActor::SolveActor(Board *bb)
 : Actor (Ac_Solve, bb), cpb(NULL), suspList (NULL), threads (0), 
   nonMonoSuspList(NULL) {
  result     = oz_newVar(bb);
  solveBoard = new Board(this, Bo_Solve);
  solveVar   = oz_newVar(solveBoard);
#ifdef CS_PROFILE
  orig_start  = (int32 *) NULL;
  copy_start  = (int32 *) NULL;
  copy_size   = 0;
#endif
}

#ifdef CS_PROFILE
TaggedRef SolveActor::getCloneDiff(void) {
  TaggedRef l = nil();
  
  if (copy_start && orig_start && (copy_size>0)) {
    int n = 0;

    while (n < copy_size) {
      if (copy_start[n] != orig_start[n]) {
	int d = 0;
	
	while ((n < copy_size) && (copy_start[n] != orig_start[n])) {
	  d++; n++;
	}
      
	LTuple *lt = new LTuple();
	lt->setHead(newSmallInt(d));
	lt->setTail(l);
	l = makeTaggedLTuple(lt);
      } else {
	n++;
      }
      
    }

    free(copy_start);

    TaggedRef ret = OZ_pair2(newSmallInt(copy_size),l);

    copy_start = (int32 *) 0;
    copy_size  = 0;
    orig_start = (int32 *) 0;

    return ret;
    
  } else {
    return OZ_pair2(newSmallInt(0),l);
  }

}
#endif

//-----------------------------------------------------------------------------
// support for nonmonotonic propagators

void SolveActor::addToNonMonoSuspList(Propagator * prop)
{
  nonMonoSuspList = nonMonoSuspList->insert(prop);
}

void SolveActor::mergeNonMonoSuspListWith(OrderedSuspList * p)
{
  for (; p != NULL; p = p->getNext())
    nonMonoSuspList = nonMonoSuspList->insert(p->getPropagator()); 
}

//-----------------------------------------------------------------------------
