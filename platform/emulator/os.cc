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

#include <errno.h>
#include <limits.h>
#include <malloc.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/times.h>
#include <sys/wait.h>
#include <sys/time.h>

#ifndef ultrix
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

#include "tagged.hh"
#include "os.hh"

// return current usertime in milliseconds
unsigned int osUserTime()
{
  struct tms buffer;

  times(&buffer);
  return (unsigned int)(buffer.tms_utime*1000.0/(double)sysconf(_SC_CLK_TCK));
}

// return current systemtime in milliseconds
unsigned int osSystemTime()
{
  struct tms buffer;
  
  times(&buffer);
  return (unsigned int)(buffer.tms_stime*1000.0/(double)sysconf(_SC_CLK_TCK));
}


Bool osBlockSignals()
{
//  return sigblock(~0) == 0;
  sigset_t s,sOld;
  sigfillset(&s);
  /* SOME SIGNALS SHOULD NOT BE BLOCKED (RS) */
//  sigdelset(&s,SIGINT);
  sigprocmask(SIG_SETMASK,&s,&sOld);
  sigemptyset(&s);
  return memcmp(&s,&sOld,sizeof(sigset_t)) == 0;
}

void osUnblockSignals()
{
//  sigsetmask(0);
  sigset_t s;
  sigemptyset(&s);
  sigprocmask(SIG_SETMASK,&s,NULL);
}


/* Oz version of signal(2)
   NOTE: (mm 13.10.94)
    Linux & Solaris are not POSIX compatible:
     therefor we need casts to (OsSigFun *). look at HERE.
    */
OsSigFun *osSignal(int signo, OsSigFun *fun)
{
  struct sigaction act, oact;

  /* type of act.sa_handler ist not the same on all platforms, 
   * therefore this quite intricateness */
  OsSigFun **f = (OsSigFun**)&act.sa_handler;
  *f = fun;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  /* The following peace of code is from Stevens: Advanced Programming */
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
}



/* Oz version of system(3)
 * Posix 2 requires this call not to be interuptible by a signal
 * however some OS (like Solaris 2.3) do return EINTR, so we define our
 * own one, stolen from Stevens.0
 */

int osSystem(char *cmd)
{
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

int osSelect(int nfds, fd_set *readfds, fd_set *writefds,
	     fd_set *exceptfds, struct timeval *timeout)
{
#ifdef HPUX_700
  return select(nfds, (int*)readfds,(int*)writefds,(int*)exceptfds,timeout);
#else
  return select(nfds, readfds,writefds,exceptfds,timeout);
#endif
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

int osSetAlarmTimer(int t)
{
#ifdef DEBUG_DET
//  warning("setTimer: not allowed in DEBUG_DET mode");
  return 0;
#else
  struct itimerval newT;
  struct itimerval oldT;

  newT.it_interval.tv_sec = 0;
  newT.it_interval.tv_usec = t;
  newT.it_value.tv_sec = 0;
  newT.it_value.tv_usec = t;

  if (setitimer(ITIMER_REAL,&newT,&oldT) < 0) {
    ozpwarning("setitimer");
    return -1;
  }

  return (oldT.it_value.tv_sec * 1000) + (oldT.it_value.tv_usec/1000);
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

/*
 * Sockets
 */

long openMax;
fd_set globalReadFDs;	// mask of active read FDs

int osOpenMax()
{
  int ret = sysconf(_SC_OPEN_MAX);
  if (ret == -1) {
    ret = _POSIX_OPEN_MAX;
  }
  return ret;
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

  if (osHasJobControl()) {
    /* start a new process group */
    if (setpgid(0,0)<0) {
      DebugCheckT(ozpwarning("setpgid(0,0)"));
    }
  } else {
    DebugCheckT(warning("OS does not support POSIX job control\n"));
  }
  
  DebugCheck(CLOCK_TICK < 1000, error("CLOCK_TICK must be greater than 1 ms"));

  openMax=osOpenMax();
  DebugCheckT(printf("openMax: %d\n",openMax));

  FD_ZERO(&globalReadFDs);
}

void osWatchReadFD(int fd)
{
  FD_SET(fd,&globalReadFDs);
}
void osClrWatchedReadFD(int fd)
{
  FD_CLR(fd,&globalReadFDs);
}

Bool osIsWatchedReadFD(int fd)
{
  return FD_ISSET(fd,&globalReadFDs);
}

void osBlockSelect()
{
  fd_set copyReadFDs = globalReadFDs;
  osUnblockSignals();
  int numbOfFDs = osSelect( openMax, &copyReadFDs, NULL, NULL, NULL );
  osBlockSignals();
}

/* osClearSocketErrors
     remove the closed/failed descriptors from the fd_sets
   */
void osClearSocketErrors()
{
  for (int i = 0; i < openMax; i++) {
    if (FD_ISSET(i,&globalReadFDs)) {
      fd_set copyReadFDs;
      FD_ZERO(&copyReadFDs);
      FD_SET(i,&copyReadFDs);

      struct timeval timeout;
    loop:
      timeout.tv_sec = 0;
      timeout.tv_usec = 0;

      if (osSelect(i+1,&copyReadFDs,NULL,NULL,&timeout) < 0) {
	if (errno == EINTR) {
	  goto loop;
	}
	FD_CLR(i,&globalReadFDs);
      }
    }
  }
}

Bool osTestSelect(int fd)
{
  fd_set copyReadFDs;

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  FD_ZERO(&copyReadFDs);
  FD_SET(fd,&copyReadFDs);
  if (osSelect( fd+1, &copyReadFDs, NULL, NULL, &timeout ) == 1) {
    return OK;
  }
  return NO;
}

/* -------------------------------------------------------------------------
 * subroutines for AM::handleIO
 * ------------------------------------------------------------------------- */
   
static fd_set tmpReadFDs;

/* signals are blocked */
int osFirstReadSelect()
{
  struct timeval timeout;
  int index, numbOfFDs;

 loop:
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  tmpReadFDs = globalReadFDs;

  numbOfFDs = osSelect(openMax, &tmpReadFDs, NULL, NULL, &timeout );

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

Bool osNextReadSelect(int fd)
{
  Assert(fd<openMax);
  if (FD_ISSET(fd, &tmpReadFDs)) {
    FD_CLR(fd, &tmpReadFDs);
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
  
  fd_set copyReadFDs = globalReadFDs;
  int numbOfFDs = osSelect( openMax, &copyReadFDs, NULL, NULL, &timeout );
  if (numbOfFDs < 0) {
    if (errno == EINTR) goto loop;
    if (errno != EBADF) { /* the compiler may have terminated (rs) */
      ozpwarning("checkIO: select failed");
    }
    osClearSocketErrors();
  }
  return numbOfFDs;
}


void osKillChilds()
{
  if (osHasJobControl()) {
    // terminate all our children
    OsSigFun *save=osSignal(SIGTERM,(OsSigFun*)SIG_IGN);
    if (kill(-getpid(),SIGTERM) < 0) {
      ozpwarning("kill");
    }
    osSignal(SIGTERM,save);
  }
}
