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
#pragma implementation "protocol_invalid.hh"
#endif

#include "protocol_invalid.hh"

namespace _dss_internal{ //Start namespace

  // Quick description of the protocol.
  //
  // This protocol implements a two-phase commit scheme for updating a
  // mutable entity.  All proxies have a copy of the current state of
  // the entity.  Read operations are purely local, while write
  // operations are performed by the manager.  For the latter, the
  // manager asks proxies to temporarily invalidate their state.  This
  // guarantees consistent causality between sites.
  //
  // Proxy P requests for reading (once in the eager case, required
  // after invalidation in the lazy case):
  //
  //    P                M
  //    |----INV_READ--->|
  //    |<--INV_COMMIT---| if M is not in invalidation phase
  //
  // Proxy P requests a write operation: (1) P sends the operation to
  // manager M.  M starts an invalidation phase, if no other request
  // is pending.  (2) Once all proxies have invalidated their state, M
  // performs all write operations, and sends results back to proxies.
  // (3) Then M commits the new entity state to all reader proxies.
  //
  // (1) P                    M                    P'
  //     |-----INV_WRITE----->|                    |
  //     |<--INV_INVALIDATE---|---INV_INVALIDATE-->|
  //     |                   ...                   |
  // (2) |----INV_INVALID---->|<----INV_INVALID----|
  //     |<----INV_RESULT-----|                    | M performs operation
  //     |                   ...                   |
  // (3) |<----INV_COMMIT-----|-----INV_COMMIT---->|
  //
  // Proxies that fail are simply discarded by the manager, without
  // affecting the entity's state.  Write operations are performed by
  // the manager in order to avoid proxy dependencies in terms of
  // failures.

  namespace {
    enum Invalid_Message {
      INV_READ,         // p2m: proxy asks for reading
      INV_WRITE,        // p2m: proxy requests write operation
      INV_RETURN,       // m2p: manager returns operation result
      INV_INVALIDATE,   // m2p: asks proxy to invalidate
      INV_INVALID,      // p2m: proxy notifies invalidation
      INV_COMMIT        // m2p: commit new entity state
    };
  }

  // duplicate a PstInContainerInterface
  inline
  PstInContainerInterface* duplicate(PstInContainerInterface* pst) {
    if (pst == NULL) return NULL;
    PstOutContainerInterface* pst1 = pst->loopBack2Out();
    PstInContainerInterface* pst2 = pst1->loopBack2In();
    pst1->dispose();
    return pst2;
  }



  /******************** ProtocolInvalidManager ********************/

  ProtocolInvalidManager::ProtocolInvalidManager(DSite *s, bool isLazy) {
    a_readers.push(s);
    a_valid = 1;
    setStatus(isLazy);
  }

  void ProtocolInvalidManager::sendMigrateInfo(MsgContainer* msg) {
    ProtocolManager::sendMigrateInfo(msg);
    msgPush(msg, a_readers.size());
    for (Position<DSite*> p(a_readers); p(); p++) msgPush(msg, *p);
    msgPush(msg, a_valid);
    msgPush(msg, a_requests.size());
    for (Position<Request> p(a_requests); p(); p++) {
      msgPush(msg, (*p).arg->loopBack2Out());
      if ((*p).caller) {
	msgPush(msg, true); msgPush(msg, (*p).caller);
      } else {
	msgPush(msg, false);
      }
    }
  }

  ProtocolInvalidManager::ProtocolInvalidManager(MsgContainer* msg) :
    ProtocolManager(msg)
  {
    int len = msg->popIntVal();
    while (len--) a_readers.push(msg->popDSiteVal());
    a_valid = msg->popIntVal();
    len = msg->popIntVal();
    while (len--) {
      PstInContainerInterface* arg = duplicate(gf_popPstIn(msg));
      GlobalThread* caller = msg->popIntVal() ? popThreadId(msg) : NULL;
      a_requests.append(Request(caller, arg));
    }
  }

  ProtocolInvalidManager::~ProtocolInvalidManager() {
    while (!a_requests.isEmpty()) a_requests.pop().dispose();
  }

  void ProtocolInvalidManager::makeGCpreps() {
    ProtocolManager::makeGCpreps();
    t_gcList(a_readers);
    for (Position<Request> p(a_requests); p(); p++) (*p).makeGCpreps();
  }


  void  ProtocolInvalidManager::printStatus(){
    if (isPermFail()) {
      printf("Failed\n");
    } else {
      printf("Readers: %d valid among\n", a_valid); 
      for (Position<DSite*> p(a_readers); p(); p++)
	printf("   %s\n", (*p)->m_stringrep());
      if (a_requests.isEmpty()) {
	printf("No write request\n");
      } else {
	printf("Write requests from\n");
	for (Position<Request> p(a_requests); p(); p++)
	  printf("   %s\n", (*p).caller ?
		 (*p).caller->m_getGUIdSite()->m_stringrep() : "unknown");
      }
    }
  }


  // register a proxy as a reader
  void
  ProtocolInvalidManager::m_register(DSite* s) {
    Assert(!a_readers.contains(s));
    a_readers.push(s);
    if (a_requests.isEmpty()) m_commit(s);
  }

  // ask all readers to invalidate their state
  void 
  ProtocolInvalidManager::m_invalidate() {
    Assert(!a_requests.isEmpty());
    for (Position<DSite*> p(a_readers); p(); p++)
      sendToProxy(*p, INV_INVALIDATE);
    // just in case we are ready already...
    m_checkOperations();
  }

  // count a proxy as invalidated, remove it from a_readers if
  // required, and check write operations
  void
  ProtocolInvalidManager::m_invalid(DSite* s, bool remove) {
    Assert(remove || a_readers.contains(s));
    if (!remove || a_readers.remove(s)) {
      a_valid--; m_checkOperations();
    }
  }

  // perform operations if all proxies have invalidated their state
  void 
  ProtocolInvalidManager::m_checkOperations(){
    if (a_valid > 0) return;

    // process all write requests
    while (!a_requests.isEmpty()) {
      Request req = a_requests.pop();

      // perform operation, and return result
      PstOutContainerInterface* ans = NULL;
      AOcallback ret =
	a_coordinator->m_doe(AO_STATE_WRITE, req.caller, NULL, req.arg, ans);
      Assert(ret == AOCB_FINISH);
      if (req.caller)
	sendToProxy(req.caller->m_getGUIdSite(), INV_RETURN, req.caller, ans);

      // dispose Psts
      if (req.caller == NULL && ans) ans->dispose();
      req.dispose();
    }
    // all requests have been served, commit new state
    for (Position<DSite*> p(a_readers); p(); p++) m_commit(*p);
  }

  // commit new state to one proxy
  void 
  ProtocolInvalidManager::m_commit(DSite *s) {
    sendToProxy(s, INV_COMMIT, a_coordinator->retrieveEntityState());
    a_valid++;
  }

  // notify permfail to all registered proxies
  void
  ProtocolInvalidManager::m_failed() {
    while (!a_readers.isEmpty()) a_readers.pop();
    while (!a_requests.isEmpty()) a_requests.pop().dispose();
    makePermFail();
  }


  void
  ProtocolInvalidManager::msgReceived(::MsgContainer* msg, DSite* sender){
    if (isPermFail()) {
      sendToProxy(sender, PROT_PERMFAIL); return;
    }
    int message = msg->popIntVal();
    switch (message) {
    case PROT_REGISTER: {
      registerProxy(sender);
      break;
    }
    case PROT_DEREGISTER: {
      deregisterProxy(sender);
      m_invalid(sender, REMOVE);
      break;
    }
    case INV_READ: {
      m_register(sender);
      break;
    }
    case INV_WRITE: {
      bool trigger = a_requests.isEmpty();
      PstInContainerInterface* arg = duplicate(gf_popPstIn(msg));
      GlobalThread* caller = msg->m_isEmpty() ? NULL : popThreadId(msg);
      a_requests.append(Request(caller, arg));
      if (trigger) m_invalidate();
      break;
    }
    case INV_INVALID: {
      m_invalid(sender, m_isLazy());
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


  // handle proxy failures
  void
  ProtocolInvalidManager::m_siteStateChange(DSite* s,
					    const FaultState& state) {
    if (!isPermFail() && state == FS_GLOBAL_PERM) {
      deregisterProxy(s); m_invalid(s, REMOVE);
    }
  }



  /******************** ProtocolInvalidProxy ********************/

  ProtocolInvalidProxy::ProtocolInvalidProxy(bool isLazy):
    ProtocolProxy(isLazy ? PN_LAZY_INVALID : PN_EAGER_INVALID), a_numRead(0) {
    if (isLazy) m_setLazy();
    m_setReader(true);
    m_setValid(true);          // default for home proxy
  }

  bool
  ProtocolInvalidProxy::m_initRemoteProt(DssReadBuffer*) {
    m_setReader(false);
    m_setValid(false);
    if (!m_isLazy()) m_subscribe();     // subscribe now if eager
    return false;
  }

  ProtocolInvalidProxy::~ProtocolInvalidProxy(){
    Assert(a_susps.isEmpty()); 
    if (!a_proxy->m_isHomeProxy()) protocol_Deregister();
  }

  
  void ProtocolInvalidProxy::m_subscribe() {
    sendToManager(INV_READ);
    m_setReader(true);
  }

  OpRetVal
  ProtocolInvalidProxy::operationRead(GlobalThread* th_id,
				      PstOutContainerInterface**& msg){
    dssLog(DLL_BEHAVIOR,"InvalidProxy::Read");
    msg = NULL;
    if (isPermFail()) return DSS_RAISE;
    if (m_isValid()) return DSS_PROCEED;   // state is valid
    if (!m_isReader()) m_subscribe();
    a_susps.push(th_id);
    a_numRead++;
    return DSS_SUSPEND;
  }

  OpRetVal
  ProtocolInvalidProxy::operationWrite(GlobalThread* th_id,
				       PstOutContainerInterface**& msg){
    dssLog(DLL_BEHAVIOR,"InvalidProxy::Write");
    msg = NULL;
    if (isPermFail()) return DSS_RAISE;
    if (th_id) {
      sendToManager(INV_WRITE, UnboundPst(msg), th_id);
      a_susps.append(th_id);
    } else {
      sendToManager(INV_WRITE, UnboundPst(msg));
    }
    return DSS_SUSPEND;
  }


  void
  ProtocolInvalidProxy::msgReceived(::MsgContainer* msg, DSite*){
    if (isPermFail()) return;
    int message = msg->popIntVal();
    switch (message) {
    case INV_RETURN: {
      GlobalThread* caller = popThreadId(msg);
      PstInContainerInterface* ans = gf_popPstIn(msg);
      caller->resumeRemoteDone(ans);
      a_susps.remove(caller);
      break;
    }
    case INV_INVALIDATE: {
      m_setValid(false);
      if (m_isLazy()) m_setReader(false);
      sendToManager(INV_INVALID);
      break;
    }
    case INV_COMMIT: {
      PstInContainerInterface* val = gf_popPstIn(msg);
      a_proxy->installEntityState(val);
      m_setValid(true);
      // wake up read operations
      while (a_numRead > 0) {
	a_susps.pop()->resumeDoLocal(NULL);
	a_numRead--;
      }
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
  ProtocolInvalidProxy::siteStateChanged(DSite* s, const FaultState& state) {
    if (!isPermFail() && a_proxy->m_getCoordinatorSite() == s) {
      switch (state) {
      case FS_OK:          return FS_STATE_OK;
      case FS_TEMP:        return FS_STATE_TEMP;
      case FS_LOCAL_PERM:  makePermFail(state); return FS_STATE_LOCAL_PERM;
      case FS_GLOBAL_PERM: makePermFail(state); return FS_STATE_GLOBAL_PERM;
      default:
	dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
      }
    }
    return 0;
  }

} //End namespace
