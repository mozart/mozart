%builtins_all =
(
    'newMailbox' => { in     => [],
				  out    => ['+string'],
				  BI     => BIVSnewMailbox},

    'initServer' => { in     => ['+string'],
				  out    => [],
				  BI     => BIVSinitServer},

    'removeMailbox' => { in     => ['+string'],
				  out    => [],
				  BI     => BIVSremoveMailbox},

 );
