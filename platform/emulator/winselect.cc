/* select(2) emulation under windows:
 *
 *    - currently only works for read fds
 *    - spawn a thread for each fd: it reads one single character and 
 *      then sets event "char_avail" and waits for
 *      event "char_consumed" to continue
 *    - read(2) is wrapped by osread and reads in the rest
 *     --> WILL NOT WORK IF ONLY EXACTLY ONE CHAR IS AVAILABLE
 */


/* abstract timeout values */
#define WAIT_NULL     (int*) -1

#ifdef WINDOWS

class IOChannel {
public:
  int fd;
  HANDLE char_avail;      /* set iff char has been read*/
  HANDLE char_consumed;   /* used to restart reader thread*/
  char chr;               /* this is the char, that was read */
  Bool status;            /* true iff input is available */
  unsigned long thrd;     /* reader thread */
  IOChannel *next;

  IOChannel(int f) {
    fd = f;
    chr = 0;
    status = NO;
    thrd = 0;
    next = NULL;
    char_avail    = CreateEvent(NULL, TRUE, FALSE, NULL);
    char_consumed = CreateEvent(NULL, TRUE, FALSE, NULL);

  }
};


static IOChannel *channels = NULL;

IOChannel *findChannel(int fd)
{
  if (channels == NULL) {
    channels = new IOChannel(fd);
    return channels;
  }
  IOChannel *aux = channels;
  while(aux->next != NULL) {
    if (aux->fd==fd) {
      return aux;
    }
    aux = aux->next;
  }

  aux->next = new IOChannel(fd);
  return aux->next;
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
    sr->status=NO;
    int ret;
#ifdef EMULATOR
    if (FD_ISSET(sr->fd,&isSocket))
      ret = recv(sr->fd, &sr->chr, sizeof(char),0);
    else
#endif
      ret = read(sr->fd, &sr->chr, sizeof(char));
    if (ret<0) {
      break;
    }

    sr->status = OK;
    SetEvent(sr->char_avail);

    /* Wait until our input is acknowledged before reading again */
    if (WaitForSingleObject(sr->char_consumed, INFINITE) != WAIT_OBJECT_0)
      break;
    ResetEvent(sr->char_consumed);
  }
  sr->status = NO;
  sr->thrd = NULL;
  _endthreadex(0);
  return 0;
}

unsigned __stdcall acceptThread(void *arg)
{
  IOChannel *sr = (IOChannel *)arg;
  
  sr->status=NO;
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(sr->fd,&readfds);

  int ret = select(1,&readfds,NULL,NULL,NULL);
  if (ret<=0) {
    warning ("acceptThread(%d) failed, error=%d\n",
	    sr->fd,WSAGetLastError());
  } else {
    sr->status = OK;
    SetEvent(sr->char_avail);
  }
  sr->thrd = NULL;
  _endthreadex(0);
  return 0;
}

static 
void deleteReader(int fd)
{
  TerminateThread((HANDLE)findChannel(fd)->thrd,0);
  findChannel(fd)->thrd = 0;
}

Bool createReader(int fd, Bool doAcceptSelect)
{
  IOChannel *sr = findChannel(fd);

  if (sr->thrd != NULL)
    return OK;

  ResetEvent(sr->char_avail);
  ResetEvent(sr->char_consumed);

  unsigned thrid;
  sr->thrd = _beginthreadex(NULL,0,
			    doAcceptSelect ? &acceptThread : &readerThread,
			    sr,0,&thrid);
  if (sr->thrd != NULL) {
    return OK;
  }

  int id = GetLastError();
  warning ("createReader(%d) failed: %d\n",fd,id);
  CloseHandle(sr->char_consumed);
  CloseHandle(sr->char_avail);
  
  return NO;
}


static 
int getAvailFDs(int nfds, fd_set *readfds)
{
  int ret=0, i;
  for (i=0; i<nfds; i++) {
    if (FD_ISSET(i,readfds)) {
      if (findChannel(i)->status==OK) {
	FD_SET(i,readfds);
	ret++;
      } else {
	FD_CLR(i,readfds);
      }
    }
  }
  return ret;
}


static
int win32Select(int maxfd, fd_set *fds, int *timeout)
{
  if (timeout == WAIT_NULL)
    return getAvailFDs(maxfd,fds);
  
  int wait = (*timeout==0) ? INFINITE : *timeout;

  HANDLE wait_hnd[OPEN_MAX+FD_SETSIZE];
  
  int nh = 0;
  int i;
  for (i=0; i< maxfd; i++) {
    if (FD_ISSET(i,fds) && findChannel(i)->thrd!=NULL) {
      wait_hnd[nh++] = findChannel(i)->char_avail;
    }
  }

  if (nh == 0) {
    /* Nothing to wait on, so fail */
    errno = ECHILD;
    return -1;
  }
  
  DWORD startTime = timeGetTime();
  DWORD active = WaitForMultipleObjects(nh, wait_hnd, FALSE, wait);

  if (active == WAIT_FAILED) {
    errno = EBADF;
    return -1;
  } 

  if (active == WAIT_TIMEOUT) {
    *timeout = 0;
    return 0;
  }
  
  if (*timeout != 0) {
    DWORD endTime = timeGetTime();
    int resttime = wait - (endTime-startTime);
    *timeout = max(0, resttime);
  }

  return getAvailFDs(maxfd, fds);
}

#endif /* WINDOWS */
