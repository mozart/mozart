/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *    Zacharias El Banna (zeb@sics.se)
 * 
 *  Contributors:
 *    Anna Neiderud (annan@sics.se)
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

#include "dss_comService.hh"

#ifndef __GLUE_IOFACTORY_HH
#define __GLUE_IOFACTORY_HH


// implementation of DSS channels, using sockets
class SocketChannel : public DssChannel {
private:
  int fd_in;                       // file descriptor (for reading)
  int fd_out;                      // file descriptor (for writing)
  bool lost;                       // true when connection lost
  DssChannelCallback* worker;      // callback object

  SocketChannel() {}
  SocketChannel(SocketChannel&) {}

public:
  SocketChannel(int fd) :
    fd_in(fd), fd_out(fd), lost(false), worker(NULL) {}
  SocketChannel(int in, int out) :
    fd_in(in), fd_out(out), lost(false), worker(NULL) {}
  ~SocketChannel();

  virtual bool setCallback(DssChannelCallback*);
  virtual void registerRead(bool);
  virtual void registerWrite(bool);
  virtual int read(void* buf, const unsigned int& len);
  virtual int write(void* buf, const unsigned int& len);
  virtual void close();

  bool invoke_reader();
  bool invoke_writer();
};


#endif



