/*
 *  Authors:
 *    Konstantin Popov
 * 
 *  Contributors:
 *
 *  Copyright:
 *    Konstantin Popov 1997-1998
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __VS_COMM_HH
#define __VS_COMM_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"
#include "dpBase.hh"

#ifdef VIRTUALSITES

#include "am.hh"
#include "genhashtbl.hh"
#include "virtual.hh"
#include "vs_msgbuffer.hh"
#include "vs_mailbox.hh"

//
// defined in virtual.cc (require static memory managers, etc.);
VSMsgBufferOwned* composeVSInitMsg();
VSMsgBufferOwned* composeVSSiteIsAliveMsg(DSite *s);
VSMsgBufferOwned* composeVSSiteAliveMsg(DSite *s, VirtualSite *vs);
VSMsgBufferOwned* composeVSUnusedShmIdMsg(DSite *s, key_t shmid);
//
void decomposeVSInitMsg(VSMsgBuffer *mb, DSite* &s);
void decomposeVSSiteIsAliveMsg(VSMsgBuffer *mb, DSite* &s);
void decomposeVSSiteAliveMsg(VSMsgBuffer *mb, DSite* &s, VirtualSite* &vs);
void decomposeVSUnusedShmIdMsg(VSMsgBuffer *mb, DSite* &s, key_t &shmid);

//
// Messages are queued up using the field of the message itself, 
// which is declared in its superclass:
class VSMsgQueueNode {
  friend class VSFreeMessagePool;
  friend class VSMsgQueue;
private:
  VSMsgQueueNode *nextVSMsgQueueNode, *prevVSMsgQueueNode;

  //
protected:
  VSMsgQueueNode* getNextVSMsgQueueNode() { return (nextVSMsgQueueNode); }
  void setNextVSMsgQueueNode(VSMsgQueueNode *n) {
    nextVSMsgQueueNode = n;
  }
  VSMsgQueueNode* getPrevVSMsgQueueNode() { return (prevVSMsgQueueNode); }
  void setPrevVSMsgQueueNode(VSMsgQueueNode *p) {
    prevVSMsgQueueNode = p;
  }

public:
  VSMsgQueueNode() {
    DebugCode(nextVSMsgQueueNode = (VSMsgQueueNode *) -1;);
    DebugCode(prevVSMsgQueueNode = (VSMsgQueueNode *) -1;);
  }
  ~VSMsgQueueNode() {}

  //
  // A message as a queue node can delete itself out of the queue
  // which is a double-linked ring of nodes, one of whose is a
  // "marker" node;
  void retractVSMsgQueueNode() {
    getNextVSMsgQueueNode()->setPrevVSMsgQueueNode(getPrevVSMsgQueueNode());
    getPrevVSMsgQueueNode()->setNextVSMsgQueueNode(getNextVSMsgQueueNode());
    DebugCode(nextVSMsgQueueNode = (VSMsgQueueNode *) -1;);
    DebugCode(prevVSMsgQueueNode = (VSMsgQueueNode *) -1;);
  }
};

//
// "Messages" are necessary for the case when a message cannot be
// delivered (i.e. put in a mailbox) immediately (when e.g. that
// mailbox is locked or full);
class VSMessage : public VSMsgQueueNode {
  friend class VSFreeMessagePool;
  friend class VSMsgQueue;
private:
  VSMsgBufferOwned *mb;
  MessageType msgType;
  // 'DSite*' is not needed (stored in the message buffer);
  int storeIndex;

  //
public:
  //
  void* operator new(size_t size) {
    OZ_error("VSMessage allocated using 'new(size_t)'");
    return ((void *) -1);	// gcc warning;
  }
  void* operator new(size_t, void *place) { return (place); }

  //
  VSMessage(VSMsgBufferOwned *mbIn, MessageType mtIn, DSite *sIn, int stIn)
    : mb(mbIn), msgType(mtIn), storeIndex(stIn)
  {}
  ~VSMessage() { OZ_error("VSMessage destroyed??"); }

  //
  VSMsgBufferOwned* getMsgBuffer() { return (mb); }
  MessageType getMessageType() { return (msgType); }
  DSite *getSite() { return (mb->getSite()); }
  int getStoreIndex() { return (storeIndex); }
};

//
// A list of free messages;
class VSFreeMessagePool {
private:
  VSMessage *freeList;

  //
public:
  VSFreeMessagePool() : freeList((VSMessage *) 0) {}
  ~VSFreeMessagePool() {}

  //
  VSMessage *allocate() {
    if (freeList) {
      VSMessage *b = freeList;
      freeList = (VSMessage *) freeList->getNextVSMsgQueueNode();
      DebugCode(b->setNextVSMsgQueueNode((VSMessage *) 0));
      return (b);
    } else {
      return ((VSMessage *) malloc(sizeof(VSMessage)));
    }
  }
  void dispose(VSMessage *b) {
    Assert(b->getNextVSMsgQueueNode() == (VSMessage *) -1);
    b->setNextVSMsgQueueNode(freeList);
    freeList = b;
  }
};

//
// A queue of messages waiting for delivery (an element of a "virtual
// site" object);
class VSMsgQueue : private VSMsgQueueNode {
private:
  VSMsgQueueNode *next;		// for 'getFirst()'/'getNext()';
  // a next element is pointed out in order to make the "current" one
  // removable;

  //
public:
  VSMsgQueue() : next((VSMsgQueueNode *) 0) {
    setNextVSMsgQueueNode(this); setPrevVSMsgQueueNode(this);
  }
  ~VSMsgQueue() { Assert(isEmpty()); }

  //
  Bool isEmpty() { return (getNextVSMsgQueueNode() == this); }
  Bool isNotEmpty() { return (getNextVSMsgQueueNode() != this); }

  //
  void enqueue(VSMessage *n) {
    VSMsgQueueNode *p = getPrevVSMsgQueueNode(); // could be 'this';

    //
    setPrevVSMsgQueueNode(n);
    n->setPrevVSMsgQueueNode(p);
    p->setNextVSMsgQueueNode(n);
    n->setNextVSMsgQueueNode(this);
  }

  //
  VSMessage* dequeue() {
    VSMsgQueueNode *n = getNextVSMsgQueueNode();
    if (n == this) {
      return ((VSMessage *) 0);
    } else {
      VSMsgQueueNode *nn = n->getNextVSMsgQueueNode();

      //
      setNextVSMsgQueueNode(nn);
      nn->setPrevVSMsgQueueNode(this);
      DebugCode(n->setNextVSMsgQueueNode((VSMsgQueueNode *) -1););
      DebugCode(n->setPrevVSMsgQueueNode((VSMsgQueueNode *) -1););
      //
      return ((VSMessage *) n);	// safe by 'enqueue()';
    }
  }

  //
  VSMessage* getFirst() {
    VSMsgQueueNode *vm;

    //
    vm = getNextVSMsgQueueNode();
    if (vm == this) {
      DebugCode(next = (VSMsgQueueNode *) 0;);
      return ((VSMessage *) 0);
    } else {
      next = vm->getNextVSMsgQueueNode();
      return ((VSMessage *) vm);
    }
  }
  VSMessage* getNext() {
    VSMsgQueueNode *vm;

    //
    vm = next;
    if (vm == this) {
      DebugCode(next = (VSMsgQueueNode *) 0;);
      return ((VSMessage *) 0);
    } else {
      next = vm->getNextVSMsgQueueNode();
      return ((VSMessage *) vm);
    }
  }
};

//
// Virtual sites that are unable to accept messages at the moment
// (thus, whose mailboxes locked) are kept in a list;
//
// I (kost@) have choosen the 'two-layer' scheme for keeping unsent
// messages, since (a) each VS has to know what's going on with its
// messages anyway (e.g. for the needs of statistic/control), and (b)
// the implementations of both queues can be changed separately.
//
// 'VirtualSite's are inherited from 'VSSiteQueueNode'. Nodes provide
// for retracting themselves out of a queue they stay in.
class VSSiteQueue;
class VSSiteQueueNode {
  friend class VSSiteQueue;
private:
  VSSiteQueueNode *nextVSSiteQueueNode;
  VSSiteQueueNode *prevVSSiteQueueNode;
  // Cannot cast in general since 'VSSiteQueueNode' can be not a sole
  // superclass;
  VirtualSite *vs;

  //
private:
  //
  VSSiteQueueNode* getNextVSSiteQueueNode() {
    return (nextVSSiteQueueNode);
  }
  void setNextVSSiteQueueNode(VSSiteQueueNode *n) {
    nextVSSiteQueueNode = n;
  }
  VSSiteQueueNode* getPrevVSSiteQueueNode() {
    return (prevVSSiteQueueNode);
  }
  void setPrevVSSiteQueueNode(VSSiteQueueNode *n) {
    prevVSSiteQueueNode = n;
  }

  //
  VirtualSite *getVS() { return (vs); }

  //
public:
  VSSiteQueueNode(VirtualSite *vsIn) : vs(vsIn) {
    DebugCode(nextVSSiteQueueNode = (VSSiteQueueNode *) -1;);
    DebugCode(prevVSSiteQueueNode = (VSSiteQueueNode *) -1;);
  }
  ~VSSiteQueueNode() {}

  //
  void retractVSSiteQueueNode() {
    getNextVSSiteQueueNode()
      ->setPrevVSSiteQueueNode(getPrevVSSiteQueueNode());
    getPrevVSSiteQueueNode()
      ->setNextVSSiteQueueNode(getNextVSSiteQueueNode());
    DebugCode(nextVSSiteQueueNode = (VSSiteQueueNode *) -1;);
    DebugCode(prevVSSiteQueueNode = (VSSiteQueueNode *) -1;);
  }
};

//
// 'VSRegister' keeps 'VirtualSite' objects ever created. It is used 
// for probing virtual sites, and for trying to reclaim resources of 
// those of them that went down. Note that VSs are probed regardless
// whether there are language-level probes installed;
class VSRegisterNode {
  friend class VSRegister;
  friend class VirtualSite;
private:
  VSRegisterNode *nextVSRegisterNode, *prevVSRegisterNode;
  // Cannot cast in general since 'VSRegisterNode' can be not a sole
  // superclass;
  VirtualSite *vs;

  //
protected:
  VSRegisterNode* getNextVSRegisterNode() { return (nextVSRegisterNode); }
  void setNextVSRegisterNode(VSRegisterNode *n) { nextVSRegisterNode = n; }
  VSRegisterNode* getPrevVSRegisterNode() { return (prevVSRegisterNode); }
  void setPrevVSRegisterNode(VSRegisterNode *p) { prevVSRegisterNode = p; }

  //
  void retractVSRegisterNode() {
    getNextVSRegisterNode()->setPrevVSRegisterNode(getPrevVSRegisterNode());
    getPrevVSRegisterNode()->setNextVSRegisterNode(getNextVSRegisterNode());
    DebugCode(nextVSRegisterNode = (VSRegisterNode *) -1;);
    DebugCode(prevVSRegisterNode = (VSRegisterNode *) -1;);
  }

  //
  VirtualSite *getVS() { return (vs); }

  //
public:
  VSRegisterNode(VirtualSite *vsIn) : vs(vsIn) {
    DebugCode(nextVSRegisterNode = (VSRegisterNode *) -1;);
    DebugCode(prevVSRegisterNode = (VSRegisterNode *) -1;);
  }
  ~VSRegisterNode() {}
};

//
// Internal message header;
class VSMsgHeader {
protected:
  VSMsgType type;

public:
  VSMsgHeader() { DebugCode(type = VS_M_INVALID;); }
  ~VSMsgHeader() {}
};

//
class VSMsgHeaderOwned : public VSMsgHeader {
public:
  VSMsgHeaderOwned(VSMsgType typeIn) {
    type = typeIn;
  }
  ~VSMsgHeaderOwned() {}

  //
  void marshal(VSMsgBuffer *mb) {
    Assert(sizeof(VSMsgType) <= sizeof(unsigned int));
    marshalNumber((unsigned int) type, mb);
  }
};
//
class VSMsgHeaderImported : public VSMsgHeader {
public:
  VSMsgHeaderImported(VSMsgBuffer *mb) {
    Assert(sizeof(VSMsgType) <= sizeof(unsigned int));
    type = (VSMsgType) unmarshalNumber(mb);
  }
  ~VSMsgHeaderImported() {}

  //
  VSMsgType getMsgType() { return (type); }
};

//
// Internal state of a virtual site, i.e. details that are (must be)
// hidden from the communication layer (i.e. the 'Site' object);
// 
// Shared memory can be unmapped (which never happens now);
#define VS_PENDING_UNMAP_MBOX  0x1

//
// That's the thing which is referenced by a site object denoting a
// virtual site, and created whenever something is to be sent there;
class VirtualSite : public VSSiteQueueNode, 
		    public VSMsgQueue, 
		    public VSRegisterNode {
  friend class VSSiteQueue;
  friend class VSProbingObject;
private:
  DSite *site;			// backward (cross) reference;
  SiteStatus status;		// (values;)
  int vsStatus;			// (flags;)
  //
  // 'isAliveSent==0' means no request was sent;
  unsigned long isAliveSent;		// when a last 'is alive' was sent
  unsigned long aliveAck;		// and received;
  //
  int probeAllCnt;
  int probePermCnt;

  //
  VSMailboxManagerImported *mboxMgr; // ... of that virtual site;

  //
  // Message bodies are allocated from this pool:
  VSFreeMessagePool *fmp;

  // If 'sendTo' fails to deliver something 'inline', it puts itself
  // (the virtual site) into the queue:
  VSSiteQueue *sq;

  //
  // GC of lost resources: whenever the site ("this" site) is discovered
  // to be dead, this list of segment keys is used for destroying 
  // corresponding shm pages;
  int segKeysNum, segKeysArraySize;
  key_t *segKeys;

  //
private:
  //
  void setStatus(SiteStatus statusIn) { status = statusIn; }

  //
  // All currently unsent messages to a site being disconnected
  // will be sent ('VS_PENDING_UNMAP_MBOX');
  void connect();
  void disconnect();

  //
  Bool isSetVSFlag(int flag) { return ((Bool) vsStatus & flag); }
  void setVSFlag(int flag) { vsStatus |= flag; }
  void clearVSFlag(int flag) { vsStatus &= ~flag; }

  //
  // special stuff - probing, etc.
  //
  unsigned long getTimeIsAliveSent() { return (isAliveSent); }
  void setTimeIsAliveSent(unsigned long ms) { isAliveSent = ms; }
  unsigned long getTimeAliveAck() { return (aliveAck); }

  //
  Bool hasProbesAll() { return (probeAllCnt); }
  Bool hasProbesPerm() { return (probePermCnt); }
  void incProbesAll() { probeAllCnt++; }
  void incProbesPerm() { probePermCnt++; }
  void decProbesAll() { probeAllCnt--; }
  void decProbesPerm() { probePermCnt--; }

  //
  // 'gcResources()' is used when a site is discovered to be dead:
  // Note that for chunk pool segments it's *not* sufficient just to
  // kill the page; we need also to unmap them;
  void gcResources(VSMsgChunkPoolManagerImported *cpm);

  //
public:
  VirtualSite(DSite *s, VSFreeMessagePool *fmpIn, VSSiteQueue *sqIn);
  ~VirtualSite() { Assert(status == SITE_PERM); }

  //
  SiteStatus getSiteStatus() {
    Assert(status != SITE_PERM);
    return (status);
  }

  // 'drop' means that the virtual site will not be used anymore 
  // since it is known to be dead (e.g. from a third party). 
  void drop();

  //
  // The message type, store site and store index parameters 
  // are opaque data (just stored);
  int sendTo(VSMsgBufferOwned *mb, MessageType mt,
	     DSite *storeSite, int storeIndex,
	     FreeListDataManager<VSMsgBufferOwned> *freeMBs);
  // ... resend it ('TRUE' if we succeeded);
  Bool tryToSendToAgain(VSMessage *vsm,
			FreeListDataManager<VSMsgBufferOwned> *freeMBs);

  //
  // We don't do anything in this case. We could 'disconnect()' but
  // then we would need (a) check for connection every time doing
  // 'sendTo', and (b) do it carefully with site registers (probing,
  // etc.);
  void zeroReferences() {}

  //
  // If 'sendTo' could say "unsent, stored as #msgNum", then with this
  // method we could try to remove that message from the queue:
  int discardUnsentMessage(int msgNum) { return (MSG_SENT); }
  //
  // ... again, the queue is considered to be opaque by now:
  int getQueueStatus(int &noMsgs) {
    noMsgs = 0; 
    return (0);
  }

  //
  DSite *getSite() { return (site); }

  //
  // special stuff - probing, etc.
  //
  // Both 'VSProbingObject' and exit hook need pids of virtual sites;
  int getVSPid() { return (mboxMgr->getMailbox()->getPid()); }

  //
  void setTimeAliveAck(unsigned long ms) { aliveAck = ms; }

  //
  // When cleaning up upon exiting, slaves virtual sites known to us
  // are explicitely killed;
  Bool isSlave() { return (isLocalKey(mboxMgr->getSHMKey())); }

  //
  // 'alive ack' message contains also a list of ipc keys - of shared
  // memory pages (mailboxes, chunk pool segments, may be even
  // something else?). These are maintained using these methods:
  void marshalLocalResources(MsgBuffer *mb,
			     VSMailboxManagerOwned *mbm,
			     VSMsgChunkPoolManagerOwned *cpm);
  void unmarshalResources(MsgBuffer *mb); 
};

//
// A queue of sites that have some pending messages for delivery;
//
// An object of this class is tried to empty (i.e. to send out pending
// messages to those sites) at regular intervals (tasks; see
// virtual.cc and am.*);
class VSSiteQueue : private VSSiteQueueNode {
private:
  VSSiteQueueNode *next;	// used by 'getFirst()'/'getNext()';
  // a next element is pointed out in order to make the "current" one
  // removable;

  //
public:
  VSSiteQueue()
    : VSSiteQueueNode((VirtualSite *) -1), next((VSSiteQueueNode *) 0) {
    setNextVSSiteQueueNode(this); setPrevVSSiteQueueNode(this);
  }
  ~VSSiteQueue() {}

  //
  Bool isEmpty() { return (getNextVSSiteQueueNode() == this); }
  Bool isNotEmpty() { return (getNextVSSiteQueueNode() != this); }

  //
  void enqueue(VirtualSite *n) {
    VSSiteQueueNode *p = getPrevVSSiteQueueNode();

    //
    setPrevVSSiteQueueNode(n);
    n->setPrevVSSiteQueueNode(p);
    p->setNextVSSiteQueueNode(n);
    n->setNextVSSiteQueueNode(this);
  }
  VirtualSite* dequeue() {
    VSSiteQueueNode *n = getNextVSSiteQueueNode();
    if (n == this) {
      return ((VirtualSite *) 0);
    } else {
      VSSiteQueueNode *nn = n->getNextVSSiteQueueNode();

      //
      setNextVSSiteQueueNode(nn);
      nn->setPrevVSSiteQueueNode(this);
      DebugCode(n->setNextVSSiteQueueNode((VSSiteQueueNode *) -1););
      DebugCode(n->setPrevVSSiteQueueNode((VSSiteQueueNode *) -1););

      //
      return (n->getVS());
    }
  }

  //
  VirtualSite* getFirst() {
    VSSiteQueueNode *n;

    //
    n = getNextVSSiteQueueNode();
    if (n == this) {
      DebugCode(next = (VSSiteQueueNode *) 0);
      return ((VirtualSite *) 0);
    } else {
      next = n->getNextVSSiteQueueNode();
      return (n->getVS());
    }
  }
  VirtualSite* getNext() {
    VSSiteQueueNode *n;

    //
    n = next;
    if (n == this) {
      DebugCode(next = (VSSiteQueueNode *) 0);
      return ((VirtualSite *) 0);
    } else {
      next = n->getNextVSSiteQueueNode();
      return (n->getVS());
    }
  }
};

//
class VSRegister : private VSRegisterNode {
private:
  VSRegisterNode *next;		// used by 'getFirst()'/'getNext()';

  //
public:
  VSRegister()
    : VSRegisterNode((VirtualSite *) -1), next((VSRegisterNode *) 0) {
    setNextVSRegisterNode(this), setPrevVSRegisterNode(this);
  }
  ~VSRegister() {}

  //
  Bool isEmpty() { return (getNextVSRegisterNode() == this); }
  Bool isNotEmpty() { return (getNextVSRegisterNode() != this); }
  
  //
  void registerVS(VSRegisterNode *n) {
    VSRegisterNode *p = getPrevVSRegisterNode();

    //
    setPrevVSRegisterNode(n);
    n->setPrevVSRegisterNode(p);
    p->setNextVSRegisterNode(n);
    n->setNextVSRegisterNode(this);
  }
  void retractVS(VSRegisterNode *n) { n->retractVSRegisterNode(); }

  //
  VirtualSite* getFirst() {
    VSRegisterNode *n;

    //
    n = getNextVSRegisterNode();
    if (n == this) {
      DebugCode(next = (VSRegisterNode *) 0);
      return ((VirtualSite *) 0);
    } else {
      next = n->getNextVSRegisterNode();
      return (n->getVS());
    }
  }
  VirtualSite* getNext() {
    VSRegisterNode *n;

    //
    n = next;
    if (n == this) {
      DebugCode(next = (VSRegisterNode *) 0);
      return ((VirtualSite *) 0);
    } else {
      next = n->getNextVSRegisterNode();
      return (n->getVS());
    }
  }
};

//
// Probing for virtual sites.
// There are two components of probing: (a) checking the site's
// process using kill(2) system call, and (b) sending 'ping'
// requests. Note that the first component does not guarantee the
// process with pid is still the right one. Note also that ping"ing
// ('M_SITE_IS_ALIVE'/'M_SITE_ALIVE') require in the worst case some
// unpredictable umount of time (speaking more precisely, that's the
// time for processing all the messages in the mailbox given that each
// message can be as long as senders (sic!) are allowed to create),
// since these messages are delivered with the same priority as all
// other messages;

//
//
class VSSiteHashTable : public GenHashTable {
private:
  int seqIndex;
  GenHashNode *seqGHN;		// both needed for 'getFirst'/'getNext';

  //
private:
  //
  // OK, other hash functions can be used as well...
  unsigned int hash(DSite *s) { return (s->hashPrimary()); }

  //
  GenHashNode* findNode(int hvalue, DSite *s);

  //
protected:
  VSSiteHashTable(int size) : GenHashTable(size) {}
  ~VSSiteHashTable() {}

  //
  Bool check(DSite *s);
  void enter(DSite *s);
  void remove(DSite *s);

  //
  DSite *getFirst();
  DSite *getNext();
};

//
//
class VSProbingObject : VSSiteHashTable {
private:
  //
  VSRegister *vsRegister;	// all known (virtual) sites;
  //
  int probesNum;
  // Perform checks at this interval. Right now checks are performed
  // at the same frequency for all virtual sites requested, which is
  // a minimal frequency among requested ones;
  unsigned long minInterval;
  // The time last checks were performed;
  unsigned long lastCheck;
  // The time last pings were sent out;
  unsigned long lastPing;

  //
public:
  VSProbingObject(int size, VSRegister *vsRegisterIn);
  ~VSProbingObject() {}

  //
  // Note we cannot ignore probe types even though there is only type
  // of real problems - the permanent one. This is because the perdio
  // layer can ask for different probes separately;
  ProbeReturn installProbe(VirtualSite *vs, ProbeType pt, int frequency);
  ProbeReturn deinstallProbe(VirtualSite *vs, ProbeType pt);

  //
  Bool checkProbes(unsigned long clock) {
    if (clock - lastCheck > minInterval) {
      lastCheck = clock;
      return (TRUE);
    } else {
      return (FALSE);
    }
  }

  //
  Bool processProbes(unsigned long clock,
		     VSMsgChunkPoolManagerImported *cpm);
};

#endif // VIRTUALSITES

#endif // __VS_COMM_HH
