
/*  A Bison parser, made from /home/denys/mozart/platform/emulator/parser.yy
    by GNU Bison version 1.28  */

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
#define T_2DOTS 274
#define T_attr  275
#define T_at    276
#define T_case  277
#define T_catch 278
#define T_choice        279
#define T_class 280
#define T_cond  281
#define T_declare       282
#define T_define        283
#define T_dis   284
#define T_else  285
#define T_elsecase      286
#define T_elseif        287
#define T_elseof        288
#define T_end   289
#define T_export        290
#define T_fail  291
#define T_false 292
#define T_FALSE_LABEL   293
#define T_feat  294
#define T_finally       295
#define T_from  296
#define T_fun   297
#define T_functor       298
#define T_if    299
#define T_import        300
#define T_in    301
#define T_local 302
#define T_lock  303
#define T_meth  304
#define T_not   305
#define T_of    306
#define T_or    307
#define T_prepare       308
#define T_proc  309
#define T_prop  310
#define T_raise 311
#define T_require       312
#define T_self  313
#define T_skip  314
#define T_then  315
#define T_thread        316
#define T_true  317
#define T_TRUE_LABEL    318
#define T_try   319
#define T_unit  320
#define T_UNIT_LABEL    321
#define T_for   322
#define T_FOR   323
#define T_do    324
#define T_ENDOFFILE     325
#define T_REGEX 326
#define T_lex   327
#define T_mode  328
#define T_parser        329
#define T_prod  330
#define T_scanner       331
#define T_syn   332
#define T_token 333
#define T_REDUCE        334
#define T_SEP   335
#define T_ITER  336
#define T_OOASSIGN      337
#define T_DOTASSIGN     338
#define T_orelse        339
#define T_andthen       340
#define T_COMPARE       341
#define T_FDCOMPARE     342
#define T_LMACRO        343
#define T_RMACRO        344
#define T_FDIN  345
#define T_ADD   346
#define T_FDMUL 347
#define T_OTHERMUL      348
#define T_DEREFF        349


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
#define PA_fDotAssign                           _PA_AtomTab[108]
#define PA_fFOR                                 _PA_AtomTab[109]

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
        "fDotAssign",                           //108
        "fFOR",                                 //109
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



#define YYFINAL         803
#define YYFLAG          -32768
#define YYNTBASE        117

#define YYTRANSLATE(x) ((unsigned)(x) <= 349 ? yytranslate[x] : 262)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   116,     2,    94,   109,     2,     2,     2,   106,
   107,     2,   104,    98,   105,   100,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,   114,   115,     2,
    83,     2,     2,   102,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   110,     2,   111,   101,   108,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   112,    93,   113,    99,     2,     2,     2,     2,
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
    88,    89,    90,    91,    92,    95,    96,    97,   103
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
   323,   329,   337,   345,   346,   349,   353,   357,   359,   364,
   368,   369,   372,   375,   376,   379,   382,   384,   393,   400,
   401,   404,   405,   408,   409,   411,   416,   423,   428,   433,
   438,   445,   450,   451,   455,   462,   465,   467,   471,   474,
   479,   480,   483,   484,   487,   492,   494,   496,   498,   500,
   502,   504,   509,   511,   512,   515,   517,   521,   522,   526,
   527,   530,   538,   546,   548,   550,   552,   554,   556,   557,
   560,   565,   566,   568,   570,   572,   574,   576,   578,   580,
   582,   584,   591,   594,   597,   601,   603,   611,   618,   621,
   624,   628,   630,   632,   636,   640,   642,   646,   650,   652,
   657,   664,   669,   674,   676,   681,   689,   691,   693,   694,
   697,   702,   707,   712,   717,   718,   721,   725,   727,   729,
   731,   733,   735,   737,   739,   740,   743,   749,   751,   756,
   758,   760,   762,   764,   766,   771,   777,   779,   781,   785,
   787,   789,   791,   794,   795,   798,   803,   805,   807,   809,
   813,   814,   820,   823,   824,   826,   830,   835,   841,   845,
   849,   852,   857,   862,   868,   870,   874,   876,   878,   880,
   884,   886,   888,   890,   892,   893,   894,   903,   905,   907,
   909,   912,   915,   918,   924,   930,   935,   937,   939,   944,
   945,   948,   951,   953,   955,   965,   967,   969,   972,   975,
   976,   979,   981,   984,   986,   990,   992,   995,   997,  1000,
  1001,  1011,  1012,  1013,  1024,  1031,  1035,  1038,  1041,  1043,
  1044,  1047,  1048,  1049,  1056,  1057,  1058,  1065,  1066,  1067,
  1074,  1076,  1080,  1082,  1084,  1086,  1087,  1089,  1091,  1093,
  1094,  1095,  1098,  1100,  1103,  1108,  1113,  1121,  1122,  1125,
  1127,  1129,  1131,  1133,  1135,  1139,  1142,  1146,  1147,  1150,
  1153,  1159,  1164,  1167,  1170,  1172,  1174,  1176,  1179,  1183,
  1185,  1187,  1192,  1194,  1200,  1202,  1204,  1210,  1215,  1216,
  1217,  1227,  1228,  1229,  1239,  1240,  1241,  1251,  1253,  1259,
  1261,  1263,  1265
};

static const short yyrhs[] = {   118,
    71,     0,     1,     0,   123,   119,     0,   216,   119,     0,
   119,     0,   200,   138,   119,     0,   120,   118,     0,    28,
   201,   123,    47,   200,   119,     0,    28,   201,   123,    47,
   123,   119,     0,    28,   201,   123,   200,   119,     0,     0,
     3,   121,     0,     5,     0,     6,     0,     7,     0,     0,
   122,   121,     0,   104,     4,     0,   105,     4,     0,   124,
     0,   124,   123,     0,   124,    83,   201,   124,     0,   124,
    85,   201,   124,     0,   124,    84,   201,   124,     0,   124,
    86,   201,   124,     0,   124,    87,   201,   124,     0,   124,
   144,   201,   124,     0,   124,   145,   201,   124,     0,   124,
   146,   201,   124,     0,   124,    93,   201,   124,     0,   126,
     0,   126,    94,   201,   125,     0,   126,     0,   126,    94,
   125,     0,   126,   147,   201,   126,     0,   126,   148,   201,
   126,     0,   126,   149,   201,   126,     0,   126,    98,   201,
   126,     0,    99,   201,   126,     0,   126,   100,   201,   126,
     0,   126,    13,     0,   126,   101,   201,   126,     0,   102,
   201,   126,     0,   103,   201,   126,     0,   106,   150,   107,
     0,   194,     0,   196,     0,   108,     0,    66,     0,    63,
     0,    38,     0,    59,     0,   109,     0,   197,     0,   198,
     0,   199,     0,   155,     0,   110,   201,   124,   152,   111,
   201,     0,   112,   201,   124,   151,   113,   201,     0,    55,
   201,   136,   112,   124,   151,   113,   150,    35,   201,     0,
    43,   201,   136,   112,   124,   151,   113,   150,    35,   201,
     0,    44,   201,   172,   137,    35,   201,     0,   171,     0,
    48,   201,   123,    47,   123,    35,     0,    45,   162,     0,
    23,   164,     0,    49,   201,   150,    35,   201,     0,    49,
   201,   124,    61,   150,    35,   201,     0,    62,   201,   150,
    35,   201,     0,    65,   201,   150,   153,   154,    35,   201,
     0,    57,   201,   150,    35,   201,     0,    60,     0,    37,
     0,    51,   201,   150,    35,   201,     0,    27,   187,     0,
    53,   201,   191,    35,   201,     0,    30,   201,   191,    35,
   201,     0,    25,   201,   193,    35,   201,     0,   202,     0,
   210,     0,    90,   201,   151,    91,   201,     0,    68,   201,
   133,    70,   150,    35,   201,     0,    69,   201,   127,    70,
   150,    35,   201,     0,     0,   128,   127,     0,   194,   114,
   124,     0,   124,    47,   129,     0,   124,     0,   124,    20,
   124,   130,     0,   124,   115,   131,     0,     0,   115,   124,
     0,   124,   132,     0,     0,   115,   124,     0,   133,   134,
     0,   134,     0,   195,    47,   201,   124,    20,   124,   135,
   201,     0,   195,    47,   201,   124,   135,   201,     0,     0,
   115,   124,     0,     0,   194,   136,     0,     0,   138,     0,
    58,   201,   139,   137,     0,    54,   201,   123,    47,   123,
   137,     0,    54,   201,   123,   137,     0,    46,   201,   139,
   137,     0,    36,   201,   143,   137,     0,    29,   201,   123,
    47,   123,   137,     0,    29,   201,   123,   137,     0,     0,
   195,   142,   139,     0,   140,   106,   141,   107,   142,   139,
     0,    16,   201,     0,   161,     0,   161,   114,   195,     0,
   161,   141,     0,   161,   114,   195,   141,     0,     0,    22,
   194,     0,     0,   195,   143,     0,   161,   114,   195,   143,
     0,    88,     0,    89,     0,    92,     0,    95,     0,    96,
     0,    97,     0,   123,    47,   201,   123,     0,   123,     0,
     0,   124,   151,     0,   200,     0,   200,   124,   152,     0,
     0,    24,   201,   167,     0,     0,    41,   150,     0,   156,
   201,   106,   158,   159,   107,   201,     0,   157,   201,   106,
   158,   159,   107,   201,     0,     9,     0,    67,     0,    64,
     0,    39,     0,    16,     0,     0,   124,   158,     0,   160,
   114,   124,   158,     0,     0,    19,     0,   194,     0,   195,
     0,   198,     0,    66,     0,    63,     0,    38,     0,   194,
     0,   198,     0,   201,   123,    61,   150,   163,   201,     0,
    33,   162,     0,    32,   164,     0,    31,   150,    35,     0,
    35,     0,   201,   123,    61,   201,   150,   165,   201,     0,
   201,   123,    52,   166,   165,   201,     0,    33,   162,     0,
    32,   164,     0,    31,   150,    35,     0,    35,     0,   168,
     0,   168,    18,   166,     0,   168,    34,   166,     0,   168,
     0,   168,    18,   167,     0,   169,    61,   150,     0,   170,
     0,   170,    87,   200,   123,     0,   170,    87,   200,   123,
    47,   123,     0,   170,    83,   201,   170,     0,   170,    93,
   201,   170,     0,   126,     0,   126,    94,   201,   125,     0,
    26,   201,   172,   173,   178,    35,   201,     0,   124,     0,
   200,     0,     0,   174,   173,     0,    42,   201,   124,   151,
     0,    21,   201,   176,   175,     0,    40,   201,   176,   175,
     0,    56,   201,   124,   151,     0,     0,   176,   175,     0,
   177,   114,   124,     0,   177,     0,   194,     0,   196,     0,
   198,     0,    66,     0,    63,     0,    38,     0,     0,   179,
   178,     0,    50,   201,   180,   150,    35,     0,   181,     0,
   181,    83,   201,   195,     0,   194,     0,   196,     0,    66,
     0,    63,     0,    38,     0,   182,   106,   183,   107,     0,
   182,   106,   183,    19,   107,     0,     9,     0,    16,     0,
   116,   201,    16,     0,    67,     0,    64,     0,    39,     0,
   184,   183,     0,     0,   185,   186,     0,   160,   114,   185,
   186,     0,   195,     0,   109,     0,   108,     0,    17,   201,
   124,     0,     0,   201,   189,   188,    35,   201,     0,    31,
   150,     0,     0,   190,     0,   190,    18,   189,     0,   123,
    61,   201,   150,     0,   123,    47,   123,    61,   150,     0,
   192,    18,   192,     0,   192,    18,   191,     0,   123,   200,
     0,   123,    47,   123,   200,     0,   123,   200,    61,   150,
     0,   123,    47,   123,    61,   150,     0,   150,     0,   150,
    18,   193,     0,     8,     0,    15,     0,   195,     0,   116,
   201,   195,     0,    14,     0,    11,     0,    12,     0,    10,
     0,     0,     0,    77,   201,   195,   173,   178,   203,    35,
   201,     0,   204,     0,   205,     0,   207,     0,   204,   203,
     0,   205,   203,     0,   207,   203,     0,    73,   194,    83,
   206,    35,     0,    73,   195,    83,   206,    35,     0,    73,
   206,   150,    35,     0,    72,     0,    14,     0,    74,   195,
   208,    35,     0,     0,   209,   208,     0,    42,   215,     0,
   205,     0,   207,     0,    75,   201,   195,   173,   178,   212,
   211,    35,   201,     0,   239,     0,   217,     0,   239,   211,
     0,   217,   211,     0,     0,    79,   213,     0,   214,     0,
   214,   213,     0,   194,     0,   194,   114,   124,     0,   195,
     0,   195,   215,     0,   217,     0,   217,   216,     0,     0,
    76,   195,    83,   218,   221,   236,   237,   242,    35,     0,
     0,     0,    76,   109,   219,    83,   220,   221,   236,   237,
   242,    35,     0,    76,   221,   236,   237,   242,    35,     0,
   223,   195,   234,     0,   195,   235,     0,   222,   224,     0,
   223,     0,     0,   194,   114,     0,     0,     0,   106,   225,
   231,   107,   226,   234,     0,     0,     0,   110,   227,   231,
   111,   228,   234,     0,     0,     0,   112,   229,   231,   113,
   230,   234,     0,   232,     0,   232,   233,   231,     0,   195,
     0,   108,     0,    81,     0,     0,   235,     0,    95,     0,
    96,     0,     0,     0,   238,    47,     0,   239,     0,   239,
   238,     0,    78,   194,   242,    35,     0,    78,   195,   242,
    35,     0,    78,   260,   106,   240,   107,   242,    35,     0,
     0,   241,   240,     0,   195,     0,   109,     0,   108,     0,
   243,     0,   244,     0,   244,    18,   243,     0,   200,   246,
     0,    60,   201,   245,     0,     0,    80,   150,     0,   247,
   246,     0,   247,   235,   201,   248,   236,     0,   247,    83,
   250,   248,     0,    47,   248,     0,   251,   248,     0,   245,
     0,   195,     0,   245,     0,   249,   248,     0,   196,    83,
   250,     0,   250,     0,   195,     0,   195,   235,   201,   236,
     0,   252,     0,   116,   201,   195,    83,   250,     0,   252,
     0,   259,     0,   223,   201,   259,   234,   236,     0,   259,
   235,   201,   236,     0,     0,     0,   222,   201,   106,   253,
   261,   107,   254,   234,   236,     0,     0,     0,   222,   201,
   110,   255,   261,   111,   256,   234,   236,     0,     0,     0,
   222,   201,   112,   257,   261,   113,   258,   234,   236,     0,
   194,     0,   260,   201,   106,   151,   107,     0,     9,     0,
    16,     0,   242,     0,   242,   233,   261,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   687,   689,   693,   695,   698,   700,   705,   707,   710,   712,
   715,   719,   721,   723,   725,   729,   731,   735,   742,   751,
   753,   757,   759,   761,   763,   765,   767,   770,   772,   774,
   776,   778,   784,   786,   790,   793,   796,   799,   801,   804,
   807,   810,   813,   815,   818,   820,   822,   824,   826,   828,
   830,   832,   834,   836,   838,   840,   842,   844,   848,   850,
   853,   856,   858,   860,   862,   864,   866,   868,   870,   872,
   874,   876,   878,   880,   882,   884,   886,   888,   890,   892,
   894,   896,   900,   904,   906,   910,   912,   916,   918,   920,
   924,   926,   930,   934,   936,   940,   944,   947,   961,   985,
   986,   990,   992,   997,   999,  1004,  1006,  1008,  1011,  1013,
  1015,  1017,  1022,  1024,  1026,  1030,  1034,  1036,  1038,  1040,
  1044,  1046,  1050,  1052,  1054,  1059,  1063,  1067,  1071,  1075,
  1079,  1083,  1085,  1089,  1091,  1095,  1097,  1103,  1105,  1109,
  1111,  1115,  1120,  1127,  1129,  1131,  1133,  1137,  1141,  1143,
  1145,  1149,  1151,  1155,  1157,  1159,  1161,  1163,  1165,  1169,
  1171,  1175,  1179,  1181,  1183,  1185,  1189,  1193,  1197,  1199,
  1201,  1203,  1207,  1209,  1211,  1215,  1217,  1221,  1225,  1227,
  1230,  1234,  1236,  1238,  1240,  1246,  1251,  1253,  1258,  1260,
  1264,  1266,  1268,  1270,  1274,  1276,  1280,  1282,  1286,  1288,
  1290,  1292,  1294,  1296,  1300,  1302,  1306,  1310,  1312,  1316,
  1318,  1320,  1322,  1324,  1326,  1328,  1332,  1334,  1336,  1338,
  1340,  1342,  1346,  1348,  1352,  1354,  1358,  1360,  1362,  1367,
  1369,  1373,  1377,  1379,  1383,  1385,  1389,  1391,  1395,  1397,
  1401,  1405,  1407,  1410,  1414,  1416,  1420,  1424,  1428,  1430,
  1434,  1438,  1440,  1444,  1448,  1452,  1462,  1470,  1472,  1474,
  1476,  1478,  1480,  1484,  1486,  1490,  1494,  1496,  1500,  1504,
  1506,  1510,  1512,  1514,  1520,  1528,  1530,  1532,  1534,  1538,
  1540,  1544,  1546,  1550,  1552,  1556,  1558,  1562,  1564,  1568,
  1570,  1572,  1573,  1574,  1576,  1580,  1582,  1584,  1588,  1589,
  1592,  1596,  1597,  1597,  1598,  1599,  1599,  1600,  1601,  1601,
  1604,  1606,  1610,  1611,  1614,  1618,  1619,  1622,  1623,  1626,
  1634,  1636,  1640,  1642,  1646,  1648,  1650,  1654,  1656,  1660,
  1662,  1664,  1668,  1672,  1674,  1678,  1687,  1691,  1693,  1697,
  1699,  1709,  1714,  1721,  1723,  1727,  1731,  1733,  1737,  1739,
  1743,  1745,  1751,  1755,  1758,  1763,  1765,  1769,  1773,  1774,
  1775,  1777,  1778,  1779,  1781,  1782,  1783,  1787,  1789,  1793,
  1795,  1800,  1802
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
"T_REDUCE","T_SEP","T_ITER","'='","T_OOASSIGN","T_DOTASSIGN","T_orelse","T_andthen",
"T_COMPARE","T_FDCOMPARE","T_LMACRO","T_RMACRO","T_FDIN","'|'","'#'","T_ADD",
"T_FDMUL","T_OTHERMUL","','","'~'","'.'","'^'","'@'","T_DEREFF","'+'","'-'",
"'('","')'","'_'","'$'","'['","']'","'{'","'}'","':'","';'","'!'","file","queries",
"queries1","directive","switchList","switch","sequence","phrase","hashes","phrase2",
"FOR_decls","FOR_decl","FOR_gen","FOR_genOptInt","FOR_genOptC","FOR_genOptC2",
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
   117,   117,   118,   118,   118,   118,   119,   119,   119,   119,
   119,   120,   120,   120,   120,   121,   121,   122,   122,   123,
   123,   124,   124,   124,   124,   124,   124,   124,   124,   124,
   124,   124,   125,   125,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
   126,   126,   126,   127,   127,   128,   128,   129,   129,   129,
   130,   130,   131,   132,   132,   133,   133,   134,   134,   135,
   135,   136,   136,   137,   137,   138,   138,   138,   138,   138,
   138,   138,   139,   139,   139,   140,   141,   141,   141,   141,
   142,   142,   143,   143,   143,   144,   145,   146,   147,   148,
   149,   150,   150,   151,   151,   152,   152,   153,   153,   154,
   154,   155,   155,   156,   156,   156,   156,   157,   158,   158,
   158,   159,   159,   160,   160,   160,   160,   160,   160,   161,
   161,   162,   163,   163,   163,   163,   164,   164,   165,   165,
   165,   165,   166,   166,   166,   167,   167,   168,   169,   169,
   169,   170,   170,   170,   170,   171,   172,   172,   173,   173,
   174,   174,   174,   174,   175,   175,   176,   176,   177,   177,
   177,   177,   177,   177,   178,   178,   179,   180,   180,   181,
   181,   181,   181,   181,   181,   181,   182,   182,   182,   182,
   182,   182,   183,   183,   184,   184,   185,   185,   185,   186,
   186,   187,   188,   188,   189,   189,   190,   190,   191,   191,
   192,   192,   192,   192,   193,   193,   194,   195,   196,   196,
   197,   198,   198,   199,   200,   201,   202,   203,   203,   203,
   203,   203,   203,   204,   204,   205,   206,   206,   207,   208,
   208,   209,   209,   209,   210,   211,   211,   211,   211,   212,
   212,   213,   213,   214,   214,   215,   215,   216,   216,   218,
   217,   219,   220,   217,   217,   221,   221,   221,   222,   222,
   223,   225,   226,   224,   227,   228,   224,   229,   230,   224,
   231,   231,   232,   232,   233,   234,   234,   235,   235,   236,
   237,   237,   238,   238,   239,   239,   239,   240,   240,   241,
   241,   241,   242,   243,   243,   244,   244,   245,   245,   246,
   246,   246,   246,   246,   246,   247,   248,   248,   249,   249,
   250,   250,   250,   251,   251,   252,   252,   252,   253,   254,
   252,   255,   256,   252,   257,   258,   252,   259,   259,   260,
   260,   261,   261
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
     5,     7,     7,     0,     2,     3,     3,     1,     4,     3,
     0,     2,     2,     0,     2,     2,     1,     8,     6,     0,
     2,     0,     2,     0,     1,     4,     6,     4,     4,     4,
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
     2,    16,    13,    14,    15,   247,   144,   254,   252,   253,
   251,   248,   148,   256,   256,   256,   256,   256,   256,    73,
    51,   147,   256,   256,   256,   256,   256,   256,   256,   256,
   256,    52,    72,   256,    50,   146,   256,    49,   145,   256,
   256,   256,   300,   256,   256,   256,   256,   256,     0,    48,
    53,   256,   256,   256,     0,     5,   255,    11,    20,    31,
    57,   256,   256,    63,    46,   249,    47,    54,    55,    56,
     0,    79,    80,    11,   288,     0,     0,    12,    16,    66,
     0,     0,   255,    75,     0,     0,     0,   102,   255,    65,
     0,     0,     0,     0,     0,   102,     0,     0,     0,     0,
    84,     0,   292,     0,     0,   320,     0,   299,     0,   134,
     0,     0,     0,   133,     0,     0,     0,     0,     1,     7,
     3,   256,   256,   256,   256,   256,   126,   127,   128,   256,
    21,   256,   256,   256,    41,   256,   129,   130,   131,   256,
   256,   256,   256,   256,   256,     0,     0,   256,   256,   256,
   256,   256,    11,     4,   289,    18,    19,    17,     0,   245,
     0,   187,   189,   188,     0,   234,   235,   255,   255,     0,
     0,     0,   102,   104,     0,     0,    20,     0,     0,     0,
     0,     0,     0,   138,     0,    97,     0,     0,     0,    84,
    46,   189,     0,   301,   290,   318,   319,   297,   321,   302,
   305,   308,   298,   316,   189,   134,     0,    39,    43,    44,
   256,    45,   255,   134,   250,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   149,   149,     0,   123,   113,     0,   113,     6,     0,
   256,     0,   256,   256,   256,   256,   256,   205,   189,     0,
   256,     0,     0,     0,   255,    11,     0,   241,   256,     0,
     0,   103,     0,   105,     0,     0,     0,   256,   256,   256,
     0,   256,   256,   256,   140,     0,    96,   256,     0,     0,
    85,     0,   205,   293,   300,     0,   255,     0,   323,     0,
     0,     0,   296,   317,   205,   135,   256,     0,     0,   136,
     0,    22,    24,    23,    25,    26,    30,    27,    28,    29,
    32,    33,    38,    40,    42,    35,    36,    37,    51,    50,
    49,   149,   152,     0,    46,   249,    55,   152,   104,   104,
     0,   160,   123,   161,   256,   104,     0,   121,   104,   104,
   184,     0,   173,     0,   179,     0,   246,    78,     0,     0,
     0,     0,   256,     0,   205,   190,     0,     0,   233,   256,
   236,    11,    11,    10,   255,     0,    77,   240,   239,   134,
   256,     0,     0,     0,    67,    74,    76,   134,    71,    69,
     0,     0,     0,     0,     0,    88,    87,     0,    86,   280,
   300,     0,   320,   370,   371,   255,   255,     0,   256,   338,
     0,   333,   334,   322,   324,   314,   313,     0,   311,     0,
     0,     0,    81,   132,   256,   255,   256,     0,   150,   153,
     0,     0,     0,     0,   112,   110,     0,   124,   116,   109,
     0,     0,   113,     0,   108,   106,   256,     0,   256,   256,
   172,   256,     0,     0,     0,   256,   255,   256,     0,   204,
   203,   202,   195,   198,   199,   200,   201,   195,   134,   134,
     0,   256,   206,     0,   237,   232,     9,     8,     0,   242,
   243,     0,    62,     0,   256,   256,   166,   256,    64,   256,
     0,   139,   176,   141,   256,   256,   100,     0,     0,   256,
     0,     0,   320,   321,     0,     0,   328,   338,   338,     0,
   256,   368,   346,   256,   256,   345,   336,   338,   338,   355,
   356,   256,   295,   255,   303,   315,     0,   306,   309,     0,
     0,     0,   258,   259,   260,    58,   137,    59,    34,   256,
   149,   256,   104,   123,     0,   117,   122,   114,   104,     0,
     0,   170,   169,   168,   174,   175,   178,     0,     0,     0,
   256,   192,   195,     0,   193,   191,   194,   217,   218,   214,
   222,   213,   221,   212,   220,   256,     0,   208,     0,   210,
   211,   186,   238,   244,     0,     0,   164,   163,   162,    68,
     0,     0,    70,    82,     0,     0,   256,    91,    94,    90,
    83,   284,   281,   282,     0,   277,   276,   321,   255,   325,
   326,   332,   331,   330,     0,   328,   337,   351,     0,   347,
   343,   338,   350,   353,   339,     0,     0,     0,   300,   256,
   340,   344,   256,     0,   335,   316,   312,   316,   316,   268,
   267,     0,     0,     0,   270,   256,   261,   262,   263,   142,
   151,   143,   111,   125,   121,     0,   119,   107,   185,   171,
   182,   180,   183,   167,   196,   197,     0,     0,   256,   224,
     0,   165,     0,   177,   100,   101,    99,     0,    89,     0,
    93,     0,   283,   256,   279,   278,   255,     0,   255,   329,
   256,   300,   348,     0,   359,   362,   365,   368,   316,   351,
   338,   338,   320,   134,   304,   307,   310,     0,     0,     0,
     0,     0,   273,   274,     0,   270,   257,   113,   118,     0,
   219,   207,     0,   159,   158,   157,   229,   228,     0,     0,
   224,   231,   154,   227,   156,   256,   256,   256,    92,    95,
   285,   275,     0,   291,     0,   320,   349,   300,   255,   255,
   255,   320,   342,   320,   358,     0,     0,     0,   266,   286,
   272,   269,   271,   115,   120,   181,   209,     0,     0,   215,
   223,   256,   225,    61,    60,    98,   294,   327,   352,   354,
   372,     0,     0,     0,   357,   341,   369,   264,   265,   287,
   231,   227,   216,     0,   255,   360,   363,   366,   226,   230,
   373,   316,   316,   316,   320,   320,   320,   361,   364,   367,
     0,     0,     0
};

static const short yydefgoto[] = {   801,
    55,    56,    57,    78,    79,   114,    59,   311,    60,   189,
   190,   387,   669,   590,   671,   185,   186,   587,   172,   263,
   264,   336,   337,   535,   433,   330,   132,   133,   134,   143,
   144,   145,   160,   207,   299,   275,   383,    61,    62,    63,
   323,   421,   324,   331,    90,   478,    80,   442,   342,   482,
   343,   344,   345,    64,   163,   248,   249,   552,   553,   454,
   354,   355,   567,   568,   569,   720,   721,   722,   763,    84,
   253,   166,   167,   170,   171,   161,    65,    66,    67,    68,
    69,    70,   400,    81,    72,   522,   523,   524,   634,   525,
   705,   706,    73,   595,   492,   593,   594,   751,    74,    75,
   285,   193,   391,   106,   504,   505,   203,   290,   626,   291,
   628,   292,   629,   408,   409,   517,   293,   294,   199,   287,
   288,   289,   605,   606,   771,   402,   403,   610,   507,   508,
   611,   612,   613,   509,   614,   739,   792,   740,   793,   741,
   794,   511,   512,   772
};

static const short yypact[] = {  1206,
-32768,   101,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,    77,-32768,-32768,-32768,-32768,-32768,  1754,-32768,
-32768,-32768,-32768,-32768,    41,-32768,  1318,   391,  1536,   292,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   179,-32768,-32768,   391,   -20,    74,   116,-32768,   101,-32768,
  1754,  1754,  1754,-32768,  1754,  1754,  1754,   172,  1754,-32768,
  1754,  1754,  1754,  1754,  1754,   172,  1754,  1754,  1754,   175,
  1754,   175,-32768,    86,    -9,-32768,   227,   175,   175,  1754,
  1754,  1754,  1754,   187,   135,  1754,  1754,   175,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   133,   150,-32768,-32768,-32768,
-32768,-32768,   391,-32768,-32768,-32768,-32768,-32768,   160,   246,
   252,   826,   262,-32768,   166,   288,   282,   278,   299,   334,
   363,   271,   172,   179,   324,   352,  1427,   366,   376,   380,
   311,   394,   399,   412,   132,-32768,   392,   564,   371,  1754,
   329,   262,   365,-32768,-32768,-32768,-32768,-32768,   386,-32768,
-32768,-32768,-32768,   155,   262,  1536,   378,   103,-32768,-32768,
-32768,-32768,   826,  1536,-32768,  1754,  1754,  1754,  1754,  1754,
  1754,  1754,  1754,  1754,  1754,  1754,  1754,  1754,  1754,  1754,
  1754,  1863,  1863,  1754,   591,   296,  1754,   296,-32768,  1754,
-32768,  1754,-32768,-32768,-32768,-32768,-32768,   432,   262,  1754,
-32768,  1754,   445,  1754,  1754,   391,  1754,   423,-32768,  1754,
  1754,-32768,   463,-32768,  1754,  1754,  1754,-32768,-32768,-32768,
  1754,-32768,-32768,-32768,   471,  1754,-32768,-32768,  1754,  1754,
-32768,  1754,   432,-32768,   131,   479,   456,   475,   386,    40,
    40,    40,-32768,-32768,   432,-32768,-32768,  1754,   419,  1754,
   425,   826,   779,   421,   851,   756,   441,   436,   224,   224,
-32768,   359,   140,-32768,-32768,   404,   140,   140,   426,   427,
   434,  1645,   520,   440,   442,   444,   447,   520,   506,   179,
   451,-32768,   591,-32768,-32768,   179,   453,   541,   629,   179,
   449,   538,    62,   507,   345,  1754,-32768,-32768,   157,   157,
  1754,  1754,-32768,   545,   432,-32768,   515,  1754,-32768,-32768,
-32768,   391,   391,-32768,   526,  1754,-32768,-32768,   363,  1536,
-32768,   562,   546,   553,-32768,-32768,-32768,  1536,-32768,-32768,
  1754,  1754,   556,   561,  1754,   320,-32768,   566,   826,   525,
   131,   155,-32768,-32768,-32768,   456,   456,   501,-32768,   730,
   574,-32768,   592,-32768,-32768,-32768,-32768,   508,   533,   509,
   504,   268,-32768,-32768,-32768,   826,-32768,  1754,-32768,-32768,
   511,  1754,   516,  1754,-32768,-32768,   175,-32768,-32768,-32768,
    87,   172,   296,  1754,-32768,-32768,-32768,  1754,-32768,-32768,
-32768,-32768,  1754,  1754,  1754,-32768,-32768,-32768,   538,-32768,
-32768,-32768,   157,   521,-32768,-32768,-32768,   157,  1536,  1536,
   260,-32768,-32768,  1754,-32768,-32768,-32768,-32768,  1754,-32768,
-32768,   512,-32768,  1754,-32768,-32768,-32768,-32768,-32768,-32768,
   523,-32768,   604,-32768,-32768,-32768,   769,  1754,  1754,-32768,
   172,   225,-32768,   386,   602,   606,    35,   565,    75,  1754,
-32768,    86,-32768,-32768,   261,-32768,-32768,   718,    75,-32768,
   155,-32768,-32768,   456,-32768,-32768,    40,-32768,-32768,   264,
   175,   620,   268,   268,   268,-32768,-32768,-32768,-32768,-32768,
  1645,-32768,   179,   591,   560,    65,-32768,-32768,   179,  1754,
   625,-32768,-32768,-32768,-32768,-32768,-32768,  1754,  1754,  1754,
-32768,-32768,   157,  1754,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,  1754,   580,   563,-32768,
-32768,-32768,-32768,-32768,  1754,   633,-32768,-32768,-32768,-32768,
  1754,  1754,-32768,-32768,  1754,  1754,-32768,   610,   808,-32768,
-32768,   558,-32768,   172,   643,   225,   225,   386,   456,-32768,
-32768,-32768,-32768,-32768,   575,    35,-32768,   270,   603,-32768,
-32768,    75,-32768,-32768,-32768,   175,   304,   208,   479,-32768,
-32768,-32768,-32768,   582,-32768,   155,-32768,   155,   155,-32768,
-32768,   608,   622,  1754,   255,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   541,   175,-32768,-32768,-32768,-32768,
    10,   659,   614,-32768,-32768,   826,   430,   673,-32768,   269,
   674,-32768,   679,-32768,   841,   826,-32768,  1754,-32768,  1754,
-32768,  1754,-32768,-32768,-32768,-32768,   456,   682,   456,-32768,
-32768,   479,-32768,   636,-32768,-32768,-32768,-32768,   155,   155,
    75,    75,-32768,  1754,-32768,-32768,-32768,    51,    51,   685,
   175,    51,-32768,-32768,   688,   255,-32768,   296,    87,  1754,
-32768,-32768,   175,-32768,-32768,-32768,-32768,-32768,   615,    52,
   269,   711,-32768,   444,-32768,-32768,-32768,-32768,   826,   826,
   826,-32768,   695,-32768,   696,-32768,-32768,   479,   456,   456,
   456,-32768,-32768,-32768,-32768,   628,   701,   702,-32768,   175,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,    89,   634,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   533,   635,   632,   631,-32768,-32768,-32768,-32768,-32768,-32768,
   711,-32768,-32768,  1754,   456,-32768,-32768,-32768,-32768,   826,
-32768,   155,   155,   155,-32768,-32768,-32768,-32768,-32768,-32768,
   748,   751,-32768
};

static const short yypgoto[] = {-32768,
   703,    -4,-32768,   684,-32768,    97,   410,  -395,   527,   578,
-32768,-32768,-32768,-32768,-32768,-32768,   579,   104,   -35,  -272,
   699,  -232,-32768,  -500,   127,  -315,-32768,-32768,-32768,-32768,
-32768,-32768,   216,  -171,   357,-32768,-32768,-32768,-32768,-32768,
  -209,   448,  -597,  -414,  -374,-32768,  -370,   332,     7,   196,
  -360,-32768,  -242,-32768,   697,    43,-32768,  -412,   117,-32768,
  -223,-32768,-32768,-32768,-32768,    63,-32768,    27,     6,-32768,
-32768,   536,-32768,   -46,   532,   551,   489,    58,   -79,-32768,
   351,-32768,     5,   -15,-32768,  -163,-32768,  -535,  -226,  -529,
    88,-32768,-32768,   -78,-32768,   205,-32768,    50,   727,  -433,
-32768,-32768,-32768,  -240,   -36,   -23,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,  -251,-32768,    32,  -547,   -71,  -318,  -441,
   517,  -403,   199,-32768,  -235,   298,-32768,  -358,   307,-32768,
  -481,-32768,  -568,-32768,  -356,-32768,-32768,-32768,-32768,-32768,
-32768,   200,   531,  -455
};


#define YYLAST          1979


static const short yytable[] = {    82,
    83,    85,    86,    87,    71,   340,   107,    88,    89,    91,
    92,    93,    94,    95,    96,    97,   536,   428,    98,   108,
   483,    99,   529,   328,   100,   101,   102,   622,   109,   110,
   111,   112,   113,   198,   296,   647,   116,   117,   118,   410,
   411,   506,   301,   510,   393,   555,   146,   147,   180,    12,
   691,   401,   599,   121,    12,    43,   425,   426,   596,   390,
   181,    71,   719,   430,   630,   543,   435,   436,   542,   154,
   759,   412,     6,   195,   494,     9,    10,   156,   695,   443,
   696,   697,     6,   394,     6,   196,   197,   164,   597,    12,
   395,    12,   446,   164,     6,   444,    58,     9,    10,   703,
   105,   578,   448,    12,   577,   704,   216,   217,   218,   219,
   220,   119,   419,   737,   221,   135,   222,   223,   224,   157,
   225,   536,   631,   719,   226,   227,   228,   229,   230,   231,
   683,   463,   234,   235,   236,   237,   238,   262,     6,   607,
   655,   742,   602,   603,   649,    12,    12,   406,   239,   506,
   493,   510,   135,    58,   500,   131,   677,   187,   760,   192,
   495,   496,   596,   596,     6,   204,   205,     9,    10,   770,
   703,    12,   256,   258,   598,   215,   704,   159,   646,     6,
  -300,   165,   168,   169,  -300,   103,  -300,   175,   176,    12,
    54,   169,   597,   597,   450,   298,   717,   718,   472,   194,
   538,   276,   141,   142,    76,    77,   481,   148,   755,   743,
   744,   240,   250,   368,   149,     6,   394,   300,   644,   451,
   241,   483,   452,   395,   150,   346,   251,   348,   349,   350,
   351,   352,   151,   211,   283,   358,   152,   140,   232,   141,
   142,   212,   187,   367,   795,   796,   797,   295,   107,   196,
   197,   364,   375,   376,   377,   233,   379,   380,   381,   363,
   643,   108,   385,   242,   115,   627,   648,     6,   558,   456,
   456,     6,    54,   131,    12,   559,     6,   630,    12,     9,
    10,   413,   244,    12,   773,   774,   243,   556,   557,   326,
   326,   356,   333,   338,   536,   338,   701,   560,   561,   254,
    43,   245,   286,   246,   135,   651,   714,   653,   178,   179,
    12,   335,   182,   183,   184,-32768,   130,   247,   252,   429,
   198,   641,   562,   563,   255,   564,   565,   702,   521,   791,
   329,   715,   200,   339,   716,   631,   201,   461,   202,   488,
   520,   521,   392,   397,   466,   257,   357,   407,   407,   407,
   165,   362,  -249,   365,   107,   473,   169,   467,   468,   637,
   638,   639,   373,   678,   196,   197,  -299,   108,   259,   470,
  -299,   135,  -299,   456,   745,   566,   717,   718,   456,   326,
   260,   571,   261,   498,   265,   136,   137,   138,   139,   140,
   333,   141,   142,     2,   414,     3,     4,     5,   266,   526,
   268,   528,   122,   123,   124,   125,   126,   127,   128,   685,
   269,   129,   130,   686,   270,   687,   135,   769,    18,   609,
   300,   540,   271,   775,    91,   776,   544,   446,   272,   609,
   548,   447,   550,   273,   489,   274,   620,   448,   278,   623,
   280,   733,   282,   735,    12,   711,   572,   284,   392,   545,
   546,   549,   418,   137,   138,   139,   140,   503,   141,   142,
    91,   135,   579,   286,   580,   453,   458,   359,   297,   583,
   584,   747,   748,   456,   591,   754,   798,   799,   800,   360,
   372,   353,   374,   366,   534,   616,     6,   394,   617,   618,
   338,   384,   162,    12,   395,   388,   624,   371,   162,   138,
   139,   140,   177,   141,   142,-32768,   125,   126,   127,   128,
   188,   382,   129,   130,   640,   399,   642,   675,   676,   206,
   533,   404,   746,-32768,-32768,   213,   214,   129,   130,   415,
   539,   104,   609,   130,   148,   654,   681,   417,   420,  -159,
  -158,   149,   437,   137,   138,   139,   140,  -157,   141,   142,
   657,   150,   424,   422,   604,  -154,   608,  -155,   431,   151,
  -156,   449,   432,   152,   427,   503,   608,   445,   438,   439,
   440,   667,   441,   465,   407,   464,   173,   633,   635,   462,
   479,   471,   327,   327,   173,   334,   469,   480,   326,   191,
   485,   333,   474,   475,   476,   486,   477,   484,     6,   188,
   490,     9,    10,   491,   692,    12,   497,   693,   513,   514,
   279,   609,   609,   516,   515,   206,   519,   530,   681,   518,
   707,   582,   532,   206,   575,   302,   303,   304,   305,   306,
   307,   308,   309,   310,   554,   581,   600,   208,   209,   210,
   601,   322,   322,   713,   500,   652,   122,   123,   124,   125,
   126,   127,   128,   541,   636,   129,   130,   148,   732,   650,
   547,   173,   659,   604,   149,   736,   645,   662,   660,   608,
   370,   672,   327,   684,   150,   434,   690,   674,   191,   573,
   378,   679,   151,   334,   574,   682,   152,   694,   386,   576,
   698,   389,   122,   123,   124,   125,   126,   127,   128,   457,
   457,   129,   130,   709,   699,   710,   448,   712,   726,   416,
   764,   765,   766,   727,   215,   615,   734,   724,   738,   749,
   325,   325,   752,   332,   668,     6,   394,   762,   758,   767,
   768,   322,    12,   395,   777,   778,   779,     6,   394,   690,
   783,   786,   787,   788,    12,   395,   784,   802,   608,   608,
   803,   312,   313,   314,   315,   316,   317,   318,   750,   120,
   459,   460,   158,   277,   499,   338,   341,   281,   728,   153,
   757,   708,   527,   104,   396,   423,   499,   664,   724,   206,
   551,   334,   658,   761,   781,   174,   789,   206,   585,   361,
   661,   369,   347,   753,   487,   690,   663,   500,   673,   780,
   619,   155,   785,   457,   680,   405,   756,   750,   457,   500,
   325,   625,   196,   197,   621,   782,   398,   689,     0,     0,
     0,   332,     0,  -300,     0,     0,     0,  -300,     0,  -300,
     0,   531,     0,   501,     0,  -300,     0,   455,   455,  -300,
     0,  -300,   126,   127,   128,   501,     0,   129,   130,   700,
     0,   122,   123,   124,   125,   126,   127,   128,     0,     0,
   129,   130,   123,   124,   125,   126,   127,   128,   206,   206,
   129,   130,     0,     0,     0,     0,     0,     0,     0,   104,
     0,   327,     0,   586,   334,     0,   334,     0,   502,     0,
   122,   123,   124,   125,   126,   127,   128,   588,   589,   129,
   130,     0,     0,   457,     0,     0,     0,   341,   122,   123,
   124,   125,   126,   127,   128,     0,     0,   129,   130,   332,
   537,     0,   670,   122,   123,   124,   125,   126,   127,   128,
     0,     0,   129,   130,     0,     0,   125,   126,   127,   128,
   322,   455,   129,   130,   312,     0,   455,     0,     0,   570,
     0,     0,     0,     0,     0,   586,     0,     0,     0,     0,
     0,     0,     0,   656,     0,     0,     0,     0,     0,   341,
   341,     0,     0,     0,     0,     0,     0,     0,     0,   592,
     0,     0,     0,     0,     0,     0,     0,   502,     0,     0,
     0,     0,     0,     0,   665,   666,   502,   502,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   632,     0,
   725,     0,     0,     0,     0,     0,     0,     0,     0,   325,
     0,     0,   332,     0,   332,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   455,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   334,
     0,     0,     0,     0,     0,     0,   312,     0,     0,     0,
     0,   725,     0,     0,   341,     0,   341,   729,     0,   730,
     0,   731,   592,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
   502,     0,     0,   206,     0,     0,   688,   502,   341,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   723,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
   502,     0,     0,     0,     0,     0,     0,     0,     0,   502,
   502,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   790,     0,     0,     0,   332,     0,     0,
     0,     0,     0,     0,     0,     0,     1,     0,     2,   723,
     3,     4,     5,     6,     7,     8,     9,    10,     0,    11,
    12,    13,     0,     0,     0,     0,   502,     0,    14,     0,
    15,    16,    17,    18,  -255,    19,     0,     0,     0,     0,
     0,  -255,    20,    21,    22,     0,     0,     0,    23,    24,
    25,  -255,     0,    26,    27,     0,    28,     0,    29,  -255,
    30,     0,    31,  -255,    32,    33,     0,    34,    35,    36,
    37,    38,    39,    40,    41,     0,   -11,     0,     0,     0,
    42,    43,    44,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    45,     0,     0,     0,     0,
     0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
     0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
     2,    54,     3,     4,     5,     6,     7,     8,     9,    10,
     0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
    14,     0,    15,    16,    17,    18,     0,    19,     0,     0,
     0,     0,     0,     0,    20,    21,    22,     0,     0,     0,
    23,    24,    25,     0,     0,    26,    27,     0,    28,     0,
    29,     0,    30,     0,    31,     0,    32,    33,     0,    34,
    35,    36,    37,    38,    39,    40,    41,     0,   -11,     0,
     0,     0,    42,    43,    44,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    45,     0,     0,
     0,     0,     0,     0,     0,     0,    46,     0,     0,    47,
    48,     0,     0,    49,     0,    50,    51,    52,     0,    53,
     0,     0,     0,    54,     6,     7,     8,     9,    10,     0,
    11,    12,    13,     0,     0,     0,     0,     0,     0,    14,
     0,    15,    16,    17,     0,     0,    19,     0,     0,     0,
     0,     0,     0,    20,    21,    22,     0,     0,     0,    23,
    24,    25,     0,     0,    26,    27,     0,    28,     0,    29,
     0,    30,     0,    31,     0,    32,    33,   267,    34,    35,
    36,    37,    38,    39,    40,    41,     0,     0,     0,     0,
     0,    42,     0,    44,     0,     0,     0,     0,     0,   122,
   123,   124,   125,   126,   127,   128,    45,     0,   129,   130,
     0,     0,     0,     0,     0,    46,     0,     0,    47,    48,
     0,     0,    49,     0,    50,    51,    52,     0,    53,     0,
     0,     0,    54,     6,     7,     8,     9,    10,     0,    11,
    12,    13,     0,     0,     0,     0,     0,     0,    14,     0,
    15,    16,    17,     0,     0,    19,     0,     0,     0,     0,
     0,     0,    20,    21,    22,     0,     0,     0,    23,    24,
    25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
    30,     0,    31,     0,    32,    33,     0,    34,    35,    36,
    37,    38,    39,    40,    41,     0,     0,     0,     0,     0,
    42,     0,    44,     0,     0,     0,     0,     0,   122,   123,
   124,   125,   126,   127,   128,    45,     0,   129,   130,     0,
     0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
     0,    49,     0,    50,    51,    52,     0,    53,     0,     0,
     0,    54,     6,     7,     8,     9,    10,     0,    11,    12,
    13,     0,     0,     0,     0,     0,     0,    14,     0,    15,
    16,    17,     0,     0,    19,     0,     0,     0,     0,     0,
     0,    20,   319,    22,     0,     0,     0,    23,    24,    25,
     0,     0,    26,    27,     0,    28,     0,    29,     0,    30,
     0,    31,     0,    32,    33,     0,    34,   320,    36,    37,
   321,    39,    40,    41,     0,     0,     0,     0,     0,    42,
     0,    44,     0,     0,     0,     0,     0,   122,   123,   124,
   125,   126,   127,   128,    45,     0,   129,   130,     0,     0,
     0,     0,     0,    46,     0,     0,    47,    48,     0,     0,
    49,     0,    50,    51,    52,     0,    53,     0,     0,     0,
    54,     6,     7,     8,     9,    10,     0,    11,    12,    13,
     0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
    17,     0,     0,    19,     0,     0,     0,     0,     0,     0,
    20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
     0,    26,    27,     0,    28,     0,    29,     0,    30,     0,
    31,     0,    32,    33,     0,    34,    35,    36,    37,    38,
    39,    40,    41,     0,     0,     0,     0,     0,    42,     0,
    44,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    45,     0,     0,     0,     0,     0,     0,
     0,     0,    46,     0,     0,    47,    48,     0,     0,    49,
     0,    50,    51,    52,     0,    53,     0,     0,     0,    54,
     6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
     0,     0,     0,     0,     0,    14,     0,    15,    16,    17,
     0,     0,    19,     0,     0,     0,     0,     0,     0,    20,
   319,    22,     0,     0,     0,    23,    24,    25,     0,     0,
    26,    27,     0,    28,     0,    29,     0,    30,     0,    31,
     0,    32,    33,     0,    34,   320,    36,    37,   321,    39,
    40,    41,     0,     0,     0,     0,     0,    42,     0,    44,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    45,     0,     0,     0,     0,     0,     0,     0,
     0,    46,     0,     0,    47,    48,     0,     0,    49,     0,
    50,    51,    52,     0,    53,     0,     0,     0,    54
};

static const short yycheck[] = {    15,
    16,    17,    18,    19,     0,   238,    43,    23,    24,    25,
    26,    27,    28,    29,    30,    31,   431,   333,    34,    43,
   381,    37,   418,   233,    40,    41,    42,   509,    44,    45,
    46,    47,    48,   105,   206,   536,    52,    53,    54,   291,
   292,   400,   214,   400,   285,   458,    62,    63,    95,    15,
   619,   287,   494,    58,    15,    76,   329,   330,   492,   283,
    96,    57,   660,   336,    14,   440,   339,   340,   439,    74,
    19,   295,     8,    83,   393,    11,    12,     4,   626,    18,
   628,   629,     8,     9,     8,    95,    96,    83,   492,    15,
    16,    15,    83,    89,     8,    34,     0,    11,    12,   635,
    43,   476,    93,    15,   475,   635,   122,   123,   124,   125,
   126,    71,   322,   682,   130,    13,   132,   133,   134,     4,
   136,   536,    72,   721,   140,   141,   142,   143,   144,   145,
   612,   355,   148,   149,   150,   151,   152,   173,     8,   498,
   553,   689,   108,   109,   540,    15,    15,   108,   153,   508,
   391,   508,    13,    57,    80,    59,   598,   100,   107,   102,
   396,   397,   596,   597,     8,   108,   109,    11,    12,   738,
   706,    15,   168,   169,   493,   118,   706,    81,   114,     8,
   106,    85,    86,    87,   110,   109,   112,    91,    92,    15,
   116,    95,   596,   597,    38,   211,   108,   109,   370,   114,
   433,    70,   100,   101,   104,   105,   378,    29,   709,   691,
   692,    52,    47,   260,    36,     8,     9,   213,   534,    63,
    61,   582,    66,    16,    46,   241,    61,   243,   244,   245,
   246,   247,    54,    47,   192,   251,    58,    98,   106,   100,
   101,   107,   185,   259,   792,   793,   794,   205,   285,    95,
    96,   256,   268,   269,   270,   106,   272,   273,   274,   255,
   533,   285,   278,    18,    49,   517,   539,     8,     9,   349,
   350,     8,   116,   177,    15,    16,     8,    14,    15,    11,
    12,   297,    21,    15,   740,   741,    35,   459,   460,   232,
   233,   249,   235,   236,   709,   238,    42,    38,    39,    18,
    76,    40,    78,    42,    13,   548,    38,   550,    93,    94,
    15,    16,    97,    98,    99,    92,    93,    56,    31,   335,
   392,   531,    63,    64,    47,    66,    67,    73,    74,   785,
   234,    63,   106,   237,    66,    72,   110,   353,   112,    20,
    73,    74,   285,   286,   360,    47,   250,   290,   291,   292,
   254,   255,    83,   257,   391,   371,   260,   362,   363,   523,
   524,   525,   266,   599,    95,    96,   106,   391,    35,   365,
   110,    13,   112,   453,   693,   116,   108,   109,   458,   322,
    18,   461,   112,   399,    61,    94,    95,    96,    97,    98,
   333,   100,   101,     3,   298,     5,     6,     7,    47,   415,
    35,   417,    83,    84,    85,    86,    87,    88,    89,   106,
    35,    92,    93,   110,    35,   112,    13,   736,    28,   499,
   416,   437,   112,   742,   440,   744,   442,    83,    35,   509,
   446,    87,   448,    35,   115,    24,   508,    93,    47,   511,
    70,   677,   114,   679,    15,    16,   462,    83,   391,   443,
   444,   447,    94,    95,    96,    97,    98,   400,   100,   101,
   476,    13,   478,    78,   480,   349,   350,   252,    91,   485,
   486,   698,   699,   553,   490,   708,   795,   796,   797,    35,
   265,    50,   267,    61,   427,   501,     8,     9,   504,   505,
   433,   276,    83,    15,    16,   280,   512,    35,    89,    96,
    97,    98,    93,   100,   101,    85,    86,    87,    88,    89,
   101,    41,    92,    93,   530,    60,   532,   596,   597,   110,
   424,    47,   694,    88,    89,   116,   117,    92,    93,   111,
   434,    43,   612,    93,    29,   551,   608,   113,    19,   114,
   114,    36,    94,    95,    96,    97,    98,   114,   100,   101,
   566,    46,    47,   114,   497,   114,   499,   114,   106,    54,
   114,   346,    22,    58,   114,   508,   509,    61,    31,    32,
    33,   587,    35,   358,   517,    61,    88,   520,   521,    35,
    35,   366,   232,   233,    96,   235,    61,    35,   531,   101,
    35,   534,    31,    32,    33,    35,    35,   382,     8,   190,
    35,    11,    12,    79,   620,    15,   106,   623,    35,    18,
    47,   691,   692,    81,   107,   206,   113,   107,   690,   111,
   636,    18,   107,   214,   113,   216,   217,   218,   219,   220,
   221,   222,   223,   224,   114,   113,    35,   111,   112,   113,
    35,   232,   233,   659,    80,   549,    83,    84,    85,    86,
    87,    88,    89,   438,    35,    92,    93,    29,   674,    35,
   445,   173,    83,   606,    36,   681,   107,    35,   106,   612,
   261,   114,   322,   616,    46,    47,   619,    35,   190,   464,
   271,   107,    54,   333,   469,    83,    58,   106,   279,   474,
    83,   282,    83,    84,    85,    86,    87,    88,    89,   349,
   350,    92,    93,   646,    83,    47,    93,    35,    35,   300,
   726,   727,   728,    35,   657,   500,    35,   660,    83,    35,
   232,   233,    35,   235,   115,     8,     9,    17,   114,    35,
    35,   322,    15,    16,   107,    35,    35,     8,     9,   682,
   107,   107,   111,   113,    15,    16,   762,     0,   691,   692,
     0,   225,   226,   227,   228,   229,   230,   231,   701,    57,
   351,   352,    79,   185,    47,   708,   240,   190,   665,    71,
   713,   645,   416,   285,   286,   328,    47,   582,   721,   370,
   449,   431,   567,   721,   758,    89,   781,   378,    20,   254,
   575,   260,   242,   706,   385,   738,   581,    80,   594,   750,
    83,    75,   771,   453,   606,   289,   710,   750,   458,    80,
   322,   514,    95,    96,   508,   758,   286,   618,    -1,    -1,
    -1,   333,    -1,   106,    -1,    -1,    -1,   110,    -1,   112,
    -1,   422,    -1,   116,    -1,   106,    -1,   349,   350,   110,
    -1,   112,    87,    88,    89,   116,    -1,    92,    93,   634,
    -1,    83,    84,    85,    86,    87,    88,    89,    -1,    -1,
    92,    93,    84,    85,    86,    87,    88,    89,   459,   460,
    92,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   391,
    -1,   531,    -1,   115,   534,    -1,   536,    -1,   400,    -1,
    83,    84,    85,    86,    87,    88,    89,   488,   489,    92,
    93,    -1,    -1,   553,    -1,    -1,    -1,   381,    83,    84,
    85,    86,    87,    88,    89,    -1,    -1,    92,    93,   431,
   432,    -1,   115,    83,    84,    85,    86,    87,    88,    89,
    -1,    -1,    92,    93,    -1,    -1,    86,    87,    88,    89,
   531,   453,    92,    93,   418,    -1,   458,    -1,    -1,   461,
    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,   554,    -1,    -1,    -1,    -1,    -1,   443,
   444,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   491,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,   499,    -1,    -1,
    -1,    -1,    -1,    -1,   585,   586,   508,   509,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   520,    -1,
   660,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   531,
    -1,    -1,   534,    -1,   536,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,   553,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   709,
    -1,    -1,    -1,    -1,    -1,    -1,   540,    -1,    -1,    -1,
    -1,   721,    -1,    -1,   548,    -1,   550,   668,    -1,   670,
    -1,   672,   594,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
   612,    -1,    -1,   694,    -1,    -1,   618,   619,   582,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   660,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
   682,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   691,
   692,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,   784,    -1,    -1,    -1,   709,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,     3,   721,
     5,     6,     7,     8,     9,    10,    11,    12,    -1,    14,
    15,    16,    -1,    -1,    -1,    -1,   738,    -1,    23,    -1,
    25,    26,    27,    28,    29,    30,    -1,    -1,    -1,    -1,
    -1,    36,    37,    38,    39,    -1,    -1,    -1,    43,    44,
    45,    46,    -1,    48,    49,    -1,    51,    -1,    53,    54,
    55,    -1,    57,    58,    59,    60,    -1,    62,    63,    64,
    65,    66,    67,    68,    69,    -1,    71,    -1,    -1,    -1,
    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    99,    -1,    -1,   102,   103,    -1,
    -1,   106,    -1,   108,   109,   110,    -1,   112,    -1,    -1,
     3,   116,     5,     6,     7,     8,     9,    10,    11,    12,
    -1,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
    23,    -1,    25,    26,    27,    28,    -1,    30,    -1,    -1,
    -1,    -1,    -1,    -1,    37,    38,    39,    -1,    -1,    -1,
    43,    44,    45,    -1,    -1,    48,    49,    -1,    51,    -1,
    53,    -1,    55,    -1,    57,    -1,    59,    60,    -1,    62,
    63,    64,    65,    66,    67,    68,    69,    -1,    71,    -1,
    -1,    -1,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,   102,
   103,    -1,    -1,   106,    -1,   108,   109,   110,    -1,   112,
    -1,    -1,    -1,   116,     8,     9,    10,    11,    12,    -1,
    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,
    -1,    25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,
    -1,    -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,
    44,    45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,
    -1,    55,    -1,    57,    -1,    59,    60,    61,    62,    63,
    64,    65,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
    -1,    75,    -1,    77,    -1,    -1,    -1,    -1,    -1,    83,
    84,    85,    86,    87,    88,    89,    90,    -1,    92,    93,
    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,   102,   103,
    -1,    -1,   106,    -1,   108,   109,   110,    -1,   112,    -1,
    -1,    -1,   116,     8,     9,    10,    11,    12,    -1,    14,
    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,
    25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,
    -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,
    45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,
    55,    -1,    57,    -1,    59,    60,    -1,    62,    63,    64,
    65,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,
    75,    -1,    77,    -1,    -1,    -1,    -1,    -1,    83,    84,
    85,    86,    87,    88,    89,    90,    -1,    92,    93,    -1,
    -1,    -1,    -1,    -1,    99,    -1,    -1,   102,   103,    -1,
    -1,   106,    -1,   108,   109,   110,    -1,   112,    -1,    -1,
    -1,   116,     8,     9,    10,    11,    12,    -1,    14,    15,
    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,
    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,
    -1,    57,    -1,    59,    60,    -1,    62,    63,    64,    65,
    66,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    75,
    -1,    77,    -1,    -1,    -1,    -1,    -1,    83,    84,    85,
    86,    87,    88,    89,    90,    -1,    92,    93,    -1,    -1,
    -1,    -1,    -1,    99,    -1,    -1,   102,   103,    -1,    -1,
   106,    -1,   108,   109,   110,    -1,   112,    -1,    -1,    -1,
   116,     8,     9,    10,    11,    12,    -1,    14,    15,    16,
    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,    26,
    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,    -1,
    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,    -1,
    57,    -1,    59,    60,    -1,    62,    63,    64,    65,    66,
    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    75,    -1,
    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    99,    -1,    -1,   102,   103,    -1,    -1,   106,
    -1,   108,   109,   110,    -1,   112,    -1,    -1,    -1,   116,
     8,     9,    10,    11,    12,    -1,    14,    15,    16,    -1,
    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,    26,    27,
    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,
    38,    39,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,
    48,    49,    -1,    51,    -1,    53,    -1,    55,    -1,    57,
    -1,    59,    60,    -1,    62,    63,    64,    65,    66,    67,
    68,    69,    -1,    -1,    -1,    -1,    -1,    75,    -1,    77,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    99,    -1,    -1,   102,   103,    -1,    -1,   106,    -1,
   108,   109,   110,    -1,   112,    -1,    -1,    -1,   116
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
{ yyval.t = newCTerm(PA_fFOR,yyvsp[-4].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 84:
{ yyval.t = AtomNil; ;
    break;}
case 85:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 86:
{ yyval.t = newCTerm(oz_atom("forFeature"),yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 87:
{ yyval.t = newCTerm(oz_atom("forPattern"),yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 88:
{ yyval.t = newCTerm(oz_atom("forGeneratorList"),yyvsp[0].t); ;
    break;}
case 89:
{ yyval.t = newCTerm(oz_atom("forGeneratorInt"),yyvsp[-3].t,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 90:
{ yyval.t = newCTerm(oz_atom("forGeneratorC"),yyvsp[-2].t,oz_head(yyvsp[0].t),oz_tail(yyvsp[0].t)); ;
    break;}
case 91:
{ yyval.t = NameUnit; ;
    break;}
case 92:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 93:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 94:
{ yyval.t = NameUnit; ;
    break;}
case 95:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 96:
{
                    yyval.t = newCTerm(PA_fAnd,yyvsp[-1].t,yyvsp[0].t);
                  ;
    break;}
case 98:
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
                                  makeLongPos(OZ_subtree(yyvsp[-7].t,makeTaggedSmallInt(2)),yyvsp[0].t));
                  ;
    break;}
case 99:
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
                                    makeLongPos(OZ_subtree(yyvsp[-5].t,makeTaggedSmallInt(2)),yyvsp[0].t));
                    } else {
                      yyval.t = newCTerm(PA_fMacro,
                                    oz_list(newCTerm(PA_fAtom,oz_atom("for"),NameUnit),
                                            newCTerm(PA_fEq,yyvsp[-5].t,yyvsp[-2].t,NameUnit),
                                            newCTerm(PA_fAtom,oz_atom("next"),NameUnit),
                                            yyvsp[-1].t,
                                            0),
                                    makeLongPos(OZ_subtree(yyvsp[-5].t,makeTaggedSmallInt(2)),yyvsp[0].t));
                    }
                  ;
    break;}
case 100:
{ yyval.t = 0; ;
    break;}
case 101:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 102:
{ yyval.t = AtomNil; ;
    break;}
case 103:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 104:
{ yyval.t = AtomNil; ;
    break;}
case 105:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 106:
{ yyval.t = oz_cons(newCTerm(PA_fRequire,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 107:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 108:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-1].t,
                                           newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 109:
{ yyval.t = oz_cons(newCTerm(PA_fImport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 110:
{ yyval.t = oz_cons(newCTerm(PA_fExport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 111:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 112:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-1].t,
                                           newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 113:
{ yyval.t = AtomNil; ;
    break;}
case 114:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-2].t,AtomNil,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 115:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 116:
{ yyval.t = newCTerm(PA_fVar,OZ_atom(xytext),yyvsp[0].t); ;
    break;}
case 117:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 118:
{ yyval.t = oz_mklist(oz_pair2(yyvsp[0].t,yyvsp[-2].t)); ;
    break;}
case 119:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 120:
{ yyval.t = oz_cons(oz_pair2(yyvsp[-1].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 121:
{ yyval.t = PA_fNoImportAt; ;
    break;}
case 122:
{ yyval.t = newCTerm(PA_fImportAt,yyvsp[0].t); ;
    break;}
case 123:
{ yyval.t = AtomNil; ;
    break;}
case 124:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 125:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,
                                           newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t)),yyvsp[0].t); ;
    break;}
case 126:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 127:
{ yyval.t = OZ_atom(xytext); ;
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
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 133:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 134:
{ yyval.t = AtomNil; ;
    break;}
case 135:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 136:
{ yyval.t = newCTerm(PA_fAtom,AtomNil,yyvsp[0].t); ;
    break;}
case 137:
{ yyval.t = newCTerm(PA_fRecord,
                                  newCTerm(PA_fAtom,AtomCons,yyvsp[-2].t),
                                  oz_mklist(yyvsp[-1].t,yyvsp[0].t)); ;
    break;}
case 138:
{ yyval.t = PA_fNoCatch; ;
    break;}
case 139:
{ yyval.t = newCTerm(PA_fCatch,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 140:
{ yyval.t = PA_fNoFinally; ;
    break;}
case 141:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 142:
{
                    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
                                  newCTerm(PA_fAtom,yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
                  ;
    break;}
case 143:
{
                    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
                                  makeVar(yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
                  ;
    break;}
case 144:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 145:
{ yyval.t = NameUnit; ;
    break;}
case 146:
{ yyval.t = NameTrue; ;
    break;}
case 147:
{ yyval.t = NameFalse; ;
    break;}
case 148:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 149:
{ yyval.t = AtomNil; ;
    break;}
case 150:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 151:
{ yyval.t = oz_cons(newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 152:
{ yyval.t = NameFalse; ;
    break;}
case 153:
{ yyval.t = NameTrue; ;
    break;}
case 154:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 155:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 156:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 157:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 158:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 159:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 160:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 161:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 162:
{ yyval.t = newCTerm(PA_fBoolCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 163:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 164:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 165:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 166:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 167:
{ checkDeprecation(yyvsp[-3].t);
                    yyval.t = newCTerm(PA_fBoolCase,yyvsp[-5].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-6].t,yyvsp[0].t));
                  ;
    break;}
case 168:
{ yyval.t = newCTerm(PA_fCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 169:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 170:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 171:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 172:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 173:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 174:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 175:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 176:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 177:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 178:
{ yyval.t = newCTerm(PA_fCaseClause,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 179:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 180:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-3].t,
                                  newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 181:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-5].t,yyvsp[-2].t,yyvsp[0].t,yyvsp[-3].t); ;
    break;}
case 182:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 183:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 184:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 185:
{ yyval.t = newCTerm(PA_fRecord,
                                  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
                                  oz_cons(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 186:
{ yyval.t = newCTerm(PA_fClass,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 187:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 188:
{ yyval.t = newCTerm(PA_fDollar,yyvsp[0].t); ;
    break;}
case 189:
{ yyval.t = AtomNil; ;
    break;}
case 190:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 191:
{ yyval.t = newCTerm(PA_fFrom,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 192:
{ yyval.t = newCTerm(PA_fAttr,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 193:
{ yyval.t = newCTerm(PA_fFeat,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 194:
{ yyval.t = newCTerm(PA_fProp,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 195:
{ yyval.t = AtomNil; ;
    break;}
case 196:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 197:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 198:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 199:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 200:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 201:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 202:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 203:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 204:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 205:
{ yyval.t = AtomNil; ;
    break;}
case 206:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 207:
{ yyval.t = newCTerm(PA_fMeth,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-3].t); ;
    break;}
case 208:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 209:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 210:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 211:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 212:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 213:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 214:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 215:
{ yyval.t = newCTerm(PA_fRecord,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 216:
{ yyval.t = newCTerm(PA_fOpenRecord,yyvsp[-4].t,yyvsp[-2].t); ;
    break;}
case 217:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 218:
{ yyval.t = makeVar(xytext); ;
    break;}
case 219:
{ yyval.t = newCTerm(PA_fEscape,makeVar(xytext),yyvsp[-1].t); ;
    break;}
case 220:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 221:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 222:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 223:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 224:
{ yyval.t = AtomNil; ;
    break;}
case 225:
{ yyval.t = newCTerm(PA_fMethArg,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 226:
{ yyval.t = newCTerm(PA_fMethColonArg,yyvsp[-3].t,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 227:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 228:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 229:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 230:
{ yyval.t = newCTerm(PA_fDefault,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 231:
{ yyval.t = PA_fNoDefault; ;
    break;}
case 232:
{ yyval.t = newCTerm(PA_fCond,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 233:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 234:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 235:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 236:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 237:
{ yyval.t = newCTerm(PA_fClause,newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 238:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 239:
{ yyval.t = oz_mklist(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 240:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 241:
{ yyval.t = newCTerm(PA_fClause,
                                  newCTerm(PA_fSkip,yyvsp[0].t),
                                  yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 242:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-3].t,yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 243:
{ yyval.t = newCTerm(PA_fClause,
                                  newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 244:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 245:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 246:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 247:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 248:
{ yyval.t = makeVar(xytext); ;
    break;}
case 249:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 250:
{ yyval.t = newCTerm(PA_fEscape,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 251:
{ yyval.t = makeString(xytext,pos()); ;
    break;}
case 252:
{ yyval.t = makeInt(xytext,pos()); ;
    break;}
case 253:
{ yyval.t = makeInt(xytext[0],pos()); ;
    break;}
case 254:
{ yyval.t = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); ;
    break;}
case 255:
{ yyval.t = pos(); ;
    break;}
case 256:
{ yyval.t = pos(); ;
    break;}
case 257:
{ OZ_Term prefix =
                      scannerPrefix? scannerPrefix: PA_zy;
                    yyval.t = newCTerm(PA_fScanner,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,prefix,
                                  makeLongPos(yyvsp[-6].t,yyvsp[0].t)); ;
    break;}
case 258:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 259:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 260:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 261:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 262:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 263:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 264:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 265:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 266:
{ yyval.t = newCTerm(PA_fLexicalRule,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 267:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 268:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 269:
{ yyval.t = newCTerm(PA_fMode,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 270:
{ yyval.t = AtomNil; ;
    break;}
case 271:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 272:
{ yyval.t = newCTerm(PA_fInheritedModes,yyvsp[0].t); ;
    break;}
case 273:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 274:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 275:
{ OZ_Term expect = parserExpect? parserExpect: makeTaggedSmallInt(0);
                    yyval.t = newCTerm(PA_fParser,yyvsp[-6].t,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,expect,
                                  makeLongPos(yyvsp[-7].t,yyvsp[0].t)); ;
    break;}
case 276:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 277:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 278:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 279:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 280:
{ yyval.t = newCTerm(PA_fToken,AtomNil); ;
    break;}
case 281:
{ yyval.t = newCTerm(PA_fToken,yyvsp[0].t); ;
    break;}
case 282:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 283:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 284:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 285:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 286:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 287:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 288:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 289:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 290:
{ *prodKey[depth]++ = '='; ;
    break;}
case 291:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 292:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 293:
{ *prodKey[depth]++ = '='; ;
    break;}
case 294:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 295:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,PA_none); ;
    break;}
case 296:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 297:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 298:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 301:
{ prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg(yyvsp[-1].t,0))); ;
    break;}
case 302:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 303:
{ depth--; ;
    break;}
case 304:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 305:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 306:
{ depth--; ;
    break;}
case 307:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 308:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 309:
{ depth--; ;
    break;}
case 310:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 311:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 312:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 313:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 314:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 315:
{ *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; ;
    break;}
case 318:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 319:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 320:
{ *prodKey[depth] = '\0';
                    yyval.t = oz_pair2(prodName[depth],OZ_string(prodKeyBuffer[depth]));
                    prodName[depth] = PA_none;
                    prodKey[depth] = prodKeyBuffer[depth];
                  ;
    break;}
case 321:
{ yyval.t = AtomNil; ;
    break;}
case 322:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 323:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 324:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 325:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 326:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 327:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 328:
{ yyval.t = AtomNil; ;
    break;}
case 329:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 330:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 331:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 332:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 333:
{ yyval.t = newCTerm(PA_fSynAlternative, yyvsp[0].t); ;
    break;}
case 334:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 335:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 336:
{ OZ_Term t = yyvsp[0].t;
                    while (terms[depth]) {
                      t = oz_cons(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
                      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
                    }
                    yyval.t = newCTerm(PA_fSynSequence, decls[depth], t, yyvsp[-1].t);
                    decls[depth] = AtomNil;
                  ;
    break;}
case 337:
{ yyval.t = newCTerm(PA_fSynSequence, AtomNil, yyvsp[0].t, yyvsp[-1].t); ;
    break;}
case 338:
{ yyval.t = AtomNil; ;
    break;}
case 339:
{ yyval.t = oz_mklist(newCTerm(PA_fSynAction,yyvsp[0].t)); ;
    break;}
case 340:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 341:
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
case 342:
{ yyval.t = oz_cons(newCTerm(PA_fSynAssignment, terms[depth]->term, yyvsp[-1].t),
                                  yyvsp[0].t);
                    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
                  ;
    break;}
case 343:
{ while (terms[depth]) {
                      decls[depth] = oz_cons(terms[depth]->term, decls[depth]);
                      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
                    }
                    yyval.t = yyvsp[0].t;
                  ;
    break;}
case 344:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 345:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 346:
{ terms[depth] = new TermNode(yyvsp[0].t, terms[depth]); ;
    break;}
case 347:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 348:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 349:
{ yyval.t = newCTerm(PA_fSynAssignment,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 350:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 351:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 352:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
                                  oz_cons(newCTerm(PA_fSynApplication,yyvsp[-3].t,
                                                    AtomNil),
                                           AtomNil),yyvsp[-1].t);
                  ;
    break;}
case 353:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 354:
{ yyval.t = newCTerm(PA_fSynAssignment,
                                  newCTerm(PA_fEscape,yyvsp[-2].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 355:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 356:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 357:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
                                  oz_mklist(yyvsp[-2].t),yyvsp[-3].t);
                  ;
    break;}
case 358:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
                                  oz_mklist(yyvsp[-3].t),yyvsp[-1].t);
                  ;
    break;}
case 359:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 360:
{ depth--; ;
    break;}
case 361:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 362:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 363:
{ depth--; ;
    break;}
case 364:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 365:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 366:
{ depth--; ;
    break;}
case 367:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 368:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 369:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[-4].t,yyvsp[-1].t); ;
    break;}
case 370:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 371:
{ yyval.t = makeVar(xytext); ;
    break;}
case 372:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 373:
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
