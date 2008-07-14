###$module_init_fun_name = "module_init_gesetvar";

###$module_init_fun      = "oz_init_module";

###$module_name          = "GFS";

%builtins_all = 
(
 #
 # Built-ins
 #
 
 'bounds'=> { 
     in  => ['+value','+value'],
     out => ['set'],
     bi  => new_bounds,
     fcp => ignore
 },

 'isVar'=> { 
     in  => ['+value'],
     out => ['bool'],
     bi  => var_is,
     fcp => ignore
 },

 'comp'=> {
     in  => ['+set'],
     out => ['set'],
     bi  => new_comp,
     fcp => ignore
 },

 'complIn'=> {
     in  => ['+set', '+set'],
     out => ['set'],
     bi  => new_complin,
     fcp => ignore
 },

 'incVal'=> { 
     in  => ['value', '+set'],
     out => [],
     bi  => inc_val,
     fcp => ignore
 },

 'excVal'=> { 
     in  => ['value', '+set'],
     out => [],
     bi  => exc_val,
     fcp => ignore
 },

 'isIn'=> {
     in  => ['+set','int','+bool'],
     out => ['bool'],
     bi  => is_In,
     fcp => ignore
 },


 ## misc. operations
 'inf' => {
     in  => [],
     out => ['int'],
     bi  => set_inf
 },

 'sup'=> {
     in  => [],
     out => ['int'],
     bi  => set_sup
 },


##    'reflect.min'        => { in  => ['*int'],
##      out => ['+int'],
##      bi  => intvar_getMin},
 
##    'reflect.max'        => { in  => ['*int'],
##      out => ['+int'],
##      bi  => intvar_getMax},

##    'reflect.size'       => { in  => ['*int'],
##      out => ['+int'],
##      bi  => intvar_getSize},

##    'reflect.med'        => { in  => ['*int'],
##                      out => ['+int'],
##      bi  => intvar_getMed},

##    'reflect.width'      => { in  => ['*int'],
##                      out => ['+int'],
##      bi  => intvar_getWidth},

##    'reflect.regretMin'  => { in  => ['*int'],
##                      out => ['+int'],
##      bi  => intvar_getRegretMin},

##    'reflect.regretMax'  => { in  => ['*int'],
##                      out => ['+int'],
##      bi  => intvar_getRegretMax},

##Propagators

##    'eq' => { in  => ['+value','+value', 'int'],
##     out => [],
##     bi  => int_eq},

##    'rel' => { in  => ['+value', 'int', '+value', 'int'],
##     out => [],
##     bi  => int_rel},

##    'distinct' => { in  => ['+value', 'int'],
##     out => [],
##     bi  => int_dist},

##    'distinct2' => { in  => ['+value','+value', 'int'],
##     out => [],
##     bi  => int_dist2},
 
##    'mult'               => { in  => ['+value','+value','+value'],
##                      out => [],
##      bi  => int_mult},
 
##    'bool_and'           => { in  => ['+value','+value','+value','+value'],
##                      out => [],
##      bi  => bool_and},

##    'linear' => { in  => ['+value','int', '+value', 'int'],
##     out => [],
##     bi  => int_linear},

##    'linear2' => { in  => ['+value','+value', 'int', '+value', 'int'],
##     out => [],
##     bi  => int_linear2},

##    'linearR' => { in  => ['+value','int','+value','+value','int'],
##     out => [],
##     bi  => int_linearR},

##    'linearCR' => { in  => ['+value','+value', 'int', '+value', '+value','int'],
##     out => [],
##     bi  => int_linearCR},

##    'count'              => { in  => ['+value','int','+value','int','+value'],
##                     out  => [],
##     bi   => int_count},

##    'int_Gabs'           => { in  => ['+value','+value','+value'],
##                      out => [],
##      bi  => int_Gabs},

##    'int_sortedness'     => { in  => ['+value','+value','+int'],
##                      out => [],
##      bi  => int_sortedness},
 
##Mozart propagators
##    'int_inf'            => { in  => [],
##                      out => ['int'],
##      bi  => int_inf},

##    'int_sup'            => { in  => [],
##                      out => ['int'],
##      bi  => int_sup},

##    'int_domList'        => { in  => ['+value'],
##                      out => ['+value'],
##      bi  => int_domList},

##    'int_dom'            => { in  => ['+value'],
##                      out => ['+value'],
##      bi  => int_dom},

##    'int_nextLarger'     => { in  => ['+value','int'],
##                      out => ['int'],
##      bi  => int_sup},

##    'int_nextSmaller'    => { in  => ['+value','int'],
##                      out => ['int'],
##      bi  => int_sup},

 ##int watch propagators

##    'int_watch_min'      => { in  => ['+value','+value','int'],
##                      out => [],
##      bi  => int_watch_min},

##    'int_watch_max'      => { in  => ['+value','+value','int'],
##                      out => [],
##      bi  => int_watch_max},

##    'int_watch_size'     => { in  => ['+value','+value','int'],
##                      out => [],
##      bi  => int_watch_size},

##    'int_sumCN'          => { in  => ['+value','+value','int','+value'],
##                      out => [],
##      bi  => int_sumCN},

 ##reified_int
 
##    'int_disjoint'       => { in  => ['+value','+value','int','+value'],
##                      out => [],
##      bi  => int_disjoint},

##    'bool_Gand'          => { in  => ['+value','+value','+value','int'],
##                      out => [],
##      bi  => bool_Gand},

##    'bool_Gor'          => { in  => ['+value','+value','+value','int'],
##                     out => [],
##     bi  => bool_Gor},

##    'bool_Gxor'         => { in  => ['+value','+value','+value','int'],
##                     out => [],
##     bi  => bool_Gxor},

##    'bool_Gnot'          => { in  => ['+value','+value','int'],
##                      out => [],
##      bi  => bool_Gnot},

##    'bool_Gimp'          => { in  => ['+value','+value','+value','int'],
##                      out => [],
##      bi  => bool_Gimp},

##    'bool_Geqv'          => { in  => ['+value','+value','+value','int'],
##                      out => [],
##      bi  => bool_Geqv},

);
