/*
 *  Authors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Denys Duchier, 1998
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

// This file implements a template for generic queues of non-null
// pointers.  Each queue consists of a linked list of blocks.  Each
// contains an array of pointers and knows the size of this array.
// Blocks in a queue are not necessarily all the same size: for
// example because different kinds of queues may be merged, e.g.
// propagator queues and var suspension lists.  A queue may have
// holes represented by null pointers: operations on queues will
// skip these holes.  Holes may be created by removing an element,
// which is realized by setting to 0 the location where it appears
// in the queue.

// class LinkedQueueBlock
//	the generic representation of a block of pointers
// class LinkedQueueImpl
//	the generic representation of a queue
// class LinkedQueue<T,SIZE>
//	queue of T* with blocks of size=SIZE by default
// class LinkedQueueIteratorImpl
//	generic iterator on generic queues
// class LinkedQueueIterator<T,SIZE>
//	iterator on LinkedQueue<T,SIZE>

// class LinkedQueue<T,SIZE>
//	void enqueue(T*,int n = SIZE)
//		enqueue an element, optionally supply the size of
//		blocks for growing the queue
//	T* dequeue()
//	int isEmpty()
//	int getSize()
//	T** find(T*)
//		return a pointer to location of the first occurrence
//		of a given pointer in the queue, or 0 if not found
//	void remove(T*)
//		delete the first occurrence of a given pointer in the
//		queue; it becomes a hole.
//	LinkedQueue<T,SIZE>* merge(LinkedQueue<T,SIZE>*)
//		merge a given queue into the current one.  This is a
//		destructive operation.

// class LinkedQueueIterator<T,SIZE>
//	LinkedQueueIterator()
//	LinkedQueueIterator(LinkedQueue<T,SIZE>*)
//	LinkedQueueIterator(LinkedQueue<T,SIZE>&)
//	T* getNext()
//		returns the next queue element or 0 when no more
//	T** getPointerToNext()
//		returns a pointer to the next element or 0 when no more
//	void reset((LinkedQueue<T,SIZE>*)
//	void reset((LinkedQueue<T,SIZE>&)
//		starts iterating on the given queue instead
//
// Typical usage pattern:
//
// LinkedQueueIterator<T,SIZE> iter(q);
// T* ptr;
// while ((ptr=iter.getNext())) { ... }

#ifndef __LQUEUE_HH
#define __LQUEUE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include <string.h>
#include "mem.hh"

#ifndef LQ_TRACE
#define LQ_TRACE(X)
#endif

// the size of a block's array must be a multiple of 2:
// size = 2 * n		for any n in [1..FreeListSize] = [1..512]

class LinkedQueueBlock {
  friend class LinkedQueueImpl;
  friend class LinkedQueueIteratorImpl;

  // free list for all 2n array sizes up to n=512

public:
  static const int FreeListSize = 512;
private:
  static LinkedQueueBlock* freelist[FreeListSize];

  // representation of a block
  // it is made public because I cannot figure out a reasonable
  // way to make LinkedQueue<T,SIZE> a friend

private:
  int size;		// each block knows the size of its array
  LinkedQueueBlock*next;
  void* array[1];

public:
  // obtain a block with an array of SIZE elements
  static inline LinkedQueueBlock* allocate(int SIZE) {
    LQ_TRACE(cerr << "allocate " << SIZE);
    Assert(SIZE >= 2 && SIZE <= 2*FreeListSize && !(SIZE&1));
    register int i = SIZE>>1;
    LinkedQueueBlock* block = freelist[i];
    if (block) {
      LQ_TRACE(cerr << " from free list" << endl);
      freelist[i] = block->next;
    } else {
      LQ_TRACE(cerr << " from heap" << endl);
      block = (LinkedQueueBlock*)
	// allocate a block with an array of SIZE elements
	oz_heapMalloc(sizeof(LinkedQueueBlock)+(SIZE-1)*sizeof(void*));
      block->size = SIZE;
    }
    // set all pointers to 0
    block->next = 0;
    memset(block->array,0,SIZE*sizeof(void*));
    return block;
  }
  void dispose() {
    LQ_TRACE(cerr << "dispose " << size << " to free list" << endl);
    // return block to free list
    register int i = size>>1;
    next = freelist[i];
    freelist[i] = this;
  }
  static void initFreeList() {
    LQ_TRACE(cerr << "init free list" << endl);
    // to be called for system initialization and
    memset(freelist,0,FreeListSize*sizeof(LinkedQueueBlock*));
  }
};

class LinkedQueueImpl {
  friend class LinkedQueueIteratorImpl;
protected:
  LinkedQueueBlock * head;
  LinkedQueueBlock * tail;
  int head_index;
  int tail_index;
  int size;

  // merge another queue into this one
  // this operation is unsafe because we don't know if the
  // queues contains pointers of the same type
  // derived classes should define type safe merge methods
  // that are wrappers around mergeUnsafe
  LinkedQueueImpl * mergeUnsafe(LinkedQueueImpl * q);

public:
  LinkedQueueImpl()
    :head(0),tail(0),head_index(0),tail_index(0),size(0){}

  int isEmpty() { return size==0; }
  int getSize() { return size; }
  void zeroAll() {
    head=tail=0;
    head_index=tail_index=size=0;
  }

protected:
  // this should be inlined
  void grow_head(int SIZE) {
    LinkedQueueBlock * block =
      LinkedQueueBlock::allocate(SIZE);
    if (head) {
      head->next = block;
      head       = block;
      head_index = SIZE;
    }
    else {
      // no blocks yet
      head=tail=block;
      head_index=tail_index=SIZE;
    }
  }

  void enqueueInternal(void* x,int SIZE) {
    LQ_TRACE(cerr << "enqueue" << endl);
    if (head_index==0) grow_head(SIZE);
    head->array[--head_index]=x;
    size++;
  }

  void drop_tail() {
    LinkedQueueBlock * block = tail;
    // next block is not necessarily of size=SIZE
    tail       = block->next;
    tail_index = tail->size;
    block->dispose();
  }

  void* dequeueInternal() {
    LQ_TRACE(cerr << "dequeue" << endl);
    Assert(size>0);
    // skips null entries, i.e. holes
    register void* x;
    do {
      if (tail_index==0) drop_tail();
      x = tail->array[--tail_index];
    } while (x==0);
    size--;
    return x;
  }

public:
  // return a queue's linked list of blocks to the free list
  void dispose() {
    LinkedQueueBlock * block;
    while (tail) {
      block = tail;
      tail  = tail->next;
      block->dispose();
    }
    zeroAll();
  }

  // return the location of a particular element pointer in the
  // queue or 0 if not found
  void** find(void*);

  // remove 1st occurrence of a particular element pointer
  void remove(void*);

};

template <class T,const int SIZE> 
class LinkedQueue : public LinkedQueueImpl {
public:
  LinkedQueue():LinkedQueueImpl(){}

  void enqueue(T* x,int n = SIZE) { enqueueInternal(x,SIZE); }
  T* dequeue() { return (T*) dequeueInternal(); }

  T** find(T* x) { return (T**) LinkedQueueImpl::find(x); }
  void remove(T* x) { LinkedQueueImpl::remove(x); }

  // one possible type-safe merge method
  LinkedQueue<T,SIZE>* merge(LinkedQueue<T,SIZE>* q) {
    return (LinkedQueue<T,SIZE>*) mergeUnsafe(q);
  }
};

class LinkedQueueIteratorImpl {
private:
  LinkedQueueBlock * block;
  int size;
  int index;
public:
  LinkedQueueIteratorImpl()
    :block(0),size(0),index(0){}
  LinkedQueueIteratorImpl(LinkedQueueImpl*q)
    :block(q->tail),size(q->size),index(q->tail_index){}
  LinkedQueueIteratorImpl(LinkedQueueImpl&q)
    :block(q.tail),size(q.size),index(q.tail_index){}

  // reset on a new queue

  void reset(LinkedQueueImpl*q) {
    block = q->tail;
    size  = q->size;
    index = q->tail_index;
  }

  void reset(LinkedQueueImpl&q) {
    block = q.tail;
    size  = q.size;
    index = q.tail_index;
  }

  // return next queue element or 0 if none

  void* getNext() {
    register void* x;
    if (size==0) return 0;
    do
      {
	if (index==0) {
	  block = block->next;
	  index = block->size;
	}
	x = block->array[--index];
      }
    while (x==0);
    size--;
    return x;
  }

  // return pointer to next queue element or 0 if none

  void* * getPointerToNext() {
    register void* * x;
    if (size==0) return 0;
    do
      {
	if (index==0) {
	  block = block->next;
	  index = block->size;
	}
	x = & block->array[--index];
      }
    while (*x==0);
    size--;
    return x;
  }
};

template <class T,const int SIZE>
class LinkedQueueIterator : public LinkedQueueIteratorImpl
{
public:
  LinkedQueueIterator()
    : LinkedQueueIteratorImpl( ){}
  LinkedQueueIterator(LinkedQueue<T,SIZE>*q)
    : LinkedQueueIteratorImpl(q){}
  LinkedQueueIterator(LinkedQueue<T,SIZE>&q)
    : LinkedQueueIteratorImpl(q){}
  T* getNext() {
    return (T* ) LinkedQueueIteratorImpl::getNext();
  }
  T** getPointerToNext() {
    return (T**) LinkedQueueIteratorImpl::getPointerToNext();
  }
};

#endif /* __LQUEUE_HH */
