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

#ifdef HEAP_PROFILE
  printCount();
#endif

  fprintf(fd,"  FD variables created: %d\n", fdvarsCreated.total);
  fprintf(fd,"  Propagators created : %d\n\n", propagatorsCreated.total); 

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

  fdvarsCreated.reset();
  propagatorsCreated.reset();
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

OZ_Term Statistics::getStatistics()
{
  unsigned int timeNow = osUserTime();

  OZ_Term r=OZ_pairAI("r",timeNow-(timeForGC.total+timeForLoading.total+
				   timeForCopy.total));
  OZ_Term g=OZ_pairAI("g",timeForGC.total);
  OZ_Term l=OZ_pairAI("l",timeForLoading.total);
  OZ_Term c=OZ_pairAI("c",timeForCopy.total);
  OZ_Term h=OZ_pairAI("h",heapUsed.total+getUsedMemory());
  OZ_Term s=OZ_pairAI("s",osSystemTime());
  OZ_Term u=OZ_pairAI("u",timeNow);

  OZ_Term a2=OZ_pairAI("a",solveAlt.total);
  OZ_Term c2=OZ_pairAI("c",solveClone.total);
  OZ_Term s2=OZ_pairAI("s",solveSolved.total);
  OZ_Term f2=OZ_pairAI("f",solveFailed.total);
  OZ_Term e=OZ_pair(OZ_CToAtom("e"),
		    OZ_recordInit(OZ_CToAtom("enum"),
				  OZ_cons(a2,OZ_cons(c2,OZ_cons(s2,OZ_cons(f2,OZ_nil()))))));
  
  return OZ_recordInit(OZ_CToAtom("stat"),
		       OZ_cons(r,OZ_cons(g,OZ_cons(l,OZ_cons(c,OZ_cons(h,OZ_cons(s,OZ_cons(u,OZ_cons(e,nil())))))))));

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
  cSolve=cLocal=cJob=cCont=cXCont=cACont=cDebugCont=cExceptHandler=0;
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
