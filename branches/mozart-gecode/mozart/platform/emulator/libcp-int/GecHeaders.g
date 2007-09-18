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

@header {
	import java.util.Vector;
	import java.io.FileOutputStream;
	import java.io.PrintStream;
}



@members {
	// pst functions counter
	int post_count = 0;
	
	// stores the number of recognized arguments of a post function
	int param_count = 0;
	
	// stores the name of the last recognized space
	String space_name;
	
	// true if the declaration needs a space as an argument
	boolean need_space = false;
	
	// The actual call to the gecode post function
	Vector call = new Vector();
	
	// A computed suffix for the post function in terms of its arguments
	String pref = new String();
	
	String declareBlock = new String();
	
	// c output 
	FileOutputStream FOSbuiltins;
	PrintStream PSbuiltins;
	
	//spec output
	FileOutputStream FOSspec;
	PrintStream PSspec;
	
	void initOS() {
		try {
			FOSbuiltins = new FileOutputStream("XXXbuiltins.cc");
			PSbuiltins = new PrintStream(FOSbuiltins);
			
			FOSspec = new FileOutputStream("XXXbuiltins.spec");
			PSspec = new PrintStream(FOSspec);
			
		} catch(Exception e) {
			System.err.println ("Error writing with output files");
		}
	}
	
	void write_builtins(String s) { PSbuiltins.println(s); }
	void write_spec(String s) { PSspec.println(s); }
	
	void closeOS() {
		PSbuiltins.close();
		PSspec.close();
	}
	
	void initGlobalVars() {
		param_count = 0;
		space_name = "";
		need_space = false;
		call.clear();
		pref = "";
		declareBlock = "";
	}
	
	void declareParam(String prefix, String name) {
		String s = new String("__"+name);
		call.addElement(s);
		String sblock = new String();
		int pos = param_count-1;
		if (need_space) {
			sblock += prefix+pos+", __"+name+", "+space_name+");\n\t";
			//System.out.println(sblock);
		} else {
			sblock += prefix+pos+", __"+name+");\n\t";
			//System.out.println(s);
		}
		declareBlock += sblock;
	}
	
	String postProp(String name) {
		String c = new String("try{\n\t\t"+name+"("+space_name);
		for (int i = 0; i < call.size (); i++)
            	c += ","+call.elementAt(i);
        c += ");\n\t} catch(Exception e){\n\t\tRAISE_GE_EXCEPTION(e);\n\t}\n\tGZ_RETURN("+space_name+");\n";
        return c; //System.out.println(c);
	}
	void head(String name) {
		String signature = new String(pref.substring(0,pref.length()-1));
		String builtinCall = new String(name+"_"+signature);
		String c = new String();
		c += "OZ_BI_define(" + builtinCall + ","+(param_count-1)+",0) {\n\t";
		c += declareBlock;
		c += postProp(name);
		c += "\n}OZ_BI_end\n\n";
		//System.out.println(c);
		write_builtins(c);
	}
	
	void spec(String name) {
		String signature = new String(pref.substring(0,pref.length()-1));
		String builtinCall = new String(name+"_"+signature);
		String s = new String();
		
		// remove the space from the call
		call.remove(0);
		if (post_count != 0)
			s += ",\n\n";
		s += "'"+builtinCall+"'=> { in => [";
		for (int i=0; i<call.size(); i++) {
			if (i==0)
				s += "'+value'";
			s += ",'+value'";
		}
		s += "],\n\t\t out=>[],\n\t\t bi => "+builtinCall+"}";
		//System.out.println(s);
		write_spec(s);
	}
}  // end members


decls 
	:	{initOS();} funcDecl (funcDecl)* {closeOS();}
	;
	
funcDecl 
	:	{initGlobalVars();} type funcName '(' params ')' ';'
		{
			head($funcName.value);
			spec($funcName.value);
			post_count++;
		}
	;

funcName returns [String value]
	:	(nameSpace)? ID	{$value = $ID.text;}
	;
	
nameSpace 
	:	ID '::'
	;

params 
	:	term  (','term)*
	;

term 
	:	(accMod)? type ID ('=' value)?
		{
			if (param_count == 0) {
				space_name = $ID.text;
				declareBlock += $type.value+space_name+");\n\t";
			} else
				declareParam($type.value,$ID.text);
			param_count++;
		}
	|	type '=' value
		{
			declareParam($type.value,$value.text); 
			param_count++;
		}
	;
	
type  returns [String value]
	:	basicType (memMod)?	
		{
			$value = $basicType.value;
		}
	;


basicType returns [String value]
	:	gecType		{$value = $gecType.value;}
	|	cType		{$value = $cType.value;}
	;
	

gecType returns [String value]
	:	'Space'				{$value = new String("DeclareGSpace(");}
	|	'IntVar'			{pref+= "IV_";$value = new String("DeclareGeIntVar(");need_space=true;}
	|	'IntVarArgs'		{pref+= "IVA_";$value = new String("DECLARE_INTVARARGS(");need_space=true;} 
	|	'IntArgs'			{pref+= "IA_";$value = new String("DECLARE_INTARGS(");need_space=false;} 
	|	'IntRelType'		{pref+= "IRT_";$value = new String("RelType(");need_space=false;} 
	|	'IntSet'			{pref+= "IV_";$value = new String("DECLARE_INT_SET(");need_space=false;}
	|	'IntConLevel'		{pref+= "ICL_";$value = new String("ConLevel(");need_space=false;}
	|	'DFA'				{pref+= "DFA_";$value = new String("DECLARE_DFA(");need_space=false;}
	|	'BoolVar'			{pref+= "BV_";$value = new String("DeclareBoolVar(");need_space=true;}
	|	'BoolVarArgs'		{pref+= "BVA_";$value = new String("DECLARE_INTVARARGS(");need_space=true;} 
	;

cType returns [String value]
	:	'int'				{pref+= "In_";$value = new String("DeclareGeIntVar(");need_space=true;}
	|	'intS'				{pref+= "InS_";$value = new String("OZ_declareInt(");need_space=false;}
	|	'bool'				{pref+= "Bl_";$value = new String("DeclareBoolVar(");need_space=true;}
	|	'boolS'				{pref+= "BlS_";$value = new String("OZ_declareInt(");need_space=false;}
	|	'void'				{$value = new String("Should never happend(");need_space=false;}
	;

accMod :	'const';

memMod 
	:	pointer
	|	reference
	;
	
pointer :	'*';

reference :	 '&';

value returns [String text]
	:	ID		{$text = $ID.text;}
	|	NUMBER	{$text = $NUMBER.text;}
	;
	 
ID 	:	('a'..'z'|'A'..'Z' |'_' )('a'..'z'|'A'..'Z'|'_'|'0'..'9')* ;

NUMBER :	('0'..'9')*;
WS      : (' '|'\t'|'\n')+ {channel=99;};

COMMENT
    :   '/*' ( options {greedy=false;} : . )* '*/' {channel=99;}
    ;

LINE_COMMENT
    : '//' ~('\n'|'\r')* '\r'? '\n' {channel=99;}
    ;
    
