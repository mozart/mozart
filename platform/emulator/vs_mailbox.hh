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

#ifndef __VS_MAILBOXHH
#define __VS_MAILBOXHH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"
#include "genhashtbl.hh"
#include "vs_lock.hh"

#ifdef VIRTUALSITES
#include <sys/ipc.h>
#include <sys/shm.h>
#else
#define key_t int
#endif


//
#define VS_MAILBOX_KEY       0x2

//
// The mailbox for Virutal Sites in the Distributed Oz.
// 
// The virtual site's mailbox serve for sending messages to that site
// from neighbor virtual sites. 
// 
// The mailbox, together with its content, is placed in a shared
// memory page, and, therefore, needs protection by a lock. A mailbox
// for a slave site is created by its master (so, the master is able 
// to send it a message);
// 
// Note that there is a single mailbox that collects all the
// (incoming) messages. Thus, communication to neighbor sites is
// connectionless and do not need to be monitored for aliveness (at
// this level), like what is done for remote sites (have a look at
// 'tcpPreReadHandler()');

//
class VSMailboxMsg {
  friend class VSMailbox;
private:
  key_t msgBufferKey;		// key of a sm page with the message;
  int chunkNumber;		// ... in the message buffer;
public:
};

//
// kost@ : i just have no idea right now how large this number should
// be ...
#define	MAX_MSGS_ATONCE		1024

//
class VSMailbox : private LockObj {
private:
  //
  int memSize;			// in bytes;
  // for symmetry reasons: keep the shared memory key here, 
  // like in message buffers:
  key_t shmkey;

  //
  // queue (unfortunately, it cannot be moved out since then we cannot
  // inherit from such a 'queue' class (because of the var-size 'msgs'
  // array);
  int maxSize;
  int size, head, tail;
  VSMailboxMsg msgs[0];		// ... but more in the reallity;

  //
public:
  //
  // This is the constructor for initializing the mailbox, what happens
  // at the master site;
  VSMailbox(key_t keyIn, int memSizeIn, int sizeIn)
    : shmkey(keyIn), memSize(memSizeIn),
      maxSize(sizeIn), size(0), head(0), tail(sizeIn)
  {
    // The current implementation allows the queue of arbitrary size;
    // #ifdef DEBUG_CHECK
    //     // check whether sizeIn is a power of 2;
    //     int bits = 0;
    //     for (int i = 0; i < 32; i++) {
    //       int bit = 0x1 << i;
    //       if (sizeIn & bit) bits++;
    //     }
    //     if (bits != 1) error("VSMailbox's size must be a power of 2");
    // #endif
  }

  //
  // This constructor is called at the VS's site (owner, but not
  // creator (see above)), and can be used for consistency checks:
  VSMailbox() {}
  ~VSMailbox() { error("VSMailbox deallocated?"); }

  //
  int getMemSize() { return (memSize); }

  //
  // The 'enqueue' method is used by the sender site. This method 
  // may not lock the object for a while. However, it could happen
  // that a sender site is preempted while holding the lock - in that
  // case dequeuing methods should skip;
  //
  // 'key' is a system- (e.g. unix) wide key for the shared memory
  // page, and 'offset' - the chunk number within that page; 'TRUE'
  // means successful, and 'FALSE' means that the object is locked
  // or the queue is full now;
  Bool enqueue(key_t msgBufferKey, int chunkNumber) {
    if (tryToLock()) {
      Assert(isLocked());

      //
      if (size == maxSize) { 
	// If it's full, then act like it were locked - just try
	// it later;
	return (FALSE);
      } else {
	tail++;
	if (tail == maxSize) tail = 0;
	msgs[tail].msgBufferKey = msgBufferKey;
	msgs[tail].chunkNumber = chunkNumber;
	size++;

	//
	unlock();
	return (TRUE);
      }
    } else {
      return (FALSE);
    }
  }

  // 
  // 'getSize' yields a snapshot of the size; Observe that this is
  // safe since only receiver (owner) is using it (& 'dequeue');
  int getSize() { return (size); }
  Bool isNotEmpty() { return (size); }

  //
  // The return value says whether dequeuing was successful or not, 
  // similarly to the 'enqueue' method;
  Bool dequeue(key_t &msgBufferKey, int &chunkNumber) {
    if (tryToLock()) {
      Assert(isLocked());

      //
      msgBufferKey = msgs[head].msgBufferKey;
      chunkNumber = msgs[head].chunkNumber;
      head++;
      if (head == maxSize) head = 0;
      size--;

      //
      unlock();
      return (TRUE);
    } else {
      DebugCode(msgBufferKey = (key_t) -1);
      DebugCode(chunkNumber = (int) -1);
      return (FALSE);
    }
  }
};

//
// (other keys in vs_msgbuffer.hh;)
#define VS_MAILXBOX_KEY      0x2

// 
// The 'VSMailboxManager' structure keeps the per-procees
// identification (number) of the mailbox's semaphore. Objects of this
// class are allocated in a private site's memory;
class VSMailboxManager {
private:
  int memSize;			// in bytes;
  //
  key_t shmkey;			// for the mailbox itself;
  int shmid;			// 
  void *mem;			// just a pointer;
  //
  // A virtual site knows its VSMailboxManager only;
  VSMailbox *mbox;

  //
public:
  // 
  // As for the message buffer class, there are two constructors:
  // one for the receiver site (owner), and another - for senders:
  VSMailboxManager(long memSizeIn); // 'long' because of data type clash;
#ifdef VIRTUALSITES
  VSMailboxManager(key_t shmkeyIn);
#endif

  //
  // 'delete' just unmaps the shm page, and 'destroy' deletes it from
  // the system (the later one is to be used by the owner only);
  void unmap();			// unmap;
  void destroy();		// unmap & destroy;
  ~VSMailboxManager() {}

  //
  VSMailbox *getMailbox() { return (mbox); }
  key_t getSHMKey() { return (shmkey); }

  //
  // The mailbox manager class just keeps the mailbox object, which,
  // in turn, provides for desired functionality (queuing/dequeuing);
};

//
// The register object for imported mailboxes - similar to the 
// 'VSChunkPoolRegister' (see vs_msgbuffer.hh);
class VSMailboxRegister : public GenHashTable {
private:
  unsigned int hash(key_t key);

  //
private:
  VSMailboxManager* find(key_t key);
  void add(key_t key, VSMailboxManager *pool);

  //
public:
  VSMailboxRegister(int size) : GenHashTable(size) {}
  ~VSMailboxRegister() {}

  //
  //
  VSMailboxManager *import(key_t key) {
    VSMailboxManager *aux;

    //
    aux = find(key);
    if (!aux) { 
      aux = new VSMailboxManager(key);
      add(key, aux);
    }

    //
    return (aux);
  }

  //
  void unregister(VSMailboxManager *mbm) {
    key_t shmkey = mbm->getSHMKey();
    int hvalue = hash(shmkey);
    GenHashNode *ghn;
    Assert(find(shmkey) == mbm);

    //
    GenCast(mbm, VSMailboxManager*, ghn, GenHashNode*);
    htSub(hvalue, ghn);
  }
};

#endif /* __VS_MAILBOXHH */
