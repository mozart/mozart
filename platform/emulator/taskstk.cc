/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------

  exported variables/classes: process task stack class

  exported procedures:

  ------------------------------------------------------------------------

  internal static variables:

  internal procedures:

  ------------------------------------------------------------------------

*/

#if defined(__GNUC__) && !defined(NOPRAGMA)
#pragma implementation "taskstk.hh"
#endif

#include "taskstk.hh"

#include "debug.hh"
#include "codearea.hh"


void TaskStack::resize(int newsize)
{
  if (newsize < 100) {
    newsize = 200;
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

void TaskStack::printDebug(Bool verbose, int depth)
{
  if (this == NULL) {
    printf("TaskStack empty.\n");
    return;
  }

  TaskStackEntry *p = getTop();

  while (isEmpty() == NO && depth-- > 0) {
    TaggedBoard tb = (TaggedBoard) pop();
    ContFlag flag = getContFlag(tb);
    Board* n = getBoard(tb,flag);
    switch (flag){
    case C_CONT:
      {
        ProgramCounter PC = (ProgramCounter) pop();
        RefsArray Y = (RefsArray) pop();
        RefsArray G = (RefsArray) pop();
        if (!verbose) break;
        printf("\tC_CONT: board=0x%x, PC=0x%x, Y=0x%x, G=0x%x\n\t",
               n, PC, Y, G);
        CodeArea::display(CodeArea::definitionStart(PC),1,stdout);
      }
      break;
    case C_XCONT:
      {
        ProgramCounter PC = (ProgramCounter) pop();
        RefsArray Y = (RefsArray) pop();
        RefsArray G = (RefsArray) pop();
        RefsArray X = (RefsArray) pop();
        if (!verbose) break;
        printf("\tC_XCONT: board=0x%x, PC=0x%x, Y=0x%x, G=0x%x\n",
               n, PC, Y, G);
        int XSize = getRefsArraySize(X);
        while (--XSize >= 0) {
          printf("\t\tX[%d]=0x%x\n", XSize, X[XSize]);
        }
        break;
      }
    case C_NERVOUS:
      if (!verbose) break;
      printf("\tC_NERVOUS: board=0x%x\n", n);
      break;
    case C_CFUNC_CONT:
      {
        OZ_CFun biFun = (OZ_CFun) pop();
        Suspension* susp = (Suspension*) pop();
        RefsArray X = (RefsArray) pop();
        printf("\tC_CFUNC_CONT: board=0x%x, biFun=0x%x, susp=0x%x\n",
               n , biFun, susp);
        if (!verbose) break;
        if (X != NULL) {
          int XSize = getRefsArraySize(X);
          while (--XSize >= 0) {
            printf("\t\tX[%d]=0x%x\n", XSize, X[XSize]);
          }
        } else {
          printf("\t\tNo arguments.\n");
        }
        break;
      }

    case C_DEBUG_CONT:
      {
        OzDebug *deb = (OzDebug*) pop();
        deb->printCall();
        break;
      }

    case C_CALL_CONT:
      {
        SRecord *s = (SRecord *) pop();
        RefsArray X = (RefsArray) pop();
        if (!verbose) break;
        printf("\tC_DEBUG_CONT: board=0x%x, Pred=0x%x, X=0x%x\n", n, s, X);
        break;
      }

    default:
      error("printDebug: unexpected task found.");
    } // switch
  } // while

  setTop(p);
}

#ifdef DEBUG_CHECK
#include "board.hh"
void nodeCheckY(Board *n)
{
  if (isFreedRefsArray(n->getBodyPtr()->getY())) {
    error("Referencing freed environment");
  }
}

#endif
