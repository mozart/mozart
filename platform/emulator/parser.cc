
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

OZ_Term _PA_AtomTab[106];

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



#define YYFINAL         763
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
   326,   332,   338,   339,   342,   343,   345,   350,   357,   362,
   367,   372,   379,   384,   385,   389,   396,   399,   401,   405,
   408,   413,   414,   417,   418,   421,   426,   428,   430,   432,
   434,   436,   438,   443,   445,   446,   449,   451,   455,   456,
   460,   461,   464,   472,   480,   482,   484,   486,   488,   490,
   491,   494,   499,   500,   502,   504,   506,   508,   510,   512,
   514,   516,   518,   525,   528,   531,   535,   537,   545,   552,
   555,   558,   562,   564,   566,   570,   574,   576,   580,   584,
   586,   591,   598,   603,   608,   610,   615,   623,   625,   627,
   628,   631,   636,   641,   646,   651,   652,   655,   659,   661,
   663,   665,   667,   669,   671,   673,   674,   677,   683,   685,
   690,   692,   694,   696,   698,   700,   705,   711,   713,   715,
   719,   721,   723,   725,   728,   729,   732,   737,   739,   741,
   743,   747,   748,   754,   757,   758,   760,   764,   769,   775,
   779,   783,   786,   791,   796,   802,   804,   808,   810,   812,
   814,   818,   820,   822,   824,   826,   827,   828,   837,   839,
   841,   843,   846,   849,   852,   858,   864,   869,   871,   873,
   878,   879,   882,   885,   887,   889,   899,   901,   903,   906,
   909,   910,   913,   915,   918,   920,   924,   926,   929,   931,
   934,   935,   945,   946,   947,   958,   965,   969,   972,   975,
   977,   978,   981,   982,   983,   990,   991,   992,   999,  1000,
  1001,  1008,  1010,  1014,  1016,  1018,  1020,  1021,  1023,  1025,
  1027,  1028,  1029,  1032,  1034,  1037,  1042,  1047,  1055,  1056,
  1059,  1061,  1063,  1065,  1067,  1069,  1073,  1076,  1080,  1081,
  1084,  1087,  1093,  1098,  1101,  1104,  1106,  1108,  1110,  1113,
  1117,  1119,  1121,  1126,  1128,  1134,  1136,  1138,  1144,  1149,
  1150,  1151,  1161,  1162,  1163,  1173,  1174,  1175,  1185,  1187,
  1193,  1195,  1197,  1199
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
    34,   186,     0,   187,     0,   195,     0,    67,   186,   136,
    46,   117,    34,   186,     0,    67,   186,   117,    34,   186,
     0,    85,   186,   136,    86,   186,     0,     0,   179,   121,
     0,     0,   123,     0,    57,   186,   124,   122,     0,    53,
   186,   117,    46,   117,   122,     0,    53,   186,   117,   122,
     0,    45,   186,   124,   122,     0,    35,   186,   128,   122,
     0,    28,   186,   117,    46,   117,   122,     0,    28,   186,
   117,   122,     0,     0,   180,   127,   124,     0,   125,   101,
   126,   102,   127,   124,     0,    16,   186,     0,   146,     0,
   146,   109,   180,     0,   146,   126,     0,   146,   109,   180,
   126,     0,     0,    21,   179,     0,     0,   180,   128,     0,
   146,   109,   180,   128,     0,    83,     0,    84,     0,    87,
     0,    90,     0,    91,     0,    92,     0,   117,    46,   186,
   117,     0,   117,     0,     0,   118,   136,     0,   185,     0,
   185,   118,   137,     0,     0,    23,   186,   152,     0,     0,
    40,   135,     0,   141,   186,   101,   143,   144,   102,   186,
     0,   142,   186,   101,   143,   144,   102,   186,     0,     9,
     0,    66,     0,    63,     0,    38,     0,    16,     0,     0,
   118,   143,     0,   145,   109,   118,   143,     0,     0,    19,
     0,   179,     0,   180,     0,   183,     0,    65,     0,    62,
     0,    37,     0,   179,     0,   183,     0,   186,   117,    60,
   135,   148,   186,     0,    32,   147,     0,    31,   149,     0,
    30,   135,    34,     0,    34,     0,   186,   117,    60,   186,
   135,   150,   186,     0,   186,   117,    51,   151,   150,   186,
     0,    32,   147,     0,    31,   149,     0,    30,   135,    34,
     0,    34,     0,   153,     0,   153,    18,   151,     0,   153,
    33,   151,     0,   153,     0,   153,    18,   152,     0,   154,
    60,   135,     0,   155,     0,   155,    82,   185,   117,     0,
   155,    82,   185,   117,    46,   117,     0,   155,    79,   186,
   155,     0,   155,    88,   186,   155,     0,   120,     0,   120,
    89,   186,   119,     0,    25,   186,   157,   158,   163,    34,
   186,     0,   118,     0,   185,     0,     0,   159,   158,     0,
    41,   186,   118,   136,     0,    20,   186,   161,   160,     0,
    39,   186,   161,   160,     0,    55,   186,   118,   136,     0,
     0,   161,   160,     0,   162,   109,   118,     0,   162,     0,
   179,     0,   181,     0,   183,     0,    65,     0,    62,     0,
    37,     0,     0,   164,   163,     0,    49,   186,   165,   135,
    34,     0,   166,     0,   166,    79,   186,   180,     0,   179,
     0,   181,     0,    65,     0,    62,     0,    37,     0,   167,
   101,   168,   102,     0,   167,   101,   168,    19,   102,     0,
     9,     0,    16,     0,   110,   186,    16,     0,    66,     0,
    63,     0,    38,     0,   169,   168,     0,     0,   170,   171,
     0,   145,   109,   170,   171,     0,   180,     0,   104,     0,
   103,     0,    17,   186,   118,     0,     0,   186,   174,   173,
    34,   186,     0,    30,   135,     0,     0,   175,     0,   175,
    18,   174,     0,   117,    60,   186,   135,     0,   117,    46,
   117,    60,   135,     0,   177,    18,   177,     0,   177,    18,
   176,     0,   117,   185,     0,   117,    46,   117,   185,     0,
   117,   185,    60,   135,     0,   117,    46,   117,    60,   135,
     0,   135,     0,   135,    18,   178,     0,     8,     0,    15,
     0,   180,     0,   110,   186,   180,     0,    14,     0,    11,
     0,    12,     0,    10,     0,     0,     0,    74,   186,   180,
   158,   163,   188,    34,   186,     0,   189,     0,   190,     0,
   192,     0,   189,   188,     0,   190,   188,     0,   192,   188,
     0,    70,   179,    79,   191,    34,     0,    70,   180,    79,
   191,    34,     0,    70,   191,   135,    34,     0,    69,     0,
    14,     0,    71,   180,   193,    34,     0,     0,   194,   193,
     0,    41,   200,     0,   190,     0,   192,     0,    72,   186,
   180,   158,   163,   197,   196,    34,   186,     0,   224,     0,
   202,     0,   224,   196,     0,   202,   196,     0,     0,    76,
   198,     0,   199,     0,   199,   198,     0,   179,     0,   179,
   109,   118,     0,   180,     0,   180,   200,     0,   202,     0,
   202,   201,     0,     0,    73,   180,    79,   203,   206,   221,
   222,   227,    34,     0,     0,     0,    73,   104,   204,    79,
   205,   206,   221,   222,   227,    34,     0,    73,   206,   221,
   222,   227,    34,     0,   208,   180,   219,     0,   180,   220,
     0,   207,   209,     0,   208,     0,     0,   179,   109,     0,
     0,     0,   101,   210,   216,   102,   211,   219,     0,     0,
     0,   105,   212,   216,   106,   213,   219,     0,     0,     0,
   107,   214,   216,   108,   215,   219,     0,   217,     0,   217,
   218,   216,     0,   180,     0,   103,     0,    78,     0,     0,
   220,     0,    90,     0,    91,     0,     0,     0,   223,    46,
     0,   224,     0,   224,   223,     0,    75,   179,   227,    34,
     0,    75,   180,   227,    34,     0,    75,   245,   101,   225,
   102,   227,    34,     0,     0,   226,   225,     0,   180,     0,
   104,     0,   103,     0,   228,     0,   229,     0,   229,    18,
   228,     0,   185,   231,     0,    59,   186,   230,     0,     0,
    77,   135,     0,   232,   231,     0,   232,   220,   186,   233,
   221,     0,   232,    79,   235,   233,     0,    46,   233,     0,
   236,   233,     0,   230,     0,   180,     0,   230,     0,   234,
   233,     0,   181,    79,   235,     0,   235,     0,   180,     0,
   180,   220,   186,   221,     0,   237,     0,   110,   186,   180,
    79,   235,     0,   237,     0,   244,     0,   208,   186,   244,
   219,   221,     0,   244,   220,   186,   221,     0,     0,     0,
   207,   186,   101,   238,   246,   102,   239,   219,   221,     0,
     0,     0,   207,   186,   105,   240,   246,   106,   241,   219,
   221,     0,     0,     0,   207,   186,   107,   242,   246,   108,
   243,   219,   221,     0,   179,     0,   245,   186,   101,   136,
   102,     0,     9,     0,    16,     0,   227,     0,   227,   218,
   246,     0
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
   879,   881,   885,   887,   892,   894,   899,   901,   903,   906,
   908,   910,   912,   917,   919,   921,   925,   929,   931,   933,
   935,   939,   941,   945,   947,   949,   954,   958,   962,   966,
   970,   974,   978,   980,   984,   986,   990,   992,   998,  1000,
  1004,  1006,  1010,  1015,  1022,  1024,  1026,  1028,  1032,  1036,
  1038,  1040,  1044,  1046,  1050,  1052,  1054,  1056,  1058,  1060,
  1064,  1066,  1070,  1074,  1076,  1078,  1080,  1084,  1088,  1092,
  1094,  1096,  1098,  1102,  1104,  1106,  1110,  1112,  1116,  1120,
  1122,  1125,  1129,  1131,  1133,  1135,  1141,  1146,  1148,  1153,
  1155,  1159,  1161,  1163,  1165,  1169,  1171,  1175,  1177,  1181,
  1183,  1185,  1187,  1189,  1191,  1195,  1197,  1201,  1205,  1207,
  1211,  1213,  1215,  1217,  1219,  1221,  1223,  1227,  1229,  1231,
  1233,  1235,  1237,  1241,  1243,  1247,  1249,  1253,  1255,  1257,
  1262,  1264,  1268,  1272,  1274,  1278,  1280,  1284,  1286,  1290,
  1292,  1296,  1300,  1302,  1305,  1309,  1311,  1315,  1319,  1323,
  1325,  1329,  1333,  1335,  1339,  1343,  1347,  1357,  1365,  1367,
  1369,  1371,  1373,  1375,  1379,  1381,  1385,  1389,  1391,  1395,
  1399,  1401,  1405,  1407,  1409,  1415,  1423,  1425,  1427,  1429,
  1433,  1435,  1439,  1441,  1445,  1447,  1451,  1453,  1457,  1459,
  1463,  1465,  1467,  1468,  1469,  1471,  1475,  1477,  1479,  1483,
  1484,  1487,  1491,  1492,  1492,  1493,  1494,  1494,  1495,  1496,
  1496,  1499,  1501,  1505,  1506,  1509,  1513,  1514,  1517,  1518,
  1521,  1529,  1531,  1535,  1537,  1541,  1543,  1545,  1549,  1551,
  1555,  1557,  1559,  1563,  1567,  1569,  1573,  1582,  1586,  1588,
  1592,  1594,  1604,  1609,  1616,  1618,  1622,  1626,  1628,  1632,
  1634,  1638,  1640,  1646,  1650,  1653,  1658,  1660,  1664,  1668,
  1669,  1670,  1672,  1673,  1674,  1676,  1677,  1678,  1682,  1684,
  1688,  1690,  1695,  1697
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
   120,   120,   121,   121,   122,   122,   123,   123,   123,   123,
   123,   123,   123,   124,   124,   124,   125,   126,   126,   126,
   126,   127,   127,   128,   128,   128,   129,   130,   131,   132,
   133,   134,   135,   135,   136,   136,   137,   137,   138,   138,
   139,   139,   140,   140,   141,   141,   141,   141,   142,   143,
   143,   143,   144,   144,   145,   145,   145,   145,   145,   145,
   146,   146,   147,   148,   148,   148,   148,   149,   149,   150,
   150,   150,   150,   151,   151,   151,   152,   152,   153,   154,
   154,   154,   155,   155,   155,   155,   156,   157,   157,   158,
   158,   159,   159,   159,   159,   160,   160,   161,   161,   162,
   162,   162,   162,   162,   162,   163,   163,   164,   165,   165,
   166,   166,   166,   166,   166,   166,   166,   167,   167,   167,
   167,   167,   167,   168,   168,   169,   169,   170,   170,   170,
   171,   171,   172,   173,   173,   174,   174,   175,   175,   176,
   176,   177,   177,   177,   177,   178,   178,   179,   180,   181,
   181,   182,   183,   183,   184,   185,   186,   187,   188,   188,
   188,   188,   188,   188,   189,   189,   190,   191,   191,   192,
   193,   193,   194,   194,   194,   195,   196,   196,   196,   196,
   197,   197,   198,   198,   199,   199,   200,   200,   201,   201,
   203,   202,   204,   205,   202,   202,   206,   206,   206,   207,
   207,   208,   210,   211,   209,   212,   213,   209,   214,   215,
   209,   216,   216,   217,   217,   218,   219,   219,   220,   220,
   221,   222,   222,   223,   223,   224,   224,   224,   225,   225,
   226,   226,   226,   227,   228,   228,   229,   229,   230,   230,
   231,   231,   231,   231,   231,   231,   232,   233,   233,   234,
   234,   235,   235,   235,   236,   236,   237,   237,   237,   238,
   239,   237,   240,   241,   237,   242,   243,   237,   244,   244,
   245,   245,   246,   246
};

static const short yyr2[] = {     0,
     2,     1,     2,     2,     1,     3,     2,     6,     6,     5,
     0,     2,     1,     1,     1,     0,     2,     2,     2,     1,
     2,     4,     4,     4,     4,     4,     4,     4,     4,     1,
     4,     1,     3,     4,     4,     4,     4,     3,     4,     2,
     4,     3,     3,     3,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     6,     6,    10,    10,
     6,     1,     6,     2,     2,     5,     7,     5,     7,     5,
     1,     1,     5,     2,     5,     5,     5,     1,     1,     7,
     5,     5,     0,     2,     0,     1,     4,     6,     4,     4,
     4,     6,     4,     0,     3,     6,     2,     1,     3,     2,
     4,     0,     2,     0,     2,     4,     1,     1,     1,     1,
     1,     1,     4,     1,     0,     2,     1,     3,     0,     3,
     0,     2,     7,     7,     1,     1,     1,     1,     1,     0,
     2,     4,     0,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     6,     2,     2,     3,     1,     7,     6,     2,
     2,     3,     1,     1,     3,     3,     1,     3,     3,     1,
     4,     6,     4,     4,     1,     4,     7,     1,     1,     0,
     2,     4,     4,     4,     4,     0,     2,     3,     1,     1,
     1,     1,     1,     1,     1,     0,     2,     5,     1,     4,
     1,     1,     1,     1,     1,     4,     5,     1,     1,     3,
     1,     1,     1,     2,     0,     2,     4,     1,     1,     1,
     3,     0,     5,     2,     0,     1,     3,     4,     5,     3,
     3,     2,     4,     4,     5,     1,     3,     1,     1,     1,
     3,     1,     1,     1,     1,     0,     0,     8,     1,     1,
     1,     2,     2,     2,     5,     5,     4,     1,     1,     4,
     0,     2,     2,     1,     1,     9,     1,     1,     2,     2,
     0,     2,     1,     2,     1,     3,     1,     2,     1,     2,
     0,     9,     0,     0,    10,     6,     3,     2,     2,     1,
     0,     2,     0,     0,     6,     0,     0,     6,     0,     0,
     6,     1,     3,     1,     1,     1,     0,     1,     1,     1,
     0,     0,     2,     1,     2,     4,     4,     7,     0,     2,
     1,     1,     1,     1,     1,     3,     2,     3,     0,     2,
     2,     5,     4,     2,     2,     1,     1,     1,     2,     3,
     1,     1,     4,     1,     5,     1,     1,     5,     4,     0,
     0,     9,     0,     0,     9,     0,     0,     9,     1,     5,
     1,     1,     1,     3
};

static const short yydefact[] = {     0,
     2,    16,    13,    14,    15,   228,   125,   235,   233,   234,
   232,   229,   129,   237,   237,   237,   237,   237,   237,    72,
    50,   128,   237,   237,   237,   237,   237,   237,   237,   237,
   237,    51,    71,   237,    49,   127,   237,    48,   126,   237,
   237,   281,   237,   237,   237,   237,   237,     0,    47,    52,
   237,   237,   237,     0,     5,   236,    11,    20,    30,    56,
   237,   237,    62,    45,   230,    46,    53,    54,    55,     0,
    78,    79,    11,   269,     0,     0,    12,    16,    65,     0,
     0,   236,    74,     0,     0,     0,    83,   236,    64,     0,
     0,     0,     0,     0,    83,     0,     0,     0,   115,     0,
   273,     0,     0,   301,     0,   280,     0,   115,     0,     0,
     0,   114,     0,     0,     0,     0,     1,     7,     3,   237,
   237,   237,   237,   107,   108,   109,   237,    21,   237,   237,
   237,    40,   237,   110,   111,   112,   237,   237,   237,   237,
   237,   237,     0,     0,   237,   237,   237,   237,   237,    11,
     4,   270,    18,    19,    17,     0,   226,     0,   168,   170,
   169,     0,   215,   216,   236,   236,     0,     0,     0,    83,
    85,     0,     0,    20,     0,     0,     0,     0,     0,     0,
   119,     0,    20,     0,   170,     0,   282,   271,   299,   300,
   278,   302,   283,   286,   289,   279,   297,   170,   115,     0,
    38,    42,    43,   237,    44,   236,   115,   231,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   130,   130,     0,   104,    94,     0,    94,
     6,     0,   237,     0,   237,   237,   237,   237,   237,   186,
   170,     0,   237,     0,     0,     0,   236,    11,     0,   222,
   237,     0,     0,    84,     0,    86,     0,     0,     0,   237,
   237,   237,     0,   237,   237,   237,   121,   237,   116,     0,
   186,   274,   281,     0,   236,     0,   304,     0,     0,     0,
   277,   298,   186,   237,     0,     0,   117,     0,    22,    23,
    24,    25,    29,    26,    27,    28,    31,    32,    37,    39,
    41,    34,    35,    36,    50,    49,    48,   130,   133,     0,
    45,   230,    54,   133,    85,    85,     0,   141,   104,   142,
   237,    85,     0,   102,    85,    85,   165,     0,   154,     0,
   160,     0,   227,    77,     0,     0,     0,     0,   237,     0,
   186,   171,     0,     0,   214,   237,   217,    11,    11,    10,
   236,     0,    76,   221,   220,   115,   237,     0,     0,     0,
    66,    73,    75,   115,    70,    68,     0,     0,     0,    81,
     0,   261,   281,     0,   301,   351,   352,   236,   236,     0,
   237,   319,     0,   314,   315,   303,   305,   295,   294,     0,
   292,     0,     0,     0,    82,   113,   237,   236,   237,     0,
   131,   134,     0,     0,     0,     0,    93,    91,     0,   105,
    97,    90,     0,     0,    94,     0,    89,    87,   237,     0,
   237,   237,   153,   237,     0,     0,     0,   237,   236,   237,
     0,   185,   184,   183,   176,   179,   180,   181,   182,   176,
   115,   115,     0,   237,   187,     0,   218,   213,     9,     8,
     0,   223,   224,     0,    61,     0,   237,   237,   147,   237,
    63,   237,     0,   120,   157,   122,   237,   237,     0,     0,
   301,   302,     0,     0,   309,   319,   319,     0,   237,   349,
   327,   237,   237,   326,   317,   319,   319,   336,   337,   237,
   276,   236,   284,   296,     0,   287,   290,     0,     0,     0,
   239,   240,   241,    57,   118,    58,    33,   237,   130,   237,
    85,   104,     0,    98,   103,    95,    85,     0,     0,   151,
   150,   149,   155,   156,   159,     0,     0,     0,   237,   173,
   176,     0,   174,   172,   175,   198,   199,   195,   203,   194,
   202,   193,   201,   237,     0,   189,     0,   191,   192,   167,
   219,   225,     0,     0,   145,   144,   143,    67,     0,     0,
    69,    80,   265,   262,   263,     0,   258,   257,   302,   236,
   306,   307,   313,   312,   311,     0,   309,   318,   332,     0,
   328,   324,   319,   331,   334,   320,     0,     0,     0,   281,
   237,   321,   325,   237,     0,   316,   297,   293,   297,   297,
   249,   248,     0,     0,     0,   251,   237,   242,   243,   244,
   123,   132,   124,    92,   106,   102,     0,   100,    88,   166,
   152,   163,   161,   164,   148,   177,   178,     0,     0,   237,
   205,     0,   146,     0,   158,     0,   264,   237,   260,   259,
   236,     0,   236,   310,   237,   281,   329,     0,   340,   343,
   346,   349,   297,   332,   319,   319,   301,   115,   285,   288,
   291,     0,     0,     0,     0,     0,   254,   255,     0,   251,
   238,    94,    99,     0,   200,   188,     0,   140,   139,   138,
   210,   209,     0,     0,   205,   212,   135,   208,   137,   237,
   237,   266,   256,     0,   272,     0,   301,   330,   281,   236,
   236,   236,   301,   323,   301,   339,     0,     0,     0,   247,
   267,   253,   250,   252,    96,   101,   162,   190,     0,     0,
   196,   204,   237,   206,    60,    59,   275,   308,   333,   335,
   353,     0,     0,     0,   338,   322,   350,   245,   246,   268,
   212,   208,   197,     0,   236,   341,   344,   347,   207,   211,
   354,   297,   297,   297,   301,   301,   301,   342,   345,   348,
     0,     0,     0
};

static const short yydefgoto[] = {   761,
    54,    55,    56,    77,    78,   112,    58,   297,    59,   169,
   255,   256,   322,   323,   513,   415,   316,   129,   130,   131,
   140,   141,   142,   157,   269,   286,   267,   369,    60,    61,
    62,   309,   403,   310,   317,    89,   460,    79,   424,   328,
   464,   329,   330,   331,    63,   160,   240,   241,   530,   531,
   436,   340,   341,   545,   546,   547,   684,   685,   686,   724,
    83,   245,   163,   164,   167,   168,   158,    64,    65,    66,
    67,    68,    69,   382,    80,    71,   500,   501,   502,   605,
   503,   669,   670,    72,   566,   470,   564,   565,   712,    73,
    74,   273,   186,   373,   104,   482,   483,   196,   278,   597,
   279,   599,   280,   600,   390,   391,   495,   281,   282,   192,
   275,   276,   277,   576,   577,   731,   384,   385,   581,   485,
   486,   582,   583,   584,   487,   585,   700,   752,   701,   753,
   702,   754,   489,   490,   732
};

static const short yypact[] = {  1039,
-32768,   116,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,    63,-32768,-32768,-32768,-32768,-32768,  1660,-32768,-32768,
-32768,-32768,-32768,    11,-32768,  1145,   435,  1454,   636,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   295,
-32768,-32768,   435,     9,    86,   134,-32768,   116,-32768,  1660,
  1660,  1660,-32768,  1660,  1660,  1660,    96,  1660,-32768,  1660,
  1660,  1660,  1660,  1660,    96,  1660,  1660,  1660,  1660,   105,
-32768,    79,   246,-32768,   258,   105,   105,  1660,  1660,  1660,
  1660,   153,   104,  1660,  1660,   105,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   118,   125,-32768,-32768,-32768,-32768,-32768,   435,
-32768,-32768,-32768,-32768,-32768,   124,   212,   210,   816,   250,
-32768,    38,   228,   255,   230,   251,   273,   285,   215,    96,
   295,   272,   283,  1248,   317,   324,   338,   276,   346,   352,
   371,   378,  1351,   368,   250,   340,-32768,-32768,-32768,-32768,
-32768,   350,-32768,-32768,-32768,-32768,   150,   250,  1454,   335,
    84,-32768,-32768,-32768,-32768,   816,  1454,-32768,  1660,  1660,
  1660,  1660,  1660,  1660,  1660,  1660,  1660,  1660,  1660,  1660,
  1660,  1660,  1660,  1763,  1763,  1660,   494,   277,  1660,   277,
-32768,  1660,-32768,  1660,-32768,-32768,-32768,-32768,-32768,   381,
   250,  1660,-32768,  1660,   412,  1660,  1660,   435,  1660,   388,
-32768,  1660,  1660,-32768,   417,-32768,  1660,  1660,  1660,-32768,
-32768,-32768,  1660,-32768,-32768,-32768,   418,-32768,-32768,  1660,
   381,-32768,   102,   311,   404,   426,   350,    55,    55,    55,
-32768,-32768,   381,-32768,  1660,   384,  1660,   366,   816,   798,
   373,   444,   409,   432,   224,   224,-32768,   772,   100,-32768,
-32768,   188,   100,   100,   392,   402,   408,  1557,   499,   416,
   434,   437,   446,   499,   300,   295,   449,-32768,   494,-32768,
-32768,   295,   440,   492,   443,   295,   781,   505,    68,   504,
   155,  1660,-32768,-32768,   605,   605,  1660,  1660,-32768,   531,
   381,-32768,   509,  1660,-32768,-32768,-32768,   435,   435,-32768,
   510,  1660,-32768,-32768,   285,  1454,-32768,   529,   538,   539,
-32768,-32768,-32768,  1454,-32768,-32768,  1660,  1660,   541,-32768,
   543,   506,   102,   150,-32768,-32768,-32768,   404,   404,   479,
-32768,   747,   547,-32768,   565,-32768,-32768,-32768,-32768,   482,
   507,   480,   485,   320,-32768,-32768,-32768,   816,-32768,  1660,
-32768,-32768,   486,  1660,   495,  1660,-32768,-32768,   105,-32768,
-32768,-32768,   537,    96,   277,  1660,-32768,-32768,-32768,  1660,
-32768,-32768,-32768,-32768,  1660,  1660,  1660,-32768,-32768,-32768,
   505,-32768,-32768,-32768,   605,   489,-32768,-32768,-32768,   605,
  1454,  1454,   623,-32768,-32768,  1660,-32768,-32768,-32768,-32768,
  1660,-32768,-32768,   493,-32768,  1660,-32768,-32768,-32768,-32768,
-32768,-32768,   497,-32768,   582,-32768,-32768,-32768,    96,   160,
-32768,   350,   572,   573,    65,   532,    80,  1660,-32768,    79,
-32768,-32768,   269,-32768,-32768,   461,    80,-32768,   150,-32768,
-32768,   404,-32768,-32768,    55,-32768,-32768,   217,   105,   574,
   320,   320,   320,-32768,-32768,-32768,-32768,-32768,  1557,-32768,
   295,   494,   508,    51,-32768,-32768,   295,  1660,   577,-32768,
-32768,-32768,-32768,-32768,-32768,  1660,  1660,  1660,-32768,-32768,
   605,  1660,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,  1660,   535,   520,-32768,-32768,-32768,
-32768,-32768,  1660,   590,-32768,-32768,-32768,-32768,  1660,  1660,
-32768,-32768,   517,-32768,    96,   595,   160,   160,   350,   404,
-32768,-32768,-32768,-32768,-32768,   533,    65,-32768,   270,   555,
-32768,-32768,    80,-32768,-32768,-32768,   105,   380,   359,   311,
-32768,-32768,-32768,-32768,   540,-32768,   150,-32768,   150,   150,
-32768,-32768,   557,   561,  1660,   201,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   492,   105,-32768,-32768,-32768,
-32768,    23,   598,   558,-32768,-32768,   816,   301,   611,-32768,
   192,   616,-32768,   617,-32768,  1660,-32768,-32768,-32768,-32768,
   404,   619,   404,-32768,-32768,   311,-32768,   568,-32768,-32768,
-32768,-32768,   150,   150,    80,    80,-32768,  1660,-32768,-32768,
-32768,    60,    60,   620,   105,    60,-32768,-32768,   621,   201,
-32768,   277,   537,  1660,-32768,-32768,   105,-32768,-32768,-32768,
-32768,-32768,   548,    41,   192,   639,-32768,   437,-32768,-32768,
-32768,   816,-32768,   624,-32768,   625,-32768,-32768,   311,   404,
   404,   404,-32768,-32768,-32768,-32768,   566,   635,   637,-32768,
   105,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    70,   571,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   507,   575,   578,   570,-32768,-32768,-32768,-32768,-32768,-32768,
   639,-32768,-32768,  1660,   404,-32768,-32768,-32768,-32768,   816,
-32768,   150,   150,   150,-32768,-32768,-32768,-32768,-32768,-32768,
   679,   680,-32768
};

static const short yypgoto[] = {-32768,
   627,   -50,-32768,   614,-32768,    92,   186,  -382,   613,    -4,
  -250,   626,  -213,-32768,  -479,    77,  -298,-32768,-32768,-32768,
-32768,-32768,-32768,   330,   -54,   299,-32768,-32768,-32768,-32768,
-32768,  -205,   387,  -468,  -386,  -364,-32768,  -338,   274,   -20,
   143,  -333,-32768,  -238,-32768,   618,     7,-32768,  -372,   130,
-32768,  -202,-32768,-32768,-32768,-32768,    19,-32768,   -10,   -31,
-32768,-32768,   467,-32768,   -55,   462,   483,   329,    35,   156,
-32768,   259,-32768,     5,   -15,-32768,   -66,-32768,  -549,  -183,
  -533,    46,-32768,-32768,   -64,-32768,   154,-32768,    10,   656,
  -406,-32768,-32768,-32768,  -224,   -18,    -9,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,  -239,-32768,     4,  -544,   -97,  -324,
  -424,   459,  -403,   161,-32768,  -223,   245,-32768,  -332,   254,
-32768,  -443,-32768,  -547,-32768,  -340,-32768,-32768,-32768,-32768,
-32768,-32768,   152,   470,  -463
};


#define YYLAST          1873


static const short yytable[] = {    81,
    82,    84,    85,    86,    70,   191,   119,    87,    88,    90,
    91,    92,    93,    94,    95,    96,   326,   507,    97,   314,
   410,    98,   151,   105,    99,   100,   514,   107,   108,   109,
   110,   111,   106,   465,   618,   114,   115,   116,   177,   392,
   393,   488,   655,   593,   184,   143,   144,   570,   375,   484,
   472,   383,   659,   200,   660,   661,   667,   521,     6,   720,
    70,     9,    10,   567,   407,   408,   568,   533,   372,    12,
     6,   412,   668,   601,   417,   418,   103,    12,   117,    12,
   394,    42,   520,   242,    12,   425,   161,     6,   376,   153,
   178,    57,   161,   556,    12,   377,   132,   243,   698,   231,
   426,   428,   401,     6,   209,   210,   211,   212,   703,     6,
   430,   213,   132,   214,   215,   216,    12,   217,   555,    12,
   667,   218,   219,   220,   221,   222,   223,   514,   602,   226,
   227,   228,   229,   230,   185,   620,   668,   154,   445,   647,
   197,   198,   721,   578,   641,   488,   569,    57,   471,   128,
   208,   730,   288,   484,   473,   474,   478,   388,   626,   617,
   567,   567,   683,   568,   568,   254,   101,   573,   574,   248,
   250,   156,   681,   682,   232,   162,   165,   166,   138,   139,
  -281,   172,   173,   233,  -281,   166,  -281,   187,   285,    53,
   182,   271,   137,   716,   138,   139,   354,   350,   204,     6,
   132,   516,     9,    10,   283,   205,    12,   755,   756,   757,
   287,   704,   705,   615,    75,    76,   683,   332,   224,   334,
   335,   336,   337,   338,     6,   225,   465,   344,   678,   234,
   601,    12,    42,   428,   274,   353,   429,   733,   734,   189,
   190,   665,   430,   235,   361,   362,   363,   342,   365,   366,
   367,   349,   370,   679,   105,   598,   680,   244,   312,   312,
   614,   319,   324,   106,   324,   128,   619,   159,   395,   236,
   666,   499,   246,   159,   128,   247,   191,   174,   135,   136,
   137,   751,   138,   139,   183,   602,   514,   622,   237,   624,
   238,    12,   321,   199,   681,   682,   249,   449,   450,   206,
   207,   454,   252,   612,   239,   411,   251,   374,   379,   463,
-32768,   127,   389,   389,   389,    12,   675,   315,     6,   376,
   325,   253,   145,   443,   188,    12,   377,   145,   258,   146,
   448,   257,   706,   343,   146,   189,   190,   162,   348,   147,
   351,   455,   312,   166,   147,   406,   642,   148,  -230,   359,
   260,   149,   148,   319,   105,   452,   149,   261,   193,   189,
   190,   371,   194,   106,   195,   476,     6,   376,   183,  -280,
   102,   262,   729,  -280,   377,  -280,   396,   113,   735,   264,
   736,   504,   263,   506,   199,   265,   534,   535,   591,   498,
   499,   594,   199,   266,   289,   290,   291,   292,   293,   294,
   295,   296,   287,   518,   523,   524,    90,   374,   522,   308,
   308,   268,   526,   270,   528,   170,   481,   694,   272,   696,
   284,   175,   176,   170,   274,   179,   180,   181,   550,   339,
   758,   759,   760,   527,   608,   609,   610,     2,   356,     3,
     4,     5,    90,   512,   557,   346,   558,   352,   364,   324,
   357,   561,   562,   122,   123,   124,   125,   368,   715,   126,
   127,    18,   381,   587,   435,   440,   588,   589,     6,   376,
   145,   386,   398,   399,   595,    12,   377,   146,   708,   709,
   649,   645,   313,   313,   650,   320,   651,   147,   416,   397,
   438,   438,   611,   308,   613,   148,   127,   511,   170,   149,
  -140,     6,   639,   640,     9,    10,   477,   517,    12,   575,
  -139,   579,   414,   625,-32768,-32768,  -138,   402,   126,   127,
   481,   579,   441,   442,   404,   123,   124,   125,   628,   389,
   126,   127,   604,   606,   420,   421,   422,   478,   423,   590,
   413,   199,  -135,   312,     6,  -136,   319,     9,    10,   199,
   189,   190,   311,   311,  -137,   318,   645,   409,   456,   457,
   458,  -281,   459,   427,   444,  -281,   313,  -281,   446,   451,
   479,   461,   462,   345,   467,   656,   468,   320,   657,   475,
   491,   469,   492,   493,   494,   496,   358,   508,   360,   509,
   438,   671,   497,   439,   439,   438,   510,   532,   549,   560,
   553,   102,   378,   707,   559,   571,   572,   607,   478,   616,
   621,   575,     6,   630,   677,     9,    10,   579,   623,    12,
   631,   648,   693,   633,   654,   636,   199,   199,   638,   697,
     6,   536,   580,   646,   643,   662,   311,    12,   537,   663,
   658,   432,   580,   674,   676,   430,   699,   318,   132,   690,
   691,   673,   695,   710,   713,   723,   719,   727,   728,   538,
   539,   431,   208,   437,   437,   688,   433,   737,   738,   434,
   739,   320,   743,   447,   725,   726,   746,   748,   762,   763,
   654,   453,   118,   747,   540,   541,   438,   542,   543,   579,
   579,   155,   672,   439,   308,   150,   505,   466,   439,   711,
   405,   102,   635,   722,   529,   171,   324,   744,   741,   749,
   480,   718,   347,   355,    53,   714,   333,   627,   637,   688,
   740,   201,   202,   203,   133,   134,   135,   136,   137,   152,
   138,   139,   544,   654,   745,   387,   596,   644,   580,   592,
   653,   318,   515,   380,     0,   711,     0,     0,     0,   519,
     0,     0,     0,   742,     6,   376,   525,     0,     0,     0,
     0,    12,   377,   437,     0,   717,     0,   313,   437,     0,
   320,   548,   320,     0,     0,   551,     0,     0,     0,     0,
   552,     0,     0,     0,   132,   554,     0,     0,     0,   439,
     0,     0,   477,   132,     0,     0,     0,   563,     0,     0,
     0,     0,     0,     0,     0,   480,     0,   586,     0,     0,
   580,   580,     0,     0,   480,   480,     0,     0,     0,     0,
     0,   692,     0,   478,     0,     0,   603,     0,     0,   298,
   299,   300,   301,   302,   303,   304,     0,   311,     0,     0,
   318,     0,   318,   199,   327,     0,     0,  -281,     0,     0,
     0,  -281,     0,  -281,     0,     0,   479,     0,     0,   437,
   400,   134,   135,   136,   137,     0,   138,   139,     0,   419,
   134,   135,   136,   137,   629,   138,   139,   121,   122,   123,
   124,   125,   632,     0,   126,   127,     0,     0,   634,   689,
     0,     0,     0,   563,   120,   121,   122,   123,   124,   125,
     0,     0,   126,   127,     0,     0,     0,     0,     0,     0,
     0,   480,     0,     0,     0,     0,     0,   652,   480,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   750,
     0,   320,     0,     0,   664,     0,     0,     0,     0,     0,
     0,     0,     0,   689,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   687,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   480,     0,     0,     0,     0,   327,
     0,     0,     0,   480,   480,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   318,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,   298,   687,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   480,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   327,   327,     1,
     0,     2,     0,     3,     4,     5,     6,     7,     8,     9,
    10,     0,    11,    12,    13,     0,     0,     0,     0,     0,
    14,     0,    15,    16,    17,    18,  -236,    19,     0,     0,
     0,     0,     0,  -236,    20,    21,    22,     0,     0,     0,
    23,    24,    25,  -236,     0,    26,    27,     0,    28,     0,
    29,  -236,    30,     0,    31,  -236,    32,    33,     0,    34,
    35,    36,    37,    38,    39,    40,   -11,     0,     0,     0,
    41,    42,    43,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    44,     0,     0,     0,     0,     0,     0,
   298,     0,    45,     0,     0,    46,    47,     0,   327,    48,
   327,    49,    50,    51,     0,    52,     0,     2,    53,     3,
     4,     5,     6,     7,     8,     9,    10,     0,    11,    12,
    13,     0,     0,     0,     0,     0,    14,     0,    15,    16,
    17,    18,   327,    19,     0,     0,     0,     0,     0,     0,
    20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
     0,    26,    27,     0,    28,     0,    29,     0,    30,     0,
    31,     0,    32,    33,     0,    34,    35,    36,    37,    38,
    39,    40,   -11,     0,     0,     0,    41,    42,    43,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    44,
     0,     0,     0,     0,     0,     0,     0,     0,    45,     0,
     0,    46,    47,     0,     0,    48,     0,    49,    50,    51,
     0,    52,     0,     0,    53,     6,     7,     8,     9,    10,
     0,    11,    12,    13,     0,     0,     0,     0,     0,    14,
     0,    15,    16,    17,     0,     0,    19,     0,     0,     0,
     0,     0,     0,    20,    21,    22,     0,     0,     0,    23,
    24,    25,     0,     0,    26,    27,     0,    28,     0,    29,
     0,    30,     0,    31,     0,    32,    33,   259,    34,    35,
    36,    37,    38,    39,    40,     0,     0,     0,     0,    41,
     0,    43,     0,     0,     0,     0,   120,   121,   122,   123,
   124,   125,    44,     0,   126,   127,     0,     0,     0,     0,
     0,    45,     0,     0,    46,    47,     0,     0,    48,     0,
    49,    50,    51,     0,    52,     0,     0,    53,     6,     7,
     8,     9,    10,     0,    11,    12,    13,     0,     0,     0,
     0,     0,    14,     0,    15,    16,    17,     0,     0,    19,
     0,     0,     0,     0,     0,     0,    20,    21,    22,     0,
     0,     0,    23,    24,    25,     0,  -115,    26,    27,     0,
    28,     0,    29,     0,    30,     0,    31,     0,    32,    33,
     0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
     0,     0,    41,     0,    43,     0,     0,     0,     0,   120,
   121,   122,   123,   124,   125,    44,     0,   126,   127,     0,
     0,     0,     0,     0,    45,     0,     0,    46,    47,     0,
     0,    48,     0,    49,    50,    51,     0,    52,     0,     0,
    53,     6,     7,     8,     9,    10,     0,    11,    12,    13,
     0,     0,     0,     0,     0,    14,     0,    15,    16,    17,
     0,     0,    19,     0,     0,     0,     0,     0,     0,    20,
    21,    22,     0,     0,     0,    23,    24,    25,     0,     0,
    26,    27,     0,    28,     0,    29,     0,    30,     0,    31,
     0,    32,    33,     0,    34,    35,    36,    37,    38,    39,
    40,     0,     0,     0,     0,    41,     0,    43,     0,     0,
     0,     0,   120,   121,   122,   123,   124,   125,    44,     0,
   126,   127,     0,     0,     0,     0,     0,    45,     0,     0,
    46,    47,     0,     0,    48,     0,    49,    50,    51,     0,
    52,     0,     0,    53,     6,     7,     8,     9,    10,     0,
    11,    12,    13,     0,     0,     0,     0,     0,    14,     0,
    15,    16,    17,     0,     0,    19,     0,     0,     0,     0,
     0,     0,    20,   305,    22,     0,     0,     0,    23,    24,
    25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
    30,     0,    31,     0,    32,    33,     0,    34,   306,    36,
    37,   307,    39,    40,     0,     0,     0,     0,    41,     0,
    43,     0,     0,     0,     0,   120,   121,   122,   123,   124,
   125,    44,     0,   126,   127,     0,     0,     0,     0,     0,
    45,     0,     0,    46,    47,     0,     0,    48,     0,    49,
    50,    51,     0,    52,     0,     0,    53,     6,     7,     8,
     9,    10,     0,    11,    12,    13,     0,     0,     0,     0,
     0,    14,     0,    15,    16,    17,     0,     0,    19,     0,
     0,     0,     0,     0,     0,    20,    21,    22,     0,     0,
     0,    23,    24,    25,     0,     0,    26,    27,     0,    28,
     0,    29,     0,    30,     0,    31,     0,    32,    33,     0,
    34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
     0,    41,     0,    43,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    44,     0,     0,     0,     0,     0,
     0,     0,     0,    45,     0,     0,    46,    47,     0,     0,
    48,     0,    49,    50,    51,     0,    52,     0,     0,    53,
     6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
     0,     0,     0,     0,    14,     0,    15,    16,    17,     0,
     0,    19,     0,     0,     0,     0,     0,     0,    20,   305,
    22,     0,     0,     0,    23,    24,    25,     0,     0,    26,
    27,     0,    28,     0,    29,     0,    30,     0,    31,     0,
    32,    33,     0,    34,   306,    36,    37,   307,    39,    40,
     0,     0,     0,     0,    41,     0,    43,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    44,     0,     0,
     0,     0,     0,     0,     0,     0,    45,     0,     0,    46,
    47,     0,     0,    48,     0,    49,    50,    51,     0,    52,
     0,     0,    53
};

static const short yycheck[] = {    15,
    16,    17,    18,    19,     0,   103,    57,    23,    24,    25,
    26,    27,    28,    29,    30,    31,   230,   400,    34,   225,
   319,    37,    73,    42,    40,    41,   413,    43,    44,    45,
    46,    47,    42,   367,   514,    51,    52,    53,    94,   279,
   280,   382,   590,   487,    99,    61,    62,   472,   273,   382,
   375,   275,   597,   108,   599,   600,   606,   422,     8,    19,
    56,    11,    12,   470,   315,   316,   470,   440,   271,    15,
     8,   322,   606,    14,   325,   326,    42,    15,    68,    15,
   283,    73,   421,    46,    15,    18,    82,     8,     9,     4,
    95,     0,    88,   458,    15,    16,    13,    60,   646,   150,
    33,    79,   308,     8,   120,   121,   122,   123,   653,     8,
    88,   127,    13,   129,   130,   131,    15,   133,   457,    15,
   670,   137,   138,   139,   140,   141,   142,   514,    69,   145,
   146,   147,   148,   149,   100,   518,   670,     4,   341,   583,
   106,   107,   102,   476,   569,   486,   471,    56,   373,    58,
   116,   699,   207,   486,   378,   379,    77,   103,   531,   109,
   567,   568,   631,   567,   568,   170,   104,   103,   104,   165,
   166,    80,   103,   104,    51,    84,    85,    86,    95,    96,
   101,    90,    91,    60,   105,    94,   107,   109,   204,   110,
    99,   185,    93,   673,    95,    96,   252,   248,    46,     8,
    13,   415,    11,    12,   198,   102,    15,   752,   753,   754,
   206,   655,   656,   512,    99,   100,   685,   233,   101,   235,
   236,   237,   238,   239,     8,   101,   560,   243,    37,    18,
    14,    15,    73,    79,    75,   251,    82,   701,   702,    90,
    91,    41,    88,    34,   260,   261,   262,   241,   264,   265,
   266,   247,   268,    62,   273,   495,    65,    30,   224,   225,
   511,   227,   228,   273,   230,   174,   517,    82,   284,    20,
    70,    71,    18,    88,   183,    46,   374,    92,    91,    92,
    93,   745,    95,    96,    99,    69,   673,   526,    39,   528,
    41,    15,    16,   108,   103,   104,    46,   348,   349,   114,
   115,   356,    18,   509,    55,   321,    34,   273,   274,   364,
    87,    88,   278,   279,   280,    15,    16,   226,     8,     9,
   229,   107,    28,   339,    79,    15,    16,    28,    46,    35,
   346,    60,   657,   242,    35,    90,    91,   246,   247,    45,
   249,   357,   308,   252,    45,    46,   570,    53,    79,   258,
    34,    57,    53,   319,   373,   351,    57,    34,   101,    90,
    91,   270,   105,   373,   107,   381,     8,     9,   183,   101,
    42,    34,   697,   105,    16,   107,   285,    48,   703,    34,
   705,   397,   107,   399,   199,    34,   441,   442,   486,    70,
    71,   489,   207,    23,   209,   210,   211,   212,   213,   214,
   215,   216,   398,   419,   425,   426,   422,   373,   424,   224,
   225,    34,   428,    46,   430,    87,   382,   641,    79,   643,
    86,    92,    93,    95,    75,    96,    97,    98,   444,    49,
   755,   756,   757,   429,   501,   502,   503,     3,   253,     5,
     6,     7,   458,   409,   460,    34,   462,    60,   263,   415,
    34,   467,   468,    81,    82,    83,    84,    40,   672,    87,
    88,    27,    59,   479,   335,   336,   482,   483,     8,     9,
    28,    46,   287,   108,   490,    15,    16,    35,   662,   663,
   101,   579,   224,   225,   105,   227,   107,    45,    46,   106,
   335,   336,   508,   308,   510,    53,    88,   406,   170,    57,
   109,     8,   567,   568,    11,    12,    46,   416,    15,   475,
   109,   477,    21,   529,    83,    84,   109,    19,    87,    88,
   486,   487,   337,   338,   109,    82,    83,    84,   544,   495,
    87,    88,   498,   499,    30,    31,    32,    77,    34,    79,
   101,   356,   109,   509,     8,   109,   512,    11,    12,   364,
    90,    91,   224,   225,   109,   227,   654,   109,    30,    31,
    32,   101,    34,    60,    34,   105,   308,   107,    60,    60,
   110,    34,    34,   244,    34,   591,    34,   319,   594,   101,
    34,    76,    18,   102,    78,   106,   257,   102,   259,   404,
   435,   607,   108,   335,   336,   440,   102,   109,   443,    18,
   108,   273,   274,   658,   108,    34,    34,    34,    77,   102,
    34,   577,     8,    79,   630,    11,    12,   583,   527,    15,
   101,   587,   638,    34,   590,   109,   441,   442,    34,   645,
     8,     9,   477,    79,   102,    79,   308,    15,    16,    79,
   101,    37,   487,    46,    34,    88,    79,   319,    13,    34,
    34,   617,    34,    34,    34,    17,   109,    34,    34,    37,
    38,   332,   628,   335,   336,   631,    62,   102,    34,    65,
    34,   413,   102,   344,   690,   691,   102,   108,     0,     0,
   646,   352,    56,   106,    62,    63,   531,    65,    66,   655,
   656,    78,   616,   435,   509,    70,   398,   368,   440,   665,
   314,   373,   560,   685,   431,    88,   672,   723,   719,   741,
   382,   677,   246,   252,   110,   670,   234,   532,   565,   685,
   711,   109,   110,   111,    89,    90,    91,    92,    93,    74,
    95,    96,   110,   699,   731,   277,   492,   577,   583,   486,
   589,   413,   414,   274,    -1,   711,    -1,    -1,    -1,   420,
    -1,    -1,    -1,   719,     8,     9,   427,    -1,    -1,    -1,
    -1,    15,    16,   435,    -1,   674,    -1,   509,   440,    -1,
   512,   443,   514,    -1,    -1,   446,    -1,    -1,    -1,    -1,
   451,    -1,    -1,    -1,    13,   456,    -1,    -1,    -1,   531,
    -1,    -1,    46,    13,    -1,    -1,    -1,   469,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,   477,    -1,   478,    -1,    -1,
   655,   656,    -1,    -1,   486,   487,    -1,    -1,    -1,    -1,
    -1,   636,    -1,    77,    -1,    -1,   498,    -1,    -1,   217,
   218,   219,   220,   221,   222,   223,    -1,   509,    -1,    -1,
   512,    -1,   514,   658,   232,    -1,    -1,   101,    -1,    -1,
    -1,   105,    -1,   107,    -1,    -1,   110,    -1,    -1,   531,
    89,    90,    91,    92,    93,    -1,    95,    96,    -1,    89,
    90,    91,    92,    93,   545,    95,    96,    80,    81,    82,
    83,    84,   553,    -1,    87,    88,    -1,    -1,   559,   631,
    -1,    -1,    -1,   565,    79,    80,    81,    82,    83,    84,
    -1,    -1,    87,    88,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,   583,    -1,    -1,    -1,    -1,    -1,   589,   590,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   744,
    -1,   673,    -1,    -1,   605,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,   685,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   631,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,   646,    -1,    -1,    -1,    -1,   367,
    -1,    -1,    -1,   655,   656,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,   673,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,   400,   685,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,   699,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,   425,   426,     1,
    -1,     3,    -1,     5,     6,     7,     8,     9,    10,    11,
    12,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,
    22,    -1,    24,    25,    26,    27,    28,    29,    -1,    -1,
    -1,    -1,    -1,    35,    36,    37,    38,    -1,    -1,    -1,
    42,    43,    44,    45,    -1,    47,    48,    -1,    50,    -1,
    52,    53,    54,    -1,    56,    57,    58,    59,    -1,    61,
    62,    63,    64,    65,    66,    67,    68,    -1,    -1,    -1,
    72,    73,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,
   518,    -1,    94,    -1,    -1,    97,    98,    -1,   526,   101,
   528,   103,   104,   105,    -1,   107,    -1,     3,   110,     5,
     6,     7,     8,     9,    10,    11,    12,    -1,    14,    15,
    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,    25,
    26,    27,   560,    29,    -1,    -1,    -1,    -1,    -1,    -1,
    36,    37,    38,    -1,    -1,    -1,    42,    43,    44,    -1,
    -1,    47,    48,    -1,    50,    -1,    52,    -1,    54,    -1,
    56,    -1,    58,    59,    -1,    61,    62,    63,    64,    65,
    66,    67,    68,    -1,    -1,    -1,    72,    73,    74,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    -1,
    -1,    97,    98,    -1,    -1,   101,    -1,   103,   104,   105,
    -1,   107,    -1,    -1,   110,     8,     9,    10,    11,    12,
    -1,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    22,
    -1,    24,    25,    26,    -1,    -1,    29,    -1,    -1,    -1,
    -1,    -1,    -1,    36,    37,    38,    -1,    -1,    -1,    42,
    43,    44,    -1,    -1,    47,    48,    -1,    50,    -1,    52,
    -1,    54,    -1,    56,    -1,    58,    59,    60,    61,    62,
    63,    64,    65,    66,    67,    -1,    -1,    -1,    -1,    72,
    -1,    74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
    83,    84,    85,    -1,    87,    88,    -1,    -1,    -1,    -1,
    -1,    94,    -1,    -1,    97,    98,    -1,    -1,   101,    -1,
   103,   104,   105,    -1,   107,    -1,    -1,   110,     8,     9,
    10,    11,    12,    -1,    14,    15,    16,    -1,    -1,    -1,
    -1,    -1,    22,    -1,    24,    25,    26,    -1,    -1,    29,
    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    38,    -1,
    -1,    -1,    42,    43,    44,    -1,    46,    47,    48,    -1,
    50,    -1,    52,    -1,    54,    -1,    56,    -1,    58,    59,
    -1,    61,    62,    63,    64,    65,    66,    67,    -1,    -1,
    -1,    -1,    72,    -1,    74,    -1,    -1,    -1,    -1,    79,
    80,    81,    82,    83,    84,    85,    -1,    87,    88,    -1,
    -1,    -1,    -1,    -1,    94,    -1,    -1,    97,    98,    -1,
    -1,   101,    -1,   103,   104,   105,    -1,   107,    -1,    -1,
   110,     8,     9,    10,    11,    12,    -1,    14,    15,    16,
    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,    25,    26,
    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    36,
    37,    38,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,
    47,    48,    -1,    50,    -1,    52,    -1,    54,    -1,    56,
    -1,    58,    59,    -1,    61,    62,    63,    64,    65,    66,
    67,    -1,    -1,    -1,    -1,    72,    -1,    74,    -1,    -1,
    -1,    -1,    79,    80,    81,    82,    83,    84,    85,    -1,
    87,    88,    -1,    -1,    -1,    -1,    -1,    94,    -1,    -1,
    97,    98,    -1,    -1,   101,    -1,   103,   104,   105,    -1,
   107,    -1,    -1,   110,     8,     9,    10,    11,    12,    -1,
    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,
    24,    25,    26,    -1,    -1,    29,    -1,    -1,    -1,    -1,
    -1,    -1,    36,    37,    38,    -1,    -1,    -1,    42,    43,
    44,    -1,    -1,    47,    48,    -1,    50,    -1,    52,    -1,
    54,    -1,    56,    -1,    58,    59,    -1,    61,    62,    63,
    64,    65,    66,    67,    -1,    -1,    -1,    -1,    72,    -1,
    74,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,
    84,    85,    -1,    87,    88,    -1,    -1,    -1,    -1,    -1,
    94,    -1,    -1,    97,    98,    -1,    -1,   101,    -1,   103,
   104,   105,    -1,   107,    -1,    -1,   110,     8,     9,    10,
    11,    12,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,
    -1,    22,    -1,    24,    25,    26,    -1,    -1,    29,    -1,
    -1,    -1,    -1,    -1,    -1,    36,    37,    38,    -1,    -1,
    -1,    42,    43,    44,    -1,    -1,    47,    48,    -1,    50,
    -1,    52,    -1,    54,    -1,    56,    -1,    58,    59,    -1,
    61,    62,    63,    64,    65,    66,    67,    -1,    -1,    -1,
    -1,    72,    -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    94,    -1,    -1,    97,    98,    -1,    -1,
   101,    -1,   103,   104,   105,    -1,   107,    -1,    -1,   110,
     8,     9,    10,    11,    12,    -1,    14,    15,    16,    -1,
    -1,    -1,    -1,    -1,    22,    -1,    24,    25,    26,    -1,
    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,
    38,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,    47,
    48,    -1,    50,    -1,    52,    -1,    54,    -1,    56,    -1,
    58,    59,    -1,    61,    62,    63,    64,    65,    66,    67,
    -1,    -1,    -1,    -1,    72,    -1,    74,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    94,    -1,    -1,    97,
    98,    -1,    -1,   101,    -1,   103,   104,   105,    -1,   107,
    -1,    -1,   110
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
{ yyval.t = newCTerm(PA_fLoop,yyvsp[-4].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 81:
{ yyval.t = newCTerm(PA_fLoop,AtomNil,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 82:
{ yyval.t = newCTerm(PA_fMacro,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 83:
{ yyval.t = AtomNil; ;
    break;}
case 84:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 85:
{ yyval.t = AtomNil; ;
    break;}
case 86:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 87:
{ yyval.t = oz_cons(newCTerm(PA_fRequire,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 88:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 89:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-1].t,
                                           newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 90:
{ yyval.t = oz_cons(newCTerm(PA_fImport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 91:
{ yyval.t = oz_cons(newCTerm(PA_fExport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 92:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 93:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-1].t,
                                           newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 94:
{ yyval.t = AtomNil; ;
    break;}
case 95:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-2].t,AtomNil,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 96:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 97:
{ yyval.t = newCTerm(PA_fVar,OZ_atom(xytext),yyvsp[0].t); ;
    break;}
case 98:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 99:
{ yyval.t = oz_mklist(oz_pair2(yyvsp[0].t,yyvsp[-2].t)); ;
    break;}
case 100:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 101:
{ yyval.t = oz_cons(oz_pair2(yyvsp[-1].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 102:
{ yyval.t = PA_fNoImportAt; ;
    break;}
case 103:
{ yyval.t = newCTerm(PA_fImportAt,yyvsp[0].t); ;
    break;}
case 104:
{ yyval.t = AtomNil; ;
    break;}
case 105:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 106:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,
                                           newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t)),yyvsp[0].t); ;
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
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 113:
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 114:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 115:
{ yyval.t = AtomNil; ;
    break;}
case 116:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 117:
{ yyval.t = newCTerm(PA_fAtom,AtomNil,yyvsp[0].t); ;
    break;}
case 118:
{ yyval.t = newCTerm(PA_fRecord,
                                  newCTerm(PA_fAtom,AtomCons,yyvsp[-2].t),
                                  oz_mklist(yyvsp[-1].t,yyvsp[0].t)); ;
    break;}
case 119:
{ yyval.t = PA_fNoCatch; ;
    break;}
case 120:
{ yyval.t = newCTerm(PA_fCatch,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 121:
{ yyval.t = PA_fNoFinally; ;
    break;}
case 122:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 123:
{
                    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
                                  newCTerm(PA_fAtom,yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
                  ;
    break;}
case 124:
{
                    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
                                  makeVar(yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
                  ;
    break;}
case 125:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 126:
{ yyval.t = NameUnit; ;
    break;}
case 127:
{ yyval.t = NameTrue; ;
    break;}
case 128:
{ yyval.t = NameFalse; ;
    break;}
case 129:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 130:
{ yyval.t = AtomNil; ;
    break;}
case 131:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 132:
{ yyval.t = oz_cons(newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 133:
{ yyval.t = NameFalse; ;
    break;}
case 134:
{ yyval.t = NameTrue; ;
    break;}
case 135:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 136:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 137:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 138:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 139:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 140:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 141:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 142:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 143:
{ yyval.t = newCTerm(PA_fBoolCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 144:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 145:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 146:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 147:
{ yyval.t = newCTerm(PA_fSkip,pos()); ;
    break;}
case 148:
{ checkDeprecation(yyvsp[-3].t);
                    yyval.t = newCTerm(PA_fBoolCase,yyvsp[-5].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-6].t,yyvsp[0].t));
                  ;
    break;}
case 149:
{ yyval.t = newCTerm(PA_fCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
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
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 155:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 156:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 157:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 158:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 159:
{ yyval.t = newCTerm(PA_fCaseClause,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 160:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 161:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-3].t,
                                  newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 162:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-5].t,yyvsp[-2].t,yyvsp[0].t,yyvsp[-3].t); ;
    break;}
case 163:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 164:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 165:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 166:
{ yyval.t = newCTerm(PA_fRecord,
                                  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
                                  oz_cons(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 167:
{ yyval.t = newCTerm(PA_fClass,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 168:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 169:
{ yyval.t = newCTerm(PA_fDollar,yyvsp[0].t); ;
    break;}
case 170:
{ yyval.t = AtomNil; ;
    break;}
case 171:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 172:
{ yyval.t = newCTerm(PA_fFrom,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 173:
{ yyval.t = newCTerm(PA_fAttr,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 174:
{ yyval.t = newCTerm(PA_fFeat,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 175:
{ yyval.t = newCTerm(PA_fProp,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 176:
{ yyval.t = AtomNil; ;
    break;}
case 177:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 178:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
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
{ yyval.t = yyvsp[0].t; ;
    break;}
case 183:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 184:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 185:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 186:
{ yyval.t = AtomNil; ;
    break;}
case 187:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 188:
{ yyval.t = newCTerm(PA_fMeth,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-3].t); ;
    break;}
case 189:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 190:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 191:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 192:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 193:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 194:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 195:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 196:
{ yyval.t = newCTerm(PA_fRecord,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 197:
{ yyval.t = newCTerm(PA_fOpenRecord,yyvsp[-4].t,yyvsp[-2].t); ;
    break;}
case 198:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 199:
{ yyval.t = makeVar(xytext); ;
    break;}
case 200:
{ yyval.t = newCTerm(PA_fEscape,makeVar(xytext),yyvsp[-1].t); ;
    break;}
case 201:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 202:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 203:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 204:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 205:
{ yyval.t = AtomNil; ;
    break;}
case 206:
{ yyval.t = newCTerm(PA_fMethArg,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 207:
{ yyval.t = newCTerm(PA_fMethColonArg,yyvsp[-3].t,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 208:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 209:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 210:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 211:
{ yyval.t = newCTerm(PA_fDefault,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 212:
{ yyval.t = PA_fNoDefault; ;
    break;}
case 213:
{ yyval.t = newCTerm(PA_fCond,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 214:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 215:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 216:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 217:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 218:
{ yyval.t = newCTerm(PA_fClause,newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 219:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 220:
{ yyval.t = oz_mklist(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 221:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 222:
{ yyval.t = newCTerm(PA_fClause,
                                  newCTerm(PA_fSkip,yyvsp[0].t),
                                  yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 223:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-3].t,yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 224:
{ yyval.t = newCTerm(PA_fClause,
                                  newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 225:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 226:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 227:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 228:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 229:
{ yyval.t = makeVar(xytext); ;
    break;}
case 230:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 231:
{ yyval.t = newCTerm(PA_fEscape,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 232:
{ yyval.t = makeString(xytext,pos()); ;
    break;}
case 233:
{ yyval.t = makeInt(xytext,pos()); ;
    break;}
case 234:
{ yyval.t = makeInt(xytext[0],pos()); ;
    break;}
case 235:
{ yyval.t = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); ;
    break;}
case 236:
{ yyval.t = pos(); ;
    break;}
case 237:
{ yyval.t = pos(); ;
    break;}
case 238:
{ OZ_Term prefix =
                      scannerPrefix? scannerPrefix: PA_zy;
                    yyval.t = newCTerm(PA_fScanner,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,prefix,
                                  makeLongPos(yyvsp[-6].t,yyvsp[0].t)); ;
    break;}
case 239:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 240:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 241:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 242:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 243:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 244:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 245:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 246:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 247:
{ yyval.t = newCTerm(PA_fLexicalRule,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 248:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 249:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 250:
{ yyval.t = newCTerm(PA_fMode,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 251:
{ yyval.t = AtomNil; ;
    break;}
case 252:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 253:
{ yyval.t = newCTerm(PA_fInheritedModes,yyvsp[0].t); ;
    break;}
case 254:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 255:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 256:
{ OZ_Term expect = parserExpect? parserExpect: newSmallInt(0);
                    yyval.t = newCTerm(PA_fParser,yyvsp[-6].t,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,expect,
                                  makeLongPos(yyvsp[-7].t,yyvsp[0].t)); ;
    break;}
case 257:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 258:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 259:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 260:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 261:
{ yyval.t = newCTerm(PA_fToken,AtomNil); ;
    break;}
case 262:
{ yyval.t = newCTerm(PA_fToken,yyvsp[0].t); ;
    break;}
case 263:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 264:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 265:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 266:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 267:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 268:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 269:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 270:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 271:
{ *prodKey[depth]++ = '='; ;
    break;}
case 272:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 273:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 274:
{ *prodKey[depth]++ = '='; ;
    break;}
case 275:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 276:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,PA_none); ;
    break;}
case 277:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 278:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 279:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 282:
{ prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg(yyvsp[-1].t,0))); ;
    break;}
case 283:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 284:
{ depth--; ;
    break;}
case 285:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 286:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 287:
{ depth--; ;
    break;}
case 288:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 289:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 290:
{ depth--; ;
    break;}
case 291:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 292:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 293:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 294:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 295:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 296:
{ *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; ;
    break;}
case 299:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 300:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 301:
{ *prodKey[depth] = '\0';
                    yyval.t = oz_pair2(prodName[depth],OZ_string(prodKeyBuffer[depth]));
                    prodName[depth] = PA_none;
                    prodKey[depth] = prodKeyBuffer[depth];
                  ;
    break;}
case 302:
{ yyval.t = AtomNil; ;
    break;}
case 303:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 304:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 305:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 306:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 307:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 308:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 309:
{ yyval.t = AtomNil; ;
    break;}
case 310:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 311:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 312:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 313:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 314:
{ yyval.t = newCTerm(PA_fSynAlternative, yyvsp[0].t); ;
    break;}
case 315:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 316:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 317:
{ OZ_Term t = yyvsp[0].t;
                    while (terms[depth]) {
                      t = oz_cons(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
                      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
                    }
                    yyval.t = newCTerm(PA_fSynSequence, decls[depth], t, yyvsp[-1].t);
                    decls[depth] = AtomNil;
                  ;
    break;}
case 318:
{ yyval.t = newCTerm(PA_fSynSequence, AtomNil, yyvsp[0].t, yyvsp[-1].t); ;
    break;}
case 319:
{ yyval.t = AtomNil; ;
    break;}
case 320:
{ yyval.t = oz_mklist(newCTerm(PA_fSynAction,yyvsp[0].t)); ;
    break;}
case 321:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 322:
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
case 323:
{ yyval.t = oz_cons(newCTerm(PA_fSynAssignment, terms[depth]->term, yyvsp[-1].t),
                                  yyvsp[0].t);
                    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
                  ;
    break;}
case 324:
{ while (terms[depth]) {
                      decls[depth] = oz_cons(terms[depth]->term, decls[depth]);
                      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
                    }
                    yyval.t = yyvsp[0].t;
                  ;
    break;}
case 325:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 326:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 327:
{ terms[depth] = new TermNode(yyvsp[0].t, terms[depth]); ;
    break;}
case 328:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 329:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 330:
{ yyval.t = newCTerm(PA_fSynAssignment,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 331:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 332:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 333:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
                                  oz_cons(newCTerm(PA_fSynApplication,yyvsp[-3].t,
                                                    AtomNil),
                                           AtomNil),yyvsp[-1].t);
                  ;
    break;}
case 334:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 335:
{ yyval.t = newCTerm(PA_fSynAssignment,
                                  newCTerm(PA_fEscape,yyvsp[-2].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 336:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 337:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 338:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
                                  oz_mklist(yyvsp[-2].t),yyvsp[-3].t);
                  ;
    break;}
case 339:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
                                  oz_mklist(yyvsp[-3].t),yyvsp[-1].t);
                  ;
    break;}
case 340:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 341:
{ depth--; ;
    break;}
case 342:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 343:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 344:
{ depth--; ;
    break;}
case 345:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 346:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 347:
{ depth--; ;
    break;}
case 348:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 349:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 350:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[-4].t,yyvsp[-1].t); ;
    break;}
case 351:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 352:
{ yyval.t = makeVar(xytext); ;
    break;}
case 353:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 354:
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
