$cmode='dyn';

%builtins_all =
(
    #* Pickles

    'save'		=> { in     => ['value','+virtualString'],
			     out    => [],
			     BI     => BIsave,
			     native => true},

    'load'		=> { in     => ['value','value'],
			     out    => [],
			     BI     => BIload,
			     native => true},


 );
