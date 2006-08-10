/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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

using namespace _dss_internal;



/************************* AbstractEntity *************************/

AbstractEntity::AbstractEntity() : a_proxy(NULL) {}

AbstractEntity::~AbstractEntity() {
  if (a_proxy) delete static_cast<Proxy*>(a_proxy);
}

void AbstractEntity::setCoordinatorAssistant(CoordinatorAssistant* p) {
  if (a_proxy) delete static_cast<Proxy*>(a_proxy);
  a_proxy = p;
  if (a_proxy) static_cast<Proxy*>(a_proxy)->setAbstractEntity(this);
}

void
AbstractEntity::remoteInitatedOperationCompleted(DssOperationId* opId,
						 PstOutContainerInterface* pstOut){
  Assert(a_proxy);
  ProtocolProxy* pp = static_cast<Proxy*>(a_proxy)->m_getProtocol();
  pp->remoteInitatedOperationCompleted(opId, pstOut);
}

void AbstractEntity::localInitatedOperationCompleted() {
  Assert(a_proxy);
  ProtocolProxy* pp = static_cast<Proxy*>(a_proxy)->m_getProtocol();
  pp->localInitatedOperationCompleted();
}

OpRetVal AbstractEntity::abstractOperation_Kill() {
  if (!a_proxy) return DSS_INTERNAL_ERROR_NO_PROXY;
  ProtocolProxy* pp = static_cast<Proxy*>(a_proxy)->m_getProtocol();
  switch (pp->getProtocolName()) {
  case PN_SIMPLE_CHANNEL:
    return static_cast<ProtocolSimpleChannelProxy*>(pp)->protocol_Kill();
  case PN_MIGRATORY_STATE:
    return static_cast<ProtocolMigratoryProxy*>(pp)->protocol_Kill();
  case PN_TRANSIENT:
    return static_cast<ProtocolOnceOnlyProxy*>(pp)->protocol_Kill();
  case PN_TRANSIENT_REMOTE:
    return static_cast<ProtocolTransientRemoteProxy*>(pp)->protocol_Kill();
  case PN_EAGER_INVALID:
    return static_cast<ProtocolEagerInvalidProxy*>(pp)->protocol_Kill();
  case PN_PILGRIM_STATE:
    return static_cast<ProtocolPilgrimProxy*>(pp)->protocol_Kill();
  case PN_LAZY_INVALID:
    return static_cast<ProtocolLazyInvalidProxy*>(pp)->protocol_Kill();
  case PN_IMMUTABLE_LAZY:
    return static_cast<ProtocolImmutableLazyProxy*>(pp)->protocol_Kill();
  case PN_IMMUTABLE_EAGER:
    return static_cast<ProtocolImmutableEagerProxy*>(pp)->protocol_Kill();
  default: 
    Assert(0); 
  }
  return DSS_INTERNAL_ERROR_SEVERE;
}

OpRetVal AbstractEntity::abstractOperation_Monitor() {
  if (!a_proxy) return DSS_INTERNAL_ERROR_NO_PROXY;
  ProtocolProxy* pp = static_cast<Proxy*>(a_proxy)->m_getProtocol();
  switch (pp->getProtocolName()) {
  case PN_SIMPLE_CHANNEL:
    return static_cast<ProtocolSimpleChannelProxy*>(pp)->protocol_Register();
  case PN_MIGRATORY_STATE:
    return static_cast<ProtocolMigratoryProxy*>(pp)->protocol_Register();
  case PN_TRANSIENT:
    return static_cast<ProtocolOnceOnlyProxy*>(pp)->protocol_Register();
  case PN_TRANSIENT_REMOTE:
    return static_cast<ProtocolTransientRemoteProxy*>(pp)->protocol_Register();
  case PN_EAGER_INVALID:
    return static_cast<ProtocolEagerInvalidProxy*>(pp)->protocol_Register();
  case PN_PILGRIM_STATE:
    return static_cast<ProtocolPilgrimProxy*>(pp)->protocol_Register();
  case PN_LAZY_INVALID:
    return static_cast<ProtocolLazyInvalidProxy*>(pp)->protocol_Register();
  case PN_IMMUTABLE_LAZY:
    return static_cast<ProtocolImmutableLazyProxy*>(pp)->protocol_Register();
  case PN_IMMUTABLE_EAGER:
    return static_cast<ProtocolImmutableEagerProxy*>(pp)->protocol_Register();
  default: 
    Assert(0); 
  }
  return DSS_INTERNAL_ERROR_SEVERE;
}



/******************** MutableAbstractEntity ********************/

MutableAbstractEntity::MutableAbstractEntity() {}

OpRetVal 
MutableAbstractEntity::abstractOperation_Read(DssThreadId *id,
					      PstOutContainerInterface**& out)
{
  if (!a_proxy) return DSS_INTERNAL_ERROR_NO_PROXY;
  ProtocolProxy* pp = static_cast<Proxy*>(a_proxy)->m_getProtocol();
  GlobalThread* gid = static_cast<GlobalThread*>(id);
  switch (pp->getProtocolName()) {
  case PN_SIMPLE_CHANNEL:
    return static_cast<ProtocolSimpleChannelProxy*>(pp)->protocol_Synch(gid,out,AO_STATE_READ);
  case PN_MIGRATORY_STATE:
    return static_cast<ProtocolMigratoryProxy*>(pp)->protocol_Access(gid,out);
  case PN_EAGER_INVALID:
    return static_cast<ProtocolEagerInvalidProxy*>(pp)->protocol_Read(gid,out);
  case PN_LAZY_INVALID:
    return static_cast<ProtocolLazyInvalidProxy*>(pp)->protocol_Read(gid,out);
  case PN_PILGRIM_STATE:
    return static_cast<ProtocolPilgrimProxy*>(pp)->protocol_Access(gid,out);
  default: 
    Assert(0);
  }
  return DSS_INTERNAL_ERROR_SEVERE;
}

OpRetVal
MutableAbstractEntity::abstractOperation_Write(DssThreadId *id,
					       PstOutContainerInterface**& out)
{
  if (!a_proxy) return DSS_INTERNAL_ERROR_NO_PROXY;
  ProtocolProxy* pp = static_cast<Proxy*>(a_proxy)->m_getProtocol();
  GlobalThread* gid = static_cast<GlobalThread*>(id);
  switch (pp->getProtocolName()) {
  case PN_SIMPLE_CHANNEL:
    return static_cast<ProtocolSimpleChannelProxy*>(pp)->protocol_Synch(gid,out,AO_STATE_WRITE);
  case PN_MIGRATORY_STATE:
    return static_cast<ProtocolMigratoryProxy*>(pp)->protocol_Access(gid,out);
  case PN_EAGER_INVALID:
    return static_cast<ProtocolEagerInvalidProxy*>(pp)->protocol_Write(gid,out);
  case PN_LAZY_INVALID:
    return static_cast<ProtocolLazyInvalidProxy*>(pp)->protocol_Write(gid,out);
  case PN_PILGRIM_STATE:
    return static_cast<ProtocolPilgrimProxy*>(pp)->protocol_Access(gid,out);
  default: 
    Assert(0);
  }
  return DSS_INTERNAL_ERROR_SEVERE;
}



/******************** RelaxedMutableAbstractEntity ********************/

RelaxedMutableAbstractEntity::RelaxedMutableAbstractEntity() {}

OpRetVal 
RelaxedMutableAbstractEntity::abstractOperation_Read(DssThreadId *id,
						     PstOutContainerInterface**& out)
{
  if (!a_proxy) return DSS_INTERNAL_ERROR_NO_PROXY;
  ProtocolProxy* pp = static_cast<Proxy*>(a_proxy)->m_getProtocol();
  GlobalThread* gid = static_cast<GlobalThread*>(id); 
  switch (pp->getProtocolName()) {
  case PN_SIMPLE_CHANNEL:
    return static_cast<ProtocolSimpleChannelProxy*>(pp)->protocol_Synch(gid,out,AO_STATE_READ);
  default:
    Assert(0);
  }
  return DSS_INTERNAL_ERROR_SEVERE;
}

OpRetVal
RelaxedMutableAbstractEntity::abstractOperation_Write(PstOutContainerInterface**& out){
  if (!a_proxy) return DSS_INTERNAL_ERROR_NO_PROXY;
  ProtocolProxy* pp = static_cast<Proxy*>(a_proxy)->m_getProtocol();
  switch (pp->getProtocolName()) {
  case PN_SIMPLE_CHANNEL:
    return static_cast<ProtocolSimpleChannelProxy*>(pp)->protocol_Asynch(out,AO_STATE_WRITE);
  default:
    Assert(0);
  }
  return DSS_INTERNAL_ERROR_SEVERE;
}



/******************** MonotonicAbstractEntity ********************/

MonotonicAbstractEntity::MonotonicAbstractEntity() {}

OpRetVal
MonotonicAbstractEntity::abstractOperation_Bind(DssThreadId *id,
						PstOutContainerInterface**& out)
{
  if (!a_proxy) return DSS_INTERNAL_ERROR_NO_PROXY;
  ProtocolProxy* pp = static_cast<Proxy*>(a_proxy)->m_getProtocol();
  GlobalThread* gid = static_cast<GlobalThread*>(id); 
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
MonotonicAbstractEntity::abstractOperation_Append(DssThreadId *id,
						  PstOutContainerInterface**& out)
{
  if (!a_proxy) return DSS_INTERNAL_ERROR_NO_PROXY;
  ProtocolProxy* pp = static_cast<Proxy*>(a_proxy)->m_getProtocol();
  GlobalThread* gid = static_cast<GlobalThread*>(id); 
  switch (pp->getProtocolName()) {
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



/******************** ImmutableAbstractEntity ********************/

ImmutableAbstractEntity::ImmutableAbstractEntity() {}
  
OpRetVal
ImmutableAbstractEntity::abstractOperation_Read(DssThreadId *id,
						PstOutContainerInterface**& out)
{
  if (!a_proxy) return DSS_INTERNAL_ERROR_NO_PROXY;
  ProtocolProxy* pp = static_cast<Proxy*>(a_proxy)->m_getProtocol();
  GlobalThread* gid = static_cast<GlobalThread*>(id); 
  switch (pp->getProtocolName()) {
  case PN_SIMPLE_CHANNEL:
    return static_cast<ProtocolSimpleChannelProxy*>(pp)->protocol_Synch(gid,out,AO_STATE_READ);
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
}



namespace _dss_internal{ //Start namespace

  /******************** applyAbstractOperation ********************/

  AOcallback applyAbstractOperation(AbstractEntity* ae, const AbsOp& aop,
				    DssThreadId* tid, DssOperationId* oid,
				    PstInContainerInterface* pstin,
				    PstOutContainerInterface*& pstout) {
    pstout = NULL;
    switch (ae->getAEName()) {
    case AEN_MUTABLE: {
      MutableAbstractEntity* mae = dynamic_cast<MutableAbstractEntity*>(ae);
      switch (aop) {
      case AO_STATE_WRITE: return mae->callback_Write(tid, oid, pstin, pstout);
      case AO_STATE_READ:  return mae->callback_Read(tid, oid, pstin, pstout);
      default: Assert(0); return AOCB_FINISH;
      }
    }
    case AEN_RELAXED_MUTABLE: {
      RelaxedMutableAbstractEntity* rmae =
	dynamic_cast<RelaxedMutableAbstractEntity*>(ae);
      switch (aop) {
      case AO_STATE_WRITE: return rmae->callback_Write(tid, oid, pstin);
      case AO_STATE_READ:  return rmae->callback_Read(tid, oid, pstin, pstout);
      default: Assert(0); return AOCB_FINISH;
      }
    }
    case AEN_TRANSIENT: {
      MonotonicAbstractEntity* mae =
	dynamic_cast<MonotonicAbstractEntity*>(ae);
      switch (aop) {
      case AO_OO_BIND:    return mae->callback_Bind(oid, pstin);
      case AO_OO_UPDATE:  return mae->callback_Append(oid, pstin);
      case AO_OO_CHANGES: return mae->callback_Changes(oid, pstout);
      default: Assert(0); return AOCB_FINISH;
      }
    }
    case AEN_IMMUTABLE:
    case AEN_IMMUTABLE_UNNAMED: {
      ImmutableAbstractEntity* iae =
	dynamic_cast<ImmutableAbstractEntity*>(ae);
      return iae->callback_Read(tid, oid, pstin, pstout);
    }
    default:
      Assert(0); return AOCB_FINISH;
    }
  }

}
