$module_init_fun_name      = "gesvp_init";

$boot_module_name      	   = "GFS";

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
    bi => set_subS}

);