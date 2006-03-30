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

namespace _dss_internal{ //Start namespace

  class ProtocolTransientRemoteManager : public ProtocolManager {
  private:
    SimpleList<DSite*> a_proxies;     // the registered proxies
    bool a_bound;                     // whether the transient is bound
    DSite *a_current;                 // the proxy that has the write token

    // Invariant: a_current is not a member of a_proxies if it is remote.

    ProtocolTransientRemoteManager(const ProtocolTransientRemoteManager&):
      a_proxies(), a_bound(false), a_current(NULL) {}
    ProtocolTransientRemoteManager&
    operator=(const ProtocolTransientRemoteManager&) { return *this; }

  public:
    ProtocolTransientRemoteManager(DSite* const site);
    ProtocolTransientRemoteManager(::MsgContainer * const);
    ~ProtocolTransientRemoteManager();

    void makeGCpreps();
    void msgReceived(::MsgContainer*,DSite*);
    void sendRedirect(DSite*);
    void sendMigrateInfo(MsgContainer*); 

    // register a remote proxy
    void register_remote(DSite*);
    // register a proxy, and returns true if it is given the write token
    bool register_token(DSite*);
  };



  class ProtocolTransientRemoteProxy : public ProtocolProxy {
    friend class ProtocolTransientRemoteManager;

  private:
    SimpleList<GlobalThread*> a_susps;     // suspended threads
    bool a_bound;                          // whether the transient is bound
    bool a_writeToken;                     // whether this has the write token

    ProtocolTransientRemoteProxy(const ProtocolTransientRemoteProxy&):
      ProtocolProxy(PN_NO_PROTOCOL), a_susps(), a_bound(false),
      a_writeToken(false) {}
    ProtocolTransientRemoteProxy&
    operator=(const ProtocolTransientRemoteProxy&) { return *this; }

    // wake up the suspended threads
    void wkSuspThrs(); 

  public:
    ProtocolTransientRemoteProxy();
    ~ProtocolTransientRemoteProxy();

    OpRetVal protocol_Terminate(GlobalThread* const th_id,
				::PstOutContainerInterface**& msg,
				const AbsOp& aop);
    OpRetVal protocol_Update(GlobalThread* const th_id,
			     ::PstOutContainerInterface**& msg,
			     const AbsOp& aop);

    bool isWeakRoot() { return !a_susps.isEmpty(); }

    void makeGCpreps(); //threads should be guarded from the glue as well as...
    void msgReceived(::MsgContainer*,DSite*);
  
    // Marshaling and unmarshaling proxy information
    virtual bool marshal_protocol_info(DssWriteBuffer *buf, DSite*);
    virtual bool dispose_protocol_info(DssReadBuffer *buf );
    virtual bool m_initRemoteProt(DssReadBuffer*);
    
    virtual void
    remoteInitatedOperationCompleted(DssOperationId* opId,
				     PstOutContainerInterface* pstOut) {}
    virtual void localInitatedOperationCompleted() { Assert(0); }

    // check fault state
    virtual FaultState siteStateChanged(DSite*, const DSiteState&);
  };

} //End namespace
#endif 
