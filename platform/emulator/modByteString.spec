# -*-perl-*-
$module_init_fun_name = "ByteString_init";

%builtins_all =
    (

     'is'       => { in  => ['+value'],
                     out => ['+bool'],
                     bi  => BIByteString_is},

     'make'     => { in  => ['+string'],
                     out => ['+bytestring'],
                     bi  => BIByteString_make},

     'get'      => { in  => ['+bytestring','+int'],
                     out => ['+int'],
                     bi  => BIByteString_get},

     'append'   => { in  => ['+bytestring','+bytestring'],
                     out => ['+bytestring'],
                     bi  => BIByteString_append},

     'slice'    => { in  => ['+bytestring','+int','+int'],
                     out => ['+bytestring'],
                     bi  => BIByteString_slice},

     'width'    => { in  => ['+bytestring'],
                     out => ['+int'],
                     bi  => BIByteString_width},

     'toString' => { in  => ['+bytestring'],
                     out => ['+string'],
                     bi  => BIByteString_toString}
     );
1;;
