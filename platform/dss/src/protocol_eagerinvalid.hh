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

  // generic classes for eager and lazy invalidation protocols

  class ProtocolInvalidManager : public ProtocolManager {
  private:
    // The status tells whether manager uses the lazy protocol.

    SimpleList<DSite*> a_readers;     // reader proxies
    unsigned int a_valid;             // how many proxies not invalid yet

    // a_readers contains all reader proxies (all proxies if eager,
    // strict readers if lazy).  Those proxies are asked to invalidate
    // their state once a write request must be served.  a_valid
    // proxies among a_readers have not notified invalidation.

    struct Request {   // buffered write requests
      GlobalThread* caller; PstInContainerInterface* arg;
      Request(GlobalThread* t, PstInContainerInterface* a): caller(t), arg(a) {}
      void makeGCpreps() { if (caller) caller->m_makeGCpreps(); }
      void dispose() { if (arg) arg->dispose(); }
    };

    SimpleQueue<Request> a_requests;

    // a_requests contains the pending write operations requested by
    // proxies.  Those operations are performed by the manager once
    // a_valid reaches zero.

    ProtocolInvalidManager(const ProtocolInvalidManager&) {}
    ProtocolInvalidManager& operator=(const ProtocolInvalidManager&){
      return *this; }

  public:
    ProtocolInvalidManager(DSite *mysite, bool isLazy);
    void sendMigrateInfo(::MsgContainer*); 
    ProtocolInvalidManager(::MsgContainer*);
    ~ProtocolInvalidManager();

    void msgReceived(::MsgContainer*, DSite*);
    void makeGCpreps();
    void m_siteStateChange(DSite*, const FaultState&);

  private: 
    bool m_isLazy() const { return getStatus(); }
    void m_register(DSite* s);
    void m_invalidate();
    void m_invalid(DSite* s, bool remove = false);
    static const bool REMOVE = true;
    void m_checkOperations();
    void m_commit(DSite*);
    void m_failed();
    void printStatus();
  };


  class ProtocolInvalidProxy : public ProtocolProxy{
  private:
    // The status of the proxy tells whether it uses the lazy
    // protocol, whether the proxy is a reader, and whether the state
    // is valid.  a_susps contains all pending operations; the first
    // a_numRead elements are read operations, the remaining ones are
    // write operations.  (The suspended writes operations are stored
    // in order to keep the suspended thread alive in memory.)
    unsigned int a_numRead;

    ProtocolInvalidProxy(const ProtocolInvalidProxy&):
      ProtocolProxy(PN_EAGER_INVALID) {}
    ProtocolInvalidProxy& operator=(const ProtocolInvalidProxy&){
      return *this; }

  public:
    ProtocolInvalidProxy(bool isLazy);
    bool m_initRemoteProt(DssReadBuffer*);
    ~ProtocolInvalidProxy();

    virtual OpRetVal operationRead(GlobalThread*, PstOutContainerInterface**&);
    virtual OpRetVal operationWrite(GlobalThread*, PstOutContainerInterface**&);

    void msgReceived(::MsgContainer*,DSite*);

    virtual void remoteInitatedOperationCompleted(DssOperationId*,
						  ::PstOutContainerInterface*)
    { Assert(0); }
    void localInitatedOperationCompleted(){Assert(0);} 

    virtual FaultState siteStateChanged(DSite*, const FaultState&);

  private:
    bool m_isLazy() const { return getStatus() & 1; }
    void m_setLazy() { setStatus(getStatus() | 1); }
    bool m_isReader() const { return getStatus() & 2; }
    void m_setReader(bool b) { setStatus((getStatus() & ~2) | (b ? 2 : 0)); }
    bool m_isValid() const { return getStatus() & 4; }
    void m_setValid(bool b) { setStatus((getStatus() & ~4) | (b ? 4 : 0)); }
    void m_subscribe();
  };



  // manager and proxy for the eager protocol

  class ProtocolEagerInvalidManager : public ProtocolInvalidManager {
  public:
    ProtocolEagerInvalidManager(DSite* s) :
      ProtocolInvalidManager(s, false) {}
    ProtocolEagerInvalidManager(MsgContainer* msg) :
      ProtocolInvalidManager(msg) {}
  };

  class ProtocolEagerInvalidProxy : public ProtocolInvalidProxy {
  public:
    ProtocolEagerInvalidProxy() : ProtocolInvalidProxy(false) {}
  };

} //End namespace
#endif 

