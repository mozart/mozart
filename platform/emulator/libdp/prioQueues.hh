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

#ifndef __PRIOQUEUES_HH
#define __PRIOQUEUES_HH

#include "base.hh"
#include "dpBase.hh"

class MsgContainer;
struct Queue {
  MsgContainer *first;
  MsgContainer *last;
};
typedef struct Queue Queue;

class PrioQueues {
private:
  Queue qs[5];
  MsgContainer *unackedList; // Sorted list of unacked msgCs
  MsgContainer *recList;     // Unsorted list of msgCs being received

  Queue *curq;               // Used when msg is partly delivered (only temp.)
  DebugCode(MsgContainer *curm);

  int prio_val_4;
  int prio_val_3;
  int prio_val_2;

  int noMsgs;
public:
  void init();

  void enqueue(MsgContainer *msgC, int prio);
  MsgContainer *getNext(Bool working); // Unless working return only prio 5
  void insertUnacked(MsgContainer *msgC);
  void requeue(MsgContainer *msgC);    // A msg is put back first in the 
                                       // queue since it was not fully sent
  int msgAcked(int num,Bool resend,Bool calcrtt);

  void putRec(MsgContainer *msgC);
  MsgContainer *getRec(int num);
  void clearRec();
  void clearCont();

  Bool hasNeed();
  Bool hasQueued();
  int getQueueStatus();

  void clear5();  // Clears prio 5 (+ recList no Unmarshalcont)
  void clearAll();
  void startGCMsgCs();
  void gcMsgCs();
  void finishGCMsgCs();
};

#endif
