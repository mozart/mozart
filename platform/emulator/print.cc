/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl

  ------------------------------------------------------------------------
*/

#include "ozstrstream.h"

#include "am.hh"

#include "genvar.hh"
#include "fdomn.hh"
#include "dictionary.hh"

class Indent {
public:
  int len;
  Indent(int n) : len(n) {};
};

inline ostream& operator<<(ostream& o, Indent m) 
{
  int i = m.len;
  while(i--) {
    o << ' ';
  }
  return o;
}

inline Indent indent(int i) {
  return Indent(i);
}


void printWhere(ostream &cout,ProgramCounter PC);


#define PRINT(C) \
     void C::print(ostream &stream, int depth, int offset)

#define PRINTLONG(C) \
     void C::printLong(ostream &stream, int depth, int offset)


/* print tuple & records in one or more lines ??? */
// #define NEWLINE(off) stream << endl << indent((off));
#define NEWLINE(off) stream << " ";


#define DEC(depth) ((depth)==-1?(depth):(depth)-1)
#define CHECKDEPTH							      \
{									      \
  if (depth == 0) {							      \
    stream << ",,,";							      \
    return;								      \
  }									      \
}

#define CHECKDEPTHLONG							      \
{									      \
  if (depth == 0) {							      \
    stream << indent(offset) << ",,," << endl;				      \
    return;								      \
  }									      \
}

//-----------------------------------------------------------------------------
//                         Miscellaneous stuff

// mm2
// returns OK if associated suspension is alive
inline Bool isEffectiveSusp(SuspList* sl)
{
  return OK;
  Thread *thr = sl->getElem ();
  if (thr->isDeadThread ())
    return NO;
  if (!(thr->getBoard()))
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

void tagged2Stream(TaggedRef ref, ostream &stream, int depth, int offset) 
{
  CHECKDEPTH;
  if (ref == makeTaggedNULL()) {
    stream << "*** NULL TERM ***";
    return;
  }

  TaggedRef origRef = ref;
  DEREF(ref,refPtr,tag)    
  switch(tag) {
  case UVAR:
    stream << "_"
	   << hex << ToInt32(refPtr) << dec;
    break;
  case SVAR:
    tagged2SVar(ref)->print(stream,depth,offset,origRef);
    break;
  case CVAR:
    tagged2CVar(ref)->print(stream, depth, offset,origRef);
    break;
  case SRECORD:
    tagged2SRecord(ref)->print(stream,depth,offset);
    break;
  case LTUPLE:
    tagged2LTuple(ref)->print(stream,depth,offset);
    break;
  case LITERAL:
    tagged2Literal(ref)->print(stream,depth,offset);
    break;
  case OZFLOAT:
    tagged2Float(ref)->print(stream,depth,offset);
    break;
  case BIGINT:
  case SMALLINT:
    {
      stream << toC(ref);
    }
    break;
  case OZCONST:
    tagged2Const(ref)->print(stream,depth,offset);
    break;
  case FSETVALUE:
    ((FSetValue *) tagged2FSetValue(ref))->print(stream,depth,offset);
    break;
  default:
    if (isRef(ref)) {
      stream << "PRINT: REF detected";
    } else {
      stream << "PRINT: unknown tag in term: " << (int) tag;
    }
    break;
  }
}

void printTerm(TaggedRef t, ostream &stream, int depth = 10, int offset= 0){
  CHECKDEPTH;
  tagged2Stream(t, stream, depth, offset);
}
    

void SVariable::print(ostream &stream, int depth, int offset, TaggedRef v)
{
  CHECKDEPTH;
  stream << "SV:"
	 << getVarName(v)
	 << "@"
	 << this
	 << (isEffectiveList(suspList) == OK ? "*" : "");
}

void GenCVariable::print(ostream &stream, int depth, int offset, TaggedRef v)
{
  CHECKDEPTH;
  switch(getType()){
  case FDVariable:
    {
      stream << indent(offset)
	     << "<CV: "
	     << getVarName(v)
	     << " @"
	     << this;
      if (isEffectiveList(suspList) == OK)
        stream << " a" << suspList->length();
  
      GenFDVariable * me = (GenFDVariable *) this;
      if (isEffectiveList(me->fdSuspList[fd_prop_singl]) == OK)
	stream << " s("
	       << me->fdSuspList[fd_prop_singl]->length()
	       << '/'
	       << me->fdSuspList[fd_prop_singl]->lengthProp()
	       << ')';
      if (isEffectiveList(me->fdSuspList[fd_prop_bounds]) == OK)
	stream << " b(" << me->fdSuspList[fd_prop_bounds]->length()
	       << '/'
	       << me->fdSuspList[fd_prop_bounds]->lengthProp()
	       << ')';
      stream << ' ' <<  me->getDom() << ">";
      break;
    }

  case BoolVariable:
    {
      stream << indent(offset)
	     << "<CV: "
	     << getVarName(v)
	     << " @"
	     << this;
      if (isEffectiveList(suspList))
        stream << " a" << suspList->length();
  
      stream << " {0 1}>";
      break;
    }

  case FSetVariable:
    {
      stream << indent(offset)
	     << "<CV: "
	     << getVarName(v)
	     << " @"
	     << this;
      if (isEffectiveList(suspList) == OK)
        stream << " a" << suspList->length();
  
      GenFSetVariable * me = (GenFSetVariable *) this;
      if (isEffectiveList(me->fsSuspList[fs_prop_val]) == OK)
	stream << " val("
	       << me->fsSuspList[fs_prop_val]->length()
	       << '/'
	       << me->fsSuspList[fs_prop_val]->lengthProp()
	       << ')';
      if (isEffectiveList(me->fsSuspList[fs_prop_glb]) == OK)
	stream << " glb(" << me->fsSuspList[fs_prop_glb]->length()
	       << '/'
	       << me->fsSuspList[fs_prop_glb]->lengthProp()
	       << ')';
      if (isEffectiveList(me->fsSuspList[fs_prop_lub]) == OK)
	stream << " lub(" << me->fsSuspList[fs_prop_lub]->length()
	       << '/'
	       << me->fsSuspList[fs_prop_lub]->lengthProp()
	       << ')';
      stream << ' ' <<  me->getSet() << ">";
      break;
    }

  case OFSVariable:
    {
      stream << indent(offset)
	     << "<CV: "
	     << getVarName(v)
	     << " @"
	     << this;
      if (isEffectiveList(suspList))
        stream << " a" << suspList->length();

      stream << ' ';
      GenOFSVariable* me = (GenOFSVariable *) this;
      tagged2Stream(me->getLabel(),stream,DEC(depth),offset);
      // me->getLabel()->print(stream,DEC(depth), offset);
      me->getTable()->print(stream,DEC(depth), offset+2);
      break;
   }

  case MetaVariable:
    {
      GenMetaVariable* me = (GenMetaVariable *) this;
      stream << indent(offset) << "<MV." << me->getName() << ": "
	     << getVarName(v) << " @" << this;

      if (isEffectiveList(suspList))
	stream << " a" << suspList->length();
      
      stream << ' ' << me->toString(DEC(depth)) << '>';
      break;
    }
  case AVAR:
    stream << indent(offset) << "<AVAR "
	     << getVarName(v) << " @" << this << ">";
      break;
  case PerdioVariable:
    stream << indent(offset) << "<PerdioVariable "
	     << getVarName(v) << " @" << this << ">";
      break;

  default:
    error("Unexpected type of generic variable at %s:%d.",
	  __FILE__, __LINE__);
    break;
  }
} // PRINT(GenCVariable)


// Non-Name Features are output in alphanumeric order (ints before atoms):
PRINT(DynamicTable)
{
    CHECKDEPTH;
    stream << '(';
    int nonempty=FALSE;
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
	    nonempty=TRUE;
            CHECK_DEREF(tmplit);
	    if (isAtom(tmplit)||isInt(tmplit)) nAtomOrInt++; else nName++;
	}
    }
    // Allocate array on heap, put Atoms in array:
    //STuple *stuple=STuple::newSTuple(AtomNil,nAtomOrInt);
    //TaggedRef *arr=stuple->getRef();
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
        stream << ' ';
        tagged2Stream(arr[ai],stream,depth);
        stream << ':';
        stream << ' ';
        tagged2Stream(lookup(arr[ai]),stream,depth);
    }
    // Output the Names last, unordered:
    for (di=0; di<size; di++) {
	tmplit=table[di].ident;
        tmpval=table[di].value;
	if (tmpval!=makeTaggedNULL() && !(isAtom(tmplit)||isInt(tmplit))) {
            stream << ' ';
            tagged2Stream(tmplit,stream,depth);
            stream << ':';
            stream << ' ';
            tagged2Stream(tmpval,stream,depth);
        }
    }
    // Deallocate array:
    delete arr;
    // Finish up the output:
    if (nonempty) stream << ' ';
    stream << "...)" ;
}


PRINTLONG(DynamicTable)
{
  print(stream, depth, offset);
}

PRINT(SRecord)
{
  CHECKDEPTH;
  tagged2Stream(getLabel(),stream,depth,offset);

  TaggedRef ar = getArityList();
  CHECK_DEREF(ar);
  if (isCons(ar)) {
    stream << "(";
    int i=1;
    Bool isTuple=OK;
    while (isCons(ar)) {
      NEWLINE(offset+2);
      TaggedRef feat = head(ar);
      CHECK_DEREF(feat);
      if (isTuple && isSmallInt(feat) && smallIntValue(feat)==i) {
	i++;
      } else {
	isTuple = NO;
	tagged2Stream(feat,stream,depth,offset);
	stream << ": ";
      }
      ar = tail(ar);
      CHECK_DEREF(ar);
      tagged2Stream(getFeature(feat),stream,DEC(depth),offset+2);
    }
    NEWLINE(offset);
    stream << ")";
  }
}

PRINT(LTuple)
{
  CHECKDEPTH;
  TaggedRef headd = getHead();
  DEREF(headd,_1,tag1);
  if (isLTuple(tag1) ) {
    stream << "(";
    tagged2Stream(headd,stream, DEC(depth),offset);
    stream << ")"
	   << NameOfCons;
  } else {
    tagged2Stream(getHead(),stream, DEC(depth),offset);
    stream << NameOfCons;
  }
  tagged2Stream(getTail(),stream, DEC(depth), offset);
}


PRINT(Literal)
{
  CHECKDEPTH;
  stream << toC(makeTaggedLiteral(this));
}

PRINT(Float)
{
  CHECKDEPTH;
  stream << toC(makeTaggedFloat(this));
}

PRINT(Cell)
{
  CHECKDEPTH;
  stream << "C@" << this;
}

PRINT(PortLocal)
{
  CHECKDEPTH;
  stream << "PortLocal@" << this;
}

PRINT(PortProxy)
{
  CHECKDEPTH;
  stream << "PortProxy@" << this;
}

PRINT(PortManager)
{
  CHECKDEPTH;
  stream << "PortManager@" << this;
}

PRINT(Space)
{
  CHECKDEPTH;
  stream << "Space@" << this;
}

PRINT(OzArray)
{
  CHECKDEPTH;
  depth--;
  stream << "Array@" << this << "[ ";
  for (int i=getLow(); i<=getHigh(); i++) {
    stream << i << ": ";
    TaggedRef t=getArg(i);
    tagged2Stream(t,stream, depth,offset);
    stream << " ";
  }
  stream << "]";
}

PRINTLONG(OzArray)
{
  CHECKDEPTHLONG;
  print(stream,depth+1,offset);
}

PRINT(FSetValue)
{
  CHECKDEPTH;
  print2stream(stream);
}

PRINTLONG(FSetValue)
{
  CHECKDEPTHLONG;
  print(stream,depth+1,offset);
}

PRINT(OzDictionary)
{
  CHECKDEPTH;
  stream << "<Dictionary@" << this << ">";
}

PRINTLONG(OzDictionary)
{
  CHECKDEPTHLONG;
  print(stream,depth+1,offset);
}

PRINT(OzLock)
{
  CHECKDEPTH;
  stream << "<Lock@" << this << ">";
}

PRINTLONG(OzLock)
{
  CHECKDEPTHLONG;
  stream << "<Lock@" << this << ">";
}

PRINT(SChunk)
{
  CHECKDEPTH;
  stream << "Chunk@" << this;
}


PRINT(Abstraction)
{
  CHECKDEPTH;
  stream << "P:"
	 << getPrintName() << "/" << getArity();
//	 << "@" << this;
}

PRINT(Object)
{
  CHECKDEPTH;
  stream << "<O:" << getPrintName()
         << ", ";
  if (getFreeRecord())
    getFreeRecord()->print(stream,depth+1,offset);
  else
    stream << "nofreefeatures";
  stream << ", State: ";
  if (getState())
    getState()->print(stream,depth+1,offset);
  stream << ">";
}

PRINT(BuiltinTabEntry)
{
  CHECKDEPTH;
  stream << "B:"
	 << getPrintName() << "/" << getArity();
}


PRINT(Arity)
{
  CHECKDEPTH;
  stream << "Arity: ";
  tagged2Stream(list,stream,depth,offset);
  stream << endl;

  for(int i=0; i< size ; i++) {
    stream << "Slot: "
	   << i
	   << " Entry: ";
    if (keytable[i] == makeTaggedNULL()) {
      stream << "<empty>\n";
    } else { 
      tagged2Stream(keytable[i],stream,depth,offset);
      stream << " Value: " << indextable[i] << endl;
    }
  }
#ifdef DEBUG_CHECK
  stream << numberofentries
    << " entries, but only "
    << numberofcollisions
    << " collisions.\n";
#endif
}


PRINT(ArityTable)
{
  CHECKDEPTH;
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

PRINT(SuspList)
{
  CHECKDEPTH;
  if (isEffectiveList(this) == NO) {
    stream << indent(offset) << "- empty -" << endl;
    return;
  }

  for (SuspList* sl = this; sl != NULL; sl = sl->getNext()) {
    if (isEffectiveSusp(sl) ) {
      stream << indent(offset);
      (sl->getElem ())->print (stream);
      stream << endl;
    }
  } // for
}

// ----------------------------------------------------------------
// PRINT LONG
// ----------------------------------------------------------------

static void tagged2StreamLong(TaggedRef ref,ostream &stream = cout,
			      int depth = 1,int offset = 0)
{
  CHECKDEPTHLONG;
  if (ref == makeTaggedNULL()) {
    stream << indent(offset) << "*** NULL TERM ***" << endl;
    return;
  }

  if (isRef(ref)) {
    stream << indent(offset)
	   << '@'
	   << (void *) tagged2Ref(ref)
	   << ": "
	   << (void *) *tagged2Ref(ref)
	   << endl;
    tagged2StreamLong(*tagged2Ref(ref),stream,DEC(depth),offset+2);
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
      tagged2VarHome(ref)->derefBoard()->print(stream,DEC(depth));
      stream << endl;
    }
    break;

  case SVAR:
    tagged2SVar(ref)->printLong(stream,depth,offset,AtomVoid);
    break;
  case CVAR:
    tagged2CVar(ref)->printLong(stream, depth, offset,AtomVoid);
    break;
  case SRECORD:
    tagged2SRecord(ref)->printLong(stream,depth,offset);
    break;
  case LTUPLE:
    tagged2LTuple(ref)->printLong(stream,depth,offset);
    break;
  case LITERAL:
    tagged2Literal(ref)->printLong(stream,depth,offset);
    break;
  case OZFLOAT:
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
      tagged2Stream(ref,stream,depth,offset);
      stream << endl;
    }
    break;
  case OZCONST:
    tagged2Const(ref)->printLong(stream,depth,offset);
    break;
  case FSETVALUE:
    ((FSetValue *) tagged2FSetValue(ref))->printLong(stream,depth,offset);
    break;
  default:
    if (isRef(ref)) {
      stream << "PRINT: REF detected" << endl;
    } else {
      stream << "PRINT: unknown tag in term: " << (int) tagTypeOf(ref) << endl;
    }
    break;
  }
}

PRINTLONG(ConstTerm)
{
  CHECKDEPTHLONG;
  switch (getType()) {
  case Co_Board:      ((Board *) this)->printLong(stream, depth, offset);     break;
  case Co_Actor:      ((Actor *) this)->printLong(stream, depth, offset);     break;
  case Co_HeapChunk:  ((HeapChunk *) this)->printLong(stream, depth, offset); break;
  case Co_Abstraction:((Abstraction *) this)->printLong(stream,depth,offset); break;
  case Co_Object:     ((Object *) this)->printLong(stream,depth,offset);      break;
  case Co_Cell:	      ((Cell *) this)->printLong(stream,depth,offset);        break;
  case Co_Port:	      
    switch(((Tertiary *)this)->getTertType()){
    case Te_Local:   ((PortLocal *)   this)->printLong(stream,depth,offset); break;
    case Te_Manager: ((PortManager *) this)->printLong(stream,depth,offset); break;
    case Te_Proxy:   ((PortProxy *)   this)->printLong(stream,depth,offset); break;
    default:         Assert(NO);
    }
    break;
  case Co_Space:      ((Space *) this)->printLong(stream,depth,offset);       break;
  case Co_Chunk:      ((SChunk *) this)->printLong(stream,depth,offset);      break;
  case Co_Array:      ((OzArray *) this)->printLong(stream,depth,offset);     break;
  case Co_Dictionary: ((OzDictionary *) this)->printLong(stream,depth,offset);break;
  case Co_Lock:       ((OzLock *) this)->printLong(stream,depth,offset);break;
  case Co_Thread:     ((Thread *) this)->printLong(stream,depth,offset);    break;
  case Co_Builtin:    ((BuiltinTabEntry *) this)->printLong(stream,depth,offset);     break;
  default: 	      Assert(NO);
  }
}

PRINT(ConstTerm)
{
  CHECKDEPTH;
  switch (getType()) {
  case Co_Board:       ((Board *) this)->print(stream, depth, offset);     break;
  case Co_Actor:       ((Actor *) this)->print(stream, depth, offset);     break;
  case Co_HeapChunk:   ((HeapChunk *) this)->print(stream, depth, offset); break;
  case Co_Abstraction: ((Abstraction *) this)->print(stream,depth,offset); break;
  case Co_Object:      ((Object *) this)->print(stream,depth,offset);      break;
  case Co_Cell:        ((Cell *) this)->print(stream,depth,offset);        break;
  case Co_Port:        ((Port *) this)->print(stream,depth,offset);        break;
  case Co_Space:       ((Space *) this)->print(stream,depth,offset);       break;
  case Co_Chunk:       ((SChunk *) this)->print(stream,depth,offset);      break;
  case Co_Array:       ((OzArray *) this)->print(stream,depth,offset);     break;
  case Co_Dictionary:  ((OzDictionary *) this)->print(stream,depth,offset);break;
  case Co_Lock:        ((OzLock *) this)->print(stream,depth,offset);break;
  case Co_Thread:      ((Thread *) this)->print(stream,depth,offset);    break;
  case Co_Builtin:     ((BuiltinTabEntry *) this)->print(stream,depth,offset);     break;
  default:             Assert(NO);
  }
}

PRINT(HeapChunk)
{
  CHECKDEPTH;
  stream << indent(offset)
	 << "heap chunk: " << chunk_size << " bytes at " << this << '.';
/*
  char * data = (char *) chunk_data;
  for (int i = 0; i < chunk_size; i += 1)
    stream << indent(offset + 3)
	   << "chunk_data[" << i << "]@" << &data[i] << "="
	   << data[i] << endl;
	   */
}

PRINTLONG(HeapChunk)
{
  CHECKDEPTHLONG;
  stream << indent(offset)
	 << "heap chunk: " << chunk_size << " bytes at " << this << '.';
}


PRINT(ObjectClass)
{
  CHECKDEPTH;
  depth++;
  stream << indent(offset) << "class(fastMethods: ";
  fastMethods->print(stream,depth,offset);
  stream << ", printName: ";
  printName->print(stream,depth,offset);
  stream << ", slowMethods: " << toC(makeTaggedConst(getSlowMethods())) << ", send: ";
  send->print(stream,depth,offset);
  stream << ",";
  if (unfreeFeatures) {
    stream << ", ";
    unfreeFeatures->print(stream,depth,offset);
  }
  stream << ")";
}

PRINTLONG(ObjectClass)
{
  CHECKDEPTHLONG;
  depth++;
  stream << indent(offset) << "class(fastMethods: ";
  fastMethods->printLong(stream,depth,offset);
  stream << ", printName: ";
  printName->printLong(stream,depth,offset);
  stream << ", slowMethods: " << toC(makeTaggedConst(getSlowMethods())) << ", send: ";
  send->printLong(stream,depth,offset);
  stream << ")";
}

void AM::print()
{
  cout << "class AM" << endl
       << "  currentBoard: ";
  currentBoard->print(cout,~1,0);
  cout << endl
       << "  rootBoard:    ";
  rootBoard->print(cout,~1,0);
  cout << endl;
  ThreadsPool::printThreads();
}

PRINT(Board)
{
  CHECKDEPTH;
  stream << indent(offset);
  if (!this) {
    stream << "(NULL Board)";
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

  if (isInstalled()) stream << 'I';
  if (isNervous())   stream << 'N';
  if (isWaitTop())   stream << 'T';
  if (isPathMark())  stream << 'P';
  if (isFailed())    stream << 'F';
  if (isCommitted()) stream << 'C';
  if (isWaiting())   stream << 'W';
  
  stream << " #" << suspCount;
  stream << ']';
}

PRINTLONG(Board)
{
  CHECKDEPTHLONG;
  print(stream,depth,offset); stream << endl;
  stream << indent(offset) << "Flags: " << (void *) flags << endl;
  stream << indent(offset) << "Script: " << endl;
  script.printLong(stream,DEC(depth),offset+2);
  if (isRoot()) return;
  if (isCommitted()) {
    stream << indent(offset) << "Board:" << endl;
    u.ref->printLong(stream,DEC(depth),offset+2);
  } else {
    stream << indent(offset) << "Actor:" << endl;
    u.actor->printLong(stream,DEC(depth),offset+2);
  }
}

void AM::printBoards()
{
  cout << "class Board" << endl
       << "  currentBoard: ";
  currentBoard->print(cout,-1,0);
  cout << endl
       << "  rootBoard:    ";
  rootBoard->print(cout,-1,0);
  cout << endl;
}

PRINT(Script)
{
  CHECKDEPTH;
  stream << indent(offset);
  if (getSize() <= 0) {
    stream << "- empty -";
    return;
  }
  for (int i = 0; i < getSize(); i++) {
    (*this)[i].print(stream,depth,offset);
    stream << ", ";
  }
}

PRINTLONG(Script)
{
  CHECKDEPTHLONG;
  print(stream,depth,offset);
  stream << endl;
}

PRINT(Equation)
{
  CHECKDEPTH;
  tagged2Stream(getLeft(),stream,depth,offset);
  stream << " = ";
  tagged2Stream(getRight(),stream,depth,offset);
}

PRINTLONG(Equation)
{
  CHECKDEPTHLONG;
  print(stream,depth,offset);
}

PRINT(Actor)
{
  CHECKDEPTH;
  if (!this) {
    stream << indent(offset) << "(NULL Actor)";
    return;
  }

  stream << indent(offset);
  if (isAsk()) {
    stream << "Ask";
  } else if (isWait()) {
    stream << "Wait";
  } else if (isSolve()) {
    stream << "Solve";
  } else {
    stream << "Unknown";
  }
  stream << "Actor @"
	 << this;
  if (isCommitted()) {
    stream << " (committed)";
  }
}

PRINTLONG(Actor)
{
  CHECKDEPTHLONG;
  print(stream,depth,offset);
  stream << endl;
  stream << indent(offset) << "Priority: " << priority << endl;
  if (isSolve()) {
    ((SolveActor *)this)->printLong(stream,depth,offset);
  }
  stream << indent(offset) << "Board: " << endl;
  board->printLong(stream,DEC(depth),offset+2);
}

PRINT(SolveActor)
{
  CHECKDEPTH;
}

PRINTLONG(SolveActor)
{
  CHECKDEPTHLONG;
  stream  << indent(offset) << "solveVar=";
  tagged2Stream(solveVar,stream,DEC(depth),0);
  stream << endl;
  stream << indent(offset) << "result=";
  tagged2Stream(result,stream,DEC(depth),0);
  stream << endl;
  stream << indent(offset) << "threads=" << threads << endl;
  stream << indent(offset) << "SuspList:" << endl;
  suspList->print(stream,DEC(depth),offset+2);
  stream << "Local thread queue: " << localThreadQueue << endl;
}

void ThreadsPool::printThreads()
{
  cout << "Threads" << endl
       << "  running: ";
  currentThread->print(cout,-1,0);
  cout << endl
       << "  toplevel:    ";
  rootThread->print(cout,-1,0);
  cout << endl
       << "  runnable:" << endl;

  if (!hiQueue.isEmpty()) {
    cout << "   prio = HI";
    hiQueue.printThreads();
  }
  if (!midQueue.isEmpty()) {
    cout << "   prio = MID";
    midQueue.printThreads();
  }
  if (!lowQueue.isEmpty()) {
    cout << "   prio = LOW";
    lowQueue.printThreads();
  }
}

void ThreadQueue::printThreads()
{
  int i = getSize();
  cout << " #" << i << " threads" << endl;

  for (; i; i--) {
    Thread *th = dequeue();
    th->print (cout,-1,4);
    if (th == am.currentThread)
      cout << " RUNNING ";
    if (th == am.rootThread)
      cout << " ROOT ";
    cout << endl;
    enqueue(th);
  }
}

PRINT(Thread)
{
  CHECKDEPTH;
  if (!this) {
    stream << indent(offset) << "(NULL Thread)" << endl;
    return;
  }

  if (isDeadThread ()) {
    stream << indent(offset) << "(Dead Thread @" << this << ")" << endl;
    return;
  }

  stream << indent(offset) << "Thread @" << this;
  switch (getPriority()) {
  case LOW_PRIORITY:
    stream << " low";
    break;
  case MID_PRIORITY:
    stream << " mid";
    break;
  case HI_PRIORITY:
    stream << " hi";
    break;
  default:
    stream << " (unknown Priority " << getPriority() << ")";
    break;
  }
  stream << (isSuspended() ? " S" : " R");

  switch (getThrType ()) {
  case S_RTHREAD:
    stream << " #" << item.threadBody->taskStack.tasks();
    break;

  case S_WAKEUP: 
    stream << " W";
    break;

  case S_PR_THR:
    stream << " P: " << *getPropagator();
    break;

  default:
    stream << "(unknown)";
  }

  if ((getFlags ()) & T_solve)     stream << " S";
  if ((getFlags ()) & T_ext)       stream << " E";
  if ((getFlags ()) & T_loca)      stream << " L";
  if ((getFlags ()) & T_unif)      stream << " U";
  if ((getFlags ()) & T_ofs)       stream << " O";
  if ((getFlags ()) & T_tag)       stream << " T";
  if ((getFlags ()) & T_ltq)       stream << " Q";
  stream << " <";
  getBoard()->print(stream, DEC(depth));
  stream << ">";
}

PRINTLONG(Thread)
{
  CHECKDEPTHLONG;
  this->print(stream,depth,offset);
  stream << endl;
  if (hasStack ()) 
    item.threadBody->taskStack.printTaskStack (NOCODE,NO,depth);
}

PRINTLONG(Literal)
{
  CHECKDEPTHLONG;
  if (isAtom()) {
    stream << indent(offset) << "Atom @" << this << ": ";
  } else {
    stream << indent(offset) << "Name @" << this << ": ";
  }
  this->print(stream,depth,offset);
  stream << endl;
}


void SVariable::printLong(ostream &stream, int depth, int offset, TaggedRef v)
{
  CHECKDEPTHLONG;
  stream << indent(offset)
	 << "SV "
	 << getVarName(v)
	 << " @"
	 << this
	 << endl
	 << indent(offset)
	 << "SuspList:\n"; 
  suspList->print(stream, DEC(depth), offset+3);
  
  stream << indent(offset)
	 << "HomeNode: ";
  getBoard()->print(stream,DEC(depth));
  stream << endl;
}

void GenCVariable::printLong(ostream &stream, int depth, int offset,
			     TaggedRef v)
{
  CHECKDEPTHLONG;
  stream << indent(offset)
	 << "CV "
	 << getVarName(v)
	 << " @"
	 << this
	 << endl;

  stream << indent(offset) << "Home board: ";
  getBoard()->print(stream,depth);
  stream << endl;

  stream << indent(offset) << "Suspension List:\n"; 
  suspList->print(stream, depth, offset+3);

  switch(getType()){
  case FDVariable:
    stream << indent(offset) << "FD Singleton SuspList:\n"; 
    ((GenFDVariable*)this)->fdSuspList[fd_prop_singl]->print(stream, depth, offset+3);
  
    stream << indent(offset) << "FD Bounds SuspList:\n"; 
    ((GenFDVariable*)this)->fdSuspList[fd_prop_bounds]->print(stream, depth, offset+3);
    stream << indent(offset) << "FD Domain:\n"; 
    ((OZ_FiniteDomainImpl *) &((GenFDVariable*)this)->getDom())->printLong(stream, offset+3);
    break;

  case BoolVariable:
    stream << indent(offset) << "Boolean Domain: {0 1}" << endl; 
    break;

  case FSetVariable:
    stream << indent(offset) << "FSet val SuspList:\n"; 
    ((GenFSetVariable*)this)->fsSuspList[fs_prop_val]->print(stream, depth, offset+3);
  
    stream << indent(offset) << "FSet glb SuspList:\n"; 
    ((GenFSetVariable*)this)->fsSuspList[fs_prop_glb]->print(stream, depth, offset+3);
    
    stream << indent(offset) << "FSet lub SuspList:\n"; 
    ((GenFSetVariable*)this)->fsSuspList[fs_prop_lub]->print(stream, depth, offset+3);
    
    stream << indent(offset) << "FSet :\n" << indent(offset + 3); 
    ((OZ_FSetImpl *) &((GenFSetVariable*)this)->getSet())->print(stream);
    stream << endl;
    break;

  case OFSVariable:
    {
      stream << indent(offset);
      GenOFSVariable* me = (GenOFSVariable *) this;
      tagged2Stream(me->getLabel(),stream,DEC(depth),offset);
      // me->getLabel()->print(stream, DEC(depth), offset);
      me->getTable()->print(stream, DEC(depth), offset+2);
      stream << endl;
      break;
    }

  case MetaVariable:
    {
      GenMetaVariable* me = (GenMetaVariable *) this;
      stream << indent(offset)
	     << "<<MV: '" << me->getName() << "' " << me->toString(DEC(depth))
	     << endl;
      tagged2Stream(me->data, stream, DEC(depth), offset + 2);
      stream << endl << indent(offset) << ">>" << endl;
      break;
    }
  case AVAR:
    {
      AVar* me = (AVar *) this;
      stream << indent(offset); this->print(stream,depth, 0,v);
      stream << endl << indent(offset) << "Value: ";
      tagged2Stream(me->getValue(), stream, DEC(depth), offset + 2);
      stream << endl;
      break;
    }
  default:
    error("Unexpected type generic variable at %s:%d.",
	  __FILE__, __LINE__);
    break;
  }

} // PRINTLONG(GenCVariable)


PRINTLONG(LTuple)
{
  CHECKDEPTHLONG;
  stream << indent(offset) << "List @" << this << endl;

  stream << indent(offset) << "Head:\n";
  tagged2StreamLong(args[0],stream,DEC(depth),offset+2);
  stream << indent(offset) << "Tail:\n";
  tagged2StreamLong(args[1],stream,DEC(depth),offset+2);
}

PRINTLONG(Object)
{
  CHECKDEPTHLONG;
  stream << indent(offset)
	 << "Object: "
	 << getPrintName() << endl
         << "Features: ";
  if (getFreeRecord()) {
    getFreeRecord()->printLong(stream,depth,offset);
  }
  stream << endl;
  stream << "State: ";
  if (getState())
    getState()->printLong(stream,depth,offset);
}

PRINTLONG(Abstraction)
{
  CHECKDEPTHLONG;
  stream << indent(offset)
	 << "Abstraction @id"
	 << this << endl;
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


void AssReg::print()
{
  char c;
  switch (kind) {
  case XReg:
    c = 'X';
    break;
  case YReg:
    c = 'Y';
    break;
  case GReg:
  default:
    c = 'G';
    break;
  }
  printf ("%c%d", c, number);
}

PRINTLONG(PrTabEntry)
{
  CHECKDEPTHLONG;
  stream << indent(offset)
	 <<  "Name: " << getPrintName()
	 << "/" << arity << endl
	 << indent(offset)
	 <<  "ProgramCounter: " << (void *) PC << endl
	 << indent(offset) <<  "Arity: " << arity << endl;
}

PRINTLONG(BuiltinTabEntry)
{
  CHECKDEPTHLONG;
  print(stream,depth,offset);
  stream << endl;
  stream << indent(offset) << "gRegs: -" << endl;
}


PRINTLONG(Cell)
{
  CHECKDEPTHLONG;
  stream << indent(offset)
	 << "Cell@id" << this << endl
	 << indent(offset)
	 << " value:"<<endl;
  tagged2StreamLong(val,stream,depth,offset+2);
}


PRINTLONG(PortLocal)
{
  CHECKDEPTHLONG;
  stream << indent(offset)
	 << "PortLocal@id" << this << endl
	 << indent(offset)
	 << " stream:"<<endl;
  tagged2StreamLong(strm,stream,depth,offset+2);  
}


PRINTLONG(PortManager)
{
  CHECKDEPTHLONG;
  stream << indent(offset)
	 << "PortManager@id" << this << endl
	 << indent(offset)
	 << " stream:"<<endl;
  tagged2StreamLong(strm,stream,depth,offset+2);  
}

PRINTLONG(PortProxy)
{
  CHECKDEPTHLONG;
  stream << indent(offset)
	 << "PortProxy@id" << this << endl;  
}

PRINTLONG(Space)
{
  CHECKDEPTHLONG;
  stream << indent(offset)
	 << "Space@id" << this << endl
	 << indent(offset)
	 << " actor:"<<endl;
  ((SolveActor *) solve->getActor())->printLong(stream,depth,offset+2);
}

PRINTLONG(SChunk)
{
  CHECKDEPTHLONG;
  stream << indent(offset)
	 << "Chunk@id" << this << endl
	 << indent(offset)
	 << " value:"<<endl;
  tagged2StreamLong(value,stream,depth,offset+2);
  stream << indent(offset)
    << " home:"<<endl;
  ((Board *)getPtr())->printLong(stream,depth,offset);
}

PRINTLONG(SRecord)
{
  CHECKDEPTHLONG;
  if (isTuple()) {
    int i;

    stream << indent(offset) << "Tuple @" << this << endl
	   << indent(offset) << "Label: ";
    tagged2StreamLong(label,stream,depth,offset);
    stream << endl;
    for (i = 0; i < getWidth(); i++) {
      stream << indent(offset) <<  "Arg "<< i << ":\n";
      tagged2StreamLong(args[i],stream,DEC(depth),offset);
      stream << " ";
    }
    stream << endl;
    return;
  }
  stream << indent(offset);
  stream << "Record @"
	 << this << ":\n"
	 << indent(offset)
	 << "Label: ";
  tagged2Stream(label,stream,DEC(depth),offset);
  stream << endl;

  stream << indent(offset) << "Args:\n";
  TaggedRef ar = getArityList();
  CHECK_DEREF(ar);
  while (isCons(ar)) {
    stream << indent(offset+2);
    TaggedRef feat = head(ar);
    CHECK_DEREF(feat);
    tagged2Stream(feat,stream,DEC(depth),offset+2);
    ar = tail(ar);
    CHECK_DEREF(ar);
    stream << ": ";
    tagged2StreamLong(getFeature(feat),stream,DEC(depth),offset+2);
  }
  stream << indent(offset) << "End of Args\n";
}


PRINTLONG(Float)
{
  CHECKDEPTHLONG;
  stream << indent(offset) << "Float @" << this << ": ";
  print(stream,depth,offset);
  stream << endl;
}

PRINTLONG(BigInt)
{
  CHECKDEPTHLONG;
  stream << indent(offset)
	 << "BigInt @" << this << ": ";
  tagged2Stream(makeTaggedBigInt(this),stream,depth,offset);
  stream << endl;
}


static
void printX(FILE *fd, RefsArray X)
{
  int xsize = getRefsArraySize(X);
  while (--xsize >= 0) {
    fprintf(fd,"\t\tX[%d]=0x%x\n", xsize, X[xsize]);
  }
}

void TaskStack::printTaskStack(ProgramCounter pc, Bool verbose, int depth)
{
  message("\n");
  message("Stack dump:\n");
  message("-----------\n");

  Assert(this);
  if (pc == NOCODE && isEmpty()) {
    message("\tEMPTY\n");
    return;
  }

  if (pc != NOCODE) {
    CodeArea::printDef(pc);
  }

  TaskStackEntry *auxtos = getTop();

  while (depth-- > 0) {
    PopFrame(auxtos,PC,Y,G);
    if (PC==C_EMPTY_STACK)
      break;
    if (verbose) {
      message("\tC_CONT: PC=0x%x, Y=0x%x, G=0x%x\n\t",
	      PC, Y, G);
    }
    CodeArea::printDef(PC);
  }
}

TaggedRef TaskStack::dbgGetTaskStack(ProgramCounter pc, int depth)
{
  Assert(this);

  TaggedRef out = nil();

  if (pc != NOCODE) {
    out = cons(CodeArea::dbgGetDef(pc),out);
  }

  TaskStackEntry *auxtos = getTop();

  while (depth-- > 0) {
    PopFrame(auxtos,PC,Y,G);
    if (PC==C_EMPTY_STACK)
      break;

    if (PC==C_CFUNC_CONT_Ptr) {
      OZ_CFun biFun    = (OZ_CFun) Y;
      RefsArray X      = (RefsArray) G;
      TaggedRef args = nil();
      
      if (X)
	for (int i=getRefsArraySize(X)-1; i>=0; i--)
	  args = cons(X[i],args);
      else
	args = nil();

      TaggedRef pairlist = 
	cons(OZ_pairA("name", OZ_atom(builtinTab.getName((void *) biFun))),
	     cons(OZ_pairA("args", args),
		  nil()));
      TaggedRef entry = OZ_recordInit(OZ_atom("builtin"), pairlist);
      out = cons(entry, out);
      continue;
    }
    out = cons(CodeArea::dbgGetDef(PC,G,Y),out);
  }

  return reverseC(out);
}


void ThreadQueueImpl::print(void)
{
  if (isEmpty()) {
    message("Thread queue empty.\n");
  } else {
    cout << "Thread #" << size << endl << flush;

    for (int aux_size = size, aux_head = head; 
	 aux_size; 
	 aux_head = (aux_head + 1) & (maxsize - 1), aux_size--) {
      cout << "queue[" << aux_head << "]=" << flush;
      queue[aux_head]->print();
    }
  }
}


#define RANGESTR "#"

void printFromTo(ostream &ofile, int f, int t)
{
  if (f == t)
    ofile << f;
  else if ((t - f) == 1)
    ofile << f << ' ' << t;
  else
    ofile << f << RANGESTR << t;
}

void FDIntervals::print(ostream &ofile, int idnt) const
{
  Bool flag = FALSE;

  ofile << indent(idnt) << '{';
  for (int i = 0; i < high; i += 1) {
    if (flag) ofile << ' '; else flag = TRUE; 
    printFromTo(ofile, i_arr[i].left, i_arr[i].right);
  }
  ofile << "}";
}

void FDIntervals::printLong(ostream &ofile, int idnt) const
{
  ofile << endl << indent(idnt) << "high=" << endl;
  print(ofile, idnt);
  for (int i = 0; i < high; i += 1)
    ofile << endl << indent(idnt)
	  << "i_arr[" << i << "]@" << (void*) &i_arr[i]
	  << " left=" << i_arr[i].left << " right=" << i_arr[i].right;
  ofile << endl;
}

void FDIntervals::printDebug(void) const
{
  print(cerr, 0);
  cerr << endl;
  cerr.flush();
}

void FDIntervals::printDebugLong(void) const
{
  printLong(cerr, 0);
  cerr << endl;
  cerr.flush();
}

void FDBitVector::print(ostream &ofile, int idnt) const
{
  Bool flag = FALSE;

  ofile << indent(idnt) << '{';

  int len = mkRawOutline(fd_bv_left_conv, fd_bv_right_conv);
  for (int i = 0; i < len; i += 1) {
    if (flag) ofile << ' '; else flag = TRUE; 
    ofile << fd_bv_left_conv[i];
    if (fd_bv_left_conv[i] != fd_bv_right_conv[i])
      if (fd_bv_left_conv[i] + 1 == fd_bv_right_conv[i])
	ofile << ' ' << fd_bv_right_conv[i];
      else
	ofile << RANGESTR << fd_bv_right_conv[i];
  }
  ofile << '}';
}

void FDBitVector::printLong(ostream &ofile, int idnt) const
{
  ofile << "  high=" << high << endl;
  print(ofile, idnt);
  for (int i = 0; i < high; i++) {
    ofile << endl << indent(idnt + 2) << '[' << i << "]:  ";
    for (int j = 31; j >= 0; j--) {
      ofile << ((b_arr[i] & (1 << j)) ? '1' : 'o');
      if (j % 8 == 0) ofile << ' ';
    }
  }
  ofile << endl;
}

void FDBitVector::printDebug(void) const
{
  print(cerr, 0);
  cerr << endl;
  cerr.flush();
}

void FDBitVector::printDebugLong(void) const
{
  printLong(cerr, 0);
  cerr << endl;
  cerr.flush();
}

void OZ_FiniteDomainImpl::print(ostream &ofile, int idnt) const
{
  if (getSize() == 0)
    ofile << indent(idnt) << "{ empty }";
  else switch (getType()) {
  case fd_descr:    
      ofile << indent(idnt) << '{';
      printFromTo(ofile, min_elem, max_elem);
      ofile << "}";
    break;
  case bv_descr:
    get_bv()->print(ofile, idnt);
    break;
  case iv_descr:
    get_iv()->print(ofile, idnt);
    break;
  default:
    error("unexpected case");
  }
  //  DEBUG_FD_IR(FALSE, ((getType() == fd_descr) ? 'f' :
  //	      (getType() == bv_descr ? 'b' : 'i')) << '#' << size);
}

void OZ_FiniteDomainImpl::printLong(ostream &ofile, int idnt) const
{
  static char * descr_type_text[3] = {"bv_descr", "iv_descr", "fd_descr"};

  ofile << indent(idnt) << "min_elem=" << min_elem
	<< " max_elem=" << max_elem << " size=" << getSize()
	<< " descr=" << get_iv() << " type=" << descr_type_text[getType()];
  
  switch (getType()) {
  case fd_descr:
    ofile << endl;
    print(ofile, idnt);
    ofile << endl;
    break;
  case bv_descr:
    get_bv()->printLong(ofile, idnt);
    break;
  case iv_descr:
    get_iv()->printLong(ofile, idnt);
    break;
  default:
    error("unexpected case");
  }
}

void OZ_FiniteDomainImpl::printDebug(void) const
{
  print(cerr, 0);
  cerr << endl;
  cerr.flush();
}

void OZ_FiniteDomainImpl::printDebugLong(void) const
{
  printLong(cerr, 0);
  cerr << endl;
  cerr.flush();
}

#ifdef RECINSTRFETCH

#define InstrDumpFile "fetchedInstr.dump"

void CodeArea::writeInstr(void){
  FILE* ofile;
  if((ofile = fopen(InstrDumpFile, "w"))){
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
    bb->print(cout,1,off);
    cout << endl;
    Assert(!bb->isCommitted());
    off++;
    aa = bb->u.actor;
    aa->print(cout,1,off);
    cout << endl;
    off++;
    bb = aa->getBoard();
  }
  bb->print(cout,1,off);
  cout << endl;
}

void printWhere(ostream &stream,ProgramCounter PC)
{
  PC = CodeArea::definitionStart(PC);

  if (PC == NOCODE) {
    stream << "in toplevel code";
  } else {
    TaggedRef file      = getLiteralArg(PC+3);
    TaggedRef line      = getNumberArg(PC+4);
    PrTabEntry *pred    = getPredArg(PC+5);
	
    stream << "procedure "
	   << (pred ? pred->getPrintName() : "(NULL)")
	   << " in file "
	   << toC(file)
	   << " at line "
	   << toC(line);
  }
}
