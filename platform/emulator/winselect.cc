/* select(2) emulation under windows:
 *
 *    - currently only works for read fds
 *    - spawn a thread for each fd: it reads one single character and 
 *      then sets event "char_avail" and waits for
 *      event "char_consumed" to continue
 *    - read(2) is wrapped by osread and reads in the rest
 *     --> WILL NOT WORK IF ONLY EXACTLY ONE CHAR IS AVAILABLE
 */


#ifndef EMULATOR
#define maxSocket OPEN_MAX
#define rawread(fd,buf,sz) read(fd,buf,sz)
#define isSocket(fd) 0
#define Assert(x)
#define getTime() 0
#endif


/* under windows FD_SET is not idempotent */
#define OZ_FD_SET(i,fds) if (!FD_ISSET(i,fds)) { FD_SET(i,fds); }


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

unsigned __stdcall readerThread(void *arg)
{
  IOChannel *sr = (IOChannel *)arg;

  while(1) {
    sr->status=ST_NOTAVAIL;
    int ret = rawread(sr->fd, &sr->chr, sizeof(char));
    if (ret < 0) {
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
  _endthreadex(0);
  return 0;
}


static 
void deleteReader(IOChannel *ch)
{
  TerminateThread((HANDLE)ch->thrd,0);
  ch->thrd = 0;
}

static int maxfd = 0; /* highest fd for which we ever created a reader */

Bool createReader(int fd)
{
  IOChannel *sr = findChannel(fd);

  if (sr->thrd != 0)
    return OK;

  ResetEvent(sr->char_avail);
  ResetEvent(sr->char_consumed);

  unsigned thrid;
  sr->thrd = (HANDLE) _beginthreadex(0,0,&readerThread,sr,0,&thrid);
  if (sr->thrd != 0) {
    maxfd = max(fd+1,maxfd);
    return OK;
  }

  int id = GetLastError();
  warning ("createReader(%d) failed: %d\n",fd,id);
  CloseHandle(sr->char_consumed);
  CloseHandle(sr->char_avail);
  
  return NO;
}


static
int splitFDs(int nfds, fd_set *in, fd_set *out)
{
  FD_ZERO(out);

  int ret=0;
  for (int i=0; i<nfds; i++) {
    if (isSocket(i) && FD_ISSET(i,in)) {
      ret++;
      FD_CLR(i,in);
      OZ_FD_SET(i,out);
    }
  }
  return ret;
}



static
void orFDs(int nfds, fd_set *out, fd_set *other)
{
  for (int i=0; i<nfds; i++) {
    if (FD_ISSET(i,other)) {
      OZ_FD_SET(i,out);
    }
  }
}

static 
int getAvailFDs(fd_set *rfds, fd_set *wfds)
{
  int nfds = max(maxSocket,maxfd)+1;

  int ret=0;

#ifdef EMULATOR
  fd_set rselectfds;
  fd_set wselectfds;
  int numsockets = splitFDs(nfds,rfds,&rselectfds);
  numsockets    += splitFDs(nfds,wfds,&wselectfds);

  if (numsockets>0) {
    ret += nonBlockSelect(nfds,&rselectfds,&wselectfds);
  }
#endif

  for (int i=0; i<maxfd; i++) {
    if (FD_ISSET(i,rfds)) {
      if (lookupChannel(i)->status==ST_AVAIL) {
	ret++;
      } else {
	FD_CLR(i,rfds);
      }
    }
  }

#ifdef EMULATOR
  if (numsockets>0) {
    orFDs(nfds,rfds,&rselectfds);
    orFDs(nfds,wfds,&wselectfds);
  }
#endif

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


unsigned __stdcall selectThread(void *arg)
{
  SelectInfo *si = (SelectInfo *) arg;

  /* cache everything locally; a change of timestamp means: we werwe canceled */
  int timeout = si->timeout;
  if (timeout==INFINITE)
    timeout = 1<<30;  /* use a very long timeout */

  fd_set rfds = si->rfds;
  fd_set wfds = si->wfds;
  int timestamp = si->timestamp;

  while(1) {
    /* poll every second */
    int mstowait = (timeout > 1000) ? 1000 : timeout;
    timeout -= mstowait;

    struct timeval tv;
    tv.tv_sec  = mstowait/1000;
    tv.tv_usec = (mstowait*1000)%1000000;

    int ret = select(maxSocket,&rfds,&wfds,NULL,&tv);
    if (ret<0 || ret>0 || ret==0 && si->timeout<=0 || si->timestamp!=timestamp)
      break;
  }
  
  if (si->timestamp==timestamp) {
    si->rfds = rfds;
    si->wfds = wfds;    
    SetEvent(si->event);  
  }
  _endthreadex(1);
  return 1;
}


static
int win32Select(fd_set *rfds, fd_set *wfds, int *timeout)
{
  if (timeout == WAIT_NULL)
    return getAvailFDs(rfds,wfds);
  
  int wait = (*timeout==0) ? INFINITE : *timeout;

  int nfds = max(maxSocket,maxfd)+1;

  fd_set copyrfds = *rfds;
  fd_set copywfds = *wfds;

  static SelectInfo *si = NULL;
  if (si==NULL) { si = new SelectInfo(); }

  HANDLE wait_hnd[OPEN_MAX+1];
  int nh = 0;

#ifdef EMULATOR
  int numsockets = splitFDs(nfds,&copyrfds,&si->rfds);
  numsockets    += splitFDs(nfds,&copywfds,&si->wfds);

  if (numsockets > 0) {
    ResetEvent(si->event);
    wait_hnd[nh++] = si->event;
    unsigned tid;
    HANDLE ret = _beginthreadex(NULL,0,&selectThread,si,0,&tid);
    Assert(ret!=0);
  }
#endif
  
  int i;
  for (i=0; i < maxfd; i++) {
    IOChannel *ch = lookupChannel(i);
    if (FD_ISSET(i,&copyrfds) && ch && ch->thrd!=0) {
      wait_hnd[nh++] = ch->char_avail;
    }
  }

  if (nh == 0) {
    /* Nothing to wait on, so fail */
    errno = ECHILD;
    return -1;
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

void printfds(fd_set *fds)
{
  printf("FDS: ");
  for(int i=0; i<50; i++) {
    if (FD_ISSET(i,fds)) {
      printf("%d,",i);
    }
  }
  printf("\n");
}
