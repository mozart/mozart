# -*-perl-*-
$module_init_fun_name = "BitString_init";

%builtins_all =
    (
     'BitString.is'     => { in  => ['+value'],
                             out => ['+bool'],
                             bi  => BIBitString_is},

     'BitString.make'   => { in  => ['+[int]'],
                             out => ['+bitstring'],
                             bi  => BIBitString_make},

     'BitString.conj'   => { in  => ['+bitstring','+bitstring'],
                             out => ['+bitstring'],
                             bi  => BIBitString_conj},

     'BitString.disj'   => { in  => ['+bitstring','+bitstring'],
                             out => ['+bitstring'],
                             bi  => BIBitString_disj},

     'BitString.nega'   => { in  => ['+bitstring'],
                             out => ['+bitstring'],
                             bi  => BIBitString_nega},

     'BitString.get'    => { in  => ['+bitstring','+int'],
                             out => ['+bool'],
                             bi  => BIBitString_get},

     'BitString.put'    => { in  => ['+bitstring','+int','+bool'],
                             out => ['+bitstring'],
                             bi  => BIBitString_put},

     'BitString.width'  => { in  => ['+bitstring'],
                             out => ['+int'],
                             bi  => BIBitString_width},

     'BitString.toList' => { in  => ['+bitstring'],
                             out => ['+[int]'],
                             bi  => BIBitString_toList}
     );

1;;
