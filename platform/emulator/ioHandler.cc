/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *
 *  Copyright:
 *    Michael Mehl (1997)
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

#include "value.hh"
#include "os.hh"
#include "am.hh"

// LATENCY DELAY should, if used, be set to number of microseconds to delay

#ifdef LATENCY_DELAY
#include <sys/time.h>

static timezone tz = {0,0};

// x - y = diff (in microseconds)
long difftime(struct timeval*  x, struct timeval* y){
  return (x->tv_usec - y->tv_usec + (x->tv_sec - y->tv_sec) * 1000000);
}

#endif

class IONode {
private:
  Bool isprotected[2];
public:
  int fd;
  OZ_IOHandler handler[2];
  OZ_IOHandler hSusp[2];
#ifdef LATENCY_DELAY
  // ZACHARIAS: for introducing the delay directly on the IONode, we only need it on the write socket though
  timeval offset[2];
#endif
  void *readwritepair[2];
  IONode *next;
  IONode(int f, IONode *nxt): fd(f), next(nxt) {
    isprotected[0] = isprotected[1] = NO;
    handler[0] = handler[1] = 0;
    hSusp[0] = hSusp[1] = 0;
    readwritepair[0] = readwritepair[1] = 0;
  }
  void protect(int mode) {
    if (!isprotected[mode]) {
      isprotected[mode] = OK;
      oz_protect((TaggedRef *)&readwritepair[mode]);
    }
  }
  void unprotect(int mode) {
    if (isprotected[mode]) {
      isprotected[mode] = NO;
      oz_unprotect((TaggedRef *)&readwritepair[mode]);
    }
  }
};

static
IONode *ioNodes = NULL;

static
IONode *findIONode(int fd)
{
  IONode *aux = ioNodes;
  while(aux) {
    if (aux->fd == fd) return aux;
    aux = aux->next;
  }
  ioNodes = new IONode(fd,ioNodes);
  return ioNodes;
}

void oz_io_select(int fd, int mode, OZ_IOHandler fun, void *val)
{
  if (!oz_onToplevel()) {
    OZ_warning("select only on toplevel");
    return;
  }
  IONode *ion = findIONode(fd);
  ion->readwritepair[mode]=val;
  ion->handler[mode]=fun;
#ifdef LATENCY_DELAY
  gettimeofday(&(ion->offset[mode]),&tz);
#endif

  osWatchFD(fd,mode);
}


void oz_io_acceptSelect(int fd, OZ_IOHandler fun, void *val)
{
  if (!oz_onToplevel()) {
    OZ_warning("select only on toplevel");
    return;
  }

  IONode *ion = findIONode(fd);
  ion->readwritepair[SEL_READ]=val;
  ion->handler[SEL_READ]=fun;
  osWatchAccept(fd);
}

void oz_io_awakeVar(TaggedRef var)
{
  Assert(oz_onToplevel());
  Assert(oz_isCons(var));

  OZ_unifyInThread(OZ_head(var),OZ_tail(var));
}

static
int oz_io_awake(int, void *var)
{
  oz_io_awakeVar((TaggedRef) var);
  return 1;
}

//  The 'l' and 'r' terms are unified at earliest during the next i/o
//  handling, even if there is some data available immediately.
int oz_io_select(int fd, int mode, TaggedRef l, TaggedRef r)
{
  if (!oz_onToplevel()) {
    OZ_warning("select only on toplevel");
    return OK;
  }
  IONode *ion = findIONode(fd);
  ion->readwritepair[mode]=(void *) oz_cons(l,r);
  ion->protect(mode);
#ifdef LATENCY_DELAY
  gettimeofday(&(ion->offset[mode]),&tz);
#endif

  ion->handler[mode]=oz_io_awake;
  osWatchFD(fd,mode);
  return OK;
}

void oz_io_acceptSelect(int fd,TaggedRef l,TaggedRef r)
{
  if (!oz_onToplevel()) {
    OZ_warning("acceptSelect only on toplevel");
    return;
  }

  IONode *ion = findIONode(fd);
  ion->readwritepair[SEL_READ]=(void *) oz_cons(l,r);
  ion->protect(SEL_READ);

  ion->handler[SEL_READ]=oz_io_awake;
  osWatchAccept(fd);
}

void oz_io_deSelect(int fd,int mode)
{
  osClrWatchedFD(fd,mode);
  IONode *ion = findIONode(fd);
  ion->readwritepair[mode]  = 0;
  ion->unprotect(mode);
  ion->handler[mode]  = 0;
}

void oz_io_deSelect(int fd)
{
  oz_io_deSelect(fd,SEL_READ);
  oz_io_deSelect(fd,SEL_WRITE);
}

//
void oz_io_suspend(int fd, int mode)
{
  osClrWatchedFD(fd, mode);
  IONode *ion = findIONode(fd);
  Assert(ion->hSusp[mode] == 0);
  ion->hSusp[mode] = ion->handler[mode];
  ion->handler[mode] = 0;
}

void oz_io_resume(int fd, int mode)
{
  osWatchFD(fd, mode);
  IONode *ion = findIONode(fd);
  Assert(ion->handler[mode] == 0);
  ion->handler[mode] = ion->hSusp[mode];
  ion->hSusp[mode] = 0;
}

#ifdef DENYS_EVENTS
static int io_event_sent = 0;
#endif

// called if IOReady (signals are blocked)
void oz_io_handle()
#ifdef DENYS_EVENTS
{
  if (io_event_sent) return;
  {
    static TaggedRef io = oz_atom("io");
    OZ_eventPush(io);
    io_event_sent = 1;
  }
}
OZ_BI_define(io_handler,1,0)
#endif
{
  am.unsetSFlag(IOReady);
#ifdef DENYS_EVENTS
  io_event_sent = 0;
#endif
  int numbOfFDs = osFirstSelect();

#ifdef LATENCY_DELAY
  timeval now;
  gettimeofday(&now,&tz);
#endif

  // find the nodes to awake
  for (int index = 0; numbOfFDs > 0; index++) {

    //for(int mode=SEL_READ; mode <= SEL_WRITE; mode++) {
    // }

    if (osNextSelect(index, SEL_READ) ) {

      numbOfFDs--;

      IONode *ion = findIONode(index);
      if ((ion->handler[SEL_READ]) &&  /* Perdio: handlers may do a deselect for other fds*/
          (ion->handler[SEL_READ])(index, ion->readwritepair[SEL_READ])) {
        ion->readwritepair[SEL_READ] = 0;
        ion->unprotect(SEL_READ);
        ion->handler[SEL_READ] = 0;
        osClrWatchedFD(index,SEL_READ);
      }
    }

    if (osNextSelect(index, SEL_WRITE) ) {

      numbOfFDs--;

      IONode *ion = findIONode(index);
      if ((ion->handler[SEL_WRITE]) &&  /* Perdio: handlers may do a deselect for other fds*/
#ifdef LATENCY_DELAY
          (difftime(&now, &(ion->offset[SEL_WRITE])) > LATENCY_DELAY) &&
#endif
          (ion->handler[SEL_WRITE])(index, ion->readwritepair[SEL_WRITE])) {
        ion->readwritepair[SEL_WRITE] = 0;
        ion->unprotect(SEL_WRITE);
        ion->handler[SEL_WRITE] = 0;
        osClrWatchedFD(index,SEL_WRITE);
      }
    }

  }
#ifdef DENYS_EVENTS
  return PROCEED;
}
OZ_BI_end
#else
}
#endif

//
// is NOT called from a real signal handler, but from
// 'checkStatus()' (and from 'suspendEngine()').
void oz_io_check()
{
#ifdef DENYS_EVENTS
  // kost@ : won't work in the new model, when 'oz_io_handle()'
  // doesn't call 'osSelect()' at all?
  if (am.isSetSFlag(IOReady)) return;
#endif
  if (osCheckIO() > 0)
    am.setSFlag(IOReady);
}

//
// kost@ : when purging borrow entries during shutdown, we want to
// make sure that no incomming messages are processed (since they can
// refer entries just purged);
void oz_io_stopReadingOnShutdown()
{
  IONode *aux = ioNodes;
  while (aux) {
    oz_io_suspend(aux->fd, SEL_READ);
    aux = aux->next;
  }
}

//
int oz_io_numOfSelected()
{
  int num = 0;
  IONode *aux = ioNodes;
  while (aux) {
    if (aux->handler[SEL_READ] || aux->handler[SEL_WRITE])
      num++;
    aux = aux->next;
  }
  return (num);
}
