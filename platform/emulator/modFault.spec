%builtins_all =
(
    'installHW'         => { in  => ['value','value','value'],
                             out => [],
                             BI  => BIhwInstall},

    'deInstallHW'       =>  { in  => ['value','value','value'],
                             out => [],
                             BI  => BIhwDeInstall},

    'setNetBufferSize'  =>  { in  => ['+value'],
                             out => [],
                             BI  => BIsetNetBufferSize},

    'getNetBufferSize'  =>  { in  => [],
                             out => ['value'],
                             BI  => BIgetNetBufferSize},

    'getEntityCond'     =>  { in  => ['value'],
                             out => ['value'],
                             BI  => BIgetEntityCond},

    'tempSimulate'      => { in  => ['+int'],
                             out => ['+int'],
                             BI  => BIcloseCon},

 );
