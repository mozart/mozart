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

#include "resources.hh"
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

#ifdef VIRTUALSITES

///
/// (semi?) constants;
///

//
// The size of a mailbox in messages;
const int vs_mailbox_size = VS_MAILBOX_SIZE / sizeof(VSMailboxMsg);

//
// The number of chunks in a message buffers pool
// (currently that's just a constant, but that could be changed);
const int vs_chunk_size = VS_CHUNK_SIZE;
const int vs_chunks_num = VS_CHUNKS_NUM;
//
// Currently 4MB altogether;

//
const int vsRegisterHTSize = VS_REGISTER_HT_SIZE;

///
/// Static managers
/// (in the *file* scope, and most of them - with indefinite extent);
///
//
// Free bodies of "message buffers" (message buffers are used for
// storing incoming/outgoing messages while marshaling&sending;
static FreeListDataManager<VSMsgBufferOwned> freeMsgBufferPool(malloc);
static VSMsgBufferImported *myVSMsgBufferImported;

//
// Free bodies of "messages" (which are used for keeping unsent
// messages). Note that there is its own implementation!
static VSFreeMessagePool freeMessagePool;

//
// Imported mailboxes (used for sending messages to neighbor VS"s):
static VSMailboxRegister mailboxRegister(vsRegisterHTSize);

//
// Created mailboxes (their keys);
static VSMailboxKeysRegister slavesRegister;

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
static VSMailboxManagerOwned *myVSMailboxManager;
static VSMsgChunkPoolManagerOwned *myVSChunksPoolManager;

//
//
// The 'check'&'process' procedures for processing incoming messages
// (see am.hh, class 'TaskNode'):
// static TaskCheckProc checkVSMessages;
static Bool checkVSMessages(double clock, void *mbox);
// ... and the read handler itself:
// static TaskProcessProc readVSMessages;
static Bool readVSMessages(double clock, void *mbox);

//
// ... and the pair for processing unsent messages:
// static TaskCheckProc checkMessageQueue;
static Bool checkMessageQueue(double clock, VSSiteQueue *sq);
// static TaskProcessProc processMessageQueue;
static Bool processMessageQueue(double clock, VSSiteQueue *sq);

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
  VirtualInfo *voidVI = freeVirtualInfoPool.allocate();
  VirtualInfo *vi = new (voidVI) VirtualInfo(mb);
  return (vi);
}
//
// Defined in vs_comm.cc;
void unmarshalUselessVirtualInfo(MsgBuffer *);

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
  return (vs->sendTo((VSMsgBufferOwned *) mb, mt, storeSite, storeIndex,
                     &freeMsgBufferPool));
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
  warning("'monitorQueue_VirtualSite()' is not implemented!");
  return (MONITOR_OK);
}

//
MonitorReturn demonitorQueue_VirtualSite(VirtualSite* vs)
{
  return (MONITOR_OK);
}

//
// There are no probes by now.
// Probes can be implemented with dummy messages which must read
// (e.g. freed) by the receiver;
ProbeReturn
installProbe_VirtualSite(VirtualSite *vs,
                         ProbeType pt, int frequency, void *storePtr)
{
  Assert(frequency > 0);
  return (PROBE_INSTALLED);
}

//
ProbeReturn deinstallProbe_VirtualSite(VirtualSite*, ProbeType pt)
{
  return (PROBE_DEINSTALLED);
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
// While iterating over the 'vsSiteQueue' we need to stop somewhere:
#define VSQueueMark     ((VirtualSite *) -1)

//
// 'discoveryPerm' means that a site is known (from a third party) to
// be dead.
void discoveryPerm_VirtualSite(VirtualSite *vs)
{
  //
  // If there are unsent messages, then the site is in the vsSiteQueue
  // and must be removed out there;
  if (vs->isNotEmpty()) {
    vsSiteQueue.enqueue(VSQueueMark);

    //
    while (1) {
      VirtualSite *vsq = vsSiteQueue.dequeue();
      if (vs == VSQueueMark)
        break;                  // ready;
      if (vsq != vs)
        vsSiteQueue.enqueue(vsq);
    }
  }

  //
  vs->drop();
  delete vs;
}

//
#undef  VSQueueMark

//
void dumpVirtualInfo(VirtualInfo* vi)
{
  vi->destroy();
  freeVirtualInfoPool.dispose(vi);
}

//
MsgBuffer* getVirtualMsgBuffer(Site* site)
{
  VSMsgBufferOwned *voidBUF = freeMsgBufferPool.allocate();
  VSMsgBufferOwned *buf =
    new (voidBUF) VSMsgBufferOwned(myVSChunksPoolManager, site);

  //
  return (buf);                 // upcast (actually, even 2 steps);
}

//
//
static Bool checkVSMessages(double clock, void *vMBox)
{
  // unsafe by now - some magic number should be added;
  VSMailboxOwned *mbox = (VSMailboxOwned *) vMBox;
  return ((Bool) mbox->getSize());
}

//
static Bool readVSMessages(double clock, void *vMBox)
{
  // unsafe by now - some magic number(s) should be added;
  VSMailboxOwned *mbox = (VSMailboxOwned *) vMBox;
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
      VSMsgChunkPoolManagerImported *cpm =
        chunkPoolRegister.import(msgChunkPoolKey);
      myVSMsgBufferImported->
        VSMsgBufferImported::VSMsgBufferImported(cpm, chunkNumber);
      //
      // Note that the mailbox is NOT locked at this place;
      msgReceived(myVSMsgBufferImported);

      //
      myVSMsgBufferImported->cleanup();
    } else {
      // is locked - then let's try to read later;
      return (FALSE);
    }

    //
    msgs--;
  }

  //
  return (TRUE);
}

//
// (see comments few lines below;)
#define VSQueueMark     ((VirtualSite *) -1)
#define VSMsgQueueMark  ((VSMessage *) -1)

//
//
static Bool checkMessageQueue(double clock, void *sqi)
{
  // unsafe by now - some magic number should be added;
  VSSiteQueue *sq = (VSSiteQueue *) sqi;
  return (sq->isNotEmpty());
}

//
static Bool processMessageQueue(double clock, void *sqi)
{
  // unsafe by now - some magic number should be added;
  VSSiteQueue *sq = (VSSiteQueue *) sqi;
  Assert(sq->isNotEmpty());
  Bool ready = TRUE;

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
      break;                    // ready;

    //
    Assert(vs->isNotEmpty());
    vs->enqueue(VSMsgQueueMark);
    while (1) {
      VSMessage *vsm = vs->dequeue();
      if (vsm == VSMsgQueueMark)
        break;                  // ready;

      //
      ready = ready && vs->tryToSendToAgain(vsm, &freeMsgBufferPool);
    }
  }

  //
  return (ready);
}

//
#undef  VSQueueMark
#undef  VSMsgQueueMark

//
void dumpVirtualMsgBuffer(MsgBuffer* m)
{
  VSMsgBufferOwned *buf = (VSMsgBufferOwned *) m;

  buf->releaseChunks();
  buf->cleanup();
  freeMsgBufferPool.dispose(buf);
}

//
// Builtins for virtual sites - only two of them are needed:
// (I) Creating a new mailbox (at the master site):
OZ_BI_define(BIVSnewMailbox,0,1)
{
  //
  VSMailboxManagerCreated *mbm;
  VSMsgBufferOwned *voidBUF, *buf;
  char keyChars[sizeof(key_t)*2 + 3]; // in the form "0xNNNNNNNN";

  //
  // Check whether this site is (already) a virtual one:
  if (!mySite->hasVirtualInfo()) {
    VirtualInfo *vi;
    key_t myMBoxKey;

    //
    mbm = new VSMailboxManagerCreated(vs_mailbox_size);
    myMBoxKey = mbm->getSHMKey();
    mbm->unmap();
    delete mbm;
    DebugCode(mbm = (VSMailboxManagerCreated *) 0);

    myVSMailboxManager = new VSMailboxManagerOwned(myMBoxKey);
    myVSChunksPoolManager =
      new VSMsgChunkPoolManagerOwned(vs_chunk_size, vs_chunks_num);
    myVSMsgBufferImported =
      (VSMsgBufferImported *) malloc(sizeof(VSMsgBufferImported));

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
  mbm = new VSMailboxManagerCreated(vs_mailbox_size);
  slavesRegister.add(mbm->getSHMKey());

  //
  // Put the 'M_INIT_VS' message into it;
  voidBUF = freeMsgBufferPool.allocate();
  buf = new (voidBUF) VSMsgBufferOwned(myVSChunksPoolManager, (Site *) -1);
  marshal_M_INIT_VS(buf, mySite);
  if (!mbm->getMailbox()->enqueue(buf->getSHMKey(),
                                  buf->getFirstChunk()))
    error("Virtual sites: unable to put the M_INIT_VS message");

  //
  mbm->unmap();                 // we don't need that object now anymore;
  Assert(sizeof(key_t) <= sizeof(int));
  sprintf(keyChars, "0x%x", mbm->getSHMKey());
  delete mbm;

  //
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
  key_t mbKey;
  VSMailboxOwned *mbox;

  //
  Assert(sizeof(key_t) == sizeof(int));
  if (sscanf(mbKeyChars, "%i", &mbKey) != 1)
    return oz_raise(E_ERROR,E_SYSTEM,"VSinitServer: invalid arg",0);

  //
  // new process group - otherwise failed virtual sites will kill
  // the whole virtual site group ;-)
  if (setpgid(0, 0))
    error("failed to form a new process group");

  //
  if (myVSMailboxManager)
    return oz_raise(E_ERROR,E_SYSTEM,"VSinitServer again",0);

  //
  // We know here the id of the mailbox, so let's fetch it:
  myVSMailboxManager = new VSMailboxManagerOwned(mbKey);
  myVSChunksPoolManager =
    new VSMsgChunkPoolManagerOwned(vs_chunk_size, vs_chunks_num);

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
  myVSMsgBufferImported =
    (VSMsgBufferImported *) malloc(sizeof(VSMsgBufferImported));
  VSMsgChunkPoolManagerImported *cpm =
    chunkPoolRegister.import(msgChunkPoolKey);
  myVSMsgBufferImported->
    VSMsgBufferImported::VSMsgBufferImported(cpm, chunkNumber);
  msgReceived(myVSMsgBufferImported);
  myVSMsgBufferImported->cleanup();

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

//
//
OZ_BI_define(BIVSremoveMailbox,1,0)
{
  //
  OZ_declareVirtualStringIN(0, mbKeyChars);
  Assert(sizeof(key_t) <= sizeof(int));
  key_t mbKey;

  //
  Assert(sizeof(key_t) == sizeof(int));
  if (sscanf(mbKeyChars, "%i", &mbKey) != 1)
    return oz_raise(E_ERROR,E_SYSTEM,"VSremoveMailbox: invalid arg",0);
  markDestroy(mbKey);

  //
  return (PROCEED);
} OZ_BI_end

///
/// An exit hook - reclaim shared memory;
///
void virtualSitesExit()
{
  //
  am.removeTask((void *) &vsSiteQueue, checkMessageQueue);

  //
  // Kill those virtual sites we know about that are the slaves
  // (i.e. whose mailboxes have been created here):
  VSMailboxManagerImported *mbm;
  mbm = mailboxRegister.getFirst();
  while (mbm) {
    if (slavesRegister.isInThere(mbm->getSHMKey())) {
      VSMailboxManagerImported *mbmDel = mbm;
      int pid = mbm->getMailbox()->getPid();
      if (pid)
        (void) oskill(pid, SIGTERM);
      mbm = mailboxRegister.getNext(mbm);
      mailboxRegister.unregister(mbmDel);
    } else {
      mbm = mailboxRegister.getNext(mbm);
    }
  }

  //
  if (myVSMailboxManager) {
    am.removeTask((void *) myVSMailboxManager->getMailbox(),
                  checkVSMessages);
    myVSMailboxManager->destroy();
    delete myVSMailboxManager;
    myVSMailboxManager = (VSMailboxManagerOwned *) 0;
  }

  //
  if (myVSChunksPoolManager) {
    delete myVSChunksPoolManager;
    myVSChunksPoolManager = (VSMsgChunkPoolManagerOwned *) 0;
  }

  //
  if (myVSMsgBufferImported) {
    free(myVSMsgBufferImported);
    myVSMsgBufferImported = (VSMsgBufferImported *) 0;
  }
}

#else  // VIRTUALSITES

//
VirtualSite* createVirtualSite(Site *)
{
  error("'createVirtualSite' called without 'VIRTUALSITES'?");
  return ((VirtualSite *) 0);
}

//
void marshalVirtualInfo(VirtualInfo *, MsgBuffer *)
{
  error("'marshalVirtualInfo' called without 'VIRTUALSITES'?");
}
//
VirtualInfo* unmarshalVirtualInfo(MsgBuffer *)
{
  error("'unmarshalVirtualInfo' called without 'VIRTUALSITES'?");
  return ((VirtualInfo *) 0);
}
//
void unmarshalUselessVirtualInfo(MsgBuffer *)
{
  error("'unmarshalUselessVirtualInfo' called without 'VIRTUALSITES'?");
}

//
void zeroRefsToVirtual(VirtualSite *)
{
  error("'zeroRefsToVirtual' called without 'VIRTUALSITES'?");
}

//
int sendTo_VirtualSite(VirtualSite*, MsgBuffer*, MessageType, Site*, int)
{
  error("'sendTo_VirtualSite' called without 'VIRTUALSITES'?");
  return (-1);
}

//
int discardUnsentMessage_VirtualSite(VirtualSite*, int)
{
  error("'discardUnsentMessage_VirtualSite' called without 'VIRTUALSITES'?");
  return (-1);
}

//
int getQueueStatus_VirtualSite(VirtualSite*, int &)
{
  error("'getQueueStatus_VirtualSite' called without 'VIRTUALSITES'?");
  return (-1);
}

//
SiteStatus siteStatus_VirtualSite(VirtualSite *)
{
  error("'siteStatus_VirtualSite' called without 'VIRTUALSITES'?");
  return ((SiteStatus) -1);
}

//
MonitorReturn monitorQueue_VirtualSite(VirtualSite*, int, int, void*)
{
  error("'monitorQueue_VirtualSite' called without 'VIRTUALSITES'?");
  return ((MonitorReturn) -1);
}

//
MonitorReturn demonitorQueue_VirtualSite(VirtualSite *)
{
  error("'demonitorQueue_VirtualSite' called without 'VIRTUALSITES'?");
  return ((MonitorReturn) -1);
}

//
ProbeReturn installProbe_VirtualSite(VirtualSite*, ProbeType, int, void*)
{
  error("'installProbe_VirtualSite' called without 'VIRTUALSITES'?");
  return ((ProbeReturn) -1);
}

//
ProbeReturn deinstallProbe_VirtualSite(VirtualSite*, ProbeType pt)
{
  error("'installProbe_VirtualSite' called without 'VIRTUALSITES'?");
  return ((ProbeReturn) -1);
}

//
ProbeReturn probeStatus_VirtualSite(VirtualSite*, ProbeType&, int&, void*&)
{
  error("'probeStatus_VirtualSite' called without 'VIRTUALSITES'?");
  return ((ProbeReturn) -1);
}

//
GiveUpReturn giveUp_VirtualSite(VirtualSite *)
{
  error("'giveUp_VirtualSite' called without 'VIRTUALSITES'?");
  return ((GiveUpReturn) -1);
}

//
void discoveryPerm_VirtualSite(VirtualSite *)
{
  error("'discoveryPerm_VirtualSite' called without 'VIRTUALSITES'?");
}

//
void dumpVirtualInfo(VirtualInfo *)
{
  error("'dumpVirtualInfo' called without 'VIRTUALSITES'?");
}

//
MsgBuffer* getVirtualMsgBuffer(Site *)
{
  error("'getVirtualMsgBuffer' called without 'VIRTUALSITES'?");
  return ((MsgBuffer *) 0);
}

//
void dumpVirtualMsgBuffer(MsgBuffer *)
{
  error("'dumpVirtualMsgBuffer' called without 'VIRTUALSITES'?");
}

//
// Builtins for virtual sites - only two of them are needed:
// (I) Creating a new mailbox (at the master site):
OZ_BI_define(BIVSnewMailbox,0,1)
{
  return oz_raise(E_ERROR, E_SYSTEM,
                  "VSnewMailbox: virtual sites not configured", 0);
} OZ_BI_end

//
// (II) Initializing a virtual site given its mailbox (which contains
// also the parent's id);
OZ_BI_define(BIVSinitServer,1,0)
{
  return oz_raise(E_ERROR, E_SYSTEM,
                  "VSinitServer: virtual sites not configured", 0);
} OZ_BI_end

//
//
OZ_BI_define(BIVSremoveMailbox,1,0)
{
  return oz_raise(E_ERROR, E_SYSTEM,
                  "VSremoveMailbox: virtual sites not configured", 0);
} OZ_BI_end

#endif // VIRTUALSITES
