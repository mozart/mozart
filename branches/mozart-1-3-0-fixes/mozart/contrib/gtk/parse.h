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

#ifndef __PARSE_H__
#define __PARSE_H__

#include <stdio.h>
#include <mozart.h>

#define YYSTYPE OZ_Term

extern FILE *yyin;
extern int yylex(void);
extern int yyparse(void);
extern void yyerror(char *s);

#endif
