%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef WINDOWS
#include <malloc.h>
#undef SP_ERROR
#include <winsock.h>
#include <process.h>
#include <time.h>
#else
#include <sys/time.h>
#include <sys/wait.h>
#endif

static void parserInit();
static unsigned int parseFile(char *file);
static unsigned int parseVirtualString(char *str);

#include "../include/config.h"
#include "oz.h"

typedef OZ_Term CTerm;

static CTerm nilAtom;

extern int xy_showInsert, xy_gumpSyntax;

OZ_C_proc_begin(ozparser_init, 0)
{
  nilAtom = OZ_nil();
  parserInit();
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(ozparser_setShowInsert, 1)
{
  OZ_declareNonvarArg(0, showInsert);
  xy_showInsert = OZ_isTrue(showInsert);
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(ozparser_setGumpSyntax, 1)
{
  OZ_declareNonvarArg(0, gumpSyntax);
  xy_gumpSyntax = OZ_isTrue(gumpSyntax);
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(ozparser_parseFile, 2)
{
  OZ_declareVirtualStringArg(0, str);
  return OZ_unify(OZ_getCArg(1), parseFile(str));
}
OZ_C_proc_end

OZ_C_proc_begin(ozparser_parseVirtualString, 2)
{
  OZ_declareVirtualStringArg(0, str);
  return OZ_unify(OZ_getCArg(1), parseVirtualString(str));
}
OZ_C_proc_end


static CTerm newCTerm(char *l) {
  return OZ_atom(l);
}

static CTerm newCTerm(char *l, CTerm t1) {
  OZ_Term t = OZ_tuple(OZ_atom(l), 1);
  OZ_putArg(t, 0, t1);
  return t;
}

static CTerm newCTerm(char *l, CTerm t1, CTerm t2) {
  OZ_Term t = OZ_tuple(OZ_atom(l), 2);
  OZ_putArg(t, 0, t1);
  OZ_putArg(t, 1, t2);
  return t;
}

static CTerm newCTerm(char *l, CTerm t1, CTerm t2, CTerm t3) {
  OZ_Term t = OZ_tuple(OZ_atom(l), 3);
  OZ_putArg(t, 0, t1);
  OZ_putArg(t, 1, t2);
  OZ_putArg(t, 2, t3);
  return t;
}

static CTerm newCTerm(char *l, CTerm t1, CTerm t2, CTerm t3, CTerm t4) {
  OZ_Term t = OZ_tuple(OZ_atom(l), 4);
  OZ_putArg(t, 0, t1);
  OZ_putArg(t, 1, t2);
  OZ_putArg(t, 2, t3);
  OZ_putArg(t, 3, t4);
  return t;
}

static CTerm newCTerm(char *l, CTerm t1, CTerm t2, CTerm t3, CTerm t4, CTerm t5) {
  OZ_Term t = OZ_tuple(OZ_atom(l), 5);
  OZ_putArg(t, 0, t1);
  OZ_putArg(t, 1, t2);
  OZ_putArg(t, 2, t3);
  OZ_putArg(t, 3, t4);
  OZ_putArg(t, 4, t5);
  return t;
}

static CTerm newCTerm(char *l, CTerm t1, CTerm t2, CTerm t3, CTerm t4, CTerm t5, CTerm t6) {
  OZ_Term t = OZ_tuple(OZ_atom(l), 6);
  OZ_putArg(t, 0, t1);
  OZ_putArg(t, 1, t2);
  OZ_putArg(t, 2, t3);
  OZ_putArg(t, 3, t4);
  OZ_putArg(t, 4, t5);
  OZ_putArg(t, 5, t6);
  return t;
}

#define pair(left, right)	OZ_pair2(left, right)
#define consList(head, tail)	OZ_cons(head, tail)

#define YYMAXDEPTH 1000000
#define YYERROR_VERBOSE


//**********************
// INTERFACE TO SCANNER
//**********************

void xyscannerInit();
int xy_init_from_file(char *file);
void xy_init_from_string(char *str);
void xy_exit();

int xylex();

extern int xylino;

/* Defined by (f)lex: */
extern char *xytext;
extern char *xylastline;

extern FILE *xyin;

extern char xyFileName[];       // name of the current file, "" means stdin
extern CTerm xyFileNameAtom;
extern char xyhelpFileName[];

static void xyerror(char *);
extern "C" int xyreportError(char *, char *, char *, int ,int);

static inline int xycharno() {
  int n = (xytext - xylastline) + 1;
  if (n > 0)
    return n;
  else
    return 0;
}

static CTerm yyoutput;

inline CTerm pos() {
  return newCTerm("pos",xyFileNameAtom,OZ_int(xylino),OZ_int(xycharno()));
}

inline CTerm makeVar(char *functor) {
  return newCTerm("fVar",newCTerm(functor),pos());
}

inline CTerm makeCompound(char *functor, CTerm first, CTerm second) {
   return newCTerm("fRecord",
		   newCTerm("fAtom",newCTerm(functor),pos()),
		   consList(first,consList(second,nilAtom)));
}

static CTerm makeInt(char *chars) {
  return newCTerm("fInt",OZ_CStringToInt(chars),pos());
}

static CTerm makeInt(char c) {
  return newCTerm("fInt",OZ_int((unsigned char) c),pos());
}

static CTerm makeString(char *chars) {
  if (chars[0] == '\0')
    return newCTerm("fAtom",newCTerm("nil"),pos());
  else
    return makeCompound("|",makeInt(chars[0]),makeString(&chars[1]));
}


//-----------------
// Gump Extensions
//-----------------

#define DEPTH 20

static int depth;

static char prodKeyBuffer[DEPTH][80];
static char *prodKey[DEPTH];
static CTerm prodName[DEPTH];

struct TermNode {
  CTerm term;
  TermNode *next;
  TermNode(CTerm t, TermNode *n) { term = t; next = n; }
};
static TermNode *terms[DEPTH];
static CTerm decls[DEPTH];

%}

%union {
  CTerm t;
  int i;
}

%token ENDOFFILE
%token proc _fun_ OZATOM
%token OZFLOAT OZINT end CHOICE
%token VARIABLE VARIABLE_LABEL UNIT_LABEL TRUE_LABEL FALSE_LABEL
%token _if_ then _else_ elseif elseof elsecase
%token or dis choice
%token skip true fail false unit local LOCK declare ATOM_LABEL
%token _in_ thread not catch finally try raise
%token ASSIGN _meth_ _class_ _from_ attr feat prop
%token _condis_
%token LDOTS DOTINT
%token FEED THREADEDFEED
%token MACHINE PRECOMPILE HELP FILENAME SWITCH
%token SHOWSWITCHES SWITCHNAME ON OFF
%token CORE CORETREE TOPVARS
%token STRING with
%token OBJPATTERNOPEN OBJPATTERNCLOSE
%token self _case_ of DEFAULT andthen orelse
%token ADD FDMUL COMPARE FDCOMPARE FDIN AMPER

%token REGEX lex _mode_ _parser_ prod _scanner_ syn token
%token REDUCE SEP

%type <t>  file
%type <t>  queries
%type <t>  queries1

%type <t>  add
%type <t>  amper
%type <t>  at
%type <t>  argumentList
%type <t>  apply
%type <t>  caseClause
%type <t>  caseClauseList
%type <t>  choicE
%type <t>  choiceList
%type <t>  choiceList1
%type <t>  classDescriptor
%type <t>  classDescriptorList
%type <t>  clause
%type <t>  clauseList
%type <t>  ifMain
%type <t>  ifRest
%type <t>  caseMain
%type <t>  elseOfList
%type <t>  compare
%type <t>  createExpression
%type <t>  createMeth
%type <t>  createMethList
%type <t>  colonPair
%type <t>  attrFeat
%type <t>  attrFeatList
%type <t>  constant
%type <t>  condisClause
%type <t>  condisClauseList
%type <t>  condisClauseList1
%type <t>  directive
%type <t>  caseRest
%type <t>  expression
%type <t>  fdcompare
%type <t>  fdexpression
%type <t>  fdin
%type <t>  fdmul
%type <t>  funExpression
%type <t>  functor
%type <t>  inexpression
%type <t>  int
%type <t>  label
%type <t>  methHead
%type <t>  methHead1
%type <t>  methHeadFunctor
%type <t>  methHeadArgumentList
%type <t>  methHeadArgumentList1
%type <t>  methHeadTerm
%type <t>  methHeadColonPair
%type <t>  methHeadDefaultEquation
%type <t>  feature
%type <t>  attrFeatFeature
%type <t>  nakedVariable
%type <t>  number
%type <t>  dotint
%type <t>  optcatch
%type <t>  optfinally
%type <t>  othermul
%type <t>  phi
%type <t>  position
%type <t>  pred
%type <t>  string
%type <t>  switchList
%type <t>  switch
%type <t>  term
%type <t>  hashes
%type <t>  termNoHash
%type <t>  fixedListArgs
%type <t>  termList
%type <t>  termOpt
%type <t>  uminus
%type <t>  variable
%type <t>  xi
%type <t>  xiList
%type <t>  xiList1

%type <t>  prodClauseList

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
%type <t>  synFunctor
%type <t>  synProdCallParams

/* Operator precedences */

%right    '='
%right    ASSIGN
%right    orelse
%right    andthen
%nonassoc COMPARE FDCOMPARE
%nonassoc FDIN
%right    '|'
%right    '#'
%left     ADD
%left     FDMUL OTHERMUL
%right    ','
%left     '~'
%left     '.' '^' DOTINT
%left     '@'

%%

file		: queries ENDOFFILE
		  { if (yynerrs) {
		      yyoutput = newCTerm("parseError");
		      YYABORT;
		    } else {
		      yyoutput = $1;
		      YYACCEPT;
		    }
		  }
		| prodClauseList ENDOFFILE
		  { if (yynerrs) {
		      yyoutput = newCTerm("parseError");
		      YYABORT;
		    } else {
		      yyoutput = newCTerm("fSynTopLevelProductionTemplates",$1);
		      YYACCEPT;
		    }
		  }
		| error
		  { yyoutput = newCTerm("parseError");
		    YYABORT;
		  }
		;

queries		: expression queries1
		  { $$ = consList($1,$2); }
		| queries1
		  { $$ = $1; }
		;

queries1	: directive queries
		  { $$ = consList($1,$2); }
		| declare position expression _in_ expression queries1
		  { $$ = consList(newCTerm("fDeclare",$3,$5,$2),$6); }
		| declare position expression position queries1
		  { $$ = consList(newCTerm("fDeclare",$3,
					   newCTerm("fSkip",$4),$2),$5); }
		| /* empty */
		  { $$ = nilAtom; }
		;

directive	: HELP
		  { $$ = newCTerm("dirHelp"); }
		| SWITCH switchList
		  { $$ = newCTerm("dirSwitch",$2); }
		| SHOWSWITCHES
		  { $$ = newCTerm("dirShowSwitches"); }
		| FEED FILENAME
		  { $$ = newCTerm("dirFeed",newCTerm(xyhelpFileName)); }
		| THREADEDFEED FILENAME
		  { $$ = newCTerm("dirThreadedFeed",newCTerm(xyhelpFileName)); }
		| CORETREE FILENAME
		  { $$ = newCTerm("dirCoreTree",newCTerm(xyhelpFileName)); }
		| CORE FILENAME
		  { $$ = newCTerm("dirCore",newCTerm(xyhelpFileName)); }
		| MACHINE FILENAME
		  { $$ = newCTerm("dirMachine",newCTerm(xyhelpFileName)); }
		| PRECOMPILE FILENAME
		  { $$ = newCTerm("dirPreCompile",newCTerm(xyhelpFileName)); }
		| TOPVARS FILENAME
		  { $$ = newCTerm("dirTopVars",newCTerm(xyhelpFileName)); }
		;

switchList	: /* empty */
		  { $$ = nilAtom; }
		| switch switchList
		  { $$ = consList($1,$2); }
		;

switch		: ON SWITCHNAME
		  { $$ = newCTerm("on",newCTerm(xytext),pos()); }
		| OFF SWITCHNAME
		  { $$ = newCTerm("off",newCTerm(xytext),pos()); }
		;

prodClauseList	: prodClause
		  { $$ = consList($1,nilAtom); }
		| prodClause prodClauseList
		  { $$ = consList($1,$2); }
		;

inexpression	: expression _in_ position expression
		  { $$ = newCTerm("fLocal",$1,$4,$3); }
		| expression
		  { $$ = $1; }

expression	: term
		  { $$ = $1; }
		| term expression
		  { $$ = newCTerm("fAnd",$1,$2); }
		;

term		: term '=' position term
		  { $$ = newCTerm("fEq",$1,$4,$3); }
		| term ASSIGN position term
		  { $$ = newCTerm("fAssign",$1,$4,$3); }
		| term orelse position term
		  { $$ = newCTerm("fOrElse",$1,$4,$3); }
		| term andthen position term
		  { $$ = newCTerm("fAndThen",$1,$4,$3); }
		| term compare position term %prec COMPARE
		  { $$ = newCTerm("fOpApply",$2,
				  consList($1,consList($4,nilAtom)),$3); }
		| term fdcompare position term   %prec FDIN
		  { $$ = newCTerm("fFdCompare",$2,$1,$4,$3); }
		| term fdin position term   %prec FDIN
		  { $$ = newCTerm("fFdIn",$2,$1,$4,$3); }
		| term '|' term
		  { $$ = makeCompound("|",$1,$3); }
		| termNoHash
		  { $$ = $1; }
		| termNoHash '#' hashes
		  { $$ = newCTerm("fRecord",
				  newCTerm("fAtom",newCTerm("#"),pos()),
				  consList($1,$3)); }
		;

hashes		: termNoHash
		  { $$ = consList($1,nilAtom); }
		| termNoHash '#' hashes
		  { $$ = consList($1,$3); }
		;

termNoHash	: termNoHash add position termNoHash %prec ADD
		  { $$ = newCTerm("fOpApply",$2,
				  consList($1,consList($4,nilAtom)),$3); }
		| termNoHash fdmul position termNoHash %prec FDMUL
		  { $$ = newCTerm("fOpApply",$2,
				  consList($1,consList($4,nilAtom)),$3); }
		| termNoHash othermul position termNoHash %prec OTHERMUL
		  { $$ = newCTerm("fOpApply",$2,
				  consList($1,consList($4,nilAtom)),$3); }
		| termNoHash ',' position termNoHash
		  { $$ = newCTerm("fObjApply",$1,$4,$3); }
		| uminus position termNoHash %prec '~'
		  { $$ = newCTerm("fOpApply",$1,
				  consList($3,nilAtom),$2); }
		| termNoHash '.' position termNoHash
		  { $$ = newCTerm("fOpApply",newCTerm("."),
				  consList($1,consList($4,nilAtom)),$3); }
		| termNoHash dotint
		  { $$ = newCTerm("fOpApply",newCTerm("."),
				  consList($1,consList($2,nilAtom)),pos()); }
		| termNoHash '^' position termNoHash
		  { $$ = newCTerm("fOpApply",newCTerm("^"),
				  consList($1,consList($4,nilAtom)),$3); }
		| at
		  { $$ = $1; }
		| '(' inexpression ')'
		  { $$ = $2; }
		| constant
		  { $$ = $1; }
		| variable
		  { $$ = $1; }
		| '_'
		  { $$ = newCTerm("fWildcard",pos()); }
		| unit
		  { $$ = newCTerm("fEscape",makeVar("`unit`"),pos()); }
		| true
		  { $$ = newCTerm("fEscape",makeVar("`true`"),pos()); }
		| false
		   { $$ = newCTerm("fEscape",makeVar("`false`"),pos()); }
		| self
		  { $$ = newCTerm("fSelf",pos()); }
		| '$'
		  { $$ = newCTerm("fDollar",pos()); }
		| string
		  { $$ = $1; }
		| number
		  { $$ = $1; }
		| functor OBJPATTERNOPEN argumentList OBJPATTERNCLOSE
		  { $$ = newCTerm("fObjPattern",$1,$3); }
		| functor '(' argumentList ')'
		  { $$ = newCTerm("fRecord",$1,$3); }
		| functor '(' argumentList LDOTS ')'
		  { $$ = newCTerm("fOpenRecord",$1,$3); }
		| '[' fixedListArgs ']'
		  { $$ = $2; }
		| apply
		  { $$ = $1; }
		| pred
		  { $$ = $1; }
		| funExpression
		  { $$ = $1; }
		| createExpression
		  { $$ = $1; }
		| _case_ caseMain
		  { $$ = $2; }
		| local position expression _in_ expression end
		  { $$ =  newCTerm("fLocal",$3,$5,$2); }
		| LOCK position inexpression end
		  { $$ =  newCTerm("fLock",$3,$2); }
		| LOCK position term then inexpression end
		  { $$ =  newCTerm("fLockThen",$3,$5,$2); }
		| thread position inexpression end
		  { $$ = newCTerm("fThread",$3,$2); }
		| try position inexpression optcatch optfinally end
		  { $$ = newCTerm("fTry",$3,$4,$5,$2); }
		| raise position inexpression end
		  { $$ = newCTerm("fRaise",$3,$2); }
		| raise position inexpression with inexpression end
		  { $$ = newCTerm("fRaiseWith",$3,$5,$2); }
		| phi      /* constraint */
		  { $$ = $1; }
		| not position inexpression end
		  { $$ = newCTerm("fNot",$3,$2); }
		| _if_ ifMain
		  { $$ = $2; }
		| or position xiList end
		  { $$ = newCTerm("fOr",$3,newCTerm("for"),$2); }
		| dis position xiList end
		  { $$ = newCTerm("fOr",$3,newCTerm("fdis"),$2); }
		| choice position choiceList1 end
		  { $$ = newCTerm("fOr",$3,newCTerm("fchoice"),$2); }
		| _condis_ position condisClauseList end
		  { $$ = newCTerm("fCondis",$3,$2); }
		| scannerSpecification
		  { $$ = $1; }
		| parserSpecification
		  { $$ = $1; }
		;

fixedListArgs	: term
		  { $$ = newCTerm("fRecord",
				  newCTerm("fAtom",newCTerm("|"),pos()),
				  consList($1,consList(newCTerm("fAtom",
								newCTerm("nil"),
								pos()),
						       nilAtom))); }
		| term fixedListArgs
		  { $$ = newCTerm("fRecord",
				  newCTerm("fAtom",newCTerm("|"),pos()),
				  consList($1,consList($2,nilAtom))); }
		;

optcatch	: /* empty */
		  { $$ = newCTerm("fNoCatch"); }
		| catch caseClauseList
		  { $$ = $2; }
		;

optfinally	: /* empty */
		  { $$ = newCTerm("fNoFinally"); }
		| finally inexpression
		  { $$ = $2; }
		;


ifMain		: position clauseList ifRest
		  { $$ = newCTerm("fIf",$2,$3,$1); }
		;

ifRest		: elseif ifMain
		  { $$ = $2; }
		| _else_ inexpression end
		  { $$ = $2; }
		| end
		  { $$ = newCTerm("fNoElse",pos()); }
		;

clauseList	: clause
		  { $$ = consList($1,nilAtom); }
		| clause CHOICE clauseList
		  { $$ = consList($1,$3); }
		;



caseMain	: position expression then inexpression caseRest
		  { $$ = newCTerm("fBoolCase",$2,$4,$5,$1); }
		| position expression of elseOfList caseRest
		  { $$ = newCTerm("fCase",$2,$4,$5,$1); }
		;

caseRest	: elsecase caseMain
		  { $$ = $2; }
		| _else_ inexpression end
		  { $$ = $2; }
		| end position
		  { $$ = newCTerm("fNoElse",$2); }
		;

elseOfList	: caseClauseList
		  { $$ = consList($1,nilAtom); }
		| caseClauseList elseof elseOfList
		  { $$ = consList($1,$3); }
		;

caseClauseList	: caseClause
		  { $$ = consList($1,nilAtom); }
		| caseClause CHOICE caseClauseList
		  { $$ = consList($1,$3); }
		;

caseClause	: inexpression then inexpression
		  { $$ = newCTerm("fCaseClause",$1,$3); }
		;



clause		: position expression _in_ expression
		  then inexpression
		  { $$ = newCTerm("fClause",$2,$4,$6,$1); }
		| position expression then position inexpression
		  { $$ = newCTerm("fClause",
				  newCTerm("fSkip",$1),$2,$5,$4); }
		;



xiList		: xi CHOICE xiList1
		  { $$ = consList($1,$3); }
		;

xiList1		: xi
		  { $$ = consList($1,nilAtom); }
		| xiList
		  { $$ = $1; }

		;

xi		: expression _in_ expression then inexpression
		  { $$ = newCTerm("fClause",$1,$3,$5); }
		| expression _in_ expression
		  { $$ = newCTerm("fClause",$1,$3,newCTerm("fNoThen",pos())); }

		| expression then inexpression
		  { $$ = newCTerm("fClause",
				  newCTerm("fSkip",pos()),$1,$3); }
		| expression
		  { $$ = newCTerm("fClause",
				  newCTerm("fSkip",pos()),
				  $1,newCTerm("fNoThen",pos())); }
		;

choiceList	: choicE CHOICE choiceList1
		  { $$ = consList($1,$3); }
		;

choiceList1	: choicE
		  { $$ = consList($1,nilAtom); }
		| choiceList
		  { $$ = $1; }

		;

choicE		: expression _in_ position expression then inexpression
		  { $$ = newCTerm("fClause",$1,$4,$6); }
		| inexpression
		  { $$ = newCTerm("fClause",
				  newCTerm("fSkip",pos()),
			          newCTerm("fSkip",pos()),
				  $1); }
		| expression then inexpression
		  { $$ = newCTerm("fClause",
				  newCTerm("fSkip",pos()),$1,$3); }
		;

condisClauseList
		: condisClause CHOICE condisClauseList1
		  { $$ = consList($1,$3); }
		;

condisClauseList1
		: condisClause
		  { $$ = consList($1,nilAtom); }
		| condisClauseList
		  { $$ = $1; }
		;

condisClause	: fdexpression
		  { $$ = consList($1,nilAtom); }
		| fdexpression condisClause
		  { $$ = consList($1,$2); }
		;

fdexpression	: term fdcompare position term
		  { $$ = newCTerm("fFdCompare",$2,$1,$4,$3); }
		| term fdin position term
		  { $$ = newCTerm("fFdIn",$2,$1,$4,$3); }
		;

createExpression: _class_ position
		  termOpt
		  classDescriptorList
		  createMethList
		  end
		  { $$ = newCTerm("fClass",$3,$4,$5,$2); }
		;

classDescriptorList: /* empty */
		  { $$ = nilAtom; }
		|  classDescriptor classDescriptorList
		  { $$ = consList($1,$2); }

classDescriptor: _from_ position term termList
		  { $$ = newCTerm("fFrom",consList($3,$4),$2); }
		| attr position attrFeat attrFeatList
		  { $$ = newCTerm("fAttr",consList($3,$4),$2); }
		| feat position attrFeat attrFeatList
		  { $$ = newCTerm("fFeat",consList($3,$4),$2); }
		| prop position term termList
		  { $$ = newCTerm("fProp",consList($3,$4),$2); }
		;

termOpt		: term
		  { $$ = $1; }
		| /* empty */
		  { $$ = newCTerm("fDollar",pos()); }
		;


phi		: skip
		  { $$ = newCTerm("fSkip",pos()); }
		| fail
		  { $$ = newCTerm("fFail",pos()); }
		;

position	: /* empty */
		  { $$ = pos(); }
		;

label		: constant
		  { $$ = $1; }
		| variable
		  { $$ = $1; }
		;

termList	:  /* empty */
		  { $$ = nilAtom; }
		|  term termList
		  { $$ = consList($1,$2); }
		;

argumentList	:  /* empty */
		  { $$ = nilAtom; }
		| term argumentList
		  { $$ = consList($1,$2); }
		|  colonPair argumentList
		  { $$ = consList($1,$2); }
		;

colonPair	: feature ':' term
		  { $$ = newCTerm("fColon",$1,$3); }
		;

pred		: proc position '{' term termList '}' inexpression end
		  { $$ = newCTerm("fProc",$4,$5,$7,$2); }
		;

funExpression	: _fun_ position '{' term termList '}' inexpression end
		  { $$ = newCTerm("fFun",$4,$5,$7,$2); }
		;

amper		: AMPER
		  { $$ = makeInt(xytext[0]); }
		;

at		: '@' position termNoHash
		  { $$ = newCTerm("fAt",$3,$2); }
		;

apply		: '{' position term termList '}'
		  { $$ = newCTerm("fApply",$3,$4,$2); }
		;

constant	: OZATOM
		  { $$ = newCTerm("fAtom",newCTerm(xytext),pos()); }
		;

string		: STRING
		  { $$ = makeString(xytext); }
		;

functor		: ATOM_LABEL
		  { $$ =  newCTerm("fAtom",newCTerm(xytext),pos()); }
		| VARIABLE_LABEL
		  { $$ = makeVar(xytext); }
		| UNIT_LABEL
		  { $$ = makeVar("`unit`"); }
		| TRUE_LABEL
		  { $$ = makeVar("`true`"); }
		| FALSE_LABEL
		  { $$ = makeVar("`false`"); }
		;

nakedVariable	: VARIABLE
		  { $$ = makeVar(xytext); }
		;

variable	: nakedVariable
		  { $$ = $1; }
		| '!' position nakedVariable
		  { $$ = newCTerm("fEscape",$3,$2); }
		;

number		: int
		  { $$ = $1; }
		| OZFLOAT
		  { $$ = newCTerm("fFloat",OZ_CStringToFloat(xytext),pos()); }
		;

int		: OZINT
		  { $$ = makeInt(xytext); }
	        | amper
	          { $$ = $1; }
		;

dotint		: DOTINT
		  { $$ = makeInt(xytext); }
		;

attrFeat	: attrFeatFeature ':' term
		  { $$ = pair($1,$3); }
		| attrFeatFeature
		  { $$ = $1; }
		;

attrFeatList	: /* empty */
		  { $$ = nilAtom; }
	        | attrFeat attrFeatList
		  { $$ = consList($1,$2); }
		;

createMeth	: _meth_ position methHead inexpression end
		  { $$ = newCTerm("fMeth",$3,$4,$2); }
		;

methHead	: methHead1
		  { $$ = $1; }
		| methHead1 '=' position nakedVariable
		  { $$ = newCTerm("fEq",$1,$4,$3); }
		;

methHead1	: label
		  { $$ = $1; }
		| methHeadFunctor '(' methHeadArgumentList ')'
		  { $$ = newCTerm("fRecord",$1,$3); }
		| methHeadFunctor '(' methHeadArgumentList LDOTS ')'
		  { $$ = newCTerm("fOpenRecord",$1,$3); }
		;

methHeadFunctor	: ATOM_LABEL
		  { $$ =  newCTerm("fAtom",newCTerm(xytext),pos()); }
		| VARIABLE_LABEL
		  { $$ = makeVar(xytext); }
		| '!' position VARIABLE_LABEL
		  { $$ = newCTerm("fEscape",makeVar(xytext),$2); }
		| UNIT_LABEL
		  { $$ = newCTerm("fEscape",makeVar("`unit`"),pos()); }
		| TRUE_LABEL
		  { $$ = newCTerm("fEscape",makeVar("`true`"),pos()); }
		| FALSE_LABEL
		  { $$ = newCTerm("fEscape",makeVar("`false`"),pos()); }
		;

methHeadArgumentList
		: methHeadTerm methHeadArgumentList
		  { $$ = consList(newCTerm("fMethArg",$1,
					   newCTerm("fNoDefault")),$2); }
		| methHeadColonPair methHeadArgumentList
		  { $$ = consList($1,$2); }
		| methHeadDefaultEquation methHeadArgumentList1
		  { $$ = consList($1,$2); }
		| /* empty */
		  { $$ = nilAtom; }
		;

methHeadArgumentList1
		: /* empty */
		  { $$ = nilAtom; }
		| methHeadDefaultEquation methHeadArgumentList1
		  { $$ = consList($1,$2); }
		| methHeadColonPair methHeadArgumentList1
		  { $$ = consList($1,$2); }
		;

methHeadTerm	: nakedVariable
		  { $$ = $1; }
		| '$'
		  { $$ = newCTerm("fDollar",pos()); }
		| '_'
		  { $$ = newCTerm("fWildcard",pos()); }
		;

methHeadColonPair
		: feature ':' methHeadTerm
		  { $$ = newCTerm("fMethColonArg",$1,$3,
				  newCTerm("fNoDefault")); }
		| feature ':' methHeadTerm DEFAULT position term
		  { $$ = newCTerm("fMethColonArg",$1,$3,
				  newCTerm("fDefault",$6,$5)); }
		;

methHeadDefaultEquation
		: methHeadTerm DEFAULT position term
		  { $$ = newCTerm("fMethArg",$1,newCTerm("fDefault",$4,$3)); }
		;

feature		: constant
		  { $$ = $1; }
		| nakedVariable
		  { $$ = $1; }
		| int
		  { $$ = $1; }
		| unit
		  { $$ = makeVar("`unit`"); }
		| true
		  { $$ = makeVar("`true`"); }
		| false
		  { $$ = makeVar("`false`"); }
		;

attrFeatFeature	: constant
		  { $$ = $1; }
		| variable
		  { $$ = $1; }
		| int
		  { $$ = $1; }
		| unit
		  { $$ = newCTerm("fEscape",makeVar("`unit`"),pos()); }
		| true
		  { $$ = newCTerm("fEscape",makeVar("`true`"),pos()); }
		| false
		  { $$ = newCTerm("fEscape",makeVar("`false`"),pos()); }
		;

createMethList	: /* empty */
		  { $$ = nilAtom; }
		| createMeth createMethList
		  { $$ = consList($1,$2); }
		;

compare		: COMPARE
		  { $$=newCTerm(xytext); }
		;

fdcompare	: FDCOMPARE
		  { $$=newCTerm(xytext); }
		;

fdin		: FDIN
		  { $$=newCTerm(xytext); }
		;

fdmul		: FDMUL
		  { $$=newCTerm(xytext); }
		;

othermul	: OTHERMUL
		  { $$=newCTerm(xytext); }
		;

uminus		: '~'
		  { $$=newCTerm(xytext); }
		;

add		: ADD
		  { $$=newCTerm(xytext); }
		;



scannerSpecification
		: _scanner_ position nakedVariable
		  classDescriptorList createMethList scannerRules end
		  { $$ = newCTerm("fScanner",$3,$4,$5,$6,$2); }
		;

scannerRules	: lexAbbrev
		  { $$ = consList($1,nilAtom); }
		| lexRule
		  { $$ = consList($1,nilAtom); }
		| modeClause
		  { $$ = consList($1,nilAtom); }
		| lexAbbrev scannerRules
		  { $$ = consList($1,$2); }
		| lexRule scannerRules
		  { $$ = consList($1,$2); }
		| modeClause scannerRules
		  { $$ = consList($1,$2); }
		;

lexAbbrev	: lex constant '=' regex end
		  { $$ = newCTerm("fLexicalAbbreviation",$2,$4); }
		| lex nakedVariable '=' regex end
		  { $$ = newCTerm("fLexicalAbbreviation",$2,$4); }
		;

lexRule		: lex regex inexpression end
		  { $$ = newCTerm("fLexicalRule",$2,$3); }
		;

regex		: REGEX
		  { $$ = OZ_string(xytext); }
		;

modeClause	: _mode_ nakedVariable modeDescrs end
		  { $$ = newCTerm("fMode",$2,$3); }
		;

modeDescrs	: /* empty */
		  { $$ = nilAtom; }
		| modeDescr modeDescrs
		  { $$ = consList($1,$2); }
		;

modeDescr	: _from_ modeFromList
		  { $$ = newCTerm("fInheritedModes",$2); }
		| lexRule
		  { $$ = $1; }
		| modeClause
		  { $$ = $1; }
		;



parserSpecification
		: _parser_ position nakedVariable
		  classDescriptorList createMethList
		  tokenClause parserRules end
		  { $$ = newCTerm("fParser",$3,$4,$5,$6,$7,$2); }
		;

parserRules	: synClause
		  { $$ = consList($1,nilAtom); }
		| prodClause
		  { $$ = consList($1,nilAtom); }
		| synClause parserRules
		  { $$ = consList($1,$2); }
		| prodClause parserRules
		  { $$ = consList($1,$2); }
		;

tokenClause	: token tokenList
		  { $$ = newCTerm("fToken",$2); }
		;

tokenList	: tokenDecl
		  { $$ = consList($1,nilAtom); }
		| tokenDecl tokenList
		  { $$ = consList($1,$2); }
		;

tokenDecl	: constant
		  { $$ = $1; }
		| constant ':' term
		  { $$ = pair($1,$3); }
		;

modeFromList	: nakedVariable
		  { $$ = consList($1,nilAtom); }
		| nakedVariable modeFromList
		  { $$ = consList($1,$2); }
		;

prodClause	: prod nakedVariable '='
		  { *prodKey[depth]++ = '='; }
		  prodHeadRest prodMakeKey localRules synAlt end
		  { $$ = newCTerm("fProductionTemplate",$6,$5,$7,$8,$2); }
		| prod '$' { $<t>$ = newCTerm("fDollar",pos()); } '='
		  { *prodKey[depth]++ = '='; }
		  prodHeadRest prodMakeKey localRules synAlt end
		  { $$ = newCTerm("fProductionTemplate",$7,$6,$8,$9,$<t>3); }
		| prod prodHeadRest prodMakeKey localRules synAlt end
		  { $$ = newCTerm("fProductionTemplate",$3,$2,$4,$5,newCTerm("none")); }
		;

prodHeadRest	: prodNameAtom nakedVariable optTerminatorOp
		  { $$ = consList($2,nilAtom); }
		| nakedVariable terminatorOp
		  { $$ = consList($1,nilAtom); }
		| prodName prodKey
		  { $$ = $2; }
		;

prodName	: prodNameAtom
		| /* empty */
		;

prodNameAtom	: constant ':'
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
		  { $$ = consList($1,nilAtom); }
		| prodParam separatorOp prodParams
		  { $$ = consList($1,$3); }
		;

prodParam	: nakedVariable { $$ = $1; }
		| '_' { $$ = newCTerm("fWildcard",pos()); }
		;

separatorOp	: SEP
		  { *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; }
		;

optTerminatorOp	: /* empty */
		| terminatorOp
		;

terminatorOp	: ADD { *prodKey[depth]++ = xytext[0]; }
		| FDMUL { *prodKey[depth]++ = xytext[0]; }
		;

prodMakeKey	: { *prodKey[depth] = '\0';
		    $$ = pair(prodName[depth],OZ_string(prodKeyBuffer[depth]));
		    prodName[depth] = newCTerm("none");
		    prodKey[depth] = prodKeyBuffer[depth];
		  }
		;

localRules	: /* empty */
		  { $$ = nilAtom; }
		| localRulesSub _in_
		  { $$ = $1; }
		;

localRulesSub	: synClause
		  { $$ = consList($1,nilAtom); }
		| synClause localRulesSub
		  { $$ = consList($1,$2); }
		;

synClause	: syn constant synAlt end
		  { $$ = newCTerm("fSyntaxRule",$2,nilAtom,$3); }
		| syn nakedVariable synAlt end
		  { $$ = newCTerm("fSyntaxRule",$2,nilAtom,$3); }
		| syn synFunctor '(' synParams ')' synAlt end
		  { $$ = newCTerm("fSyntaxRule",$2,$4,$6); }
		;

synParams	: /* empty */
		  { $$ = nilAtom; }
		| synParam synParams
		  { $$ = consList($1,$2); }
		;

synParam	: nakedVariable
		  { $$ = $1; }
		| '$' position
		  { $$ = newCTerm("fDollar",pos()); }
		| '_'
		  { $$ = newCTerm("fWildcard",pos()); }
		;

synAlt		: synSeqs
		  { $$ = newCTerm("fSynAlternative", $1); }
		;

synSeqs		: synSeq
		  { $$ = consList($1,nilAtom); }
		| synSeq CHOICE synSeqs
		  { $$ = consList($1,$3); }
		;

synSeq		: nonEmptySeq
		  { CTerm t = $1;
		    while (terms[depth]) {
		      t = consList(newCTerm("fSynApplication", terms[depth]->term, nilAtom), t);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    $$ = newCTerm("fSynSequence", decls[depth], t);
		    decls[depth] = nilAtom;
		  }
		| skip optSynAction
		  { $$ = newCTerm("fSynSequence", nilAtom, $2); }
		;

optSynAction	: /* empty */
		  { $$ = nilAtom; }
		| REDUCE inexpression
		  { $$ = consList(newCTerm("fSynAction",$2),nilAtom); }
		;

nonEmptySeq	: synVariable nonEmptySeq
		  { $$ = $2; }
		| synVariable terminatorOp position synPrims prodMakeKey
		  { $$ = consList(newCTerm("fSynTemplateInstantiation", $5,
					   consList(newCTerm("fSynApplication",
							     terms[depth]->term,
							     nilAtom),
						    nilAtom),
					   $3),
				  $4);
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  }
		| synVariable '=' synPrimNoAssign synPrims
		  { $$ = consList(newCTerm("fSynAssignment", terms[depth]->term, $3),
				  $4);
		    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		  }
		| _in_ synPrims
		  { while (terms[depth]) {
		      decls[depth] = consList(terms[depth]->term, decls[depth]);
		      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
		    }
		    $$ = $2;
		  }
		| synPrimNoVar synPrims
		  { $$ = consList($1,$2); }
		| optSynAction
		  { $$ = $1; }
		;

synVariable	: nakedVariable
		  { terms[depth] = new TermNode($1, terms[depth]); }
		;

synPrims	: optSynAction
		  { $$ = $1; }
		| synPrim synPrims
		  { $$ = consList($1,$2); }
		;

synPrim		: variable '=' synPrimNoAssign
		  { $$ = newCTerm("fSynAssignment",$1,$3); }
		| synPrimNoAssign
		  { $$ = $1; }
		;

synPrimNoAssign	: nakedVariable
		  { $$ = newCTerm("fSynApplication",$1,nilAtom); }
		| nakedVariable terminatorOp position prodMakeKey
		  { $$ = newCTerm("fSynTemplateInstantiation",$4,
				  consList(newCTerm("fSynApplication",$1,
						    nilAtom),
					   nilAtom),$3);
		  }
		| synPrimNoVarNoAssign
		  { $$ = $1; }
		;

synPrimNoVar	: '!' position nakedVariable '=' synPrimNoAssign
		  { $$ = newCTerm("fSynAssignment",
				  newCTerm("fEscape",$3,$2),$5); }
		| synPrimNoVarNoAssign
		  { $$ = $1; }
		;

synPrimNoVarNoAssign
		: synInstTerm
		  { $$ = $1; }
		| prodNameAtom position synInstTerm optTerminatorOp prodMakeKey
		  { $$ = newCTerm("fSynTemplateInstantiation",$5,
				  consList($3,nilAtom),$2);
		  }
		| synInstTerm terminatorOp position prodMakeKey
		  { $$ = newCTerm("fSynTemplateInstantiation",$4,
				  consList($1,nilAtom),$3);
		  }
		| prodName position '(' { *prodKey[depth]++ = '('; depth++; }
		  synProdCallParams ')' { depth--; }
		  optTerminatorOp prodMakeKey
		  { $$ = newCTerm("fSynTemplateInstantiation",$9,$5,$2); }
		| prodName position '[' { *prodKey[depth]++ = '['; depth++; }
		  synProdCallParams ']' { depth--; }
		  optTerminatorOp prodMakeKey
		  { $$ = newCTerm("fSynTemplateInstantiation",$9,$5,$2); }
		| prodName position '{' { *prodKey[depth]++ = '{'; depth++; }
		  synProdCallParams '}' { depth--; }
		  optTerminatorOp prodMakeKey
		  { $$ = newCTerm("fSynTemplateInstantiation",$9,$5,$2); }
		;

synInstTerm	: constant
		  { $$ = newCTerm("fSynApplication",$1,nilAtom); }
		| synFunctor position '(' termList ')'
		  { $$ = newCTerm("fSynApplication",$1,$4); }
		;

synFunctor	: ATOM_LABEL
		  { $$ = newCTerm("fAtom",newCTerm(xytext),pos()); }
		| VARIABLE_LABEL
		  { $$ = makeVar(xytext); }
		;

synProdCallParams
		: synAlt
		  { $$ = consList($1,nilAtom); }
		| synAlt separatorOp synProdCallParams
		  { $$ = consList($1,$3); }
		;

%%

/* position file pointer at the beginning of the <line>-nth line */
static void seekLine(FILE *stream, int line) {
  int ch;
  while(line > 1) {
    ch = getc(stream);
    if (ch == EOF)
      return;
    if (ch == '\n')
      line--;
  }
}

int xyreportError(char *kind, char *msg, char *file, int line, int offset) {
  int TabSpaces = 8;        /* definition of the width of a tabspace */
  int TabCount = 0;
  FILE *pFile;
  int c;

  if (strcmp(kind,"warning"))
    yynerrs++;

  fprintf(stderr, "\n%%************ %s **********\n", kind);
  fprintf(stderr, "%%**\n%%**\t%s\n%c", msg, MSG_ERROR);
  if (line < 0)
    return 1;

  fprintf(stderr, "%%**\tin file \"%s\", line %d, column %d",
	  file, line, offset);

  if (!(pFile = fopen(file,"r"))) {  /* open file */
    fprintf(stderr, "\n%%**\n");
    return 1;
  }

  seekLine(pFile, line);        /* go to the position */

  fprintf(stderr, ":\n%%**\n%%**\t");
  do {				/* print the line (including '\n') */
    c = getc(pFile);
    if (c == EOF)
      putc('\n', stderr);
    else if (c == '\t') {	/* print tabs explicitly */
      TabCount++;		
      putc(c, stderr);
    } else
      putc(c, stderr);
  } while (c != '\n' &&  c != EOF);
  fclose(pFile);
  fprintf(stderr, "%%**\t");
  for (int i = 0; i < (offset - 1) + (TabSpaces - 1) * TabCount; i++)
    putc(' ',stderr);
  fprintf(stderr, "^-- *** here\n%%**\n");
  return 1;
}

static void xyerror(char *s) {
  char *news;
  if (strlen(s) > 13)
    news = s + 13;
  else
    news = "";
  xyreportError("parse error", news, xyFileName, xylino, xycharno());
}


static void parserInit() {
  xyscannerInit();
  for (int i = 0; i < DEPTH; i++)
    terms[i] = 0;
}

static CTerm parse() {
  // in case there was a syntax error during the last parse, delete garbage:
  for (int i = 0; i < DEPTH; i++) {
    prodKey[i] = prodKeyBuffer[i];
    prodName[i] = newCTerm("none");
    while (terms[i]) {
      TermNode *tmp = terms[i]; terms[i] = terms[i]->next; delete tmp;
    }
    decls[i] = nilAtom;
  }
  depth = 0;

  yyoutput = 0;
  xyparse();
  xy_exit();

  if (yyoutput == 0)
    yyoutput = newCTerm("parseError");

  return yyoutput;
}

static CTerm parseFile(char *file) {
  if (!xy_init_from_file(file))
    return newCTerm("fileNotFound");
  strncpy(xyFileName,file,99);
  xyFileNameAtom = OZ_atom(xyFileName);
  CTerm res = parse();
  fclose(xyin);
  return res;
}

static CTerm parseVirtualString(char *str) {
  strcpy(xyFileName,"/");
  xyFileNameAtom = OZ_atom(xyFileName);
  xy_init_from_string(str);
  return parse();
}
