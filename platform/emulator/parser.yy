/*
 *  Authors:
 *    Martin Henz <henz@iscs.nus.sg>
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Martin Henz and Leif Kornstaedt, 1996-1999
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation of Oz 3:
 *    http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *    http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

%{
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

%}

%union {
  OZ_Term t;
  int i;
}

%token T_SWITCH T_SWITCHNAME T_LOCALSWITCHES T_PUSHSWITCHES T_POPSWITCHES
%token T_OZATOM T_ATOM_LABEL T_OZFLOAT T_OZINT T_AMPER T_DOTINT T_STRING
%token T_VARIABLE T_VARIABLE_LABEL
%token T_DEFAULT T_CHOICE T_LDOTS T_2DOTS
%token T_attr T_at T_case T_catch T_choice T_class T_cond T_declare T_define
%token T_dis T_else T_elsecase T_elseif T_elseof T_end T_export T_fail 
%token T_false T_FALSE_LABEL T_feat T_finally T_from T_fun T_functor 
%token T_if T_import T_in T_local T_lock T_meth T_not T_of T_or 
%token T_prepare T_proc T_prop T_raise T_require T_self T_skip T_then
%token T_thread T_true T_TRUE_LABEL T_try T_unit T_UNIT_LABEL T_for T_FOR T_do

%token T_ENDOFFILE

%token T_REGEX T_lex T_mode T_parser T_prod T_scanner T_syn T_token
%token T_REDUCE T_SEP

%nonassoc T_ITER
%right    '='
%right    T_OOASSIGN
%nonassoc T_DOTASSIGN
%right    T_orelse
%right    T_andthen
%nonassoc T_COMPARE T_FDCOMPARE T_LMACRO T_RMACRO
%nonassoc T_FDIN
%right    '|'
%right    '#'
%left     T_ADD
%left     T_FDMUL T_OTHERMUL
%right    ','
%left     '~'
%left     '.' '^' T_DOTINT
%left     '@' T_DEREFF

%type <t>  file
%type <t>  queries
%type <t>  queries1
%type <t>  directive
%type <t>  switchList
%type <t>  switch
%type <t>  sequence
%type <t>  phrase
%type <t>  hashes
%type <t>  phrase2
%type <t>  procFlags
%type <t>  optFunctorDescriptorList
%type <t>  functorDescriptorList
%type <t>  importDecls
%type <t>  variableLabel
%type <t>  featureList
%type <t>  optImportAt
%type <t>  exportDecls
%type <t>  compare
%type <t>  fdCompare
%type <t>  fdIn
%type <t>  add
%type <t>  fdMul
%type <t>  otherMul
%type <t>  inSequence
%type <t>  phraseList
%type <t>  fixedListArgs
%type <t>  optCatch
%type <t>  optFinally
%type <t>  record
%type <t>  recordAtomLabel
%type <t>  recordVarLabel
%type <t>  recordArguments
%type <t>  optDots
%type <t>  feature
%type <t>  featureNoVar
%type <t>  ifMain
%type <t>  ifRest
%type <t>  caseMain
%type <t>  caseRest
%type <t>  elseOfList
%type <t>  caseClauseList
%type <t>  caseClause
%type <t>  sideCondition
%type <t>  pattern
%type <t>  class
%type <t>  phraseOpt
%type <t>  classDescriptorList
%type <t>  classDescriptor
%type <t>  attrFeatList
%type <t>  attrFeat
%type <t>  attrFeatFeature
%type <t>  methList
%type <t>  meth
%type <t>  methHead
%type <t>  methHead1
%type <t>  methHeadLabel
%type <t>  methFormals
%type <t>  methFormal
%type <t>  methFormalTerm
%type <t>  methFormalOptDefault
%type <t>  condMain
%type <t>  condElse
%type <t>  condClauseList
%type <t>  condClause
%type <t>  orClauseList
%type <t>  orClause
%type <t>  choiceClauseList
%type <t>  atom
%type <t>  nakedVariable
%type <t>  variable
%type <t>  string
%type <t>  int
%type <t>  float
%type <t>  thisCoord
%type <t>  coord

%type <t>  scannerSpecification
%type <t>  scannerRules
%type <t>  lexAbbrev
%type <t>  lexRule
%type <t>  regex
%type <t>  modeClause
%type <t>  modeDescrs
%type <t>  modeDescr
%type <t>  modeFromList

%type <t>  parserSpecification
%type <t>  parserRules
%type <t>  tokenClause
%type <t>  tokenList
%type <t>  tokenDecl
%type <t>  prodClauseList
%type <t>  prodClause
%type <t>  prodHeadRest
%type <t>  prodKey
%type <t>  prodParams
%type <t>  prodParam
%type <t>  prodMakeKey
%type <t>  localRules
%type <t>  localRulesSub
%type <t>  synClause
%type <t>  synParams
%type <t>  synParam
%type <t>  synAlt
%type <t>  synSeqs
%type <t>  synSeq
%type <t>  nonEmptySeq
%type <t>  optSynAction
%type <t>  synPrims
%type <t>  synPrim
%type <t>  synPrimNoAssign
%type <t>  synPrimNoVar
%type <t>  synPrimNoVarNoAssign
%type <t>  synInstTerm
%type <t>  synLabel
%type <t>  synProdCallParams
%type <t>  iterators
%type <t>  iterator
%type <t>  optIteratorStep
%type <t>  FOR_decls
%type <t>  FOR_decl
%type <t>  FOR_gen
%type <t>  FOR_genOptInt
%type <t>  FOR_genOptC
%type <t>  FOR_genOptC2

%%

file		: queries T_ENDOFFILE
		  { yyoutput = $1; YYACCEPT; }
		| error
		  { yyoutput = PA_parseError; YYABORT; }
		;

queries		: sequence queries1
		  { $$ = oz_cons($1,$2); }
		| prodClauseList queries1
		  { $$ = oz_cons(newCTerm(PA_fSynTopLevelProductionTemplates,
					   $1),$2); }
		| queries1
		  { $$ = $1; }
		| thisCoord functorDescriptorList queries1
		  { $$ = oz_cons(newCTerm(PA_fFunctor,newCTerm(PA_fDollar,$1),
					   $2,$1),$3); }
		;

queries1	: directive queries
		  { $$ = oz_cons($1,$2); }
		| T_declare coord sequence T_in thisCoord queries1
		  { $$ = oz_cons(newCTerm(PA_fDeclare,$3,newCTerm(PA_fSkip,$5),
					   $2),$6); }
		| T_declare coord sequence T_in sequence queries1
		  { $$ = oz_cons(newCTerm(PA_fDeclare,$3,$5,$2),$6); }
		| T_declare coord sequence thisCoord queries1
		  { $$ = oz_cons(newCTerm(PA_fDeclare,$3,
					   newCTerm(PA_fSkip,$4),$2),$5); }
		| /* empty */
		  { $$ = AtomNil; }
		;

directive	: T_SWITCH switchList
		  { $$ = newCTerm(PA_dirSwitch,$2); }
		| T_LOCALSWITCHES
		  { $$ = PA_dirLocalSwitches; }
		| T_PUSHSWITCHES
		  { $$ = PA_dirPushSwitches; }
		| T_POPSWITCHES
		  { $$ = PA_dirPopSwitches; }
		;

switchList	: /* empty */
		  { $$ = AtomNil; }
		| switch switchList
		  { $$ = oz_cons($1,$2); }
		;

switch		: '+' T_SWITCHNAME
		  { if (!strcmp(xytext,"gump"))
		      xy_gumpSyntax = 1;
		    if (!strcmp(xytext,"allowdeprecated"))
		      xy_allowDeprecated = 1;
		    $$ = newCTerm(PA_on,OZ_atom(xytext),pos());
		  }
		| '-' T_SWITCHNAME
		  { if (!strcmp(xytext,"gump"))
		      xy_gumpSyntax = 0;
		    if (!strcmp(xytext,"allowdeprecated"))
		      xy_allowDeprecated = 0;
		    $$ = newCTerm(PA_off,OZ_atom(xytext),pos());
		  }
		;

sequence	: phrase
		  { $$ = $1; }
		| phrase sequence
		  { $$ = newCTerm(PA_fAnd,$1,$2); }
		;

phrase		: phrase '=' coord phrase
		  { $$ = newCTerm(PA_fEq,$1,$4,$3); }
		| phrase T_DOTASSIGN coord phrase
		  { $$ = newCTerm(PA_fDotAssign,$1,$4,$3); }
		| phrase T_OOASSIGN coord phrase
		  { $$ = newCTerm(PA_fAssign,$1,$4,$3); }
		| phrase T_orelse coord phrase
		  { $$ = newCTerm(PA_fOrElse,$1,$4,$3); }
		| phrase T_andthen coord phrase
		  { $$ = newCTerm(PA_fAndThen,$1,$4,$3); }
		| phrase compare coord phrase %prec T_COMPARE
		  { $$ = newCTerm(PA_fOpApply,$2,
				  oz_mklist($1,$4),$3); }
		| phrase fdCompare coord phrase %prec T_FDIN
		  { $$ = newCTerm(PA_fFdCompare,$2,$1,$4,$3); }
		| phrase fdIn coord phrase %prec T_FDIN
		  { $$ = newCTerm(PA_fFdIn,$2,$1,$4,$3); }
		| phrase '|' coord phrase
		  { $$ = makeCons($1,$4,$3); }
		| phrase2
		  { $$ = $1; }
		| phrase2 '#' coord hashes
		  { $$ = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,$3),
				  oz_cons($1,$4)); }
		;

hashes		: phrase2
		  { $$ = oz_mklist($1); }
		| phrase2 '#' hashes
		  { $$ = oz_cons($1,$3); }
		;

phrase2		: phrase2 add coord phrase2 %prec T_ADD
		  { $$ = newCTerm(PA_fOpApply,$2,
				  oz_mklist($1,$4),$3); }
		| phrase2 fdMul coord phrase2 %prec T_FDMUL
		  { $$ = newCTerm(PA_fOpApply,$2,
				  oz_mklist($1,$4),$3); }
		| phrase2 otherMul coord phrase2 %prec T_OTHERMUL
		  { $$ = newCTerm(PA_fOpApply,$2,
				  oz_mklist($1,$4),$3); }
		| phrase2 ',' coord phrase2
		  { $$ = newCTerm(PA_fObjApply,$1,$4,$3); }
		| '~' coord phrase2 %prec '~'
		  { $$ = newCTerm(PA_fOpApply,AtomTilde,
				  oz_mklist($3),$2); }
		| phrase2 '.' coord phrase2
		  { $$ = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklist($1,$4),$3); }
		| phrase2 T_DOTINT
		  { $$ = newCTerm(PA_fOpApply,AtomDot,
				  oz_mklist($1,makeInt(xytext,pos())),pos()); }
		| phrase2 '^' coord phrase2
		  { $$ = newCTerm(PA_fOpApply,AtomHat,
				  oz_mklist($1,$4),$3); }
		| '@' coord phrase2
		  { $$ = newCTerm(PA_fAt,$3,$2); }
		| T_DEREFF coord phrase2
		  { $$ = newCTerm(PA_fOpApply,AtomDExcl,
				  oz_mklist($3),$2); }
		| '(' inSequence ')'
		  { $$ = $2; }
		| atom
		  { $$ = $1; }
		| variable
		  { $$ = $1; }
		| '_'
		  { $$ = newCTerm(PA_fWildcard,pos()); }
		| T_unit
		  { $$ = newCTerm(PA_fAtom,NameUnit,pos()); }
		| T_true
		  { $$ = newCTerm(PA_fAtom,NameTrue,pos()); }
		| T_false
		  { $$ = newCTerm(PA_fAtom,NameFalse,pos()); }
		| T_self
		  { $$ = newCTerm(PA_fSelf,pos()); }
		| '$'
		  { $$ = newCTerm(PA_fDollar,pos()); }
		| string
		  { $$ = $1; }
		| int
		  { $$ = $1; }
		| float
		  { $$ = $1; }
		| record
		  { $$ = $1; }
		| '[' coord phrase fixedListArgs ']' coord
		  { $$ = newCTerm(PA_fRecord,newCTerm(PA_fAtom,AtomCons,
						     makeLongPos($2,$6)),
				  oz_mklist($3,$4)); }
		| '{' coord phrase phraseList '}' coord
		  { $$ = newCTerm(PA_fApply,$3,$4,makeLongPos($2,$6)); }
		| T_proc coord procFlags '{' phrase phraseList '}'
		  inSequence T_end coord
		  { $$ = newCTerm(PA_fProc,$5,$6,$8,$3,makeLongPos($2,$10)); }
		| T_fun coord procFlags '{' phrase phraseList '}'
		  inSequence T_end coord
		  { $$ = newCTerm(PA_fFun,$5,$6,$8,$3,makeLongPos($2,$10)); }
		| T_functor coord phraseOpt optFunctorDescriptorList T_end coord
		  { $$ = newCTerm(PA_fFunctor,$3,$4,makeLongPos($2,$6)); }
		| class
		  { $$ = $1; }
		| T_local coord sequence T_in sequence T_end
		  { $$ = newCTerm(PA_fLocal,$3,$5,$2); }
		| T_if ifMain
		  { $$ = $2; }
		| T_case caseMain
		  { $$ = $2; }
		| T_lock coord inSequence T_end coord
		  { $$ = newCTerm(PA_fLock,$3,makeLongPos($2,$5)); }
		| T_lock coord phrase T_then inSequence T_end coord
		  { $$ = newCTerm(PA_fLockThen,$3,$5,makeLongPos($2,$7)); }
		| T_thread coord inSequence T_end coord
		  { $$ = newCTerm(PA_fThread,$3,makeLongPos($2,$5)); }
		| T_try coord inSequence optCatch optFinally T_end coord
		  { $$ = newCTerm(PA_fTry,$3,$4,$5,makeLongPos($2,$7)); }
		| T_raise coord inSequence T_end coord
		  { $$ = newCTerm(PA_fRaise,$3,makeLongPos($2,$5)); }
		| T_skip
		  { $$ = newCTerm(PA_fSkip,pos()); }
		| T_fail
		  { $$ = newCTerm(PA_fFail,pos()); }
		| T_not coord inSequence T_end coord
		  { $$ = newCTerm(PA_fNot,$3,makeLongPos($2,$5)); }
		| T_cond condMain
		  { $$ = $2; }
		| T_or coord orClauseList T_end coord
		  { $$ = newCTerm(PA_fOr,$3,makeLongPos($2,$5)); }
		| T_dis coord orClauseList T_end coord
		  { $$ = newCTerm(PA_fDis,$3,makeLongPos($2,$5)); }
		| T_choice coord choiceClauseList T_end coord
		  { $$ = newCTerm(PA_fChoice,$3,makeLongPos($2,$5)); }
		| scannerSpecification
		  { $$ = $1; }
		| parserSpecification
		  { $$ = $1; }
		| T_LMACRO coord phraseList T_RMACRO coord
		  { $$ = newCTerm(PA_fMacro,$3,makeLongPos($2,$5)); }
		| T_for coord iterators T_do inSequence T_end coord
		  { $$ = newCTerm(PA_fLoop,
				  newCTerm(PA_fAnd,$3,$5),
				  makeLongPos($2,$7)); }
		| T_FOR coord FOR_decls T_do inSequence T_end coord
		  { $$ = newCTerm(PA_fFOR,$3,$5,makeLongPos($2,$7)); }
		;

FOR_decls	: /* empty */
		  { $$ = AtomNil; }
		| FOR_decl FOR_decls
		  { $$ = oz_cons($1,$2); }
		;

FOR_decl	: atom ':' phrase
		  { $$ = newCTerm(oz_atom("forFeature"),$1,$3); }
		| phrase T_in FOR_gen
		  { $$ = newCTerm(oz_atom("forPattern"),$1,$3); }
		| phrase T_from phrase
		  { $$ = newCTerm(oz_atom("forFrom"),$1,$3); }
		;

FOR_gen		: phrase
		  { $$ = newCTerm(oz_atom("forGeneratorList"),$1); }
		| phrase T_2DOTS phrase FOR_genOptInt
		  { $$ = newCTerm(oz_atom("forGeneratorInt"),$1,$3,$4); }
		| phrase ';' FOR_genOptC
		  { $$ = newCTerm(oz_atom("forGeneratorC"),$1,oz_head($3),oz_tail($3)); }
		| '(' phrase ';' FOR_genOptC ')'
		  { $$ = newCTerm(oz_atom("forGeneratorC"),$2,oz_head($4),oz_tail($4)); }
		;

FOR_genOptInt	: /* empty */
		  { $$ = NameUnit; }
		| ';' phrase
		  { $$ = $2; }
		;

FOR_genOptC	: phrase FOR_genOptC2
		  { $$ = oz_cons($1,$2); }
		;

FOR_genOptC2	: /* empty */
		  { $$ = NameUnit; }
		| ';' phrase
		  { $$ = $2; }
		;

iterators	: iterators iterator
		  {
		    $$ = newCTerm(PA_fAnd,$1,$2);
		  }
		| iterator
		;

iterator	: nakedVariable T_in coord phrase T_2DOTS phrase optIteratorStep coord
		  {
		    $$ = newCTerm(PA_fMacro,
				  oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					  $1,
					  newCTerm(PA_fAtom,oz_atom("from"),NameUnit),
					  $4,
					  newCTerm(PA_fAtom,oz_atom("to"),NameUnit),
					  $6,
					  newCTerm(PA_fAtom,oz_atom("by"),NameUnit),
					  ($7 == 0)?makeInt("1",NameUnit):$7,
					  0),
				  makeLongPos(OZ_subtree($1,makeTaggedSmallInt(2)),$8));
		  }
		| nakedVariable T_in coord phrase optIteratorStep coord
                  {
		    /* <<for X 'in' L>>
		       <<for X = E1 'then' E2>> */
		    if ($5 == 0) {
		      $$ = newCTerm(PA_fMacro,
				    oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					    $1,
					    newCTerm(PA_fAtom,oz_atom("in"),NameUnit),
					    $4,
					    0),
				    makeLongPos(OZ_subtree($1,makeTaggedSmallInt(2)),$6));
		    } else {
		      $$ = newCTerm(PA_fMacro,
				    oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
					    newCTerm(PA_fEq,$1,$4,NameUnit),
					    newCTerm(PA_fAtom,oz_atom("next"),NameUnit),
					    $5,
					    0),
				    makeLongPos(OZ_subtree($1,makeTaggedSmallInt(2)),$6));
		    }
		  }
		;

optIteratorStep	: { $$ = 0; }
		| ';' phrase
		  { $$ = $2; }
		;

procFlags	: /* empty */
		  { $$ = AtomNil; }
		| atom procFlags
		  { $$ = oz_cons($1,$2); }
		;

optFunctorDescriptorList
		: /* empty */
		  { $$ = AtomNil; }
		| functorDescriptorList
		  { $$ = $1; }
		;

functorDescriptorList
		: T_require coord importDecls optFunctorDescriptorList
		  { $$ = oz_cons(newCTerm(PA_fRequire,$3,$2),$4); }
		| T_prepare coord sequence T_in sequence optFunctorDescriptorList
		  { $$ = oz_cons(newCTerm(PA_fPrepare,$3,$5,$2),$6); }
		| T_prepare coord sequence optFunctorDescriptorList
		  { $$ = oz_cons(newCTerm(PA_fPrepare,$3,
					   newCTerm(PA_fSkip,$2),$2),$4); }
		| T_import coord importDecls optFunctorDescriptorList
		  { $$ = oz_cons(newCTerm(PA_fImport,$3,$2),$4); }
		| T_export coord exportDecls optFunctorDescriptorList
		  { $$ = oz_cons(newCTerm(PA_fExport,$3,$2),$4); }
		| T_define coord sequence T_in sequence optFunctorDescriptorList
		  { $$ = oz_cons(newCTerm(PA_fDefine,$3,$5,$2),$6); }
		| T_define coord sequence optFunctorDescriptorList
		  { $$ = oz_cons(newCTerm(PA_fDefine,$3,
					   newCTerm(PA_fSkip,$2),$2),$4); }
		;

importDecls	: /* empty */
		  { $$ = AtomNil; }
		| nakedVariable optImportAt importDecls
		  { $$ = oz_cons(newCTerm(PA_fImportItem,$1,AtomNil,$2),$3); }
		| variableLabel '(' featureList ')' optImportAt importDecls
		  { $$ = oz_cons(newCTerm(PA_fImportItem,$1,$3,$5),$6); }
		;

variableLabel	: T_VARIABLE_LABEL coord
		  { $$ = newCTerm(PA_fVar,OZ_atom(xytext),$2); }
		;

featureList	: featureNoVar
		  { $$ = oz_mklist($1); }
		| featureNoVar ':' nakedVariable
		  { $$ = oz_mklist(oz_pair2($3,$1)); }
		| featureNoVar featureList
		  { $$ = oz_cons($1,$2); }
		| featureNoVar ':' nakedVariable featureList
		  { $$ = oz_cons(oz_pair2($3,$1),$4); }
		;

optImportAt	: /* empty */
		  { $$ = PA_fNoImportAt; }
		| T_at atom
		  { $$ = newCTerm(PA_fImportAt,$2); }
		;

exportDecls	: /* empty */
		  { $$ = AtomNil; }
		| nakedVariable exportDecls
		  { $$ = oz_cons(newCTerm(PA_fExportItem,$1),$2); }
		| featureNoVar ':' nakedVariable exportDecls
		  { $$ = oz_cons(newCTerm(PA_fExportItem,
					   newCTerm(PA_fColon,$1,$3)),$4); }
		;

compare		: T_COMPARE
		  { $$ = OZ_atom(xytext); }
		;

fdCompare	: T_FDCOMPARE
		  { $$ = OZ_atom(xytext); }
		;

fdIn		: T_FDIN
		  { $$ = OZ_atom(xytext); }
		;

add		: T_ADD
		  { $$ = OZ_atom(xytext); }
		;

fdMul		: T_FDMUL
		  { $$ = OZ_atom(xytext); }
		;

otherMul	: T_OTHERMUL
		  { $$ = OZ_atom(xytext); }
		;

inSequence	: sequence T_in coord sequence
		  { $$ = newCTerm(PA_fLocal,$1,$4,$3); }
		| sequence
		  { $$ = $1; }
		;

phraseList	: /* empty */
		  { $$ = AtomNil; }
		| phrase phraseList
		  { $$ = oz_cons($1,$2); }
		;

fixedListArgs	: thisCoord
		  { $$ = newCTerm(PA_fAtom,AtomNil,$1); }
		| thisCoord phrase fixedListArgs
		  { $$ = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomCons,$1),
				  oz_mklist($2,$3)); }
		;

optCatch	: /* empty */
		  { $$ = PA_fNoCatch; }
		| T_catch coord caseClauseList
		  { $$ = newCTerm(PA_fCatch,$3,$2); }
		;

optFinally	: /* empty */
		  { $$ = PA_fNoFinally; }
		| T_finally inSequence
		  { $$ = $2; }
		;

record		: recordAtomLabel coord '(' recordArguments optDots ')' coord
		  {
		    $$ = newCTerm(OZ_isTrue($5)? PA_fOpenRecord : PA_fRecord,
				  newCTerm(PA_fAtom,$1,makeLongPos($2,$7)),$4);
		  }
		| recordVarLabel coord '(' recordArguments optDots ')' coord
		  {
		    $$ = newCTerm(OZ_isTrue($5)? PA_fOpenRecord : PA_fRecord,
				  makeVar($1,makeLongPos($2,$7)),$4);
		  }
		;

recordAtomLabel	: T_ATOM_LABEL
		  { $$ = OZ_atom(xytext); }
		| T_UNIT_LABEL
		  { $$ = NameUnit; }
		| T_TRUE_LABEL
		  { $$ = NameTrue; }
		| T_FALSE_LABEL
		  { $$ = NameFalse; }
		;

recordVarLabel	: T_VARIABLE_LABEL
		  { $$ = OZ_atom(xytext); }
		;

recordArguments	: /* empty */
		  { $$ = AtomNil; }
		| phrase recordArguments
		  { $$ = oz_cons($1,$2); }
		| feature ':' phrase recordArguments
		  { $$ = oz_cons(newCTerm(PA_fColon,$1,$3),$4); }
		;

optDots		: /* empty */
		  { $$ = NameFalse; }
		| T_LDOTS
		  { $$ = NameTrue; }
		;

feature		: atom
		  { $$ = $1; }
		| nakedVariable
		  { $$ = $1; }
		| int
		  { $$ = $1; }
		| T_unit
		  { $$ = newCTerm(PA_fAtom,NameUnit,pos()); }
		| T_true
		  { $$ = newCTerm(PA_fAtom,NameTrue,pos()); }
		| T_false
		  { $$ = newCTerm(PA_fAtom,NameFalse,pos()); }
		;

featureNoVar	: atom
		  { $$ = $1; }
		| int
		  { $$ = $1; }
		;

ifMain		: coord sequence T_then inSequence ifRest coord
		  { $$ = newCTerm(PA_fBoolCase,$2,$4,$5,makeLongPos($1,$6)); }
		;

ifRest		: T_elseif ifMain
		  { $$ = $2; }
		| T_elsecase caseMain
		  { $$ = $2; }
		| T_else inSequence T_end
		  { $$ = $2; }
		| T_end
		  { $$ = newCTerm(PA_fNoElse,pos()); }
		;

caseMain	: coord sequence T_then coord inSequence caseRest coord
		  { checkDeprecation($4);
		    $$ = newCTerm(PA_fBoolCase,$2,$5,$6,makeLongPos($1,$7));
		  }
		| coord sequence T_of elseOfList caseRest coord
		  { $$ = newCTerm(PA_fCase,$2,$4,$5,makeLongPos($1,$6)); }
		;

caseRest	: T_elseif ifMain
		  { $$ = $2; }
		| T_elsecase caseMain
		  { $$ = $2; }
		| T_else inSequence T_end
		  { $$ = $2; }
		| T_end
		  { $$ = newCTerm(PA_fNoElse,pos()); }
		;

elseOfList	: caseClause
		  { $$ = oz_mklist($1); }
		| caseClause T_CHOICE elseOfList
		  { $$ = oz_cons($1,$3); }
		| caseClause T_elseof elseOfList
		  { $$ = oz_cons($1,$3); }
		;

caseClauseList	: caseClause
		  { $$ = oz_mklist($1); }
		| caseClause T_CHOICE caseClauseList
		  { $$ = oz_cons($1,$3); }
		;

caseClause	: sideCondition T_then inSequence
		  { $$ = newCTerm(PA_fCaseClause,$1,$3); }
		;

sideCondition	: pattern
		  { $$ = $1; }
		| pattern T_andthen thisCoord sequence
		  { $$ = newCTerm(PA_fSideCondition,$1,
				  newCTerm(PA_fSkip,$3),$4,$3); }
		| pattern T_andthen thisCoord sequence T_in sequence
		  { $$ = newCTerm(PA_fSideCondition,$1,$4,$6,$3); }
		;

pattern		: pattern '=' coord pattern
		  { $$ = newCTerm(PA_fEq,$1,$4,$3); }
		| pattern '|' coord pattern
		  { $$ = makeCons($1,$4,$3); }
		| phrase2
		  { $$ = $1; }
		| phrase2 '#' coord hashes
		  { $$ = newCTerm(PA_fRecord,
				  newCTerm(PA_fAtom,AtomPair,$3),
				  oz_cons($1,$4)); }
		;

class		: T_class coord phraseOpt classDescriptorList methList
		  T_end coord
		  { $$ = newCTerm(PA_fClass,$3,$4,$5,makeLongPos($2,$7)); }
		;

phraseOpt	: phrase
		  { $$ = $1; }
		| thisCoord
		  { $$ = newCTerm(PA_fDollar,$1); }
		;

classDescriptorList
		: /* empty */
		  { $$ = AtomNil; }
		|  classDescriptor classDescriptorList
		  { $$ = oz_cons($1,$2); }
		;

classDescriptor	: T_from coord phrase phraseList
		  { $$ = newCTerm(PA_fFrom,oz_cons($3,$4),$2); }
		| T_attr coord attrFeat attrFeatList
		  { $$ = newCTerm(PA_fAttr,oz_cons($3,$4),$2); }
		| T_feat coord attrFeat attrFeatList
		  { $$ = newCTerm(PA_fFeat,oz_cons($3,$4),$2); }
		| T_prop coord phrase phraseList
		  { $$ = newCTerm(PA_fProp,oz_cons($3,$4),$2); }
		;

attrFeatList	: /* empty */
		  { $$ = AtomNil; }
	        | attrFeat attrFeatList
		  { $$ = oz_cons($1,$2); }
		;

attrFeat	: attrFeatFeature ':' phrase
		  { $$ = oz_pair2($1,$3); }
		| attrFeatFeature
		  { $$ = $1; }
		;

attrFeatFeature	: atom
		  { $$ = $1; }
		| variable
		  { $$ = $1; }
		| int
		  { $$ = $1; }
		| T_unit
		  { $$ = newCTerm(PA_fAtom,NameUnit,pos()); }
		| T_true
		  { $$ = newCTerm(PA_fAtom,NameTrue,pos()); }
		| T_false
		  { $$ = newCTerm(PA_fAtom,NameFalse,pos()); }
		;

methList	: /* empty */
		  { $$ = AtomNil; }
		| meth methList
		  { $$ = oz_cons($1,$2); }
		;

meth		: T_meth coord methHead inSequence T_end
		  { $$ = newCTerm(PA_fMeth,$3,$4,$2); }
		;

methHead	: methHead1
		  { $$ = $1; }
		| methHead1 '=' coord nakedVariable
		  { $$ = newCTerm(PA_fEq,$1,$4,$3); }
		;

methHead1	: atom
		  { $$ = $1; }
		| variable
		  { $$ = $1; }
		| T_unit
		  { $$ = newCTerm(PA_fAtom,NameUnit,pos()); }
		| T_true
		  { $$ = newCTerm(PA_fAtom,NameTrue,pos()); }
		| T_false
		  { $$ = newCTerm(PA_fAtom,NameFalse,pos()); }
		| methHeadLabel '(' methFormals ')'
		  { $$ = newCTerm(PA_fRecord,$1,$3); }
		| methHeadLabel '(' methFormals T_LDOTS ')'
		  { $$ = newCTerm(PA_fOpenRecord,$1,$3); }
		;

methHeadLabel	: T_ATOM_LABEL
		  { $$ = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); }
		| T_VARIABLE_LABEL
		  { $$ = makeVar(xytext); }
		| '!' coord T_VARIABLE_LABEL
		  { $$ = newCTerm(PA_fEscape,makeVar(xytext),$2); }
		| T_UNIT_LABEL
		  { $$ = newCTerm(PA_fAtom,NameUnit,pos()); }
		| T_TRUE_LABEL
		  { $$ = newCTerm(PA_fAtom,NameTrue,pos()); }
		| T_FALSE_LABEL
		  { $$ = newCTerm(PA_fAtom,NameFalse,pos()); }
		;

methFormals	: methFormal methFormals
		  { $$ = oz_cons($1,$2); }
		| /* empty */
		  { $$ = AtomNil; }
		;

methFormal	: methFormalTerm methFormalOptDefault
		  { $$ = newCTerm(PA_fMethArg,$1,$2); }
		| feature ':' methFormalTerm methFormalOptDefault
		  { $$ = newCTerm(PA_fMethColonArg,$1,$3,$4); }
		;

methFormalTerm	: nakedVariable
		  { $$ = $1; }
		| '$'
		  { $$ = newCTerm(PA_fDollar,pos()); }
		| '_'
		  { $$ = newCTerm(PA_fWildcard,pos()); }
		;

methFormalOptDefault
		: T_DEFAULT coord phrase
		  { $$ = newCTerm(PA_fDefault,$3,$2); }
		| /* empty */
		  { $$ = PA_fNoDefault; }
		;

condMain	: coord condClauseList condElse T_end coord
		  { $$ = newCTerm(PA_fCond,$2,$3,makeLongPos($1,$5)); }
		;

condElse	: T_else inSequence
		  { $$ = $2; }
		| /* empty */
		  { $$ = newCTerm(PA_fNoElse,pos()); }
		;

condClauseList	: condClause
		  { $$ = oz_mklist($1); }
		| condClause T_CHOICE condClauseList
		  { $$ = oz_cons($1,$3); }
		;

condClause	: sequence T_then coord inSequence
		  { $$ = newCTerm(PA_fClause,newCTerm(PA_fSkip,$3),$1,$4); }
		| sequence T_in sequence T_then inSequence
		  { $$ = newCTerm(PA_fClause,$1,$3,$5); }
		;

orClauseList	: orClause T_CHOICE orClause
		  { $$ = oz_mklist($1,$3); }
		| orClause T_CHOICE orClauseList
		  { $$ = oz_cons($1,$3); }
		;

orClause	: sequence thisCoord
		  { $$ = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,$2),
				  $1,newCTerm(PA_fNoThen,$2)); }
		| sequence T_in sequence thisCoord
		  { $$ = newCTerm(PA_fClause,$1,$3,newCTerm(PA_fNoThen,$4)); }
		| sequence thisCoord T_then inSequence
		  { $$ = newCTerm(PA_fClause,
				  newCTerm(PA_fSkip,$2),$1,$4); }
		| sequence T_in sequence T_then inSequence
		  { $$ = newCTerm(PA_fClause,$1,$3,$5); }
		;

choiceClauseList: inSequence
		  { $$ = oz_mklist($1); }
		| inSequence T_CHOICE choiceClauseList
		  { $$ = oz_cons($1,$3); }
		;

atom		: T_OZATOM
		  { $$ = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); }
		;

nakedVariable	: T_VARIABLE
		  { $$ = makeVar(xytext); }
		;

variable	: nakedVariable
		  { $$ = $1; }
		| '!' coord nakedVariable
		  { $$ = newCTerm(PA_fEscape,$3,$2); }
		;

string		: T_STRING
		  { $$ = makeString(xytext,pos()); }
		;

int		: T_OZINT
		  { $$ = makeInt(xytext,pos()); }
		| T_AMPER
		  { $$ = makeInt(xytext[0],pos()); }
		;

float		: T_OZFLOAT
		  { $$ = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); }
		;

thisCoord	: /* empty */
		  { $$ = pos(); }   /*--** should be: coords of next token */
		;

coord		: /* empty */
		  { $$ = pos(); }
		;


/*--------------------------------------------------------------------*/
/* Gump Extensions                                                    */
/*--------------------------------------------------------------------*/

scannerSpecification
		: T_scanner coord nakedVariable
		  classDescriptorList methList scannerRules T_end coord
		  { OZ_Term prefix =
		      scannerPrefix? scannerPrefix: PA_zy;
		    $$ = newCTerm(PA_fScanner,$3,$4,$5,$6,prefix,
				  makeLongPos($2,$8)); }
		;

scannerRules	: lexAbbrev
		  { $$ = oz_mklist($1); }
		| lexRule
		  { $$ = oz_mklist($1); }
		| modeClause
		  { $$ = oz_mklist($1); }
		| lexAbbrev scannerRules
		  { $$ = oz_cons($1,$2); }
		| lexRule scannerRules
		  { $$ = oz_cons($1,$2); }
		| modeClause scannerRules
		  { $$ = oz_cons($1,$2); }
		;

lexAbbrev	: T_lex atom '=' regex T_end
		  { $$ = newCTerm(PA_fLexicalAbbreviation,$2,$4); }
		| T_lex nakedVariable '=' regex T_end
		  { $$ = newCTerm(PA_fLexicalAbbreviation,$2,$4); }
		;

lexRule		: T_lex regex inSequence T_end
		  { $$ = newCTerm(PA_fLexicalRule,$2,$3); }
		;

regex		: T_REGEX
		  { $$ = OZ_string(xytext); }
		| T_STRING
		  { $$ = OZ_string(xytext); }
		;

modeClause	: T_mode nakedVariable modeDescrs T_end
		  { $$ = newCTerm(PA_fMode,$2,$3); }
		;

modeDescrs	: /* empty */
		  { $$ = AtomNil; }
		| modeDescr modeDescrs
		  { $$ = oz_cons($1,$2); }
		;

modeDescr	: T_from modeFromList
		  { $$ = newCTerm(PA_fInheritedModes,$2); }
		| lexRule
		  { $$ = $1; }
		| modeClause
		  { $$ = $1; }
		;


parserSpecification
		: T_parser coord nakedVariable
		  classDescriptorList methList
		  tokenClause parserRules T_end coord
		  { OZ_Term expect = parserExpect? parserExpect: makeTaggedSmallInt(0);
		    $$ = newCTerm(PA_fParser,$3,$4,$5,$6,$7,expect,
				  makeLongPos($2,$9)); }
		;

parserRules	: synClause
		  { $$ = oz_mklist($1); }
		| prodClause
		  { $$ = oz_mklist($1); }
		| synClause parserRules
		  { $$ = oz_cons($1,$2); }
		| prodClause parserRules
		  { $$ = oz_cons($1,$2); }
		;

tokenClause	: /* empty */
		  { $$ = newCTerm(PA_fToken,AtomNil); }
		| T_token tokenList
		  { $$ = newCTerm(PA_fToken,$2); }
		;

tokenList	: tokenDecl
		  { $$ = oz_mklist($1); }
		| tokenDecl tokenList
		  { $$ = oz_cons($1,$2); }
		;

tokenDecl	: atom
		  { $$ = $1; }
		| atom ':' phrase
		  { $$ = oz_pair2($1,$3); }
		;

modeFromList	: nakedVariable
		  { $$ = oz_mklist($1); }
		| nakedVariable modeFromList
		  { $$ = oz_cons($1,$2); }
		;

prodClauseList	: prodClause
		  { $$ = oz_mklist($1); }
		| prodClause prodClauseList
		  { $$ = oz_cons($1,$2); }
		;

prodClause	: T_prod nakedVariable '='
		  { *prodKey[depth]++ = '='; }
		  prodHeadRest prodMakeKey localRules synAlt T_end
		  { $$ = newCTerm(PA_fProductionTemplate,$6,$5,$7,$8,$2); }
		| T_prod '$' { $<t>$ = newCTerm(PA_fDollar,pos()); } '='
		  { *prodKey[depth]++ = '='; }
		  prodHeadRest prodMakeKey localRules synAlt T_end
		  { $$ = newCTerm(PA_fProductionTemplate,$7,$6,$8,$9,$<t>3); }
		| T_prod prodHeadRest prodMakeKey localRules synAlt T_end
		  { $$ = newCTerm(PA_fProductionTemplate,$3,$2,$4,$5,PA_none); }
		;

prodHeadRest	: prodNameAtom nakedVariable optTerminatorOp
		  { $$ = oz_mklist($2); }
		| nakedVariable terminatorOp
		  { $$ = oz_mklist($1); }
		| prodName prodKey
		  { $$ = $2; }
		;

prodName	: prodNameAtom
		| /* empty */
		;

prodNameAtom	: atom ':'
		  { prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg($1,0))); }
		;

prodKey		: '(' { *prodKey[depth]++ = '('; depth++; }
		  prodParams ')' { depth--; } optTerminatorOp { $$ = $3; }
		| '[' { *prodKey[depth]++ = '['; depth++; }
		  prodParams ']' { depth--; } optTerminatorOp { $$ = $3; }
		| '{' { *prodKey[depth]++ = '{'; depth++; }
		  prodParams '}' { depth--; } optTerminatorOp { $$ = $3; }
		;

prodParams	: prodParam
		  { $$ = oz_mklist($1); }
		| prodParam separatorOp prodParams
		  { $$ = oz_cons($1,$3); }
		;

prodParam	: nakedVariable { $$ = $1; }
		| '_' { $$ = newCTerm(PA_fWildcard,pos()); }
		;

separatorOp	: T_SEP
		  { *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; }
		;

optTerminatorOp	: /* empty */
		| terminatorOp
		;

terminatorOp	: T_ADD { *prodKey[depth]++ = xytext[0]; }
		| T_FDMUL { *prodKey[depth]++ = xytext[0]; }
		;

prodMakeKey	: /* empty */
		  { *prodKey[depth] = '\0';
		    $$ = oz_pair2(prodName[depth],OZ_string(prodKeyBuffer[depth]));
		    prodName[depth] = PA_none;
		    prodKey[depth] = prodKeyBuffer[depth];
		  }
		;

localRules	: /* empty */
		  { $$ = AtomNil; }
		| localRulesSub T_in
		  { $$ = $1; }
		;

localRulesSub	: synClause
		  { $$ = oz_mklist($1); }
		| synClause localRulesSub
		  { $$ = oz_cons($1,$2); }
		;

synClause	: T_syn atom synAlt T_end
		  { $$ = newCTerm(PA_fSyntaxRule,$2,AtomNil,$3); }
		| T_syn nakedVariable synAlt T_end
		  { $$ = newCTerm(PA_fSyntaxRule,$2,AtomNil,$3); }
		| T_syn synLabel '(' synParams ')' synAlt T_end
		  { $$ = newCTerm(PA_fSyntaxRule,$2,$4,$6); }
		;

synParams	: /* empty */
		  { $$ = AtomNil; }
		| synParam synParams
		  { $$ = oz_cons($1,$2); }
		;

synParam	: nakedVariable
		  { $$ = $1; }
		| '$'
		  { $$ = newCTerm(PA_fDollar,pos()); }
		| '_'
		  { $$ = newCTerm(PA_fWildcard,pos()); }
		;

synAlt		: synSeqs
		  { $$ = newCTerm(PA_fSynAlternative, $1); }
		;

synSeqs		: synSeq
		  { $$ = oz_mklist($1); }
		| synSeq T_CHOICE synSeqs
		  { $$ = oz_cons($1,$3); }
		;

synSeq		: thisCoord nonEmptySeq
		  { OZ_Term t = $2;
		    while (terms[depth]) {
		      t = oz_cons(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    $$ = newCTerm(PA_fSynSequence, decls[depth], t, $1);
		    decls[depth] = AtomNil;
		  }
		| T_skip coord optSynAction
		  { $$ = newCTerm(PA_fSynSequence, AtomNil, $3, $2); }
		;

optSynAction	: /* empty */
		  { $$ = AtomNil; }
		| T_REDUCE inSequence
		  { $$ = oz_mklist(newCTerm(PA_fSynAction,$2)); }
		;

nonEmptySeq	: synVariable nonEmptySeq
		  { $$ = $2; }
		| synVariable terminatorOp coord synPrims prodMakeKey
		  { $$ = oz_cons(newCTerm(PA_fSynTemplateInstantiation, $5,
					   oz_cons(newCTerm(PA_fSynApplication,
							     terms[depth]->term,
							     AtomNil),
						    AtomNil),
					   $3),
				  $4);
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  }
		| synVariable '=' synPrimNoAssign synPrims
		  { $$ = oz_cons(newCTerm(PA_fSynAssignment, terms[depth]->term, $3),
				  $4);
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  }
		| T_in synPrims
		  { while (terms[depth]) {
		      decls[depth] = oz_cons(terms[depth]->term, decls[depth]);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    $$ = $2;
		  }
		| synPrimNoVar synPrims
		  { $$ = oz_cons($1,$2); }
		| optSynAction
		  { $$ = $1; }
		;

synVariable	: nakedVariable
		  { terms[depth] = new TermNode($1, terms[depth]); }
		;

synPrims	: optSynAction
		  { $$ = $1; }
		| synPrim synPrims
		  { $$ = oz_cons($1,$2); }
		;

synPrim		: variable '=' synPrimNoAssign
		  { $$ = newCTerm(PA_fSynAssignment,$1,$3); }
		| synPrimNoAssign
		  { $$ = $1; }
		;

synPrimNoAssign	: nakedVariable
		  { $$ = newCTerm(PA_fSynApplication,$1,AtomNil); }
		| nakedVariable terminatorOp coord prodMakeKey
		  { $$ = newCTerm(PA_fSynTemplateInstantiation,$4,
				  oz_cons(newCTerm(PA_fSynApplication,$1,
						    AtomNil),
					   AtomNil),$3);
		  }
		| synPrimNoVarNoAssign
		  { $$ = $1; }
		;

synPrimNoVar	: '!' coord nakedVariable '=' synPrimNoAssign
		  { $$ = newCTerm(PA_fSynAssignment,
				  newCTerm(PA_fEscape,$3,$2),$5); }
		| synPrimNoVarNoAssign
		  { $$ = $1; }
		;

synPrimNoVarNoAssign
		: synInstTerm
		  { $$ = $1; }
		| prodNameAtom coord synInstTerm optTerminatorOp prodMakeKey
		  { $$ = newCTerm(PA_fSynTemplateInstantiation,$5,
				  oz_mklist($3),$2);
		  }
		| synInstTerm terminatorOp coord prodMakeKey
		  { $$ = newCTerm(PA_fSynTemplateInstantiation,$4,
				  oz_mklist($1),$3);
		  }
		| prodName coord '(' { *prodKey[depth]++ = '('; depth++; }
		  synProdCallParams ')' { depth--; }
		  optTerminatorOp prodMakeKey
		  { $$ = newCTerm(PA_fSynTemplateInstantiation,$9,$5,$2); }
		| prodName coord '[' { *prodKey[depth]++ = '['; depth++; }
		  synProdCallParams ']' { depth--; }
		  optTerminatorOp prodMakeKey
		  { $$ = newCTerm(PA_fSynTemplateInstantiation,$9,$5,$2); }
		| prodName coord '{' { *prodKey[depth]++ = '{'; depth++; }
		  synProdCallParams '}' { depth--; }
		  optTerminatorOp prodMakeKey
		  { $$ = newCTerm(PA_fSynTemplateInstantiation,$9,$5,$2); }
		;

synInstTerm	: atom
		  { $$ = newCTerm(PA_fSynApplication,$1,AtomNil); }
		| synLabel coord '(' phraseList ')'
		  { $$ = newCTerm(PA_fSynApplication,$1,$4); }
		;

synLabel	: T_ATOM_LABEL
		  { $$ = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); }
		| T_VARIABLE_LABEL
		  { $$ = makeVar(xytext); }
		;

synProdCallParams
		: synAlt
		  { $$ = oz_mklist($1); }
		| synAlt separatorOp synProdCallParams
		  { $$ = oz_cons($1,$3); }
		;

%%

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
