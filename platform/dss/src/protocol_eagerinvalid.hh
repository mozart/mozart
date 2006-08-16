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

#ifndef __PROTOCOL_EAGERINVALID_HH
#define __PROTOCOL_EAGERINVALID_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"
#include "dss_templates.hh"

namespace _dss_internal{ //Start namespace

  class ProtocolEagerInvalidManager:public ProtocolManager {
  private:
    SimpleList<DSite*> a_readers;
    SimpleQueue<DSite*> a_writers;

    // All proxies are registered in a_proxies.  a_readers contains
    // all reader proxies that have not invalidated their state yet.
    // They are asked to invalidate once a write request must be
    // served; the write token is given once a_readers becomes empty.
    //
    // a_writers contains the proxies that made a write request.  The
    // first element of a_writers has the write token if a_readers is
    // empty.

    ProtocolEagerInvalidManager(const ProtocolEagerInvalidManager&) {}
    ProtocolEagerInvalidManager& operator=(const ProtocolEagerInvalidManager&){
      return *this; }
								    
  public:
    ProtocolEagerInvalidManager(DSite *mysite);
    void sendMigrateInfo(::MsgContainer*); 
    ProtocolEagerInvalidManager(::MsgContainer*);
    ~ProtocolEagerInvalidManager() {}

    void msgReceived(::MsgContainer*,DSite*);
    void m_siteStateChange(DSite*, const DSiteState&);

  private: 
    void m_register(DSite* s);
    void m_deregister(DSite* s);
    void m_invalidateReaders();
    void m_invalidated(DSite* s);
    void m_sendWriteToken();
    void m_updateOneReader(DSite*);
    void m_updateAllReaders(DSite*);
    void m_failed();
    void printStatus();
  };


  class ProtocolEagerInvalidProxy:public ProtocolProxy{
  private:
    // The status of the proxy tells whether the state is valid.  The
    // first a_reads elements of a_susps are read operations, and the
    // remaining ones are write operations
    int a_reads;

    ProtocolEagerInvalidProxy(const ProtocolEagerInvalidProxy&):
      ProtocolProxy(PN_EAGER_INVALID), a_reads(0) {}
    ProtocolEagerInvalidProxy& operator=(const ProtocolEagerInvalidProxy&){
      return *this; }

  public:
    ProtocolEagerInvalidProxy();
    bool m_initRemoteProt(DssReadBuffer*);
    ~ProtocolEagerInvalidProxy();

    bool isWeakRoot() { return !isPermFail() && a_reads < a_susps.size(); }

    OpRetVal protocol_Read(GlobalThread* const th_id,
			   PstOutContainerInterface**& msg);
    OpRetVal protocol_Write(GlobalThread* const th_id,
			    PstOutContainerInterface**& msg);

    void msgReceived(::MsgContainer*,DSite*);

    virtual void remoteInitatedOperationCompleted(DssOperationId*,
						  ::PstOutContainerInterface*)
    { Assert(0); }
    void localInitatedOperationCompleted(){Assert(0);} 

    virtual FaultState siteStateChanged(DSite*, const DSiteState&);
  };

} //End namespace
#endif 

