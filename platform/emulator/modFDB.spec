%builtins_all =
(
    #* Finite Domain Base

    'is'                => { in  => ['*value','bool'],
                             out => [],
                             bi  => BIfdIs},

    'isVar'             => { in  => ['value'],
                             out => [],
                             BI  => BIisFdVar},

    'isVarB'            => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIisFdVarB},

    'getLimits' => { in  => [],
                             out => ['+int','+int'],
                             BI  => BIgetFDLimits},

    'getMin'            => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMin},

    'getMid'            => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMid},

    'getMax'            => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdMax},

    'getDom'            => { in  => ['*int','+[value]'],
                             out => [],
                             bi  => BIfdGetAsList},

    'getCard'           => { in  => ['*int','int'],
                             out => [],
                             bi  => BIfdGetCardinality},

    'getNextSmaller'    => { in  => ['+int','*int','int'],
                             out => [],
                             bi  => BIfdNextSmaller},

    'getNextLarger'     => { in  => ['+int','*int','int'],
                             out => [],
                             bi  => BIfdNextLarger},

    'tellConstraint'    => { in  => ['int','+value'],
                             out => [],
                             bi  => BIfdTellConstraint},

    'watchSize' => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchSize},

    'watchMin'  => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchMin},

    'watchMax'  => { in  => ['*int','+int','bool'],
                             out => [],
                             bi  => BIfdWatchMax},

    'constrDisjSetUp'   => { in  => ['+value','+value','+value','+value'],
                             out => [],
                             bi  => BIfdConstrDisjSetUp},

    'constrDisj'        => { in  => ['+value','+value','+value'],
                             out => [],
                             bi  => BIfdConstrDisj},

    'tellConstraintCD'=> { in  => ['value','value','value'],
                             out => [],
                             bi  => BIfdTellConstraintCD},

    'debugStable'       => { in  => [],
                             out => [],
                             bi  => debugStable,
                             ifdef =>DEBUG_STABLE},

    'resetStable'       => { in  => [],
                             out => [],
                             bi  => resetStable,
                             ifdef =>DEBUG_STABLE},

 );
