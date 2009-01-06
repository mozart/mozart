/*
 *  Authors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Raphael Collet, 2008
 * 
 *  Last change:
 *    $Date: $ by $Author: $
 *    $Revision: $
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
#ifndef __PROTOCOL_SITED_HH
#define __PROTOCOL_SITED_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"

namespace _dss_internal{ //Start namespace

  // This protocol provides a way to restrict all language operations
  // of an entity on its home site.  The home site performs operations
  // locally, while remote proxies don't provide any operation.

  class ProtocolSitedManager : public ProtocolManager {
  private:
    ProtocolSitedManager(const ProtocolSitedManager&) {}
    ProtocolSitedManager& operator=(const ProtocolSitedManager&){ return *this; }
  public:
    ProtocolSitedManager() {}
    ProtocolSitedManager(::MsgContainer *) { Assert(0); }
    ~ProtocolSitedManager(){}

    void msgReceived(::MsgContainer*,DSite*) {}
    void sendMigrateInfo(::MsgContainer*) { Assert(0); }
  };


  class ProtocolSitedProxy : public ProtocolProxy {
  public: 
    ProtocolSitedProxy() : ProtocolProxy(PN_SITED) {}
    ~ProtocolSitedProxy() {}

    bool isHomeProxy() { return getProxy()->m_isHomeProxy(); }
    
    virtual OpRetVal operationKill();
    virtual OpRetVal operationRead(GlobalThread*, PstOutContainerInterface**&);
    virtual OpRetVal operationWrite(GlobalThread*, PstOutContainerInterface**&);
    virtual OpRetVal operationWrite(PstOutContainerInterface**&);
    
    void msgReceived(::MsgContainer*,DSite*) {}

    // check fault state
    virtual FaultState siteStateChanged(DSite*, const FaultState&);
  };

} //End namespace
#endif
