###
### Authors:
###   Denys Duchier <duchier@ps.uni-sb.de>
###   Christian Schulte <schulte@ps.uni-sb.de>
###
### Copyright:
###   Denys Duchier, 1998
###   Christian Schulte, 1998
###
### Last change:
###   $Date$ by $Author$
###   $Revision$
###
### This file is part of Mozart, an implementation 
### of Oz 3:
###    http://www.mozart-oz.org
###
### See the file "LICENSE" or
###    http://www.mozart-oz.org/LICENSE.html
### for information on usage and redistribution 
### of this file, and for a DISCLAIMER OF ALL 
### WARRANTIES.
###

$module_init_fun_name = "fdp_init";

$boot_module_name     = "FDP";

%builtins_all = 
(
 
 #
 # Distribution stuff
 #
 'assign'        => { in =>  ['+atom','+value'],
                      out => ['value'],
                      bi =>  fdd_assign },

 'distribute'    => { in =>  ['+int','+int','+value'],
                      out => ['value'],
                      bi =>  fdd_distribute },

 #
 # Propagators
 #

 'sum'		=> { in  => ['+value','+atom','int'],
			     out => [],
			     bi  => fdp_sum},

 'sumC'		=> { in  => ['+value','+value','+atom','int'],
			     out => [],
			     bi  => fdp_sumC},
 
 'sumCN'		=> { in  => ['+value','+value','+atom','int'],
			     out => [],
			     bi  => fdp_sumCN},
 
 'reified.sum'		=> { in  => ['+value','+atom','*int','*int'],
			     out => [],
			     bi  => fdp_sumR},
 
 'reified.sumC'		=> { in  => ['+value','+value','+atom','*int','*int'],
			     out => [],
			     bi  => fdp_sumCR},
 
 'reified.sumCN'	        => { in  => ['+value','+value','+atom','*int','*int'],
			     out => [],
			     bi  => fdp_sumCNR},
 
 'plus'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_plus},
 
 'times'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_times},
 
 'plusD'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_plusD},
 
 'timesD'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_timesD},
 
 'power'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_power},
 
 'divD'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_divD},
 
 'divI'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_divI},
 
 'modD'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_modD},
 
 'modI'		=> { in  => ['int','+int','int'],
			     out => [],
			     bi  => fdp_modI},
 
 'conj'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_conj},
 
 'disj'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_disj},
 
 'exor'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_exor},
 
 'impl'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_impl},
 
 'equi'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_equi},
 
 'nega'		=> { in  => ['int','int'],
			     out => [],
			     bi  => fdp_nega},
 
 'reified.int'		=> { in  => ['int','+value','int'],
			     out => [],
			     bi  => fdp_intR},
 
 'reified.card'		=> { in  => ['+value','int','int','int'],
			     out => [],
			     bi  => fdp_card},
 
 'exactly'	=> { in  => ['int','+value','+int'],
		     out => [],
		     bi  => fdp_exactly},
 
 'atLeast'	=> { in  => ['int','+value','+int'],
		     out => [],
		     bi  => fdp_atLeast},
 
 'atMost'	=> { in  => ['int','+value','+int'],
		     out => [],
		     bi  => fdp_atMost},
 
 'element'	=> { in  => ['int','+value','int'],
		     out => [],
		     bi  => fdp_element},
 
 'lessEqOff'	=> { in  => ['int','int','+int'],
			     out => [],
			     bi  => fdp_lessEqOff},
 
 'min'	=> { in  => ['int','int','int'],
		     out => [],
		     bi  => fdp_minimum},
 
 'max'	=> { in  => ['int','int','int'],
		     out => [],
		     bi  => fdp_maximum},
 
 'inter'	=> { in  => ['int','int','int'],
		     out => [],
		     bi  => fdp_inter},
 
 'union'	=> { in  => ['int','int','int'],
		     out => [],
		     bi  => fdp_union},
 
 'distinct'	=> { in  => ['+value'],
		     out => [],
		     bi  => fdp_distinct},
 
 'distinctB'	=> { in  => ['+value'],
		     out => [],
		     bi  => fdp_distinctB},
 
 'distinctD'	=> { in  => ['+value'],
			     out => [],
			     bi  => fdp_distinctD},
 
 'distinctStream'=> { in  => ['+value','value'],
			  out => [],
			  bi  => fdp_distinctStream},
 
 'distinctOffset'=> { in  => ['+value','+value'],
			  out => [],
			  bi  => fdp_distinctOffset},
 
 'disjoint'=> { in  => ['int','+int','int','+int'],
		    out => [],
		    bi  => fdp_disjoint},
 
 'disjointC'=> { in  => ['int','+int','int','+int','int'],
		     out => [],
		     bi  => fdp_disjointC},
 
 'tasksOverlap'=> { in  => ['int','+int','int','+int','int'],
		     out => [],
		     bi  => fdp_tasksOverlap},
 
 'distance'	=> { in  => ['int','int','+atom','int'],
		     out => [],
		     bi  => fdp_distance},
 
 'distinct2'	=> { in  => ['+value','+value','+value','+value'],
			     out => [],
			     bi  => fdp_distinct2},
 
 'sumD'		=> { in  => ['+value','+atom','int'],
			     out => [],
			     bi  => fdp_dsum},
 
 'sumCD'		=> { in  => ['+value','+value','+atom','int'],
			     out => [],
			     bi  => fdp_dsumC},
 
 'sumAC'		=> { in  => ['+value','+value','+atom','int'],
			     out => [],
			     bi  => fdp_sumAC},
 
 );
