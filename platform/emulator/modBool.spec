# -*-perl-*-

%builtins_all =
    (
     'is'       => { in  => ['+value'],
                     out => ['+bool'],
                     bi  => BIisBoolB},

     'not'      => { in  => ['+bool'],
                     out => ['+bool'],
                     bi  => BInot},

     'and'      => { in  => ['+bool','+bool'],
                     out => ['+bool'],
                     bi  => BIand},

     'or'       => { in  => ['+bool','+bool'],
                     out => ['+bool'],
                     bi  => BIor},
     );
1;;
