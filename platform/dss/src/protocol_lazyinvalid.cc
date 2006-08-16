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
#pragma implementation "protocol_lazyinvalid.hh"
#endif

#include "protocol_lazyinvalid.hh"

namespace _dss_internal{ //Start namespace

  // Quick description of the protocol.
  //
  // This protocol is a variant of the "eager invalidation" protocol.
  // In this case reader proxies must explicitly register.  The
  // protocol actually implements a single writer/multiple reader
  // token.  All read/write requests are queued in the manager.
  // Contiguous read requests in the queue are served in parallel.
  // Serving a write request requires all former readers to invalidate
  // their state.
  //
  // Proxy P asks for writing, P' asks for reading, and P" asks for
  // writing (in that order).  Assume that P already has the write
  // token.  (1) P' registers.  (2) Once P''s request is served, the
  // former writer (P) is asked to give back the write token, and
  // invalidate.  (3) P' is then given the state.  Assume that P" has
  // registered after (1).  (4) For serving P", all readers are asked
  // to invalidate.  (5) Once done, the state and the write token are
  // given to P".
  //
  //     P                          M                         P'
  // (1) |                          |<----LI_READ_REQUEST-----|
  //     |                         ...                        |
  // (2) |<----LI_INVALID_WRITE-----|                         |
  //     |---LI_WRITE_INVALIDATED-->|                         |
  // (3)                            |------LI_READ_TOKEN----->|
  //                               ...                        |
  // (4)                            |-----LI_INVALID_READ---->|
  //     P"                         |<--LI_READ_INVALIDATED---|
  //     |                         ...
  // (5) |<-----LI_WRITE_TOKEN------|
  //
  // Like the eager invalidation protocol, reader proxies that fail
  // are simply discarded by the manager, without affecting the
  // entity's state.  Only the proxy with the write token makes the
  // entity permfail when it fails.

  namespace {
    enum LazyInvalid_Message {
      LI_READ_REQUEST,      // p2m: proxy requests to read
      LI_READ_TOKEN,        // m2p: provides new entity state
      LI_INVALID_READ,      // m2p: invalidates reader state
      LI_READ_INVALIDATED,  // p2m: confirms invalidation
      LI_WRITE_REQUEST,     // p2m: proxy requests to write
      LI_WRITE_TOKEN,       // m2p: provides new state and write token
      LI_INVALID_WRITE,     // m2p: writer is asked to give back write token
      LI_WRITE_INVALIDATED, // p2m: give back write token (but not read)
      LI_PERMFAIL           // mp,pm: make entity permfail
    };
  }



  /******************** ProtocolLazyInvalidManager ********************/

  ProtocolLazyInvalidManager::ProtocolLazyInvalidManager(DSite *mysite) :
    a_writer(mysite) {}

  void ProtocolLazyInvalidManager::sendMigrateInfo(::MsgContainer* msg){
    ProtocolManager::sendMigrateInfo(msg);
    msg->pushIntVal(a_requests.size());
    for (Position<Pair<DSite*,bool> > req(a_requests); req(); req++) {
      msg->pushDSiteVal((*req).first); msg->pushIntVal((*req).second);
    }
    msg->pushIntVal(a_readers.size());
    for (Position<DSite*> r(a_readers); r(); r++) msg->pushDSiteVal(*r);
    msg->pushDSiteVal(a_writer);
  }

  ProtocolLazyInvalidManager::ProtocolLazyInvalidManager(::MsgContainer* msg):
    ProtocolManager(msg) {
    int len = msg->popIntVal();
    while (len--)
      a_requests.append(makePair(msg->popDSiteVal(), (bool) msg->popIntVal()));
    len = msg->popIntVal();
    while (len--) a_readers.push(msg->popDSiteVal());
    a_writer = msg->popDSiteVal();
  }

  void ProtocolLazyInvalidManager::makeGCpreps(){
    ProtocolManager::makeGCpreps();
    t_gcList(a_requests);
    t_gcList(a_readers);
    if (a_writer) a_writer->m_makeGCpreps();
  }


  // all proxies have invalidated their state, send write token
  void 
  ProtocolLazyInvalidManager::m_sendWriteToken(){
    a_writer = a_requests.pop().first;
    sendToProxy(a_writer, LI_WRITE_TOKEN,
		a_coordinator->retrieveEntityState());
  }

  // send state to one reader
  void 
  ProtocolLazyInvalidManager::m_updateOneReader(DSite *s) {
    sendToProxy(s, LI_READ_TOKEN, a_coordinator->retrieveEntityState());
    a_readers.push(s);
  }

  // try to handle next request
  void 
  ProtocolLazyInvalidManager::m_handleNextRequest() {
    if (a_writer) {
      // We cannot handle the next request yet, the current writer
      // must give back the write token.
      sendToProxy(a_writer, LI_INVALID_WRITE);

    } else {
      // send the state to all read requests in front of a_requests
      while (!a_requests.isEmpty() && a_requests.peek().second)
	m_updateOneReader(a_requests.pop().first);

      if (!a_requests.isEmpty()) {
	// The next requests is a write;
	Assert(!a_requests.peek().second);
	// invalidate all readers, except the writer itself.
	a_readers.remove(a_requests.peek().first);
	for (Position<DSite*> r(a_readers); r(); r++)
	  sendToProxy(*r, LI_INVALID_READ);
	// optimization: if there is no reader left, send immediately
	if (a_readers.isEmpty()) {
	  m_sendWriteToken();
	  // So, we have now passed the state to a writer.  If there
	  // are more requests in the Pipe, they must be handled.
	  if (!a_requests.isEmpty()) m_handleNextRequest();
	}
      }
    }
  }

  // signal failure to all known proxies
  void
  ProtocolLazyInvalidManager::m_failed() {
    // put all known proxies in a_proxies
    while (!a_requests.isEmpty()) {
      DSite* s = a_requests.pop().first;
      if (!isRegisteredProxy(s)) registerProxy(s);
    }
    while (!a_readers.isEmpty()) {
      DSite* s = a_readers.pop();
      if (!isRegisteredProxy(s)) registerProxy(s);
    }
    if (a_writer) {
      if (!isRegisteredProxy(a_writer)) registerProxy(a_writer);
      a_writer = NULL;
    }
    makePermFail();
  }


  void
  ProtocolLazyInvalidManager::msgReceived(::MsgContainer* msg, DSite* sender){
    if (isPermFail()) {
      sendToProxy(sender, PROT_PERMFAIL); return;
    }
    int message = msg->popIntVal();
    switch(message) {
    case PROT_REGISTER: {
      registerProxy(sender);
      break;
    }
    case PROT_DEREGISTER: {
      deregisterProxy(sender);
      break;
    }
    case LI_READ_REQUEST: {
      bool ready = a_requests.isEmpty();
      a_requests.append(makePair(sender, true));
      if (ready) m_handleNextRequest();
      break;
    }
    case LI_READ_INVALIDATED: {
      if (a_readers.remove(sender) && a_readers.isEmpty())
	m_handleNextRequest();
      break;
    }
    case LI_WRITE_REQUEST: {
      bool ready = a_requests.isEmpty();
      a_requests.append(makePair(sender, false));
      if (ready) m_handleNextRequest();
      break;
    }
    case LI_WRITE_INVALIDATED: {
      PstInContainerInterface* builder = gf_popPstIn(msg); 
      a_coordinator->installEntityState(builder); 
      Assert(sender == a_writer);
      a_writer = NULL;
      Assert(a_readers.isEmpty());
      a_readers.push(sender);     // the sender has kept its state valid
      m_handleNextRequest();
      break;
    }
    case LI_PERMFAIL: {
      m_failed();
      break;
    }
    default:
      Assert(0);
    }
  }


  // handle site failures
  void
  ProtocolLazyInvalidManager::m_siteStateChange(DSite* s,
						const DSiteState& state) {
    if (!isPermFail() && state >= DSite_GLOBAL_PRM) {
      // discard it as a reader
      bool ready = a_readers.remove(s) && a_readers.isEmpty();
      // discard all its requests, too
      Position<Pair<DSite*, bool> > req(a_requests);
      while (req.find(s)) req.remove();
      a_requests.check();
      // check for the next request
      if (ready) {
	Assert(a_writer == NULL);
	m_handleNextRequest();
      } else if (s == a_writer) {
	// we have lost the writer, and the state!
	a_writer = NULL;
	m_failed();
      }
    }
  }



  /******************** ProtocolLazyInvalidProxy ********************/

  ProtocolLazyInvalidProxy::ProtocolLazyInvalidProxy():
    ProtocolProxy(PN_LAZY_INVALID), a_reads(0) {
    setStatus(LIT_WRITER);
  }

  bool
  ProtocolLazyInvalidProxy::m_initRemoteProt(DssReadBuffer*) {
    setStatus(LIT_INVALID);
    return true;
  }

  ProtocolLazyInvalidProxy::~ProtocolLazyInvalidProxy() {
    Assert(a_susps.isEmpty());
    if (!a_proxy->m_isHomeProxy()) protocol_Deregister();
  }


  // request read and write tokens
  void ProtocolLazyInvalidProxy::m_requestReadToken() {
    sendToManager(LI_READ_REQUEST);
  }

  void ProtocolLazyInvalidProxy::m_requestWriteToken() {
    sendToManager(LI_WRITE_REQUEST);
  }


  OpRetVal
  ProtocolLazyInvalidProxy::protocol_Read(GlobalThread* const th_id,
					  PstOutContainerInterface**& msg){
    dssLog(DLL_BEHAVIOR,"LazyInvalidProxy::Read");
    msg = NULL;
    if (isPermFail()) return DSS_RAISE;
    switch (getStatus()) {
    case LIT_INVALID:
      if (a_reads == 0) m_requestReadToken();
      a_susps.push(th_id); a_reads++;
      return DSS_SUSPEND;
    case LIT_READER:
    case LIT_WRITER:
      return DSS_PROCEED;
    default:
      return DSS_RAISE;
    }
  }

  OpRetVal
  ProtocolLazyInvalidProxy::protocol_Write(GlobalThread* const th_id,
					   PstOutContainerInterface**& msg){
    dssLog(DLL_BEHAVIOR,"LazyInvalidProxy::Write");
    msg = NULL;
    if (isPermFail()) return DSS_RAISE;
    switch (getStatus()) {
    case LIT_INVALID:
    case LIT_READER:
      if (a_reads == a_susps.size()) m_requestWriteToken();
      a_susps.append(th_id);
      return DSS_SUSPEND;
    case LIT_WRITER:
      return DSS_PROCEED;
    default:
      return DSS_RAISE;
    }
  }


  void
  ProtocolLazyInvalidProxy::msgReceived(::MsgContainer* msg, DSite*){
    if (isPermFail()) return;
    int message = msg->popIntVal();
    switch (message) {
    case LI_READ_TOKEN: {
      setStatus(LIT_READER);
      ::PstInContainerInterface* builder = gf_popPstIn(msg);
      a_proxy->installEntityState(builder);
      // wake up read operations
      for (; a_reads > 0; a_reads--) a_susps.pop()->resumeDoLocal(NULL);
      break; 
    }
    case LI_INVALID_READ: {
      setStatus(LIT_INVALID);
      sendToManager(LI_READ_INVALIDATED);
      break;
    }
    case LI_WRITE_TOKEN: {
      setStatus(LIT_WRITER);
      PstInContainerInterface* builder = gf_popPstIn(msg);
      a_proxy->installEntityState(builder);
      // wake up read and write operations
      while (!a_susps.isEmpty()) a_susps.pop()->resumeDoLocal(NULL);
      break; 
    }
    case LI_INVALID_WRITE: {
      setStatus(LIT_READER);
      sendToManager(LI_WRITE_INVALIDATED, a_proxy->retrieveEntityState());
      break;
    }
    case LI_PERMFAIL: {
      makePermFail();
      break;
    }
    default:
      Assert(0);
    }
  }


  // interpret site failures
  FaultState
  ProtocolLazyInvalidProxy::siteStateChanged(DSite* s,
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
