# -*-perl-*-

%builtins_all =
    (
     'is'	=> { in  => ['+value'],
		     out => ['+bool'],
		     bi  => BIisAtomB},

     'toString'	=> { in  => ['+atom'],
		     out => ['+string'],
		     bi  => BIatomToString},

     );
1;;
