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

#ifdef VIRTUALSITES

#include "genhashtbl.hh"
#include "vs_lock.hh"
#include "os.hh"

#include <sys/ipc.h>
#include <sys/shm.h>

//
#define VS_MAILBOX_KEY       0x2

//
// The mailbox for Virutal Sites in the Distributed Oz.
//
// The virtual site's mailbox serve for sending messages to that site
// from neighbor virtual sites.
//
// The mailbox, together with its content, is placed in a shared
// memory page, and, therefore, needs protection by a lock. 'Owned'
// mailboxes are read, and "imported" are filled in. A mailbox for a
// slave site is created by its master (so, the master is able to send
// it a message);
//
// Note that there is a single mailbox that collects all the
// (incoming) messages. Thus, communication to neighbor sites is
// connectionless and do not need to be monitored for aliveness (at
// this level), like what is done for remote sites (have a look at
// 'tcpPreReadHandler()');

//
class VSMailboxMsg {
  friend class VSMailboxOwned;
  friend class VSMailboxImported;
private:
  key_t msgBufferKey;           // key of a sm page with the message;
  int chunkNumber;              // ... in the message buffer;
public:
};

//
// kost@ : i just have no idea right now how large this number should
// be ...
#define MAX_MSGS_ATONCE         1024

//
class VSMailbox : protected LockObj {
protected:
  //
  int memSize;                  // in bytes;
  int pid;                      // of the owner; 0 says "not yet known";
  //
  // queue (unfortunately, it cannot be moved out since then we cannot
  // inherit from such a 'queue' class (because of the var-size 'msgs'
  // array);
  int maxSize;
  int size, head, tail;
  VSMailboxMsg msgs[1];         // ... but more in the reallity;

  //
public:
  //
  VSMailbox() { error("VSMailbox allocated?"); }
  ~VSMailbox() { error("VSMailbox deallocated?"); }

  //
  int getMemSize() { return (memSize); }

  //
  // 'getSize' yields a snapshot of the size; Observe that this is
  // safe since only receiver (owner) is using it (& 'dequeue');
  int getSize() { return (size); }
  Bool isNotEmpty() { return (size); }
};

//
class VSMailboxOwned : public VSMailbox {
public:
  //
  // (and consistency checks!)
  void init(int pidIn) {
    // 'VSMailboxOwned' and 'VSMailboxImported' are just two
    // interfaces to the 'VSMailbox';
    Assert(sizeof(VSMailboxOwned) == sizeof(VSMailbox));
    //
    pid = pidIn;
  }

  //
  // The return value says whether dequeuing was successful or not,
  // similarly to the 'enqueue' method;
  Bool dequeue(key_t &msgBufferKey, int &chunkNumber) {
    Assert(isNotEmpty());
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
class VSMailboxImported : public VSMailbox {
public:
  //
  // (can be used for consistency checks;)
  void init() {
    // 'VSMailboxOwned' and 'VSMailboxImported' are just two
    // interfaces to the 'VSMailbox';
    Assert(sizeof(VSMailboxImported) == sizeof(VSMailbox));
  }

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

        //
        // The 'USR2' must be sent at least after increasing the size -
        // since 'checkVSMessages' does not lock the mailbox;
        if (pid)
          oskill(pid, SIGUSR2);

        //
        return (TRUE);
      }
    } else {
      return (FALSE);
    }
  }
};

//
// A newly created mailbox (for a slave VS) gets an initial message in
// it (M_INIT_VS), so it should be able also to behave as an imported
// one;
//
// A mailbox created for the site itself ('myVSMailbox(Manager)')
// requires special treatment: it is created first as an "created"
// and immediately after that re-created as "owned";
class VSMailboxCreated : public VSMailboxImported {
public:
  //
  void init(int memSizeIn, int sizeIn) {
    Assert(sizeof(VSMailboxCreated) == sizeof(VSMailbox));

    //
    memSize = memSizeIn;
    pid = 0;
    maxSize = sizeIn;
    size = 0;
    head = 0;
    tail = sizeIn-1;

    // check boundaries;
    DebugCode(char *fb = (char *) this);
    DebugCode(char *lb = fb + memSizeIn - 1);
    Assert(*fb || !*fb);
    Assert(*lb || !*lb);
  }
};

//
// (other keys in vs_msgbuffer.hh;)
#define VS_MAILXBOX_KEY      0x2


//
// The mailbox manager class just keeps the, which,
// in turn, provides for desired functionality (queuing/dequeuing);
//
// The 'VSMailboxManager*' keep the mailbox object together with the
// per-procees identification of the shared memory page.  Enqueuing &
// dequeuing etc. functionality is provided by mailboxes themselves.
// 'VSMailboxManager*' objects are allocated in the private site's
// memory;
class VSMailboxManager {
protected:
  int memSize;                  // in bytes;
  //
  key_t shmkey;                 // for the mailbox itself;
  int shmid;                    //
  //
  void *mem;                    // just a pointer;

  //
public:
  //
  VSMailboxManager() {}
  ~VSMailboxManager() {}

  //
  key_t getSHMKey() { return (shmkey); }
};

//
// The 'VSMailboxManager' structure keeps the per-procees
// identification (number) of the mailbox's semaphore. Objects of this
// class are allocated in a private site's memory;
class VSMailboxManagerOwned : public VSMailboxManager {
private:
  //
  // A virtual site knows its VSMailboxManager only;
  VSMailboxOwned *mbox;

  //
private:
  void markDestroy();           // mark for destruction;

  //
public:
  //
  // The 'owned' mailbox is created at the peer site, so here a key
  // of its shared memory page is passed;
  VSMailboxManagerOwned(key_t shmkeyIn);
  // Should at least unmap it before deleting;
  ~VSMailboxManagerOwned() {
    Assert(mbox == (VSMailboxOwned *) 0);
    Assert(mem == (void *) 0);
    Assert(memSize == -1);
  }

  //
  // 'unmap' just unmaps the shm page, 'destroy' unmaps & marks it for
  // destroying (may be again?), and the class destructor just does
  // nothing (checks consistency);
  void unmap();                 // unmap;
  void destroy();               // unmap & mark;

  //
  VSMailboxOwned *getMailbox() { return (mbox); }
};

//
// The 'VSMailboxManager' structure keeps the per-procees
// identification (number) of the mailbox's semaphore. Objects of this
// class are allocated in a private site's memory;
class VSMailboxManagerImported : public VSMailboxManager {
private:
  //
  // A virtual site knows its VSMailboxManager only;
  VSMailboxImported *mbox;

  //
public:
  //
  // 'Imported' mailboxes are initialized by supplying their shared
  // memory key;
  VSMailboxManagerImported(key_t shmkeyIn);
  ~VSMailboxManagerImported() {
    Assert(mbox == (VSMailboxImported *) 0);
    Assert(mem == (void *) 0);
    Assert(memSize == -1);
  }

  //
  // Importers can only unmap it - but not destruct;
  void unmap();

  //
  VSMailboxImported *getMailbox() { return (mbox); }
};

//
// There is one constructor more - the "proper" one;
class VSMailboxManagerCreated : public VSMailboxManager {
private:
  //
  // A virtual site knows its VSMailboxManager only;
  VSMailboxCreated *mbox;

  //
public:
  //
  VSMailboxManagerCreated(int memSizeIn);
  void unmap();
  ~VSMailboxManagerCreated() {
    Assert(mbox == (VSMailboxCreated *) 0);
    Assert(mem == (void *) 0);
    Assert(memSize == -1);
  }

  //
  VSMailboxCreated *getMailbox() { return (mbox); }
};


//
// 'markDestroy(key_t shmkey)' tries to destroy a shm page which
// could survive potential faults of a virtual site;
void markDestroy(key_t shmkey);

//
// The register object for imported mailboxes - similar to the
// 'VSChunkPoolRegister' (see vs_msgbuffer.hh);
class VSMailboxRegister : public GenHashTable {
private:
  unsigned int hash(key_t key);

  //
private:
  VSMailboxManagerImported* find(key_t key);
  void add(key_t key, VSMailboxManagerImported *pool);

  //
public:
  VSMailboxRegister(int size) : GenHashTable(size) {}
  ~VSMailboxRegister() {}

  //
  //
  VSMailboxManagerImported *import(key_t key) {
    VSMailboxManagerImported *aux;

    //
    aux = find(key);
    if (!aux) {
      aux = new VSMailboxManagerImported(key);
      add(key, aux);
    }

    //
    return (aux);
  }

  //
  void unregister(VSMailboxManagerImported *mbm) {
    key_t shmkey = mbm->getSHMKey();
    int hvalue = hash(shmkey);
    GenHashNode *ghn;
    Assert(find(shmkey) == mbm);

    //
    GenCast(mbm, VSMailboxManagerImported*, ghn, GenHashNode*);
    htSub(hvalue, ghn);
  }
};

#endif // VIRTUALSITES

#endif /* __VS_MAILBOXHH */
