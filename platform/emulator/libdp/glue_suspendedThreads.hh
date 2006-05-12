/*
 *  Authors:
 *    Zacharias El Banna, 2002
 *    Erik Klintskog, 2002
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Zacharias El Banna, 2002
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

#ifndef __GLUE_SUSPENDEDTHREADS_HH
#define __GLUE_SUSPENDEDTHREADS_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "value.hh"
#include "glue_mediators.hh"
#include "glue_tables.hh"
#include "dss_classes.hh"


/*
  A SuspendedOperation object is a "thread mediator" for the DSS, and
  is in charge of resuming a distributed operation in the emulator.
  It is given as a thread mediator to a DssThreadId, and is called
  back when the DSS operation has completed.  It completes the
  operation in the emulator, and releases the DssThreadId.

  A distributed operation is often initiated by an Oz thread.
  However, for the sake of simplicity, we do not mix DssThreadIds with
  Oz threads.  An Oz thread is always resumed by means of a control
  variable.  The corresponding SuspendedOperation binds the control
  variable, and releases the DssThreadId.

  Note.  A new SuspendedOperation automatically makes itself the
  thread mediator of the current DssThreadId, and forces the renewal
  of the current DssThreadId.

 */



// return the current DssThreadId
DssThreadId* currentThreadId();

// Those ones are called automatically by SuspendedOperations...
// set the ThreadMediator of the current DssThreadId (which is renewed)
void setCurrentThreadMediator(ThreadMediator* tm);
// release a DssThreadId after usage
void releaseThreadId(DssThreadId* tid);



// garbage collect SuspendedOperation objects, and DssThreadIds too
void gCollectSuspendedOperations();



// generic class SuspendedOperation
class SuspendedOperation : public ThreadMediator {
  friend void gCollectSuspendedOperations();

protected:
  TaggedRef ctlVar;           // control var for thread synchronization
  Mediator* mediator;         // entity mediator
  DssThreadId* threadId;      // which DssThreadId is mediated
  SuspendedOperation *next;   // used for garbage collection

  // suspend the current thread on the control variable
  void suspend();
  
  // resume the suspended thread (synchronize ctlVar)
  void resume();
  void resumeRaise(TaggedRef exc);
  void resumeUnify(TaggedRef a, TaggedRef b);

public:
  SuspendedOperation(Mediator*);
  Mediator* getMediator() { return mediator; }
  bool gc();        // returns TRUE and collect stuff if needed
  
  // inherited from ThreadMediator
  virtual WakeRetVal resumeDoLocal(DssOperationId*) = 0; 
  virtual WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin) = 0;
  virtual WakeRetVal resumeFailed();

  // returns TRUE and collect stuff if needed
  virtual bool gCollect() = 0;
};


// dummy suspended operation (nothing to do for resume)
class SuspendedDummy : public SuspendedOperation {
public:
  SuspendedDummy();
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  bool gCollect();
};


// suspended cell operations
class SuspendedCellAccess: public SuspendedOperation {
private:
  OZ_Term result;   // must be an Oz variable
public:
  SuspendedCellAccess(Mediator*, OZ_Term);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  bool gCollect();
};

class SuspendedCellExchange: public SuspendedOperation {
private:
  OZ_Term newValue, result;
public:
  SuspendedCellExchange(Mediator*, OZ_Term, OZ_Term);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  bool gCollect();
};


// suspended lock operations
class SuspendedLockTake: public SuspendedOperation {
private:
  TaggedRef ozthread;
public:
  SuspendedLockTake(Mediator*, TaggedRef);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  bool gCollect();
};

class SuspendedLockRelease: public SuspendedOperation {
private:
  TaggedRef ozthread;
public:
  // Note: we do not suspend the requesting thread!
  SuspendedLockRelease(Mediator*, TaggedRef);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  bool gCollect();
};


// suspended array operations
class SuspendedArrayGet: public SuspendedOperation {
private:
  int index;
  OZ_Term result;
public:
  SuspendedArrayGet(Mediator*, int idx, OZ_Term);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  bool gCollect();
};

class SuspendedArrayPut: public SuspendedOperation {
private:
  int index;
  OZ_Term value;
public:
  SuspendedArrayPut(Mediator*, int idx, OZ_Term);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  bool gCollect();
};


// suspended dictionary operations
class SuspendedDictionaryGet: public SuspendedOperation {
private:
  OZ_Term key;
  OZ_Term result;
public:
  SuspendedDictionaryGet(Mediator*, OZ_Term, OZ_Term);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  bool gCollect();
};

class SuspendedDictionaryPut: public SuspendedOperation {
private:
  OZ_Term key;
  OZ_Term value;
public:
  SuspendedDictionaryPut(Mediator*, OZ_Term, OZ_Term);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  bool gCollect();
};


#endif 
