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

#if defined(INTERFACE)
#pragma implementation "protocol_lazyinvalid.hh"
#endif

#include "protocol_lazyinvalid.hh"
namespace _dss_internal{ //Start namespace

// **************************  Migratory Token  ***************************

  namespace {
    enum CacheInvalid_Message {
      LCI_INVALID_READ,     // m2p
      LCI_READ_TOKEN,       // m2p
      LCI_READ_INVALIDATED, // p2m
      LCI_READ_REQUEST,     // p2m

      LCI_WRITE_REQUEST,    // p2m
      LCI_WRITE_TOKEN,      // m2p
      LCI_WRITE_INVALIDATED,// p2m
      LCI_INVALID_WRITE     // m2p
    };

    /*
    static char *msg_strings[] = { "LCI_INVALID_READ",
				   "LCI_READ_TOKEN",      
				   "LCI_READ_INVALIDATED",
				   "LCI_READ_REQUEST",   
				   "LCI_WRITE_REQUEST", 
				   "LCI_WRITE_TOKEN",   
				   "LCI_WRITE_INVALIDATED",
				   "LCI_INVALID_WRITE"     
				   
				   };
    */
  
    
  
  void 
  ProtocolLazyInvalidManager::m_updateOneReader(DSite *target){
    MsgContainer *msgC = manager->m_createProxyProtMsg();
    gf_createSndMsg(msgC,  LCI_READ_TOKEN, manager->retrieveEntityState()); 
    target->m_sendMsg(msgC); 
  }
    
    
    
  
  
    void 
  ProtocolLazyInvalidManager::m_handleNextRequest()
  {
    if(a_writer!=NULL)
      {
	MsgContainer *msgC = manager->m_createProxyProtMsg();
	gf_createSndMsg(msgC,  LCI_INVALID_WRITE); 
	a_writer->m_sendMsg(msgC); 
      }
    else
      {
	while(!a_requests.isEmpty() && a_requests.peek()->a_contain2) // Get all queued read requests
	  {
	    TwoContainer<DSite, bool> *cnt= a_requests.drop(); 
	    cnt->a_next = a_readers; 
	    a_readers = cnt; 
	    m_updateOneReader(cnt->a_contain1); 
	  }
	if (!a_requests.isEmpty())
	  {
	    // Invalidate all readers, dont invalidate the next writer, AND if 
	    // the readers contained no other process that teh writer, send imediatly
	    DSite *WriterSite = a_requests.peek()->a_contain1;
	    TwoContainer<DSite, bool>** ptr = &a_readers;
	    while((*ptr)!=NULL){
	      DSite *rdr = (*ptr)->a_contain1; 
	      if ( rdr == WriterSite){
		TwoContainer<DSite, bool>* tmp = *ptr; 
		(*ptr) = (*ptr)->a_next; 
		delete tmp; 
	      }
	      else{
		MsgContainer *msgC = manager->m_createProxyProtMsg();
		gf_createSndMsg(msgC,  LCI_INVALID_READ); 
		rdr->m_sendMsg(msgC);
		ptr = &(*ptr)->a_next;
	      }
	    }
	    if (a_readers == NULL) 
	      {
		m_sendWriteRight();
		a_writer = WriterSite; 
		
		// So, we have now passed the state to a writer. However, if there 
		// are more requests in the Pipe, they must be handled.
		if (!a_requests.isEmpty())
		  m_handleNextRequest();
	      }
	  }
      }
  }
  
  
  void 
  ProtocolLazyInvalidManager::m_sendWriteRight(){
    TwoContainer<DSite,bool>* ptr = a_requests.drop();
    MsgContainer *msgC = manager->m_createProxyProtMsg();
    gf_createSndMsg(msgC, LCI_WRITE_TOKEN, manager->retrieveEntityState()); 
    (ptr->a_contain1)->m_sendMsg(msgC);
    
    delete ptr; 
  }
  
  void
  ProtocolLazyInvalidManager::msgReceived(::MsgContainer* msg, DSite* sender){
    int message = msg->popIntVal();
    switch(message) {
    case LCI_WRITE_REQUEST:
      {
	if (a_requests.isEmpty()){
	  a_requests.append(new TwoContainer<DSite,bool>(sender, false, NULL)); 
	  m_handleNextRequest();
	}
	else{
	  a_requests.append(new TwoContainer<DSite,bool>(sender, false, NULL));
	}
	break; 
      }
    case LCI_WRITE_INVALIDATED:
      {
	PstInContainerInterface* buildcont =  gf_popPstIn(msg); 
	manager->installEntityState(buildcont); 
	Assert(sender == a_writer);
	a_writer = NULL;
	Assert(a_readers == NULL); 
	a_readers = new TwoContainer<DSite, bool>(sender, true, NULL); 
	m_handleNextRequest();
	break; 
      }
    case LCI_READ_INVALIDATED: 
      {
	
	DebugCode(bool found =) t_deleteCompare(&a_readers, sender); 
	Assert(found); 
	if(a_readers == NULL)
	  m_handleNextRequest();
	break; 
      }
    case LCI_READ_REQUEST:
      {
	if (a_requests.isEmpty()){
	  a_requests.append(new TwoContainer<DSite,bool>(sender, true, NULL));
	  m_handleNextRequest();
	}
	else {
	  a_requests.append(new TwoContainer<DSite,bool>(sender, true, NULL));
	}
	break;
      }
    default:
      Assert(0);
    }
  }


    void ProtocolLazyInvalidManager::sendMigrateInfo(::MsgContainer* msg){
    int len = 0; 
    TwoContainer<DSite, bool> *ptr = a_readers;
    for(; ptr != NULL ; ptr = ptr->a_next)
      {
	len++; 
      }
    msg->pushIntVal(len);
    for(ptr = a_readers; ptr != NULL ; ptr = ptr->a_next){
      msg->pushDSiteVal(ptr->a_contain1);
      msg->pushIntVal(static_cast<bool>(ptr->a_contain2));
      
    }
    Assert(0) // We need to move the writers as well!
      }
    
    ProtocolLazyInvalidManager::ProtocolLazyInvalidManager(::MsgContainer* msg):
      a_readers(NULL), a_requests(), a_writer(NULL){
      int len = msg->popIntVal();
      for(;len>0; len --) 
	a_readers = new TwoContainer<DSite,bool>(msg->popDSiteVal(),static_cast<bool>(msg->popIntVal()), a_readers);
    }
    
    ProtocolLazyInvalidManager::ProtocolLazyInvalidManager(DSite *mysite):
      a_readers(NULL),a_requests(), a_writer(mysite)
    {} 
    


  ProtocolLazyInvalidProxy::ProtocolLazyInvalidProxy():
    ProtocolProxy(PN_LAZY_INVALID),a_readers(NULL), a_writers(NULL), a_token(LCITS_WRITE_TOKEN){ }


  ProtocolLazyInvalidProxy::ProtocolLazyInvalidProxy(DssReadBuffer*):
    ProtocolProxy(PN_LAZY_INVALID),a_readers(NULL), a_writers(NULL), a_token(LCITS_INVALID){}
  

  bool
  ProtocolLazyInvalidProxy::m_initRemoteProt(DssReadBuffer*){
    a_token = LCITS_INVALID;
    return true;
  }

  ProtocolLazyInvalidProxy::~ProtocolLazyInvalidProxy(){
    Assert(a_writers == NULL); 
    Assert(a_readers == NULL); 
  }

  void
  ProtocolLazyInvalidProxy::m_requestWriteToken(){
    MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
    gf_createSndMsg(msgC,LCI_WRITE_REQUEST);
    a_proxy->m_sendToCoordinator(msgC);
  }

  void
  ProtocolLazyInvalidProxy::m_requestReadToken(){
    MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
    gf_createSndMsg(msgC,LCI_READ_REQUEST);
    a_proxy->m_sendToCoordinator(msgC);
  }
  
  
  OpRetVal
  ProtocolLazyInvalidProxy::protocol_Read(GlobalThread* const th_id, PstOutContainerInterface**& msg){
    dssLog(DLL_BEHAVIOR,"LazyInvalidProxy::Read");
    msg = NULL;
    switch(a_token){
    case LCITS_INVALID:
      if(a_readers == NULL)
	m_requestReadToken();
      a_readers = new OneContainer<GlobalThread>(th_id,a_readers);
      return DSS_SUSPEND;
    case  LCITS_READ_TOKEN:
    case  LCITS_WRITE_TOKEN:
      return DSS_PROCEED;
    default:
      Assert(0);
      return DSS_INTERNAL_ERROR_SEVERE; // Whatever....
    }
  }

  OpRetVal
  ProtocolLazyInvalidProxy::protocol_Write(GlobalThread* const th_id, PstOutContainerInterface**& msg){
    dssLog(DLL_BEHAVIOR,"LazyInvalidProxy::Write");
    msg = NULL;
    if (a_token == LCITS_WRITE_TOKEN){
      return DSS_PROCEED;
    }
    
    if (a_writers == NULL)
      m_requestWriteToken();
    
    a_writers = new OneContainer<GlobalThread>(th_id, a_writers); 
    return DSS_SUSPEND;
  }

	  
  void
  ProtocolLazyInvalidProxy::msgReceived(::MsgContainer* msg, DSite*){
    int message = msg->popIntVal();
    switch(message) {
    case LCI_READ_TOKEN:{
      ::PstInContainerInterface* buildcont =  gf_popPstIn(msg);
      a_token = LCITS_READ_TOKEN;
      a_proxy->installEntityState(buildcont);
      while(a_readers!=NULL){
	OneContainer<GlobalThread>* ptr = a_readers->a_next; 
	(a_readers->a_contain1)->resumeDoLocal(NULL);
	delete a_readers;
	a_readers  = ptr;
      }
      break; 
    }
    case LCI_WRITE_TOKEN:{
      a_token = LCITS_WRITE_TOKEN;
      PstInContainerInterface* buildcont =  gf_popPstIn(msg);
      a_proxy->installEntityState(buildcont);
      while(a_writers != NULL){
	OneContainer<GlobalThread> *tmp = a_writers; 
	(a_writers->a_contain1)->resumeDoLocal(NULL);
	a_writers = a_writers->a_next;
	delete tmp; 
      }
      break; 
    }
    case LCI_INVALID_READ:
      {
	a_token = LCITS_INVALID;
	MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
	gf_createSndMsg(msgC, LCI_READ_INVALIDATED);
	a_proxy->m_sendToCoordinator(msgC); 
	break; 
      }
    case LCI_INVALID_WRITE:
      {	
	a_token = LCITS_READ_TOKEN;
	MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
	gf_createSndMsg(msgC, LCI_WRITE_INVALIDATED, a_proxy->retrieveEntityState());
	a_proxy->m_sendToCoordinator(msgC); 
	break; 
      }
    default:
      Assert(0);
    }
  }


  void ProtocolLazyInvalidManager::makeGCpreps(){
    a_requests.m_makeGCpreps();
    t_gcList(a_readers); 
    if(a_writer) a_writer->m_makeGCpreps(); 
  }
  } //End namespace
}
