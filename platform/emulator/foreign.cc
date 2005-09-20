/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Michael Mehl (1997)
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "iso-ctype.hh"

#include "value.hh"
#include "var_base.hh"

#include "os.hh"
#include "thr_int.hh"
#include "builtins.hh"
#include "dictionary.hh"

#include "bytedata.hh"

#include "mozart.h"

// forward decl
static
void term2Buffer(ostream &out, OZ_Term term, int depth=0);

/* ------------------------------------------------------------------------ *
 * tests
 * ------------------------------------------------------------------------ */

OZ_Term OZ_deref(OZ_Term term)
{
  return oz_deref(term);
}

int OZ_isBool(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isBool(term);
}

int OZ_isAtom(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isAtom(term);
}

int OZ_isCell(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isCell(term);
}

int OZ_isPort(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isPort(term);
}

int OZ_isChunk(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isChunk(term);
}

int OZ_isDictionary(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isDictionary(term);
}

int OZ_isCons(OZ_Term term)
{
  term = oz_deref(term);
  Assert(!oz_isRef(term));
  return oz_isLTupleOrRef(term);
}

int OZ_isFloat(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isFloat(term);
}

int OZ_isInt(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isInt(term);
}

int OZ_isNumber(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isNumber(term);
}

int OZ_isSmallInt(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isSmallInt(term);
}

int OZ_isBigInt(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isBigInt(term);
}

int OZ_isProcedure(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isProcedure(term);
}


int OZ_isLiteral(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isLiteral(term);
}

int OZ_isFeature(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isFeature(term);
}

int OZ_isName(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isName(term);
}

int OZ_isNil(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isNil(term);
}

int OZ_isObject(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isObject(term);
}

int OZ_isThread(OZ_Term t)
{
  t = oz_deref(t);
  return oz_isThread(t);
}

int OZ_isBitString(OZ_Term t)
{
  t = oz_deref(t);
  return oz_isBitString(t);
}

int OZ_isFSetValue(OZ_Term t)
{
  t = oz_deref(t);
  return oz_isFSetValue(t);
}


OZ_Boolean OZ_BitStringGet(OZBitString bs,int index)
{
  return ((BitString*)bs)->get(index) ? OZ_TRUE : OZ_FALSE;
}

OZBitString OZ_getBitString(OZ_Term bs)
{
  return tagged2BitString(bs);
}


int OZ_isByteString(OZ_Term t)
{
  t = oz_deref(t);
  return oz_isByteString(t);
}

int OZ_isString(OZ_Term term,OZ_Term *var)
{
  OZ_Term ret = oz_checkList(term,OZ_CHECK_CHAR);
  if (oz_isRef(ret)) {
    if (var) *var = ret;
    return 0;
  }
  if (var) *var = 0;
  return !oz_isFalse(ret);
}

int OZ_isProperString(OZ_Term term,OZ_Term *var)
{
  OZ_Term ret = oz_checkList(term,OZ_CHECK_CHAR_NONZERO);
  if (oz_isRef(ret)) {
    if (var) *var = ret;
    return 0;
  }
  if (var) *var = 0;
  return !oz_isFalse(ret);
}

int OZ_isList(OZ_Term term,OZ_Term *var)
{
  OZ_Term ret = oz_checkList(term);
  if (oz_isRef(ret)) {
    if (var) *var = ret;
    return 0;
  }
  if (var) *var = 0;
  return !oz_isFalse(ret);
}

int OZ_isTrue(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isTrue(term);
}

int OZ_isFalse(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isFalse(term);
}

int OZ_isUnit(OZ_Term term)
{
  term = oz_deref(term);
  return oz_eq(term,NameUnit);
}


int OZ_isPair(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isPair(term);
}

int OZ_isPair2(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isPair2(term);
}

int OZ_isRecord(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isRecord(term);
}

int OZ_isTuple(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isTuple(term);
}

int OZ_isValue(OZ_Term term)
{
  term = oz_deref(term);
  Assert(!oz_isRef(term));
  return !oz_isVarOrRef(term);
}

int OZ_isVariable(OZ_Term term)
{
  term = oz_deref(term);
  Assert(!oz_isRef(term));
  return oz_isVarOrRef(term);
}

inline
TaggedRef oz_valueType(OZ_Term term) {
  Assert(!oz_isRef(term));

  switch (tagged2ltag(term)) {
  case LTAG_VAR0:
  case LTAG_VAR1:
    return AtomVariable;
  case LTAG_SMALLINT:
    return AtomInt;
  case LTAG_LITERAL:
    return tagged2Literal(term)->isAtom() ? AtomAtom : AtomName;
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
    return AtomTuple;
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    return tagged2SRecord(term)->isTuple() ? AtomTuple : AtomRecord;
  case LTAG_CONST0:
  case LTAG_CONST1:
    switch (tagged2Const(term)->getType()) {
    case Co_Extension:
      return tagged2Extension(term)->typeV();
    case Co_Float:    
      return AtomFloat;
    case Co_BigInt:
      return AtomInt;
    case Co_FSetValue:
      return AtomFSet;
    case Co_Foreign_Pointer:
      return AtomForeignPointer;
    case Co_Abstraction:
    case Co_Builtin:
      return AtomProcedure;
    case Co_Cell:
      return AtomCell;
    case Co_Space:
      return AtomSpace;
    case Co_Object:
      return AtomObject;
    case Co_Port:
      return AtomPort;
    case Co_Chunk:
      return AtomChunk;
    case Co_Array:
      return AtomArray;
    case Co_Dictionary:
      return AtomDictionary;
    case Co_Lock:
      return AtomLock;
    case Co_Class:
      return AtomClass;
    case Co_Resource:
      return AtomResource;
    default:
      Assert(0);
    }
  default:
    Assert(0);
  }
  return makeTaggedNULL();
}

OZ_Term OZ_termType(OZ_Term term)
{
  return oz_valueType(oz_deref(term));
}

/* -----------------------------------------------------------------
 * providing constants
 *------------------------------------------------------------------*/

int OZ_getLowPrio(void) 
{
  return LOW_PRIORITY;
}

int OZ_getMediumPrio(void) 
{
  return MID_PRIORITY;
}

int OZ_getHighPrio(void) 
{
  return HI_PRIORITY;
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
  return oz_false();
}

OZ_Term OZ_true(void)
{
  return oz_true();
}

OZ_Term OZ_unit(void)
{
  return NameUnit;
}

/* -----------------------------------------------------------------
 * convert: C from/to Oz datastructure
 * -----------------------------------------------------------------*/

/*
 * Ints
 */

OZ_Term OZ_int(int i)
{
  return oz_int(i);
}

OZ_Term OZ_long(long i) {
  return oz_long(i);
}

OZ_Term OZ_unsignedInt(unsigned int i)
{
  return oz_unsignedInt(i);
}

OZ_Term OZ_unsignedLong(unsigned long i)
{
  return oz_unsignedLong(i);
}

/*
 * BigInt: return INT_MAX/INT_MIN if too big
 */
int OZ_intToC(OZ_Term term)
{
  return oz_intToC(oz_deref(term));
}

long OZ_intToCL(OZ_Term term)
{
  return oz_intToCL(oz_deref(term));
}

unsigned long OZ_intToCulong(OZ_Term term)
{
  term = oz_deref(term);
  if (oz_isSmallInt(term))
    return (unsigned long) tagged2SmallInt(term);
  else
    return tagged2BigInt(term)->getUnsignedLong();
}

OZ_Term OZ_CStringToInt(char *str)
{
  if (!str || str[0] == '\0')
    return 0;

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
      return makeTaggedSmallInt(0);
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
  return oz_float(i);
}

double OZ_floatToC(OZ_Term term)
{
  term = oz_deref(term);
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

  return oz_float(res);
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
  term1 = oz_deref(term1);
  term2 = oz_deref(term2);
  return featureCmp(term1,term2);
}


/*
 * PRINTING
 */
inline
void smallInt2buffer(ostream &out, OZ_Term term, const char sign)
{
  int i = tagged2SmallInt(term);
  if (i < 0) {
    out << sign << -i;
  } else {
    out << i;
  }
}

inline
void bigInt2buffer(ostream &out, BigInt *bb, const char sign)
{
  char *str = new char[bb->stringLength()+1];
  bb->getString(str);
  if ((*str == '-') && (sign != '-')) 
    *str = sign;
  out << str;
  delete [] str;
}


inline
char *strAndDelete(ostrstream *out)
{
  (*out) << ends;
  int n = out->pcount();
  char *ret = new char[n+1];
  memcpy((void*)ret,(void*)out->str(),n);
  ret[n] = '\0';
  delete out;
  return ret;
}

static
void float2buffer(ostream &out, OZ_Term term, const char sign)
{
  double f = floatValue(term);

  ostrstream *tmp = new ostrstream;
  (*tmp) << f;
  char *str = strAndDelete(tmp);

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
      out << sign;
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
void atomq2buffer(ostream &out, const char *s)
{
  char c;
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
    } else if (iso_isprint(c)) {
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
    } else {
      out << '\\';
      octOut(out,c);
    }
    s++;
  }
}


inline
Bool checkAtom(const char *s)
{
  const char *t = s;
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
    return strcmp(t, "andthen") && strcmp(t, "at")
	&& strcmp(t, "attr")? OK: NO;
  case 'c':
    return strcmp(t, "case") && strcmp(t, "catch")
	&& strcmp(t, "choice") && strcmp(t, "class")
	&& strcmp(t, "cond")? OK: NO;
  case 'd':
    return strcmp(t, "declare") && strcmp(t, "define")
	&& strcmp(t, "dis") && strcmp(t, "div")
	&& strcmp(t, "do")? OK: NO;
  case 'e':
    return strcmp(t, "else") && strcmp(t, "elsecase")
	&& strcmp(t, "elseif") && strcmp(t, "elseof")
	&& strcmp(t, "end") && strcmp(t, "export")? OK: NO;
  case 'f':
    return strcmp(t, "false") && strcmp(t, "feat")
	&& strcmp(t, "finally") && strcmp(t, "for")
	&& strcmp(t, "from") && strcmp(t, "fun")
	&& strcmp(t, "functor") && strcmp(t, "fail")? OK: NO;
  case 'i':
    return strcmp(t, "if") && strcmp(t, "import")
	&& strcmp(t, "in")? OK: NO;
  case 'l':
    return strcmp(t, "local") && strcmp(t, "lock");
  case 'm':
    return strcmp(t, "meth") && strcmp(t, "mod")? OK: NO;
  case 'n':
    return strcmp(t, "not")? OK: NO;
  case 'o':
    return strcmp(t, "of") && strcmp(t, "or")
	&& strcmp(t, "orelse")? OK: NO;
  case 'p':
    return strcmp(t, "prepare") && strcmp(t, "proc")
	&& strcmp(t, "prop")? OK: NO;
  case 'r':
    return strcmp(t, "raise") && strcmp(t, "require")? OK: NO;
  case 's':
    return strcmp(t, "self") && strcmp(t, "skip")? OK: NO;
  case 't':
    return strcmp(t, "then") && strcmp(t, "thread")
	&& strcmp(t, "true") && strcmp(t, "try")? OK: NO;
  case 'u':
    return strcmp(t, "unit")? OK: NO;
  default:
    return OK;
  }
}

inline
void atom2buffer(ostream &out, Literal *a)
{
  const char *s = a->getPrintName();
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
  const char *s = a->getPrintName();

  if (oz_isTrue(makeTaggedLiteral(a)))  {
    out << "true";
  } else if (oz_isFalse(makeTaggedLiteral(a))) {
    out << "false";
  } else if (oz_eq(makeTaggedLiteral(a),NameUnit)) {
    out << "unit";
  } else if (!*s) {
    out << "<N>";
  } else {
    out << "<N: " << s << '>';
  }
}

static
void fset2buffer(ostream &out, OZ_FSetValue * fs) 
{
  out << ((const OZ_FSetValue *) fs)->toString();
}


inline
void const2buffer(ostream &out, ConstTerm *c,const char sign,int depth)
{
  const char *s = c->getPrintName();

  switch (c->getType()) {
  case Co_Extension:
    {
      int n;
      char * s = OZ_virtualStringToC(const2Extension(c)->printV(depth),
				     &n);
      while (n--) out << *s++;
    }
    break;
  case Co_BigInt:
    bigInt2buffer(out,(BigInt *)c,sign);
    break;
  case Co_Float:
    float2buffer(out,makeTaggedConst(c),'~');
    break;
  case Co_FSetValue:
    fset2buffer(out, ((ConstFSetValue *) c)->getValue());
    break;
  case Co_Abstraction:
  case Co_Builtin:
    out << "<P/" << c->getArity();
    if (*s != 0) {
      out << ' ' << s;
    }
    out << '>';
    break;

  case Co_Cell:       out << "<Cell>"; break;
  case Co_Port:       out << "<Port>"; break;
  case Co_Space:      out << "<Space>"; break;
  case Co_Lock:       out << "<Lock>"; break;
  case Co_Array:      out << "<Array>"; break;
  case Co_Dictionary: out << "<Dictionary>"; break;
  case Co_Resource:   out << "<Resource>"; break;
  case Co_Class:
  case Co_Object:
    if (*s == '_' && *(s+1) == 0) {
      out << (isObjectClass(c) ? "<C>" : "<O>");
    } else {
      out << (isObjectClass(c) ? "<C: " : "<O: ") << s << '>';
    }
    break;

  case Co_Foreign_Pointer:
    out << "<ForeignPointer " << ((ForeignPointer *) c)->getPointer() << ">";
    break;

  default:
    out << "<Chunk>";
    break;
  }
}


inline
void feature2buffer(ostream &out, SRecord *sr, OZ_Term fea, int depth)
{
  term2Buffer(out,fea);
  out << ':';
  term2Buffer(out,sr->getFeature(fea),depth);
}

inline
Bool isNiceHash(OZ_Term t, int width) {
  if (width <= 0) return NO;

  if (!oz_isSTuple(t) || !oz_eq(tagged2SRecord(t)->getLabel(),AtomPair)) 
    return NO;

  int w = tagged2SRecord(t)->getWidth();

  return ((w <= width) && (w > 1)) ? OK : NO;
}

inline
Bool isNiceList(OZ_Term l, int width) {
  if (width <= 0) return NO;

  Assert(!oz_isRef(l));
  while (oz_isLTuple(l) && width-- > 0) {
    l = oz_deref(oz_tail(l));
  }
  
  if (oz_isNil(l)) return OK;

  return NO;
}

inline
void record2buffer(ostream &out, SRecord *sr,int depth) {
  if (depth <= 0 || ozconf.printWidth <= 0) {
    term2Buffer(out,sr->getLabel());
    out << "(,,,)";
    return;
  }

  if (isNiceHash(makeTaggedSRecord(sr), ozconf.printWidth)) {
    int len = sr->getWidth();
    for (int i=0; i < len; i++) {
      OZ_Term arg = oz_deref(sr->getArg(i));
      Assert(!oz_isRef(arg));
      if (isNiceHash(arg,ozconf.printWidth) ||
	  (oz_isLTupleOrRef(arg) && !isNiceList(arg,ozconf.printWidth))) {
	out << '(';
	term2Buffer(out, sr->getArg(i), depth-1);
	out << ')';
      } else {
	term2Buffer(out, sr->getArg(i), depth-1);
      }
      if (i+1!=len)
	out << '#';
    }
    return;
  }

  term2Buffer(out,sr->getLabel());
  out << '(';
  if (sr->isTuple()) {
    int len = min(ozconf.printWidth, sr->getWidth());
    term2Buffer(out,sr->getArg(0), depth-1);
    for (int i=1; i < len; i++) {
      out << ' ';
      term2Buffer(out,sr->getArg(i),depth-1);
    }
      
    if (sr->getWidth() > ozconf.printWidth)
      out << " ,,,";
  } else {
    OZ_Term as = sr->getArityList();
    Assert(oz_isCons(as));

    int next    = 1;

    while (oz_isCons(as) && next <= ozconf.printWidth &&
	   oz_isSmallInt(oz_head(as)) && 
	   tagged2SmallInt(oz_head(as)) == next) {
      term2Buffer(out, sr->getFeature(oz_head(as)), depth-1);
      out << ' ';
      as = oz_tail(as);
      next++;
    }
    Assert(oz_isCons(as));

    if (next <= ozconf.printWidth) {
	
      feature2buffer(out,sr,oz_head(as),depth-1);
      next++;
      as = oz_tail(as);
      while (next <= ozconf.printWidth && oz_isCons(as)) {
	out << ' ';
	feature2buffer(out,sr,oz_head(as),depth-1);
	as = oz_tail(as);
	next++;
      }
    }

    if (sr->getWidth() > ozconf.printWidth)
      out << " ,,,";
  }
  out << ')';
}

inline
void list2buffer(ostream &out, LTuple *list,int depth) {
  int width = ozconf.printWidth;

  if (width > 0 && depth > 0) {

    if (isNiceList(makeTaggedLTuple(list),width)) {
      out << '[';
      OZ_Term l = makeTaggedLTuple(list);
      while (oz_isLTuple(l)) {
	term2Buffer(out, oz_head(l), depth-1);
	l = oz_deref(oz_tail(l));
	if (oz_isCons(l)) {
	  out << ' ';
	}
      }
      out << ']';
      return;
    }

    while (width-- > 0) { 
      OZ_Term a=oz_deref(list->getHead());
      if (oz_isCons(a) && !isNiceList(a,ozconf.printWidth)) {
	out << '('; term2Buffer(out,list->getHead(),depth-1); out << ')';
      } else {
	term2Buffer(out,list->getHead(),depth-1);
      }
      out << '|';
      OZ_Term t=oz_deref(list->getTail());
      if (!oz_isCons(t)) {
	term2Buffer(out,list->getTail(),depth);
	return;
      }
      list = tagged2LTuple(t);
    }
  }
  out << ",,,|,,,";
}

// genvar.cc
void oz_var_printStream(ostream &out, const char *s, OzVariable *cv,
			int depth);

void oz_printStream(OZ_Term term, ostream &out, int depth, int width)
{
  int old = ozconf.printWidth;

  if (width>=0) {
    ozconf.printWidth=width;
  }
  if (depth<0) {
    depth = ozconf.printDepth;
  }

  term2Buffer(out,term,depth);
  flush(out);

  ozconf.printWidth=old;
}

#ifdef DEBUG_CHECK

// for printing from gdb: no default args needed
void oz_print(OZ_Term term) {
  oz_printStream(term,cerr);
  cerr << endl;
  flush(cerr);
}

#endif

static
void term2Buffer(ostream &out, OZ_Term term, int depth)
{
  if (!term) {
    out << "<Null pointer>";
    return;
  }

  DEREF(term,termPtr);
  switch (tagged2ltag(term)) {
  case LTAG_VAR0:
  case LTAG_VAR1:
    {
      if (!termPtr) {
	out << "<Oz_Dereferenced variable>";
	break;
      }
      const char *s = oz_varGetName(makeTaggedRef(termPtr));
      Assert(!oz_isRef(term));
      if (oz_isVarOrRef(term)) {
	oz_var_printStream(out, s,tagged2Var(term),depth);
      } else {
	out << s;
      }
      break;
    }
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    record2buffer(out,tagged2SRecord(term),depth);
    break;
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
    list2buffer(out,tagged2LTuple(term),depth);
    break;
  case LTAG_CONST0:
  case LTAG_CONST1:
    const2buffer(out,tagged2Const(term),'~',depth);
    break;
  case LTAG_LITERAL:
    {
      Literal *a = tagged2Literal(term);
      if (a->isAtom()) {
	atom2buffer(out,a);
      } else {
	name2buffer(out,a);
      }
      break;
    }
  case LTAG_SMALLINT:
    smallInt2buffer(out,term,'~');
    break;
  default:
    out << "<Unknown Tag: UNKNOWN >";
    break;
  }
}

char *OZ__toC(OZ_Term term, int depth, int width,int* len)
{
  static char *tmpString = 0;
  if (tmpString) {
    delete [] tmpString;
  }

  ostrstream *out = new ostrstream;

  oz_printStream(term,*out,depth,width);
  if (len!=0) *len = out->pcount();

  tmpString = strAndDelete(out);
  return tmpString;
}

char* OZ_toC(OZ_Term term, int depth, int width)
{
  return OZ__toC(term,depth,width,0);
}

char *toC(OZ_Term term)
{
  return OZ_toC(term,ozconf.errorPrintDepth,ozconf.errorPrintWidth);
}

int OZ_termGetSize(OZ_Term term, int depth, int width)
{
  ostrstream *out=new ostrstream;
  int old=ozconf.printWidth;

  ozconf.printWidth = width;
  term2Buffer(*out,term,depth);
  ozconf.printWidth = old;

  int ret = out->pcount ();
  delete out;
  return ret;
}

/*
 * Atoms
 */

OZ_CONST char *OZ_atomToC(OZ_Term term)
{
  term = oz_deref(term);

  Literal *a = tagged2Literal(term);
  return (OZ_CONST char *)a->getPrintName();
}

/* OZ_atom is defined in codearea.cc */

int OZ_boolToC(OZ_Term term)
{
  term = oz_deref(term);
  return oz_isTrue(term);
}

/* -----------------------------------------------------------------
 * virtual strings
 * -----------------------------------------------------------------*/

/* convert a C string (char*) to an Oz string */
OZ_Term OZ_string(OZ_CONST char *s) {
  return s ? oz_string(s,strlen(s),AtomNil) : AtomNil;
}

static void string2buffer(ostream &out,OZ_Term list,int nulok)
{
  OZ_Term tmp = oz_deref(list);
  for (; oz_isCons(tmp); tmp=oz_deref(oz_tail(tmp))) {
    OZ_Term hh = oz_deref(oz_head(tmp));
    if (!oz_isSmallInt(hh)) {
      message("no small int %s",toC(hh));
      printf(" in string %s\n",toC(list));
      return;
    }
    int i = tagged2SmallInt(hh);
    if (i < 0 || i > 255 || (i==0 && !nulok)) {
      message("no small int %d",i);
      printf(" in string %s\n",toC(list));
      return;
    }
    out << (char) i;
  }
  if (!oz_isNil(tmp)) {
    message("no string %s\n",toC(list));
  }
}

/*
 * convert Oz string to C string
 * PRE: list is a proper string
 */
char *OZ_stringToC(OZ_Term list,int*len)
{
  static char *tmpString = 0;
  if (tmpString) {
    delete [] tmpString;
    tmpString = 0;
  }

  ostrstream *out = new ostrstream;

  string2buffer(*out,list,0);

  if (len!=0) { *len = out->pcount(); }
  tmpString = strAndDelete(out);
  return tmpString;
}



inline
void vsatom2buffer(ostream &out, OZ_Term term)
{
  const char *s = tagged2Literal(term)->getPrintName();
  out << s;
}

#include "bytedata.hh"
inline
void byteString2buffer(ostream &out,OZ_Term term)
{
  ByteString* bs = tagged2ByteString(term);
  int n = bs->getWidth();
  for (int i=0;i<n;i++) out << bs->get(i);
}

void virtualString2buffer(ostream &out,OZ_Term term,int nulok)
{
  OZ_Term t=oz_deref(term);
  if (oz_isAtom(t)) {
    if (oz_isNil(t) || oz_isPair(t)) return;
    vsatom2buffer(out,t);
    return;
  }
  if (oz_isSmallInt(t)) {
    smallInt2buffer(out,t,'-');
    return;
  }
  if (oz_isCons(t)) {
    string2buffer(out,t,nulok);
    return;
  }
  if (oz_isBigInt(t)) {
    bigInt2buffer(out,tagged2BigInt(t),'-');
    return;
  }
  if (oz_isFloat(t)) {
    float2buffer(out,t,'-');
    return;
  }
  if (oz_isByteString(t)) {
    byteString2buffer(out,t);
    return;
  }

  if (!oz_isPair(t)) {
    OZ_error("no virtual string: %s",toC(term));
    return;
  }

  SRecord *sr=tagged2SRecord(t);
  int len=sr->getWidth();
  for (int i=0; i < len; i++) {
    virtualString2buffer(out,sr->getArg(i),nulok);
  }
}

/*
 * convert Oz virtual string to C string
 * PRE: list is a virtual string
 */
char *OZ_virtualStringToC(OZ_Term t,int*len)
{
  // tmpString is freed the next time OZ_virtualStringToC is called
  static char *tmpString = 0;
  if (tmpString) {
    delete [] tmpString;
    tmpString = 0;
  }

  ostrstream *out = new ostrstream;

  virtualString2buffer(*out,t,1);

  if (len!=0) { *len = out->pcount(); }
  tmpString = strAndDelete(out);
  return tmpString;
}

/* Here is an optimized version that checks for
 * the common cases
 */

inline char* dropConst(const char* s)
{
  union {
    const char* s1;
    char* s2;
  } u;
  u.s1 = s;
  return u.s2;
}

char* OZ_vsToC(OZ_Term t,int*n)
{
  char * s;
  if (OZ_isNil(t)) {
    static char *null = "";
    if (n!=0) *n = 0;
    return null;
  }
  else if (OZ_isAtom(t)) {
    s = dropConst(OZ_atomToC(t));
    if (n!=0) *n = strlen(s);
  } else {
    // Do not use bytestring directly, they are not null terminated! CS
    s = OZ_virtualStringToC(t,n);
  }
  return s;
}

/* -----------------------------------------------------------------
 * tuple
 * -----------------------------------------------------------------*/

OZ_Term OZ_label(OZ_Term term)
{
  DEREF(term,termPtr);

  switch (tagged2ltag(term)) {
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
    return AtomCons;
  case LTAG_LITERAL:
    return term;
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    return tagged2SRecord(term)->getLabel();
  default:
    OZ_error("OZ_label: no record");
    return 0;
  }
}

int OZ_width(OZ_Term term)
{
  DEREF(term,termPtr);

  switch (tagged2ltag(term)) {
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
    return 2;
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    return tagged2SRecord(term)->getWidth();
  case LTAG_LITERAL:
    return 0;
  default:
    OZ_error("OZ_width: no record");
    return 0;
  }
}

OZ_Term OZ_tuple(OZ_Term label, int width) 
{
  label = oz_deref(label);
  if (!oz_isLiteral(label)) {
    OZ_error("OZ_tuple: label is no literal");
    return 0;
  }

  if (width == 2 && oz_eq(label,AtomCons)) {
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
  term = oz_deref(term);
  Assert(!oz_isRef(term));
  if (oz_isLTupleOrRef(term)) {
    switch (pos) {
    case 0:
      tagged2LTuple(term)->setHead(newTerm);
      return;
    case 1:
      tagged2LTuple(term)->setTail(newTerm);
      return;
    }
  }
  if (!oz_isSTuple(term)) {
    OZ_error("OZ_putArg: no record");
    return;
  }
  tagged2SRecord(term)->setArg(pos,newTerm);
}

OZ_Term OZ_getArg(OZ_Term term, int pos)
{
  term = oz_deref(term);
  Assert(!oz_isRef(term));
  if (oz_isLTupleOrRef(term)) {
    switch (pos) {
    case 0:
      return tagged2LTuple(term)->getHead();
    case 1:
      return tagged2LTuple(term)->getTail();
    }
  }
  if (!oz_isSRecord(term)) {
    OZ_error("OZ_getArg: no record");
    return 0;
  }
  if (pos < 0 || pos >= tagged2SRecord(term)->getWidth()) {
    OZ_error("OZ_getArg: invalid index: %d",pos);
    return 0;
  }
  return tagged2SRecord(term)->getArg(pos);
}

OZ_Term OZ_nil()
{
  return oz_nil();
}

OZ_Term OZ_cons(OZ_Term hd,OZ_Term tl)
{
  return oz_cons(hd,tl);
}

OZ_Term OZ_head(OZ_Term term)
{
  term = oz_deref(term);
#ifdef DEBUG_CHECK
  Assert(!oz_isRef(term));
  if (!oz_isLTupleOrRef(term)) {
    OZ_error("OZ_head: no cons");
    return 0;
  }
#endif
  return oz_head(term);
}

OZ_Term OZ_tail(OZ_Term term)
{
  term = oz_deref(term);
#ifdef DEBUG_CHECK
  Assert(!oz_isRef(term));
  if (!oz_isLTupleOrRef(term)) {
    OZ_error("OZ_tail: no cons");
    return 0;
  }
#endif
  return oz_tail(term);
}

/*
 * Compute the length of a list
 */
int OZ_length(OZ_Term l)
{
  OZ_Term ret=oz_checkList(l);
  if (!oz_isSmallInt(ret)) return -1;
  return tagged2SmallInt(ret);
}


OZ_Term OZ_toList(int len, OZ_Term *tuple)
{
  OZ_Term l = oz_nil();
  while (--len >= 0) {
    l = oz_cons(tuple[len],l);
  }
  return l;
}

OZ_Term _OZ_LOC_TO_LIST(int n, OZ_Term **loc) {
  OZ_Term l = AtomNil;
  for (int i = n; n--; ) 
    l = oz_cons(*loc[i], l);
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
  return oz_pair2(t1,t2);
}

/* -----------------------------------------------------------------
 * record
 * -----------------------------------------------------------------*/

OZ_Arity OZ_makeArity(OZ_Term list)
{
  list = packsortlist(list);
  if (!list) return 0;
  return aritytable.find(list);
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
  term = oz_deref(term);
  Assert(!oz_isRef(term));
  if (oz_isLTupleOrRef(term)) {
    int i2 = tagged2SmallInt(feature);

    switch (i2) {
    case 1:
      tagged2LTuple(term)->setHead(value);
      return;
    case 2:
      tagged2LTuple(term)->setTail(value);
      return;
    }
    OZ_error("OZ_putSubtree: invalid feature");
    return;
  }
  if (!oz_isSRecord(term)) {
    OZ_error("OZ_putSubtree: invalid record");
    return;
  }
  if (!tagged2SRecord(term)->setFeature(feature,value)) {
    OZ_error("OZ_putSubtree: invalid feature");
    return;
  }
}

OZ_Term OZ_adjoinAt(OZ_Term rec, OZ_Term fea, OZ_Term val)
{
  rec = oz_deref(rec);
  fea = oz_deref(fea);
  if (!oz_isFeature(fea) || !oz_isRecord(rec)) return 0;

  if (oz_isLiteral(rec)) {
    SRecord *srec =
      SRecord::newSRecord(rec, aritytable.find(oz_cons(fea, oz_nil())));
    srec->setArg(0,val);
    return makeTaggedSRecord(srec);
  } else {
    return oz_adjoinAt(makeRecord(rec),fea,val);
  }
}

OZ_Term OZ_subtree(OZ_Term term, OZ_Term fea)
{
  DEREF(term,termPtr);
  fea=oz_deref(fea);

  switch (tagged2ltag(term)) {
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
    {
      if (!oz_isSmallInt(fea)) return 0;

      int i2 = tagged2SmallInt(fea);

      switch (i2) {
      case 1:
	return tagged2LTuple(term)->getHead();
      case 2:
	return tagged2LTuple(term)->getTail();
      }
      return 0;
    }
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    return tagged2SRecord(term)->getFeature(fea);

  case LTAG_CONST0:
  case LTAG_CONST1:
    {
      ConstTerm *ct = tagged2Const(term);
      switch (ct->getType()) {
      case Co_Extension:
	return tagged2Extension(term)->getFeatureV(fea);
      case Co_Object:
	return ((OzObject *) ct)->getFeature(fea);
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
  return oz_unify(t1,t2);
}

int OZ_eq(OZ_Term t1, OZ_Term t2)
{
  return oz_eq(oz_safeDeref(t1),oz_safeDeref(t2));
}


OZ_Term OZ_newVariable()
{
  return oz_newVariable();
}

/* -----------------------------------------------------------------
 * IO
 * -----------------------------------------------------------------*/

void OZ_registerReadHandler(int fd,OZ_IOHandler fun,void *val) {
  oz_io_select(fd,SEL_READ,fun,val);
}

void OZ_unregisterRead(int fd) {
  oz_io_deSelect(fd,SEL_READ);
}

void OZ_registerWriteHandler(int fd,OZ_IOHandler fun,void *val) {
  oz_io_select(fd,SEL_WRITE,fun,val);
}

void OZ_unregisterWrite(int fd) {
  oz_io_deSelect(fd,SEL_WRITE);
}

void OZ_registerAcceptHandler(int fd,OZ_IOHandler fun,void *val) {
  oz_io_acceptSelect(fd,fun,val);
}





OZ_Return OZ_readSelect(int fd,OZ_Term l,OZ_Term r) {
  return oz_io_select(fd,SEL_READ,l,r) ? PROCEED : FAILED;
}

OZ_Return OZ_writeSelect(int fd,OZ_Term l,OZ_Term r) {
  return oz_io_select(fd,SEL_WRITE,l,r) ? PROCEED : FAILED;
}

OZ_Return OZ_acceptSelect(int fd,OZ_Term l,OZ_Term r) {
  oz_io_acceptSelect(fd,l,r);
  return PROCEED;
}

void OZ_deSelect(int fd) {
  oz_io_deSelect(fd);
}


/* -----------------------------------------------------------------
 * garbage collection
 * -----------------------------------------------------------------*/

int OZ_protect(OZ_Term *t)
{
  if (!oz_protect(t)) {
    return 0;
  }
  return 1;
}

int OZ_unprotect(OZ_Term *t)
{
  if (!oz_unprotect(t)) {
    return 0;
  }
  return 1;
}

void OZ_gCollect(OZ_Term * to) {
  oz_gCollectTerm(*to, *to);
}

void (*OZ_sCloneBlockDynamic)(OZ_Term *,OZ_Term *,const int) = NULL;

void OZ_sClone(OZ_Term * to) {
  (*OZ_sCloneBlockDynamic)(to, to, 1);
}

/* -----------------------------------------------------------------
 * vs
 * -----------------------------------------------------------------*/


static
int oz_isVirtualStringNoZero(OZ_Term vs, OZ_Term * var) {
  if (oz_isRef(vs)) {
    DEREF(vs,vsPtr);
    Assert(!oz_isRef(vs));
    if (oz_isVarOrRef(vs))  {
      if (var) 
	*var = makeTaggedRef(vsPtr);
      return 0;
    }
  }

  if (oz_isInt(vs) || oz_isFloat(vs) || oz_isAtom(vs) ||
      oz_isByteString(vs))
    return 1;

  if (oz_isPair(vs)) {
    SRecord * sr = tagged2SRecord(vs);
    for (int i = sr->getWidth(); i--;) {
      if (!oz_isVirtualStringNoZero(sr->getArg(i),var)) 
	return 0;
    }
    return 1;
  }

  Assert(!oz_isRef(vs));
  if (oz_isLTupleOrRef(vs)) {
    OZ_Term ret = oz_checkList(vs,OZ_CHECK_CHAR_NONZERO);
    if (oz_isRef(ret)) {
      if (var) 
	*var = ret;
      return 0;
    }
    if (var) 
      *var = 0;
    return oz_isFalse(ret) ? 0 : 1;
  }

  return 0;
}

int OZ_isVirtualStringNoZero(OZ_Term vs, OZ_Term * var)
{
  if (var) 
    *var = 0;

  return oz_isVirtualStringNoZero(vs,var);
}



static
int oz_isVirtualString(OZ_Term vs, OZ_Term * var) {
  if (oz_isRef(vs)) {
    DEREF(vs,vsPtr);
    Assert(!oz_isRef(vs));
    if (oz_isVarOrRef(vs))  {
      if (var) 
	*var = makeTaggedRef(vsPtr);
      return 0;
    }
  }

  if (oz_isInt(vs) || oz_isFloat(vs) || oz_isAtom(vs) ||
      oz_isByteString(vs))
    return 1;

  if (oz_isPair(vs)) {
    SRecord * sr = tagged2SRecord(vs);
    for (int i = sr->getWidth(); i--;) {
      if (!oz_isVirtualString(sr->getArg(i),var)) 
	return 0;
    }
    return 1;
  }

  Assert(!oz_isRef(vs));
  if (oz_isLTupleOrRef(vs)) {
    OZ_Term ret = oz_checkList(vs,OZ_CHECK_CHAR);
    if (oz_isRef(ret)) {
      if (var) 
	*var = ret;
      return 0;
    }
    if (var) 
      *var = 0;
    return oz_isFalse(ret) ? 0 : 1;
  }

  return 0;
}

int OZ_isVirtualString(OZ_Term vs, OZ_Term * var)
{
  if (var) 
    *var = 0;

  return oz_isVirtualString(vs,var);
}


/* -----------------------------------------------------------------
 * names
 * -----------------------------------------------------------------*/

OZ_Term OZ_newName()
{
  return oz_newName();
}

// don't raise errors, because debug info is never included!
// use OZ_raiseError
OZ_Return OZ_raise(OZ_Term exc) {
  am.setException(exc,FALSE);
  return RAISE;
}

// OZ_raiseDebug(E) raises exception E, but adds debuging information
// if E is a record with feature `debug' and subtree `unit', and
// E either (1) has label `error' or (2) ozconf.errorDebug is true

OZ_Return OZ_raiseDebug(OZ_Term exc) {
  int debug = FALSE;
  if (OZ_isRecord(exc)) {
    OZ_Term x = OZ_subtree(exc,AtomDebug);
    if (x && OZ_eq(x,oz_unit())) {
      debug = oz_eq(OZ_label(exc),E_ERROR) || ozconf.errorDebug;
    }
  }
  am.setException(exc,debug);
  return RAISE;
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

OZ_Return OZ_raiseError(OZ_Term exc) {
  OZ_Term ret = OZ_record(AtomError,
			  oz_mklist(makeTaggedSmallInt(1),AtomDebug));
  OZ_putSubtree(ret, makeTaggedSmallInt(1), exc);
  OZ_putSubtree(ret, AtomDebug,      NameUnit);
  
  am.setException(ret,TRUE);
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
 
OZ_Term OZ_makeException(OZ_Term cat,OZ_Term key,char*label,int arity,...)
{
  OZ_Term exc=OZ_tuple(key,arity+1);
  OZ_putArg(exc,0,OZ_atom(label));

  va_list ap;
  va_start(ap,arity);

  for (int i = 0; i < arity; i++) {
    OZ_putArg(exc,i+1,va_arg(ap,OZ_Term));
  }

  va_end(ap);


  OZ_Term ret = OZ_record(cat,oz_mklist(makeTaggedSmallInt(1),AtomDebug));
  OZ_putSubtree(ret, makeTaggedSmallInt(1), exc);
  OZ_putSubtree(ret, AtomDebug,      NameUnit);
  return ret;
}

/* -----------------------------------------------------------------
 * threads
 * -----------------------------------------------------------------*/

void OZ_pushCall(OZ_Thread thr,OZ_Term proc,OZ_Term *args,int arity)
{
  ((Thread *)thr)->pushCall(proc, RefsArray::make(args, arity));
}

OZ_Thread OZ_newSuspendedThread()
{
#ifdef SHOW_SUSPENSIONS
  static int xxx=0;
  printf("Suspension(%d):",xxx++);
  for(int i=0; i<arity; i++) {
    printf("%s, ",toC(args[i]));
  }
  printf("\n");
#endif

  return (OZ_Thread) oz_newThreadSuspended();
}

OZ_Thread OZ_makeSuspendedThread(OZ_Term proc,OZ_Term *args,int arity)
{
  Thread *thr=oz_newThreadSuspended();
  thr->pushCall(proc,RefsArray::make(args,arity));
  return (OZ_Thread) thr;
}

OZ_Thread OZ_newRunnableThread()
{
  return (OZ_Thread) oz_newThread();
}

void OZ_makeRunnableThread(OZ_Term proc, OZ_Term *args,int arity)
{
  Thread *thr = oz_newThread();
  thr->pushCall(proc,RefsArray::make(args,arity));
}

void OZ_unifyInThread(OZ_Term val1,OZ_Term val2)
{
  int ret = oz_unify(val1,val2);
  if (ret == PROCEED) return;
  switch (ret) {
  case SUSPEND:
    {
      Thread *thr = oz_newThreadSuspended();
      thr->pushCall(BI_Unify,RefsArray::make(val1,val2));
      ret = am.suspendOnVarList(thr);
      if (ret == PROCEED) oz_wakeupThread(thr);
      if (ret != SUSPEND) {
	am.emptyPreparedCalls();
	oz_wakeupThread(thr);
      }
      break;
    }
  case BI_REPLACEBICALL:
    {
      OZ_Thread thr = oz_newThread();
      am.pushPreparedCalls((Thread *) thr);
      break;
    }
  default:
    {
      Thread *thr = oz_newThread();
      thr->pushCall(BI_Unify,RefsArray::make(val1,val2));
      break;
    }
  }
}

/* Suspensions */
// mm2: bug
void OZ_addThread(OZ_Term var, OZ_Thread thr)
{
  DEREF(var, varPtr);
  Assert(!oz_isRef(var));
  if (!oz_isVarOrRef(var)) {
    OZ_error("OZ_addThread(%s): var arg expected", toC(var));
    return;
  }

  oz_var_addSusp(varPtr, (Thread *) thr); //mm2 handle return
}

OZ_Return OZ_suspendOnInternal(OZ_Term var)
{
  oz_suspendOn(var);
}

OZ_Return OZ_suspendOnInternal2(OZ_Term var1,OZ_Term var2)
{
  oz_suspendOn2(var1,var2);
}

OZ_Return OZ_suspendOnInternal3(OZ_Term var1,OZ_Term var2,OZ_Term var3)
{
  oz_suspendOn3(var1,var2,var3);
}

/* -----------------------------------------------------------------
 * 
 * -----------------------------------------------------------------*/

OZ_Term OZ_newCell(OZ_Term val)
{
  return oz_newCell(val);
}

OZ_Term OZ_newChunk(OZ_Term val)
{
  val=oz_deref(val);
  if (!oz_isRecord(val)) return 0;
  return oz_newChunk(oz_currentBoard(),val);
}

int OZ_onToplevel()
{
  return oz_onToplevel();
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
  oz_typeError(pos,type);
}

void OZ_main(int argc,char **argv)
{
  am.init(argc,argv);
  scheduler();
  am.exitOz(0);
}

OZ_Term OZ_newPort(OZ_Term val) 
{
  return oz_newPort(val);
}

void OZ_send(OZ_Term port, OZ_Term val)
{
  port = oz_deref(port);
  if (!oz_isPort(port)) return;

  (void) oz_sendPort(port,val);
}

// mm2: this is not longer needed in Oz 3.0, but for compatibility with
// modules compiled for Oz 2.0 OZ_raiseA has to be defined
extern "C" OZ_Return OZ_raiseA(char*, int, int);
OZ_Return OZ_raiseA(char *name, int was, int shouldBe)
{
  return oz_raise(E_ERROR,E_SYSTEM,"inconsistentArity",3,
		  OZ_atom(name),OZ_int(was),OZ_int(shouldBe));
}


