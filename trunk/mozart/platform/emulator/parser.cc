
/*  A Bison parser, made from /home/chris/devel/mozart/platform/emulator/parser.yy
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
#define	T_attr	274
#define	T_at	275
#define	T_case	276
#define	T_catch	277
#define	T_choice	278
#define	T_class	279
#define	T_cond	280
#define	T_declare	281
#define	T_define	282
#define	T_dis	283
#define	T_else	284
#define	T_elsecase	285
#define	T_elseif	286
#define	T_elseof	287
#define	T_end	288
#define	T_export	289
#define	T_fail	290
#define	T_false	291
#define	T_FALSE_LABEL	292
#define	T_feat	293
#define	T_finally	294
#define	T_from	295
#define	T_fun	296
#define	T_functor	297
#define	T_if	298
#define	T_import	299
#define	T_in	300
#define	T_local	301
#define	T_lock	302
#define	T_meth	303
#define	T_not	304
#define	T_of	305
#define	T_or	306
#define	T_prepare	307
#define	T_proc	308
#define	T_prop	309
#define	T_raise	310
#define	T_require	311
#define	T_self	312
#define	T_skip	313
#define	T_then	314
#define	T_thread	315
#define	T_true	316
#define	T_TRUE_LABEL	317
#define	T_try	318
#define	T_unit	319
#define	T_UNIT_LABEL	320
#define	T_ENDOFFILE	321
#define	T_REGEX	322
#define	T_lex	323
#define	T_mode	324
#define	T_parser	325
#define	T_prod	326
#define	T_scanner	327
#define	T_syn	328
#define	T_token	329
#define	T_REDUCE	330
#define	T_SEP	331
#define	T_OOASSIGN	332
#define	T_orelse	333
#define	T_andthen	334
#define	T_COMPARE	335
#define	T_FDCOMPARE	336
#define	T_FDIN	337
#define	T_ADD	338
#define	T_FDMUL	339
#define	T_OTHERMUL	340
#define	T_DEREFF	341


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

OZ_Term _PA_AtomTab[106];

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
};

void parser_init(void) {
   for (int i = 106; i--; )
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



#define	YYFINAL		747
#define	YYFLAG		-32768
#define	YYNTBASE	108

#define YYTRANSLATE(x) ((unsigned)(x) <= 341 ? yytranslate[x] : 244)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   107,     2,    86,   101,     2,     2,     2,    98,
    99,     2,    96,    90,    97,    92,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,   106,     2,     2,
    78,     2,     2,    94,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   102,     2,   103,    93,   100,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   104,    85,   105,    91,     2,     2,     2,     2,
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
    77,    79,    80,    81,    82,    83,    84,    87,    88,    89,
    95
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     5,     8,    11,    13,    17,    20,    27,    34,
    40,    41,    44,    46,    48,    50,    51,    54,    57,    60,
    62,    65,    70,    75,    80,    85,    90,    95,   100,   105,
   107,   112,   114,   118,   123,   128,   133,   138,   142,   147,
   150,   155,   159,   163,   167,   169,   171,   173,   175,   177,
   179,   181,   183,   185,   187,   189,   191,   198,   205,   216,
   227,   234,   236,   243,   246,   249,   255,   263,   269,   277,
   283,   285,   287,   293,   296,   302,   308,   314,   316,   318,
   319,   322,   323,   325,   330,   337,   342,   347,   352,   359,
   364,   365,   369,   376,   379,   381,   385,   388,   393,   394,
   397,   398,   401,   406,   408,   410,   412,   414,   416,   418,
   423,   425,   426,   429,   431,   435,   436,   440,   441,   444,
   452,   460,   462,   464,   466,   468,   470,   471,   474,   479,
   480,   482,   484,   486,   488,   490,   492,   494,   496,   498,
   505,   508,   511,   515,   517,   525,   532,   535,   538,   542,
   544,   546,   550,   554,   556,   560,   564,   566,   571,   578,
   583,   588,   590,   595,   603,   605,   607,   608,   611,   616,
   621,   626,   631,   632,   635,   639,   641,   643,   645,   647,
   649,   651,   653,   654,   657,   663,   665,   670,   672,   674,
   676,   678,   680,   685,   691,   693,   695,   699,   701,   703,
   705,   708,   709,   712,   717,   719,   721,   723,   727,   728,
   734,   737,   738,   740,   744,   749,   755,   759,   763,   766,
   771,   776,   782,   784,   788,   790,   792,   794,   798,   800,
   802,   804,   806,   807,   808,   817,   819,   821,   823,   826,
   829,   832,   838,   844,   849,   851,   853,   858,   859,   862,
   865,   867,   869,   879,   881,   883,   886,   889,   890,   893,
   895,   898,   900,   904,   906,   909,   911,   914,   915,   925,
   926,   927,   938,   945,   949,   952,   955,   957,   958,   961,
   962,   963,   970,   971,   972,   979,   980,   981,   988,   990,
   994,   996,   998,  1000,  1001,  1003,  1005,  1007,  1008,  1009,
  1012,  1014,  1017,  1022,  1027,  1035,  1036,  1039,  1041,  1043,
  1045,  1047,  1049,  1053,  1056,  1060,  1061,  1064,  1067,  1073,
  1078,  1081,  1084,  1086,  1088,  1090,  1093,  1097,  1099,  1101,
  1106,  1108,  1114,  1116,  1118,  1124,  1129,  1130,  1131,  1141,
  1142,  1143,  1153,  1154,  1155,  1165,  1167,  1173,  1175,  1177,
  1179
};

static const short yyrhs[] = {   109,
    67,     0,     1,     0,   114,   110,     0,   198,   110,     0,
   110,     0,   182,   120,   110,     0,   111,   109,     0,    27,
   183,   114,    46,   182,   110,     0,    27,   183,   114,    46,
   114,   110,     0,    27,   183,   114,   182,   110,     0,     0,
     3,   112,     0,     5,     0,     6,     0,     7,     0,     0,
   113,   112,     0,    96,     4,     0,    97,     4,     0,   115,
     0,   115,   114,     0,   115,    78,   183,   115,     0,   115,
    79,   183,   115,     0,   115,    80,   183,   115,     0,   115,
    81,   183,   115,     0,   115,   126,   183,   115,     0,   115,
   127,   183,   115,     0,   115,   128,   183,   115,     0,   115,
    85,   183,   115,     0,   117,     0,   117,    86,   183,   116,
     0,   117,     0,   117,    86,   116,     0,   117,   129,   183,
   117,     0,   117,   130,   183,   117,     0,   117,   131,   183,
   117,     0,   117,    90,   183,   117,     0,    91,   183,   117,
     0,   117,    92,   183,   117,     0,   117,    13,     0,   117,
    93,   183,   117,     0,    94,   183,   117,     0,    95,   183,
   117,     0,    98,   132,    99,     0,   176,     0,   178,     0,
   100,     0,    65,     0,    62,     0,    37,     0,    58,     0,
   101,     0,   179,     0,   180,     0,   181,     0,   137,     0,
   102,   183,   115,   134,   103,   183,     0,   104,   183,   115,
   133,   105,   183,     0,    54,   183,   118,   104,   115,   133,
   105,   132,    34,   183,     0,    42,   183,   118,   104,   115,
   133,   105,   132,    34,   183,     0,    43,   183,   154,   119,
    34,   183,     0,   153,     0,    47,   183,   114,    46,   114,
    34,     0,    44,   144,     0,    22,   146,     0,    48,   183,
   132,    34,   183,     0,    48,   183,   115,    60,   132,    34,
   183,     0,    61,   183,   132,    34,   183,     0,    64,   183,
   132,   135,   136,    34,   183,     0,    56,   183,   132,    34,
   183,     0,    59,     0,    36,     0,    50,   183,   132,    34,
   183,     0,    26,   169,     0,    52,   183,   173,    34,   183,
     0,    29,   183,   173,    34,   183,     0,    24,   183,   175,
    34,   183,     0,   184,     0,   192,     0,     0,   176,   118,
     0,     0,   120,     0,    57,   183,   121,   119,     0,    53,
   183,   114,    46,   114,   119,     0,    53,   183,   114,   119,
     0,    45,   183,   121,   119,     0,    35,   183,   125,   119,
     0,    28,   183,   114,    46,   114,   119,     0,    28,   183,
   114,   119,     0,     0,   177,   124,   121,     0,   122,    98,
   123,    99,   124,   121,     0,    16,   183,     0,   143,     0,
   143,   106,   177,     0,   143,   123,     0,   143,   106,   177,
   123,     0,     0,    21,   176,     0,     0,   177,   125,     0,
   143,   106,   177,   125,     0,    82,     0,    83,     0,    84,
     0,    87,     0,    88,     0,    89,     0,   114,    46,   183,
   114,     0,   114,     0,     0,   115,   133,     0,   182,     0,
   182,   115,   134,     0,     0,    23,   183,   149,     0,     0,
    40,   132,     0,   138,   183,    98,   140,   141,    99,   183,
     0,   139,   183,    98,   140,   141,    99,   183,     0,     9,
     0,    66,     0,    63,     0,    38,     0,    16,     0,     0,
   115,   140,     0,   142,   106,   115,   140,     0,     0,    19,
     0,   176,     0,   177,     0,   180,     0,    65,     0,    62,
     0,    37,     0,   176,     0,   180,     0,   183,   114,    60,
   132,   145,   183,     0,    32,   144,     0,    31,   146,     0,
    30,   132,    34,     0,    34,     0,   183,   114,    60,   183,
   132,   147,   183,     0,   183,   114,    51,   148,   147,   183,
     0,    32,   144,     0,    31,   146,     0,    30,   132,    34,
     0,    34,     0,   150,     0,   150,    18,   148,     0,   150,
    33,   148,     0,   150,     0,   150,    18,   149,     0,   151,
    60,   132,     0,   152,     0,   152,    81,   182,   114,     0,
   152,    81,   182,   114,    46,   114,     0,   152,    78,   183,
   152,     0,   152,    85,   183,   152,     0,   117,     0,   117,
    86,   183,   116,     0,    25,   183,   154,   155,   160,    34,
   183,     0,   115,     0,   182,     0,     0,   156,   155,     0,
    41,   183,   115,   133,     0,    20,   183,   158,   157,     0,
    39,   183,   158,   157,     0,    55,   183,   115,   133,     0,
     0,   158,   157,     0,   159,   106,   115,     0,   159,     0,
   176,     0,   178,     0,   180,     0,    65,     0,    62,     0,
    37,     0,     0,   161,   160,     0,    49,   183,   162,   132,
    34,     0,   163,     0,   163,    78,   183,   177,     0,   176,
     0,   178,     0,    65,     0,    62,     0,    37,     0,   164,
    98,   165,    99,     0,   164,    98,   165,    19,    99,     0,
     9,     0,    16,     0,   107,   183,    16,     0,    66,     0,
    63,     0,    38,     0,   166,   165,     0,     0,   167,   168,
     0,   142,   106,   167,   168,     0,   177,     0,   101,     0,
   100,     0,    17,   183,   115,     0,     0,   183,   171,   170,
    34,   183,     0,    30,   132,     0,     0,   172,     0,   172,
    18,   171,     0,   114,    60,   183,   132,     0,   114,    46,
   114,    60,   132,     0,   174,    18,   174,     0,   174,    18,
   173,     0,   114,   182,     0,   114,    46,   114,   182,     0,
   114,   182,    60,   132,     0,   114,    46,   114,    60,   132,
     0,   132,     0,   132,    18,   175,     0,     8,     0,    15,
     0,   177,     0,   107,   183,   177,     0,    14,     0,    11,
     0,    12,     0,    10,     0,     0,     0,    73,   183,   177,
   155,   160,   185,    34,   183,     0,   186,     0,   187,     0,
   189,     0,   186,   185,     0,   187,   185,     0,   189,   185,
     0,    69,   176,    78,   188,    34,     0,    69,   177,    78,
   188,    34,     0,    69,   188,   132,    34,     0,    68,     0,
    14,     0,    70,   177,   190,    34,     0,     0,   191,   190,
     0,    41,   197,     0,   187,     0,   189,     0,    71,   183,
   177,   155,   160,   194,   193,    34,   183,     0,   221,     0,
   199,     0,   221,   193,     0,   199,   193,     0,     0,    75,
   195,     0,   196,     0,   196,   195,     0,   176,     0,   176,
   106,   115,     0,   177,     0,   177,   197,     0,   199,     0,
   199,   198,     0,     0,    72,   177,    78,   200,   203,   218,
   219,   224,    34,     0,     0,     0,    72,   101,   201,    78,
   202,   203,   218,   219,   224,    34,     0,    72,   203,   218,
   219,   224,    34,     0,   205,   177,   216,     0,   177,   217,
     0,   204,   206,     0,   205,     0,     0,   176,   106,     0,
     0,     0,    98,   207,   213,    99,   208,   216,     0,     0,
     0,   102,   209,   213,   103,   210,   216,     0,     0,     0,
   104,   211,   213,   105,   212,   216,     0,   214,     0,   214,
   215,   213,     0,   177,     0,   100,     0,    77,     0,     0,
   217,     0,    87,     0,    88,     0,     0,     0,   220,    46,
     0,   221,     0,   221,   220,     0,    74,   176,   224,    34,
     0,    74,   177,   224,    34,     0,    74,   242,    98,   222,
    99,   224,    34,     0,     0,   223,   222,     0,   177,     0,
   101,     0,   100,     0,   225,     0,   226,     0,   226,    18,
   225,     0,   182,   228,     0,    59,   183,   227,     0,     0,
    76,   132,     0,   229,   228,     0,   229,   217,   183,   230,
   218,     0,   229,    78,   232,   230,     0,    46,   230,     0,
   233,   230,     0,   227,     0,   177,     0,   227,     0,   231,
   230,     0,   178,    78,   232,     0,   232,     0,   177,     0,
   177,   217,   183,   218,     0,   234,     0,   107,   183,   177,
    78,   232,     0,   234,     0,   241,     0,   205,   183,   241,
   216,   218,     0,   241,   217,   183,   218,     0,     0,     0,
   204,   183,    98,   235,   243,    99,   236,   216,   218,     0,
     0,     0,   204,   183,   102,   237,   243,   103,   238,   216,
   218,     0,     0,     0,   204,   183,   104,   239,   243,   105,
   240,   216,   218,     0,   176,     0,   242,   183,    98,   133,
    99,     0,     9,     0,    16,     0,   224,     0,   224,   215,
   243,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   668,   670,   674,   676,   679,   681,   686,   688,   691,   693,
   696,   700,   702,   704,   706,   710,   712,   716,   723,   732,
   734,   738,   740,   742,   744,   746,   749,   751,   753,   755,
   757,   763,   765,   769,   772,   775,   778,   780,   783,   786,
   789,   792,   794,   797,   799,   801,   803,   805,   807,   809,
   811,   813,   815,   817,   819,   821,   823,   827,   829,   832,
   835,   837,   839,   841,   843,   845,   847,   849,   851,   853,
   855,   857,   859,   861,   863,   865,   867,   869,   871,   875,
   877,   882,   884,   889,   891,   893,   896,   898,   900,   902,
   907,   909,   911,   915,   919,   921,   923,   925,   929,   931,
   935,   937,   939,   944,   948,   952,   956,   960,   964,   968,
   970,   974,   976,   980,   982,   988,   990,   994,   996,  1000,
  1005,  1012,  1014,  1016,  1018,  1022,  1026,  1028,  1030,  1034,
  1036,  1040,  1042,  1044,  1046,  1048,  1050,  1054,  1056,  1060,
  1064,  1066,  1068,  1070,  1074,  1078,  1082,  1084,  1086,  1088,
  1092,  1094,  1096,  1100,  1102,  1106,  1110,  1112,  1115,  1119,
  1121,  1123,  1125,  1131,  1136,  1138,  1143,  1145,  1149,  1151,
  1153,  1155,  1159,  1161,  1165,  1167,  1171,  1173,  1175,  1177,
  1179,  1181,  1185,  1187,  1191,  1195,  1197,  1201,  1203,  1205,
  1207,  1209,  1211,  1213,  1217,  1219,  1221,  1223,  1225,  1227,
  1231,  1233,  1237,  1239,  1243,  1245,  1247,  1252,  1254,  1258,
  1262,  1264,  1268,  1270,  1274,  1276,  1280,  1282,  1286,  1290,
  1292,  1295,  1299,  1301,  1305,  1309,  1313,  1315,  1319,  1323,
  1325,  1329,  1333,  1337,  1347,  1355,  1357,  1359,  1361,  1363,
  1365,  1369,  1371,  1375,  1379,  1381,  1385,  1389,  1391,  1395,
  1397,  1399,  1405,  1413,  1415,  1417,  1419,  1423,  1425,  1429,
  1431,  1435,  1437,  1441,  1443,  1447,  1449,  1453,  1455,  1457,
  1458,  1459,  1461,  1465,  1467,  1469,  1473,  1474,  1477,  1481,
  1482,  1482,  1483,  1484,  1484,  1485,  1486,  1486,  1489,  1491,
  1495,  1496,  1499,  1503,  1504,  1507,  1508,  1511,  1519,  1521,
  1525,  1527,  1531,  1533,  1535,  1539,  1541,  1545,  1547,  1549,
  1553,  1557,  1559,  1563,  1572,  1576,  1578,  1582,  1584,  1594,
  1599,  1606,  1608,  1612,  1616,  1618,  1622,  1624,  1628,  1630,
  1636,  1640,  1643,  1648,  1650,  1654,  1658,  1659,  1660,  1662,
  1663,  1664,  1666,  1667,  1668,  1672,  1674,  1678,  1680,  1685,
  1687
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","T_SWITCH",
"T_SWITCHNAME","T_LOCALSWITCHES","T_PUSHSWITCHES","T_POPSWITCHES","T_OZATOM",
"T_ATOM_LABEL","T_OZFLOAT","T_OZINT","T_AMPER","T_DOTINT","T_STRING","T_VARIABLE",
"T_VARIABLE_LABEL","T_DEFAULT","T_CHOICE","T_LDOTS","T_attr","T_at","T_case",
"T_catch","T_choice","T_class","T_cond","T_declare","T_define","T_dis","T_else",
"T_elsecase","T_elseif","T_elseof","T_end","T_export","T_fail","T_false","T_FALSE_LABEL",
"T_feat","T_finally","T_from","T_fun","T_functor","T_if","T_import","T_in","T_local",
"T_lock","T_meth","T_not","T_of","T_or","T_prepare","T_proc","T_prop","T_raise",
"T_require","T_self","T_skip","T_then","T_thread","T_true","T_TRUE_LABEL","T_try",
"T_unit","T_UNIT_LABEL","T_ENDOFFILE","T_REGEX","T_lex","T_mode","T_parser",
"T_prod","T_scanner","T_syn","T_token","T_REDUCE","T_SEP","'='","T_OOASSIGN",
"T_orelse","T_andthen","T_COMPARE","T_FDCOMPARE","T_FDIN","'|'","'#'","T_ADD",
"T_FDMUL","T_OTHERMUL","','","'~'","'.'","'^'","'@'","T_DEREFF","'+'","'-'",
"'('","')'","'_'","'$'","'['","']'","'{'","'}'","':'","'!'","file","queries",
"queries1","directive","switchList","switch","sequence","phrase","hashes","phrase2",
"procFlags","optFunctorDescriptorList","functorDescriptorList","importDecls",
"variableLabel","featureList","optImportAt","exportDecls","compare","fdCompare",
"fdIn","add","fdMul","otherMul","inSequence","phraseList","fixedListArgs","optCatch",
"optFinally","record","recordAtomLabel","recordVarLabel","recordArguments","optDots",
"feature","featureNoVar","ifMain","ifRest","caseMain","caseRest","elseOfList",
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
   108,   108,   109,   109,   109,   109,   110,   110,   110,   110,
   110,   111,   111,   111,   111,   112,   112,   113,   113,   114,
   114,   115,   115,   115,   115,   115,   115,   115,   115,   115,
   115,   116,   116,   117,   117,   117,   117,   117,   117,   117,
   117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
   117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
   117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
   117,   117,   117,   117,   117,   117,   117,   117,   117,   118,
   118,   119,   119,   120,   120,   120,   120,   120,   120,   120,
   121,   121,   121,   122,   123,   123,   123,   123,   124,   124,
   125,   125,   125,   126,   127,   128,   129,   130,   131,   132,
   132,   133,   133,   134,   134,   135,   135,   136,   136,   137,
   137,   138,   138,   138,   138,   139,   140,   140,   140,   141,
   141,   142,   142,   142,   142,   142,   142,   143,   143,   144,
   145,   145,   145,   145,   146,   146,   147,   147,   147,   147,
   148,   148,   148,   149,   149,   150,   151,   151,   151,   152,
   152,   152,   152,   153,   154,   154,   155,   155,   156,   156,
   156,   156,   157,   157,   158,   158,   159,   159,   159,   159,
   159,   159,   160,   160,   161,   162,   162,   163,   163,   163,
   163,   163,   163,   163,   164,   164,   164,   164,   164,   164,
   165,   165,   166,   166,   167,   167,   167,   168,   168,   169,
   170,   170,   171,   171,   172,   172,   173,   173,   174,   174,
   174,   174,   175,   175,   176,   177,   178,   178,   179,   180,
   180,   181,   182,   183,   184,   185,   185,   185,   185,   185,
   185,   186,   186,   187,   188,   188,   189,   190,   190,   191,
   191,   191,   192,   193,   193,   193,   193,   194,   194,   195,
   195,   196,   196,   197,   197,   198,   198,   200,   199,   201,
   202,   199,   199,   203,   203,   203,   204,   204,   205,   207,
   208,   206,   209,   210,   206,   211,   212,   206,   213,   213,
   214,   214,   215,   216,   216,   217,   217,   218,   219,   219,
   220,   220,   221,   221,   221,   222,   222,   223,   223,   223,
   224,   225,   225,   226,   226,   227,   227,   228,   228,   228,
   228,   228,   228,   229,   230,   230,   231,   231,   232,   232,
   232,   233,   233,   234,   234,   234,   235,   236,   234,   237,
   238,   234,   239,   240,   234,   241,   241,   242,   242,   243,
   243
};

static const short yyr2[] = {     0,
     2,     1,     2,     2,     1,     3,     2,     6,     6,     5,
     0,     2,     1,     1,     1,     0,     2,     2,     2,     1,
     2,     4,     4,     4,     4,     4,     4,     4,     4,     1,
     4,     1,     3,     4,     4,     4,     4,     3,     4,     2,
     4,     3,     3,     3,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     6,     6,    10,    10,
     6,     1,     6,     2,     2,     5,     7,     5,     7,     5,
     1,     1,     5,     2,     5,     5,     5,     1,     1,     0,
     2,     0,     1,     4,     6,     4,     4,     4,     6,     4,
     0,     3,     6,     2,     1,     3,     2,     4,     0,     2,
     0,     2,     4,     1,     1,     1,     1,     1,     1,     4,
     1,     0,     2,     1,     3,     0,     3,     0,     2,     7,
     7,     1,     1,     1,     1,     1,     0,     2,     4,     0,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     6,
     2,     2,     3,     1,     7,     6,     2,     2,     3,     1,
     1,     3,     3,     1,     3,     3,     1,     4,     6,     4,
     4,     1,     4,     7,     1,     1,     0,     2,     4,     4,
     4,     4,     0,     2,     3,     1,     1,     1,     1,     1,
     1,     1,     0,     2,     5,     1,     4,     1,     1,     1,
     1,     1,     4,     5,     1,     1,     3,     1,     1,     1,
     2,     0,     2,     4,     1,     1,     1,     3,     0,     5,
     2,     0,     1,     3,     4,     5,     3,     3,     2,     4,
     4,     5,     1,     3,     1,     1,     1,     3,     1,     1,
     1,     1,     0,     0,     8,     1,     1,     1,     2,     2,
     2,     5,     5,     4,     1,     1,     4,     0,     2,     2,
     1,     1,     9,     1,     1,     2,     2,     0,     2,     1,
     2,     1,     3,     1,     2,     1,     2,     0,     9,     0,
     0,    10,     6,     3,     2,     2,     1,     0,     2,     0,
     0,     6,     0,     0,     6,     0,     0,     6,     1,     3,
     1,     1,     1,     0,     1,     1,     1,     0,     0,     2,
     1,     2,     4,     4,     7,     0,     2,     1,     1,     1,
     1,     1,     3,     2,     3,     0,     2,     2,     5,     4,
     2,     2,     1,     1,     1,     2,     3,     1,     1,     4,
     1,     5,     1,     1,     5,     4,     0,     0,     9,     0,
     0,     9,     0,     0,     9,     1,     5,     1,     1,     1,
     3
};

static const short yydefact[] = {     0,
     2,    16,    13,    14,    15,   225,   122,   232,   230,   231,
   229,   226,   126,   234,   234,   234,   234,   234,   234,    72,
    50,   125,   234,   234,   234,   234,   234,   234,   234,   234,
   234,    51,    71,   234,    49,   124,   234,    48,   123,   234,
   278,   234,   234,   234,   234,     0,    47,    52,   234,   234,
   234,     0,     5,   233,    11,    20,    30,    56,   234,   234,
    62,    45,   227,    46,    53,    54,    55,     0,    78,    79,
    11,   266,     0,     0,    12,    16,    65,     0,     0,   233,
    74,     0,     0,     0,    80,   233,    64,     0,     0,     0,
     0,     0,    80,     0,     0,     0,     0,   270,     0,     0,
   298,     0,   277,     0,     0,     0,     0,   111,     0,     0,
     0,     0,     1,     7,     3,   234,   234,   234,   234,   104,
   105,   106,   234,    21,   234,   234,   234,    40,   234,   107,
   108,   109,   234,   234,   234,   234,   234,   234,     0,     0,
   234,   234,   234,   234,   234,    11,     4,   267,    18,    19,
    17,     0,   223,     0,   165,   167,   166,     0,   212,   213,
   233,   233,     0,     0,     0,    80,    82,     0,     0,    20,
     0,     0,     0,     0,     0,     0,   116,   167,     0,   279,
   268,   296,   297,   275,   299,   280,   283,   286,   276,   294,
   167,    38,    42,    43,   234,    44,   233,   112,   228,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   127,   127,     0,   101,    91,     0,
    91,     6,     0,   234,     0,   234,   234,   234,   234,   234,
   183,   167,     0,   234,     0,     0,     0,   233,    11,     0,
   219,   234,     0,     0,    81,     0,    83,     0,     0,     0,
   234,   234,   234,     0,   234,   234,   234,   118,   183,   271,
   278,     0,   233,     0,   301,     0,     0,     0,   274,   295,
   183,     0,     0,   114,   112,     0,    22,    23,    24,    25,
    29,    26,    27,    28,    31,    32,    37,    39,    41,    34,
    35,    36,    50,    49,    48,   127,   130,     0,    45,   227,
    54,   130,    82,    82,     0,   138,   101,   139,   234,    82,
     0,    99,    82,    82,   162,     0,   151,     0,   157,     0,
   224,    77,     0,     0,     0,     0,   234,     0,   183,   168,
     0,     0,   211,   234,   214,    11,    11,    10,   233,     0,
    76,   218,   217,   112,   234,     0,     0,     0,    66,    73,
    75,   112,    70,    68,     0,     0,     0,   258,   278,     0,
   298,   348,   349,   233,   233,     0,   234,   316,     0,   311,
   312,   300,   302,   292,   291,     0,   289,     0,     0,     0,
   110,   234,   233,   113,   234,     0,   128,   131,     0,     0,
     0,     0,    90,    88,     0,   102,    94,    87,     0,     0,
    91,     0,    86,    84,   234,     0,   234,   234,   150,   234,
     0,     0,     0,   234,   233,   234,     0,   182,   181,   180,
   173,   176,   177,   178,   179,   173,   112,   112,     0,   234,
   184,     0,   215,   210,     9,     8,     0,   220,   221,     0,
    61,     0,   234,   234,   144,   234,    63,   234,     0,   117,
   154,   119,   234,     0,     0,   298,   299,     0,     0,   306,
   316,   316,     0,   234,   346,   324,   234,   234,   323,   314,
   316,   316,   333,   334,   234,   273,   233,   281,   293,     0,
   284,   287,     0,     0,     0,   236,   237,   238,    57,   115,
    58,    33,   234,   127,   234,    82,   101,     0,    95,   100,
    92,    82,     0,     0,   148,   147,   146,   152,   153,   156,
     0,     0,     0,   234,   170,   173,     0,   171,   169,   172,
   195,   196,   192,   200,   191,   199,   190,   198,   234,     0,
   186,     0,   188,   189,   164,   216,   222,     0,     0,   142,
   141,   140,    67,     0,     0,    69,   262,   259,   260,     0,
   255,   254,   299,   233,   303,   304,   310,   309,   308,     0,
   306,   315,   329,     0,   325,   321,   316,   328,   331,   317,
     0,     0,     0,   278,   234,   318,   322,   234,     0,   313,
   294,   290,   294,   294,   246,   245,     0,     0,     0,   248,
   234,   239,   240,   241,   120,   129,   121,    89,   103,    99,
     0,    97,    85,   163,   149,   160,   158,   161,   145,   174,
   175,     0,     0,   234,   202,     0,   143,     0,   155,     0,
   261,   234,   257,   256,   233,     0,   233,   307,   234,   278,
   326,     0,   337,   340,   343,   346,   294,   329,   316,   316,
   298,   112,   282,   285,   288,     0,     0,     0,     0,     0,
   251,   252,     0,   248,   235,    91,    96,     0,   197,   185,
     0,   137,   136,   135,   207,   206,     0,     0,   202,   209,
   132,   205,   134,   234,   234,   263,   253,     0,   269,     0,
   298,   327,   278,   233,   233,   233,   298,   320,   298,   336,
     0,     0,     0,   244,   264,   250,   247,   249,    93,    98,
   159,   187,     0,     0,   193,   201,   234,   203,    60,    59,
   272,   305,   330,   332,   350,     0,     0,     0,   335,   319,
   347,   242,   243,   265,   209,   205,   194,     0,   233,   338,
   341,   344,   204,   208,   351,   294,   294,   294,   298,   298,
   298,   339,   342,   345,     0,     0,     0
};

static const short yydefgoto[] = {   745,
    52,    53,    54,    75,    76,   108,    56,   285,    57,   165,
   246,   247,   310,   311,   498,   401,   304,   125,   126,   127,
   136,   137,   138,   153,   276,   273,   258,   357,    58,    59,
    60,   297,   389,   298,   305,    87,   446,    77,   410,   316,
   450,   317,   318,   319,    61,   156,   231,   232,   515,   516,
   422,   328,   329,   530,   531,   532,   668,   669,   670,   708,
    81,   236,   159,   160,   163,   164,   154,    62,    63,    64,
    65,    66,    67,   368,    78,    69,   485,   486,   487,   589,
   488,   653,   654,    70,   550,   455,   548,   549,   696,    71,
    72,   261,   179,   359,   101,   467,   468,   189,   266,   581,
   267,   583,   268,   584,   376,   377,   480,   269,   270,   185,
   263,   264,   265,   560,   561,   715,   370,   371,   565,   470,
   471,   566,   567,   568,   472,   569,   684,   736,   685,   737,
   686,   738,   474,   475,   716
};

static const short yypact[] = {  1093,
-32768,   101,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
    51,-32768,-32768,-32768,-32768,  1596,-32768,-32768,-32768,-32768,
-32768,   -21,-32768,  1196,   469,  1396,   320,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   312,-32768,-32768,
   469,    -3,    95,   109,-32768,   101,-32768,  1596,  1596,  1596,
-32768,  1596,  1596,  1596,   126,  1596,-32768,  1596,  1596,  1596,
  1596,  1596,   126,  1596,  1596,  1596,   153,-32768,    68,   112,
-32768,   209,   153,   153,  1596,  1596,  1596,   133,    88,  1596,
  1596,   153,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   137,   141,
-32768,-32768,-32768,-32768,-32768,   469,-32768,-32768,-32768,-32768,
-32768,    33,   189,   197,   701,   300,-32768,    32,   225,   269,
   253,   259,   276,   303,   224,   126,   312,   284,   279,  1296,
   314,   328,   337,   260,   341,   352,   365,   300,   318,-32768,
-32768,-32768,-32768,-32768,   324,-32768,-32768,-32768,-32768,   266,
   300,   173,-32768,-32768,-32768,-32768,   701,  1396,-32768,  1596,
  1596,  1596,  1596,  1596,  1596,  1596,  1596,  1596,  1596,  1596,
  1596,  1596,  1596,  1596,  1696,  1696,  1596,   348,   389,  1596,
   389,-32768,  1596,-32768,  1596,-32768,-32768,-32768,-32768,-32768,
   351,   300,  1596,-32768,  1596,   377,  1596,  1596,   469,  1596,
   354,-32768,  1596,  1596,-32768,   393,-32768,  1596,  1596,  1596,
-32768,-32768,-32768,  1596,-32768,-32768,-32768,   397,   351,-32768,
   280,   409,   380,   402,   324,    42,    42,    42,-32768,-32768,
   351,  1596,   370,  1596,  1396,   349,   701,   443,   723,   573,
   383,   502,   360,   360,-32768,   369,   178,-32768,-32768,   140,
   178,   178,   371,   378,   388,  1496,   452,   394,   399,   404,
   406,   452,   592,   312,   414,-32768,   348,-32768,-32768,   312,
   391,   482,   674,   312,   668,   516,    29,   473,   139,  1596,
-32768,-32768,   210,   210,  1596,  1596,-32768,   501,   351,-32768,
   484,  1596,-32768,-32768,-32768,   469,   469,-32768,   494,  1596,
-32768,-32768,   303,  1396,-32768,   543,   519,   523,-32768,-32768,
-32768,  1396,-32768,-32768,  1596,  1596,   531,   503,   280,   266,
-32768,-32768,-32768,   380,   380,   481,-32768,    59,   548,-32768,
   571,-32768,-32768,-32768,-32768,   496,   515,   490,   492,   381,
-32768,-32768,   701,-32768,-32768,  1596,-32768,-32768,   499,  1596,
   504,  1596,-32768,-32768,   153,-32768,-32768,-32768,   368,   126,
   389,  1596,-32768,-32768,-32768,  1596,-32768,-32768,-32768,-32768,
  1596,  1596,  1596,-32768,-32768,-32768,   516,-32768,-32768,-32768,
   210,   498,-32768,-32768,-32768,   210,  1396,  1396,   493,-32768,
-32768,  1596,-32768,-32768,-32768,-32768,  1596,-32768,-32768,   500,
-32768,  1596,-32768,-32768,-32768,-32768,-32768,-32768,   507,-32768,
   590,-32768,-32768,   126,    43,-32768,   324,   579,   581,    64,
   540,   270,  1596,-32768,    68,-32768,-32768,   247,-32768,-32768,
   690,   270,-32768,   266,-32768,-32768,   380,-32768,-32768,    42,
-32768,-32768,   268,   153,   583,   381,   381,   381,-32768,-32768,
-32768,-32768,-32768,  1496,-32768,   312,   348,   522,    53,-32768,
-32768,   312,  1596,   584,-32768,-32768,-32768,-32768,-32768,-32768,
  1596,  1596,  1596,-32768,-32768,   210,  1596,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,  1596,
   547,   530,-32768,-32768,-32768,-32768,-32768,  1596,   595,-32768,
-32768,-32768,-32768,  1596,  1596,-32768,   524,-32768,   126,   597,
    43,    43,   324,   380,-32768,-32768,-32768,-32768,-32768,   534,
    64,-32768,   114,   557,-32768,-32768,   270,-32768,-32768,-32768,
   153,   330,   265,   409,-32768,-32768,-32768,-32768,   541,-32768,
   266,-32768,   266,   266,-32768,-32768,   562,   563,  1596,   221,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   482,
   153,-32768,-32768,-32768,-32768,   230,   605,   567,-32768,-32768,
   701,   451,   629,-32768,   611,   630,-32768,   632,-32768,  1596,
-32768,-32768,-32768,-32768,   380,   633,   380,-32768,-32768,   409,
-32768,   593,-32768,-32768,-32768,-32768,   266,   266,   270,   270,
-32768,  1596,-32768,-32768,-32768,    73,    73,   634,   153,    73,
-32768,-32768,   638,   221,-32768,   389,   368,  1596,-32768,-32768,
   153,-32768,-32768,-32768,-32768,-32768,   576,    41,   611,   660,
-32768,   404,-32768,-32768,-32768,   701,-32768,   645,-32768,   649,
-32768,-32768,   409,   380,   380,   380,-32768,-32768,-32768,-32768,
   586,   652,   653,-32768,   153,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,    83,   594,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   515,   596,   591,   598,-32768,-32768,
-32768,-32768,-32768,-32768,   660,-32768,-32768,  1596,   380,-32768,
-32768,-32768,-32768,   701,-32768,   266,   266,   266,-32768,-32768,
-32768,-32768,-32768,-32768,   697,   700,-32768
};

static const short yypgoto[] = {-32768,
   647,   -13,-32768,   631,-32768,    89,   336,  -349,   358,   -10,
  -233,   640,  -216,-32768,  -461,   110,  -281,-32768,-32768,-32768,
-32768,-32768,-32768,   -40,  -258,   331,-32768,-32768,-32768,-32768,
-32768,  -196,   413,  -509,  -368,  -335,-32768,  -159,   299,    71,
   172,  -322,-32768,  -176,-32768,   635,    86,-32768,  -354,   164,
-32768,   -25,-32768,-32768,-32768,-32768,    54,-32768,    19,     0,
-32768,-32768,   491,-32768,   -68,   489,   508,   428,    35,   162,
-32768,   346,-32768,    96,   -15,-32768,   -67,-32768,  -572,  -154,
  -505,    80,-32768,-32768,   -35,-32768,   188,-32768,    44,   669,
  -358,-32768,-32768,-32768,  -208,    -9,    -1,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,  -219,-32768,    25,  -493,   -93,  -298,
  -416,   477,  -348,   182,-32768,  -240,   271,-32768,  -325,   275,
-32768,  -451,-32768,  -535,-32768,  -316,-32768,-32768,-32768,-32768,
-32768,-32768,   176,   488,  -480
};


#define	YYLAST		1803


static const short yytable[] = {    79,
    80,    82,    83,    84,   314,   109,   184,    85,    86,    88,
    89,    90,    91,    92,    93,    94,   384,   651,    95,   302,
   577,    96,   369,   173,    97,   396,   104,   105,   106,   107,
   499,   102,   451,   110,   111,   112,   492,   602,   639,   103,
   554,   115,   469,   139,   140,   113,   411,   378,   379,   171,
   172,   473,   361,   175,   176,   177,    12,   147,     6,   704,
     6,   412,   457,     9,    10,    12,     6,   362,    41,   393,
   394,   518,   506,    12,   363,   100,   398,   233,    12,   403,
   404,   651,   174,   223,   652,   440,   585,   643,    55,   644,
   645,   234,   224,   449,   682,    68,   551,    12,   149,   387,
   200,   201,   202,   203,   462,   667,   552,   204,   541,   205,
   206,   207,   150,   208,    41,   631,   262,   209,   210,   211,
   212,   213,   214,   458,   459,   217,   218,   219,   220,   221,
   499,   178,   222,     6,   463,   562,   625,   190,   191,   705,
   586,   374,    55,   687,   124,   469,   199,   714,   652,    68,
   456,    98,   128,   604,   473,   245,  -278,   553,   601,   667,
  -278,   610,  -278,   557,   558,   464,   152,    12,   519,   520,
   158,   161,   162,   180,   342,   157,   168,   169,   195,   272,
   162,   157,   665,   666,   501,   128,   196,   688,   689,   181,
   128,  -227,   551,   551,   333,   700,    73,    74,   182,   183,
   182,   183,   552,   552,   717,   718,   225,   346,   320,   348,
   322,   323,   324,   325,   326,   599,   414,     6,   332,   415,
     9,    10,   451,   416,    12,   338,   341,   131,   132,   133,
   226,   134,   135,   358,   215,   349,   350,   351,   216,   353,
   354,   355,   739,   740,   741,   380,   418,   505,   735,   300,
   300,   102,   307,   312,   235,   312,   239,   241,   124,   103,
   582,   649,   598,   259,   134,   135,   184,   133,   603,   134,
   135,   419,     6,   362,   420,     6,   271,     6,   362,   417,
   363,   585,    12,   540,    12,   363,   237,     6,   499,   650,
   484,   433,   274,   397,    12,   360,   365,   596,   238,   439,
   375,   375,   375,   431,   240,   303,   186,   414,   313,   242,
   187,   429,   188,   626,   416,   452,    51,   330,   434,   227,
   243,   331,   435,   436,   249,   158,   336,   244,   339,   441,
   300,   162,   128,   337,   606,   586,   608,   347,   228,   141,
   229,   307,   690,   248,  -277,   463,   142,   251,  -277,   102,
  -277,   461,   182,   183,   230,     6,   143,   103,     9,    10,
   381,   252,    12,   254,   144,   504,   489,  -278,   145,   491,
   253,  -278,   510,  -278,   255,     6,    51,   575,     9,    10,
   578,   128,   713,   691,   678,   256,   680,   257,   719,   503,
   720,   536,    88,   360,   507,   260,   537,   262,   511,   327,
   513,   539,   466,    12,   309,   129,   130,   131,   132,   133,
   334,   134,   135,   340,   535,   155,     6,   362,   592,   593,
   594,   155,   570,    12,   363,   170,   345,   633,    88,   497,
   542,   634,   543,   635,   438,   312,   356,   546,   367,   699,
   742,   743,   744,-32768,   123,   197,   198,   372,   571,   483,
   484,   572,   573,   385,   386,   130,   131,   132,   133,   579,
   134,   135,   192,   193,   194,    12,   659,   123,    99,   629,
   388,     2,   382,     3,     4,     5,  -137,   595,   274,   597,
   496,   508,   509,  -136,   424,   424,   421,   426,   399,   613,
   502,   692,   693,  -135,   559,    18,   563,   616,   609,   390,
     6,   521,   400,   618,  -132,   466,   563,    12,   522,  -133,
   512,  -134,   166,   612,   375,   623,   624,   588,   590,   395,
   166,   117,   118,   119,   120,   121,   122,   123,   300,   523,
   524,   307,   413,   275,   430,   277,   278,   279,   280,   281,
   282,   283,   284,   432,   629,   406,   407,   408,   648,   409,
   296,   296,   447,   437,   525,   526,   448,   527,   528,   640,
   301,   301,   641,   308,   453,   286,   287,   288,   289,   290,
   291,   292,   442,   443,   444,   655,   445,   454,   460,   344,
   315,   476,   424,-32768,-32768,   122,   123,   424,   477,   352,
   534,   479,   481,   166,   478,   559,   482,   493,   661,   529,
   607,   563,   495,   517,   538,   632,   677,   545,   638,   383,
   275,   544,   555,   681,   556,   463,   591,   605,     6,   141,
   600,     9,    10,   564,   614,    12,   142,   615,   617,   620,
   622,   296,   627,   564,   630,   657,   143,   392,   642,   646,
   647,   301,   299,   299,   144,   306,   199,   662,   145,   672,
   658,   416,   308,   119,   120,   121,   122,   123,   709,   710,
   427,   428,   660,   674,   638,   675,   679,   694,   425,   425,
   683,   697,   663,   563,   563,   664,   707,   424,   711,   275,
   128,   703,   712,   695,   721,   722,   723,   275,    99,   364,
   312,   728,   727,   731,   730,   702,   746,     6,   362,   747,
   114,   141,   732,   672,    12,   363,   151,   146,   142,   656,
   665,   666,   315,   490,   391,   514,   619,   638,   143,   402,
   167,   725,   706,   299,   733,   494,   144,   335,   564,   695,
   145,   343,   321,   698,   306,   462,   621,   726,   724,   729,
   148,   373,   628,   286,   308,   576,   701,   580,   637,   366,
   423,   423,     0,   405,   130,   131,   132,   133,     0,   134,
   135,     0,   275,   275,     0,   463,   425,   574,   315,   315,
     0,   425,     0,     0,     0,     0,   182,   183,   116,   117,
   118,   119,   120,   121,   122,   123,    99,  -278,     0,     0,
     0,  -278,     0,  -278,     0,   465,   464,     0,     0,     0,
   564,   564,   118,   119,   120,   121,   122,   123,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   306,   500,     0,   296,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   301,
     0,     0,   308,     0,   308,     0,     0,     0,   423,     0,
     0,     0,   611,   423,     0,     0,   533,     0,     0,     0,
   286,   425,     0,     0,     0,     0,     0,     0,   315,     0,
   315,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   547,     0,     0,     0,     0,     0,     0,     0,   465,
     0,     0,     0,     0,     0,     0,     0,     0,   465,   465,
     0,     0,   315,     0,     0,     0,     0,     0,     0,     0,
   587,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   299,     0,     0,   306,     0,   306,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   423,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   676,     0,     0,     0,     0,
   673,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   547,   275,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   465,     0,     0,     0,     0,     0,
   636,   465,   308,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   673,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,   671,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   465,     0,     0,
     0,     0,     0,   734,     0,     0,   465,   465,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   306,     0,     0,     0,     0,     0,
     0,     0,     0,     1,     0,     2,   671,     3,     4,     5,
     6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
   465,     0,     0,     0,    14,     0,    15,    16,    17,    18,
  -233,    19,     0,     0,     0,     0,     0,  -233,    20,    21,
    22,     0,     0,     0,    23,    24,    25,  -233,     0,    26,
    27,     0,    28,     0,    29,  -233,    30,     0,    31,  -233,
    32,    33,     0,    34,    35,    36,    37,    38,    39,   -11,
     0,     0,     0,    40,    41,    42,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    43,     0,     0,    44,    45,     0,     0,
    46,     0,    47,    48,    49,     0,    50,     0,     2,    51,
     3,     4,     5,     6,     7,     8,     9,    10,     0,    11,
    12,    13,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,    18,     0,    19,     0,     0,     0,     0,     0,
     0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,     0,    34,    35,    36,    37,
    38,    39,   -11,     0,     0,     0,    40,    41,    42,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    43,     0,     0,    44,
    45,     0,     0,    46,     0,    47,    48,    49,     0,    50,
     0,     0,    51,     6,     7,     8,     9,    10,     0,    11,
    12,    13,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,   250,    34,    35,    36,    37,
    38,    39,     0,     0,     0,     0,    40,     0,    42,     0,
     0,     0,     0,   116,   117,   118,   119,   120,   121,   122,
   123,     0,     0,     0,     0,     0,    43,     0,     0,    44,
    45,     0,     0,    46,     0,    47,    48,    49,     0,    50,
     0,     0,    51,     6,     7,     8,     9,    10,     0,    11,
    12,    13,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,     0,    34,    35,    36,    37,
    38,    39,     0,     0,     0,     0,    40,     0,    42,     0,
     0,     0,     0,   116,   117,   118,   119,   120,   121,   122,
   123,     0,     0,     0,     0,     0,    43,     0,     0,    44,
    45,     0,     0,    46,     0,    47,    48,    49,     0,    50,
     0,     0,    51,     6,     7,     8,     9,    10,     0,    11,
    12,    13,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,   293,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,     0,    34,   294,    36,    37,
   295,    39,     0,     0,     0,     0,    40,     0,    42,     0,
     0,     0,     0,   116,   117,   118,   119,   120,   121,   122,
   123,     0,     0,     0,     0,     0,    43,     0,     0,    44,
    45,     0,     0,    46,     0,    47,    48,    49,     0,    50,
     0,     0,    51,     6,     7,     8,     9,    10,     0,    11,
    12,    13,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,     0,    34,    35,    36,    37,
    38,    39,     0,     0,     0,     0,    40,     0,    42,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    43,     0,     0,    44,
    45,     0,     0,    46,     0,    47,    48,    49,     0,    50,
     0,     0,    51,     6,     7,     8,     9,    10,     0,    11,
    12,    13,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,   293,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,     0,    34,   294,    36,    37,
   295,    39,     0,     0,     0,     0,    40,     0,    42,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    43,     0,     0,    44,
    45,     0,     0,    46,     0,    47,    48,    49,     0,    50,
     0,     0,    51
};

static const short yycheck[] = {    15,
    16,    17,    18,    19,   221,    46,   100,    23,    24,    25,
    26,    27,    28,    29,    30,    31,   275,   590,    34,   216,
   472,    37,   263,    92,    40,   307,    42,    43,    44,    45,
   399,    41,   355,    49,    50,    51,   386,   499,   574,    41,
   457,    55,   368,    59,    60,    67,    18,   267,   268,    90,
    91,   368,   261,    94,    95,    96,    15,    71,     8,    19,
     8,    33,   361,    11,    12,    15,     8,     9,    72,   303,
   304,   426,   408,    15,    16,    41,   310,    46,    15,   313,
   314,   654,    93,    51,   590,   344,    14,   581,     0,   583,
   584,    60,    60,   352,   630,     0,   455,    15,     4,   296,
   116,   117,   118,   119,    46,   615,   455,   123,   444,   125,
   126,   127,     4,   129,    72,   567,    74,   133,   134,   135,
   136,   137,   138,   364,   365,   141,   142,   143,   144,   145,
   499,    97,   146,     8,    76,   461,   553,   103,   104,    99,
    68,   100,    54,   637,    56,   471,   112,   683,   654,    54,
   359,   101,    13,   503,   471,   166,    98,   456,   106,   669,
   102,   516,   104,   100,   101,   107,    78,    15,   427,   428,
    82,    83,    84,   106,   243,    80,    88,    89,    46,   195,
    92,    86,   100,   101,   401,    13,    99,   639,   640,    78,
    13,    78,   551,   552,   235,   657,    96,    97,    87,    88,
    87,    88,   551,   552,   685,   686,    18,   248,   224,   250,
   226,   227,   228,   229,   230,   497,    78,     8,   234,    81,
    11,    12,   545,    85,    15,   239,   242,    88,    89,    90,
    34,    92,    93,   259,    98,   251,   252,   253,    98,   255,
   256,   257,   736,   737,   738,   271,    37,   407,   729,   215,
   216,   261,   218,   219,    30,   221,   161,   162,   170,   261,
   480,    41,   496,   178,    92,    93,   360,    90,   502,    92,
    93,    62,     8,     9,    65,     8,   191,     8,     9,   320,
    16,    14,    15,   443,    15,    16,    18,     8,   657,    69,
    70,   332,   197,   309,    15,   261,   262,   494,    46,   340,
   266,   267,   268,   329,    46,   217,    98,    78,   220,    34,
   102,   327,   104,   554,    85,   356,   107,   232,   334,    20,
    18,   233,   336,   337,    46,   237,   238,   104,   240,   345,
   296,   243,    13,   238,   511,    68,   513,   249,    39,    28,
    41,   307,   641,    60,    98,    76,    35,    34,   102,   359,
   104,   367,    87,    88,    55,     8,    45,   359,    11,    12,
   272,    34,    15,   104,    53,   406,   382,    98,    57,   385,
    34,   102,   413,   104,    34,     8,   107,   471,    11,    12,
   474,    13,   681,   642,   625,    34,   627,    23,   687,   405,
   689,   432,   408,   359,   410,    78,   437,    74,   414,    49,
   416,   442,   368,    15,    16,    86,    87,    88,    89,    90,
    34,    92,    93,    60,   430,    80,     8,     9,   486,   487,
   488,    86,   463,    15,    16,    90,    34,    98,   444,   395,
   446,   102,   448,   104,   339,   401,    40,   453,    59,   656,
   739,   740,   741,    84,    85,   110,   111,    46,   464,    69,
    70,   467,   468,   105,    86,    87,    88,    89,    90,   475,
    92,    93,   105,   106,   107,    15,    16,    85,    41,   563,
    19,     3,   103,     5,     6,     7,   106,   493,   383,   495,
   392,   411,   412,   106,   323,   324,   323,   324,    98,   530,
   402,   646,   647,   106,   460,    27,   462,   538,   514,   106,
     8,     9,    21,   544,   106,   471,   472,    15,    16,   106,
   415,   106,    85,   529,   480,   551,   552,   483,   484,   106,
    93,    79,    80,    81,    82,    83,    84,    85,   494,    37,
    38,   497,    60,   198,    34,   200,   201,   202,   203,   204,
   205,   206,   207,    60,   638,    30,    31,    32,   589,    34,
   215,   216,    34,    60,    62,    63,    34,    65,    66,   575,
   215,   216,   578,   218,    34,   208,   209,   210,   211,   212,
   213,   214,    30,    31,    32,   591,    34,    75,    98,   244,
   223,    34,   421,    82,    83,    84,    85,   426,    18,   254,
   429,    77,   103,   166,    99,   561,   105,    99,   614,   107,
   512,   567,    99,   106,   105,   571,   622,    18,   574,   274,
   275,   105,    34,   629,    34,    76,    34,    34,     8,    28,
    99,    11,    12,   462,    78,    15,    35,    98,    34,   106,
    34,   296,    99,   472,    78,   601,    45,    46,    98,    78,
    78,   296,   215,   216,    53,   218,   612,    37,    57,   615,
    46,    85,   307,    81,    82,    83,    84,    85,   674,   675,
   325,   326,    34,    34,   630,    34,    34,    34,   323,   324,
    78,    34,    62,   639,   640,    65,    17,   516,    34,   344,
    13,   106,    34,   649,    99,    34,    34,   352,   261,   262,
   656,   707,    99,   103,    99,   661,     0,     8,     9,     0,
    54,    28,   105,   669,    15,    16,    76,    68,    35,   600,
   100,   101,   355,   383,   302,   417,   545,   683,    45,    46,
    86,   703,   669,   296,   725,   390,    53,   237,   567,   695,
    57,   243,   225,   654,   307,    46,   549,   703,   695,   715,
    72,   265,   561,   386,   399,   471,   658,   477,   573,   262,
   323,   324,    -1,    86,    87,    88,    89,    90,    -1,    92,
    93,    -1,   427,   428,    -1,    76,   421,    78,   411,   412,
    -1,   426,    -1,    -1,    -1,    -1,    87,    88,    78,    79,
    80,    81,    82,    83,    84,    85,   359,    98,    -1,    -1,
    -1,   102,    -1,   104,    -1,   368,   107,    -1,    -1,    -1,
   639,   640,    80,    81,    82,    83,    84,    85,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,   399,   400,    -1,   494,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   494,
    -1,    -1,   497,    -1,   499,    -1,    -1,    -1,   421,    -1,
    -1,    -1,   517,   426,    -1,    -1,   429,    -1,    -1,    -1,
   503,   516,    -1,    -1,    -1,    -1,    -1,    -1,   511,    -1,
   513,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,   454,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   462,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   471,   472,
    -1,    -1,   545,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
   483,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,   494,    -1,    -1,   497,    -1,   499,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,   516,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,   620,    -1,    -1,    -1,    -1,
   615,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,   549,   642,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,   567,    -1,    -1,    -1,    -1,    -1,
   573,   574,   657,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,   669,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,   615,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,   630,    -1,    -1,
    -1,    -1,    -1,   728,    -1,    -1,   639,   640,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,   657,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,     1,    -1,     3,   669,     5,     6,     7,
     8,     9,    10,    11,    12,    -1,    14,    15,    16,    -1,
   683,    -1,    -1,    -1,    22,    -1,    24,    25,    26,    27,
    28,    29,    -1,    -1,    -1,    -1,    -1,    35,    36,    37,
    38,    -1,    -1,    -1,    42,    43,    44,    45,    -1,    47,
    48,    -1,    50,    -1,    52,    53,    54,    -1,    56,    57,
    58,    59,    -1,    61,    62,    63,    64,    65,    66,    67,
    -1,    -1,    -1,    71,    72,    73,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    91,    -1,    -1,    94,    95,    -1,    -1,
    98,    -1,   100,   101,   102,    -1,   104,    -1,     3,   107,
     5,     6,     7,     8,     9,    10,    11,    12,    -1,    14,
    15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,
    25,    26,    27,    -1,    29,    -1,    -1,    -1,    -1,    -1,
    -1,    36,    37,    38,    -1,    -1,    -1,    42,    43,    44,
    -1,    -1,    47,    48,    -1,    50,    -1,    52,    -1,    54,
    -1,    56,    -1,    58,    59,    -1,    61,    62,    63,    64,
    65,    66,    67,    -1,    -1,    -1,    71,    72,    73,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    94,
    95,    -1,    -1,    98,    -1,   100,   101,   102,    -1,   104,
    -1,    -1,   107,     8,     9,    10,    11,    12,    -1,    14,
    15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,
    25,    26,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
    -1,    36,    37,    38,    -1,    -1,    -1,    42,    43,    44,
    -1,    -1,    47,    48,    -1,    50,    -1,    52,    -1,    54,
    -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
    65,    66,    -1,    -1,    -1,    -1,    71,    -1,    73,    -1,
    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
    85,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    94,
    95,    -1,    -1,    98,    -1,   100,   101,   102,    -1,   104,
    -1,    -1,   107,     8,     9,    10,    11,    12,    -1,    14,
    15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,
    25,    26,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
    -1,    36,    37,    38,    -1,    -1,    -1,    42,    43,    44,
    -1,    -1,    47,    48,    -1,    50,    -1,    52,    -1,    54,
    -1,    56,    -1,    58,    59,    -1,    61,    62,    63,    64,
    65,    66,    -1,    -1,    -1,    -1,    71,    -1,    73,    -1,
    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
    85,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    94,
    95,    -1,    -1,    98,    -1,   100,   101,   102,    -1,   104,
    -1,    -1,   107,     8,     9,    10,    11,    12,    -1,    14,
    15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,
    25,    26,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
    -1,    36,    37,    38,    -1,    -1,    -1,    42,    43,    44,
    -1,    -1,    47,    48,    -1,    50,    -1,    52,    -1,    54,
    -1,    56,    -1,    58,    59,    -1,    61,    62,    63,    64,
    65,    66,    -1,    -1,    -1,    -1,    71,    -1,    73,    -1,
    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
    85,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    94,
    95,    -1,    -1,    98,    -1,   100,   101,   102,    -1,   104,
    -1,    -1,   107,     8,     9,    10,    11,    12,    -1,    14,
    15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,
    25,    26,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
    -1,    36,    37,    38,    -1,    -1,    -1,    42,    43,    44,
    -1,    -1,    47,    48,    -1,    50,    -1,    52,    -1,    54,
    -1,    56,    -1,    58,    59,    -1,    61,    62,    63,    64,
    65,    66,    -1,    -1,    -1,    -1,    71,    -1,    73,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    94,
    95,    -1,    -1,    98,    -1,   100,   101,   102,    -1,   104,
    -1,    -1,   107,     8,     9,    10,    11,    12,    -1,    14,
    15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,
    25,    26,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
    -1,    36,    37,    38,    -1,    -1,    -1,    42,    43,    44,
    -1,    -1,    47,    48,    -1,    50,    -1,    52,    -1,    54,
    -1,    56,    -1,    58,    59,    -1,    61,    62,    63,    64,
    65,    66,    -1,    -1,    -1,    -1,    71,    -1,    73,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    94,
    95,    -1,    -1,    98,    -1,   100,   101,   102,    -1,   104,
    -1,    -1,   107
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
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 23:
{ yyval.t = newCTerm(PA_fAssign,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 24:
{ yyval.t = newCTerm(PA_fOrElse,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 25:
{ yyval.t = newCTerm(PA_fAndThen,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 26:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 27:
{ yyval.t = newCTerm(PA_fFdCompare,yyvsp[-2].t,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 28:
{ yyval.t = newCTerm(PA_fFdIn,yyvsp[-2].t,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 29:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 30:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 31:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
				  oz_cons(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 32:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 33:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 34:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
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
{ yyval.t = newCTerm(PA_fObjApply,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 38:
{ yyval.t = newCTerm(PA_fOpApply,AtomTilde,
				  oz_mklist(yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 39:
{ yyval.t = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 40:
{ yyval.t = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklist(yyvsp[-1].t,makeInt(xytext,pos())),pos()); ;
    break;}
case 41:
{ yyval.t = newCTerm(PA_fOpApply,AtomHat,
				  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 42:
{ yyval.t = newCTerm(PA_fAt,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 43:
{ yyval.t = newCTerm(PA_fOpApply,AtomDExcl,
				  oz_mklist(yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 44:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 45:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 46:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 47:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 48:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 49:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 50:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 51:
{ yyval.t = newCTerm(PA_fSelf,pos()); ;
    break;}
case 52:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 53:
{ yyval.t = yyvsp[0].t; ;
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
{ yyval.t = newCTerm(PA_fRecord,newCTerm(PA_fAtom,AtomCons,
						     makeLongPos(yyvsp[-4].t,yyvsp[0].t)),
				  oz_mklist(yyvsp[-3].t,yyvsp[-2].t)); ;
    break;}
case 58:
{ yyval.t = newCTerm(PA_fApply,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 59:
{ yyval.t = newCTerm(PA_fProc,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-7].t,makeLongPos(yyvsp[-8].t,yyvsp[0].t)); ;
    break;}
case 60:
{ yyval.t = newCTerm(PA_fFun,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-7].t,makeLongPos(yyvsp[-8].t,yyvsp[0].t)); ;
    break;}
case 61:
{ yyval.t = newCTerm(PA_fFunctor,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 62:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 63:
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t); ;
    break;}
case 64:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 65:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 66:
{ yyval.t = newCTerm(PA_fLock,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 67:
{ yyval.t = newCTerm(PA_fLockThen,yyvsp[-4].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 68:
{ yyval.t = newCTerm(PA_fThread,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 69:
{ yyval.t = newCTerm(PA_fTry,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 70:
{ yyval.t = newCTerm(PA_fRaise,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 71:
{ yyval.t = newCTerm(PA_fSkip,pos()); ;
    break;}
case 72:
{ yyval.t = newCTerm(PA_fFail,pos()); ;
    break;}
case 73:
{ yyval.t = newCTerm(PA_fNot,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 74:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 75:
{ yyval.t = newCTerm(PA_fOr,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 76:
{ yyval.t = newCTerm(PA_fDis,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 77:
{ yyval.t = newCTerm(PA_fChoice,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 78:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 79:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 80:
{ yyval.t = AtomNil; ;
    break;}
case 81:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 82:
{ yyval.t = AtomNil; ;
    break;}
case 83:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 84:
{ yyval.t = oz_cons(newCTerm(PA_fRequire,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 85:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 86:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-1].t,
					   newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 87:
{ yyval.t = oz_cons(newCTerm(PA_fImport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 88:
{ yyval.t = oz_cons(newCTerm(PA_fExport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 89:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 90:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-1].t,
					   newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 91:
{ yyval.t = AtomNil; ;
    break;}
case 92:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-2].t,AtomNil,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 93:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 94:
{ yyval.t = newCTerm(PA_fVar,OZ_atom(xytext),yyvsp[0].t); ;
    break;}
case 95:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 96:
{ yyval.t = oz_mklist(oz_pair2(yyvsp[0].t,yyvsp[-2].t)); ;
    break;}
case 97:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 98:
{ yyval.t = oz_cons(oz_pair2(yyvsp[-1].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 99:
{ yyval.t = PA_fNoImportAt; ;
    break;}
case 100:
{ yyval.t = newCTerm(PA_fImportAt,yyvsp[0].t); ;
    break;}
case 101:
{ yyval.t = AtomNil; ;
    break;}
case 102:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 103:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,
					   newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t)),yyvsp[0].t); ;
    break;}
case 104:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 105:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 106:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 107:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 108:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 109:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 110:
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 111:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 112:
{ yyval.t = AtomNil; ;
    break;}
case 113:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 114:
{ yyval.t = newCTerm(PA_fAtom,AtomNil,yyvsp[0].t); ;
    break;}
case 115:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomCons,yyvsp[-2].t),
				  oz_mklist(yyvsp[-1].t,yyvsp[0].t)); ;
    break;}
case 116:
{ yyval.t = PA_fNoCatch; ;
    break;}
case 117:
{ yyval.t = newCTerm(PA_fCatch,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 118:
{ yyval.t = PA_fNoFinally; ;
    break;}
case 119:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 120:
{
		    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
				  newCTerm(PA_fAtom,yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
		  ;
    break;}
case 121:
{
		    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
				  makeVar(yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
		  ;
    break;}
case 122:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 123:
{ yyval.t = NameUnit; ;
    break;}
case 124:
{ yyval.t = NameTrue; ;
    break;}
case 125:
{ yyval.t = NameFalse; ;
    break;}
case 126:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 127:
{ yyval.t = AtomNil; ;
    break;}
case 128:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 129:
{ yyval.t = oz_cons(newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 130:
{ yyval.t = NameFalse; ;
    break;}
case 131:
{ yyval.t = NameTrue; ;
    break;}
case 132:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 133:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 134:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 135:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 136:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 137:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 138:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 139:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 140:
{ yyval.t = newCTerm(PA_fBoolCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 141:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 142:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 143:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 144:
{ yyval.t = newCTerm(PA_fSkip,pos()); ;
    break;}
case 145:
{ checkDeprecation(yyvsp[-3].t);
		    yyval.t = newCTerm(PA_fBoolCase,yyvsp[-5].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-6].t,yyvsp[0].t));
		  ;
    break;}
case 146:
{ yyval.t = newCTerm(PA_fCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 147:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 148:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 149:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 150:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 151:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 152:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 153:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 154:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 155:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 156:
{ yyval.t = newCTerm(PA_fCaseClause,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 157:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 158:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-3].t,
				  newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 159:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-5].t,yyvsp[-2].t,yyvsp[0].t,yyvsp[-3].t); ;
    break;}
case 160:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 161:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 162:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 163:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
				  oz_cons(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 164:
{ yyval.t = newCTerm(PA_fClass,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 165:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 166:
{ yyval.t = newCTerm(PA_fDollar,yyvsp[0].t); ;
    break;}
case 167:
{ yyval.t = AtomNil; ;
    break;}
case 168:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 169:
{ yyval.t = newCTerm(PA_fFrom,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 170:
{ yyval.t = newCTerm(PA_fAttr,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 171:
{ yyval.t = newCTerm(PA_fFeat,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 172:
{ yyval.t = newCTerm(PA_fProp,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 173:
{ yyval.t = AtomNil; ;
    break;}
case 174:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 175:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 176:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 177:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 178:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 179:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 180:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 181:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 182:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 183:
{ yyval.t = AtomNil; ;
    break;}
case 184:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 185:
{ yyval.t = newCTerm(PA_fMeth,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-3].t); ;
    break;}
case 186:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 187:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 188:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 189:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 190:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 191:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 192:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 193:
{ yyval.t = newCTerm(PA_fRecord,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 194:
{ yyval.t = newCTerm(PA_fOpenRecord,yyvsp[-4].t,yyvsp[-2].t); ;
    break;}
case 195:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 196:
{ yyval.t = makeVar(xytext); ;
    break;}
case 197:
{ yyval.t = newCTerm(PA_fEscape,makeVar(xytext),yyvsp[-1].t); ;
    break;}
case 198:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 199:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 200:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 201:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 202:
{ yyval.t = AtomNil; ;
    break;}
case 203:
{ yyval.t = newCTerm(PA_fMethArg,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 204:
{ yyval.t = newCTerm(PA_fMethColonArg,yyvsp[-3].t,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 205:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 206:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 207:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 208:
{ yyval.t = newCTerm(PA_fDefault,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 209:
{ yyval.t = PA_fNoDefault; ;
    break;}
case 210:
{ yyval.t = newCTerm(PA_fCond,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 211:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 212:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 213:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 214:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 215:
{ yyval.t = newCTerm(PA_fClause,newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 216:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 217:
{ yyval.t = oz_mklist(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 218:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 219:
{ yyval.t = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,yyvsp[0].t),
				  yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 220:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-3].t,yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 221:
{ yyval.t = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 222:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 223:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 224:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 225:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 226:
{ yyval.t = makeVar(xytext); ;
    break;}
case 227:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 228:
{ yyval.t = newCTerm(PA_fEscape,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 229:
{ yyval.t = makeString(xytext,pos()); ;
    break;}
case 230:
{ yyval.t = makeInt(xytext,pos()); ;
    break;}
case 231:
{ yyval.t = makeInt(xytext[0],pos()); ;
    break;}
case 232:
{ yyval.t = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); ;
    break;}
case 233:
{ yyval.t = pos(); ;
    break;}
case 234:
{ yyval.t = pos(); ;
    break;}
case 235:
{ OZ_Term prefix =
		      scannerPrefix? scannerPrefix: PA_zy;
		    yyval.t = newCTerm(PA_fScanner,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,prefix,
				  makeLongPos(yyvsp[-6].t,yyvsp[0].t)); ;
    break;}
case 236:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 237:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 238:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 239:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 240:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 241:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 242:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 243:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 244:
{ yyval.t = newCTerm(PA_fLexicalRule,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 245:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 246:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 247:
{ yyval.t = newCTerm(PA_fMode,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 248:
{ yyval.t = AtomNil; ;
    break;}
case 249:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 250:
{ yyval.t = newCTerm(PA_fInheritedModes,yyvsp[0].t); ;
    break;}
case 251:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 252:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 253:
{ OZ_Term expect = parserExpect? parserExpect: newSmallInt(0);
		    yyval.t = newCTerm(PA_fParser,yyvsp[-6].t,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,expect,
				  makeLongPos(yyvsp[-7].t,yyvsp[0].t)); ;
    break;}
case 254:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 255:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 256:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 257:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 258:
{ yyval.t = newCTerm(PA_fToken,AtomNil); ;
    break;}
case 259:
{ yyval.t = newCTerm(PA_fToken,yyvsp[0].t); ;
    break;}
case 260:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 261:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 262:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 263:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 264:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 265:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 266:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 267:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 268:
{ *prodKey[depth]++ = '='; ;
    break;}
case 269:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 270:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 271:
{ *prodKey[depth]++ = '='; ;
    break;}
case 272:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 273:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,PA_none); ;
    break;}
case 274:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 275:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 276:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 279:
{ prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg(yyvsp[-1].t,0))); ;
    break;}
case 280:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 281:
{ depth--; ;
    break;}
case 282:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 283:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 284:
{ depth--; ;
    break;}
case 285:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 286:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 287:
{ depth--; ;
    break;}
case 288:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 289:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 290:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 291:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 292:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 293:
{ *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; ;
    break;}
case 296:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 297:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 298:
{ *prodKey[depth] = '\0';
		    yyval.t = oz_pair2(prodName[depth],OZ_string(prodKeyBuffer[depth]));
		    prodName[depth] = PA_none;
		    prodKey[depth] = prodKeyBuffer[depth];
		  ;
    break;}
case 299:
{ yyval.t = AtomNil; ;
    break;}
case 300:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 301:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 302:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 303:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 304:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 305:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 306:
{ yyval.t = AtomNil; ;
    break;}
case 307:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 308:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 309:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 310:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 311:
{ yyval.t = newCTerm(PA_fSynAlternative, yyvsp[0].t); ;
    break;}
case 312:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 313:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 314:
{ OZ_Term t = yyvsp[0].t;
		    while (terms[depth]) {
		      t = oz_cons(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    yyval.t = newCTerm(PA_fSynSequence, decls[depth], t, yyvsp[-1].t);
		    decls[depth] = AtomNil;
		  ;
    break;}
case 315:
{ yyval.t = newCTerm(PA_fSynSequence, AtomNil, yyvsp[0].t, yyvsp[-1].t); ;
    break;}
case 316:
{ yyval.t = AtomNil; ;
    break;}
case 317:
{ yyval.t = oz_mklist(newCTerm(PA_fSynAction,yyvsp[0].t)); ;
    break;}
case 318:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 319:
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
case 320:
{ yyval.t = oz_cons(newCTerm(PA_fSynAssignment, terms[depth]->term, yyvsp[-1].t),
				  yyvsp[0].t);
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  ;
    break;}
case 321:
{ while (terms[depth]) {
		      decls[depth] = oz_cons(terms[depth]->term, decls[depth]);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    yyval.t = yyvsp[0].t;
		  ;
    break;}
case 322:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 323:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 324:
{ terms[depth] = new TermNode(yyvsp[0].t, terms[depth]); ;
    break;}
case 325:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 326:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 327:
{ yyval.t = newCTerm(PA_fSynAssignment,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 328:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 329:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 330:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_cons(newCTerm(PA_fSynApplication,yyvsp[-3].t,
						    AtomNil),
					   AtomNil),yyvsp[-1].t);
		  ;
    break;}
case 331:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 332:
{ yyval.t = newCTerm(PA_fSynAssignment,
				  newCTerm(PA_fEscape,yyvsp[-2].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 333:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 334:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 335:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_mklist(yyvsp[-2].t),yyvsp[-3].t);
		  ;
    break;}
case 336:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_mklist(yyvsp[-3].t),yyvsp[-1].t);
		  ;
    break;}
case 337:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 338:
{ depth--; ;
    break;}
case 339:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 340:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 341:
{ depth--; ;
    break;}
case 342:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 343:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 344:
{ depth--; ;
    break;}
case 345:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 346:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 347:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[-4].t,yyvsp[-1].t); ;
    break;}
case 348:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 349:
{ yyval.t = makeVar(xytext); ;
    break;}
case 350:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 351:
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
  char *news;
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
