$cmode='dyn';

%builtins_all = 
(
 
 #
 # Distribution stuff
 #
 'selVarNaive'	=> { in     => ['+tuple'],
			     out    => ['*int'],
			     bi     => BIfdd_selVarNaive,
			     module => libfd,
			     native => true},
 
 'selVarSize'	=> { in     => ['+tuple'],
			     out    => ['*int'],
			     bi     => BIfdd_selVarSize,
			     module => libfd,
			     native => true},
 
 'selVarMin'	=> { in     => ['+tuple'],
			     out    => ['*int'],
			     bi     => BIfdd_selVarMin,
			     module => libfd,
			     native => true},
 
 'selVarMax'	=> { in     => ['+tuple'],
			     out    => ['*int'],
			     bi     => BIfdd_selVarMax,
			     module => libfd,
			     native => true},
 
 'selVarNbSusps' => { in     => ['+tuple'],
			  out    => ['*int'],
			  bi     => BIfdd_selVarNbSusps,
			  module => libfd,
			  native => true},

 #
 # Propagators
 #
 'sum'		=> { in  => ['+value','+atom','int'],
			     out => [],
			     bi  => fdp_sum,
			     module => libfd,
			     native => true},

 'sumC'		=> { in  => ['+value','+value','+atom','int'],
			     out => [],
			     bi  => fdp_sumC,
			     module => libfd,
			     native => true},
 
 'sumCN'		=> { in  => ['+value','+value','+atom','int'],
			     out => [],
			     bi  => fdp_sumCN,
			     module => libfd,
			     native => true},
 
 'sumR'		=> { in  => ['+value','+atom','*int','*int'],
			     out => [],
			     bi  => fdp_sumR,
			     module => libfd,
			     native => true},
 
 'sumCR'		=> { in  => ['+value','+value','+atom','*int','*int'],
			     out => [],
			     bi  => fdp_sumCR,
			     module => libfd,
			     native => true},
 
 'sumCNR'	        => { in  => ['+value','+value','+atom','*int','*int'],
			     out => [],
			     bi  => fdp_sumCNR,
			     module => libfd,
			     native => true},
 
 'sumCD'		=> { in  => ['+value','+atom','*int','*int'],
			     out => [],
			     bi  => fdp_sumCD,
			     module => libfd,
			     native => true},
 
 'sumCCD'	=> { in  => ['+value','+value','+atom','*int','*int'],
		     out => [],
		     bi  => fdp_sumCCD,
		     module => libfd,
		     native => true},
 
 'sumCNCD'	=> { in  => ['+value','+value','+atom','*int','*int'],
		     out => [],
		     bi  => fdp_sumCNCD,
		     module => libfd,
		     native => true},
 
 'plus'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_plus,
			     module => libfd,
			     native => true},
 
 'minus'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_minus,
			     module => libfd,
			     native => true},
 
 'times'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_times,
			     module => libfd,
                             native => true},
 
 'power'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_power,
			     module => libfd,
			     native => true},
 
 'divD'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_divD,
			     module => libfd,
			     native => true},
 
 'divI'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_divI,
			     module => libfd,
			     native => true},
 
 'modD'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_modD,
			     module => libfd,
			     native => true},
 
 'modI'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_modI,
			     module => libfd,
			     native => true},
 
 'conj'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_conj,
			     module => libfd,
			     native => true},
 
 'disj'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_disj,
			     module => libfd,
			     native => true},
 
 'exor'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_exor,
			     module => libfd,
			     native => true},
 
 'impl'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_impl,
			     module => libfd,
			     native => true},
 
 'equi'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_equi,
			     module => libfd,
			     native => true},
 
 'nega'		=> { in  => ['int','int'],
			     out => [],
			     bi  => fdp_nega,
			     module => libfd,
			     native => true},
 
 'intR'		=> { in  => ['int','+value','int'],
			     out => [],
			     bi  => fdp_intR,
			     module => libfd,
			     native => true},
 
 'card'		=> { in  => ['+value','int','int','int'],
			     out => [],
			     bi  => fdp_card,
			     module => libfd,
			     native => true},
 
 'exactly'	=> { in  => ['int','+value','+int'],
		     out => [],
		     bi  => fdp_exactly,
		     module => libfd,
		     native => true},
 
 'atLeast'	=> { in  => ['int','+value','+int'],
		     out => [],
		     bi  => fdp_atLeast,
		     module => libfd,
		     native => true},
 
 'atMost'	=> { in  => ['int','+value','+int'],
		     out => [],
		     bi  => fdp_atMost,
		     module => libfd,
		     native => true},
 
 'element'	=> { in  => ['int','+value','int'],
		     out => [],
		     bi  => fdp_element,
		     module => libfd,
		     native => true},
 
 'lessEqOff'	=> { in  => ['int','int','+int'],
			     out => [],
			     bi  => fdp_lessEqOff,
			     module => libfd,
			     native => true},
 
 'minimum'	=> { in  => ['int','int','int'],
		     out => [],
		     bi  => fdp_minimum,
		     module => libfd,
		     native => true},
 
 'maximum'	=> { in  => ['int','int','int'],
		     out => [],
		     bi  => fdp_maximum,
		     module => libfd,
		     native => true},
 
 'inter'	=> { in  => ['int','int','int'],
		     out => [],
		     bi  => fdp_inter,
		     module => libfd,
		     native => true},
 
 'union'	=> { in  => ['int','int','int'],
		     out => [],
		     bi  => fdp_union,
		     module => libfd,
		     native => true},
 
 'distinct'	=> { in  => ['+value'],
		     out => [],
		     bi  => fdp_distinct,
		     module => libfd,
		     native => true},
 
 'distinctD'	=> { in  => ['+value'],
			     out => [],
			     bi  => fdp_distinctD,
			     module => libfd,
			     native => true},
 
 'distinctStream'=> { in  => ['+value','value'],
			  out => [],
			  bi  => fdp_distinctStream,
			  module => libfd,
			  native => true},
 
 'distinctOffset'=> { in  => ['+value','+value'],
			  out => [],
			  bi  => fdp_distinctOffset,
			  module => libfd,
			  native => true},
 
 'disjoint'=> { in  => ['int','+int','int','+int'],
		    out => [],
		    bi  => fdp_disjoint,
		    module => libfd,
		    native => true},
 
 'disjointC'=> { in  => ['int','+int','int','+int','int'],
		     out => [],
		     bi  => fdp_disjointC,
		     module => libfd,
		     native => true},
 
 'distance'	=> { in  => ['int','int','+atom','int'],
		     out => [],
		     bi  => fdp_distance,
		     module => libfd,
		     native => true},
 
 'distinct2'	=> { in  => ['+value','+value','+value','+value'],
			     out => [],
			     bi  => fdp_distinct2,
			     module => libfd,
			     native => true},
 
 'subset'	=> { in  => ['int','int'],
		     out => [],
		     bi  => fdp_subset,
		     module => libfd,
		     native => true},
 
 'dsum'		=> { in  => ['+value','+atom','int'],
			     out => [],
			     bi  => fdp_dsum,
			     module => libfd,
			     native => true},
 
 'dsumC'		=> { in  => ['+value','+value','+atom','int'],
			     out => [],
			     bi  => fdp_dsumC,
			     module => libfd,
			     native => true},
 
 'sumAC'		=> { in  => ['+value','+value','+atom','int'],
			     out => [],
			     bi  => fdp_sumAC,
			     module => libfd,
			     native => true},
 
 );
