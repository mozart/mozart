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
#ifndef __PROTOCOL_EAGERINVALID_HH
#define __PROTOCOL_EAGERINVALID_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"
#include "dss_templates.hh"

namespace _dss_internal{ //Start namespace

// **************************  Migratory Token  ***************************
  enum ECItokenStatus {
    ECITS_VALID,
    ECITS_INVALID
  };

  class ProtocolEagerInvalidManager:public ProtocolManager {
  private:
    TwoContainer<DSite, bool>* a_readers;
    FifoQueue<OneContainer<DSite> > a_writers; 

    ProtocolEagerInvalidManager(const ProtocolEagerInvalidManager&):
      a_readers(NULL), a_writers(){}
    ProtocolEagerInvalidManager& operator=(const ProtocolEagerInvalidManager&){ return *this; }
								    
  public:
    ProtocolEagerInvalidManager(DSite *mysite):
      a_readers(NULL), a_writers(){ 
      a_readers = new  TwoContainer<DSite, bool>(mysite, true,NULL); 
    }
    ProtocolEagerInvalidManager(::MsgContainer*);
    ~ProtocolEagerInvalidManager(){ t_deleteList(a_readers); }
    void makeGCpreps();
    void msgReceived(::MsgContainer*,DSite*);
    void sendMigrateInfo(::MsgContainer*); 
  private: 
    void m_invalidateReaders();
    void m_sendWriteRight();
    void m_updateOneReader(DSite *target);
    void m_updateAllReaders(DSite *exclude);
    void printStatus();
  };


  class ProtocolEagerInvalidProxy:public ProtocolProxy{
  private:
    OneContainer<GlobalThread> *a_readers; 
    OneContainer<GlobalThread> *a_writers; 
    ECItokenStatus  a_token;

    ProtocolEagerInvalidProxy(const ProtocolEagerInvalidProxy&):
      ProtocolProxy(PN_EAGER_INVALID), a_readers(NULL), a_writers(NULL), a_token(ECITS_INVALID){}
    ProtocolEagerInvalidProxy& operator=(const ProtocolEagerInvalidProxy&){ return *this; }
  public:
    ProtocolEagerInvalidProxy();
    ProtocolEagerInvalidProxy(DssReadBuffer*);

    OpRetVal protocol_Read( GlobalThread* const th_id, PstOutContainerInterface**& msg);
    OpRetVal protocol_Write( GlobalThread* const th_id, PstOutContainerInterface**& msg);

    void makeGCpreps(){;}
    bool isWeakRoot(){ return (a_readers != NULL || a_writers != NULL); };

    void msgReceived(::MsgContainer*,DSite*);
    ~ProtocolEagerInvalidProxy();

    bool m_initRemoteProt(DssReadBuffer*);

    virtual void remoteInitatedOperationCompleted(DssOperationId* opId,
						  ::PstOutContainerInterface* pstOut){;}  
    void localInitatedOperationCompleted(){Assert(0);} 
  private: 
    void m_writeDone(); 
    
  };

} //End namespace
#endif 

