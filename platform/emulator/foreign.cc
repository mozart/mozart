/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */


#include "oz.h"


#include "am.hh"
#include "bignum.hh"
#include "builtins.hh"
#include "cell.hh"
#include "io.hh"
#include "records.hh"
#include "thread.hh"
#include "suspension.hh"

/* ------------------------------------------------------------------------ *
 * tests
 * ------------------------------------------------------------------------ */

int OZ_isAtom(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isXAtom(term) == OK) ? 1 : 0;
}

int OZ_isCell(OZ_Term term)
{
  DEREF(term,_1,_2);
  return (isCell(term) == OK) ? 1 : 0;
}

int OZ_isCons(OZ_Term term)
{
  return (isCons(term) == OK) ? 1 : 0;
}

int OZ_isFloat(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isFloat(tag) == OK) ? 1 : 0;
}

int OZ_isInt(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isInt(tag) == OK) ? 1 : 0;
}

int OZ_isLiteral(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isLiteral(tag) == OK) ? 1 : 0;
}

int OZ_isName(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isXName(term) == OK) ? 1 : 0;
}

int OZ_isNil(OZ_Term term)
{
  return (isNil(term) == OK) ? 1 : 0;
}

int OZ_isNoNumber(OZ_Term term)
{
  DEREF(term,_1,tag);
  return isRecord(tag) || isTuple(tag);
}

int OZ_isRecord(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isRecord(tag) == OK) ? 1 : 0;
}

int OZ_isTuple(OZ_Term term)
{
  DEREF(term,_1,tag);
  return (isTuple(tag) == OK) ? 1 : 0;
}

int OZ_isValue(OZ_Term term)
{
  DEREF(term,_1,tag);
  return !isAnyVar(tag);
}

int OZ_isVariable(OZ_Term term)
{
  DEREF(term,_1,tag);
  return isAnyVar(tag);
}

OZ_Term OZ_termType(OZ_Term term)
{
  DEREF(term,_,tag);

  if (isAnyVar(tag)) {
    return AtomVariable;
  }

  if (isInt(tag)) {
    return AtomInt;
  }

  if (isFloat(tag)) {
    return AtomFloat;
  }

  if (isLiteral(tag)) {
    return (tagged2Atom(term)->isXName() ? AtomName : AtomAtom);
  }

  if (isSTuple(tag) || isLTuple(tag)) {
    return AtomTuple;
  }

  if (isProcedure(term)) { // before isSRecord test !!
    return AtomProcedure;
  }

  if (isCell(term)) { // before isSRecord test !!
    return AtomCell;
  }

  if (isSRecord(tag)) {
    return AtomRecord;
  }

  return AtomUnknown;
}

/* -----------------------------------------------------------------
 * convert: C from/to Oz datastructure
 * -----------------------------------------------------------------*/

char *OZ_atomToC(OZ_Term term)
{
  DEREF(term,_1,tag);
  if (isXAtom(term)) {
//    return ozstrdup(tagged2Atom(term)->getPrintName());
    return tagged2Atom(term)->getPrintName();
  }
  OZ_warning("atomToC(%s): atom arg expected",tagged2String(term));
  return NULL;
}

float OZ_floatToC(OZ_Term term)
{
  DEREF(term,_1,tag);

  if (isFloat(tag)) {
    return floatValue(term);
  }
  OZ_warning("floatToC(%s): float arg expected",tagged2String(term));
  return 0.0;
}


int OZ_intToC(OZ_Term term)
{
  DEREF(term,_1,tag);

  if (isSmallInt(tag)) {
    return smallIntValue(term);
  } else if (isBigInt(term)) {
    return tagged2BigInt(term)->BigInt2Int();
  }
  OZ_warning("intToC(%s): int arg expected",tagged2String(term));
  return 0;
}

char *OZ_toC(OZ_Term term)
{
  if (term == makeTaggedNULL()) {
    return "*** NULL TERM ***";
  }

  DEREF(term,termPtr,tag)
  switch(tag) {
  case UVAR:
    return tagged2String(term);
//    stream << "UV@" << termPtr;
    break;
  case SVAR:
    return tagged2String(term);
//    tagged2SVar(term)->print(stream,depth,offset);
    break;
  case CVAR:
    return tagged2String(term);
//    tagged2CVar(term)->print(stream, depth, offset);
    break;
  case STUPLE:
    return tagged2String(term);
//    tagged2STuple(term)->print(stream,depth,offset);
    break;
  case SRECORD:
    return tagged2String(term);
//    tagged2SRecord(term)->print(stream,depth,offset);
    break;
  case LTUPLE:
    return tagged2String(term);
//    tagged2LTuple(term)->print(stream,depth,offset);
    break;
  case ATOM:
    {
      Atom *a = tagged2Atom(term);
      char *s = a->getPrintName();
      if (a->isXName()) {
        sprintf(TmpBuffer,"*%s-0x%x*",s,a);
        return ozstrdup(TmpBuffer);
      } else {
//      return ozstrdup(s);
        return s;
      }
    }
  case FLOAT:
    {
      float f = floatValue(term);
      sprintf(TmpBuffer,"%e",f);
      return ozstrdup(TmpBuffer);
    }
  case BIGINT:
    {
      BigInt *b = tagged2BigInt(term);
      return b->stringMinus();
    }
  case SMALLINT:
    {
      int value = smallIntValue(term);
      sprintf(TmpBuffer,"%d",value);
      return ozstrdup(TmpBuffer);
    }
    break;
  case CONST:
    return tagged2String(term);
//    tagged2Const(term)->print(stream,depth,offset);
    break;

  default:
    Assert(0);
    break;
  }

  warning("OZ_toC: failed");
  return ozstrdup("unknown term");
}


OZ_Term OZ_CToFloat (float i)
{
  return floatToTerm(i);
}

OZ_Term OZ_CToInt(int i)
{
  return intToTerm(i);
}

OZ_Term OZ_CToAtom(char *s)
{
  return makeTaggedAtom(s);
}

/* NOTE: sign can be '-' or '~' */
OZ_Term OZ_CToNumber(char *s)
{
  return numberToTerm(s);
}

/* -----------------------------------------------------------------
 * virtual strings
 * -----------------------------------------------------------------*/

/* convert a C string (char*) to an Oz string */
OZ_Term OZ_CToString(char *s)
{
  char *p=s;
  while (*p!='\0') {
    p++;
  }
  OZ_Term ret = OZ_nil();
  while (p!=s) {
    ret = OZ_cons(OZ_CToInt((unsigned char)*(--p)), ret);
  }
  return ret;
}

/* -----------------------------------------------------------------
 * tuple
 * -----------------------------------------------------------------*/

OZ_Term OZ_label(OZ_Term term)
{
  DEREF(term,_,tag);

  switch (tag) {
  case LTUPLE:
    return tagged2LTuple(term)->getLabel();
    break;
  case STUPLE:
    return tagged2STuple(term)->getLabel();
  case ATOM:
    return term;
  case SRECORD:
    return tagged2SRecord(term)->getLabel();
  default:
    break;
  }
  OZ_warning("OZ_label(%s): no number expected",OZ_toC(term));
  return nil();
}

int OZ_width(OZ_Term term)
{
  DEREF(term,_,tag);

  switch (tag) {
  case LTUPLE:
    return 2;
  case STUPLE:
    return tagged2STuple(term)->getSize ();
  case SRECORD:
    return tagged2SRecord(term)->getWidth();
  case ATOM:
    return 0;
  default:
    break;
  }
  OZ_warning("OZ_width(%s): no number expected",OZ_toC(term));
  return 0;
}

OZ_Term OZ_tuple(OZ_Term label, int width)
{
  if (!isLiteral(label)) {
    OZ_warning("OZ_tuple(%s,%d): literal expected",
               OZ_toC(label),width);
    return nil();
  }
  if ((width == 2) && (OZ_unify(label,AtomCons) == PROCEED)) {
    // have to make a list
    return makeTaggedLTuple(new LTuple());
  } else if (width <= 0) {
    return label;
  }
  return makeTaggedSTuple(STuple::newSTuple(label,width));
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
      OZ_warning("OZ_putArg(%s,%d,%s): bad arg",
                 OZ_toC(term),pos,OZ_toC(term));
      return 0;
    }
  }
  if (isSTuple(term) && (pos >= 1) && pos <= tagged2STuple(term)->getSize ()) {
    tagged2STuple(term)->setArg(pos-1,newTerm);
    return 1;
  }

  OZ_warning("OZ_putArg(%s,%d,%s): bad arg",
             OZ_toC(term),pos,OZ_toC(term));

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
      OZ_warning("OZ_getArg(%s,%d): bad arg", OZ_toC(term),pos);
      return nil();
    }
  }
  if (isSTuple(term) && (pos >= 1) && pos <= tagged2STuple(term)->getSize ())
    return tagged2STuple(term)->getArg(pos-1);

  OZ_warning("OZ_getArg(%s,%d): bad arg",OZ_toC(term),pos);
  return nil();
}

OZ_Term OZ_nil()
{
  return nil();
}

OZ_Term OZ_cons(OZ_Term hd,OZ_Term tl)
{
  return cons(hd,tl);
}

OZ_Term OZ_head(OZ_Term list)
{
  return head(list);
}

OZ_Term OZ_tail(OZ_Term list)
{
  return tail(list);
}

/* -----------------------------------------------------------------
 * record
 * -----------------------------------------------------------------*/

OZ_Term OZ_getRecordArg(OZ_Term term, OZ_Term fea)
{
  DEREF(term,_1,tag);
  if (isSRecord(tag)) {
    OZ_Term ret = tagged2SRecord(term)->getFeature(fea);
    if (ret) {
      return ret;
    }
  }
  OZ_warning("OZ_getArg(%s,%%): bad arg",OZ_toC(term),OZ_toC(fea));
  return 0;
}

/* -----------------------------------------------------------------
 * unification
 * -----------------------------------------------------------------*/

OZ_Bool OZ_unify(OZ_Term t1, OZ_Term t2)
{
  if (!am.unify(t1, t2)) {
    return FAILED;
  }
  return PROCEED;
}

OZ_Term OZ_newVariable()
{
  return makeTaggedRef(newTaggedUVar(am.currentBoard));
}

/* -----------------------------------------------------------------
 * IO
 * -----------------------------------------------------------------*/

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

/* -----------------------------------------------------------------
 * garbage collection
 * -----------------------------------------------------------------*/

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

/* -----------------------------------------------------------------
 * free strings
 * -----------------------------------------------------------------*/

void OZ_free(char *s) {
  delete [] s;
}

/* -----------------------------------------------------------------
 *
 * -----------------------------------------------------------------*/

int OZ_addBuiltin(char *name, int arity, OZ_CFun fun)
{
  return BIadd(name,arity,fun,OK) == NULL ? 0 : 1;
}

/* -----------------------------------------------------------------
 * Suspending builtins
 * -----------------------------------------------------------------*/

OZ_Suspension OZ_makeSuspension(OZ_Bool (*fun)(int,OZ_Term[]),
                                 OZ_Term *args,int arity)
{
  am.currentBoard->incSuspCount();
  return (OZ_Suspension)
    new Suspension(new CFuncContinuation(am.currentBoard,
                                         am.currentThread->getPriority(),
                                         fun, args, arity));
}

void OZ_addSuspension(OZ_Term var, OZ_Suspension susp)
{
  DEREF(var, varPtr, varTag);
  if (!isAnyVar(varTag)) {
    OZ_warning("OZ_addSuspension(%s): var arg expected",
               OZ_toC(var));
    return;
  }

  SVariable *svar = taggedBecomesSuspVar(varPtr);
  Suspension *s = (Suspension *) susp;

  svar->addSuspension(s);
}

/* -----------------------------------------------------------------
 *
 * -----------------------------------------------------------------*/

int OZ_onToplevel()
{
  return am.isToplevel() ? 1 : 0;
}
