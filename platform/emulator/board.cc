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

#ifdef OUTLINE
#define inline
#endif

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
    Bo_Root: is the root board
      Bo_WaitTop
    Bo_Installed: board is installed
    Bo_Nervous: board should be visited to check entailment/failure
    Bo_Failed: board has failed
    Bo_Committed
    Bo_Discarded
    */

enum BoardFlags {
  Bo_Ask        = 1<<0,
  Bo_Wait       = 1<<1,
  Bo_Root       = 1<<2,
  Bo_Installed  = 1<<3,
  Bo_Nervous    = 1<<4,
  Bo_WaitTop    = 1<<5,
  Bo_PathMark   = 1<<6,
  Bo_Failed     = 1<<7,
  Bo_Committed  = 1<<8,
  Bo_Discarded  = 1<<9,
  Bo_Waiting    = 1<<10
};

Board *Board::Root;
Board *Board::Current;

void Board::Init() {
  Root = new Board(NULL,Bo_Root);
  Root->setInstalled();
  SetCurrent(Root,NO);
}
void Board::NewCurrentAsk(Actor *a)
{
  Board *b=new Board(a,Bo_Ask);
  b->setInstalled();
  SetCurrent(b,OK);
}

void Board::NewCurrentWait(Actor *a)
{
  Board *b=new Board(a,Bo_Wait);
  b->setInstalled();
  SetCurrent(b,OK);
}

#ifdef DEBUG_CHECK
static Board *oldBoard = Board::GetRoot();
#endif
void Board::SetCurrent(Board *c, Bool checkNotGC)
{
  DebugCheck(!c,error("Board::SetCurrent"));
  DebugCheck(checkNotGC &&
             (!oldBoard
              || oldBoard != Current),
             error("someone has changed 'Board::Current'"));
  Current = c;
  am.currentUVarPrototype = makeTaggedUVar(c);
  DebugCheckT(oldBoard=c);
}

Board *Board::GetCurrent()
{
  return Current;
}

Board *Board::GetRoot()
{
  return Root;
}

Board::Board(Actor *a,int typ)
: ConstTerm(Co_Board)
{
  DebugCheck(!a && typ!=Bo_Root,error("Board::Board"));
  DebugCheck(typ!=Bo_Root && typ!=Bo_Ask && typ!=Bo_Wait,
             error("Board::Board"));
  flags=typ;
  if (a) {
    a->addChild(this);
  }
  suspCount=0;
  actor=a;
}

Board::~Board() {
  error("mm2: not yet impl");
}

inline void Board::addSuspension()
{
  suspCount++;
}

inline Actor *Board::getActor()
{
  DebugCheck(isCommitted(),error("Board::getActor"));
  return actor;
}

inline Continuation *Board::getBodyPtr()
{
  return &body;
}

inline Board *Board::getParentBoard()
{
  DebugCheck(isCommitted(),error("Board::getParentBoard"));
  return actor->getBoard();
}

inline ConsList &Board::getScriptRef()
{
  return script;
}

inline Board *Board::getBoard()
{
  return board;
}

/* return NULL if board is dead */
inline Board *Board::getBoardDeref()
{
  Board *bb = this;
  while (OK) {
    if (bb->isDiscarded() || bb->isFailed()) {
      return NULL;
    } else if (bb->isCommitted()) {
      bb = bb->board;
    } else {
      return bb;
    }
  }
}

inline int Board::getSuspCount()
{
  return suspCount;
}

inline Bool Board::hasSuspension()
{
  return suspCount == 0 ? NO : OK;
}

inline Bool Board::isAsk()
{
  return flags & Bo_Ask ? OK : NO;
}

inline Bool Board::isCommitted()
{
  return flags & Bo_Committed ? OK : NO;
}

/* are we a sibling of a committed board ?
   caution: handle root node correctly */
inline Bool Board::isDiscarded()
{
  Bool ret=NO;
  if (flags & Bo_Discarded) {
    ret = OK;
  } else if (actor && actor->isCommitted()) {
    DebugCheck(isInstalled(),error("Board: discarded & installed"));
    flags |= Bo_Discarded;
    ret = OK;
  }
  return ret;
}

inline Bool Board::isFailed()
{
  return flags & Bo_Failed ? OK : NO;
}

inline Bool Board::isInstalled()
{
  return flags & Bo_Installed ? OK : NO;
}

inline Bool Board::isNervous()
{
  return flags & Bo_Nervous ? OK : NO;
}

inline Bool Board::isWaiting()
{
  return flags & Bo_Waiting ? OK : NO;
}

inline Bool Board::isPathMark()
{
  return flags & Bo_PathMark ? OK : NO;
}

inline Bool Board::isWaitTop()
{
  return flags & Bo_WaitTop ? OK : NO;
}

inline Bool Board::isWait()
{
  return flags & Bo_Wait ? OK : NO;
}

inline Bool Board::isRoot()
{
  return flags & Bo_Root ? OK : NO;
}

inline void Board::newScript(int size)
{
  script.allocate(size);
}

inline void Board::removeSuspension()
{
  DebugCheck(suspCount<=0, error("removeSuspension"));
  suspCount--;
}

inline void Board::setBody(ProgramCounter p,RefsArray y,
                            RefsArray g,RefsArray x,int i)
{
  body.setPC(p);
  body.setY(y);
  body.setG(g);
  body.setX(x,i);
}

Actor *Board::FailCurrent()
{
  DebugCheck(!Current->isInstalled(),error("Board::FailCurrent"));
  Actor *ret=Current->getActor();
  ret->failChild(Current);
  Current->flags |= Bo_Failed;
  am.reduceTrailOnFail();
  Current->unsetInstalled();
  SetCurrent(ret->getBoard()->getBoardDeref());
  return ret;
}

inline void Board::setInstalled()
{
  flags |= Bo_Installed;
}

inline void Board::setNervous()
{
  flags |= Bo_Nervous;
}

inline void Board::setWaiting()
{
  flags |= Bo_Waiting;
}

inline void Board::setPathMark()
{
  flags |= Bo_PathMark;
}

inline void Board::setScript(int i,TaggedRef *v,TaggedRef r)
{
  script[i].setLeft(v);
  script[i].setRight(r);
}

inline void Board::setCommitted(Board *s)
{
  DebugCheck(isInstalled(),error("setCommitted"));
  flags |= Bo_Committed;
  actor->setCommitted();
  board = s;
}

inline void Board::setWaitTop()
{
  flags |= Bo_WaitTop;
}

inline void Board::unsetInstalled()
{
  flags &= ~Bo_Installed;
}

inline void Board::unsetNervous()
{
  flags &= ~Bo_Nervous;
}

inline void Board::unsetPathMark()
{
  flags &= ~Bo_PathMark;
}

// -------------------------------------------------------------------------


#ifdef OUTLINE
#include "board.icc"
#undef inline
#endif
