/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "taskstk.hh"
#endif

#include "tagged.hh"
#include "stack.hh"

#include "cont.hh"
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
    if (isEmpty()) {
      tos=oldTos;
      return -1;
    }
    TaskStackEntry entry=*(--tos);
    ContFlag cFlag = getContFlag(ToInt32(entry));

    if (cFlag == C_JOB) {
      return oldTos-tos-1;
    }
    if (cFlag == C_SET_CUROBJECT) {
      am.setCurrentObject((Object*) *(tos-1));
    }
    Assert(cFlag != C_LOCAL);
    tos = tos - frameSize(cFlag) + 1;
  }
}

#ifdef DEBUG_CHECK
int TaskStack::hasJobDebug ()
{
  TaskStackEntry *oldTos=tos;

  while (1) {
    if (isEmpty()) {
      tos=oldTos;
      return NO;
    }
    TaskStackEntry entry=*(--tos);
    ContFlag cFlag = getContFlag(ToInt32(entry));

    if (cFlag == C_JOB) {
      tos=oldTos;
      return OK;
    }
    tos = tos - frameSize(cFlag) + 1;
  }
}
#endif

/*
 * copySeq: copy the sequential part of newStack
 *  NOTE: the tos of newStack points to the JOB task
 */
void TaskStack::copySeq(TaskStack *newStack,int len)
{
  TaskStackEntry *next=newStack->tos+1;

  for (;len>0;len--) {
    push(*next++);
  }
}

int TaskStack::frameSize(ContFlag cFlag)
{
  switch (cFlag){
  case C_JOB:
  case C_LOCAL:
  case C_SET_MODETOP:
    return 1;
  case C_CONT:
    return 3;
  case C_XCONT:
    return 4;
  case C_DEBUG_CONT:
  case C_EXCEPT_HANDLER:
  case C_SET_CAA:
  case C_SET_CUROBJECT:
    return 2;
  case C_CALL_CONT:
    return 3;
  case C_CFUNC_CONT:
    return 3;

  default:
    Assert(0);
    return -1;
  }
}
