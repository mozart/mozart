%builtins_all =
(
    'get'	=> { in  => ['+literal'],
			     out => ['value'],
			     BI  => BIgetProperty},

    'condGet'	=> { in  => ['+literal','value'],
			     out => ['value'],
			     BI  => BIcondGetProperty},

    'put'	=> { in  => ['+literal','value'],
			     out => [],
			     BI  => BIputProperty},

 );
