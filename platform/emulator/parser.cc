/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

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
     T_for = 323,
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
#define T_for 323
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

extern int xy_gumpSyntax, xy_allowDeprecated;
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
void xyreportWarning(char *kind, char *msg, OZ_Term coord);
void xyreportError(char *kind, char *msg, OZ_Term coord);
void xyreportError(char *kind, char *msg,
		   const char *file, int line, int column);


//-----------------
// Local Variables
//-----------------

#define YYMAXDEPTH 1000000
#define YYERROR_VERBOSE

static OZ_Term yyoutput;

static void xyerror(char *);

//-----------------
// Atom definitions
//-----------------

#define PA_MAX_ATOM 111

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
#define PA_fLoop				_PA_AtomTab[106]
#define PA_fMacro				_PA_AtomTab[107]
#define PA_fDotAssign				_PA_AtomTab[108]
#define PA_fFOR					_PA_AtomTab[109]
#define PA_fColonEquals				_PA_AtomTab[110]
#define PA_parens				_PA_AtomTab[111]

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
	"fLoop",				//106
	"fMacro",				//107
	"fDotAssign",				//108
	"fFOR",					//109
	"fColonEquals",				//110
	"parentheses",				//111
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
OZ_Term makeInt(char * chars, OZ_Term pos) {
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

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
  OZ_Term t;
  int i;
} YYSTYPE;
/* Line 190 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 213 of yacc.c.  */


#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
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
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  120
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2151

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  118
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  146
/* YYNRULES -- Number of rules. */
#define YYNRULES  377
/* YYNRULES -- Number of states. */
#define YYNSTATES  810

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   351

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
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
static const unsigned short int yyprhs[] =
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
     394,   397,   399,   408,   415,   416,   419,   420,   423,   424,
     426,   431,   438,   443,   448,   453,   460,   465,   466,   470,
     477,   480,   482,   486,   489,   494,   495,   498,   499,   502,
     507,   509,   511,   513,   515,   517,   519,   524,   526,   527,
     530,   532,   536,   537,   541,   542,   545,   553,   561,   563,
     565,   567,   569,   571,   572,   575,   580,   581,   583,   585,
     587,   589,   591,   593,   595,   597,   599,   606,   609,   612,
     616,   618,   626,   633,   636,   639,   643,   645,   647,   651,
     655,   657,   661,   665,   667,   672,   679,   684,   689,   691,
     696,   704,   706,   708,   709,   712,   717,   722,   727,   732,
     733,   736,   740,   742,   744,   746,   748,   750,   752,   754,
     755,   758,   764,   766,   771,   773,   775,   777,   779,   781,
     786,   792,   794,   796,   800,   802,   804,   806,   809,   810,
     813,   818,   820,   822,   824,   828,   829,   835,   838,   839,
     841,   845,   850,   856,   860,   864,   867,   872,   877,   883,
     885,   889,   891,   893,   895,   899,   901,   903,   905,   907,
     908,   909,   918,   920,   922,   924,   927,   930,   933,   939,
     945,   950,   952,   954,   959,   960,   963,   966,   968,   970,
     980,   982,   984,   987,   990,   991,   994,   996,   999,  1001,
    1005,  1007,  1010,  1012,  1015,  1016,  1026,  1027,  1028,  1039,
    1046,  1050,  1053,  1056,  1058,  1059,  1062,  1063,  1064,  1071,
    1072,  1073,  1080,  1081,  1082,  1089,  1091,  1095,  1097,  1099,
    1101,  1102,  1104,  1106,  1108,  1109,  1110,  1113,  1115,  1118,
    1123,  1128,  1136,  1137,  1140,  1142,  1144,  1146,  1148,  1150,
    1154,  1157,  1161,  1162,  1165,  1168,  1174,  1179,  1182,  1185,
    1187,  1189,  1191,  1194,  1198,  1200,  1202,  1207,  1209,  1215,
    1217,  1219,  1225,  1230,  1231,  1232,  1242,  1243,  1244,  1254,
    1255,  1256,  1266,  1268,  1274,  1276,  1278,  1280
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
     119,     0,    -1,   120,    71,    -1,     1,    -1,   125,   121,
      -1,   218,   121,    -1,   121,    -1,   202,   140,   121,    -1,
     122,   120,    -1,    28,   203,   125,    47,   202,   121,    -1,
      28,   203,   125,    47,   125,   121,    -1,    28,   203,   125,
     202,   121,    -1,    -1,     3,   123,    -1,     5,    -1,     6,
      -1,     7,    -1,    -1,   124,   123,    -1,   105,     4,    -1,
     106,     4,    -1,   126,    -1,   126,   125,    -1,   126,    83,
     203,   126,    -1,   126,    86,   203,   126,    -1,   126,    84,
     203,   126,    -1,   126,    87,   203,   126,    -1,   126,    88,
     203,   126,    -1,   126,   146,   203,   126,    -1,   126,   147,
     203,   126,    -1,   126,   148,   203,   126,    -1,   126,    94,
     203,   126,    -1,   128,    -1,   128,    95,   203,   127,    -1,
     128,    -1,   128,    95,   127,    -1,   128,   149,   203,   128,
      -1,   128,   150,   203,   128,    -1,   128,   151,   203,   128,
      -1,   128,    99,   203,   128,    -1,   100,   203,   128,    -1,
     128,   101,   203,   128,    -1,   128,    13,    -1,   128,   102,
     203,   128,    -1,   103,   203,   128,    -1,   104,   203,   128,
      -1,   107,   152,   108,    -1,   196,    -1,   198,    -1,   109,
      -1,    66,    -1,    63,    -1,    38,    -1,    59,    -1,   110,
      -1,   199,    -1,   200,    -1,   201,    -1,   157,    -1,   111,
     203,   126,   154,   112,   203,    -1,   113,   203,   126,   153,
     114,   203,    -1,    55,   203,   138,   113,   126,   153,   114,
     152,    35,   203,    -1,    43,   203,   138,   113,   126,   153,
     114,   152,    35,   203,    -1,    44,   203,   174,   139,    35,
     203,    -1,   173,    -1,    48,   203,   125,    47,   125,    35,
      -1,    45,   164,    -1,    23,   166,    -1,    49,   203,   152,
      35,   203,    -1,    49,   203,   126,    61,   152,    35,   203,
      -1,    62,   203,   152,    35,   203,    -1,    65,   203,   152,
     155,   156,    35,   203,    -1,    57,   203,   152,    35,   203,
      -1,    60,    -1,    37,    -1,    51,   203,   152,    35,   203,
      -1,    27,   189,    -1,    53,   203,   193,    35,   203,    -1,
      30,   203,   193,    35,   203,    -1,    25,   203,   195,    35,
     203,    -1,   204,    -1,   212,    -1,    90,   203,   153,    89,
     203,    -1,    68,   203,   135,    70,   152,    35,   203,    -1,
      69,   203,   129,    70,   152,    35,   203,    -1,    -1,   130,
     129,    -1,   196,    -1,   196,   115,   126,    -1,   126,    47,
     131,    -1,   126,    42,   126,    -1,   126,    -1,   126,    20,
     126,   132,    -1,   126,   116,   133,    -1,   107,   126,   116,
     133,   108,    -1,    -1,   116,   126,    -1,   126,   134,    -1,
      -1,   116,   126,    -1,   135,   136,    -1,   136,    -1,   197,
      47,   203,   126,    20,   126,   137,   203,    -1,   197,    47,
     203,   126,   137,   203,    -1,    -1,   116,   126,    -1,    -1,
     196,   138,    -1,    -1,   140,    -1,    58,   203,   141,   139,
      -1,    54,   203,   125,    47,   125,   139,    -1,    54,   203,
     125,   139,    -1,    46,   203,   141,   139,    -1,    36,   203,
     145,   139,    -1,    29,   203,   125,    47,   125,   139,    -1,
      29,   203,   125,   139,    -1,    -1,   197,   144,   141,    -1,
     142,   107,   143,   108,   144,   141,    -1,    16,   203,    -1,
     163,    -1,   163,   115,   197,    -1,   163,   143,    -1,   163,
     115,   197,   143,    -1,    -1,    22,   196,    -1,    -1,   197,
     145,    -1,   163,   115,   197,   145,    -1,    92,    -1,    91,
      -1,    93,    -1,    96,    -1,    98,    -1,    97,    -1,   125,
      47,   203,   125,    -1,   125,    -1,    -1,   126,   153,    -1,
     202,    -1,   202,   126,   154,    -1,    -1,    24,   203,   169,
      -1,    -1,    41,   152,    -1,   158,   203,   107,   160,   161,
     108,   203,    -1,   159,   203,   107,   160,   161,   108,   203,
      -1,     9,    -1,    67,    -1,    64,    -1,    39,    -1,    16,
      -1,    -1,   126,   160,    -1,   162,   115,   126,   160,    -1,
      -1,    19,    -1,   196,    -1,   197,    -1,   200,    -1,    66,
      -1,    63,    -1,    38,    -1,   196,    -1,   200,    -1,   203,
     125,    61,   152,   165,   203,    -1,    33,   164,    -1,    32,
     166,    -1,    31,   152,    35,    -1,    35,    -1,   203,   125,
      61,   203,   152,   167,   203,    -1,   203,   125,    52,   168,
     167,   203,    -1,    33,   164,    -1,    32,   166,    -1,    31,
     152,    35,    -1,    35,    -1,   170,    -1,   170,    18,   168,
      -1,   170,    34,   168,    -1,   170,    -1,   170,    18,   169,
      -1,   171,    61,   152,    -1,   172,    -1,   172,    88,   202,
     125,    -1,   172,    88,   202,   125,    47,   125,    -1,   172,
      83,   203,   172,    -1,   172,    94,   203,   172,    -1,   128,
      -1,   128,    95,   203,   127,    -1,    26,   203,   174,   175,
     180,    35,   203,    -1,   126,    -1,   202,    -1,    -1,   176,
     175,    -1,    42,   203,   126,   153,    -1,    21,   203,   178,
     177,    -1,    40,   203,   178,   177,    -1,    56,   203,   126,
     153,    -1,    -1,   178,   177,    -1,   179,   115,   126,    -1,
     179,    -1,   196,    -1,   198,    -1,   200,    -1,    66,    -1,
      63,    -1,    38,    -1,    -1,   181,   180,    -1,    50,   203,
     182,   152,    35,    -1,   183,    -1,   183,    83,   203,   197,
      -1,   196,    -1,   198,    -1,    66,    -1,    63,    -1,    38,
      -1,   184,   107,   185,   108,    -1,   184,   107,   185,    19,
     108,    -1,     9,    -1,    16,    -1,   117,   203,    16,    -1,
      67,    -1,    64,    -1,    39,    -1,   186,   185,    -1,    -1,
     187,   188,    -1,   162,   115,   187,   188,    -1,   197,    -1,
     110,    -1,   109,    -1,    17,   203,   126,    -1,    -1,   203,
     191,   190,    35,   203,    -1,    31,   152,    -1,    -1,   192,
      -1,   192,    18,   191,    -1,   125,    61,   203,   152,    -1,
     125,    47,   125,    61,   152,    -1,   194,    18,   194,    -1,
     194,    18,   193,    -1,   125,   202,    -1,   125,    47,   125,
     202,    -1,   125,   202,    61,   152,    -1,   125,    47,   125,
      61,   152,    -1,   152,    -1,   152,    18,   195,    -1,     8,
      -1,    15,    -1,   197,    -1,   117,   203,   197,    -1,    14,
      -1,    11,    -1,    12,    -1,    10,    -1,    -1,    -1,    77,
     203,   197,   175,   180,   205,    35,   203,    -1,   206,    -1,
     207,    -1,   209,    -1,   206,   205,    -1,   207,   205,    -1,
     209,   205,    -1,    73,   196,    83,   208,    35,    -1,    73,
     197,    83,   208,    35,    -1,    73,   208,   152,    35,    -1,
      72,    -1,    14,    -1,    74,   197,   210,    35,    -1,    -1,
     211,   210,    -1,    42,   217,    -1,   207,    -1,   209,    -1,
      75,   203,   197,   175,   180,   214,   213,    35,   203,    -1,
     241,    -1,   219,    -1,   241,   213,    -1,   219,   213,    -1,
      -1,    79,   215,    -1,   216,    -1,   216,   215,    -1,   196,
      -1,   196,   115,   126,    -1,   197,    -1,   197,   217,    -1,
     219,    -1,   219,   218,    -1,    -1,    76,   197,    83,   220,
     223,   238,   239,   244,    35,    -1,    -1,    -1,    76,   110,
     221,    83,   222,   223,   238,   239,   244,    35,    -1,    76,
     223,   238,   239,   244,    35,    -1,   225,   197,   236,    -1,
     197,   237,    -1,   224,   226,    -1,   225,    -1,    -1,   196,
     115,    -1,    -1,    -1,   107,   227,   233,   108,   228,   236,
      -1,    -1,    -1,   111,   229,   233,   112,   230,   236,    -1,
      -1,    -1,   113,   231,   233,   114,   232,   236,    -1,   234,
      -1,   234,   235,   233,    -1,   197,    -1,   109,    -1,    81,
      -1,    -1,   237,    -1,    96,    -1,    98,    -1,    -1,    -1,
     240,    47,    -1,   241,    -1,   241,   240,    -1,    78,   196,
     244,    35,    -1,    78,   197,   244,    35,    -1,    78,   262,
     107,   242,   108,   244,    35,    -1,    -1,   243,   242,    -1,
     197,    -1,   110,    -1,   109,    -1,   245,    -1,   246,    -1,
     246,    18,   245,    -1,   202,   248,    -1,    60,   203,   247,
      -1,    -1,    80,   152,    -1,   249,   248,    -1,   249,   237,
     203,   250,   238,    -1,   249,    83,   252,   250,    -1,    47,
     250,    -1,   253,   250,    -1,   247,    -1,   197,    -1,   247,
      -1,   251,   250,    -1,   198,    83,   252,    -1,   252,    -1,
     197,    -1,   197,   237,   203,   238,    -1,   254,    -1,   117,
     203,   197,    83,   252,    -1,   254,    -1,   261,    -1,   225,
     203,   261,   236,   238,    -1,   261,   237,   203,   238,    -1,
      -1,    -1,   224,   203,   107,   255,   263,   108,   256,   236,
     238,    -1,    -1,    -1,   224,   203,   111,   257,   263,   112,
     258,   236,   238,    -1,    -1,    -1,   224,   203,   113,   259,
     263,   114,   260,   236,   238,    -1,   196,    -1,   262,   203,
     107,   153,   108,    -1,     9,    -1,    16,    -1,   244,    -1,
     244,   235,   263,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   750,   750,   752,   756,   758,   761,   763,   768,   770,
     773,   775,   779,   782,   784,   786,   788,   793,   794,   798,
     805,   814,   816,   820,   822,   830,   832,   834,   836,   839,
     841,   843,   845,   847,   853,   855,   859,   862,   865,   868,
     870,   873,   876,   879,   882,   884,   887,   889,   891,   893,
     895,   897,   899,   901,   903,   905,   907,   909,   911,   913,
     917,   919,   922,   925,   927,   929,   931,   933,   935,   937,
     939,   941,   943,   945,   947,   949,   951,   953,   955,   957,
     959,   961,   963,   965,   969,   974,   975,   979,   981,   983,
     985,   989,   991,   993,   996,  1002,  1003,  1007,  1012,  1013,
    1017,  1021,  1024,  1038,  1062,  1063,  1068,  1069,  1075,  1076,
    1081,  1083,  1085,  1088,  1090,  1092,  1094,  1100,  1101,  1103,
    1107,  1111,  1113,  1115,  1117,  1122,  1123,  1128,  1129,  1131,
    1136,  1140,  1144,  1148,  1152,  1156,  1160,  1162,  1167,  1168,
    1172,  1174,  1181,  1182,  1187,  1188,  1192,  1197,  1204,  1206,
    1208,  1210,  1214,  1219,  1220,  1222,  1227,  1228,  1232,  1234,
    1236,  1238,  1240,  1242,  1246,  1248,  1252,  1256,  1258,  1260,
    1262,  1266,  1270,  1274,  1276,  1278,  1280,  1284,  1286,  1288,
    1292,  1294,  1298,  1302,  1304,  1307,  1311,  1313,  1315,  1317,
    1323,  1328,  1330,  1336,  1337,  1341,  1343,  1345,  1347,  1352,
    1353,  1357,  1359,  1363,  1365,  1367,  1369,  1371,  1373,  1378,
    1379,  1383,  1387,  1389,  1393,  1395,  1397,  1399,  1401,  1403,
    1405,  1409,  1411,  1413,  1415,  1417,  1419,  1423,  1426,  1429,
    1431,  1435,  1437,  1439,  1444,  1447,  1450,  1454,  1457,  1460,
    1462,  1466,  1468,  1472,  1474,  1478,  1482,  1484,  1487,  1491,
    1493,  1497,  1501,  1505,  1507,  1511,  1515,  1517,  1521,  1526,
    1530,  1539,  1547,  1549,  1551,  1553,  1555,  1557,  1561,  1563,
    1567,  1571,  1573,  1577,  1582,  1583,  1587,  1589,  1591,  1597,
    1605,  1607,  1609,  1611,  1616,  1617,  1621,  1623,  1627,  1629,
    1633,  1635,  1639,  1641,  1646,  1645,  1649,  1650,  1649,  1653,
    1657,  1659,  1661,  1665,  1666,  1669,  1673,  1674,  1673,  1675,
    1676,  1675,  1677,  1678,  1677,  1681,  1683,  1687,  1688,  1691,
    1695,  1696,  1699,  1700,  1704,  1712,  1713,  1717,  1719,  1723,
    1725,  1727,  1732,  1733,  1737,  1739,  1741,  1745,  1749,  1751,
    1755,  1764,  1769,  1770,  1774,  1776,  1786,  1791,  1798,  1800,
    1804,  1808,  1810,  1814,  1816,  1820,  1822,  1828,  1832,  1835,
    1840,  1842,  1846,  1850,  1851,  1850,  1854,  1855,  1854,  1858,
    1859,  1858,  1864,  1866,  1870,  1872,  1877,  1879
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
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
  "T_TRUE_LABEL", "T_try", "T_unit", "T_UNIT_LABEL", "T_for", "T_FOR",
  "T_do", "T_ENDOFFILE", "T_REGEX", "T_lex", "T_mode", "T_parser",
  "T_prod", "T_scanner", "T_syn", "T_token", "T_REDUCE", "T_SEP", "T_ITER",
  "'='", "T_OOASSIGN", "T_DOTASSIGN", "T_COLONEQUALS", "T_orelse",
  "T_andthen", "T_RMACRO", "T_LMACRO", "T_FDCOMPARE", "T_COMPARE",
  "T_FDIN", "'|'", "'#'", "T_ADD", "T_OTHERMUL", "T_FDMUL", "','", "'~'",
  "'.'", "'^'", "'@'", "T_DEREFF", "'+'", "'-'", "'('", "')'", "'_'",
  "'$'", "'['", "']'", "'{'", "'}'", "':'", "';'", "'!'", "$accept",
  "file", "queries", "queries1", "directive", "switchList", "switch",
  "sequence", "phrase", "hashes", "phrase2", "FOR_decls", "FOR_decl",
  "FOR_gen", "FOR_genOptInt", "FOR_genOptC", "FOR_genOptC2", "iterators",
  "iterator", "optIteratorStep", "procFlags", "optFunctorDescriptorList",
  "functorDescriptorList", "importDecls", "variableLabel", "featureList",
  "optImportAt", "exportDecls", "compare", "fdCompare", "fdIn", "add",
  "fdMul", "otherMul", "inSequence", "phraseList", "fixedListArgs",
  "optCatch", "optFinally", "record", "recordAtomLabel", "recordVarLabel",
  "recordArguments", "optDots", "feature", "featureNoVar", "ifMain",
  "ifRest", "caseMain", "caseRest", "elseOfList", "caseClauseList",
  "caseClause", "sideCondition", "pattern", "class", "phraseOpt",
  "classDescriptorList", "classDescriptor", "attrFeatList", "attrFeat",
  "attrFeatFeature", "methList", "meth", "methHead", "methHead1",
  "methHeadLabel", "methFormals", "methFormal", "methFormalTerm",
  "methFormalOptDefault", "condMain", "condElse", "condClauseList",
  "condClause", "orClauseList", "orClause", "choiceClauseList", "atom",
  "nakedVariable", "variable", "string", "int", "float", "thisCoord",
  "coord", "scannerSpecification", "scannerRules", "lexAbbrev", "lexRule",
  "regex", "modeClause", "modeDescrs", "modeDescr", "parserSpecification",
  "parserRules", "tokenClause", "tokenList", "tokenDecl", "modeFromList",
  "prodClauseList", "prodClause", "@1", "@2", "@3", "prodHeadRest",
  "prodName", "prodNameAtom", "prodKey", "@4", "@5", "@6", "@7", "@8",
  "@9", "prodParams", "prodParam", "separatorOp", "optTerminatorOp",
  "terminatorOp", "prodMakeKey", "localRules", "localRulesSub",
  "synClause", "synParams", "synParam", "synAlt", "synSeqs", "synSeq",
  "optSynAction", "nonEmptySeq", "synVariable", "synPrims", "synPrim",
  "synPrimNoAssign", "synPrimNoVar", "synPrimNoVarNoAssign", "@10", "@11",
  "@12", "@13", "@14", "@15", "synInstTerm", "synLabel",
  "synProdCallParams", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
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
static const unsigned short int yyr1[] =
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
     135,   135,   136,   136,   137,   137,   138,   138,   139,   139,
     140,   140,   140,   140,   140,   140,   140,   141,   141,   141,
     142,   143,   143,   143,   143,   144,   144,   145,   145,   145,
     146,   147,   148,   149,   150,   151,   152,   152,   153,   153,
     154,   154,   155,   155,   156,   156,   157,   157,   158,   158,
     158,   158,   159,   160,   160,   160,   161,   161,   162,   162,
     162,   162,   162,   162,   163,   163,   164,   165,   165,   165,
     165,   166,   166,   167,   167,   167,   167,   168,   168,   168,
     169,   169,   170,   171,   171,   171,   172,   172,   172,   172,
     173,   174,   174,   175,   175,   176,   176,   176,   176,   177,
     177,   178,   178,   179,   179,   179,   179,   179,   179,   180,
     180,   181,   182,   182,   183,   183,   183,   183,   183,   183,
     183,   184,   184,   184,   184,   184,   184,   185,   185,   186,
     186,   187,   187,   187,   188,   188,   189,   190,   190,   191,
     191,   192,   192,   193,   193,   194,   194,   194,   194,   195,
     195,   196,   197,   198,   198,   199,   200,   200,   201,   202,
     203,   204,   205,   205,   205,   205,   205,   205,   206,   206,
     207,   208,   208,   209,   210,   210,   211,   211,   211,   212,
     213,   213,   213,   213,   214,   214,   215,   215,   216,   216,
     217,   217,   218,   218,   220,   219,   221,   222,   219,   219,
     223,   223,   223,   224,   224,   225,   227,   228,   226,   229,
     230,   226,   231,   232,   226,   233,   233,   234,   234,   235,
     236,   236,   237,   237,   238,   239,   239,   240,   240,   241,
     241,   241,   242,   242,   243,   243,   243,   244,   245,   245,
     246,   246,   247,   247,   248,   248,   248,   248,   248,   248,
     249,   250,   250,   251,   251,   252,   252,   252,   253,   253,
     254,   254,   254,   255,   256,   254,   257,   258,   254,   259,
     260,   254,   261,   261,   262,   262,   263,   263
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
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
       2,     1,     8,     6,     0,     2,     0,     2,     0,     1,
       4,     6,     4,     4,     4,     6,     4,     0,     3,     6,
       2,     1,     3,     2,     4,     0,     2,     0,     2,     4,
       1,     1,     1,     1,     1,     1,     4,     1,     0,     2,
       1,     3,     0,     3,     0,     2,     7,     7,     1,     1,
       1,     1,     1,     0,     2,     4,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     6,     2,     2,     3,
       1,     7,     6,     2,     2,     3,     1,     1,     3,     3,
       1,     3,     3,     1,     4,     6,     4,     4,     1,     4,
       7,     1,     1,     0,     2,     4,     4,     4,     4,     0,
       2,     3,     1,     1,     1,     1,     1,     1,     1,     0,
       2,     5,     1,     4,     1,     1,     1,     1,     1,     4,
       5,     1,     1,     3,     1,     1,     1,     2,     0,     2,
       4,     1,     1,     1,     3,     0,     5,     2,     0,     1,
       3,     4,     5,     3,     3,     2,     4,     4,     5,     1,
       3,     1,     1,     1,     3,     1,     1,     1,     1,     0,
       0,     8,     1,     1,     1,     2,     2,     2,     5,     5,
       4,     1,     1,     4,     0,     2,     2,     1,     1,     9,
       1,     1,     2,     2,     0,     2,     1,     2,     1,     3,
       1,     2,     1,     2,     0,     9,     0,     0,    10,     6,
       3,     2,     2,     1,     0,     2,     0,     0,     6,     0,
       0,     6,     0,     0,     6,     1,     3,     1,     1,     1,
       0,     1,     1,     1,     0,     0,     2,     1,     2,     4,
       4,     7,     0,     2,     1,     1,     1,     1,     1,     3,
       2,     3,     0,     2,     2,     5,     4,     2,     2,     1,
       1,     1,     2,     3,     1,     1,     4,     1,     5,     1,
       1,     5,     4,     0,     0,     9,     0,     0,     9,     0,
       0,     9,     1,     5,     1,     1,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short int yydefact[] =
{
       0,     3,    17,    14,    15,    16,   251,   148,   258,   256,
     257,   255,   252,   152,   260,   260,   260,   260,   260,   260,
      74,    52,   151,   260,   260,   260,   260,   260,   260,   260,
     260,   260,    53,    73,   260,    51,   150,   260,    50,   149,
     260,   260,   260,   304,   260,   260,   260,   260,   260,     0,
      49,    54,   260,   260,   260,     0,     0,     6,   259,    12,
      21,    32,    58,   260,   260,    64,    47,   253,    48,    55,
      56,    57,     0,    80,    81,    12,   292,     0,     0,    13,
      17,    67,     0,     0,   259,    76,     0,     0,     0,   106,
     259,    66,     0,     0,     0,     0,     0,   106,     0,     0,
       0,     0,    85,     0,   296,     0,     0,   324,     0,   303,
       0,   138,     0,     0,     0,   137,     0,     0,     0,     0,
       1,     2,     8,     4,   260,   260,   260,   260,   260,   131,
     130,   132,   260,    22,   260,   260,   260,    42,   260,   133,
     135,   134,   260,   260,   260,   260,   260,   260,     0,     0,
     260,   260,   260,   260,   260,    12,     5,   293,    19,    20,
      18,     0,   249,     0,   191,   193,   192,     0,   238,   239,
     259,   259,     0,     0,     0,   106,   108,     0,     0,    21,
       0,     0,     0,     0,     0,     0,   142,     0,   101,     0,
       0,     0,    85,    87,   193,     0,   305,   294,   322,   323,
     301,   325,   306,   309,   312,   302,   320,   193,   138,     0,
      40,    44,    45,   260,    46,   259,   138,   254,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   153,   153,     0,   127,   117,     0,
     117,     7,     0,   260,     0,   260,   260,   260,   260,   260,
     209,   193,     0,   260,     0,     0,     0,   259,    12,     0,
     245,   260,     0,     0,   107,     0,   109,     0,     0,     0,
     260,   260,   260,     0,   260,   260,   260,   144,     0,   100,
     260,     0,     0,     0,    86,     0,   209,   297,   304,     0,
     259,     0,   327,     0,     0,     0,   300,   321,   209,   139,
     260,     0,     0,   140,     0,    23,    25,    24,    26,    27,
      31,    28,    29,    30,    33,    34,    39,    41,    43,    36,
      37,    38,    52,    51,    50,   153,   156,     0,    47,   253,
      56,   156,   108,   108,     0,   164,   127,   165,   260,   108,
       0,   125,   108,   108,   188,     0,   177,     0,   183,     0,
     250,    79,     0,     0,     0,     0,   260,     0,   209,   194,
       0,     0,   237,   260,   240,    12,    12,    11,   259,     0,
      78,   244,   243,   138,   260,     0,     0,     0,    68,    75,
      77,   138,    72,    70,     0,     0,     0,     0,     0,    90,
       0,    91,    89,     0,    88,   284,   304,     0,   324,   374,
     375,   259,   259,     0,   260,   342,     0,   337,   338,   326,
     328,   318,   317,     0,   315,     0,     0,     0,    82,   136,
     260,   259,   260,     0,   154,   157,     0,     0,     0,     0,
     116,   114,     0,   128,   120,   113,     0,     0,   117,     0,
     112,   110,   260,     0,   260,   260,   176,   260,     0,     0,
       0,   260,   259,   260,     0,   208,   207,   206,   199,   202,
     203,   204,   205,   199,   138,   138,     0,   260,   210,     0,
     241,   236,    10,     9,     0,   246,   247,     0,    63,     0,
     260,   260,   170,   260,    65,   260,     0,   143,   180,   145,
     260,   260,   104,    21,     0,     0,   260,     0,     0,   324,
     325,     0,     0,   332,   342,   342,     0,   260,   372,   350,
     260,   260,   349,   340,   342,   342,   359,   360,   260,   299,
     259,   307,   319,     0,   310,   313,     0,     0,     0,   262,
     263,   264,    59,   141,    60,    35,   260,   153,   260,   108,
     127,     0,   121,   126,   118,   108,     0,     0,   174,   173,
     172,   178,   179,   182,     0,     0,     0,   260,   196,   199,
       0,   197,   195,   198,   221,   222,   218,   226,   217,   225,
     216,   224,   260,     0,   212,     0,   214,   215,   190,   242,
     248,     0,     0,   168,   167,   166,    69,     0,     0,    71,
      83,     0,     0,   260,     0,    95,    98,    93,    84,   288,
     285,   286,     0,   281,   280,   325,   259,   329,   330,   336,
     335,   334,     0,   332,   341,   355,     0,   351,   347,   342,
     354,   357,   343,     0,     0,     0,   304,   260,   344,   348,
     260,     0,   339,   320,   316,   320,   320,   272,   271,     0,
       0,     0,   274,   260,   265,   266,   267,   146,   155,   147,
     115,   129,   125,     0,   123,   111,   189,   175,   186,   184,
     187,   171,   200,   201,     0,     0,   260,   228,     0,   169,
       0,   181,   104,   105,   103,     0,     0,    92,     0,    97,
       0,   287,   260,   283,   282,   259,     0,   259,   333,   260,
     304,   352,     0,   363,   366,   369,   372,   320,   355,   342,
     342,   324,   138,   308,   311,   314,     0,     0,     0,     0,
       0,   277,   278,     0,   274,   261,   117,   122,     0,   223,
     211,     0,   163,   162,   161,   233,   232,     0,     0,   228,
     235,   158,   231,   160,   260,   260,   260,    94,    96,    99,
     289,   279,     0,   295,     0,   324,   353,   304,   259,   259,
     259,   324,   346,   324,   362,     0,     0,     0,   270,   290,
     276,   273,   275,   119,   124,   185,   213,     0,     0,   219,
     227,   260,   229,    62,    61,   102,   298,   331,   356,   358,
     376,     0,     0,     0,   361,   345,   373,   268,   269,   291,
     235,   231,   220,     0,   259,   364,   367,   370,   230,   234,
     377,   320,   320,   320,   324,   324,   324,   365,   368,   371
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,    55,    56,    57,    58,    79,    80,   115,    60,   314,
      61,   191,   192,   392,   677,   597,   679,   187,   188,   593,
     174,   265,   266,   339,   340,   541,   438,   333,   134,   135,
     136,   145,   146,   147,   116,   209,   302,   277,   386,    62,
      63,    64,   326,   426,   327,   334,    91,   483,    81,   447,
     345,   487,   346,   347,   348,    65,   165,   250,   251,   558,
     559,   459,   357,   358,   573,   574,   575,   728,   729,   730,
     772,    85,   255,   168,   169,   172,   173,   163,    66,    67,
      68,    69,    70,    71,   405,    82,    73,   528,   529,   530,
     641,   531,   713,   714,    74,   602,   498,   600,   601,   760,
      75,    76,   288,   195,   396,   107,   510,   511,   205,   293,
     633,   294,   635,   295,   636,   413,   414,   523,   296,   297,
     201,   290,   291,   292,   612,   613,   780,   407,   408,   617,
     513,   514,   618,   619,   620,   515,   621,   748,   801,   749,
     802,   750,   803,   517,   518,   781
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -584
static const short int yypact[] =
{
    1151,  -584,   248,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,    50,  -584,  -584,  -584,  -584,  -584,  1814,
    -584,  -584,  -584,  -584,  -584,    86,    25,  -584,  1264,   543,
    1594,   388,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,   346,  -584,  -584,   543,   -13,   112,   130,  -584,
     248,  -584,  1814,  1814,  1814,  -584,  1814,  1814,  1814,   137,
    1814,  -584,  1814,  1814,  1814,  1814,  1814,   137,  1814,  1814,
    1814,   188,  1814,   188,  -584,   106,   168,  -584,   232,   188,
     188,  1814,  1814,  1814,  1814,   164,   117,  1814,  1814,   188,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,   141,   151,
    -584,  -584,  -584,  -584,  -584,   543,  -584,  -584,  -584,  -584,
    -584,   210,   277,   263,   910,   265,  -584,   152,   275,   294,
     283,   319,   287,   363,   270,   137,   346,   325,   344,  1374,
     359,   364,   368,   293,   376,   380,   405,   171,  -584,   379,
     330,   372,  1814,   941,   265,   381,  -584,  -584,  -584,  -584,
    -584,   373,  -584,  -584,  -584,  -584,     8,   265,  1594,   370,
      48,  -584,  -584,  -584,  -584,   910,  1594,  -584,  1814,  1814,
    1814,  1814,  1814,  1814,  1814,  1814,  1814,  1814,  1814,  1814,
    1814,  1814,  1814,  1814,  1924,  1924,  1814,   445,   347,  1814,
     347,  -584,  1814,  -584,  1814,  -584,  -584,  -584,  -584,  -584,
     430,   265,  1814,  -584,  1814,   447,  1814,  1814,   543,  1814,
     432,  -584,  1814,  1814,  -584,   456,  -584,  1814,  1814,  1814,
    -584,  -584,  -584,  1814,  -584,  -584,  -584,   461,  1814,  -584,
    -584,  1814,  2034,  1814,  -584,  1814,   430,  -584,   302,   595,
     450,   465,   373,    53,    53,    53,  -584,  -584,   430,  -584,
    -584,  1814,   410,  1814,   431,   910,   353,   442,   746,   502,
     457,   631,   303,   303,  -584,   605,   202,  -584,  -584,   227,
     202,   202,   444,   448,   449,  1704,   541,   451,   459,   460,
     464,   541,   618,   346,   468,  -584,   445,  -584,  -584,   346,
     473,   550,   632,   346,   718,   523,    67,   525,   290,  1814,
    -584,  -584,   176,   176,  1814,  1814,  -584,   549,   430,  -584,
     537,  1814,  -584,  -584,  -584,   543,   543,  -584,   539,  1814,
    -584,  -584,   363,  1594,  -584,   592,   570,   572,  -584,  -584,
    -584,  1594,  -584,  -584,  1814,  1814,   573,   578,  1814,   910,
    1814,   808,  -584,   586,   910,   547,   302,     8,  -584,  -584,
    -584,   450,   450,   522,  -584,    44,   596,  -584,   624,  -584,
    -584,  -584,  -584,   538,   564,   554,   548,   361,  -584,  -584,
    -584,   910,  -584,  1814,  -584,  -584,   545,  1814,   561,  1814,
    -584,  -584,   188,  -584,  -584,  -584,   532,   137,   347,  1814,
    -584,  -584,  -584,  1814,  -584,  -584,  -584,  -584,  1814,  1814,
    1814,  -584,  -584,  -584,   523,  -584,  -584,  -584,   176,   540,
    -584,  -584,  -584,   176,  1594,  1594,   553,  -584,  -584,  1814,
    -584,  -584,  -584,  -584,  1814,  -584,  -584,   557,  -584,  1814,
    -584,  -584,  -584,  -584,  -584,  -584,   559,  -584,   662,  -584,
    -584,  -584,   829,  1484,  1814,  1814,  -584,   137,   262,  -584,
     373,   647,   648,   126,   604,    63,  1814,  -584,   106,  -584,
    -584,   394,  -584,  -584,   764,    63,  -584,     8,  -584,  -584,
     450,  -584,  -584,    53,  -584,  -584,    61,   188,   654,   361,
     361,   361,  -584,  -584,  -584,  -584,  -584,  1704,  -584,   346,
     445,   583,   114,  -584,  -584,   346,  1814,   658,  -584,  -584,
    -584,  -584,  -584,  -584,  1814,  1814,  1814,  -584,  -584,   176,
    1814,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,  1814,   611,   590,  -584,  -584,  -584,  -584,
    -584,  1814,   663,  -584,  -584,  -584,  -584,  1814,  1814,  -584,
    -584,  1814,  1814,  -584,  1814,   706,   847,  -584,  -584,   593,
    -584,   137,   674,   262,   262,   373,   450,  -584,  -584,  -584,
    -584,  -584,   606,   126,  -584,   192,   629,  -584,  -584,    63,
    -584,  -584,  -584,   188,   402,   311,   595,  -584,  -584,  -584,
    -584,   609,  -584,     8,  -584,     8,     8,  -584,  -584,   630,
     635,  1814,   240,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,   550,   188,  -584,  -584,  -584,  -584,    20,   680,
     634,  -584,  -584,   910,   404,   697,  -584,   722,   700,  -584,
     703,  -584,   884,   910,  -584,   633,  1814,  -584,  1814,  -584,
    1814,  -584,  -584,  -584,  -584,   450,   705,   450,  -584,  -584,
     595,  -584,   659,  -584,  -584,  -584,  -584,     8,     8,    63,
      63,  -584,  1814,  -584,  -584,  -584,    70,    70,   708,   188,
      70,  -584,  -584,   709,   240,  -584,   347,   532,  1814,  -584,
    -584,   188,  -584,  -584,  -584,  -584,  -584,   636,    51,   722,
     728,  -584,   460,  -584,  -584,  -584,  -584,  -584,   910,   910,
     910,  -584,   711,  -584,   713,  -584,  -584,   595,   450,   450,
     450,  -584,  -584,  -584,  -584,   642,   720,   724,  -584,   188,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,   182,   645,  -584,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
     564,   655,   652,   651,  -584,  -584,  -584,  -584,  -584,  -584,
     728,  -584,  -584,  1814,   450,  -584,  -584,  -584,  -584,   910,
    -584,     8,     8,     8,  -584,  -584,  -584,  -584,  -584,  -584
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -584,  -584,   712,    14,  -584,   686,  -584,   108,   414,  -379,
     104,   575,  -584,  -584,  -584,   183,  -584,  -584,   589,   109,
     -31,  -292,   710,  -219,  -584,  -497,   132,  -331,  -584,  -584,
    -584,  -584,  -584,  -584,     0,  -188,   365,  -584,  -584,  -584,
    -584,  -584,  -228,   470,  -552,  -402,  -202,  -584,  -199,   349,
      29,   217,  -378,  -584,  -286,  -584,   717,    43,  -584,  -401,
      96,  -584,  -206,  -584,  -584,  -584,  -584,    79,  -584,    45,
      33,  -584,  -584,   568,  -584,   -50,   563,   566,   422,    62,
     291,  -584,  -147,  -584,   284,   -15,  -584,   -58,  -584,  -561,
    -189,  -560,   116,  -584,  -584,   -65,  -584,   234,  -584,    77,
     766,  -421,  -584,  -584,  -584,  -221,   -25,    -8,  -584,  -584,
    -584,  -584,  -584,  -584,  -584,  -271,  -584,    65,  -579,   -89,
    -343,  -426,   551,  -396,   233,  -584,  -254,   328,  -584,  -312,
     337,  -584,  -473,  -584,  -583,  -584,  -341,  -584,  -584,  -584,
    -584,  -584,  -584,   228,   565,  -476
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -305
static const short int yytable[] =
{
      83,    84,    86,    87,    88,   433,   488,   331,    89,    90,
      92,    93,    94,    95,    96,    97,    98,   200,   108,    99,
     299,   343,   100,   415,   416,   101,   102,   103,   304,   110,
     111,   112,   113,   114,   542,   109,   406,   117,   118,   119,
     430,   431,   629,   699,   535,   654,   182,   435,   148,   149,
     440,   441,     6,   399,   703,   500,   704,   705,     6,    12,
     400,   137,   561,    43,   516,    12,   183,   398,    12,     6,
     768,     6,   399,   123,   606,   637,    12,   603,    12,   400,
     395,   711,   712,   162,   637,   448,   120,   330,   330,   156,
     337,   505,   417,   512,   180,   181,   121,   424,   184,   185,
     186,   449,   604,   451,   198,   106,   199,   746,    59,   218,
     219,   220,   221,   222,   453,   727,   158,   223,   751,   224,
     225,   226,     6,   227,   506,     9,    10,   228,   229,   230,
     231,   232,   233,   638,   159,   236,   237,   238,   239,   240,
     542,    12,   638,   506,   264,     6,   691,   501,   502,   143,
     144,  -304,   468,   711,   712,  -304,   605,  -304,   662,   769,
     104,   507,   411,   189,   779,   194,    59,   656,   133,   241,
    -304,   206,   207,   516,  -304,   499,  -304,   727,   330,   685,
      54,   217,   603,   603,     6,   477,    12,     9,    10,   337,
     161,    12,   614,   486,   167,   170,   171,    12,   301,   252,
     177,   178,   512,    12,   171,   462,   462,   604,   604,   651,
     488,   213,   371,   253,   455,   137,   210,   211,   212,   544,
     764,   196,   804,   805,   806,   214,   752,   753,   349,   653,
     351,   352,   353,   354,   355,   609,   610,   286,   361,   456,
     137,   278,   457,   549,   162,   548,   370,   650,   234,   189,
     298,   197,   634,   655,   362,   378,   379,   380,   235,   382,
     383,   384,   242,   108,   198,   388,   199,   375,   658,   377,
     660,   243,   367,   782,   783,  -253,   562,   563,   387,   584,
     109,   583,   709,   393,    72,   418,   246,   133,   198,   337,
     199,   725,   726,    54,   359,   244,   329,   329,   245,   336,
     341,   142,   341,   143,   144,   247,   254,   248,   200,   648,
       6,   462,   256,   710,   527,   542,   462,    12,   800,     6,
     399,   249,   261,   434,   140,   141,   142,   400,   143,   144,
     257,   315,   316,   317,   318,   319,   320,   321,    43,   202,
     289,   466,    72,   203,   332,   204,   344,   342,   471,   454,
     397,   402,   686,    77,    78,   412,   412,   412,   754,   478,
     360,   470,    12,   338,   167,   365,   259,   368,   166,   476,
     171,   108,   281,   451,   166,   150,   376,   282,   452,   472,
     473,   262,   151,   263,   453,   489,   267,   329,   109,   504,
     330,   268,   152,   337,   270,   337,  -305,   132,   336,   271,
     153,   137,   778,   272,   154,   532,   273,   534,   784,   419,
     785,   274,   462,   124,   125,   275,   126,   127,   128,    12,
     719,   129,   130,   131,   132,   627,   280,   546,   630,   276,
      92,   742,   550,   744,   526,   527,   554,   125,   556,   126,
     127,   128,   283,   547,   129,   130,   131,   132,   458,   463,
     553,   289,   578,     6,   258,   260,     9,    10,   397,   300,
      12,   807,   808,   809,   287,   105,    92,   509,   585,   579,
     586,   644,   645,   646,   580,   589,   590,   551,   552,   582,
     356,   598,   363,   138,   139,   140,   141,   142,   344,   143,
     144,   374,   623,   369,   540,   624,   625,   763,   164,   303,
     341,  -303,   385,   631,   164,  -303,   622,  -303,   179,   693,
     404,   175,   409,   694,   755,   695,   190,   756,   757,   175,
     733,   647,   420,   649,   193,   208,   689,   315,   126,   127,
     128,   215,   216,   129,   130,   131,   132,   539,   683,   684,
       6,   366,   661,     9,    10,   422,     2,   545,     3,     4,
       5,   132,   344,   344,   443,   444,   445,   664,   446,  -163,
     425,     6,   564,  -162,  -161,   611,   427,   615,    12,   565,
     337,    18,   437,   665,  -158,  -159,   509,   615,   674,  -160,
     436,   668,   733,   432,   467,   412,   450,   670,   640,   642,
     128,   566,   567,   129,   130,   131,   132,   175,   469,   329,
     474,   133,   336,     6,   399,   484,   190,   485,   490,   689,
      12,   400,   700,   491,   193,   701,   568,   569,   137,   570,
     571,   496,   208,   479,   480,   481,   497,   482,   715,   503,
     208,   519,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   708,   520,   461,   461,   522,   521,   150,   325,   325,
     315,   721,   475,   536,   151,   560,   328,   328,   344,   335,
     344,   150,   525,   659,   152,   429,   524,   741,   151,   538,
     572,   581,   153,   587,   745,   611,   154,   373,   152,   439,
     588,   615,   607,   608,   506,   692,   153,   381,   698,   643,
     154,   652,   344,   657,   666,   389,   391,   667,   669,   394,
     423,   139,   140,   141,   142,   303,   143,   144,   680,   682,
     105,   401,   690,   706,   687,   717,   702,   421,   707,   773,
     774,   775,  -305,  -305,   131,   132,   217,   718,   453,   732,
       6,   137,   720,     9,    10,   734,   555,    12,   735,   325,
     743,   737,   747,   758,   761,   771,   776,   328,   777,   461,
     786,   767,   698,   792,   461,   787,   793,   577,   335,   788,
     722,   615,   615,   795,   796,   797,   160,   284,   464,   465,
     122,   759,     6,   399,   460,   460,   279,   675,   341,    12,
     400,   736,   155,   766,   716,   723,   533,   208,   724,   124,
     125,   732,   126,   127,   128,   208,   616,   129,   130,   131,
     132,   428,   492,   557,   493,   671,   616,   176,   770,   698,
     350,   505,   790,   442,   139,   140,   141,   142,   105,   143,
     144,   759,   676,   798,   364,   372,   765,   508,   494,   791,
     762,   725,   726,   127,   128,   681,   789,   129,   130,   131,
     132,   537,   157,   410,   506,   794,   688,   626,   632,   591,
     461,   628,     0,   697,   403,     0,     0,     0,   335,   543,
     198,     0,   199,     0,     0,     0,     0,     0,     0,     0,
       0,  -304,     0,     0,     0,  -304,     0,  -304,   208,   208,
     460,   507,     0,     0,     0,   460,     0,     0,   576,     0,
       0,   124,   125,     0,   126,   127,   128,     0,     0,   129,
     130,   131,   132,     0,     0,     0,     0,     0,   595,   596,
     616,     0,   124,   125,     0,   126,   127,   128,     0,   599,
     129,   130,   131,   132,   495,     0,     0,   508,     0,     0,
     124,   125,     0,   126,   127,   128,   508,   508,   129,   130,
     131,   132,     0,     0,     0,   592,     0,     0,   639,     0,
       0,   325,     0,     0,   -47,     0,     0,     0,     0,   328,
       0,     0,   335,   678,   335,     0,     0,   124,   125,     0,
     126,   127,   128,     0,   663,   129,   130,   131,   132,     0,
       0,   460,     0,   -47,     0,     0,     0,     0,   -47,     0,
     616,   616,     0,   124,   125,     0,   126,   127,   128,     0,
     592,   129,   130,   131,   132,   672,   673,     0,   596,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   599,   -47,   -47,     0,   -47,   -47,   -47,
       0,     0,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,
     -47,   508,   -47,   -47,     0,     0,     0,   696,   508,     0,
       0,     0,     0,     0,     0,     0,   285,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   731,
     738,     0,   739,     0,   740,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   508,     0,     0,     0,   208,     0,     0,     0,
       0,   508,   508,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   335,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   731,     1,     0,     2,     0,     3,     4,     5,     6,
       7,     8,     9,    10,     0,    11,    12,    13,     0,   508,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
    -259,    19,     0,     0,     0,     0,     0,  -259,    20,    21,
      22,     0,     0,     0,    23,    24,    25,  -259,     0,    26,
      27,     0,    28,     0,    29,  -259,    30,   799,    31,  -259,
      32,    33,     0,    34,    35,    36,    37,    38,    39,    40,
      41,     0,   -12,     0,     0,     0,    42,    43,    44,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,     0,    47,    48,     0,     0,    49,     0,
      50,    51,    52,     0,    53,     0,     0,     2,    54,     3,
       4,     5,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,     0,    19,     0,     0,     0,     0,     0,
       0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
       0,    31,     0,    32,    33,     0,    34,    35,    36,    37,
      38,    39,    40,    41,     0,   -12,     0,     0,     0,    42,
      43,    44,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
       0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
       0,    54,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
       0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
       0,    31,     0,    32,    33,   269,    34,    35,    36,    37,
      38,    39,    40,    41,     0,     0,     0,     0,     0,    42,
       0,    44,     0,     0,     0,     0,     0,   124,   125,     0,
     126,   127,   128,     0,    45,   129,   130,   131,   132,     0,
       0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
       0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
       0,    54,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
       0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
       0,    31,     0,    32,    33,     0,    34,    35,    36,    37,
      38,    39,    40,    41,     0,     0,     0,     0,     0,    42,
       0,    44,     0,     0,     0,     0,     0,   124,   125,     0,
     126,   127,   128,     0,    45,   129,   130,   131,   132,     0,
       0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
       0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
     594,    54,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
       0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
       0,    31,     0,    32,    33,     0,    34,    35,    36,    37,
      38,    39,    40,    41,     0,     0,     0,     0,     0,    42,
       0,    44,     0,     0,     0,     0,     0,   124,   125,     0,
     126,   127,   128,     0,    45,   129,   130,   131,   132,     0,
       0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
       0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
       0,    54,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
       0,    20,   322,    22,     0,     0,     0,    23,    24,    25,
       0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
       0,    31,     0,    32,    33,     0,    34,   323,    36,    37,
     324,    39,    40,    41,     0,     0,     0,     0,     0,    42,
       0,    44,     0,     0,     0,     0,     0,   124,   125,     0,
     126,   127,   128,     0,    45,   129,   130,   131,   132,     0,
       0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
       0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
       0,    54,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
       0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
       0,    31,     0,    32,    33,     0,    34,    35,    36,    37,
      38,    39,    40,    41,     0,     0,     0,     0,     0,    42,
       0,    44,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
       0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
       0,    54,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
       0,    20,   322,    22,     0,     0,     0,    23,    24,    25,
       0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
       0,    31,     0,    32,    33,     0,    34,   323,    36,    37,
     324,    39,    40,    41,     0,     0,     0,     0,     0,    42,
       0,    44,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
       0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
       0,    54,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
       0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
       0,    31,     0,    32,    33,     0,    34,    35,    36,    37,
      38,    39,    40,    41,     0,     0,     0,     0,     0,    42,
       0,    44,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
       0,   390,     0,    50,    51,    52,     0,    53,     0,     0,
       0,    54
};

static const short int yycheck[] =
{
      15,    16,    17,    18,    19,   336,   384,   235,    23,    24,
      25,    26,    27,    28,    29,    30,    31,   106,    43,    34,
     208,   240,    37,   294,   295,    40,    41,    42,   216,    44,
      45,    46,    47,    48,   436,    43,   290,    52,    53,    54,
     332,   333,   515,   626,   423,   542,    96,   339,    63,    64,
     342,   343,     8,     9,   633,   398,   635,   636,     8,    15,
      16,    13,   463,    76,   405,    15,    97,   288,    15,     8,
      19,     8,     9,    59,   500,    14,    15,   498,    15,    16,
     286,   642,   642,    83,    14,    18,     0,   234,   235,    75,
     237,    47,   298,   405,    94,    95,    71,   325,    98,    99,
     100,    34,   498,    83,    96,    43,    98,   690,     0,   124,
     125,   126,   127,   128,    94,   667,     4,   132,   697,   134,
     135,   136,     8,   138,    80,    11,    12,   142,   143,   144,
     145,   146,   147,    72,     4,   150,   151,   152,   153,   154,
     542,    15,    72,    80,   175,     8,   619,   401,   402,   101,
     102,   107,   358,   714,   714,   111,   499,   113,   559,   108,
     110,   117,   109,   101,   747,   103,    58,   546,    60,   155,
     107,   109,   110,   514,   111,   396,   113,   729,   325,   605,
     117,   119,   603,   604,     8,   373,    15,    11,    12,   336,
      82,    15,   504,   381,    86,    87,    88,    15,   213,    47,
      92,    93,   514,    15,    96,   352,   353,   603,   604,   540,
     588,    47,   262,    61,    38,    13,   112,   113,   114,   438,
     717,   115,   801,   802,   803,   108,   699,   700,   243,   115,
     245,   246,   247,   248,   249,   109,   110,   194,   253,    63,
      13,    70,    66,   445,   244,   444,   261,   539,   107,   187,
     207,    83,   523,   545,   254,   270,   271,   272,   107,   274,
     275,   276,    52,   288,    96,   280,    98,   267,   554,   269,
     556,    61,   258,   749,   750,    83,   464,   465,   278,   481,
     288,   480,    42,   283,     0,   300,    21,   179,    96,   436,
      98,   109,   110,   117,   251,    18,   234,   235,    35,   237,
     238,    99,   240,   101,   102,    40,    31,    42,   397,   537,
       8,   458,    18,    73,    74,   717,   463,    15,   794,     8,
       9,    56,    35,   338,    97,    98,    99,    16,   101,   102,
      47,   227,   228,   229,   230,   231,   232,   233,    76,   107,
      78,   356,    58,   111,   236,   113,   242,   239,   363,   349,
     288,   289,   606,   105,   106,   293,   294,   295,   701,   374,
     252,   361,    15,    16,   256,   257,    47,   259,    84,   369,
     262,   396,    42,    83,    90,    29,   268,    47,    88,   365,
     366,    18,    36,   113,    94,   385,    61,   325,   396,   404,
     537,    47,    46,   540,    35,   542,    93,    94,   336,    35,
      54,    13,   745,    35,    58,   420,   113,   422,   751,   301,
     753,    35,   559,    83,    84,    35,    86,    87,    88,    15,
      16,    91,    92,    93,    94,   514,    47,   442,   517,    24,
     445,   685,   447,   687,    73,    74,   451,    84,   453,    86,
      87,    88,    70,   443,    91,    92,    93,    94,   352,   353,
     450,    78,   467,     8,   170,   171,    11,    12,   396,    89,
      15,   804,   805,   806,    83,    43,   481,   405,   483,   469,
     485,   529,   530,   531,   474,   490,   491,   448,   449,   479,
      50,   496,    35,    95,    96,    97,    98,    99,   384,   101,
     102,    35,   507,    61,   432,   510,   511,   716,    84,   215,
     438,   107,    41,   518,    90,   111,   506,   113,    94,   107,
      60,    89,    47,   111,   702,   113,   102,   706,   707,    97,
     667,   536,   112,   538,   102,   111,   615,   423,    86,    87,
      88,   117,   118,    91,    92,    93,    94,   429,   603,   604,
       8,   257,   557,    11,    12,   114,     3,   439,     5,     6,
       7,    94,   448,   449,    31,    32,    33,   572,    35,   115,
      19,     8,     9,   115,   115,   503,   115,   505,    15,    16,
     717,    28,    22,   573,   115,   115,   514,   515,   593,   115,
     107,   581,   729,   115,    35,   523,    61,   587,   526,   527,
      88,    38,    39,    91,    92,    93,    94,   175,    61,   537,
      61,   493,   540,     8,     9,    35,   192,    35,    35,   698,
      15,    16,   627,    35,   192,   630,    63,    64,    13,    66,
      67,    35,   208,    31,    32,    33,    79,    35,   643,   107,
     216,    35,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   641,    18,   352,   353,    81,   108,    29,   234,   235,
     546,   666,   368,   108,    36,   115,   234,   235,   554,   237,
     556,    29,   114,   555,    46,    47,   112,   682,    36,   108,
     117,   114,    54,   114,   689,   613,    58,   263,    46,    47,
      18,   619,    35,    35,    80,   623,    54,   273,   626,    35,
      58,   108,   588,    35,    83,   281,   282,   107,    35,   285,
      95,    96,    97,    98,    99,   421,   101,   102,   115,    35,
     288,   289,    83,    83,   108,   653,   107,   303,    83,   734,
     735,   736,    91,    92,    93,    94,   664,    47,    94,   667,
       8,    13,    35,    11,    12,    35,   452,    15,    35,   325,
      35,   108,    83,    35,    35,    17,    35,   325,    35,   458,
     108,   115,   690,   108,   463,    35,   771,   466,   336,    35,
      38,   699,   700,   108,   112,   114,    80,   192,   354,   355,
      58,   709,     8,     9,   352,   353,   187,   594,   716,    15,
      16,   672,    72,   721,   652,    63,   421,   373,    66,    83,
      84,   729,    86,    87,    88,   381,   505,    91,    92,    93,
      94,   331,   388,   454,   390,   588,   515,    90,   729,   747,
     244,    47,   767,    95,    96,    97,    98,    99,   396,   101,
     102,   759,   116,   790,   256,   262,   718,   405,    20,   767,
     714,   109,   110,    87,    88,   601,   759,    91,    92,    93,
      94,   427,    76,   292,    80,   780,   613,    83,   520,    20,
     559,   514,    -1,   625,   289,    -1,    -1,    -1,   436,   437,
      96,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   107,    -1,    -1,    -1,   111,    -1,   113,   464,   465,
     458,   117,    -1,    -1,    -1,   463,    -1,    -1,   466,    -1,
      -1,    83,    84,    -1,    86,    87,    88,    -1,    -1,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,   494,   495,
     619,    -1,    83,    84,    -1,    86,    87,    88,    -1,   497,
      91,    92,    93,    94,   116,    -1,    -1,   505,    -1,    -1,
      83,    84,    -1,    86,    87,    88,   514,   515,    91,    92,
      93,    94,    -1,    -1,    -1,   116,    -1,    -1,   526,    -1,
      -1,   537,    -1,    -1,    13,    -1,    -1,    -1,    -1,   537,
      -1,    -1,   540,   116,   542,    -1,    -1,    83,    84,    -1,
      86,    87,    88,    -1,   560,    91,    92,    93,    94,    -1,
      -1,   559,    -1,    42,    -1,    -1,    -1,    -1,    47,    -1,
     699,   700,    -1,    83,    84,    -1,    86,    87,    88,    -1,
     116,    91,    92,    93,    94,   591,   592,    -1,   594,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   601,    83,    84,    -1,    86,    87,    88,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   619,   101,   102,    -1,    -1,    -1,   625,   626,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   667,
     676,    -1,   678,    -1,   680,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   690,    -1,    -1,    -1,   702,    -1,    -1,    -1,
      -1,   699,   700,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   717,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   729,     1,    -1,     3,    -1,     5,     6,     7,     8,
       9,    10,    11,    12,    -1,    14,    15,    16,    -1,   747,
      -1,    -1,    -1,    -1,    23,    -1,    25,    26,    27,    28,
      29,    30,    -1,    -1,    -1,    -1,    -1,    36,    37,    38,
      39,    -1,    -1,    -1,    43,    44,    45,    46,    -1,    48,
      49,    -1,    51,    -1,    53,    54,    55,   793,    57,    58,
      59,    60,    -1,    62,    63,    64,    65,    66,    67,    68,
      69,    -1,    71,    -1,    -1,    -1,    75,    76,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,   107,    -1,
     109,   110,   111,    -1,   113,    -1,    -1,     3,   117,     5,
       6,     7,     8,     9,    10,    11,    12,    -1,    14,    15,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,
      26,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
      -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,
      -1,    57,    -1,    59,    60,    -1,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    71,    -1,    -1,    -1,    75,
      76,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,    -1,
      -1,   117,     8,     9,    10,    11,    12,    -1,    14,    15,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,
      26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
      -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,
      -1,    57,    -1,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    83,    84,    -1,
      86,    87,    88,    -1,    90,    91,    92,    93,    94,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,    -1,
      -1,   117,     8,     9,    10,    11,    12,    -1,    14,    15,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,
      26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
      -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,
      -1,    57,    -1,    59,    60,    -1,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    83,    84,    -1,
      86,    87,    88,    -1,    90,    91,    92,    93,    94,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,    -1,
     116,   117,     8,     9,    10,    11,    12,    -1,    14,    15,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,
      26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
      -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,
      -1,    57,    -1,    59,    60,    -1,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    83,    84,    -1,
      86,    87,    88,    -1,    90,    91,    92,    93,    94,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,    -1,
      -1,   117,     8,     9,    10,    11,    12,    -1,    14,    15,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,
      26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
      -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,
      -1,    57,    -1,    59,    60,    -1,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    83,    84,    -1,
      86,    87,    88,    -1,    90,    91,    92,    93,    94,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,    -1,
      -1,   117,     8,     9,    10,    11,    12,    -1,    14,    15,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,
      26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
      -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,
      -1,    57,    -1,    59,    60,    -1,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,    -1,
      -1,   117,     8,     9,    10,    11,    12,    -1,    14,    15,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,
      26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
      -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,
      -1,    57,    -1,    59,    60,    -1,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,    -1,
      -1,   117,     8,     9,    10,    11,    12,    -1,    14,    15,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,
      26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
      -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,
      -1,    57,    -1,    59,    60,    -1,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,    -1,
      -1,   117
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short int yystos[] =
{
       0,     1,     3,     5,     6,     7,     8,     9,    10,    11,
      12,    14,    15,    16,    23,    25,    26,    27,    28,    30,
      37,    38,    39,    43,    44,    45,    48,    49,    51,    53,
      55,    57,    59,    60,    62,    63,    64,    65,    66,    67,
      68,    69,    75,    76,    77,    90,   100,   103,   104,   107,
     109,   110,   111,   113,   117,   119,   120,   121,   122,   125,
     126,   128,   157,   158,   159,   173,   196,   197,   198,   199,
     200,   201,   202,   204,   212,   218,   219,   105,   106,   123,
     124,   166,   203,   203,   203,   189,   203,   203,   203,   203,
     203,   164,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   203,   203,   110,   196,   197,   223,   224,   225,
     203,   203,   203,   203,   203,   125,   152,   203,   203,   203,
       0,    71,   120,   121,    83,    84,    86,    87,    88,    91,
      92,    93,    94,   125,   146,   147,   148,    13,    95,    96,
      97,    98,    99,   101,   102,   149,   150,   151,   203,   203,
      29,    36,    46,    54,    58,   140,   121,   218,     4,     4,
     123,   125,   152,   195,   126,   174,   202,   125,   191,   192,
     125,   125,   193,   194,   138,   196,   174,   125,   125,   126,
     152,   152,   193,   138,   152,   152,   152,   135,   136,   197,
     126,   129,   130,   196,   197,   221,   115,    83,    96,    98,
     237,   238,   107,   111,   113,   226,   197,   197,   126,   153,
     128,   128,   128,    47,   108,   126,   126,   197,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   203,   203,   107,   107,   203,   203,   203,   203,
     203,   121,    52,    61,    18,    35,    21,    40,    42,    56,
     175,   176,    47,    61,    31,   190,    18,    47,   202,    47,
     202,    35,    18,   113,   138,   139,   140,    61,    47,    61,
      35,    35,    35,   113,    35,    35,    24,   155,    70,   136,
      47,    42,    47,    70,   129,   115,   175,    83,   220,    78,
     239,   240,   241,   227,   229,   231,   236,   237,   175,   153,
      89,   203,   154,   202,   153,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   127,   128,   128,   128,   128,   128,
     128,   128,    38,    63,    66,   126,   160,   162,   196,   197,
     200,   160,   125,   145,   163,   196,   197,   200,    16,   141,
     142,   197,   125,   141,   128,   168,   170,   171,   172,   203,
     195,   203,   203,   203,   203,   203,    50,   180,   181,   175,
     125,   203,   152,    35,   191,   125,   202,   121,   125,    61,
     203,   193,   194,   126,    35,   152,   125,   152,   203,   203,
     203,   126,   203,   203,   203,    41,   156,   152,   203,   126,
     107,   126,   131,   152,   126,   180,   222,   197,   223,     9,
      16,   196,   197,   262,    60,   202,   244,   245,   246,    47,
     240,   109,   197,   233,   234,   233,   233,   180,   203,   125,
     112,   126,   114,    95,   160,    19,   161,   115,   161,    47,
     139,   139,   115,   145,   203,   139,   107,    22,   144,    47,
     139,   139,    95,    31,    32,    33,    35,   167,    18,    34,
      61,    83,    88,    94,   152,    38,    63,    66,   178,   179,
     196,   198,   200,   178,   126,   126,   203,    35,   180,    61,
     152,   203,   121,   121,    61,   202,   152,   153,   203,    31,
      32,    33,    35,   165,    35,    35,   153,   169,   170,   152,
      35,    35,   126,   126,    20,   116,    35,    79,   214,   223,
     238,   244,   244,   107,   203,    47,    80,   117,   196,   197,
     224,   225,   247,   248,   249,   253,   254,   261,   262,    35,
      18,   108,    81,   235,   112,   114,    73,    74,   205,   206,
     207,   209,   203,   154,   203,   127,   108,   126,   108,   125,
     197,   143,   163,   196,   141,   125,   203,   152,   166,   164,
     203,   168,   168,   152,   203,   202,   203,   167,   177,   178,
     115,   177,   153,   153,     9,    16,    38,    39,    63,    64,
      66,    67,   117,   182,   183,   184,   196,   198,   203,   152,
     152,   114,   152,   166,   164,   203,   203,   114,    18,   203,
     203,    20,   116,   137,   116,   126,   126,   133,   203,   196,
     215,   216,   213,   219,   241,   238,   239,    35,    35,   109,
     110,   197,   242,   243,   247,   197,   198,   247,   250,   251,
     252,   254,   152,   203,   203,   203,    83,   237,   248,   250,
     237,   203,   245,   228,   233,   230,   232,    14,    72,   196,
     197,   208,   197,    35,   205,   205,   205,   203,   160,   203,
     139,   145,   108,   115,   143,   139,   127,    35,   172,   125,
     172,   203,   177,   126,   203,   152,    83,   107,   152,    35,
     152,   169,   126,   126,   203,   133,   116,   132,   116,   134,
     115,   215,    35,   213,   213,   239,   244,   108,   242,   237,
      83,   250,   197,   107,   111,   113,   196,   261,   197,   252,
     203,   203,   107,   236,   236,   236,    83,    83,   152,    42,
      73,   207,   209,   210,   211,   203,   144,   197,    47,    16,
      35,   203,    38,    63,    66,   109,   110,   162,   185,   186,
     187,   196,   197,   200,    35,    35,   137,   108,   126,   126,
     126,   203,   244,    35,   244,   203,   252,    83,   255,   257,
     259,   236,   250,   250,   238,   153,   208,   208,    35,   197,
     217,    35,   210,   141,   143,   125,   197,   115,    19,   108,
     185,    17,   188,   203,   203,   203,    35,    35,   238,   252,
     244,   263,   263,   263,   238,   238,   108,    35,    35,   217,
     187,   197,   108,   203,   235,   108,   112,   114,   188,   126,
     263,   256,   258,   260,   236,   236,   236,   238,   238,   238
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

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
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
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
    while (0)
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
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Type, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

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
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
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
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

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
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
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
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

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


  yyvsp[0] = yylval;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
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

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

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

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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

    { yyoutput = (yyvsp[-1].t); YYACCEPT; }
    break;

  case 3:

    { yyoutput = PA_parseError; YYABORT; }
    break;

  case 4:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 5:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fSynTopLevelProductionTemplates,
					   (yyvsp[-1].t)),(yyvsp[0].t)); }
    break;

  case 6:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 7:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fFunctor,newCTerm(PA_fDollar,(yyvsp[-2].t)),
					   (yyvsp[-1].t),(yyvsp[-2].t)),(yyvsp[0].t)); }
    break;

  case 8:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 9:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fDeclare,(yyvsp[-3].t),newCTerm(PA_fSkip,(yyvsp[-1].t)),
					   (yyvsp[-4].t)),(yyvsp[0].t)); }
    break;

  case 10:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fDeclare,(yyvsp[-3].t),(yyvsp[-1].t),(yyvsp[-4].t)),(yyvsp[0].t)); }
    break;

  case 11:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fDeclare,(yyvsp[-2].t),
					   newCTerm(PA_fSkip,(yyvsp[-1].t)),(yyvsp[-3].t)),(yyvsp[0].t)); }
    break;

  case 12:

    { (yyval.t) = AtomNil; }
    break;

  case 13:

    { (yyval.t) = newCTerm(PA_dirSwitch,(yyvsp[0].t)); }
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

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 19:

    { if (!strcmp(xytext,"gump"))
		      xy_gumpSyntax = 1;
		    if (!strcmp(xytext,"allowdeprecated"))
		      xy_allowDeprecated = 1;
		    (yyval.t) = newCTerm(PA_on,OZ_atom(xytext),pos());
		  }
    break;

  case 20:

    { if (!strcmp(xytext,"gump"))
		      xy_gumpSyntax = 0;
		    if (!strcmp(xytext,"allowdeprecated"))
		      xy_allowDeprecated = 0;
		    (yyval.t) = newCTerm(PA_off,OZ_atom(xytext),pos());
		  }
    break;

  case 21:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 22:

    { (yyval.t) = newCTerm(PA_fAnd,(yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 23:

    { (yyval.t) = newCTerm(PA_fEq,(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 24:

    { if (oz_isSRecord((yyvsp[-3].t)) && 
                        oz_eq(OZ_label((yyvsp[-3].t)), PA_fOpApply) && 
                        oz_eq(OZ_getArg((yyvsp[-3].t),0), AtomDot)) {
                       (yyval.t) = newCTerm(PA_fDotAssign,(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t));
			}
                    else
                       (yyval.t) = newCTerm(PA_fColonEquals,(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 25:

    { (yyval.t) = newCTerm(PA_fAssign,(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 26:

    { (yyval.t) = newCTerm(PA_fOrElse,(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 27:

    { (yyval.t) = newCTerm(PA_fAndThen,(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 28:

    { (yyval.t) = newCTerm(PA_fOpApply,(yyvsp[-2].t),
				  oz_mklistUnwrap((yyvsp[-3].t),(yyvsp[0].t)),(yyvsp[-1].t)); }
    break;

  case 29:

    { (yyval.t) = newCTerm(PA_fFdCompare,(yyvsp[-2].t),(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 30:

    { (yyval.t) = newCTerm(PA_fFdIn,(yyvsp[-2].t),(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 31:

    { (yyval.t) = makeCons((yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 32:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 33:

    { (yyval.t) = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,(yyvsp[-1].t)),
				  oz_consUnwrap((yyvsp[-3].t),(yyvsp[0].t))); }
    break;

  case 34:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 35:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 36:

    { (yyval.t) = newCTerm(PA_fOpApply,(yyvsp[-2].t),
				  oz_mklistUnwrap((yyvsp[-3].t),(yyvsp[0].t)),(yyvsp[-1].t)); }
    break;

  case 37:

    { (yyval.t) = newCTerm(PA_fOpApply,(yyvsp[-2].t),
				  oz_mklistUnwrap((yyvsp[-3].t),(yyvsp[0].t)),(yyvsp[-1].t)); }
    break;

  case 38:

    { (yyval.t) = newCTerm(PA_fOpApply,(yyvsp[-2].t),
				  oz_mklistUnwrap((yyvsp[-3].t),(yyvsp[0].t)),(yyvsp[-1].t)); }
    break;

  case 39:

    { (yyval.t) = newCTerm(PA_fObjApply,(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 40:

    { (yyval.t) = newCTerm(PA_fOpApply,AtomTilde,
				  oz_mklistUnwrap((yyvsp[0].t)),(yyvsp[-1].t)); }
    break;

  case 41:

    { (yyval.t) = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklistUnwrap((yyvsp[-3].t),(yyvsp[0].t)),(yyvsp[-1].t)); }
    break;

  case 42:

    { (yyval.t) = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklistUnwrap((yyvsp[-1].t),makeInt(xytext,pos())),pos()); }
    break;

  case 43:

    { (yyval.t) = newCTerm(PA_fOpApply,AtomHat,
				  oz_mklistUnwrap((yyvsp[-3].t),(yyvsp[0].t)),(yyvsp[-1].t)); }
    break;

  case 44:

    { (yyval.t) = newCTerm(PA_fAt,(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 45:

    { (yyval.t) = newCTerm(PA_fOpApply,AtomDExcl,
				  oz_mklistUnwrap((yyvsp[0].t)),(yyvsp[-1].t)); }
    break;

  case 46:

    { (yyval.t) = newCTerm(PA_parens,(yyvsp[-1].t)); }
    break;

  case 47:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 48:

    { (yyval.t) = (yyvsp[0].t); }
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

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 56:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 57:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 58:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 59:

    { (yyval.t) = newCTerm(PA_fRecord,newCTerm(PA_fAtom,AtomCons,
						     makeLongPos((yyvsp[-4].t),(yyvsp[0].t))),
				  oz_mklistUnwrap((yyvsp[-3].t),(yyvsp[-2].t))); }
    break;

  case 60:

    { (yyval.t) = newCTerm(PA_fApply,(yyvsp[-3].t),(yyvsp[-2].t),makeLongPos((yyvsp[-4].t),(yyvsp[0].t))); }
    break;

  case 61:

    { (yyval.t) = newCTerm(PA_fProc,(yyvsp[-5].t),(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[-7].t),makeLongPos((yyvsp[-8].t),(yyvsp[0].t))); }
    break;

  case 62:

    { (yyval.t) = newCTerm(PA_fFun,(yyvsp[-5].t),(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[-7].t),makeLongPos((yyvsp[-8].t),(yyvsp[0].t))); }
    break;

  case 63:

    { (yyval.t) = newCTerm(PA_fFunctor,(yyvsp[-3].t),(yyvsp[-2].t),makeLongPos((yyvsp[-4].t),(yyvsp[0].t))); }
    break;

  case 64:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 65:

    { (yyval.t) = newCTerm(PA_fLocal,(yyvsp[-3].t),(yyvsp[-1].t),(yyvsp[-4].t)); }
    break;

  case 66:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 67:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 68:

    { (yyval.t) = newCTerm(PA_fLock,(yyvsp[-2].t),makeLongPos((yyvsp[-3].t),(yyvsp[0].t))); }
    break;

  case 69:

    { (yyval.t) = newCTerm(PA_fLockThen,(yyvsp[-4].t),(yyvsp[-2].t),makeLongPos((yyvsp[-5].t),(yyvsp[0].t))); }
    break;

  case 70:

    { (yyval.t) = newCTerm(PA_fThread,(yyvsp[-2].t),makeLongPos((yyvsp[-3].t),(yyvsp[0].t))); }
    break;

  case 71:

    { (yyval.t) = newCTerm(PA_fTry,(yyvsp[-4].t),(yyvsp[-3].t),(yyvsp[-2].t),makeLongPos((yyvsp[-5].t),(yyvsp[0].t))); }
    break;

  case 72:

    { (yyval.t) = newCTerm(PA_fRaise,(yyvsp[-2].t),makeLongPos((yyvsp[-3].t),(yyvsp[0].t))); }
    break;

  case 73:

    { (yyval.t) = newCTerm(PA_fSkip,pos()); }
    break;

  case 74:

    { (yyval.t) = newCTerm(PA_fFail,pos()); }
    break;

  case 75:

    { (yyval.t) = newCTerm(PA_fNot,(yyvsp[-2].t),makeLongPos((yyvsp[-3].t),(yyvsp[0].t))); }
    break;

  case 76:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 77:

    { (yyval.t) = newCTerm(PA_fOr,(yyvsp[-2].t),makeLongPos((yyvsp[-3].t),(yyvsp[0].t))); }
    break;

  case 78:

    { (yyval.t) = newCTerm(PA_fDis,(yyvsp[-2].t),makeLongPos((yyvsp[-3].t),(yyvsp[0].t))); }
    break;

  case 79:

    { (yyval.t) = newCTerm(PA_fChoice,(yyvsp[-2].t),makeLongPos((yyvsp[-3].t),(yyvsp[0].t))); }
    break;

  case 80:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 81:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 82:

    { (yyval.t) = newCTerm(PA_fMacro,(yyvsp[-2].t),makeLongPos((yyvsp[-3].t),(yyvsp[0].t))); }
    break;

  case 83:

    { (yyval.t) = newCTerm(PA_fLoop,
				  newCTerm(PA_fAnd,(yyvsp[-4].t),(yyvsp[-2].t)),
				  makeLongPos((yyvsp[-5].t),(yyvsp[0].t))); }
    break;

  case 84:

    { (yyval.t) = newCTerm(PA_fFOR,(yyvsp[-4].t),(yyvsp[-2].t),makeLongPos((yyvsp[-5].t),(yyvsp[0].t))); }
    break;

  case 85:

    { (yyval.t) = AtomNil; }
    break;

  case 86:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 87:

    { (yyval.t) = newCTerm(oz_atom("forFlag"),(yyvsp[0].t)); }
    break;

  case 88:

    { (yyval.t) = newCTerm(oz_atom("forFeature"),(yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 89:

    { (yyval.t) = newCTerm(oz_atom("forPattern"),(yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 90:

    { (yyval.t) = newCTerm(oz_atom("forFrom"),(yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 91:

    { (yyval.t) = newCTerm(oz_atom("forGeneratorList"),(yyvsp[0].t)); }
    break;

  case 92:

    { (yyval.t) = newCTerm(oz_atom("forGeneratorInt"),(yyvsp[-3].t),(yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 93:

    { (yyval.t) = newCTerm(oz_atom("forGeneratorC"),(yyvsp[-2].t),oz_headUnwrap((yyvsp[0].t)),
                                                              oz_tailUnwrap((yyvsp[0].t))); }
    break;

  case 94:

    { (yyval.t) = newCTerm(oz_atom("forGeneratorC"),(yyvsp[-3].t),oz_headUnwrap((yyvsp[-1].t)),
                                                              oz_tailUnwrap((yyvsp[-1].t))); }
    break;

  case 95:

    { (yyval.t) = NameUnit; }
    break;

  case 96:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 97:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 98:

    { (yyval.t) = NameUnit; }
    break;

  case 99:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 100:

    {
		    (yyval.t) = newCTerm(PA_fAnd,(yyvsp[-1].t),(yyvsp[0].t));
		  }
    break;

  case 102:

    {
		    (yyval.t) = newCTerm(PA_fMacro,
				  oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					  unwrap((yyvsp[-7].t)),
					  newCTerm(PA_fAtom,oz_atom("from"),NameUnit),
					  unwrap((yyvsp[-4].t)),
					  newCTerm(PA_fAtom,oz_atom("to"),NameUnit),
					  unwrap((yyvsp[-2].t)),
					  newCTerm(PA_fAtom,oz_atom("by"),NameUnit),
					  (unwrap((yyvsp[-1].t)) == 0)?makeInt("1",NameUnit):unwrap((yyvsp[-1].t)),
					  0),
				  makeLongPos(OZ_subtree((yyvsp[-7].t),makeTaggedSmallInt(2)),(yyvsp[0].t)));
		  }
    break;

  case 103:

    {
		    /* <<for X 'in' L>>
		       <<for X = E1 'then' E2>> */
		    if ((yyvsp[-1].t) == 0) {
		      (yyval.t) = newCTerm(PA_fMacro,
				    oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					    unwrap((yyvsp[-5].t)),
					    newCTerm(PA_fAtom,oz_atom("in"),NameUnit),
					    unwrap((yyvsp[-2].t)),
					    0),
				    makeLongPos(OZ_subtree((yyvsp[-5].t),makeTaggedSmallInt(2)),(yyvsp[0].t)));
		    } else {
		      (yyval.t) = newCTerm(PA_fMacro,
				    oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					    newCTerm(PA_fEq,(yyvsp[-5].t),(yyvsp[-2].t),NameUnit),
					    newCTerm(PA_fAtom,oz_atom("next"),NameUnit),
					    unwrap((yyvsp[-1].t)),
					    0),
				    makeLongPos(OZ_subtree((yyvsp[-5].t),makeTaggedSmallInt(2)),(yyvsp[0].t)));
		    }
		  }
    break;

  case 104:

    { (yyval.t) = 0; }
    break;

  case 105:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 106:

    { (yyval.t) = AtomNil; }
    break;

  case 107:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 108:

    { (yyval.t) = AtomNil; }
    break;

  case 109:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 110:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fRequire,(yyvsp[-1].t),(yyvsp[-2].t)),(yyvsp[0].t)); }
    break;

  case 111:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fPrepare,(yyvsp[-3].t),(yyvsp[-1].t),(yyvsp[-4].t)),(yyvsp[0].t)); }
    break;

  case 112:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fPrepare,(yyvsp[-1].t),
					   newCTerm(PA_fSkip,(yyvsp[-2].t)),(yyvsp[-2].t)),(yyvsp[0].t)); }
    break;

  case 113:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fImport,(yyvsp[-1].t),(yyvsp[-2].t)),(yyvsp[0].t)); }
    break;

  case 114:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fExport,(yyvsp[-1].t),(yyvsp[-2].t)),(yyvsp[0].t)); }
    break;

  case 115:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fDefine,(yyvsp[-3].t),(yyvsp[-1].t),(yyvsp[-4].t)),(yyvsp[0].t)); }
    break;

  case 116:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fDefine,(yyvsp[-1].t),
					   newCTerm(PA_fSkip,(yyvsp[-2].t)),(yyvsp[-2].t)),(yyvsp[0].t)); }
    break;

  case 117:

    { (yyval.t) = AtomNil; }
    break;

  case 118:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fImportItem,(yyvsp[-2].t),AtomNil,(yyvsp[-1].t)),(yyvsp[0].t)); }
    break;

  case 119:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fImportItem,(yyvsp[-5].t),(yyvsp[-3].t),(yyvsp[-1].t)),(yyvsp[0].t)); }
    break;

  case 120:

    { (yyval.t) = newCTerm(PA_fVar,OZ_atom(xytext),(yyvsp[0].t)); }
    break;

  case 121:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 122:

    { (yyval.t) = oz_mklistUnwrap(oz_pair2Unwrap((yyvsp[0].t),(yyvsp[-2].t))); }
    break;

  case 123:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 124:

    { (yyval.t) = oz_consUnwrap(oz_pair2Unwrap((yyvsp[-1].t),(yyvsp[-3].t)),(yyvsp[0].t)); }
    break;

  case 125:

    { (yyval.t) = PA_fNoImportAt; }
    break;

  case 126:

    { (yyval.t) = newCTerm(PA_fImportAt,(yyvsp[0].t)); }
    break;

  case 127:

    { (yyval.t) = AtomNil; }
    break;

  case 128:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fExportItem,(yyvsp[-1].t)),(yyvsp[0].t)); }
    break;

  case 129:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fExportItem,
					   newCTerm(PA_fColon,(yyvsp[-3].t),(yyvsp[-1].t))),(yyvsp[0].t)); }
    break;

  case 130:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 131:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 132:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 133:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 134:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 135:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 136:

    { (yyval.t) = newCTerm(PA_fLocal,(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 137:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 138:

    { (yyval.t) = AtomNil; }
    break;

  case 139:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 140:

    { (yyval.t) = newCTerm(PA_fAtom,AtomNil,(yyvsp[0].t)); }
    break;

  case 141:

    { (yyval.t) = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomCons,(yyvsp[-2].t)),
				  oz_mklistUnwrap((yyvsp[-1].t),(yyvsp[0].t))); }
    break;

  case 142:

    { (yyval.t) = PA_fNoCatch; }
    break;

  case 143:

    { (yyval.t) = newCTerm(PA_fCatch,(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 144:

    { (yyval.t) = PA_fNoFinally; }
    break;

  case 145:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 146:

    {
		    (yyval.t) = newCTerm(OZ_isTrue((yyvsp[-2].t))? PA_fOpenRecord : PA_fRecord,
				  newCTerm(PA_fAtom,(yyvsp[-6].t),makeLongPos((yyvsp[-5].t),(yyvsp[0].t))),(yyvsp[-3].t));
		  }
    break;

  case 147:

    {
		    (yyval.t) = newCTerm(OZ_isTrue((yyvsp[-2].t))? PA_fOpenRecord : PA_fRecord,
				  makeVar((yyvsp[-6].t),makeLongPos((yyvsp[-5].t),(yyvsp[0].t))),(yyvsp[-3].t));
		  }
    break;

  case 148:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 149:

    { (yyval.t) = NameUnit; }
    break;

  case 150:

    { (yyval.t) = NameTrue; }
    break;

  case 151:

    { (yyval.t) = NameFalse; }
    break;

  case 152:

    { (yyval.t) = OZ_atom(xytext); }
    break;

  case 153:

    { (yyval.t) = AtomNil; }
    break;

  case 154:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 155:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fColon,(yyvsp[-3].t),(yyvsp[-1].t)),(yyvsp[0].t)); }
    break;

  case 156:

    { (yyval.t) = NameFalse; }
    break;

  case 157:

    { (yyval.t) = NameTrue; }
    break;

  case 158:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 159:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 160:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 161:

    { (yyval.t) = newCTerm(PA_fAtom,NameUnit,pos()); }
    break;

  case 162:

    { (yyval.t) = newCTerm(PA_fAtom,NameTrue,pos()); }
    break;

  case 163:

    { (yyval.t) = newCTerm(PA_fAtom,NameFalse,pos()); }
    break;

  case 164:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 165:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 166:

    { (yyval.t) = newCTerm(PA_fBoolCase,(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[-1].t),makeLongPos((yyvsp[-5].t),(yyvsp[0].t))); }
    break;

  case 167:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 168:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 169:

    { (yyval.t) = (yyvsp[-1].t); }
    break;

  case 170:

    { (yyval.t) = newCTerm(PA_fNoElse,pos()); }
    break;

  case 171:

    { checkDeprecation((yyvsp[-3].t));
		    (yyval.t) = newCTerm(PA_fBoolCase,(yyvsp[-5].t),(yyvsp[-2].t),(yyvsp[-1].t),makeLongPos((yyvsp[-6].t),(yyvsp[0].t)));
		  }
    break;

  case 172:

    { (yyval.t) = newCTerm(PA_fCase,(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[-1].t),makeLongPos((yyvsp[-5].t),(yyvsp[0].t))); }
    break;

  case 173:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 174:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 175:

    { (yyval.t) = (yyvsp[-1].t); }
    break;

  case 176:

    { (yyval.t) = newCTerm(PA_fNoElse,pos()); }
    break;

  case 177:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 178:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 179:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 180:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 181:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 182:

    { (yyval.t) = newCTerm(PA_fCaseClause,(yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 183:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 184:

    { (yyval.t) = newCTerm(PA_fSideCondition,(yyvsp[-3].t),
				  newCTerm(PA_fSkip,(yyvsp[-1].t)),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 185:

    { (yyval.t) = newCTerm(PA_fSideCondition,(yyvsp[-5].t),(yyvsp[-2].t),(yyvsp[0].t),(yyvsp[-3].t)); }
    break;

  case 186:

    { (yyval.t) = newCTerm(PA_fEq,(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 187:

    { (yyval.t) = makeCons((yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 188:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 189:

    { (yyval.t) = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,(yyvsp[-1].t)),
				  oz_consUnwrap((yyvsp[-3].t),(yyvsp[0].t))); }
    break;

  case 190:

    { (yyval.t) = newCTerm(PA_fClass,(yyvsp[-4].t),(yyvsp[-3].t),(yyvsp[-2].t),makeLongPos((yyvsp[-5].t),(yyvsp[0].t))); }
    break;

  case 191:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 192:

    { (yyval.t) = newCTerm(PA_fDollar,(yyvsp[0].t)); }
    break;

  case 193:

    { (yyval.t) = AtomNil; }
    break;

  case 194:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 195:

    { (yyval.t) = newCTerm(PA_fFrom,oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)),(yyvsp[-2].t)); }
    break;

  case 196:

    { (yyval.t) = newCTerm(PA_fAttr,oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)),(yyvsp[-2].t)); }
    break;

  case 197:

    { (yyval.t) = newCTerm(PA_fFeat,oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)),(yyvsp[-2].t)); }
    break;

  case 198:

    { (yyval.t) = newCTerm(PA_fProp,oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)),(yyvsp[-2].t)); }
    break;

  case 199:

    { (yyval.t) = AtomNil; }
    break;

  case 200:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 201:

    { (yyval.t) = oz_pair2Unwrap((yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 202:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 203:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 204:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 205:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 206:

    { (yyval.t) = newCTerm(PA_fAtom,NameUnit,pos()); }
    break;

  case 207:

    { (yyval.t) = newCTerm(PA_fAtom,NameTrue,pos()); }
    break;

  case 208:

    { (yyval.t) = newCTerm(PA_fAtom,NameFalse,pos()); }
    break;

  case 209:

    { (yyval.t) = AtomNil; }
    break;

  case 210:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 211:

    { (yyval.t) = newCTerm(PA_fMeth,(yyvsp[-2].t),(yyvsp[-1].t),(yyvsp[-3].t)); }
    break;

  case 212:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 213:

    { (yyval.t) = newCTerm(PA_fEq,(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 214:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 215:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 216:

    { (yyval.t) = newCTerm(PA_fAtom,NameUnit,pos()); }
    break;

  case 217:

    { (yyval.t) = newCTerm(PA_fAtom,NameTrue,pos()); }
    break;

  case 218:

    { (yyval.t) = newCTerm(PA_fAtom,NameFalse,pos()); }
    break;

  case 219:

    { (yyval.t) = newCTerm(PA_fRecord,(yyvsp[-3].t),(yyvsp[-1].t)); }
    break;

  case 220:

    { (yyval.t) = newCTerm(PA_fOpenRecord,(yyvsp[-4].t),(yyvsp[-2].t)); }
    break;

  case 221:

    { (yyval.t) = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); }
    break;

  case 222:

    { (yyval.t) = makeVar(xytext); }
    break;

  case 223:

    { (yyval.t) = newCTerm(PA_fEscape,makeVar(xytext),(yyvsp[-1].t)); }
    break;

  case 224:

    { (yyval.t) = newCTerm(PA_fAtom,NameUnit,pos()); }
    break;

  case 225:

    { (yyval.t) = newCTerm(PA_fAtom,NameTrue,pos()); }
    break;

  case 226:

    { (yyval.t) = newCTerm(PA_fAtom,NameFalse,pos()); }
    break;

  case 227:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 228:

    { (yyval.t) = AtomNil; }
    break;

  case 229:

    { (yyval.t) = newCTerm(PA_fMethArg,(yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 230:

    { (yyval.t) = newCTerm(PA_fMethColonArg,(yyvsp[-3].t),(yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 231:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 232:

    { (yyval.t) = newCTerm(PA_fDollar,pos()); }
    break;

  case 233:

    { (yyval.t) = newCTerm(PA_fWildcard,pos()); }
    break;

  case 234:

    { (yyval.t) = newCTerm(PA_fDefault,(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 235:

    { (yyval.t) = PA_fNoDefault; }
    break;

  case 236:

    { (yyval.t) = newCTerm(PA_fCond,(yyvsp[-3].t),(yyvsp[-2].t),makeLongPos((yyvsp[-4].t),(yyvsp[0].t))); }
    break;

  case 237:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 238:

    { (yyval.t) = newCTerm(PA_fNoElse,pos()); }
    break;

  case 239:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 240:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 241:

    { (yyval.t) = newCTerm(PA_fClause,newCTerm(PA_fSkip,(yyvsp[-1].t)),(yyvsp[-3].t),(yyvsp[0].t)); }
    break;

  case 242:

    { (yyval.t) = newCTerm(PA_fClause,(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 243:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 244:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 245:

    { (yyval.t) = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,(yyvsp[0].t)),
				  (yyvsp[-1].t),newCTerm(PA_fNoThen,(yyvsp[0].t))); }
    break;

  case 246:

    { (yyval.t) = newCTerm(PA_fClause,(yyvsp[-3].t),(yyvsp[-1].t),newCTerm(PA_fNoThen,(yyvsp[0].t))); }
    break;

  case 247:

    { (yyval.t) = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,(yyvsp[-2].t)),(yyvsp[-3].t),(yyvsp[0].t)); }
    break;

  case 248:

    { (yyval.t) = newCTerm(PA_fClause,(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 249:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 250:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 251:

    { (yyval.t) = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); }
    break;

  case 252:

    { (yyval.t) = makeVar(xytext); }
    break;

  case 253:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 254:

    { (yyval.t) = newCTerm(PA_fEscape,(yyvsp[0].t),(yyvsp[-1].t)); }
    break;

  case 255:

    { (yyval.t) = makeString(xytext,pos()); }
    break;

  case 256:

    { (yyval.t) = makeInt(xytext,pos()); }
    break;

  case 257:

    { (yyval.t) = makeInt(xytext[0],pos()); }
    break;

  case 258:

    { (yyval.t) = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); }
    break;

  case 259:

    { (yyval.t) = pos(); }
    break;

  case 260:

    { (yyval.t) = pos(); }
    break;

  case 261:

    { OZ_Term prefix =
		      scannerPrefix? scannerPrefix: PA_zy;
		    (yyval.t) = newCTerm(PA_fScanner,(yyvsp[-5].t),(yyvsp[-4].t),(yyvsp[-3].t),(yyvsp[-2].t),prefix,
				  makeLongPos((yyvsp[-6].t),(yyvsp[0].t))); }
    break;

  case 262:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 263:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 264:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 265:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 266:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 267:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 268:

    { (yyval.t) = newCTerm(PA_fLexicalAbbreviation,(yyvsp[-3].t),(yyvsp[-1].t)); }
    break;

  case 269:

    { (yyval.t) = newCTerm(PA_fLexicalAbbreviation,(yyvsp[-3].t),(yyvsp[-1].t)); }
    break;

  case 270:

    { (yyval.t) = newCTerm(PA_fLexicalRule,(yyvsp[-2].t),(yyvsp[-1].t)); }
    break;

  case 271:

    { (yyval.t) = OZ_string(xytext); }
    break;

  case 272:

    { (yyval.t) = OZ_string(xytext); }
    break;

  case 273:

    { (yyval.t) = newCTerm(PA_fMode,(yyvsp[-2].t),(yyvsp[-1].t)); }
    break;

  case 274:

    { (yyval.t) = AtomNil; }
    break;

  case 275:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 276:

    { (yyval.t) = newCTerm(PA_fInheritedModes,(yyvsp[0].t)); }
    break;

  case 277:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 278:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 279:

    { OZ_Term expect = parserExpect? parserExpect: makeTaggedSmallInt(0);
		    (yyval.t) = newCTerm(PA_fParser,(yyvsp[-6].t),(yyvsp[-5].t),(yyvsp[-4].t),(yyvsp[-3].t),(yyvsp[-2].t),expect,
				  makeLongPos((yyvsp[-7].t),(yyvsp[0].t))); }
    break;

  case 280:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 281:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 282:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 283:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 284:

    { (yyval.t) = newCTerm(PA_fToken,AtomNil); }
    break;

  case 285:

    { (yyval.t) = newCTerm(PA_fToken,(yyvsp[0].t)); }
    break;

  case 286:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 287:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 288:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 289:

    { (yyval.t) = oz_pair2Unwrap((yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 290:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 291:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 292:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 293:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 294:

    { *prodKey[depth]++ = '='; }
    break;

  case 295:

    { (yyval.t) = newCTerm(PA_fProductionTemplate,(yyvsp[-3].t),(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[-1].t),(yyvsp[-7].t)); }
    break;

  case 296:

    { (yyval.t) = newCTerm(PA_fDollar,pos()); }
    break;

  case 297:

    { *prodKey[depth]++ = '='; }
    break;

  case 298:

    { (yyval.t) = newCTerm(PA_fProductionTemplate,(yyvsp[-3].t),(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[-1].t),(yyvsp[-7].t)); }
    break;

  case 299:

    { (yyval.t) = newCTerm(PA_fProductionTemplate,(yyvsp[-3].t),(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[-1].t),PA_none); }
    break;

  case 300:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[-1].t)); }
    break;

  case 301:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[-1].t)); }
    break;

  case 302:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 305:

    { prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg((yyvsp[-1].t),0))); }
    break;

  case 306:

    { *prodKey[depth]++ = '('; depth++; }
    break;

  case 307:

    { depth--; }
    break;

  case 308:

    { (yyval.t) = (yyvsp[-3].t); }
    break;

  case 309:

    { *prodKey[depth]++ = '['; depth++; }
    break;

  case 310:

    { depth--; }
    break;

  case 311:

    { (yyval.t) = (yyvsp[-3].t); }
    break;

  case 312:

    { *prodKey[depth]++ = '{'; depth++; }
    break;

  case 313:

    { depth--; }
    break;

  case 314:

    { (yyval.t) = (yyvsp[-3].t); }
    break;

  case 315:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 316:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 317:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 318:

    { (yyval.t) = newCTerm(PA_fWildcard,pos()); }
    break;

  case 319:

    { *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; }
    break;

  case 322:

    { *prodKey[depth]++ = xytext[0]; }
    break;

  case 323:

    { *prodKey[depth]++ = xytext[0]; }
    break;

  case 324:

    { *prodKey[depth] = '\0';
		    (yyval.t) = oz_pair2Unwrap(prodName[depth],OZ_string(prodKeyBuffer[depth]));
		    prodName[depth] = PA_none;
		    prodKey[depth] = prodKeyBuffer[depth];
		  }
    break;

  case 325:

    { (yyval.t) = AtomNil; }
    break;

  case 326:

    { (yyval.t) = (yyvsp[-1].t); }
    break;

  case 327:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 328:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 329:

    { (yyval.t) = newCTerm(PA_fSyntaxRule,(yyvsp[-2].t),AtomNil,(yyvsp[-1].t)); }
    break;

  case 330:

    { (yyval.t) = newCTerm(PA_fSyntaxRule,(yyvsp[-2].t),AtomNil,(yyvsp[-1].t)); }
    break;

  case 331:

    { (yyval.t) = newCTerm(PA_fSyntaxRule,(yyvsp[-5].t),(yyvsp[-3].t),(yyvsp[-1].t)); }
    break;

  case 332:

    { (yyval.t) = AtomNil; }
    break;

  case 333:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 334:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 335:

    { (yyval.t) = newCTerm(PA_fDollar,pos()); }
    break;

  case 336:

    { (yyval.t) = newCTerm(PA_fWildcard,pos()); }
    break;

  case 337:

    { (yyval.t) = newCTerm(PA_fSynAlternative, (yyvsp[0].t)); }
    break;

  case 338:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 339:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 340:

    { OZ_Term t = (yyvsp[0].t);
		    while (terms[depth]) {
		      t = oz_consUnwrap(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    (yyval.t) = newCTerm(PA_fSynSequence, decls[depth], t, (yyvsp[-1].t));
		    decls[depth] = AtomNil;
		  }
    break;

  case 341:

    { (yyval.t) = newCTerm(PA_fSynSequence, AtomNil, (yyvsp[0].t), (yyvsp[-1].t)); }
    break;

  case 342:

    { (yyval.t) = AtomNil; }
    break;

  case 343:

    { (yyval.t) = oz_mklistUnwrap(newCTerm(PA_fSynAction,(yyvsp[0].t))); }
    break;

  case 344:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 345:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fSynTemplateInstantiation, (yyvsp[0].t),
					   oz_consUnwrap(newCTerm(PA_fSynApplication,
							     terms[depth]->term,
							     AtomNil),
						    AtomNil),
					   (yyvsp[-2].t)),
				  (yyvsp[-1].t));
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  }
    break;

  case 346:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fSynAssignment, terms[depth]->term, (yyvsp[-1].t)),
				  (yyvsp[0].t));
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  }
    break;

  case 347:

    { while (terms[depth]) {
		      decls[depth] = oz_consUnwrap(terms[depth]->term, decls[depth]);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    (yyval.t) = (yyvsp[0].t);
		  }
    break;

  case 348:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 349:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 350:

    { terms[depth] = new TermNode((yyvsp[0].t), terms[depth]); }
    break;

  case 351:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 352:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }
    break;

  case 353:

    { (yyval.t) = newCTerm(PA_fSynAssignment,(yyvsp[-2].t),(yyvsp[0].t)); }
    break;

  case 354:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 355:

    { (yyval.t) = newCTerm(PA_fSynApplication,(yyvsp[0].t),AtomNil); }
    break;

  case 356:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[0].t),
				  oz_consUnwrap(newCTerm(PA_fSynApplication,(yyvsp[-3].t),
						    AtomNil),
					   AtomNil),(yyvsp[-1].t));
		  }
    break;

  case 357:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 358:

    { (yyval.t) = newCTerm(PA_fSynAssignment,
				  newCTerm(PA_fEscape,(yyvsp[-2].t),(yyvsp[-3].t)),(yyvsp[0].t)); }
    break;

  case 359:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 360:

    { (yyval.t) = (yyvsp[0].t); }
    break;

  case 361:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[0].t),
				  oz_mklistUnwrap((yyvsp[-2].t)),(yyvsp[-3].t));
		  }
    break;

  case 362:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[0].t),
				  oz_mklistUnwrap((yyvsp[-3].t)),(yyvsp[-1].t));
		  }
    break;

  case 363:

    { *prodKey[depth]++ = '('; depth++; }
    break;

  case 364:

    { depth--; }
    break;

  case 365:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[0].t),(yyvsp[-4].t),(yyvsp[-7].t)); }
    break;

  case 366:

    { *prodKey[depth]++ = '['; depth++; }
    break;

  case 367:

    { depth--; }
    break;

  case 368:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[0].t),(yyvsp[-4].t),(yyvsp[-7].t)); }
    break;

  case 369:

    { *prodKey[depth]++ = '{'; depth++; }
    break;

  case 370:

    { depth--; }
    break;

  case 371:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[0].t),(yyvsp[-4].t),(yyvsp[-7].t)); }
    break;

  case 372:

    { (yyval.t) = newCTerm(PA_fSynApplication,(yyvsp[0].t),AtomNil); }
    break;

  case 373:

    { (yyval.t) = newCTerm(PA_fSynApplication,(yyvsp[-4].t),(yyvsp[-1].t)); }
    break;

  case 374:

    { (yyval.t) = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); }
    break;

  case 375:

    { (yyval.t) = makeVar(xytext); }
    break;

  case 376:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }
    break;

  case 377:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }
    break;


    }

/* Line 1037 of yacc.c.  */


  yyvsp -= yylen;
  yyssp -= yylen;


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
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {

		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 yydestruct ("Error: popping",
                             yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval);
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

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

yyvsp -= yylen;
  yyssp -= yylen;
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


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
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
  yydestruct ("Error: discarding lookahead",
              yytoken, &yylval);
  yychar = YYEMPTY;
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}





void checkDeprecation(OZ_Term coord) {
  char *msg = "use `if' instead of `case' for boolean conditionals";
  if (xy_allowDeprecated) {
    xyreportWarning("deprecation warning",msg,coord);
  } else {
    xyreportError("deprecation error",msg,coord);
  }
}

void xyreportWarning(char *kind, char *msg, OZ_Term coord) {
  OZ_Term args = oz_mklist(oz_pair2(PA_coord, coord),
			   oz_pair2(PA_kind,  OZ_atom(kind)),
			   oz_pair2(PA_msg,   OZ_atom(msg)));
  xy_errorMessages = OZ_cons(OZ_recordInit(PA_warn,args),
			     xy_errorMessages);
}

void xyreportError(char *kind, char *msg, OZ_Term coord) {
  OZ_Term args = oz_mklist(oz_pair2(PA_coord, coord),
			   oz_pair2(PA_kind,  OZ_atom(kind)),
			   oz_pair2(PA_msg,   OZ_atom(msg)));
  xy_errorMessages = OZ_cons(OZ_recordInit(PA_error,args),
			     xy_errorMessages);
}

void xyreportError(char *kind, char *msg, const char *file,
		   int line, int column) {
  xyreportError(kind,msg,OZ_mkTupleC("pos",3,OZ_atom((char*)file),
				     oz_int(line),oz_int(column)));
}

static void xyerror(char *s) {
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

