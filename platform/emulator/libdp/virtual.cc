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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "base.hh"
#include "dpBase.hh"

#include "builtins.hh"

#ifdef VIRTUALSITES

#include "perdio.hh"
#include "msgbuffer.hh"
#include "vs_mailbox.hh"
#include "vs_msgbuffer.hh"
#include "vs_comm.hh"

//
// Here we have the actual virtual sites implementation
// ('*Impl()' methods);
//

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
// Virtual sites we know about;
static VSRegister vsRegister;

//
// The guy busy with probing...
static VSProbingObject vsProbingObject(VS_REGISTER_HT_SIZE, &vsRegister);

//
// Created mailboxes (their keys);
static VSMailboxKeysRegister slavesRegister;

//
// A queue of sites that have pending messages (still to be sent):
static VSSiteQueue vsSiteQueue;

//
// There are also three managers that are allocated explicitly:
static VSMailboxManagerOwned *myVSMailboxManager;
static VSMsgChunkPoolManagerOwned *myVSChunksPoolManager;
static VSMsgChunkPoolManagerImported *importedVSChunksPoolManager;

//
// By now we need four tasks (MAXTASKS?):
// (a) input from neighbour virtual sites,
// (b) pending (because of locks at the receiver site) sends,
// (c) probing of virtual sites.
// (d) GCing of (own) chunk pools;

//
//
// The 'check'&'process' procedures for processing incoming messages
// (see am.hh, class 'TaskNode'):
// static TaskCheckProc checkVSMessages;
static Bool checkVSMessages(unsigned long clock, void *mbox);
// ... and the read handler itself:
// static TaskProcessProc readVSMessages;
static Bool readVSMessages(unsigned long clock, void *mbox);

//
// ... and the pair for processing unsent messages:
// static TaskCheckProc checkMessageQueue;
static Bool checkMessageQueue(unsigned long clock, VSSiteQueue *sq);
// static TaskProcessProc processMessageQueue;
static Bool processMessageQueue(unsigned long clock, VSSiteQueue *sq);

//
// ... dealing with probes;
// static TaskCheckProc checkProbes;
static Bool checkProbes(unsigned long clock, VSProbingObject *po);
// static TaskProcessProc processProbes;
static Bool processProbes(unsigned long clock, VSProbingObject *po);

//
// ... GCing of chunk pools of outgoing messages;
// static TaskCheckProc checkGCMsgChunks;
static Bool checkGCMsgChunks(unsigned long clock,
                             VSMsgChunkPoolManagerOwned *cpm);
// static TaskProcessProc processGCMsgChunks;
static Bool processGCMsgChunks(unsigned long clock,
                               VSMsgChunkPoolManagerOwned *cpm);

//
DebugCode(VSSendRecvCounter vsSRCounter;);

///
/// (static) interface methods for virtual sites;
///

//
VirtualSite* createVirtualSiteImpl(DSite* s)
{
  VirtualSite *vs = new VirtualSite(s, &freeMessagePool, &vsSiteQueue);
  vsRegister.registerVS(vs);
  return (vs);
}

//
void zeroRefsToVirtualImpl(VirtualSite *vs)
{
  vs->zeroReferences();
}

//
// The 'mt', 'storeSite' and 'storeIndex' parameters are used whenever a
// communication layer reports a problem with a message which has
// been previously accepted for delivery;
int sendTo_VirtualSiteImpl(VirtualSite *vs, MsgBuffer *mb,
                           MessageType mt, DSite *storeSite, int storeIndex)
{
  DebugCode(vsSRCounter.send(););
  return (vs->sendTo((VSMsgBufferOwned *) mb, mt, storeSite, storeIndex,
                     &freeMsgBufferPool));
}

//
// By now, there can be no unsent messages - so,
// 'VirtualSite::discardUnsentMessage()' does nothing;
int discardUnsentMessage_VirtualSiteImpl(VirtualSite *vs, int msgNum)
{
  return (vs->discardUnsentMessage(msgNum));
}

//
// (There are queues, but they are transparent by now:
int getQueueStatus_VirtualSiteImpl(VirtualSite *vs, int &noMsgs)
{
  return (vs->getQueueStatus(noMsgs));
}

//
SiteStatus siteStatus_VirtualSiteImpl(VirtualSite* vs)
{
  return (vs->getSiteStatus());
}

//
// There are no monitors now;
MonitorReturn
monitorQueue_VirtualSiteImpl(VirtualSite *vs,
                             int size, int no_msgs, void *storePtr)
{
  OZ_warning("'monitorQueue_VirtualSite()' is not implemented!");
  return (MONITOR_OK);
}

//
MonitorReturn demonitorQueue_VirtualSiteImpl(VirtualSite* vs)
{
  return (MONITOR_OK);
}

//
ProbeReturn
installProbe_VirtualSiteImpl(VirtualSite *vs, ProbeType pt, int frequency)
{
  Assert(frequency > 0);
  return (vsProbingObject.installProbe(vs, pt, frequency));
}

//
ProbeReturn deinstallProbe_VirtualSiteImpl(VirtualSite *vs, ProbeType pt)
{
  return (vsProbingObject.deinstallProbe(vs, pt));
}

//
ProbeReturn
probeStatus_VirtualSiteImpl(VirtualSite *vs,
                            ProbeType &pt, int &frequncey, void* &storePtr)
{
  return (PROBE_NONEXISTENT);
}

//
// A virtual site should not be given up now since there are no
// temporary problems for virtual sites;
GiveUpReturn giveUp_VirtualSiteImpl(VirtualSite* vs)
{
  OZ_error("Virtual Site is given up!??");
  return (SITE_NOW_NORMAL);
}

//
// 'discoveryPerm' means that a site is known (e.g. by polling or from
// a third party) to be dead.
void discoveryPerm_VirtualSiteImpl(VirtualSite *vs)
{
  vsRegister.retractVS(vs);
  myVSChunksPoolManager->retractRegisteredSite(vs);
  vs->drop();
  delete vs;
}

//
// This method gives us a virtual sites message buffer without
// header;
static inline
VSMsgBufferOwned* getBasicVirtualMsgBufferImpl(DSite* site)
{
  VSMsgBufferOwned *voidBUF = freeMsgBufferPool.allocate();
  VSMsgBufferOwned *buf =
    new (voidBUF) VSMsgBufferOwned(myVSChunksPoolManager, site);
  return (buf);
}

//
static VSMsgHeaderOwned vsStdMsgHeader(VS_M_PERDIO);

//
// The non-interface method: a virtual message buffer that is not
// supposed for the actual transmitting (aka for 'BImarshalerPerf');
MsgBuffer* getCoreVirtualMsgBuffer(DSite* site)
{
  VSMsgBufferOwned *buf = getBasicVirtualMsgBufferImpl(site);
  return (buf);                 // upcast (actually, even 2 steps);
}

//
// The interface method: a message buffer for perdio messages;
MsgBuffer* getVirtualMsgBufferImpl(DSite* site)
{
  VSMsgBufferOwned *buf = getBasicVirtualMsgBufferImpl(site);
  vsStdMsgHeader.marshal(buf);

  //
  return (buf);                 // upcast (actually, even 2 steps);
}

//
// Virtual sites methods;
//
// Internal message types - those that are not handled by (passed to)
// the perdio layer but processed internally;
static VSMsgHeaderOwned vsInitMsgHeader(VS_M_INIT_VS);
static VSMsgHeaderOwned vsSiteIsAliveMsgHeader(VS_M_SITE_IS_ALIVE);
static VSMsgHeaderOwned vsSiteAliveMsgHeader(VS_M_SITE_ALIVE);
static VSMsgHeaderOwned vsUnusedShmIdMsgHeader(VS_M_UNUSED_SHMID);

//
VSMsgBufferOwned* composeVSInitMsg()
{
  VSMsgBufferOwned *buf = getBasicVirtualMsgBufferImpl((DSite *) 0);
  vsInitMsgHeader.marshal(buf);
  myDSite->marshalDSite(buf);
  return (buf);
}

//
VSMsgBufferOwned* composeVSSiteIsAliveMsg(DSite *s)
{
  VSMsgBufferOwned *buf = getBasicVirtualMsgBufferImpl(s);
  vsSiteIsAliveMsgHeader.marshal(buf);
  myDSite->marshalDSite(buf);
  return (buf);
}

//
VSMsgBufferOwned* composeVSSiteAliveMsg(DSite *s, VirtualSite *vs)
{
  VSMsgBufferOwned *buf = getBasicVirtualMsgBufferImpl(s);
  vsSiteAliveMsgHeader.marshal(buf);
  myDSite->marshalDSite(buf);
  // 'alive ack' message contains also a list of ipc keys - of
  // shared memory segments of the chunk pool;
  vs->marshalLocalResources(buf, myVSMailboxManager, myVSChunksPoolManager);
  return (buf);
}

//
VSMsgBufferOwned* composeVSUnusedShmIdMsg(DSite *s, key_t shmid)
{
  Assert(sizeof(key_t) <= sizeof(unsigned int));
  VSMsgBufferOwned *buf = getBasicVirtualMsgBufferImpl(s);
  vsUnusedShmIdMsgHeader.marshal(buf);
  myDSite->marshalDSite(buf);
  marshalNumber((unsigned int) shmid, buf);
  return (buf);
}

//
// Unmarshaling the "vs" header;
VSMsgType getVSMsgType(VSMsgBufferImported *mb)
{
  VSMsgHeaderImported vsMsgHeader(mb);
  return (vsMsgHeader.getMsgType());
}

//
void decomposeVSInitMsg(VSMsgBuffer *mb, DSite* &s)
{
  s = unmarshalDSite(mb);
}

//
void decomposeVSSiteIsAliveMsg(VSMsgBuffer *mb, DSite* &s)
{
  s = unmarshalDSite(mb);
}

//
void decomposeVSSiteAliveMsg(VSMsgBuffer *mb, DSite* &s, VirtualSite* &vs)
{
  s = unmarshalDSite(mb);
  Assert(s->virtualComm());
  vs = s->getVirtualSite();
  vs->unmarshalResources(mb);
}

//
void decomposeVSUnusedShmIdMsg(VSMsgBuffer *mb, DSite* &s, key_t &shmid)
{
  Assert(sizeof(key_t) <= sizeof(unsigned int));
  s = unmarshalDSite(mb);
  shmid = (key_t) unmarshalNumber(mb);
}


//
//
static Bool checkVSMessages(unsigned long clock, void *vMBox)
{
  // unsafe by now - some magic number should be added;
  VSMailboxOwned *mbox = (VSMailboxOwned *) vMBox;
  return ((Bool) mbox->getSize());
}

//
static Bool readVSMessages(unsigned long clock, void *vMBox)
{
  // unsafe by now - some magic number(s) should be added;
  VSMailboxOwned *mbox = (VSMailboxOwned *) vMBox;
  int msgs = MAX_MSGS_ATONCE;

  //
  //
  while (mbox->isNotEmpty() && msgs) {
    key_t msgChunkPoolKey;
    int chunkNumber;
    VSMsgType msgType;

    //
    msgs--;
    if (mbox->dequeue(msgChunkPoolKey, chunkNumber)) {
      // got a message;
      //
      myVSMsgBufferImported = new (myVSMsgBufferImported)
        VSMsgBufferImported(importedVSChunksPoolManager,
                            msgChunkPoolKey, chunkNumber);
      //
      myVSMsgBufferImported->unmarshalBegin();

      //
      if (myVSMsgBufferImported->isVoid()) {
        myVSMsgBufferImported->dropVoid();
        myVSMsgBufferImported->cleanup();
        // next message (if any could be processed??)
        continue;
      }
      //
      msgType = getVSMsgType(myVSMsgBufferImported);

      //
      if (msgType == VS_M_PERDIO) {
        DebugCode(vsSRCounter.recv(););
        msgReceived(myVSMsgBufferImported);
      } else {
        //
        switch (msgType) {
        case VS_M_INVALID:
          OZ_error("readVSMessages: M_INVALID message???");
          break;

        case VS_M_INIT_VS:
          OZ_error("readVSMessages: VS_M_INIT_VS is not expected here.");
          break;

        case VS_M_SITE_IS_ALIVE:
          {
            DSite *s;
            VirtualSite *vs;

            //
            decomposeVSSiteIsAliveMsg(myVSMsgBufferImported, s);
            vs = s->getVirtualSite();
            Assert(vs);

            //
            VSMsgBufferOwned *bs = composeVSSiteAliveMsg(s, vs);
            if (sendTo_VirtualSite(vs, bs, /* messageType */ M_NONE,
                                   /* storeSite */ (DSite *) 0,
                                   /* storeIndex */ 0) != ACCEPTED)
              OZ_error("readVSMessages: unable to send 'site_alive' message?");
            break;
          }

        case VS_M_SITE_ALIVE:
          {
            DSite *s;
            VirtualSite *vs;
            decomposeVSSiteAliveMsg(myVSMsgBufferImported, s, vs);
            s->siteAlive();
            break;
          }

        case VS_M_UNUSED_SHMID:
          {
            DSite *s;
            key_t shmid;
            decomposeVSUnusedShmIdMsg(myVSMsgBufferImported, s, shmid);
            importedVSChunksPoolManager->removeSegmentManager(shmid);
            break;
          }

        default:
          OZ_error("readVSMessages: unknown 'vs' message type!");
          break;
        }
      }

      //
      myVSMsgBufferImported->unmarshalEnd();
      myVSMsgBufferImported->releaseChunks();
      myVSMsgBufferImported->cleanup();
    } else {
      // is locked - then let's try to read later;
      return (FALSE);
    }
  }

  //
  return (TRUE);
}

//
//
static Bool checkMessageQueue(unsigned long clock, void *sqi)
{
  // unsafe by now - some magic number should be added;
  VSSiteQueue *sq = (VSSiteQueue *) sqi;
  return (sq->isNotEmpty());
}

//
static Bool processMessageQueue(unsigned long clock, void *sqi)
{
  // unsafe by now - some magic number should be added;
  VSSiteQueue *sq = (VSSiteQueue *) sqi;
  Bool ready = TRUE;
  VirtualSite *vs = sq->getFirst();

  //
  Assert(vs);
  while (vs) {
    Bool siteReady = TRUE;
    VSMessage *vsm = vs->getFirst();

    //
    Assert(vsm);
    while (vsm) {
      // spin up to the first message that cannot be delivered;
      if (vs->tryToSendToAgain(vsm, &freeMsgBufferPool)) {
        vsm = vs->getNext();
      } else {
        siteReady = FALSE;
        break;
      }
    }

    //
    if (siteReady)
      vs->retractVSSiteQueueNode();

    //
    ready = ready && siteReady;
    //
    vs = sq->getNext();
  }

  //
  return (ready);
}

//
static Bool checkProbes(unsigned long clock, void *poi)
{
  VSProbingObject *po = (VSProbingObject *) poi;
  return (po->checkProbes(clock));
}

//
static Bool processProbes(unsigned long clock, void *poi)
{
  VSProbingObject *po = (VSProbingObject *) poi;
  return (po->processProbes(clock, importedVSChunksPoolManager));
}

//
static Bool checkGCMsgChunks(unsigned long clock, void *poi)

{
  VSMsgChunkPoolManagerOwned *cpm = (VSMsgChunkPoolManagerOwned*) poi;
  return ((clock > cpm->getLastGC() + VS_SEGS_MAXPHASE_MS) ?
          TRUE : FALSE);
}

//
static Bool processGCMsgChunks(unsigned long clock, void *poi)
{
  VSMsgChunkPoolManagerOwned *cpm = (VSMsgChunkPoolManagerOwned*) poi;
  cpm->scavenge();
  return (TRUE);
}

//
void dumpVirtualMsgBufferImpl(MsgBuffer* m)
{
  VSMsgBufferOwned *buf = (VSMsgBufferOwned *) m;

  buf->releaseChunks();
  buf->cleanup();
  freeMsgBufferPool.dispose(buf);
}

//
void siteAlive_VirtualSiteImpl(VirtualSite *vs)
{
  vs->setTimeAliveAck(am.getEmulatorClock());
}

///
/// An exit hook - kill slaves, reclaim shared memory, etc...
///
void virtualSitesExitImpl()
{
  //
  // fprintf(stdout, "cleaning up VSs...\n"); fflush(stdout);
  am.removeTask(&vsSiteQueue, checkMessageQueue);
  am.removeTask(&vsProbingObject, checkProbes);

  //
  if (myVSMailboxManager) {
    am.removeTask(myVSMailboxManager->getMailbox(), checkVSMessages);
    myVSMailboxManager->destroy();
    delete myVSMailboxManager;
    myVSMailboxManager = (VSMailboxManagerOwned *) 0;
  }

  //
  if (myVSChunksPoolManager) {
    //    am.removeTask(myVSChunksPoolManager, checkGCMsgChunks);
    delete myVSChunksPoolManager;
    myVSChunksPoolManager = (VSMsgChunkPoolManagerOwned *) 0;
  }

  //
  if (importedVSChunksPoolManager) {
    delete importedVSChunksPoolManager;
    importedVSChunksPoolManager = (VSMsgChunkPoolManagerImported *) 0;
  }

  //
  if (myVSMsgBufferImported) {
    free(myVSMsgBufferImported);
    myVSMsgBufferImported = (VSMsgBufferImported *) 0;
  }

  //
  // Kill those virtual sites registered locally that are slaves
  // (i.e. whose mailboxes have been created here):
  VirtualSite *vs = vsRegister.getFirst();
  while (vs) {
    if (vs->isSlave()) {
      int pid = vs->getVSPid();
      if (pid)
        (void) oskill(pid, SIGTERM);
    }
    vs = vsRegister.getNext();
  }

  //
  // Try to kill all the mailboxes created locally (mailboxes of
  // slaves);
  key_t key;
  while ((key = slavesRegister.retrieve()))
    markDestroy(key);
}


//
// Builtins for virtual sites - only two of them are needed:
// (I) Creating a new mailbox (at the master site):
OZ_BI_define(BIVSnewMailbox,0,1)
{
  //
  // Zeroth, init perdio if it isn't;
  initDP();

  //
  // First, link the interface:
  createVirtualSite = createVirtualSiteImpl;
  zeroRefsToVirtual = zeroRefsToVirtualImpl;
  sendTo_VirtualSite = sendTo_VirtualSiteImpl;
  discardUnsentMessage_VirtualSite = discardUnsentMessage_VirtualSiteImpl;
  getQueueStatus_VirtualSite = getQueueStatus_VirtualSiteImpl;
  siteStatus_VirtualSite = siteStatus_VirtualSiteImpl;
  monitorQueue_VirtualSite = monitorQueue_VirtualSiteImpl;
  demonitorQueue_VirtualSite = demonitorQueue_VirtualSiteImpl;
  installProbe_VirtualSite = installProbe_VirtualSiteImpl;
  deinstallProbe_VirtualSite = deinstallProbe_VirtualSiteImpl;
  probeStatus_VirtualSite = probeStatus_VirtualSiteImpl;
  giveUp_VirtualSite = giveUp_VirtualSiteImpl;
  discoveryPerm_VirtualSite = discoveryPerm_VirtualSiteImpl;
  getVirtualMsgBuffer = getVirtualMsgBufferImpl;
  dumpVirtualMsgBuffer = dumpVirtualMsgBufferImpl;
  siteAlive_VirtualSite = siteAlive_VirtualSiteImpl;
  virtualSitesExit = virtualSitesExitImpl;

  //
  VSMailboxManagerCreated *mbm;
  VSMsgBufferOwned *buf;
  char keyChars[sizeof(key_t)*2 + 3]; // in the form "0xNNNNNNNN";

  //
  // Check whether this site is (already) a virtual one:
  if (!myDSite->hasVirtualInfo()) {
    VirtualInfo *vi;
    key_t myMBoxKey;

    //
    mbm = new VSMailboxManagerCreated(VS_MAILBOX_SIZE);
    myMBoxKey = mbm->getSHMKey();
    mbm->unmap();
    delete mbm;
    DebugCode(mbm = (VSMailboxManagerCreated *) 0);

    myVSMailboxManager = new VSMailboxManagerOwned(myMBoxKey);
    myVSChunksPoolManager =
      new VSMsgChunkPoolManagerOwned(VS_CHUNK_SIZE, VS_CHUNKS_NUM,
                                     VS_REGISTER_HT_SIZE);
    importedVSChunksPoolManager =
      new VSMsgChunkPoolManagerImported(VS_REGISTER_HT_SIZE);
    myVSMsgBufferImported =
      (VSMsgBufferImported *) malloc(sizeof(VSMsgBufferImported));

    //
    vi = new VirtualInfo(myDSite, myVSMailboxManager->getSHMKey());
    myDSite->makeMySiteVirtual(vi);

    //
    // Install the 'read-from-virtual-sites' & 'delayed-write' handlers;
    if (!am.registerTask((void *) myVSMailboxManager->getMailbox(),
                         checkVSMessages, readVSMessages))
      OZ_error("virtual sites: unable to register a task");
    if (!am.registerTask((void *) &vsSiteQueue,
                         checkMessageQueue, processMessageQueue))
      OZ_error("virtual sites: unable to register a task");
    if (!am.registerTask((void *) &vsProbingObject,
                         checkProbes, processProbes))
      OZ_error("virtual sites: unable to register a task");
    if (!am.registerTask((void *) myVSChunksPoolManager,
                         checkGCMsgChunks, processGCMsgChunks))
      OZ_error("virtual sites: unable to register a task");
  }

  //
  // The 'VSMailboxManager' always creates a proper object (or crashes
  // the system if it can't);
  mbm = new VSMailboxManagerCreated(VS_MAILBOX_SIZE);
  slavesRegister.add(mbm->getSHMKey());

  //
  // Put the 'VS_M_INIT_VS' message into it;
  buf = composeVSInitMsg();

  //
  if (!mbm->getMailbox()->enqueue(buf->getFirstChunkSHMKey(),
                                  buf->getFirstChunkNum()))
    OZ_error("Virtual sites: unable to put the M_INIT_VS message");
  buf->passChunks();
  buf->cleanup();
  freeMsgBufferPool.dispose(buf);
  DebugCode(buf = (VSMsgBufferOwned *) 0);

  //
  mbm->unmap();                 // we don't need that object now anymore;
  Assert(sizeof(key_t) <= sizeof(int));
  sprintf(keyChars, "0x%x", (int)mbm->getSHMKey());
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
  key_t msgChunkPoolKey;
  int chunkNumber;
  OZ_declareVirtualString(0, mbKeyChars);
  Assert(sizeof(key_t) <= sizeof(int));
  key_t mbKey;
  VSMailboxOwned *mbox;
  VSMsgType msgType;
  VirtualInfo *vi;
  DSite *ms;

  //
  initDP();

  //
  // First, link the interface:
  createVirtualSite = createVirtualSiteImpl;
  zeroRefsToVirtual = zeroRefsToVirtualImpl;
  sendTo_VirtualSite = sendTo_VirtualSiteImpl;
  discardUnsentMessage_VirtualSite = discardUnsentMessage_VirtualSiteImpl;
  getQueueStatus_VirtualSite = getQueueStatus_VirtualSiteImpl;
  siteStatus_VirtualSite = siteStatus_VirtualSiteImpl;
  monitorQueue_VirtualSite = monitorQueue_VirtualSiteImpl;
  demonitorQueue_VirtualSite = demonitorQueue_VirtualSiteImpl;
  installProbe_VirtualSite = installProbe_VirtualSiteImpl;
  deinstallProbe_VirtualSite = deinstallProbe_VirtualSiteImpl;
  probeStatus_VirtualSite = probeStatus_VirtualSiteImpl;
  giveUp_VirtualSite = giveUp_VirtualSiteImpl;
  discoveryPerm_VirtualSite = discoveryPerm_VirtualSiteImpl;
  getVirtualMsgBuffer = getVirtualMsgBufferImpl;
  dumpVirtualMsgBuffer = dumpVirtualMsgBufferImpl;
  siteAlive_VirtualSite = siteAlive_VirtualSiteImpl;
  virtualSitesExit = virtualSitesExitImpl;

  //
#ifdef DEBUG_CHECK
//   fprintf(stdout, "*** Sleeping for 10 secs - hook it up (pid %d)!\n",
//        getpid());
//   fflush(stdout);
//   sleep(10);                 // IMHO more than enough;
#endif

  //
  Assert(sizeof(key_t) == sizeof(int));
  if (sscanf(mbKeyChars, "%i", (int*)&mbKey) != 1)
    return oz_raise(E_ERROR,E_SYSTEM,"VSinitServer: invalid arg",0);

  //
  // new process group - otherwise failed virtual sites will kill
  // the whole virtual site group ;-)
  if (setpgid(0, 0))
    OZ_error("failed to form a new process group");

  //
  if (myVSMailboxManager)
    return oz_raise(E_ERROR,E_SYSTEM,"VSinitServer again",0);

  //
  // We know here the id of the mailbox, so let's fetch it:
  myVSMailboxManager = new VSMailboxManagerOwned(mbKey);
  myVSChunksPoolManager =
    new VSMsgChunkPoolManagerOwned(VS_CHUNK_SIZE, VS_CHUNKS_NUM,
                                   VS_REGISTER_HT_SIZE);
  importedVSChunksPoolManager =
    new VSMsgChunkPoolManagerImported(VS_REGISTER_HT_SIZE);

  //
  // Now, let's process the initialization message (M_INIT_VS).
  // Processing of it would result in making 'myDSite' a virtual one,
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
  myVSMsgBufferImported = new (myVSMsgBufferImported)
    VSMsgBufferImported(importedVSChunksPoolManager,
                        msgChunkPoolKey, chunkNumber);
  // (we must read-in the type field);
  myVSMsgBufferImported->unmarshalBegin();

  //
  // Check if something went wrong just from scratch:
  if (myVSMsgBufferImported->isVoid()) {
    myVSMsgBufferImported->dropVoid();
    myVSMsgBufferImported->cleanup();
    // "exit hook" should clean up everthing...
    return oz_raise(E_ERROR,E_SYSTEM,"noInitMessage",1,
                    oz_atom(mbKeyChars));
  }
  //
  msgType = getVSMsgType(myVSMsgBufferImported);
  Assert(msgType == VS_M_INIT_VS);

  //
  // The father's virtual site is registered during
  // unmarshaling, but it is NOT recognized as a virtual one
  // (since 'myDSite' has not been yet initialized - a
  // bootstrapping problem! :-))
  decomposeVSInitMsg(myVSMsgBufferImported, ms);
  myVSMsgBufferImported->unmarshalEnd();

  //
  Assert(!myDSite->hasVirtualInfo());
  // The 'myDSite' and 'ms' share the same master (which
  // might be 'ms' itself), so virtual infos differ in the
  // mailbox key only, which is to be set later:
  vi = new VirtualInfo(ms->getVirtualInfo(), mbKey);
  myDSite->makeMySiteVirtual(vi);
  Assert(myDSite->getVirtualInfo());

  //
  // Change the type of 'ms': this is a virtual site (per
  // definition);
  ms->makeActiveVirtual();

  //
  myVSMsgBufferImported->releaseChunks();
  myVSMsgBufferImported->cleanup();

  //
  // Install the 'read-from-virtual-sites' & 'delayed-write' handlers;
  if (!am.registerTask((void *) myVSMailboxManager->getMailbox(),
                       checkVSMessages, readVSMessages))
      OZ_error("virtual sites: unable to register a task");
  if (!am.registerTask((void *) &vsSiteQueue,
                       checkMessageQueue, processMessageQueue))
      OZ_error("virtual sites: unable to register a task");
  if (!am.registerTask((void *) &vsProbingObject,
                       checkProbes, processProbes))
      OZ_error("virtual sites: unable to register a task");
  if (!am.registerTask((void *) myVSChunksPoolManager,
                       checkGCMsgChunks, processGCMsgChunks))
      OZ_error("virtual sites: unable to register a task");

  //
  return (PROCEED);
} OZ_BI_end

//
//
OZ_BI_define(BIVSremoveMailbox,1,0)
{
  //
  initDP();

  //
  OZ_declareVirtualString(0, mbKeyChars);
  Assert(sizeof(key_t) <= sizeof(int));
  key_t mbKey;

  //
  Assert(sizeof(key_t) == sizeof(int));
  if (sscanf(mbKeyChars, "%i", (int*)&mbKey) != 1)
    return oz_raise(E_ERROR,E_SYSTEM,"VSremoveMailbox: invalid arg",0);
  markDestroy(mbKey);

  //
  return (PROCEED);
} OZ_BI_end

#else // VIRTUALSITES

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


/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modVirtualSite-if.cc"

#endif
