// Copyright © by Denys Duchier, Feb 1998, Universität des Saarlandes
//
// EMULATOR PROPERTIES
//
// We define here a single interface to all emulator properties.  This
// is intended to put an end to the proliferation of specialized builtins.
// It will also permit an optimized representation of emulator properties
// as Virtual Properties (see later): as ints rather than as instances of
// class VirtualProperty.
//

#include "runtime.hh"
#include "dictionary.hh"
#include "vprops.hh"

enum EmulatorPropertyIndex {
  // THREADS
  PROP_THREADS_CREATED,
  PROP_THREADS_RUNNABLE,
  PROP_THREADS_MIN,
  PROP_THREADS_MAX,
  // PRIORITIES
  PROP_PRIORITIES_HIGH,
  PROP_PRIORITIES_MEDIUM,
  // TIME
  PROP_TIME_COPY,
  PROP_TIME_GC,
  PROP_TIME_LOAD,
  PROP_TIME_PROPAGATE,
  PROP_TIME_RUN,
  PROP_TIME_SYSTEM,
  PROP_TIME_TOTAL,
  PROP_TIME_USER,
  PROP_TIME_DETAILED,
  // GC
  PROP_GC_MIN,
  PROP_GC_MAX,
  PROP_GC_FREE,
  PROP_GC_TOLERANCE,
  PROP_GC_ON,
  PROP_GC_THRESHOLD,
  PROP_GC_SIZE,
  PROP_GC_ACTIVE,
  // PRINT
  PROP_PRINT_DEPTH,
  PROP_PRINT_WIDTH,
  // FD
  PROP_FD_VARIABLES,
  PROP_FD_PROPAGATORS,
  PROP_FD_INVOKED,
  PROP_FD_THRESHOLD,
  // SPACES
  PROP_SPACES_COMMITTED,
  PROP_SPACES_CLONED,
  PROP_SPACES_CREATED,
  PROP_SPACES_FAILED,
  PROP_SPACES_SUCCEEDED,
  // ERRORS
  PROP_ERRORS_LOCATION,
  PROP_ERRORS_DEBUG,
  PROP_ERRORS_HINTS,
  PROP_ERRORS_THREAD,
  PROP_ERRORS_DEPTH,
  PROP_ERRORS_WIDTH,
  // MESSAGES
  PROP_MESSAGES_GC,
  PROP_MESSAGES_IDLE,
  PROP_MESSAGES_FEED,
  PROP_MESSAGES_FOREIGN,
  PROP_MESSAGES_LOAD,
  PROP_MESSAGES_CACHE,
  // MEMORY
  PROP_MEMORY_ATOMS,
  PROP_MEMORY_NAMES,
  PROP_MEMORY_BUILTINS,
  PROP_MEMORY_FREELIST,
  PROP_MEMORY_CODE,
  PROP_MEMORY_HEAP,
  // HEAP
  PROP_HEAP_USED,
  // LIMITS
  PROP_LIMITS_INT_MIN,
  PROP_LIMITS_INT_MAX,
  // ARGV
  PROP_ARGV,
  // MISC
  PROP_STANDALONE,
  PROP_HOME,
  PROP_OS_NAME,
  PROP_OS_CPU,

  // INTERNAL
  PROP_INTERNAL_DEBUG,
  PROP_INTERNAL_SUSPENSION,
  PROP_INTERNAL_STOP,
  PROP_INTERNAL_DEBUG_IP,
  PROP_INTERNAL_DEBUG_PERDIO,
  PROP_INTERNAL_BROWSER,
  PROP_INTERNAL_APPLET,
  // this must remain last
  PROP__LAST
};

inline OZ_Term OZ_bool(int i) { return i?NameTrue:NameFalse; }

#define CASE_INT( P,L) case P: return OZ_int( L)
#define CASE_BOOL(P,L) case P: return OZ_bool(L)
#define CASE_ATOM(P,L) case P: return OZ_atom(L)

OZ_Term GetEmulatorProperty(EmulatorPropertyIndex prop) {
  switch (prop) {
    // THREADS
    CASE_INT(PROP_THREADS_CREATED,ozstat.createdThreads.total);
    CASE_INT(PROP_THREADS_RUNNABLE,am.getRunnableNumber());
    CASE_INT(PROP_THREADS_MIN,ozconf.stackMinSize / TASKFRAMESIZE);
    CASE_INT(PROP_THREADS_MAX,ozconf.stackMaxSize / TASKFRAMESIZE);
    // PRIORITIES
    CASE_INT(PROP_PRIORITIES_HIGH,ozconf.hiMidRatio);
    CASE_INT(PROP_PRIORITIES_MEDIUM,ozconf.midLowRatio);
    // TIME
    CASE_INT(PROP_TIME_COPY,ozconf.timeDetailed && ozstat.timeForCopy.total);
    CASE_INT(PROP_TIME_GC,ozconf.timeDetailed && ozstat.timeForGC.total);
    CASE_INT(PROP_TIME_LOAD,ozconf.timeDetailed && ozstat.timeForLoading.total);
    CASE_INT(PROP_TIME_PROPAGATE,ozconf.timeDetailed && ozstat.timeForPropagation.total);
    CASE_INT(PROP_TIME_RUN,ozconf.timeDetailed &&
             (osUserTime() - (ozstat.timeForCopy.total +
                              ozstat.timeForGC.total +
                              ozstat.timeForLoading.total +
                              ozstat.timeForPropagation.total)));
    CASE_INT(PROP_TIME_SYSTEM,osSystemTime());
    CASE_INT(PROP_TIME_TOTAL,osTotalTime());
    CASE_INT(PROP_TIME_USER,osUserTime());
    CASE_BOOL(PROP_TIME_DETAILED,ozconf.timeDetailed);
    // GC
    CASE_INT(PROP_GC_MIN,ozconf.heapMinSize*KB);
    CASE_INT(PROP_GC_MAX,ozconf.heapMaxSize*KB);
    CASE_INT(PROP_GC_FREE,ozconf.heapFree);
    CASE_INT(PROP_GC_TOLERANCE,ozconf.heapTolerance);
    CASE_BOOL(PROP_GC_ON,ozconf.gcFlag);
    CASE_INT(PROP_GC_THRESHOLD,ozconf.heapThreshold*KB);
    CASE_INT(PROP_GC_SIZE,getUsedMemory()*KB);
    CASE_INT(PROP_GC_ACTIVE,ozstat.gcLastActive*KB);
    // PRINT
    CASE_INT(PROP_PRINT_WIDTH,ozconf.printDepth);
    CASE_INT(PROP_PRINT_DEPTH,ozconf.printWidth);
    // FD
    CASE_INT(PROP_FD_VARIABLES,ozstat.fdvarsCreated.total);
    CASE_INT(PROP_FD_PROPAGATORS,ozstat.propagatorsCreated.total);
    CASE_INT(PROP_FD_INVOKED,ozstat.propagatorsInvoked.total);
    CASE_INT(PROP_FD_THRESHOLD,32 * fd_bv_max_high);
    // SPACES
    CASE_INT(PROP_SPACES_COMMITTED,ozstat.solveAlt.total);
    CASE_INT(PROP_SPACES_CLONED,ozstat.solveCloned.total);
    CASE_INT(PROP_SPACES_CREATED,ozstat.solveCreated.total);
    CASE_INT(PROP_SPACES_FAILED,ozstat.solveFailed.total);
    CASE_INT(PROP_SPACES_SUCCEEDED,ozstat.solveSolved.total);
    // ERRORS
    CASE_BOOL(PROP_ERRORS_LOCATION,ozconf.errorLocation);
    CASE_BOOL(PROP_ERRORS_DEBUG,ozconf.errorDebug);
    CASE_BOOL(PROP_ERRORS_HINTS,ozconf.errorHints);
    CASE_INT(PROP_ERRORS_THREAD,ozconf.errorThreadDepth);
    CASE_INT(PROP_ERRORS_DEPTH,ozconf.errorPrintDepth);
    CASE_INT(PROP_ERRORS_WIDTH,ozconf.errorPrintWidth);
    // MESSAGES
    CASE_BOOL(PROP_MESSAGES_GC,ozconf.gcVerbosity);
    CASE_BOOL(PROP_MESSAGES_IDLE,ozconf.showIdleMessage);
    CASE_BOOL(PROP_MESSAGES_FEED,ozconf.showFastLoad);
    CASE_BOOL(PROP_MESSAGES_FOREIGN,ozconf.showForeignLoad);
    CASE_BOOL(PROP_MESSAGES_LOAD,ozconf.showLoad);
    CASE_BOOL(PROP_MESSAGES_CACHE,ozconf.showCacheLoad);
    // MEMORY
    CASE_INT(PROP_MEMORY_ATOMS,ozstat.getAtomMemory());
    CASE_INT(PROP_MEMORY_NAMES,ozstat.getNameMemory());
    CASE_INT(PROP_MEMORY_BUILTINS,builtinTab.memRequired());
    CASE_INT(PROP_MEMORY_FREELIST,getMemoryInFreeList());
    CASE_INT(PROP_MEMORY_CODE,CodeArea::totalSize);
    CASE_INT(PROP_MEMORY_HEAP,ozstat.heapUsed.total+getUsedMemory());
    // HEAP
    CASE_INT(PROP_HEAP_USED,getUsedMemory());
    // LIMITS
    CASE_INT(PROP_LIMITS_INT_MIN,OzMinInt);
    CASE_INT(PROP_LIMITS_INT_MAX,OzMaxInt);
    // ARGV
  case PROP_ARGV:
    {
      TaggedRef out = nil();
      for(int i=ozconf.argC-1; i>=0; i--)
        out = cons(oz_atom(ozconf.argV[i]),out);
      return out;
    }
  CASE_BOOL(PROP_STANDALONE,ozconf.runningUnderEmacs);
  CASE_ATOM(PROP_HOME,ozconf.ozHome);
  CASE_ATOM(PROP_OS_NAME,ozconf.osname);
  CASE_ATOM(PROP_OS_CPU,ozconf.cpu);
  // INTERNAL
  CASE_BOOL(PROP_INTERNAL_DEBUG,am.isSetSFlag(DebugMode));
  CASE_BOOL(PROP_INTERNAL_SUSPENSION,ozconf.showSuspension);
  CASE_BOOL(PROP_INTERNAL_STOP,ozconf.stopOnToplevelFailure);
  CASE_INT(PROP_INTERNAL_DEBUG_IP,ozconf.debugIP);
  CASE_INT(PROP_INTERNAL_DEBUG_PERDIO,ozconf.debugPerdio);
  CASE_BOOL(PROP_INTERNAL_BROWSER,ozconf.browser);
  CASE_BOOL(PROP_INTERNAL_APPLET,ozconf.applet);

  default:
    return 0; // not readable. 0 ok because no OZ_Term==0
  }
}

#undef CASE_BOOL
#undef CASE_INT
#undef CASE_ATOM

#define CHECK_BOOL                      \
if (OZ_isTrue(val)) VAL = 1;            \
else if (OZ_isFalse(val)) VAL = 0;      \
else oz_typeError(1,"Bool")

#define CASE_BOOL_DO(P,DO) case P: CHECK_BOOL; DO   ; return PROCEED;
#define CASE_BOOL(P,L)     case P: CHECK_BOOL; L=VAL; return PROCEED;

#define CHECK_NAT                       \
if (!OZ_isInt(val) && (VAL=OZ_intToC(val))<0) oz_typeError(1,"Int>=0")

#define CASE_NAT(P,L)     case P: CHECK_NAT; L=VAL; return PROCEED;
#define CASE_NAT_DO(P,DO) case P: CHECK_NAT; DO   ; return PROCEED;

#define CHECK_PERCENT                   \
CHECK_NAT; if (VAL<1 || VAL>100) oz_typeError(1,"Int[1..100]");

#define CASE_PERCENT(P,L) case P: CHECK_PERCENT; L=VAL; return PROCEED;

// val is guaranteed to be determined and derefed
OZ_Return SetEmulatorProperty(EmulatorPropertyIndex prop,OZ_Term val) {
  int VAL;
  switch (prop) {
    // TIME
    CASE_BOOL(PROP_TIME_DETAILED,ozconf.timeDetailed);
    // THREADS
    CASE_NAT_DO(PROP_THREADS_MIN,{
      ozconf.stackMinSize=VAL/TASKFRAMESIZE;
      if (ozconf.stackMinSize > ozconf.stackMaxSize)
        ozconf.stackMaxSize = ozconf.stackMinSize;});
    CASE_NAT_DO(PROP_THREADS_MAX,{
      ozconf.stackMaxSize=VAL/TASKFRAMESIZE;
      if (ozconf.stackMinSize > ozconf.stackMaxSize)
        ozconf.stackMinSize = ozconf.stackMaxSize;});
    // PRIORITIES
    CASE_PERCENT(PROP_PRIORITIES_HIGH,ozconf.hiMidRatio);
    CASE_PERCENT(PROP_PRIORITIES_MEDIUM,ozconf.midLowRatio);
    // GC
    CASE_NAT_DO(PROP_GC_MAX,{
      ozconf.heapMaxSize=VAL/KB;
      if (ozconf.heapMinSize > ozconf.heapMaxSize)
        ozconf.heapMinSize = ozconf.heapMaxSize;
      if (ozconf.heapThreshold > ozconf.heapMaxSize) {
        am.setSFlag(StartGC);
        return BI_PREEMPT;}});
    CASE_NAT_DO(PROP_GC_MIN,{
      ozconf.heapMinSize=VAL/KB;
      if (ozconf.heapMinSize > ozconf.heapMaxSize)
        ozconf.heapMaxSize = ozconf.heapMinSize;
      if (ozconf.heapMinSize > ozconf.heapThreshold)
        ozconf.heapThreshold = ozconf.heapMinSize;
      if (ozconf.heapThreshold > ozconf.heapMaxSize) {
        am.setSFlag(StartGC);
        return BI_PREEMPT;}});
    CASE_PERCENT(PROP_GC_FREE,ozconf.heapFree);
    CASE_PERCENT(PROP_GC_TOLERANCE,ozconf.heapTolerance);
    CASE_BOOL(PROP_GC_ON,ozconf.gcFlag);

    // PRINT
    CASE_NAT(PROP_PRINT_WIDTH,ozconf.printWidth);
    CASE_NAT(PROP_PRINT_DEPTH,ozconf.printDepth);
    // FD
    CASE_NAT_DO(PROP_FD_THRESHOLD,reInitFDs(VAL));
    // ERRORS
    CASE_BOOL(PROP_ERRORS_LOCATION,ozconf.errorLocation);
    CASE_BOOL(PROP_ERRORS_HINTS,ozconf.errorHints);
    CASE_BOOL(PROP_ERRORS_DEBUG,ozconf.errorDebug);
    CASE_NAT(PROP_ERRORS_THREAD,ozconf.errorThreadDepth);
    CASE_NAT(PROP_ERRORS_WIDTH,ozconf.errorPrintWidth);
    CASE_NAT(PROP_ERRORS_DEPTH,ozconf.errorPrintDepth);
    // MESSAGES
    CASE_BOOL(PROP_MESSAGES_GC,ozconf.gcVerbosity);
    CASE_BOOL(PROP_MESSAGES_IDLE,ozconf.showIdleMessage);
    CASE_BOOL(PROP_MESSAGES_FEED,ozconf.showFastLoad);
    CASE_BOOL(PROP_MESSAGES_FOREIGN,ozconf.showForeignLoad);
    CASE_BOOL(PROP_MESSAGES_LOAD,ozconf.showLoad);
    CASE_BOOL(PROP_MESSAGES_CACHE,ozconf.showCacheLoad);
    // INTERNAL
    CASE_BOOL_DO(PROP_INTERNAL_DEBUG,if (VAL) am.unsetSFlag(DebugMode); else am.setSFlag(DebugMode));
    CASE_BOOL(PROP_INTERNAL_SUSPENSION,ozconf.showSuspension);
    CASE_BOOL(PROP_INTERNAL_STOP,ozconf.stopOnToplevelFailure);
    CASE_NAT(PROP_INTERNAL_DEBUG_IP,ozconf.debugIP);
    CASE_NAT(PROP_INTERNAL_DEBUG_PERDIO,ozconf.debugPerdio);
    CASE_BOOL(PROP_INTERNAL_BROWSER,ozconf.browser);
    CASE_BOOL(PROP_INTERNAL_APPLET,ozconf.applet);
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

static OZ_Term  vprop_registry;
static OZ_Term system_registry;

void VirtualProperty::add(char*s,EmulatorPropertyIndex p) {
  tagged2Dictionary(vprop_registry)->setArg(oz_atom(s),OZ_int(p));
}

// in addition to the usual OZ_Return values, the following
// may also return PROP__NOT__READABLE and PROP__NOT__FOUND

OZ_Return GetProperty(TaggedRef k,TaggedRef val)
{
  TaggedRef key = k; SAFE_DEREF(key);
  if (OZ_isVariable(key)) OZ_suspendOn(key);
  if (!OZ_isAtom(key)) oz_typeError(0,"Atom");
  OzDictionary* dict;
  TaggedRef entry;
  dict = tagged2Dictionary(vprop_registry);
  if (dict->getArg(key,entry)==PROCEED)
    if (OZ_isInt(entry)) {
      entry = GetEmulatorProperty(OZ_intToC(entry));
      return entry?oz_unify(val,entry):PROP__NOT__READABLE;
    } else
      return oz_unify(val,((VirtualProperty*)
                           OZ_getForeignPointer(entry))->get());
  dict = tagged2Dictionary(system_registry);
  if (dict->getArg(key,entry)==PROCEED)
    return oz_unify(val,entry);
  return PROP__NOT__FOUND;
}

// in addition to the usual OZ_Return values, the following
// may also return PROP__NOT__WRITABLE and PROP__NOT__GLOBAL.

OZ_Return PutProperty(TaggedRef k,TaggedRef v)
{
  if (!am.onToplevel()) return PROP__NOT__GLOBAL;
  TaggedRef key = k; SAFE_DEREF(key);
  if (OZ_isVariable(key)) OZ_suspendOn(key);
  if (!OZ_isAtom(key)) oz_typeError(0,"Atom");
  TaggedRef val = v; SAFE_DEREF(val);
  OzDictionary* dict;
  TaggedRef entry;
  dict = tagged2Dictionary(vprop_registry);
  if (dict->getArg(key,entry)==PROCEED)
    if (OZ_isInt(entry)) {
      // Emulator properties must be determined
      if (OZ_isVariable(val)) OZ_suspendOn(val);
      return SetEmulatorProperty(OZ_intToC(entry),val);
    } else
      return ((VirtualProperty*)
              OZ_getForeignPointer(entry))->set(val);
  dict = tagged2Dictionary(system_registry);
  dict->setArg(key,val);
  return PROCEED;
}

OZ_C_proc_begin(BIgetProperty,2)
{
  OZ_declareArg(0,key);
  OZ_declareArg(1,val);
  OZ_Return status = GetProperty(key,val);
  if (status == PROP__NOT__READABLE)
    return oz_raise(E_ERROR,E_SYSTEM,"getProperty",1,key);
  else if (status == PROP__NOT__FOUND)
    return oz_raise(E_SYSTEM,E_KERNEL,"getProperty",1,key);
  else return status;
}
OZ_C_proc_end

OZ_C_proc_begin(BIcondGetProperty,3)
{
  OZ_declareArg(0,key);
  OZ_declareArg(0,def);
  OZ_declareArg(2,val);
  OZ_Return status = GetProperty(key,val);
  if (status == PROP__NOT__READABLE)
    return oz_raise(E_ERROR,E_KERNEL,"condGetProperty",1,key);
  else if (status == PROP__NOT__FOUND)
    return oz_unif(def,val);
  else return status;
}
OZ_C_proc_end

OZ_C_proc_begin(BIputProperty,2)
{
  OZ_declareArg(0,key);
  OZ_declareArg(1,val);
  OZ_Return status = PutProperty(key,val);
  if (status == PROP__NOT__WRITABLE)
    return oz_raise(E_ERROR,E_KERNEL,"putProperty",1,key)
  else if (status == PROP__NOT__GLOBAL)
    return oz_raise(E_ERROR,E_KERNEL,"globalState",
                    1,oz_atom("putProperty"));
  else return status;
}
OZ_C_proc_end

static BIspec vpropSpecs[] = {
  {"GetProperty"    , 2, BIgetProperty    , 0},
  {"CondGetProperty", 3, BIcondGetProperty, 0},
  {"PutProperty"    , 2, BIputProperty    , 0},
  {0,0,0,0},
};

void initVirtualProperties()
{
  vprop_registry  = makeTaggedConst(new OzDictionary(ozx_rootBoard()));
  system_registry = makeTaggedConst(new OzDictionary(ozx_rootBoard()));
  OZ_protect(vprop_registry);
  OZ_protect(system_registry);
  BIaddSpec(vpropSpecs);
  // THREADS
  VirtualProperty::add("threads.created",PROP_THREADS_CREATED);
  VirtualProperty::add("threads.runnable",PROP_THREADS_RUNNABLE);
  VirtualProperty::add("threads.min",PROP_THREADS_MIN);
  VirtualProperty::add("threads.max",PROP_THREADS_MAX);
  // PRIORITIES
  VirtualProperty::add("priorities.high",PROP_PRIORITIES_HIGH);
  VirtualProperty::add("priorities.medium",PROP_PRIORITIES_MEDIUM);
  // TIME
  VirtualProperty::add("time.copy",PROP_TIME_COPY);
  VirtualProperty::add("time.gc",PROP_TIME_GC);
  VirtualProperty::add("time.load",PROP_TIME_LOAD);
  VirtualProperty::add("time.propagate",PROP_TIME_PROPAGATE);
  VirtualProperty::add("time.run",PROP_TIME_RUN);
  VirtualProperty::add("time.system",PROP_TIME_SYSTEM);
  VirtualProperty::add("time.total",PROP_TIME_TOTAL);
  VirtualProperty::add("time.user",PROP_TIME_USER);
  VirtualProperty::add("time.detailed",PROP_TIME_DETAILED);
  // GC
  VirtualProperty::add("gc.min",PROP_GC_MIN);
  VirtualProperty::add("gc.max",PROP_GC_MAX);
  VirtualProperty::add("gc.free",PROP_GC_FREE);
  VirtualProperty::add("gc.tolerance",PROP_GC_TOLERANCE);
  VirtualProperty::add("gc.on",PROP_GC_ON);
  VirtualProperty::add("gc.threshold",PROP_GC_THRESHOLD);
  VirtualProperty::add("gc.size",PROP_GC_SIZE);
  VirtualProperty::add("gc.active",PROP_GC_ACTIVE);
  // PRINT
  VirtualProperty::add("print.depth",PROP_PRINT_DEPTH);
  VirtualProperty::add("print.width",PROP_PRINT_WIDTH);
  // FD
  VirtualProperty::add("fd.variables",PROP_FD_VARIABLES);
  VirtualProperty::add("fd.propagators",PROP_FD_PROPAGATORS);
  VirtualProperty::add("fd.invoked",PROP_FD_INVOKED);
  VirtualProperty::add("fd.threshold",PROP_FD_THRESHOLD);
  // SPACES
  VirtualProperty::add("spaces.committed",PROP_SPACES_COMMITTED);
  VirtualProperty::add("spaces.cloned",PROP_SPACES_CLONED);
  VirtualProperty::add("spaces.created",PROP_SPACES_CREATED);
  VirtualProperty::add("spaces.failed",PROP_SPACES_FAILED);
  VirtualProperty::add("spaces.succeeded",PROP_SPACES_SUCCEEDED);
  // ERRORS
  VirtualProperty::add("errors.location",PROP_ERRORS_LOCATION);
  VirtualProperty::add("errors.debug",PROP_ERRORS_DEBUG);
  VirtualProperty::add("errors.hints",PROP_ERRORS_HINTS);
  VirtualProperty::add("errors.thread",PROP_ERRORS_THREAD);
  VirtualProperty::add("errors.depth",PROP_ERRORS_DEPTH);
  VirtualProperty::add("errors.width",PROP_ERRORS_WIDTH);
  // MESSAGES
  VirtualProperty::add("messages.gc",PROP_MESSAGES_GC);
  VirtualProperty::add("messages.idle",PROP_MESSAGES_IDLE);
  VirtualProperty::add("messages.feed",PROP_MESSAGES_FEED);
  VirtualProperty::add("messages.foreign",PROP_MESSAGES_FOREIGN);
  VirtualProperty::add("messages.load",PROP_MESSAGES_LOAD);
  VirtualProperty::add("messages.cache",PROP_MESSAGES_CACHE);
  // MEMORY
  VirtualProperty::add("memory.atoms",PROP_MEMORY_ATOMS);
  VirtualProperty::add("memory.names",PROP_MEMORY_NAMES);
  VirtualProperty::add("memory.builtins",PROP_MEMORY_BUILTINS);
  VirtualProperty::add("memory.freelist",PROP_MEMORY_FREELIST);
  VirtualProperty::add("memory.code",PROP_MEMORY_CODE);
  VirtualProperty::add("memory.heap",PROP_MEMORY_HEAP);
  // HEAP
  VirtualProperty::add("heap.used",PROP_HEAP_USED);
  // LIMITS
  VirtualProperty::add("limits.int.min",PROP_LIMITS_INT_MIN);
  VirtualProperty::add("limits.int.max",PROP_LIMITS_INT_MAX);
  // ARGV
  VirtualProperty::add("argv",PROP_ARGV);
  // MISC
  VirtualProperty::add("oz.standalone",PROP_STANDALONE);
  VirtualProperty::add("oz.home",PROP_HOME);
  VirtualProperty::add("os.name",PROP_OS_NAME);
  VirtualProperty::add("os.cpu",PROP_OS_CPU);
  // INTERNAL
  VirtualProperty::add("internal.debug",PROP_INTERNAL_DEBUG);
  VirtualProperty::add("internal.suspension",PROP_INTERNAL_SUSPENSION);
  VirtualProperty::add("internal.stop",PROP_INTERNAL_STOP);
  VirtualProperty::add("internal.ip.debug",PROP_INTERNAL_DEBUG_IP);
  VirtualProperty::add("internal.perdio.debug",PROP_INTERNAL_DEBUG_PERDIO);
  VirtualProperty::add("internal.browser",PROP_INTERNAL_BROWSER);
  VirtualProperty::add("internal.applet",PROP_INTERNAL_APPLET);
}
