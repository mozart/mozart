%builtins_all =
(
    'save'              => { in     => ['value','+virtualString'],
                             out    => [],
                             BI     => BIsave},

    'gzsave'            => { in     => ['value','+virtualString','+int'],
                             out    => [],
                             BI     => BIgzsave},

    'load'              => { in     => ['value','value'],
                             out    => [],
                             BI     => BIload},

 );
