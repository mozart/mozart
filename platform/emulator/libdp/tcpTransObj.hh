/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
 * 
 *  Contributors:
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

#ifndef __TCPTRANSOBJ_HH
#define __TCPTRANSOBJ_HH

#include "transObj.hh"
class TCPTransController;

enum unmarshalReturn {
  U_MORE,
  U_WAIT,
  U_CLOSED
};

class TCPTransObj: public TransObj {
  friend TCPTransController;
protected:
  ByteBuffer *readBuffer;
  ByteBuffer *writeBuffer;
private:
  int fd;
  int minSend;

  inline void marshal(MsgContainer *msgC, int acknum);
  inline enum unmarshalReturn unmarshal();
public:
  void init();
  void close();
  void deliver();
  void readyToReceive();

  void setSite(DSite *site);
  void setOwner(ComObj *comObj);
  OZ_Return setUp(DSite *site,ComObj *comObj,OZ_Term settings);

  Bool hasEmptyBuffers();

  TransController *getTransController();

  int writeHandler(int fd);
  int readHandler(int fd);
};

class TCPTransController: public TransController {
public:
  int getInfo(int &size);
  
protected:
  TransObj *newTransObj();  
  void deleteTransObj(TransObj* transObj);

  int getMaxNumOfResources() {
    return ozconf.perdioMaxTCPCache*2;
  }

  int getWeakMaxNumOfResources() {
    return ozconf.perdioMaxTCPCache;
  }
}; 

extern TCPTransController *tcptransController;
#endif
