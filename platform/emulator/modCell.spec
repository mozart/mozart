# -*-perl-*-

%builtins_all =
    (
     'is'		=> { in  => ['+value'],
			     out => ['+bool'],
			     bi  => BIisCellB},

     'new'		=> { in  => ['value'],
			     out => ['+cell'],
			     BI  => BInewCell},

     'exchange'		=> { in  => ['+cell','value','value'],
			     out => [],
			     bi  => BIexchangeCell},

     'access'		=> { in  => ['+cell'],
			     out => ['value'],
			     bi  => BIaccessCell},

     'assign'		=> { in  => ['+cell','value'],
			     out => [],
			     bi  => BIassignCell},
     );
1;;
