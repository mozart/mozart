$cmode='dyn';

%builtins_all =
(
    ##* WIF (Wish InterFace)

    'init'	        => { in     => ['value','value','value'],
			     out    => [],
			     BI     => BIwif_init,
			     module => 'wif',
			     native => true},

    'write'		=> { in     => ['!value'],
			     out    => [],
			     BI     => BIwif_write,
			     module => 'wif',
			     native => true},

    'writeReturn'	=> { in     => ['!value','value','value'],
			     out    => [],
			     BI     => BIwif_writeReturn,
			     module => 'wif',
			     native => true},

    'writeReturnMess'=> { in     => ['!value','value','value','value'],
			     out    => [],
			     BI     => BIwif_writeReturnMess,
			     module => 'wif',
			     native => true},

    'writeBatch'	=> { in     => ['!value'],
			     out    => [],
			     BI     => BIwif_writeBatch,
			     module => 'wif',
			     native => true},

    'writeTuple'	=> { in     => ['!value','value'],
			     out    => [],
			     BI     => BIwif_writeTuple,
			     module => 'wif',
			     native => true},

    'writeTagTuple'	=> { in     => ['!value','value','value'],
			     out    => [],
			     BI     => BIwif_writeTagTuple,
			     module => 'wif',
			     native => true},

    'writeFilter'	=> { in     => ['!value','value','value',
				        'value','value'],
			     out    => [],
			     BI     => BIwif_writeFilter,
			     module => 'wif',
			     native => true},

    'close'		=> { in     => ['!value','value'],
			     out    => [],
			     BI     => BIwif_close,
			     module => 'wif',
			     native => true},

    'genTopName'	=> { in     => [],
			     out    => ['value'],
			     BI     => BIwif_genTopName,
			     module => 'wif',
			     native => true},

    'genWidgetName'	=> { in     => ['value'],
			     out    => ['value'],
			     BI     => BIwif_genWidgetName,
			     module => 'wif',
			     native => true},

    'genTagName'	=> { in     => [],
			     out    => ['value'],
			     BI     => BIwif_genTagName,
			     module => 'wif',
			     native => true},

    'genVarName'	=> { in     => [],
			     out    => ['value'],
			     BI     => BIwif_genVarName,
			     module => 'wif',
			     native => true},

    'genImageName'	=> { in     => [],
			     out    => ['value'],
			     BI     => BIwif_genImageName,
			     module => 'wif',
			     native => true},

    'getNames'	=> { in     => [],
			     out    => ['value','value','value'],
			     BI     => BIwif_getNames,
			     module => 'wif',
			     native => true},
 );

1;
