/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
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

#include "value.hh" // For that darn Bool
#include "glue_ioFactory.hh"  
#include "os.hh"
#ifndef WINDOWS
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#endif

/************* I/O ************************/

ErrorClass classifyError() {
  switch(ossockerrno()) {
  case EINTR:
    return EC_GO_AHEAD;
    
  case EHOSTUNREACH:
  case EAGAIN:
  case ETIMEDOUT:
    return EC_CONTINUE_LATER;
      
  case EINPROGRESS: 
  case EPIPE:
  case ECONNRESET:
  case EBADF:
    return EC_LOST;  // This connection is broken! The site might still be up.
  default: 
    return EC_LOST;
  }
}

Bool glue_io_read(int fd, void *arg){
  return static_cast<SocketChannel*>(arg)->invoke_reader();
}

Bool glue_io_write(int fd, void *arg){
  return static_cast<SocketChannel*>(arg)->invoke_writer();
}



/************************* SocketChannel *************************/

SocketChannel::~SocketChannel() {
  osclose(fd_in);
  if (fd_out != fd_in) osclose(fd_out);
}

void
SocketChannel::close() {
}

bool
SocketChannel::setCallback(DssChannelCallback* cbk) {
  if (lost) return false;
  worker = cbk;
  return true;
}

void
SocketChannel::registerRead(bool on) {
  if (on) {
    OZ_registerReadHandler(fd_in, glue_io_read, this);
  } else {
    OZ_unregisterRead(fd_in);
  }
}

void
SocketChannel::registerWrite(bool on) {
  if (on) {
    OZ_registerWriteHandler(fd_out, glue_io_write, this);
  } else {
    OZ_unregisterWrite(fd_out);
  }
}

int
SocketChannel::read(void* buf, const unsigned int& len) {
  int ret = ossaferead(fd_in, (char*) buf, len);
  if (ret > 0) return ret;         // normal return
  if (classifyError() == EC_LOST) lost = true;
  return 0;
}

int
SocketChannel::write(void* buf, const unsigned int& len) {
  int ret = ossafewrite(fd_out, (char*) buf, len);
  if (ret >= 0) return ret;        // normal return
  if (classifyError() == EC_LOST) lost = true;
  return 0;
}

bool
SocketChannel::invoke_reader() {
  // Check to see that we have a callback object, a null pointer
  // indicates that no interest exists for the channel
  if (worker == NULL || lost) return false;

  bool hasmore = worker->readDataAvailable();

  // After the call we check to see if the channel was lost while
  // reading it.  Note that we _dont_ tell the callback object while
  // reading about the fact that the channel was lost.
  if (lost) {
    worker->connectionLost();
    return false;
  }
  return hasmore;
}

bool
SocketChannel::invoke_writer() {
  // Check to see that we have a callback object, a null pointer
  // indicates that no interest exists for the channel
  if (worker == NULL || lost) return false;

  bool hasmore = worker->writeDataAvailable();

  // After the call we check to see if the channel was lost while
  // writing it.  Note that we _dont_ tell the callback object while
  // writing about the fact that the channel was lost.
  if (lost) {
    worker->connectionLost();
    return false;
  }
  return hasmore;
}
