# -*-perl-*-
#$module_init_fun_name = "BitString_init";

%builtins_all =
    (
     'is'       => { in  => ['+value'],
                     out => ['+bool'],
                     bi  => BIBitString_is},

     'make'     => { in  => ['+int','+[int]'],
                     out => ['+bitString'],
                     bi  => BIBitString_make},

     'conj'     => { in  => ['+bitString','+bitString'],
                     out => ['+bitString'],
                     bi  => BIBitString_conj},

     'disj'     => { in  => ['+bitString','+bitString'],
                     out => ['+bitString'],
                     bi  => BIBitString_disj},

     'nega'     => { in  => ['+bitString'],
                     out => ['+bitString'],
                     bi  => BIBitString_nega},

     'get'      => { in  => ['+bitString','+int'],
                     out => ['+bool'],
                     bi  => BIBitString_get},

     'put'      => { in  => ['+bitString','+int','+bool'],
                     out => ['+bitString'],
                     bi  => BIBitString_put},

     'width'    => { in  => ['+bitString'],
                     out => ['+int'],
                     bi  => BIBitString_width},

     'toList'   => { in  => ['+bitString'],
                     out => ['+[int]'],
                     bi  => BIBitString_toList}
     );

1;;
