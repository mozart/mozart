%builtins_all =
(

    'threadUnleash'	=> { in  => ['+thread','+int'],
			     out => [],
			     BI  => BIthreadUnleash},

    'getStream'	        => { in  => [],
			     out => ['value'],
			     BI  => BIgetDebugStream},

    'setStepFlag'	=> { in  => ['+thread','+bool'],
			     out => [],
			     BI  => BIsetStepFlag},

    'setTraceFlag'      => { in  => ['+thread','+bool'],
			     out => [],
			     BI  => BIsetTraceFlag},

    'checkStopped'      => { in  => ['+thread'],
			     out => ['+bool'],
			     BI  => BIcheckStopped},

    'breakpointAt'      => { in  => ['+atom','+int','+bool'],
			     out => ['+bool'],
			     BI  => BIbreakpointAt},

    'breakpoint'	=> { in  => [],
			     out => [],
			     BI  => BIbreakpoint},

    'procedureCoord'    => { in  => ['+procedure'],
			     out => ['+record'],
			     BI  => BIprocedureCoord},

 );
