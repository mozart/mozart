$cmode='dyn';

%builtins_all =
(
    #* Connection

    'get'               => { in  => [],
                             out => ['+record'],
                             BI  => BIGetPID,
                             native => true},

    'received'  => { in  => ['value'],
                             out => [],
                             BI  => BIReceivedPID,
                             native => true},

    'toPort'    => { in  => ['+virtualString','+int','+int','+int'],
                             out => ['+port'],
                             BI  => BITicket2Port,
                             native => true},

 );
