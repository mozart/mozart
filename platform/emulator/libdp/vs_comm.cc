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

#if defined(INTERFACE)
#pragma implementation "vs_comm.hh"
#endif
#if defined(INTERFACE)
#pragma implementation "vs_lock.hh"
#endif

#include "am.hh"

#ifdef VIRTUALSITES

#include "vs_comm.hh"
#include "virtual.hh"


#include <sys/types.h>
#include <sys/wait.h>

//
void VirtualSite::connect()
{
  //
  if (mboxMgr == (VSMailboxManagerImported *) 0) {
    if (isSetVSFlag(VS_PENDING_UNMAP_MBOX)) {
      clearVSFlag(VS_PENDING_UNMAP_MBOX);
    } else {
      //
      // Creation of a mailbox manager includes mapping the mailbox
      // into our address space (that is, a "write" connection is 
      // established;
      key_t mboxKey = site->getVirtualInfo()->getMailboxKey();
      mboxMgr = new VSMailboxManagerImported(mboxKey);
    }
  }
}

//
//
VirtualSite::VirtualSite(DSite *s,
			 VSFreeMessagePool *fmpIn,
			 VSSiteQueue *sqIn)
  : VSRegisterNode(this), VSSiteQueueNode(this),
    site(s), status(SITE_OK), vsStatus(0), 
    isAliveSent(0), aliveAck(0),
    probeAllCnt(0), probePermCnt(0),
    fmp(fmpIn), sq(sqIn),
    mboxMgr((VSMailboxManagerImported *) 0),
    segKeysNum(0), segKeysArraySize(0), segKeys((key_t *) 0)
{
  connect();
}

//
void VirtualSite::disconnect()
{
  if (isNotEmpty()) {
    setVSFlag(VS_PENDING_UNMAP_MBOX);
  } else {
    mboxMgr->unmap();
    delete mboxMgr;
    clearVSFlag(VS_PENDING_UNMAP_MBOX);
    mboxMgr = (VSMailboxManagerImported *) 0;
  }    
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
      site->communicationProblem(vsm->getMessageType(),
				 vsm->getSite(), vsm->getStoreIndex(),
				 COMM_FAULT_PERM_NOT_SENT,
				 (FaultInfo) vsm->getMsgBuffer());
      fmp->dispose(vsm);
    }
  }

  //
  // should result in unmapping:
  disconnect();
  Assert(mboxMgr == (VSMailboxManagerImported *) 0);

  //
  // 'SITE_PERM' is redundant - used for consistency checks only;
  status = SITE_PERM;
  Assert(!isSetVSFlag(VS_PENDING_UNMAP_MBOX));

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
int VirtualSite::sendTo(VSMsgBufferOwned *mb, MessageType mt,
			DSite *storeSite, int storeIndex,
			FreeListDataManager<VSMsgBufferOwned> *freeMBs)
{
  //
  // 'mb' must be indeed an object of the 'VSMsgBufferOwned' type;
  Assert(mb->getSite()->virtualComm());

  //
  // 
  switch (getSiteStatus()) {
  case SITE_TEMP: OZ_error("A virtual site temporarily down???");
  case SITE_PERM: OZ_error("Attempt to send to a 'perm' virtual site!");
  case SITE_OK:
    //
    // First, let's try to deliver it *now*.
    // If it fails, a message (job) for delayed delivery is created; 
    Assert(mboxMgr);		// must be already connected;
    VSMailboxImported *mbox = mboxMgr->getMailbox();
    VirtualInfo *myVI = myDSite->getVirtualInfo();

    //
    //
    if (isNotEmpty() || !mbox->enqueue(mb->getFirstChunkSHMKey(),
				       mb->getFirstChunkNum())) {
      // The queue wasn't empty, or failed to enqueue it inline - then
      // create a job which will try to do that later;
      VSMessage *voidM = fmp->allocate();
      VSMessage *m = new (voidM) VSMessage(mb, mt, storeSite, storeIndex);

      //
      // These queues are checked and processed on regular intervals
      // (OS' alarms);
      if (isEmpty())
	sq->enqueue(this);
      enqueue(m);

      // Note that there is no 'TEMP_NOT_SENT' now;
    } else {
      mb->passChunks();
      mb->cleanup();
      freeMBs->dispose(mb);
    }

    //
    return (ACCEPTED);
  }
  return(ACCEPTED);	// to make gcc happy;
}

//
// ... retry to send it with (it takes an unsent message, compared to
// 'sendTo()'). It says 'TRUE' if we got it;
Bool VirtualSite::tryToSendToAgain(VSMessage *vsm, 
				  FreeListDataManager<VSMsgBufferOwned> *freeMBs)
{
  //
  // 
  switch (getSiteStatus()) {
  case SITE_TEMP:
    OZ_error("A virtual site temporarily down???");
    return (TRUE);

  case SITE_PERM:
    OZ_error("Attempt to re-send to a 'perm' virtual site!");
    return (PERM_NOT_SENT);

  case SITE_OK:
    {
      //
      // First, let's try to deliver it *now*.
      // If it fails, a message (job) for delayed delivery is created; 
      VSMsgBufferOwned *mb = vsm->getMsgBuffer();
      VSMailboxImported *mbox = mboxMgr->getMailbox();
      VirtualInfo *myVI = myDSite->getVirtualInfo();

      //
      if (mbox->enqueue(mb->getFirstChunkSHMKey(),
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
void VirtualSite::marshalLocalResources(MsgBuffer *mb,
					VSMailboxManagerOwned *mbm,
					VSMsgChunkPoolManagerOwned *cpm)
{
  Assert(sizeof(int) <= sizeof(unsigned int));
  Assert(sizeof(key_t) <= sizeof(unsigned int));
  int num;
  key_t mbKey = mbm->getSHMKey();

  //
  num = cpm->getSegsNum();
  marshalNumber(num+1, mb);	// mailbox key;
  //
  marshalNumber(mbKey, mb);
  for (int i = 0; i < num; i++)
    marshalNumber(cpm->getSegSHMKey(i), mb);
}

//
// The 'segKeys' array is allocated with "
void VirtualSite::unmarshalResources(MsgBuffer *mb)
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
void VirtualSite::gcResources(VSMsgChunkPoolManagerImported *cpm)
{
  for (int i = 0; i < segKeysNum; i++) {
    markDestroy(segKeys[i]);
    // ... aka got an 'VS_M_UNUSED_SHMID' message:
    cpm->removeSegmentManager(segKeys[i]);
  }
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

//
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

#ifdef DEBUG_CHECK
#define VS_PROBING_BELIEVE_OK
#endif

//
Bool VSProbingObject::processProbes(unsigned long clock,
				    VSMsgChunkPoolManagerImported *cpm)
{
  VirtualSite *vs;

  //
  vs = vsRegister->getFirst();
  while (vs) {
    DSite *s = vs->getSite();
    //
    if (s->isConnected()) {
      int pid;

      //
      pid = vs->getVSPid();
      if (pid) {
	// delete zombie;
	(void) waitpid(pid, (int *) 0, WNOHANG);
	if (oskill(pid, 0)) {
	  // no such process;
	  vs->gcResources(cpm);
	  s->discoveryPerm();
	  if (check(s)) {
	    Assert(vs->hasProbesAll() || vs->hasProbesPerm());
	    s->probeFault(PROBE_PERM);
	  }
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
      //
      if (s->isConnected()) {
	int pid;

	//
	// First check old pings;
#ifndef VS_PROBING_BELIEVE_OK
	if (vs->getTimeIsAliveSent() >= lastPing &&
	    vs->getTimeIsAliveSent() > vs->getTimeAliveAck()) {
#else
	if (0) {
#endif // VS_PROBING_BELIEVE_OK
	  // effectively dead;
	  vs->gcResources(cpm);
	  s->discoveryPerm();
	  if (check(s))
	    s->probeFault(PROBE_PERM);
	} else {
	  // if it seems to be alive, init a new round...
	  VSMsgBufferOwned *mb = composeVSSiteIsAliveMsg(s);
	  if (sendTo_VirtualSiteImpl(vs, mb, /* messageType */ M_NONE,
				     /* storeSite */ (DSite *) 0,
				     /* storeIndex */ 0) != ACCEPTED) {
	    vs->gcResources(cpm);
	    s->discoveryPerm();
	    if (check(s)) {
	      Assert(vs->hasProbesAll() || vs->hasProbesPerm());
	      s->probeFault(PROBE_PERM);
	    }
	  }
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
