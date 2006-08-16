/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
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

#if defined(INTERFACE)
#pragma implementation "protocol_eagerinvalid.hh"
#endif

#include "protocol_eagerinvalid.hh"

namespace _dss_internal{ //Start namespace

  // Quick description of the protocol.
  //
  // This protocol implements a two-phase commit scheme for updating a
  // mutable entity.  All proxies are considered readers (hence the
  // "eager" name), and those who want to write are given a special
  // token in sequence.  The write token is only given when all
  // readers have invalidated their state.  This guarantees consistent
  // causality between sites.
  //
  // Proxy P registers (at creation):
  //    P                   M
  //    |---PROT_REGISTER-->|
  //    |<--EI_READ_TOKEN---| if no write request is being served
  //
  // Proxy P requests write token: (1) P first registers to manager M.
  // (2) Once all requests before P have been served, M asks all other
  // proxies P' to invalidate their state.  (3) When all proxies have
  // invalidated, P is given the write token.  When its operations are
  // performed, it gives back the token, and M sends the new state to
  // all proxies.
  //
  // (1) P                      M
  //     |---EI_WRITE_REQUEST-->|
  //     |                     ...                        P'
  // (2) |                      |-----EI_INVALID_READ---->|
  //     |                      |<--EI_READ_INVALIDATED---|
  //     |                     ...                        |
  // (3) |<---EI_WRITE_TOKEN----|                         |
  //     |----EI_WRITE_DONE---->|                         |
  //     |                      |------EI_READ_TOKEN----->|
  //
  // Reader proxies that fail are simply discarded by the manager,
  // without affecting the entity's state.  Only the proxy with the
  // write token makes the entity permfail when it fails.

  namespace {
    enum EagerInvalid_Message {
      EI_INVALID_READ,     // m2p: invalidates reader state
      EI_READ_INVALIDATED, // p2m: confirms invalidation
      EI_READ_TOKEN,       // m2p: provides new state
      EI_WRITE_REQUEST,    // p2m: register writer
      EI_WRITE_TOKEN,      // m2p: give write token
      EI_WRITE_DONE        // p2m: give back write token and new state
    };
  }



  /******************** ProtocolEagerInvalidManager ********************/

  ProtocolEagerInvalidManager::ProtocolEagerInvalidManager(DSite *mysite) {
    registerProxy(mysite);
    a_readers.push(mysite);
  }

  void ProtocolEagerInvalidManager::sendMigrateInfo(MsgContainer* msg) {
    ProtocolManager::sendMigrateInfo(msg);
    msgPush(msg, a_readers.size());
    for (Position<DSite*> p(a_readers); p(); p++) msgPush(msg, *p);
    msgPush(msg, a_writers.size());
    for (Position<DSite*> p(a_writers); p(); p++) msgPush(msg, *p);
  }

  ProtocolEagerInvalidManager::ProtocolEagerInvalidManager(MsgContainer* msg) :
    ProtocolManager(msg)
  {
    int len = msg->popIntVal();
    while (len--) a_readers.push(msg->popDSiteVal());
    len = msg->popIntVal();
    while (len--) a_writers.append(msg->popDSiteVal());
  }


  void  ProtocolEagerInvalidManager::printStatus(){
    if (isPermFail()) {
      printf("Failed\n");
    } else {
      printf("Readers:\n"); 
      for (Position<DSite*> p(a_proxies); p(); p++)
	printf("   %s %s\n", (*p)->m_stringrep(),
	       a_readers.contains(*p) ? "+" : "-");
      printf("Writers:\n");
      Position<DSite*> p(a_writers);
      if (p()) {
	printf("   %s %s\n", (*p)->m_stringrep(), a_readers.isEmpty()?"(*)":"");
	for (p++; p(); p++) printf("   %s\n", (*p)->m_stringrep());
      }
    }
  }


  // register a proxy
  void
  ProtocolEagerInvalidManager::m_register(DSite* s) {
    registerProxy(s);
    if (a_writers.isEmpty()) m_updateOneReader(s);
  }

  // deregister a proxy (possibly because of a site failure)
  void
  ProtocolEagerInvalidManager::m_deregister(DSite* s) {
    if (a_readers.isEmpty() && !a_writers.isEmpty() && s == a_writers.peek()) {
      // s has the write token, so the state is lost
      m_failed();
    } else {
      deregisterProxy(s);
      if (a_writers.remove(s)) a_writers.check();
      m_invalidated(s);
    }
  }

  // ask all readers to invalidate their state
  void 
  ProtocolEagerInvalidManager::m_invalidateReaders() {
    Assert(!a_writers.isEmpty());
    // we don't invalidate the proxy that will receive the token
    a_readers.remove(a_writers.peek());
    // we ask the remaining readers to invalidate their state
    for (Position<DSite*> p(a_readers); p(); p++)
      sendToProxy(*p, EI_INVALID_READ);
    // in case we are ready already
    if (a_readers.isEmpty()) m_sendWriteToken();
  }

  // mark a proxy as invalidated, and possibly send the write token
  void
  ProtocolEagerInvalidManager::m_invalidated(DSite* s) {
    if (a_readers.remove(s) && a_readers.isEmpty()) m_sendWriteToken();
  }

  // all proxies have invalidated their state, send write token
  void 
  ProtocolEagerInvalidManager::m_sendWriteToken(){
    if (!a_writers.isEmpty()) {
      sendToProxy(a_writers.peek(), EI_WRITE_TOKEN);
    } else {
      // all requests have been dropped, make state valid again
      m_updateAllReaders(NULL);
    }
  }

  // send state to one reader
  void 
  ProtocolEagerInvalidManager::m_updateOneReader(DSite *s) {
    sendToProxy(s, EI_READ_TOKEN, a_coordinator->retrieveEntityState());
    a_readers.push(s);
  }

  // send state to all readers, and prepare next request
  void 
  ProtocolEagerInvalidManager::m_updateAllReaders(DSite* writer) {
    for (Position<DSite*> p(a_proxies); p(); p++) {
      if (*p == writer) {
	a_readers.push(writer);   // writer already has state
      } else {
	m_updateOneReader(*p);   // send new state
      }
    }
    if (!a_writers.isEmpty()) m_invalidateReaders();
  }

  // notify permfail to all known proxies
  void
  ProtocolEagerInvalidManager::m_failed() {
    while (!a_readers.isEmpty()) a_readers.pop();
    while (!a_writers.isEmpty()) a_writers.pop();
    makePermFail();
  }


  void
  ProtocolEagerInvalidManager::msgReceived(::MsgContainer* msg, DSite* sender){
    if (isPermFail()) {
      sendToProxy(sender, PROT_PERMFAIL); return;
    }
    int message = msg->popIntVal();
    switch(message) {
    case PROT_REGISTER: {
      m_register(sender);
      break;
    }
    case PROT_DEREGISTER: {
      m_deregister(sender);
      break;
    }
    case EI_READ_INVALIDATED: { // a proxy confirms its invalidation
      m_invalidated(sender);
      break; 
    }
    case EI_WRITE_REQUEST: {
      bool ready = a_writers.isEmpty();
      a_writers.append(sender); 
      if (ready) m_invalidateReaders();
      break;
    }
    case EI_WRITE_DONE: {
      Assert(sender == a_writers.peek());
      PstInContainerInterface* builder = gf_popPstIn(msg);  
      a_coordinator->installEntityState(builder);
      a_writers.pop();
      m_updateAllReaders(sender);
      break;
    }
    case PROT_PERMFAIL: {
      m_failed();
      break;
    }
    default:
      Assert(0);
    }
  }


  // handle site failures
  void
  ProtocolEagerInvalidManager::m_siteStateChange(DSite* s,
						 const DSiteState& state) {
    if (!isPermFail() && state >= DSite_GLOBAL_PRM) m_deregister(s);
  }



  /******************** ProtocolEagerInvalidProxy ********************/

  ProtocolEagerInvalidProxy::ProtocolEagerInvalidProxy():
    ProtocolProxy(PN_EAGER_INVALID), a_reads(0) {
    setStatus(true);
    setRegistered(true);
  }

  bool
  ProtocolEagerInvalidProxy::m_initRemoteProt(DssReadBuffer*) {
    protocol_Register();
    setStatus(false);
    return true;
  }

  ProtocolEagerInvalidProxy::~ProtocolEagerInvalidProxy(){
    Assert(a_susps.isEmpty()); 
    if (!a_proxy->m_isHomeProxy()) protocol_Deregister();
  }

  
  OpRetVal
  ProtocolEagerInvalidProxy::protocol_Read(GlobalThread* const th_id,
					   PstOutContainerInterface**& msg){
    dssLog(DLL_BEHAVIOR,"EagerInvalidProxy::Read");
    msg = NULL;
    if (isPermFail()) return DSS_RAISE;
    if (getStatus()) return DSS_PROCEED;
    a_susps.push(th_id); a_reads++;
    return DSS_SUSPEND;
  }

  OpRetVal
  ProtocolEagerInvalidProxy::protocol_Write(GlobalThread* const th_id,
					    PstOutContainerInterface**& msg){
    dssLog(DLL_BEHAVIOR,"EagerInvalidProxy::Write");
    msg = NULL;
    if (isPermFail()) return DSS_RAISE;
    if (a_reads == a_susps.size()) sendToManager(EI_WRITE_REQUEST);
    a_susps.append(th_id);
    return DSS_SUSPEND;
  }


  void
  ProtocolEagerInvalidProxy::msgReceived(::MsgContainer* msg, DSite*){
    if (isPermFail()) return;
    int message = msg->popIntVal();
    switch (message) {
    case EI_INVALID_READ: {
      setStatus(false);
      sendToManager(EI_READ_INVALIDATED);
      break; 
    }
    case EI_READ_TOKEN:{
      PstInContainerInterface* buildcont = gf_popPstIn(msg);
      a_proxy->installEntityState(buildcont);
      setStatus(true);
      // wake up read operations
      for (; a_reads > 0; a_reads--) a_susps.pop()->resumeDoLocal(NULL);
      break;
    }
    case EI_WRITE_TOKEN:{
      // wake up all readers and writers, then send back result to manager
      while (!a_susps.isEmpty()) a_susps.pop()->resumeDoLocal(NULL);
      a_reads = 0;
      sendToManager(EI_WRITE_DONE, a_proxy->retrieveEntityState());
      break; 
    }
    case PROT_PERMFAIL: {
      makePermFail();
      break;
    }
    default:
      Assert(0);
    }
  }


  // interpret site failures
  FaultState
  ProtocolEagerInvalidProxy::siteStateChanged(DSite* s,
					      const DSiteState& state) {
    if (!isPermFail() && a_proxy->m_getCoordinatorSite() == s) {
      switch (state) {
      case DSite_OK:         return FS_PROT_STATE_OK;
      case DSite_TMP:        return FS_PROT_STATE_TMP_UNAVAIL;
      case DSite_GLOBAL_PRM:
      case DSite_LOCAL_PRM:  makePermFail(); return FS_PROT_STATE_PRM_UNAVAIL;
      default:
	dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
      }
    }
    return 0;
  }

} //End namespace
