$module_init_fun_name      = "geivp_init";

$boot_module_name      		  = "GFD";

%builtins_all = 
(
 #
 # Built-ins
 #
##Propagators

## Miscellaneous Propagators


    'eq'		 => { in  => ['+value','+value', 'int'],
			     out => [],
			     bi  => int_eq},

    'rel'		 => { in  => ['+value', 'int', '+value', 'int'],
			     out => [],
			     bi  => int_rel},

    'distinct'		 => { in  => ['+value', 'int'],
			     out => [],
			     bi  => int_dist},

    'distinct2'		 => { in  => ['+value','+value', 'int'],
			     out => [],
			     bi  => int_dist2},
    
    'mult'               => { in  => ['+value','+value','+value'],
	                      out => [],
			      bi  => int_mult},
    
    'linear'		 => { in  => ['+value','int', '+value', 'int'],
			     out => [],
			     bi  => int_linear},

    'linear2'		 => { in  => ['+value','+value', 'int', '+value', 'int'],
			     out => [],
			     bi  => int_linear2},

    'linearR'		 => { in  => ['+value','int','+value','+value','int'],
			     out => [],
			     bi  => int_linearR},

    'linearCR'		 => { in  => ['+value','+value', 'int', '+value', '+value','int'],
			     out => [],
			     bi  => int_linearCR},

    'count'              => { in  => ['+value','int','+value','int','+value'],
	                     out  => [],
			     bi   => int_count},

    'int_Gabs'           => { in  => ['+value','+value','+value'],
	                      out => [],
			      bi  => int_Gabs},

    'int_sortedness'     => { in  => ['+value','+value','+int'],
	                      out => [],
			      bi  => int_sortedness},
    
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

'dom_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => dom_5 }
,

'dom_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => dom_4 }
,

'dom_6' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => dom_6 }
,

'rel_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_5 }
,

'rel_6' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_6 }
,

'rel_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_4 }
,


'element_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => element_5 }
,

'distinct_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => distinct_3 }
,

'distinct_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => distinct_4 }
,

'distinct_1' => { in => ['+value'],
	out=>[],
	bi => distinct_1 }
,

'distinct_2' => { in => ['+value', '+value'],
	out=>[],
	bi => distinct_2 }
,

'channel_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => channel_4 }
,

'channel_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => channel_5 }
,

'channel_2' => { in => ['+value', '+value'],
	out=>[],
	bi => channel_2 }
,

'cumulatives_9' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => cumulatives_9 }
,

'cumulatives_7' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => cumulatives_7 }
,

'sorted_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => sorted_4 }
,

'sorted_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => sorted_5 }
,

'sorted_2' => { in => ['+value', '+value'],
	out=>[],
	bi => sorted_2 }
,

'sorted_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => sorted_3 }
,

'count_6' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_6 }
,

'count_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_4 }
,

'count_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_5 }
,

'count_2' => { in => ['+value', '+value'],
	out=>[],
	bi => count_2 }
,

'count_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => count_3 }
,

'extensional_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => extensional_4 }
,

'extensional_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => extensional_5 }
,

'extensional_2' => { in => ['+value', '+value'],
	out=>[],
	bi => extensional_2 }
,

'extensional_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => extensional_3 }
,

'max_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => max_5 }
,

'max_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => max_4 }
,

'max_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => max_3 }
,

'max_2' => { in => ['+value', '+value'],
	out=>[],
	bi => max_2 }
,

'mult_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => mult_5 }
,

'mult_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => mult_3 }
,

'min_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => min_5 }
,

'min_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => min_4 }
,

'min_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => min_3 }
,

'min_2' => { in => ['+value', '+value'],
	out=>[],
	bi => min_2 }
,

'abs_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => abs_4 }
,

'abs_2' => { in => ['+value', '+value'],
	out=>[],
	bi => abs_2 }
,

'linear_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => linear_5 }
,

'linear_6' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => linear_6 }
,

'linear_7' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => linear_7 }
,

'linear_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => linear_3 }
,

'linear_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => linear_4 }
);
