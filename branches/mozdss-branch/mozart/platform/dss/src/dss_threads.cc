/* 
 *  Authors:
 *    Erik Klintskog
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
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
#pragma implementation "dss_threads.hh"
#endif

#include "dss_threads.hh"
#include "dssBase.hh"
#include "msl_serialize.hh"
#include "dss_msgLayerInterface.hh"
namespace _dss_internal{ //Start namespace
  
#ifdef DEBUG_CHECK
  int GlobalThread::a_allocated=0;
#endif
  
  GlobalThread::GlobalThread(NetIdentity ni, GlobalThreadTable* const ext):
    NetIdNode(ni),
    DssThreadId(), 
    a_exit(ext){
    DebugCode(a_allocated++);
  };


  GlobalThread::GlobalThread(GlobalThreadTable* const ext):
    NetIdNode(),
    DssThreadId(), 
    a_exit(ext){
    DebugCode(a_allocated++);
  };
  
  void
  GlobalThread::dispose(){
    a_exit->m_del(this); // remove from table and deletes too
    delete this; 
  }
  
  void 
  GlobalThread::m_makeGCpreps(){
    m_getGUIdSite()->m_makeGCpreps();
  }


  GlobalThread*
  GlobalThreadTable::insertDistThread(NetIdentity ni){
    GlobalThread* thr = new GlobalThread(ni, this); 
    m_insertNetId(thr); 
    return thr; 
  }  

  GlobalThread*
  GlobalThreadTable::createDistThread(){
    GlobalThread* thr = new GlobalThread(this); 
    m_addNetId(thr); 
    return thr; 
  }  

  GlobalThread *gf_popThreadIdVal(::MsgContainer *msg, DSS_Environment* env){
    NetIdentity ni = gf_popNetIdentity(msg); 
    GlobalThread *thread = env->a_threadTable->m_find(ni); 
    if(thread == NULL){
      thread = env->a_threadTable->insertDistThread(ni);
    }
    return thread;
  }
  
  void  gf_pushThreadIdVal(::MsgContainer* msg, GlobalThread* th){
    gf_pushNetIdentity(msg, th->m_getNetId()); 
  }
  
}// End Namespace
