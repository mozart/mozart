$cmode='dyn';

%builtins_all =
(
##* Printing

 'print'	=> { in     => ['value'],
		     out    => [],
		     bi     => BIprint,
		     native => true},

 'show'		=> { in     => ['value'],
		     out    => [],
		     bi     => BIshow,
		     native => true},
 
 'printName'	=> { in     => ['value'],
		     out    => ['+atom'],
		     BI     => BIgetPrintName,
		     native => true},

 'printInfo'	=> { in     => ['virtualString'],
		     out    => [],
		     BI     => BIprintInfo,
		     native => true},

 'printError'	=> { in     => ['virtualString'],
		     out    => [],
		     BI     => BIprintError,
		     native => true},

 'showInfo'	=> { in     => ['virtualString'],
		     out    => [],
		     BI     => BIshowInfo,
		     native => true},

 'showError'	=> { in     => ['virtualString'],
		     out    => [],
		     BI     => BIshowError,
		     native => true},
 
 'valueToVirtualString'=> { in     => ['value','+int','+int'],
			    out    => ['+string'],
			    BI     => BItermToVS,
			    native => true},

 'exit'		=> { in  => ['+int'],
		     out => [],
		     BI  => BIshutdown,
		     doesNotReturn => 1,
		     native => true},

 'gcDo'	=> { in  => [],
	     out => [],
	     BI  => BIgarbageCollection,
	     native => true},
 
 'apply'	=> { in  => ['+procedureOrObject','+[value]'],
		     out => [],
		     BI  => BIapply,
		     native => true},

 'eq'		=> { in  => ['value','value'],
		     out => ['+bool'],
		     BI  => BIsystemEq,
		     native => true},
 
 'nbSusps'	=> { in  => ['value'],
		     out => ['+int'],
		     BI  => BIconstraints,
		     native => true},

  'onToplevel'	=> { in  => [],
			     out => ['+bool'],
			     BI  => BIonToplevel,
			     native => true},


 );
