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

#ifdef VIRTUALSITES

#include <sys/stat.h>

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
static unsigned int seq_num = 0;
//
// compose the 'semkey' :  type id, sequential number, and pid; 
key_t vsTypeToKey(int type)
{
  // type is allowed to be 4 bits;
  Assert(!(type & ~0xf));
  Assert(VS_KEYTYPE_SIZE + VS_SEQ_SIZE + VS_PID_SIZE == 32);
  Assert(sizeof(key_t) >= (int) sizeof(int32));
  Assert(seq_num < (int) (1 << VS_SEQ_SIZE));

  //
  int idOffset = sizeof(key_t)*8 - VS_KEYTYPE_SIZE;
  int seqOffset = idOffset - VS_SEQ_SIZE;

  //
  return ((key_t) ((type << idOffset) |
		   (seq_num++ << seqOffset) |
		   (osgetpid() & VS_PID_MASK)));
}

//
void VSMsgChunkPoolOwned::init(key_t shmkeyIn,
			       int chunkSizeIn, int chunksNumIn)
{
  shmkey = shmkeyIn;
  chunkSize = chunkSizeIn;
  chunksNum = chunksNumIn;
  wakeupMode = FALSE;
  Assert(sizeof(this) <= (unsigned int) chunkSizeIn);
  initReservedChunk();
  semkey = vsTypeToKey(VS_WAKEUPSEM_KEY);
}

//
VSMsgChunkPoolManagerOwned::VSMsgChunkPoolManagerOwned(int chunkSizeIn,
						       int chunksNumIn)
{
  chunkSize = chunkSizeIn;
  chunksNum = chunksNumIn;
  SemOptArg arg;

  // 
  // Get&attach a shared memory page;
  shmkey = vsTypeToKey(VS_MSGBUFFER_KEY);
  if ((int) (shmid = shmget(shmkey, chunkSizeIn*chunksNumIn,
			    (IPC_CREAT | IPC_EXCL | S_IRWXU))) < 0) 
    error("Virtual Sites: failed to allocate a shared memory page");
  if ((int) (mem = shmat(shmid, (char *) 0, 0)) < 0)
    error("Virtual Sites:: failed to attach a shared-memory page");

  //
  // We cannot mark it for destruction right here, since then it
  // cannot be accessed by any other process ("key already
  // destroyed");
  // markDestroy();

  // 
  // The first chunk is used for the "pool" object, and the rest - 
  // for "VSMsgChunk" objects;
  pool = (VSMsgChunkPoolOwned *) mem;
  pool->init(shmkey, chunkSizeIn, chunksNumIn);
  chunks = (VSMsgChunkOwned *) mem;
  // Chunks are initialized lazily - prior usage;

  //
  // A semaphore used in the "wakeup" mode;
  semkey = pool->getSemkey();
  if ((semid = semget(semkey, 1, (IPC_CREAT | IPC_EXCL | S_IRWXU))) < 0)
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
  map[0] &= ~0x1;		// ... but except the one for the pool;
}

//
VSMsgChunkPoolManagerImported::VSMsgChunkPoolManagerImported(key_t shmkeyIn)
{
  shmkey = shmkeyIn;
  //
  // we don't know in advance how large it is - so just try to swallow
  // the page "as is";
  if ((int) (shmid = shmget(shmkey, /* size */ 0, S_IRWXU)) < 0)
    error("Virtual Sites: failed to get a shared memory page");
  if ((int) (mem = shmat(shmid, (char *) 0, 0)) < 0) 
    error("Virtual Sites:: failed to attach a shared-memory page");

  // locations;
  pool = (VSMsgChunkPoolImported *) mem; // initialized by the owner;
  pool->init(shmkeyIn);		 // actually empty;
  chunks = ((VSMsgChunkImported *) mem);

  // Borrow data from the 'VSMsgChunkPool' object
  // This information is necessary for debugging only (like checking 
  // whether a chunk really belongs to the pool);
  chunkSize = pool->getChunkSize();
  chunksNum = pool->getChunksNum();

  //
  // A semaphore used in the "wakeup" mode;
  semkey = pool->getSemkey();
  if ((semid = semget(semkey, 1, S_IRWXU)) < 0)
    error("Virtual Sites: failed to find the semphore");
}

//
void VSMsgChunkPoolManagerOwned::markDestroy()
{
  DebugCode(pool->checkConsistency());
  //
  if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
    if (errno != EIDRM) 
      error("Virtual Sites: cannot remove the shared memory");
  }

  //
  SemOptArg arg;
  arg.val = 0;			// junk;
  if (semctl(semid, 0, IPC_RMID, arg) < 0) {
    if (errno != EIDRM) 
      error("Virtual Sites: cannot remove the semaphore");
  }
}

//
VSMsgChunkPoolManagerOwned::~VSMsgChunkPoolManagerOwned()
{
  DebugCode(pool->checkConsistency());
  //
  markDestroy();
  //
  if (shmdt((char *) mem) < 0) {
    error("Virtual Sites: can't detach the shared memory.");
  }

  //
  delete map;
  //
  DebugCode(chunkSize = chunksNum = 0);
  DebugCode(shmid = 0);
  DebugCode(shmkey = (key_t) 0);
  DebugCode(semid = 0);
  DebugCode(semkey = (key_t) 0);
  DebugCode(mem = (void *) 0);
  DebugCode(chunks = (VSMsgChunkOwned *) 0);
  DebugCode(pool = (VSMsgChunkPoolOwned *) 0);
  DebugCode(mapSize = 0);
  DebugCode(map = (int32 *) 0);
}

//
VSMsgChunkPoolManagerImported::~VSMsgChunkPoolManagerImported()
{
  DebugCode(pool->checkConsistency());
  //
  if (shmdt((char *) mem) < 0) {
    error("Virtual Sites: can't detach the shared memory.");
  }

  //
  DebugCode(chunkSize = chunksNum = 0);
  DebugCode(shmid = 0);
  DebugCode(shmkey = (key_t) 0);
  DebugCode(semid = 0);
  DebugCode(semkey = (key_t) 0);
  DebugCode(mem = (void *) 0);
  DebugCode(chunks = (VSMsgChunkImported *) 0);
  DebugCode(pool = (VSMsgChunkPoolImported *) 0);
}

//
void VSMsgChunkPoolManagerOwned::scavengeMemory()
{
  DebugCode(pool->checkConsistency());
  for (int i = 1; i < chunksNum; i++) // the first one is busy anyway;
    if (!(getChunkAddr(i)->isBusy()))
      markFreeInMap(i);
}

//
int VSMsgChunkPoolManagerOwned::getMsgChunkOutline()
{
  DebugCode(pool->checkConsistency());
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
  if (chunkNum < 0) {		// still none - then block really;
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
  pool->stopWakeupMode();	// don't care about the semaphore now;
  Assert(chunkNum != 0);	// zeroth is always busy;
  return (chunkNum);
}

//
void VSMsgChunkPoolManagerImported::wakeupOwner()
{
  DebugCode(pool->checkConsistency());
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
VSMsgChunkPoolManagerImported* VSChunkPoolRegister::find(key_t key)
{
  int hvalue = hash(key);
  GenHashNode *aux = htFindFirst(hvalue);
  while(aux) {
    key_t auxKey;
    GenCast(aux->getBaseKey(), GenHashBaseKey*, auxKey, key_t);

    //
    if (key == auxKey) {
      VSMsgChunkPoolManagerImported *msgChunkPoolManager;
      GenCast(aux->getEntry(), GenHashEntry*,
	      msgChunkPoolManager, VSMsgChunkPoolManagerImported*);
      return (msgChunkPoolManager);
    }

    //
    aux = htFindNext(aux, hvalue);
  }
  return ((VSMsgChunkPoolManagerImported *) 0);
}

//
void VSChunkPoolRegister::add(key_t key, VSMsgChunkPoolManagerImported *pool)
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
void VSChunkPoolRegister::remove(key_t key)
{
  int hvalue = hash(key);
  GenHashNode *aux = htFindFirst(hvalue);
  while(aux) {
    key_t auxKey;
    GenCast(aux->getBaseKey(), GenHashBaseKey*, auxKey, key_t);

    //
    if (key == auxKey) {
      htSub(hvalue,aux);
      break;
    }

    //
    aux = htFindNext(aux, hvalue);
  }
}

//
char* VSMsgBufferOwned::siteStringrep()
{
  if (site != (Site *) -1) 
    return (site->stringrep());
  else
    return ("(initializing a new virtual site - it is not yet known)");
}

//
char* VSMsgBufferImported::siteStringrep()
{
  return ("(an unknown virtual site a message received from)");
}

#endif // VIRTUALSITES
