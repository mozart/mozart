$cmode='dyn';

%builtins_all =
(
    ##* Virtual Properties

    'get'	=> { in  => ['+literal'],
			     out => ['value'],
			     BI  => BIgetProperty,
			     module=> 'vprop',
			     native => true},

    'condGet'	=> { in  => ['+literal','value'],
			     out => ['value'],
			     BI  => BIcondGetProperty,
			     module=> 'vprop',
			     native => true},

    'put'	=> { in  => ['+literal','value'],
			     out => [],
			     BI  => BIputProperty,
			     module=>'vprop',
			     native => true},



 );
