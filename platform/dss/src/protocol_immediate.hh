/*
 *  Authors:
 *    Per Sahlin (sahlin@sics.se)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
#ifndef __PROTOCOL_IMMEDIATE_HH
#define __PROTOCOL_IMMEDIATE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "dss_comService.hh"
#include "protocols.hh"
namespace _dss_internal{ //Start namespace

  class ProtocolImmediateManager:public ProtocolManager{
  public:
    ProtocolImmediateManager();
    virtual void msgReceived(MsgContainer*,DSite*);
  };


  class ProtocolImmediateProxy:public ProtocolProxy{
    // bool stateHolder: true;
  public:
    ProtocolImmediateProxy();
    virtual void msgReceived(MsgContainer*,DSite*);
    virtual void remoteInitatedOperationCompleted(DssOperationId*, PstOutContainerInterface*);
    virtual void localInitatedOperationCompleted();

    virtual bool marshal_protocol_info(DssWriteBuffer *buf, DSite *);
    virtual bool dispose_protocol_info(DssReadBuffer *buf); //töm bufferten
    virtual bool m_initRemoteProt(DssReadBuffer*);

    virtual OpRetVal operationMonitor() { return DSS_INTERNAL_ERROR_NO_OP; }
    virtual OpRetVal operationKill() { return DSS_INTERNAL_ERROR_NO_OP; }
    virtual OpRetVal operationRead(GlobalThread*, PstOutContainerInterface**&);
  };


} //End namespace
#endif
