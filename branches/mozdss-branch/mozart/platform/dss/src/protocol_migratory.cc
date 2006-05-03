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
#pragma implementation "protocol_migratory.hh"
#endif

#include "protocol_migratory.hh"
#include "dssBase.hh"

namespace _dss_internal{ //Start namespace

  // Quick description of the protocol.
  //
  // The protocol makes a unique token migrate among proxies, and the
  // state of the abstract entity migrates with the token.  The proxy
  // that has the token can update the entity locally.  The protocol
  // dynamically builds a forwarding chain among the proxies that want
  // to update the abstract entity.
  //
  // Proxy P asks for the token:
  //    P              M                   P'
  //    |---MIGM_GET-->|                   |   P' is the last proxy
  //    |              |--MIGM_FORWARD(P)->|   in the forwarding chain
  //
  // Proxy P forwards the token (after reception):
  //    P                  P'
  //    |----MIGM_TOKEN--->|
  //
  // Proxy P has the token but no successor:
  //    P                     M
  //    |---MIGM_TOKEN_HERE-->|   (not mandatory, optimization only)
  //
  // Proxy P wants to get rid of the token:
  //    P                       M
  //    |---MIGM_NEED_NO_MORE-->|   (P has the token and no successor)
  //
  // The following part extends the protocol above in order to avoid
  // failed proxies, and diagnose the loss of the token.
  //
  // P detects that its successor P' failed:
  //    P                      M
  //    |---MIGM_FAILED_SUCC-->|
  //    |<--MIGM_FORWARD(P")---|   if P" is the successor of P'
  //
  // Manager inquires the predecessor of a failed proxy:
  //    M                     P
  //    |---MIGM_CHECK_SUCC-->|
  //    |<---MIGM_OLD_SUCC----|   if P already forwarded token
  // or:
  //    |<--MIGM_FAILED_SUCC--|   if P has not forwarded yet
  //
  // Manager inquires the successor of a failed proxy (only when the
  // latter has no predecessor):
  //    M                     P
  //    |---MIGM_CHECK_PRED-->|
  //    |<---MIGM_OLD_PRED----|   if P already forwarded token
  // or:
  //    |<--MIGM_TOKEN_HERE---|   if P has the token
  // or:
  //    |<--MIGM_FAILED_PRED--|   if P has not received it yet
  //
  // In the latter reply, we can infer that P will never receive the
  // token.  It is therefore lost, and the manager notifies proxies
  // with MIGM_PERMFAIL.
  //
  // Proxy P makes the entity fail (if not failed yet):
  //    P                   M                   P'
  //    |---MIGM_PERMFAIL-->|                   |
  //    |<--MIGM_PERMFAIL---|---MIGM_PERMFAIL-->|   (sent to all proxies)

  namespace {
    // messages
    enum Migratory_Message {
      MIGM_GET,          // PM: request access
      MIGM_FORWARD,      // MP: send the successor of a proxy
      MIGM_TOKEN,        // PP: pass to token from proxy to proxy
      MIGM_TOKEN_HERE,   // PM: informs manager
      MIGM_NEED_NO_MORE, // PM: please remove the token from here
      MIGM_CHECK_SUCC,   // MP: manager inquiring to find token
      MIGM_CHECK_PRED,   // MP: manager inquiring to find token
      MIGM_FAILED_SUCC,  // PM: proxy will not forward to its successor
      MIGM_FAILED_PRED,  // PM: proxy will not receive from this successor
      MIGM_OLD_SUCC,     // PM: reply to an old request
      MIGM_OLD_PRED,     // PM: reply to an old request
      MIGM_PERMFAIL      // MP,PM: token is lost
    };
  }



  /******************** ProtocolMigratoryManager ********************/

  ProtocolMigratoryManager::ProtocolMigratoryManager(DSite* mysite) {
    a_chain.append(makePair(mysite, 0));
    a_last = mysite;    // the token is at home
  }


  void
  ProtocolMigratoryManager::sendMigrateInfo(::MsgContainer* msg) {
    for (Position<Pair<DSite*, int> > p(a_chain); p(); p++) {
      msg->pushDSiteVal((*p).first);
      msg->pushIntVal((*p).second);
    }
  }

  ProtocolMigratoryManager::ProtocolMigratoryManager(::MsgContainer* msg) {
    a_last = NULL;
    while (!msg->m_isEmpty()) {
      a_last = msg->popDSiteVal();
      a_chain.append(makePair(a_last, msg->popIntVal()));
    }
  }


  void
  ProtocolMigratoryManager::msgReceived(::MsgContainer* msg, DSite* sender) {
    int msgType = msg->popIntVal();
    switch (msgType) {
    case MIGM_GET: {
      if (a_last == NULL) sendToProxy(sender, MIGM_PERMFAIL);
      else if (sender != a_last) {
	int reqid = msg->popIntVal();
	sendToProxy(a_last, MIGM_FORWARD, sender);
	if (a_chain.front().find(sender)) {
	  // remove everything up to the sender
	  while (a_chain.pop().first != sender) ;
	}
	a_chain.append(makePair(sender, reqid));
	a_last = sender;
      }
      break;
    }
    case MIGM_TOKEN_HERE: {
      if (a_last && a_chain.front().find(sender)) {
	Position<Pair<DSite*,int> > p(a_chain);
	// remove all elements before sender
	while ((*p).first != sender) p.pop();
      }
      break;
    }
    case MIGM_NEED_NO_MORE:{
      if (sender == a_last) {
	// The sender has no successor; force the home proxy to
	// request the state.
	ProtocolProxy* pp = a_coordinator->m_getProxy()->m_getProtocol();
	static_cast<ProtocolMigratoryProxy*>(pp)->requestToken();
      }
      break;
    }
    case MIGM_FAILED_SUCC: {
      // a proxy notifies that its successor has failed
      if (a_last) {
	Position<Pair<DSite*,int> > p(a_chain);
	p.find(sender); p++;     // p is on the sender's successor
	Assert(p());
	p.remove();              // remove the successor
	if (p()) { // forward to the new successor
	  sendToProxy(sender, MIGM_FORWARD, (*p).first);
	} else { // we removed the last proxy in a_chain
	  a_chain.check();
	  a_last = sender;
	}
      }
      break;
    }
    case MIGM_PERMFAIL:
    case MIGM_FAILED_PRED: {
      // The sender tells that it will not receive the token; it is
      // therefore lost (we know the predecessor has had it).
      if (a_last) lostToken();
      break;
    }
    case MIGM_OLD_SUCC:
    case MIGM_OLD_PRED: {
      // The sender informs the manager that it no longer has the
      // state (for the given request).
      if (a_last) {
	int reqid = msg->popIntVal();
	Position<Pair<DSite*,int> > p(a_chain);
	if (p.find(sender) && (*p).second == reqid) {
	  // drop all elements up to sender
	  while (a_chain.pop().first != sender) ;
	  
	  if (msgType == MIGM_OLD_SUCC) {
	    // see whether a successor has had the token
	    inquire(a_chain.front().element().first);
	  }
	}
      }
      break;
    }
    default:
      Assert(0);
    }
  }


  // inquiry protocol around a failed proxy
  void ProtocolMigratoryManager::inquire(DSite* s) {
    Position<Pair<DSite*,int> > cur(a_chain);
    bool found = false;
    Position<Pair<DSite*,int> > other;
    
    // first try to find the closest non-failed predecessor
    while ((*cur).first != s) {
      if ((*cur).first->m_getFaultState() <= DSite_TMP)
	found = true, other = cur;
      cur++;
    }
    if (found) { // we have a predecessor, inquire it
      sendToProxy((*other).first, MIGM_CHECK_SUCC, (*other).second);
      return;
    }
    
    // try to find the closest non-failed successor, then
    cur++;
    while (cur()) {
      if ((*cur).first->m_getFaultState() <= DSite_TMP)
	found = true, other = cur;
      cur++;
    }
    if (found) { // we have a successor, inquire it
      sendToProxy((*other).first, MIGM_CHECK_PRED, (*other).second);
      return;
    }
    
    // all proxies in the chain failed; we've lost the state
    while (!a_chain.isEmpty()) a_chain.pop();
    lostToken();
  }

  // notify proxies that the token has been lost
  void ProtocolMigratoryManager::lostToken() {
    a_last = NULL;
    // notify eagerly the home proxy (take the short cut...)
    ProtocolProxy* pp = a_coordinator->m_getProxy()->m_getProtocol();
    static_cast<ProtocolMigratoryProxy*>(pp)->lostToken();
    // notify all proxies left in a_chain
    while (!a_chain.isEmpty())
      sendToProxy(a_chain.pop().first, MIGM_PERMFAIL);
  }


  // check for failed proxies
  void ProtocolMigratoryManager::m_siteStateChange(DSite* s,
						   const DSiteState& state) {
    if (a_last && state >= DSite_GLOBAL_PRM && a_chain.front().find(s))
      inquire(s);
  }



  /******************** ProtocolMigratoryProxy ********************/

  ProtocolMigratoryProxy::ProtocolMigratoryProxy() :
    ProtocolProxy(PN_MIGRATORY_STATE),
    a_token(MIGT_HERE), a_successor(NULL), a_request(0) {}
  
  bool 
  ProtocolMigratoryProxy::m_initRemoteProt(DssReadBuffer*) {
    a_token = MIGT_EMPTY;
    return true;
  }

  void
  ProtocolMigratoryProxy::makeGCpreps() {
    if (a_successor) a_successor->m_makeGCpreps();
    t_gcList(a_susps);
  }


  void
  ProtocolMigratoryProxy::requestToken(){
    Assert(a_token == MIGT_EMPTY);
    dssLog(DLL_BEHAVIOR,"MigratoryProxy::requestToken");
    sendToManager(MIGM_GET, a_request);
    a_token = MIGT_REQUESTED;
  }

  void
  ProtocolMigratoryProxy::forwardToken(){
    Assert(a_token == MIGT_HERE && a_successor);
    dssLog(DLL_BEHAVIOR,"MigratoryProxy::forwardToken : To:%s",
	   a_successor->m_stringrep());
    sendToProxy(a_successor, MIGM_TOKEN, a_proxy->deinstallEntityState());
    a_token = MIGT_EMPTY;
    a_successor = NULL;
    a_request++;
  }

  void
  ProtocolMigratoryProxy::resumeOperations(){
    Assert(!a_susps.isEmpty() || a_proxy->m_isHomeProxy());
    // wake up all suspended threads
    while (!a_susps.isEmpty())
      a_susps.pop()->resumeDoLocal(NULL);   // NULL is not cool...
    // forward the token
    if (a_successor)
      forwardToken();
    else
      sendToManager(MIGM_TOKEN_HERE);
  }

  void
  ProtocolMigratoryProxy::lostToken() {
    // we have lost the state, notify the upper layers
    a_token = MIGT_LOST;
    a_successor = NULL;
    a_proxy->updateFaultState(FS_PROT_STATE_PRM_UNAVAIL);
    // resume suspended threads
    while (!a_susps.isEmpty()) a_susps.pop()->resumeFailed();
  }


  OpRetVal
  ProtocolMigratoryProxy::protocol_Access(GlobalThread* const id,
					  ::PstOutContainerInterface**& out){
    dssLog(DLL_BEHAVIOR,"MigratoryProxy::Access operation (%d)",a_token);
    out = NULL; // Never transmit
    switch(a_token){
    case MIGT_HERE:
      return DSS_PROCEED;
    case MIGT_EMPTY:
      requestToken();
      // fall through
    case MIGT_REQUESTED:
      a_susps.append(id);
      return DSS_SUSPEND;
    case MIGT_LOST:
      return DSS_RAISE;
    default:
      Assert(0);
      return DSS_INTERNAL_ERROR_SEVERE; // Whatever....
    }
  }

  // kill the entity
  OpRetVal
  ProtocolMigratoryProxy::protocol_Kill() {
    if (a_token != MIGT_LOST) sendToManager(MIGM_PERMFAIL);
    return DSS_SKIP;
  }

  
  void
  ProtocolMigratoryProxy::msgReceived(::MsgContainer* msg, DSite* sender){
    int msgType = msg->popIntVal();
    switch (msgType) {
    case MIGM_FORWARD:{
      if (a_token == MIGT_LOST) break;   // should not happen
      Assert(a_successor == NULL);
      a_successor = msg->popDSiteVal();
      if (a_token == MIGT_HERE) forwardToken();
      break;
    }
    case MIGM_TOKEN:{
      if (a_token == MIGT_LOST) break;   // Yes, we have to drop it!
      a_token = MIGT_HERE;
      ::PstInContainerInterface* builder = gf_popPstIn(msg);
      a_proxy->installEntityState(builder);
      resumeOperations();
      break;
    }
    case MIGM_CHECK_SUCC: {
      if (a_token == MIGT_LOST) break;
      int reqid = msg->popIntVal();
      if (a_request != reqid) { // this is an old request
	sendToManager(MIGM_OLD_SUCC, reqid);
      } else if (a_successor) { // ask for a new successor, then!
	a_successor = NULL;
	sendToManager(MIGM_FAILED_SUCC);
      }
      break;
    }
    case MIGM_CHECK_PRED: {
      if (a_token == MIGT_LOST) break;
      int reqid = msg->popIntVal();
      if (a_request != reqid) { // this is an old request
	sendToManager(MIGM_OLD_PRED, reqid);
      } else if (a_token == MIGT_HERE) { // the token is here
	sendToManager(MIGM_TOKEN_HERE);
      } else { // we will never receive the token
	sendToManager(MIGM_FAILED_PRED);
	lostToken();     // early failure (beware!)
      }
      break;
    }
    case MIGM_PERMFAIL: {
      if (a_token == MIGT_LOST) break;
      lostToken();
      break;
    }
    default:
      Assert(0);
    }
  }

  bool
  ProtocolMigratoryProxy::clearWeakRoot(){
    // Must check isWeakRoot() again since we don't know if something
    // happened since last check
    if (isWeakRoot()) {
      if (a_token == MIGT_HERE) {
	if (a_successor) forwardToken();
	else sendToManager(MIGM_NEED_NO_MORE);
      }
      return false; //Whatever happened we've only tried to clear it
    }
    return true;
  }

  // interpret a site failure
  FaultState
  ProtocolMigratoryProxy::siteStateChanged(DSite* s, const DSiteState& state) {
    if (a_token != MIGT_LOST) {
      if (a_proxy->m_getCoordinatorSite() == s) {
	switch (state) {
	case DSite_OK:
	  return FS_PROT_STATE_OK;
	case DSite_TMP:
	  return FS_PROT_STATE_TMP_UNAVAIL;
	case DSite_GLOBAL_PRM: case DSite_LOCAL_PRM:
	  lostToken();
	  return FS_PROT_STATE_PRM_UNAVAIL;
	default:
	  dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
	}
      }
      if (a_successor == s && state >= DSite_GLOBAL_PRM) {
	a_successor = NULL;
	sendToManager(MIGM_FAILED_SUCC);
      }
    }
    return 0;
  }

} //End namespace
