# -*- mode: perl -*-

$module_init_fun_name      = "geivp_init";

$boot_module_name      		  = "GFD";

%builtins_all = 
(
## Distributor
 'distribute'    => { in =>  ['+int', '+int', '+value'],
                      out => ['value'],
                      bi =>  gfd_distribute },

## Assignment
 'assign'        => { in =>  ['+atom','+value'],
                      out => ['value'],
                      bi =>  gfd_assign },

##Propagators
 'int_sortedness' => { 
     in  => ['+value','+value','+int'],
     out => [],
     bi  => int_sortedness
 },
 
## Watching variables
 'watch.min'  => { 
     in  => ['+value','+value','int'],
     out => [],
     bi  => int_watch_min
 },

 'watch.max'  => { 
     in  => ['+value','+value','int'],
     out => [],
     bi  => int_watch_max
 },
 
 'watch.size' => { 
     in  => ['+value','+value','int'],
     out => [],
     bi  => int_watch_size
 },
 
##Mozart propagators

 'int_sumCN' => {
     in  => ['+value','+value','int','+value'],
     out => [],
     bi  => int_sumCN
 },
 
 'reified_sumAC' => {
     in  => ['+value', '+value', '+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => reified_sumAC
 },
 
 'reified_sumCN' => {
     in  => ['+value', '+value', '+value', '+value', '+value', '+value','+value'],
     out => [],
     bi  => reified_sumCN
 },
 
  'reified_sumACN' => {
     in  => ['+value', '+value', '+value', '+value', '+value', '+value','+value'],
     out => [],
     bi  => reified_sumACN
 },
 
 'reified_dom' => {
     in  => ['+value', '+value', '+value'],
     out => [],
     bi  => reified_dom
 },

 ##reified_int
 'int_reified' => { 
     in  => ['+value','+value','+value'],
     out => [],
     bi  => int_reified
 },
 
 'int_disjoint' => {
     in  => ['+value','int', '+value','+int'],
     out => [],
     bi  => int_disjoint
 },

 'int_disjointC' => {
     in  => ['+value','int','+value','int','+value'],
     out => [],
     bi  => int_disjointC
 },

###
## TODO: what is this for? the same as distributor? 
# 'assign' => { 
#     in  => ['+value','int'],
#     out => [],
#     bi  => int_assign
# },
 
 'int_ext' => {
     in  => ['+value','+value'],
     out => [],
     bi  => int_ext
 },
 
### Finite domain propagators from gecode

## domain propagators

 'gfd_dom_5' => { 
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_dom_5 
 },

 'gfd_dom_4' => { 
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_dom_4 
 },

 'gfd_dom_6' => { 
     in  => ['+value', '+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_dom_6 
 },

## relation propagators

 'gfd_rel_5' => { 
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_rel_5 
 },

 'gfd_rel_6' => {
     in  => ['+value', '+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_rel_6 
 },

 'gfd_rel_4' => {
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_rel_4
 },

## element propagators

 'gfd_element_5' => { 
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_element_5 
 },

## distinct propagators

 'gfd_distinct_3' => { 
     in  => ['+value', '+value', '+value'],
     out => [],
     bi  => gfd_distinct_3
 },

 'gfd_distinct_4' => {
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_distinct_4 
 },

## channel propagators

 'gfd_channel_4' => {
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_channel_4 
 },

 'gfd_channel_5' => {
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_channel_5 
 },

## graph propagators

 'gfd_circuit_3' => { 
     in  => ['+value', '+value', '+value'],
     out => [],
     bi  => gfd_circuit_3
 },

## scheduling propagators

 'gfd_cumulatives_9' => { 
     in  => ['+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_cumulatives_9 
 },

## sorted propagators

 'gfd_sorted_4' => {
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_sorted_4 
 },

 'gfd_sorted_5' => {
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_sorted_5
 },

## cardinality propagators

 'gfd_count_6' => {
     in  => ['+value', '+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_count_6 
 },

 'gfd_count_4' => {
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_count_4 
 },

 'gfd_count_5' => { 
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_count_5
 },

# this is done by gfd_count_4 and gfd_count_5
# 'gfd_count_2' => { 
#     in  => ['+value', '+value'],
#     out => [],
#     bi  => gfd_count_2 
# },
#
# 'gfd_count_3' => {
#     in  => ['+value', '+value', '+value'],
#     out => [],
#     bi  => gfd_count_3
# },


##  extencional propagators

 'gfd_extensional_4' => { 
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_extensional_4
 },

## arithmetic propagators

 'gfd_max_5' => { 
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_max_5
 },

 'gfd_max_4' => {
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_max_4 
 },

# this is done by gfd_max_5 and gfd_max_4
# 'gfd_max_3' => {
#     in  => ['+value', '+value', '+value'],
#     out => [],
#     bi  => gfd_max_3
# },
#
# 'gfd_max_2' => { 
#     in  => ['+value', '+value'],
#     out => [],
#     bi  => gfd_max_2 
# },

 'gfd_mult_5' => { 
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_mult_5
 },

 'gfd_div_5' => { 
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_div_5
 },
 
 'gfd_divmod_6' => { 
     in  => ['+value', '+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_divmod_6
 },

 'gfd_sqrt_4' => { 
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_sqrt_4
 },
 
	'gfd_mod_5' => { 
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_mod_5
 },
	
	'gfd_power_5' => { 
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_power_5
 },
 
# this is done by gfd_mult_5
# 'gfd_mult_3' => {
#     in  => ['+value', '+value', '+value'],
#     out => [],
#     bi  => gfd_mult_3
# },

 'gfd_min_5' => {
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_min_5 
 },

 'gfd_min_4' => { 
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_min_4 
 },

# this is done by gfd_min_5
# 'gfd_min_3' => { 
#     in  => ['+value', '+value', '+value'],
#     out => [],
#     bi  => gfd_min_3
# },

# this is done by gfd_min_4
# 'gfd_min_2' => {
#     in  => ['+value', '+value'],
#     out => [],
#     bi  => gfd_min_2 
# },

 'gfd_abs_4' => { 
     in  => ['+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_abs_4
 },

# this is done by gfd_abs_4
# 'gfd_abs_2' => {
#     in  => ['+value', '+value'],
#     out => [],
#     bi  => gfd_abs_2
# },


## linear propagators

 'gfd_linear_5' => { 
     in  => ['+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_linear_5
 },

 'gfd_linear_6' => {
     in  => ['+value', '+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_linear_6
 },

 'gfd_linear_7' => {
     in  => ['+value', '+value', '+value', '+value', '+value', '+value', '+value'],
     out => [],
     bi  => gfd_linear_7
 }
 
);
