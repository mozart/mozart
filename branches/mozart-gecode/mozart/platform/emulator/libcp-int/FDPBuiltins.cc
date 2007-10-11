OZ_BI_define(dom_IV_In_In_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	OZ_declareInt(1, __l);
	OZ_declareInt(2, __m);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::dom(home, __x, __l, __m, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(dom_IVA_In_In_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	OZ_declareInt(1, __l);
	OZ_declareInt(2, __m);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::dom(home, __x, __l, __m, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(dom_IV_IS_ICL_PK,4,0)
{
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	DECLARE_INT_SET(1, __s);
	DeclareIntConLevel(2, __ICL_DEF);
	DeclarePropKind(3, __PK_DEF);
	try{
		Gecode::dom(home, __x, __s, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(dom_IVA_IS_ICL_PK,4,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INT_SET(1, __s);
	DeclareIntConLevel(2, __ICL_DEF);
	DeclarePropKind(3, __PK_DEF);
	try{
		Gecode::dom(home, __x, __s, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(dom_IV_In_In_BV_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	OZ_declareInt(1, __l);
	OZ_declareInt(2, __m);
	DeclareGeBoolVar(3, __b, home);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::dom(home, __x, __l, __m, __b, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(dom_IV_IS_BV_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	DECLARE_INT_SET(1, __s);
	DeclareGeBoolVar(2, __b, home);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::dom(home, __x, __s, __b, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_IV_IRT_IV_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	RelType(1, __r);
	DeclareGeIntVar(2, __x1, home);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::rel(home, __x0, __r, __x1, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_IV_IRT_In_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	RelType(1, __r);
	OZ_declareInt(2, __c);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::rel(home, __x, __r, __c, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_IV_IRT_IV_BV_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	RelType(1, __r);
	DeclareGeIntVar(2, __x1, home);
	DeclareGeBoolVar(3, __b, home);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::rel(home, __x0, __r, __x1, __b, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_IV_IRT_In_BV_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x, home);
	RelType(1, __r);
	OZ_declareInt(2, __c);
	DeclareGeBoolVar(3, __b, home);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::rel(home, __x, __r, __c, __b, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_IVA_IRT_ICL_PK,4,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	RelType(1, __r);
	DeclareIntConLevel(2, __ICL_DEF);
	DeclarePropKind(3, __PK_DEF);
	try{
		Gecode::rel(home, __x, __r, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_IVA_IRT_IVA_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	RelType(1, __r);
	DECLARE_INTVARARGS(2, __y, home);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::rel(home, __x, __r, __y, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_BV_IRT_BV_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DeclareGeBoolVar(0, __x0, home);
	RelType(1, __r);
	DeclareGeBoolVar(2, __x1, home);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::rel(home, __x0, __r, __x1, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_BV_IRT_In_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DeclareGeBoolVar(0, __x, home);
	RelType(1, __r);
	OZ_declareInt(2, __n);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::rel(home, __x, __r, __n, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_BVA_IRT_BVA_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DECLARE_BOOLVARARGS(0, __x, home);
	RelType(1, __r);
	DECLARE_BOOLVARARGS(2, __y, home);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::rel(home, __x, __r, __y, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_BVA_IRT_ICL_PK,4,0)
{
	DeclareGSpace(home);
	DECLARE_BOOLVARARGS(0, __x, home);
	RelType(1, __r);
	DeclareIntConLevel(2, __ICL_DEF);
	DeclarePropKind(3, __PK_DEF);
	try{
		Gecode::rel(home, __x, __r, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_BV_BOT_BV_BV_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DeclareGeBoolVar(0, __x0, home);
	DeclareBoolOpType(1, __o);
	DeclareGeBoolVar(2, __x1, home);
	DeclareGeBoolVar(3, __x2, home);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::rel(home, __x0, __o, __x1, __x2, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_BV_BOT_BV_In_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DeclareGeBoolVar(0, __x0, home);
	DeclareBoolOpType(1, __o);
	DeclareGeBoolVar(2, __x1, home);
	OZ_declareInt(3, __n);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::rel(home, __x0, __o, __x1, __n, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_BVA_BOT_BV_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DECLARE_BOOLVARARGS(0, __x, home);
	DeclareBoolOpType(1, __o);
	DeclareGeBoolVar(2, __y, home);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::rel(home, __x, __o, __y, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(rel_BVA_BOT_In_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DECLARE_BOOLVARARGS(0, __x, home);
	DeclareBoolOpType(1, __o);
	OZ_declareInt(2, __n);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::rel(home, __x, __o, __n, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(element_IA_IV_IV_In_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_INTARGS(0, __n);
	DeclareGeIntVar(1, __x0, home);
	DeclareGeIntVar(2, __x1, home);
	OZ_declareInt(3, __offset);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::element(home, __n, __x0, __x1, __offset, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(element_IA_IV_BV_In_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_INTARGS(0, __n);
	DeclareGeIntVar(1, __x0, home);
	DeclareGeBoolVar(2, __x1, home);
	OZ_declareInt(3, __offset);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::element(home, __n, __x0, __x1, __offset, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(element_IA_IV_In_In_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_INTARGS(0, __n);
	DeclareGeIntVar(1, __x0, home);
	OZ_declareInt(2, __x1);
	OZ_declareInt(3, __offset);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::element(home, __n, __x0, __x1, __offset, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(element_IVA_IV_IV_In_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y0, home);
	DeclareGeIntVar(2, __y1, home);
	OZ_declareInt(3, __offset);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::element(home, __x, __y0, __y1, __offset, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(element_IVA_IV_In_In_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y0, home);
	OZ_declareInt(2, __y1);
	OZ_declareInt(3, __offset);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::element(home, __x, __y0, __y1, __offset, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(element_BVA_IV_BV_In_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_BOOLVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y0, home);
	DeclareGeBoolVar(2, __y1, home);
	OZ_declareInt(3, __offset);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::element(home, __x, __y0, __y1, __offset, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(element_BVA_IV_In_In_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_BOOLVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y0, home);
	OZ_declareInt(2, __y1);
	OZ_declareInt(3, __offset);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::element(home, __x, __y0, __y1, __offset, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(distinct_IVA_ICL_PK,3,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareIntConLevel(1, __ICL_DEF);
	DeclarePropKind(2, __PK_DEF);
	try{
		Gecode::distinct(home, __x, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(distinct_IA_IVA_ICL_PK,4,0)
{
	DeclareGSpace(home);
	DECLARE_INTARGS(0, __n);
	DECLARE_INTVARARGS(1, __x, home);
	DeclareIntConLevel(2, __ICL_DEF);
	DeclarePropKind(3, __PK_DEF);
	try{
		Gecode::distinct(home, __n, __x, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(channel_IVA_IVA_ICL_PK,4,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INTVARARGS(1, __y, home);
	DeclareIntConLevel(2, __ICL_DEF);
	DeclarePropKind(3, __PK_DEF);
	try{
		Gecode::channel(home, __x, __y, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(channel_BV_IV_ICL_PK,4,0)
{
	DeclareGSpace(home);
	DeclareGeBoolVar(0, __x0, home);
	DeclareGeIntVar(1, __x1, home);
	DeclareIntConLevel(2, __ICL_DEF);
	DeclarePropKind(3, __PK_DEF);
	try{
		Gecode::channel(home, __x0, __x1, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(channel_IV_BV_ICL_PK,4,0)
{
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	DeclareGeBoolVar(1, __x1, home);
	DeclareIntConLevel(2, __ICL_DEF);
	DeclarePropKind(3, __PK_DEF);
	try{
		Gecode::channel(home, __x0, __x1, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(channel_BVA_IV_In_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DECLARE_BOOLVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y, home);
	OZ_declareInt(2, __o);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::channel(home, __x, __y, __o, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(circuit_IVA_ICL_PK,3,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareIntConLevel(1, __ICL_DEF);
	DeclarePropKind(2, __PK_DEF);
	try{
		Gecode::circuit(home, __x, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(sorted_IVA_IVA_ICL_PK,4,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INTVARARGS(1, __y, home);
	DeclareIntConLevel(2, __ICL_DEF);
	DeclarePropKind(3, __PK_DEF);
	try{
		Gecode::sorted(home, __x, __y, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(count_IVA_In_IRT_In_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	OZ_declareInt(1, __n);
	RelType(2, __r);
	OZ_declareInt(3, __m);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::count(home, __x, __n, __r, __m, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(count_IVA_IV_IRT_In_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y, home);
	RelType(2, __r);
	OZ_declareInt(3, __m);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::count(home, __x, __y, __r, __m, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(count_IVA_IA_IRT_In_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INTARGS(1, __y);
	RelType(2, __r);
	OZ_declareInt(3, __m);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::count(home, __x, __y, __r, __m, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(count_IVA_In_IRT_IV_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	OZ_declareInt(1, __n);
	RelType(2, __r);
	DeclareGeIntVar(3, __z, home);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::count(home, __x, __n, __r, __z, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(count_IVA_IV_IRT_IV_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y, home);
	RelType(2, __r);
	DeclareGeIntVar(3, __z, home);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::count(home, __x, __y, __r, __z, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(count_IVA_IA_IRT_IV_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INTARGS(1, __y);
	RelType(2, __r);
	DeclareGeIntVar(3, __z, home);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::count(home, __x, __y, __r, __z, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(count_IVA_IVA_ICL_PK,4,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DECLARE_INTVARARGS(1, __c, home);
	DeclareIntConLevel(2, __ICL_DEF);
	DeclarePropKind(3, __PK_DEF);
	try{
		Gecode::count(home, __x, __c, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(min_IV_IV_IV_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	DeclareGeIntVar(1, __x1, home);
	DeclareGeIntVar(2, __x2, home);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::min(home, __x0, __x1, __x2, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(min_IVA_IV_ICL_PK,4,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y, home);
	DeclareIntConLevel(2, __ICL_DEF);
	DeclarePropKind(3, __PK_DEF);
	try{
		Gecode::min(home, __x, __y, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(max_IV_IV_IV_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	DeclareGeIntVar(1, __x1, home);
	DeclareGeIntVar(2, __x2, home);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::max(home, __x0, __x1, __x2, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(max_IVA_IV_ICL_PK,4,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y, home);
	DeclareIntConLevel(2, __ICL_DEF);
	DeclarePropKind(3, __PK_DEF);
	try{
		Gecode::max(home, __x, __y, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(abs_IV_IV_ICL_PK,4,0)
{
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	DeclareGeIntVar(1, __x1, home);
	DeclareIntConLevel(2, __ICL_DEF);
	DeclarePropKind(3, __PK_DEF);
	try{
		Gecode::abs(home, __x0, __x1, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(mult_IV_IV_IV_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DeclareGeIntVar(0, __x0, home);
	DeclareGeIntVar(1, __x1, home);
	DeclareGeIntVar(2, __x2, home);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::mult(home, __x0, __x1, __x2, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(linear_IVA_IRT_In_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	RelType(1, __r);
	OZ_declareInt(2, __c);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::linear(home, __x, __r, __c, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(linear_IVA_IRT_IV_ICL_PK,5,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	RelType(1, __r);
	DeclareGeIntVar(2, __y, home);
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	try{
		Gecode::linear(home, __x, __r, __y, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(linear_IVA_IRT_In_BV_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	RelType(1, __r);
	OZ_declareInt(2, __c);
	DeclareGeBoolVar(3, __b, home);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::linear(home, __x, __r, __c, __b, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(linear_IVA_IRT_IV_BV_ICL_PK,6,0)
{
	DeclareGSpace(home);
	DECLARE_INTVARARGS(0, __x, home);
	RelType(1, __r);
	DeclareGeIntVar(2, __y, home);
	DeclareGeBoolVar(3, __b, home);
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	try{
		Gecode::linear(home, __x, __r, __y, __b, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

