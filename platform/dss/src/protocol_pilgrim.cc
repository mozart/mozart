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
  //    |-----PLGM_ENTER----->|                     | P is inserted
  //    |<--PLGM_FORWARD(P2)--|---PLGM_FORWARD(P)-->| between P1 and P2
  //
  // Proxy P wants to leave the ring:
  //    P                     M                     P1
  //    |-----PLGM_LEAVE----->|                     | P was between
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
  //    |<---PROT_PERMFAIL-----| if token has not been encountered
  //
  // All proxies that enter the ring are automatically registered in
  // a_proxies; deregistration is manual, however.  The manager keeps
  // track of the proxies that left the ring, and forwards them the
  // new color after the coloring.  A proxy that has a different color
  // knows that it is unreachable from the ring.
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
      PLGM_ENTER,       // PM: proxy wants to be inserted in ring
      PLGM_LEAVE,       // PM: proxy wants to leave the ring
      PLGM_FORWARD,     // MP: manager assigns new successor to proxy
      PLGM_TOKEN,       // PP: pass token
      PLGM_COLOR_START, // PM: proxy requests a color change (no arg),
                        // MP: manager launches a color change
      PLGM_COLOR,       // PP: change color
      PLGM_COLOR_DONE,  // PM,MP: notify termination of coloring
      PLGM_FAILED       // PM: proxy asks manager to remove failed proxy
    };
  }



  /******************** ProtocolPilgrimManager ********************/

  ProtocolPilgrimManager::ProtocolPilgrimManager(DSite* s) :
    a_lastLeaving(false), a_color(1) {
    a_ring.push(s);
    registerProxy(s);
  }

  void
  ProtocolPilgrimManager::sendMigrateInfo(::MsgContainer* msg){
    Assert(0);
  }

  ProtocolPilgrimManager::ProtocolPilgrimManager(::MsgContainer* msg) {
    Assert(0);
  }


  void
  ProtocolPilgrimManager::m_lostToken() {
    makePermFail();
    while (!a_ring.isEmpty()) a_ring.pop();   // empty the ring
  }

  void
  ProtocolPilgrimManager::m_removeFailed(DSite* s) {
    deregisterProxy(s);
    if (a_ring.find(s)) { // remove s from the ring
      a_ring.pop();
      if (a_ring.isEmpty()) {
        m_lostToken(); return;
      }
      sendToProxy(a_ring.predecessor(), PLGM_FORWARD, a_ring.current());
    }
    // initiate a color change, in order to diagnose a possible failure
    a_color++;
    sendToProxy(a_ring.current(), PLGM_COLOR_START, (int) a_color);
  }


  void
  ProtocolPilgrimManager::msgReceived(::MsgContainer* msg, DSite* sender){
    if (isPermFail()) {
      sendToProxy(sender, PROT_PERMFAIL); return;
    }
    int message = msg->popIntVal();
    switch(message) {
    case PROT_REGISTER: { // explicit registration
      registerProxy(sender);
      break;
    }
    case PROT_DEREGISTER: {
      Assert(!a_ring.find(sender));
      deregisterProxy(sender);
      break;
    }
    case PLGM_ENTER: {
      Assert(!a_ring.find(sender) || a_lastLeaving);
      if (!isRegisteredProxy(sender)) registerProxy(sender);
      if (a_lastLeaving) {
        Assert(a_ring.size() == 1);
        if (a_ring.current() != sender) {
          // the proxy in the ring still wants to leave; it can leave
          // the ring now, and the sender will thus be alone
          sendToProxy(a_ring.current(), PLGM_FORWARD, sender);
          sendToProxy(sender, PLGM_FORWARD, sender);
          a_ring.pop();
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
    case PLGM_LEAVE: {
      if (a_ring.find(sender)) {
        if (a_ring.size() <= 1) {
          // the proxy is alone, and cannot leave the ring right now
          a_lastLeaving = true;
          // force the home proxy to register, that may help
          if (sender != a_coordinator->m_getEnvironment()->a_myDSite) {
            ProtocolProxy* pp = a_coordinator->m_getProxy()->m_getProtocol();
            static_cast<ProtocolPilgrimProxy*>(pp)->m_enter();
          }
        } else {
          a_ring.pop();
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
        // send a_color to all registered proxies out of the ring
        for (Position<DSite*> p(a_proxies); p(); p++)
          if (!a_ring.find(*p)) sendToProxy(*p, PLGM_COLOR_DONE, (int) a_color);
      }
      break;
    }
    case PLGM_FAILED: {
      DSite* s = msg->popDSiteVal();
      m_removeFailed(s);
      break;
    }
    case PROT_PERMFAIL: {
      m_lostToken();
      break;
    }
    default:
      Assert(0);
    }
  }


  void
  ProtocolPilgrimManager::m_siteStateChange(DSite* s,
                                            const FaultState &state) {
    if (!isPermFail() && state == FS_GLOBAL_PERM && isRegisteredProxy(s))
      m_removeFailed(s);
  }



  /******************** ProtocolPilgrimProxy ********************/

  ProtocolPilgrimProxy::ProtocolPilgrimProxy(DSite *s) :
    ProtocolProxy(PN_PILGRIM_STATE), a_successor(s), a_freeRounds(0),
    a_color(1), a_jobsLeft(0)
  {
    hasToken(true); isInRing(true); isReachable(true);
    isColoring(false); isStrict(true);
    setRegistered(true);
  }

  bool
  ProtocolPilgrimProxy::m_initRemoteProt(DssReadBuffer*) {
    hasToken(false); isInRing(false); isReachable(false);
    a_successor = NULL;
    return false;
  }

  ProtocolPilgrimProxy::~ProtocolPilgrimProxy() {
    if (!a_proxy->m_isHomeProxy()) protocol_Deregister();
  }

  void
  ProtocolPilgrimProxy::makeGCpreps() {
    ProtocolProxy::makeGCpreps();
    if (a_successor) a_successor->m_makeGCpreps();
  }

  bool
  ProtocolPilgrimProxy::clearWeakRoot() {
    if (!isReachable()) return true;
    if (isInRing()) {
      m_leave();
    } else {
      sendToManager(PLGM_COLOR_START);
    }
    return false;
  }

  void
  ProtocolPilgrimProxy::m_enter() {
    if (isInRing()) return;
    sendToManager(PLGM_ENTER);
    isInRing(true); isReachable(true); isColoring(false);
    setRegistered(true);
    a_successor = NULL;
    a_color.whiten();
  }

  void
  ProtocolPilgrimProxy::m_leave() {
    Assert(isInRing());
    sendToManager(PLGM_LEAVE);
    isInRing(false);
  }

  bool
  ProtocolPilgrimProxy::m_isAlone() {
    return a_successor == a_proxy->m_getEnvironment()->a_myDSite;
  }

  void
  ProtocolPilgrimProxy::m_forwardToken() {
    Assert(hasToken() && a_jobsLeft == 0);
    if (a_successor && !m_isAlone()) {
      sendToProxy(a_successor, PLGM_TOKEN,
                  a_proxy->deinstallEntityState(), (int) a_color);
      hasToken(false);
    }
  }

  bool
  ProtocolPilgrimProxy::m_acceptTokenColor(PilgrimColor const &col) {
    if (a_color.isWhite()) a_color = col;
    return !isStrict() || a_color == col;
  }

  void
  ProtocolPilgrimProxy::m_forwardColor() {
    isColoring(true);
    if (a_successor) {
      sendToProxy(a_successor, PLGM_COLOR, (int) a_color);
      isColoring(false);
    }
  }

  void
  ProtocolPilgrimProxy::m_resumeOperations(){
    while (!a_susps.isEmpty()) {
      a_susps.pop()->resumeDoLocal();
    }
  }

  void
  ProtocolPilgrimProxy::m_lostToken() {
    makePermFail();
    a_successor = NULL;
  }


  OpRetVal
  ProtocolPilgrimProxy::protocol_Access(GlobalThread* const id,
                                        PstOutContainerInterface**& msg) {
    msg = NULL;
    if (isPermFail()) return DSS_RAISE;
    if (hasToken()) return DSS_PROCEED;

    // the token is not here, register if necessary, and wait
    if (!isInRing()) m_enter();
    a_susps.append(id);
    return DSS_SUSPEND;
  }

  OpRetVal
  ProtocolPilgrimProxy::operationRead(GlobalThread* thr,
                                      PstOutContainerInterface**& out) {
    return protocol_Access(thr, out);
  }

  OpRetVal
  ProtocolPilgrimProxy::operationWrite(GlobalThread* thr,
                                       PstOutContainerInterface**& out) {
    return protocol_Access(thr, out);
  }


  void
  ProtocolPilgrimProxy::msgReceived(::MsgContainer* msg, DSite* sender){
    if (isPermFail()) return;
    int message = msg->popIntVal();
    switch(message) {
    case PLGM_FORWARD: {
      a_successor = msg->popDSiteVal();
      if (isColoring()) m_forwardColor();
      if (hasToken() && a_jobsLeft == 0) m_forwardToken();
      break;
    }
    case PLGM_TOKEN: {
      PstInContainerInterface* builder = gf_popPstIn(msg);
      PilgrimColor col = msg->popIntVal();
      if (m_acceptTokenColor(col)) {
        hasToken(true);
        a_color.darken();
        isStrict(true);
        a_proxy->installEntityState(builder);
        if (a_susps.isEmpty()) { // this is a free round
          if (isInRing()) { // check counter, and possibly deregister
            a_freeRounds--;
            if (a_freeRounds == 0) m_leave();
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
        if (hasToken()) a_color.darken();
        isStrict(hasToken());
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
          sendToManager(PROT_PERMFAIL);
          m_lostToken();
        }
      } else if (a_color < col) { // use the most recent color only
        a_color = col;
        if (hasToken()) a_color.darken();
        m_forwardColor();
      }
      break;
    }
    case PLGM_COLOR_DONE: {
      PilgrimColor col = msg->popIntVal();
      if (!isInRing() && isReachable() && !(a_color == col)) isReachable(false);
      break;
    }
    case PROT_PERMFAIL: {
      m_lostToken();
      break;
    }
    default:
      Assert(0);
    }
  }

  // interpret site failures
  FaultState
  ProtocolPilgrimProxy::siteStateChanged(DSite* s, const FaultState& state) {
    if (isPermFail()) return 0;
    if (s == a_proxy->m_getCoordinatorSite()) {
      switch (state) {
      case FS_OK:          return FS_STATE_OK;
      case FS_LOCAL_PERM:  makePermFail(state); return FS_STATE_LOCAL_PERM;
      case FS_GLOBAL_PERM: m_lostToken(); return FS_STATE_GLOBAL_PERM;
      default:             return 0;
      }
    }
    if (s == a_successor && state == FS_GLOBAL_PERM) {
      a_successor = NULL;
      sendToManager(PLGM_FAILED, s);
    }
    return 0;
  }

} //End namespace
