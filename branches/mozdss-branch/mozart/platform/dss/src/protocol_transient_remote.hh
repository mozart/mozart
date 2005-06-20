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

#ifndef __ONCE_TRANSIENT_REMOTE_PROTOCOL_HH
#define __ONCE_TRANSIENT_REMOTE_PROTOCOL_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"
#include "dss_templates.hh"
namespace _dss_internal{ //Start namespace

  class ProtocolTransientRemoteManager:public ProtocolManager{
  private:
    OneContainer<DSite> *a_proxies;
    int a_aop;
    bool a_bound;
    DSite *a_current; 

    
    ProtocolTransientRemoteManager(const ProtocolTransientRemoteManager&):
      a_proxies(NULL), a_aop(0), a_bound(false), a_current(NULL){}

    ProtocolTransientRemoteManager& operator=(const ProtocolTransientRemoteManager&){ return *this; }
  public:

    ProtocolTransientRemoteManager(DSite* const site);
    ProtocolTransientRemoteManager(::MsgContainer * const);
    ~ProtocolTransientRemoteManager();
    void makeGCpreps();
    void msgReceived(::MsgContainer*,DSite*);

    void sendRedirect(DSite*);
    void sendMigrateInfo(MsgContainer*); 
    bool register_remote(DSite*);
  };


  class ProtocolTransientRemoteProxy:public ProtocolProxy{
  private:
    OneContainer<GlobalThread*> *a_susps; 
    bool a_bound;
    bool a_writeToken; 

    ProtocolTransientRemoteProxy(const ProtocolTransientRemoteProxy&):
      ProtocolProxy(PN_NO_PROTOCOL), a_susps(NULL),a_bound(false),a_writeToken(false){}

    ProtocolTransientRemoteProxy& operator=(const ProtocolTransientRemoteProxy&){ return *this; }

    void wkSuspThrs(); 
  public:
    ProtocolTransientRemoteProxy();
    OpRetVal protocol_Terminate(GlobalThread* const th_id, ::PstOutContainerInterface**& msg,const AbsOp& aop);
    OpRetVal protocol_Update(GlobalThread* const th_id, ::PstOutContainerInterface**& msg,const AbsOp& aop);

    void makeGCpreps(); //threads should be guarded from the glue as well as....
    bool isWeakRoot(){ return (a_susps != NULL); };


    void msgReceived(::MsgContainer*,DSite*);
    ~ProtocolTransientRemoteProxy();
  
    bool m_initRemoteProt(DssReadBuffer*);
  
    void sendMigrateInfo(MsgContainer*); 
    void instantiateMigrateInfo(::MsgContainer*);

    // Marshaling and unmarshaling proxy information
    virtual bool marshal_protocol_info(DssWriteBuffer *buf, DSite*);
    virtual bool dispose_protocol_info(DssReadBuffer *buf );
    
    virtual void remoteInitatedOperationCompleted(DssOperationId* opId,PstOutContainerInterface* pstOut) {;} 
    virtual void localInitatedOperationCompleted() {Assert(0);} 
  };


} //End namespace
#endif 



