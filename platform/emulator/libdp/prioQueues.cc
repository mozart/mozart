#include "prioQueues.hh"
#include "msgContainer.hh"
#include "am.hh"

#define Q_PRIO_VAL_4 2
#define Q_PRIO_VAL_3 2

//  inline void u_enqueue(MsgContainer *,Queue &);
//  inline Bool u_isEmpty(Queue &);
//  inline MsgContainer *u_removeFirst(Queue &);
//  inline void u_addFirst(MsgContainer *,Queue *);
//  inline void u_insertUnacked(MsgContainer *&unackedList,MsgContainer *msgC);
//  inline void u_gcQueue(Queue const &);
//  inline void u_gcUnackedList(MsgContainer *);
//  inline void u_gcRecList(MsgContainer *);

// Local utilities /////////////////////////////////////////
inline void u_enqueue(MsgContainer *msgC,Queue &q) {
  Assert(((q.first==NULL) && (q.last==NULL))||
         ((q.first!=NULL) && (q.last!=NULL)));

  if(q.last!=NULL) {
    q.last->next=msgC;
    Assert(msgC->next==NULL);
  }
  else {
    Assert(q.first==NULL);
    q.first=msgC;
  }
  q.last=msgC;
  msgC->next=NULL;
  return;
}

inline Bool u_isEmpty(Queue &q) {
  Assert(((q.first==NULL) && (q.last==NULL))||
         ((q.first!=NULL) && (q.last!=NULL)));
  return q.first == NULL;
}

inline MsgContainer *u_removeFirst(Queue &q) {
  Assert(((q.first==NULL) && (q.last==NULL))||
         ((q.first!=NULL) && (q.last!=NULL)));

  if(q.first == NULL) {
    Assert(q.last == NULL);
    return NULL;
  }
  MsgContainer *msgC=q.first;
  q.first = q.first->next;
  if(q.first == NULL)
    q.last = NULL;
  return msgC;
}

inline void u_addFirst(MsgContainer *msgC,Queue *q) {
  msgC->next=q->first;
  q->first=msgC;
  if(q->last==NULL)
    q->last=msgC;
}

inline void u_insertUnacked(MsgContainer *&unackedList,MsgContainer *msgC) {
  MsgContainer *tmp=unackedList;
  MsgContainer *prev=NULL;
  while(tmp!=NULL && msgC->getMsgNum()<tmp->getMsgNum()) {
    prev=tmp;
    tmp=tmp->next;
  }

  if(prev==NULL) {
    msgC->next=unackedList;
    unackedList=msgC;
  }
  else {
    prev->next=msgC;
    msgC->next=tmp;
  }
}

//
static inline
void u_gcQueue(Queue const &q)
{
  MsgContainer *tmp = q.first;
  while (tmp) {
    tmp->gcMsgC();
    tmp = tmp->next;
  }
}

//
static inline
void u_startGCQueue(Queue const &q)
{
  Assert(((q.first == NULL) && (q.last == NULL)) ||
         ((q.first != NULL) && (q.last != NULL)));
  MsgContainer *tmp = q.first;
  while (tmp) {
    tmp->gcStart();
    tmp=tmp->next;
  }
}

//
static inline
void u_finishGCQueue(Queue const &q)
{
  MsgContainer *tmp = q.first;
  while (tmp) {
    tmp->gcFinish();
    tmp = tmp->next;
  }
}

//
static inline
void u_gcUnackedList(MsgContainer *unackedList)
{
  MsgContainer *tmp = unackedList;
  while (tmp) {
    tmp->gcMsgC();
    tmp = tmp->next;
  }
}

//
static inline
void u_startGCUnackedList(MsgContainer *unackedList)
{
  MsgContainer *tmp = unackedList;
  while (tmp) {
    tmp->gcStart();
    tmp = tmp->next;
  }
}

//
static inline
void u_finishGCUnackedList(MsgContainer *unackedList)
{
  MsgContainer *tmp = unackedList;
  while (tmp) {
    tmp->gcFinish();
    tmp = tmp->next;
  }
}

//
static inline
void u_gcRecList(MsgContainer *recList)
{
  MsgContainer *tmp = recList;
  while (tmp) {
    tmp->gcMsgC();
    tmp=tmp->next;
  }
}

//
static inline
void u_startGCRecList(MsgContainer *recList)
{
  MsgContainer *tmp = recList;
  while (tmp) {
    tmp->gcStart();
    tmp = tmp->next;
  }
}

//
static inline
void u_finishGCRecList(MsgContainer *recList)
{
  MsgContainer *tmp = recList;
  while (tmp) {
    tmp->gcFinish();
    tmp = tmp->next;
  }
}

// Class methods /////////////////////////////////////////////////////

void PrioQueues::init() {
  for (int i=0;i<5;i++)
    qs[i].first=qs[i].last=NULL;
  curq=NULL;
  unackedList=recList=NULL;
  prio_val_4=Q_PRIO_VAL_4;
  prio_val_3=Q_PRIO_VAL_3;
}

void PrioQueues::enqueue(MsgContainer *msgC, int prio) {
  //  if (prio!=5&&prio!=3) prio = 3;
  u_enqueue(msgC, qs[prio-1]);
}

MsgContainer *PrioQueues::getNext(Bool working) {
  MsgContainer *ret=NULL;
  if(!u_isEmpty(qs[5-1])) {
    ret=u_removeFirst(qs[5-1]);
    curq=&qs[5-1];
  }
  else if(working) {
    do {
      if (prio_val_4>0 && !u_isEmpty(qs[4-1])) {
        ret=u_removeFirst(qs[4-1]);
        curq=&qs[4-1];
        prio_val_4--;
        break;
      }
      else if(prio_val_3>0) {
        prio_val_4=Q_PRIO_VAL_4;
        if(!u_isEmpty(qs[3-1])) {
          ret=u_removeFirst(qs[3-1]);
          curq=&qs[3-1];
          prio_val_3--;
          break;
        }
      }
      else {
        prio_val_3=Q_PRIO_VAL_3;
        if(!u_isEmpty(qs[2-1])) {
          curq=&qs[2-1];
          ret=u_removeFirst(qs[2-1]);
          break;
        }
      }
    } while(!u_isEmpty(qs[4-1]) || !u_isEmpty(qs[3-1]));

    if (ret==NULL) {
      ret=u_removeFirst(qs[1-1]);
      curq=&qs[1-1];
    }
  }
  curm=ret;
  return ret;
}

void PrioQueues::insertUnacked(MsgContainer *msgC) {
  u_insertUnacked(unackedList,msgC);
}

void PrioQueues::requeue(MsgContainer *msgC) {
  Assert(curm==msgC);
  u_addFirst(msgC,curq);
}

int PrioQueues::msgAcked(int num,Bool resend,Bool calcrtt) {
  MsgContainer *cur=unackedList;
  MsgContainer *prev=NULL;

  int ret=-1;
  while(cur!=NULL && cur->getMsgNum()>num) {
    prev=cur;
    if(resend) {
      unackedList=cur->next;
      u_addFirst(cur,&qs[4-1]);
      printf("resend %s %d\n",mess_names[cur->getMessageType()],
             cur->getMsgNum());
      cur=unackedList;
    }
    else
      cur=cur->next;
  }
  if (prev!=NULL && !resend)
    prev->next=NULL;
  else
    unackedList=NULL;
  if(calcrtt && cur!=NULL) {
    int sendtime=cur->getSendTime();
    if(sendtime!=-1) // else probing wasn't on
      ret=am.getEmulatorClock() - cur->getSendTime();
  }
  while(cur!=NULL) {
    prev=cur;
    cur=cur->next;
    // kost@ : it's here 'cause there is neither a proper destructor for
    // MsgContainer"s nor a flag saying "that's an outgoing message":
    prev->deleteSnapshot();
    msgContainerManager->deleteMsgContainer(prev);
  }
  return ret;
}

void PrioQueues::putRec(MsgContainer *msgC) {
    msgC->next=recList;
    recList=msgC;
}

MsgContainer *PrioQueues::getRec(int num) {
  MsgContainer *tmp=recList;
  MsgContainer *prev=NULL;
  while(tmp!=NULL) {
    if (tmp->getMsgNum()==num) {
      if(prev!=NULL)
        prev->next=tmp->next;
      else
        recList=tmp->next;
      return tmp;
    }
    else {
      prev=tmp;
      tmp=tmp->next;
    }
  }
  return NULL;
}

//  void PrioQueues::clearRec(MsgContainer *msgC) {
//    MsgContainer *tmp=recList;
//    MsgContainer *prev=NULL;

//    while(tmp!=NULL) {
//      if(msgC==tmp) {
//        if(prev==NULL)
//      recList=tmp->next;
//        else
//      prev->next=tmp->next;
//        tmp->deleteSnapshot();
//        msgContainerManager->deleteMsgContainer(tmp);
//        return;
//      }
//      else {
//        prev=tmp;
//        tmp=tmp->next;
//      }
//    }
//    printf("Unknown msgC to clearRec\n");
//  }

Bool PrioQueues::hasQueued() {
  for(int i=1;i<=5;i++) {
    if(qs[i-1].first!=NULL) {
      Assert(qs[i-1].last!=NULL);
      return TRUE;
    }
  }
  return FALSE;
}

Bool PrioQueues::hasNeed() {
//    printf("hasNeed unackedList %s %d next %x\n",
//       unackedList!=NULL?mess_names[unackedList->getMessageType()]:"empty",
//       unackedList!=NULL?unackedList->getMsgNum():0-1,
//       unackedList!=NULL?(int) unackedList->next:0);
  return hasQueued() || unackedList!=NULL;
}

void PrioQueues::clear5() {
  MsgContainer *list=qs[5-1].first;
  MsgContainer *tmp;
  qs[5-1].first=qs[5-1].last=NULL;
  while(list!=NULL) {
    tmp=list;
    list=list->next;
    // Assume for now that prio5 msgs are never in unackedList
    tmp->deleteSnapshot();
    msgContainerManager->deleteMsgContainer(tmp);
  }
  // Take care of unacked prio5 msgs.
}

// To be called before deletion, will return unsent messages to perdio
// Unless we are perm none should be left.
void PrioQueues::clearAll() {
  MsgContainer *msgC;
  for(int i=1;i<5;i++) {
    msgC=qs[i-1].first;
    while(msgC!=NULL) {
      qs[i-1].first=msgC->next;
      msgC->deleteSnapshot();
      msgContainerManager->deleteMsgContainer(msgC,COMM_FAULT_PERM_NOT_SENT);
      msgC=qs[i-1].first;
    }
    qs[i-1].last=NULL;
  }
  msgC=unackedList;
  while(msgC!=NULL) {
    unackedList=msgC->next;
    msgC->deleteSnapshot(); // AN! can unacked have snapshot?
    msgContainerManager->deleteMsgContainer(msgC,COMM_FAULT_PERM_MAYBE_SENT);
    msgC=unackedList;
  }
  msgC=recList;
  while(msgC!=NULL) {
    recList=msgC->next;
    msgC->deleteSnapshot(); // AN! for incoming?
    msgContainerManager->deleteMsgContainer(msgC);
    msgC=recList;
  }
}

//
void PrioQueues::gcMsgCs()
{
  for (int i = 1; i<=5; i++)
    u_gcQueue(qs[i-1]);
  u_gcUnackedList(unackedList);
  u_gcRecList(recList);
}

//
void PrioQueues::startGCMsgCs()
{
  for (int i = 1; i<=5; i++)
    u_startGCQueue(qs[i-1]);
  u_startGCUnackedList(unackedList);
  u_startGCRecList(recList);
}

//
void PrioQueues::finishGCMsgCs()
{
  for(int i=1;i<=5;i++)
    u_finishGCQueue(qs[i-1]);
  u_finishGCUnackedList(unackedList);
  u_finishGCRecList(recList);
}
