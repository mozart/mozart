/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "statisti.hh"
#endif

#include <stdio.h>

#include "tagged.hh"
#include "config.hh"
#include "statisti.hh"

#include "am.hh"

#include "fdomn.hh"


#if !defined(OSF1_ALPHA) && !defined(WINDOWS) && !defined(FREEBSD) && !defined(NETBSD)
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
    fprintf(fd,"%u.%03u sec",t/1000,t%1000);
  }
}

static
void printPercent(FILE *fd,char *s,unsigned int t,unsigned int total)
{
  fprintf(fd,s);
  if (total == 0) {
    fprintf(fd,"0");
  } else {
    unsigned int rel = (t*100)/total;
    fprintf(fd,"%u",rel);
  }
}

int workaroundForBugInGCC1 = KB;
int workaroundForBugInGCC2 = MB;

static
void printMem(FILE *fd,char *s,double m)
{
  fprintf(fd,s);
  if (m < KB) {
    fprintf(fd,"%.0lf B",m);
    return;
  }
  if (m < MB) {
    fprintf(fd,"%.1lf kB",m/workaroundForBugInGCC1);
    return;
  }

  fprintf(fd,"%.1lf MB",m/workaroundForBugInGCC2);
}

ProfileCode(
void Statistics::heapAlloced(int sz)
{
  COUNT1(totalAllocated,sz);
  //  if (currAbstr)
  //    currAbstr->getPred()->heapUsed += sz;
}
)

int Statistics::getAtomMemory() {
  return CodeArea::atomTab.memRequired(sizeof(Literal));
}

int Statistics::getNameMemory() {
  return CodeArea::nameTab.memRequired(sizeof(Literal));
}

int Statistics::getCodeMemory() {
  return CodeArea::totalSize;
}

Statistics::Statistics()
{
  reset();
  timeUtime.total = osUserTime();
  timeUtime.idle();
}

void Statistics::reset()
{
  gcLastActive = 0;

  gcCollected.reset();
  heapUsed.reset();
  timeForPropagation.reset();
  timeForGC.reset();
  timeForCopy.reset();
  timeForLoading.reset();

  solveAlt.reset();
  solveCloned.reset();
  solveCreated.reset();
  solveSolved.reset();
  solveFailed.reset();

  fdvarsCreated.reset();
  propagatorsCreated.reset();
  propagatorsInvoked.reset();

  createdThreads.reset();
  runableThreads.reset();
}


//----------------------------------------------------------------------

void Statistics::printRunning(FILE *fd) {
  if (ozconf.showIdleMessage) {
    fprintf(fd,"running...\n");
    fflush(fd);
  }

#ifdef WINDOWS
  /* Windows 95 osUserTime() returns also clock ticks of idle time */
  timeUtime.incf(osUserTime()-timeUtime.sinceIdle);
  timeUtime.idle();
#endif
}

void Statistics::printIdle(FILE *fd)
{
  unsigned int timeNow = osUserTime();
  timeUtime.incf(timeNow-timeUtime.sinceIdle);
  int totalHeap = getUsedMemory()+heapUsed.total;

  if (ozconf.showIdleMessage) {
    fprintf(fd,"idle  ");
    printTime(fd,"r: ", timeUtime.sinceidle());
    printPercent(fd," (",
                 timeForPropagation.sinceidle(),
                 timeUtime.sinceidle());
    printPercent(fd,"%%p, ",
                 timeForCopy.sinceidle(),
                 timeUtime.sinceidle());
    printPercent(fd,"%%c, ",
                 timeForGC.sinceidle(),
                 timeUtime.sinceidle());
    printMem(fd,"%%g), h: ", (totalHeap-heapUsed.sinceIdle)*KB);
    fprintf(fd,"\n");
    fflush(fd);
  }
  heapUsed.sinceIdle = totalHeap;
  timeForPropagation.idle();
  timeForGC.idle();
  timeForCopy.idle();
  timeForLoading.idle();
  timeUtime.idle();
}

void Statistics::initGcMsg(int level)
{
  if (level > 0) {
    printf("Heap garbage collection...");
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

  /* do not count amount of memory copied */
  heapUsed.incf(-getUsedMemory());
  gcLastActive = getUsedMemory();

  if (level > 0) {
    printMem(stdout, " disposed ", gc_mem*KB);
    printf(" in %d msec.\n", gc_utime);
    fflush(stdout);
  }
}

void Statistics::initCount() {
#ifdef HEAP_PROFILE
  literal=0;
  ozfloat=0;
  bigInt=0;
  scriptLen=0;
  refsArray=0;
  refsArrayLen=0;
  continuation=0;
  suspCFun=0;
  sTuple=0;
  sTupleLen=0;
  lTuple=0;
  sRecord=0;
  sRecordLen=0;
  suspList=0;
  uvar=0;
  svar=0;
  cvar=0;
  dynamicTable= dynamicTableLen=0;
  taskStack=taskStackLen=0;
  cSolve=cCatch=cLocal=cCont=cXCont=cACont=cDebugCont=cExceptHandler=0;
  cCallCont=0;
  abstraction=flatObject=cell=chunk=0;
  heapChunk=thread=0;
  board=objectClass=0;
  askActor=waitActor=solveActor=waitChild=0;

  freeListAllocated = 0;
  freeListDisposed = 0;
  totalAllocated = 0;
  nonvarNonvarUnify = varNonvarUnify = varVarUnify = recRecUnify = totalUnify = 0;
  maxStackDepth = 0;
  maxEnvSize = 0;
  sizeClosures = numClosures = sizeGs = 0;
  sizeObjects = sizeRecords = sizeLists = 0;
  sizeStackVars = sizeEnvs = numEnvAllocs = 0;
  numDerefs = longestDeref = 0;
  for(int i=0; i<=maxDerefLength; i++) {
    lengthDerefs[i] = 0;
  }

  fastcalls=bicalls=nonoptcalls=inlinecalls=inlinedots=sendmsg=applmeth=0;
  nonoptbicalls=nonoptsendmsg=0;
  numNewName=numNewNamedName=0;
  numThreads=0;

#endif

  currAbstr = NULL;
  PrTabEntry::profileReset();
  OZ_CFunHeader::profileReset();
}


void Statistics::leaveCall(PrTabEntry  *newp)
{
  int usedHeap = getUsedMemoryBytes();
  if (currAbstr) {
    Assert(currAbstr->lastHeap>0);
    currAbstr->heapUsed += usedHeap - currAbstr->lastHeap;
    currAbstr->lastHeap = 0;
  }
  if (newp)
    newp->lastHeap = usedHeap;

  currAbstr = newp;
}


#ifdef HEAP_PROFILE

#include "ofgenvar.hh"

#define PrintVar(Var) \
  printf("%20s:          %8d\n",OZStringify(Var),Var)

#define PrintFloat(Var) \
  printf("%20s:          %8.2f\n",OZStringify(Var),Var)

#define PrintVarPercent(Var,Total) \
  printf("%20s:          %8d (%5.2f%%)\n",OZStringify(Var),Var,((double)Var*100.0)/(double)Total)

void Statistics::printCount() {
  printf("Heap after last GC:\n\n");
  printf("values:\n");
  printf("literal         %d (%dB)\n",literal,sizeof(Literal));
  printf("ozfloat         %d (%dB)\n",ozfloat,sizeof(Float));
  printf("bigInt          %d (%dB)\n",bigInt,sizeof(BigInt));
  printf("sTupleLen       %d (%dB)\n",sTupleLen,sizeof(TaggedRef));
  printf("lTuple          %d (%dB)\n",lTuple,sizeof(LTuple));
  printf("sRecord         %d (%dB)\n",sRecord,sizeof(SRecord));
  printf("sRecordLen      %d (%dB)\n",sRecordLen,sizeof(TaggedRef));
  printf("abstraction     %d (%dB)\n",abstraction,sizeof(Abstraction));
  printf("flatObject      %d (%dB)\n",flatObject,sizeof(Object));
  printf("objectClass     %d (%dB)\n",objectClass,sizeof(ObjectClass));
  printf("chunk           %d (%dB)\n",chunk,sizeof(SChunk));
  printf("heapChunk       %d (%dB)\n",heapChunk,sizeof(HeapChunk));

  printf("refsArray       %d (%dB)\n",refsArray,0);
  printf("refsArrayLen    %d (%dB)\n",refsArrayLen,sizeof(TaggedRef));

  printf("\nVariables:\n");
  printf("uvar            %d (%dB)\n",uvar,sizeof(TaggedRef));
  printf("svar            %d (%dB)\n",svar,sizeof(SVariable));
  printf("cvar            %d (%dB)\n",cvar,sizeof(GenCVariable));

  printf("\nLocal spaces\n");
  printf("scriptLen       %d (%dB)\n",scriptLen,sizeof(Equation));
  printf("board           %d (%dB)\n",board,sizeof(Board));
  printf("askActor        %d (%dB)\n",askActor,sizeof(AskActor));
  printf("waitActor       %d (%dB)\n",waitActor,sizeof(WaitActor));
  printf("solveActor      %d (%dB)\n",solveActor,sizeof(SolveActor));
  printf("waitChild       %d (%dB)\n",waitChild,sizeof(Board *));

  printf("\nThreads\n");
  printf("thread          %d (%dB)\n",thread,sizeof(Thread));
  printf("taskStack       %d (%dB)\n",taskStack,sizeof(TaskStack));
  printf("taskStackLen    %d (%dB)\n",taskStackLen,0);
  printf("cCatch          %d (%dB)\n",cCatch,4);
  printf("cLocal          %d (%dB)\n",cLocal,4);
  printf("cCont           %d (%dB)\n",cCont,12);
  printf("cXCont          %d (%dB)\n",cXCont,16);
  printf("cSetCaa         %d (%dB)\n",cSetCaa,8);
  printf("cDebugCont      %d (%dB)\n",cDebugCont,8);
  printf("cExceptHandler  %d (%dB)\n",cExceptHandler,8);
  printf("cCallCont       %d (%dB)\n",cCallCont,12);

  printf("continuation    %d (%dB)\n",continuation,sizeof(Continuation));
  printf("suspList        %d (%dB)\n",suspList,sizeof(SuspList));

  printf("\nOFS\n");
  printf("dynamicTable    %d (%dB)\n",dynamicTable,sizeof(DynamicTable));
  printf("dynamicTableLen %d (%dB)\n",dynamicTableLen,sizeof(HashElement));

  printf("\nRS\n");
  PrintVar(freeListAllocated);
  PrintVar(freeListDisposed);
  PrintVar(totalAllocated);
  PrintVar(totalUnify);
  PrintVarPercent(varVarUnify,totalUnify);
  PrintVarPercent(varNonvarUnify,totalUnify);
  PrintVarPercent(nonvarNonvarUnify,totalUnify);
  PrintVarPercent(recRecUnify,totalUnify);
  PrintVar(maxStackDepth);
  PrintVar(maxEnvSize);

  int szAbstr = sizeof(Abstraction);
  int sztr = sizeof(TaggedRef);
  double avrgNumGs = sizeGs / (double) numClosures;
  PrintVar(numClosures);
  PrintVarPercent(sizeClosures,totalAllocated);
  PrintVar(szAbstr);
  PrintFloat(avrgNumGs);
  PrintVarPercent(sizeObjects,totalAllocated);
  PrintVarPercent(sizeRecords,totalAllocated);
  PrintVarPercent(sizeLists,totalAllocated);
  int freeListSize = getMemoryInFreeList();
  PrintVarPercent(freeListSize,totalAllocated);

  PrintVarPercent(sizeStackVars,totalAllocated);

  double avrgEnvSize = ((double) sizeEnvs) / (double) numEnvAllocs;
  PrintVar(sizeEnvs);
  PrintVar(numEnvAllocs);
  PrintFloat(avrgEnvSize);

  PrintVar(numNewName);
  PrintVar(numNewNamedName);
  PrintVar(numThreads);

  printDeref();

  //int totCalls = fastcalls+bicalls+nonoptcalls+inlinecalls+inlinedots+sendmsg+applmeth;
  int totCalls = fastcalls+bicalls+nonoptcalls+inlinecalls+sendmsg+applmeth;

  printf("\nCalls\n");
  PrintVar(totCalls);
  PrintVarPercent(fastcalls,totCalls);
  PrintVarPercent(nonoptcalls,totCalls);
  PrintVar(nonoptbicalls);
  PrintVar(nonoptsendmsg);
  PrintVarPercent(bicalls,totCalls);
  PrintVarPercent(inlinecalls,totCalls);
  PrintVarPercent(sendmsg,totCalls);
  PrintVarPercent(applmeth,totCalls);
  PrintVar(inlinedots);

  int userCalls = fastcalls+sendmsg+nonoptcalls-nonoptbicalls;
  int envsPerUserCall = numEnvAllocs;
  printf("\n");
  PrintVar(userCalls);
  PrintVarPercent(envsPerUserCall,userCalls);
}


void Statistics::printDeref()
{
  printf("\n");
  PrintVar(numDerefs);
  PrintVar(longestDeref);
  for(int i=0; i<=maxDerefLength; i++) {
    if (lengthDerefs[i] || i==maxDerefLength)
      printf("\tlengthDerefs[%s%d]=%9d (%4.2f%%)\n",
             i==maxDerefLength ? ">=" : "",
             i,lengthDerefs[i],((double)lengthDerefs[i]*100)/(double)numDerefs);
  }
}

void Statistics::derefChain(int n)
{
  if (n>8)
    warning("long reference chain (length=%d)",n);
  numDerefs++;
  longestDeref = max(n,longestDeref);
  n = min(n,maxDerefLength);
  lengthDerefs[n]++;
}


#endif
