
/*  A Bison parser, made from /lhome/denys/mozart/platform/emulator/parser.yy
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse xyparse
#define yylex xylex
#define yyerror xyerror
#define yylval xylval
#define yychar xychar
#define yydebug xydebug
#define yynerrs xynerrs
#define T_SWITCH        258
#define T_SWITCHNAME    259
#define T_LOCALSWITCHES 260
#define T_PUSHSWITCHES  261
#define T_POPSWITCHES   262
#define T_OZATOM        263
#define T_ATOM_LABEL    264
#define T_OZFLOAT       265
#define T_OZINT 266
#define T_AMPER 267
#define T_DOTINT        268
#define T_STRING        269
#define T_VARIABLE      270
#define T_VARIABLE_LABEL        271
#define T_DEFAULT       272
#define T_CHOICE        273
#define T_LDOTS 274
#define T_2DOTS 275
#define T_attr  276
#define T_at    277
#define T_case  278
#define T_catch 279
#define T_choice        280
#define T_class 281
#define T_cond  282
#define T_declare       283
#define T_define        284
#define T_dis   285
#define T_else  286
#define T_elsecase      287
#define T_elseif        288
#define T_elseof        289
#define T_end   290
#define T_export        291
#define T_fail  292
#define T_false 293
#define T_FALSE_LABEL   294
#define T_feat  295
#define T_finally       296
#define T_from  297
#define T_fun   298
#define T_functor       299
#define T_if    300
#define T_import        301
#define T_in    302
#define T_local 303
#define T_lock  304
#define T_meth  305
#define T_not   306
#define T_of    307
#define T_or    308
#define T_prepare       309
#define T_proc  310
#define T_prop  311
#define T_raise 312
#define T_require       313
#define T_self  314
#define T_skip  315
#define T_then  316
#define T_thread        317
#define T_true  318
#define T_TRUE_LABEL    319
#define T_try   320
#define T_unit  321
#define T_UNIT_LABEL    322
#define T_loop  323
#define T_ENDOFFILE     324
#define T_REGEX 325
#define T_lex   326
#define T_mode  327
#define T_parser        328
#define T_prod  329
#define T_scanner       330
#define T_syn   331
#define T_token 332
#define T_REDUCE        333
#define T_SEP   334
#define T_ITER  335
#define T_OOASSIGN      336
#define T_orelse        337
#define T_andthen       338
#define T_COMPARE       339
#define T_FDCOMPARE     340
#define T_LMACRO        341
#define T_RMACRO        342
#define T_FDIN  343
#define T_ADD   344
#define T_FDMUL 345
#define T_OTHERMUL      346
#define T_DEREFF        347


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



#define YYFINAL         768
#define YYFLAG          -32768
#define YYNTBASE        114

#define YYTRANSLATE(x) ((unsigned)(x) <= 347 ? yytranslate[x] : 251)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   113,     2,    91,   107,     2,     2,     2,   104,
   105,     2,   101,    95,   102,    97,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,   112,   103,     2,
    81,     2,     2,    99,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   108,     2,   109,    98,   106,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   110,    90,   111,    96,     2,     2,     2,     2,
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
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
    66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
    76,    77,    78,    79,    80,    82,    83,    84,    85,    86,
    87,    88,    89,    92,    93,    94,   100
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     5,     8,    11,    13,    17,    20,    27,    34,
    40,    41,    44,    46,    48,    50,    51,    54,    57,    60,
    62,    65,    66,    69,    74,    79,    88,    95,   100,   105,
   110,   115,   120,   125,   127,   132,   134,   138,   143,   148,
   153,   158,   162,   167,   170,   175,   179,   183,   187,   189,
   191,   193,   195,   197,   199,   201,   203,   205,   207,   209,
   211,   218,   225,   236,   247,   254,   256,   263,   266,   269,
   275,   283,   289,   297,   303,   305,   307,   313,   316,   322,
   328,   334,   336,   338,   344,   350,   351,   354,   355,   357,
   362,   369,   374,   379,   384,   391,   396,   397,   401,   408,
   411,   413,   417,   420,   425,   426,   429,   430,   433,   438,
   440,   442,   444,   446,   448,   450,   455,   457,   458,   461,
   463,   467,   468,   472,   473,   476,   484,   492,   494,   496,
   498,   500,   502,   503,   506,   511,   512,   514,   516,   518,
   520,   522,   524,   526,   528,   530,   537,   540,   543,   547,
   549,   557,   564,   567,   570,   574,   576,   578,   582,   586,
   588,   592,   596,   598,   603,   610,   615,   620,   622,   627,
   635,   637,   639,   640,   643,   648,   653,   658,   663,   664,
   667,   671,   673,   675,   677,   679,   681,   683,   685,   686,
   689,   695,   697,   702,   704,   706,   708,   710,   712,   717,
   723,   725,   727,   731,   733,   735,   737,   740,   741,   744,
   749,   751,   753,   755,   759,   760,   766,   769,   770,   772,
   776,   781,   787,   791,   795,   798,   803,   808,   814,   816,
   820,   822,   824,   826,   830,   832,   834,   836,   838,   839,
   840,   849,   851,   853,   855,   858,   861,   864,   870,   876,
   881,   883,   885,   890,   891,   894,   897,   899,   901,   911,
   913,   915,   918,   921,   922,   925,   927,   930,   932,   936,
   938,   941,   943,   946,   947,   957,   958,   959,   970,   977,
   981,   984,   987,   989,   990,   993,   994,   995,  1002,  1003,
  1004,  1011,  1012,  1013,  1020,  1022,  1026,  1028,  1030,  1032,
  1033,  1035,  1037,  1039,  1040,  1041,  1044,  1046,  1049,  1054,
  1059,  1067,  1068,  1071,  1073,  1075,  1077,  1079,  1081,  1085,
  1088,  1092,  1093,  1096,  1099,  1105,  1110,  1113,  1116,  1118,
  1120,  1122,  1125,  1129,  1131,  1133,  1138,  1140,  1146,  1148,
  1150,  1156,  1161,  1162,  1163,  1173,  1174,  1175,  1185,  1186,
  1187,  1197,  1199,  1205,  1207,  1209,  1211
};

static const short yyrhs[] = {   115,
    69,     0,     1,     0,   120,   116,     0,   205,   116,     0,
   116,     0,   189,   127,   116,     0,   117,   115,     0,    28,
   190,   120,    47,   189,   116,     0,    28,   190,   120,    47,
   120,   116,     0,    28,   190,   120,   189,   116,     0,     0,
     3,   118,     0,     5,     0,     6,     0,     7,     0,     0,
   119,   118,     0,   101,     4,     0,   102,     4,     0,   122,
     0,   122,   120,     0,     0,   103,   124,     0,   122,    81,
   190,   122,     0,   122,    82,   190,   122,     0,   122,    80,
   190,   124,    20,   124,   121,   190,     0,   122,    80,   190,
   124,   121,   190,     0,   122,    83,   190,   122,     0,   122,
    84,   190,   122,     0,   122,   133,   190,   122,     0,   122,
   134,   190,   122,     0,   122,   135,   190,   122,     0,   122,
    90,   190,   122,     0,   124,     0,   124,    91,   190,   123,
     0,   124,     0,   124,    91,   123,     0,   124,   136,   190,
   124,     0,   124,   137,   190,   124,     0,   124,   138,   190,
   124,     0,   124,    95,   190,   124,     0,    96,   190,   124,
     0,   124,    97,   190,   124,     0,   124,    13,     0,   124,
    98,   190,   124,     0,    99,   190,   124,     0,   100,   190,
   124,     0,   104,   139,   105,     0,   183,     0,   185,     0,
   106,     0,    66,     0,    63,     0,    38,     0,    59,     0,
   107,     0,   186,     0,   187,     0,   188,     0,   144,     0,
   108,   190,   122,   141,   109,   190,     0,   110,   190,   122,
   140,   111,   190,     0,    55,   190,   125,   110,   122,   140,
   111,   139,    35,   190,     0,    43,   190,   125,   110,   122,
   140,   111,   139,    35,   190,     0,    44,   190,   161,   126,
    35,   190,     0,   160,     0,    48,   190,   120,    47,   120,
    35,     0,    45,   151,     0,    23,   153,     0,    49,   190,
   139,    35,   190,     0,    49,   190,   122,    61,   139,    35,
   190,     0,    62,   190,   139,    35,   190,     0,    65,   190,
   139,   142,   143,    35,   190,     0,    57,   190,   139,    35,
   190,     0,    60,     0,    37,     0,    51,   190,   139,    35,
   190,     0,    27,   176,     0,    53,   190,   180,    35,   190,
     0,    30,   190,   180,    35,   190,     0,    25,   190,   182,
    35,   190,     0,   191,     0,   199,     0,    68,   190,   139,
    35,   190,     0,    87,   190,   140,    88,   190,     0,     0,
   183,   125,     0,     0,   127,     0,    58,   190,   128,   126,
     0,    54,   190,   120,    47,   120,   126,     0,    54,   190,
   120,   126,     0,    46,   190,   128,   126,     0,    36,   190,
   132,   126,     0,    29,   190,   120,    47,   120,   126,     0,
    29,   190,   120,   126,     0,     0,   184,   131,   128,     0,
   129,   104,   130,   105,   131,   128,     0,    16,   190,     0,
   150,     0,   150,   112,   184,     0,   150,   130,     0,   150,
   112,   184,   130,     0,     0,    22,   183,     0,     0,   184,
   132,     0,   150,   112,   184,   132,     0,    85,     0,    86,
     0,    89,     0,    92,     0,    93,     0,    94,     0,   120,
    47,   190,   120,     0,   120,     0,     0,   122,   140,     0,
   189,     0,   189,   122,   141,     0,     0,    24,   190,   156,
     0,     0,    41,   139,     0,   145,   190,   104,   147,   148,
   105,   190,     0,   146,   190,   104,   147,   148,   105,   190,
     0,     9,     0,    67,     0,    64,     0,    39,     0,    16,
     0,     0,   122,   147,     0,   149,   112,   122,   147,     0,
     0,    19,     0,   183,     0,   184,     0,   187,     0,    66,
     0,    63,     0,    38,     0,   183,     0,   187,     0,   190,
   120,    61,   139,   152,   190,     0,    33,   151,     0,    32,
   153,     0,    31,   139,    35,     0,    35,     0,   190,   120,
    61,   190,   139,   154,   190,     0,   190,   120,    52,   155,
   154,   190,     0,    33,   151,     0,    32,   153,     0,    31,
   139,    35,     0,    35,     0,   157,     0,   157,    18,   155,
     0,   157,    34,   155,     0,   157,     0,   157,    18,   156,
     0,   158,    61,   139,     0,   159,     0,   159,    84,   189,
   120,     0,   159,    84,   189,   120,    47,   120,     0,   159,
    81,   190,   159,     0,   159,    90,   190,   159,     0,   124,
     0,   124,    91,   190,   123,     0,    26,   190,   161,   162,
   167,    35,   190,     0,   122,     0,   189,     0,     0,   163,
   162,     0,    42,   190,   122,   140,     0,    21,   190,   165,
   164,     0,    40,   190,   165,   164,     0,    56,   190,   122,
   140,     0,     0,   165,   164,     0,   166,   112,   122,     0,
   166,     0,   183,     0,   185,     0,   187,     0,    66,     0,
    63,     0,    38,     0,     0,   168,   167,     0,    50,   190,
   169,   139,    35,     0,   170,     0,   170,    81,   190,   184,
     0,   183,     0,   185,     0,    66,     0,    63,     0,    38,
     0,   171,   104,   172,   105,     0,   171,   104,   172,    19,
   105,     0,     9,     0,    16,     0,   113,   190,    16,     0,
    67,     0,    64,     0,    39,     0,   173,   172,     0,     0,
   174,   175,     0,   149,   112,   174,   175,     0,   184,     0,
   107,     0,   106,     0,    17,   190,   122,     0,     0,   190,
   178,   177,    35,   190,     0,    31,   139,     0,     0,   179,
     0,   179,    18,   178,     0,   120,    61,   190,   139,     0,
   120,    47,   120,    61,   139,     0,   181,    18,   181,     0,
   181,    18,   180,     0,   120,   189,     0,   120,    47,   120,
   189,     0,   120,   189,    61,   139,     0,   120,    47,   120,
    61,   139,     0,   139,     0,   139,    18,   182,     0,     8,
     0,    15,     0,   184,     0,   113,   190,   184,     0,    14,
     0,    11,     0,    12,     0,    10,     0,     0,     0,    75,
   190,   184,   162,   167,   192,    35,   190,     0,   193,     0,
   194,     0,   196,     0,   193,   192,     0,   194,   192,     0,
   196,   192,     0,    71,   183,    81,   195,    35,     0,    71,
   184,    81,   195,    35,     0,    71,   195,   139,    35,     0,
    70,     0,    14,     0,    72,   184,   197,    35,     0,     0,
   198,   197,     0,    42,   204,     0,   194,     0,   196,     0,
    73,   190,   184,   162,   167,   201,   200,    35,   190,     0,
   228,     0,   206,     0,   228,   200,     0,   206,   200,     0,
     0,    77,   202,     0,   203,     0,   203,   202,     0,   183,
     0,   183,   112,   122,     0,   184,     0,   184,   204,     0,
   206,     0,   206,   205,     0,     0,    74,   184,    81,   207,
   210,   225,   226,   231,    35,     0,     0,     0,    74,   107,
   208,    81,   209,   210,   225,   226,   231,    35,     0,    74,
   210,   225,   226,   231,    35,     0,   212,   184,   223,     0,
   184,   224,     0,   211,   213,     0,   212,     0,     0,   183,
   112,     0,     0,     0,   104,   214,   220,   105,   215,   223,
     0,     0,     0,   108,   216,   220,   109,   217,   223,     0,
     0,     0,   110,   218,   220,   111,   219,   223,     0,   221,
     0,   221,   222,   220,     0,   184,     0,   106,     0,    79,
     0,     0,   224,     0,    92,     0,    93,     0,     0,     0,
   227,    47,     0,   228,     0,   228,   227,     0,    76,   183,
   231,    35,     0,    76,   184,   231,    35,     0,    76,   249,
   104,   229,   105,   231,    35,     0,     0,   230,   229,     0,
   184,     0,   107,     0,   106,     0,   232,     0,   233,     0,
   233,    18,   232,     0,   189,   235,     0,    60,   190,   234,
     0,     0,    78,   139,     0,   236,   235,     0,   236,   224,
   190,   237,   225,     0,   236,    81,   239,   237,     0,    47,
   237,     0,   240,   237,     0,   234,     0,   184,     0,   234,
     0,   238,   237,     0,   185,    81,   239,     0,   239,     0,
   184,     0,   184,   224,   190,   225,     0,   241,     0,   113,
   190,   184,    81,   239,     0,   241,     0,   248,     0,   212,
   190,   248,   223,   225,     0,   248,   224,   190,   225,     0,
     0,     0,   211,   190,   104,   242,   250,   105,   243,   223,
   225,     0,     0,     0,   211,   190,   108,   244,   250,   109,
   245,   223,   225,     0,     0,     0,   211,   190,   110,   246,
   250,   111,   247,   223,   225,     0,   183,     0,   249,   190,
   104,   140,   105,     0,     9,     0,    16,     0,   231,     0,
   231,   222,   250,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   674,   676,   680,   682,   685,   687,   692,   694,   697,   699,
   702,   706,   708,   710,   712,   716,   718,   722,   729,   738,
   740,   744,   745,   749,   751,   753,   770,   792,   794,   796,
   799,   801,   803,   805,   807,   813,   815,   819,   822,   825,
   828,   830,   833,   836,   839,   842,   844,   847,   849,   851,
   853,   855,   857,   859,   861,   863,   865,   867,   869,   871,
   873,   877,   879,   882,   885,   887,   889,   891,   893,   895,
   897,   899,   901,   903,   905,   907,   909,   911,   913,   915,
   917,   919,   921,   923,   925,   929,   931,   936,   938,   943,
   945,   947,   950,   952,   954,   956,   961,   963,   965,   969,
   973,   975,   977,   979,   983,   985,   989,   991,   993,   998,
  1002,  1006,  1010,  1014,  1018,  1022,  1024,  1028,  1030,  1034,
  1036,  1042,  1044,  1048,  1050,  1054,  1059,  1066,  1068,  1070,
  1072,  1076,  1080,  1082,  1084,  1088,  1090,  1094,  1096,  1098,
  1100,  1102,  1104,  1108,  1110,  1114,  1118,  1120,  1122,  1124,
  1128,  1132,  1136,  1138,  1140,  1142,  1146,  1148,  1150,  1154,
  1156,  1160,  1164,  1166,  1169,  1173,  1175,  1177,  1179,  1185,
  1190,  1192,  1197,  1199,  1203,  1205,  1207,  1209,  1213,  1215,
  1219,  1221,  1225,  1227,  1229,  1231,  1233,  1235,  1239,  1241,
  1245,  1249,  1251,  1255,  1257,  1259,  1261,  1263,  1265,  1267,
  1271,  1273,  1275,  1277,  1279,  1281,  1285,  1287,  1291,  1293,
  1297,  1299,  1301,  1306,  1308,  1312,  1316,  1318,  1322,  1324,
  1328,  1330,  1334,  1336,  1340,  1344,  1346,  1349,  1353,  1355,
  1359,  1363,  1367,  1369,  1373,  1377,  1379,  1383,  1387,  1391,
  1401,  1409,  1411,  1413,  1415,  1417,  1419,  1423,  1425,  1429,
  1433,  1435,  1439,  1443,  1445,  1449,  1451,  1453,  1459,  1467,
  1469,  1471,  1473,  1477,  1479,  1483,  1485,  1489,  1491,  1495,
  1497,  1501,  1503,  1507,  1509,  1511,  1512,  1513,  1515,  1519,
  1521,  1523,  1527,  1528,  1531,  1535,  1536,  1536,  1537,  1538,
  1538,  1539,  1540,  1540,  1543,  1545,  1549,  1550,  1553,  1557,
  1558,  1561,  1562,  1565,  1573,  1575,  1579,  1581,  1585,  1587,
  1589,  1593,  1595,  1599,  1601,  1603,  1607,  1611,  1613,  1617,
  1626,  1630,  1632,  1636,  1638,  1648,  1653,  1660,  1662,  1666,
  1670,  1672,  1676,  1678,  1682,  1684,  1690,  1694,  1697,  1702,
  1704,  1708,  1712,  1713,  1714,  1716,  1717,  1718,  1720,  1721,
  1722,  1726,  1728,  1732,  1734,  1739,  1741
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
"T_TRUE_LABEL","T_try","T_unit","T_UNIT_LABEL","T_loop","T_ENDOFFILE","T_REGEX",
"T_lex","T_mode","T_parser","T_prod","T_scanner","T_syn","T_token","T_REDUCE",
"T_SEP","T_ITER","'='","T_OOASSIGN","T_orelse","T_andthen","T_COMPARE","T_FDCOMPARE",
"T_LMACRO","T_RMACRO","T_FDIN","'|'","'#'","T_ADD","T_FDMUL","T_OTHERMUL","','",
"'~'","'.'","'^'","'@'","T_DEREFF","'+'","'-'","';'","'('","')'","'_'","'$'",
"'['","']'","'{'","'}'","':'","'!'","file","queries","queries1","directive",
"switchList","switch","sequence","optByPhrase","phrase","hashes","phrase2","procFlags",
"optFunctorDescriptorList","functorDescriptorList","importDecls","variableLabel",
"featureList","optImportAt","exportDecls","compare","fdCompare","fdIn","add",
"fdMul","otherMul","inSequence","phraseList","fixedListArgs","optCatch","optFinally",
"record","recordAtomLabel","recordVarLabel","recordArguments","optDots","feature",
"featureNoVar","ifMain","ifRest","caseMain","caseRest","elseOfList","caseClauseList",
"caseClause","sideCondition","pattern","class","phraseOpt","classDescriptorList",
"classDescriptor","attrFeatList","attrFeat","attrFeatFeature","methList","meth",
"methHead","methHead1","methHeadLabel","methFormals","methFormal","methFormalTerm",
"methFormalOptDefault","condMain","condElse","condClauseList","condClause","orClauseList",
"orClause","choiceClauseList","atom","nakedVariable","variable","string","int",
"float","thisCoord","coord","scannerSpecification","scannerRules","lexAbbrev",
"lexRule","regex","modeClause","modeDescrs","modeDescr","parserSpecification",
"parserRules","tokenClause","tokenList","tokenDecl","modeFromList","prodClauseList",
"prodClause","@1","@2","@3","prodHeadRest","prodName","prodNameAtom","prodKey",
"@4","@5","@6","@7","@8","@9","prodParams","prodParam","separatorOp","optTerminatorOp",
"terminatorOp","prodMakeKey","localRules","localRulesSub","synClause","synParams",
"synParam","synAlt","synSeqs","synSeq","optSynAction","nonEmptySeq","synVariable",
"synPrims","synPrim","synPrimNoAssign","synPrimNoVar","synPrimNoVarNoAssign",
"@10","@11","@12","@13","@14","@15","synInstTerm","synLabel","synProdCallParams", NULL
};
#endif

static const short yyr1[] = {     0,
   114,   114,   115,   115,   115,   115,   116,   116,   116,   116,
   116,   117,   117,   117,   117,   118,   118,   119,   119,   120,
   120,   121,   121,   122,   122,   122,   122,   122,   122,   122,
   122,   122,   122,   122,   122,   123,   123,   124,   124,   124,
   124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
   124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
   124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
   124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
   124,   124,   124,   124,   124,   125,   125,   126,   126,   127,
   127,   127,   127,   127,   127,   127,   128,   128,   128,   129,
   130,   130,   130,   130,   131,   131,   132,   132,   132,   133,
   134,   135,   136,   137,   138,   139,   139,   140,   140,   141,
   141,   142,   142,   143,   143,   144,   144,   145,   145,   145,
   145,   146,   147,   147,   147,   148,   148,   149,   149,   149,
   149,   149,   149,   150,   150,   151,   152,   152,   152,   152,
   153,   153,   154,   154,   154,   154,   155,   155,   155,   156,
   156,   157,   158,   158,   158,   159,   159,   159,   159,   160,
   161,   161,   162,   162,   163,   163,   163,   163,   164,   164,
   165,   165,   166,   166,   166,   166,   166,   166,   167,   167,
   168,   169,   169,   170,   170,   170,   170,   170,   170,   170,
   171,   171,   171,   171,   171,   171,   172,   172,   173,   173,
   174,   174,   174,   175,   175,   176,   177,   177,   178,   178,
   179,   179,   180,   180,   181,   181,   181,   181,   182,   182,
   183,   184,   185,   185,   186,   187,   187,   188,   189,   190,
   191,   192,   192,   192,   192,   192,   192,   193,   193,   194,
   195,   195,   196,   197,   197,   198,   198,   198,   199,   200,
   200,   200,   200,   201,   201,   202,   202,   203,   203,   204,
   204,   205,   205,   207,   206,   208,   209,   206,   206,   210,
   210,   210,   211,   211,   212,   214,   215,   213,   216,   217,
   213,   218,   219,   213,   220,   220,   221,   221,   222,   223,
   223,   224,   224,   225,   226,   226,   227,   227,   228,   228,
   228,   229,   229,   230,   230,   230,   231,   232,   232,   233,
   233,   234,   234,   235,   235,   235,   235,   235,   235,   236,
   237,   237,   238,   238,   239,   239,   239,   240,   240,   241,
   241,   241,   242,   243,   241,   244,   245,   241,   246,   247,
   241,   248,   248,   249,   249,   250,   250
};

static const short yyr2[] = {     0,
     2,     1,     2,     2,     1,     3,     2,     6,     6,     5,
     0,     2,     1,     1,     1,     0,     2,     2,     2,     1,
     2,     0,     2,     4,     4,     8,     6,     4,     4,     4,
     4,     4,     4,     1,     4,     1,     3,     4,     4,     4,
     4,     3,     4,     2,     4,     3,     3,     3,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     6,     6,    10,    10,     6,     1,     6,     2,     2,     5,
     7,     5,     7,     5,     1,     1,     5,     2,     5,     5,
     5,     1,     1,     5,     5,     0,     2,     0,     1,     4,
     6,     4,     4,     4,     6,     4,     0,     3,     6,     2,
     1,     3,     2,     4,     0,     2,     0,     2,     4,     1,
     1,     1,     1,     1,     1,     4,     1,     0,     2,     1,
     3,     0,     3,     0,     2,     7,     7,     1,     1,     1,
     1,     1,     0,     2,     4,     0,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     6,     2,     2,     3,     1,
     7,     6,     2,     2,     3,     1,     1,     3,     3,     1,
     3,     3,     1,     4,     6,     4,     4,     1,     4,     7,
     1,     1,     0,     2,     4,     4,     4,     4,     0,     2,
     3,     1,     1,     1,     1,     1,     1,     1,     0,     2,
     5,     1,     4,     1,     1,     1,     1,     1,     4,     5,
     1,     1,     3,     1,     1,     1,     2,     0,     2,     4,
     1,     1,     1,     3,     0,     5,     2,     0,     1,     3,
     4,     5,     3,     3,     2,     4,     4,     5,     1,     3,
     1,     1,     1,     3,     1,     1,     1,     1,     0,     0,
     8,     1,     1,     1,     2,     2,     2,     5,     5,     4,
     1,     1,     4,     0,     2,     2,     1,     1,     9,     1,
     1,     2,     2,     0,     2,     1,     2,     1,     3,     1,
     2,     1,     2,     0,     9,     0,     0,    10,     6,     3,
     2,     2,     1,     0,     2,     0,     0,     6,     0,     0,
     6,     0,     0,     6,     1,     3,     1,     1,     1,     0,
     1,     1,     1,     0,     0,     2,     1,     2,     4,     4,
     7,     0,     2,     1,     1,     1,     1,     1,     3,     2,
     3,     0,     2,     2,     5,     4,     2,     2,     1,     1,
     1,     2,     3,     1,     1,     4,     1,     5,     1,     1,
     5,     4,     0,     0,     9,     0,     0,     9,     0,     0,
     9,     1,     5,     1,     1,     1,     3
};

static const short yydefact[] = {     0,
     2,    16,    13,    14,    15,   231,   128,   238,   236,   237,
   235,   232,   132,   240,   240,   240,   240,   240,   240,    76,
    54,   131,   240,   240,   240,   240,   240,   240,   240,   240,
   240,    55,    75,   240,    53,   130,   240,    52,   129,   240,
   240,   284,   240,   240,   240,   240,   240,     0,    51,    56,
   240,   240,   240,     0,     5,   239,    11,    20,    34,    60,
   240,   240,    66,    49,   233,    50,    57,    58,    59,     0,
    82,    83,    11,   272,     0,     0,    12,    16,    69,     0,
     0,   239,    78,     0,     0,     0,    86,   239,    68,     0,
     0,     0,     0,     0,    86,     0,     0,     0,     0,     0,
   276,     0,     0,   304,     0,   283,     0,   118,     0,     0,
     0,   117,     0,     0,     0,     0,     1,     7,     3,   240,
   240,   240,   240,   240,   110,   111,   112,   240,    21,   240,
   240,   240,    44,   240,   113,   114,   115,   240,   240,   240,
   240,   240,   240,     0,     0,   240,   240,   240,   240,   240,
    11,     4,   273,    18,    19,    17,     0,   229,     0,   171,
   173,   172,     0,   218,   219,   239,   239,     0,     0,     0,
    86,    88,     0,     0,    20,     0,     0,     0,     0,     0,
     0,   122,     0,   173,     0,   285,   274,   302,   303,   281,
   305,   286,   289,   292,   282,   300,   173,   118,     0,    42,
    46,    47,   240,    48,   239,   118,   234,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   133,   133,     0,   107,    97,     0,    97,
     6,     0,   240,     0,   240,   240,   240,   240,   240,   189,
   173,     0,   240,     0,     0,     0,   239,    11,     0,   225,
   240,     0,     0,    87,     0,    89,     0,     0,     0,   240,
   240,   240,     0,   240,   240,   240,   124,   240,   189,   277,
   284,     0,   239,     0,   307,     0,     0,     0,   280,   301,
   189,   119,   240,     0,     0,   120,     0,    22,    24,    25,
    28,    29,    33,    30,    31,    32,    35,    36,    41,    43,
    45,    38,    39,    40,    54,    53,    52,   133,   136,     0,
    49,   233,    58,   136,    88,    88,     0,   144,   107,   145,
   240,    88,     0,   105,    88,    88,   168,     0,   157,     0,
   163,     0,   230,    81,     0,     0,     0,     0,   240,     0,
   189,   174,     0,     0,   217,   240,   220,    11,    11,    10,
   239,     0,    80,   224,   223,   118,   240,     0,     0,     0,
    70,    77,    79,   118,    74,    72,     0,     0,     0,    84,
   264,   284,     0,   304,   354,   355,   239,   239,     0,   240,
   322,     0,   317,   318,   306,   308,   298,   297,     0,   295,
     0,     0,     0,    85,   116,   240,   239,   240,     0,     0,
   240,     0,   134,   137,     0,     0,     0,     0,    96,    94,
     0,   108,   100,    93,     0,     0,    97,     0,    92,    90,
   240,     0,   240,   240,   156,   240,     0,     0,     0,   240,
   239,   240,     0,   188,   187,   186,   179,   182,   183,   184,
   185,   179,   118,   118,     0,   240,   190,     0,   221,   216,
     9,     8,     0,   226,   227,     0,    65,     0,   240,   240,
   150,   240,    67,   240,     0,   123,   160,   125,   240,     0,
     0,   304,   305,     0,     0,   312,   322,   322,     0,   240,
   352,   330,   240,   240,   329,   320,   322,   322,   339,   340,
   240,   279,   239,   287,   299,     0,   290,   293,     0,     0,
     0,   242,   243,   244,    61,   121,    62,    22,    23,    27,
    37,   240,   133,   240,    88,   107,     0,   101,   106,    98,
    88,     0,     0,   154,   153,   152,   158,   159,   162,     0,
     0,     0,   240,   176,   179,     0,   177,   175,   178,   201,
   202,   198,   206,   197,   205,   196,   204,   240,     0,   192,
     0,   194,   195,   170,   222,   228,     0,     0,   148,   147,
   146,    71,     0,     0,    73,   268,   265,   266,     0,   261,
   260,   305,   239,   309,   310,   316,   315,   314,     0,   312,
   321,   335,     0,   331,   327,   322,   334,   337,   323,     0,
     0,     0,   284,   240,   324,   328,   240,     0,   319,   300,
   296,   300,   300,   252,   251,     0,     0,     0,   254,   240,
   245,   246,   247,   240,   126,   135,   127,    95,   109,   105,
     0,   103,    91,   169,   155,   166,   164,   167,   151,   180,
   181,     0,     0,   240,   208,     0,   149,     0,   161,     0,
   267,   240,   263,   262,   239,     0,   239,   313,   240,   284,
   332,     0,   343,   346,   349,   352,   300,   335,   322,   322,
   304,   118,   288,   291,   294,     0,     0,     0,     0,     0,
   257,   258,     0,   254,   241,    26,    97,   102,     0,   203,
   191,     0,   143,   142,   141,   213,   212,     0,     0,   208,
   215,   138,   211,   140,   240,   240,   269,   259,     0,   275,
     0,   304,   333,   284,   239,   239,   239,   304,   326,   304,
   342,     0,     0,     0,   250,   270,   256,   253,   255,    99,
   104,   165,   193,     0,     0,   199,   207,   240,   209,    64,
    63,   278,   311,   336,   338,   356,     0,     0,     0,   341,
   325,   353,   248,   249,   271,   215,   211,   200,     0,   239,
   344,   347,   350,   210,   214,   357,   300,   300,   300,   304,
   304,   304,   345,   348,   351,     0,     0,     0
};

static const short yydefgoto[] = {   766,
    54,    55,    56,    77,    78,   112,   401,    58,   297,    59,
   170,   255,   256,   322,   323,   517,   417,   316,   130,   131,
   132,   141,   142,   143,   158,   199,   285,   267,   369,    60,
    61,    62,   309,   405,   310,   317,    89,   462,    79,   426,
   328,   466,   329,   330,   331,    63,   161,   240,   241,   534,
   535,   438,   340,   341,   549,   550,   551,   689,   690,   691,
   729,    83,   245,   164,   165,   168,   169,   159,    64,    65,
    66,    67,    68,    69,   381,    80,    71,   501,   502,   503,
   608,   504,   673,   674,    72,   569,   471,   567,   568,   717,
    73,    74,   271,   185,   372,   104,   483,   484,   195,   276,
   600,   277,   602,   278,   603,   389,   390,   496,   279,   280,
   191,   273,   274,   275,   579,   580,   736,   383,   384,   584,
   486,   487,   585,   586,   587,   488,   588,   705,   757,   706,
   758,   707,   759,   490,   491,   737
};

static const short yypact[] = {   981,
-32768,   228,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,    63,-32768,-32768,-32768,-32768,-32768,  1514,-32768,-32768,
-32768,-32768,-32768,    -7,-32768,  1090,   407,  1302,   763,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   289,
-32768,-32768,   407,    27,   110,   192,-32768,   228,-32768,  1514,
  1514,  1514,-32768,  1514,  1514,  1514,   219,  1514,-32768,  1514,
  1514,  1514,  1514,  1514,   219,  1514,  1514,  1514,  1514,   214,
-32768,   122,   149,-32768,   159,   214,   214,  1514,  1514,  1514,
  1514,   190,   166,  1514,  1514,   214,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   161,   178,-32768,-32768,-32768,-32768,-32768,
   407,-32768,-32768,-32768,-32768,-32768,    42,   276,   269,   720,
   198,-32768,    32,   279,   303,   312,   316,   291,   356,   277,
   219,   289,   332,   349,  1196,   367,   401,   404,   297,   406,
   409,   426,   418,   198,   381,-32768,-32768,-32768,-32768,-32768,
   396,-32768,-32768,-32768,-32768,   246,   198,  1302,   386,    45,
-32768,-32768,-32768,-32768,   720,  1302,-32768,  1514,  1514,  1514,
  1514,  1514,  1514,  1514,  1514,  1514,  1514,  1514,  1514,  1514,
  1514,  1514,  1514,  1620,  1620,  1514,    72,   342,  1514,   342,
-32768,  1514,-32768,  1514,-32768,-32768,-32768,-32768,-32768,   425,
   198,  1514,-32768,  1514,   459,  1514,  1514,   407,  1514,   434,
-32768,  1514,  1514,-32768,   463,-32768,  1514,  1514,  1514,-32768,
-32768,-32768,  1514,-32768,-32768,-32768,   464,-32768,   425,-32768,
   201,   336,   440,   457,   396,    55,    55,    55,-32768,-32768,
   425,-32768,-32768,  1514,   402,  1514,   410,   625,   793,   830,
   398,   451,   417,   474,   281,   281,-32768,   774,   160,-32768,
-32768,   181,   160,   160,   400,   405,   411,  1408,   505,   413,
   414,   427,   430,   505,   372,   289,   433,-32768,    72,-32768,
-32768,   289,   442,   507,   456,   289,   813,   589,    57,   489,
   233,  1514,-32768,-32768,   646,   646,  1514,  1514,-32768,   518,
   425,-32768,   504,  1514,-32768,-32768,-32768,   407,   407,-32768,
   509,  1514,-32768,-32768,   356,  1302,-32768,   633,   533,   536,
-32768,-32768,-32768,  1302,-32768,-32768,  1514,  1514,   538,-32768,
   497,   201,   246,-32768,-32768,-32768,   440,   440,   471,-32768,
   500,   541,-32768,   562,-32768,-32768,-32768,-32768,   476,   506,
   484,   472,   305,-32768,-32768,-32768,   720,-32768,  1514,  1514,
-32768,  1514,-32768,-32768,   491,  1514,   493,  1514,-32768,-32768,
   214,-32768,-32768,-32768,   308,   219,   342,  1514,-32768,-32768,
-32768,  1514,-32768,-32768,-32768,-32768,  1514,  1514,  1514,-32768,
-32768,-32768,   589,-32768,-32768,-32768,   646,   482,-32768,-32768,
-32768,   646,  1302,  1302,   632,-32768,-32768,  1514,-32768,-32768,
-32768,-32768,  1514,-32768,-32768,   495,-32768,  1514,-32768,-32768,
-32768,-32768,-32768,-32768,   496,-32768,   584,-32768,-32768,   219,
    12,-32768,   396,   580,   581,    80,   540,   685,  1514,-32768,
   122,-32768,-32768,   330,-32768,-32768,   739,   685,-32768,   246,
-32768,-32768,   440,-32768,-32768,    55,-32768,-32768,   218,   214,
   590,   305,   305,   305,-32768,-32768,-32768,   730,   366,-32768,
-32768,-32768,  1408,-32768,   289,    72,   521,    65,-32768,-32768,
   289,  1514,   593,-32768,-32768,-32768,-32768,-32768,-32768,  1514,
  1514,  1514,-32768,-32768,   646,  1514,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,  1514,   548,
   529,-32768,-32768,-32768,-32768,-32768,  1514,   600,-32768,-32768,
-32768,-32768,  1514,  1514,-32768,   525,-32768,   219,   604,    12,
    12,   396,   440,-32768,-32768,-32768,-32768,-32768,   539,    80,
-32768,   167,   569,-32768,-32768,   685,-32768,-32768,-32768,   214,
   338,   352,   336,-32768,-32768,-32768,-32768,   551,-32768,   246,
-32768,   246,   246,-32768,-32768,   588,   591,  1514,    18,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   507,
   214,-32768,-32768,-32768,-32768,   180,   615,   583,-32768,-32768,
   720,   383,   639,-32768,   546,   640,-32768,   643,-32768,  1514,
-32768,-32768,-32768,-32768,   440,   644,   440,-32768,-32768,   336,
-32768,   601,-32768,-32768,-32768,-32768,   246,   246,   685,   685,
-32768,  1514,-32768,-32768,-32768,    98,    98,   648,   214,    98,
-32768,-32768,   650,    18,-32768,-32768,   342,   308,  1514,-32768,
-32768,   214,-32768,-32768,-32768,-32768,-32768,   574,    49,   546,
   670,-32768,   427,-32768,-32768,-32768,   720,-32768,   654,-32768,
   655,-32768,-32768,   336,   440,   440,   440,-32768,-32768,-32768,
-32768,   587,   667,   668,-32768,   214,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,    84,   599,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   506,   602,   596,   597,-32768,
-32768,-32768,-32768,-32768,-32768,   670,-32768,-32768,  1514,   440,
-32768,-32768,-32768,-32768,   720,-32768,   246,   246,   246,-32768,
-32768,-32768,-32768,-32768,-32768,   714,   721,-32768
};

static const short yypgoto[] = {-32768,
   673,   -40,-32768,   652,-32768,   120,   216,    87,  -358,   369,
   -41,  -259,   656,  -225,-32768,  -494,   111,  -301,-32768,-32768,
-32768,-32768,-32768,-32768,    48,  -171,   337,-32768,-32768,-32768,
-32768,-32768,  -172,   421,  -553,  -365,  -339,-32768,  -234,   309,
   -43,   177,  -333,-32768,  -428,-32768,   662,  -123,-32768,  -383,
    85,-32768,  -159,-32768,-32768,-32768,-32768,    54,-32768,    28,
     5,-32768,-32768,   510,-32768,   -49,   501,   523,   295,    56,
  -271,-32768,   324,-32768,   266,   -15,-32768,  -213,-32768,  -517,
  -244,  -509,    88,-32768,-32768,   -81,-32768,   196,-32768,    51,
   684,  -420,-32768,-32768,-32768,  -223,   -36,   -19,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,  -257,-32768,    34,  -560,   -96,
  -305,  -434,   499,  -390,   195,-32768,  -218,   284,-32768,  -329,
   292,-32768,  -447,-32768,  -521,-32768,  -332,-32768,-32768,-32768,
-32768,-32768,-32768,   186,   512,  -463
};


#define YYLAST          1733


static const short yytable[] = {    81,
    82,    84,    85,    86,   326,   105,   190,    87,    88,    90,
    91,    92,    93,    94,    95,    96,   119,   412,    97,   391,
   392,    98,   106,   622,    99,   100,   282,   107,   108,   109,
   110,   111,   152,   467,   287,   114,   115,   116,   573,   663,
   596,   664,   665,   511,   178,   144,   145,   374,   489,   518,
   570,   485,   314,   179,   382,   409,   410,   133,   537,   669,
   269,   117,   414,   440,   440,   419,   420,   725,   473,    12,
     6,   659,     6,   281,   427,     9,    10,    12,   242,     6,
   571,   688,     9,    10,   525,    42,    12,   272,   670,   500,
   428,   671,   243,   232,    12,   113,   708,   103,    12,   672,
    42,   626,   233,   628,   208,   209,   210,   211,   212,   371,
   231,   604,   213,   154,   214,   215,   216,   342,   217,    57,
   560,   393,   218,   219,   220,   221,   222,   223,   703,   254,
   226,   227,   228,   229,   230,   403,   688,   645,   651,   176,
   177,   139,   140,   180,   181,   182,   183,   581,   472,   570,
   570,   630,   518,   726,   489,   184,   671,   485,   474,   475,
   387,   196,   197,   624,   672,   440,   572,   605,   160,   101,
   440,   207,   133,   553,   160,    57,   621,   129,   175,   571,
   571,   447,   735,   721,   456,   576,   577,   284,   524,   686,
   687,   520,   465,   133,   198,   155,   760,   761,   762,   157,
   205,   206,   354,   163,   166,   167,   583,   350,     6,   173,
   174,   709,   710,   167,   619,    12,   583,   332,   236,   334,
   335,   336,   337,   338,   559,     6,     6,   344,    12,   187,
   467,   604,    12,   186,   105,   353,   203,   237,   601,   238,
   188,   189,   738,   739,   361,   362,   363,  -233,   365,   366,
   367,   106,   370,   239,   138,   618,   139,   140,   188,   189,
   430,   623,   192,   440,   224,    70,   193,   394,   194,   432,
   204,   538,   539,   136,   137,   138,   190,   139,   140,   312,
   312,   225,   319,   324,   198,   324,   756,   605,   611,   612,
   613,   345,   198,   234,   129,   289,   290,   291,   292,   293,
   294,   295,   296,   235,   358,   413,   360,   451,   452,   244,
   308,   308,   518,   430,   583,     6,   431,   146,     9,    10,
   246,    70,   432,   445,   147,   251,   373,   378,    75,    76,
   450,   388,   388,   388,   148,   105,   102,   188,   189,   356,
   616,   457,   149,     6,   375,   315,   150,   162,   325,   364,
    12,   376,   106,   162,   646,   711,    12,   321,   247,     6,
   375,   343,   249,   312,   477,   163,   348,   376,   351,-32768,
   128,   167,   397,   252,   319,   499,   500,   359,   133,   433,
   505,   171,   507,   527,   528,   510,   253,   583,   583,   171,
   594,   449,   257,   597,   308,   258,   734,    12,   680,   455,
   146,   260,   740,   395,   741,   522,   263,   147,    90,     2,
   526,     3,     4,     5,   530,   468,   532,   148,   408,   437,
   442,   713,   714,   443,   444,   149,   699,   373,   701,   150,
   554,   248,   250,  -283,    18,   261,   482,  -283,   262,  -283,
   264,   653,   198,   265,    90,   654,   561,   655,   562,   266,
   198,   720,   268,   565,   763,   764,   765,   135,   136,   137,
   138,   270,   139,   140,   590,   171,   516,   591,   592,   523,
   286,   272,   324,   283,   339,   598,   529,   200,   201,   202,
   123,   124,   125,   126,   146,   649,   127,   128,   643,   644,
   712,   147,   513,   346,   352,   555,   615,   357,   617,   380,
   556,   148,   418,   385,   368,   558,   128,     6,   375,   149,
   396,  -143,   349,   150,    12,   376,  -142,   629,   311,   311,
   398,   318,  -141,   404,   406,  -138,   589,   515,   416,   198,
   198,   578,   632,   582,   124,   125,   126,   521,  -139,   127,
   128,  -140,   482,   582,   411,   415,   478,   313,   313,   429,
   320,   388,   446,     6,   607,   609,     9,    10,-32768,-32768,
    12,   649,   127,   128,   448,   102,   377,   463,   312,   453,
   464,   319,   469,   470,   476,   492,   288,   479,   660,   493,
   494,   661,   498,   683,   495,   298,   299,   300,   301,   302,
   303,   304,   497,   536,   675,   512,   633,   514,   676,   308,
   327,   564,   311,  -284,   636,   557,   563,  -284,   684,  -284,
   638,   685,   480,   318,   574,   575,   454,   479,   682,   422,
   423,   424,   631,   425,   610,   620,   698,   625,   634,   439,
   439,   313,   635,   702,   637,   578,   640,   133,   642,     6,
   540,   582,   320,   647,   399,   652,    12,   541,   658,   650,
   627,   686,   687,     6,   662,   668,     9,    10,   441,   441,
    12,   679,   286,   458,   459,   460,   102,   461,   666,   542,
   543,   667,   432,   681,   695,   481,   678,   696,   700,   730,
   731,   704,   715,   434,   718,   724,   728,   207,   732,   733,
   693,   742,     6,   375,   544,   545,   531,   546,   547,    12,
   376,   743,   744,   748,   752,   658,   751,   753,   435,   318,
   519,   436,   749,   767,   582,   582,   135,   136,   137,   138,
   768,   139,   140,   614,   716,   151,   697,   400,   118,   156,
   677,   439,   324,   506,   407,   327,   439,   723,   320,   552,
   639,   533,   133,   727,   548,   693,     6,   375,   198,   172,
   754,   746,   355,    12,   376,   347,   333,   153,    53,   658,
   441,   719,   479,   641,   566,   441,   745,   508,   509,   750,
   298,   716,   481,   386,   648,   133,   599,   657,   595,   747,
     0,   481,   481,   379,     0,   478,   133,     0,  -284,     0,
     0,     0,  -284,   606,  -284,   327,   327,    53,   722,   120,
   121,   122,   123,   124,   125,   126,     0,   311,   127,   128,
   318,     0,   318,     0,     0,     0,   479,     0,     0,   593,
     0,   135,   136,   137,   138,   133,   139,   140,     0,   439,
   188,   189,   400,     0,     0,   755,   313,     0,     0,   320,
     0,   320,  -284,     0,     0,     0,  -284,     0,  -284,     0,
     0,   480,     0,   134,   135,   136,   137,   138,   441,   139,
   140,     0,   566,     0,   402,   135,   136,   137,   138,     0,
   139,   140,     0,   121,   122,   123,   124,   125,   126,     0,
   481,   127,   128,     0,     0,     0,   656,   481,     0,     0,
   298,     0,     0,     0,     0,     0,     0,     0,   327,     0,
   327,     0,     0,   421,   135,   136,   137,   138,     0,   139,
   140,   122,   123,   124,   125,   126,     0,     0,   127,   128,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   692,
     0,     0,   327,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   481,     0,     0,     0,     0,     0,
     0,     0,     0,   481,   481,     0,     0,     0,   694,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,   318,     0,     0,     0,     0,     0,     0,     0,
     0,     1,     0,     2,   692,     3,     4,     5,     6,     7,
     8,     9,    10,     0,    11,    12,    13,     0,   481,     0,
     0,   320,     0,    14,     0,    15,    16,    17,    18,  -239,
    19,     0,     0,   694,     0,     0,  -239,    20,    21,    22,
     0,     0,     0,    23,    24,    25,  -239,     0,    26,    27,
     0,    28,     0,    29,  -239,    30,     0,    31,  -239,    32,
    33,     0,    34,    35,    36,    37,    38,    39,    40,   -11,
     0,     0,     0,    41,    42,    43,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    44,     0,     0,
     0,     0,     0,     0,     0,     0,    45,     0,     0,    46,
    47,     0,     0,     0,    48,     0,    49,    50,    51,     0,
    52,     0,     2,    53,     3,     4,     5,     6,     7,     8,
     9,    10,     0,    11,    12,    13,     0,     0,     0,     0,
     0,     0,    14,     0,    15,    16,    17,    18,     0,    19,
     0,     0,     0,     0,     0,     0,    20,    21,    22,     0,
     0,     0,    23,    24,    25,     0,     0,    26,    27,     0,
    28,     0,    29,     0,    30,     0,    31,     0,    32,    33,
     0,    34,    35,    36,    37,    38,    39,    40,   -11,     0,
     0,     0,    41,    42,    43,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    44,     0,     0,     0,
     0,     0,     0,     0,     0,    45,     0,     0,    46,    47,
     0,     0,     0,    48,     0,    49,    50,    51,     0,    52,
     0,     0,    53,     6,     7,     8,     9,    10,     0,    11,
    12,    13,     0,     0,     0,     0,     0,     0,    14,     0,
    15,    16,    17,     0,     0,    19,     0,     0,     0,     0,
     0,     0,    20,    21,    22,     0,     0,     0,    23,    24,
    25,     0,     0,    26,    27,     0,    28,     0,    29,     0,
    30,     0,    31,     0,    32,    33,   259,    34,    35,    36,
    37,    38,    39,    40,     0,     0,     0,     0,    41,     0,
    43,     0,     0,     0,     0,   120,   121,   122,   123,   124,
   125,   126,    44,     0,   127,   128,     0,     0,     0,     0,
     0,    45,     0,     0,    46,    47,     0,     0,     0,    48,
     0,    49,    50,    51,     0,    52,     0,     0,    53,     6,
     7,     8,     9,    10,     0,    11,    12,    13,     0,     0,
     0,     0,     0,     0,    14,     0,    15,    16,    17,     0,
     0,    19,     0,     0,     0,     0,     0,     0,    20,    21,
    22,     0,     0,     0,    23,    24,    25,     0,     0,    26,
    27,     0,    28,     0,    29,     0,    30,     0,    31,     0,
    32,    33,     0,    34,    35,    36,    37,    38,    39,    40,
     0,     0,     0,     0,    41,     0,    43,     0,     0,     0,
     0,   120,   121,   122,   123,   124,   125,   126,    44,     0,
   127,   128,     0,     0,     0,     0,     0,    45,     0,     0,
    46,    47,     0,     0,     0,    48,     0,    49,    50,    51,
     0,    52,     0,     0,    53,     6,     7,     8,     9,    10,
     0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
    14,     0,    15,    16,    17,     0,     0,    19,     0,     0,
     0,     0,     0,     0,    20,   305,    22,     0,     0,     0,
    23,    24,    25,     0,     0,    26,    27,     0,    28,     0,
    29,     0,    30,     0,    31,     0,    32,    33,     0,    34,
   306,    36,    37,   307,    39,    40,     0,     0,     0,     0,
    41,     0,    43,     0,     0,     0,     0,   120,   121,   122,
   123,   124,   125,   126,    44,     0,   127,   128,     0,     0,
     0,     0,     0,    45,     0,     0,    46,    47,     0,     0,
     0,    48,     0,    49,    50,    51,     0,    52,     0,     0,
    53,     6,     7,     8,     9,    10,     0,    11,    12,    13,
     0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
    17,     0,     0,    19,     0,     0,     0,     0,     0,     0,
    20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
     0,    26,    27,     0,    28,     0,    29,     0,    30,     0,
    31,     0,    32,    33,     0,    34,    35,    36,    37,    38,
    39,    40,     0,     0,     0,     0,    41,     0,    43,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    44,     0,     0,     0,     0,     0,     0,     0,     0,    45,
     0,     0,    46,    47,     0,     0,     0,    48,     0,    49,
    50,    51,     0,    52,     0,     0,    53,     6,     7,     8,
     9,    10,     0,    11,    12,    13,     0,     0,     0,     0,
     0,     0,    14,     0,    15,    16,    17,     0,     0,    19,
     0,     0,     0,     0,     0,     0,    20,   305,    22,     0,
     0,     0,    23,    24,    25,     0,     0,    26,    27,     0,
    28,     0,    29,     0,    30,     0,    31,     0,    32,    33,
     0,    34,   306,    36,    37,   307,    39,    40,     0,     0,
     0,     0,    41,     0,    43,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    44,     0,     0,     0,
     0,     0,     0,     0,     0,    45,     0,     0,    46,    47,
     0,     0,     0,    48,     0,    49,    50,    51,     0,    52,
     0,     0,    53
};

static const short yycheck[] = {    15,
    16,    17,    18,    19,   230,    42,   103,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    57,   319,    34,   277,
   278,    37,    42,   518,    40,    41,   198,    43,    44,    45,
    46,    47,    73,   367,   206,    51,    52,    53,   473,   600,
   488,   602,   603,   402,    94,    61,    62,   271,   381,   415,
   471,   381,   225,    95,   273,   315,   316,    13,   442,    42,
   184,    69,   322,   335,   336,   325,   326,    19,   374,    15,
     8,   593,     8,   197,    18,    11,    12,    15,    47,     8,
   471,   635,    11,    12,   424,    74,    15,    76,    71,    72,
    34,   609,    61,    52,    15,    48,   657,    42,    15,   609,
    74,   530,    61,   532,   120,   121,   122,   123,   124,   269,
   151,    14,   128,     4,   130,   131,   132,   241,   134,     0,
   460,   281,   138,   139,   140,   141,   142,   143,   650,   171,
   146,   147,   148,   149,   150,   308,   690,   572,   586,    92,
    93,    97,    98,    96,    97,    98,    99,   477,   372,   570,
   571,   535,   518,   105,   487,   100,   674,   487,   377,   378,
   106,   106,   107,   522,   674,   437,   472,    70,    82,   107,
   442,   116,    13,   445,    88,    56,   112,    58,    92,   570,
   571,   341,   704,   678,   356,   106,   107,   203,   423,   106,
   107,   417,   364,    13,   108,     4,   757,   758,   759,    80,
   114,   115,   252,    84,    85,    86,   478,   248,     8,    90,
    91,   659,   660,    94,   516,    15,   488,   233,    21,   235,
   236,   237,   238,   239,   459,     8,     8,   243,    15,    81,
   564,    14,    15,   112,   271,   251,    47,    40,   496,    42,
    92,    93,   706,   707,   260,   261,   262,    81,   264,   265,
   266,   271,   268,    56,    95,   515,    97,    98,    92,    93,
    81,   521,   104,   535,   104,     0,   108,   283,   110,    90,
   105,   443,   444,    93,    94,    95,   373,    97,    98,   224,
   225,   104,   227,   228,   198,   230,   750,    70,   502,   503,
   504,   244,   206,    18,   175,   209,   210,   211,   212,   213,
   214,   215,   216,    35,   257,   321,   259,   348,   349,    31,
   224,   225,   678,    81,   586,     8,    84,    29,    11,    12,
    18,    56,    90,   339,    36,    35,   271,   272,   101,   102,
   346,   276,   277,   278,    46,   372,    42,    92,    93,   253,
   513,   357,    54,     8,     9,   226,    58,    82,   229,   263,
    15,    16,   372,    88,   573,   661,    15,    16,    47,     8,
     9,   242,    47,   308,   380,   246,   247,    16,   249,    89,
    90,   252,   286,    18,   319,    71,    72,   258,    13,   332,
   396,    87,   398,   427,   428,   401,   110,   659,   660,    95,
   487,   344,    61,   490,   308,    47,   702,    15,    16,   352,
    29,    35,   708,   284,   710,   421,   110,    36,   424,     3,
   426,     5,     6,     7,   430,   368,   432,    46,    47,   335,
   336,   666,   667,   337,   338,    54,   645,   372,   647,    58,
   446,   166,   167,   104,    28,    35,   381,   108,    35,   110,
    35,   104,   356,    35,   460,   108,   462,   110,   464,    24,
   364,   677,    35,   469,   760,   761,   762,    92,    93,    94,
    95,    81,    97,    98,   480,   171,   411,   483,   484,   422,
   205,    76,   417,    88,    50,   491,   429,   109,   110,   111,
    83,    84,    85,    86,    29,   582,    89,    90,   570,   571,
   662,    36,   406,    35,    61,   448,   512,    35,   514,    60,
   453,    46,    47,    47,    41,   458,    90,     8,     9,    54,
   109,   112,   247,    58,    15,    16,   112,   533,   224,   225,
   111,   227,   112,    19,   112,   112,   479,   408,    22,   443,
   444,   476,   548,   478,    84,    85,    86,   418,   112,    89,
    90,   112,   487,   488,   112,   104,    47,   224,   225,    61,
   227,   496,    35,     8,   499,   500,    11,    12,    85,    86,
    15,   658,    89,    90,    61,   271,   272,    35,   513,    61,
    35,   516,    35,    77,   104,    35,   208,    78,   594,    18,
   105,   597,   111,    38,    79,   217,   218,   219,   220,   221,
   222,   223,   109,   112,   610,   105,   549,   105,   614,   513,
   232,    18,   308,   104,   557,   111,   111,   108,    63,   110,
   563,    66,   113,   319,    35,    35,   351,    78,   634,    31,
    32,    33,   536,    35,    35,   105,   642,    35,    81,   335,
   336,   308,   104,   649,    35,   580,   112,    13,    35,     8,
     9,   586,   319,   105,    20,   590,    15,    16,   593,    81,
   531,   106,   107,     8,   104,   608,    11,    12,   335,   336,
    15,    47,   397,    31,    32,    33,   372,    35,    81,    38,
    39,    81,    90,    35,    35,   381,   621,    35,    35,   695,
   696,    81,    35,    38,    35,   112,    17,   632,    35,    35,
   635,   105,     8,     9,    63,    64,   431,    66,    67,    15,
    16,    35,    35,   105,   109,   650,   105,   111,    63,   415,
   416,    66,   728,     0,   659,   660,    92,    93,    94,    95,
     0,    97,    98,   508,   669,    70,   640,   103,    56,    78,
   620,   437,   677,   397,   314,   367,   442,   682,   415,   445,
   564,   433,    13,   690,   113,   690,     8,     9,   662,    88,
   746,   724,   252,    15,    16,   246,   234,    74,   113,   704,
   437,   674,    78,   568,   470,   442,   716,   399,   400,   736,
   402,   716,   478,   275,   580,    13,   493,   592,   487,   724,
    -1,   487,   488,   272,    -1,    47,    13,    -1,   104,    -1,
    -1,    -1,   108,   499,   110,   427,   428,   113,   679,    80,
    81,    82,    83,    84,    85,    86,    -1,   513,    89,    90,
   516,    -1,   518,    -1,    -1,    -1,    78,    -1,    -1,    81,
    -1,    92,    93,    94,    95,    13,    97,    98,    -1,   535,
    92,    93,   103,    -1,    -1,   749,   513,    -1,    -1,   516,
    -1,   518,   104,    -1,    -1,    -1,   108,    -1,   110,    -1,
    -1,   113,    -1,    91,    92,    93,    94,    95,   535,    97,
    98,    -1,   568,    -1,    91,    92,    93,    94,    95,    -1,
    97,    98,    -1,    81,    82,    83,    84,    85,    86,    -1,
   586,    89,    90,    -1,    -1,    -1,   592,   593,    -1,    -1,
   522,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   530,    -1,
   532,    -1,    -1,    91,    92,    93,    94,    95,    -1,    97,
    98,    82,    83,    84,    85,    86,    -1,    -1,    89,    90,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   635,
    -1,    -1,   564,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,   650,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,   659,   660,    -1,    -1,    -1,   635,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,   678,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,     1,    -1,     3,   690,     5,     6,     7,     8,     9,
    10,    11,    12,    -1,    14,    15,    16,    -1,   704,    -1,
    -1,   678,    -1,    23,    -1,    25,    26,    27,    28,    29,
    30,    -1,    -1,   690,    -1,    -1,    36,    37,    38,    39,
    -1,    -1,    -1,    43,    44,    45,    46,    -1,    48,    49,
    -1,    51,    -1,    53,    54,    55,    -1,    57,    58,    59,
    60,    -1,    62,    63,    64,    65,    66,    67,    68,    69,
    -1,    -1,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    96,    -1,    -1,    99,
   100,    -1,    -1,    -1,   104,    -1,   106,   107,   108,    -1,
   110,    -1,     3,   113,     5,     6,     7,     8,     9,    10,
    11,    12,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,
    -1,    -1,    23,    -1,    25,    26,    27,    28,    -1,    30,
    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,    -1,
    -1,    -1,    43,    44,    45,    -1,    -1,    48,    49,    -1,
    51,    -1,    53,    -1,    55,    -1,    57,    -1,    59,    60,
    -1,    62,    63,    64,    65,    66,    67,    68,    69,    -1,
    -1,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    96,    -1,    -1,    99,   100,
    -1,    -1,    -1,   104,    -1,   106,   107,   108,    -1,   110,
    -1,    -1,   113,     8,     9,    10,    11,    12,    -1,    14,
    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,
    25,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,
    -1,    -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,
    45,    -1,    -1,    48,    49,    -1,    51,    -1,    53,    -1,
    55,    -1,    57,    -1,    59,    60,    61,    62,    63,    64,
    65,    66,    67,    68,    -1,    -1,    -1,    -1,    73,    -1,
    75,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
    85,    86,    87,    -1,    89,    90,    -1,    -1,    -1,    -1,
    -1,    96,    -1,    -1,    99,   100,    -1,    -1,    -1,   104,
    -1,   106,   107,   108,    -1,   110,    -1,    -1,   113,     8,
     9,    10,    11,    12,    -1,    14,    15,    16,    -1,    -1,
    -1,    -1,    -1,    -1,    23,    -1,    25,    26,    27,    -1,
    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
    39,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,    48,
    49,    -1,    51,    -1,    53,    -1,    55,    -1,    57,    -1,
    59,    60,    -1,    62,    63,    64,    65,    66,    67,    68,
    -1,    -1,    -1,    -1,    73,    -1,    75,    -1,    -1,    -1,
    -1,    80,    81,    82,    83,    84,    85,    86,    87,    -1,
    89,    90,    -1,    -1,    -1,    -1,    -1,    96,    -1,    -1,
    99,   100,    -1,    -1,    -1,   104,    -1,   106,   107,   108,
    -1,   110,    -1,    -1,   113,     8,     9,    10,    11,    12,
    -1,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
    23,    -1,    25,    26,    27,    -1,    -1,    30,    -1,    -1,
    -1,    -1,    -1,    -1,    37,    38,    39,    -1,    -1,    -1,
    43,    44,    45,    -1,    -1,    48,    49,    -1,    51,    -1,
    53,    -1,    55,    -1,    57,    -1,    59,    60,    -1,    62,
    63,    64,    65,    66,    67,    68,    -1,    -1,    -1,    -1,
    73,    -1,    75,    -1,    -1,    -1,    -1,    80,    81,    82,
    83,    84,    85,    86,    87,    -1,    89,    90,    -1,    -1,
    -1,    -1,    -1,    96,    -1,    -1,    99,   100,    -1,    -1,
    -1,   104,    -1,   106,   107,   108,    -1,   110,    -1,    -1,
   113,     8,     9,    10,    11,    12,    -1,    14,    15,    16,
    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    25,    26,
    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,    -1,
    -1,    48,    49,    -1,    51,    -1,    53,    -1,    55,    -1,
    57,    -1,    59,    60,    -1,    62,    63,    64,    65,    66,
    67,    68,    -1,    -1,    -1,    -1,    73,    -1,    75,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,
    -1,    -1,    99,   100,    -1,    -1,    -1,   104,    -1,   106,
   107,   108,    -1,   110,    -1,    -1,   113,     8,     9,    10,
    11,    12,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,
    -1,    -1,    23,    -1,    25,    26,    27,    -1,    -1,    30,
    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,    -1,
    -1,    -1,    43,    44,    45,    -1,    -1,    48,    49,    -1,
    51,    -1,    53,    -1,    55,    -1,    57,    -1,    59,    60,
    -1,    62,    63,    64,    65,    66,    67,    68,    -1,    -1,
    -1,    -1,    73,    -1,    75,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    96,    -1,    -1,    99,   100,
    -1,    -1,    -1,   104,    -1,   106,   107,   108,    -1,   110,
    -1,    -1,   113
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */


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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         -2
#define YYEOF           0
#define YYACCEPT        return(0)
#define YYABORT         return(1)
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

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

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
     int count;
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
__yy_memcpy (char *to, char *from, int count)
{
  register char *f = from;
  register char *t = to;
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
          return 2;
        }
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
        yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1, size * sizeof (*yylsp));
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
{ yyval.t = 0; ;
    break;}
case 23:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 24:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 25:
{ yyval.t = newCTerm(PA_fAssign,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 26:
{
                    /* <<for X 'from' E1 to E2 by E3>> */
                    /* coord after T_ITER somehow avoids shift/reduce conflict,
                       but serves no other purpose */
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
case 27:
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
case 28:
{ yyval.t = newCTerm(PA_fOrElse,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 29:
{ yyval.t = newCTerm(PA_fAndThen,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 30:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
                                  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 31:
{ yyval.t = newCTerm(PA_fFdCompare,yyvsp[-2].t,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 32:
{ yyval.t = newCTerm(PA_fFdIn,yyvsp[-2].t,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 33:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 34:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 35:
{ yyval.t = newCTerm(PA_fRecord,
                                  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
                                  oz_cons(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 36:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 37:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 38:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
                                  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 39:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
                                  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 40:
{ yyval.t = newCTerm(PA_fOpApply,yyvsp[-2].t,
                                  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 41:
{ yyval.t = newCTerm(PA_fObjApply,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 42:
{ yyval.t = newCTerm(PA_fOpApply,AtomTilde,
                                  oz_mklist(yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 43:
{ yyval.t = newCTerm(PA_fOpApply,AtomDot,
                                  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 44:
{ yyval.t = newCTerm(PA_fOpApply,AtomDot,
                                  oz_mklist(yyvsp[-1].t,makeInt(xytext,pos())),pos()); ;
    break;}
case 45:
{ yyval.t = newCTerm(PA_fOpApply,AtomHat,
                                  oz_mklist(yyvsp[-3].t,yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 46:
{ yyval.t = newCTerm(PA_fAt,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 47:
{ yyval.t = newCTerm(PA_fOpApply,AtomDExcl,
                                  oz_mklist(yyvsp[0].t),yyvsp[-1].t); ;
    break;}
case 48:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 49:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 50:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 51:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 52:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 53:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 54:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 55:
{ yyval.t = newCTerm(PA_fSelf,pos()); ;
    break;}
case 56:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 57:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 58:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 59:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 60:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 61:
{ yyval.t = newCTerm(PA_fRecord,newCTerm(PA_fAtom,AtomCons,
                                                     makeLongPos(yyvsp[-4].t,yyvsp[0].t)),
                                  oz_mklist(yyvsp[-3].t,yyvsp[-2].t)); ;
    break;}
case 62:
{ yyval.t = newCTerm(PA_fApply,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 63:
{ yyval.t = newCTerm(PA_fProc,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-7].t,makeLongPos(yyvsp[-8].t,yyvsp[0].t)); ;
    break;}
case 64:
{ yyval.t = newCTerm(PA_fFun,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-7].t,makeLongPos(yyvsp[-8].t,yyvsp[0].t)); ;
    break;}
case 65:
{ yyval.t = newCTerm(PA_fFunctor,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 66:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 67:
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t); ;
    break;}
case 68:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 69:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 70:
{ yyval.t = newCTerm(PA_fLock,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 71:
{ yyval.t = newCTerm(PA_fLockThen,yyvsp[-4].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 72:
{ yyval.t = newCTerm(PA_fThread,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 73:
{ yyval.t = newCTerm(PA_fTry,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 74:
{ yyval.t = newCTerm(PA_fRaise,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 75:
{ yyval.t = newCTerm(PA_fSkip,pos()); ;
    break;}
case 76:
{ yyval.t = newCTerm(PA_fFail,pos()); ;
    break;}
case 77:
{ yyval.t = newCTerm(PA_fNot,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 78:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 79:
{ yyval.t = newCTerm(PA_fOr,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 80:
{ yyval.t = newCTerm(PA_fDis,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 81:
{ yyval.t = newCTerm(PA_fChoice,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 82:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 83:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 84:
{ yyval.t = newCTerm(PA_fLoop,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 85:
{ yyval.t = newCTerm(PA_fMacro,yyvsp[-2].t,makeLongPos(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 86:
{ yyval.t = AtomNil; ;
    break;}
case 87:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 88:
{ yyval.t = AtomNil; ;
    break;}
case 89:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 90:
{ yyval.t = oz_cons(newCTerm(PA_fRequire,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 91:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 92:
{ yyval.t = oz_cons(newCTerm(PA_fPrepare,yyvsp[-1].t,
                                           newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 93:
{ yyval.t = oz_cons(newCTerm(PA_fImport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 94:
{ yyval.t = oz_cons(newCTerm(PA_fExport,yyvsp[-1].t,yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 95:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-3].t,yyvsp[-1].t,yyvsp[-4].t),yyvsp[0].t); ;
    break;}
case 96:
{ yyval.t = oz_cons(newCTerm(PA_fDefine,yyvsp[-1].t,
                                           newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-2].t),yyvsp[0].t); ;
    break;}
case 97:
{ yyval.t = AtomNil; ;
    break;}
case 98:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-2].t,AtomNil,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 99:
{ yyval.t = oz_cons(newCTerm(PA_fImportItem,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 100:
{ yyval.t = newCTerm(PA_fVar,OZ_atom(xytext),yyvsp[0].t); ;
    break;}
case 101:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 102:
{ yyval.t = oz_mklist(oz_pair2(yyvsp[0].t,yyvsp[-2].t)); ;
    break;}
case 103:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 104:
{ yyval.t = oz_cons(oz_pair2(yyvsp[-1].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 105:
{ yyval.t = PA_fNoImportAt; ;
    break;}
case 106:
{ yyval.t = newCTerm(PA_fImportAt,yyvsp[0].t); ;
    break;}
case 107:
{ yyval.t = AtomNil; ;
    break;}
case 108:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 109:
{ yyval.t = oz_cons(newCTerm(PA_fExportItem,
                                           newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t)),yyvsp[0].t); ;
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
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 114:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 115:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 116:
{ yyval.t = newCTerm(PA_fLocal,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 117:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 118:
{ yyval.t = AtomNil; ;
    break;}
case 119:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 120:
{ yyval.t = newCTerm(PA_fAtom,AtomNil,yyvsp[0].t); ;
    break;}
case 121:
{ yyval.t = newCTerm(PA_fRecord,
                                  newCTerm(PA_fAtom,AtomCons,yyvsp[-2].t),
                                  oz_mklist(yyvsp[-1].t,yyvsp[0].t)); ;
    break;}
case 122:
{ yyval.t = PA_fNoCatch; ;
    break;}
case 123:
{ yyval.t = newCTerm(PA_fCatch,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 124:
{ yyval.t = PA_fNoFinally; ;
    break;}
case 125:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 126:
{
                    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
                                  newCTerm(PA_fAtom,yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
                  ;
    break;}
case 127:
{
                    yyval.t = newCTerm(OZ_isTrue(yyvsp[-2].t)? PA_fOpenRecord : PA_fRecord,
                                  makeVar(yyvsp[-6].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)),yyvsp[-3].t);
                  ;
    break;}
case 128:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 129:
{ yyval.t = NameUnit; ;
    break;}
case 130:
{ yyval.t = NameTrue; ;
    break;}
case 131:
{ yyval.t = NameFalse; ;
    break;}
case 132:
{ yyval.t = OZ_atom(xytext); ;
    break;}
case 133:
{ yyval.t = AtomNil; ;
    break;}
case 134:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 135:
{ yyval.t = oz_cons(newCTerm(PA_fColon,yyvsp[-3].t,yyvsp[-1].t),yyvsp[0].t); ;
    break;}
case 136:
{ yyval.t = NameFalse; ;
    break;}
case 137:
{ yyval.t = NameTrue; ;
    break;}
case 138:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 139:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 140:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 141:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 142:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 143:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 144:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 145:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 146:
{ yyval.t = newCTerm(PA_fBoolCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 147:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 148:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 149:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 150:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 151:
{ checkDeprecation(yyvsp[-3].t);
                    yyval.t = newCTerm(PA_fBoolCase,yyvsp[-5].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-6].t,yyvsp[0].t));
                  ;
    break;}
case 152:
{ yyval.t = newCTerm(PA_fCase,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 153:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 154:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 155:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 156:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 157:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 158:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 159:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 160:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 161:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 162:
{ yyval.t = newCTerm(PA_fCaseClause,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 163:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 164:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-3].t,
                                  newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 165:
{ yyval.t = newCTerm(PA_fSideCondition,yyvsp[-5].t,yyvsp[-2].t,yyvsp[0].t,yyvsp[-3].t); ;
    break;}
case 166:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 167:
{ yyval.t = makeCons(yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 168:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 169:
{ yyval.t = newCTerm(PA_fRecord,
                                  newCTerm(PA_fAtom,AtomPair,yyvsp[-1].t),
                                  oz_cons(yyvsp[-3].t,yyvsp[0].t)); ;
    break;}
case 170:
{ yyval.t = newCTerm(PA_fClass,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-5].t,yyvsp[0].t)); ;
    break;}
case 171:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 172:
{ yyval.t = newCTerm(PA_fDollar,yyvsp[0].t); ;
    break;}
case 173:
{ yyval.t = AtomNil; ;
    break;}
case 174:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 175:
{ yyval.t = newCTerm(PA_fFrom,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 176:
{ yyval.t = newCTerm(PA_fAttr,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 177:
{ yyval.t = newCTerm(PA_fFeat,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 178:
{ yyval.t = newCTerm(PA_fProp,oz_cons(yyvsp[-1].t,yyvsp[0].t),yyvsp[-2].t); ;
    break;}
case 179:
{ yyval.t = AtomNil; ;
    break;}
case 180:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 181:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 182:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 183:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 184:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 185:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 186:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 187:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 188:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 189:
{ yyval.t = AtomNil; ;
    break;}
case 190:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 191:
{ yyval.t = newCTerm(PA_fMeth,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-3].t); ;
    break;}
case 192:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 193:
{ yyval.t = newCTerm(PA_fEq,yyvsp[-3].t,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 194:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 195:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 196:
{ yyval.t = newCTerm(PA_fAtom,NameUnit,pos()); ;
    break;}
case 197:
{ yyval.t = newCTerm(PA_fAtom,NameTrue,pos()); ;
    break;}
case 198:
{ yyval.t = newCTerm(PA_fAtom,NameFalse,pos()); ;
    break;}
case 199:
{ yyval.t = newCTerm(PA_fRecord,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 200:
{ yyval.t = newCTerm(PA_fOpenRecord,yyvsp[-4].t,yyvsp[-2].t); ;
    break;}
case 201:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 202:
{ yyval.t = makeVar(xytext); ;
    break;}
case 203:
{ yyval.t = newCTerm(PA_fEscape,makeVar(xytext),yyvsp[-1].t); ;
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
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 208:
{ yyval.t = AtomNil; ;
    break;}
case 209:
{ yyval.t = newCTerm(PA_fMethArg,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 210:
{ yyval.t = newCTerm(PA_fMethColonArg,yyvsp[-3].t,yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 211:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 212:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 213:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 214:
{ yyval.t = newCTerm(PA_fDefault,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 215:
{ yyval.t = PA_fNoDefault; ;
    break;}
case 216:
{ yyval.t = newCTerm(PA_fCond,yyvsp[-3].t,yyvsp[-2].t,makeLongPos(yyvsp[-4].t,yyvsp[0].t)); ;
    break;}
case 217:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 218:
{ yyval.t = newCTerm(PA_fNoElse,pos()); ;
    break;}
case 219:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 220:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 221:
{ yyval.t = newCTerm(PA_fClause,newCTerm(PA_fSkip,yyvsp[-1].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 222:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 223:
{ yyval.t = oz_mklist(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 224:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 225:
{ yyval.t = newCTerm(PA_fClause,
                                  newCTerm(PA_fSkip,yyvsp[0].t),
                                  yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 226:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-3].t,yyvsp[-1].t,newCTerm(PA_fNoThen,yyvsp[0].t)); ;
    break;}
case 227:
{ yyval.t = newCTerm(PA_fClause,
                                  newCTerm(PA_fSkip,yyvsp[-2].t),yyvsp[-3].t,yyvsp[0].t); ;
    break;}
case 228:
{ yyval.t = newCTerm(PA_fClause,yyvsp[-4].t,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 229:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 230:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 231:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 232:
{ yyval.t = makeVar(xytext); ;
    break;}
case 233:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 234:
{ yyval.t = newCTerm(PA_fEscape,yyvsp[0].t,yyvsp[-1].t); ;
    break;}
case 235:
{ yyval.t = makeString(xytext,pos()); ;
    break;}
case 236:
{ yyval.t = makeInt(xytext,pos()); ;
    break;}
case 237:
{ yyval.t = makeInt(xytext[0],pos()); ;
    break;}
case 238:
{ yyval.t = newCTerm(PA_fFloat,OZ_CStringToFloat(xytext),pos()); ;
    break;}
case 239:
{ yyval.t = pos(); ;
    break;}
case 240:
{ yyval.t = pos(); ;
    break;}
case 241:
{ OZ_Term prefix =
                      scannerPrefix? scannerPrefix: PA_zy;
                    yyval.t = newCTerm(PA_fScanner,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,prefix,
                                  makeLongPos(yyvsp[-6].t,yyvsp[0].t)); ;
    break;}
case 242:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 243:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 244:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 245:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 246:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 247:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 248:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 249:
{ yyval.t = newCTerm(PA_fLexicalAbbreviation,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 250:
{ yyval.t = newCTerm(PA_fLexicalRule,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 251:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 252:
{ yyval.t = OZ_string(xytext); ;
    break;}
case 253:
{ yyval.t = newCTerm(PA_fMode,yyvsp[-2].t,yyvsp[-1].t); ;
    break;}
case 254:
{ yyval.t = AtomNil; ;
    break;}
case 255:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 256:
{ yyval.t = newCTerm(PA_fInheritedModes,yyvsp[0].t); ;
    break;}
case 257:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 258:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 259:
{ OZ_Term expect = parserExpect? parserExpect: newSmallInt(0);
                    yyval.t = newCTerm(PA_fParser,yyvsp[-6].t,yyvsp[-5].t,yyvsp[-4].t,yyvsp[-3].t,yyvsp[-2].t,expect,
                                  makeLongPos(yyvsp[-7].t,yyvsp[0].t)); ;
    break;}
case 260:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 261:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 262:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 263:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 264:
{ yyval.t = newCTerm(PA_fToken,AtomNil); ;
    break;}
case 265:
{ yyval.t = newCTerm(PA_fToken,yyvsp[0].t); ;
    break;}
case 266:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 267:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 268:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 269:
{ yyval.t = oz_pair2(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 270:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 271:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 272:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 273:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 274:
{ *prodKey[depth]++ = '='; ;
    break;}
case 275:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 276:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 277:
{ *prodKey[depth]++ = '='; ;
    break;}
case 278:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,yyvsp[-7].t); ;
    break;}
case 279:
{ yyval.t = newCTerm(PA_fProductionTemplate,yyvsp[-3].t,yyvsp[-4].t,yyvsp[-2].t,yyvsp[-1].t,PA_none); ;
    break;}
case 280:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 281:
{ yyval.t = oz_mklist(yyvsp[-1].t); ;
    break;}
case 282:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 285:
{ prodName[depth] = OZ_string(OZ_atomToC(OZ_getArg(yyvsp[-1].t,0))); ;
    break;}
case 286:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 287:
{ depth--; ;
    break;}
case 288:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 289:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 290:
{ depth--; ;
    break;}
case 291:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 292:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 293:
{ depth--; ;
    break;}
case 294:
{ yyval.t = yyvsp[-3].t; ;
    break;}
case 295:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 296:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 297:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 298:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 299:
{ *prodKey[depth - 1]++ = '/'; *prodKey[depth - 1]++ = '/'; ;
    break;}
case 302:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 303:
{ *prodKey[depth]++ = xytext[0]; ;
    break;}
case 304:
{ *prodKey[depth] = '\0';
                    yyval.t = oz_pair2(prodName[depth],OZ_string(prodKeyBuffer[depth]));
                    prodName[depth] = PA_none;
                    prodKey[depth] = prodKeyBuffer[depth];
                  ;
    break;}
case 305:
{ yyval.t = AtomNil; ;
    break;}
case 306:
{ yyval.t = yyvsp[-1].t; ;
    break;}
case 307:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 308:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 309:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 310:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-2].t,AtomNil,yyvsp[-1].t); ;
    break;}
case 311:
{ yyval.t = newCTerm(PA_fSyntaxRule,yyvsp[-5].t,yyvsp[-3].t,yyvsp[-1].t); ;
    break;}
case 312:
{ yyval.t = AtomNil; ;
    break;}
case 313:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 314:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 315:
{ yyval.t = newCTerm(PA_fDollar,pos()); ;
    break;}
case 316:
{ yyval.t = newCTerm(PA_fWildcard,pos()); ;
    break;}
case 317:
{ yyval.t = newCTerm(PA_fSynAlternative, yyvsp[0].t); ;
    break;}
case 318:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 319:
{ yyval.t = oz_cons(yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 320:
{ OZ_Term t = yyvsp[0].t;
                    while (terms[depth]) {
                      t = oz_cons(newCTerm(PA_fSynApplication, terms[depth]->term, AtomNil), t);
                      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
                    }
                    yyval.t = newCTerm(PA_fSynSequence, decls[depth], t, yyvsp[-1].t);
                    decls[depth] = AtomNil;
                  ;
    break;}
case 321:
{ yyval.t = newCTerm(PA_fSynSequence, AtomNil, yyvsp[0].t, yyvsp[-1].t); ;
    break;}
case 322:
{ yyval.t = AtomNil; ;
    break;}
case 323:
{ yyval.t = oz_mklist(newCTerm(PA_fSynAction,yyvsp[0].t)); ;
    break;}
case 324:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 325:
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
case 326:
{ yyval.t = oz_cons(newCTerm(PA_fSynAssignment, terms[depth]->term, yyvsp[-1].t),
                                  yyvsp[0].t);
                    TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
                  ;
    break;}
case 327:
{ while (terms[depth]) {
                      decls[depth] = oz_cons(terms[depth]->term, decls[depth]);
                      TermNode *tmp = terms[depth]; terms[depth] = terms[depth]->next; delete tmp;
                    }
                    yyval.t = yyvsp[0].t;
                  ;
    break;}
case 328:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 329:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 330:
{ terms[depth] = new TermNode(yyvsp[0].t, terms[depth]); ;
    break;}
case 331:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 332:
{ yyval.t = oz_cons(yyvsp[-1].t,yyvsp[0].t); ;
    break;}
case 333:
{ yyval.t = newCTerm(PA_fSynAssignment,yyvsp[-2].t,yyvsp[0].t); ;
    break;}
case 334:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 335:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 336:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
                                  oz_cons(newCTerm(PA_fSynApplication,yyvsp[-3].t,
                                                    AtomNil),
                                           AtomNil),yyvsp[-1].t);
                  ;
    break;}
case 337:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 338:
{ yyval.t = newCTerm(PA_fSynAssignment,
                                  newCTerm(PA_fEscape,yyvsp[-2].t,yyvsp[-3].t),yyvsp[0].t); ;
    break;}
case 339:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 340:
{ yyval.t = yyvsp[0].t; ;
    break;}
case 341:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
                                  oz_mklist(yyvsp[-2].t),yyvsp[-3].t);
                  ;
    break;}
case 342:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,
                                  oz_mklist(yyvsp[-3].t),yyvsp[-1].t);
                  ;
    break;}
case 343:
{ *prodKey[depth]++ = '('; depth++; ;
    break;}
case 344:
{ depth--; ;
    break;}
case 345:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 346:
{ *prodKey[depth]++ = '['; depth++; ;
    break;}
case 347:
{ depth--; ;
    break;}
case 348:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 349:
{ *prodKey[depth]++ = '{'; depth++; ;
    break;}
case 350:
{ depth--; ;
    break;}
case 351:
{ yyval.t = newCTerm(PA_fSynTemplateInstantiation,yyvsp[0].t,yyvsp[-4].t,yyvsp[-7].t); ;
    break;}
case 352:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[0].t,AtomNil); ;
    break;}
case 353:
{ yyval.t = newCTerm(PA_fSynApplication,yyvsp[-4].t,yyvsp[-1].t); ;
    break;}
case 354:
{ yyval.t = newCTerm(PA_fAtom,OZ_atom(xytext),pos()); ;
    break;}
case 355:
{ yyval.t = makeVar(xytext); ;
    break;}
case 356:
{ yyval.t = oz_mklist(yyvsp[0].t); ;
    break;}
case 357:
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
