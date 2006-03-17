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
#pragma implementation "protocol_pilgrim.hh"
#endif

#include "protocol_pilgrim.hh"
namespace _dss_internal{ //Start namespace

  // **************************  Migratory Token  ***************************
    
    enum Pilgrim_Message {
      PLGM_REGISTER_REQUEST,
      PLGM_NEW_NEXT,
      PLGM_ENTER_LEAVE_DONE, 
      PLGM_PROXY_FLUSH,
      PLGM_DEREGISTER_REQUEST,
      PLGM_CONTENTS, 
      PLGM_SOLE_HOLDER,
      PLGM_CLEAR_NEXT,
      PLGM_REMOVED_FROM_RING,
      PLGM_INSERTED_IN_RING
      
    };
    
    
    //     static char* plgMsgVector[PLGM_REMOVED_FROM_RING + 1] = 
    //       {
    // 	"register",
    // 	"set_next", 
    // 	"deregsiter", 
    // 	"contents", 
    // 	"sole_holder",
    // 	"clear_next",
    // 	"removed"
    //       };
    
    //     static char* plgSttVector[ PLGT_SOLE_MEMBER + 1] = 
    //       {
    // 	"non_member", 
    // 	"member", 
    // 	"spawning",
    // 	"waiting",
    // 	"sole_member"
    //       };
  
  
    RingElement::RingElement(DSite* s, RingElement* p, RingElement* n):
      a_site(s), a_prev(p), a_next(n){;}
    
    
    ProtocolPilgrimManager::ProtocolPilgrimManager(DSite* s):
      a_ringEle(NULL), a_enterLeaveQueue(){
      a_ringEle = new RingElement(s,NULL, NULL);
      a_ringEle->a_next = a_ringEle; 
      a_ringEle->a_prev = a_ringEle; 
    }
  
    ProtocolPilgrimProxy::ProtocolPilgrimProxy(DSite *s):
      ProtocolProxy(PN_PILGRIM_STATE),  a_next(s), a_state(PLGT_SOLE_MEMBER),
      a_Pqueue(), a_jobs(0), a_use(1){
    }
    
    bool
    ProtocolPilgrimProxy::m_initRemoteProt(DssReadBuffer*)
    {
      a_state = PLGT_NON_MEMBER;
      return true;
    }
    
    void
    ProtocolPilgrimManager::m_enterLeave(){
      
      TwoContainer<DSite, bool> *first = a_enterLeaveQueue.peek(); 
      DSite *sender = first->a_contain1; 
      //      printf("%s: %d %s\n", first->a_contain2?"Enter":"Leave", a_coordinator->m_getGUIdIndex(), sender->stringrep()); 
      
      if(first->a_contain2)// enter 
	{
	  RingElement *next = a_ringEle->a_next; 
	  RingElement *re = new RingElement(sender, a_ringEle, next);
	  gf_sendManagerToProxy(a_coordinator,a_ringEle->a_site,PLGM_NEW_NEXT,sender);
	  a_ringEle->a_next = re; 
	  next->a_prev = re; 
	}
      else // leave
	{
	  RingElement *ctr = a_ringEle; 
	  RingElement *rm = NULL;
	  do{
	    if(ctr->a_site == sender){
	      rm = ctr; 
	      break; 
	    }
	    ctr = ctr->a_next; 
	  }while(ctr!=a_ringEle);
	  Assert(rm != NULL);
	 
	  if (a_ringEle->a_next == a_ringEle)
	    {
	      gf_sendManagerToProxy(a_coordinator, sender, PLGM_SOLE_HOLDER);
	      return; 
	    }
	  RingElement *prev = rm->a_prev; 
	  RingElement *next = rm->a_next;
	  
	  prev->a_next = next; 
	  next->a_prev = prev; 
	  gf_sendManagerToProxy(a_coordinator, prev->a_site, PLGM_CLEAR_NEXT, next->a_site);
	  if(rm == a_ringEle)
	    a_ringEle = rm->a_next;
	  delete rm; 
	}
      // RingElement *ptr = a_ringEle; 
      //      printf("Ring\n"); 
      //      do{
      //	printf("%s\n", ptr->a_site->stringrep()); 
      //	ptr = ptr->a_next; 
      //    }while(ptr!=a_ringEle); 

	
    }
    
    void
    ProtocolPilgrimManager::msgReceived(::MsgContainer* msg, DSite* sender){
      int message = msg->popIntVal();
      switch(message) {
      case PLGM_REGISTER_REQUEST:{
	bool doDirect = a_enterLeaveQueue.isEmpty(); 
	a_enterLeaveQueue.append(new TwoContainer<DSite,bool>(sender, true, NULL)); 
	if (doDirect) 
	  m_enterLeave(); 
	break;
      }
      case PLGM_ENTER_LEAVE_DONE:
	{
	  TwoContainer<DSite,bool>* ele = a_enterLeaveQueue.drop();
	  delete ele; 
	  if(!a_enterLeaveQueue.isEmpty())
	    m_enterLeave();
	  break; 
	}
      case PLGM_DEREGISTER_REQUEST:{
	bool doDirect = a_enterLeaveQueue.isEmpty(); 
	a_enterLeaveQueue.append(new TwoContainer<DSite,bool>(sender, false, NULL)); 
	if (doDirect) 
	  m_enterLeave(); 
	break;
      }
      default:
	Assert(0);
      }
    }


    ProtocolPilgrimManager::ProtocolPilgrimManager(::MsgContainer* msg):
      ProtocolManager(), a_ringEle(NULL),
      a_enterLeaveQueue(){
      Assert(0);
    }
  

    void
    ProtocolPilgrimManager::sendMigrateInfo(::MsgContainer* msg){
      Assert(0); 
    }


    void
    ProtocolPilgrimProxy::m_deregisterToken(){
      gf_sendProxyToManager(a_proxy,PLGM_DEREGISTER_REQUEST);
    }


    void
    ProtocolPilgrimProxy::m_requestToken(){
      gf_sendProxyToManager(a_proxy,PLGM_REGISTER_REQUEST);
      a_state = PLGT_RING_MEMBER;
    }
    
    void
    ProtocolPilgrimProxy::m_forwardToken(){
      PstOutContainerInterface* ans = a_proxy->retrieveEntityState();
      gf_sendProxyToProxy(a_proxy, a_next, PLGM_CONTENTS,ans);   
      a_state = PLGT_RING_MEMBER;
    }

    void
    ProtocolPilgrimProxy::m_resumeOperations(){
      OneContainer<GlobalThread>* cont;
      while(!a_Pqueue.isEmpty()){
	cont =  a_Pqueue.drop(); // Drop a container..
	if ((cont->a_contain1)->resumeDoLocal(NULL) == WRV_CONTINUING)
	  a_jobs ++; 
	delete cont; // .. and delete it
      }
    }


    OpRetVal
    ProtocolPilgrimProxy::protocol_Access(GlobalThread* const id, PstOutContainerInterface**& msg){
      msg = NULL;
    
      a_use = 3; // we have to fidle with this...
      switch (a_state)
	{
	case PLGT_NON_MEMBER:
	  {
	    m_requestToken(); 
	    // Continue and suspend the thread
	  }
	case PLGT_WAITING_FOR_JOBS: 
	  // the state is here, but we cannot 
	  // afford more than one invokatins to avoid startvation
	case PLGT_RING_MEMBER:
	  {
	    a_Pqueue.append(new OneContainer<GlobalThread>(id,NULL));
	    return DSS_SUSPEND;
	  }
	case PLGT_SOLE_MEMBER: 
	  {
	    // a_jobs ++;
	    return DSS_PROCEED;
	  }
	default: 
	  Assert(0); 
	}
      return DSS_INTERNAL_ERROR_NO_OP;
    }
  
  
    void
    ProtocolPilgrimProxy::msgReceived(::MsgContainer* msg, DSite* sender){
      int message = msg->popIntVal();
      switch(message) {
      case PLGM_NEW_NEXT:{
	Assert(a_state!=PLGT_NON_MEMBER); 
	DSite* new_next = msg->popDSiteVal();
	gf_sendProxyToProxy(a_proxy, new_next, PLGM_INSERTED_IN_RING, a_next);
	a_next = new_next; 
	// For complete correctnes, the old next should be queried as well, this
	// to make sure that eventual state in the pipe is flushed. Thus, the handeling 
	// of enter and leave requests cannot continue untill two messages are received,
	// one from the new next and one from the old next. At that point the ring is in a
	// concistent state. 
      
	if(a_state ==  PLGT_SOLE_MEMBER)
	  {
	    if(a_jobs == 0){
	      a_state = PLGT_RING_MEMBER;
	      m_forwardToken(); 
	    }
	    else
	      a_state = PLGT_WAITING_FOR_JOBS;
	  }
	break; 
      }
    
      case PLGM_INSERTED_IN_RING:{
	a_next = msg->popDSiteVal();
	gf_sendProxyToManager(a_proxy, PLGM_ENTER_LEAVE_DONE);
	break; 
      }
      case PLGM_CONTENTS:
	{
	  Assert(a_state == PLGT_RING_MEMBER); 
	  PstInContainerInterface* buildcont =  gf_popPstIn(msg);
	  a_proxy->installEntityState(buildcont);
	  a_state = PLGT_SPAWNING_JOBS;
	  bool noOps = a_Pqueue.isEmpty();
	  m_resumeOperations();
	  if(a_next == (a_proxy->m_getEnvironment())->a_myDSite)
	    {
	      a_state = PLGT_SOLE_MEMBER;
	      //	    printf("I'm, %d, a SOUL maan!\n",a_proxy->m_getGUIdIndex());
	    }
	  else
	    if (a_jobs == 0)
	      {
		if(noOps)
		  {
		    if(a_use ==  1) 
		      m_deregisterToken();
		    a_use --; 
		  }
		m_forwardToken(); 
	      }
	    else
	      a_state = PLGT_WAITING_FOR_JOBS; 
	  return; 
	}
      case PLGM_SOLE_HOLDER:{
	//      printf("The server thinks I have SOUL %d\n", a_proxy->m_getGUIdIndex());
	// The instance should know this!!
	break; 
      }
      case PLGM_CLEAR_NEXT:{
	gf_sendProxyToProxy(a_proxy,a_next, PLGM_REMOVED_FROM_RING);
	a_next = msg->popDSiteVal();
	//if( a_next == (a_proxy->m_getEnvironment())->a_myDSite)
	//printf("I'll soon have the SOUL, see ya at the cross road %d\n", a_proxy->m_getGUIdIndex()); 
	if ( a_state == PLGT_WAITING_FOR_JOBS && a_next == (a_proxy->m_getEnvironment())->a_myDSite)
	  a_state = PLGT_SOLE_MEMBER;
	break; 
      }
      case PLGM_PROXY_FLUSH: 
	gf_sendProxyToManager(a_proxy, PLGM_ENTER_LEAVE_DONE); 
	break; 
      case PLGM_REMOVED_FROM_RING:{
	gf_sendProxyToProxy(a_proxy,a_next, PLGM_PROXY_FLUSH);
	if(a_state == PLGT_WAITING_FOR_JOBS || !a_Pqueue.isEmpty()){
	  // A race condition, new requests arrived while deregsestring, 
	  // reregestier
	  m_requestToken();
	}
	else
	  {
	    a_next = NULL; 
	    a_state = PLGT_NON_MEMBER;
	  }
	break; 
      }
      default:
	{
	  Assert(0); 
	}
    
      }
    }
    

    bool
    ProtocolPilgrimProxy::clearWeakRoot(){
      return false;
    }
  

    void   
    ProtocolPilgrimProxy::remoteInitatedOperationCompleted(DssOperationId* opId,PstOutContainerInterface* pstOut){
      // This should _NOT_ happend!!!
      Assert(0); 
    }

    void 
    ProtocolPilgrimManager::makeGCpreps(){
      RingElement *ctr = a_ringEle; 
      do{
	ctr->a_site->m_makeGCpreps();
	ctr = ctr->a_next; 
      }while(ctr!=a_ringEle);
      a_enterLeaveQueue.m_makeGCpreps();
    }

    void
    ProtocolPilgrimProxy::localInitatedOperationCompleted()
    {
      Assert(a_jobs > 0);
      a_jobs--; 
      if (a_jobs == 0 && a_state == PLGT_WAITING_FOR_JOBS) 
	{ 
	  // Here we must check so we dont send to ourselfsf....
	  // However, not a problem until we can deregister
	  m_forwardToken();
	}
    }
  
  ProtocolPilgrimProxy::~ProtocolPilgrimProxy(){
    //    printf("deleteing %d\n", a_proxy->m_getGUIdIndex());
  }

  void 
  ProtocolPilgrimProxy::makeGCpreps()
  { 
    if (a_next != NULL) a_next->m_makeGCpreps();
    a_Pqueue.m_makeGCpreps();
  }
  
} //End namespace






