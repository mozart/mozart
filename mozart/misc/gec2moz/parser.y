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

// Some auxiliar functions
%{                    
  #include "lexer.h"
  #include <string>
  #include <stdio.h>
  #include <stdlib.h>
  #include <vector> 
  #include <map> 
  
  int yyerror(char *s);
  extern int nro_lineas;
  vector<pg> C;
  map<string,string> Check;
  map<string,string> Initials;
  map<string,string> Dec;
  map<string,string> Space;
  
  
  int yyerror(char *s){
     printf ("\nLinea: %d: sintactic error: %s\n", nro_lineas, s);
     return 1;
  }
  	  
   
%}

// Tokens definition
%union{
       char vlex[50];
       st Code;
	   
}
%token	<vlex>TINTEGER
%token	TFLOAT	
%token  <vlex>TID     
%token  TAST     
%token	TIZQPAR
%token	TDERPAR	
%token	TIZQCOR
%token	TDERCOR	
%token	TIZQLLA
%token	TDERLLA
%token	TPTO
%token	T2PTO
%token	TCOMA
%token	TPC
%token  TINT
%token  TFLO
%token  TVOID
%token  TINTVAR
%token  TINTVARARRAY
%token  TRELTYPE
%token  TCONLEVEL
%token  TSPACE
%token  TINTARRAY
%token  TGECODE
%token  T4PTO
%token  TCOMMENT
%token  TIGUAL
%token  TAMP
%token  TCONST
%token  <vlex>TNSP
%token  TIZQCM
%token  TDERCM
%token  TIZQDEC
%token  TDERDEC

%type<Code> FUNCTION_LIST FUNCTION_DEC FUN_TYPE ARG_LIST EXP N_SPACE BODYCM BODYDEC SP COMMENT
%%
/***************************************************************************/


CODE:	COMMENT FUNCTION_LIST					{}

COMMENT:	TIZQCM BODYCM BODYDEC TDERCM 		{
													
												}
			
BODYCM:		TID T2PTO TID T2PTO TID TPC					{
															
															Check[$1] = $3;
															Initials[$1] = $5;
														}
			|BODYCM TID T2PTO TID T2PTO TID TPC			{
															
															Check[$2] = $4;
															Initials[$2] = $6;
														}
BODYDEC:	TID TIZQDEC TID TDERDEC SP TPC				{
															Dec[$1] = $3;
															if($5.space == 1){
																Space[$1] = "true";
															}
															else{
																Space[$1] = "false";
															}
														}
			
			|BODYDEC TID TIZQDEC TID TDERDEC SP TPC		{
															Dec[$2] = $4;
															if($6.space == 1){
																Space[$2] = "true";
															}
															else{
																Space[$2] = "false";
															}
														
														}

SP: TID		{
				$$.space = 1;
			}
	|		{
				$$.space = 0;
			}

FUNCTION_LIST: FUNCTION_DEC                   	{}                               
			   | FUNCTION_LIST FUNCTION_DEC   	{}
			  
FUNCTION_DEC:  FUN_TYPE N_SPACE TIZQPAR ARG_LIST TDERPAR TPC 	{	
																	pg tmp = (pg)malloc(sizeof(struct ppg));
																	tmp->nppg = (char*)malloc(sizeof(char)*strlen($2.Prop->nppg));
																	tmp->nspace = (char*)malloc(sizeof(char)*strlen($2.Prop->nspace));
																	strcpy(tmp->nppg,$2.Prop->nppg);
																	strcpy(tmp->nspace,$2.Prop->nspace);
																	tmp->parlist = $4.Prop->parlist;
																	tmp->n_param = $4.Prop->parlist->size();
																	C.push_back(tmp);
																}
															
FUN_TYPE: TVOID                    				{}
		  
N_SPACE: TNSP TID						{
											
											$$.Prop = (pg)malloc(sizeof(struct ppg));
											$$.Prop->nspace = (char*)malloc(sizeof(char)*strlen($1));
											$$.Prop->nppg = (char*)malloc(sizeof(char)*strlen($2));
											strcpy($$.Prop->nspace,$1);
											strcpy($$.Prop->nppg,$2);
										}
		 

ARG_LIST: TYPE_ID TID EXP               		{
													$$.Prop = (pg)malloc(sizeof(struct ppg));
													$$.Prop->parlist = new vector<sp>();
													$3.Par->tvar = (char*)malloc(sizeof(char)*strlen($2));
													strcpy($3.Par->tvar,$2);
													$$.Prop->parlist->push_back($3.Par);
																										
												}
		  |ARG_LIST TCOMA TYPE_ID TID EXP     	{
		  											$5.Par->tvar = (char*)malloc(sizeof(char)*strlen($4));
													strcpy($5.Par->tvar,$4);
													$1.Prop->parlist->push_back($5.Par);
													$$.Prop = $1.Prop;
													
		  										}
		  
TYPE_ID: |TCONST						{}
		  
EXP:	TID								{
											$$.Par = (sp)malloc(sizeof(struct spar));
											$$.Par->var = (char*)malloc(sizeof(char)*strlen($1));
											strcpy($$.Par->var,$1);
										}
		|TAST TID						{
											$$.Par = (sp)malloc(sizeof(struct spar));
											$$.Par->var = (char*)malloc(sizeof(char)*strlen($2));
											strcpy($$.Par->var,$2);
										}
		|TAMP TID						{
											$$.Par = (sp)malloc(sizeof(struct spar));
											$$.Par->var = (char*)malloc(sizeof(char)*strlen($2));
											strcpy($$.Par->var,$2);
										}
		|TIGUAL TID						{
											$$.Par = (sp)malloc(sizeof(struct spar));
											$$.Par->vdef = (char*)malloc(sizeof(char)*strlen($2));
											strcpy($$.Par->vdef,$2);
										}
		|TID TIGUAL TID					{
											$$.Par = (sp)malloc(sizeof(struct spar));
											$$.Par->var = (char*)malloc(sizeof(char)*strlen($1));
											$$.Par->vdef = (char*)malloc(sizeof(char)*strlen($3));
											strcpy($$.Par->var,$1);
											strcpy($$.Par->vdef,$3);
										}
		|TID TIGUAL TINTEGER			{
											$$.Par = (sp)malloc(sizeof(struct spar));
											$$.Par->var = (char*)malloc(sizeof(char)*strlen($1));
											$$.Par->vdef = (char*)malloc(sizeof(char)*strlen($3));
											strcpy($$.Par->var,$1);
											strcpy($$.Par->vdef,$3);
										}
		|TAST TID TIGUAL TID			{
											$$.Par = (sp)malloc(sizeof(struct spar));
											$$.Par->var = (char*)malloc(sizeof(char)*strlen($2));
											$$.Par->vdef = (char*)malloc(sizeof(char)*strlen($4));
											strcpy($$.Par->var,$2);
											strcpy($$.Par->vdef,$4);
										}
		|TAST TID TIGUAL TINTEGER		{
											$$.Par = (sp)malloc(sizeof(struct spar));
											$$.Par->var = (char*)malloc(sizeof(char)*strlen($2));
											$$.Par->vdef = (char*)malloc(sizeof(char)*strlen($4));
											strcpy($$.Par->var,$2);
											strcpy($$.Par->vdef,$4);
										}	
		  

		  
		  

        
/***************************************************************************/
%%
