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

#ifdef __GNUC__
#pragma implementation "print.hh"
#endif

#include <ctype.h>
#include <strstream.h>

#include "actor.hh"
#include "am.hh"
#include "builtins.hh"
#include "bignum.hh"
#include "board.hh"
#include "cell.hh"
#include "fdgenvar.hh"
#include "fdomn.hh"
#include "misc.hh"
#include "records.hh"
#include "taskstack.hh"
#include "thread.hh"
#include "objects.hh"


#define PRINT(C) \
     void C::print(ostream &stream, int depth, int offset)

#define PRINTLONG(C) \
     void C::printLong(ostream &stream, int depth, int offset)


/* print tuple & records in one or more lines ??? */
// #define NEWLINE(off) stream << endl << indent((off));
#define NEWLINE(off) stream << " ";

//-----------------------------------------------------------------------------
//                         Miscellaneous stuff

// mm2
// returns OK if associated suspension is alive
inline Bool isEffectiveSusp(SuspList* sl)
{
  Suspension* s = sl->getSusp();
  if (s->isDead() == OK)
    return NO;
  if (!s->getNode()->getBoardDeref())
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
    stream << "UV@"
	   << refPtr;
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
  stream << "SV:"
	 << getPrintName()
	 << "@"
	 << this
	 << (isEffectiveList(suspList) == OK ? "*" : "");
}

PRINT(GenCVariable){
  stream << indent(offset)
	 << "<CV: "
	 << getPrintName()
	 << " @"
	 << this;
  if (isEffectiveList(suspList) == OK)
    stream << " a" << suspList->length();
  
  switch(type){
  case FDVariable:
    {
      GenFDVariable * me = (GenFDVariable *) this;
      if (isEffectiveList(me->fdSuspList[det]) == OK)
	stream << " d("
	       << me->fdSuspList[det]->length()
	       << '/'
	       << me->fdSuspList[det]->lengthProp()
	       << ')';
      if (isEffectiveList(me->fdSuspList[bounds]) == OK)
	stream << " b(" << me->fdSuspList[bounds]->length()
	       << '/'
	       << me->fdSuspList[bounds]->lengthProp()
	       << ')';
      if (isEffectiveList(me->fdSuspList[size]) == OK)
	stream << " s(" << me->fdSuspList[size]->length()
	       << '/'
	       << me->fdSuspList[size]->lengthProp()
	       << ')';
      if (isEffectiveList(me->fdSuspList[eqvar]) == OK)
	stream << " e(" << me->fdSuspList[eqvar]->length()
	       << '/'
	       << me->fdSuspList[eqvar]->lengthProp()
	       << ')';
      stream << ' ';
      me->getDom().print(stream, 0);
      break;
    }
  default:
    error("Unexpected type generic variable at %s:%d.",
	  __FILE__, __LINE__);
    break;
  }
  
  stream << ">";
} // PRINT(GenCVariable)


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
  TypeOfRecord type = getType();
  switch (type) {
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
  } else if (type != R_ABSTRACTION && type != R_OBJECT) {
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
      stream << ": ";
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
    TaggedRef headd = getHead();
    DEREF(headd,_1,tag1);
    if (isLTuple(tag1) ) {
      stream << "(";
      tagged2Stream(headd,stream, depth-1,offset);
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
    stream << "N:"
	   << s
	   << "-"
	   << seqNumber;
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
  char *s = stringTilde();
  stream << s;
//       << "B";

  delete [] s;
}

PRINT(Cell)
{
  stream << "C:"
	 << getPrintName()
	 <<"@" << (void*) getId();
}

PRINT(Abstraction)
{
  if (getType() == R_OBJECT) {
    stream << "O:" << ((Object*)this)->getPrintName();
    return;
  }
  stream << "P:"
	 << getPrintName() << "/" << getArity()
	 << "@" << (void*) getId();
}

PRINT(Builtin)
{
  stream << "B:"
	 << getPrintName() << "/" << getArity()
	 << "@" << (void*) getId();
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

    stream << " [";
    if (sl->getSusp()->isDead()) stream << 'D';
    if (sl->getSusp()->isPropagated()) stream << 'P';
    if (sl->getSusp()->isResistant()) stream << 'R';
    if (sl->getSusp()->isExtSusp()) stream << 'E';
    if (sl->getSusp()->isSurvSusp()) stream << 'S';
    if (sl->getSusp()->isEqvSusp()) stream << 'V';
    
    stream << "] -> ";
    if (sl->getSusp()->getCont())
      stream << "cont ";
    else if (sl->getSusp()->getCCont())
      stream  << "ccont = (*" << (void*) sl->getSusp()->getCCont()->getCFunc()
	      << ")(), ";
    else
      stream << "board ";
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
  switch (typeOf()) {
  case Co_Board:
    ((Board *) this)->printLong(stream, depth, offset);
  case Co_Thread:
    ((Thread *) this)->printLong(stream, depth, offset);
  case Co_Actor:
    ((Actor *) this)->printLong(stream, depth, offset);
  default:
    error("ConstTerm::printLong");
  }
}

PRINT(ConstTerm)
{
  switch (typeOf()) {
  case Co_Board:
    ((Board *) this)->print(stream, depth, offset);
    break;
  case Co_Thread:
    ((Thread *) this)->print(stream, depth, offset);
    break;
  case Co_Actor:
    ((Actor *) this)->print(stream, depth, offset);
    break;
  default:
    error("ConstTerm::print");
  }
}

void AM::print()
{
  cout << "class AM" << endl
       << "  currentBoard: ";
  currentBoard->print(cout,0,0);
  cout << endl
       << "  rootBoard:    ";
  rootBoard->print(cout,0,0);
  cout << endl
       << "  currentThread: ";
  currentThread->print(cout,0,0);
  cout << endl
       << "  rootThread:    ";
  rootThread->print(cout,0,0);
  cout << endl;
}

PRINT(Board)
{
  if (!this) {
    stream << indent(offset) << "(NULL Board)";
    return;
  }

  if (isRoot()) {
    stream << "Root";
  } else if (isWait()) {
    stream << "Wait";
  } else if (isAsk()) {
    stream << "Ask";
  } else if (isSolve ()) {
    stream << "Solve";
  }

  stream << "Board @" << this << " [";

  if (isCommitted()) {
    stream << 'C';
  }
  if (isReflected()) {
    stream << 'R';
  }
  if (isInstalled()) {
    stream << 'I';
  }
  if (isNervous()) {
    stream << 'N';
  }
  if (isWaitTop()) {
    stream << 'T';
  }
  if (isPathMark()) {
    stream << 'P';
  }
  if (isFailed()) {
    stream << 'F';
  }
  if (isDiscarded()) {
    stream << 'D';
  }
  if (isWaiting()) {
    stream << 'W';
  }
  stream << " #" << suspCount;
  stream << ']';
}

PRINTLONG(Board)
{
  print(stream,depth,offset);
  stream << endl;
}

void Board::Print()
{
  cout << "class Board" << endl
       << "  currentBoard: ";
  am.currentBoard->print(cout,0,0);
  cout << endl
       << "  rootBoard:    ";
  am.rootBoard->print(cout,0,0);
  cout << endl;
}

PRINT(Actor)
{
  if (!this) {
    stream << indent(offset) << "(NULL Actor)";
    return;
  }

  stream << indent(offset)
    << "Actor @"
    << this;
}

PRINTLONG(Actor)
{
  print(stream,depth,offset);
  stream << endl;
}

void Thread::Print()
{
  cout << "class Thread" << endl
       << "  currentThread: ";
  am.currentThread->print(cout,0,0);
  cout << endl
       << "  rootThread:    ";
  am.rootThread->print(cout,0,0);
  cout << endl
       << "  Queue:" << endl;
  for (Thread *th=Head; th; th = th->next) {
    th->print(cout,0,4);
    if (th == Head) {
      cout << " HEAD";
    }
    if (th == Tail) {
      cout << " TAIL";
    }
    if (th == am.rootThread) {
      cout << " ROOT";
    }
    if (th == am.currentThread) {
      cout << " CURRENT";
    }
    cout << endl;
  }
}

PRINT(Thread)
{
  if (!this) {
    stream << indent(offset) << "(NULL Thread)";
    return;
  }

  stream << indent(offset)
    << "Thread @" << this
    << " [ prio: " << priority
    << ", ";
  if (isNormal()) {
    if (u.taskStack) {
      stream << " Normal (#"
	     << u.taskStack->getUsed()-1
	     << ")";
    } else {
      stream << " Normal (uninit)";
    }
  }
  if (isSuspCont()) {
    stream << " SuspCont";
  }
  if (isSuspCCont()) {
    stream << " SuspCCont";
  }
  if (isNervous()) {
    stream << " Nervous";
  }
  stream << " ]";
}

PRINTLONG(Thread)
{
  this->print(stream,depth,offset);
  stream << endl;
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
  home->getBoardDeref()->print(stream,0);
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
  home->getBoardDeref()->print(stream,0);
  stream << endl;

  switch(type){
  case FDVariable:
    ((GenFDVariable*)this)->getDom().printLong(stream, offset);
    break;
  default:
    error("Unexpected type generic variable at %s:%d.",
	  __FILE__, __LINE__);
    break;
  }

} // PRINTLONG(GenCVariable)


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
  char *s = stringTilde();

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
    for (void **i = tos - 1; i != array; i--) {
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

void Board::printTree()
{
  Board *bb = this;
  Actor *aa;
  int off=0;
  while (bb!=am.rootBoard) {
    bb->print(cout,0,off);
    cout << endl;
    if (bb->isCommitted()) {
      bb=bb->u.board;
    } else {
      off++;
      aa = bb->u.actor;
      aa->print(cout,0,off);
      cout << endl;
      off++;
      bb = aa->getBoard();
    }
  }
}
