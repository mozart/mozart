# -*-perl-*-

%builtins_all =
    (
     'is'       => { in  => ['+value'],
                     out => ['+bool'],
                     bi  => BIisIntB},

     'toFloat'  => { in     => ['+int'],
                     out    => ['+float'],
                     bi     => BIintToFloat},

     'toString' => { in     => ['+int'],
                     out    => ['+string'],
                     BI     => BIintToString},

     'div'      => { in  => ['+int','+int'],
                     out => ['+int'],
                     bi  => BIdiv},

     'mod'      => { in  => ['+int','+int'],
                     out => ['+int'],
                     bi  => BImod},

     '+1'       => { in  => ['+int'],
                     out => ['+int'],
                     bi  => BIadd1},

     '-1'       => { in  => ['+int'],
                     out => ['+int'],
                     bi  => BIsub1},
     );
1;;
