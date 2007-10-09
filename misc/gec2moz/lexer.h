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

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <ctype.h>
#include <vector>
#include <iostream>

using namespace std;

extern FILE *yyin;
extern char *yytext;



struct spar{
  char *tvar;
  char *var;
  char *vdef;
};

typedef spar * sp;

struct ppg{
  char *nspace;
  char *nppg;
  vector<sp> *parlist;
  int n_param;
};

typedef ppg * pg;


typedef struct {
  char vlex[50];
  pg Prop;
  sp Par;
  int space;
  //string nprop;
} st;


extern int yylex();
extern int yyparse();
