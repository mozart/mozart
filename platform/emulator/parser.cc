
/*  A Bison parser, made from /home/duchier/mozart/platform/emulator/parser.yy
 by  GNU Bison version 1.27
  */

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
#define	T_loop	322
#define	T_for	323
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
#define	T_orelse	338
#define	T_andthen	339
#define	T_COMPARE	340
#define	T_FDCOMPARE	341
#define	T_LMACRO	342
#define	T_RMACRO	343
#define	T_FDIN	344
#define	T_ADD	345
#define	T_FDMUL	346
#define	T_OTHERMUL	347
#define	T_DEREFF	348


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

OZ_Term _PA_AtomTab[108];

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
};

void parser_init(void) {
   for (int i = 108; i--; )
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
	     OZ_subtree(pos1,newSmallInt(1)),OZ_subtree(pos1,newSmallInt(2)),
	     OZ_subtree(pos1,newSmallInt(3)),OZ_subtree(pos2,newSmallInt(1)),
	     OZ_subtree(pos2,newSmallInt(2)),OZ_subtree(pos2,newSmallInt(3)));
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
  t->setArg(0, newSmallInt((unsigned char) c));
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



#define	YYFINAL		788
#define	YYFLAG		-32768
#define	YYNTBASE	116

#define YYTRANSLATE(x) ((unsigned)(x) <= 348 ? yytranslate[x] : 256)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   115,     2,    93,   109,     2,     2,     2,   106,
   107,     2,   103,    97,   104,    99,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,   114,   105,     2,
    83,     2,     2,   101,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   110,     2,   111,   100,   108,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   112,    92,   113,    98,     2,     2,     2,     2,
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
    88,    89,    90,    91,    94,    95,    96,   102
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     5,     8,    11,    13,    17,    20,    27,    34,
    40,    41,    44,    46,    48,    50,    51,    54,    57,    60,
    62,    65,    66,    69,    74,    79,    88,    95,   100,   105,
   110,   115,   120,   125,   127,   132,   134,   138,   143,   148,
   153,   158,   162,   167,   170,   175,   179,   183,   187,   189,
   191,   193,   195,   197,   199,   201,   203,   205,   207,   209,
   211,   218,   225,   236,   247,   254,   256,   263,   266,   269,
   275,   283,   289,   297,   303,   305,   307,   313,   316,   322,
   328,   334,   336,   338,   344,   350,   358,   361,   363,   372,
   379,   381,   382,   385,   386,   388,   393,   400,   405,   410,
   415,   422,   427,   428,   432,   439,   442,   444,   448,   451,
   456,   457,   460,   461,   464,   469,   471,   473,   475,   477,
   479,   481,   486,   488,   489,   492,   494,   498,   499,   503,
   504,   507,   515,   523,   525,   527,   529,   531,   533,   534,
   537,   542,   543,   545,   547,   549,   551,   553,   555,   557,
   559,   561,   568,   571,   574,   578,   580,   588,   595,   598,
   601,   605,   607,   609,   613,   617,   619,   623,   627,   629,
   634,   641,   646,   651,   653,   658,   666,   668,   670,   671,
   674,   679,   684,   689,   694,   695,   698,   702,   704,   706,
   708,   710,   712,   714,   716,   717,   720,   726,   728,   733,
   735,   737,   739,   741,   743,   748,   754,   756,   758,   762,
   764,   766,   768,   771,   772,   775,   780,   782,   784,   786,
   790,   791,   797,   800,   801,   803,   807,   812,   818,   822,
   826,   829,   834,   839,   845,   847,   851,   853,   855,   857,
   861,   863,   865,   867,   869,   870,   871,   880,   882,   884,
   886,   889,   892,   895,   901,   907,   912,   914,   916,   921,
   922,   925,   928,   930,   932,   942,   944,   946,   949,   952,
   953,   956,   958,   961,   963,   967,   969,   972,   974,   977,
   978,   988,   989,   990,  1001,  1008,  1012,  1015,  1018,  1020,
  1021,  1024,  1025,  1026,  1033,  1034,  1035,  1042,  1043,  1044,
  1051,  1053,  1057,  1059,  1061,  1063,  1064,  1066,  1068,  1070,
  1071,  1072,  1075,  1077,  1080,  1085,  1090,  1098,  1099,  1102,
  1104,  1106,  1108,  1110,  1112,  1116,  1119,  1123,  1124,  1127,
  1130,  1136,  1141,  1144,  1147,  1149,  1151,  1153,  1156,  1160,
  1162,  1164,  1169,  1171,  1177,  1179,  1181,  1187,  1192,  1193,
  1194,  1204,  1205,  1206,  1216,  1217,  1218,  1228,  1230,  1236,
  1238,  1240,  1242
};

static const short yyrhs[] = {   117,
    71,     0,     1,     0,   122,   118,     0,   210,   118,     0,
   118,     0,   194,   132,   118,     0,   119,   117,     0,    28,
   195,   122,    47,   194,   118,     0,    28,   195,   122,    47,
   122,   118,     0,    28,   195,   122,   194,   118,     0,     0,
     3,   120,     0,     5,     0,     6,     0,     7,     0,     0,
   121,   120,     0,   103,     4,     0,   104,     4,     0,   124,
     0,   124,   122,     0,     0,   105,   126,     0,   124,    83,
   195,   124,     0,   124,    84,   195,   124,     0,   124,    82,
   195,   126,    20,   126,   123,   195,     0,   124,    82,   195,
   126,   123,   195,     0,   124,    85,   195,   124,     0,   124,
    86,   195,   124,     0,   124,   138,   195,   124,     0,   124,
   139,   195,   124,     0,   124,   140,   195,   124,     0,   124,
    92,   195,   124,     0,   126,     0,   126,    93,   195,   125,
     0,   126,     0,   126,    93,   125,     0,   126,   141,   195,
   126,     0,   126,   142,   195,   126,     0,   126,   143,   195,
   126,     0,   126,    97,   195,   126,     0,    98,   195,   126,
     0,   126,    99,   195,   126,     0,   126,    13,     0,   126,
   100,   195,   126,     0,   101,   195,   126,     0,   102,   195,
   126,     0,   106,   144,   107,     0,   188,     0,   190,     0,
   108,     0,    66,     0,    63,     0,    38,     0,    59,     0,
   109,     0,   191,     0,   192,     0,   193,     0,   149,     0,
   110,   195,   124,   146,   111,   195,     0,   112,   195,   124,
   145,   113,   195,     0,    55,   195,   130,   112,   124,   145,
   113,   144,    35,   195,     0,    43,   195,   130,   112,   124,
   145,   113,   144,    35,   195,     0,    44,   195,   166,   131,
    35,   195,     0,   165,     0,    48,   195,   122,    47,   122,
    35,     0,    45,   156,     0,    23,   158,     0,    49,   195,
   144,    35,   195,     0,    49,   195,   124,    61,   144,    35,
   195,     0,    62,   195,   144,    35,   195,     0,    65,   195,
   144,   147,   148,    35,   195,     0,    57,   195,   144,    35,
   195,     0,    60,     0,    37,     0,    51,   195,   144,    35,
   195,     0,    27,   181,     0,    53,   195,   185,    35,   195,
     0,    30,   195,   185,    35,   195,     0,    25,   195,   187,
    35,   195,     0,   196,     0,   204,     0,    68,   195,   144,
    35,   195,     0,    89,   195,   145,    90,   195,     0,    69,
   195,   127,    70,   144,    35,   195,     0,   127,   128,     0,
   128,     0,   189,    47,   195,   126,    20,   126,   129,   195,
     0,   189,    47,   195,   126,   129,   195,     0,   123,     0,
     0,   188,   130,     0,     0,   132,     0,    58,   195,   133,
   131,     0,    54,   195,   122,    47,   122,   131,     0,    54,
   195,   122,   131,     0,    46,   195,   133,   131,     0,    36,
   195,   137,   131,     0,    29,   195,   122,    47,   122,   131,
     0,    29,   195,   122,   131,     0,     0,   189,   136,   133,
     0,   134,   106,   135,   107,   136,   133,     0,    16,   195,
     0,   155,     0,   155,   114,   189,     0,   155,   135,     0,
   155,   114,   189,   135,     0,     0,    22,   188,     0,     0,
   189,   137,     0,   155,   114,   189,   137,     0,    87,     0,
    88,     0,    91,     0,    94,     0,    95,     0,    96,     0,
   122,    47,   195,   122,     0,   122,     0,     0,   124,   145,
     0,   194,     0,   194,   124,   146,     0,     0,    24,   195,
   161,     0,     0,    41,   144,     0,   150,   195,   106,   152,
   153,   107,   195,     0,   151,   195,   106,   152,   153,   107,
   195,     0,     9,     0,    67,     0,    64,     0,    39,     0,
    16,     0,     0,   124,   152,     0,   154,   114,   124,   152,
     0,     0,    19,     0,   188,     0,   189,     0,   192,     0,
    66,     0,    63,     0,    38,     0,   188,     0,   192,     0,
   195,   122,    61,   144,   157,   195,     0,    33,   156,     0,
    32,   158,     0,    31,   144,    35,     0,    35,     0,   195,
   122,    61,   195,   144,   159,   195,     0,   195,   122,    52,
   160,   159,   195,     0,    33,   156,     0,    32,   158,     0,
    31,   144,    35,     0,    35,     0,   162,     0,   162,    18,
   160,     0,   162,    34,   160,     0,   162,     0,   162,    18,
   161,     0,   163,    61,   144,     0,   164,     0,   164,    86,
   194,   122,     0,   164,    86,   194,   122,    47,   122,     0,
   164,    83,   195,   164,     0,   164,    92,   195,   164,     0,
   126,     0,   126,    93,   195,   125,     0,    26,   195,   166,
   167,   172,    35,   195,     0,   124,     0,   194,     0,     0,
   168,   167,     0,    42,   195,   124,   145,     0,    21,   195,
   170,   169,     0,    40,   195,   170,   169,     0,    56,   195,
   124,   145,     0,     0,   170,   169,     0,   171,   114,   124,
     0,   171,     0,   188,     0,   190,     0,   192,     0,    66,
     0,    63,     0,    38,     0,     0,   173,   172,     0,    50,
   195,   174,   144,    35,     0,   175,     0,   175,    83,   195,
   189,     0,   188,     0,   190,     0,    66,     0,    63,     0,
    38,     0,   176,   106,   177,   107,     0,   176,   106,   177,
    19,   107,     0,     9,     0,    16,     0,   115,   195,    16,
     0,    67,     0,    64,     0,    39,     0,   178,   177,     0,
     0,   179,   180,     0,   154,   114,   179,   180,     0,   189,
     0,   109,     0,   108,     0,    17,   195,   124,     0,     0,
   195,   183,   182,    35,   195,     0,    31,   144,     0,     0,
   184,     0,   184,    18,   183,     0,   122,    61,   195,   144,
     0,   122,    47,   122,    61,   144,     0,   186,    18,   186,
     0,   186,    18,   185,     0,   122,   194,     0,   122,    47,
   122,   194,     0,   122,   194,    61,   144,     0,   122,    47,
   122,    61,   144,     0,   144,     0,   144,    18,   187,     0,
     8,     0,    15,     0,   189,     0,   115,   195,   189,     0,
    14,     0,    11,     0,    12,     0,    10,     0,     0,     0,
    77,   195,   189,   167,   172,   197,    35,   195,     0,   198,
     0,   199,     0,   201,     0,   198,   197,     0,   199,   197,
     0,   201,   197,     0,    73,   188,    83,   200,    35,     0,
    73,   189,    83,   200,    35,     0,    73,   200,   144,    35,
     0,    72,     0,    14,     0,    74,   189,   202,    35,     0,
     0,   203,   202,     0,    42,   209,     0,   199,     0,   201,
     0,    75,   195,   189,   167,   172,   206,   205,    35,   195,
     0,   233,     0,   211,     0,   233,   205,     0,   211,   205,
     0,     0,    79,   207,     0,   208,     0,   208,   207,     0,
   188,     0,   188,   114,   124,     0,   189,     0,   189,   209,
     0,   211,     0,   211,   210,     0,     0,    76,   189,    83,
   212,   215,   230,   231,   236,    35,     0,     0,     0,    76,
   109,   213,    83,   214,   215,   230,   231,   236,    35,     0,
    76,   215,   230,   231,   236,    35,     0,   217,   189,   228,
     0,   189,   229,     0,   216,   218,     0,   217,     0,     0,
   188,   114,     0,     0,     0,   106,   219,   225,   107,   220,
   228,     0,     0,     0,   110,   221,   225,   111,   222,   228,
     0,     0,     0,   112,   223,   225,   113,   224,   228,     0,
   226,     0,   226,   227,   225,     0,   189,     0,   108,     0,
    81,     0,     0,   229,     0,    94,     0,    95,     0,     0,
     0,   232,    47,     0,   233,     0,   233,   232,     0,    78,
   188,   236,    35,     0,    78,   189,   236,    35,     0,    78,
   254,   106,   234,   107,   236,    35,     0,     0,   235,   234,
     0,   189,     0,   109,     0,   108,     0,   237,     0,   238,
     0,   238,    18,   237,     0,   194,   240,     0,    60,   195,
   239,     0,     0,    80,   144,     0,   241,   240,     0,   241,
   229,   195,   242,   230,     0,   241,    83,   244,   242,     0,
    47,   242,     0,   245,   242,     0,   239,     0,   189,     0,
   239,     0,   243,   242,     0,   190,    83,   244,     0,   244,
     0,   189,     0,   189,   229,   195,   230,     0,   246,     0,
   115,   195,   189,    83,   244,     0,   246,     0,   253,     0,
   217,   195,   253,   228,   230,     0,   253,   229,   195,   230,
     0,     0,     0,   216,   195,   106,   247,   255,   107,   248,
   228,   230,     0,     0,     0,   216,   195,   110,   249,   255,
   111,   250,   228,   230,     0,     0,     0,   216,   195,   112,
   251,   255,   113,   252,   228,   230,     0,   188,     0,   254,
   195,   106,   145,   107,     0,     9,     0,    16,     0,   236,
     0,   236,   227,   255,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   677,   679,   683,   685,   688,   690,   695,   697,   700,   702,
   705,   709,   711,   713,   715,   719,   721,   725,   732,   741,
   743,   747,   748,   752,   754,   756,   773,   795,   797,   799,
   802,   804,   806,   808,   810,   816,   818,   822,   825,   828,
   831,   833,   836,   839,   842,   845,   847,   850,   852,   854,
   856,   858,   860,   862,   864,   866,   868,   870,   872,   874,
   876,   880,   882,   885,   888,   890,   892,   894,   896,   898,
   900,   902,   904,   906,   908,   910,   912,   914,   916,   918,
   920,   922,   924,   926,   928,   930,   936,   940,   943,   957,
   981,   983,   985,   990,   992,   997,   999,  1001,  1004,  1006,
  1008,  1010,  1015,  1017,  1019,  1023,  1027,  1029,  1031,  1033,
  1037,  1039,  1043,  1045,  1047,  1052,  1056,  1060,  1064,  1068,
  1072,  1076,  1078,  1082,  1084,  1088,  1090,  1096,  1098,  1102,
  1104,  1108,  1113,  1120,  1122,  1124,  1126,  1130,  1134,  1136,
  1138,  1142,  1144,  1148,  1150,  1152,  1154,  1156,  1158,  1162,
  1164,  1168,  1172,  1174,  1176,  1178,  1182,  1186,  1190,  1192,
  1194,  1196,  1200,  1202,  1204,  1208,  1210,  1214,  1218,  1220,
  1223,  1227,  1229,  1231,  1233,  1239,  1244,  1246,  1251,  1253,
  1257,  1259,  1261,  1263,  1267,  1269,  1273,  1275,  1279,  1281,
  1283,  1285,  1287,  1289,  1293,  1295,  1299,  1303,  1305,  1309,
  1311,  1313,  1315,  1317,  1319,  1321,  1325,  1327,  1329,  1331,
  1333,  1335,  1339,  1341,  1345,  1347,  1351,  1353,  1355,  1360,
  1362,  1366,  1370,  1372,  1376,  1378,  1382,  1384,  1388,  1390,
  1394,  1398,  1400,  1403,  1407,  1409,  1413,  1417,  1421,  1423,
  1427,  1431,  1433,  1437,  1441,  1445,  1455,  1463,  1465,  1467,
  1469,  1471,  1473,  1477,  1479,  1483,  1487,  1489,  1493,  1497,
  1499,  1503,  1505,  1507,  1513,  1521,  1523,  1525,  1527,  1531,
  1533,  1537,  1539,  1543,  1545,  1549,  1551,  1555,  1557,  1561,
  1563,  1565,  1566,  1567,  1569,  1573,  1575,  1577,  1581,  1582,
  1585,  1589,  1590,  1590,  1591,  1592,  1592,  1593,  1594,  1594,
  1597,  1599,  1603,  1604,  1607,  1611,  1612,  1615,  1616,  1619,
  1627,  1629,  1633,  1635,  1639,  1641,  1643,  1647,  1649,  1653,
  1655,  1657,  1661,  1665,  1667,  1671,  1680,  1684,  1686,  1690,
  1692,  1702,  1707,  1714,  1716,  1720,  1724,  1726,  1730,  1732,
  1736,  1738,  1744,  1748,  1751,  1756,  1758,  1762,  1766,  1767,
  1768,  1770,  1771,  1772,  1774,  1775,  1776,  1780,  1782,  1786,
  1788,  1793,  1795
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
"T_TRUE_LABEL","T_try","T_unit","T_UNIT_LABEL","T_loop","T_for","T_do","T_ENDOFFILE",
"T_REGEX","T_lex","T_mode","T_parser","T_prod","T_scanner","T_syn","T_token",
"T_REDUCE","T_SEP","T_ITER","'='","T_OOASSIGN","T_orelse","T_andthen","T_COMPARE",
"T_FDCOMPARE","T_LMACRO","T_RMACRO","T_FDIN","'|'","'#'","T_ADD","T_FDMUL","T_OTHERMUL",
"','","'~'","'.'","'^'","'@'","T_DEREFF","'+'","'-'","';'","'('","')'","'_'",
"'$'","'['","']'","'{'","'}'","':'","'!'","file","queries","queries1","directive",
"switchList","switch","sequence","optByPhrase","phrase","hashes","phrase2","iterators",
"iterator","optIteratorStep","procFlags","optFunctorDescriptorList","functorDescriptorList",
"importDecls","variableLabel","featureList","optImportAt","exportDecls","compare",
"fdCompare","fdIn","add","fdMul","otherMul","inSequence","phraseList","fixedListArgs",
"optCatch","optFinally","record","recordAtomLabel","recordVarLabel","recordArguments",
"optDots","feature","featureNoVar","ifMain","ifRest","caseMain","caseRest","elseOfList",
"caseClauseList","caseClause","sideCondition","pattern","class","phraseOpt",
"classDescriptorList","classDescriptor","attrFeatList","attrFeat","attrFeatFeature",
"methList","meth","methHead","methHead1","methHeadLabel","methFormals","methFormal",
"methFormalTerm","methFormalOptDefault","condMain","condElse","condClauseList",
"condClause","orClauseList","orClause","choiceClauseList","atom","nakedVariable",
"variable","string","int","float","thisCoord","coord","scannerSpecification",
"scannerRules","lexAbbrev","lexRule","regex","modeClause","modeDescrs","modeDescr",
"parserSpecification","parserRules","tokenClause","tokenList","tokenDecl","modeFromList",
"prodClauseList","prodClause","@1","@2","@3","prodHeadRest","prodName","prodNameAtom",
"prodKey","@4","@5","@6","@7","@8","@9","prodParams","prodParam","separatorOp",
"optTerminatorOp","terminatorOp","prodMakeKey","localRules","localRulesSub",
"synClause","synParams","synParam","synAlt","synSeqs","synSeq","optSynAction",
"nonEmptySeq","synVariable","synPrims","synPrim","synPrimNoAssign","synPrimNoVar",
"synPrimNoVarNoAssign","@10","@11","@12","@13","@14","@15","synInstTerm","synLabel",
"synProdCallParams", NULL
};
#endif

static const short yyr1[] = {     0,
   116,   116,   117,   117,   117,   117,   118,   118,   118,   118,
   118,   119,   119,   119,   119,   120,   120,   121,   121,   122,
   122,   123,   123,   124,   124,   124,   124,   124,   124,   124,
   124,   124,   124,   124,   124,   125,   125,   126,   126,   126,
   126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   126,   126,   126,   127,   127,   128,   128,
   129,   130,   130,   131,   131,   132,   132,   132,   132,   132,
   132,   132,   133,   133,   133,   134,   135,   135,   135,   135,
   136,   136,   137,   137,   137,   138,   139,   140,   141,   142,
   143,   144,   144,   145,   145,   146,   146,   147,   147,   148,
   148,   149,   149,   150,   150,   150,   150,   151,   152,   152,
   152,   153,   153,   154,   154,   154,   154,   154,   154,   155,
   155,   156,   157,   157,   157,   157,   158,   158,   159,   159,
   159,   159,   160,   160,   160,   161,   161,   162,   163,   163,
   163,   164,   164,   164,   164,   165,   166,   166,   167,   167,
   168,   168,   168,   168,   169,   169,   170,   170,   171,   171,
   171,   171,   171,   171,   172,   172,   173,   174,   174,   175,
   175,   175,   175,   175,   175,   175,   176,   176,   176,   176,
   176,   176,   177,   177,   178,   178,   179,   179,   179,   180,
   180,   181,   182,   182,   183,   183,   184,   184,   185,   185,
   186,   186,   186,   186,   187,   187,   188,   189,   190,   190,
   191,   192,   192,   193,   194,   195,   196,   197,   197,   197,
   197,   197,   197,   198,   198,   199,   200,   200,   201,   202,
   202,   203,   203,   203,   204,   205,   205,   205,   205,   206,
   206,   207,   207,   208,   208,   209,   209,   210,   210,   212,
   211,   213,   214,   211,   211,   215,   215,   215,   216,   216,
   217,   219,   220,   218,   221,   222,   218,   223,   224,   218,
   225,   225,   226,   226,   227,   228,   228,   229,   229,   230,
   231,   231,   232,   232,   233,   233,   233,   234,   234,   235,
   235,   235,   236,   237,   237,   238,   238,   239,   239,   240,
   240,   240,   240,   240,   240,   241,   242,   242,   243,   243,
   244,   244,   244,   245,   245,   246,   246,   246,   247,   248,
   246,   249,   250,   246,   251,   252,   246,   253,   253,   254,
   254,   255,   255
};

static const short yyr2[] = {     0,
     2,     1,     2,     2,     1,     3,     2,     6,     6,     5,
     0,     2,     1,     1,     1,     0,     2,     2,     2,     1,
     2,     0,     2,     4,     4,     8,     6,     4,     4,     4,
     4,     4,     4,     1,     4,     1,     3,     4,     4,     4,
     4,     3,     4,     2,     4,     3,     3,     3,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     6,     6,    10,    10,     6,     1,     6,     2,     2,     5,
     7,     5,     7,     5,     1,     1,     5,     2,     5,     5,
     5,     1,     1,     5,     5,     7,     2,     1,     8,     6,
     1,     0,     2,     0,     1,     4,     6,     4,     4,     4,
     6,     4,     0,     3,     6,     2,     1,     3,     2,     4,
     0,     2,     0,     2,     4,     1,     1,     1,     1,     1,
     1,     4,     1,     0,     2,     1,     3,     0,     3,     0,
     2,     7,     7,     1,     1,     1,     1,     1,     0,     2,
     4,     0,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     6,     2,     2,     3,     1,     7,     6,     2,     2,
     3,     1,     1,     3,     3,     1,     3,     3,     1,     4,
     6,     4,     4,     1,     4,     7,     1,     1,     0,     2,
     4,     4,     4,     4,     0,     2,     3,     1,     1,     1,
     1,     1,     1,     1,     0,     2,     5,     1,     4,     1,
     1,     1,     1,     1,     4,     5,     1,     1,     3,     1,
     1,     1,     2,     0,     2,     4,     1,     1,     1,     3,
     0,     5,     2,     0,     1,     3,     4,     5,     3,     3,
     2,     4,     4,     5,     1,     3,     1,     1,     1,     3,
     1,     1,     1,     1,     0,     0,     8,     1,     1,     1,
     2,     2,     2,     5,     5,     4,     1,     1,     4,     0,
     2,     2,     1,     1,     9,     1,     1,     2,     2,     0,
     2,     1,     2,     1,     3,     1,     2,     1,     2,     0,
     9,     0,     0,    10,     6,     3,     2,     2,     1,     0,
     2,     0,     0,     6,     0,     0,     6,     0,     0,     6,
     1,     3,     1,     1,     1,     0,     1,     1,     1,     0,
     0,     2,     1,     2,     4,     4,     7,     0,     2,     1,
     1,     1,     1,     1,     3,     2,     3,     0,     2,     2,
     5,     4,     2,     2,     1,     1,     1,     2,     3,     1,
     1,     4,     1,     5,     1,     1,     5,     4,     0,     0,
     9,     0,     0,     9,     0,     0,     9,     1,     5,     1,
     1,     1,     3
};

static const short yydefact[] = {     0,
     2,    16,    13,    14,    15,   237,   134,   244,   242,   243,
   241,   238,   138,   246,   246,   246,   246,   246,   246,    76,
    54,   137,   246,   246,   246,   246,   246,   246,   246,   246,
   246,    55,    75,   246,    53,   136,   246,    52,   135,   246,
   246,   246,   290,   246,   246,   246,   246,   246,     0,    51,
    56,   246,   246,   246,     0,     5,   245,    11,    20,    34,
    60,   246,   246,    66,    49,   239,    50,    57,    58,    59,
     0,    82,    83,    11,   278,     0,     0,    12,    16,    69,
     0,     0,   245,    78,     0,     0,     0,    92,   245,    68,
     0,     0,     0,     0,     0,    92,     0,     0,     0,     0,
     0,     0,   282,     0,     0,   310,     0,   289,     0,   124,
     0,     0,     0,   123,     0,     0,     0,     0,     1,     7,
     3,   246,   246,   246,   246,   246,   116,   117,   118,   246,
    21,   246,   246,   246,    44,   246,   119,   120,   121,   246,
   246,   246,   246,   246,   246,     0,     0,   246,   246,   246,
   246,   246,    11,     4,   279,    18,    19,    17,     0,   235,
     0,   177,   179,   178,     0,   224,   225,   245,   245,     0,
     0,     0,    92,    94,     0,     0,    20,     0,     0,     0,
     0,     0,     0,   128,     0,     0,    88,     0,   179,     0,
   291,   280,   308,   309,   287,   311,   292,   295,   298,   288,
   306,   179,   124,     0,    42,    46,    47,   246,    48,   245,
   124,   240,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   139,   139,
     0,   113,   103,     0,   103,     6,     0,   246,     0,   246,
   246,   246,   246,   246,   195,   179,     0,   246,     0,     0,
     0,   245,    11,     0,   231,   246,     0,     0,    93,     0,
    95,     0,     0,     0,   246,   246,   246,     0,   246,   246,
   246,   130,   246,     0,    87,   246,   195,   283,   290,     0,
   245,     0,   313,     0,     0,     0,   286,   307,   195,   125,
   246,     0,     0,   126,     0,    22,    24,    25,    28,    29,
    33,    30,    31,    32,    35,    36,    41,    43,    45,    38,
    39,    40,    54,    53,    52,   139,   142,     0,    49,   239,
    58,   142,    94,    94,     0,   150,   113,   151,   246,    94,
     0,   111,    94,    94,   174,     0,   163,     0,   169,     0,
   236,    81,     0,     0,     0,     0,   246,     0,   195,   180,
     0,     0,   223,   246,   226,    11,    11,    10,   245,     0,
    80,   230,   229,   124,   246,     0,     0,     0,    70,    77,
    79,   124,    74,    72,     0,     0,     0,    84,     0,     0,
   270,   290,     0,   310,   360,   361,   245,   245,     0,   246,
   328,     0,   323,   324,   312,   314,   304,   303,     0,   301,
     0,     0,     0,    85,   122,   246,   245,   246,     0,     0,
   246,     0,   140,   143,     0,     0,     0,     0,   102,   100,
     0,   114,   106,    99,     0,     0,   103,     0,    98,    96,
   246,     0,   246,   246,   162,   246,     0,     0,     0,   246,
   245,   246,     0,   194,   193,   192,   185,   188,   189,   190,
   191,   185,   124,   124,     0,   246,   196,     0,   227,   222,
     9,     8,     0,   232,   233,     0,    65,     0,   246,   246,
   156,   246,    67,   246,     0,   129,   166,   131,   246,   246,
    22,     0,     0,   310,   311,     0,     0,   318,   328,   328,
     0,   246,   358,   336,   246,   246,   335,   326,   328,   328,
   345,   346,   246,   285,   245,   293,   305,     0,   296,   299,
     0,     0,     0,   248,   249,   250,    61,   127,    62,    22,
    23,    27,    37,   246,   139,   246,    94,   113,     0,   107,
   112,   104,    94,     0,     0,   160,   159,   158,   164,   165,
   168,     0,     0,     0,   246,   182,   185,     0,   183,   181,
   184,   207,   208,   204,   212,   203,   211,   202,   210,   246,
     0,   198,     0,   200,   201,   176,   228,   234,     0,     0,
   154,   153,   152,    71,     0,     0,    73,    86,     0,    91,
   246,   274,   271,   272,     0,   267,   266,   311,   245,   315,
   316,   322,   321,   320,     0,   318,   327,   341,     0,   337,
   333,   328,   340,   343,   329,     0,     0,     0,   290,   246,
   330,   334,   246,     0,   325,   306,   302,   306,   306,   258,
   257,     0,     0,     0,   260,   246,   251,   252,   253,   246,
   132,   141,   133,   101,   115,   111,     0,   109,    97,   175,
   161,   172,   170,   173,   157,   186,   187,     0,     0,   246,
   214,     0,   155,     0,   167,    22,    90,     0,   273,   246,
   269,   268,   245,     0,   245,   319,   246,   290,   338,     0,
   349,   352,   355,   358,   306,   341,   328,   328,   310,   124,
   294,   297,   300,     0,     0,     0,     0,     0,   263,   264,
     0,   260,   247,    26,   103,   108,     0,   209,   197,     0,
   149,   148,   147,   219,   218,     0,     0,   214,   221,   144,
   217,   146,   246,   246,   246,   275,   265,     0,   281,     0,
   310,   339,   290,   245,   245,   245,   310,   332,   310,   348,
     0,     0,     0,   256,   276,   262,   259,   261,   105,   110,
   171,   199,     0,     0,   205,   213,   246,   215,    64,    63,
    89,   284,   317,   342,   344,   362,     0,     0,     0,   347,
   331,   359,   254,   255,   277,   221,   217,   206,     0,   245,
   350,   353,   356,   216,   220,   363,   306,   306,   306,   310,
   310,   310,   351,   354,   357,     0,     0,     0
};

static const short yydefgoto[] = {   786,
    55,    56,    57,    78,    79,   114,   580,    59,   305,    60,
   186,   187,   581,   172,   260,   261,   330,   331,   529,   427,
   324,   132,   133,   134,   143,   144,   145,   160,   204,   293,
   272,   377,    61,    62,    63,   317,   415,   318,   325,    90,
   472,    80,   436,   336,   476,   337,   338,   339,    64,   163,
   245,   246,   546,   547,   448,   348,   349,   561,   562,   563,
   707,   708,   709,   748,    84,   250,   166,   167,   170,   171,
   161,    65,    66,    67,    68,    69,    70,   391,    81,    72,
   513,   514,   515,   624,   516,   691,   692,    73,   585,   483,
   583,   584,   736,    74,    75,   279,   190,   382,   106,   495,
   496,   200,   284,   616,   285,   618,   286,   619,   399,   400,
   508,   287,   288,   196,   281,   282,   283,   595,   596,   756,
   393,   394,   600,   498,   499,   601,   602,   603,   500,   604,
   724,   777,   725,   778,   726,   779,   502,   503,   757
};

static const short yypact[] = {  1070,
-32768,   209,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,    63,-32768,-32768,-32768,-32768,-32768,  1613,-32768,
-32768,-32768,-32768,-32768,     2,-32768,  1181,   355,  1397,   420,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   739,-32768,-32768,   355,     5,    91,    97,-32768,   209,-32768,
  1613,  1613,  1613,-32768,  1613,  1613,  1613,   104,  1613,-32768,
  1613,  1613,  1613,  1613,  1613,   104,  1613,  1613,  1613,  1613,
   105,   105,-32768,    94,   203,-32768,   239,   105,   105,  1613,
  1613,  1613,  1613,   165,   117,  1613,  1613,   105,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   128,   134,-32768,-32768,-32768,
-32768,-32768,   355,-32768,-32768,-32768,-32768,-32768,   214,   227,
   212,   800,   317,-32768,    28,   241,   274,   282,   288,   272,
   322,   231,   104,   739,   291,   316,  1289,   334,   337,   339,
   277,   357,   360,   391,   394,   215,-32768,   373,   317,   340,
-32768,-32768,-32768,-32768,-32768,   348,-32768,-32768,-32768,-32768,
   260,   317,  1397,   342,   182,-32768,-32768,-32768,-32768,   800,
  1397,-32768,  1613,  1613,  1613,  1613,  1613,  1613,  1613,  1613,
  1613,  1613,  1613,  1613,  1613,  1613,  1613,  1613,  1721,  1721,
  1613,   459,   385,  1613,   385,-32768,  1613,-32768,  1613,-32768,
-32768,-32768,-32768,-32768,   401,   317,  1613,-32768,  1613,   409,
  1613,  1613,   355,  1613,   397,-32768,  1613,  1613,-32768,   428,
-32768,  1613,  1613,  1613,-32768,-32768,-32768,  1613,-32768,-32768,
-32768,   427,-32768,  1613,-32768,-32768,   401,-32768,   287,   434,
   412,   429,   348,    51,    51,    51,-32768,-32768,   401,-32768,
-32768,  1613,   371,  1613,   374,   194,   822,   831,   674,   599,
   392,   505,   362,   362,-32768,   717,   138,-32768,-32768,   223,
   138,   138,   375,   377,   379,  1505,   484,   396,   407,   419,
   430,   484,   528,   739,   438,-32768,   459,-32768,-32768,   739,
   406,   520,   635,   739,   732,   494,    54,   487,    -1,  1613,
-32768,-32768,   262,   262,  1613,  1613,-32768,   521,   401,-32768,
   497,  1613,-32768,-32768,-32768,   355,   355,-32768,   504,  1613,
-32768,-32768,   322,  1397,-32768,   575,   542,   544,-32768,-32768,
-32768,  1397,-32768,-32768,  1613,  1613,   549,-32768,   550,  1613,
   511,   287,   260,-32768,-32768,-32768,   412,   412,   488,-32768,
   742,   564,-32768,   587,-32768,-32768,-32768,-32768,   509,   538,
   510,   507,   388,-32768,-32768,-32768,   800,-32768,  1613,  1613,
-32768,  1613,-32768,-32768,   515,  1613,   516,  1613,-32768,-32768,
   105,-32768,-32768,-32768,   423,   104,   385,  1613,-32768,-32768,
-32768,  1613,-32768,-32768,-32768,-32768,  1613,  1613,  1613,-32768,
-32768,-32768,   494,-32768,-32768,-32768,   262,   513,-32768,-32768,
-32768,   262,  1397,  1397,   609,-32768,-32768,  1613,-32768,-32768,
-32768,-32768,  1613,-32768,-32768,   519,-32768,  1613,-32768,-32768,
-32768,-32768,-32768,-32768,   524,-32768,   610,-32768,-32768,-32768,
   626,   104,    20,-32768,   348,   603,   605,    79,   554,   389,
  1613,-32768,    94,-32768,-32768,   278,-32768,-32768,    84,   389,
-32768,   260,-32768,-32768,   412,-32768,-32768,    51,-32768,-32768,
    69,   105,   607,   388,   388,   388,-32768,-32768,-32768,   271,
   473,-32768,-32768,-32768,  1505,-32768,   739,   459,   537,    52,
-32768,-32768,   739,  1613,   614,-32768,-32768,-32768,-32768,-32768,
-32768,  1613,  1613,  1613,-32768,-32768,   262,  1613,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  1613,   568,   548,-32768,-32768,-32768,-32768,-32768,  1613,   620,
-32768,-32768,-32768,-32768,  1613,  1613,-32768,-32768,  1613,-32768,
-32768,   545,-32768,   104,   623,    20,    20,   348,   412,-32768,
-32768,-32768,-32768,-32768,   555,    79,-32768,   232,   578,-32768,
-32768,   389,-32768,-32768,-32768,   105,   312,   398,   434,-32768,
-32768,-32768,-32768,   571,-32768,   260,-32768,   260,   260,-32768,
-32768,   597,   613,  1613,   158,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   520,   105,-32768,-32768,-32768,
-32768,   188,   641,   617,-32768,-32768,   800,   463,   666,-32768,
   645,   670,-32768,   671,-32768,   271,-32768,  1613,-32768,-32768,
-32768,-32768,   412,   677,   412,-32768,-32768,   434,-32768,   644,
-32768,-32768,-32768,-32768,   260,   260,   389,   389,-32768,  1613,
-32768,-32768,-32768,   197,   197,   694,   105,   197,-32768,-32768,
   699,   158,-32768,-32768,   385,   423,  1613,-32768,-32768,   105,
-32768,-32768,-32768,-32768,-32768,   556,    35,   645,   718,-32768,
   419,-32768,-32768,-32768,-32768,   800,-32768,   702,-32768,   704,
-32768,-32768,   434,   412,   412,   412,-32768,-32768,-32768,-32768,
   633,   708,   712,-32768,   105,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   107,   648,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   538,   649,   637,   651,-32768,
-32768,-32768,-32768,-32768,-32768,   718,-32768,-32768,  1613,   412,
-32768,-32768,-32768,-32768,   800,-32768,   260,   260,   260,-32768,
-32768,-32768,-32768,-32768,-32768,   767,   770,-32768
};

static const short yypgoto[] = {-32768,
   714,   -40,-32768,   703,-32768,    90,  -276,   320,  -366,   491,
-32768,   595,   127,     7,  -265,   713,  -229,-32768,  -490,   154,
  -306,-32768,-32768,-32768,-32768,-32768,-32768,   211,  -175,   384,
-32768,-32768,-32768,-32768,-32768,  -177,   470,  -412,  -390,  -373,
-32768,  -346,   352,    59,   218,  -334,-32768,  -358,-32768,   710,
     3,-32768,  -395,   163,-32768,  -203,-32768,-32768,-32768,-32768,
    92,-32768,    61,    36,-32768,-32768,   557,-32768,   -53,   558,
   566,   351,   101,  -299,-32768,   216,-32768,    49,   -15,-32768,
  -104,-32768,  -549,  -138,  -509,   115,-32768,-32768,   -25,-32768,
   234,-32768,    85,   744,  -432,-32768,-32768,-32768,  -217,   -36,
   -26,-32768,-32768,-32768,-32768,-32768,-32768,-32768,  -262,-32768,
    65,  -514,  -100,  -227,  -418,   540,  -413,   238,-32768,  -225,
   325,-32768,  -339,   338,-32768,  -457,-32768,  -554,-32768,  -341,
-32768,-32768,-32768,-32768,-32768,-32768,   230,   559,  -646
};


#define	YYLAST		1836


static const short yytable[] = {    82,
    83,    85,    86,    87,   195,   334,   107,    88,    89,    91,
    92,    93,    94,    95,    96,    97,   108,   121,    98,   411,
   422,    99,   401,   402,   100,   101,   102,   290,   109,   110,
   111,   112,   113,   154,   530,   295,   116,   117,   118,   638,
   477,   180,   612,   450,   450,   523,   146,   147,    71,   501,
   586,   497,   322,   744,   677,   392,   549,   419,   420,     6,
   537,   384,     9,    10,   424,    12,   589,   429,   430,   587,
     6,   437,   119,   381,   247,   689,     6,    12,   758,   759,
    43,   440,   620,    12,   441,   403,   536,   438,   248,    58,
   442,     6,   385,    12,   156,    43,   572,   280,    12,   386,
   157,   681,   181,   682,   683,    71,   213,   214,   215,   216,
   217,     6,   236,   722,   218,   690,   219,   220,   221,    12,
   222,    12,   571,   776,   223,   224,   225,   226,   227,   228,
   490,   164,   231,   232,   233,   234,   235,   164,   413,   530,
   621,   745,   689,   105,   669,   457,    58,   450,   131,   597,
   135,   646,   450,   586,   586,   565,   485,   501,   397,   497,
   727,   486,   487,   491,   484,   637,   609,   640,   755,   663,
   159,   103,   587,   587,   165,   168,   169,   193,   194,   259,
   175,   176,   690,   642,   169,   644,   592,   593,   466,  -290,
   599,   277,   292,  -290,   135,  -290,   475,   532,   492,   687,
   599,   188,   189,   362,   289,   740,   135,   191,   201,   202,
   620,   208,   358,   409,   704,   705,   253,   255,   212,   728,
   729,   635,   340,   209,   342,   343,   344,   345,   346,    12,
   688,   512,   352,   229,   140,   135,   141,   142,   706,   230,
   361,   477,   107,   630,   239,   617,   240,   450,   350,   369,
   370,   371,   108,   373,   374,   375,   588,   378,   294,   115,
   380,   634,   780,   781,   782,   237,   131,   639,   621,     6,
   440,   249,     9,    10,   238,   404,    12,   550,   551,   442,
   141,   142,   195,   135,   274,   192,   188,   137,   138,   139,
   140,   251,   141,   142,     6,   706,   193,   194,   410,   444,
   357,    12,   599,   178,   179,   530,   256,   182,   183,   184,
   185,    76,    77,   423,  -239,   461,   462,   138,   139,   140,
   323,   141,   142,   333,   445,   193,   194,   446,   252,   320,
   320,   455,   327,   332,   254,   332,   351,   241,   460,   257,
   165,   356,   258,   359,   197,   107,   169,   632,   198,   467,
   199,   262,   367,   193,   194,   108,   242,     2,   243,     3,
     4,     5,   263,   664,   137,   138,   139,   140,   265,   141,
   142,   266,   244,   267,   489,   410,    54,   599,   599,   383,
   388,   405,    18,  -289,   398,   398,   398,  -289,   268,  -289,
   517,   269,   519,   104,   270,   522,     6,   385,   610,    12,
   329,   613,   162,    12,   386,     6,   385,   464,   162,   627,
   628,   629,   177,   386,   271,   534,   320,   671,    91,   276,
   538,   672,   278,   673,   542,   280,   544,   327,   273,   203,
     6,   291,   135,     9,    10,   210,   211,   718,   173,   720,
   566,     6,   385,   354,   321,   321,   173,   328,    12,   386,
   347,   730,-32768,   130,    91,   294,   573,   360,   574,   353,
   511,   512,   365,   577,   578,   739,     6,   376,   491,     9,
    10,   390,   366,    12,   368,   395,   606,    12,   698,   607,
   608,   406,   383,   130,   379,   135,   408,   614,  -149,   543,
  -148,   494,  -147,   754,  -290,   539,   540,   667,  -290,   760,
  -290,   761,   414,    54,   731,   447,   452,   527,   631,   416,
   633,   425,   136,   137,   138,   139,   140,   533,   141,   142,
  -144,   528,   203,   173,   432,   433,   434,   332,   435,   645,
   203,   321,  -145,   297,   298,   299,   300,   301,   302,   303,
   304,   426,   328,  -146,   648,   732,   733,   439,   316,   316,
   443,   421,   783,   784,   785,   456,   148,   458,   451,   451,
   661,   662,   459,   149,   463,   657,   137,   138,   139,   140,
   465,   141,   142,   150,   418,   667,   473,   364,   474,   319,
   319,   151,   326,   479,   480,   152,   478,   372,   594,   482,
   598,-32768,-32768,   488,   678,   129,   130,   679,   504,   494,
   598,   205,   206,   207,   505,   468,   469,   470,   398,   471,
   693,   623,   625,   407,   694,   506,     6,   552,   507,   510,
   509,   524,   526,    12,   553,   320,   548,   576,   327,   104,
   387,   569,   643,   491,   700,   316,   575,   590,   135,   591,
   328,   626,   535,   636,   717,   579,   554,   555,   641,   541,
   650,   721,     6,   651,   653,     9,    10,   660,   658,    12,
   668,   665,   451,   148,   453,   454,   319,   451,   567,   743,
   149,   556,   557,   568,   558,   559,   680,   326,   570,   684,
   150,   428,   701,   203,   126,   127,   128,   697,   151,   129,
   130,   203,   152,   449,   449,   685,   594,   749,   750,   751,
   699,   605,   598,   296,   713,   714,   670,   702,   442,   676,
   703,   719,   306,   307,   308,   309,   310,   311,   312,   137,
   138,   139,   140,   560,   141,   142,   723,   335,   734,   135,
   410,   769,   104,   737,   747,   525,   752,   696,   753,   762,
   321,   493,   763,   328,   135,   328,   764,   772,   212,     6,
   385,   711,   704,   705,   768,   771,    12,   386,   125,   126,
   127,   128,   451,   773,   129,   130,   787,   148,   676,   788,
   120,   649,   203,   203,   149,   326,   531,   598,   598,   652,
   275,   158,   715,   153,   150,   654,   741,   735,   490,   695,
   518,   417,   151,   655,   545,   332,   152,   449,   174,   746,
   742,   774,   449,   766,   341,   564,   738,   355,   711,   412,
   137,   138,   139,   140,   363,   141,   142,   659,   155,   765,
   770,   491,   396,   676,   431,   137,   138,   139,   140,   615,
   141,   142,   582,   666,   686,   735,   611,   675,   389,     0,
   493,     0,     0,   767,   316,     0,     0,  -290,     0,   493,
   493,  -290,     0,  -290,     0,     0,   492,     0,     0,     0,
     0,   622,     0,     0,     0,   335,   712,   647,     0,     0,
   481,     0,     0,     0,     0,   319,     0,     0,   326,     0,
   326,   122,   123,   124,   125,   126,   127,   128,     0,     0,
   129,   130,     0,     0,     0,     0,     0,   449,     0,   520,
   521,     0,   306,     0,   123,   124,   125,   126,   127,   128,
     0,   328,   129,   130,   124,   125,   126,   127,   128,     0,
     0,   129,   130,   712,     0,     0,     0,   335,   335,     0,
     0,     0,     0,     0,   582,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,   493,     0,     0,     0,     0,     0,   674,   493,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   716,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   203,
     0,   710,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   493,     0,
     0,     0,     0,     0,   306,     0,     0,   493,   493,     0,
     0,     0,   335,     0,   335,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   326,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   710,     0,
     0,     0,     0,     0,     0,     0,   335,     0,     0,   656,
     1,     0,     2,   493,     3,     4,     5,     6,     7,     8,
     9,    10,     0,    11,    12,    13,     0,     0,   775,     0,
     0,     0,    14,     0,    15,    16,    17,    18,  -245,    19,
     0,     0,     0,     0,     0,  -245,    20,    21,    22,     0,
     0,     0,    23,    24,    25,  -245,     0,    26,    27,     0,
    28,     0,    29,  -245,    30,     0,    31,  -245,    32,    33,
     0,    34,    35,    36,    37,    38,    39,    40,    41,     0,
   -11,     0,     0,     0,    42,    43,    44,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    45,     0,
     0,     0,     0,     0,     0,     0,     0,    46,     0,     0,
    47,    48,     0,     0,     0,    49,     0,    50,    51,    52,
     0,    53,     0,     2,    54,     3,     4,     5,     6,     7,
     8,     9,    10,     0,    11,    12,    13,     0,     0,     0,
     0,     0,     0,    14,     0,    15,    16,    17,    18,     0,
    19,     0,     0,     0,     0,     0,     0,    20,    21,    22,
     0,     0,     0,    23,    24,    25,     0,     0,    26,    27,
     0,    28,     0,    29,     0,    30,     0,    31,     0,    32,
    33,     0,    34,    35,    36,    37,    38,    39,    40,    41,
     0,   -11,     0,     0,     0,    42,    43,    44,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    45,
     0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
     0,    47,    48,     0,     0,     0,    49,     0,    50,    51,
    52,     0,    53,     0,     0,    54,     6,     7,     8,     9,
    10,     0,    11,    12,    13,     0,     0,     0,     0,     0,
     0,    14,     0,    15,    16,    17,     0,     0,    19,     0,
     0,     0,     0,     0,     0,    20,    21,    22,     0,     0,
     0,    23,    24,    25,     0,     0,    26,    27,     0,    28,
     0,    29,     0,    30,     0,    31,     0,    32,    33,   264,
    34,    35,    36,    37,    38,    39,    40,    41,     0,     0,
     0,     0,     0,    42,     0,    44,     0,     0,     0,     0,
   122,   123,   124,   125,   126,   127,   128,    45,     0,   129,
   130,     0,     0,     0,     0,     0,    46,     0,     0,    47,
    48,     0,     0,     0,    49,     0,    50,    51,    52,     0,
    53,     0,     0,    54,     6,     7,     8,     9,    10,     0,
    11,    12,    13,     0,     0,     0,     0,     0,     0,    14,
     0,    15,    16,    17,     0,     0,    19,     0,     0,     0,
     0,     0,     0,    20,    21,    22,     0,     0,     0,    23,
    24,    25,     0,     0,    26,    27,     0,    28,     0,    29,
     0,    30,     0,    31,     0,    32,    33,     0,    34,    35,
    36,    37,    38,    39,    40,    41,     0,     0,     0,     0,
     0,    42,     0,    44,     0,     0,     0,     0,   122,   123,
   124,   125,   126,   127,   128,    45,     0,   129,   130,     0,
     0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
     0,     0,    49,     0,    50,    51,    52,     0,    53,     0,
     0,    54,     6,     7,     8,     9,    10,     0,    11,    12,
    13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,   313,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,     0,    34,   314,    36,    37,
   315,    39,    40,    41,     0,     0,     0,     0,     0,    42,
     0,    44,     0,     0,     0,     0,   122,   123,   124,   125,
   126,   127,   128,    45,     0,   129,   130,     0,     0,     0,
     0,     0,    46,     0,     0,    47,    48,     0,     0,     0,
    49,     0,    50,    51,    52,     0,    53,     0,     0,    54,
     6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
     0,     0,     0,     0,     0,    14,     0,    15,    16,    17,
     0,     0,    19,     0,     0,     0,     0,     0,     0,    20,
    21,    22,     0,     0,     0,    23,    24,    25,     0,     0,
    26,    27,     0,    28,     0,    29,     0,    30,     0,    31,
     0,    32,    33,     0,    34,    35,    36,    37,    38,    39,
    40,    41,     0,     0,     0,     0,     0,    42,     0,    44,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
    46,     0,     0,    47,    48,     0,     0,     0,    49,     0,
    50,    51,    52,     0,    53,     0,     0,    54,     6,     7,
     8,     9,    10,     0,    11,    12,    13,     0,     0,     0,
     0,     0,     0,    14,     0,    15,    16,    17,     0,     0,
    19,     0,     0,     0,     0,     0,     0,    20,   313,    22,
     0,     0,     0,    23,    24,    25,     0,     0,    26,    27,
     0,    28,     0,    29,     0,    30,     0,    31,     0,    32,
    33,     0,    34,   314,    36,    37,   315,    39,    40,    41,
     0,     0,     0,     0,     0,    42,     0,    44,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    45,
     0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
     0,    47,    48,     0,     0,     0,    49,     0,    50,    51,
    52,     0,    53,     0,     0,    54
};

static const short yycheck[] = {    15,
    16,    17,    18,    19,   105,   235,    43,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    43,    58,    34,   296,
   327,    37,   285,   286,    40,    41,    42,   203,    44,    45,
    46,    47,    48,    74,   425,   211,    52,    53,    54,   530,
   375,    95,   500,   343,   344,   412,    62,    63,     0,   391,
   483,   391,   230,    19,   609,   281,   452,   323,   324,     8,
   434,   279,    11,    12,   330,    15,   485,   333,   334,   483,
     8,    18,    71,   277,    47,   625,     8,    15,   725,   726,
    76,    83,    14,    15,    86,   289,   433,    34,    61,     0,
    92,     8,     9,    15,     4,    76,   470,    78,    15,    16,
     4,   616,    96,   618,   619,    57,   122,   123,   124,   125,
   126,     8,   153,   668,   130,   625,   132,   133,   134,    15,
   136,    15,   469,   770,   140,   141,   142,   143,   144,   145,
    47,    83,   148,   149,   150,   151,   152,    89,   316,   530,
    72,   107,   692,    43,   602,   349,    57,   447,    59,   489,
    13,   547,   452,   586,   587,   455,   384,   499,   108,   499,
   675,   387,   388,    80,   382,   114,    83,   534,   723,   588,
    81,   109,   586,   587,    85,    86,    87,    94,    95,   173,
    91,    92,   692,   542,    95,   544,   108,   109,   364,   106,
   490,   189,   208,   110,    13,   112,   372,   427,   115,    42,
   500,   101,   102,   257,   202,   696,    13,   114,   108,   109,
    14,    47,   253,    20,   108,   109,   168,   169,   118,   677,
   678,   528,   238,   107,   240,   241,   242,   243,   244,    15,
    73,    74,   248,   106,    97,    13,    99,   100,   651,   106,
   256,   576,   279,   520,    18,   508,    35,   547,   246,   265,
   266,   267,   279,   269,   270,   271,   484,   273,   210,    49,
   276,   527,   777,   778,   779,    52,   177,   533,    72,     8,
    83,    31,    11,    12,    61,   291,    15,   453,   454,    92,
    99,   100,   383,    13,    70,    83,   186,    94,    95,    96,
    97,    18,    99,   100,     8,   708,    94,    95,   105,    38,
   252,    15,   602,    93,    94,   696,    35,    97,    98,    99,
   100,   103,   104,   329,    83,   356,   357,    95,    96,    97,
   231,    99,   100,   234,    63,    94,    95,    66,    47,   229,
   230,   347,   232,   233,    47,   235,   247,    21,   354,    18,
   251,   252,   112,   254,   106,   382,   257,   525,   110,   365,
   112,    61,   263,    94,    95,   382,    40,     3,    42,     5,
     6,     7,    47,   589,    94,    95,    96,    97,    35,    99,
   100,    35,    56,    35,   390,   105,   115,   677,   678,   279,
   280,   292,    28,   106,   284,   285,   286,   110,   112,   112,
   406,    35,   408,    43,    35,   411,     8,     9,   499,    15,
    16,   502,    83,    15,    16,     8,     9,   359,    89,   514,
   515,   516,    93,    16,    24,   431,   316,   106,   434,    47,
   436,   110,    83,   112,   440,    78,   442,   327,    35,   110,
     8,    90,    13,    11,    12,   116,   117,   663,    88,   665,
   456,     8,     9,    35,   229,   230,    96,   232,    15,    16,
    50,   679,    91,    92,   470,   407,   472,    61,   474,   249,
    73,    74,    35,   479,   480,   695,     8,    41,    80,    11,
    12,    60,   262,    15,   264,    47,   492,    15,    16,   495,
   496,   111,   382,    92,   274,    13,   113,   503,   114,   441,
   114,   391,   114,   721,   106,   437,   438,   598,   110,   727,
   112,   729,    19,   115,   680,   343,   344,   418,   524,   114,
   526,   106,    93,    94,    95,    96,    97,   428,    99,   100,
   114,   421,   203,   173,    31,    32,    33,   427,    35,   545,
   211,   316,   114,   214,   215,   216,   217,   218,   219,   220,
   221,    22,   327,   114,   560,   684,   685,    61,   229,   230,
   340,   114,   780,   781,   782,    35,    29,    61,   343,   344,
   586,   587,   352,    36,    61,   581,    94,    95,    96,    97,
   360,    99,   100,    46,    47,   676,    35,   258,    35,   229,
   230,    54,   232,    35,    35,    58,   376,   268,   488,    79,
   490,    87,    88,   106,   610,    91,    92,   613,    35,   499,
   500,   111,   112,   113,    18,    31,    32,    33,   508,    35,
   626,   511,   512,   294,   630,   107,     8,     9,    81,   113,
   111,   107,   107,    15,    16,   525,   114,    18,   528,   279,
   280,   113,   543,    80,   650,   316,   113,    35,    13,    35,
   425,    35,   432,   107,   660,    20,    38,    39,    35,   439,
    83,   667,     8,   106,    35,    11,    12,    35,   114,    15,
    83,   107,   447,    29,   345,   346,   316,   452,   458,   114,
    36,    63,    64,   463,    66,    67,   106,   327,   468,    83,
    46,    47,    38,   364,    86,    87,    88,    47,    54,    91,
    92,   372,    58,   343,   344,    83,   596,   713,   714,   715,
    35,   491,   602,   213,    35,    35,   606,    63,    92,   609,
    66,    35,   222,   223,   224,   225,   226,   227,   228,    94,
    95,    96,    97,   115,    99,   100,    83,   237,    35,    13,
   105,   747,   382,    35,    17,   416,    35,   637,    35,   107,
   525,   391,    35,   528,    13,   530,    35,   111,   648,     8,
     9,   651,   108,   109,   107,   107,    15,    16,    85,    86,
    87,    88,   547,   113,    91,    92,     0,    29,   668,     0,
    57,   561,   453,   454,    36,   425,   426,   677,   678,   569,
   186,    79,   656,    71,    46,   575,   697,   687,    47,   636,
   407,   322,    54,   576,   443,   695,    58,   447,    89,   708,
   700,   766,   452,   743,   239,   455,   692,   251,   708,    93,
    94,    95,    96,    97,   257,    99,   100,   584,    75,   735,
   756,    80,   283,   723,    93,    94,    95,    96,    97,   505,
    99,   100,   482,   596,   624,   735,   499,   608,   280,    -1,
   490,    -1,    -1,   743,   525,    -1,    -1,   106,    -1,   499,
   500,   110,    -1,   112,    -1,    -1,   115,    -1,    -1,    -1,
    -1,   511,    -1,    -1,    -1,   375,   651,   548,    -1,    -1,
   380,    -1,    -1,    -1,    -1,   525,    -1,    -1,   528,    -1,
   530,    82,    83,    84,    85,    86,    87,    88,    -1,    -1,
    91,    92,    -1,    -1,    -1,    -1,    -1,   547,    -1,   409,
   410,    -1,   412,    -1,    83,    84,    85,    86,    87,    88,
    -1,   696,    91,    92,    84,    85,    86,    87,    88,    -1,
    -1,    91,    92,   708,    -1,    -1,    -1,   437,   438,    -1,
    -1,    -1,    -1,    -1,   584,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,   602,    -1,    -1,    -1,    -1,    -1,   608,   609,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,   658,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   680,
    -1,   651,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   668,    -1,
    -1,    -1,    -1,    -1,   534,    -1,    -1,   677,   678,    -1,
    -1,    -1,   542,    -1,   544,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,   696,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   708,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,   576,    -1,    -1,   579,
     1,    -1,     3,   723,     5,     6,     7,     8,     9,    10,
    11,    12,    -1,    14,    15,    16,    -1,    -1,   769,    -1,
    -1,    -1,    23,    -1,    25,    26,    27,    28,    29,    30,
    -1,    -1,    -1,    -1,    -1,    36,    37,    38,    39,    -1,
    -1,    -1,    43,    44,    45,    46,    -1,    48,    49,    -1,
    51,    -1,    53,    54,    55,    -1,    57,    58,    59,    60,
    -1,    62,    63,    64,    65,    66,    67,    68,    69,    -1,
    71,    -1,    -1,    -1,    75,    76,    77,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
   101,   102,    -1,    -1,    -1,   106,    -1,   108,   109,   110,
    -1,   112,    -1,     3,   115,     5,     6,     7,     8,     9,
    10,    11,    12,    -1,    14,    15,    16,    -1,    -1,    -1,
    -1,    -1,    -1,    23,    -1,    25,    26,    27,    28,    -1,
    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,
    -1,    -1,    -1,    43,    44,    45,    -1,    -1,    48,    49,
    -1,    51,    -1,    53,    -1,    55,    -1,    57,    -1,    59,
    60,    -1,    62,    63,    64,    65,    66,    67,    68,    69,
    -1,    71,    -1,    -1,    -1,    75,    76,    77,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
    -1,   101,   102,    -1,    -1,    -1,   106,    -1,   108,   109,
   110,    -1,   112,    -1,    -1,   115,     8,     9,    10,    11,
    12,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,
    -1,    23,    -1,    25,    26,    27,    -1,    -1,    30,    -1,
    -1,    -1,    -1,    -1,    -1,    37,    38,    39,    -1,    -1,
    -1,    43,    44,    45,    -1,    -1,    48,    49,    -1,    51,
    -1,    53,    -1,    55,    -1,    57,    -1,    59,    60,    61,
    62,    63,    64,    65,    66,    67,    68,    69,    -1,    -1,
    -1,    -1,    -1,    75,    -1,    77,    -1,    -1,    -1,    -1,
    82,    83,    84,    85,    86,    87,    88,    89,    -1,    91,
    92,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,   101,
   102,    -1,    -1,    -1,   106,    -1,   108,   109,   110,    -1,
   112,    -1,    -1,   115,     8,     9,    10,    11,    12,    -1,
    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,
    -1,    25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,
    -1,    -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,
    44,    45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,
    -1,    55,    -1,    57,    -1,    59,    60,    -1,    62,    63,
    64,    65,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
    -1,    75,    -1,    77,    -1,    -1,    -1,    -1,    82,    83,
    84,    85,    86,    87,    88,    89,    -1,    91,    92,    -1,
    -1,    -1,    -1,    -1,    98,    -1,    -1,   101,   102,    -1,
    -1,    -1,   106,    -1,   108,   109,   110,    -1,   112,    -1,
    -1,   115,     8,     9,    10,    11,    12,    -1,    14,    15,
    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,
    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,
    -1,    57,    -1,    59,    60,    -1,    62,    63,    64,    65,
    66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    75,
    -1,    77,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
    86,    87,    88,    89,    -1,    91,    92,    -1,    -1,    -1,
    -1,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,    -1,
   106,    -1,   108,   109,   110,    -1,   112,    -1,    -1,   115,
     8,     9,    10,    11,    12,    -1,    14,    15,    16,    -1,
    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,    26,    27,
    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,
    38,    39,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,
    48,    49,    -1,    51,    -1,    53,    -1,    55,    -1,    57,
    -1,    59,    60,    -1,    62,    63,    64,    65,    66,    67,
    68,    69,    -1,    -1,    -1,    -1,    -1,    75,    -1,    77,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    98,    -1,    -1,   101,   102,    -1,    -1,    -1,   106,    -1,
   108,   109,   110,    -1,   112,    -1,    -1,   115,     8,     9,
    10,    11,    12,    -1,    14,    15,    16,    -1,    -1,    -1,
    -1,    -1,    -1,    23,    -1,    25,    26,    27,    -1,    -1,
    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,
    -1,    -1,    -1,    43,    44,    45,    -1,    -1,    48,    49,
    -1,    51,    -1,    53,    -1,    55,    -1,    57,    -1,    59,
    60,    -1,    62,    63,    64,    65,    66,    67,    68,    69,
    -1,    -1,    -1,    -1,    -1,    75,    -1,    77,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
    -1,   101,   102,    -1,    -1,    -1,   106,    -1,   108,   109,
   110,    -1,   112,    -1,    -1,   115
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */

/* This file comes from bison-1.27.  */

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
{ yyval.t = 0; ;
    break;}
case 23:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 24:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 25:
{ yyval.t = newCTerm(PA_fAssign,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 26:
{
		    /* <<for X 'from' E1 to E2 by E3>> */
		    /* coord after T_ITER somehow avoids shift/reduce conflict,
		       but serves no other purpose */
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
				  makeLongPos(OZ_subtree(yyvsp[-7].t,newSmallInt(2)),yyvsp[0].t));
		  ;
    break;}
case 27:
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
				    makeLongPos(OZ_subtree(yyvsp[-5].t,newSmallInt(2)),yyvsp[0].t));
		    } else {
		      yyval.t = newCTerm(PA_fMacro,
				    oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					    newCTerm(PA_fEq,yyvsp[-5].t,yyvsp[-2].t,NameUnit),
					    newCTerm(PA_fAtom,oz_atom("next"),NameUnit),
					    yyvsp[-1].t,
					    0),
				    makeLongPos(OZ_subtree(yyvsp[-5].t,newSmallInt(2)),yyvsp[0].t));
		    }
		  ;
    break;}
case 28:
{ yyval.t = newCTerm(PA_fOrElse,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 29:
{ yyval.t = newCTerm(PA_fAndThen,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 30:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 31:
{ yyval.t = newCTerm(PA_fFdCompare,yyvsp[-2].t,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 32:
{ yyval.t = newCTerm(PA_fFdIn,yyvsp[-2].t,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 33:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 34:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 35:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
				  oz_cons(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 36:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 37:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 38:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 39:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 40:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 41:
{ yyval.t = newCTerm(PA_fObjApply,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 42:
{ yyval.t = newCTerm(PA_fOpApply,AtomTilde,
				  oz_mklist(yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 43:
{ yyval.t = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 44:
{ yyval.t = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklist(yyvsp[-1].t,makeInt(xytext,pos())),pos()); ;
    break;}
case 45:
{ yyval.t = newCTerm(PA_fOpApply,AtomHat,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 46:
{ yyval.t = newCTerm(PA_fAt,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 47:
{ yyval.t = newCTerm(PA_fOpApply,AtomDExcl,
				  oz_mklist(yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 48:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 49:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 50:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 51:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 52:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 53:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 54:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 55:
{ yyval.t = newCTerm(PA_fSelf,pos()); ;
    break;}
case 56:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 57:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 58:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 59:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 60:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 61:
{ yyval.t = newCTerm(PA_fRecord,newCTerm(PA_fAtom,AtomCons,
						     makeLongPos(yyvsp[-4].t,yyvsp[0].t)),
				  oz_mklist(yyvsp[-3].t,yyvsp[-2].t)); ;
    break;}
case 62:
{ yyval.t = newCTerm(PA_fApply,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 63:
{ yyval.t = newCTerm(PA_fProc,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-7].t,makeLongPos(yyvsp[-8].t,yyvsp[0].t)); ;
    break;}
case 64:
{ yyval.t = newCTerm(PA_fFun,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-7].t,makeLongPos(yyvsp[-8].t,yyvsp[0].t)); ;
    break;}
case 65:
{ yyval.t = newCTerm(PA_fFunctor,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 66:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 67:
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t); ;
    break;}
case 68:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 69:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 70:
{ yyval.t = newCTerm(PA_fLock,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 71:
{ yyval.t = newCTerm(PA_fLockThen,yyvsp[-4].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 72:
{ yyval.t = newCTerm(PA_fThread,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 73:
{ yyval.t = newCTerm(PA_fTry,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 74:
{ yyval.t = newCTerm(PA_fRaise,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 75:
{ yyval.t = newCTerm(PA_fSkip,pos()); ;
    break;}
case 76:
{ yyval.t = newCTerm(PA_fFail,pos()); ;
    break;}
case 77:
{ yyval.t = newCTerm(PA_fNot,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 78:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 79:
{ yyval.t = newCTerm(PA_fOr,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 80:
{ yyval.t = newCTerm(PA_fDis,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 81:
{ yyval.t = newCTerm(PA_fChoice,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 82:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 83:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 84:
{ yyval.t = newCTerm(PA_fLoop,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 85:
{ yyval.t = newCTerm(PA_fMacro,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 86:
{ yyval.t = newCTerm(PA_fLoop,
				  newCTerm(PA_fAnd,yyvsp[-4].t,yyvsp[-2].t),
				  makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 87:
{
		    yyval.t = newCTerm(PA_fAnd,yyvsp[-1].t,yyvsp[0].t);
		  ;
    break;}
case 89:
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
				  makeLongPos(OZ_subtree(yyvsp[-7].t,newSmallInt(2)),yyvsp[0].t));
		  ;
    break;}
case 90:
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
				    makeLongPos(OZ_subtree(yyvsp[-5].t,newSmallInt(2)),yyvsp[0].t));
		    } else {
		      yyval.t = newCTerm(PA_fMacro,
				    oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					    newCTerm(PA_fEq,yyvsp[-5].t,yyvsp[-2].t,NameUnit),
					    newCTerm(PA_fAtom,oz_atom("next"),NameUnit),
					    yyvsp[-1].t,
					    0),
				    makeLongPos(OZ_subtree(yyvsp[-5].t,newSmallInt(2)),yyvsp[0].t));
		    }
		  ;
    break;}
case 92:
{ yyval.t = AtomNil; ;
    break;}
case 93:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 94:
{ yyval.t = AtomNil; ;
    break;}
case 95:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 96:
{ yyval.t = oz_cons(newCTerm(PA_fRequire,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 97:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 98:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-1].t,
					   newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 99:
{ yyval.t = oz_cons(newCTerm(PA_fImport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 100:
{ yyval.t = oz_cons(newCTerm(PA_fExport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 101:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 102:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-1].t,
					   newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 103:
{ yyval.t = AtomNil; ;
    break;}
case 104:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-2].t,AtomNil,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 105:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 106:
{ yyval.t = newCTerm(PA_fVar,OZ_atom(xytext),yyvsp[0].t); ;
    break;}
case 107:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 108:
{ yyval.t = oz_mklist(oz_pair2(yyvsp[0].t,yyvsp[-2].t)); ;
    break;}
case 109:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 110:
{ yyval.t = oz_cons(oz_pair2(yyvsp[-1].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 111:
{ yyval.t = PA_fNoImportAt; ;
    break;}
case 112:
{ yyval.t = newCTerm(PA_fImportAt,yyvsp[0].t); ;
    break;}
case 113:
{ yyval.t = AtomNil; ;
    break;}
case 114:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 115:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,
					   newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t)),yyvsp[0].t); ;
    break;}
case 116:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 117:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 118:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 119:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 120:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 121:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 122:
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 123:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 124:
{ yyval.t = AtomNil; ;
    break;}
case 125:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 126:
{ yyval.t = newCTerm(PA_fAtom,AtomNil,yyvsp[0].t); ;
    break;}
case 127:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomCons,yyvsp[-2].t),
				  oz_mklist(yyvsp[-1].t,yyvsp[0].t)); ;
    break;}
case 128:
{ yyval.t = PA_fNoCatch; ;
    break;}
case 129:
{ yyval.t = newCTerm(PA_fCatch,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 130:
{ yyval.t = PA_fNoFinally; ;
    break;}
case 131:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 132:
{
		    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
				  newCTerm(PA_fAtom,yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
		  ;
    break;}
case 133:
{
		    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
				  makeVar(yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
		  ;
    break;}
case 134:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 135:
{ yyval.t = NameUnit; ;
    break;}
case 136:
{ yyval.t = NameTrue; ;
    break;}
case 137:
{ yyval.t = NameFalse; ;
    break;}
case 138:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 139:
{ yyval.t = AtomNil; ;
    break;}
case 140:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 141:
{ yyval.t = oz_cons(newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 142:
{ yyval.t = NameFalse; ;
    break;}
case 143:
{ yyval.t = NameTrue; ;
    break;}
case 144:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 145:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 146:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 147:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 148:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 149:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 150:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 151:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 152:
{ yyval.t = newCTerm(PA_fBoolCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 153:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 154:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 155:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 156:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 157:
{ checkDeprecation(yyvsp[-3].t);
		    yyval.t = newCTerm(PA_fBoolCase,yyvsp[-5].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-6].t,yyvsp[0].t));
		  ;
    break;}
case 158:
{ yyval.t = newCTerm(PA_fCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 159:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 160:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 161:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 162:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 163:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 164:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 165:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 166:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 167:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 168:
{ yyval.t = newCTerm(PA_fCaseClause,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 169:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 170:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-3].t,
				  newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 171:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-5].t,yyvsp[-2].t,yyvsp[0].t,yyvsp[-3].t); ;
    break;}
case 172:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 173:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 174:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 175:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
				  oz_cons(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 176:
{ yyval.t = newCTerm(PA_fClass,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 177:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 178:
{ yyval.t = newCTerm(PA_fDollar,yyvsp[0].t); ;
    break;}
case 179:
{ yyval.t = AtomNil; ;
    break;}
case 180:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 181:
{ yyval.t = newCTerm(PA_fFrom,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 182:
{ yyval.t = newCTerm(PA_fAttr,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 183:
{ yyval.t = newCTerm(PA_fFeat,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 184:
{ yyval.t = newCTerm(PA_fProp,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 185:
{ yyval.t = AtomNil; ;
    break;}
case 186:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 187:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 188:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 189:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 190:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 191:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 192:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 193:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 194:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 195:
{ yyval.t = AtomNil; ;
    break;}
case 196:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 197:
{ yyval.t = newCTerm(PA_fMeth,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-3].t); ;
    break;}
case 198:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 199:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 200:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 201:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 202:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 203:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 204:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 205:
{ yyval.t = newCTerm(PA_fRecord,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 206:
{ yyval.t = newCTerm(PA_fOpenRecord,yyvsp[-4].t,yyvsp[-2].t); ;
    break;}
case 207:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 208:
{ yyval.t = makeVar(xytext); ;
    break;}
case 209:
{ yyval.t = newCTerm(PA_fEscape,makeVar(xytext),yyvsp[-1].t); ;
    break;}
case 210:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 211:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 212:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 213:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 214:
{ yyval.t = AtomNil; ;
    break;}
case 215:
{ yyval.t = newCTerm(PA_fMethArg,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 216:
{ yyval.t = newCTerm(PA_fMethColonArg,yyvsp[-3].t,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 217:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 218:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 219:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 220:
{ yyval.t = newCTerm(PA_fDefault,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 221:
{ yyval.t = PA_fNoDefault; ;
    break;}
case 222:
{ yyval.t = newCTerm(PA_fCond,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 223:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 224:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 225:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 226:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 227:
{ yyval.t = newCTerm(PA_fClause,newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 228:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 229:
{ yyval.t = oz_mklist(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 230:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 231:
{ yyval.t = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,yyvsp[0].t),
				  yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 232:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-3].t,yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 233:
{ yyval.t = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 234:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 235:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 236:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 237:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 238:
{ yyval.t = makeVar(xytext); ;
    break;}
case 239:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 240:
{ yyval.t = newCTerm(PA_fEscape,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 241:
{ yyval.t = makeString(xytext,pos()); ;
    break;}
case 242:
{ yyval.t = makeInt(xytext,pos()); ;
    break;}
case 243:
{ yyval.t = makeInt(xytext[0],pos()); ;
    break;}
case 244:
{ yyval.t = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); ;
    break;}
case 245:
{ yyval.t = pos(); ;
    break;}
case 246:
{ yyval.t = pos(); ;
    break;}
case 247:
{ OZ_Term prefix =
		      scannerPrefix? scannerPrefix: PA_zy;
		    yyval.t = newCTerm(PA_fScanner,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,prefix,
				  makeLongPos(yyvsp[-6].t,yyvsp[0].t)); ;
    break;}
case 248:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 249:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 250:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 251:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 252:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 253:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 254:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 255:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 256:
{ yyval.t = newCTerm(PA_fLexicalRule,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 257:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 258:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 259:
{ yyval.t = newCTerm(PA_fMode,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 260:
{ yyval.t = AtomNil; ;
    break;}
case 261:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 262:
{ yyval.t = newCTerm(PA_fInheritedModes,yyvsp[0].t); ;
    break;}
case 263:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 264:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 265:
{ OZ_Term expect = parserExpect? parserExpect: newSmallInt(0);
		    yyval.t = newCTerm(PA_fParser,yyvsp[-6].t,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,expect,
				  makeLongPos(yyvsp[-7].t,yyvsp[0].t)); ;
    break;}
case 266:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 267:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 268:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 269:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 270:
{ yyval.t = newCTerm(PA_fToken,AtomNil); ;
    break;}
case 271:
{ yyval.t = newCTerm(PA_fToken,yyvsp[0].t); ;
    break;}
case 272:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 273:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 274:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 275:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 276:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 277:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 278:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 279:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 280:
{ *prodKey[depth]++ = '='; ;
    break;}
case 281:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 282:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 283:
{ *prodKey[depth]++ = '='; ;
    break;}
case 284:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 285:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,PA_none); ;
    break;}
case 286:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 287:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 288:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 291:
{ prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg(yyvsp[-1].t,0))); ;
    break;}
case 292:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 293:
{ depth--; ;
    break;}
case 294:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 295:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 296:
{ depth--; ;
    break;}
case 297:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 298:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 299:
{ depth--; ;
    break;}
case 300:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 301:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 302:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 303:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 304:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 305:
{ *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; ;
    break;}
case 308:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 309:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 310:
{ *prodKey[depth] = '\0';
		    yyval.t = oz_pair2(prodName[depth],OZ_string(prodKeyBuffer[depth]));
		    prodName[depth] = PA_none;
		    prodKey[depth] = prodKeyBuffer[depth];
		  ;
    break;}
case 311:
{ yyval.t = AtomNil; ;
    break;}
case 312:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 313:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 314:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 315:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 316:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 317:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 318:
{ yyval.t = AtomNil; ;
    break;}
case 319:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 320:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 321:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 322:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 323:
{ yyval.t = newCTerm(PA_fSynAlternative, yyvsp[0].t); ;
    break;}
case 324:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 325:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 326:
{ OZ_Term t = yyvsp[0].t;
		    while (terms[depth]) {
		      t = oz_cons(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    yyval.t = newCTerm(PA_fSynSequence, decls[depth], t, yyvsp[-1].t);
		    decls[depth] = AtomNil;
		  ;
    break;}
case 327:
{ yyval.t = newCTerm(PA_fSynSequence, AtomNil, yyvsp[0].t, yyvsp[-1].t); ;
    break;}
case 328:
{ yyval.t = AtomNil; ;
    break;}
case 329:
{ yyval.t = oz_mklist(newCTerm(PA_fSynAction,yyvsp[0].t)); ;
    break;}
case 330:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 331:
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
case 332:
{ yyval.t = oz_cons(newCTerm(PA_fSynAssignment, terms[depth]->term, yyvsp[-1].t),
				  yyvsp[0].t);
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  ;
    break;}
case 333:
{ while (terms[depth]) {
		      decls[depth] = oz_cons(terms[depth]->term, decls[depth]);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    yyval.t = yyvsp[0].t;
		  ;
    break;}
case 334:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 335:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 336:
{ terms[depth] = new TermNode(yyvsp[0].t, terms[depth]); ;
    break;}
case 337:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 338:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 339:
{ yyval.t = newCTerm(PA_fSynAssignment,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 340:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 341:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 342:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_cons(newCTerm(PA_fSynApplication,yyvsp[-3].t,
						    AtomNil),
					   AtomNil),yyvsp[-1].t);
		  ;
    break;}
case 343:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 344:
{ yyval.t = newCTerm(PA_fSynAssignment,
				  newCTerm(PA_fEscape,yyvsp[-2].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 345:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 346:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 347:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_mklist(yyvsp[-2].t),yyvsp[-3].t);
		  ;
    break;}
case 348:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_mklist(yyvsp[-3].t),yyvsp[-1].t);
		  ;
    break;}
case 349:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 350:
{ depth--; ;
    break;}
case 351:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 352:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 353:
{ depth--; ;
    break;}
case 354:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 355:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 356:
{ depth--; ;
    break;}
case 357:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 358:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 359:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[-4].t,yyvsp[-1].t); ;
    break;}
case 360:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 361:
{ yyval.t = makeVar(xytext); ;
    break;}
case 362:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 363:
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
