# -*-perl-*-

%builtins_all =
    (
     'is'       => { in  => ['+value'],
                     out => ['+bool'],
                     BI  => BIisString},

     'toAtom'   => { in  => ['+string'],
                     out => ['+atom'],
                     BI  => BIstringToAtom},

     'toInt'    => { in  => ['+string'],
                     out => ['+int'],
                     BI  => BIstringToInt},

     'toFloat'  => { in  => ['+string'],
                     out => ['+float'],
                     BI  => BIstringToFloat},
     );
1;;
