/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------

exported:
  char *tagged2String(TaggedRef ref, int depth=10, int offset=0)
  void taggedPrint(TaggedRef ref, int depth=10, int offset=0)
  void taggedPrintLong(TaggedRef ref, int depth=10, int offset=0)

  ------------------------------------------------------------------------
*/

#include <ctype.h>

#include "builtins.hh"
#include "bignum.hh"
#include "cell.hh"
#include "codeArea.hh"
#include "fdomn.hh"
#include "genvar.hh"
#include "fdgenvar.hh"
#include "misc.hh"
#include "records.hh"
#include "taskstack.hh"
#include "term.hh"

#define PRINT(C) \
     void C::print(ostream &stream, int depth, int offset)

#define PRINTLONG(C) \
     void C::printLong(ostream &stream, int depth, int offset)


/* print tuple & records in one or more lines ??? */
// #define NEWLINE(off) stream << endl << indent((off));
#define NEWLINE(off) stream << " ";

//-----------------------------------------------------------------------------
//                         Miscellaneous stuff

// returns OK if associated suspension is alive
inline Bool isEffectiveSusp(SuspList* sl)
{
  Suspension* s = sl->getSusp();
  if (s->isDead() == OK)
    return NO;
  if (s->getNode()->isDead() == OK)
    return NO;
  return OK;
}

// returns OK if sl contains at least one alive suspension element
inline Bool isEffectiveList(SuspList* sl) {
  for (; sl != NULL; sl = sl->getNext())
    if (isEffectiveSusp(sl) == OK)
      return OK;
  return NO;
}


// ----------------------------------------------------------------
// PRINT
// ----------------------------------------------------------------

static void tagged2Stream(TaggedRef ref,ostream &stream=cout,
                          int depth = 10,int offset = 0) {

  if (ref == makeTaggedNULL()) {
    stream << "*** NULL TERM ***";
    return;
  }

  DEREF(ref,refPtr,tag)
  switch(tag) {
  case UVAR:
    stream << "<UV: _ @"
           << refPtr
           << ">" ;
    break;
  case SVAR:
    tagged2SVar(ref)->print(stream,depth,offset);
    break;
  case CVAR:
    tagged2CVar(ref)->print(stream, depth, offset);
    break;
  case STUPLE:
    tagged2STuple(ref)->print(stream,depth,offset);
    break;
  case SRECORD:
    tagged2SRecord(ref)->print(stream,depth,offset);
    break;
  case LTUPLE:
    tagged2LTuple(ref)->print(stream,depth,offset);
    break;
  case ATOM:
    tagged2Atom(ref)->print(stream,depth,offset);
    break;
  case FLOAT:
    tagged2Float(ref)->print(stream,depth,offset);
    break;
  case BIGINT:
    tagged2BigInt(ref)->print(stream,depth,offset);
    break;
  case SMALLINT:
    {
      int value = smallIntValue(ref);
      if (value < 0) {
        stream << "~"
               << -value;
      } else {
        stream << value;
      }
    }
    break;
  case CONST:
    tagged2Const(ref)->print(stream,depth,offset);
    break;
  default:
    if (isRef(ref)) {
      stream << "PRINT: REF detected";
    } else {
      stream << "PRINT: unknown tag in term: " << tag;
    }
    break;
  }
}


PRINT(SVariable)
{
  stream << "<SV: "
         << getPrintName()
         << " @"
         << this
         << (isEffectiveList(suspList) == OK ? " * >" : ">");
}

PRINT(GenCVariable){
  stream << indent(offset)
         << "<CV: "
         << getPrintName()
         << " @"
         << this;
  if (isEffectiveList(suspList) == OK)
    stream << " a";

  switch(type){
  case FDVariable:
    ((GenFDVariable*)this)->print(stream, depth, offset);
    break;
  default:
    error("Unexpected type generic variable at %s:%d.",
          __FILE__, __LINE__);
    break;
  }

  stream << ">";
} // PRINT(GenCVariable)

PRINT(GenFDVariable){
  if (isEffectiveList(fdSuspList[det]) == OK)
    stream << " d";
  if (isEffectiveList(fdSuspList[bounds]) == OK)
    stream << " b";
  if (isEffectiveList(fdSuspList[size]) == OK)
    stream << " s";
  if (isEffectiveList(fdSuspList[eqvar]) == OK)
    stream << " e";
  stream << ' ';
  finiteDomain.print(stream, 0);
} // PRINT(GenFDVariable)

PRINT(STuple)
{
  int i;

  tagged2Stream(getLabel(),stream,depth,offset);
  if ( depth <= 0 ) {
    stream << "(...)";
  } else {
    stream << "(";
    NEWLINE(offset+2);
    for (i = 0; i < getSize (); i++) {
      tagged2Stream(getArg(i),stream, depth-1,offset+2);
      NEWLINE(offset+2);
    }
    stream << ")";
  }
}

PRINT(SRecord)
{
  switch (getType()) {
  case R_ABSTRACTION:
  case R_OBJECT:
    ((Abstraction *) this)->print(stream,depth,offset);
    break;
  case R_CELL:
    ((Cell *) this)->print(stream,depth,offset);
    break;
  case R_BUILTIN:
    ((Builtin *) this)->print(stream,depth,offset);
    break;
  case R_CHUNK:
  case R_RECORD:
    tagged2Stream(getLabel(),stream,depth,offset);
    break;
  }

  if (depth <= 0) {
    stream << "(...)";
  } else {
    stream << "(";
    NEWLINE(offset+2);
    TaggedRef ar = getArityList();
    CHECK_DEREF(ar);
    while (isCons(ar)) {
      TaggedRef feat = head(ar);
      CHECK_DEREF(feat);
      tagged2Stream(feat,stream,depth,offset);
      ar = tail(ar);
      CHECK_DEREF(ar);
      stream << " : ";
      tagged2Stream(getFeature(feat),stream,depth-1,offset+2);
      NEWLINE(offset+2);
    }
    stream << ")";
  }
}

PRINT(LTuple)
{
  if ( depth <= 0 ) {
    stream << " ... ";
  } else {
    TaggedRef head = getHead();
    DEREF(head,_1,tag1);
    if (isLTuple(tag1) ) {
      stream << "(";
      tagged2Stream(head,stream, depth-1,offset);
      stream << ")"
             << NameOfCons;
    } else {
      tagged2Stream(getHead(),stream, depth-1,offset);
      stream << NameOfCons;
    }
    tagged2Stream(getTail(),stream, depth-1, offset);
  }
}

static Bool isWellFormed(char *s) {
  if (!islower(*s)) {
    return NO;
  }
  s++;
  while (*s) {
    if (!isalnum(*s) && *s != '_') {
      return NO;
    }
    s++;
  }
  return OK;
}

static void ppAtom(ostream &stream, char *s) {
  stream << "'";
  char c;
  while (c = *s) {
    switch (c) {
    case '\'':
      stream << "\\'";
      break;
    case '\n':
      stream << "\\n";
      break;
    case '\r':
      stream << "\\r";
      break;
    case '\\':
      stream << "\\\\";
      break;
    case '\t':
      stream << "\\t";
      break;
    default:
      stream << c;
      break;
    }
    s++;
  }
  stream << "'";
}

PRINT(Atom)
{
  char *s = getPrintName();
  if (isXName()) {
    stream << "*"
           << s
           << "-"
           << this
           << "*";
  } else {
#define WELLFORMED
#ifdef WELLFORMED
    if (isWellFormed(s)) {
      stream << s;
    } else {
      ppAtom(stream,s);
    }
#else
    stream << s;
#endif
  }
}

PRINT(Float)
{
  stream << tagged2Atom(floatToAtomTerm(value))->getPrintName();
}

PRINT(BigInt)
{
  char *s = string();
  stream << s;
//       << "B";

  delete [] s;
}

PRINT(Cell)
{
  stream << "<cell:"
         << getPrintName()
         <<" @id" << getId()
         << ">";
}

PRINT(Abstraction)
{
  stream << (getType() == R_OBJECT ? "<object:" : "<procedure:")
         << getPrintName() << "/" << getArity()
         << " @id" << getId()
         << ">";
}

PRINT(Builtin)
{
  stream << "<builtin:"
         << getPrintName() << "/" << getArity()
         << " @id" << getId()
         << ">";
}


PRINT(BuiltinTabEntry)
{
  stream << "<builtin "
         << getPrintName()
         << "/"
         << arity
         << " @"
         << this
         << ">";
}


PRINT(Arity)
{
  stream << "Arity: ";
  tagged2Stream(list,stream,depth,offset);
  stream << endl;

  for(int i=0; i< size ; i++) {
    stream << "Slot: "
           << i
           << " Entry: ";
    if (keytable[i] == NULL) {
      stream << "<empty>\n";
    } else {
      keytable[i]->print(stream,depth,offset);
      stream << " Value: " << indextable[i] << endl;
    }
  }
  stream << numberofentries
    << " entries, but only "
    << numberofcollisions
    << " collisions.\n";
}


PRINT(ArityTable)
{
  Arity *c;
  for (int i = 0 ; i < size ; i ++) {
    stream << "Position " << i << endl;
    c = table[i];
    while (c != NULL ) {
      c->print(stream,depth,offset);
      c = c->next;
    }
  }
}


// ---------------------------------------------------



PRINT(SuspList){
  if (isEffectiveList(this) == NO) {
    stream << indent(offset) << "- empty -" << endl;
    return;
  }

  for (SuspList* sl = this; sl != NULL; sl = sl->getNext()) {
    if (isEffectiveSusp(sl) == NO)
      continue;
    if (sl->isCondSusp() == OK)
      stream << indent(offset)
             << ((CondSuspList*)sl)->getCondNum() << " conds";
    else
      stream << indent(offset) << "true";

    stream << " -> ";
    if (sl->getSusp()->getCont())
      stream << "cont ";
    else if (sl->getSusp()->getCCont())
      stream << "ccont ";
    else
      stream << "node ";
    sl->getSusp()->getNode()->print(stream, 0);
    stream << endl;
  } // for
}

// ----------------------------------------------------------------
// PRINT LONG
// ----------------------------------------------------------------


static void tagged2StreamLong(TaggedRef ref,ostream &stream = cout,
                              int depth = 0,int offset = 0)
{
  if (ref == makeTaggedNULL()) {
    stream << "*** NULL TERM ***" << endl;
    return;
  }

  if (isRef(ref)) {
    stream << indent(offset)
           << "@"
           << (void *) tagged2Ref(ref)
           << ": "
           << (void *) *tagged2Ref(ref)
           << endl;
    tagged2StreamLong(*tagged2Ref(ref),stream,depth-1,offset+2);
    return;
  }

  switch(tagTypeOf(ref)) {
  case UVAR:
    {
      stream << indent(offset)
             << "UV @"
             << tagValueOf(ref)
             << endl
             << indent(offset)
             << "HomeNode: ";
      tagged2VarHome(ref)->getBoardDeref()->print(stream,0);
      stream << endl;
    }
    break;

  case SVAR:
    tagged2SVar(ref)->printLong(stream,depth,offset);
    break;
  case CVAR:
    tagged2CVar(ref)->printLong(stream, depth, offset);
    break;
  case STUPLE:
    tagged2STuple(ref)->printLong(stream,depth,offset);
    break;
  case SRECORD:
    tagged2SRecord(ref)->printLong(stream,depth,offset);
    break;
  case LTUPLE:
    tagged2LTuple(ref)->printLong(stream,depth,offset);
    break;
  case ATOM:
    tagged2Atom(ref)->printLong(stream,depth,offset);
    break;
  case FLOAT:
    tagged2Float(ref)->printLong(stream,depth,offset);
    break;
  case BIGINT:
    tagged2BigInt(ref)->printLong(stream,depth,offset);
    break;
  case SMALLINT:
    {
      stream << indent(offset)
             << "SmallInt @"
             << ref
             << ": ";
      tagged2Stream(ref,stream);
      stream << endl;
    }
    break;
  case CONST:
    tagged2Const(ref)->printLong(stream,depth,offset);
    break;
  default:
    if (isRef(ref)) {
      stream << "PRINT: REF detected" << endl;
    } else {
      stream << "PRINT: unknown tag in term: " << tagTypeOf(ref) << endl;
    }
    break;
  }
}

PRINTLONG(ConstTerm)
{
  error("mm2: not impl");
}

PRINT(ConstTerm)
{
  error("mm2: not impl");
}

PRINT(Board)
{
  error("mm2: not impl");
}

PRINTLONG(Atom)
{
  if (isXName()) {
    stream << indent(offset) << "Name @" << this << ": ";
  } else {
    stream << indent(offset) << "Atom @" << this << ": ";
  }
  this->print(stream,depth,offset);
  stream << endl;
}


PRINTLONG(SVariable)
{
  stream << indent(offset)
         << "SV "
         << getPrintName()
         << " @"
         << this
         << endl
         << indent(offset)
         << "SuspList:\n";
  suspList->print(stream, 0, offset+3);

  stream << indent(offset)
         << "HomeNode: ";
  clusterNode->getBoardDeref()->print(stream,0);
  stream << endl;
}

PRINTLONG(GenCVariable){
  stream << indent(offset)
         << "CV "
         << getPrintName()
         << " @"
         << this
         << endl
         << indent(offset)
         << "Any SuspList:\n";
  suspList->print(stream, 0, offset+3);

  if (type == FDVariable) {
    stream << indent(offset) << "Det SuspList:\n";
    ((GenFDVariable*)this)->fdSuspList[det]->print(stream, 0, offset+3);

    stream << indent(offset) << "Bounds SuspList:\n";
    ((GenFDVariable*)this)->fdSuspList[bounds]->print(stream, 0, offset+3);

    stream << indent(offset) << "Size SuspList:\n";
    ((GenFDVariable*)this)->fdSuspList[size]->print(stream, 0, offset+3);

    stream << indent(offset) << "Equal variables SuspList:\n";
    ((GenFDVariable*)this)->fdSuspList[eqvar]->print(stream, 0, offset+3);

   }

  stream << indent(offset) << "HomeNode: ";
  clusterNode->getBoardDeref()->print(stream,0);
  stream << endl;

  switch(type){
  case FDVariable:
    ((GenFDVariable*)this)->printLong(stream, depth, offset);
    break;
  default:
    error("Unexpected type generic variable at %s:%d.",
          __FILE__, __LINE__);
    break;
  }

} // PRINTLONG(GenCVariable)


PRINTLONG(GenFDVariable){
  finiteDomain.printLong(stream, offset);
} // PRINTLONG(GenFDVariable)


PRINTLONG(STuple)
{
  int i;

  stream << indent(offset) << "Tuple @" << this << endl
         << indent(offset) << "Label: ";
  tagged2StreamLong(label,stream,depth,offset);
  stream << endl;
  if ( depth <= 0 ) {
    stream << indent(offset) <<  "Args: ...";
  } else {
    for (i = 0; i < getSize (); i++) {
      stream << indent(offset) <<  "Arg "<< i << ":\n";
      tagged2StreamLong (getArg(i),stream,depth-1,offset);
      stream << " ";
    }
  }
  stream << endl;
}


PRINTLONG(LTuple)
{
  stream << indent(offset) << "List @" << this << endl;

  if ( depth <= 0 ) {
    stream << indent(offset) << "Args: ...\n";
  } else {
    stream << indent(offset) << "Head:\n";
    tagged2StreamLong(getHead(),stream,depth-1,offset+2);
    stream << indent(offset) << "Tail:\n";
    tagged2StreamLong(getTail(),stream,depth-1,offset+2);
  }
}

PRINTLONG(Abstraction)
{
  stream << indent(offset)
         << (getType() == R_OBJECT ? "Object " : "")
         << "Abstraction @id"
         << getId() << endl;
  pred->printLong(stream,depth,offset);
  int n = gRegs ? getRefsArraySize(gRegs) : 0;
  if (offset == 0) {
    if (n > 0) {
      stream <<  "G Regs:";
      for (int i = 0; i < n; i++) {
        stream << " G[" << i << "]:\n";
        tagged2StreamLong(gRegs[i],stream,depth,offset+2);
      }
    } else {
      stream << "No G-Regs" << endl;
    }
  } else {
    stream << indent(offset) << "G Regs: #" << n << endl;
  }
}

PRINTLONG(PrTabEntry)
{
  stream << indent(offset)
         <<  "Name: " << getPrintName()
         << "/" << arity
         << "/" << gRegs.getSize() << endl
         << indent(offset)
         <<  "ProgramCounter: " << (void *) PC << endl
         << indent(offset) <<  "Arity: " << arity << endl;
}

PRINTLONG(Builtin)
{
  print(stream,depth,offset);
  stream << endl;
}



PRINTLONG(Cell)
{
  stream << indent(offset)
         << "Cell " << getPrintName()
         << " @id"  << getId() << endl
         << indent(offset)
         << " value:"<<endl;
  tagged2StreamLong(val,stream,depth,offset+2);
}

PRINTLONG(SRecord)
{
  stream << indent(offset);
  switch (getType()) {
  case R_ABSTRACTION:
  case R_OBJECT:
    ((Abstraction *) this)->printLong(stream,depth,offset);
    break;
  case R_CELL:
    ((Cell *) this)->printLong(stream,depth,offset);
    break;
  case R_BUILTIN:
    ((Builtin *) this)->printLong(stream,depth,offset);
    break;
  case R_CHUNK:
    stream << "Chunk @" << this << ":\n";
    break;
  case R_RECORD:
    stream << "Record @"
           << this << ":\n"
           << indent(offset)
           << "Label: ";
    tagged2Stream(u.label,stream,depth-1,offset);
    stream << endl;
    break;
  }

  if (depth < 0) {
    stream << indent(offset) << "Args: ...\n";
  } else {
    stream << indent(offset) << "Args:\n";
    TaggedRef ar = getArityList();
    CHECK_DEREF(ar);
    while (isCons(ar)) {
      stream << indent(offset+2);
      TaggedRef feat = head(ar);
      CHECK_DEREF(feat);
      tagged2Stream(feat,stream,depth-1,offset+2);
      ar = tail(ar);
      CHECK_DEREF(ar);
      stream << ": ";
      tagged2StreamLong(getFeature(feat),stream,depth-1,offset+2);
    }
    stream << indent(offset) << "End of Args\n";
  }
}


PRINTLONG(Float)
{
  stream << indent(offset) << "Float @" << this << ": "
         << tagged2Atom(floatToAtomTerm(value))->getPrintName();
}

PRINTLONG(BigInt)
{
  char *s = string();

  stream << indent(offset)
         << "BigInt @" << this << ": "
         << s
         << endl;

  delete [] s;
}





PRINT(TaskStack)
{
  if (isEmpty() == OK) {
    stream << "*** empty ***";
  } else {
    for (void **i = top - 1; i != stack; i--) {
      stream << "@" << *i << ",";
    }
  }
}


#ifdef RECINSTRFETCH

#define InstrDumpFile "fetchedInstr.dump"

void CodeArea::writeInstr(void){
  FILE* ofile;
  if(ofile = fopen(InstrDumpFile, "w")){
    int i = fetchedInstr;
//    ofile=stdout;
    do {
      if (ops[i]) {
        display(ops[i], 1, ofile);
      }
      i++;
      if(i >= RECINSTRFETCH)
        i = 0;
    } while (i != fetchedInstr);
    fclose(ofile);
    fprintf(stderr,
            "Wrote the %d most recently fetched instructions in file '%s'\n",
            RECINSTRFETCH, InstrDumpFile);
  } else
    error("Cannot open file '%s'.", InstrDumpFile);
} // CodeArea::writeInstr
#endif



// ----------------------------------------------------

char *tagged2String(TaggedRef ref,int depth,int offset)
{
  ostrstream out;

  tagged2Stream(ref,out,depth,offset);
  out << ends;
  char *s = out.str();
  return s;
}

void taggedPrint(TaggedRef ref, int depth, int offset)
{
  tagged2Stream(ref,cout,depth,offset);
  cout << flush;
}

void taggedPrintLong(TaggedRef ref, int depth, int offset)
{
  tagged2StreamLong(ref,cout,depth,offset);
  cout << flush;
}
