$cmode='stat';

%builtins_all =
(
    #* Finite Domains

    # Internal stuff (always included)

    'fdIs'		=> { in  => ['*value','bool'],
			     out => [],
			     bi  => BIfdIs,
			     module => fd,
			     native => false},

    'fdIsVar'		=> { in  => ['value'],
			     out => [],
			     BI  => BIisFdVar,
			     module => fd,
			     native => false},

    'fdIsVarB'		=> { in  => ['value'],
			     out => ['+bool'],
			     BI  => BIisFdVarB,
			     module => fd,
			     native => false},

    'fdGetLimits'	=> { in  => [],
			     out => ['+int','+int'],
			     BI  => BIgetFDLimits,
			     module => fd,
			     native => true},

    'fdGetMin'		=> { in  => ['*int','int'],
			     out => [],
			     bi  => BIfdMin,
			     module => fd,
			     native => true},

    'fdGetMid'		=> { in  => ['*int','int'],
			     out => [],
			     bi  => BIfdMid,
			     module => fd,
			     native => true},

    'fdGetMax'		=> { in  => ['*int','int'],
			     out => [],
			     bi  => BIfdMax,
			     module => fd,
			     native => true},

    'fdGetDom'		=> { in  => ['*int','+[value]'],
			     out => [],
			     bi  => BIfdGetAsList,
			     module => fd,
			     native => true},

    'fdGetCard'		=> { in  => ['*int','int'],
			     out => [],
			     bi  => BIfdGetCardinality,
			     module => fd,
			     native => true},

    'fdGetNextSmaller'	=> { in  => ['+int','*int','int'],
			     out => [],
			     bi  => BIfdNextSmaller,
			     module => fd,
			     native => true},

    'fdGetNextLarger'	=> { in  => ['+int','*int','int'],
			     out => [],
			     bi  => BIfdNextLarger,
			     module => fd,
			     native => true},

    'fdTellConstraint'	=> { in  => ['int','+value'],
			     out => [],
			     bi  => BIfdTellConstraint,
			     module => fd,
			     native => true},

    'fdWatchSize'	=> { in  => ['*int','+int','bool'],
			     out => [],
			     bi  => BIfdWatchSize,
			     module => fd,
			     native => true},

    'fdWatchMin'	=> { in  => ['*int','+int','bool'],
			     out => [],
			     bi  => BIfdWatchMin,
			     module => fd,
			     native => true},

    'fdWatchMax'	=> { in  => ['*int','+int','bool'],
			     out => [],
			     bi  => BIfdWatchMax,
			     module => fd,
			     native => true},

    'fdConstrDisjSetUp'	=> { in  => ['+value','+value','+value','+value'],
			     out => [],
			     bi  => BIfdConstrDisjSetUp,
			     module => fd,
			     native => true},

    'fdConstrDisj'	=> { in  => ['+value','+value','+value'],
			     out => [],
			     bi  => BIfdConstrDisj,
			     module => fd,
			     native => true},

    'fdTellConstraintCD'=> { in  => ['value','value','value'],
			     out => [],
			     bi  => BIfdTellConstraintCD,
			     module => fd,
			     native => true},

    'debugStable'	=> { in  => [],
			     out => [],
			     bi  => debugStable,
			     ifdef =>DEBUG_STABLE,
			     module => fd,
			     native => true},


    'resetStable'	=> { in  => [],
			     out => [],
			     bi  => resetStable,
			     ifdef =>DEBUG_STABLE,
			     module => fd,
			     native => true},

 );
