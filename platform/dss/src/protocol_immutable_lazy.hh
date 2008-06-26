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
#ifndef __PROTOCOL_IMMUTABLE_LAZY_HH
#define __PROTOCOL_IMMUTABLE_LAZY_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "protocol_immutable_eager.hh"

namespace _dss_internal{ //Start namespace

  // See protocol_immutable_eager.hh for ProtocolImmutableManager and
  // ProtocolImmutableProxy.

  class ProtocolImmutableLazyManager : public ProtocolImmutableManager {
  public:
    ProtocolImmutableLazyManager() : ProtocolImmutableManager() {}
    ProtocolImmutableLazyManager(MsgContainer* msg) :
      ProtocolImmutableManager(msg) {}
  };

  class ProtocolImmutableLazyProxy : public ProtocolImmutableProxy {
  public:
    ProtocolImmutableLazyProxy() :
      ProtocolImmutableProxy(PN_IMMUTABLE_LAZY) {}

    virtual OpRetVal operationRead(GlobalThread*, PstOutContainerInterface**&);
  };

} //End namespace
#endif
