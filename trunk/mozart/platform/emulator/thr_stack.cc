/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(__GNUC__) && !defined(NOPRAGMA)
#pragma implementation "taskstk.hh"
#endif

#include "tagged.hh"
#include "stack.hh"

#include "taskstk.hh"

// #include "debug.hh"
// #include "codearea.hh"


void TaskStack::resize(int newsize)
{
  if (newsize < 20) {
    newsize = 50;
  }
  Stack::resize(newsize);
}

void TaskStack::deallocate(StackEntry *p, int n)
{
  freeListDispose(p, n*sizeof(StackEntry));
}

StackEntry *TaskStack::reallocate(StackEntry *p, int oldsize, int newsize) 
{
  TaskStackEntry *ret = (TaskStackEntry *)freeListMalloc(newsize*sizeof(TaskStackEntry));
  
  for (int i=0; i < oldsize; i++) {
    ret[i] = p[i];
  }
  
  deallocate(array,oldsize);
  return ret;
}


#ifdef DEBUG_CHECK
#include "constter.hh"
#include "board.hh"
void nodeCheckY(Board *n)
{
  if (isFreedRefsArray(n->getBodyPtr()->getY())) {
    error("Referencing freed environment");
  }
}

#endif

/*
 * getSeqSize:
 *   calculate the size of the sequential part of the taskstack
 *   and update the tos ("remove" all sequential tasks)
 * NOTE: the entry must not be destroyed, because copySeq needs them
 */
int TaskStack::getSeqSize()
{
  TaskStackEntry *oldTos=tos;

  while (1) {
    Assert(!isEmpty());
    TaskStackEntry entry=*(--tos);
    TaggedBoard tb = (TaggedBoard) ToInt32(entry);
    ContFlag cFlag = getContFlag(tb);
    if (cFlag == C_COMP_MODE) {
      Assert(getCompMode(entry)==PARMODE);
      break;
    }

    switch (cFlag){

    case C_NERVOUS:
      break;

    case C_COMP_MODE:
      Assert(0);
      break;

    case C_CONT:
      tos-=3; // PC Y G
      break;
      
    case C_XCONT:
      tos-=4; // PC Y G X
      break;

    case C_DEBUG_CONT: 
      tos--;
      break;

    case C_CALL_CONT: 
      tos-=2;
      break;

    case C_CFUNC_CONT:
      tos-=3;
      break;

    default:
      Assert(0);
      break;
    }
  } // while not task stack is empty

  return oldTos-tos-1;
} // TaskStack::getSeqSize

/*
 * copySeq: copy the sequential part of newStack
 *  NOTE: the tos of newStack points to the compMode task
 */
void TaskStack::copySeq(TaskStack *newStack,int len)
{
  TaskStackEntry *next=newStack->tos+1;
  for (;len>0;len--) {
    push(*next++);
  }
}
