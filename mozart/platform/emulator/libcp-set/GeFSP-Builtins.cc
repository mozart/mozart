/*
 *  Main authors:
 *     Diana Lorena Velasco <dlvelasco@puj.edu.co>
 *     Juan Gabriel Torres  <juantorres@puj.edu.co>
 *
 *  Contributing authors:
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>
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

//#include "GeFSP-Builtins.hh"
#include "SetVarMacros.hh"

OZ_BI_define(gfs_dom_3,3,0){
	DeclareGSpace(home);
	DeclareGeSetVar(0, __x, home);
	DeclareSetRelType(1, __r);
	DeclareInt2(2, __i);
	try{
		Gecode::dom(home, __x, __r, __i);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_dom_4,4,0){
	DeclareGSpace(home);
	DeclareGeSetVar(0, __x, home);
	DeclareSetRelType(1, __r);
	DeclareInt2(2, __i);
	if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isInt(OZ_in(3))){
		DeclareInt2(3, __j);
		try{
			Gecode::dom(home, __x, __r, __i, __j);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
		DeclareGeBoolVar(3, __b, home);
		try{
			Gecode::dom(home, __x, __r, __i, __b);
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

OZ_BI_define(gfs_dom_5,5,0){
	DeclareGSpace(home);
	DeclareGeSetVar(0, __x, home);
	DeclareSetRelType(1, __r);
	DeclareInt2(2, __i);
	DeclareInt2(3, __j);
	DeclareGeBoolVar(4, __b, home);
	try{
		Gecode::dom(home, __x, __r, __i, __j, __b);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_rel_3,3,0){
	DeclareGSpace(home);
	if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2))){
		DeclareGeSetVar(0, __x, home);
		DeclareSetRelType(1, __r);
		DeclareGeSetVar(2, __y, home);
		try{
			Gecode::rel(home, __x, __r, __y);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2))){
		DeclareGeSetVar(0, __s, home);
		DeclareSetRelType(1, __r);
		DeclareGeIntVar(2, __x, home);
		try{
			Gecode::rel(home, __s, __r, __x);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2))){
		DeclareGeIntVar(0, __x, home);
		DeclareSetRelType(1, __r);
		DeclareGeSetVar(2, __s, home);
		try{
			Gecode::rel(home, __x, __r, __s);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2))){
		DeclareGeSetVar(0, __s, home);
		DeclareSetRelType(1, __r);
		DeclareGeIntVar(2, __x, home);
		try{
			Gecode::rel(home, __s, __r, __x);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2))){
		DeclareGeIntVar(0, __x, home);
		DeclareSetRelType(1, __r);
		DeclareGeSetVar(2, __s, home);
		try{
			Gecode::rel(home, __x, __r, __s);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isSetOpType(OZ_in(0)) && OZ_isSetVarArgs(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2))){
		DeclareSetOpType(0, __op);
		DECLARE_SETVARARGS(1, __x, home);
		DeclareGeSetVar(2, __y, home);
		try{
			Gecode::rel(home, __op, __x, __y);
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

OZ_BI_define(gfs_rel_4,4,0){
	DeclareGSpace(home);
	DeclareSetRelType(1, __r);
	DeclareGeBoolVar(3, __b, home);
	if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
		DeclareGeSetVar(0, __x, home);
		DeclareGeSetVar(2, __y, home);
		try{
			Gecode::rel(home, __x, __r, __y, __b);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
		DeclareGeSetVar(0, __s, home);
		DeclareGeIntVar(2, __x, home);
		try{
			Gecode::rel(home, __s, __r, __x, __b);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
		DeclareGeIntVar(0, __x, home);
		DeclareGeSetVar(2, __s, home);
		try{
			Gecode::rel(home, __x, __r, __s, __b);
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

OZ_BI_define(gfs_rel_5,5,0){
	DeclareGSpace(home);
	DeclareGeSetVar(0, __x, home);
	DeclareSetOpType(1, __op);
	DeclareGeSetVar(2, __y, home);
	DeclareSetRelType(3, __r);
	DeclareGeSetVar(4, __z, home);
	try{
		Gecode::rel(home, __x, __op, __y, __r, __z);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_sequence_1,1,0){
	DeclareGSpace(home);
	DECLARE_SETVARARGS(0, __x, home);
	try{
		Gecode::sequence(home, __x);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_min_2,2,0){
	DeclareGSpace(home);
	DeclareGeSetVar(0, __s, home);
	DeclareGeIntVar(1, __x, home);
	try{
		Gecode::min(home, __s, __x);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_match_2,2,0){
	DeclareGSpace(home);
	DeclareGeSetVar(0, __s, home);
	DECLARE_INTVARARGS(1, __x, home);
	try{
		Gecode::match(home, __s, __x);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_channel_2,2,0){
	DeclareGSpace(home);
	if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isSetVarArgs(OZ_in(1))){
		DECLARE_INTVARARGS(0, __x, home);
		DECLARE_SETVARARGS(1, __y, home);
		try{
			Gecode::channel(home, __x, __y);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
	}
	else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isGeSetVar(OZ_in(1))){
		DECLARE_BOOLVARARGS(0, __x, home);
		DeclareGeSetVar(1, __y, home);
		try{
			Gecode::channel(home, __x, __y);
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

OZ_BI_define(gfs_selectUnion_3,3,0){
	DeclareGSpace(home);
	DECLARE_SETVARARGS(0, __x, home);
	DeclareGeSetVar(1, __y, home);
	DeclareGeSetVar(2, __z, home);
	try{
		Gecode::selectUnion(home, __x, __y, __z);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_selectDisjoint_2,2,0){
	DeclareGSpace(home);
	DECLARE_SETVARARGS(0, __x, home);
	DeclareGeSetVar(1, __y, home);
	try{
		Gecode::selectDisjoint(home, __x, __y);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_selectSet_3,3,0){
	DeclareGSpace(home);
	DECLARE_SETVARARGS(0, __x, home);
	DeclareGeIntVar(1, __y, home);
	DeclareGeSetVar(2, __z, home);
	try{
		Gecode::selectSet(home, __x, __y, __z);
	}
	catch(Exception e){
		RAISE_GE_EXCEPTION(e);
	}
	CHECK_POST(home);
}OZ_BI_end

#endif

