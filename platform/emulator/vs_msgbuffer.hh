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

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

#include "base.hh"
#include "genhashtbl.hh"
#include "msgbuffer.hh"

//
// IDs of IPC keys used for different things (4 bits);
#define VS_KEYTYPE_SIZE      4
#define VS_MSGBUFFER_KEY     0x1
#define VS_WAKEUPSEM_KEY     0x2

//
// Pieces of memory composing a 'VSMsgBuffer'.
// These objects are located in the shared memory page containing 
// outgoing messages;
// They are designed to fit into a 'VS_BLOCKSIZE' block;
class VSMsgChunk {
  friend class VSMsgBuffer;
private:
  int next;			// next chunk if any (and -1 otherwise); 
  volatile Bool busy;		// set by owner and dropped by receiver;
  BYTE buffer[0];		// fills a chunk;

  //
public:
  // The body of the chunk contains garbage initially;
  VSMsgChunk() : next(-1), busy(FALSE) {}
  ~VSMsgChunk() { error("VSMsgChunk deallocated???"); }

  //
  Bool isBusy() { return (busy); }
  void markFree() { busy = FALSE; }
};

class VSMsgChunkPoolManager;
//
// The pool object identifies the pool; it is located (together with 
// data chunks) in the shared memory page (since its 'wakeupMode' and
// 'semkey' are to be shared);
//
class VSMsgChunkPool {
  friend class VSMsgChunkPoolManager;
private:
  int chunkSize, chunksNum;	// used by receiver sites;
  volatile Bool wakeupMode;	// 'OK' if receiver(s) has to increment;
  key_t semkey;			// the id of the 'wakeupMode' semaphor;

  //
public:
  //
  // The first constructor is for the "owner" site,
  // and another - for a "receiver" one;
  VSMsgChunkPool(int chunkSizeIn, int chunksNumIn);
  // There might be some consistency checks, but otherwise it's already
  // initialized (by the owner);
  VSMsgChunkPool() {}
  ~VSMsgChunkPool() { error("VSMsgChunkPool destroyed?"); }

  //
  key_t getSemkey() { return (semkey); }
  int getChunkSize() { return (chunkSize); }
  int getChunkDataSize() { return (chunkSize - sizeof(VSMsgChunk)); }
  int getchunksNum() { return (chunksNum); }
  //
  void startWakeupMode() { wakeupMode = TRUE; }
  void stopWakeupMode() { wakeupMode = FALSE; }
};

//
// Linux has the structure already defined, while others 
// (e.g. Solaris) have not;
#if defined(LINUX)
typedef union semun SemOptArg;
#else
typedef union semun {
  int val;
  struct semid_ds *buf;
  ushort *array;
} SemOptArg;
#endif

//
// The 'Manager' class, like the mailbox's one, is visible to the 
// owner Oz process only. (Actually, there is some redundancy in data);
class VSMsgChunkPoolManager {
private:
  //
  int chunkSize, chunksNum;	// just to remember;
  //
  key_t shmkey;			// for the shared memory;
  int shmid;			// 
  //
  // These three addresses are actually the same;
  // Note that the first chunk is permanently busy;
  void *mem;			// the address of the shm page;
  VSMsgChunkPool *pool;
  VSMsgChunk *chunks;		// data chunks;
  //
  key_t semkey;			// for the "wakeup" semaphore;
  int semid;			// 
  //
  int mapSize;			// chunksNum/32;
  int32 *map;			// the usage bitmap - '1' if free;

  //
private:
  //
  void markFreeInMap(int cn) {
    Assert(cn > 0 && cn < chunksNum);
    int wordNum = cn >> 5;	// 5 bits for a 'int32' value; 
    int bitNum = cn & 0x1f;
    map[wordNum] |= bitNum;
  }
  //
  // find the first free page in the map;
  int allocateFromMap() {
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
	return ((i << 5) | j);	// 5 bits for 'int32' value; 
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
  // ... to be used by the receiver site in the "wakeup" mode;
  void wakeupOwner();

  //
public:
  // There are as well two constructors - for owner and receivers;
  VSMsgChunkPoolManager(int chunkSizeIn, int chunksNumIn);
  VSMsgChunkPoolManager(key_t shmkey);

  //
  void destroy();		// unmap & destroy;
  ~VSMsgChunkPoolManager() {}

  //
  // 'getMsgChunk' tries first to find out an unused chunk in the map,
  // and if it fails - it scavenges for free chunks (and updates the
  // map correspondingly). When no more free chunks are found, it
  // calls the "outline" version which blocks on the semaphore which
  // must be stepped up by a VS that frees a chunk (see the first
  // assertion in the constructor);
  //
  // (to be used by the sender site;)
  int getMsgChunk() {
    // 
    int chunkNum = allocateFromMap();
    //
    if (chunkNum < 0) return (getMsgChunkOutline());
    else return (chunkNum);
  }
  VSMsgChunk *getChunkAddr(int i) { return (&chunks[i]); }
  int getChunkDataSize() { return (chunkSize - sizeof(VSMsgChunk)); }

  //
  // To be used by the receiver site - drop the "busy" flag(s) in its
  // chunk(s) and rises the semaphor if necessary. Note that it does
  // not immediately frees it;
  void releaseChunk(VSMsgChunk *chunk) {
    chunk->markFree();
    if (pool->wakeupMode)
      wakeupOwner();
  }

  //
  key_t getSHMKey() { return (shmkey);	}
};

//
// The register object - it keeps track of imported chunk pools or 
// creates a new one if necessary;
class VSChunkPoolRegister : public GenHashTable {
private:
  unsigned int hash(key_t key);

  //
private:
  VSMsgChunkPoolManager* find(key_t key);
  void add(key_t key, VSMsgChunkPoolManager *pool);

  //
public:
  VSChunkPoolRegister(int size) : GenHashTable(size) {}
  ~VSChunkPoolRegister() {}

  //
  //
  VSMsgChunkPoolManager *import(key_t key) {
    VSMsgChunkPoolManager *aux;

    //
    aux = find(key);
    if (!aux) { 
      aux = new VSMsgChunkPoolManager(key);
      add(key, aux);
    }

    //
    return (aux);
  }
};

//
// The message buffer used for communications between VS"s.
//
// Objects of this class reside in the private VS memory and used 
// for constructing/reading message buffers. These objects are allocated 
// from a special "free list";
//
class VSMsgBuffer : public MsgBuffer {
private:
  // chunks;
  Site *site;
  VSMsgChunkPoolManager *cpm;
  int first, current;		// chunk indexes;
  VSMsgChunk *currentAddr;	// chunk itself;
  // within the current chunk;
  BYTE *end;
  BYTE *ptr;

  //
private:
  // Called whenever 'put' faces no space in the buffer (sender);
  void allocateNextChunk() {
    int next = cpm->getMsgChunk();
    VSMsgChunk *nextAddr = cpm->getChunkAddr(next);

    //
    ptr = &(nextAddr->buffer[0]);
    end = ptr + cpm->getChunkDataSize(); // in bytes;

    // link it up;
    currentAddr->next = next;
    current = next;
    currentAddr = nextAddr;
  }

  //
  // Jump to the next buffer (receiver);
  void gotoNextChunk() {
    current = currentAddr->next;
    Assert(current >= 0);

    //
    currentAddr = cpm->getChunkAddr(current);

    //
    ptr = &(currentAddr->buffer[0]);
    end = ptr + cpm->getChunkDataSize(); // in bytes;
  }

  //
public:
  // There are two constructors for the buffer: for usage at the
  // sender and receiver sites (or, as a variation of the last one, at
  // the sender site when an unsent message needs to be unmarshaled
  // back);
  VSMsgBuffer(VSMsgChunkPoolManager *cpmIn, Site *siteIn);
  VSMsgBuffer(VSMsgChunkPoolManager *cpmIn, int chunkIndex);
  virtual ~VSMsgBuffer() { error("VSMsgBuffer destroyed?"); }

  //
  // Mark (shared) memory chunks free (so the sender could reuse them);
  void releaseChunks() {
    current = first;
    Assert(current >= 0);

    //
    while (current >= 0) {
      currentAddr = cpm->getChunkAddr(current);
      int next = currentAddr->next;

      //
      cpm->releaseChunk(currentAddr);

      //
      current = next;
    }
    Assert(current == -1);

    //
    DebugCode(first = -1);
    DebugCode(ptr = (BYTE *) 0);
    DebugCode(end = (BYTE *) 0);
  }

  //
  // "to be provided" methods;
  void marshalBegin();		// allocates first chunk;
  void marshalEnd() {}		// no special action now;
  void unmarshalBegin() {}	// 
  void unmarshalEnd();		// marks chunks free;

  //
  BYTE get() {
    if (ptr >= end) 
      gotoNextChunk();
    return (*ptr++);
  }
  void put(BYTE byte) {
    if (ptr >= end) 
      allocateNextChunk();
      *ptr++ = byte;
  }

  //
  // Yields the first chunk's address - for putting in a mailbox;
  int getFirstChunk() { return (first); }
  key_t getSHMKey() { return (cpm->getSHMKey()); }

  //
  // Special stuff;
  // ('siteStringrep' exists just for debugging;)
  char* siteStringrep() { return ("virtual site"); }
  Site* getSite() { return (site); }
};

#endif  // __VS_MSGBUFFER_HH
