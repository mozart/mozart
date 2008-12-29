/*
 *  Authors:
 *    Erik Klintskog(erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
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


#ifndef __DSSBASE_HH
#define __DSSBASE_HH

#ifdef INTERFACE  
#pragma interface
#endif


#include "dss_enums.hh"
#include "dss_classes.hh"
#include "dss_templates.hh"
#include "dss_comService.hh"
#include "base.hh"

namespace _dss_internal{ //Start namespace


  // ***************** NAMING CONVENTIONS FOR THE DSS ********************
  //
  //  Classes:
  //    Attribute = a_
  //    Static    = s_
  //    Methods   = m_
  //    Special Attributes (Fast access to the environment) = e_
  //    - If they are private they should start with _ i.e (_a_, _s_, _m_, _e_) 
  //
  //  Class Definitions:
  //    if the class is purely internal to a specific other class its name
  //    should start with _
  //    This is typical for "Container" classes, holding an pointer or something
  //
  //  Variables:
  //    Global    = g_
  //    - No globally static variables are used anymore since we are OO
  //
  //  Functions: (stand alone)
  //    - These are functions which may be used anywhere in the DSS but the
  //      naming limits their usage
  //    Global    = gf_
  //    Local     = sf_
  //    Local inline = if_
  //
  //
  //
  //  Logging/printing
  //  - an object might implement the stringrep() function
  //  - an object can also implement a log_print() function (typically for debugging)
  //  - an object (typically tables, containers etc.) might implement the log_print_content()
  //
  //  The last two types of printing chould be declared under the DSS_LOG flag i.e:
  // #ifdef DSS_LOG
  //     void log_print(){}
  // #endif
  //


  // ************************ FORWARD DECLARATIONS ************************

  class ProtocolManager;
  class ProtocolProxy;
  class DssReadByteBuffer;
  class DssWriteByteBuffer;
  class Proxy;
  class ProxyTable;
  class CoordinatorTable; 
  class GlobalThreadTable;
  class GlobalThread;
  class DssMslClbk; 
  class DksBackbone; 
  
  class DSS_Environment;
  class DksInstanceHT; 
  class MD5;

  class DssSimpleWriteDct;
  class DssSimpleReadDct;
  class DssSimpleWriteBuffer;
  class DssSimpleReadBuffer;

  // ****************** DEFINES, FLAGS & DEBUG AND UTILITIES **************************
  //
  //  The different compiler flags used in the DSS are defined as
  //  either pure compiler choices, choosing between, for instance,
  //  windows- or linux header files.  The other type is "enablers",
  //  those are used with inlines and macros, optimized away by a good
  //  compiler when not need.
  //
  //  Flags:
  //  - DEBUG_CHECK, enables asserts
  //  - INTERFACE,   enables pragma directives
  //  - DSS_LOG,     enables the logging utility.
  //  - WIN32        chooses between windows environment and unix environment (header files)

  
  class DSS_Environment_Base{
    DSS_Environment* const a_environment;
  protected:
    DSS_Environment_Base(DSS_Environment* env):a_environment(env){};
  public:
    DSS_Environment* m_getEnvironment() const { return a_environment; }

    MACRO_NO_DEFAULT_CONSTRUCTORS(DSS_Environment_Base);
  };


  // ********************** Scheduled Events to be run ASAP ****************************
  // 

    // ******************************* ENUMS AND CONSTANTS ****************************

  enum MonitorReturn{
    MONITOR_OK,
    SIZE_THRESHOLD_REACHED,
    NO_MSGS_THRESHOLD_REACHED,
    MONITOR_ALREADY_EXISTS,
    NO_MONITOR_EXISTS,
    MONITOR_PERM
  };
  
  enum ProbeReturn{
    PROBE_INSTALLED,
    PROBE_ALREADY_INSTALLED,      
    PROBE_DEINSTALLED,
    PROBE_NONEXISTENT,
    PROBE_OF_DIFFERENT_KIND,
    PROBE_PERM,
    PROBE_TEMP,
    PROBE_OK
  };
  
  enum GiveUpInput{
    ALL_GIVEUP,         // debug purpose only
    TEMP_GIVEUP
  };
  
  enum GiveUpReturn{
    GIVES_UP,
    SITE_NOW_NORMAL,
    SITE_NOW_PERM
  };
  
  enum ProbeType{
    PROBE_TYPE_ALL,
    PROBE_TYPE_PERM,
    PROBE_TYPE_NONE
  };
  
  enum ProxyStatus{
    PROXY_STATUS_UNSET,
    PROXY_STATUS_REMOTE,
    PROXY_STATUS_HOME
  };
  
  // ******************************** ENVIRONMENT ************************************

  class DssConfigData {
  private:
    // distributed reference consistencey
    static const int DP_TL_LEASE;
    static const int DP_TL_UPDATE;
    static const int DP_WRC_ALPHA;
    
  public:
    static const int PRIMARY_SITE_TABLE_SIZE;
    static const int DEFAULT_MANAGER_TABLE_SIZE;
    static const int DEFAULT_PROXY_TABLE_SIZE;
    static const int DEFAULT_NAME_TABLE_SIZE;
    
    unsigned int gc_wrc_alpha;
    int          gc_tl_updateTime;
    int          gc_tl_leaseTime;
    
    DssConfigData();
    ~DssConfigData(){}
  };


  // ********************************** THE ENVIRONMENTS **************************************
  //
  // This is the actual DSS, the environment is used by the stateless
  // DSS components to form a complete unit
  //

  class DSS_Environment{
    // *********************** LOCAL VARIABLES ************************
  public:
    // *********************** GLOBAL VARIABLES ************************
    Mediation_Object*    const a_map;
    
    DksInstanceHT*             a_dksInstHT; 
    ProxyTable*                a_proxyTable;
    CoordinatorTable*          a_coordinatorTable;
    GlobalThreadTable*         a_threadTable; 
    DSite*                     a_myDSite;

    DssConfigData              a_dssconf;
    DssMslClbk*                a_dssMslClbk;
    MsgnLayer*                 a_msgnLayer;
    
    DksBackbone*                a_dksBackbone; 

    // For evaluation of how many operations done per site.
    // This is good to know when optimizing reference handling and 
    // configurating of dgc algs.
 
    int                        a_CreateXistRefCounter;
    int                        a_CreateNonXRefCounter;
    int                        a_DuplicateXistRefCounter;
    int                        a_DuplicateNonXRefCounter;
    int                        a_DuplicateToOwnerRefCounter;

  private:
    DSS_Environment(const DSS_Environment& de);
    DSS_Environment& operator=(const DSS_Environment& de){ return *this; }

  public:
    DSS_Environment(ComServiceInterface *  const sa, 
		    Mediation_Object* const mo,
		    const bool& sec);
    
    virtual ~DSS_Environment(); //closeDSS

    DSite                     *m_getDestDSite();
    DSite                     *m_getSrcDSite();

    // Proxy "factory" methods, creates proxies according to
    // specifications.

    Proxy* m_initializeCoordination(const ProtocolName& prot,
				    const AccessArchitecture& aa, 
				    const RCalg& GC_annot);

    Proxy* m_unmarshalProxy(DssReadBuffer* const buf,
			    const ProxyUnmarshalFlag& flag,
			    AbstractEntityName& aen,
			    bool &trailingState);

    DssThreadId *m_createDssThreadId(); 
    
    bool m_orderEntities(AbstractEntity* const ae_first,
			 AbstractEntity* const ae_second);
    
    void            m_gcDssResources();
    
    ParamRetVal     m_operateIntParam(const DSS_AREA& area, const DSS_AREA_ID& id, const int& param, int& arg);
    ParamRetVal     m_operateStrParam(const DSS_AREA& area, const DSS_AREA_ID& id, const int& param, const char* const str);
  
    KbrInstance* m_createKbr(int K, int Bits, int Fail, KbrCallbackInterface* inf);
    bool m_unmarshalKbr(DssReadBuffer* buf, KbrInstance* &inst);

    void m_setupBackbone(DssWriteBuffer* buf);
    void m_joinBackbone(DssReadBuffer *buf);
  };
  

} // End namespace

#endif
