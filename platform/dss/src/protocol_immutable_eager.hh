/*
 *  Authors:
 *    Per Sahlin (sahlin@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Per Sahlin, 2003
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
#ifndef __PROTOCOL_IMMUTABLE_EAGER_HH
#define __PROTOCOL_IMMUTABLE_EAGER_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"
#include "dss_templates.hh"

namespace _dss_internal{ //Start namespace

  // The eager and lazy immutable protocols are almost identical.
  // Their managers are identical, and their proxies only differ in
  // the moment they request the state.  I have therefore factored out
  // the common stuff here.

  class ProtocolImmutableManager : public ProtocolManager {
  public:
    ProtocolImmutableManager() {}
    ProtocolImmutableManager(MsgContainer*);
    ~ProtocolImmutableManager() {}
    virtual void sendMigrateInfo(MsgContainer*);
    virtual void msgReceived(MsgContainer*,DSite*);
  };

  class ProtocolImmutableProxy : public ProtocolProxy {
    // the status of the proxy tells whether the state is installed
  public: 
    ProtocolImmutableProxy(const ProtocolName&);
    ~ProtocolImmutableProxy();

    virtual bool m_initRemoteProt(DssReadBuffer*);

    void m_requestState();
    void m_installState(PstInContainerInterface*);

    virtual void msgReceived(MsgContainer*,DSite*);   

    virtual FaultState siteStateChanged(DSite*, const FaultState&);
  };



  // Now comes the specific stuff for the eager protocol:

  class ProtocolImmutableEagerManager : public ProtocolImmutableManager {
  public:
    ProtocolImmutableEagerManager() : ProtocolImmutableManager() {}
    ProtocolImmutableEagerManager(MsgContainer* msg) :
      ProtocolImmutableManager(msg) {}
  };

  class ProtocolImmutableEagerProxy : public ProtocolImmutableProxy {
  public: 
    ProtocolImmutableEagerProxy() :
      ProtocolImmutableProxy(PN_IMMUTABLE_EAGER) {}

    virtual bool m_initRemoteProt(DssReadBuffer*); 

    virtual OpRetVal operationRead(GlobalThread*, PstOutContainerInterface**&);
  };

} //End namespace
#endif
