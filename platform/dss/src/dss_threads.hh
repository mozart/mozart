/*
 *  Authors:
 *   Zacharias El Banna
 *   Erik Klintskog
 *
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
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

#ifndef __DSS_THREAD_HH
#define __DSS_THREAD_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "dss_msgLayerInterface.hh"
#include "dss_netId.hh"

namespace _dss_internal{ //Start namespace

  class GlobalThreadTable;

  class GlobalThread : public NetIdNode, public BucketHashNode<GlobalThread>,
                       public DssThreadId {
    friend class GlobalThreadTable;
  private:
    GlobalThreadTable* a_exit;

    GlobalThread(const GlobalThread&);
    GlobalThread& operator=(const GlobalThread&){ return *this; }

  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif

    GlobalThread(NetIdentity ni, GlobalThreadTable* const ext);
    GlobalThread(GlobalThreadTable* const ext);
    ~GlobalThread() { DebugCode(a_allocated--); }

    WakeRetVal resumeDoLocal(DssOperationId* id){
      return getThreadMediator()->resumeDoLocal(id);
    }
    WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin){
      return getThreadMediator()->resumeRemoteDone(pstin);
    }
    WakeRetVal resumeFailed() {
      return getThreadMediator()->resumeFailed();
    }
    virtual void dispose();
    void m_makeGCpreps();
  };


  class GlobalThreadTable : public NetIdHT, public BucketHashTable<GlobalThread> {
  public:
    GlobalThreadTable(const int& sz, DSS_Environment* env) :
      NetIdHT(env), BucketHashTable<GlobalThread>(sz) {}

    GlobalThread *m_find(NetIdentity ni) {
      return lookup(ni.hashCode(), ni);
    }
    GlobalThread* insertDistThread(NetIdentity);
    GlobalThread* createDistThread();

    void m_gcResources();
  };

  GlobalThread *gf_popThreadIdVal(::MsgContainer*, DSS_Environment* );
  void  gf_pushThreadIdVal(::MsgContainer*, GlobalThread*);
}

#endif
