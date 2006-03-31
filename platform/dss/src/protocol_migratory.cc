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
#pragma implementation "protocol_migratory.hh"
#endif

#include "protocol_migratory.hh"
#include "dssBase.hh"

namespace _dss_internal{ //Start namespace

// **************************  Migratory Token  ***************************
  namespace { // We don't need to expose these outside the file so..

    enum Migratory_Message {
      MIGM_TOKEN_GET,
      MIGM_NEED_NO_MORE,
      MIGM_TOKEN,
      MIGM_TOKEN_FORWARD,
      MIGM_TOKEN_FORWARD_SPECIAL
    };
  }



  ProtocolMigratoryProxy::ProtocolMigratoryProxy():
    ProtocolProxy(PN_MIGRATORY_STATE),a_next(NULL), a_token(MIGT_HERE),a_operations(){}
  
  bool 
  ProtocolMigratoryProxy::m_initRemoteProt(DssReadBuffer*)
  {
    a_token = MIGT_EMPTY;
    return true;
  }

  void
  ProtocolMigratoryManager::msgReceived(::MsgContainer* msg, DSite* sender){
    Migratory_Message message = static_cast<Migratory_Message>(msg->popIntVal());
    switch(message) {
    case MIGM_TOKEN_GET:{
      if(current != sender){
	::MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
	msgC->pushIntVal(MIGM_TOKEN_FORWARD);
	msgC->pushDSiteVal(sender);
	a_coordinator->m_sendToProxy(current, msgC); 
	current = sender;
      } else {
	// it is already coming to the (home) proxy
	dssLog(DLL_BEHAVIOR,"MigratoryManager::special gc case");
      }
      break;
    }
    case MIGM_NEED_NO_MORE:{
      if (current == sender){ // have not forwarded it to someone else 
	::MsgContainer *msgC = a_coordinator->m_createProxyProtMsg();
	current = a_coordinator->m_getEnvironment()->a_myDSite;
	msgC->pushIntVal(MIGM_TOKEN_FORWARD);
	msgC->pushDSiteVal(current);
	a_coordinator->m_sendToProxy(sender, msgC); 
      }
      break;
    }
    default:
      Assert(0);
    }
  }


  ProtocolMigratoryManager::ProtocolMigratoryManager(::MsgContainer* msg):current(NULL){
    current = msg->popDSiteVal();
  }


  void
  ProtocolMigratoryManager::sendMigrateInfo(::MsgContainer* msg){
    msg->pushDSiteVal(current);
  }


  void
  ProtocolMigratoryProxy::requestToken(){
    ::MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
    msgC->pushIntVal(MIGM_TOKEN_GET);
    dssLog(DLL_BEHAVIOR,"MigratoryProxy::requestToken");
    a_token = MIGT_REQUESTED;
    a_proxy->m_sendToCoordinator(msgC);
  }

  void
  ProtocolMigratoryProxy::forwardToken(){
    ::PstOutContainerInterface* ans = a_proxy->retrieveEntityState();
    dssLog(DLL_BEHAVIOR,"MigratoryProxy::forwardToken : To:%s",a_next->m_stringrep());
    ::MsgContainer *msgC = a_proxy->m_createProxyProtMsg();
    msgC->pushIntVal(MIGM_TOKEN);
    gf_pushPstOut(msgC, ans);
    a_token = MIGT_EMPTY;
    a_proxy->m_sendToProxy(a_next,msgC);
    a_next  = NULL;
  }


  void
  ProtocolMigratoryProxy::resumeOperations(){
    Assert(!a_operations.isEmpty() ||
	   a_proxy->m_getProxyStatus() == PROXY_STATUS_HOME);
    while (!a_operations.isEmpty()) {
      Pair<Migratory_Operation, GlobalThread*> op = a_operations.pop();
      dssLog(DLL_BEHAVIOR,"MigratoryProxy::resumeOperation: %d, Thread:%d",
	     op.first, op.second);
      switch (op.first) {
      case MIGO_ACCESS:
	(op.second)->resumeDoLocal(NULL); // ERIK!!
	break;
      case MIGO_FORWARD:
	forwardToken(); // always forward token and...
	// if Queue is not empty then request it again
	if (!a_operations.isEmpty()) requestToken();
	return;
      default:
	Assert(0);
	a_proxy->m_getEnvironment()->a_map->GL_error("Migratory Proxy: error, unknown operation %d", op.first);
	return;
      }
    }
    if(a_next != NULL){ // How did we get here (no lock anymore)?
      // Assert(0);
      forwardToken();
    }
  }


  OpRetVal
  ProtocolMigratoryProxy::protocol_Access(GlobalThread* const id, ::PstOutContainerInterface**& out){
    dssLog(DLL_BEHAVIOR,"MigratoryProxy::Access operation (%d)",a_token);
    out = NULL; // Never transmit
    switch(a_token){
    case MIGT_HERE:
      return DSS_PROCEED;
    case MIGT_EMPTY:
      requestToken();
      // fall through
    case MIGT_REQUESTED:
      a_operations.append(makePair(MIGO_ACCESS, id));
      return DSS_SUSPEND;
    default:
      Assert(0);
      return DSS_INTERNAL_ERROR_SEVERE; // Whatever....
    }
  }
  
  void
  ProtocolMigratoryProxy::msgReceived(::MsgContainer* msg, DSite* sender){
    Migratory_Message message = static_cast<Migratory_Message>(msg->popIntVal());
    switch(message) {
    case MIGM_TOKEN_FORWARD:{
      Assert(a_next == NULL);
      a_next = msg->popDSiteVal(); // The manager should never send it twice
      if (a_token == MIGT_HERE)
	forwardToken();
      else
	a_operations.append(makePair(MIGO_FORWARD, (GlobalThread*) NULL));
      break;
    }
    case MIGM_TOKEN:{
      a_token = MIGT_HERE;
      ::PstInContainerInterface* buildcont =  gf_popPstIn(msg);
      a_proxy->installEntityState(buildcont);
      resumeOperations();
      break;
    }
    default:
      Assert(0);
    }
  }


  bool
  ProtocolMigratoryProxy::clearWeakRoot(){
    if(isWeakRoot()){ // Must check again since we don't know if something happened since last check
      //if token == MIGT_REQUESTED then we should rely on the glue to mark it so skip it here
      if(a_next == NULL){
	dssLog(DLL_BEHAVIOR,"MigratoryProxy::clearing weak root\n");
	::MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
	msgC->pushIntVal(MIGM_NEED_NO_MORE);
	a_proxy->m_sendToCoordinator(msgC);
      } else
	forwardToken(); // How could this happen? (lock removed)
      return false; //Whatever happened we've only tried to clear it
    }
    return true;
  }


  void
  ProtocolMigratoryProxy::makeGCpreps() {
    if (a_next) a_next->m_makeGCpreps();

    Position<Pair<Migratory_Operation, GlobalThread*> > pos;
    for (pos(a_operations); pos(); pos++)
      if ((*pos).first == MIGO_ACCESS) (*pos).second->m_makeGCpreps();
  }

} //End namespace
