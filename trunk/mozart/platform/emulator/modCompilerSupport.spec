$cmode='dyn';

%builtins_all =
(
    ##* CompilerSupport

    'concatenateAtomAndInt' => { in  => ['+atom','+int'],
					  out => ['+atom'],
					  BI  => BIconcatenateAtomAndInt,
					  native => true},

    'isBuiltin' => { in  => ['+value'],
			      out => ['+bool'],
			      BI  => BIisBuiltin,
			      native => true},

    'nameVariable' => { in  => ['value','+atom'],
				 out => [],
				 BI  => BInameVariable,
				 native => true},

    'newNamedName' => { in  => ['+atom'],
				 out => ['+literal'],
				 BI  => BInewNamedName,
				 native => true},

    'newCopyableName' => { in  => ['+atom'],
				    out => ['+literal'],
				    BI  => BInewCopyableName,
				    native => true},

    'isCopyableName' => { in  => ['+value'],
				   out => ['+bool'],
				   BI  => BIisCopyableName,
				   native => true},

    'isUniqueName' => { in  => ['+value'],
				 out => ['+bool'],
				 BI  => BIisUniqueName,
				 native => true},

    'newPredicateRef' => { in  => [],
				    out => ['+foreignPointer'],
				    BI  => BInewPredicateRef,
				    native => true},

    'newCopyablePredicateRef' => { in  => [],
					    out => ['+foreignPointer'],
					    BI  => BInewCopyablePredicateRef,
					    native => true},

    'isCopyablePredicateRef' => { in  => ['+foreignPointer'],
					   out => ['+bool'],
					   BI  => BIisCopyablePredicateRef,
					   native => true},

 );
