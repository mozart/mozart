#ifndef __PRIOQUEUES_HH
#define __PRIOQUEUES_HH
typedef int Bool;

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
  MsgContainer *curm;        // Debug-check

  int prio_val_4;
  int prio_val_3;
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
  //  void clearRec(MsgContainer *msgC);

  Bool hasNeed();
  Bool hasQueued();
  void clear5();  // Clears prio 5 (+ recList no Unmarshalcont)
  void clearAll();
  void gcMsgCs();
};

#endif
