/*  Authors:
 *    Zacharias El Banna, 2002 (zeb@sics.se) Wrote and designed the original coordinator
 *                                           structure called manager. This was a direct 
 *                                           continuation of what was found in Mozart.
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
#pragma implementation "coordinator_fwdchain.hh"
#endif


#include "dssBase.hh"
#include "msl_serialize.hh"
#include "coordinator.hh"
#include "protocols.hh"
#include "referenceConsistency.hh"
#include "coordinator_fwdchain.hh"

// Why these includes
#include "protocol_once_only.hh"
#include "protocol_simple_channel.hh"
#include "protocol_migratory.hh"
#include "protocol_eagerinvalid.hh"
#include "protocol_lazyinvalid.hh"
#include "dss_msgLayerInterface.hh"

namespace _dss_internal{ //Start namespace
  

  enum FWDC_MSG {
    MA_REQUEST, 
    MA_FORWARD,
    MA_NEWREF,
    MA_ABORT,
    MA_COMMIT,
    MA_REFREMOVED,
    MA_YES
  };

#define  FWDC_NO_REFINFO 0
#define  FWDC_REFINFO    1

  
  class ProxyDct; 
  
#define DRI_triplet ThreeContainer<DSite, Reference, int>
#define SrcMsg_pair TwoClassContainer<DSite, MsgContainer>  

  void
  CoordinatorFwdChain::m_forwardMessage(MsgContainer* msgC, DSite* fromsite, DSite *dest){
    MsgContainer *msg = m_createASMsg(M_COORD_COORD_CNET);
    msg->pushIntVal(MA_FORWARD);
    msg->pushDSiteVal(fromsite);
    msg->pushMsgC(msgC); 
    dest->m_sendMsg(msg);
  }
  
  int CoordinatorFwdChain::m_getEpoch(){
    switch(a_ms){
    case MANAGER_STATUS_REF_INC:
    case MANAGER_STATUS_PROT:
      return a_refList->a_contain2;  
    case MANAGER_STATUS_WAITING: 
    case MANAGER_STATUS_REF_COM:
      return static_cast<ProxyFwdChain*>(a_proxy)->a_epoch; 
    default:
      return 0;
    } 
  }
  
  DSite* CoordinatorFwdChain::m_getCoordSite(){
    switch(a_ms){
    case MANAGER_STATUS_REF_INC:
    case MANAGER_STATUS_PROT:
      return m_getEnvironment()->a_myDSite;  
    case MANAGER_STATUS_WAITING: 
    case MANAGER_STATUS_REF_COM:
      return static_cast<ProxyFwdChain*>(a_proxy)->a_coordSite;
    default: 
      return NULL; 
    } 
  }
  
  void 
  CoordinatorFwdChain::m_sendRefUpdateCoord(DSite* s){
    MsgContainer *ans = m_createASMsg(M_COORD_COORD_CNET);
    ans->pushIntVal(MA_NEWREF);
    ans->pushDSiteVal(m_getEnvironment()->a_myDSite); 
    ans->pushIntVal(a_refList->a_contain2); 
    InfiniteWriteBuffer *bs =new InfiniteWriteBuffer();
    a_refList->a_contain1->m_getReferenceInfo(bs, s);
    gf_pushEBA(ans, new EdcByteArea(bs->m_getBuffer())); 
    s->m_sendMsg(ans); 
    delete bs;
  }

  void 
  CoordinatorFwdChain::m_sendRefUpdateProxy(DSite* s){
    MsgContainer *ans = m_createASMsg(M_COORD_PROXY_CNET);
    ans->pushIntVal(MA_NEWREF);
    ans->pushDSiteVal(m_getEnvironment()->a_myDSite); 
    ans->pushIntVal(a_refList->a_contain2); 
    InfiniteWriteBuffer *bs =new InfiniteWriteBuffer();
    a_refList->a_contain1->m_getReferenceInfo(bs, s);
    gf_pushEBA(ans, new EdcByteArea(bs->m_getBuffer())); 
    s->m_sendMsg(ans); 
    delete bs;
  }
  
  
  // ********************* CONSTRUCTORS **************************************
  CoordinatorFwdChain::CoordinatorFwdChain( ProtocolManager* const pm, const unsigned int& gc_annot,
					    DSS_Environment* const env):
    Coordinator( AA_MIGRATORY_MANAGER, pm, env),
    a_refList(NULL), 
    a_deliverQueue(),
    a_coordPtr(env->a_myDSite),
    a_ms(MANAGER_STATUS_PROT){
    pm->manager = this;
    a_refList = new TwoContainer<HomeReference,int>(new HomeReference(this, gc_annot), 0, NULL); 
  }

  
  CoordinatorFwdChain::CoordinatorFwdChain(NetIdentity ni, ProxyFwdChain* const p,
				     DSS_Environment* const env):
    Coordinator(ni, AA_MIGRATORY_MANAGER, NULL, env),
    a_refList(NULL),
    a_deliverQueue(),
    a_coordPtr(NULL),
    a_ms(MANAGER_STATUS_REF_COM){ //We are a comlete ref
    a_proxy  = p;
    p->a_man = this; 
    a_refList = NULL;
  }


  CoordinatorFwdChain::~CoordinatorFwdChain(){
    printf("deleteing a coordinator - fwdchain\n"); 
  }

  
  void
  CoordinatorFwdChain::m_queueProtMessage(MsgContainer *m, DSite* fromsite){
    dssLog(DLL_ALL,"QUEUEING migratory message");
    a_deliverQueue.append(new SrcMsg_pair(fromsite, m, NULL));
  }
  
  

  // if called with this site as destination, all the messages are 
  // delivered localy.
  void
  CoordinatorFwdChain::m_deliverProtMessages(DSite* dest){
    dssLog(DLL_ALL,"DELIVERING migratory messages");
    TwoClassContainer<DSite, MsgContainer>  *q;
    while((q = a_deliverQueue.drop()) != NULL){
      dssLog(DLL_ALL,"delivering migratory message from %s",q->a_contain1->m_stringrep());
      MsgContainer *msg = q->a_contain2; 
      if(dest == m_getEnvironment()->a_myDSite){
	dssLog(DLL_DEBUG,"Migratory protocol message delivered locally");
	a_prot->msgReceived(msg,q->a_contain1);
      } else {
	dssLog(DLL_DEBUG,"Migratory protocol message delivered forwarded");  
	m_forwardMessage(msg,q->a_contain1,dest);
      }
    }
  }

  
  void
  CoordinatorFwdChain::m_initProxy(Proxy *p){
    a_proxy  = p;
  }

  void
  CoordinatorFwdChain::m_initiateMigration(){
    Assert(a_proxy->a_man == this);
    MsgContainer *msgC = m_createASMsg(M_COORD_COORD_CNET);
    msgC->pushIntVal(MA_REQUEST);
    m_getCoordSite()->m_sendMsg(msgC);
    a_ms = MANAGER_STATUS_WAITING;
    a_coordPtr  =  m_getCoordSite(); 
  }
  
  // ***************** COMMUNICATION ****************************

  ::MsgContainer * CoordinatorFwdChain::m_createProxyProtMsg(){
    ::MsgContainer *msg = m_createASMsg(M_COORD_PROXY_PROTOCOL);
    msg->pushIntVal(m_getEpoch()); 
    return msg;
  }
  ::MsgContainer * CoordinatorFwdChain::m_createProxyRefMsg(){
    ::MsgContainer *msg = m_createASMsg(M_COORD_PROXY_REF);
    msg->pushIntVal(m_getEpoch()); 
    return msg;
  }
  bool  CoordinatorFwdChain::m_sendToProxy(DSite* dest, MsgContainer* msg){
    return dest->m_sendMsg(msg); 
  }
  
  
  void 
  CoordinatorFwdChain::m_receiveRefMsg(MsgContainer *msgC, DSite* fromsite){
    unsigned int e = msgC->popIntVal();
    TwoContainer<HomeReference, int> *ptr = NULL; 
    for(ptr = a_refList; ptr!=NULL && ptr->a_contain2 != e ; ptr = ptr->a_next);
    if(ptr)
      ptr->a_contain1->m_msgToGcAlg(msgC,fromsite);
  }





   void
   CoordinatorFwdChain::m_receiveProtMsg(MsgContainer *msgC, DSite* fromsite){
     printf("CoordinatorFwdChain::m_receiveProtMsg\n"); 
     if(a_ms == MANAGER_STATUS_PROT){
       int e = msgC->popIntVal(); 
       if(e < m_getEpoch()) 
	 m_sendRefUpdateProxy(fromsite);  
       a_prot->msgReceived(msgC,fromsite);
     }
     else if(a_ms == MANAGER_STATUS_WAITING)
       m_queueProtMessage(msgC,fromsite);
     else
       m_forwardMessage(msgC,fromsite,m_getCoordSite()); 
   }


   void
   CoordinatorFwdChain::m_sendMigratingState(DSite *fromsite)
   {
     dssLog(DLL_DEBUG,"Sending Migrating State to %s", fromsite->m_stringrep());
     Assert(a_ms == MANAGER_STATUS_PROT);

     a_ms = MANAGER_STATUS_REF_INC;
     a_coordPtr = fromsite;

     a_proxy->m_setProxyStatus(PROXY_STATUS_REMOTE);

     MsgContainer *ans = m_createASMsg(M_COORD_COORD_CNET);
     ans->pushIntVal(MA_YES);

     ans->pushIntVal(m_getEpoch() + 1); // The next a_epoch and ...
     ans->pushIntVal(a_homeRef->m_getAlgorithms()); // .. what algs to use
     a_prot->sendMigrateInfo(ans);
     fromsite->m_sendMsg(ans);
   }

   void
   CoordinatorFwdChain::m_receiveAsMsg(MsgContainer *msgC, DSite* fromsite){
     int type = msgC->popIntVal();
     dssLog(DLL_ALL,"Received AS Message:%d",type);

     switch(type){
     case MA_REQUEST:{ // Initiate migration
       if (a_ms == MANAGER_STATUS_PROT){// Ok initiate migration
	 m_sendMigratingState(fromsite);
       } else {                       // Else we are either in transit or building
	 m_sendRefUpdateCoord(fromsite); 
	 MsgContainer *ans = m_createASMsg(M_COORD_COORD_CNET);
	 ans->pushIntVal(MA_ABORT);
	 fromsite->m_sendMsg(ans);
       }
       break;
     }
     case MA_ABORT: // Abort, when RootList == NULL it will be ok to remove
       Assert(a_ms == MANAGER_STATUS_WAITING);
       Assert(a_coordPtr = reinterpret_cast<DSite*>(0xbedda));
       a_ms = MANAGER_STATUS_REF_COM; // wait until rootList == NULL
       // Anyway, deliver stored messages from our site to old manager
       m_deliverProtMessages(m_getCoordSite());
       // we try to migrate again! 
       m_initiateMigration();
       break;
     case MA_YES:{
       unsigned int e = msgC->popIntVal();
       unsigned int a = msgC->popIntVal();
       
       a_prot = gf_createProtManagarer(msgC, a_proxy->m_getProtocol()->getProtocolName()); 
       
       a_prot->manager = this;


       // Droping the reference to the old coord and 
       // creating a new reference domain. 
       static_cast<ProxyFwdChain*>(a_proxy)->a_ref->m_dropReference();
       delete static_cast<ProxyFwdChain*>(a_proxy)->a_ref;
       static_cast<ProxyFwdChain*>(a_proxy)->a_ref = NULL;
       
       a_refList = new TwoContainer<HomeReference, int>(new HomeReference(this,a),
							e,
							a_refList->a_next);
       // send new reference
       m_sendRefUpdateCoord(fromsite);
      
       MsgContainer *ans = m_createASMsg(M_COORD_COORD_CNET);
      ans->pushIntVal(MA_COMMIT);
      fromsite->m_sendMsg(ans);

      // Make the Manager start receiving messages. 
      a_ms = MANAGER_STATUS_PROT;
    
      // Manipulating the proxy
      a_proxy->m_setProxyStatus(PROXY_STATUS_HOME);
      
      //deliver all messages to protocol
      m_deliverProtMessages(m_getEnvironment()->a_myDSite);
      break;
    }
    case MA_COMMIT:{
      a_ms = MANAGER_STATUS_REF_COM;
      Assert(a_coordPtr = reinterpret_cast<DSite*>(0xbedda));
      break;
    }
    case MA_FORWARD:{
      DSite* from = msgC->popDSiteVal();
      if (a_ms == MANAGER_STATUS_REF_INC || a_ms == MANAGER_STATUS_REF_COM){
	m_forwardMessage(msgC,from,m_getCoordSite());
      }
      else {

	if (a_ms == MANAGER_STATUS_PROT){
	  m_sendRefUpdateProxy(from); 
	  a_prot->msgReceived(msgC,from);
	}
	else m_queueProtMessage(msgC,from);
      }
      break;
    }
     case MA_NEWREF:
       {
	 DSite *loc = msgC->popDSiteVal(); 
	 int epoch = msgC->popIntVal();
	 EdcByteArea* eba = gf_popEBA(msgC); 
	 static_cast<ProxyFwdChain*>(a_proxy)->_mergeReference(epoch,loc, eba->m_getReadBufInterface()); 
	 break;
	}
      default: Assert(0);
      }
    }


   DSS_GC
   CoordinatorFwdChain::m_getDssDGCStatus(){
     // Old ref versions 
     TwoContainer<HomeReference, int> **ptr = &a_refList;
     while((*ptr) != NULL){
       if((*ptr)->a_contain1->m_isRoot())
	 return DSS_GC_PRIMARY; 
       TwoContainer<HomeReference, int> *tmp = (*ptr); 
       (*ptr) = (*ptr)->a_next;
       delete tmp;
     }
     if(a_ms == MANAGER_STATUS_REF_COM) return DSS_GC_NONE; // Migrated
     return DSS_GC_LOCALIZE; // Protocol manager
   }
  
  void
  CoordinatorFwdChain::m_makeGCpreps(){
    for(TwoClassContainer<DSite, MsgContainer>* ptr = a_deliverQueue.peek(); 
	ptr!= NULL ; ptr = ptr->a_next)
      ptr->a_contain1->m_makeGCpreps(); 
    t_gcList(a_refList);
    a_prot->makeGCpreps();
  }


  char *
  CoordinatorFwdChain::m_stringrep(){
    static char buf[300];
    sprintf(buf,"MIGRATORY [Status:%d Ref:%s]\n", a_ms, a_homeRef->m_stringrep());
    return buf;
  }
  
  // **********************' FAILURE ******************************
  
  void CoordinatorFwdChain::m_msgUnsent(DSite*s, MsgContainer* msg){
    ;
  }
  void CoordinatorFwdChain::m_siteStateChange(DSite *, const DSiteState&){
    ;
  }
  
  void CoordinatorFwdChain::m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg){
    ;
  }
  void CoordinatorFwdChain::m_noProxyAtDest(DSite* sender, MessageType mtt, MsgContainer* msg){
    ;
  }

  


  // ********************************* PROXYFWDCHAIN *************************************

  // ********* REFERENCE ***********
  
  
  void 
  ProxyFwdChain::m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest){
    int epoch; 
    DSite *coordSite; 
    Reference *ref; 
    // if we have a remote ref, use it. If not, we share home ref with the 
    // coordinator that must be here. 
    if(a_ref){
      ref = a_ref; 
      epoch = a_epoch;
      coordSite = a_coordSite; 
    }
    else{
      coordSite = m_getEnvironment()->a_myDSite;
      ref       = static_cast<CoordinatorFwdChain*>(a_man)->a_refList->a_contain1; 
      epoch     = static_cast<CoordinatorFwdChain*>(a_man)->a_refList->a_contain2;
    }
    
    if( dest == coordSite){
      bs->putByte(FWDC_NO_REFINFO); 
    }else{
    bs->putByte(FWDC_REFINFO); 
    gf_MarshalNumber(bs, epoch); 
    coordSite->m_marshalDSite(bs); 
    ref->m_getReferenceInfo(bs, dest);
    }
  }
  
  void ProxyFwdChain::m_mergeReferenceInfo(DssReadBuffer *bs){
    if(bs->getByte() == FWDC_NO_REFINFO)
      return; 
    
    unsigned int epoch = gf_UnmarshalNumber(bs);
    DSite *coordSite = m_getEnvironment()->a_msgnLayer->m_UnmarshalDSite(bs);
    _mergeReference(epoch, coordSite, bs); 
  }
  
  void
  ProxyFwdChain::_mergeReference(int epoch, DSite *coord, DssReadBuffer* bs){
    int myEpoch =m_getEpoch();
    
    if(myEpoch == epoch){
      printf(" Received epoch equal to held \n"); 
      if(a_ref)
	a_ref->m_mergeReferenceInfo(bs); 
      else
	static_cast<CoordinatorFwdChain*>(a_man)->a_refList->a_contain1->m_mergeReferenceInfo(bs); 
      return; 
    }
    
    if(myEpoch < epoch){
      if(a_ref){
	a_ref->m_dropReference();
	delete a_ref; 
      }
      a_coordSite =  coord;
      a_ref       =  new RemoteReference(this, bs);
      a_epoch     =  epoch;
      return; 
    }
    printf(" Received epoch smaller than held");

    DSite            *tmpS = a_coordSite; 
    RemoteReference  *tmpR = a_ref;
    int               tmpE = a_epoch; 
    
    // installing new
    a_coordSite =  coord ;
    a_epoch     =  epoch;
    a_ref       =  new RemoteReference(this, bs);
    
    // droping new
    // The remote reference needs to find the coordinator(located in the 
    // a_coordSite field to properly drop): 
    a_ref->m_dropReference();
    delete a_ref; 
  
    // reinstalling
    a_coordSite =  tmpS;
    a_epoch     =  tmpE;
    a_ref       =  tmpR; 

  }
  
  void ProxyFwdChain::m_makePersistent(){
    if(a_ref)
      a_ref->m_makePersistent();
    static_cast<CoordinatorFwdChain*>(a_man)->a_refList->a_contain1->m_makePersistent(); 
  }

  
  // *******************  CNET **********************
  
  bool 
  ProxyFwdChain::m_sendToCoordinator(::MsgContainer* msg){
    return m_getCoordinatorSite()->m_sendMsg(msg); 
  }
  
  DSite* 
  ProxyFwdChain::m_getCoordinatorSite(){
    if(a_ref)
      return a_coordSite;
    return m_getEnvironment()->a_myDSite;
  }
  
  
  ::MsgContainer * 
  ProxyFwdChain::m_createCoordProtMsg(){
    ::MsgContainer *msg = m_createASMsg(M_PROXY_COORD_PROTOCOL);
    msg->pushIntVal(m_getEpoch());
    return msg; 
  }
  
  ::MsgContainer * 
  ProxyFwdChain::m_createProxyProtMsg(){
    ::MsgContainer *msg = m_createASMsg(M_PROXY_PROXY_PROTOCOL);
    msg->pushIntVal(m_getEpoch());
    return msg; 
  };
  
  ::MsgContainer * 
  ProxyFwdChain::m_createCoordRefMsg(){
    ::MsgContainer *msg = m_createASMsg(M_PROXY_COORD_REF);
    msg->pushIntVal(m_getEpoch());
    return msg; 
  };

  ::MsgContainer * 
  ProxyFwdChain::m_createProxyRefMsg(){
    ::MsgContainer *msg = m_createASMsg(M_PROXY_PROXY_REF);
    msg->pushIntVal(m_getEpoch());
    return msg; 
  };
  
  // ************* Communication ********************
  
  ProxyFwdChain::ProxyFwdChain(NetIdentity ni, ProtocolProxy* const p,
				 AE_ProxyCallbackInterface *ae, DSS_Environment* const env):
    Proxy(ni, AA_MIGRATORY_MANAGER,p,ae,env), a_coordSite(NULL), a_ref(NULL), a_epoch(0){
    p->a_proxy = this;
  }

  void
  ProxyFwdChain::m_initHomeProxy(Coordinator *m){
    a_ps  = PROXY_STATUS_HOME;
    a_man = m;
    m->m_initProxy(this);
  }
  
  bool
  ProxyFwdChain::m_initRemoteProxy(DssReadBuffer *bs){
    a_ps  = PROXY_STATUS_REMOTE;
    BYTE stat = bs->getByte();
    if(stat = FWDC_REFINFO){
      a_epoch        = gf_UnmarshalNumber(bs);
      a_coordSite    = m_getEnvironment()->a_msgnLayer->m_UnmarshalDSite(bs);
      a_ref          = new RemoteReference(this,bs);
    }else{
      printf("something is seriously wrong with a fwd-chaining proxy\n");
      printf("no reference info is received.\n"); 
      Assert(0); 
    }
    bool skel = m_getProtocol()->m_initRemoteProt(bs);
    return skel;
  }
  
  ProxyFwdChain::~ProxyFwdChain(){
    // We must first delete the protocol instance. 
    // If the ref goes first, we encounter possible 
    // problems. 
    // 1. The protocol might need the ref for its id. ( getProtManagerSite() )
    // 2. The Manager might come to the conclusion
    // that the entity can be reclaimed, if the ref goes
    // before the protoocol.
    delete a_prot;
    
    if(a_ps == PROXY_STATUS_REMOTE){ // independent of manager it should drop if deleted
      a_remoteRef->m_dropReference();
      delete a_remoteRef;
    }
    if (a_man != NULL)
      delete a_man;
  }


  void 
  ProxyFwdChain::m_receiveRefMsg(MsgContainer *msgC, DSite* fromsite){
    unsigned int epoch = msgC->popIntVal();
    if(a_ref && epoch == a_epoch){
      RCalg remove = a_ref->m_msgToGcAlg(msgC,fromsite);
      if (remove != RC_ALG_PERSIST){
	MsgContainer *msg = m_createASMsg(M_PROXY_PROXY_CNET);	
	msg->pushIntVal(a_epoch);
	msg->pushIntVal(remove);
	fromsite->m_sendMsg(msg);
      }
      printf("msg not handled epoch:%d our epoch:%d\n", epoch, a_epoch); 
    }
  }
  
  void
  ProxyFwdChain::m_receiveAsMsg(MsgContainer *msgC, DSite*){
    int type = msgC->popIntVal();
    switch(type){
    case MA_NEWREF:{
      DSite *coord = msgC->popDSiteVal(); 
      int epoch    = msgC->popIntVal();
      EdcByteArea* eba = gf_popEBA(msgC); 
      _mergeReference(epoch,coord, eba->m_getReadBufInterface());
      break; 
    }
    case MA_REFREMOVED:{
      int epoch    = msgC->popIntVal();
      RCalg rm = static_cast<RCalg>(msgC->popIntVal()); 
      if(a_ref && a_epoch == epoch)
	a_ref->m_removeAlgorithmType(rm); 
      break; 
    }
    default: Assert(0);
    }
  }
  

      

  void
  ProxyFwdChain::m_makeGCpreps(){
    a_prot->makeGCpreps();
    if(a_ref){
      a_ref->m_makeGCpreps(); 
      a_coordSite->m_makeGCpreps(); 
    }
  }
  
  DSS_GC
  ProxyFwdChain::getDssDGCStatus(){
    if(a_man){
      DSS_GC man_status = static_cast<CoordinatorFwdChain*>(a_man)->m_getDssDGCStatus();
      if (man_status != DSS_GC_NONE)
	return man_status;
    }
    if (a_remoteRef->m_isRoot()) return DSS_GC_PRIMARY;
    if (a_prot->isWeakRoot()) return DSS_GC_WEAK;
    return DSS_GC_NONE;
  }
  
  
  void
  ProxyFwdChain::m_receiveProtMsg(MsgContainer *msgC, DSite* fromsite){
    int epoch = msgC->popIntVal(); 
    if (epoch < m_getEpoch())
      m_updateRemote(fromsite); 
    if(epoch > m_getEpoch())
      printf("we are out of epoch\n"); 
    a_prot->msgReceived(msgC,fromsite);
  }
  
  
  char *
  ProxyFwdChain::m_stringrep(){
    static char buf[300];
    sprintf(buf,"MIGRATORY %s %s\n",a_remoteRef->m_stringrep(), a_prot->m_stringrep());
    return buf;
  }


  bool
  ProxyFwdChain::manipulateCNET(void* opaque){
    
    // Ok here we initialize the first step in the migration:
    // - create an incomplete manager (if none exist) this 
    // enables two things, first we have an endpoint for comm.
    // and second we will guard our proxy. HOWEVER, we wont 
    // install the ref just yet.
    // - send a requets for migration
    
    CoordinatorFwdChain *mm = static_cast<CoordinatorFwdChain *>(a_man);
    if(mm == NULL){
      mm = new CoordinatorFwdChain(m_getNetId(),this, m_getEnvironment());
    }
    // Lock proxy and send request
    switch(static_cast<CoordinatorFwdChain*>(mm)->a_ms){
      //Pass Thread ID below if we want suspension
    case MANAGER_STATUS_REF_COM: 
      static_cast<CoordinatorFwdChain*>(mm)->m_initiateMigration();
    case MANAGER_STATUS_WAITING:
    case MANAGER_STATUS_PROT:    return true;
    case MANAGER_STATUS_REF_INC: return false;
    default: Assert(0); m_getEnvironment()->a_map->GL_error("Migratory manager state invalid");
    }
    return false; 
  }
  
  int
  ProxyFwdChain::m_getEpoch(){
    if(a_ref) return a_epoch; 
    return static_cast<CoordinatorFwdChain*>(a_man)->a_refList->a_contain2; 
  }

  void 
  ProxyFwdChain::m_updateRemote(DSite* s){
    printf("we must fix the update remote\n"); 
  }
  // ***************** Failures *******************************
  void ProxyFwdChain::m_siteStateChange(DSite *, const DSiteState&){
    ;
  }
  void ProxyFwdChain::m_msgUnsent(DSite*s, MsgContainer* msg){
    ;
  }
  
  void ProxyFwdChain::m_noCoordAtDest(DSite* sender, MessageType mtt, MsgContainer* msg){
    ;
  }
  void ProxyFwdChain::m_noProxyAtDest(DSite* sender, MessageType mtt, MsgContainer* msg){
    ;
  }

  

} //End namespace 
