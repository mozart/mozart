$cmode='dyn';

%builtins_all =
(
    'newMailbox' => { in     => [],
				  out    => ['+string'],
				  BI     => BIVSnewMailbox,
				  native => true},

    'initServer' => { in     => ['+string'],
				  out    => [],
				  BI     => BIVSinitServer,
				  native => true},

    'removeMailbox' => { in     => ['+string'],
				  out    => [],
				  BI     => BIVSremoveMailbox,
				  native => true},

 );
