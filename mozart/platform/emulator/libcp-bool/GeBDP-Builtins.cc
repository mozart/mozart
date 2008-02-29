/*
 *  Main authors:
 *     Diana Lorena Velasco <dlvelasco@puj.edu.co>
 *     Juan Gabriel Torres  <juantorres@puj.edu.co>
 *
 *  Contributing authors:
 *  	Andres Felipe Barco (anfelbar@univalle.edu.co)
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
	DeclareIntConLevel(3, __ICL_DEF);
	DeclarePropKind(4, __PK_DEF);
	if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
		DeclareGeBoolVar(0, __x0, home);
		DeclareIntRelType(1, __r);
		DeclareGeBoolVar(2, __x1, home);
		try{
			Gecode::rel(home, __x0, __r, __x1, __ICL_DEF, __PK_DEF);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
		DECLARE_BOOLVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareGeBoolVar(2, __y, home);
		try{
			Gecode::rel(home, __x, __r, __y, __ICL_DEF, __PK_DEF);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
		DeclareGeBoolVar(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareInt2(2, __n);
		try{
			Gecode::rel(home, __x, __r, __n, __ICL_DEF, __PK_DEF);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolOpType(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
		DeclareBoolOpType(0, __o);
		DECLARE_BOOLVARARGS(1, __x, home);
		DeclareGeBoolVar(2, __y, home);
		try{
			Gecode::rel(home, __o, __x, __y, __ICL_DEF, __PK_DEF);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolOpType(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
		DeclareBoolOpType(0, __o);
		DECLARE_BOOLVARARGS(1, __x, home);
		DeclareInt2(2, __n);
		try{
			Gecode::rel(home, __o, __x, __n, __ICL_DEF, __PK_DEF);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
		DECLARE_INTVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareInt2(2, __c);
		try{
			Gecode::rel(home, __x, __r, __c, __ICL_DEF, __PK_DEF);
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
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isBoolOpType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
		DeclareGeBoolVar(3, __x2, home);
		try{
			Gecode::rel(home, __x0, __o, __x1, __x2, __ICL_DEF, __PK_DEF);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isBoolOpType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
		DeclareInt2(3, __n);
		try{
			Gecode::rel(home, __x0, __o, __x1, __n, __ICL_DEF, __PK_DEF);
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

OZ_BI_define(gbd_rel_3,3,0){
	DeclareGSpace(home);
	if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2))){
		DeclareGeBoolVar(0, __x0, home);
		DeclareIntRelType(1, __r);
		DeclareGeBoolVar(2, __x1, home);
		try{
			Gecode::rel(home, __x0, __r, __x1);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2))){
		DECLARE_BOOLVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareGeBoolVar(2, __y, home);
		try{
			Gecode::rel(home, __x, __r, __y);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2))){
		DeclareGeBoolVar(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareInt2(2, __n);
		try{
			Gecode::rel(home, __x, __r, __n);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolOpType(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2))){
		DeclareBoolOpType(0, __o);
		DECLARE_BOOLVARARGS(1, __x, home);
		DeclareGeBoolVar(2, __y, home);
		try{
			Gecode::rel(home, __o, __x, __y);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolOpType(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isInt(OZ_in(2))){
		DeclareBoolOpType(0, __o);
		DECLARE_BOOLVARARGS(1, __x, home);
		DeclareInt2(2, __n);
		try{
			Gecode::rel(home, __o, __x, __n);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2))){
		DECLARE_INTVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareInt2(2, __c);
		try{
			Gecode::rel(home, __x, __r, __c);
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

OZ_BI_define(gbd_rel_4,4,0){

if(OZ_isGeBoolVar(OZ_in(0)))
{printf("posicion 0 si es geboolvar\n"); fflush(stdout);}

if(OZ_isBoolOpType(OZ_in(1)))
{printf("posicion 1 si es booloptype\n"); fflush(stdout);}

if(OZ_isGeBoolVar(OZ_in(2)))
{printf("posicion 2 si es geboolvar\n"); fflush(stdout);}

if(OZ_isGeBoolVar(OZ_in(3)))
{printf("posicion 3 si es geboolvar\n"); fflush(stdout);}

	DeclareGSpace(home);
	DeclareGeBoolVar(0, __x0, home);
	DeclareBoolOpType(1, __o);
	DeclareGeBoolVar(2, __x1, home);
	if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isBoolOpType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
		DeclareGeBoolVar(3, __x2, home);
		try{
			Gecode::rel(home, __x0, __o, __x1, __x2);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isBoolOpType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isInt(OZ_in(3))){
		DeclareInt2(3, __n);
		try{
			Gecode::rel(home, __x0, __o, __x1, __n);
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
	if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
		DECLARE_BOOLVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareInt2(2, __c);
		DeclareIntConLevel(3, __ICL_DEF);
		DeclarePropKind(4, __PK_DEF);
		try{
			Gecode::linear(home, __x, __r, __c, __ICL_DEF, __PK_DEF);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
		DECLARE_BOOLVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareGeIntVar(2, __y, home);
		DeclareIntConLevel(3, __ICL_DEF);
		DeclarePropKind(4, __PK_DEF);
		try{
			Gecode::linear(home, __x, __r, __y, __ICL_DEF, __PK_DEF);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isIntArgs(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isGeBoolVar(OZ_in(4))){
		DECLARE_INTARGS(0, __a);
		DECLARE_BOOLVARARGS(1, __x, home);
		DeclareIntRelType(2, __r);
		DeclareInt2(3, __c);
		DeclareGeBoolVar(4, __b, home);
		try{
			Gecode::linear(home, __a, __x, __r, __c, __b);
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
	DeclareIntConLevel(4, __ICL_DEF);
	DeclarePropKind(5, __PK_DEF);
	if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
		DECLARE_BOOLVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareInt2(2, __c);
		DeclareGeBoolVar(3, __b, home);
		try{
			Gecode::linear(home, __x, __r, __c, __b, __ICL_DEF, __PK_DEF);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
		DECLARE_BOOLVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareGeIntVar(2, __y, home);
		DeclareGeBoolVar(3, __b, home);
		try{
			Gecode::linear(home, __x, __r, __y, __b, __ICL_DEF, __PK_DEF);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isIntArgs(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
		DECLARE_INTARGS(0, __a);
		DECLARE_BOOLVARARGS(1, __x, home);
		DeclareIntRelType(2, __r);
		DeclareInt2(3, __c);
		try{
			Gecode::linear(home, __a, __x, __r, __c, __ICL_DEF, __PK_DEF);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isIntArgs(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
		DECLARE_INTARGS(0, __a);
		DECLARE_BOOLVARARGS(1, __x, home);
		DeclareIntRelType(2, __r);
		DeclareGeIntVar(3, __y, home);
		try{
			Gecode::linear(home, __a, __x, __r, __y, __ICL_DEF, __PK_DEF);
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
	DeclareInt2(3, __c);
	DeclareGeBoolVar(4, __b, home);
	DeclareIntConLevel(5, __ICL_DEF);
	DeclarePropKind(6, __PK_DEF);
	try{
		Gecode::linear(home, __a, __x, __r, __c, __b, __ICL_DEF, __PK_DEF);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gbd_linear_3,3,0){
	DeclareGSpace(home);
	DECLARE_BOOLVARARGS(0, __x, home);
	DeclareIntRelType(1, __r);
	if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2))){
		DeclareInt2(2, __c);
		try{
			Gecode::linear(home, __x, __r, __c);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2))){
		DeclareGeIntVar(2, __y, home);
		try{
			Gecode::linear(home, __x, __r, __y);
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

OZ_BI_define(gbd_linear_4,4,0){
	DeclareGSpace(home);
	if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
		DECLARE_BOOLVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareInt2(2, __c);
		DeclareGeBoolVar(3, __b, home);
		try{
			Gecode::linear(home, __x, __r, __c, __b);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
		DECLARE_BOOLVARARGS(0, __x, home);
		DeclareIntRelType(1, __r);
		DeclareGeIntVar(2, __y, home);
		DeclareGeBoolVar(3, __b, home);
		try{
			Gecode::linear(home, __x, __r, __y, __b);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isIntArgs(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3))){
		DECLARE_INTARGS(0, __a);
		DECLARE_BOOLVARARGS(1, __x, home);
		DeclareIntRelType(2, __r);
		DeclareInt2(3, __c);
		try{
			Gecode::linear(home, __a, __x, __r, __c);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isIntArgs(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3))){
		DECLARE_INTARGS(0, __a);
		DECLARE_BOOLVARARGS(1, __x, home);
		DeclareIntRelType(2, __r);
		DeclareGeIntVar(3, __y, home);
		try{
			Gecode::linear(home, __a, __x, __r, __y);
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

