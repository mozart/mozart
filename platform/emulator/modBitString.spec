# -*-perl-*-
$module_init_fun_name = "BitString_init";

%builtins_all =
    (
     'is'       => { in  => ['+value'],
                     out => ['+bool'],
                     bi  => BIBitString_is},

     'make'     => { in  => ['+int','+[int]'],
                     out => ['+bitstring'],
                     bi  => BIBitString_make},

     'conj'     => { in  => ['+bitstring','+bitstring'],
                     out => ['+bitstring'],
                     bi  => BIBitString_conj},

     'disj'     => { in  => ['+bitstring','+bitstring'],
                     out => ['+bitstring'],
                     bi  => BIBitString_disj},

     'nega'     => { in  => ['+bitstring'],
                     out => ['+bitstring'],
                     bi  => BIBitString_nega},

     'get'      => { in  => ['+bitstring','+int'],
                     out => ['+bool'],
                     bi  => BIBitString_get},

     'put'      => { in  => ['+bitstring','+int','+bool'],
                     out => ['+bitstring'],
                     bi  => BIBitString_put},

     'width'    => { in  => ['+bitstring'],
                     out => ['+int'],
                     bi  => BIBitString_width},

     'toList'   => { in  => ['+bitstring'],
                     out => ['+[int]'],
                     bi  => BIBitString_toList}
     );

1;;
