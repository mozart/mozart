
/*  A Bison parser, made from /home/kornstae/mozart/platform/emulator/parser.yy
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
#define T_SWITCH        257
#define T_SWITCHNAME    258
#define T_LOCALSWITCHES 259
#define T_PUSHSWITCHES  260
#define T_POPSWITCHES   261
#define T_OZATOM        262
#define T_ATOM_LABEL    263
#define T_OZFLOAT       264
#define T_OZINT 265
#define T_AMPER 266
#define T_DOTINT        267
#define T_STRING        268
#define T_VARIABLE      269
#define T_VARIABLE_LABEL        270
#define T_DEFAULT       271
#define T_CHOICE        272
#define T_LDOTS 273
#define T_attr  274
#define T_at    275
#define T_case  276
#define T_catch 277
#define T_choice        278
#define T_class 279
#define T_cond  280
#define T_declare       281
#define T_define        282
#define T_dis   283
#define T_else  284
#define T_elsecase      285
#define T_elseif        286
#define T_elseof        287
#define T_end   288
#define T_export        289
#define T_fail  290
#define T_false 291
#define T_FALSE_LABEL   292
#define T_feat  293
#define T_finally       294
#define T_from  295
#define T_fun   296
#define T_functor       297
#define T_if    298
#define T_import        299
#define T_in    300
#define T_local 301
#define T_lock  302
#define T_meth  303
#define T_not   304
#define T_of    305
#define T_or    306
#define T_prepare       307
#define T_proc  308
#define T_prop  309
#define T_raise 310
#define T_require       311
#define T_self  312
#define T_skip  313
#define T_then  314
#define T_thread        315
#define T_true  316
#define T_TRUE_LABEL    317
#define T_try   318
#define T_unit  319
#define T_UNIT_LABEL    320
#define T_loop  321
#define T_ENDOFFILE     322
#define T_REGEX 323
#define T_lex   324
#define T_mode  325
#define T_parser        326
#define T_prod  327
#define T_scanner       328
#define T_syn   329
#define T_token 330
#define T_REDUCE        331
#define T_SEP   332
#define T_OOASSIGN      333
#define T_orelse        334
#define T_andthen       335
#define T_COMPARE       336
#define T_FDCOMPARE     337
#define T_LMACRO        338
#define T_RMACRO        339
#define T_FDIN  340
#define T_ADD   341
#define T_FDMUL 342
#define T_OTHERMUL      343
#define T_DEREFF        344


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

#define PA_allowdeprecated                      _PA_AtomTab[0]
#define PA_coord                                _PA_AtomTab[1]
#define PA_defines                              _PA_AtomTab[2]
#define PA_deprecation_error                    _PA_AtomTab[3]
#define PA_deprecation_warning                  _PA_AtomTab[4]
#define PA_dirLocalSwitches                     _PA_AtomTab[5]
#define PA_dirPopSwitches                       _PA_AtomTab[6]
#define PA_dirPushSwitches                      _PA_AtomTab[7]
#define PA_dirSwitch                            _PA_AtomTab[8]
#define PA_error                                _PA_AtomTab[9]
#define PA_fAnd                                 _PA_AtomTab[10]
#define PA_fAndThen                             _PA_AtomTab[11]
#define PA_fApply                               _PA_AtomTab[12]
#define PA_fAssign                              _PA_AtomTab[13]
#define PA_fAt                                  _PA_AtomTab[14]
#define PA_fAtom                                _PA_AtomTab[15]
#define PA_fAttr                                _PA_AtomTab[16]
#define PA_fBoolCase                            _PA_AtomTab[17]
#define PA_fCase                                _PA_AtomTab[18]
#define PA_fCaseClause                          _PA_AtomTab[19]
#define PA_fCatch                               _PA_AtomTab[20]
#define PA_fChoice                              _PA_AtomTab[21]
#define PA_fClass                               _PA_AtomTab[22]
#define PA_fClause                              _PA_AtomTab[23]
#define PA_fColon                               _PA_AtomTab[24]
#define PA_fCond                                _PA_AtomTab[25]
#define PA_fDeclare                             _PA_AtomTab[26]
#define PA_fDefault                             _PA_AtomTab[27]
#define PA_fDefine                              _PA_AtomTab[28]
#define PA_fDis                                 _PA_AtomTab[29]
#define PA_fDollar                              _PA_AtomTab[30]
#define PA_fEq                                  _PA_AtomTab[31]
#define PA_fEscape                              _PA_AtomTab[32]
#define PA_fExport                              _PA_AtomTab[33]
#define PA_fExportItem                          _PA_AtomTab[34]
#define PA_fFail                                _PA_AtomTab[35]
#define PA_fFdCompare                           _PA_AtomTab[36]
#define PA_fFdIn                                _PA_AtomTab[37]
#define PA_fFeat                                _PA_AtomTab[38]
#define PA_fFloat                               _PA_AtomTab[39]
#define PA_fFrom                                _PA_AtomTab[40]
#define PA_fFun                                 _PA_AtomTab[41]
#define PA_fFunctor                             _PA_AtomTab[42]
#define PA_fImport                              _PA_AtomTab[43]
#define PA_fImportAt                            _PA_AtomTab[44]
#define PA_fImportItem                          _PA_AtomTab[45]
#define PA_fInheritedModes                      _PA_AtomTab[46]
#define PA_fInt                                 _PA_AtomTab[47]
#define PA_fLexicalAbbreviation                 _PA_AtomTab[48]
#define PA_fLexicalRule                         _PA_AtomTab[49]
#define PA_fLocal                               _PA_AtomTab[50]
#define PA_fLock                                _PA_AtomTab[51]
#define PA_fLockThen                            _PA_AtomTab[52]
#define PA_fMeth                                _PA_AtomTab[53]
#define PA_fMethArg                             _PA_AtomTab[54]
#define PA_fMethColonArg                        _PA_AtomTab[55]
#define PA_fMode                                _PA_AtomTab[56]
#define PA_fNoCatch                             _PA_AtomTab[57]
#define PA_fNoDefault                           _PA_AtomTab[58]
#define PA_fNoElse                              _PA_AtomTab[59]
#define PA_fNoFinally                           _PA_AtomTab[60]
#define PA_fNoImportAt                          _PA_AtomTab[61]
#define PA_fNoThen                              _PA_AtomTab[62]
#define PA_fNot                                 _PA_AtomTab[63]
#define PA_fObjApply                            _PA_AtomTab[64]
#define PA_fOpApply                             _PA_AtomTab[65]
#define PA_fOpenRecord                          _PA_AtomTab[66]
#define PA_fOr                                  _PA_AtomTab[67]
#define PA_fOrElse                              _PA_AtomTab[68]
#define PA_fParser                              _PA_AtomTab[69]
#define PA_fPrepare                             _PA_AtomTab[70]
#define PA_fProc                                _PA_AtomTab[71]
#define PA_fProductionTemplate                  _PA_AtomTab[72]
#define PA_fProp                                _PA_AtomTab[73]
#define PA_fRaise                               _PA_AtomTab[74]
#define PA_fRecord                              _PA_AtomTab[75]
#define PA_fRequire                             _PA_AtomTab[76]
#define PA_fScanner                             _PA_AtomTab[77]
#define PA_fSelf                                _PA_AtomTab[78]
#define PA_fSideCondition                       _PA_AtomTab[79]
#define PA_fSkip                                _PA_AtomTab[80]
#define PA_fSynAction                           _PA_AtomTab[81]
#define PA_fSynAlternative                      _PA_AtomTab[82]
#define PA_fSynApplication                      _PA_AtomTab[83]
#define PA_fSynAssignment                       _PA_AtomTab[84]
#define PA_fSynSequence                         _PA_AtomTab[85]
#define PA_fSynTemplateInstantiation            _PA_AtomTab[86]
#define PA_fSynTopLevelProductionTemplates      _PA_AtomTab[87]
#define PA_fSyntaxRule                          _PA_AtomTab[88]
#define PA_fThread                              _PA_AtomTab[89]
#define PA_fToken                               _PA_AtomTab[90]
#define PA_fTry                                 _PA_AtomTab[91]
#define PA_fVar                                 _PA_AtomTab[92]
#define PA_fWildcard                            _PA_AtomTab[93]
#define PA_fileNotFound                         _PA_AtomTab[94]
#define PA_gump                                 _PA_AtomTab[95]
#define PA_kind                                 _PA_AtomTab[96]
#define PA_msg                                  _PA_AtomTab[97]
#define PA_none                                 _PA_AtomTab[98]
#define PA_off                                  _PA_AtomTab[99]
#define PA_on                                   _PA_AtomTab[100]
#define PA_parse                                _PA_AtomTab[101]
#define PA_parseError                           _PA_AtomTab[102]
#define PA_pos                                  _PA_AtomTab[103]
#define PA_warn                                 _PA_AtomTab[104]
#define PA_zy                                   _PA_AtomTab[105]
#define PA_fLoop                                _PA_AtomTab[106]
#define PA_fMacro                               _PA_AtomTab[107]

const char * _PA_CharTab[] = {
        "allowdeprecated",                      //0
        "coord",                                //1
        "defines",                              //2
        "deprecation error",                    //3
        "deprecation warning",                  //4
        "dirLocalSwitches",                     //5
        "dirPopSwitches",                       //6
        "dirPushSwitches",                      //7
        "dirSwitch",                            //8
        "error",                                //9
        "fAnd",                                 //10
        "fAndThen",                             //11
        "fApply",                               //12
        "fAssign",                              //13
        "fAt",                                  //14
        "fAtom",                                //15
        "fAttr",                                //16
        "fBoolCase",                            //17
        "fCase",                                //18
        "fCaseClause",                          //19
        "fCatch",                               //20
        "fChoice",                              //21
        "fClass",                               //22
        "fClause",                              //23
        "fColon",                               //24
        "fCond",                                //25
        "fDeclare",                             //26
        "fDefault",                             //27
        "fDefine",                              //28
        "fDis",                                 //29
        "fDollar",                              //30
        "fEq",                                  //31
        "fEscape",                              //32
        "fExport",                              //33
        "fExportItem",                          //34
        "fFail",                                //35
        "fFdCompare",                           //36
        "fFdIn",                                //37
        "fFeat",                                //38
        "fFloat",                               //39
        "fFrom",                                //40
        "fFun",                                 //41
        "fFunctor",                             //42
        "fImport",                              //43
        "fImportAt",                            //44
        "fImportItem",                          //45
        "fInheritedModes",                      //46
        "fInt",                                 //47
        "fLexicalAbbreviation",                 //48
        "fLexicalRule",                         //49
        "fLocal",                               //50
        "fLock",                                //51
        "fLockThen",                            //52
        "fMeth",                                //53
        "fMethArg",                             //54
        "fMethColonArg",                        //55
        "fMode",                                //56
        "fNoCatch",                             //57
        "fNoDefault",                           //58
        "fNoElse",                              //59
        "fNoFinally",                           //60
        "fNoImportAt",                          //61
        "fNoThen",                              //62
        "fNot",                                 //63
        "fObjApply",                            //64
        "fOpApply",                             //65
        "fOpenRecord",                          //66
        "fOr",                                  //67
        "fOrElse",                              //68
        "fParser",                              //69
        "fPrepare",                             //70
        "fProc",                                //71
        "fProductionTemplate",                  //72
        "fProp",                                //73
        "fRaise",                               //74
        "fRecord",                              //75
        "fRequire",                             //76
        "fScanner",                             //77
        "fSelf",                                //78
        "fSideCondition",                       //79
        "fSkip",                                //80
        "fSynAction",                           //81
        "fSynAlternative",                      //82
        "fSynApplication",                      //83
        "fSynAssignment",                       //84
        "fSynSequence",                         //85
        "fSynTemplateInstantiation",            //86
        "fSynTopLevelProductionTemplates",      //87
        "fSyntaxRule",                          //88
        "fThread",                              //89
        "fToken",                               //90
        "fTry",                                 //91
        "fVar",                                 //92
        "fWildcard",                            //93
        "fileNotFound",                         //94
        "gump",                                 //95
        "kind",                                 //96
        "msg",                                  //97
        "none",                                 //98
        "off",                                  //99
        "on",                                   //100
        "parse",                                //101
        "parseError",                           //102
        "pos",                                  //103
        "warn",                                 //104
        "zy",                                   //105
        "fLoop",                                //106
        "fMacro",                               //107
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



#define YYFINAL         757
#define YYFLAG          -32768
#define YYNTBASE        111

#define YYTRANSLATE(x) ((unsigned)(x) <= 344 ? yytranslate[x] : 247)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   110,     2,    89,   104,     2,     2,     2,   101,
   102,     2,    99,    93,   100,    95,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,   109,     2,     2,
    79,     2,     2,    97,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   105,     2,   106,    96,   103,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   107,    88,   108,    94,     2,     2,     2,     2,
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
    77,    78,    80,    81,    82,    83,    84,    85,    86,    87,
    90,    91,    92,    98
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
   324,   330,   331,   334,   335,   337,   342,   349,   354,   359,
   364,   371,   376,   377,   381,   388,   391,   393,   397,   400,
   405,   406,   409,   410,   413,   418,   420,   422,   424,   426,
   428,   430,   435,   437,   438,   441,   443,   447,   448,   452,
   453,   456,   464,   472,   474,   476,   478,   480,   482,   483,
   486,   491,   492,   494,   496,   498,   500,   502,   504,   506,
   508,   510,   517,   520,   523,   527,   529,   537,   544,   547,
   550,   554,   556,   558,   562,   566,   568,   572,   576,   578,
   583,   590,   595,   600,   602,   607,   615,   617,   619,   620,
   623,   628,   633,   638,   643,   644,   647,   651,   653,   655,
   657,   659,   661,   663,   665,   666,   669,   675,   677,   682,
   684,   686,   688,   690,   692,   697,   703,   705,   707,   711,
   713,   715,   717,   720,   721,   724,   729,   731,   733,   735,
   739,   740,   746,   749,   750,   752,   756,   761,   767,   771,
   775,   778,   783,   788,   794,   796,   800,   802,   804,   806,
   810,   812,   814,   816,   818,   819,   820,   829,   831,   833,
   835,   838,   841,   844,   850,   856,   861,   863,   865,   870,
   871,   874,   877,   879,   881,   891,   893,   895,   898,   901,
   902,   905,   907,   910,   912,   916,   918,   921,   923,   926,
   927,   937,   938,   939,   950,   957,   961,   964,   967,   969,
   970,   973,   974,   975,   982,   983,   984,   991,   992,   993,
  1000,  1002,  1006,  1008,  1010,  1012,  1013,  1015,  1017,  1019,
  1020,  1021,  1024,  1026,  1029,  1034,  1039,  1047,  1048,  1051,
  1053,  1055,  1057,  1059,  1061,  1065,  1068,  1072,  1073,  1076,
  1079,  1085,  1090,  1093,  1096,  1098,  1100,  1102,  1105,  1109,
  1111,  1113,  1118,  1120,  1126,  1128,  1130,  1136,  1141,  1142,
  1143,  1153,  1154,  1155,  1165,  1166,  1167,  1177,  1179,  1185,
  1187,  1189,  1191
};

static const short yyrhs[] = {   112,
    68,     0,     1,     0,   117,   113,     0,   201,   113,     0,
   113,     0,   185,   123,   113,     0,   114,   112,     0,    27,
   186,   117,    46,   185,   113,     0,    27,   186,   117,    46,
   117,   113,     0,    27,   186,   117,   185,   113,     0,     0,
     3,   115,     0,     5,     0,     6,     0,     7,     0,     0,
   116,   115,     0,    99,     4,     0,   100,     4,     0,   118,
     0,   118,   117,     0,   118,    79,   186,   118,     0,   118,
    80,   186,   118,     0,   118,    81,   186,   118,     0,   118,
    82,   186,   118,     0,   118,   129,   186,   118,     0,   118,
   130,   186,   118,     0,   118,   131,   186,   118,     0,   118,
    88,   186,   118,     0,   120,     0,   120,    89,   186,   119,
     0,   120,     0,   120,    89,   119,     0,   120,   132,   186,
   120,     0,   120,   133,   186,   120,     0,   120,   134,   186,
   120,     0,   120,    93,   186,   120,     0,    94,   186,   120,
     0,   120,    95,   186,   120,     0,   120,    13,     0,   120,
    96,   186,   120,     0,    97,   186,   120,     0,    98,   186,
   120,     0,   101,   135,   102,     0,   179,     0,   181,     0,
   103,     0,    65,     0,    62,     0,    37,     0,    58,     0,
   104,     0,   182,     0,   183,     0,   184,     0,   140,     0,
   105,   186,   118,   137,   106,   186,     0,   107,   186,   118,
   136,   108,   186,     0,    54,   186,   121,   107,   118,   136,
   108,   135,    34,   186,     0,    42,   186,   121,   107,   118,
   136,   108,   135,    34,   186,     0,    43,   186,   157,   122,
    34,   186,     0,   156,     0,    47,   186,   117,    46,   117,
    34,     0,    44,   147,     0,    22,   149,     0,    48,   186,
   135,    34,   186,     0,    48,   186,   118,    60,   135,    34,
   186,     0,    61,   186,   135,    34,   186,     0,    64,   186,
   135,   138,   139,    34,   186,     0,    56,   186,   135,    34,
   186,     0,    59,     0,    36,     0,    50,   186,   135,    34,
   186,     0,    26,   172,     0,    52,   186,   176,    34,   186,
     0,    29,   186,   176,    34,   186,     0,    24,   186,   178,
    34,   186,     0,   187,     0,   195,     0,    67,   186,   135,
    34,   186,     0,    85,   186,   136,    86,   186,     0,     0,
   179,   121,     0,     0,   123,     0,    57,   186,   124,   122,
     0,    53,   186,   117,    46,   117,   122,     0,    53,   186,
   117,   122,     0,    45,   186,   124,   122,     0,    35,   186,
   128,   122,     0,    28,   186,   117,    46,   117,   122,     0,
    28,   186,   117,   122,     0,     0,   180,   127,   124,     0,
   125,   101,   126,   102,   127,   124,     0,    16,   186,     0,
   146,     0,   146,   109,   180,     0,   146,   126,     0,   146,
   109,   180,   126,     0,     0,    21,   179,     0,     0,   180,
   128,     0,   146,   109,   180,   128,     0,    83,     0,    84,
     0,    87,     0,    90,     0,    91,     0,    92,     0,   117,
    46,   186,   117,     0,   117,     0,     0,   118,   136,     0,
   185,     0,   185,   118,   137,     0,     0,    23,   186,   152,
     0,     0,    40,   135,     0,   141,   186,   101,   143,   144,
   102,   186,     0,   142,   186,   101,   143,   144,   102,   186,
     0,     9,     0,    66,     0,    63,     0,    38,     0,    16,
     0,     0,   118,   143,     0,   145,   109,   118,   143,     0,
     0,    19,     0,   179,     0,   180,     0,   183,     0,    65,
     0,    62,     0,    37,     0,   179,     0,   183,     0,   186,
   117,    60,   135,   148,   186,     0,    32,   147,     0,    31,
   149,     0,    30,   135,    34,     0,    34,     0,   186,   117,
    60,   186,   135,   150,   186,     0,   186,   117,    51,   151,
   150,   186,     0,    32,   147,     0,    31,   149,     0,    30,
   135,    34,     0,    34,     0,   153,     0,   153,    18,   151,
     0,   153,    33,   151,     0,   153,     0,   153,    18,   152,
     0,   154,    60,   135,     0,   155,     0,   155,    82,   185,
   117,     0,   155,    82,   185,   117,    46,   117,     0,   155,
    79,   186,   155,     0,   155,    88,   186,   155,     0,   120,
     0,   120,    89,   186,   119,     0,    25,   186,   157,   158,
   163,    34,   186,     0,   118,     0,   185,     0,     0,   159,
   158,     0,    41,   186,   118,   136,     0,    20,   186,   161,
   160,     0,    39,   186,   161,   160,     0,    55,   186,   118,
   136,     0,     0,   161,   160,     0,   162,   109,   118,     0,
   162,     0,   179,     0,   181,     0,   183,     0,    65,     0,
    62,     0,    37,     0,     0,   164,   163,     0,    49,   186,
   165,   135,    34,     0,   166,     0,   166,    79,   186,   180,
     0,   179,     0,   181,     0,    65,     0,    62,     0,    37,
     0,   167,   101,   168,   102,     0,   167,   101,   168,    19,
   102,     0,     9,     0,    16,     0,   110,   186,    16,     0,
    66,     0,    63,     0,    38,     0,   169,   168,     0,     0,
   170,   171,     0,   145,   109,   170,   171,     0,   180,     0,
   104,     0,   103,     0,    17,   186,   118,     0,     0,   186,
   174,   173,    34,   186,     0,    30,   135,     0,     0,   175,
     0,   175,    18,   174,     0,   117,    60,   186,   135,     0,
   117,    46,   117,    60,   135,     0,   177,    18,   177,     0,
   177,    18,   176,     0,   117,   185,     0,   117,    46,   117,
   185,     0,   117,   185,    60,   135,     0,   117,    46,   117,
    60,   135,     0,   135,     0,   135,    18,   178,     0,     8,
     0,    15,     0,   180,     0,   110,   186,   180,     0,    14,
     0,    11,     0,    12,     0,    10,     0,     0,     0,    74,
   186,   180,   158,   163,   188,    34,   186,     0,   189,     0,
   190,     0,   192,     0,   189,   188,     0,   190,   188,     0,
   192,   188,     0,    70,   179,    79,   191,    34,     0,    70,
   180,    79,   191,    34,     0,    70,   191,   135,    34,     0,
    69,     0,    14,     0,    71,   180,   193,    34,     0,     0,
   194,   193,     0,    41,   200,     0,   190,     0,   192,     0,
    72,   186,   180,   158,   163,   197,   196,    34,   186,     0,
   224,     0,   202,     0,   224,   196,     0,   202,   196,     0,
     0,    76,   198,     0,   199,     0,   199,   198,     0,   179,
     0,   179,   109,   118,     0,   180,     0,   180,   200,     0,
   202,     0,   202,   201,     0,     0,    73,   180,    79,   203,
   206,   221,   222,   227,    34,     0,     0,     0,    73,   104,
   204,    79,   205,   206,   221,   222,   227,    34,     0,    73,
   206,   221,   222,   227,    34,     0,   208,   180,   219,     0,
   180,   220,     0,   207,   209,     0,   208,     0,     0,   179,
   109,     0,     0,     0,   101,   210,   216,   102,   211,   219,
     0,     0,     0,   105,   212,   216,   106,   213,   219,     0,
     0,     0,   107,   214,   216,   108,   215,   219,     0,   217,
     0,   217,   218,   216,     0,   180,     0,   103,     0,    78,
     0,     0,   220,     0,    90,     0,    91,     0,     0,     0,
   223,    46,     0,   224,     0,   224,   223,     0,    75,   179,
   227,    34,     0,    75,   180,   227,    34,     0,    75,   245,
   101,   225,   102,   227,    34,     0,     0,   226,   225,     0,
   180,     0,   104,     0,   103,     0,   228,     0,   229,     0,
   229,    18,   228,     0,   185,   231,     0,    59,   186,   230,
     0,     0,    77,   135,     0,   232,   231,     0,   232,   220,
   186,   233,   221,     0,   232,    79,   235,   233,     0,    46,
   233,     0,   236,   233,     0,   230,     0,   180,     0,   230,
     0,   234,   233,     0,   181,    79,   235,     0,   235,     0,
   180,     0,   180,   220,   186,   221,     0,   237,     0,   110,
   186,   180,    79,   235,     0,   237,     0,   244,     0,   208,
   186,   244,   219,   221,     0,   244,   220,   186,   221,     0,
     0,     0,   207,   186,   101,   238,   246,   102,   239,   219,
   221,     0,     0,     0,   207,   186,   105,   240,   246,   106,
   241,   219,   221,     0,     0,     0,   207,   186,   107,   242,
   246,   108,   243,   219,   221,     0,   179,     0,   245,   186,
   101,   136,   102,     0,     9,     0,    16,     0,   227,     0,
   227,   218,   246,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   672,   674,   678,   680,   683,   685,   690,   692,   695,   697,
   700,   704,   706,   708,   710,   714,   716,   720,   727,   736,
   738,   742,   744,   746,   748,   750,   753,   755,   757,   759,
   761,   767,   769,   773,   776,   779,   782,   784,   787,   790,
   793,   796,   798,   801,   803,   805,   807,   809,   811,   813,
   815,   817,   819,   821,   823,   825,   827,   831,   833,   836,
   839,   841,   843,   845,   847,   849,   851,   853,   855,   857,
   859,   861,   863,   865,   867,   869,   871,   873,   875,   877,
   879,   883,   885,   890,   892,   897,   899,   901,   904,   906,
   908,   910,   915,   917,   919,   923,   927,   929,   931,   933,
   937,   939,   943,   945,   947,   952,   956,   960,   964,   968,
   972,   976,   978,   982,   984,   988,   990,   996,   998,  1002,
  1004,  1008,  1013,  1020,  1022,  1024,  1026,  1030,  1034,  1036,
  1038,  1042,  1044,  1048,  1050,  1052,  1054,  1056,  1058,  1062,
  1064,  1068,  1072,  1074,  1076,  1078,  1082,  1086,  1090,  1092,
  1094,  1096,  1100,  1102,  1104,  1108,  1110,  1114,  1118,  1120,
  1123,  1127,  1129,  1131,  1133,  1139,  1144,  1146,  1151,  1153,
  1157,  1159,  1161,  1163,  1167,  1169,  1173,  1175,  1179,  1181,
  1183,  1185,  1187,  1189,  1193,  1195,  1199,  1203,  1205,  1209,
  1211,  1213,  1215,  1217,  1219,  1221,  1225,  1227,  1229,  1231,
  1233,  1235,  1239,  1241,  1245,  1247,  1251,  1253,  1255,  1260,
  1262,  1266,  1270,  1272,  1276,  1278,  1282,  1284,  1288,  1290,
  1294,  1298,  1300,  1303,  1307,  1309,  1313,  1317,  1321,  1323,
  1327,  1331,  1333,  1337,  1341,  1345,  1355,  1363,  1365,  1367,
  1369,  1371,  1373,  1377,  1379,  1383,  1387,  1389,  1393,  1397,
  1399,  1403,  1405,  1407,  1413,  1421,  1423,  1425,  1427,  1431,
  1433,  1437,  1439,  1443,  1445,  1449,  1451,  1455,  1457,  1461,
  1463,  1465,  1466,  1467,  1469,  1473,  1475,  1477,  1481,  1482,
  1485,  1489,  1490,  1490,  1491,  1492,  1492,  1493,  1494,  1494,
  1497,  1499,  1503,  1504,  1507,  1511,  1512,  1515,  1516,  1519,
  1527,  1529,  1533,  1535,  1539,  1541,  1543,  1547,  1549,  1553,
  1555,  1557,  1561,  1565,  1567,  1571,  1580,  1584,  1586,  1590,
  1592,  1602,  1607,  1614,  1616,  1620,  1624,  1626,  1630,  1632,
  1636,  1638,  1644,  1648,  1651,  1656,  1658,  1662,  1666,  1667,
  1668,  1670,  1671,  1672,  1674,  1675,  1676,  1680,  1682,  1686,
  1688,  1693,  1695
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
"T_unit","T_UNIT_LABEL","T_loop","T_ENDOFFILE","T_REGEX","T_lex","T_mode","T_parser",
"T_prod","T_scanner","T_syn","T_token","T_REDUCE","T_SEP","'='","T_OOASSIGN",
"T_orelse","T_andthen","T_COMPARE","T_FDCOMPARE","T_LMACRO","T_RMACRO","T_FDIN",
"'|'","'#'","T_ADD","T_FDMUL","T_OTHERMUL","','","'~'","'.'","'^'","'@'","T_DEREFF",
"'+'","'-'","'('","')'","'_'","'$'","'['","']'","'{'","'}'","':'","'!'","file",
"queries","queries1","directive","switchList","switch","sequence","phrase","hashes",
"phrase2","procFlags","optFunctorDescriptorList","functorDescriptorList","importDecls",
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
   111,   111,   112,   112,   112,   112,   113,   113,   113,   113,
   113,   114,   114,   114,   114,   115,   115,   116,   116,   117,
   117,   118,   118,   118,   118,   118,   118,   118,   118,   118,
   118,   119,   119,   120,   120,   120,   120,   120,   120,   120,
   120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
   120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
   120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
   120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
   120,   121,   121,   122,   122,   123,   123,   123,   123,   123,
   123,   123,   124,   124,   124,   125,   126,   126,   126,   126,
   127,   127,   128,   128,   128,   129,   130,   131,   132,   133,
   134,   135,   135,   136,   136,   137,   137,   138,   138,   139,
   139,   140,   140,   141,   141,   141,   141,   142,   143,   143,
   143,   144,   144,   145,   145,   145,   145,   145,   145,   146,
   146,   147,   148,   148,   148,   148,   149,   149,   150,   150,
   150,   150,   151,   151,   151,   152,   152,   153,   154,   154,
   154,   155,   155,   155,   155,   156,   157,   157,   158,   158,
   159,   159,   159,   159,   160,   160,   161,   161,   162,   162,
   162,   162,   162,   162,   163,   163,   164,   165,   165,   166,
   166,   166,   166,   166,   166,   166,   167,   167,   167,   167,
   167,   167,   168,   168,   169,   169,   170,   170,   170,   171,
   171,   172,   173,   173,   174,   174,   175,   175,   176,   176,
   177,   177,   177,   177,   178,   178,   179,   180,   181,   181,
   182,   183,   183,   184,   185,   186,   187,   188,   188,   188,
   188,   188,   188,   189,   189,   190,   191,   191,   192,   193,
   193,   194,   194,   194,   195,   196,   196,   196,   196,   197,
   197,   198,   198,   199,   199,   200,   200,   201,   201,   203,
   202,   204,   205,   202,   202,   206,   206,   206,   207,   207,
   208,   210,   211,   209,   212,   213,   209,   214,   215,   209,
   216,   216,   217,   217,   218,   219,   219,   220,   220,   221,
   222,   222,   223,   223,   224,   224,   224,   225,   225,   226,
   226,   226,   227,   228,   228,   229,   229,   230,   230,   231,
   231,   231,   231,   231,   231,   232,   233,   233,   234,   234,
   235,   235,   235,   236,   236,   237,   237,   237,   238,   239,
   237,   240,   241,   237,   242,   243,   237,   244,   244,   245,
   245,   246,   246
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
     5,     0,     2,     0,     1,     4,     6,     4,     4,     4,
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
     2,    16,    13,    14,    15,   227,   124,   234,   232,   233,
   231,   228,   128,   236,   236,   236,   236,   236,   236,    72,
    50,   127,   236,   236,   236,   236,   236,   236,   236,   236,
   236,    51,    71,   236,    49,   126,   236,    48,   125,   236,
   236,   280,   236,   236,   236,   236,   236,     0,    47,    52,
   236,   236,   236,     0,     5,   235,    11,    20,    30,    56,
   236,   236,    62,    45,   229,    46,    53,    54,    55,     0,
    78,    79,    11,   268,     0,     0,    12,    16,    65,     0,
     0,   235,    74,     0,     0,     0,    82,   235,    64,     0,
     0,     0,     0,     0,    82,     0,     0,     0,     0,     0,
   272,     0,     0,   300,     0,   279,     0,   114,     0,     0,
     0,   113,     0,     0,     0,     0,     1,     7,     3,   236,
   236,   236,   236,   106,   107,   108,   236,    21,   236,   236,
   236,    40,   236,   109,   110,   111,   236,   236,   236,   236,
   236,   236,     0,     0,   236,   236,   236,   236,   236,    11,
     4,   269,    18,    19,    17,     0,   225,     0,   167,   169,
   168,     0,   214,   215,   235,   235,     0,     0,     0,    82,
    84,     0,     0,    20,     0,     0,     0,     0,     0,     0,
   118,     0,   169,     0,   281,   270,   298,   299,   277,   301,
   282,   285,   288,   278,   296,   169,   114,     0,    38,    42,
    43,   236,    44,   235,   114,   230,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   129,   129,     0,   103,    93,     0,    93,     6,     0,
   236,     0,   236,   236,   236,   236,   236,   185,   169,     0,
   236,     0,     0,     0,   235,    11,     0,   221,   236,     0,
     0,    83,     0,    85,     0,     0,     0,   236,   236,   236,
     0,   236,   236,   236,   120,   236,   185,   273,   280,     0,
   235,     0,   303,     0,     0,     0,   276,   297,   185,   115,
   236,     0,     0,   116,     0,    22,    23,    24,    25,    29,
    26,    27,    28,    31,    32,    37,    39,    41,    34,    35,
    36,    50,    49,    48,   129,   132,     0,    45,   229,    54,
   132,    84,    84,     0,   140,   103,   141,   236,    84,     0,
   101,    84,    84,   164,     0,   153,     0,   159,     0,   226,
    77,     0,     0,     0,     0,   236,     0,   185,   170,     0,
     0,   213,   236,   216,    11,    11,    10,   235,     0,    76,
   220,   219,   114,   236,     0,     0,     0,    66,    73,    75,
   114,    70,    68,     0,     0,     0,    80,   260,   280,     0,
   300,   350,   351,   235,   235,     0,   236,   318,     0,   313,
   314,   302,   304,   294,   293,     0,   291,     0,     0,     0,
    81,   112,   236,   235,   236,     0,   130,   133,     0,     0,
     0,     0,    92,    90,     0,   104,    96,    89,     0,     0,
    93,     0,    88,    86,   236,     0,   236,   236,   152,   236,
     0,     0,     0,   236,   235,   236,     0,   184,   183,   182,
   175,   178,   179,   180,   181,   175,   114,   114,     0,   236,
   186,     0,   217,   212,     9,     8,     0,   222,   223,     0,
    61,     0,   236,   236,   146,   236,    63,   236,     0,   119,
   156,   121,   236,     0,     0,   300,   301,     0,     0,   308,
   318,   318,     0,   236,   348,   326,   236,   236,   325,   316,
   318,   318,   335,   336,   236,   275,   235,   283,   295,     0,
   286,   289,     0,     0,     0,   238,   239,   240,    57,   117,
    58,    33,   236,   129,   236,    84,   103,     0,    97,   102,
    94,    84,     0,     0,   150,   149,   148,   154,   155,   158,
     0,     0,     0,   236,   172,   175,     0,   173,   171,   174,
   197,   198,   194,   202,   193,   201,   192,   200,   236,     0,
   188,     0,   190,   191,   166,   218,   224,     0,     0,   144,
   143,   142,    67,     0,     0,    69,   264,   261,   262,     0,
   257,   256,   301,   235,   305,   306,   312,   311,   310,     0,
   308,   317,   331,     0,   327,   323,   318,   330,   333,   319,
     0,     0,     0,   280,   236,   320,   324,   236,     0,   315,
   296,   292,   296,   296,   248,   247,     0,     0,     0,   250,
   236,   241,   242,   243,   122,   131,   123,    91,   105,   101,
     0,    99,    87,   165,   151,   162,   160,   163,   147,   176,
   177,     0,     0,   236,   204,     0,   145,     0,   157,     0,
   263,   236,   259,   258,   235,     0,   235,   309,   236,   280,
   328,     0,   339,   342,   345,   348,   296,   331,   318,   318,
   300,   114,   284,   287,   290,     0,     0,     0,     0,     0,
   253,   254,     0,   250,   237,    93,    98,     0,   199,   187,
     0,   139,   138,   137,   209,   208,     0,     0,   204,   211,
   134,   207,   136,   236,   236,   265,   255,     0,   271,     0,
   300,   329,   280,   235,   235,   235,   300,   322,   300,   338,
     0,     0,     0,   246,   266,   252,   249,   251,    95,   100,
   161,   189,     0,     0,   195,   203,   236,   205,    60,    59,
   274,   307,   332,   334,   352,     0,     0,     0,   337,   321,
   349,   244,   245,   267,   211,   207,   196,     0,   235,   340,
   343,   346,   206,   210,   353,   296,   296,   296,   300,   300,
   300,   341,   344,   347,     0,     0,     0
};

static const short yydefgoto[] = {   755,
    54,    55,    56,    77,    78,   112,    58,   294,    59,   169,
   253,   254,   319,   320,   508,   411,   313,   129,   130,   131,
   140,   141,   142,   157,   198,   283,   265,   366,    60,    61,
    62,   306,   399,   307,   314,    89,   456,    79,   420,   325,
   460,   326,   327,   328,    63,   160,   238,   239,   525,   526,
   432,   337,   338,   540,   541,   542,   678,   679,   680,   718,
    83,   243,   163,   164,   167,   168,   158,    64,    65,    66,
    67,    68,    69,   378,    80,    71,   495,   496,   497,   599,
   498,   663,   664,    72,   560,   465,   558,   559,   706,    73,
    74,   269,   184,   369,   104,   477,   478,   194,   274,   591,
   275,   593,   276,   594,   386,   387,   490,   277,   278,   190,
   271,   272,   273,   570,   571,   725,   380,   381,   575,   480,
   481,   576,   577,   578,   482,   579,   694,   746,   695,   747,
   696,   748,   484,   485,   726
};

static const short yypact[] = {   978,
-32768,    -2,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,    63,-32768,-32768,-32768,-32768,-32768,  1499,-32768,-32768,
-32768,-32768,-32768,    11,-32768,  1087,   473,  1293,   190,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   530,
-32768,-32768,   473,    37,   117,   176,-32768,    -2,-32768,  1499,
  1499,  1499,-32768,  1499,  1499,  1499,   180,  1499,-32768,  1499,
  1499,  1499,  1499,  1499,   180,  1499,  1499,  1499,  1499,   175,
-32768,   116,   123,-32768,   105,   175,   175,  1499,  1499,  1499,
  1499,   151,   125,  1499,  1499,   175,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   132,   135,-32768,-32768,-32768,-32768,-32768,   473,
-32768,-32768,-32768,-32768,-32768,    32,   221,   212,   750,   305,
-32768,    28,   229,   253,   218,   241,   250,   289,   209,   180,
   530,   267,   296,  1190,   315,   323,   329,   259,   340,   350,
   383,   362,   305,   331,-32768,-32768,-32768,-32768,-32768,   333,
-32768,-32768,-32768,-32768,   165,   305,  1293,   326,   157,-32768,
-32768,-32768,-32768,   750,  1293,-32768,  1499,  1499,  1499,  1499,
  1499,  1499,  1499,  1499,  1499,  1499,  1499,  1499,  1499,  1499,
  1499,  1602,  1602,  1499,   411,   308,  1499,   308,-32768,  1499,
-32768,  1499,-32768,-32768,-32768,-32768,-32768,   368,   305,  1499,
-32768,  1499,   386,  1499,  1499,   473,  1499,   372,-32768,  1499,
  1499,-32768,   404,-32768,  1499,  1499,  1499,-32768,-32768,-32768,
  1499,-32768,-32768,-32768,   402,-32768,   368,-32768,    88,   373,
   392,   399,   333,    57,    57,    57,-32768,-32768,   368,-32768,
-32768,  1499,   349,  1499,   352,   750,   775,   668,   346,   370,
   246,   407,   407,-32768,   263,   204,-32768,-32768,   198,   204,
   204,   355,   380,   395,  1396,   452,   397,   426,   445,   448,
   452,   341,   530,   459,-32768,   411,-32768,-32768,   530,   391,
   456,   510,   530,   502,   361,    69,   421,   238,  1499,-32768,
-32768,    74,    74,  1499,  1499,-32768,   479,   368,-32768,   477,
  1499,-32768,-32768,-32768,   473,   473,-32768,   509,  1499,-32768,
-32768,   289,  1293,-32768,   480,   538,   540,-32768,-32768,-32768,
  1293,-32768,-32768,  1499,  1499,   542,-32768,   501,    88,   165,
-32768,-32768,-32768,   392,   392,   481,-32768,   525,   544,-32768,
   563,-32768,-32768,-32768,-32768,   482,   507,   483,   491,   428,
-32768,-32768,-32768,   750,-32768,  1499,-32768,-32768,   499,  1499,
   505,  1499,-32768,-32768,   175,-32768,-32768,-32768,   475,   180,
   308,  1499,-32768,-32768,-32768,  1499,-32768,-32768,-32768,-32768,
  1499,  1499,  1499,-32768,-32768,-32768,   361,-32768,-32768,-32768,
    74,   494,-32768,-32768,-32768,    74,  1293,  1293,   782,-32768,
-32768,  1499,-32768,-32768,-32768,-32768,  1499,-32768,-32768,   496,
-32768,  1499,-32768,-32768,-32768,-32768,-32768,-32768,   500,-32768,
   570,-32768,-32768,   180,    44,-32768,   333,   576,   577,    65,
   537,   707,  1499,-32768,   116,-32768,-32768,   201,-32768,-32768,
   763,   707,-32768,   165,-32768,-32768,   392,-32768,-32768,    57,
-32768,-32768,   223,   175,   581,   428,   428,   428,-32768,-32768,
-32768,-32768,-32768,  1396,-32768,   530,   411,   518,    50,-32768,
-32768,   530,  1499,   587,-32768,-32768,-32768,-32768,-32768,-32768,
  1499,  1499,  1499,-32768,-32768,    74,  1499,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,  1499,
   543,   524,-32768,-32768,-32768,-32768,-32768,  1499,   593,-32768,
-32768,-32768,-32768,  1499,  1499,-32768,   519,-32768,   180,   595,
    44,    44,   333,   392,-32768,-32768,-32768,-32768,-32768,   532,
    65,-32768,   222,   559,-32768,-32768,   707,-32768,-32768,-32768,
   175,   360,   363,   373,-32768,-32768,-32768,-32768,   539,-32768,
   165,-32768,   165,   165,-32768,-32768,   560,   562,  1499,   153,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   456,
   175,-32768,-32768,-32768,-32768,     5,   596,   555,-32768,-32768,
   750,   486,   611,-32768,   353,   613,-32768,   614,-32768,  1499,
-32768,-32768,-32768,-32768,   392,   615,   392,-32768,-32768,   373,
-32768,   571,-32768,-32768,-32768,-32768,   165,   165,   707,   707,
-32768,  1499,-32768,-32768,-32768,   193,   193,   617,   175,   193,
-32768,-32768,   618,   153,-32768,   308,   475,  1499,-32768,-32768,
   175,-32768,-32768,-32768,-32768,-32768,   550,    41,   353,   636,
-32768,   445,-32768,-32768,-32768,   750,-32768,   627,-32768,   628,
-32768,-32768,   373,   392,   392,   392,-32768,-32768,-32768,-32768,
   566,   630,   631,-32768,   175,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,    75,   569,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   507,   574,   573,   564,-32768,-32768,
-32768,-32768,-32768,-32768,   636,-32768,-32768,  1499,   392,-32768,
-32768,-32768,-32768,   750,-32768,   165,   165,   165,-32768,-32768,
-32768,-32768,-32768,-32768,   677,   680,-32768
};

static const short yypgoto[] = {-32768,
   625,   -50,-32768,   604,-32768,    91,   339,  -352,   545,   -25,
  -271,   619,  -222,-32768,  -475,    77,  -298,-32768,-32768,-32768,
-32768,-32768,-32768,   376,  -170,   294,-32768,-32768,-32768,-32768,
-32768,  -206,   379,  -439,  -389,  -359,-32768,  -349,   266,    97,
   140,  -340,-32768,  -328,-32768,   608,  -102,-32768,  -371,   189,
-32768,  -166,-32768,-32768,-32768,-32768,    18,-32768,   -10,   -28,
-32768,-32768,   454,-32768,   -45,   460,   472,   227,    35,   247,
-32768,  -159,-32768,   319,   -15,-32768,  -223,-32768,  -460,  -130,
  -456,    47,-32768,-32768,   -31,-32768,   159,-32768,     7,   647,
  -398,-32768,-32768,-32768,  -213,   -37,     1,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,  -236,-32768,    -1,  -518,   -82,  -314,
  -413,   453,  -396,   156,-32768,  -221,   243,-32768,  -333,   251,
-32768,  -449,-32768,  -531,-32768,  -323,-32768,-32768,-32768,-32768,
-32768,-32768,   150,   465,  -497
};


#define YYLAST          1712


static const short yytable[] = {    81,
    82,    84,    85,    86,   105,   323,   119,    87,    88,    90,
    91,    92,    93,    94,    95,    96,   311,   406,    97,   509,
   189,    98,   151,   461,    99,   100,   280,   107,   108,   109,
   110,   111,   587,   612,   285,   114,   115,   116,   388,   389,
   403,   404,   106,   502,   479,   143,   144,   408,   177,   379,
   413,   414,   649,   564,   483,   371,   467,     6,   516,   714,
     9,    10,   310,   310,   528,   317,   561,   515,   562,   178,
     6,    12,   653,   240,   654,   655,   103,    12,   117,    12,
   267,     6,   230,   424,     9,    10,   421,   241,    12,    12,
    57,   231,   426,   279,   551,     6,    75,    76,   397,   229,
   368,   422,    12,   550,   207,   208,   209,   210,   692,    42,
   428,   211,   390,   212,   213,   214,    42,   215,   270,   509,
   153,   216,   217,   218,   219,   220,   221,   641,   697,   224,
   225,   226,   227,   228,   183,   429,   339,   572,   430,   661,
   195,   196,   715,   662,   252,   310,    57,   479,   128,   635,
   206,   563,   468,   469,   620,   466,   317,   483,   611,   384,
   614,   724,   561,   561,   562,   562,   101,   567,   568,   132,
   156,   441,   435,   435,   162,   165,   166,   675,   676,   154,
   172,   173,   450,    53,   166,   677,   282,     6,   511,    12,
   459,   710,   616,   659,   618,   347,   202,   727,   728,   698,
   699,   186,   132,   661,   351,   191,   595,   662,   609,   192,
   132,   193,   187,   188,   461,   329,   132,   331,   332,   333,
   334,   335,   660,   494,   185,   341,   203,   749,   750,   751,
     6,   105,   222,   350,   608,   223,   595,    12,   232,   677,
   613,   745,   358,   359,   360,   233,   362,   363,   364,   317,
   367,   138,   139,   592,   187,   188,   309,   309,   242,   316,
   321,   596,   321,   245,   128,   391,   529,   530,   102,   106,
   244,   435,   602,   603,   604,   132,   435,   509,   133,   134,
   135,   136,   137,   249,   138,   139,   247,   189,   135,   136,
   137,   596,   138,   139,   445,   446,   137,   606,   138,   139,
  -229,  -279,   407,   370,   375,  -279,   250,  -279,   385,   385,
   385,   187,   188,   170,   312,   251,   424,   322,    70,   425,
   439,   170,    12,   318,   234,   426,   255,   444,-32768,-32768,
   340,   105,   126,   127,   162,   345,   700,   348,   451,   309,
   166,   256,   636,   235,   310,   236,   356,   317,   258,   317,
   316,   396,   134,   135,   136,   137,   259,   138,   139,   237,
     6,   471,   260,     9,    10,   261,   435,    12,   145,   106,
     6,   372,   392,   262,    70,   146,   723,   499,   373,   501,
     6,   372,   729,   263,   730,   147,   402,    12,   373,   672,
   416,   417,   418,   148,   419,   266,   170,   149,   585,   513,
   161,   588,    90,   370,   517,   264,   161,   270,   521,   268,
   523,   281,   476,   688,   673,   690,   336,   674,     6,   343,
   159,     9,    10,   113,   545,    12,   159,   123,   124,   125,
   174,   349,   126,   127,   752,   753,   754,   354,    90,   507,
   552,   365,   553,   709,   382,   321,   197,   556,   308,   308,
   377,   315,   204,   205,   393,   675,   676,   127,   581,   395,
   643,   582,   583,  -139,   644,   683,   645,   175,   176,   589,
   398,   179,   180,   181,   182,     2,   410,     3,     4,     5,
   423,   701,     6,   246,   248,     9,    10,   605,  -138,   607,
   639,   409,   506,-32768,   127,   102,   374,   493,   494,    18,
    12,   669,   512,  -137,   569,   400,   573,   317,   619,   452,
   453,   454,   440,   455,   132,   476,   573,   518,   519,   683,
   431,   436,   284,   622,   385,   702,   703,   598,   600,   633,
   634,   308,     6,   372,  -134,   197,   442,   145,   309,    12,
   373,   316,   315,   197,   146,   286,   287,   288,   289,   290,
   291,   292,   293,  -135,   147,   412,  -136,   145,   433,   433,
   305,   305,   148,   346,   146,   639,   149,   405,   447,   650,
   472,   457,   651,   458,   147,   463,   464,   486,   434,   434,
   487,   470,   148,   488,   489,   665,   149,   555,   491,   353,
   415,   134,   135,   136,   137,   102,   138,   139,   492,   361,
   503,   473,   527,   548,   475,   569,   505,   554,   671,   565,
   566,   573,   617,   473,   601,   642,   687,   342,   648,   610,
   615,   624,   394,   691,   625,  -280,   627,   630,   632,  -280,
   355,  -280,   357,   637,   474,   315,   510,   640,   656,   652,
   657,   668,   426,   305,   670,   667,   684,   685,   689,   693,
   704,   707,   717,   199,   200,   201,   206,   433,   713,   682,
   721,   722,   433,   732,   733,   543,   448,   731,   719,   720,
   737,   742,   437,   438,   648,   740,   756,   434,   741,   757,
   118,   155,   434,   573,   573,   544,   666,   500,   150,   401,
   557,   197,   524,   705,   629,   171,   716,   344,   475,   197,
   321,   738,   735,   330,   427,   712,   743,   475,   475,   352,
   708,   734,   284,   682,     6,   372,   443,   631,   574,   597,
   152,    12,   373,   739,   449,   383,   638,   648,   574,   590,
   308,   586,   647,   315,   376,   315,     0,     0,   504,   705,
   462,     0,     0,   522,     0,     0,     0,   736,   122,   123,
   124,   125,   433,     0,   126,   127,     0,     0,   711,   295,
   296,   297,   298,   299,   300,   301,     0,     0,     0,     0,
     6,   372,   434,     0,   324,   197,   197,    12,   373,     0,
     0,     0,     0,   473,     0,   557,     0,     0,     0,     6,
   531,   514,     0,     0,     0,     0,    12,   532,   520,     0,
     0,     0,     0,   475,     0,     0,     0,  -280,   472,   646,
   475,  -280,     0,  -280,     0,     0,    53,   546,   533,   534,
     0,     0,   547,   574,     0,     0,     0,   549,   120,   121,
   122,   123,   124,   125,     0,     0,   126,   127,     0,   473,
     0,   584,   305,   535,   536,     0,   537,   538,   580,     0,
     0,   681,   187,   188,   121,   122,   123,   124,   125,     0,
     0,   126,   127,  -280,     0,   621,   475,  -280,     0,  -280,
     0,     0,   474,     0,     0,   475,   475,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   539,     0,   315,     0,   574,   574,     0,     0,     0,
     0,     0,     0,     0,     0,   681,     0,     0,   324,     0,
     0,     0,     0,     0,     0,   623,     0,     0,     0,   475,
     0,     0,     0,   626,     0,     0,     0,     0,     0,   628,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
   295,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   324,   324,     0,   686,     0,
     0,     0,     0,     0,   658,     0,     0,     0,     1,     0,
     2,     0,     3,     4,     5,     6,     7,     8,     9,    10,
   197,    11,    12,    13,     0,     0,     0,     0,     0,    14,
     0,    15,    16,    17,    18,  -235,    19,     0,     0,     0,
     0,     0,  -235,    20,    21,    22,     0,     0,     0,    23,
    24,    25,  -235,     0,    26,    27,     0,    28,     0,    29,
  -235,    30,     0,    31,  -235,    32,    33,     0,    34,    35,
    36,    37,    38,    39,    40,   -11,     0,     0,     0,    41,
    42,    43,     0,     0,     0,     0,     0,   295,     0,     0,
     0,     0,    44,     0,     0,   324,     0,   324,     0,     0,
     0,    45,     0,     0,    46,    47,   744,     0,    48,     0,
    49,    50,    51,     0,    52,     0,     0,    53,     0,     2,
     0,     3,     4,     5,     6,     7,     8,     9,    10,   324,
    11,    12,    13,     0,     0,     0,     0,     0,    14,     0,
    15,    16,    17,    18,     0,    19,     0,     0,     0,     0,
     0,     0,    20,    21,    22,     0,     0,     0,    23,    24,
    25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
    30,     0,    31,     0,    32,    33,     0,    34,    35,    36,
    37,    38,    39,    40,   -11,     0,     0,     0,    41,    42,
    43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    44,     0,     0,     0,     0,     0,     0,     0,     0,
    45,     0,     0,    46,    47,     0,     0,    48,     0,    49,
    50,    51,     0,    52,     0,     0,    53,     6,     7,     8,
     9,    10,     0,    11,    12,    13,     0,     0,     0,     0,
     0,    14,     0,    15,    16,    17,     0,     0,    19,     0,
     0,     0,     0,     0,     0,    20,    21,    22,     0,     0,
     0,    23,    24,    25,     0,     0,    26,    27,     0,    28,
     0,    29,     0,    30,     0,    31,     0,    32,    33,   257,
    34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
     0,    41,     0,    43,     0,     0,     0,     0,   120,   121,
   122,   123,   124,   125,    44,     0,   126,   127,     0,     0,
     0,     0,     0,    45,     0,     0,    46,    47,     0,     0,
    48,     0,    49,    50,    51,     0,    52,     0,     0,    53,
     6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
     0,     0,     0,     0,    14,     0,    15,    16,    17,     0,
     0,    19,     0,     0,     0,     0,     0,     0,    20,    21,
    22,     0,     0,     0,    23,    24,    25,     0,     0,    26,
    27,     0,    28,     0,    29,     0,    30,     0,    31,     0,
    32,    33,     0,    34,    35,    36,    37,    38,    39,    40,
     0,     0,     0,     0,    41,     0,    43,     0,     0,     0,
     0,   120,   121,   122,   123,   124,   125,    44,     0,   126,
   127,     0,     0,     0,     0,     0,    45,     0,     0,    46,
    47,     0,     0,    48,     0,    49,    50,    51,     0,    52,
     0,     0,    53,     6,     7,     8,     9,    10,     0,    11,
    12,    13,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,   302,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,     0,    34,   303,    36,    37,
   304,    39,    40,     0,     0,     0,     0,    41,     0,    43,
     0,     0,     0,     0,   120,   121,   122,   123,   124,   125,
    44,     0,   126,   127,     0,     0,     0,     0,     0,    45,
     0,     0,    46,    47,     0,     0,    48,     0,    49,    50,
    51,     0,    52,     0,     0,    53,     6,     7,     8,     9,
    10,     0,    11,    12,    13,     0,     0,     0,     0,     0,
    14,     0,    15,    16,    17,     0,     0,    19,     0,     0,
     0,     0,     0,     0,    20,    21,    22,     0,     0,     0,
    23,    24,    25,     0,     0,    26,    27,     0,    28,     0,
    29,     0,    30,     0,    31,     0,    32,    33,     0,    34,
    35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
    41,     0,    43,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    44,     0,     0,     0,     0,     0,     0,
     0,     0,    45,     0,     0,    46,    47,     0,     0,    48,
     0,    49,    50,    51,     0,    52,     0,     0,    53,     6,
     7,     8,     9,    10,     0,    11,    12,    13,     0,     0,
     0,     0,     0,    14,     0,    15,    16,    17,     0,     0,
    19,     0,     0,     0,     0,     0,     0,    20,   302,    22,
     0,     0,     0,    23,    24,    25,     0,     0,    26,    27,
     0,    28,     0,    29,     0,    30,     0,    31,     0,    32,
    33,     0,    34,   303,    36,    37,   304,    39,    40,     0,
     0,     0,     0,    41,     0,    43,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    44,     0,     0,     0,
     0,     0,     0,     0,     0,    45,     0,     0,    46,    47,
     0,     0,    48,     0,    49,    50,    51,     0,    52,     0,
     0,    53
};

static const short yycheck[] = {    15,
    16,    17,    18,    19,    42,   228,    57,    23,    24,    25,
    26,    27,    28,    29,    30,    31,   223,   316,    34,   409,
   103,    37,    73,   364,    40,    41,   197,    43,    44,    45,
    46,    47,   482,   509,   205,    51,    52,    53,   275,   276,
   312,   313,    42,   396,   378,    61,    62,   319,    94,   271,
   322,   323,   584,   467,   378,   269,   371,     8,   418,    19,
    11,    12,   222,   223,   436,   225,   465,   417,   465,    95,
     8,    15,   591,    46,   593,   594,    42,    15,    68,    15,
   183,     8,    51,    79,    11,    12,    18,    60,    15,    15,
     0,    60,    88,   196,   454,     8,    99,   100,   305,   150,
   267,    33,    15,   453,   120,   121,   122,   123,   640,    73,
    37,   127,   279,   129,   130,   131,    73,   133,    75,   509,
     4,   137,   138,   139,   140,   141,   142,   577,   647,   145,
   146,   147,   148,   149,   100,    62,   239,   471,    65,   600,
   106,   107,   102,   600,   170,   305,    56,   481,    58,   563,
   116,   466,   374,   375,   526,   369,   316,   481,   109,   103,
   513,   693,   561,   562,   561,   562,   104,   103,   104,    13,
    80,   338,   332,   333,    84,    85,    86,   103,   104,     4,
    90,    91,   353,   110,    94,   625,   202,     8,   411,    15,
   361,   667,   521,    41,   523,   246,    46,   695,   696,   649,
   650,    79,    13,   664,   250,   101,    14,   664,   507,   105,
    13,   107,    90,    91,   555,   231,    13,   233,   234,   235,
   236,   237,    70,    71,   109,   241,   102,   746,   747,   748,
     8,   269,   101,   249,   506,   101,    14,    15,    18,   679,
   512,   739,   258,   259,   260,    34,   262,   263,   264,   409,
   266,    95,    96,   490,    90,    91,   222,   223,    30,   225,
   226,    69,   228,    46,   174,   281,   437,   438,    42,   269,
    18,   431,   496,   497,   498,    13,   436,   667,    89,    90,
    91,    92,    93,    34,    95,    96,    46,   370,    91,    92,
    93,    69,    95,    96,   345,   346,    93,   504,    95,    96,
    79,   101,   318,   269,   270,   105,    18,   107,   274,   275,
   276,    90,    91,    87,   224,   107,    79,   227,     0,    82,
   336,    95,    15,    16,    20,    88,    60,   343,    83,    84,
   240,   369,    87,    88,   244,   245,   651,   247,   354,   305,
   250,    46,   564,    39,   504,    41,   256,   507,    34,   509,
   316,    89,    90,    91,    92,    93,    34,    95,    96,    55,
     8,   377,    34,    11,    12,   107,   526,    15,    28,   369,
     8,     9,   282,    34,    56,    35,   691,   393,    16,   395,
     8,     9,   697,    34,   699,    45,    46,    15,    16,    37,
    30,    31,    32,    53,    34,    34,   170,    57,   481,   415,
    82,   484,   418,   369,   420,    23,    88,    75,   424,    79,
   426,    86,   378,   635,    62,   637,    49,    65,     8,    34,
    82,    11,    12,    48,   440,    15,    88,    82,    83,    84,
    92,    60,    87,    88,   749,   750,   751,    34,   454,   405,
   456,    40,   458,   666,    46,   411,   108,   463,   222,   223,
    59,   225,   114,   115,   106,   103,   104,    88,   474,   108,
   101,   477,   478,   109,   105,   625,   107,    92,    93,   485,
    19,    96,    97,    98,    99,     3,    21,     5,     6,     7,
    60,   652,     8,   165,   166,    11,    12,   503,   109,   505,
   573,   101,   402,    87,    88,   269,   270,    70,    71,    27,
    15,    16,   412,   109,   470,   109,   472,   667,   524,    30,
    31,    32,    34,    34,    13,   481,   482,   421,   422,   679,
   332,   333,   204,   539,   490,   656,   657,   493,   494,   561,
   562,   305,     8,     9,   109,   197,    60,    28,   504,    15,
    16,   507,   316,   205,    35,   207,   208,   209,   210,   211,
   212,   213,   214,   109,    45,    46,   109,    28,   332,   333,
   222,   223,    53,   245,    35,   648,    57,   109,    60,   585,
    46,    34,   588,    34,    45,    34,    76,    34,   332,   333,
    18,   101,    53,   102,    78,   601,    57,    18,   106,   251,
    89,    90,    91,    92,    93,   369,    95,    96,   108,   261,
   102,    77,   109,   108,   378,   571,   102,   108,   624,    34,
    34,   577,   522,    77,    34,   581,   632,   242,   584,   102,
    34,    79,   284,   639,   101,   101,    34,   109,    34,   105,
   255,   107,   257,   102,   110,   409,   410,    79,    79,   101,
    79,    46,    88,   305,    34,   611,    34,    34,    34,    79,
    34,    34,    17,   109,   110,   111,   622,   431,   109,   625,
    34,    34,   436,    34,    34,   439,   348,   102,   684,   685,
   102,   108,   334,   335,   640,   102,     0,   431,   106,     0,
    56,    78,   436,   649,   650,   439,   610,   394,    70,   311,
   464,   353,   427,   659,   555,    88,   679,   244,   472,   361,
   666,   717,   713,   232,   329,   671,   735,   481,   482,   250,
   664,   705,   394,   679,     8,     9,   341,   559,   472,   493,
    74,    15,    16,   725,   349,   273,   571,   693,   482,   487,
   504,   481,   583,   507,   270,   509,    -1,    -1,   400,   705,
   365,    -1,    -1,   425,    -1,    -1,    -1,   713,    81,    82,
    83,    84,   526,    -1,    87,    88,    -1,    -1,   668,   215,
   216,   217,   218,   219,   220,   221,    -1,    -1,    -1,    -1,
     8,     9,   526,    -1,   230,   437,   438,    15,    16,    -1,
    -1,    -1,    -1,    77,    -1,   559,    -1,    -1,    -1,     8,
     9,   416,    -1,    -1,    -1,    -1,    15,    16,   423,    -1,
    -1,    -1,    -1,   577,    -1,    -1,    -1,   101,    46,   583,
   584,   105,    -1,   107,    -1,    -1,   110,   442,    37,    38,
    -1,    -1,   447,   577,    -1,    -1,    -1,   452,    79,    80,
    81,    82,    83,    84,    -1,    -1,    87,    88,    -1,    77,
    -1,    79,   504,    62,    63,    -1,    65,    66,   473,    -1,
    -1,   625,    90,    91,    80,    81,    82,    83,    84,    -1,
    -1,    87,    88,   101,    -1,   527,   640,   105,    -1,   107,
    -1,    -1,   110,    -1,    -1,   649,   650,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,   110,    -1,   667,    -1,   649,   650,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,   679,    -1,    -1,   364,    -1,
    -1,    -1,    -1,    -1,    -1,   540,    -1,    -1,    -1,   693,
    -1,    -1,    -1,   548,    -1,    -1,    -1,    -1,    -1,   554,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
   396,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,   421,   422,    -1,   630,    -1,
    -1,    -1,    -1,    -1,   599,    -1,    -1,    -1,     1,    -1,
     3,    -1,     5,     6,     7,     8,     9,    10,    11,    12,
   652,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    22,
    -1,    24,    25,    26,    27,    28,    29,    -1,    -1,    -1,
    -1,    -1,    35,    36,    37,    38,    -1,    -1,    -1,    42,
    43,    44,    45,    -1,    47,    48,    -1,    50,    -1,    52,
    53,    54,    -1,    56,    57,    58,    59,    -1,    61,    62,
    63,    64,    65,    66,    67,    68,    -1,    -1,    -1,    72,
    73,    74,    -1,    -1,    -1,    -1,    -1,   513,    -1,    -1,
    -1,    -1,    85,    -1,    -1,   521,    -1,   523,    -1,    -1,
    -1,    94,    -1,    -1,    97,    98,   738,    -1,   101,    -1,
   103,   104,   105,    -1,   107,    -1,    -1,   110,    -1,     3,
    -1,     5,     6,     7,     8,     9,    10,    11,    12,   555,
    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,
    24,    25,    26,    27,    -1,    29,    -1,    -1,    -1,    -1,
    -1,    -1,    36,    37,    38,    -1,    -1,    -1,    42,    43,
    44,    -1,    -1,    47,    48,    -1,    50,    -1,    52,    -1,
    54,    -1,    56,    -1,    58,    59,    -1,    61,    62,    63,
    64,    65,    66,    67,    68,    -1,    -1,    -1,    72,    73,
    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    94,    -1,    -1,    97,    98,    -1,    -1,   101,    -1,   103,
   104,   105,    -1,   107,    -1,    -1,   110,     8,     9,    10,
    11,    12,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,
    -1,    22,    -1,    24,    25,    26,    -1,    -1,    29,    -1,
    -1,    -1,    -1,    -1,    -1,    36,    37,    38,    -1,    -1,
    -1,    42,    43,    44,    -1,    -1,    47,    48,    -1,    50,
    -1,    52,    -1,    54,    -1,    56,    -1,    58,    59,    60,
    61,    62,    63,    64,    65,    66,    67,    -1,    -1,    -1,
    -1,    72,    -1,    74,    -1,    -1,    -1,    -1,    79,    80,
    81,    82,    83,    84,    85,    -1,    87,    88,    -1,    -1,
    -1,    -1,    -1,    94,    -1,    -1,    97,    98,    -1,    -1,
   101,    -1,   103,   104,   105,    -1,   107,    -1,    -1,   110,
     8,     9,    10,    11,    12,    -1,    14,    15,    16,    -1,
    -1,    -1,    -1,    -1,    22,    -1,    24,    25,    26,    -1,
    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,
    38,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,    47,
    48,    -1,    50,    -1,    52,    -1,    54,    -1,    56,    -1,
    58,    59,    -1,    61,    62,    63,    64,    65,    66,    67,
    -1,    -1,    -1,    -1,    72,    -1,    74,    -1,    -1,    -1,
    -1,    79,    80,    81,    82,    83,    84,    85,    -1,    87,
    88,    -1,    -1,    -1,    -1,    -1,    94,    -1,    -1,    97,
    98,    -1,    -1,   101,    -1,   103,   104,   105,    -1,   107,
    -1,    -1,   110,     8,     9,    10,    11,    12,    -1,    14,
    15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,
    25,    26,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
    -1,    36,    37,    38,    -1,    -1,    -1,    42,    43,    44,
    -1,    -1,    47,    48,    -1,    50,    -1,    52,    -1,    54,
    -1,    56,    -1,    58,    59,    -1,    61,    62,    63,    64,
    65,    66,    67,    -1,    -1,    -1,    -1,    72,    -1,    74,
    -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,    84,
    85,    -1,    87,    88,    -1,    -1,    -1,    -1,    -1,    94,
    -1,    -1,    97,    98,    -1,    -1,   101,    -1,   103,   104,
   105,    -1,   107,    -1,    -1,   110,     8,     9,    10,    11,
    12,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,
    22,    -1,    24,    25,    26,    -1,    -1,    29,    -1,    -1,
    -1,    -1,    -1,    -1,    36,    37,    38,    -1,    -1,    -1,
    42,    43,    44,    -1,    -1,    47,    48,    -1,    50,    -1,
    52,    -1,    54,    -1,    56,    -1,    58,    59,    -1,    61,
    62,    63,    64,    65,    66,    67,    -1,    -1,    -1,    -1,
    72,    -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    94,    -1,    -1,    97,    98,    -1,    -1,   101,
    -1,   103,   104,   105,    -1,   107,    -1,    -1,   110,     8,
     9,    10,    11,    12,    -1,    14,    15,    16,    -1,    -1,
    -1,    -1,    -1,    22,    -1,    24,    25,    26,    -1,    -1,
    29,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    38,
    -1,    -1,    -1,    42,    43,    44,    -1,    -1,    47,    48,
    -1,    50,    -1,    52,    -1,    54,    -1,    56,    -1,    58,
    59,    -1,    61,    62,    63,    64,    65,    66,    67,    -1,
    -1,    -1,    -1,    72,    -1,    74,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    94,    -1,    -1,    97,    98,
    -1,    -1,   101,    -1,   103,   104,   105,    -1,   107,    -1,
    -1,   110
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

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         -2
#define YYEOF           0
#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL          goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do                                                              \
  if (yychar == YYEMPTY && yylen == 1)                          \
    { yychar = (token), yylval = (value);                       \
      yychar1 = YYTRANSLATE (yychar);                           \
      YYPOPSTACK;                                               \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    { yyerror ("syntax error: cannot back up"); YYERROR; }      \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YYPURE
#define YYLEX           yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX           yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX           yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX           yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX           yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int     yychar;                 /*  the lookahead symbol                */
YYSTYPE yylval;                 /*  the semantic value of the           */
                                /*  lookahead symbol                    */

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;                 /*  location data for the lookahead     */
                                /*  symbol                              */
#endif

int yynerrs;                    /*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;                    /*  nonzero means print parse trace     */
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks       */

#ifndef YYINITDEPTH
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

#if __GNUC__ > 1                /* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)      __builtin_memcpy(TO,FROM,COUNT)
#else                           /* not GNU C or C++ */
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
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;              /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YYSTYPE yyvsa[YYINITDEPTH];   /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;        /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];   /*  the location stack                  */
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

  YYSTYPE yyval;                /*  the variable used to return         */
                                /*  semantic values from the action     */
                                /*  routines                            */

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;             /* Cause a token to be read.  */

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

  if (yychar <= 0)              /* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;           /* Don't call YYLEX any more */

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
{ yyval.t = newCTerm(PA_fLoop,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 81:
{ yyval.t = newCTerm(PA_fMacro,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 82:
{ yyval.t = AtomNil; ;
    break;}
case 83:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 84:
{ yyval.t = AtomNil; ;
    break;}
case 85:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 86:
{ yyval.t = oz_cons(newCTerm(PA_fRequire,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 87:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 88:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-1].t,
                                           newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 89:
{ yyval.t = oz_cons(newCTerm(PA_fImport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 90:
{ yyval.t = oz_cons(newCTerm(PA_fExport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 91:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 92:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-1].t,
                                           newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 93:
{ yyval.t = AtomNil; ;
    break;}
case 94:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-2].t,AtomNil,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 95:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 96:
{ yyval.t = newCTerm(PA_fVar,OZ_atom(xytext),yyvsp[0].t); ;
    break;}
case 97:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 98:
{ yyval.t = oz_mklist(oz_pair2(yyvsp[0].t,yyvsp[-2].t)); ;
    break;}
case 99:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 100:
{ yyval.t = oz_cons(oz_pair2(yyvsp[-1].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 101:
{ yyval.t = PA_fNoImportAt; ;
    break;}
case 102:
{ yyval.t = newCTerm(PA_fImportAt,yyvsp[0].t); ;
    break;}
case 103:
{ yyval.t = AtomNil; ;
    break;}
case 104:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 105:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,
                                           newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t)),yyvsp[0].t); ;
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
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 111:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 112:
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 113:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 114:
{ yyval.t = AtomNil; ;
    break;}
case 115:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 116:
{ yyval.t = newCTerm(PA_fAtom,AtomNil,yyvsp[0].t); ;
    break;}
case 117:
{ yyval.t = newCTerm(PA_fRecord,
                                  newCTerm(PA_fAtom,AtomCons,yyvsp[-2].t),
                                  oz_mklist(yyvsp[-1].t,yyvsp[0].t)); ;
    break;}
case 118:
{ yyval.t = PA_fNoCatch; ;
    break;}
case 119:
{ yyval.t = newCTerm(PA_fCatch,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 120:
{ yyval.t = PA_fNoFinally; ;
    break;}
case 121:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 122:
{
                    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
                                  newCTerm(PA_fAtom,yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
                  ;
    break;}
case 123:
{
                    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
                                  makeVar(yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
                  ;
    break;}
case 124:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 125:
{ yyval.t = NameUnit; ;
    break;}
case 126:
{ yyval.t = NameTrue; ;
    break;}
case 127:
{ yyval.t = NameFalse; ;
    break;}
case 128:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 129:
{ yyval.t = AtomNil; ;
    break;}
case 130:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 131:
{ yyval.t = oz_cons(newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 132:
{ yyval.t = NameFalse; ;
    break;}
case 133:
{ yyval.t = NameTrue; ;
    break;}
case 134:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 135:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 136:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 137:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 138:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 139:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 140:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 141:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 142:
{ yyval.t = newCTerm(PA_fBoolCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 143:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 144:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 145:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 146:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 147:
{ checkDeprecation(yyvsp[-3].t);
                    yyval.t = newCTerm(PA_fBoolCase,yyvsp[-5].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-6].t,yyvsp[0].t));
                  ;
    break;}
case 148:
{ yyval.t = newCTerm(PA_fCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
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
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 154:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 155:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 156:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 157:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 158:
{ yyval.t = newCTerm(PA_fCaseClause,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 159:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 160:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-3].t,
                                  newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 161:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-5].t,yyvsp[-2].t,yyvsp[0].t,yyvsp[-3].t); ;
    break;}
case 162:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 163:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 164:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 165:
{ yyval.t = newCTerm(PA_fRecord,
                                  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
                                  oz_cons(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 166:
{ yyval.t = newCTerm(PA_fClass,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 167:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 168:
{ yyval.t = newCTerm(PA_fDollar,yyvsp[0].t); ;
    break;}
case 169:
{ yyval.t = AtomNil; ;
    break;}
case 170:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 171:
{ yyval.t = newCTerm(PA_fFrom,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 172:
{ yyval.t = newCTerm(PA_fAttr,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 173:
{ yyval.t = newCTerm(PA_fFeat,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 174:
{ yyval.t = newCTerm(PA_fProp,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 175:
{ yyval.t = AtomNil; ;
    break;}
case 176:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 177:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 178:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 179:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 180:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 181:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 182:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 183:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 184:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 185:
{ yyval.t = AtomNil; ;
    break;}
case 186:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 187:
{ yyval.t = newCTerm(PA_fMeth,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-3].t); ;
    break;}
case 188:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 189:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
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
{ yyval.t = newCTerm(PA_fRecord,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 196:
{ yyval.t = newCTerm(PA_fOpenRecord,yyvsp[-4].t,yyvsp[-2].t); ;
    break;}
case 197:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 198:
{ yyval.t = makeVar(xytext); ;
    break;}
case 199:
{ yyval.t = newCTerm(PA_fEscape,makeVar(xytext),yyvsp[-1].t); ;
    break;}
case 200:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 201:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 202:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 203:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 204:
{ yyval.t = AtomNil; ;
    break;}
case 205:
{ yyval.t = newCTerm(PA_fMethArg,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 206:
{ yyval.t = newCTerm(PA_fMethColonArg,yyvsp[-3].t,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 207:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 208:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 209:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 210:
{ yyval.t = newCTerm(PA_fDefault,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 211:
{ yyval.t = PA_fNoDefault; ;
    break;}
case 212:
{ yyval.t = newCTerm(PA_fCond,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 213:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 214:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 215:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 216:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 217:
{ yyval.t = newCTerm(PA_fClause,newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 218:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 219:
{ yyval.t = oz_mklist(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 220:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 221:
{ yyval.t = newCTerm(PA_fClause,
                                  newCTerm(PA_fSkip,yyvsp[0].t),
                                  yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 222:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-3].t,yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 223:
{ yyval.t = newCTerm(PA_fClause,
                                  newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 224:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 225:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 226:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 227:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 228:
{ yyval.t = makeVar(xytext); ;
    break;}
case 229:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 230:
{ yyval.t = newCTerm(PA_fEscape,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 231:
{ yyval.t = makeString(xytext,pos()); ;
    break;}
case 232:
{ yyval.t = makeInt(xytext,pos()); ;
    break;}
case 233:
{ yyval.t = makeInt(xytext[0],pos()); ;
    break;}
case 234:
{ yyval.t = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); ;
    break;}
case 235:
{ yyval.t = pos(); ;
    break;}
case 236:
{ yyval.t = pos(); ;
    break;}
case 237:
{ OZ_Term prefix =
                      scannerPrefix? scannerPrefix: PA_zy;
                    yyval.t = newCTerm(PA_fScanner,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,prefix,
                                  makeLongPos(yyvsp[-6].t,yyvsp[0].t)); ;
    break;}
case 238:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 239:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 240:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 241:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 242:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 243:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 244:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 245:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 246:
{ yyval.t = newCTerm(PA_fLexicalRule,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 247:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 248:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 249:
{ yyval.t = newCTerm(PA_fMode,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 250:
{ yyval.t = AtomNil; ;
    break;}
case 251:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 252:
{ yyval.t = newCTerm(PA_fInheritedModes,yyvsp[0].t); ;
    break;}
case 253:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 254:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 255:
{ OZ_Term expect = parserExpect? parserExpect: newSmallInt(0);
                    yyval.t = newCTerm(PA_fParser,yyvsp[-6].t,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,expect,
                                  makeLongPos(yyvsp[-7].t,yyvsp[0].t)); ;
    break;}
case 256:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 257:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 258:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 259:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 260:
{ yyval.t = newCTerm(PA_fToken,AtomNil); ;
    break;}
case 261:
{ yyval.t = newCTerm(PA_fToken,yyvsp[0].t); ;
    break;}
case 262:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 263:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 264:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 265:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 266:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 267:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 268:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 269:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 270:
{ *prodKey[depth]++ = '='; ;
    break;}
case 271:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 272:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 273:
{ *prodKey[depth]++ = '='; ;
    break;}
case 274:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 275:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,PA_none); ;
    break;}
case 276:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 277:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 278:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 281:
{ prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg(yyvsp[-1].t,0))); ;
    break;}
case 282:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 283:
{ depth--; ;
    break;}
case 284:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 285:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 286:
{ depth--; ;
    break;}
case 287:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 288:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 289:
{ depth--; ;
    break;}
case 290:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 291:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 292:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 293:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 294:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 295:
{ *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; ;
    break;}
case 298:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 299:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 300:
{ *prodKey[depth] = '\0';
                    yyval.t = oz_pair2(prodName[depth],OZ_string(prodKeyBuffer[depth]));
                    prodName[depth] = PA_none;
                    prodKey[depth] = prodKeyBuffer[depth];
                  ;
    break;}
case 301:
{ yyval.t = AtomNil; ;
    break;}
case 302:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 303:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 304:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 305:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 306:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 307:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 308:
{ yyval.t = AtomNil; ;
    break;}
case 309:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 310:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 311:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 312:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 313:
{ yyval.t = newCTerm(PA_fSynAlternative, yyvsp[0].t); ;
    break;}
case 314:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 315:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 316:
{ OZ_Term t = yyvsp[0].t;
                    while (terms[depth]) {
                      t = oz_cons(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
                      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
                    }
                    yyval.t = newCTerm(PA_fSynSequence, decls[depth], t, yyvsp[-1].t);
                    decls[depth] = AtomNil;
                  ;
    break;}
case 317:
{ yyval.t = newCTerm(PA_fSynSequence, AtomNil, yyvsp[0].t, yyvsp[-1].t); ;
    break;}
case 318:
{ yyval.t = AtomNil; ;
    break;}
case 319:
{ yyval.t = oz_mklist(newCTerm(PA_fSynAction,yyvsp[0].t)); ;
    break;}
case 320:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 321:
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
case 322:
{ yyval.t = oz_cons(newCTerm(PA_fSynAssignment, terms[depth]->term, yyvsp[-1].t),
                                  yyvsp[0].t);
                    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
                  ;
    break;}
case 323:
{ while (terms[depth]) {
                      decls[depth] = oz_cons(terms[depth]->term, decls[depth]);
                      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
                    }
                    yyval.t = yyvsp[0].t;
                  ;
    break;}
case 324:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 325:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 326:
{ terms[depth] = new TermNode(yyvsp[0].t, terms[depth]); ;
    break;}
case 327:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 328:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 329:
{ yyval.t = newCTerm(PA_fSynAssignment,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 330:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 331:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 332:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
                                  oz_cons(newCTerm(PA_fSynApplication,yyvsp[-3].t,
                                                    AtomNil),
                                           AtomNil),yyvsp[-1].t);
                  ;
    break;}
case 333:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 334:
{ yyval.t = newCTerm(PA_fSynAssignment,
                                  newCTerm(PA_fEscape,yyvsp[-2].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 335:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 336:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 337:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
                                  oz_mklist(yyvsp[-2].t),yyvsp[-3].t);
                  ;
    break;}
case 338:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
                                  oz_mklist(yyvsp[-3].t),yyvsp[-1].t);
                  ;
    break;}
case 339:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 340:
{ depth--; ;
    break;}
case 341:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 342:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 343:
{ depth--; ;
    break;}
case 344:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 345:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 346:
{ depth--; ;
    break;}
case 347:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 348:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 349:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[-4].t,yyvsp[-1].t); ;
    break;}
case 350:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 351:
{ yyval.t = makeVar(xytext); ;
    break;}
case 352:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 353:
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

  yyerrstatus = 3;              /* Each real token shifted decrements this */

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
