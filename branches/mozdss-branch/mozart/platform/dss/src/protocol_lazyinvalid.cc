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



  void 
  ProtocolLazyInvalidManager::m_updateOneReader(DSite *target){
    MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
    gf_createSndMsg(msgC,  LCI_READ_TOKEN, a_coordinator->retrieveEntityState()); 
    target->m_sendMsg(msgC); 
  }

  void 
  ProtocolLazyInvalidManager::m_handleNextRequest() {
    if (a_writer!=NULL) {
      MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
      gf_createSndMsg(msgC,  LCI_INVALID_WRITE); 
      a_writer->m_sendMsg(msgC); 

    } else {
      // Get all queued read requests
      while (!a_requests.isEmpty() && a_requests.peek().second) {
	Pair<DSite*, bool> req = a_requests.pop();
	a_readers.push(req);
	m_updateOneReader(req.first);
      }
      if (!a_requests.isEmpty()) {
	// Invalidate all readers, dont invalidate the next writer
	DSite* writer = a_requests.peek().first;
	Position<Pair<DSite*, bool> > r(a_readers);
	while (r()) {
	  if ((*r).first == writer) {
	    r.remove();
	  } else {
	    MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
	    gf_createSndMsg(msgC,  LCI_INVALID_READ); 
	    (*r).first->m_sendMsg(msgC);
	    r++;
	  }
	}
	// optimization: if the readers contained no other process
	// that the writer, send immediately
	if (a_readers.isEmpty()) {
	  m_sendWriteRight();
	  a_writer = writer;
	  // So, we have now passed the state to a writer.  If there
	  // are more requests in the Pipe, they must be handled.
	  if (!a_requests.isEmpty()) m_handleNextRequest();
	}
      }
    }
  }


  void 
  ProtocolLazyInvalidManager::m_sendWriteRight(){
    Pair<DSite*, bool> req = a_requests.pop();
    MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
    gf_createSndMsg(msgC, LCI_WRITE_TOKEN, a_coordinator->retrieveEntityState()); 
    req.first->m_sendMsg(msgC);
  }
  
  void
  ProtocolLazyInvalidManager::msgReceived(::MsgContainer* msg, DSite* sender){
    int message = msg->popIntVal();
    switch(message) {
    case LCI_WRITE_REQUEST: {
      bool ready = a_requests.isEmpty();
      a_requests.append(makePair(sender, false));
      if (ready) m_handleNextRequest();
      break;
    }
    case LCI_WRITE_INVALIDATED: {
      PstInContainerInterface* buildcont =  gf_popPstIn(msg); 
      a_coordinator->installEntityState(buildcont); 
      Assert(sender == a_writer);
      a_writer = NULL;
      Assert(a_readers.isEmpty());
      a_readers.push(makePair(sender, true));
      m_handleNextRequest();
      break;
    }
    case LCI_READ_INVALIDATED: {
      Position<Pair<DSite*, bool> > p(a_readers);
      if (p.find(sender)) p.remove(); else { Assert(0); }
      if (a_readers.isEmpty()) m_handleNextRequest();
      break;
    }
    case LCI_READ_REQUEST: {
      bool ready = a_requests.isEmpty();
      a_requests.append(makePair(sender, true));
      if (ready) m_handleNextRequest();
      break;
    }
    default:
      Assert(0);
    }
  }


  void ProtocolLazyInvalidManager::sendMigrateInfo(::MsgContainer* msg){
    int len = 0;
    for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++) len++;
    msg->pushIntVal(len);
    for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++) {
      msg->pushDSiteVal((*p).first);
      msg->pushIntVal((*p).second);
    }
    // send writer and requests as well
    msg->pushDSiteVal(a_writer);
    for (Position<Pair<DSite*,bool> > p(a_readers); p(); p++) {
      msg->pushDSiteVal((*p).first);
      msg->pushIntVal((*p).second);
    }
  }

  ProtocolLazyInvalidManager::ProtocolLazyInvalidManager(::MsgContainer* msg):
    a_readers(), a_requests(), a_writer(NULL)
  {
    int len = msg->popIntVal();
    for (;len>0; len --)
      a_readers.push(makePair(msg->popDSiteVal(), (bool) msg->popIntVal()));
    // then get writer and requests
    a_writer = msg->popDSiteVal();
    while (!msg->m_isEmpty())
      a_requests.append(makePair(msg->popDSiteVal(), (bool) msg->popIntVal()));
  }

  ProtocolLazyInvalidManager::ProtocolLazyInvalidManager(DSite *mysite):
    a_readers(),a_requests(), a_writer(mysite)
  {} 
    


  ProtocolLazyInvalidProxy::ProtocolLazyInvalidProxy():
    ProtocolProxy(PN_LAZY_INVALID),a_readers(), a_writers(), a_token(LCITS_WRITE_TOKEN){ }


  ProtocolLazyInvalidProxy::ProtocolLazyInvalidProxy(DssReadBuffer*):
    ProtocolProxy(PN_LAZY_INVALID),a_readers(), a_writers(), a_token(LCITS_INVALID){}
  

  bool
  ProtocolLazyInvalidProxy::m_initRemoteProt(DssReadBuffer*){
    a_token = LCITS_INVALID;
    return true;
  }

  ProtocolLazyInvalidProxy::~ProtocolLazyInvalidProxy(){
    Assert(a_writers.isEmpty());
    Assert(a_readers.isEmpty());
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
      if (a_readers.isEmpty()) m_requestReadToken();
      a_readers.append(th_id);
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
    if (a_token == LCITS_WRITE_TOKEN) {
      return DSS_PROCEED;
    } else {
      if (a_writers.isEmpty()) m_requestWriteToken();
      a_writers.append(th_id);
      return DSS_SUSPEND;
    }
  }

	  
  void
  ProtocolLazyInvalidProxy::msgReceived(::MsgContainer* msg, DSite*){
    int message = msg->popIntVal();
    switch(message) {
    case LCI_READ_TOKEN:{
      ::PstInContainerInterface* buildcont =  gf_popPstIn(msg);
      a_token = LCITS_READ_TOKEN;
      a_proxy->installEntityState(buildcont);
      // wake up readers
      while (!a_readers.isEmpty())
	a_readers.pop()->resumeDoLocal(NULL);
      break; 
    }
    case LCI_WRITE_TOKEN:{
      a_token = LCITS_WRITE_TOKEN;
      PstInContainerInterface* buildcont =  gf_popPstIn(msg);
      a_proxy->installEntityState(buildcont);
      // wake up writers
      while (!a_writers.isEmpty())
	a_writers.pop()->resumeDoLocal(NULL);
      break; 
    }
    case LCI_INVALID_READ: {
      a_token = LCITS_INVALID;
      MsgContainer *msgC  = a_proxy->m_createCoordProtMsg();
      gf_createSndMsg(msgC, LCI_READ_INVALIDATED);
      a_proxy->m_sendToCoordinator(msgC);
      break;
    }
    case LCI_INVALID_WRITE: {	
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
    t_gcList(a_readers);
    t_gcList(a_requests);
    if (a_writer) a_writer->m_makeGCpreps();
  }

  void ProtocolLazyInvalidProxy::makeGCpreps(){
    t_gcList(a_readers);
    t_gcList(a_writers);
  }

} //End namespace
