# -*-perl-*-

%builtins_all =
    (
     'is'       => { in  => ['+value'],
                     out => ['+bool'],
                     bi  => BIisNumberB},

     'abs'      => { in  => ['+number'],
                     out => ['+number'],
                     bi  => BIabs},

     '/'        => { in  => ['+float','+float'],
                     out => ['+float'],
                     bi  => BIfdiv},

     '*'        => { in  => ['+number','+number'],
                     out => ['+number'],
                     bi  => BImult},

     '-'        => { in  => ['+number','+number'],
                     out => ['+number'],
                     bi  => BIminus},

     '+'        => { in  => ['+number','+number'],
                     out => ['+number'],
                     bi  => BIplus},

     '~'        => { in  => ['+number'],
                     out => ['+number'],
                     bi  => BIuminus},
     );
1;;
