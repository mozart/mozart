/*
 *  Authors:
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

/* select(2) emulation under windows:
 *
 *    - currently only works for read fds
 *    - spawn a thread for each fd: it reads one single character and
 *      then sets event "char_avail" and waits for
 *      event "char_consumed" to continue
 *    - read(2) is wrapped by osread and reads in the rest
 *     --> WILL NOT WORK IF ONLY EXACTLY ONE CHAR IS AVAILABLE
 */



void printfds(fd_set *fds);

/* under windows FD_SET is not idempotent */
#define OZ_FD_SET(i,fds) if (!FD_ISSET(i,fds)) { FD_SET(i,fds); }
#define OZ_FD_CLR(i,fds) if (FD_ISSET(i,fds))  { FD_CLR(i,fds); }


/* abstract timeout values */
#define WAIT_NULL     (int*) -1

#ifdef WINDOWS

class IOChannel;
static void deleteReader(IOChannel *ch);


static IOChannel *channels = NULL;

typedef enum {ST_ERROR, ST_AVAIL, ST_NOTAVAIL, ST_EOF} ReadStatus;

class IOChannel {
public:
  int fd;
  HANDLE char_avail;      /* set iff char has been read*/
  HANDLE char_consumed;   /* used to restart reader thread*/
  char chr;               /* this is the char, that was read */
  ReadStatus status;
  HANDLE thrd;            /* reader thread */
  IOChannel *next;

  IOChannel(int f, IOChannel *nxt) {
    fd = f;
    chr = 0;
    status = ST_NOTAVAIL;
    thrd = 0;
    next = nxt;
    char_avail    = CreateEvent(NULL, TRUE, FALSE, NULL);
    char_consumed = CreateEvent(NULL, TRUE, FALSE, NULL);
  }

  void close()
  {
    CloseHandle(char_avail);
    CloseHandle(char_consumed);
    deleteReader(this);

    if (channels==this) {
      channels = next;
    } else {
      IOChannel *aux = channels;
      while(aux->next!=this) {
        aux = aux->next;
      }
      aux->next = aux->next->next;
    }
    delete this;
  }
};


IOChannel *findChannel(int fd)
{
  IOChannel *aux = channels;
  while(aux) {
    if (aux->fd==fd) {
      return aux;
    }
    aux = aux->next;
  }

  channels = new IOChannel(fd,channels);
  return channels;
}

IOChannel *lookupChannel(int fd)
{
  IOChannel *aux = channels;
  while(aux != NULL && aux->fd != fd) {
    aux = aux->next;
  }

  return aux;
}

DWORD __stdcall readerThread(void *arg)
{
  IOChannel *sr = (IOChannel *)arg;

  while(1) {
    sr->status=ST_NOTAVAIL;

    HANDLE hd = WrappedHandle::find(sr->fd)->hd;
    Assert(hd!=0);
    DWORD ret;
    if (ReadFile(hd,&sr->chr, sizeof(char),&ret,0)==FALSE) {
      if (ERROR_BROKEN_PIPE==GetLastError()) // mimic Unix behaviour
        ret = 0;
      else
        ret = (unsigned)-1;
    }
    if ((int)ret < 0) {
      sr->status = ST_ERROR;
      break;
    }
    if (ret==0) {
      sr->status = ST_EOF;
      break;
    }

    sr->status = ST_AVAIL;
    SetEvent(sr->char_avail);

    /* Wait until our input is acknowledged before reading again */
    if (WaitForSingleObject(sr->char_consumed, INFINITE) != WAIT_OBJECT_0) {
      sr->status = ST_ERROR;
      break;
    }
    ResetEvent(sr->char_consumed);
  }
  sr->thrd = 0;
  ExitThread(0);
  return 0;
}


static
void deleteReader(IOChannel *ch)
{
  TerminateThread((HANDLE)ch->thrd,0);
  CloseHandle((HANDLE)ch->thrd);
  ch->thrd = 0;
}

Bool createReader(int fd)
{
  IOChannel *sr = findChannel(fd);

  if (sr->thrd != 0)
    return OK;

  ResetEvent(sr->char_avail);
  ResetEvent(sr->char_consumed);

  DWORD thrid;
  sr->thrd = CreateThread(0,10000,&readerThread,sr,0,&thrid);
  if (sr->thrd != 0) {
    return OK;
  }

  int id = GetLastError();
  OZ_warning ("createReader(%d) failed: %d\n",fd,id);
  CloseHandle(sr->char_consumed);
  CloseHandle(sr->char_avail);

  return NO;
}


static
int splitFDs(fd_set *in, fd_set *out)
{
  FD_ZERO(out);

  /* hack: optimized scanning "in" by using definition of adt "fd_set" */
  int ret=0;
  fd_set copyin = *in;
  for (int i = 0; i < copyin.fd_count ; i++) {
    int fd = copyin.fd_array[i];
    if (isSocket(fd)) {
      ret++;
      OZ_FD_CLR(fd,in);
      OZ_FD_SET(fd,out);
    }
  }
  return ret;
}



static
int getAvailFDs(fd_set *rfds, fd_set *wfds)
{
  fd_set rselectfds, wselectfds;

  int numsockets = splitFDs(rfds,&rselectfds);
  numsockets    += splitFDs(wfds,&wselectfds);

  int ret = 0;
  if (numsockets>0) {
    ret += nonBlockSelect(maxSocket+1,&rselectfds,&wselectfds);
    if (ret<0) {
      //message("ret<0: %d,%d,%d\n",maxSocket+1,WSAGetLastError(),numsockets);
      //printfds(&rselectfds);
      //printfds(&socketFDs);
      return ret;
    }
  }

  IOChannel *aux = channels;
  while(aux != NULL) {
    if (FD_ISSET(aux->fd,rfds) && aux->status==ST_AVAIL) {
      OZ_FD_SET(aux->fd,&rselectfds);
      ret++;
    }
    aux = aux->next;
  }

  *rfds = rselectfds;
  *wfds = wselectfds;

  return ret;
}

class SelectInfo {
public:
  fd_set rfds, wfds;
  HANDLE event;
  int timeout;
  int timestamp;
  SelectInfo()
  {
    timeout   = 0;
    timestamp = 0;
    event     = CreateEvent(NULL, TRUE, FALSE, NULL);
  }
};


DWORD __stdcall selectThread(void *arg)
{
  SelectInfo *si = (SelectInfo *) arg;

  /* cache everything locally; a change of timestamp means: we were canceled */
  int timeout = si->timeout;
  if (timeout==INFINITE)
    timeout = 1<<30;  /* use a very long timeout */

  fd_set origrfds = si->rfds;
  fd_set origwfds = si->wfds;
  int timestamp = si->timestamp;

  while(1) {
    /* poll every 2 seconds */
    int mstowait = min(timeout,2000);
    timeout -= mstowait;

    struct timeval tv;
    tv.tv_sec  = mstowait/1000;
    tv.tv_usec = (mstowait*1000)%1000000;

    // select annulls rfds and wfds
    fd_set rfds = origrfds;
    fd_set wfds = origwfds;

    int ret = select(maxSocket+1,&rfds,&wfds,NULL,&tv);

    if (ret<0 || ret>0 || ret==0 && timeout<=0 || si->timestamp!=timestamp) {
      origrfds = rfds;
      origwfds = wfds;
      break;
    }
  }

  if (si->timestamp==timestamp) {
    si->rfds = origrfds;
    si->wfds = origwfds;
    SetEvent(si->event);
  }

  ExitThread(1);
  return 1;
}


static
int win32Select(fd_set *rfds, fd_set *wfds, unsigned int *timeout)
{
  if (timeout == (unsigned int *) WAIT_NULL)
    return getAvailFDs(rfds,wfds);

  int wait = (*timeout==0) ? INFINITE : *timeout;

  fd_set copyrfds = *rfds;
  fd_set copywfds = *wfds;

  static SelectInfo *si = NULL;
  if (si==NULL) { si = new SelectInfo(); }

  HANDLE wait_hnd[OPEN_MAX+1];
  int nh = 0;

  int numsockets = splitFDs(&copyrfds,&si->rfds);
  numsockets    += splitFDs(&copywfds,&si->wfds);

  if (numsockets > 0) {
    ResetEvent(si->event);
    wait_hnd[nh++] = si->event;
    si->timeout = wait;
    DWORD tid;
    HANDLE ret = CreateThread(NULL,10000,&selectThread,si,0,&tid);
    Assert(ret!=0);
    CloseHandle(ret);
  }

  IOChannel *aux = channels;
  while(aux != NULL) {
    if (FD_ISSET(aux->fd,&copyrfds) && aux->thrd!=0) {
      wait_hnd[nh++] = aux->char_avail;
    }
    aux = aux->next;
  }

#if 0
  static int xnh = 0;
  if (nh!=xnh) {
    message("Achanged nh: %d --> %d\n",xnh,nh);
    printfds(rfds);
    xnh = nh;
  }
#endif

  if (nh == 0) {
    if (wait==INFINITE) {
      /* wait forever */
      while(1) {
        Sleep(100000);
      }
    }
    /* Nothing to wait on, so fail */
    errno = ECHILD;
    return 0;
  }

  unsigned int startTime = getTime();
  DWORD active = WaitForMultipleObjects(nh, wait_hnd, FALSE, wait);
  si->timestamp++;  /* cancel select thread */

  if (active == WAIT_FAILED) {
    errno = EBADF;
    return -1;
  }

  if (active == WAIT_TIMEOUT) {
    *timeout = 0;
    return 0;
  }

  if (*timeout != 0) {
    unsigned int endTime = getTime();
    int resttime = wait - (endTime-startTime);
    *timeout = max(0, resttime);
  }

  return getAvailFDs(rfds,wfds);
}

#endif /* WINDOWS */

int osOpenMax();

void printfds(fd_set *fds)
{
  fprintf(stderr,"FDS: ");
  for(int i=0; i<osOpenMax(); i++) {
    if (FD_ISSET(i,fds)) {
      fprintf(stderr,"%d,",i);
    }
  }
  fprintf(stderr,"\n");
  fflush(stderr);
}
