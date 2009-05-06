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

#if defined(INTERFACE)
#pragma implementation "msl_prioQueues.hh"
#endif

#include "msl_prioQueues.hh"
#include "msl_msgContainer.hh"
#include "msl_timers.hh"

namespace _msl_internal{ //Start namespace

  // ******************** Local utilities ***********************
  namespace {
    inline void if_enqueue(MsgCnt *msgC,Queue &q) {
      Assert(((q.first==NULL) && (q.last==NULL))|| 
	     ((q.first!=NULL) && (q.last!=NULL)));
      
      if(q.last!=NULL)
	q.last->a_next=msgC;
      else {
	Assert(q.first==NULL);
	q.first=msgC;
      }
      q.last=msgC;
      msgC->a_next=NULL;
      return;
    }

    inline bool if_isEmpty(Queue &q) {
      Assert(((q.first==NULL) && (q.last==NULL))|| 
	     ((q.first!=NULL) && (q.last!=NULL)));
      return q.first == NULL;
    }

    inline MsgCnt *if_removeFirst(Queue &q) {
      Assert(((q.first==NULL) && (q.last==NULL))|| 
	     ((q.first!=NULL) && (q.last!=NULL)));
      
      if(q.first == NULL) {
	Assert(q.last == NULL);
	return NULL; 
      }
      MsgCnt *msgC=q.first;
      q.first = q.first->a_next;
      if(q.first == NULL)
	q.last = NULL;
      return msgC;
    }

    inline void if_addFirst(MsgCnt *msgC,Queue *q) {
      msgC->a_next=q->first;
      q->first=msgC;
      if(q->last==NULL)
	q->last=msgC;
    }

    inline void if_insertUnacked(Queue *unackedList,MsgCnt *msgC) {
      msgC->a_next=NULL; 
      if(unackedList->first == NULL)
	unackedList->last = msgC; 
      else
	unackedList->first->a_next=msgC;
      unackedList->first = msgC;	  
    }
  }
  

  // Class methods /////////////////////////////////////////////////////

#define Q_PRIO_VAL_4 10
#define Q_PRIO_VAL_3 10
#define Q_PRIO_VAL_2 100


  PrioQueues::PrioQueues(Timers* tim):
    e_timers(tim),
    unackedMsgs(Queue()),
    recList(NULL),
    curq(NULL),
    prio_val_4(Q_PRIO_VAL_4),
    prio_val_3(Q_PRIO_VAL_3),
    prio_val_2(Q_PRIO_VAL_2)
  {
    DebugCode(noMsgs=0);
    //dssLog(DLL_MOST,"Created PrioQueue");
    for (int i=0;i<5;i++)
      qs[i].first=qs[i].last=NULL;
    unackedMsgs.first=unackedMsgs.last=NULL;
  }

  // ***************** SENDING CONTAINER METHODS *****************

  void PrioQueues::enqueue(MsgCnt *msgC, int prio) {
    Assert(prio >= 0); 
    Assert(prio < 5); 
    DebugCode(noMsgs++);
    if_enqueue(msgC, qs[prio]);
  }

  MsgCnt *PrioQueues::getNext(bool working) {
    MsgCnt *ret=NULL;
    if(!if_isEmpty(qs[5-1])) {
      ret=if_removeFirst(qs[5-1]);
      curq=&qs[5-1];
    }
    else if(working) {
      do {
	if (prio_val_4>0 && !if_isEmpty(qs[4-1])) {
	  ret=if_removeFirst(qs[4-1]);
	  curq=&qs[4-1];
	  prio_val_4--;
	  break;
	}
	else {
	  prio_val_4=Q_PRIO_VAL_4;
	  if(prio_val_3>0 && !if_isEmpty(qs[3-1])) {
	    ret=if_removeFirst(qs[3-1]);
	    curq=&qs[3-1];
	    prio_val_3--;
	    break;
	  }
	  else {
	    prio_val_3=Q_PRIO_VAL_3;
	    if(!if_isEmpty(qs[2-1])) {
	      curq=&qs[2-1];
	      ret=if_removeFirst(qs[2-1]);
	      break;
	    }
	    else {
	      prio_val_2=Q_PRIO_VAL_2;
	      if(!if_isEmpty(qs[1-1])) {
		curq=&qs[1-1];
		ret=if_removeFirst(qs[1-1]);
		break;
	      }
	    }
	  }
	}
      } while(!if_isEmpty(qs[4-1]) || !if_isEmpty(qs[3-1]) || !if_isEmpty(qs[2-1]));
    }
    Assert(ret!=NULL || hasQueued()==false || !working);
    DebugCode(if (ret!=NULL) noMsgs--);
    return ret;
  }

  void PrioQueues::insertUnacked(MsgCnt *msgC) {
    if_insertUnacked(&unackedMsgs,msgC);
  }

  void PrioQueues::requeue(MsgCnt *msgC) {
    DebugCode(noMsgs++);
    if_addFirst(msgC,curq);
  }
  int PrioQueues::msgAcked(int num,bool resend,bool calcrtt) {
    MsgCnt *tmp, *cur=unackedMsgs.last;
    int ret = -1;
    while(cur!=NULL && cur->getMsgNum()<num){
      tmp = cur;
      cur=cur->a_next;
      delete tmp;}
    if(cur!=NULL){
      if(calcrtt){ 
	DSS_LongTime sendtime=cur->getSendTime();
	DSS_LongTime zero;
	if(sendtime!=zero) // else probing wasn't on
	  ret=e_timers->currTime() - sendtime;
      }
      tmp = cur;
      cur=cur->a_next;
      delete tmp;
    }
    unackedMsgs.last = cur;
    if(cur == NULL)
      unackedMsgs.first=cur;

    if(resend) {
      while(cur){
	tmp=cur->a_next;
	cur->resetMarshaling();
	enqueue(cur,3);
	cur=tmp;}
      unackedMsgs.last = unackedMsgs.first = NULL;
    }
    return ret;
  }

  void PrioQueues::clearCont() {
    // Clear continuations from all partly sent messages. These can 
    // currently only be found in the head of each priority.
    MsgCnt *cur;
    for(int i=1;i<=5;i++) {
      cur=qs[i-1].first;
      if(cur!=NULL) 
	cur->resetMarshaling();
    }
  }


  // ***************** RECEIVING CONTAINER METHODS *****************

  void PrioQueues::putRec(MsgCnt *msgC) {
    msgC->a_next=recList;
    recList=msgC;
  }

  MsgCnt* PrioQueues::getRec(int num) {
    MsgCnt** tmp = &recList;
    while((*tmp) != NULL){
      if ((*tmp)->getMsgNum()==num){
	MsgCnt* tmp2 = (*tmp);
	(*tmp) = (*tmp)->a_next; 
	return tmp2;
      } else
	tmp = &((*tmp)->a_next);
    }
    return NULL;
  }

  void PrioQueues::clearRec() {
    t_deleteList(recList);
  }

  // **************************************************************

  bool PrioQueues::hasQueued() {
    for(int i=1;i<=5;i++) {
      if(qs[i-1].first!=NULL) {
	Assert(qs[i-1].last!=NULL);
	Assert(noMsgs>0);
	return true;
      }
    }
    Assert(noMsgs==0);
    return false;
  }

  bool PrioQueues::hasNeed() {
    return hasQueued() ||  unackedMsgs.first!=NULL;
  }

  void PrioQueues::clear5() {
    MsgCnt *list=qs[5-1].first;
    MsgCnt *tmp;
    qs[5-1].first=qs[5-1].last=NULL;
    while(list!=NULL) {
      DebugCode(noMsgs--);
      tmp=list;
      list=list->a_next;
      // Assume for now that prio5 msgs are never in unackedList
      delete tmp;
    }
    // Take care of unacked prio5 msgs.
  }

  // To be called before deletion, will return unsent messages to perdio
  // The messages are returned unsorted. Thus they do not preserv their 
  // internal ordering. 
  MsgCnt*
  PrioQueues::clearAll() {
    MsgCnt* ans = NULL; 
    MsgCnt *tmp, *msgC; 
    for(int i=0;i<4;i++) {
      msgC=qs[i].first;
      while(msgC!=NULL) {
	DebugCode(noMsgs--);
	tmp = msgC->a_next;
	msgC->a_next = ans; 
	ans = msgC; 
	msgC = tmp; 
      }
      qs[i].last=NULL;
      qs[i].first=NULL;
    }
    msgC=unackedMsgs.last;
    while(msgC!=NULL) {
      tmp = msgC->a_next;
      msgC->a_next = ans; 
      ans = msgC; 
      msgC = tmp; 
    }
    unackedMsgs.first = NULL;
    unackedMsgs.last = NULL;
    clearRec();
    return ans; 
  }

  //
  void PrioQueues::gcMsgCs()
  {
    for (int i = 1; i<=5; i++)
      t_gcList(qs[i-1].first);
    t_gcList(unackedMsgs.last);
    t_gcList(recList);
  }
}
