/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  ------------------------------------------------------------------------
  Unix/Posix functions
  ------------------------------------------------------------------------
*/

#if defined(__GNUC__) && !defined(NOPRAGMA)
#pragma implementation "os.hh"
#endif

#include "wsock.hh"

#include <errno.h>
#include <limits.h>
#include <malloc.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef WINDOWS
#include <sys/times.h>
#include <sys/wait.h>
#include <sys/time.h>
#endif

#if !defined(ultrix) && !defined(WINDOWS)
# include <sys/socket.h>
#endif

#ifdef AIX3_RS6000
#include <sys/select.h>
#endif

#ifdef IRIX5_MIPS
// #include <bstring.h>
#endif

#ifdef LINUX
// #include <sys/utsname.h>
#endif

#ifdef WINDOWS
/* can be removed if osSelect is adapted */
#include "am.hh"
#endif

#include "tagged.hh"
#include "os.hh"

// return current usertime in milliseconds
unsigned int osUserTime()
{
#ifdef WINDOWS
  return 0;
#else
  struct tms buffer;

  times(&buffer);
  return (unsigned int)(buffer.tms_utime*1000.0/(double)sysconf(_SC_CLK_TCK));
#endif
}

// return current systemtime in milliseconds
unsigned int osSystemTime()
{
#ifdef WINDOWS
  return 0;
#else
  struct tms buffer;
  
  times(&buffer);
  return (unsigned int)(buffer.tms_stime*1000.0/(double)sysconf(_SC_CLK_TCK));
#endif
}


void osBlockSignals(Bool check)
{
#ifndef WINDOWS
  sigset_t s,sOld;
  sigfillset(&s);

  /* some signals should not be blocked */
  sigdelset(&s,SIGINT);
  sigdelset(&s,SIGHUP);
  sigdelset(&s,SIGTERM);

  sigprocmask(SIG_SETMASK,&s,&sOld);

#ifdef DEBUG_CHECK
  if (check) {
    sigemptyset(&s);
    if (memcmp(&s,&sOld,sizeof(sigset_t)) != 0) {
      warning("blockSignals: there are blocked signals");
    }
  }
#endif
#endif
}

void osUnblockSignals()
{
#ifndef WINDOWS
  sigset_t s;
  sigemptyset(&s);
  sigprocmask(SIG_SETMASK,&s,NULL);
#endif
}


/* Oz version of signal(2)
   NOTE: (mm 13.10.94)
    Linux & Solaris are not POSIX compatible:
     therefor we need casts to (OsSigFun *). look at HERE.
    */
OsSigFun *osSignal(int signo, OsSigFun *fun)
{
#ifdef WINDOWS
  signal(signo,fun);
  return NULL;
#else
  struct sigaction act, oact;

  /* type of act.sa_handler ist not the same on all platforms, 
   * therefore this quite intricateness */
  OsSigFun **f = (OsSigFun**)&act.sa_handler;
  *f = fun;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  /* The following piece of code is from Stevens: Advanced UNIX Programming */
  if (signo == SIGALRM) {
#ifdef SA_INTERUPT /* SunOS */
    act.sa_flags |= SA_INTERUPT;
#endif
  } else {
#ifdef SA_RESTART /* Sys V */
    act.sa_flags |= SA_RESTART;
#endif
  }
  if (sigaction(signo,&act,&oact) <0) {
    /* HERE */
    return (OsSigFun *) SIG_ERR;
  }
  /* HERE */
  return (OsSigFun *) oact.sa_handler;
#endif
}



/* Oz version of system(3)
 * Posix 2 requires this call not to be interuptible by a signal
 * however some OS (like Solaris 2.3) do return EINTR, so we define our
 * own one, stolen from Stevens.0
 */


int osSystem(char *cmd)
{
#ifdef WINDOWS
  return system(cmd);
#else
  if (cmd == NULL) {
    return 1;
  }

  pid_t pid;
  if ((pid = fork()) < 0) {
    return -1;
  }

  if (pid == 0) {
    execl("/bin/sh","sh","-c",cmd, (char*) NULL);
    _exit(127);     /* execl error */
  }

  int status;
  while(waitpid(pid,&status,0) < 0) {
    if (errno != EINTR) {
      return -1;
    }
  }
  
  return status;
#endif
}


#ifndef __GNUC__
// Linking object files compiled with gcc needs these:

/* void * operator new (size_t sz) */
extern "C" void * __builtin_new (size_t sz)
{
  void *p;

  /* malloc (0) is unpredictable; avoid it.  */
  if (sz == 0)
    sz = 1;
  p = (void *) malloc (sz);
  return p;
}

/* void operator delete (void *ptr) */
extern "C" void __builtin_delete (void *ptr)
{
  if (ptr)
    free (ptr);
}

#endif  /* __GNUC__ */



#ifdef HPUX_700
#include <time.h>
#endif

/* The prototypes for select are wrong on HP-UX 9.x */
static int osSelect(int nfds, fd_set *readfds, fd_set *writefds,
		    fd_set *exceptfds, struct timeval *timeout)
{
#ifdef WINDOWS
  /* force a blocking read on compiler input */
  if (timeout == NULL) {
    FD_SET(am.compStream->csfileno(),readfds);
    return 1;
  } else {
    return 0;
  }

#else

#ifdef HPUX_700
  return select(nfds, (int*)readfds,(int*)writefds,(int*)exceptfds,timeout);
#else
  return select(nfds, readfds,writefds,exceptfds,timeout);
#endif

#endif  /* WINDOWS */
}

void osInitSignals()
{
#ifndef DEBUG_DET
  osSignal(SIGINT,handlerINT);
  osSignal(SIGTERM,handlerTERM);
  osSignal(SIGSEGV,handlerSEGV);
  osSignal(SIGBUS,handlerBUS);
  osSignal(SIGPIPE,handlerPIPE);
  osSignal(SIGUSR1,handlerUSR1);
  osSignal(SIGCHLD,handlerCHLD);
  osSignal(SIGFPE,handlerFPE);
  osSignal(SIGALRM,handlerALRM);
#endif
}

void osSetAlarmTimer(int t, Bool interval)
{
#ifdef DEBUG_DET
  return;
#else
  struct itimerval newT;

  int sec  = t/1000;
  int usec = (t*1000)%1000000;
  newT.it_interval.tv_sec  = (interval ? sec : 0);
  newT.it_interval.tv_usec = (interval ? usec : 0);
  newT.it_value.tv_sec     = sec;
  newT.it_value.tv_usec    = usec;

  if (setitimer(ITIMER_REAL,&newT,NULL) < 0) {
    ozpwarning("setitimer");
  }
#endif
}

int osGetAlarmTimer()
{
#ifdef DEBUG_DET
  return 0;
#else
  struct itimerval timer;
  if (getitimer(ITIMER_REAL,&timer) < 0) {
    ozpwarning("getitimer");
    return -1;
  }

  return (timer.it_value.tv_sec * 1000) + (timer.it_value.tv_usec/1000);
#endif
}

Bool osHasJobControl()
{
#if defined(_POSIX_JOB_CONTROL)
  return OK;
#elif defined(_SC_JOB_CONTROL)
  return sysconf(_SC_JOB_CONTROL)==-1:NO:OK;
#else
  return NO;
#endif
}



/*********************************************************
 *       Sockets                                         *
 *********************************************************/

static long openMax;
static fd_set globalFDs[2];     // mask of active read/write FDs

int osOpenMax()
{
#ifdef WINDOWS
  return OPEN_MAX;
#else
  int ret = sysconf(_SC_OPEN_MAX);
  if (ret == -1) {
    ret = _POSIX_OPEN_MAX;
  }
  return ret;
#endif
}

void osInit()
{
#ifdef MAKEANEWPGRP // mm2
  // create a new process group, so that we can
  // easily terminate all our children
  if (setpgrp(getpid(),getpid()) < 0) {
    ozperror("setpgrp");
  }
#endif

#ifndef WINDOWS
  if (osHasJobControl()) {
    /* start a new process group */
    if (setpgid(0,0)<0) {
      DebugCheckT(ozpwarning("setpgid(0,0)"));
    }
  } else {
    DebugCheckT(warning("OS does not support POSIX job control\n"));
  }
#endif

  DebugCheck(CLOCK_TICK < 1000, error("CLOCK_TICK must be greater than 1 ms"));

  openMax=osOpenMax();
  DebugCheckT(printf("openMax: %d\n",openMax));

  FD_ZERO(&globalFDs[SEL_READ]);
  FD_ZERO(&globalFDs[SEL_WRITE]);
}

#define CheckMode(mode) Assert(mode==SEL_READ || mode==SEL_WRITE)


void osWatchFD(int fd, int mode)
{
  CheckMode(mode);
  FD_SET(fd,&globalFDs[mode]); 
}

void osClrWatchedFD(int fd, int mode)
{
  CheckMode(mode);
  FD_CLR(fd,&globalFDs[mode]); 
}

Bool osIsWatchedFD(int fd, int mode)
{
  CheckMode(mode);
  return (FD_ISSET(fd,&globalFDs[mode])); 
}


/* do a select, that waits "ticks" ticks.
 * if "ticks" <= 0 do a blocking select
 * return number of ticks left
 */
int osBlockSelect(int ticks)
{
  fd_set copyFDs[2];
  copyFDs[SEL_READ]  = globalFDs[SEL_READ];
  copyFDs[SEL_WRITE] = globalFDs[SEL_WRITE];
  int wait = osClockTickToMs(ticks);
  osSetAlarmTimer(wait,NO);
  osUnblockSignals();
  int ret = osSelect(openMax,&copyFDs[SEL_READ],&copyFDs[SEL_WRITE],NULL,NULL);
  osBlockSignals();
  int ticksleft = osMsToClockTick(osGetAlarmTimer());
  return ticksleft;
}

/* osClearSocketErrors
     remove the closed/failed descriptors from the fd_sets
   */
void osClearSocketErrors()
{
  for (int i = 0; i < openMax; i++) {
    for(int mode=SEL_READ; mode <= SEL_WRITE; mode++) {
      if (FD_ISSET(i,&globalFDs[mode]) && osTestSelect(i,mode) < 0) {
	FD_CLR(i,&globalFDs[mode]);
      }
    }
  }
}



/* returns: 1 if select succeeded on fd
 *          0 did not succeed
 *         -1 on error
 */
int osTestSelect(int fd, int mode)
{
  CheckMode(mode);
  while(1) {
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    fd_set fdset, *readFDs=NULL, *writeFDs=NULL;
    FD_ZERO(&fdset);
    FD_SET(fd,&fdset);
    
    if (mode==SEL_READ) {
      readFDs = &fdset;
    } else {
      writeFDs = &fdset;
    }
    int ret = osSelect(fd+1, readFDs, writeFDs, NULL, &timeout);
    if (ret >= 0 || errno != EINTR) {
      return ret;
    }
  }
}

/* -------------------------------------------------------------------------
 * subroutines for AM::handleIO
 * ------------------------------------------------------------------------- */
   
static fd_set tmpFDs[2];

/* signals are blocked */
int osFirstSelect()
{
 loop:
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  tmpFDs[SEL_READ]  = globalFDs[SEL_READ];
  tmpFDs[SEL_WRITE] = globalFDs[SEL_WRITE];

  int numbOfFDs = osSelect(openMax,&tmpFDs[SEL_READ],&tmpFDs[SEL_WRITE], NULL,&timeout);

  if (numbOfFDs < 0) {
    if (errno == EINTR) {
      goto loop;
    }

    if (errno != EBADF) {  /* the compiler may have terminated */
      ozpwarning("select failed");
    }
    osClearSocketErrors();
  }

  return numbOfFDs;
}

Bool osNextSelect(int fd, int mode)
{
  Assert(fd<openMax);
  CheckMode(mode);

  if (FD_ISSET(fd,&tmpFDs[mode])) {
    FD_CLR(fd,&tmpFDs[mode]);
    return OK;
  }
  return NO;
}

/* -------------------------------------------------------------------------
 * subroutine for AM::checkIO
 * ------------------------------------------------------------------------- */

// called from AM::checkIO
int osCheckIO()
{
 loop:
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  
  fd_set copyFDs[2];
  copyFDs[SEL_READ]  = globalFDs[SEL_READ];
  copyFDs[SEL_WRITE] = globalFDs[SEL_WRITE];
  
  int numbOfFDs = osSelect(openMax,&copyFDs[SEL_READ],&copyFDs[SEL_WRITE],NULL,&timeout);
  if (numbOfFDs < 0) {
    if (errno == EINTR) goto loop;
    if (errno != EBADF) { /* the compiler may have terminated (rs) */
      ozpwarning("checkIO: select failed");
    }
    osClearSocketErrors();
  }
  return numbOfFDs;
}


void osKillChildren()
{
  if (osHasJobControl()) {
    // terminate all our children
    OsSigFun *save=osSignal(SIGTERM,(OsSigFun*)SIG_IGN);
#ifndef WINDOWS
    if (kill(-getpid(),SIGTERM) < 0) {
      ozpwarning("kill");
    }
#endif
    osSignal(SIGTERM,save);
  }
}
