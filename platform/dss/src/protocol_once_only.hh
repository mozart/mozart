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

  // status of transient entity, when not permfail (used by both
  // transient protocols)
  enum TransientStatus {
    TRANS_STATUS_FREE,      // not bound yet
    TRANS_STATUS_WAITING,   // bind/kill attempt sent, waiting for result
    TRANS_STATUS_BOUND      // bound (final state)
  };


  class ProtocolOnceOnlyManager : public ProtocolManager {
  private:
    ProtocolOnceOnlyManager(const ProtocolOnceOnlyManager&) {}
    ProtocolOnceOnlyManager& operator=(const ProtocolOnceOnlyManager&)
    { return *this; }

  public:
    ProtocolOnceOnlyManager(DSite* const site);
    ProtocolOnceOnlyManager(MsgContainer* const msg) : ProtocolManager(msg) {}
    ~ProtocolOnceOnlyManager() {}

    // inherited from ProtocolManager: makeGCpreps(),
    // sendMigrateInfo(), m_siteStateChange()

    void registerRemote(DSite*);   // for remote proxies only
    void sendRedirect(DSite*);

    void msgReceived(MsgContainer*,DSite*);
  };



  class ProtocolOnceOnlyProxy : public ProtocolProxy {
  private:
    // Note. We use a_susps from ProtocolProxy.  a_susps used to be a
    // TwoContainer<GlobalThread,ProtOOop>.  I simplified it, because
    // in practice we do not need to know on which operation a
    // GlobalThread suspends.

    ProtocolOnceOnlyProxy(const ProtocolOnceOnlyProxy&) :
      ProtocolProxy(PN_TRANSIENT) {}
    ProtocolOnceOnlyProxy& operator=(const ProtocolOnceOnlyProxy&)
    { return *this; }

  public:
    ProtocolOnceOnlyProxy();
    ~ProtocolOnceOnlyProxy();

    virtual OpRetVal operationBind(GlobalThread*, PstOutContainerInterface**&);
    virtual OpRetVal operationAppend(GlobalThread*,PstOutContainerInterface**&);
    virtual OpRetVal operationKill();

    virtual bool isWeakRoot() { return !a_susps.isEmpty(); }

    void msgReceived(MsgContainer*,DSite*);

    // Marshaling and unmarshaling proxy information
    virtual bool marshal_protocol_info(DssWriteBuffer *buf, DSite*);
    virtual bool dispose_protocol_info(DssReadBuffer *buf );
    virtual int  getMarshaledSize() const { return 1; }
    virtual bool m_initRemoteProt(DssReadBuffer*);

    // check fault state
    virtual FaultState siteStateChanged(DSite*, const FaultState&);
  };

} //End namespace
#endif
