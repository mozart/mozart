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
#pragma implementation "vs_aux.hh" // for the debug mode;
#endif

#include "base.hh"
#include "dpBase.hh"

#ifdef VIRTUALSITES

#include "am.hh"
#include "os.hh"

#include "vs_msgbuffer.hh"
#include "virtual.hh"
#include "vs_comm.hh"

#include <sys/stat.h>

//
// #define TRACE_SEGMENTS

//
static unsigned int seq_num = 0;
//
// compose an "ipc key" :  type id, sequential number, and pid; 
//
// NOTE: The naming scheme is not 100% safe: two logically distinct
// keys can match because pid# are not sufficient to identify an entry
// with larger extent that the process itself (the problem is similar
// to the one in distributed Oz site naming scheme);
//
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
// For a given key says whether it has been produced by the (local)
// process.
//
Bool isLocalKey(key_t key)
{
  return (((key & VS_PID_MASK) == osgetpid()) ? TRUE : FALSE);
}

//
void VSMsgChunkPoolSegmentOwned::init(key_t shmkeyIn,
				      int chunkSizeIn, int chunksNumIn)
{
  shmkey = shmkeyIn;
  chunkSize = chunkSizeIn;
  chunksNum = chunksNumIn;
  Assert(sizeof(VSMsgChunkPoolSegmentOwned) <= (unsigned int) chunkSizeIn);
  initReservedChunk();
}

//
VSMsgChunkPoolSegmentManagerOwned::
VSMsgChunkPoolSegmentManagerOwned(int chunkSizeIn, int chunksNumIn,
				  int regInitSize)
  : fs(chunksNumIn), phaseNumber(0), importersRegister(regInitSize)
{
  chunkSize = chunkSizeIn;
  chunksNum = chunksNumIn;

  // 
  // Get&attach a shared memory page;
  shmkey = vsTypeToKey(VS_MSGBUFFER_KEY);
  if ((int) (shmid = shmget(shmkey, chunkSizeIn*chunksNumIn,
			    (IPC_CREAT | IPC_EXCL | S_IRWXU))) < 0) 
    OZ_error("Virtual Sites: failed to allocate a shared memory page");
  if ((int) (mem = shmat(shmid, (char *) 0, 0)) == -1)
    OZ_error("Virtual Sites:: failed to attach a shared-memory page");
  //
#ifdef TRACE_SEGMENTS
  fprintf(stdout, "*** segment created 0x%X (pid %d)\n",
	  shmkey, getpid());
  fflush(stdout);
#endif

  //
  // We cannot mark it for destruction right here, since then it
  // cannot be accessed by any other process ("key already
  // destroyed");
  // markDestroy();

  // 
  // The first chunk is used for the "pool" object, and the rest - 
  // for "VSMsgChunk" objects;
  pool = (VSMsgChunkPoolSegmentOwned *) mem;
  pool->init(shmkey, chunkSizeIn, chunksNumIn);
  chunks = (VSMsgChunkOwned *) mem;
  // Chunks are initialized lazily - prior usage;

  //
  for (int i = 1; i < chunksNum; i++) {
    DebugCode(getChunkAddr(i)->freeDebug(chunkSize););
    fs.push(i);
  }
}

//
void VSMsgChunkPoolSegmentManagerOwned::markDestroy()
{
  DebugCode(pool->checkConsistency());
  //
  if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
    if (errno != EIDRM && errno != EINVAL) {
      OZ_error("Virtual Sites: cannot remove the shared memory");
    }
  }
}

//
// Send out 'M_UNUSED_ID' messages.  This method and the destuctor
// must be separated - otherwise we will not be able to delete all
// segments (one is needed to send 'M_UNUSED_ID' messages);
// Note also that *before* broadcasting the page must be deleted, 
// thus making importation of it impossible;
void VSMsgChunkPoolSegmentManagerOwned::deleteAndBroadcast()
{
  //
  DebugCode(pool->checkConsistency());
#ifdef TRACE_SEGMENTS
  fprintf(stdout, "*** segment destroyed 0x%X (pid %d)\n",
	  getSHMKey(), getpid());
  fflush(stdout);
#endif

  //
  markDestroy();

  //
  VirtualSite *vs = importersRegister.getFirst();
  while (vs) {
    DSite *site = vs->getSite();
    key_t key = getSHMKey(); 
    VSMsgBufferOwned *mb = composeVSUnusedShmIdMsg(site, key);
    if (sendTo_VirtualSiteImpl(vs, mb,  /* messageType */ M_NONE,
			       /* storeSite */ (DSite *) 0,
			       /* storeIndex */ 0) != ACCEPTED) {
      OZ_error("Unable to send 'M_UNUSED_ID' message to a virtual site!");
    }
    //
    vs = importersRegister.getNext();
  }
}

//
VSMsgChunkPoolSegmentManagerOwned::~VSMsgChunkPoolSegmentManagerOwned()
{
  //
  if (shmdt((char *) mem) < 0) {
    OZ_error("Virtual Sites: can't detach the shared memory.");
  }

  //
  DebugCode(chunkSize = chunksNum = 0);
  DebugCode(shmid = 0);
  DebugCode(shmkey = (key_t) 0);
  DebugCode(mem = (void *) 0);
  DebugCode(chunks = (VSMsgChunkOwned *) 0);
  DebugCode(pool = (VSMsgChunkPoolSegmentOwned *) 0);
}

//
int VSMsgChunkPoolSegmentManagerOwned::scavenge()
{
  int busy = 0;
  DebugCode(pool->checkConsistency());

  //
  purgeMap();
  for (int i = 1; i < chunksNum; i++) // the first one is busy anyway;
    if (!(getChunkAddr(i)->isBusy())) {
      markFreeInMap(i);
      DebugCode(getChunkAddr(i)->checkFreedDebug(chunkSize));
    } else {
      busy++;
    }

  //
  return (busy);
}

//
VSMsgChunkPoolSegmentManagerImported::VSMsgChunkPoolSegmentManagerImported(key_t shmkeyIn)
{
  shmkey = shmkeyIn;
  //
  // we don't know in advance how large it is - so just try to swallow
  // the page "as is";
  if ((int) (shmid = shmget(shmkey, /* size */ 0, S_IRWXU)) < 0) {
    // A missing shared memory page means that its owner is dead;
    // 'pool' must be set to '-1' - used by 'isVoid';
    pool = (VSMsgChunkPoolSegmentImported *) -1;
    chunks = (VSMsgChunkImported *) -1;
    chunkSize = chunksNum = -1;
    return;
  }
  if ((int) (mem = shmat(shmid, (char *) 0, 0)) == -1) 
    OZ_error("Virtual Sites:: failed to attach a shared-memory page");
  //
#ifdef TRACE_SEGMENTS
  fprintf(stdout, "*** segment attached 0x%X (pid %d)\n",
	  shmkey, getpid());
  fflush(stdout);
#endif

  // locations;
  pool = (VSMsgChunkPoolSegmentImported *) mem; // initialized by the owner;
  pool->init(shmkeyIn);		 // actually empty;
  chunks = (VSMsgChunkImported *) mem;

  // Borrow data from the 'VSMsgChunkPoolSegment' object
  // This information is necessary for debugging only (like checking 
  // whether a chunk really belongs to the pool);
  chunkSize = pool->getChunkSize();
  chunksNum = pool->getChunksNum();
}

//
VSMsgChunkPoolSegmentManagerImported::~VSMsgChunkPoolSegmentManagerImported()
{
  DebugCode(pool->checkConsistency());
  //
#ifdef TRACE_SEGMENTS
  fprintf(stdout, "*** segment detached 0x%X (pid %d)\n",
	  shmkey, getpid());
  fflush(stdout);
#endif

  //
  if (isNotVoid() && shmdt((char *) mem) < 0) {
    OZ_error("Virtual Sites: can't detach the shared memory.");
  }

  //
  DebugCode(chunkSize = chunksNum = 0);
  DebugCode(shmid = 0);
  DebugCode(shmkey = (key_t) 0);
  DebugCode(mem = (void *) 0);
  DebugCode(chunks = (VSMsgChunkImported *) 0);
  DebugCode(pool = (VSMsgChunkPoolSegmentImported *) 0);
}

//
unsigned int VSSegmentImportersRegister::hash(VirtualSite *vs)
{
  unsigned char *p = (unsigned char *) &vs;
  unsigned int h = 0, g;

  //
  for(int i = 0; i < (int) sizeof(vs); i++,p++) {
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }
  return (h);
}

//
VirtualSite* VSSegmentImportersRegister::find(VirtualSite *vs)
{
  unsigned int hvalue = hash(vs);
  GenHashNode *aux = htFindFirst(hvalue);
  while(aux) {
    VirtualSite *auxVS;
    GenCast(aux->getBaseKey(), GenHashBaseKey*, auxVS, VirtualSite*);

    //
    if (vs == auxVS) {
      VirtualSite *fvs;
      GenCast(aux->getEntry(), GenHashEntry*,
	      fvs, VirtualSite*);
      return (fvs);
    }

    //
    aux = htFindNext(aux, hvalue);
  }

  //
  return ((VirtualSite *) 0);
}

//
void VSSegmentImportersRegister::insert(VirtualSite *vs)
{
  GenHashBaseKey* ghn_bk;
  GenHashEntry* ghn_e;
  unsigned int hvalue = hash(vs);

  //
  GenCast(vs, VirtualSite*, ghn_bk, GenHashBaseKey*);
  GenCast(vs, VirtualSite*, ghn_e, GenHashEntry*);
  //
  htAdd(hvalue, ghn_bk, ghn_e);
}

//
void VSSegmentImportersRegister::retract(VirtualSite *vs)
{
  Assert(find(vs) == vs);
  unsigned int hvalue = hash(vs);
  GenHashNode *ghn = htFindFirst(hvalue);
  while (ghn) {
    VirtualSite *auxVS;
    GenCast(ghn->getBaseKey(), GenHashBaseKey*, auxVS, VirtualSite*);

    //
    if (vs == auxVS) {
      htSub(hvalue, ghn);
      break;
    }

    //
    ghn = htFindNext(ghn, hvalue);
  }
}

//
VirtualSite *VSSegmentImportersRegister::getFirst()
{
  VirtualSite *vs;
  seqGHN = GenHashTable::getFirst(seqIndex);
  if (seqGHN) {
    GenCast(seqGHN->getEntry(), GenHashEntry*,
	    vs, VirtualSite*);
  } else {
    vs = (VirtualSite *) 0;
  }
  return (vs);
}

//
VirtualSite* VSSegmentImportersRegister::getNext()
{
  VirtualSite *vs;
  seqGHN = GenHashTable::getNext(seqGHN, seqIndex);
  if (seqGHN) {
    GenCast(seqGHN->getEntry(), GenHashEntry*,
	    vs, VirtualSite*);
  } else {
    vs = (VirtualSite *) 0;
  }
  return (vs);
}

//
// Just one segment is created originally;
VSMsgChunkPoolManagerOwned::VSMsgChunkPoolManagerOwned(int chunkSizeIn, 
						       int chunksNumIn,
						       int sizeIn)
  : chunkSize(chunkSizeIn), chunksNum(chunksNumIn),
    currentSegMgrIndex(0), toBeUsedSegments(1), phaseNumber(0)
{
  lastGC = am.getEmulatorClock();
  VSMsgChunkPoolSegmentManagerOwned *fsm =
    new VSMsgChunkPoolSegmentManagerOwned(chunkSizeIn, chunksNumIn, sizeIn);
  push(fsm);
}

//
VSMsgChunkPoolManagerOwned::~VSMsgChunkPoolManagerOwned()
{
  for (int i = 0; i < getSize(); i++) {
    VSMsgChunkPoolSegmentManagerOwned *sm = get(i);
    sm->deleteAndBroadcast();
  }

  //
  while (getSize()) {
    VSMsgChunkPoolSegmentManagerOwned *sm = pop();
    delete sm;
  }
}

//
//
void VSMsgChunkPoolManagerOwned::scavenge()
{
  int chunks = 0;		// used chunks;
  int totalSegments = getSize();
  int usedSegments;
  // An upper limit for the number of messages that can be sent out
  // during scavenging;
  int maxMessages = 0, maxSegmentsMessages;

  //
  // Reset the number *before* any "M_UNUSED_ID' could be ever
  // composed;
  currentSegMgrIndex = 0;

  //
  for (int i = 0; i < totalSegments; i++) {
    VSMsgChunkPoolSegmentManagerOwned *fsm = get(i);
    chunks = chunks + fsm->scavenge();
    // For each segment, each registered (for it) site could get a
    // M_UNUSED_ID message:
    maxMessages = maxMessages + fsm->getNumOfRegisteredSites();
  }

  //
  // (elapsed number of segments - if they'd fully packed;)
  usedSegments = chunks/chunksNum + 1;
  toBeUsedSegments = usedSegments * VS_MSGCHUNKS_USAGE;

  //
  maxSegmentsMessages = maxMessages/chunksNum + 1;
  // It should not happen that during GCing of segments (next
  // paragraph) a segment is removed that is used for the GCing itself
  // ('M_UNUSED_ID' messages). If there are more free segments than
  // 'maxSegmentsMessages', it will not happen:
  if (maxSegmentsMessages > toBeUsedSegments - usedSegments) 
    // i.e. needed segments > free segments among those that will
    // survive GC - then set the new border;
    toBeUsedSegments = maxSegmentsMessages + usedSegments;

  //
  // Don't try to reclaim segments that are to be used in the
  // following allocation phase;
  for (int i = totalSegments-1; i >= toBeUsedSegments; i--) {
    //
    VSMsgChunkPoolSegmentManagerOwned *fsm = get(i);
    if (fsm->getPhaseNumber() + VS_SEGS_MAXIDLE_PHASES < phaseNumber) {
      VSMsgChunkPoolSegmentManagerOwned *sm = pop();

      //
      Assert(sm == fsm);
      fsm->deleteAndBroadcast();
      delete fsm;
    } else {
      break;
    }
  }

  //
  setLastGC(am.getEmulatorClock());
  phaseNumber++;
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
VSMsgChunkPoolSegmentManagerImported* VSChunkPoolRegister::find(key_t key)
{
  int hvalue = hash(key);
  GenHashNode *aux = htFindFirst(hvalue);
  while(aux) {
    key_t auxKey;
    GenCast(aux->getBaseKey(), GenHashBaseKey*, auxKey, key_t);

    //
    if (key == auxKey) {
      VSMsgChunkPoolSegmentManagerImported *msgChunkPoolManager;
      GenCast(aux->getEntry(), GenHashEntry*,
	      msgChunkPoolManager, VSMsgChunkPoolSegmentManagerImported*);
      return (msgChunkPoolManager);
    }

    //
    aux = htFindNext(aux, hvalue);
  }
  return ((VSMsgChunkPoolSegmentManagerImported *) 0);
}

//
void VSChunkPoolRegister::add(key_t key, VSMsgChunkPoolSegmentManagerImported *pool)
{
  GenHashBaseKey* ghn_bk;
  GenHashEntry* ghn_e;
  int hvalue = hash(key);

  //
  GenCast(key, key_t, ghn_bk, GenHashBaseKey*);
  GenCast(pool, VSMsgChunkPoolSegmentManager*, ghn_e, GenHashEntry*);
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
  if (site) 
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
