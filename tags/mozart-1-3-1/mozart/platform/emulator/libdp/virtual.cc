/*
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 *    Per Brand <perbrand@sics.se>
 * 
 *  Contributors:
 *
 *  Copyright:
 *    1997-1998 Konstantin Popov
 *    1997 Per Brand
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

#include "base.hh"
#include "dpBase.hh"

#include "builtins.hh"

#ifdef VIRTUALSITES

#include "perdio.hh"
#include "mbuffer.hh"
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
static FreeListDataManager<VSMarshalerBufferOwned> freeMarshalerBufferPool(malloc);
static VSMarshalerBufferImported *myVSMarshalerBufferImported;

//
// Free bodies of "messages" (which are used for keeping unsent
// messages). Note that there is its own implementation!
static VSFreeMessagePool freeMessagePool;

//
// Virtual sites we (thus, virtual sites comm layer) know about;
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
// Resource manager for virtual sites comm layer.
// This guy keeps track of failed/satisfied create/map shm page
// requests; this information is used
static
VSResourceManager vsResourceManager;

//
// 'vsTable' maps indexes to virtual sites. This mapping gives a
// receiver site the message's source site.
static VSTable vsTable(VS_VSTABLE_SIZE);

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
DebugVSMsgs(VSSendRecvCounter vsSRCounter;);

///
/// (static) interface methods for virtual sites;
///

//
VirtualSite* createVirtualSiteImpl(DSite* s)
{
  VirtualSite *vs = 
    new VirtualSite(s, &freeMessagePool, &vsSiteQueue,
		    &vsResourceManager, importedVSChunksPoolManager);
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
int sendTo_VirtualSiteImpl(VirtualSite *vs, MarshalerBuffer *mb, 
			   MessageType mt, DSite *storeSite, int storeIndex)
{
  DebugVSMsgs(vsSRCounter.send(););
  return (vs->sendTo((VSMarshalerBufferOwned *) mb, mt, storeSite, storeIndex,
		     &freeMarshalerBufferPool));
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
// ProbeReturn
// installProbe_VirtualSiteImpl(VirtualSite *vs, ProbeType pt, int frequency)
// {
//   Assert(frequency > 0);
//   return (vsProbingObject.installProbe(vs, pt, frequency));
// }

//
// ProbeReturn deinstallProbe_VirtualSiteImpl(VirtualSite *vs, ProbeType pt)
// {
//   return (vsProbingObject.deinstallProbe(vs, pt));
// }

//
// ProbeReturn
// probeStatus_VirtualSiteImpl(VirtualSite *vs,
// 			    ProbeType &pt, int &frequncey, void* &storePtr)
// {
//   return (PROBE_NONEXISTENT);
// }

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
  vsTable.drop(vs);
  delete vs;
}

//
// This method gives us a virtual sites message buffer without
// the header;
static inline
VSMarshalerBufferOwned* getBasicVirtualMarshalerBufferImpl(DSite* site)
{
  VSMarshalerBufferOwned *voidBUF = freeMarshalerBufferPool.allocate();
  VSMarshalerBufferOwned *buf = 
    new (voidBUF) VSMarshalerBufferOwned(myVSChunksPoolManager, site);
  return (buf);
}

//
// The non-interface method: a virtual message buffer that is not
// supposed for the actual transmitting (aka for 'BImarshalerPerf');
MarshalerBuffer* getCoreVirtualMarshalerBuffer(DSite* site)
{
  VSMarshalerBufferOwned *buf = getBasicVirtualMarshalerBufferImpl(site);
  return (buf);			// upcast (actually, even 2 steps);
}

//
// The header is decomposed by the 'readVSMessges';
static inline
void putVSMsgHeader(VSMarshalerBufferOwned *buf, VSMsgType type, DSite *dest)
{
  Assert(sizeof(BYTE) > ((int) VS_M_LAST)/256);
  buf->put(type);

  //
  // Normally we know our 'vsTable' index at the remote site, but as
  // long as no corresponding 'VS_M_YOUR_INDEX_HERE' is not received the
  // 'myDSite' is sent (see also comments for 'VirtualSite::vsIndex');
  VirtualSite *vs = dest->getVirtualSite();
  int vsIndex = vs->getVSIndex();
  if (vsIndex >= 0) {
    marshalNumber(buf, (unsigned int) vsIndex);	// why bother with shorts?
  } else {
    // '-1' takes some place but the whole thing happens at least seldom;
    marshalNumber(buf, (unsigned int) -1);
    myDSite->marshalDSite(buf);
  }
}

//
// The interface method: a message buffer for perdio messages;
MarshalerBuffer* getVirtualMarshalerBufferImpl(DSite* dest)
{
  VSMarshalerBufferOwned *buf = getBasicVirtualMarshalerBufferImpl(dest);
  putVSMsgHeader(buf, VS_M_PERDIO, dest);
  return (buf);			// upcast (actually, even 2 steps);
}

//
// Virtual sites methods;
//

//
// 'init' message does not contain the header (it does not need to nor
// it can since the destination site is unknown (an may be even is not
// existing yet));
VSMarshalerBufferOwned* composeVSInitMsg()
{
  VSMarshalerBufferOwned *buf = getBasicVirtualMarshalerBufferImpl((DSite *) 0);
  //
  Assert(sizeof(BYTE) > ((int) VS_M_LAST)/256);
  buf->put(VS_M_INIT_VS);
  //
  myDSite->marshalDSite(buf);
  return (buf);
}

//
// It contains the destination site, so that site can compare itself
// with the marshaled site and report an error if they mismatch;
VSMarshalerBufferOwned* composeVSSiteIsAliveMsg(DSite *dest)
{
  VSMarshalerBufferOwned *buf = getBasicVirtualMarshalerBufferImpl(dest);
  //
  putVSMsgHeader(buf, VS_M_SITE_IS_ALIVE, dest);
  dest->marshalDSite(buf);
  //
  return (buf);
}

//
VSMarshalerBufferOwned* composeVSSiteAliveMsg(DSite *dest, VirtualSite *vs)
{
  VSMarshalerBufferOwned *buf = getBasicVirtualMarshalerBufferImpl(dest);
  //
  putVSMsgHeader(buf, VS_M_SITE_ALIVE, dest);
  //
  myDSite->marshalDSite(buf);
  // 'alive ack' message contains also a list of ipc keys - of 
  // shared memory segments of the chunk pool;
  vs->marshalLocalResources(buf, myVSMailboxManager, myVSChunksPoolManager);
  return (buf);
}

//
VSMarshalerBufferOwned* composeVSSiteDeadMsg(DSite *dest, DSite *ds)
{
  VSMarshalerBufferOwned *buf = getBasicVirtualMarshalerBufferImpl(dest);
  //
  putVSMsgHeader(buf, VS_M_SITE_DEAD, dest);
  ds->marshalDSite(buf);
  //
  return (buf);
}

//
VSMarshalerBufferOwned* composeVSYourIndexHereMsg(DSite *dest, int index)
{
  VSMarshalerBufferOwned *buf = getBasicVirtualMarshalerBufferImpl(dest);
  //
  putVSMsgHeader(buf, VS_M_YOUR_INDEX_HERE, dest);
  //
  marshalNumber(buf, (unsigned int) index);
  return (buf);
}

//
VSMarshalerBufferOwned* composeVSUnusedShmIdMsg(DSite *dest, key_t shmid)
{
  Assert(sizeof(key_t) <= sizeof(unsigned int));
  VSMarshalerBufferOwned *buf = getBasicVirtualMarshalerBufferImpl(dest);
  //
  putVSMsgHeader(buf, VS_M_UNUSED_SHMID, dest);
  //
  myDSite->marshalDSite(buf);
  marshalNumber(buf, (unsigned int) shmid);
  return (buf);
}

//
VSMsgType getVSMsgType(VSMarshalerBufferImported *mb)
{
  Assert(sizeof(BYTE) > ((int) VS_M_LAST)/256);
  return ((VSMsgType) mb->get());
}

//
void decomposeVSInitMsg(VSMarshalerBuffer *mb, DSite* &s)
{
  s = unmarshalDSite(mb);
}

//
void decomposeVSSiteIsAliveMsg(VSMarshalerBuffer *mb, DSite* &src)
{
  src = unmarshalDSite(mb);
}

//
void decomposeVSSiteAliveMsg(VSMarshalerBuffer *mb, DSite* &s, VirtualSite* &vs)
{
  s = unmarshalDSite(mb);
  Assert(s->virtualComm());
  vs = s->getVirtualSite();
  vs->unmarshalResources(mb);
}

//
// 'dvs' may be zero - when the site has been recognized locally as
// dead and GC'ed after that;
void decomposeVSSiteDeadMsg(VSMarshalerBuffer *mb, DSite* &ds,
			    VirtualSite* &dvs)
{
  // Note that the 's' is not marked in the stream as 'PERM',
  // so we can get here both 'PERM' and alive sites;
  ds = unmarshalDSite(mb);
  Assert(ds->virtualComm());
  //
  if (ds->isPerm()) {
    dvs = (VirtualSite *) 0;
  } else if (ds->isConnected()) {
    dvs = ds->getVirtualSite();
    // must be logically connected in this case:
    Assert(dvs);
    // however, it may be physically disconnected, of course;
  } else {
    // both logically and physically disconnected - we won't connect,
    // so virtual site won't contain any useful "resources" info;
    dvs = (VirtualSite *) 0;
  }
}

void decomposeVSYourIndexHereMsg(VSMarshalerBuffer *mb, int &index)
{
  index = (int) unmarshalNumber(mb);
}

//
void decomposeVSUnusedShmIdMsg(VSMarshalerBuffer *mb, DSite* &s, key_t &shmid)
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

    //
    msgs--;
    if (mbox->dequeue(msgChunkPoolKey, chunkNumber)) {
      // got a message;
      VSMsgType msgType;
      // sender's index, virtual and DSite;
      int vsIndex;
      VirtualSite *sVS;
      DSite *sS;

      //
      myVSMarshalerBufferImported = new (myVSMarshalerBufferImported)
	VSMarshalerBufferImported(importedVSChunksPoolManager,
			    msgChunkPoolKey, chunkNumber);
      //
      myVSMarshalerBufferImported->unmarshalBegin();

      //
      if (myVSMarshalerBufferImported->isVoid()) {
	// We cannot do much here, since it's not even known where the
	// message came from. However, we *must* guarantee that this
	// message loss is NOT due to resource problems; otherwise the
	// PERDIO layer will be confused;
	myVSMarshalerBufferImported->dropVoid();
	myVSMarshalerBufferImported->cleanup();
	// next message (if any could be processed??)
	continue;
      }
      //
      msgType = getVSMsgType(myVSMarshalerBufferImported);

      //
      // Take the site info out of the stream:
      vsIndex = (int) unmarshalNumber(myVSMarshalerBufferImported);
      if (vsIndex >= 0) {
	sVS = vsTable[vsIndex];
	sS = sVS->getSite();
	//
	myVSMarshalerBufferImported->setSite(sS);
	myVSMarshalerBufferImported->setKeysRegister(sVS->getKeysRegister());
	vsResourceManager.startMsgReceived(sVS);
      } else {
	sS = unmarshalDSite(myVSMarshalerBufferImported);
	sVS = sS->getVirtualSite();
	//
	// Exactly in this order: first, complete the initializing the
	// message buffer, notify the resource manager that we are
	// unmarshalling a message, and then - compose the 'your index
	// here' message:
	myVSMarshalerBufferImported->setSite(sS);
	myVSMarshalerBufferImported->setKeysRegister(sVS->getKeysRegister());
	vsResourceManager.startMsgReceived(sVS);

	//
	// Now, assign the index and send it out:
	int vsIndex = vsTable.put(sVS);
	//
	VSMarshalerBufferOwned *bs = composeVSYourIndexHereMsg(sS, vsIndex);
	if (sendTo_VirtualSiteImpl(sVS, bs, /* messageType */ M_NONE,
				   /* storeSite */ (DSite *) 0,
				   /* storeIndex */ 0) != ACCEPTED)
	  OZ_error("readVSMessages: unable to send 'your index here' msg?");
      }

      //
      switch (msgType) {

      case VS_M_PERDIO:
	//
	DebugVSMsgs(vsSRCounter.recv(););
	fflush(stdout);
	msgReceived(myVSMarshalerBufferImported);
	fflush(stdout);
	break;

      case VS_M_INVALID:
	OZ_error("readVSMessages: M_INVALID message???");
	break;

      case VS_M_INIT_VS:
	OZ_error("readVSMessages: VS_M_INIT_VS is not expected here.");
	break;

      case VS_M_SITE_IS_ALIVE:
	{
	  DSite *myS;
	  //
	  // 'myS' is supposed to be 'myDSite' - otherwise it is dead;
	  decomposeVSSiteIsAliveMsg(myVSMarshalerBufferImported, myS);

	  //
	  if (myS == myDSite) {
	    VSMarshalerBufferOwned *bs = composeVSSiteAliveMsg(sS, sVS);
	    if (sendTo_VirtualSiteImpl(sVS, bs, /* messageType */ M_NONE,
				       /* storeSite */ (DSite *) 0,
				       /* storeIndex */ 0) != ACCEPTED)
	      OZ_error("readVSMessages: unable to send 'site alive' msg?");

	    //
	  } else {
	    // The site 'myS' is dead: there cann't be two distinct
	    // processes with the same pid. Pid"s are the same because
	    // of the mailboxes naming scheme - two keys can be equal
	    // only if their sites' pid"s are equal.
	    VSMarshalerBufferOwned *bs = composeVSSiteDeadMsg(sS, myS);
	    if (!myS->isPerm() && myS->isConnected()) {
	      VirtualSite *vs = myS->getVirtualSite();
	      Assert(vs);
	      vs->killResources();
	    }
	    myS->discoveryPerm();
	    // if (vsProbingObject.isProbed(myS))
	    myS->probeFault(PROBE_PERM);
	  }

	  break;
	}

      case VS_M_SITE_ALIVE:
	{
	  DSite *s;
	  VirtualSite *vs;
	  decomposeVSSiteAliveMsg(myVSMarshalerBufferImported, s, vs);
	  s->siteAlive();
	  break;
	}

      case VS_M_SITE_DEAD:
	{
	  DSite *ds;
	  VirtualSite *dvs;
	  decomposeVSSiteDeadMsg(myVSMarshalerBufferImported, ds, dvs);
	  // effectively dead;
	  if (dvs) 
	    dvs->killResources();
	  ds->discoveryPerm();
	  // if (vsProbingObject.isProbed(ds))
	  ds->probeFault(PROBE_PERM);
	}

      case VS_M_YOUR_INDEX_HERE:
	{
	  int index;
	  decomposeVSYourIndexHereMsg(myVSMarshalerBufferImported, index);
	  sVS->setVSIndex(index);
	  break;
	}

      case VS_M_UNUSED_SHMID:
	{
	  DSite *s;
	  key_t shmid;
	  decomposeVSUnusedShmIdMsg(myVSMarshalerBufferImported, s, shmid);
	  importedVSChunksPoolManager->removeSegmentManager(shmid);
	  // kill the segment from the virtual site's 'keys' register:
	  sVS->dropSegManager(shmid);
	  break;
	}

      default:
	OZ_error("readVSMessages: unknown 'vs' message type!");
	break;
      }

      //
      myVSMarshalerBufferImported->unmarshalEnd();
      myVSMarshalerBufferImported->releaseChunks();
      myVSMarshalerBufferImported->cleanup();

      //
      vsResourceManager.finishMsgReceived();
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
      if (vs->tryToSendToAgain(vsm, &freeMarshalerBufferPool)) {
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

#ifndef DENYS_EVENTS
  //
  if (sq->isEmpty())
    am.setMinimalTaskInterval(sqi, 0);
#endif

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
void dumpVirtualMarshalerBufferImpl(MarshalerBuffer* m)
{
  VSMarshalerBufferOwned *buf = (VSMarshalerBufferOwned *) m;

  buf->releaseChunks();
  buf->cleanup();
  freeMarshalerBufferPool.dispose(buf);
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
#ifndef DENYS_EVENTS
  //
  // fprintf(stdout, "cleaning up VSs...\n"); fflush(stdout);
  am.removeTask(&vsSiteQueue, checkMessageQueue);
  am.removeTask(&vsProbingObject, checkProbes);
#endif

  //  
  if (myVSMailboxManager) {
#ifndef DENYS_EVENTS
    am.removeTask(myVSMailboxManager->getMailbox(), checkVSMessages);
#endif
    myVSMailboxManager->destroy();
    delete myVSMailboxManager;
    myVSMailboxManager = (VSMailboxManagerOwned *) 0;
  }

  //
  if (myVSChunksPoolManager) {
#ifndef DENYS_EVENTS
    am.removeTask(myVSChunksPoolManager, checkGCMsgChunks);
#endif
    delete myVSChunksPoolManager;
    myVSChunksPoolManager = (VSMsgChunkPoolManagerOwned *) 0;
  }

  //
  if (importedVSChunksPoolManager) {
    delete importedVSChunksPoolManager;
    importedVSChunksPoolManager = (VSMsgChunkPoolManagerImported *) 0;
  }

  //
  if (myVSMarshalerBufferImported) {
    free(myVSMarshalerBufferImported);
    myVSMarshalerBufferImported = (VSMarshalerBufferImported *) 0;
  }

  //
  // Kill those virtual sites registered locally that are slaves
  // (i.e. whose mailboxes have been created here). Note that
  // 'isSlave()' is not complete: it returns 'NO' for disconnected
  // slaves;
// kost: don't kill anybody because we well may have 'detached'
//       compute (remote) servers;
//   VirtualSite *vs = vsRegister.getFirst();
//   while (vs) {
//     if (vs->isSlave()) {
//       int pid = vs->getVSPid();
//       if (pid)
// 	(void) oskill(pid, SIGTERM);
//     }
//     vs = vsRegister.getNext();
//   }

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
  // installProbe_VirtualSite = installProbe_VirtualSiteImpl;
  // deinstallProbe_VirtualSite = deinstallProbe_VirtualSiteImpl;
  // probeStatus_VirtualSite = probeStatus_VirtualSiteImpl;
  giveUp_VirtualSite = giveUp_VirtualSiteImpl;
  discoveryPerm_VirtualSite = discoveryPerm_VirtualSiteImpl;
  getVirtualMarshalerBuffer = getVirtualMarshalerBufferImpl;
  dumpVirtualMarshalerBuffer = dumpVirtualMarshalerBufferImpl;
  siteAlive_VirtualSite = siteAlive_VirtualSiteImpl;
  virtualSitesExit = virtualSitesExitImpl;

  //
  VSMailboxManagerCreated *mbm;
  VSMarshalerBufferOwned *buf;
  char keyChars[sizeof(key_t)*2 + 3]; // in the form "0xNNNNNNNN";

  //
  // Check whether this site is (already) a virtual one:
  if (!myDSite->hasVirtualInfo()) {
    VirtualInfo *vi;
    key_t myMBoxKey;

    //
    mbm = new VSMailboxManagerCreated(VS_MAILBOX_SIZE);
    if (mbm->isVoid()) {
      delete mbm;
      message("Virtual sites: cannot allocate a shm page!");
      message("Please ask your system administrator to enable");
      message("the shared memory facility/increase system limits.");
      return oz_raise(E_ERROR, E_SYSTEM, "BIVSnewMailbox: cannot create", 0);
    }
    //
    myMBoxKey = mbm->getSHMKey();
    mbm->unmap();
    delete mbm;
    DebugCode(mbm = (VSMailboxManagerCreated *) 0);

    //
    myVSMailboxManager = 
      new VSMailboxManagerOwned(myMBoxKey, &vsResourceManager);
    if (myVSMailboxManager->isVoid()) {
      delete myVSMailboxManager;
      message("Virtual sites: cannot attach a shm page!");
      message("Please ask your system administrator to increase");
      message("the number of shared memory pages allowed.");
      return oz_raise(E_ERROR, E_SYSTEM, "BIVSnewMailbox: cannot attach", 0);
    }

    //
    myVSChunksPoolManager =
      new VSMsgChunkPoolManagerOwned(&vsResourceManager,
				     VS_CHUNK_SIZE, VS_CHUNKS_NUM,
				     VS_REGISTER_HT_SIZE);
    importedVSChunksPoolManager = 
      new VSMsgChunkPoolManagerImported(&vsResourceManager, 
					VS_REGISTER_HT_SIZE);
    myVSMarshalerBufferImported =
      (VSMarshalerBufferImported *) malloc(sizeof(VSMarshalerBufferImported));

    //
    vsResourceManager.init(&vsRegister,
			   myVSChunksPoolManager, 
			   importedVSChunksPoolManager);

    //    
    vi = new VirtualInfo(myDSite, myVSMailboxManager->getSHMKey());
    myDSite->makeMySiteVirtual(vi);

#ifndef DENYS_EVENTS
    //
    // Install the 'read-from-virtual-sites' & 'delayed-write' handlers;
    if (!am.registerTask((void *) myVSMailboxManager->getMailbox(),
			 checkVSMessages, readVSMessages))
      OZ_error("virtual sites: unable to register a task");
    // This task is actually not needed - only if the implementation
    // is buggy and thus some messages are lost;
    am.setMinimalTaskInterval((void *) myVSMailboxManager->getMailbox(),
			      CHECKMAIL_INTERVAL);

    if (!am.registerTask((void *) &vsSiteQueue,
			 checkMessageQueue, processMessageQueue))
      OZ_error("virtual sites: unable to register a task");
    // 'vsSiteQueue' gets pretty short interval, but only in case
    // when delayed messages emerge;

    if (!am.registerTask((void *) &vsProbingObject,
			 checkProbes, processProbes))
      OZ_error("virtual sites: unable to register a task");
    // (if the install/deinstall probes business is re-enabled again then
    // this 'setMinimalTaskInterval' should be removed);
    am.setMinimalTaskInterval((void *) &vsProbingObject, PROBE_INTERVAL);

    if (!am.registerTask((void *) myVSChunksPoolManager,
			 checkGCMsgChunks, processGCMsgChunks))
      OZ_error("virtual sites: unable to register a task");
    // This should be a pretty large interval;
    am.setMinimalTaskInterval((void *) myVSChunksPoolManager,
			      VS_SEGS_MAXPHASE_MS);
#endif
  }

  //
  mbm = new VSMailboxManagerCreated(VS_MAILBOX_SIZE);
  if (mbm->isVoid()) {
    delete mbm;
    (void) vsResourceManager.doResourceGC(VS_RM_NewMbox, VS_RM_Create,
					  VS_RM_Block);
    // next, and last try:
    mbm = new VSMailboxManagerCreated(VS_MAILBOX_SIZE);
    if (mbm->isVoid()) {
      delete mbm;
      message("Virtual sites: cannot attach a shm page!");
      message("Please ask your system administrator to increase");
      message("the number of shared memory pages allowed.");
      return oz_raise(E_ERROR, E_SYSTEM, "BIVSnewMailbox: cannot create", 0);
    }
  }
  //
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
  freeMarshalerBufferPool.dispose(buf);
  DebugCode(buf = (VSMarshalerBufferOwned *) 0);

  //
  mbm->unmap();			// we don't need that object now anymore;
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
  // installProbe_VirtualSite = installProbe_VirtualSiteImpl;
  // deinstallProbe_VirtualSite = deinstallProbe_VirtualSiteImpl;
  // probeStatus_VirtualSite = probeStatus_VirtualSiteImpl;
  giveUp_VirtualSite = giveUp_VirtualSiteImpl;
  discoveryPerm_VirtualSite = discoveryPerm_VirtualSiteImpl;
  getVirtualMarshalerBuffer = getVirtualMarshalerBufferImpl;
  dumpVirtualMarshalerBuffer = dumpVirtualMarshalerBufferImpl;
  siteAlive_VirtualSite = siteAlive_VirtualSiteImpl;
  virtualSitesExit = virtualSitesExitImpl;

  //
#ifdef DEBUG_CHECK
//   fprintf(stdout, "*** Sleeping for 10 secs - hook it up (pid %d)!\n",
// 	  osgetpid());
//   fflush(stdout);
//   sleep(10);			// IMHO more than enough;
#endif

  //
  Assert(sizeof(key_t) == sizeof(int));
  if (sscanf(mbKeyChars, "%i", (int*)&mbKey) != 1)
    return oz_raise(E_ERROR,E_SYSTEM,"VSinitServer: invalid arg",0);

  //
  if (myVSMailboxManager)
    return oz_raise(E_ERROR,E_SYSTEM,"VSinitServer again",0);

  //
  // We know here the id of the mailbox, so let's fetch it:
  myVSMailboxManager = new VSMailboxManagerOwned(mbKey, &vsResourceManager);
  if (myVSMailboxManager->isVoid()) {
    delete myVSMailboxManager;
    message("Virtual sites: cannot attach a shm page!");
    message("Please ask your system administrator to enable");
    message("the shared memory facility/increase system limits.");
    return oz_raise(E_ERROR, E_SYSTEM, "BIVSnewMailbox: cannot attach", 0);
  }

  //
  myVSChunksPoolManager =
    new VSMsgChunkPoolManagerOwned(&vsResourceManager,
				   VS_CHUNK_SIZE, VS_CHUNKS_NUM,
				   VS_REGISTER_HT_SIZE);
  importedVSChunksPoolManager = 
    new VSMsgChunkPoolManagerImported(&vsResourceManager, 
				      VS_REGISTER_HT_SIZE);

  //
  vsResourceManager.init(&vsRegister,
			 myVSChunksPoolManager, 
			 importedVSChunksPoolManager);

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
  myVSMarshalerBufferImported =
    (VSMarshalerBufferImported *) malloc(sizeof(VSMarshalerBufferImported));
  myVSMarshalerBufferImported = new (myVSMarshalerBufferImported)
    VSMarshalerBufferImported(importedVSChunksPoolManager,
			msgChunkPoolKey, chunkNumber);
  // (we must read-in the type field);
  myVSMarshalerBufferImported->unmarshalBegin();

  //
  // Check if something went wrong just from scratch:
  if (myVSMarshalerBufferImported->isVoid()) {
    myVSMarshalerBufferImported->dropVoid();
    myVSMarshalerBufferImported->cleanup();
    // "exit hook" should clean up everthing...
    return oz_raise(E_ERROR, E_SYSTEM, "VS: no init message", 1,
		    oz_atom(mbKeyChars));
  }
  //
  msgType = getVSMsgType(myVSMarshalerBufferImported);
  if (msgType != VS_M_INIT_VS)
    OZ_error("Virtual sites: malformed init message");

  //
  // The father's virtual site is registered during
  // unmarshaling, but it is NOT recognized as a virtual one
  // (since 'myDSite' has not been yet initialized - a
  // bootstrapping problem! :-))
  decomposeVSInitMsg(myVSMarshalerBufferImported, ms);
  myVSMarshalerBufferImported->unmarshalEnd();

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
  myVSMarshalerBufferImported->releaseChunks();
  myVSMarshalerBufferImported->cleanup();

#ifndef DENYS_EVENTS
  //
  // (see the comments in BIVSnewMailbox;)
  if (!am.registerTask((void *) myVSMailboxManager->getMailbox(),
		       checkVSMessages, readVSMessages))
      OZ_error("virtual sites: unable to register a task");
  am.setMinimalTaskInterval((void *) myVSMailboxManager->getMailbox(),
			    CHECKMAIL_INTERVAL);
  if (!am.registerTask((void *) &vsSiteQueue,
		       checkMessageQueue, processMessageQueue))
      OZ_error("virtual sites: unable to register a task");
  if (!am.registerTask((void *) &vsProbingObject,
		       checkProbes, processProbes))
      OZ_error("virtual sites: unable to register a task");
  am.setMinimalTaskInterval((void *) &vsProbingObject, PROBE_INTERVAL);
  if (!am.registerTask((void *) myVSChunksPoolManager,
		       checkGCMsgChunks, processGCMsgChunks))
      OZ_error("virtual sites: unable to register a task");
  am.setMinimalTaskInterval((void *) myVSChunksPoolManager,
			    VS_SEGS_MAXPHASE_MS);
#endif
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

#ifdef DENYS_EVENTS

OZ_BI_define(BIVSprocess_mailbox,0,1)
{
  // returns true iff there are still messages in the mailbox
  unsigned long clock = am.getEmulatorClock();
  readVSMessages(clock,(void*)myVSMailboxManager->getMailbox());
  OZ_RETURN_BOOL(myVSMailboxManager->getMailbox()->isNotEmpty());
}
OZ_BI_end

OZ_BI_define(BIVSprocess_messageq,0,1)
{
  // returns true iff there are still messages in the queue
  unsigned long clock = am.getEmulatorClock();
  if (vsSiteQueue.isNotEmpty())
    processMessageQueue(clock,(void*)&vsSiteQueue);
  OZ_RETURN_BOOL(vsSiteQueue.isNotEmpty());
}
OZ_BI_end

OZ_BI_define(BIVSprocess_probes,0,1)
{
  unsigned long clock = am.getEmulatorClock();
  if (checkProbes(clock,(void*)&vsProbingObject))
    processProbes(clock,(void*)&vsProbingObject);
  OZ_RETURN(OZ_true());
}
OZ_BI_end

OZ_BI_define(BIVSprocess_gc,0,1)
{
  unsigned long clock = am.getEmulatorClock();
  if (checkGCMsgChunks(clock,(void*)myVSChunksPoolManager))
    processGCMsgChunks(clock,(void*)myVSChunksPoolManager);
  OZ_RETURN(OZ_true());
}
OZ_BI_end

#endif

/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modVirtualSite-if.cc"

#endif
