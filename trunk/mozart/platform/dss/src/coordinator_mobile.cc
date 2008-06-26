/*  
 *  Authors:
 *    Erik Klintskog, 2004 (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *   Erik Klintskog, 2004
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
#pragma implementation "coordinator_mobile.hh"
#endif

#include "coordinator_mobile.hh"
#include "protocols.hh"
#include "msl_serialize.hh"
#include "dss_largeMessages.hh"
#include "dss_dksBackbone.hh"

namespace _dss_internal{ //Start namespace
  
  enum MobileCoordinatorMsgs{
    MCM_COORDSTATE,
    MCM_REQUEST,
    MCM_MIGRATED,
    MCM_NEWLOCATION,
    MCM_REQUESTLOC
  };



  class MobileCoordBS: public BackboneService{
  private:
    int a_epoch; 
    DSite *a_location;
  public: 
    MobileCoordBS(LargeMessage *lm):a_epoch(lm->popInt()), a_location(lm->popDSiteVal())
    {
      printf("MobileCoordBC -- installing(%d)\n", a_epoch); 
    }
    
    MobileCoordBS(DSite* loc, int epoch):
      a_epoch(epoch), a_location(loc)
    {
      printf("Building MobilCoordBC(%d)\n", a_epoch); 
    }
    
    void m_messageReceived(LargeMessage* lm, DSS_Environment* env){
      printf("MobileCoordBC -- Receiving msg\n");
      switch(lm->popInt()){
      case MCM_NEWLOCATION:{
	int epoch = lm->popInt();
	DSite* loc = lm->popDSiteVal(); 
	if(epoch > a_epoch)
	  {
	    a_epoch = epoch; 
	    a_location = loc; 
	  }
	break; 
      }
      case MCM_REQUESTLOC:{
	DSite *sender =  lm->popDSiteVal(); 
	NetIdentity   ni =  lm->popNetId(); 
	::MsgContainer *msgC = env->a_msgnLayer->createAppSendMsgContainer(); 
	msgC->pushIntVal(M_PROXY_CNET);
	gf_pushNetIdentity(msgC, ni); 
	msgC->pushIntVal(MCM_MIGRATED); 
	msgC->pushDSiteVal(a_location); 
	msgC->pushIntVal(a_epoch); 
	sender->m_sendMsg(msgC); 
	break; 
      }
      default:
	printf("MobileCoordBS -- unhandeled message\n"); 
      }
    }
    
    LargeMessage* m_transferService(){
      printf("MobileCoordBC(%d) -- m_transferService\n", a_epoch); 
      LargeMessage* lm = new LargeMessage();
      lm->pushInt(a_epoch); 
      lm->pushDSiteVal(a_location); 
      return lm;
    } 

    int m_getType(){
      return  BST_MOBILE_COORDINATOR;
    }
  };
  
  
  BackboneService* gf_createMcBackbineST(LargeMessage* lm){
    return new MobileCoordBS(lm); 
  }
  
  // *********************** CoordinatorMobileInternals **********************************

   
  void
  CoordinatorMobile::m_initProxy(Proxy *p){
    a_proxy  = p;
  }
  
  
  void 
  CoordinatorMobile::m_migrateCoordinator(DSite* sender){
    if(a_proxy){
      static_cast<ProxyMobile*>(a_proxy)->m_makeRemote(sender, a_epoch +1 ); 
    }
    MsgContainer *msg =  m_createASMsg(M_COORD_PROXY_CNET);
    msg->pushIntVal(MCM_COORDSTATE);
    msg->pushIntVal(a_epoch+1); 
    a_prot->sendMigrateInfo(msg);
    sender->m_sendMsg(msg); 
    m_getEnvironment()->a_coordinatorTable->m_del(this);
  }
  
  // *********************** CoordinatorMobileInterface **********************************
  
  
  

  // ************** CONSTRUCTORS *******************
  CoordinatorMobile::CoordinatorMobile(ProtocolManager* const prot,
				       const RCalg& gc_annot,
				       DSS_Environment* const env):
    Coordinator(AA_MOBILE_COORDINATOR, prot, env),
    a_epoch(1)
   {
     prot->a_coordinator = this;
     a_homeRef = new HomeReference(this, gc_annot); 
     MobileCoordBS* mcbs = new MobileCoordBS(env->a_myDSite, a_epoch); 
     env->a_dksBackbone->m_insertService( m_getNetId(), mcbs); 
   }
  
  CoordinatorMobile::CoordinatorMobile(NetIdentity ni,
				       DSS_Environment* const env, 
				       int epoch, 
				       ProxyMobile *prx,
				       MsgContainer *msg):
     Coordinator(ni, AA_MOBILE_COORDINATOR, NULL, env),
     a_epoch(epoch){
     a_proxy = prx; 
     a_homeRef = new HomeReference(this,RC_ALG_PERSIST); 
     a_prot = gf_createProtManager(msg, a_proxy->m_getProtocol()->getProtocolName()); 
     a_prot->a_coordinator = this;

     // informing the backbone service about the new loction
     LargeMessage *lmsg = new LargeMessage(); 
     lmsg->pushInt(MCM_NEWLOCATION); 
     lmsg->pushInt(a_epoch); 
     lmsg->pushDSiteVal(env->a_myDSite); 
     env->a_dksBackbone->m_sendToService(ni, lmsg); 
   }
    
  CoordinatorMobile::~CoordinatorMobile()
  {
    ;
  }
  
  // ****************** Message Abstractions **********************
   
  ::MsgContainer *
  CoordinatorMobile::m_createProxyProtMsg(){
    ::MsgContainer *msg =  m_createASMsg(M_COORD_PROXY_PROTOCOL);
    msg->pushIntVal(a_epoch); 
    return msg; 
  }
  
  ::MsgContainer *
  CoordinatorMobile::m_createProxyRefMsg(){
    ::MsgContainer *msg  = m_createASMsg(M_COORD_PROXY_REF);
    msg->pushIntVal(a_epoch); 
    return msg; 
  }
  
  bool 
  CoordinatorMobile::m_sendToProxy(DSite* dest, MsgContainer* msg){
    return dest->m_sendMsg(msg);
  }
  
  // ****************** Message Receivers *************************
   void 
  CoordinatorMobile::m_receiveProtMsg(MsgContainer *msgC, DSite* fromsite){
     printf("received prot msg\n"); 
     a_prot->msgReceived(msgC, fromsite); 
   }
  
  void 
  CoordinatorMobile::m_receiveRefMsg( MsgContainer *msgC, DSite* fromsite){
    a_homeRef->m_msgToGcAlg(msgC,fromsite);
  }
  
  void 
  CoordinatorMobile::m_receiveAsMsg(  MsgContainer *msgC, DSite* sender){
    printf("receiving as %d\n", a_epoch); 
    int type = msgC->popIntVal(); 
    switch(type){
    case MCM_REQUEST:
      m_migrateCoordinator(sender); 
      break; 
    }
  }  
  // ******************* MISC *************************************
  
  char *
  CoordinatorMobile::m_stringrep(){ 
    return "Mobile Coordinator"; 
  }
  
  
  //******************* GC routines ******************************
   
  void 
  CoordinatorMobile::m_makeGCpreps(){
    if(a_prot) a_prot->makeGCpreps(); // Gc the protocol coordinator
    
    // The proxy and the coordinator share reference. If the proxy exists, it will 
    // g-collect the dgc structure, if not it has to be done by the coordinatorx
    if (a_proxy == NULL) a_homeRef->m_makeGCpreps(); 
  }
  
  DSS_GC 
  CoordinatorMobile::m_getDssDGCStatus(){

    if(a_homeRef->m_isRoot()) return DSS_GC_PRIMARY; 
    return DSS_GC_LOCALIZE;
  }
  
  // ******************* Failure handlers ************************
  void     
  CoordinatorMobile::m_siteStateChange(DSite *s , const FaultState& st){
    printf("CoordinatorMobile::m_siteStateChange -- dont care\n"); 
  }
  
  void  
  CoordinatorMobile::m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg){
    printf("CoordinatorMobile::m_noCoordAtDest -- dont care\n"); 
  }
  
  void  
  CoordinatorMobile::m_noProxyAtDest(DSite* sender, MessageType mtt, MsgContainer* msg){
    printf("CoordinatorMobile::m_noProxyAtDest -- dont care\n"); 
  }
  

  void 
  CoordinatorMobile::m_undeliveredCoordMsg(DSite* dest, MessageType mtt,MsgContainer* msg){
    printf("CoordinatorMobile::m_undeliveredCoordMsg -- dont care\n"); 
  }
  void 
  CoordinatorMobile::m_undeliveredProxyMsg(DSite* dest, MessageType mtt,MsgContainer* msg){
    printf("CoordinatorMobile::m_undeliveredProxyMsg -- dont care\n"); 
  }

  

  // ******************** ProxyMobile Class ********************************

  ProxyMobile::ProxyMobile(NetIdentity ni, ProtocolProxy* const p,
			   DSS_Environment* const env):
    Proxy(ni, AA_MOBILE_COORDINATOR, p, env),
    a_coordSite(NULL), a_fl_coordLost(false), a_epoch(-1)
  {
    p->a_proxy = this;
  }
  
  void 
  ProxyMobile::m_initHomeProxy(Coordinator *m){
    a_ps  = PROXY_STATUS_HOME;
    a_coordinator = m;
    a_epoch = 1; 
    m->m_initProxy(this);
    a_coordSite = m_getEnvironment()->a_myDSite; 
  }
  
  bool
  ProxyMobile::m_initRemoteProxy(DssReadBuffer *bs){
    a_coordSite  = m_getEnvironment()->a_msgnLayer->m_UnmarshalDSite(bs);
    a_epoch      = gf_UnmarshalNumber(bs); 
    a_ps         = PROXY_STATUS_REMOTE;
    BYTE refInfo = bs->getByte(); 
    if((bool) refInfo)
      a_remoteRef  = new RemoteReference(this,bs);
    else // make a persistent ref(no danger)
      a_remoteRef  = new RemoteReference(this); 
    return m_getProtocol()->m_initRemoteProt(bs);
  }
  void 
  ProxyMobile::m_makePersistent(){
    if(a_remoteRef)
      a_remoteRef->m_makePersistent(); 
    else
      a_coordinator->a_homeRef->m_makePersistent(); 
  }

  ProxyMobile::~ProxyMobile(){
    delete a_prot;
    if(a_ps == PROXY_STATUS_REMOTE){
      a_remoteRef->m_dropReference();
      delete a_remoteRef;
    }
    if (a_coordinator != NULL)
      delete a_coordinator;
  }


  
  
   // ************** Message Receivers  ********************
  

  void
  ProxyMobile::m_receiveProtMsg(MsgContainer *msgC, DSite* fromsite){
    int epoch = msgC->popIntVal(); 
    if(epoch > a_epoch && !a_fl_coordLost){
      a_fl_coordLost = true;
      a_coordSite = NULL;
      m_locateCoordinator(); 
    }
    a_prot->msgReceived(msgC,fromsite);
  }
  
  void 
  ProxyMobile::m_receiveRefMsg(MsgContainer *msgC, DSite* fromsite){
    int epoch = msgC->popIntVal(); 
    if(epoch > a_epoch && !a_fl_coordLost){
      a_fl_coordLost = true;
      a_coordSite = NULL;
      m_locateCoordinator(); 
    }
      
      a_remoteRef->m_msgToGcAlg(msgC,fromsite);
  }
  
  void
  ProxyMobile::m_receiveAsMsg(MsgContainer *msgC, DSite*){
    int type = msgC->popIntVal();
    switch(type){
    case MCM_COORDSTATE:
      {
	int epoch = msgC->popIntVal(); 
	a_coordinator = new CoordinatorMobile(m_getNetId(), m_getEnvironment(),
					      epoch, this, msgC);
	a_ps  = PROXY_STATUS_HOME;
	m_receivedNewCoordInfo(m_getEnvironment()->a_myDSite, epoch);
	break; 
      }
    case MCM_MIGRATED:
      {
	DSite* site = msgC->popDSiteVal(); 
	int epoch = msgC->popIntVal(); 
	m_receivedNewCoordInfo(site, epoch);
	break;
      }
    default: Assert(0);
    }
  }
  
  // **************** GC ***************************

  void
  ProxyMobile::m_makeGCpreps(){
    a_prot->makeGCpreps();
    a_remoteRef->m_makeGCpreps();
    printf("we should g-collect stored msgs\n"); 
  }
  
  char *
  ProxyMobile::m_stringrep(){
    return "Proxy Mobile"; 
  }
  

 // ***************** REFERENCE *******************
  
  void  
  ProxyMobile::m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest){
    m_getCoordinatorSite()->m_marshalDSite(bs);
    gf_MarshalNumber(bs, a_epoch); 
    if (dest != m_getCoordinatorSite())
      bs->putByte((BYTE)false);
    else{
      bs->putByte((BYTE)true);
      a_remoteRef->m_getReferenceInfo(bs, dest); 
    }
  }

  int
  ProxyMobile::m_getReferenceSize(DSite* dest) {
    DSite* coord = m_getCoordinatorSite();
    return coord->m_getMarshaledSize() + sz_MNumberMax + 1 +
      (dest != coord ? 0 : a_remoteRef->m_getReferenceSize());
  }

  void
  ProxyMobile::m_mergeReferenceInfo(DssReadBuffer *bs){
    if(bs->getByte())
      a_remoteRef->m_mergeReferenceInfo(bs);
  }

  DSS_GC
  ProxyMobile::getDssDGCStatus(){
    if (a_remoteRef->m_isRoot()) return DSS_GC_PRIMARY; 
    if (a_unsentMsgs.isEmpty() == false) return DSS_GC_PRIMARY;
    return DSS_GC_LOCALIZE; 
  }
  
  
  bool
  ProxyMobile::manipulateCNET(void* arg){
    MsgContainer *msgC = m_createASMsg(M_PROXY_COORD_CNET);
    msgC->pushIntVal(MCM_REQUEST);
    m_sendToCoordinator(msgC); 
    return true; 
  }
    
  
  // ************* Communication ********************

  bool
  ProxyMobile::m_sendToCoordinator(MsgContainer* msg){
    if(a_fl_coordLost) {
      a_unsentMsgs.append(msg); 
      return true;
    }
    a_coordSite->m_sendMsg(msg);
    return true; 
  }
  
  bool 
  ProxyMobile:: m_sendToProxy(DSite* dest, ::MsgContainer* msg) {
    return dest->m_sendMsg(msg); 
  }
  

  DSite*
  ProxyMobile::m_getCoordinatorSite(){
    return a_coordSite; 
  }

  ::MsgContainer *
  ProxyMobile::m_createProxyProtMsg(){
     ::MsgContainer *msg =  m_createASMsg(M_PROXY_PROXY_PROTOCOL);
    msg->pushIntVal(a_epoch); 
    return msg; 
  }
  ::MsgContainer *
  ProxyMobile::m_createProxyRefMsg(){
    ::MsgContainer *msg  = m_createASMsg(M_PROXY_PROXY_REF);
    msg->pushIntVal(a_epoch); 
    return msg; 
  }

  // ****************** FAILURES ************************
  void
  ProxyMobile::m_siteStateChange(DSite *s, const FaultState& state) {
    if (s == a_coordSite && (state & FS_PERM)) {
      a_fl_coordLost = true;
      a_coordSite = NULL; 
      m_locateCoordinator();
    }
  }
  

  void 
  ProxyMobile::m_undeliveredCoordMsg(DSite*, MessageType mtt, MsgContainer* msg){
    // We only store messages addressed to the coordinator
    a_unsentMsgs.append(msg->reincarnate()); 
    if(!a_fl_coordLost){
      a_fl_coordLost = true;
      a_coordSite = NULL;
      m_locateCoordinator(); 
    }
  }
  
  void 
  ProxyMobile::m_undeliveredProxyMsg(DSite*, MessageType, MsgContainer*) {}

  void 
  ProxyMobile::m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg){
    printf("ProxyMobile::m_noCoordAtDest\n"); 
    a_unsentMsgs.append(msg->reincarnate()); 
    if(!a_fl_coordLost){
      a_fl_coordLost = true;
      a_coordSite = NULL;
      m_locateCoordinator(); 
    }
  }
  
  void 
  ProxyMobile::m_noProxyAtDest(DSite*, MessageType, MsgContainer*) {}

  // **************** MOBILITY METHODS *******************
    
  void
  ProxyMobile::m_receivedNewCoordInfo(DSite* site, int epoch){
    printf("New coordination info %d\n", a_epoch); 
    if(a_epoch < epoch){
      a_epoch  = epoch; 
      a_coordSite = site;
      a_fl_coordLost = false; 
      while (!a_unsentMsgs.isEmpty()) {
	MsgContainer* msg = a_unsentMsgs.pop();
	msg->m_convert2Send();
	m_sendToCoordinator(msg);
      }
    }
  }
  
  void
  ProxyMobile::m_locateCoordinator(){
    printf("trying to find the coordinator\n"); 
    LargeMessage *lmsg = new LargeMessage(); 
    lmsg->pushInt(MCM_REQUESTLOC);
    lmsg->pushDSiteVal(m_getEnvironment()->a_myDSite);
    lmsg->pushNetId(m_getNetId()); 
    m_getEnvironment()->a_dksBackbone->m_sendToService(m_getNetId(), lmsg); 
  }

  
  void 
  ProxyMobile::m_makeRemote(DSite* sender, int epoch){
    m_setProxyStatus(PROXY_STATUS_REMOTE);
    if(a_remoteRef)
      a_remoteRef->m_makePersistent(); 
    else
      a_remoteRef = new RemoteReference(this); 
    a_coordinator = NULL; 
    a_epoch = epoch; 
    a_coordSite = sender; 
  }
  
} // end namespace
