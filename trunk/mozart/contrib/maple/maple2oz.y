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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define YYSTYPE char*
char buffer[64000];
char myinput[64000];
char *myinputptr;
int myinputlength;
FILE * OUTFILE;
%}




%token TID NUMBER '(' ')' '{' '}' ',' ';' ABS
%left '+' '-'
%left '*' '/'
%nonassoc '^'

%%

input: expr { fprintf(OUTFILE, "\"%s\"\n", $1); return 0; }
;


expr: TID       { sprintf(buffer, "'%s'",  $1);
		  $$ = strdup(buffer); }
| ABS '(' expr ')'  { sprintf(buffer, "'abs'(%s)",  $3);
		  $$ = strdup(buffer); }
| expr '+' expr { sprintf(buffer, "'+'(%s %s)", $1, $3);
		  $$ = strdup(buffer); }
| expr '-' expr { sprintf(buffer, "'-'(%s %s)", $1, $3);
		  $$ = strdup(buffer); }
| expr '*' expr { sprintf(buffer, "'*'(%s %s)", $1, $3);
		  $$ = strdup(buffer); }
| expr '/' expr { sprintf(buffer, "'/'(%s %s)", $1, $3);
		  $$ = strdup(buffer); }
| expr '^' expr { sprintf(buffer, "'^'(%s %s)", $1, $3);
		  $$ = strdup(buffer); }
| '-' NUMBER    { sprintf(buffer, "~0%s", $2);
		  $$ = strdup(buffer); }
| NUMBER        { sprintf(buffer, "0%s", $1);
		  $$ = strdup(buffer); }
| '-' expr      { sprintf(buffer, "'-' (0 %s)", $2);
		  $$ = strdup(buffer); }
| '(' expr ')' { $$ = $2; }
;

%%

FILE * INPUTFILE; 

void divdiv2divmult (void)

/* Somebody who knows more about C should do this better.       */
/* This procedure transforms Maple-ouput like a/b/c to a/(b*c), */
/* and stores it in myinput[].                                  */
/* It should handle leading terms like 1/2 * a/b correctly.     */
/* The result is given to the parser.                           */
/* Added the handling of quotations: "..." -> ...;              */

{
  int end=0;
  int enddiv=0;
  int change=0;
  char inchar;
  int counter=0;
  int bracket;
  int countquote=0;
  while((inchar = getc(INPUTFILE)) != EOF) {
    switch(inchar)
	{
	case '/':                      //go into '/'-modus
	  enddiv=0;
	  myinput[counter]=inchar;
	  counter++;
	  myinput[counter]='(';
	  counter++;
	  do
	    {
	      scanf("%c", &inchar);         //read the next items from here
	      switch(inchar)
		{
		case '/':                   //transform /a/.. to /(a*..
		  myinput[counter]='*';
		  counter++;
		  break;
		case '(':                   //don't touch things in brackets, just copy
		  myinput[counter]=inchar;
		  counter++;
		  bracket=1;
		  do{
		    scanf("%c", &inchar);
		    myinput[counter]=inchar;
		    counter++;
		    if (inchar=')') bracket--;
		    else if (inchar='(') bracket++;
		  }
		  while(bracket!=0);
		  break;
		case '-':                   //aha, get out of the the '/'-modus
		  myinput[counter]=')';
		  counter++;
		  myinput[counter]=inchar;
		  counter++;
		  enddiv=1;
		  break;
		case '+':                   //same
		  myinput[counter]=')';
		  counter++;
		  myinput[counter]=inchar;
		  counter++;
		  enddiv=1;
		  break;
		case '*':                   //same
		  myinput[counter]=')';
		  counter++;
		  myinput[counter]=inchar;
		  counter++;
		  enddiv=1;
		  break;
		case '"':                   //the end, close everthing and go out of here
		  myinput[counter]=')';
		  counter++;
		  myinput[counter]=';';
		  counter++;
		  end=1;
		  enddiv=1;
		  break;
		case ';':                   //the end, close everthing and go out of here
		  myinput[counter]=')';
		  counter++;
		  myinput[counter]=inchar;
		  counter++;
		  end=1;
		  enddiv=1;
		  break;
		default:                    //just copy everything else
		  myinput[counter]=inchar;
		  counter++;
		  break;
		}
	    }
	  while(enddiv==0);
	  break;
	case '"':                     // ignore the first '"', replace the second '"' with ';' and exit
	  if (countquote==0) 
	    countquote++;
	  else
	    {
	      myinput[counter]=';';
	      counter++;
	      end=1;
	    }
	  break;
	case ';':                    //bye
	  myinput[counter]=inchar;
	  counter++;
	  end=1;
	  break;
	default:                     //everything else
	  myinput[counter]=inchar;
	  counter++;
	  break;
	}
    if (end!=0) break;
  }

  
  myinputlength=counter;                           //var for lex
  myinputptr=malloc(myinputlength*sizeof(char));
  strcpy(myinputptr,myinput);                      //pointer to the formula for lex
 }


void maple2oz (FILE * in, FILE * out)
{

  OUTFILE = out; 
  INPUTFILE = in;

  divdiv2divmult();
  yyparse ();
}

#include <stdio.h>

yyerror (s)  /* Called by yyparse on error */
     char *s;
{
   fprintf (OUTFILE, "Error: %s\n", s);
}

