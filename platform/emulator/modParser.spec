# -*- perl -*-

$cmode='dyn';

%builtins_all =
(
    ###* Parser

    'Parser.parseFile'		=> { in     => ['+virtualString','+record'],
				     out    => ['+value'],
				     bi     => parser_parseFile,
				     module => libparser,
				     native => true},

    'Parser.parseVirtualString' => { in     => ['+virtualString','+record'],
				     out    => ['+value'],
				     bi     => parser_parseVirtualString,
				     module => libparser,
				     native => true},

 );
