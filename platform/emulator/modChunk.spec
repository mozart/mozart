# -*-perl-*-

%builtins_all =
    (
     'is'	=> { in  => ['+value'],
		     out => ['+bool'],
		     bi  => BIisChunkB},

     'new'	=> { in  => ['+record'],
		     out => ['+chunk'],
		     BI  => BInewChunk},
     );
1;;
