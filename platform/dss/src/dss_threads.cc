/*
 *  Authors:
 *    Erik Klintskog
 *
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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
    NetIdNode(ni), BucketHashNode<GlobalThread>(),
    DssThreadId(),
    a_exit(ext)
  {
    DebugCode(a_allocated++);
  }

  GlobalThread::GlobalThread(GlobalThreadTable* const ext):
    NetIdNode(), BucketHashNode<GlobalThread>(),
    DssThreadId(),
    a_exit(ext)
  {
    DebugCode(a_allocated++);
  }

  void
  GlobalThread::dispose(){
    a_exit->remove(this);   // remove from table and deletes too
    delete this;
  }

  void
  GlobalThread::m_makeGCpreps(){
    m_getGUIdSite()->m_makeGCpreps();
  }

  /************************* GlobalThreadTable *************************/

  GlobalThread*
  GlobalThreadTable::insertDistThread(NetIdentity ni){
    GlobalThread* thr = new GlobalThread(ni, this);
    insert(thr);
    return thr;
  }

  GlobalThread*
  GlobalThreadTable::createDistThread(){
    return insertDistThread(m_createNetIdentity());
  }

  void GlobalThreadTable::m_gcResources() {
    for (GlobalThread* t = getFirst(); t; t = getNext(t)) {
      t->m_makeGCpreps();
    }
    checkSize();
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
