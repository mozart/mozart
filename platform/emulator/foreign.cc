
/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */


#define OZ_Suspension Suspension
#include "foreign.h"


#include "builtins.hh"
#include "bignum.hh"
#include "records.hh"
#include "cell.hh"
#include "am.hh"

int OZ_isInteger(OZ_Term term)
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

int OZ_termToInt(OZ_Term term, int*n)
{
  DEREF(term,_1,tag);

  if ( isSmallInt(tag) ) {
    *n = smallIntValue(term);
    return(1);
  } else if (isBigInt(term) && (*n = tagged2BigInt(term)->BigInt2Int()) != 0) {
                                                                                   return(1);
  } else {
    return(0);
  }
}

int OZ_termToFloat(OZ_Term term, OZ_Float *n)
{
  DEREF(term,_1,tag);

  if ( isFloat(tag) ) {
    *n = floatValue(term);
    return(1);
  } else
    return(0);
}

int OZ_termToString(OZ_Term term, char **s)
{
  DEREF(term,_1,tag);
  if (isXAtom(term)) {
    *s = tagged2Atom(term)->getPrintName();
    return 1;
  }
  return 0;
}

char *OZ_termToStr(OZ_Term term)
{
  return tagged2String(term);
}


// from ss
char *OZ_intTermToString(OZ_Term term)
{
  DEREF(term,_1,tag);
  if (isInt(term))
    {
      char *s = tagged2String(term);

      if (s[0] == '~')
        {
          s[0] = '-';
        }
      return s;
    }
  else
    {
      return NULL;
    }
}


OZ_Term OZ_intToTerm (int i)
{
  return intToTerm(i);
}

OZ_Term OZ_floatToTerm (OZ_Float i)
{
  return floatToTerm(i);
}

OZ_Term OZ_stringToTerm(char *s)
{
  return makeTaggedAtom(s);
}

OZ_Term OZ_numberToTerm(char *s)
{
  return numberToTerm(s);
}

OZ_Bool OZ_unify(OZ_Term t1, OZ_Term t2)
{
  if (!am.unify(t1, t2)) {
    return OZ_FAILED;
  }
  return OZ_PROCEED;
}

OZ_Term OZ_newVariable(char *name)
{
  SVariable *cvar = new SVariable(Board::Current,
                                  OZ_stringToTerm(name));
  return makeTaggedRef(newTaggedSVar(cvar));
}

OZ_Term OZ_newVar()
{
  return makeTaggedRef(newTaggedUVar(Board::Current));
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
  if (am.setIORequest(fd) == NO) {
    return 0;
  }
  return 1;
}

int OZ_openIO(int fd) {
  am.openIO(fd);
  return 1;
}

int OZ_closeIO(int fd) {
  am.closeIO(fd);
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
  taggedPrint(t,am.printDepthVal);
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


OZ_Suspension *OZ_makeSuspension(OZ_Bool (*fun)(int,OZ_Term[]),
                                 OZ_Term *args,int arity)
{
  Board::Current->addSuspension();
  return new OZ_Suspension(new CFuncContinuation(Board::Current,
                                                 fun, args, arity));
}


void OZ_addSuspension(OZ_Term *var, OZ_Suspension *s)
{
  SVariable *svar = taggedBecomesSuspVar(var);

  svar->addSuspension(s);
}
