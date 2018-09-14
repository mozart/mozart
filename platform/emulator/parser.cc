/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         xyparse
#define yylex           xylex
#define yyerror         xyerror
#define yydebug         xydebug
#define yynerrs         xynerrs

#define yylval          xylval
#define yychar          xychar

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
void xyreportWarning(char const *kind, char const *msg, OZ_Term coord);
void xyreportError(char const *kind, char const *msg, OZ_Term coord);
void xyreportError(char const *kind, char const *msg,
		   const char *file, int line, int column);


//-----------------
// Local Variables
//-----------------

#define YYMAXDEPTH 1000000
#define YYERROR_VERBOSE

static OZ_Term yyoutput;

static void xyerror(char const *);

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
OZ_Term makeInt(char const * chars, OZ_Term pos) {
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




# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_XY_Y_TAB_H_INCLUDED
# define YY_XY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int xydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
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
    T_COMPARE = 343,
    T_FDCOMPARE = 344,
    T_LMACRO = 345,
    T_RMACRO = 346,
    T_FDIN = 347,
    T_ADD = 348,
    T_FDMUL = 349,
    T_OTHERMUL = 350,
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
#define T_COMPARE 343
#define T_FDCOMPARE 344
#define T_LMACRO 345
#define T_RMACRO 346
#define T_FDIN 347
#define T_ADD 348
#define T_FDMUL 349
#define T_OTHERMUL 350
#define T_DEREFF 351

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{


  OZ_Term t;
  int i;


};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE xylval;

int xyparse (void);

#endif /* !YY_XY_Y_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */



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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  120
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2151

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  118
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  146
/* YYNRULES -- Number of rules.  */
#define YYNRULES  376
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  810

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   351

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
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
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
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
     987,   989,   991,   994,  1000,  1001,  1005,  1010,  1011,  1015,
    1019,  1022,  1036,  1060,  1061,  1066,  1067,  1073,  1074,  1079,
    1081,  1083,  1086,  1088,  1090,  1092,  1098,  1099,  1101,  1105,
    1109,  1111,  1113,  1115,  1120,  1121,  1126,  1127,  1129,  1134,
    1138,  1142,  1146,  1150,  1154,  1158,  1160,  1165,  1166,  1170,
    1172,  1179,  1180,  1185,  1186,  1190,  1195,  1202,  1204,  1206,
    1208,  1212,  1217,  1218,  1220,  1225,  1226,  1230,  1232,  1234,
    1236,  1238,  1240,  1244,  1246,  1250,  1254,  1256,  1258,  1260,
    1264,  1268,  1272,  1274,  1276,  1278,  1282,  1284,  1286,  1290,
    1292,  1296,  1300,  1302,  1305,  1309,  1311,  1313,  1315,  1321,
    1326,  1328,  1334,  1335,  1339,  1341,  1343,  1345,  1350,  1351,
    1355,  1357,  1361,  1363,  1365,  1367,  1369,  1371,  1376,  1377,
    1381,  1385,  1387,  1391,  1393,  1395,  1397,  1399,  1401,  1403,
    1407,  1409,  1411,  1413,  1415,  1417,  1421,  1424,  1427,  1429,
    1433,  1435,  1437,  1442,  1445,  1448,  1452,  1455,  1458,  1460,
    1464,  1466,  1470,  1472,  1476,  1480,  1482,  1485,  1489,  1491,
    1495,  1499,  1503,  1505,  1509,  1513,  1515,  1519,  1524,  1528,
    1537,  1545,  1547,  1549,  1551,  1553,  1555,  1559,  1561,  1565,
    1569,  1571,  1575,  1580,  1581,  1585,  1587,  1589,  1595,  1603,
    1605,  1607,  1609,  1614,  1615,  1619,  1621,  1625,  1627,  1631,
    1633,  1637,  1639,  1644,  1643,  1647,  1648,  1647,  1651,  1655,
    1657,  1659,  1663,  1664,  1667,  1671,  1672,  1671,  1673,  1674,
    1673,  1675,  1676,  1675,  1679,  1681,  1685,  1686,  1689,  1693,
    1694,  1697,  1698,  1702,  1710,  1711,  1715,  1717,  1721,  1723,
    1725,  1730,  1731,  1735,  1737,  1739,  1743,  1747,  1749,  1753,
    1762,  1767,  1768,  1772,  1774,  1784,  1789,  1796,  1798,  1802,
    1806,  1808,  1812,  1814,  1818,  1820,  1826,  1830,  1833,  1838,
    1840,  1844,  1848,  1849,  1848,  1852,  1853,  1852,  1856,  1857,
    1856,  1862,  1864,  1868,  1870,  1875,  1877
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
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
  "T_TRUE_LABEL", "T_try", "T_unit", "T_UNIT_LABEL", "T_for", "T_FOR",
  "T_do", "T_ENDOFFILE", "T_REGEX", "T_lex", "T_mode", "T_parser",
  "T_prod", "T_scanner", "T_syn", "T_token", "T_REDUCE", "T_SEP", "T_ITER",
  "'='", "T_OOASSIGN", "T_DOTASSIGN", "T_COLONEQUALS", "T_orelse",
  "T_andthen", "T_COMPARE", "T_FDCOMPARE", "T_LMACRO", "T_RMACRO",
  "T_FDIN", "'|'", "'#'", "T_ADD", "T_FDMUL", "T_OTHERMUL", "','", "'~'",
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
  "prodClauseList", "prodClause", "$@1", "@2", "$@3", "prodHeadRest",
  "prodName", "prodNameAtom", "prodKey", "$@4", "$@5", "$@6", "$@7", "$@8",
  "$@9", "prodParams", "prodParam", "separatorOp", "optTerminatorOp",
  "terminatorOp", "prodMakeKey", "localRules", "localRulesSub",
  "synClause", "synParams", "synParam", "synAlt", "synSeqs", "synSeq",
  "optSynAction", "nonEmptySeq", "synVariable", "synPrims", "synPrim",
  "synPrimNoAssign", "synPrimNoVar", "synPrimNoVarNoAssign", "$@10",
  "$@11", "$@12", "$@13", "$@14", "$@15", "synInstTerm", "synLabel",
  "synProdCallParams", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
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

#define YYPACT_NINF -584

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-584)))

#define YYTABLE_NINF -304

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-304)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1151,  -584,   343,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,    50,  -584,  -584,  -584,  -584,  -584,  1814,
    -584,  -584,  -584,  -584,  -584,    86,    25,  -584,  1264,   543,
    1594,   388,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,   346,  -584,  -584,   543,   -13,   112,   130,  -584,
     343,  -584,  1814,  1814,  1814,  -584,  1814,  1814,  1814,   137,
    1814,  -584,  1814,  1814,  1814,  1814,  1814,   137,  1814,  1814,
    1814,   188,  1814,   188,  -584,    96,   223,  -584,   157,   188,
     188,  1814,  1814,  1814,  1814,   174,   117,  1814,  1814,   188,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,   141,   159,
    -584,  -584,  -584,  -584,  -584,   543,  -584,  -584,  -584,  -584,
    -584,   210,   277,   263,   899,   265,  -584,   152,   244,   292,
     270,   275,   295,   294,   214,   137,   346,   305,   334,  1374,
     303,   348,   351,   278,   359,   364,   379,   171,  -584,   368,
     330,   336,  1814,   296,   265,   338,  -584,  -584,  -584,  -584,
    -584,   366,  -584,  -584,  -584,  -584,   266,   265,  1594,   345,
      48,  -584,  -584,  -584,  -584,   899,  1594,  -584,  1814,  1814,
    1814,  1814,  1814,  1814,  1814,  1814,  1814,  1814,  1814,  1814,
    1814,  1814,  1814,  1814,  1924,  1924,  1814,   548,   381,  1814,
     381,  -584,  1814,  -584,  1814,  -584,  -584,  -584,  -584,  -584,
     376,   265,  1814,  -584,  1814,   429,  1814,  1814,   543,  1814,
     396,  -584,  1814,  1814,  -584,   445,  -584,  1814,  1814,  1814,
    -584,  -584,  -584,  1814,  -584,  -584,  -584,   441,  1814,  -584,
    -584,  1814,  2034,  1814,  -584,  1814,   376,  -584,   243,   426,
     431,   455,   366,    53,    53,    53,  -584,  -584,   376,  -584,
    -584,  1814,   403,  1814,   408,   899,   749,   923,   446,   590,
     399,   490,   384,   384,  -584,   605,   202,  -584,  -584,   227,
     202,   202,   413,   423,   430,  1704,   532,   443,   449,   451,
     457,   532,   393,   346,   459,  -584,   548,  -584,  -584,   346,
     468,   564,   618,   346,   718,   563,    67,   529,   290,  1814,
    -584,  -584,   176,   176,  1814,  1814,  -584,   558,   376,  -584,
     539,  1814,  -584,  -584,  -584,   543,   543,  -584,   547,  1814,
    -584,  -584,   294,  1594,  -584,   572,   575,   576,  -584,  -584,
    -584,  1594,  -584,  -584,  1814,  1814,   578,   586,  1814,   899,
    1814,   779,  -584,   588,   899,   545,   243,   266,  -584,  -584,
    -584,   431,   431,   518,  -584,    44,   591,  -584,   609,  -584,
    -584,  -584,  -584,   521,   550,   530,   531,   436,  -584,  -584,
    -584,   899,  -584,  1814,  -584,  -584,   538,  1814,   554,  1814,
    -584,  -584,   188,  -584,  -584,  -584,   448,   137,   381,  1814,
    -584,  -584,  -584,  1814,  -584,  -584,  -584,  -584,  1814,  1814,
    1814,  -584,  -584,  -584,   563,  -584,  -584,  -584,   176,   540,
    -584,  -584,  -584,   176,  1594,  1594,   553,  -584,  -584,  1814,
    -584,  -584,  -584,  -584,  1814,  -584,  -584,   552,  -584,  1814,
    -584,  -584,  -584,  -584,  -584,  -584,   555,  -584,   635,  -584,
    -584,  -584,   810,  1484,  1814,  1814,  -584,   137,    28,  -584,
     366,   626,   633,   126,   593,    63,  1814,  -584,    96,  -584,
    -584,   232,  -584,  -584,   764,    63,  -584,   266,  -584,  -584,
     431,  -584,  -584,    53,  -584,  -584,    61,   188,   636,   436,
     436,   436,  -584,  -584,  -584,  -584,  -584,  1704,  -584,   346,
     548,   574,   114,  -584,  -584,   346,  1814,   651,  -584,  -584,
    -584,  -584,  -584,  -584,  1814,  1814,  1814,  -584,  -584,   176,
    1814,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,  -584,  1814,   606,   583,  -584,  -584,  -584,  -584,
    -584,  1814,   656,  -584,  -584,  -584,  -584,  1814,  1814,  -584,
    -584,  1814,  1814,  -584,  1814,   828,   845,  -584,  -584,   579,
    -584,   137,   658,    28,    28,   366,   431,  -584,  -584,  -584,
    -584,  -584,   589,   126,  -584,   257,   615,  -584,  -584,    63,
    -584,  -584,  -584,   188,   394,   437,   426,  -584,  -584,  -584,
    -584,   601,  -584,   266,  -584,   266,   266,  -584,  -584,   629,
     630,  1814,   240,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
    -584,  -584,   564,   188,  -584,  -584,  -584,  -584,    20,   662,
     620,  -584,  -584,   899,   497,   681,  -584,   722,   683,  -584,
     687,  -584,   882,   899,  -584,   616,  1814,  -584,  1814,  -584,
    1814,  -584,  -584,  -584,  -584,   431,   688,   431,  -584,  -584,
     426,  -584,   642,  -584,  -584,  -584,  -584,   266,   266,    63,
      63,  -584,  1814,  -584,  -584,  -584,    70,    70,   692,   188,
      70,  -584,  -584,   693,   240,  -584,   381,   448,  1814,  -584,
    -584,   188,  -584,  -584,  -584,  -584,  -584,   617,    51,   722,
     721,  -584,   451,  -584,  -584,  -584,  -584,  -584,   899,   899,
     899,  -584,   700,  -584,   705,  -584,  -584,   426,   431,   431,
     431,  -584,  -584,  -584,  -584,   634,   706,   708,  -584,   188,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,   182,   637,  -584,
    -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,  -584,
     550,   638,   632,   639,  -584,  -584,  -584,  -584,  -584,  -584,
     721,  -584,  -584,  1814,   431,  -584,  -584,  -584,  -584,   899,
    -584,   266,   266,   266,  -584,  -584,  -584,  -584,  -584,  -584
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     3,    17,    14,    15,    16,   250,   147,   257,   255,
     256,   254,   251,   151,   259,   259,   259,   259,   259,   259,
      74,    52,   150,   259,   259,   259,   259,   259,   259,   259,
     259,   259,    53,    73,   259,    51,   149,   259,    50,   148,
     259,   259,   259,   303,   259,   259,   259,   259,   259,     0,
      49,    54,   259,   259,   259,     0,     0,     6,   258,    12,
      21,    32,    58,   259,   259,    64,    47,   252,    48,    55,
      56,    57,     0,    80,    81,    12,   291,     0,     0,    13,
      17,    67,     0,     0,   258,    76,     0,     0,     0,   105,
     258,    66,     0,     0,     0,     0,     0,   105,     0,     0,
       0,     0,    85,     0,   295,     0,     0,   323,     0,   302,
       0,   137,     0,     0,     0,   136,     0,     0,     0,     0,
       1,     2,     8,     4,   259,   259,   259,   259,   259,   129,
     130,   131,   259,    22,   259,   259,   259,    42,   259,   132,
     133,   134,   259,   259,   259,   259,   259,   259,     0,     0,
     259,   259,   259,   259,   259,    12,     5,   292,    19,    20,
      18,     0,   248,     0,   190,   192,   191,     0,   237,   238,
     258,   258,     0,     0,     0,   105,   107,     0,     0,    21,
       0,     0,     0,     0,     0,     0,   141,     0,   100,     0,
       0,     0,    85,    47,   192,     0,   304,   293,   321,   322,
     300,   324,   305,   308,   311,   301,   319,   192,   137,     0,
      40,    44,    45,   259,    46,   258,   137,   253,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   152,   152,     0,   126,   116,     0,
     116,     7,     0,   259,     0,   259,   259,   259,   259,   259,
     208,   192,     0,   259,     0,     0,     0,   258,    12,     0,
     244,   259,     0,     0,   106,     0,   108,     0,     0,     0,
     259,   259,   259,     0,   259,   259,   259,   143,     0,    99,
     259,     0,     0,     0,    86,     0,   208,   296,   303,     0,
     258,     0,   326,     0,     0,     0,   299,   320,   208,   138,
     259,     0,     0,   139,     0,    23,    25,    24,    26,    27,
      31,    28,    29,    30,    33,    34,    39,    41,    43,    36,
      37,    38,    52,    51,    50,   152,   155,     0,    47,   252,
      56,   155,   107,   107,     0,   163,   126,   164,   259,   107,
       0,   124,   107,   107,   187,     0,   176,     0,   182,     0,
     249,    79,     0,     0,     0,     0,   259,     0,   208,   193,
       0,     0,   236,   259,   239,    12,    12,    11,   258,     0,
      78,   243,   242,   137,   259,     0,     0,     0,    68,    75,
      77,   137,    72,    70,     0,     0,     0,     0,     0,    89,
       0,    90,    88,     0,    87,   283,   303,     0,   323,   373,
     374,   258,   258,     0,   259,   341,     0,   336,   337,   325,
     327,   317,   316,     0,   314,     0,     0,     0,    82,   135,
     259,   258,   259,     0,   153,   156,     0,     0,     0,     0,
     115,   113,     0,   127,   119,   112,     0,     0,   116,     0,
     111,   109,   259,     0,   259,   259,   175,   259,     0,     0,
       0,   259,   258,   259,     0,   207,   206,   205,   198,   201,
     202,   203,   204,   198,   137,   137,     0,   259,   209,     0,
     240,   235,    10,     9,     0,   245,   246,     0,    63,     0,
     259,   259,   169,   259,    65,   259,     0,   142,   179,   144,
     259,   259,   103,    21,     0,     0,   259,     0,     0,   323,
     324,     0,     0,   331,   341,   341,     0,   259,   371,   349,
     259,   259,   348,   339,   341,   341,   358,   359,   259,   298,
     258,   306,   318,     0,   309,   312,     0,     0,     0,   261,
     262,   263,    59,   140,    60,    35,   259,   152,   259,   107,
     126,     0,   120,   125,   117,   107,     0,     0,   173,   172,
     171,   177,   178,   181,     0,     0,     0,   259,   195,   198,
       0,   196,   194,   197,   220,   221,   217,   225,   216,   224,
     215,   223,   259,     0,   211,     0,   213,   214,   189,   241,
     247,     0,     0,   167,   166,   165,    69,     0,     0,    71,
      83,     0,     0,   259,     0,    94,    97,    92,    84,   287,
     284,   285,     0,   280,   279,   324,   258,   328,   329,   335,
     334,   333,     0,   331,   340,   354,     0,   350,   346,   341,
     353,   356,   342,     0,     0,     0,   303,   259,   343,   347,
     259,     0,   338,   319,   315,   319,   319,   271,   270,     0,
       0,     0,   273,   259,   264,   265,   266,   145,   154,   146,
     114,   128,   124,     0,   122,   110,   188,   174,   185,   183,
     186,   170,   199,   200,     0,     0,   259,   227,     0,   168,
       0,   180,   103,   104,   102,     0,     0,    91,     0,    96,
       0,   286,   259,   282,   281,   258,     0,   258,   332,   259,
     303,   351,     0,   362,   365,   368,   371,   319,   354,   341,
     341,   323,   137,   307,   310,   313,     0,     0,     0,     0,
       0,   276,   277,     0,   273,   260,   116,   121,     0,   222,
     210,     0,   162,   161,   160,   232,   231,     0,     0,   227,
     234,   157,   230,   159,   259,   259,   259,    93,    95,    98,
     288,   278,     0,   294,     0,   323,   352,   303,   258,   258,
     258,   323,   345,   323,   361,     0,     0,     0,   269,   289,
     275,   272,   274,   118,   123,   184,   212,     0,     0,   218,
     226,   259,   228,    62,    61,   101,   297,   330,   355,   357,
     375,     0,     0,     0,   360,   344,   372,   267,   268,   290,
     234,   230,   219,     0,   258,   363,   366,   369,   229,   233,
     376,   319,   319,   319,   323,   323,   323,   364,   367,   370
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -584,  -584,   690,    14,  -584,   670,  -584,   108,   414,  -379,
     104,   559,  -584,  -584,  -584,   161,  -584,  -584,   577,    87,
     -31,  -292,   691,  -219,  -584,  -497,   113,  -331,  -584,  -584,
    -584,  -584,  -584,  -584,     0,  -188,   349,  -584,  -584,  -584,
    -584,  -584,  -228,   435,  -552,  -402,  -202,  -584,  -199,   313,
      69,   189,  -378,  -584,  -266,  -584,   686,    43,  -584,  -401,
     177,  -584,  -206,  -584,  -584,  -584,  -584,    52,  -584,    15,
      -6,  -584,  -584,   533,  -584,   -50,   524,   546,   422,    62,
     291,  -584,  -147,  -584,   284,   -15,  -584,   -58,  -584,  -561,
    -163,  -560,    78,  -584,  -584,   -49,  -584,   192,  -584,    35,
     724,  -421,  -584,  -584,  -584,  -221,   -25,    -8,  -584,  -584,
    -584,  -584,  -584,  -584,  -584,  -271,  -584,    17,  -579,   -89,
    -343,  -426,   506,  -396,   190,  -584,  -254,   281,  -584,  -312,
     293,  -584,  -473,  -584,  -583,  -584,  -341,  -584,  -584,  -584,
    -584,  -584,  -584,   180,   519,  -476
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
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

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
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
     186,   449,   604,   451,    43,   106,   289,   746,    59,   218,
     219,   220,   221,   222,   453,   727,   158,   223,   751,   224,
     225,   226,     6,   227,   506,     9,    10,   228,   229,   230,
     231,   232,   233,   638,   159,   236,   237,   238,   239,   240,
     542,    12,   638,   506,   264,     6,   691,   501,   502,   143,
     144,  -303,   468,   711,   712,  -303,   605,  -303,   662,   769,
     104,   507,   411,   189,   779,   194,    59,   656,   133,   241,
    -303,   206,   207,   516,  -303,   499,  -303,   727,   330,   685,
      54,   217,   603,   603,     6,   477,    12,     9,    10,   337,
     161,    12,   614,   486,   167,   170,   171,    12,   301,   252,
     177,   178,   512,    12,   171,   462,   462,   604,   604,   651,
     488,   196,   371,   253,   455,   137,   210,   211,   212,   544,
     764,   213,   804,   805,   806,   214,   752,   753,   349,   653,
     351,   352,   353,   354,   355,   609,   610,   286,   361,   456,
     137,   278,   457,   549,   162,   548,   370,   650,   234,   189,
     298,     6,   634,   655,   362,   378,   379,   380,    12,   382,
     383,   384,   242,   108,   202,   388,   235,   375,   203,   377,
     204,   243,   367,   782,   783,   254,   562,   563,   387,   584,
     109,   583,   709,   393,    72,   418,   246,   133,   658,   337,
     660,   725,   726,    54,   359,   244,   329,   329,   245,   336,
     341,   142,   341,   143,   144,   247,   197,   248,   200,   648,
     256,   462,   262,   710,   527,   542,   462,   257,   800,   198,
     199,   249,   259,   434,   140,   141,   142,   263,   143,   144,
     261,   315,   316,   317,   318,   319,   320,   321,   270,  -302,
    -252,   466,    72,  -302,   332,  -302,   344,   342,   471,   454,
     397,   402,   686,   198,   199,   412,   412,   412,   754,   478,
     360,   470,   198,   199,   167,   365,   267,   368,   166,   476,
     171,   108,   281,   451,   166,   150,   376,   282,   452,   472,
     473,   268,   151,   271,   453,   489,   272,   329,   109,   504,
     330,   273,   152,   337,   274,   337,    12,   338,   336,   275,
     153,   137,   778,   276,   154,   532,   283,   534,   784,   419,
     785,   285,   462,   124,   125,   280,   126,   127,   128,   129,
     130,   287,   150,   131,   132,   627,   356,   546,   630,   151,
      92,   742,   550,   744,     6,   399,   554,   300,   556,   152,
     429,    12,   400,   547,   289,     6,   399,   153,    77,    78,
     553,   154,   578,   400,   258,   260,     6,   369,   397,     9,
      10,   807,   808,   809,   363,   105,    92,   509,   585,   579,
     586,   644,   645,   646,   580,   589,   590,  -304,   132,   582,
     374,   598,   385,   138,   139,   140,   141,   142,   344,   143,
     144,   404,   623,   132,   540,   624,   625,   763,   164,   303,
     341,   693,   409,   631,   164,   694,   622,   695,   179,   526,
     527,   175,    12,   719,   755,   420,   190,   551,   552,   175,
     733,   647,   422,   649,   193,   208,   689,   315,  -162,   458,
     463,   215,   216,   127,   128,   129,   130,   539,  -161,   131,
     132,   366,   661,   756,   757,  -160,     2,   545,     3,     4,
       5,   425,   344,   344,   683,   684,     6,   664,   427,     9,
      10,     6,   564,    12,  -157,   611,  -158,   615,    12,   565,
     337,    18,  -159,   665,   432,   436,   509,   615,   674,  -304,
    -304,   668,   733,   131,   132,   412,   437,   670,   640,   642,
     450,   566,   567,   467,   443,   444,   445,   175,   446,   329,
     469,   133,   336,   479,   480,   481,   190,   482,   474,   689,
     484,   485,   700,   490,   193,   701,   568,   569,   137,   570,
     571,   491,   208,   496,   497,   503,   519,   520,   715,   521,
     208,   522,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   708,   524,   461,   461,   525,   536,   150,   325,   325,
     315,   721,   475,   588,   151,   560,   328,   328,   344,   335,
     344,   607,   538,   659,   152,   439,   581,   741,   608,   587,
     572,   643,   153,   506,   745,   611,   154,   373,   128,   129,
     130,   615,   652,   131,   132,   692,   657,   381,   698,   666,
     667,   669,   344,   682,   680,   389,   391,   687,   690,   394,
     423,   139,   140,   141,   142,   303,   143,   144,   702,   718,
     105,   401,   706,   707,   453,   717,   720,   421,   734,   773,
     774,   775,   735,   743,   737,   747,   217,   758,   761,   732,
       6,   137,   767,     9,    10,   776,   555,    12,   771,   325,
     777,   787,   786,   788,   796,   792,   795,   328,   122,   461,
     160,   284,   698,   797,   461,   675,   793,   577,   335,   736,
     722,   615,   615,   155,   279,   716,   428,   557,   464,   465,
     533,   759,     6,   399,   460,   460,   176,   671,   341,    12,
     400,   770,   790,   766,   798,   723,   372,   208,   724,   364,
     350,   732,   762,   681,   789,   208,   616,   794,   410,   494,
     157,   632,   492,   688,   493,   697,   616,   628,   403,   698,
       0,   505,     0,   442,   139,   140,   141,   142,   105,   143,
     144,   759,     0,     0,     0,     0,   765,   508,     0,   791,
     591,   725,   726,   125,     0,   126,   127,   128,   129,   130,
       0,   537,   131,   132,   506,     0,     0,   626,     0,     0,
     461,     0,     0,     0,     0,     0,     0,     0,   335,   543,
     198,   199,   124,   125,     0,   126,   127,   128,   129,   130,
       0,  -303,   131,   132,     0,  -303,     0,  -303,   208,   208,
     460,   507,     0,     0,     0,   460,     0,     0,   576,     0,
       0,     0,     0,   124,   125,   495,   126,   127,   128,   129,
     130,     0,     0,   131,   132,     0,     0,     0,   595,   596,
     616,   124,   125,     0,   126,   127,   128,   129,   130,   599,
       0,   131,   132,     0,     0,     0,   592,   508,   124,   125,
       0,   126,   127,   128,   129,   130,   508,   508,   131,   132,
       0,     0,     0,     0,   676,     0,     0,     0,   639,     0,
       0,   325,     0,     0,     0,     0,     0,     0,     0,   328,
       0,   678,   335,     0,   335,   124,   125,     0,   126,   127,
     128,   129,   130,     0,   663,   131,   132,     0,     0,     0,
       0,   460,   124,   125,     0,   126,   127,   128,   129,   130,
     616,   616,   131,   132,     0,     0,     0,     0,   592,     0,
       0,     0,     0,     0,     0,   672,   673,     0,   596,   126,
     127,   128,   129,   130,     0,     0,   131,   132,     0,     0,
       0,     0,     0,   599,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   508,     0,     0,     0,     0,     0,   696,   508,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
    -258,    19,     0,     0,     0,     0,     0,  -258,    20,    21,
      22,     0,     0,     0,    23,    24,    25,  -258,     0,    26,
      27,     0,    28,     0,    29,  -258,    30,   799,    31,  -258,
      32,    33,     0,    34,    35,    36,    37,    38,    39,    40,
      41,     0,   -12,     0,     0,     0,    42,    43,    44,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    45,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    45,     0,     0,     0,     0,
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
     126,   127,   128,   129,   130,    45,     0,   131,   132,     0,
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
     126,   127,   128,   129,   130,    45,     0,   131,   132,     0,
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
     126,   127,   128,   129,   130,    45,     0,   131,   132,     0,
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
     126,   127,   128,   129,   130,    45,     0,   131,   132,     0,
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
       0,     0,     0,     0,     0,    45,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    45,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
       0,   390,     0,    50,    51,    52,     0,    53,     0,     0,
       0,    54
};

static const yytype_int16 yycheck[] =
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
     100,    34,   498,    83,    76,    43,    78,   690,     0,   124,
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
     588,   115,   262,    61,    38,    13,   112,   113,   114,   438,
     717,    47,   801,   802,   803,   108,   699,   700,   243,   115,
     245,   246,   247,   248,   249,   109,   110,   194,   253,    63,
      13,    70,    66,   445,   244,   444,   261,   539,   107,   187,
     207,     8,   523,   545,   254,   270,   271,   272,    15,   274,
     275,   276,    52,   288,   107,   280,   107,   267,   111,   269,
     113,    61,   258,   749,   750,    31,   464,   465,   278,   481,
     288,   480,    42,   283,     0,   300,    21,   179,   554,   436,
     556,   109,   110,   117,   251,    18,   234,   235,    35,   237,
     238,    99,   240,   101,   102,    40,    83,    42,   397,   537,
      18,   458,    18,    73,    74,   717,   463,    47,   794,    96,
      97,    56,    47,   338,    97,    98,    99,   113,   101,   102,
      35,   227,   228,   229,   230,   231,   232,   233,    35,   107,
      83,   356,    58,   111,   236,   113,   242,   239,   363,   349,
     288,   289,   606,    96,    97,   293,   294,   295,   701,   374,
     252,   361,    96,    97,   256,   257,    61,   259,    84,   369,
     262,   396,    42,    83,    90,    29,   268,    47,    88,   365,
     366,    47,    36,    35,    94,   385,    35,   325,   396,   404,
     537,   113,    46,   540,    35,   542,    15,    16,   336,    35,
      54,    13,   745,    24,    58,   420,    70,   422,   751,   301,
     753,   115,   559,    83,    84,    47,    86,    87,    88,    89,
      90,    83,    29,    93,    94,   514,    50,   442,   517,    36,
     445,   685,   447,   687,     8,     9,   451,    92,   453,    46,
      47,    15,    16,   443,    78,     8,     9,    54,   105,   106,
     450,    58,   467,    16,   170,   171,     8,    61,   396,    11,
      12,   804,   805,   806,    35,    43,   481,   405,   483,   469,
     485,   529,   530,   531,   474,   490,   491,    93,    94,   479,
      35,   496,    41,    95,    96,    97,    98,    99,   384,   101,
     102,    60,   507,    94,   432,   510,   511,   716,    84,   215,
     438,   107,    47,   518,    90,   111,   506,   113,    94,    73,
      74,    89,    15,    16,   702,   112,   102,   448,   449,    97,
     667,   536,   114,   538,   102,   111,   615,   423,   115,   352,
     353,   117,   118,    87,    88,    89,    90,   429,   115,    93,
      94,   257,   557,   706,   707,   115,     3,   439,     5,     6,
       7,    19,   448,   449,   603,   604,     8,   572,   115,    11,
      12,     8,     9,    15,   115,   503,   115,   505,    15,    16,
     717,    28,   115,   573,   115,   107,   514,   515,   593,    89,
      90,   581,   729,    93,    94,   523,    22,   587,   526,   527,
      61,    38,    39,    35,    31,    32,    33,   175,    35,   537,
      61,   493,   540,    31,    32,    33,   192,    35,    61,   698,
      35,    35,   627,    35,   192,   630,    63,    64,    13,    66,
      67,    35,   208,    35,    79,   107,    35,    18,   643,   108,
     216,    81,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   641,   112,   352,   353,   114,   108,    29,   234,   235,
     546,   666,   368,    18,    36,   115,   234,   235,   554,   237,
     556,    35,   108,   555,    46,    47,   114,   682,    35,   114,
     117,    35,    54,    80,   689,   613,    58,   263,    88,    89,
      90,   619,   108,    93,    94,   623,    35,   273,   626,    83,
     107,    35,   588,    35,   115,   281,   282,   108,    83,   285,
      95,    96,    97,    98,    99,   421,   101,   102,   107,    47,
     288,   289,    83,    83,    94,   653,    35,   303,    35,   734,
     735,   736,    35,    35,   108,    83,   664,    35,    35,   667,
       8,    13,   115,    11,    12,    35,   452,    15,    17,   325,
      35,    35,   108,    35,   112,   108,   108,   325,    58,   458,
      80,   192,   690,   114,   463,   594,   771,   466,   336,   672,
      38,   699,   700,    72,   187,   652,   331,   454,   354,   355,
     421,   709,     8,     9,   352,   353,    90,   588,   716,    15,
      16,   729,   767,   721,   790,    63,   262,   373,    66,   256,
     244,   729,   714,   601,   759,   381,   505,   780,   292,    20,
      76,   520,   388,   613,   390,   625,   515,   514,   289,   747,
      -1,    47,    -1,    95,    96,    97,    98,    99,   396,   101,
     102,   759,    -1,    -1,    -1,    -1,   718,   405,    -1,   767,
      20,   109,   110,    84,    -1,    86,    87,    88,    89,    90,
      -1,   427,    93,    94,    80,    -1,    -1,    83,    -1,    -1,
     559,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   436,   437,
      96,    97,    83,    84,    -1,    86,    87,    88,    89,    90,
      -1,   107,    93,    94,    -1,   111,    -1,   113,   464,   465,
     458,   117,    -1,    -1,    -1,   463,    -1,    -1,   466,    -1,
      -1,    -1,    -1,    83,    84,   116,    86,    87,    88,    89,
      90,    -1,    -1,    93,    94,    -1,    -1,    -1,   494,   495,
     619,    83,    84,    -1,    86,    87,    88,    89,    90,   497,
      -1,    93,    94,    -1,    -1,    -1,   116,   505,    83,    84,
      -1,    86,    87,    88,    89,    90,   514,   515,    93,    94,
      -1,    -1,    -1,    -1,   116,    -1,    -1,    -1,   526,    -1,
      -1,   537,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   537,
      -1,   116,   540,    -1,   542,    83,    84,    -1,    86,    87,
      88,    89,    90,    -1,   560,    93,    94,    -1,    -1,    -1,
      -1,   559,    83,    84,    -1,    86,    87,    88,    89,    90,
     699,   700,    93,    94,    -1,    -1,    -1,    -1,   116,    -1,
      -1,    -1,    -1,    -1,    -1,   591,   592,    -1,   594,    86,
      87,    88,    89,    90,    -1,    -1,    93,    94,    -1,    -1,
      -1,    -1,    -1,   601,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   619,    -1,    -1,    -1,    -1,    -1,   625,   626,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
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
      86,    87,    88,    89,    90,    91,    -1,    93,    94,    -1,
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
      86,    87,    88,    89,    90,    91,    -1,    93,    94,    -1,
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
      86,    87,    88,    89,    90,    91,    -1,    93,    94,    -1,
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
      86,    87,    88,    89,    90,    91,    -1,    93,    94,    -1,
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
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,   107,    -1,   109,   110,   111,    -1,   113,    -1,    -1,
      -1,   117
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     1,     3,     5,     6,     7,     8,     9,    10,    11,
      12,    14,    15,    16,    23,    25,    26,    27,    28,    30,
      37,    38,    39,    43,    44,    45,    48,    49,    51,    53,
      55,    57,    59,    60,    62,    63,    64,    65,    66,    67,
      68,    69,    75,    76,    77,    91,   100,   103,   104,   107,
     109,   110,   111,   113,   117,   119,   120,   121,   122,   125,
     126,   128,   157,   158,   159,   173,   196,   197,   198,   199,
     200,   201,   202,   204,   212,   218,   219,   105,   106,   123,
     124,   166,   203,   203,   203,   189,   203,   203,   203,   203,
     203,   164,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   203,   203,   110,   196,   197,   223,   224,   225,
     203,   203,   203,   203,   203,   125,   152,   203,   203,   203,
       0,    71,   120,   121,    83,    84,    86,    87,    88,    89,
      90,    93,    94,   125,   146,   147,   148,    13,    95,    96,
      97,    98,    99,   101,   102,   149,   150,   151,   203,   203,
      29,    36,    46,    54,    58,   140,   121,   218,     4,     4,
     123,   125,   152,   195,   126,   174,   202,   125,   191,   192,
     125,   125,   193,   194,   138,   196,   174,   125,   125,   126,
     152,   152,   193,   138,   152,   152,   152,   135,   136,   197,
     126,   129,   130,   196,   197,   221,   115,    83,    96,    97,
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
      92,   203,   154,   202,   153,   126,   126,   126,   126,   126,
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
     131,   131,   131,   131,   132,   132,   133,   134,   134,   135,
     135,   136,   136,   137,   137,   138,   138,   139,   139,   140,
     140,   140,   140,   140,   140,   140,   141,   141,   141,   142,
     143,   143,   143,   143,   144,   144,   145,   145,   145,   146,
     147,   148,   149,   150,   151,   152,   152,   153,   153,   154,
     154,   155,   155,   156,   156,   157,   157,   158,   158,   158,
     158,   159,   160,   160,   160,   161,   161,   162,   162,   162,
     162,   162,   162,   163,   163,   164,   165,   165,   165,   165,
     166,   166,   167,   167,   167,   167,   168,   168,   168,   169,
     169,   170,   171,   171,   171,   172,   172,   172,   172,   173,
     174,   174,   175,   175,   176,   176,   176,   176,   177,   177,
     178,   178,   179,   179,   179,   179,   179,   179,   180,   180,
     181,   182,   182,   183,   183,   183,   183,   183,   183,   183,
     184,   184,   184,   184,   184,   184,   185,   185,   186,   186,
     187,   187,   187,   188,   188,   189,   190,   190,   191,   191,
     192,   192,   193,   193,   194,   194,   194,   194,   195,   195,
     196,   197,   198,   198,   199,   200,   200,   201,   202,   203,
     204,   205,   205,   205,   205,   205,   205,   206,   206,   207,
     208,   208,   209,   210,   210,   211,   211,   211,   212,   213,
     213,   213,   213,   214,   214,   215,   215,   216,   216,   217,
     217,   218,   218,   220,   219,   221,   222,   219,   219,   223,
     223,   223,   224,   224,   225,   227,   228,   226,   229,   230,
     226,   231,   232,   226,   233,   233,   234,   234,   235,   236,
     236,   237,   237,   238,   239,   239,   240,   240,   241,   241,
     241,   242,   242,   243,   243,   243,   244,   245,   245,   246,
     246,   247,   247,   248,   248,   248,   248,   248,   248,   249,
     250,   250,   251,   251,   252,   252,   252,   253,   253,   254,
     254,   254,   255,   256,   254,   257,   258,   254,   259,   260,
     254,   261,   261,   262,   262,   263,   263
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
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
       1,     1,     5,     7,     7,     0,     2,     3,     3,     3,
       1,     4,     3,     5,     0,     2,     2,     0,     2,     2,
       1,     8,     6,     0,     2,     0,     2,     0,     1,     4,
       6,     4,     4,     4,     6,     4,     0,     3,     6,     2,
       1,     3,     2,     4,     0,     2,     0,     2,     4,     1,
       1,     1,     1,     1,     1,     4,     1,     0,     2,     1,
       3,     0,     3,     0,     2,     7,     7,     1,     1,     1,
       1,     1,     0,     2,     4,     0,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     6,     2,     2,     3,     1,
       7,     6,     2,     2,     3,     1,     1,     3,     3,     1,
       3,     3,     1,     4,     6,     4,     4,     1,     4,     7,
       1,     1,     0,     2,     4,     4,     4,     4,     0,     2,
       3,     1,     1,     1,     1,     1,     1,     1,     0,     2,
       5,     1,     4,     1,     1,     1,     1,     1,     4,     5,
       1,     1,     3,     1,     1,     1,     2,     0,     2,     4,
       1,     1,     1,     3,     0,     5,     2,     0,     1,     3,
       4,     5,     3,     3,     2,     4,     4,     5,     1,     3,
       1,     1,     1,     3,     1,     1,     1,     1,     0,     0,
       8,     1,     1,     1,     2,     2,     2,     5,     5,     4,
       1,     1,     4,     0,     2,     2,     1,     1,     9,     1,
       1,     2,     2,     0,     2,     1,     2,     1,     3,     1,
       2,     1,     2,     0,     9,     0,     0,    10,     6,     3,
       2,     2,     1,     0,     2,     0,     0,     6,     0,     0,
       6,     0,     0,     6,     1,     3,     1,     1,     1,     0,
       1,     1,     1,     0,     0,     2,     1,     2,     4,     4,
       7,     0,     2,     1,     1,     1,     1,     1,     3,     2,
       3,     0,     2,     2,     5,     4,     2,     2,     1,     1,
       1,     2,     3,     1,     1,     4,     1,     5,     1,     1,
       5,     4,     0,     0,     9,     0,     0,     9,     0,     0,
       9,     1,     5,     1,     1,     1,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
     '$$ = $1'.

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

    { (yyval.t) = newCTerm(oz_atom("forFeature"),(yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 88:

    { (yyval.t) = newCTerm(oz_atom("forPattern"),(yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 89:

    { (yyval.t) = newCTerm(oz_atom("forFrom"),(yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 90:

    { (yyval.t) = newCTerm(oz_atom("forGeneratorList"),(yyvsp[0].t)); }

    break;

  case 91:

    { (yyval.t) = newCTerm(oz_atom("forGeneratorInt"),(yyvsp[-3].t),(yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 92:

    { (yyval.t) = newCTerm(oz_atom("forGeneratorC"),(yyvsp[-2].t),oz_headUnwrap((yyvsp[0].t)),
                                                              oz_tailUnwrap((yyvsp[0].t))); }

    break;

  case 93:

    { (yyval.t) = newCTerm(oz_atom("forGeneratorC"),(yyvsp[-3].t),oz_headUnwrap((yyvsp[-1].t)),
                                                              oz_tailUnwrap((yyvsp[-1].t))); }

    break;

  case 94:

    { (yyval.t) = NameUnit; }

    break;

  case 95:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 96:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 97:

    { (yyval.t) = NameUnit; }

    break;

  case 98:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 99:

    {
		    (yyval.t) = newCTerm(PA_fAnd,(yyvsp[-1].t),(yyvsp[0].t));
		  }

    break;

  case 101:

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

  case 102:

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

  case 103:

    { (yyval.t) = 0; }

    break;

  case 104:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 105:

    { (yyval.t) = AtomNil; }

    break;

  case 106:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 107:

    { (yyval.t) = AtomNil; }

    break;

  case 108:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 109:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fRequire,(yyvsp[-1].t),(yyvsp[-2].t)),(yyvsp[0].t)); }

    break;

  case 110:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fPrepare,(yyvsp[-3].t),(yyvsp[-1].t),(yyvsp[-4].t)),(yyvsp[0].t)); }

    break;

  case 111:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fPrepare,(yyvsp[-1].t),
					   newCTerm(PA_fSkip,(yyvsp[-2].t)),(yyvsp[-2].t)),(yyvsp[0].t)); }

    break;

  case 112:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fImport,(yyvsp[-1].t),(yyvsp[-2].t)),(yyvsp[0].t)); }

    break;

  case 113:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fExport,(yyvsp[-1].t),(yyvsp[-2].t)),(yyvsp[0].t)); }

    break;

  case 114:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fDefine,(yyvsp[-3].t),(yyvsp[-1].t),(yyvsp[-4].t)),(yyvsp[0].t)); }

    break;

  case 115:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fDefine,(yyvsp[-1].t),
					   newCTerm(PA_fSkip,(yyvsp[-2].t)),(yyvsp[-2].t)),(yyvsp[0].t)); }

    break;

  case 116:

    { (yyval.t) = AtomNil; }

    break;

  case 117:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fImportItem,(yyvsp[-2].t),AtomNil,(yyvsp[-1].t)),(yyvsp[0].t)); }

    break;

  case 118:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fImportItem,(yyvsp[-5].t),(yyvsp[-3].t),(yyvsp[-1].t)),(yyvsp[0].t)); }

    break;

  case 119:

    { (yyval.t) = newCTerm(PA_fVar,OZ_atom(xytext),(yyvsp[0].t)); }

    break;

  case 120:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 121:

    { (yyval.t) = oz_mklistUnwrap(oz_pair2Unwrap((yyvsp[0].t),(yyvsp[-2].t))); }

    break;

  case 122:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 123:

    { (yyval.t) = oz_consUnwrap(oz_pair2Unwrap((yyvsp[-1].t),(yyvsp[-3].t)),(yyvsp[0].t)); }

    break;

  case 124:

    { (yyval.t) = PA_fNoImportAt; }

    break;

  case 125:

    { (yyval.t) = newCTerm(PA_fImportAt,(yyvsp[0].t)); }

    break;

  case 126:

    { (yyval.t) = AtomNil; }

    break;

  case 127:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fExportItem,(yyvsp[-1].t)),(yyvsp[0].t)); }

    break;

  case 128:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fExportItem,
					   newCTerm(PA_fColon,(yyvsp[-3].t),(yyvsp[-1].t))),(yyvsp[0].t)); }

    break;

  case 129:

    { (yyval.t) = OZ_atom(xytext); }

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

    { (yyval.t) = newCTerm(PA_fLocal,(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }

    break;

  case 136:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 137:

    { (yyval.t) = AtomNil; }

    break;

  case 138:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 139:

    { (yyval.t) = newCTerm(PA_fAtom,AtomNil,(yyvsp[0].t)); }

    break;

  case 140:

    { (yyval.t) = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomCons,(yyvsp[-2].t)),
				  oz_mklistUnwrap((yyvsp[-1].t),(yyvsp[0].t))); }

    break;

  case 141:

    { (yyval.t) = PA_fNoCatch; }

    break;

  case 142:

    { (yyval.t) = newCTerm(PA_fCatch,(yyvsp[0].t),(yyvsp[-1].t)); }

    break;

  case 143:

    { (yyval.t) = PA_fNoFinally; }

    break;

  case 144:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 145:

    {
		    (yyval.t) = newCTerm(OZ_isTrue((yyvsp[-2].t))? PA_fOpenRecord : PA_fRecord,
				  newCTerm(PA_fAtom,(yyvsp[-6].t),makeLongPos((yyvsp[-5].t),(yyvsp[0].t))),(yyvsp[-3].t));
		  }

    break;

  case 146:

    {
		    (yyval.t) = newCTerm(OZ_isTrue((yyvsp[-2].t))? PA_fOpenRecord : PA_fRecord,
				  makeVar((yyvsp[-6].t),makeLongPos((yyvsp[-5].t),(yyvsp[0].t))),(yyvsp[-3].t));
		  }

    break;

  case 147:

    { (yyval.t) = OZ_atom(xytext); }

    break;

  case 148:

    { (yyval.t) = NameUnit; }

    break;

  case 149:

    { (yyval.t) = NameTrue; }

    break;

  case 150:

    { (yyval.t) = NameFalse; }

    break;

  case 151:

    { (yyval.t) = OZ_atom(xytext); }

    break;

  case 152:

    { (yyval.t) = AtomNil; }

    break;

  case 153:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 154:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fColon,(yyvsp[-3].t),(yyvsp[-1].t)),(yyvsp[0].t)); }

    break;

  case 155:

    { (yyval.t) = NameFalse; }

    break;

  case 156:

    { (yyval.t) = NameTrue; }

    break;

  case 157:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 158:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 159:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 160:

    { (yyval.t) = newCTerm(PA_fAtom,NameUnit,pos()); }

    break;

  case 161:

    { (yyval.t) = newCTerm(PA_fAtom,NameTrue,pos()); }

    break;

  case 162:

    { (yyval.t) = newCTerm(PA_fAtom,NameFalse,pos()); }

    break;

  case 163:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 164:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 165:

    { (yyval.t) = newCTerm(PA_fBoolCase,(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[-1].t),makeLongPos((yyvsp[-5].t),(yyvsp[0].t))); }

    break;

  case 166:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 167:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 168:

    { (yyval.t) = (yyvsp[-1].t); }

    break;

  case 169:

    { (yyval.t) = newCTerm(PA_fNoElse,pos()); }

    break;

  case 170:

    { checkDeprecation((yyvsp[-3].t));
		    (yyval.t) = newCTerm(PA_fBoolCase,(yyvsp[-5].t),(yyvsp[-2].t),(yyvsp[-1].t),makeLongPos((yyvsp[-6].t),(yyvsp[0].t)));
		  }

    break;

  case 171:

    { (yyval.t) = newCTerm(PA_fCase,(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[-1].t),makeLongPos((yyvsp[-5].t),(yyvsp[0].t))); }

    break;

  case 172:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 173:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 174:

    { (yyval.t) = (yyvsp[-1].t); }

    break;

  case 175:

    { (yyval.t) = newCTerm(PA_fNoElse,pos()); }

    break;

  case 176:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 177:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 178:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 179:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 180:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 181:

    { (yyval.t) = newCTerm(PA_fCaseClause,(yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 182:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 183:

    { (yyval.t) = newCTerm(PA_fSideCondition,(yyvsp[-3].t),
				  newCTerm(PA_fSkip,(yyvsp[-1].t)),(yyvsp[0].t),(yyvsp[-1].t)); }

    break;

  case 184:

    { (yyval.t) = newCTerm(PA_fSideCondition,(yyvsp[-5].t),(yyvsp[-2].t),(yyvsp[0].t),(yyvsp[-3].t)); }

    break;

  case 185:

    { (yyval.t) = newCTerm(PA_fEq,(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }

    break;

  case 186:

    { (yyval.t) = makeCons((yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }

    break;

  case 187:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 188:

    { (yyval.t) = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,(yyvsp[-1].t)),
				  oz_consUnwrap((yyvsp[-3].t),(yyvsp[0].t))); }

    break;

  case 189:

    { (yyval.t) = newCTerm(PA_fClass,(yyvsp[-4].t),(yyvsp[-3].t),(yyvsp[-2].t),makeLongPos((yyvsp[-5].t),(yyvsp[0].t))); }

    break;

  case 190:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 191:

    { (yyval.t) = newCTerm(PA_fDollar,(yyvsp[0].t)); }

    break;

  case 192:

    { (yyval.t) = AtomNil; }

    break;

  case 193:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 194:

    { (yyval.t) = newCTerm(PA_fFrom,oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)),(yyvsp[-2].t)); }

    break;

  case 195:

    { (yyval.t) = newCTerm(PA_fAttr,oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)),(yyvsp[-2].t)); }

    break;

  case 196:

    { (yyval.t) = newCTerm(PA_fFeat,oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)),(yyvsp[-2].t)); }

    break;

  case 197:

    { (yyval.t) = newCTerm(PA_fProp,oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)),(yyvsp[-2].t)); }

    break;

  case 198:

    { (yyval.t) = AtomNil; }

    break;

  case 199:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 200:

    { (yyval.t) = oz_pair2Unwrap((yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 201:

    { (yyval.t) = (yyvsp[0].t); }

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

    { (yyval.t) = newCTerm(PA_fAtom,NameUnit,pos()); }

    break;

  case 206:

    { (yyval.t) = newCTerm(PA_fAtom,NameTrue,pos()); }

    break;

  case 207:

    { (yyval.t) = newCTerm(PA_fAtom,NameFalse,pos()); }

    break;

  case 208:

    { (yyval.t) = AtomNil; }

    break;

  case 209:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 210:

    { (yyval.t) = newCTerm(PA_fMeth,(yyvsp[-2].t),(yyvsp[-1].t),(yyvsp[-3].t)); }

    break;

  case 211:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 212:

    { (yyval.t) = newCTerm(PA_fEq,(yyvsp[-3].t),(yyvsp[0].t),(yyvsp[-1].t)); }

    break;

  case 213:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 214:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 215:

    { (yyval.t) = newCTerm(PA_fAtom,NameUnit,pos()); }

    break;

  case 216:

    { (yyval.t) = newCTerm(PA_fAtom,NameTrue,pos()); }

    break;

  case 217:

    { (yyval.t) = newCTerm(PA_fAtom,NameFalse,pos()); }

    break;

  case 218:

    { (yyval.t) = newCTerm(PA_fRecord,(yyvsp[-3].t),(yyvsp[-1].t)); }

    break;

  case 219:

    { (yyval.t) = newCTerm(PA_fOpenRecord,(yyvsp[-4].t),(yyvsp[-2].t)); }

    break;

  case 220:

    { (yyval.t) = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); }

    break;

  case 221:

    { (yyval.t) = makeVar(xytext); }

    break;

  case 222:

    { (yyval.t) = newCTerm(PA_fEscape,makeVar(xytext),(yyvsp[-1].t)); }

    break;

  case 223:

    { (yyval.t) = newCTerm(PA_fAtom,NameUnit,pos()); }

    break;

  case 224:

    { (yyval.t) = newCTerm(PA_fAtom,NameTrue,pos()); }

    break;

  case 225:

    { (yyval.t) = newCTerm(PA_fAtom,NameFalse,pos()); }

    break;

  case 226:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 227:

    { (yyval.t) = AtomNil; }

    break;

  case 228:

    { (yyval.t) = newCTerm(PA_fMethArg,(yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 229:

    { (yyval.t) = newCTerm(PA_fMethColonArg,(yyvsp[-3].t),(yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 230:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 231:

    { (yyval.t) = newCTerm(PA_fDollar,pos()); }

    break;

  case 232:

    { (yyval.t) = newCTerm(PA_fWildcard,pos()); }

    break;

  case 233:

    { (yyval.t) = newCTerm(PA_fDefault,(yyvsp[0].t),(yyvsp[-1].t)); }

    break;

  case 234:

    { (yyval.t) = PA_fNoDefault; }

    break;

  case 235:

    { (yyval.t) = newCTerm(PA_fCond,(yyvsp[-3].t),(yyvsp[-2].t),makeLongPos((yyvsp[-4].t),(yyvsp[0].t))); }

    break;

  case 236:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 237:

    { (yyval.t) = newCTerm(PA_fNoElse,pos()); }

    break;

  case 238:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 239:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 240:

    { (yyval.t) = newCTerm(PA_fClause,newCTerm(PA_fSkip,(yyvsp[-1].t)),(yyvsp[-3].t),(yyvsp[0].t)); }

    break;

  case 241:

    { (yyval.t) = newCTerm(PA_fClause,(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 242:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 243:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 244:

    { (yyval.t) = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,(yyvsp[0].t)),
				  (yyvsp[-1].t),newCTerm(PA_fNoThen,(yyvsp[0].t))); }

    break;

  case 245:

    { (yyval.t) = newCTerm(PA_fClause,(yyvsp[-3].t),(yyvsp[-1].t),newCTerm(PA_fNoThen,(yyvsp[0].t))); }

    break;

  case 246:

    { (yyval.t) = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,(yyvsp[-2].t)),(yyvsp[-3].t),(yyvsp[0].t)); }

    break;

  case 247:

    { (yyval.t) = newCTerm(PA_fClause,(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 248:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 249:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 250:

    { (yyval.t) = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); }

    break;

  case 251:

    { (yyval.t) = makeVar(xytext); }

    break;

  case 252:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 253:

    { (yyval.t) = newCTerm(PA_fEscape,(yyvsp[0].t),(yyvsp[-1].t)); }

    break;

  case 254:

    { (yyval.t) = makeString(xytext,pos()); }

    break;

  case 255:

    { (yyval.t) = makeInt(xytext,pos()); }

    break;

  case 256:

    { (yyval.t) = makeInt(xytext[0],pos()); }

    break;

  case 257:

    { (yyval.t) = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); }

    break;

  case 258:

    { (yyval.t) = pos(); }

    break;

  case 259:

    { (yyval.t) = pos(); }

    break;

  case 260:

    { OZ_Term prefix =
		      scannerPrefix? scannerPrefix: PA_zy;
		    (yyval.t) = newCTerm(PA_fScanner,(yyvsp[-5].t),(yyvsp[-4].t),(yyvsp[-3].t),(yyvsp[-2].t),prefix,
				  makeLongPos((yyvsp[-6].t),(yyvsp[0].t))); }

    break;

  case 261:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 262:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 263:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 264:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 265:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 266:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 267:

    { (yyval.t) = newCTerm(PA_fLexicalAbbreviation,(yyvsp[-3].t),(yyvsp[-1].t)); }

    break;

  case 268:

    { (yyval.t) = newCTerm(PA_fLexicalAbbreviation,(yyvsp[-3].t),(yyvsp[-1].t)); }

    break;

  case 269:

    { (yyval.t) = newCTerm(PA_fLexicalRule,(yyvsp[-2].t),(yyvsp[-1].t)); }

    break;

  case 270:

    { (yyval.t) = OZ_string(xytext); }

    break;

  case 271:

    { (yyval.t) = OZ_string(xytext); }

    break;

  case 272:

    { (yyval.t) = newCTerm(PA_fMode,(yyvsp[-2].t),(yyvsp[-1].t)); }

    break;

  case 273:

    { (yyval.t) = AtomNil; }

    break;

  case 274:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 275:

    { (yyval.t) = newCTerm(PA_fInheritedModes,(yyvsp[0].t)); }

    break;

  case 276:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 277:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 278:

    { OZ_Term expect = parserExpect? parserExpect: makeTaggedSmallInt(0);
		    (yyval.t) = newCTerm(PA_fParser,(yyvsp[-6].t),(yyvsp[-5].t),(yyvsp[-4].t),(yyvsp[-3].t),(yyvsp[-2].t),expect,
				  makeLongPos((yyvsp[-7].t),(yyvsp[0].t))); }

    break;

  case 279:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 280:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 281:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 282:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 283:

    { (yyval.t) = newCTerm(PA_fToken,AtomNil); }

    break;

  case 284:

    { (yyval.t) = newCTerm(PA_fToken,(yyvsp[0].t)); }

    break;

  case 285:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 286:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 287:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 288:

    { (yyval.t) = oz_pair2Unwrap((yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 289:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 290:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 291:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 292:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 293:

    { *prodKey[depth]++ = '='; }

    break;

  case 294:

    { (yyval.t) = newCTerm(PA_fProductionTemplate,(yyvsp[-3].t),(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[-1].t),(yyvsp[-7].t)); }

    break;

  case 295:

    { (yyval.t) = newCTerm(PA_fDollar,pos()); }

    break;

  case 296:

    { *prodKey[depth]++ = '='; }

    break;

  case 297:

    { (yyval.t) = newCTerm(PA_fProductionTemplate,(yyvsp[-3].t),(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[-1].t),(yyvsp[-7].t)); }

    break;

  case 298:

    { (yyval.t) = newCTerm(PA_fProductionTemplate,(yyvsp[-3].t),(yyvsp[-4].t),(yyvsp[-2].t),(yyvsp[-1].t),PA_none); }

    break;

  case 299:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[-1].t)); }

    break;

  case 300:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[-1].t)); }

    break;

  case 301:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 304:

    { prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg((yyvsp[-1].t),0))); }

    break;

  case 305:

    { *prodKey[depth]++ = '('; depth++; }

    break;

  case 306:

    { depth--; }

    break;

  case 307:

    { (yyval.t) = (yyvsp[-3].t); }

    break;

  case 308:

    { *prodKey[depth]++ = '['; depth++; }

    break;

  case 309:

    { depth--; }

    break;

  case 310:

    { (yyval.t) = (yyvsp[-3].t); }

    break;

  case 311:

    { *prodKey[depth]++ = '{'; depth++; }

    break;

  case 312:

    { depth--; }

    break;

  case 313:

    { (yyval.t) = (yyvsp[-3].t); }

    break;

  case 314:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 315:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 316:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 317:

    { (yyval.t) = newCTerm(PA_fWildcard,pos()); }

    break;

  case 318:

    { *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; }

    break;

  case 321:

    { *prodKey[depth]++ = xytext[0]; }

    break;

  case 322:

    { *prodKey[depth]++ = xytext[0]; }

    break;

  case 323:

    { *prodKey[depth] = '\0';
		    (yyval.t) = oz_pair2Unwrap(prodName[depth],OZ_string(prodKeyBuffer[depth]));
		    prodName[depth] = PA_none;
		    prodKey[depth] = prodKeyBuffer[depth];
		  }

    break;

  case 324:

    { (yyval.t) = AtomNil; }

    break;

  case 325:

    { (yyval.t) = (yyvsp[-1].t); }

    break;

  case 326:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 327:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 328:

    { (yyval.t) = newCTerm(PA_fSyntaxRule,(yyvsp[-2].t),AtomNil,(yyvsp[-1].t)); }

    break;

  case 329:

    { (yyval.t) = newCTerm(PA_fSyntaxRule,(yyvsp[-2].t),AtomNil,(yyvsp[-1].t)); }

    break;

  case 330:

    { (yyval.t) = newCTerm(PA_fSyntaxRule,(yyvsp[-5].t),(yyvsp[-3].t),(yyvsp[-1].t)); }

    break;

  case 331:

    { (yyval.t) = AtomNil; }

    break;

  case 332:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 333:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 334:

    { (yyval.t) = newCTerm(PA_fDollar,pos()); }

    break;

  case 335:

    { (yyval.t) = newCTerm(PA_fWildcard,pos()); }

    break;

  case 336:

    { (yyval.t) = newCTerm(PA_fSynAlternative, (yyvsp[0].t)); }

    break;

  case 337:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 338:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 339:

    { OZ_Term t = (yyvsp[0].t);
		    while (terms[depth]) {
		      t = oz_consUnwrap(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    (yyval.t) = newCTerm(PA_fSynSequence, decls[depth], t, (yyvsp[-1].t));
		    decls[depth] = AtomNil;
		  }

    break;

  case 340:

    { (yyval.t) = newCTerm(PA_fSynSequence, AtomNil, (yyvsp[0].t), (yyvsp[-1].t)); }

    break;

  case 341:

    { (yyval.t) = AtomNil; }

    break;

  case 342:

    { (yyval.t) = oz_mklistUnwrap(newCTerm(PA_fSynAction,(yyvsp[0].t))); }

    break;

  case 343:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 344:

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

  case 345:

    { (yyval.t) = oz_consUnwrap(newCTerm(PA_fSynAssignment, terms[depth]->term, (yyvsp[-1].t)),
				  (yyvsp[0].t));
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  }

    break;

  case 346:

    { while (terms[depth]) {
		      decls[depth] = oz_consUnwrap(terms[depth]->term, decls[depth]);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    (yyval.t) = (yyvsp[0].t);
		  }

    break;

  case 347:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 348:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 349:

    { terms[depth] = new TermNode((yyvsp[0].t), terms[depth]); }

    break;

  case 350:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 351:

    { (yyval.t) = oz_consUnwrap((yyvsp[-1].t),(yyvsp[0].t)); }

    break;

  case 352:

    { (yyval.t) = newCTerm(PA_fSynAssignment,(yyvsp[-2].t),(yyvsp[0].t)); }

    break;

  case 353:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 354:

    { (yyval.t) = newCTerm(PA_fSynApplication,(yyvsp[0].t),AtomNil); }

    break;

  case 355:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[0].t),
				  oz_consUnwrap(newCTerm(PA_fSynApplication,(yyvsp[-3].t),
						    AtomNil),
					   AtomNil),(yyvsp[-1].t));
		  }

    break;

  case 356:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 357:

    { (yyval.t) = newCTerm(PA_fSynAssignment,
				  newCTerm(PA_fEscape,(yyvsp[-2].t),(yyvsp[-3].t)),(yyvsp[0].t)); }

    break;

  case 358:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 359:

    { (yyval.t) = (yyvsp[0].t); }

    break;

  case 360:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[0].t),
				  oz_mklistUnwrap((yyvsp[-2].t)),(yyvsp[-3].t));
		  }

    break;

  case 361:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[0].t),
				  oz_mklistUnwrap((yyvsp[-3].t)),(yyvsp[-1].t));
		  }

    break;

  case 362:

    { *prodKey[depth]++ = '('; depth++; }

    break;

  case 363:

    { depth--; }

    break;

  case 364:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[0].t),(yyvsp[-4].t),(yyvsp[-7].t)); }

    break;

  case 365:

    { *prodKey[depth]++ = '['; depth++; }

    break;

  case 366:

    { depth--; }

    break;

  case 367:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[0].t),(yyvsp[-4].t),(yyvsp[-7].t)); }

    break;

  case 368:

    { *prodKey[depth]++ = '{'; depth++; }

    break;

  case 369:

    { depth--; }

    break;

  case 370:

    { (yyval.t) = newCTerm(PA_fSynTemplateInstantiation,(yyvsp[0].t),(yyvsp[-4].t),(yyvsp[-7].t)); }

    break;

  case 371:

    { (yyval.t) = newCTerm(PA_fSynApplication,(yyvsp[0].t),AtomNil); }

    break;

  case 372:

    { (yyval.t) = newCTerm(PA_fSynApplication,(yyvsp[-4].t),(yyvsp[-1].t)); }

    break;

  case 373:

    { (yyval.t) = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); }

    break;

  case 374:

    { (yyval.t) = makeVar(xytext); }

    break;

  case 375:

    { (yyval.t) = oz_mklistUnwrap((yyvsp[0].t)); }

    break;

  case 376:

    { (yyval.t) = oz_consUnwrap((yyvsp[-2].t),(yyvsp[0].t)); }

    break;



      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
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

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
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

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
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
  return yyresult;
}



void checkDeprecation(OZ_Term coord) {
  char const *msg = "use `if' instead of `case' for boolean conditionals";
  if (xy_allowDeprecated) {
    xyreportWarning("deprecation warning",msg,coord);
  } else {
    xyreportError("deprecation error",msg,coord);
  }
}

void xyreportWarning(char const *kind, char const *msg, OZ_Term coord) {
  OZ_Term args = oz_mklist(oz_pair2(PA_coord, coord),
			   oz_pair2(PA_kind,  OZ_atom(kind)),
			   oz_pair2(PA_msg,   OZ_atom(msg)));
  xy_errorMessages = OZ_cons(OZ_recordInit(PA_warn,args),
			     xy_errorMessages);
}

void xyreportError(char const *kind, char const *msg, OZ_Term coord) {
  OZ_Term args = oz_mklist(oz_pair2(PA_coord, coord),
			   oz_pair2(PA_kind,  OZ_atom(kind)),
			   oz_pair2(PA_msg,   OZ_atom(msg)));
  xy_errorMessages = OZ_cons(OZ_recordInit(PA_error,args),
			     xy_errorMessages);
}

void xyreportError(char const *kind, char const *msg, const char *file,
		   int line, int column) {
  xyreportError(kind,msg,OZ_mkTupleC("pos",3,OZ_atom((char*)file),
				     oz_int(line),oz_int(column)));
}

static void xyerror(char const *s) {
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
