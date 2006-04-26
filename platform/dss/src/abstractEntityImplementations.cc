/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
 *    Erik Klintskog (erik@sics.se)
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
#pragma implementation "abstractEntityImplementations.hh"
#endif

#include "abstractEntityImplementations.hh"
#include "coordinator.hh"
#include "protocols.hh"
#include "protocol_eagerinvalid.hh"
#include "protocol_lazyinvalid.hh"
#include "protocol_migratory.hh"
#include "protocol_once_only.hh"
#include "protocol_transient_remote.hh"
#include "protocol_pilgrim.hh"
#include "protocol_simple_channel.hh"
#include "protocol_immutable_lazy.hh"
#include "protocol_immutable_eager.hh"
#include "protocol_immediate.hh"
#include "protocol_dksBroadcast.hh"

namespace _dss_internal{ //Start namespace

  
#ifdef DEBUG_CHECK
  int AE_ProxyCallbackInterface::a_allocated=0;
#endif

  
  // *************************** AE_ProxyCallbackInterface ****************************'
  
  AE_ProxyCallbackInterface::AE_ProxyCallbackInterface():
    a_coordinationProxy(NULL){
    DebugCode(a_allocated++);
  }
  
  AE_ProxyCallbackInterface::~AE_ProxyCallbackInterface(){
    DebugCode(a_allocated--);
    delete a_coordinationProxy;
  }
  
  
  void AE_ProxyCallbackInterface::setCoordinationProxy(Proxy *pr)
  {
    a_coordinationProxy = pr; 
  }

  
  // **************************** MutableAbstractEntityImpl *****************************
  
  MutableAbstractEntityImpl::MutableAbstractEntityImpl()
  {
    assignMediator(NULL); 
  }

  
  OpRetVal 
  MutableAbstractEntityImpl::abstractOperation_Read(DssThreadId *id,
						    PstOutContainerInterface**& out)
  {
    ProtocolProxy* pp = a_coordinationProxy->m_getProtocol();
    GlobalThread *gid = static_cast<GlobalThread*>(id);
    switch(pp->getProtocolName()){
    case PN_MIGRATORY_STATE: return static_cast<ProtocolMigratoryProxy*>(pp)->protocol_Access(gid,out);
    case PN_SIMPLE_CHANNEL:  return static_cast<ProtocolSimpleChannelProxy*>(pp)->protocol_Synch(gid,out,AO_STATE_READ);
    case PN_EAGER_INVALID:   return static_cast<ProtocolEagerInvalidProxy*>(pp)->protocol_Read(gid,out);
    case PN_LAZY_INVALID:    return static_cast<ProtocolLazyInvalidProxy*>(pp)->protocol_Read(gid,out);
    case PN_PILGRIM_STATE:   return static_cast<ProtocolPilgrimProxy*>(pp)->protocol_Access(gid,out);
    default: 
      Assert(0); 
    }
    return DSS_INTERNAL_ERROR_SEVERE;
  }

  OpRetVal
  MutableAbstractEntityImpl::abstractOperation_Write(DssThreadId *id,
						     PstOutContainerInterface**& out)
  {
    ProtocolProxy* pp = a_coordinationProxy->m_getProtocol();
    GlobalThread *gid = static_cast<GlobalThread*>(id);
    switch(pp->getProtocolName()){
    case PN_MIGRATORY_STATE: return static_cast<ProtocolMigratoryProxy*>(pp)->protocol_Access(gid,out);
    case PN_SIMPLE_CHANNEL:  return static_cast<ProtocolSimpleChannelProxy*>(pp)->protocol_Synch(gid,out,AO_STATE_WRITE);
    case PN_EAGER_INVALID:   return static_cast<ProtocolEagerInvalidProxy*>(pp)->protocol_Write(gid,out);
    case PN_LAZY_INVALID:    return static_cast<ProtocolLazyInvalidProxy*>(pp)->protocol_Write(gid,out);
    case PN_PILGRIM_STATE:   return static_cast<ProtocolPilgrimProxy*>(pp)->protocol_Access(gid,out);
    default: 
      Assert(0); 
    }
    return DSS_INTERNAL_ERROR_SEVERE;
  }

  void
  MutableAbstractEntityImpl::reportFaultState(const FaultState& fs){
    if (a_mediator) a_mediator->reportFaultState(fs);
  }

  PstOutContainerInterface* 
  MutableAbstractEntityImpl::retrieveEntityState(){
    MediatorInterface *mi = this->accessMediator();
    return mi->retrieveEntityRepresentation();
  }

  void 
  MutableAbstractEntityImpl::installEntityState(::PstInContainerInterface* builder){
    MediatorInterface *mi = this->accessMediator();
    mi->installEntityRepresentation(builder); 
  }

  
  AbstractEntity *MutableAbstractEntityImpl::m_getAEreference(){
    return this;
  }


  
  AOcallback 
  MutableAbstractEntityImpl::applyAbstractOperation(const AbsOp& aop,
						    DssThreadId* thid,
						    DssOperationId *opid,
						    PstInContainerInterface* builder , 
						    PstOutContainerInterface*& ans)
  {
    MediatorInterface *mi = this->accessMediator();
    MutableMediatorInterface *mmi = static_cast<MutableMediatorInterface*>(mi);  
    if (aop == AO_STATE_WRITE)
      return mmi->callback_Write(thid,opid,builder,ans); 
    else
      return mmi->callback_Read(thid,opid,builder,ans); 
  }

  AbstractEntityName
  MutableAbstractEntityImpl::m_getName()
  {
    return AEN_MUTABLE; 
  }

  CoordinatorAssistantInterface *
  MutableAbstractEntityImpl::getCoordinatorAssistant(void) const
  {
    
    return  static_cast<CoordinatorAssistantInterface*>(a_coordinationProxy);
  }
  
  
  
  
  // ******************************** TransientAbstractEntityImpl **************************'


  PstOutContainerInterface* 
  MonotonicAbstractEntityImpl::retrieveEntityState(){
    MediatorInterface *mi = this->accessMediator();
    return mi->retrieveEntityRepresentation(); 
  }

  void 
  MonotonicAbstractEntityImpl::installEntityState(::PstInContainerInterface* builder){
    MediatorInterface *mi = this->accessMediator();
    mi->installEntityRepresentation(builder); 
  }


  
  MonotonicAbstractEntityImpl::MonotonicAbstractEntityImpl()
  {
    assignMediator(NULL); 
  }

  AbstractEntity *MonotonicAbstractEntityImpl::m_getAEreference(){
    return this;
  }
  

  OpRetVal
  MonotonicAbstractEntityImpl::abstractOperation_Bind(DssThreadId *id,
						      PstOutContainerInterface**& out)
  {
    ProtocolProxy* pp = a_coordinationProxy->m_getProtocol();
    GlobalThread *gid = static_cast<GlobalThread*>(id); 
    switch (pp->getProtocolName()) {
    case PN_TRANSIENT:
      return static_cast<ProtocolOnceOnlyProxy*>(pp)->protocol_Terminate(gid,out,AO_OO_BIND);
    case PN_TRANSIENT_REMOTE:
      return static_cast<ProtocolTransientRemoteProxy*>(pp)->protocol_Terminate(gid,out,AO_OO_BIND);
    default:
      Assert(0);
    }
    return DSS_INTERNAL_ERROR_SEVERE;
  }

  
  OpRetVal 
  MonotonicAbstractEntityImpl::abstractOperation_Append(DssThreadId *id,
							PstOutContainerInterface**& out)
  {
    ProtocolProxy* pp = a_coordinationProxy->m_getProtocol();
    GlobalThread *gid = static_cast<GlobalThread*>(id); 
    switch(pp->getProtocolName()){
    case PN_TRANSIENT:
      return static_cast<ProtocolOnceOnlyProxy*>(pp)->protocol_Update(gid,out,AO_OO_UPDATE); 
    case PN_TRANSIENT_REMOTE:
      return static_cast<ProtocolTransientRemoteProxy*>(pp)->protocol_Update(gid,out,AO_OO_UPDATE); 
    case PN_DKSBROADCAST:
      return static_cast<ProtocolDksBcProxy*>(pp)-> m_broadCast(out,AO_OO_UPDATE); 
    default:
      Assert(0); 
    }
    return DSS_INTERNAL_ERROR_SEVERE;
  }

  AOcallback 
  MonotonicAbstractEntityImpl::applyAbstractOperation(const AbsOp& aop,
						      DssThreadId* thid,
						      DssOperationId *opid,
						      PstInContainerInterface* builder, 
						      PstOutContainerInterface*& ans){
    MediatorInterface *mi = this->accessMediator();
    MonotonicMediatorInterface *mmi = static_cast<MonotonicMediatorInterface*>(mi);  
    ans = NULL; 
    switch(aop){
    case AO_OO_BIND:
      return mmi->callback_Bind(opid,builder);
    case AO_OO_UPDATE:
      return mmi->callback_Append(opid,builder);
    case AO_OO_CHANGES:
      return mmi->callback_Changes(opid, ans);
    default:
      Assert(0);
    }
    return AOCB_FINISH;
  }
    

  
  void 
  MonotonicAbstractEntityImpl::reportFaultState(const FaultState& fs) {
    if (a_mediator) a_mediator->reportFaultState(fs);
  }

  CoordinatorAssistantInterface *
  MonotonicAbstractEntityImpl::getCoordinatorAssistant(void) const
  {
    return  static_cast<CoordinatorAssistantInterface*>(a_coordinationProxy);
  }
  
  AbstractEntityName
  MonotonicAbstractEntityImpl::m_getName()
  {
    return AEN_TRANSIENT; 
  }
  

  // *********************************   ImmutableAbstractEntityImpl **************************

  PstOutContainerInterface* 
  ImmutableAbstractEntityImpl::retrieveEntityState(){
    MediatorInterface *mi = this->accessMediator();
    return mi->retrieveEntityRepresentation();
  }

  void 
  ImmutableAbstractEntityImpl::installEntityState(::PstInContainerInterface* builder){
    MediatorInterface *mi = this->accessMediator();
    mi->installEntityRepresentation(builder); 
  }
  

  AbstractEntityName
  ImmutableAbstractEntityImpl::m_getName()
  {
    
    return AEN_IMMUTABLE; 
  }

  
  ImmutableAbstractEntityImpl::ImmutableAbstractEntityImpl()
  {
    assignMediator(NULL); 
  }
  
  OpRetVal
  ImmutableAbstractEntityImpl::abstractOperation_Read(DssThreadId *id,
							  PstOutContainerInterface**& pstout)
  {
   ProtocolProxy* pp = a_coordinationProxy->m_getProtocol();
    GlobalThread *gid = static_cast<GlobalThread*>(id);
    switch(pp->getProtocolName()){
    case PN_SIMPLE_CHANNEL:  
      return static_cast<ProtocolSimpleChannelProxy*>(pp)->protocol_Synch(gid,pstout,AO_STATE_READ);
    case PN_IMMUTABLE_LAZY:
      return static_cast<ProtocolImmutableLazyProxy*>(pp)->protocol_Access(gid);
    case PN_IMMUTABLE_EAGER:
      return static_cast<ProtocolImmutableEagerProxy*>(pp)->protocol_Access(gid);
    case PN_IMMEDIATE:
      return static_cast<ProtocolImmediateProxy*>(pp)->protocol_send(gid);
    default: 
      Assert(0); 
    }
    return DSS_INTERNAL_ERROR_SEVERE;

    //  return (DSS_SUSPEND);
  }

  AOcallback
  ImmutableAbstractEntityImpl::applyAbstractOperation(const AbsOp& aop,
						      DssThreadId* thid,
						      DssOperationId *opid,
						      PstInContainerInterface* builder , 
						      PstOutContainerInterface*& ans)
  {
    MediatorInterface *mi = this->accessMediator();
    ImmutableMediatorInterface *mmi = static_cast<ImmutableMediatorInterface*>(mi);  
    return mmi->callback_Read(thid,opid,builder,ans); 
    
    return AOCB_FINISH; 
  }
  
  AbstractEntity *ImmutableAbstractEntityImpl::m_getAEreference(){
    return this;
  }

  void 
  ImmutableAbstractEntityImpl::reportFaultState(const FaultState& fs) {
    if (a_mediator) a_mediator->reportFaultState(fs);
  }
  
  CoordinatorAssistantInterface*
  ImmutableAbstractEntityImpl::getCoordinatorAssistant() const 
  {
    return static_cast<CoordinatorAssistantInterface*>(a_coordinationProxy);
  }
  
  // ********************************* RelaxedMutableAbstractEntityImpl **************************'

  RelaxedMutableAbstractEntityImpl::RelaxedMutableAbstractEntityImpl()
  {
    assignMediator(NULL); 
  }
  
  OpRetVal 
  RelaxedMutableAbstractEntityImpl::abstractOperation_Read(DssThreadId *id,
							   PstOutContainerInterface**& out)
  {
    ProtocolProxy* pp = a_coordinationProxy->m_getProtocol();
    GlobalThread *gid = static_cast<GlobalThread*>(id); 
    switch(pp->getProtocolName()) {
    case PN_SIMPLE_CHANNEL:  return static_cast<ProtocolSimpleChannelProxy*>(pp)->protocol_Synch(gid,out,AO_STATE_READ);
    default:
      Assert(0); 
    }
    return DSS_INTERNAL_ERROR_SEVERE;
  }

  OpRetVal
  RelaxedMutableAbstractEntityImpl::abstractOperation_Write(::PstOutContainerInterface**& out){
    ProtocolProxy* pp = a_coordinationProxy->m_getProtocol();
    switch(pp->getProtocolName()){
    case PN_SIMPLE_CHANNEL:  return static_cast<ProtocolSimpleChannelProxy*>(pp)->protocol_Asynch(out,AO_STATE_WRITE);
    default:
      Assert(0); 
    }
    return DSS_INTERNAL_ERROR_SEVERE;
  }
  
  void
  RelaxedMutableAbstractEntityImpl::reportFaultState(const FaultState& fs){
    if (a_mediator) a_mediator->reportFaultState(fs);
  }
  
  PstOutContainerInterface* 
  RelaxedMutableAbstractEntityImpl::retrieveEntityState(){
    MediatorInterface *mi = this->accessMediator();
    return mi->retrieveEntityRepresentation();
  }

  void 
  RelaxedMutableAbstractEntityImpl::installEntityState(::PstInContainerInterface* builder){
    MediatorInterface *mi = this->accessMediator();
    mi->installEntityRepresentation(builder); 
  }

  CoordinatorAssistantInterface*
  RelaxedMutableAbstractEntityImpl::getCoordinatorAssistant() const
  {
    return  static_cast<CoordinatorAssistantInterface*>(a_coordinationProxy); 
  }
  
  AbstractEntityName 
  RelaxedMutableAbstractEntityImpl::m_getName(){
    return AEN_RELAXED_MUTABLE;
  }
  
  AbstractEntity *
  RelaxedMutableAbstractEntityImpl::m_getAEreference(){
    return this;
  }

  AOcallback
  RelaxedMutableAbstractEntityImpl::applyAbstractOperation(const AbsOp& aop,
							   DssThreadId* thid,
							   DssOperationId *opid,
							   PstInContainerInterface* builder , 
							   PstOutContainerInterface*& ans)
  {
    MediatorInterface *mi = this->accessMediator();
    Assert(dynamic_cast<RelaxedMutableMediatorInterface*>(mi)); 
    RelaxedMutableMediatorInterface *mmi = static_cast<RelaxedMutableMediatorInterface*>(mi);  
    if (aop == AO_STATE_WRITE)
      return mmi->callback_Write(thid,opid,builder); 
    else
      return mmi->callback_Read(thid,opid,builder,ans); 
  }

  void  ImmutableAbstractEntityImpl::remoteInitatedOperationCompleted(DssOperationId* opId,
					PstOutContainerInterface* pstOut){
    ProtocolProxy* pp = a_coordinationProxy->m_getProtocol();
    pp->remoteInitatedOperationCompleted(opId, pstOut);
  }

  void MonotonicAbstractEntityImpl::remoteInitatedOperationCompleted(DssOperationId* opId,
					PstOutContainerInterface* pstOut){
    ProtocolProxy* pp = a_coordinationProxy->m_getProtocol();
    pp->remoteInitatedOperationCompleted(opId, pstOut);
  }

  void  RelaxedMutableAbstractEntityImpl::remoteInitatedOperationCompleted(DssOperationId* opId,
					PstOutContainerInterface* pstOut){
    ProtocolProxy* pp = a_coordinationProxy->m_getProtocol();
    pp->remoteInitatedOperationCompleted(opId, pstOut);
  }

  void  MutableAbstractEntityImpl::remoteInitatedOperationCompleted(DssOperationId* opId,
					PstOutContainerInterface* pstOut){
    ProtocolProxy* pp = a_coordinationProxy->m_getProtocol();
    pp->remoteInitatedOperationCompleted(opId, pstOut);
  }


  void  MutableAbstractEntityImpl::localInitatedOperationCompleted()
  {
    ProtocolProxy* pp = a_coordinationProxy->m_getProtocol();
    pp->localInitatedOperationCompleted();
  }

  
}






