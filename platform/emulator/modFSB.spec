%builtins_all =
(
    'valueToString'	=> { in  => ['+fset'],
			     out => ['+string'],
			     BI  => BIfsValueToString},

    'isVarB'		=> { in  => ['value'],
			     out => ['+bool'],
			     BI  => BIfsIsVarB},

    'isValueB'	=> { in  => ['+value','bool'],
			     out => [],
			     bi  => BIfsIsValueB},

    'setValue'	=> { in  => ['+value','fset'],
			     out => [],
			     bi  => BIfsSetValue},

    'set'		=> { in  => ['+value','+value','fset'],
			     out => [],
			     bi  => BIfsSet},

    'sup'		=> { in  => [],
			     out => ['+int'],
			     BI  => BIfsSup},

    'getKnownIn'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetKnownIn},

    'getKnownNotIn'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetKnownNotIn},

    'getUnknown'	=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetUnknown},

    'getGlb'		=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetKnownIn},

    'getLub'		=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetLub},

    'getCard'		=> { in  => ['fset','value'],
			     out => [],
			     bi  => BIfsGetCard},

    'cardRange'	=> { in  => ['int','int','fset'],
			     out => [],
			     bi  => BIfsCardRange},

    'getNumOfKnownIn'	=> { in  => ['fset','int'],
			     out => [],
			     bi  => BIfsGetNumOfKnownIn},

    'getNumOfKnownNotIn'=> { in  => ['fset','int'],
			       out => [],
			       bi  => BIfsGetNumOfKnownNotIn},
    
    'getNumOfUnknown'	=> { in  => ['fset','int'],
			     out => [],
			     bi  => BIfsGetNumOfUnknown},

    'clone'		=> { in  => ['fset','fset'],
			     out => [],
			     bi  => BIfsClone},

);

