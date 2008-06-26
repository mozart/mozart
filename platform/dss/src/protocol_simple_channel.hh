/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 *
 *  Copyright:
 *    Erik Klintskog, 2002
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
#ifndef __PROTOCOL_SIMPLE_CHANNEL_HH
#define __PROTOCOL_SIMPLE_CHANNEL_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"

namespace _dss_internal{ //Start namespace

  class ProtocolSimpleChannelProxy;

  class ProtocolSimpleChannelManager:public ProtocolManager{
  private:
    ProtocolSimpleChannelManager(const ProtocolSimpleChannelManager&){};
    ProtocolSimpleChannelManager& operator=(const ProtocolSimpleChannelManager&){ return *this; }
    ProtocolSimpleChannelProxy* homeProxy() const;
  public:
    ProtocolSimpleChannelManager(DSite*);
    ProtocolSimpleChannelManager(::MsgContainer *);
    ~ProtocolSimpleChannelManager(){}

    void msgReceived(::MsgContainer*,DSite*);
    void sendMigrateInfo(::MsgContainer*);
  };


  class ProtocolSimpleChannelProxy:public ProtocolProxy{
    // This proxy if the state holder iff getStatus() returns true.
  public:
    ProtocolSimpleChannelProxy();
    ~ProtocolSimpleChannelProxy();

    OpRetVal protocol_Synch(GlobalThread* const th_id, ::PstOutContainerInterface**& msg, const AbsOp& aop);
    OpRetVal protocol_Asynch(::PstOutContainerInterface**& msg, const AbsOp& aop);
    virtual OpRetVal operationRead(GlobalThread*, PstOutContainerInterface**&);
    virtual OpRetVal operationWrite(GlobalThread*, PstOutContainerInterface**&);
    virtual OpRetVal operationWrite(PstOutContainerInterface**&);

    void do_operation(DSite*, GlobalThread*, AbsOp, PstInContainerInterface*);
    void remoteInitatedOperationCompleted(DssOperationId*, PstOutContainerInterface*);
    void localInitatedOperationCompleted() { Assert(0); }

    void msgReceived(::MsgContainer*,DSite*);
    bool m_initRemoteProt(DssReadBuffer*);

    // check fault state
    virtual FaultState siteStateChanged(DSite*, const FaultState&);
  };

} //End namespace
#endif
