$module_init_fun_name      = "gesvp_init";

$boot_module_name      		  = "GFS";

%builtins_all = 
(
'diff' => { in => ['+set', '+set', '+set'],
    out=>[],
    bi => set_diff}
,

'intersect' => { in => ['+set', '+set', '+set'],
    out=>[],
    bi => set_inter}
,

'intersectN' => { in => ['+set', '+set'],
    out=>[],
    bi => set_interN}
,

'union' => { in => ['+set', '+set', '+set'],
    out=>[],
    bi => set_union}
,

'unionN' => { in => ['+set', '+set'],
    out=>[],
    bi => set_unionN}
,

'subset' => { in => ['+set', '+set'],
    out=>[],
    bi => set_subS}
,

'disjoint' => { in => ['+set', '+set'],
    out=>[],
    bi => set_disjoint}
,

'distinct' => { in => ['+set', '+set'],
    out=>[],
    bi => set_subS},

'dom_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => dom_3 }
,

'dom_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => dom_4 }
,

'dom_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => dom_5 }
,

'rel_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => rel_3 }
,

'rel_4' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_4 }
,

'rel_5' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_5 }
,

'sequence_1' => { in => ['+value'],
	out=>[],
	bi => sequence_1 }
,

'min_2' => { in => ['+value', '+value'],
	out=>[],
	bi => min_2 }
,

'match_2' => { in => ['+value', '+value'],
	out=>[],
	bi => match_2 }
,

'channel_2' => { in => ['+value', '+value'],
	out=>[],
	bi => channel_2 }
,

'selectUnion_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => selectUnion_3 }
,

'selectDisjoint_2' => { in => ['+value', '+value'],
	out=>[],
	bi => selectDisjoint_2 }
,

'selectSet_3' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => selectSet_3 }
);
