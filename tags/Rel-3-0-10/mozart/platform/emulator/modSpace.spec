# -*-perl-*-

%builtins_all =
    (
     'new'		=> { in  => ['+procedure/1'],
			     out => ['+space'],
			     BI  => BInewSpace},

     'is'		=> { in  => ['+value'],
			     out => ['+bool'],
			     BI  => BIisSpace},

     'ask'		=> { in  => ['+space'],
			     out => ['+tuple'],
			     BI  => BIaskSpace},

     'askVerbose'	=> { in  => ['+space','!value'],
			     out => [],
			     BI  => BIaskVerboseSpace},

     'merge'	        => { in  => ['+space'],
			     out => ['+value'],
			     BI  => BImergeSpace},

     'clone'	        => { in  => ['+space'],
			     out => ['+space'],
			     BI  => BIcloneSpace},

     'commit'	        => { in  => ['+space','+value'],
			     out => [],
			     BI  => BIcommitSpace},

     'inject'	        => { in  => ['+space','+procedure/1'],
			     out => [],
			     BI  => BIinjectSpace},
     );
1;;
