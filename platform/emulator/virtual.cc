/*
 *  Authors:
 *    Konstantin Popov, Per Brand
 * 
 *  Contributors:
 *
 *  Copyright:
 *    Konstantin Popov, Per Brand 1997-1998
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

#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#include "perdio.hh"
#include "perdio_debug.hh"  
#include "genvar.hh"
#include "perdiovar.hh"
#include "gc.hh"
#include "dictionary.hh"
#include "urlc.hh"
#include "marshaler.hh"
#include "comm.hh"
#include "msgbuffer.hh"
#include "builtins.hh"
#include "vs_mailbox.hh"
#include "vs_msgbuffer.hh"
#include "vs_comm.hh"

/// 
/// (semi?) constants;
///

//
// The size of a mailbox in messages;
const int vs_mailbox_size = (1024 * 1024) / sizeof(VSMailboxMsg);

//
// The number of chunks in a message buffers pool
// (currently that's just a constant, but that could be changed);
const int vs_chunk_size = 1024;
const int vs_chunks_num = 4096;
//
// Currently 4MB altogether;

//
// 12,5% fill-up for 32 sites?
const int vsRegisterHTSize = 256;

///
/// Static managers
/// (in the *file* scope, and most of them - with indefinite extent);
///
//
// Free bodies of "message buffers" (message buffers are used storing
// incoming/outgoing messages while marshaling-sending resp.
// receiving-unmarshaling):
static FreeListDataManager<VSMsgBuffer> freeMsgBufferPool(malloc);

//
// Free bodies of "messages" (which are used for keeping unsent
// messages). Note that there is its own implementation!
static VSFreeMessagePool freeMessagePool;

//
// Imported mailboxes (used for sending messages to neighbor VS"s):
static VSMailboxRegister mailboxRegister(vsRegisterHTSize);

//
// Imported pools of chunks for message buffers (incoming messages are
// read from there):
static VSChunkPoolRegister chunkPoolRegister(vsRegisterHTSize);

//
// Free bodies of "virtual info" objects.
// We need a memory management for that since there can be "virtual
// info" objects for "remote" virtual site groups;
static FreeListDataManager<VirtualInfo> freeVirtualInfoPool(malloc);

//
// A queue of sites that have pending messages (still to be sent):
static VSSiteQueue vsSiteQueue;

//
// There are also two managers that are allocated explicitly:
static VSMailboxManager *myVSMailboxManager;
static VSMsgChunkPoolManager *myVSChunksPoolManager;

//
//
// The 'check'&'process' procedures for processing incoming messages
// (see am.hh, class 'TaskNode'):
// static TaskCheckProc checkVSMessages;
static Bool checkVSMessages(void *mbox);
// ... and the read handler itself:
// static TaskProcessProc readVSMessages;
static void readVSMessages(void *mbox);

//
// ... and the pair for processing unsent messages:
// static TaskCheckProc checkMessageQueue;
static Bool checkMessageQueue(VSSiteQueue *sq);
// static TaskProcessProc processMessageQueue;
static void processMessageQueue(VSSiteQueue *sq);

///
/// (static) interface methods for virtual sites;
///

//
VirtualSite* createVirtualSite(Site* s)
{
  return (new VirtualSite(s, &freeMessagePool, &vsSiteQueue));
}

//
void marshalVirtualInfo(VirtualInfo *vi, MsgBuffer *mb)
{
  vi->marshal(mb);
}
//
VirtualInfo* unmarshalVirtualInfo(MsgBuffer *mb)
{
  VirtualInfo *vi = freeVirtualInfoPool.allocate();
  vi->VirtualInfo::VirtualInfo(mb);
  return (vi);
}
//
// Defined in vs_comm.cc;
void unmarshalUselessVirtualInfo(MsgBuffer*);

//
void zeroRefsToVirtual(VirtualSite *vs)
{
  vs->zeroReferences();
}

//
// The 'mt', 'storeSite' and 'storeIndex' parameters are used whenever a 
// communication layer reports a problem with a message which has
// been previously accepted for delivery;
int sendTo_VirtualSite(VirtualSite *vs, MsgBuffer *mb, 
		       MessageType mt, Site *storeSite, int storeIndex)
{
  return (vs->sendTo((VSMsgBuffer *) mb, mt, storeSite, storeIndex));
}

//
// By now, there can be no unsent messages - so,
// 'VirtualSite::discardUnsentMessage()' does nothing;
int discardUnsentMessage_VirtualSite(VirtualSite *vs, int msgNum)
{
  return (vs->discardUnsentMessage(msgNum));
}

//
// (There are queues, but they are transparent by now: 
int getQueueStatus_VirtualSite(VirtualSite *vs, int &noMsgs)
{
  return (vs->getQueueStatus(noMsgs));
}

//
SiteStatus siteStatus_VirtualSite(VirtualSite* vs)
{
  return (vs->getSiteStatus());
}

//
// There are no monitors now;
MonitorReturn
monitorQueue_VirtualSite(VirtualSite *vs, 
			 int size, int no_msgs, void *storePtr)
{
  return (NO_MONITOR_EXISTS);
}

//
MonitorReturn demonitorQueue_VirtualSite(VirtualSite* vs)
{
  return (NO_MONITOR_EXISTS);
}

//
// There are no probes by now.
// Probes can be implemented with dummy messages which must read
// (e.g. freed) by the receiver;
ProbeReturn
installProbe_VirtualSite(VirtualSite *vs,
			 ProbeType pt, int frequency, void *storePtr)
{
  return (PROBE_NONEXISTENT);
}

//
ProbeReturn deinstallProbe_VirtualSite(VirtualSite*, ProbeType pt)
{
  return (PROBE_NONEXISTENT);
}

//
ProbeReturn
probeStatus_VirtualSite(VirtualSite *vs,
			ProbeType &pt, int &frequncey, void* &storePtr)
{
  return (PROBE_NONEXISTENT);
}

//
// A virtual site should not be given up now since there are no
// temporary problems for virtual sites;
GiveUpReturn giveUp_VirtualSite(VirtualSite* vs)
{
  error("Virtual Site is given up!??");
  return (SITE_NOW_NORMAL);
}

//
// 'discoveryPerm' means that a site is known (from a third party) to
// be dead.
void discoveryPerm_VirtualSite(VirtualSite *vs)
{
  vs->drop();
  delete vs;
}

//
void dumpVirtualInfo(VirtualInfo* vi)
{
  vi->destroy();
  freeVirtualInfoPool.dispose(vi);
}

//
MsgBuffer* getVirtualMsgBuffer(Site* site)
{
  VSMsgBuffer *buf = freeMsgBufferPool.allocate();
  buf->VSMsgBuffer::VSMsgBuffer(myVSChunksPoolManager, site);

  //
  return (buf);			// upcast;
}

//
//
static Bool checkVSMessages(void *vMBox)
{
  // unsafe by now - some magic number should be added;
  VSMailbox *mbox = (VSMailbox *) vMBox;
  return ((Bool) mbox->getSize());
}

//
static void readVSMessages(void *vMBox)
{
  // unsafe by now - some magic number(s) should be added;
  VSMailbox *mbox = (VSMailbox *) vMBox;
  int msgs = MAX_MSGS_ATONCE;

  //
  // 
  while (mbox->isNotEmpty() && msgs) {
    key_t msgChunkPoolKey;
    int chunkNumber;

    //
    if (mbox->dequeue(msgChunkPoolKey, chunkNumber)) {
      // got a message;
      //
      VSMsgBuffer *buf = freeMsgBufferPool.allocate();
      VSMsgChunkPoolManager *cpm = chunkPoolRegister.import(msgChunkPoolKey);
      buf->VSMsgBuffer::VSMsgBuffer(cpm, chunkNumber);

      //
      // Note that the mailbox is NOT locked at this place;
      msgReceived(buf);

      //
      buf->releaseChunks();
      freeMsgBufferPool.dispose(buf);
    } else {
      // is locked - then let's try to read later;
      return;
    }

    //
    msgs--;
  }
}

//
// (see comments few lines below;)
#define	VSQueueMark	((VirtualSite *) -1)
#define	VSMsgQueueMark	((VSMessage *) -1)

//
//
static Bool checkMessageQueue(void *sqi)
{
  // unsafe by now - some magic number should be added;
  VSSiteQueue *sq = (VSSiteQueue *) sqi;
  return (sq->isNotEmpty());
}

//
static void processMessageQueue(void *sqi)
{
  // unsafe by now - some magic number should be added;
  VSSiteQueue *sq = (VSSiteQueue *) sqi;
  Assert(sq->isNotEmpty());

  //
  // Observe that a virtual site/message can be put again into queues,
  // so a simple loop "while 'is not empty'" would be endless. A
  // special mark is put in the queues, and elements are picked up to
  // that mark;
  sq->enqueue(VSQueueMark);

  //
  while (1) {
    VirtualSite *vs = sq->dequeue();
    if (vs == VSQueueMark)
      break;			// ready;

    //    
    Assert(vs->isNotEmpty());
    vs->enqueue(VSMsgQueueMark);
    while (1) {
      VSMessage *vsm = vs->dequeue();
      if (vsm == VSMsgQueueMark)
	break;			// ready;

      // 
      vs->tryToSendToAgain(vsm);
    }
  }
}

//
#undef	VSQueueMark
#undef	VSMsgQueueMark

//
void dumpVirtualMsgBuffer(MsgBuffer* m)
{
  VSMsgBuffer *buf = (VSMsgBuffer *) m;

  buf->releaseChunks();
  freeMsgBufferPool.dispose(buf);
}

//
// Builtins for virtual sites - only two of them are needed:
// (I) Creating a new mailbox (at the master site):
OZ_BI_define(BIVSnewMailbox,0,1)
{
  //
  VSMailboxManager *mbm;
  char keyChars[sizeof(key_t)+3]; // in the form "0xNNNNNNNN";

  //
  // Check whether this site is (already) a virtual one:
  if (mySite->hasVirtualInfo()) {
    VirtualInfo *vi;

    //
    myVSMailboxManager = new VSMailboxManager((long) vs_mailbox_size);
    myVSChunksPoolManager =
      new VSMsgChunkPoolManager(vs_chunk_size, vs_chunks_num);

    //    
    vi = new VirtualInfo(mySite, myVSMailboxManager->getSHMKey());
    mySite->makeMySiteVirtual(vi);

    //
    // Install the 'read-from-virtual-sites' & 'delayed-write' handlers;
    am.registerTask((void *) myVSMailboxManager->getMailbox(),
		    checkVSMessages, readVSMessages);
    am.registerTask((void *) &vsSiteQueue,
		    checkMessageQueue, processMessageQueue);
  }

  //
  // The 'VSMailboxManager' always creates a proper object (or crashes
  // the system if it can't);
  mbm = new VSMailboxManager((long) vs_mailbox_size);
  mbm->unmap();			// we don't need that object now anymore;
  delete mbm;

  //
  Assert(sizeof(key_t) <= sizeof(int));
  sprintf(keyChars, "0x%x", mbm->getSHMKey());
  OZ_RETURN(OZ_string(keyChars));
} OZ_BI_end

//
// (II) Initializing a virtual site given its mailbox (which contains
// also the parent's id);
OZ_BI_define(BIVSinitServer,1,0)
{
  //
  VirtualInfo *vi;
  key_t msgChunkPoolKey;
  int chunkNumber;
  OZ_declareVirtualStringIN(0, mbKeyChars);
  Assert(sizeof(key_t) <= sizeof(int));
  key_t mbKey = (key_t) atoi(mbKeyChars);
  key_t cpKey = myVSChunksPoolManager->getSHMKey();
  VSMailbox *mbox;

  //
  if (myVSMailboxManager)
    return oz_raise(E_ERROR,E_SYSTEM,"initServer again",0);

  //
  // We know here the id of the mailbox, so let's fetch it:
  myVSMailboxManager = new VSMailboxManager(mbKey);
  myVSChunksPoolManager =
    new VSMsgChunkPoolManager(vs_chunk_size, vs_chunks_num);

  //
  // Now, let's process the initialization message (M_INIT_VS).
  // Processing of it would result in making 'mySite' a virtual one, 
  // and creating&registering a site object for the (immediate!) 
  // master object;
  mbox = myVSMailboxManager->getMailbox();
  if (mbox->isNotEmpty()) {
    if (!mbox->dequeue(msgChunkPoolKey, chunkNumber))
      return oz_raise(E_ERROR,E_SYSTEM,"mailboxLocked",1,
		      oz_atom(mbKeyChars));
  } else {
    return oz_raise(E_ERROR,E_SYSTEM,"mailboxEmpty",1,
		    oz_atom(mbKeyChars));
  }

  //
  VSMsgBuffer *buf = freeMsgBufferPool.allocate();
  VSMsgChunkPoolManager *cpm = chunkPoolRegister.import(msgChunkPoolKey);
  buf->VSMsgBuffer::VSMsgBuffer(cpm, chunkNumber);
  msgReceived(buf);

  //
  // Now, mySite's virtual info is updated to have a shared memory key
  // (which is not available in the context of 'msgReceived()');
  Assert(mySite->getVirtualInfo());
  mySite->getVirtualInfo()->setMailboxKey(myVSMailboxManager->getSHMKey());

  //
  // Install the 'read-from-virtual-sites' & 'delayed-write' handlers;
  am.registerTask((void *) myVSMailboxManager->getMailbox(),
		  checkVSMessages, readVSMessages);
  am.registerTask((void *) &vsSiteQueue,
		  checkMessageQueue, processMessageQueue);

  //
  return (PROCEED);
} OZ_BI_end

///
/// An exit hook - reclaim shared memory;
///
void virtualSitesExit()
{
  if (myVSMailboxManager) {
    myVSMailboxManager->destroy();
    delete myVSMailboxManager;
    myVSMailboxManager = (VSMailboxManager *) 0;
  }
  if (myVSChunksPoolManager) {
    myVSChunksPoolManager->destroy();
    delete myVSChunksPoolManager;
    myVSChunksPoolManager = (VSMsgChunkPoolManager *) 0;
  }
}

