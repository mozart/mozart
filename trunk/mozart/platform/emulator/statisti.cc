/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(__GNUC__) && !defined(NOPRAGMA)
#pragma implementation "statisti.hh"
#endif

#include <stdio.h>

#include "statisti.hh"
#include "am.hh"
#include "actor.hh"
#include "fdomn.hh"
#include "board.hh"
#include "builtins.hh"


inline
int percent(int i, int sum)
{
  return sum>0 ? i*100/sum : 0;
}

#ifndef OSF1_ALPHA
extern "C" void *sbrk(int incr);
#endif

static
void printTime(FILE *fd,char *s,unsigned int t) {
  fprintf(fd,s);
  if (t < 1000) {
    fprintf(fd,"%u ms",t);
  } else {
    fprintf(fd,"%u.%u sec",t/1000,(t/100)%100);
  }
}

static
void printMem(FILE *fd,char *s,double m) {
  fprintf(fd,s);
  if (m < KB) {
    fprintf(fd,"%.0lf B",m);
  } else if (m < MB) {
    fprintf(fd,"%.1lf kB",m/KB);
  } else {
    fprintf(fd,"%.1lf MB",m/MB);
  }
}

void Statistics::print(FILE *fd) {
  int sum;


  fprintf(fd,"\n******************************\n");
  fprintf(fd,"***  Oz System Statistics  ***\n");
  fprintf(fd,"******************************\n\n");

  unsigned total = getUsedMemory()*KB;
  unsigned freeList = getMemoryInFreeList();
  unsigned occupied = total-freeList;

  fprintf(fd,  "  Memory areas:");
  printMem(fd, "\n    Code size is ", CodeArea::totalSize);
  printMem(fd, ".\n    Heap allocated is  ", getAllocatedMemory()*KB);
  printMem(fd, ".\n    Heap in freelist is ", freeList);
  printMem(fd, ".\n    Heap used is ", occupied);
  printMem(fd, ".\n    Hashtable for atoms is ",
	   CodeArea::atomTab.memRequired());
  printMem(fd, ".\n    Hashtable for names is ",
	   CodeArea::nameTab.memRequired());
  printMem(fd, ".\n    Hashtable for built-ins is ",
	   builtinTab.memRequired());
  
  unsigned int utime    = usertime();
  unsigned int systime  = systemtime();

  fprintf(fd, ".\n\n  Process resources consumed:");
  printTime(fd,"\n    User time is ", utime);
  printTime(fd,".\n    System time is ", systime);
  printMem(fd,".\n    Size is ", ToInt32(sbrk(0)));
  fprintf(fd, ".\n\n");

#ifdef PROFILE

  fprintf(fd,"  Memory statistics:\n");
  fprintf(fd,"    Allocate uses \t\t %d bytes (%d%% of used heap)\n",
	  allocateCounter,
	  (allocateCounter*100)/occupied);
  fprintf(fd,"    Dellocate frees \t\t %d bytes (%d%%)\n",
	  deallocateCounter, (deallocateCounter*100)/allocateCounter);
  fprintf(fd,"    Process uses \t\t %d bytes\n",procCounter);
  fprintf(fd,"    Conds uses \t\t %d bytes\n",askCounter);
  fprintf(fd,"    Disj  uses \t\t %d bytes\n",waitCounter);
  fprintf(fd,"    Variables in envs use \t\t %d bytes (%d%% of used heap)\n",
	  localVariableCounter,
	  (localVariableCounter*100)/occupied);
  fprintf(fd,"    There were %d protected entries into the heap\n",
	  protectedCounter);

  sum = allocateCounter+ procCounter+waitCounter+askCounter;
  fprintf(fd,"    Sum\t\t\t %d bytes ( = %d %%)\n",
	  sum, 	  (sum*100)/occupied );

#endif

#ifdef MM2
  sum = wakeUpNode+wakeUpCont+wakeUpBI;
  fprintf(fd,"  Number of WakeUps: %d\n",sum);
  fprintf(fd,"    Normal: %d=%d%%\n",
	  wakeUpNode,percent(wakeUpNode,sum));
  fprintf(fd,"    Continuations: %d=%d%%\n",
	  wakeUpCont,percent(wakeUpCont,sum));
  fprintf(fd,"      Opt: %d=%d%%\n",
	  wakeUpContOpt,percent(wakeUpContOpt,sum));
  fprintf(fd,"    Builtins: %d=%d%%\n\n",
	  wakeUpBI,percent(wakeUpBI,sum));
#endif

  // solve stuff
  fprintf(fd,"  Solve:\n");
  fprintf(fd,"    distributions: %d\n", solveDistributed);
  fprintf(fd,"    solutions:     %d\n", solveSolved);
  fprintf(fd,"    failures:      %d\n\n", solveFailed);
  
  fprintf(fd,"******************************\n");
  fprintf(fd,"***   End of Statistics    ***\n");
  fprintf(fd,"******************************\n\n");
}

Statistics::Statistics()
{
  reset();

  heapAllocated = 0.0;
  sumHeap = 0.0;
  timeForGC = 0;
  timeForCopy = 0;
  timeForLoading = 0;

  sumTimeForGC = 0;
  sumTimeForCopy = 0;
  sumTimeForLoading = 0;

  timeSinceLastIdle = usertime();
}

void Statistics::reset() {
#ifdef PROFILE
  allocateCounter = 0;
  deallocateCounter = 0;
  procCounter = 0;
  waitCounter = 0;
  askCounter = 0;
  protectedCounter = 0;
#endif
  wakeUpNode = 0;
  wakeUpCont = 0;
  wakeUpContOpt = 0;
  wakeUpBI = 0;

  solveDistributed = 0;
  solveSolved = 0;
  solveFailed = 0;
}


void Statistics::printIdle(FILE *fd)
{
  unsigned int timeNow = usertime();
  unsigned int mem = getUsedMemory()*KB;
  heapAllocated += mem;

  if (am.conf.showIdleMessage) {
    printf("idle (", timeNow-timeSinceLastIdle);
    printTime(fd,"r: ",
	      (timeNow-timeSinceLastIdle)-timeForGC
	      -timeForCopy-timeForLoading);
    printTime(fd,", c: ",timeForCopy);
    printTime(fd,", g: ",timeForGC);
    printTime(fd,", l: ",timeForLoading);
    printMem(fd,", h: ",heapAllocated);
    fprintf(fd,")\n");
    fflush(fd);
  }
  sumTimeForGC += timeForGC;
  sumTimeForCopy += timeForCopy;
  sumTimeForLoading += timeForLoading;
  timeForGC = timeForCopy = timeForLoading = 0;
  heapAllocated = - (int) mem;
  timeSinceLastIdle = usertime();
}

int Statistics::Statistics_gcSoFar = 0;

void Statistics::initGcMsg(int level)
{
  if (level > 0) printf("Heap garbage collection");

  gc_level = level;
  gc_utime = usertime();
  gc_usedMem = getUsedMemory()*KB;
  gc_allocMem = getAllocatedMemory()*KB;
}

void Statistics::printGcMsg(void)
{
// print final message
  Statistics_gcSoFar += (gc_usedMem/KB - getUsedMemory());
  gc_utime = usertime() - gc_utime;
  timeForGC += gc_utime;
  heapAllocated += (gc_usedMem - getUsedMemory()*KB);
  sumHeap += (gc_usedMem - getUsedMemory()*KB);
  if (gc_level > 0) {
    printMem(stdout, " disposed ", gc_usedMem - getUsedMemory()*KB);
    printf(" in %d msec.\n", gc_utime);

    if (gc_level > 1) {
      printf("Statistics:");
      printMem(stdout, "\n\tMemory used was ", gc_usedMem);
      printMem(stdout, " and is now ", getUsedMemory()*KB);
      printMem(stdout, ".\n\tMemory allocated was ", gc_allocMem );
      printMem(stdout, " and is now ", getAllocatedMemory()*KB);
      printMem(stdout, ".\nTotal garbage collected: ",
	       ((double) Statistics_gcSoFar)*KB);
      printf(".\n\n");
    }
    fflush(stdout);
  }
}
