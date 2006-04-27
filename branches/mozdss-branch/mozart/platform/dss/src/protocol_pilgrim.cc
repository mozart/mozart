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
#pragma implementation "protocol_pilgrim.hh"
#endif

#include "protocol_pilgrim.hh"

namespace _dss_internal{ //Start namespace

  // Quick description of the protocol.
  //
  // This protocol can be seen as a variant of the migratory token
  // protocol, where the proxies accessing the token form a ring
  // instead of a chain.  Each proxy in the ring has a successor, to
  // which it forwards the token.  The manager inserts and removes
  // proxies in the ring.  It reduces the interaction with the manager
  // when a set of proxies regularly access the token.
  //
  // Proxy P wants to enter the ring:
  //    P                     M                     P1
  //    |----PLGM_REGISTER--->|                     | P is inserted
  //    |<--PLGM_FORWARD(P2)--|---PLGM_FORWARD(P)-->| between P1 and P2
  //
  // Proxy P wants to leave the ring:
  //    P                     M                     P1
  //    |---PLGM_DEREGISTER-->|                     | P was between
  //    |                     |---PLGM_FORWARD(P2)->| P1 and P2
  //
  // Proxy P forwards the token (after reception):
  //    P                  P'
  //    |----MIGM_TOKEN--->|
  //
  // This basic protocol allows to make state updates.  Beside it
  // there is a proxy coloring protocol, which is used both for
  // garbage collection and failure detection.
  //
  // All the proxies and the token in the ring should have the same
  // color.  At any time the manager can change the color.  The color
  // is changed from proxy to proxy, following the ring structure.  If
  // the token has not been encountered, it is considered to be lost.
  // To ensure this, the proxies do not accept a token with a wrong
  // color, except the proxy that initiated the coloring.
  //
  // The color is changed each time a proxy fails.  It is also changed
  // when a proxy wants to clear its "weak root" status.  The proxy is
  // guaranteed to be unreachable if its color has not been changed by
  // the process.
  //
  // Proxy requests a color change:
  //    P                      M
  //    |---PLGM_COLOR_START-->|   which triggers...
  //
  // Manager initiates a color change:
  //    M                      P1               P2      ...      Pn
  //    |---PLGM_COLOR_START-->|                |                |
  //    |                      |---PLGM_COLOR-->|                |
  //    |                      |                |       ...      |
  //    |                      |                                 |
  //    |                      |<----------PLGM_COLOR------------|
  //    |                      |
  //    |<--PLGM_COLOR_DONE----| if token has been encountered
  // or:
  //    |<---PLGM_PERMFAIL-----| if token has not been encountered
  //
  // The manager keeps track of all deregistered proxies, and forwards
  // the new color after the coloring.  If the token was lost, the
  // manager sends PLGM_PERMFAIL to all the proxies it knows of.
  //
  // Note that the coloring protocol is robust, and permits several
  // concurrent color changes.  Eventually all proxies change to the
  // most recent color.
  //
  // Proxy P discards it successor P1:
  //    P                  M
  //    |---PLGM_FAILED--->|
  //    |<--PLGM_FORWARD---|   if P was in the ring
  //
  // After such an event, the manager initiates a color change.

  namespace {
    enum Pilgrim_Message {
      PLGM_REGISTER,    // PM: proxy wants to be inserted in ring
      PLGM_DEREGISTER,  // PM: proxy wants to leave the ring
      PLGM_FORWARD,     // MP: manager assigns new successor to proxy
      PLGM_TOKEN,       // PP: pass token
      PLGM_COLOR_START, // PM: proxy requests a color change (no arg),
			// MP: manager launches a color change
      PLGM_COLOR,       // PP: change color
      PLGM_COLOR_DONE,  // PM,MP: notify termination of coloring
      PLGM_GONE,        // PM: proxy tells it is no longer reachable
      PLGM_FAILED,      // PM: proxy asks manager to remove failed proxy
      PLGM_PERMFAIL     // PM,MP: make the entity permfail
    };
  }



  /******************** ProtocolPilgrimManager ********************/

  ProtocolPilgrimManager::ProtocolPilgrimManager(DSite* s) :
    ProtocolManager(), a_ring(), a_lastLeaving(false), a_leaving(), a_color()
  {
    a_color++;
    a_ring.push(s);
  }

  void
  ProtocolPilgrimManager::sendMigrateInfo(::MsgContainer* msg){
    Assert(0); 
  }

  ProtocolPilgrimManager::ProtocolPilgrimManager(::MsgContainer* msg):
    ProtocolManager(), a_ring(), a_lastLeaving(false), a_leaving(), a_color()
  {
    Assert(0);
  }

  void 
  ProtocolPilgrimManager::makeGCpreps() {
    for (int n = a_ring.size(); n--; a_ring.step())
      a_ring.current()->m_makeGCpreps();
    t_gcList(a_leaving);
  }


  void
  ProtocolPilgrimManager::m_lostToken() {
    // notify as many proxies as possible
    sendToProxy(a_coordinator->m_getEnvironment()->a_myDSite, PLGM_PERMFAIL);
    while (!a_ring.isEmpty()) sendToProxy(a_ring.pop(), PLGM_PERMFAIL);
    while (!a_leaving.isEmpty()) sendToProxy(a_leaving.pop(), PLGM_PERMFAIL);
  }

  void
  ProtocolPilgrimManager::m_removeFailed(DSite* s) {
    if (a_ring.find(s)) { // remove s from the ring
      a_ring.pop();
      if (a_ring.isEmpty()) {
	m_lostToken(); return;
      }
      sendToProxy(a_ring.predecessor(), PLGM_FORWARD, a_ring.current());
    } else {
      a_leaving.remove(s);
    }
    // initiate a color change, in order to diagnose a possible failure
    a_color++;
    sendToProxy(a_ring.current(), PLGM_COLOR_START, (int) a_color);
  }


  void
  ProtocolPilgrimManager::msgReceived(::MsgContainer* msg, DSite* sender){
    if (a_ring.isEmpty()) { // failed
      sendToProxy(sender, PLGM_PERMFAIL);
      return;
    }
    int message = msg->popIntVal();
    switch(message) {
    case PLGM_REGISTER: {
      Assert(!a_ring.find(sender) || a_lastLeaving);
      a_leaving.remove(sender);
      if (a_lastLeaving) {
	Assert(a_ring.size() == 1);
	if (a_ring.current() != sender) {
	  // the proxy in the ring still wants to leave; it can leave
	  // the ring now, and the sender will thus be alone
	  sendToProxy(a_ring.current(), PLGM_FORWARD, sender);
	  sendToProxy(sender, PLGM_FORWARD, sender);
	  a_leaving.push(a_ring.pop());
	  a_ring.push(sender);
	}
	a_lastLeaving = false;
      } else {
	a_ring.push(sender);     // sender is now the current proxy
	sendToProxy(a_ring.predecessor(), PLGM_FORWARD, sender);
	sendToProxy(sender, PLGM_FORWARD, a_ring.successor());
      }
      break;
    }
    case PLGM_DEREGISTER: {
      if (a_ring.find(sender)) {
	if (a_ring.size() <= 1) {
	  // the proxy is alone, and cannot leave the ring right now
	  a_lastLeaving = true;
	  // force the home proxy to register, that may help
	  if (sender != a_coordinator->m_getEnvironment()->a_myDSite) {
	    ProtocolProxy* pp = a_coordinator->m_getProxy()->m_getProtocol();
	    static_cast<ProtocolPilgrimProxy*>(pp)->m_register();
	  }
	} else {
	  a_leaving.push(a_ring.pop());
	  sendToProxy(a_ring.predecessor(), PLGM_FORWARD, a_ring.current());
	}
      }
      break;
    }
    case PLGM_COLOR_START: {
      a_color++;
      sendToProxy(a_ring.current(), PLGM_COLOR_START, (int) a_color);
      break;
    }
    case PLGM_COLOR_DONE: {
      PilgrimColor col = msg->popIntVal();
      if (a_color == col) {
	for (Position<DSite*> p(a_leaving); p(); p++)
	  sendToProxy(*p, PLGM_COLOR_DONE, (int) a_color);
      }
      break;
    }
    case PLGM_GONE: {
      a_leaving.remove(sender);
      break;
    }
    case PLGM_FAILED: {
      DSite* s = msg->popDSiteVal();
      m_removeFailed(s);
      break;
    }
    case PLGM_PERMFAIL: {
      a_leaving.push(sender);
      m_lostToken();
      break;
    }
    default:
      Assert(0);
    }
  }


  void
  ProtocolPilgrimManager::m_siteStateChange(DSite* s,
					    const DSiteState &state) {
    if (a_ring.isEmpty()) return;
    if (state >= DSite_GLOBAL_PRM &&
	(a_ring.find(s) || a_leaving.contains(s))) {
      m_removeFailed(s);
    }
  }


  
  /******************** ProtocolPilgrimProxy ********************/

  ProtocolPilgrimProxy::ProtocolPilgrimProxy(DSite *s) :
    ProtocolProxy(PN_PILGRIM_STATE), a_token(PLGT_HERE), a_successor(s),
    a_registered(true), a_reachable(true), a_freeRounds(0), a_color(),
    a_coloring(false), a_strict(true), a_susps(), a_jobsLeft(0)
  {
    a_color++;
  }
    
  bool
  ProtocolPilgrimProxy::m_initRemoteProt(DssReadBuffer*) {
    a_token = PLGT_EMPTY;
    a_reachable = a_registered = false;
    a_successor = NULL;
    return true;
  }

  void
  ProtocolPilgrimProxy::m_register() {
    if (a_registered) return;
    sendToManager(PLGM_REGISTER);
    a_reachable = a_registered = true;
    a_successor = NULL;
    a_color.whiten();
    a_coloring = false;
  }

  void
  ProtocolPilgrimProxy::m_deregister() {
    Assert(a_registered);
    sendToManager(PLGM_DEREGISTER);
    a_registered = false;
  }

  bool
  ProtocolPilgrimProxy::m_isAlone() {
    return a_successor == a_proxy->m_getEnvironment()->a_myDSite;
  }

  void
  ProtocolPilgrimProxy::m_forwardToken() {
    Assert(a_token == PLGT_HERE && a_jobsLeft == 0);
    if (a_successor && !m_isAlone()) {
      sendToProxy(a_successor, PLGM_TOKEN,
		  a_proxy->deinstallEntityState(), (int) a_color);
      a_token = PLGT_EMPTY;
    }
  }

  bool
  ProtocolPilgrimProxy::m_acceptTokenColor(PilgrimColor const &col) {
    if (a_color.isWhite()) a_color = col;
    return !a_strict || a_color == col;
  }

  void
  ProtocolPilgrimProxy::m_forwardColor() {
    a_coloring = true;
    if (a_successor) {
      sendToProxy(a_successor, PLGM_COLOR, (int) a_color);
      a_coloring = false;
    }
  }

  void
  ProtocolPilgrimProxy::m_resumeOperations(){
    while (!a_susps.isEmpty()) {
      if (a_susps.pop()->resumeDoLocal(NULL) == WRV_CONTINUING) a_jobsLeft++; 
    }
    // a_jobsLeft is decremented when operations actually resume
  }

  void
  ProtocolPilgrimProxy::m_lostToken() {
    a_token = PLGT_LOST;
    a_reachable = a_registered = false;
    a_successor = NULL;
    a_proxy->updateFaultState(FS_PROT_STATE_PRM_UNAVAIL);
    // wake up all suspensions?
  }


  OpRetVal
  ProtocolPilgrimProxy::protocol_Access(GlobalThread* const id,
					PstOutContainerInterface**& msg) {
    msg = NULL;
    if (a_token == PLGT_HERE) return DSS_PROCEED;
    if (a_token == PLGT_LOST) return DSS_RAISE;

    // the token is not here, register if necessary, and wait
    if (!a_registered) m_register();
    a_susps.append(id);
    return DSS_SUSPEND;
  }

  // kill the entity
  OpRetVal
  ProtocolPilgrimProxy::protocol_Kill(GlobalThread* const th_id) {
    if (a_token == PLGT_LOST) return DSS_SKIP;
    if (!sendToManager(PLGM_PERMFAIL)) return DSS_RAISE;
    a_susps.append(th_id);
    return DSS_SUSPEND;
  }
  
  
  void
  ProtocolPilgrimProxy::msgReceived(::MsgContainer* msg, DSite* sender){
    if (a_token == PLGT_LOST) return;
    int message = msg->popIntVal();
    switch(message) {
    case PLGM_FORWARD: {
      a_successor = msg->popDSiteVal();
      if (a_coloring) m_forwardColor();
      if (a_token == PLGT_HERE && a_jobsLeft == 0) m_forwardToken();
      break;
    }
    case PLGM_TOKEN: {
      PstInContainerInterface* builder = gf_popPstIn(msg);
      PilgrimColor col = msg->popIntVal();
      if (m_acceptTokenColor(col)) {
	a_token = PLGT_HERE;
	a_color.darken();
	a_strict = true;
	a_proxy->installEntityState(builder);
	if (a_susps.isEmpty()) { // this is a free round
	  if (a_registered) { // check counter, and possibly deregister
	    a_freeRounds--;
	    if (a_freeRounds == 0) m_deregister();
	  }
	} else { // resume suspended operations
	  a_freeRounds = 3;
	  m_resumeOperations();
	}
	if (a_jobsLeft == 0) m_forwardToken();
      } else {
	m_lostToken();
      }
      break;
    }
    case PLGM_COLOR_START: {
      PilgrimColor col = msg->popIntVal();
      if (a_color.isWhite() || a_color < col) {
	a_color = col;
	if (a_token == PLGT_HERE) a_color.darken();
	a_strict = (a_token == PLGT_HERE);
	m_forwardColor();
      }
      break;
    }
    case PLGM_COLOR: {
      PilgrimColor col = msg->popIntVal();
      if (a_color == col) { // the coloring started here
	if (a_color.isDark() || col.isDark()) { // token was met
	  sendToManager(PLGM_COLOR_DONE, (int) col);
	} else { // token has not been met, and is therefore lost
	  sendToManager(PLGM_PERMFAIL);
	  m_lostToken();
	}
      } else if (a_color < col) { // use the most recent color only
	a_color = col;
	if (a_token == PLGT_HERE) a_color.darken();
	m_forwardColor();
      }
      break;
    }
    case PLGM_COLOR_DONE: {
      PilgrimColor col = msg->popIntVal();
      if (!a_registered && a_reachable && !(a_color == col)) {
	a_reachable = false;
	sendToManager(PLGM_GONE);
      }
      break;
    }
    case PLGM_PERMFAIL: {
      m_lostToken();
      break;
    }
    default:
      Assert(0); 
    }
  }
    

  bool
  ProtocolPilgrimProxy::isWeakRoot() {
    return a_reachable;
  }

  bool
  ProtocolPilgrimProxy::clearWeakRoot() {
    if (!a_reachable) return true;
    if (a_registered) {
      m_deregister();
    } else {
      sendToManager(PLGM_COLOR_START);
    }
    return false;
  }


  void   
  ProtocolPilgrimProxy::remoteInitatedOperationCompleted(DssOperationId* opId,PstOutContainerInterface* pstOut){
    // This should _NOT_ happend!!!
    Assert(0); 
  }

  void
  ProtocolPilgrimProxy::localInitatedOperationCompleted() {
    Assert(a_token == PLGT_HERE && a_jobsLeft > 0);
    a_jobsLeft--; 
    if (a_jobsLeft == 0) m_forwardToken();
  }

  ProtocolPilgrimProxy::~ProtocolPilgrimProxy() {}

  void 
  ProtocolPilgrimProxy::makeGCpreps() { 
    if (a_successor) a_successor->m_makeGCpreps();
    t_gcList(a_susps);
  }

  // interpret site failures
  FaultState
  ProtocolPilgrimProxy::siteStateChanged(DSite* s, const DSiteState& state) {
    if (a_token == PLGT_LOST) return 0;
    if (s == a_proxy->m_getCoordinatorSite()) {
      switch (state) {
      case DSite_OK:         return FS_PROT_STATE_OK;
      case DSite_GLOBAL_PRM:
      case DSite_LOCAL_PRM:  return FS_PROT_STATE_PRM_UNAVAIL;
      default:               return 0;
      }
    }
    if (s == a_successor && state >= DSite_GLOBAL_PRM) {
      a_successor = NULL;
      sendToManager(PLGM_FAILED, s);
    }
    return 0;
  }

} //End namespace
