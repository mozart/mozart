/*
 *  Authors:
 *   Zacharias El Banna
 *   Erik Klintskog 
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
  
  class GlobalThread: public NetIdNode, public DssThreadId {
    friend class GlobalThreadTable;
    GlobalThreadTable* a_exit;

  private:
    GlobalThread(const GlobalThread&):NetIdNode(), a_exit(NULL){}
    GlobalThread& operator=(const GlobalThread&){ return *this; }

  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif

    GlobalThread(NetIdentity ni, GlobalThreadTable* const ext); 
    GlobalThread(GlobalThreadTable* const ext); 
    ~GlobalThread(){ DebugCode(a_allocated--); }
    
    inline WakeRetVal resumeDoLocal(DssOperationId* id){
      return getThreadMediator()->resumeDoLocal(id);
    }
    inline WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin){
      return getThreadMediator()->resumeRemoteDone(pstin); 
    }
    inline WakeRetVal resumeFailed() {
      return getThreadMediator()->resumeFailed();
    }
    virtual void dispose();
    void m_makeGCpreps();
  };
  
  
  class GlobalThreadTable:private NetIdHT {
  public:
    GlobalThreadTable(const int& sz, DSS_Environment* env):
      NetIdHT(sz, env){}
    
    //inline void m_del(const unsigned int& id1, DSite* const id2) { htSubPkSk(id1,reinterpret_cast<u32>(id2)); }
    inline void m_del(GlobalThread* const th){ m_removeNetId(th); }
    
    inline GlobalThread *m_find(NetIdentity ni){
      return reinterpret_cast<GlobalThread*>(m_findNetId(ni));
    }
    void m_gcResources();
    
    GlobalThread* insertDistThread(NetIdentity); 
    GlobalThread* createDistThread(); 
  };

  GlobalThread *gf_popThreadIdVal(::MsgContainer*, DSS_Environment* );
  void  gf_pushThreadIdVal(::MsgContainer*, GlobalThread*);

}  
#endif
