/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
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

// Operating system stuff

#ifndef __MISCH
#define __MISCH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <signal.h>

#ifdef AIX3_RS6000
#include <sys/select.h>
#endif

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
  int getdomainname(const char *name, int namelen);

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
void osSetAlarmTimer(int t);
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
void osBlockSelect(unsigned int &ms);
void osClearSocketErrors();
int  osFirstSelect();
Bool osNextSelect(int fd, int mode);
int  osCheckIO();

void osInit();
void osExit(int status);


#ifndef SIGQUIT
#define SIGQUIT SIGINT
#endif

int oskill(int pid, int sig);


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
int osconnect(int s, struct sockaddr *addr, int namelen);
int osdup(int fd);

char *osinet_ntoa(char *);


/* check for EINTR and make sure everything is written */
int ossafewrite(int fd, char *buf, unsigned int len);
int ossaferead(int fd, char *buf, unsigned int len);

char *ostmpnam(char *s);

char *oslocalhostname();

void registerSocket(int fd);

char *osfgets(char *s, int n, FILE *stream);

#ifdef _MSC_VER
#define _hdopen(file,flags) _open_osfhandle(file,flags)
#define _os_handle(fd) _get_osfhandle(fd)
#define SIGUSR1 SIGINT
#endif

#if defined(__CYGWIN32__) || defined(__MINGW32__)
extern int _hdopen(int, int flags);
#endif

#ifdef __MINGW32__
#define SIGUSR1 SIGINT
#define O_NONBLOCK 0
#define O_NOCTTY   0
#endif


#ifdef WINDOWS
#define PathSeparator ';'
#else
#define PathSeparator ':'
#endif

TaggedRef osDlopen(char *filename, void **out);
int osDlclose(void* handle);
void *osDlsym(void *handle,const char *name);

int atomToSignal(const char *signame);

Bool osSignal(const char *signame, OZ_Term proc);
void pushSignalHandlers();

void osStackDump();

#endif

