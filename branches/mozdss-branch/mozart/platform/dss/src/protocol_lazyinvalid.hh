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
#include "dss_templates.hh"

namespace _dss_internal{ //Start namespace

  class ProtocolLazyInvalidManager:public ProtocolManager {
  private:
    bool a_failed;
    SimpleQueue<Pair<DSite*, bool> > a_requests;
    SimpleList<DSite*> a_readers;
    DSite* a_writer;

    // a_requests contains pairs (site, b), where b is true for read
    // requests, and false for write requests.
    //
    // a_readers contains all reader proxies that have not invalidated
    // their state yet.  They are asked to invalidate once a write
    // request must be served; the write token is given once a_readers
    // becomes empty.
    //
    // a_writer is the proxy that currently has the write token.

    ProtocolLazyInvalidManager(const ProtocolLazyInvalidManager&):
      a_failed(false), a_requests(), a_readers(), a_writer(NULL) {}
    ProtocolLazyInvalidManager operator=(const ProtocolLazyInvalidManager&){
      return *this; }

  public:
    ProtocolLazyInvalidManager(DSite *mysite);
    ProtocolLazyInvalidManager(MsgContainer*);
    ~ProtocolLazyInvalidManager() {}
    void makeGCpreps();
    void msgReceived(MsgContainer*,DSite*);
    void sendMigrateInfo(MsgContainer*); 

    void m_siteStateChange(DSite*, const DSiteState&);

  private: 
    void m_sendWriteToken();
    void m_updateOneReader(DSite *);
    void m_handleNextRequest();
    void m_failed();
  };


  enum LazyInvalidToken {
    LIT_INVALID,     // proxy has no valid state
    LIT_READER,      // proxy has read token
    LIT_WRITER,      // proxy has write token
    LIT_FAILED       // state is lost
  };

  class ProtocolLazyInvalidProxy:public ProtocolProxy{
  private:
    LazyInvalidToken a_token;     // status of this proxy
    SimpleQueue<GlobalThread*> a_readers;
    SimpleQueue<GlobalThread*> a_writers;

    ProtocolLazyInvalidProxy(const ProtocolLazyInvalidProxy&):
      ProtocolProxy(PN_EAGER_INVALID), a_token(LIT_INVALID),
      a_readers(), a_writers() {}
    ProtocolLazyInvalidProxy operator=(const ProtocolLazyInvalidProxy&){
      return *this; }

  public:
    ProtocolLazyInvalidProxy();
    bool m_initRemoteProt(DssReadBuffer*);
    ~ProtocolLazyInvalidProxy();

    OpRetVal protocol_Read(GlobalThread* const th_id,
			   PstOutContainerInterface**& msg);
    OpRetVal protocol_Write(GlobalThread* const th_id,
			    PstOutContainerInterface**& msg);
    OpRetVal protocol_Kill();

    void makeGCpreps();
    bool isWeakRoot() { return a_token == LIT_WRITER || !a_writers.isEmpty(); }

    void msgReceived(MsgContainer*,DSite*);

    virtual void
    remoteInitatedOperationCompleted(DssOperationId* opId,
				     PstOutContainerInterface* pstOut) {}
    void localInitatedOperationCompleted() { Assert(0); }

    virtual FaultState siteStateChanged(DSite*, const DSiteState&);

  private: 
    void m_requestReadToken();
    void m_requestWriteToken();
  };

} //End namespace
#endif 

