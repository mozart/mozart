# -*-perl-*-

%builtins_all =
    (
     'is'	=> { in  => ['!+value'],
		     out => ['+bool'],
		     BI  => BIvsIs},

     'length'  => { in  => ['!virtualString','!+int'],
		    out => ['+int'],
		    BI  => BIvsLength},

     'toByteString'	=> { in  => ['!+virtualString','!+int',
				     '+virtualString'],
			     out => ['+byteString'],
			     bi  => BIvsToBs},
     );
1;;
