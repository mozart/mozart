/*  
 *  Authors:
 *    Zacharias El Banna, 2002 (zeb@sics.se)
 *    Erik Klintskog, 2003 (erik@sics.se)
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


#ifdef DSS_INTERFACE  
#pragma implementation "dss_object.hh"
#endif

#include "dssBase.hh"

/*************** DSS_INTERFACE PART I: THE ENVIRONMENT ***************/

#include "coordinator.hh"
#include "protocols.hh"

#ifndef WIN32 
#include <unistd.h> // for _exit()
#else
#include <process.h>
#endif
//
//
//

namespace _dss_internal{
  
  // Note: On some compilers there may be a problem with
  // initialization and "this".
  //
  // We should be able to pass primes to the DSS_Environment to reuse
  // identity...  for now I don't see the need though
  DSS_Environment::DSS_Environment(ComServiceInterface *  const sa, 
				   Mediation_Object* const mo,
				   const bool& sec):
    a_map(                      mo),
    a_proxyTable(             NULL),
    a_coordinatorTable(           NULL),
    a_threadTable(            NULL),
    a_myDSite(                NULL),
    a_dssconf(     DssConfigData()),
    a_dssMslClbk(             NULL),
    a_msgnLayer(              NULL),
    a_CreateXistRefCounter(      0),
    a_CreateNonXRefCounter(      0),
    a_DuplicateXistRefCounter(   0),
    a_DuplicateNonXRefCounter(   0),
    a_DuplicateToOwnerRefCounter(0){
    // *************************************************
    dssLog(DLL_BEHAVIOR,"ENVIRONMENT CREATED %p",this);

    // Initializing the msgnlayer
    a_dssMslClbk         = new DssMslClbk(this);
    a_msgnLayer        = new ::MsgnLayer(a_dssMslClbk, sa, sec);
    a_myDSite          = a_msgnLayer->a_myDSite;
    // at this point, the msgng layer is started and runnig. 
    // Note that the DSIte is first initialized here, myDSite.  
    a_proxyTable       = new ProxyTable(a_dssconf.DEFAULT_PROXY_TABLE_SIZE, this);
    a_coordinatorTable     = new CoordinatorTable(a_dssconf.DEFAULT_MANAGER_TABLE_SIZE, this);
    a_threadTable      = new GlobalThreadTable(10, this);
  }

  DSS_Environment::~DSS_Environment(){
    // Network

    // Rest
    delete a_threadTable;
    delete a_coordinatorTable;
    delete a_proxyTable;
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
					    const RCalg& GC_annot)
  {
    ProtocolManager *pman; 
    ProtocolProxy *pprox;
    gf_createProtocolProxyManager(prot, this, pman, pprox); 
  
    Coordinator *m = gf_createCoordinator(aa, pman, GC_annot, this); 
    Proxy *p = gf_createCoordinationProxy(aa, m->m_getNetId(), pprox, this); 
    p->m_initHomeProxy(m);
    return p;
  }


  //
  // Internal routine.  Used by the unmarshaler to create its proxies.
  //  
  // raph: This is a simplification of the former method
  // m_unmarshalProxy(), because abstract entities are no longer
  // created by the DSS itself.  It returns the coordination proxy
  // instead of the abstract entity.
  //
  Proxy*
  DSS_Environment::m_unmarshalProxy(DssReadBuffer* const buf,
				    const ProxyUnmarshalFlag& flag, 
				    AbstractEntityName& ae_name,
				    bool &trailingState)
  {
    if (flag == PUF_ORDINARY && m_getSrcDSite() == NULL) {
      a_map->GL_warning("Called unmarshalProxy without source");
      return NULL;
      // ERIK, exception!!
    }

    // unmarshalInternal - check with marshal (Proxy) that the
    // "protocol" is correct
    int header = gf_Unmarshal8bitInt(buf) << 8 | gf_Unmarshal8bitInt(buf);
    NetIdentity ni = gf_unmarshalNetIdentity(buf,this);

    // the proxy might exist already, look it up
    Proxy* p = a_proxyTable->m_find(ni);

    if (p != NULL) {
      p->m_mergeReferenceInfo(buf);
      trailingState = p->m_getProtocol()->dispose_protocol_info(buf); 
      return p;

    } else {
      // create a proxy
      AbstractEntityName aen =
	static_cast<AbstractEntityName>((header >> PMF_NBITS) & AEN_MASK);
      ProtocolName pn =
	static_cast<ProtocolName>((header >> (PMF_NBITS+AEN_NBITS)) & PN_MASK);
      AccessArchitecture aa =
	static_cast<AccessArchitecture>((header >> (PMF_NBITS+AEN_NBITS+PN_NBITS)) & AA_MASK);

      switch (aen) { // simple check
      case AEN_MUTABLE:
      case AEN_RELAXED_MUTABLE:
      case AEN_TRANSIENT:
      case AEN_IMMUTABLE:
	ae_name = aen;
	break;
      default:
	Assert(0);
	a_map->GL_error("Not a valid abstract entity type %x", aen);
	return NULL;
      }

      ProtocolProxy *prox = gf_createRemoteProxy(pn, a_myDSite);

      // Create proxy, don't forget to init the protocol after creation
      // (perhaps one should "init" the proxy instead...
      p = gf_createCoordinationProxy(aa, ni, prox, this); 
      trailingState = p->m_initRemoteProxy(buf);
      return p;
    }
    Assert(0);
  }


  
  void
  DSS_Environment::m_gcDssResources(){
    // Scans all tables and components and marks up resources
    
    // For resources connected to managers
    // This is:
    // - DSite marking
    // - Managers without homeproxies
    a_coordinatorTable->m_gcResources();
    //For resources connected to proxys (before also to markup the mediator)
    a_proxyTable->m_gcResources();
    
    a_threadTable->m_gcResources();

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
#endif
#ifdef DEBUG_CHECK
      case DSS_STATIC_MEMORY_ALLOCATION:
	printf("******* ALLOCATION STATUS *******\n");
	Assert(0); //printf("DSites:%d ComObj:%d TCPtransObj:%d\n",
	//DSite::a_allocated,
	//     ComObj::a_allocated,
	//     TCPTransObj::a_allocated);
	
	printf("Prots:[M:%d | P:%d] Coord:[M:%d | P:%d]\n",
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


  DssThreadId*
  DSS_Environment::m_createDssThreadId(){
    return a_threadTable->createDistThread(); 
  }


  bool
  DSS_Environment::m_orderEntities(AbstractEntity* const ae1,
				   AbstractEntity* const ae2){
    // use the order defined on NetIds
    Proxy* p1 = static_cast<Proxy*>(ae1->getCoordinatorAssistant());
    Proxy* p2 = static_cast<Proxy*>(ae2->getCoordinatorAssistant());
    return p1->m_getNetId() < p2->m_getNetId();
  }
}





/*************** DSS_INTERFACE PART II: THE INTERFACE ***************/

using namespace _dss_internal;

#include "dss_object.hh"


DSS_Object::DSS_Object(ComServiceInterface *  const sa, 
		       Mediation_Object* const mo,
		       const bool& sec):
  _a_env(new DSS_Environment(sa, mo, sec)){
  ;
}


DSS_Object::~DSS_Object(){
  delete _a_env;
}


CoordinatorAssistant*
DSS_Object::createProxy(const ProtocolName& prot,
			const AccessArchitecture& aa,
			const RCalg& GC_annot)
{
  return _a_env->m_initializeCoordination(prot, aa, GC_annot);
}

CoordinatorAssistant*
DSS_Object::unmarshalProxy(DssReadBuffer* const buf, 
			   const ProxyUnmarshalFlag& flag,
			   AbstractEntityName &aen,
			   bool &trailingState)
{
  return _a_env->m_unmarshalProxy(buf, flag, aen, trailingState); 
}


void DSS_Object::gcDssResources(){ _a_env->m_gcDssResources(); }


ParamRetVal DSS_Object::operateIntParam(const DSS_AREA& area, const DSS_AREA_ID& id, const int& param, int& arg){
  return _a_env->m_operateIntParam(area,id,param,arg);
}

ParamRetVal DSS_Object::operateStrParam(const DSS_AREA& area, const DSS_AREA_ID& id, const int& param, const char* const str){
  return _a_env->m_operateStrParam(area,id,param,str);
}

  
DssThreadId* DSS_Object::m_createDssThreadId() {
  return _a_env->m_createDssThreadId();
}

bool DSS_Object::m_orderEntities(AbstractEntity* const ae_first,
				 AbstractEntity* const ae_second)
{
  return _a_env->m_orderEntities(ae_first,ae_second);
}
