### -*-perl-*-

%builtins_all =
    (
     'setTimer'		=> { in  => ['+int'],
			     out => [],
			     BI  => BItimer_setTimer },

     'mTime'		=> { in  => [],
			     out => ['+int'],
			     BI  => BItimer_mTime }
     );

