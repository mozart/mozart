/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#include "types.hh"

#include "am.hh"
#include "board.hh"
#include "actor.hh"

#ifdef OUTLINE
#define inline
#endif


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
     if Sp_Board is not set
    */

/*
  enum BoardFlags
    Sp_None: is a commited board
    Sp_Ask: is an conditional board
    Sp_Wait: is a disjunctive board
    Sp_Root: is the root board
    Sp_Installed: board is installed
    Sp_Nervous
    Sp_WaitTop
    */

enum BoardFlags {
  Sp_None       = 0,
  Sp_Ask        = 1<<0,
  Sp_Wait       = 1<<1,
  Sp_Root       = 1<<2,
  Sp_Installed  = 1<<3,
  Sp_Nervous    = 1<<4,
  Sp_WaitTop    = 1<<5,
};

Board *Board::Root = new RootBoard;
Board *Board::Current = Board::Root;

#ifdef DEBUG_CHECK
static Board *oldBoard=NULL;
#endif
void Board::SetCurrent(Board *c, Bool checkNotGC)
{
  DebugCheck(checkNotGC &&
             (!oldBoard
              || oldBoard != Current),
             error("someone has changed 'Board::Current'"));
  Current = c;
  am.currentUVarPrototype = makeTaggedUVar(c);
  DebugCheckT(oldBoard=c);
}

Board::Board(Actor *a,int type)
: ConstTerm(Co_Board)
{
  flags=type;
  suspCount=0;
  actor=a;
  script=NULL;
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
  return actor;
}

inline Continuation *Board::getBodyPtr()
{
  return &body;
}

inline Board *Board::getParentBoardDeref()
{
  return actor->getBoardDeref();
}

inline ConsList &Board::getScriptRef()
{
  return script;
}

inline Board *Board::getBoardDeref()
{
  return flags==Sp_None ? board->getBoardDeref() : this;
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
  return flags&Sp_Ask ? OK : NO;
}

inline Bool Board::isCommitted()
{
  return flags == Sp_None ? OK : NO;
}

inline Bool Board::isDead()
{
  return flags == Sp_None && board == (Board *) NULL ? OK : NO;
}

inline Bool Board::isInstalled()
{
  return flags&Sp_Installed ? OK : NO;
}

inline Bool Board::isNervous()
{
  return flags&Sp_Nervous ? OK : NO;
}

inline Bool Board::isWaitTop()
{
  return flags&Sp_WaitTop ? OK : NO;
}

inline Bool Board::isWait()
{
  return flags&Sp_Wait ? OK : NO;
}

inline Bool Board::isRoot()
{
  return flags&Sp_Root ? OK : NO;
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

inline void Board::setInstalled()
{
  flags |= Sp_Installed;
}

inline void Board::setNervous()
{
  flags |= Sp_Nervous;
}

inline void Board::setScript(int i,TaggedRef *v,TaggedRef r)
{
  script[i].setLeft(v);
  script[i].setRight(r);
}

inline void Board::setBoard(Board *s)
{
  flags = Sp_None;
  board = s;
}

inline void Board::setWaitTop()
{
  flags |= Sp_WaitTop;
}

inline void Board::unsetInstalled()
{
  flags &= ~Sp_Installed;
}

inline void Board::unsetNervous()
{
  flags &= ~Sp_Nervous;
}

// -------------------------------------------------------------------------

AskBoard::AskBoard(Actor *a) : Board(a,Sp_Ask)
{
  a->addChild(this);
}

// -------------------------------------------------------------------------

WaitBoard::WaitBoard(Actor *a) : Board(a, Sp_Wait)
{
  a->addChild(this);
}

// -------------------------------------------------------------------------

RootBoard::RootBoard() : Board(NULL, Sp_Root)
{
  setInstalled();
}

#ifdef OUTLINE
#include "board.icc"
#undef inline
#endif
