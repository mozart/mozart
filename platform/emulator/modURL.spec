%builtins_all =
(
    #* URL

    'localize'	=> { in  => ['+virtualString'],
			     out => ['+record'],
			     BI  => BIurl_localize},

    'open'		=> { in  => ['+virtualString'],
			     out => ['+int'],
			     BI  => BIurl_open},

    'load'		=> { in  => ['+virtualString'],
			     out => ['value'],
			     BI  => BIurl_load},

 );
