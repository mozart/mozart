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
#pragma implementation "vs_msgbuffer.hh"
#endif

#include "vs_msgbuffer.hh"

//
// ('SEM_UNDO' are not usable here, since the semaphore does not really
// count resources;)
static struct sembuf block[1] = {
  { 0, -1, 0 }
};
static struct sembuf wakeup[1] = {
  { 0, 1, 0 }
};

//
// compose the 'semkey' : the 'PERDIO' byte, type id and pid beneath;
key_t vsTypeToKey(int type)
{
  // type is allowed to be 4 bits;
  Assert(!(type & ~0xf));
  int key_tSize = sizeof(key_t); // in bytes (chars);
  int idOffset = (key_tSize - 1) * 8; // in bits;
  int typeOffset = idOffset - VS_KEYTYPE_SIZE;
  int seqMask = 2^typeOffset - 1;

  //
  return((key_t) ((PERDIO_ID << idOffset) |
                  (type << typeOffset) |
                  (((int) getpid()) & seqMask)));
}

//
VSMsgChunkPool::VSMsgChunkPool(int chunkSizeIn, int chunksNumIn)
  : chunkSize(chunkSizeIn), chunksNum(chunksNumIn), wakeupMode(FALSE)
{
  Assert(sizeof(this) <= (unsigned int) chunkSizeIn);
  semkey = vsTypeToKey(VS_WAKEUPSEM_KEY);
}

//
VSMsgChunkPoolManager::VSMsgChunkPoolManager(int chunkSizeIn,
                                             int chunksNumIn)
  : chunkSize(chunkSizeIn), chunksNum(chunksNumIn)
{
  SemOptArg arg;

  //
  // Get&attach a shared memory page;
  shmkey = vsTypeToKey(VS_MSGBUFFER_KEY);
  if ((shmid = shmget(shmkey, chunkSizeIn*chunksNumIn, IPC_CREAT)) < 0)
    error("Virtual Sites: failed to allocate a shared memory page");
  if ((mem = shmat(shmkey, (char *) 0, 0)) < 0)
    error("Virtual Sites:: failed to attach a shared-memory page");

  //
  // The first chunk is used for the "pool" object, and the rest -
  // for "VSMsgChunk" objects;
  pool = (VSMsgChunkPool *) mem;
  pool->VSMsgChunkPool::VSMsgChunkPool(chunkSizeIn, chunksNumIn);
  chunks = (VSMsgChunk *) mem;
  for (int i = 1; i < chunksNum; i++) // don't try to init the first one;
    chunks[i].VSMsgChunk::VSMsgChunk();

  //
  // A semaphore used in the "wakeup" mode;
  semkey = pool->getSemkey();
  if ((semid = semget(semkey, 1, IPC_CREAT)) < 0)
    error("Virtual Sites: failed to allocate a semphore");
  arg.val = 0;
  if (semctl(semid, 0, SETVAL, arg) < 0)
    error("Virtual Sites: Failed to initialize the semaphore");

  //
  // A usage bitmap;
  Assert(!(chunksNum%32));
  mapSize = chunksNum/32;
  map = (int32 *) malloc(mapSize*4); // in bytes;
  for (int i = 0; i < mapSize; i++)
    map[i] = (int32) 0xffffffff;     // everything is free originally;
  map[0] &= ~0x1;               // ... but except the one for the pool;
}

//
VSMsgChunkPoolManager::VSMsgChunkPoolManager(key_t shmkeyIn)
  : shmkey(shmkeyIn)
{
  //
  // we don't know in advance how large it is - so just try to swallow
  // the page "as is";
  if ((shmid = shmget(shmkey, /* size */ 0, /* args */ 0)) < 0)
    error("Virtual Sites: failed to get a shared memory page");
  if ((mem = shmat(shmkey, (char *) 0, 0)) < 0)
    error("Virtual Sites:: failed to attach a shared-memory page");

  // locations;
  pool = (VSMsgChunkPool *) mem; // initialized by the owner;
  pool->VSMsgChunkPool::VSMsgChunkPool();       // actually empty;
  chunks = ((VSMsgChunk *) mem);

  // Borrow data from the 'VSMsgChunkPool' object
  // This information is necessary for debugging only (like checking
  // whether a chunk really belongs to the pool);
  chunkSize = pool->getChunkSize();
  chunksNum = pool->getchunksNum();

  //
  // A semaphore used in the "wakeup" mode;
  semkey = pool->getSemkey();
  if ((semid = semget(semkey, 1, 0)) < 0)
    error("Virtual Sites: failed to find the semphore");

  //
  // (no usage bitmap at receiver sites;)
  mapSize = 0;
  map = (int32 *) 0;
}

//
void VSMsgChunkPoolManager::destroy()
{
  if (shmdt((char *) mem) < 0) {
    error("Virtual Sites: can't detach the shared memory.");
  }
  DebugCode(mem = (void *) 0);
  DebugCode(pool = (VSMsgChunkPool *) 0);
  DebugCode(chunks = (VSMsgChunk *) 0);
  DebugCode(chunkSize = chunksNum = -1);

  //
  if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
    error("Virtual Sites: cannot remove the shared memory");
  }
  DebugCode(shmid = 0);
  DebugCode(shmkey = (key_t) 0);
}

//
void VSMsgChunkPoolManager::scavengeMemory()
{
  for (int i = 1; i < chunksNum; i++) // the first one is busy anyway;
    if (chunks[i].isBusy())
      markFreeInMap(i);
}

//
int VSMsgChunkPoolManager::getMsgChunkOutline()
{
  int chunkNum;
  SemOptArg arg;

  //
  // First, just scavenge the memory:
  scavengeMemory();
  chunkNum = allocateFromMap();
  //
  if (chunkNum >= 0) return (chunkNum);

  //
  // If it has not helped, then wait using the semaphore;
retry:
  pool->startWakeupMode();

  //
  arg.val = 0;
  if (semctl(semid, 0, SETVAL, arg) < 0)
    error("Virtual Sites: unable to setup the semaphore");

  //
  scavengeMemory();
  chunkNum = allocateFromMap();
  //
  if (chunkNum < 0) {           // still none - then block really;
    while (semop(semid, &block[0], 1) < 0)
      if (errno == EINTR) continue;
      else error("Virtual Sites: unable to block on the semaphore");

    //
    // Resumed by somebody else - let's check it again;
    scavengeMemory();
    chunkNum = allocateFromMap();
    if (chunkNum < 0) goto retry; // otherwise fall through;
  }

  //
  pool->stopWakeupMode();       // don't care about the semaphore now;
  return (chunkNum);
}

//
void VSMsgChunkPoolManager::wakeupOwner()
{
  // Note that the "wakeup" mode can be dropped here already (though
  // that's tested in the 'markFree' method), but we don't care!
  while (semop(semid, &wakeup[0], 1) < 0)
    if (errno == EINTR) continue;
    else error("Virtual Sites: unable to wakeup a sender");
}

//
// Hash the key like a 4-character sequence (Aho, Ullman etc.)
unsigned int VSChunkPoolRegister::hash(key_t key)
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
VSMsgChunkPoolManager* VSChunkPoolRegister::find(key_t key)
{
  int hvalue = hash(key);
  GenHashNode *aux = htFindFirst(hvalue);
  while(aux) {
    key_t auxKey;
    GenCast(aux->getBaseKey(), GenHashBaseKey*, auxKey, key_t);

    //
    if (key == auxKey) {
      VSMsgChunkPoolManager *msgChunkPoolManager;
      GenCast(aux->getEntry(), GenHashEntry*,
              msgChunkPoolManager, VSMsgChunkPoolManager*);
      return (msgChunkPoolManager);
    }

    //
    aux = htFindNext(aux, hvalue);
  }
  return ((VSMsgChunkPoolManager *) 0);
}

//
void VSChunkPoolRegister::add(key_t key, VSMsgChunkPoolManager *pool)
{
  GenHashBaseKey* ghn_bk;
  GenHashEntry* ghn_e;
  int hvalue = hash(key);

  //
  GenCast(key, key_t, ghn_bk, GenHashBaseKey*);
  GenCast(pool, VSMsgChunkPoolManager*, ghn_e, GenHashEntry*);
  htAdd(hvalue, ghn_bk, ghn_e);
}

//
// Sender site:
VSMsgBuffer::VSMsgBuffer(VSMsgChunkPoolManager *cpmIn, Site *siteIn)
  : cpm(cpmIn), site(siteIn), first(-1)
{
  DebugCode(current = -1);
  DebugCode(currentAddr = (VSMsgChunk *) 0);
  DebugCode(ptr = end = (BYTE *) 0);
}

//
// Receiver site:
VSMsgBuffer::VSMsgBuffer(VSMsgChunkPoolManager *cpmIn, int chunkIndex)
  : cpm(cpmIn), current(chunkIndex), first(chunkIndex)
{
  currentAddr = cpm->getChunkAddr(current);
  ptr = &(currentAddr->buffer[0]);
  end = ptr + cpm->getChunkDataSize(); // in bytes;
}

//
void VSMsgBuffer::marshalBegin()
{
  Assert(first < 0);
  // allocate the first chunk unconditionally
  // (there are no empty messages?)
  current = first = cpm->getMsgChunk();
  currentAddr = cpm->getChunkAddr(current);
  //
  ptr = &(currentAddr->buffer[0]);
  end = ptr + cpm->getChunkDataSize(); // in bytes;
}

//
void VSMsgBuffer::unmarshalEnd()
{
  while (first >= 0) {
    VSMsgChunk *chunkAddr = cpm->getChunkAddr(first);
    first = chunkAddr->next;
    cpm->releaseChunk(chunkAddr);
  }
  DebugCode(current = -1);
  DebugCode(currentAddr = (VSMsgChunk *) 0);
  DebugCode(ptr = end = (BYTE *) 0);
}
