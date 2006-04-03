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

    enum ECI_pendType{
      ECIPT_WRITE,
      ECIPT_READ
    };
  }
  
  
  void  ProtocolEagerInvalidManager::printStatus(){
    printf("Writers:\n");
    for (Position<DSite*> p(a_writers); p(); p++)
      printf("%s\n", (*p)->m_stringrep()); 
    printf("Readers:\n"); 
    for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++)
      printf("%s ----> %s\n", (*p).first->m_stringrep(), (*p).second?"+":"-");
  }

  
  void 
  ProtocolEagerInvalidManager::m_invalidateReaders() {
    Assert(!a_writers.isEmpty());
    DSite* writer = a_writers.peek();
    bool hasNotSent = true; 
    for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++) {
      if ((*p).first == writer) {
	(*p).second = false;
      } else {
	MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
	gf_createSndMsg(msgC, ECI_INVALID_READ); 
	Assert((*p).second);
	(*p).first->m_sendMsg(msgC);
	hasNotSent = false; 
      }
    }
    if (hasNotSent) m_sendWriteRight();
  }

  void 
  ProtocolEagerInvalidManager::m_updateOneReader(DSite *target){
    MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
    gf_createSndMsg(msgC,  ECI_READ_TOKEN, a_coordinator->retrieveEntityState()); 
    target->m_sendMsg(msgC); 
  }
  
  

  void 
  ProtocolEagerInvalidManager::m_updateAllReaders(DSite *exclude) {
    for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++) {
      if ((*p).first != exclude) m_updateOneReader((*p).first);
      // The current holder does not have to have a copy, howver, 
      // he was marked as invalidated when the invalidation for his
      // previous read vent out... That tok me quite a while to 
      // understand .... Erik
      (*p).second = true;
    }
  }
  
  void 
  ProtocolEagerInvalidManager::m_sendWriteRight(){
    DSite* writer = a_writers.peek();
    MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
    gf_createSndMsg(msgC, ECI_WRITE_TOKEN); 
    writer->m_sendMsg(msgC);
  }
  
  void
  ProtocolEagerInvalidManager::msgReceived(::MsgContainer* msg, DSite* sender){
    int message = msg->popIntVal();
    //    printf("InvM rec %s from: %s\n",msg_strings[message], sender->stringrep());
    //    printStatus();
    //    printf("\n"); 
    switch(message) {
    case ECI_WRITE_REQUEST: {
      bool clearReads = a_writers.isEmpty();
      a_writers.append(sender); 
      if (clearReads) m_invalidateReaders();
      break;
    }
    case ECI_WRITE_DONE: {
      DSite* writer = a_writers.pop();
      Assert(writer == sender);
      PstInContainerInterface* buildcont =  gf_popPstIn(msg);  
      a_coordinator->installEntityState(buildcont); 
      m_updateAllReaders(sender); 
      if(!a_writers.isEmpty()) m_invalidateReaders();
      break; 
    }
    case ECI_READ_INVALIDATED: {
      bool allFalse = false;
      for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++) {
	if ((*p).first == sender) { (*p).second = false; }
	allFalse = allFalse || (*p).second; 
      }
      if (!allFalse) m_sendWriteRight();
      break; 
    }
    case ECI_REGISTER_READ: {
      if (a_writers.isEmpty()) {
	a_readers.push(makePair(sender, true));
	m_updateOneReader(sender);
      } else {
	a_readers.push(makePair(sender, false));
      }
      break;
    }
    case ECI_DEREGISTER_READ: {
      Position<Pair<DSite*,bool> > p(a_readers);
      if (p.find(sender))
	p.remove();
      else
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
    for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++) len++;
    msg->pushIntVal(len);
    for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++) {
      msg->pushDSiteVal((*p).first);
      msg->pushIntVal((*p).second);
    }
    // move the writers as well
    for (Position<DSite*> p(a_writers); p(); p++)
      msg->pushDSiteVal(*p);
  }
  
  ProtocolEagerInvalidManager::ProtocolEagerInvalidManager(MsgContainer* msg) :
    a_readers(),a_writers()
  {
    int len = msg->popIntVal();
    for (;len>0; len--)
      a_readers.push(makePair(msg->popDSiteVal(), (bool) msg->popIntVal()));
    // then get writers
    while (!msg->m_isEmpty())
      a_writers.append(msg->popDSiteVal());
  }
  

  
  ProtocolEagerInvalidProxy::ProtocolEagerInvalidProxy():
    ProtocolProxy(PN_EAGER_INVALID),a_readers(), a_writers(), a_token(ECITS_VALID){ }


  ProtocolEagerInvalidProxy::ProtocolEagerInvalidProxy(DssReadBuffer*):
    ProtocolProxy(PN_EAGER_INVALID),a_readers(), a_writers(), a_token(ECITS_INVALID){
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
    Assert(a_writers.isEmpty()); 
    Assert(a_readers.isEmpty()); 
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
      a_readers.append(th_id);
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
    if (a_writers.isEmpty()) {
      MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
      msgC->pushIntVal(ECI_WRITE_REQUEST);
      a_proxy->m_sendToCoordinator(msgC);
    }
    a_writers.append(th_id);
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
      // wake up readers
      while (!a_readers.isEmpty())
	a_readers.pop()->resumeDoLocal(NULL);
      break; 
    }
    case ECI_WRITE_TOKEN:{
      // wake up all writers, then "propagate" result
      while (!a_writers.isEmpty())
	a_writers.pop()->resumeDoLocal(NULL);
      m_writeDone();
      break; 
    }
    case ECI_INVALID_READ: {
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
    t_gcList(a_readers);
    t_gcList(a_writers);
  }

  void ProtocolEagerInvalidProxy::makeGCpreps(){
    t_gcList(a_readers);
    t_gcList(a_writers);
  }

} //End namespace
