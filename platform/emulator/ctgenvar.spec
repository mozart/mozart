$cmode='stat';

%builtins_all =
(
    ###* GenCtVar

    'isCtVarB'	        => { in  => ['value'],
			     out => ['+bool'],
			     BI  => BIIsGenCtVarB,
			     module =>  ct,
			     native => true},

    'getCtVarConstraintAsAtom' => { in  => ['value','atom'],
			     out => [],
			     BI  => BIGetCtVarConstraintAsAtom,
			     module => ct,
			     native => true},

    'getCtVarNameAsAtom'   => { in  => ['value','atom'],
			     out => [],
			     BI  => BIGetCtVarNameAsAtom,
			     module =>  ct,
			     native => true},

 );
