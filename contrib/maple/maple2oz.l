/*
 *  Authors:
 *    Jürgen Zimmer (jzimmer@ps.uni-sb.de)
 *    Martin Pollet (pollet@ags.uni-sb.de)
 *
 *  Contributors:
 *    Tobias Müller (tmueller@ps.uni-sb.de)
 *
 *  Copyright:
 *    1999
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

%{
#define YYSTYPE char*
#include "maple2oz.tab.h"
#undef YY_INPUT
#define YY_INPUT(b,r,ms) (r=my_yyinput(b, ms))
extern int myinputlength;
extern char myinput[64000];
extern char *myinputptr;
%}

%option noyywrap

IDENT   [a-zA-Z_][a-zA-Z0-9_]*
NUMBER  [0-9]*[.]*[0-9]+

%%

abs return ABS;
{IDENT}   { yylval = yytext; return TID; }
{NUMBER}  { yylval = yytext; return NUMBER; }
[ \n\t]+
[-+*/^(){};] { yylval = yytext; return yytext[0]; }

%%


int my_yyinput(char *buf, int max_size)
{
  int n;
  if (max_size>myinputlength)
    n=myinputlength;
  else
    n=max_size;

  if (n>0)
    {
      memcpy(buf, myinputptr, n);
      myinputptr += n;
    }
  return n;
}






