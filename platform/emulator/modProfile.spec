%builtins_all =
(

 'mode'         => { in  => ['+bool'],
                     out => [],
                     BI  => BIsetProfileMode},

 'reset'        => { in  => [],
                     out => [],
                     BI  => BIstatisticsReset},

 'getInfo'      => { in  => [],
                     out => ['+value'],
                     BI  => BIstatisticsGetProcs},

 );
