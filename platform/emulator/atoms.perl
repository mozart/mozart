#
# Generate declarations (file atoms.hh) and
# initialization (atoms.cc) for some often
# used atomes and names
#



%atoms = (
          ( AtomNil,       "nil"),
          ( AtomCons,      "|"),
          ( AtomPair,      "#"),
          ( AtomVoid,      "_"),
          ( AtomDot,       "."),

          ( AtomSup,       "sup"),
          ( AtomCompl,     "compl"),

          ( AtomEmpty,     ""),
          ( AtomUpper,     "upper"),
          ( AtomLower,     "lower"),
          ( AtomDigit,     "digit"),
          ( AtomCharSpace, "space"),
          ( AtomPunct,     "punct"),
          ( AtomOther,     "other"),

          ( AtomSucceeded,  "succeeded"),
          ( AtomAlt,        "alternatives"),
          ( AtomEntailed,   "entailed"),
          ( AtomSuspended,  "suspended"),
          ( AtomBlocked,    "blocked"),
          ( AtomMerged,     "merged"),
          ( AtomFailed,     "failed"),

          ( AtomDebugCallC, "call/c"),
          ( AtomDebugCallF, "call/f"),
          ( AtomDebugCondC, "conditional/c"),
          ( AtomDebugCondF, "conditional/f"),
          ( AtomDebugLockC, "lock/c"),
          ( AtomDebugLockF, "lock/f"),
          ( AtomDebugNameC, "name generation/c"),
          ( AtomDebugNameF, "name generation/f"),

          ( AtomUnify,      "unify"),
          ( AtomException,  "exception"),

          ( AtomExport,     "export"),
          ( AtomObtain,     "obtain"),
          ( AtomBoot,       "Boot"),

          ( AtomNew,        "new"),
          ( AtomApply,      "apply"),
          ( AtomApplyList,  "applyList"),

          ( AtomMin,        "min"),
          ( AtomMax,        "max"),
          ( AtomMid,        "mid"),
          ( AtomNaive,      "naive"),
          ( AtomSize,       "size"),
          ( AtomNbSusps,    "nbSusps"),

          ( AtomLow,        "low"),

          # For system set and get
          ( AtomActive,      "active"),
          ( AtomAtoms,       "atoms"),
          ( AtomBuiltins,    "builtins"),
          ( AtomBytecodeXRegisters, "bytecode.xregisters"),
          ( AtomCache,       "cache"),
          ( AtomCommitted,   "committed"),
          ( AtomCloned,      "cloned"),
          ( AtomCode,        "code"),
          ( AtomCopy,        "copy"),
          ( AtomCreated,     "created"),
          ( AtomDebug,       "debug"),
          ( AtomDepth,       "depth"),
          ( AtomFeed,        "feed"),
          ( AtomForeign,     "foreign"),
          ( AtomFree,        "free"),
          ( AtomFreelist,    "freelist"),
          ( AtomGC,          "gc"),
          ( AtomCodeCycles,  "codeCycles"),
          ( AtomHigh,        "high"),
          ( AtomHints,       "hints"),
          ( AtomIdle,        "idle"),
          ( AtomIntMax,      "int.max"),
          ( AtomIntMin,      "int.min"),
          ( AtomInvoked,     "invoked"),
          ( AtomLimits,      "limits"),
          ( AtomLoad,        "load"),
          ( AtomLocation,    "location"),
          ( AtomMedium,      "medium"),
          ( AtomNames,       "names"),
          ( AtomOn,          "on"),
          ( AtomPropagate,   "propagate"),
          ( AtomPropagators, "propagators"),
          ( AtomRun,         "run"),
          ( AtomRunnable,    "runnable"),
          ( AtomShowSuspension, "showSuspension"),
          ( AtomStopOnToplevelFailure, "stopOnToplevelFailure"),
          ( AtomSystem,      "system"),
          ( AtomThreshold,   "threshold"),
          ( AtomTolerance,   "tolerance"),
          ( AtomTotal,       "total"),
          ( AtomUser,        "user"),
          ( AtomVariables,   "variables"),
          ( AtomWidth,       "width"),
          ( AtomHeap,        "heap"),
          ( AtomDetailed,    "detailed"),
          ( AtomBrowser,     "browser"),
          ( AtomApplet,      "applet"),
          ( AtomArgs,        "args"),
          ( AtomURL,         "url"),
          ( AtomOs,          "os"),
          ( AtomArch,        "arch"),

          ( AtomKinded,      "kinded"),
          ( AtomDet,         "det"),
          ( AtomFuture,      "future"),
          ( AtomUnknown,     "unknown"),

          ( AtomDebugIP,     "debugIP"),
          ( AtomDebugPerdio, "debugPerdio"),


          # Failure
          ( AtomBlocked,        "blocked"),
          ( AtomPermBlocked,    "permBlocked"),
          ( AtomTempBlocked,    "tempBlocked"),
          ( AtomWillBlock,      "willBlock"),
          ( AtomPermWillBlock,  "permWillBlock"),
          ( AtomTempWillBlock,  "tempWillBlock"),
          ( AtomSome,           "some"),
          ( AtomPermSome,       "permSome"),
          ( AtomTempSome,       "tempSome"),
          ( AtomAll,            "all"),
          ( AtomPermAll,        "permAll"),
          ( AtomTempAll,        "tempAll"),
          ( AtomNormal,         "normal"),
          ( AtomThis,           "this"),
          ( AtomSingle,         "single"),
          ( AtomRetry,          "retry"),
          ( AtomSkip,           "skip"),
          ( AtomSiteWatcher,    "siteWatcher"),
          ( AtomNetWatcher,     "netWatcher"),
          ( AtomInjector,       "injector"),

        # Types
          ( AtomBool,        "bool"),
          ( AtomInt,         "int"),
          ( AtomThread,      "thread"),
          ( AtomRecord,      "record"),
          ( AtomFSet,        "fset"),
          ( AtomVariable,    "variable"),
          ( AtomFloat,       "float" ),
          ( AtomName,        "name" ),
          ( AtomAtom,        "atom" ),
          ( AtomTuple,       "tuple" ),
          ( AtomForeignPointer, "foreignPointer" ),
          ( AtomProcedure,      "procedure" ),
          ( AtomCell,           "cell" ),
          ( AtomSpace,          "space" ),
          ( AtomObject,         "object" ),
          ( AtomPort,           "port" ),
          ( AtomChunk,          "chunk" ),
          ( AtomArray,          "array" ),
          ( AtomDictionary,     "dictionary" ),
          ( AtomLock,           "lock" ),
          ( AtomClass,          "class" ),
          ( AtomResource,       "resource" ),

          ( E_ERROR,            "error"),
          ( E_KERNEL,           "kernel"),
          ( E_OBJECT,           "object"),
          ( E_TK,               "tk"),
          ( E_OS,               "os"),
          ( E_SYSTEM,           "system"),
          ( E_DISTRIBUTION,     "distribution"),
          );


%names = (
          ( NameUnit,          "unit"),
          ( NameGroupVoid,     "group(void)"),
          ( NameNonExportable, "nonExportable"),


          # needed for instances: methods
          ( NameOoMeth,        "ooMeth"),
          ( NameOoFastMeth,    "ooFastMeth"),
          ( NameOoDefaults,    "ooDefaults"),
          # optional arguments
          ( NameOoDefaultVar,  "ooDefaultVar"),
          ( NameOoRequiredArg, "ooRequiredArg"),
          # needed for instances: attributes
          ( NameOoAttr,        "ooAttr"),
          # needed for instances: features
          ( NameOoUnFreeFeat,  "ooUnFreeFeat"),
          ( NameOoFreeFeatR,   "ooFreeFeatR"),
          ( NameOoFreeFlag,    "ooFreeFlag"),
          # misc stuff
          ( NameOoPrintName,   "ooPrintName"),
          ( NameOoFallback,    "ooFallback"),
          # inheritance related
          ( NameOoMethSrc,     "ooMethSrc"),
          ( NameOoAttrSrc,     "ooAttrSrc"),
          ( NameOoFeatSrc,     "ooFeatSrc"),
          );


$option = $ARGV[0];

if ("$option" eq "-body") {

    print "#include\"value.hh\"\n";

    print "TaggedRef\n";
    foreach $key (keys %atoms) { print "   $key,\n"; }
    foreach $key (keys %names) { print "   $key,\n"; }
    print "dummyend;\n";

    print "void initAtomsAndNames() {\n";

    while (($key,$val) = each %atoms) {
        printf("   %-20s = oz_atom(\"%s\");\n",$key,$val);
    }

    print "\n\n";

    while (($key,$val) = each %names) {
        printf("   %-20s = oz_uniqueName(\"%s\");\n",$key,$val);
    }

    print "}\n\n";

} elsif ("$option" eq "-header") {

    print "void initAtomsAndNames();\n\n\n";
    print "extern TaggedRef\n";
    foreach $key (keys %atoms) { print "   $key,\n"; }
    foreach $key (keys %names) { print "   $key,\n"; }
    print "dummyend;\n";

} else {

    die "usage: $ARGV[0] -body|-header\n";

}
