%builtins_all =
(
    ##* Finalization

    'register'	=> { in  => ['+value','+value'],
		     out => [],
		     BI  => BIfinalize_register},

    'setHandler'=> { in  => ['+value'],
		     out => [],
		     BI  => BIfinalize_setHandler},

 );
