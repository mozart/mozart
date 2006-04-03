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
#ifndef __PROTOCOL_IMMUTABLE_EAGER_HH
#define __PROTOCOL_IMMUTABLE_EAGER_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "dss_comService.hh"
#include "protocols.hh"
#include "dss_templates.hh"

namespace _dss_internal{ //Start namespace

  class ProtocolImmutableEagerManager:public ProtocolManager{
  public:
    ProtocolImmutableEagerManager();
    ~ProtocolImmutableEagerManager(){};
    virtual void msgReceived(MsgContainer*,DSite*);    
  };
  
  class ProtocolImmutableEagerProxy:public ProtocolProxy{
    bool stateHolder: true; 
    SimpleList<GlobalThread*> a_readers;
  public: 
    ProtocolImmutableEagerProxy();
    ~ProtocolImmutableEagerProxy(){};
    
    OpRetVal protocol_send(GlobalThread* const th_id);
 
    virtual void msgReceived(MsgContainer*,DSite*);   
    virtual void remoteInitatedOperationCompleted(DssOperationId*, PstOutContainerInterface*); 
    virtual void localInitatedOperationCompleted(); 
        
    virtual bool isWeakRoot(){ return stateHolder; }; // The glue should know if a thread is relying on proxy
    virtual void makeGCpreps();
    virtual bool m_initRemoteProt(DssReadBuffer*); 
  };

} //End namespace
#endif
