%builtins_all =
(

    'threadUnleash'	=> { in  => ['+thread','+int'],
			     out => [],
			     BI  => BIthreadUnleash},

    'getStream'	=> { in  => [],
			     out => ['value'],
			     BI  => BIgetDebugStream},

    'setStepFlag'	=> { in  => ['+thread','+bool'],
			     out => [],
			     BI  => BIsetStepFlag},

    'setTraceFlag'=> { in  => ['+thread','+bool'],
			     out => [],
			     BI  => BIsetTraceFlag},

    'checkStopped'=> { in  => ['+thread'],
			     out => ['+bool'],
			     BI  => BIcheckStopped},

    'print'	=> { in  => ['value','+int'],
			     out => [],
			     BI  => BIdebugPrint},

    'printLong'	=> { in  => ['value','+int'],
			     out => [],
			     BI  => BIdebugPrintLong},

    'prepareDumpThreads'	=> { in  => [],
				     out => [],
				     BI  => BIprepareDumpThreads},
    
    'dumpThreads'	=> { in  => [],
			     out => [],
			     BI  => BIdumpThreads},

    'listThreads'	=> { in  => [],
			     out => ['+[thread]'],
			     BI  => BIlistThreads},

    'breakpointAt'=> { in  => ['+atom','+int','+bool'],
			     out => ['+bool'],
			     BI  => BIbreakpointAt},

    'breakpoint'	=> { in  => [],
			     out => [],
			     BI  => BIbreakpoint},

    'displayDef'	=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIdisplayDef},

    'displayCode'	=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIdisplayCode},

    'procedureCode'=> { in  => ['+procedure'],
			      out => ['+int'],
			      BI  => BIprocedureCode},
    
    'procedureCoord'=> { in  => ['+procedure'],
			       out => ['+record'],
			       BI  => BIprocedureCoord},

 );
