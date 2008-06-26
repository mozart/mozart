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

#ifdef INTERFACE
#pragma interface
#endif

//#include "dssBase.hh"
#include <stdio.h> //NULL

namespace _msl_internal{ //Start namespace

  class MsgCnt;
  class Timers;


  struct Queue {
    MsgCnt *first;
    MsgCnt *last;
  };
  typedef struct Queue Queue;

  class PrioQueues {
  private:
    // *********** Only internal ***********
    static const int Q_PRIO_VAL_4;
    static const int Q_PRIO_VAL_3;
    static const int Q_PRIO_VAL_2;

    Timers* const    e_timers;
    Queue            qs[5];
    Queue            unackedMsgs; // Sorted list of unacked msgCs
    MsgCnt* recList;     // Unsorted list of msgCs being received

    Queue *curq;               // Used when msg is partly delivered (only temp.)

    int prio_val_4;
    int prio_val_3;
    int prio_val_2;

    int noMsgs;

    PrioQueues(const PrioQueues&):e_timers(NULL), unackedMsgs(Queue()), recList(NULL),
                                  curq(NULL), prio_val_4(0), prio_val_3(0), prio_val_2(0),
                                  noMsgs(0){}
    PrioQueues& operator=(const PrioQueues&){ return *this; };
  public:
    PrioQueues(Timers* tim);
    ~PrioQueues(){};

    // *********** SEND ************
    void enqueue(MsgCnt *msgC, int prio);
    MsgCnt *getNext(bool working); // Unless working return only prio 5
    void insertUnacked(MsgCnt *msgC);
    void requeue(MsgCnt *msgC);    // A msg is put back first in the
    // queue since it was not fully sent
    int msgAcked(int num,bool resend,bool calcrtt);
    void clearCont();

    // ********** RECEVED **********
    void putRec(MsgCnt *msgC);
    MsgCnt *getRec(int num);
    void clearRec();

    // ----------------------------
    bool hasNeed();
    bool hasQueued();
    int getQueueStatus();

    void clear5();  // Clears prio 5 (+ recList no Unmarshalcont)
    MsgCnt* clearAll();
    void gcMsgCs();
  };

} //End namespace
#endif
