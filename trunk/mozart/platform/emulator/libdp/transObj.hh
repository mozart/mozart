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
  friend class TransController;
protected:
  ComObj *comObj;
  DSite *site;
  int bufferSize;

  virtual void init()=0;
public:
  virtual void close()=0;
  // Boolean tells wether this transobj is up and running with a comObj 
  // registered in running of the transcontroller. This is important for
  // cancelled transobjs from connection.cc.
  virtual void close(Bool)=0;
  // ComObj keeps track of its deliver calls so that as long as 
  // it has called deliver, and TransObj has not pulled all of its
  // messages with getNextMsgContainer, it will not say deliver again.
  virtual void deliver()=0;
  virtual void readyToReceive()=0;

  virtual void setSite(DSite *site)=0;
  virtual void setOwner(ComObj *comObj)=0;
  virtual OZ_Return setUp(DSite *site,ComObj *comObj,OZ_Term settings)=0;

  virtual Bool hasEmptyBuffers()=0;

  int getBufferSize() { return (bufferSize); }
  // 'setBufferSize' is used by a comObj if the buffer size needs to
  // be changed;
  virtual void setBufferSize(int bufSizeIn) = 0;

  virtual TransController *getTransController()=0;
};

#define TransController_CUTOFF 100
class TransController: public FreeListManager, public DPMarshalers {
  unsigned int used;
  unsigned int allocm,usedm,usedum;
public:
  TransController();
  virtual ~TransController() {}

  // For statistics
  int getUsed();
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
  void transObjFreed(ComObj *comObj,TransObj *transObj,Bool isrunning);
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
