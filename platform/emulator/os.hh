/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  Operating system stuff
  ------------------------------------------------------------------------
*/

#ifndef __MISCH
#define __MISCH

#ifdef INTERFACE
#pragma interface
#endif

#include "types.hh"

#include <stdio.h>
#include <sys/types.h>
#include <iostream.h>

#ifdef WINDOWS

#define pid_t int

// getpid
#include <process.h>

#endif

#if defined(SUNOS_SPARC) || defined(ULTRIX_MIPS)
extern "C" {

  int getitimer(int which, struct itimerval *value);
  int setitimer(int which, struct itimerval *value,struct itimerval *ovalue);

  int listen(int s, int backlog);
  int accept(int s, struct sockaddr *addr, int *addrlen);
  int bind(int s, struct sockaddr *name, int namelen);
  int socket(int domain, int type, int protocol);
  int socketpair(int domain, int type, int protocol, int sockvec[2]);
  int connect(int s, struct sockaddr *name, int namelen);
  int getsockname(int s, struct sockaddr *name, int *namelen);
  int send(int s,const char *msg, int len, int flags);
  int sendto(int s, const char *msg, int len, int flags,
	     struct sockaddr *to, int tolen);
  int sendmsg(int s,   struct msghdr *msg, int flags);
  int recv(int s, char *buf, int len, int flags);
  int recvfrom(int s, char *buf, int len, int flags,
	       struct sockaddr *from, int *fromlen);
  int shutdown(int s, int how);

  int gethostname(const char *name, int namelen);

  int select(int width,
	     fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	     struct timeval *timeout);

  int gettimeofday(struct timeval *, void *);

  /* missing prototypes from libc */
  int getopt(int argc, char *const *argv, const char *optstring);
  void bzero(char *b, int length);
  int putenv(char *buf);
}
#endif

unsigned int osSystemTime(); // return current systemtime in milliseconds
unsigned int osUserTime();   // return current usertime in milliseconds
unsigned int osTotalTime(); // return total system time in milliseconds
void osInitSignals();        // initialize signal handler
void osSetAlarmTimer(int t, Bool interval=OK);
void osBlockSignals(Bool check=NO); // check: check if no other signals are blocked
void osUnblockSignals();

int osSystem(char *cmd);     /* Oz version of system(3) */

void addChildProc(pid_t pid);


#define SEL_READ  0
#define SEL_WRITE 1

int osTestSelect(int fd, int mode);
void osWatchAccept(int fd);

void osWatchFD(int fd, int mode);
Bool osIsWatchedFD(int fd, int mode);
void osClrWatchedFD(int fd, int mode);
int osBlockSelect(int ticks);
void osClearSocketErrors();
int  osFirstSelect();
Bool osNextSelect(int fd, int mode);
int  osCheckIO();

int osOpenMax();
void osInit();
void osExit(int status);

#ifdef WINDOWS
#define oskill(pid,sig) raise(sig)
#else
#define oskill(pid,sig) kill(pid,sig)
#endif

/* abstract acess to OS file handles */
int osread(int fd, void *buf, unsigned int len);
int oswrite(int fd, void *buf, unsigned int len);
int osclose(int fd);
void ossleep(int sec);
int osgetpid();
int ossockerrno();
int osopen(const char *path, int flags, int mode);
int ossocket(int domain, int type, int protocol);
int osaccept(int s, struct sockaddr *addr, int *addrlen);
void registerSocket(int fd);

char *osfgets(char *s, int n, FILE *stream);

inline
int osMsToClockTick(int ms)
{
  int clockMS = CLOCK_TICK/1000;
  return (ms+clockMS-1) / clockMS;
}

inline
int osClockTickToMs(int cl)
{
  int clockMS = CLOCK_TICK/1000;
  return cl * clockMS;
}

#ifdef _MSC_VER
#define _hdopen(file,flags) _open_osfhandle(file,flags)
#define _os_handle(fd) _get_osfhandle(fd)
#define SIGUSR1 SIGINT
#endif

#ifdef GNUWIN32
extern int _hdopen(int, int flags);
#endif

#endif

