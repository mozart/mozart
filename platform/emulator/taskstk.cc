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

#include "taskstk.hh"

#include "debug.hh"
#include "codearea.hh"


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


static
void printX(FILE *fd, RefsArray X)
{
  int xsize = getRefsArraySize(X);
  while (--xsize >= 0) {
    fprintf(fd,"\t\tX[%d]=0x%x\n", xsize, X[xsize]);
  }
}

static
void printDef(ProgramCounter PC)
{
  Reg reg;
  ProgramCounter next;
  TaggedRef file, line;
  PrTabEntry *pred;

  ProgramCounter pc = CodeArea::definitionStart(PC);
  if (pc == NOCODE) {
    message("\tOn toplevel\n");
    return;
  }

  CodeArea::getDefinitionArgs(pc,reg,next,file,line,pred);

  message("\tIn procedure %s (File %s, line %s)\n",
          pred ? pred->getPrintName() : "???",
          tagged2String(file,10),tagged2String(line,10));
}


void TaskStack::printDebug(ProgramCounter pc, Bool verbose, int depth)
{
  if (this == NULL) {
    message("TaskStack empty.\n");
    return;
  }

  message("\n");
  message("Stack dump:\n");
  message("-----------\n");

  if (pc != NOCODE && pc !=NULL) {
    printDef(pc);
  }
  TaskStackEntry *p = getTop();

  while (isEmpty() == NO && depth-- > 0) {
    TaggedBoard tb = (TaggedBoard) ToInt32(pop());
    ContFlag flag = getContFlag(tb);
    Board* n = getBoard(tb,flag);
    switch (flag){
    case C_CONT:
      {
        ProgramCounter PC = (ProgramCounter) pop();
        RefsArray Y = (RefsArray) pop();
        RefsArray G = (RefsArray) pop();
        if (verbose) {
          message("\tC_CONT: board=0x%x, PC=0x%x, Y=0x%x, G=0x%x\n\t",
                 n, PC, Y, G);
        }
        printDef(PC);
      }
      break;
    case C_XCONT:
      {
        ProgramCounter PC = (ProgramCounter) pop();
        RefsArray Y = (RefsArray) pop();
        RefsArray G = (RefsArray) pop();
        RefsArray X = (RefsArray) pop();
        if (verbose) {
          message("\tC_XCONT: board=0x%x, PC=0x%x, Y=0x%x, G=0x%x\n",
                 n, PC, Y, G);
          printX(stdout,X);
        }
        printDef(PC);
        break;
      }
    case C_NERVOUS:
      if (!verbose) break;
      message("\tC_NERVOUS: board=0x%x\n", n);
      break;
    case C_COMP_MODE:
      message("\t%sMODE\n",((int) tb)>>4==SEQMODE?"SEQ":"PAR");
      break;
    case C_CFUNC_CONT:
      {
        OZ_CFun biFun    = (OZ_CFun) pop();
        Suspension* susp = (Suspension*) pop();
        RefsArray X      = (RefsArray) pop();
        message("\tC_CFUNC_CONT: board=0x%x, biFun=0x%x, susp=0x%x\n",
               n , biFun, susp);
        if (X != NULL) {
          printX(stdout,X);
        } else {
          message("\t\tNo arguments.\n");
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
        message("\tC_DEBUG_CONT: board=0x%x, Pred=0x%x\n", n, s);
        printX(stdout,X);
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
