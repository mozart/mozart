$cmode='dyn';

%builtins_all =
(
    #* Finite Domain Base

    'is'                => { in  => ['*value','bool'],
                             out => [],
                             bi  => BIfdIs,
                             module => fd,
                             native => true},

    'isVar'             => { in  => ['value'],
                             out => [],
                             BI  => BIisFdVar,
                             module => fd,
                             native => true},

    'isVarB'            => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIisFdVarB,
                             module => fd,
                             native => true},

    'getLimits' => { in  => [],
                             out => ['+int','+int'],
                             BI  => BIgetFDLimits,
                             module => fd,
                             native => true},

    'getMin'            => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMin,
                             module => fd,
                             native => true},

    'getMid'            => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMid,
                             module => fd,
                             native => true},

    'getMax'            => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMax,
                             module => fd,
                             native => true},

    'getDom'            => { in  => ['*int','+[value]'],
                             out => [],
                             bi  => BIfdGetAsList,
                             module => fd,
                             native => true},

    'getCard'           => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdGetCardinality,
                             module => fd,
                             native => true},

    'getNextSmaller'    => { in  => ['+int','*int','int'],
                             out => [],
                             bi  => BIfdNextSmaller,
                             module => fd,
                             native => true},

    'getNextLarger'     => { in  => ['+int','*int','int'],
                             out => [],
                             bi  => BIfdNextLarger,
                             module => fd,
                             native => true},

    'tellConstraint'    => { in  => ['int','+value'],
                             out => [],
                             bi  => BIfdTellConstraint,
                             module => fd,
                             native => true},

    'watchSize' => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchSize,
                             module => fd,
                             native => true},

    'watchMin'  => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchMin,
                             module => fd,
                             native => true},

    'watchMax'  => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchMax,
                             module => fd,
                             native => true},

    'constrDisjSetUp'   => { in  => ['+value','+value','+value','+value'],
                             out => [],
                             bi  => BIfdConstrDisjSetUp,
                             module => fd,
                             native => true},

    'constrDisj'        => { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => BIfdConstrDisj,
                             module => fd,
                             native => true},

    'tellConstraintCD'=> { in  => ['value','value','value'],
                             out => [],
                             bi  => BIfdTellConstraintCD,
                             module => fd,
                             native => true},

    'debugStable'       => { in  => [],
                             out => [],
                             bi  => debugStable,
                             ifdef =>DEBUG_STABLE,
                             module => fd,
                             native => true},


    'resetStable'       => { in  => [],
                             out => [],
                             bi  => resetStable,
                             ifdef =>DEBUG_STABLE,
                             module => fd,
                             native => true},

 );
