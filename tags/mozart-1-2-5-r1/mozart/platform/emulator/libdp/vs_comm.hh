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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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
VSMarshalerBufferOwned* composeVSInitMsg();
VSMarshalerBufferOwned* composeVSSiteIsAliveMsg(DSite *s);
VSMarshalerBufferOwned* composeVSSiteAliveMsg(DSite *s, VirtualSite *vs);
VSMarshalerBufferOwned* composeVSSiteDeadMsg(DSite *dest, DSite *ds);
VSMarshalerBufferOwned* composeVSYourIndexHereMsg(DSite *s, int index);
VSMarshalerBufferOwned* composeVSUnusedShmIdMsg(DSite *s, key_t shmid);

//
VSMsgType getVSMsgType(VSMarshalerBufferImported *mb);
//
#ifdef USE_FAST_UNMARSHALER   
void decomposeVSInitMsg(VSMarshalerBuffer *mb, DSite* &s);
void decomposeVSSiteIsAliveMsg(VSMarshalerBuffer *mb, DSite* &s);
void decomposeVSSiteAliveMsg(VSMarshalerBuffer *mb, DSite* &s, VirtualSite* &vs);
void decomposeVSSiteDeadMsg(VSMarshalerBuffer *mb, DSite* &ds, VirtualSite* &dvs);
void decomposeVSYourIndexHereMsg(VSMarshalerBuffer *mb, int &index);
void decomposeVSUnusedShmIdMsg(VSMarshalerBuffer *mb, DSite* &s, key_t &shmid);
#else
void decomposeVSInitMsgRobust(VSMarshalerBuffer *mb, DSite* &s, int* error);
void decomposeVSSiteIsAliveMsgRobust(VSMarshalerBuffer *mb, DSite* &s, int* error);
void decomposeVSSiteAliveMsgRobust(VSMarshalerBuffer *mb, DSite* &s, 
				   VirtualSite* &vs, int* error);
void decomposeVSSiteDeadMsgRobust(VSMarshalerBuffer *mb, DSite* &ds, 
				  VirtualSite* &dvs, int* error);
void decomposeVSYourIndexHereMsgRobust(VSMarshalerBuffer *mb, 
				       int &index, int* error);
void decomposeVSUnusedShmIdMsgRobust(VSMarshalerBuffer *mb, DSite* &s, 
				     key_t &shmid, int* error);
#endif

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
  VSMarshalerBufferOwned *mb;
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
  VSMessage(VSMarshalerBufferOwned *mbIn, MessageType mtIn, DSite *sIn, int stIn)
    : mb(mbIn), msgType(mtIn), storeIndex(stIn)
  {}
  ~VSMessage() { OZ_error("VSMessage destroyed??"); }

  //
  VSMarshalerBufferOwned* getMarshalerBuffer() { return (mb); }
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
// Resource manager is responsible for reclaiming of currently unused
// resources. It also keeps track of allocations/deallocations of
// those resources. Note that 'created' mailboxes are not counted,
// since those are transient;
enum RMState {
  VS_RM_idle = 0,
  VS_RM_scavenge,
  VS_RM_unmapMBufs,
  VS_RM_scavengeBurst,		// scavenging with low 'usage' values;
  VS_RM_disconnect,
  VS_RM_wait
};
enum VS_RM_Source {
  VS_RM_OwnSeg = 0,
  VS_RM_ImpSeg,
  VS_RM_Mailbox,
  VS_RM_NewMbox
};
enum VS_RM_Reason {
  VS_RM_Create = 0,
  VS_RM_Attach
};
enum VS_RM_GCMode {
  VS_RM_Block = 0,
  VS_RM_Nonblock
};

//
// Simulate resource deficiency, aka in Solaris with default settings;
// #define VS_DEBUG_RESOURCES
#define VS_DEBUG_SHMPAGES     6

//
class VSRegister;
class VSResourceManager {
private:
  RMState state;
  //
  int mapped;
  DebugCode(int seqMapped;);
  //
  VSRegister *vsRegister;
  VSMsgChunkPoolManagerOwned *myCPM;
  VSMsgChunkPoolManagerImported *impCPM;
  // starting VS for the next 'resourceGC' round:
  int currentStart;

  // We cannot neither disconnect the site we are receiving a message
  // from nor unmap any msg buffers from that site (since it's not
  // known which of those carry the current message);
  VirtualSite *peerSite;

  //
public:
  VSResourceManager()
    : state(VS_RM_idle), mapped(0), currentStart(0),
      vsRegister((VSRegister *) 0) DebugArg(seqMapped(0)),
      myCPM((VSMsgChunkPoolManagerOwned *) 0), 
      peerSite((VirtualSite *) 0),
      impCPM((VSMsgChunkPoolManagerImported *) 0) {}
  void init(VSRegister *vsRIn,
	    VSMsgChunkPoolManagerOwned *myCPMIn,
	    VSMsgChunkPoolManagerImported *impCPMIn) {
    vsRegister = vsRIn;
    myCPM = myCPMIn;
    impCPM = impCPMIn;
  }
  ~VSResourceManager() {}

  // 
  // These should be called by corresponding delete/unmap methods;
  void shmPageMapped() {
    mapped++;
    DebugCode(seqMapped++;);
  }
  void shmPageUnmapped() { mapped--; }
  int getMapped() { return (mapped); }

  //
#ifdef VS_DEBUG_RESOURCES
  Bool canAllocate() { return (mapped < VS_DEBUG_SHMPAGES); }
#endif

  //
  void startMsgReceived(VirtualSite *ps) { peerSite = ps; }
  void finishMsgReceived() { peerSite = (VirtualSite *) 0; }

  //
  // Particular cleaning/scavenging/etc. methods are controlled by
  // these hooks:
  int getVSMsgChunksUsage() {
    if (state == VS_RM_scavengeBurst)
      return (VS_SEGS_MAXIDLE_PHASES_GC);
    else
      return (VS_MSGCHUNKS_USAGE);
  }
  int getVSSegsMaxIdlePhases() {
    if (state == VS_RM_scavengeBurst)
      return (VS_SEGS_MAXIDLE_PHASES_GC);
    else
      return (VS_SEGS_MAXIDLE_PHASES);
  }

  //
  Bool doResourceGC(VS_RM_Source source, VS_RM_Reason reason,
		    VS_RM_GCMode gcmode);
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
// Internal state of a virtual site, i.e. details that are (must be)
// hidden from the communication layer (i.e. the 'Site' object);
#define VS_NONE		0x0

//
// That's the thing which is referenced by a site object denoting a
// virtual site, and created whenever something is to be sent there;
class VirtualSite : public VSSiteQueueNode, 
		    public VSMsgQueue, 
		    public VSRegisterNode
{
  friend class VSSiteQueue;
  friend class VSProbingObject;
private:
  DSite *site;			// backward (cross) reference;
  // 'vsIndex' is an index at the remote site (that is represented by
  // this VSite) of a VSite representing the local site. It is
  // assigned by the remote site and communicated here using the
  // 'VS_M_YOUR_INDEX_HERE' vs comm layer service message;
  int vsIndex;			// originally -1;
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
  // Keeps track of imported mailboxes' shm keys. These keys are used
  // for unmapping corresponding shm pages when the (low-level)
  // connection is taken down. The register is updated whenever a new
  // mbuffer segment is imported, and when an 'UNUSED_SHMID' message
  // is received;
  VSSegKeysRegister keysRegister;

  //
  VSMailboxManagerImported *mboxMgr; // ... of that virtual site;
  //
  // 'cpmImp' is the global entity; virtual sites need to keep it
  // because of resource management;
  VSMsgChunkPoolManagerImported *cpmImp;

  //
  // Message bodies are allocated from this pool:
  VSFreeMessagePool *fmp;

  // If 'sendTo' fails to deliver something 'inline', it puts itself
  // (the virtual site) into the queue:
  VSSiteQueue *sq;

  //
  // Resource manager;
  VSResourceManager *vsRM;

  //
  // GC of lost resources: whenever the site ("this" site) is
  // discovered to be dead, this list of segment keys is used for
  // destroying corresponding shm pages (if they were not already, in
  // which case that's a noop). This info is received by piggy-backing
  // on "alive!" acks from virtual sites (speak, virtual site just
  // composes the list of local resources into that 'ack');
  int segKeysNum, segKeysArraySize;
  key_t *segKeys;

  //
private:
  //
  void setStatus(SiteStatus statusIn) { status = statusIn; }

  //
  void connect();

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
  // 'gcResources()' is good for releasing shm pages (thus, unmapping
  // them) when the (low-level) connection is closed down, i.e. due to
  // the lack of resources. 'killResources' kills (marks destroyed) in
  // addition all the pages that are known to be owned by a dead site;
  void gcResources();
public:
  void killResources();

  //
public:
  VirtualSite(DSite *s, VSFreeMessagePool *fmpIn, VSSiteQueue *sqIn, 
	      VSResourceManager* vsRMin, 
	      VSMsgChunkPoolManagerImported *cpmImpIn);
  ~VirtualSite() { 
    Assert(status == SITE_PERM);
    DebugCode((int32 &) site |= 0x1);
  }

  //
  Bool isConnected() { return ((Bool) mboxMgr); }
  // resource manager is allowed to disconnect;
  void disconnect();

  //
  int getVSIndex() { return (vsIndex); }
  void setVSIndex(int vsiIn) {
    // it may have been arrived already;
    Assert(vsIndex == -1 || vsIndex == vsiIn);
    vsIndex = vsiIn;
  }

  //
  SiteStatus getSiteStatus() { return (status); }

  // 'drop' means that the virtual site will not be used anymore 
  // since it is known to be dead (e.g. from a third party). 
  void drop();

  //
  // The message type, store site and store index parameters 
  // are opaque data (just stored);
  int sendTo(VSMarshalerBufferOwned *mb, MessageType mt,
	     DSite *storeSite, int storeIndex,
	     FreeListDataManager<VSMarshalerBufferOwned> *freeMBs);
  // ... resend it ('TRUE' if we succeeded);
  Bool tryToSendToAgain(VSMessage *vsm,
			FreeListDataManager<VSMarshalerBufferOwned> *freeMBs);

  //
  void zeroReferences() { 
    if (isConnected())
      disconnect();
  }

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
  int getVSPid() {
    Assert(isConnected());
    return (mboxMgr->getMailbox()->getPid());
  }

  //
  void setTimeAliveAck(unsigned long ms) { aliveAck = ms; }

  //
  // When cleaning up upon exiting, slave virtual sites known to us
  // are explicitely killed. Note that 'isSlave()' is not complete: it
  // returns 'NO' for disconnected slaves;
  Bool isSlave() {
    // must be connected in order to know its pid;
    return (isConnected() && isLocalKey(mboxMgr->getSHMKey()));
  }

  //
  // Drop an imported mbuffer's segment manager from the register;
  VSSegKeysRegister* getKeysRegister() { return (&keysRegister); }
  // There is no "add" method: an "imported" message buffer gets hold
  // of the VS's 'VSSegKeysRegister' directly;
  void dropSegManager(key_t key) {
    // Note that 'retract()' can be an empty action - resource manager
    // could have discarded it already;
    keysRegister.retract(key);
  }

  //
  // 'alive ack' message contains also a list of ipc keys - of shared
  // memory pages (mailboxes, chunk pool segments, may be even
  // something else?). These are maintained using these methods:
  void marshalLocalResources(MarshalerBuffer *mb,
			     VSMailboxManagerOwned *mbm,
			     VSMsgChunkPoolManagerOwned *cpm);
  void unmarshalResources(MarshalerBuffer *mb); 
  void unmarshalResourcesRobust(MarshalerBuffer *mb, int *error);
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
  // for instance, the resource manager has to know how many virtual 
  // sites are there;
  int size;

  //
public:
  VSRegister()
    : VSRegisterNode((VirtualSite *) -1), next((VSRegisterNode *) 0),
      size(0)
  {
    setNextVSRegisterNode(this), setPrevVSRegisterNode(this);
  }
  ~VSRegister() {}

  //
  Bool isEmpty() {
    Assert((size == 0) == (getNextVSRegisterNode() == this));
    return (getNextVSRegisterNode() == this);
  }
  Bool isNotEmpty() {
    Assert((size == 0) == (getNextVSRegisterNode() == this));
    return (getNextVSRegisterNode() != this);
  }
  int getSize() { return (size); }

  //
  void registerVS(VSRegisterNode *n) {
    VSRegisterNode *p = getPrevVSRegisterNode();

    //
    setPrevVSRegisterNode(n);
    n->setPrevVSRegisterNode(p);
    p->setNextVSRegisterNode(n);
    n->setNextVSRegisterNode(this);
    //
    size++;
  }
  void retractVS(VSRegisterNode *n) { 
    n->retractVSRegisterNode();
    //
    size--;
  }

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

  //
  // Resource manager tries to reclaim something from arbitrarily
  // chosen (physically connected) virtual sites. For doing so we must
  // be able to start sequential traversing not from the first
  // element, as well as to traverse the list circularly (i.e. from
  // the last node to the first one):
  VirtualSite *getNth(int num) {
    if (size <= 0) return ((VirtualSite *) 0);
    num = num%size;

    //
    VSRegisterNode *n = getNextVSRegisterNode();
    while (num > 0) {
      n = n->getNextVSRegisterNode();
      Assert(n);
      num--;
    }
    //
    next = n->getNextVSRegisterNode();
    return (n->getVS());
  }

  //
  // (never returns zero;)
  VirtualSite *getNextCircular() {
    Assert(size);
    while (1) {
      VSRegisterNode *n;

      //
      n = next;
      if (n == this) {
	next = getNextVSRegisterNode();
      } else {
	next = n->getNextVSRegisterNode();
	return (n->getVS());
      }
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
  void enter(DSite *s);
  void remove(DSite *s);
  Bool check(DSite *s);

  //
  DSite *getFirst();
  DSite *getNext();
};

//
//
class VSProbingObject : private VSSiteHashTable {
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
  // kost@ : now probes are implicit on every site, so the following
  // three functions are obsolete:
  //
  // Note we cannot ignore probe types even though there is only type
  // of real problems - the permanent one. This is because the perdio
  // layer can ask for different probes separately;
  //
  // ProbeReturn installProbe(VirtualSite *vs, ProbeType pt, int frequency);
  // ProbeReturn deinstallProbe(VirtualSite *vs, ProbeType pt);
  //
  // Bool isProbed(DSite *s) { return (check(s)); }

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

//
// 'VSTable' object maps indexes to virtual sites (see virtual.cc);
class VSTable {
private:
  VirtualSite **tab;
  int size;
  int cnt;

  //
private:
  void enlarge(int sizeIn) {
    Assert(sizeIn >= size);
    VirtualSite **newtab = new VirtualSite*[sizeIn];
    int i;
    for (i = 0; i < size; i++)
      newtab[i] = tab[i];
    for (; i < sizeIn; i++)
      newtab[i] = (VirtualSite *) 0;
    size = sizeIn;
  }

  //
public:
  VSTable(int sizeIn) : size(sizeIn), cnt(0) {
    tab = new VirtualSite*[size];
    for (int i = 0; i < size; i++)
      tab[i] = (VirtualSite *) 0;
  }
  ~VSTable() { 
    delete tab;
    DebugCode(tab = (VirtualSite **) 0;);
    DebugCode(size = cnt = 0;);
  }

  //
  VirtualSite* operator[](int i) { 
    Assert(i >= 0 && i < size);
    return (tab[i]);
  }

  //
  // put it in it is not already in there. The last can happen when a
  // site first sends out a couple of messages without obtaining the
  // index from the peer;
  int put(VirtualSite *vs) {
    int free = -1;
    int nseen = cnt;
    int index = 0;

    // scanning;
    while (nseen) {
      Assert(index < size);
      if (!tab[index]) {
	if (free < 0)
	  free = index;
      } else {
	if (tab[index] == vs)
	  return (index);
	nseen--;
      }
      index++;
    }

    // not found:
    if (free < 0) {
      if (index >= size) {
	free = size;
	// 'size' changes as well;
	enlarge(size*2);
      } else {
	free = index;
	Assert(!tab[free]);
      }
    }

    //
    tab[free] = vs;
    cnt++;
    return (free);
  }

  //
  void drop(VirtualSite *vs) {
    for (int i = 0; i < size; i++) {
      if (tab[i] == vs) {
	cnt--;
	tab[i] = (VirtualSite *) 0;
      }
    }
  }
  //
#ifdef DEBUG_CHECK  
  void release(int i) {
    cnt--;
    tab[i] = (VirtualSite *) 0;
  }
#endif
};

#endif // VIRTUALSITES

#endif // __VS_COMM_HH
