$cmode='dyn';

%builtins_all =
(

    ###
    ### Connection
    ###

    'close'		=> { in  => [],
			     out => [],
			     BI  => BIClosePID,
			     native => true},

    'send'		=> { in  => ['+virtualString','+int','+int','+int','+int','value'],
			     out => [],
			     BI  => BISendPID,
			     native => true},

    ###
    ### Perdio
    ###


    'PerdioVar.is'	=> { in  => ['value'],
			     out => ['+bool'],
			     BI  =>   PerdioVar_is,
			     module=> 'perdiovar',
			     native => true},

    'crash'		=> { in  => [],
			     out => [],
			     BI  => BIcrash,
			     doesNotReturn=>1,
			     native => true},

    'dvset'		=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIdvset,
			     ifdef=>DEBUG_PERDIO,
			     module=>'perdio',
			     native => true},

    'siteStatistics'	=> { in  => [],
			     out => ['+[value]'],
			     BI  => BIsiteStatistics,
			     module=>'perdio',
			     native => true},

    'printBorrowTable'	=> { in  => [],
			     out => [],
			     BI  => BIprintBorrowTable,
			     module=>'perdio',
			     native => true},

    'printOwnerTable'	=> { in  => [],
			     out => [],
			     BI  => BIprintOwnerTable,
			     module=>'perdio',
			     native => true},


    'portWait'         =>  { in  => ['+port','+int'],
			     out => [],
			     BI  => BIportWait,
			     module=>'perdio',
			     native => true},


    'perdioStatistics'	=> { in  => [],
			     out => ['+record'],
			     BI  => BIperdioStatistics,
			     module=>'perdio' ,
			     native => true},


    'slowNet'           => { in  => ['+int', '+int'],
			     out => [],
			     bi  => BIslowNet,
			     native => true},

    ###
    ### Debug
    ###
        
    'Debug.inspect'     => { in  => ['value'],
                             out => ['+value'],
			     BI  => BIinspect,
		             native => true},


    'Debug.livenessX'	=> { in  => ['+int'],
			     out => ['+int'],
			     BI  => BIlivenessX,
			     native => true},

    'procedureEnvironment'=> { in  => ['+procedure'],
			       out => ['+tuple'],
			       BI  => BIprocedureEnvironment,
			       native => true},

    ###
    ### Statistics
    ###
        
    'statisticsPrint'	=> { in  => ['+virtualString'],
			     out => [],
			     BI  => BIstatisticsPrint,
			     native => true},

    'statisticsPrintProcs'=> { in  => [],
			       out => [],
			       BI  => BIstatisticsPrintProcs,
			       native => true},

    'instructionsPrint'	=> { in  => [],
			     out => [],
			     BI  => BIinstructionsPrint,
			     ifdef=>'PROFILE_INSTR',
			     native => true},

    'instructionsPrintCollapsable' => { in  => [],
			     out => [],
			     BI  => BIinstructionsPrintCollapsable,
			     ifdef=>'PROFILE_INSTR',
			     native => true},

    'instructionsPrintReset' => { in  => [],
			     out => [],
			     BI  => BIinstructionsPrintReset,
			     ifdef=>'PROFILE_INSTR',
			     native => true},

    'biPrint'		=> { in  => [],
			     out => [],
			     BI  => BIbiPrint,
			     ifdef=>'PROFILE_BI',
			     native => true},

    'halt'		=> { in  => [],
			     out => [],
			     BI  => BIhalt,
			     ifdef=>'DEBUG_TRACE',
			     native => true},

    ###
    ### Christian's private stuff
    ###

    'GetCloneDiff'	=> { in  => ['+space'],
			     out => ['+value'],
			     BI  => BIgetCloneDiff,
			     ifdef=>'CS_PROFILE',
			     native => true},


    ###
    ### Ralf's private stuff
    ###

    'funReturn'		=> { in  => ['value'],
			     out => [],
			     doesNotReturn => 1,
			     BI  => BIfunReturn,
			     native => true},

    'getReturn'		=> { in  => [],
			     out => ['value'],
			     BI  => BIgetReturn,
			     native => true},


    ###
    ### Tobias's private stuff
    ###

    'getConstraints'    => { in  => ['+value','+[value]'],
			     out => [],
			     bi  => BIgetConstraints,
			     native => true},

    ###
    ### Michael's private stuff
    ###

 );
