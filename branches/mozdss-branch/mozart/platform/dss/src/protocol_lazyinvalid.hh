/*
 *  Authors:
 *    Erik Klintskog
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

  enum LCItokenStatus {
    LCITS_READ_TOKEN,
    LCITS_WRITE_TOKEN,
    LCITS_INVALID
  };

  class ProtocolLazyInvalidManager:public ProtocolManager {
  private:
    SimpleList<Pair<DSite*, bool> > a_readers;
    SimpleQueue<Pair<DSite*, bool> > a_requests;
    DSite* a_writer;

    // a_requests contains pairs (site, b), where b is true for read
    // requests, and false for write requests.

    ProtocolLazyInvalidManager(const ProtocolLazyInvalidManager&):
      a_readers(), a_requests(), a_writer(NULL){}
    ProtocolLazyInvalidManager operator=(const ProtocolLazyInvalidManager&){ return *this; }

  public:
    ProtocolLazyInvalidManager(DSite *mysite);
    ProtocolLazyInvalidManager(MsgContainer*);
    ~ProtocolLazyInvalidManager() {}
    void makeGCpreps();
    void msgReceived(MsgContainer*,DSite*);
    void sendMigrateInfo(MsgContainer*); 

  private: 
    void m_handleNextRequest();
    void m_sendWriteRight();
    void m_updateOneReader(DSite *target);
    void m_updateAllReaders(DSite *exclude);
  };


  class ProtocolLazyInvalidProxy:public ProtocolProxy{
  private:
    SimpleQueue<GlobalThread*> a_readers;
    SimpleQueue<GlobalThread*> a_writers;
    LCItokenStatus  a_token;

    ProtocolLazyInvalidProxy(const ProtocolLazyInvalidProxy&):
      ProtocolProxy(PN_EAGER_INVALID), a_readers(), a_writers(), a_token(LCITS_INVALID){}
    ProtocolLazyInvalidProxy operator=(const ProtocolLazyInvalidProxy&){ return *this; }

  public:
    ProtocolLazyInvalidProxy();
    ProtocolLazyInvalidProxy(DssReadBuffer*);

    OpRetVal protocol_Read( GlobalThread* const th_id, PstOutContainerInterface**& msg);
    OpRetVal protocol_Write(GlobalThread* const th_id, PstOutContainerInterface**& msg);

    void makeGCpreps();
    bool isWeakRoot(){
      return !(a_readers.isEmpty() && a_writers.isEmpty() &&
	       a_token == LCITS_INVALID);
    }

    void msgReceived(MsgContainer*,DSite*);
    ~ProtocolLazyInvalidProxy();

    bool m_initRemoteProt(DssReadBuffer*);

    virtual void remoteInitatedOperationCompleted(DssOperationId* opId,
						  PstOutContainerInterface* pstOut){;}  
    void localInitatedOperationCompleted(){Assert(0);} 

  private: 
    void m_requestWriteToken();
    void m_requestReadToken();
  };

} //End namespace
#endif 

