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

/* class Actor:
     may be conditional or disjunction
     member data:
       flags: see enum ActorFlags
       next: continuation for next clause
       board: board in which actor is installed
     */



// ------------------------------------------------------------------------

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



// ------------------------------------------------------------------------

/* class AskActor:
   member data
     elsePC: programm counter of else
     */


// ------------------------------------------------------------------------

/* class WaitActor
    member data
      childs: list of childs
      */



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

// ------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#include "actor.icc"
#undef inline
#endif
