/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

  class ProtocolOnceOnlyManager:public ProtocolManager{
  private:
    OneContainer<DSite> *a_proxies;
    bool a_bound;

    ProtocolOnceOnlyManager(const ProtocolOnceOnlyManager&):a_proxies(NULL),a_bound(false){};
    ProtocolOnceOnlyManager& operator=(const ProtocolOnceOnlyManager&){ return *this; }
  public:

    ProtocolOnceOnlyManager(DSite* const site);
    ProtocolOnceOnlyManager(MsgContainer * const);
    ~ProtocolOnceOnlyManager();
    void makeGCpreps();
    void msgReceived(MsgContainer*,DSite*);

    void sendRedirect(DSite*);
    void sendMigrateInfo(MsgContainer*); 
  };

  enum ProtOOop{
    ProtOO_wait, 
    ProtOO_write, 
    ProtOO_terminate 
  };

  class ProtocolOnceOnlyProxy:public ProtocolProxy{
  private:
    TwoContainer<GlobalThread, ProtOOop> *a_susps; 
    bool a_bound;

    ProtocolOnceOnlyProxy(const ProtocolOnceOnlyProxy&):ProtocolProxy(PN_TRANSIENT),a_susps(NULL),a_bound(false){};
    ProtocolOnceOnlyProxy& operator=(const ProtocolOnceOnlyProxy&){ return *this; }
  public:
    ProtocolOnceOnlyProxy();
    
    OpRetVal protocol_Terminate(GlobalThread* const th_id, ::PstOutContainerInterface**& msg,const AbsOp& aop);
    OpRetVal protocol_Update(GlobalThread* const th_id, ::PstOutContainerInterface**& msg,const AbsOp& aop);
    
    virtual bool isWeakRoot(){ return (a_susps != NULL); };

    void makeGCpreps(); //threads should be guarded from the glue as well as....
    void msgReceived(MsgContainer*,DSite*);
    ~ProtocolOnceOnlyProxy();
  
    bool m_initRemoteProt(DssReadBuffer*);
  
    virtual void remoteInitatedOperationCompleted(DssOperationId* opId,::PstOutContainerInterface* pstOut) {;} 
    void localInitatedOperationCompleted(){Assert(0);} 

    // check fault state
    virtual FaultState siteStateChanged(DSite*, const DSiteState&);
  };

} //End namespace
#endif 
