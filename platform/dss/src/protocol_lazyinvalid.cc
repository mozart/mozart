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
    a_failed(false), a_requests(), a_readers(), a_writer(mysite) {}

  void ProtocolLazyInvalidManager::sendMigrateInfo(::MsgContainer* msg){
    msg->pushIntVal(a_failed);
    if (a_failed) return;
    int len = 0;
    for (Position<DSite*> r(a_readers); r(); r++) len++;
    // send readers, current writer, then requests
    msg->pushIntVal(len);
    for (Position<DSite*> r(a_readers); r(); r++) msg->pushDSiteVal(*r);
    msg->pushDSiteVal(a_writer);
    for (Position<Pair<DSite*,bool> > req(a_requests); req(); req++) {
      msg->pushDSiteVal((*req).first);
      msg->pushIntVal((*req).second);
    }
  }

  ProtocolLazyInvalidManager::ProtocolLazyInvalidManager(::MsgContainer* msg):
    a_failed(false), a_requests(), a_readers(), a_writer(NULL)
  {
    a_failed = msg->popIntVal();
    if (a_failed) return;
    int len = msg->popIntVal();
    for (;len>0; len --) a_readers.push(msg->popDSiteVal());
    a_writer = msg->popDSiteVal();
    while (!msg->m_isEmpty())
      a_requests.append(makePair(msg->popDSiteVal(), (bool) msg->popIntVal()));
  }

  void ProtocolLazyInvalidManager::makeGCpreps(){
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
      while (!a_requests.isEmpty() && a_requests.peek().second) {
	DSite* r = a_requests.pop().first;
	m_updateOneReader(r);
	a_readers.push(r);
      }
      if (!a_requests.isEmpty()) {
	// The next requests is a write; invalidate all readers,
	// except the writer itself.
	DSite* writer = a_requests.peek().first;
	Assert(!a_requests.peek().second);
	a_readers.remove(writer);
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
    a_failed = true;
    while (!a_requests.isEmpty())
      sendToProxy(a_requests.pop().first, LI_PERMFAIL);
    while (!a_readers.isEmpty())
      sendToProxy(a_readers.pop(), LI_PERMFAIL);
    if (a_writer) sendToProxy(a_writer, LI_PERMFAIL);
    a_writer = NULL;
  }


  void
  ProtocolLazyInvalidManager::msgReceived(::MsgContainer* msg, DSite* sender){
    if (a_failed) {
      sendToProxy(sender, LI_PERMFAIL); return;
    }
    int message = msg->popIntVal();
    switch(message) {
    case LI_READ_REQUEST: {
      bool ready = a_requests.isEmpty();
      a_requests.append(makePair(sender, true));
      if (ready) m_handleNextRequest();
      break;
    }
    case LI_READ_INVALIDATED: {
      Position<DSite*> r(a_readers);
      if (r.find(sender)) r.remove();
      if (a_readers.isEmpty()) m_handleNextRequest();
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
      a_readers.push(sender);
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
    if (!a_failed && state >= DSite_GLOBAL_PRM) {
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
    ProtocolProxy(PN_LAZY_INVALID), a_token(LIT_WRITER),
    a_readers(), a_writers() {}

  bool
  ProtocolLazyInvalidProxy::m_initRemoteProt(DssReadBuffer*) {
    a_token = LIT_INVALID;
    return true;
  }

  ProtocolLazyInvalidProxy::~ProtocolLazyInvalidProxy() {
    Assert(a_writers.isEmpty());
    Assert(a_readers.isEmpty());
  }

  void ProtocolLazyInvalidProxy::makeGCpreps() {
    t_gcList(a_readers);
    t_gcList(a_writers);
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
    switch (a_token) {
    case LIT_INVALID:
      if (a_readers.isEmpty()) m_requestReadToken();
      a_readers.append(th_id);
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
    switch (a_token) {
    case LIT_INVALID:
    case LIT_READER:
      if (a_writers.isEmpty()) m_requestWriteToken();
      a_writers.append(th_id);
      return DSS_SUSPEND;
    case LIT_WRITER:
      return DSS_PROCEED;
    default:
      return DSS_RAISE;
    }
  }

  OpRetVal
  ProtocolLazyInvalidProxy::protocol_Kill(GlobalThread* const th_id) {
    dssLog(DLL_BEHAVIOR,"LazyInvalidProxy::Kill");
    if (a_token == LIT_FAILED) return DSS_SKIP;
    sendToManager(LI_PERMFAIL);
    a_readers.append(th_id);
    return DSS_SUSPEND;
  }


  void
  ProtocolLazyInvalidProxy::msgReceived(::MsgContainer* msg, DSite*){
    if (a_token == LIT_FAILED) return;
    int message = msg->popIntVal();
    switch (message) {
    case LI_READ_TOKEN: {
      a_token = LIT_READER;
      ::PstInContainerInterface* builder = gf_popPstIn(msg);
      a_proxy->installEntityState(builder);
      // wake up readers
      while (!a_readers.isEmpty()) a_readers.pop()->resumeDoLocal(NULL);
      break; 
    }
    case LI_INVALID_READ: {
      if (a_token == LIT_READER) {
	a_token = LIT_INVALID;
	sendToManager(LI_READ_INVALIDATED);
      }
      break;
    }
    case LI_WRITE_TOKEN: {
      a_token = LIT_WRITER;
      PstInContainerInterface* builder = gf_popPstIn(msg);
      a_proxy->installEntityState(builder);
      // wake up writers
      while (!a_writers.isEmpty()) a_writers.pop()->resumeDoLocal(NULL);
      break; 
    }
    case LI_INVALID_WRITE: {
      if (a_token == LIT_WRITER) {
	a_token = LIT_READER;
	sendToManager(LI_WRITE_INVALIDATED, a_proxy->retrieveEntityState());
      }
      break;
    }
    case LI_PERMFAIL: {
      a_token = LIT_FAILED;
      a_proxy->updateFaultState(FS_PROT_STATE_PRM_UNAVAIL);
      // we should also wake up all readers and writers!
      break;
    }
    default:
      Assert(0);
    }
  }


  // weak root status
  bool
  ProtocolLazyInvalidProxy::clearWeakRoot() {
    if (isWeakRoot() && a_readers.isEmpty() && a_writers.isEmpty()) {
      // get rid of the tokens...
      switch (a_token) {
      case LIT_WRITER:
	a_token = LIT_READER;
	sendToManager(LI_WRITE_INVALIDATED, a_proxy->retrieveEntityState());
	// fall through
      case LIT_READER:
	a_token = LIT_INVALID;
	sendToManager(LI_READ_INVALIDATED);
	return true;
      default:
	Assert(0); break;
      }
    }
    return false;
  }


  // interpret site failures
  FaultState
  ProtocolLazyInvalidProxy::siteStateChanged(DSite* s,
					     const DSiteState& state) {
    if (a_token != LIT_FAILED && a_proxy->m_getCoordinatorSite() == s) {
      switch (state) {
      case DSite_OK:
	return FS_PROT_STATE_OK;
      case DSite_TMP:
	return FS_PROT_STATE_TMP_UNAVAIL;
      case DSite_GLOBAL_PRM: case DSite_LOCAL_PRM:
	a_token = LIT_FAILED;
	return FS_PROT_STATE_PRM_UNAVAIL;
      default:
	dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
      }
    }
    return 0;
  }

} //End namespace
