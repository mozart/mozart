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

#include "oz.h"

#include "am.hh"

#include "StringBuffer.hh"

/* TmpBuffer
   must be sufficiently large to convert
     smallInts and floats to strings/BigInts */
StringBuffer TmpBuffer;

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


inline
OZ_Return suspendOnVar(OZ_Term *varPtr)
{
  am.addSuspendVarList(varPtr);
  return SUSPEND;
}

/*
 * list checking
 */

inline
OZ_Return isList(OZ_Term l, Bool checkChar)
{
  while (1) {
    DEREF(l,lPtr,lTag)
    if (isAnyVar(lTag)) return suspendOnVar(lPtr);

    if (isCons(lTag)) {
      if (checkChar) {
        OZ_Term h = head(l);
        DEREF(h,hPtr,hTag);
        if (isAnyVar(hTag)) {
          return suspendOnVar(hPtr);
        }
        if (!isSmallInt(hTag)) return FAILED;
        int i=smallIntValue(h);
        if (i<0 || i>255) return FAILED;
      }
      l = tail(l);
    } else if (isNil(l)) {
      return PROCEED;
    } else {
      return FAILED;
    }
  }
}

OZ_Return OZ_isString(OZ_Term term)
{
  return isList(term,OK);
}

OZ_Return OZ_isList(OZ_Term term)
{
  return isList(term,NO);
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
    return AtomVariable;
  }

  if (isInt(term)) {
    return AtomInt;
  }

  if (isFloat(term)) {
    return AtomFloat;
  }

  if (isLiteral(term)) {
    return (tagged2Literal(term)->isAtom() ? AtomAtom : AtomName);
  }

  if (isTuple(term)) {
    return AtomTuple;
  }

  if (isProcedure(term)) {
    return AtomProcedure;
  }

  if (isCell(term)) {
    return AtomCell;
  }

  if (isChunk(term)) {
    return AtomChunk;
  }

  if (isSRecord(term)) {
    return AtomRecord;
  }

  return AtomUnknown;
}

/* -----------------------------------------------------------------
 * providing constants
 *------------------------------------------------------------------*/

int OZ_getMinPrio(void)
{
  return OZMIN_PRIORITY;
}

int OZ_getMaxPrio(void)
{
  return OZMAX_PRIORITY ;
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
    return NULL;
  }
  while (isdigit(*p)) {
    p++;
  }
  return p;
}


char *OZ_intToCString(OZ_Term term)
{
  term = deref(term);
  if (isSmallInt(term)) {
    TmpBuffer.reset();
    TmpBuffer.put_int(smallIntValue(term));
    char *s = TmpBuffer.string();
    if (s[0] == '-') s[0] = '~';
    return s;
  }
  BigInt *bb = tagged2BigInt(term);
  TmpBuffer.reset();
  char *str = TmpBuffer.allocate(bb->stringLength());
  bb->getString(str);
  if (*str == '-') *str = '~';
  return TmpBuffer.string();
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

char *OZ_floatToCString(OZ_Term term)
{
  term = deref(term);

  double f = floatValue(term);
  TmpBuffer.reset();
  TmpBuffer.put_float(f);
  char *str = ozstrdup(TmpBuffer.string());

  // normalize float
  Bool hasDot = NO;
  Bool hasE = NO;
  Bool hasDigits = NO;
  TmpBuffer.reset();
  char *s = str;
  for (char c=*s++; c ; c=*s++) {
    switch (c) {
    case 'e':
      if (!hasDot) TmpBuffer.put('.');
      TmpBuffer.put(c);
      break;
    case '.':
      if (!hasDigits) TmpBuffer.put('0');
      hasDot = OK;
      TmpBuffer.put(c);
      break;
    case '-':
      TmpBuffer.put('~');
      break;
    case '+':
      break;
    default:
      if (isdigit(c)) hasDigits=OK;
      TmpBuffer.put(c);
      break;
    }
  }
  delete [] str;
  return TmpBuffer.string();
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
 * Literals
 */
char *literalToC(OZ_Term term)
{
  term = deref(term);

  Literal *a = tagged2Literal(term);
  char *s = a->getPrintName();
  if (a->isAtom()) {
    return s;
  }
  int len = strlen(s)+20;
  TmpBuffer.reset();
  char *tmp = TmpBuffer.allocate(len);
  sprintf(tmp,"N:%s-%d",s,a->getSeqNumber());
  return tmp;
}


int OZ_featureCmp(OZ_Term term1, OZ_Term term2)
{
  term1 = deref(term1);
  term2 = deref(term2);
  return featureCmp(term1,term2);
}

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

/*
 * Any
 */

char *toC(OZ_Term term)
{
  return OZ_toC(term,ozconf.errorPrintDepth);
}


char *OZ_toC(OZ_Term term, int depth)
{
  if (!term) {
    return "*** NULL TERM ***";
  }

  DEREF(term,termPtr,tag)
  switch(tag) {
  case UVAR:
  case SVAR:
  case CVAR:
    return getVarName(term);
  case SRECORD:
  case LTUPLE:
    return tagged2String(term, depth);
  case OZCONST:
    return tagged2String(term, depth);;
  case LITERAL:
    return literalToC(term);
  case OZFLOAT:
    return OZ_floatToCString(term);
  case BIGINT:
  case SMALLINT:
    return OZ_intToCString(term);

  default:
    break;
  }

  warning("OZ_toC: failed");
  return 0;
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
  TmpBuffer.reset();

  for (OZ_Term tmp = deref(list); isCons(tmp); tmp=deref(tail(tmp))) {
    OZ_Term hh = deref(head(tmp));
    int i = smallIntValue(hh);
    if (i < 0 || i > 255) return 0;
    TmpBuffer.put(i);
  }
  return TmpBuffer.string();
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
    char *s=OZ_intToCString(t);
    printf("%s",s);
  }
}

void OZ_printFloat(OZ_Term t)
{
  char *s=OZ_floatToCString(t);
  printf("%s",s);
}

void OZ_printVS(OZ_Term t)
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

OZ_Term OZ_termToVS(OZ_Term t)
{
  t=deref(t);

  switch (tagTypeOf(t)) {
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
    return OZ_atom(toC(t));
  case LITERAL:
    if (isAtom(t)) return t;
    return OZ_atom(toC(t));
  default:
    return OZ_atom("OZ_termToVS: unknown Tag");
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

OZ_Return OZ_isVirtualString(OZ_Term vs)
{
  DEREF(vs,vsPtr,vsTag);
  if (isAnyVar(vsTag)) return suspendOnVar(vsPtr);

  if (isInt(vs) || isFloat(vs) || isAtom(vs))  return PROCEED;

  if (isPair(vs)) {
    SRecord *sr = tagged2SRecord(vs);
    int len = sr->getWidth();
    for (int i=0; i < len; i++) {
      OZ_Return argstate = OZ_isVirtualString(sr->getArg(i));
      if (argstate!=PROCEED) return argstate;
    }
    return PROCEED;
  }

  if (isCons(vs)) return isList(vs,OK);

  return FAILED;
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

OZ_Thread OZ_makeThread(OZ_Return (*fun)(int,OZ_Term[]),
                        OZ_Term *args,int arity)
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
  return makeTaggedConst(new SChunk(am.currentBoard, tagged2SRecord(val)));
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
