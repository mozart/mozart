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
  void *close();
  void deliver();
  void readyToReceive();

  void setFD(int fd);
  void setSite(DSite *site);
  void setOwner(ComObj *comObj);

  Bool hasEmptyBuffers();

  TransController *getTransController();

  int writeHandler(int fd);
  int readHandler(int fd);
};

class TCPTransController: public TransController {
public:
  // AN: kostja why?
  virtual ~TCPTransController() {}

protected:
  // AN: kostja why virtual?
  virtual TransObj *newTransObj();
  virtual void deleteTransObj(TransObj* transObj);

  virtual int getMaxNumOfResources() {
    return ozconf.perdioMaxTCPCache*2;
  }

  virtual int getWeakMaxNumOfResources() {
    return ozconf.perdioMaxTCPCache;
  }
};

extern TCPTransController *tcptransController;
#endif
