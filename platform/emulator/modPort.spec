# -*-perl-*-

%builtins_all =
    (
     'is'       => { in  => ['+value'],
                     out => ['+bool'],
                     bi  => BIisPortB},

     'new'      => { in  => ['value'],
                     out => ['+port'],
                     BI  => BInewPort},

     'send'     => { in  => ['+port','value'],
                     out => [],
                     BI  => BIsendPort},
     );
1;;
