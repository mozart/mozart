### -*-perl-*-

%builtins_all =
    (
     'is'       => { in  => ['+value'],
                     out => ['+bool'],
                     BI  => weakdict_is },

     'new'      => { in  => [],
                     out => ['[feature#value]','+value'],
                     BI  => weakdict_new },

     'put'      => { in  => ['+value','+feature','+value'],
                     out => [],
                     BI  => weakdict_put },

     'get'      => { in  => ['+value','+feature'],
                     out => ['+value'],
                     BI  => weakdict_get },

     'close'    => { in  => ['+value'],
                     out => [],
                     BI  => weakdict_close }
     );
