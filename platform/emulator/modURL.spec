$cmode='dyn';

%builtins_all =
(
    #* URL

    'localize'  => { in  => ['+virtualString'],
                             out => ['+record'],
                             BI  => BIurl_localize,
                             native => true},

    'open'              => { in  => ['+virtualString'],
                             out => ['+int'],
                             BI  => BIurl_open,
                             native => true},

    'load'              => { in  => ['+virtualString'],
                             out => ['value'],
                             BI  => BIurl_load,
                             native => true},

 );
