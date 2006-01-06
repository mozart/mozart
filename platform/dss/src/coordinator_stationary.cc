/*  
 *  Authors:
 *    Zacharias El Banna, 2002 (zeb@sics.se)
 *    Erik Klintskog,     2004 (erik@sics.se)
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
#pragma implementation "coordinator_stationary.hh"
#endif

#include "dssBase.hh"
#include "msl_serialize.hh"
#include "coordinator.hh"
#include "protocols.hh"
#include "referenceConsistency.hh"
#include "coordinator_stationary.hh"


namespace _dss_internal{ //Start namespace

  
  // ****************************************************************************
  // *
  // *  Stationary AS
  // *
  // *  - one reference, no moving manager
  // ****************************************************************************
  
  

  CoordinatorStationary::CoordinatorStationary(ProtocolManager* const m,
					       const RCalg& gc_annot,
					       DSS_Environment* const env):
    Coordinator(AA_STATIONARY_MANAGER, m, env){
    m->manager = this;
    a_homeRef = new HomeReference(this, gc_annot);
  }
  
  CoordinatorStationary::~CoordinatorStationary(){
    delete a_homeRef;
  }

  void 
  CoordinatorStationary::m_initProxy(Proxy *p){
    a_proxy = p;
  }
  

  // ********************* MESSAGE ***********************************
  bool  
  CoordinatorStationary::m_sendToProxy(DSite* dest, ::MsgContainer* msg){ 
    return dest->m_sendMsg(msg); }

  ::MsgContainer *
  CoordinatorStationary::m_createProxyProtMsg(){
    return m_createASMsg(M_COORD_PROXY_PROTOCOL);
  }
  
  ::MsgContainer *
  CoordinatorStationary::m_createProxyRefMsg(){
    return m_createASMsg(M_COORD_PROXY_REF);
  }
  
  
  DSS_GC
  CoordinatorStationary::m_getDssDGCStatus(){
    if (a_homeRef->m_isRoot()) return DSS_GC_PRIMARY; 
    return DSS_GC_LOCALIZE;
  }

  void 
  CoordinatorStationary::m_receiveProtMsg(MsgContainer *msgC, DSite* sender){
    a_prot->msgReceived(msgC, sender); 
  }

  void
  CoordinatorStationary::m_makeGCpreps(){
    a_prot->makeGCpreps();
    a_homeRef->m_makeGCpreps(); 
  }


  char *
  CoordinatorStationary::m_stringrep(){
    static char buf[300];
    sprintf(buf,"STATIONARY %s",a_homeRef->m_stringrep());
    return buf;
  }


  // ************************  ProxyStationary ***************************

  
  // ********************** CONSTRUCTOR    *******************************

  ProxyStationary::ProxyStationary(NetIdentity ni, ProtocolProxy* const prot,
				   AE_ProxyCallbackInterface *ae, DSS_Environment* const env):
    Proxy(ni,AA_STATIONARY_MANAGER,prot, ae, env){
    prot->a_proxy = this;
  }


  
  ProxyStationary::~ProxyStationary(){
    //prot is deleted by Proxy
    delete a_prot;

    if (a_man == NULL){
      Assert(a_ps == PROXY_STATUS_REMOTE && a_remoteRef);
      a_remoteRef->m_dropReference();
      delete a_remoteRef;
    }
    else
      delete a_man;
  }
  
  
  void
  ProxyStationary::m_initHomeProxy(Coordinator *m){
    a_ps  = PROXY_STATUS_HOME;
    a_man = m;
    m->m_initProxy(this);
  }


  bool
  ProxyStationary::m_initRemoteProxy(DssReadBuffer *bs){
    a_ps  = PROXY_STATUS_REMOTE;
    a_remoteRef = new RemoteReference(this,bs);
    bool skel = m_getProtocol()->m_initRemoteProt(bs);
		
    DSite *hs = m_getGUIdSite(); 
    DSiteState state = hs->m_getFaultState();
    if(state != DSite_OK){
      m_siteStateChange(hs,state);
    }
    return skel;
  }


  // **************** REFERENCE *******************
  Reference*
  ProxyStationary::m_getReferenceStructure(){
    if (a_remoteRef) return a_remoteRef;
    if (a_man) return a_man->a_homeRef;
    return NULL; 
  }

  void  
  ProxyStationary::m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest){
    if (dest != m_getCoordinatorSite())
      m_getReferenceStructure()->m_getReferenceInfo(bs, dest); 
  }
  
  void  
  ProxyStationary::m_mergeReferenceInfo(DssReadBuffer *bs){
    m_getReferenceStructure()->m_mergeReferenceInfo(bs);
  }
  
  void  
  ProxyStationary::m_makePersistent(){
    m_getReferenceStructure()->m_makePersistent(); 
  }
  
  // ********************* COMMUNICATION ****************************


  bool 
  ProxyStationary::m_sendToCoordinator(::MsgContainer* msg){
    return m_getGUIdSite()->m_sendMsg(msg); 
  }
  DSite*  
  ProxyStationary::m_getCoordinatorSite(){
    return m_getGUIdSite();
  }
 
  
  
  
  // ******************* Messages ***********************
  DSS_GC
  ProxyStationary::getDssDGCStatus(){
    if (a_man == NULL){
      Assert(a_ps == PROXY_STATUS_REMOTE && a_remoteRef && a_prot);
      if (a_remoteRef->m_isRoot()) return DSS_GC_PRIMARY;
      if (a_prot->isWeakRoot()) return DSS_GC_WEAK;
      return DSS_GC_NONE;
    }
    return a_man->m_getDssDGCStatus();
  }
  

  
  void
  ProxyStationary::m_receiveProtMsg(MsgContainer *msgC, DSite* sender){
    a_prot->msgReceived(msgC, sender);
  }

  void
  ProxyStationary::m_receiveRefMsg(MsgContainer *msgC, DSite* sender){
    Assert(a_remoteRef);
    RCalg remove = a_remoteRef->m_msgToGcAlg(msgC, sender);
    if (remove != RC_ALG_PERSIST){
      MsgContainer *msg =m_createASMsg(M_PROXY_PROXY_CNET);
      msg->pushIntVal(remove);
      sender->m_sendMsg(msg);
    }
  }
  
  void
  ProxyStationary::m_makeGCpreps(){
    a_prot->makeGCpreps();
    // raph: not sure the following is correct if a_remoteRef==NULL
    if (a_remoteRef) a_remoteRef->m_makeGCpreps();
    Assert(a_man == NULL || a_ps == PROXY_STATUS_HOME);
  }
  
  char *
  ProxyStationary::m_stringrep(){
    static char buf[300];
    sprintf(buf, "STATIONARY %s %s",
	    m_getReferenceStructure()->m_stringrep(), a_prot->m_stringrep());
    return buf;
  }


  bool
  ProxyStationary::manipulateCNET(void* opaque){
    return false;
  }

  void 
  ProxyStationary::m_receiveAsMsg(MsgContainer *msgC, DSite* fromsite){
    RCalg rm = static_cast<RCalg>(msgC->popIntVal()); 
    m_getReferenceStructure()->m_removeAlgorithmType(rm); 
  }


  // ***************** Failures *******************************

    
  void
  ProxyStationary::m_siteStateChange(DSite*s, const DSiteState& state)
  {
    // ********* Access Architecture part **********
    Assert(s != m_getEnvironment()->a_myDSite);
    if(s == m_getGUIdSite()){ 
      // The Home site of the entity is affected. This directly affects the 
      // proxy. If the home site is unaccessable for any reason the proxy 
      // cannot guarante its functionality 
      switch(state){
	// remove tmp, if any 
      case DSite_OK://  setFaultState(getFaultState() & FS_AA_HOME_TMP_UNAVAIL); break; //OLD, suspicious
	setFaultState(getFaultState() & ~FS_AA_MASK);
      case DSite_TMP: setFaultState(getFaultState() | FS_AA_HOME_TMP_UNAVAIL); break;
      case DSite_GLOBAL_PRM:
      case DSite_LOCAL_PRM: setFaultState(getFaultState() | FS_AA_HOME_PRM_UNAVAIL); break;
      default:
	dssError("Unknown DSite state %d for %s",state,s->m_stringrep());
      }
    }
    
    // Clear the old protocol fault state. 
    setFaultState(getFaultState() & ~FS_PROT_MASK);
    
    // Ask the protocol if the erronous site affects its 
    // functionality. 
    setFaultState(getFaultState() | a_prot->siteStateChanged(s, state));
    if(getRegisteredFS() & getFaultState()){ 
      // GLUE_INTERFACE 
      a_AbsEnt_Interface->reportFaultState(getRegisteredFS() & getFaultState());
    }

  }
  
    void 
  ProxyStationary::m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg){
      m_siteStateChange(m_getGUIdSite(), DSite_LOCAL_PRM); 
  }
  
}
