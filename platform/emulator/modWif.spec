$cmode='dyn';

%builtins_all =
(
    ##* WIF (Wish InterFace)

    'wifInit'           => { in     => ['value','value','value'],
                             out    => [],
                             BI     => BIwif_init,
                             module => 'wif',
                             native => true},

    'wifWrite'          => { in     => ['!value'],
                             out    => [],
                             BI     => BIwif_write,
                             module => 'wif',
                             native => true},

    'wifWriteReturn'    => { in     => ['!value','value','value'],
                             out    => [],
                             BI     => BIwif_writeReturn,
                             module => 'wif',
                             native => true},

    'wifWriteReturnMess'=> { in     => ['!value','value','value','value'],
                             out    => [],
                             BI     => BIwif_writeReturnMess,
                             module => 'wif',
                             native => true},

    'wifWriteBatch'     => { in     => ['!value'],
                             out    => [],
                             BI     => BIwif_writeBatch,
                             module => 'wif',
                             native => true},

    'wifWriteTuple'     => { in     => ['!value','value'],
                             out    => [],
                             BI     => BIwif_writeTuple,
                             module => 'wif',
                             native => true},

    'wifWriteTagTuple'  => { in     => ['!value','value','value'],
                             out    => [],
                             BI     => BIwif_writeTagTuple,
                             module => 'wif',
                             native => true},

    'wifWriteFilter'    => { in     => ['!value','value','value',
                                        'value','value'],
                             out    => [],
                             BI     => BIwif_writeFilter,
                             module => 'wif',
                             native => true},

    'wifClose'          => { in     => ['!value','value'],
                             out    => [],
                             BI     => BIwif_close,
                             module => 'wif',
                             native => true},

    'wifGenTopName'     => { in     => [],
                             out    => ['value'],
                             BI     => BIwif_genTopName,
                             module => 'wif',
                             native => true},

    'wifGenWidgetName'  => { in     => ['value'],
                             out    => ['value'],
                             BI     => BIwif_genWidgetName,
                             module => 'wif',
                             native => true},

    'wifGenTagName'     => { in     => [],
                             out    => ['value'],
                             BI     => BIwif_genTagName,
                             module => 'wif',
                             native => true},

    'wifGenVarName'     => { in     => [],
                             out    => ['value'],
                             BI     => BIwif_genVarName,
                             module => 'wif',
                             native => true},

    'wifGenImageName'   => { in     => [],
                             out    => ['value'],
                             BI     => BIwif_genImageName,
                             module => 'wif',
                             native => true},

    'wifGetNames'       => { in     => [],
                             out    => ['value','value','value'],
                             BI     => BIwif_getNames,
                             module => 'wif',
                             native => true},
 );

1;
