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

    switch (cFlag){
    case C_COMP_MODE:
      Assert(getCompMode(entry)==PARMODE);
      return oldTos-tos-1;

    case C_NERVOUS: break;
    case C_SOLVE:   break;
    case C_LOCAL:   break;

    case C_CONT:
      tos-=2; // PC Y G
      break;
      
    case C_XCONT:
      tos-=3; // PC Y G X
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
} // TaskStack::getSeqSize

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
  TaskStackEntry *oldTos=tos;

  while (!isEmpty()) {
    TaskStackEntry entry=*(--tos);
    ContFlag cFlag = getContFlag(ToInt32(entry));

    switch (cFlag){
    case C_COMP_MODE: break;
    case C_NERVOUS:   break;
    case C_LOCAL:     return OK;
    case C_SOLVE:     return OK;

    case C_CONT:
      tos-=2; // Y G
      break;
      
    case C_XCONT:
      tos-=3; // Y G X
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
  return NO;
}
