$cmode='dyn';

%builtins_all =
(
    #* URL

    'URL.localize'	=> { in  => ['+virtualString'],
			     out => ['+record'],
			     BI  => BIurl_localize,
			     native => true},

    'URL.open'		=> { in  => ['+virtualString'],
			     out => ['+int'],
			     BI  => BIurl_open,
			     native => true},

    'URL.load'		=> { in  => ['+virtualString'],
			     out => ['value'],
			     BI  => BIurl_load,
			     native => true},

 );
