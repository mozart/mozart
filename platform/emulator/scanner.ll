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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

%option noyywrap noreject nodefault

%{
#include <string.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/stat.h>

#include "base.hh"
#include "oz.h"
#include "os.hh"

#include "parser.hh"

void xyreportError(char *kind, char *message,
                   const char *file, int line, int offset);

char xyFileName[100];
char xyhelpFileName[100];
OZ_Term xyFileNameAtom;

int xy_showInsert, xy_gumpSyntax, xy_systemVariables;
OZ_Term xy_errorMessages;

static int errorFlag;


//*******************
// THE INPUT ROUTINE
//*******************

int xylino;                             // current line number
char *xylastline;                       // remember where we have put the input

static inline int xycharno() {
  int n = xytext - xylastline;
  if (n > 0)
    return n;
  else
    return 0;
}

#undef YY_INPUT
#define YY_INPUT(buf, result, max_size)   xy_input(buf, result, max_size)

static void xy_input(char *buf, int &result, const int max_size) {
  xylastline = buf;

  // read one line into buf
  int curpos = 0;
  int c = fgetc(xyin);
  while(curpos < max_size && c != EOF && c != OZEOF && c != '\n') {
    buf[curpos++] = c;
    if (curpos < max_size)
      c = fgetc(xyin);
  }

  if (c == EOF || c == OZEOF) {
    if (curpos > 0)   // did we read other chars than EOF?
      result = curpos;
    else
      result = YY_NULL;
  } else {
    if (curpos < max_size)
      buf[curpos++] = c;
    result = curpos;
  }
}


//****************
// OUTPUT OF FLEX
//****************

#undef YY_DECL
#define YY_DECL static int xymylex()


//******************
// HASHING
// for \define etc.
//******************

#define PRIME 11987 // has to be a prime number

class ScannerListNode {
public:
  char *key;
  ScannerListNode *next;

  ScannerListNode(char *k, ScannerListNode *n) {
    key = new char[strlen(k) + 1];
    strcpy(key, k);
    next = n;
  }
  ~ScannerListNode() {
    delete[] key;
  }
};

typedef ScannerListNode *ScannerListNodePtr;

class XyScannerHashTable {
private:
  int TableSize;
  ScannerListNode **Table;

  int hashFunc(char *s) {
    // taken from 'Aho, Sethi, Ullman: Compilers ...', page 436
    char *p;
    unsigned h = 0, g;
    for(p = s; *p; p++) {
      h = (h << 4) + (*p);
      if ((g = h & 0xf0000000)) {
        h = h ^ (g >> 24);
        h = h ^ g;
      }
    }
    return h % TableSize;
  }

public:

  XyScannerHashTable(int size = PRIME) {
    TableSize = size;
    Table = new ScannerListNodePtr[TableSize];
    for (int i = 0; i < TableSize; i++)
      Table[i] = (ScannerListNode *) 0;
  }

  ~XyScannerHashTable() {
    for (int i = 0; i < TableSize; i++) {
      ScannerListNode *next;
      for (ScannerListNode *l = Table[i]; l != 0; l = next) {
        next = l->next;
        delete l;
      }
    }
    delete [] Table;
  }

  void insert(char *key) {
    int hashkey = hashFunc(key);
    Table[hashkey] = new ScannerListNode(key, Table[hashkey]);
  }

  int find(char *key) {
    for (ScannerListNode *l = Table[hashFunc(key)]; l != 0; l = l->next)
      if (!strcmp(l->key, key))
        return 1;
    return 0;
  }

  int remove(char *key) {
    ScannerListNode **prev = &Table[hashFunc(key)];
    for (ScannerListNode *l = *prev; l != 0; prev = &l->next, l = l->next)
      if (!strcmp(l->key, key)) {
        *prev = l->next;
        delete l;
        return 1;
      }
    return 0;
  }
};

static XyScannerHashTable *hashTable;


//*************************
// CONDITIONAL COMPILATION
// uses a stack of flags;
// \ifdef and \ifndef push
// \else toggles top
// \endif pops.
//*************************

#define CONDITIONALMAXDEPTH 1000

static int conditional[CONDITIONALMAXDEPTH];
static int conditional_p;       // points to top of stack
static int conditional_basep;   // points to bottom of stack

static void push_cond(int flag) {
  if (conditional_p < CONDITIONALMAXDEPTH - 1)
    conditional[++conditional_p] = flag;
  else
    xyreportError("macro directive limitation",
                  "conditionals nested too deep",
                  xyFileName,xylino,xycharno());
}

static void pop_cond() {
  if (conditional_p > conditional_basep)
    conditional_p--;
  else
    xyreportError("macro directive error",
                  "\\endif without previous corresponding \\ifdef or \\ifndef",
                  xyFileName,xylino,xycharno());
}

static int cond() {
  int i = conditional_p;
  while (i > conditional_basep)
    if (!conditional[i--])
      return 0;
  return 1;
}


//***********************
// STACK OF FILE ENTRIES
// for \insert
//***********************

class XyFileEntry {
public:
  YY_BUFFER_STATE buffer;
  OZ_Term fileNameAtom;
  int lino;
  int conditional_basep;
  XyFileEntry *previous;

  XyFileEntry(YY_BUFFER_STATE b, OZ_Term f, int l, int c, XyFileEntry *p):
      buffer(b), fileNameAtom(f), lino(l), conditional_basep(c), previous(p) {}
};

static XyFileEntry *bufferStack;

static void push_insert(FILE *filep, char *fileName) {
  bufferStack = new XyFileEntry(YY_CURRENT_BUFFER, xyFileNameAtom, xylino,
                                conditional_basep, bufferStack);
  strncpy(xyFileName, fileName, 99);
  xyFileName[99] = '\0';
  xyFileNameAtom = OZ_atom(fileName);
  xyin = filep;
  BEGIN(INITIAL);
  xy_switch_to_buffer(xy_create_buffer(xyin, YY_BUF_SIZE));
  xylino = 1;
  conditional_basep = conditional_p;
}

static int pop_insert() {
  if (conditional_p > conditional_basep)
    xyreportError("macro directive error",
                  "unterminated \\ifdef or \\ifndef",
                  xyFileName,xylino,xycharno());
  errorFlag = 0;
  if (bufferStack != NULL) {
    fclose(xyin);
    xy_switch_to_buffer(bufferStack->buffer);
    xyFileNameAtom = bufferStack->fileNameAtom;
    const char *fileName = OZ_atomToC(xyFileNameAtom);
    strncpy(xyFileName, fileName, 99);
    xyFileName[99] = '\0';
    xylino = bufferStack->lino;
    conditional_basep = bufferStack->conditional_basep;
    XyFileEntry *old = bufferStack;
    bufferStack = bufferStack->previous;
    delete old;
    return 0;
  } else
    return 1;
}


//**********
// COMMENTS
//**********

static int commentdepth;
static OZ_Term commentfile;
static int commentlino;
static int commentoffset;
static int commentlastmode;


//*********************
// FILE NAME EXPANSION
//*********************

static char *getHomeUser(char *user) {
#ifdef WINDOWS
  return NULL;
#else
  struct passwd *pwentry = getpwnam(user);
  return pwentry != NULL? pwentry->pw_dir: (char *) NULL;
#endif
}

static int isReadableFile(char *file) {
  struct stat buf;

  if (access(file, F_OK) < 0 || stat(file, &buf) < 0)
    return 0;

  return !S_ISDIR(buf.st_mode);
}

static char *checkAccess(char *file) {
  char *ret = new char[strlen(file) + 1 + 3];

  strcpy(ret, file);
  if (isReadableFile(ret))
    return ret;

  strcat(ret, ".oz");
  if (isReadableFile(ret))
    return ret;

  delete[] ret;
  return NULL;
}

static char *scExpndFileName(char *fileName, char *curfile) {
  // full pathname given?
  if (fileName[0] == '/' ||
#ifdef WINDOWS
      fileName[1] == ':' ||   // good old DOS filename like E:...
#endif
      !strncmp(fileName, "./", 2))
    return checkAccess(fileName);

  // expand "~"
  if (fileName[0] == '~') {
    char *userhome;
    int len = 0;
    if (fileName[1] == '/') {
      userhome = getenv("HOME");
      len = 2;
    } else {
      char *rest = strchr(fileName, '/');
      if (rest == NULL)
        userhome = NULL;
      else {
        len = (rest + 1) - fileName;
        rest[0] = '\0';
        userhome = getHomeUser(&fileName[1]);
        rest[0] = '/';
      }
    }
    if (userhome == NULL)
      return NULL;

    char *help = new char[strlen(userhome) + 1 + strlen(&fileName[len]) + 1];
    sprintf(help, "%s/%s", userhome, &fileName[len]);
    char *ret = checkAccess(help);
    delete[] help;

    return ret;
  }

  // search in "current" directory
  if (curfile != NULL) {
    int i = strlen(curfile);
    while (i != 0 && curfile[i - 1] != '/')   // i. e., the dir part of curfile
      i--;
    if (i != 0) {
      char *help = new char[i + strlen(fileName) + 1];
      strncpy(help, curfile, i);
      strcpy(&help[i], fileName);
      char *ret = checkAccess(help);
      delete[] help;

      if (ret != NULL)
        return ret;
    }
  }

  // search in OZPATH
  char *path = getenv("OZPATH");
  if (path == NULL)
    path = ".";

  while (path[0] != '\0') {
    int i;
    for (i = 0; path[i] != PathSeparator && path[i] != '\0'; i++);
    char *help = new char[i + 1 + strlen(fileName) + 1];
    strncpy(help, path, i);
    help[i] = '/';
    strcpy(&help[i + 1], fileName);
    char *ret = checkAccess(help);
    delete[] help;
    if (ret != NULL)
      return ret;
    if (path[i] == '\0')
      break;
    path = &path[i + 1];
  }

  return NULL;
}


//***************************
// TREATING STRINGS AND SUCH
//***************************

static void stripDot() {
  int i, j;
  for (i = 0; xytext[i] < '0' || xytext[i] > '9'; i++)
    if (xytext[i] == '\n') {
      xylino++;
      xylastline = &xytext[i + 1];
    }
  for (j = 0; xytext[i] != '\0'; xytext[j++] = xytext[i++]);
  xytext[j] = '\0';
}

static void strip(char c) {
  if (xytext[0] == c) {
    int i = 1;
    while (xytext[i] != '\0') {
      xytext[i - 1] = xytext[i];
      i++;
    }
    xytext[i - 2] = '\0';
  }
}

static void stripRegex() {
  int i = 1;
  while (xytext[i] != '\0') {
    if (xytext[i] == '\n') {
      xylino++;
      xylastline = &xytext[i + 1];
    }
    xytext[i - 1] = xytext[i];
    i++;
  }
  xytext[i - 2] = '\0';
}

static void transBody(char c, char *text, int &i, int &j) {
  int jstart = j;
  while (text[j] != c) {
    if (text[j] == '\\') {
      j++;
      switch (text[j]) {
      case 'a':
        text[i] = '\a';
        break;
      case 'b':
        text[i] = '\b';
        break;
      case 'f':
        text[i] = '\f';
        break;
      case 'n':
        text[i] = '\n';
        break;
      case 'r':
        text[i] = '\r';
        break;
      case 't':
        text[i] = '\t';
        break;
      case 'v':
        text[i] = '\v';
        break;
      case 'x':
        { char hexstring[3];
          hexstring[0] = text[++j];
          hexstring[1] = text[++j];
          hexstring[2] = '\0';
          int hexnum = (int) strtol(hexstring, NULL, 16);
          if (hexnum == 0)
            xyreportError("lexical error",
                          "character in hexadecimal notation =< 0",
                          xyFileName,xylino,xycharno() + (j - jstart));
          text[i] = hexnum;
        }
        break;
      case '\\':
        text[i] = '\\';
        break;
      case '`':
        text[i] = '`';
        break;
      case '\"':
        text[i] = '\"';
        break;
      case '\'':
        text[i] = '\'';
        break;
      default:
        { char octstring[4];
          octstring[0] = text[j++];
          octstring[1] = text[j++];
          octstring[2] = text[j];
          octstring[3] = '\0';
          int octnum = (int) strtol(octstring, NULL, 8);
          if (octnum == 0 || octnum > 255)
            xyreportError("lexical error",
                          "character in octal notation =< 0 or >= 256",
                          xyFileName,xylino,xycharno() + (j - jstart));
          text[i] = octnum & 0xFF;
        }
      }
    } else
      text[i] = text[j];
    i++;
    j++;
  }
}

static void stripTrans(char c) {
  if (xytext[0] == c) {
    int i = 0;
    int j = 1;
    transBody(c, xytext, i, j);
    xytext[i] = '\0';
  }
}

static void trans(char c) {
  if (xytext[0] == c) {
    if (!xy_systemVariables && c == '`')
      xyreportError("lexical error",
                    "use of system variables not allowed in user programs",
                    xyFileName,xylino,xycharno());
    int i = 1;
    int j = 1;
    transBody(c, xytext, i, j);
    xytext[i++] = c;
    xytext[i] = '\0';
  }
}

%}

SPACE        [? \t\r\v\f]
BLANK        [ \r\t]

ADD          "+"|"-"
FDMUL        "*"|"/"
OTHERMUL     "div"|"mod"
COMPARE      "<"|">"|"=<"|">="|"\\="
FDIN         "::"|":::"

LOWER        [a-z\337-\366\370-\377]
UPPER        [A-Z\300-\326\330-\336]
DIGIT        [0-9]
NONZERODIGIT [1-9]
ALPHANUM     {LOWER}|{UPPER}|{DIGIT}|_
CHAR         [^\\\x00]
ATOMCHAR     [^'\\\x00]
STRINGCHAR   [^\"\\\x00]
VARIABLECHAR [^`\\\x00]
ESCAPE       [abfnrtv\\'\"`]
BIN          [0-1]
OCT          [0-7]
HEX          [0-9a-fA-F]
PSEUDOCHAR   \\({OCT}{OCT}{OCT}|x{HEX}{HEX}|{ESCAPE})

ANYCHAR      {CHAR}|{PSEUDOCHAR}
OZATOM       ({LOWER}{ALPHANUM}*)|("'"({ATOMCHAR}|{PSEUDOCHAR})*"'")
VARIABLE     ({UPPER}{ALPHANUM}*)|("`"({VARIABLECHAR}|{PSEUDOCHAR})*"`")
STRING                            "\""({STRINGCHAR}|{PSEUDOCHAR})*"\""

INT          {DIGIT}+

OZINT        ~?(0{OCT}*|0(x|X){HEX}+|0(b|B){BIN}+|{NONZERODIGIT}{DIGIT}*)

FILENAME     ([-0-9a-zA-Z/_~]|\..)+|'["-~]+'

REGEXCHAR    "["([^\]\\]|\\.)+"]"|\"[^"]+\"|\\.|[^<>"\[\]\\\n]

%x COMMENT
%x IGNOREDIRECTIVE DIRECTIVE
%x LINE SWITCHDIR INPUTFILE INSERT DEFINE IFDEF IFNDEF UNDEF

%s LEX

%%

<LEX,INITIAL,DIRECTIVE>%.*     ;

<LEX,INITIAL,DIRECTIVE>"/*"    { commentdepth = 1;
                                 commentfile = xyFileNameAtom;
                                 commentlino = xylino;
                                 commentoffset = xycharno();
                                 commentlastmode = YYSTATE;
                                 BEGIN(COMMENT);
                               }

<COMMENT>"/*"                  { commentdepth++; }
<COMMENT>"*/"                  { if (--commentdepth == 0)
                                   BEGIN(commentlastmode);
                               }
<COMMENT>[^*/\n]+              ;
<COMMENT>\n                    { xylino++;
                                 xylastline = xytext + 1;
                               }
<COMMENT>.                     ;
<COMMENT><<EOF>>               { if (cond()) {
                                   const char *file = OZ_atomToC(commentfile);
                                   xyreportError("lexical error",
                                                 "unterminated comment",
                                                 file,commentlino,commentoffset);
                                 }
                                 BEGIN(DIRECTIVE);
                                 if (pop_insert())
                                   return ENDOFFILE;
                               }


\\he(lp?)?                     { BEGIN(DIRECTIVE); return HELP; }
\\l(i(ne?)?)?                  { if (cond()) BEGIN(LINE); }
\\s(w(i(t(ch?)?)?)?)?          { BEGIN(SWITCHDIR); return SWITCH; }
\\sh(o(w(Switches)?)?)?        { BEGIN(DIRECTIVE); return SHOWSWITCHES; }
\\pu(s(h(Switches)?)?)?        { BEGIN(DIRECTIVE); return PUSHSWITCHES; }
\\po(p(Switches)?)?            { BEGIN(DIRECTIVE); return POPSWITCHES; }
\\f(e(ed?)?)?                  { BEGIN(INPUTFILE); return FEED; }
\\threadedfeed                 { BEGIN(INPUTFILE); return THREADEDFEED; }
\\c(o(re?)?)?                  { BEGIN(INPUTFILE); return CORE; }
\\m(a(c(h(i(ne?)?)?)?)?)?      { BEGIN(INPUTFILE); return OZMACHINE; }

\\in(s(e(rt?)?)?)?             { BEGIN(INSERT); }
\\d(e(f(i(ne?)?)?)?)?          { BEGIN(DEFINE); }
\\ifd(ef?)?                    { BEGIN(IFDEF); }
\\ifn(d(ef?)?)?                { BEGIN(IFNDEF); }
\\el(se?)?                     { if (conditional_p > conditional_basep) {
                                   // toggle top of flag stack
                                   if (conditional[conditional_p])
                                     conditional[conditional_p] = 0;
                                   else
                                     conditional[conditional_p] = 1;
                                 } else
                                   xyreportError("macro directive error",
                                                 "\\endif without previous corresponding \\ifdef or \\ifndef",
                                                 xyFileName,xylino,xycharno());
                                 BEGIN(DIRECTIVE);
                               }
\\e(n(d(if?)?)?)?              { pop_cond();
                                 BEGIN(DIRECTIVE);
                               }
\\u(n(d(ef?)?)?)?              { BEGIN(UNDEF);}

<IGNOREDIRECTIVE>{
  {BLANK}                      ;
  .                            ;
  \n                           { BEGIN(INITIAL);
                                 xylino++;
                                 xylastline = xytext + 1;
                               }
  <<EOF>>                      { BEGIN(DIRECTIVE);
                                 if (pop_insert())
                                   return ENDOFFILE;
                               }
}
<DIRECTIVE>{
  {BLANK}                      ;
  .                            { errorFlag = 1; }
  \n                           { if (errorFlag) {
                                   xyreportError("directive error",
                                                 "illegal directive syntax",
                                                 xyFileName,xylino,xycharno());
                                   errorFlag = 0;
                                 }
                                 xylino++;
                                 xylastline = xytext + 1;
                                 BEGIN(INITIAL);
                               }
  <<EOF>>                      { BEGIN(DIRECTIVE);
                                 if (pop_insert())
                                   return ENDOFFILE;
                               }
}
<LINE>{
  [0-9]+                       { xylino = atol(xytext) - 1; }
  {FILENAME}                   { strip('\'');
                                 char *fullname = scExpndFileName(xytext,xyFileName);
                                 if (fullname != NULL) {
                                   strncpy(xyFileName, fullname, 99);
                                   delete[] fullname;
                                 } else
                                   strncpy(xyFileName, xytext, 99);
                                 xyFileName[99] = '\0';
                                 xyFileNameAtom = OZ_atom(xyFileName);
                                 BEGIN(DIRECTIVE);
                               }
  {BLANK}                      ;
  .                            { errorFlag = 1; }
  \n                           { if (errorFlag) {
                                   xyreportError("directive error",
                                                 "illegal directive syntax",
                                                 xyFileName,xylino,xycharno());
                                   errorFlag = 0;
                                 }
                                 xylino++;
                                 xylastline = xytext + 1;
                                 BEGIN(INITIAL);
                               }
  <<EOF>>                      { xyreportError("directive error",
                                               "unterminated directive",
                                               xyFileName,xylino,xycharno());
                                 BEGIN(DIRECTIVE);
                                 if (pop_insert())
                                   return ENDOFFILE;
                               }
}
<SWITCHDIR>{
  "+"                          { return '+'; }
  "-"                          { return '-'; }
  [A-Za-z0-9]+                 { return SWITCHNAME; }
  {BLANK}                      ;
  .                            { errorFlag = 1; }
  \n                           { if (errorFlag) {
                                   xyreportError("directive error",
                                                 "illegal directive syntax",
                                                 xyFileName,xylino,xycharno());
                                   errorFlag = 0;
                                 }
                                 xylino++;
                                 xylastline = xytext + 1;
                                 BEGIN(INITIAL);
                               }
  <<EOF>>                      { BEGIN(DIRECTIVE);
                                 if (pop_insert())
                                   return ENDOFFILE;
                               }
}
<INPUTFILE>{
  {FILENAME}                   { if (cond()) {
                                   strip('\'');
                                   char *help = scExpndFileName(xytext,xyFileName);
                                   if (help != NULL) {
                                     strncpy(xyhelpFileName, help, 99);
                                     delete[] help;
                                   } else
                                     strncpy(xyhelpFileName, xytext, 99);
                                   xyhelpFileName[99] = '\0';
                                   BEGIN(DIRECTIVE);
                                   return FILENAME;
                                 } else
                                   BEGIN(DIRECTIVE);
                               }
  {BLANK}                      ;
  .                            { errorFlag = 1; }
  \n                           { if (errorFlag) {
                                   xyreportError("directive error",
                                                 "illegal directive syntax",
                                                 xyFileName,xylino,xycharno());
                                   errorFlag = 0;
                                 }
                                 xylino++;
                                 xylastline = xytext + 1;
                                 BEGIN(INITIAL);
                               }
  <<EOF>>                      { xyreportError("directive error",
                                               "unterminated directive",
                                               xyFileName,xylino,xycharno());
                                 BEGIN(DIRECTIVE);
                                 if (pop_insert())
                                   return ENDOFFILE;
                               }
}

<INSERT>{
  {FILENAME}                   { if (cond()) {
                                   strip('\'');
                                   char *fullname = scExpndFileName(xytext,xyFileName);
                                   if (fullname != NULL) {
                                     if (xy_showInsert) {
                                       char *s = new char[27+strlen(fullname)];
                                       sprintf(s, "%%%%%%     inserting file \"%s\"\n",fullname);
                                       xy_errorMessages = OZ_pair2(xy_errorMessages,OZ_string(s));
                                       delete[] s;
                                       fflush(stdout);
                                     }
                                     FILE *filep = fopen(fullname, "r");
                                     push_insert(filep, fullname);
                                     delete[] fullname;
                                     BEGIN(INITIAL);
                                   } else {
                                     xyreportError("macro directive error",
                                                   "could not open file to insert",
                                                   xyFileName,xylino,xycharno());
                                     BEGIN(DIRECTIVE);
                                   }
                                 } else
                                   BEGIN(INITIAL);
                               }
  {BLANK}                      ;
  .                            { errorFlag = 1; }
  \n                           { if (errorFlag) {
                                   xyreportError("directive error",
                                                 "illegal directive syntax",
                                                 xyFileName,xylino,xycharno());
                                   errorFlag = 0;
                                 }
                                 xylino++;
                                 xylastline = xytext + 1;
                                 BEGIN(INITIAL);
                               }
  <<EOF>>                      { xyreportError("directive error",
                                               "unterminated directive",
                                               xyFileName,xylino,xycharno());
                                 BEGIN(DIRECTIVE);
                                 if (pop_insert())
                                   return ENDOFFILE;
                               }
}

<DEFINE>{
  {VARIABLE}                   { if (cond()) {
                                   if (!hashTable->find(xytext))
                                     hashTable->insert(xytext);
                                   BEGIN(DIRECTIVE);
                                 } else
                                   BEGIN(INITIAL);
                               }
  {BLANK}                      ;
  .                            { errorFlag = 1; }
  \n                           { if (errorFlag) {
                                   xyreportError("directive error",
                                                 "illegal directive syntax",
                                                 xyFileName,xylino,xycharno());
                                   errorFlag = 0;
                                 }
                                 xylino++;
                                 xylastline = xytext + 1;
                                 BEGIN(INITIAL);
                               }
  <<EOF>>                      { xyreportError("directive error",
                                               "unterminated directive",
                                               xyFileName,xylino,xycharno());
                                 BEGIN(DIRECTIVE);
                                 if (pop_insert())
                                   return ENDOFFILE;
                               }
}
<UNDEF>{
  {VARIABLE}                   { if (cond()) {
                                   hashTable->remove(xytext);
                                   BEGIN(DIRECTIVE);
                                 } else
                                   BEGIN(INITIAL);
                               }
  {BLANK}                      ;
  .                            { errorFlag = 1; }
  \n                           { if (errorFlag) {
                                   xyreportError("directive error",
                                                 "illegal directive syntax",
                                                 xyFileName,xylino,xycharno());
                                   errorFlag = 0;
                                 }
                                 xylino++;
                                 xylastline = xytext + 1;
                                 BEGIN(INITIAL);
                               }
  <<EOF>>                      { xyreportError("directive error",
                                               "unterminated directive",
                                               xyFileName,xylino,xycharno());
                                 BEGIN(DIRECTIVE);
                                 if (pop_insert())
                                   return ENDOFFILE;
                               }
}
<IFDEF>{
  {VARIABLE}                   { if (hashTable->find(xytext))
                                   push_cond(1);
                                 else
                                   push_cond(0);
                                 BEGIN(DIRECTIVE);
                               }
  {BLANK}                      ;
  .                            { errorFlag = 1; }
  \n                           { if (errorFlag) {
                                   xyreportError("directive error",
                                                 "illegal directive syntax",
                                                 xyFileName,xylino,xycharno());
                                   errorFlag = 0;
                                 }
                                 xylino++;
                                 xylastline = xytext + 1;
                                 BEGIN(INITIAL);
                               }
  <<EOF>>                      { xyreportError("directive error",
                                               "unterminated directive",
                                               xyFileName,xylino,xycharno());
                                 BEGIN(DIRECTIVE);
                                 if (pop_insert())
                                   return ENDOFFILE;
                               }
}
<IFNDEF>{
  {VARIABLE}                   { if (hashTable->find(xytext))
                                   push_cond(0);
                                 else
                                   push_cond(1);
                                 BEGIN(DIRECTIVE);
                               }
  {BLANK}                      ;
  .                            { errorFlag = 1; }
  \n                           { if (errorFlag) {
                                   xyreportError("directive error",
                                                 "illegal directive syntax",
                                                 xyFileName,xylino,xycharno());
                                   errorFlag = 0;
                                 }
                                 xylino++;
                                 xylastline = xytext + 1;
                                 BEGIN(INITIAL);
                               }
  <<EOF>>                      { xyreportError("directive error",
                                               "unterminated directive",
                                               xyFileName,xylino,xycharno());
                                 BEGIN(DIRECTIVE);
                                 if (pop_insert())
                                   return ENDOFFILE;
                               }
}

<LEX>"<<EOF>>"                 { BEGIN(INITIAL); return REGEX; }
<LEX>"<"{REGEXCHAR}+">"        { BEGIN(INITIAL); stripRegex(); return REGEX; }

"[]"                           { return CHOICE; }
"..."                          { return LDOTS; }
"<-"                           { return ASSIGN; }
"<="                           { return DEFAULT; }
"=>"                           { return REDUCE; }
"//"                           { return SEP; }
{ADD}                          { return ADD; }
{FDMUL}                        { return FDMUL; }
{OTHERMUL}                     { return OTHERMUL; }
{OTHERMUL}/\(                  { return OTHERMUL; }
"=="|{COMPARE}                 { return COMPARE; }
{FDIN}                         { return FDIN; }
("="|{COMPARE})":"             { return FDCOMPARE; }

"."({SPACE}|\n)*[0-9]+         { // Hack to avoid strange parsing of X.1.1:
                                 // If "." is followed by integer, then
                                 // a special token is returned.
                                 // If this rule would not be there, the
                                 // resulting tokens would be "X" "." "1.1",
                                 // where the last one is a float.
                                 // Caveat: Comments are not allowed
                                 //         between . and number.
                                 stripDot(); return DOTINT; }

{INT}/\.\.\.                   { // Hack to avoid parsing error for a(b:1...):
                                 // If int is followed by ..., int is returned.
                                 // If this rule would not be there, the rule
                                 // for floats would match and an error would
                                 // occur.
                                 return OZINT; }

{OZINT}                        { return OZINT; }

~?{INT}\.{DIGIT}*((e|E)~?{INT})? { return OZFLOAT; }

"unit"/\(                      { return UNIT_LABEL; }
"true"/\(                      { return TRUE_LABEL; }
"false"/\(                     { return FALSE_LABEL; }

"andthen"                      { return andthen; }
"andthen"/\(                   { return andthen; }
"attr"                         { return attr; }
"attr"/\(                      { return attr; }
"case"                         { return _case_; }
"case"/\(                      { return _case_; }
"catch"                        { return catch; }
"catch"/\(                     { return catch; }
"choice"                       { return choice; }
"choice"/\(                    { return choice; }
"class"                        { return _class_; }
"class"/\(                     { return _class_; }
"condis"                       { return _condis_; }
"condis"/\(                    { return _condis_; }
"declare"                      { return declare; }
"declare"/\(                   { return declare; }
"dis"                          { return dis; }
"dis"/\(                       { return dis; }
"else"                         { return _else_; }
"else"/\(                      { return _else_; }
"elsecase"                     { return elsecase; }
"elsecase"/\(                  { return elsecase; }
"elseif"                       { return elseif; }
"elseif"/\(                    { return elseif; }
"elseof"                       { return elseof; }
"elseof"/\(                    { return elseof; }
"end"                          { return end; }
"end"/\(                       { return end; }
"fail"                         { return fail; }
"fail"/\(                      { return fail; }
"false"                        { return false; }
"feat"                         { return feat; }
"feat"/\(                      { return feat; }
"finally"                      { return finally; }
"finally"/\(                   { return finally; }
"from"                         { return _from_; }
"from"/\(                      { return _from_; }
"fun"                          { return _fun_; }
"fun"/\(                       { return _fun_; }
"functor"                      { return functor; }
"functor"/\(                   { return functor; }
"if"                           { return _if_; }
"if"/\(                        { return _if_; }
"in"                           { return _in_; }
"in"/\(                        { return _in_; }
"lex"                          { if (xy_gumpSyntax) { BEGIN(LEX); return lex; } else return OZATOM; }
"lex"/\(                       { if (xy_gumpSyntax) { BEGIN(LEX); return lex; } else return ATOM_LABEL; }
"local"                        { return local; }
"local"/\(                     { return local; }
"lock"                         { return _lock_; }
"lock"/\(                      { return _lock_; }
"meth"                         { return _meth_; }
"meth"/\(                      { return _meth_; }
"mode"                         { return xy_gumpSyntax? _mode_: OZATOM; }
"mode"/\(                      { return xy_gumpSyntax? _mode_: ATOM_LABEL; }
"not"                          { return not; }
"not"/\(                       { return not; }
"of"                           { return of; }
"of"/\(                        { return of; }
"or"                           { return or; }
"or"/\(                        { return or; }
"orelse"                       { return orelse; }
"orelse"/\(                    { return orelse; }
"parser"                       { return xy_gumpSyntax? _parser_: OZATOM; }
"parser"/\(                    { return xy_gumpSyntax? _parser_: ATOM_LABEL; }
"proc"                         { return proc; }
"proc"/\(                      { return proc; }
"prod"                         { return xy_gumpSyntax? prod: OZATOM; }
"prod"/\(                      { return xy_gumpSyntax? prod: ATOM_LABEL; }
"prop"                         { return prop; }
"prop"/\(                      { return prop; }
"raise"                        { return ozraise; }
"raise"/\(                     { return ozraise; }
"scanner"                      { return xy_gumpSyntax? _scanner_: OZATOM; }
"scanner"/\(                   { return xy_gumpSyntax? _scanner_: ATOM_LABEL; }
"self"                         { return self; }
"self"/\(                      { return self; }
"skip"                         { return skip; }
"skip"/\(                      { return skip; }
"syn"                          { return xy_gumpSyntax? syn: OZATOM; }
"syn"/\(                       { return xy_gumpSyntax? syn: ATOM_LABEL; }
"then"                         { return then; }
"then"/\(                      { return then; }
"token"                        { return xy_gumpSyntax? token: OZATOM; }
"token"/\(                     { return xy_gumpSyntax? token: ATOM_LABEL; }
"thread"                       { return thread; }
"thread"/\(                    { return thread; }
"true"                         { return true; }
"try"                          { return try; }
"try"/\(                       { return try; }
"unit"                         { return unit; }
"with"                         { return with; }
"with"/\(                      { return with; }

{OZATOM}                       { stripTrans('\''); return OZATOM; }
"'"[^']*"'"                    { xyreportError("lexical error","illegal atom syntax",xyFileName,xylino,xycharno()); return OZATOM;}
{OZATOM}/\(                    { stripTrans('\''); return ATOM_LABEL; }
"'"[^']*"'"/\(                 { xyreportError("lexical error","illegal atom syntax",xyFileName,xylino,xycharno()); return ATOM_LABEL;}
{VARIABLE}                     { trans('`'); return VARIABLE; }
"`"[^`]*"`"                    { xyreportError("lexical error","illegal variable syntax",xyFileName,xylino,xycharno()); return VARIABLE;}
{VARIABLE}/\(                  { trans('`'); return VARIABLE_LABEL; }
"`"[^`]*"`"/\(                 { xyreportError("lexical error","illegal variable syntax",xyFileName,xylino,xycharno()); return VARIABLE;}
{STRING}                       { stripTrans('\"'); return STRING; }
"\""[^\"]*"\""                 { xyreportError("lexical error","illegal string syntax",xyFileName,xylino,xycharno()); return STRING;}

"&"{ANYCHAR}                   { int i = 0;
                                 int j = 1;
                                 transBody(0, xytext, i, j);
                                 return AMPER;
                               }

"{"|"}"|"("|")"|"["|"]"|"|"|"#"|":"|"="|"."|"^"|"@"|"$"|"!"|"~"|"_"|"," {
                                 return xytext[0];
                               }

{SPACE}                        ;
\n                             { xylino++;
                                 xylastline = xytext + 1;
                               }

\\[a-zA-Z]+                    { xyreportError("lexical error",
                                               "unknown directive",
                                               xyFileName,xylino,xycharno());
                                 BEGIN(IGNOREDIRECTIVE);
                               }

.                              { xyreportError("lexical error",
                                               "illegal character",
                                               xyFileName,xylino,xycharno());
                               }

<<EOF>>                        { BEGIN(DIRECTIVE);
                                 if (pop_insert())
                                   return ENDOFFILE;
                               }

%%

static void xy_init(OZ_Term defines) {
  xylino = 1;
  errorFlag = 0;

  bufferStack = NULL;

  hashTable = new XyScannerHashTable;
  char version[strlen(OZVERSION)+3];
  strcpy(version,"Oz_");
  strcat(version+3,OZVERSION);
  int count = 0;
  for (char *s = version+3; *s; s++)
    if (*s == '.') {
      count++;
      *s = '_';
    }
  hashTable->insert(version);
  while (count >= 1) {
    *strrchr(version,'_') = '\0';
    hashTable->insert(version);
    count--;
  }
  hashTable->insert("NEWCOMPILER");
  while (OZ_isCons(defines)) {
    char *x = OZ_virtualStringToC(OZ_head(defines));
    hashTable->insert(x);
    defines = OZ_tail(defines);
  }

  conditional_p = 0;
  conditional_basep = 0;
  commentdepth = 0;

  BEGIN(INITIAL);
}

int xy_init_from_file(char *file, OZ_Term defines) {
  char *fullname = scExpndFileName(file, NULL);
  if (fullname == NULL)
    return 0;
  xyin = fopen(fullname, "r");
  if (xyin == NULL)
    return 0;
  xy_switch_to_buffer(xy_create_buffer(xyin, YY_BUF_SIZE));
  strncpy(xyFileName,fullname,99);
  xyFileName[99] = '\0';
  xyFileNameAtom = OZ_atom(xyFileName);
  delete[] fullname;
  xy_init(defines);
  return 1;
}

void xy_init_from_string(char *str, OZ_Term defines) {
  strcpy(xyFileName,"nofile");
  xyFileNameAtom = OZ_atom(xyFileName);
  xyin = NULL;
  xy_scan_string(str);
  xylastline = YY_CURRENT_BUFFER->yy_ch_buf;
  xy_init(defines);
}

char *xy_expand_file_name(char *file) {
  return scExpndFileName(file, NULL);
}

void xy_exit() {
  xy_delete_buffer(YY_CURRENT_BUFFER);
  delete hashTable;
  while (bufferStack != NULL) {
    XyFileEntry *old = bufferStack;
    bufferStack = bufferStack->previous;
    delete old;
  }
  if (xyin)
    fclose(xyin);
}

int xylex() {
  int next = xymylex();
  if (cond() || next == 0 || next == ENDOFFILE)
    return next;
  else
    return xylex();
}
