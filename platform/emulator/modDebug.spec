$cmode='dyn';

%builtins_all =
(

    'threadUnleash'	=> { in  => ['+thread','+int'],
			     out => [],
			     BI  => BIthreadUnleash,
			     native => true},

    'getStream'	=> { in  => [],
			     out => ['value'],
			     BI  => BIgetDebugStream,
			     native => true},

    'setStepFlag'	=> { in  => ['+thread','+bool'],
			     out => [],
			     BI  => BIsetStepFlag,
			     native => true},

    'setTraceFlag'=> { in  => ['+thread','+bool'],
			     out => [],
			     BI  => BIsetTraceFlag,
			     native => true},

    'checkStopped'=> { in  => ['+thread'],
			     out => ['+bool'],
			     BI  => BIcheckStopped,
			     native => true},

    'print'	=> { in  => ['value','+int'],
			     out => [],
			     BI  => BIdebugPrint,
			     ifdef=>'DEBUG_PRINT',
			     native => true},

    'printLong'	=> { in  => ['value','+int'],
			     out => [],
			     BI  => BIdebugPrintLong,
			     ifdef=>'DEBUG_PRINT',
			     native => true},

    'prepareDumpThreads'	=> { in  => [],
				     out => [],
				     BI  => BIprepareDumpThreads,
				     native => true},
    
    'dumpThreads'	=> { in  => [],
			     out => [],
			     BI  => BIdumpThreads,
			     native => true},

    'listThreads'	=> { in  => [],
			     out => ['+[thread]'],
			     BI  => BIlistThreads,
			     native => true},

    'breakpointAt'=> { in  => ['+atom','+int','+bool'],
			     out => ['+bool'],
			     BI  => BIbreakpointAt,
			     native => true},

    'breakpoint'	=> { in  => [],
			     out => [],
			     BI  => BIbreakpoint,
			     native => true},

    'displayDef'	=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIdisplayDef,
			     native => true},

    'displayCode'	=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIdisplayCode,
			     native => true},

    'procedureCode'=> { in  => ['+procedure'],
			      out => ['+int'],
			      BI  => BIprocedureCode,
			      native => true},
    
    'procedureCoord'=> { in  => ['+procedure'],
			       out => ['+record'],
			       BI  => BIprocedureCoord,
			       native => true},
 );
