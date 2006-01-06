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

#ifndef __PROTOCOL_HH
#define __PROTOCOL_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "coordinator.hh"
#include "dss_threads.hh"
namespace _dss_internal{ //Start namespace

  class ProtocolManager{
  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif
    Coordinator *manager;

  public:


    ProtocolManager():manager(NULL){ DebugCode(a_allocated++); };
    virtual ~ProtocolManager(){ DebugCode(a_allocated--); }

    virtual void msgReceived(MsgContainer*,DSite*)=0;
    virtual void makeGCpreps(){}

    inline AbstractEntityName getAEname(){ Assert(0); return AEN_NOT_DEFINED;}
  

    virtual void sendMigrateInfo(MsgContainer*){ 
      manager->m_getEnvironment()->a_map->GL_warning("Migrating non Migratable Manager");
    }

    MACRO_NO_DEFAULT_CONSTRUCTORS(ProtocolManager);
  };

  class ProtocolProxy{
  private:
    ProtocolName p_name;
  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif

    Proxy *a_proxy;

  public:

    ProtocolProxy(const ProtocolName& name):p_name(name),a_proxy(NULL){ DebugCode(a_allocated++); };
    virtual ~ProtocolProxy(){ DebugCode(a_allocated--); };
    
    // ** PROTOCOL OPERATIONS **
    virtual void msgReceived(MsgContainer*,DSite*)=0;

    // ** SERVICES **
    inline Proxy       *getProxy(){ return a_proxy; };
    inline ProtocolName getProtocolName(){ return p_name; }

    virtual bool isWeakRoot(){ return false; };

    // Usually if we were a weak root we cannot change the state
    virtual bool clearWeakRoot(){ return false; };
    virtual void makeGCpreps(){};
  
    // called when the proxy is fully installed in the system. 
    // Typical usage case: registration of variable proxies. 
    //
    // ZACHARIAS: explain what bool return means, thanks ;)
    virtual bool m_initRemoteProt(DssReadBuffer*){ return true; };

    // Called when the state of a site changes. It is up to the
    // protocol to deduce if it is affected by the site. 
    virtual int siteStateChanged(DSite*, const DSiteState&){return 0;}

    //located in dss_access.cc, the only placed accessed from
    virtual char *m_stringrep();

    // Marshaling and unmarshaling proxy information
    //+++++++++++++
    virtual bool marshal_protocol_info(DssWriteBuffer *buf, DSite *){ return true; };
    virtual bool dispose_protocol_info(DssReadBuffer *buf){ return true; };
    
    virtual void remoteInitatedOperationCompleted(DssOperationId* opId,
						  ::PstOutContainerInterface* pstOut) = 0; 
    virtual void localInitatedOperationCompleted() = 0; 

    MACRO_NO_DEFAULT_CONSTRUCTORS(ProtocolProxy);
   
  };
  

  //// Proxy to Manager /////
  void gf_sendProxyToManager(Proxy* p, int i1);
  
  ///// Proxy to Proxy
  void gf_sendProxyToProxy(Proxy* p, DSite* s, int i1);
  void gf_sendProxyToProxy(Proxy* p, DSite* s, int i1, DSite* s1);
  void gf_sendProxyToProxy(Proxy* p, DSite* s, int i1, PstOutContainerInterface* out );

  //// Manager to Proxy 
  
  void gf_sendManagerToProxy(Coordinator* m, DSite* s, int i1);
  void gf_sendManagerToProxy(Coordinator* m, DSite* s, int i1, DSite *s1);
  
  // Creating protocl proxies and managers
  ProtocolManager *gf_createProtManager(MsgContainer* msgC, ProtocolName pn);
  void gf_createProtocolProxyManager(ProtocolName prot, DSS_Environment* env, ProtocolManager *&pman, ProtocolProxy *&pprox);
  ProtocolProxy* gf_createRemoteProxy(ProtocolName prot, DSite* myDSite);
  
} //End namespace
#endif 



