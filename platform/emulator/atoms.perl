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
	  ( AtomComma,     ","),
	  ( AtomHat,       "^"),
	  ( AtomTilde,     "~"),
	  ( AtomDExcl,     "!!"),
 
	  ( AtomSup,	   "sup"),
	  ( AtomCompl,	   "compl"),

	  ( AtomEmpty,     ""),
	  ( AtomNewLine,   "\\n"),

	  ( AtomUpper,     "upper"),
	  ( AtomLower,     "lower"),
	  ( AtomDigit,     "digit"),
	  ( AtomCharSpace, "space"),
	  ( AtomPunct,     "punct"),
	  ( AtomOther,     "other"),

	  ( AtomSucceeded,  "succeeded"),
	  ( AtomAlt,        "alternatives"),
	  ( AtomEntailed,   "entailed"),
	  ( AtomStuck,      "stuck"),
	  ( AtomSuspended,  "suspended"),
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
	  ( AtomGetInternal,"getInternal"),
	  ( AtomGetNative,  "getNative"),
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

	  ( AtomFail,       "fail"),

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
          ( AtomCells,       "cells"),
	  ( AtomOn,	     "on"),
	  ( AtomPropagate,   "propagate"),
	  ( AtomPropagators, "propagators"),
	  ( AtomPropLocation,"propLocation"),
	  ( AtomFile,        "file"),
	  ( AtomPath,        "path"),
	  ( AtomLine,        "line"),
	  ( AtomColumn,      "column"),
	  ( AtomInvoc,       "invoc"),
	  ( AtomCallerInvoc, "callerInvoc"),
	  ( AtomPropInvoc,   "propInvoc"),
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
	  ( AtomVerbose,     "verbose"),
	  ( AtomHeap,        "heap"),
	  ( AtomDetailed,    "detailed"),
	  ( AtomBrowser,     "browser"),
	  ( AtomApplet,      "applet"),
	  ( AtomArgs,        "args"),
	  ( AtomURL,         "url"),
	  ( AtomGUI,	     "gui"),
	  ( AtomOs,          "os"),
	  ( AtomArch,        "arch"),

	  ( AtomKinded,      "kinded"),
	  ( AtomDet,         "det"),
	  ( AtomFuture,      "future"),
	  ( AtomUnknown,     "unknown"),

	  ( AtomDebugIP,     "debugIP"),
	  ( AtomDebugPerdio, "debugPerdio"),


	  # Error formatting
          ( AtomStack,          "stack"),
          ( AtomD,              "d"),
          ( AtomFailure,        "failure"),
	  ( AtomError,          "error"),

	  # Failure
	  ( AtomBlocked,	"blocked"),
	  ( AtomPermBlocked,	"permBlocked"),
	  ( AtomTempBlocked,	"tempBlocked"),
	  ( AtomWillBlock,	"willBlock"),
	  ( AtomPermWillBlock,	"permWillBlock"),
	  ( AtomTempWillBlock,	"tempWillBlock"),
	  ( AtomSome,		"some"),
	  ( AtomPermSome,	"permSome"),
	  ( AtomTempSome,	"tempSome"),
	  ( AtomAll,	        "all"),
	  ( AtomPermAll,	"permAll"),
	  ( AtomTempAll,	"tempAll"),
	  ( AtomPermFail,	"permFail"),
	  ( AtomTempFail,	"tempFail"),
	  ( AtomRemoteProblem,	"remoteProblem"),
	  ( AtomAny,	        "any"),
	  ( AtomNormal,	        "normal"),
	  ( AtomThis,           "this"),
	  ( AtomSingle,	        "single"),
	  ( AtomRetry,		"retry"),
	  ( AtomSkip,		"skip"),
	  ( AtomSiteWatcher,	"siteWatcher"),
	  ( AtomNetWatcher,	"netWatcher"),
	  ( AtomWatcher,	"watcher"),
	  ( AtomInjector,	"injector"),
	  ( AtomSafeInjector,	"safeInjector"),
	  ( AtomInfo,	        "info"),
	  ( AtomWait,	        "wait"),
	  ( AtomObjectFetch,	"objectFetch"),
	  ( AtomDp,	        "dp"),
	  ( AtomEntity,	        "entity"),
	  ( AtomConditions,	"conditions"),
	  ( AtomOp,	        "op"),
	  ( AtomOwner,	        "owner"),
	  ( AtomSystem,	        "system"),
	  ( AtomState,	        "state"),
	  ( AtomIO,	        "io"),
	  
	# Types
	  ( AtomBool,        "bool"),
	  ( AtomInt,         "int"),
	  ( AtomThread,      "thread"),
	  ( AtomRecord,      "record"),
	  ( AtomOnRecord,    "onRecord"),
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

	  ( AtomExtension,      "extension" ),
	  ( AtomBitArray,       "bitArray" ),
	  ( AtomBitString,      "bitString" ),
	  ( AtomByteString,     "byteString" ),
	  ( AtomSituatedExtension, "situatedExtension" ),
	  ( AtomSited,             "sited" ),
          ( AtomAssembler,       "assembler" ),          

	  ( E_ERROR, 		"error"),
	  ( E_KERNEL,		"kernel"),
	  ( E_OBJECT,		"object"),
	  ( E_TK,    		"tk"),
	  ( E_OS,    		"os"),
	  ( E_SYSTEM,		"system"),
	  ( E_DISTRIBUTION,	"distribution"),

	  # debugger related
          ( AtomDebugExit,     "DEBUG_EXIT"),
          ( AtomDebugStep,     "DEBUG_STEP"),
          ( AtomDebugNoStep,   "DEBUG_NOSTEP"),

          ( AtomEntry,         "entry" ),
          ( AtomExit,          "exit" ),
          ( AtomThr,           "thr" ),
          ( AtomOrigin,        "origin" ),
          ( AtomDebugFrame,    "debugFrame" ),
          ( AtomProcedureFrame,"procedureFrame" ),
          ( AtomPC,            "PC" ),
          ( AtomKind,          "kind" ),
          ( AtomArgs,          "args" ),
          ( AtomVars,          "vars" ),
          ( AtomFrameID,       "frameID" ),
          ( AtomData,          "data" ),
          ( AtomCall,          "call" ),
          ( AtomY,             "Y" ),
          ( AtomG,             "G" ),
          ( AtomV,             "v" ),

	  # browser-related;
          ( AtomBBuiltin,       "(builtin)" ),
          ( AtomDash,           "-" ),

	  # alice-related;
          ( E_ALICE,            "alice" ),
          ( AtomAliceRPC,       "alice.rpc" ),

	  );


%names = (
	  ( NameTrue,          "true"),
	  ( NameFalse,         "false"),
	  ( NameUnit,          "unit"),
	  ( NameGroupVoid,     "group(void)"),
	  ( NameNonExportable, "nonExportable"),

	  # for marking uninitialized Y registers
	  ( NameVoidRegister,  "VoidRegister" ),

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
	  ( NameOoFreeFeat,    "ooFreeFeat"),
	  ( NameOoFeat,        "ooFeat"),
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

    $atoms = 0;
    
    print "const char * _StaticAtomChars[] = {\n";
    foreach $key (keys %atoms) { 
	print "   \"$atoms{$key}\",\n"; $atoms++;
    }
    print "};\n\n";
    print "ATOMVOLATILE TaggedRef _StaticAtomTable[$atoms];\n\n";
    $names = 0;
    
    print "const char * _StaticNameChars[] = {\n";
    foreach $key (keys %names) { 
	print "   \"$names{$key}\",\n"; $names++;
    }
    print "};\n\n";
    print "NAMEVOLATILE TaggedRef _StaticNameTable[$names];\n\n";

    print <<EOF;
	void initAtomsAndNames() {
	    int i;
	    for (i = $atoms; i--; )
		_StaticAtomTable[i] = oz_atomNoDup(_StaticAtomChars[i]);
             
	    for (i = $names; i--; )
		_StaticNameTable[i] = oz_uniqueName(_StaticNameChars[i]);
             
	};

EOF


} elsif ("$option" eq "-header") {

    print "void initAtomsAndNames();\n\n\n";
    print "#ifndef ATOMVOLATILE\n";
    print "#define ATOMVOLATILE\n";
    print "#endif\n\n\n";
    print "#ifndef NAMEVOLATILE\n";
    print "#define NAMEVOLATILE volatile\n";
    print "#endif\n\n\n";
    print "extern ATOMVOLATILE TaggedRef _StaticAtomTable[];\n";
    print "extern NAMEVOLATILE TaggedRef _StaticNameTable[];\n\n\n";
    $i=0;
    foreach $key (keys %atoms) { 
	print "#define $key _StaticAtomTable[$i] \n"; $i++;
    }	
    $i=0;
    foreach $key (keys %names) { 
	print "#define $key _StaticNameTable[$i] \n"; $i++;
    }
    print "\n";

} else {

    die "usage: $ARGV[0] -body|-header\n";

}
