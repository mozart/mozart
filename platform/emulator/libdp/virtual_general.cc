/*
 *  Authors:
 *    Andreas Sundstrom <andreas@sics.se>
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

//---------------------------------------------------------------------
// General unmarshaling procedures included in virtual.cc
//---------------------------------------------------------------------

//
// 'dvs' may be zero - when the site has been recognized locally as
// dead and GC'ed after that;
#ifdef ROBUST_UNMARSHALER
void decomposeVSSiteDeadMsgRobust(VSMsgBuffer *mb, DSite* &ds, 
				  VirtualSite* &dvs, int *error)
#else
void decomposeVSSiteDeadMsg(VSMsgBuffer *mb, DSite* &ds, VirtualSite* &dvs)
#endif
{
  // Note that the 's' is not marked in the stream as 'PERM',
  // so we can get here both 'PERM' and alive sites;
#ifdef ROBUST_UNMARSHALER
  ds = unmarshalDSiteRobust(mb, error);
#else
  ds = unmarshalDSite(mb);
#endif
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

//
#ifdef ROBUST_UNMARSHALER
static Bool readVSMessagesRobust(unsigned long clock, void *vMBox, int *error)
#else
static Bool readVSMessages(unsigned long clock, void *vMBox)
#endif
{
  // unsafe by now - some magic number(s) should be added;
  VSMailboxOwned *mbox = (VSMailboxOwned *) vMBox;
  int msgs = MAX_MSGS_ATONCE;
#ifdef ROBUST_UNMARSHALER
  int e1, e2=NO;
#endif

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
      myVSMsgBufferImported = new (myVSMsgBufferImported)
	VSMsgBufferImported(importedVSChunksPoolManager,
			    msgChunkPoolKey, chunkNumber);
      //
      myVSMsgBufferImported->unmarshalBegin();

      //
      if (myVSMsgBufferImported->isVoid()) {
	// We cannot do much here, since it's not even known where the
	// message came from. However, we *must* guarantee that this
	// message loss is NOT due to resource problems; otherwise the
	// PERDIO layer will be confused;
	myVSMsgBufferImported->dropVoid();
	myVSMsgBufferImported->cleanup();
	// next message (if any could be processed??)
	continue;
      }
      //
      msgType = getVSMsgType(myVSMsgBufferImported);

      //
      // Take the site info out of the stream:
#ifdef ROBUST_UNMARSHALER
      vsIndex = (int) unmarshalNumberRobust(myVSMsgBufferImported, &e1);
#else
      vsIndex = (int) unmarshalNumber(myVSMsgBufferImported);
#endif
      if (vsIndex >= 0) {
	sVS = vsTable[vsIndex];
	sS = sVS->getSite();
	//
	myVSMsgBufferImported->setSite(sS);
	myVSMsgBufferImported->setKeysRegister(sVS->getKeysRegister());
	vsResourceManager.startMsgReceived(sVS);
      } else {
#ifdef ROBUST_UNMARSHALER
	sS = unmarshalDSiteRobust(myVSMsgBufferImported, &e1);
#else
	sS = unmarshalDSite(myVSMsgBufferImported);
#endif
	sVS = sS->getVirtualSite();
	//
	// Exactly in this order: first, complete the initializing the
	// message buffer, notify the resource manager that we are
	// unmarshalling a message, and then - compose the 'your index
	// here' message:
	myVSMsgBufferImported->setSite(sS);
	myVSMsgBufferImported->setKeysRegister(sVS->getKeysRegister());
	vsResourceManager.startMsgReceived(sVS);

	//
	// Now, assign the index and send it out:
	int vsIndex = vsTable.put(sVS);
	//
	VSMsgBufferOwned *bs = composeVSYourIndexHereMsg(sS, vsIndex);
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
	msgReceived(myVSMsgBufferImported);
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
	  decomposeVSSiteIsAliveMsg(myVSMsgBufferImported, myS);

	  //
	  if (myS == myDSite) {
	    VSMsgBufferOwned *bs = composeVSSiteAliveMsg(sS, sVS);
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
	    VSMsgBufferOwned *bs = composeVSSiteDeadMsg(sS, myS);
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
	  decomposeVSSiteAliveMsg(myVSMsgBufferImported, s, vs);
	  s->siteAlive();
	  break;
	}

      case VS_M_SITE_DEAD:
	{
	  DSite *ds;
	  VirtualSite *dvs;
	  decomposeVSSiteDeadMsg(myVSMsgBufferImported, ds, dvs);
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
	  decomposeVSYourIndexHereMsg(myVSMsgBufferImported, index);
	  sVS->setVSIndex(index);
	  break;
	}

      case VS_M_UNUSED_SHMID:
	{
	  DSite *s;
	  key_t shmid;
	  decomposeVSUnusedShmIdMsg(myVSMsgBufferImported, s, shmid);
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
      myVSMsgBufferImported->unmarshalEnd();
      myVSMsgBufferImported->releaseChunks();
      myVSMsgBufferImported->cleanup();

      //
      vsResourceManager.finishMsgReceived();
#ifdef ROBUST_UNMARSHALER
      *error = e1 || e2;
#endif
    } else {
      // is locked - then let's try to read later;
#ifdef ROBUST_UNMARSHALER
      *error = NO;
#endif
      return (FALSE);
    }
  }

  //
  return (TRUE);
}
