# -*-perl-*-

%builtins_all =
    (
     'is'	=> { in  => ['!+value'],
		     out => ['+bool'],
		     BI  => BIvsIs},

     'length'  => { in  => ['!virtualString','!+int'],
		    out => ['+int'],
		    BI  => BIvsLength},
     );
1;;
