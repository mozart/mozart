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
class SocketChannel : public VirtualChannelInterface {
private:
  int fd;                          // socket file descriptor
  bool lost;                       // true when connection lost
  VcCbkClassInterface* worker;     // callback object

public:
  SocketChannel(int _fd) : fd(_fd), lost(false), worker(NULL) {}
  ~SocketChannel();

  virtual bool setCallback(VcCbkClassInterface*);
  virtual void registerRead(bool);
  virtual void registerWrite(bool);
  virtual int read(void* buf, const unsigned int& len);
  virtual int write(void* buf, const unsigned int& len);

  bool invoke_reader();
  bool invoke_writer();
};


#endif



