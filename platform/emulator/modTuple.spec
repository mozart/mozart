# -*-perl-*-

%builtins_all =
    (
     'is'               => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisTupleB},

     'make'             => { in  => ['+literal','+int'],
                             out => ['+tuple'],
                             bi  => BItuple},
     );
1;;
