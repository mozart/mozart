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
  Assert(!(type & ~VS_KEYTYPE_MASK));
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
VSMsgChunkPoolSegmentManagerOwned(VSResourceManager *vsRMin,
				  int chunkSizeIn, int chunksNumIn,
				  int regInitSize)
  : vsRM(vsRMin),
    fs(chunksNumIn), phaseNumber(0), importersRegister(regInitSize)
{
  chunkSize = chunkSizeIn;
  chunksNum = chunksNumIn;

#ifdef VS_DEBUG_RESOURCES
  if (!vsRM->canAllocate()) {
    setVoid();
#if defined(SOLARIS)
    lERRNO = EMFILE;
#else
    lERRNO = ENOMEM;
#endif
    DebugCode(pool = (VSMsgChunkPoolSegmentOwned *) -1;);
    DebugCode(chunks = (VSMsgChunkOwned *) -1;);
    return;
  }
#endif

  // 
  // Get&attach a shared memory page;
repeat:
  shmkey = vsTypeToKey(VS_MSGBUFFER_KEY);

  //
  if ((int) (shmid = shmget(shmkey, chunkSizeIn*chunksNumIn,
			    (IPC_CREAT | IPC_EXCL | S_IRWXU))) < 0) {
    setVoid();
    lERRNO = errno;
    DebugCode(pool = (VSMsgChunkPoolSegmentOwned *) -1;);
    DebugCode(chunks = (VSMsgChunkOwned *) -1;);
    Assert(isVoid());		// check 'isVoid()' itself;
  } else if ((mem = shmat(shmid, (char *) 0, 0)) == (void *) -1) {
    (void) shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0);
    DebugCode(shmid = -1);
    setVoid();
    lERRNO = errno;
    DebugCode(pool = (VSMsgChunkPoolSegmentOwned *) -1;);
    DebugCode(chunks = (VSMsgChunkOwned *) -1;);
    Assert(isVoid());		// check 'isVoid()' itself;
  } else {
    vsRM->shmPageMapped();
    //
#ifdef TRACE_SEGMENTS
    fprintf(stdout, "*** segment created 0x%X (pid %d)\n",
	    shmkey, osgetpid());
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
    Assert(isEmpty());		// to check 'isEmpty()' itself;
    Assert(!isVoid());		// check 'isVoid()' itself;
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
// Send out 'M_UNUSED_ID' messages.  This method and the destructor
// must be separated - otherwise we will not be able to delete all
// segments (one is needed to send 'M_UNUSED_ID' messages); Note also
// that *before* broadcasting the page must be deleted, thus making
// importation of it impossible;
void VSMsgChunkPoolSegmentManagerOwned::deleteAndBroadcast()
{
  //
  // kost@ it CAN be empty: (a) scavenging not done (that is, it IS empty
  // but not with this test, and (b) termination - we don't care...
  // Assert(isEmpty());
  DebugCode(pool->checkConsistency());
#ifdef TRACE_SEGMENTS
  fprintf(stdout, "*** segment destroyed 0x%X (pid %d)\n",
	  getSHMKey(), osgetpid());
  fflush(stdout);
#endif

  //
  markDestroy();

  //
  VirtualSite *vs = importersRegister.getFirst();
  while (vs) {
    DSite *site = vs->getSite();
    key_t key = getSHMKey(); 
    VSMarshalerBufferOwned *mb = composeVSUnusedShmIdMsg(site, key);
    // 'sendTo' may be noop if the site is already disconnected;
    (void) sendTo_VirtualSiteImpl(vs, mb,  /* messageType */ M_NONE,
				  /* storeSite */ (DSite *) 0,
				  /* storeIndex */ 0);
    //
    vs = importersRegister.getNext();
  }
}

//
VSMsgChunkPoolSegmentManagerOwned::~VSMsgChunkPoolSegmentManagerOwned()
{
  //
  if (!isVoid()) {
    vsRM->shmPageUnmapped();
    if (shmdt((char *) mem) < 0)
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
VSMsgChunkPoolSegmentManagerImported::VSMsgChunkPoolSegmentManagerImported(VSResourceManager *vsRMin, key_t shmkeyIn)
  : VSMsgChunkPoolSegmentManager(shmkeyIn), vsRM(vsRMin)
{
#ifdef VS_DEBUG_RESOURCES
  if (!vsRM->canAllocate()) {
    setVoid();
#if defined(SOLARIS)
    lERRNO = EMFILE;
#else
    lERRNO = ENOMEM;
#endif
    DebugCode(chunks = (VSMsgChunkImported *) -1;);
    DebugCode(chunkSize = chunksNum = -1;);
    return;
  }
#endif
  
  //
  // we don't know in advance how large it is - so just try to swallow
  // the page "as is";
  if ((int) (shmid = shmget(shmkey, /* size */ 0, S_IRWXU)) < 0) {
    // A missing shared memory page means that its owner is dead;
    // 'pool' must be set to '-1' - used by 'isVoid';
    setVoid();
    lERRNO = errno;
    DebugCode(chunks = (VSMsgChunkImported *) -1;);
    DebugCode(chunkSize = chunksNum = -1;);
    Assert(isVoid());
  } else if ((mem = shmat(shmid, (char *) 0, 0)) == (void *) -1) {
    setVoid();
    lERRNO = errno;
    DebugCode(chunks = (VSMsgChunkImported *) -1;);
    DebugCode(chunkSize = chunksNum = -1;);
    Assert(isVoid());
  } else {
    vsRM->shmPageMapped();
#ifdef TRACE_SEGMENTS
    fprintf(stdout, "*** segment attached 0x%X (pid %d)\n",
	    shmkey, osgetpid());
    fflush(stdout);
#endif

    //
    // locations;
    // initialized by the owner;
    pool = (VSMsgChunkPoolSegmentImported *) mem;
    pool->init(shmkeyIn);	// actually empty;
    chunks = (VSMsgChunkImported *) mem;

    // Borrow data from the 'VSMsgChunkPoolSegment' object
    // This information is necessary for debugging only (like checking 
    // whether a chunk really belongs to the pool);
    chunkSize = pool->getChunkSize();
    chunksNum = pool->getChunksNum();
    Assert(!isVoid());
  }
}

//
VSMsgChunkPoolSegmentManagerImported::~VSMsgChunkPoolSegmentManagerImported()
{
  DebugCode(pool->checkConsistency());
  //
#ifdef TRACE_SEGMENTS
  fprintf(stdout, "*** segment detached 0x%X (pid %d)\n",
	  shmkey, osgetpid());
  fflush(stdout);
#endif

  //
  if (!isVoid()) {
    vsRM->shmPageUnmapped();
    if (shmdt((char *) mem) < 0)
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
    auxVS = (VirtualSite*) aux->getBaseKey();

    //
    if (vs == auxVS) {
      VirtualSite *fvs;
      fvs = (VirtualSite*) aux->getEntry();
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
  ghn_bk = (GenHashBaseKey*) vs;
  ghn_e = (GenHashEntry*) vs;
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
    auxVS = (VirtualSite*) ghn->getBaseKey();

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
    vs = (VirtualSite*) seqGHN->getEntry();
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
    vs = (VirtualSite*) seqGHN->getEntry();
  } else {
    vs = (VirtualSite *) 0;
  }
  return (vs);
}

//
// Just one segment is created originally;
VSMsgChunkPoolManagerOwned::VSMsgChunkPoolManagerOwned(VSResourceManager *vsRMin,
						       int chunkSizeIn, 
						       int chunksNumIn,
						       int sizeIn)
  : vsRM(vsRMin), chunkSize(chunkSizeIn), chunksNum(chunksNumIn), 
    currentSegMgrIndex(0), toBeUsedSegments(1), phaseNumber(0)
{
  lastGC = am.getEmulatorClock();
  VSMsgChunkPoolSegmentManagerOwned *fsm =
    new VSMsgChunkPoolSegmentManagerOwned(vsRM,
					  chunkSizeIn, chunksNumIn, sizeIn);
  if (fsm->isVoid()) {
    // We will not be able obviously to do anything when there are
    // resource problems right from the beginning;
    message("Virtual sites: cannot allocate a shm page!");
    message("Please ask your system administrator to increase");
    message("the number of shared memory pages allowed.");
    am.exitOz(-1);
  } else {
    push(fsm);
  }
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
VSMsgChunkPoolSegmentManagerOwned* 
VSMsgChunkPoolManagerOwned::allocateSegmentManager()
{
  // we don't have one left over from the past - get it now;
  VSMsgChunkPoolSegmentManagerOwned *fsm =
    new VSMsgChunkPoolSegmentManagerOwned(vsRM,
					  VS_CHUNK_SIZE,
					  VS_CHUNKS_NUM,
					  VS_REGISTER_HT_SIZE);

  //
  if (fsm->isVoid()) {
    delete fsm;

    //
    // Hopefully, we'll get something out of this.
    // ('VS_RM_Block' means don't return without a page
    // reclaimed. Terminate the process if nothing can be
    // found within a reasonable amount of time (see
    // resource.hh - 'VS_RESOURCE_WAIT' and
    // 'VS_WAIT_ROUNDS'));
    //
    // Observe also that all the manager's control parameters
    // ('toBeUsedSegments', etc.) can change after this point
    // (since resource manager can decide to do scavenging on
    // its own );
    (void) vsRM->doResourceGC(VS_RM_OwnSeg, VS_RM_Attach, VS_RM_Block);

    //
    // 'doResourceGC()' can perform scavenging, in which case free
    // chunks can appear in already allocated pages, and in any case
    // the 'currentSegMgrIndex' would be reset:
    while (currentSegMgrIndex < getSize()) {
      fsm = get(currentSegMgrIndex);
      if (fsm->getAvailChunksNum())
	break;
    }

    //
    if (currentSegMgrIndex >= getSize()) {
      // being here means no free chunks have been reclaimed, thus we
      // still have to allocate a new page:
      // 
      fsm = new VSMsgChunkPoolSegmentManagerOwned(vsRM,
						  VS_CHUNK_SIZE,
						  VS_CHUNKS_NUM,
						  VS_REGISTER_HT_SIZE);

      //
      // If the allocation fails again, then it means that we
      // have reached some system-wide limits we cannot object
      // against (that's aka "virtual memory exhausted") - we
      // bail out;
      if (fsm->isVoid()) {
	message("Virtual sites: cannot allocate a shm page!");
	message("Please ask your system administrator to increase");
	message("the number of shared memory pages allowed.");
	am.exitOz(-1);
	return ((VSMsgChunkPoolSegmentManagerOwned *) 0);
      }
      //
      push(fsm);
    }
    // else 'fsm' is a segment manager with free chunk(s);
  } else {
    Assert(!fsm->isVoid());
    push(fsm);
  }

  //
  Assert(fsm == get(currentSegMgrIndex));
  return (fsm);
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
  int maxMessages = 0;

  //
  // Reset the number *before* any 'M_UNUSED_ID' could be ever
  // composed;
  currentSegMgrIndex = 0;

  //
  for (int i = 0; i < totalSegments; i++) {
    VSMsgChunkPoolSegmentManagerOwned *fsm = get(i);
    chunks = chunks + fsm->scavenge();
    // For each segment, each registered (for it) site could get a
    // M_UNUSED_ID message:
    if (chunks == 0)
      maxMessages = maxMessages + fsm->getNumOfRegisteredSites();
  }

  //
  // There must be enough room for sending 'M_UNUSED_ID' messages:
  chunks = chunks + maxMessages; // each messages takes a chunk;

  //
  // (elapsed number of segments - if they'd be fully packed;)
  usedSegments = chunks/chunksNum + 1;
  // Resource manager can decide to trim allocated pages as strong
  // as possible (by means of "usage factor");
  toBeUsedSegments = usedSegments * vsRM->getVSMsgChunksUsage();
  // 'toBeUsedSegments' can be in no case less than necessary for
  // 'M_UNUSED_ID' messages (that's essential since "chunks usage" can
  // be actually zero):
  toBeUsedSegments = max(toBeUsedSegments, (maxMessages/chunksNum + 1));

  //
  // Resource manager can define "max idle phases" to be 0, thus all
  // the free pages will be purged;
  const int maxidle = vsRM->getVSSegsMaxIdlePhases();

  //
  // Don't try to reclaim segments that are to be used in the
  // following allocation phase;
  for (int i = totalSegments-1; i >= toBeUsedSegments; i--) {
    //
    // ... Neither try to reclaim currently busy (non-empty) segments:
    VSMsgChunkPoolSegmentManagerOwned *fsm = get(i);
    if (fsm->isEmpty() && 
	fsm->getPhaseNumber() + maxidle <= phaseNumber) {
      VSMsgChunkPoolSegmentManagerOwned *sm = pop();

      //
      Assert(sm == fsm);
      fsm->deleteAndBroadcast();
      delete fsm;
    } else {
      // That's it: cannot proceed further right now. However, this
      // delays reclaiming of free segments if they happen to be
      // "below" a non-free one.
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
    auxKey = (key_t) aux->getBaseKey();

    //
    if (key == auxKey) {
      VSMsgChunkPoolSegmentManagerImported *msgChunkPoolManager;
      msgChunkPoolManager = (VSMsgChunkPoolSegmentManagerImported*) aux->getEntry();
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
  ghn_bk = (GenHashBaseKey*) key;
  ghn_e = (GenHashEntry*) pool;
  htAdd(hvalue, ghn_bk, ghn_e);
}

//
void VSChunkPoolRegister::remove(key_t key)
{
  int hvalue = hash(key);
  GenHashNode *aux = htFindFirst(hvalue);
  while(aux) {
    key_t auxKey;
    auxKey = (key_t) aux->getBaseKey();

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
VSMsgChunkPoolSegmentManagerImported* 
VSChunkPoolRegister::handleVoid(key_t key,
				VSMsgChunkPoolSegmentManagerImported *aux)
{
  switch (aux->getErrno()) {

#if defined(SOLARIS)
  case EMFILE:
#endif
  case ENOMEM:
    // resources problem - let's try to get a page free;
    (void) vsRM->doResourceGC(VS_RM_ImpSeg, VS_RM_Attach, VS_RM_Block);

    //
    delete aux;
    aux = new VSMsgChunkPoolSegmentManagerImported(vsRM, key);
    if (aux->isVoid()) {
      switch (aux->getErrno()) {
      case EMFILE:
	message("Virtual sites: cannot attach a shm page!");
	message("Please ask your system administrator to increase");
	message("the number of shared memory pages allowed.");
	am.exitOz(-1);

      default:
	// ... now a non-resource problem - proceed with it;
	break;
      }

      //
      delete aux;
      aux = (VSMsgChunkPoolSegmentManagerImported *) 0;
    }
    // here 'aux' can be a complete (non-void) segment manager;
    break;

  default:
    // there is no such shared memory page;
    delete aux;
    aux = (VSMsgChunkPoolSegmentManagerImported *) 0;
    break;
  }

  //
  return (aux);
}

#endif // VIRTUALSITES
