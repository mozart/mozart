/*
 *  Authors:
 *    Martin Henz (henz@iscs.nus.sg)
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 *
 *  Copyright:
 *    Martin Henz and Leif Kornstaedt, 1996, 1997
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "base.hh"


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

static inline int xycharno() {
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

static OZ_Term nilAtom;
static OZ_Term yyoutput;

static void xyerror(char *);


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

#define pair(left,right) OZ_pair2(left,right)
#define consList(head,tail) OZ_cons(head,tail)

inline OZ_Term newCTerm(char *l) {
  return OZ_atom(l);
}

inline OZ_Term newCTerm(char *l, OZ_Term t1) {
  return OZ_mkTupleC(l,1,t1);
}

inline OZ_Term newCTerm(char *l, OZ_Term t1, OZ_Term t2) {
  return OZ_mkTupleC(l,2,t1,t2);
}

inline OZ_Term newCTerm(char *l, OZ_Term t1, OZ_Term t2, OZ_Term t3) {
  return OZ_mkTupleC(l,3,t1,t2,t3);
}

inline OZ_Term newCTerm(char *l, OZ_Term t1, OZ_Term t2, OZ_Term t3, OZ_Term t4) {
  return OZ_mkTupleC(l,4,t1,t2,t3,t4);
}

inline OZ_Term newCTerm(char *l, OZ_Term t1, OZ_Term t2, OZ_Term t3, OZ_Term t4, OZ_Term t5) {
  return OZ_mkTupleC(l,5,t1,t2,t3,t4,t5);
}

inline OZ_Term newCTerm(char *l, OZ_Term t1, OZ_Term t2, OZ_Term t3, OZ_Term t4, OZ_Term t5, OZ_Term t6) {
  return OZ_mkTupleC(l,6,t1,t2,t3,t4,t5,t6);
}

inline OZ_Term newCTerm(char *l, OZ_Term t1, OZ_Term t2, OZ_Term t3, OZ_Term t4, OZ_Term t5, OZ_Term t6, OZ_Term t7) {
  return OZ_mkTupleC(l,7,t1,t2,t3,t4,t5,t6,t7);
}

static OZ_Term makeLongPos(OZ_Term pos1, OZ_Term pos2) {
  return newCTerm("pos",OZ_subtree(pos1,OZ_int(1)),OZ_subtree(pos1,OZ_int(2)),
                  OZ_subtree(pos1,OZ_int(3)),OZ_subtree(pos2,OZ_int(1)),
                  OZ_subtree(pos2,OZ_int(2)),OZ_subtree(pos2,OZ_int(3)));
}

inline OZ_Term pos() {
  return newCTerm("pos",xyFileNameAtom,OZ_int(xylino),OZ_int(xycharno()));
}

inline OZ_Term makeVar(OZ_Term printName, OZ_Term pos) {
  return newCTerm("fVar",printName,pos);
}

inline OZ_Term makeVar(char *printName) {
  return makeVar(OZ_atom(printName),pos());
}

inline OZ_Term makeCons(OZ_Term first, OZ_Term second, OZ_Term pos) {
   return newCTerm("fRecord",
                   newCTerm("fAtom",OZ_atom("|"),pos),
                   consList(first,consList(second,nilAtom)));
}

static OZ_Term makeInt(char *chars, OZ_Term pos) {
  return newCTerm("fInt",OZ_CStringToInt(chars),pos);
}

static OZ_Term makeInt(char c, OZ_Term pos) {
  return newCTerm("fInt",OZ_int((unsigned char) c),pos);
}

static OZ_Term makeString(char *chars, OZ_Term pos) {
  if (chars[0] == '\0')
    return newCTerm("fAtom",nilAtom,pos);
  else
    return makeCons(makeInt(chars[0],pos),makeString(&chars[1],pos),pos);
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

%token SWITCH SWITCHNAME LOCALSWITCHES PUSHSWITCHES POPSWITCHES
%token OZATOM ATOM_LABEL OZFLOAT OZINT AMPER DOTINT STRING
%token VARIABLE VARIABLE_LABEL
%token DEFAULT CHOICE LDOTS
%token attr at _case_ catch choice _class_ cond _condis_ declare define
%token dis _else_ elsecase elseif elseof end export fail false FALSE_LABEL
%token feat finally _from_ _fun_ functor _if_ import _in_ local _lock_
%token _meth_ not of or prepare proc prop _raise_ require self skip then
%token thread true TRUE_LABEL try unit UNIT_LABEL

%token ENDOFFILE

%token REGEX lex _mode_ _parser_ prod _scanner_ syn token
%token REDUCE SEP

%right    '='
%right    OOASSIGN
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
%left     '@' DEREFF

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
                  { yyoutput = $1; YYACCEPT; }
                | error
                  { yyoutput = OZ_atom("parseError"); YYABORT; }
                ;

queries         : sequence queries1
                  { $$ = consList($1,$2); }
                | prodClauseList queries1
                  { $$ = consList(newCTerm("fSynTopLevelProductionTemplates",
                                           $1),$2); }
                | queries1
                  { $$ = $1; }
                | thisCoord functorDescriptorList queries1
                  { $$ = consList(newCTerm("fFunctor",newCTerm("fDollar",$1),
                                           $2,$1),$3); }
                ;

queries1        : directive queries
                  { $$ = consList($1,$2); }
                | declare coord sequence _in_ thisCoord queries1
                  { $$ = consList(newCTerm("fDeclare",$3,newCTerm("fSkip",$5),
                                           $2),$6); }
                | declare coord sequence _in_ sequence queries1
                  { $$ = consList(newCTerm("fDeclare",$3,$5,$2),$6); }
                | declare coord sequence thisCoord queries1
                  { $$ = consList(newCTerm("fDeclare",$3,
                                           newCTerm("fSkip",$4),$2),$5); }
                | /* empty */
                  { $$ = nilAtom; }
                ;

directive       : SWITCH switchList
                  { $$ = newCTerm("dirSwitch",$2); }
                | LOCALSWITCHES
                  { $$ = newCTerm("dirLocalSwitches"); }
                | PUSHSWITCHES
                  { $$ = newCTerm("dirPushSwitches"); }
                | POPSWITCHES
                  { $$ = newCTerm("dirPopSwitches"); }
                ;

switchList      : /* empty */
                  { $$ = nilAtom; }
                | switch switchList
                  { $$ = consList($1,$2); }
                ;

switch          : '+' SWITCHNAME
                  { if (!strcmp(xytext,"gump"))
                      xy_gumpSyntax = 1;
                    if (!strcmp(xytext,"allowdeprecated"))
                      xy_allowDeprecated = 1;
                    $$ = newCTerm("on",newCTerm(xytext),pos());
                  }
                | '-' SWITCHNAME
                  { if (!strcmp(xytext,"gump"))
                      xy_gumpSyntax = 0;
                    if (!strcmp(xytext,"allowdeprecated"))
                      xy_allowDeprecated = 0;
                    $$ = newCTerm("off",newCTerm(xytext),pos());
                  }
                ;

sequence        : phrase
                  { $$ = $1; }
                | phrase sequence
                  { $$ = newCTerm("fAnd",$1,$2); }
                ;

phrase          : phrase '=' coord phrase
                  { $$ = newCTerm("fEq",$1,$4,$3); }
                | phrase OOASSIGN coord phrase
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
                | DEREFF coord phrase2
                  { $$ = newCTerm("fOpApply",newCTerm("!!"),
                                  consList($3,nilAtom),$2); }
                | '(' inSequence ')'
                  { $$ = $2; }
                | atom
                  { $$ = $1; }
                | variable
                  { $$ = $1; }
                | '_'
                  { $$ = newCTerm("fWildcard",pos()); }
                | unit
                  { $$ = newCTerm("fAtom",OZ_unit(),pos()); }
                | true
                  { $$ = newCTerm("fAtom",OZ_true(),pos()); }
                | false
                  { $$ = newCTerm("fAtom",OZ_false(),pos()); }
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
                | record
                  { $$ = $1; }
                | '[' coord phrase fixedListArgs ']' coord
                  { $$ = newCTerm("fRecord",newCTerm("fAtom",newCTerm("|"),
                                                     makeLongPos($2,$6)),
                                  consList($3,consList($4,nilAtom))); }
                | '{' coord phrase phraseList '}' coord
                  { $$ = newCTerm("fApply",$3,$4,makeLongPos($2,$6)); }
                | proc coord procFlags '{' phrase phraseList '}'
                  inSequence end coord
                  { $$ = newCTerm("fProc",$5,$6,$8,$3,makeLongPos($2,$10)); }
                | _fun_ coord procFlags '{' phrase phraseList '}'
                  inSequence end coord
                  { $$ = newCTerm("fFun",$5,$6,$8,$3,makeLongPos($2,$10)); }
                | functor coord phraseOpt optFunctorDescriptorList end coord
                  { $$ = newCTerm("fFunctor",$3,$4,makeLongPos($2,$6)); }
                | class
                  { $$ = $1; }
                | local coord sequence _in_ sequence end
                  { $$ = newCTerm("fLocal",$3,$5,$2); }
                | _if_ ifMain
                  { $$ = $2; }
                | _case_ caseMain
                  { $$ = $2; }
                | _lock_ coord inSequence end coord
                  { $$ = newCTerm("fLock",$3,makeLongPos($2,$5)); }
                | _lock_ coord phrase then inSequence end coord
                  { $$ = newCTerm("fLockThen",$3,$5,makeLongPos($2,$7)); }
                | thread coord inSequence end coord
                  { $$ = newCTerm("fThread",$3,makeLongPos($2,$5)); }
                | try coord inSequence optCatch optFinally end coord
                  { $$ = newCTerm("fTry",$3,$4,$5,makeLongPos($2,$7)); }
                | _raise_ coord inSequence end coord
                  { $$ = newCTerm("fRaise",$3,makeLongPos($2,$5)); }
                | skip
                  { $$ = newCTerm("fSkip",pos()); }
                | fail
                  { $$ = newCTerm("fFail",pos()); }
                | not coord inSequence end coord
                  { $$ = newCTerm("fNot",$3,makeLongPos($2,$5)); }
                | cond condMain
                  { $$ = $2; }
                | or coord orClauseList end coord
                  { $$ = newCTerm("fOr",$3,newCTerm("for"),
                                  makeLongPos($2,$5)); }
                | dis coord orClauseList end coord
                  { $$ = newCTerm("fOr",$3,newCTerm("fdis"),
                                  makeLongPos($2,$5)); }
                | choice coord choiceClauseList end coord
                  { $$ = newCTerm("fOr",$3,newCTerm("fchoice"),
                                  makeLongPos($2,$5)); }
                | _condis_ coord condisClauseList end coord
                  { $$ = newCTerm("fCondis",$3,makeLongPos($2,$5)); }
                | scannerSpecification
                  { $$ = $1; }
                | parserSpecification
                  { $$ = $1; }
                ;

procFlags       : /* empty */
                  { $$ = nilAtom; }
                | atom procFlags
                  { $$ = consList($1,$2); }
                ;

optFunctorDescriptorList
                : /* empty */
                  { $$ = nilAtom; }
                | functorDescriptorList
                  { $$ = $1; }
                ;

functorDescriptorList
                : require coord importDecls optFunctorDescriptorList
                  { $$ = consList(newCTerm("fRequire",$3,$2),$4); }
                | prepare coord sequence _in_ sequence optFunctorDescriptorList
                  { $$ = consList(newCTerm("fPrepare",$3,$5,$2),$6); }
                | prepare coord sequence optFunctorDescriptorList
                  { $$ = consList(newCTerm("fPrepare",$3,
                                           newCTerm("fSkip",$2),$2),$4); }
                | import coord importDecls optFunctorDescriptorList
                  { $$ = consList(newCTerm("fImport",$3,$2),$4); }
                | export coord exportDecls optFunctorDescriptorList
                  { $$ = consList(newCTerm("fExport",$3,$2),$4); }
                | define coord sequence _in_ sequence optFunctorDescriptorList
                  { $$ = consList(newCTerm("fDefine",$3,$5,$2),$6); }
                | define coord sequence optFunctorDescriptorList
                  { $$ = consList(newCTerm("fDefine",$3,
                                           newCTerm("fSkip",$2),$2),$4); }
                ;

importDecls     : /* empty */
                  { $$ = nilAtom; }
                | nakedVariable optImportAt importDecls
                  { $$ = consList(newCTerm("fImportItem",$1,nilAtom,$2),$3); }
                | variableLabel '(' featureList ')' optImportAt importDecls
                  { $$ = consList(newCTerm("fImportItem",$1,$3,$5),$6); }
                ;

variableLabel   : VARIABLE_LABEL coord
                  { $$ = newCTerm("fVar",OZ_atom(xytext),$2); }
                ;

featureList     : featureNoVar
                  { $$ = consList($1,nilAtom); }
                | featureNoVar ':' nakedVariable
                  { $$ = consList(pair($3,$1),nilAtom); }
                | featureNoVar featureList
                  { $$ = consList($1,$2); }
                | featureNoVar ':' nakedVariable featureList
                  { $$ = consList(pair($3,$1),$4); }
                ;

optImportAt     : /* empty */
                  { $$ = newCTerm("fNoImportAt"); }
                | at atom
                  { $$ = newCTerm("fImportAt",$2); }
                ;

exportDecls     : /* empty */
                  { $$ = nilAtom; }
                | nakedVariable exportDecls
                  { $$ = consList(newCTerm("fExportItem",$1),$2); }
                | featureNoVar ':' nakedVariable exportDecls
                  { $$ = consList(newCTerm("fExportItem",
                                           newCTerm("fColon",$1,$3)),$4); }
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
                  { $$ = newCTerm(xytext); }
                ;

fdMul           : FDMUL
                  { $$ = newCTerm(xytext); }
                ;

otherMul        : OTHERMUL
                  { $$ = newCTerm(xytext); }
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

fixedListArgs   : thisCoord
                  { $$ = newCTerm("fAtom",nilAtom,$1); }
                | thisCoord phrase fixedListArgs
                  { $$ = newCTerm("fRecord",
                                  newCTerm("fAtom",newCTerm("|"),$1),
                                  consList($2,consList($3,nilAtom))); }
                ;

optCatch        : /* empty */
                  { $$ = newCTerm("fNoCatch"); }
                | catch coord caseClauseList
                  { $$ = newCTerm("fCatch",$3,$2); }
                ;

optFinally      : /* empty */
                  { $$ = newCTerm("fNoFinally"); }
                | finally inSequence
                  { $$ = $2; }
                ;

record          : recordAtomLabel coord '(' recordArguments optDots ')' coord
                  {
                    $$ = newCTerm(OZ_isTrue($5)? "fOpenRecord": "fRecord",
                                  newCTerm("fAtom",$1,makeLongPos($2,$7)),$4);
                  }
                | recordVarLabel coord '(' recordArguments optDots ')' coord
                  {
                    $$ = newCTerm(OZ_isTrue($5)? "fOpenRecord": "fRecord",
                                  makeVar($1,makeLongPos($2,$7)),$4);
                  }
                ;

recordAtomLabel : ATOM_LABEL
                  { $$ = OZ_atom(xytext); }
                | UNIT_LABEL
                  { $$ = OZ_unit(); }
                | TRUE_LABEL
                  { $$ = OZ_true(); }
                | FALSE_LABEL
                  { $$ = OZ_false(); }
                ;

recordVarLabel  : VARIABLE_LABEL
                  { $$ = OZ_atom(xytext); }
                ;

recordArguments : /* empty */
                  { $$ = nilAtom; }
                | phrase recordArguments
                  { $$ = consList($1,$2); }
                | feature ':' phrase recordArguments
                  { $$ = consList(newCTerm("fColon",$1,$3),$4); }
                ;

optDots         : /* empty */
                  { $$ = OZ_false(); }
                | LDOTS
                  { $$ = OZ_true(); }
                ;

feature         : atom
                  { $$ = $1; }
                | nakedVariable
                  { $$ = $1; }
                | int
                  { $$ = $1; }
                | unit
                  { $$ = newCTerm("fAtom",OZ_unit(),pos()); }
                | true
                  { $$ = newCTerm("fAtom",OZ_true(),pos()); }
                | false
                  { $$ = newCTerm("fAtom",OZ_false(),pos()); }
                ;

featureNoVar    : atom
                  { $$ = $1; }
                | int
                  { $$ = $1; }
                ;

ifMain          : coord sequence then inSequence ifRest coord
                  { $$ = newCTerm("fBoolCase",$2,$4,$5,makeLongPos($1,$6)); }
                ;

ifRest          : elseif ifMain
                  { $$ = $2; }
                | elsecase caseMain
                  { $$ = $2; }
                | _else_ inSequence end
                  { $$ = $2; }
                | end
                  { $$ = newCTerm("fSkip",pos()); }
                ;

caseMain        : coord sequence then coord inSequence caseRest coord
                  { checkDeprecation($4);
                    $$ = newCTerm("fBoolCase",$2,$5,$6,makeLongPos($1,$7));
                  }
                | coord sequence of elseOfList caseRest coord
                  { $$ = newCTerm("fCase",$2,$4,$5,makeLongPos($1,$6)); }
                ;

caseRest        : elseif ifMain
                  { $$ = $2; }
                | elsecase caseMain
                  { $$ = $2; }
                | _else_ inSequence end
                  { $$ = $2; }
                | end
                  { $$ = newCTerm("fNoElse",pos()); }
                ;

elseOfList      : caseClause
                  { $$ = consList($1,nilAtom); }
                | caseClause CHOICE elseOfList
                  { $$ = consList($1,$3); }
                | caseClause elseof elseOfList
                  { $$ = consList($1,$3); }
                ;

caseClauseList  : caseClause
                  { $$ = consList($1,nilAtom); }
                | caseClause CHOICE caseClauseList
                  { $$ = consList($1,$3); }
                ;

caseClause      : sideCondition then inSequence
                  { $$ = newCTerm("fCaseClause",$1,$3); }
                ;

sideCondition   : pattern
                  { $$ = $1; }
                | pattern andthen thisCoord sequence
                  { $$ = newCTerm("fSideCondition",$1,
                                  newCTerm("fSkip",$3),$4,$3); }
                | pattern andthen thisCoord sequence _in_ sequence
                  { $$ = newCTerm("fSideCondition",$1,$4,$6,$3); }
                ;

pattern         : pattern '=' coord pattern
                  { $$ = newCTerm("fEq",$1,$4,$3); }
                | pattern '|' coord pattern
                  { $$ = makeCons($1,$4,$3); }
                | phrase2
                  { $$ = $1; }
                | phrase2 '#' coord hashes
                  { $$ = newCTerm("fRecord",
                                  newCTerm("fAtom",newCTerm("#"),$3),
                                  consList($1,$4)); }
                ;

class           : _class_ coord phraseOpt classDescriptorList methList
                  end coord
                  { $$ = newCTerm("fClass",$3,$4,$5,makeLongPos($2,$7)); }
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
                  { $$ = newCTerm("fAtom",OZ_unit(),pos()); }
                | true
                  { $$ = newCTerm("fAtom",OZ_true(),pos()); }
                | false
                  { $$ = newCTerm("fAtom",OZ_false(),pos()); }
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
                  { $$ = newCTerm("fAtom",OZ_unit(),pos()); }
                | true
                  { $$ = newCTerm("fAtom",OZ_true(),pos()); }
                | false
                  { $$ = newCTerm("fAtom",OZ_false(),pos()); }
                | methHeadLabel '(' methFormals ')'
                  { $$ = newCTerm("fRecord",$1,$3); }
                | methHeadLabel '(' methFormals LDOTS ')'
                  { $$ = newCTerm("fOpenRecord",$1,$3); }
                ;

methHeadLabel   : ATOM_LABEL
                  { $$ = newCTerm("fAtom",newCTerm(xytext),pos()); }
                | VARIABLE_LABEL
                  { $$ = makeVar(xytext); }
                | '!' coord VARIABLE_LABEL
                  { $$ = newCTerm("fEscape",makeVar(xytext),$2); }
                | UNIT_LABEL
                  { $$ = newCTerm("fAtom",OZ_unit(),pos()); }
                | TRUE_LABEL
                  { $$ = newCTerm("fAtom",OZ_true(),pos()); }
                | FALSE_LABEL
                  { $$ = newCTerm("fAtom",OZ_false(),pos()); }
                ;

methFormals     : methFormal methFormals
                  { $$ = consList($1,$2); }
                | /* empty */
                  { $$ = nilAtom; }
                ;

methFormal      : methFormalTerm methFormalOptDefault
                  { $$ = newCTerm("fMethArg",$1,$2); }
                | feature ':' methFormalTerm methFormalOptDefault
                  { $$ = newCTerm("fMethColonArg",$1,$3,$4); }
                ;

methFormalTerm  : nakedVariable
                  { $$ = $1; }
                | '$'
                  { $$ = newCTerm("fDollar",pos()); }
                | '_'
                  { $$ = newCTerm("fWildcard",pos()); }
                ;

methFormalOptDefault
                : DEFAULT coord phrase
                  { $$ = newCTerm("fDefault",$3,$2); }
                | /* empty */
                  { $$ = newCTerm("fNoDefault"); }
                ;

condMain        : coord condClauseList condElse end coord
                  { $$ = newCTerm("fCond",$2,$3,makeLongPos($1,$5)); }
                ;

condElse        : _else_ inSequence
                  { $$ = $2; }
                | /* empty */
                  { $$ = newCTerm("fNoElse",pos()); }
                ;

condClauseList  : condClause
                  { $$ = consList($1,nilAtom); }
                | condClause CHOICE condClauseList
                  { $$ = consList($1,$3); }
                ;

condClause      : sequence then coord inSequence
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
                  classDescriptorList methList scannerRules end coord
                  { OZ_Term prefix =
                      scannerPrefix? scannerPrefix: OZ_atom("zy");
                    $$ = newCTerm("fScanner",$3,$4,$5,$6,prefix,
                                  makeLongPos($2,$8)); }
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
                | STRING
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
                  tokenClause parserRules end coord
                  { OZ_Term expect = parserExpect? parserExpect: OZ_int(0);
                    $$ = newCTerm("fParser",$3,$4,$5,$6,$7,expect,
                                  makeLongPos($2,$9)); }
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

tokenClause     : /* empty */
                  { $$ = newCTerm("fToken",nilAtom); }
                | token tokenList
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

synSeq          : thisCoord nonEmptySeq
                  { OZ_Term t = $2;
                    while (terms[depth]) {
                      t = consList(newCTerm("fSynApplication", terms[depth]->term, nilAtom), t);
                      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
                    }
                    $$ = newCTerm("fSynSequence", decls[depth], t, $1);
                    decls[depth] = nilAtom;
                  }
                | skip coord optSynAction
                  { $$ = newCTerm("fSynSequence", nilAtom, $3, $2); }
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

void checkDeprecation(OZ_Term coord) {
  char *msg = "use `if' instead of `case' for boolean conditionals";
  if (xy_allowDeprecated) {
    xyreportWarning("deprecation warning",msg,coord);
  } else {
    xyreportError("deprecation error",msg,coord);
  }
}

void xyreportWarning(char *kind, char *msg, OZ_Term coord) {
  OZ_Term args = OZ_cons(OZ_pairA("coord",coord),
                         OZ_cons(OZ_pairAA("kind",kind),
                                 OZ_cons(OZ_pairAA("msg",msg),OZ_nil())));
  xy_errorMessages = OZ_cons(OZ_recordInit(OZ_atom("warn"),args),
                             xy_errorMessages);
}

void xyreportError(char *kind, char *msg, OZ_Term coord) {
  OZ_Term args = OZ_cons(OZ_pairA("coord",coord),
                         OZ_cons(OZ_pairAA("kind",kind),
                                 OZ_cons(OZ_pairAA("msg",msg),OZ_nil())));
  xy_errorMessages = OZ_cons(OZ_recordInit(OZ_atom("error"),args),
                             xy_errorMessages);
}

void xyreportError(char *kind, char *msg, const char *file,
                   int line, int column) {
  xyreportError(kind,msg,OZ_mkTupleC("pos",3,OZ_atom(file),
                                     OZ_int(line),OZ_int(column)));
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

  x = OZ_subtree(optRec, OZ_atom("gump"));
  xy_gumpSyntax = x == 0? 0: OZ_eq(x, OZ_true());

  x = OZ_subtree(optRec, OZ_atom("allowdeprecated"));
  xy_allowDeprecated = x == 0? 1: OZ_eq(x, OZ_true());

  OZ_Term defines = OZ_subtree(optRec, OZ_atom("defines"));
  return defines;
}

static OZ_Term parse() {
  nilAtom = OZ_nil();

  int i;
  for (i = 0; i < DEPTH; i++) {
    prodKey[i] = prodKeyBuffer[i];
    prodName[i] = OZ_atom("none");
    terms[i] = 0;
    decls[i] = nilAtom;
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
    OZ_RETURN(OZ_pair2(OZ_atom("fileNotFound"), OZ_nil()));
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
