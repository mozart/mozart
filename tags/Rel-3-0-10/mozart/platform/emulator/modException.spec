# -*-perl-*-

%builtins_all =
    (
     'raise'	       => { in  => ['value'],
			    out => [],
			    BI  => BIraise,
			    doesNotReturn => 1},

     'raiseError'      => { in  => ['value'],
			    out => [],
			    BI  => BIraiseError,
			    doesNotReturn => 1},

     'raiseDebug'      => { in  => ['value'],
			    out => [],
			    BI  => BIraiseDebug,
			    doesNotReturn => 1},

     'raiseDebugCheck' => { in  => ['value'],
			    out => ['+bool'],
			    BI  => BIraiseDebugCheck},

     'taskStackError'  => { in  => [],
			    out => ['+[record]'],
			    BI  => BIexceptionTaskStackError},

     'location'	       => { in  => [],
			    out => ['+[atom]'],
			    BI  => BIexceptionLocation},
     );
1;;
