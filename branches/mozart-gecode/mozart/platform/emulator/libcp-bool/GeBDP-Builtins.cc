/*
 *  Main authors:
 *     Diana Lorena Velasco <dlvelasco@puj.edu.co>
 *     Juan Gabriel Torres  <juantorres@puj.edu.co>
 *
 *  Contributing authors:
 *  		Andres Felipe Barco <anfelbar@univalle.edu.co>   
 *
 *  Copyright:
 *     Diana Lorena Velasco, 2007
 *     Juan Gabriel Torres, 2007
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 *
 *  See the file 'LICENSE' for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */


/**
		This file is generated automatic by other aplication, 
            please do not modified this file.
*/

#ifndef __GEOZ_BOOLVAR_PROP_BUILTINS_CC__
#define __GEOZ_BOOLVAR_PROP_BUILTINS_CC__

//#include "GeBDP-Builtins.hh"
#include "BoolVarMacros.hh"

OZ_BI_define(gbd_rel_5,5,0){
	DeclareGSpace(home);
	DeclareIntConLevel(3, __icl);
	DeclarePropKind(4, __pk);
	if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
 /**
      void  Gecode::rel (Space *home, BoolVar x0, IntRelType r, BoolVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
        Post propagator for $ x_0 \sim_r x_1$. 
  */
		DeclareGeBoolVar(0, __x0, home);
		DeclareIntRelType(1, __r);
		DeclareGeBoolVar(2, __x1, home);
		try{
			Gecode::rel(home, __x0, __r, __x1, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
/**
    void  Gecode::rel (Space *home, const BoolVarArgs &x, IntRelType r, BoolVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
      Post propagator for $ x_i \sim_r y $ for all $0\leq i<|x|$.   
*/
		DECLARE_BOOLVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareGeBoolVar(2, __y, home);
		try{
			Gecode::rel(home, __x, __r, __y, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
/**
    void  Gecode::rel (Space *home, BoolVar x, IntRelType r, int n, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
      Propagates $ x \sim_r n$. 
 */ 
		DeclareGeBoolVar(0, __x0, home);
		DeclareIntRelType(1, __r);
		DeclareInt2(2, __n);
		try{
			Gecode::rel(home, __x0, __r, __n, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolOpType(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
/**
    void  Gecode::rel (Space *home, BoolOpType o, const BoolVarArgs &x, BoolVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
  Post propagator for Boolean operation on x.   
*/
		DeclareBoolOpType(0, __o);
		DECLARE_BOOLVARARGS(1, __x, home);
		DeclareGeBoolVar(2, __y, home);
		try{
			Gecode::rel(home, __o, __x, __y, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolOpType(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
/**
    void  Gecode::rel (Space *home, BoolOpType o, const BoolVarArgs &x, int n, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
  Post propagator for Boolean operation on x. 
*/  
		DeclareBoolOpType(0, __o);
		DECLARE_BOOLVARARGS(1, __x, home);
		DeclareInt2(2, __n);
		try{
			Gecode::rel(home, __o, __x, __n, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
/**
    void Gecode::rel (Space* home, const BoolVarArgs& x, IntRelType r, int n, IntConLevel icl, PropKind pk);  
*/
		DECLARE_BOOLVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareInt2(2, __n);
		try{
			Gecode::rel(home, __x, __r, __n, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else{
		OZ_typeError(0, "Malformed Propagator");
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gbd_rel_6,6,0){
	DeclareGSpace(home);
	DeclareGeBoolVar(0, __x0, home);
	DeclareBoolOpType(1, __o);
	DeclareGeBoolVar(2, __x1, home);
	DeclareIntConLevel(4, __icl);
	DeclarePropKind(5, __pk);
	if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isBoolOpType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
/**
    void  Gecode::rel (Space *home, BoolVar x0, BoolOpType o, BoolVar x1, BoolVar x2, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
  Post propagator for Boolean operation on x0 and x1.  
 */
		DeclareGeBoolVar(3, __x2, home);
		try{
			Gecode::rel(home, __x0, __o, __x1, __x2, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isBoolOpType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
/**
    void  Gecode::rel (Space *home, BoolVar x0, BoolOpType o, BoolVar x1, int n, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
    Post propagator for Boolean operation on x0 and x1.  
*/
		DeclareInt2(3, __n);
		try{
			Gecode::rel(home, __x0, __o, __x1, __n, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else{
		OZ_typeError(0, "Malformed Propagator");
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gbd_linear_5,5,0){
	DeclareGSpace(home);
	DECLARE_BOOLVARARGS(0, __x, home);
	DeclareIntRelType(1, __r);
	DeclareIntConLevel(3, __icl);
	DeclarePropKind(4, __pk);
	if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
/**
    void Gecode::linear (Space* home, const BoolVarArgs& x, IntRelType r, int c, IntConLevel icl, PropKind pk); 
 */
		DeclareInt2(2, __c);
		try{
			Gecode::linear(home, __x, __r, __c, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
/**
    void Gecode::linear (Space* home, const BoolVarArgs& x, IntRelType r, IntVar y, IntConLevel icl, PropKind pk); 
 */
		DeclareGeIntVar(2, __y, home);
		try{
			Gecode::linear(home, __x, __r, __y, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else{
		OZ_typeError(0, "Malformed Propagator");
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gbd_linear_6,6,0){
	DeclareGSpace(home);
	DeclareIntConLevel(4, __icl);
	DeclarePropKind(5, __pk);
	if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
/**
    void Gecode::linear (Space* home, const BoolVarArgs& x, IntRelType r, int c, BoolVar b, IntConLevel icl, PropKind pk); 
 */
		DECLARE_BOOLVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareInt2(2, __c);
		DeclareGeBoolVar(3, __b, home);
		try{
			Gecode::linear(home, __x, __r, __c, __b, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
/**
     void Gecode::linear (Space* home, const BoolVarArgs& x, IntRelType r, IntVar y, BoolVar b, IntConLevel icl,  PropKind pk);
 */
		DECLARE_BOOLVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareGeIntVar(2, __y, home);
		DeclareGeBoolVar(3, __b, home);
		try{
			Gecode::linear(home, __x, __r, __y, __b, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isIntArgs(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
/**
    void Gecode::linear (Space* home, const IntArgs& a, const BoolVarArgs& x, IntRelType r, int c, IntConLevel icl, PropKind pk); 
 */
		DECLARE_INTARGS(0, __a);
		DECLARE_BOOLVARARGS(1, __x, home);
		DeclareIntRelType(2, __r);
		DeclareInt2(3, __c);
		try{
			Gecode::linear(home, __a, __x, __r, __c, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isIntArgs(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
/**
    void Gecode::linear (Space* home, const IntArgs& a, const BoolVarArgs& x, IntRelType r, IntVar y, IntConLevel icl, PropKind pk);  
*/
		DECLARE_INTARGS(0, __a);
		DECLARE_BOOLVARARGS(1, __x, home);
		DeclareIntRelType(2, __r);
		DeclareGeIntVar(3, __y, home);
		try{
			Gecode::linear(home, __a, __x, __r, __y, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else{
		OZ_typeError(0, "Malformed Propagator");
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gbd_linear_7,7,0){
	DeclareGSpace(home);
	DECLARE_INTARGS(0, __a);
	DECLARE_BOOLVARARGS(1, __x, home);
	DeclareIntRelType(2, __r);
	DeclareGeBoolVar(4, __b, home);
	DeclareIntConLevel(5, __icl);
	DeclarePropKind(6, __pk);
	if(OZ_isIntArgs(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isGeBoolVar(OZ_in(4)) && OZ_isIntConLevel(OZ_in(5)) && OZ_isPropKind(OZ_in(6))){
/**
    void Gecode::linear (Space* home, const IntArgs& a, const BoolVarArgs& x, IntRelType r, int c, BoolVar b, IntConLevel icl, PropKind pk);
 */ 
		DeclareInt2(3, __c);
		try{
			Gecode::linear(home, __a, __x, __r, __c, __b, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isIntArgs(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isGeBoolVar(OZ_in(4)) && OZ_isIntConLevel(OZ_in(5)) && OZ_isPropKind(OZ_in(6))){
/**
    void Gecode::linear (Space* home, const IntArgs& a, const BoolVarArgs& x, IntRelType r, IntVar y, BoolVar b, IntConLevel icl, PropKind pk);
 */   
		DeclareGeIntVar(3, __y, home);
		try{
			Gecode::linear(home, __a, __x, __r, __y, __b, __icl, __pk);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else{
		OZ_typeError(0, "Malformed Propagator");
	}
	CHECK_POST(home);
}OZ_BI_end

#endif
