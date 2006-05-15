/*
 *  Authors:
 *    Denys Duchier <duchier@ps.uni-sb.de>
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Denys Duchier, 1997
 *    Christian Schulte, 1997
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

// EMULATOR PROPERTIES
//
// We define here a single interface to all emulator properties.  This
// is intended to put an end to the proliferation of specialized builtins.
// It will also permit an optimized representation of emulator properties
// as Virtual Properties (see later): as ints rather than as instances of
// class VirtualProperty.
//

#include "vprops.hh"
#include "hashtbl.hh"
#include "fdomn.hh"
#include "am.hh"
#include "os.hh"
#include "codearea.hh"
#include "OZCONF.h"
#include "builtins.hh"
#include "dictionary.hh"

#include <stdarg.h>

extern char *AMVersion, *AMDate;

// "ozplatform" (defined in version.cc) has the form <osname-cpu>, 
extern char *ozplatform;

enum EmulatorPropertyIndex {
  // THREADS
  PROP_THREADS_CREATED,
  PROP_THREADS_RUNNABLE,
  PROP_THREADS_MIN,
  PROP_THREADS,
  // PRIORITIES
  PROP_PRIORITIES_HIGH,
  PROP_PRIORITIES_MEDIUM,
  PROP_PRIORITIES,
  // PICKLES;
  PROP_PICKLE_CELLS,
  PROP_PICKLE,
  // TIME
  PROP_TIME_COPY,
  PROP_TIME_GC,
  PROP_TIME_PROPAGATE,
  PROP_TIME_RUN,
  PROP_TIME_SYSTEM,
  PROP_TIME_TOTAL,
  PROP_TIME_USER,
  PROP_TIME_IDLE,
  PROP_TIME_DETAILED,
  PROP_TIME,
  // GC
  PROP_GC_MIN,
  PROP_GC_FREE,
  PROP_GC_TOLERANCE,
  PROP_GC_ON,
  PROP_GC_THRESHOLD,
  PROP_GC_SIZE,
  PROP_GC_ACTIVE,
  PROP_GC_CODE_CYCLES,
  PROP_GC,
  // PRINT
  PROP_PRINT_DEPTH,
  PROP_PRINT_WIDTH,
  PROP_PRINT_FLOATPRECISION,
  PROP_PRINT_SCIENTIFICFLOATS,
  PROP_PRINT_VERBOSE,
  PROP_PRINT,
  // FD
  PROP_FD_VARIABLES,
  PROP_FD_PROPAGATORS,
  PROP_FD_INVOKED,
  PROP_FD_THRESHOLD,
  PROP_FD,
  // SPACES
  PROP_SPACES_COMMITTED,
  PROP_SPACES_CLONED,
  PROP_SPACES_CREATED,
  PROP_SPACES_FAILED,
  PROP_SPACES_SUCCEEDED,
  PROP_SPACES,
  // ERRORS
  PROP_ERRORS_HANDLER,
  PROP_ERRORS_DEBUG,
  PROP_ERRORS_THREAD,
  PROP_ERRORS_DEPTH,
  PROP_ERRORS_WIDTH,
  PROP_ERRORS,
  // MESSAGES
  PROP_MESSAGES_GC,
  PROP_MESSAGES_IDLE,
  PROP_MESSAGES,
  // MEMORY
  PROP_MEMORY_ATOMS,
  PROP_MEMORY_NAMES,
  PROP_MEMORY_FREELIST,
  PROP_MEMORY_CODE,
  PROP_MEMORY_HEAP,
  PROP_MEMORY,
  // LIMITS
  PROP_LIMITS_INT_MIN,
  PROP_LIMITS_INT_MAX,
  PROP_LIMITS_BYTECODE_XREGISTERS,
  PROP_LIMITS,
  // APPLICATION
  PROP_APPLICATION_ARGS,
  PROP_APPLICATION_URL,
  PROP_APPLICATION_GUI,
  PROP_APPLICATION,
  // PLATFORM
  PROP_PLATFORM_NAME,
  PROP_PLATFORM_OS,
  PROP_PLATFORM_ARCH,
  PROP_PLATFORM,
  // MISC
  PROP_STANDALONE,
  PROP_OZ_CONFIGURE_HOME,
  PROP_OZ_EMULATOR_HOME,
  PROP_OZ_VERSION,
  PROP_OZ_DATE,
  // DISTRIBUTION
  PROP_DISTRIBUTION_VIRTUALSITES,
  // Marshaler
  PROP_MARSHALER_VERSION,
  // INTERNAL
  PROP_INTERNAL_DEBUG,
  PROP_INTERNAL_PROPLOCATION,
  PROP_INTERNAL_SUSPENSION,
  PROP_INTERNAL_STOP,
  PROP_INTERNAL_ENGINE_SUSP,
  PROP_INTERNAL_DEBUG_IP,
  PROP_INTERNAL,
  // PERDIO renamed DP
  PROP_DP_SEIFHANDLER,
  PROP_DP_FLOWBUFFERSIZE,
  PROP_DP_FLOWBUFFERTIME,
  PROP_DP_DEBUG,
  PROP_DP_VERSION,
  PROP_DP_CLOCKTICK,
  PROP_DP_USEALTVARPROTOCOL,
  PROP_DP_RETRYTIMECEILING,
  PROP_DP_RETRYTIMEFLOOR,
  PROP_DP_RETRYTIMEFACTOR,
  PROP_DP_TCPHARDLIMIT,
  PROP_DP_TCPWEAKLIMIT,
  PROP_DP_PROBEINTERVAL,
  PROP_DP_PROBETIMEOUT,
  PROP_DP_OPENTIMEOUT,
  PROP_DP_CLOSETIMEOUT,
  PROP_DP_WFREMOTETIMEOUT,
  PROP_DP_FIREWALLREOPENTIMEOUT,
  PROP_DP_DEFAULTBUFFERSIZE,
  PROP_DP_DEFAULTMAXBUFFERSIZE,
  PROP_DP_BUFFERSIZE,
  PROP_DP_MAXBUFFERSIZE,
  PROP_DP,
  // DPTABLE
  PROP_DPTABLE_DEFAULTOWNERTABLESIZE,
  PROP_DPTABLE_DEFAULTBORROWTABLESIZE,
  PROP_DPTABLE_LOWLIMIT,
  PROP_DPTABLE_EXPANDFACTOR,
  PROP_DPTABLE_BUFFER,
  PROP_DPTABLE_WORTHWHILEREALLOC,
  PROP_DPTABLE,
  // DPLOG
  PROP_DPLOG_CONNECTLOG,
  PROP_DPLOG_MESSAGELOG,
  PROP_DPLOG,
  // DPGC
  PROP_DPGC_TIMELEASE,
  PROP_DPGC_FRACWRC,
  PROP_DPGC_TL_LEASETIME,
  PROP_DPGC_TL_UPDATETIME,
  PROP_DPGC_WRC_ALPHA,
  PROP_DPGC,
  
  PROP_CLOSE_TIME,

  PROP_OZ_STYLE_USE_FUTURES,
  // this must remain last
  PROP__LAST
};


static OZ_Term getApplicationArgs(void) {
  TaggedRef out = oz_nil();
  for(int i=ozconf.argC-1; i>=0; i--)
    out = oz_cons(oz_atomNoDup(ozconf.argV[i]),out);
  return out;
}

// Handle the case of indexed property P whose value can be
// found at location L.  Return the corresponding Oz term.

#define CASE_INT( P,L) case P: return OZ_int(L)
#define CASE_BOOL(P,L) case P: return oz_bool(L)
#define CASE_ATOM(P,L) case P: return oz_atomNoDup(L)
#define CASE_UNSIGNEDINT(P,L) case P: return OZ_unsignedInt(L)

// Construct an Arity given `n' atoms.  First argument is n
// i.e. the number of features, the following arguments are
// the n atoms.  Creating an arity is an expensive operation,
// but we are going to cache the required arity in local
// static variables.  Each arity is computed only the 1st time
// it is needed.

static
OZ_Arity mkArity(int n,...)
{
  va_list(ap);
  va_start(ap,n);
  OZ_Term list = oz_nil();
  for (int i=0;i<n;i++) list = oz_cons(va_arg(ap,OZ_Term),list);
  va_end(ap);
  return OZ_makeArity(list);
}

// DEFINE_REC(L,A) puts in REC__ a record with label L and
// arity A.  Both the record and arity are cached in local
// static variables and are computed only the 1st time they
// are needed.  L is specified as a C string, and A is of
// the form (n,F1,...,Fn) where F1 to Fn are Atoms which we
// assume are already initialized (they are expected to have
// been created in value.cc)

#define DEFINE_REC(L,A)				\
static OZ_Term  LAB__ = 0;			\
static OZ_Arity ARY__;				\
if (LAB__==0) {					\
  LAB__ = oz_atomNoDup(L);			\
  ARY__ = mkArity A ;				\
}						\
REC__ = SRecord::newSRecord(LAB__,(Arity*)ARY__);

// tag and return the result record REC__
#define RETURN_REC return makeTaggedSRecord(REC__);

// set feature F of the result record REC__
#define SET_REC(F,V) REC__->setFeature(F,V)

// create a result record REC__, DO something, an return
// the tagged result

#define DO_REC(L,A,DO) { DEFINE_REC(L,A); DO; RETURN_REC; }

// Handle the case of indexed property P, whose result is
// a record with label L and arity A, and DO something to
// initialize its features before returning it.

#define CASE_REC(P,L,A,DO) case P: DO_REC(L,A,DO);

// set feature F or REC__ to appropriate term

#define SET_INT( F,I) SET_REC(F,OZ_int( I))
#define SET_BOOL(F,B) SET_REC(F,oz_bool(B))
#define SET_ATOM(F,A) SET_REC(F,oz_atom(A))
#define SET_UNSIGNEDINT(F,I) SET_REC(F,OZ_unsignedInt(I))

OZ_Term GetEmulatorProperty(EmulatorPropertyIndex prop) {
  SRecord * REC__;
  switch (prop) {
    // THREADS
    CASE_INT(PROP_THREADS_CREATED,ozstat.createdThreads.total);
    CASE_INT(PROP_THREADS_RUNNABLE,am.threadsPool.getRunnableNumber());
    CASE_INT(PROP_THREADS_MIN,ozconf.stackMinSize / TASKFRAMESIZE);
    CASE_REC(PROP_THREADS,"threads",
	     (3,AtomCreated,AtomRunnable,AtomMin),
	     SET_INT(AtomCreated ,ozstat.createdThreads.total);
	     SET_INT(AtomRunnable,am.threadsPool.getRunnableNumber());
	     SET_INT(AtomMin     ,ozconf.stackMinSize/TASKFRAMESIZE););
    // PRIORITIES
    CASE_INT(PROP_PRIORITIES_HIGH,ozconf.hiMidRatio);
    CASE_INT(PROP_PRIORITIES_MEDIUM,ozconf.midLowRatio);
    CASE_REC(PROP_PRIORITIES,"priorities",(2,AtomHigh,AtomMedium),
	     SET_INT(AtomHigh,ozconf.hiMidRatio);
	     SET_INT(AtomMedium,ozconf.midLowRatio););
    // PICKLES
    CASE_BOOL(PROP_PICKLE_CELLS,ozconf.pickleCells);
    CASE_REC(PROP_PICKLE,"pickle",(1,AtomCells),
	     SET_BOOL(AtomCells,ozconf.pickleCells););
    // TIME
    CASE_INT(PROP_TIME_COPY,ozconf.timeDetailed?ozstat.timeForCopy.total:0);
    CASE_INT(PROP_TIME_GC,ozconf.timeDetailed?ozstat.timeForGC.total:0);
    CASE_INT(PROP_TIME_PROPAGATE,ozconf.timeDetailed?ozstat.timeForPropagation.total:0);
    CASE_INT(PROP_TIME_RUN,ozconf.timeDetailed ?
	     (osUserTime() - (ozstat.timeForCopy.total +
			      ozstat.timeForGC.total +
			      ozstat.timeForPropagation.total)):0);
    CASE_INT(PROP_TIME_SYSTEM,osSystemTime());
    CASE_INT(PROP_TIME_TOTAL,osTotalTime());
    CASE_INT(PROP_TIME_USER,osUserTime());
    CASE_INT(PROP_TIME_IDLE,(int) ozstat.timeIdle);
    CASE_BOOL(PROP_TIME_DETAILED,ozconf.timeDetailed);
    CASE_REC(PROP_TIME,"time",
	     (9,AtomCopy,AtomGC,AtomPropagate,AtomRun,
	      AtomSystem,AtomTotal,AtomUser,AtomDetailed,AtomIdle),
	     unsigned int timeNow = osUserTime();
	     unsigned int copy = 0;
	     unsigned int gc   = 0;
	     unsigned int prop = 0;
	     unsigned int run  = 0;
	     if (ozconf.timeDetailed) {
	       copy = ozstat.timeForCopy.total;
	       gc   = ozstat.timeForGC.total;
	       prop = ozstat.timeForPropagation.total;
	       run  = timeNow - (copy + gc + prop);
	     }
	     SET_INT(AtomCopy,copy);
	     SET_INT(AtomGC,gc);
	     SET_INT(AtomPropagate,prop);
	     SET_INT(AtomRun,run);
	     SET_INT(AtomSystem,osSystemTime());
	     SET_INT(AtomTotal,osTotalTime());
	     SET_INT(AtomUser,timeNow);
	     SET_INT(AtomIdle,(int) ozstat.timeIdle);
	     SET_BOOL(AtomDetailed,ozconf.timeDetailed););
    // GC
    CASE_INT(PROP_GC_MIN,ozconf.heapMinSize*KB);
    CASE_INT(PROP_GC_FREE,ozconf.heapFree);
    CASE_INT(PROP_GC_TOLERANCE,ozconf.heapTolerance);
    CASE_INT(PROP_GC_CODE_CYCLES,ozconf.codeGCcycles);
    CASE_BOOL(PROP_GC_ON,ozconf.gcFlag);
    CASE_UNSIGNEDINT(PROP_GC_THRESHOLD,ozconf.heapThreshold*KB);
    CASE_UNSIGNEDINT(PROP_GC_SIZE,getUsedMemory()*KB);
    CASE_UNSIGNEDINT(PROP_GC_ACTIVE,ozstat.gcLastActive*KB);
    CASE_REC(PROP_GC,"gc",
	     (8,AtomCodeCycles,AtomMin,AtomFree,AtomTolerance,
	      AtomOn,AtomThreshold,AtomSize,AtomActive),
	     SET_UNSIGNEDINT(AtomMin,       ozconf.heapMinSize*KB);
	     SET_UNSIGNEDINT(AtomFree,      ozconf.heapFree);
	     SET_UNSIGNEDINT(AtomTolerance, ozconf.heapTolerance);
	     SET_BOOL(AtomOn,       ozconf.gcFlag);
	     SET_UNSIGNEDINT(AtomThreshold, ozconf.heapThreshold*KB);
	     SET_UNSIGNEDINT(AtomSize,      getUsedMemory()*KB);
	     SET_INT(AtomCodeCycles, ozconf.codeGCcycles);
	     SET_INT(AtomActive,    ozstat.gcLastActive*KB););
    // PRINT
    CASE_INT(PROP_PRINT_DEPTH,ozconf.printDepth);
    CASE_INT(PROP_PRINT_WIDTH,ozconf.printWidth);
    CASE_INT(PROP_PRINT_FLOATPRECISION,ozconf.printFloatPrecision);
    CASE_BOOL(PROP_PRINT_SCIENTIFICFLOATS,ozconf.printScientificFloats);
    CASE_BOOL(PROP_PRINT_VERBOSE,ozconf.printVerbose);
    CASE_REC(PROP_PRINT,"print",(5,
				 AtomDepth,AtomWidth,
				 AtomFloatPrecision,AtomScientificFloats,
				 AtomVerbose),
	     SET_INT(AtomDepth, ozconf.printDepth);
	     SET_INT(AtomWidth, ozconf.printWidth);
	     SET_INT(AtomFloatPrecision, ozconf.printFloatPrecision);
	     SET_BOOL(AtomScientificFloats, ozconf.printScientificFloats);
	     SET_BOOL(AtomVerbose, ozconf.printVerbose););
    // FD
    CASE_INT(PROP_FD_VARIABLES,ozstat.fdvarsCreated.total);
    CASE_INT(PROP_FD_PROPAGATORS,ozstat.propagatorsCreated.total);
    CASE_INT(PROP_FD_INVOKED,ozstat.propagatorsInvoked.total);
    CASE_INT(PROP_FD_THRESHOLD,32 * fd_bv_max_high);
    CASE_REC(PROP_FD,"fd",
	     (4,AtomVariables,AtomPropagators,AtomInvoked,AtomThreshold),
	     SET_INT(AtomVariables,   ozstat.fdvarsCreated.total);
	     SET_INT(AtomPropagators, ozstat.propagatorsCreated.total);
	     SET_INT(AtomInvoked,     ozstat.propagatorsInvoked.total);
	     SET_INT(AtomThreshold,   32 * fd_bv_max_high););
    // SPACES
    CASE_INT(PROP_SPACES_COMMITTED,ozstat.solveAlt.total);
    CASE_INT(PROP_SPACES_CLONED,ozstat.solveCloned.total);
    CASE_INT(PROP_SPACES_CREATED,ozstat.solveCreated.total);
    CASE_INT(PROP_SPACES_FAILED,ozstat.solveFailed.total);
    CASE_INT(PROP_SPACES_SUCCEEDED,ozstat.solveSolved.total);
    CASE_REC(PROP_SPACES,"spaces",
	     (5,AtomCommitted,AtomCloned,AtomCreated,AtomFailed,AtomSucceeded),
	     SET_INT(AtomCommitted,ozstat.solveAlt.total);
	     SET_INT(AtomCloned,ozstat.solveCloned.total);
	     SET_INT(AtomCreated,ozstat.solveCreated.total);
	     SET_INT(AtomFailed,ozstat.solveFailed.total);
	     SET_INT(AtomSucceeded,ozstat.solveSolved.total););
    // ERRORS
  case PROP_ERRORS_HANDLER: {
    TaggedRef ehdl = am.getDefaultExceptionHdl();
    return ehdl ? ehdl : oz_nil();
  }
    CASE_BOOL(PROP_ERRORS_DEBUG,ozconf.errorDebug);
    CASE_INT(PROP_ERRORS_THREAD,ozconf.errorThreadDepth);
    CASE_INT(PROP_ERRORS_DEPTH,ozconf.errorPrintDepth);
    CASE_INT(PROP_ERRORS_WIDTH,ozconf.errorPrintWidth);
    CASE_REC(PROP_ERRORS,"errors",
	     (4,AtomDebug,AtomThread,
	      AtomDepth,AtomWidth),
	     SET_BOOL(AtomDebug,ozconf.errorDebug);
	     SET_INT(AtomThread,ozconf.errorThreadDepth);
	     SET_INT(AtomDepth,ozconf.errorPrintDepth);
	     SET_INT(AtomWidth,ozconf.errorPrintWidth););
    // MESSAGES
    CASE_BOOL(PROP_MESSAGES_GC,ozconf.gcVerbosity);
    CASE_BOOL(PROP_MESSAGES_IDLE,ozconf.showIdleMessage);
    CASE_REC(PROP_MESSAGES,"messages",
	     (2,AtomGC,AtomIdle),
	     SET_BOOL(AtomGC,ozconf.gcVerbosity);
	     SET_BOOL(AtomIdle,ozconf.showIdleMessage););
    // MEMORY
    CASE_INT(PROP_MEMORY_ATOMS,ozstat.getAtomMemory());
    CASE_INT(PROP_MEMORY_NAMES,ozstat.getNameMemory());
    CASE_INT(PROP_MEMORY_FREELIST,FL_Manager::getSize());
    CASE_INT(PROP_MEMORY_CODE,CodeArea::getTotalSize());
    CASE_INT(PROP_MEMORY_HEAP,ozstat.heapUsed.total+getUsedMemory());
    CASE_REC(PROP_MEMORY,"memory",
	     (5,AtomAtoms,AtomNames,AtomFreelist,
	      AtomCode,AtomHeap),
	     SET_INT(AtomAtoms,ozstat.getAtomMemory());
	     SET_INT(AtomNames,ozstat.getNameMemory());
	     SET_INT(AtomFreelist,FL_Manager::getSize());
	     SET_INT(AtomCode,CodeArea::getTotalSize());
	     SET_INT(AtomHeap,ozstat.heapUsed.total+getUsedMemory()););
    // LIMITS
    CASE_INT(PROP_LIMITS_INT_MIN,OzMinInt);
    CASE_INT(PROP_LIMITS_INT_MAX,OzMaxInt);
    CASE_INT(PROP_LIMITS_BYTECODE_XREGISTERS,NumberOfXRegisters);
    CASE_REC(PROP_LIMITS,"limits",
	     (3,AtomIntMin,AtomIntMax,AtomBytecodeXRegisters),
	     SET_INT(AtomIntMin,OzMinInt);
	     SET_INT(AtomIntMax,OzMaxInt);
	     SET_INT(AtomBytecodeXRegisters,NumberOfXRegisters););
    // APPLICATION
  case PROP_APPLICATION_ARGS: { return getApplicationArgs(); }
    CASE_ATOM(PROP_APPLICATION_URL,ozconf.url);
    CASE_BOOL(PROP_APPLICATION_GUI,ozconf.gui==1);
    CASE_REC(PROP_APPLICATION,"application",
	     (3,AtomArgs,AtomURL,AtomGUI),
	     SET_BOOL(AtomGUI,ozconf.gui==1);
	     SET_ATOM(AtomURL,ozconf.url);
	     SET_REC(AtomArgs,getApplicationArgs()););
    // PLATFORM
    CASE_ATOM(PROP_PLATFORM_NAME, ozplatform);
    CASE_ATOM(PROP_PLATFORM_OS,   ozconf.osname);
    CASE_ATOM(PROP_PLATFORM_ARCH, ozconf.cpu);
    CASE_REC(PROP_PLATFORM,"platform",
             (3,AtomName, AtomOs, AtomArch),
             SET_ATOM(AtomName,ozplatform);
	     SET_ATOM(AtomOs,ozconf.osname);
	     SET_ATOM(AtomArch,ozconf.cpu););
    // MISC
  CASE_BOOL(PROP_STANDALONE,!ozconf.runningUnderEmacs);
  CASE_ATOM(PROP_OZ_CONFIGURE_HOME,OZ_CONFIGURE_PREFIX);
  CASE_ATOM(PROP_OZ_EMULATOR_HOME,ozconf.emuhome);
  CASE_ATOM(PROP_OZ_VERSION,AMVersion);
  CASE_ATOM(PROP_OZ_DATE,AMDate);
  // DISTRIBUTION
#ifdef VIRTUALSITES
  CASE_BOOL(PROP_DISTRIBUTION_VIRTUALSITES,OK);
#else
  CASE_BOOL(PROP_DISTRIBUTION_VIRTUALSITES,NO);
#endif
  // Marshaler
  case PROP_MARSHALER_VERSION: return OZ_pair2(oz_int(MARSHALERMAJOR),
					       oz_int(MARSHALERMINOR));
  // INTERNAL
  CASE_BOOL(PROP_INTERNAL_DEBUG,am.debugmode());
  CASE_BOOL(PROP_INTERNAL_PROPLOCATION,am.isPropagatorLocation());
  CASE_BOOL(PROP_INTERNAL_SUSPENSION,ozconf.showSuspension);
  CASE_BOOL(PROP_INTERNAL_STOP,ozconf.stopOnToplevelFailure);
  CASE_INT(PROP_INTERNAL_ENGINE_SUSP,am.getSuspCnt());
  CASE_INT(PROP_INTERNAL_DEBUG_IP,ozconf.debugIP);
  // DP
  CASE_INT(PROP_DP_DEBUG,ozconf.debugPerdio);
  CASE_BOOL(PROP_DP_SEIFHANDLER,ozconf.dpSeifHandler);
  CASE_INT(PROP_DP_FLOWBUFFERSIZE,ozconf.dpFlowBufferSize);
  CASE_INT(PROP_DP_FLOWBUFFERTIME,ozconf.dpFlowBufferTime);
  CASE_INT(PROP_DP_RETRYTIMECEILING,ozconf.dpRetryTimeCeiling);
  CASE_INT(PROP_DP_RETRYTIMEFLOOR,ozconf.dpRetryTimeFloor);
  CASE_INT(PROP_DP_RETRYTIMEFACTOR,ozconf.dpRetryTimeFactor);
  CASE_INT(PROP_DP_TCPHARDLIMIT,ozconf.dpTCPHardLimit);
  CASE_INT(PROP_DP_TCPWEAKLIMIT,ozconf.dpTCPWeakLimit);
  CASE_INT(PROP_DP_PROBEINTERVAL,ozconf.dpProbeInterval);
  CASE_INT(PROP_DP_PROBETIMEOUT,ozconf.dpProbeTimeout);
  CASE_INT(PROP_DP_OPENTIMEOUT,ozconf.dpOpenTimeout);
  CASE_INT(PROP_DP_CLOSETIMEOUT,ozconf.dpCloseTimeout);
  CASE_INT(PROP_DP_WFREMOTETIMEOUT,ozconf.dpWFRemoteTimeout);
  CASE_INT(PROP_DP_FIREWALLREOPENTIMEOUT,ozconf.dpFirewallReopenTimeout);
  CASE_INT(PROP_DP_DEFAULTBUFFERSIZE,ozconf.dpDefaultBufferSize);
  CASE_INT(PROP_DP_DEFAULTMAXBUFFERSIZE,ozconf.dpDefaultMaxBufferSize);
  CASE_INT(PROP_DP_BUFFERSIZE,ozconf.dpBufferSize);
  CASE_INT(PROP_DP_MAXBUFFERSIZE,ozconf.dpMaxBufferSize);

  case PROP_DP_VERSION: return OZ_pair2(oz_int(PERDIOMAJOR),
					    oz_int(PERDIOMINOR));
  case PROP_DP_CLOCKTICK: return (oz_int(CLOCK_TICK/1000));

  CASE_BOOL(PROP_DP_USEALTVARPROTOCOL,ozconf.dpUseAltVarProtocol);
  CASE_REC(PROP_DP,"dp",
	   (22,oz_atomNoDup("useAltVarProtocol"),
	    oz_atomNoDup("seifHandler"),oz_atomNoDup("debug"),
	    oz_atomNoDup("flowBufferSize"),oz_atomNoDup("flowBufferTime"),
	    oz_atomNoDup("version"),oz_atomNoDup("retryTimeCeiling"),
	    oz_atomNoDup("retryTimeFloor"),oz_atomNoDup("retryTimeFactor"),
	    oz_atomNoDup("tcpHardLimit"),oz_atomNoDup("tcpWeakLimit"),
	    oz_atomNoDup("probeInterval"),oz_atomNoDup("probeTimeout"),
	    oz_atomNoDup("openTimeout"),oz_atomNoDup("closeTimeout"),
	    oz_atomNoDup("wfRemoteTimeout"),
	    oz_atomNoDup("firewallReopenTimeout"),
	    oz_atomNoDup("defaultBufferSize"),
	    oz_atomNoDup("defaultMaxBufferSize"),
	    oz_atomNoDup("bufferSize"),
	    oz_atomNoDup("maxBufferSize"),
	    oz_atomNoDup("clockTick")),
	   SET_BOOL(oz_atomNoDup("useAltVarProtocol"),
		    ozconf.dpUseAltVarProtocol);
	   SET_BOOL(oz_atomNoDup("seifHandler"), ozconf.dpSeifHandler);
	   SET_INT(oz_atomNoDup("debug"), ozconf.debugPerdio);
	   SET_INT(oz_atomNoDup("flowBufferSize"),ozconf.dpFlowBufferSize);
	   SET_INT(oz_atomNoDup("flowBufferTime"),ozconf.dpFlowBufferTime);
	   SET_INT(oz_atomNoDup("retryTimeCeiling"), 
		   ozconf.dpRetryTimeCeiling);
	   SET_INT(oz_atomNoDup("retryTimeFloor"),ozconf.dpRetryTimeFloor);
	   SET_INT(oz_atomNoDup("retryTimeFactor"), 
		   ozconf.dpRetryTimeFactor);
	   SET_INT(oz_atomNoDup("tcpHardLimit"), ozconf.dpTCPHardLimit);
	   SET_INT(oz_atomNoDup("tcpWeakLimit"), ozconf.dpTCPWeakLimit);
	   SET_INT(oz_atomNoDup("probeInterval"), 
		   ozconf.dpProbeInterval);
	   SET_INT(oz_atomNoDup("probeTimeout"), 
		   ozconf.dpProbeTimeout);
	   SET_INT(oz_atomNoDup("openTimeout"), 
		   ozconf.dpOpenTimeout);
	   SET_INT(oz_atomNoDup("closeTimeout"), 
		   ozconf.dpCloseTimeout);
	   SET_INT(oz_atomNoDup("wfRemoteTimeout"), 
		   ozconf.dpWFRemoteTimeout);
	   SET_INT(oz_atomNoDup("firewallReopenTimeout"), 
		   ozconf.dpFirewallReopenTimeout);
	   SET_INT(oz_atomNoDup("defaultBufferSize"),
		   ozconf.dpDefaultBufferSize);
	   SET_INT(oz_atomNoDup("defaultMaxBufferSize"),
		   ozconf.dpDefaultMaxBufferSize);
	   SET_INT(oz_atomNoDup("bufferSize"),
		   ozconf.dpBufferSize);
	   SET_INT(oz_atomNoDup("maxBufferSize"),
		   ozconf.dpMaxBufferSize);
	   SET_REC(oz_atomNoDup("version"), OZ_pair2(oz_int(PERDIOMAJOR),
						     oz_int(PERDIOMINOR)));
	   SET_INT(oz_atomNoDup("clockTick"), CLOCK_TICK/1000);
	   );
  CASE_INT(PROP_DPTABLE_DEFAULTOWNERTABLESIZE,
	   ozconf.dpTableDefaultOwnerTableSize);
  CASE_INT(PROP_DPTABLE_DEFAULTBORROWTABLESIZE,
	   (1<<ozconf.dpTableDefaultBorrowTableSize));
  CASE_INT(PROP_DPTABLE_LOWLIMIT, ozconf.dpTableLowLimit);
  CASE_INT(PROP_DPTABLE_EXPANDFACTOR, ozconf.dpTableExpandFactor);
  CASE_INT(PROP_DPTABLE_BUFFER, ozconf.dpTableBuffer);
  CASE_INT(PROP_DPTABLE_WORTHWHILEREALLOC, ozconf.dpTableWorthwhileRealloc);
  CASE_REC(PROP_DPTABLE,"dpTable",
	   (6,oz_atomNoDup("defaultOwnerTableSize"),
	    oz_atomNoDup("defaultBorrowTableSize"),
	    oz_atomNoDup("lowLimit"),oz_atomNoDup("expandFactor"),oz_atomNoDup("buffer"),
	    oz_atomNoDup("worthwhileRealloc")),
	   SET_INT(oz_atomNoDup("defaultOwnerTableSize"),
		   ozconf.dpTableDefaultOwnerTableSize);
	   SET_INT(oz_atomNoDup("defaultBorrowTableSize"),
		   (1<<ozconf.dpTableDefaultBorrowTableSize));
	   SET_INT(oz_atomNoDup("lowLimit"), ozconf.dpTableLowLimit);
	   SET_INT(oz_atomNoDup("expandFactor"), ozconf.dpTableExpandFactor);
	   SET_INT(oz_atomNoDup("buffer"), ozconf.dpTableBuffer);
	   SET_INT(oz_atomNoDup("worthwhileRealloc"), 
		   ozconf.dpTableWorthwhileRealloc);
	   );

  CASE_BOOL(PROP_DPLOG_CONNECTLOG,ozconf.dpLogConnectLog);
  CASE_BOOL(PROP_DPLOG_MESSAGELOG,ozconf.dpLogMessageLog);
  CASE_REC(PROP_DPLOG,"dpLog",
	   (2,oz_atomNoDup("connectLog"),oz_atomNoDup("messageLog")),
	   SET_BOOL(oz_atomNoDup("connectLog"),ozconf.dpLogConnectLog);
	   SET_BOOL(oz_atomNoDup("messageLog"),ozconf.dpLogMessageLog);
	   );
  
  CASE_BOOL(PROP_DPGC_TIMELEASE,ozconf.dpUseTimeLease);
  CASE_BOOL(PROP_DPGC_FRACWRC,ozconf.dpUseFracWRC);
  CASE_INT(PROP_DPGC_TL_LEASETIME,ozconf.dp_tl_leaseTime);
  CASE_INT(PROP_DPGC_WRC_ALPHA,ozconf.dp_wrc_alpha);
  CASE_INT(PROP_DPGC_TL_UPDATETIME,ozconf.dp_tl_updateTime);

  CASE_REC(PROP_DPGC,"dpGC",
	   (5,oz_atomNoDup("useTimeLease"),oz_atomNoDup("useWRC"),oz_atomNoDup("wrc_alpha"),oz_atomNoDup("tl_leaseTime"),oz_atomNoDup("tl_updateTime")),
	   SET_BOOL(oz_atomNoDup("useTimeLease"),ozconf.dpUseTimeLease);
	   SET_BOOL(oz_atomNoDup("useWRC"),ozconf.dpUseFracWRC);
	   SET_INT(oz_atomNoDup("wrc_alpha"),ozconf.dp_wrc_alpha);
	   SET_INT(oz_atomNoDup("tl_leaseTime"),ozconf.dp_tl_leaseTime);
	   SET_INT(oz_atomNoDup("tl_updateTime"),ozconf.dp_tl_updateTime);
	   );
  
  
  CASE_INT(PROP_CLOSE_TIME,ozconf.closetime);
  CASE_BOOL(PROP_OZ_STYLE_USE_FUTURES,ozconf.useFutures);
  default:
    return 0; // not readable. 0 ok because no OZ_Term==0
  }
}

#undef CASE_INT
#undef CASE_BOOL
#undef CASE_ATOM
#undef DEFINE_REC
#undef RETURN_REC
#undef SET_REC
#undef DO_REC
#undef CASE_REC
#undef SET_INT
#undef SET_BOOL

// Macros for manipulating the `val' argument of SetEmulatorProperty
// val has been DEREFed and there is also val_ptr and val_tag, and it
// is guaranteed that val is determined.
//
// Here we check that it is a boolean.  And we put its value in the
// local integer variable INT__

#define CHECK_BOOL				\
if      (oz_isTrue(val )) INT__ = 1;		\
else if (oz_isFalse(val)) INT__ = 0;		\
else oz_typeError(1,"Bool");

// Handle a particular indexed property P, check that the specified
// val is a boolean, and do something (presumably using variable INT__)
// CASE_BOOL(P,L) is a specialization to update location L

#define CASE_BOOL_DO(P,DO) case P: CHECK_BOOL; DO; return PROCEED;
#define CASE_BOOL(P,L) CASE_BOOL_DO(P,L=INT__);

// Check that the value is a non-negative small integer

#define CHECK_NAT				\
if (!oz_isSmallInt(val) ||			\
    (INT__=tagged2SmallInt(val))<0)		\
  oz_typeError(1,"Int>=0");

// Handle the case of indexed property P that should be an int>=0

#define CASE_NAT_DO(P,DO) case P: CHECK_NAT; DO; return PROCEED;
#define CASE_NAT(P,L) CASE_NAT_DO(P,L=INT__);

// Check that the value is an integer in [1..100], i.e. a percentage

#define CHECK_PERCENT				\
if (!oz_isSmallInt(val) ||			\
    (INT__=tagged2SmallInt(val))<1 ||		\
    (INT__>100))				\
  oz_typeError(1,"Int[1..100]");

// Handle the case of indexed property P that should a percentage

#define CASE_PERCENT_DO(P,DO) case P: CHECK_PERCENT; DO; return PROCEED;
#define CASE_PERCENT(P,L) CASE_PERCENT_DO(P,L=INT__);

// Check that the value is a record, if so untag it into REC__

#define CHECK_REC				\
if (!oz_isSRecord(val))			        \
{oz_typeError(1,"SRecord");}			\
else REC__=tagged2SRecord(val);

// Handle the case of an indexed property P that should be a record,
// and DO something (presumably using REC__)

#define CASE_REC(P,DO)				\
case P: { CHECK_REC; DO; return PROCEED; }

// Signal that feature F on the record value is not of the
// expected type T

#define BAD_FEAT(F,T)				\
return oz_raise(E_ERROR,E_SYSTEM,"putProperty",2,F,oz_atom(T));

// Lookup feature F.  If it exists, make sure that it is a
// determined integer, then DO something (presumably with INT__)
// Note: we are using the equivalence between OZ_Term and int
// to reuse variable INT__ both as the term which is the value
// of the feature and as the corresonding integer value obtained
// by untagging it.

#define DO_INT(F,DO)				\
INT__ = REC__->getFeature(F);			\
if (INT__) {					\
  DEREF(INT__,PTR__);			        \
  Assert(!oz_isRef(INT__));			\
  if (oz_isVarOrRef(INT__))  oz_suspendOnPtr(PTR__); \
  if (oz_isSmallInt(INT__)) {                   \
    INT__=tagged2SmallInt(INT__);		\
  } else if (oz_isBigInt(INT__)) {              \
    INT__=tagged2BigInt(INT__)->getInt();       \
  } else {                                      \
    BAD_FEAT(F,"Int");	                        \
  }                                             \
  DO;						\
}

// set location L to integer value on feature F
#define SET_INT(F,L) DO_INT(F,L=INT__);

// Feature F should be a boolean, then DO something
#define DO_BOOL(F,DO)					\
INT__ = REC__->getFeature(F);				\
if(INT__) {						\
  DEREF(INT__,PTR__);				        \
  Assert(!oz_isRef(INT__));				\
  if (oz_isVarOrRef(INT__)) oz_suspendOnPtr(PTR__);	\
  if (!oz_isLiteral(INT__)) BAD_FEAT(F,"Bool");		\
  if      (oz_isTrue(INT__)) INT__=1;			\
  else if (oz_isFalse(INT__)) INT__=0;			\
  else BAD_FEAT(F,"Bool");				\
  DO;							\
}

// set location L to boolean value on feature F
#define SET_BOOL(F,L) DO_BOOL(F,L=INT__);

// Feature F should be a non-negative integer, then DO something
#define DO_NAT(F,DO)				\
DO_INT(F,if (INT__<0) {oz_typeError(1,"Int>=0");}; DO);

// set location L to non-negative integer value on feature F
#define SET_NAT(F,L) DO_NAT(F,L=INT__);

// Feature F should be a percentage, then DO something
#define DO_PERCENT(F,DO)			\
DO_INT(F,if (INT__<1||INT__>100) {oz_typeError(1,"Int[1..100]");}; DO);

// set location L to percentage value on feature F
#define SET_PERCENT(F,L) DO_PERCENT(F,L=INT__);

// val is guaranteed to be determined and derefed
OZ_Return SetEmulatorProperty(EmulatorPropertyIndex prop,OZ_Term val) {
  DEREF(val,val_ptr);
    if (oz_isVar(val))
      return SUSPEND;
    
  int      INT__;
  SRecord* REC__;
  switch (prop) {
    // TIME
    CASE_BOOL(PROP_TIME_DETAILED,ozconf.timeDetailed);
    CASE_REC(PROP_TIME,SET_BOOL(AtomDetailed,ozconf.timeDetailed););
    // THREADS
    CASE_NAT_DO(PROP_THREADS_MIN,{
      ozconf.stackMinSize=INT__/TASKFRAMESIZE;});
    CASE_REC(PROP_THREADS,
	     DO_NAT(AtomMin,
		    ozconf.stackMinSize=INT__/TASKFRAMESIZE;););
    // PRIORITIES
    CASE_PERCENT(PROP_PRIORITIES_HIGH,ozconf.hiMidRatio);
    CASE_PERCENT(PROP_PRIORITIES_MEDIUM,ozconf.midLowRatio);
    CASE_REC(PROP_PRIORITIES,
	     SET_PERCENT(AtomHigh,ozconf.hiMidRatio);
	     SET_PERCENT(AtomMedium,ozconf.midLowRatio););
    // PICKLE
    CASE_BOOL(PROP_PICKLE_CELLS,ozconf.pickleCells);
    CASE_REC(PROP_PICKLE,
	     SET_BOOL(AtomCells,ozconf.pickleCells););
    // GC
    CASE_NAT_DO(PROP_GC_MIN,{
      ozconf.heapMinSize=INT__/KB;
    });
    CASE_PERCENT(PROP_GC_FREE,ozconf.heapFree);
    CASE_PERCENT(PROP_GC_TOLERANCE,ozconf.heapTolerance);
    CASE_NAT(PROP_GC_CODE_CYCLES,ozconf.codeGCcycles);
    CASE_BOOL(PROP_GC_ON,ozconf.gcFlag);
    CASE_REC(PROP_GC,
	     DO_NAT(AtomMin,ozconf.heapMinSize=INT__/KB);
	     SET_NAT(AtomCodeCycles,ozconf.codeGCcycles);
	     SET_PERCENT(AtomFree,ozconf.heapFree);
	     SET_PERCENT(AtomTolerance,ozconf.heapTolerance);
	     SET_BOOL(AtomOn,ozconf.gcFlag);
	     );
    // PRINT
    CASE_NAT(PROP_PRINT_WIDTH,ozconf.printWidth);
    CASE_NAT(PROP_PRINT_DEPTH,ozconf.printDepth);
    CASE_NAT(PROP_PRINT_FLOATPRECISION,ozconf.printFloatPrecision);
    CASE_BOOL(PROP_PRINT_SCIENTIFICFLOATS,ozconf.printScientificFloats);
    CASE_BOOL(PROP_PRINT_VERBOSE,ozconf.printVerbose);
    CASE_REC(PROP_PRINT,
	     SET_NAT(AtomWidth,ozconf.printWidth);
	     SET_NAT(AtomDepth,ozconf.printDepth);
	     SET_NAT(AtomFloatPrecision,ozconf.printFloatPrecision);
	     SET_BOOL(AtomScientificFloats,ozconf.printScientificFloats);
	     SET_BOOL(AtomVerbose,ozconf.printVerbose););
    // FD
    CASE_NAT_DO(PROP_FD_THRESHOLD,reInitFDs(INT__));
    CASE_REC(PROP_FD,
	     DO_NAT(AtomThreshold,reInitFDs(INT__)););
    // ERRORS
  case PROP_ERRORS_HANDLER: {
    if (!oz_isProcedure(val) || tagged2Const(val)->getArity()!=1) {
      oz_typeError(0,"Procedure/1");
    }
    
    am.setDefaultExceptionHdl(val);
    return PROCEED;
  }
    CASE_BOOL(PROP_ERRORS_DEBUG,ozconf.errorDebug);
    CASE_NAT(PROP_ERRORS_THREAD,ozconf.errorThreadDepth);
    CASE_NAT(PROP_ERRORS_WIDTH,ozconf.errorPrintWidth);
    CASE_NAT(PROP_ERRORS_DEPTH,ozconf.errorPrintDepth);
    CASE_REC(PROP_ERRORS,
	     SET_BOOL(AtomDebug,ozconf.errorDebug);
	     SET_NAT(AtomThread,ozconf.errorThreadDepth);
	     SET_NAT(AtomWidth,ozconf.errorPrintWidth);
	     SET_NAT(AtomDepth,ozconf.errorPrintDepth););
    // MESSAGES
    CASE_BOOL(PROP_MESSAGES_GC,ozconf.gcVerbosity);
    CASE_BOOL(PROP_MESSAGES_IDLE,ozconf.showIdleMessage);
    CASE_REC(PROP_MESSAGES,
	     SET_BOOL(AtomGC,ozconf.gcVerbosity);
	     SET_BOOL(AtomIdle,ozconf.showIdleMessage););
    // INTERNAL
    CASE_BOOL_DO(PROP_INTERNAL_DEBUG,
		 if (INT__) am.setdebugmode(OK);
		 else       am.setdebugmode(NO));
    CASE_BOOL_DO(PROP_INTERNAL_PROPLOCATION,
		 if (INT__) am.setPropagatorLocation(OK);
		 else       am.setPropagatorLocation(NO));
    CASE_BOOL(PROP_INTERNAL_SUSPENSION,ozconf.showSuspension);
    CASE_BOOL(PROP_INTERNAL_STOP,ozconf.stopOnToplevelFailure);
    CASE_NAT(PROP_INTERNAL_DEBUG_IP,ozconf.debugIP);
    CASE_REC(PROP_INTERNAL,
	     DO_BOOL(AtomDebug, am.setdebugmode((INT__)?OK:NO));
	     SET_BOOL(AtomShowSuspension,ozconf.showSuspension);
	     SET_BOOL(AtomStopOnToplevelFailure,ozconf.stopOnToplevelFailure);
	     SET_NAT(AtomDebugIP,ozconf.debugIP));


    CASE_NAT(PROP_DP_DEBUG,ozconf.debugPerdio);

    CASE_BOOL(PROP_DP_USEALTVARPROTOCOL,ozconf.dpUseAltVarProtocol);
    CASE_BOOL(PROP_DP_SEIFHANDLER,ozconf.dpSeifHandler);
    CASE_NAT(PROP_DP_FLOWBUFFERSIZE,ozconf.dpFlowBufferSize);
    CASE_NAT(PROP_DP_FLOWBUFFERTIME,ozconf.dpFlowBufferTime);
    CASE_NAT(PROP_DP_RETRYTIMECEILING,ozconf.dpRetryTimeCeiling);
    CASE_NAT(PROP_DP_RETRYTIMEFLOOR,ozconf.dpRetryTimeFloor);
    CASE_NAT(PROP_DP_RETRYTIMEFACTOR,ozconf.dpRetryTimeFactor);
    CASE_NAT_DO(PROP_DP_TCPHARDLIMIT,{
      ozconf.dpTCPHardLimit=INT__;
      changeTCPLimit();
    });
    CASE_NAT_DO(PROP_DP_TCPWEAKLIMIT,{
      ozconf.dpTCPWeakLimit=INT__;
      changeTCPLimit();
    });
    CASE_NAT(PROP_DP_PROBEINTERVAL,ozconf.dpProbeInterval);
    CASE_NAT(PROP_DP_PROBETIMEOUT,ozconf.dpProbeTimeout);
    CASE_NAT(PROP_DP_OPENTIMEOUT,ozconf.dpOpenTimeout);
    CASE_NAT(PROP_DP_CLOSETIMEOUT,ozconf.dpCloseTimeout);
    CASE_NAT(PROP_DP_WFREMOTETIMEOUT,ozconf.dpWFRemoteTimeout);
    CASE_NAT(PROP_DP_FIREWALLREOPENTIMEOUT,ozconf.dpFirewallReopenTimeout);
    CASE_NAT_DO(PROP_DP_BUFFERSIZE,{
      if (INT__ <= ozconf.dpMaxBufferSize)
	ozconf.dpBufferSize = INT__;
    });
    CASE_NAT_DO(PROP_DP_MAXBUFFERSIZE,{
      ozconf.dpMaxBufferSize = INT__;
      ozconf.dpBufferSize = min(ozconf.dpBufferSize, INT__);
    });

    // DP    
    CASE_REC(PROP_DP,
	     SET_NAT(AtomDebugPerdio,ozconf.debugPerdio);
	     SET_NAT(oz_atomNoDup("flowbuffersize"),ozconf.dpFlowBufferSize);
 	     SET_NAT(oz_atomNoDup("flowbuffertime"),ozconf.dpFlowBufferTime);
	     SET_NAT(oz_atomNoDup("seifHandler"),ozconf.dpSeifHandler);
	     SET_NAT(oz_atomNoDup("retryTimeCeiling"),
		      ozconf.dpRetryTimeCeiling);
	     SET_NAT(oz_atomNoDup("retryTimeFloor"),
		      ozconf.dpRetryTimeFloor);
	     SET_NAT(oz_atomNoDup("retryTimeFactor"),
		      ozconf.dpRetryTimeFactor);
	     SET_NAT(oz_atomNoDup("tcpHardLimit"),ozconf.dpTCPHardLimit);
	     SET_NAT(oz_atomNoDup("tcpweaklimit"),ozconf.dpTCPWeakLimit);
	     SET_NAT(oz_atomNoDup("probeInterval"),
		      ozconf.dpProbeInterval);
	     SET_NAT(oz_atomNoDup("probeTimeout"),
		      ozconf.dpProbeTimeout);
	     SET_NAT(oz_atomNoDup("openTimeout"),
		      ozconf.dpOpenTimeout);
	     SET_NAT(oz_atomNoDup("closeTimeout"),
		      ozconf.dpCloseTimeout);
	     SET_NAT(oz_atomNoDup("wfRemoteTimeout"),
		      ozconf.dpWFRemoteTimeout);
	     SET_NAT(oz_atomNoDup("firewallReopenTimeout"),
		      ozconf.dpFirewallReopenTimeout);
	     DO_NAT(oz_atomNoDup("bufferSize"),{
	       if (INT__ <= ozconf.dpMaxBufferSize)
		 ozconf.dpBufferSize = INT__;
	     });
	     DO_NAT(oz_atomNoDup("maxBufferSize"),{
	       ozconf.dpMaxBufferSize = INT__;
	       ozconf.dpBufferSize = min(ozconf.dpBufferSize, INT__);
	     });
	     SET_BOOL(oz_atomNoDup("useAltVarProtocol"),
		      ozconf.dpUseAltVarProtocol););
    // DPTABLE
    CASE_NAT_DO(PROP_DPTABLE_DEFAULTOWNERTABLESIZE,{
      ozconf.dpTableDefaultOwnerTableSize=INT__;
      am.setSFlag(StartGC);
      return BI_PREEMPT;});
    CASE_NAT_DO(PROP_DPTABLE_DEFAULTBORROWTABLESIZE,{
      ozconf.dpTableDefaultBorrowTableSize=log2ceiling(INT__);
      am.setSFlag(StartGC);
      return BI_PREEMPT;});      
    CASE_PERCENT_DO(PROP_DPTABLE_LOWLIMIT,{
      ozconf.dpTableLowLimit=INT__;
      am.setSFlag(StartGC);
      return BI_PREEMPT;});      
    CASE_NAT_DO(PROP_DPTABLE_EXPANDFACTOR,{
      ozconf.dpTableExpandFactor=INT__;
      am.setSFlag(StartGC);
      return BI_PREEMPT;});      
    CASE_NAT_DO(PROP_DPTABLE_BUFFER, {
      ozconf.dpTableBuffer=INT__;
      am.setSFlag(StartGC);
      return BI_PREEMPT;});      
    CASE_NAT_DO(PROP_DPTABLE_WORTHWHILEREALLOC,{
      ozconf.dpTableWorthwhileRealloc=INT__;
      am.setSFlag(StartGC);
      return BI_PREEMPT;});      
    CASE_REC(PROP_DPTABLE,
	     SET_NAT(oz_atomNoDup("defaultOwnerTableSize"), 
		   ozconf.dpTableDefaultOwnerTableSize);
	     DO_NAT(oz_atomNoDup("defaultBorrowTableSize"),{
	       ozconf.dpTableDefaultBorrowTableSize=log2ceiling(INT__);
	       am.setSFlag(StartGC);
	       return BI_PREEMPT;
	     });      
	     SET_NAT(oz_atomNoDup("lowLimit"), ozconf.dpTableLowLimit);
	     SET_NAT(oz_atomNoDup("expandFactor"),
		     ozconf.dpTableExpandFactor);
	     SET_NAT(oz_atomNoDup("buffer"), ozconf.dpTableBuffer);
	     SET_NAT(oz_atomNoDup("worthwhileRealloc"), 
		     ozconf.dpTableWorthwhileRealloc);
	     );

    CASE_BOOL(PROP_DPLOG_CONNECTLOG,ozconf.dpLogConnectLog);
    CASE_BOOL(PROP_DPLOG_MESSAGELOG,ozconf.dpLogMessageLog);
    CASE_REC(PROP_DPLOG,
	     SET_BOOL(oz_atomNoDup("connectLog"),ozconf.dpLogConnectLog);
	     SET_BOOL(oz_atomNoDup("messageLog"),ozconf.dpLogMessageLog);
	     );

    CASE_BOOL(PROP_DPGC_TIMELEASE,ozconf.dpUseTimeLease);
    CASE_BOOL(PROP_DPGC_FRACWRC,ozconf.dpUseFracWRC);
    CASE_NAT(PROP_DPGC_TL_LEASETIME,ozconf.dp_tl_leaseTime);
    CASE_NAT(PROP_DPGC_TL_UPDATETIME,ozconf.dp_tl_updateTime);
    CASE_NAT(PROP_DPGC_WRC_ALPHA, ozconf.dp_wrc_alpha);
    CASE_REC(PROP_DPGC,
	     SET_BOOL(oz_atomNoDup("useTimeLease"),ozconf.dpUseTimeLease);
	     SET_BOOL(oz_atomNoDup("useWRC"),ozconf.dpUseFracWRC);
	     SET_NAT(oz_atomNoDup("wrc_alpha"),ozconf.dp_wrc_alpha);
	     SET_NAT(oz_atomNoDup("tl_leaseTime"),ozconf.dp_tl_leaseTime);
	     SET_NAT(oz_atomNoDup("tl_updateTime"),ozconf.dp_tl_updateTime);
	     );
    
    CASE_NAT(PROP_CLOSE_TIME,ozconf.closetime);
    CASE_BOOL_DO(PROP_STANDALONE,ozconf.runningUnderEmacs=!INT__);
    CASE_BOOL(PROP_OZ_STYLE_USE_FUTURES,ozconf.useFutures);
default:
    return PROP__NOT__WRITABLE;
  }
}

// VIRTUAL PROPERTIES
//

// The idea is to make system information available through a
// dictionary-like interface, in a manner that is trivially extensible,
// and such that we avoid the proliferation of specialized builtins.
//
// A virtual property is an object with virtual functions get and set.
// get() returns an OZ_Term as the (current) value of the property.
// set(V) sets the value of the property to the value described by the
// OZ_Term V.  It returns an OZ_Return status that indicate how the
// operation fared: it could succeed, fail, suspend, or raise an error.
//
// Virtual properties will be recorded as foreign pointers in
// dictionary vprop_registry.  Emulator properties will enjoy an
// optimized representation: they will simply be represented as
// integers in vprop_registry, to be interpreted by GetEmulatorProperty
// and SetEmulatorProperty.

OZ_Term   VirtualProperty::get()        { Assert(0); return NameUnit; }
OZ_Return VirtualProperty::set(OZ_Term) { Assert(0); return FAILED  ; }

static OZ_Term vprop_registry;
OZ_Term system_registry;	// eventually make it static [TODO]

inline
void VirtualProperty::add(const char * s, const int p) {
  tagged2Dictionary(vprop_registry)->setArg(oz_atomNoDup(s),
					    makeTaggedSmallInt(p));
}

// in addition to the usual OZ_Return values, the following
// may also return PROP__NOT__READABLE and PROP__NOT__FOUND

OZ_Return GetProperty(TaggedRef k,TaggedRef& val)
{
  TaggedRef key = k;
  DEREF(key,key_ptr);
  Assert(!oz_isRef(key));
  if (oz_isVarOrRef(key)) oz_suspendOnPtr(key_ptr);
  if (!oz_isAtom(key)) oz_typeError(0,"Atom");
  OzDictionary* dict;
  TaggedRef entry;
  dict = tagged2Dictionary(vprop_registry);
  entry = dict->getArg(key);
  if (entry) 
    if (oz_isInt(entry)) {
      entry = GetEmulatorProperty((EmulatorPropertyIndex)
				  oz_IntToC(entry));
      if (entry) { val=entry; return PROCEED; }
      else return PROP__NOT__READABLE;
    } else {
      val = ((VirtualProperty*)
	     OZ_getForeignPointer(entry))->get();
      return PROCEED;
    }
  dict = tagged2Dictionary(system_registry);
  entry = dict->getArg(key);
  if (entry) {
    val=entry; return PROCEED;
  }
  return PROP__NOT__FOUND;
}

// in addition to the usual OZ_Return values, the following
// may also return PROP__NOT__WRITABLE and PROP__NOT__GLOBAL.

OZ_Return PutProperty(TaggedRef k,TaggedRef v)
{
  if (!oz_onToplevel()) return PROP__NOT__GLOBAL;
  TaggedRef key = k;
  DEREF(key,key_ptr);
  Assert(!oz_isRef(key));
  if (oz_isVarOrRef(key)) oz_suspendOnPtr(key_ptr);
  if (!oz_isAtom(key)) oz_typeError(0,"Atom");
  OzDictionary* dict;
  TaggedRef entry;
  dict = tagged2Dictionary(vprop_registry);
  entry = dict->getArg(key);
  if (entry)
    if (OZ_isInt(entry)) {
      return SetEmulatorProperty((EmulatorPropertyIndex)
				 oz_IntToC(entry),v);
    } else
      return ((VirtualProperty*)
	      OZ_getForeignPointer(entry))->set(v);
  dict = tagged2Dictionary(system_registry);
  dict->setArg(key,v);
  return PROCEED;
}

OZ_BI_define(BIgetProperty,1,1)
{
  OZ_declareTerm(0,key);
  OZ_Return status = GetProperty(key,OZ_out(0));
  if (status == PROP__NOT__READABLE)
    return oz_raise(E_ERROR,E_SYSTEM,"getProperty",1,key);
  else if (status == PROP__NOT__FOUND)
    return oz_raise(E_SYSTEM,E_SYSTEM,"getProperty",1,key);
  else return status;
} OZ_BI_end

OZ_BI_define(BIcondGetProperty,2,1)
{
  OZ_declareTerm(0,key);
  OZ_declareTerm(1,def);
  OZ_Return status = GetProperty(key,OZ_out(0));
  if (status == PROP__NOT__READABLE)
    return oz_raise(E_ERROR,E_SYSTEM,"condGetProperty",1,key);
  else if (status == PROP__NOT__FOUND)
    OZ_RETURN(def);
  else return status;
} OZ_BI_end

OZ_BI_define(BIputProperty,2,0)
{
  OZ_declareTerm(0,key);
  OZ_declareTerm(1,val);
  OZ_Return status = PutProperty(key,val);
  if (status == PROP__NOT__WRITABLE)
    return oz_raise(E_ERROR,E_SYSTEM,"putProperty",1,key);
  else if (status == PROP__NOT__GLOBAL)
    return oz_raise(E_ERROR,E_KERNEL,"globalState",
		    1,oz_atomNoDup("putProperty"));
  else return status;
} OZ_BI_end

struct prop_entry {
  const char * name;
  enum EmulatorPropertyIndex epi;
};
  
static const struct prop_entry prop_entries[] = {
  // THREADS
  {"threads.created",PROP_THREADS_CREATED},
  {"threads.runnable",PROP_THREADS_RUNNABLE},
  {"threads.min",PROP_THREADS_MIN},
  {"threads",PROP_THREADS},
  // PRIORITIES
  {"priorities.high",PROP_PRIORITIES_HIGH},
  {"priorities.medium",PROP_PRIORITIES_MEDIUM},
  {"priorities",PROP_PRIORITIES},
  // pickles;
  {"pickle.cells",PROP_PICKLE_CELLS},
  {"pickle",PROP_PICKLE},
  // TIME
  {"time.copy",PROP_TIME_COPY},
  {"time.gc",PROP_TIME_GC},
  {"time.propagate",PROP_TIME_PROPAGATE},
  {"time.run",PROP_TIME_RUN},
  {"time.system",PROP_TIME_SYSTEM},
  {"time.total",PROP_TIME_TOTAL},
  {"time.user",PROP_TIME_USER},
  {"time.idle",PROP_TIME_IDLE},
  {"time.detailed",PROP_TIME_DETAILED},
  {"time",PROP_TIME},
  // GC
  {"gc.min",PROP_GC_MIN},
  {"gc.free",PROP_GC_FREE},
  {"gc.tolerance",PROP_GC_TOLERANCE},
  {"gc.on",PROP_GC_ON},
  {"gc.codeCycles",PROP_GC_CODE_CYCLES},
  {"gc.threshold",PROP_GC_THRESHOLD},
  {"gc.size",PROP_GC_SIZE},
  {"gc.active",PROP_GC_ACTIVE},
  {"gc",PROP_GC},
  // PRINT
  {"print.depth",PROP_PRINT_DEPTH},
  {"print.width",PROP_PRINT_WIDTH},
  {"print.floatPrecision",PROP_PRINT_FLOATPRECISION},
  {"print.scientificFloats",PROP_PRINT_SCIENTIFICFLOATS},
  {"print.verbose",PROP_PRINT_VERBOSE},
  {"print",PROP_PRINT},
  // FD
  {"fd.variables",PROP_FD_VARIABLES},
  {"fd.propagators",PROP_FD_PROPAGATORS},
  {"fd.invoked",PROP_FD_INVOKED},
  {"fd.threshold",PROP_FD_THRESHOLD},
  {"fd",PROP_FD},
  // SPACES
  {"spaces.committed",PROP_SPACES_COMMITTED},
  {"spaces.cloned",PROP_SPACES_CLONED},
  {"spaces.created",PROP_SPACES_CREATED},
  {"spaces.failed",PROP_SPACES_FAILED},
  {"spaces.succeeded",PROP_SPACES_SUCCEEDED},
  {"spaces",PROP_SPACES},
  // ERRORS
  {"errors.handler",PROP_ERRORS_HANDLER},
  {"errors.debug",PROP_ERRORS_DEBUG},
  {"errors.thread",PROP_ERRORS_THREAD},
  {"errors.depth",PROP_ERRORS_DEPTH},
  {"errors.width",PROP_ERRORS_WIDTH},
  {"errors",PROP_ERRORS},
  // MESSAGES
  {"messages.gc",PROP_MESSAGES_GC},
  {"messages.idle",PROP_MESSAGES_IDLE},
  {"messages",PROP_MESSAGES},
  // MEMORY
  {"memory.atoms",PROP_MEMORY_ATOMS},
  {"memory.names",PROP_MEMORY_NAMES},
  {"memory.freelist",PROP_MEMORY_FREELIST},
  {"memory.code",PROP_MEMORY_CODE},
  {"memory.heap",PROP_MEMORY_HEAP},
  {"memory",PROP_MEMORY},
  // LIMITS
  {"limits.int.min",PROP_LIMITS_INT_MIN},
  {"limits.int.max",PROP_LIMITS_INT_MAX},
  {"limits.bytecode.xregisters",
		       PROP_LIMITS_BYTECODE_XREGISTERS},
  {"limits",PROP_LIMITS},
  // APPLICATION
  {"application.args",PROP_APPLICATION_ARGS},
  {"application.url",PROP_APPLICATION_URL},
  {"application.gui",PROP_APPLICATION_GUI},
  {"application",PROP_APPLICATION},
  // PLATFORM
  {"platform.name", PROP_PLATFORM_NAME},
  {"platform.os",   PROP_PLATFORM_OS},
  {"platform.arch", PROP_PLATFORM_ARCH},
  {"platform",      PROP_PLATFORM},
  // MISC
  {"oz.standalone",PROP_STANDALONE},
  {"oz.configure.home",PROP_OZ_CONFIGURE_HOME},
  {"oz.emulator.home",PROP_OZ_EMULATOR_HOME},
  {"oz.version",PROP_OZ_VERSION},
  {"oz.date",PROP_OZ_DATE},
  // Suspending on SimpleVars raises an exception
  {"oz.style.useFutures",PROP_OZ_STYLE_USE_FUTURES},
  // Distribution
  {"distribution.virtualsites",
		       PROP_DISTRIBUTION_VIRTUALSITES},
  // Marshaler
  {"marshaler.version", PROP_MARSHALER_VERSION},
  // INTERNAL
  {"internal",PROP_INTERNAL},
  {"internal.debug",PROP_INTERNAL_DEBUG},
  {"internal.propLocation",PROP_INTERNAL_PROPLOCATION},
  {"internal.suspension",PROP_INTERNAL_SUSPENSION},
  {"internal.stop",PROP_INTERNAL_STOP},
  {"internal.engineSusps",PROP_INTERNAL_ENGINE_SUSP},
  {"internal.ip.debug",PROP_INTERNAL_DEBUG_IP},
  // DP
  {"dp.debug",PROP_DP_DEBUG},
  {"dp.useAltVarProtocol", 
		       PROP_DP_USEALTVARPROTOCOL},
  {"dp.version",PROP_DP_VERSION},
  {"dp.clockTick",PROP_DP_CLOCKTICK},
  {"dp.flowBufferSize",PROP_DP_FLOWBUFFERSIZE},
  {"dp.flowBufferTime",PROP_DP_FLOWBUFFERTIME},
  {"dp.seifHandler",PROP_DP_SEIFHANDLER},
  {"dp.retryTimeCeiling",PROP_DP_RETRYTIMECEILING},
  {"dp.retryTimeFloor",PROP_DP_RETRYTIMEFLOOR},
  {"dp.retryTimeFactor",PROP_DP_RETRYTIMEFACTOR},
  {"dp.tcpHardLimit",PROP_DP_TCPHARDLIMIT},
  {"dp.tcpWeakLimit",PROP_DP_TCPWEAKLIMIT},
  {"dp.probeInterval",PROP_DP_PROBEINTERVAL},
  {"dp.probeTimeout",PROP_DP_PROBETIMEOUT},
  {"dp.openTimeout",PROP_DP_OPENTIMEOUT},
  {"dp.closeTimeout",PROP_DP_CLOSETIMEOUT},
  {"dp.wfRemoteTimeout",PROP_DP_WFREMOTETIMEOUT},
  {"dp.firewallReopenTimeout",PROP_DP_FIREWALLREOPENTIMEOUT},
  {"dp.defaultBufferSize",PROP_DP_DEFAULTBUFFERSIZE},
  {"dp.defaultMaxBufferSize",PROP_DP_DEFAULTMAXBUFFERSIZE},
  {"dp.bufferSize",PROP_DP_BUFFERSIZE},
  {"dp.maxBufferSize",PROP_DP_MAXBUFFERSIZE},
  {"dp",PROP_DP},
  // DPTABLE
  {"dpTable.defaultOwnerTableSize",
		       PROP_DPTABLE_DEFAULTOWNERTABLESIZE},
  {"dpTable.defaultBorrowTableSize",
		       PROP_DPTABLE_DEFAULTBORROWTABLESIZE},
  {"dpTable.lowLimit", PROP_DPTABLE_LOWLIMIT},
  {"dpTable.expandFactor", PROP_DPTABLE_EXPANDFACTOR},
  {"dpTable.buffer", PROP_DPTABLE_BUFFER},
  {"dpTable.worthwhileRealloc",
		       PROP_DPTABLE_WORTHWHILEREALLOC},
  {"dpTable", PROP_DPTABLE},
  // DPLOG
  {"dpLog.connectLog",PROP_DPLOG_CONNECTLOG},
  {"dpLog.messageLog",PROP_DPLOG_MESSAGELOG},
  {"dpLog",PROP_DPLOG},
  // DPGC
  {"dpGC.useTimeLease",PROP_DPGC_TIMELEASE },
  {"dpGC.useWRC",PROP_DPGC_FRACWRC},
  {"dpGC.wrc_alpha",PROP_DPGC_WRC_ALPHA},
  {"dpGC.tl_leaseTime",PROP_DPGC_TL_LEASETIME},
  {"dpGC.tl_updateTime",PROP_DPGC_TL_UPDATETIME},
  {"dpGC",PROP_DPGC},
  
  //CLOSE
  {"close.time",PROP_CLOSE_TIME},
  {0,PROP__LAST},
};

void initVirtualProperties()
{
  vprop_registry  = makeTaggedConst(new OzDictionary(oz_rootBoard()));
  system_registry = makeTaggedConst(new OzDictionary(oz_rootBoard()));
  OZ_protect(&vprop_registry);
  OZ_protect(&system_registry);
  // POPULATE THE SYSTEM REGISTRY
  {
    OzDictionary * dict = tagged2Dictionary(system_registry);
    dict->setArg(oz_atomNoDup("oz.home"),oz_atom(ozconf.ozHome));
  }
  for (const struct prop_entry * pe = prop_entries; pe->name; pe++)
    VirtualProperty::add(pe->name,pe->epi);
}




