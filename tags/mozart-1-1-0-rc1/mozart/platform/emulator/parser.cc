
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
#define	T_for	322
#define	T_do	323
#define	T_ENDOFFILE	324
#define	T_REGEX	325
#define	T_lex	326
#define	T_mode	327
#define	T_parser	328
#define	T_prod	329
#define	T_scanner	330
#define	T_syn	331
#define	T_token	332
#define	T_REDUCE	333
#define	T_SEP	334
#define	T_ITER	335
#define	T_OOASSIGN	336
#define	T_DOTASSIGN	337
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

OZ_Term _PA_AtomTab[109];

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
};

void parser_init(void) {
   for (int i = 109; i--; )
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



#define	YYFINAL		776
#define	YYFLAG		-32768
#define	YYNTBASE	116

#define YYTRANSLATE(x) ((unsigned)(x) <= 348 ? yytranslate[x] : 255)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   115,     2,    93,   108,     2,     2,     2,   105,
   106,     2,   103,    97,   104,    99,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,   114,   113,     2,
    82,     2,     2,   101,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   109,     2,   110,   100,   107,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   111,    92,   112,    98,     2,     2,     2,     2,
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
    77,    78,    79,    80,    81,    83,    84,    85,    86,    87,
    88,    89,    90,    91,    94,    95,    96,   102
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
   323,   329,   337,   340,   342,   351,   358,   359,   362,   363,
   366,   367,   369,   374,   381,   386,   391,   396,   403,   408,
   409,   413,   420,   423,   425,   429,   432,   437,   438,   441,
   442,   445,   450,   452,   454,   456,   458,   460,   462,   467,
   469,   470,   473,   475,   479,   480,   484,   485,   488,   496,
   504,   506,   508,   510,   512,   514,   515,   518,   523,   524,
   526,   528,   530,   532,   534,   536,   538,   540,   542,   549,
   552,   555,   559,   561,   569,   576,   579,   582,   586,   588,
   590,   594,   598,   600,   604,   608,   610,   615,   622,   627,
   632,   634,   639,   647,   649,   651,   652,   655,   660,   665,
   670,   675,   676,   679,   683,   685,   687,   689,   691,   693,
   695,   697,   698,   701,   707,   709,   714,   716,   718,   720,
   722,   724,   729,   735,   737,   739,   743,   745,   747,   749,
   752,   753,   756,   761,   763,   765,   767,   771,   772,   778,
   781,   782,   784,   788,   793,   799,   803,   807,   810,   815,
   820,   826,   828,   832,   834,   836,   838,   842,   844,   846,
   848,   850,   851,   852,   861,   863,   865,   867,   870,   873,
   876,   882,   888,   893,   895,   897,   902,   903,   906,   909,
   911,   913,   923,   925,   927,   930,   933,   934,   937,   939,
   942,   944,   948,   950,   953,   955,   958,   959,   969,   970,
   971,   982,   989,   993,   996,   999,  1001,  1002,  1005,  1006,
  1007,  1014,  1015,  1016,  1023,  1024,  1025,  1032,  1034,  1038,
  1040,  1042,  1044,  1045,  1047,  1049,  1051,  1052,  1053,  1056,
  1058,  1061,  1066,  1071,  1079,  1080,  1083,  1085,  1087,  1089,
  1091,  1093,  1097,  1100,  1104,  1105,  1108,  1111,  1117,  1122,
  1125,  1128,  1130,  1132,  1134,  1137,  1141,  1143,  1145,  1150,
  1152,  1158,  1160,  1162,  1168,  1173,  1174,  1175,  1185,  1186,
  1187,  1197,  1198,  1199,  1209,  1211,  1217,  1219,  1221,  1223
};

static const short yyrhs[] = {   117,
    70,     0,     1,     0,   122,   118,     0,   209,   118,     0,
   118,     0,   193,   131,   118,     0,   119,   117,     0,    28,
   194,   122,    47,   193,   118,     0,    28,   194,   122,    47,
   122,   118,     0,    28,   194,   122,   193,   118,     0,     0,
     3,   120,     0,     5,     0,     6,     0,     7,     0,     0,
   121,   120,     0,   103,     4,     0,   104,     4,     0,   123,
     0,   123,   122,     0,   123,    82,   194,   123,     0,   123,
    84,   194,   123,     0,   123,    83,   194,   123,     0,   123,
    85,   194,   123,     0,   123,    86,   194,   123,     0,   123,
   137,   194,   123,     0,   123,   138,   194,   123,     0,   123,
   139,   194,   123,     0,   123,    92,   194,   123,     0,   125,
     0,   125,    93,   194,   124,     0,   125,     0,   125,    93,
   124,     0,   125,   140,   194,   125,     0,   125,   141,   194,
   125,     0,   125,   142,   194,   125,     0,   125,    97,   194,
   125,     0,    98,   194,   125,     0,   125,    99,   194,   125,
     0,   125,    13,     0,   125,   100,   194,   125,     0,   101,
   194,   125,     0,   102,   194,   125,     0,   105,   143,   106,
     0,   187,     0,   189,     0,   107,     0,    66,     0,    63,
     0,    38,     0,    59,     0,   108,     0,   190,     0,   191,
     0,   192,     0,   148,     0,   109,   194,   123,   145,   110,
   194,     0,   111,   194,   123,   144,   112,   194,     0,    55,
   194,   129,   111,   123,   144,   112,   143,    35,   194,     0,
    43,   194,   129,   111,   123,   144,   112,   143,    35,   194,
     0,    44,   194,   165,   130,    35,   194,     0,   164,     0,
    48,   194,   122,    47,   122,    35,     0,    45,   155,     0,
    23,   157,     0,    49,   194,   143,    35,   194,     0,    49,
   194,   123,    61,   143,    35,   194,     0,    62,   194,   143,
    35,   194,     0,    65,   194,   143,   146,   147,    35,   194,
     0,    57,   194,   143,    35,   194,     0,    60,     0,    37,
     0,    51,   194,   143,    35,   194,     0,    27,   180,     0,
    53,   194,   184,    35,   194,     0,    30,   194,   184,    35,
   194,     0,    25,   194,   186,    35,   194,     0,   195,     0,
   203,     0,    89,   194,   144,    90,   194,     0,    68,   194,
   126,    69,   143,    35,   194,     0,   126,   127,     0,   127,
     0,   188,    47,   194,   123,    20,   123,   128,   194,     0,
   188,    47,   194,   123,   128,   194,     0,     0,   113,   123,
     0,     0,   187,   129,     0,     0,   131,     0,    58,   194,
   132,   130,     0,    54,   194,   122,    47,   122,   130,     0,
    54,   194,   122,   130,     0,    46,   194,   132,   130,     0,
    36,   194,   136,   130,     0,    29,   194,   122,    47,   122,
   130,     0,    29,   194,   122,   130,     0,     0,   188,   135,
   132,     0,   133,   105,   134,   106,   135,   132,     0,    16,
   194,     0,   154,     0,   154,   114,   188,     0,   154,   134,
     0,   154,   114,   188,   134,     0,     0,    22,   187,     0,
     0,   188,   136,     0,   154,   114,   188,   136,     0,    87,
     0,    88,     0,    91,     0,    94,     0,    95,     0,    96,
     0,   122,    47,   194,   122,     0,   122,     0,     0,   123,
   144,     0,   193,     0,   193,   123,   145,     0,     0,    24,
   194,   160,     0,     0,    41,   143,     0,   149,   194,   105,
   151,   152,   106,   194,     0,   150,   194,   105,   151,   152,
   106,   194,     0,     9,     0,    67,     0,    64,     0,    39,
     0,    16,     0,     0,   123,   151,     0,   153,   114,   123,
   151,     0,     0,    19,     0,   187,     0,   188,     0,   191,
     0,    66,     0,    63,     0,    38,     0,   187,     0,   191,
     0,   194,   122,    61,   143,   156,   194,     0,    33,   155,
     0,    32,   157,     0,    31,   143,    35,     0,    35,     0,
   194,   122,    61,   194,   143,   158,   194,     0,   194,   122,
    52,   159,   158,   194,     0,    33,   155,     0,    32,   157,
     0,    31,   143,    35,     0,    35,     0,   161,     0,   161,
    18,   159,     0,   161,    34,   159,     0,   161,     0,   161,
    18,   160,     0,   162,    61,   143,     0,   163,     0,   163,
    86,   193,   122,     0,   163,    86,   193,   122,    47,   122,
     0,   163,    82,   194,   163,     0,   163,    92,   194,   163,
     0,   125,     0,   125,    93,   194,   124,     0,    26,   194,
   165,   166,   171,    35,   194,     0,   123,     0,   193,     0,
     0,   167,   166,     0,    42,   194,   123,   144,     0,    21,
   194,   169,   168,     0,    40,   194,   169,   168,     0,    56,
   194,   123,   144,     0,     0,   169,   168,     0,   170,   114,
   123,     0,   170,     0,   187,     0,   189,     0,   191,     0,
    66,     0,    63,     0,    38,     0,     0,   172,   171,     0,
    50,   194,   173,   143,    35,     0,   174,     0,   174,    82,
   194,   188,     0,   187,     0,   189,     0,    66,     0,    63,
     0,    38,     0,   175,   105,   176,   106,     0,   175,   105,
   176,    19,   106,     0,     9,     0,    16,     0,   115,   194,
    16,     0,    67,     0,    64,     0,    39,     0,   177,   176,
     0,     0,   178,   179,     0,   153,   114,   178,   179,     0,
   188,     0,   108,     0,   107,     0,    17,   194,   123,     0,
     0,   194,   182,   181,    35,   194,     0,    31,   143,     0,
     0,   183,     0,   183,    18,   182,     0,   122,    61,   194,
   143,     0,   122,    47,   122,    61,   143,     0,   185,    18,
   185,     0,   185,    18,   184,     0,   122,   193,     0,   122,
    47,   122,   193,     0,   122,   193,    61,   143,     0,   122,
    47,   122,    61,   143,     0,   143,     0,   143,    18,   186,
     0,     8,     0,    15,     0,   188,     0,   115,   194,   188,
     0,    14,     0,    11,     0,    12,     0,    10,     0,     0,
     0,    76,   194,   188,   166,   171,   196,    35,   194,     0,
   197,     0,   198,     0,   200,     0,   197,   196,     0,   198,
   196,     0,   200,   196,     0,    72,   187,    82,   199,    35,
     0,    72,   188,    82,   199,    35,     0,    72,   199,   143,
    35,     0,    71,     0,    14,     0,    73,   188,   201,    35,
     0,     0,   202,   201,     0,    42,   208,     0,   198,     0,
   200,     0,    74,   194,   188,   166,   171,   205,   204,    35,
   194,     0,   232,     0,   210,     0,   232,   204,     0,   210,
   204,     0,     0,    78,   206,     0,   207,     0,   207,   206,
     0,   187,     0,   187,   114,   123,     0,   188,     0,   188,
   208,     0,   210,     0,   210,   209,     0,     0,    75,   188,
    82,   211,   214,   229,   230,   235,    35,     0,     0,     0,
    75,   108,   212,    82,   213,   214,   229,   230,   235,    35,
     0,    75,   214,   229,   230,   235,    35,     0,   216,   188,
   227,     0,   188,   228,     0,   215,   217,     0,   216,     0,
     0,   187,   114,     0,     0,     0,   105,   218,   224,   106,
   219,   227,     0,     0,     0,   109,   220,   224,   110,   221,
   227,     0,     0,     0,   111,   222,   224,   112,   223,   227,
     0,   225,     0,   225,   226,   224,     0,   188,     0,   107,
     0,    80,     0,     0,   228,     0,    94,     0,    95,     0,
     0,     0,   231,    47,     0,   232,     0,   232,   231,     0,
    77,   187,   235,    35,     0,    77,   188,   235,    35,     0,
    77,   253,   105,   233,   106,   235,    35,     0,     0,   234,
   233,     0,   188,     0,   108,     0,   107,     0,   236,     0,
   237,     0,   237,    18,   236,     0,   193,   239,     0,    60,
   194,   238,     0,     0,    79,   143,     0,   240,   239,     0,
   240,   228,   194,   241,   229,     0,   240,    82,   243,   241,
     0,    47,   241,     0,   244,   241,     0,   238,     0,   188,
     0,   238,     0,   242,   241,     0,   189,    82,   243,     0,
   243,     0,   188,     0,   188,   228,   194,   229,     0,   245,
     0,   115,   194,   188,    82,   243,     0,   245,     0,   252,
     0,   216,   194,   252,   227,   229,     0,   252,   228,   194,
   229,     0,     0,     0,   215,   194,   105,   246,   254,   106,
   247,   227,   229,     0,     0,     0,   215,   194,   109,   248,
   254,   110,   249,   227,   229,     0,     0,     0,   215,   194,
   111,   250,   254,   112,   251,   227,   229,     0,   187,     0,
   253,   194,   105,   144,   106,     0,     9,     0,    16,     0,
   235,     0,   235,   226,   254,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   679,   681,   685,   687,   690,   692,   697,   699,   702,   704,
   707,   711,   713,   715,   717,   721,   723,   727,   734,   743,
   745,   749,   751,   753,   755,   757,   759,   762,   764,   766,
   768,   770,   776,   778,   782,   785,   788,   791,   793,   796,
   799,   802,   805,   807,   810,   812,   814,   816,   818,   820,
   822,   824,   826,   828,   830,   832,   834,   836,   840,   842,
   845,   848,   850,   852,   854,   856,   858,   860,   862,   864,
   866,   868,   870,   872,   874,   876,   878,   880,   882,   884,
   886,   888,   894,   898,   901,   915,   939,   940,   944,   946,
   951,   953,   958,   960,   962,   965,   967,   969,   971,   976,
   978,   980,   984,   988,   990,   992,   994,   998,  1000,  1004,
  1006,  1008,  1013,  1017,  1021,  1025,  1029,  1033,  1037,  1039,
  1043,  1045,  1049,  1051,  1057,  1059,  1063,  1065,  1069,  1074,
  1081,  1083,  1085,  1087,  1091,  1095,  1097,  1099,  1103,  1105,
  1109,  1111,  1113,  1115,  1117,  1119,  1123,  1125,  1129,  1133,
  1135,  1137,  1139,  1143,  1147,  1151,  1153,  1155,  1157,  1161,
  1163,  1165,  1169,  1171,  1175,  1179,  1181,  1184,  1188,  1190,
  1192,  1194,  1200,  1205,  1207,  1212,  1214,  1218,  1220,  1222,
  1224,  1228,  1230,  1234,  1236,  1240,  1242,  1244,  1246,  1248,
  1250,  1254,  1256,  1260,  1264,  1266,  1270,  1272,  1274,  1276,
  1278,  1280,  1282,  1286,  1288,  1290,  1292,  1294,  1296,  1300,
  1302,  1306,  1308,  1312,  1314,  1316,  1321,  1323,  1327,  1331,
  1333,  1337,  1339,  1343,  1345,  1349,  1351,  1355,  1359,  1361,
  1364,  1368,  1370,  1374,  1378,  1382,  1384,  1388,  1392,  1394,
  1398,  1402,  1406,  1416,  1424,  1426,  1428,  1430,  1432,  1434,
  1438,  1440,  1444,  1448,  1450,  1454,  1458,  1460,  1464,  1466,
  1468,  1474,  1482,  1484,  1486,  1488,  1492,  1494,  1498,  1500,
  1504,  1506,  1510,  1512,  1516,  1518,  1522,  1524,  1526,  1527,
  1528,  1530,  1534,  1536,  1538,  1542,  1543,  1546,  1550,  1551,
  1551,  1552,  1553,  1553,  1554,  1555,  1555,  1558,  1560,  1564,
  1565,  1568,  1572,  1573,  1576,  1577,  1580,  1588,  1590,  1594,
  1596,  1600,  1602,  1604,  1608,  1610,  1614,  1616,  1618,  1622,
  1626,  1628,  1632,  1641,  1645,  1647,  1651,  1653,  1663,  1668,
  1675,  1677,  1681,  1685,  1687,  1691,  1693,  1697,  1699,  1705,
  1709,  1712,  1717,  1719,  1723,  1727,  1728,  1729,  1731,  1732,
  1733,  1735,  1736,  1737,  1741,  1743,  1747,  1749,  1754,  1756
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
"T_TRUE_LABEL","T_try","T_unit","T_UNIT_LABEL","T_for","T_do","T_ENDOFFILE",
"T_REGEX","T_lex","T_mode","T_parser","T_prod","T_scanner","T_syn","T_token",
"T_REDUCE","T_SEP","T_ITER","'='","T_OOASSIGN","T_DOTASSIGN","T_orelse","T_andthen",
"T_COMPARE","T_FDCOMPARE","T_LMACRO","T_RMACRO","T_FDIN","'|'","'#'","T_ADD",
"T_FDMUL","T_OTHERMUL","','","'~'","'.'","'^'","'@'","T_DEREFF","'+'","'-'",
"'('","')'","'_'","'$'","'['","']'","'{'","'}'","';'","':'","'!'","file","queries",
"queries1","directive","switchList","switch","sequence","phrase","hashes","phrase2",
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
   116,   116,   117,   117,   117,   117,   118,   118,   118,   118,
   118,   119,   119,   119,   119,   120,   120,   121,   121,   122,
   122,   123,   123,   123,   123,   123,   123,   123,   123,   123,
   123,   123,   124,   124,   125,   125,   125,   125,   125,   125,
   125,   125,   125,   125,   125,   125,   125,   125,   125,   125,
   125,   125,   125,   125,   125,   125,   125,   125,   125,   125,
   125,   125,   125,   125,   125,   125,   125,   125,   125,   125,
   125,   125,   125,   125,   125,   125,   125,   125,   125,   125,
   125,   125,   126,   126,   127,   127,   128,   128,   129,   129,
   130,   130,   131,   131,   131,   131,   131,   131,   131,   132,
   132,   132,   133,   134,   134,   134,   134,   135,   135,   136,
   136,   136,   137,   138,   139,   140,   141,   142,   143,   143,
   144,   144,   145,   145,   146,   146,   147,   147,   148,   148,
   149,   149,   149,   149,   150,   151,   151,   151,   152,   152,
   153,   153,   153,   153,   153,   153,   154,   154,   155,   156,
   156,   156,   156,   157,   157,   158,   158,   158,   158,   159,
   159,   159,   160,   160,   161,   162,   162,   162,   163,   163,
   163,   163,   164,   165,   165,   166,   166,   167,   167,   167,
   167,   168,   168,   169,   169,   170,   170,   170,   170,   170,
   170,   171,   171,   172,   173,   173,   174,   174,   174,   174,
   174,   174,   174,   175,   175,   175,   175,   175,   175,   176,
   176,   177,   177,   178,   178,   178,   179,   179,   180,   181,
   181,   182,   182,   183,   183,   184,   184,   185,   185,   185,
   185,   186,   186,   187,   188,   189,   189,   190,   191,   191,
   192,   193,   194,   195,   196,   196,   196,   196,   196,   196,
   197,   197,   198,   199,   199,   200,   201,   201,   202,   202,
   202,   203,   204,   204,   204,   204,   205,   205,   206,   206,
   207,   207,   208,   208,   209,   209,   211,   210,   212,   213,
   210,   210,   214,   214,   214,   215,   215,   216,   218,   219,
   217,   220,   221,   217,   222,   223,   217,   224,   224,   225,
   225,   226,   227,   227,   228,   228,   229,   230,   230,   231,
   231,   232,   232,   232,   233,   233,   234,   234,   234,   235,
   236,   236,   237,   237,   238,   238,   239,   239,   239,   239,
   239,   239,   240,   241,   241,   242,   242,   243,   243,   243,
   244,   244,   245,   245,   245,   246,   247,   245,   248,   249,
   245,   250,   251,   245,   252,   252,   253,   253,   254,   254
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
     5,     7,     2,     1,     8,     6,     0,     2,     0,     2,
     0,     1,     4,     6,     4,     4,     4,     6,     4,     0,
     3,     6,     2,     1,     3,     2,     4,     0,     2,     0,
     2,     4,     1,     1,     1,     1,     1,     1,     4,     1,
     0,     2,     1,     3,     0,     3,     0,     2,     7,     7,
     1,     1,     1,     1,     1,     0,     2,     4,     0,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     6,     2,
     2,     3,     1,     7,     6,     2,     2,     3,     1,     1,
     3,     3,     1,     3,     3,     1,     4,     6,     4,     4,
     1,     4,     7,     1,     1,     0,     2,     4,     4,     4,
     4,     0,     2,     3,     1,     1,     1,     1,     1,     1,
     1,     0,     2,     5,     1,     4,     1,     1,     1,     1,
     1,     4,     5,     1,     1,     3,     1,     1,     1,     2,
     0,     2,     4,     1,     1,     1,     3,     0,     5,     2,
     0,     1,     3,     4,     5,     3,     3,     2,     4,     4,
     5,     1,     3,     1,     1,     1,     3,     1,     1,     1,
     1,     0,     0,     8,     1,     1,     1,     2,     2,     2,
     5,     5,     4,     1,     1,     4,     0,     2,     2,     1,
     1,     9,     1,     1,     2,     2,     0,     2,     1,     2,
     1,     3,     1,     2,     1,     2,     0,     9,     0,     0,
    10,     6,     3,     2,     2,     1,     0,     2,     0,     0,
     6,     0,     0,     6,     0,     0,     6,     1,     3,     1,
     1,     1,     0,     1,     1,     1,     0,     0,     2,     1,
     2,     4,     4,     7,     0,     2,     1,     1,     1,     1,
     1,     3,     2,     3,     0,     2,     2,     5,     4,     2,
     2,     1,     1,     1,     2,     3,     1,     1,     4,     1,
     5,     1,     1,     5,     4,     0,     0,     9,     0,     0,
     9,     0,     0,     9,     1,     5,     1,     1,     1,     3
};

static const short yydefact[] = {     0,
     2,    16,    13,    14,    15,   234,   131,   241,   239,   240,
   238,   235,   135,   243,   243,   243,   243,   243,   243,    73,
    51,   134,   243,   243,   243,   243,   243,   243,   243,   243,
   243,    52,    72,   243,    50,   133,   243,    49,   132,   243,
   243,   287,   243,   243,   243,   243,   243,     0,    48,    53,
   243,   243,   243,     0,     5,   242,    11,    20,    31,    57,
   243,   243,    63,    46,   236,    47,    54,    55,    56,     0,
    79,    80,    11,   275,     0,     0,    12,    16,    66,     0,
     0,   242,    75,     0,     0,     0,    89,   242,    65,     0,
     0,     0,     0,     0,    89,     0,     0,     0,     0,     0,
   279,     0,     0,   307,     0,   286,     0,   121,     0,     0,
     0,   120,     0,     0,     0,     0,     1,     7,     3,   243,
   243,   243,   243,   243,   113,   114,   115,   243,    21,   243,
   243,   243,    41,   243,   116,   117,   118,   243,   243,   243,
   243,   243,   243,     0,     0,   243,   243,   243,   243,   243,
    11,     4,   276,    18,    19,    17,     0,   232,     0,   174,
   176,   175,     0,   221,   222,   242,   242,     0,     0,     0,
    89,    91,     0,     0,    20,     0,     0,     0,     0,     0,
     0,   125,     0,    84,     0,   176,     0,   288,   277,   305,
   306,   284,   308,   289,   292,   295,   285,   303,   176,   121,
     0,    39,    43,    44,   243,    45,   242,   121,   237,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   136,   136,     0,   110,   100,
     0,   100,     6,     0,   243,     0,   243,   243,   243,   243,
   243,   192,   176,     0,   243,     0,     0,     0,   242,    11,
     0,   228,   243,     0,     0,    90,     0,    92,     0,     0,
     0,   243,   243,   243,     0,   243,   243,   243,   127,     0,
    83,   243,   192,   280,   287,     0,   242,     0,   310,     0,
     0,     0,   283,   304,   192,   122,   243,     0,     0,   123,
     0,    22,    24,    23,    25,    26,    30,    27,    28,    29,
    32,    33,    38,    40,    42,    35,    36,    37,    51,    50,
    49,   136,   139,     0,    46,   236,    55,   139,    91,    91,
     0,   147,   110,   148,   243,    91,     0,   108,    91,    91,
   171,     0,   160,     0,   166,     0,   233,    78,     0,     0,
     0,     0,   243,     0,   192,   177,     0,     0,   220,   243,
   223,    11,    11,    10,   242,     0,    77,   227,   226,   121,
   243,     0,     0,     0,    67,    74,    76,   121,    71,    69,
     0,     0,     0,     0,     0,   267,   287,     0,   307,   357,
   358,   242,   242,     0,   243,   325,     0,   320,   321,   309,
   311,   301,   300,     0,   298,     0,     0,     0,    81,   119,
   243,   242,   243,     0,   137,   140,     0,     0,     0,     0,
    99,    97,     0,   111,   103,    96,     0,     0,   100,     0,
    95,    93,   243,     0,   243,   243,   159,   243,     0,     0,
     0,   243,   242,   243,     0,   191,   190,   189,   182,   185,
   186,   187,   188,   182,   121,   121,     0,   243,   193,     0,
   224,   219,     9,     8,     0,   229,   230,     0,    62,     0,
   243,   243,   153,   243,    64,   243,     0,   126,   163,   128,
   243,   243,    87,     0,     0,   307,   308,     0,     0,   315,
   325,   325,     0,   243,   355,   333,   243,   243,   332,   323,
   325,   325,   342,   343,   243,   282,   242,   290,   302,     0,
   293,   296,     0,     0,     0,   245,   246,   247,    58,   124,
    59,    34,   243,   136,   243,    91,   110,     0,   104,   109,
   101,    91,     0,     0,   157,   156,   155,   161,   162,   165,
     0,     0,     0,   243,   179,   182,     0,   180,   178,   181,
   204,   205,   201,   209,   200,   208,   199,   207,   243,     0,
   195,     0,   197,   198,   173,   225,   231,     0,     0,   151,
   150,   149,    68,     0,     0,    70,    82,     0,     0,   243,
   271,   268,   269,     0,   264,   263,   308,   242,   312,   313,
   319,   318,   317,     0,   315,   324,   338,     0,   334,   330,
   325,   337,   340,   326,     0,     0,     0,   287,   243,   327,
   331,   243,     0,   322,   303,   299,   303,   303,   255,   254,
     0,     0,     0,   257,   243,   248,   249,   250,   129,   138,
   130,    98,   112,   108,     0,   106,    94,   172,   158,   169,
   167,   170,   154,   183,   184,     0,     0,   243,   211,     0,
   152,     0,   164,    87,    88,    86,     0,   270,   243,   266,
   265,   242,     0,   242,   316,   243,   287,   335,     0,   346,
   349,   352,   355,   303,   338,   325,   325,   307,   121,   291,
   294,   297,     0,     0,     0,     0,     0,   260,   261,     0,
   257,   244,   100,   105,     0,   206,   194,     0,   146,   145,
   144,   216,   215,     0,     0,   211,   218,   141,   214,   143,
   243,   243,   243,   272,   262,     0,   278,     0,   307,   336,
   287,   242,   242,   242,   307,   329,   307,   345,     0,     0,
     0,   253,   273,   259,   256,   258,   102,   107,   168,   196,
     0,     0,   202,   210,   243,   212,    61,    60,    85,   281,
   314,   339,   341,   359,     0,     0,     0,   344,   328,   356,
   251,   252,   274,   218,   214,   203,     0,   242,   347,   350,
   353,   213,   217,   360,   303,   303,   303,   307,   307,   307,
   348,   351,   354,     0,     0,     0
};

static const short yydefgoto[] = {   774,
    54,    55,    56,    77,    78,   112,    58,   301,    59,   183,
   184,   570,   170,   257,   258,   326,   327,   518,   419,   320,
   130,   131,   132,   141,   142,   143,   158,   201,   289,   269,
   373,    60,    61,    62,   313,   407,   314,   321,    89,   464,
    79,   428,   332,   468,   333,   334,   335,    63,   161,   242,
   243,   535,   536,   440,   344,   345,   550,   551,   552,   695,
   696,   697,   736,    83,   247,   164,   165,   168,   169,   159,
    64,    65,    66,    67,    68,    69,   386,    80,    71,   505,
   506,   507,   613,   508,   680,   681,    72,   574,   475,   572,
   573,   724,    73,    74,   275,   187,   377,   104,   487,   488,
   197,   280,   605,   281,   607,   282,   608,   394,   395,   500,
   283,   284,   193,   277,   278,   279,   584,   585,   744,   388,
   389,   589,   490,   491,   590,   591,   592,   492,   593,   712,
   765,   713,   766,   714,   767,   494,   495,   745
};

static const short yypact[] = {  1068,
-32768,   112,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,    69,-32768,-32768,-32768,-32768,-32768,   865,-32768,-32768,
-32768,-32768,-32768,    20,-32768,  1179,   276,  1395,   647,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   385,
-32768,-32768,   276,    19,    95,   133,-32768,   112,-32768,   865,
   865,   865,-32768,   865,   865,   865,   159,   865,-32768,   865,
   865,   865,   865,   865,   159,   865,   865,   865,   184,   184,
-32768,    81,   203,-32768,   108,   184,   184,   865,   865,   865,
   865,   182,   127,   865,   865,   184,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   140,   160,-32768,-32768,-32768,-32768,-32768,
   276,-32768,-32768,-32768,-32768,-32768,    36,   251,   243,   947,
   263,-32768,    12,   255,   281,   247,   259,   279,   307,   218,
   159,   385,   271,   296,  1287,   303,   318,   322,   250,   337,
   350,   380,    83,-32768,   360,   263,   330,-32768,-32768,-32768,
-32768,-32768,   338,-32768,-32768,-32768,-32768,   180,   263,  1395,
   335,   101,-32768,-32768,-32768,-32768,   947,  1395,-32768,   865,
   865,   865,   865,   865,   865,   865,   865,   865,   865,   865,
   865,   865,   865,   865,   865,  1611,  1611,   865,   551,   305,
   865,   305,-32768,   865,-32768,   865,-32768,-32768,-32768,-32768,
-32768,   370,   263,   865,-32768,   865,   400,   865,   865,   276,
   865,   376,-32768,   865,   865,-32768,   406,-32768,   865,   865,
   865,-32768,-32768,-32768,   865,-32768,-32768,-32768,   409,   865,
-32768,-32768,   370,-32768,   219,   394,   408,   413,   338,    66,
    66,    66,-32768,-32768,   370,-32768,-32768,   865,   356,   865,
   365,   947,   959,   441,   466,   519,   389,   510,   309,   309,
-32768,   737,    43,-32768,-32768,   171,    43,    43,   371,   373,
   374,  1503,   471,   378,   383,   396,   399,   471,   287,   385,
   401,-32768,   551,-32768,-32768,   385,   411,   498,   308,   385,
   753,   560,   178,   461,    10,   865,-32768,-32768,    55,    55,
   865,   865,-32768,   488,   370,-32768,   474,   865,-32768,-32768,
-32768,   276,   276,-32768,   475,   865,-32768,-32768,   307,  1395,
-32768,   700,   504,   507,-32768,-32768,-32768,  1395,-32768,-32768,
   865,   865,   509,   512,   865,   470,   219,   180,-32768,-32768,
-32768,   408,   408,   444,-32768,   709,   530,-32768,   550,-32768,
-32768,-32768,-32768,   463,   491,   465,   464,   295,-32768,-32768,
-32768,   947,-32768,   865,-32768,-32768,   472,   865,   477,   865,
-32768,-32768,   184,-32768,-32768,-32768,   379,   159,   305,   865,
-32768,-32768,-32768,   865,-32768,-32768,-32768,-32768,   865,   865,
   865,-32768,-32768,-32768,   560,-32768,-32768,-32768,    55,   476,
-32768,-32768,-32768,    55,  1395,  1395,   522,-32768,-32768,   865,
-32768,-32768,-32768,-32768,   865,-32768,-32768,   496,-32768,   865,
-32768,-32768,-32768,-32768,-32768,-32768,   497,-32768,   596,-32768,
-32768,-32768,   914,   159,   234,-32768,   338,   586,   589,    61,
   543,   604,   865,-32768,    81,-32768,-32768,   269,-32768,-32768,
   669,   604,-32768,   180,-32768,-32768,   408,-32768,-32768,    66,
-32768,-32768,   229,   184,   590,   295,   295,   295,-32768,-32768,
-32768,-32768,-32768,  1503,-32768,   385,   551,   520,    37,-32768,
-32768,   385,   865,   592,-32768,-32768,-32768,-32768,-32768,-32768,
   865,   865,   865,-32768,-32768,    55,   865,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   865,
   548,   526,-32768,-32768,-32768,-32768,-32768,   865,   600,-32768,
-32768,-32768,-32768,   865,   865,-32768,-32768,   865,   865,-32768,
   524,-32768,   159,   607,   234,   234,   338,   408,-32768,-32768,
-32768,-32768,-32768,   533,    61,-32768,   282,   561,-32768,-32768,
   604,-32768,-32768,-32768,   184,   325,   331,   394,-32768,-32768,
-32768,-32768,   539,-32768,   180,-32768,   180,   180,-32768,-32768,
   565,   567,   865,   254,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   498,   184,-32768,-32768,-32768,-32768,   115,
   603,   564,-32768,-32768,   947,   429,   617,-32768,   696,   618,
-32768,   622,-32768,   902,   947,-32768,   865,-32768,-32768,-32768,
-32768,   408,   623,   408,-32768,-32768,   394,-32768,   579,-32768,
-32768,-32768,-32768,   180,   180,   604,   604,-32768,   865,-32768,
-32768,-32768,   175,   175,   628,   184,   175,-32768,-32768,   629,
   254,-32768,   305,   379,   865,-32768,-32768,   184,-32768,-32768,
-32768,-32768,-32768,   554,    39,   696,   653,-32768,   396,-32768,
-32768,-32768,-32768,   947,-32768,   636,-32768,   638,-32768,-32768,
   394,   408,   408,   408,-32768,-32768,-32768,-32768,   573,   645,
   646,-32768,   184,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
    72,   576,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   491,   583,   584,   588,-32768,-32768,-32768,
-32768,-32768,-32768,   653,-32768,-32768,   865,   408,-32768,-32768,
-32768,-32768,   947,-32768,   180,   180,   180,-32768,-32768,-32768,
-32768,-32768,-32768,   695,   701,-32768
};

static const short yypgoto[] = {-32768,
   649,   -22,-32768,   634,-32768,    64,   291,  -360,   587,-32768,
   523,    70,   -68,  -258,   651,  -225,-32768,  -480,    98,  -303,
-32768,-32768,-32768,-32768,-32768,-32768,    90,  -158,   321,-32768,
-32768,-32768,-32768,-32768,  -221,   410,  -446,  -383,  -361,-32768,
  -240,   292,    23,   161,  -347,-32768,  -183,-32768,   641,    -8,
-32768,  -355,   124,-32768,  -173,-32768,-32768,-32768,-32768,    42,
-32768,    14,    -1,-32768,-32768,   506,-32768,   -51,   501,   516,
   353,   -21,   -50,-32768,   155,-32768,   110,   -15,-32768,   -79,
-32768,  -540,  -219,  -510,    76,-32768,-32768,  -100,-32768,   185,
-32768,    38,   686,  -415,-32768,-32768,-32768,  -220,   -19,    -2,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,  -264,-32768,    21,
  -525,   -98,  -346,  -424,   489,  -400,   187,-32768,  -236,   270,
-32768,  -317,   278,-32768,  -435,-32768,  -546,-32768,  -332,-32768,
-32768,-32768,-32768,-32768,-32768,   176,   499,  -451
};


#define	YYLAST		1726


static const short yytable[] = {    81,
    82,    84,    85,    86,   192,   318,   330,    87,    88,    90,
    91,    92,    93,    94,    95,    96,   396,   397,    97,   414,
   103,    98,   105,   469,    99,   100,   179,   107,   108,   109,
   110,   111,   477,   519,   119,   114,   115,   116,   626,   106,
   387,   286,   178,   512,     6,   144,   145,     9,    10,   291,
   152,   666,   578,   493,   379,   133,   601,   732,   244,   575,
   411,   412,     6,    57,   526,     9,    10,   416,   489,    12,
   421,   422,   245,   678,   576,    12,     6,   185,   186,   670,
    12,   671,   672,    12,   198,   199,    12,   234,   538,   117,
   405,   432,   436,    42,   209,   433,   235,    12,   154,   376,
   561,   434,   256,   679,   210,   211,   212,   213,   214,    70,
   710,   398,   215,   133,   216,   217,   218,   437,   219,    57,
   438,   129,   220,   221,   222,   223,   224,   225,   233,   577,
   228,   229,   230,   231,   232,   519,   155,   113,   715,   138,
   678,   139,   140,   157,   733,   478,   479,   163,   166,   167,
   625,   270,   652,   173,   174,   658,   476,   167,   493,   575,
   575,   185,   628,   586,   743,    70,     6,   581,   582,    53,
   679,   449,   392,   489,   576,   576,   101,   273,   692,   693,
   634,   176,   177,   133,   525,   180,   181,   182,   609,   288,
   285,   162,   694,   521,   188,   429,   432,   162,    12,   139,
   140,   458,   358,   728,   316,   316,   434,   323,   328,   467,
   328,   430,   194,   623,    75,    76,   195,   469,   196,   336,
   560,   338,   339,   340,   341,   342,     6,   354,   205,   348,
   716,   717,   206,    12,   346,   606,     6,   357,   129,   768,
   769,   770,   609,    12,   226,   610,   365,   366,   367,   694,
   369,   370,   371,   378,   383,   105,   375,   622,   393,   393,
   393,   746,   747,   627,   227,   136,   137,   138,   236,   139,
   140,   399,   106,   190,   191,   250,   252,   237,     2,   192,
     3,     4,     5,   238,   189,   246,   539,   540,   442,   442,
   316,   319,   620,   249,   329,   676,   190,   191,   248,   610,
   519,   323,   239,    18,   240,   251,   764,   347,    42,   415,
   276,   163,   352,   253,   355,   146,   290,   167,   241,    12,
   325,   718,   147,   363,   254,   677,   504,   447,   255,   453,
   454,   259,   148,   410,   452,   349,   146,   262,     6,   380,
   149,   653,   260,   147,   150,   459,   381,   630,   362,   632,
   364,   400,   263,   148,   420,   378,   264,   105,   353,   374,
   265,   149,   742,  -236,   486,   150,   503,   504,   748,   481,
   749,   266,   160,  -286,   106,   190,   191,  -286,   160,  -286,
   317,   317,   175,   324,   267,   509,     6,   511,   442,     9,
    10,   517,   599,   442,   102,   602,   554,   328,   200,-32768,
   128,     6,   380,   268,   207,   208,   272,   523,    12,   381,
    90,   274,   527,   146,   276,   706,   531,   708,   533,   343,
   147,   771,   772,   773,   287,   435,   616,   617,   618,   660,
   148,   588,   555,   661,   350,   662,   356,   451,   149,   171,
   361,   588,   150,    12,   686,   457,    90,   171,   562,   372,
   563,   528,   529,   720,   721,   566,   567,   727,   583,   390,
   587,   470,   439,   444,   456,   401,   317,   385,   595,   486,
   587,   596,   597,   516,   650,   651,   403,   324,   393,   603,
   128,   612,   614,   522,  -146,   442,  -145,  -144,   656,   406,
   200,   408,   316,   443,   443,   323,  -141,   619,   200,   621,
   292,   293,   294,   295,   296,   297,   298,   299,   300,  -142,
   719,   290,  -143,   524,   413,   417,   312,   312,   633,   418,
   530,   431,   448,   171,-32768,   123,   124,   125,   126,     6,
   541,   127,   128,   636,   450,   455,    12,   542,   465,   556,
   588,   466,   532,   471,   557,   360,   472,   474,   480,   559,
   123,   124,   125,   126,   646,   368,   127,   128,     6,   543,
   544,     9,    10,   583,   496,    12,   656,   497,   498,   587,
   499,   324,   594,   659,   501,   502,   665,   513,   315,   315,
   402,   322,   515,   667,   545,   546,   668,   547,   548,   537,
   424,   425,   426,   443,   427,   631,-32768,-32768,   443,   682,
   127,   128,   312,   684,   124,   125,   126,   558,   564,   127,
   128,     6,   380,   565,   209,   588,   588,   699,    12,   381,
   579,   483,   688,   580,   615,   624,   629,   102,   382,   638,
   639,   445,   446,   705,   641,   665,   549,   647,   654,   637,
   709,   649,   657,   669,   587,   587,   673,   640,   674,   685,
   200,   687,   701,   642,   723,   434,   702,   707,   200,   133,
   711,   328,   722,   725,   315,   473,   730,   731,   317,   735,
   740,   324,   741,   324,   699,   322,     6,   380,   750,   751,
   752,   756,   483,    12,   381,   737,   738,   739,   759,   665,
   443,   441,   441,   760,   775,   202,   203,   204,   514,   761,
   776,   723,   675,     6,   118,   271,     9,    10,  -287,   755,
    12,   156,  -287,   703,  -287,   482,     6,   380,    53,   757,
   151,   683,   510,    12,   381,   643,   534,   409,   172,   102,
   460,   461,   462,   689,   463,   200,   200,   734,   485,   134,
   135,   136,   137,   138,   754,   139,   140,   483,   729,   133,
   598,   337,   762,   351,   359,   482,   726,   648,   690,   153,
   753,   691,   190,   191,   758,   133,   604,   391,   600,   322,
   520,   655,   664,  -287,   384,     0,     0,  -287,     0,  -287,
     0,     0,     0,   484,     0,     0,     0,   483,     0,     0,
     0,   441,     0,   700,     0,     0,   441,     0,     0,   553,
     0,     0,   692,   693,   312,   302,   303,   304,   305,   306,
   307,   308,     0,  -287,     0,     0,     0,  -287,     0,  -287,
   331,     0,     0,   484,     0,     0,   571,   635,     0,   404,
   135,   136,   137,   138,   485,   139,   140,     0,   324,     0,
     0,     0,     0,   485,   485,   423,   135,   136,   137,   138,
   700,   139,   140,     0,     0,   611,     0,     0,   644,   645,
     0,     0,     0,     0,     0,     0,   315,     0,     0,   322,
     0,   322,     6,     7,     8,     9,    10,     0,    11,    12,
    13,     0,     0,     0,     0,     0,     0,    14,   441,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,   571,    34,    35,    36,    37,
    38,    39,    40,   568,     0,     0,     0,   704,    41,     0,
    43,     0,     0,   485,     0,     0,     0,     0,     0,   663,
   485,     0,     0,    44,     0,     0,     0,   331,     0,   200,
     0,     0,    45,     0,     0,    46,    47,     0,     0,    48,
     0,    49,    50,    51,     0,    52,     0,     0,     0,    53,
     0,     0,     0,   120,   121,   122,   123,   124,   125,   126,
   302,   698,   127,   128,     0,   120,   121,   122,   123,   124,
   125,   126,     0,     0,   127,   128,     0,     0,     0,   485,
     0,     0,     0,     0,   569,   331,   331,     0,   485,   485,
     0,     0,     0,     0,     0,     0,   569,     0,   120,   121,
   122,   123,   124,   125,   126,     0,   322,   127,   128,     0,
     0,   121,   122,   123,   124,   125,   126,   763,   698,   127,
   128,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   485,     0,     0,     0,     0,     1,     0,
     2,     0,     3,     4,     5,     6,     7,     8,     9,    10,
     0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
    14,     0,    15,    16,    17,    18,  -242,    19,     0,     0,
     0,     0,     0,  -242,    20,    21,    22,     0,     0,   302,
    23,    24,    25,  -242,     0,    26,    27,   331,    28,   331,
    29,  -242,    30,     0,    31,  -242,    32,    33,     0,    34,
    35,    36,    37,    38,    39,    40,     0,   -11,     0,     0,
     0,    41,    42,    43,     0,     0,     0,     0,     0,     0,
     0,   331,     0,     0,     0,     0,    44,     0,     0,     0,
     0,     0,     0,     0,     0,    45,     0,     0,    46,    47,
     0,     0,    48,     0,    49,    50,    51,     0,    52,     0,
     0,     2,    53,     3,     4,     5,     6,     7,     8,     9,
    10,     0,    11,    12,    13,     0,     0,     0,     0,     0,
     0,    14,     0,    15,    16,    17,    18,     0,    19,     0,
     0,     0,     0,     0,     0,    20,    21,    22,     0,     0,
     0,    23,    24,    25,     0,     0,    26,    27,     0,    28,
     0,    29,     0,    30,     0,    31,     0,    32,    33,     0,
    34,    35,    36,    37,    38,    39,    40,     0,   -11,     0,
     0,     0,    41,    42,    43,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    44,     0,     0,
     0,     0,     0,     0,     0,     0,    45,     0,     0,    46,
    47,     0,     0,    48,     0,    49,    50,    51,     0,    52,
     0,     0,     0,    53,     6,     7,     8,     9,    10,     0,
    11,    12,    13,     0,     0,     0,     0,     0,     0,    14,
     0,    15,    16,    17,     0,     0,    19,     0,     0,     0,
     0,     0,     0,    20,    21,    22,     0,     0,     0,    23,
    24,    25,     0,     0,    26,    27,     0,    28,     0,    29,
     0,    30,     0,    31,     0,    32,    33,   261,    34,    35,
    36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
    41,     0,    43,     0,     0,     0,     0,     0,   120,   121,
   122,   123,   124,   125,   126,    44,     0,   127,   128,     0,
     0,     0,     0,     0,    45,     0,     0,    46,    47,     0,
     0,    48,     0,    49,    50,    51,     0,    52,     0,     0,
     0,    53,     6,     7,     8,     9,    10,     0,    11,    12,
    13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,     0,    34,    35,    36,    37,
    38,    39,    40,     0,     0,     0,     0,     0,    41,     0,
    43,     0,     0,     0,     0,     0,   120,   121,   122,   123,
   124,   125,   126,    44,     0,   127,   128,     0,     0,     0,
     0,     0,    45,     0,     0,    46,    47,     0,     0,    48,
     0,    49,    50,    51,     0,    52,     0,     0,     0,    53,
     6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
     0,     0,     0,     0,     0,    14,     0,    15,    16,    17,
     0,     0,    19,     0,     0,     0,     0,     0,     0,    20,
   309,    22,     0,     0,     0,    23,    24,    25,     0,     0,
    26,    27,     0,    28,     0,    29,     0,    30,     0,    31,
     0,    32,    33,     0,    34,   310,    36,    37,   311,    39,
    40,     0,     0,     0,     0,     0,    41,     0,    43,     0,
     0,     0,     0,     0,   120,   121,   122,   123,   124,   125,
   126,    44,     0,   127,   128,     0,     0,     0,     0,     0,
    45,     0,     0,    46,    47,     0,     0,    48,     0,    49,
    50,    51,     0,    52,     0,     0,     0,    53,     6,     7,
     8,     9,    10,     0,    11,    12,    13,     0,     0,     0,
     0,     0,     0,    14,     0,    15,    16,    17,     0,     0,
    19,     0,     0,     0,     0,     0,     0,    20,   309,    22,
     0,     0,     0,    23,    24,    25,     0,     0,    26,    27,
     0,    28,     0,    29,     0,    30,     0,    31,     0,    32,
    33,     0,    34,   310,    36,    37,   311,    39,    40,     0,
     0,     0,     0,     0,    41,     0,    43,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    44,
     0,     0,     0,     0,     0,     0,     0,     0,    45,     0,
     0,    46,    47,     0,     0,    48,     0,    49,    50,    51,
     0,    52,     0,     0,     0,    53
};

static const short yycheck[] = {    15,
    16,    17,    18,    19,   103,   227,   232,    23,    24,    25,
    26,    27,    28,    29,    30,    31,   281,   282,    34,   323,
    42,    37,    42,   371,    40,    41,    95,    43,    44,    45,
    46,    47,   379,   417,    57,    51,    52,    53,   519,    42,
   277,   200,    94,   404,     8,    61,    62,    11,    12,   208,
    73,   598,   477,   386,   275,    13,   492,    19,    47,   475,
   319,   320,     8,     0,   426,    11,    12,   326,   386,    15,
   329,   330,    61,   614,   475,    15,     8,    99,   100,   605,
    15,   607,   608,    15,   106,   107,    15,    52,   444,    70,
   312,    82,    38,    75,   116,    86,    61,    15,     4,   273,
   462,    92,   171,   614,   120,   121,   122,   123,   124,     0,
   657,   285,   128,    13,   130,   131,   132,    63,   134,    56,
    66,    58,   138,   139,   140,   141,   142,   143,   151,   476,
   146,   147,   148,   149,   150,   519,     4,    48,   664,    97,
   681,    99,   100,    80,   106,   382,   383,    84,    85,    86,
   114,    69,   577,    90,    91,   591,   377,    94,   491,   575,
   576,   183,   523,   481,   711,    56,     8,   107,   108,   115,
   681,   345,   107,   491,   575,   576,   108,   186,   107,   108,
   536,    92,    93,    13,   425,    96,    97,    98,    14,   205,
   199,    82,   639,   419,   114,    18,    82,    88,    15,    99,
   100,   360,   254,   684,   226,   227,    92,   229,   230,   368,
   232,    34,   105,   517,   103,   104,   109,   565,   111,   235,
   461,   237,   238,   239,   240,   241,     8,   250,    47,   245,
   666,   667,   106,    15,   243,   500,     8,   253,   175,   765,
   766,   767,    14,    15,   105,    71,   262,   263,   264,   696,
   266,   267,   268,   275,   276,   275,   272,   516,   280,   281,
   282,   713,   714,   522,   105,    95,    96,    97,    18,    99,
   100,   287,   275,    94,    95,   166,   167,    35,     3,   378,
     5,     6,     7,    21,    82,    31,   445,   446,   339,   340,
   312,   228,   514,    47,   231,    42,    94,    95,    18,    71,
   684,   323,    40,    28,    42,    47,   758,   244,    75,   325,
    77,   248,   249,    35,   251,    29,   207,   254,    56,    15,
    16,   668,    36,   260,    18,    72,    73,   343,   111,   352,
   353,    61,    46,    47,   350,   246,    29,    35,     8,     9,
    54,   578,    47,    36,    58,   361,    16,   531,   259,   533,
   261,   288,    35,    46,    47,   377,    35,   377,   249,   270,
   111,    54,   709,    82,   386,    58,    72,    73,   715,   385,
   717,    35,    82,   105,   377,    94,    95,   109,    88,   111,
   226,   227,    92,   229,    35,   401,     8,   403,   439,    11,
    12,   413,   491,   444,    42,   494,   447,   419,   108,    91,
    92,     8,     9,    24,   114,   115,    47,   423,    15,    16,
   426,    82,   428,    29,    77,   652,   432,   654,   434,    50,
    36,   768,   769,   770,    90,   336,   506,   507,   508,   105,
    46,   482,   448,   109,    35,   111,    61,   348,    54,    87,
    35,   492,    58,    15,    16,   356,   462,    95,   464,    41,
   466,   429,   430,   673,   674,   471,   472,   683,   480,    47,
   482,   372,   339,   340,   355,   110,   312,    60,   484,   491,
   492,   487,   488,   410,   575,   576,   112,   323,   500,   495,
    92,   503,   504,   420,   114,   536,   114,   114,   587,    19,
   200,   114,   514,   339,   340,   517,   114,   513,   208,   515,
   210,   211,   212,   213,   214,   215,   216,   217,   218,   114,
   669,   402,   114,   424,   114,   105,   226,   227,   534,    22,
   431,    61,    35,   171,    84,    85,    86,    87,    88,     8,
     9,    91,    92,   549,    61,    61,    15,    16,    35,   450,
   591,    35,   433,    35,   455,   255,    35,    78,   105,   460,
    85,    86,    87,    88,   570,   265,    91,    92,     8,    38,
    39,    11,    12,   585,    35,    15,   665,    18,   106,   591,
    80,   417,   483,   595,   110,   112,   598,   106,   226,   227,
   290,   229,   106,   599,    63,    64,   602,    66,    67,   114,
    31,    32,    33,   439,    35,   532,    87,    88,   444,   615,
    91,    92,   312,   625,    86,    87,    88,   112,   112,    91,
    92,     8,     9,    18,   636,   666,   667,   639,    15,    16,
    35,    79,   638,    35,    35,   106,    35,   275,   276,    82,
   105,   341,   342,   649,    35,   657,   115,   114,   106,   550,
   656,    35,    82,   105,   666,   667,    82,   558,    82,    47,
   360,    35,    35,   564,   676,    92,    35,    35,   368,    13,
    82,   683,    35,    35,   312,   375,   688,   114,   514,    17,
    35,   517,    35,   519,   696,   323,     8,     9,   106,    35,
    35,   106,    79,    15,    16,   701,   702,   703,   106,   711,
   536,   339,   340,   110,     0,   109,   110,   111,   408,   112,
     0,   723,   613,     8,    56,   183,    11,    12,   105,   731,
    15,    78,   109,   644,   111,    47,     8,     9,   115,   735,
    70,   624,   402,    15,    16,   565,   435,   318,    88,   377,
    31,    32,    33,    38,    35,   445,   446,   696,   386,    93,
    94,    95,    96,    97,   731,    99,   100,    79,   685,    13,
    82,   236,   754,   248,   254,    47,   681,   573,    63,    74,
   723,    66,    94,    95,   744,    13,   497,   279,   491,   417,
   418,   585,   597,   105,   276,    -1,    -1,   109,    -1,   111,
    -1,    -1,    -1,   115,    -1,    -1,    -1,    79,    -1,    -1,
    -1,   439,    -1,   639,    -1,    -1,   444,    -1,    -1,   447,
    -1,    -1,   107,   108,   514,   219,   220,   221,   222,   223,
   224,   225,    -1,   105,    -1,    -1,    -1,   109,    -1,   111,
   234,    -1,    -1,   115,    -1,    -1,   474,   537,    -1,    93,
    94,    95,    96,    97,   482,    99,   100,    -1,   684,    -1,
    -1,    -1,    -1,   491,   492,    93,    94,    95,    96,    97,
   696,    99,   100,    -1,    -1,   503,    -1,    -1,   568,   569,
    -1,    -1,    -1,    -1,    -1,    -1,   514,    -1,    -1,   517,
    -1,   519,     8,     9,    10,    11,    12,    -1,    14,    15,
    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,   536,    25,
    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,
    -1,    57,    -1,    59,    60,   573,    62,    63,    64,    65,
    66,    67,    68,    20,    -1,    -1,    -1,   647,    74,    -1,
    76,    -1,    -1,   591,    -1,    -1,    -1,    -1,    -1,   597,
   598,    -1,    -1,    89,    -1,    -1,    -1,   371,    -1,   669,
    -1,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,   105,
    -1,   107,   108,   109,    -1,   111,    -1,    -1,    -1,   115,
    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
   404,   639,    91,    92,    -1,    82,    83,    84,    85,    86,
    87,    88,    -1,    -1,    91,    92,    -1,    -1,    -1,   657,
    -1,    -1,    -1,    -1,   113,   429,   430,    -1,   666,   667,
    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    82,    83,
    84,    85,    86,    87,    88,    -1,   684,    91,    92,    -1,
    -1,    83,    84,    85,    86,    87,    88,   757,   696,    91,
    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,   711,    -1,    -1,    -1,    -1,     1,    -1,
     3,    -1,     5,     6,     7,     8,     9,    10,    11,    12,
    -1,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
    23,    -1,    25,    26,    27,    28,    29,    30,    -1,    -1,
    -1,    -1,    -1,    36,    37,    38,    39,    -1,    -1,   523,
    43,    44,    45,    46,    -1,    48,    49,   531,    51,   533,
    53,    54,    55,    -1,    57,    58,    59,    60,    -1,    62,
    63,    64,    65,    66,    67,    68,    -1,    70,    -1,    -1,
    -1,    74,    75,    76,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,   565,    -1,    -1,    -1,    -1,    89,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,   101,   102,
    -1,    -1,   105,    -1,   107,   108,   109,    -1,   111,    -1,
    -1,     3,   115,     5,     6,     7,     8,     9,    10,    11,
    12,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,
    -1,    23,    -1,    25,    26,    27,    28,    -1,    30,    -1,
    -1,    -1,    -1,    -1,    -1,    37,    38,    39,    -1,    -1,
    -1,    43,    44,    45,    -1,    -1,    48,    49,    -1,    51,
    -1,    53,    -1,    55,    -1,    57,    -1,    59,    60,    -1,
    62,    63,    64,    65,    66,    67,    68,    -1,    70,    -1,
    -1,    -1,    74,    75,    76,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,   101,
   102,    -1,    -1,   105,    -1,   107,   108,   109,    -1,   111,
    -1,    -1,    -1,   115,     8,     9,    10,    11,    12,    -1,
    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,
    -1,    25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,
    -1,    -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,
    44,    45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,
    -1,    55,    -1,    57,    -1,    59,    60,    61,    62,    63,
    64,    65,    66,    67,    68,    -1,    -1,    -1,    -1,    -1,
    74,    -1,    76,    -1,    -1,    -1,    -1,    -1,    82,    83,
    84,    85,    86,    87,    88,    89,    -1,    91,    92,    -1,
    -1,    -1,    -1,    -1,    98,    -1,    -1,   101,   102,    -1,
    -1,   105,    -1,   107,   108,   109,    -1,   111,    -1,    -1,
    -1,   115,     8,     9,    10,    11,    12,    -1,    14,    15,
    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,
    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,
    -1,    57,    -1,    59,    60,    -1,    62,    63,    64,    65,
    66,    67,    68,    -1,    -1,    -1,    -1,    -1,    74,    -1,
    76,    -1,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
    86,    87,    88,    89,    -1,    91,    92,    -1,    -1,    -1,
    -1,    -1,    98,    -1,    -1,   101,   102,    -1,    -1,   105,
    -1,   107,   108,   109,    -1,   111,    -1,    -1,    -1,   115,
     8,     9,    10,    11,    12,    -1,    14,    15,    16,    -1,
    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,    26,    27,
    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,
    38,    39,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,
    48,    49,    -1,    51,    -1,    53,    -1,    55,    -1,    57,
    -1,    59,    60,    -1,    62,    63,    64,    65,    66,    67,
    68,    -1,    -1,    -1,    -1,    -1,    74,    -1,    76,    -1,
    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,
    88,    89,    -1,    91,    92,    -1,    -1,    -1,    -1,    -1,
    98,    -1,    -1,   101,   102,    -1,    -1,   105,    -1,   107,
   108,   109,    -1,   111,    -1,    -1,    -1,   115,     8,     9,
    10,    11,    12,    -1,    14,    15,    16,    -1,    -1,    -1,
    -1,    -1,    -1,    23,    -1,    25,    26,    27,    -1,    -1,
    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,
    -1,    -1,    -1,    43,    44,    45,    -1,    -1,    48,    49,
    -1,    51,    -1,    53,    -1,    55,    -1,    57,    -1,    59,
    60,    -1,    62,    63,    64,    65,    66,    67,    68,    -1,
    -1,    -1,    -1,    -1,    74,    -1,    76,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
    -1,   101,   102,    -1,    -1,   105,    -1,   107,   108,   109,
    -1,   111,    -1,    -1,    -1,   115
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
{
		    yyval.t = newCTerm(PA_fAnd,yyvsp[-1].t,yyvsp[0].t);
		  ;
    break;}
case 85:
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
case 86:
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
case 87:
{ yyval.t = 0; ;
    break;}
case 88:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 89:
{ yyval.t = AtomNil; ;
    break;}
case 90:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 91:
{ yyval.t = AtomNil; ;
    break;}
case 92:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 93:
{ yyval.t = oz_cons(newCTerm(PA_fRequire,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 94:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 95:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-1].t,
					   newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 96:
{ yyval.t = oz_cons(newCTerm(PA_fImport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 97:
{ yyval.t = oz_cons(newCTerm(PA_fExport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 98:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 99:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-1].t,
					   newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 100:
{ yyval.t = AtomNil; ;
    break;}
case 101:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-2].t,AtomNil,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 102:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 103:
{ yyval.t = newCTerm(PA_fVar,OZ_atom(xytext),yyvsp[0].t); ;
    break;}
case 104:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 105:
{ yyval.t = oz_mklist(oz_pair2(yyvsp[0].t,yyvsp[-2].t)); ;
    break;}
case 106:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 107:
{ yyval.t = oz_cons(oz_pair2(yyvsp[-1].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 108:
{ yyval.t = PA_fNoImportAt; ;
    break;}
case 109:
{ yyval.t = newCTerm(PA_fImportAt,yyvsp[0].t); ;
    break;}
case 110:
{ yyval.t = AtomNil; ;
    break;}
case 111:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 112:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,
					   newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t)),yyvsp[0].t); ;
    break;}
case 113:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 114:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 115:
{ yyval.t = OZ_atom(xytext); ;
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
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 120:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 121:
{ yyval.t = AtomNil; ;
    break;}
case 122:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 123:
{ yyval.t = newCTerm(PA_fAtom,AtomNil,yyvsp[0].t); ;
    break;}
case 124:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomCons,yyvsp[-2].t),
				  oz_mklist(yyvsp[-1].t,yyvsp[0].t)); ;
    break;}
case 125:
{ yyval.t = PA_fNoCatch; ;
    break;}
case 126:
{ yyval.t = newCTerm(PA_fCatch,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 127:
{ yyval.t = PA_fNoFinally; ;
    break;}
case 128:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 129:
{
		    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
				  newCTerm(PA_fAtom,yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
		  ;
    break;}
case 130:
{
		    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
				  makeVar(yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
		  ;
    break;}
case 131:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 132:
{ yyval.t = NameUnit; ;
    break;}
case 133:
{ yyval.t = NameTrue; ;
    break;}
case 134:
{ yyval.t = NameFalse; ;
    break;}
case 135:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 136:
{ yyval.t = AtomNil; ;
    break;}
case 137:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 138:
{ yyval.t = oz_cons(newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 139:
{ yyval.t = NameFalse; ;
    break;}
case 140:
{ yyval.t = NameTrue; ;
    break;}
case 141:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 142:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 143:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 144:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 145:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 146:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 147:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 148:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 149:
{ yyval.t = newCTerm(PA_fBoolCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 150:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 151:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 152:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 153:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 154:
{ checkDeprecation(yyvsp[-3].t);
		    yyval.t = newCTerm(PA_fBoolCase,yyvsp[-5].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-6].t,yyvsp[0].t));
		  ;
    break;}
case 155:
{ yyval.t = newCTerm(PA_fCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 156:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 157:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 158:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 159:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 160:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 161:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 162:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 163:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 164:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 165:
{ yyval.t = newCTerm(PA_fCaseClause,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 166:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 167:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-3].t,
				  newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 168:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-5].t,yyvsp[-2].t,yyvsp[0].t,yyvsp[-3].t); ;
    break;}
case 169:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 170:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 171:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 172:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
				  oz_cons(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 173:
{ yyval.t = newCTerm(PA_fClass,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 174:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 175:
{ yyval.t = newCTerm(PA_fDollar,yyvsp[0].t); ;
    break;}
case 176:
{ yyval.t = AtomNil; ;
    break;}
case 177:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 178:
{ yyval.t = newCTerm(PA_fFrom,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 179:
{ yyval.t = newCTerm(PA_fAttr,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 180:
{ yyval.t = newCTerm(PA_fFeat,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 181:
{ yyval.t = newCTerm(PA_fProp,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 182:
{ yyval.t = AtomNil; ;
    break;}
case 183:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 184:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 185:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 186:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 187:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 188:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 189:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 190:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 191:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 192:
{ yyval.t = AtomNil; ;
    break;}
case 193:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 194:
{ yyval.t = newCTerm(PA_fMeth,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-3].t); ;
    break;}
case 195:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 196:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 197:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 198:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 199:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 200:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 201:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 202:
{ yyval.t = newCTerm(PA_fRecord,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 203:
{ yyval.t = newCTerm(PA_fOpenRecord,yyvsp[-4].t,yyvsp[-2].t); ;
    break;}
case 204:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 205:
{ yyval.t = makeVar(xytext); ;
    break;}
case 206:
{ yyval.t = newCTerm(PA_fEscape,makeVar(xytext),yyvsp[-1].t); ;
    break;}
case 207:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 208:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 209:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 210:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 211:
{ yyval.t = AtomNil; ;
    break;}
case 212:
{ yyval.t = newCTerm(PA_fMethArg,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 213:
{ yyval.t = newCTerm(PA_fMethColonArg,yyvsp[-3].t,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 214:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 215:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 216:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 217:
{ yyval.t = newCTerm(PA_fDefault,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 218:
{ yyval.t = PA_fNoDefault; ;
    break;}
case 219:
{ yyval.t = newCTerm(PA_fCond,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 220:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 221:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 222:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 223:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 224:
{ yyval.t = newCTerm(PA_fClause,newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 225:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 226:
{ yyval.t = oz_mklist(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 227:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 228:
{ yyval.t = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,yyvsp[0].t),
				  yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 229:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-3].t,yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 230:
{ yyval.t = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 231:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 232:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 233:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 234:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 235:
{ yyval.t = makeVar(xytext); ;
    break;}
case 236:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 237:
{ yyval.t = newCTerm(PA_fEscape,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 238:
{ yyval.t = makeString(xytext,pos()); ;
    break;}
case 239:
{ yyval.t = makeInt(xytext,pos()); ;
    break;}
case 240:
{ yyval.t = makeInt(xytext[0],pos()); ;
    break;}
case 241:
{ yyval.t = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); ;
    break;}
case 242:
{ yyval.t = pos(); ;
    break;}
case 243:
{ yyval.t = pos(); ;
    break;}
case 244:
{ OZ_Term prefix =
		      scannerPrefix? scannerPrefix: PA_zy;
		    yyval.t = newCTerm(PA_fScanner,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,prefix,
				  makeLongPos(yyvsp[-6].t,yyvsp[0].t)); ;
    break;}
case 245:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 246:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 247:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 248:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 249:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 250:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 251:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 252:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 253:
{ yyval.t = newCTerm(PA_fLexicalRule,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 254:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 255:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 256:
{ yyval.t = newCTerm(PA_fMode,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 257:
{ yyval.t = AtomNil; ;
    break;}
case 258:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 259:
{ yyval.t = newCTerm(PA_fInheritedModes,yyvsp[0].t); ;
    break;}
case 260:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 261:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 262:
{ OZ_Term expect = parserExpect? parserExpect: newSmallInt(0);
		    yyval.t = newCTerm(PA_fParser,yyvsp[-6].t,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,expect,
				  makeLongPos(yyvsp[-7].t,yyvsp[0].t)); ;
    break;}
case 263:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 264:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 265:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 266:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 267:
{ yyval.t = newCTerm(PA_fToken,AtomNil); ;
    break;}
case 268:
{ yyval.t = newCTerm(PA_fToken,yyvsp[0].t); ;
    break;}
case 269:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 270:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 271:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 272:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 273:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 274:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 275:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 276:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 277:
{ *prodKey[depth]++ = '='; ;
    break;}
case 278:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 279:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 280:
{ *prodKey[depth]++ = '='; ;
    break;}
case 281:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 282:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,PA_none); ;
    break;}
case 283:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 284:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 285:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 288:
{ prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg(yyvsp[-1].t,0))); ;
    break;}
case 289:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 290:
{ depth--; ;
    break;}
case 291:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 292:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 293:
{ depth--; ;
    break;}
case 294:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 295:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 296:
{ depth--; ;
    break;}
case 297:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 298:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 299:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 300:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 301:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 302:
{ *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; ;
    break;}
case 305:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 306:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 307:
{ *prodKey[depth] = '\0';
		    yyval.t = oz_pair2(prodName[depth],OZ_string(prodKeyBuffer[depth]));
		    prodName[depth] = PA_none;
		    prodKey[depth] = prodKeyBuffer[depth];
		  ;
    break;}
case 308:
{ yyval.t = AtomNil; ;
    break;}
case 309:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 310:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 311:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 312:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 313:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 314:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 315:
{ yyval.t = AtomNil; ;
    break;}
case 316:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 317:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 318:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 319:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 320:
{ yyval.t = newCTerm(PA_fSynAlternative, yyvsp[0].t); ;
    break;}
case 321:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 322:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 323:
{ OZ_Term t = yyvsp[0].t;
		    while (terms[depth]) {
		      t = oz_cons(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    yyval.t = newCTerm(PA_fSynSequence, decls[depth], t, yyvsp[-1].t);
		    decls[depth] = AtomNil;
		  ;
    break;}
case 324:
{ yyval.t = newCTerm(PA_fSynSequence, AtomNil, yyvsp[0].t, yyvsp[-1].t); ;
    break;}
case 325:
{ yyval.t = AtomNil; ;
    break;}
case 326:
{ yyval.t = oz_mklist(newCTerm(PA_fSynAction,yyvsp[0].t)); ;
    break;}
case 327:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 328:
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
case 329:
{ yyval.t = oz_cons(newCTerm(PA_fSynAssignment, terms[depth]->term, yyvsp[-1].t),
				  yyvsp[0].t);
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  ;
    break;}
case 330:
{ while (terms[depth]) {
		      decls[depth] = oz_cons(terms[depth]->term, decls[depth]);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    yyval.t = yyvsp[0].t;
		  ;
    break;}
case 331:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 332:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 333:
{ terms[depth] = new TermNode(yyvsp[0].t, terms[depth]); ;
    break;}
case 334:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 335:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 336:
{ yyval.t = newCTerm(PA_fSynAssignment,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 337:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 338:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 339:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_cons(newCTerm(PA_fSynApplication,yyvsp[-3].t,
						    AtomNil),
					   AtomNil),yyvsp[-1].t);
		  ;
    break;}
case 340:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 341:
{ yyval.t = newCTerm(PA_fSynAssignment,
				  newCTerm(PA_fEscape,yyvsp[-2].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 342:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 343:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 344:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_mklist(yyvsp[-2].t),yyvsp[-3].t);
		  ;
    break;}
case 345:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_mklist(yyvsp[-3].t),yyvsp[-1].t);
		  ;
    break;}
case 346:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 347:
{ depth--; ;
    break;}
case 348:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 349:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 350:
{ depth--; ;
    break;}
case 351:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 352:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 353:
{ depth--; ;
    break;}
case 354:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 355:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 356:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[-4].t,yyvsp[-1].t); ;
    break;}
case 357:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 358:
{ yyval.t = makeVar(xytext); ;
    break;}
case 359:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 360:
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
