$module_init_fun_name      = "GFDProp";

$module_name      		  = "GFDProp";

%builtins_all = 
(

'dom_IV_In_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => dom_IV_In_In_ICL_PK}
,

'dom_IVA_In_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => dom_IVA_In_In_ICL_PK}
,

'dom_IV_IS_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => dom_IV_IS_ICL_PK}
,

'dom_IVA_IS_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => dom_IVA_IS_ICL_PK}
,

'dom_IV_In_In_BV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => dom_IV_In_In_BV_ICL_PK}
,

'dom_IV_IS_BV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => dom_IV_IS_BV_ICL_PK}
,

'rel_IV_IRT_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_IV_IRT_IV_ICL_PK}
,

'rel_IV_IRT_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_IV_IRT_In_ICL_PK}
,

'rel_IV_IRT_IV_BV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_IV_IRT_IV_BV_ICL_PK}
,

'rel_IV_IRT_In_BV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_IV_IRT_In_BV_ICL_PK}
,

'rel_IVA_IRT_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_IVA_IRT_ICL_PK}
,

'rel_IVA_IRT_IVA_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_IVA_IRT_IVA_ICL_PK}
,

'rel_BV_IRT_BV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_BV_IRT_BV_ICL_PK}
,

'rel_BV_IRT_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_BV_IRT_In_ICL_PK}
,

'rel_BV_BOT_BV_BV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_BV_BOT_BV_BV_ICL_PK}
,

'rel_BV_BOT_BV_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_BV_BOT_BV_In_ICL_PK}
,

'distinct_IVA_ICL_PK' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => distinct_IVA_ICL_PK}
,

'channel_IVA_IVA_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => channel_IVA_IVA_ICL_PK}
,

'channel_BV_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => channel_BV_IV_ICL_PK}
,

'channel_IV_BV_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => channel_IV_BV_ICL_PK}
,

'sorted_IVA_IVA_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => sorted_IVA_IVA_ICL_PK}
,

'count_IVA_In_IRT_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_IVA_In_IRT_In_ICL_PK}
,

'count_IVA_IV_IRT_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_IVA_IV_IRT_In_ICL_PK}
,

'count_IVA_In_IRT_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_IVA_In_IRT_IV_ICL_PK}
,

'count_IVA_IV_IRT_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_IVA_IV_IRT_IV_ICL_PK}
,

'count_IVA_IVA_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_IVA_IVA_ICL_PK}
,

'min_IV_IV_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => min_IV_IV_IV_ICL_PK}
,

'min_IVA_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => min_IVA_IV_ICL_PK}
,

'max_IV_IV_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => max_IV_IV_IV_ICL_PK}
,

'max_IVA_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => max_IVA_IV_ICL_PK}
,

'abs_IV_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => abs_IV_IV_ICL_PK}
,

'mult_IV_IV_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => mult_IV_IV_IV_ICL_PK}
,

'linear_IVA_IRT_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => linear_IVA_IRT_In_ICL_PK}
,

'linear_IVA_IRT_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => linear_IVA_IRT_IV_ICL_PK}
,

'linear_IVA_IRT_In_BV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => linear_IVA_IRT_In_BV_ICL_PK}
,

'linear_IVA_IRT_IV_BV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => linear_IVA_IRT_IV_BV_ICL_PK}

);