/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "statisti.hh"
#endif

#include <stdio.h>

#include "statisti.hh"
#include "ozconfig.hh"

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
    fprintf(fd,"%.0f B",m);
    return;
  }
  if (m < MB) {
    fprintf(fd,"%.1f kB",m/workaroundForBugInGCC1);
    return;
  }
  
  fprintf(fd,"%.1f MB",m/workaroundForBugInGCC2);
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

    if (ozconf.timeDetailed) {
      printPercent(fd," (",
		   timeForPropagation.sinceidle(),
		   timeUtime.sinceidle());
      printPercent(fd,"%%p, ",
		   timeForCopy.sinceidle(),
		   timeUtime.sinceidle());
      printPercent(fd,"%%c, ",
		   timeForGC.sinceidle(),
		   timeUtime.sinceidle());
      fprintf(fd,"%%g)");
    }
    printMem(fd,", h: ", (totalHeap-heapUsed.sinceIdle)*KB);
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
  sizeHeapChunks = sizeStackVars = sizeEnvs = numEnvAllocs = 0;
  numDerefs = longestDeref = 0;
  for(int i=0; i<=maxDerefLength; i++) {
    lengthDerefs[i] = 0;
  }

  fastcalls=bicalls=nonoptcalls=inlinecalls=inlinedots=sendmsg=applmeth=0;
  nonoptbicalls=nonoptsendmsg=0;
  numNewName=numNewNamedName=0;
  numThreads=0;

#endif

#ifdef PROFILE_INSTR
  for (int i = 0; i < PROFILE_INSTR_MAX; i++) instr[i] = 0;
#endif

  currAbstr = NULL;
  PrTabEntry::profileReset();
  OZ_CFunHeader::profileReset();
}


void Statistics::leaveCall(PrTabEntry  *newp) 
{
  unsigned int usedHeap = getUsedMemoryBytes();
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
  fprintf(out,"%20s:          %8d\n",OZStringify(Var),(int)Var)

#define PrintFloat(Var) \
  fprintf(out,"%20s:          %8.2f\n",OZStringify(Var),Var)

#define PrintVarPercent(Var,Total) \
  fprintf(out,"%20s:          %8d (%5.2f%%)\n",OZStringify(Var),(int)Var,((double)Var*100.0)/(double)Total)

void Statistics::printCount(char *file) {

  FILE *out = (strcmp(file,"-")==0) ? stdout : fopen(file,"w");
  if (out==NULL) {
    warning("cannot open '%s': %s\n",file,OZ_unixError(errno));
    return;
  }
  fprintf(out,"Heap after last GC:\n\n");
  fprintf(out,"values:\n");
  fprintf(out,"literal         %ld (%dB)\n",literal,sizeof(Literal));
  fprintf(out,"ozfloat         %ld (%dB)\n",ozfloat,sizeof(Float));
  fprintf(out,"bigInt          %ld (%dB)\n",bigInt,sizeof(BigInt));
  fprintf(out,"sTupleLen       %ld (%dB)\n",sTupleLen,sizeof(TaggedRef));
  fprintf(out,"lTuple          %ld (%dB)\n",lTuple,sizeof(LTuple));
  fprintf(out,"sRecord         %ld (%dB)\n",sRecord,sizeof(SRecord));
  fprintf(out,"sRecordLen      %ld (%dB)\n",sRecordLen,sizeof(TaggedRef));
  fprintf(out,"abstraction     %ld (%dB)\n",abstraction,sizeof(Abstraction));
  fprintf(out,"flatObject      %ld (%dB)\n",flatObject,sizeof(Object));
  fprintf(out,"objectClass     %ld (%dB)\n",objectClass,sizeof(ObjectClass));
  fprintf(out,"chunk           %ld (%dB)\n",chunk,sizeof(SChunk));
  fprintf(out,"heapChunk       %ld (%dB)\n",heapChunk,sizeof(HeapChunk));
  
  fprintf(out,"refsArray       %ld (%dB)\n",refsArray,0);
  fprintf(out,"refsArrayLen    %ld (%dB)\n",refsArrayLen,sizeof(TaggedRef));

  fprintf(out,"\nVariables:\n");
  fprintf(out,"uvar            %ld (%dB)\n",uvar,sizeof(TaggedRef));
  fprintf(out,"svar            %ld (%dB)\n",svar,sizeof(SVariable));
  fprintf(out,"cvar            %ld (%dB)\n",cvar,sizeof(GenCVariable));

  fprintf(out,"\nLocal spaces\n");
  fprintf(out,"scriptLen       %ld (%dB)\n",scriptLen,sizeof(Equation));
  fprintf(out,"board           %ld (%dB)\n",board,sizeof(Board));
  fprintf(out,"askActor        %ld (%dB)\n",askActor,sizeof(AskActor));
  fprintf(out,"waitActor       %ld (%dB)\n",waitActor,sizeof(WaitActor));
  fprintf(out,"solveActor      %ld (%dB)\n",solveActor,sizeof(SolveActor));
  fprintf(out,"waitChild       %ld (%dB)\n",waitChild,sizeof(Board *));

  fprintf(out,"\nThreads\n");
  fprintf(out,"thread          %ld (%dB)\n",thread,sizeof(Thread));
  fprintf(out,"taskStack       %ld (%dB)\n",taskStack,sizeof(TaskStack));
  fprintf(out,"taskStackLen    %ld (%dB)\n",taskStackLen,0);
  fprintf(out,"cCatch          %ld (%dB)\n",cCatch,4);
  fprintf(out,"cLocal          %ld (%dB)\n",cLocal,4);
  fprintf(out,"cCont           %ld (%dB)\n",cCont,12);
  fprintf(out,"cXCont          %ld (%dB)\n",cXCont,16);
  fprintf(out,"cSetCaa         %ld (%dB)\n",cSetCaa,8);
  fprintf(out,"cDebugCont      %ld (%dB)\n",cDebugCont,8);
  fprintf(out,"cExceptHandler  %ld (%dB)\n",cExceptHandler,8);
  fprintf(out,"cCallCont       %ld (%dB)\n",cCallCont,12);

  fprintf(out,"continuation    %ld (%dB)\n",continuation,sizeof(Continuation));
  fprintf(out,"suspList        %ld (%dB)\n",suspList,sizeof(SuspList));

  fprintf(out,"\nOFS\n");
  fprintf(out,"dynamicTable    %ld (%dB)\n",dynamicTable,sizeof(DynamicTable));
  fprintf(out,"dynamicTableLen %ld (%dB)\n",dynamicTableLen,sizeof(HashElement));

  fprintf(out,"\nRS\n");
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

  PrintVarPercent(sizeHeapChunks,totalAllocated);
  PrintVarPercent(sizeStackVars,totalAllocated);

  double avrgEnvSize = ((double) sizeEnvs) / (double) numEnvAllocs;
  PrintVar(sizeEnvs);
  PrintVar(numEnvAllocs);
  PrintFloat(avrgEnvSize);

  PrintVar(numNewName);
  PrintVar(numNewNamedName);
  PrintVar(numThreads);

  printDeref(out);

  //int totCalls = fastcalls+bicalls+nonoptcalls+inlinecalls+inlinedots+sendmsg+applmeth;
  int totCalls = fastcalls+bicalls+nonoptcalls+inlinecalls+sendmsg+applmeth;

  fprintf(out,"\nCalls\n");
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

  int userCalls = fastcalls+sendmsg+nonoptcalls+applmeth-nonoptbicalls;
  int envsPerUserCall = numEnvAllocs;
  fprintf(out,"\n");
  PrintVar(userCalls);
  PrintVarPercent(envsPerUserCall,userCalls);

  if (out != stdout)
    fclose(out);
}


void Statistics::printDeref(FILE *out)
{
  fprintf(out,"\n");
  PrintVar(numDerefs);
  PrintVar(longestDeref);
  for(int i=0; i<=maxDerefLength; i++) {
    if (lengthDerefs[i] || i==maxDerefLength)
      fprintf(out,"\tlengthDerefs[%s%d]=%9ld (%4.2f%%)\n",
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


#ifdef PROFILE_INSTR
#include "codearea.hh"
void Statistics::printInstr()
{
  printf("Instruction profile:\n");
  unsigned long sum = 0;
  for (int i=0; i<PROFILE_INSTR_MAX; i++) {
    sum += instr[i];
    if (instr[i]!=0)
      printf("%010lu x %s\n",instr[i],CodeArea::opToString[i]);
  }
  printf("----------\n%010lu\n",sum);
}
#endif
