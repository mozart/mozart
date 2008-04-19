/*
 *  Authors:
 *   Zacharias El Banna, zeb@sics.se
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
 */

#ifndef __DSS_ENUMS_HH
#define __DSS_ENUMS_HH

typedef void* opaque;
typedef unsigned char BYTE;
typedef unsigned int u32; // unsigned 32 bit. if ever going' 64, some simplifications can be done

#define MACRO_NO_DEFAULT_CONSTRUCTORS(aclass)	\
private:                                \
  aclass(const aclass &);		\
  aclass &operator = (const aclass&)


#define MACRO_NO_DEFAULT_EQUALITY(aclass)	\
private:                                \
  aclass &operator = (const aclass&)


// ****************** Exceptions ********************
// Exceptions are used in various functions to declare something wrong
// has happened.
//

enum ExceptionType{
  EXCEPTION_NO_DATA = 1  // Thrown by DssReadBuffer.getByte()
};



// ************ Abstract operation return values ********
// The return values for the doAbstractOperation pair signals to the
// caller how it should act.
// - PROCEED means that the operation should be carried out immediatly
//   by the caller.
// - SKIP means that the caller should continue but the operation will
//   be carried by the DSS
// - RAISE, Something has happended and the protocol has found out that..??? 
// - SUSPEND The caller should suspend and await a resume operation
// - INTERNAL_ERROR_XXX something was supplied to the protocol which
//   was not ok.
//

enum OpRetVal{
  DSS_PROCEED,
  DSS_SKIP,
  DSS_RAISE,
  DSS_SUSPEND,
  DSS_INTERNAL_ERROR_NO_PROXY,
  DSS_INTERNAL_ERROR_NO_OP,
  DSS_INTERNAL_ERROR_THREAD_OP,
  DSS_INTERNAL_ERROR_SEVERE
};


enum ProxyMarshalFlag{
  PMF_ORDINARY, 
  PMF_PUSH,
  PMF_FREE,
  PMF_MASK  = 0xF  // bit mask
};
const int PMF_NBITS = 4;     // must be consistent with PMF_MASK


enum ASop{
  ASO_MIGRATE,
  ASO_PUSH
};


// ****************** DSS_GC ******************
// The return values of the Proxy::manipulatAS function are not very
// thorough, either the action succeded (OK) ( is successfully
// initiated) or it failed because the argument wasn't (ARG_FAIL)
// valid, or the operation could not be executed for the current
// Resolver protocol (OP_FAIL).

enum MASretVal{
  MAS_OK, 
  MAS_ARG_FAIL, 
  MAS_OP_FAIL
};


// ****************** DSS_GC ******************
// DSS_GC is the value returned from getDssGCStatus
// it indicates what the current garbage collection status of the 
// proxy is. From the DSS perspective of course
// - DSS_GC_NONE, means that nothing prevents the Proxy from being reclaimed
// - DSS_GC_WEAK ,means that the Protocol (or ORP used in the documentation)
//   is in a state where it would/might hurt the entire entity's computation if this
//   particular proxy was deleted.
// - DSS_GC_PRIMARY, the Proxy is a strong root to the DSS, should never be
//   removed by the MAP (else the behavior of the entity is uncertain)
// - DSS_LOCALIZE, the Proxy has concluded that no other references to the entity
//   exists except this one and thus it might be removed from the DSS and the state
//   might be controlled by the local CSS again. This is just an indication, the MAP
//   is not required to localize the entity.

enum DSS_GC{
  DSS_GC_NONE,
  DSS_GC_WEAK,
  DSS_GC_PRIMARY,
  DSS_GC_LOCALIZE
};



// FaultState describe the fault status of entities and sites.  In
// order to simplify registration and reporting, bit masking is used,
// so every state corresponds to one bit.
//
// Our model defines the following four states: FS_OK means no failure
// detected; FS_TEMP is a failure suspicion, the state may go back to
// FS_OK; FS_LOCAL_PERM is a permanent crash for the current site (but
// other sites may be not affected); FS_GLOBAL_PERM is a permanent
// crash for all sites (global guarantee).
typedef unsigned int FaultState;
const FaultState FS_OK          = 1;
const FaultState FS_TEMP        = 2;
const FaultState FS_LOCAL_PERM  = 4;
const FaultState FS_GLOBAL_PERM = 8;

// other useful bit masks
const FaultState FS_MASK        = 0xF;
const FaultState FS_PERM        = FS_LOCAL_PERM | FS_GLOBAL_PERM;
const int FS_NBITS = 4;

// Entities have two fault states: one for their access architecture
// (coordination level), and one for their state (protocol level).
// They are combined into one fault state by bit-shifting.
const FaultState FS_COORD_OK          = FS_OK          << FS_NBITS;
const FaultState FS_COORD_TEMP        = FS_TEMP        << FS_NBITS;
const FaultState FS_COORD_LOCAL_PERM  = FS_LOCAL_PERM  << FS_NBITS;
const FaultState FS_COORD_GLOBAL_PERM = FS_GLOBAL_PERM << FS_NBITS;
const FaultState FS_COORD_MASK        = FS_MASK        << FS_NBITS;

const FaultState FS_STATE_OK          = FS_OK;
const FaultState FS_STATE_TEMP        = FS_TEMP;
const FaultState FS_STATE_LOCAL_PERM  = FS_LOCAL_PERM;
const FaultState FS_STATE_GLOBAL_PERM = FS_GLOBAL_PERM;
const FaultState FS_STATE_MASK        = FS_MASK;



// AbstractEntityName
enum AbstractEntityName {
  AEN_NOT_DEFINED       = 0, // Don't use, only here for internal debugging 
  AEN_MUTABLE           = 1,
  AEN_RELAXED_MUTABLE   = 2,
  AEN_TRANSIENT         = 3,
  AEN_IMMUTABLE         = 4,
  AEN_IMMUTABLE_UNNAMED = 5,
  AEN_MASK              = 0xF   // bit mask
};
const int AEN_NBITS = 4;     // must be consistent with AE_MASK



// ProtocolName
enum ProtocolName {
  PN_NO_PROTOCOL     = 0, // Don't use, only here for internal debugging 
  PN_SIMPLE_CHANNEL  = 1, 
  PN_MIGRATORY_STATE = 2,
  PN_PILGRIM_STATE   = 3,
  PN_EAGER_INVALID   = 4,
  PN_LAZY_INVALID    = 5,
  PN_TRANSIENT       = 6, 
  PN_TRANSIENT_REMOTE= 7,  
  PN_IMMUTABLE_LAZY  = 8,
  PN_IMMUTABLE_EAGER = 9,
  PN_IMMEDIATE       = 10,
  PN_DKSBROADCAST    = 11,
  PN_SITED           = 12,
  PN_MASK            = 0xF   // bit mask
};
const int PN_NBITS = 4;     // must be consistent with PN_MASK

// PN_SITED provides no actual protocol.  The home proxy has full
// access to the entity, while remote proxies have no access at all.
// Remote proxies can only detect when the home site fails.



// ********* AccessArchitecture *********
// The access architecture, or (R)esolver (L)ookup (P)rotocol as it
// is referred to in the documentation, see section XXX, defines the
// behavior of the resolver (manager). This is a property which is silently 
// transfered during passing of entities so the only time the property is
// considered is during Proxy creation.
// - STATIONARY_MANAGER, the resolver resides on the site which created the first
//   Proxy for an entity (by invoking createEMU with the alla parameters)
// - MIGRATORY_MANAGER, the resolver can utilise the migration features of the 
//   manipulateAS Proxy function.

enum AccessArchitecture{
  AA_NO_ARCHITECTURE    = 0x0,  // Don't use, only here for internal debugging 
  AA_STATIONARY_MANAGER = 0x1,
  AA_MIGRATORY_MANAGER  = 0x2,
  AA_MOBILE_COORDINATOR = 0x4,
  AA_MASK               = 0xF   // bit mask
};
const int AA_NBITS = 4;     // must be consistent with AA_MASK



// ***************** GC *******************
// During proxy creation a number of algs can be chosen
// by combining one or many of the below algs (bit masking)
// the desired set is acheived. see section XXX in the documentation
// and appendix XXX for details about the algorithms.
// - PERSIST means the proxy will be persistent, no reference info
// is used, the proxy will always be a root at the home
// - WRC, fractional WRC
// - TL, time lease, uses a timer to keep track of external refs.
// - RC, reference counting (don't use)
// - RLV1, Reference Listing version 1, keeps a list of external refs's sites
// - RLV2, version 2 of above.
// - IRC, indirectional RC (don't use)

enum RCalg{
  RC_ALG_NONE    = 0x00,   // Don't use, only here for internal debugging
  RC_ALG_PERSIST = 0x01,   // should not be combined with others!
  RC_ALG_WRC     = 0x02,
  RC_ALG_TL      = 0x04,
  RC_ALG_RC      = 0x08,
  RC_ALG_RLV1    = 0x10,
  RC_ALG_RLV2    = 0x20,
  RC_ALG_IRC     = 0x40,
  RC_ALG_ERROR   = 0x80,
  RC_ALG_MASK    = 0xFF   // bit mask
};
const int RC_ALG_NBITS = 8;     // must be consistent with RC_ALG_MASK



// ************** RC_OPS *****************
// remember that all value changing ops are "local" to the 
// specific proxy in manipulateRC.
//
// WRC_ALPHA is the value used to divide the weight with.
// for TimeLease there are two values, either:
// - the LEASE_PERIOD (for Home part of algorithm) which is amount 
// of time added to the current time representing when it might,
// earliest, be reclaimed.
// - the UPDATE_PERIOD time (for Remote part of algorithm), represent the period
// before the expiration of the lease, which is used to get an update of the
// lease. The better a connection is the shorter period needed.
// 

enum RCop{
  RC_OP_REMOVE_ALG,
  RC_OP_SET_WRC_ALPHA,
  RC_OP_GET_WRC_ALPHA,
  RC_OP_SET_TL_LEASE_PERIOD,
  RC_OP_GET_TL_LEASE_PERIOD,
  RC_OP_SET_TL_UPDATE_PERIOD,
  RC_OP_GET_TL_UPDATE_PERIOD
};

// *********** ProxyUnmarshalFlag ***********
// Depending on the marshsaling strategy
// this flag hints to the unmarshaling function if a source is
// to expect or not (for safety reasons)
// ORDINARY unmarshalling requires a source while FREE does not
// see section XXX in the 

enum ProxyUnmarshalFlag{
  PUF_ORDINARY,
  PUF_FREE
};

enum AOcallback{
  AOCB_FINISH,
  AOCB_CONTINUE
};

enum ResumeCode{
  TRC_CONTINUE,
  TRC_REDO,
  TRC_RAISE,
  TRC_ATOMIC
};


enum WakeRetVal{
  WRV_DONE,       // The operation was completed, an atomoc operation
  WRV_CONTINUING, // The operation is scheduled to be executed
  WRV_NO_THREAD,  // The operation is not executed, the thread is busy elsewhere
  WRV_NO_OP       // ? 
};


enum BufOpRet{
  BOR_OP_OK,
  BOR_SPACE_UNAWAIL
};


enum ErrorClass {
  EC_OK,
  EC_GO_AHEAD,
  EC_CONTINUE_LATER,
  EC_LOST              // This perm only means the transObj should give up
};


enum ConGrantReturn{
  CGR_CONTINUE,
  CGR_SUSPENDED, 
  CGR_FAILED, 
  CGR_STATE_ERROR
};


enum ConnectionType{
  CT_TCP_CONNECTION,
  CT_ROUTE_CONNECTION
};


enum ConnectionFailType{
  CFT_CONNECTION_LOST,
  CFT_SITE_GLOBAL_PERM, 
  CFT_SITE_LOCAL_PERM
};


enum SiteFaultInterest{
  SFI_NOTDEFINEDYET
}; 


enum SiteFaultState{
  SFS_NOTDEFINEDYET
}; 


enum DSS_AREA{
  DSS_STATIC,
  DSS_AREA_TABLES,
  DSS_AREA_MESSAGES,
  DSS_AREA_SITES
};


enum DSS_AREA_ID{
  DSS_AREA_NO_OP,
  DSS_STATIC_LOG_PARAMETER,
  DSS_STATIC_DEBUG_TABLES,
  DSS_STATIC_MEMORY_ALLOCATION,
  DSS_STATIC_GET_COMINFO
};


enum ParamRetVal{
  PRV_OK,
  PRV_TYPE_ERROR,
  PRV_AREA_NOT_FOUND,
  PRV_DYN_PARAM_NOT_FOUND,
  PRV_STAT_PARAM_NOT_FOUND
};


// ************************ LOG_LEVEL ***************************
//
// When the DSS is compiled with the DSS_LOG flag the logging feature
// is activated. When a level is set only info at that level and lower
// (value) levels is printed.
// - DLL_NOTHING   the utility is effectivly deactivated.
// - DLL_PRINT     information which is commanded to be printed using printing tools.
// - DLL_IMPRTANT important info.
// - DLL_BEHAVIOR  Characteristic behavior of the DSS like connections and such 
// - DLL_DEBUG     Debugging info like internal messages, typically the 
//                 necessesary info when creating a MAP.
// - DLL_ALL       Loads of information, from internal function behavior
//                 to unmarshaled fields.

enum DSS_LOG_LEVEL{
  DLL_NOTHING   = 0, // The loglevel is set to output nothing (i.e. no one should use the level)
  DLL_PRINT     = 1, // this level should be found in ordinary printing utilities
  DLL_IMPORTANT = 2,
  DLL_BEHAVIOR  = 3,
  DLL_DEBUG     = 4,
  DLL_ALL       = 5
};

// The new bunch! 

typedef unsigned int ConnectivityStatus;
const ConnectivityStatus CS_NONE          = 0x000;
const ConnectivityStatus CS_COMMUNICATING = 0x001;
const ConnectivityStatus CS_CHANNEL       = 0x002;
const ConnectivityStatus CS_CIRCUIT       = 0x004;
const ConnectivityStatus CS_OPENING       = 0x008;
const ConnectivityStatus CS_CLOSING       = 0x010;
const ConnectivityStatus CS_OPEN          = 0x020;
const ConnectivityStatus CS_UNACKED       = 0x040;
const ConnectivityStatus CS_FRAGMENTED    = 0x080;
const ConnectivityStatus CS_QUEUED        = 0x100;
const ConnectivityStatus CS_EXTNEED       = 0x200;
const ConnectivityStatus CS_ROUTER        = 0x400;
const ConnectivityStatus CS_CLOSED        = 0x800;

enum ConnectionFailReason{
  CFR_TIMEOUT, 
  CFR_NO_ROUTE,
  CFR_NO_HOST,
  CFR_NO_RESOURCE,
  CFR_INVALID_ADDRESS
};


enum KbrResult{
  KBR_LOCAL, 
  KBR_REMOTE,
  KBR_FAILED_OPENING,
  KBR_FAILED_CLOSING,
  KBR_FAILED_INVALIDKEY
};

#endif
