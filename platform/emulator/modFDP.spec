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

%builtins_all = 
(
 
 #
 # Distribution stuff
 #
 'selVarNaive'	=> { in     => ['+tuple'],
			     out    => ['*int'],
			     bi     => BIfdd_selVarNaive},
 
 'selVarSize'	=> { in     => ['+tuple'],
			     out    => ['*int'],
			     bi     => BIfdd_selVarSize},
 
 'selVarMin'	=> { in     => ['+tuple'],
			     out    => ['*int'],
			     bi     => BIfdd_selVarMin},
 
 'selVarMax'	=> { in     => ['+tuple'],
			     out    => ['*int'],
			     bi     => BIfdd_selVarMax},
 
 'selVarNbSusps' => { in     => ['+tuple'],
			  out    => ['*int'],
			  bi     => BIfdd_selVarNbSusps},

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
 
 'sum_CD'		=> { in  => ['+value','+atom','*int','*int'],
			     out => [],
			     ifdef => FDCD,
			     bi  => fdp_sumCD},
 
 'sumC_CD'	=> { in  => ['+value','+value','+atom','*int','*int'],
		     out => [],
		     ifdef => FDCD,
		     bi  => fdp_sumCCD},
 
 'sumCN_CD'	=> { in  => ['+value','+value','+atom','*int','*int'],
		     out => [],
		     ifdef => FDCD,
		     bi  => fdp_sumCNCD},
 
 'plus'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_plus},
 
 'minus'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_minus},
 
 'times'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_times},
 
 'plusD'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_plusD},
 
 'minusD'		=> { in  => ['int','int','int'],
			     out => [],
			     bi  => fdp_minusD},
 
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
 
 'distance'	=> { in  => ['int','int','+atom','int'],
		     out => [],
		     bi  => fdp_distance},
 
 'distinct2'	=> { in  => ['+value','+value','+value','+value'],
			     out => [],
			     bi  => fdp_distinct2},
 
 'subset'	=> { in  => ['int','int'],
		     out => [],
		     bi  => fdp_subset},
 
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
