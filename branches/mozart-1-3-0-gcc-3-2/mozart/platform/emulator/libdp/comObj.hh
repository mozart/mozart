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

#ifndef __COMOBJ_HH
#define __COMOBJ_HH

#include "dpBase.hh"
#include "memaux.hh"
#include "prioQueues.hh"

#define NO_MSG_NUM -1

class TransObj;
class MsgContainer;
class DSite;
class TimerElement;

enum CState {
  CLOSED,                      // CLOSED means no transObj (=no connection)
  CLOSED_WF_HANDOVER,          // WF=WAIT FOR
  CLOSED_WF_REMOTE,            //        waiting for incoming connection
  CLOSED_PROBLEM,
  ANONYMOUS_WF_NEGOTIATE,
  OPENING_WF_PRESENT,          
  OPENING_WF_NEGOTIATE_ANS,
  WORKING,
  CLOSING_HARD,
  CLOSING_WEAK,
  CLOSING_WF_DISCONNECT
};

class ComObj {
  friend class ComController;
  friend class TransController;
private:
  CState state;

  // Pointers to distribution-layer and transport-layer entities
  TransObj *transObj;
protected:
  DSite *site;
private:

  // Storage for MsgContainers
  PrioQueues queues;

  // Numbers for messages and acking-scheme
  int lastSent;
  int lastReceived;
  //  Bool ackQueued;

  int sentLrgMsg;
  int receivedLrgMsg;

  // One timer to be used for opening/closing/acking (one at a time)
  TimerElement *timer;
  TimerElement *reopentimer;
  TimerElement *closetimer;

  // For probing
  Bool probing;
  Bool probeFired;
  int minrtt;
  int maxrtt;
  int probeinterval;
  TimerElement *probeIntervalTimer;
  TimerElement *probeFaultTimer;

  // Lost connections
  int retryTimeout;

  Bool localRef;
  Bool remoteRef;
  Bool sentclearref;
  int msgAckTimeOut;
  int msgAckLength;

  // Statistics
  int nosm; // Number of sent messages since getNOSM() was last called
  int norm; // Number of received messages since getNORM() was last called
  int lastrtt;

  // private methods
  void open();
  void closeTemp();
  inline TaggedRef createCI(int);
  inline void extractCI(TaggedRef, int &bufferSize, int &maxBufferSize);
  void errorRec(int);
  Bool hasNeed();
  void adoptCI(OZ_Term channelinfo);
  Bool merge(ComObj *old,ComObj *anon,OZ_Term channelinfo);
  void close(CState statetobe,Bool merging);
protected:
  ComObj *next; // For ComController usage
  void close(CState statetobe);
  void gcComObj() { queues.gcMsgCs(); }
  void startGCComObj() { queues.startGCMsgCs(); }
  void finishGCComObj() { queues.finishGCMsgCs(); }
  void clearTimers();
  void shutDown();
  ComObj *next_cache; // For TransController usage
  Bool canBeClosed();
public:
  Bool hasQueued();
  ComObj(DSite *site);
  void init(DSite *site);

  DSite *getSite() {return site;}
  CState getState() {return state;}


  void send(MsgContainer *);
  void installProbe(int lowerBound, int higherBound, int interval);  
           // Should this be moved to the comController?
  Bool canBeFreed(); // A question that implicitly tells the comObj
                     // that no local references exist.
  int getQueueStatus();
  
  // For TransController
  void preemptTransObj();

  // For TransObj:
  MsgContainer *getNextMsgContainer(int &); // Provides the TransObj 
                  // with the MsgContainer of the next message to be 
                  // sent. The current acknowledgement number is given by 
                  // the int &.
  void msgSent(MsgContainer *);
  void msgPartlySent(MsgContainer *); // Store away a message to be 
                                       // continued later.
  void msgPartlyReceived(MsgContainer *);
  void msgAcked(int num);

  MsgContainer *getMsgContainer(); // Gives a new clean MsgContainer 
                                // to be filled with an incomming message.
  MsgContainer *getMsgContainer(int num); // Gives the priviously stored 
                          // MsgContainer for message num to be continued.
  Bool msgReceived(MsgContainer *); // A full message was received and is 
                                    // now handed up. Return: continue?
  void connectionLost();

  // For connection procedure
  Bool handover(TransObj *);
  void accept(TransObj *);

  // Statistics
  int getNOSM();
  int getNORM();
  int getLastRTT();
  OZ_Term getStateStatistics();

  // Extras for internal use (must be public anyway)
  Bool openTimerExpired();
  Bool closeTimerExpired();
  Bool sendProbePing();
  Bool probeFault();
  Bool sendAck(Bool timerInvoked);
  Bool reopen();

  // For connection.cc usage
  OZ_Term connectVar;
  Bool connectgrantrequested;
  OZ_Term transtype;
};

void comController_acceptHandler(TransObj *);

#define ComObj_CUTOFF 100

class ComController: public FreeListManager {
private:
  ComObj *list;

public:
  ComController():FreeListManager(ComObj_CUTOFF) {
    list=NULL;
    wc = 0;
  }
  ~ComController();
  int wc;

  ComObj *newComObj(DSite *site);
  void deleteComObj(ComObj* comObj);
  int getCTR(){ return wc;}

  void startGCComObjs();
  void gcComObjs();
  void finishGCComObjs();
  int closeDownCount();
  void closeAll();
  Bool valid(ComObj *testObj);
};

extern ComController *comController;

#endif // __COMOBJ_HH
