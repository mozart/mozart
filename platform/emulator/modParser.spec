# -*- perl -*-

$cmode='dyn';

%builtins_all =
(
    ###* Parser

    'file'              => { in     => ['+virtualString','+record'],
                             out    => ['+value'],
                             bi     => parser_parseFile,
                             module => libparser,
                             native => true},

    'virtualString' => { in     => ['+virtualString','+record'],
                         out    => ['+value'],
                         bi     => parser_parseVirtualString,
                         module => libparser,
                         native => true},

 );
