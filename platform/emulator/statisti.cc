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


#if !defined(OSF1_ALPHA) && !defined(WINDOWS)
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

void Statistics::print(FILE *fd)
{
  fprintf(fd,"\n******************************\n");
  fprintf(fd,"***  Oz System Statistics  ***\n");
  fprintf(fd,"******************************\n");

  unsigned total    = getUsedMemory()*KB;
  unsigned freeList = getMemoryInFreeList();
  unsigned occupied = total-freeList;

  fprintf(fd,  "\n  Memory areas:");
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
#ifndef WINDOWS
  printMem(fd,".\n    Size is ", ToInt32(sbrk(0))-mallocBase);
#endif
  fprintf(fd, ".\n");

#ifdef PROFILE
  fprintf(fd,"\n  Memory statistics:\n");
  if (allocateCounter) {
    fprintf(fd,"    Allocate uses \t\t %d bytes (%d%% of used heap)\n",
            allocateCounter,
            (allocateCounter*100)/occupied);
    fprintf(fd,"    Dellocate frees \t\t %d bytes (%d%%)\n",
            deallocateCounter, (deallocateCounter*100)/allocateCounter);
  }
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
          sum,    (sum*100)/occupied );
#endif

  fprintf(fd,"\n  Threads:\n");
  fprintf(fd,"    created        : %d\n",
          createdThreads.total);

#ifdef MM
  fprintf(fd,"    runable        : %d\n",
          runableThreads.total);
#endif

  fprintf(fd,"\n  Spaces (Search):\n");
  fprintf(fd,"    created:               %d\n",   solveCreated.total);
  fprintf(fd,"    cloned:                %d\n",   solveCloned.total);
  fprintf(fd,"    succeeded:             %d\n",   solveSolved.total);
  fprintf(fd,"    failed:                %d\n",   solveFailed.total);
  fprintf(fd,"    alternatives (choose): %d\n",   solveAlt.total);

#ifdef HEAP_PROFILE
  printCount();
#endif

  fprintf(fd,"\n  Finite Domain Constraints:\n");
  fprintf(fd,"    Variables created          : %d\n",
          fdvarsCreated.total);
  fprintf(fd,"    Propagators created        : %d\n",
          propagatorsCreated.total);
  fprintf(fd,"    Propagator invocations     : %d\n",
          propagatorsInvoked.total);
  printTime(fd,"    Time spent in propagators  : ", timeForPropagation.total);
  if (timeForPropagation.total > 0) {
    fprintf(fd,"\n    Propagator runs per second : %.0lf\n\n",
            1000*double(propagatorsInvoked.total)/timeForPropagation.total);
  } else {
    fprintf(fd,"\n");
  }

  fprintf(fd,"\n******************************\n");
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


static void recSetArg(OZ_Term record, char *feat, unsigned int val)
{
  OZ_Term t = OZ_subtree(record, OZ_atom(feat));
  if (t == 0 || !OZ_unifyInt(t,val)) {
    OZ_warning("recSetArg(%s,%s,%d) failed",toC(record),feat,val);
  }
}


OZ_Term Statistics::getThreads() {
  OZ_Term created  = OZ_pairAI("created",  createdThreads.total);
  OZ_Term runnable = OZ_pairAI("runnable", 0);

  return OZ_recordInit(OZ_atom("threads"),
                       OZ_cons(created,
                               OZ_cons(runnable,nil())));
}

OZ_Term Statistics::getTime() {
  unsigned int timeNow = osUserTime();

  OZ_Term copy      = OZ_pairAI("copy",      timeForCopy.total);
  OZ_Term gc        = OZ_pairAI("gc",        timeForGC.total);
  OZ_Term load      = OZ_pairAI("load",      timeForLoading.total);
  OZ_Term propagate = OZ_pairAI("propagate", timeForPropagation.total);
  OZ_Term run       = OZ_pairAI("run",       timeNow-(timeForGC.total +
                                                      timeForLoading.total +
                                                      timeForPropagation.total +
                                                      timeForCopy.total));
  OZ_Term system    = OZ_pairAI("system",    osSystemTime());
  OZ_Term user      = OZ_pairAI("user",      timeNow);

  return OZ_recordInit(OZ_atom("time"),
                       OZ_cons(copy,OZ_cons(gc,
                         OZ_cons(load,OZ_cons(propagate,
                           OZ_cons(run,OZ_cons(system,
                             OZ_cons(user,nil()))))))));
}

OZ_Term Statistics::getMemory() {
  OZ_Term atoms    = OZ_pairAI("atoms",
                               CodeArea::atomTab.memRequired(sizeof(Literal)));
  OZ_Term names    = OZ_pairAI("names",
                               CodeArea::nameTab.memRequired(sizeof(Literal)));
  OZ_Term builtins = OZ_pairAI("builtins", builtinTab.memRequired());
  OZ_Term freelist = OZ_pairAI("freelist", getMemoryInFreeList());
  OZ_Term code     = OZ_pairAI("code",     CodeArea::totalSize);

  return OZ_recordInit(OZ_atom("memory"),
                       OZ_cons(atoms, OZ_cons(builtins,
                         OZ_cons(code, OZ_cons(freelist,
                           OZ_cons(names, nil()))))));
}

OZ_Term Statistics::getSpaces() {
  OZ_Term chosen  = OZ_pairAI("chosen",    solveAlt.total);
  OZ_Term cloned  = OZ_pairAI("cloned",    solveCloned.total);
  OZ_Term created = OZ_pairAI("created",   solveCreated.total);
  OZ_Term failed  = OZ_pairAI("failed",    solveFailed.total);
  OZ_Term solved  = OZ_pairAI("succeeded", solveSolved.total);

  return OZ_recordInit(OZ_atom("spaces"),
                       OZ_cons(chosen,OZ_cons(cloned,
                         OZ_cons(created,OZ_cons(failed,
                           OZ_cons(solved,nil()))))));
}

OZ_Term Statistics::getFD() {
  OZ_Term variables   = OZ_pairAI("variables",   fdvarsCreated.total);
  OZ_Term propagators = OZ_pairAI("propagators", propagatorsCreated.total);
  OZ_Term invoked     = OZ_pairAI("invoked",     propagatorsInvoked.total);
  OZ_Term runs        = OZ_pairAI("runs",
                                  (timeForPropagation.total>0) ?
                                  ((int) (1000*double(propagatorsInvoked.total)/
                                          timeForPropagation.total)) :
                                  0);

  return OZ_recordInit(OZ_atom("fd"),
                       OZ_cons(variables,
                               OZ_cons(propagators,
                                       OZ_cons(invoked,
                                               OZ_cons(runs,nil())))));
}

void Statistics::printRunning(FILE *fd)
{
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

  /* do not count amount of meory copied */
  heapUsed.incf(-getUsedMemory());
  gcLastActive = getUsedMemory();

  if (level > 0) {
    printMem(stdout, " disposed ", gc_mem*KB);
    printf(" in %d msec.\n", gc_utime);

  }
}

#ifdef HEAP_PROFILE
void Statistics::initCount() {
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
  suspension=0;
  suspList=0;
  uvar=0;
  svar=0;
  cvar=0;
  dynamicTable= dynamicTableLen=0;
  taskStack=taskStackLen=0;
  cSolve=cCatch=cLocal=cJob=cCont=cXCont=cACont=cDebugCont=cExceptHandler=0;
  cCallCont= cCFuncCont=0;
  abstraction=deepObject=flatObject=cell=chunk=0;
  oneCallBuiltin=solvedBuiltin=builtin=0;
  heapChunk=thread=0;
  board=objectClass=0;
  askActor=waitActor=solveActor=waitChild=0;
  solveDLLStack=0;
}

#include "ofgenvar.hh"

void Statistics::printCount() {
  printf("Heap after last GC:\n\n");
  printf("values:\n");
  printf("literal         %d (%dB)\n",literal,sizeof(Literal));
  printf("ozfloat         %d (%dB)\n",ozfloat,sizeof(Float));
  printf("bigInt          %d (%dB)\n",bigInt,sizeof(BigInt));
  printf("sTuple          %d (%dB)\n",sTuple,sizeof(STuple));
  printf("sTupleLen       %d (%dB)\n",sTupleLen,sizeof(TaggedRef));
  printf("lTuple          %d (%dB)\n",lTuple,sizeof(LTuple));
  printf("sRecord         %d (%dB)\n",sRecord,sizeof(SRecord));
  printf("sRecordLen      %d (%dB)\n",sRecordLen,sizeof(TaggedRef));
  printf("abstraction     %d (%dB)\n",abstraction,sizeof(Abstraction));
  printf("deepObject      %d (%dB)\n",deepObject,sizeof(DeepObject));
  printf("flatObject      %d (%dB)\n",flatObject,sizeof(Object));
  printf("objectClass     %d (%dB)\n",objectClass,sizeof(ObjectClass));
  printf("cell            %d (%dB)\n",cell,sizeof(Cell));
  printf("chunk           %d (%dB)\n",chunk,sizeof(SChunk));
  printf("oneCallBuiltin  %d (%dB)\n",oneCallBuiltin,sizeof(OneCallBuiltin));
  printf("solvedBuiltin   %d (%dB)\n",solvedBuiltin,sizeof(SolvedBuiltin));
  printf("builtin         %d (%dB)\n",builtin,sizeof(Builtin));
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
  printf("solveDLLStack   %d (%dB)\n",solveDLLStack,sizeof(DLLStackEntry *));

  printf("\nThreads\n");
  printf("thread          %d (%dB)\n",thread,sizeof(Thread));
  printf("taskStack       %d (%dB)\n",taskStack,sizeof(TaskStack));
  printf("taskStackLen    %d (%dB)\n",taskStackLen,0);
  printf("cCatch          %d (%dB)\n",cCatch,4);
  printf("cLocal          %d (%dB)\n",cLocal,4);
  printf("cJob            %d (%dB)\n",cJob,4);
  printf("cCont           %d (%dB)\n",cCont,12);
  printf("cXCont          %d (%dB)\n",cXCont,16);
  printf("cSetCaa         %d (%dB)\n",cSetCaa,8);
  printf("cDebugCont      %d (%dB)\n",cDebugCont,8);
  printf("cExceptHandler  %d (%dB)\n",cExceptHandler,8);
  printf("cCallCont       %d (%dB)\n",cCallCont,12);
  printf("cCFuncCont      %d (%dB)\n",cCFuncCont,16);

  printf("continuation    %d (%dB)\n",continuation,sizeof(Continuation));
  printf("suspCFun        %d (%dB)\n",suspCFun,sizeof(CFuncContinuation));
  printf("suspension      %d (%dB)\n",suspension,sizeof(Suspension));
  printf("suspList        %d (%dB)\n",suspList,sizeof(SuspList));

  printf("\nOFS\n");
  printf("dynamicTable    %d (%dB)\n",dynamicTable,sizeof(DynamicTable));
  printf("dynamicTableLen %d (%dB)\n",dynamicTableLen,sizeof(HashElement));
}
#endif
