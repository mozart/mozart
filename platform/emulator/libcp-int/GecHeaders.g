/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alejandro Arbelaez (aarbelaez@cic.puj.edu.co)
 *
 *  Contributing authors:
 *
 *  Copyright:
 *    Alejandro Arbelaez, 2007
 *    Gustavo Gutierrez, 2007
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


grammar GecHeaders;

decls 
	:	funcDecl (funcDecl)*
	;
	
funcDecl 
	:	type funcName '(' params ')' ';'
	;

funcName 
	:	(nameSpace)? ID	
	;
	
nameSpace 
	:	ID '::'
	;

params 
	:	term  (','term)*
	;

term 
	:	(accMod)? type ID ('=' value)?
	|	type '=' value
	;
	
type  
	:	basicType (memMod)?
	;


basicType 
	:	gecType
	|	cType
	;
	

gecType 
	:	'Space'
	|	'IntVar'
	|	'IntVarArgs'
	|	'IntArgs'
	|	'IntRelType'
	|	'IntSet'
	|	'IntConLevel'
	|	'DFA'
	|	'BoolVar'
	|	'BoolVarArgs'
	;

cType 
	:	'int'
	|	'bool'
	|	'void'
	;

accMod :	'const';

memMod 
	:	pointer
	|	reference
	;
	
pointer :	'*';

reference :	 '&';

value 
	:	ID
	|	NUMBER
	;
	 
ID 	:	('a'..'z'|'A'..'Z' |'_' )('a'..'z'|'A'..'Z'|'_'|'0'..'9')* ;

NUMBER :	('0'..'9')*;
WS      : (' '|'\t'|'\n')+ {channel=99;};
