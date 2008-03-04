$module_init_fun_name      = "geivp_init";

$boot_module_name      		  = "GFD";

%builtins_all = 
(
##Propagators
    'int_sortedness'     => { in  => ['+value','+value','+int'],
	                      out => [],
			      bi  => int_sortedness},
            
## Watching variables
    'watch.min'  => { in  => ['+value','+value','int'],
                  out => [],
            bi  => int_watch_min},

    'watch.max'  => { in  => ['+value','+value','int'],
                  out => [],
            bi  => int_watch_max},

    'watch.size' => { in  => ['+value','+value','int'],
                    out => [],
            bi  => int_watch_size},
    
##Mozart propagators

    'int_sumCN'          => { in  => ['+value','+value','int','+value'],
	                      out => [],
			      bi  => int_sumCN},

    ##reified_int
    'int_reified'       => { in  => ['+value','+value','+value'],
	                      out => [],
			      bi  => int_reified},
    
    'int_disjoint'       => { in  => ['+value','+value','int','+value'],
	                      out => [],
			      bi  => int_disjoint},
###

	'assign'    => { in  => ['+value','int'],
	                      out => [],
			      bi  => int_assign},

	'int_ext'    => { in  => ['+value','+value'],
	                      out => [],
			      bi  => int_ext},
			      
### Finite domain propagators from gecode

'gfd_dom_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_dom_5 }
,

'gfd_dom_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_dom_4 }
,

'gfd_dom_6' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_dom_6 }
,

'gfd_rel_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_rel_5 }
,

'gfd_rel_6' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_rel_6 }
,

'gfd_rel_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_rel_4 }
,


'gfd_element_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_element_5 }
,

'gfd_distinct_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => gfd_distinct_3 }
,

'gfd_distinct_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_distinct_4 }
,

'gfd_channel_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_channel_4 }
,

'gfd_channel_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_channel_5 }
,
'gfd_circuit_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => gfd_circuit_3 }
,


'gfd_cumulatives_9' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_cumulatives_9 }
,

'gfd_sorted_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_sorted_4 }
,

'gfd_sorted_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_sorted_5 }
,

'gfd_count_6' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_count_6 }
,

'gfd_count_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_count_4 }
,

'gfd_count_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_count_5 }
,

'gfd_count_2' => { in => ['+value', '+value'],
	out=>[],
	bi => gfd_count_2 }
,

'gfd_count_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => gfd_count_3 }
,

'gfd_extensional_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_extensional_4 }
,

'gfd_max_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_max_5 }
,

'gfd_max_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_max_4 }
,

'gfd_max_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => gfd_max_3 }
,

'gfd_max_2' => { in => ['+value', '+value'],
	out=>[],
	bi => gfd_max_2 }
,

'gfd_mult_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_mult_5 }
,

'gfd_mult_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => gfd_mult_3 }
,

'gfd_min_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_min_5 }
,

'gfd_min_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_min_4 }
,

'gfd_min_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => gfd_min_3 }
,

'gfd_min_2' => { in => ['+value', '+value'],
	out=>[],
	bi => gfd_min_2 }
,

'gfd_abs_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_abs_4 }
,

'gfd_abs_2' => { in => ['+value', '+value'],
	out=>[],
	bi => gfd_abs_2 }
,

'gfd_linear_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_linear_5 }
,

'gfd_linear_6' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_linear_6 }
,

'gfd_linear_7' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => gfd_linear_7 }

);
