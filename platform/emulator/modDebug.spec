%builtins_all =
(

    'threadUnleash'     => { in  => ['+thread','+int'],
                             out => [],
                             BI  => BIthreadUnleash},

    'getStream'         => { in  => [],
                             out => ['value'],
                             BI  => BIgetDebugStream},

    'setStepFlag'       => { in  => ['+thread','+bool'],
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

    'breakpoint'        => { in  => [],
                             out => [],
                             BI  => BIbreakpoint},

    'procedureCoord'    => { in  => ['+procedure'],
                             out => ['+record'],
                             BI  => BIprocedureCoord},

 'getId'                  => { in  => ['+thread'],
                               out => ['+int'],
                               BI  => BIthreadID},

 'setId'                  => { in  => ['+thread','+int'],
                               out => [],
                               BI  => BIsetThreadID},

 'getParentId'    => { in  => ['+thread'],
                               out => ['+int'],
                               BI  => BIparentThreadID},

 'setRaiseOnBlock' => { in  => ['+thread','+bool'],
                               out => [],
                               BI  => BIthreadSetRaiseOnBlock},

 'getRaiseOnBlock' => { in  => ['+thread'],
                               out => ['+bool'],
                               BI  => BIthreadGetRaiseOnBlock},

 'getTaskStack'  => { in  => ['+thread','+int','+bool'],
                              out => ['+[record]'],
                              BI  => BIthreadTaskStack},

 'getFrameVariables' => { in  => ['+thread','+int'],
                              out => ['+record'],
                              BI  => BIthreadFrameVariables},

 'getLocation'   => { in  => ['+thread'],
                              out => ['+[atom]'],
                              BI  => BIthreadLocation},


 );
