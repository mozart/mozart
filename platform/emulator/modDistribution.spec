$cmode='dyn';

%builtins_all =
(
    'export'		=> { in     => ['value'],
			     out    => [],
			     BI     => BIexport,
			     native => true},

    # all these are needed within emulator 

    'controlVarHandler'	=> { in  => ['+value'],
			     out => [],
			     BI  => BIcontrolVarHandler,
			     native => true},

    'probe'		=> { in  => ['value'],
			     out => [],
			     BI  => BIprobe,
			     native => true},

    'startTmp'		=> { in  => ['+int','+int'],
			     out => [],
			     BI  => BIstartTmp,
			     module=>'perdio',
			     native => true},

    'portWait'         =>  { in  => ['+port','+int'],
			     out => [],
			     BI  => BIportWait,
			     module=>'perdio',
			     native => true},

     'atRedo'		=> { in  => ['+feature', 'value'],
			     out => [],
			     bi  => BIatRedo,
			     native => true},
    

 );
