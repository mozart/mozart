$cmode='dyn';

%builtins_all =
(
    #* Finite Set Base

    'valueToString'	=> { in  => ['+fset'],
			     out => ['+string'],
			     BI  => BIfsValueToString,
			     module=>fset ,
			     native => true},

    'isVarB'		=> { in  => ['value'],
			     out => ['+bool'],
			     BI  => BIfsIsVarB,
			     module=>fset ,
			     native => true},

    'isValueB'	=> { in  => ['+value','bool'],
			     out => [],
			     bi  => BIfsIsValueB,
			     module=>fset ,
			     native => true},

    'setValue'	=> { in  => ['+value','fset'],
			     out => [],
			     bi  => BIfsSetValue,
			     module=>fset ,
			     native => true},

    'set'		=> { in  => ['+value','+value','fset'],
			     out => [],
			     bi  => BIfsSet,
			     module=>fset ,
			     native => true},

    'sup'		=> { in  => [],
			     out => ['+int'],
			     BI  => BIfsSup,
			     module=>fset ,
			     native => true},

    'getKnownIn'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetKnownIn,
			     module=>fset ,
			     native => true},

    'getKnownNotIn'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetKnownNotIn,
			     module=>fset ,
			     native => true},

    'getUnknown'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetUnknown,
			     module=>fset ,
			     native => true},

    'getGlb'		=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetKnownIn,
			     module=>fset ,
			     native => true},

    'getLub'		=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetLub,
			     module=>fset ,
			     native => true},

    'getCard'		=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetCard,
			     module=>fset ,
			     native => true},

    'cardRange'	=> { in  => ['int','int','fset'],
			     out => [],
			     bi  => BIfsCardRange,
			     module=>fset ,
			     native => true},

    'getNumOfKnownIn'	=> { in  => ['fset','int'],
			     out => [],
			     bi  => BIfsGetNumOfKnownIn,
			     module=>fset ,
			     native => true},

    'getNumOfKnownNotIn'=> { in  => ['fset','int'],
			       out => [],
			       bi  => BIfsGetNumOfKnownNotIn,
			       module=>fset,
			       native => true},
    
    'getNumOfUnknown'	=> { in  => ['fset','int'],
			     out => [],
			     bi  => BIfsGetNumOfUnknown,
			     module=>fset ,
			     native => true},

    'clone'		=> { in  => ['fset','fset'],
			     out => [],
			     bi  => BIfsClone,
			     module=>fset ,
			     native => true},

);

