# -*-perl-*-

%builtins_all =
    (
     'is'               => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIisNameB},

     'new'              => { in  => [],
                             out => ['+name'],
                             BI  => BInewName},

     'newUnique'        => { in  => ['+atom'],
                             out => ['+name'],
                             BI  => BInewUniqueName},
     );
1;;
