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

#if defined(INTERFACE)
#pragma implementation "vs_comm.hh"
#endif
#if defined(INTERFACE)
#pragma implementation "vs_lock.hh"
#endif

#include "vs_comm.hh"

//
void VirtualSite::connect()
{
  //
  if (mboxMgr == (VSMailboxManager *) 0) {
    if (isSetVSFlag(VS_PENDING_UNMAP_MBOX)) {
      clearVSFlag(VS_PENDING_UNMAP_MBOX);
    } else {
      //
      // Creation of a mailbox manager includes mapping the mailbox
      // into our address space (that is, a "write" connection is
      // established;
      key_t mboxKey = site->getVirtualInfo()->getMailboxKey();
      mboxMgr = new VSMailboxManager(mboxKey);
    }
  }
}

//
//
VirtualSite::VirtualSite(Site *s,
                         VSFreeMessagePool *fmpIn,
                         VSSiteQueue *sqIn)
  : site(s), status(SITE_OK), vsStatus(0), fmp(fmpIn), sq(sqIn),
    mboxMgr((VSMailboxManager *) 0)
{
  connect();
}

//
//
void VirtualSite::disconnect()
{
  if (isNotEmpty()) {
    setVSFlag(VS_PENDING_UNMAP_MBOX);
  } else {
    mboxMgr->unmap();
    delete mboxMgr;
    mboxMgr = (VSMailboxManager *) 0;
  }
}

//
// Droping a virtual site occurs when it is known (e.g. from a third
// party) to be dead;
void VirtualSite::drop()
{
  //
  // Throw away all the undeliverable now messages (if any);
  while (isNotEmpty()) {
    VSMessage *vsm = dequeue();
    site->communicationProblem(vsm->getMessageType(),
                               vsm->getSite(), vsm->getStoreIndex(),
                               COMM_FAULT_PERM_NOT_SENT,
                               (FaultInfo) vsm->getMsgBuffer());
    fmp->dispose(vsm);
  }

  //
  // should result in unmapping:
  disconnect();
  Assert(mboxMgr == (VSMailboxManager *) 0);

  //
  // 'SITE_PERM' is redundant - used for consistency checks only;
  status = SITE_PERM;
  vsStatus = 0;
}

//
// The message type, store site and store index parameters
// are opaque data (just stored);
int VirtualSite::sendTo(VSMsgBuffer *mb, MessageType mt,
                        Site *storeSite, int storeIndex)
{
  //
  // 'mb' must be indeed an object of the 'VSMsgBuffer' type;
  Assert(mb->getSite()->virtualComm());

  //
  //
  switch (getSiteStatus()) {
  case SITE_TEMP: error("A virtual site temporarily down???");
  case SITE_PERM: error("Attempt to send to a 'perm' virtual site!");
  case SITE_OK:
    //
    // First, let's try to deliver it *now*.
    // If it fails, a message (job) for delayed delivery is created;
    Assert(mboxMgr);            // must be already connected;
    VSMailbox *mbox = mboxMgr->getMailbox();
    VirtualInfo *myVI = mySite->getVirtualInfo();

    //
    if (!mbox->enqueue(mb->getSHMKey(), mb->getFirstChunk())) {
      // Failed to enqueue it inline - then create a job which will
      // try to do that later;
      VSMessage *m = fmp->allocate();
      m->VSMessage::VSMessage(mb, mt, storeSite, storeIndex);

      //
      // These queues are checked and processed on regular intervals
      // (OS' alarms);
      enqueue(m);
      sq->enqueue(this);

      // Note that there is no 'TEMP_NOT_SENT' now;
    }

    //
    return (ACCEPTED);
  }
  return(ACCEPTED);     // to make gcc happy;
}

//
// ... retry to send it with (it takes un unsent message, compared to
// 'sendTo()');
int VirtualSite::tryToSendToAgain(VSMessage *vsm)
{
  //
  //
  switch (getSiteStatus()) {
  case SITE_TEMP:
    error("A virtual site temporarily down???");

  case SITE_PERM:
    fmp->dispose(vsm);
    return (PERM_NOT_SENT);

  case SITE_OK:
    //
    // First, let's try to deliver it *now*.
    // If it fails, a message (job) for delayed delivery is created;
    VSMsgBuffer *mb = vsm->getMsgBuffer();
    VSMailbox *mbox = mboxMgr->getMailbox();
    VirtualInfo *myVI = mySite->getVirtualInfo();

    //
    if (mbox->enqueue(mb->getSHMKey(), mb->getFirstChunk())) {
      fmp->dispose(vsm);
    } else {
      // Failed to enqueue it inline - then queue up the message again;
      enqueue(vsm);
      sq->enqueue(this);
    }

    //
    return (ACCEPTED);
  }
  return(ACCEPTED);     // to make gcc happy;
}

//
// Throw away the "virtual info" object representation from a message
// buffer;
void unmarshalUselessVirtualInfo(MsgBuffer *mb)
{
  int size =
    sizeof(unsigned int) + sizeof(unsigned short) +
    sizeof(unsigned int) + sizeof(unsigned int) + sizeof(unsigned int);
  for (; size > 0; size--)
    (void) mb->get();
}
