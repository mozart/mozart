# -*-perl-*-

%builtins_all =
    (
     'is'	  => { in  => ['+value'],
		       out => ['+bool'],
		       bi  => BIisRecordB},

     'isC'	  => { in  => ['+value'],
		       out => ['+bool'],
		       bi  => BIisRecordCB},

     'adjoin'	  => { in  => ['+record','+record'],
		       out => ['+record'],
		       bi  => BIadjoin},

     'adjoinList' => { in  => ['+record','+[feature#value]'],
		       out => ['+record'],
		       BI  => BIadjoinList},

     'record'	  => { in  => ['+literal','+[feature#value]'],
		       out => ['+record'],
		       BI  => BImakeRecord},

     'arity'	  => { in  => ['+record'],
		       out => ['+[feature]'],
		       bi  => BIarity},

     'adjoinAt'	  => { in  => ['+record','+feature','value'],
		       out => ['+record'],
		       BI  => BIadjoinAt},

     'label'	  => { in  => ['*recordC'],
		       out => ['+literal'],
		       bi  => BIlabel},

     'hasLabel'	  => { in  => ['value'],
		       out => ['+bool'],
		       bi  => BIhasLabel},

     'tellRecord' => { in  => ['+literal','record'],
		       out => [],
		       BI  => BIrecordTell},

     'widthC'	  => { in  => ['*record','int'],
		       out => [],
		       BI  => BIwidthC},

     'monitorArity'=> { in  => ['*recordC','value','[feature]'],
			out => [],
			BI  => BImonitorArity},

     'tellRecordSize'=> { in  => ['+literal','+int','record'],
			  out => [],
			  BI  => BIsystemTellSize},

     '.'	  => { in  => ['*recordCOrChunk','+feature'],
		       out => ['value'],
		       bi  => BIdot},

     '^'	  => { in  => ['*recordCOrChunk','+feature'],
		       out => ['value'],
		       bi  => BIuparrowBlocking},

     'width'	  => { in  => ['+record'],
		       out => ['+int'],
		       bi  => BIwidth},
     );
1;;
