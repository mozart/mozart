/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */


#include "foreign.h"


#include "am.hh"
#include "bignum.hh"
#include "builtins.hh"
#include "cell.hh"
#include "io.hh"
#include "records.hh"
#include "thread.hh"
#include "suspension.hh"

int OZ_isInt(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isInt(tag) == OK) ? 1 : 0;
}

int OZ_isFloat(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isFloat(tag) == OK) ? 1 : 0;
}

int OZ_isAtom(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isXAtom(term) == OK) ? 1 : 0;
}

int OZ_isVar(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isAnyVar(tag) == OK) ? 1 : 0;
}

int OZ_isNotCVar(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isNotCVar(tag) == OK) ? 1 : 0;
}

int OZ_isCVar(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isCVar(tag) == OK) ? 1 : 0;
}

int OZ_isTuple(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isTuple(tag) == OK) ? 1 : 0;
}

int OZ_isRecord(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isRecord(tag) == OK) ? 1 : 0;
}

int OZ_isCell(OZ_Term term)
{
  DEREF(term,_1,_2);
  return (isCell(term) == OK) ? 1 : 0;
}

/* ----------------------------------------------------------------- */


// mm2
int OZ_termToInt(OZ_Term term, int* n) {
  if (OZ_isInt(term)) {
    *n = OZ_OzIntToCInt(term);
    return 1;
  }
  return 0;
}

int OZ_OzIntToCInt(OZ_Term term)
{
  DEREF(term,_1,tag);

  if (isSmallInt(tag)) {
    return smallIntValue(term);
  } else if (isBigInt(term)) {
    return tagged2BigInt(term)->BigInt2Int();
  }
  OZ_warning("OzIntToCInt(%s): int arg expected",tagged2String(term));
  return 0;
}

// mm2
int OZ_termToFloat(OZ_Term term, OZ_Float *n)
{
  if (OZ_isFloat(term)) {
    *n = OZ_OzFloatToCFloat(term);
    return 1;
  }
  return 0;
}

OZ_Float OZ_OzFloatToCFloat(OZ_Term term)
{
  DEREF(term,_1,tag);

  if (isFloat(tag)) {
    return floatValue(term);
  }
  OZ_warning("OzFloatToCFloat(%s): float arg expected",tagged2String(term));
  return 0.0;
}

// mm2
int OZ_termToString(OZ_Term term, char **s)
{
  if (OZ_isAtom(term)) {
    *s = OZ_OzAtomToCString(term);
    return 1;
  }
  return 0;
}

char *OZ_OzAtomToCString(OZ_Term term)
{
  DEREF(term,_1,tag);
  if (isXAtom(term)) {
    return tagged2Atom(term)->getPrintName();
  }
  OZ_warning("OzAtomToCString(%s): atom arg expected",tagged2String(term));
  return NULL;
}

// mm2
char *OZ_termToStr(OZ_Term term)
{
  return OZ_OzTermToCString(term);
}

char *OZ_OzTermToCString(OZ_Term term)
{
  return tagged2String(term);
}

// mm2
char *OZ_intTermToString(OZ_Term term)
{
  return OZ_OzIntToCString(term);
}

char *OZ_OzIntToCString(OZ_Term term)
{
  DEREF(term,_1,tag);

  if (isSmallInt(term)) {
    char buf[1000];
    sprintf(buf,"%d",smallIntValue(term));
    return ozstrdup(buf);
  }

  if (isBigInt(term)) {
    char *s =  tagged2BigInt(term)->string();
    if (s[0] == '~') {
      s[0] = '-';
    }
    return s;
  }

  OZ_warning("OzIntToCString(%s): int arg expected",tagged2String(term));
  return NULL;
}


// mm2
OZ_Term OZ_intToTerm (int i)
{
  return intToTerm(i);
}

OZ_Term OZ_CIntToOzInt(int i)
{
  return intToTerm(i);
}

// mm2
OZ_Term OZ_floatToTerm (OZ_Float i)
{
  return floatToTerm(i);
}

OZ_Term OZ_CFloatToOzFloat (OZ_Float i)
{
  return floatToTerm(i);
}

// mm2
OZ_Term OZ_stringToTerm(char *s)
{
  return makeTaggedAtom(s);
}

OZ_Term OZ_CStringToOzAtom(char *s)
{
  return makeTaggedAtom(s);
}

// mm2
OZ_Term OZ_numberToTerm(char *s)
{
  return numberToTerm(s);
}

OZ_Term OZ_CStringToOzNumber(char *s)
{
  return numberToTerm(s);
}

/* ----------------------------------------------------------------- */

OZ_Bool OZ_unify(OZ_Term t1, OZ_Term t2)
{
  if (!am.unify(t1, t2)) {
    return FAILED;
  }
  return PROCEED;
}

OZ_Term OZ_newVariable(char *name)
{
  SVariable *cvar = new SVariable(am.currentBoard,
                                  OZ_stringToTerm(name));
  return makeTaggedRef(newTaggedSVar(cvar));
}

OZ_Term OZ_newVar()
{
  return makeTaggedRef(newTaggedUVar(am.currentBoard));
}

int OZ_label(OZ_Term term, char **label)
{
  Atom *a;

  DEREF(term,_,tag);
  switch (tag) {
  case LTUPLE:
    a = tagged2LTuple(term)->getLabelAtom();
    break;
  case STUPLE:
    a = tagged2STuple(term)->getLabelAtom();
    break;
  case ATOM:
    a = tagged2Atom(term);
    break;
  case SRECORD:
    a = tagged2Atom(tagged2SRecord(term)->getLabel());
    break;

  default:
    return 0;
  }

  if (a == NULL) {
    return 0;
  }

  *label = a->getPrintName();
  return 1;
}


int OZ_width(OZ_Term term, int *width)
{
  DEREF(term,_,tag);

  switch (tag) {
  case LTUPLE:
    *width = 2;
    return 1;
  case STUPLE:
    *width  = tagged2STuple(term)->getSize ();
    return 1;
  case SRECORD:
    *width = tagged2SRecord(term)->getWidth();
    return 1;
  case ATOM:
    *width = 0;
    return 1;
  default:
    return 0;
  }
}

OZ_Term OZ_makeTuple(char *label, int width)
{
  if ((width == 2) && (strcmp(label,NameOfCons) == 0)) {
    // have to make a list
    return makeTaggedLTuple(new LTuple());
  } else if (width <= 0) {
    return OZ_stringToTerm(label);
  }
  return makeTaggedSTuple(STuple::newSTuple(addToAtomTab(label),width));
}

int OZ_putArg(OZ_Term term, int pos, OZ_Term newTerm)
{
  DEREF(term,_1,tag);

  if (isLTuple(tag)) {
    switch (pos) {
    case 1:
      tagged2LTuple(term)->setHead(newTerm);
      return 1;
    case 2:
      tagged2LTuple(term)->setTail(newTerm);
      return 1;
    default:
      return 0;
    }
  }
  if (isSTuple(term) && (pos >= 1) && pos <= tagged2STuple(term)->getSize ()) {
    tagged2STuple(term)->setArg(pos-1,newTerm);
    return 1;
  }

  return 0;
}

OZ_Term OZ_getArg(OZ_Term term, int pos)
{
  DEREF(term,_1,tag);
  if (isLTuple(tag)) {
    switch (pos) {
    case 1:
      return tagged2LTuple(term)->getHead();
    case 2:
      return tagged2LTuple(term)->getTail();
    default:
      return 0;
    }
  }
  if (isSTuple(term) && (pos >= 1) && pos <= tagged2STuple(term)->getSize ())
    return tagged2STuple(term)->getArg(pos-1);

  return 0;
}


OZ_Term OZ_getRecordArg(OZ_Term term, char *fea)
{
  DEREF(term,_1,tag);
  if (isSRecord(tag)) {
    return tagged2SRecord(term)
      ->getFeature(OZ_stringToTerm(fea));
  }
  return 0;
}


// ----------------------------------------------------------------------
//          IO
// ----------------------------------------------------------------------

int OZ_select(int fd) {
  if (IO::setIORequest(fd) == NO) {
    return 0;
  }
  return 1;
}

int OZ_openIO(int fd) {
  IO::openIO(fd);
  return 1;
}

int OZ_closeIO(int fd) {
  IO::closeIO(fd);
  return 1;
}


int OZ_protect(OZ_Term *t) {
  if (!gcProtect(t)) {
    OZ_warning("protect: failed");
    return 0;
  }
  return 1;
}

int OZ_unprotect(OZ_Term *t) {
  if (!gcUnprotect(t)) {
    OZ_warning("unprotect: failed");
    return 0;
  }
  return 1;
}


OZ_Term OZ_protectCopy(OZ_Term t) {
  return gcProtectCopy(t);
}

  int OZ_unprotectCopy(OZ_Term t) {
  if (!isRef(t)) {
    OZ_warning("unprotectCopy: no REF");
    return 0;
  }
  gcUnprotectCopy(tagged2Ref(t));
  return 1;
}


void OZ_print(OZ_Term t)
{
  taggedPrint(t,am.conf.printDepth);
  fflush(stdout);
}

OZ_Term OZ_nil()
{
  return nil();
}

OZ_Term OZ_cons(OZ_Term head,OZ_Term tail)
{
  return cons(head,tail);
}


int addBuiltin(char *name, int arity, OZ_CFun fun)
{
  return BIreplace(name,arity,fun) == NULL ? 0 : 1;
}



/* Suspending builtins */


Suspension *OZ_makeSuspension(OZ_Bool (*fun)(int,OZ_Term[]),
                                 OZ_Term *args,int arity)
{
  am.currentBoard->addSuspension();
  return new Suspension(new CFuncContinuation(am.currentBoard,
                                              am.currentThread->getPriority(),
                                              fun, args, arity));
}

Suspension *OZ_makeHeadSuspension(OZ_Bool (*fun)(int,OZ_Term[]),
                                     OZ_Term *args,int arity)
{
  return new Suspension(new CFuncContinuation(am.currentBoard,
                                              am.currentThread->getPriority(),
                                              fun, args, arity));
}

void OZ_addSuspension(OZ_Term *var, Suspension *s)
{
  SVariable *svar = taggedBecomesSuspVar(var);

  if (am.setExtSuspension (svar->getHome (), s) == OK) {
    s->setExtSusp ();
  }

  svar->addSuspension(s);
}


OZ_Bool OZ_onToplevel()
{
  return am.isToplevel() == OK ? PROCEED : FAILED;
}


// mm2: obsolete
OZ_Bool onToplevel()
{
  return am.isToplevel() == OK ? PROCEED : FAILED;
}


/* convert a C string (char*) to an Oz string */
OZ_Term OZ_CStringToOzString(char *s)
{
  char *p=s;
  while (*p!='\0') {
    p++;
  }
  OZ_Term ret = AtomNil;
  while (p!=s) {
    ret = cons(OZ_intToTerm((unsigned char)*(--p)), ret);
  }
  return ret;
}
