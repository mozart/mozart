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
      /*
       * mm2: hack to fix a bug:
       *  Thread is in SEQMODE instead of ALLSEQMODE
       *  don't know why this happens ...
       */
      return -(oldTos-tos);
    }
    TaskStackEntry entry=*(--tos);
    ContFlag cFlag = getContFlag(ToInt32(entry));

    if (cFlag == C_COMP_MODE) {
      Assert(getCompMode(entry)==PARMODE);
      return oldTos-tos-1;
    }
    tos = tos - frameSize(cFlag) + 1;
  }
}

/*
 * copySeq: copy the sequential part of newStack
 *  NOTE: the tos of newStack points to the compMode task
 */
void TaskStack::copySeq(TaskStack *newStack,int len)
{
  TaskStackEntry *next=newStack->tos+1;

  /*
   * mm2: hack part II (see above)
   */
  if (len < 0) { len=-len; next--; }
  for (;len>0;len--) {
    push(*next++);
  }
}

/*
 * remove local tasks
 * return OK, if done
 * return NO, if no C_LOCAL/C_SOLVE found
 */
Bool TaskStack::discardLocalTasks()
{
  while (!isEmpty()) {
    TaskStackEntry entry=*(--tos);
    ContFlag cFlag = getContFlag(ToInt32(entry));

    switch (cFlag){
    case C_LOCAL:
    case C_SOLVE:
      return OK;
    default:
      tos = tos - frameSize(cFlag) + 1;
      break;
    }
  }
  return NO;
}

Chunk *TaskStack::findExceptionHandler()
{
  while (!isEmpty()) {
    ContFlag cFlag = getContFlag(ToInt32(*(tos-1)));

    if (cFlag == C_EXCEPT_HANDLER) {
      Chunk *ret = (Chunk*) *(tos-2);
      tos -=2 ;
      return ret;
    }
    tos = tos - frameSize(cFlag);
  }
  return NULL;
}


int frameSize(ContFlag cFlag)
{
  switch (cFlag){
  case C_COMP_MODE:
  case C_NERVOUS:
  case C_SOLVE:
  case C_LOCAL:
    return 1;
  case C_CONT:
    return 3;
  case C_XCONT:
    return 4;
  case C_DEBUG_CONT:
  case C_EXCEPT_HANDLER:
    return 2;
  case C_CALL_CONT:
    return 3;
  case C_CFUNC_CONT:
    return 4;

  default:
    Assert(0);
    return -1;
  }
}
