$cmode='dyn';

%builtins_all =
(
    ###* Parser

    'parser_parseFile'          => { in     => ['+virtualString','+record'],
                                     out    => ['+value'],
                                     bi     => parser_parseFile,
                                     module => libparser,
                                     native => true},

    'parser_parseVirtualString' => { in     => ['+virtualString','+record'],
                                     out    => ['+value'],
                                     bi     => parser_parseVirtualString,
                                     module => libparser,
                                     native => true},

 );
