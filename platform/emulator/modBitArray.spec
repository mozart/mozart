# -*-perl-*-

%builtins_all =
    (
     'new'             => { in  => ['+int','+int'],
                            out => ['+bitArray'],
                            BI  => BIbitArray_new},

     'is'              => { in  => ['+value'],
                            out => ['+bool'],
                            BI  => BIbitArray_is},

     'set'             => { in  => ['+bitArray','+int'],
                            out => [],
                            BI  => BIbitArray_set},

     'clear'            => { in  => ['+bitArray','+int'],
                             out => [],
                             BI  => BIbitArray_clear},

     'test'             => { in  => ['+bitArray','+int'],
                             out => ['+bool'],
                             BI  => BIbitArray_test},

     'low'              => { in  => ['+bitArray'],
                             out => ['+int'],
                             BI  => BIbitArray_low},

     'high'             => { in  => ['+bitArray'],
                             out => ['+int'],
                             BI  => BIbitArray_high},

     'clone'            => { in  => ['+bitArray'],
                             out => ['+bitArray'],
                             BI  => BIbitArray_clone},

     'or'               => { in  => ['+bitArray','+bitArray'],
                             out => [],
                             BI  => BIbitArray_or},

     'and'              => { in  => ['+bitArray','+bitArray'],
                             out => [],
                             BI  => BIbitArray_and},

     'card'             => { in  => ['+bitArray'],
                             out => ['+int'],
                             BI  => BIbitArray_card},

     'disjoint'         => { in  => ['+bitArray','+bitArray'],
                             out => ['+bool'],
                             BI  => BIbitArray_disjoint},

     'nimpl'            => { in  => ['+bitArray','+bitArray'],
                             out => [],
                             BI  => BIbitArray_nimpl},

     'toList'           => { in  => ['+bitArray'],
                             out => ['+[int]'],
                             BI  => BIbitArray_toList},

     'complementToList' => { in  => ['+bitArray'],
                             out => ['+[int]'],
                             BI  => BIbitArray_complementToList},
     );
1;;
