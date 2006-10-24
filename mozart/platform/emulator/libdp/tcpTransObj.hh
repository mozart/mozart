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
  friend class TCPTransController;
protected:
  ByteBuffer *readBuffer;
  ByteBuffer *writeBuffer;
private:
  int fd;

  inline void marshal(MsgContainer *msgC, int acknum);
  inline enum unmarshalReturn unmarshal();
public:
  void init();
  void close();
  void close(Bool isrunning);
  void deliver();
  void readyToReceive();

  void setSite(DSite *site);
  void setOwner(ComObj *comObj);
  OZ_Return setUp(DSite *site,ComObj *comObj,OZ_Term settings);

  Bool hasEmptyBuffers();
  void setBufferSize(int bufSizeIn);

  TransController *getTransController();

  int writeHandler(int fd);
  int readHandler(int fd);
};

class TCPTransController: public TransController {
public:
  ~TCPTransController();

  int getInfo(int &size);
  
protected:
  TransObj *newTransObj();  
  void deleteTransObj(TransObj* transObj);

  int getMaxNumOfResources() {
    return ozconf.dpTCPHardLimit;
  }

  int getWeakMaxNumOfResources() {
    int w, h;
    w=ozconf.dpTCPWeakLimit;
    h=ozconf.dpTCPHardLimit;
    return w<h?w:h-1;
  }
}; 

extern TCPTransController *tcptransController;
#endif
