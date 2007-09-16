OZ_BI_define(linear_IVA_IRT_In_ICL,5,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __c, sp);
	ConLevel(4, __ICL_DEF);
	try{
		linear(sp,__x,__r,__c,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IVA_IRT_IV_ICL,5,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __y, sp);
	ConLevel(4, __ICL_DEF);
	try{
		linear(sp,__x,__r,__y,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IVA_IRT_In_BV_ICL,6,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __c, sp);
	DeclareBoolVar(4, __b, sp);
	ConLevel(5, __ICL_DEF);
	try{
		linear(sp,__x,__r,__c,__b,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IVA_IRT_IV_BV_ICL,6,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __y, sp);
	DeclareBoolVar(4, __b, sp);
	ConLevel(5, __ICL_DEF);
	try{
		linear(sp,__x,__r,__y,__b,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IA_IVA_IRT_In_ICL,6,0) {
	DeclareGSpace(sp);
	DECLARE_INTARGS(1, __a);
	DECLARE_INTVARARRAY(2, __x, sp);
	RelType(3, __r);
	DeclareGeIntVar(4, __c, sp);
	ConLevel(5, __ICL_DEF);
	try{
		linear(sp,__a,__x,__r,__c,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IA_IVA_IRT_IV_ICL,6,0) {
	DeclareGSpace(sp);
	DECLARE_INTARGS(1, __a);
	DECLARE_INTVARARRAY(2, __x, sp);
	RelType(3, __r);
	DeclareGeIntVar(4, __y, sp);
	ConLevel(5, __ICL_DEF);
	try{
		linear(sp,__a,__x,__r,__y,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IA_IVA_IRT_In_BV_ICL,7,0) {
	DeclareGSpace(sp);
	DECLARE_INTARGS(1, __a);
	DECLARE_INTVARARRAY(2, __x, sp);
	RelType(3, __r);
	DeclareGeIntVar(4, __c, sp);
	DeclareBoolVar(5, __b, sp);
	ConLevel(6, __ICL_DEF);
	try{
		linear(sp,__a,__x,__r,__c,__b,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_IA_IVA_IRT_IV_BV_ICL,7,0) {
	DeclareGSpace(sp);
	DECLARE_INTARGS(1, __a);
	DECLARE_INTVARARRAY(2, __x, sp);
	RelType(3, __r);
	DeclareGeIntVar(4, __y, sp);
	DeclareBoolVar(5, __b, sp);
	ConLevel(6, __ICL_DEF);
	try{
		linear(sp,__a,__x,__r,__y,__b,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_BVA_IRT_In_ICL,5,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __c, sp);
	ConLevel(4, __ICL_DEF);
	try{
		linear(sp,__x,__r,__c,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(linear_BVA_IRT_IV_ICL,5,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __y, sp);
	ConLevel(4, __ICL_DEF);
	try{
		linear(sp,__x,__r,__y,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(eq_IV_IV_ICL,4,0) {
	DeclareGSpace(sp);
	DeclareGeIntVar(1, __x0, sp);
	DeclareGeIntVar(2, __x1, sp);
	ConLevel(3, __icl);
	try{
		eq(sp,__x0,__x1,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(eq_IV_In_ICL,4,0) {
	DeclareGSpace(sp);
	DeclareGeIntVar(1, __x, sp);
	DeclareGeIntVar(2, __n, sp);
	ConLevel(3, __ICL_DEF);
	try{
		eq(sp,__x,__n,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(eq_IV_IV_BV_ICL,5,0) {
	DeclareGSpace(sp);
	DeclareGeIntVar(1, __x0, sp);
	DeclareGeIntVar(2, __x1, sp);
	DeclareBoolVar(3, __b, sp);
	ConLevel(4, __icl);
	try{
		eq(sp,__x0,__x1,__b,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(eq_IV_In_BV_ICL,5,0) {
	DeclareGSpace(sp);
	DeclareGeIntVar(1, __x, sp);
	DeclareGeIntVar(2, __n, sp);
	DeclareBoolVar(3, __b, sp);
	ConLevel(4, __icl);
	try{
		eq(sp,__x,__n,__b,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(eq_IVA_ICL,3,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	ConLevel(2, __icl);
	try{
		eq(sp,__x,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(count_IVA_In_IRT_In_ICL,6,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	DeclareGeIntVar(2, __n, sp);
	RelType(3, __r);
	DeclareGeIntVar(4, __m, sp);
	ConLevel(5, __icl);
	try{
		count(sp,__x,__n,__r,__m,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(count_IVA_IV_IRT_In_ICL,6,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	DeclareGeIntVar(2, __y, sp);
	RelType(3, __r);
	DeclareGeIntVar(4, __m, sp);
	ConLevel(5, __icl);
	try{
		count(sp,__x,__y,__r,__m,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(count_IVA_In_IRT_IV_ICL,6,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	DeclareGeIntVar(2, __n, sp);
	RelType(3, __r);
	DeclareGeIntVar(4, __z, sp);
	ConLevel(5, __icl);
	try{
		count(sp,__x,__n,__r,__z,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(count_IVA_IV_IRT_IV_ICL,6,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	DeclareGeIntVar(2, __y, sp);
	RelType(3, __r);
	DeclareGeIntVar(4, __z, sp);
	ConLevel(5, __icl);
	try{
		count(sp,__x,__y,__r,__z,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(sortedness_IVA_IVA_ICL,4,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	DECLARE_INTVARARRAY(2, __y, sp);
	ConLevel(3, __icl);
	try{
		sortedness(sp,__x,__y,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(sortedness_IVA_IVA_IVA_ICL,5,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	DECLARE_INTVARARRAY(2, __y, sp);
	DECLARE_INTVARARRAY(3, __z, sp);
	ConLevel(4, __icl);
	try{
		sortedness(sp,__x,__y,__z,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(distinct_IVA_ICL,3,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	ConLevel(2, __icl);
	try{
		distinct(sp,__x,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(distinct_IA_IVA_ICL,4,0) {
	DeclareGSpace(sp);
	DECLARE_INTARGS(1, __n);
	DECLARE_INTVARARRAY(2, __x, sp);
	ConLevel(3, __icl);
	try{
		distinct(sp,__n,__x,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(rel_IV_IRT_IV_ICL,5,0) {
	DeclareGSpace(sp);
	DeclareGeIntVar(1, __x0, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __x1, sp);
	ConLevel(4, __icl);
	try{
		rel(sp,__x0,__r,__x1,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(rel_IV_IRT_In_ICL,5,0) {
	DeclareGSpace(sp);
	DeclareGeIntVar(1, __x, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __c, sp);
	ConLevel(4, __icl);
	try{
		rel(sp,__x,__r,__c,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(rel_IV_IRT_IV_BV_ICL,6,0) {
	DeclareGSpace(sp);
	DeclareGeIntVar(1, __x0, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __x1, sp);
	DeclareBoolVar(4, __b, sp);
	ConLevel(5, __icl);
	try{
		rel(sp,__x0,__r,__x1,__b,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(rel_IV_IRT_In_BV_ICL,6,0) {
	DeclareGSpace(sp);
	DeclareGeIntVar(1, __x, sp);
	RelType(2, __r);
	DeclareGeIntVar(3, __c, sp);
	DeclareBoolVar(4, __b, sp);
	ConLevel(5, __icl);
	try{
		rel(sp,__x,__r,__c,__b,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(rel_IVA_IRT_IVA_ICL,5,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	RelType(2, __r);
	DECLARE_INTVARARRAY(3, __y, sp);
	ConLevel(4, __icl);
	try{
		rel(sp,__x,__r,__y,__icl);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(min_IV_IV_IV_ICL,5,0) {
	DeclareGSpace(sp);
	DeclareGeIntVar(1, __x0, sp);
	DeclareGeIntVar(2, __x1, sp);
	DeclareGeIntVar(3, __x2, sp);
	ConLevel(4, __ICL_DEF);
	try{
		min(sp,__x0,__x1,__x2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(min_IVA_IV_ICL,4,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	DeclareGeIntVar(2, __y, sp);
	ConLevel(3, __ICL_DEF);
	try{
		min(sp,__x,__y,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(max_IV_IV_IV_ICL,5,0) {
	DeclareGSpace(sp);
	DeclareGeIntVar(1, __x0, sp);
	DeclareGeIntVar(2, __x1, sp);
	DeclareGeIntVar(3, __x2, sp);
	ConLevel(4, __ICL_DEF);
	try{
		max(sp,__x0,__x1,__x2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(max_IVA_IV_ICL,4,0) {
	DeclareGSpace(sp);
	DECLARE_INTVARARRAY(1, __x, sp);
	DeclareGeIntVar(2, __y, sp);
	ConLevel(3, __ICL_DEF);
	try{
		max(sp,__x,__y,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(abs_IV_IV_ICL,4,0) {
	DeclareGSpace(sp);
	DeclareGeIntVar(1, __x0, sp);
	DeclareGeIntVar(2, __x1, sp);
	ConLevel(3, __ICL_DEF);
	try{
		abs(sp,__x0,__x1,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


OZ_BI_define(mult_IV_IV_IV_ICL,5,0) {
	DeclareGSpace(sp);
	DeclareGeIntVar(1, __x0, sp);
	DeclareGeIntVar(2, __x1, sp);
	DeclareGeIntVar(3, __x2, sp);
	ConLevel(4, __ICL_DEF);
	try{
		mult(sp,__x0,__x1,__x2,__ICL_DEF);
	} catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	GZ_RETURN(sp);

}OZ_BI_end


