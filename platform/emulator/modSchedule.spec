$cmode='dyn';

%builtins_all =
(
    'sched_disjoint_card'=> { in  => ['int','+int','int','+int'],
			     out => [],
			     bi  => sched_disjoint_card,
			     module => libschedule,
			      native => true},

    'sched_cpIterate'	=> { in  => ['+value','+value','+value'],
			     out => [],
			     bi  => sched_cpIterate,
			     module => libschedule,
			     native => true},

    'sched_cpIterateCap'=> { in  => ['+value','+value','+value',
				     '+value','+value','+int'],
			     out => [],
			     bi  => sched_cpIterateCap,
			     module => libschedule,
			     native => true},

    'sched_cumulativeTI'=> { in  => ['+value','+value','+value',
				     '+value','+value'],
			     out => [],
			     bi  => sched_cumulativeTI,
			     module => libschedule,
			     native => true},

    'sched_cpIterateCapUp'=> { in  => ['+value','+value','+value',
				       '+value','+value'],
			     out => [],
			     bi  => sched_cpIterateCapUp,
			     module => libschedule,
			       native => true},

    'sched_taskIntervals'=> { in  => ['+value','+value','+value'],
			     out => [],
			     bi  => sched_taskIntervals,
			     module => libschedule,
			      native => true},

    'sched_disjunctive'	=> { in  => ['+value','+value','+value'],
			     out => [],
			     bi  => sched_disjunctive,
			     module => libschedule,
			     native => true},

    'sched_disjunctiveStream'=> { in  => ['+value','+value','value'],
			     out => [],
			     bi  => sched_disjunctiveStream,
			     module => libschedule,
				  native => true},

    'sched_taskIntervalsProof'=> { in  => ['value','value','value','value',
					   'value'],
			     out => [],
			     bi  => sched_taskIntervalsProof,
			     module => libschedule,
				   native => true},

    'sched_firstsLasts'	=> { in  => ['value','value','value','value',
				     'value'],
			     out => [],
			     bi  => sched_firstsLasts,
			     module => libschedule,
			     native => true},

 );
