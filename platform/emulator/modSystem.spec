%builtins_all =
(

 'print'                => { in     => ['value'],
                             out    => [],
                             bi     => BIprint},

 'show'                 => { in     => ['value'],
                             out    => [],
                             bi     => BIshow},

 'printName'            => { in     => ['value'],
                             out    => ['+atom'],
                             BI     => BIgetPrintName},

 'printInfo'            => { in     => ['virtualString'],
                             out    => [],
                             BI     => BIprintInfo},

 'printError'           => { in     => ['virtualString'],
                             out    => [],
                             BI     => BIprintError},

 'showInfo'             => { in     => ['virtualString'],
                             out    => [],
                             BI     => BIshowInfo},

 'showError'            => { in     => ['virtualString'],
                             out    => [],
                             BI     => BIshowError},

 'valueToVirtualString' => { in     => ['value','+int','+int'],
                             out    => ['+string'],
                             BI     => BItermToVS},

 'exit'                 => { in  => ['+int'],
                             out => [],
                             BI  => BIshutdown,
                             doesNotReturn => 1},

 'gcDo'                 => { in  => [],
                             out => [],
                             BI  => BIgarbageCollection},

 'apply'                => { in  => ['+procedureOrObject','+[value]'],
                             out => [],
                             BI  => BIapply},

 'eq'                   => { in  => ['value','value'],
                             out => ['+bool'],
                             BI  => BIsystemEq},

 'nbSusps'              => { in  => ['value'],
                             out => ['+int'],
                             BI  => BIconstraints},

  'onToplevel'          => { in  => [],
                             out => ['+bool'],
                             BI  => BIonToplevel},


 );
