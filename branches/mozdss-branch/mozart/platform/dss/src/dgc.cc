/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2004
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
#pragma implementation "dgc.hh"
#endif

#include "dgc.hh"
#include "referenceConsistency.hh"
#include "coordinator.hh"

namespace _dss_internal{
#ifdef DEBUG_CHECK
  int GCalgorithm::a_allocated=0;
#endif

#ifdef DEBUG_CHECK
  int HomeReference::a_allocated=0;
  int RemoteReference::a_allocated=0;
#endif


  // ************************  Reference ***************************

  char *
  GCalgorithm::m_stringrep(){
    return "not implemented"; 
  }


  DSS_Environment* HomeGCalgorithm::m_getEnvironment() const { return a_homeRef->m_getEnvironment(); }
  
  MsgContainer* 
  HomeGCalgorithm::m_createRemoteMsg(){
    return a_homeRef->a_coordinator->m_createProxyRefMsg();
  }
  
  bool 
  HomeGCalgorithm::m_sendToRemote(DSite* s, MsgContainer* msg){
    return a_homeRef->a_coordinator->m_sendToProxy(s, msg); 
  }
  // ********************* class RemoteGCalgorithm ********************************************
  
  DSS_Environment* 
  RemoteGCalgorithm::m_getEnvironment() const {
    return a_remoteRef->m_getEnvironment(); 
  }
  
  bool 
  RemoteGCalgorithm::m_isHomeSite(DSite* s){
    return a_remoteRef->a_proxy->m_getCoordinatorSite() == s; 
  }
  
  MsgContainer* 
  RemoteGCalgorithm::m_createHomeMsg(){
    return a_remoteRef->a_proxy->m_createCoordRefMsg();
  }
  
  MsgContainer*
  RemoteGCalgorithm::m_createRemoteMsg(){ 
    return a_remoteRef->a_proxy->m_createProxyRefMsg();
  }
  
  void 
  RemoteGCalgorithm::m_sendToHome(MsgContainer* msg){
    a_remoteRef->a_proxy->m_sendToCoordinator(msg); 
  }

  void
  RemoteGCalgorithm::m_sendToRemote(DSite* s, MsgContainer* msg){
    a_remoteRef->a_proxy->m_sendToProxy(s, msg); 

  }
  
}
