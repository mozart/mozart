/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
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
#pragma implementation "protocol_simple_channel.hh"
#endif

#include "protocol_simple_channel.hh"
namespace _dss_internal{ //Start namespace

  namespace {
    enum SIMPLE_PROT_MODE{
      ASYNCH,
      SYNCH
    };
  }

  class SimpleOp: public DssOperationId{
  public: 
    GlobalThread *a_threadId; 
    DSite* a_sender; 
    SimpleOp(GlobalThread* id, DSite* sender):a_threadId(id), a_sender(sender){;}

  private:
    SimpleOp(const SimpleOp&):a_threadId(NULL), a_sender(NULL){}
    SimpleOp operator=(const SimpleOp&){ return *this; }
  };
  
  ProtocolSimpleChannelManager::ProtocolSimpleChannelManager(){}

  ProtocolSimpleChannelManager::ProtocolSimpleChannelManager(::MsgContainer *msg){
    ::PstInContainerInterface* builder = gf_popPstIn(msg);
    static_cast<ProtocolSimpleChannelProxy*>(manager->m_getProxy()->m_getProtocol())->stateHolder = true; 
    manager->installEntityState(builder); 
   }

   void
   ProtocolSimpleChannelManager::msgReceived(::MsgContainer *msg, DSite* sender){
     int absOp   = msg->popIntVal();
     int synch   = msg->popIntVal();
     ::PstInContainerInterface* builder = gf_popPstIn(msg);
     SimpleOp* so;
     // Must be declared here since we need a ref to it in m_doe
     ::PstOutContainerInterface* ans = NULL;
     if(synch == SYNCH){
       GlobalThread *id = gf_popThreadIdVal(msg, manager->m_getEnvironment());
       so = new SimpleOp(id,sender);

       if (manager->m_doe(static_cast<AbsOp>(absOp), id, so, builder, ans) == AOCB_FINISH){
        delete so; 
        ::MsgContainer *msgC = manager->m_createProxyProtMsg();
        gf_pushThreadIdVal(msgC,id);
        gf_pushPstOut(msgC,ans);
        sender->m_sendMsg(msgC);
       }
     } else {
       so = new SimpleOp(NULL,NULL);
       if (manager->m_doe(static_cast<AbsOp>(absOp), NULL, so, builder, ans) == AOCB_FINISH)
        delete so; 
     }
   }

   void
   ProtocolSimpleChannelProxy::remoteInitatedOperationCompleted(DssOperationId* opId,
								::PstOutContainerInterface* pstOut)
   {
     SimpleOp *so = static_cast<SimpleOp*>(opId); 
     if (so->a_sender!=NULL){
       ::MsgContainer *msgC = a_proxy->m_createProxyProtMsg();
       gf_pushThreadIdVal(msgC,so->a_threadId);
       gf_pushPstOut(msgC,pstOut);
       (so->a_sender)->m_sendMsg(msgC);
     }
     delete so; 
   }

   void
   ProtocolSimpleChannelProxy::msgReceived(::MsgContainer *msg, DSite* u){
     GlobalThread *th = gf_popThreadIdVal(msg, a_proxy->m_getEnvironment());
     ::PstInContainerInterface* load = gf_popPstIn(msg);
     th->resumeRemoteDone(load); 
   }


   OpRetVal
   ProtocolSimpleChannelProxy::protocol_Synch(GlobalThread* const th_id, ::PstOutContainerInterface**& msg,
					      const AbsOp& aop){
     // Check if we it is a home proxy, in such cases the 
     // operation can be performed without sending messages. 
     if (stateHolder){
       msg = NULL;
       return DSS_PROCEED;
     }
     ::MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
     msgC->pushIntVal(aop);
     msgC->pushIntVal(SYNCH);
     msg = gf_pushUnboundPstOut(msgC);
     gf_pushThreadIdVal(msgC,th_id);
     if(a_proxy->m_sendToCoordinator(msgC) == false)
       {
	 msg = NULL; 
	 return (DSS_RAISE);
       }
     return (DSS_SUSPEND);
   }

   OpRetVal
   ProtocolSimpleChannelProxy::protocol_Asynch(::PstOutContainerInterface**& msg,
					       const AbsOp& aop){
     if (stateHolder){
       msg = NULL;
       return DSS_PROCEED;
     }
     ::MsgContainer *msgC = a_proxy->m_createCoordProtMsg();
     msgC->pushIntVal(aop);
     msgC->pushIntVal(ASYNCH);
     // Assert(0); 
     // Being removed during restructring
     msg = gf_pushUnboundPstOut(msgC);
     if (a_proxy->m_sendToCoordinator(msgC) == false)
       {
	 msg = NULL; 
	 return DSS_RAISE;
       }
     return (DSS_SKIP);
   }



   ProtocolSimpleChannelProxy::ProtocolSimpleChannelProxy():ProtocolProxy(PN_SIMPLE_CHANNEL),stateHolder(true){}

  void ProtocolSimpleChannelManager::sendMigrateInfo(::MsgContainer* msg){
    gf_pushPstOut(msg, manager->retrieveEntityState());
    static_cast<ProtocolSimpleChannelProxy*>(manager->m_getProxy()->m_getProtocol())->stateHolder = false; 
  }

  bool ProtocolSimpleChannelProxy::m_initRemoteProt(DssReadBuffer*){
    stateHolder = false; // change to non-stateholder
    return true;
  }
  
  void ProtocolSimpleChannelProxy::localInitatedOperationCompleted(){ ; }

} //end namespace
