# -*-perl-*-

%builtins_all =
    (
     'is'       => { in  => ['+value'],
                     out => ['+bool'],
                     bi  => BIisLockB},

     'new'      => { in  => [],
                     out => ['+lock'],
                     BI  => BInewLock},

     'lock'     => { in  => ['+lock'],
                     out => [],
                     BI  => BIlockLock},

     'unlock'   => { in  => ['+lock'],
                     out => [],
                     BI  => BIunlockLock},
     );
1;;
