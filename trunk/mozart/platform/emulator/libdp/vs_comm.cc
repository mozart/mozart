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

#if defined(INTERFACE)
#pragma implementation "vs_comm.hh"
#endif
#if defined(INTERFACE)
#pragma implementation "vs_lock.hh"
#endif

#include "base.hh"
#include "dpBase.hh"

#ifdef VIRTUALSITES

#include "am.hh"
#include "vs_comm.hh"
#include "virtual.hh"

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

//
// Currently all the parameters are ignored: resource manager
// works until a shm page is reclaimed (or raises an error if that
// take too long);
Bool VSResourceManager::doResourceGC(VS_RM_Source source,
				     VS_RM_Reason reason,
				     VS_RM_GCMode gcmode)
{
  int wasMapped = mapped;
  int rounds = 0;
  // 'minShmPages' is the minimal number of pages we need to have
  // allocated (see comments below);
  int minShmPages;

  // being in a non-idle state means that's a recursive call;
  if (state != VS_RM_idle)
    return (OK);		// speculative;

  //
  // There are (a) 1 own mailbox, (b) at least 1 own msg buffer
  // segment, (c) may be peer's mailbox, (d) may be peer's msg
  // buffers. So, at least 2:
  minShmPages = 2;		// min own configuration;
  if (peerSite) {
    minShmPages++;		// peer's mailbox and msg buffers;
    minShmPages += peerSite->getKeysRegister()->getSize();
  }
  //
  Assert(mapped >= minShmPages);
  // If the limit is already reached, just bail out
  if (mapped == minShmPages)
    return (NO);

  //
  Assert(state == VS_RM_idle);
  state = VS_RM_scavenge;	// start with plain scavenging;

  //
  while (mapped >= wasMapped) {
    //
    switch (state) {

    case VS_RM_scavenge:
      myCPM->scavenge();
      state = VS_RM_unmapMBufs;
      break;			// switch

    case VS_RM_unmapMBufs:
      {
	//
	// Every time we get here the process starts with another
	// virtual site (a next one wrt the order in the register);
	int vsRegSize = vsRegister->getSize();
	Assert(vsRegSize);		// how it can happen???
	if (currentStart > vsRegSize)
	  currentStart = 0;

	//
	VirtualSite *vs = vsRegister->getNth(currentStart++);
	while (vs && vsRegSize) {
	  // don't try to unmap anything from a site we are receiving
	  // a message;
	  if (vs != peerSite) {
	    //
	    // shmid"s are taken in a round-robin fashion: we try to
	    // continue from a place we've been last time (if there is
	    // any, of course);
	    VSSegKeysRegister* vsSKR = vs->getKeysRegister();
	    if (!vsSKR->canGetNext())
	      vsSKR->startSeq();

	    //
	    key_t shmid = vsSKR->getNext();
	    while (shmid != (key_t) -1) {
	      impCPM->removeSegmentManager(shmid);
	      vs->dropSegManager(shmid);

	      //
	      // actually, that should be always the case:
	      if (mapped < wasMapped)
		break;		// while
	      shmid = vsSKR->getNext();
	    }

	    //	    
	    if (mapped < wasMapped)
	      break;		// while
	  }

	  //
	  vs = vsRegister->getNextCircular();
	  vsRegSize--;
	}

	//
	state = VS_RM_scavengeBurst;
	break;			// switch
      }

    case VS_RM_scavengeBurst:
      myCPM->scavenge();
      state = VS_RM_disconnect;
      break;			// switch

    case VS_RM_disconnect:
      {
	//
	int vsRegSize = vsRegister->getSize();
	Assert(vsRegSize);		// how it can happen???
	if (currentStart > vsRegSize)
	  currentStart = 0;

	//
	VirtualSite *vs = vsRegister->getNth(currentStart++);
	while (vs && vsRegSize) {
	  if (vs != peerSite && vs->isConnected())
	    vs->disconnect();
	  //
	  // actually, should be always the case;
	  if (mapped < wasMapped)
	    break;		// while

	  //
	  vs = vsRegister->getNextCircular();
	  vsRegSize--;
	}

	//
	state = VS_RM_wait;
	break;			// switch
      }

    case VS_RM_wait:
      OZ_warning("Virtual sites: stalling because of lack of resources...");
      rounds++;
      if (rounds > VS_WAIT_ROUNDS) {
	OZ_warning("Virtual sites: cannot reclaim a resource!");
	state = VS_RM_idle;
	return (NO);
      } else {
	ossleep(VS_RESOURCE_WAIT);
	state = VS_RM_scavenge;
      }
      break;

    default:
      OZ_error("Virtual sites: illegal resource manager's state");
    }
  }

  //
  state = VS_RM_idle;
  return (OK);
}

//
//
void VirtualSite::connect()
{
  //
  Assert(!isConnected());
  switch (getSiteStatus()) {

  case SITE_TEMP: OZ_error("A virtual site temporarily unreachable???");
  case SITE_PERM: break;

  case SITE_OK:
    {
      //
      // Creation of a mailbox manager includes mapping the mailbox into
      // our address space (in terms of the "network" transport layer, a
      // "write" connection is established);
      key_t mboxKey = site->getVirtualInfo()->getMailboxKey();
      mboxMgr = new VSMailboxManagerImported(mboxKey, vsRM);

      //
      if (mboxMgr->isVoid()) {
	// upgrade the status of:
	switch (mboxMgr->getErrno()) {

#if defined(SOLARIS)
	case EMFILE:
#else
	  // Linux, in turn, can handle apparently as many shm pages
	  // as needed until their total size does not hit the process
	  // size limitation (i'm not sure - that's a guess);
#endif
	case ENOMEM:
	  // resources problem - do GC & just remain unconnected (with
	  // the hope that the next step will be successful);
	  (void) vsRM->doResourceGC(VS_RM_Mailbox, VS_RM_Attach, VS_RM_Block);
	  break;

	default:
	  // we cann't handle all other problems;
	  status = SITE_PERM;
	  break;
	}

	//
	delete mboxMgr;
	mboxMgr = (VSMailboxManagerImported *) 0;
	Assert(!isConnected());
      } else {
	Assert(isConnected());
      }
      break;
    }

  default:
    OZ_error("wrong virtual site status!");
  }
}

//
//
VirtualSite::VirtualSite(DSite *s, VSFreeMessagePool *fmpIn,
			 VSSiteQueue *sqIn, VSResourceManager* vsRMin,
			 VSMsgChunkPoolManagerImported *cpmImpIn)
  : VSRegisterNode(this), VSSiteQueueNode(this),
    site(s), vsIndex(-1), status(SITE_OK), vsStatus(0), 
    isAliveSent(0), aliveAck(0),
    probeAllCnt(0), probePermCnt(0),
    vsRM(vsRMin), cpmImp(cpmImpIn), fmp(fmpIn), sq(sqIn),
    mboxMgr((VSMailboxManagerImported *) 0),
    segKeysNum(0), segKeysArraySize(0), segKeys((key_t *) 0),
    keysRegister(VS_KEYSREG_SIZE)
{
  // not connected originally;
}

//
void VirtualSite::disconnect()
{
  Assert(isConnected());
  gcResources();		// unmap msgbuffers' shm pages;
  mboxMgr->unmap();		// this op can be done always;
  delete mboxMgr;
  mboxMgr = (VSMailboxManagerImported *) 0;
}

//
// Droping a virtual site occurs when it is known (e.g. from a third
// party) to be dead;
void VirtualSite::drop()
{
  //
  // Throw away all the undeliverable now messages (if any);
  if (isNotEmpty()) {
    retractVSSiteQueueNode();
    //
    while (isNotEmpty()) {
      VSMessage *vsm = dequeue();
      /* tmueller
      site->communicationProblem(vsm->getMessageType(),
				 vsm->getSite(), vsm->getStoreIndex(),
				 COMM_FAULT_PERM_NOT_SENT,
				 (FaultInfo) vsm->getMarshalerBuffer());
      */
      fmp->dispose(vsm);
    }
  }

  //
  // should result in unmapping:
  if (isConnected()) disconnect();
  // both assertions are supposed to be equivalent:
  Assert(!isConnected());
  Assert(mboxMgr == (VSMailboxManagerImported *) 0);

  //
  status = SITE_PERM;

  //
  DebugCode(segKeysNum = 0;);
  if (segKeys)  {
    free(segKeys);
    segKeys = (key_t *) 0;
    segKeysArraySize = 0;
  }
}

//
// The message type, store site and store index parameters 
// are opaque data (just stored);
int VirtualSite::sendTo(VSMarshalerBufferOwned *mb, MessageType mt,
			DSite *storeSite, int storeIndex,
			FreeListDataManager<VSMarshalerBufferOwned> *freeMBs)
{
  //
  // 'mb' must be indeed an object of the 'VSMarshalerBufferOwned' type;
  Assert(mb->getSite()->virtualComm());

  //
  // Let's TRY to connect if it isn't yet (this can fail).
  // 'connect()' updates eventually the status of the VS;
  if (!isConnected()) connect();

  //
  // 
  switch (getSiteStatus()) {
    // temporary problems are hidden by the VS comm layer:
  case SITE_TEMP:
    OZ_error("A virtual site temporarily unreachable???");
    return (PERM_NOT_SENT);

  case SITE_PERM:
    return (PERM_NOT_SENT);

  case SITE_OK:
    //
    // First, let's try to deliver it *now*.
    // If it fails, a message (job) for delayed delivery is created; 
    VSMailboxImported *mbox;
    VirtualInfo *myVI;
    //
    if (isConnected()) 
      mbox = mboxMgr->getMailbox();
    DebugCode(else mbox = (VSMailboxImported *) -1;);
    myVI = myDSite->getVirtualInfo();

    //
    // In either of the cases (note - the first one means we had
    // problems with mapping the memory page right before!);
    if (!isConnected () ||
	isNotEmpty() || 
	!mbox->enqueue(mb->getFirstChunkSHMKey(),
		       mb->getFirstChunkNum())) {
      // The queue wasn't empty, or failed to enqueue it inline - then
      // create a job that will try to do that later;
      VSMessage *voidM = fmp->allocate();
      VSMessage *m = new (voidM) VSMessage(mb, mt, storeSite, storeIndex);

      //
      // These queues are checked and processed on regular intervals
      // (OS' alarms);
      if (isEmpty()) {
	Bool wasEmptySQ = sq->isEmpty();
	sq->enqueue(this);
#ifndef DENYS_EVENTS
	if (wasEmptySQ)
	  am.setMinimalTaskInterval((void *) sq, SITEQUEUE_INTERVAL);
#endif
      }
      enqueue(m);

      //
      // Note that there is no 'TEMP_NOT_SENT' now;
    } else {
      mb->passChunks();
      mb->cleanup();
      freeMBs->dispose(mb);
    }

    //
#ifdef DENYS_EVENTS
    {
      static TaggedRef VSMsgQ = oz_atom("VSMsgQ");
      OZ_eventPush(VSMsgQ);
    }
#endif
    return (ACCEPTED);
  }
  return(ACCEPTED);	// to make gcc happy;
}

//
// ... retry to send it with (it takes an unsent message, compared to
// 'sendTo()'). It says 'TRUE' if we got it;
Bool
VirtualSite::tryToSendToAgain(VSMessage *vsm, 
			      FreeListDataManager<VSMarshalerBufferOwned> *freeMBs)
{
  //
  if (!isConnected()) connect();

  //
  // 
  switch (getSiteStatus()) {
  case SITE_TEMP:
    OZ_error("A virtual site temporarily unreachable???");
    return (TRUE);

  case SITE_PERM: 
    return (PERM_NOT_SENT);

  case SITE_OK:
    {
      //
      // First, let's try to deliver it *now*.
      // If it fails, a message (job) for delayed delivery is created; 
      VSMarshalerBufferOwned *mb;
      VSMailboxImported *mbox;
      VirtualInfo *myVI;

      //
      mb = vsm->getMarshalerBuffer();
      if (isConnected()) 
	mbox = mboxMgr->getMailbox();
      myVI = myDSite->getVirtualInfo();

      //
      if (isConnected() && mbox->enqueue(mb->getFirstChunkSHMKey(),
					 mb->getFirstChunkNum())) {
	vsm->retractVSMsgQueueNode();
	fmp->dispose(vsm);
	mb->passChunks();
	mb->cleanup();
	freeMBs->dispose(mb);

	//
	return (TRUE);
      } else {
	// Failed (again) to enqueue it;
	return (FALSE);
      }
    }

  default: 
    OZ_error("wrong virtual site status!");
    return (TRUE);
  }
}

//
void VirtualSite::marshalLocalResources(MarshalerBuffer *mb,
					VSMailboxManagerOwned *mbm,
					VSMsgChunkPoolManagerOwned *cpm)
{
  Assert(sizeof(int) <= sizeof(unsigned int));
  Assert(sizeof(key_t) <= sizeof(unsigned int));
  int num;
  key_t mbKey = mbm->getSHMKey();

  //
  num = cpm->getSegsNum();
  marshalNumber(mb, num+1);	// mailbox key;
  //
  marshalNumber(mb, mbKey);
  for (int i = 0; i < num; i++)
    marshalNumber(mb, cpm->getSegSHMKey(i));
}

//
// The 'segKeys' array is allocated with "
void VirtualSite::unmarshalResources(MarshalerBuffer *mb)
{
  segKeysNum = unmarshalNumber(mb);
  Assert(segKeysNum);
  if (segKeysNum > segKeysArraySize) {
    int acc = segKeysNum, bits = 0;
    while (acc) {
      acc = acc/2;
      bits++;
    }
    bits = max(bits+2, 4);	// heuristic...
    segKeysArraySize = (int) (0x1 << bits);

    //
    if (segKeys) free (segKeys);
    segKeys = (key_t *) malloc(sizeof(key_t) * segKeysArraySize);
  }
  Assert(segKeys);
  Assert(segKeysArraySize >= segKeysNum);

  //
  for (int i = 0; i < segKeysNum; i++) 
    segKeys[i] = (key_t) unmarshalNumber(mb);
}

//
void VirtualSite::gcResources()
{
  keysRegister.startSeq();
  key_t shmid = keysRegister.getNext();
  while (shmid != (key_t) -1) {
    cpmImp->removeSegmentManager(shmid);
    shmid = keysRegister.getNext();
  }
}
//
// ... yet mark all the known shmid"s dead (if they were not killed
// already by the owner itself or even some other contacting with it
// site);
void VirtualSite::killResources()
{
  gcResources();
  for (int i = 0; i < segKeysNum; i++)
    markDestroy(segKeys[i]);
}

//
//
GenHashNode* VSSiteHashTable::findNode(int hvalue, DSite *s)
{
  GenHashNode *aux = htFindFirst(hvalue);
  DSite *sf;

  while(aux) {
    GenCast(aux->getEntry(), GenHashEntry*, sf, DSite*);

    //
    if (!s->compareSites(sf))
      return (aux);

    //
    aux = htFindNext(aux, hvalue);
  }

  //
  return ((GenHashNode *) 0);
}

//
//
Bool VSSiteHashTable::check(DSite *s)
{
  GenHashNode *ghn = findNode(hash(s), s);
  return (ghn ? TRUE : FALSE);
}

//
void VSSiteHashTable::enter(DSite *s)
{
  GenHashBaseKey* ghn_bk;
  GenHashEntry* ghn_e;
  int hvalue = hash(s);
  Assert(!check(s));

  //
  // Actually we don't need keys...
  GenCast(s, DSite*, ghn_bk, GenHashBaseKey*);
  GenCast(s, DSite*, ghn_e, GenHashEntry*);
  //
  htAdd(hvalue, ghn_bk, ghn_e);
}

//
void VSSiteHashTable::remove(DSite *s) {
  int hvalue = hash(s);
  GenHashNode *ghn = findNode(hvalue, s);
  Assert(ghn);
  htSub(hvalue, ghn);
}

//
DSite *VSSiteHashTable::getFirst()
{
  DSite *s;
  seqGHN = GenHashTable::getFirst(seqIndex);
  if (seqGHN) {
    GenCast(seqGHN->getEntry(), GenHashEntry*, s, DSite*);
  } else {
    s = (DSite *) 0;
  }
  return (s);
}

//
DSite *VSSiteHashTable::getNext()
{
  DSite *s;
  seqGHN = GenHashTable::getNext(seqGHN, seqIndex);
  if (seqGHN) {
    GenCast(seqGHN->getEntry(), GenHashEntry*, s, DSite*);
  } else {
    s = (DSite *) 0;
  }
  return (s);
}

//
VSProbingObject::VSProbingObject(int size, VSRegister *vsRegisterIn)
  : VSSiteHashTable(size), 
      vsRegister(vsRegisterIn),
      probesNum(0), lastCheck(0), lastPing(0), 
    minInterval(PROBE_INTERVAL)
{}

//
// Note we cannot ignore probe types even though there is only type
// of problems - the permanent one. This is because the perdio layer
// can ask for different probes separately;
//
// kost@ : if the 'installProbe()'/'deinstallProbe()' business is going
// to be re-enabled, then 'am.setMinimalTaskInterval()' in virtual.cc
// should be removed;
/*
ProbeReturn VSProbingObject::installProbe(VirtualSite *vs,
					  ProbeType pt, int frequency)
{
  DSite *s = vs->getSite();
  Assert(s->virtualComm());

  //
  if (!probesNum)
    am.setMinimalTaskInterval((void *) this, PROBE_INTERVAL);

  //
  if (check(s)) {
    Assert(vs->hasProbesAll() || vs->hasProbesPerm());
  } else {
    Assert(!vs->hasProbesAll() && !vs->hasProbesPerm());
    enter(s);
    probesNum++;
  }

  //
  switch (pt) {
  case PROBE_TYPE_ALL:
    vs->incProbesAll();
    break;

  case PROBE_TYPE_PERM:
    vs->incProbesPerm();
    break;

  default:
    OZ_error("Virtual sites: unexpected type of probe");
    break;
  }

  //
  return (PROBE_INSTALLED);
}
*/

//
/*
ProbeReturn VSProbingObject::deinstallProbe(VirtualSite *vs, ProbeType pt)
{
  ProbeReturn ret;
  DSite *s = vs->getSite();
  Assert(s->virtualComm());

  //
  switch (pt) {
  case PROBE_TYPE_ALL:
    if (vs->hasProbesAll()) {
      vs->decProbesAll();
      ret = PROBE_DEINSTALLED;
    } else {
      ret = PROBE_NONEXISTENT;
    }
    break;

  case PROBE_TYPE_PERM:
    if (vs->hasProbesPerm()) {
      vs->decProbesPerm();
      ret = PROBE_DEINSTALLED;
    } else {
      ret = PROBE_NONEXISTENT;
    }
    break;

  default:
    OZ_error("Virtual sites: unexpected type of probe");
    ret = PROBE_NONEXISTENT;
    break;
  }

  //
  Assert(check(s));
  if (!vs->hasProbesAll() && !vs->hasProbesPerm()) {
    remove(s);
    probesNum--;
  }

  //
  if (!probesNum)
    am.setMinimalTaskInterval((void *) this, 0);

  //
  return (ret);
}
*/

//
Bool VSProbingObject::processProbes(unsigned long clock,
				    VSMsgChunkPoolManagerImported *cpm)
{
  VirtualSite *vs;

  //
  vs = vsRegister->getFirst();
  while (vs) {
    DSite *s = vs->getSite();
    Assert(s->isConnected());

    //
    // We would do this check always, even when the site is physically
    // disconnected, but - unfortunately - its pid is not known in
    // that case;
    if (vs->isConnected()) {
      int pid = vs->getVSPid();
      if (pid) {
	// delete zombie;
	(void) waitpid(pid, (int *) 0, WNOHANG);
	if (oskill(pid, 0)) {
	  // no such process;
	  vs->killResources();
	  s->discoveryPerm();
	  // kost@ : we don't check now whether the site is registered
	  // for probing:
	  // if (check(s)) {
	  // Assert(vs->hasProbesAll() || vs->hasProbesPerm());
	  s->probeFault(PROBE_PERM);
	}
      }
    }

    //
    vs = vsRegister->getNext();
  }

  //
  if (clock - lastPing > PROBE_WAIT_TIME) {
    //
    vs = vsRegister->getFirst();
    while (vs) {
      DSite *s = vs->getSite();
      Assert(s->isConnected());

      //
      if (vs->isConnected()) {
	//
	// First check old pings;
	if (vs->getTimeIsAliveSent() >= lastPing &&
	    vs->getTimeIsAliveSent() > vs->getTimeAliveAck()) {
	  // kost@ : TODO: being here means we have a temporary
	  // problem (either the site is just slow, or it is spinning
	  // due to a bug, or it is being traced (debugged));
	  ;

	  //
	} else {
	  //
	  // if it seems to be alive, init a new round...
	  VSMarshalerBufferOwned *mb = composeVSSiteIsAliveMsg(s);
	  if (sendTo_VirtualSiteImpl(vs, mb, /* messageType */ M_NONE,
				     /* storeSite */ (DSite *) 0,
				     /* storeIndex */ 0) != ACCEPTED) {
	    vs->killResources();
	    s->discoveryPerm();
	    // if (check(s)) {
	    // Assert(vs->hasProbesAll() || vs->hasProbesPerm());
	    s->probeFault(PROBE_PERM);
	  }

	  //
	  vs->setTimeIsAliveSent(clock);
	}
      }

      //
      vs = vsRegister->getNext();
    }

    //
    lastPing = clock;
  }

  //
  return (TRUE);
}

#endif // VIRTUALSITES
