%builtins_all =
(
    'disjoint_card'=> { in  => ['int','+int','int','+int'],
			     out => [],
			     bi  => sched_disjoint_card},

    'cpIterate'	=> { in  => ['+value','+value','+value'],
			     out => [],
			     bi  => sched_cpIterate},

    'cpIterateCap'=> { in  => ['+value','+value','+value',
				     '+value','+value','+int'],
			     out => [],
			     bi  => sched_cpIterateCap},

    'cumulativeTI'=> { in  => ['+value','+value','+value',
				     '+value','+value'],
			     out => [],
			     bi  => sched_cumulativeTI},

    'cpIterateCapUp'=> { in  => ['+value','+value','+value',
				       '+value','+value'],
			     out => [],
			     bi  => sched_cpIterateCapUp},

    'taskIntervals'=> { in  => ['+value','+value','+value'],
			     out => [],
			     bi  => sched_taskIntervals},

    'disjunctive'	=> { in  => ['+value','+value','+value'],
			     out => [],
			     bi  => sched_disjunctive},

    'disjunctiveStream'=> { in  => ['+value','+value','value'],
			     out => [],
			     bi  => sched_disjunctiveStream},

    'taskIntervalsProof'=> { in  => ['value','value','value','value',
					   'value'],
			     out => [],
			     bi  => sched_taskIntervalsProof},

    'firstsLasts'	=> { in  => ['value','value','value','value',
				     'value'],
			     out => [],
			     bi  => sched_firstsLasts},

 );
