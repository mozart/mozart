# -*-perl-*-

%builtins_all =
    (
     'is'	  => { in  => ['+value'],
		       out => ['+bool'],
		       BI  => BIthreadIs},

     'this'	  => { in  => [],
		       out => ['+thread'],
		       BI  => BIthreadThis},

     'suspend'	  => { in  => ['+thread'],
		       out => [],
		       BI  => BIthreadSuspend},

     'resume'	  => { in  => ['+thread'],
		       out => [],
		       BI  => BIthreadResume},

     'injectException' => { in  => ['+thread','+value'],
			    out => [],
			    BI  => BIthreadRaise},

     'preempt'	  => { in  => ['+thread'],
		       out => [],
		       BI  => BIthreadPreempt},

     'setPriority'=> { in  => ['+thread','+atom'],
		       out => [],
		       BI  => BIthreadSetPriority},

     'getPriority'=> { in  => ['+thread'],
		       out => ['+atom'],
		       BI  => BIthreadGetPriority},

     'isSuspended'=> { in  => ['+thread'],
		       out => ['+bool'],
		       BI  => BIthreadIsSuspended},

     'state'	  => { in  => ['+thread'],
		       out => ['+atom'],
		       BI  => BIthreadState},

     'create'     => { in  => ['+procedure/0'],
		       out => [],
		       BI  => BIthreadCreate},
     );
1;;
