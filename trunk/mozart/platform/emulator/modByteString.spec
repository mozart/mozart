# -*-perl-*-
#$module_init_fun_name = "ByteString_init";

%builtins_all =
    (

     'is'	=> { in  => ['+value'],
		     out => ['+bool'],
		     bi  => BIByteString_is},

     'make'	=> { in  => ['+string'],
		     out => ['+byteString'],
		     bi  => BIByteString_make},

     'get'	=> { in  => ['+byteString','+int'],
		     out => ['+int'],
		     bi  => BIByteString_get},

     'append'	=> { in  => ['+byteString','+byteString'],
		     out => ['+byteString'],
		     bi  => BIByteString_append},

     'slice'	=> { in  => ['+byteString','+int','+int'],
		     out => ['+byteString'],
		     bi  => BIByteString_slice},

     'width'	=> { in  => ['+byteString'],
		     out => ['+int'],
		     bi  => BIByteString_width},

     'toString'	=> { in  => ['+byteString'],
		     out => ['+string'],
		     bi  => BIByteString_toString}
     );
1;;
