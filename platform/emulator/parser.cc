
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
#define	T_orelse	337
#define	T_andthen	338
#define	T_COMPARE	339
#define	T_FDCOMPARE	340
#define	T_LMACRO	341
#define	T_RMACRO	342
#define	T_FDIN	343
#define	T_ADD	344
#define	T_FDMUL	345
#define	T_OTHERMUL	346
#define	T_DEREFF	347


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



#define	YYFINAL		773
#define	YYFLAG		-32768
#define	YYNTBASE	115

#define YYTRANSLATE(x) ((unsigned)(x) <= 347 ? yytranslate[x] : 254)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   114,     2,    92,   107,     2,     2,     2,   104,
   105,     2,   102,    96,   103,    98,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,   113,   112,     2,
    82,     2,     2,   100,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   108,     2,   109,    99,   106,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   110,    91,   111,    97,     2,     2,     2,     2,
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
    88,    89,    90,    93,    94,    95,   101
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
   324,   332,   335,   337,   346,   353,   354,   357,   358,   361,
   362,   364,   369,   376,   381,   386,   391,   398,   403,   404,
   408,   415,   418,   420,   424,   427,   432,   433,   436,   437,
   440,   445,   447,   449,   451,   453,   455,   457,   462,   464,
   465,   468,   470,   474,   475,   479,   480,   483,   491,   499,
   501,   503,   505,   507,   509,   510,   513,   518,   519,   521,
   523,   525,   527,   529,   531,   533,   535,   537,   544,   547,
   550,   554,   556,   564,   571,   574,   577,   581,   583,   585,
   589,   593,   595,   599,   603,   605,   610,   617,   622,   627,
   629,   634,   642,   644,   646,   647,   650,   655,   660,   665,
   670,   671,   674,   678,   680,   682,   684,   686,   688,   690,
   692,   693,   696,   702,   704,   709,   711,   713,   715,   717,
   719,   724,   730,   732,   734,   738,   740,   742,   744,   747,
   748,   751,   756,   758,   760,   762,   766,   767,   773,   776,
   777,   779,   783,   788,   794,   798,   802,   805,   810,   815,
   821,   823,   827,   829,   831,   833,   837,   839,   841,   843,
   845,   846,   847,   856,   858,   860,   862,   865,   868,   871,
   877,   883,   888,   890,   892,   897,   898,   901,   904,   906,
   908,   918,   920,   922,   925,   928,   929,   932,   934,   937,
   939,   943,   945,   948,   950,   953,   954,   964,   965,   966,
   977,   984,   988,   991,   994,   996,   997,  1000,  1001,  1002,
  1009,  1010,  1011,  1018,  1019,  1020,  1027,  1029,  1033,  1035,
  1037,  1039,  1040,  1042,  1044,  1046,  1047,  1048,  1051,  1053,
  1056,  1061,  1066,  1074,  1075,  1078,  1080,  1082,  1084,  1086,
  1088,  1092,  1095,  1099,  1100,  1103,  1106,  1112,  1117,  1120,
  1123,  1125,  1127,  1129,  1132,  1136,  1138,  1140,  1145,  1147,
  1153,  1155,  1157,  1163,  1168,  1169,  1170,  1180,  1181,  1182,
  1192,  1193,  1194,  1204,  1206,  1212,  1214,  1216,  1218
};

static const short yyrhs[] = {   116,
    70,     0,     1,     0,   121,   117,     0,   208,   117,     0,
   117,     0,   192,   130,   117,     0,   118,   116,     0,    28,
   193,   121,    47,   192,   117,     0,    28,   193,   121,    47,
   121,   117,     0,    28,   193,   121,   192,   117,     0,     0,
     3,   119,     0,     5,     0,     6,     0,     7,     0,     0,
   120,   119,     0,   102,     4,     0,   103,     4,     0,   122,
     0,   122,   121,     0,   122,    82,   193,   122,     0,   122,
    83,   193,   122,     0,   122,    84,   193,   122,     0,   122,
    85,   193,   122,     0,   122,   136,   193,   122,     0,   122,
   137,   193,   122,     0,   122,   138,   193,   122,     0,   122,
    91,   193,   122,     0,   124,     0,   124,    92,   193,   123,
     0,   124,     0,   124,    92,   123,     0,   124,   139,   193,
   124,     0,   124,   140,   193,   124,     0,   124,   141,   193,
   124,     0,   124,    96,   193,   124,     0,    97,   193,   124,
     0,   124,    98,   193,   124,     0,   124,    13,     0,   124,
    99,   193,   124,     0,   100,   193,   124,     0,   101,   193,
   124,     0,   104,   142,   105,     0,   186,     0,   188,     0,
   106,     0,    66,     0,    63,     0,    38,     0,    59,     0,
   107,     0,   189,     0,   190,     0,   191,     0,   147,     0,
   108,   193,   122,   144,   109,   193,     0,   110,   193,   122,
   143,   111,   193,     0,    55,   193,   128,   110,   122,   143,
   111,   142,    35,   193,     0,    43,   193,   128,   110,   122,
   143,   111,   142,    35,   193,     0,    44,   193,   164,   129,
    35,   193,     0,   163,     0,    48,   193,   121,    47,   121,
    35,     0,    45,   154,     0,    23,   156,     0,    49,   193,
   142,    35,   193,     0,    49,   193,   122,    61,   142,    35,
   193,     0,    62,   193,   142,    35,   193,     0,    65,   193,
   142,   145,   146,    35,   193,     0,    57,   193,   142,    35,
   193,     0,    60,     0,    37,     0,    51,   193,   142,    35,
   193,     0,    27,   179,     0,    53,   193,   183,    35,   193,
     0,    30,   193,   183,    35,   193,     0,    25,   193,   185,
    35,   193,     0,   194,     0,   202,     0,    88,   193,   143,
    89,   193,     0,    68,   193,   125,    69,   142,    35,   193,
     0,   125,   126,     0,   126,     0,   187,    47,   193,   122,
    20,   122,   127,   193,     0,   187,    47,   193,   122,   127,
   193,     0,     0,   112,   122,     0,     0,   186,   128,     0,
     0,   130,     0,    58,   193,   131,   129,     0,    54,   193,
   121,    47,   121,   129,     0,    54,   193,   121,   129,     0,
    46,   193,   131,   129,     0,    36,   193,   135,   129,     0,
    29,   193,   121,    47,   121,   129,     0,    29,   193,   121,
   129,     0,     0,   187,   134,   131,     0,   132,   104,   133,
   105,   134,   131,     0,    16,   193,     0,   153,     0,   153,
   113,   187,     0,   153,   133,     0,   153,   113,   187,   133,
     0,     0,    22,   186,     0,     0,   187,   135,     0,   153,
   113,   187,   135,     0,    86,     0,    87,     0,    90,     0,
    93,     0,    94,     0,    95,     0,   121,    47,   193,   121,
     0,   121,     0,     0,   122,   143,     0,   192,     0,   192,
   122,   144,     0,     0,    24,   193,   159,     0,     0,    41,
   142,     0,   148,   193,   104,   150,   151,   105,   193,     0,
   149,   193,   104,   150,   151,   105,   193,     0,     9,     0,
    67,     0,    64,     0,    39,     0,    16,     0,     0,   122,
   150,     0,   152,   113,   122,   150,     0,     0,    19,     0,
   186,     0,   187,     0,   190,     0,    66,     0,    63,     0,
    38,     0,   186,     0,   190,     0,   193,   121,    61,   142,
   155,   193,     0,    33,   154,     0,    32,   156,     0,    31,
   142,    35,     0,    35,     0,   193,   121,    61,   193,   142,
   157,   193,     0,   193,   121,    52,   158,   157,   193,     0,
    33,   154,     0,    32,   156,     0,    31,   142,    35,     0,
    35,     0,   160,     0,   160,    18,   158,     0,   160,    34,
   158,     0,   160,     0,   160,    18,   159,     0,   161,    61,
   142,     0,   162,     0,   162,    85,   192,   121,     0,   162,
    85,   192,   121,    47,   121,     0,   162,    82,   193,   162,
     0,   162,    91,   193,   162,     0,   124,     0,   124,    92,
   193,   123,     0,    26,   193,   164,   165,   170,    35,   193,
     0,   122,     0,   192,     0,     0,   166,   165,     0,    42,
   193,   122,   143,     0,    21,   193,   168,   167,     0,    40,
   193,   168,   167,     0,    56,   193,   122,   143,     0,     0,
   168,   167,     0,   169,   113,   122,     0,   169,     0,   186,
     0,   188,     0,   190,     0,    66,     0,    63,     0,    38,
     0,     0,   171,   170,     0,    50,   193,   172,   142,    35,
     0,   173,     0,   173,    82,   193,   187,     0,   186,     0,
   188,     0,    66,     0,    63,     0,    38,     0,   174,   104,
   175,   105,     0,   174,   104,   175,    19,   105,     0,     9,
     0,    16,     0,   114,   193,    16,     0,    67,     0,    64,
     0,    39,     0,   176,   175,     0,     0,   177,   178,     0,
   152,   113,   177,   178,     0,   187,     0,   107,     0,   106,
     0,    17,   193,   122,     0,     0,   193,   181,   180,    35,
   193,     0,    31,   142,     0,     0,   182,     0,   182,    18,
   181,     0,   121,    61,   193,   142,     0,   121,    47,   121,
    61,   142,     0,   184,    18,   184,     0,   184,    18,   183,
     0,   121,   192,     0,   121,    47,   121,   192,     0,   121,
   192,    61,   142,     0,   121,    47,   121,    61,   142,     0,
   142,     0,   142,    18,   185,     0,     8,     0,    15,     0,
   187,     0,   114,   193,   187,     0,    14,     0,    11,     0,
    12,     0,    10,     0,     0,     0,    76,   193,   187,   165,
   170,   195,    35,   193,     0,   196,     0,   197,     0,   199,
     0,   196,   195,     0,   197,   195,     0,   199,   195,     0,
    72,   186,    82,   198,    35,     0,    72,   187,    82,   198,
    35,     0,    72,   198,   142,    35,     0,    71,     0,    14,
     0,    73,   187,   200,    35,     0,     0,   201,   200,     0,
    42,   207,     0,   197,     0,   199,     0,    74,   193,   187,
   165,   170,   204,   203,    35,   193,     0,   231,     0,   209,
     0,   231,   203,     0,   209,   203,     0,     0,    78,   205,
     0,   206,     0,   206,   205,     0,   186,     0,   186,   113,
   122,     0,   187,     0,   187,   207,     0,   209,     0,   209,
   208,     0,     0,    75,   187,    82,   210,   213,   228,   229,
   234,    35,     0,     0,     0,    75,   107,   211,    82,   212,
   213,   228,   229,   234,    35,     0,    75,   213,   228,   229,
   234,    35,     0,   215,   187,   226,     0,   187,   227,     0,
   214,   216,     0,   215,     0,     0,   186,   113,     0,     0,
     0,   104,   217,   223,   105,   218,   226,     0,     0,     0,
   108,   219,   223,   109,   220,   226,     0,     0,     0,   110,
   221,   223,   111,   222,   226,     0,   224,     0,   224,   225,
   223,     0,   187,     0,   106,     0,    80,     0,     0,   227,
     0,    93,     0,    94,     0,     0,     0,   230,    47,     0,
   231,     0,   231,   230,     0,    77,   186,   234,    35,     0,
    77,   187,   234,    35,     0,    77,   252,   104,   232,   105,
   234,    35,     0,     0,   233,   232,     0,   187,     0,   107,
     0,   106,     0,   235,     0,   236,     0,   236,    18,   235,
     0,   192,   238,     0,    60,   193,   237,     0,     0,    79,
   142,     0,   239,   238,     0,   239,   227,   193,   240,   228,
     0,   239,    82,   242,   240,     0,    47,   240,     0,   243,
   240,     0,   237,     0,   187,     0,   237,     0,   241,   240,
     0,   188,    82,   242,     0,   242,     0,   187,     0,   187,
   227,   193,   228,     0,   244,     0,   114,   193,   187,    82,
   242,     0,   244,     0,   251,     0,   215,   193,   251,   226,
   228,     0,   251,   227,   193,   228,     0,     0,     0,   214,
   193,   104,   245,   253,   105,   246,   226,   228,     0,     0,
     0,   214,   193,   108,   247,   253,   109,   248,   226,   228,
     0,     0,     0,   214,   193,   110,   249,   253,   111,   250,
   226,   228,     0,   186,     0,   252,   193,   104,   143,   105,
     0,     9,     0,    16,     0,   234,     0,   234,   225,   253,
     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   676,   678,   682,   684,   687,   689,   694,   696,   699,   701,
   704,   708,   710,   712,   714,   718,   720,   724,   731,   740,
   742,   746,   748,   750,   752,   754,   757,   759,   761,   763,
   765,   771,   773,   777,   780,   783,   786,   788,   791,   794,
   797,   800,   802,   805,   807,   809,   811,   813,   815,   817,
   819,   821,   823,   825,   827,   829,   831,   835,   837,   840,
   843,   845,   847,   849,   851,   853,   855,   857,   859,   861,
   863,   865,   867,   869,   871,   873,   875,   877,   879,   881,
   883,   889,   893,   896,   910,   934,   935,   939,   941,   946,
   948,   953,   955,   957,   960,   962,   964,   966,   971,   973,
   975,   979,   983,   985,   987,   989,   993,   995,   999,  1001,
  1003,  1008,  1012,  1016,  1020,  1024,  1028,  1032,  1034,  1038,
  1040,  1044,  1046,  1052,  1054,  1058,  1060,  1064,  1069,  1076,
  1078,  1080,  1082,  1086,  1090,  1092,  1094,  1098,  1100,  1104,
  1106,  1108,  1110,  1112,  1114,  1118,  1120,  1124,  1128,  1130,
  1132,  1134,  1138,  1142,  1146,  1148,  1150,  1152,  1156,  1158,
  1160,  1164,  1166,  1170,  1174,  1176,  1179,  1183,  1185,  1187,
  1189,  1195,  1200,  1202,  1207,  1209,  1213,  1215,  1217,  1219,
  1223,  1225,  1229,  1231,  1235,  1237,  1239,  1241,  1243,  1245,
  1249,  1251,  1255,  1259,  1261,  1265,  1267,  1269,  1271,  1273,
  1275,  1277,  1281,  1283,  1285,  1287,  1289,  1291,  1295,  1297,
  1301,  1303,  1307,  1309,  1311,  1316,  1318,  1322,  1326,  1328,
  1332,  1334,  1338,  1340,  1344,  1346,  1350,  1354,  1356,  1359,
  1363,  1365,  1369,  1373,  1377,  1379,  1383,  1387,  1389,  1393,
  1397,  1401,  1411,  1419,  1421,  1423,  1425,  1427,  1429,  1433,
  1435,  1439,  1443,  1445,  1449,  1453,  1455,  1459,  1461,  1463,
  1469,  1477,  1479,  1481,  1483,  1487,  1489,  1493,  1495,  1499,
  1501,  1505,  1507,  1511,  1513,  1517,  1519,  1521,  1522,  1523,
  1525,  1529,  1531,  1533,  1537,  1538,  1541,  1545,  1546,  1546,
  1547,  1548,  1548,  1549,  1550,  1550,  1553,  1555,  1559,  1560,
  1563,  1567,  1568,  1571,  1572,  1575,  1583,  1585,  1589,  1591,
  1595,  1597,  1599,  1603,  1605,  1609,  1611,  1613,  1617,  1621,
  1623,  1627,  1636,  1640,  1642,  1646,  1648,  1658,  1663,  1670,
  1672,  1676,  1680,  1682,  1686,  1688,  1692,  1694,  1700,  1704,
  1707,  1712,  1714,  1718,  1722,  1723,  1724,  1726,  1727,  1728,
  1730,  1731,  1732,  1736,  1738,  1742,  1744,  1749,  1751
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
"T_REDUCE","T_SEP","T_ITER","'='","T_OOASSIGN","T_orelse","T_andthen","T_COMPARE",
"T_FDCOMPARE","T_LMACRO","T_RMACRO","T_FDIN","'|'","'#'","T_ADD","T_FDMUL","T_OTHERMUL",
"','","'~'","'.'","'^'","'@'","T_DEREFF","'+'","'-'","'('","')'","'_'","'$'",
"'['","']'","'{'","'}'","';'","':'","'!'","file","queries","queries1","directive",
"switchList","switch","sequence","phrase","hashes","phrase2","iterators","iterator",
"optIteratorStep","procFlags","optFunctorDescriptorList","functorDescriptorList",
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
   115,   115,   116,   116,   116,   116,   117,   117,   117,   117,
   117,   118,   118,   118,   118,   119,   119,   120,   120,   121,
   121,   122,   122,   122,   122,   122,   122,   122,   122,   122,
   122,   123,   123,   124,   124,   124,   124,   124,   124,   124,
   124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
   124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
   124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
   124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
   124,   125,   125,   126,   126,   127,   127,   128,   128,   129,
   129,   130,   130,   130,   130,   130,   130,   130,   131,   131,
   131,   132,   133,   133,   133,   133,   134,   134,   135,   135,
   135,   136,   137,   138,   139,   140,   141,   142,   142,   143,
   143,   144,   144,   145,   145,   146,   146,   147,   147,   148,
   148,   148,   148,   149,   150,   150,   150,   151,   151,   152,
   152,   152,   152,   152,   152,   153,   153,   154,   155,   155,
   155,   155,   156,   156,   157,   157,   157,   157,   158,   158,
   158,   159,   159,   160,   161,   161,   161,   162,   162,   162,
   162,   163,   164,   164,   165,   165,   166,   166,   166,   166,
   167,   167,   168,   168,   169,   169,   169,   169,   169,   169,
   170,   170,   171,   172,   172,   173,   173,   173,   173,   173,
   173,   173,   174,   174,   174,   174,   174,   174,   175,   175,
   176,   176,   177,   177,   177,   178,   178,   179,   180,   180,
   181,   181,   182,   182,   183,   183,   184,   184,   184,   184,
   185,   185,   186,   187,   188,   188,   189,   190,   190,   191,
   192,   193,   194,   195,   195,   195,   195,   195,   195,   196,
   196,   197,   198,   198,   199,   200,   200,   201,   201,   201,
   202,   203,   203,   203,   203,   204,   204,   205,   205,   206,
   206,   207,   207,   208,   208,   210,   209,   211,   212,   209,
   209,   213,   213,   213,   214,   214,   215,   217,   218,   216,
   219,   220,   216,   221,   222,   216,   223,   223,   224,   224,
   225,   226,   226,   227,   227,   228,   229,   229,   230,   230,
   231,   231,   231,   232,   232,   233,   233,   233,   234,   235,
   235,   236,   236,   237,   237,   238,   238,   238,   238,   238,
   238,   239,   240,   240,   241,   241,   242,   242,   242,   243,
   243,   244,   244,   244,   245,   246,   244,   247,   248,   244,
   249,   250,   244,   251,   251,   252,   252,   253,   253
};

static const short yyr2[] = {     0,
     2,     1,     2,     2,     1,     3,     2,     6,     6,     5,
     0,     2,     1,     1,     1,     0,     2,     2,     2,     1,
     2,     4,     4,     4,     4,     4,     4,     4,     4,     1,
     4,     1,     3,     4,     4,     4,     4,     3,     4,     2,
     4,     3,     3,     3,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     6,     6,    10,    10,
     6,     1,     6,     2,     2,     5,     7,     5,     7,     5,
     1,     1,     5,     2,     5,     5,     5,     1,     1,     5,
     7,     2,     1,     8,     6,     0,     2,     0,     2,     0,
     1,     4,     6,     4,     4,     4,     6,     4,     0,     3,
     6,     2,     1,     3,     2,     4,     0,     2,     0,     2,
     4,     1,     1,     1,     1,     1,     1,     4,     1,     0,
     2,     1,     3,     0,     3,     0,     2,     7,     7,     1,
     1,     1,     1,     1,     0,     2,     4,     0,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     6,     2,     2,
     3,     1,     7,     6,     2,     2,     3,     1,     1,     3,
     3,     1,     3,     3,     1,     4,     6,     4,     4,     1,
     4,     7,     1,     1,     0,     2,     4,     4,     4,     4,
     0,     2,     3,     1,     1,     1,     1,     1,     1,     1,
     0,     2,     5,     1,     4,     1,     1,     1,     1,     1,
     4,     5,     1,     1,     3,     1,     1,     1,     2,     0,
     2,     4,     1,     1,     1,     3,     0,     5,     2,     0,
     1,     3,     4,     5,     3,     3,     2,     4,     4,     5,
     1,     3,     1,     1,     1,     3,     1,     1,     1,     1,
     0,     0,     8,     1,     1,     1,     2,     2,     2,     5,
     5,     4,     1,     1,     4,     0,     2,     2,     1,     1,
     9,     1,     1,     2,     2,     0,     2,     1,     2,     1,
     3,     1,     2,     1,     2,     0,     9,     0,     0,    10,
     6,     3,     2,     2,     1,     0,     2,     0,     0,     6,
     0,     0,     6,     0,     0,     6,     1,     3,     1,     1,
     1,     0,     1,     1,     1,     0,     0,     2,     1,     2,
     4,     4,     7,     0,     2,     1,     1,     1,     1,     1,
     3,     2,     3,     0,     2,     2,     5,     4,     2,     2,
     1,     1,     1,     2,     3,     1,     1,     4,     1,     5,
     1,     1,     5,     4,     0,     0,     9,     0,     0,     9,
     0,     0,     9,     1,     5,     1,     1,     1,     3
};

static const short yydefact[] = {     0,
     2,    16,    13,    14,    15,   233,   130,   240,   238,   239,
   237,   234,   134,   242,   242,   242,   242,   242,   242,    72,
    50,   133,   242,   242,   242,   242,   242,   242,   242,   242,
   242,    51,    71,   242,    49,   132,   242,    48,   131,   242,
   242,   286,   242,   242,   242,   242,   242,     0,    47,    52,
   242,   242,   242,     0,     5,   241,    11,    20,    30,    56,
   242,   242,    62,    45,   235,    46,    53,    54,    55,     0,
    78,    79,    11,   274,     0,     0,    12,    16,    65,     0,
     0,   241,    74,     0,     0,     0,    88,   241,    64,     0,
     0,     0,     0,     0,    88,     0,     0,     0,     0,     0,
   278,     0,     0,   306,     0,   285,     0,   120,     0,     0,
     0,   119,     0,     0,     0,     0,     1,     7,     3,   242,
   242,   242,   242,   112,   113,   114,   242,    21,   242,   242,
   242,    40,   242,   115,   116,   117,   242,   242,   242,   242,
   242,   242,     0,     0,   242,   242,   242,   242,   242,    11,
     4,   275,    18,    19,    17,     0,   231,     0,   173,   175,
   174,     0,   220,   221,   241,   241,     0,     0,     0,    88,
    90,     0,     0,    20,     0,     0,     0,     0,     0,     0,
   124,     0,    83,     0,   175,     0,   287,   276,   304,   305,
   283,   307,   288,   291,   294,   284,   302,   175,   120,     0,
    38,    42,    43,   242,    44,   241,   120,   236,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   135,   135,     0,   109,    99,     0,    99,
     6,     0,   242,     0,   242,   242,   242,   242,   242,   191,
   175,     0,   242,     0,     0,     0,   241,    11,     0,   227,
   242,     0,     0,    89,     0,    91,     0,     0,     0,   242,
   242,   242,     0,   242,   242,   242,   126,     0,    82,   242,
   191,   279,   286,     0,   241,     0,   309,     0,     0,     0,
   282,   303,   191,   121,   242,     0,     0,   122,     0,    22,
    23,    24,    25,    29,    26,    27,    28,    31,    32,    37,
    39,    41,    34,    35,    36,    50,    49,    48,   135,   138,
     0,    45,   235,    54,   138,    90,    90,     0,   146,   109,
   147,   242,    90,     0,   107,    90,    90,   170,     0,   159,
     0,   165,     0,   232,    77,     0,     0,     0,     0,   242,
     0,   191,   176,     0,     0,   219,   242,   222,    11,    11,
    10,   241,     0,    76,   226,   225,   120,   242,     0,     0,
     0,    66,    73,    75,   120,    70,    68,     0,     0,     0,
     0,     0,   266,   286,     0,   306,   356,   357,   241,   241,
     0,   242,   324,     0,   319,   320,   308,   310,   300,   299,
     0,   297,     0,     0,     0,    80,   118,   242,   241,   242,
     0,   136,   139,     0,     0,     0,     0,    98,    96,     0,
   110,   102,    95,     0,     0,    99,     0,    94,    92,   242,
     0,   242,   242,   158,   242,     0,     0,     0,   242,   241,
   242,     0,   190,   189,   188,   181,   184,   185,   186,   187,
   181,   120,   120,     0,   242,   192,     0,   223,   218,     9,
     8,     0,   228,   229,     0,    61,     0,   242,   242,   152,
   242,    63,   242,     0,   125,   162,   127,   242,   242,    86,
     0,     0,   306,   307,     0,     0,   314,   324,   324,     0,
   242,   354,   332,   242,   242,   331,   322,   324,   324,   341,
   342,   242,   281,   241,   289,   301,     0,   292,   295,     0,
     0,     0,   244,   245,   246,    57,   123,    58,    33,   242,
   135,   242,    90,   109,     0,   103,   108,   100,    90,     0,
     0,   156,   155,   154,   160,   161,   164,     0,     0,     0,
   242,   178,   181,     0,   179,   177,   180,   203,   204,   200,
   208,   199,   207,   198,   206,   242,     0,   194,     0,   196,
   197,   172,   224,   230,     0,     0,   150,   149,   148,    67,
     0,     0,    69,    81,     0,     0,   242,   270,   267,   268,
     0,   263,   262,   307,   241,   311,   312,   318,   317,   316,
     0,   314,   323,   337,     0,   333,   329,   324,   336,   339,
   325,     0,     0,     0,   286,   242,   326,   330,   242,     0,
   321,   302,   298,   302,   302,   254,   253,     0,     0,     0,
   256,   242,   247,   248,   249,   128,   137,   129,    97,   111,
   107,     0,   105,    93,   171,   157,   168,   166,   169,   153,
   182,   183,     0,     0,   242,   210,     0,   151,     0,   163,
    86,    87,    85,     0,   269,   242,   265,   264,   241,     0,
   241,   315,   242,   286,   334,     0,   345,   348,   351,   354,
   302,   337,   324,   324,   306,   120,   290,   293,   296,     0,
     0,     0,     0,     0,   259,   260,     0,   256,   243,    99,
   104,     0,   205,   193,     0,   145,   144,   143,   215,   214,
     0,     0,   210,   217,   140,   213,   142,   242,   242,   242,
   271,   261,     0,   277,     0,   306,   335,   286,   241,   241,
   241,   306,   328,   306,   344,     0,     0,     0,   252,   272,
   258,   255,   257,   101,   106,   167,   195,     0,     0,   201,
   209,   242,   211,    60,    59,    84,   280,   313,   338,   340,
   358,     0,     0,     0,   343,   327,   355,   250,   251,   273,
   217,   213,   202,     0,   241,   346,   349,   352,   212,   216,
   359,   302,   302,   302,   306,   306,   306,   347,   350,   353,
     0,     0,     0
};

static const short yydefgoto[] = {   771,
    54,    55,    56,    77,    78,   112,    58,   298,    59,   182,
   183,   567,   169,   255,   256,   323,   324,   515,   416,   317,
   129,   130,   131,   140,   141,   142,   157,   200,   287,   267,
   370,    60,    61,    62,   310,   404,   311,   318,    89,   461,
    79,   425,   329,   465,   330,   331,   332,    63,   160,   240,
   241,   532,   533,   437,   341,   342,   547,   548,   549,   692,
   693,   694,   733,    83,   245,   163,   164,   167,   168,   158,
    64,    65,    66,    67,    68,    69,   383,    80,    71,   502,
   503,   504,   610,   505,   677,   678,    72,   571,   472,   569,
   570,   721,    73,    74,   273,   186,   374,   104,   484,   485,
   196,   278,   602,   279,   604,   280,   605,   391,   392,   497,
   281,   282,   192,   275,   276,   277,   581,   582,   741,   385,
   386,   586,   487,   488,   587,   588,   589,   489,   590,   709,
   762,   710,   763,   711,   764,   491,   492,   742
};

static const short yypact[] = {  1036,
-32768,   -31,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,    68,-32768,-32768,-32768,-32768,-32768,  1464,-32768,-32768,
-32768,-32768,-32768,    -9,-32768,   833,   330,  1250,   610,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   355,
-32768,-32768,   330,    19,    62,   113,-32768,   -31,-32768,  1464,
  1464,  1464,-32768,  1464,  1464,  1464,   218,  1464,-32768,  1464,
  1464,  1464,  1464,  1464,   218,  1464,  1464,  1464,   225,   225,
-32768,   163,   204,-32768,   242,   225,   225,  1464,  1464,  1464,
  1464,   221,   178,  1464,  1464,   225,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   191,   205,-32768,-32768,-32768,-32768,-32768,   330,
-32768,-32768,-32768,-32768,-32768,     4,   269,   270,   472,   173,
-32768,   155,   287,   303,   283,   291,   306,   327,   241,   218,
   355,   299,   316,  1143,   346,   352,   365,   293,   371,   376,
   418,    76,-32768,   398,   173,   373,-32768,-32768,-32768,-32768,
-32768,   375,-32768,-32768,-32768,-32768,   233,   173,  1250,   358,
   183,-32768,-32768,-32768,-32768,   472,  1250,-32768,  1464,  1464,
  1464,  1464,  1464,  1464,  1464,  1464,  1464,  1464,  1464,  1464,
  1464,  1464,  1464,  1571,  1571,  1464,   470,   341,  1464,   341,
-32768,  1464,-32768,  1464,-32768,-32768,-32768,-32768,-32768,   410,
   173,  1464,-32768,  1464,   430,  1464,  1464,   330,  1464,   427,
-32768,  1464,  1464,-32768,   459,-32768,  1464,  1464,  1464,-32768,
-32768,-32768,  1464,-32768,-32768,-32768,   457,  1464,-32768,-32768,
   410,-32768,    85,   419,   440,   458,   375,    45,    45,    45,
-32768,-32768,   410,-32768,-32768,  1464,   411,  1464,   403,   472,
   717,   438,   648,   441,   518,   382,   382,-32768,   628,    83,
-32768,-32768,   412,    83,    83,   423,   432,   436,  1357,   521,
   437,   453,   464,   465,   521,   318,   355,   476,-32768,   470,
-32768,-32768,   355,   439,   531,   570,   355,   652,   405,   201,
   534,     7,  1464,-32768,-32768,   522,   522,  1464,  1464,-32768,
   557,   410,-32768,   535,  1464,-32768,-32768,-32768,   330,   330,
-32768,   539,  1464,-32768,-32768,   327,  1250,-32768,   537,   563,
   572,-32768,-32768,-32768,  1250,-32768,-32768,  1464,  1464,   583,
   584,  1464,   524,    85,   233,-32768,-32768,-32768,   440,   440,
   517,-32768,   578,   587,-32768,   609,-32768,-32768,-32768,-32768,
   525,   552,   526,   529,   402,-32768,-32768,-32768,   472,-32768,
  1464,-32768,-32768,   540,  1464,   542,  1464,-32768,-32768,   225,
-32768,-32768,-32768,   451,   218,   341,  1464,-32768,-32768,-32768,
  1464,-32768,-32768,-32768,-32768,  1464,  1464,  1464,-32768,-32768,
-32768,   405,-32768,-32768,-32768,   522,   543,-32768,-32768,-32768,
   522,  1250,  1250,   120,-32768,-32768,  1464,-32768,-32768,-32768,
-32768,  1464,-32768,-32768,   533,-32768,  1464,-32768,-32768,-32768,
-32768,-32768,-32768,   544,-32768,   630,-32768,-32768,-32768,   739,
   218,   196,-32768,   375,   623,   631,    94,   591,   681,  1464,
-32768,   163,-32768,-32768,   267,-32768,-32768,   567,   681,-32768,
   233,-32768,-32768,   440,-32768,-32768,    45,-32768,-32768,   277,
   225,   638,   402,   402,   402,-32768,-32768,-32768,-32768,-32768,
  1357,-32768,   355,   470,   569,    33,-32768,-32768,   355,  1464,
   641,-32768,-32768,-32768,-32768,-32768,-32768,  1464,  1464,  1464,
-32768,-32768,   522,  1464,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,  1464,   581,   574,-32768,
-32768,-32768,-32768,-32768,  1464,   644,-32768,-32768,-32768,-32768,
  1464,  1464,-32768,-32768,  1464,  1464,-32768,   580,-32768,   218,
   645,   196,   196,   375,   440,-32768,-32768,-32768,-32768,-32768,
   589,    94,-32768,   219,   613,-32768,-32768,   681,-32768,-32768,
-32768,   225,   284,   370,   419,-32768,-32768,-32768,-32768,   594,
-32768,   233,-32768,   233,   233,-32768,-32768,   619,   634,  1464,
   247,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   531,   225,-32768,-32768,-32768,-32768,   202,   663,   621,-32768,
-32768,   472,   468,   678,-32768,   703,   690,-32768,   693,-32768,
   832,   472,-32768,  1464,-32768,-32768,-32768,-32768,   440,   695,
   440,-32768,-32768,   419,-32768,   654,-32768,-32768,-32768,-32768,
   233,   233,   681,   681,-32768,  1464,-32768,-32768,-32768,    48,
    48,   705,   225,    48,-32768,-32768,   714,   247,-32768,   341,
   451,  1464,-32768,-32768,   225,-32768,-32768,-32768,-32768,-32768,
   639,    38,   703,   737,-32768,   464,-32768,-32768,-32768,-32768,
   472,-32768,   720,-32768,   721,-32768,-32768,   419,   440,   440,
   440,-32768,-32768,-32768,-32768,   657,   722,   728,-32768,   225,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   105,   660,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   552,   662,   659,   661,-32768,-32768,-32768,-32768,-32768,-32768,
   737,-32768,-32768,  1464,   440,-32768,-32768,-32768,-32768,   472,
-32768,   233,   233,   233,-32768,-32768,-32768,-32768,-32768,-32768,
   770,   771,-32768
};

static const short yypgoto[] = {-32768,
   726,    24,-32768,   706,-32768,    82,   208,  -366,   558,-32768,
   601,   145,    -7,  -253,   723,  -223,-32768,  -489,   166,  -300,
-32768,-32768,-32768,-32768,-32768,-32768,   182,  -140,   395,-32768,
-32768,-32768,-32768,-32768,  -207,   482,  -549,  -364,  -375,-32768,
  -214,   366,    65,   237,  -335,-32768,   -99,-32768,   724,    69,
-32768,  -392,   176,-32768,  -203,-32768,-32768,-32768,-32768,   118,
-32768,    86,    64,-32768,-32768,   573,-32768,   -55,   561,   582,
   317,   -21,  -294,-32768,  -172,-32768,   149,   -15,-32768,  -106,
-32768,  -490,  -152,  -440,   140,-32768,-32768,   -34,-32768,   250,
-32768,   107,   758,  -395,-32768,-32768,-32768,  -219,   -25,    -8,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,  -256,-32768,    93,
  -501,   -98,  -263,  -405,   560,  -382,   253,-32768,  -269,   359,
-32768,  -308,   367,-32768,  -431,-32768,  -555,-32768,  -332,-32768,
-32768,-32768,-32768,-32768,-32768,   252,   588,  -512
};


#define	YYLAST		1685


static const short yytable[] = {    81,
    82,    84,    85,    86,   191,   384,   327,    87,    88,    90,
    91,    92,    93,    94,    95,    96,   105,   315,    97,   411,
   103,    98,   393,   394,    99,   100,   623,   107,   108,   109,
   110,   111,   466,   106,   509,   114,   115,   116,   177,   663,
     6,   439,   439,     9,    10,   143,   144,   523,   535,   516,
   490,   314,   314,   376,   321,   232,   729,   598,   284,    12,
   117,   606,   408,   409,   233,   153,   289,   373,   575,   413,
    75,    76,   418,   419,   486,     6,   572,   184,   185,   395,
   119,    57,    12,   558,   197,   198,   691,   178,   429,   573,
    12,   430,     6,    42,   208,   132,   151,   431,   707,    12,
   667,   402,   668,   669,   209,   210,   211,   212,    12,   475,
   476,   213,   474,   214,   215,   216,   154,   217,   607,    12,
   675,   218,   219,   220,   221,   222,   223,     6,   538,   226,
   227,   228,   229,   230,    12,   539,   314,    57,   446,   128,
   631,   439,   730,   691,   268,   622,   439,   321,    70,   551,
   389,   516,   740,   625,   473,   490,   655,   540,   541,   712,
   184,   156,   254,   440,   440,   162,   165,   166,   649,   583,
   676,   172,   173,   231,   101,   166,   572,   572,   137,   486,
   138,   139,   542,   543,   585,   544,   545,   675,   286,   573,
   573,   725,   518,   236,   585,   132,   355,   743,   744,   578,
   579,   242,   313,   313,    70,   320,   325,   522,   325,   574,
   689,   690,   237,   620,   238,   243,   455,   333,   426,   335,
   336,   337,   338,   339,   464,     6,   466,   345,   239,   113,
   161,   713,   714,   546,   427,   354,   161,   676,   439,    12,
   603,   321,   761,   557,   362,   363,   364,   105,   366,   367,
   368,   375,   380,   271,   372,   128,   390,   390,   390,   619,
   765,   766,   767,   440,   106,   624,   283,   204,   440,   396,
    42,   351,   274,   175,   176,   187,   191,   179,   180,   181,
   138,   139,   205,   429,     6,   188,   234,   313,   673,   159,
   606,    12,   431,   585,   224,   159,   189,   190,   320,   174,
  -235,   536,   537,   617,   235,   650,   412,   316,   225,   343,
   326,   189,   190,   248,   250,   199,   516,   244,   674,   501,
   246,   206,   207,   344,   444,   189,   190,   162,   349,   247,
   352,   449,     2,   166,     3,     4,     5,   249,   314,   360,
   251,   321,   456,   321,   252,   193,   145,   607,   105,   194,
   253,   195,   375,   146,   288,    12,   322,    18,   102,   257,
   440,   483,   258,   147,   407,   106,   478,   397,   585,   585,
  -285,   148,   450,   451,  -285,   149,  -285,     6,   377,   703,
   260,   705,   506,   145,   508,   378,   261,   657,   514,   596,
   146,   658,   599,   659,   325,   350,   613,   614,   615,   262,
   147,   715,   263,   170,   520,   264,   199,    90,   148,   524,
   265,   170,   149,   528,   199,   530,   290,   291,   292,   293,
   294,   295,   296,   297,   132,   346,     6,   377,   627,   552,
   629,   309,   309,    12,   378,   421,   422,   423,   359,   424,
   361,   266,   739,    90,   270,   559,   285,   560,   745,   371,
   746,   274,   563,   564,   272,   580,   724,   584,     6,   340,
   357,     9,    10,   697,   347,   592,   483,   584,   593,   594,
   365,-32768,   127,   500,   501,   390,   600,     6,   609,   611,
     9,    10,    12,   683,    12,   653,   170,   353,   513,   313,
   525,   526,   320,   358,   616,   399,   618,   369,   519,   382,
   453,   768,   769,   770,   387,   135,   136,   137,   321,   138,
   139,   436,   441,   400,   432,   630,   309,   717,   718,   398,
   697,   122,   123,   124,   125,   716,   448,   126,   127,     6,
   633,   127,     9,    10,   454,  -145,    12,   647,   648,   403,
   312,   312,   414,   319,  -144,   442,   443,   288,  -143,   405,
   467,   643,   415,   120,   121,   122,   123,   124,   125,   433,
   580,   126,   127,   653,   199,  -140,   584,   457,   458,   459,
   656,   460,   199,   662,     6,   377,  -141,  -142,   529,   470,
   664,    12,   378,   665,   434,     6,   377,   435,   410,   102,
   379,   445,    12,   378,   428,   447,   679,   462,   145,   452,
   681,   471,   521,-32768,-32768,   146,   463,   126,   127,   527,
   628,   208,   511,   479,   696,   147,   417,   468,   469,   685,
   477,   493,   132,   148,   479,   312,   494,   149,   553,   495,
   702,   496,   662,   554,   498,    53,   319,   706,   556,   499,
   132,   584,   584,   555,   510,   480,   512,   562,   595,   199,
   199,   720,   438,   438,   561,   534,   480,   576,   325,   189,
   190,   591,   635,   727,   132,   577,   201,   202,   203,   480,
  -286,   696,   612,   621,  -286,   626,  -286,   636,   638,   646,
   481,  -286,   734,   735,   736,  -286,   662,  -286,     6,   377,
   102,   481,   644,   651,   654,    12,   378,   666,   720,   482,
   670,   133,   134,   135,   136,   137,   752,   138,   139,   682,
     6,   431,   684,     9,    10,   671,   754,    12,   309,   401,
   134,   135,   136,   137,   698,   138,   139,   699,   634,   704,
   319,   517,   123,   124,   125,   708,   637,   126,   127,   719,
   686,   632,   639,   420,   134,   135,   136,   137,   722,   138,
   139,   728,   438,   732,   737,   738,   748,   438,   565,   480,
   550,   747,   749,   726,   753,   687,   756,   757,   688,   772,
   773,   758,   641,   642,   299,   300,   301,   302,   303,   304,
   305,   118,   269,   155,  -286,   700,   680,   568,  -286,   328,
  -286,   672,   150,   507,    53,   482,   406,   531,   640,   121,
   122,   123,   124,   125,   482,   482,   126,   127,   689,   690,
   731,   171,   356,   751,   759,   334,   608,   723,   348,   645,
   120,   121,   122,   123,   124,   125,   750,   312,   126,   127,
   319,   152,   319,   755,   652,     2,   388,     3,     4,     5,
     6,     7,     8,     9,    10,   661,    11,    12,    13,   438,
   566,   701,   601,     0,   597,    14,     0,    15,    16,    17,
    18,   381,    19,     0,     0,     0,     0,     0,     0,    20,
    21,    22,     0,   199,     0,    23,    24,    25,     0,     0,
    26,    27,     0,    28,     0,    29,   568,    30,     0,    31,
     0,    32,    33,     0,    34,    35,    36,    37,    38,    39,
    40,     0,   -11,     0,   482,     0,    41,    42,    43,     0,
   660,   482,     0,   120,   121,   122,   123,   124,   125,     0,
    44,   126,   127,     0,     0,   328,     0,     0,     0,    45,
     0,     0,    46,    47,     0,     0,    48,     0,    49,    50,
    51,     0,    52,   566,     0,     0,    53,     0,     0,     0,
     0,     0,   695,     0,     0,     0,     0,     0,   299,     0,
     0,   760,     0,     0,     0,     0,     0,     0,     0,     0,
   482,     0,     0,     0,     0,     0,     0,     0,     0,   482,
   482,     0,     0,   328,   328,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   319,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   695,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   482,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     1,     0,     2,     0,
     3,     4,     5,     6,     7,     8,     9,    10,     0,    11,
    12,    13,     0,     0,     0,     0,     0,     0,    14,     0,
    15,    16,    17,    18,  -241,    19,     0,     0,     0,     0,
     0,  -241,    20,    21,    22,     0,     0,   299,    23,    24,
    25,  -241,     0,    26,    27,   328,    28,   328,    29,  -241,
    30,     0,    31,  -241,    32,    33,     0,    34,    35,    36,
    37,    38,    39,    40,     0,   -11,     0,     0,     0,    41,
    42,    43,     0,     0,     0,     0,     0,     0,     0,   328,
     0,     0,     0,    44,     0,     0,     0,     0,     0,     0,
     0,     0,    45,     0,     0,    46,    47,     0,     0,    48,
     0,    49,    50,    51,     0,    52,     0,     0,     0,    53,
     6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
     0,     0,     0,     0,     0,    14,     0,    15,    16,    17,
     0,     0,    19,     0,     0,     0,     0,     0,     0,    20,
    21,    22,     0,     0,     0,    23,    24,    25,     0,     0,
    26,    27,     0,    28,     0,    29,     0,    30,     0,    31,
     0,    32,    33,   259,    34,    35,    36,    37,    38,    39,
    40,     0,     0,     0,     0,     0,    41,     0,    43,     0,
     0,     0,     0,     0,   120,   121,   122,   123,   124,   125,
    44,     0,   126,   127,     0,     0,     0,     0,     0,    45,
     0,     0,    46,    47,     0,     0,    48,     0,    49,    50,
    51,     0,    52,     0,     0,     0,    53,     6,     7,     8,
     9,    10,     0,    11,    12,    13,     0,     0,     0,     0,
     0,     0,    14,     0,    15,    16,    17,     0,     0,    19,
     0,     0,     0,     0,     0,     0,    20,    21,    22,     0,
     0,     0,    23,    24,    25,     0,     0,    26,    27,     0,
    28,     0,    29,     0,    30,     0,    31,     0,    32,    33,
     0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
     0,     0,     0,    41,     0,    43,     0,     0,     0,     0,
     0,   120,   121,   122,   123,   124,   125,    44,     0,   126,
   127,     0,     0,     0,     0,     0,    45,     0,     0,    46,
    47,     0,     0,    48,     0,    49,    50,    51,     0,    52,
     0,     0,     0,    53,     6,     7,     8,     9,    10,     0,
    11,    12,    13,     0,     0,     0,     0,     0,     0,    14,
     0,    15,    16,    17,     0,     0,    19,     0,     0,     0,
     0,     0,     0,    20,   306,    22,     0,     0,     0,    23,
    24,    25,     0,     0,    26,    27,     0,    28,     0,    29,
     0,    30,     0,    31,     0,    32,    33,     0,    34,   307,
    36,    37,   308,    39,    40,     0,     0,     0,     0,     0,
    41,     0,    43,     0,     0,     0,     0,     0,   120,   121,
   122,   123,   124,   125,    44,     0,   126,   127,     0,     0,
     0,     0,     0,    45,     0,     0,    46,    47,     0,     0,
    48,     0,    49,    50,    51,     0,    52,     0,     0,     0,
    53,     6,     7,     8,     9,    10,     0,    11,    12,    13,
     0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
    17,     0,     0,    19,     0,     0,     0,     0,     0,     0,
    20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
     0,    26,    27,     0,    28,     0,    29,     0,    30,     0,
    31,     0,    32,    33,     0,    34,    35,    36,    37,    38,
    39,    40,     0,     0,     0,     0,     0,    41,     0,    43,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    44,     0,     0,     0,     0,     0,     0,     0,     0,
    45,     0,     0,    46,    47,     0,     0,    48,     0,    49,
    50,    51,     0,    52,     0,     0,     0,    53,     6,     7,
     8,     9,    10,     0,    11,    12,    13,     0,     0,     0,
     0,     0,     0,    14,     0,    15,    16,    17,     0,     0,
    19,     0,     0,     0,     0,     0,     0,    20,   306,    22,
     0,     0,     0,    23,    24,    25,     0,     0,    26,    27,
     0,    28,     0,    29,     0,    30,     0,    31,     0,    32,
    33,     0,    34,   307,    36,    37,   308,    39,    40,     0,
     0,     0,     0,     0,    41,     0,    43,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    44,     0,
     0,     0,     0,     0,     0,     0,     0,    45,     0,     0,
    46,    47,     0,     0,    48,     0,    49,    50,    51,     0,
    52,     0,     0,     0,    53
};

static const short yycheck[] = {    15,
    16,    17,    18,    19,   103,   275,   230,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    42,   225,    34,   320,
    42,    37,   279,   280,    40,    41,   516,    43,    44,    45,
    46,    47,   368,    42,   401,    51,    52,    53,    94,   595,
     8,   336,   337,    11,    12,    61,    62,   423,   441,   414,
   383,   224,   225,   273,   227,    52,    19,   489,   199,    15,
    70,    14,   316,   317,    61,     4,   207,   271,   474,   323,
   102,   103,   326,   327,   383,     8,   472,    99,   100,   283,
    57,     0,    15,   459,   106,   107,   636,    95,    82,   472,
    15,    85,     8,    75,   116,    13,    73,    91,   654,    15,
   602,   309,   604,   605,   120,   121,   122,   123,    15,   379,
   380,   127,   376,   129,   130,   131,     4,   133,    71,    15,
   611,   137,   138,   139,   140,   141,   142,     8,     9,   145,
   146,   147,   148,   149,    15,    16,   309,    56,   342,    58,
   533,   436,   105,   693,    69,   113,   441,   320,     0,   444,
   106,   516,   708,   520,   374,   488,   588,    38,    39,   661,
   182,    80,   170,   336,   337,    84,    85,    86,   574,   478,
   611,    90,    91,   150,   107,    94,   572,   573,    96,   488,
    98,    99,    63,    64,   479,    66,    67,   678,   204,   572,
   573,   681,   416,    21,   489,    13,   252,   710,   711,   106,
   107,    47,   224,   225,    56,   227,   228,   422,   230,   473,
   106,   107,    40,   514,    42,    61,   357,   233,    18,   235,
   236,   237,   238,   239,   365,     8,   562,   243,    56,    48,
    82,   663,   664,   114,    34,   251,    88,   678,   533,    15,
   497,   414,   755,   458,   260,   261,   262,   273,   264,   265,
   266,   273,   274,   185,   270,   174,   278,   279,   280,   513,
   762,   763,   764,   436,   273,   519,   198,    47,   441,   285,
    75,   248,    77,    92,    93,   113,   375,    96,    97,    98,
    98,    99,   105,    82,     8,    82,    18,   309,    42,    82,
    14,    15,    91,   588,   104,    88,    93,    94,   320,    92,
    82,   442,   443,   511,    35,   575,   322,   226,   104,   241,
   229,    93,    94,   165,   166,   108,   681,    31,    72,    73,
    18,   114,   115,   242,   340,    93,    94,   246,   247,    47,
   249,   347,     3,   252,     5,     6,     7,    47,   511,   258,
    35,   514,   358,   516,    18,   104,    29,    71,   374,   108,
   110,   110,   374,    36,   206,    15,    16,    28,    42,    61,
   533,   383,    47,    46,    47,   374,   382,   286,   663,   664,
   104,    54,   349,   350,   108,    58,   110,     8,     9,   649,
    35,   651,   398,    29,   400,    16,    35,   104,   410,   488,
    36,   108,   491,   110,   416,   247,   503,   504,   505,    35,
    46,   665,   110,    87,   420,    35,   199,   423,    54,   425,
    35,    95,    58,   429,   207,   431,   209,   210,   211,   212,
   213,   214,   215,   216,    13,   244,     8,     9,   528,   445,
   530,   224,   225,    15,    16,    31,    32,    33,   257,    35,
   259,    24,   706,   459,    47,   461,    89,   463,   712,   268,
   714,    77,   468,   469,    82,   477,   680,   479,     8,    50,
   253,    11,    12,   636,    35,   481,   488,   489,   484,   485,
   263,    90,    91,    72,    73,   497,   492,     8,   500,   501,
    11,    12,    15,    16,    15,   584,   170,    61,   407,   511,
   426,   427,   514,    35,   510,   288,   512,    41,   417,    60,
   352,   765,   766,   767,    47,    94,    95,    96,   681,    98,
    99,   336,   337,   111,   333,   531,   309,   670,   671,   109,
   693,    84,    85,    86,    87,   666,   345,    90,    91,     8,
   546,    91,    11,    12,   353,   113,    15,   572,   573,    19,
   224,   225,   104,   227,   113,   338,   339,   399,   113,   113,
   369,   567,    22,    82,    83,    84,    85,    86,    87,    38,
   582,    90,    91,   662,   357,   113,   588,    31,    32,    33,
   592,    35,   365,   595,     8,     9,   113,   113,   430,   372,
   596,    15,    16,   599,    63,     8,     9,    66,   113,   273,
   274,    35,    15,    16,    61,    61,   612,    35,    29,    61,
   622,    78,   421,    86,    87,    36,    35,    90,    91,   428,
   529,   633,   405,    47,   636,    46,    47,    35,    35,   635,
   104,    35,    13,    54,    47,   309,    18,    58,   447,   105,
   646,    80,   654,   452,   109,   114,   320,   653,   457,   111,
    13,   663,   664,   111,   105,    79,   105,    18,    82,   442,
   443,   673,   336,   337,   111,   113,    79,    35,   680,    93,
    94,   480,    82,   685,    13,    35,   109,   110,   111,    79,
   104,   693,    35,   105,   108,    35,   110,   104,    35,    35,
   114,   104,   698,   699,   700,   108,   708,   110,     8,     9,
   374,   114,   113,   105,    82,    15,    16,   104,   720,   383,
    82,    92,    93,    94,    95,    96,   728,    98,    99,    47,
     8,    91,    35,    11,    12,    82,   732,    15,   511,    92,
    93,    94,    95,    96,    35,    98,    99,    35,   547,    35,
   414,   415,    85,    86,    87,    82,   555,    90,    91,    35,
    38,   534,   561,    92,    93,    94,    95,    96,    35,    98,
    99,   113,   436,    17,    35,    35,    35,   441,    20,    79,
   444,   105,    35,   682,   105,    63,   105,   109,    66,     0,
     0,   111,   565,   566,   217,   218,   219,   220,   221,   222,
   223,    56,   182,    78,   104,   641,   621,   471,   108,   232,
   110,   610,    70,   399,   114,   479,   315,   432,   562,    83,
    84,    85,    86,    87,   488,   489,    90,    91,   106,   107,
   693,    88,   252,   728,   751,   234,   500,   678,   246,   570,
    82,    83,    84,    85,    86,    87,   720,   511,    90,    91,
   514,    74,   516,   741,   582,     3,   277,     5,     6,     7,
     8,     9,    10,    11,    12,   594,    14,    15,    16,   533,
   112,   644,   494,    -1,   488,    23,    -1,    25,    26,    27,
    28,   274,    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,
    38,    39,    -1,   666,    -1,    43,    44,    45,    -1,    -1,
    48,    49,    -1,    51,    -1,    53,   570,    55,    -1,    57,
    -1,    59,    60,    -1,    62,    63,    64,    65,    66,    67,
    68,    -1,    70,    -1,   588,    -1,    74,    75,    76,    -1,
   594,   595,    -1,    82,    83,    84,    85,    86,    87,    -1,
    88,    90,    91,    -1,    -1,   368,    -1,    -1,    -1,    97,
    -1,    -1,   100,   101,    -1,    -1,   104,    -1,   106,   107,
   108,    -1,   110,   112,    -1,    -1,   114,    -1,    -1,    -1,
    -1,    -1,   636,    -1,    -1,    -1,    -1,    -1,   401,    -1,
    -1,   754,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
   654,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   663,
   664,    -1,    -1,   426,   427,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,   681,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   693,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,   708,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,     3,    -1,
     5,     6,     7,     8,     9,    10,    11,    12,    -1,    14,
    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,
    25,    26,    27,    28,    29,    30,    -1,    -1,    -1,    -1,
    -1,    36,    37,    38,    39,    -1,    -1,   520,    43,    44,
    45,    46,    -1,    48,    49,   528,    51,   530,    53,    54,
    55,    -1,    57,    58,    59,    60,    -1,    62,    63,    64,
    65,    66,    67,    68,    -1,    70,    -1,    -1,    -1,    74,
    75,    76,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   562,
    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    97,    -1,    -1,   100,   101,    -1,    -1,   104,
    -1,   106,   107,   108,    -1,   110,    -1,    -1,    -1,   114,
     8,     9,    10,    11,    12,    -1,    14,    15,    16,    -1,
    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,    26,    27,
    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,
    38,    39,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,
    48,    49,    -1,    51,    -1,    53,    -1,    55,    -1,    57,
    -1,    59,    60,    61,    62,    63,    64,    65,    66,    67,
    68,    -1,    -1,    -1,    -1,    -1,    74,    -1,    76,    -1,
    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    87,
    88,    -1,    90,    91,    -1,    -1,    -1,    -1,    -1,    97,
    -1,    -1,   100,   101,    -1,    -1,   104,    -1,   106,   107,
   108,    -1,   110,    -1,    -1,    -1,   114,     8,     9,    10,
    11,    12,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,
    -1,    -1,    23,    -1,    25,    26,    27,    -1,    -1,    30,
    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,    -1,
    -1,    -1,    43,    44,    45,    -1,    -1,    48,    49,    -1,
    51,    -1,    53,    -1,    55,    -1,    57,    -1,    59,    60,
    -1,    62,    63,    64,    65,    66,    67,    68,    -1,    -1,
    -1,    -1,    -1,    74,    -1,    76,    -1,    -1,    -1,    -1,
    -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
    91,    -1,    -1,    -1,    -1,    -1,    97,    -1,    -1,   100,
   101,    -1,    -1,   104,    -1,   106,   107,   108,    -1,   110,
    -1,    -1,    -1,   114,     8,     9,    10,    11,    12,    -1,
    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,
    -1,    25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,
    -1,    -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,
    44,    45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,
    -1,    55,    -1,    57,    -1,    59,    60,    -1,    62,    63,
    64,    65,    66,    67,    68,    -1,    -1,    -1,    -1,    -1,
    74,    -1,    76,    -1,    -1,    -1,    -1,    -1,    82,    83,
    84,    85,    86,    87,    88,    -1,    90,    91,    -1,    -1,
    -1,    -1,    -1,    97,    -1,    -1,   100,   101,    -1,    -1,
   104,    -1,   106,   107,   108,    -1,   110,    -1,    -1,    -1,
   114,     8,     9,    10,    11,    12,    -1,    14,    15,    16,
    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,    26,
    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,    -1,
    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,    -1,
    57,    -1,    59,    60,    -1,    62,    63,    64,    65,    66,
    67,    68,    -1,    -1,    -1,    -1,    -1,    74,    -1,    76,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    97,    -1,    -1,   100,   101,    -1,    -1,   104,    -1,   106,
   107,   108,    -1,   110,    -1,    -1,    -1,   114,     8,     9,
    10,    11,    12,    -1,    14,    15,    16,    -1,    -1,    -1,
    -1,    -1,    -1,    23,    -1,    25,    26,    27,    -1,    -1,
    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,
    -1,    -1,    -1,    43,    44,    45,    -1,    -1,    48,    49,
    -1,    51,    -1,    53,    -1,    55,    -1,    57,    -1,    59,
    60,    -1,    62,    63,    64,    65,    66,    67,    68,    -1,
    -1,    -1,    -1,    -1,    74,    -1,    76,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    97,    -1,    -1,
   100,   101,    -1,    -1,   104,    -1,   106,   107,   108,    -1,
   110,    -1,    -1,    -1,   114
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
{ yyval.t = newCTerm(PA_fMacro,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 81:
{ yyval.t = newCTerm(PA_fLoop,
				  newCTerm(PA_fAnd,yyvsp[-4].t,yyvsp[-2].t),
				  makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 82:
{
		    yyval.t = newCTerm(PA_fAnd,yyvsp[-1].t,yyvsp[0].t);
		  ;
    break;}
case 84:
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
case 85:
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
case 86:
{ yyval.t = 0; ;
    break;}
case 87:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 88:
{ yyval.t = AtomNil; ;
    break;}
case 89:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 90:
{ yyval.t = AtomNil; ;
    break;}
case 91:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 92:
{ yyval.t = oz_cons(newCTerm(PA_fRequire,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 93:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 94:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-1].t,
					   newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 95:
{ yyval.t = oz_cons(newCTerm(PA_fImport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 96:
{ yyval.t = oz_cons(newCTerm(PA_fExport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 97:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 98:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-1].t,
					   newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 99:
{ yyval.t = AtomNil; ;
    break;}
case 100:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-2].t,AtomNil,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 101:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 102:
{ yyval.t = newCTerm(PA_fVar,OZ_atom(xytext),yyvsp[0].t); ;
    break;}
case 103:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 104:
{ yyval.t = oz_mklist(oz_pair2(yyvsp[0].t,yyvsp[-2].t)); ;
    break;}
case 105:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 106:
{ yyval.t = oz_cons(oz_pair2(yyvsp[-1].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 107:
{ yyval.t = PA_fNoImportAt; ;
    break;}
case 108:
{ yyval.t = newCTerm(PA_fImportAt,yyvsp[0].t); ;
    break;}
case 109:
{ yyval.t = AtomNil; ;
    break;}
case 110:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 111:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,
					   newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t)),yyvsp[0].t); ;
    break;}
case 112:
{ yyval.t = OZ_atom(xytext); ;
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
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 119:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 120:
{ yyval.t = AtomNil; ;
    break;}
case 121:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 122:
{ yyval.t = newCTerm(PA_fAtom,AtomNil,yyvsp[0].t); ;
    break;}
case 123:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomCons,yyvsp[-2].t),
				  oz_mklist(yyvsp[-1].t,yyvsp[0].t)); ;
    break;}
case 124:
{ yyval.t = PA_fNoCatch; ;
    break;}
case 125:
{ yyval.t = newCTerm(PA_fCatch,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 126:
{ yyval.t = PA_fNoFinally; ;
    break;}
case 127:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 128:
{
		    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
				  newCTerm(PA_fAtom,yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
		  ;
    break;}
case 129:
{
		    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
				  makeVar(yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
		  ;
    break;}
case 130:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 131:
{ yyval.t = NameUnit; ;
    break;}
case 132:
{ yyval.t = NameTrue; ;
    break;}
case 133:
{ yyval.t = NameFalse; ;
    break;}
case 134:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 135:
{ yyval.t = AtomNil; ;
    break;}
case 136:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 137:
{ yyval.t = oz_cons(newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 138:
{ yyval.t = NameFalse; ;
    break;}
case 139:
{ yyval.t = NameTrue; ;
    break;}
case 140:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 141:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 142:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 143:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 144:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 145:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 146:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 147:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 148:
{ yyval.t = newCTerm(PA_fBoolCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 149:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 150:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 151:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 152:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 153:
{ checkDeprecation(yyvsp[-3].t);
		    yyval.t = newCTerm(PA_fBoolCase,yyvsp[-5].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-6].t,yyvsp[0].t));
		  ;
    break;}
case 154:
{ yyval.t = newCTerm(PA_fCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 155:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 156:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 157:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 158:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 159:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 160:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 161:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 162:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 163:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 164:
{ yyval.t = newCTerm(PA_fCaseClause,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 165:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 166:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-3].t,
				  newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 167:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-5].t,yyvsp[-2].t,yyvsp[0].t,yyvsp[-3].t); ;
    break;}
case 168:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 169:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 170:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 171:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
				  oz_cons(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 172:
{ yyval.t = newCTerm(PA_fClass,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 173:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 174:
{ yyval.t = newCTerm(PA_fDollar,yyvsp[0].t); ;
    break;}
case 175:
{ yyval.t = AtomNil; ;
    break;}
case 176:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 177:
{ yyval.t = newCTerm(PA_fFrom,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 178:
{ yyval.t = newCTerm(PA_fAttr,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 179:
{ yyval.t = newCTerm(PA_fFeat,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 180:
{ yyval.t = newCTerm(PA_fProp,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 181:
{ yyval.t = AtomNil; ;
    break;}
case 182:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 183:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 184:
{ yyval.t = yyvsp[0].t; ;
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
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 189:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 190:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 191:
{ yyval.t = AtomNil; ;
    break;}
case 192:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 193:
{ yyval.t = newCTerm(PA_fMeth,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-3].t); ;
    break;}
case 194:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 195:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 196:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 197:
{ yyval.t = yyvsp[0].t; ;
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
{ yyval.t = newCTerm(PA_fRecord,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 202:
{ yyval.t = newCTerm(PA_fOpenRecord,yyvsp[-4].t,yyvsp[-2].t); ;
    break;}
case 203:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 204:
{ yyval.t = makeVar(xytext); ;
    break;}
case 205:
{ yyval.t = newCTerm(PA_fEscape,makeVar(xytext),yyvsp[-1].t); ;
    break;}
case 206:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 207:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 208:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 209:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 210:
{ yyval.t = AtomNil; ;
    break;}
case 211:
{ yyval.t = newCTerm(PA_fMethArg,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 212:
{ yyval.t = newCTerm(PA_fMethColonArg,yyvsp[-3].t,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 213:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 214:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 215:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 216:
{ yyval.t = newCTerm(PA_fDefault,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 217:
{ yyval.t = PA_fNoDefault; ;
    break;}
case 218:
{ yyval.t = newCTerm(PA_fCond,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 219:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 220:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 221:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 222:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 223:
{ yyval.t = newCTerm(PA_fClause,newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 224:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 225:
{ yyval.t = oz_mklist(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 226:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 227:
{ yyval.t = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,yyvsp[0].t),
				  yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 228:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-3].t,yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 229:
{ yyval.t = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 230:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 231:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 232:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 233:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 234:
{ yyval.t = makeVar(xytext); ;
    break;}
case 235:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 236:
{ yyval.t = newCTerm(PA_fEscape,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 237:
{ yyval.t = makeString(xytext,pos()); ;
    break;}
case 238:
{ yyval.t = makeInt(xytext,pos()); ;
    break;}
case 239:
{ yyval.t = makeInt(xytext[0],pos()); ;
    break;}
case 240:
{ yyval.t = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); ;
    break;}
case 241:
{ yyval.t = pos(); ;
    break;}
case 242:
{ yyval.t = pos(); ;
    break;}
case 243:
{ OZ_Term prefix =
		      scannerPrefix? scannerPrefix: PA_zy;
		    yyval.t = newCTerm(PA_fScanner,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,prefix,
				  makeLongPos(yyvsp[-6].t,yyvsp[0].t)); ;
    break;}
case 244:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 245:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 246:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 247:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 248:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 249:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 250:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 251:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 252:
{ yyval.t = newCTerm(PA_fLexicalRule,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 253:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 254:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 255:
{ yyval.t = newCTerm(PA_fMode,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 256:
{ yyval.t = AtomNil; ;
    break;}
case 257:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 258:
{ yyval.t = newCTerm(PA_fInheritedModes,yyvsp[0].t); ;
    break;}
case 259:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 260:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 261:
{ OZ_Term expect = parserExpect? parserExpect: newSmallInt(0);
		    yyval.t = newCTerm(PA_fParser,yyvsp[-6].t,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,expect,
				  makeLongPos(yyvsp[-7].t,yyvsp[0].t)); ;
    break;}
case 262:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 263:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 264:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 265:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 266:
{ yyval.t = newCTerm(PA_fToken,AtomNil); ;
    break;}
case 267:
{ yyval.t = newCTerm(PA_fToken,yyvsp[0].t); ;
    break;}
case 268:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 269:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 270:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 271:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 272:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 273:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 274:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 275:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 276:
{ *prodKey[depth]++ = '='; ;
    break;}
case 277:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 278:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 279:
{ *prodKey[depth]++ = '='; ;
    break;}
case 280:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 281:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,PA_none); ;
    break;}
case 282:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 283:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 284:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 287:
{ prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg(yyvsp[-1].t,0))); ;
    break;}
case 288:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 289:
{ depth--; ;
    break;}
case 290:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 291:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 292:
{ depth--; ;
    break;}
case 293:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 294:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 295:
{ depth--; ;
    break;}
case 296:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 297:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 298:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 299:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 300:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 301:
{ *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; ;
    break;}
case 304:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 305:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 306:
{ *prodKey[depth] = '\0';
		    yyval.t = oz_pair2(prodName[depth],OZ_string(prodKeyBuffer[depth]));
		    prodName[depth] = PA_none;
		    prodKey[depth] = prodKeyBuffer[depth];
		  ;
    break;}
case 307:
{ yyval.t = AtomNil; ;
    break;}
case 308:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 309:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 310:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 311:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 312:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 313:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 314:
{ yyval.t = AtomNil; ;
    break;}
case 315:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 316:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 317:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 318:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 319:
{ yyval.t = newCTerm(PA_fSynAlternative, yyvsp[0].t); ;
    break;}
case 320:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 321:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 322:
{ OZ_Term t = yyvsp[0].t;
		    while (terms[depth]) {
		      t = oz_cons(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    yyval.t = newCTerm(PA_fSynSequence, decls[depth], t, yyvsp[-1].t);
		    decls[depth] = AtomNil;
		  ;
    break;}
case 323:
{ yyval.t = newCTerm(PA_fSynSequence, AtomNil, yyvsp[0].t, yyvsp[-1].t); ;
    break;}
case 324:
{ yyval.t = AtomNil; ;
    break;}
case 325:
{ yyval.t = oz_mklist(newCTerm(PA_fSynAction,yyvsp[0].t)); ;
    break;}
case 326:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 327:
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
case 328:
{ yyval.t = oz_cons(newCTerm(PA_fSynAssignment, terms[depth]->term, yyvsp[-1].t),
				  yyvsp[0].t);
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  ;
    break;}
case 329:
{ while (terms[depth]) {
		      decls[depth] = oz_cons(terms[depth]->term, decls[depth]);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    yyval.t = yyvsp[0].t;
		  ;
    break;}
case 330:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 331:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 332:
{ terms[depth] = new TermNode(yyvsp[0].t, terms[depth]); ;
    break;}
case 333:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 334:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 335:
{ yyval.t = newCTerm(PA_fSynAssignment,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 336:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 337:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 338:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_cons(newCTerm(PA_fSynApplication,yyvsp[-3].t,
						    AtomNil),
					   AtomNil),yyvsp[-1].t);
		  ;
    break;}
case 339:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 340:
{ yyval.t = newCTerm(PA_fSynAssignment,
				  newCTerm(PA_fEscape,yyvsp[-2].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 341:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 342:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 343:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_mklist(yyvsp[-2].t),yyvsp[-3].t);
		  ;
    break;}
case 344:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_mklist(yyvsp[-3].t),yyvsp[-1].t);
		  ;
    break;}
case 345:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 346:
{ depth--; ;
    break;}
case 347:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 348:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 349:
{ depth--; ;
    break;}
case 350:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 351:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 352:
{ depth--; ;
    break;}
case 353:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 354:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 355:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[-4].t,yyvsp[-1].t); ;
    break;}
case 356:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 357:
{ yyval.t = makeVar(xytext); ;
    break;}
case 358:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 359:
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
