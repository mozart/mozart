$cmode='dyn';

%builtins_all =
(
    'disjoint_card'=> { in  => ['int','+int','int','+int'],
			     out => [],
			     bi  => sched_disjoint_card,
			     module => libschedule,
			      native => true},

    'cpIterate'	=> { in  => ['+value','+value','+value'],
			     out => [],
			     bi  => sched_cpIterate,
			     module => libschedule,
			     native => true},

    'cpIterateCap'=> { in  => ['+value','+value','+value',
				     '+value','+value','+int'],
			     out => [],
			     bi  => sched_cpIterateCap,
			     module => libschedule,
			     native => true},

    'cumulativeTI'=> { in  => ['+value','+value','+value',
				     '+value','+value'],
			     out => [],
			     bi  => sched_cumulativeTI,
			     module => libschedule,
			     native => true},

    'cpIterateCapUp'=> { in  => ['+value','+value','+value',
				       '+value','+value'],
			     out => [],
			     bi  => sched_cpIterateCapUp,
			     module => libschedule,
			       native => true},

    'taskIntervals'=> { in  => ['+value','+value','+value'],
			     out => [],
			     bi  => sched_taskIntervals,
			     module => libschedule,
			      native => true},

    'disjunctive'	=> { in  => ['+value','+value','+value'],
			     out => [],
			     bi  => sched_disjunctive,
			     module => libschedule,
			     native => true},

    'disjunctiveStream'=> { in  => ['+value','+value','value'],
			     out => [],
			     bi  => sched_disjunctiveStream,
			     module => libschedule,
				  native => true},

    'taskIntervalsProof'=> { in  => ['value','value','value','value',
					   'value'],
			     out => [],
			     bi  => sched_taskIntervalsProof,
			     module => libschedule,
				   native => true},

    'firstsLasts'	=> { in  => ['value','value','value','value',
				     'value'],
			     out => [],
			     bi  => sched_firstsLasts,
			     module => libschedule,
			     native => true},

 );
