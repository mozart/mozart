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
#pragma implementation "vs_mailbox.hh"
#endif

#include "vs_mailbox.hh"

#ifdef VIRTUALSITES

#include <errno.h>
#include <sys/stat.h>

//
// #define TRACE_MAILBOXES

//
// defined in vs_msgbuffer.cc;
key_t vsTypeToKey(int type);

//
VSMailboxManagerOwned::VSMailboxManagerOwned(key_t shmkeyIn)
{
  //
  shmkey = shmkeyIn;
  if ((int) (shmid = shmget(shmkey, /* size */ 0, S_IRWXU)) < 0)
    OZ_error("Virtual Sites: failed to get the shared memory page");
  if ((int) (mem = shmat(shmid, (char *) 0, 0)) == -1) 
    OZ_error("Virtual Sites:: failed to attach the shared-memory page");
#ifdef TRACE_MAILBOXES
  fprintf(stdout, "*** mailbox obtained 0x%X (pid %d)\n",
	  shmkey, getpid());
  fflush(stdout);
#endif

  //
  // We cannot mark it for destruction right here, since then it
  // cannot be accessed by any other process:
  // markDestroy();

  //
  mbox = (VSMailboxOwned *) mem;
  mbox->init(osgetpid());
  //
  memSize = mbox->getMemSize();
}

//
VSMailboxManagerImported::VSMailboxManagerImported(key_t shmkeyIn)
{
  //
  shmkey = shmkeyIn;
  if ((int) (shmid = shmget(shmkey, /* size */ 0, S_IRWXU)) < 0)
    OZ_error("Virtual Sites: failed to get the shared memory page");
  if ((int) (mem = shmat(shmid, (char *) 0, 0)) == -1) 
    OZ_error("Virtual Sites:: failed to attach the shared-memory page");
#ifdef TRACE_MAILBOXES
  fprintf(stdout, "*** mailbox attached 0x%X (pid %d)\n",
	  shmkey, getpid());
  fflush(stdout);
#endif

  //
  mbox = (VSMailboxImported *) mem;
  mbox->init();
  //
  memSize = mbox->getMemSize();
}

//
VSMailboxManagerCreated::VSMailboxManagerCreated(int memSizeIn)
{
  memSize = memSizeIn;
  int restSize = memSize - sizeof(VSMailbox);
  //
#ifdef DEBUG_CHECK
  // one msg is allocated statically (see vs_mailbox.hh):
  int msgsNum = restSize / sizeof(VSMailboxMsg) + 1;
#else
  int msgsNum = restSize / sizeof(VSMailboxMsg);
#endif
  // There must be place for at least one message;
  Assert(msgsNum);

  //
  shmkey = vsTypeToKey(VS_MAILBOX_KEY);
  if ((int) (shmid = shmget(shmkey, memSizeIn, 
			    (IPC_CREAT | IPC_EXCL | S_IRWXU))) < 0)
    OZ_error("Virtual Sites: failed to allocate a shared memory page");
  if ((int) (mem = shmat(shmid, (char *) 0, 0)) == -1) 
    OZ_error("Virtual Sites:: failed to attach a shared-memory page");

  //
  mbox = (VSMailboxCreated *) mem;
  mbox->init(memSize, msgsNum); 
}

//
void VSMailboxManagerOwned::unmap()
{
  if (shmdt((char *) mem) < 0) {
    OZ_error("Virtual Sites: can't detach the shared memory.");
  }
#ifdef TRACE_MAILBOXES
  fprintf(stdout, "*** unmap owned mailbox 0x%X (pid %d)\n",
	  shmkey, getpid());
  fflush(stdout);
#endif
  DebugCode(mbox = (VSMailboxOwned *) 0);
  DebugCode(mem = (void *) 0);
  DebugCode(memSize = -1);
}

//
void VSMailboxManagerImported::unmap()
{
  if (shmdt((char *) mem) < 0) {
    OZ_error("Virtual Sites: can't detach the shared memory.");
  }
#ifdef TRACE_MAILBOXES
  fprintf(stdout, "*** unmap imported mailbox 0x%X (pid %d)\n",
	  shmkey, getpid());
  fflush(stdout);
#endif
  DebugCode(mbox = (VSMailboxImported *) 0);
  DebugCode(mem = (void *) 0);
  DebugCode(memSize = -1);
}

//
void VSMailboxManagerCreated::unmap()
{
  if (shmdt((char *) mem) < 0) {
    OZ_error("Virtual Sites: can't detach the shared memory.");
  }
  DebugCode(mbox = (VSMailboxCreated *) 0);
  DebugCode(mem = (void *) 0);
  DebugCode(memSize = -1);
}

//
void VSMailboxManagerOwned::markDestroy()
{
  //
  if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
    // If an already removed id is removed (tried to) again,
    // e.g. linux says 'EIDRM' while Solaris 'EINVAL';
    if (errno != EIDRM && errno != EINVAL) 
      OZ_error("Virtual Sites: cannot mark the shared memory for destroying");
  }
#ifdef TRACE_MAILBOXES
  fprintf(stdout, "*** mailbox removed 0x%X (pid %d)\n",
	  shmkey, getpid());
  fflush(stdout);
#endif
}

//
void VSMailboxManagerOwned::destroy()
{
  markDestroy();
  unmap();
  DebugCode(shmid = 0);
  DebugCode(shmkey = (key_t) 0);
}

//
//
void markDestroy(key_t shmkey)
{
  //
  int shmid;

  //
  if ((int) (shmid = shmget(shmkey, /* size */ 0, S_IRWXU)) < 0)
    return;			// already destroyed;
  (void) shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0);
#ifdef TRACE_MAILBOXES
  fprintf(stdout, "*** mailbox killed 0x%X (pid %d)\n",
	  shmkey, getpid());
  fflush(stdout);
#endif
}

//
#ifdef VS_NEEDS_MAILBOXREGISTER
//
// By now that's the same code as for the 'VSChunkPoolRegister' one;
unsigned int VSMailboxRegister::hash(key_t key)
{
  unsigned char *p = (unsigned char *) &key;
  unsigned int h = 0, g;

  //
  for(int i = 0; i < (int) sizeof(key); i++,p++) {
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }
  return (h);
}

//
VSMailboxManagerImported* VSMailboxRegister::find(key_t key)
{
  int hvalue = hash(key);
  GenHashNode *aux = htFindFirst(hvalue);
  while(aux) {
    key_t auxKey;
    GenCast(aux->getBaseKey(), GenHashBaseKey*, auxKey, key_t);

    //
    if (key == auxKey) {
      VSMailboxManagerImported *mailboxManager;
      GenCast(aux->getEntry(), GenHashEntry*,
	      mailboxManager, VSMailboxManagerImported*);
      return (mailboxManager);
    }

    //
    aux = htFindNext(aux, hvalue);
  }

  //
  return ((VSMailboxManagerImported *) 0);
}

//
void VSMailboxRegister::add(key_t key, VSMailboxManagerImported *pool)
{
  GenHashBaseKey* ghn_bk;
  GenHashEntry* ghn_e;
  int hvalue = hash(key);

  //
  GenCast(key, key_t, ghn_bk, GenHashBaseKey*);
  GenCast(pool, VSMailboxManagerImported*, ghn_e, GenHashEntry*);
  //
  htAdd(hvalue, ghn_bk, ghn_e);
}

//
VSMailboxManagerImported *VSMailboxRegister::getFirst()
{
  VSMailboxManagerImported *mbm;
  seqGHN = GenHashTable::getFirst(seqIndex);
  if (seqGHN) {
    GenCast(seqGHN->getEntry(), GenHashEntry*,
	    mbm, VSMailboxManagerImported*);
  } else {
    mbm = (VSMailboxManagerImported *) 0;
  }
  return (mbm);
}

//
VSMailboxManagerImported* VSMailboxRegister::getNext()
{
  VSMailboxManagerImported *mbm;
  seqGHN = GenHashTable::getNext(seqGHN, seqIndex);
  if (seqGHN) {
    GenCast(seqGHN->getEntry(), GenHashEntry*,
	    mbm, VSMailboxManagerImported*);
  } else {
    mbm = (VSMailboxManagerImported *) 0;
  }
  return (mbm);
}
#endif // VS_NEEDS_MAILBOXREGISTER

#endif // VIRTUALSITES
