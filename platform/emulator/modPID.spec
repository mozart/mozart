$cmode='dyn';

%builtins_all =
(
    #* Connection

    'PID.get'		=> { in  => [],
			     out => ['+record'],
			     BI  => BIGetPID,
			     native => true},

    'PID.received'	=> { in  => ['value'],
			     out => [],
			     BI  => BIReceivedPID,
			     native => true},

    'PID.close'		=> { in  => [],
			     out => [],
			     BI  => BIClosePID,
			     native => true},

    'PID.send'		=> { in  => ['+virtualString','+int','+int','+int','+int','value'],
			     out => [],
			     BI  => BISendPID,
			     native => true},

    'PID.toPort'	=> { in  => ['+virtualString','+int','+int','+int'],
			     out => ['+port'],
			     BI  => BITicket2Port,
			     native => true},

 );
