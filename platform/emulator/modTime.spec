# -*-perl-*-

%builtins_all =
    (
     'alarm'            => { in  => ['+int','unit'],
                             out => [],
                             BI  => BIalarm},

     'delay'            => { in  => ['!+int'],
                             out => [],
                             BI  => BIdelay},

     'time'             => { in  => [],
                             out => ['+int'],
                             BI  => BItimeTime},
     );
1;;
