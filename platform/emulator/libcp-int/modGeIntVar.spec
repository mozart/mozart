$module_init_fun_name = "module_init_geintvar";

$module_init_fun      = "oz_init_module";

$module_name          = "GFD";

%builtins_all = 
(
 #
 # Built-ins
 #
 
 ## Variable declaration
	'int'		=> { in  => ['+value'],
			     	 out => ['int'],
					 bi  => new_intvar,
					 fcp => ignore},
 ## Variable type testing
    'is'		=> { in  => ['+value'],
			     	 out => ['bool'],
					 bi  => intvar_is,
                     fcp => ignore},
 ## misc. operations
	'inf'       => { in  => [],
	     			 out => ['int'],
					 bi  => int_inf},

    'sup'       => { in  => [],
	               	 out => ['int'],
					 bi  => int_sup},
 
 
 ## Variable reflection
    'reflect.min'		=> { in  => ['*int'],
					 out => ['+int'],
					 bi  => intvar_getMin},
    
    'reflect.max'	    => { in  => ['*int'],
			      	 out => ['+int'],
					 bi  => intvar_getMax},

    'reflect.size'      => { in  => ['*int'],
			  	     out => ['+int'],
					 bi  => intvar_getSize},
	
	'reflect.domList'   => { in  => ['+value'],
	                      out => ['+value'],
						  bi  => int_domList},
	'reflect.dom'       => { in  => ['+value'],
	                      out => ['+value'],
						  bi  => int_dom},
    'reflect.nextLarger'     => { in  => ['+value','int'],
	                      out => ['int'],
			      bi  => int_nextLarger},

    'reflect.nextSmaller'    => { in  => ['+value','int'],
	                      out => ['int'],
			      bi  => int_nextSmaller},

## Still missing:
## -nbSusps -> degree
## -mid
    'reflect.med' 		=> { in  => ['*int'],
	                 out => ['+int'],
					 bi  => intvar_getMed},

    'reflect.width'      => { in  => ['*int'],
	                  out => ['+int'],
					  bi  => intvar_getWidth},

    'reflect.regretMin'  => { in  => ['*int'],
	 		          out => ['+int'],
					  bi  => intvar_getRegretMin},

    'reflect.regretMax'  => { in  => ['*int'],
	                  out => ['+int'],
					  bi  => intvar_getRegretMax},

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
    
    'bool_and'           => { in  => ['+value','+value','+value','+value'],
	                      out => [],
			      bi  => bool_and},

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
    
    'int_disjoint'       => { in  => ['+value','+value','int','+value'],
	                      out => [],
			      bi  => int_disjoint},

    'bool_Gand'          => { in  => ['+value','+value','+value','int'],
	                      out => [],
			      bi  => bool_Gand},

    'bool_Gor'          => { in  => ['+value','+value','+value','int'],
	                     out => [],
			     bi  => bool_Gor},

    'bool_Gxor'         => { in  => ['+value','+value','+value','int'],
	                     out => [],
			     bi  => bool_Gxor},

    'bool_Gnot'          => { in  => ['+value','+value','int'],
	                      out => [],
			      bi  => bool_Gnot},

    'bool_Gimp'          => { in  => ['+value','+value','+value','int'],
	                      out => [],
			      bi  => bool_Gimp},

    'bool_Geqv'          => { in  => ['+value','+value','+value','int'],
	                      out => [],
			      bi  => bool_Geqv},

####
'linear_IVA_IRT_In_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => linear_IVA_IRT_In_ICL}
,

'linear_IVA_IRT_IV_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => linear_IVA_IRT_IV_ICL}
,

'linear_IVA_IRT_In_BV_ICL'=> { in => ['+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => linear_IVA_IRT_In_BV_ICL}
,

'linear_IVA_IRT_IV_BV_ICL'=> { in => ['+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => linear_IVA_IRT_IV_BV_ICL}
,

'linear_IA_IVA_IRT_In_ICL'=> { in => ['+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => linear_IA_IVA_IRT_In_ICL}
,

'linear_IA_IVA_IRT_IV_ICL'=> { in => ['+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => linear_IA_IVA_IRT_IV_ICL}
,

'linear_IA_IVA_IRT_In_BV_ICL'=> { in => ['+value','+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => linear_IA_IVA_IRT_In_BV_ICL}
,

'linear_IA_IVA_IRT_IV_BV_ICL'=> { in => ['+value','+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => linear_IA_IVA_IRT_IV_BV_ICL}
,

'linear_BVA_IRT_In_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => linear_BVA_IRT_In_ICL}
,

'linear_BVA_IRT_IV_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => linear_BVA_IRT_IV_ICL}
,

'eq_IV_IV_ICL'=> { in => ['+value','+value','+value','+value'],
		 out=>[],
		 bi => eq_IV_IV_ICL}
,

'eq_IV_In_ICL'=> { in => ['+value','+value','+value','+value'],
		 out=>[],
		 bi => eq_IV_In_ICL}
,

'eq_IV_IV_BV_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => eq_IV_IV_BV_ICL}
,

'eq_IV_In_BV_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => eq_IV_In_BV_ICL}
,

'eq_IVA_ICL'=> { in => ['+value','+value','+value'],
		 out=>[],
		 bi => eq_IVA_ICL}
,

'count_IVA_In_IRT_In_ICL'=> { in => ['+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => count_IVA_In_IRT_In_ICL}
,

'count_IVA_IV_IRT_In_ICL'=> { in => ['+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => count_IVA_IV_IRT_In_ICL}
,

'count_IVA_In_IRT_IV_ICL'=> { in => ['+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => count_IVA_In_IRT_IV_ICL}
,

'count_IVA_IV_IRT_IV_ICL'=> { in => ['+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => count_IVA_IV_IRT_IV_ICL}
,

'gcc_IVA_IA_InS_InS_InS_InS_InS_ICL'=> { in => ['+value','+value','+value','+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => gcc_IVA_IA_InS_InS_InS_InS_InS_ICL}
,

'gcc_IVA_IA_InS_InS_InS_InS_ICL'=> { in => ['+value','+value','+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => gcc_IVA_IA_InS_InS_InS_InS_ICL}
,

'gcc_IVA_InS_InS_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => gcc_IVA_InS_InS_ICL}
,

'gcc_IVA_InS_ICL'=> { in => ['+value','+value','+value','+value'],
		 out=>[],
		 bi => gcc_IVA_InS_ICL}
,

'gcc_IVA_IVA_InS_InS_ICL'=> { in => ['+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => gcc_IVA_IVA_InS_InS_ICL}
,

'gcc_IVA_IA_IVA_InS_InS_InS_BlS_InS_InS_ICL'=> { in => ['+value','+value','+value','+value','+value','+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => gcc_IVA_IA_IVA_InS_InS_InS_BlS_InS_InS_ICL}
,

'gcc_IVA_IA_IVA_InS_InS_BlS_InS_InS_ICL'=> { in => ['+value','+value','+value','+value','+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => gcc_IVA_IA_IVA_InS_InS_BlS_InS_InS_ICL}
,

'dom_IV_InS_InS_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => dom_IV_InS_InS_ICL}
,

'dom_IVA_InS_InS_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => dom_IVA_InS_InS_ICL}
,

'dom_IV_InS_InS_BV_ICL'=> { in => ['+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => dom_IV_InS_InS_BV_ICL}
,

'bool_not_BV_BV_ICL'=> { in => ['+value','+value','+value','+value'],
		 out=>[],
		 bi => bool_not_BV_BV_ICL}
,

'bool_eq_BV_BV_ICL'=> { in => ['+value','+value','+value','+value'],
		 out=>[],
		 bi => bool_eq_BV_BV_ICL}
,

'bool_and_BV_BV_BV_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => bool_and_BV_BV_BV_ICL}
,

'bool_and_BV_BV_Bl_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => bool_and_BV_BV_Bl_ICL}
,

'bool_or_BV_BV_BV_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => bool_or_BV_BV_BV_ICL}
,

'bool_or_BV_BV_Bl_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => bool_or_BV_BV_Bl_ICL}
,

'bool_imp_BV_BV_BV_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => bool_imp_BV_BV_BV_ICL}
,

'bool_imp_BV_BV_Bl_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => bool_imp_BV_BV_Bl_ICL}
,

'bool_eqv_BV_BV_BV_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => bool_eqv_BV_BV_BV_ICL}
,

'bool_eqv_BV_BV_Bl_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => bool_eqv_BV_BV_Bl_ICL}
,

'bool_xor_BV_BV_BV_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => bool_xor_BV_BV_BV_ICL}
,

'bool_xor_BV_BV_Bl_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => bool_xor_BV_BV_Bl_ICL}
,

'sortedness_IVA_IVA_ICL'=> { in => ['+value','+value','+value','+value'],
		 out=>[],
		 bi => sortedness_IVA_IVA_ICL}
,

'sortedness_IVA_IVA_IVA_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => sortedness_IVA_IVA_IVA_ICL}
,

'distinct_IVA_ICL'=> { in => ['+value','+value','+value'],
		 out=>[],
		 bi => distinct_IVA_ICL}
,

'distinct_IA_IVA_ICL'=> { in => ['+value','+value','+value','+value'],
		 out=>[],
		 bi => distinct_IA_IVA_ICL}
,

'rel_IV_IRT_IV_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => rel_IV_IRT_IV_ICL}
,

'rel_IV_IRT_In_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => rel_IV_IRT_In_ICL}
,

'rel_IV_IRT_IV_BV_ICL'=> { in => ['+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => rel_IV_IRT_IV_BV_ICL}
,

'rel_IV_IRT_In_BV_ICL'=> { in => ['+value','+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => rel_IV_IRT_In_BV_ICL}
,

'rel_IVA_IRT_IVA_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => rel_IVA_IRT_IVA_ICL}
,

'min_IV_IV_IV_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => min_IV_IV_IV_ICL}
,

'min_IVA_IV_ICL'=> { in => ['+value','+value','+value','+value'],
		 out=>[],
		 bi => min_IVA_IV_ICL}
,

'max_IV_IV_IV_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => max_IV_IV_IV_ICL}
,

'max_IVA_IV_ICL'=> { in => ['+value','+value','+value','+value'],
		 out=>[],
		 bi => max_IVA_IV_ICL}
,

'abs_IV_IV_ICL'=> { in => ['+value','+value','+value','+value'],
		 out=>[],
		 bi => abs_IV_IV_ICL}
,

'mult_IV_IV_IV_ICL'=> { in => ['+value','+value','+value','+value','+value'],
		 out=>[],
		 bi => mult_IV_IV_IV_ICL}
		 
 );
