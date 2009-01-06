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
#if defined(INTERFACE)
#pragma implementation "protocol_immediate.hh"
#endif

#include "protocol_immediate.hh"
namespace _dss_internal{ //Start namespace
  //  namespace{
//      enum IM_Cont {
//         IM_C_FULL = 0,
//         IM_C_SKEL = 1
//       };
//   }

  ProtocolImmediateManager::ProtocolImmediateManager(){;}

  void
  ProtocolImmediateManager::msgReceived(MsgContainer *msg, DSite* sender){;}

  bool
  ProtocolImmediateProxy::m_initRemoteProt(DssReadBuffer*) {
    return true;
  }

  bool
  ProtocolImmediateProxy::marshal_protocol_info(DssWriteBuffer*, DSite*) {
    return true;
  }

  bool
  ProtocolImmediateProxy::dispose_protocol_info(DssReadBuffer*) {
    return true;
  }

  void
  ProtocolImmediateProxy::msgReceived(MsgContainer *msg, DSite* u) {}

  OpRetVal
  ProtocolImmediateProxy::operationRead(GlobalThread*,
                                        PstOutContainerInterface**&) {
    return DSS_PROCEED;
  }

  ProtocolImmediateProxy::ProtocolImmediateProxy():ProtocolProxy(PN_IMMEDIATE){;}

} //end namespace
