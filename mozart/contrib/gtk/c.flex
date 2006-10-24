/*
 * Author:
 *   Thorsten Brunklaus <bruni@ps.uni-sb.de>
 *
 * Copyright:
 *   Thorsten Brunklaus, 2001
 *
 * Last Change:
 *   $Date$ by $Author$
 *   $Revision$
 *
 * This file is part of Mozart, an implementation of Oz 3:
 *   http://www.mozart-oz.org
 *
 * See the file "LICENSE" or
 *   http://www.mozart-oz.org/LICENSE.html
 * for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "c.bison.tab.hh"

extern char *yytext;

OZ_Term newToken() {
  return OZ_atom(yytext);
}

extern int lookupType(OZ_Term id);
extern int line_num;

int check_type();
%}

%%
"/*"			{ /* Skip comment handling */ }

"auto"			{yylval = newToken(); return(AUTO); }
"break"			{yylval = newToken(); return(BREAK); }
"case"			{yylval = newToken(); return(CASE); }
"char"			{yylval = newToken(); return(CHAR); }
"const"			{yylval = newToken(); return(CONST); }
"continue"		{yylval = newToken(); return(CONTINUE); }
"default"		{yylval = newToken(); return(DEFAULT); }
"do"			{yylval = newToken(); return(DO); }
"double"		{yylval = newToken(); return(DOUBLE); }
"else"			{yylval = newToken(); return(ELSE); }
"enum"			{yylval = newToken(); return(ENUM); }
"extern"		{yylval = newToken(); return(EXTERN); }
"float"			{yylval = newToken(); return(FLOAT); }
"for"			{yylval = newToken(); return(FOR); }
"goto"			{yylval = newToken(); return(GOTO); }
"if"			{yylval = newToken(); return(IF); }
"inline"		{yylval = newToken(); return(INLINE); }
"int"			{yylval = newToken(); return(INT); }
"long"			{yylval = newToken(); return(LONG); }
"restrict"		{yylval = newToken(); return(RESTRICT); }
"register"		{yylval = newToken(); return(REGISTER); }
"return"		{yylval = newToken(); return(RETURN); }
"short"			{yylval = newToken(); return(SHORT); }
"signed"		{yylval = newToken(); return(SIGNED); }
"sizeof"		{yylval = newToken(); return(SIZEOF); }
"static"		{yylval = newToken(); return(STATIC); }
"struct"		{yylval = newToken(); return(STRUCT); }
"switch"		{yylval = newToken(); return(SWITCH); }
"typedef"		{yylval = newToken(); return(TYPEDEF); }
"union"			{yylval = newToken(); return(UNION); }
"unsigned"		{yylval = newToken(); return(UNSIGNED); }
"void"			{yylval = newToken(); return(VOID); }
"volatile"		{yylval = newToken(); return(VOLATILE); }
"while"			{yylval = newToken(); return(WHILE); }
"..."			{yylval = newToken(); return(ELIPSIS); }

{L}({L}|{D})*		{ return(check_type()); }

0[xX]{H}+{IS}?		{yylval = newToken(); return(CONSTANT); }
0{D}+{IS}?		{yylval = newToken(); return(CONSTANT); }
{D}+{IS}?		{yylval = newToken(); return(CONSTANT); }
'(\\.|[^\\'])+'		{yylval = newToken(); return(CONSTANT); }

{D}+{E}{FS}?		{yylval = newToken(); return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{yylval = newToken(); return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{yylval = newToken(); return(CONSTANT); }

\"(\\.|[^\\"])*\"	{yylval = newToken(); return(STRING_LITERAL); }

">>="			{yylval = newToken(); return(RIGHT_ASSIGN); }
"<<="			{yylval = newToken(); return(LEFT_ASSIGN); }
"+="			{yylval = newToken(); return(ADD_ASSIGN); }
"-="			{yylval = newToken(); return(SUB_ASSIGN); }
"*="			{yylval = newToken(); return(MUL_ASSIGN); }
"/="			{yylval = newToken(); return(DIV_ASSIGN); }
"%="			{yylval = newToken(); return(MOD_ASSIGN); }
"&="			{yylval = newToken(); return(AND_ASSIGN); }
"^="			{yylval = newToken(); return(XOR_ASSIGN); }
"|="			{yylval = newToken(); return(OR_ASSIGN); }
">>"			{yylval = newToken(); return(RIGHT_OP); }
"<<"			{yylval = newToken(); return(LEFT_OP); }
"++"			{yylval = newToken(); return(INC_OP); }
"--"			{yylval = newToken(); return(DEC_OP); }
"->"			{yylval = newToken(); return(PTR_OP); }
"&&"			{yylval = newToken(); return(AND_OP); }
"||"			{yylval = newToken(); return(OR_OP); }
"<="			{yylval = newToken(); return(LE_OP); }
">="			{yylval = newToken(); return(GE_OP); }
"=="			{yylval = newToken(); return(EQ_OP); }
"!="			{yylval = newToken(); return(NE_OP); }
";"			{yylval = newToken(); return(SEPARATOR); }
"{"			{yylval = newToken(); return(OBRACE); }
"}"			{yylval = newToken(); return(CBRACE); }
","			{yylval = newToken(); return(COMMA); }
":"			{yylval = newToken(); return(COLON); }
"="			{yylval = newToken(); return(EQUAL_OP); }
"("			{yylval = newToken(); return(OPARENT); }
")"			{yylval = newToken(); return(CPARENT); }
"["			{yylval = newToken(); return(OBRACKET); }
"]"			{yylval = newToken(); return(CBRACKET); }
"."			{yylval = newToken(); return(DOT); }
"&"			{yylval = newToken(); return(ADDR_OP); }
"!"			{yylval = newToken(); return(DEREF_OP); }
"~"			{yylval = newToken(); return(TILDE); }
"-"			{yylval = newToken(); return(SUB_OP); }
"+"			{yylval = newToken(); return(PLUS_OP); }
"*"			{yylval = newToken(); return(MUL_OP); }
"/"			{yylval = newToken(); return(DIV_OP); }
"%"			{yylval = newToken(); return(MOD_OP); }
"<"			{yylval = newToken(); return(LESS_OP); }
">"			{yylval = newToken(); return(GREATER_OP); }
"^"			{yylval = newToken(); return(ROOF); }
"|"			{yylval = newToken(); return(PIPE); }
"?"			{yylval = newToken(); return(QUESTIONMARK); }

[\n]			{ line_num++; }
[ \t\v\f]		{  }
.			{ /* ignore bad characters */ }

%%

int yywrap()
{
	return(1);
}

int check_type()
{
	yylval = newToken();
#if defined(DEBUG)
/*	printf("looking up: %s\n", yytext); */
#endif
	return (lookupType(yylval) ? TYPE_NAME : IDENTIFIER);
}
