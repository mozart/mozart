$cmode='dyn';

%builtins_all =
(
    #* Constraint variables

    'isB'               => { in  => ['value'],
                             out => ['+bool'],
                             BI  => BIIsGenCtVarB,
                             module =>  ct,
                             native => true},

    'getConstraintAsAtom' => { in  => ['value','atom'],
                             out => [],
                             BI  => BIGetCtVarConstraintAsAtom,
                             module => ct,
                             native => true},

    'getNameAsAtom'   => { in  => ['value','atom'],
                             out => [],
                             BI  => BIGetCtVarNameAsAtom,
                             module =>  ct,
                             native => true},

 );
