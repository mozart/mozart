# -*-perl-*-

%builtins_all =
    (
     'is'	 => { in  => ['+value'],
		      out => ['+bool'],
		      bi  => BIisFloatB},

     'exp'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIexp},

     'log'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIlog},

     'sqrt'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIsqrt},

     'sin'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIsin},

     'asin'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIasin},

     'cos'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIcos},

     'acos'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIacos},

     'tan'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BItan},

     'atan'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIatan},

     'ceil'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIceil},

     'floor'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIfloor},

     'round'	 => { in  => ['+float'],
		      out => ['+float'],
		      bi  => BIround},

     'atan2'	 => { in  => ['+float','+float'],
		      out => ['+float'],
		      bi  => BIatan2},

     'fPow'	 => { in  => ['+float','+float'],
		      out => ['+float'],
		      bi  => BIfPow},

     'toString'  => { in  => ['+float'],
		      out => ['+string'],
		      BI  => BIfloatToString},


     'toInt'	 => { in  => ['+float'],
		      out => ['+int'],
		      bi  => BIfloatToInt},
     );
1;;
