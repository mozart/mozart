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

#ifndef __VS_MSGBUFFER_HH
#define __VS_MSGBUFFER_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"

#ifdef VIRTUALSITES

#include "genhashtbl.hh"
#include "msgbuffer.hh"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

//
// IDs of IPC keys used for different things (4 bits);
#define VS_KEYTYPE_SIZE      4
#define VS_MSGBUFFER_KEY     0x1
#define VS_WAKEUPSEM_KEY     0x2
//
#define VS_SEQ_SIZE          12
// altogether - 32 bits, so sizeof(key_t) >= sizeof(int32)
#define VS_PID_SIZE          16
#define VS_PID_MASK          0xffff

//
// Pieces of memory composing a 'VSMsgBuffer'.
// These objects are located in the shared memory page containing 
// outgoing messages;
// They are designed to fit into a 'VS_BLOCKSIZE' block;
class VSMsgChunk {
protected:
  int next;			// next chunk if any (and -1 otherwise); 
  volatile Bool busy;		// set by owner and dropped by receiver;
  BYTE buffer[0];		// actually more - fills up the chunk;

  //
public:
  // These objects are unusable;
  VSMsgChunk()  { error("VSMsgChunk allocated???"); }
  ~VSMsgChunk() { error("VSMsgChunk deallocated???"); }

  //
  BYTE *getDataAddr() { return (&buffer[0]); }
  int getNext() { return (next); }
};

//
class VSMsgChunkOwned : public VSMsgChunk {
public:
  // The body of the chunk contains garbage initially.
  // (Chunks are initialized lazily - prior usage;)
  void init() {
    next = -1;
    busy = TRUE;
  }

  //
  Bool isBusy() { return (busy); }
  void markFree() { busy = FALSE; }

  //
  void setNext(int nIn) { next = nIn; }
};

//
class VSMsgChunkImported : public VSMsgChunk {
public:
  //
  void markFree() { busy = FALSE; }
};

//
class VSMsgChunkReserved : public VSMsgChunk {
public:
  //
  void initReservedChunk() {
    next = -1;
    busy = TRUE;
  }
};

//
// The pool object identifies the pool; it is located in the shared
// memory page as a zeroth special "reserved" data chunk (since its
// 'wakeupMode' and 'semkey' are to be shared);
//
class VSMsgChunkPool : protected VSMsgChunkReserved {
protected:
  int chunkSize, chunksNum;	// used by receiver sites;
  volatile Bool wakeupMode;	// 'OK' if receiver(s) has to increment;
  key_t shmkey;			// for consistency check;
  key_t semkey;			// the id of the 'wakeupMode' semaphor;

  //
public:
  // These objects are unusable;
  VSMsgChunkPool() { error("VSMsgChunkPool allocated?"); }
  ~VSMsgChunkPool() { error("VSMsgChunkPool destroyed?"); }

  //
#ifdef DEBUG_CHECK
  void checkConsistency() {
    // check for overwriting...
    Assert(wakeupMode == TRUE || wakeupMode == FALSE);
  }
#endif

  //
  key_t getSemkey()  { DebugCode(checkConsistency()); return (semkey); }
  int getChunkSize() { DebugCode(checkConsistency()); return (chunkSize); }
  int getChunksNum() { DebugCode(checkConsistency()); return (chunksNum); }
};

//
class VSMsgChunkPoolOwned : public VSMsgChunkPool {
public:
  //
  VSMsgChunkPoolOwned() { error("VSMsgChunkPoolOwned allocated?"); }
  ~VSMsgChunkPoolOwned() { error("VSMsgChunkPoolOwned destroyed?"); }

  //
  void init(key_t shmkeyIn, int chunkSizeIn, int chunksNumIn);

  //
  void startWakeupMode() {
    DebugCode(checkConsistency());
    wakeupMode = TRUE;
  }
  void stopWakeupMode() {
    DebugCode(checkConsistency());
    wakeupMode = FALSE;
  }
  DebugCode(Bool isInWakeupMode() { return (wakeupMode); })
};

//
class VSMsgChunkPoolImported : public VSMsgChunkPool {
public:
  //
  VSMsgChunkPoolImported() { error("VSMsgChunkPoolImported allocated?"); }
  ~VSMsgChunkPoolImported() { error("VSMsgChunkPoolImported destroyed?"); }

  //
  void init(key_t shmkeyIn) {
    Assert(shmkey == shmkeyIn);
  }

  //
  Bool isInWakeupMode() { return (wakeupMode); }
};

//
// E.g. Linux has the structure already defined (which is what i'd
// expect), while Solaris has not;
#if defined(SOLARIS)
typedef union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
} SemOptArg;
#else  // defined(LINUX) || defined(NETBSD)
typedef union semun SemOptArg;
#endif

//
// The 'Manager' class, like the mailbox's one, is visible to the 
// local Oz process only. (Actually, 'key_t' keys are replicated);
class VSMsgChunkPoolManager {
protected:
  //
  int chunkSize, chunksNum;	//
  //
  key_t shmkey;			// shared memory;
  int shmid;			// 
  //
  key_t semkey;			// run-over semaphore;
  int semid;			// 
  //
  // Pointers ('mem', 'chunks' and 'pool') are actually the same; 
  // Note that the first chunk is permanently busy (reserved);
  void *mem;			// the address of the shm page;

  //
public:
  //
  VSMsgChunkPoolManager() { 
    DebugCode(chunkSize = chunksNum = 0);
    DebugCode(shmid = 0);
    DebugCode(shmkey = (key_t) 0);
    DebugCode(semid = 0);
    DebugCode(semkey = (key_t) 0);
    DebugCode(mem = (void *) 0);
  }
  ~VSMsgChunkPoolManager() {}
};

//
class VSMsgChunkPoolManagerOwned : public VSMsgChunkPoolManager {
private:
  //
  VSMsgChunkOwned *chunks;
  VSMsgChunkPoolOwned *pool;
  //
  int mapSize;			// chunksNum/32;
  int32 *map;			// the usage bitmap - '1' if free;

  //
private:
  //
  void markFreeInMap(int cn) {
    DebugCode(pool->checkConsistency());
    Assert(cn > 0 && cn < chunksNum);
    int wordNum = cn/32;	// in an 'int32' value; 
    int bitNum = cn%32;
    map[wordNum] |= (0x1 << bitNum);
  }

  //
  // find the first free page in the map;
  int allocateFromMap() {
    DebugCode(pool->checkConsistency());
    for (int i = 0; i < mapSize; i++) {
      if (map[i]) {
	int32 seg = map[i];
	int j = 0;

	//
	while (!(seg & 0x1)) {
	  j++;
	  seg = seg>>1;
	}

	//
	map[i] &= ~(0x1 << j);	// got it;
	int num = i*32 + j;
	Assert(num != 0);	// zeroth is always busy;
	return (num);
      }
    }
    return (-1);		// none found;
  }

  //
  // update the map by analyzing chunks themselves;
  void scavengeMemory();

  //
  int getMsgChunkOutline();

  //
public:
  //
  VSMsgChunkPoolManagerOwned(int chunkSizeIn, int chunksNumIn);
  ~VSMsgChunkPoolManagerOwned();

  //
  void markDestroy();		// mark for destruction;

  //
  VSMsgChunkOwned *getChunkAddr(int i) {
    Assert(sizeof(VSMsgChunk) == sizeof(VSMsgChunkOwned));
    return ((VSMsgChunkOwned *) (((char *) chunks) + i*chunkSize));
  }
  int getChunkDataSize() {
    Assert(sizeof(VSMsgChunk) == sizeof(VSMsgChunkOwned));
    return (chunkSize - sizeof(VSMsgChunkOwned));
  }

  //
  key_t getSHMKey() {
    DebugCode(pool->checkConsistency());
    return (shmkey);
  }

  //
  // 'getMsgChunk' tries first to find out an unused chunk in the map,
  // and if it fails - it scavenges for free chunks (and updates the
  // map correspondingly). When no more free chunks are found, it
  // calls the "outline" version which blocks on the semaphore which
  // must be stepped up by a VS that frees a chunk (see the first
  // assertion in the constructor);
  int getMsgChunk() {
    DebugCode(pool->checkConsistency());
    // 
    int chunkNum = allocateFromMap();
    if (chunkNum < 0) 
      chunkNum = getMsgChunkOutline();
    //
    getChunkAddr(chunkNum)->init();
    //
    return (chunkNum);
  }

  //
  // (sender site release - when e.g. message cannot be sent);
  void releaseChunk(VSMsgChunkOwned *chunk) {
    DebugCode(pool->checkConsistency());
    chunk->markFree();
    Assert(!pool->isInWakeupMode());
  }

  //
  // one chunk is permanently reserved;
  int getChunksNum() { return (pool->getChunksNum()-1); }
};

//
class VSMsgChunkPoolManagerImported : public VSMsgChunkPoolManager {
private:
  VSMsgChunkImported *chunks;
  VSMsgChunkPoolImported *pool;

  //
private:
  //
  void wakeupOwner();

  //
public:
  //
  VSMsgChunkPoolManagerImported(key_t shmkey);
  ~VSMsgChunkPoolManagerImported();

  //
  // At the receiver site the semaphore must checked as well;
  void releaseChunk(VSMsgChunkImported *chunk) {
    DebugCode(pool->checkConsistency());
    chunk->markFree();
    if (pool->isInWakeupMode())
      wakeupOwner();
  }

  //
  VSMsgChunkImported *getChunkAddr(int i) {
    Assert(sizeof(VSMsgChunk) == sizeof(VSMsgChunkImported));
    return ((VSMsgChunkImported *) (((char *) chunks) + i*chunkSize));
  }
  int getChunkDataSize() {
    Assert(sizeof(VSMsgChunk) == sizeof(VSMsgChunkImported));
    return (chunkSize - sizeof(VSMsgChunkImported));
  }
};

//
// The register object - it keeps track of imported chunk pools or 
// creates a new one if necessary;
class VSChunkPoolRegister : public GenHashTable {
private:
  unsigned int hash(key_t key);

  //
private:
  void add(key_t key, VSMsgChunkPoolManagerImported *pool);
  void remove(key_t key);

  //
public:
  VSChunkPoolRegister(int size) : GenHashTable(size) {}
  ~VSChunkPoolRegister() {}

  //
  VSMsgChunkPoolManagerImported* find(key_t key);

  //
  VSMsgChunkPoolManagerImported* import(key_t key) {
    VSMsgChunkPoolManagerImported *aux;

    //
    aux = find(key);
    if (!aux) { 
      aux = new VSMsgChunkPoolManagerImported(key);
      add(key, aux);
    }

    //
    return (aux);
  }

  //
  void forget(key_t key) {
    remove (key);
  }
};

//
// The message buffer used for communications between VS"s.
//
// Objects of this (and inherited) class(es) reside in the private
// memory and used for constructing/reading message buffers.
//
class VSMsgBuffer : public MsgBuffer {
protected:
  //
  int first, current;		// chunk indexes;
  // 'BYTE *posMB' and BYTE *endMB' are inherited from 'MsgBuffer';

  //
public:
  VSMsgBuffer() : first(-1) {
    // Ralf Scheidhauer' liebe fuer 'init()' anstatt class
    // constructors, $%@#$# @#$ #@$*@#$ $#$$*@&#*$* &@ $@#^@#
    // $**#& !!!!!!!!!!!!!
    MsgBuffer::init();
    DebugCode(current = -1);
    // this should be in MsgBuffer's constructor (or initializer, 
    // whichever is preferred);
    DebugCode(posMB = endMB = (BYTE *) 0);
  }
  virtual ~VSMsgBuffer() { error("VSMsgBuffer destroyed?"); }

  //
  // Various special stuff;
  // SB guys changed the interface without notifying us:
  Bool isPersistentBuffer() { return (NO); }
  // ... EK???
  void unmarshalReset() {}
};

//
// The message buffer used for sending out messages. 
//
// Note that these objects can be used for both marshaling AND
// unmarshaling. The last happens e.g. when the message cannot be sent
// out;
//
// These objects are allocated from a special "free list" (defined in
// virtual.cc);
//
class VSMsgBufferOwned : public VSMsgBuffer {
private:
  //
  Site *site;			// has to know the site when marshaling;
  VSMsgChunkPoolManagerOwned *cpm;
  VSMsgChunkOwned *currentAddr;	// of the current chunk;
  int allocatedChunks;		// keep track of size;

  //
private:
  //
  // Called whenever 'put' faces no space in the buffer;
  void allocateNextChunk() {
    //
    if (allocatedChunks >= cpm->getChunksNum())
      error("no space for a message to a virtual site!");

    //
    // 'getMsgChunk()' ultimately gets new chunks (including waiting
    // in there if needed);
    int next = cpm->getMsgChunk();
    allocatedChunks++;
    VSMsgChunkOwned *nextAddr = cpm->getChunkAddr(next);

    //
    posMB = nextAddr->getDataAddr();
    endMB = posMB + cpm->getChunkDataSize() - 1; // in bytes; // kost@ : TODO

    // link it up;
    currentAddr->setNext(next);
    current = next;
    currentAddr = nextAddr;
  }

  //
  // Jump to the next buffer (when unmarshaling);
  void gotoNextChunk() {
    current = currentAddr->getNext();
    Assert(current >= 0);

    //
    currentAddr = cpm->getChunkAddr(current);

    //
    posMB = currentAddr->getDataAddr();
    endMB = posMB + cpm->getChunkDataSize(); // in bytes;
  }

  //
private:
  //
  BYTE getNext() {
    Assert(posMB == endMB);
    gotoNextChunk();
    return (*posMB++);
  }
  void putNext(BYTE byte) {
    Assert(posMB == endMB+1);	// kost@ : TODO
    allocateNextChunk();
    *posMB++ = byte;
  }

  //
public:
  //
  void* operator new(size_t size) {
    error("VSMsgBufferOwned allocated using 'new(size_t)'");
    return ((void *) -1);	// gcc warning;
  }
  void* operator new(size_t, void *place) { return (place); }

  //
  VSMsgBufferOwned(VSMsgChunkPoolManagerOwned *cpmIn, Site *siteIn) 
    : cpm(cpmIn), site(siteIn) { 
    DebugCode(currentAddr = (VSMsgChunkOwned *) 0);
  }
  virtual ~VSMsgBufferOwned() {
    error("VSMsgBufferOwned destroyed?");
  }

  //
  // Mark (shared) memory chunks free - when unmarshaling unsent
  // message back;
  void releaseChunks() {
    Assert(first >= 0);
    current = first;
    do {
      VSMsgChunkOwned *chunkAddr = cpm->getChunkAddr(current);
      current = chunkAddr->getNext(); // this&next in this order;
      cpm->releaseChunk(chunkAddr);
    } while (current >= 0);
    Assert(current == -1);

    //
    DebugCode(first = -1);
    DebugCode(currentAddr = (VSMsgChunkOwned *) 0);
    DebugCode(posMB = endMB = (BYTE *) 0);
  }

  //
  // ... hand them over (to a receiver);
  void passChunks() {
    Assert(first >= 0);
    //
    DebugCode(first = -1);
    DebugCode(current = -1);
    DebugCode(currentAddr = (VSMsgChunkOwned *) 0);
    DebugCode(posMB = endMB = (BYTE *) 0);
  }

  //
  // (Consistency check - must be released/passed already:)
  void cleanup() {
    Assert(first == -1);
    Assert(current == -1);
    Assert(currentAddr == (VSMsgChunk *) 0);
    Assert(posMB == (BYTE *) 0);
    Assert(endMB == (BYTE *) 0);
  }

  //
  // "to be provided" methods;
  void marshalBegin() {		// allocates first chunk;
    Assert(first < 0);
    // allocate the first chunk unconditionally
    // (there are no empty messages?)
    current = first = cpm->getMsgChunk();
    allocatedChunks = 1;
    currentAddr = cpm->getChunkAddr(current);
    //
    posMB = currentAddr->getDataAddr();
    endMB = posMB + cpm->getChunkDataSize() - 1; // in bytes; // kost@ : TODO
  }
  void marshalEnd() {}		// no special action now;
  void unmarshalBegin() {}	// already initialized (allocated);
  void unmarshalEnd() {
    releaseChunks();
  }

  //
  // Yields the first chunk's address - for putting it in a mailbox;
  int getFirstChunk() { return (first); }
  key_t getSHMKey() { return (cpm->getSHMKey()); }

  //
  // ('siteStringrep' exists just for debugging;)
  char* siteStringrep();
  // The reasons for having 'getSite()' are:
  // (a) the marshaling routine has to know sometimes where the 
  //     stuff is to be sent;
  // (b) while discarding unsent messages, we have to know the type
  //     of each particular buffer;
  Site* getSite() { return (site); }
};

//
// The message buffer used for feeding in incoming messages.
//
// (There is a single object of this type, which is re-initialized
// each time a message arrives);
//
class VSMsgBufferImported : public VSMsgBuffer {
private:
  //
  VSMsgChunkPoolManagerImported *cpm;
  VSMsgChunkImported *currentAddr; // chunk itself;

  //
private:
  //
  void gotoNextChunk() {
    current = currentAddr->getNext();
    Assert(current >= 0);

    //
    currentAddr = cpm->getChunkAddr(current);

    //
    posMB = currentAddr->getDataAddr();
    endMB = posMB + cpm->getChunkDataSize(); // in bytes;
  }

  //
private:
  //
  BYTE getNext() {
    Assert(posMB == endMB);
    gotoNextChunk();
    return (*posMB++);
  }

  //
public:
  //
  VSMsgBufferImported(VSMsgChunkPoolManagerImported *cpmIn, int chunkIndex)
    : cpm(cpmIn) {
    current = first = chunkIndex;
    currentAddr = cpm->getChunkAddr(current);
    posMB = currentAddr->getDataAddr();
    endMB = posMB + cpm->getChunkDataSize(); // in bytes;
  }
  virtual ~VSMsgBufferImported() {
    error("VSMsgBufferImported destroyed?");
  }

  //
  // Mark (shared) memory chunks free (so the sender could reuse them);
  void releaseChunks() {
    Assert(first >= 0);
    current = first;
    do {
      VSMsgChunkImported *chunkAddr = cpm->getChunkAddr(current);
      current = chunkAddr->getNext(); // this&next in this order;
      cpm->releaseChunk(chunkAddr);
    } while (current >= 0);
    Assert(current == -1);

    //
    DebugCode(first = -1);
    DebugCode(currentAddr = (VSMsgChunkImported *) 0);
    DebugCode(posMB = endMB = (BYTE *) 0);
  }

  //
  // Consistency check - must be released already:
  void cleanup() {
    Assert(first == -1);
    Assert(current == -1);
    Assert(currentAddr == (VSMsgChunkImported *) 0);
    Assert(posMB == (BYTE *) 0);
    Assert(endMB == (BYTE *) 0);
  }

  //
  // "to be provided" methods;
  void marshalBegin() { error("VSMsgBufferImported::marshalBegin?"); }
  void marshalEnd()   { error("VSMsgBufferImported::marshalBegin?"); }
  void unmarshalBegin() {}	// already initialized (allocated);
  void unmarshalEnd() { releaseChunks(); }

  //
  // ('siteStringrep' exists just for debugging;)
  char* siteStringrep();
};

#endif // VIRTUALSITES

#endif  // __VS_MSGBUFFER_HH
