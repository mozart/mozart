$cmode='dyn';

%builtins_all = 
(
 
 #
 # Distribution stuff
 #
 'fdd_selVarNaive'	=> { in     => ['+tuple'],
			     out    => ['*int'],
			     bi     => BIfdd_selVarNaive,
			     module => libfd,
			     native => true},
 
 'fdd_selVarSize'	=> { in     => ['+tuple'],
			     out    => ['*int'],
			     bi     => BIfdd_selVarSize,
			     module => libfd,
			     native => true},
 
 'fdd_selVarMin'	=> { in     => ['+tuple'],
			     out    => ['*int'],
			     bi     => BIfdd_selVarMin,
			     module => libfd,
			     native => true},
 
 'fdd_selVarMax'	=> { in     => ['+tuple'],
			     out    => ['*int'],
			     bi     => BIfdd_selVarMax,
			     module => libfd,
			     native => true},
 
 'fdd_selVarNbSusps' => { in     => ['+tuple'],
			  out    => ['*int'],
			  bi     => BIfdd_selVarNbSusps,
			  module => libfd,
			  native => true},

 #
 # Propagators
 #
 'fdp_sum'		=> { in  => ['+value','+atom','int'],
			     out => [],
			     bi  => fdp_sum,
			     module => libfd,
			     native => true},

 'fdp_sumC'		=> { in  => ['+value','+value','+atom','int'],
			     out => [],
			     bi  => fdp_sumC,
			     module => libfd,
			     native => true},
 
 'fdp_sumCN'		=> { in  => ['+value','+value','+atom','int'],
			     out => [],
			     bi  => fdp_sumCN,
			     module => libfd,
			     native => true},
 
 'fdp_sumR'		=> { in  => ['+value','+atom','*int','*int'],
			     out => [],
			     bi  => fdp_sumR,
			     module => libfd,
			     native => true},
 
 'fdp_sumCR'		=> { in  => ['+value','+value','+atom','*int','*int'],
			     out => [],
			     bi  => fdp_sumCR,
			     module => libfd,
			     native => true},
 
 'fdp_sumCNR'	        => { in  => ['+value','+value','+atom','*int','*int'],
			     out => [],
			     bi  => fdp_sumCNR,
			     module => libfd,
			     native => true},
 
 'fdp_sumCD'		=> { in  => ['+value','+atom','*int','*int'],
			     out => [],
			     bi  => fdp_sumCD,
			     module => libfd,
			     native => true},
 
 'fdp_sumCCD'	=> { in  => ['+value','+value','+atom','*int','*int'],
		     out => [],
		     bi  => fdp_sumCCD,
		     module => libfd,
		     native => true},
 
 'fdp_sumCNCD'	=> { in  => ['+value','+value','+atom','*int','*int'],
		     out => [],
		     bi  => fdp_sumCNCD,
		     module => libfd,
		     native => true},
 
 'fdp_plus'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_plus,
			     module => libfd,
			     native => true},
 
 'fdp_minus'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_minus,
			     module => libfd,
			     native => true},
 
 'fdp_times'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_times,
			     module => libfd,
                             native => true},
 
 'fdp_power'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_power,
			     module => libfd,
			     native => true},
 
 'fdp_divD'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_divD,
			     module => libfd,
			     native => true},
 
 'fdp_divI'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_divI,
			     module => libfd,
			     native => true},
 
 'fdp_modD'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_modD,
			     module => libfd,
			     native => true},
 
 'fdp_modI'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_modI,
			     module => libfd,
			     native => true},
 
 'fdp_conj'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_conj,
			     module => libfd,
			     native => true},
 
 'fdp_disj'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_disj,
			     module => libfd,
			     native => true},
 
 'fdp_exor'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_exor,
			     module => libfd,
			     native => true},
 
 'fdp_impl'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_impl,
			     module => libfd,
			     native => true},
 
 'fdp_equi'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_equi,
			     module => libfd,
			     native => true},
 
 'fdp_nega'		=> { in  => ['int','int'],
			     out => [],
			     bi  => fdp_nega,
			     module => libfd,
			     native => true},
 
 'fdp_intR'		=> { in  => ['int','+value','int'],
			     out => [],
			     bi  => fdp_intR,
			     module => libfd,
			     native => true},
 
 'fdp_card'		=> { in  => ['+value','int','int','int'],
			     out => [],
			     bi  => fdp_card,
			     module => libfd,
			     native => true},
 
 'fdp_exactly'	=> { in  => ['int','+value','+int'],
		     out => [],
		     bi  => fdp_exactly,
		     module => libfd,
		     native => true},
 
 'fdp_atLeast'	=> { in  => ['int','+value','+int'],
		     out => [],
		     bi  => fdp_atLeast,
		     module => libfd,
		     native => true},
 
 'fdp_atMost'	=> { in  => ['int','+value','+int'],
		     out => [],
		     bi  => fdp_atMost,
		     module => libfd,
		     native => true},
 
 'fdp_element'	=> { in  => ['int','+value','int'],
		     out => [],
		     bi  => fdp_element,
		     module => libfd,
		     native => true},
 
 'fdp_lessEqOff'	=> { in  => ['int','int','+int'],
			     out => [],
			     bi  => fdp_lessEqOff,
			     module => libfd,
			     native => true},
 
 'fdp_minimum'	=> { in  => ['int','int','int'],
		     out => [],
		     bi  => fdp_minimum,
		     module => libfd,
		     native => true},
 
 'fdp_maximum'	=> { in  => ['int','int','int'],
		     out => [],
		     bi  => fdp_maximum,
		     module => libfd,
		     native => true},
 
 'fdp_inter'	=> { in  => ['int','int','int'],
		     out => [],
		     bi  => fdp_inter,
		     module => libfd,
		     native => true},
 
 'fdp_union'	=> { in  => ['int','int','int'],
		     out => [],
		     bi  => fdp_union,
		     module => libfd,
		     native => true},
 
 'fdp_distinct'	=> { in  => ['+value'],
		     out => [],
		     bi  => fdp_distinct,
		     module => libfd,
		     native => true},
 
 'fdp_distinctD'	=> { in  => ['+value'],
			     out => [],
			     bi  => fdp_distinctD,
			     module => libfd,
			     native => true},
 
 'fdp_distinctStream'=> { in  => ['+value','value'],
			  out => [],
			  bi  => fdp_distinctStream,
			  module => libfd,
			  native => true},
 
 'fdp_distinctOffset'=> { in  => ['+value','+value'],
			  out => [],
			  bi  => fdp_distinctOffset,
			  module => libfd,
			  native => true},
 
 'fdp_disjoint'=> { in  => ['int','+int','int','+int'],
		    out => [],
		    bi  => fdp_disjoint,
		    module => libfd,
		    native => true},
 
 'fdp_disjointC'=> { in  => ['int','+int','int','+int','int'],
		     out => [],
		     bi  => fdp_disjointC,
		     module => libfd,
		     native => true},
 
 'fdp_distance'	=> { in  => ['int','int','+atom','int'],
		     out => [],
		     bi  => fdp_distance,
		     module => libfd,
		     native => true},
 
 'fdp_distinct2'	=> { in  => ['+value','+value','+value','+value'],
			     out => [],
			     bi  => fdp_distinct2,
			     module => libfd,
			     native => true},
 
 'fdp_subset'	=> { in  => ['int','int'],
		     out => [],
		     bi  => fdp_subset,
		     module => libfd,
		     native => true},
 
 'fdp_dsum'		=> { in  => ['+value','+atom','int'],
			     out => [],
			     bi  => fdp_dsum,
			     module => libfd,
			     native => true},
 
 'fdp_dsumC'		=> { in  => ['+value','+value','+atom','int'],
			     out => [],
			     bi  => fdp_dsumC,
			     module => libfd,
			     native => true},
 
 'fdp_sumAC'		=> { in  => ['+value','+value','+atom','int'],
			     out => [],
			     bi  => fdp_sumAC,
			     module => libfd,
			     native => true},
 
 );
