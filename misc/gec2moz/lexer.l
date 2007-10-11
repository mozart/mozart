/*
 *  Main authors:
 *     Diana Lorena Velasco <dlvelasco@puj.edu.co>
 *     Juan Gabriel Torres  <juantorres@puj.edu.co>
 *
 *  Contributing authors:
 *     
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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

%option noyywrap
%{
	#include "lexer.h" 
	#include "parser.h"
	#include <string.h>
	#include <vector>	
	
	int nro_lineas = 1;
	
%}

%x comment

%%



void		{return(TVOID);}
const		{return(TCONST);}
";"			{return(TPC);}
","			{return(TCOMA);}
"."			{return(TPTO);}
":"			{return(T2PTO);}
"("			{return(TIZQPAR);}
")"			{return(TDERPAR);}
"["			{return(TIZQCOR);}
"]"			{return(TDERCOR);}
"{"			{return(TIZQLLA);}
"}"			{return(TDERLLA);}
"*"			{return(TAST);}
"="			{return(TIGUAL);}
"&"			{return(TAMP);}

"//"[ a-zA-Z][a-zA-Z *]*	{}

"/*{"						{return(TIZQCM);}

"}*/"						{return(TDERCM);}

"<<"						{return(TIZQDEC);}

">>"						{return(TDERDEC);}

([a-zA-Z]+"::")+  			{strcpy(yylval.vlex,yytext);return(TNSP);}

[0-9]+ 		{return(TINTEGER);}

[a-zA-Z'_'][a-zA-Z'_'0-9]*	{strcpy(yylval.vlex,yytext);return(TID);}


"/*"         BEGIN(comment);

<comment>[^*\n]*        /* eat anything that's not a '*' */
<comment>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
<comment>\n             ++nro_lineas;
<comment>"*"+"/"        BEGIN(INITIAL);



[ \t]+				/* obvia espacios en blanco */
[\n(\r\n)]				{nro_lineas++;}
.					{printf(" Linea %d : Caracter desconocido  %s\n", nro_lineas, yytext);}

%%
