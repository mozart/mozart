/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

/*
 * oz_printStream(OZ_Term t, ostream &stream, int depth=-1, int width=-1);
 *   print t in canonical for on stream without newlines
 *   (-1 means use ozconf.printDepth/Width)
 *
 * oz_print(OZ_Term t);
 *   dito, but on stderr and with newline
 *
 * char *OZ_toC(OZ_Term t, int depth, int width)
 *   dito, but return a string
 *   NOTE: the string is allocated in a static area and the next call
 *         of OZ_toC overwrites it.
 *
 * char *toC(OZ_Term)
 *    dito, using ozconf.errorPrintDepth/Width
 *
 *
 * naming convention for debug methods of objects:
 *
 *   printStream(ostream &stream, int depth=20)
 *     print short description of object without newline on stream
 *
 *   print()
 *     dito, but with newline on stderr
 *
 *   printLongStream(ostream &stream, int depth=20)
 *     print long description of object using several lines on stream
 *
 *   printLong()
 *     dito, but on stderr
 *
 * printing values
 *
 *   ozd_printStream(OZ_Term val, ostream &stream, int depth=20)
 *     print short description of val without newline on stream
 *
 *   ozd_print(OZ_Term val)
 *     dito, but with newline on stderr
 *
 *   ozd_printLongStream(OZ_Term val, ostream &stream,
 *                       int depth = 20, int offset = 0)
 *     print long description of val using several lines on stream
 *
 *   ozd_printLong(OZ_Term val)
 *     dito, but on stderr
 *
 *
 * macros used in class definitions
 *
 *   OZPRINT
 *     declare all debug methods
 *     only printStream needs to be implemented
 *
 *   OZPRINTLONG
 *     dito, but also printLongStream must be implemented
 */

#include "runtime.hh"

#ifdef DEBUG_PRINT

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


#define PRINT_DEPTH_DEC(depth) ((depth)==-1?(depth):(depth)-1)

//-----------------------------------------------------------------------------
//                         Miscellaneous stuff

// returns OK if associated suspension is alive
inline Bool isEffectiveSusp(SuspList* sl)
{
  Thread *thr = sl->getElem ();
  if (thr->isDeadThread ())
    return NO;
  if (!GETBOARD(thr))
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


void ozd_printStream(OZ_Term val, ostream &stream, int depth)
{
  if (!val) {
    stream << "<NULL>";
    return;
  }

  if (depth == 0) {
    stream << ",,,";
    return;
  }

  OZ_Term ref=deref(val);

  switch(tagTypeOf(ref)) {
  case UVAR:
    stream << getVarName(val);
    stream << "<UV @" << &ref << ">";
    break;

  case SVAR:
    stream << getVarName(val);
    tagged2SVar(ref)->printStream(stream,depth);
    break;
  case CVAR:
    stream << getVarName(val);
    tagged2CVar(ref)->printStream(stream, depth);
    break;
  case SRECORD:
    tagged2SRecord(ref)->printStream(stream,depth);
    break;
  case LTUPLE:
    tagged2LTuple(ref)->printStream(stream,depth);
    break;
  case LITERAL:
    tagged2Literal(ref)->printStream(stream,depth);
    break;
  case OZFLOAT:
    tagged2Float(ref)->printStream(stream,depth);
    break;
  case BIGINT:
    tagged2BigInt(ref)->printStream(stream,depth);
    break;
  case SMALLINT:
    stream << "<SmallInt @" << &ref << ": " << toC(ref) << ">";
    break;
  case OZCONST:
    tagged2Const(ref)->printStream(stream,depth);
    break;
  case FSETVALUE:
    ((FSetValue *) tagged2FSetValue(ref))->print(stream,depth);
    break;
  default:
    stream << "<unknown tag " << (int) tagTypeOf(ref) << ">";
    break;
  }
}

void ozd_print(OZ_Term val) {
  ozd_printStream(val,cerr);
  cerr << endl;
  flush(cerr);
}

void ozd_printLongStream(OZ_Term val, ostream &stream, int depth, int offset)
{
  if (!val) {
    stream << indent(offset) << "*** NULL TERM ***" << endl;
    return;
  }

  if (depth == 0) {
    stream << indent(offset) << ",,," << endl;
    return;
  }

  OZ_Term ref=val;
  if (isRef(ref)) {
    stream << indent(offset) << "Reference chain: ";
    while (isRef(ref)) {
      stream << '@'
             << (void *) tagged2Ref(ref);
      ref = *tagged2Ref(ref);
    }
    stream << endl;
  }

  switch(tagTypeOf(ref)) {
  case UVAR:
    stream << indent(offset) << getVarName(val);
    stream << indent(offset) << "<UV @" << &ref << ">" << endl;
    stream << indent(offset) << "Home: ";
    tagged2VarHome(ref)->derefBoard()
      ->printStream(stream,PRINT_DEPTH_DEC(depth));
    stream << endl;
    break;

  case SVAR:
    stream << indent(offset) << getVarName(val);
    tagged2SVar(ref)->printLongStream(stream,depth,offset);
    break;
  case CVAR:
    stream << indent(offset) << getVarName(val);
    tagged2CVar(ref)->printLongStream(stream, depth, offset);
    break;
  case SRECORD:
    tagged2SRecord(ref)->printLongStream(stream,depth,offset);
    break;
  case LTUPLE:
    tagged2LTuple(ref)->printLongStream(stream,depth,offset);
    break;
  case LITERAL:
    tagged2Literal(ref)->printLongStream(stream,depth,offset);
    break;
  case OZFLOAT:
    tagged2Float(ref)->printLongStream(stream,depth,offset);
    break;
  case BIGINT:
    tagged2BigInt(ref)->printLongStream(stream,depth,offset);
    break;
  case SMALLINT:
    break;
  case OZCONST:
    tagged2Const(ref)->printLongStream(stream,depth,offset);
    break;
  case FSETVALUE:
    ((FSetValue *) tagged2FSetValue(ref))
      ->print(stream,depth);
    break;
  default:
    break;
  }
}

void ozd_printLong(OZ_Term val) {
  ozd_printLongStream(val,cerr);
  flush(cerr);
}


// ----------------------------------------------------------------
// PRINT
// ----------------------------------------------------------------

void SVariable::printStream(ostream &stream, int depth)
{
  stream << "<SV @" << this
         << (isEffectiveList(suspList) ? "*" : "") << ">";
}

void SVariable::printLongStream(ostream &stream, int depth, int offset)
{
  this->printStream(stream);
  stream << endl
         << indent(offset)
         << "SuspList:\n";
  suspList->printLongStream(stream, PRINT_DEPTH_DEC(depth), offset+3);

  stream << indent(offset)
         << "Home: ";
  GETBOARD(this)->printStream(stream,PRINT_DEPTH_DEC(depth));
  stream << endl;
}

void GenCVariable::printStream(ostream &stream, int depth)
{
  stream << "<CV @" << this;
  if (isEffectiveList(suspList))
    stream << " a" << suspList->length();

  switch(getType()){
  case FDVariable:
    {
      GenFDVariable * me = (GenFDVariable *) this;
      if (isEffectiveList(me->fdSuspList[fd_prop_singl]))
        stream << " s("
               << me->fdSuspList[fd_prop_singl]->length()
               << '/'
               << me->fdSuspList[fd_prop_singl]->lengthProp()
               << ')';
      if (isEffectiveList(me->fdSuspList[fd_prop_bounds]))
        stream << " b(" << me->fdSuspList[fd_prop_bounds]->length()
               << '/'
               << me->fdSuspList[fd_prop_bounds]->lengthProp()
               << ')';
      stream << ' ' <<  me->getDom().toString();
      break;
    }

  case BoolVariable:
    {
      stream << " {0 1}";
      break;
    }

  case FSetVariable:
    {
      GenFSetVariable * me = (GenFSetVariable *) this;
      if (isEffectiveList(me->fsSuspList[fs_prop_val]))
        stream << " val("
               << me->fsSuspList[fs_prop_val]->length()
               << '/'
               << me->fsSuspList[fs_prop_val]->lengthProp()
               << ')';
      if (isEffectiveList(me->fsSuspList[fs_prop_glb]))
        stream << " glb(" << me->fsSuspList[fs_prop_glb]->length()
               << '/'
               << me->fsSuspList[fs_prop_glb]->lengthProp()
               << ')';
      if (isEffectiveList(me->fsSuspList[fs_prop_lub]))
        stream << " lub(" << me->fsSuspList[fs_prop_lub]->length()
               << '/'
               << me->fsSuspList[fs_prop_lub]->lengthProp()
               << ')';
      stream << ' ' <<  me->getSet().toString();
      break;
    }

  case OFSVariable:
    {
      stream << ' ';
      GenOFSVariable* me = (GenOFSVariable *) this;
      stream << " ";
      ozd_printStream(me->getLabel(),stream,depth);
      me->getTable()->printStream(stream,PRINT_DEPTH_DEC(depth));
      break;
   }

  case MetaVariable:
    {
      GenMetaVariable* me = (GenMetaVariable *) this;
      stream << " MV " << me->toString(PRINT_DEPTH_DEC(depth));
      break;
    }
  case AVAR:
    stream << " AVAR";
    break;
  case PerdioVariable:
    stream << " PerdioVariable";
      break;

  default:
    stream << " unknown type: " << getType();
    break;
  }
  stream << ">";
} // PRINTSTREAM(GenCVariable)

void GenCVariable::printLongStream(ostream &stream, int depth, int offset)
{
  this->printStream(stream);

  stream << endl
         << indent(offset) << "Home: ";
  GETBOARD(this)->printStream(stream,depth);
  stream << endl;

  stream << indent(offset) << "Suspension List:\n";
  suspList->printLongStream(stream, depth, offset+3);

  switch(getType()){
  case FDVariable:
    stream << indent(offset) << "FD Singleton SuspList:\n";
    ((GenFDVariable*)this)->fdSuspList[fd_prop_singl]
      ->printLongStream(stream, depth, offset+3);
      stream << indent(offset) << "FD Bounds SuspList:\n";
    ((GenFDVariable*)this)->fdSuspList[fd_prop_bounds]
      ->printLongStream(stream, depth, offset+3);
    stream << indent(offset) << "FD Domain:\n";
    ((OZ_FiniteDomainImpl *) &((GenFDVariable*)this)->getDom())
      ->printLong(stream, offset+3);
    break;

  case BoolVariable:
    stream << indent(offset) << "Boolean Domain: {0 1}" << endl;
    break;

  case FSetVariable:
    stream << indent(offset) << "FSet val SuspList:\n";
    ((GenFSetVariable*)this)->fsSuspList[fs_prop_val]
      ->printLongStream(stream, depth, offset+3);
    stream << indent(offset) << "FSet glb SuspList:\n";
    ((GenFSetVariable*)this)->fsSuspList[fs_prop_glb]
      ->printLongStream(stream, depth, offset+3);
    stream << indent(offset) << "FSet lub SuspList:\n";
    ((GenFSetVariable*)this)->fsSuspList[fs_prop_lub]
      ->printLongStream(stream, depth, offset+3);
    stream << indent(offset) << "FSet :\n" << indent(offset + 3);
    ((FSetConstraint *) &((GenFSetVariable*)this)->getSet())
      ->print(stream);
    stream << endl;
    break;

  case OFSVariable:
    {
      stream << indent(offset);
      GenOFSVariable* me = (GenOFSVariable *) this;
      ozd_printStream(me->getLabel(),stream,PRINT_DEPTH_DEC(depth));
      // me->getLabel()->printStream(stream, PRINT_DEPTH_DEC(depth));
      me->getTable()->printStream(stream, PRINT_DEPTH_DEC(depth));
      stream << endl;
      break;
    }

  case MetaVariable:
    {
      GenMetaVariable* me = (GenMetaVariable *) this;
      stream << indent(offset)
             << "<<MV: '" << me->getName() << "' "
             << me->toString(PRINT_DEPTH_DEC(depth))
             << endl;
      ozd_printStream(me->data, stream, PRINT_DEPTH_DEC(depth));
      stream << endl << indent(offset) << ">>" << endl;
      break;
    }
  case PerdioVariable:
    stream << indent(offset) << "<PerdioVariable *" << this << ">" << endl;
      break;
  case AVAR:
    {
      AVar* me = (AVar *) this;
      stream << indent(offset); this->printStream(stream,depth);
      stream << endl << indent(offset) << "Value: ";
      ozd_printStream(me->getValue(), stream, PRINT_DEPTH_DEC(depth));
      stream << endl;
      break;
    }
  default:
    stream << indent(offset) << " unknown type: " << getType() << endl;
    break;
  }

} // printLongStream(GenCVariable)


// Non-Name Features are output in alphanumeric order (ints before atoms):
void DynamicTable::printStream(ostream &stream, int depth)
{
    stream << '(';
    int nonempty=FALSE;
    // Count Atoms & Names in dynamictable:
    OZ_Term tmplit,tmpval;
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
    TaggedRef *arr = new TaggedRef[nAtomOrInt+1];
    //      +1 since nAtomOrInt may be zero
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
      stream << " ";
      ozd_printStream(arr[ai],stream,depth);
      stream << ": ";
      ozd_printStream(lookup(arr[ai]),stream,PRINT_DEPTH_DEC(depth));
    }
    // Output the Names last, unordered:
    for (di=0; di<size; di++) {
        tmplit=table[di].ident;
        tmpval=table[di].value;
        if (tmpval!=makeTaggedNULL() && !(isAtom(tmplit)||isInt(tmplit))) {
          stream << " ";
          ozd_printStream(tmplit,stream,depth);
          stream << ": ";
          ozd_printStream(tmpval,stream,PRINT_DEPTH_DEC(depth));
        }
    }
    // Deallocate array:
    delete arr;
    // Finish up the output:
    if (nonempty) stream << ' ';
    stream << "...)" ;
}

void SRecord::printStream(ostream &stream, int depth)
{
  ozd_printStream(getLabel(),stream,depth);
  stream << '(';
  if (isTuple()) {
    int len = getWidth();
    for (int i=0; i < len; i++) {
      stream << ' ';
      ozd_printStream(getArg(i), stream, PRINT_DEPTH_DEC(depth));
    }
  } else {
    OZ_Term as = getArityList();
    Assert(isCons(as));
    while (isCons(as)) {
      stream << ' ';
      ozd_printStream(head(as), stream, PRINT_DEPTH_DEC(depth));
      stream << ": ";
      ozd_printStream(getFeature(head(as)), stream, PRINT_DEPTH_DEC(depth));
      as = tail(as);
    }
  }
  stream << ')';
}

void LTuple::printStream(ostream &stream, int depth)
{
  stream << "|(";
  ozd_printStream(getHead(), stream, PRINT_DEPTH_DEC(depth));
  ozd_printStream(getTail(), stream, PRINT_DEPTH_DEC(depth));
  stream << ")";
}


void Literal::printStream(ostream &stream, int depth)
{
  if (isAtom()) {
    stream << "<Atom " << getPrintName() << ">";
  } else {
    stream << "<";
    if (isNamedName()) {
      stream << "Named";
    }
    if (isUniqueName()) {
      stream << "Unique";
    }
    stream << "Name " << getPrintName() << ">";
  }
}

void Float::printStream(ostream &stream, int depth)
{
  stream << "<Float " << toC(makeTaggedFloat(this)) << ">";
}

void CellLocal::printStream(ostream &stream, int depth)
{
  stream << "CellLocal@" << this;
}

void CellManager::printStream(ostream &stream, int depth)
{
  stream << "CellManager@" << this;
}

void CellProxy::printStream(ostream &stream, int depth)
{
  stream << "CellProxy@" << this;
}

void CellFrame::printStream(ostream &stream, int depth)
{
  stream << "CellFrame@" << this;
}

void PortLocal::printStream(ostream &stream, int depth)
{
  stream << "PortLocal@" << this;
}

void PortProxy::printStream(ostream &stream, int depth)
{
  stream << "PortProxy@" << this;
}

void PortManager::printStream(ostream &stream, int depth)
{
  stream << "PortManager@" << this;
}

void Space::printStream(ostream &stream, int depth)
{
  stream << "Space@" << this;
}

void OzArray::printStream(ostream &stream, int depth)
{
  stream << "Array@" << this << "[ ";
  for (int i=getLow(); i<=getHigh(); i++) {
    stream << i << ": ";
    TaggedRef t=getArg(i);
    ozd_printStream(t,stream, PRINT_DEPTH_DEC(depth));
    stream << " ";
  }
  stream << "]";
}

void OzDictionary::printStream(ostream &stream, int depth)
{
  stream << "<Dictionary@" << this << ">";
}

void LockLocal::printStream(ostream &stream, int depth)
{
  stream << "<LockLocal@" << this << ">";
}

void LockProxy::printStream(ostream &stream, int depth)
{
  stream << "<LockProxy@" << this << ">";
}

void LockFrame::printStream(ostream &stream, int depth)
{
  stream << "<LockFrame@" << this << ">";
}

void LockManager::printStream(ostream &stream, int depth)
{
  stream << "<LockManager@" << this << ">";
}

void SChunk::printStream(ostream &stream, int depth)
{
  stream << "Chunk@" << this;
}

void Abstraction::printStream(ostream &stream, int depth)
{
  stream << "P:"
         << getPrintName() << "/" << getArity();
}

void Object::printStream(ostream &stream, int depth)
{
  stream << "<O:" << getPrintName()
         << ", ";
  if (getFreeRecord())
    getFreeRecord()->printStream(stream,depth);
  else
    stream << "nofreefeatures";
  stream << ", State: ";
  RecOrCell state = getState();
  if (stateIsCell(state)) {
    getCell(state)->printStream(stream,depth);
  } else if(getRecord(state)) {
    getRecord(state)->printStream(stream,depth);
  }
  stream << ">";
}

void ObjectClass::printStream(ostream &stream, int depth)
{
  stream << "C:" << getPrintName();
}

void BuiltinTabEntry::printStream(ostream &stream, int depth)
{
  stream << "B:"
         << getPrintName() << "/" << getArity();
}


void Arity::printStream(ostream &stream, int depth)
{
  stream << (isTuple() ? "Tuple" : "Record") << "Arity: #"
         << getWidth() << endl;
  ozd_printStream(list,stream,depth);
  stream << endl;
  if (!isTuple()) {
    stream << "Hashtable:" << endl;
    DebugCode(stream << " Collisions: " << numberOfCollisions << endl);
    for(int i=0; i<getSize(); i++) {
      stream << " " << i << ": ";
      if (!table[i].key) {
        stream << "<empty>" << endl;
      } else {
        ozd_printStream(table[i].key,stream,depth);
        stream << " " << table[i].index
               << " (#" << featureHash(table[i].key)
               << " " << hashfold(featureHash(table[i].key)) << ")"
               << endl;
      }
    }
  }
}


void ArityTable::printStream(ostream &stream, int depth)
{
  for (int i = 0 ; i < size ; i ++) {
    Arity *ar = table[i];
    while (ar) {
      ar->printStream(stream,depth);
      ar = ar->next;
    }
  }
}


// ---------------------------------------------------

void SuspList::printStream(ostream &stream, int depth) {
}

void SuspList::printLongStream(ostream &stream, int depth, int offset)
{
  if (!isEffectiveList(this)) {
    stream << indent(offset) << "- empty -" << endl;
    return;
  }

  for (SuspList* sl = this; sl != NULL; sl = sl->getNext()) {
    if (isEffectiveSusp(sl)) {
      stream << indent(offset);
      (sl->getElem ())->printStream(stream);
      stream << endl;
    }
  } // for
}

void ConstTerm::printLongStream(ostream &stream, int depth, int offset)
{
  switch (getType()) {
  case Co_HeapChunk:
    ((HeapChunk *) this)->printLongStream(stream, depth, offset);
    break;
  case Co_Abstraction:
    ((Abstraction *) this)->printLongStream(stream,depth,offset);
    break;
  case Co_Object:
    ((Object *) this)->printLongStream(stream,depth,offset);
    break;
  case Co_Class:
    ((ObjectClass *) this)->printLongStream(stream,depth,offset);
    break;
  case Co_Cell:
    switch(((Tertiary *)this)->getTertType()){
    case Te_Local:
      ((CellLocal *)   this)->printLongStream(stream,depth,offset);
      break;
    case Te_Frame:
      ((CellFrame *)   this)->printLongStream(stream,depth,offset);
      break;
    case Te_Manager:
      ((CellManager *) this)->printLongStream(stream,depth,offset);
      break;
    case Te_Proxy:
      ((CellProxy *)   this)->printLongStream(stream,depth,offset);
      break;
    default:
      Assert(NO);
    }
    break;
  case Co_Port:
    switch(((Tertiary *)this)->getTertType()){
    case Te_Local:
      ((PortLocal *)   this)->printLongStream(stream,depth,offset);
      break;
    case Te_Manager:
      ((PortManager *) this)->printLongStream(stream,depth,offset);
      break;
    case Te_Proxy:
      ((PortProxy *)   this)->printLongStream(stream,depth,offset);
      break;
    default:
      Assert(NO);
    }
    break;
  case Co_Space:
    ((Space *) this)->printLongStream(stream,depth,offset);
    break;
  case Co_Chunk:
    ((SChunk *) this)->printLongStream(stream,depth,offset);
    break;
  case Co_Array:
    ((OzArray *) this)->printLongStream(stream,depth,offset);
    break;
  case Co_Dictionary:
    ((OzDictionary *) this)->printLongStream(stream,depth,offset);
    break;
  case Co_Lock:
    switch(((Tertiary *)this)->getTertType()){
    case Te_Local:
      ((LockLocal *)   this)->printLongStream(stream,depth,offset);
      break;
    case Te_Frame:
      ((LockFrame *)   this)->printLongStream(stream,depth,offset);
      break;
    case Te_Manager:
      ((LockManager *) this)->printLongStream(stream,depth,offset);
      break;
    case Te_Proxy:
      ((LockProxy *)   this)->printLongStream(stream,depth,offset);
      break;
    default:         Assert(NO);
    }
    break;
  case Co_Thread:
    ((Thread *) this)->printLongStream(stream,depth,offset);
    break;
  case Co_Builtin:
    ((BuiltinTabEntry *) this)->printLongStream(stream,depth,offset);
    break;
#ifdef FOREIGN_POINTER
  case Co_Foreign_Pointer:
    ((ForeignPointer*)this)->printLongStream(stream,depth,offset); break;
#endif
  default:            Assert(NO);
  }
}

void ConstTerm::printStream(ostream &stream, int depth)
{
  switch (getType()) {
  case Co_HeapChunk:   ((HeapChunk *) this)->printStream(stream, depth);
    break;
  case Co_Abstraction: ((Abstraction *) this)->printStream(stream,depth);
    break;
  case Co_Object:      ((Object *) this)->printStream(stream,depth);
    break;
  case Co_Class:       ((ObjectClass *) this)->printStream(stream,depth);
    break;
  case Co_Cell:        ((CellLocal *) this)->printStream(stream,depth);
    break;
  case Co_Port:        ((Port *) this)->printStream(stream,depth);
    break;
  case Co_Space:       ((Space *) this)->printStream(stream,depth);
    break;
  case Co_Chunk:       ((SChunk *) this)->printStream(stream,depth);
    break;
  case Co_Array:       ((OzArray *) this)->printStream(stream,depth);
    break;
  case Co_Dictionary:  ((OzDictionary *) this)->printStream(stream,depth);
    break;
  case Co_Lock:        ((LockLocal *) this)->printStream(stream,depth);
    break;
  case Co_Thread:      ((Thread *) this)->printStream(stream,depth);
    break;
  case Co_Builtin:     ((BuiltinTabEntry *) this)->printStream(stream,depth);
    break;
#ifdef FOREIGN_POINTER
  case Co_Foreign_Pointer:
    ((ForeignPointer*)this)->printStream(stream,depth); break;
#endif
  default:             Assert(NO);
  }
}

void HeapChunk::printStream(ostream &stream, int depth)
{
  stream << "heap chunk: " << (int) chunk_size << " bytes at " << this << '.';
/*
  char * data = (char *) chunk_data;
  for (int i = 0; i < chunk_size; i += 1)
    stream << "chunk_data[" << i << "]@" << &data[i] << "="
           << data[i] << endl;
           */
}

#ifdef FOREIGN_POINTER
void ForeignPointer::printStream(ostream &stream, int depth)
{
  stream << "foreign pointer: " << getPointer() << " at " << this << '.';
}
#endif


void ObjectClass::printLongStream(ostream &stream, int depth, int offset)
{
  stream << indent(offset)
         << "Class: "
         << getPrintName() << endl;
}


void Board::printStream(ostream &stream, int depth)
{
  if (!this) {
    stream << "(NULL Board)";
    return;
  }

  if (_isRoot()) {
    stream << "Root";
  } else if (isWait()) {
    stream << "Wait";
  } else if (isAsk()) {
    stream << "Ask";
  } else if (isSolve ()) {
    stream << "Solve";
  }

  stream << "Board @" << this << " [";

  if (isInstalled())    stream << " Installed";
  if (isNervous())      stream << " Nervous";
  if (isWaitTop())      stream << " WaitTop";
  if (isMarkedGlobal()) stream << " MarkedGlobal";
  if (isFailed())       stream << " Failed";
  if (isCommitted())    stream << " Committed";
  if (isWaiting())      stream << " Waiting";

  stream << " Suspensions: #" << suspCount;
  stream << " ]";
}

void Board::printLongStream(ostream &stream, int depth, int offset)
{
  // printTree?
  if (depth == 0) {
    stream << indent(offset) << ",,," << endl;
    return;
  }

  stream << indent(offset);
  printStream(stream,depth);
  stream << endl;

  stream << indent(offset) << "Flags: " << (void *) flags << endl;
  stream << indent(offset) << "Script: " << endl;
  script.printLongStream(stream,PRINT_DEPTH_DEC(depth),offset+2);
  if (_isRoot()) return;
  if (isCommitted()) {
    stream << indent(offset) << "Board:" << endl;
    u.ref->printLongStream(stream,PRINT_DEPTH_DEC(depth),offset+2);
  } else {
    stream << indent(offset) << "Actor:" << endl;
    u.actor->printLongStream(stream,PRINT_DEPTH_DEC(depth),offset+2);
  }
}

void Script::printStream(ostream &stream, int depth)
{
  if (getSize() <= 0) {
    stream << "- empty -";
    return;
  }
  for (int i = 0; i < getSize(); i++) {
    (*this)[i].printStream(stream,depth);
    stream << ", ";
  }
}

void Equation::printStream(ostream &stream, int depth)
{
  ozd_printStream(getLeft(),stream,depth);
  stream << " = ";
  ozd_printStream(getRight(),stream,depth);
}

void Actor::printStream(ostream &stream, int depth)
{
  if (!this) {
    stream << "(NULL Actor)";
    return;
  }
  if (depth == 0) {
    stream << ",,,";
    return;
  }

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

void Actor::printLongStream(ostream &stream, int depth, int offset)
{
  if (depth == 0) {
    stream << indent(offset) << ",,," << endl;
    return;
  }
  printStream(stream,depth);
  stream << endl;
  if (isSolve()) {
    ((SolveActor *)this)->printLongStreamSolve(stream,depth,offset);
  }
  stream << indent(offset) << "Board: " << endl;
  board->printLongStream(stream,PRINT_DEPTH_DEC(depth),offset+2);
}

void SolveActor::printLongStreamSolve(ostream &stream, int depth, int offset)
{
  stream  << indent(offset) << "solveVar=";
  ozd_printStream(solveVar,stream,PRINT_DEPTH_DEC(depth));
  stream << endl;
  stream << indent(offset) << "result=";
  ozd_printStream(result,stream,PRINT_DEPTH_DEC(depth));
  stream << endl;
  stream << indent(offset) << "threads=" << threads << endl;
  stream << indent(offset) << "SuspList:" << endl;
  suspList->printLongStream(stream,PRINT_DEPTH_DEC(depth),offset+2);
  stream << "Local thread queue: " << localThreadQueue << endl;
}

void ThreadsPool::printThreads()
{
  cout << "Threads" << endl
       << "  running: ";
  _currentThread->printStream(cout,-1);
  cout << endl
       << "  toplevel:    ";
  _rootThread->printStream(cout,-1);
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
    th->printStream(cout,-1);
    if (th == am.currentThread())
      cout << " RUNNING ";
    if (th == am.rootThread())
      cout << " ROOT ";
    cout << endl;
    enqueue(th);
  }
}

void ozd_printBoards()
{
  cout << "class Board" << endl
       << "  currentBoard: ";
  am.currentBoard()->printStream(cout,-1);
  cout << endl;
#ifdef NOMORE
  cout << "  rootBoard:    ";
  am.rootBoard->printStream(cout,-1);
  cout << endl;
#endif
}

void ozd_printThreads()
{
  am.printThreads();
}

void ozd_printAM()
{
  cout << "class AM" << endl;
  ozd_printBoards();
  ozd_printThreads();
}

void Thread::printStream(ostream &stream, int depth)
{
  if (!this) {
    stream << "NULL Thread" << endl;
    return;
  }

  if (isDeadThread ()) {
    stream << "Dead Thread @" << this << "" << endl;
    return;
  }

  stream << "Thread @" << this;

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

  stream << (isSuspended() ? " (susp)" : " (run)");

  switch (getThrType ()) {
  case S_RTHREAD:
    stream << " Tasks #" << item.threadBody->taskStack.tasks();
    break;

  case S_WAKEUP:
    stream << " Wakeup";
    break;

  case S_PR_THR:
    stream << " Propagator: " << getPropagator()->toString();
    break;

  default:
    stream << "(unknown type " << getThrType() << ")";
  }

  if ((getFlags ()) & T_solve)     stream << " solve";
  if ((getFlags ()) & T_ext)       stream << " ext";
  if ((getFlags ()) & T_loca)      stream << " loca";
  if ((getFlags ()) & T_unif)      stream << " unif";
  if ((getFlags ()) & T_ofs)       stream << " ofs";
  if ((getFlags ()) & T_tag)       stream << " tag";
  if ((getFlags ()) & T_ltq)       stream << " ltq";
  if ((getFlags ()) & T_nmo)       stream << " nmo";
  stream << " <";
  GETBOARD(this)->printStream(stream, PRINT_DEPTH_DEC(depth));
  stream << ">";
}

void Thread::printLongStream(ostream &stream, int depth, int offset)
{
  this->printStream(stream,depth);
  stream << endl;
  if (hasStack())
    item.threadBody->taskStack.printTaskStack(depth);
}

void Literal::printLongStream(ostream &stream, int depth, int offset)
{
  stream << indent(offset);
  this->printStream(stream,depth);
  stream << endl;
  stream << " Hash: " << this->hash() << " @" << this;
  stream << endl;
}

void LTuple::printLongStream(ostream &stream, int depth, int offset)
{
  stream << indent(offset) << "List @" << this << endl;

  stream << indent(offset) << "Head:\n";
  ozd_printLongStream(args[0],stream,PRINT_DEPTH_DEC(depth),offset+2);
  stream << indent(offset) << "Tail:\n";
  ozd_printLongStream(args[1],stream,PRINT_DEPTH_DEC(depth),offset+2);
}

void Object::printLongStream(ostream &stream, int depth, int offset)
{
  stream << indent(offset)
         << "Object: "
         << getPrintName() << endl
         << "Features: ";
  if (getFreeRecord()) {
    getFreeRecord()->printLongStream(stream,depth,offset);
  }
  stream << endl;
  stream << "State: ";
  if (stateIsCell(state)) {
    getCell(state)->printLongStream(stream,depth,offset);
  } else if(getRecord(state)) {
    getRecord(state)->printLongStream(stream,depth,offset);
  }
}

void Abstraction::printLongStream(ostream &stream, int depth, int offset)
{
  stream << indent(offset)
         << "Abstraction @id"
         << this << endl;
  getPred()->printLongStream(stream,depth,offset);
  int n = gRegs ? getRefsArraySize(gRegs) : 0;
  if (offset == 0) {
    if (n > 0) {
      stream <<  "G Regs:";
      for (int i = 0; i < n; i++) {
        stream << " G[" << i << "]:\n";
        ozd_printLongStream(gRegs[i],stream,depth,offset+2);
      }
    } else {
      stream << "No G-Regs" << endl;
    }
  } else {
    stream << indent(offset) << "G Regs: #" << n << endl;
  }
}


void AssReg::printStream(ostream &stream, int depth)
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
  stream << c << number;
}

void PrTabEntry::printStream(ostream &stream, int depth) {
  stream << "<PrTabEntry: " << getPrintName()
         << "/" << (int) arity <<  "PC: " << (void *) PC
         << ">";
}

void BuiltinTabEntry::printLongStream(ostream &stream, int depth, int offset)
{
  printStream(stream,depth);
  stream << endl;
  stream << indent(offset) << "gRegs: -" << endl;
}


void CellLocal::printLongStream(ostream &stream, int depth, int offset)
{
  stream << indent(offset)
         << "CellLocal@id" << this << endl
         << indent(offset)
         << " value:"<<endl;
  ozd_printLongStream(val,stream,depth,offset+2);
}

void PortLocal::printLongStream(ostream &stream, int depth, int offset)
{
  stream << indent(offset)
         << "PortLocal@id" << this << endl
         << indent(offset)
         << " stream:"<<endl;
  ozd_printLongStream(strm,stream,depth,offset+2);
}


void PortManager::printLongStream(ostream &stream, int depth, int offset)
{
  stream << indent(offset)
         << "PortManager@id" << this << endl
         << indent(offset)
         << " stream:"<<endl;
  ozd_printLongStream(strm,stream,depth,offset+2);
}

void Space::printLongStream(ostream &stream, int depth, int offset)
{
  stream << indent(offset)
         << "Space@id" << this << endl
         << indent(offset)
         << " actor:"<<endl;
  ((SolveActor *) solve->getActor())->printLongStream(stream,depth,offset+2);
}

void SChunk::printLongStream(ostream &stream, int depth, int offset)
{
  stream << indent(offset)
         << "Chunk@id" << this << endl
         << indent(offset)
         << " value:"<<endl;
  ozd_printLongStream(value,stream,depth,offset+2);
  stream << indent(offset)
    << " home:"<<endl;
  ((Board *)getPtr())->printLongStream(stream,depth,offset);
}

void SRecord::printLongStream(ostream &stream, int depth, int offset)
{
  if (isTuple()) {
    int i;

    stream << indent(offset) << "Tuple @" << this << endl
           << indent(offset) << "Label: ";
    ozd_printStream(label,stream,depth);
    stream << endl;
    for (i = 0; i < getWidth(); i++) {
      stream << indent(offset) <<  "Arg "<< i << ":\n";
      ozd_printLongStream(args[i],stream,PRINT_DEPTH_DEC(depth),offset);
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
  ozd_printStream(label,stream,depth);
  stream << endl;

  stream << indent(offset) << "Args:\n";
  TaggedRef ar = getArityList();
  CHECK_DEREF(ar);
  while (isCons(ar)) {
    stream << indent(offset+2);
    TaggedRef feat = head(ar);
    CHECK_DEREF(feat);
    ozd_printStream(feat,stream);
    ar = tail(ar);
    CHECK_DEREF(ar);
    stream << ": ";
    ozd_printLongStream(getFeature(feat),stream,
                        PRINT_DEPTH_DEC(depth),offset+2);
  }
  stream << indent(offset) << "End of Args\n";
}


void Float::printLongStream(ostream &stream, int depth, int offset)
{
  stream << indent(offset) << "Float @" << this << ": ";
  this->printStream(stream);
  stream << endl;
}

void BigInt::printStream(ostream &stream, int depth)
{
  stream << "<BigInt @" << this << ": " << toC(makeTaggedBigInt(this)) << ">";
}


void ThreadQueueImpl::printStream(ostream &stream, int depth)
{
  if (isEmpty()) {
    stream << "Thread queue empty.\n";
  } else {
    stream << "Thread #" << size << endl << flush;

    for (int aux_size = size, aux_head = head;
         aux_size;
         aux_head = (aux_head + 1) & (maxsize - 1), aux_size--) {
      stream << "queue[" << aux_head << "]=" << flush;
      queue[aux_head]->printStream(stream,depth);
    }
  }
}

void FDIntervals::printLong(ostream &stream, int idnt) const
{
  stream << endl << indent(idnt) << "high=" << endl;
  print(stream, idnt);
  for (int i = 0; i < high; i += 1)
    stream << endl << indent(idnt)
          << "i_arr[" << i << "]@" << (const void*) &i_arr[i]
          << " left=" << i_arr[i].left << " right=" << i_arr[i].right;
  stream << endl;
}

void FDBitVector::printLong(ostream &stream, int idnt) const
{
  stream << "  high=" << high << endl;
  print(stream, idnt);
  for (int i = 0; i < high; i++) {
    stream << endl << indent(idnt + 2) << '[' << i << "]:  ";
    for (int j = 31; j >= 0; j--) {
      stream << ((b_arr[i] & (1 << j)) ? '1' : 'o');
      if (j % 8 == 0) stream << ' ';
    }
  }
  stream << endl;
}

void OZ_FiniteDomainImpl::printLong(ostream &stream, int idnt) const
{
  static char * descr_type_text[3] = {"bv_descr", "iv_descr", "fd_descr"};

  stream << indent(idnt) << "min_elem=" << min_elem
        << " max_elem=" << max_elem << " size=" << getSize()
        << " type=" << descr_type_text[getType()];

  switch (getType()) {
  case fd_descr:
    stream << endl;
    print(stream, idnt);
    stream << endl;
    break;
  case bv_descr:
    get_bv()->printLong(stream, idnt);
    break;
  case iv_descr:
    get_iv()->printLong(stream, idnt);
    break;
  default:
    error("unexpected case");
  }
}

// ----------------------------------------------------

// mm2!
void Board::printTree()
{
  Board *bb = this;
  Actor *aa;
  int off=0;
  while (!am.isRootBoard(bb)) {
    cout << indent(off);
    bb->printStream(cout,1);
    cout << endl;
    Assert(!bb->isCommitted());
    off++;
    aa = bb->u.actor;
    cout << indent(off);
    aa->printStream(cout,1);
    cout << endl;
    off++;
    bb = GETBOARD(aa);
  }
  cout << indent(off);
  bb->printStream(cout,1);
  cout << endl;
}

void LocalPropagationQueue::printStream(ostream &stream, int depth)
{
  int psize = size, phead = head;

  for (; psize; psize --) {
    stream << "lpqueue[" << phead << "]="
         << "@" << queue[phead].thr << endl;

    stream << "(" << endl;
    queue[phead].thr->printStream(stream,depth);
    stream << ")" << endl;

    phead = (phead + 1) & (maxsize - 1);
  }
}


// -----------------------------------------------------------------------
// Debug.print Builtins
// -----------------------------------------------------------------------

OZ_C_proc_begin(BIdebugPrint,2)
{
  oz_declareArg(0,t);
  oz_declareIntArg(1,depth);
  ozd_printStream(t,cerr,depth);
  cerr << endl;
  flush(cerr);
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIdebugPrintLong,2)
{
  oz_declareArg(0,t);
  oz_declareIntArg(1,depth);
  ozd_printLongStream(t,cerr,depth);
  return PROCEED;
}
OZ_C_proc_end

#endif
