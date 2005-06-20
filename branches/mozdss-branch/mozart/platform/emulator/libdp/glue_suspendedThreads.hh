/*
 *  Authors:
 *    Zacharias El Banna, 2002
 *    Erik Klintskog, 2002
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

#include "glue_tables.hh"
#include "dss_classes.hh"
#include "value.hh"
#include "glue_mediators.hh"


void gCollectSuspThreads(); 

class SuspendedThread: public ThreadMediator
{
public:
  OZ_Term a_cntrlVar;
  Mediator *a_mediator; 
  SuspendedThread *a_next;
public:
  SuspendedThread(Mediator *m);
  
  OZ_Return suspend();
  OZ_Return resume();
  
  virtual WakeRetVal resumeDoLocal(DssOperationId*) = 0; 
  virtual WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin) = 0;
  
  bool gc();
  virtual bool gCollect() = 0; 
  Mediator* getMediator(){
    return a_mediator; 
  }
};



class SuspendedVarBind:public SuspendedThread{
public:
  OZ_Term a_val; 
  SuspendedVarBind(OZ_Term, Mediator*);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  virtual bool gCollect();
};

class SuspendedCellAccess:public SuspendedThread{
public:
  OZ_Term a_var; 
  SuspendedCellAccess(Mediator*, OZ_Term);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  virtual bool gCollect();
};


class SuspendedCellExchange:public SuspendedThread{
public:
  OZ_Term a_var; 
  OZ_Term a_newVal; 
  SuspendedCellExchange(Mediator*, OZ_Term, OZ_Term);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  virtual bool gCollect();
};


class SuspendedLockTake:public SuspendedThread{
public:
  TaggedRef a_ozThread; 
  SuspendedLockTake(Mediator*, TaggedRef);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  virtual bool gCollect();
};

class SuspendedLockRelease:public SuspendedThread{
public:
  // This is a phony construct since I'm not able 
  // to suspend an unlocking thread. Thus this 
  // suspension holder does _not_ refere or represent
  // a proper thread..
  bool used; 
  SuspendedLockRelease(Mediator*);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  virtual bool gCollect();
};


class SuspendedArrayGet:public SuspendedThread{
public:
  OZ_Term a_var; 
  int a_indx;
  SuspendedArrayGet(Mediator*, OZ_Term, int indx);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  virtual bool gCollect();
};


class SuspendedArrayPut:public SuspendedThread{
public:
  OZ_Term a_val; 
  int a_indx;
  SuspendedArrayPut(Mediator*, OZ_Term, int indx);
  WakeRetVal resumeDoLocal(DssOperationId*);
  WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin);
  virtual bool gCollect();
};


#endif 







