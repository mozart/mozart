%builtins_all =
(

    ###
    ### Perdio
    ###


    'crash'		=> { in  => [],
			     out => [],
			     BI  => BIcrash,
			     doesNotReturn=>1},

    'dvset'		=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIdvset,
			     ifdef=>DEBUG_PERDIO},

    'siteStatistics'	=> { in  => [],
			     out => ['+[value]'],
			     BI  => BIsiteStatistics},

    'printBorrowTable'	=> { in  => [],
			     out => [],
			     BI  => BIprintBorrowTable},

    'printOwnerTable'	=> { in  => [],
			     out => [],
			     BI  => BIprintOwnerTable},

    'perdioStatistics'	=> { in  => [],
			     out => ['+record'],
			     BI  => BIperdioStatistics},

    'slowNet'           => { in  => ['+int', '+int'],
			     out => [],
			     bi  => BIslowNet},

    ###
    ### Statistics
    ###
        
    'statisticsPrint'	=> { in  => ['+virtualString'],
			     out => [],
			     BI  => BIstatisticsPrint},

    'statisticsPrintProcs'=> { in  => [],
			       out => [],
			       BI  => BIstatisticsPrintProcs},

    'instructionsPrint'	=> { in  => [],
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

    'biPrint'		=> { in  => [],
			     out => [],
			     BI  => BIbiPrint,
			     ifdef=>'PROFILE_BI'},

    ###
    ### Christian's private stuff
    ###

    'GetCloneDiff'	=> { in  => ['+space'],
			     out => ['+value'],
			     BI  => BIgetCloneDiff,
			     ifdef=>'CS_PROFILE'},


    ###
    ### Ralf's private stuff
    ###

    'funReturn'		=> { in  => ['value'],
			     out => [],
			     doesNotReturn => 1,
			     BI  => BIfunReturn},

    'getReturn'		=> { in  => [],
			     out => ['value'],
			     BI  => BIgetReturn},


    ###
    ### Tobias's private stuff
    ###


 );
