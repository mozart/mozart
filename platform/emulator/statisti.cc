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

#include "tagged.hh"
#include "config.hh"
#include "statisti.hh"

#include "am.hh"

#include "fdomn.hh"


#ifndef OSF1_ALPHA
extern "C" void *sbrk(int incr);
#endif

Statistics ozstat;

static
void printTime(FILE *fd,char *s,unsigned int t)
{
  fprintf(fd,s);
  if (t < 1000) {
    fprintf(fd,"%u ms",t);
  } else {
    fprintf(fd,"%u.%u sec",t/1000,t%1000);
  }
}

static
void printMem(FILE *fd,char *s,double m)
{
  fprintf(fd,s);
  if (m < KB) {
    fprintf(fd,"%.0lf B",m);
    return;
  }
  if (m < MB) {
    fprintf(fd,"%.1lf kB",m/KB);
    return;
  }
  
  fprintf(fd,"%.1lf MB",m/MB);
}

void Statistics::print(FILE *fd)
{
  fprintf(fd,"\n******************************\n");
  fprintf(fd,"***  Oz System Statistics  ***\n");
  fprintf(fd,"******************************\n\n");

  unsigned total    = getUsedMemory()*KB;
  unsigned freeList = getMemoryInFreeList();
  unsigned occupied = total-freeList;

  fprintf(fd,  "  Memory areas:");
  printMem(fd, "\n    Code size is ", CodeArea::totalSize);
  printMem(fd, ".\n    Heap allocated is  ", getAllocatedMemory()*KB);
  printMem(fd, ".\n    Heap in freelist is ", freeList);
  printMem(fd, ".\n    Heap used is ", occupied);
  printMem(fd, ".\n    Hashtable for atoms is ",
	   CodeArea::atomTab.memRequired(sizeof(Literal)));
  printMem(fd, ".\n    Hashtable for names is ",
	   CodeArea::nameTab.memRequired(sizeof(Literal)));
  printMem(fd, ".\n    Hashtable for built-ins is ", builtinTab.memRequired());
  
  fprintf(fd, ".\n\n  Process resources consumed:");
  printTime(fd,"\n    User time is ", osUserTime());
  printTime(fd,".\n    System time is ", osSystemTime());
  printMem(fd,".\n    Size is ", ToInt32(sbrk(0))-mallocBase);
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

  int sum = allocateCounter+ procCounter+waitCounter+askCounter;
  fprintf(fd,"    Sum\t\t\t %d bytes ( = %d %%)\n",
	  sum, 	  (sum*100)/occupied );
#endif

  fprintf(fd,"  Search:\n");
  fprintf(fd,"    alternatives: %d\n",   solveAlt.total);
  fprintf(fd,"    clones:       %d\n",   solveClone.total);
  fprintf(fd,"    solutions:    %d\n",   solveSolved.total);
  fprintf(fd,"    failures:     %d\n\n", solveFailed.total);
  
  fprintf(fd,"******************************\n");
  fprintf(fd,"***   End of Statistics    ***\n");
  fprintf(fd,"******************************\n\n");
}

Statistics::Statistics()
{
  reset();
  timeUtime.total = osUserTime();
  timeUtime.idle();
}

void Statistics::reset()
{
#ifdef PROFILE
  allocateCounter = deallocateCounter = procCounter = waitCounter = 
    askCounter = protectedCounter = 0;
#endif

  gcCollected.reset();
  heapUsed.reset();
  timeForGC.reset();
  timeForCopy.reset();
  timeForLoading.reset();

  solveAlt.reset();
  solveClone.reset();
  solveSolved.reset();
  solveFailed.reset();
}


//----------------------------------------------------------------------


static void recSetArg(OZ_Term record, char *feat, unsigned int val)
{
  OZ_Term t = OZ_getRecordArg(record, OZ_CToAtom(feat));
  if (t == 0 || !OZ_unifyInt(t,val)) {
    OZ_warning("recSetArg(%s,%s,%d) failed",OZ_toC(record),feat,val);
  }
}


/*
 * fill two records of the form
 *
 *        statistics(r:_  g:_  l:_  c:_  h:_  s:_  u:_  e:_)
 *        enum(a:_  c:_  s:_  f:_)
 */

void Statistics::getStatistics(TaggedRef rec, TaggedRef enu)
{
  unsigned int timeNow = osUserTime();

  recSetArg(rec,"r",timeNow-(timeForGC.total+timeForLoading.total+timeForCopy.total));
  recSetArg(rec,"g",timeForGC.total);
  recSetArg(rec,"l",timeForLoading.total);
  recSetArg(rec,"c",timeForCopy.total);
  recSetArg(rec,"h",heapUsed.total+getUsedMemory());
  recSetArg(rec,"s",osSystemTime());
  recSetArg(rec,"u",timeNow);

  recSetArg(enu,"a",solveAlt.total);
  recSetArg(enu,"c",solveClone.total);
  recSetArg(enu,"s",solveSolved.total);
  recSetArg(enu,"f",solveFailed.total);
}


void Statistics::printIdle(FILE *fd)
{
  unsigned int timeNow = osUserTime();
  timeUtime.incf(timeNow-timeUtime.sinceIdle);
  int totalHeap = getUsedMemory()+heapUsed.total;
  
  if (ozconf.showIdleMessage) {
    fprintf(fd,"idle (");
    printTime(fd,"r: ",
	      timeUtime.sinceidle()-
	      (timeForGC.sinceidle()+timeForLoading.sinceidle()+timeForCopy.sinceidle()));
    printTime(fd,", c: ",timeForCopy.sinceidle());
    printTime(fd,", g: ",timeForGC.sinceidle());
    printTime(fd,", l: ",timeForLoading.sinceidle());
    printMem(fd,", h: ", (totalHeap-heapUsed.sinceIdle)*KB);
    fprintf(fd,")\n");
    fflush(fd);
  }
  heapUsed.sinceIdle = totalHeap;
  timeForGC.idle();
  timeForCopy.idle();
  timeForLoading.idle();
  timeUtime.idle();
}

void Statistics::initGcMsg(int level)
{
  if (level > 0) {
    printf("Heap garbage collection");
    fflush(stdout);
  }

  gcStarttime = osUserTime();
  gcStartmem  = getUsedMemory();
  heapUsed.incf(gcStartmem);
}

void Statistics::printGcMsg(int level)
{
  int gc_utime = osUserTime()-gcStarttime;
  int gc_mem   = gcStartmem-getUsedMemory();
 
  timeForGC.incf(gc_utime);
  gcCollected.incf(gc_mem);

  /* do not count amount of meory copied */
  heapUsed.incf(-getUsedMemory());
  
  if (level > 0) {
    printMem(stdout, " disposed ", gc_mem*KB);
    printf(" in %d msec.\n", gc_utime);

    if (level > 1) {
      printf("Statistics:");
      printMem(stdout, "\n\tMemory used was ", gcStartmem*KB);
      printMem(stdout, " and is now ", getUsedMemory()*KB);
      printMem(stdout, ".\nTotal garbage collected: ", gcCollected.total*KB);
      printf(".\n\n");
    }
    fflush(stdout);
  }
}
