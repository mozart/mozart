/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifdef __GNUC__
#pragma implementation "board.hh"
#endif


#include "types.hh"

#include "am.hh"
#include "board.hh"
#include "actor.hh"

/* some random comments:
   flags:
     type:
       Ask - Wait - Root
         with addition WaitTop
     needs entailment & failure check
       Nervous
     constraints are realized on heap with BIND
       Installed
     garbage collector
       PathMark
     disjunction has elab the guard 'WAIT'
       Waiting
     state
       active: -
       committed: Committed (entailed or unit committed)
       failed: Failed (constraints are inconsistent)
       discarded: Discarded (sibling is committed)
         -> dead = failed or discarded
       */
     
/*
  class Board
  member data:
    flags: see enum BoardFlags
    suspCount: #tasks + #actors
    body: then continuation
    board: parent board after commitment
    actor: actor who has introced me
     NOTE: parent and actor are shared
    script: the part of trail - when we leave this node;
  comments:
    A series of boards linked with 'parent' is seen as one board with
    the topmost board A as representant.
    It points immediately to A or is a chained link to A,
     if Bo_Board is not set
    */

/*
  enum BoardFlags
    Bo_Ask: is an conditional board
    Bo_Wait: is a disjunctive board
    Bo_Solve: is a solve board; 
    Bo_Root: is the root board
      Bo_WaitTop
    Bo_Installed: board is installed
    Bo_Nervous: board should be visited to check entailment/failure
    Bo_Failed: board has failed
    Bo_Committed
    Bo_Discarded
    */    

Board* Board::getSolveBoard ()
{
  Board *b = this;
  Board *rb = am.rootBoard;
  DebugCheck ((b->isCommitted () == OK),
	      error ("committed board in Board::getSolveBoard ()"));
  while (b != (Board *) NULL && b != rb) {
    if (b->isSolve () == OK)
      return (b);
    b = (b->getParentBoard ())->getBoardDeref ();
  }
  return ((Board *) NULL); 
}

Board::Board(Actor *a,int typ)
: ConstTerm(Co_Board)
{
  Assert(a!=NULL || typ==Bo_Root);
  Assert (typ==Bo_Root || typ==Bo_Ask || typ==Bo_Wait || typ==Bo_Solve);
  flags=typ;
  if (a != (Actor *) NULL && a->isAskWait () == OK) {
    (AWActor::Cast (a))->addChild(this);
  }
  suspCount=0;
  u.actor=a;
}

Board::~Board() {
  error("mm2: not yet impl");
}

// -------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#include "board.icc"
#undef inline
#endif
