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
#include "term.hh"
#include "constter.hh"
#include "actor.hh"
#include "board.hh"
#include "stack.hh"
#include "taskstk.hh"
#include "am.hh"
#include "thread.hh"

#include "thrqueue.hh"

void ThreadQueue::resize () 
{
  int new_maxsize = maxsize * 2;
  ThreadPtr *new_queue = ::new ThreadPtr[new_maxsize];
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

