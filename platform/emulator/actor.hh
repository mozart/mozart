/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl

  implementation of proper actors
  ------------------------------------------------------------------------
*/

#ifndef __ACTORH
#define __ACTORH

#ifdef INTERFACE
#pragma interface
#endif

#include "cpbag.hh"

// ------------------------------------------------------------------------
//  all 'proper' actors; 

enum ActorFlags {
  Ac_None	= 0,
  Ac_Ask	= 0x01,
  Ac_Wait	= 0x02,
  Ac_Solve      = 0x04,
  Ac_Committed	= 0x08,
  Ac_Choice     = 0x10, // in disjunction with Ac_Wait
  Ac_Ground     = 0x20  // in disjunction with Ac_Solve 
};

class Actor : public ConstTerm {
public:
protected:
  int flags;
  Board *board;
public:
  Actor(); // fake for compiler
  Actor(Actor &); // fake for compiler
  // ~Actor(); // fake for compiler

protected:
  Actor(int typ,Board *bb)
    : ConstTerm(Co_Actor),board(bb)
  {
    flags = typ;
  }

public:
  USEHEAPMEMORY;
  Actor *gcActor();
  void gcRecurse(void);
  OZPRINT;
  OZPRINTLONG;

  Board *getBoardInternal() { return board; }
  Bool isCommitted() { return flags & Ac_Committed; }
  Bool isAsk() { return ((flags & Ac_Ask) ? OK : NO); }
  Bool isWait() { return ((flags & Ac_Wait) ? OK : NO); }
  Bool isAskWait() { return ((flags & (Ac_Ask|Ac_Wait)) ? OK : NO); } 
  Bool isSolve() { return ((flags & Ac_Solve) ? OK : NO); }
  Bool isChoice() { return ((flags & Ac_Choice) ? OK : NO); }
  Bool isGround() { return ((flags & Ac_Ground) ? OK : NO); }
  void setCommitted() { flags |= Ac_Committed; }
  void setGround() { flags |= Ac_Ground; }
  void unsetGround() { flags &= ~Ac_Ground; }

  void discardActor() { setCommitted(); }
};

// ------------------------------------------------------------------------
//  'ask' or 'wait' actors; 

class AWActor : public Actor {
public:
  static AWActor *Cast(Actor *a) {
    DebugCheck((a->isAskWait () == NO), error ("AWActor::Cast"));
    return ((AWActor *) a);
  }
protected:
  Thread *thread;
  Continuation next;
  int childCount;
public:
  AWActor();  // fake for compiler
  //~AWActor();  // fake for compiler
  AWActor(AWActor&);  // fake for compiler
  AWActor(int typ,Board *bb,Thread *tt,
	  ProgramCounter p=NOCODE,RefsArray y=0,RefsArray g=0,
	  RefsArray x=0,int i=0)
    : Actor (typ, bb)
  {
    thread=tt;
    childCount=0;
    next.setPC(p);
    next.setY(y);
    next.setG(g);
    next.setX(x,i);
  }

  USEHEAPMEMORY;

  void gcRecurse(void);
  void addChild() {
    childCount++;
  }

  void failChild() {
    childCount--;
    Assert(childCount>=0);
  }
  Continuation *getNext() { return &next; }
  Bool hasNext() { return ((next.getPC() == NOCODE) ? NO : OK); }
  /* see also: hasOneChild() */
  Bool isLeaf() { return childCount == 0 && next.getPC() == NOCODE; }
  void lastClause() { next.setPC(NOCODE); }
  void nextClause(ProgramCounter pc) { next.setPC(pc); }
  void setThread(Thread *th) { thread = th; }
  Thread *getThread() { return thread; }
};

// ------------------------------------------------------------------------

class AskActor : public AWActor {
public:
  static AskActor *Cast(Actor *a)
  { DebugCheck(!a->isAsk(),error("AskActor::Cast")); return (AskActor *) a; }
private:
  ProgramCounter elsePC;
public:
  AskActor(); // fake for compiler
  ~AskActor(); // fake for compiler
  AskActor(AskActor&); // fake for compiler
  AskActor(Board *s,Thread *tt,
	   ProgramCounter elsepc,
	   ProgramCounter p, RefsArray y,RefsArray g, RefsArray x, int i)
    : AWActor(Ac_Ask,s,tt,p,y,g,x,i)
  {
    elsePC = elsepc;
  }

  void addAskChild(Board *bb) {
    addChild();
  }
  void failAskChild(Board *n) {
    failChild();
  }

  void gcRecurse();

  ProgramCounter getElsePC() { return elsePC; }
};

// ------------------------------------------------------------------------

class WaitActor : public AWActor {
public:
  static WaitActor *Cast(Actor *a)
  {
    Assert(a->isWait()); return (WaitActor *) a;
  }
private:
  Board   **children;
  CpBag *cpb;
public:
  WaitActor();
  ~WaitActor();
  WaitActor(WaitActor&);
  WaitActor(Board *s,Thread *tt,
	    ProgramCounter p,RefsArray y,RefsArray g,RefsArray x, int i,
	    Bool d)
    : AWActor((d ? (ActorFlags)(Ac_Wait | Ac_Choice) : Ac_Wait),s,tt,
	      p,y,g,x,i)
  {
    children  = NULL;
    cpb       = NULL;
  }

  USEFREELISTMEMORY;

  void gcRecurse();

  void addWaitChild(Board *n);
  void failWaitChild(Board *n);
  Board *getLastChild() { Board* b=children[0]; children[0] = NULL; return b; }
  // returns the first created child; this child is unlinked from the actor;
  Board *getChildRef() { return children[0]; }
  // the same, but a child is not unlinked from the actor; 
  int getChildCount() { return childCount; };
  Bool hasOneChildNoChoice() { return
				 childCount == 1 && 
				 !isChoice() && 
				 !hasNext(); }
  Bool hasNoChildren() { return ((childCount == 0 && !hasNext()) ? OK : NO); }
  int selectOrFailChildren(int l, int r);

  void dispose(void) {
    //  freeListDispose(children-1,(ToInt32(children[-1])+1)*sizeof(Board *));
    //  freeListDispose(this,sizeof(WaitActor));
  }

  Bool isAliveUpToSolve(void);
 
  void addChoice(WaitActor *wa) {
    cpb = cpb->add(wa);
  }
  void mergeChoices(CpBag *mcpb) {
    cpb = cpb->merge(mcpb);
  }
  CpBag * getCpb() {
    return cpb;
  }

};

// ------------------------------------------------------------------------

#endif
