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
    SimpleList<Pair<DSite*, bool> > a_readers;
    SimpleQueue<DSite*> a_requests;
    DSite* a_writer;

    // a_readers contain pairs (site, b), where b is false when the
    // proxy has invalidated its state.
    //
    // a_requests contains the proxies that will update the state,
    // while a_writer is the proxy that currently has the write token
    // (or NULL if none).

    // Invariant: a_readers is empty iff the entity is permfail

    ProtocolEagerInvalidManager(const ProtocolEagerInvalidManager&):
      a_readers(), a_requests(), a_writer(NULL) {}
    ProtocolEagerInvalidManager& operator=(const ProtocolEagerInvalidManager&){
      return *this; }
								    
  public:
    ProtocolEagerInvalidManager(DSite *mysite);
    ProtocolEagerInvalidManager(::MsgContainer*);
    ~ProtocolEagerInvalidManager() {}
    void makeGCpreps();
    void msgReceived(::MsgContainer*,DSite*);
    void sendMigrateInfo(::MsgContainer*); 

    void m_siteStateChange(DSite*, const DSiteState&);

  private: 
    bool m_isFailed() { return a_readers.isEmpty(); }
    void m_register(DSite* s);
    void m_deregister(DSite* s);
    void m_invalidateReaders();
    void m_invalidated(DSite* s);
    void m_sendWriteToken();
    void m_updateOneReader(DSite *s);
    void m_updateAllReaders();
    void m_failed();
    void printStatus();
  };


  class ProtocolEagerInvalidProxy:public ProtocolProxy{
  private:
    bool a_failed:1;     // true when the state is permfail
    bool a_valid:1;      // true iff the state is valid
    SimpleQueue<GlobalThread*> a_readers;
    SimpleQueue<GlobalThread*> a_writers;

    ProtocolEagerInvalidProxy(const ProtocolEagerInvalidProxy&):
      ProtocolProxy(PN_EAGER_INVALID), a_failed(false), a_valid(false),
      a_readers(), a_writers() {}
    ProtocolEagerInvalidProxy& operator=(const ProtocolEagerInvalidProxy&){
      return *this; }

  public:
    ProtocolEagerInvalidProxy();
    bool m_initRemoteProt(DssReadBuffer*);
    ~ProtocolEagerInvalidProxy();

    OpRetVal protocol_Read(GlobalThread* const th_id,
			   PstOutContainerInterface**& msg);
    OpRetVal protocol_Write(GlobalThread* const th_id,
			    PstOutContainerInterface**& msg);
    OpRetVal protocol_Kill();

    void makeGCpreps();
    bool isWeakRoot() { return !a_writers.isEmpty(); }

    void msgReceived(::MsgContainer*,DSite*);
    virtual void remoteInitatedOperationCompleted(DssOperationId* opId,
						  ::PstOutContainerInterface* pstOut){}
    void localInitatedOperationCompleted(){Assert(0);} 

    virtual FaultState siteStateChanged(DSite*, const DSiteState&);

  private: 
    void m_writeDone(); 
    void m_failed();
  };

} //End namespace
#endif 

