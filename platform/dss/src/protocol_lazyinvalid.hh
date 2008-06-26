/*
 *  Authors:
 *    Erik Klintskog
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Zacharias El Banna, 2002
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
#ifndef __PROTOCOL_LAZYINVALID_HH
#define __PROTOCOL_LAZYINVALID_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"
#include "protocol_eagerinvalid.hh"
#include "dss_templates.hh"

namespace _dss_internal{ //Start namespace

  // manager and proxy for the eager protocol.  ProtocolInvalidManager
  // and ProtocolInvalidProxy are defined in protocol_eagerinvalid.hh.

  class ProtocolLazyInvalidManager : public ProtocolInvalidManager {
  public:
    ProtocolLazyInvalidManager(DSite* s) :
      ProtocolInvalidManager(s, true) {}
    ProtocolLazyInvalidManager(MsgContainer* msg) :
      ProtocolInvalidManager(msg) {}
  };

  class ProtocolLazyInvalidProxy : public ProtocolInvalidProxy {
  public:
    ProtocolLazyInvalidProxy() : ProtocolInvalidProxy(true) {}
  };

} //End namespace
#endif 

