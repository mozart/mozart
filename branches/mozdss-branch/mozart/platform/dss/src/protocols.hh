/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Erik Klintskog, 1998
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

#ifndef __PROTOCOL_HH
#define __PROTOCOL_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "coordinator.hh"
#include "dss_threads.hh"

namespace _dss_internal{ //Start namespace

  // This struct eases the building of messages where an unbound
  // PstOutContainer is pushed.
  struct UnboundPst {
    PstOutContainerInterface*** ptr;
    UnboundPst(PstOutContainerInterface** &m) : ptr(&m) {}
  };


  // basic set of messages used by all protocols
  enum PROT_Messages {
    PROT_REGISTER = -3,       // PM: register proxy at manager
    PROT_DEREGISTER = -2,     // PM: remote proxy registration
    PROT_PERMFAIL = -1        // **: make entity state permanently failed
  };


  class ProtocolManager{
  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif
    Coordinator* a_coordinator;

    int                a_status;      // generic status (bit 0 = permfail)
    SimpleList<DSite*> a_proxies;     // list of registered proxies

  public:
    ProtocolManager() : a_coordinator(NULL), a_status(0), a_proxies() {
      DebugCode(a_allocated++); }
    virtual ~ProtocolManager(){ DebugCode(a_allocated--); }

    inline AbstractEntityName getAEname(){ Assert(0); return AEN_NOT_DEFINED;}

    virtual void msgReceived(MsgContainer*,DSite*)=0;
    virtual void makeGCpreps() { t_gcList(a_proxies); }

    // those two automatically migrate a_status and a_proxies.  Extra
    // info can be safely put after these in the MsgContainer.
    virtual void sendMigrateInfo(MsgContainer*);
    ProtocolManager(MsgContainer*);

    // Called when the state of a site changes.  It is up to the
    // protocol to deduce if it is affected by the site.  By default
    // it removes failed sites from a_proxies.
    virtual void m_siteStateChange(DSite*, const DSiteState&);

    // basic protocol functionalities
    int getStatus() const { return a_status >> 1; }
    void setStatus(int v) { a_status = (v << 1) | (a_status & 1); }

    bool isRegisteredProxy(DSite* s) const { return a_proxies.contains(s); }
    void registerProxy(DSite* s) { a_proxies.push(s); }
    void deregisterProxy(DSite* s) { a_proxies.remove(s); }

    bool isPermFail() const { return a_status & 1; }
    void makePermFail();   // send PROT_PERMFAIL to all registered proxies

    // Templates to build and send messages
    bool sendToProxy(DSite* s);
    template <typename A>
    bool sendToProxy(DSite* s, A const &a);
    template <typename A, typename B>
    bool sendToProxy(DSite* s, A const &a, B const &b);
    template <typename A, typename B, typename C>
    bool sendToProxy(DSite* s, A const &a, B const &b, C const &c);
    template <typename A, typename B, typename C, typename D>
    bool sendToProxy(DSite* s, A const &a, B const &b, C const &c, D const &d);

    // This one simplifies code...
    GlobalThread* popThreadId(MsgContainer* msg) {
      return gf_popThreadIdVal(msg, a_coordinator->m_getEnvironment()); }

    MACRO_NO_DEFAULT_CONSTRUCTORS(ProtocolManager);
  };


  class ProtocolProxy{
  private:
    ProtocolName p_name;
  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif
    Proxy* a_proxy;

    int a_status;     // generic status (bit 0 = permfail, bit 1 = registered)
    SimpleQueue<GlobalThread*> a_susps;     // suspended threads

  public:
    ProtocolProxy(const ProtocolName& name) :
      p_name(name), a_proxy(NULL), a_status(0), a_susps()
    { DebugCode(a_allocated++); }
    virtual ~ProtocolProxy() { DebugCode(a_allocated--); }

    // ** SERVICES **
    inline Proxy       *getProxy() { return a_proxy; }
    inline ProtocolName getProtocolName() { return p_name; }
    
    // ** PROTOCOL OPERATIONS **
    virtual void msgReceived(MsgContainer*,DSite*)=0;

    // Usually if we were a weak root we cannot change the state
    virtual bool isWeakRoot() { return false; }
    virtual bool clearWeakRoot() { return false; }
    virtual void makeGCpreps() { t_gcList(a_susps); }

    // Marshaling and unmarshaling proxy information.  The information
    // is used to initialize a remote proxy, or disposed if the proxy
    // already exists.
    virtual bool marshal_protocol_info(DssWriteBuffer*, DSite*) { return true; }
    virtual bool m_initRemoteProt(DssReadBuffer*) { return true; }
    virtual bool dispose_protocol_info(DssReadBuffer*) { return true; }

    // Called when the state of a site changes. It is up to the
    // protocol to deduce if it is affected by the site. 
    virtual FaultState siteStateChanged(DSite*, const DSiteState&) {
      return 0; }

    //located in dss_access.cc, the only placed accessed from
    virtual char *m_stringrep();

    // finalize remote callbacks and local resumptions
    virtual void remoteInitatedOperationCompleted(DssOperationId*,
						  PstOutContainerInterface*)=0;
    virtual void localInitatedOperationCompleted() = 0;

    // basic protocol functionalities
    int getStatus() const { return a_status >> 2; }
    void setStatus(int v) { a_status = (v << 2) | (a_status & 3); }

    bool isRegistered() const { return a_status & 2; }
    void setRegistered(bool r) { if (r) a_status |= 2; else a_status &= ~2; }
    OpRetVal protocol_Register();     // send PROT_REGISTER to manager
    OpRetVal protocol_Deregister();   // send PROT_DEREGISTER to manager

    bool isPermFail() const { return a_status & 1; }
    void makePermFail();
    OpRetVal protocol_Kill();      // send PROT_PERMFAIL to manager

    // Templates to build and send messages
    bool sendToManager();
    template <typename A>
    bool sendToManager(A const &a);
    template <typename A, typename B>
    bool sendToManager(A const &a, B const &b);
    template <typename A, typename B, typename C>
    bool sendToManager(A const &a, B const &b, C const &c);
    template <typename A, typename B, typename C, typename D>
    bool sendToManager(A const &a, B const &b, C const &c, D const &d);

    bool sendToProxy(DSite* s);
    template <typename A>
    bool sendToProxy(DSite* s, A const &a);
    template <typename A, typename B>
    bool sendToProxy(DSite* s, A const &a, B const &b);
    template <typename A, typename B, typename C>
    bool sendToProxy(DSite* s, A const &a, B const &b, C const &c);
    template <typename A, typename B, typename C, typename D>
    bool sendToProxy(DSite* s, A const &a, B const &b, C const &c, D const &d);

    // This one simplifies code...
    GlobalThread* popThreadId(MsgContainer* msg) {
      return gf_popThreadIdVal(msg, a_proxy->m_getEnvironment()); }

    MACRO_NO_DEFAULT_CONSTRUCTORS(ProtocolProxy);
  };


  // This overloaded function eases the buiding of messages.
  inline void msgPush(MsgContainer* msg, int v) { msg->pushIntVal(v); }
  inline void msgPush(MsgContainer* msg, DSite* v) { msg->pushDSiteVal(v); }
  inline void msgPush(MsgContainer* msg, GlobalThread* v) {
    gf_pushThreadIdVal(msg, v); }
  inline void msgPush(MsgContainer* msg, PstOutContainerInterface* v) {
    gf_pushPstOut(msg, v); }
  inline void msgPush(MsgContainer* msg, UnboundPst v) {
    *(v.ptr) = gf_pushUnboundPstOut(msg); }

  // Manager to Proxy
  inline bool
  ProtocolManager::sendToProxy(DSite* s) {
    MsgContainer* msgC = a_coordinator->m_createProxyProtMsg();
    return s->m_sendMsg(msgC);
  }
  template <typename A> inline bool
  ProtocolManager::sendToProxy(DSite* s, A const &a) {
    MsgContainer* msgC = a_coordinator->m_createProxyProtMsg();
    msgPush(msgC, a);
    return s->m_sendMsg(msgC);
  }
  template <typename A, typename B> inline bool
  ProtocolManager::sendToProxy(DSite* s, A const &a, B const &b) {
    MsgContainer* msgC = a_coordinator->m_createProxyProtMsg();
    msgPush(msgC, a); msgPush(msgC, b);
    return s->m_sendMsg(msgC);
  }
  template <typename A, typename B, typename C> inline bool
  ProtocolManager::sendToProxy(DSite* s, A const &a, B const &b, C const &c) {
    MsgContainer* msgC = a_coordinator->m_createProxyProtMsg();
    msgPush(msgC, a); msgPush(msgC, b); msgPush(msgC, c);
    return s->m_sendMsg(msgC);
  }
  template <typename A, typename B, typename C, typename D> inline bool
  ProtocolManager::sendToProxy(DSite* s, A const &a, B const &b, C const &c, D const &d) {
    MsgContainer* msgC = a_coordinator->m_createProxyProtMsg();
    msgPush(msgC, a); msgPush(msgC, b); msgPush(msgC, c); msgPush(msgC, d);
    return s->m_sendMsg(msgC);
  }

  // Proxy to Manager
  inline bool
  ProtocolProxy::sendToManager() {
    MsgContainer* msgC = a_proxy->m_createCoordProtMsg();
    return a_proxy->m_sendToCoordinator(msgC);
  }
  template <typename A> inline bool
  ProtocolProxy::sendToManager(A const &a) {
    MsgContainer* msgC = a_proxy->m_createCoordProtMsg();
    msgPush(msgC, a);
    return a_proxy->m_sendToCoordinator(msgC);
  }
  template <typename A, typename B> inline bool
  ProtocolProxy::sendToManager(A const &a, B const &b) {
    MsgContainer* msgC = a_proxy->m_createCoordProtMsg();
    msgPush(msgC, a); msgPush(msgC, b);
    return a_proxy->m_sendToCoordinator(msgC);
  }
  template <typename A, typename B, typename C> inline bool
  ProtocolProxy::sendToManager(A const &a, B const &b, C const &c) {
    MsgContainer* msgC = a_proxy->m_createCoordProtMsg();
    msgPush(msgC, a); msgPush(msgC, b); msgPush(msgC, c);
    return a_proxy->m_sendToCoordinator(msgC);
  }
  template <typename A, typename B, typename C, typename D> inline bool
  ProtocolProxy::sendToManager(A const &a, B const &b, C const &c, D const &d) {
    MsgContainer* msgC = a_proxy->m_createCoordProtMsg();
    msgPush(msgC, a); msgPush(msgC, b); msgPush(msgC, c); msgPush(msgC, d);
    return a_proxy->m_sendToCoordinator(msgC);
  }

  // Proxy to Proxy
  inline bool
  ProtocolProxy::sendToProxy(DSite* s) {
    MsgContainer* msgC = a_proxy->m_createProxyProtMsg();
    return s->m_sendMsg(msgC);
  }
  template <typename A> inline bool
  ProtocolProxy::sendToProxy(DSite* s, A const &a) {
    MsgContainer* msgC = a_proxy->m_createProxyProtMsg();
    msgPush(msgC, a);
    return s->m_sendMsg(msgC);
  }
  template <typename A, typename B> inline bool
  ProtocolProxy::sendToProxy(DSite* s, A const &a, B const &b) {
    MsgContainer* msgC = a_proxy->m_createProxyProtMsg();
    msgPush(msgC, a); msgPush(msgC, b);
    return s->m_sendMsg(msgC);
  }
  template <typename A, typename B, typename C> inline bool
  ProtocolProxy::sendToProxy(DSite* s, A const &a, B const &b, C const &c) {
    MsgContainer* msgC = a_proxy->m_createProxyProtMsg();
    msgPush(msgC, a); msgPush(msgC, b); msgPush(msgC, c);
    return s->m_sendMsg(msgC);
  }
  template <typename A, typename B, typename C, typename D> inline bool
  ProtocolProxy::sendToProxy(DSite* s, A const &a, B const &b, C const &c, D const &d) {
    MsgContainer* msgC = a_proxy->m_createProxyProtMsg();
    msgPush(msgC, a); msgPush(msgC, b); msgPush(msgC, c); msgPush(msgC, d);
    return s->m_sendMsg(msgC);
  }

  // Creating protocol proxies and managers
  void gf_createProtocolProxyManager(ProtocolName, DSS_Environment*,
				     ProtocolManager*&, ProtocolProxy*&);
  ProtocolProxy* gf_createRemoteProxy(ProtocolName prot, DSite* myDSite);
  ProtocolManager *gf_createProtManager(MsgContainer* msgC, ProtocolName pn);

} //End namespace
#endif 
