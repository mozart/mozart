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

$module_init_fun_name = "sched_init";

$boot_module_name     = "Schedule";

%builtins_all =
(
    'disjoint'         => { in  => ['int','+int','int','+int'],
			     out => [],
			     bi  => sched_disjoint_card},

    'serialized'	=> { in  => ['+value','+value','+value'],
			     out => [],
			     bi  => sched_cpIterate},

    'cummulative{,EF}'=> { in  => ['+value','+value','+value',
				     '+value','+value','+int'],
			     out => [],
			     bi  => sched_cpIterateCap},

    'cumulativeTI'=> { in  => ['+value','+value','+value',
				     '+value','+value'],
			     out => [],
			     bi  => sched_cumulativeTI},

    'cummulativeUp'=> { in  => ['+value','+value','+value',
				       '+value','+value'],
			     out => [],
			     bi  => sched_cpIterateCapUp},

    'taskIntervals'=> { in  => ['+value','+value','+value'],
			     out => [],
			     bi  => sched_taskIntervals},

    'serializedDisj'	=> { in  => ['+value','+value','+value'],
			     out => [],
			     bi  => sched_disjunctive},

    'disjunctiveStream'=> { in  => ['+value','+value','value'],
			     out => [],
			     bi  => sched_disjunctiveStream},

    'TaskIntervalsDist{P,O}'=> { in  => ['value','value','value','value',
					   'value'],
			     out => [],
			     bi  => sched_taskIntervalsProof},

    'firstsLastsDist'	=> { in  => ['value','value','value','value',
				     'value'],
			     out => [],
			     bi  => sched_firstsLasts},

 ); 
