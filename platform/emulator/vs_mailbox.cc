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

//
// defined in vs_msgbuffer.cc;
key_t vsTypeToKey(int type);

//
VSMailboxManager::VSMailboxManager(long memSizeIn)
  : memSize(memSizeIn)
{
  int msgsNum = (memSize - sizeof(VSMailbox))/sizeof(VSMailboxMsg);
  // There must be a place for at least one message;
  Assert(msgsNum);

  //
  shmkey = vsTypeToKey(VS_MAILBOX_KEY);
  if ((shmid = shmget(shmkey, memSizeIn, IPC_CREAT)) < 0)
    error("Virtual Sites: failed to allocate a shared memory page");
  if ((mem = shmat(shmkey, (void *) 0, 0)) < 0)
    error("Virtual Sites:: failed to attach a shared-memory page");

  //
  mbox = (VSMailbox *) mem;
  mbox->VSMailbox::VSMailbox(shmkey, memSize, msgsNum);
  // Note that the mailbox is not yet ready - 'virtual info' is missing;
}

//
VSMailboxManager::VSMailboxManager(key_t shmkeyIn)
  : shmkey(shmkeyIn)
{
  //
  if ((shmid = shmget(shmkey, /* size */ 0, /* flags */0)) < 0)
    error("Virtual Sites: failed to get the shared memory page");
  if ((mem = shmat(shmkey, (void *) 0, 0)) < 0)
    error("Virtual Sites:: failed to attach the shared-memory page");

  //
  mbox = (VSMailbox *) mem;
  mbox->VSMailbox::VSMailbox();
  //
  memSize = mbox->getMemSize();
}

//
void VSMailboxManager::unmap()
{
  if (shmdt(mem) < 0) {
    error("Virtual Sites: can't detach the shared memory.");
  }
  DebugCode(mbox = (VSMailbox *) 0);
  DebugCode(mem = (void *) 0);
}

//
void VSMailboxManager::destroy()
{
  unmap();

  //
  if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
    error("Virtual Sites: cannot remove the shared memory");
  }
  DebugCode(shmid = 0);
  DebugCode(shmkey = (key_t) 0);
  DebugCode(memSize = -1);
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
VSMailboxManager* VSMailboxRegister::find(key_t key)
{
  int hvalue = hash(key);
  GenHashNode *aux = htFindFirst(hvalue);
  while(aux) {
    key_t auxKey;
    GenCast(aux->getBaseKey(), GenHashBaseKey*, auxKey, key_t);

    //
    if (key == auxKey) {
      VSMailboxManager *mailboxManager;
      GenCast(aux->getEntry(), GenHashEntry*,
              mailboxManager, VSMailboxManager*);
      return (mailboxManager);
    }

    //
    aux = htFindNext(aux, hvalue);
  }

  //
  return ((VSMailboxManager *) 0);
}

//
void VSMailboxRegister::add(key_t key, VSMailboxManager *pool)
{
  GenHashBaseKey* ghn_bk;
  GenHashEntry* ghn_e;
  int hvalue = hash(key);

  //
  GenCast(key, key_t, ghn_bk, GenHashBaseKey*);
  GenCast(pool, VSMailboxManager*, ghn_e, GenHashEntry*);
  //
  htAdd(hvalue, ghn_bk, ghn_e);
}
