# -*-perl-*-

%builtins_all =
    (
     'is'    => { in  => ['+value'],
		  out => ['+bool'],
		  bi  => BIisProcedureB},

     'arity' => { in  => ['+procedure'],
		  out => ['+int'],
		  bi  => BIprocedureArity},
     );
1;;
