# -*- perl -*-

%builtins_all =
(
    'file'              => { in     => ['+virtualString','+record'],
                             out    => ['+value'],
                             bi     => parser_parseFile},

    'virtualString' => { in     => ['+virtualString','+record'],
                         out    => ['+value'],
                         bi     => parser_parseVirtualString},

 );
