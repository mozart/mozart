# -*-perl-*-

%builtins_all =
    (
     'is'    => { in  => ['+value'],
                  out => ['+bool'],
                  BI  => BIisForeignPointer},

     'toInt' => { in  => ['+foreignPointer'],
                  out => ['+int'],
                  BI  => BIForeignPointerToInt},
     );
1;;
