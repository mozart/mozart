%builtins_all =
(
    #* Connection

    'get'               => { in  => [],
                             out => ['+record'],
                             BI  => BIGetPID},

    'received'  => { in  => ['value'],
                             out => [],
                             BI  => BIReceivedPID},

    'toPort'    => { in  => ['+virtualString','+int','+int','+int'],
                             out => ['+port'],
                             BI  => BITicket2Port},

 );
