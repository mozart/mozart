/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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


#include "wsock.hh"
#include "base.hh"

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <signal.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef AIX3_RS6000
#include <sys/select.h>
#endif

#ifdef WINDOWS

#define pid_t int

// getpid
#include <process.h>

#ifdef _MSC_VER
#include <io.h>
#include <sys/stat.h>

#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

/* Some defines for _access nAccessMode (MS doesn't define them, but
 * it doesn't seem to hurt to add them). */
#define F_OK    0       /* Check for file existence */
#define W_OK    2       /* Check for write permission */
#define R_OK    4       /* Check for read permission */

#define _S_IRWXU        (_S_IREAD | _S_IWRITE | _S_IEXEC)
#define _S_IXUSR        _S_IEXEC
#define _S_IWUSR        _S_IWRITE
#define _S_IRUSR        _S_IREAD

#define S_IFIFO         _S_IFIFO
#define S_IFCHR         _S_IFCHR
#define S_IFBLK         _S_IFBLK
#define S_IFDIR         _S_IFDIR
#define S_IFREG         _S_IFREG
#define S_IFMT          _S_IFMT
#define S_IEXEC         _S_IEXEC
#define S_IWRITE        _S_IWRITE
#define S_IREAD         _S_IREAD
#define S_IRWXU         _S_IRWXU
#define S_IXUSR         _S_IXUSR
#define S_IWUSR         _S_IWUSR
#define S_IRUSR         _S_IRUSR

#define S_ISDIR(m)      ((m) & S_IFDIR)
#define S_ISFIFO(m)     ((m) & S_IFIFO)
#define S_ISCHR(m)      ((m) & S_IFCHR)
#define S_ISBLK(m)      ((m) & S_IFBLK)
#define S_ISREG(m)      ((m) & S_IFREG)


#define lseek(x,y,z) _lseek(x,y,z)

/* the following is stolen from mingw32 */
struct dirent
{
        long            d_ino;          /* Always zero. */
        unsigned short  d_reclen;       /* Always zero. */
        unsigned short  d_namlen;       /* Length of name in d_name. */
        char*           d_name;         /* File name. */
        /* NOTE: The name in the dirent structure points to the name in the
         *       finddata_t structure in the DIR. */
};


typedef struct
{
        /* disk transfer area for this dir */
        struct _finddata_t      dd_dta;

        /* dirent struct to return from dir (NOTE: this makes this thread
         * safe as long as only one thread uses a particular DIR struct at
         * a time) */
        struct dirent           dd_dir;

        /* _findnext handle */
        long                    dd_handle;

        /*
         * Status of search:
         *   0 = not started yet (next entry to read is first entry)
         *  -1 = off the end
         *   positive = 0 based index of next entry
         */
        short                   dd_stat;

        /* given path for dir with search pattern (struct is extended) */
        char                    dd_name[1];
} DIR;


DIR*            opendir (const char* szPath);
struct dirent*  readdir (DIR* dir);
int             closedir (DIR* dir);

#endif


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


int oskill(int pid, int sig);


/* abstract acess to OS file handles */
int osread(int fd, void *buf, unsigned int len);
int oswrite(int fd, void *buf, unsigned int len);
int osclose(int fd);
void ossleep(int sec);
int osgetpid();
int osgetEpid();
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
char *osgetenv(char *var);

char *oslocalhostname();

void registerSocket(int fd);

char *osfgets(char *s, int n, FILE *stream);

#ifdef WINDOWS

void createReader(SOCKET s,HANDLE h);
void createWriter(SOCKET s,HANDLE h);

int ossocketpair(int, int type, int, int *sb);
#define O_NONBLOCK 0
#define O_NOCTTY   0
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

