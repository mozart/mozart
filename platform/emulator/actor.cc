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


#ifdef __GNUC__
#pragma implementation "actor.hh"
#endif

#include "actor.hh"
#include "board.hh"

#ifdef OUTLINE
#define inline
#endif



/* class Actor:
     may be conditional or disjunction
     member data:
       flags: see enum ActorFlags
       next: continuation for next clause
       board: board in which actor is installed
     */


enum ActorFlags {
  Ac_None       = 0,
  Ac_Ask        = 1<<0,
  Ac_Wait       = 1<<1,
  Ac_Committed  = 1<<2,
  Ac_WaitTop    = 1<<3,
  Ac_DisWait    = 1<<4,
};

// ------------------------------------------------------------------------

inline Actor::Actor(int type,Board *bb,int prio,ProgramCounter p,RefsArray y,
                     RefsArray g,RefsArray x,int i)
: ConstTerm(Co_Actor),flags(type),board(bb),priority(prio)
{
  childCount=0;
  bb->addSuspension();
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

inline Board *Actor::getBoard()
{
  return board;
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
  return childCount == 0 && next.getPC() == NOCODE ? OK : NO;
}

inline Bool Actor::isWait()
{
  return flags & Ac_Wait ? OK : NO;
}

inline Bool Actor::isDisWait()
{
  return flags & Ac_DisWait ? OK : NO;
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

void Actor::setDisWait()
{
  flags |= Ac_DisWait;
}

// ------------------------------------------------------------------------

/* class AskActor:
   member data
     elsePC: programm counter of else
     */

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

/* class WaitActor
    member data
      childs: list of childs
      */

WaitActor::WaitActor(Board *s,int prio,ProgramCounter p,RefsArray y,
                     RefsArray g,RefsArray x,int i)
: Actor(Ac_Wait,s,prio,p,y,g,x,i)
{
  childs=NULL;
}


void WaitActor::addChildInternal(Board *bb)
{
  if (!childs) {
    childs=(Board **) freeListMalloc(3*sizeof(Board *));
    *childs++ = (Board *) 2;
    childs[0] = bb;
    childs[1] = NULL;
    return;
  }
  int max=(int) childs[-1];
  for (int i = 0; i < max; i++) {
    if (!childs[i]) {
      childs[i] = bb;
      return;
    }
  }
  int size = 2*max;
  Board **cc = (Board **) freeListMalloc((size+1)*sizeof(Board *));
  *cc++ = (Board *) size;
  freeListDispose(childs-1,(((int) childs[-1])+1)*sizeof(Board *));
  childs = cc;
  childs[max] = bb;
  for (int j = max+1; j < size; j++) {
    childs[j] = NULL;
  }
}

void WaitActor::failChildInternal(Board *bb)
{
  int max=(int) childs[-1];
  for (int i = 0; i < max; i++) {
    if (childs[i] == bb) {
      childs[i] = NULL;
      return;
    }
  }
  error("WaitActor::failChildInternal");
}

Board *WaitActor::getChild()
{
  int max=(int) childs[-1];
  for (int i = 0; i < max; i++) {
    if (childs[i]) {
      return childs[i];
    }
  }
  error("WaitActor::getChild");
}

// see also: isLeaf()
Bool WaitActor::hasOneChild()
{
  return childCount == 1 && next.getPC() == NOCODE ? OK : NO;
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
