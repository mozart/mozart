#ifndef __BRANCH_QUEUE_HH__
#define __BRANCH_QUEUE_HH__

#include "base.hh"
#include "mem.hh"

/**
	\file Queue implementation for branching descriptions allocated
	using freelist memory.
*/
class BranchQueue {
private:
  class node {
  public:
    USEFREELISTMEMORY;
    node *next;
    TaggedRef branch;
    node(void) { next=NULL; }
    node(TaggedRef e) { next=NULL; branch=e;}
    void setBranch(TaggedRef e) { branch = e; }
    TaggedRef getBranch(void) { return branch; }
  };
  node *first;
  node *last;
public:
  USEHEAPMEMORY;
  BranchQueue(void) : first(NULL), last(NULL) {}
  void enqueue(TaggedRef);
  TaggedRef dequeue(void);
  bool isEmpty(void);
  BranchQueue *gCollect(void);
  BranchQueue *sClone(void);
  void print(void);
};

#include "branch_queue.icc"

#endif
