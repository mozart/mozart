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

#ifndef __VS_MSGBUFFER_HH
#define __VS_MSGBUFFER_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"
#include "dpBase.hh"

#ifdef VIRTUALSITES

#include "genhashtbl.hh"
#include "dsite.hh"
#include "msgbuffer.hh"
#include "vs_aux.hh"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

//
// IDs of IPC keys used for different things (4 bits);
#define VS_KEYTYPE_SIZE      4
#define VS_MSGBUFFER_KEY     0x1
//
#define VS_SEQ_SIZE          12
// altogether - 32 bits, so sizeof(key_t) >= sizeof(int32)
#define VS_PID_SIZE          16
#define VS_PID_MASK          0xffff
//
Bool isLocalKey(key_t key);

//
// Pieces of memory composing a 'VSMsgBuffer'.
// These objects are located in the shared memory page containing 
// outgoing messages;
// They are designed to fit into a 'VS_BLOCKSIZE' block;
class VSMsgChunk {
protected:
  int next;			// next chunk if any (and -1 otherwise); 
  key_t shmKey;			// ... of that (next) chunk;
  volatile Bool busy;		// set by owner and dropped by receiver;
  BYTE buffer[1];		// actually more - fills up the chunk;

  //
public:
  // These objects are unusable;
  VSMsgChunk()  { OZ_error("VSMsgChunk allocated???"); }
  ~VSMsgChunk() { OZ_error("VSMsgChunk deallocated???"); }

  //
  BYTE *getDataAddr() { return (&buffer[0]); }
  int getNext() { return (next); }
  key_t getSHMKey() { return (shmKey); }
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
  void setSHMKey(key_t keyIn) { shmKey = keyIn; }
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
// memory page as a zeroth special "reserved" data chunk;
//
class VSMsgChunkPoolSegment : protected VSMsgChunkReserved {
protected:
  int chunkSize, chunksNum;	// used by receiver sites;
  key_t shmkey;			// for consistency check;

  //
public:
  // These objects are unusable;
  VSMsgChunkPoolSegment() { OZ_error("VSMsgChunkPoolSegment allocated?"); }
  ~VSMsgChunkPoolSegment() { OZ_error("VSMsgChunkPoolSegment destroyed?"); }

  //
#ifdef DEBUG_CHECK
  void checkConsistency() {}
#endif

  //
  int getChunkSize() { DebugCode(checkConsistency()); return (chunkSize); }
  int getChunksNum() { DebugCode(checkConsistency()); return (chunksNum); }
};

//
class VSMsgChunkPoolSegmentOwned : public VSMsgChunkPoolSegment {
public:
  //
  VSMsgChunkPoolSegmentOwned() {
    OZ_error("VSMsgChunkPoolSegmentOwned allocated?");
  }
  ~VSMsgChunkPoolSegmentOwned() {
    OZ_error("VSMsgChunkPoolSegmentOwned destroyed?");
  }

  //
  void init(key_t shmkeyIn, int chunkSizeIn, int chunksNumIn);
};

//
class VSMsgChunkPoolSegmentImported : public VSMsgChunkPoolSegment {
public:
  //
  VSMsgChunkPoolSegmentImported() {
    OZ_error("VSMsgChunkPoolSegmentImported allocated?");
  }
  ~VSMsgChunkPoolSegmentImported() {
    OZ_error("VSMsgChunkPoolSegmentImported destroyed?");
  }

  //
  void init(key_t shmkeyIn) {
    Assert(shmkey == shmkeyIn);
  }
};

//
// The 'Manager' class, like the mailbox's one, is visible to the 
// local Oz process only. (Actually, 'key_t' keys are replicated);
class VSMsgChunkPoolSegmentManager {
protected:
  //
  int chunkSize, chunksNum;	//
  //
  key_t shmkey;			// shared memory;
  int shmid;			// 
  //
  // Pointers ('mem', 'chunks' and 'pool') are actually the same; 
  // Note that the first chunk is permanently busy (reserved);
  void *mem;			// the address of the shm page;

  //
public:
  //
  VSMsgChunkPoolSegmentManager() { 
    DebugCode(chunkSize = chunksNum = 0);
    DebugCode(shmid = 0);
    DebugCode(shmkey = (key_t) 0);
    DebugCode(mem = (void *) 0);
  }
  ~VSMsgChunkPoolSegmentManager() {}
};

//
// A register of virtual sites. This register is a part of a chunk
// pool segment's manager; it contains VSs that supposedly got the
// segment imported.
//
// The register is used whenever a segment is to be destroyed, so
// importers have to detach it as well (otherwise it will stuck in the
// system).
// 
class VSSegmentImportersRegister : public GenHashTable {
private:
  int seqIndex;			// does not need to be initialized;
  GenHashNode *seqGHN;		// both needed for 'getFirst'/'getNext';

  //
private:
  unsigned int hash(VirtualSite *vs);
  //
  VirtualSite* find(VirtualSite *vs);
  void insert(VirtualSite *vs);
  void retract(VirtualSite *vs);

  //
public:
  VSSegmentImportersRegister(int size) : GenHashTable(size) {}
  ~VSSegmentImportersRegister() {}

  //
  void put(VirtualSite *vs) { if (vs && !find(vs)) insert(vs); }
  void forget(VirtualSite *vs) { if (find(vs)) retract(vs); }

  //
  int getRegisterSize() { return (getUsed()); }

  //
  VirtualSite *getFirst();
  VirtualSite *getNext();
};

//
class VSMsgChunkPoolSegmentManagerOwned
  : public VSMsgChunkPoolSegmentManager
{
private:
  //
  VSMsgChunkOwned *chunks;
  VSMsgChunkPoolSegmentOwned *pool;
  //
  long phaseNumber;      // to determine idle segments for GCing;
  //
  FixedSizeStack fs;     // free chunks;
  //
  // (see comment above - before 'VSSegmentImportersRegister'
  // definition);
  VSSegmentImportersRegister importersRegister;

  //
private:
  // (the segment itself;)
  void markDestroy();

  //
  // (The thing referred here as a "map" is actually a stack;)
  void markFreeInMap(int cn) { fs.push(cn); }
  void purgeMap() { fs.purge(); }

  //
public:
  //
  VSMsgChunkPoolSegmentManagerOwned(int chunkSizeIn, int chunksNumIn,
				    int regInitSite);
  ~VSMsgChunkPoolSegmentManagerOwned();

  //
  void deleteAndBroadcast();

  //
  // 'getChunkAddr' is public since the pool manager object has to
  // know both the chunk's number and address;
  VSMsgChunkOwned *getChunkAddr(int i) {
    Assert(pool->getChunksNum() > i);
    Assert(sizeof(VSMsgChunk) == sizeof(VSMsgChunkOwned));
    return ((VSMsgChunkOwned *) (((char *) chunks) + i*chunkSize));
  }

  //
  // 'getMsgChunk' should be applied only to segments that have
  // at least one free chunk;
  int getMsgChunk(VirtualSite *vs) {
    DebugCode(pool->checkConsistency());
    Assert(!fs.isEmpty());

    // 
    int chunkNum = fs.pop();
    getChunkAddr(chunkNum)->init();
    importersRegister.put(vs);

    //
    return (chunkNum);
  }

  //
  // Reclaim chunks already freed (by importers). Returns the number
  // of still used chunks;
  int scavenge();

  //
  void setPhaseNumber(long pn) { phaseNumber = pn; }
  long getPhaseNumber() { return (phaseNumber); }

  //
  key_t getSHMKey() {
    DebugCode(pool->checkConsistency());
    return (shmkey);
  }

  //
  // one chunk is permanently reserved;
  int getChunksNum() { return (pool->getChunksNum()-1); }
  int getAvailChunksNum() { return (fs.getSize()); }

  //
  int getNumOfRegisteredSites() {
    return (importersRegister.getRegisterSize());
  }
  void retractRegisteredSite(VirtualSite *vs) {
    importersRegister.forget(vs);
  }
};

//
class VSMsgChunkPoolSegmentManagerImported
  : public VSMsgChunkPoolSegmentManager {
private:
  VSMsgChunkImported *chunks;
  VSMsgChunkPoolSegmentImported *pool;

  //
public:
  //
  VSMsgChunkPoolSegmentManagerImported(key_t shmkey);
  ~VSMsgChunkPoolSegmentManagerImported();

  //
  Bool isVoid() {
    return (pool == (VSMsgChunkPoolSegmentImported *) -1);
  }
  Bool isNotVoid() {
    return (pool != (VSMsgChunkPoolSegmentImported *) -1);
  }

  //
  VSMsgChunkImported *getChunkAddr(int i) {
    Assert(sizeof(VSMsgChunk) == sizeof(VSMsgChunkImported));
    return ((VSMsgChunkImported *) (((char *) chunks) + i*chunkSize));
  }

  //
  int getChunkSize() { return (pool->getChunkSize()); }
};

//
class SegmentStack : private PtrStackArray {
public:
  //
  int getSize() { return (PtrStackArray::getSize()); }
  void push(VSMsgChunkPoolSegmentManagerOwned *sm) {
    PtrStackArray::push((void*) sm); }
  VSMsgChunkPoolSegmentManagerOwned* pop() {
    return ((VSMsgChunkPoolSegmentManagerOwned *) PtrStackArray::pop());
  }
  VSMsgChunkPoolSegmentManagerOwned* get(int elem) {
    return ((VSMsgChunkPoolSegmentManagerOwned *)
	    // do it so: (*((PtrStackArray *) this))[elem]);
	    // ... or just so:
	    (*this)[elem]);
    // but NOT so:  ((PtrStackArray) *this)[elem]);
    // ... then the object's destructor is called (?)
  }
};

//
// Real interface to the chunks pool at the owner site - it hides the
// internal organization (segments, etc.); 
//
// The manager allocates chunks in "allocation phases", which span
// between scavenger runs. Within a phase, segments are used up
// sequentially. Scavenger sets the number of segments to be used in
// the current allocation phase, which is proportional to the number
// of chunks still in use ('VS_MSGCHUNKS_USAGE' is the proportion
// factor), rounded up to the next full segment. Originally (in the
// very first phase) one segment is to be used;
//
// Scavenger walks through (all) segments with 'scavenge' method,
// counts still used chunks, and finally decides how many segments to
// be used in the next phase. Then it checks not-to-be-used segments,
// and deletes those of them that have not been used for a while
// (VS_SEGS_MAXIDLE_PHASES) & sends corresponding 'M_UNUSED_ID'
// message to those sites that have got them imported;
//
class VSMsgChunkPoolManagerOwned : private SegmentStack {
private:
  int chunkSize, chunksNum; // used by receiver sites;
  int currentSegMgrIndex;   // where allocation takes place;
  int toBeUsedSegments;     // ... in the current allocation phase;
  long phaseNumber;         // allocation phases are numbererd seq"ly;
  unsigned long lastGC;	    // 

  //
private:
  // 
  // Yields the segment manager that is to be used in the next
  // 'getMsgBuffer' operation. This can include allocation of a new
  // one, or(and) scavenging of existing ones. The segment returned
  // contains at least one free chunk;
  VSMsgChunkPoolSegmentManagerOwned* getSegmentManager() {
    VSMsgChunkPoolSegmentManagerOwned *fsm;

    //
    fsm = get(currentSegMgrIndex);
    while (fsm->getAvailChunksNum() == 0) {
      currentSegMgrIndex++;
      if (currentSegMgrIndex < toBeUsedSegments) {
	if (currentSegMgrIndex >= getSize()) {
	  fsm = new VSMsgChunkPoolSegmentManagerOwned(VS_CHUNK_SIZE,
						      VS_CHUNKS_NUM,
						      VS_REGISTER_HT_SIZE);
	  push(fsm);
	  Assert(fsm == get(currentSegMgrIndex));
	} else {
	  fsm = get(currentSegMgrIndex);
	}
      } else {
	scavenge();		// on-demand GCing;

	//
	fsm = get(currentSegMgrIndex);
      }
    }

    //
    fsm->setPhaseNumber(phaseNumber);
    return (fsm);
  }

  //
public:
  //
  VSMsgChunkPoolManagerOwned(int chunkSizeIn, int chunksNumIn, int sizeIn);
  ~VSMsgChunkPoolManagerOwned();

  //
  // Scavenging process - scavenge segments, set 'toBeUsedSegments'
  // and GC segments to be freed;
  void scavenge();

  //
  unsigned long getLastGC() { return (lastGC); }
  void setLastGC(unsigned long t) { lastGC = t; }

  //
  // (for marshaling;)
  VSMsgChunkOwned *getMsgChunk(VirtualSite *vs,
			       key_t &shmKey, int &cn) {
    VSMsgChunkPoolSegmentManagerOwned *fsm = getSegmentManager();

    //
    cn = fsm->getMsgChunk(vs);
    Assert(cn > 0);
    shmKey = fsm->getSHMKey();
    return (fsm->getChunkAddr(cn));
  }

  //
  // (for unmarsahaling at the sender site - a rare operation;)
  VSMsgChunkOwned *getMsgChunk(key_t shmKey, int cn) {
    for (int i = 0; i < getSize(); i++) {
      VSMsgChunkPoolSegmentManagerOwned *fsm = get(i);
      if (shmKey == fsm->getSHMKey())
	return (fsm->getChunkAddr(cn));
    }
    Assert(0);		 // there must be a segment with 'shmKey';
    return ((VSMsgChunkOwned *) 0);     // to make gcc happy;
  }

  //
  // (sender site release - when e.g. the message cannot be sent);
  // (actually, this 'releaseChunk' just calls 'markFree' of a chunk)
  void releaseChunk(VSMsgChunkOwned *chunk) {
    chunk->markFree();
  }

  //
  int getChunkDataSize() {
    Assert(sizeof(VSMsgChunk) == sizeof(VSMsgChunkOwned));
    return (chunkSize - sizeof(VSMsgChunk));
  }

  //
  // A list of ipc keys for a "alive ack" message is assembled using:
  int getSegsNum() { return (getSize()); }
  key_t getSegSHMKey(int i) {
    Assert(i >= 0 && i < getSize());
    VSMsgChunkPoolSegmentManagerOwned *fsm = get(i); 
    return (fsm->getSHMKey());
  }

  //
  // Whenever a VS connection is dropped ('VirtualSite' object is
  // destroyed), we should clean up registries in segment managers...
  void retractRegisteredSite(VirtualSite *vs) {
    for (int i = 0; i < getSize(); i++) {
      VSMsgChunkPoolSegmentManagerOwned *fsm = get(i);
      fsm->retractRegisteredSite(vs);
    }
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
  void add(key_t key, VSMsgChunkPoolSegmentManagerImported *pool);
  void remove(key_t key);

  //
protected:
  VSChunkPoolRegister(int size) : GenHashTable(size) {}
  ~VSChunkPoolRegister() {}

  //
  VSMsgChunkPoolSegmentManagerImported* find(key_t key);

  //
public:
  VSMsgChunkPoolSegmentManagerImported* import(key_t key) {
    VSMsgChunkPoolSegmentManagerImported *aux;

    //
    aux = find(key);
    if (!aux) { 
      aux = new VSMsgChunkPoolSegmentManagerImported(key);
      if (aux->isVoid()) {
	// there is no such shared memory page;
	delete aux;
	return ((VSMsgChunkPoolSegmentManagerImported *) 0);
      } else {
	add(key, aux);
      }
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
// 'VSMsgChunkPoolManagerImported' basically keeps track of imported
// chunk pool segments (that keep incoming messages).
class VSMsgChunkPoolManagerImported : private VSChunkPoolRegister {
public:
  VSMsgChunkPoolManagerImported(int sizeIn) 
    : VSChunkPoolRegister(sizeIn) {}
  // do nothing when deleting: OS will unmap everithing itself; 
  ~VSMsgChunkPoolManagerImported() {}

  //
  // Note that 'getSegmentManager()' can fail all the time - a lot 
  // of things can really happen...
  VSMsgChunkPoolSegmentManagerImported* getSegmentManager(key_t shmKey) {
    return (import(shmKey));
  }
  VSMsgChunkImported *getMsgChunk(VSMsgChunkPoolSegmentManagerImported* fsm,
				  int cn) {
    return (fsm->getChunkAddr(cn));
  }

  //
  void removeSegmentManager(key_t shmKey) {
    VSMsgChunkPoolSegmentManagerImported* sm = find(shmKey);
    if (sm) {
      forget(shmKey);
      delete sm;
    }
  }

  //
  // Segments with different chunk sizes can be imported, so 
  // we ask every particular segment manager about its size:
  int getChunkDataSize(VSMsgChunkPoolSegmentManagerImported* fsm) {
    Assert(sizeof(VSMsgChunk) == sizeof(VSMsgChunkImported));
    return (fsm->getChunkSize() - sizeof(VSMsgChunk));
  }

  //
  void releaseChunk(VSMsgChunkImported *chunk) {
    chunk->markFree();
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
  int firstChunkNum;
  key_t firstSegSHMKey;
  // 'BYTE *posMB' and BYTE *endMB' are inherited from 'MsgBuffer';

  //
public:
  VSMsgBuffer() {
    DebugCode(firstChunkNum = -1);
    DebugCode(firstSegSHMKey = (key_t) -1);
    // Ralf Scheidhauer' liebe fuer 'init()' anstatt class
    // constructors, $%@#$# @#$ #@$*@#$ $#$$*@&#*$* &@ $@#^@#
    // $**#& !!!!!!!!!!!!!
    // RS: nein, da ist Per dran schuld: siehe bufferManager->getByteStream();
    MsgBuffer::init();
    // this should be in MsgBuffer's constructor (or initializer, 
    // whichever is preferred);
    DebugCode(posMB = endMB = (BYTE *) 0);
  }
  VSMsgBuffer(key_t shmKey, int chunkIndex) 
    : firstChunkNum(chunkIndex), firstSegSHMKey(shmKey) {
    // Ralf Scheidhauer' liebe fuer 'init()' anstatt class
    // constructors, $%@#$# @#$ #@$*@#$ $#$$*@&#*$* &@ $@#^@#
    // $**#& !!!!!!!!!!!!!
    // RS: nein, da ist Per dran schuld: siehe bufferManager->getByteStream();
    MsgBuffer::init();
    // this should be in MsgBuffer's constructor (or initializer, 
    // whichever is preferred);
    DebugCode(posMB = endMB = (BYTE *) 0);
  }
  virtual ~VSMsgBuffer() { OZ_error("VSMsgBuffer destroyed?"); }

  //
  // Various special (weird) stuff;
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
  DSite *site;			// has to know the site when marshaling;
  VirtualSite *vs;		// cached;
  VSMsgChunkPoolManagerOwned *cpm;
  VSMsgChunkOwned *currentAddr;	// of the current chunk;
  int allocatedChunks;		// keep track of size;

  //
private:
  //
  // Called whenever 'put' faces no space in the buffer;
  void allocateNextChunk() {
    int chunkNum;
    key_t segSHMKey;
    VSMsgChunkOwned *nextAddr;

    //
    // 'getMsgChunk(VirtualSite*)' ultimately gets new chunks
    // (including waiting in there if needed);
    nextAddr = cpm->getMsgChunk(vs, segSHMKey, chunkNum);
    allocatedChunks++;

    //
    posMB = nextAddr->getDataAddr();
    endMB = posMB + cpm->getChunkDataSize() - 1; // in bytes; // kost@ : TODO

    //
    // link it up;
    currentAddr->setNext(chunkNum);
    currentAddr->setSHMKey(segSHMKey);
    //
    currentAddr = nextAddr;
  }

  //
  // Jump to the next buffer (when unmarshaling upon sending failure);
  void gotoNextChunk() {
    int nextNum = currentAddr->getNext();
    key_t nextKey = currentAddr->getSHMKey();
    Assert(nextNum >= 0);

    //
    currentAddr = cpm->getMsgChunk(nextKey, nextNum);

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
    OZ_error("VSMsgBufferOwned allocated using 'new(size_t)'");
    return ((void *) -1);	// gcc warning;
  }
  void* operator new(size_t, void *place) { return (place); }

  //
  // kost@ : Note the 'owned' buffer is initialized when created
  // since virtual sites put (at least) a "vs" header into it;
  VSMsgBufferOwned(VSMsgChunkPoolManagerOwned *cpmIn, DSite *siteIn)
    : cpm(cpmIn), site(siteIn) { 
    if (siteIn) {
      vs = siteIn->getVirtualSite();
      Assert(vs);
    } else {
      vs = (VirtualSite *) 0;
    }

    //
    Assert(firstChunkNum < 0);
    // allocate the first chunk unconditionally
    // (there are no empty messages?)
    currentAddr = cpm->getMsgChunk(vs, firstSegSHMKey, firstChunkNum);
    allocatedChunks = 1;

    //
    posMB = currentAddr->getDataAddr();
    endMB = posMB + cpm->getChunkDataSize() - 1; // in bytes; // kost@ : TODO
  }
  virtual ~VSMsgBufferOwned() {
    OZ_error("VSMsgBufferOwned destroyed?");
  }

  //
  // Mark (shared) memory chunks free - when unmarshaling an unsent
  // message back;
  void releaseChunks() {
    Assert(firstChunkNum >= 0);
    int currentNum = firstChunkNum;
    key_t currentKey = firstSegSHMKey;
    do {
      VSMsgChunkOwned *chunkAddr = 
	cpm->getMsgChunk(currentKey, currentNum);
      currentNum = chunkAddr->getNext();
      currentKey = chunkAddr->getSHMKey();
      cpm->releaseChunk(chunkAddr);
    } while (currentNum >= 0);
    Assert(currentNum == -1);

    //
    DebugCode(firstChunkNum = -1);
    DebugCode(firstSegSHMKey = (key_t) -1);
    DebugCode(currentAddr = (VSMsgChunkOwned *) 0);
    DebugCode(posMB = endMB = (BYTE *) 0);
  }

  //
  // ... hand them over (to a receiver);
  void passChunks() {
    Assert(firstChunkNum >= 0);
    //
    DebugCode(firstChunkNum = -1);
    DebugCode(firstSegSHMKey = (key_t) -1);
    DebugCode(currentAddr = (VSMsgChunkOwned *) 0);
    DebugCode(posMB = endMB = (BYTE *) 0);
  }

  //
  // (Consistency check - must be released/passed already:)
  void cleanup() {
    Assert(firstChunkNum == -1);
    Assert(firstSegSHMKey == (key_t) -1);
    Assert(currentAddr == (VSMsgChunk *) 0);
    Assert(posMB == (BYTE *) 0);
    Assert(endMB == (BYTE *) 0);
  }

  //
  // "to be provided" methods;
  void marshalBegin() {}	// already initialized;
  void marshalEnd() {}		// no special action now;
  // already initialized (allocated) but pointers are not set:
  void unmarshalBegin() {
    currentAddr = cpm->getMsgChunk(firstSegSHMKey, firstChunkNum);
    posMB = currentAddr->getDataAddr();
    endMB = posMB + cpm->getChunkDataSize(); // in bytes;
  }
  void unmarshalEnd() {}        // but the chunks need to be released yet;

  //
  // Yields the first chunk's address - for putting it in a mailbox;
  int getFirstChunkNum() { return (firstChunkNum); }
  key_t getFirstChunkSHMKey() { return (firstSegSHMKey); }

  //
  // ('siteStringrep' exists just for debugging;)
  char* siteStringrep();
  // The reasons for having 'getSite()' are:
  // (a) the marshaling routine has to know sometimes where the 
  //     stuff is to be sent;
  // (b) while discarding unsent messages, we have to know the type
  //     of each particular buffer;
  DSite* getSite() { return (site); }
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
    VSMsgChunkPoolSegmentManagerImported *fsm;
    int nextNum = currentAddr->getNext();
    key_t nextKey = currentAddr->getSHMKey();
    Assert(nextNum >= 0);

    //
    fsm = cpm->getSegmentManager(nextKey);
    // because all the segments are feeded in already:
    Assert(fsm);
    currentAddr = cpm->getMsgChunk(fsm, nextNum);

    //
    posMB = currentAddr->getDataAddr();
    endMB = posMB + cpm->getChunkDataSize(fsm); // in bytes;
  }

  //
private:
  //
  BYTE getNext() {
    Assert(posMB == endMB);
    gotoNextChunk();
    return (*posMB++);
  }
  void putNext(BYTE) { 
    OZ_error("there is no 'putNext' for VSMsgBufferImported!");
  }

  //
public:
  //
  void* operator new(size_t size) {
    OZ_error("VSMsgBufferImported allocated using 'new(size_t)'");
    return ((void *) -1);	// gcc warning;
  }
  void* operator new(size_t, void *place) { return (place); }

  //
  // kost@ : Note the 'imported' buffer is initialized when created
  // (imported) since virtual sites put (at least) a "vs" header into
  // it;
  VSMsgBufferImported(VSMsgChunkPoolManagerImported *cpmIn,
		      key_t shmKey, int chunkIndex)
    : VSMsgBuffer(shmKey, chunkIndex), cpm(cpmIn) {
    DebugCode(currentAddr = (VSMsgChunkImported *) 0);
    DebugCode(posMB = (BYTE *) 0);
    DebugCode(endMB = (BYTE *) 0);
  }
  virtual ~VSMsgBufferImported() {
    OZ_error("VSMsgBufferImported destroyed?");
  }

  //
  Bool isVoid() { return (currentAddr == (VSMsgChunkImported *) -1); }
  Bool isNotVoid() { return (currentAddr != (VSMsgChunkImported *) -1); }

  //
  // Mark (shared) memory chunks free (so the sender could reuse them);
  void releaseChunks() {
    Assert(currentAddr == (VSMsgChunkImported *) 0);
    Assert(posMB == (BYTE *) 0);
    Assert(endMB == (BYTE *) 0);
    Assert(isNotVoid());
    Assert(firstChunkNum >= 0);
    int currentNum = firstChunkNum;
    key_t currentKey = firstSegSHMKey;
    do {
      VSMsgChunkPoolSegmentManagerImported *fsm;

      //
      fsm = cpm->getSegmentManager(currentKey);

      //
      if (!fsm) {
	// (one of) the chunk is missing - i.e. its owner is gone;
	DebugCode(currentNum = -1;);
	break;
      }

      //
      VSMsgChunkImported *chunkAddr = cpm->getMsgChunk(fsm, currentNum);
      currentNum = chunkAddr->getNext();
      currentKey = chunkAddr->getSHMKey();

      //
      cpm->releaseChunk(chunkAddr);
    } while (currentNum >= 0);
    Assert(currentNum == -1);

    //
    DebugCode(firstChunkNum = -1);
    DebugCode(firstSegSHMKey = (key_t) -1);
  }

  //
  void dropVoid() {
    DebugCode(firstChunkNum = -1);
    DebugCode(firstSegSHMKey = (key_t) -1);
    DebugCode(currentAddr = (VSMsgChunkImported *) 0);
    DebugCode(posMB = endMB = (BYTE *) 0);
  }

  //
  // Consistency check - must be released already:
  void cleanup() {
    Assert(firstChunkNum == -1);
    Assert(firstSegSHMKey == (key_t) -1);
    Assert(currentAddr == (VSMsgChunkImported *) 0);
    Assert(posMB == (BYTE *) 0);
    Assert(endMB == (BYTE *) 0);
  }

  //
  // "to be provided" methods;
  void marshalBegin() { OZ_error("VSMsgBufferImported::marshalBegin?"); }
  void marshalEnd()   { OZ_error("VSMsgBufferImported::marshalBegin?"); }
  // 
  void unmarshalBegin() {
    VSMsgChunkPoolSegmentManagerImported *fsm;

    //
    fsm = cpm->getSegmentManager(firstSegSHMKey);
    if (fsm) {
      currentAddr = cpm->getMsgChunk(fsm, firstChunkNum);
      //
      posMB = currentAddr->getDataAddr();
      endMB = posMB + cpm->getChunkDataSize(fsm); // in bytes;

      //
      // pre-fetch all segments. Otherwise, unmarshaling process could
      // break: it cannot be interrupted if some data to be read is
      // unavailable; it requires that the whole thing can be read;
      VSMsgChunkImported *tmpAddr = currentAddr;
      int nextNum;
      //
      while ((nextNum = tmpAddr->getNext()) >= 0) {
	key_t nextKey = tmpAddr->getSHMKey();
	VSMsgChunkPoolSegmentManagerImported *fsm = 
	  cpm->getSegmentManager(nextKey);

	//
	if (fsm) {
	  // going ahead:
	  tmpAddr = cpm->getMsgChunk(fsm, nextNum);
	} else {
	  // ... or premature EOF - mark the whole thing as void;
	  currentAddr = (VSMsgChunkImported *) -1;
	  DebugCode(posMB = endMB = (BYTE *) -1;);
	  break;
	}
      }
    } else {
      currentAddr = (VSMsgChunkImported *) -1;
      DebugCode(posMB = endMB = (BYTE *) -1;);
    }
  }
  // and the chunks need to be released:
  void unmarshalEnd() {
    DebugCode(currentAddr = (VSMsgChunkImported *) 0);
    DebugCode(posMB = endMB = (BYTE *) 0);
  }

  //
  // ('siteStringrep' exists just for debugging;)
  char* siteStringrep();

  //
  DSite* getSite() { 
    OZ_error("there is no 'getSite' method for VSMsgBufferImported!");
    return ((DSite *) -1);
  }
};

#endif // VIRTUALSITES

#endif  // __VS_MSGBUFFER_HH
