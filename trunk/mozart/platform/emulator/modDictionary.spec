# -*-perl-*-

%builtins_all =
    (
     'is' 	=> { in  => ['+value'],
		     out => ['+bool'],
		     bi  => BIisDictionary},

     'new' 	=> { in  => [],
		     out => ['+dictionary'],
		     BI  => BIdictionaryNew},

     'get'	=> { in  => ['+dictionary','+feature'],
		     out => ['value'],
		     bi  => BIdictionaryGet},

     'condGet'  => { in  => ['+dictionary','+feature','value'],
		     out => ['value'],
		     bi  => BIdictionaryCondGet},

     'put'	=> { in  => ['+dictionary','+feature','value'],
		     out => [],
		     bi  => BIdictionaryPut},

     'remove'	=> { in  => ['+dictionary','+feature'],
		     out => [],
		     bi  => BIdictionaryRemove},

     'removeAll'=> { in  => ['+dictionary'],
		     out => [],
		     BI  => BIdictionaryRemoveAll},

     'member'	=> { in  => ['+dictionary','+feature'],
		     out => ['+bool'],
		     bi  => BIdictionaryMember},

     'keys'     => { in  => ['+dictionary'],
		     out => ['+[feature]'],
		     BI  => BIdictionaryKeys},

     'entries'  => { in  => ['+dictionary'],
		     out => ['+[feature#value]'],
		     BI  => BIdictionaryEntries},

     'items'    => { in  => ['+dictionary'],
		     out => ['+[value]'],
		     BI  => BIdictionaryItems},

     'clone'    => { in  => ['+dictionary'],
		     out => ['+dictionary'],
		     BI  => BIdictionaryClone},

     'markSafe' => { in  => ['+dictionary'],
		     out => [],
		     BI  => BIdictionaryMarkSafe},
     );;
1;;
