# -*-perl-*-
$module_init_fun_name = "ByteString_init";

%builtins_all =
    (

     'ByteString.is'    => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIByteString_is},

     'ByteString.make'  => { in  => ['+string'],
                             out => ['+bytestring'],
                             bi  => BIByteString_make},

     'ByteString.get'   => { in  => ['+bytestring','+int'],
                             out => ['+int'],
                             bi  => BIByteString_get},

     'ByteString.append'=> { in  => ['+bytestring','+bytestring'],
                             out => ['+bytestring'],
                             bi  => BIByteString_append},

     'ByteString.slice' => { in  => ['+bytestring','+int','+int'],
                             out => ['+bytestring'],
                             bi  => BIByteString_slice},

     'ByteString.width' => { in  => ['+bytestring'],
                             out => ['+int'],
                             bi  => BIByteString_width},

     'ByteString.toString'=> { in  => ['+bytestring'],
                             out => ['+string'],
                             bi  => BIByteString_toList}
     );
1;;
