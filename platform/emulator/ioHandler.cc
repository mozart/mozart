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

class IONode {
private:
  Bool isprotected[2];  // whether
public:
  int fd;
  OZ_IOHandler handler[2];
  void *readwritepair[2];
  IONode *next;
  IONode(int f, IONode *nxt): fd(f), next(nxt) {
    isprotected[0] = isprotected[1] = NO;
    handler[0] = handler[1] = 0;
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

int oz_io_select(int fd, int mode,TaggedRef l,TaggedRef r)
{
  if (!oz_onToplevel()) {
    OZ_warning("select only on toplevel");
    return OK;
  }
  if (osTestSelect(fd,mode)==1) {
    OZ_unifyInThread(l,r);
    return OK;
  }
  IONode *ion = findIONode(fd);
  ion->readwritepair[mode]=(void *) oz_cons(l,r);
  ion->protect(mode);

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

// called if IOReady (signals are blocked)
void oz_io_handle()
{
  am.unsetSFlag(IOReady);
  int numbOfFDs = osFirstSelect();

  // find the nodes to awake
  for (int index = 0; numbOfFDs > 0; index++) {
    for(int mode=SEL_READ; mode <= SEL_WRITE; mode++) {

      if (osNextSelect(index, mode) ) {

        numbOfFDs--;

        IONode *ion = findIONode(index);
        if ((ion->handler[mode]) &&  /* Perdio: handlers may do a deselect for other fds*/
            (ion->handler[mode])(index, ion->readwritepair[mode])) {
          ion->readwritepair[mode] = 0;
          ion->unprotect(mode);
          ion->handler[mode] = 0;
          osClrWatchedFD(index,mode);
        }
      }
    }
  }
}

//
// called from signal handler
void oz_io_check()
{
  int numbOfFDs = osCheckIO();
  if (!am.isCritical() && (numbOfFDs > 0)) {
    am.setSFlag(IOReady);
  }
}
