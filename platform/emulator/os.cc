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

#if defined(INTERFACE) && !defined(PEANUTS)
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

#ifdef FOPEN_MAX
#define OPEN_MAX  FOPEN_MAX
#endif

#ifdef MAX_OPEN
#define OPEN_MAX  MAX_OPEN
#endif

#ifdef WINDOWS
#include <time.h>
#include <process.h>
extern "C" int _fmode;

#else
#include <sys/times.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#endif

#if !defined(ultrix) && !defined(WINDOWS)
# include <sys/socket.h>
#endif

#ifdef AIX3_RS6000
#include <sys/select.h>
#endif

#include "tagged.hh"
#include "os.hh"


fd_set isSocket;
static long openMax;

#ifdef WINDOWS
int runningUnderNT = 0;

unsigned long fileTimeToMS(FILETIME *ft);
int getTime();

#endif


// return current usertime in milliseconds
unsigned int osUserTime()
{
#if defined(WINDOWS)
  if (!runningUnderNT) {
    return getTime();
  }

  /* only NT supports this */
  FILETIME ct,et,kt,ut;
  GetProcessTimes(GetCurrentProcess(),&ct,&et,&kt,&ut);
  return fileTimeToMS(&ut);

#else
  struct tms buffer;

  times(&buffer);
  return (unsigned int)(buffer.tms_utime*1000.0/(double)sysconf(_SC_CLK_TCK));
#endif
}

// return current systemtime in milliseconds
unsigned int osSystemTime()
{
#if defined(WINDOWS)
  if (!runningUnderNT) {
    return 0;
  }

  /* only NT supports this */
  FILETIME ct,et,kt,ut;
  GetProcessTimes(GetCurrentProcess(),&ct,&et,&kt,&ut);
  return fileTimeToMS(&kt);

#else
  struct tms buffer;

  times(&buffer);
  return (unsigned int)(buffer.tms_stime*1000.0/(double)sysconf(_SC_CLK_TCK));
#endif
}



#ifdef WINDOWS
class TimerThread {
public:
  HANDLE thrd;
  Bool die;
  int wait;
  TimerThread(int w);
};


static TimerThread *timerthread = NULL;

#endif


void osBlockSignals(Bool check)
{
#ifdef WINDOWS
  if (timerthread)
    SuspendThread(timerthread->thrd);
#else
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
#ifdef WINDOWS
  if (timerthread)
    ResumeThread(timerthread->thrd);
#else
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

typedef void OsSigFun(void);

static
OsSigFun *osSignal(int signo, OsSigFun *fun)
{
#ifdef WINDOWS
  signal(signo,(_sig_func_ptr)fun);
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


#ifdef GNUWIN32

const int wrappedHDStart = 30; /* !!!!!!!!!!!!! */

class WrappedHandles {
public:
  static int nextno;
  static WrappedHandles *allHandles;
  HANDLE hd;
  int fd;
  WrappedHandles *next;
  WrappedHandles(HANDLE h)
  {
    hd = h;
    fd = nextno++;
    next = allHandles;
    allHandles = this;
  }

  static HANDLE find(int f)
  {
    WrappedHandles *aux = allHandles;
    while(aux) {
      if (f == aux->fd)
        return aux->hd;
      aux = aux->next;
    }
    return 0;
  }
};

int WrappedHandles::nextno = wrappedHDStart;

WrappedHandles *WrappedHandles::allHandles = NULL;

int lowread(int fd, void *buf, int sz)
{
  if (fd < wrappedHDStart)
    return read(fd,buf,sz);

  HANDLE hd = WrappedHandles::find(fd);
  Assert(hd!=0);

  unsigned int ret;
  if (ReadFile(hd,buf,sz,&ret,0)==FALSE)
    return -1;
  return ret;
}


int lowwrite(int fd, void *buf, int sz)
{
  if (fd < wrappedHDStart)
    return write(fd,buf,sz);

  HANDLE hd = WrappedHandles::find(fd);
  Assert(hd!=0);

  unsigned int ret;
  if (WriteFile(hd,buf,sz,&ret,0)==FALSE)
    return -1;
  return ret;
}

int _hdopen(int handle, int flags)
{
  WrappedHandles *wh = new WrappedHandles((HANDLE)handle);
  return wh->fd;
}

#else

#define lowread(fd,buf,sz) read(fd,buf,sz)
#define lowwrite(fd,buf,sz) write(fd,buf,sz)

#endif



#define EMULATOR
#include "winselect.cc"
#undef EMULATOR


#ifdef WINDOWS

unsigned __stdcall timerFun(void *p)
{
  TimerThread *ti = (TimerThread*) p;
  while(1) {
    Sleep(ti->wait);
    if (ti->die)
      break;
    handlerALRM();
  }
  delete ti;
  _endthreadex(1);
  return 1;
}

TimerThread::TimerThread(int w)
{
  wait = w;
  die = NO;
  unsigned tid;
  thrd = (HANDLE) _beginthreadex(NULL,0,&timerFun,this,0,&tid);
  if (thrd==NULL) {
    ozpwarning("osSetAlarmTimer(start thread)");
  }
}
#endif


void osSetAlarmTimer(int t, Bool interval)
{
#ifdef DEBUG_DET
  if (interval==OK)
    return;
#endif

#ifdef WINDOWS

  Assert(t>0 && interval==OK);
  if (timerthread==NULL) {
    unsigned tid;
    timerthread = new TimerThread(t);
  }
  //  timerthread->die = OK;
  timerthread->wait = t;
  ResumeThread(timerthread->thrd);
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

#ifndef WINDOWS
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
#endif


/* wait *timeout msecs on given fds
 * return number of fds ready and return in *timeout msecs left
 */
static
int osSelect(int nfds, fd_set *readfds, fd_set *writefds, int *timeout)
{
#ifdef WINDOWS

  return win32Select(readfds,timeout);

#else

  struct timeval timeoutstruct, *timeoutptr;
  if (timeout == WAIT_NULL) {
    timeoutstruct.tv_sec = 0;
    timeoutstruct.tv_usec = 0;
    timeoutptr = &timeoutstruct;
  } else {
    timeoutptr=NULL;
    osSetAlarmTimer(*timeout,NO);
    osUnblockSignals();
  }

/* The prototypes for select are wrong on HP-UX 9.x */
#ifdef HPUX_700
  int ret = select(nfds,(int*)readfds,(int*)writefds,NULL,timeoutptr);
#else
  int ret = select(nfds,readfds,writefds,NULL,timeoutptr);
#endif

  if (timeout!=WAIT_NULL) {
    *timeout = osGetAlarmTimer();
    osBlockSignals();
  }
  return ret;
#endif  /* WINDOWS */
}

void osInitSignals()
{
#ifndef WINDOWS
  osSignal(SIGALRM,handlerALRM);
#endif
#ifndef DEBUG_DET
  osSignal(SIGINT,handlerINT);
  osSignal(SIGTERM,handlerTERM);
  osSignal(SIGSEGV,handlerSEGV);
  osSignal(SIGUSR1,handlerUSR1);
  osSignal(SIGFPE,handlerFPE);
#ifndef WINDOWS
  osSignal(SIGBUS,handlerBUS);
  osSignal(SIGPIPE,handlerPIPE);
  osSignal(SIGCHLD,handlerCHLD);
#endif
#endif
}


/*********************************************************
 *       Sockets                                         *
 *********************************************************/

static fd_set globalFDs[2];     // mask of active read/write FDs

int osOpenMax()
{
#ifdef WINDOWS
  return OPEN_MAX+FD_SETSIZE;
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

  DebugCheck(CLOCK_TICK < 1000, error("CLOCK_TICK must be greater than 1 ms"));

  openMax=osOpenMax();
  DebugCheckT(printf("openMax: %d\n",openMax));

  FD_ZERO(&globalFDs[SEL_READ]);
  FD_ZERO(&globalFDs[SEL_WRITE]);

  FD_ZERO(&isSocket);
#ifdef WINDOWS
  OSVERSIONINFO vi;
  vi.dwOSVersionInfoSize = sizeof(vi);
  BOOL b = GetVersionExW(&vi);
  runningUnderNT = (vi.dwPlatformId==VER_PLATFORM_WIN32_NT);
  /* make sure everything is opened in binary mode */
  setmode(fileno(stdin),O_BINARY);  // otherwise input blocks!!
  _fmode = O_BINARY;

#endif
}

#define CheckMode(mode) Assert(mode==SEL_READ || mode==SEL_WRITE)

static
void osWatchFDInternal(int fd, int mode, Bool watchAccept)
{
  CheckMode(mode);
  FD_SET(fd,&globalFDs[mode]);
#ifdef WINDOWS
  Assert(mode==SEL_READ);
  createReader(fd,watchAccept);
#endif
}


void osWatchFD(int fd, int mode)
{
  osWatchFDInternal(fd,mode,NO);
}

void osWatchAccept(int fd)
{
  osWatchFDInternal(fd,SEL_READ,OK);
}


void osClrWatchedFD(int fd, int mode)
{
  CheckMode(mode);
  FD_CLR(fd,&globalFDs[mode]);
#ifdef WINDOWS
  // Assert(mode==SEL_READ);
  // deleteReader(fd);
#endif
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
  int ret = osSelect(openMax,&copyFDs[SEL_READ],&copyFDs[SEL_WRITE],&wait);
  int ticksleft = osMsToClockTick(wait);
  return ticksleft;
}

/* osClearSocketErrors
 * remove the closed/failed descriptors from the fd_sets
 */
void osClearSocketErrors()
{
  for (int i = 0; i < openMax; i++) {
    for(int mode=SEL_READ; mode <= SEL_WRITE; mode++) {
      if (FD_ISSET(i,&globalFDs[mode]) && osTestSelect(i,mode) < 0) {
        osClrWatchedFD(i,mode);
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
#ifdef WINDOWS
  /* no select on write */
  if (mode==SEL_WRITE)
    return 1;

  /* for a disk file hopefully input will not block */
  IOChannel *ch = findChannel(fd);
  //  fprintf(stderr,"before GFT(%d,%d)\n",h,fd); fflush(stderr);
  if (!FD_ISSET(fd,&isSocket) && ch == NULL) {
    //    fprintf(stderr,"after GFT(%d), success\n",h); fflush(stderr);
    return 1;
  }

  //  fprintf(stderr,"after GFT(%d), fail\n",h); fflush(stderr);
  return ch->status==OK;
#else
  while(1) {
    fd_set fdset, *readFDs=NULL, *writeFDs=NULL;
    FD_ZERO(&fdset);
    FD_SET(fd,&fdset);

    if (mode==SEL_READ) {
      readFDs = &fdset;
    } else {
      writeFDs = &fdset;
    }
    int ret = osSelect(fd+1, readFDs, writeFDs, WAIT_NULL);
    if (ret >= 0 || errno != EINTR) {
      return ret;
    }
  }
#endif
}

/* -------------------------------------------------------------------------
 * subroutines for AM::handleIO
 * ------------------------------------------------------------------------- */

static fd_set tmpFDs[2];

/* signals are blocked */
int osFirstSelect()
{
 loop:
  tmpFDs[SEL_READ]  = globalFDs[SEL_READ];
  tmpFDs[SEL_WRITE] = globalFDs[SEL_WRITE];

  int numbOfFDs = osSelect(openMax,&tmpFDs[SEL_READ],&tmpFDs[SEL_WRITE], WAIT_NULL);

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
#ifndef WINDOWS
  /* socket numbers may grow larger on win32 */
  Assert(fd<openMax);
#endif
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
  fd_set copyFDs[2];
  copyFDs[SEL_READ]  = globalFDs[SEL_READ];
  copyFDs[SEL_WRITE] = globalFDs[SEL_WRITE];

  int numbOfFDs = osSelect(openMax,&copyFDs[SEL_READ],&copyFDs[SEL_WRITE],WAIT_NULL);
  if (numbOfFDs < 0) {
    if (errno == EINTR) goto loop;
    if (errno != EBADF) { /* the compiler may have terminated (rs) */
      ozpwarning("checkIO: select failed");
    }
    osClearSocketErrors();
  }
  return numbOfFDs;
}





/*
 *  we want to kill all our children on exit *
 */


class ChildProc {
public:
  pid_t pid;
  ChildProc *next;
  ChildProc(pid_t p, ChildProc *nxt) : next(nxt), pid(p) {}
  static ChildProc *allchildren;
  static void add(pid_t p)
  {
    allchildren = new ChildProc(p,allchildren);
  }
};

ChildProc *ChildProc::allchildren = NULL;


void addChildProc(pid_t pid)
{
  ChildProc::add(pid);
}


void osExit(int status)
{
  /* terminate all our children */
  ChildProc *aux = ChildProc::allchildren;
  while(aux) {
#ifdef WINDOWS
    TerminateProcess((HANDLE)aux->pid,0);
#else
    int ret = oskill(aux->pid,SIGTERM);
    /* ignore return value */
#endif
    aux = aux->next;
  }

#ifdef WINDOWS
  ExitProcess(status);
#else
  exit(status);
#endif
}



void registerSocket(int fd)
{
  FD_SET(fd,&isSocket);
}


int osread(int fd, void *buf, unsigned int len)
{
#ifdef WINDOWS
  IOChannel *ch = lookupChannel(fd);
  if (ch!=NULL && ch->thrd) {
    WaitForSingleObject(ch->char_avail, INFINITE);

    *(char*)buf = ch->chr;
    int ret;
      if (FD_ISSET(fd,&isSocket))
        ret = recv(fd,((char*)buf)+1,len-1,0);
      else {
        ret = lowread(fd,((char*)buf)+1,len-1);
      }
    if (ret<0)
      return ret;
    ch->status=NO;
    ResetEvent(ch->char_avail);
    SetEvent(ch->char_consumed);
    return ret+1;
  }
#endif
  return lowread(fd, buf, len);
}


/* currently no wrapping for write */
int oswrite(int fd, void *buf, unsigned int len)
{
  if (FD_ISSET(fd,&isSocket))
    return send(fd, (char *)buf, len, 0);
  return lowwrite(fd, buf, len);
}

int osclose(int fd)
{
#ifdef WINDOWS
  if (FD_ISSET(fd,&isSocket))
    return closesocket(fd);
#endif
  FD_CLR(fd,&isSocket);
  return close(fd);
}

int osopen(const char *path, int flags, int mode)
{
  int ret = open(path,flags,mode);
  if (ret >= 0)
    FD_CLR(ret,&isSocket);
  return ret;
}

int ossocket(int domain, int type, int protocol)
{
  int ret = socket(domain,type,protocol);
  if (ret >= 0)
    FD_SET(ret,&isSocket);
  return ret;
}

void ossleep(int secs)
{
#ifdef WINDOWS
  Sleep(secs*1000);
#else
  sleep(secs);
#endif
}

int osgetpid()
{
  return getpid();
}
