$cmode='dyn';

%builtins_all =
(
 #* Profiling support

 'mode'	        => { in  => ['+bool'],
		     out => [],
		     BI  => BIsetProfileMode,
		     native => true},
 
 'reset'	=> { in  => [],
		     out => [],
		     BI  => BIstatisticsReset,
		     native => true},

 'getInfo'      => { in  => [],
		     out => ['+value'],
		     BI  => BIstatisticsGetProcs,
		     native => true},

 );
