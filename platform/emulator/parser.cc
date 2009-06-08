/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse xyparse
#define yylex   xylex
#define yyerror xyerror
#define yylval  xylval
#define yychar  xychar
#define yydebug xydebug
#define yynerrs xynerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_SWITCH = 258,
     T_SWITCHNAME = 259,
     T_LOCALSWITCHES = 260,
     T_PUSHSWITCHES = 261,
     T_POPSWITCHES = 262,
     T_OZATOM = 263,
     T_ATOM_LABEL = 264,
     T_OZFLOAT = 265,
     T_OZINT = 266,
     T_AMPER = 267,
     T_DOTINT = 268,
     T_STRING = 269,
     T_VARIABLE = 270,
     T_VARIABLE_LABEL = 271,
     T_DEFAULT = 272,
     T_CHOICE = 273,
     T_LDOTS = 274,
     T_2DOTS = 275,
     T_attr = 276,
     T_at = 277,
     T_case = 278,
     T_catch = 279,
     T_choice = 280,
     T_class = 281,
     T_cond = 282,
     T_declare = 283,
     T_define = 284,
     T_dis = 285,
     T_else = 286,
     T_elsecase = 287,
     T_elseif = 288,
     T_elseof = 289,
     T_end = 290,
     T_export = 291,
     T_fail = 292,
     T_false = 293,
     T_FALSE_LABEL = 294,
     T_feat = 295,
     T_finally = 296,
     T_from = 297,
     T_fun = 298,
     T_functor = 299,
     T_if = 300,
     T_import = 301,
     T_in = 302,
     T_local = 303,
     T_lock = 304,
     T_meth = 305,
     T_not = 306,
     T_of = 307,
     T_or = 308,
     T_prepare = 309,
     T_proc = 310,
     T_prop = 311,
     T_raise = 312,
     T_require = 313,
     T_self = 314,
     T_skip = 315,
     T_then = 316,
     T_thread = 317,
     T_true = 318,
     T_TRUE_LABEL = 319,
     T_try = 320,
     T_unit = 321,
     T_UNIT_LABEL = 322,
     T_while = 323,
     T_FOR = 324,
     T_do = 325,
     T_ENDOFFILE = 326,
     T_REGEX = 327,
     T_lex = 328,
     T_mode = 329,
     T_parser = 330,
     T_prod = 331,
     T_scanner = 332,
     T_syn = 333,
     T_token = 334,
     T_REDUCE = 335,
     T_SEP = 336,
     T_ITER = 337,
     T_OOASSIGN = 338,
     T_DOTASSIGN = 339,
     T_COLONEQUALS = 340,
     T_orelse = 341,
     T_andthen = 342,
     T_RMACRO = 343,
     T_LMACRO = 344,
     T_FDCOMPARE = 345,
     T_COMPARE = 346,
     T_FDIN = 347,
     T_ADD = 348,
     T_OTHERMUL = 349,
     T_FDMUL = 350,
     T_DEREFF = 351
   };
#endif
/* Tokens.  */
#define T_SWITCH 258
#define T_SWITCHNAME 259
#define T_LOCALSWITCHES 260
#define T_PUSHSWITCHES 261
#define T_POPSWITCHES 262
#define T_OZATOM 263
#define T_ATOM_LABEL 264
#define T_OZFLOAT 265
#define T_OZINT 266
#define T_AMPER 267
#define T_DOTINT 268
#define T_STRING 269
#define T_VARIABLE 270
#define T_VARIABLE_LABEL 271
#define T_DEFAULT 272
#define T_CHOICE 273
#define T_LDOTS 274
#define T_2DOTS 275
#define T_attr 276
#define T_at 277
#define T_case 278
#define T_catch 279
#define T_choice 280
#define T_class 281
#define T_cond 282
#define T_declare 283
#define T_define 284
#define T_dis 285
#define T_else 286
#define T_elsecase 287
#define T_elseif 288
#define T_elseof 289
#define T_end 290
#define T_export 291
#define T_fail 292
#define T_false 293
#define T_FALSE_LABEL 294
#define T_feat 295
#define T_finally 296
#define T_from 297
#define T_fun 298
#define T_functor 299
#define T_if 300
#define T_import 301
#define T_in 302
#define T_local 303
#define T_lock 304
#define T_meth 305
#define T_not 306
#define T_of 307
#define T_or 308
#define T_prepare 309
#define T_proc 310
#define T_prop 311
#define T_raise 312
#define T_require 313
#define T_self 314
#define T_skip 315
#define T_then 316
#define T_thread 317
#define T_true 318
#define T_TRUE_LABEL 319
#define T_try 320
#define T_unit 321
#define T_UNIT_LABEL 322
#define T_while 323
#define T_FOR 324
#define T_do 325
#define T_ENDOFFILE 326
#define T_REGEX 327
#define T_lex 328
#define T_mode 329
#define T_parser 330
#define T_prod 331
#define T_scanner 332
#define T_syn 333
#define T_token 334
#define T_REDUCE 335
#define T_SEP 336
#define T_ITER 337
#define T_OOASSIGN 338
#define T_DOTASSIGN 339
#define T_COLONEQUALS 340
#define T_orelse 341
#define T_andthen 342
#define T_RMACRO 343
#define T_LMACRO 344
#define T_FDCOMPARE 345
#define T_COMPARE 346
#define T_FDIN 347
#define T_ADD 348
#define T_OTHERMUL 349
#define T_FDMUL 350
#define T_DEREFF 351




/* Copy the first part of user declarations.  */


//
// See Oz/tools/compiler/Doc/TupleSyntax for an description of the
// generated parse trees.
//

// 
// To support '. :=' as a multifix operator (see share/lib/compiler/Doc/UniformStateAccess) 
// this parser distinguishes e1.e2 := e3 from (e1.e2) := e3.  The only way
// I could find to make this work (without rewriting the parser) was to 
// keep parens in the parse tree (see rule  '(' inSequence ')' in phrase2).
// It is then the responsibility of the parent parse to remove the extra
// Paren nodes.  In most cases this is taken care of automatically by the 
// newCTerm functions,  but sometimes this has to be done by special versions
// of oz_cons, oz_mklist etc.  
// When adding/modifying parse rules,  be sure that sub-parses have any 
// outer paren node removed (with function unwrap).


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#include <malloc.h>
#include <io.h>
#else
#include <sys/time.h>
#endif

#include "base.hh"
#include "value.hh"


//----------------------
// Interface to Scanner
//----------------------

extern char xyFileName[];   // name of the current file, "" means stdin
extern char xyhelpFileName[];
extern OZ_Term xyFileNameAtom;

extern int xy_gumpSyntax, xy_allowDeprecated, xy_allowWhileLoop;
extern OZ_Term xy_errorMessages;

extern int xylino;
extern char *xytext;
extern char *xylastline;

char *xy_expand_file_name(char *file);

int xy_init_from_file(char *file, OZ_Term defines);
void xy_init_from_string(char *str, OZ_Term defines);
void xy_exit();

int xylex();

inline 
int xycharno(void) {
  int n = xytext - xylastline;
  if (n > 0)
    return n;
  else
    return 0;
}

void checkDeprecation(OZ_Term coord);
void xyreportWarning(const char *kind, const char *msg, OZ_Term coord);
void xyreportError(const char *kind, const char *msg, OZ_Term coord);
void xyreportError(const char *kind, const char *msg,
		   const char *file, int line, int column);


//-----------------
// Local Variables
//-----------------

#define YYMAXDEPTH 1000000
#define YYERROR_VERBOSE

static OZ_Term yyoutput;

static void xyerror(const char *);

//-----------------
// Atom definitions
//-----------------

#define PA_MAX_ATOM 112

OZ_Term _PA_AtomTab[PA_MAX_ATOM+1];

#define PA_allowdeprecated			_PA_AtomTab[0]
#define PA_coord				_PA_AtomTab[1]
#define PA_defines				_PA_AtomTab[2]
#define PA_deprecation_error			_PA_AtomTab[3]
#define PA_deprecation_warning			_PA_AtomTab[4]
#define PA_dirLocalSwitches			_PA_AtomTab[5]
#define PA_dirPopSwitches			_PA_AtomTab[6]
#define PA_dirPushSwitches			_PA_AtomTab[7]
#define PA_dirSwitch				_PA_AtomTab[8]
#define PA_error				_PA_AtomTab[9]
#define PA_fAnd					_PA_AtomTab[10]
#define PA_fAndThen				_PA_AtomTab[11]
#define PA_fApply				_PA_AtomTab[12]
#define PA_fAssign				_PA_AtomTab[13]
#define PA_fAt					_PA_AtomTab[14]
#define PA_fAtom				_PA_AtomTab[15]
#define PA_fAttr				_PA_AtomTab[16]
#define PA_fBoolCase				_PA_AtomTab[17]
#define PA_fCase				_PA_AtomTab[18]
#define PA_fCaseClause				_PA_AtomTab[19]
#define PA_fCatch				_PA_AtomTab[20]
#define PA_fChoice				_PA_AtomTab[21]
#define PA_fClass				_PA_AtomTab[22]
#define PA_fClause				_PA_AtomTab[23]
#define PA_fColon				_PA_AtomTab[24]
#define PA_fCond				_PA_AtomTab[25]
#define PA_fDeclare				_PA_AtomTab[26]
#define PA_fDefault				_PA_AtomTab[27]
#define PA_fDefine				_PA_AtomTab[28]
#define PA_fDis					_PA_AtomTab[29]
#define PA_fDollar				_PA_AtomTab[30]
#define PA_fEq					_PA_AtomTab[31]
#define PA_fEscape				_PA_AtomTab[32]
#define PA_fExport				_PA_AtomTab[33]
#define PA_fExportItem				_PA_AtomTab[34]
#define PA_fFail				_PA_AtomTab[35]
#define PA_fFdCompare				_PA_AtomTab[36]
#define PA_fFdIn				_PA_AtomTab[37]
#define PA_fFeat				_PA_AtomTab[38]
#define PA_fFloat				_PA_AtomTab[39]
#define PA_fFrom				_PA_AtomTab[40]
#define PA_fFun					_PA_AtomTab[41]
#define PA_fFunctor				_PA_AtomTab[42]
#define PA_fImport				_PA_AtomTab[43]
#define PA_fImportAt				_PA_AtomTab[44]
#define PA_fImportItem				_PA_AtomTab[45]
#define PA_fInheritedModes			_PA_AtomTab[46]
#define PA_fInt					_PA_AtomTab[47]
#define PA_fLexicalAbbreviation			_PA_AtomTab[48]
#define PA_fLexicalRule				_PA_AtomTab[49]
#define PA_fLocal				_PA_AtomTab[50]
#define PA_fLock				_PA_AtomTab[51]
#define PA_fLockThen				_PA_AtomTab[52]
#define PA_fMeth				_PA_AtomTab[53]
#define PA_fMethArg				_PA_AtomTab[54]
#define PA_fMethColonArg			_PA_AtomTab[55]
#define PA_fMode				_PA_AtomTab[56]
#define PA_fNoCatch				_PA_AtomTab[57]
#define PA_fNoDefault				_PA_AtomTab[58]
#define PA_fNoElse				_PA_AtomTab[59]
#define PA_fNoFinally				_PA_AtomTab[60]
#define PA_fNoImportAt				_PA_AtomTab[61]
#define PA_fNoThen				_PA_AtomTab[62]
#define PA_fNot					_PA_AtomTab[63]
#define PA_fObjApply				_PA_AtomTab[64]
#define PA_fOpApply				_PA_AtomTab[65]
#define PA_fOpenRecord				_PA_AtomTab[66]
#define PA_fOr					_PA_AtomTab[67]
#define PA_fOrElse				_PA_AtomTab[68]
#define PA_fParser				_PA_AtomTab[69]
#define PA_fPrepare				_PA_AtomTab[70]
#define PA_fProc				_PA_AtomTab[71]
#define PA_fProductionTemplate			_PA_AtomTab[72]
#define PA_fProp				_PA_AtomTab[73]
#define PA_fRaise				_PA_AtomTab[74]
#define PA_fRecord				_PA_AtomTab[75]
#define PA_fRequire				_PA_AtomTab[76]
#define PA_fScanner				_PA_AtomTab[77]
#define PA_fSelf				_PA_AtomTab[78]
#define PA_fSideCondition			_PA_AtomTab[79]
#define PA_fSkip				_PA_AtomTab[80]
#define PA_fSynAction				_PA_AtomTab[81]
#define PA_fSynAlternative			_PA_AtomTab[82]
#define PA_fSynApplication			_PA_AtomTab[83]
#define PA_fSynAssignment			_PA_AtomTab[84]
#define PA_fSynSequence				_PA_AtomTab[85]
#define PA_fSynTemplateInstantiation		_PA_AtomTab[86]
#define PA_fSynTopLevelProductionTemplates	_PA_AtomTab[87]
#define PA_fSyntaxRule				_PA_AtomTab[88]
#define PA_fThread				_PA_AtomTab[89]
#define PA_fToken				_PA_AtomTab[90]
#define PA_fTry					_PA_AtomTab[91]
#define PA_fVar					_PA_AtomTab[92]
#define PA_fWildcard				_PA_AtomTab[93]
#define PA_fileNotFound				_PA_AtomTab[94]
#define PA_gump					_PA_AtomTab[95]
#define PA_kind					_PA_AtomTab[96]
#define PA_msg					_PA_AtomTab[97]
#define PA_none					_PA_AtomTab[98]
#define PA_off					_PA_AtomTab[99]
#define PA_on					_PA_AtomTab[100]
#define PA_parse				_PA_AtomTab[101]
#define PA_parseError				_PA_AtomTab[102]
#define PA_pos					_PA_AtomTab[103]
#define PA_warn					_PA_AtomTab[104]
#define PA_zy					_PA_AtomTab[105]
#define PA_fWhile				_PA_AtomTab[106]
#define PA_fMacro				_PA_AtomTab[107]
#define PA_fDotAssign				_PA_AtomTab[108]
#define PA_fFOR					_PA_AtomTab[109]
#define PA_fColonEquals				_PA_AtomTab[110]
#define PA_parens				_PA_AtomTab[111]
#define PA_allowwhileloop			_PA_AtomTab[112]

const char * _PA_CharTab[] = {
	"allowdeprecated",			//0
	"coord",				//1
	"defines",				//2
	"deprecation error",			//3
	"deprecation warning",			//4
	"dirLocalSwitches",			//5
	"dirPopSwitches",			//6
	"dirPushSwitches",			//7
	"dirSwitch",				//8
	"error",				//9
	"fAnd",					//10
	"fAndThen",				//11
	"fApply",				//12
	"fAssign",				//13
	"fAt",					//14
	"fAtom",				//15
	"fAttr",				//16
	"fBoolCase",				//17
	"fCase",				//18
	"fCaseClause",				//19
	"fCatch",				//20
	"fChoice",				//21
	"fClass",				//22
	"fClause",				//23
	"fColon",				//24
	"fCond",				//25
	"fDeclare",				//26
	"fDefault",				//27
	"fDefine",				//28
	"fDis",					//29
	"fDollar",				//30
	"fEq",					//31
	"fEscape",				//32
	"fExport",				//33
	"fExportItem",				//34
	"fFail",				//35
	"fFdCompare",				//36
	"fFdIn",				//37
	"fFeat",				//38
	"fFloat",				//39
	"fFrom",				//40
	"fFun",					//41
	"fFunctor",				//42
	"fImport",				//43
	"fImportAt",				//44
	"fImportItem",				//45
	"fInheritedModes",			//46
	"fInt",					//47
	"fLexicalAbbreviation",			//48
	"fLexicalRule",				//49
	"fLocal",				//50
	"fLock",				//51
	"fLockThen",				//52
	"fMeth",				//53
	"fMethArg",				//54
	"fMethColonArg",			//55
	"fMode",				//56
	"fNoCatch",				//57
	"fNoDefault",				//58
	"fNoElse",				//59
	"fNoFinally",				//60
	"fNoImportAt",				//61
	"fNoThen",				//62
	"fNot",					//63
	"fObjApply",				//64
	"fOpApply",				//65
	"fOpenRecord",				//66
	"fOr",					//67
	"fOrElse",				//68
	"fParser",				//69
	"fPrepare",				//70
	"fProc",				//71
	"fProductionTemplate",			//72
	"fProp",				//73
	"fRaise",				//74
	"fRecord",				//75
	"fRequire",				//76
	"fScanner",				//77
	"fSelf",				//78
	"fSideCondition",			//79
	"fSkip",				//80
	"fSynAction",				//81
	"fSynAlternative",			//82
	"fSynApplication",			//83
	"fSynAssignment",			//84
	"fSynSequence",				//85
	"fSynTemplateInstantiation",		//86
	"fSynTopLevelProductionTemplates",	//87
	"fSyntaxRule",				//88
	"fThread",				//89
	"fToken",				//90
	"fTry",					//91
	"fVar",					//92
	"fWildcard",				//93
	"fileNotFound",				//94
	"gump",					//95
	"kind",					//96
	"msg",					//97
	"none",					//98
	"off",					//99
	"on",					//100
	"parse",				//101
	"parseError",				//102
	"pos",					//103
	"warn",					//104
	"zy",					//105
	"fWhile",				//106
	"fMacro",				//107
	"fDotAssign",				//108
	"fFOR",					//109
	"fColonEquals",				//110
	"parentheses",				//111
	"allowwhileloop",			//112
};

void parser_init(void) {
   for (int i = PA_MAX_ATOM+1; i--; )
     _PA_AtomTab[i] = oz_atomNoDup(_PA_CharTab[i]);
}

// Gump Extensions

#define DEPTH 20

static int depth;

static char prodKeyBuffer[DEPTH][80];
static char *prodKey[DEPTH];
static OZ_Term prodName[DEPTH];

struct TermNode {
  OZ_Term term;
  TermNode *next;
  TermNode(OZ_Term t, TermNode *n) { term = t; next = n; }
};
static TermNode *terms[DEPTH];
static OZ_Term decls[DEPTH];


//---------------------
// Operations on Terms
//---------------------


inline 
OZ_Term unwrap(OZ_Term t) {
  // sub-terms aren't always SRecords, e.g. when creating
  // a long position from two sub positions.
  OZ_Term ret;
  if (oz_isSRecord(t) && oz_eq(OZ_label(t), PA_parens)) {
     ret = OZ_getArg(t, 0);
     return ret;
  } 
  else
    return t;
}

inline
OZ_Term newCTerm(OZ_Term l, OZ_Term t1) {
  SRecord * t = SRecord::newSRecord(l, 1);
  t->setArg(0, unwrap(t1));
  return makeTaggedSRecord(t);
}

inline
OZ_Term newCTerm(OZ_Term l, OZ_Term t1, OZ_Term t2) {
  SRecord * t = SRecord::newSRecord(l, 2);
  t->setArg(0, unwrap(t1));
  t->setArg(1, unwrap(t2));
  return makeTaggedSRecord(t);
}

inline
OZ_Term newCTerm(OZ_Term l, OZ_Term t1, OZ_Term t2, OZ_Term t3) {
  SRecord * t = SRecord::newSRecord(l, 3);
  t->setArg(0, unwrap(t1));
  t->setArg(1, unwrap(t2));
  t->setArg(2, unwrap(t3));
  return makeTaggedSRecord(t);
}

inline
OZ_Term newCTerm(OZ_Term l, OZ_Term t1, OZ_Term t2, OZ_Term t3, OZ_Term t4) {
  SRecord * t = SRecord::newSRecord(l, 4);
  t->setArg(0, unwrap(t1));
  t->setArg(1, unwrap(t2));
  t->setArg(2, unwrap(t3));
  t->setArg(3, unwrap(t4));
  return makeTaggedSRecord(t);
}

inline
OZ_Term newCTerm(OZ_Term l, OZ_Term t1, OZ_Term t2, OZ_Term t3, OZ_Term t4, OZ_Term t5) {
  SRecord * t = SRecord::newSRecord(l, 5);
  t->setArg(0, unwrap(t1));
  t->setArg(1, unwrap(t2));
  t->setArg(2, unwrap(t3));
  t->setArg(3, unwrap(t4));
  t->setArg(4, unwrap(t5));
  return makeTaggedSRecord(t);
}

inline
OZ_Term newCTerm(OZ_Term l, OZ_Term t1, OZ_Term t2, OZ_Term t3, OZ_Term t4, OZ_Term t5, OZ_Term t6) {
  SRecord * t = SRecord::newSRecord(l, 6);
  t->setArg(0, unwrap(t1));
  t->setArg(1, unwrap(t2));
  t->setArg(2, unwrap(t3));
  t->setArg(3, unwrap(t4));
  t->setArg(4, unwrap(t5));
  t->setArg(5, unwrap(t6));
  return makeTaggedSRecord(t);
}

inline
OZ_Term newCTerm(OZ_Term l, OZ_Term t1, OZ_Term t2, OZ_Term t3, OZ_Term t4, OZ_Term t5, OZ_Term t6, OZ_Term t7) {
  SRecord * t = SRecord::newSRecord(l, 7);
  t->setArg(0, unwrap(t1));
  t->setArg(1, unwrap(t2));
  t->setArg(2, unwrap(t3));
  t->setArg(3, unwrap(t4));
  t->setArg(4, unwrap(t5));
  t->setArg(5, unwrap(t6));
  t->setArg(6, unwrap(t7));
  return makeTaggedSRecord(t);
}

static 
OZ_Term makeLongPos(OZ_Term pos1, OZ_Term pos2) {
  return 
    newCTerm(PA_pos,
	     OZ_subtree(pos1,makeTaggedSmallInt(1)),OZ_subtree(pos1,makeTaggedSmallInt(2)),
	     OZ_subtree(pos1,makeTaggedSmallInt(3)),OZ_subtree(pos2,makeTaggedSmallInt(1)),
	     OZ_subtree(pos2,makeTaggedSmallInt(2)),OZ_subtree(pos2,makeTaggedSmallInt(3)));
}

static
OZ_Term pos(void) {
  SRecord * t = SRecord::newSRecord(PA_pos, 3);
  t->setArg(0, xyFileNameAtom);
  t->setArg(1, oz_int(xylino));
  t->setArg(2, oz_int(xycharno()));
  return makeTaggedSRecord(t);
}

inline 
OZ_Term makeVar(OZ_Term printName, OZ_Term pos) {
  SRecord * t = SRecord::newSRecord(PA_fVar, 2);
  t->setArg(0, printName);
  t->setArg(1, pos);
  return makeTaggedSRecord(t);
}

inline 
OZ_Term makeVar(char *printName) {
  return makeVar(OZ_atom(printName),pos());
}

inline
TaggedRef oz_consUnwrap(TaggedRef head, TaggedRef tail)
{
  return oz_cons(unwrap(head),unwrap(tail));
}

inline
TaggedRef oz_mklistUnwrap(TaggedRef l1) {
  return oz_mklist(unwrap(l1));
}

inline
TaggedRef oz_mklistUnwrap(TaggedRef l1,TaggedRef l2) {
  return oz_mklist(unwrap(l1), unwrap(l2));
}

inline 
OZ_Term makeCons(OZ_Term first, OZ_Term second, OZ_Term pos) {
  SRecord * t1 = SRecord::newSRecord(PA_fRecord, 2);
  SRecord * t2 = SRecord::newSRecord(PA_fAtom,   2);

  t2->setArg(0, AtomCons);
  t2->setArg(1, pos);
  
  t1->setArg(0, makeTaggedSRecord(t2));
  t1->setArg(1, oz_mklist(unwrap(first),unwrap(second)));

  return makeTaggedSRecord(t1);
}

inline
OZ_Term makeInt(const char * chars, OZ_Term pos) {
  SRecord * t = SRecord::newSRecord(PA_fInt, 2);
  t->setArg(0, OZ_CStringToInt(chars));
  t->setArg(1, pos);
  return makeTaggedSRecord(t);
}

inline
OZ_Term makeInt(const char c, OZ_Term pos) {
  SRecord * t = SRecord::newSRecord(PA_fInt, 2);
  t->setArg(0, makeTaggedSmallInt((unsigned char) c));
  t->setArg(1, pos);
  return makeTaggedSRecord(t);
}

static 
OZ_Term makeString(const char * chars, OZ_Term pos) {
  int l = strlen(chars);

  SRecord * t = SRecord::newSRecord(PA_fAtom, 2);
  t->setArg(0, AtomNil);
  t->setArg(1, pos);
  
  OZ_Term s = makeTaggedSRecord(t);

  while (l--)
    s = makeCons(makeInt(chars[l],pos),s,pos);

  return s;
}

inline
TaggedRef oz_headUnwrap(TaggedRef l) {
  return oz_head(unwrap(l));
}

inline
TaggedRef oz_tailUnwrap(TaggedRef l) {
  return oz_tail(unwrap(l));
}

inline
TaggedRef oz_pair2Unwrap(TaggedRef l1,TaggedRef l2) {
  return oz_pair2(unwrap(l1), unwrap(l2));
}
//------
// Gump
//------

static OZ_Term scannerPrefix = 0;
static OZ_Term parserExpect = 0;

void xy_setScannerPrefix() {
  scannerPrefix = OZ_atom(xytext);
}

void xy_setParserExpect() {
  parserExpect = OZ_CStringToInt(xytext);
}



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE

{
  OZ_Term t;
  int i;
}
/* Line 193 of yacc.c.  */

	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */


#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  120
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2152

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  118
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  143
/* YYNRULES -- Number of rules.  */
#define YYNRULES  371
/* YYNRULES -- Number of states.  */
#define YYNSTATES  796

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   351

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   117,     2,    95,   110,     2,     2,     2,
     107,   108,     2,   105,    99,   106,   101,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   115,   116,
       2,    83,     2,     2,   103,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   111,     2,   112,   102,   109,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   113,    94,   114,   100,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    96,    97,
      98,   104
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     8,    11,    14,    16,    20,    23,
      30,    37,    43,    44,    47,    49,    51,    53,    54,    57,
      60,    63,    65,    68,    73,    78,    83,    88,    93,    98,
     103,   108,   113,   115,   120,   122,   126,   131,   136,   141,
     146,   150,   155,   158,   163,   167,   171,   175,   177,   179,
     181,   183,   185,   187,   189,   191,   193,   195,   197,   199,
     206,   213,   224,   235,   242,   244,   251,   254,   257,   263,
     271,   277,   285,   291,   293,   295,   301,   304,   310,   316,
     322,   324,   326,   332,   340,   348,   349,   352,   354,   358,
     362,   366,   368,   373,   377,   383,   384,   387,   390,   391,
     394,   395,   398,   399,   401,   406,   413,   418,   423,   428,
     435,   440,   441,   445,   452,   455,   457,   461,   464,   469,
     470,   473,   474,   477,   482,   484,   486,   488,   490,   492,
     494,   499,   501,   502,   505,   507,   511,   512,   516,   517,
     520,   528,   536,   538,   540,   542,   544,   546,   547,   550,
     555,   556,   558,   560,   562,   564,   566,   568,   570,   572,
     574,   581,   584,   587,   591,   593,   601,   608,   611,   614,
     618,   620,   622,   626,   630,   632,   636,   640,   642,   647,
     654,   659,   664,   666,   671,   679,   681,   683,   684,   687,
     692,   697,   702,   707,   708,   711,   715,   717,   719,   721,
     723,   725,   727,   729,   730,   733,   739,   741,   746,   748,
     750,   752,   754,   756,   761,   767,   769,   771,   775,   777,
     779,   781,   784,   785,   788,   793,   795,   797,   799,   803,
     804,   810,   813,   814,   816,   820,   825,   831,   835,   839,
     842,   847,   852,   858,   860,   864,   866,   868,   870,   874,
     876,   878,   880,   882,   883,   884,   893,   895,   897,   899,
     902,   905,   908,   914,   920,   925,   927,   929,   934,   935,
     938,   941,   943,   945,   955,   957,   959,   962,   965,   966,
     969,   971,   974,   976,   980,   982,   985,   987,   990,   991,
    1001,  1002,  1003,  1014,  1021,  1025,  1028,  1031,  1033,  1034,
    1037,  1038,  1039,  1046,  1047,  1048,  1055,  1056,  1057,  1064,
    1066,  1070,  1072,  1074,  1076,  1077,  1079,  1081,  1083,  1084,
    1085,  1088,  1090,  1093,  1098,  1103,  1111,  1112,  1115,  1117,
    1119,  1121,  1123,  1125,  1129,  1132,  1136,  1137,  1140,  1143,
    1149,  1154,  1157,  1160,  1162,  1164,  1166,  1169,  1173,  1175,
    1177,  1182,  1184,  1190,  1192,  1194,  1200,  1205,  1206,  1207,
    1217,  1218,  1219,  1229,  1230,  1231,  1241,  1243,  1249,  1251,
    1253,  1255
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     119,     0,    -1,   120,    71,    -1,     1,    -1,   125,   121,
      -1,   215,   121,    -1,   121,    -1,   199,   137,   121,    -1,
     122,   120,    -1,    28,   200,   125,    47,   199,   121,    -1,
      28,   200,   125,    47,   125,   121,    -1,    28,   200,   125,
     199,   121,    -1,    -1,     3,   123,    -1,     5,    -1,     6,
      -1,     7,    -1,    -1,   124,   123,    -1,   105,     4,    -1,
     106,     4,    -1,   126,    -1,   126,   125,    -1,   126,    83,
     200,   126,    -1,   126,    86,   200,   126,    -1,   126,    84,
     200,   126,    -1,   126,    87,   200,   126,    -1,   126,    88,
     200,   126,    -1,   126,   143,   200,   126,    -1,   126,   144,
     200,   126,    -1,   126,   145,   200,   126,    -1,   126,    94,
     200,   126,    -1,   128,    -1,   128,    95,   200,   127,    -1,
     128,    -1,   128,    95,   127,    -1,   128,   146,   200,   128,
      -1,   128,   147,   200,   128,    -1,   128,   148,   200,   128,
      -1,   128,    99,   200,   128,    -1,   100,   200,   128,    -1,
     128,   101,   200,   128,    -1,   128,    13,    -1,   128,   102,
     200,   128,    -1,   103,   200,   128,    -1,   104,   200,   128,
      -1,   107,   149,   108,    -1,   193,    -1,   195,    -1,   109,
      -1,    66,    -1,    63,    -1,    38,    -1,    59,    -1,   110,
      -1,   196,    -1,   197,    -1,   198,    -1,   154,    -1,   111,
     200,   126,   151,   112,   200,    -1,   113,   200,   126,   150,
     114,   200,    -1,    55,   200,   135,   113,   126,   150,   114,
     149,    35,   200,    -1,    43,   200,   135,   113,   126,   150,
     114,   149,    35,   200,    -1,    44,   200,   171,   136,    35,
     200,    -1,   170,    -1,    48,   200,   125,    47,   125,    35,
      -1,    45,   161,    -1,    23,   163,    -1,    49,   200,   149,
      35,   200,    -1,    49,   200,   126,    61,   149,    35,   200,
      -1,    62,   200,   149,    35,   200,    -1,    65,   200,   149,
     152,   153,    35,   200,    -1,    57,   200,   149,    35,   200,
      -1,    60,    -1,    37,    -1,    51,   200,   149,    35,   200,
      -1,    27,   186,    -1,    53,   200,   190,    35,   200,    -1,
      30,   200,   190,    35,   200,    -1,    25,   200,   192,    35,
     200,    -1,   201,    -1,   209,    -1,    90,   200,   150,    89,
     200,    -1,    69,   200,   129,    70,   149,    35,   200,    -1,
      68,   200,   126,    70,   149,    35,   200,    -1,    -1,   130,
     129,    -1,   193,    -1,   193,   115,   126,    -1,   126,    47,
     131,    -1,   126,    42,   126,    -1,   126,    -1,   126,    20,
     126,   132,    -1,   126,   116,   133,    -1,   107,   126,   116,
     133,   108,    -1,    -1,   116,   126,    -1,   126,   134,    -1,
      -1,   116,   126,    -1,    -1,   193,   135,    -1,    -1,   137,
      -1,    58,   200,   138,   136,    -1,    54,   200,   125,    47,
     125,   136,    -1,    54,   200,   125,   136,    -1,    46,   200,
     138,   136,    -1,    36,   200,   142,   136,    -1,    29,   200,
     125,    47,   125,   136,    -1,    29,   200,   125,   136,    -1,
      -1,   194,   141,   138,    -1,   139,   107,   140,   108,   141,
     138,    -1,    16,   200,    -1,   160,    -1,   160,   115,   194,
      -1,   160,   140,    -1,   160,   115,   194,   140,    -1,    -1,
      22,   193,    -1,    -1,   194,   142,    -1,   160,   115,   194,
     142,    -1,    92,    -1,    91,    -1,    93,    -1,    96,    -1,
      98,    -1,    97,    -1,   125,    47,   200,   125,    -1,   125,
      -1,    -1,   126,   150,    -1,   199,    -1,   199,   126,   151,
      -1,    -1,    24,   200,   166,    -1,    -1,    41,   149,    -1,
     155,   200,   107,   157,   158,   108,   200,    -1,   156,   200,
     107,   157,   158,   108,   200,    -1,     9,    -1,    67,    -1,
      64,    -1,    39,    -1,    16,    -1,    -1,   126,   157,    -1,
     159,   115,   126,   157,    -1,    -1,    19,    -1,   193,    -1,
     194,    -1,   197,    -1,    66,    -1,    63,    -1,    38,    -1,
     193,    -1,   197,    -1,   200,   125,    61,   149,   162,   200,
      -1,    33,   161,    -1,    32,   163,    -1,    31,   149,    35,
      -1,    35,    -1,   200,   125,    61,   200,   149,   164,   200,
      -1,   200,   125,    52,   165,   164,   200,    -1,    33,   161,
      -1,    32,   163,    -1,    31,   149,    35,    -1,    35,    -1,
     167,    -1,   167,    18,   165,    -1,   167,    34,   165,    -1,
     167,    -1,   167,    18,   166,    -1,   168,    61,   149,    -1,
     169,    -1,   169,    88,   199,   125,    -1,   169,    88,   199,
     125,    47,   125,    -1,   169,    83,   200,   169,    -1,   169,
      94,   200,   169,    -1,   128,    -1,   128,    95,   200,   127,
      -1,    26,   200,   171,   172,   177,    35,   200,    -1,   126,
      -1,   199,    -1,    -1,   173,   172,    -1,    42,   200,   126,
     150,    -1,    21,   200,   175,   174,    -1,    40,   200,   175,
     174,    -1,    56,   200,   126,   150,    -1,    -1,   175,   174,
      -1,   176,   115,   126,    -1,   176,    -1,   193,    -1,   195,
      -1,   197,    -1,    66,    -1,    63,    -1,    38,    -1,    -1,
     178,   177,    -1,    50,   200,   179,   149,    35,    -1,   180,
      -1,   180,    83,   200,   194,    -1,   193,    -1,   195,    -1,
      66,    -1,    63,    -1,    38,    -1,   181,   107,   182,   108,
      -1,   181,   107,   182,    19,   108,    -1,     9,    -1,    16,
      -1,   117,   200,    16,    -1,    67,    -1,    64,    -1,    39,
      -1,   183,   182,    -1,    -1,   184,   185,    -1,   159,   115,
     184,   185,    -1,   194,    -1,   110,    -1,   109,    -1,    17,
     200,   126,    -1,    -1,   200,   188,   187,    35,   200,    -1,
      31,   149,    -1,    -1,   189,    -1,   189,    18,   188,    -1,
     125,    61,   200,   149,    -1,   125,    47,   125,    61,   149,
      -1,   191,    18,   191,    -1,   191,    18,   190,    -1,   125,
     199,    -1,   125,    47,   125,   199,    -1,   125,   199,    61,
     149,    -1,   125,    47,   125,    61,   149,    -1,   149,    -1,
     149,    18,   192,    -1,     8,    -1,    15,    -1,   194,    -1,
     117,   200,   194,    -1,    14,    -1,    11,    -1,    12,    -1,
      10,    -1,    -1,    -1,    77,   200,   194,   172,   177,   202,
      35,   200,    -1,   203,    -1,   204,    -1,   206,    -1,   203,
     202,    -1,   204,   202,    -1,   206,   202,    -1,    73,   193,
      83,   205,    35,    -1,    73,   194,    83,   205,    35,    -1,
      73,   205,   149,    35,    -1,    72,    -1,    14,    -1,    74,
     194,   207,    35,    -1,    -1,   208,   207,    -1,    42,   214,
      -1,   204,    -1,   206,    -1,    75,   200,   194,   172,   177,
     211,   210,    35,   200,    -1,   238,    -1,   216,    -1,   238,
     210,    -1,   216,   210,    -1,    -1,    79,   212,    -1,   213,
      -1,   213,   212,    -1,   193,    -1,   193,   115,   126,    -1,
     194,    -1,   194,   214,    -1,   216,    -1,   216,   215,    -1,
      -1,    76,   194,    83,   217,   220,   235,   236,   241,    35,
      -1,    -1,    -1,    76,   110,   218,    83,   219,   220,   235,
     236,   241,    35,    -1,    76,   220,   235,   236,   241,    35,
      -1,   222,   194,   233,    -1,   194,   234,    -1,   221,   223,
      -1,   222,    -1,    -1,   193,   115,    -1,    -1,    -1,   107,
     224,   230,   108,   225,   233,    -1,    -1,    -1,   111,   226,
     230,   112,   227,   233,    -1,    -1,    -1,   113,   228,   230,
     114,   229,   233,    -1,   231,    -1,   231,   232,   230,    -1,
     194,    -1,   109,    -1,    81,    -1,    -1,   234,    -1,    96,
      -1,    98,    -1,    -1,    -1,   237,    47,    -1,   238,    -1,
     238,   237,    -1,    78,   193,   241,    35,    -1,    78,   194,
     241,    35,    -1,    78,   259,   107,   239,   108,   241,    35,
      -1,    -1,   240,   239,    -1,   194,    -1,   110,    -1,   109,
      -1,   242,    -1,   243,    -1,   243,    18,   242,    -1,   199,
     245,    -1,    60,   200,   244,    -1,    -1,    80,   149,    -1,
     246,   245,    -1,   246,   234,   200,   247,   235,    -1,   246,
      83,   249,   247,    -1,    47,   247,    -1,   250,   247,    -1,
     244,    -1,   194,    -1,   244,    -1,   248,   247,    -1,   195,
      83,   249,    -1,   249,    -1,   194,    -1,   194,   234,   200,
     235,    -1,   251,    -1,   117,   200,   194,    83,   249,    -1,
     251,    -1,   258,    -1,   222,   200,   258,   233,   235,    -1,
     258,   234,   200,   235,    -1,    -1,    -1,   221,   200,   107,
     252,   260,   108,   253,   233,   235,    -1,    -1,    -1,   221,
     200,   111,   254,   260,   112,   255,   233,   235,    -1,    -1,
      -1,   221,   200,   113,   256,   260,   114,   257,   233,   235,
      -1,   193,    -1,   259,   200,   107,   150,   108,    -1,     9,
      -1,    16,    -1,   241,    -1,   241,   232,   260,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   749,   749,   751,   755,   757,   760,   762,   767,   769,
     772,   774,   778,   781,   783,   785,   787,   792,   793,   797,
     806,   817,   819,   823,   825,   833,   835,   837,   839,   842,
     844,   846,   848,   850,   856,   858,   862,   865,   868,   871,
     873,   876,   879,   882,   885,   887,   890,   892,   894,   896,
     898,   900,   902,   904,   906,   908,   910,   912,   914,   916,
     920,   922,   925,   928,   930,   932,   934,   936,   938,   940,
     942,   944,   946,   948,   950,   952,   954,   956,   958,   960,
     962,   964,   966,   968,   970,   975,   976,   980,   982,   984,
     986,   990,   992,   994,   997,  1003,  1004,  1008,  1013,  1014,
    1019,  1020,  1026,  1027,  1032,  1034,  1036,  1039,  1041,  1043,
    1045,  1051,  1052,  1054,  1058,  1062,  1064,  1066,  1068,  1073,
    1074,  1079,  1080,  1082,  1087,  1091,  1095,  1099,  1103,  1107,
    1111,  1113,  1118,  1119,  1123,  1125,  1132,  1133,  1138,  1139,
    1143,  1148,  1155,  1157,  1159,  1161,  1165,  1170,  1171,  1173,
    1178,  1179,  1183,  1185,  1187,  1189,  1191,  1193,  1197,  1199,
    1203,  1207,  1209,  1211,  1213,  1217,  1221,  1225,  1227,  1229,
    1231,  1235,  1237,  1239,  1243,  1245,  1249,  1253,  1255,  1258,
    1262,  1264,  1266,  1268,  1274,  1279,  1281,  1287,  1288,  1292,
    1294,  1296,  1298,  1303,  1304,  1308,  1310,  1314,  1316,  1318,
    1320,  1322,  1324,  1329,  1330,  1334,  1338,  1340,  1344,  1346,
    1348,  1350,  1352,  1354,  1356,  1360,  1362,  1364,  1366,  1368,
    1370,  1374,  1377,  1380,  1382,  1386,  1388,  1390,  1395,  1398,
    1401,  1405,  1408,  1411,  1413,  1417,  1419,  1423,  1425,  1429,
    1433,  1435,  1438,  1442,  1444,  1448,  1452,  1456,  1458,  1462,
    1466,  1468,  1472,  1477,  1481,  1490,  1498,  1500,  1502,  1504,
    1506,  1508,  1512,  1514,  1518,  1522,  1524,  1528,  1533,  1534,
    1538,  1540,  1542,  1548,  1556,  1558,  1560,  1562,  1567,  1568,
    1572,  1574,  1578,  1580,  1584,  1586,  1590,  1592,  1597,  1596,
    1600,  1601,  1600,  1604,  1608,  1610,  1612,  1616,  1617,  1620,
    1624,  1625,  1624,  1626,  1627,  1626,  1628,  1629,  1628,  1632,
    1634,  1638,  1639,  1642,  1646,  1647,  1650,  1651,  1655,  1663,
    1664,  1668,  1670,  1674,  1676,  1678,  1683,  1684,  1688,  1690,
    1692,  1696,  1700,  1702,  1706,  1715,  1720,  1721,  1725,  1727,
    1737,  1742,  1749,  1751,  1755,  1759,  1761,  1765,  1767,  1771,
    1773,  1779,  1783,  1786,  1791,  1793,  1797,  1801,  1802,  1801,
    1805,  1806,  1805,  1809,  1810,  1809,  1815,  1817,  1821,  1823,
    1828,  1830
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_SWITCH", "T_SWITCHNAME",
  "T_LOCALSWITCHES", "T_PUSHSWITCHES", "T_POPSWITCHES", "T_OZATOM",
  "T_ATOM_LABEL", "T_OZFLOAT", "T_OZINT", "T_AMPER", "T_DOTINT",
  "T_STRING", "T_VARIABLE", "T_VARIABLE_LABEL", "T_DEFAULT", "T_CHOICE",
  "T_LDOTS", "T_2DOTS", "T_attr", "T_at", "T_case", "T_catch", "T_choice",
  "T_class", "T_cond", "T_declare", "T_define", "T_dis", "T_else",
  "T_elsecase", "T_elseif", "T_elseof", "T_end", "T_export", "T_fail",
  "T_false", "T_FALSE_LABEL", "T_feat", "T_finally", "T_from", "T_fun",
  "T_functor", "T_if", "T_import", "T_in", "T_local", "T_lock", "T_meth",
  "T_not", "T_of", "T_or", "T_prepare", "T_proc", "T_prop", "T_raise",
  "T_require", "T_self", "T_skip", "T_then", "T_thread", "T_true",
  "T_TRUE_LABEL", "T_try", "T_unit", "T_UNIT_LABEL", "T_while", "T_FOR",
  "T_do", "T_ENDOFFILE", "T_REGEX", "T_lex", "T_mode", "T_parser",
  "T_prod", "T_scanner", "T_syn", "T_token", "T_REDUCE", "T_SEP", "T_ITER",
  "'='", "T_OOASSIGN", "T_DOTASSIGN", "T_COLONEQUALS", "T_orelse",
  "T_andthen", "T_RMACRO", "T_LMACRO", "T_FDCOMPARE", "T_COMPARE",
  "T_FDIN", "'|'", "'#'", "T_ADD", "T_OTHERMUL", "T_FDMUL", "','", "'~'",
  "'.'", "'^'", "'@'", "T_DEREFF", "'+'", "'-'", "'('", "')'", "'_'",
  "'$'", "'['", "']'", "'{'", "'}'", "':'", "';'", "'!'", "$accept",
  "file", "queries", "queries1", "directive", "switchList", "switch",
  "sequence", "phrase", "hashes", "phrase2", "FOR_decls", "FOR_decl",
  "FOR_gen", "FOR_genOptInt", "FOR_genOptC", "FOR_genOptC2", "procFlags",
  "optFunctorDescriptorList", "functorDescriptorList", "importDecls",
  "variableLabel", "featureList", "optImportAt", "exportDecls", "compare",
  "fdCompare", "fdIn", "add", "fdMul", "otherMul", "inSequence",
  "phraseList", "fixedListArgs", "optCatch", "optFinally", "record",
  "recordAtomLabel", "recordVarLabel", "recordArguments", "optDots",
  "feature", "featureNoVar", "ifMain", "ifRest", "caseMain", "caseRest",
  "elseOfList", "caseClauseList", "caseClause", "sideCondition", "pattern",
  "class", "phraseOpt", "classDescriptorList", "classDescriptor",
  "attrFeatList", "attrFeat", "attrFeatFeature", "methList", "meth",
  "methHead", "methHead1", "methHeadLabel", "methFormals", "methFormal",
  "methFormalTerm", "methFormalOptDefault", "condMain", "condElse",
  "condClauseList", "condClause", "orClauseList", "orClause",
  "choiceClauseList", "atom", "nakedVariable", "variable", "string", "int",
  "float", "thisCoord", "coord", "scannerSpecification", "scannerRules",
  "lexAbbrev", "lexRule", "regex", "modeClause", "modeDescrs", "modeDescr",
  "parserSpecification", "parserRules", "tokenClause", "tokenList",
  "tokenDecl", "modeFromList", "prodClauseList", "prodClause", "@1", "@2",
  "@3", "prodHeadRest", "prodName", "prodNameAtom", "prodKey", "@4", "@5",
  "@6", "@7", "@8", "@9", "prodParams", "prodParam", "separatorOp",
  "optTerminatorOp", "terminatorOp", "prodMakeKey", "localRules",
  "localRulesSub", "synClause", "synParams", "synParam", "synAlt",
  "synSeqs", "synSeq", "optSynAction", "nonEmptySeq", "synVariable",
  "synPrims", "synPrim", "synPrimNoAssign", "synPrimNoVar",
  "synPrimNoVarNoAssign", "@10", "@11", "@12", "@13", "@14", "@15",
  "synInstTerm", "synLabel", "synProdCallParams", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,    61,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   124,    35,   348,   349,   350,    44,
     126,    46,    94,    64,   351,    43,    45,    40,    41,    95,
      36,    91,    93,   123,   125,    58,    59,    33
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   118,   119,   119,   120,   120,   120,   120,   121,   121,
     121,   121,   121,   122,   122,   122,   122,   123,   123,   124,
     124,   125,   125,   126,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   127,   127,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   129,   129,   130,   130,   130,
     130,   131,   131,   131,   131,   132,   132,   133,   134,   134,
     135,   135,   136,   136,   137,   137,   137,   137,   137,   137,
     137,   138,   138,   138,   139,   140,   140,   140,   140,   141,
     141,   142,   142,   142,   143,   144,   145,   146,   147,   148,
     149,   149,   150,   150,   151,   151,   152,   152,   153,   153,
     154,   154,   155,   155,   155,   155,   156,   157,   157,   157,
     158,   158,   159,   159,   159,   159,   159,   159,   160,   160,
     161,   162,   162,   162,   162,   163,   163,   164,   164,   164,
     164,   165,   165,   165,   166,   166,   167,   168,   168,   168,
     169,   169,   169,   169,   170,   171,   171,   172,   172,   173,
     173,   173,   173,   174,   174,   175,   175,   176,   176,   176,
     176,   176,   176,   177,   177,   178,   179,   179,   180,   180,
     180,   180,   180,   180,   180,   181,   181,   181,   181,   181,
     181,   182,   182,   183,   183,   184,   184,   184,   185,   185,
     186,   187,   187,   188,   188,   189,   189,   190,   190,   191,
     191,   191,   191,   192,   192,   193,   194,   195,   195,   196,
     197,   197,   198,   199,   200,   201,   202,   202,   202,   202,
     202,   202,   203,   203,   204,   205,   205,   206,   207,   207,
     208,   208,   208,   209,   210,   210,   210,   210,   211,   211,
     212,   212,   213,   213,   214,   214,   215,   215,   217,   216,
     218,   219,   216,   216,   220,   220,   220,   221,   221,   222,
     224,   225,   223,   226,   227,   223,   228,   229,   223,   230,
     230,   231,   231,   232,   233,   233,   234,   234,   235,   236,
     236,   237,   237,   238,   238,   238,   239,   239,   240,   240,
     240,   241,   242,   242,   243,   243,   244,   244,   245,   245,
     245,   245,   245,   245,   246,   247,   247,   248,   248,   249,
     249,   249,   250,   250,   251,   251,   251,   252,   253,   251,
     254,   255,   251,   256,   257,   251,   258,   258,   259,   259,
     260,   260
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     1,     2,     2,     1,     3,     2,     6,
       6,     5,     0,     2,     1,     1,     1,     0,     2,     2,
       2,     1,     2,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     1,     4,     1,     3,     4,     4,     4,     4,
       3,     4,     2,     4,     3,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     6,
       6,    10,    10,     6,     1,     6,     2,     2,     5,     7,
       5,     7,     5,     1,     1,     5,     2,     5,     5,     5,
       1,     1,     5,     7,     7,     0,     2,     1,     3,     3,
       3,     1,     4,     3,     5,     0,     2,     2,     0,     2,
       0,     2,     0,     1,     4,     6,     4,     4,     4,     6,
       4,     0,     3,     6,     2,     1,     3,     2,     4,     0,
       2,     0,     2,     4,     1,     1,     1,     1,     1,     1,
       4,     1,     0,     2,     1,     3,     0,     3,     0,     2,
       7,     7,     1,     1,     1,     1,     1,     0,     2,     4,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       6,     2,     2,     3,     1,     7,     6,     2,     2,     3,
       1,     1,     3,     3,     1,     3,     3,     1,     4,     6,
       4,     4,     1,     4,     7,     1,     1,     0,     2,     4,
       4,     4,     4,     0,     2,     3,     1,     1,     1,     1,
       1,     1,     1,     0,     2,     5,     1,     4,     1,     1,
       1,     1,     1,     4,     5,     1,     1,     3,     1,     1,
       1,     2,     0,     2,     4,     1,     1,     1,     3,     0,
       5,     2,     0,     1,     3,     4,     5,     3,     3,     2,
       4,     4,     5,     1,     3,     1,     1,     1,     3,     1,
       1,     1,     1,     0,     0,     8,     1,     1,     1,     2,
       2,     2,     5,     5,     4,     1,     1,     4,     0,     2,
       2,     1,     1,     9,     1,     1,     2,     2,     0,     2,
       1,     2,     1,     3,     1,     2,     1,     2,     0,     9,
       0,     0,    10,     6,     3,     2,     2,     1,     0,     2,
       0,     0,     6,     0,     0,     6,     0,     0,     6,     1,
       3,     1,     1,     1,     0,     1,     1,     1,     0,     0,
       2,     1,     2,     4,     4,     7,     0,     2,     1,     1,
       1,     1,     1,     3,     2,     3,     0,     2,     2,     5,
       4,     2,     2,     1,     1,     1,     2,     3,     1,     1,
       4,     1,     5,     1,     1,     5,     4,     0,     0,     9,
       0,     0,     9,     0,     0,     9,     1,     5,     1,     1,
       1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     3,    17,    14,    15,    16,   245,   142,   252,   250,
     251,   249,   246,   146,   254,   254,   254,   254,   254,   254,
      74,    52,   145,   254,   254,   254,   254,   254,   254,   254,
     254,   254,    53,    73,   254,    51,   144,   254,    50,   143,
     254,   254,   254,   298,   254,   254,   254,   254,   254,     0,
      49,    54,   254,   254,   254,     0,     0,     6,   253,    12,
      21,    32,    58,   254,   254,    64,    47,   247,    48,    55,
      56,    57,     0,    80,    81,    12,   286,     0,     0,    13,
      17,    67,     0,     0,   253,    76,     0,     0,     0,   100,
     253,    66,     0,     0,     0,     0,     0,   100,     0,     0,
       0,     0,    85,     0,   290,     0,     0,   318,     0,   297,
       0,   132,     0,     0,     0,   131,     0,     0,     0,     0,
       1,     2,     8,     4,   254,   254,   254,   254,   254,   125,
     124,   126,   254,    22,   254,   254,   254,    42,   254,   127,
     129,   128,   254,   254,   254,   254,   254,   254,     0,     0,
     254,   254,   254,   254,   254,    12,     5,   287,    19,    20,
      18,     0,   243,     0,   185,   187,   186,     0,   232,   233,
     253,   253,     0,     0,     0,   100,   102,     0,     0,    21,
       0,     0,     0,     0,     0,     0,   136,     0,     0,     0,
      85,    87,   187,     0,   299,   288,   316,   317,   295,   319,
     300,   303,   306,   296,   314,   187,   132,     0,    40,    44,
      45,   254,    46,   253,   132,   248,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   147,   147,     0,   121,   111,     0,   111,     7,
       0,   254,     0,   254,   254,   254,   254,   254,   203,   187,
       0,   254,     0,     0,     0,   253,    12,     0,   239,   254,
       0,     0,   101,     0,   103,     0,     0,     0,   254,   254,
     254,     0,   254,   254,   254,   138,     0,     0,     0,     0,
      86,     0,   203,   291,   298,     0,   253,     0,   321,     0,
       0,     0,   294,   315,   203,   133,   254,     0,     0,   134,
       0,    23,    25,    24,    26,    27,    31,    28,    29,    30,
      33,    34,    39,    41,    43,    36,    37,    38,    52,    51,
      50,   147,   150,     0,    47,   247,    56,   150,   102,   102,
       0,   158,   121,   159,   254,   102,     0,   119,   102,   102,
     182,     0,   171,     0,   177,     0,   244,    79,     0,     0,
       0,     0,   254,     0,   203,   188,     0,     0,   231,   254,
     234,    12,    12,    11,   253,     0,    78,   238,   237,   132,
     254,     0,     0,     0,    68,    75,    77,   132,    72,    70,
       0,     0,     0,     0,    90,     0,    91,    89,     0,    88,
     278,   298,     0,   318,   368,   369,   253,   253,     0,   254,
     336,     0,   331,   332,   320,   322,   312,   311,     0,   309,
       0,     0,     0,    82,   130,   254,   253,   254,     0,   148,
     151,     0,     0,     0,     0,   110,   108,     0,   122,   114,
     107,     0,     0,   111,     0,   106,   104,   254,     0,   254,
     254,   170,   254,     0,     0,     0,   254,   253,   254,     0,
     202,   201,   200,   193,   196,   197,   198,   199,   193,   132,
     132,     0,   254,   204,     0,   235,   230,    10,     9,     0,
     240,   241,     0,    63,     0,   254,   254,   164,   254,    65,
     254,     0,   137,   174,   139,   254,   254,    21,     0,     0,
     254,     0,     0,   318,   319,     0,     0,   326,   336,   336,
       0,   254,   366,   344,   254,   254,   343,   334,   336,   336,
     353,   354,   254,   293,   253,   301,   313,     0,   304,   307,
       0,     0,     0,   256,   257,   258,    59,   135,    60,    35,
     254,   147,   254,   102,   121,     0,   115,   120,   112,   102,
       0,     0,   168,   167,   166,   172,   173,   176,     0,     0,
       0,   254,   190,   193,     0,   191,   189,   192,   215,   216,
     212,   220,   211,   219,   210,   218,   254,     0,   206,     0,
     208,   209,   184,   236,   242,     0,     0,   162,   161,   160,
      69,     0,     0,    71,    84,     0,    95,    98,    93,    83,
     282,   279,   280,     0,   275,   274,   319,   253,   323,   324,
     330,   329,   328,     0,   326,   335,   349,     0,   345,   341,
     336,   348,   351,   337,     0,     0,     0,   298,   254,   338,
     342,   254,     0,   333,   314,   310,   314,   314,   266,   265,
       0,     0,     0,   268,   254,   259,   260,   261,   140,   149,
     141,   109,   123,   119,     0,   117,   105,   183,   169,   180,
     178,   181,   165,   194,   195,     0,     0,   254,   222,     0,
     163,     0,   175,     0,     0,    92,     0,    97,     0,   281,
     254,   277,   276,   253,     0,   253,   327,   254,   298,   346,
       0,   357,   360,   363,   366,   314,   349,   336,   336,   318,
     132,   302,   305,   308,     0,     0,     0,     0,     0,   271,
     272,     0,   268,   255,   111,   116,     0,   217,   205,     0,
     157,   156,   155,   227,   226,     0,     0,   222,   229,   152,
     225,   154,   254,   254,    94,    96,    99,   283,   273,     0,
     289,     0,   318,   347,   298,   253,   253,   253,   318,   340,
     318,   356,     0,     0,     0,   264,   284,   270,   267,   269,
     113,   118,   179,   207,     0,     0,   213,   221,   254,   223,
      62,    61,   292,   325,   350,   352,   370,     0,     0,     0,
     355,   339,   367,   262,   263,   285,   229,   225,   214,     0,
     253,   358,   361,   364,   224,   228,   371,   314,   314,   314,
     318,   318,   318,   359,   362,   365
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    55,    56,    57,    58,    79,    80,   115,    60,   310,
      61,   189,   190,   387,   665,   588,   667,   174,   263,   264,
     335,   336,   535,   433,   329,   134,   135,   136,   145,   146,
     147,   116,   207,   298,   275,   382,    62,    63,    64,   322,
     421,   323,   330,    91,   478,    81,   442,   341,   482,   342,
     343,   344,    65,   165,   248,   249,   552,   553,   454,   353,
     354,   567,   568,   569,   716,   717,   718,   759,    85,   253,
     168,   169,   172,   173,   163,    66,    67,    68,    69,    70,
      71,   400,    82,    73,   522,   523,   524,   632,   525,   701,
     702,    74,   593,   492,   591,   592,   747,    75,    76,   284,
     193,   391,   107,   504,   505,   203,   289,   624,   290,   626,
     291,   627,   408,   409,   517,   292,   293,   199,   286,   287,
     288,   603,   604,   766,   402,   403,   608,   507,   508,   609,
     610,   611,   509,   612,   735,   787,   736,   788,   737,   789,
     511,   512,   767
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -657
static const yytype_int16 yypact[] =
{
    1149,  -657,   128,  -657,  -657,  -657,  -657,  -657,  -657,  -657,
    -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,
    -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,
    -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,
    -657,  -657,  -657,    74,  -657,  -657,  -657,  -657,  -657,  1815,
    -657,  -657,  -657,  -657,  -657,    63,    32,  -657,  1265,   315,
    1595,   478,  -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,
    -657,  -657,   362,  -657,  -657,   315,    57,   104,   177,  -657,
     128,  -657,  1815,  1815,  1815,  -657,  1815,  1815,  1815,   181,
    1815,  -657,  1815,  1815,  1815,  1815,  1815,   181,  1815,  1815,
    1815,  1815,  1815,   179,  -657,    86,   249,  -657,   110,   179,
     179,  1815,  1815,  1815,  1815,   195,   144,  1815,  1815,   179,
    -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,
    -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,
    -657,  -657,  -657,  -657,  -657,  -657,  -657,  -657,   170,   184,
    -657,  -657,  -657,  -657,  -657,   315,  -657,  -657,  -657,  -657,
    -657,   143,   266,   260,   963,   243,  -657,   180,   273,   289,
     283,   295,   311,   331,   239,   181,   362,   322,   314,  1375,
     364,   369,   371,   306,   386,   391,   405,   933,   900,   366,
    1815,   817,   243,   349,  -657,  -657,  -657,  -657,  -657,   356,
    -657,  -657,  -657,  -657,   200,   243,  1595,   355,   113,  -657,
    -657,  -657,  -657,   963,  1595,  -657,  1815,  1815,  1815,  1815,
    1815,  1815,  1815,  1815,  1815,  1815,  1815,  1815,  1815,  1815,
    1815,  1815,  1925,  1925,  1815,   473,   308,  1815,   308,  -657,
    1815,  -657,  1815,  -657,  -657,  -657,  -657,  -657,   398,   243,
    1815,  -657,  1815,   419,  1815,  1815,   315,  1815,   395,  -657,
    1815,  1815,  -657,   423,  -657,  1815,  1815,  1815,  -657,  -657,
    -657,  1815,  -657,  -657,  -657,   421,  1815,  1815,  2035,  1815,
    -657,  1815,   398,  -657,    92,   325,   409,   427,   356,    35,
      35,    35,  -657,  -657,   398,  -657,  -657,  1815,   367,  1815,
     373,   963,   538,   565,   504,   517,   402,   410,   284,   284,
    -657,   773,   109,  -657,  -657,   101,   109,   109,   377,   384,
     401,  1705,   494,   408,   417,   418,   424,   494,   281,   362,
     425,  -657,   473,  -657,  -657,   362,   431,   519,   399,   362,
     838,   339,    67,   481,   186,  1815,  -657,  -657,   497,   497,
    1815,  1815,  -657,   509,   398,  -657,   487,  1815,  -657,  -657,
    -657,   315,   315,  -657,   493,  1815,  -657,  -657,   331,  1595,
    -657,   489,   515,   521,  -657,  -657,  -657,  1595,  -657,  -657,
    1815,  1815,   522,   524,   963,  1815,   729,  -657,   526,   963,
     486,    92,   200,  -657,  -657,  -657,   409,   409,   459,  -657,
     537,   532,  -657,   551,  -657,  -657,  -657,  -657,   470,   502,
     477,   485,   350,  -657,  -657,  -657,   963,  -657,  1815,  -657,
    -657,   492,  1815,   505,  1815,  -657,  -657,   179,  -657,  -657,
    -657,   278,   181,   308,  1815,  -657,  -657,  -657,  1815,  -657,
    -657,  -657,  -657,  1815,  1815,  1815,  -657,  -657,  -657,   339,
    -657,  -657,  -657,   497,   500,  -657,  -657,  -657,   497,  1595,
    1595,   351,  -657,  -657,  1815,  -657,  -657,  -657,  -657,  1815,
    -657,  -657,   498,  -657,  1815,  -657,  -657,  -657,  -657,  -657,
    -657,   520,  -657,   583,  -657,  -657,  -657,  1485,  1815,  1815,
    -657,   181,   253,  -657,   356,   581,   588,    68,   553,   612,
    1815,  -657,    86,  -657,  -657,   290,  -657,  -657,   451,   612,
    -657,   200,  -657,  -657,   409,  -657,  -657,    35,  -657,  -657,
     210,   179,   600,   350,   350,   350,  -657,  -657,  -657,  -657,
    -657,  1705,  -657,   362,   473,   510,    64,  -657,  -657,   362,
    1815,   601,  -657,  -657,  -657,  -657,  -657,  -657,  1815,  1815,
    1815,  -657,  -657,   497,  1815,  -657,  -657,  -657,  -657,  -657,
    -657,  -657,  -657,  -657,  -657,  -657,  -657,  1815,   556,   533,
    -657,  -657,  -657,  -657,  -657,  1815,   606,  -657,  -657,  -657,
    -657,  1815,  1815,  -657,  -657,  1815,   865,   914,  -657,  -657,
     528,  -657,   181,   610,   253,   253,   356,   409,  -657,  -657,
    -657,  -657,  -657,   539,    68,  -657,   297,   563,  -657,  -657,
     612,  -657,  -657,  -657,   179,   300,   259,   325,  -657,  -657,
    -657,  -657,   542,  -657,   200,  -657,   200,   200,  -657,  -657,
     580,   582,  1815,   228,  -657,  -657,  -657,  -657,  -657,  -657,
    -657,  -657,  -657,   519,   179,  -657,  -657,  -657,  -657,    21,
     613,   570,  -657,  -657,   963,   436,   631,  -657,   726,   633,
    -657,   634,  -657,   562,  1815,  -657,  1815,  -657,  1815,  -657,
    -657,  -657,  -657,   409,   637,   409,  -657,  -657,   325,  -657,
     590,  -657,  -657,  -657,  -657,   200,   200,   612,   612,  -657,
    1815,  -657,  -657,  -657,   134,   134,   640,   179,   134,  -657,
    -657,   641,   228,  -657,   308,   278,  1815,  -657,  -657,   179,
    -657,  -657,  -657,  -657,  -657,   566,    78,   726,   663,  -657,
     418,  -657,  -657,  -657,  -657,   963,   963,   963,  -657,   647,
    -657,   649,  -657,  -657,   325,   409,   409,   409,  -657,  -657,
    -657,  -657,   578,   654,   656,  -657,   179,  -657,  -657,  -657,
    -657,  -657,  -657,  -657,    81,   585,  -657,  -657,  -657,  -657,
    -657,  -657,  -657,  -657,  -657,  -657,   502,   589,   584,   586,
    -657,  -657,  -657,  -657,  -657,  -657,   663,  -657,  -657,  1815,
     409,  -657,  -657,  -657,  -657,   963,  -657,   200,   200,   200,
    -657,  -657,  -657,  -657,  -657,  -657
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -657,  -657,   645,    20,  -657,   619,  -657,     6,   577,  -390,
     -68,   514,  -657,  -657,  -657,   120,  -657,   -32,  -268,   639,
    -221,  -657,  -489,    75,  -297,  -657,  -657,  -657,  -657,  -657,
    -657,   615,  -172,   304,  -657,  -657,  -657,  -657,  -657,  -215,
     379,  -568,  -411,  -371,  -657,  -266,   267,    33,   139,  -344,
    -657,  -162,  -657,   632,    30,  -657,  -402,   146,  -657,  -220,
    -657,  -657,  -657,  -657,     7,  -657,   -28,   -49,  -657,  -657,
     476,  -657,   -53,   472,   491,   353,    73,  -161,  -657,  -180,
    -657,   155,   -15,  -657,  -210,  -657,  -556,  -184,  -555,    34,
    -657,  -657,   -65,  -657,   147,  -657,   -11,   664,  -441,  -657,
    -657,  -657,  -211,   -38,   -22,  -657,  -657,  -657,  -657,  -657,
    -657,  -657,  -267,  -657,   -24,  -540,   -99,  -353,  -403,   457,
    -424,   142,  -657,  -232,   233,  -657,  -342,   240,  -657,  -468,
    -657,  -560,  -657,  -341,  -657,  -657,  -657,  -657,  -657,  -657,
     136,   465,  -656
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -299
static const yytype_int16 yytable[] =
{
      83,    84,    86,    87,    88,   108,    59,   198,    89,    90,
      92,    93,    94,    95,    96,    97,    98,   339,   327,    99,
     536,   109,   100,   410,   411,   101,   102,   103,   529,   110,
     111,   112,   113,   114,   295,   428,   483,   117,   118,   119,
     494,   620,   300,   182,   208,   209,   210,   645,   148,   149,
      12,   594,   326,   326,   401,   333,   555,   687,   506,   510,
     425,   426,   390,   120,    59,   183,   133,   430,   595,   543,
     435,   436,     6,   393,   412,     9,    10,   699,   700,   123,
     768,   769,     6,    12,   691,   443,   692,   693,   161,    12,
     715,   597,   167,   170,   171,   156,    12,   755,   177,   178,
       6,   444,   171,   121,   446,   578,   419,    12,   158,   216,
     217,   218,   219,   220,   137,   448,   106,   221,   733,   222,
     223,   224,   137,   225,   786,   536,   137,   226,   227,   228,
     229,   230,   231,    43,   463,   234,   235,   236,   237,   238,
     596,   326,   679,   262,   406,   738,   699,   700,   628,   715,
     647,   653,   333,   594,   594,    72,   605,   311,   312,   313,
     314,   315,   316,   317,   495,   496,   506,   510,   457,   457,
     595,   595,   340,   542,   765,   239,   192,   600,   601,   644,
     493,   159,   204,   205,   104,   133,   756,   456,   456,     6,
     713,   714,   215,   673,    12,   240,   297,   472,   140,   141,
     142,   194,   143,   144,   241,   481,   629,   367,   142,   577,
     143,   144,   538,    72,   143,   144,   751,   200,     6,   739,
     740,   201,   282,   202,   628,    12,   345,   250,   347,   348,
     349,   350,   351,    77,    78,   294,   357,   642,   483,   166,
     328,   251,   211,   338,   366,   166,   108,   790,   791,   792,
     625,   333,   212,   374,   375,   376,   356,   378,   379,   380,
     167,   361,   109,   364,   244,   641,   171,     6,   394,   446,
     697,   646,   372,   457,   447,   395,   363,   232,   457,   355,
     448,   413,   629,   245,   242,   246,     6,   556,   557,     9,
      10,   233,   456,   198,   536,   243,   196,   456,   197,   247,
     571,   698,   521,   414,   252,   325,   325,   254,   332,   337,
     150,   337,   340,   635,   636,   637,   639,   151,     2,   429,
       3,     4,     5,    12,   334,   256,   258,   152,   424,    43,
     255,   285,   195,     6,   394,   153,   741,   461,   607,   154,
      12,   395,   257,    18,   466,   196,   259,   197,   607,   260,
     311,   326,   261,   108,   333,   473,   333,   392,   397,     6,
     558,   266,   407,   407,   407,   674,    12,   559,   299,   109,
     438,   439,   440,   457,   441,   340,   340,  -299,   132,   764,
    -247,   467,   468,   265,   498,   770,   649,   771,   651,   560,
     561,   150,   456,   196,   325,   197,   105,  -297,   151,   268,
     526,  -297,   528,  -297,   269,   332,   270,   681,   152,   618,
     362,   682,   621,   683,   562,   563,   153,   564,   565,   271,
     154,   272,   540,   520,   521,    92,   273,   544,   150,   274,
     533,   548,   283,   550,   285,   151,   279,   793,   794,   795,
     539,   729,   175,   731,   296,   152,   434,   572,   352,   607,
     175,    12,   707,   153,   359,   191,   365,   154,   370,     6,
     394,    92,   381,   579,   392,   580,    12,   395,   566,   399,
     583,   584,   311,   503,   404,   589,   545,   546,   721,   415,
     340,     6,   340,   750,     9,    10,   614,   417,    12,   615,
     616,   137,  -157,   133,   453,   458,   132,   622,   499,  -156,
     534,  -299,  -299,   131,   132,     6,   337,   677,     9,    10,
     743,   744,    12,   420,   340,   638,  -155,   640,   742,   470,
     474,   475,   476,   422,   477,   333,   607,   607,   175,   671,
     672,   500,  -152,  -153,   617,   450,   652,   721,   431,  -154,
     427,   432,   445,   191,   462,     6,   394,   196,   464,   197,
     479,   655,    12,   395,   469,   650,   480,   485,  -298,   486,
     451,   490,  -298,   452,  -298,   491,   497,   513,   501,   514,
     602,   299,   606,   138,   139,   140,   141,   142,   515,   143,
     144,   503,   606,   516,   499,   324,   324,   677,   331,   518,
     407,   127,   128,   631,   633,   129,   130,   131,   132,   519,
     530,   582,   549,   688,   325,   128,   689,   332,   129,   130,
     131,   132,   575,   532,    54,   554,   598,   500,   643,   703,
       6,   394,   125,   599,   126,   127,   128,    12,   395,   129,
     130,   131,   132,   500,   581,   634,   648,   105,   396,   657,
     658,   660,   709,   668,  -298,   670,   678,   675,  -298,   690,
    -298,   126,   127,   128,   501,   728,   129,   130,   131,   132,
     706,   164,   732,   694,   448,   695,   708,   164,   722,   723,
     724,   179,   730,   734,   324,   745,   748,   602,   187,   188,
     758,   754,   762,   606,   763,   331,   772,   680,   206,   773,
     686,   774,   500,   778,   213,   214,   782,   781,   162,   160,
     783,   455,   455,   122,   280,   663,   423,   760,   761,   180,
     181,   155,   752,   184,   185,   186,   551,   705,   704,  -298,
     527,   662,   176,  -298,   757,  -298,   776,   784,   215,    54,
     360,   720,   368,   346,     6,   775,   749,     9,    10,   669,
     157,    12,   780,   779,   105,   405,   676,   623,   619,   488,
     398,   686,   685,   502,     0,     0,     0,     0,     0,     0,
     606,   606,     0,     0,   710,     0,     0,   188,     0,     0,
     746,     0,     0,     0,     0,     0,     0,   337,     0,     0,
       0,     0,   753,   206,   331,   537,   137,     0,     0,   711,
     720,   206,   712,   301,   302,   303,   304,   305,   306,   307,
     308,   309,     0,     0,     0,     0,   455,   686,     0,   321,
     321,   455,   124,   125,   570,   126,   127,   128,     0,   746,
     129,   130,   131,   132,     0,     0,     0,   777,     0,     0,
     -47,     0,     0,     0,     0,   713,   714,     0,   369,     0,
       0,     0,     0,     0,   590,   489,     0,     0,   377,     0,
       0,   137,   502,     0,   384,   386,     0,   162,   389,   -47,
       0,   502,   502,     0,   -47,     0,     0,   358,   418,   139,
     140,   141,   142,   630,   143,   144,   416,     0,     0,     0,
     371,     0,   373,     0,   324,     0,     0,   331,     0,   331,
       0,   383,     0,     0,   388,     0,     0,     0,   321,     0,
     -47,   -47,     0,   -47,   -47,   -47,   455,     0,   -47,   -47,
     -47,   -47,   -47,   -47,   -47,   -47,   -47,     0,   -47,   -47,
       0,     0,     0,     0,     0,     0,     0,   459,   460,     0,
       0,     0,   281,   437,   139,   140,   141,   142,     0,   143,
     144,     0,   277,     0,     0,   590,   206,   278,   124,   125,
       0,   126,   127,   128,   206,     0,   129,   130,   131,   132,
     449,     0,   487,   502,     0,     0,     0,     0,     0,   684,
     502,     0,   465,     0,     0,     0,     0,     0,     0,     0,
     471,   664,     0,   124,   125,     0,   126,   127,   128,     0,
       0,   129,   130,   131,   132,     0,   484,   124,   125,   531,
     126,   127,   128,   276,     0,   129,   130,   131,   132,     0,
       0,   719,     0,     0,     0,     0,   124,   125,     0,   126,
     127,   128,     0,     0,   129,   130,   131,   132,     0,     0,
     666,   502,     0,     0,     0,     0,   206,   206,     0,     0,
     502,   502,     0,     0,     0,     0,   124,   125,     0,   126,
     127,   128,     0,   541,   129,   130,   131,   132,   331,     0,
     547,     0,     0,     0,     0,   586,   587,     0,     0,     0,
     719,     0,     0,     0,     0,     0,     0,     0,     0,   573,
       0,     0,     0,     0,   574,     0,     0,   502,     0,   576,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   321,     0,
       0,     0,     0,     0,     0,   613,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   654,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       1,     0,     2,     0,     3,     4,     5,     6,     7,     8,
       9,    10,   587,    11,    12,    13,     0,     0,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,  -253,    19,
       0,     0,   656,     0,     0,  -253,    20,    21,    22,     0,
     659,     0,    23,    24,    25,  -253,   661,    26,    27,     0,
      28,     0,    29,  -253,    30,     0,    31,  -253,    32,    33,
       0,    34,    35,    36,    37,    38,    39,    40,    41,     0,
     -12,     0,     0,     0,    42,    43,    44,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    45,
       0,   725,     0,   726,     0,   727,     0,   696,     0,    46,
       0,     0,    47,    48,     0,     0,    49,     0,    50,    51,
      52,     0,    53,     0,     0,     0,    54,   206,     2,     0,
       3,     4,     5,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,     0,    19,     0,     0,     0,     0,
       0,     0,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
      30,     0,    31,     0,    32,    33,     0,    34,    35,    36,
      37,    38,    39,    40,    41,     0,   -12,     0,     0,     0,
      42,    43,    44,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    45,   785,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,     0,    47,    48,
       0,     0,    49,     0,    50,    51,    52,     0,    53,     0,
       0,     0,    54,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,    17,     0,     0,    19,     0,     0,     0,     0,
       0,     0,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
      30,     0,    31,     0,    32,    33,   267,    34,    35,    36,
      37,    38,    39,    40,    41,     0,     0,     0,     0,     0,
      42,     0,    44,     0,     0,     0,     0,     0,   124,   125,
       0,   126,   127,   128,     0,    45,   129,   130,   131,   132,
       0,     0,     0,     0,     0,    46,     0,     0,    47,    48,
       0,     0,    49,     0,    50,    51,    52,     0,    53,     0,
       0,     0,    54,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,    17,     0,     0,    19,     0,     0,     0,     0,
       0,     0,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
      30,     0,    31,     0,    32,    33,     0,    34,    35,    36,
      37,    38,    39,    40,    41,     0,     0,     0,     0,     0,
      42,     0,    44,     0,     0,     0,     0,     0,   124,   125,
       0,   126,   127,   128,     0,    45,   129,   130,   131,   132,
       0,     0,     0,     0,     0,    46,     0,     0,    47,    48,
       0,     0,    49,     0,    50,    51,    52,     0,    53,     0,
       0,   585,    54,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,    17,     0,     0,    19,     0,     0,     0,     0,
       0,     0,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
      30,     0,    31,     0,    32,    33,     0,    34,    35,    36,
      37,    38,    39,    40,    41,     0,     0,     0,     0,     0,
      42,     0,    44,     0,     0,     0,     0,     0,   124,   125,
       0,   126,   127,   128,     0,    45,   129,   130,   131,   132,
       0,     0,     0,     0,     0,    46,     0,     0,    47,    48,
       0,     0,    49,     0,    50,    51,    52,     0,    53,     0,
       0,     0,    54,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,    17,     0,     0,    19,     0,     0,     0,     0,
       0,     0,    20,   318,    22,     0,     0,     0,    23,    24,
      25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
      30,     0,    31,     0,    32,    33,     0,    34,   319,    36,
      37,   320,    39,    40,    41,     0,     0,     0,     0,     0,
      42,     0,    44,     0,     0,     0,     0,     0,   124,   125,
       0,   126,   127,   128,     0,    45,   129,   130,   131,   132,
       0,     0,     0,     0,     0,    46,     0,     0,    47,    48,
       0,     0,    49,     0,    50,    51,    52,     0,    53,     0,
       0,     0,    54,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,    17,     0,     0,    19,     0,     0,     0,     0,
       0,     0,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
      30,     0,    31,     0,    32,    33,     0,    34,    35,    36,
      37,    38,    39,    40,    41,     0,     0,     0,     0,     0,
      42,     0,    44,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,     0,    47,    48,
       0,     0,    49,     0,    50,    51,    52,     0,    53,     0,
       0,     0,    54,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,    17,     0,     0,    19,     0,     0,     0,     0,
       0,     0,    20,   318,    22,     0,     0,     0,    23,    24,
      25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
      30,     0,    31,     0,    32,    33,     0,    34,   319,    36,
      37,   320,    39,    40,    41,     0,     0,     0,     0,     0,
      42,     0,    44,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,     0,    47,    48,
       0,     0,    49,     0,    50,    51,    52,     0,    53,     0,
       0,     0,    54,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,    17,     0,     0,    19,     0,     0,     0,     0,
       0,     0,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
      30,     0,    31,     0,    32,    33,     0,    34,    35,    36,
      37,    38,    39,    40,    41,     0,     0,     0,     0,     0,
      42,     0,    44,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,     0,    47,    48,
       0,     0,   385,     0,    50,    51,    52,     0,    53,     0,
       0,     0,    54
};

static const yytype_int16 yycheck[] =
{
      15,    16,    17,    18,    19,    43,     0,   106,    23,    24,
      25,    26,    27,    28,    29,    30,    31,   238,   233,    34,
     431,    43,    37,   290,   291,    40,    41,    42,   418,    44,
      45,    46,    47,    48,   206,   332,   380,    52,    53,    54,
     393,   509,   214,    96,   112,   113,   114,   536,    63,    64,
      15,   492,   232,   233,   286,   235,   458,   617,   400,   400,
     328,   329,   282,     0,    58,    97,    60,   335,   492,   440,
     338,   339,     8,   284,   294,    11,    12,   633,   633,    59,
     736,   737,     8,    15,   624,    18,   626,   627,    82,    15,
     658,   494,    86,    87,    88,    75,    15,    19,    92,    93,
       8,    34,    96,    71,    83,   476,   321,    15,     4,   124,
     125,   126,   127,   128,    13,    94,    43,   132,   678,   134,
     135,   136,    13,   138,   780,   536,    13,   142,   143,   144,
     145,   146,   147,    76,   354,   150,   151,   152,   153,   154,
     493,   321,   610,   175,   109,   685,   702,   702,    14,   717,
     540,   553,   332,   594,   595,     0,   498,   225,   226,   227,
     228,   229,   230,   231,   396,   397,   508,   508,   348,   349,
     594,   595,   240,   439,   734,   155,   103,   109,   110,   115,
     391,     4,   109,   110,   110,   179,   108,   348,   349,     8,
     109,   110,   119,   596,    15,    52,   211,   369,    97,    98,
      99,   115,   101,   102,    61,   377,    72,   260,    99,   475,
     101,   102,   433,    58,   101,   102,   705,   107,     8,   687,
     688,   111,   192,   113,    14,    15,   241,    47,   243,   244,
     245,   246,   247,   105,   106,   205,   251,   534,   582,    84,
     234,    61,    47,   237,   259,    90,   284,   787,   788,   789,
     517,   431,   108,   268,   269,   270,   250,   272,   273,   274,
     254,   255,   284,   257,    21,   533,   260,     8,     9,    83,
      42,   539,   266,   453,    88,    16,   256,   107,   458,   249,
      94,   296,    72,    40,    18,    42,     8,   459,   460,    11,
      12,   107,   453,   392,   705,    35,    96,   458,    98,    56,
     461,    73,    74,   297,    31,   232,   233,    18,   235,   236,
      29,   238,   380,   523,   524,   525,   531,    36,     3,   334,
       5,     6,     7,    15,    16,   170,   171,    46,    47,    76,
      47,    78,    83,     8,     9,    54,   689,   352,   499,    58,
      15,    16,    47,    28,   359,    96,    35,    98,   509,    18,
     418,   531,   113,   391,   534,   370,   536,   284,   285,     8,
       9,    47,   289,   290,   291,   597,    15,    16,   213,   391,
      31,    32,    33,   553,    35,   443,   444,    93,    94,   732,
      83,   361,   362,    61,   399,   738,   548,   740,   550,    38,
      39,    29,   553,    96,   321,    98,    43,   107,    36,    35,
     415,   111,   417,   113,    35,   332,    35,   107,    46,   508,
     255,   111,   511,   113,    63,    64,    54,    66,    67,   113,
      58,    35,   437,    73,    74,   440,    35,   442,    29,    24,
     424,   446,    83,   448,    78,    36,    70,   790,   791,   792,
     434,   673,    89,   675,    89,    46,    47,   462,    50,   610,
      97,    15,    16,    54,    35,   102,    61,    58,    35,     8,
       9,   476,    41,   478,   391,   480,    15,    16,   117,    60,
     485,   486,   540,   400,    47,   490,   443,   444,   658,   112,
     548,     8,   550,   704,    11,    12,   501,   114,    15,   504,
     505,    13,   115,   487,   348,   349,    94,   512,    47,   115,
     427,    91,    92,    93,    94,     8,   433,   606,    11,    12,
     694,   695,    15,    19,   582,   530,   115,   532,   690,   364,
      31,    32,    33,   115,    35,   705,   687,   688,   175,   594,
     595,    80,   115,   115,    83,    38,   551,   717,   107,   115,
     115,    22,    61,   190,    35,     8,     9,    96,    61,    98,
      35,   566,    15,    16,    61,   549,    35,    35,   107,    35,
      63,    35,   111,    66,   113,    79,   107,    35,   117,    18,
     497,   416,   499,    95,    96,    97,    98,    99,   108,   101,
     102,   508,   509,    81,    47,   232,   233,   686,   235,   112,
     517,    87,    88,   520,   521,    91,    92,    93,    94,   114,
     108,    18,   447,   618,   531,    88,   621,   534,    91,    92,
      93,    94,   114,   108,   117,   115,    35,    80,   108,   634,
       8,     9,    84,    35,    86,    87,    88,    15,    16,    91,
      92,    93,    94,    80,   114,    35,    35,   284,   285,    83,
     107,    35,   657,   115,   107,    35,    83,   108,   111,   107,
     113,    86,    87,    88,   117,   670,    91,    92,    93,    94,
      47,    84,   677,    83,    94,    83,    35,    90,    35,    35,
     108,    94,    35,    83,   321,    35,    35,   604,   101,   102,
      17,   115,    35,   610,    35,   332,   108,   614,   111,    35,
     617,    35,    80,   108,   117,   118,   112,   108,    83,    80,
     114,   348,   349,    58,   190,   585,   327,   722,   723,    94,
      95,    72,   706,    98,    99,   100,   449,   644,   643,   107,
     416,   582,    90,   111,   717,   113,   754,   776,   655,   117,
     254,   658,   260,   242,     8,   746,   702,    11,    12,   592,
      76,    15,   766,   758,   391,   288,   604,   514,   508,    20,
     285,   678,   616,   400,    -1,    -1,    -1,    -1,    -1,    -1,
     687,   688,    -1,    -1,    38,    -1,    -1,   190,    -1,    -1,
     697,    -1,    -1,    -1,    -1,    -1,    -1,   704,    -1,    -1,
      -1,    -1,   709,   206,   431,   432,    13,    -1,    -1,    63,
     717,   214,    66,   216,   217,   218,   219,   220,   221,   222,
     223,   224,    -1,    -1,    -1,    -1,   453,   734,    -1,   232,
     233,   458,    83,    84,   461,    86,    87,    88,    -1,   746,
      91,    92,    93,    94,    -1,    -1,    -1,   754,    -1,    -1,
      13,    -1,    -1,    -1,    -1,   109,   110,    -1,   261,    -1,
      -1,    -1,    -1,    -1,   491,   116,    -1,    -1,   271,    -1,
      -1,    13,   499,    -1,   277,   278,    -1,   242,   281,    42,
      -1,   508,   509,    -1,    47,    -1,    -1,   252,    95,    96,
      97,    98,    99,   520,   101,   102,   299,    -1,    -1,    -1,
     265,    -1,   267,    -1,   531,    -1,    -1,   534,    -1,   536,
      -1,   276,    -1,    -1,   279,    -1,    -1,    -1,   321,    -1,
      83,    84,    -1,    86,    87,    88,   553,    -1,    91,    92,
      93,    94,    95,    96,    97,    98,    99,    -1,   101,   102,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   350,   351,    -1,
      -1,    -1,   115,    95,    96,    97,    98,    99,    -1,   101,
     102,    -1,    42,    -1,    -1,   592,   369,    47,    83,    84,
      -1,    86,    87,    88,   377,    -1,    91,    92,    93,    94,
     345,    -1,   385,   610,    -1,    -1,    -1,    -1,    -1,   616,
     617,    -1,   357,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     365,   116,    -1,    83,    84,    -1,    86,    87,    88,    -1,
      -1,    91,    92,    93,    94,    -1,   381,    83,    84,   422,
      86,    87,    88,    70,    -1,    91,    92,    93,    94,    -1,
      -1,   658,    -1,    -1,    -1,    -1,    83,    84,    -1,    86,
      87,    88,    -1,    -1,    91,    92,    93,    94,    -1,    -1,
     116,   678,    -1,    -1,    -1,    -1,   459,   460,    -1,    -1,
     687,   688,    -1,    -1,    -1,    -1,    83,    84,    -1,    86,
      87,    88,    -1,   438,    91,    92,    93,    94,   705,    -1,
     445,    -1,    -1,    -1,    -1,   488,   489,    -1,    -1,    -1,
     717,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   464,
      -1,    -1,    -1,    -1,   469,    -1,    -1,   734,    -1,   474,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   531,    -1,
      -1,    -1,    -1,    -1,    -1,   500,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   554,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       1,    -1,     3,    -1,     5,     6,     7,     8,     9,    10,
      11,    12,   585,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    -1,    25,    26,    27,    28,    29,    30,
      -1,    -1,   567,    -1,    -1,    36,    37,    38,    39,    -1,
     575,    -1,    43,    44,    45,    46,   581,    48,    49,    -1,
      51,    -1,    53,    54,    55,    -1,    57,    58,    59,    60,
      -1,    62,    63,    64,    65,    66,    67,    68,    69,    -1,
      71,    -1,    -1,    -1,    75,    76,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,
      -1,   664,    -1,   666,    -1,   668,    -1,   632,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,   107,    -1,   109,   110,
     111,    -1,   113,    -1,    -1,    -1,   117,   690,     3,    -1,
       5,     6,     7,     8,     9,    10,    11,    12,    -1,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,
      25,    26,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,
      45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,
      55,    -1,    57,    -1,    59,    60,    -1,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    71,    -1,    -1,    -1,
      75,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    90,   779,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,
      -1,    -1,   117,     8,     9,    10,    11,    12,    -1,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,
      25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,
      45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,
      55,    -1,    57,    -1,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,
      75,    -1,    77,    -1,    -1,    -1,    -1,    -1,    83,    84,
      -1,    86,    87,    88,    -1,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,
      -1,    -1,   117,     8,     9,    10,    11,    12,    -1,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,
      25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,
      45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,
      55,    -1,    57,    -1,    59,    60,    -1,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,
      75,    -1,    77,    -1,    -1,    -1,    -1,    -1,    83,    84,
      -1,    86,    87,    88,    -1,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,
      -1,   116,   117,     8,     9,    10,    11,    12,    -1,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,
      25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,
      45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,
      55,    -1,    57,    -1,    59,    60,    -1,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,
      75,    -1,    77,    -1,    -1,    -1,    -1,    -1,    83,    84,
      -1,    86,    87,    88,    -1,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,
      -1,    -1,   117,     8,     9,    10,    11,    12,    -1,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,
      25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,
      45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,
      55,    -1,    57,    -1,    59,    60,    -1,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,
      75,    -1,    77,    -1,    -1,    -1,    -1,    -1,    83,    84,
      -1,    86,    87,    88,    -1,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,
      -1,    -1,   117,     8,     9,    10,    11,    12,    -1,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,
      25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,
      45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,
      55,    -1,    57,    -1,    59,    60,    -1,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,
      75,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,
      -1,    -1,   117,     8,     9,    10,    11,    12,    -1,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,
      25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,
      45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,
      55,    -1,    57,    -1,    59,    60,    -1,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,
      75,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,
      -1,    -1,   117,     8,     9,    10,    11,    12,    -1,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,
      25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,
      45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,
      55,    -1,    57,    -1,    59,    60,    -1,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,
      75,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,
      -1,    -1,   117
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     1,     3,     5,     6,     7,     8,     9,    10,    11,
      12,    14,    15,    16,    23,    25,    26,    27,    28,    30,
      37,    38,    39,    43,    44,    45,    48,    49,    51,    53,
      55,    57,    59,    60,    62,    63,    64,    65,    66,    67,
      68,    69,    75,    76,    77,    90,   100,   103,   104,   107,
     109,   110,   111,   113,   117,   119,   120,   121,   122,   125,
     126,   128,   154,   155,   156,   170,   193,   194,   195,   196,
     197,   198,   199,   201,   209,   215,   216,   105,   106,   123,
     124,   163,   200,   200,   200,   186,   200,   200,   200,   200,
     200,   161,   200,   200,   200,   200,   200,   200,   200,   200,
     200,   200,   200,   200,   110,   193,   194,   220,   221,   222,
     200,   200,   200,   200,   200,   125,   149,   200,   200,   200,
       0,    71,   120,   121,    83,    84,    86,    87,    88,    91,
      92,    93,    94,   125,   143,   144,   145,    13,    95,    96,
      97,    98,    99,   101,   102,   146,   147,   148,   200,   200,
      29,    36,    46,    54,    58,   137,   121,   215,     4,     4,
     123,   125,   149,   192,   126,   171,   199,   125,   188,   189,
     125,   125,   190,   191,   135,   193,   171,   125,   125,   126,
     149,   149,   190,   135,   149,   149,   149,   126,   126,   129,
     130,   193,   194,   218,   115,    83,    96,    98,   234,   235,
     107,   111,   113,   223,   194,   194,   126,   150,   128,   128,
     128,    47,   108,   126,   126,   194,   200,   200,   200,   200,
     200,   200,   200,   200,   200,   200,   200,   200,   200,   200,
     200,   200,   107,   107,   200,   200,   200,   200,   200,   121,
      52,    61,    18,    35,    21,    40,    42,    56,   172,   173,
      47,    61,    31,   187,    18,    47,   199,    47,   199,    35,
      18,   113,   135,   136,   137,    61,    47,    61,    35,    35,
      35,   113,    35,    35,    24,   152,    70,    42,    47,    70,
     129,   115,   172,    83,   217,    78,   236,   237,   238,   224,
     226,   228,   233,   234,   172,   150,    89,   200,   151,   199,
     150,   126,   126,   126,   126,   126,   126,   126,   126,   126,
     127,   128,   128,   128,   128,   128,   128,   128,    38,    63,
      66,   126,   157,   159,   193,   194,   197,   157,   125,   142,
     160,   193,   194,   197,    16,   138,   139,   194,   125,   138,
     128,   165,   167,   168,   169,   200,   192,   200,   200,   200,
     200,   200,    50,   177,   178,   172,   125,   200,   149,    35,
     188,   125,   199,   121,   125,    61,   200,   190,   191,   126,
      35,   149,   125,   149,   200,   200,   200,   126,   200,   200,
     200,    41,   153,   149,   126,   107,   126,   131,   149,   126,
     177,   219,   194,   220,     9,    16,   193,   194,   259,    60,
     199,   241,   242,   243,    47,   237,   109,   194,   230,   231,
     230,   230,   177,   200,   125,   112,   126,   114,    95,   157,
      19,   158,   115,   158,    47,   136,   136,   115,   142,   200,
     136,   107,    22,   141,    47,   136,   136,    95,    31,    32,
      33,    35,   164,    18,    34,    61,    83,    88,    94,   149,
      38,    63,    66,   175,   176,   193,   195,   197,   175,   126,
     126,   200,    35,   177,    61,   149,   200,   121,   121,    61,
     199,   149,   150,   200,    31,    32,    33,    35,   162,    35,
      35,   150,   166,   167,   149,    35,    35,   126,    20,   116,
      35,    79,   211,   220,   235,   241,   241,   107,   200,    47,
      80,   117,   193,   194,   221,   222,   244,   245,   246,   250,
     251,   258,   259,    35,    18,   108,    81,   232,   112,   114,
      73,    74,   202,   203,   204,   206,   200,   151,   200,   127,
     108,   126,   108,   125,   194,   140,   160,   193,   138,   125,
     200,   149,   163,   161,   200,   165,   165,   149,   200,   199,
     200,   164,   174,   175,   115,   174,   150,   150,     9,    16,
      38,    39,    63,    64,    66,    67,   117,   179,   180,   181,
     193,   195,   200,   149,   149,   114,   149,   163,   161,   200,
     200,   114,    18,   200,   200,   116,   126,   126,   133,   200,
     193,   212,   213,   210,   216,   238,   235,   236,    35,    35,
     109,   110,   194,   239,   240,   244,   194,   195,   244,   247,
     248,   249,   251,   149,   200,   200,   200,    83,   234,   245,
     247,   234,   200,   242,   225,   230,   227,   229,    14,    72,
     193,   194,   205,   194,    35,   202,   202,   202,   200,   157,
     200,   136,   142,   108,   115,   140,   136,   127,    35,   169,
     125,   169,   200,   174,   126,   200,   149,    83,   107,   149,
      35,   149,   166,   133,   116,   132,   116,   134,   115,   212,
      35,   210,   210,   236,   241,   108,   239,   234,    83,   247,
     194,   107,   111,   113,   193,   258,   194,   249,   200,   200,
     107,   233,   233,   233,    83,    83,   149,    42,    73,   204,
     206,   207,   208,   200,   141,   194,    47,    16,    35,   200,
      38,    63,    66,   109,   110,   159,   182,   183,   184,   193,
     194,   197,    35,    35,   108,   126,   126,   126,   200,   241,
      35,   241,   200,   249,    83,   252,   254,   256,   233,   247,
     247,   235,   150,   205,   205,    35,   194,   214,    35,   207,
     138,   140,   125,   194,   115,    19,   108,   182,    17,   185,
     200,   200,    35,    35,   235,   249,   241,   260,   260,   260,
     235,   235,   108,    35,    35,   214,   184,   194,   108,   200,
     232,   108,   112,   114,   185,   126,   260,   253,   255,   257,
     233,   233,   233,   235,   235,   235
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

    { yyoutput = (yyvsp[(1) - (2)].t); YYACCEPT; }
    break;

  case 3:

    { yyoutput = PA_parseError; YYABORT; }
    break;

  case 4:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 5:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fSynTopLevelProductionTemplates,
					   (yyvsp[(1) - (2)].t)),(yyvsp[(2) - (2)].t)); }
    break;

  case 6:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 7:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fFunctor,newCTerm(PA_fDollar,(yyvsp[(1) - (3)].t)),
					   (yyvsp[(2) - (3)].t),(yyvsp[(1) - (3)].t)),(yyvsp[(3) - (3)].t)); }
    break;

  case 8:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 9:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fDeclare,(yyvsp[(3) - (6)].t),newCTerm(PA_fSkip,(yyvsp[(5) - (6)].t)),
					   (yyvsp[(2) - (6)].t)),(yyvsp[(6) - (6)].t)); }
    break;

  case 10:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fDeclare,(yyvsp[(3) - (6)].t),(yyvsp[(5) - (6)].t),(yyvsp[(2) - (6)].t)),(yyvsp[(6) - (6)].t)); }
    break;

  case 11:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fDeclare,(yyvsp[(3) - (5)].t),
					   newCTerm(PA_fSkip,(yyvsp[(4) - (5)].t)),(yyvsp[(2) - (5)].t)),(yyvsp[(5) - (5)].t)); }
    break;

  case 12:

    { (yyval.t) = AtomNil; }
    break;

  case 13:

    { (yyval.t) = newCTerm(PA_dirSwitch,(yyvsp[(2) - (2)].t)); }
    break;

  case 14:

    { (yyval.t) = PA_dirLocalSwitches; }
    break;

  case 15:

    { (yyval.t) = PA_dirPushSwitches; }
    break;

  case 16:

    { (yyval.t) = PA_dirPopSwitches; }
    break;

  case 17:

    { (yyval.t) = AtomNil; }
    break;

  case 18:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 19:

    { if (!strcmp(xytext,"gump"))
		      xy_gumpSyntax = 1;
		    if (!strcmp(xytext,"allowdeprecated"))
		      xy_allowDeprecated = 1;
		    if (!strcmp(xytext,"allowwhileloop"))
		      xy_allowWhileLoop = 1;
		    (yyval.t) = newCTerm(PA_on,OZ_atom(xytext),pos());
		  }
    break;

  case 20:

    { if (!strcmp(xytext,"gump"))
		      xy_gumpSyntax = 0;
		    if (!strcmp(xytext,"allowdeprecated"))
		      xy_allowDeprecated = 0;
		    if (!strcmp(xytext,"allowwhileloop"))
		      xy_allowWhileLoop = 0;
		    (yyval.t) = newCTerm(PA_off,OZ_atom(xytext),pos());
		  }
    break;

  case 21:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 22:

    { (yyval.t) = newCTerm(PA_fAnd,(yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 23:

    { (yyval.t) = newCTerm(PA_fEq,(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 24:

    { if (oz_isSRecord((yyvsp[(1) - (4)].t)) && 
                        oz_eq(OZ_label((yyvsp[(1) - (4)].t)), PA_fOpApply) && 
                        oz_eq(OZ_getArg((yyvsp[(1) - (4)].t),0), AtomDot)) {
                       (yyval.t) = newCTerm(PA_fDotAssign,(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t));
			}
                    else
                       (yyval.t) = newCTerm(PA_fColonEquals,(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 25:

    { (yyval.t) = newCTerm(PA_fAssign,(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 26:

    { (yyval.t) = newCTerm(PA_fOrElse,(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 27:

    { (yyval.t) = newCTerm(PA_fAndThen,(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 28:

    { (yyval.t) = newCTerm(PA_fOpApply,(yyvsp[(2) - (4)].t),
				  oz_mklistUnwrap((yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t)),(yyvsp[(3) - (4)].t)); }
    break;

  case 29:

    { (yyval.t) = newCTerm(PA_fFdCompare,(yyvsp[(2) - (4)].t),(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 30:

    { (yyval.t) = newCTerm(PA_fFdIn,(yyvsp[(2) - (4)].t),(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 31:

    { (yyval.t) = makeCons((yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 32:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 33:

    { (yyval.t) = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,(yyvsp[(3) - (4)].t)),
				  oz_consUnwrap((yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t))); }
    break;

  case 34:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 35:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 36:

    { (yyval.t) = newCTerm(PA_fOpApply,(yyvsp[(2) - (4)].t),
				  oz_mklistUnwrap((yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t)),(yyvsp[(3) - (4)].t)); }
    break;

  case 37:

    { (yyval.t) = newCTerm(PA_fOpApply,(yyvsp[(2) - (4)].t),
				  oz_mklistUnwrap((yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t)),(yyvsp[(3) - (4)].t)); }
    break;

  case 38:

    { (yyval.t) = newCTerm(PA_fOpApply,(yyvsp[(2) - (4)].t),
				  oz_mklistUnwrap((yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t)),(yyvsp[(3) - (4)].t)); }
    break;

  case 39:

    { (yyval.t) = newCTerm(PA_fObjApply,(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 40:

    { (yyval.t) = newCTerm(PA_fOpApply,AtomTilde,
				  oz_mklistUnwrap((yyvsp[(3) - (3)].t)),(yyvsp[(2) - (3)].t)); }
    break;

  case 41:

    { (yyval.t) = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklistUnwrap((yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t)),(yyvsp[(3) - (4)].t)); }
    break;

  case 42:

    { (yyval.t) = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklistUnwrap((yyvsp[(1) - (2)].t),makeInt(xytext,pos())),pos()); }
    break;

  case 43:

    { (yyval.t) = newCTerm(PA_fOpApply,AtomHat,
				  oz_mklistUnwrap((yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t)),(yyvsp[(3) - (4)].t)); }
    break;

  case 44:

    { (yyval.t) = newCTerm(PA_fAt,(yyvsp[(3) - (3)].t),(yyvsp[(2) - (3)].t)); }
    break;

  case 45:

    { (yyval.t) = newCTerm(PA_fOpApply,AtomDExcl,
				  oz_mklistUnwrap((yyvsp[(3) - (3)].t)),(yyvsp[(2) - (3)].t)); }
    break;

  case 46:

    { (yyval.t) = newCTerm(PA_parens,(yyvsp[(2) - (3)].t)); }
    break;

  case 47:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 48:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 49:

    { (yyval.t) = newCTerm(PA_fWildcard,pos()); }
    break;

  case 50:

    { (yyval.t) = newCTerm(PA_fAtom,NameUnit,pos()); }
    break;

  case 51:

    { (yyval.t) = newCTerm(PA_fAtom,NameTrue,pos()); }
    break;

  case 52:

    { (yyval.t) = newCTerm(PA_fAtom,NameFalse,pos()); }
    break;

  case 53:

    { (yyval.t) = newCTerm(PA_fSelf,pos()); }
    break;

  case 54:

    { (yyval.t) = newCTerm(PA_fDollar,pos()); }
    break;

  case 55:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 56:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 57:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 58:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 59:

    { (yyval.t) = newCTerm(PA_fRecord,newCTerm(PA_fAtom,AtomCons,
						     makeLongPos((yyvsp[(2) - (6)].t),(yyvsp[(6) - (6)].t))),
				  oz_mklistUnwrap((yyvsp[(3) - (6)].t),(yyvsp[(4) - (6)].t))); }
    break;

  case 60:

    { (yyval.t) = newCTerm(PA_fApply,(yyvsp[(3) - (6)].t),(yyvsp[(4) - (6)].t),makeLongPos((yyvsp[(2) - (6)].t),(yyvsp[(6) - (6)].t))); }
    break;

  case 61:

    { (yyval.t) = newCTerm(PA_fProc,(yyvsp[(5) - (10)].t),(yyvsp[(6) - (10)].t),(yyvsp[(8) - (10)].t),(yyvsp[(3) - (10)].t),makeLongPos((yyvsp[(2) - (10)].t),(yyvsp[(10) - (10)].t))); }
    break;

  case 62:

    { (yyval.t) = newCTerm(PA_fFun,(yyvsp[(5) - (10)].t),(yyvsp[(6) - (10)].t),(yyvsp[(8) - (10)].t),(yyvsp[(3) - (10)].t),makeLongPos((yyvsp[(2) - (10)].t),(yyvsp[(10) - (10)].t))); }
    break;

  case 63:

    { (yyval.t) = newCTerm(PA_fFunctor,(yyvsp[(3) - (6)].t),(yyvsp[(4) - (6)].t),makeLongPos((yyvsp[(2) - (6)].t),(yyvsp[(6) - (6)].t))); }
    break;

  case 64:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 65:

    { (yyval.t) = newCTerm(PA_fLocal,(yyvsp[(3) - (6)].t),(yyvsp[(5) - (6)].t),(yyvsp[(2) - (6)].t)); }
    break;

  case 66:

    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 67:

    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 68:

    { (yyval.t) = newCTerm(PA_fLock,(yyvsp[(3) - (5)].t),makeLongPos((yyvsp[(2) - (5)].t),(yyvsp[(5) - (5)].t))); }
    break;

  case 69:

    { (yyval.t) = newCTerm(PA_fLockThen,(yyvsp[(3) - (7)].t),(yyvsp[(5) - (7)].t),makeLongPos((yyvsp[(2) - (7)].t),(yyvsp[(7) - (7)].t))); }
    break;

  case 70:

    { (yyval.t) = newCTerm(PA_fThread,(yyvsp[(3) - (5)].t),makeLongPos((yyvsp[(2) - (5)].t),(yyvsp[(5) - (5)].t))); }
    break;

  case 71:

    { (yyval.t) = newCTerm(PA_fTry,(yyvsp[(3) - (7)].t),(yyvsp[(4) - (7)].t),(yyvsp[(5) - (7)].t),makeLongPos((yyvsp[(2) - (7)].t),(yyvsp[(7) - (7)].t))); }
    break;

  case 72:

    { (yyval.t) = newCTerm(PA_fRaise,(yyvsp[(3) - (5)].t),makeLongPos((yyvsp[(2) - (5)].t),(yyvsp[(5) - (5)].t))); }
    break;

  case 73:

    { (yyval.t) = newCTerm(PA_fSkip,pos()); }
    break;

  case 74:

    { (yyval.t) = newCTerm(PA_fFail,pos()); }
    break;

  case 75:

    { (yyval.t) = newCTerm(PA_fNot,(yyvsp[(3) - (5)].t),makeLongPos((yyvsp[(2) - (5)].t),(yyvsp[(5) - (5)].t))); }
    break;

  case 76:

    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 77:

    { (yyval.t) = newCTerm(PA_fOr,(yyvsp[(3) - (5)].t),makeLongPos((yyvsp[(2) - (5)].t),(yyvsp[(5) - (5)].t))); }
    break;

  case 78:

    { (yyval.t) = newCTerm(PA_fDis,(yyvsp[(3) - (5)].t),makeLongPos((yyvsp[(2) - (5)].t),(yyvsp[(5) - (5)].t))); }
    break;

  case 79:

    { (yyval.t) = newCTerm(PA_fChoice,(yyvsp[(3) - (5)].t),makeLongPos((yyvsp[(2) - (5)].t),(yyvsp[(5) - (5)].t))); }
    break;

  case 80:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 81:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 82:

    { (yyval.t) = newCTerm(PA_fMacro,(yyvsp[(3) - (5)].t),makeLongPos((yyvsp[(2) - (5)].t),(yyvsp[(5) - (5)].t))); }
    break;

  case 83:

    { (yyval.t) = newCTerm(PA_fFOR,(yyvsp[(3) - (7)].t),(yyvsp[(5) - (7)].t),makeLongPos((yyvsp[(2) - (7)].t),(yyvsp[(7) - (7)].t))); }
    break;

  case 84:

    { (yyval.t) = newCTerm(PA_fWhile,(yyvsp[(3) - (7)].t),(yyvsp[(5) - (7)].t),makeLongPos((yyvsp[(2) - (7)].t),(yyvsp[(7) - (7)].t))); }
    break;

  case 85:

    { (yyval.t) = AtomNil; }
    break;

  case 86:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 87:

    { (yyval.t) = newCTerm(oz_atom("forFlag"),(yyvsp[(1) - (1)].t)); }
    break;

  case 88:

    { (yyval.t) = newCTerm(oz_atom("forFeature"),(yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 89:

    { (yyval.t) = newCTerm(oz_atom("forPattern"),(yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 90:

    { (yyval.t) = newCTerm(oz_atom("forFrom"),(yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 91:

    { (yyval.t) = newCTerm(oz_atom("forGeneratorList"),(yyvsp[(1) - (1)].t)); }
    break;

  case 92:

    { (yyval.t) = newCTerm(oz_atom("forGeneratorInt"),(yyvsp[(1) - (4)].t),(yyvsp[(3) - (4)].t),(yyvsp[(4) - (4)].t)); }
    break;

  case 93:

    { (yyval.t) = newCTerm(oz_atom("forGeneratorC"),(yyvsp[(1) - (3)].t),oz_headUnwrap((yyvsp[(3) - (3)].t)),
                                                              oz_tailUnwrap((yyvsp[(3) - (3)].t))); }
    break;

  case 94:

    { (yyval.t) = newCTerm(oz_atom("forGeneratorC"),(yyvsp[(2) - (5)].t),oz_headUnwrap((yyvsp[(4) - (5)].t)),
                                                              oz_tailUnwrap((yyvsp[(4) - (5)].t))); }
    break;

  case 95:

    { (yyval.t) = NameUnit; }
    break;

  case 96:

    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 97:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 98:

    { (yyval.t) = NameUnit; }
    break;

  case 99:

    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 100:

    { (yyval.t) = AtomNil; }
    break;

  case 101:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 102:

    { (yyval.t) = AtomNil; }
    break;

  case 103:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 104:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fRequire,(yyvsp[(3) - (4)].t),(yyvsp[(2) - (4)].t)),(yyvsp[(4) - (4)].t)); }
    break;

  case 105:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fPrepare,(yyvsp[(3) - (6)].t),(yyvsp[(5) - (6)].t),(yyvsp[(2) - (6)].t)),(yyvsp[(6) - (6)].t)); }
    break;

  case 106:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fPrepare,(yyvsp[(3) - (4)].t),
					   newCTerm(PA_fSkip,(yyvsp[(2) - (4)].t)),(yyvsp[(2) - (4)].t)),(yyvsp[(4) - (4)].t)); }
    break;

  case 107:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fImport,(yyvsp[(3) - (4)].t),(yyvsp[(2) - (4)].t)),(yyvsp[(4) - (4)].t)); }
    break;

  case 108:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fExport,(yyvsp[(3) - (4)].t),(yyvsp[(2) - (4)].t)),(yyvsp[(4) - (4)].t)); }
    break;

  case 109:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fDefine,(yyvsp[(3) - (6)].t),(yyvsp[(5) - (6)].t),(yyvsp[(2) - (6)].t)),(yyvsp[(6) - (6)].t)); }
    break;

  case 110:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fDefine,(yyvsp[(3) - (4)].t),
					   newCTerm(PA_fSkip,(yyvsp[(2) - (4)].t)),(yyvsp[(2) - (4)].t)),(yyvsp[(4) - (4)].t)); }
    break;

  case 111:

    { (yyval.t) = AtomNil; }
    break;

  case 112:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fImportItem,(yyvsp[(1) - (3)].t),AtomNil,(yyvsp[(2) - (3)].t)),(yyvsp[(3) - (3)].t)); }
    break;

  case 113:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fImportItem,(yyvsp[(1) - (6)].t),(yyvsp[(3) - (6)].t),(yyvsp[(5) - (6)].t)),(yyvsp[(6) - (6)].t)); }
    break;

  case 114:

    { (yyval.t) = newCTerm(PA_fVar,OZ_atom(xytext),(yyvsp[(2) - (2)].t)); }
    break;

  case 115:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 116:

    { (yyval.t) = oz_mklistUnwrap(oz_pair2Unwrap((yyvsp[(3) - (3)].t),(yyvsp[(1) - (3)].t))); }
    break;

  case 117:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 118:

    { (yyval.t) = oz_consUnwrap(oz_pair2Unwrap((yyvsp[(3) - (4)].t),(yyvsp[(1) - (4)].t)),(yyvsp[(4) - (4)].t)); }
    break;

  case 119:

    { (yyval.t) = PA_fNoImportAt; }
    break;

  case 120:

    { (yyval.t) = newCTerm(PA_fImportAt,(yyvsp[(2) - (2)].t)); }
    break;

  case 121:

    { (yyval.t) = AtomNil; }
    break;

  case 122:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fExportItem,(yyvsp[(1) - (2)].t)),(yyvsp[(2) - (2)].t)); }
    break;

  case 123:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fExportItem,
					   newCTerm(PA_fColon,(yyvsp[(1) - (4)].t),(yyvsp[(3) - (4)].t))),(yyvsp[(4) - (4)].t)); }
    break;

  case 124:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 125:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 126:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 127:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 128:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 129:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 130:

    { (yyval.t) = newCTerm(PA_fLocal,(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 131:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 132:

    { (yyval.t) = AtomNil; }
    break;

  case 133:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 134:

    { (yyval.t) = newCTerm(PA_fAtom,AtomNil,(yyvsp[(1) - (1)].t)); }
    break;

  case 135:

    { (yyval.t) = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomCons,(yyvsp[(1) - (3)].t)),
				  oz_mklistUnwrap((yyvsp[(2) - (3)].t),(yyvsp[(3) - (3)].t))); }
    break;

  case 136:

    { (yyval.t) = PA_fNoCatch; }
    break;

  case 137:

    { (yyval.t) = newCTerm(PA_fCatch,(yyvsp[(3) - (3)].t),(yyvsp[(2) - (3)].t)); }
    break;

  case 138:

    { (yyval.t) = PA_fNoFinally; }
    break;

  case 139:

    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 140:

    {
		    (yyval.t) = newCTerm(OZ_isTrue((yyvsp[(5) - (7)].t))? PA_fOpenRecord : PA_fRecord,
				  newCTerm(PA_fAtom,(yyvsp[(1) - (7)].t),makeLongPos((yyvsp[(2) - (7)].t),(yyvsp[(7) - (7)].t))),(yyvsp[(4) - (7)].t));
		  }
    break;

  case 141:

    {
		    (yyval.t) = newCTerm(OZ_isTrue((yyvsp[(5) - (7)].t))? PA_fOpenRecord : PA_fRecord,
				  makeVar((yyvsp[(1) - (7)].t),makeLongPos((yyvsp[(2) - (7)].t),(yyvsp[(7) - (7)].t))),(yyvsp[(4) - (7)].t));
		  }
    break;

  case 142:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 143:

    { (yyval.t) = NameUnit; }
    break;

  case 144:

    { (yyval.t) = NameTrue; }
    break;

  case 145:

    { (yyval.t) = NameFalse; }
    break;

  case 146:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 147:

    { (yyval.t) = AtomNil; }
    break;

  case 148:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 149:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fColon,(yyvsp[(1) - (4)].t),(yyvsp[(3) - (4)].t)),(yyvsp[(4) - (4)].t)); }
    break;

  case 150:

    { (yyval.t) = NameFalse; }
    break;

  case 151:

    { (yyval.t) = NameTrue; }
    break;

  case 152:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 153:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 154:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 155:

    { (yyval.t) = newCTerm(PA_fAtom,NameUnit,pos()); }
    break;

  case 156:

    { (yyval.t) = newCTerm(PA_fAtom,NameTrue,pos()); }
    break;

  case 157:

    { (yyval.t) = newCTerm(PA_fAtom,NameFalse,pos()); }
    break;

  case 158:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 159:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 160:

    { (yyval.t) = newCTerm(PA_fBoolCase,(yyvsp[(2) - (6)].t),(yyvsp[(4) - (6)].t),(yyvsp[(5) - (6)].t),makeLongPos((yyvsp[(1) - (6)].t),(yyvsp[(6) - (6)].t))); }
    break;

  case 161:

    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 162:

    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 163:

    { (yyval.t) = (yyvsp[(2) - (3)].t); }
    break;

  case 164:

    { (yyval.t) = newCTerm(PA_fNoElse,pos()); }
    break;

  case 165:

    { checkDeprecation((yyvsp[(4) - (7)].t));
		    (yyval.t) = newCTerm(PA_fBoolCase,(yyvsp[(2) - (7)].t),(yyvsp[(5) - (7)].t),(yyvsp[(6) - (7)].t),makeLongPos((yyvsp[(1) - (7)].t),(yyvsp[(7) - (7)].t)));
		  }
    break;

  case 166:

    { (yyval.t) = newCTerm(PA_fCase,(yyvsp[(2) - (6)].t),(yyvsp[(4) - (6)].t),(yyvsp[(5) - (6)].t),makeLongPos((yyvsp[(1) - (6)].t),(yyvsp[(6) - (6)].t))); }
    break;

  case 167:

    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 168:

    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 169:

    { (yyval.t) = (yyvsp[(2) - (3)].t); }
    break;

  case 170:

    { (yyval.t) = newCTerm(PA_fNoElse,pos()); }
    break;

  case 171:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 172:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 173:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 174:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 175:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 176:

    { (yyval.t) = newCTerm(PA_fCaseClause,(yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 177:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 178:

    { (yyval.t) = newCTerm(PA_fSideCondition,(yyvsp[(1) - (4)].t),
				  newCTerm(PA_fSkip,(yyvsp[(3) - (4)].t)),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 179:

    { (yyval.t) = newCTerm(PA_fSideCondition,(yyvsp[(1) - (6)].t),(yyvsp[(4) - (6)].t),(yyvsp[(6) - (6)].t),(yyvsp[(3) - (6)].t)); }
    break;

  case 180:

    { (yyval.t) = newCTerm(PA_fEq,(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 181:

    { (yyval.t) = makeCons((yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 182:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 183:

    { (yyval.t) = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,(yyvsp[(3) - (4)].t)),
				  oz_consUnwrap((yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t))); }
    break;

  case 184:

    { (yyval.t) = newCTerm(PA_fClass,(yyvsp[(3) - (7)].t),(yyvsp[(4) - (7)].t),(yyvsp[(5) - (7)].t),makeLongPos((yyvsp[(2) - (7)].t),(yyvsp[(7) - (7)].t))); }
    break;

  case 185:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 186:

    { (yyval.t) = newCTerm(PA_fDollar,(yyvsp[(1) - (1)].t)); }
    break;

  case 187:

    { (yyval.t) = AtomNil; }
    break;

  case 188:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 189:

    { (yyval.t) = newCTerm(PA_fFrom,oz_consUnwrap((yyvsp[(3) - (4)].t),(yyvsp[(4) - (4)].t)),(yyvsp[(2) - (4)].t)); }
    break;

  case 190:

    { (yyval.t) = newCTerm(PA_fAttr,oz_consUnwrap((yyvsp[(3) - (4)].t),(yyvsp[(4) - (4)].t)),(yyvsp[(2) - (4)].t)); }
    break;

  case 191:

    { (yyval.t) = newCTerm(PA_fFeat,oz_consUnwrap((yyvsp[(3) - (4)].t),(yyvsp[(4) - (4)].t)),(yyvsp[(2) - (4)].t)); }
    break;

  case 192:

    { (yyval.t) = newCTerm(PA_fProp,oz_consUnwrap((yyvsp[(3) - (4)].t),(yyvsp[(4) - (4)].t)),(yyvsp[(2) - (4)].t)); }
    break;

  case 193:

    { (yyval.t) = AtomNil; }
    break;

  case 194:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 195:

    { (yyval.t) = oz_pair2Unwrap((yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 196:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 197:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 198:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 199:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 200:

    { (yyval.t) = newCTerm(PA_fAtom,NameUnit,pos()); }
    break;

  case 201:

    { (yyval.t) = newCTerm(PA_fAtom,NameTrue,pos()); }
    break;

  case 202:

    { (yyval.t) = newCTerm(PA_fAtom,NameFalse,pos()); }
    break;

  case 203:

    { (yyval.t) = AtomNil; }
    break;

  case 204:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 205:

    { (yyval.t) = newCTerm(PA_fMeth,(yyvsp[(3) - (5)].t),(yyvsp[(4) - (5)].t),(yyvsp[(2) - (5)].t)); }
    break;

  case 206:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 207:

    { (yyval.t) = newCTerm(PA_fEq,(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 208:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 209:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 210:

    { (yyval.t) = newCTerm(PA_fAtom,NameUnit,pos()); }
    break;

  case 211:

    { (yyval.t) = newCTerm(PA_fAtom,NameTrue,pos()); }
    break;

  case 212:

    { (yyval.t) = newCTerm(PA_fAtom,NameFalse,pos()); }
    break;

  case 213:

    { (yyval.t) = newCTerm(PA_fRecord,(yyvsp[(1) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 214:

    { (yyval.t) = newCTerm(PA_fOpenRecord,(yyvsp[(1) - (5)].t),(yyvsp[(3) - (5)].t)); }
    break;

  case 215:

    { (yyval.t) = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); }
    break;

  case 216:

    { (yyval.t) = makeVar(xytext); }
    break;

  case 217:

    { (yyval.t) = newCTerm(PA_fEscape,makeVar(xytext),(yyvsp[(2) - (3)].t)); }
    break;

  case 218:

    { (yyval.t) = newCTerm(PA_fAtom,NameUnit,pos()); }
    break;

  case 219:

    { (yyval.t) = newCTerm(PA_fAtom,NameTrue,pos()); }
    break;

  case 220:

    { (yyval.t) = newCTerm(PA_fAtom,NameFalse,pos()); }
    break;

  case 221:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 222:

    { (yyval.t) = AtomNil; }
    break;

  case 223:

    { (yyval.t) = newCTerm(PA_fMethArg,(yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 224:

    { (yyval.t) = newCTerm(PA_fMethColonArg,(yyvsp[(1) - (4)].t),(yyvsp[(3) - (4)].t),(yyvsp[(4) - (4)].t)); }
    break;

  case 225:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 226:

    { (yyval.t) = newCTerm(PA_fDollar,pos()); }
    break;

  case 227:

    { (yyval.t) = newCTerm(PA_fWildcard,pos()); }
    break;

  case 228:

    { (yyval.t) = newCTerm(PA_fDefault,(yyvsp[(3) - (3)].t),(yyvsp[(2) - (3)].t)); }
    break;

  case 229:

    { (yyval.t) = PA_fNoDefault; }
    break;

  case 230:

    { (yyval.t) = newCTerm(PA_fCond,(yyvsp[(2) - (5)].t),(yyvsp[(3) - (5)].t),makeLongPos((yyvsp[(1) - (5)].t),(yyvsp[(5) - (5)].t))); }
    break;

  case 231:

    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 232:

    { (yyval.t) = newCTerm(PA_fNoElse,pos()); }
    break;

  case 233:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 234:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 235:

    { (yyval.t) = newCTerm(PA_fClause,newCTerm(PA_fSkip,(yyvsp[(3) - (4)].t)),(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t)); }
    break;

  case 236:

    { (yyval.t) = newCTerm(PA_fClause,(yyvsp[(1) - (5)].t),(yyvsp[(3) - (5)].t),(yyvsp[(5) - (5)].t)); }
    break;

  case 237:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 238:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 239:

    { (yyval.t) = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,(yyvsp[(2) - (2)].t)),
				  (yyvsp[(1) - (2)].t),newCTerm(PA_fNoThen,(yyvsp[(2) - (2)].t))); }
    break;

  case 240:

    { (yyval.t) = newCTerm(PA_fClause,(yyvsp[(1) - (4)].t),(yyvsp[(3) - (4)].t),newCTerm(PA_fNoThen,(yyvsp[(4) - (4)].t))); }
    break;

  case 241:

    { (yyval.t) = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,(yyvsp[(2) - (4)].t)),(yyvsp[(1) - (4)].t),(yyvsp[(4) - (4)].t)); }
    break;

  case 242:

    { (yyval.t) = newCTerm(PA_fClause,(yyvsp[(1) - (5)].t),(yyvsp[(3) - (5)].t),(yyvsp[(5) - (5)].t)); }
    break;

  case 243:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 244:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 245:

    { (yyval.t) = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); }
    break;

  case 246:

    { (yyval.t) = makeVar(xytext); }
    break;

  case 247:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 248:

    { (yyval.t) = newCTerm(PA_fEscape,(yyvsp[(3) - (3)].t),(yyvsp[(2) - (3)].t)); }
    break;

  case 249:

    { (yyval.t) = makeString(xytext,pos()); }
    break;

  case 250:

    { (yyval.t) = makeInt(xytext,pos()); }
    break;

  case 251:

    { (yyval.t) = makeInt(xytext[0],pos()); }
    break;

  case 252:

    { (yyval.t) = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); }
    break;

  case 253:

    { (yyval.t) = pos(); }
    break;

  case 254:

    { (yyval.t) = pos(); }
    break;

  case 255:

    { OZ_Term prefix =
		      scannerPrefix? scannerPrefix: PA_zy;
		    (yyval.t) = newCTerm(PA_fScanner,(yyvsp[(3) - (8)].t),(yyvsp[(4) - (8)].t),(yyvsp[(5) - (8)].t),(yyvsp[(6) - (8)].t),prefix,
				  makeLongPos((yyvsp[(2) - (8)].t),(yyvsp[(8) - (8)].t))); }
    break;

  case 256:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 257:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 258:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 259:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 260:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 261:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 262:

    { (yyval.t) = newCTerm(PA_fLexicalAbbreviation,(yyvsp[(2) - (5)].t),(yyvsp[(4) - (5)].t)); }
    break;

  case 263:

    { (yyval.t) = newCTerm(PA_fLexicalAbbreviation,(yyvsp[(2) - (5)].t),(yyvsp[(4) - (5)].t)); }
    break;

  case 264:

    { (yyval.t) = newCTerm(PA_fLexicalRule,(yyvsp[(2) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 265:

    { (yyval.t) = OZ_string(xytext); }
    break;

  case 266:

    { (yyval.t) = OZ_string(xytext); }
    break;

  case 267:

    { (yyval.t) = newCTerm(PA_fMode,(yyvsp[(2) - (4)].t),(yyvsp[(3) - (4)].t)); }
    break;

  case 268:

    { (yyval.t) = AtomNil; }
    break;

  case 269:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 270:

    { (yyval.t) = newCTerm(PA_fInheritedModes,(yyvsp[(2) - (2)].t)); }
    break;

  case 271:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 272:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 273:

    { OZ_Term expect = parserExpect? parserExpect: makeTaggedSmallInt(0);
		    (yyval.t) = newCTerm(PA_fParser,(yyvsp[(3) - (9)].t),(yyvsp[(4) - (9)].t),(yyvsp[(5) - (9)].t),(yyvsp[(6) - (9)].t),(yyvsp[(7) - (9)].t),expect,
				  makeLongPos((yyvsp[(2) - (9)].t),(yyvsp[(9) - (9)].t))); }
    break;

  case 274:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 275:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 276:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 277:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 278:

    { (yyval.t) = newCTerm(PA_fToken,AtomNil); }
    break;

  case 279:

    { (yyval.t) = newCTerm(PA_fToken,(yyvsp[(2) - (2)].t)); }
    break;

  case 280:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 281:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 282:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 283:

    { (yyval.t) = oz_pair2Unwrap((yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 284:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 285:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 286:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 287:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 288:

    { *prodKey[depth]++ = '='; }
    break;

  case 289:

    { (yyval.t) = newCTerm(PA_fProductionTemplate,(yyvsp[(6) - (9)].t),(yyvsp[(5) - (9)].t),(yyvsp[(7) - (9)].t),(yyvsp[(8) - (9)].t),(yyvsp[(2) - (9)].t)); }
    break;

  case 290:

    { (yyval.t) = newCTerm(PA_fDollar,pos()); }
    break;

  case 291:

    { *prodKey[depth]++ = '='; }
    break;

  case 292:

    { (yyval.t) = newCTerm(PA_fProductionTemplate,(yyvsp[(7) - (10)].t),(yyvsp[(6) - (10)].t),(yyvsp[(8) - (10)].t),(yyvsp[(9) - (10)].t),(yyvsp[(3) - (10)].t)); }
    break;

  case 293:

    { (yyval.t) = newCTerm(PA_fProductionTemplate,(yyvsp[(3) - (6)].t),(yyvsp[(2) - (6)].t),(yyvsp[(4) - (6)].t),(yyvsp[(5) - (6)].t),PA_none); }
    break;

  case 294:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(2) - (3)].t)); }
    break;

  case 295:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (2)].t)); }
    break;

  case 296:

    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 299:

    { prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg((yyvsp[(1) - (2)].t),0))); }
    break;

  case 300:

    { *prodKey[depth]++ = '('; depth++; }
    break;

  case 301:

    { depth--; }
    break;

  case 302:

    { (yyval.t) = (yyvsp[(3) - (6)].t); }
    break;

  case 303:

    { *prodKey[depth]++ = '['; depth++; }
    break;

  case 304:

    { depth--; }
    break;

  case 305:

    { (yyval.t) = (yyvsp[(3) - (6)].t); }
    break;

  case 306:

    { *prodKey[depth]++ = '{'; depth++; }
    break;

  case 307:

    { depth--; }
    break;

  case 308:

    { (yyval.t) = (yyvsp[(3) - (6)].t); }
    break;

  case 309:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 310:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 311:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 312:

    { (yyval.t) = newCTerm(PA_fWildcard,pos()); }
    break;

  case 313:

    { *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; }
    break;

  case 316:

    { *prodKey[depth]++ = xytext[0]; }
    break;

  case 317:

    { *prodKey[depth]++ = xytext[0]; }
    break;

  case 318:

    { *prodKey[depth] = '\0';
		    (yyval.t) = oz_pair2Unwrap(prodName[depth],OZ_string(prodKeyBuffer[depth]));
		    prodName[depth] = PA_none;
		    prodKey[depth] = prodKeyBuffer[depth];
		  }
    break;

  case 319:

    { (yyval.t) = AtomNil; }
    break;

  case 320:

    { (yyval.t) = (yyvsp[(1) - (2)].t); }
    break;

  case 321:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 322:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 323:

    { (yyval.t) = newCTerm(PA_fSyntaxRule,(yyvsp[(2) - (4)].t),AtomNil,(yyvsp[(3) - (4)].t)); }
    break;

  case 324:

    { (yyval.t) = newCTerm(PA_fSyntaxRule,(yyvsp[(2) - (4)].t),AtomNil,(yyvsp[(3) - (4)].t)); }
    break;

  case 325:

    { (yyval.t) = newCTerm(PA_fSyntaxRule,(yyvsp[(2) - (7)].t),(yyvsp[(4) - (7)].t),(yyvsp[(6) - (7)].t)); }
    break;

  case 326:

    { (yyval.t) = AtomNil; }
    break;

  case 327:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 328:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 329:

    { (yyval.t) = newCTerm(PA_fDollar,pos()); }
    break;

  case 330:

    { (yyval.t) = newCTerm(PA_fWildcard,pos()); }
    break;

  case 331:

    { (yyval.t) = newCTerm(PA_fSynAlternative, (yyvsp[(1) - (1)].t)); }
    break;

  case 332:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 333:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 334:

    { OZ_Term t = (yyvsp[(2) - (2)].t);
		    while (terms[depth]) {
		      t = oz_consUnwrap(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    (yyval.t) = newCTerm(PA_fSynSequence, decls[depth], t, (yyvsp[(1) - (2)].t));
		    decls[depth] = AtomNil;
		  }
    break;

  case 335:

    { (yyval.t) = newCTerm(PA_fSynSequence, AtomNil, (yyvsp[(3) - (3)].t), (yyvsp[(2) - (3)].t)); }
    break;

  case 336:

    { (yyval.t) = AtomNil; }
    break;

  case 337:

    { (yyval.t) = oz_mklistUnwrap(newCTerm(PA_fSynAction,(yyvsp[(2) - (2)].t))); }
    break;

  case 338:

    { (yyval.t) = (yyvsp[(2) - (2)].t); }
    break;

  case 339:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fSynTemplateInstantiation, (yyvsp[(5) - (5)].t),
					   oz_consUnwrap(newCTerm(PA_fSynApplication,
							     terms[depth]->term,
							     AtomNil),
						    AtomNil),
					   (yyvsp[(3) - (5)].t)),
				  (yyvsp[(4) - (5)].t));
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  }
    break;

  case 340:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fSynAssignment, terms[depth]->term, (yyvsp[(3) - (4)].t)),
				  (yyvsp[(4) - (4)].t));
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  }
    break;

  case 341:

    { while (terms[depth]) {
		      decls[depth] = oz_consUnwrap(terms[depth]->term, decls[depth]);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    (yyval.t) = (yyvsp[(2) - (2)].t);
		  }
    break;

  case 342:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 343:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 344:

    { terms[depth] = new TermNode((yyvsp[(1) - (1)].t), terms[depth]); }
    break;

  case 345:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 346:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (2)].t),(yyvsp[(2) - (2)].t)); }
    break;

  case 347:

    { (yyval.t) = newCTerm(PA_fSynAssignment,(yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;

  case 348:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 349:

    { (yyval.t) = newCTerm(PA_fSynApplication,(yyvsp[(1) - (1)].t),AtomNil); }
    break;

  case 350:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[(4) - (4)].t),
				  oz_consUnwrap(newCTerm(PA_fSynApplication,(yyvsp[(1) - (4)].t),
						    AtomNil),
					   AtomNil),(yyvsp[(3) - (4)].t));
		  }
    break;

  case 351:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 352:

    { (yyval.t) = newCTerm(PA_fSynAssignment,
				  newCTerm(PA_fEscape,(yyvsp[(3) - (5)].t),(yyvsp[(2) - (5)].t)),(yyvsp[(5) - (5)].t)); }
    break;

  case 353:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 354:

    { (yyval.t) = (yyvsp[(1) - (1)].t); }
    break;

  case 355:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[(5) - (5)].t),
				  oz_mklistUnwrap((yyvsp[(3) - (5)].t)),(yyvsp[(2) - (5)].t));
		  }
    break;

  case 356:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[(4) - (4)].t),
				  oz_mklistUnwrap((yyvsp[(1) - (4)].t)),(yyvsp[(3) - (4)].t));
		  }
    break;

  case 357:

    { *prodKey[depth]++ = '('; depth++; }
    break;

  case 358:

    { depth--; }
    break;

  case 359:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[(9) - (9)].t),(yyvsp[(5) - (9)].t),(yyvsp[(2) - (9)].t)); }
    break;

  case 360:

    { *prodKey[depth]++ = '['; depth++; }
    break;

  case 361:

    { depth--; }
    break;

  case 362:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[(9) - (9)].t),(yyvsp[(5) - (9)].t),(yyvsp[(2) - (9)].t)); }
    break;

  case 363:

    { *prodKey[depth]++ = '{'; depth++; }
    break;

  case 364:

    { depth--; }
    break;

  case 365:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[(9) - (9)].t),(yyvsp[(5) - (9)].t),(yyvsp[(2) - (9)].t)); }
    break;

  case 366:

    { (yyval.t) = newCTerm(PA_fSynApplication,(yyvsp[(1) - (1)].t),AtomNil); }
    break;

  case 367:

    { (yyval.t) = newCTerm(PA_fSynApplication,(yyvsp[(1) - (5)].t),(yyvsp[(4) - (5)].t)); }
    break;

  case 368:

    { (yyval.t) = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); }
    break;

  case 369:

    { (yyval.t) = makeVar(xytext); }
    break;

  case 370:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[(1) - (1)].t)); }
    break;

  case 371:

    { (yyval.t) = oz_consUnwrap((yyvsp[(1) - (3)].t),(yyvsp[(3) - (3)].t)); }
    break;


/* Line 1267 of yacc.c.  */

      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}





void checkDeprecation(OZ_Term coord) {
  const char *msg = "use `if' instead of `case' for boolean conditionals";
  if (xy_allowDeprecated) {
    xyreportWarning("deprecation warning",msg,coord);
  } else {
    xyreportError("deprecation error",msg,coord);
  }
}

void xyreportWarning(const char *kind, const char *msg, OZ_Term coord) {
  OZ_Term args = oz_mklist(oz_pair2(PA_coord, coord),
			   oz_pair2(PA_kind,  OZ_atom(kind)),
			   oz_pair2(PA_msg,   OZ_atom(msg)));
  xy_errorMessages = OZ_cons(OZ_recordInit(PA_warn,args),
			     xy_errorMessages);
}

void xyreportError(const char *kind, const char *msg, OZ_Term coord) {
  OZ_Term args = oz_mklist(oz_pair2(PA_coord, coord),
			   oz_pair2(PA_kind,  OZ_atom(kind)),
			   oz_pair2(PA_msg,   OZ_atom(msg)));
  xy_errorMessages = OZ_cons(OZ_recordInit(PA_error,args),
			     xy_errorMessages);
}

void xyreportError(const char *kind, const char *msg, const char *file,
		   int line, int column) {
  xyreportError(kind,msg,OZ_mkTupleC("pos",3,OZ_atom((char*)file),
				     oz_int(line),oz_int(column)));
}

static void xyerror(const char *s) {
  if (!strncmp(s, "parse error", 11)) {
    if (strlen(s) > 13) {
      xyreportError("parse error", s + 13, xyFileName, xylino, xycharno());
    } else if (yychar != YYEMPTY) {
      int yychar1 = YYTRANSLATE(yychar);
      char *s2 = new char[30 + strlen(yytname[yychar1])];
      sprintf(s2, "unexpected token `%s'", yytname[yychar1]);
      xyreportError("parse error", s2, xyFileName, xylino, xycharno());
      delete[] s2;
    } else {
      xyreportError("parse error", s, xyFileName, xylino, xycharno());
    }
  } else {
    xyreportError("parse error", s, xyFileName, xylino, xycharno());
  }
}

static OZ_Term init_options(OZ_Term optRec) {
  OZ_Term x;

  x = OZ_subtree(optRec, PA_gump);
  xy_gumpSyntax = x == 0? 0: OZ_eq(x, NameTrue);

  x = OZ_subtree(optRec, PA_allowdeprecated);
  xy_allowDeprecated = x == 0? 1: OZ_eq(x, NameTrue);

  x = OZ_subtree(optRec, PA_allowwhileloop);
  xy_allowWhileLoop = x == 0? 0: OZ_eq(x, NameTrue);

  OZ_Term defines = OZ_subtree(optRec, PA_defines);
  return defines;
}

static OZ_Term parse() {
  int i;
  for (i = 0; i < DEPTH; i++) {
    prodKey[i] = prodKeyBuffer[i];
    prodName[i] = PA_none;
    terms[i] = 0;
    decls[i] = AtomNil;
  }
  depth = 0;
  for (i = 0; i < DEPTH; i++)
    terms[i] = 0;

  xyparse();

  // in case there was a syntax error during the parse, delete garbage:
  xy_exit();
  for (i = 0; i < DEPTH; i++)
    while (terms[i]) {
      TermNode *tmp = terms[i]; terms[i] = terms[i]->next; delete tmp;
    }

  return OZ_pair2(unwrap(yyoutput), xy_errorMessages);
}

OZ_BI_define(parser_parseFile, 2, 1)
{
  // {ParseFile FileName OptRec ?(AST#ReporterMessages)}
  OZ_declareVirtualString(0, file);
  OZ_declareDetTerm(1, optRec);
  if (!OZ_isRecord(optRec))
    return OZ_typeError(1, "ParseOptions");
  OZ_Term defines = init_options(optRec);
  if (defines == 0 || !OZ_isDictionary(defines))
    return OZ_typeError(1, "ParseOptions");
  if (!xy_init_from_file(file, defines))
    OZ_RETURN(OZ_pair2(PA_fileNotFound, AtomNil));
  else
    OZ_RETURN(parse());
}
OZ_BI_end

OZ_BI_define(parser_parseVirtualString, 2, 1)
{
  // {ParseVirtualString VS OptRec ?(AST#ReporterMessages)}
  OZ_declareVirtualString(0, str);
  OZ_declareDetTerm(1, optRec);
  if (!OZ_isRecord(optRec))
    return OZ_typeError(1, "ParseOptions");
  OZ_Term defines = init_options(optRec);
  if (defines == 0 || !OZ_isDictionary(defines))
    return OZ_typeError(1, "ParseOptions");
  xy_init_from_string(str, defines);
  OZ_RETURN(parse());
}
OZ_BI_end

OZ_BI_define(parser_expandFileName, 1, 1)
{
  OZ_declareVirtualString(0, in);
  char *out = xy_expand_file_name(in);
  OZ_RETURN(out == NULL? NameFalse: OZ_atom(out));
}
OZ_BI_end

