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

#ifndef __ONCE_ONLY_PROTOCOL_HH
#define __ONCE_ONLY_PROTOCOL_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"
#include "dss_templates.hh"

namespace _dss_internal{ //Start namespace

  class ProtocolOnceOnlyManager : public ProtocolManager {
  private:
    SimpleList<DSite*> a_proxies;     // the registered proxies
    bool a_bound:1;                   // whether the transient is bound
    bool a_failed:1;                  // whether the state is permfail

    ProtocolOnceOnlyManager(const ProtocolOnceOnlyManager&) :
      a_proxies(), a_bound(false), a_failed(false) {}
    ProtocolOnceOnlyManager& operator=(const ProtocolOnceOnlyManager&)
    { return *this; }

  public:
    ProtocolOnceOnlyManager(DSite* const site);
    ProtocolOnceOnlyManager(MsgContainer * const);
    ~ProtocolOnceOnlyManager();

    void makeGCpreps();
    void msgReceived(MsgContainer*,DSite*);
    void sendRedirect(DSite*);
    void sendMigrateInfo(MsgContainer*); 

    // register a remote proxy
    void register_remote(DSite*);

    // check failed proxies
    void m_siteStateChange(DSite*, const DSiteState&);
  };



  class ProtocolOnceOnlyProxy : public ProtocolProxy {
  private:
    SimpleList<GlobalThread*> a_susps;   // suspended threads
    bool a_bound:1;                      // whether the transient is bound
    bool a_failed:1;                  // whether the state is permfail

    // Note. a_susps used to be a TwoContainer<GlobalThread,ProtOOop>.
    // I simplified it, because in practice we do not need to know on
    // which operation a GlobalThread suspends.

    ProtocolOnceOnlyProxy(const ProtocolOnceOnlyProxy&) :
      ProtocolProxy(PN_TRANSIENT), a_susps(), a_bound(false), a_failed(false)
    {}
    ProtocolOnceOnlyProxy& operator=(const ProtocolOnceOnlyProxy&)
    { return *this; }

  public:
    ProtocolOnceOnlyProxy();
    ~ProtocolOnceOnlyProxy();
    
    OpRetVal protocol_Terminate(GlobalThread* const th_id,
				::PstOutContainerInterface**& msg,
				const AbsOp& aop);
    OpRetVal protocol_Update(GlobalThread* const th_id,
			     ::PstOutContainerInterface**& msg,
			     const AbsOp& aop);
    OpRetVal protocol_Kill(GlobalThread* const th_id);

    virtual bool isWeakRoot() { return !a_susps.isEmpty(); }

    void makeGCpreps(); //threads should be guarded from the glue as well as...
    void msgReceived(MsgContainer*,DSite*);
  
    // Marshaling and unmarshaling proxy information
    virtual bool marshal_protocol_info(DssWriteBuffer *buf, DSite*);
    virtual bool dispose_protocol_info(DssReadBuffer *buf );
    virtual bool m_initRemoteProt(DssReadBuffer*);
  
    virtual void
    remoteInitatedOperationCompleted(DssOperationId* opId,
				     ::PstOutContainerInterface* pstOut) {} 
    virtual void localInitatedOperationCompleted() { Assert(0); }

    // check fault state
    virtual FaultState siteStateChanged(DSite*, const DSiteState&);
  };

} //End namespace
#endif 
