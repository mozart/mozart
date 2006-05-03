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
  //    P                      M
  //    |---EI_REGISTER_READ-->|
  //    |<---EI_READ_TOKEN-----| if no write request is being served
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
      EI_REGISTER_READ,    // p2m: register reader
      EI_DEREGISTER_READ,  // p2m: deregister reader
      EI_INVALID_READ,     // m2p: invalidates reader state
      EI_READ_INVALIDATED, // p2m: confirms invalidation
      EI_READ_TOKEN,       // m2p: provides new state
      EI_WRITE_REQUEST,    // p2m: register writer
      EI_WRITE_TOKEN,      // m2p: give write token
      EI_WRITE_DONE,       // p2m: give back write token and new state
      EI_PERMFAIL          // mp,pm: make entity permfail
    };
  }



  /******************** ProtocolEagerInvalidManager ********************/

  ProtocolEagerInvalidManager::ProtocolEagerInvalidManager(DSite *mysite):
    a_readers(), a_requests(), a_writer(NULL) {
    a_readers.push(makePair(mysite, true));
  }

  void ProtocolEagerInvalidManager::sendMigrateInfo(MsgContainer* msg){
    int len = 0;
    for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++) len++;
    msg->pushIntVal(len);
    for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++) {
      msg->pushDSiteVal((*p).first);
      msg->pushIntVal((*p).second);
    }
    // move current writer and pending requests
    msg->pushDSiteVal(a_writer);
    for (Position<DSite*> p(a_requests); p(); p++) msg->pushDSiteVal(*p);
  }

  ProtocolEagerInvalidManager::ProtocolEagerInvalidManager(MsgContainer* msg) :
    a_readers(), a_requests(), a_writer(NULL)
  {
    int len = msg->popIntVal();
    for (;len>0; len--)
      a_readers.push(makePair(msg->popDSiteVal(), (bool) msg->popIntVal()));
    // then get writer and requests
    a_writer = msg->popDSiteVal();
    while (!msg->m_isEmpty()) a_requests.append(msg->popDSiteVal());
  }

  void ProtocolEagerInvalidManager::makeGCpreps(){
    t_gcList(a_readers);
    t_gcList(a_requests);
    if (a_writer) a_writer->m_makeGCpreps();
  }


  void  ProtocolEagerInvalidManager::printStatus(){
    if (m_isFailed()) {
      printf("Failed\n");
    } else {
      printf("Readers:\n"); 
      for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++)
	printf("   %s %s\n", (*p).first->m_stringrep(), (*p).second?"+":"-");
      printf("Writers:\n");
      if (a_writer) printf("   %s (*)\n", a_writer->m_stringrep());
      for (Position<DSite*> p(a_requests); p(); p++)
	printf("   %s\n", (*p)->m_stringrep());
    }
  }


  // register a proxy
  void
  ProtocolEagerInvalidManager::m_register(DSite* s) {
    if (a_writer == NULL && a_requests.isEmpty()) {
      a_readers.push(makePair(s, true));
      m_updateOneReader(s);
    } else {
      a_readers.push(makePair(s, false));
    }
  }

  // deregister a proxy (possibly because of a site failure)
  void
  ProtocolEagerInvalidManager::m_deregister(DSite* s) {
    Assert(s != a_writer);
    Position<Pair<DSite*,bool> > p(a_readers);
    if (p.find(s)) {
      p.remove();
      if (a_requests.remove(s)) a_requests.check();
      if (!a_writer) m_invalidated(s);
    }
  }

  // ask all readers to invalidate their state
  void 
  ProtocolEagerInvalidManager::m_invalidateReaders() {
    Assert(a_writer == NULL && !a_requests.isEmpty());
    DSite* writer = a_requests.peek();
    bool ready = true;
    for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++) {
      if ((*p).first == writer) {
	(*p).second = false;
      } else {
	Assert((*p).second);
	sendToProxy((*p).first, EI_INVALID_READ);
	ready = false;
      }
    }
    if (ready) m_sendWriteToken();
  }

  // mark a proxy as invalidated, and possibly send the write token
  void
  ProtocolEagerInvalidManager::m_invalidated(DSite* s) {
    bool ready = true;
    for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++) {
      if ((*p).first == s) { (*p).second = false; }
      ready = ready && !(*p).second;
    }
    if (ready) m_sendWriteToken();
  }

  // all proxies have invalidated their state, send write token
  void 
  ProtocolEagerInvalidManager::m_sendWriteToken(){
    if (!a_requests.isEmpty()) {
      a_writer = a_requests.pop();
      sendToProxy(a_writer, EI_WRITE_TOKEN);
    } else {
      // all requests have been dropped, make state valid again
      m_updateAllReaders();
    }
  }

  // send state to one reader
  void 
  ProtocolEagerInvalidManager::m_updateOneReader(DSite *s) {
    sendToProxy(s, EI_READ_TOKEN, a_coordinator->retrieveEntityState());
  }

  // send state to all readers, and prepare next request
  void 
  ProtocolEagerInvalidManager::m_updateAllReaders() {
    for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++) {
      if ((*p).first != a_writer) m_updateOneReader((*p).first);
      (*p).second = true;
    }
    a_writer = NULL;
    if (!a_requests.isEmpty()) m_invalidateReaders();
  }

  // notify permfail to all known proxies
  void
  ProtocolEagerInvalidManager::m_failed() {
    while (!a_readers.isEmpty())
      sendToProxy(a_readers.pop().first, EI_PERMFAIL);
    while (!a_requests.isEmpty()) a_requests.pop();
    a_writer = NULL;
  }


  void
  ProtocolEagerInvalidManager::msgReceived(::MsgContainer* msg, DSite* sender){
    if (m_isFailed()) {
      sendToProxy(sender, EI_PERMFAIL); return;
    }
    int message = msg->popIntVal();
    switch(message) {
    case EI_REGISTER_READ: {
      m_register(sender);
      break;
    }
    case EI_DEREGISTER_READ: {
      Assert(a_writer != sender && !a_requests.contains(sender));
      m_deregister(sender);
      break;
    }
    case EI_READ_INVALIDATED: { // a proxy confirms its invalidation
      m_invalidated(sender);
      break; 
    }
    case EI_WRITE_REQUEST: {
      bool ready = (a_writer == NULL && a_requests.isEmpty());
      a_requests.append(sender); 
      if (ready) m_invalidateReaders();
      break;
    }
    case EI_WRITE_DONE: {
      Assert(sender == a_writer);
      PstInContainerInterface* builder = gf_popPstIn(msg);  
      a_coordinator->installEntityState(builder);
      m_updateAllReaders();
      break;
    }
    case EI_PERMFAIL: {
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
    if (!m_isFailed() && state >= DSite_GLOBAL_PRM) {
      if (s == a_writer) m_failed(); else m_deregister(s);
    }
  }



  /******************** ProtocolEagerInvalidProxy ********************/

  ProtocolEagerInvalidProxy::ProtocolEagerInvalidProxy():
    ProtocolProxy(PN_EAGER_INVALID), a_failed(false), a_valid(true),
    a_readers(), a_writers() {}

  bool
  ProtocolEagerInvalidProxy::m_initRemoteProt(DssReadBuffer*){
    sendToManager(EI_REGISTER_READ);
    a_valid = false;
    return true;
  }

  ProtocolEagerInvalidProxy::~ProtocolEagerInvalidProxy(){
    Assert(a_writers.isEmpty()); 
    Assert(a_readers.isEmpty()); 
    if (!a_proxy->m_isHomeProxy()) sendToManager(EI_DEREGISTER_READ);
  }

  void ProtocolEagerInvalidProxy::makeGCpreps(){
    t_gcList(a_readers);
    t_gcList(a_writers);
  }


  void 
  ProtocolEagerInvalidProxy::m_writeDone() {
    sendToManager(EI_WRITE_DONE, a_proxy->retrieveEntityState());
  }

  void
  ProtocolEagerInvalidProxy::m_failed() {
    a_failed = true;
    a_proxy->updateFaultState(FS_PROT_STATE_PRM_UNAVAIL);
    // resume suspended threads
    while (!a_readers.isEmpty()) a_readers.pop()->resumeFailed();
    while (!a_writers.isEmpty()) a_writers.pop()->resumeFailed();
  }

  
  OpRetVal
  ProtocolEagerInvalidProxy::protocol_Read(GlobalThread* const th_id,
					   PstOutContainerInterface**& msg){
    dssLog(DLL_BEHAVIOR,"EagerInvalidProxy::Read");
    msg = NULL;
    if (a_failed) return DSS_RAISE;
    if (a_valid) return DSS_PROCEED;
    a_readers.append(th_id);
    return DSS_SUSPEND;
  }
  
  OpRetVal
  ProtocolEagerInvalidProxy::protocol_Write(GlobalThread* const th_id,
					    PstOutContainerInterface**& msg){
    dssLog(DLL_BEHAVIOR,"EagerInvalidProxy::Write");
    msg = NULL;
    if (a_failed) return DSS_RAISE;
    if (a_writers.isEmpty()) sendToManager(EI_WRITE_REQUEST);
    a_writers.append(th_id);
    return DSS_SUSPEND;
  }

  OpRetVal
  ProtocolEagerInvalidProxy::protocol_Kill() {
    if (!a_failed) sendToManager(EI_PERMFAIL);
    return DSS_SKIP;
  }


  void
  ProtocolEagerInvalidProxy::msgReceived(::MsgContainer* msg, DSite*){
    if (a_failed) return;
    int message = msg->popIntVal();
    switch (message) {
    case EI_INVALID_READ: {
      a_valid = false;
      sendToManager(EI_READ_INVALIDATED);
      break; 
    }
    case EI_READ_TOKEN:{
      PstInContainerInterface* buildcont = gf_popPstIn(msg);
      a_proxy->installEntityState(buildcont);
      a_valid = true;
      // wake up readers
      while (!a_readers.isEmpty())
	a_readers.pop()->resumeDoLocal(NULL);
      break;
    }
    case EI_WRITE_TOKEN:{
      // wake up all writers, then send back result to manager
      while (!a_writers.isEmpty())
	a_writers.pop()->resumeDoLocal(NULL);
      m_writeDone();
      break; 
    }
    case EI_PERMFAIL: {
      m_failed();
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
    if (!a_failed && a_proxy->m_getCoordinatorSite() == s) {
      switch (state) {
      case DSite_OK:
	return FS_PROT_STATE_OK;
      case DSite_TMP:
	return FS_PROT_STATE_TMP_UNAVAIL;
      case DSite_GLOBAL_PRM: case DSite_LOCAL_PRM:
	m_failed();
	return FS_PROT_STATE_PRM_UNAVAIL;
      default:
	dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
      }
    }
    return 0;
  }

} //End namespace
