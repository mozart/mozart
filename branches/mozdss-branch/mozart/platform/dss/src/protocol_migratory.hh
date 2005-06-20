/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
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
#ifndef __PROTOCOL_MIGRATORY_HH
#define __PROTOCOL_MIGRATORY_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dssBase.hh"
#include "protocols.hh"
#include "dss_templates.hh"
namespace _dss_internal{ //Start namespace

// **************************  Migratory Token  ***************************
  enum Migratory_Token {
    MIGT_EMPTY,
    MIGT_HERE,
    MIGT_REQUESTED
  };

  enum Migratory_Operation {
    MIGO_ACCESS,
    MIGO_FORWARD
  };


  class ProtocolMigratoryManager:public ProtocolManager {
  public:
    DSite *current;
  private:
    ProtocolMigratoryManager(const ProtocolMigratoryManager&):current(NULL){}
    ProtocolMigratoryManager& operator=(const ProtocolMigratoryManager&){ return *this; }
  public:

    ProtocolMigratoryManager(DSite* mysite):current(mysite){};
    ProtocolMigratoryManager(::MsgContainer*);
    ~ProtocolMigratoryManager(){};
    void makeGCpreps() { current->m_makeGCpreps(); }
    void msgReceived(::MsgContainer*,DSite*);
    
    void sendMigrateInfo(::MsgContainer*); 
  };
  

  class ProtocolMigratoryProxy:public ProtocolProxy{
  private:
    DSite*    a_next;     // NULL
    Migratory_Token a_token;  //empty
    FifoQueue< TwoContainer<GlobalThread,Migratory_Operation> > a_Pqueue; // int queue

    void requestToken();
    void forwardToken();
    void resumeOperations();

    ProtocolMigratoryProxy(const ProtocolMigratoryProxy&):
      ProtocolProxy(PN_MIGRATORY_STATE),a_next(NULL), a_token(MIGT_EMPTY),
      a_Pqueue(){
      //a_Pqueue(FifoQueue< TwoContainer<GlobalThread,Migratory_Operation> >()){
    }

    ProtocolMigratoryProxy& operator=(const ProtocolMigratoryProxy&){ return *this; }

  public:
    ProtocolMigratoryProxy();
    bool m_initRemoteProt(DssReadBuffer*);
    ~ProtocolMigratoryProxy(){};

    //
    // WEAK protocol
    //
    // 1) I send a "need_no_more" to manager
    // 2) manager check if I am current, 
    //    if so it sends me a "forward" (to home proxy)
    //    else  it has already sent me a forward message and everething is ok
    //

    bool isWeakRoot(){
      return (a_token == MIGT_HERE || a_token == MIGT_REQUESTED);
    };
    
    bool clearWeakRoot();    
    
    OpRetVal protocol_Access(GlobalThread* const, ::PstOutContainerInterface**&);
    void makeGCpreps() { if (a_next != NULL) a_next->m_makeGCpreps(); }
    void msgReceived(::MsgContainer*,DSite*);
    
    virtual void remoteInitatedOperationCompleted(DssOperationId* opId,::PstOutContainerInterface* pstOut) {;} 
    void localInitatedOperationCompleted(){Assert(0);} 
  };

} //End namespace
#endif 
