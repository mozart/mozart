/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */

#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include "ozstrstream.h"

#include "iso-ctype.hh"

#include "oz.h"

#include "am.hh"

#include "genvar.hh"
#include "ofgenvar.hh"

#include "../include/gmp.h"

/* ------------------------------------------------------------------------ *
 * tests
 * ------------------------------------------------------------------------ */

OZ_Term OZ_deref(OZ_Term term)
{
  return deref(term);
}

int OZ_isAtom(OZ_Term term)
{
  term = deref(term);
  return isAtom(term);
}

int OZ_isCell(OZ_Term term)
{
  term = deref(term);
  return isCell(term);
}

int OZ_isPort(OZ_Term term)
{
  term = deref(term);
  return isPort(term);
}

int OZ_isChunk(OZ_Term term)
{
  term = deref(term);
  return isChunk(term);
}

int OZ_isCons(OZ_Term term)
{
  term = deref(term);
  return isCons(term);
}

int OZ_isFloat(OZ_Term term)
{
  term = deref(term);
  return isFloat(term);
}

int OZ_isInt(OZ_Term term)
{
  term = deref(term);
  return isInt(term);
}

int OZ_isSmallInt(OZ_Term term)
{
  term = deref(term);
  return isSmallInt(term);
}

int OZ_isBigInt(OZ_Term term)
{
  term = deref(term);
  return isBigInt(term);
}

int OZ_isProcedure(OZ_Term term)
{
  term = deref(term);
  return isProcedure(term);
}


int OZ_isLiteral(OZ_Term term)
{
  term = deref(term);
  return isLiteral(term);
}

int OZ_isFeature(OZ_Term term)
{
  term = deref(term);
  return isFeature(term);
}

int OZ_isName(OZ_Term term)
{
  term = deref(term);
  return isAtom(term);
}

int OZ_isNil(OZ_Term term)
{
  term = deref(term);
  return isNil(term);
}

int OZ_isObject(OZ_Term term)
{
  term = deref(term);
  return isObject(term);
}

int OZ_isThread(OZ_Term t)
{
  t = deref(t);
  return isThread(t);
}


/*
 * list checking
 *   checkChar:
 *     0 = any list
 *     1 = list of char
 *     2 = list of char != 0
 */

inline
int isList(OZ_Term l, OZ_Term *var, int checkChar)
{
  while (1) {
    DEREF(l,lPtr,lTag);
    if (isAnyVar(lTag)) {
      if (var) *var=makeTaggedRef(lPtr);
      return 0;
    }

    if (isCons(lTag)) {
      if (checkChar) {
	OZ_Term h = head(l);
	DEREF(h,hPtr,hTag);
	if (isAnyVar(hTag)) {
	  if (var) *var=makeTaggedRef(hPtr);
	  return 0;
	}
	if (!isSmallInt(hTag)) return 0;
	int i=smallIntValue(h);
	if (i<0 || i>255) return 0;
	if (checkChar>1 && i==0) return 0;
      }
      l = tail(l);
    } else if (isNil(l)) {
      return 1;
    } else {
      return 0;
    }
  }
}

int OZ_isString(OZ_Term term,OZ_Term *var)
{
  if (var) *var = 0;
  return isList(term,var,1);
}

int OZ_isProperString(OZ_Term term,OZ_Term *var)
{
  if (var) *var = 0;
  return isList(term,var,2);
}

int OZ_isList(OZ_Term term,OZ_Term *var)
{
  if (var) *var = 0;
  return isList(term,var,0);
}

int OZ_isTrue(OZ_Term term)
{
  term = deref(term);
  return literalEq(term,NameTrue);
}

int OZ_isFalse(OZ_Term term)
{
  term = deref(term);
  return literalEq(term,NameFalse);
}

inline
int isPair(OZ_Term term)
{
  if (isLiteral(term)) return literalEq(term,AtomPair);
  if (!isSRecord(term)) return 0;
  SRecord *sr = tagged2SRecord(term);
  if (!sr->isTuple()) return 0;
  return literalEq(sr->getLabel(),AtomPair);
}

int OZ_isPair(OZ_Term term)
{
  term = deref(term);
  return isPair(term);
}

int OZ_isPair2(OZ_Term term)
{
  term = deref(term);
  if (isLiteral(term)) return literalEq(term,AtomPair);
  if (!isSRecord(term)) return 0;
  SRecord *sr = tagged2SRecord(term);
  if (!sr->isTuple()) return 0;
  if (!literalEq(sr->getLabel(),AtomPair)) return 0;
  return sr->getWidth()==2;
}

int OZ_isRecord(OZ_Term term)
{
  term = deref(term);
  return isRecord(term);
}

int OZ_isTuple(OZ_Term term)
{
  term = deref(term);
  return isTuple(term);
}

int OZ_isValue(OZ_Term term)
{
  term = deref(term);
  return !isAnyVar(term);
}

int OZ_isVariable(OZ_Term term)
{
  term = deref(term);
  return isAnyVar(term);
}

OZ_Term OZ_termType(OZ_Term term)
{
  term = deref(term);

  if (isThread(term)) {
    return OZ_atom("thread");
  }
  
  if (isAnyVar(term)) {
    return OZ_atom("variable");
  }

  if (isInt(term)) {
    return OZ_atom("int");
  }
    
  if (isFloat(term)) {
    return OZ_atom("float");
  }
    
  if (isLiteral(term)) {
    return OZ_atom(tagged2Literal(term)->isAtom() ? "atom" : "name");
  }

  if (isTuple(term)) {
    return OZ_atom("tuple");
  }
    
  if (isProcedure(term)) {
    return OZ_atom("procedure");
  }
    
  if (isCell(term)) {
    return OZ_atom("cell");
  }

  if (isChunk(term)) {
    return OZ_atom("chunk");
  }
    
  if (isSRecord(term)) {
    return OZ_atom("record");
  }

  if (isSpace(term)) {
    return OZ_atom("space");
  }

  OZ_warning("OZ_termType: unknown type\n");
  return 0;
}

/* -----------------------------------------------------------------
 * providing constants
 *------------------------------------------------------------------*/

int OZ_getLowPrio(void) 
{
  return OZMIN_PRIORITY;
}

int OZ_getMediumPrio(void) 
{
  return DEFAULT_PRIORITY;
}

int OZ_getHighPrio(void) 
{
  return OZMAX_PRIORITY;
}

int OZ_smallIntMin(void) 
{
  return OzMinInt;
}

int OZ_smallIntMax(void) 
{
  return OzMaxInt;
}

OZ_Term OZ_false(void)
{
  return NameFalse;
}

OZ_Term OZ_true(void)
{
  return NameTrue;
}

/* -----------------------------------------------------------------
 * convert: C from/to Oz datastructure
 * -----------------------------------------------------------------*/

/*
 * Ints
 */

OZ_Term OZ_int(int i)
{
  return makeInt(i);
}

/*
 * BigInt: return INT_MAX/INT_MIN if too big
 */
int OZ_intToC(OZ_Term term)
{
  term = deref(term);
  if (isSmallInt(term)) {
    return smallIntValue(term);
  }

  return tagged2BigInt(term)->getInt();
}

OZ_Term OZ_CStringToInt(char *str)
{
  if (!str || str[0] == '\0') {
    OZ_warning("OZ_CStringToInt: empty string");
    return 0;
  }

  char *aux = str;
  int sign = 1;
  if (aux[0] == '~') {
    aux++;
    sign = -1;
  }

  MP_INT theInt;
  mpz_init(&theInt);
  if (aux[0] == '0') {
    switch (aux[1]) {
    case '\0':
      return newSmallInt(0);
    case 'x': case 'X':
      if (aux[2] == '\0' || mpz_set_str(&theInt, &aux[2], 16) == -1) {
	mpz_clear(&theInt);
	return 0;
      }
      break;
    case 'b': case 'B':
      if (aux[2] == '\0' || mpz_set_str(&theInt, &aux[2], 2) == -1) {
	mpz_clear(&theInt);
	return 0;
      }
      break;
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
      if (mpz_set_str(&theInt, &aux[1], 8) == -1) {
	mpz_clear(&theInt);
	return 0;
      }
      break;
    default:
      mpz_clear(&theInt);
      return 0;
    }
  } else if (aux[0] == '\0' || mpz_set_str(&theInt, aux, 10) == -1) {
    mpz_clear(&theInt);
    return 0;
  }

  BigInt *theBigInt = new BigInt(&theInt);
  mpz_clear(&theInt);
  if (sign > 0)
    return theBigInt->shrink();
  else
    return theBigInt->neg();
}

/*
 * parse: [~]<digit>+.<digit>*[(e|E)[~]<digit>+]
 */
char *OZ_parseFloat(char *s) {
  char *p = OZ_parseInt(s);
  if (!p || *p++ != '.') {
    return NULL;
  }
  while (iso_isdigit((unsigned char) *p)) {
    p++;
  }
  switch (*p) {
  case 'e':
  case 'E':
    p++;
    break;
  default:
    return p;
  }
  return OZ_parseInt(p);
}

/*
 * parse: [~]<digit>+
 */
char *OZ_parseInt(char *s)
{
  char *p = s;
  if (*p == '~') {
    p++;
  }
  if (!iso_isdigit((unsigned char) *p++)) {
    return 0;
  }
  while (iso_isdigit((unsigned char) *p)) {
    p++;
  }
  return p;
}


/*
 * Floats
 */

OZ_Term OZ_float(double i)
{
  return makeTaggedFloat(i);
}

double OZ_floatToC(OZ_Term term)
{
  term = deref(term);
  return floatValue(term);
}

/*
 * PRE: OZ_parseFloat succesful
 * see strtod(3)
 */
OZ_Term OZ_CStringToFloat(char *s)
{
  replChar(s,'~','-');

  char *end;
  double res = strtod(s,&end);

  // undo changes
  replChar(s,'-','~');

  return OZ_float(res);
}


/*
 * Numbers
 */

OZ_Term OZ_CStringToNumber(char *s)
{
  if (strchr(s, '.') != NULL) {
    return OZ_CStringToFloat(s);
  }
  return OZ_CStringToInt(s);
}


/*
 * Features
 */

int OZ_featureCmp(OZ_Term term1, OZ_Term term2)
{
  term1 = deref(term1);
  term2 = deref(term2);
  return featureCmp(term1,term2);
}


/*
 * PRINTING
 */

inline
void int2buffer(ostream &out, OZ_Term term)
{
  if (isSmallInt(term)) {
    int i = smallIntValue(term);
    if (i < 0) {
      out << '~' << -i;
    } else {
      out << i;
    }
  } else {
    BigInt *bb=tagged2BigInt(term);
    char *str = new char[bb->stringLength()+1];
    bb->getString(str);
    if (*str == '-') *str = '~';
    out << str;
    delete str;
  }
}

inline
void float2buffer(ostream &out, OZ_Term term)
{
  double f = floatValue(term);
  ostrstream tmp;
  tmp << f << ends;
  char *str = tmp.str();

  // normalize float
  Bool hasDot = NO;
  Bool hasDigits = NO;
  char *s = str;
  for (char c=*s++; c ; c=*s++) {
    switch (c) {
    case 'e':
      if (!hasDot) out << '.' << '0';
      hasDot = OK;
      out << c;
      break;
    case '.':
      if (!hasDigits) out << '0';
      hasDot = OK;
      out << c;
      break;
    case '-':
      out << '~';
      break;
    case '+':
      break;
    default:
      if (!iso_isdigit((unsigned char) c)) hasDot=OK;
      hasDigits=OK;
      out << c;
      break;
    }
  }
  if (!hasDot) out << '.' << '0';
  delete str;
}

inline
void octOut(ostream &out,unsigned char c)
{
  out << (char) (((c >> 6) & '\007') + '0')
      << (char) (((c >> 3) & '\007') + '0')
      << (char) (( c       & '\007') + '0');
}


inline
void atomq2buffer(ostream &out, char *s)
{
  unsigned char c;
  while ((c = *s)) {
    if (iso_iscntrl(c)) {
      out << '\\';
      switch (c) {
      case '\'':
	out << '\'';
	break;
      case '\a':
	out << 'a';
	break;
      case '\b':
	out << 'b';
	break;
      case '\f':
	out << 'f';
	break;
      case '\n':
	out << 'n';
	break;
      case '\r':
	out << 'r';
	break;
      case '\t':
	out << 't';
	break;
      case '\v':
	out << 'v';
	break;
      default:
	octOut(out,c);
	break;
      }
    } else if (!iso_isprint(c)) {
      out << '\\';
      octOut(out,c);
    } else {
      switch (c) {
      case '\'':
	out << '\\' << '\'';
	break;
      case '\\':
	out << '\\' << '\\';
	break;
      default:
	out << c;
	break;
      }
    }
    s++;
  }
}


inline
Bool checkAtom(char *s)
{
  char *t = s;
  unsigned char c = *s++;
  if (!c || !iso_islower(c)) {
    return NO;
  }
  c=*s++;
  while (c) {
    if (!iso_isalnum(c) && c != '_') {
      return NO;
    }
    c=*s++;
  }
  switch (t[0]) {
  case 'a':
    return strcmp(t, "andthen") && strcmp(t, "attr")? OK: NO;
  case 'c':
    return strcmp(t, "case") && strcmp(t, "catch")
	&& strcmp(t, "choice") && strcmp(t, "class")
	&& strcmp(t, "condis") ? OK: NO;
  case 'd':
    return strcmp(t, "declare") && strcmp(t, "dis")
	&& strcmp(t, "div")? OK: NO;
  case 'e':
    return strcmp(t, "else") && strcmp(t, "elsecase")
	&& strcmp(t, "elseif") && strcmp(t, "elseof")
	&& strcmp(t, "end")? OK: NO;
  case 'f':
    return strcmp(t, "false") && strcmp(t, "feat")
	&& strcmp(t, "finally") && strcmp(t, "from")
	&& strcmp(t, "fun") && strcmp(t, "fail")? OK: NO;
  case 'i':
    return strcmp(t, "if") && strcmp(t, "in")? OK: NO;
  case 'l':
    return strcmp(t, "local") && strcmp(t, "lock")? OK: NO;
  case 'm':
    return strcmp(t, "meth") && strcmp(t, "mod")? OK: NO;
  case 'n':
    return strcmp(t, "not")? OK: NO;
  case 'o':
    return strcmp(t, "of") && strcmp(t, "or")
	&& strcmp(t, "orelse")? OK: NO;
  case 'p':
    return strcmp(t, "proc") && strcmp(t, "prop")? OK: NO;
  case 'r':
    return strcmp(t, "raise")? OK: NO;
  case 's':
    return strcmp(t, "self") && strcmp(t, "skip")? OK: NO;
  case 't':
    return strcmp(t, "then") && strcmp(t, "thread")
        && strcmp(t, "true")
	&& strcmp(t, "try")? OK: NO;
  case 'u':
    return strcmp(t, "unit")? OK: NO;
  case 'w':
    return strcmp(t, "with")? OK: NO;
  default:
    return OK;
  }
}

inline
void atom2buffer(ostream &out, Literal *a)
{
  char *s = a->getPrintName();
  if (checkAtom(s)) {
    out << s;
  } else {
    out << '\'';
    atomq2buffer(out,s);
    out << '\'';
  }
}

inline
void name2buffer(ostream &out, Literal *a) {
  char *s = a->getPrintName();

  if (literalEq(makeTaggedLiteral(a),NameTrue))  {
    out << "true";
  } else if (literalEq(makeTaggedLiteral(a),NameFalse)) {
    out << "false";
  } else if (literalEq(makeTaggedLiteral(a),NameUnit)) {
    out << "unit";
  } else if (!*s) {
    out << "<N>";
  } else {
    out << "<N: " << s << '>';
  }
}

inline
void const2buffer(ostream &out, ConstTerm *c)
{
  char *s = c->getPrintName();
  int arity = c->getArity();

  switch (c->getType()) {
  case Co_Thread:
    out << "<Thread>";
    break; 
  case Co_Abstraction:
  case Co_Builtin:
    {
      out << "<P/" << arity;
      if (*s != 0) {
	out << ' ' << s;
      }
      out << '>';
    }
    break;
  case Co_Cell:
    out << "<Cell>";
    break;
  case Co_Port:
    out << "<Port>";
    break;
  case Co_Space:
    out << "<Space>";
    break;
  case Co_Object:
    {
      Object *o = (Object *) c;
      if (*s == '_' && *(s+1) == 0) {
	if (o->isClass()) {
	  out << "<C>";
	} else {
	  out << "<O>";
	}
      } else {
	if (o->isClass()) {
	  out << "<C: " << s << '>';
	} else {
	  out << "<O: " << s << '>';
	}
      }
    }
    break;
  default:
    if (c->isChunk()) {
      out << "<Chunk>";
    } else {
      out << "<UNKNOWN>";
    }
    break;
  }
}


/* forward declaration */
static
void value2buffer(ostream &out, OZ_Term term, int depth=0);

inline
void feature2buffer(ostream &out, SRecord *sr, OZ_Term fea, int depth)
{
  value2buffer(out,fea);
  out << ':';
  value2buffer(out,sr->getFeature(fea),depth);
}

static
int listWidth = 0;

inline
Bool isNiceHash(TaggedRef t, int width) {
  if (width <= 0) return OK;

  if (!isSTuple(t) || !literalEq(tagged2SRecord(t)->getLabel(),AtomPair)) 
    return NO;

  int w = tagged2SRecord(t)->getWidth();

  return ((w <= width) && (w > 1)) ? OK : NO;
}

inline
Bool isNiceList(TaggedRef l, int width) {
  if (width <= 0) return OK;

  while (isCons(l) && width--> 0) {
    l = deref(tail(l));
  }
  
  if (isNil(l)) return OK;

  return NO;
}

inline
void record2buffer(ostream &out, SRecord *sr,int depth) {
  if (isNiceHash(makeTaggedSRecord(sr), listWidth)) {
    int len = sr->getWidth();
    for (int i=0; i < len; i++) {
      TaggedRef arg = deref(sr->getArg(i));
      if (isNiceHash(arg,listWidth) ||
	  (isCons(arg) && !isNiceList(arg,listWidth))) {
	out << '(';
	value2buffer(out, sr->getArg(i), depth-1);
	out << ')';
      } else {
	value2buffer(out, sr->getArg(i), depth-1);
      }
      if (i+1!=len)
	out << '#';
    }
    return;
  }

  value2buffer(out,sr->getLabel());
  out << '(';
  if (depth <= 0) {
    out << ",,,";
  } else {
    if (sr->isTuple()) {
      int len = min(listWidth, sr->getWidth());
      value2buffer(out,sr->getArg(0), depth-1);
      for (int i=1; i < len; i++) {
	out << ' ';
	value2buffer(out,sr->getArg(i),depth-1);
      }
      
      if (sr->getWidth() > listWidth)
	out << " ,,,";
    } else {
      TaggedRef as = sr->getArityList();
      Assert(isCons(as));

      int next    = 1;

      while (isCons(as) && next <= listWidth &&
	     isSmallInt(head(as)) && 
	     smallIntValue(head(as)) == next) {
	value2buffer(out, sr->getFeature(head(as)), depth-1);
	out << ' ';
	as = tail(as);
	next++;
      }
      Assert(isCons(as));

      if (next <= listWidth) {
	
	feature2buffer(out,sr,head(as),depth-1);
	next++;
	as = tail(as);
	while (next <= listWidth && isCons(as)) {
	  out << ' ';
	  feature2buffer(out,sr,head(as),depth-1);
	  as = tail(as);
	  next++;
	}
      }

      if (sr->getWidth() > listWidth)
	out << " ,,,";
    }
  }
  out << ')';
}

inline
void list2buffer(ostream &out, LTuple *list,int depth) {
  int width = listWidth;

  if (width > 0 && depth > 0) {

    if (isNiceList(makeTaggedLTuple(list),width)) {
      out << '[';
      TaggedRef l = makeTaggedLTuple(list);
      while (isCons(l)) {
	value2buffer(out, head(l), depth-1);
	l = deref(tail(l));
	if (isCons(l)) {
	  out << ' ';
	}
      }
      out << ']';
      return;
    }

    while (width-- > 0) { 
      OZ_Term a=deref(list->getHead());
      if (isCons(a) && !isNiceList(a,listWidth)) {
	out << '('; value2buffer(out,list->getHead(),depth-1); out << ')';
      } else {
	value2buffer(out,list->getHead(),depth-1);
      }
      out << '|';
      OZ_Term t=deref(list->getTail());
      if (!isCons(t)) {
	value2buffer(out,list->getTail(),depth);
	return;
      }
      list = tagged2LTuple(t);
    }
  }
  out << ",,,|,,,";
}


ostream &DynamicTable::newprint(ostream &out, int depth)
{
  // Count Atoms & Names in dynamictable:
  TaggedRef tmplit,tmpval;
  dt_index di;
  long ai;
  long nAtomOrInt=0;
  long nName=0;
  for (di=0; di<size; di++) {
    tmplit=table[di].ident;
    tmpval=table[di].value;
    if (tmpval) { 
      CHECK_DEREF(tmplit);
      if (isAtom(tmplit)||isInt(tmplit)) nAtomOrInt++; else nName++;
    }
  }
  // Allocate array on heap, put Atoms in array:
  TaggedRef *arr = new TaggedRef[nAtomOrInt+1]; // +1 since nAtomOrInt may be zero
  for (ai=0,di=0; di<size; di++) {
    tmplit=table[di].ident;
    tmpval=table[di].value;
    if (tmpval!=makeTaggedNULL() && (isAtom(tmplit)||isInt(tmplit)))
      arr[ai++]=tmplit;
  }
  // Sort the Atoms according to printName:
  inplace_quicksort(arr, arr+(nAtomOrInt-1));

  // Output the Atoms first, in order:
  for (ai=0; ai<nAtomOrInt; ai++) {
    value2buffer(out,arr[ai],0);
    out << ':';
    value2buffer(out,lookup(arr[ai]),depth);
    out << ' ';
  }
  // Output the Names last, unordered:
  for (di=0; di<size; di++) {
    tmplit=table[di].ident;
    tmpval=table[di].value;
    if (tmpval!=makeTaggedNULL() && !(isAtom(tmplit)||isInt(tmplit))) {
      value2buffer(out,tmplit,0);
      out << ':';
      value2buffer(out,tmpval,depth);
      out << ' ';
    }
  }
  // Deallocate array:
  delete arr;
  return out;
}

static
void cvar2buffer(ostream &out, char *s, GenCVariable *cv, int depth)
{
  switch(cv->getType()){
  case FDVariable:
    {
      out << s;
      out << ((GenFDVariable *) cv)->getDom();
      break;
    }

  case BoolVariable:
    {
      out << s;
      out << "{0#1}";
      break;
    }

  case OFSVariable:
    {
      GenOFSVariable* ofs = (GenOFSVariable *) cv;
      value2buffer(out,ofs->getLabel(),0);
      out << '(';
      if (depth > 0) {
	ofs->getTable()->newprint(out,depth-1);
      } else {
	out << ",,, ";
	break;
      }
      out << "...)";
      break;
   }

  case AVAR:
    {
      out << s;
      break;
    }
  default:
    OZ_warning("OZ_toC: Unknown variable type\n");
    break;
  }
}

static
void value2buffer(ostream &out, OZ_Term term, int depth)
{
  if (!term) {
    out << "<NULL>";
  } else {

    DEREF(term,termPtr,tag);
    switch(tag) {
    case UVAR:
    case SVAR:
      {
	char *s = getVarName(makeTaggedRef(termPtr));
	if (!*s) {
	  out << '_';
	} else {
	  out << s;
	}
      }
      break;
    case CVAR:
      {
	char *s = getVarName(makeTaggedRef(termPtr));
	if (!*s) {
	  s = "_";
	}
	if (isCVar(tag)) { cvar2buffer(out, s,tagged2CVar(term),depth); }
      }
      break;

    case SRECORD:
      record2buffer(out,tagged2SRecord(term),depth);
      break;
    case LTUPLE:
      list2buffer(out,tagged2LTuple(term),depth);
      break;
    case OZCONST:
      const2buffer(out,tagged2Const(term));
      break;
    case LITERAL:
      {
	Literal *a = tagged2Literal(term);
	if (a->isAtom()) {
	  atom2buffer(out,a);
	} else {
	  name2buffer(out,a);
	}
      }
      break;
    case OZFLOAT:
      float2buffer(out,term);
      break;
    case BIGINT:
    case SMALLINT:
      int2buffer(out,term);
      break;
    default:
      OZ_warning("OZ_toC: unknown type");
      break;
    }
  }
}

char *OZ_toC(OZ_Term term, int depth,int width)
{
  static char *tmpString = 0;
  if (tmpString) {
    delete tmpString;
  }

  ostrstream out;

  int old=listWidth;
  listWidth = width;
  value2buffer(out,term,depth);
  listWidth = old;

  out << ends;
  tmpString = out.str();
  return tmpString;
}

int OZ_termGetSize(OZ_Term term, int depth, int width)
{
  ostrstream *out=new ostrstream;
  int old=listWidth;

  listWidth = width;
  value2buffer(*out,term,depth);
  listWidth = old;

  int ret = out->pcount ();
  delete out;
  return ret;
}

/*
 * Atoms
 */

char *OZ_atomToC(OZ_Term term)
{
  term = deref(term);

  Literal *a = tagged2Literal(term);
  return a->getPrintName();
}

OZ_Term OZ_atom(char *s)
{
  return makeTaggedAtom(s);
}

/* -----------------------------------------------------------------
 * virtual strings
 * -----------------------------------------------------------------*/

/* convert a C string (char*) to an Oz string */
OZ_Term OZ_string(char *s)
{
  if (!s) { return nil(); }
  char *p=s;
  while (*p!='\0') {
    p++;
  }
  OZ_Term ret = nil();
  while (p!=s) {
    ret = cons(makeInt((unsigned char)*(--p)), ret);
  }
  return ret;
}

void string2buffer(ostream &out,OZ_Term list)
{
  OZ_Term tmp = deref(list);
  for (; isCons(tmp); tmp=deref(tail(tmp))) {
    OZ_Term hh = deref(head(tmp));
    if (!isSmallInt(hh)) {
      message("no small int %s",toC(hh));
      printf(" in string %s\n",toC(list));
      return;
    }
    int i = smallIntValue(hh);
    if (i <= 0 || i > 255) {
      message("no small int %d",i);
      printf(" in string %s\n",toC(list));
      return;
    }
    out << (char) i;
  }
  if (!isNil(tmp)) {
    message("no string %s\n",toC(list));
  }
}

/*
 * convert Oz string to C string
 * PRE: list is a proper string
 */
char *OZ_stringToC(OZ_Term list)
{
  static char *tmpStr = 0;
  if (tmpStr) {
    delete tmpStr;
    tmpStr = 0;
  }

  ostrstream out;

  string2buffer(out,list);

  out << ends;
  tmpStr = out.str();
  return tmpStr;
}

void OZ_printString(OZ_Term term)
{
  string2buffer(cout,term);
  cout << flush;
}

void OZ_printAtom(OZ_Term t)
{
  Literal *a = tagged2Literal(t);
  printf("%s",a->getPrintName());
}

void OZ_printInt(OZ_Term t)
{
  t=deref(t);
  if (isSmallInt(t)) {
    printf("%d",smallIntValue(t));
  } else {
    char *s=toC(t);
    printf("%s",s);
  }
}

void OZ_printFloat(OZ_Term t)
{
  char *s=toC(t);
  printf("%s",s);
}



inline
void vsatom2buffer(ostream &out, OZ_Term term)
{
  char *s = tagged2Literal(term)->getPrintName();
  out << s;
}

void virtualString2buffer(ostream &out,OZ_Term term)
{
  OZ_Term t=deref(term);
  if (isCons(t)) {
    string2buffer(out,t);
    return;
  }
  if (isAtom(t)) {
    if (isNil(t) || isPair(t)) return;
    vsatom2buffer(out,t);
    return;
  }
  if (isInt(t)) {
    int2buffer(out,t);
    return;
  }
  if (isFloat(t)) {
    float2buffer(out,t);
    return;
  }

  if (!isPair(t)) {
    OZ_warning("no virtual string: %s",toC(term));
    return;
  }

  SRecord *sr=tagged2SRecord(t);
  int len=sr->getWidth();
  for (int i=0; i < len; i++) {
    virtualString2buffer(out,sr->getArg(i));
  }
}

/*
 * convert Oz virtual string to C string
 * PRE: list is a virtual string
 */
char *OZ_virtualStringToC(OZ_Term t)
{
  static char *tmpStr = 0;
  if (tmpStr) {
    delete tmpStr;
    tmpStr = 0;
  }

  ostrstream out;

  virtualString2buffer(out,t);

  out << ends;
  tmpStr = out.str();
  return tmpStr;
}


void OZ_printVirtualString(OZ_Term term)
{
  OZ_Term t=deref(term);
  if (isCons(t)) {
    OZ_printString(t);
  } else if (isAtom(t)) {
    if (isNil(t) || isPair(t)) {
      return;
    }
    OZ_printAtom(t);
  } else if (isInt(t)) {
    if (isSmallInt(t)) {
      printf("%d",smallIntValue(t));
    } else {
      char *s=toC(t);
      if (s[0] == '~') 
	s[0] = '-';
      printf("%s",s);
    }
  } else if (isFloat(t)) {
    char *s=toC(t);
    char *p=s;
    while (*p) {
      if (*p == '~') 
	*p='-';
      p++;
    }
    printf("%s",s);
  } else {
    if (!isPair(t)) {
      OZ_warning("OZ_printVirtualString: no virtual string: %s",toC(term));
      return;
    }
    SRecord *sr=tagged2SRecord(t);
    int len=sr->getWidth();
    for (int i=0; i < len; i++) {
      OZ_printVirtualString(sr->getArg(i));
    }
  }
}

OZ_Term OZ_toVirtualString(OZ_Term t,int depth, int width)
{
  switch (tagTypeOf(deref(t))) {
  case SMALLINT:
  case BIGINT:
  case OZFLOAT:
    return t;
  case UVAR:
  case SVAR:
  case CVAR:
  case LTUPLE:
  case SRECORD:
  case OZCONST:
    return OZ_string(OZ_toC(t,depth,width));
  case LITERAL:
    if (OZ_isAtom(t)) return t;
    return OZ_string(toC(t));
  default:
    return 0;
  }
}

/* -----------------------------------------------------------------
 * tuple
 * -----------------------------------------------------------------*/

OZ_Term OZ_label(OZ_Term term)
{
  DEREF(term,termPtr,termTag);

  switch (termTag) {
  case LTUPLE:
    return AtomCons;
  case LITERAL:
    return term;
  case SRECORD:
    return tagged2SRecord(term)->getLabel();
  default:
    OZ_warning("OZ_label: no record");
    return 0;
  }
}

int OZ_width(OZ_Term term)
{
  DEREF(term,termPtr,termTag);

  switch (termTag) {
  case LTUPLE:
    return 2;
  case SRECORD:
    return tagged2SRecord(term)->getWidth();
  case LITERAL:
    return 0;
  default:
    OZ_warning("OZ_width: no record");
    return 0;
  }
}

OZ_Term OZ_tuple(OZ_Term label, int width) 
{
  label = deref(label);
  if (!isLiteral(label)) {
    OZ_warning("OZ_tuple: label is no literal");
    return 0;
  }

  if (width == 2 && literalEq(label,AtomCons)) {
    // have to make a list
    return makeTaggedLTuple(new LTuple());
  }

  if (width <= 0) {
    return label;
  }
  
  return makeTaggedSRecord(SRecord::newSRecord(label,width));
}

OZ_Term OZ_mkTupleC(char *label,int arity,...)
{
  if (arity == 0) {
    return OZ_atom(label);
  }
  va_list ap;
  va_start(ap,arity);

  OZ_Term tt=OZ_tuple(OZ_atom(label),arity);
  for (int i = 0; i < arity; i++) {
    OZ_putArg(tt,i,va_arg(ap,OZ_Term));
  }

  va_end(ap);
  return tt;
}

OZ_Term OZ_mkTuple(OZ_Term label,int arity,...)
{
  va_list ap;
  va_start(ap,arity);

  OZ_Term tt=OZ_tuple(label,arity);
  for (int i = 0; i < arity; i++) {
    OZ_putArg(tt,i,va_arg(ap,OZ_Term));
  }

  va_end(ap);
  return tt;
}

void OZ_putArg(OZ_Term term, int pos, OZ_Term newTerm)
{
  term=deref(term);
  if (isLTuple(term)) {
    switch (pos) {
    case 0:
      tagged2LTuple(term)->setHead(newTerm);
      return;
    case 1:
      tagged2LTuple(term)->setTail(newTerm);
      return;
    }
  }
  if (!isSTuple(term)) {
    OZ_warning("OZ_putArg: no record");
    return;
  }
  tagged2SRecord(term)->setArg(pos,newTerm);
}

OZ_Term OZ_getArg(OZ_Term term, int pos)
{
  term=deref(term);
  if (isLTuple(term)) {
    switch (pos) {
    case 0:
      return tagged2LTuple(term)->getHead();
    case 1:
      return tagged2LTuple(term)->getTail();
    }
  }
  if (!isSRecord(term)) {
    OZ_warning("OZ_getArg: no record");
    return 0;
  }
  if (pos < 0 || pos >= tagged2SRecord(term)->getWidth()) {
    OZ_warning("OZ_getArg: invalid index: %d",pos);
    return 0;
  }
  return tagged2SRecord(term)->getArg(pos);
}

OZ_Term OZ_nil()
{
  return nil();
}

OZ_Term OZ_cons(OZ_Term hd,OZ_Term tl)
{
  return cons(hd,tl);
}

OZ_Term OZ_head(OZ_Term term)
{
  term=deref(term);
  if (!isLTuple(term)) {
    OZ_warning("OZ_head: no cons");
    return 0;
  }
  return head(term);
}

OZ_Term OZ_tail(OZ_Term term)
{
  term=deref(term);
  if (!isLTuple(term)) {
    OZ_warning("OZ_tail: no cons");
    return 0;
  }
  return tail(term);
}

/*
 * Compute the length of a list and check for determination.
 * Returns:
 *  -1, if the list end is not determined (SUSPENDED)
 *  -2, if it is not a proper list (FAILED)
 *  else the length of the list
 */
int OZ_length(OZ_Term list)
{
  return length(list);
}


OZ_Term OZ_toList(int len, OZ_Term *tuple)
{
  OZ_Term l = nil();
  while (--len >= 0) {
    l = cons(tuple[len],l);
  }
  return l;
}

/* -----------------------------------------------------------------
 * pairs
 * -----------------------------------------------------------------*/

OZ_Term OZ_pair(int n)
{
  SRecord *sr = SRecord::newSRecord(AtomPair,n);
  return makeTaggedSRecord(sr);
}

OZ_Term OZ_pair2(OZ_Term t1,OZ_Term t2) {
  SRecord *sr = SRecord::newSRecord(AtomPair,2);
  sr->setArg(0,t1);
  sr->setArg(1,t2);
  return makeTaggedSRecord(sr);
}

/* -----------------------------------------------------------------
 * record
 * -----------------------------------------------------------------*/

OZ_Arity OZ_makeArity(OZ_Term list)
{
  return (OZ_Arity) mkArity(list);
}

/* take a label and an arity (as list) and construct a record
   the fields are not initialized */
OZ_Term OZ_record(OZ_Term label, OZ_Term arity) 
{
  OZ_Arity newArity = OZ_makeArity(arity);
  return makeTaggedSRecord(SRecord::newSRecord(label,(Arity *) newArity));
}

/* take a label and a property list and construct a record */
OZ_Term OZ_recordInit(OZ_Term label, OZ_Term propList) 
{
  OZ_Term out;
  (void) adjoinPropList(label,propList,out,NO);

  return out;
}

void OZ_putSubtree(OZ_Term term, OZ_Term feature, OZ_Term value)
{
  term=deref(term);
  if (isCons(term)) {
    int i2 = smallIntValue(feature);

    switch (i2) {
    case 1:
      tagged2LTuple(term)->setHead(value);
      return;
    case 2:
      tagged2LTuple(term)->setTail(value);
      return;
    }
    OZ_warning("OZ_putSubtree: invalid feature");
    return;
  }
  if (!isSRecord(term)) {
    OZ_warning("OZ_putSubtree: invalid record");
    return;
  }
  if (!tagged2SRecord(term)->setFeature(feature,value)) {
    OZ_warning("OZ_putSubtree: invalid feature");
    return;
  }
}

OZ_Term OZ_adjoinAt(OZ_Term rec, OZ_Term fea, OZ_Term val)
{
  rec = deref(rec);
  fea = deref(fea);
  if (!isFeature(fea) || !isRecord(rec)) return 0;

  if (isLiteral(rec)) {
    SRecord *srec = SRecord::newSRecord(rec,aritytable.find(cons(fea,nil())));
    srec->setArg(0,val);
    return makeTaggedSRecord(srec);
  }

  SRecord *srec = makeRecord(rec);
  return srec->adjoinAt(fea,val);
}

OZ_Term OZ_subtree(OZ_Term term, OZ_Term fea)
{
  DEREF(term,termPtr,termTag);
  fea=deref(fea);

  switch (termTag) {
  case LTUPLE:
    {
      if (!isSmallInt(fea)) return 0;

      int i2 = smallIntValue(fea);

      switch (i2) {
      case 1:
	return tagged2LTuple(term)->getHead();
      case 2:
	return tagged2LTuple(term)->getTail();
      }
      return 0;
    }
  case SRECORD:
    return tagged2SRecord(term)->getFeature(fea);

  case OZCONST:
    {
      ConstTerm *ct = tagged2Const(term);
      switch (ct->getType()) {
      case Co_Object:
	return ((Object *) ct)->getFeature(fea);
      case Co_Chunk:
	return ((SChunk *) ct)->getFeature(fea);
      default:
	return 0;
      }
    }
  default:
    return 0;
  }
}


OZ_Term OZ_arityList(OZ_Term term)
{
  OZ_Term arity;
  (void) BIarityInline(term, arity);
  return arity;
}

/* -----------------------------------------------------------------
 * unification
 * -----------------------------------------------------------------*/

OZ_Return OZ_unify(OZ_Term t1, OZ_Term t2)
{
  return am.fastUnify(t1,t2,OK) ? PROCEED : FAILED;
}

int OZ_eq(OZ_Term t1, OZ_Term t2)
{
  return termEq(t1,t2);
}


OZ_Term OZ_newVariable()
{
  return makeTaggedRef(newTaggedUVar(am.currentBoard));
}

/* -----------------------------------------------------------------
 * IO
 * -----------------------------------------------------------------*/

void OZ_registerReadHandler(int fd,OZ_IOHandler fun,void *val) {
  am.select(fd,SEL_READ,fun,val);
}

void OZ_unregisterRead(int fd) {
  am.deSelect(fd,SEL_READ);
}

void OZ_registerWriteHandler(int fd,OZ_IOHandler fun,void *val) {
  am.select(fd,SEL_WRITE,fun,val);
}

void OZ_unregisterWrite(int fd) {
  am.deSelect(fd,SEL_WRITE);
}

void OZ_registerAcceptHandler(int fd,OZ_IOHandler fun,void *val) {
  am.acceptSelect(fd,fun,val);
}





OZ_Return OZ_readSelect(int fd,OZ_Term l,OZ_Term r) {
  return am.select(fd,SEL_READ,l,r) ? PROCEED : FAILED;
}

OZ_Return OZ_writeSelect(int fd,OZ_Term l,OZ_Term r) {
  return am.select(fd,SEL_WRITE,l,r) ? PROCEED : FAILED;
}

OZ_Return OZ_acceptSelect(int fd,OZ_Term l,OZ_Term r) {
  am.acceptSelect(fd,l,r);
  return PROCEED;
}

void OZ_deSelect(int fd) {
  am.deSelect(fd);
}


/* -----------------------------------------------------------------
 * garbage collection
 * -----------------------------------------------------------------*/

int OZ_protect(OZ_Term *t)
{
  if (!gcProtect(t)) {
    OZ_warning("protect: failed");
    return 0;
  }
  return 1;
}

int OZ_unprotect(OZ_Term *t)
{
  if (!gcUnprotect(t)) {
    OZ_warning("unprotect: failed");
    return 0;
  }
  return 1;
}

/* -----------------------------------------------------------------
 * vs
 * -----------------------------------------------------------------*/


inline
int isVirtualString(OZ_Term vs, OZ_Term *var)
{
  DEREF(vs,vsPtr,vsTag);
  if (isAnyVar(vsTag))  {
    if (var) *var = makeTaggedRef(vsPtr);
    return 0;
  }

  if (isInt(vs) || isFloat(vs) || isAtom(vs))  return 1;

  if (isPair(vs)) {
    SRecord *sr = tagged2SRecord(vs);
    int len = sr->getWidth();
    for (int i=0; i < len; i++) {
      if (!isVirtualString(sr->getArg(i),var)) return 0;
    }
    return 1;
  }

  if (isCons(vs)) return isList(vs,var,2);

  return 0;
}

int OZ_isVirtualString(OZ_Term vs, OZ_Term *var)
{
  if (var) *var = 0;

  return isVirtualString(vs,var);
}


/* -----------------------------------------------------------------
 * names
 * -----------------------------------------------------------------*/

OZ_Term OZ_newName()
{
  return makeTaggedLiteral(Name::newName(am.currentBoard));
}
/* -----------------------------------------------------------------
 * 
 * -----------------------------------------------------------------*/

int OZ_addBuiltin(char *name, int arity, OZ_CFun fun)
{
  return BIadd(name,arity,fun) == NULL ? 0 : 1;
}

void OZ_addBISpec(OZ_BIspec *spec)
{
  for (int i=0; spec[i].name; i++) {
    OZ_addBuiltin(spec[i].name,spec[i].arity,spec[i].fun);
  }
}

OZ_Return OZ_raise(OZ_Term exc)
{
  if (OZ_isVariable(exc)) {
    return am.raise(E_ERROR,E_KERNEL,"instantiation",5,
		    OZ_atom("raise"),cons(exc,nil()),
		    OZ_atom("det"),OZ_int(1),OZ_string(""));
  }
  am.exception.value=exc;
  am.exception.info=NameUnit;
  am.exception.debug=FALSE;
  return RAISE;
}

OZ_Return OZ_raiseA(char *name, int was, int shouldBe)
{
  return am.raise(E_ERROR,E_SYSTEM,"inconsistentArity",3,
		  OZ_atom(name),OZ_int(was),OZ_int(shouldBe));
}

OZ_Return OZ_raiseC(char *label,int arity,...)
{
  if (arity == 0) {
    return OZ_raise(OZ_atom(label));
  }

  va_list ap;
  va_start(ap,arity);

  OZ_Term tt=OZ_tuple(OZ_atom(label),arity);
  for (int i = 0; i < arity; i++) {
    OZ_putArg(tt,i,va_arg(ap,OZ_Term));
  }

  va_end(ap);
  return OZ_raise(tt);
}

OZ_Return OZ_raiseError(OZ_Term exc)
{
  OZ_Term ret = OZ_record(OZ_atom("error"),
			  cons(OZ_int(1),
			       cons(OZ_atom("debug"),nil())));
  OZ_putSubtree(ret,OZ_int(1),exc);
  OZ_putSubtree(ret,OZ_atom("debug"),NameUnit);

  am.exception.info = NameUnit;
  am.exception.value = ret;
  am.exception.debug = TRUE;
  return RAISE;
}

OZ_Return OZ_raiseErrorC(char *label,int arity,...)
{
  if (arity == 0) {
    return OZ_raiseError(OZ_atom(label));
  }

  va_list ap;
  va_start(ap,arity);

  OZ_Term tt=OZ_tuple(OZ_atom(label),arity);
  for (int i = 0; i < arity; i++) {
    OZ_putArg(tt,i,va_arg(ap,OZ_Term));
  }

  va_end(ap);
  return OZ_raiseError(tt);
}

/* -----------------------------------------------------------------
 * Suspending builtins
 * -----------------------------------------------------------------*/

OZ_Thread OZ_makeSuspendedThread(OZ_CFun fun,OZ_Term *args,int arity)
{
  Thread *thr;
#ifdef SHOW_SUSPENSIONS
  static int xxx=0;
  printf("Suspension(%d):",xxx++);
  for(int i=0; i<arity; i++) {
    printf("%s, ",tagged2String(args[i],2));
  }
  printf("\n");
#endif

  thr = am.mkSuspendedThread(am.currentBoard, DEFAULT_PRIORITY,0);
  thr->pushCFunCont (fun, args, arity, OK);

  return ((OZ_Thread) thr);
}

void OZ_makeRunnableThread(OZ_CFun fun, OZ_Term *args,int arity)
{
  Thread *tt = am.mkRunnableThread(DEFAULT_PRIORITY, am.currentBoard);
  tt->pushCFunCont (fun, args, arity, OK);
  am.scheduleThread (tt);
}

void OZ_addThread(OZ_Term var, OZ_Thread thr)
{
  DEREF(var, varPtr, varTag);
  if (!isAnyVar(varTag)) {
    OZ_warning("OZ_addThread(%s): var arg expected",
	       toC(var));
    return;
  }

  addSuspAnyVar(varPtr, new SuspList((Thread *) thr));
}


void OZ_suspendOnInternal(OZ_Term var)
{
  DEREF(var,varPtr,_1);
  Assert(isAnyVar(var));
  am.addSuspendVarList(varPtr);
}

void OZ_suspendOnInternal2(OZ_Term var1,OZ_Term var2)
{
  DEREF(var1,varPtr1,_1);
  if (isAnyVar(var1)) {
    am.addSuspendVarList(varPtr1);
  }
  DEREF(var2,varPtr2,_2);
  if (isAnyVar(var2)) {
    am.addSuspendVarList(varPtr2);
  }
}

void OZ_suspendOnInternal3(OZ_Term var1,OZ_Term var2,OZ_Term var3)
{
  DEREF(var1,varPtr1,_1);
  if (isAnyVar(var1)) {
    am.addSuspendVarList(varPtr1);
  }
  DEREF(var2,varPtr2,_2);
  if (isAnyVar(var2)) {
    am.addSuspendVarList(varPtr2);
  }
  DEREF(var3,varPtr3,_3);
  if (isAnyVar(var3)) {
    am.addSuspendVarList(varPtr3);
  }
}

/* -----------------------------------------------------------------
 * 
 * -----------------------------------------------------------------*/

OZ_Term OZ_newCell(OZ_Term val)
{
  return makeTaggedConst(new Cell(am.currentBoard, val));
}

OZ_Term OZ_newChunk(OZ_Term val)
{
  val=deref(val);
  if (!isRecord(val)) return 0;
  return makeTaggedConst(new SChunk(am.currentBoard, val));
}

int OZ_onToplevel()
{
  return am.isToplevel() ? 1 : 0;
}

/* -----------------------------------------------------------------
 * 
 * -----------------------------------------------------------------*/

char *OZ_unixError(int aErrno) {
#ifndef SUNOS_SPARC
  return strerror(aErrno);
#else
  extern char *sys_errlist[];
  return sys_errlist[aErrno];
#endif  
}

OZ_Return OZ_typeError(int pos,char *type)
{
  TypeErrorT(pos,type);
}

void OZ_main(int argc,char **argv)
{
  am.init(argc,argv);
  engine(NO);
  am.exitOz(0);
}

OZ_C_proc_proto(BIfail);
void OZ_fail(char *format, ...)
{
  va_list ap;

  va_start(ap,format);

  prefixWarning();

  fprintf(stderr, "*** Failure: ");
  vfprintf(stderr,format,ap);
  fprintf(stderr, "\n");

  va_end(ap);

  OZ_makeRunnableThread(BIfail,0,0);
}


OZ_Term OZ_newPort(OZ_Term val) 
{
  return makeTaggedConst(new PortWithStream(am.currentBoard, val));
}

void OZ_send(OZ_Term port, OZ_Term val)
{
  port = deref(port);
  if (!isPort(port)) return;

  (void) sendPort(port,val);
}
