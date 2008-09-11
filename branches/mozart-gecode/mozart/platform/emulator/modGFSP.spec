# -*- mode: perl -*-

$module_init_fun_name = "gesvp_init";

$boot_module_name     = "GFS";

%builtins_all = 
(
## Distributor
 'distribute'    => { in =>  ['+int', '+int', '+value'],
                      out => ['value'],
                      bi =>  gfs_distribute },

 'diff' => {
     in  => ['+set', '+set', '+set'],
     out => [],
     bi  => set_diff
 },

 'intersect' => { 
     in  => ['+set', '+set', '+set'],
     out => [],
     bi  => set_inter
 },

 'intersectN' => { 
     in  => ['+set', '+set'],
     out => [],
     bi  => set_interN
 },

 'union' => { in  => ['+set', '+set', '+set'],
	      out => [],
	      bi  => set_union
 },
 
 'unionN' => {
     in  => ['+set', '+set'],
     out => [],
     bi  => set_unionN
 },
 
 'subset' => { 
     in  => ['+set', '+set'],
     out => [],
     bi  => set_subS
 },

 'disjoint' => { 
     in  => ['+set', '+set'],
     out => [],
     bi  => set_disjoint
 },

 'distinct' => { 
     in  => ['+set', '+set'],
     out => [],
     bi  => set_subS
 },

# 'gfs_projector_4' => { in  => ['+value', '+value', '+value', '+value'],
#   out => [],
#   bi  => gfs_projector_4 }
# ,

# 'gfs_projector_3' => { in  => ['+value', '+value', '+value'],
#   out => [],
#   bi  => gfs_projector_3 }
# ,

# 'gfs_projector_5' => { in  => ['+value', '+value', '+value', '+value', '+value'],
#   out => [],
#   bi  => gfs_projector_5 }
# ,

 'gfs_dom_3' => {
     in  => ['+value', '+value', '+value'],
     out => [],
     bi  => gfs_dom_3 
 },

 'gfs_dom_4' => {
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfs_dom_4
 },

 'gfs_dom_5' => { 
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfs_dom_5
 },

 'gfs_cardinality_3' => {
     in  => ['+value', '+value', '+value'],
     out => [],
     bi  => gfs_cardinality_3
 },
 
 'gfs_rel_3' => {
     in  => ['+value', '+value', '+value'],
     out => [],
     bi  => gfs_rel_3
 },

 'gfs_rel_4' => {
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfs_rel_4
 },

 'gfs_rel_5' => {
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfs_rel_5
 },

# 'gfs_convex_1' => { in  => ['+value'],
#   out => [],
#   bi  => gfs_convex_1 }
# ,

# 'gfs_convexHull_2' => { in  => ['+value', '+value'],
#   out => [],
#   bi  => gfs_convexHull_2 }
# ,

 'gfs_sequence_1' => { 
     in  => ['+value'],
     out => [],
     bi  => gfs_sequence_1 
 },
 
 'gfs_sequentialUnion_2' => {
     in  => ['+value', '+value'],
     out => [],
     bi  => gfs_sequentialUnion_2
 },
 
 'atmostOne_2' => { 
     in  => ['+value', '+value'],
     out => [],
     bi  => atmostOne_2 
 },
 
 'gfs_min_2' => {
     in  => ['+value', '+value'],
     out => [],
     bi  => gfs_min_2
 },
 
 'gfs_match_2' => {
     in  => ['+value', '+value'],
     out => [],
     bi  => gfs_match_2 
 },
 
 'gfs_channel_2' => { 
     in  => ['+value', '+value'],
     out => [],
     bi  => gfs_channel_2
 },
 
 'gfs_cardinality_2' => {
     in  => ['+set', '+int'],
     out => [],
     bi  => gfs_cardinality_2
 },
 
 'gfs_max_2' => { 
     in  => ['+value', '+value'],
     out => [],
     bi  => gfs_max_2
 },
 
 'gfs_weights_4' => {
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfs_weights_4 
 },  

 'gfs_reifiedInclude_3' => {
     in  => ['+int', '+set', '+value'],
     out => [],
     bi  => gfs_reifiedInclude_3
 },

);
