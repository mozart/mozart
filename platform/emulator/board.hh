/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __BOARDH
#define __BOARDH

#ifdef INTERFACE
#pragma interface
#endif

#include "distributor.hh"
#include "thr_class.hh"
#include "pointer-marks.hh"

#define GETBOARD(v) ((v)->getBoardInternal()->derefBoard())
#define GETBOARDOBJ(v) ((v).getBoardInternal()->derefBoard())

struct Equation {
friend class Script;
private:
  TaggedRef left;
  TaggedRef right;
public:
  void setLeft(TaggedRef *l) { left = makeTaggedRef(l); }
  void setRight(TaggedRef r) { right = r; }
  TaggedRef getLeft() { return left; }
  TaggedRef getRight() { return right; }
  OZPRINT;
};

class Script {
public:
  void gc();

  Script() { numbOfCons = 0; first = (Equation *)NULL; }
  Script(int sizeInit);
  ~Script();
  OZPRINT;
  void allocate(int sizeInit);
  void dealloc();

  int getSize() { return numbOfCons; }
  Equation* getRef()  { return (first); }

  Equation &operator[] (int elem)  { return ( *(first + elem) ); }
  /* no bounds checking;    */

private:
  int numbOfCons;
  Equation* first;
};


class Board {
friend int engine(Bool init);
public:
  NO_DEFAULT_CONSTRUCTORS(Board);
  Board(Board *b);

  USEHEAPMEMORY;

  //
  // Home and parent access
  //
private:
  void * parent;

public:  
  int isCommitted(void) {
    return IsMarkedPointer(parent,1);
  }
  void setCommitted(void) {
    parent = MarkPointer(parent,1);
  }
  int isFailed(void) {
    return IsMarkedPointer(parent,2);
  }
  void setFailed(void) {
    parent = MarkPointer(parent,2);
  }
  Board * getParentInternal(void) {
    return (Board *) UnMarkPointer(parent,3);
  }
  void setParentInternal(Board * p) {
    parent = (void *) p;
  }
  Bool isRoot() { 
    Assert(UnMarkPointer(parent,3) || (!isCommitted() && !isFailed()));
    return !parent;               
  }
  Board *derefBoard() {
    Board *bb;
    for (bb=this; bb->isCommitted(); bb=bb->getParentInternal()) {}
    return bb;
  }
  Board *getParent() {
    Assert(!isCommitted());
    return getParentInternal()->derefBoard();
  }
  
  //
  // Garbage collection and copying
  //
public:
  Bool gcIsMarked(void);
  void gcMark(Board *);
  Board * gcGetFwd(void);
  Board *gcBoard();
  void gcRecurse(void);
  Bool gcIsAlive();
  Board *gcGetNotificationBoard ();

  void unsetGlobalMarks(void);
  void setGlobalMarks(void);
  Board * clone(void);
  
  //
  // Suspension counter
  //
private:
  int suspCount;

public:  
  void incSuspCount(int n=1) {
    Assert(!isCommitted() && !isFailed());
    suspCount += n;
    Assert(suspCount >= 0);
  }
  void decSuspCount(void) {
    Assert(!isCommitted() && !isFailed());
    Assert(suspCount > 0);
    suspCount--;
  }
  int getSuspCount(void) {
    Assert(!isFailed() && suspCount >= 0);
    return suspCount;
  }
  Bool isStable(void);
  Bool isBlocked(void);
  void incSolveThreads(void);
  void decSolveThreads(void);
  void checkSolveThreads(void);
  void checkStability(void);
  
  //
  // Thread counter
  //
private:
  int threads;

public:
  void incThreads(int n = 1) { 
    threads += n; 
  }
  int decThreads()  { 
    Assert (threads > 0); 
    return (--threads); 
  }
  int getThreads(void) { 
    return threads; 
  }

  // Script
  //
private:
  Script script;

public:
  Script &getScriptRef() { return script; }
  void newScript(int size) {
    script.allocate(size);
  }

  void setScript(int i,TaggedRef *v,TaggedRef r) {
    script[i].setLeft(v);
    script[i].setRight(r);
  }

  //
  // Suspension list
  //
private:
  SuspList  *suspList;

public:
  void addSuspension(Suspension); 
  SuspList * getSuspList(void) { 
    return suspList; 
  }
  void setSuspList(SuspList *sl) { 
    suspList=sl; 
  }
  Bool isEmptySuspList() { 
    return suspList==0; 
  }
  void clearSuspList(Suspension);

  //
  // local thread queue
  //

private:
  LocalPropagatorQueue * localPropagatorQueue;

public:
  LocalPropagatorQueue * getLocalPropagatorQueue(void) {
    return localPropagatorQueue;
  }
  void setLocalPropagatorQueue(LocalPropagatorQueue * lpq) {
    localPropagatorQueue = lpq;
  }
  void resetLocalPropagatorQueue(void);

  //
  // nonmonotonic propagators
  //

private:
  OrderedSuspList * nonMonoSuspList;

public:
  void setNonMono(OrderedSuspList * l) { 
    nonMonoSuspList = l; 
  }
  OrderedSuspList *getNonMono() { 
    return nonMonoSuspList; 
  }
  void addToNonMono(Propagator *);
  void scheduleNonMono(void);

  //
  // distributors
  //
private:
  void * bag;

  //
  // Installation and global marks
  //
public:
  int isMarkedAsInstalled(void) {
    return IsMarkedPointer(bag,1);
  }
  void markAsInstalled(void) {  
    bag = MarkPointer(bag,1);
  }
  void unMarkAsInstalled() { 
    bag = UnMarkPointer(bag,1);
  }
  int isMarkedGlobal(void) {
    return IsMarkedPointer(bag,1);
  }
  void setGlobalMark(void) {  
    bag = MarkPointer(bag,1);
  }
  void unsetGlobalMark() { 
    bag = UnMarkPointer(bag,1);
  }

public:
  DistBag * getDistBag(void) {
    Assert(!isMarkedAsInstalled() && !isMarkedGlobal());
    return (DistBag *) bag;
  }
  void setDistBag(DistBag * db) {
    Assert(!isMarkedAsInstalled() && !isMarkedGlobal());
    bag = (void *) db;
  }
  void addToDistBag(Distributor * d);
  Distributor * getDistributor(void);
  

  //
  // Operations
  //
  int commit(int, int);
  void inject(TaggedRef);
  TaggedRef merge(Board *, Bool);

  //
  // Status variable
  //
private:
  TaggedRef status;
  
public:
  TaggedRef getStatus() { 
    return status; 
  }
  void setStatus(TaggedRef v) { 
    status = v; 
  }

  void clearStatus();
  void patchAltStatus(int i);

  TaggedRef genSucceeded(Bool);
  TaggedRef genAlt(int);
  TaggedRef genFailed();
  TaggedRef genBlocked(TaggedRef arg);


  //
  // Root variable
  //
private:
  TaggedRef rootVar;

public:
  TaggedRef getRootVar() {
    return makeTaggedRef(&rootVar);
  }

  //
  // Misc
  //
  
  OZPRINTLONG;

#ifdef DEBUG_CHECK
  void printTree();
#endif

  //
  // Trailing comparison
  //

#ifdef CS_PROFILE
public:
  int32 * orig_start;
  int32 * copy_start;
  int     copy_size;

  TaggedRef getCloneDiff(void);
#endif


};

void oz_checkExtSuspension(Suspension, Board *);

#define CheckExtSuspension(susp)               \
  if (((Suspension)susp).wasExtSuspension()) { \
    GETBOARDOBJ((Suspension) susp)->checkSolveThreads();   \
  }

#ifndef OUTLINE
#include "board.icc"
#endif

#endif
