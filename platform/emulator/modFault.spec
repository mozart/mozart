$cmode='dyn';

%builtins_all =
(
    'installHW'	        => { in  => ['value','value','value'],
			     out => [],
			     BI  => BIhwInstall,
			     native => true},

    'deInstallHW'	=>  { in  => ['value','value','value'],
			     out => [],
			     BI  => BIhwDeInstall,
			     native => true},

    'setNetBufferSize' 	=>  { in  => ['+value'],
			     out => [],
			     BI  => BIsetNetBufferSize,
			     native => true},

    'getNetBufferSize' 	=>  { in  => [],
			     out => ['value'],
			     BI  => BIgetNetBufferSize,
			     native => true},

    'getEntityCond'	=>  { in  => ['value'],
			     out => ['value'],
			     BI  => BIgetEntityCond,
			     native => true},

    'tempSimulate'	=> { in  => ['+int'],
			     out => ['+int'],
			     BI  => BIcloseCon,
			     module=>'perdio',
			     native => true},

 );
