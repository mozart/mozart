%builtins_all =
(
    #* Constraint variables

    'isB'	        => { in  => ['value'],
			     out => ['+bool'],
			     BI  => BIIsGenCtVarB},

    'getConstraintAsAtom' => { in  => ['value','atom'],
			     out => [],
			     BI  => BIGetCtVarConstraintAsAtom},

    'getNameAsAtom'   => { in  => ['value','atom'],
			     out => [],
			     BI  => BIGetCtVarNameAsAtom},


 );
