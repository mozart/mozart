# -*-perl-*-

%builtins_all =
    (
     'is'    => { in  => ['+value'],
                  out => ['+bool'],
                  bi  => BIisArray},

     'new'   => { in  => ['+int','+int','value'],
                  out => ['+array'],
                  BI  => BIarrayNew},

     'high'  => { in  => ['+array'],
                  out => ['+int'],
                  bi  => BIarrayHigh},

     'low'   => { in  => ['+array'],
                  out => ['+int'],
                  bi  => BIarrayLow},

     'get'   => { in  => ['+array','+int'],
                  out => ['value'],
                  bi  => BIarrayGet},

     'put'   => { in  => ['+array','+int','value'],
                  out => [],
                  bi  => BIarrayPut}
     );
1;;
