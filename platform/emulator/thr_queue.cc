/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "thrqueue.hh"
#endif

#include "tagged.hh"
#include "value.hh"
#include "cont.hh"
#include "actor.hh"
#include "board.hh"
#include "stack.hh"
#include "taskstk.hh"
#include "am.hh"
#include "thread.hh"

#include "thrqueue.hh"

void ThreadQueueImpl::resize ()
{
  int new_maxsize = maxsize * 2;
  Thread ** new_queue = ::new Thread*[new_maxsize];
  int index = 0;
  int currentSize = size;
  int currentHead = head;
  int mod = maxsize - 1;

  DebugCode(message("Resizing thread queue 0x%x --> 0x%x.\n",
                    maxsize, new_maxsize));
  while (currentSize) {
    new_queue[index++] = queue[currentHead];
    currentHead = (currentHead + 1) & mod;
    currentSize--;
  }

  delete queue;
  queue = new_queue;
  head = 0;
  tail = size - 1;
  maxsize = new_maxsize;
}

Bool ThreadQueue::isScheduled (Thread *thr)
{
  int currentSize = size;
  int currentHead = head;
  int mod = maxsize - 1;

  while (currentSize) {
    if (queue[currentHead] == thr) return (OK);
    currentHead = (currentHead + 1) & mod;
    currentSize--;
  }

  return (NO);
}

void LocalThreadQueue::resize ()
{
  int new_maxsize = maxsize * 2;
  Thread ** new_queue =
    (Thread **) heapMalloc ((size_t) (sizeof(Thread *) * new_maxsize));
  int index = 0;
  int currentSize = size;
  int currentHead = head;
  int mod = maxsize - 1;

  DebugCode(message("Resizing thread queue 0x%x --> 0x%x.\n",
                    maxsize, new_maxsize));
  while (currentSize) {
    new_queue[index++] = queue[currentHead];
    currentHead = (currentHead + 1) & mod;
    currentSize--;
  }

  queue = new_queue;
  head = 0;
  tail = size - 1;
  maxsize = new_maxsize;
}

Board * ThreadQueueImpl::getHighestSolveDebug(void)
{
  int asize = size;
  int ahead = head;
  int mod = maxsize - 1;

  while (asize) {
    Board * c = queue[ahead]->getBoardFast()->getHighestSolveDebug();
    if (c)
      return c;
    ahead = (ahead + 1) & mod;
    asize--;
  }

  return NULL;
}
