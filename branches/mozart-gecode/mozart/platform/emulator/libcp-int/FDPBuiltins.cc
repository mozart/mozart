OZ_BI_define(linear_IVA_IRT_In_ICL,4,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARGS(0, __x, sp);
	RelType(1, __r);
	DeclareGeIntVar(2, __c, sp);
	ConLevel(3, __ICL_DEF);
	try{
		linear(sp,__x,__r,__c,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IVA_IRT_IV_ICL,4,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARGS(0, __x, sp);
	RelType(1, __r);
	DeclareGeIntVar(2, __y, sp);
	ConLevel(3, __ICL_DEF);
	try{
		linear(sp,__x,__r,__y,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IVA_IRT_In_BV_ICL,5,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARGS(0, __x, sp);
	RelType(1, __r);
	DeclareGeIntVar(2, __c, sp);
	DeclareBoolVar(3, __b, sp);
	ConLevel(4, __ICL_DEF);
	try{
		linear(sp,__x,__r,__c,__b,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IVA_IRT_IV_BV_ICL,5,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARGS(0, __x, sp);
	RelType(1, __r);
	DeclareGeIntVar(2, __y, sp);
	DeclareBoolVar(3, __b, sp);
	ConLevel(4, __ICL_DEF);
	try{
		linear(sp,__x,__r,__y,__b,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IA_IVA_IRT_In_ICL,5,0) {
	DeclareGSpace(sp);
	DECLARE_INTARGS(0, __a);
	DECLARE_INTVARARGS(1, __x, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __c, sp);
	ConLevel(4, __ICL_DEF);
	try{
		linear(sp,__a,__x,__r,__c,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IA_IVA_IRT_IV_ICL,5,0) {
	DeclareGSpace(sp);
	DECLARE_INTARGS(0, __a);
	DECLARE_INTVARARGS(1, __x, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __y, sp);
	ConLevel(4, __ICL_DEF);
	try{
		linear(sp,__a,__x,__r,__y,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IA_IVA_IRT_In_BV_ICL,6,0) {
	DeclareGSpace(sp);
	DECLARE_INTARGS(0, __a);
	DECLARE_INTVARARGS(1, __x, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __c, sp);
	DeclareBoolVar(4, __b, sp);
	ConLevel(5, __ICL_DEF);
	try{
		linear(sp,__a,__x,__r,__c,__b,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IA_IVA_IRT_IV_BV_ICL,6,0) {
	DeclareGSpace(sp);
	DECLARE_INTARGS(0, __a);
	DECLARE_INTVARARGS(1, __x, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __y, sp);
	DeclareBoolVar(4, __b, sp);
	ConLevel(5, __ICL_DEF);
	try{
		linear(sp,__a,__x,__r,__y,__b,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_BVA_IRT_In_ICL,4,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARGS(0, __x, sp);
	RelType(1, __r);
	DeclareGeIntVar(2, __c, sp);
	ConLevel(3, __ICL_DEF);
	try{
		linear(sp,__x,__r,__c,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_BVA_IRT_IV_ICL,4,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARGS(0, __x, sp);
	RelType(1, __r);
	DeclareGeIntVar(2, __y, sp);
	ConLevel(3, __ICL_DEF);
	try{
		linear(sp,__x,__r,__y,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(cumulatives_IVA_IVA_IVA_IVA_IVA_IA_BlS_ICL,8,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARGS(0, __machine, sp);
	DECLARE_INTVARARGS(1, __start, sp);
	DECLARE_INTVARARGS(2, __duration, sp);
	DECLARE_INTVARARGS(3, __end, sp);
	DECLARE_INTVARARGS(4, __height, sp);
	DECLARE_INTARGS(5, __limit);
	DeclareBool(6, __at_most);
	ConLevel(7, __icl);
	try{
		cumulatives(sp,__machine,__start,__duration,__end,__height,__limit,__at_most,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(cumulatives_IA_IVA_IVA_IVA_IVA_IA_BlS_ICL,8,0) {
	DeclareGSpace(sp);
	DECLARE_INTARGS(0, __machine);
	DECLARE_INTVARARGS(1, __start, sp);
	DECLARE_INTVARARGS(2, __duration, sp);
	DECLARE_INTVARARGS(3, __end, sp);
	DECLARE_INTVARARGS(4, __height, sp);
	DECLARE_INTARGS(5, __limit);
	DeclareBool(6, __at_most);
	ConLevel(7, __icl);
	try{
		cumulatives(sp,__machine,__start,__duration,__end,__height,__limit,__at_most,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(cumulatives_IVA_IVA_IA_IVA_IVA_IA_BlS_ICL,8,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARGS(0, __machine, sp);
	DECLARE_INTVARARGS(1, __start, sp);
	DECLARE_INTARGS(2, __duration);
	DECLARE_INTVARARGS(3, __end, sp);
	DECLARE_INTVARARGS(4, __height, sp);
	DECLARE_INTARGS(5, __limit);
	DeclareBool(6, __at_most);
	ConLevel(7, __icl);
	try{
		cumulatives(sp,__machine,__start,__duration,__end,__height,__limit,__at_most,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(cumulatives_IA_IVA_IA_IVA_IVA_IA_BlS_ICL,8,0) {
	DeclareGSpace(sp);
	DECLARE_INTARGS(0, __machine);
	DECLARE_INTVARARGS(1, __start, sp);
	DECLARE_INTARGS(2, __duration);
	DECLARE_INTVARARGS(3, __end, sp);
	DECLARE_INTVARARGS(4, __height, sp);
	DECLARE_INTARGS(5, __limit);
	DeclareBool(6, __at_most);
	ConLevel(7, __icl);
	try{
		cumulatives(sp,__machine,__start,__duration,__end,__height,__limit,__at_most,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(cumulatives_IVA_IVA_IVA_IVA_IA_IA_BlS_ICL,8,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARGS(0, __machine, sp);
	DECLARE_INTVARARGS(1, __start, sp);
	DECLARE_INTVARARGS(2, __duration, sp);
	DECLARE_INTVARARGS(3, __end, sp);
	DECLARE_INTARGS(4, __height);
	DECLARE_INTARGS(5, __limit);
	DeclareBool(6, __at_most);
	ConLevel(7, __icl);
	try{
		cumulatives(sp,__machine,__start,__duration,__end,__height,__limit,__at_most,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(cumulatives_IA_IVA_IVA_IVA_IA_IA_BlS_ICL,8,0) {
	DeclareGSpace(sp);
	DECLARE_INTARGS(0, __machine);
	DECLARE_INTVARARGS(1, __start, sp);
	DECLARE_INTVARARGS(2, __duration, sp);
	DECLARE_INTVARARGS(3, __end, sp);
	DECLARE_INTARGS(4, __height);
	DECLARE_INTARGS(5, __limit);
	DeclareBool(6, __at_most);
	ConLevel(7, __icl);
	try{
		cumulatives(sp,__machine,__start,__duration,__end,__height,__limit,__at_most,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(cumulatives_IVA_IVA_IA_IVA_IA_IA_BlS_ICL,8,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARGS(0, __machine, sp);
	DECLARE_INTVARARGS(1, __start, sp);
	DECLARE_INTARGS(2, __duration);
	DECLARE_INTVARARGS(3, __end, sp);
	DECLARE_INTARGS(4, __height);
	DECLARE_INTARGS(5, __limit);
	DeclareBool(6, __at_most);
	ConLevel(7, __icl);
	try{
		cumulatives(sp,__machine,__start,__duration,__end,__height,__limit,__at_most,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(cumulatives_IA_IVA_IA_IVA_IA_IA_BlS_ICL,8,0) {
	DeclareGSpace(sp);
	DECLARE_INTARGS(0, __machine);
	DECLARE_INTVARARGS(1, __start, sp);
	DECLARE_INTARGS(2, __duration);
	DECLARE_INTVARARGS(3, __end, sp);
	DECLARE_INTARGS(4, __height);
	DECLARE_INTARGS(5, __limit);
	DeclareBool(6, __at_most);
	ConLevel(7, __icl);
	try{
		cumulatives(sp,__machine,__start,__duration,__end,__height,__limit,__at_most,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(eq_IV_IV_ICL,3,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	DeclareGeIntVar(1, __x1, home);
	ConLevel(2, __icl);
	try{
		eq(home,__x0,__x1,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(eq_IV_In_ICL,3,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	DeclareGeIntVar(1, __n, home);
	ConLevel(2, __ICL_DEF);
	try{
		eq(home,__x,__n,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(eq_IV_IV_BV_ICL,4,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	DeclareGeIntVar(1, __x1, home);
	DeclareBoolVar(2, __b, home);
	ConLevel(3, __icl);
	try{
		eq(home,__x0,__x1,__b,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(eq_IV_In_BV_ICL,4,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	DeclareGeIntVar(1, __n, home);
	DeclareBoolVar(2, __b, home);
	ConLevel(3, __icl);
	try{
		eq(home,__x,__n,__b,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(eq_IVA_ICL,2,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	ConLevel(1, __icl);
	try{
		eq(home,__x,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(count_IVA_In_IRT_In_ICL,5,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareGeIntVar(1, __n, home);
	RelType(2, __r);
	DeclareGeIntVar(3, __m, home);
	ConLevel(4, __icl);
	try{
		count(home,__x,__n,__r,__m,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(count_IVA_IV_IRT_In_ICL,5,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y, home);
	RelType(2, __r);
	DeclareGeIntVar(3, __m, home);
	ConLevel(4, __icl);
	try{
		count(home,__x,__y,__r,__m,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(count_IVA_In_IRT_IV_ICL,5,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareGeIntVar(1, __n, home);
	RelType(2, __r);
	DeclareGeIntVar(3, __z, home);
	ConLevel(4, __icl);
	try{
		count(home,__x,__n,__r,__z,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(count_IVA_IV_IRT_IV_ICL,5,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y, home);
	RelType(2, __r);
	DeclareGeIntVar(3, __z, home);
	ConLevel(4, __icl);
	try{
		count(home,__x,__y,__r,__z,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(gcc_IVA_IA_InS_InS_InS_InS_InS_ICL,8,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INTARGS(1, __c);
	OZ_declareInt(2, __m);
	OZ_declareInt(3, __unspec_low);
	OZ_declareInt(4, __unspec_up);
	OZ_declareInt(5, __min);
	OZ_declareInt(6, __max);
	ConLevel(7, __icl);
	try{
		gcc(home,__x,__c,__m,__unspec_low,__unspec_up,__min,__max,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(gcc_IVA_IA_InS_InS_InS_InS_ICL,7,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INTARGS(1, __c);
	OZ_declareInt(2, __m);
	OZ_declareInt(3, __unspec);
	OZ_declareInt(4, __min);
	OZ_declareInt(5, __max);
	ConLevel(6, __icl);
	try{
		gcc(home,__x,__c,__m,__unspec,__min,__max,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(gcc_IVA_InS_InS_ICL,4,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	OZ_declareInt(1, __lb);
	OZ_declareInt(2, __ub);
	ConLevel(3, __icl);
	try{
		gcc(home,__x,__lb,__ub,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(gcc_IVA_InS_ICL,3,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	OZ_declareInt(1, __ub);
	ConLevel(2, __icl);
	try{
		gcc(home,__x,__ub,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(gcc_IVA_IVA_InS_InS_ICL,5,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INTVARARGS(1, __c, home);
	OZ_declareInt(2, __min);
	OZ_declareInt(3, __max);
	ConLevel(4, __icl);
	try{
		gcc(home,__x,__c,__min,__max,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(gcc_IVA_IA_IVA_InS_InS_InS_BlS_InS_InS_ICL,10,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INTARGS(1, __v);
	DECLARE_INTVARARGS(2, __c, home);
	OZ_declareInt(3, __m);
	OZ_declareInt(4, __unspec_low);
	OZ_declareInt(5, __unspec_up);
	DeclareBool(6, __all);
	OZ_declareInt(7, __min);
	OZ_declareInt(8, __max);
	ConLevel(9, __icl);
	try{
		gcc(home,__x,__v,__c,__m,__unspec_low,__unspec_up,__all,__min,__max,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(gcc_IVA_IA_IVA_InS_InS_BlS_InS_InS_ICL,9,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INTARGS(1, __v);
	DECLARE_INTVARARGS(2, __c, home);
	OZ_declareInt(3, __m);
	OZ_declareInt(4, __unspec);
	DeclareBool(5, __all);
	OZ_declareInt(6, __min);
	OZ_declareInt(7, __max);
	ConLevel(8, __icl);
	try{
		gcc(home,__x,__v,__c,__m,__unspec,__all,__min,__max,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(channel_IVA_IVA_ICL,3,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INTVARARGS(1, __y, home);
	ConLevel(2, __icl);
	try{
		channel(home,__x,__y,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(dom_IV_InS_InS_ICL,4,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	OZ_declareInt(1, __l);
	OZ_declareInt(2, __m);
	ConLevel(3, __ICL_DEF);
	try{
		dom(home,__x,__l,__m,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(dom_IVA_InS_InS_ICL,4,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	OZ_declareInt(1, __l);
	OZ_declareInt(2, __m);
	ConLevel(3, __ICL_DEF);
	try{
		dom(home,__x,__l,__m,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(dom_IV_IV_ICL,3,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	DECLARE_INT_SET(1, __s);
	ConLevel(2, __ICL_DEF);
	try{
		dom(home,__x,__s,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(dom_IVA_IV_ICL,3,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INT_SET(1, __s);
	ConLevel(2, __ICL_DEF);
	try{
		dom(home,__x,__s,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(dom_IV_InS_InS_BV_ICL,5,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	OZ_declareInt(1, __l);
	OZ_declareInt(2, __m);
	DeclareBoolVar(3, __b, home);
	ConLevel(4, __ICL_DEF);
	try{
		dom(home,__x,__l,__m,__b,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(dom_IV_IV_BV_ICL,4,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	DECLARE_INT_SET(1, __s);
	DeclareBoolVar(2, __b, home);
	ConLevel(3, __ICL_DEF);
	try{
		dom(home,__x,__s,__b,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(bool_not_BV_BV_ICL,3,0) {
	DeclareGSpace(home);
	DeclareBoolVar(0, __b0, home);
	DeclareBoolVar(1, __b1, home);
	ConLevel(2, __ICL_DEF);
	try{
		bool_not(home,__b0,__b1,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(bool_eq_BV_BV_ICL,3,0) {
	DeclareGSpace(home);
	DeclareBoolVar(0, __b0, home);
	DeclareBoolVar(1, __b1, home);
	ConLevel(2, __ICL_DEF);
	try{
		bool_eq(home,__b0,__b1,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(bool_and_BV_BV_BV_ICL,4,0) {
	DeclareGSpace(home);
	DeclareBoolVar(0, __b0, home);
	DeclareBoolVar(1, __b1, home);
	DeclareBoolVar(2, __b2, home);
	ConLevel(3, __ICL_DEF);
	try{
		bool_and(home,__b0,__b1,__b2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(bool_and_BV_BV_Bl_ICL,4,0) {
	DeclareGSpace(home);
	DeclareBoolVar(0, __b0, home);
	DeclareBoolVar(1, __b1, home);
	DeclareBoolVar(2, __b2, home);
	ConLevel(3, __ICL_DEF);
	try{
		bool_and(home,__b0,__b1,__b2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(bool_or_BV_BV_BV_ICL,4,0) {
	DeclareGSpace(home);
	DeclareBoolVar(0, __b0, home);
	DeclareBoolVar(1, __b1, home);
	DeclareBoolVar(2, __b2, home);
	ConLevel(3, __ICL_DEF);
	try{
		bool_or(home,__b0,__b1,__b2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(bool_or_BV_BV_Bl_ICL,4,0) {
	DeclareGSpace(home);
	DeclareBoolVar(0, __b0, home);
	DeclareBoolVar(1, __b1, home);
	DeclareBoolVar(2, __b2, home);
	ConLevel(3, __ICL_DEF);
	try{
		bool_or(home,__b0,__b1,__b2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(bool_imp_BV_BV_BV_ICL,4,0) {
	DeclareGSpace(home);
	DeclareBoolVar(0, __b0, home);
	DeclareBoolVar(1, __b1, home);
	DeclareBoolVar(2, __b2, home);
	ConLevel(3, __ICL_DEF);
	try{
		bool_imp(home,__b0,__b1,__b2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(bool_imp_BV_BV_Bl_ICL,4,0) {
	DeclareGSpace(home);
	DeclareBoolVar(0, __b0, home);
	DeclareBoolVar(1, __b1, home);
	DeclareBoolVar(2, __b2, home);
	ConLevel(3, __ICL_DEF);
	try{
		bool_imp(home,__b0,__b1,__b2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(bool_eqv_BV_BV_BV_ICL,4,0) {
	DeclareGSpace(home);
	DeclareBoolVar(0, __b0, home);
	DeclareBoolVar(1, __b1, home);
	DeclareBoolVar(2, __b2, home);
	ConLevel(3, __ICL_DEF);
	try{
		bool_eqv(home,__b0,__b1,__b2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(bool_eqv_BV_BV_Bl_ICL,4,0) {
	DeclareGSpace(home);
	DeclareBoolVar(0, __b0, home);
	DeclareBoolVar(1, __b1, home);
	DeclareBoolVar(2, __b2, home);
	ConLevel(3, __ICL_DEF);
	try{
		bool_eqv(home,__b0,__b1,__b2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(bool_xor_BV_BV_BV_ICL,4,0) {
	DeclareGSpace(home);
	DeclareBoolVar(0, __b0, home);
	DeclareBoolVar(1, __b1, home);
	DeclareBoolVar(2, __b2, home);
	ConLevel(3, __ICL_DEF);
	try{
		bool_xor(home,__b0,__b1,__b2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(bool_xor_BV_BV_Bl_ICL,4,0) {
	DeclareGSpace(home);
	DeclareBoolVar(0, __b0, home);
	DeclareBoolVar(1, __b1, home);
	DeclareBoolVar(2, __b2, home);
	ConLevel(3, __ICL_DEF);
	try{
		bool_xor(home,__b0,__b1,__b2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(sortedness_IVA_IVA_ICL,3,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INTVARARGS(1, __y, home);
	ConLevel(2, __icl);
	try{
		sortedness(home,__x,__y,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(sortedness_IVA_IVA_IVA_ICL,4,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INTVARARGS(1, __y, home);
	DECLARE_INTVARARGS(2, __z, home);
	ConLevel(3, __icl);
	try{
		sortedness(home,__x,__y,__z,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(distinct_IVA_ICL,2,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	ConLevel(1, __icl);
	try{
		distinct(home,__x,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(distinct_IA_IVA_ICL,3,0) {
	DeclareGSpace(home);
	DECLARE_INTARGS(0, __n);
	DECLARE_INTVARARGS(1, __x, home);
	ConLevel(2, __icl);
	try{
		distinct(home,__n,__x,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(element_IA_IV_IV_ICL,4,0) {
	DeclareGSpace(home);
	DECLARE_INTARGS(0, __n);
	DeclareGeIntVar(1, __x0, home);
	DeclareGeIntVar(2, __x1, home);
	ConLevel(3, __ICL_DEF);
	try{
		element(home,__n,__x0,__x1,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(element_IVA_IV_IV_ICL,4,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y0, home);
	DeclareGeIntVar(2, __y1, home);
	ConLevel(3, __icl);
	try{
		element(home,__x,__y0,__y1,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(rel_IV_IRT_IV_ICL,4,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	RelType(1, __r);
	DeclareGeIntVar(2, __x1, home);
	ConLevel(3, __icl);
	try{
		rel(home,__x0,__r,__x1,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(rel_IV_IRT_In_ICL,4,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	RelType(1, __r);
	DeclareGeIntVar(2, __c, home);
	ConLevel(3, __icl);
	try{
		rel(home,__x,__r,__c,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(rel_IV_IRT_IV_BV_ICL,5,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	RelType(1, __r);
	DeclareGeIntVar(2, __x1, home);
	DeclareBoolVar(3, __b, home);
	ConLevel(4, __icl);
	try{
		rel(home,__x0,__r,__x1,__b,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(rel_IV_IRT_In_BV_ICL,5,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	RelType(1, __r);
	DeclareGeIntVar(2, __c, home);
	DeclareBoolVar(3, __b, home);
	ConLevel(4, __icl);
	try{
		rel(home,__x,__r,__c,__b,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(rel_IVA_IRT_IVA_ICL,4,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	RelType(1, __r);
	DECLARE_INTVARARGS(2, __y, home);
	ConLevel(3, __icl);
	try{
		rel(home,__x,__r,__y,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(min_IV_IV_IV_ICL,4,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	DeclareGeIntVar(1, __x1, home);
	DeclareGeIntVar(2, __x2, home);
	ConLevel(3, __ICL_DEF);
	try{
		min(home,__x0,__x1,__x2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(min_IVA_IV_ICL,3,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y, home);
	ConLevel(2, __ICL_DEF);
	try{
		min(home,__x,__y,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(max_IV_IV_IV_ICL,4,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	DeclareGeIntVar(1, __x1, home);
	DeclareGeIntVar(2, __x2, home);
	ConLevel(3, __ICL_DEF);
	try{
		max(home,__x0,__x1,__x2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(max_IVA_IV_ICL,3,0) {
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y, home);
	ConLevel(2, __ICL_DEF);
	try{
		max(home,__x,__y,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(abs_IV_IV_ICL,3,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	DeclareGeIntVar(1, __x1, home);
	ConLevel(2, __ICL_DEF);
	try{
		abs(home,__x0,__x1,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


OZ_BI_define(mult_IV_IV_IV_ICL,4,0) {
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	DeclareGeIntVar(1, __x1, home);
	DeclareGeIntVar(2, __x2, home);
	ConLevel(3, __ICL_DEF);
	try{
		mult(home,__x0,__x1,__x2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(home);

}OZ_BI_end


