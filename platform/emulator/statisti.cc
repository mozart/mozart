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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "statisti.hh"
#endif

#include <stdio.h>

#include "wsock.hh"
#include "statisti.hh"
#include "ozconfig.hh"

#include "am.hh"
#include "codearea.hh"
#include "os.hh"
#include "fdomn.hh"

Statistics ozstat;

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

int Statistics::getAtomMemory() {
  return CodeArea::atomTab.memRequired(sizeof(Literal));
}

int Statistics::getNameMemory() {
  return CodeArea::nameTab.memRequired(sizeof(Literal));
}

Statistics::Statistics()
{
}

void Statistics::init()
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
  timeIdle = 0;

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
  timeUtime.idle();
}

#ifdef TRACE_ALOVER
float perc_alover;
#endif

void Statistics::initGcMsg(int level)
{
  if (level > 0) {
    printf("Heap garbage collection...");
    fflush(stdout);
  }

  gcStarttime = osUserTime();
  gcStartmem  = getUsedMemory();
  heapUsed.incf(gcStartmem);

#ifdef TRACE_ALOVER
  perc_alover = (100.0 * _oz_alover) / getUsedMemoryBytes();
  _oz_alover = 0;
#endif
}

void Statistics::printGcMsg(int level)
{
  int gc_utime = osUserTime()-gcStarttime;
  int gc_mem   = gcStartmem-getUsedMemory();

  timeForGC.incf(gc_utime);
  gcCollected.incf(gc_mem);

  /* do not count amount of memory copied */
  heapUsed.incf(- ((int) getUsedMemory()));
  gcLastActive = getUsedMemory();

  if (level > 0) {
    printMem(stdout, " disposed ", gc_mem*KB);
    printf(" in %d msec.\n", gc_utime);
#ifdef TRACE_ALOVER
    printf("Allocation overhead is %2.2f%%.\n",perc_alover);
#endif
    fflush(stdout);
  }
}

void Statistics::leaveCall(PrTabEntry  *newp)
{
  unsigned int usedHeap = getUsedMemoryBytes();
  if (currAbstr) {
    PrTabEntryProfile * prf = currAbstr->getProfile();
    Assert(prf->lastHeap>0);
    prf->heapUsed += usedHeap - prf->lastHeap;
    prf->lastHeap = 0;
  }
  if (newp)
    newp->getProfile()->lastHeap = usedHeap;

  currAbstr = newp;
}



#ifdef PROFILE_INSTR
#include "codearea.hh"

#define AUXFILE "instrprofile.out"

void Statistics::printInstr()
{
  FILE *out = fopen(AUXFILE,"w");
  if (out==NULL) {
    perror("fopen");
    return;
  }
  unsigned long sum = 0;
  for (int i=0; i<(int)OZERROR; i++) {
    sum += instr[i];
    fprintf(out,"%10lu x %s\n",instr[i],opcodeToString((Opcode)i));
  }
  fprintf(out,"----------\n%010lu\n",sum);
  fclose(out);

  printf("Instruction profile printed to '%s'.\n",AUXFILE);
}

void Statistics::printInstrCollapsable()
{
  FILE *out = fopen(AUXFILE,"w");
  if (out==NULL) {
    perror("fopen");
    return;
  }
  for (int i=0; i<PROFILE_INSTR_MAX; i++) {
    for (int j=0; j<PROFILE_INSTR_MAX; j++) {
      if (instrCollapsable[i][j]!=0)
        fprintf(out,"%10lu x %s %s\n",
                instrCollapsable[i][j],
                opcodeToString((Opcode)i),opcodeToString((Opcode)j));
    }
  }
  fclose(out);
  printf("Collapsable instruction profile printed to '%s'.\n",AUXFILE);
}

void Statistics::printInstrReset()
{
  for (int i=0; i<PROFILE_INSTR_MAX; i++) {
    instr[i]=0;
    for (int j=0; j<PROFILE_INSTR_MAX; j++) {
      instrCollapsable[i][j]=0;
    }
  }
}

#endif
