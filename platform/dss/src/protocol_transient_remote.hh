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

#ifndef __ONCE_TRANSIENT_REMOTE_PROTOCOL_HH
#define __ONCE_TRANSIENT_REMOTE_PROTOCOL_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"
#include "dss_templates.hh"

// for TransientStatus
#include "protocol_once_only.hh"

namespace _dss_internal{ //Start namespace

  // for buffered requests
  struct TR_request {
    int type; int aop; PstOutContainerInterface* pst; GlobalThread* thr;
    void makeGCpreps() { if (thr) thr->m_makeGCpreps(); }
    void dispose() { if (pst) pst->dispose(); }
  };

  class ProtocolTransientRemoteManager : public ProtocolManager {
  private:
    DSite* a_current;   // the proxy that has the write token
    SimpleQueue<TR_request> a_requests;   // buffered requests

    ProtocolTransientRemoteManager(const ProtocolTransientRemoteManager&):
      a_current(NULL) {}
    ProtocolTransientRemoteManager&
    operator=(const ProtocolTransientRemoteManager&) { return *this; }

  public:
    ProtocolTransientRemoteManager(DSite* const site);
    ~ProtocolTransientRemoteManager();

    void sendMigrateInfo(MsgContainer*);
    ProtocolTransientRemoteManager(MsgContainer* const);

    void makeGCpreps();

    // register a remote proxy.  registerToken() returns true if the
    // proxy is given the write token
    void registerRemote(DSite*);
    bool registerToken(DSite*);
    void setCurrent(DSite*);     // change current token holder

    void sendRedirect(DSite*);
    void msgReceived(MsgContainer*, DSite*);

    // check failed proxies
    void m_siteStateChange(DSite*, const FaultState&);
  };



  class ProtocolTransientRemoteProxy : public ProtocolProxy {
  private:
    ProtocolTransientRemoteProxy(const ProtocolTransientRemoteProxy&):
      ProtocolProxy(PN_NO_PROTOCOL) {}
    ProtocolTransientRemoteProxy&
    operator=(const ProtocolTransientRemoteProxy&) { return *this; }

  public:
    ProtocolTransientRemoteProxy();
    ~ProtocolTransientRemoteProxy();

    int getStatus() const { return ProtocolProxy::getStatus() >> 1; }
    bool hasToken() const { return ProtocolProxy::getStatus() & 1; }
    void setStatus(int v) {
      ProtocolProxy::setStatus((v << 1) | (hasToken() ? 1 : 0)); }
    void setToken(bool b) {
      ProtocolProxy::setStatus((getStatus() << 1) | (b ? 1 : 0)); }

    OpRetVal protocol_Terminate(GlobalThread* const th_id,
				::PstOutContainerInterface**& msg,
				const AbsOp& aop);
    OpRetVal protocol_Update(GlobalThread* const th_id,
			     ::PstOutContainerInterface**& msg,
			     const AbsOp& aop);
    OpRetVal protocol_Kill();

    bool isWeakRoot() { return !a_susps.isEmpty(); }

    void msgReceived(::MsgContainer*,DSite*);
  
    // Marshaling and unmarshaling proxy information
    virtual bool marshal_protocol_info(DssWriteBuffer *buf, DSite*);
    virtual bool dispose_protocol_info(DssReadBuffer *buf );
    virtual int  getMarshaledSize() const { return 1; }
    virtual bool m_initRemoteProt(DssReadBuffer*);
    
    virtual void
    remoteInitatedOperationCompleted(DssOperationId*,
				     PstOutContainerInterface*) { Assert(0); }
    virtual void localInitatedOperationCompleted() { Assert(0); }

    // check fault state
    virtual FaultState siteStateChanged(DSite*, const FaultState&);
  };

} //End namespace
#endif 
