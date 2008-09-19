# -*- mode: perl -*-
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

 'gfs_unknownSize'=> {
     in  => ['set'],
     out => ['+int'],
     bi  => gfs_unknownSize,
 },

 'gfs_unknown'=> {
     in  => ['set'],
     out => ['value'],
     bi  => gfs_unknown,
 },

'gfs_unknownList'=> {
     in  => ['set'],
     out => ['value'],
     bi  => gfs_unknownList,
 },

'gfs_getLub'=> {
     in  => ['set'],
     out => ['value'],
     bi  => gfs_getLub,
 },

'gfs_getGlb'=> {
     in  => ['set'],
     out => ['value'],
     bi  => gfs_getGlb,
 },

'gfs_getLubList'=> {
     in  => ['set'],
     out => ['value'],
     bi  => gfs_getLubList,
 },

'gfs_getGlbList'=> {
     in  => ['set'],
     out => ['value'],
     bi  => gfs_getGlbList,
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
 
 'value.toString' => {
     in => ['+fset'],
     out => ['+string'],
     bi => gfs_ValueToString
 }
	 
);
