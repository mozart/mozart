/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  implementation of conditional and disjunctive actors
  ------------------------------------------------------------------------
*/


#include "actor.hh"
#include "board.hh"

#ifdef OUTLINE
#define inline
#endif

enum ActorFlags {
  Ac_None	= 0,
  Ac_Ask	= 1<<0,
  Ac_Wait	= 1<<1,
  Ac_Committed	= 1<<2,
  Ac_WaitTop	= 1<<3,
};

// ------------------------------------------------------------------------

inline Actor::Actor(int type,Board *s,int prio,ProgramCounter p,RefsArray y,
		     RefsArray g,RefsArray x,int i)
: ConstTerm(Co_Actor),flags(type),board(s),priority(prio)
{
  next.setPC(p);
  next.setY(y);
  next.setG(g);
  next.setX(x,i);
}

Actor::~Actor()
{
  flags=Ac_None;
  board=(Board *) NULL;
  next.setPC(0);
  next.setY((RefsArray) NULL);
  next.setG((RefsArray) NULL);
  next.setX((RefsArray) NULL,0);
}

void Actor::addChild(Board *n) {
  childCount++;
  if (isWait()) {
    CastWaitActor(this)->addChildInternal(n);
  }
}

void Actor::failChild(Board *n) {
  childCount--;
  if (isWait()) {
    CastWaitActor(this)->failChildInternal(n);
  }
}

inline Continuation *Actor::getNext()
{
  return &next;
}

inline int Actor::getPriority()
{
  return priority;
}

inline Board *Actor::getBoardDeref()
{
  return board->getBoardDeref();
}

inline Bool Actor::hasNext()
{
  return next.getPC() == NOCODE ? NO : OK;
}

inline Bool Actor::isCommitted()
{
  return flags & Ac_Committed ? OK : NO;
}

inline Bool Actor::isAsk()
{
  return flags & Ac_Ask ? OK : NO;
}

inline Bool Actor::isLeaf()
{
  return (next.getPC() == NOCODE && childCount == 0) ? OK : NO;
}

inline Bool Actor::isWait()
{
  return flags & Ac_Wait ? OK : NO;
}


inline void Actor::lastClause()
{
  next.setPC(NOCODE);
}

inline void Actor::nextClause(ProgramCounter pc)
{
  next.setPC(pc);
}

inline void Actor::setCommitted()
{
  flags |= Ac_Committed;
}

// ------------------------------------------------------------------------

AskActor::AskActor(Board *s,int prio,ProgramCounter elsepc,
		   ProgramCounter p,RefsArray y,
		   RefsArray g,RefsArray x,int i)
: Actor(Ac_Ask,s,prio,p,y,g,x,i)
{
  elsePC = elsepc;
}

inline ProgramCounter AskActor::getElsePC()
{
  return elsePC;
}

AskActor *CastAskActor(Actor *a)
{
  DebugCheck(!a->isAsk(),error("CastAskActor"));
  return (AskActor *) a;
}

// ------------------------------------------------------------------------

WaitActor::WaitActor(Board *s,int prio,ProgramCounter p,RefsArray y,
		     RefsArray g,RefsArray x,int i)
: Actor(Ac_Wait,s,prio,p,y,g,x,i)
{
  childs=NULL;
}


void WaitActor::addChildInternal(Board *n)
{
  error("mm2: not impl");
}

void WaitActor::failChildInternal(Board *n)
{
  error("mm2: not impl");
}

WaitBoard *WaitActor::getChild()
{
  error("mm2: not impl");
}

Bool WaitActor::isThereOneChild()
{
  error("mm2: not impl");
}

WaitActor *CastWaitActor(Actor *a)
{
  DebugCheck(!a->isWait(),error("CastWaitActor"));
  return (WaitActor *) a;
}

// ------------------------------------------------------------------------

#ifdef OUTLINE
#include "actor.icc"
#undef inline
#endif
