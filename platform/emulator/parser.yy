%{
///  Programming Systems Lab,
///  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5609
///  Original Author: Martin Henz
///  Extensive modifications by Leif Kornstaedt <kornstae@ps.uni-sb.de>

//
// See Oz/tools/compiler/Doc/TupleSyntax for an description of the
// generated parse trees.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define YYDEBUG 1

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

extern int xy_showInsert, xy_gumpSyntax, xy_systemVariables;
extern CTerm xy_errorMessages;

OZ_C_proc_begin(ozparser_init, 0)
{
  nilAtom = OZ_nil();
  xy_showInsert = xy_gumpSyntax = 0;
  xy_systemVariables = 1;
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
  xy_errorMessages = OZ_nil();
  CTerm res = parseFile(str);
  printf("%s",OZ_virtualStringToC(xy_errorMessages));
  return OZ_unify(OZ_getCArg(1), res);
}
OZ_C_proc_end

OZ_C_proc_begin(ozparser_parseVirtualString, 2)
{
  OZ_declareVirtualStringArg(0, str);
  xy_errorMessages = OZ_nil();
  CTerm res = parseVirtualString(str);
  printf("%s",OZ_virtualStringToC(xy_errorMessages));
  return OZ_unify(OZ_getCArg(1), res);
}
OZ_C_proc_end

OZ_C_proc_begin(ozparser_parseFileAtomic, 6)
{
  // {ParseFile FileName ShowInsert GumpSyntax SystemVariables ?AST ?VS}
  OZ_declareVirtualStringArg(0, str);
  OZ_declareNonvarArg(1, showInsert);
  OZ_declareNonvarArg(2, gumpSyntax);
  OZ_declareNonvarArg(3, systemVariables);
  xy_showInsert = OZ_isTrue(showInsert);
  xy_gumpSyntax = OZ_isTrue(gumpSyntax);
  xy_systemVariables = OZ_isTrue(systemVariables);

  xy_errorMessages = OZ_nil();
  OZ_Return res = OZ_unify(OZ_getCArg(4), parseFile(str));
  if (res == PROCEED)
    return OZ_unify(OZ_getCArg(5), xy_errorMessages);
  else
    return res;
}
OZ_C_proc_end

OZ_C_proc_begin(ozparser_parseVirtualStringAtomic, 6)
{
  // {ParseVirtualString VS1 ShowInsert GumpSyntax SystemVariables ?AST ?VS2}
  OZ_declareVirtualStringArg(0, str);
  OZ_declareNonvarArg(1, showInsert);
  OZ_declareNonvarArg(2, gumpSyntax);
  OZ_declareNonvarArg(3, systemVariables);
  xy_showInsert = OZ_isTrue(showInsert);
  xy_gumpSyntax = OZ_isTrue(gumpSyntax);
  xy_systemVariables = OZ_isTrue(systemVariables);

  xy_errorMessages = OZ_nil();
  OZ_Return res = OZ_unify(OZ_getCArg(4), parseVirtualString(str));
  if (res == PROCEED)
    return OZ_unify(OZ_getCArg(5), xy_errorMessages);
  else
    return res;
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

#define pair(left, right)       OZ_pair2(left, right)
#define consList(head, tail)    OZ_cons(head, tail)

#define YYMAXDEPTH 1000000
#define YYERROR_VERBOSE


//----------------------
// Interface to Scanner
//----------------------

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
extern "C" void xyreportError(char *, char *, char *, int ,int);

static inline int xycharno() {
  int n = xytext - xylastline;
  if (n > 0)
    return n;
  else
    return 0;
}


//----------------------
// Operations on CTerms
//----------------------

static CTerm yyoutput;

inline CTerm pos() {
  return newCTerm("pos",xyFileNameAtom,OZ_int(xylino),OZ_int(xycharno()));
}

inline CTerm makeVar(char *printName) {
  return newCTerm("fVar",newCTerm(printName),pos());
}

inline CTerm makeCons(CTerm first, CTerm second, CTerm pos) {
   return newCTerm("fRecord",
                   newCTerm("fAtom",newCTerm("|"),pos),
                   consList(first,consList(second,nilAtom)));
}

static CTerm makeInt(char *chars, CTerm pos) {
  return newCTerm("fInt",OZ_CStringToInt(chars),pos);
}

static CTerm makeInt(char c, CTerm pos) {
  return newCTerm("fInt",OZ_int((unsigned char) c),pos);
}

static CTerm makeString(char *chars, CTerm pos) {
  if (chars[0] == '\0')
    return newCTerm("fAtom",newCTerm("nil"),pos);
  else
    return makeCons(makeInt(chars[0],pos),makeString(&chars[1],pos),pos);
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

%token HELP SWITCH SHOWSWITCHES FEED THREADEDFEED
%token CORE MACHINE TOPVARS
%token SWITCHNAME FILENAME
%token OZATOM ATOM_LABEL OZFLOAT OZINT AMPER DOTINT STRING
%token VARIABLE VARIABLE_LABEL
%token DEFAULT CHOICE LDOTS OBJPATTERNOPEN OBJPATTERNCLOSE
%token attr _case_ catch choice _class_ _condis_ declare dis
%token _else_ elsecase elseif elseof end fail false FALSE_LABEL
%token feat finally _from_ _fun_ _if_ _in_ local _lock_ _meth_
%token not of or proc prop raise self skip then
%token thread true TRUE_LABEL try unit UNIT_LABEL with

%token ENDOFFILE

%token REGEX lex _mode_ _parser_ prod _scanner_ syn token
%token REDUCE SEP

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
%type <t>  label
%type <t>  recordArguments
%type <t>  feature
%type <t>  caseMain
%type <t>  caseRest
%type <t>  elseOfList
%type <t>  caseClauseList
%type <t>  caseClause
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
%type <t>  methHeadArgumentList
%type <t>  methHeadArgumentList1
%type <t>  methHeadTerm
%type <t>  methHeadColonPair
%type <t>  methHeadDefaultEquation
%type <t>  ifMain
%type <t>  ifRest
%type <t>  ifClauseList
%type <t>  ifClause
%type <t>  condisClauseList
%type <t>  condisClause
%type <t>  fdExpression
%type <t>  orClauseList
%type <t>  orClause
%type <t>  choiceClauseList
%type <t>  choiceClause
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

%%

file            : queries ENDOFFILE
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

queries         : sequence queries1
                  { $$ = consList($1,$2); }
                | queries1
                  { $$ = $1; }
                ;

queries1        : directive queries
                  { $$ = consList($1,$2); }
                | declare coord sequence _in_ sequence queries1
                  { $$ = consList(newCTerm("fDeclare",$3,$5,$2),$6); }
                | declare coord sequence thisCoord queries1
                  { $$ = consList(newCTerm("fDeclare",$3,
                                           newCTerm("fSkip",$4),$2),$5); }
                | /* empty */
                  { $$ = nilAtom; }
                ;

directive       : HELP
                  { $$ = newCTerm("dirHelp"); }
                | SWITCH switchList
                  { $$ = newCTerm("dirSwitch",$2); }
                | SHOWSWITCHES
                  { $$ = newCTerm("dirShowSwitches"); }
                | FEED FILENAME
                  { $$ = newCTerm("dirFeed",newCTerm(xyhelpFileName)); }
                | THREADEDFEED FILENAME
                  { $$ = newCTerm("dirThreadedFeed",newCTerm(xyhelpFileName)); }
                | CORE FILENAME
                  { $$ = newCTerm("dirCore",newCTerm(xyhelpFileName)); }
                | MACHINE FILENAME
                  { $$ = newCTerm("dirMachine",newCTerm(xyhelpFileName)); }
                | TOPVARS FILENAME
                  { $$ = newCTerm("dirTopVars",newCTerm(xyhelpFileName)); }
                ;

switchList      : /* empty */
                  { $$ = nilAtom; }
                | switch switchList
                  { $$ = consList($1,$2); }
                ;

switch          : '+' SWITCHNAME
                  { $$ = newCTerm("on",newCTerm(xytext),pos()); }
                | '-' SWITCHNAME
                  { $$ = newCTerm("off",newCTerm(xytext),pos()); }
                ;

sequence        : phrase
                  { $$ = $1; }
                | phrase sequence
                  { $$ = newCTerm("fAnd",$1,$2); }
                ;

phrase          : phrase '=' coord phrase
                  { $$ = newCTerm("fEq",$1,$4,$3); }
                | phrase ASSIGN coord phrase
                  { $$ = newCTerm("fAssign",$1,$4,$3); }
                | phrase orelse coord phrase
                  { $$ = newCTerm("fOrElse",$1,$4,$3); }
                | phrase andthen coord phrase
                  { $$ = newCTerm("fAndThen",$1,$4,$3); }
                | phrase compare coord phrase %prec COMPARE
                  { $$ = newCTerm("fOpApply",$2,
                                  consList($1,consList($4,nilAtom)),$3); }
                | phrase fdCompare coord phrase %prec FDIN
                  { $$ = newCTerm("fFdCompare",$2,$1,$4,$3); }
                | phrase fdIn coord phrase %prec FDIN
                  { $$ = newCTerm("fFdIn",$2,$1,$4,$3); }
                | phrase '|' coord phrase
                  { $$ = makeCons($1,$4,$3); }
                | phrase2
                  { $$ = $1; }
                | phrase2 '#' coord hashes
                  { $$ = newCTerm("fRecord",
                                  newCTerm("fAtom",newCTerm("#"),$3),
                                  consList($1,$4)); }
                ;

hashes          : phrase2
                  { $$ = consList($1,nilAtom); }
                | phrase2 '#' hashes
                  { $$ = consList($1,$3); }
                ;

phrase2         : phrase2 add coord phrase2 %prec ADD
                  { $$ = newCTerm("fOpApply",$2,
                                  consList($1,consList($4,nilAtom)),$3); }
                | phrase2 fdMul coord phrase2 %prec FDMUL
                  { $$ = newCTerm("fOpApply",$2,
                                  consList($1,consList($4,nilAtom)),$3); }
                | phrase2 otherMul coord phrase2 %prec OTHERMUL
                  { $$ = newCTerm("fOpApply",$2,
                                  consList($1,consList($4,nilAtom)),$3); }
                | phrase2 ',' coord phrase2
                  { $$ = newCTerm("fObjApply",$1,$4,$3); }
                | '~' coord phrase2 %prec '~'
                  { $$ = newCTerm("fOpApply",newCTerm("~"),
                                  consList($3,nilAtom),$2); }
                | phrase2 '.' coord phrase2
                  { $$ = newCTerm("fOpApply",newCTerm("."),
                                  consList($1,consList($4,nilAtom)),$3); }
                | phrase2 DOTINT
                  { $$ = newCTerm("fOpApply",newCTerm("."),
                                  consList($1,consList(makeInt(xytext,pos()),
                                                       nilAtom)),pos()); }
                | phrase2 '^' coord phrase2
                  { $$ = newCTerm("fOpApply",newCTerm("^"),
                                  consList($1,consList($4,nilAtom)),$3); }
                | '@' coord phrase2
                  { $$ = newCTerm("fAt",$3,$2); }
                | '(' inSequence ')'
                  { $$ = $2; }
                | atom
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
                | int
                  { $$ = $1; }
                | float
                  { $$ = $1; }
                | label '(' recordArguments ')'
                  { $$ = newCTerm("fRecord",$1,$3); }
                | label '(' recordArguments LDOTS ')'
                  { $$ = newCTerm("fOpenRecord",$1,$3); }
                | label OBJPATTERNOPEN recordArguments OBJPATTERNCLOSE
                  { $$ = newCTerm("fObjPattern",$1,$3); }
                | '[' fixedListArgs ']'
                  { $$ = $2; }
                | '{' coord phrase phraseList '}'
                  { $$ = newCTerm("fApply",$3,$4,$2); }
                | proc coord '{' phrase phraseList '}' inSequence end
                  { $$ = newCTerm("fProc",$4,$5,$7,$2); }
                | _fun_ coord '{' phrase phraseList '}' inSequence end
                  { $$ = newCTerm("fFun",$4,$5,$7,$2); }
                | class
                  { $$ = $1; }
                | local coord sequence _in_ sequence end
                  { $$ = newCTerm("fLocal",$3,$5,$2); }
                | _case_ caseMain
                  { $$ = $2; }
                | _lock_ coord inSequence end
                  { $$ = newCTerm("fLock",$3,$2); }
                | _lock_ coord phrase then inSequence end
                  { $$ = newCTerm("fLockThen",$3,$5,$2); }
                | thread coord inSequence end
                  { $$ = newCTerm("fThread",$3,$2); }
                | try coord inSequence optCatch optFinally end
                  { $$ = newCTerm("fTry",$3,$4,$5,$2); }
                | raise coord inSequence end
                  { $$ = newCTerm("fRaise",$3,$2); }
                | raise coord inSequence with inSequence end
                  { $$ = newCTerm("fRaiseWith",$3,$5,$2); }
                | skip
                  { $$ = newCTerm("fSkip",pos()); }
                | fail
                  { $$ = newCTerm("fFail",pos()); }
                | not coord inSequence end
                  { $$ = newCTerm("fNot",$3,$2); }
                | _if_ ifMain
                  { $$ = $2; }
                | or coord orClauseList end
                  { $$ = newCTerm("fOr",$3,newCTerm("for"),$2); }
                | dis coord orClauseList end
                  { $$ = newCTerm("fOr",$3,newCTerm("fdis"),$2); }
                | choice coord choiceClauseList end
                  { $$ = newCTerm("fOr",$3,newCTerm("fchoice"),$2); }
                | _condis_ coord condisClauseList end
                  { $$ = newCTerm("fCondis",$3,$2); }
                | scannerSpecification
                  { $$ = $1; }
                | parserSpecification
                  { $$ = $1; }
                ;

compare         : COMPARE
                  { $$ = newCTerm(xytext); }
                ;

fdCompare       : FDCOMPARE
                  { $$ = newCTerm(xytext); }
                ;

fdIn            : FDIN
                  { $$ = newCTerm(xytext); }
                ;

add             : ADD
                  { $$=newCTerm(xytext); }
                ;

fdMul           : FDMUL
                  { $$=newCTerm(xytext); }
                ;

otherMul        : OTHERMUL
                  { $$=newCTerm(xytext); }
                ;

inSequence      : sequence _in_ coord sequence
                  { $$ = newCTerm("fLocal",$1,$4,$3); }
                | sequence
                  { $$ = $1; }
                ;

phraseList      : /* empty */
                  { $$ = nilAtom; }
                | phrase phraseList
                  { $$ = consList($1,$2); }
                ;

fixedListArgs   : thisCoord phrase
                  { $$ = newCTerm("fRecord",
                                  newCTerm("fAtom",newCTerm("|"),$1),
                                  consList($2,consList(newCTerm("fAtom",
                                                                newCTerm("nil"),
                                                                $1),
                                                       nilAtom))); }
                | thisCoord phrase fixedListArgs
                  { $$ = newCTerm("fRecord",
                                  newCTerm("fAtom",newCTerm("|"),$1),
                                  consList($2,consList($3,nilAtom))); }
                ;

optCatch        : /* empty */
                  { $$ = newCTerm("fNoCatch"); }
                | catch caseClauseList
                  { $$ = $2; }
                ;

optFinally      : /* empty */
                  { $$ = newCTerm("fNoFinally"); }
                | finally inSequence
                  { $$ = $2; }
                ;

label           : ATOM_LABEL
                  { $$ = newCTerm("fAtom",newCTerm(xytext),pos()); }
                | VARIABLE_LABEL
                  { $$ = makeVar(xytext); }
                | UNIT_LABEL
                  { $$ = makeVar("`unit`"); }
                | TRUE_LABEL
                  { $$ = makeVar("`true`"); }
                | FALSE_LABEL
                  { $$ = makeVar("`false`"); }
                ;

recordArguments : /* empty */
                  { $$ = nilAtom; }
                | phrase recordArguments
                  { $$ = consList($1,$2); }
                | feature ':' phrase recordArguments
                  { $$ = consList(newCTerm("fColon",$1,$3),$4); }
                ;

feature         : atom
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

caseMain        : coord sequence then inSequence caseRest
                  { $$ = newCTerm("fBoolCase",$2,$4,$5,$1); }
                | coord sequence of elseOfList caseRest
                  { $$ = newCTerm("fCase",$2,$4,$5,$1); }
                ;

caseRest        : elsecase caseMain
                  { $$ = $2; }
                | _else_ inSequence end
                  { $$ = $2; }
                | end
                  { $$ = newCTerm("fNoElse",pos()); }
                ;

elseOfList      : caseClauseList
                  { $$ = consList($1,nilAtom); }
                | caseClauseList elseof elseOfList
                  { $$ = consList($1,$3); }
                ;

caseClauseList  : caseClause
                  { $$ = consList($1,nilAtom); }
                | caseClause CHOICE caseClauseList
                  { $$ = consList($1,$3); }
                ;

caseClause      : inSequence then inSequence
                  { $$ = newCTerm("fCaseClause",$1,$3); }
                ;

class           : _class_ coord phraseOpt classDescriptorList methList end
                  { $$ = newCTerm("fClass",$3,$4,$5,$2); }
                ;

phraseOpt       : phrase
                  { $$ = $1; }
                | thisCoord
                  { $$ = newCTerm("fDollar",$1); }
                ;

classDescriptorList
                : /* empty */
                  { $$ = nilAtom; }
                |  classDescriptor classDescriptorList
                  { $$ = consList($1,$2); }
                ;

classDescriptor : _from_ coord phrase phraseList
                  { $$ = newCTerm("fFrom",consList($3,$4),$2); }
                | attr coord attrFeat attrFeatList
                  { $$ = newCTerm("fAttr",consList($3,$4),$2); }
                | feat coord attrFeat attrFeatList
                  { $$ = newCTerm("fFeat",consList($3,$4),$2); }
                | prop coord phrase phraseList
                  { $$ = newCTerm("fProp",consList($3,$4),$2); }
                ;

attrFeatList    : /* empty */
                  { $$ = nilAtom; }
                | attrFeat attrFeatList
                  { $$ = consList($1,$2); }
                ;

attrFeat        : attrFeatFeature ':' phrase
                  { $$ = pair($1,$3); }
                | attrFeatFeature
                  { $$ = $1; }
                ;

attrFeatFeature : atom
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

methList        : /* empty */
                  { $$ = nilAtom; }
                | meth methList
                  { $$ = consList($1,$2); }
                ;

meth            : _meth_ coord methHead inSequence end
                  { $$ = newCTerm("fMeth",$3,$4,$2); }
                ;

methHead        : methHead1
                  { $$ = $1; }
                | methHead1 '=' coord nakedVariable
                  { $$ = newCTerm("fEq",$1,$4,$3); }
                ;

methHead1       : atom
                  { $$ = $1; }
                | variable
                  { $$ = $1; }
                | unit
                  { $$ = newCTerm("fEscape",makeVar("`unit`"),pos()); }
                | true
                  { $$ = newCTerm("fEscape",makeVar("`true`"),pos()); }
                | false
                  { $$ = newCTerm("fEscape",makeVar("`false`"),pos()); }
                | methHeadLabel '(' methHeadArgumentList ')'
                  { $$ = newCTerm("fRecord",$1,$3); }
                | methHeadLabel '(' methHeadArgumentList LDOTS ')'
                  { $$ = newCTerm("fOpenRecord",$1,$3); }
                ;

methHeadLabel   : ATOM_LABEL
                  { $$ = newCTerm("fAtom",newCTerm(xytext),pos()); }
                | VARIABLE_LABEL
                  { $$ = makeVar(xytext); }
                | '!' coord VARIABLE_LABEL
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
                : methHeadDefaultEquation methHeadArgumentList1
                  { $$ = consList($1,$2); }
                | methHeadColonPair methHeadArgumentList1
                  { $$ = consList($1,$2); }
                | /* empty */
                  { $$ = nilAtom; }
                ;

methHeadTerm    : nakedVariable
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
                | feature ':' methHeadTerm DEFAULT coord phrase
                  { $$ = newCTerm("fMethColonArg",$1,$3,
                                  newCTerm("fDefault",$6,$5)); }
                ;

methHeadDefaultEquation
                : methHeadTerm DEFAULT coord phrase
                  { $$ = newCTerm("fMethArg",$1,newCTerm("fDefault",$4,$3)); }
                ;

ifMain          : coord ifClauseList ifRest
                  { $$ = newCTerm("fIf",$2,$3,$1); }
                ;

ifRest          : elseif ifMain
                  { $$ = $2; }
                | _else_ inSequence end
                  { $$ = $2; }
                | end
                  { $$ = newCTerm("fNoElse",pos()); }
                ;

ifClauseList    : ifClause
                  { $$ = consList($1,nilAtom); }
                | ifClause CHOICE ifClauseList
                  { $$ = consList($1,$3); }
                ;

ifClause        : sequence then coord inSequence
                  { $$ = newCTerm("fClause",newCTerm("fSkip",$3),$1,$4); }
                | sequence _in_ sequence then inSequence
                  { $$ = newCTerm("fClause",$1,$3,$5); }
                ;

condisClauseList: condisClause CHOICE condisClause
                  { $$ = consList($1,consList($3,nilAtom)); }
                | condisClause CHOICE condisClauseList
                  { $$ = consList($1,$3); }
                ;

condisClause    : fdExpression
                  { $$ = consList($1,nilAtom); }
                | fdExpression condisClause
                  { $$ = consList($1,$2); }
                ;

fdExpression    : phrase fdCompare coord phrase
                  { $$ = newCTerm("fFdCompare",$2,$1,$4,$3); }
                | phrase fdIn coord phrase
                  { $$ = newCTerm("fFdIn",$2,$1,$4,$3); }
                ;

orClauseList    : orClause CHOICE orClause
                  { $$ = consList($1,consList($3,nilAtom)); }
                | orClause CHOICE orClauseList
                  { $$ = consList($1,$3); }
                ;

orClause        : sequence thisCoord
                  { $$ = newCTerm("fClause",
                                  newCTerm("fSkip",$2),
                                  $1,newCTerm("fNoThen",$2)); }
                | sequence _in_ sequence thisCoord
                  { $$ = newCTerm("fClause",$1,$3,newCTerm("fNoThen",$4)); }
                | sequence thisCoord then inSequence
                  { $$ = newCTerm("fClause",
                                  newCTerm("fSkip",$2),$1,$4); }
                | sequence _in_ sequence then inSequence
                  { $$ = newCTerm("fClause",$1,$3,$5); }
                ;

choiceClauseList: choiceClause
                  { $$ = consList($1,nilAtom); }
                | choiceClause CHOICE choiceClauseList
                  { $$ = consList($1,$3); }
                ;

choiceClause    : sequence thisCoord
                  { $$ = newCTerm("fClause",
                                  newCTerm("fSkip",$2),
                                  newCTerm("fSkip",$2),
                                  $1); }
                | sequence thisCoord _in_ sequence
                  { $$ = newCTerm("fClause",
                                  newCTerm("fSkip",$2),
                                  newCTerm("fSkip",$2),
                                  newCTerm("fLocal",$1,$4,$2)); }
                | sequence thisCoord then inSequence
                  { $$ = newCTerm("fClause",
                                  newCTerm("fSkip",$2),$1,$4); }
                | sequence thisCoord _in_ sequence then inSequence
                  { $$ = newCTerm("fClause",$1,$4,$6); }
                ;

atom            : OZATOM
                  { $$ = newCTerm("fAtom",newCTerm(xytext),pos()); }
                ;

nakedVariable   : VARIABLE
                  { $$ = makeVar(xytext); }
                ;

variable        : nakedVariable
                  { $$ = $1; }
                | '!' coord nakedVariable
                  { $$ = newCTerm("fEscape",$3,$2); }
                ;

string          : STRING
                  { $$ = makeString(xytext,pos()); }
                ;

int             : OZINT
                  { $$ = makeInt(xytext,pos()); }
                | AMPER
                  { $$ = makeInt(xytext[0],pos()); }
                ;

float           : OZFLOAT
                  { $$ = newCTerm("fFloat",OZ_CStringToFloat(xytext),pos()); }
                ;

thisCoord       : /* empty */
                  { $$ = pos(); }   /*--** should be: coords of next token */
                ;

coord           : /* empty */
                  { $$ = pos(); }
                ;


/*--------------------------------------------------------------------*/
/* Gump Extensions                                                    */
/*--------------------------------------------------------------------*/

scannerSpecification
                : _scanner_ coord nakedVariable
                  classDescriptorList methList scannerRules end
                  { $$ = newCTerm("fScanner",$3,$4,$5,$6,$2); }
                ;

scannerRules    : lexAbbrev
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

lexAbbrev       : lex atom '=' regex end
                  { $$ = newCTerm("fLexicalAbbreviation",$2,$4); }
                | lex nakedVariable '=' regex end
                  { $$ = newCTerm("fLexicalAbbreviation",$2,$4); }
                ;

lexRule         : lex regex inSequence end
                  { $$ = newCTerm("fLexicalRule",$2,$3); }
                ;

regex           : REGEX
                  { $$ = OZ_string(xytext); }
                ;

modeClause      : _mode_ nakedVariable modeDescrs end
                  { $$ = newCTerm("fMode",$2,$3); }
                ;

modeDescrs      : /* empty */
                  { $$ = nilAtom; }
                | modeDescr modeDescrs
                  { $$ = consList($1,$2); }
                ;

modeDescr       : _from_ modeFromList
                  { $$ = newCTerm("fInheritedModes",$2); }
                | lexRule
                  { $$ = $1; }
                | modeClause
                  { $$ = $1; }
                ;


parserSpecification
                : _parser_ coord nakedVariable
                  classDescriptorList methList
                  tokenClause parserRules end
                  { $$ = newCTerm("fParser",$3,$4,$5,$6,$7,$2); }
                ;

parserRules     : synClause
                  { $$ = consList($1,nilAtom); }
                | prodClause
                  { $$ = consList($1,nilAtom); }
                | synClause parserRules
                  { $$ = consList($1,$2); }
                | prodClause parserRules
                  { $$ = consList($1,$2); }
                ;

tokenClause     : token tokenList
                  { $$ = newCTerm("fToken",$2); }
                ;

tokenList       : tokenDecl
                  { $$ = consList($1,nilAtom); }
                | tokenDecl tokenList
                  { $$ = consList($1,$2); }
                ;

tokenDecl       : atom
                  { $$ = $1; }
                | atom ':' phrase
                  { $$ = pair($1,$3); }
                ;

modeFromList    : nakedVariable
                  { $$ = consList($1,nilAtom); }
                | nakedVariable modeFromList
                  { $$ = consList($1,$2); }
                ;

prodClauseList  : prodClause
                  { $$ = consList($1,nilAtom); }
                | prodClause prodClauseList
                  { $$ = consList($1,$2); }
                ;

prodClause      : prod nakedVariable '='
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

prodHeadRest    : prodNameAtom nakedVariable optTerminatorOp
                  { $$ = consList($2,nilAtom); }
                | nakedVariable terminatorOp
                  { $$ = consList($1,nilAtom); }
                | prodName prodKey
                  { $$ = $2; }
                ;

prodName        : prodNameAtom
                | /* empty */
                ;

prodNameAtom    : atom ':'
                  { prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg($1,0))); }
                ;

prodKey         : '(' { *prodKey[depth]++ = '('; depth++; }
                  prodParams ')' { depth--; } optTerminatorOp { $$ = $3; }
                | '[' { *prodKey[depth]++ = '['; depth++; }
                  prodParams ']' { depth--; } optTerminatorOp { $$ = $3; }
                | '{' { *prodKey[depth]++ = '{'; depth++; }
                  prodParams '}' { depth--; } optTerminatorOp { $$ = $3; }
                ;

prodParams      : prodParam
                  { $$ = consList($1,nilAtom); }
                | prodParam separatorOp prodParams
                  { $$ = consList($1,$3); }
                ;

prodParam       : nakedVariable { $$ = $1; }
                | '_' { $$ = newCTerm("fWildcard",pos()); }
                ;

separatorOp     : SEP
                  { *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; }
                ;

optTerminatorOp : /* empty */
                | terminatorOp
                ;

terminatorOp    : ADD { *prodKey[depth]++ = xytext[0]; }
                | FDMUL { *prodKey[depth]++ = xytext[0]; }
                ;

prodMakeKey     : /* empty */
                  { *prodKey[depth] = '\0';
                    $$ = pair(prodName[depth],OZ_string(prodKeyBuffer[depth]));
                    prodName[depth] = newCTerm("none");
                    prodKey[depth] = prodKeyBuffer[depth];
                  }
                ;

localRules      : /* empty */
                  { $$ = nilAtom; }
                | localRulesSub _in_
                  { $$ = $1; }
                ;

localRulesSub   : synClause
                  { $$ = consList($1,nilAtom); }
                | synClause localRulesSub
                  { $$ = consList($1,$2); }
                ;

synClause       : syn atom synAlt end
                  { $$ = newCTerm("fSyntaxRule",$2,nilAtom,$3); }
                | syn nakedVariable synAlt end
                  { $$ = newCTerm("fSyntaxRule",$2,nilAtom,$3); }
                | syn synLabel '(' synParams ')' synAlt end
                  { $$ = newCTerm("fSyntaxRule",$2,$4,$6); }
                ;

synParams       : /* empty */
                  { $$ = nilAtom; }
                | synParam synParams
                  { $$ = consList($1,$2); }
                ;

synParam        : nakedVariable
                  { $$ = $1; }
                | '$'
                  { $$ = newCTerm("fDollar",pos()); }
                | '_'
                  { $$ = newCTerm("fWildcard",pos()); }
                ;

synAlt          : synSeqs
                  { $$ = newCTerm("fSynAlternative", $1); }
                ;

synSeqs         : synSeq
                  { $$ = consList($1,nilAtom); }
                | synSeq CHOICE synSeqs
                  { $$ = consList($1,$3); }
                ;

synSeq          : nonEmptySeq
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

optSynAction    : /* empty */
                  { $$ = nilAtom; }
                | REDUCE inSequence
                  { $$ = consList(newCTerm("fSynAction",$2),nilAtom); }
                ;

nonEmptySeq     : synVariable nonEmptySeq
                  { $$ = $2; }
                | synVariable terminatorOp coord synPrims prodMakeKey
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

synVariable     : nakedVariable
                  { terms[depth] = new TermNode($1, terms[depth]); }
                ;

synPrims        : optSynAction
                  { $$ = $1; }
                | synPrim synPrims
                  { $$ = consList($1,$2); }
                ;

synPrim         : variable '=' synPrimNoAssign
                  { $$ = newCTerm("fSynAssignment",$1,$3); }
                | synPrimNoAssign
                  { $$ = $1; }
                ;

synPrimNoAssign : nakedVariable
                  { $$ = newCTerm("fSynApplication",$1,nilAtom); }
                | nakedVariable terminatorOp coord prodMakeKey
                  { $$ = newCTerm("fSynTemplateInstantiation",$4,
                                  consList(newCTerm("fSynApplication",$1,
                                                    nilAtom),
                                           nilAtom),$3);
                  }
                | synPrimNoVarNoAssign
                  { $$ = $1; }
                ;

synPrimNoVar    : '!' coord nakedVariable '=' synPrimNoAssign
                  { $$ = newCTerm("fSynAssignment",
                                  newCTerm("fEscape",$3,$2),$5); }
                | synPrimNoVarNoAssign
                  { $$ = $1; }
                ;

synPrimNoVarNoAssign
                : synInstTerm
                  { $$ = $1; }
                | prodNameAtom coord synInstTerm optTerminatorOp prodMakeKey
                  { $$ = newCTerm("fSynTemplateInstantiation",$5,
                                  consList($3,nilAtom),$2);
                  }
                | synInstTerm terminatorOp coord prodMakeKey
                  { $$ = newCTerm("fSynTemplateInstantiation",$4,
                                  consList($1,nilAtom),$3);
                  }
                | prodName coord '(' { *prodKey[depth]++ = '('; depth++; }
                  synProdCallParams ')' { depth--; }
                  optTerminatorOp prodMakeKey
                  { $$ = newCTerm("fSynTemplateInstantiation",$9,$5,$2); }
                | prodName coord '[' { *prodKey[depth]++ = '['; depth++; }
                  synProdCallParams ']' { depth--; }
                  optTerminatorOp prodMakeKey
                  { $$ = newCTerm("fSynTemplateInstantiation",$9,$5,$2); }
                | prodName coord '{' { *prodKey[depth]++ = '{'; depth++; }
                  synProdCallParams '}' { depth--; }
                  optTerminatorOp prodMakeKey
                  { $$ = newCTerm("fSynTemplateInstantiation",$9,$5,$2); }
                ;

synInstTerm     : atom
                  { $$ = newCTerm("fSynApplication",$1,nilAtom); }
                | synLabel coord '(' phraseList ')'
                  { $$ = newCTerm("fSynApplication",$1,$4); }
                ;

synLabel        : ATOM_LABEL
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

static void append(char *s) {
  xy_errorMessages = OZ_pair2(xy_errorMessages,OZ_string(s));
}

static void append(int i) {
  xy_errorMessages = OZ_pair2(xy_errorMessages,OZ_int(i));
}

void xyreportError(char *kind, char *msg, char *file, int line, int offset) {
  if (strcmp(kind,"warning"))
    yynerrs++;

  char s[256];
  sprintf(s,"\n%c",MSG_ERROR);

  append("\n%************ ");
  append(kind);
  append(" **********\n%**\n%**     ");
  append(msg);
  append(s);

  if (line < 0)
    return;

  append("%**     in file \"");
  append(file);
  append("\", line ");
  append(line);
  append(", column ");
  append(offset);

  FILE *pFile = fopen(file,"r");
  if (pFile == NULL) {
    append("\n%**\n");
    return;
  }

  /* position file pointer at the beginning of the <line>-nth line */
  int c;
  while(line > 1) {
    c = fgetc(pFile);
    if (c == EOF)
      return;
    if (c == '\n')
      line--;
  }

  append(":\n%**\n%**     ");
  int col = 0, curoff = 0, n = -1;
  do {                          /* print the line (including '\n') */
    if (curoff == offset)
      n = col;
    curoff++;
    c = fgetc(pFile);
    if (c == EOF)
      s[col++] = '\n';
    else if (c == '\t') {       /* print tabs explicitly */
      while (col % 8)
        s[col++] = ' ';
    } else
      s[col++] = c;
  } while (c != '\n' && c != EOF && col < 255);
  s[col] = '\0';
  append(s);
  fclose(pFile);
  if (n > -1) {
    append("%**     ");
    s[n] = '\0';
    while(n)
      s[--n] = ' ';
    append(s);
    append("^-- *** here\n%**\n");
  }
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
