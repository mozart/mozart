$cmode='dyn';

%builtins_all =
(
    ##* Finalization

    'register'	=> { in  => ['+value','+value'],
		     out => [],
		     BI  => BIfinalize_register,
		     native => true},

    'setHandler'=> { in  => ['+value'],
		     out => [],
		     BI  => BIfinalize_setHandler,
		     native => true},

 );
