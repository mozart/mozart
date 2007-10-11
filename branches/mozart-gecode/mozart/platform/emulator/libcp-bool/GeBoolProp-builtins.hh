/*
 *  Main authors:
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *
 *  Contributing authors:
 *
 *  Copyright:
 *    Alberto Delgado, 2006-2007
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __GEOZ_BOOL_PROP_BUILTINS_HH__
#define __GEOZ_BOOL_PROP_BUILTINS_HH__



#include "GeBoolVar-builtins.hh"
#include "../libcp-int/GeIntProp-builtins.hh"
#include "gecode/int.hh"

										
#define DeclareBoolOpType(arg,var)\
	BoolOpType var;\
	{\
		OZ_declareInt(arg,op);\
		switch(op) {\
		case 0: var = BOT_AND; break;\
		case 1: var = BOT_OR; break;\
		case 2: var = BOT_IMP; break;\
		case 3: var = BOT_EQV; break;\
		case 4: var = BOT_XOR; break;\
		default: return OZ_typeError(arg,"Expecting atom with a lgical operation: and, or, imp, eqv, xor");\
	}}	
	

#endif
