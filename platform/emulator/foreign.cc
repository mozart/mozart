/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <strstream.h>

#include "oz.h"

#include "am.hh"

#include "genvar.hh"
#include "ofgenvar.hh"

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


/*
 * list checking
 */

inline
int isList(OZ_Term l, OZ_Term *var, Bool checkChar)
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
  return isList(term,var,OK);
}

int OZ_isList(OZ_Term term,OZ_Term *var)
{
  if (var) *var = 0;
  return isList(term,var,NO);
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

  OZ_warning("OZ_termType: %s unknown type\n",toC(term));
  Assert(0);
  return 0;
}

/* -----------------------------------------------------------------
 * providing constants
 *------------------------------------------------------------------*/

int OZ_getMinPrio(void) 
{
  return OZMIN_PRIORITY;
}

int OZ_getDefaultPrio(void) 
{
  return DEFAULT_PRIORITY;
}

int OZ_getPropagatorPrio(void) 
{
  return PROPAGATOR_PRIORITY;
}

int OZ_getMaxPrio(void) 
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

/*
 * PRE: OZ_parseInt was successful !
 */
OZ_Term OZ_CStringToInt(char *str)
{
  Assert(str != NULL && str[0] != '\0');

  {
    char *aux = str;
    int sign = 1;
    if (aux[0] == '~') { aux++; sign = -1; }
    int i = 0;
    while(*aux) {
      Assert(isdigit(*aux));
      i = i*10 + (*aux - '0');
      if (i > OzMaxInt)
	goto bigInt;
      aux++;
    }

    i *= sign;
    return newSmallInt(i);
  }
  
 bigInt:
  {
    int sign = 1;
    Assert(str[0] != '-');

    if (str[0] == '~') { str[0] = '-'; sign = -1; }
    OZ_Term ret = (new BigInt(str))->shrink();

    // undo destructive change of str
    if (sign < 0) str[0] = '-';

    return ret;
  }

}

/*
 * parse: [~]<int>.<digit>*[(e|E)<int>]
 */
char *OZ_parseFloat(char *s) {
  char *p = OZ_parseInt(s);
  if (!p || *p++ != '.') {
    return NULL;
  }
  while (isdigit(*p)) {
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
  if (!isdigit(*p++)) {
    return 0;
  }
  while (isdigit(*p)) {
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
      if (isdigit(c)) hasDigits=OK;
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
    if (iscntrl(c)) {
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
    } else if (c >= 127) {
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
  unsigned char c = *s++;
  if (!c || !islower(c)) {
    return NO;
  }
  c=*s++;
  while (c) {
    if (!isalnum(c) && c != '_') {
      return NO;
    }
    c=*s++;
  }
  return OK;
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
void name2buffer(ostream &out, Literal *a)
{
  char *s = a->getPrintName();
  if (!*s) {
    out << "<N>";
  } else {
    out << "<N: " << s << '>';
  }
}

inline
void const2buffer(ostream &out, ConstTerm *c)
{
  char *s = c->getPrintName();

  switch (c->getType()) {
  case Co_Abstraction:
  case Co_Builtin:
    {
      int arity = 
	(c->getType() == Co_Abstraction)
	? ((Abstraction *) c)->getArity()
	: ((Builtin *) c)->getArity();

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
  case Co_Chunk:
  case Co_Array:
  case Co_Dictionary:
    out << "<Chunk>";
    break;
  default:
    Assert(0);
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
  out << ": ";
  value2buffer(out,sr->getFeature(fea),depth);
}

inline
void record2buffer(ostream &out, SRecord *sr,int depth)
{
  value2buffer(out,sr->getLabel());
  out << '(';
  if (depth <= 0) {
    out << ",,,";
  } else {
    if (sr->isTuple()) {
      int len=sr->getWidth();
      value2buffer(out,sr->getArg(0),depth-1);
      for (int i=1; i < len; i++) {
	out << ' ';
	value2buffer(out,sr->getArg(i),depth-1);
      }
    } else {
      TaggedRef as = sr->getArityList();
      Assert(isCons(as));
      feature2buffer(out,sr,head(as),depth-1);
      as = tail(as);
      while (isCons(as)) {
	out << ' ';
	feature2buffer(out,sr,head(as),depth-1);
	as = tail(as);
      }
    }
  }
  out << ')';
}

static
int listWidth = 0;

inline
void list2buffer(ostream &out, LTuple *list,int depth)
{
  int width = listWidth;
  while (width-- > 0) {
    OZ_Term a=deref(list->getHead());
    if (isCons(a)) {
      out << '(';
      value2buffer(out,list->getHead(),depth-1);
      out << ')';
    } else {
      value2buffer(out,list->getHead(),depth-1);
    }
    out << '|';
    OZ_Term t=deref(list->getTail());
    if (!isCons(t)) {
      value2buffer(out,t,depth);
      return;
    }
    list = tagged2LTuple(t);
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
    out << ": ";
    value2buffer(out,lookup(arr[ai]),depth);
    out << ' ';
  }
  // Output the Names last, unordered:
  for (di=0; di<size; di++) {
    tmplit=table[di].ident;
    tmpval=table[di].value;
    if (tmpval!=makeTaggedNULL() && !(isAtom(tmplit)||isInt(tmplit))) {
      value2buffer(out,tmplit,0);
      out << ": ";
      value2buffer(out,tmpval,depth);
      out << ' ';
    }
  }
  // Deallocate array:
  delete arr;
  return out;
}

static
void cvar2buffer(ostream &out, char *s,GenCVariable *cv,int depth)
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

  case MetaVariable:
    {
      out << s;
      // TmpBuffer.print_string(((GenMetaVariable *)cv)->toString(0));
      break;
    }
  case AVAR:
  case DVAR:
    {
      out << s;
      break;
    }
  default:
    Assert(0);
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
      Assert(0);
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

  for (OZ_Term tmp = deref(list); isCons(tmp); tmp=deref(tail(tmp))) {
    OZ_Term hh = deref(head(tmp));
    int i = smallIntValue(hh);
    if (i < 0 || i > 255) return 0;
    out << (char) i;
  }
  out << ends;
  tmpStr = out.str();
  return tmpStr;
}

void OZ_printString(OZ_Term t)
{
  t=deref(t);
  while (isCons(t)) {
    OZ_Term hd=deref(head(t));
    int c=smallIntValue(hd);
    printf("%c",c);
    t=deref(tail(t));
  }
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

void OZ_printVirtualString(OZ_Term t)
{
  t=deref(t);
  if (isCons(t)) {
    OZ_printString(t);
  } else if (isAtom(t)) {
    if (isNil(t) || isPair(t)) {
      return;
    }
    OZ_printAtom(t);
  } else if (isInt(t)) {
    OZ_printInt(t);
  } else if (isFloat(t)) {
    OZ_printFloat(t);
  } else {
    Assert(isPair(t));
    SRecord *sr=tagged2SRecord(t);
    int len=sr->getWidth();
    for (int i=0; i < len; i++) {
      OZ_printVS(sr->getArg(i));
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
    Assert(0);
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
    Assert(0);
    return 0;
  }
}

OZ_Term OZ_tuple(OZ_Term label, int width) 
{
  label = deref(label);
  Assert(isLiteral(label));

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
  Assert(isSTuple(term));
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
  Assert(pos >= 0 &&
	 pos < tagged2SRecord(term)->getWidth());
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
  return head(term);
}

OZ_Term OZ_tail(OZ_Term term)
{
  term=deref(term);
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
    Assert(0);
  }
  (void) tagged2SRecord(term)->replaceFeature(feature,value);
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

OZ_Return OZ_readSelect(int fd,OZ_Term l,OZ_Term r)
{
  return am.select(fd,SEL_READ,l,r) ? PROCEED : FAILED;
}

OZ_Return OZ_writeSelect(int fd,OZ_Term l,OZ_Term r)
{
  return am.select(fd,SEL_WRITE,l,r) ? PROCEED : FAILED;
}

OZ_Return OZ_acceptSelect(int fd,OZ_Term l,OZ_Term r)
{
  am.acceptSelect(fd,l,r);
  return PROCEED;
}

void OZ_deSelect(int fd)
{
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

  if (isCons(vs)) return isList(vs,var,OK);

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
  return makeTaggedLiteral(new Literal(am.currentBoard));
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
  am.exception=exc;
  return RAISE;
}

/* -----------------------------------------------------------------
 * Suspending builtins
 * -----------------------------------------------------------------*/

OZ_Thread OZ_makeSuspendedThread(OZ_CFun fun,OZ_Term *args,int arity)
{
#ifdef SHOW_SUSPENSIONS
  static int xxx=0;
  printf("Suspension(%d):",xxx++);
  for(int i=0; i<arity; i++) {
    printf("%s, ",tagged2String(args[i],2));
  }
  printf("\n");
#endif

  /* create a CFuncContinuation */
  return (OZ_Thread)
    new Thread (am.currentBoard,
		ozconf.defaultPriority,
		fun, args, arity);
}

void OZ_makeRunableThread(OZ_CFun fun, OZ_Term *args,int arity)
{
  Thread *tt = new Thread (am.currentBoard,
			   ozconf.defaultPriority,
			   fun, args, arity);
  am.scheduleThread(tt);
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
  return OZ_raise(OZ_mkTupleC("typeError",1,
			      OZ_mkTupleC("pos",2,OZ_int(pos+1),
					  OZ_atom(type))));
}

void OZ_main(int argc,char **argv)
{
  am.init(argc,argv);
  engine();
  am.exitOz(0);
}
