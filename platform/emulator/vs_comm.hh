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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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

#ifdef VIRTUALSITES

#include "genhashtbl.hh"
#include "comm.hh"
#include "vs_msgbuffer.hh"
#include "vs_mailbox.hh"

//
// The 'VirtualInfo' class serves two purposes:
// (a) identifies the virtual sites group (the id of the master 
//     in this implementation).
// (b) contains information for receiving/sending messages to 
//     the site.
class VirtualInfo {
  friend Site::initVirtualInfoArg(VirtualInfo *vi);
private:
  // The "street address" of the master virtual site
  // ("virtual site group id");
  ip_address address;
  port_t port;
  TimeStamp timestamp;

  // "box" of the site itself;
  key_t mailboxKey;

  //
private:
  //
  // used by 'Site::initVirtualInfoArg(VirtualInfo *vi)';
  void setAddress(ip_address aIn) { address = aIn; }
  void setPort(port_t pIn) { port = pIn; }
  void setTimeStamp(TimeStamp &tsIn) { 
    timestamp.start = tsIn.start;
    timestamp.pid = tsIn.pid;
  }

  //
public:
  //
  // Most instances of VirtualInfo are allocated using free list 
  // memory, but not all of them (like for 'mySite');
  void* operator new(size_t size) { return (malloc(size)); }
  void* operator new(size_t, void *place) { return (place); }

  //
  // When a "plain" site declares itself as a virtual one (when it
  // creates a first slave site), copy the virtual site group id
  // from the site's 'Site' object, and put the mailbox key:
  VirtualInfo(Site *bs, key_t mboxkIn)
    : mailboxKey(mboxkIn)
  {
    bs->initVirtualInfoArg(this);
  }

  //
  // "virtual info" for a slave's 'mySite' is copied from the father's,
  // and there is no mailboxkey (yet):
  VirtualInfo(VirtualInfo *vii)
    : address(vii->address), port(vii->port), timestamp(vii->timestamp),
      mailboxKey(0)
  {}

  //
  // When the 'mySite' of a slave site is extended for the virtual
  // info (by 'BIVSinitServer'), then the mailbox key is given (the
  // only 'BIVSInitServer's argument), and the "virtual site group id"
  // is taken from the mailbox'es virtual info:
  VirtualInfo(VirtualInfo *mvi, key_t mboxkIn)
    : address(mvi->address), port(mvi->port), timestamp(mvi->timestamp),
      mailboxKey(mboxkIn)
  {}

  //
  // There is free list management of virtual info"s;
  ~VirtualInfo() { error("VirtualInfo is destroyed?"); }
  // There is nothing to be done when disposed;
  void destroy() {
    DebugCode(address = (ip_address) 0);
    DebugCode(port = (port_t) 0);
    DebugCode(timestamp.start = (time_t) 0);
    DebugCode(timestamp.pid = (int) 0);
    DebugCode(mailboxKey = (key_t) 0);
  }

  //
  // Another type of initialization - unmarshaliing:
  VirtualInfo(MsgBuffer *mb) {
    Assert(sizeof(ip_address) <= sizeof(unsigned int));
    Assert(sizeof(port_t) <= sizeof(unsigned short));
    Assert(sizeof(time_t) <= sizeof(unsigned int));
    Assert(sizeof(int) <= sizeof(unsigned int));
    Assert(sizeof(key_t) <= sizeof(unsigned int));

    //
    address = (ip_address) unmarshalNumber(mb);
    port = (port_t) unmarshalShort(mb);  
    timestamp.start = (time_t) unmarshalNumber(mb);
    timestamp.pid = (int) unmarshalNumber(mb);
    //
    mailboxKey = (key_t) unmarshalNumber(mb);
  }

  // 
  // NOTE: marshaling must be complaint with
  // '::unmarshalUselessVirtualInfo()';
  void marshal(MsgBuffer *mb) {
    Assert(sizeof(ip_address) <= sizeof(unsigned int));
    Assert(sizeof(port_t) <= sizeof(unsigned short));
    Assert(sizeof(time_t) <= sizeof(unsigned int));
    Assert(sizeof(int) <= sizeof(unsigned int));
    Assert(sizeof(key_t) <= sizeof(unsigned int));

    //
    marshalNumber(address, mb);
    marshalShort(port, mb);  
    marshalNumber(timestamp.start, mb);
    marshalNumber(timestamp.pid, mb);
    //
    marshalNumber(mailboxKey, mb);
  }

  //
  // Returns 'TRUE' if they are the same;
  Bool cmpVirtualInfos(VirtualInfo *vi) {
    if (address == vi->address && port == vi->port &&
	timestamp.start == vi->timestamp.start &&
	timestamp.pid == vi->timestamp.pid) 
      return (TRUE);
    else 
      return (FALSE);
  }

  //
  // There are NO public 'get' methods for address/port/timestamp!
  key_t getMailboxKey() { return (mailboxKey); }
  void setMailboxKey(key_t mbkIn) { mailboxKey = mbkIn; }
};

//
// Throw away the "virtual info" object representation from a message
// buffer;
void unmarshalUselessVirtualInfo(MsgBuffer *mb);
class VSMessage;

//
// Messages are queued up using the field of the message itself, 
// which is declared in its superclass:
class VSMsgQueueNode {
  friend class VSFreeMessagePool;
  friend class VSMsgQueue;
private:
  VSMessage *next;

  //
protected:
  VSMessage* getNext() { return (next); }
  void setNext(VSMessage *n) { next = n; }

public:
  VSMsgQueueNode() : next((VSMessage *) 0) {}
  ~VSMsgQueueNode() { error("VSMsgQueueNode destroyed??"); }
};

//
// "Messages" are necessary for the case when a message cannot be
// delivered (i.e. put in a mailbox) immediately (when e.g. that
// mailbox is locked or full);
class VSMessage : private VSMsgQueueNode {
  friend class VSFreeMessagePool;
  friend class VSMsgQueue;
private:
  VSMsgBufferOwned *mb;
  MessageType msgType;
  // 'Site*' is not needed (stored in the message buffer);
  int storeIndex;

  //
public:
  //
  void* operator new(size_t size) {
    error("VSMessage allocated using 'new(size_t)'");
    return ((void *) -1);	// gcc warning;
  }
  void* operator new(size_t, void *place) { return (place); }

  //
  VSMessage(VSMsgBufferOwned *mbIn, MessageType mtIn, Site *sIn, int stIn)
    : mb(mbIn), msgType(mtIn), storeIndex(stIn)
  {
    Assert(mbIn->getSite() == sIn);
  }
  ~VSMessage() { error("VSMessage destroyed??"); }

  //
  VSMsgBufferOwned* getMsgBuffer() { return (mb); }
  MessageType getMessageType() { return (msgType); }
  Site *getSite() { return (mb->getSite()); }
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
      freeList = freeList->getNext();
      DebugCode(b->setNext((VSMessage *) 0));
      return (b);
    } else {
      return ((VSMessage *) malloc(sizeof(VSMessage)));
    }
  }
  void dispose(VSMessage *b) {
    Assert(b->getNext() == (VSMessage *) 0);
    b->setNext(freeList);
    freeList = b;
  }
};

//
// A queue of messages waiting for delivery (an element of a "virtual
// site" object);
class VSMsgQueue {
private:
  VSMessage *first, *last;

  //
public:
  VSMsgQueue() : first((VSMessage *) 0), last((VSMessage *) 0) {}
  ~VSMsgQueue() { 
    Assert(first == (VSMessage *) 0);
    Assert(last == (VSMessage *) 0);
  }

  //
  Bool isNotEmpty() { return ((Bool) first); }

  //
  void enqueue(VSMessage *n) {
    if (last) {
      Assert(last->getNext() == (VSMessage *) 0);
      last->setNext(n);
    }
    last = n;
    if (first == (VSMessage *) 0) 
      first = last;
  }

  //
  VSMessage* dequeue() {
    VSMessage *r = first;
    Assert(first != (VSMessage *) 0);
    if (first == last) { 
      first = last = (VSMessage *) 0;
    } else {
      first = first->getNext();
    }
    DebugCode(r->setNext((VSMessage *) 0));
    return (r);
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
class VSSiteQueue;
class VSSiteQueueNode {
  friend class VSSiteQueue;
private:
  Bool isInFlag;		// Every site is stored at most once;
  VirtualSite *nextSiteInQueue;

  //
private:
  //
  Bool isIn() { return (isInFlag); }
  void setIn() { isInFlag = OK; }
  void setOut() { isInFlag = NO; }

  //
  VirtualSite* getNext() { return (nextSiteInQueue); }
  void setNext(VirtualSite *n) { nextSiteInQueue = n; }

  //
public:
  VSSiteQueueNode()
    : isInFlag(NO), nextSiteInQueue((VirtualSite *) 0) {}
  ~VSSiteQueueNode() { Assert(!isInFlag); }
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
class VirtualSite : private VSSiteQueueNode, public VSMsgQueue  {
  friend class VSSiteQueue;
  friend class VSProbingObject;
private:
  Site *site;			// backward (cross) reference;
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
  // 'VSProbingObject' needs pids of virtual sites;
  int getVSPid() { return (mboxMgr->getMailbox()->getPid()); }

  //
  unsigned long getTimeIsAliveSent() { return (isAliveSent); }
  unsigned long getTimeAliveAck() { return (aliveAck); }
  void setTimeIsAliveSent(unsigned long ms) { isAliveSent = ms; }

  //
  Bool hasProbesAll() { return (probeAllCnt); }
  Bool hasProbesPerm() { return (probePermCnt); }
  void incProbesAll() { probeAllCnt++; }
  void incProbesPerm() { probePermCnt++; }
  void decProbesAll() { probeAllCnt--; }
  void decProbesPerm() { probePermCnt--; }

  //
public:
  VirtualSite(Site *s, VSFreeMessagePool *fmpIn, VSSiteQueue *sqIn);
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
	     Site *storeSite, int storeIndex,
	     FreeListDataManager<VSMsgBufferOwned> *freeMBs);
  // ... resend it ('TRUE' if we succeeded);
  Bool tryToSendToAgain(VSMessage *vsm,
		       FreeListDataManager<VSMsgBufferOwned> *freeMBs);

  //
  // The mailbox is mapped/unmapped on these events:
  void zeroReferences() { disconnect(); }

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
  Site *getSite() { return (site); }

  //
  void setTimeAliveAck(unsigned long ms) { aliveAck = ms; }
};

//
// A queue of sites that have some pending messages for delivery;
//
// An object of this class is tried to empty (i.e. to send out pending
// messages to those sites) at regular intervals (tasks; see
// virtual.cc and am.*);
//
// (We cannot use templates for 'VSMsgQueue' and 'VSSiteQueue'
// implementations since their corresponding "queue node" classes 
// should define those queue classes as "friends";)
class VSSiteQueue {
private:
  VirtualSite *first, *last;

  //
public:
  VSSiteQueue() : 
    first((VirtualSite *) 0), last((VirtualSite *) 0) 
  {}
  ~VSSiteQueue() {}

  //
  Bool isEmpty() { return (first == (VirtualSite *) 0); }
  Bool isNotEmpty() { return ((Bool) first); }

  //
  void enqueue(VirtualSite *n) {
    if (!n->isIn()) {
      n->setIn();

      //
      if (last) {
	Assert(last->getNext() == (VirtualSite *) 0);
	last->setNext(n);
      }
      last = n;
      if (first == (VirtualSite *) 0) 
	first = last;
    }
  }

  //
  VirtualSite* dequeue() {
    VirtualSite *r = first;
    Assert(r->isIn());
    r->setOut();

    //
    Assert(first != (VirtualSite *) 0);
    if (first == last) { 
      first = last = (VirtualSite *) 0;
    } else {
      first = first->getNext();
    }
    DebugCode(r->setNext((VirtualSite *) 0));

    //
    return (r);
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
  unsigned int hash(Site *s) { return (s->hashPrimary()); }

  //
  GenHashNode* findNode(int hvalue, Site *s);

  //
protected:
  VSSiteHashTable(int size) : GenHashTable(size) {}
  ~VSSiteHashTable() {}

  //
  Bool check(Site *s);
  void enter(Site *s);
  void remove(Site *s);

  //
  Site *getFirst();
  Site *getNext(Site *prev);
};

//
//
class VSProbingObject : VSSiteHashTable {
private:
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
  VSProbingObject(int size)
    : VSSiteHashTable(size),
      probesNum(0), lastCheck(0), lastPing(0), 
      minInterval(PROBE_INTERVAL) {
    // kost@ : task manager does not keep track who and which minimal
    // service interval has requested, virtual sites just say from
    // scratch "we want to have PROBE_INTERVAL";
    am.setMinimalTaskInterval(PROBE_INTERVAL);
  }
  ~VSProbingObject() {}

  //
  // Note we cannot ignore probe types even though there is only type
  // of real problems - the permanent one. This is because the perdio
  // layer can ask for different probes separately;
  ProbeReturn installProbe(VirtualSite *vs, ProbeType pt, int frequency);
  ProbeReturn deinstallProbe(VirtualSite *vs, ProbeType pt);

  //
  Bool checkProbes(unsigned long clock) {
    if (!probesNum) 
      return (FALSE);

    //
    if (clock - lastCheck > minInterval) {
      lastCheck = clock;
      return (TRUE);
    } else {
      return (FALSE);
    }
  }

  //
  Bool processProbes(unsigned long clock);
};

#endif // VIRTUALSITES

#endif // __VS_COMM_HH
