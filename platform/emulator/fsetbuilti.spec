$cmode='stat';

%builtins_all =
(
    #* Finite Sets

    'fsValueToString'	=> { in  => ['+fset'],
			     out => ['+string'],
			     BI  => BIfsValueToString,
			     module=>fset ,
			     native => true},

    'fsIsVarB'		=> { in  => ['value'],
			     out => ['+bool'],
			     BI  => BIfsIsVarB,
			     module=>fset ,
			     native => false},

    'fsIsValueB'	=> { in  => ['+value','bool'],
			     out => [],
			     bi  => BIfsIsValueB,
			     module=>fset ,
			     native => false},

    'fsSetValue'	=> { in  => ['+value','fset'],
			     out => [],
			     bi  => BIfsSetValue,
			     module=>fset ,
			     native => true},

    'fsSet'		=> { in  => ['+value','+value','fset'],
			     out => [],
			     bi  => BIfsSet,
			     module=>fset ,
			     native => true},

    'fsSup'		=> { in  => [],
			     out => ['+int'],
			     BI  => BIfsSup,
			     module=>fset ,
			     native => true},

    'fsGetKnownIn'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetKnownIn,
			     module=>fset ,
			     native => true},

    'fsGetKnownNotIn'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetKnownNotIn,
			     module=>fset ,
			     native => true},

    'fsGetUnknown'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetUnknown,
			     module=>fset ,
			     native => true},

    'fsGetGlb'		=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetKnownIn,
			     module=>fset ,
			     native => true},

    'fsGetLub'		=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetLub,
			     module=>fset ,
			     native => true},

    'fsGetCard'		=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetCard,
			     module=>fset ,
			     native => true},

    'fsCardRange'	=> { in  => ['int','int','fset'],
			     out => [],
			     bi  => BIfsCardRange,
			     module=>fset ,
			     native => true},

    'fsGetNumOfKnownIn'	=> { in  => ['fset','int'],
			     out => [],
			     bi  => BIfsGetNumOfKnownIn,
			     module=>fset ,
			     native => true},

    'fsGetNumOfKnownNotIn'=> { in  => ['fset','int'],
			       out => [],
			       bi  => BIfsGetNumOfKnownNotIn,
			       module=>fset,
			       native => true},
    
    'fsGetNumOfUnknown'	=> { in  => ['fset','int'],
			     out => [],
			     bi  => BIfsGetNumOfUnknown,
			     module=>fset ,
			     native => true},

    'fsClone'		=> { in  => ['fset','fset'],
			     out => [],
			     bi  => BIfsClone,
			     module=>fset ,
			     native => true},

);
