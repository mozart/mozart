#ifndef __TRANSOBJ_HH
#define __TRANSOBJ_HH

#include "dpBase.hh"
#include "dsite.hh"
#include "dpMarshaler.hh"

class ComObj;
class MsgContainer;
class ByteBuffer;
class TransController;
class TimerElement;

class TransObj {
  friend TransController;
protected:
  ComObj *comObj;
  DSite *site;
  int bufferSize;

  virtual void init()=0;
public:
  virtual void *close()=0;
  // ComObj keeps track of its deliver calls so that as long as
  // it has called deliver, and TransObj has not pulled all of its
  // messages with getNextMsgContainer, it will not say deliver again.
  virtual void deliver()=0;
  virtual void readyToReceive()=0;

  virtual void setSite(DSite *site)=0;
  virtual void setOwner(ComObj *comObj)=0;

  virtual Bool hasEmptyBuffers()=0;

  int getBufferSize() {
    return bufferSize;
  }
  virtual TransController *getTransController()=0;
};

#define TransController_CUTOFF 100
class TransController: public FreeListManager, public DPMarshalers {
  unsigned int used;
  unsigned int allocm,usedm,usedum;
public:
  TransController();
  virtual ~TransController() {}

  // The receiver side: If resources are available a fresh transObj
  // to be used at accept procedure is returned, else NULL
  TransObj *getTransObj();
  // Must be called by the comObj after accept
  void addRunning(ComObj *comObj);
  // Called when comObj:s pass on a connection
  void switchRunning(ComObj *inList,ComObj *newc);
  // The initiators side: When (possibly now) resources are
  // available comObj->transObjReady is called with a fresh transObj.
  void getTransObj(ComObj *comObj);
  // After comObj->preemptTransObj this method shall be called by comObj
  // When the comObj is done the transObj is returned with this method
  void transObjFreed(ComObj *comObj,TransObj *transObj);
  // Initiator site: When the comObj is removed and no longer wants to
  // wait for a resource (i.e. a transport object). The request is
  // removed from the queue:
  void comObjDone(ComObj *comObj);

  void changeNumOfResources();
  int getCTR(){ return wc;}

  //
  // 'allocateMarshalersForResources' maps the number of resources
  // to a number of marshalers:
  void allocateMarshalersForResources(int numOfResources);
  //
  DPMarshaler *getMarshaler();
  Builder *getUnmarshaler();
  void returnMarshaler(DPMarshaler *dpm);
  void returnUnmarshaler(Builder *dpb);
protected:
  int wc;

  virtual TransObj *newTransObj() { Assert(0); return ((TransObj *) 0); }
  virtual void deleteTransObj(TransObj *transObj) { Assert(0); }
  virtual int getMaxNumOfResources() { Assert(0); return (0); }
  virtual int getWeakMaxNumOfResources() { Assert(0); return (0); }
private:
  ComObj *running,*running_last;
  ComObj *waiting,*waiting_last;
  TimerElement *timer;

  ComObj *getFirst(ComObj *&list,ComObj *&list_last);
  void addLast(ComObj *&list,ComObj *&list_last,ComObj *c);
  void enqueue(ComObj *c);
  void remove(ComObj *&list,ComObj *&list_last,ComObj *c);
public:
  // technical reasons for public
  Bool closeOne();
};

#endif
