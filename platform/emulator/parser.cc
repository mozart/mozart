
/*  A Bison parser, made from /home/denys/mozart/platform/emulator/parser.yy
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse xyparse
#define yylex xylex
#define yyerror xyerror
#define yylval xylval
#define yychar xychar
#define yydebug xydebug
#define yynerrs xynerrs
#define	T_SWITCH	257
#define	T_SWITCHNAME	258
#define	T_LOCALSWITCHES	259
#define	T_PUSHSWITCHES	260
#define	T_POPSWITCHES	261
#define	T_OZATOM	262
#define	T_ATOM_LABEL	263
#define	T_OZFLOAT	264
#define	T_OZINT	265
#define	T_AMPER	266
#define	T_DOTINT	267
#define	T_STRING	268
#define	T_VARIABLE	269
#define	T_VARIABLE_LABEL	270
#define	T_DEFAULT	271
#define	T_CHOICE	272
#define	T_LDOTS	273
#define	T_2DOTS	274
#define	T_attr	275
#define	T_at	276
#define	T_case	277
#define	T_catch	278
#define	T_choice	279
#define	T_class	280
#define	T_cond	281
#define	T_declare	282
#define	T_define	283
#define	T_dis	284
#define	T_else	285
#define	T_elsecase	286
#define	T_elseif	287
#define	T_elseof	288
#define	T_end	289
#define	T_export	290
#define	T_fail	291
#define	T_false	292
#define	T_FALSE_LABEL	293
#define	T_feat	294
#define	T_finally	295
#define	T_from	296
#define	T_fun	297
#define	T_functor	298
#define	T_if	299
#define	T_import	300
#define	T_in	301
#define	T_local	302
#define	T_lock	303
#define	T_meth	304
#define	T_not	305
#define	T_of	306
#define	T_or	307
#define	T_prepare	308
#define	T_proc	309
#define	T_prop	310
#define	T_raise	311
#define	T_require	312
#define	T_self	313
#define	T_skip	314
#define	T_then	315
#define	T_thread	316
#define	T_true	317
#define	T_TRUE_LABEL	318
#define	T_try	319
#define	T_unit	320
#define	T_UNIT_LABEL	321
#define	T_for	322
#define	T_FOR	323
#define	T_do	324
#define	T_ENDOFFILE	325
#define	T_REGEX	326
#define	T_lex	327
#define	T_mode	328
#define	T_parser	329
#define	T_prod	330
#define	T_scanner	331
#define	T_syn	332
#define	T_token	333
#define	T_REDUCE	334
#define	T_SEP	335
#define	T_ITER	336
#define	T_OOASSIGN	337
#define	T_DOTASSIGN	338
#define	T_orelse	339
#define	T_andthen	340
#define	T_COMPARE	341
#define	T_FDCOMPARE	342
#define	T_LMACRO	343
#define	T_RMACRO	344
#define	T_FDIN	345
#define	T_ADD	346
#define	T_FDMUL	347
#define	T_OTHERMUL	348
#define	T_DEREFF	349


//
// See Oz/tools/compiler/Doc/TupleSyntax for an description of the
// generated parse trees.
//

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

OZ_Term _PA_AtomTab[110];

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
};

void parser_init(void) {
   for (int i = 110; i--; )
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
OZ_Term newCTerm(OZ_Term l, OZ_Term t1) {
  SRecord * t = SRecord::newSRecord(l, 1);
  t->setArg(0, t1);
  return makeTaggedSRecord(t);
}

inline
OZ_Term newCTerm(OZ_Term l, OZ_Term t1, OZ_Term t2) {
  SRecord * t = SRecord::newSRecord(l, 2);
  t->setArg(0, t1);
  t->setArg(1, t2);
  return makeTaggedSRecord(t);
}

inline
OZ_Term newCTerm(OZ_Term l, OZ_Term t1, OZ_Term t2, OZ_Term t3) {
  SRecord * t = SRecord::newSRecord(l, 3);
  t->setArg(0, t1);
  t->setArg(1, t2);
  t->setArg(2, t3);
  return makeTaggedSRecord(t);
}

inline
OZ_Term newCTerm(OZ_Term l, OZ_Term t1, OZ_Term t2, OZ_Term t3, OZ_Term t4) {
  SRecord * t = SRecord::newSRecord(l, 4);
  t->setArg(0, t1);
  t->setArg(1, t2);
  t->setArg(2, t3);
  t->setArg(3, t4);
  return makeTaggedSRecord(t);
}

inline
OZ_Term newCTerm(OZ_Term l, OZ_Term t1, OZ_Term t2, OZ_Term t3, OZ_Term t4, OZ_Term t5) {
  SRecord * t = SRecord::newSRecord(l, 5);
  t->setArg(0, t1);
  t->setArg(1, t2);
  t->setArg(2, t3);
  t->setArg(3, t4);
  t->setArg(4, t5);
  return makeTaggedSRecord(t);
}

inline
OZ_Term newCTerm(OZ_Term l, OZ_Term t1, OZ_Term t2, OZ_Term t3, OZ_Term t4, OZ_Term t5, OZ_Term t6) {
  SRecord * t = SRecord::newSRecord(l, 6);
  t->setArg(0, t1);
  t->setArg(1, t2);
  t->setArg(2, t3);
  t->setArg(3, t4);
  t->setArg(4, t5);
  t->setArg(5, t6);
  return makeTaggedSRecord(t);
}

inline
OZ_Term newCTerm(OZ_Term l, OZ_Term t1, OZ_Term t2, OZ_Term t3, OZ_Term t4, OZ_Term t5, OZ_Term t6, OZ_Term t7) {
  SRecord * t = SRecord::newSRecord(l, 7);
  t->setArg(0, t1);
  t->setArg(1, t2);
  t->setArg(2, t3);
  t->setArg(3, t4);
  t->setArg(4, t5);
  t->setArg(5, t6);
  t->setArg(6, t7);
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
OZ_Term makeCons(OZ_Term first, OZ_Term second, OZ_Term pos) {
  SRecord * t1 = SRecord::newSRecord(PA_fRecord, 2);
  SRecord * t2 = SRecord::newSRecord(PA_fAtom,   2);

  t2->setArg(0, AtomCons);
  t2->setArg(1, pos);
  
  t1->setArg(0, makeTaggedSRecord(t2));
  t1->setArg(1, oz_mklist(first,second));

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


typedef union {
  OZ_Term t;
  int i;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		808
#define	YYFLAG		-32768
#define	YYNTBASE	117

#define YYTRANSLATE(x) ((unsigned)(x) <= 349 ? yytranslate[x] : 262)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   116,     2,    94,   109,     2,     2,     2,   106,
   107,     2,   104,    98,   105,   100,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,   114,   115,     2,
    83,     2,     2,   102,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   110,     2,   111,   101,   108,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   112,    93,   113,    99,     2,     2,     2,     2,
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
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
    57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
    67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
    77,    78,    79,    80,    81,    82,    84,    85,    86,    87,
    88,    89,    90,    91,    92,    95,    96,    97,   103
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     5,     8,    11,    13,    17,    20,    27,    34,
    40,    41,    44,    46,    48,    50,    51,    54,    57,    60,
    62,    65,    70,    75,    80,    85,    90,    95,   100,   105,
   110,   112,   117,   119,   123,   128,   133,   138,   143,   147,
   152,   155,   160,   164,   168,   172,   174,   176,   178,   180,
   182,   184,   186,   188,   190,   192,   194,   196,   203,   210,
   221,   232,   239,   241,   248,   251,   254,   260,   268,   274,
   282,   288,   290,   292,   298,   301,   307,   313,   319,   321,
   323,   329,   337,   345,   346,   349,   353,   357,   359,   364,
   368,   374,   375,   378,   381,   382,   385,   388,   390,   399,
   406,   407,   410,   411,   414,   415,   417,   422,   429,   434,
   439,   444,   451,   456,   457,   461,   468,   471,   473,   477,
   480,   485,   486,   489,   490,   493,   498,   500,   502,   504,
   506,   508,   510,   515,   517,   518,   521,   523,   527,   528,
   532,   533,   536,   544,   552,   554,   556,   558,   560,   562,
   563,   566,   571,   572,   574,   576,   578,   580,   582,   584,
   586,   588,   590,   597,   600,   603,   607,   609,   617,   624,
   627,   630,   634,   636,   638,   642,   646,   648,   652,   656,
   658,   663,   670,   675,   680,   682,   687,   695,   697,   699,
   700,   703,   708,   713,   718,   723,   724,   727,   731,   733,
   735,   737,   739,   741,   743,   745,   746,   749,   755,   757,
   762,   764,   766,   768,   770,   772,   777,   783,   785,   787,
   791,   793,   795,   797,   800,   801,   804,   809,   811,   813,
   815,   819,   820,   826,   829,   830,   832,   836,   841,   847,
   851,   855,   858,   863,   868,   874,   876,   880,   882,   884,
   886,   890,   892,   894,   896,   898,   899,   900,   909,   911,
   913,   915,   918,   921,   924,   930,   936,   941,   943,   945,
   950,   951,   954,   957,   959,   961,   971,   973,   975,   978,
   981,   982,   985,   987,   990,   992,   996,   998,  1001,  1003,
  1006,  1007,  1017,  1018,  1019,  1030,  1037,  1041,  1044,  1047,
  1049,  1050,  1053,  1054,  1055,  1062,  1063,  1064,  1071,  1072,
  1073,  1080,  1082,  1086,  1088,  1090,  1092,  1093,  1095,  1097,
  1099,  1100,  1101,  1104,  1106,  1109,  1114,  1119,  1127,  1128,
  1131,  1133,  1135,  1137,  1139,  1141,  1145,  1148,  1152,  1153,
  1156,  1159,  1165,  1170,  1173,  1176,  1178,  1180,  1182,  1185,
  1189,  1191,  1193,  1198,  1200,  1206,  1208,  1210,  1216,  1221,
  1222,  1223,  1233,  1234,  1235,  1245,  1246,  1247,  1257,  1259,
  1265,  1267,  1269,  1271
};

static const short yyrhs[] = {   118,
    71,     0,     1,     0,   123,   119,     0,   216,   119,     0,
   119,     0,   200,   138,   119,     0,   120,   118,     0,    28,
   201,   123,    47,   200,   119,     0,    28,   201,   123,    47,
   123,   119,     0,    28,   201,   123,   200,   119,     0,     0,
     3,   121,     0,     5,     0,     6,     0,     7,     0,     0,
   122,   121,     0,   104,     4,     0,   105,     4,     0,   124,
     0,   124,   123,     0,   124,    83,   201,   124,     0,   124,
    85,   201,   124,     0,   124,    84,   201,   124,     0,   124,
    86,   201,   124,     0,   124,    87,   201,   124,     0,   124,
   144,   201,   124,     0,   124,   145,   201,   124,     0,   124,
   146,   201,   124,     0,   124,    93,   201,   124,     0,   126,
     0,   126,    94,   201,   125,     0,   126,     0,   126,    94,
   125,     0,   126,   147,   201,   126,     0,   126,   148,   201,
   126,     0,   126,   149,   201,   126,     0,   126,    98,   201,
   126,     0,    99,   201,   126,     0,   126,   100,   201,   126,
     0,   126,    13,     0,   126,   101,   201,   126,     0,   102,
   201,   126,     0,   103,   201,   126,     0,   106,   150,   107,
     0,   194,     0,   196,     0,   108,     0,    66,     0,    63,
     0,    38,     0,    59,     0,   109,     0,   197,     0,   198,
     0,   199,     0,   155,     0,   110,   201,   124,   152,   111,
   201,     0,   112,   201,   124,   151,   113,   201,     0,    55,
   201,   136,   112,   124,   151,   113,   150,    35,   201,     0,
    43,   201,   136,   112,   124,   151,   113,   150,    35,   201,
     0,    44,   201,   172,   137,    35,   201,     0,   171,     0,
    48,   201,   123,    47,   123,    35,     0,    45,   162,     0,
    23,   164,     0,    49,   201,   150,    35,   201,     0,    49,
   201,   124,    61,   150,    35,   201,     0,    62,   201,   150,
    35,   201,     0,    65,   201,   150,   153,   154,    35,   201,
     0,    57,   201,   150,    35,   201,     0,    60,     0,    37,
     0,    51,   201,   150,    35,   201,     0,    27,   187,     0,
    53,   201,   191,    35,   201,     0,    30,   201,   191,    35,
   201,     0,    25,   201,   193,    35,   201,     0,   202,     0,
   210,     0,    90,   201,   151,    91,   201,     0,    68,   201,
   133,    70,   150,    35,   201,     0,    69,   201,   127,    70,
   150,    35,   201,     0,     0,   128,   127,     0,   194,   114,
   124,     0,   124,    47,   129,     0,   124,     0,   124,    20,
   124,   130,     0,   124,   115,   131,     0,   106,   124,   115,
   131,   107,     0,     0,   115,   124,     0,   124,   132,     0,
     0,   115,   124,     0,   133,   134,     0,   134,     0,   195,
    47,   201,   124,    20,   124,   135,   201,     0,   195,    47,
   201,   124,   135,   201,     0,     0,   115,   124,     0,     0,
   194,   136,     0,     0,   138,     0,    58,   201,   139,   137,
     0,    54,   201,   123,    47,   123,   137,     0,    54,   201,
   123,   137,     0,    46,   201,   139,   137,     0,    36,   201,
   143,   137,     0,    29,   201,   123,    47,   123,   137,     0,
    29,   201,   123,   137,     0,     0,   195,   142,   139,     0,
   140,   106,   141,   107,   142,   139,     0,    16,   201,     0,
   161,     0,   161,   114,   195,     0,   161,   141,     0,   161,
   114,   195,   141,     0,     0,    22,   194,     0,     0,   195,
   143,     0,   161,   114,   195,   143,     0,    88,     0,    89,
     0,    92,     0,    95,     0,    96,     0,    97,     0,   123,
    47,   201,   123,     0,   123,     0,     0,   124,   151,     0,
   200,     0,   200,   124,   152,     0,     0,    24,   201,   167,
     0,     0,    41,   150,     0,   156,   201,   106,   158,   159,
   107,   201,     0,   157,   201,   106,   158,   159,   107,   201,
     0,     9,     0,    67,     0,    64,     0,    39,     0,    16,
     0,     0,   124,   158,     0,   160,   114,   124,   158,     0,
     0,    19,     0,   194,     0,   195,     0,   198,     0,    66,
     0,    63,     0,    38,     0,   194,     0,   198,     0,   201,
   123,    61,   150,   163,   201,     0,    33,   162,     0,    32,
   164,     0,    31,   150,    35,     0,    35,     0,   201,   123,
    61,   201,   150,   165,   201,     0,   201,   123,    52,   166,
   165,   201,     0,    33,   162,     0,    32,   164,     0,    31,
   150,    35,     0,    35,     0,   168,     0,   168,    18,   166,
     0,   168,    34,   166,     0,   168,     0,   168,    18,   167,
     0,   169,    61,   150,     0,   170,     0,   170,    87,   200,
   123,     0,   170,    87,   200,   123,    47,   123,     0,   170,
    83,   201,   170,     0,   170,    93,   201,   170,     0,   126,
     0,   126,    94,   201,   125,     0,    26,   201,   172,   173,
   178,    35,   201,     0,   124,     0,   200,     0,     0,   174,
   173,     0,    42,   201,   124,   151,     0,    21,   201,   176,
   175,     0,    40,   201,   176,   175,     0,    56,   201,   124,
   151,     0,     0,   176,   175,     0,   177,   114,   124,     0,
   177,     0,   194,     0,   196,     0,   198,     0,    66,     0,
    63,     0,    38,     0,     0,   179,   178,     0,    50,   201,
   180,   150,    35,     0,   181,     0,   181,    83,   201,   195,
     0,   194,     0,   196,     0,    66,     0,    63,     0,    38,
     0,   182,   106,   183,   107,     0,   182,   106,   183,    19,
   107,     0,     9,     0,    16,     0,   116,   201,    16,     0,
    67,     0,    64,     0,    39,     0,   184,   183,     0,     0,
   185,   186,     0,   160,   114,   185,   186,     0,   195,     0,
   109,     0,   108,     0,    17,   201,   124,     0,     0,   201,
   189,   188,    35,   201,     0,    31,   150,     0,     0,   190,
     0,   190,    18,   189,     0,   123,    61,   201,   150,     0,
   123,    47,   123,    61,   150,     0,   192,    18,   192,     0,
   192,    18,   191,     0,   123,   200,     0,   123,    47,   123,
   200,     0,   123,   200,    61,   150,     0,   123,    47,   123,
    61,   150,     0,   150,     0,   150,    18,   193,     0,     8,
     0,    15,     0,   195,     0,   116,   201,   195,     0,    14,
     0,    11,     0,    12,     0,    10,     0,     0,     0,    77,
   201,   195,   173,   178,   203,    35,   201,     0,   204,     0,
   205,     0,   207,     0,   204,   203,     0,   205,   203,     0,
   207,   203,     0,    73,   194,    83,   206,    35,     0,    73,
   195,    83,   206,    35,     0,    73,   206,   150,    35,     0,
    72,     0,    14,     0,    74,   195,   208,    35,     0,     0,
   209,   208,     0,    42,   215,     0,   205,     0,   207,     0,
    75,   201,   195,   173,   178,   212,   211,    35,   201,     0,
   239,     0,   217,     0,   239,   211,     0,   217,   211,     0,
     0,    79,   213,     0,   214,     0,   214,   213,     0,   194,
     0,   194,   114,   124,     0,   195,     0,   195,   215,     0,
   217,     0,   217,   216,     0,     0,    76,   195,    83,   218,
   221,   236,   237,   242,    35,     0,     0,     0,    76,   109,
   219,    83,   220,   221,   236,   237,   242,    35,     0,    76,
   221,   236,   237,   242,    35,     0,   223,   195,   234,     0,
   195,   235,     0,   222,   224,     0,   223,     0,     0,   194,
   114,     0,     0,     0,   106,   225,   231,   107,   226,   234,
     0,     0,     0,   110,   227,   231,   111,   228,   234,     0,
     0,     0,   112,   229,   231,   113,   230,   234,     0,   232,
     0,   232,   233,   231,     0,   195,     0,   108,     0,    81,
     0,     0,   235,     0,    95,     0,    96,     0,     0,     0,
   238,    47,     0,   239,     0,   239,   238,     0,    78,   194,
   242,    35,     0,    78,   195,   242,    35,     0,    78,   260,
   106,   240,   107,   242,    35,     0,     0,   241,   240,     0,
   195,     0,   109,     0,   108,     0,   243,     0,   244,     0,
   244,    18,   243,     0,   200,   246,     0,    60,   201,   245,
     0,     0,    80,   150,     0,   247,   246,     0,   247,   235,
   201,   248,   236,     0,   247,    83,   250,   248,     0,    47,
   248,     0,   251,   248,     0,   245,     0,   195,     0,   245,
     0,   249,   248,     0,   196,    83,   250,     0,   250,     0,
   195,     0,   195,   235,   201,   236,     0,   252,     0,   116,
   201,   195,    83,   250,     0,   252,     0,   259,     0,   223,
   201,   259,   234,   236,     0,   259,   235,   201,   236,     0,
     0,     0,   222,   201,   106,   253,   261,   107,   254,   234,
   236,     0,     0,     0,   222,   201,   110,   255,   261,   111,
   256,   234,   236,     0,     0,     0,   222,   201,   112,   257,
   261,   113,   258,   234,   236,     0,   194,     0,   260,   201,
   106,   151,   107,     0,     9,     0,    16,     0,   242,     0,
   242,   233,   261,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   687,   689,   693,   695,   698,   700,   705,   707,   710,   712,
   715,   719,   721,   723,   725,   729,   731,   735,   742,   751,
   753,   757,   759,   761,   763,   765,   767,   770,   772,   774,
   776,   778,   784,   786,   790,   793,   796,   799,   801,   804,
   807,   810,   813,   815,   818,   820,   822,   824,   826,   828,
   830,   832,   834,   836,   838,   840,   842,   844,   848,   850,
   853,   856,   858,   860,   862,   864,   866,   868,   870,   872,
   874,   876,   878,   880,   882,   884,   886,   888,   890,   892,
   894,   896,   900,   904,   906,   910,   912,   916,   918,   920,
   922,   926,   928,   932,   936,   938,   942,   946,   949,   963,
   987,   988,   992,   994,   999,  1001,  1006,  1008,  1010,  1013,
  1015,  1017,  1019,  1024,  1026,  1028,  1032,  1036,  1038,  1040,
  1042,  1046,  1048,  1052,  1054,  1056,  1061,  1065,  1069,  1073,
  1077,  1081,  1085,  1087,  1091,  1093,  1097,  1099,  1105,  1107,
  1111,  1113,  1117,  1122,  1129,  1131,  1133,  1135,  1139,  1143,
  1145,  1147,  1151,  1153,  1157,  1159,  1161,  1163,  1165,  1167,
  1171,  1173,  1177,  1181,  1183,  1185,  1187,  1191,  1195,  1199,
  1201,  1203,  1205,  1209,  1211,  1213,  1217,  1219,  1223,  1227,
  1229,  1232,  1236,  1238,  1240,  1242,  1248,  1253,  1255,  1260,
  1262,  1266,  1268,  1270,  1272,  1276,  1278,  1282,  1284,  1288,
  1290,  1292,  1294,  1296,  1298,  1302,  1304,  1308,  1312,  1314,
  1318,  1320,  1322,  1324,  1326,  1328,  1330,  1334,  1336,  1338,
  1340,  1342,  1344,  1348,  1350,  1354,  1356,  1360,  1362,  1364,
  1369,  1371,  1375,  1379,  1381,  1385,  1387,  1391,  1393,  1397,
  1399,  1403,  1407,  1409,  1412,  1416,  1418,  1422,  1426,  1430,
  1432,  1436,  1440,  1442,  1446,  1450,  1454,  1464,  1472,  1474,
  1476,  1478,  1480,  1482,  1486,  1488,  1492,  1496,  1498,  1502,
  1506,  1508,  1512,  1514,  1516,  1522,  1530,  1532,  1534,  1536,
  1540,  1542,  1546,  1548,  1552,  1554,  1558,  1560,  1564,  1566,
  1570,  1572,  1574,  1575,  1576,  1578,  1582,  1584,  1586,  1590,
  1591,  1594,  1598,  1599,  1599,  1600,  1601,  1601,  1602,  1603,
  1603,  1606,  1608,  1612,  1613,  1616,  1620,  1621,  1624,  1625,
  1628,  1636,  1638,  1642,  1644,  1648,  1650,  1652,  1656,  1658,
  1662,  1664,  1666,  1670,  1674,  1676,  1680,  1689,  1693,  1695,
  1699,  1701,  1711,  1716,  1723,  1725,  1729,  1733,  1735,  1739,
  1741,  1745,  1747,  1753,  1757,  1760,  1765,  1767,  1771,  1775,
  1776,  1777,  1779,  1780,  1781,  1783,  1784,  1785,  1789,  1791,
  1795,  1797,  1802,  1804
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","T_SWITCH",
"T_SWITCHNAME","T_LOCALSWITCHES","T_PUSHSWITCHES","T_POPSWITCHES","T_OZATOM",
"T_ATOM_LABEL","T_OZFLOAT","T_OZINT","T_AMPER","T_DOTINT","T_STRING","T_VARIABLE",
"T_VARIABLE_LABEL","T_DEFAULT","T_CHOICE","T_LDOTS","T_2DOTS","T_attr","T_at",
"T_case","T_catch","T_choice","T_class","T_cond","T_declare","T_define","T_dis",
"T_else","T_elsecase","T_elseif","T_elseof","T_end","T_export","T_fail","T_false",
"T_FALSE_LABEL","T_feat","T_finally","T_from","T_fun","T_functor","T_if","T_import",
"T_in","T_local","T_lock","T_meth","T_not","T_of","T_or","T_prepare","T_proc",
"T_prop","T_raise","T_require","T_self","T_skip","T_then","T_thread","T_true",
"T_TRUE_LABEL","T_try","T_unit","T_UNIT_LABEL","T_for","T_FOR","T_do","T_ENDOFFILE",
"T_REGEX","T_lex","T_mode","T_parser","T_prod","T_scanner","T_syn","T_token",
"T_REDUCE","T_SEP","T_ITER","'='","T_OOASSIGN","T_DOTASSIGN","T_orelse","T_andthen",
"T_COMPARE","T_FDCOMPARE","T_LMACRO","T_RMACRO","T_FDIN","'|'","'#'","T_ADD",
"T_FDMUL","T_OTHERMUL","','","'~'","'.'","'^'","'@'","T_DEREFF","'+'","'-'",
"'('","')'","'_'","'$'","'['","']'","'{'","'}'","':'","';'","'!'","file","queries",
"queries1","directive","switchList","switch","sequence","phrase","hashes","phrase2",
"FOR_decls","FOR_decl","FOR_gen","FOR_genOptInt","FOR_genOptC","FOR_genOptC2",
"iterators","iterator","optIteratorStep","procFlags","optFunctorDescriptorList",
"functorDescriptorList","importDecls","variableLabel","featureList","optImportAt",
"exportDecls","compare","fdCompare","fdIn","add","fdMul","otherMul","inSequence",
"phraseList","fixedListArgs","optCatch","optFinally","record","recordAtomLabel",
"recordVarLabel","recordArguments","optDots","feature","featureNoVar","ifMain",
"ifRest","caseMain","caseRest","elseOfList","caseClauseList","caseClause","sideCondition",
"pattern","class","phraseOpt","classDescriptorList","classDescriptor","attrFeatList",
"attrFeat","attrFeatFeature","methList","meth","methHead","methHead1","methHeadLabel",
"methFormals","methFormal","methFormalTerm","methFormalOptDefault","condMain",
"condElse","condClauseList","condClause","orClauseList","orClause","choiceClauseList",
"atom","nakedVariable","variable","string","int","float","thisCoord","coord",
"scannerSpecification","scannerRules","lexAbbrev","lexRule","regex","modeClause",
"modeDescrs","modeDescr","parserSpecification","parserRules","tokenClause","tokenList",
"tokenDecl","modeFromList","prodClauseList","prodClause","@1","@2","@3","prodHeadRest",
"prodName","prodNameAtom","prodKey","@4","@5","@6","@7","@8","@9","prodParams",
"prodParam","separatorOp","optTerminatorOp","terminatorOp","prodMakeKey","localRules",
"localRulesSub","synClause","synParams","synParam","synAlt","synSeqs","synSeq",
"optSynAction","nonEmptySeq","synVariable","synPrims","synPrim","synPrimNoAssign",
"synPrimNoVar","synPrimNoVarNoAssign","@10","@11","@12","@13","@14","@15","synInstTerm",
"synLabel","synProdCallParams", NULL
};
#endif

static const short yyr1[] = {     0,
   117,   117,   118,   118,   118,   118,   119,   119,   119,   119,
   119,   120,   120,   120,   120,   121,   121,   122,   122,   123,
   123,   124,   124,   124,   124,   124,   124,   124,   124,   124,
   124,   124,   125,   125,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   127,   127,   128,   128,   129,   129,   129,
   129,   130,   130,   131,   132,   132,   133,   133,   134,   134,
   135,   135,   136,   136,   137,   137,   138,   138,   138,   138,
   138,   138,   138,   139,   139,   139,   140,   141,   141,   141,
   141,   142,   142,   143,   143,   143,   144,   145,   146,   147,
   148,   149,   150,   150,   151,   151,   152,   152,   153,   153,
   154,   154,   155,   155,   156,   156,   156,   156,   157,   158,
   158,   158,   159,   159,   160,   160,   160,   160,   160,   160,
   161,   161,   162,   163,   163,   163,   163,   164,   164,   165,
   165,   165,   165,   166,   166,   166,   167,   167,   168,   169,
   169,   169,   170,   170,   170,   170,   171,   172,   172,   173,
   173,   174,   174,   174,   174,   175,   175,   176,   176,   177,
   177,   177,   177,   177,   177,   178,   178,   179,   180,   180,
   181,   181,   181,   181,   181,   181,   181,   182,   182,   182,
   182,   182,   182,   183,   183,   184,   184,   185,   185,   185,
   186,   186,   187,   188,   188,   189,   189,   190,   190,   191,
   191,   192,   192,   192,   192,   193,   193,   194,   195,   196,
   196,   197,   198,   198,   199,   200,   201,   202,   203,   203,
   203,   203,   203,   203,   204,   204,   205,   206,   206,   207,
   208,   208,   209,   209,   209,   210,   211,   211,   211,   211,
   212,   212,   213,   213,   214,   214,   215,   215,   216,   216,
   218,   217,   219,   220,   217,   217,   221,   221,   221,   222,
   222,   223,   225,   226,   224,   227,   228,   224,   229,   230,
   224,   231,   231,   232,   232,   233,   234,   234,   235,   235,
   236,   237,   237,   238,   238,   239,   239,   239,   240,   240,
   241,   241,   241,   242,   243,   243,   244,   244,   245,   245,
   246,   246,   246,   246,   246,   246,   247,   248,   248,   249,
   249,   250,   250,   250,   251,   251,   252,   252,   252,   253,
   254,   252,   255,   256,   252,   257,   258,   252,   259,   259,
   260,   260,   261,   261
};

static const short yyr2[] = {     0,
     2,     1,     2,     2,     1,     3,     2,     6,     6,     5,
     0,     2,     1,     1,     1,     0,     2,     2,     2,     1,
     2,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     1,     4,     1,     3,     4,     4,     4,     4,     3,     4,
     2,     4,     3,     3,     3,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     6,     6,    10,
    10,     6,     1,     6,     2,     2,     5,     7,     5,     7,
     5,     1,     1,     5,     2,     5,     5,     5,     1,     1,
     5,     7,     7,     0,     2,     3,     3,     1,     4,     3,
     5,     0,     2,     2,     0,     2,     2,     1,     8,     6,
     0,     2,     0,     2,     0,     1,     4,     6,     4,     4,
     4,     6,     4,     0,     3,     6,     2,     1,     3,     2,
     4,     0,     2,     0,     2,     4,     1,     1,     1,     1,
     1,     1,     4,     1,     0,     2,     1,     3,     0,     3,
     0,     2,     7,     7,     1,     1,     1,     1,     1,     0,
     2,     4,     0,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     6,     2,     2,     3,     1,     7,     6,     2,
     2,     3,     1,     1,     3,     3,     1,     3,     3,     1,
     4,     6,     4,     4,     1,     4,     7,     1,     1,     0,
     2,     4,     4,     4,     4,     0,     2,     3,     1,     1,
     1,     1,     1,     1,     1,     0,     2,     5,     1,     4,
     1,     1,     1,     1,     1,     4,     5,     1,     1,     3,
     1,     1,     1,     2,     0,     2,     4,     1,     1,     1,
     3,     0,     5,     2,     0,     1,     3,     4,     5,     3,
     3,     2,     4,     4,     5,     1,     3,     1,     1,     1,
     3,     1,     1,     1,     1,     0,     0,     8,     1,     1,
     1,     2,     2,     2,     5,     5,     4,     1,     1,     4,
     0,     2,     2,     1,     1,     9,     1,     1,     2,     2,
     0,     2,     1,     2,     1,     3,     1,     2,     1,     2,
     0,     9,     0,     0,    10,     6,     3,     2,     2,     1,
     0,     2,     0,     0,     6,     0,     0,     6,     0,     0,
     6,     1,     3,     1,     1,     1,     0,     1,     1,     1,
     0,     0,     2,     1,     2,     4,     4,     7,     0,     2,
     1,     1,     1,     1,     1,     3,     2,     3,     0,     2,
     2,     5,     4,     2,     2,     1,     1,     1,     2,     3,
     1,     1,     4,     1,     5,     1,     1,     5,     4,     0,
     0,     9,     0,     0,     9,     0,     0,     9,     1,     5,
     1,     1,     1,     3
};

static const short yydefact[] = {     0,
     2,    16,    13,    14,    15,   248,   145,   255,   253,   254,
   252,   249,   149,   257,   257,   257,   257,   257,   257,    73,
    51,   148,   257,   257,   257,   257,   257,   257,   257,   257,
   257,    52,    72,   257,    50,   147,   257,    49,   146,   257,
   257,   257,   301,   257,   257,   257,   257,   257,     0,    48,
    53,   257,   257,   257,     0,     5,   256,    11,    20,    31,
    57,   257,   257,    63,    46,   250,    47,    54,    55,    56,
     0,    79,    80,    11,   289,     0,     0,    12,    16,    66,
     0,     0,   256,    75,     0,     0,     0,   103,   256,    65,
     0,     0,     0,     0,     0,   103,     0,     0,     0,     0,
    84,     0,   293,     0,     0,   321,     0,   300,     0,   135,
     0,     0,     0,   134,     0,     0,     0,     0,     1,     7,
     3,   257,   257,   257,   257,   257,   127,   128,   129,   257,
    21,   257,   257,   257,    41,   257,   130,   131,   132,   257,
   257,   257,   257,   257,   257,     0,     0,   257,   257,   257,
   257,   257,    11,     4,   290,    18,    19,    17,     0,   246,
     0,   188,   190,   189,     0,   235,   236,   256,   256,     0,
     0,     0,   103,   105,     0,     0,    20,     0,     0,     0,
     0,     0,     0,   139,     0,    98,     0,     0,     0,    84,
    46,   190,     0,   302,   291,   319,   320,   298,   322,   303,
   306,   309,   299,   317,   190,   135,     0,    39,    43,    44,
   257,    45,   256,   135,   251,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   150,   150,     0,   124,   114,     0,   114,     6,     0,
   257,     0,   257,   257,   257,   257,   257,   206,   190,     0,
   257,     0,     0,     0,   256,    11,     0,   242,   257,     0,
     0,   104,     0,   106,     0,     0,     0,   257,   257,   257,
     0,   257,   257,   257,   141,     0,    97,   257,     0,     0,
    85,     0,   206,   294,   301,     0,   256,     0,   324,     0,
     0,     0,   297,   318,   206,   136,   257,     0,     0,   137,
     0,    22,    24,    23,    25,    26,    30,    27,    28,    29,
    32,    33,    38,    40,    42,    35,    36,    37,    51,    50,
    49,   150,   153,     0,    46,   250,    55,   153,   105,   105,
     0,   161,   124,   162,   257,   105,     0,   122,   105,   105,
   185,     0,   174,     0,   180,     0,   247,    78,     0,     0,
     0,     0,   257,     0,   206,   191,     0,     0,   234,   257,
   237,    11,    11,    10,   256,     0,    77,   241,   240,   135,
   257,     0,     0,     0,    67,    74,    76,   135,    71,    69,
     0,     0,     0,     0,     0,     0,    88,    87,     0,    86,
   281,   301,     0,   321,   371,   372,   256,   256,     0,   257,
   339,     0,   334,   335,   323,   325,   315,   314,     0,   312,
     0,     0,     0,    81,   133,   257,   256,   257,     0,   151,
   154,     0,     0,     0,     0,   113,   111,     0,   125,   117,
   110,     0,     0,   114,     0,   109,   107,   257,     0,   257,
   257,   173,   257,     0,     0,     0,   257,   256,   257,     0,
   205,   204,   203,   196,   199,   200,   201,   202,   196,   135,
   135,     0,   257,   207,     0,   238,   233,     9,     8,     0,
   243,   244,     0,    62,     0,   257,   257,   167,   257,    64,
   257,     0,   140,   177,   142,   257,   257,   101,    20,     0,
     0,   257,     0,     0,   321,   322,     0,     0,   329,   339,
   339,     0,   257,   369,   347,   257,   257,   346,   337,   339,
   339,   356,   357,   257,   296,   256,   304,   316,     0,   307,
   310,     0,     0,     0,   259,   260,   261,    58,   138,    59,
    34,   257,   150,   257,   105,   124,     0,   118,   123,   115,
   105,     0,     0,   171,   170,   169,   175,   176,   179,     0,
     0,     0,   257,   193,   196,     0,   194,   192,   195,   218,
   219,   215,   223,   214,   222,   213,   221,   257,     0,   209,
     0,   211,   212,   187,   239,   245,     0,     0,   165,   164,
   163,    68,     0,     0,    70,    82,     0,     0,   257,     0,
    92,    95,    90,    83,   285,   282,   283,     0,   278,   277,
   322,   256,   326,   327,   333,   332,   331,     0,   329,   338,
   352,     0,   348,   344,   339,   351,   354,   340,     0,     0,
     0,   301,   257,   341,   345,   257,     0,   336,   317,   313,
   317,   317,   269,   268,     0,     0,     0,   271,   257,   262,
   263,   264,   143,   152,   144,   112,   126,   122,     0,   120,
   108,   186,   172,   183,   181,   184,   168,   197,   198,     0,
     0,   257,   225,     0,   166,     0,   178,   101,   102,   100,
     0,     0,    89,     0,    94,     0,   284,   257,   280,   279,
   256,     0,   256,   330,   257,   301,   349,     0,   360,   363,
   366,   369,   317,   352,   339,   339,   321,   135,   305,   308,
   311,     0,     0,     0,     0,     0,   274,   275,     0,   271,
   258,   114,   119,     0,   220,   208,     0,   160,   159,   158,
   230,   229,     0,     0,   225,   232,   155,   228,   157,   257,
   257,   257,    91,    93,    96,   286,   276,     0,   292,     0,
   321,   350,   301,   256,   256,   256,   321,   343,   321,   359,
     0,     0,     0,   267,   287,   273,   270,   272,   116,   121,
   182,   210,     0,     0,   216,   224,   257,   226,    61,    60,
    99,   295,   328,   353,   355,   373,     0,     0,     0,   358,
   342,   370,   265,   266,   288,   232,   228,   217,     0,   256,
   361,   364,   367,   227,   231,   374,   317,   317,   317,   321,
   321,   321,   362,   365,   368,     0,     0,     0
};

static const short yydefgoto[] = {   806,
    55,    56,    57,    78,    79,   114,    59,   311,    60,   189,
   190,   388,   673,   593,   675,   185,   186,   589,   172,   263,
   264,   336,   337,   537,   434,   330,   132,   133,   134,   143,
   144,   145,   115,   207,   299,   275,   383,    61,    62,    63,
   323,   422,   324,   331,    90,   479,    80,   443,   342,   483,
   343,   344,   345,    64,   163,   248,   249,   554,   555,   455,
   354,   355,   569,   570,   571,   724,   725,   726,   768,    84,
   253,   166,   167,   170,   171,   161,    65,    66,    67,    68,
    69,    70,   401,    81,    72,   524,   525,   526,   637,   527,
   709,   710,    73,   598,   494,   596,   597,   756,    74,    75,
   285,   193,   392,   106,   506,   507,   203,   290,   629,   291,
   631,   292,   632,   409,   410,   519,   293,   294,   199,   287,
   288,   289,   608,   609,   776,   403,   404,   613,   509,   510,
   614,   615,   616,   511,   617,   744,   797,   745,   798,   746,
   799,   513,   514,   777
};

static const short yypact[] = {  1253,
-32768,   186,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,    57,-32768,-32768,-32768,-32768,-32768,  1034,-32768,
-32768,-32768,-32768,-32768,    19,-32768,  1367,   399,  1694,   235,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   621,-32768,-32768,   399,    -3,   110,   120,-32768,   186,-32768,
  1034,  1034,  1034,-32768,  1034,  1034,  1034,   146,  1034,-32768,
  1034,  1034,  1034,  1034,  1034,   146,  1034,  1034,  1034,   145,
  1034,   145,-32768,    90,    -1,-32768,    73,   145,   145,  1034,
  1034,  1034,  1034,   178,   128,  1034,  1034,   145,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   134,   158,-32768,-32768,-32768,
-32768,-32768,   399,-32768,-32768,-32768,-32768,-32768,   129,   255,
   241,   735,   247,-32768,    27,   257,   292,   281,   287,   302,
   321,   229,   146,   621,   285,   315,  1476,   314,   332,   335,
   276,   356,   379,   416,    68,-32768,   414,   654,   395,  1034,
   353,   247,   390,-32768,-32768,-32768,-32768,-32768,   397,-32768,
-32768,-32768,-32768,   142,   247,  1694,   385,   184,-32768,-32768,
-32768,-32768,   735,  1694,-32768,  1034,  1034,  1034,  1034,  1034,
  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034,
  1034,  1912,  1912,  1034,   470,   300,  1034,   300,-32768,  1034,
-32768,  1034,-32768,-32768,-32768,-32768,-32768,   433,   247,  1034,
-32768,  1034,   451,  1034,  1034,   399,  1034,   428,-32768,  1034,
  1034,-32768,   460,-32768,  1034,  1034,  1034,-32768,-32768,-32768,
  1034,-32768,-32768,-32768,   456,  1034,-32768,-32768,  2021,  1034,
-32768,  1034,   433,-32768,   108,   310,   438,   457,   397,    48,
    48,    48,-32768,-32768,   433,-32768,-32768,  1034,   392,  1034,
   393,   735,   854,   901,   271,   530,   417,   319,   284,   284,
-32768,   355,   180,-32768,-32768,   541,   180,   180,   401,   402,
   404,  1803,   490,   406,   411,   415,   430,   490,   726,   621,
   432,-32768,   470,-32768,-32768,   621,   407,   509,   925,   621,
   492,   387,    53,   480,   218,  1034,-32768,-32768,   598,   598,
  1034,  1034,-32768,   507,   433,-32768,   495,  1034,-32768,-32768,
-32768,   399,   399,-32768,   498,  1034,-32768,-32768,   321,  1694,
-32768,   572,   527,   534,-32768,-32768,-32768,  1694,-32768,-32768,
  1034,  1034,   536,   540,  1034,  1034,   750,-32768,   543,   735,
   486,   108,   142,-32768,-32768,-32768,   438,   438,   473,-32768,
   427,   547,-32768,   558,-32768,-32768,-32768,-32768,   476,   510,
   489,   483,   306,-32768,-32768,-32768,   735,-32768,  1034,-32768,
-32768,   505,  1034,   521,  1034,-32768,-32768,   145,-32768,-32768,
-32768,   238,   146,   300,  1034,-32768,-32768,-32768,  1034,-32768,
-32768,-32768,-32768,  1034,  1034,  1034,-32768,-32768,-32768,   387,
-32768,-32768,-32768,   598,   502,-32768,-32768,-32768,   598,  1694,
  1694,   485,-32768,-32768,  1034,-32768,-32768,-32768,-32768,  1034,
-32768,-32768,   484,-32768,  1034,-32768,-32768,-32768,-32768,-32768,
-32768,   517,-32768,   625,-32768,-32768,-32768,   784,  1585,  1034,
  1034,-32768,   146,    24,-32768,   397,   610,   611,    91,   568,
   618,  1034,-32768,    90,-32768,-32768,   155,-32768,-32768,   519,
   618,-32768,   142,-32768,-32768,   438,-32768,-32768,    48,-32768,
-32768,   207,   145,   619,   306,   306,   306,-32768,-32768,-32768,
-32768,-32768,  1803,-32768,   621,   470,   546,    47,-32768,-32768,
   621,  1034,   620,-32768,-32768,-32768,-32768,-32768,-32768,  1034,
  1034,  1034,-32768,-32768,   598,  1034,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,  1034,   573,
   553,-32768,-32768,-32768,-32768,-32768,  1034,   630,-32768,-32768,
-32768,-32768,  1034,  1034,-32768,-32768,  1034,  1034,-32768,  1034,
   797,   822,-32768,-32768,   552,-32768,   146,   634,    24,    24,
   397,   438,-32768,-32768,-32768,-32768,-32768,   566,    91,-32768,
     2,   577,-32768,-32768,   618,-32768,-32768,-32768,   145,   196,
   305,   310,-32768,-32768,-32768,-32768,   570,-32768,   142,-32768,
   142,   142,-32768,-32768,   591,   595,  1034,   270,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   509,   145,-32768,
-32768,-32768,-32768,     6,   635,   592,-32768,-32768,   735,   378,
   651,-32768,   698,   652,-32768,   653,-32768,   840,   735,-32768,
   583,  1034,-32768,  1034,-32768,  1034,-32768,-32768,-32768,-32768,
   438,   658,   438,-32768,-32768,   310,-32768,   614,-32768,-32768,
-32768,-32768,   142,   142,   618,   618,-32768,  1034,-32768,-32768,
-32768,    70,    70,   664,   145,    70,-32768,-32768,   665,   270,
-32768,   300,   238,  1034,-32768,-32768,   145,-32768,-32768,-32768,
-32768,-32768,   588,    45,   698,   686,-32768,   415,-32768,-32768,
-32768,-32768,-32768,   735,   735,   735,-32768,   669,-32768,   670,
-32768,-32768,   310,   438,   438,   438,-32768,-32768,-32768,-32768,
   600,   673,   683,-32768,   145,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   133,   612,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   510,   616,   615,   622,-32768,
-32768,-32768,-32768,-32768,-32768,   686,-32768,-32768,  1034,   438,
-32768,-32768,-32768,-32768,   735,-32768,   142,   142,   142,-32768,
-32768,-32768,-32768,-32768,-32768,   727,   729,-32768
};

static const short yypgoto[] = {-32768,
   674,   -40,-32768,   671,-32768,   132,   579,  -398,   -20,   542,
-32768,-32768,-32768,   143,-32768,-32768,   564,    83,   -30,  -279,
   682,  -232,-32768,  -515,   106,  -297,-32768,-32768,-32768,-32768,
-32768,-32768,    75,  -186,   339,-32768,-32768,-32768,-32768,-32768,
  -226,   431,  -594,  -389,  -365,-32768,  -354,   313,   -48,   176,
  -341,-32768,  -338,-32768,   676,    -4,-32768,  -405,    95,-32768,
  -216,-32768,-32768,-32768,-32768,    41,-32768,     5,   -11,-32768,
-32768,   522,-32768,   -78,   511,   535,   459,    62,    25,-32768,
  -155,-32768,   203,   -15,-32768,   -88,-32768,  -570,  -256,  -563,
    72,-32768,-32768,  -140,-32768,   181,-32768,    28,   711,  -424,
-32768,-32768,-32768,  -233,   -19,    -2,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,  -247,-32768,    12,  -528,  -100,  -332,  -443,
   500,  -413,   182,-32768,  -252,   274,-32768,  -359,   303,-32768,
  -462,-32768,  -566,-32768,  -355,-32768,-32768,-32768,-32768,-32768,
-32768,   173,   524,  -494
};


#define	YYLAST		2137


static const short yytable[] = {    82,
    83,    85,    86,    87,   198,   340,   328,    88,    89,    91,
    92,    93,    94,    95,    96,    97,   180,   121,    98,   296,
   531,    99,   650,   107,   100,   101,   102,   301,   109,   110,
   111,   112,   113,   154,   402,   429,   116,   117,   118,   484,
   108,   508,   538,   411,   412,   512,   146,   147,   625,   426,
   427,   394,   602,   557,     6,   695,   431,     9,    10,   436,
   437,   496,    12,   764,     6,   181,   391,   707,   723,   599,
   444,    12,    43,   250,   708,   545,   327,   327,   413,   334,
   600,   195,    12,   633,  -250,   544,   445,   251,   447,   119,
   208,   209,   210,   196,   197,   420,   196,   197,   449,    43,
   699,   286,   700,   701,   105,    12,   216,   217,   218,   219,
   220,   580,   239,   156,   221,     6,   222,   223,   224,   742,
   225,   579,    12,   157,   226,   227,   228,   229,   230,   231,
   723,    58,   234,   235,   236,   237,   238,   276,   464,   707,
   610,   634,   262,   652,   497,   498,   708,    12,   538,   658,
   508,   765,   687,     6,   512,   407,   160,   681,   495,    12,
   649,   187,   601,   192,   747,   103,   327,   178,   179,   204,
   205,   182,   183,   184,   599,   599,   775,   334,   200,   215,
   240,   368,   201,   473,   202,   600,   600,   283,    58,   241,
   131,   482,   135,   458,   458,   298,   135,   760,   605,   606,
   295,   540,    71,   194,   312,   313,   314,   315,   316,   317,
   318,   654,   159,   656,     6,   364,   165,   168,   169,   341,
   633,    12,   175,   176,   211,   346,   169,   348,   349,   350,
   351,   352,   748,   749,   212,   358,   196,   197,   647,   232,
   721,   722,   484,   367,   356,     6,   187,   135,     9,    10,
   778,   779,   375,   376,   377,   646,   379,   380,   381,    71,
  -300,   651,   385,   233,  -300,   107,  -300,   244,   800,   801,
   802,   630,   242,   558,   559,   243,   334,   140,   634,   141,
   142,   414,   108,   141,   142,   164,   245,   252,   246,    76,
    77,   164,   198,   326,   326,   796,   333,   338,   458,   338,
   447,   689,   247,   458,   448,   690,   644,   691,   131,   254,
   449,   705,     6,   395,    12,   335,   160,     6,   395,   430,
   396,   468,   469,   538,    12,   396,   359,   255,   136,   137,
   138,   139,   140,   257,   141,   142,   259,   462,   260,   372,
   261,   374,   706,   523,   467,   265,   393,   398,   268,   682,
   384,   408,   408,   408,   389,   474,   125,   126,   127,   128,
   341,   266,   129,   130,   750,   329,   269,   135,   339,   270,
   256,   258,   107,   457,   457,-32768,   130,   327,   522,   523,
   334,   357,   334,   326,   500,   165,   362,   271,   365,   108,
   272,   169,    12,   715,   333,   547,   548,   373,   312,   458,
   528,     2,   530,     3,     4,     5,-32768,-32768,   774,   623,
   129,   130,   626,   273,   780,   300,   781,   439,   440,   441,
   450,   442,   542,   341,   341,    91,    18,   546,   738,   415,
   740,   550,   466,   552,     6,   395,   640,   641,   642,   274,
   472,    12,   396,   454,   459,   752,   753,   574,   419,   137,
   138,   139,   140,   393,   141,   142,   485,   363,   679,   680,
   278,    91,   505,   581,   280,   582,   282,   803,   804,   805,
   585,   586,   284,   501,   286,   297,   594,     6,   457,   759,
     9,    10,   353,   457,    12,   360,   573,   619,   366,   536,
   620,   621,     6,   560,   371,   338,   382,   400,   627,    12,
   561,   104,   416,   405,   135,   418,   502,   729,   421,   130,
   685,   751,   432,   543,  -160,  -159,   643,  -158,   645,   423,
   549,   312,   562,   563,  -155,   612,     6,   395,  -156,   341,
   433,   341,  -301,    12,   396,   612,  -301,   657,  -301,   575,
   446,   463,   503,  -157,   576,   428,   173,   564,   565,   578,
   566,   567,   660,   135,   173,   465,   535,   334,   470,   191,
   607,   480,   611,   341,   493,   501,   541,   471,   481,   729,
   486,   505,   611,   670,   487,   516,   618,   492,   499,   457,
   408,   515,   517,   636,   638,   438,   137,   138,   139,   140,
   518,   141,   142,   685,   326,   521,   577,   333,   502,   520,
   568,   622,   475,   476,   477,     6,   478,   696,     9,    10,
   697,   532,    12,   196,   197,   556,   126,   127,   128,   300,
   131,   129,   130,   711,  -301,     6,   395,   534,  -301,   583,
  -301,   173,    12,   396,   503,   451,   138,   139,   140,   612,
   141,   142,   584,   661,   603,   604,   717,   502,   191,   148,
   551,   664,   648,   639,   653,   662,   149,   666,   663,   686,
   452,   162,   737,   453,   665,   676,   150,   162,   678,   741,
   607,   177,   683,   702,   151,   698,   611,   703,   152,   188,
   688,   714,   655,   694,   449,   716,   730,   731,   206,   733,
   325,   325,   739,   332,   213,   214,   743,   502,   754,   757,
   279,   763,   767,   772,   773,     6,   782,   783,     9,    10,
   713,   704,    12,    54,   769,   770,   771,   784,   788,   612,
   612,   215,   791,  -301,   728,   792,   807,  -301,   808,  -301,
   120,   281,   671,    54,   793,   718,   122,   123,   124,   125,
   126,   127,   128,   104,   397,   129,   130,   694,   277,   158,
   732,   789,   153,   712,   148,   529,   611,   611,   424,   667,
   719,   149,   553,   720,   174,   766,   755,   786,   188,   490,
   369,   150,   425,   338,   794,   361,   347,   677,   762,   151,
   325,   758,   785,   152,   206,   155,   728,   790,   406,   628,
   684,   332,   206,   693,   302,   303,   304,   305,   306,   307,
   308,   309,   310,   587,   694,   721,   722,   456,   456,   399,
   322,   322,   624,     0,     0,     0,   755,   122,   123,   124,
   125,   126,   127,   128,   787,     0,   129,   130,     0,     0,
     0,     0,   122,   123,   124,   125,   126,   127,   128,   370,
     0,   129,   130,     0,     0,   761,     0,     0,     0,   378,
   104,     0,     0,     0,     0,     0,     0,   387,     0,   504,
   390,     0,     0,     0,   491,     0,   122,   123,   124,   125,
   126,   127,   128,     0,     0,   129,   130,     0,   417,   122,
   123,   124,   125,   126,   127,   128,     0,     0,   129,   130,
   332,   539,     0,     0,     0,     0,     0,     0,   588,     0,
   322,     0,     0,     0,   122,   123,   124,   125,   126,   127,
   128,   672,   456,   129,   130,     0,     0,   456,     0,     0,
   572,     0,   122,   123,   124,   125,   126,   127,   128,   460,
   461,   129,   130,     0,     0,     0,   674,   123,   124,   125,
   126,   127,   128,     0,     0,   129,   130,     0,   206,     0,
     0,   595,     0,   148,   588,     0,   206,     0,     0,   504,
   149,     0,     0,   488,   489,     0,     0,     0,   504,   504,
   150,   435,     0,     0,     0,     0,     0,     0,   151,     0,
   635,     0,   152,     0,     0,-32768,   125,   126,   127,   128,
     0,   325,   129,   130,   332,     0,   332,     0,     0,     0,
     0,   533,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   456,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   206,   206,
     0,     6,     7,     8,     9,    10,     0,    11,    12,    13,
     0,     0,     0,     0,     0,   595,    14,     0,    15,    16,
    17,     0,     0,    19,     0,     0,     0,     0,   591,   592,
    20,    21,    22,   504,     0,     0,    23,    24,    25,   692,
   504,    26,    27,     0,    28,     0,    29,     0,    30,     0,
    31,     0,    32,    33,     0,    34,    35,    36,    37,    38,
    39,    40,    41,     0,     0,     0,     0,     0,    42,     0,
    44,   322,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   727,     0,    45,     0,     0,     0,     0,     0,     0,
     0,     0,    46,     0,   659,    47,    48,     0,     0,    49,
     0,    50,    51,    52,   504,    53,     0,     0,     0,    54,
     0,     0,     0,   504,   504,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   668,   669,     0,   592,     0,
     0,   332,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   727,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   504,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
   734,     0,   735,     1,   736,     2,     0,     3,     4,     5,
     6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
     0,     0,     0,     0,     0,    14,   206,    15,    16,    17,
    18,  -256,    19,     0,     0,     0,     0,     0,  -256,    20,
    21,    22,     0,     0,     0,    23,    24,    25,  -256,     0,
    26,    27,     0,    28,     0,    29,  -256,    30,     0,    31,
  -256,    32,    33,     0,    34,    35,    36,    37,    38,    39,
    40,    41,     0,   -11,     0,     0,     0,    42,    43,    44,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    45,     0,     0,     0,     0,     0,     0,     0,
     0,    46,     0,     0,    47,    48,     0,     0,    49,     0,
    50,    51,    52,     0,    53,     0,     0,   795,    54,     2,
     0,     3,     4,     5,     6,     7,     8,     9,    10,     0,
    11,    12,    13,     0,     0,     0,     0,     0,     0,    14,
     0,    15,    16,    17,    18,     0,    19,     0,     0,     0,
     0,     0,     0,    20,    21,    22,     0,     0,     0,    23,
    24,    25,     0,     0,    26,    27,     0,    28,     0,    29,
     0,    30,     0,    31,     0,    32,    33,     0,    34,    35,
    36,    37,    38,    39,    40,    41,     0,   -11,     0,     0,
     0,    42,    43,    44,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    45,     0,     0,     0,
     0,     0,     0,     0,     0,    46,     0,     0,    47,    48,
     0,     0,    49,     0,    50,    51,    52,     0,    53,     0,
     0,     0,    54,     6,     7,     8,     9,    10,     0,    11,
    12,    13,     0,     0,     0,     0,     0,     0,    14,     0,
    15,    16,    17,     0,     0,    19,     0,     0,     0,     0,
     0,     0,    20,    21,    22,     0,     0,     0,    23,    24,
    25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
    30,     0,    31,     0,    32,    33,   267,    34,    35,    36,
    37,    38,    39,    40,    41,     0,     0,     0,     0,     0,
    42,     0,    44,     0,     0,     0,     0,     0,   122,   123,
   124,   125,   126,   127,   128,    45,     0,   129,   130,     0,
     0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
     0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
     0,    54,     6,     7,     8,     9,    10,     0,    11,    12,
    13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,     0,    34,    35,    36,    37,
    38,    39,    40,    41,     0,     0,     0,     0,     0,    42,
     0,    44,     0,     0,     0,     0,     0,   122,   123,   124,
   125,   126,   127,   128,    45,     0,   129,   130,     0,     0,
     0,     0,     0,    46,     0,     0,    47,    48,     0,     0,
    49,     0,    50,    51,    52,     0,    53,     0,     0,   590,
    54,     6,     7,     8,     9,    10,     0,    11,    12,    13,
     0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
    17,     0,     0,    19,     0,     0,     0,     0,     0,     0,
    20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
     0,    26,    27,     0,    28,     0,    29,     0,    30,     0,
    31,     0,    32,    33,     0,    34,    35,    36,    37,    38,
    39,    40,    41,     0,     0,     0,     0,     0,    42,     0,
    44,     0,     0,     0,     0,     0,   122,   123,   124,   125,
   126,   127,   128,    45,     0,   129,   130,     0,     0,     0,
     0,     0,    46,     0,     0,    47,    48,     0,     0,    49,
     0,    50,    51,    52,     0,    53,     0,     0,     0,    54,
     6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
     0,     0,     0,     0,     0,    14,     0,    15,    16,    17,
     0,     0,    19,     0,     0,     0,     0,     0,     0,    20,
   319,    22,     0,     0,     0,    23,    24,    25,     0,     0,
    26,    27,     0,    28,     0,    29,     0,    30,     0,    31,
     0,    32,    33,     0,    34,   320,    36,    37,   321,    39,
    40,    41,     0,     0,     0,     0,     0,    42,     0,    44,
     0,     0,     0,     0,     0,   122,   123,   124,   125,   126,
   127,   128,    45,     0,   129,   130,     0,     0,     0,     0,
     0,    46,     0,     0,    47,    48,     0,     0,    49,     0,
    50,    51,    52,     0,    53,     0,     0,     0,    54,     6,
     7,     8,     9,    10,     0,    11,    12,    13,     0,     0,
     0,     0,     0,     0,    14,     0,    15,    16,    17,     0,
     0,    19,     0,     0,     0,     0,     0,     0,    20,   319,
    22,     0,     0,     0,    23,    24,    25,     0,     0,    26,
    27,     0,    28,     0,    29,     0,    30,     0,    31,     0,
    32,    33,     0,    34,   320,    36,    37,   321,    39,    40,
    41,     0,     0,     0,     0,     0,    42,     0,    44,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
    46,     0,     0,    47,    48,     0,     0,    49,     0,    50,
    51,    52,     0,    53,     0,     0,     0,    54,     6,     7,
     8,     9,    10,     0,    11,    12,    13,     0,     0,     0,
     0,     0,     0,    14,     0,    15,    16,    17,     0,     0,
    19,     0,     0,     0,     0,     0,     0,    20,    21,    22,
     0,     0,     0,    23,    24,    25,     0,     0,    26,    27,
     0,    28,     0,    29,     0,    30,     0,    31,     0,    32,
    33,     0,    34,    35,    36,    37,    38,    39,    40,    41,
     0,     0,     0,     0,     0,    42,     0,    44,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
     0,     0,    47,    48,     0,     0,   386,     0,    50,    51,
    52,     0,    53,     0,     0,     0,    54
};

static const short yycheck[] = {    15,
    16,    17,    18,    19,   105,   238,   233,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    95,    58,    34,   206,
   419,    37,   538,    43,    40,    41,    42,   214,    44,    45,
    46,    47,    48,    74,   287,   333,    52,    53,    54,   381,
    43,   401,   432,   291,   292,   401,    62,    63,   511,   329,
   330,   285,   496,   459,     8,   622,   336,    11,    12,   339,
   340,   394,    15,    19,     8,    96,   283,   638,   663,   494,
    18,    15,    76,    47,   638,   441,   232,   233,   295,   235,
   494,    83,    15,    14,    83,   440,    34,    61,    83,    71,
   111,   112,   113,    95,    96,   322,    95,    96,    93,    76,
   629,    78,   631,   632,    43,    15,   122,   123,   124,   125,
   126,   477,   153,     4,   130,     8,   132,   133,   134,   686,
   136,   476,    15,     4,   140,   141,   142,   143,   144,   145,
   725,     0,   148,   149,   150,   151,   152,    70,   355,   710,
   500,    72,   173,   542,   397,   398,   710,    15,   538,   555,
   510,   107,   615,     8,   510,   108,    82,   601,   392,    15,
   114,   100,   495,   102,   693,   109,   322,    93,    94,   108,
   109,    97,    98,    99,   599,   600,   743,   333,   106,   118,
    52,   260,   110,   370,   112,   599,   600,   192,    57,    61,
    59,   378,    13,   349,   350,   211,    13,   713,   108,   109,
   205,   434,     0,   114,   225,   226,   227,   228,   229,   230,
   231,   550,    81,   552,     8,   256,    85,    86,    87,   240,
    14,    15,    91,    92,    47,   241,    95,   243,   244,   245,
   246,   247,   695,   696,   107,   251,    95,    96,   536,   106,
   108,   109,   584,   259,   249,     8,   185,    13,    11,    12,
   745,   746,   268,   269,   270,   535,   272,   273,   274,    57,
   106,   541,   278,   106,   110,   285,   112,    21,   797,   798,
   799,   519,    18,   460,   461,    35,   432,    98,    72,   100,
   101,   297,   285,   100,   101,    83,    40,    31,    42,   104,
   105,    89,   393,   232,   233,   790,   235,   236,   454,   238,
    83,   106,    56,   459,    87,   110,   533,   112,   177,    18,
    93,    42,     8,     9,    15,    16,   242,     8,     9,   335,
    16,   362,   363,   713,    15,    16,   252,    47,    94,    95,
    96,    97,    98,    47,   100,   101,    35,   353,    18,   265,
   112,   267,    73,    74,   360,    61,   285,   286,    35,   602,
   276,   290,   291,   292,   280,   371,    86,    87,    88,    89,
   381,    47,    92,    93,   697,   234,    35,    13,   237,    35,
   168,   169,   392,   349,   350,    92,    93,   533,    73,    74,
   536,   250,   538,   322,   400,   254,   255,   112,   257,   392,
    35,   260,    15,    16,   333,   444,   445,   266,   419,   555,
   416,     3,   418,     5,     6,     7,    88,    89,   741,   510,
    92,    93,   513,    35,   747,   213,   749,    31,    32,    33,
   346,    35,   438,   444,   445,   441,    28,   443,   681,   298,
   683,   447,   358,   449,     8,     9,   525,   526,   527,    24,
   366,    15,    16,   349,   350,   702,   703,   463,    94,    95,
    96,    97,    98,   392,   100,   101,   382,   255,   599,   600,
    47,   477,   401,   479,    70,   481,   114,   800,   801,   802,
   486,   487,    83,    47,    78,    91,   492,     8,   454,   712,
    11,    12,    50,   459,    15,    35,   462,   503,    61,   428,
   506,   507,     8,     9,    35,   434,    41,    60,   514,    15,
    16,    43,   111,    47,    13,   113,    80,   663,    19,    93,
   611,   698,   106,   439,   114,   114,   532,   114,   534,   114,
   446,   542,    38,    39,   114,   501,     8,     9,   114,   550,
    22,   552,   106,    15,    16,   511,   110,   553,   112,   465,
    61,    35,   116,   114,   470,   114,    88,    63,    64,   475,
    66,    67,   568,    13,    96,    61,   425,   713,    61,   101,
   499,    35,   501,   584,    79,    47,   435,   365,    35,   725,
    35,   510,   511,   589,    35,    18,   502,    35,   106,   555,
   519,    35,   107,   522,   523,    94,    95,    96,    97,    98,
    81,   100,   101,   694,   533,   113,   113,   536,    80,   111,
   116,    83,    31,    32,    33,     8,    35,   623,    11,    12,
   626,   107,    15,    95,    96,   114,    87,    88,    89,   417,
   489,    92,    93,   639,   106,     8,     9,   107,   110,   113,
   112,   173,    15,    16,   116,    38,    96,    97,    98,   615,
   100,   101,    18,   569,    35,    35,   662,    80,   190,    29,
   448,   577,   107,    35,    35,    83,    36,   583,   106,    83,
    63,    83,   678,    66,    35,   114,    46,    89,    35,   685,
   609,    93,   107,    83,    54,   106,   615,    83,    58,   101,
   619,    47,   551,   622,    93,    35,    35,    35,   110,   107,
   232,   233,    35,   235,   116,   117,    83,    80,    35,    35,
    47,   114,    17,    35,    35,     8,   107,    35,    11,    12,
   649,   637,    15,   116,   730,   731,   732,    35,   107,   695,
   696,   660,   107,   106,   663,   111,     0,   110,     0,   112,
    57,   190,   590,   116,   113,    38,    83,    84,    85,    86,
    87,    88,    89,   285,   286,    92,    93,   686,   185,    79,
   668,   767,    71,   648,    29,   417,   695,   696,   328,   584,
    63,    36,   450,    66,    89,   725,   705,   763,   190,    20,
   260,    46,    47,   712,   786,   254,   242,   597,   717,    54,
   322,   710,   755,    58,   206,    75,   725,   776,   289,   516,
   609,   333,   214,   621,   216,   217,   218,   219,   220,   221,
   222,   223,   224,    20,   743,   108,   109,   349,   350,   286,
   232,   233,   510,    -1,    -1,    -1,   755,    83,    84,    85,
    86,    87,    88,    89,   763,    -1,    92,    93,    -1,    -1,
    -1,    -1,    83,    84,    85,    86,    87,    88,    89,   261,
    -1,    92,    93,    -1,    -1,   714,    -1,    -1,    -1,   271,
   392,    -1,    -1,    -1,    -1,    -1,    -1,   279,    -1,   401,
   282,    -1,    -1,    -1,   115,    -1,    83,    84,    85,    86,
    87,    88,    89,    -1,    -1,    92,    93,    -1,   300,    83,
    84,    85,    86,    87,    88,    89,    -1,    -1,    92,    93,
   432,   433,    -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,
   322,    -1,    -1,    -1,    83,    84,    85,    86,    87,    88,
    89,   115,   454,    92,    93,    -1,    -1,   459,    -1,    -1,
   462,    -1,    83,    84,    85,    86,    87,    88,    89,   351,
   352,    92,    93,    -1,    -1,    -1,   115,    84,    85,    86,
    87,    88,    89,    -1,    -1,    92,    93,    -1,   370,    -1,
    -1,   493,    -1,    29,   115,    -1,   378,    -1,    -1,   501,
    36,    -1,    -1,   385,   386,    -1,    -1,    -1,   510,   511,
    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,
   522,    -1,    58,    -1,    -1,    85,    86,    87,    88,    89,
    -1,   533,    92,    93,   536,    -1,   538,    -1,    -1,    -1,
    -1,   423,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,   555,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   460,   461,
    -1,     8,     9,    10,    11,    12,    -1,    14,    15,    16,
    -1,    -1,    -1,    -1,    -1,   597,    23,    -1,    25,    26,
    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,   490,   491,
    37,    38,    39,   615,    -1,    -1,    43,    44,    45,   621,
   622,    48,    49,    -1,    51,    -1,    53,    -1,    55,    -1,
    57,    -1,    59,    60,    -1,    62,    63,    64,    65,    66,
    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    75,    -1,
    77,   533,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,   663,    -1,    90,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    99,    -1,   556,   102,   103,    -1,    -1,   106,
    -1,   108,   109,   110,   686,   112,    -1,    -1,    -1,   116,
    -1,    -1,    -1,   695,   696,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,   587,   588,    -1,   590,    -1,
    -1,   713,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,   725,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,   743,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
   672,    -1,   674,     1,   676,     3,    -1,     5,     6,     7,
     8,     9,    10,    11,    12,    -1,    14,    15,    16,    -1,
    -1,    -1,    -1,    -1,    -1,    23,   698,    25,    26,    27,
    28,    29,    30,    -1,    -1,    -1,    -1,    -1,    36,    37,
    38,    39,    -1,    -1,    -1,    43,    44,    45,    46,    -1,
    48,    49,    -1,    51,    -1,    53,    54,    55,    -1,    57,
    58,    59,    60,    -1,    62,    63,    64,    65,    66,    67,
    68,    69,    -1,    71,    -1,    -1,    -1,    75,    76,    77,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    99,    -1,    -1,   102,   103,    -1,    -1,   106,    -1,
   108,   109,   110,    -1,   112,    -1,    -1,   789,   116,     3,
    -1,     5,     6,     7,     8,     9,    10,    11,    12,    -1,
    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,
    -1,    25,    26,    27,    28,    -1,    30,    -1,    -1,    -1,
    -1,    -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,
    44,    45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,
    -1,    55,    -1,    57,    -1,    59,    60,    -1,    62,    63,
    64,    65,    66,    67,    68,    69,    -1,    71,    -1,    -1,
    -1,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,   102,   103,
    -1,    -1,   106,    -1,   108,   109,   110,    -1,   112,    -1,
    -1,    -1,   116,     8,     9,    10,    11,    12,    -1,    14,
    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,
    25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,
    -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,
    45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,
    55,    -1,    57,    -1,    59,    60,    61,    62,    63,    64,
    65,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,
    75,    -1,    77,    -1,    -1,    -1,    -1,    -1,    83,    84,
    85,    86,    87,    88,    89,    90,    -1,    92,    93,    -1,
    -1,    -1,    -1,    -1,    99,    -1,    -1,   102,   103,    -1,
    -1,   106,    -1,   108,   109,   110,    -1,   112,    -1,    -1,
    -1,   116,     8,     9,    10,    11,    12,    -1,    14,    15,
    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,
    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,
    -1,    57,    -1,    59,    60,    -1,    62,    63,    64,    65,
    66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    75,
    -1,    77,    -1,    -1,    -1,    -1,    -1,    83,    84,    85,
    86,    87,    88,    89,    90,    -1,    92,    93,    -1,    -1,
    -1,    -1,    -1,    99,    -1,    -1,   102,   103,    -1,    -1,
   106,    -1,   108,   109,   110,    -1,   112,    -1,    -1,   115,
   116,     8,     9,    10,    11,    12,    -1,    14,    15,    16,
    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,    26,
    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,    -1,
    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,    -1,
    57,    -1,    59,    60,    -1,    62,    63,    64,    65,    66,
    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    75,    -1,
    77,    -1,    -1,    -1,    -1,    -1,    83,    84,    85,    86,
    87,    88,    89,    90,    -1,    92,    93,    -1,    -1,    -1,
    -1,    -1,    99,    -1,    -1,   102,   103,    -1,    -1,   106,
    -1,   108,   109,   110,    -1,   112,    -1,    -1,    -1,   116,
     8,     9,    10,    11,    12,    -1,    14,    15,    16,    -1,
    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,    26,    27,
    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,
    38,    39,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,
    48,    49,    -1,    51,    -1,    53,    -1,    55,    -1,    57,
    -1,    59,    60,    -1,    62,    63,    64,    65,    66,    67,
    68,    69,    -1,    -1,    -1,    -1,    -1,    75,    -1,    77,
    -1,    -1,    -1,    -1,    -1,    83,    84,    85,    86,    87,
    88,    89,    90,    -1,    92,    93,    -1,    -1,    -1,    -1,
    -1,    99,    -1,    -1,   102,   103,    -1,    -1,   106,    -1,
   108,   109,   110,    -1,   112,    -1,    -1,    -1,   116,     8,
     9,    10,    11,    12,    -1,    14,    15,    16,    -1,    -1,
    -1,    -1,    -1,    -1,    23,    -1,    25,    26,    27,    -1,
    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
    39,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,    48,
    49,    -1,    51,    -1,    53,    -1,    55,    -1,    57,    -1,
    59,    60,    -1,    62,    63,    64,    65,    66,    67,    68,
    69,    -1,    -1,    -1,    -1,    -1,    75,    -1,    77,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    99,    -1,    -1,   102,   103,    -1,    -1,   106,    -1,   108,
   109,   110,    -1,   112,    -1,    -1,    -1,   116,     8,     9,
    10,    11,    12,    -1,    14,    15,    16,    -1,    -1,    -1,
    -1,    -1,    -1,    23,    -1,    25,    26,    27,    -1,    -1,
    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,
    -1,    -1,    -1,    43,    44,    45,    -1,    -1,    48,    49,
    -1,    51,    -1,    53,    -1,    55,    -1,    57,    -1,    59,
    60,    -1,    62,    63,    64,    65,    66,    67,    68,    69,
    -1,    -1,    -1,    -1,    -1,    75,    -1,    77,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
    -1,    -1,   102,   103,    -1,    -1,   106,    -1,   108,   109,
   110,    -1,   112,    -1,    -1,    -1,   116
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */

/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

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

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif



/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
{ yyoutput = yyvsp[-1].t; YYACCEPT; ;
    break;}
case 2:
{ yyoutput = PA_parseError; YYABORT; ;
    break;}
case 3:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 4:
{ yyval.t = oz_cons(newCTerm(PA_fSynTopLevelProductionTemplates,
					   yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 5:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 6:
{ yyval.t = oz_cons(newCTerm(PA_fFunctor,newCTerm(PA_fDollar,yyvsp[-2].t),
					   yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 7:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 8:
{ yyval.t = oz_cons(newCTerm(PA_fDeclare,yyvsp[-3].t,newCTerm(PA_fSkip,yyvsp[-1].t),
					   yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 9:
{ yyval.t = oz_cons(newCTerm(PA_fDeclare,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 10:
{ yyval.t = oz_cons(newCTerm(PA_fDeclare,yyvsp[-2].t,
					   newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 11:
{ yyval.t = AtomNil; ;
    break;}
case 12:
{ yyval.t = newCTerm(PA_dirSwitch,yyvsp[0].t); ;
    break;}
case 13:
{ yyval.t = PA_dirLocalSwitches; ;
    break;}
case 14:
{ yyval.t = PA_dirPushSwitches; ;
    break;}
case 15:
{ yyval.t = PA_dirPopSwitches; ;
    break;}
case 16:
{ yyval.t = AtomNil; ;
    break;}
case 17:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 18:
{ if (!strcmp(xytext,"gump"))
		      xy_gumpSyntax = 1;
		    if (!strcmp(xytext,"allowdeprecated"))
		      xy_allowDeprecated = 1;
		    yyval.t = newCTerm(PA_on,OZ_atom(xytext),pos());
		  ;
    break;}
case 19:
{ if (!strcmp(xytext,"gump"))
		      xy_gumpSyntax = 0;
		    if (!strcmp(xytext,"allowdeprecated"))
		      xy_allowDeprecated = 0;
		    yyval.t = newCTerm(PA_off,OZ_atom(xytext),pos());
		  ;
    break;}
case 20:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 21:
{ yyval.t = newCTerm(PA_fAnd,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 22:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 23:
{ yyval.t = newCTerm(PA_fDotAssign,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 24:
{ yyval.t = newCTerm(PA_fAssign,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 25:
{ yyval.t = newCTerm(PA_fOrElse,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 26:
{ yyval.t = newCTerm(PA_fAndThen,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 27:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 28:
{ yyval.t = newCTerm(PA_fFdCompare,yyvsp[-2].t,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 29:
{ yyval.t = newCTerm(PA_fFdIn,yyvsp[-2].t,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 30:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 31:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 32:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
				  oz_cons(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 33:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 34:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 35:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 36:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 37:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 38:
{ yyval.t = newCTerm(PA_fObjApply,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 39:
{ yyval.t = newCTerm(PA_fOpApply,AtomTilde,
				  oz_mklist(yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 40:
{ yyval.t = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 41:
{ yyval.t = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklist(yyvsp[-1].t,makeInt(xytext,pos())),pos()); ;
    break;}
case 42:
{ yyval.t = newCTerm(PA_fOpApply,AtomHat,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 43:
{ yyval.t = newCTerm(PA_fAt,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 44:
{ yyval.t = newCTerm(PA_fOpApply,AtomDExcl,
				  oz_mklist(yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 45:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 46:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 47:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 48:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 49:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 50:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 51:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 52:
{ yyval.t = newCTerm(PA_fSelf,pos()); ;
    break;}
case 53:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 54:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 55:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 56:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 57:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 58:
{ yyval.t = newCTerm(PA_fRecord,newCTerm(PA_fAtom,AtomCons,
						     makeLongPos(yyvsp[-4].t,yyvsp[0].t)),
				  oz_mklist(yyvsp[-3].t,yyvsp[-2].t)); ;
    break;}
case 59:
{ yyval.t = newCTerm(PA_fApply,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 60:
{ yyval.t = newCTerm(PA_fProc,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-7].t,makeLongPos(yyvsp[-8].t,yyvsp[0].t)); ;
    break;}
case 61:
{ yyval.t = newCTerm(PA_fFun,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-7].t,makeLongPos(yyvsp[-8].t,yyvsp[0].t)); ;
    break;}
case 62:
{ yyval.t = newCTerm(PA_fFunctor,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 63:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 64:
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t); ;
    break;}
case 65:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 66:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 67:
{ yyval.t = newCTerm(PA_fLock,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 68:
{ yyval.t = newCTerm(PA_fLockThen,yyvsp[-4].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 69:
{ yyval.t = newCTerm(PA_fThread,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 70:
{ yyval.t = newCTerm(PA_fTry,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 71:
{ yyval.t = newCTerm(PA_fRaise,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 72:
{ yyval.t = newCTerm(PA_fSkip,pos()); ;
    break;}
case 73:
{ yyval.t = newCTerm(PA_fFail,pos()); ;
    break;}
case 74:
{ yyval.t = newCTerm(PA_fNot,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 75:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 76:
{ yyval.t = newCTerm(PA_fOr,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 77:
{ yyval.t = newCTerm(PA_fDis,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 78:
{ yyval.t = newCTerm(PA_fChoice,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 79:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 80:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 81:
{ yyval.t = newCTerm(PA_fMacro,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 82:
{ yyval.t = newCTerm(PA_fLoop,
				  newCTerm(PA_fAnd,yyvsp[-4].t,yyvsp[-2].t),
				  makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 83:
{ yyval.t = newCTerm(PA_fFOR,yyvsp[-4].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 84:
{ yyval.t = AtomNil; ;
    break;}
case 85:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 86:
{ yyval.t = newCTerm(oz_atom("forFeature"),yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 87:
{ yyval.t = newCTerm(oz_atom("forPattern"),yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 88:
{ yyval.t = newCTerm(oz_atom("forGeneratorList"),yyvsp[0].t); ;
    break;}
case 89:
{ yyval.t = newCTerm(oz_atom("forGeneratorInt"),yyvsp[-3].t,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 90:
{ yyval.t = newCTerm(oz_atom("forGeneratorC"),yyvsp[-2].t,oz_head(yyvsp[0].t),oz_tail(yyvsp[0].t)); ;
    break;}
case 91:
{ yyval.t = newCTerm(oz_atom("forGeneratorC"),yyvsp[-3].t,oz_head(yyvsp[-1].t),oz_tail(yyvsp[-1].t)); ;
    break;}
case 92:
{ yyval.t = NameUnit; ;
    break;}
case 93:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 94:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 95:
{ yyval.t = NameUnit; ;
    break;}
case 96:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 97:
{
		    yyval.t = newCTerm(PA_fAnd,yyvsp[-1].t,yyvsp[0].t);
		  ;
    break;}
case 99:
{
		    yyval.t = newCTerm(PA_fMacro,
				  oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					  yyvsp[-7].t,
					  newCTerm(PA_fAtom,oz_atom("from"),NameUnit),
					  yyvsp[-4].t,
					  newCTerm(PA_fAtom,oz_atom("to"),NameUnit),
					  yyvsp[-2].t,
					  newCTerm(PA_fAtom,oz_atom("by"),NameUnit),
					  (yyvsp[-1].t == 0)?makeInt("1",NameUnit):yyvsp[-1].t,
					  0),
				  makeLongPos(OZ_subtree(yyvsp[-7].t,makeTaggedSmallInt(2)),yyvsp[0].t));
		  ;
    break;}
case 100:
{
		    /* <<for X 'in' L>>
		       <<for X = E1 'then' E2>> */
		    if (yyvsp[-1].t == 0) {
		      yyval.t = newCTerm(PA_fMacro,
				    oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					    yyvsp[-5].t,
					    newCTerm(PA_fAtom,oz_atom("in"),NameUnit),
					    yyvsp[-2].t,
					    0),
				    makeLongPos(OZ_subtree(yyvsp[-5].t,makeTaggedSmallInt(2)),yyvsp[0].t));
		    } else {
		      yyval.t = newCTerm(PA_fMacro,
				    oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					    newCTerm(PA_fEq,yyvsp[-5].t,yyvsp[-2].t,NameUnit),
					    newCTerm(PA_fAtom,oz_atom("next"),NameUnit),
					    yyvsp[-1].t,
					    0),
				    makeLongPos(OZ_subtree(yyvsp[-5].t,makeTaggedSmallInt(2)),yyvsp[0].t));
		    }
		  ;
    break;}
case 101:
{ yyval.t = 0; ;
    break;}
case 102:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 103:
{ yyval.t = AtomNil; ;
    break;}
case 104:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 105:
{ yyval.t = AtomNil; ;
    break;}
case 106:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 107:
{ yyval.t = oz_cons(newCTerm(PA_fRequire,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 108:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 109:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-1].t,
					   newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 110:
{ yyval.t = oz_cons(newCTerm(PA_fImport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 111:
{ yyval.t = oz_cons(newCTerm(PA_fExport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 112:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 113:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-1].t,
					   newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 114:
{ yyval.t = AtomNil; ;
    break;}
case 115:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-2].t,AtomNil,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 116:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 117:
{ yyval.t = newCTerm(PA_fVar,OZ_atom(xytext),yyvsp[0].t); ;
    break;}
case 118:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 119:
{ yyval.t = oz_mklist(oz_pair2(yyvsp[0].t,yyvsp[-2].t)); ;
    break;}
case 120:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 121:
{ yyval.t = oz_cons(oz_pair2(yyvsp[-1].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 122:
{ yyval.t = PA_fNoImportAt; ;
    break;}
case 123:
{ yyval.t = newCTerm(PA_fImportAt,yyvsp[0].t); ;
    break;}
case 124:
{ yyval.t = AtomNil; ;
    break;}
case 125:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 126:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,
					   newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t)),yyvsp[0].t); ;
    break;}
case 127:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 128:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 129:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 130:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 131:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 132:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 133:
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 134:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 135:
{ yyval.t = AtomNil; ;
    break;}
case 136:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 137:
{ yyval.t = newCTerm(PA_fAtom,AtomNil,yyvsp[0].t); ;
    break;}
case 138:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomCons,yyvsp[-2].t),
				  oz_mklist(yyvsp[-1].t,yyvsp[0].t)); ;
    break;}
case 139:
{ yyval.t = PA_fNoCatch; ;
    break;}
case 140:
{ yyval.t = newCTerm(PA_fCatch,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 141:
{ yyval.t = PA_fNoFinally; ;
    break;}
case 142:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 143:
{
		    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
				  newCTerm(PA_fAtom,yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
		  ;
    break;}
case 144:
{
		    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
				  makeVar(yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
		  ;
    break;}
case 145:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 146:
{ yyval.t = NameUnit; ;
    break;}
case 147:
{ yyval.t = NameTrue; ;
    break;}
case 148:
{ yyval.t = NameFalse; ;
    break;}
case 149:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 150:
{ yyval.t = AtomNil; ;
    break;}
case 151:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 152:
{ yyval.t = oz_cons(newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 153:
{ yyval.t = NameFalse; ;
    break;}
case 154:
{ yyval.t = NameTrue; ;
    break;}
case 155:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 156:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 157:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 158:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 159:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 160:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 161:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 162:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 163:
{ yyval.t = newCTerm(PA_fBoolCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 164:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 165:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 166:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 167:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 168:
{ checkDeprecation(yyvsp[-3].t);
		    yyval.t = newCTerm(PA_fBoolCase,yyvsp[-5].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-6].t,yyvsp[0].t));
		  ;
    break;}
case 169:
{ yyval.t = newCTerm(PA_fCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 170:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 171:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 172:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 173:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 174:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 175:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 176:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 177:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 178:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 179:
{ yyval.t = newCTerm(PA_fCaseClause,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 180:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 181:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-3].t,
				  newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 182:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-5].t,yyvsp[-2].t,yyvsp[0].t,yyvsp[-3].t); ;
    break;}
case 183:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 184:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 185:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 186:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
				  oz_cons(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 187:
{ yyval.t = newCTerm(PA_fClass,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 188:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 189:
{ yyval.t = newCTerm(PA_fDollar,yyvsp[0].t); ;
    break;}
case 190:
{ yyval.t = AtomNil; ;
    break;}
case 191:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 192:
{ yyval.t = newCTerm(PA_fFrom,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 193:
{ yyval.t = newCTerm(PA_fAttr,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 194:
{ yyval.t = newCTerm(PA_fFeat,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 195:
{ yyval.t = newCTerm(PA_fProp,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 196:
{ yyval.t = AtomNil; ;
    break;}
case 197:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 198:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 199:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 200:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 201:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 202:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 203:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 204:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 205:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 206:
{ yyval.t = AtomNil; ;
    break;}
case 207:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 208:
{ yyval.t = newCTerm(PA_fMeth,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-3].t); ;
    break;}
case 209:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 210:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 211:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 212:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 213:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 214:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 215:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 216:
{ yyval.t = newCTerm(PA_fRecord,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 217:
{ yyval.t = newCTerm(PA_fOpenRecord,yyvsp[-4].t,yyvsp[-2].t); ;
    break;}
case 218:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 219:
{ yyval.t = makeVar(xytext); ;
    break;}
case 220:
{ yyval.t = newCTerm(PA_fEscape,makeVar(xytext),yyvsp[-1].t); ;
    break;}
case 221:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 222:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 223:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 224:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 225:
{ yyval.t = AtomNil; ;
    break;}
case 226:
{ yyval.t = newCTerm(PA_fMethArg,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 227:
{ yyval.t = newCTerm(PA_fMethColonArg,yyvsp[-3].t,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 228:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 229:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 230:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 231:
{ yyval.t = newCTerm(PA_fDefault,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 232:
{ yyval.t = PA_fNoDefault; ;
    break;}
case 233:
{ yyval.t = newCTerm(PA_fCond,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 234:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 235:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 236:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 237:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 238:
{ yyval.t = newCTerm(PA_fClause,newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 239:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 240:
{ yyval.t = oz_mklist(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 241:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 242:
{ yyval.t = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,yyvsp[0].t),
				  yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 243:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-3].t,yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 244:
{ yyval.t = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 245:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 246:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 247:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 248:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 249:
{ yyval.t = makeVar(xytext); ;
    break;}
case 250:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 251:
{ yyval.t = newCTerm(PA_fEscape,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 252:
{ yyval.t = makeString(xytext,pos()); ;
    break;}
case 253:
{ yyval.t = makeInt(xytext,pos()); ;
    break;}
case 254:
{ yyval.t = makeInt(xytext[0],pos()); ;
    break;}
case 255:
{ yyval.t = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); ;
    break;}
case 256:
{ yyval.t = pos(); ;
    break;}
case 257:
{ yyval.t = pos(); ;
    break;}
case 258:
{ OZ_Term prefix =
		      scannerPrefix? scannerPrefix: PA_zy;
		    yyval.t = newCTerm(PA_fScanner,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,prefix,
				  makeLongPos(yyvsp[-6].t,yyvsp[0].t)); ;
    break;}
case 259:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 260:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 261:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 262:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 263:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 264:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 265:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 266:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 267:
{ yyval.t = newCTerm(PA_fLexicalRule,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 268:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 269:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 270:
{ yyval.t = newCTerm(PA_fMode,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 271:
{ yyval.t = AtomNil; ;
    break;}
case 272:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 273:
{ yyval.t = newCTerm(PA_fInheritedModes,yyvsp[0].t); ;
    break;}
case 274:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 275:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 276:
{ OZ_Term expect = parserExpect? parserExpect: makeTaggedSmallInt(0);
		    yyval.t = newCTerm(PA_fParser,yyvsp[-6].t,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,expect,
				  makeLongPos(yyvsp[-7].t,yyvsp[0].t)); ;
    break;}
case 277:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 278:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 279:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 280:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 281:
{ yyval.t = newCTerm(PA_fToken,AtomNil); ;
    break;}
case 282:
{ yyval.t = newCTerm(PA_fToken,yyvsp[0].t); ;
    break;}
case 283:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 284:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 285:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 286:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 287:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 288:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 289:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 290:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 291:
{ *prodKey[depth]++ = '='; ;
    break;}
case 292:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 293:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 294:
{ *prodKey[depth]++ = '='; ;
    break;}
case 295:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 296:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,PA_none); ;
    break;}
case 297:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 298:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 299:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 302:
{ prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg(yyvsp[-1].t,0))); ;
    break;}
case 303:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 304:
{ depth--; ;
    break;}
case 305:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 306:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 307:
{ depth--; ;
    break;}
case 308:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 309:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 310:
{ depth--; ;
    break;}
case 311:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 312:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 313:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 314:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 315:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 316:
{ *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; ;
    break;}
case 319:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 320:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 321:
{ *prodKey[depth] = '\0';
		    yyval.t = oz_pair2(prodName[depth],OZ_string(prodKeyBuffer[depth]));
		    prodName[depth] = PA_none;
		    prodKey[depth] = prodKeyBuffer[depth];
		  ;
    break;}
case 322:
{ yyval.t = AtomNil; ;
    break;}
case 323:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 324:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 325:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 326:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 327:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 328:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 329:
{ yyval.t = AtomNil; ;
    break;}
case 330:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 331:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 332:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 333:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 334:
{ yyval.t = newCTerm(PA_fSynAlternative, yyvsp[0].t); ;
    break;}
case 335:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 336:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 337:
{ OZ_Term t = yyvsp[0].t;
		    while (terms[depth]) {
		      t = oz_cons(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    yyval.t = newCTerm(PA_fSynSequence, decls[depth], t, yyvsp[-1].t);
		    decls[depth] = AtomNil;
		  ;
    break;}
case 338:
{ yyval.t = newCTerm(PA_fSynSequence, AtomNil, yyvsp[0].t, yyvsp[-1].t); ;
    break;}
case 339:
{ yyval.t = AtomNil; ;
    break;}
case 340:
{ yyval.t = oz_mklist(newCTerm(PA_fSynAction,yyvsp[0].t)); ;
    break;}
case 341:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 342:
{ yyval.t = oz_cons(newCTerm(PA_fSynTemplateInstantiation, yyvsp[0].t,
					   oz_cons(newCTerm(PA_fSynApplication,
							     terms[depth]->term,
							     AtomNil),
						    AtomNil),
					   yyvsp[-2].t),
				  yyvsp[-1].t);
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  ;
    break;}
case 343:
{ yyval.t = oz_cons(newCTerm(PA_fSynAssignment, terms[depth]->term, yyvsp[-1].t),
				  yyvsp[0].t);
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  ;
    break;}
case 344:
{ while (terms[depth]) {
		      decls[depth] = oz_cons(terms[depth]->term, decls[depth]);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    yyval.t = yyvsp[0].t;
		  ;
    break;}
case 345:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 346:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 347:
{ terms[depth] = new TermNode(yyvsp[0].t, terms[depth]); ;
    break;}
case 348:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 349:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 350:
{ yyval.t = newCTerm(PA_fSynAssignment,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 351:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 352:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 353:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_cons(newCTerm(PA_fSynApplication,yyvsp[-3].t,
						    AtomNil),
					   AtomNil),yyvsp[-1].t);
		  ;
    break;}
case 354:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 355:
{ yyval.t = newCTerm(PA_fSynAssignment,
				  newCTerm(PA_fEscape,yyvsp[-2].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 356:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 357:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 358:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_mklist(yyvsp[-2].t),yyvsp[-3].t);
		  ;
    break;}
case 359:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_mklist(yyvsp[-3].t),yyvsp[-1].t);
		  ;
    break;}
case 360:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 361:
{ depth--; ;
    break;}
case 362:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 363:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 364:
{ depth--; ;
    break;}
case 365:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 366:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 367:
{ depth--; ;
    break;}
case 368:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 369:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 370:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[-4].t,yyvsp[-1].t); ;
    break;}
case 371:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 372:
{ yyval.t = makeVar(xytext); ;
    break;}
case 373:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 374:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */


  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
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

  return OZ_pair2(yyoutput, xy_errorMessages);
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
