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
#pragma implementation "protocol_eagerinvalid.hh"
#endif

#include "protocol_eagerinvalid.hh"
namespace _dss_internal{ //Start namespace

// **************************  Migratory Token  ***************************

  namespace {
    enum CacheInvalid_Message {
      ECI_INVALID_READ,    // m2p
      ECI_READ_TOKEN,      // m2p
      ECI_READ_INVALIDATED,// p2m
      ECI_REGISTER_READ,   // p2m
      ECI_DEREGISTER_READ, // p2m
      ECI_WRITE_REQUEST,   // p2m
      ECI_WRITE_TOKEN,     // m2p
      ECI_WRITE_DONE       // p2m
    };
    /*    
    static char *msg_strings[] = { "ECI_INVALID_READ",
				   "ECI_READ_TOKEN",      
				   "ECI_READ_INVALIDATED",
				   "ECI_REGISTER_READ",   
				   "ECI_DEREGISTER_READ", 
				   "ECI_WRITE_REQUEST",   
				   "ECI_WRITE_TOKEN",     
				   "ECI_WRITE_DONE" };
    
    */
    enum ECI_pendType{
      ECIPT_WRITE,
      ECIPT_READ
    };

  }
  
  
  void  ProtocolEagerInvalidManager::printStatus(){
    printf("Writers:\n");
    for(OneContainer<DSite> *ptr = a_writers.peek(); ptr!=NULL; ptr = ptr->a_next)
      printf("%s\n",ptr->a_contain1->m_stringrep()); 
    printf("Readers:\n"); 
    for(TwoContainer<DSite,bool> *ptr2 = a_readers; ptr2!=NULL; ptr2 = ptr2->a_next)
      printf("%s ----> %s\n",ptr2->a_contain1->m_stringrep(), ptr2->a_contain2?"+":"-"); 
  }

  
  void 
  ProtocolEagerInvalidManager::m_invalidateReaders()
  {
    DSite *WriterSite = a_writers.peek()->a_contain1;
    bool hasNotSent = true; 
    for(TwoContainer<DSite, bool>* ptr = a_readers; ptr!=NULL; ptr = ptr->a_next)
      {
	if (ptr->a_contain1 == WriterSite){
	  ptr->a_contain2 = false; 
	}
	else
	  {
	    MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
	    gf_createSndMsg(msgC, ECI_INVALID_READ); 
	    Assert(ptr->a_contain2);
	    (ptr->a_contain1)->m_sendMsg(msgC);
	    hasNotSent = false; 
	}
      }
    if(hasNotSent)
      m_sendWriteRight();
  }

  void 
  ProtocolEagerInvalidManager::m_updateOneReader(DSite *target){
    MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
    gf_createSndMsg(msgC,  ECI_READ_TOKEN, a_coordinator->retrieveEntityState()); 
    target->m_sendMsg(msgC); 
  }
  
  

  void 
  ProtocolEagerInvalidManager::m_updateAllReaders(DSite *exclude)
  {
    for(TwoContainer<DSite, bool>* ptr = a_readers; ptr!=NULL; ptr = ptr->a_next)
      {
	if (ptr->a_contain1 != exclude){
	  m_updateOneReader(ptr->a_contain1);
	}
	// The current holder does not have to have a copy, howver, 
	// he was marked as invalidated when the invalidation for his
	// previous read vent out... That tok me quite a while to 
	// understand .... Erik
	ptr->a_contain2 = true; 
      }
  }
  
  void 
  ProtocolEagerInvalidManager::m_sendWriteRight(){
    OneContainer<DSite>* ptr = a_writers.peek();
    MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
    gf_createSndMsg(msgC, ECI_WRITE_TOKEN); 
    (ptr->a_contain1)->m_sendMsg(msgC);
  }
  
  void
  ProtocolEagerInvalidManager::msgReceived(::MsgContainer* msg, DSite* sender){
    int message = msg->popIntVal();
    //    printf("InvM rec %s from: %s\n",msg_strings[message], sender->stringrep());
    //    printStatus();
    //    printf("\n"); 
    switch(message) {
    case ECI_WRITE_REQUEST:
      {
	bool clearReads      = a_writers.isEmpty();
	a_writers.append(new OneContainer<DSite>(sender, NULL)); 
	if (clearReads)
	  m_invalidateReaders();
	break; 
      }
    case ECI_WRITE_DONE:
      {
	OneContainer<DSite>* ele = a_writers.drop();
	Assert(ele->a_contain1 == sender);
	delete ele; 
	PstInContainerInterface* buildcont =  gf_popPstIn(msg);  
	a_coordinator->installEntityState(buildcont); 
	m_updateAllReaders(sender); 
	if(!a_writers.isEmpty())
	  m_invalidateReaders();
	break; 
      }
    case ECI_READ_INVALIDATED: 
      {
	bool allFalse = false; 
	for(TwoContainer<DSite,bool>* ptr = a_readers; ptr!= NULL; ptr = ptr->a_next)
	  {
	    if(ptr->a_contain1 == sender) {
	      ptr->a_contain2 = false; 
	    }
	    allFalse = allFalse || ptr->a_contain2; 
	  }
	if (!allFalse)
	  {
	    m_sendWriteRight();
	  }
	break; 
      }
    case ECI_REGISTER_READ:
      {
	if (a_writers.isEmpty()){
	  a_readers = new TwoContainer<DSite,bool>(sender, true, a_readers);
	  m_updateOneReader(sender);
	}
	else {
	  a_readers = new TwoContainer<DSite,bool>(sender, false, a_readers);
	}
	break;
      }
    case  ECI_DEREGISTER_READ:
      {
	if(!t_deleteCompare(&a_readers, sender))
	  a_coordinator->m_getEnvironment()->a_map->GL_warning("Deregesetering non reegistered site");
	break;
      }
    default:
      Assert(0);
    }
    //    printStatus();
    //    printf("\n\n"); 
  }


  void ProtocolEagerInvalidManager::sendMigrateInfo(MsgContainer* msg){
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
  
  ProtocolEagerInvalidManager::ProtocolEagerInvalidManager(MsgContainer* msg):a_readers(NULL),a_writers(){
    int len = msg->popIntVal();
    for(;len>0; len --) 
      a_readers = new TwoContainer<DSite,bool>(msg->popDSiteVal(),static_cast<bool>(msg->popIntVal()), a_readers);
  }
  

  
  ProtocolEagerInvalidProxy::ProtocolEagerInvalidProxy():
    ProtocolProxy(PN_EAGER_INVALID),a_readers(NULL), a_writers(NULL), a_token(ECITS_VALID){ }


  ProtocolEagerInvalidProxy::ProtocolEagerInvalidProxy(DssReadBuffer*):
    ProtocolProxy(PN_EAGER_INVALID),a_readers(NULL), a_writers(NULL), a_token(ECITS_INVALID){
  }
  

  bool
  ProtocolEagerInvalidProxy::m_initRemoteProt(DssReadBuffer*){
    ::MsgContainer *msgC  = a_proxy->m_createCoordProtMsg(); 
    gf_createSndMsg(msgC,ECI_REGISTER_READ); 
    a_proxy->m_sendToCoordinator(msgC);
    a_token = ECITS_INVALID;
    return true;
  }

  ProtocolEagerInvalidProxy::~ProtocolEagerInvalidProxy(){
    Assert(a_writers == NULL); 
    Assert(a_readers == NULL); 
    if (a_proxy->m_getProxyStatus() == PROXY_STATUS_HOME) return;
    MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
    gf_createSndMsg(msgC,ECI_DEREGISTER_READ);
    a_proxy->m_sendToCoordinator(msgC);
  }

  void 
  ProtocolEagerInvalidProxy::m_writeDone(){
    MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
    gf_createSndMsg(msgC,  ECI_WRITE_DONE, a_proxy->retrieveEntityState());
    a_proxy->m_sendToCoordinator(msgC); 
    
  }
  
  OpRetVal
  ProtocolEagerInvalidProxy::protocol_Read(GlobalThread* const th_id, PstOutContainerInterface**& msg){
    dssLog(DLL_BEHAVIOR,"EagerInvalidProxy::Read");
    msg = NULL;
    switch(a_token){
    case ECITS_INVALID:
      a_readers = new OneContainer<GlobalThread>(th_id,a_readers);
      return DSS_SUSPEND;
    case ECITS_VALID:{
      // 	  printf("Count the thread!\n");
      return DSS_PROCEED;
    }
    default:
      Assert(0);
      return DSS_INTERNAL_ERROR_SEVERE; // Whatever....
    }
  }
  
  OpRetVal
  ProtocolEagerInvalidProxy::protocol_Write(GlobalThread* const th_id, PstOutContainerInterface**& msg){
    dssLog(DLL_BEHAVIOR,"EagerInvalidProxy::Write");
    msg = NULL;
    if (a_writers == NULL){
      MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
      msgC->pushIntVal(ECI_WRITE_REQUEST);
      a_proxy->m_sendToCoordinator(msgC);
    }
    a_writers = new OneContainer<GlobalThread>(th_id, a_writers); 
    return DSS_SUSPEND;
  }

  void
  ProtocolEagerInvalidProxy::msgReceived(::MsgContainer* msg, DSite*){
    int message = msg->popIntVal();
    //    printf("invalid prx msgReceived %s\n",msg_strings[message]);
    switch(message) {
    case ECI_READ_TOKEN:{
      PstInContainerInterface* buildcont =  gf_popPstIn(msg);
      a_token = ECITS_VALID;
      a_proxy->installEntityState(buildcont);
      while(a_readers!=NULL){
	OneContainer<GlobalThread>* ptr = a_readers->a_next; 
	(a_readers->a_contain1)->resumeDoLocal(NULL);
	delete a_readers;
	a_readers  = ptr;
      }
      break; 
    }
    case ECI_WRITE_TOKEN:{
      while(a_writers != NULL){
	OneContainer<GlobalThread> *tmp = a_writers; 
	(a_writers->a_contain1)->resumeDoLocal(NULL);
	a_writers = a_writers->a_next;
	delete tmp; 
      }
      m_writeDone();
      break; 
    }
    case ECI_INVALID_READ:
      {
	a_token = ECITS_INVALID;
	MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
	gf_createSndMsg(msgC, ECI_READ_INVALIDATED);
	a_proxy->m_sendToCoordinator(msgC); 
	break; 
      }
    default:
      Assert(0);
    }
  }


  void ProtocolEagerInvalidManager::makeGCpreps(){
    a_writers.m_makeGCpreps();
    t_gcList(a_readers); 
  }
} //End namespace
