### -*-perl-*-

%builtins_all =
    (
     'is'	=> { in  => ['+value'],
		     out => ['+bool'],
		     BI  => weakdict_is },

     'new'	=> { in  => [],
		     out => ['[feature#value]','+value'],
		     BI  => weakdict_new },

     'put'	=> { in  => ['+value','+feature','+value'],
		     out => [],
		     BI  => weakdict_put },

     'get'	=> { in  => ['+value','+feature'],
		     out => ['+value'],
		     BI  => weakdict_get },

     'condGet'	=> { in  => ['+value','+feature','value'],
		     out => ['value'],
		     BI  => weakdict_condGet },

     'close'	=> { in  => ['+value'],
		     out => [],
		     BI  => weakdict_close },

     'keys'	=> { in  => ['+value'],
		     out => ['+[feature]'],
		     BI  => weakdict_keys },

     'entries'	=> { in  => ['+value'],
		     out => ['+[feature#value]'],
		     BI  => weakdict_entries },

     'items'	=> { in  => ['+value'],
		     out => ['+[value]'],
		     BI  => weakdict_items },

     'isEmpty'	=> { in  => ['+value'],
		     out => ['+bool'],
		     BI  => weakdict_isempty },

     'toRecord'	=> { in  => ['+literal','+value'],
		     out => ['+record'],
		     BI  => weakdict_torecord },

     'remove'	=> { in  => ['+value','+feature'],
		     out => [],
		     BI  => weakdict_remove },

     'removeAll'=> { in  => ['+value'],
		     out => [],
		     BI  => weakdict_remove_all },

     'member'	=> { in  => ['+value','+feature'],
		     out => ['+bool'],
		     BI  => weakdict_member },
     );
