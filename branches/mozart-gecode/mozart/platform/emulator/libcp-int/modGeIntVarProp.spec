$module_init_fun_name      = "geivp_init";

$boot_module_name      		  = "GFDProp";

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

'rel_BVA_IRT_BVA_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_BVA_IRT_BVA_ICL_PK}
,

'rel_BVA_IRT_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_BVA_IRT_ICL_PK}
,

'rel_BV_BOT_BV_BV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_BV_BOT_BV_BV_ICL_PK}
,

'rel_BV_BOT_BV_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_BV_BOT_BV_In_ICL_PK}
,

'rel_BVA_BOT_BV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_BVA_BOT_BV_ICL_PK}
,

'rel_BVA_BOT_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => rel_BVA_BOT_In_ICL_PK}
,

'element_IA_IV_IV_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => element_IA_IV_IV_In_ICL_PK}
,

'element_IA_IV_BV_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => element_IA_IV_BV_In_ICL_PK}
,

'element_IA_IV_In_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => element_IA_IV_In_In_ICL_PK}
,

'element_IVA_IV_IV_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => element_IVA_IV_IV_In_ICL_PK}
,

'element_IVA_IV_In_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => element_IVA_IV_In_In_ICL_PK}
,

'element_BVA_IV_BV_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => element_BVA_IV_BV_In_ICL_PK}
,

'element_BVA_IV_In_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => element_BVA_IV_In_In_ICL_PK}
,

'distinct_IVA_ICL_PK' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => distinct_IVA_ICL_PK}
,

'distinct_IA_IVA_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => distinct_IA_IVA_ICL_PK}
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

'channel_BVA_IV_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => channel_BVA_IV_In_ICL_PK}
,

'circuit_IVA_ICL_PK' => { in => ['+value', '+value', '+value'],
	out=>[],
	bi => circuit_IVA_ICL_PK}
,

'cumulatives_IVA_IVA_IVA_IVA_IVA_IA_Bl_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => cumulatives_IVA_IVA_IVA_IVA_IVA_IA_Bl_ICL_PK}
,

'cumulatives_IA_IVA_IVA_IVA_IVA_IA_Bl_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => cumulatives_IA_IVA_IVA_IVA_IVA_IA_Bl_ICL_PK}
,

'cumulatives_IVA_IVA_IA_IVA_IVA_IA_Bl_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => cumulatives_IVA_IVA_IA_IVA_IVA_IA_Bl_ICL_PK}
,

'cumulatives_IA_IVA_IA_IVA_IVA_IA_Bl_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => cumulatives_IA_IVA_IA_IVA_IVA_IA_Bl_ICL_PK}
,

'cumulatives_IVA_IVA_IVA_IVA_IA_IA_Bl_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => cumulatives_IVA_IVA_IVA_IVA_IA_IA_Bl_ICL_PK}
,

'cumulatives_IA_IVA_IVA_IVA_IA_IA_Bl_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => cumulatives_IA_IVA_IVA_IVA_IA_IA_Bl_ICL_PK}
,

'cumulatives_IVA_IVA_IA_IVA_IA_IA_Bl_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => cumulatives_IVA_IVA_IA_IVA_IA_IA_Bl_ICL_PK}
,

'cumulatives_IA_IVA_IA_IVA_IA_IA_Bl_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => cumulatives_IA_IVA_IA_IVA_IA_IA_Bl_ICL_PK}
,

'sorted_IVA_IVA_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => sorted_IVA_IVA_ICL_PK}
,

'sorted_IVA_IVA_IVA_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => sorted_IVA_IVA_IVA_ICL_PK}
,

'count_IVA_In_IRT_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_IVA_In_IRT_In_ICL_PK}
,

'count_IVA_IV_IRT_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_IVA_IV_IRT_In_ICL_PK}
,

'count_IVA_IA_IRT_In_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_IVA_IA_IRT_In_ICL_PK}
,

'count_IVA_In_IRT_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_IVA_In_IRT_IV_ICL_PK}
,

'count_IVA_IV_IRT_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_IVA_IV_IRT_IV_ICL_PK}
,

'count_IVA_IA_IRT_IV_ICL_PK' => { in => ['+value', '+value', '+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_IVA_IA_IRT_IV_ICL_PK}
,

'count_IVA_IVA_ICL_PK' => { in => ['+value', '+value', '+value', '+value'],
	out=>[],
	bi => count_IVA_IVA_ICL_PK}

);