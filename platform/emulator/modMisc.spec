%builtins_all =
(

    ###
    ### Perdio
    ###


    'PerdioVar.is'      => { in  => ['value'],
                             out => ['+bool'],
                             BI  =>   PerdioVar_is},

    'crash'             => { in  => [],
                             out => [],
                             BI  => BIcrash,
                             doesNotReturn=>1},

    'dvset'             => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIdvset,
                             ifdef=>DEBUG_PERDIO},

    'siteStatistics'    => { in  => [],
                             out => ['+[value]'],
                             BI  => BIsiteStatistics},

    'printBorrowTable'  => { in  => [],
                             out => [],
                             BI  => BIprintBorrowTable},

    'printOwnerTable'   => { in  => [],
                             out => [],
                             BI  => BIprintOwnerTable},

    'perdioStatistics'  => { in  => [],
                             out => ['+record'],
                             BI  => BIperdioStatistics},

    'slowNet'           => { in  => ['+int', '+int'],
                             out => [],
                             bi  => BIslowNet},

    ###
    ### Debug
    ###

    'displayDef'        => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIdisplayDef},

    'displayCode'       => { in  => ['+int','+int'],
                             out => [],
                             BI  => BIdisplayCode},

    'procedureCode'=> { in  => ['+procedure'],
                              out => ['+int'],
                              BI  => BIprocedureCode},

    'print'     => { in  => ['value','+int'],
                             out => [],
                             BI  => BIdebugPrint},

    'printLong' => { in  => ['value','+int'],
                             out => [],
                             BI  => BIdebugPrintLong},

    'Debug.inspect'     => { in  => ['value'],
                             out => ['+value'],
                             BI  => BIinspect},

    'Debug.livenessX'   => { in  => ['+int'],
                             out => ['+int'],
                             BI  => BIlivenessX},

    'procedureEnvironment'=> { in  => ['+procedure'],
                               out => ['+tuple'],
                               BI  => BIprocedureEnvironment},

    ###
    ### Statistics
    ###

    'statisticsPrint'   => { in  => ['+virtualString'],
                             out => [],
                             BI  => BIstatisticsPrint},

    'statisticsPrintProcs'=> { in  => [],
                               out => [],
                               BI  => BIstatisticsPrintProcs},

    'instructionsPrint' => { in  => [],
                             out => [],
                             BI  => BIinstructionsPrint,
                             ifdef=>'PROFILE_INSTR'},

    'instructionsPrintCollapsable' => { in  => [],
                             out => [],
                             BI  => BIinstructionsPrintCollapsable,
                             ifdef=>'PROFILE_INSTR'},

    'instructionsPrintReset' => { in  => [],
                             out => [],
                             BI  => BIinstructionsPrintReset,
                             ifdef=>'PROFILE_INSTR'},

    'biPrint'           => { in  => [],
                             out => [],
                             BI  => BIbiPrint,
                             ifdef=>'PROFILE_BI'},

    'halt'              => { in  => [],
                             out => [],
                             BI  => BIhalt,
                             ifdef=>'DEBUG_TRACE'},

    ###
    ### Christian's private stuff
    ###

    'GetCloneDiff'      => { in  => ['+space'],
                             out => ['+value'],
                             BI  => BIgetCloneDiff,
                             ifdef=>'CS_PROFILE'},


    ###
    ### Ralf's private stuff
    ###

    'funReturn'         => { in  => ['value'],
                             out => [],
                             doesNotReturn => 1,
                             BI  => BIfunReturn},

    'getReturn'         => { in  => [],
                             out => ['value'],
                             BI  => BIgetReturn},


    ###
    ### Tobias's private stuff
    ###


    ###
    ### Michael's private stuff
    ###

 );
