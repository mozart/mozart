/*
 *  Perdio Project, DFKI & SICS,
 *  Universit"at des Saarlandes
 *  Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
 *  SICS
 *  Box 1263, S-16428 Sweden, Phone (+46) 8 7521500
 *  Author: kost
 *  Last modified: $Date$ from $Author$
 *  Version: $Revision$
 *  State: $State$
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
// defined in vs_msgbuffer.cc;
key_t vsTypeToKey(int type);

//
VSMailboxManagerOwned::VSMailboxManagerOwned(key_t shmkeyIn)
{
  //
  shmkey = shmkeyIn;
  if ((int) (shmid = shmget(shmkey, /* size */ 0, S_IRWXU)) < 0)
    error("Virtual Sites: failed to get the shared memory page");
  if ((int) (mem = shmat(shmid, (char *) 0, 0)) < 0)
    error("Virtual Sites:: failed to attach the shared-memory page");

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
    error("Virtual Sites: failed to get the shared memory page");
  if ((int) (mem = shmat(shmid, (char *) 0, 0)) < 0)
    error("Virtual Sites:: failed to attach the shared-memory page");

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
  // add padding:
  restSize = (restSize / sizeof(VSMailboxMsg)) * sizeof(VSMailboxMsg);
  // one msg is allocated statically:
  int msgsNum = restSize / sizeof(VSMailboxMsg) + 1;
  // There must be place for at least one message;
  Assert(msgsNum);

  //
  shmkey = vsTypeToKey(VS_MAILBOX_KEY);
  if ((int) (shmid = shmget(shmkey, memSizeIn,
                            (IPC_CREAT | IPC_EXCL | S_IRWXU))) < 0)
    error("Virtual Sites: failed to allocate a shared memory page");
  if ((int) (mem = shmat(shmid, (char *) 0, 0)) < 0)
    error("Virtual Sites:: failed to attach a shared-memory page");

  //
  mbox = (VSMailboxCreated *) mem;
  mbox->init(memSize, msgsNum);
}

//
void VSMailboxManagerOwned::unmap()
{
  if (shmdt((char *) mem) < 0) {
    error("Virtual Sites: can't detach the shared memory.");
  }
  DebugCode(mbox = (VSMailboxOwned *) 0);
  DebugCode(mem = (void *) 0);
  DebugCode(memSize = -1);
}

//
void VSMailboxManagerImported::unmap()
{
  if (shmdt((char *) mem) < 0) {
    error("Virtual Sites: can't detach the shared memory.");
  }
  DebugCode(mbox = (VSMailboxImported *) 0);
  DebugCode(mem = (void *) 0);
  DebugCode(memSize = -1);
}

//
void VSMailboxManagerCreated::unmap()
{
  if (shmdt((char *) mem) < 0) {
    error("Virtual Sites: can't detach the shared memory.");
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
    if (errno != EIDRM)
      error("Virtual Sites: cannot mark the shared memory for destroying");
  }
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
    return;                     // already destroyed;
  (void) shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0);
}

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
  GenHashNode *ghn;

  ghn = GenHashTable::getFirst(seqIndex);
  GenCast(ghn, GenHashNode*, mbm, VSMailboxManagerImported*);
  return (mbm);
}

//
VSMailboxManagerImported*
VSMailboxRegister::getNext(VSMailboxManagerImported *prev)
{
  VSMailboxManagerImported *mbm;
  GenHashNode *ghn;

  GenCast(prev, VSMailboxManagerImported*, ghn, GenHashNode*);
  ghn = GenHashTable::getNext(ghn, seqIndex);
  GenCast(ghn, GenHashNode*, mbm, VSMailboxManagerImported*);
  return (mbm);
}


#endif // VIRTUALSITES
