
/*  A Bison parser, made from /usr/staff/glynn/MOZART/uniform/mozart/platform/emulator/parser.yy
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
#define	T_COLONEQUALS	339
#define	T_orelse	340
#define	T_andthen	341
#define	T_COMPARE	342
#define	T_FDCOMPARE	343
#define	T_LMACRO	344
#define	T_RMACRO	345
#define	T_FDIN	346
#define	T_ADD	347
#define	T_FDMUL	348
#define	T_OTHERMUL	349
#define	T_DEREFF	350


//
// See Oz/tools/compiler/Doc/TupleSyntax for an description of the
// generated parse trees.
//

// 
// To support '. :=' as a multifix operator (see uniform state proposal) 
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
	// Should I delete term t, 
        // or will it be reclaimed by GC? (and HOW!) ???             KG
	//     delete t;
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



#define	YYFINAL		810
#define	YYFLAG		-32768
#define	YYNTBASE	118

#define YYTRANSLATE(x) ((unsigned)(x) <= 350 ? yytranslate[x] : 263)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   117,     2,    95,   110,     2,     2,     2,   107,
   108,     2,   105,    99,   106,   101,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,   115,   116,     2,
    83,     2,     2,   103,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   111,     2,   112,   102,   109,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   113,    94,   114,   100,     2,     2,     2,     2,
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
    88,    89,    90,    91,    92,    93,    96,    97,    98,   104
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
   323,   329,   337,   345,   346,   349,   353,   357,   361,   363,
   368,   372,   378,   379,   382,   385,   386,   389,   392,   394,
   403,   410,   411,   414,   415,   418,   419,   421,   426,   433,
   438,   443,   448,   455,   460,   461,   465,   472,   475,   477,
   481,   484,   489,   490,   493,   494,   497,   502,   504,   506,
   508,   510,   512,   514,   519,   521,   522,   525,   527,   531,
   532,   536,   537,   540,   548,   556,   558,   560,   562,   564,
   566,   567,   570,   575,   576,   578,   580,   582,   584,   586,
   588,   590,   592,   594,   601,   604,   607,   611,   613,   621,
   628,   631,   634,   638,   640,   642,   646,   650,   652,   656,
   660,   662,   667,   674,   679,   684,   686,   691,   699,   701,
   703,   704,   707,   712,   717,   722,   727,   728,   731,   735,
   737,   739,   741,   743,   745,   747,   749,   750,   753,   759,
   761,   766,   768,   770,   772,   774,   776,   781,   787,   789,
   791,   795,   797,   799,   801,   804,   805,   808,   813,   815,
   817,   819,   823,   824,   830,   833,   834,   836,   840,   845,
   851,   855,   859,   862,   867,   872,   878,   880,   884,   886,
   888,   890,   894,   896,   898,   900,   902,   903,   904,   913,
   915,   917,   919,   922,   925,   928,   934,   940,   945,   947,
   949,   954,   955,   958,   961,   963,   965,   975,   977,   979,
   982,   985,   986,   989,   991,   994,   996,  1000,  1002,  1005,
  1007,  1010,  1011,  1021,  1022,  1023,  1034,  1041,  1045,  1048,
  1051,  1053,  1054,  1057,  1058,  1059,  1066,  1067,  1068,  1075,
  1076,  1077,  1084,  1086,  1090,  1092,  1094,  1096,  1097,  1099,
  1101,  1103,  1104,  1105,  1108,  1110,  1113,  1118,  1123,  1131,
  1132,  1135,  1137,  1139,  1141,  1143,  1145,  1149,  1152,  1156,
  1157,  1160,  1163,  1169,  1174,  1177,  1180,  1182,  1184,  1186,
  1189,  1193,  1195,  1197,  1202,  1204,  1210,  1212,  1214,  1220,
  1225,  1226,  1227,  1237,  1238,  1239,  1249,  1250,  1251,  1261,
  1263,  1269,  1271,  1273,  1275
};

static const short yyrhs[] = {   119,
    71,     0,     1,     0,   124,   120,     0,   217,   120,     0,
   120,     0,   201,   139,   120,     0,   121,   119,     0,    28,
   202,   124,    47,   201,   120,     0,    28,   202,   124,    47,
   124,   120,     0,    28,   202,   124,   201,   120,     0,     0,
     3,   122,     0,     5,     0,     6,     0,     7,     0,     0,
   123,   122,     0,   105,     4,     0,   106,     4,     0,   125,
     0,   125,   124,     0,   125,    83,   202,   125,     0,   125,
    86,   202,   125,     0,   125,    84,   202,   125,     0,   125,
    87,   202,   125,     0,   125,    88,   202,   125,     0,   125,
   145,   202,   125,     0,   125,   146,   202,   125,     0,   125,
   147,   202,   125,     0,   125,    94,   202,   125,     0,   127,
     0,   127,    95,   202,   126,     0,   127,     0,   127,    95,
   126,     0,   127,   148,   202,   127,     0,   127,   149,   202,
   127,     0,   127,   150,   202,   127,     0,   127,    99,   202,
   127,     0,   100,   202,   127,     0,   127,   101,   202,   127,
     0,   127,    13,     0,   127,   102,   202,   127,     0,   103,
   202,   127,     0,   104,   202,   127,     0,   107,   151,   108,
     0,   195,     0,   197,     0,   109,     0,    66,     0,    63,
     0,    38,     0,    59,     0,   110,     0,   198,     0,   199,
     0,   200,     0,   156,     0,   111,   202,   125,   153,   112,
   202,     0,   113,   202,   125,   152,   114,   202,     0,    55,
   202,   137,   113,   125,   152,   114,   151,    35,   202,     0,
    43,   202,   137,   113,   125,   152,   114,   151,    35,   202,
     0,    44,   202,   173,   138,    35,   202,     0,   172,     0,
    48,   202,   124,    47,   124,    35,     0,    45,   163,     0,
    23,   165,     0,    49,   202,   151,    35,   202,     0,    49,
   202,   125,    61,   151,    35,   202,     0,    62,   202,   151,
    35,   202,     0,    65,   202,   151,   154,   155,    35,   202,
     0,    57,   202,   151,    35,   202,     0,    60,     0,    37,
     0,    51,   202,   151,    35,   202,     0,    27,   188,     0,
    53,   202,   192,    35,   202,     0,    30,   202,   192,    35,
   202,     0,    25,   202,   194,    35,   202,     0,   203,     0,
   211,     0,    91,   202,   152,    92,   202,     0,    68,   202,
   134,    70,   151,    35,   202,     0,    69,   202,   128,    70,
   151,    35,   202,     0,     0,   129,   128,     0,   195,   115,
   125,     0,   125,    47,   130,     0,   125,    42,   125,     0,
   125,     0,   125,    20,   125,   131,     0,   125,   116,   132,
     0,   107,   125,   116,   132,   108,     0,     0,   116,   125,
     0,   125,   133,     0,     0,   116,   125,     0,   134,   135,
     0,   135,     0,   196,    47,   202,   125,    20,   125,   136,
   202,     0,   196,    47,   202,   125,   136,   202,     0,     0,
   116,   125,     0,     0,   195,   137,     0,     0,   139,     0,
    58,   202,   140,   138,     0,    54,   202,   124,    47,   124,
   138,     0,    54,   202,   124,   138,     0,    46,   202,   140,
   138,     0,    36,   202,   144,   138,     0,    29,   202,   124,
    47,   124,   138,     0,    29,   202,   124,   138,     0,     0,
   196,   143,   140,     0,   141,   107,   142,   108,   143,   140,
     0,    16,   202,     0,   162,     0,   162,   115,   196,     0,
   162,   142,     0,   162,   115,   196,   142,     0,     0,    22,
   195,     0,     0,   196,   144,     0,   162,   115,   196,   144,
     0,    89,     0,    90,     0,    93,     0,    96,     0,    97,
     0,    98,     0,   124,    47,   202,   124,     0,   124,     0,
     0,   125,   152,     0,   201,     0,   201,   125,   153,     0,
     0,    24,   202,   168,     0,     0,    41,   151,     0,   157,
   202,   107,   159,   160,   108,   202,     0,   158,   202,   107,
   159,   160,   108,   202,     0,     9,     0,    67,     0,    64,
     0,    39,     0,    16,     0,     0,   125,   159,     0,   161,
   115,   125,   159,     0,     0,    19,     0,   195,     0,   196,
     0,   199,     0,    66,     0,    63,     0,    38,     0,   195,
     0,   199,     0,   202,   124,    61,   151,   164,   202,     0,
    33,   163,     0,    32,   165,     0,    31,   151,    35,     0,
    35,     0,   202,   124,    61,   202,   151,   166,   202,     0,
   202,   124,    52,   167,   166,   202,     0,    33,   163,     0,
    32,   165,     0,    31,   151,    35,     0,    35,     0,   169,
     0,   169,    18,   167,     0,   169,    34,   167,     0,   169,
     0,   169,    18,   168,     0,   170,    61,   151,     0,   171,
     0,   171,    88,   201,   124,     0,   171,    88,   201,   124,
    47,   124,     0,   171,    83,   202,   171,     0,   171,    94,
   202,   171,     0,   127,     0,   127,    95,   202,   126,     0,
    26,   202,   173,   174,   179,    35,   202,     0,   125,     0,
   201,     0,     0,   175,   174,     0,    42,   202,   125,   152,
     0,    21,   202,   177,   176,     0,    40,   202,   177,   176,
     0,    56,   202,   125,   152,     0,     0,   177,   176,     0,
   178,   115,   125,     0,   178,     0,   195,     0,   197,     0,
   199,     0,    66,     0,    63,     0,    38,     0,     0,   180,
   179,     0,    50,   202,   181,   151,    35,     0,   182,     0,
   182,    83,   202,   196,     0,   195,     0,   197,     0,    66,
     0,    63,     0,    38,     0,   183,   107,   184,   108,     0,
   183,   107,   184,    19,   108,     0,     9,     0,    16,     0,
   117,   202,    16,     0,    67,     0,    64,     0,    39,     0,
   185,   184,     0,     0,   186,   187,     0,   161,   115,   186,
   187,     0,   196,     0,   110,     0,   109,     0,    17,   202,
   125,     0,     0,   202,   190,   189,    35,   202,     0,    31,
   151,     0,     0,   191,     0,   191,    18,   190,     0,   124,
    61,   202,   151,     0,   124,    47,   124,    61,   151,     0,
   193,    18,   193,     0,   193,    18,   192,     0,   124,   201,
     0,   124,    47,   124,   201,     0,   124,   201,    61,   151,
     0,   124,    47,   124,    61,   151,     0,   151,     0,   151,
    18,   194,     0,     8,     0,    15,     0,   196,     0,   117,
   202,   196,     0,    14,     0,    11,     0,    12,     0,    10,
     0,     0,     0,    77,   202,   196,   174,   179,   204,    35,
   202,     0,   205,     0,   206,     0,   208,     0,   205,   204,
     0,   206,   204,     0,   208,   204,     0,    73,   195,    83,
   207,    35,     0,    73,   196,    83,   207,    35,     0,    73,
   207,   151,    35,     0,    72,     0,    14,     0,    74,   196,
   209,    35,     0,     0,   210,   209,     0,    42,   216,     0,
   206,     0,   208,     0,    75,   202,   196,   174,   179,   213,
   212,    35,   202,     0,   240,     0,   218,     0,   240,   212,
     0,   218,   212,     0,     0,    79,   214,     0,   215,     0,
   215,   214,     0,   195,     0,   195,   115,   125,     0,   196,
     0,   196,   216,     0,   218,     0,   218,   217,     0,     0,
    76,   196,    83,   219,   222,   237,   238,   243,    35,     0,
     0,     0,    76,   110,   220,    83,   221,   222,   237,   238,
   243,    35,     0,    76,   222,   237,   238,   243,    35,     0,
   224,   196,   235,     0,   196,   236,     0,   223,   225,     0,
   224,     0,     0,   195,   115,     0,     0,     0,   107,   226,
   232,   108,   227,   235,     0,     0,     0,   111,   228,   232,
   112,   229,   235,     0,     0,     0,   113,   230,   232,   114,
   231,   235,     0,   233,     0,   233,   234,   232,     0,   196,
     0,   109,     0,    81,     0,     0,   236,     0,    96,     0,
    97,     0,     0,     0,   239,    47,     0,   240,     0,   240,
   239,     0,    78,   195,   243,    35,     0,    78,   196,   243,
    35,     0,    78,   261,   107,   241,   108,   243,    35,     0,
     0,   242,   241,     0,   196,     0,   110,     0,   109,     0,
   244,     0,   245,     0,   245,    18,   244,     0,   201,   247,
     0,    60,   202,   246,     0,     0,    80,   151,     0,   248,
   247,     0,   248,   236,   202,   249,   237,     0,   248,    83,
   251,   249,     0,    47,   249,     0,   252,   249,     0,   246,
     0,   196,     0,   246,     0,   250,   249,     0,   197,    83,
   251,     0,   251,     0,   196,     0,   196,   236,   202,   237,
     0,   253,     0,   117,   202,   196,    83,   251,     0,   253,
     0,   260,     0,   224,   202,   260,   235,   237,     0,   260,
   236,   202,   237,     0,     0,     0,   223,   202,   107,   254,
   262,   108,   255,   235,   237,     0,     0,     0,   223,   202,
   111,   256,   262,   112,   257,   235,   237,     0,     0,     0,
   223,   202,   113,   258,   262,   114,   259,   235,   237,     0,
   195,     0,   261,   202,   107,   152,   108,     0,     9,     0,
    16,     0,   243,     0,   243,   234,   262,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   753,   755,   759,   761,   764,   766,   771,   773,   776,   778,
   781,   785,   787,   789,   791,   795,   797,   801,   808,   817,
   819,   823,   825,   833,   835,   837,   839,   842,   844,   846,
   848,   850,   856,   858,   862,   865,   868,   871,   873,   876,
   879,   882,   885,   887,   890,   892,   894,   896,   898,   900,
   902,   904,   906,   908,   910,   912,   914,   916,   920,   922,
   925,   928,   930,   932,   934,   936,   938,   940,   942,   944,
   946,   948,   950,   952,   954,   956,   958,   960,   962,   964,
   966,   968,   972,   976,   978,   982,   984,   986,   990,   992,
   994,   997,  1002,  1004,  1008,  1012,  1014,  1018,  1022,  1025,
  1039,  1063,  1064,  1068,  1070,  1075,  1077,  1082,  1084,  1086,
  1089,  1091,  1093,  1095,  1100,  1102,  1104,  1108,  1112,  1114,
  1116,  1118,  1122,  1124,  1128,  1130,  1132,  1137,  1141,  1145,
  1149,  1153,  1157,  1161,  1163,  1167,  1169,  1173,  1175,  1181,
  1183,  1187,  1189,  1193,  1198,  1205,  1207,  1209,  1211,  1215,
  1219,  1221,  1223,  1227,  1229,  1233,  1235,  1237,  1239,  1241,
  1243,  1247,  1249,  1253,  1257,  1259,  1261,  1263,  1267,  1271,
  1275,  1277,  1279,  1281,  1285,  1287,  1289,  1293,  1295,  1299,
  1303,  1305,  1308,  1312,  1314,  1316,  1318,  1324,  1329,  1331,
  1336,  1338,  1342,  1344,  1346,  1348,  1352,  1354,  1358,  1360,
  1364,  1366,  1368,  1370,  1372,  1374,  1378,  1380,  1384,  1388,
  1390,  1394,  1396,  1398,  1400,  1402,  1404,  1406,  1410,  1412,
  1414,  1416,  1418,  1420,  1424,  1426,  1430,  1432,  1436,  1438,
  1440,  1445,  1447,  1451,  1455,  1457,  1461,  1463,  1467,  1469,
  1473,  1475,  1479,  1483,  1485,  1488,  1492,  1494,  1498,  1502,
  1506,  1508,  1512,  1516,  1518,  1522,  1526,  1530,  1540,  1548,
  1550,  1552,  1554,  1556,  1558,  1562,  1564,  1568,  1572,  1574,
  1578,  1582,  1584,  1588,  1590,  1592,  1598,  1606,  1608,  1610,
  1612,  1616,  1618,  1622,  1624,  1628,  1630,  1634,  1636,  1640,
  1642,  1646,  1648,  1650,  1651,  1652,  1654,  1658,  1660,  1662,
  1666,  1667,  1670,  1674,  1675,  1675,  1676,  1677,  1677,  1678,
  1679,  1679,  1682,  1684,  1688,  1689,  1692,  1696,  1697,  1700,
  1701,  1704,  1712,  1714,  1718,  1720,  1724,  1726,  1728,  1732,
  1734,  1738,  1740,  1742,  1746,  1750,  1752,  1756,  1765,  1769,
  1771,  1775,  1777,  1787,  1792,  1799,  1801,  1805,  1809,  1811,
  1815,  1817,  1821,  1823,  1829,  1833,  1836,  1841,  1843,  1847,
  1851,  1852,  1853,  1855,  1856,  1857,  1859,  1860,  1861,  1865,
  1867,  1871,  1873,  1878,  1880
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
"T_REDUCE","T_SEP","T_ITER","'='","T_OOASSIGN","T_DOTASSIGN","T_COLONEQUALS",
"T_orelse","T_andthen","T_COMPARE","T_FDCOMPARE","T_LMACRO","T_RMACRO","T_FDIN",
"'|'","'#'","T_ADD","T_FDMUL","T_OTHERMUL","','","'~'","'.'","'^'","'@'","T_DEREFF",
"'+'","'-'","'('","')'","'_'","'$'","'['","']'","'{'","'}'","':'","';'","'!'",
"file","queries","queries1","directive","switchList","switch","sequence","phrase",
"hashes","phrase2","FOR_decls","FOR_decl","FOR_gen","FOR_genOptInt","FOR_genOptC",
"FOR_genOptC2","iterators","iterator","optIteratorStep","procFlags","optFunctorDescriptorList",
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
   118,   118,   119,   119,   119,   119,   120,   120,   120,   120,
   120,   121,   121,   121,   121,   122,   122,   123,   123,   124,
   124,   125,   125,   125,   125,   125,   125,   125,   125,   125,
   125,   125,   126,   126,   127,   127,   127,   127,   127,   127,
   127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
   127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
   127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
   127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
   127,   127,   127,   128,   128,   129,   129,   129,   130,   130,
   130,   130,   131,   131,   132,   133,   133,   134,   134,   135,
   135,   136,   136,   137,   137,   138,   138,   139,   139,   139,
   139,   139,   139,   139,   140,   140,   140,   141,   142,   142,
   142,   142,   143,   143,   144,   144,   144,   145,   146,   147,
   148,   149,   150,   151,   151,   152,   152,   153,   153,   154,
   154,   155,   155,   156,   156,   157,   157,   157,   157,   158,
   159,   159,   159,   160,   160,   161,   161,   161,   161,   161,
   161,   162,   162,   163,   164,   164,   164,   164,   165,   165,
   166,   166,   166,   166,   167,   167,   167,   168,   168,   169,
   170,   170,   170,   171,   171,   171,   171,   172,   173,   173,
   174,   174,   175,   175,   175,   175,   176,   176,   177,   177,
   178,   178,   178,   178,   178,   178,   179,   179,   180,   181,
   181,   182,   182,   182,   182,   182,   182,   182,   183,   183,
   183,   183,   183,   183,   184,   184,   185,   185,   186,   186,
   186,   187,   187,   188,   189,   189,   190,   190,   191,   191,
   192,   192,   193,   193,   193,   193,   194,   194,   195,   196,
   197,   197,   198,   199,   199,   200,   201,   202,   203,   204,
   204,   204,   204,   204,   204,   205,   205,   206,   207,   207,
   208,   209,   209,   210,   210,   210,   211,   212,   212,   212,
   212,   213,   213,   214,   214,   215,   215,   216,   216,   217,
   217,   219,   218,   220,   221,   218,   218,   222,   222,   222,
   223,   223,   224,   226,   227,   225,   228,   229,   225,   230,
   231,   225,   232,   232,   233,   233,   234,   235,   235,   236,
   236,   237,   238,   238,   239,   239,   240,   240,   240,   241,
   241,   242,   242,   242,   243,   244,   244,   245,   245,   246,
   246,   247,   247,   247,   247,   247,   247,   248,   249,   249,
   250,   250,   251,   251,   251,   252,   252,   253,   253,   253,
   254,   255,   253,   256,   257,   253,   258,   259,   253,   260,
   260,   261,   261,   262,   262
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
     5,     7,     7,     0,     2,     3,     3,     3,     1,     4,
     3,     5,     0,     2,     2,     0,     2,     2,     1,     8,
     6,     0,     2,     0,     2,     0,     1,     4,     6,     4,
     4,     4,     6,     4,     0,     3,     6,     2,     1,     3,
     2,     4,     0,     2,     0,     2,     4,     1,     1,     1,
     1,     1,     1,     4,     1,     0,     2,     1,     3,     0,
     3,     0,     2,     7,     7,     1,     1,     1,     1,     1,
     0,     2,     4,     0,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     6,     2,     2,     3,     1,     7,     6,
     2,     2,     3,     1,     1,     3,     3,     1,     3,     3,
     1,     4,     6,     4,     4,     1,     4,     7,     1,     1,
     0,     2,     4,     4,     4,     4,     0,     2,     3,     1,
     1,     1,     1,     1,     1,     1,     0,     2,     5,     1,
     4,     1,     1,     1,     1,     1,     4,     5,     1,     1,
     3,     1,     1,     1,     2,     0,     2,     4,     1,     1,
     1,     3,     0,     5,     2,     0,     1,     3,     4,     5,
     3,     3,     2,     4,     4,     5,     1,     3,     1,     1,
     1,     3,     1,     1,     1,     1,     0,     0,     8,     1,
     1,     1,     2,     2,     2,     5,     5,     4,     1,     1,
     4,     0,     2,     2,     1,     1,     9,     1,     1,     2,
     2,     0,     2,     1,     2,     1,     3,     1,     2,     1,
     2,     0,     9,     0,     0,    10,     6,     3,     2,     2,
     1,     0,     2,     0,     0,     6,     0,     0,     6,     0,
     0,     6,     1,     3,     1,     1,     1,     0,     1,     1,
     1,     0,     0,     2,     1,     2,     4,     4,     7,     0,
     2,     1,     1,     1,     1,     1,     3,     2,     3,     0,
     2,     2,     5,     4,     2,     2,     1,     1,     1,     2,
     3,     1,     1,     4,     1,     5,     1,     1,     5,     4,
     0,     0,     9,     0,     0,     9,     0,     0,     9,     1,
     5,     1,     1,     1,     3
};

static const short yydefact[] = {     0,
     2,    16,    13,    14,    15,   249,   146,   256,   254,   255,
   253,   250,   150,   258,   258,   258,   258,   258,   258,    73,
    51,   149,   258,   258,   258,   258,   258,   258,   258,   258,
   258,    52,    72,   258,    50,   148,   258,    49,   147,   258,
   258,   258,   302,   258,   258,   258,   258,   258,     0,    48,
    53,   258,   258,   258,     0,     5,   257,    11,    20,    31,
    57,   258,   258,    63,    46,   251,    47,    54,    55,    56,
     0,    79,    80,    11,   290,     0,     0,    12,    16,    66,
     0,     0,   257,    75,     0,     0,     0,   104,   257,    65,
     0,     0,     0,     0,     0,   104,     0,     0,     0,     0,
    84,     0,   294,     0,     0,   322,     0,   301,     0,   136,
     0,     0,     0,   135,     0,     0,     0,     0,     1,     7,
     3,   258,   258,   258,   258,   258,   128,   129,   130,   258,
    21,   258,   258,   258,    41,   258,   131,   132,   133,   258,
   258,   258,   258,   258,   258,     0,     0,   258,   258,   258,
   258,   258,    11,     4,   291,    18,    19,    17,     0,   247,
     0,   189,   191,   190,     0,   236,   237,   257,   257,     0,
     0,     0,   104,   106,     0,     0,    20,     0,     0,     0,
     0,     0,     0,   140,     0,    99,     0,     0,     0,    84,
    46,   191,     0,   303,   292,   320,   321,   299,   323,   304,
   307,   310,   300,   318,   191,   136,     0,    39,    43,    44,
   258,    45,   257,   136,   252,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   151,   151,     0,   125,   115,     0,   115,     6,     0,
   258,     0,   258,   258,   258,   258,   258,   207,   191,     0,
   258,     0,     0,     0,   257,    11,     0,   243,   258,     0,
     0,   105,     0,   107,     0,     0,     0,   258,   258,   258,
     0,   258,   258,   258,   142,     0,    98,   258,     0,     0,
     0,    85,     0,   207,   295,   302,     0,   257,     0,   325,
     0,     0,     0,   298,   319,   207,   137,   258,     0,     0,
   138,     0,    22,    24,    23,    25,    26,    30,    27,    28,
    29,    32,    33,    38,    40,    42,    35,    36,    37,    51,
    50,    49,   151,   154,     0,    46,   251,    55,   154,   106,
   106,     0,   162,   125,   163,   258,   106,     0,   123,   106,
   106,   186,     0,   175,     0,   181,     0,   248,    78,     0,
     0,     0,     0,   258,     0,   207,   192,     0,     0,   235,
   258,   238,    11,    11,    10,   257,     0,    77,   242,   241,
   136,   258,     0,     0,     0,    67,    74,    76,   136,    71,
    69,     0,     0,     0,     0,     0,    88,     0,    89,    87,
     0,    86,   282,   302,     0,   322,   372,   373,   257,   257,
     0,   258,   340,     0,   335,   336,   324,   326,   316,   315,
     0,   313,     0,     0,     0,    81,   134,   258,   257,   258,
     0,   152,   155,     0,     0,     0,     0,   114,   112,     0,
   126,   118,   111,     0,     0,   115,     0,   110,   108,   258,
     0,   258,   258,   174,   258,     0,     0,     0,   258,   257,
   258,     0,   206,   205,   204,   197,   200,   201,   202,   203,
   197,   136,   136,     0,   258,   208,     0,   239,   234,     9,
     8,     0,   244,   245,     0,    62,     0,   258,   258,   168,
   258,    64,   258,     0,   141,   178,   143,   258,   258,   102,
    20,     0,     0,   258,     0,     0,   322,   323,     0,     0,
   330,   340,   340,     0,   258,   370,   348,   258,   258,   347,
   338,   340,   340,   357,   358,   258,   297,   257,   305,   317,
     0,   308,   311,     0,     0,     0,   260,   261,   262,    58,
   139,    59,    34,   258,   151,   258,   106,   125,     0,   119,
   124,   116,   106,     0,     0,   172,   171,   170,   176,   177,
   180,     0,     0,     0,   258,   194,   197,     0,   195,   193,
   196,   219,   220,   216,   224,   215,   223,   214,   222,   258,
     0,   210,     0,   212,   213,   188,   240,   246,     0,     0,
   166,   165,   164,    68,     0,     0,    70,    82,     0,     0,
   258,     0,    93,    96,    91,    83,   286,   283,   284,     0,
   279,   278,   323,   257,   327,   328,   334,   333,   332,     0,
   330,   339,   353,     0,   349,   345,   340,   352,   355,   341,
     0,     0,     0,   302,   258,   342,   346,   258,     0,   337,
   318,   314,   318,   318,   270,   269,     0,     0,     0,   272,
   258,   263,   264,   265,   144,   153,   145,   113,   127,   123,
     0,   121,   109,   187,   173,   184,   182,   185,   169,   198,
   199,     0,     0,   258,   226,     0,   167,     0,   179,   102,
   103,   101,     0,     0,    90,     0,    95,     0,   285,   258,
   281,   280,   257,     0,   257,   331,   258,   302,   350,     0,
   361,   364,   367,   370,   318,   353,   340,   340,   322,   136,
   306,   309,   312,     0,     0,     0,     0,     0,   275,   276,
     0,   272,   259,   115,   120,     0,   221,   209,     0,   161,
   160,   159,   231,   230,     0,     0,   226,   233,   156,   229,
   158,   258,   258,   258,    92,    94,    97,   287,   277,     0,
   293,     0,   322,   351,   302,   257,   257,   257,   322,   344,
   322,   360,     0,     0,     0,   268,   288,   274,   271,   273,
   117,   122,   183,   211,     0,     0,   217,   225,   258,   227,
    61,    60,   100,   296,   329,   354,   356,   374,     0,     0,
     0,   359,   343,   371,   266,   267,   289,   233,   229,   218,
     0,   257,   362,   365,   368,   228,   232,   375,   318,   318,
   318,   322,   322,   322,   363,   366,   369,     0,     0,     0
};

static const short yydefgoto[] = {   808,
    55,    56,    57,    78,    79,   114,    59,   312,    60,   189,
   190,   390,   675,   595,   677,   185,   186,   591,   172,   263,
   264,   337,   338,   539,   436,   331,   132,   133,   134,   143,
   144,   145,   115,   207,   300,   275,   384,    61,    62,    63,
   324,   424,   325,   332,    90,   481,    80,   445,   343,   485,
   344,   345,   346,    64,   163,   248,   249,   556,   557,   457,
   355,   356,   571,   572,   573,   726,   727,   728,   770,    84,
   253,   166,   167,   170,   171,   161,    65,    66,    67,    68,
    69,    70,   403,    81,    72,   526,   527,   528,   639,   529,
   711,   712,    73,   600,   496,   598,   599,   758,    74,    75,
   286,   193,   394,   106,   508,   509,   203,   291,   631,   292,
   633,   293,   634,   411,   412,   521,   294,   295,   199,   288,
   289,   290,   610,   611,   778,   405,   406,   615,   511,   512,
   616,   617,   618,   513,   619,   746,   799,   747,   800,   748,
   801,   515,   516,   779
};

static const short yypact[] = {  1285,
-32768,   102,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,    63,-32768,-32768,-32768,-32768,-32768,  1835,-32768,
-32768,-32768,-32768,-32768,     9,-32768,   924,   312,  1615,   735,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   453,-32768,-32768,   312,    20,    96,   100,-32768,   102,-32768,
  1835,  1835,  1835,-32768,  1835,  1835,  1835,   106,  1835,-32768,
  1835,  1835,  1835,  1835,  1835,   106,  1835,  1835,  1835,   108,
  1835,   108,-32768,    31,   144,-32768,   203,   108,   108,  1835,
  1835,  1835,  1835,   121,    76,  1835,  1835,   108,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,    81,    85,-32768,-32768,-32768,
-32768,-32768,   312,-32768,-32768,-32768,-32768,-32768,    25,   181,
   167,   814,   320,-32768,    28,   186,   257,   252,   262,   269,
   293,   219,   106,   453,   274,   298,  1395,   317,   321,   333,
   273,   347,   376,   397,   105,-32768,   379,   832,   362,  1835,
   325,   320,   363,-32768,-32768,-32768,-32768,-32768,   371,-32768,
-32768,-32768,-32768,   196,   320,  1615,   368,   201,-32768,-32768,
-32768,-32768,   814,  1615,-32768,  1835,  1835,  1835,  1835,  1835,
  1835,  1835,  1835,  1835,  1835,  1835,  1835,  1835,  1835,  1835,
  1835,  1945,  1945,  1835,   401,   313,  1835,   313,-32768,  1835,
-32768,  1835,-32768,-32768,-32768,-32768,-32768,   404,   320,  1835,
-32768,  1835,   435,  1835,  1835,   312,  1835,   408,-32768,  1835,
  1835,-32768,   440,-32768,  1835,  1835,  1835,-32768,-32768,-32768,
  1835,-32768,-32768,-32768,   430,  1835,-32768,-32768,  1835,  2055,
  1835,-32768,  1835,   404,-32768,    87,   318,   416,   431,   371,
    48,    48,    48,-32768,-32768,   404,-32768,-32768,  1835,   374,
  1835,   366,   814,   601,   554,   650,   668,   393,   452,   249,
   249,-32768,   755,   187,-32768,-32768,   434,   187,   187,   373,
   380,   394,  1725,   472,   402,   403,   405,   411,   472,   548,
   453,   419,-32768,   401,-32768,-32768,   453,   437,   486,   674,
   453,   764,   387,    72,   451,   183,  1835,-32768,-32768,   447,
   447,  1835,  1835,-32768,   513,   404,-32768,   491,  1835,-32768,
-32768,-32768,   312,   312,-32768,   493,  1835,-32768,-32768,   293,
  1615,-32768,   557,   524,   525,-32768,-32768,-32768,  1615,-32768,
-32768,  1835,  1835,   532,   533,  1835,   814,  1835,   618,-32768,
   537,   814,   499,    87,   196,-32768,-32768,-32768,   416,   416,
   473,-32768,   436,   546,-32768,   564,-32768,-32768,-32768,-32768,
   483,   512,   487,   484,   292,-32768,-32768,-32768,   814,-32768,
  1835,-32768,-32768,   496,  1835,   501,  1835,-32768,-32768,   108,
-32768,-32768,-32768,   516,   106,   313,  1835,-32768,-32768,-32768,
  1835,-32768,-32768,-32768,-32768,  1835,  1835,  1835,-32768,-32768,
-32768,   387,-32768,-32768,-32768,   447,   492,-32768,-32768,-32768,
   447,  1615,  1615,   707,-32768,-32768,  1835,-32768,-32768,-32768,
-32768,  1835,-32768,-32768,   497,-32768,  1835,-32768,-32768,-32768,
-32768,-32768,-32768,   498,-32768,   583,-32768,-32768,-32768,   727,
  1505,  1835,  1835,-32768,   106,   159,-32768,   371,   579,   581,
    51,   538,   322,  1835,-32768,    31,-32768,-32768,   284,-32768,
-32768,   297,   322,-32768,   196,-32768,-32768,   416,-32768,-32768,
    48,-32768,-32768,   198,   108,   582,   292,   292,   292,-32768,
-32768,-32768,-32768,-32768,  1725,-32768,   453,   401,   514,    61,
-32768,-32768,   453,  1835,   585,-32768,-32768,-32768,-32768,-32768,
-32768,  1835,  1835,  1835,-32768,-32768,   447,  1835,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  1835,   540,   518,-32768,-32768,-32768,-32768,-32768,  1835,   593,
-32768,-32768,-32768,-32768,  1835,  1835,-32768,-32768,  1835,  1835,
-32768,  1835,   794,   956,-32768,-32768,   544,-32768,   106,   625,
   159,   159,   371,   416,-32768,-32768,-32768,-32768,-32768,   553,
    51,-32768,   177,   586,-32768,-32768,   322,-32768,-32768,-32768,
   108,   350,   355,   318,-32768,-32768,-32768,-32768,   555,-32768,
   196,-32768,   196,   196,-32768,-32768,   587,   588,  1835,   176,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   486,
   108,-32768,-32768,-32768,-32768,   185,   617,   584,-32768,-32768,
   814,   383,   640,-32768,   558,   641,-32768,   645,-32768,  1021,
   814,-32768,   573,  1835,-32768,  1835,-32768,  1835,-32768,-32768,
-32768,-32768,   416,   647,   416,-32768,-32768,   318,-32768,   614,
-32768,-32768,-32768,-32768,   196,   196,   322,   322,-32768,  1835,
-32768,-32768,-32768,    70,    70,   663,   108,    70,-32768,-32768,
   664,   176,-32768,   313,   516,  1835,-32768,-32768,   108,-32768,
-32768,-32768,-32768,-32768,   594,    57,   558,   683,-32768,   405,
-32768,-32768,-32768,-32768,-32768,   814,   814,   814,-32768,   690,
-32768,   691,-32768,-32768,   318,   416,   416,   416,-32768,-32768,
-32768,-32768,   621,   695,   696,-32768,   108,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,    68,   633,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   512,   634,   637,
   619,-32768,-32768,-32768,-32768,-32768,-32768,   683,-32768,-32768,
  1835,   416,-32768,-32768,-32768,-32768,   814,-32768,   196,   196,
   196,-32768,-32768,-32768,-32768,-32768,-32768,   751,   753,-32768
};

static const short yypgoto[] = {-32768,
   698,   -13,-32768,   684,-32768,   124,   413,  -370,   426,   574,
-32768,-32768,-32768,   175,-32768,-32768,   590,   109,   -34,  -281,
   701,  -233,-32768,  -533,   128,  -300,-32768,-32768,-32768,-32768,
-32768,-32768,     0,  -178,   361,-32768,-32768,-32768,-32768,-32768,
  -210,   454,  -586,  -428,  -256,-32768,  -299,   330,   -46,   199,
  -347,-32768,  -272,-32768,   697,  -104,-32768,  -408,    56,-32768,
  -193,-32768,-32768,-32768,-32768,    60,-32768,    23,     2,-32768,
-32768,   539,-32768,   -71,   531,   556,   562,    62,  -309,-32768,
  -165,-32768,    65,   -15,-32768,  -205,-32768,  -559,  -267,  -553,
    84,-32768,-32768,  -159,-32768,   204,-32768,    43,   729,  -411,
-32768,-32768,-32768,  -222,   -25,   -22,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,  -249,-32768,    24,  -579,   -88,  -246,  -452,
   515,  -404,   195,-32768,  -268,   291,-32768,  -346,   300,-32768,
  -473,-32768,  -550,-32768,  -345,-32768,-32768,-32768,-32768,-32768,
-32768,   200,   535,  -502
};


#define	YYLAST		2172


static const short yytable[] = {    82,
    83,    85,    86,    87,   341,   540,   652,    88,    89,    91,
    92,    93,    94,    95,    96,    97,   198,   107,    98,   404,
   108,    99,   329,   180,   100,   101,   102,   297,   109,   110,
   111,   112,   113,   431,   486,   302,   116,   117,   118,   627,
   459,   459,   413,   414,   121,   604,   146,   147,   428,   429,
   533,   701,   559,   702,   703,   433,   510,   514,   438,   439,
   154,   181,    12,   396,    71,    12,   328,   328,     6,   335,
     6,     9,    10,   697,   250,   766,   240,    12,   725,   119,
   709,   160,    12,   635,   601,   241,   710,   284,   251,   446,
   393,   602,   178,   179,     6,    43,   182,   183,   184,   156,
   296,    12,   415,   157,   105,   447,   216,   217,   218,   219,
   220,   540,   422,     6,   221,   749,   222,   223,   224,    12,
   225,    71,    12,    58,   226,   227,   228,   229,   230,   231,
   499,   500,   234,   235,   236,   237,   238,   744,   262,   239,
   725,   636,   546,   689,   357,   194,   459,   164,   660,   498,
   683,   459,   709,   164,   575,   612,   409,   328,   710,   607,
   608,   187,   466,   192,   767,   510,   514,   211,   335,   204,
   205,   497,   103,   654,   276,   651,   723,   724,   581,   215,
    58,   762,   131,   212,   460,   460,   547,   232,   369,   601,
   601,   233,   475,   614,   777,   299,   602,   602,   242,   135,
   484,   243,   542,   614,   159,     6,    76,    77,   165,   168,
   169,   635,    12,   135,   175,   176,   252,   707,   169,   802,
   803,   804,   582,   750,   751,   347,   195,   349,   350,   351,
   352,   353,   256,   258,    43,   359,   287,   649,   486,   196,
   197,   160,   365,   368,   780,   781,   187,   459,   708,   525,
   603,   360,   376,   377,   378,   648,   380,   381,   382,  -251,
   107,   653,   386,   108,   373,   449,   375,   449,   335,   636,
   450,   632,   196,   197,   254,   385,   451,   301,   451,   656,
   391,   658,   416,   560,   561,   140,   540,   141,   142,   798,
   460,   196,   197,   327,   327,   460,   334,   339,   255,   339,
   131,   141,   142,   259,     6,   397,   198,   614,   257,   200,
   260,    12,   398,   201,     2,   202,     3,     4,     5,   364,
   432,   642,   643,   644,   646,     6,   397,    12,   336,     6,
   397,   261,    12,   398,   265,   684,    12,   398,   464,    18,
   244,-32768,   130,   503,   266,   469,   452,   395,   400,   470,
   471,   268,   410,   410,   410,   269,   476,   330,   468,   245,
   340,   246,     6,   397,   524,   525,   474,   270,   107,   328,
   398,   108,   335,   358,   335,   247,   504,   165,   363,   624,
   366,   272,   487,   169,   327,   271,   502,   614,   614,   374,
  -301,   460,   196,   197,  -301,   334,  -301,    12,   717,   549,
   550,   504,   530,  -302,   532,   456,   461,  -302,     6,  -302,
   273,     9,    10,   505,   740,    12,   742,   441,   442,   443,
   274,   444,   417,   625,   544,   278,   628,    91,  -302,   548,
   473,   281,  -302,   552,  -302,   554,   754,   755,    54,   283,
   545,   681,   682,     6,   397,   285,   135,   551,   287,   576,
    12,   398,   752,   354,     6,   395,   691,     9,    10,   298,
   692,    12,   693,    91,   507,   583,   577,   584,   367,   361,
   383,   578,   587,   588,   372,   402,   580,   407,   596,   420,
   761,   148,   503,   301,   453,   418,   130,  -161,   149,   621,
   423,   538,   622,   623,  -160,   162,   776,   339,   150,   731,
   629,   162,   782,   620,   783,   177,   151,   435,  -159,   454,
   152,   448,   455,   188,   553,   504,   425,  -156,   645,  -157,
   647,   753,   206,     6,   687,  -158,     9,    10,   213,   214,
   138,   139,   140,   430,   141,   142,   208,   209,   210,   659,
-32768,-32768,  -302,   434,   129,   130,  -302,   465,  -302,   335,
   537,   467,   505,   472,   662,   805,   806,   807,   482,   483,
   543,   731,   609,    54,   613,     6,   488,   489,     9,    10,
   663,   494,    12,   507,   613,   672,   148,   495,   666,   501,
   517,   518,   410,   149,   668,   638,   640,   477,   478,   479,
   519,   480,   520,   150,   427,   720,   327,   523,   522,   334,
   586,   151,   188,   534,   104,   152,   558,   687,   536,   698,
   579,   585,   699,   605,   131,   606,   641,   504,   206,   655,
   721,   650,   664,   722,   665,   713,   206,   667,   303,   304,
   305,   306,   307,   308,   309,   310,   311,   492,   706,   124,
   125,   126,   127,   128,   323,   323,   129,   130,   719,   173,
   313,   314,   315,   316,   317,   318,   319,   173,   678,   680,
   685,   700,   191,   716,   739,   342,   723,   724,   688,   704,
   705,   743,   609,   371,   718,   732,   657,   451,   613,   733,
   735,   741,   690,   379,   123,   696,   124,   125,   126,   127,
   128,   387,   389,   129,   130,   392,   745,   756,   759,   769,
   122,   123,   148,   124,   125,   126,   127,   128,   765,   149,
   129,   130,   715,   419,     6,   562,   771,   772,   773,   150,
   437,    12,   563,   215,   774,   775,   730,   151,   784,   785,
   786,   152,   795,   493,   173,   323,   125,   126,   127,   128,
   790,   793,   129,   130,   564,   565,   589,   135,   794,   696,
   809,   191,   810,   791,   120,   126,   127,   128,   613,   613,
   129,   130,   158,   282,   462,   463,   673,   135,   757,   566,
   567,   153,   568,   569,   277,   339,   135,   714,   734,   531,
   764,   555,   426,   206,   669,   174,   768,   788,   730,   796,
   370,   206,   362,   326,   326,   760,   333,   348,   490,   787,
   491,   792,   679,   155,   408,   686,   696,   342,   630,   122,
   123,   626,   124,   125,   126,   127,   128,     0,   757,   129,
   130,   401,   695,   570,     0,     0,   789,     0,     0,   136,
   137,   138,   139,   140,     0,   141,   142,   535,     0,   763,
     0,     0,   590,     0,     0,     0,   313,   104,   399,   421,
   137,   138,   139,   140,     0,   141,   142,     0,   440,   137,
   138,   139,   140,     0,   141,   142,     0,     0,     0,     0,
     0,   342,   342,   279,   206,   206,   122,   123,   280,   124,
   125,   126,   127,   128,   326,     0,   129,   130,     0,     0,
     0,     0,     0,     0,     0,   333,   122,   123,     0,   124,
   125,   126,   127,   128,   593,   594,   129,   130,     0,   674,
     0,   458,   458,     0,   122,   123,     0,   124,   125,   126,
   127,   128,     0,     0,   129,   130,     2,     0,     3,     4,
     5,     6,     7,     8,     9,    10,     0,    11,    12,    13,
     0,     0,     0,     0,     0,     0,    14,   323,    15,    16,
    17,    18,     0,    19,     0,   104,     0,     0,     0,     0,
    20,    21,    22,     0,   506,     0,    23,    24,    25,   313,
   661,    26,    27,     0,    28,     0,    29,   342,    30,   342,
    31,     0,    32,    33,     0,    34,    35,    36,    37,    38,
    39,    40,    41,     0,   -11,   333,   541,     0,    42,    43,
    44,   670,   671,     0,   594,     0,     0,     0,     0,     0,
     0,   342,     0,     0,    45,     0,     0,   458,     0,     0,
     0,     0,   458,    46,     0,   574,    47,    48,     0,     0,
    49,     0,    50,    51,    52,     0,    53,     0,   122,   123,
    54,   124,   125,   126,   127,   128,     0,     0,   129,   130,
     0,     0,     0,     0,     0,     0,   597,     0,     0,     0,
     0,     0,     0,     0,   506,     0,     0,     0,     0,     0,
     0,   676,     0,   506,   506,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   637,   736,     0,   737,     0,
   738,     0,     0,     0,     0,     0,   326,     0,     0,   333,
     0,   333,     0,   122,   123,     0,   124,   125,   126,   127,
   128,     0,   206,   129,   130,     0,     0,     0,   458,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   590,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
   597,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   506,     0,
     0,     0,     0,     0,   694,   506,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   797,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   729,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   506,
     0,     0,     0,     0,     0,     0,     0,     0,   506,   506,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   333,     0,     0,     0,
     0,     0,     0,     0,     0,     1,     0,     2,   729,     3,
     4,     5,     6,     7,     8,     9,    10,     0,    11,    12,
    13,     0,     0,     0,     0,     0,   506,    14,     0,    15,
    16,    17,    18,  -257,    19,     0,     0,     0,     0,     0,
  -257,    20,    21,    22,     0,     0,     0,    23,    24,    25,
  -257,     0,    26,    27,     0,    28,     0,    29,  -257,    30,
     0,    31,  -257,    32,    33,     0,    34,    35,    36,    37,
    38,    39,    40,    41,     0,   -11,     0,     0,     0,    42,
    43,    44,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    45,     0,     0,     0,     0,
     0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
     0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
     0,    54,     6,     7,     8,     9,    10,     0,    11,    12,
    13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,   267,    34,    35,    36,    37,
    38,    39,    40,    41,     0,     0,     0,     0,     0,    42,
     0,    44,     0,     0,     0,     0,     0,   122,   123,     0,
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
     0,    44,     0,     0,     0,     0,     0,   122,   123,     0,
   124,   125,   126,   127,   128,    45,     0,   129,   130,     0,
     0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
     0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
   592,    54,     6,     7,     8,     9,    10,     0,    11,    12,
    13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,    21,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,     0,    34,    35,    36,    37,
    38,    39,    40,    41,     0,     0,     0,     0,     0,    42,
     0,    44,     0,     0,     0,     0,     0,   122,   123,     0,
   124,   125,   126,   127,   128,    45,     0,   129,   130,     0,
     0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
     0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
     0,    54,     6,     7,     8,     9,    10,     0,    11,    12,
    13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,   320,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,     0,    34,   321,    36,    37,
   322,    39,    40,    41,     0,     0,     0,     0,     0,    42,
     0,    44,     0,     0,     0,     0,     0,   122,   123,     0,
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
     0,    44,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    45,     0,     0,     0,     0,
     0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
     0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
     0,    54,     6,     7,     8,     9,    10,     0,    11,    12,
    13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,   320,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,     0,    34,   321,    36,    37,
   322,    39,    40,    41,     0,     0,     0,     0,     0,    42,
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
     0,   388,     0,    50,    51,    52,     0,    53,     0,     0,
     0,    54
};

static const short yycheck[] = {    15,
    16,    17,    18,    19,   238,   434,   540,    23,    24,    25,
    26,    27,    28,    29,    30,    31,   105,    43,    34,   288,
    43,    37,   233,    95,    40,    41,    42,   206,    44,    45,
    46,    47,    48,   334,   382,   214,    52,    53,    54,   513,
   350,   351,   292,   293,    58,   498,    62,    63,   330,   331,
   421,   631,   461,   633,   634,   337,   403,   403,   340,   341,
    74,    96,    15,   286,     0,    15,   232,   233,     8,   235,
     8,    11,    12,   624,    47,    19,    52,    15,   665,    71,
   640,    82,    15,    14,   496,    61,   640,   192,    61,    18,
   284,   496,    93,    94,     8,    76,    97,    98,    99,     4,
   205,    15,   296,     4,    43,    34,   122,   123,   124,   125,
   126,   540,   323,     8,   130,   695,   132,   133,   134,    15,
   136,    57,    15,     0,   140,   141,   142,   143,   144,   145,
   399,   400,   148,   149,   150,   151,   152,   688,   173,   153,
   727,    72,   442,   617,   249,   115,   456,    83,   557,   396,
   603,   461,   712,    89,   464,   502,   109,   323,   712,   109,
   110,   100,   356,   102,   108,   512,   512,    47,   334,   108,
   109,   394,   110,   544,    70,   115,   109,   110,   478,   118,
    57,   715,    59,   108,   350,   351,   443,   107,   260,   601,
   602,   107,   371,   503,   745,   211,   601,   602,    18,    13,
   379,    35,   436,   513,    81,     8,   105,   106,    85,    86,
    87,    14,    15,    13,    91,    92,    31,    42,    95,   799,
   800,   801,   479,   697,   698,   241,    83,   243,   244,   245,
   246,   247,   168,   169,    76,   251,    78,   538,   586,    96,
    97,   242,   256,   259,   747,   748,   185,   557,    73,    74,
   497,   252,   268,   269,   270,   537,   272,   273,   274,    83,
   286,   543,   278,   286,   265,    83,   267,    83,   434,    72,
    88,   521,    96,    97,    18,   276,    94,   213,    94,   552,
   281,   554,   298,   462,   463,    99,   715,   101,   102,   792,
   456,    96,    97,   232,   233,   461,   235,   236,    47,   238,
   177,   101,   102,    35,     8,     9,   395,   617,    47,   107,
    18,    15,    16,   111,     3,   113,     5,     6,     7,   255,
   336,   527,   528,   529,   535,     8,     9,    15,    16,     8,
     9,   113,    15,    16,    61,   604,    15,    16,   354,    28,
    21,    93,    94,    47,    47,   361,   347,   286,   287,   363,
   364,    35,   291,   292,   293,    35,   372,   234,   359,    40,
   237,    42,     8,     9,    73,    74,   367,    35,   394,   535,
    16,   394,   538,   250,   540,    56,    80,   254,   255,    83,
   257,    35,   383,   260,   323,   113,   402,   697,   698,   266,
   107,   557,    96,    97,   111,   334,   113,    15,    16,   446,
   447,    80,   418,   107,   420,   350,   351,   111,     8,   113,
    35,    11,    12,   117,   683,    15,   685,    31,    32,    33,
    24,    35,   299,   512,   440,    47,   515,   443,   107,   445,
   366,    70,   111,   449,   113,   451,   704,   705,   117,   115,
   441,   601,   602,     8,     9,    83,    13,   448,    78,   465,
    15,    16,   699,    50,     8,   394,   107,    11,    12,    92,
   111,    15,   113,   479,   403,   481,   467,   483,    61,    35,
    41,   472,   488,   489,    35,    60,   477,    47,   494,   114,
   714,    29,    47,   419,    38,   112,    94,   115,    36,   505,
    19,   430,   508,   509,   115,    83,   743,   436,    46,   665,
   516,    89,   749,   504,   751,    93,    54,    22,   115,    63,
    58,    61,    66,   101,   450,    80,   115,   115,   534,   115,
   536,   700,   110,     8,   613,   115,    11,    12,   116,   117,
    97,    98,    99,   115,   101,   102,   111,   112,   113,   555,
    89,    90,   107,   107,    93,    94,   111,    35,   113,   715,
   427,    61,   117,    61,   570,   802,   803,   804,    35,    35,
   437,   727,   501,   117,   503,     8,    35,    35,    11,    12,
   571,    35,    15,   512,   513,   591,    29,    79,   579,   107,
    35,    18,   521,    36,   585,   524,   525,    31,    32,    33,
   108,    35,    81,    46,    47,    38,   535,   114,   112,   538,
    18,    54,   190,   108,    43,    58,   115,   696,   108,   625,
   114,   114,   628,    35,   491,    35,    35,    80,   206,    35,
    63,   108,    83,    66,   107,   641,   214,    35,   216,   217,
   218,   219,   220,   221,   222,   223,   224,    20,   639,    86,
    87,    88,    89,    90,   232,   233,    93,    94,   664,    88,
   225,   226,   227,   228,   229,   230,   231,    96,   115,    35,
   108,   107,   101,    47,   680,   240,   109,   110,    83,    83,
    83,   687,   611,   261,    35,    35,   553,    94,   617,    35,
   108,    35,   621,   271,    84,   624,    86,    87,    88,    89,
    90,   279,   280,    93,    94,   283,    83,    35,    35,    17,
    83,    84,    29,    86,    87,    88,    89,    90,   115,    36,
    93,    94,   651,   301,     8,     9,   732,   733,   734,    46,
    47,    15,    16,   662,    35,    35,   665,    54,   108,    35,
    35,    58,   114,   116,   173,   323,    87,    88,    89,    90,
   108,   108,    93,    94,    38,    39,    20,    13,   112,   688,
     0,   190,     0,   769,    57,    88,    89,    90,   697,   698,
    93,    94,    79,   190,   352,   353,   592,    13,   707,    63,
    64,    71,    66,    67,   185,   714,    13,   650,   670,   419,
   719,   452,   329,   371,   586,    89,   727,   765,   727,   788,
   260,   379,   254,   232,   233,   712,   235,   242,   386,   757,
   388,   778,   599,    75,   290,   611,   745,   382,   518,    83,
    84,   512,    86,    87,    88,    89,    90,    -1,   757,    93,
    94,   287,   623,   117,    -1,    -1,   765,    -1,    -1,    95,
    96,    97,    98,    99,    -1,   101,   102,   425,    -1,   716,
    -1,    -1,   116,    -1,    -1,    -1,   421,   286,   287,    95,
    96,    97,    98,    99,    -1,   101,   102,    -1,    95,    96,
    97,    98,    99,    -1,   101,   102,    -1,    -1,    -1,    -1,
    -1,   446,   447,    42,   462,   463,    83,    84,    47,    86,
    87,    88,    89,    90,   323,    -1,    93,    94,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,   334,    83,    84,    -1,    86,
    87,    88,    89,    90,   492,   493,    93,    94,    -1,   116,
    -1,   350,   351,    -1,    83,    84,    -1,    86,    87,    88,
    89,    90,    -1,    -1,    93,    94,     3,    -1,     5,     6,
     7,     8,     9,    10,    11,    12,    -1,    14,    15,    16,
    -1,    -1,    -1,    -1,    -1,    -1,    23,   535,    25,    26,
    27,    28,    -1,    30,    -1,   394,    -1,    -1,    -1,    -1,
    37,    38,    39,    -1,   403,    -1,    43,    44,    45,   544,
   558,    48,    49,    -1,    51,    -1,    53,   552,    55,   554,
    57,    -1,    59,    60,    -1,    62,    63,    64,    65,    66,
    67,    68,    69,    -1,    71,   434,   435,    -1,    75,    76,
    77,   589,   590,    -1,   592,    -1,    -1,    -1,    -1,    -1,
    -1,   586,    -1,    -1,    91,    -1,    -1,   456,    -1,    -1,
    -1,    -1,   461,   100,    -1,   464,   103,   104,    -1,    -1,
   107,    -1,   109,   110,   111,    -1,   113,    -1,    83,    84,
   117,    86,    87,    88,    89,    90,    -1,    -1,    93,    94,
    -1,    -1,    -1,    -1,    -1,    -1,   495,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,   503,    -1,    -1,    -1,    -1,    -1,
    -1,   116,    -1,   512,   513,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,   524,   674,    -1,   676,    -1,
   678,    -1,    -1,    -1,    -1,    -1,   535,    -1,    -1,   538,
    -1,   540,    -1,    83,    84,    -1,    86,    87,    88,    89,
    90,    -1,   700,    93,    94,    -1,    -1,    -1,   557,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,   116,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
   599,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   617,    -1,
    -1,    -1,    -1,    -1,   623,   624,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,   791,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,   665,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   688,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   697,   698,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,   715,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,     1,    -1,     3,   727,     5,
     6,     7,     8,     9,    10,    11,    12,    -1,    14,    15,
    16,    -1,    -1,    -1,    -1,    -1,   745,    23,    -1,    25,
    26,    27,    28,    29,    30,    -1,    -1,    -1,    -1,    -1,
    36,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
    46,    -1,    48,    49,    -1,    51,    -1,    53,    54,    55,
    -1,    57,    58,    59,    60,    -1,    62,    63,    64,    65,
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
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 4:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fSynTopLevelProductionTemplates,
					   yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 5:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 6:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fFunctor,newCTerm(PA_fDollar,yyvsp[-2].t),
					   yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 7:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 8:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fDeclare,yyvsp[-3].t,newCTerm(PA_fSkip,yyvsp[-1].t),
					   yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 9:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fDeclare,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 10:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fDeclare,yyvsp[-2].t,
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
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
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
{ if (oz_isSRecord(yyvsp[-3].t) && 
                        oz_eq(OZ_label(yyvsp[-3].t), PA_fOpApply) && 
                        oz_eq(OZ_getArg(yyvsp[-3].t,0), AtomDot)) {
                       yyval.t = newCTerm(PA_fDotAssign,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t);
			}
                    else
                       yyval.t = newCTerm(PA_fColonEquals,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
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
				  oz_mklistUnwrap(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
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
				  oz_consUnwrap(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 33:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 34:
{ yyval.t = oz_consUnwrap(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 35:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
				  oz_mklistUnwrap(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 36:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
				  oz_mklistUnwrap(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 37:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
				  oz_mklistUnwrap(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 38:
{ yyval.t = newCTerm(PA_fObjApply,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 39:
{ yyval.t = newCTerm(PA_fOpApply,AtomTilde,
				  oz_mklistUnwrap(yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 40:
{ yyval.t = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklistUnwrap(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 41:
{ yyval.t = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklistUnwrap(yyvsp[-1].t,makeInt(xytext,pos())),pos()); ;
    break;}
case 42:
{ yyval.t = newCTerm(PA_fOpApply,AtomHat,
				  oz_mklistUnwrap(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 43:
{ yyval.t = newCTerm(PA_fAt,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 44:
{ yyval.t = newCTerm(PA_fOpApply,AtomDExcl,
				  oz_mklistUnwrap(yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 45:
{ yyval.t = newCTerm(PA_parens,yyvsp[-1].t); ;
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
				  oz_mklistUnwrap(yyvsp[-3].t,yyvsp[-2].t)); ;
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
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 86:
{ yyval.t = newCTerm(oz_atom("forFeature"),yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 87:
{ yyval.t = newCTerm(oz_atom("forPattern"),yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 88:
{ yyval.t = newCTerm(oz_atom("forFrom"),yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 89:
{ yyval.t = newCTerm(oz_atom("forGeneratorList"),yyvsp[0].t); ;
    break;}
case 90:
{ yyval.t = newCTerm(oz_atom("forGeneratorInt"),yyvsp[-3].t,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 91:
{ yyval.t = newCTerm(oz_atom("forGeneratorC"),yyvsp[-2].t,oz_headUnwrap(yyvsp[0].t),
                                                              oz_tailUnwrap(yyvsp[0].t)); ;
    break;}
case 92:
{ yyval.t = newCTerm(oz_atom("forGeneratorC"),yyvsp[-3].t,oz_headUnwrap(yyvsp[-1].t),
                                                              oz_tailUnwrap(yyvsp[-1].t)); ;
    break;}
case 93:
{ yyval.t = NameUnit; ;
    break;}
case 94:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 95:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 96:
{ yyval.t = NameUnit; ;
    break;}
case 97:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 98:
{
		    yyval.t = newCTerm(PA_fAnd,yyvsp[-1].t,yyvsp[0].t);
		  ;
    break;}
case 100:
{
		    yyval.t = newCTerm(PA_fMacro,
				  oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					  unwrap(yyvsp[-7].t),
					  newCTerm(PA_fAtom,oz_atom("from"),NameUnit),
					  unwrap(yyvsp[-4].t),
					  newCTerm(PA_fAtom,oz_atom("to"),NameUnit),
					  unwrap(yyvsp[-2].t),
					  newCTerm(PA_fAtom,oz_atom("by"),NameUnit),
					  (unwrap(yyvsp[-1].t) == 0)?makeInt("1",NameUnit):unwrap(yyvsp[-1].t),
					  0),
				  makeLongPos(OZ_subtree(yyvsp[-7].t,makeTaggedSmallInt(2)),yyvsp[0].t));
		  ;
    break;}
case 101:
{
		    /* <<for X 'in' L>>
		       <<for X = E1 'then' E2>> */
		    if (yyvsp[-1].t == 0) {
		      yyval.t = newCTerm(PA_fMacro,
				    oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					    unwrap(yyvsp[-5].t),
					    newCTerm(PA_fAtom,oz_atom("in"),NameUnit),
					    unwrap(yyvsp[-2].t),
					    0),
				    makeLongPos(OZ_subtree(yyvsp[-5].t,makeTaggedSmallInt(2)),yyvsp[0].t));
		    } else {
		      yyval.t = newCTerm(PA_fMacro,
				    oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					    newCTerm(PA_fEq,yyvsp[-5].t,yyvsp[-2].t,NameUnit),
					    newCTerm(PA_fAtom,oz_atom("next"),NameUnit),
					    unwrap(yyvsp[-1].t),
					    0),
				    makeLongPos(OZ_subtree(yyvsp[-5].t,makeTaggedSmallInt(2)),yyvsp[0].t));
		    }
		  ;
    break;}
case 102:
{ yyval.t = 0; ;
    break;}
case 103:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 104:
{ yyval.t = AtomNil; ;
    break;}
case 105:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 106:
{ yyval.t = AtomNil; ;
    break;}
case 107:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 108:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fRequire,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 109:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fPrepare,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 110:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fPrepare,yyvsp[-1].t,
					   newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 111:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fImport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 112:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fExport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 113:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fDefine,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 114:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fDefine,yyvsp[-1].t,
					   newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 115:
{ yyval.t = AtomNil; ;
    break;}
case 116:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fImportItem,yyvsp[-2].t,AtomNil,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 117:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fImportItem,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 118:
{ yyval.t = newCTerm(PA_fVar,OZ_atom(xytext),yyvsp[0].t); ;
    break;}
case 119:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 120:
{ yyval.t = oz_mklistUnwrap(oz_pair2Unwrap(yyvsp[0].t,yyvsp[-2].t)); ;
    break;}
case 121:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 122:
{ yyval.t = oz_consUnwrap(oz_pair2Unwrap(yyvsp[-1].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 123:
{ yyval.t = PA_fNoImportAt; ;
    break;}
case 124:
{ yyval.t = newCTerm(PA_fImportAt,yyvsp[0].t); ;
    break;}
case 125:
{ yyval.t = AtomNil; ;
    break;}
case 126:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fExportItem,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 127:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fExportItem,
					   newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t)),yyvsp[0].t); ;
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
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 134:
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 135:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 136:
{ yyval.t = AtomNil; ;
    break;}
case 137:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 138:
{ yyval.t = newCTerm(PA_fAtom,AtomNil,yyvsp[0].t); ;
    break;}
case 139:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomCons,yyvsp[-2].t),
				  oz_mklistUnwrap(yyvsp[-1].t,yyvsp[0].t)); ;
    break;}
case 140:
{ yyval.t = PA_fNoCatch; ;
    break;}
case 141:
{ yyval.t = newCTerm(PA_fCatch,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 142:
{ yyval.t = PA_fNoFinally; ;
    break;}
case 143:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 144:
{
		    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
				  newCTerm(PA_fAtom,yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
		  ;
    break;}
case 145:
{
		    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
				  makeVar(yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
		  ;
    break;}
case 146:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 147:
{ yyval.t = NameUnit; ;
    break;}
case 148:
{ yyval.t = NameTrue; ;
    break;}
case 149:
{ yyval.t = NameFalse; ;
    break;}
case 150:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 151:
{ yyval.t = AtomNil; ;
    break;}
case 152:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 153:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 154:
{ yyval.t = NameFalse; ;
    break;}
case 155:
{ yyval.t = NameTrue; ;
    break;}
case 156:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 157:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 158:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 159:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 160:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 161:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 162:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 163:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 164:
{ yyval.t = newCTerm(PA_fBoolCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 165:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 166:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 167:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 168:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 169:
{ checkDeprecation(yyvsp[-3].t);
		    yyval.t = newCTerm(PA_fBoolCase,yyvsp[-5].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-6].t,yyvsp[0].t));
		  ;
    break;}
case 170:
{ yyval.t = newCTerm(PA_fCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 171:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 172:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 173:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 174:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 175:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 176:
{ yyval.t = oz_consUnwrap(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 177:
{ yyval.t = oz_consUnwrap(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 178:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 179:
{ yyval.t = oz_consUnwrap(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 180:
{ yyval.t = newCTerm(PA_fCaseClause,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 181:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 182:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-3].t,
				  newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 183:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-5].t,yyvsp[-2].t,yyvsp[0].t,yyvsp[-3].t); ;
    break;}
case 184:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 185:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 186:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 187:
{ yyval.t = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
				  oz_consUnwrap(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 188:
{ yyval.t = newCTerm(PA_fClass,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 189:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 190:
{ yyval.t = newCTerm(PA_fDollar,yyvsp[0].t); ;
    break;}
case 191:
{ yyval.t = AtomNil; ;
    break;}
case 192:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 193:
{ yyval.t = newCTerm(PA_fFrom,oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 194:
{ yyval.t = newCTerm(PA_fAttr,oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 195:
{ yyval.t = newCTerm(PA_fFeat,oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 196:
{ yyval.t = newCTerm(PA_fProp,oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 197:
{ yyval.t = AtomNil; ;
    break;}
case 198:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 199:
{ yyval.t = oz_pair2Unwrap(yyvsp[-2].t,yyvsp[0].t); ;
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
{ yyval.t = yyvsp[0].t; ;
    break;}
case 204:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 205:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 206:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 207:
{ yyval.t = AtomNil; ;
    break;}
case 208:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 209:
{ yyval.t = newCTerm(PA_fMeth,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-3].t); ;
    break;}
case 210:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 211:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 212:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 213:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 214:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 215:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 216:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 217:
{ yyval.t = newCTerm(PA_fRecord,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 218:
{ yyval.t = newCTerm(PA_fOpenRecord,yyvsp[-4].t,yyvsp[-2].t); ;
    break;}
case 219:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 220:
{ yyval.t = makeVar(xytext); ;
    break;}
case 221:
{ yyval.t = newCTerm(PA_fEscape,makeVar(xytext),yyvsp[-1].t); ;
    break;}
case 222:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 223:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 224:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 225:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 226:
{ yyval.t = AtomNil; ;
    break;}
case 227:
{ yyval.t = newCTerm(PA_fMethArg,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 228:
{ yyval.t = newCTerm(PA_fMethColonArg,yyvsp[-3].t,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 229:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 230:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 231:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 232:
{ yyval.t = newCTerm(PA_fDefault,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 233:
{ yyval.t = PA_fNoDefault; ;
    break;}
case 234:
{ yyval.t = newCTerm(PA_fCond,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 235:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 236:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 237:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 238:
{ yyval.t = oz_consUnwrap(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 239:
{ yyval.t = newCTerm(PA_fClause,newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 240:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 241:
{ yyval.t = oz_mklistUnwrap(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 242:
{ yyval.t = oz_consUnwrap(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 243:
{ yyval.t = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,yyvsp[0].t),
				  yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 244:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-3].t,yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 245:
{ yyval.t = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 246:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 247:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 248:
{ yyval.t = oz_consUnwrap(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 249:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 250:
{ yyval.t = makeVar(xytext); ;
    break;}
case 251:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 252:
{ yyval.t = newCTerm(PA_fEscape,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 253:
{ yyval.t = makeString(xytext,pos()); ;
    break;}
case 254:
{ yyval.t = makeInt(xytext,pos()); ;
    break;}
case 255:
{ yyval.t = makeInt(xytext[0],pos()); ;
    break;}
case 256:
{ yyval.t = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); ;
    break;}
case 257:
{ yyval.t = pos(); ;
    break;}
case 258:
{ yyval.t = pos(); ;
    break;}
case 259:
{ OZ_Term prefix =
		      scannerPrefix? scannerPrefix: PA_zy;
		    yyval.t = newCTerm(PA_fScanner,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,prefix,
				  makeLongPos(yyvsp[-6].t,yyvsp[0].t)); ;
    break;}
case 260:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 261:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 262:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 263:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 264:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 265:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 266:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 267:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 268:
{ yyval.t = newCTerm(PA_fLexicalRule,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 269:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 270:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 271:
{ yyval.t = newCTerm(PA_fMode,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 272:
{ yyval.t = AtomNil; ;
    break;}
case 273:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 274:
{ yyval.t = newCTerm(PA_fInheritedModes,yyvsp[0].t); ;
    break;}
case 275:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 276:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 277:
{ OZ_Term expect = parserExpect? parserExpect: makeTaggedSmallInt(0);
		    yyval.t = newCTerm(PA_fParser,yyvsp[-6].t,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,expect,
				  makeLongPos(yyvsp[-7].t,yyvsp[0].t)); ;
    break;}
case 278:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 279:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 280:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 281:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 282:
{ yyval.t = newCTerm(PA_fToken,AtomNil); ;
    break;}
case 283:
{ yyval.t = newCTerm(PA_fToken,yyvsp[0].t); ;
    break;}
case 284:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 285:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 286:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 287:
{ yyval.t = oz_pair2Unwrap(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 288:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 289:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 290:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 291:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 292:
{ *prodKey[depth]++ = '='; ;
    break;}
case 293:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 294:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 295:
{ *prodKey[depth]++ = '='; ;
    break;}
case 296:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 297:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,PA_none); ;
    break;}
case 298:
{ yyval.t = oz_mklistUnwrap(yyvsp[-1].t); ;
    break;}
case 299:
{ yyval.t = oz_mklistUnwrap(yyvsp[-1].t); ;
    break;}
case 300:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 303:
{ prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg(yyvsp[-1].t,0))); ;
    break;}
case 304:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 305:
{ depth--; ;
    break;}
case 306:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 307:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 308:
{ depth--; ;
    break;}
case 309:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 310:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 311:
{ depth--; ;
    break;}
case 312:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 313:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 314:
{ yyval.t = oz_consUnwrap(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 315:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 316:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 317:
{ *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; ;
    break;}
case 320:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 321:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 322:
{ *prodKey[depth] = '\0';
		    yyval.t = oz_pair2Unwrap(prodName[depth],OZ_string(prodKeyBuffer[depth]));
		    prodName[depth] = PA_none;
		    prodKey[depth] = prodKeyBuffer[depth];
		  ;
    break;}
case 323:
{ yyval.t = AtomNil; ;
    break;}
case 324:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 325:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 326:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 327:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 328:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 329:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 330:
{ yyval.t = AtomNil; ;
    break;}
case 331:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 332:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 333:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 334:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 335:
{ yyval.t = newCTerm(PA_fSynAlternative, yyvsp[0].t); ;
    break;}
case 336:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 337:
{ yyval.t = oz_consUnwrap(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 338:
{ OZ_Term t = yyvsp[0].t;
		    while (terms[depth]) {
		      t = oz_consUnwrap(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    yyval.t = newCTerm(PA_fSynSequence, decls[depth], t, yyvsp[-1].t);
		    decls[depth] = AtomNil;
		  ;
    break;}
case 339:
{ yyval.t = newCTerm(PA_fSynSequence, AtomNil, yyvsp[0].t, yyvsp[-1].t); ;
    break;}
case 340:
{ yyval.t = AtomNil; ;
    break;}
case 341:
{ yyval.t = oz_mklistUnwrap(newCTerm(PA_fSynAction,yyvsp[0].t)); ;
    break;}
case 342:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 343:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fSynTemplateInstantiation, yyvsp[0].t,
					   oz_consUnwrap(newCTerm(PA_fSynApplication,
							     terms[depth]->term,
							     AtomNil),
						    AtomNil),
					   yyvsp[-2].t),
				  yyvsp[-1].t);
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  ;
    break;}
case 344:
{ yyval.t = oz_consUnwrap(newCTerm(PA_fSynAssignment, terms[depth]->term, yyvsp[-1].t),
				  yyvsp[0].t);
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  ;
    break;}
case 345:
{ while (terms[depth]) {
		      decls[depth] = oz_consUnwrap(terms[depth]->term, decls[depth]);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    yyval.t = yyvsp[0].t;
		  ;
    break;}
case 346:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 347:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 348:
{ terms[depth] = new TermNode(yyvsp[0].t, terms[depth]); ;
    break;}
case 349:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 350:
{ yyval.t = oz_consUnwrap(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 351:
{ yyval.t = newCTerm(PA_fSynAssignment,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 352:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 353:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 354:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_consUnwrap(newCTerm(PA_fSynApplication,yyvsp[-3].t,
						    AtomNil),
					   AtomNil),yyvsp[-1].t);
		  ;
    break;}
case 355:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 356:
{ yyval.t = newCTerm(PA_fSynAssignment,
				  newCTerm(PA_fEscape,yyvsp[-2].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 357:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 358:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 359:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_mklistUnwrap(yyvsp[-2].t),yyvsp[-3].t);
		  ;
    break;}
case 360:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
				  oz_mklistUnwrap(yyvsp[-3].t),yyvsp[-1].t);
		  ;
    break;}
case 361:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 362:
{ depth--; ;
    break;}
case 363:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 364:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 365:
{ depth--; ;
    break;}
case 366:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 367:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 368:
{ depth--; ;
    break;}
case 369:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 370:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 371:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[-4].t,yyvsp[-1].t); ;
    break;}
case 372:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 373:
{ yyval.t = makeVar(xytext); ;
    break;}
case 374:
{ yyval.t = oz_mklistUnwrap(yyvsp[0].t); ;
    break;}
case 375:
{ yyval.t = oz_consUnwrap(yyvsp[-2].t,yyvsp[0].t); ;
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
