/*  
 *  Authors:
 *    Zacharias El Banna, 2002 (zeb@sics.se)
 *    Erik Klintskog, 2003 (erik@sics.se)
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


#ifdef INTERFACE  
#pragma implementation "dss_object.hh"
#endif

#include "dssBase.hh"


// ********************* DSS_INTERFACE PART I: THE ENVIRONMENT *********************
//
//
#include "msl_serialize.hh"
#include "coordinator.hh"
#include "dss_global_name.hh"
#include "protocols.hh"
#include "dss_psDKS.hh"


#include "dss_dksBackbone.hh"

#ifndef WIN32 
#include <unistd.h> // for _exit()
#else
#include <process.h>
#endif
//
//
//

namespace _dss_internal{



  
  // Note: On some compilers there may be a problem with initialization and "this"
  //
  // We should be able to pass primes to the DSS_Environment to reuse identity...
  // for now I don't see the need though
  DSS_Environment::DSS_Environment(IoFactoryInterface * const io, 
				   ComServiceInterface *  const sa, 
				   Mediation_Object* const mo,
				   const bool& sec):
    a_map(                      mo),
    a_dksInstHT(              NULL),
    a_proxyTable(             NULL),
    a_coordinatorTable(           NULL),
    a_threadTable(            NULL),
    a_nameTable(              NULL),
    a_myDSite(                NULL),
    a_dssconf(     DssConfigData()),
    a_dssMslClbk(             NULL),
    a_msgnLayer(              NULL),
    a_dksBackbone(         NULL),
    a_CreateXistRefCounter(      0),
    a_CreateNonXRefCounter(      0),
    a_DuplicateXistRefCounter(   0),
    a_DuplicateNonXRefCounter(   0),
    a_DuplicateToOwnerRefCounter(0){
    // *************************************************
    dssLog(DLL_BEHAVIOR,"ENVIRONMENT CREATED %p",this);

    // Initializing the msgnlayer
    a_dssMslClbk         = new DssMslClbk(this);
    a_msgnLayer        = new ::MsgnLayer(a_dssMslClbk, sa, io, sec);
    a_myDSite          = a_msgnLayer->a_myDSite;
    // at this point, the msgng layer is started and runnig. 
    // Note that the DSIte is first initialized here, myDSite.  
    a_dksInstHT        = new DksInstanceHT(10, this); 
    a_proxyTable       = new ProxyTable(a_dssconf.DEFAULT_PROXY_TABLE_SIZE, this);
    a_coordinatorTable     = new CoordinatorTable(a_dssconf.DEFAULT_MANAGER_TABLE_SIZE, this);
    a_threadTable      = new GlobalThreadTable(10, this);
    a_nameTable        = new GlobalNameTable(a_dssconf.DEFAULT_NAME_TABLE_SIZE, this);
  }

  DSS_Environment::~DSS_Environment(){
    // Network

    // Rest
    delete a_threadTable;
    delete a_coordinatorTable;
    delete a_proxyTable;
    delete a_nameTable;
  }
  DSite*
  DSS_Environment::m_getDestDSite(){
    return a_msgnLayer->m_getDestDSite();
  }

  DSite*
  DSS_Environment::m_getSrcDSite(){
    return a_msgnLayer->m_getSourceDSite();
  }
    
  Proxy*
  DSS_Environment::m_initializeCoordination(const ProtocolName& prot,
					    const AccessArchitecture& aa, 
					    const RCalg& GC_annot,
					    AE_ProxyCallbackInterface *ae){
    
    
    ProtocolManager *pman; 
    ProtocolProxy *pprox;
    gf_createProtocolProxyManager(prot, this, pman, pprox); 
  
    Coordinator *m = gf_createCoordinator(aa, pman, GC_annot, this); 
    Proxy   *p = gf_createCoordinationProxy(aa, m->m_getNetId(), pprox, ae, this); 
    p->m_initHomeProxy(m);
    return p;
  }


  //
  //   Internal routin. Used by the unmarshaler to create its proxies. 
  //  
 
  
  DSS_unmarshal_status
  DSS_Environment::m_unmarshalProxy(AbstractEntity* &ae_instance, 
				    DssReadBuffer* const buf,
				    const ProxyUnmarshalFlag& flag, 
				    AbstractEntityName& ae_name){
    DSS_unmarshal_status ret;
    bool skel;
    if (flag == PUF_ORDINARY && m_getSrcDSite() == NULL){
      a_map->GL_warning("Called unmarshalProxy without source");
      ae_instance = NULL;
      ret.exist = false;
      return ret;
      // ERIK, exception!!
    }
    // unmarshalInternal - check with marshal (Proxy) that the "protocol" is correct
    bool exist;
    unsigned int head = gf_UnmarshalNumber(buf);
    
    NetIdentity ni = gf_unmarshalNetIdentity(buf,this); 
    
    // Casting problems. AbstractEntity is the mother and the aepc is
    // the father of an ae implementation... do you get it? The Proxy
    // needs the aepc, so we have to fig it out when we initialize the
    // implementations, since the inoprmation is lost in the
    // abstraecEntity representation.
    
    AE_ProxyCallbackInterface *aepc_interface; 
    
    Proxy* pe = a_proxyTable->m_find(ni);
    if(exist = (pe != NULL)) { // fixed so that exists actually is set right      
      //Will return bool! if false: the whole structure is sent./P
      pe->m_mergeReferenceInfo(buf);
      skel = pe->m_getProtocol()->dispose_protocol_info(buf); 
      AE_ProxyCallbackInterface* aePCI = pe->getAEpki();
      ae_instance = aePCI->m_getAEreference();
      ret.skel = skel;
      ret.exist = true;
      return ret;
    } else {
      switch((head >> 4) & 0xF){
      case AE_MUTABLE:      
      {
        MutableAbstractEntityImpl *MAE = new MutableAbstractEntityImpl();
        aepc_interface = MAE; 
        ae_instance = MAE; 
        ae_name = AE_MUTABLE; 
        break; 
      }
      case AE_RELAXED_MUTABLE: 
	{
	  RelaxedMutableAbstractEntityImpl *MAE = new RelaxedMutableAbstractEntityImpl();
	  ae_instance =  MAE;
	  aepc_interface = MAE; 
	  ae_name = AE_RELAXED_MUTABLE; 
	  break; 
	}
      case AE_TRANSIENT:      
	{
	  MonotonicAbstractEntityImpl *MAE = new MonotonicAbstractEntityImpl();
	  ae_instance =  MAE;
	  aepc_interface = MAE; 
	  ae_name = AE_TRANSIENT;  
	  break; 
	}
      case AE_IMMUTABLE:        
	{
	  ImmutableAbstractEntityImpl * MAE = new ImmutableAbstractEntityImpl();
	  ae_instance =  MAE;
	  aepc_interface = MAE; 
	  ae_name = AE_IMMUTABLE;  
	  break; 
	}
       case AE_IMMUTABLE_UNNAMED: 
 	{
 	  ImmutableAbstractEntityImpl *MAE = new ImmutableAbstractEntityImpl();
 	  ae_instance =  MAE;
 	  aepc_interface = MAE; 
 	  ae_name = AE_IMMUTABLE_UNNAMED;  
 	  break; 
 	}
      default:
	{
	  aepc_interface = NULL;
	  ae_instance = NULL;
	  Assert(0);
	  a_map->GL_error("Not a valid abstract entity type %x",((head >> 4) & 0xF));
	}
      }
      
      ProtocolProxy *prox = gf_createRemoteProxy((head >> 8) & 0xF, a_myDSite);
      
      // Create proxy, don't forget to init the protocol after creation
      // (perhaps one should "init" the proxy instead...
      
      pe = gf_createCoordinationProxy(((head << 4) & 0xF0000), ni, prox, aepc_interface, this); 
      
      skel = pe->m_initRemoteProxy(buf);
      aepc_interface->setCoordinationProxy(pe); 
    }
    
    
    ret.exist = exist;
    ret.skel = skel;
    
    return ret;
  }


  
  void
  DSS_Environment::m_gcDssResources(){
    printf("***************************** => GC\n"); 
    
    // Scans all tables and components and marks up resources
    
    // For resources connected to managers
    // This is:
    // - DSite marking
    // - Managers without homeproxies
    a_coordinatorTable->m_gcResources();
    //For resources connected to proxys (before also to markup the mediator)
    a_proxyTable->m_gcResources();
    
    a_dksInstHT->m_gcResources();
    
    a_msgnLayer->m_gcResources();
  }


  ParamRetVal
  DSS_Environment::m_operateIntParam(const DSS_AREA& area, const DSS_AREA_ID& id, const int& param, int& arg){
    switch(area){
    case DSS_STATIC:{
      switch(id){
      case DSS_STATIC_GET_COMINFO:
	{
	  switch(param){
	  case 0: 
	    {
	      Assert(0); //	      arg = a_SendCounter;
	      return PRV_OK;
	    }
	  case 1: 
	    {
	      Assert(0); //arg = a_RecCounter;
	      return PRV_OK;
	    }
	  case 2: 
	    {
	      Assert(0); //arg = a_OSWriteCounter;
	      return PRV_OK;
	    }
	  case 3: 
	    {
	      Assert(0); //arg = a_OSReadCounter;
	      return PRV_OK;
	    }
	  case 4: 
	    {
	      Assert(0); //arg = a_ContCounter;
	      return PRV_OK;
	    }
	  }
	}
#ifdef DSS_LOG
      case DSS_STATIC_DEBUG_TABLES:
	a_coordinatorTable->log_print_content();
	a_proxyTable->log_print_content();
	break;
      case DSS_STATIC_LOG_PARAMETER:
	g_dssLogLevel = static_cast<DSS_LOG_LEVEL>(arg);
	dssLog(g_dssLogLevel,"<= New loglevel set");
	break;
      case DSS_STATIC_MEMORY_ALLOCATION:
	printf("******* ALLOCATION STATUS *******\n");
	Assert(0); //printf("DSites:%d ComObj:%d TCPtransObj:%d\n",
	//DSite::a_allocated,
	//     ComObj::a_allocated,
	//     TCPTransObj::a_allocated);
	
	printf("AEs:%d Prots:[M:%d | P:%d] Coord:[M:%d | P:%d]\n",
	       AE_ProxyCallbackInterface::a_allocated,
	       ProtocolManager::a_allocated,
	       ProtocolProxy::a_allocated,
	       Coordinator::a_allocated,
	       Proxy::a_allocated);
	
	printf("HRs:%d RRs:%d GCalgs:%d\n", // Sends:%d Recs:%d\n",
	       HomeReference::a_allocated,
	       RemoteReference::a_allocated,
	       GCalgorithm::a_allocated
	       );
	       //MsgContainer::a_allocated,
	       //MsgContainer::a_allocated);
	
	// printf("Timers:%d Threads:%d Events:%d\n",
// 	       TimerElement::a_allocated,
// 	       GlobalThread::a_allocated,
// 	       Event::a_allocated);
	       break;
#endif
      default: 
	return PRV_STAT_PARAM_NOT_FOUND; 
      }
      break;
    }
    case DSS_AREA_TABLES:
    case DSS_AREA_MESSAGES:
    case DSS_AREA_SITES: break;
    default:
      Assert(0);
      return PRV_AREA_NOT_FOUND;
    }
    return PRV_OK;
  }


  ParamRetVal
  DSS_Environment::m_operateStrParam(const DSS_AREA&, const DSS_AREA_ID&, const int& param, const char* const str){
    return PRV_AREA_NOT_FOUND;
  }


  MutableAbstractEntity*
  DSS_Environment::m_createMutableAbstractEntity(const ProtocolName& prot,
						 const AccessArchitecture& aa,
						 const RCalg& GC_annot){
    MutableAbstractEntityImpl *MAE;
    MAE = new MutableAbstractEntityImpl();
    Proxy *prx = m_initializeCoordination(prot,  aa,  GC_annot, MAE);
    MAE->setCoordinationProxy(prx); 
    return static_cast<MutableAbstractEntity*>(MAE);
  }
  
  
  RelaxedMutableAbstractEntity*
  DSS_Environment::m_createRelaxedMutableAbstractEntity(const ProtocolName& prot,
							const AccessArchitecture& aa,
							const RCalg& GC_annot){
    RelaxedMutableAbstractEntityImpl *MAE;
    MAE = new RelaxedMutableAbstractEntityImpl();
    Proxy *prx = m_initializeCoordination(prot,  aa,  GC_annot, MAE);
    MAE->setCoordinationProxy(prx); 
    return static_cast<RelaxedMutableAbstractEntity*>(MAE);
  }
  
  
  MonotonicAbstractEntity*
  DSS_Environment::m_createMonotonicAbstractEntity(const ProtocolName& prot,
						   const AccessArchitecture& aa,
						   const RCalg& GC_annot){
    MonotonicAbstractEntityImpl *MAE;
    MAE = new MonotonicAbstractEntityImpl();
    Proxy *prx = m_initializeCoordination(prot,  aa,  GC_annot, MAE);
    MAE->setCoordinationProxy(prx); 
    return static_cast<MonotonicAbstractEntity*>(MAE);
  }

  //  Added 03dec05 by Per Sahlin
  ImmutableAbstractEntity*
  DSS_Environment::m_createImmutableAbstractEntity(const ProtocolName& prot,
						   const AccessArchitecture& aa,
						   const RCalg& GC_annot){
    ImmutableAbstractEntityImpl *MAE;
    MAE = new ImmutableAbstractEntityImpl();
    Proxy *prx = m_initializeCoordination(prot,  aa,  GC_annot, MAE);
    MAE->setCoordinationProxy(prx); 
    return static_cast<ImmutableAbstractEntity*>(MAE);
  }


  DssThreadId*
  DSS_Environment::m_createDssThreadId(){
    return a_threadTable->createDistThread(); 
  }


  bool
  DSS_Environment::m_orderEntities(AbstractEntity* const ae_first, AbstractEntity* const ae_second){
  // *** Sort on dsite first and then number ***

  // Get the AS_Node through dynamic casting, this is necessary since
  // there is no straight inheritance line
  AS_Node* asn1 = dynamic_cast<AS_Node*>(ae_first->getCoordinatorAssistant());
  AS_Node* asn2 = dynamic_cast<AS_Node*>(ae_second->getCoordinatorAssistant());
  Assert(asn1);
  Assert(asn2);
  return asn1->m_getNetId() < asn2->m_getNetId();
}

  
  
  KbrInstance*
  DSS_Environment::m_createKbr(int K, int Bits, int Fail, KbrCallbackInterface* inf){
    printf("Clculating the DKS K:%d bits:%d 2^%d = %d\n", K, Bits, Bits, 1 << (Bits)); 
    PS_DKS_userClass *interface = new PS_DKS_userClass( this, inf); 
    DksInstance*      instance  = new DksInstance(1 << (Bits), K, Fail, interface, this); 
    KbrInstanceImpl*      kInst     = new KbrInstanceImpl(instance, interface); 
    interface->m_setKbrInstance(kInst); 
    return kInst;
  }
  
  bool
  DSS_Environment::m_unmarshalKbr(DssReadBuffer* buf, KbrInstance* &inst){
    DksInstance* instance;
    bool exists  =  a_dksInstHT->m_unmarshalDksInstance(buf, instance); 
    if(exists) {
      PS_DKS_userClass *interface  = static_cast<PS_DKS_userClass*>(instance->getCallBackService());
      inst = interface->m_getKbrInstance(); 
      return true; 
    }
    PS_DKS_userClass *interface = new PS_DKS_userClass(this, NULL); 
    instance->setCallBackService(interface); 
    KbrInstanceImpl*      kInst     = new KbrInstanceImpl(instance, interface); 
    interface->m_setKbrInstance(kInst); 
    inst = kInst; 
    return false; 
  }

  void
  DSS_Environment::m_setupBackbone(DssWriteBuffer* buf){
    if(a_dksBackbone == NULL){
      a_dksBackbone = new DksBackbone(this); 
      DksInstance*      instance  = new DksInstance(1 << 16, 2, 1 , a_dksBackbone, this); 
      a_dksBackbone->a_instance = instance; 
    }
    a_dksBackbone->a_instance->m_marshal(buf); 
  }
  
  void 
  DSS_Environment::m_joinBackbone(DssReadBuffer *buf){
    if(a_dksBackbone) return ; 
    DksInstance* instance = NULL;
    Assert(a_dksInstHT->m_unmarshalDksInstance(buf, instance) == false); 
    a_dksBackbone = new DksBackbone(instance, this); 
    instance->setCallBackService(a_dksBackbone);
    instance->m_joinDksRing(); 
  }
}







// ********************* DSS_INTERFACE PART II: THE INTERFACE *********************

using namespace _dss_internal;

#include "dss_object.hh"


DSS_Object::DSS_Object(IoFactoryInterface * const io, 
		       ComServiceInterface *  const sa, 
		       Mediation_Object* const mo,
		       const bool& sec):
  _a_env(new DSS_Environment(io, sa, mo, sec)){
  ;
}


DSS_Object::~DSS_Object(){
  delete _a_env;
}


DSS_unmarshal_status DSS_Object::unmarshalProxy(AbstractEntity* &proxy, 
						DssReadBuffer* const buf, 
						const ProxyUnmarshalFlag& flag,
						AbstractEntityName& cm){
  return _a_env->m_unmarshalProxy(proxy,buf,flag,cm); 
}

void DSS_Object::gcDssResources(){ _a_env->m_gcDssResources(); }


ParamRetVal DSS_Object::operateIntParam(const DSS_AREA& area, const DSS_AREA_ID& id, const int& param, int& arg){
  return _a_env->m_operateIntParam(area,id,param,arg);
}

ParamRetVal DSS_Object::operateStrParam(const DSS_AREA& area, const DSS_AREA_ID& id, const int& param, const char* const str){
  return _a_env->m_operateStrParam(area,id,param,str);
}


MutableAbstractEntity*
DSS_Object::m_createMutableAbstractEntity(const ProtocolName& prot,
					  const AccessArchitecture& aa,
					  const RCalg& GC_annot){
  return _a_env->m_createMutableAbstractEntity(prot, aa, GC_annot); 
}


RelaxedMutableAbstractEntity*
DSS_Object::m_createRelaxedMutableAbstractEntity(const ProtocolName& prot,
						 const AccessArchitecture& aa,
						 const RCalg& GC_annot){
  return _a_env->m_createRelaxedMutableAbstractEntity(prot, aa, GC_annot); 
}


MonotonicAbstractEntity*
DSS_Object::m_createMonotonicAbstractEntity(const ProtocolName& prot, 
					    const AccessArchitecture& aa, 
					    const RCalg& GC_annot){
  return _a_env->m_createMonotonicAbstractEntity(prot, aa, GC_annot);
}

ImmutableAbstractEntity*
DSS_Object::m_createImmutableAbstractEntity(const ProtocolName& prot, 
					    const AccessArchitecture& aa, 
					    const RCalg& GC_annot){
  
  return _a_env->m_createImmutableAbstractEntity(prot, aa, GC_annot); 
}
  
DssThreadId*
DSS_Object::m_createDssThreadId(){
  return _a_env->m_createDssThreadId();
}

bool
DSS_Object::m_orderEntities(AbstractEntity* const ae_first, AbstractEntity* const ae_second){
  return _a_env->m_orderEntities(ae_first,ae_second);
}


// NEW IO interface


GlobalNameInterface* DSS_Object::getName(void* ref){
  return new GlobalName((reinterpret_cast<u32>(_a_env->a_myDSite)),
			_a_env->a_nameTable->getNextId(), 
			ref);
}
  
GlobalNameInterface* DSS_Object::findName(GlobalNameInterface* ni) {
  return _a_env->a_nameTable->m_find(static_cast<GlobalName*>(ni));
}


void DSS_Object::addName(GlobalNameInterface * ni) {
  _a_env->a_nameTable->m_add(static_cast<GlobalName*>(ni));
}

GlobalNameInterface* DSS_Object::unmarshalName(DssReadBuffer* buf){
  unsigned int pk = gf_UnmarshalNumber(buf);
  unsigned int sk = gf_UnmarshalNumber(buf);
  return new GlobalName(pk, sk, NULL);
}



KbrInstance*
DSS_Object::m_createKbr(int K, int Bits, int Fail, KbrCallbackInterface* intf){
  return _a_env->m_createKbr(K, Bits, Fail, intf);
}

bool
DSS_Object::m_unmarshalKbr(DssReadBuffer* buf, KbrInstance*& inst){
  return _a_env->m_unmarshalKbr(buf, inst); 
}

void
DSS_Object::m_createBackboneTicket(DssWriteBuffer* buf){
   _a_env->m_setupBackbone(buf); 
}

void
DSS_Object::m_joinBackbone(DssReadBuffer *buf){
  _a_env->m_joinBackbone(buf); 
}


