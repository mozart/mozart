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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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

#include "base.hh"

#ifdef DEBUG_PRINT

#include "var_base.hh"
#include "fdomn.hh"
#include "dictionary.hh"
#include "builtins.hh"
#include "var_fd.hh"
#include "var_fs.hh"
#include "var_ct.hh"
#include "var_of.hh"
#include "var_ext.hh"
#include "lps.hh"

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
  Suspension susp = sl->getSuspension();
  if (susp.isDead())
    return NO;
  if (!GETBOARDOBJ(susp))
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

  OZ_Term ref=oz_deref(val);

  switch(tagTypeOf(ref)) {
  case UVAR:
    stream << oz_varGetName(val);
    stream << "<UV @" << &ref << ">";
    break;

    //FUT

  case CVAR:
    stream << oz_varGetName(val);
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
  case SMALLINT:
    stream << "<SmallInt @" << &ref << ": " << toC(ref) << ">";
    break;
  case EXT:
    {
      int n;
      char * s = OZ_virtualStringToC(oz_tagged2Extension(ref)->printV(depth),
                                     &n);
      stream << s;
      break;
    }
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
  if (oz_isRef(ref)) {
    stream << indent(offset) << "Reference chain: ";
    while (oz_isRef(ref)) {
      stream << '@'
             << (void *) tagged2Ref(ref);
      ref = *tagged2Ref(ref);
    }
    stream << endl;
  }

  switch(tagTypeOf(ref)) {
  case UVAR:
    stream << indent(offset) << oz_varGetName(val);
    stream << indent(offset) << "<UV @" << &ref << ">" << endl;
    stream << indent(offset) << "Home: ";
    tagged2VarHome(ref)->derefBoard()
      ->printStream(stream,PRINT_DEPTH_DEC(depth));
    stream << endl;
    break;

    // FUT

  case CVAR:
    stream << indent(offset) << oz_varGetName(val);
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
  case SMALLINT:
    stream << indent(offset);
    ozd_printStream(val,stream,depth);
    stream << endl;
    break;
  case EXT:
    {
      int n;
      char* s = OZ_virtualStringToC(oz_tagged2Extension(ref)->printLongV(depth,offset),&n);
      stream << s;
      break;
    }
  case OZCONST:
    tagged2Const(ref)->printLongStream(stream,depth,offset);
    break;
  case FSETVALUE:
    ((FSetValue *) tagged2FSetValue(ref))
      ->print(stream,depth);
    break;
  default:
    stream << "unknown tag " << (int) tagTypeOf(ref) << endl;
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

void OzVariable::printStream(ostream &stream, int depth)
{
  stream << "<Var @" << this;
  if (isEffectiveList(suspList))
    stream << " a" << suspList->length();

  switch(getType()){
  case OZ_VAR_FD:
    {
      OzFDVariable * me = (OzFDVariable *) this;
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

  case OZ_VAR_BOOL:
    {
      stream << " {0 1}";
      break;
    }

  case OZ_VAR_FS:
    {
      OzFSVariable * me = (OzFSVariable *) this;
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

  case OZ_VAR_OF:
    {
      stream << ' ';
      OzOFVariable* me = (OzOFVariable *) this;
      stream << " ";
      ozd_printStream(me->getLabel(),stream,depth);
      me->getTable()->printStream(stream,PRINT_DEPTH_DEC(depth));
      break;
   }

  case OZ_VAR_CT:
    {
      OzCtVariable * me = (OzCtVariable *) this;

      for (int i = 0; i < me->getNoOfSuspLists(); i += 1) {
        SuspList * sl = me->_susp_lists[i];
        if (isEffectiveList(sl))
          stream << "sl[" << i << "]("
                 << sl->length()
                 << '/'
                 << sl->lengthProp()
                 << ')';
      }
      stream << ' ' <<  me->getConstraint()->toString(PRINT_DEPTH_DEC(depth));
      break;
    }

  case OZ_VAR_EXT:
    ((ExtVar*)this)->printStreamV(stream,depth);
    break;

  default:
    stream << " unknown type: " << (int) getType();
    break;
  }
  stream << ">";
} // PRINTSTREAM(OzVariable)

void OzVariable::printLongStream(ostream &stream, int depth, int offset)
{
  this->printStream(stream);

  stream << endl
         << indent(offset) << "Home: ";
  GETBOARD(this)->printStream(stream,depth);
  stream << endl;

  stream << indent(offset) << "Suspension List:\n";
  suspList->printLongStream(stream, depth, offset+3);

  switch(getType()){
  case OZ_VAR_FD:
    stream << indent(offset) << "FD Singleton SuspList:\n";
    ((OzFDVariable*)this)->fdSuspList[fd_prop_singl]
      ->printLongStream(stream, depth, offset+3);
      stream << indent(offset) << "FD Bounds SuspList:\n";
    ((OzFDVariable*)this)->fdSuspList[fd_prop_bounds]
      ->printLongStream(stream, depth, offset+3);
    stream << indent(offset) << "FD Domain:\n";
    ((OZ_FiniteDomainImpl *) &((OzFDVariable*)this)->getDom())
      ->printLong(stream, offset+3);
    break;

  case OZ_VAR_BOOL:
    stream << indent(offset) << "Boolean Domain: {0 1}" << endl;
    break;

  case OZ_VAR_FS:
    stream << indent(offset) << "FSet val SuspList:\n";
    ((OzFSVariable*)this)->fsSuspList[fs_prop_val]
      ->printLongStream(stream, depth, offset+3);
    stream << indent(offset) << "FSet glb SuspList:\n";
    ((OzFSVariable*)this)->fsSuspList[fs_prop_glb]
      ->printLongStream(stream, depth, offset+3);
    stream << indent(offset) << "FSet lub SuspList:\n";
    ((OzFSVariable*)this)->fsSuspList[fs_prop_lub]
      ->printLongStream(stream, depth, offset+3);
    stream << indent(offset) << "FSet :\n" << indent(offset + 3);
    ((FSetConstraint *) &((OzFSVariable*)this)->getSet())
      ->print(stream);
    stream << endl;
    break;

  case OZ_VAR_OF:
    {
      stream << indent(offset);
      OzOFVariable* me = (OzOFVariable *) this;
      ozd_printStream(me->getLabel(),stream,PRINT_DEPTH_DEC(depth));
      // me->getLabel()->printStream(stream, PRINT_DEPTH_DEC(depth));
      me->getTable()->printStream(stream, PRINT_DEPTH_DEC(depth));
      stream << endl;
      break;
    }

  case OZ_VAR_CT:
    {
      OzCtVariable * me = (OzCtVariable *) this;

      for (int i = 0; i < me->getNoOfSuspLists(); i += 1) {
        SuspList * sl = me->_susp_lists[i];
        stream << indent(offset) << "Ct Var SuspList[" << i << "]:\n";
        sl->printLongStream(stream, depth, offset+3);
      }
      stream << ' ' <<  me->getConstraint()->toString(PRINT_DEPTH_DEC(depth));
      break;
    }

  case OZ_VAR_EXT:
    ((ExtVar*)this)->printLongStreamV(stream,depth,offset);
    break;

  default:
    stream << indent(offset) << " unknown type: " << (int) getType() << endl;
    break;
  }

} // printLongStream(OzVariable)


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
            if (oz_isAtom(tmplit)||oz_isInt(tmplit)) nAtomOrInt++; else nName++;
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
        if (tmpval!=makeTaggedNULL() && (oz_isAtom(tmplit)||oz_isInt(tmplit)))
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
        if (tmpval!=makeTaggedNULL() && !(oz_isAtom(tmplit)||oz_isInt(tmplit))) {
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
    Assert(oz_isCons(as));
    while (oz_isCons(as)) {
      stream << ' ';
      ozd_printStream(oz_head(as), stream, PRINT_DEPTH_DEC(depth));
      stream << ": ";
      ozd_printStream(getFeature(oz_head(as)), stream, PRINT_DEPTH_DEC(depth));
      as = oz_tail(as);
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

void Builtin::printStream(ostream &stream, int depth)
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
      sl->getSuspension().printStream(stream);
      stream << endl;
    }
  } // for
}

void ConstTerm::printLongStream(ostream &stream, int depth, int offset)
{
  switch (getType()) {
  case Co_BigInt:
    ((BigInt *) this)->printLongStream(stream, depth, offset);
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
      stream << "CellLocal@" << this;
      break;
    case Te_Frame:
      stream << "CellFrame@" << this;
      break;
    case Te_Manager:
      stream << "CellManager@" << this;
      break;
    case Te_Proxy:
      stream << "CellProxy@" << this;
      break;
    default:
      Assert(NO);
    }
    break;
  case Co_Port:
    switch(((Tertiary *)this)->getTertType()){
    case Te_Local:
      stream << "PortLocal@" << this;
      break;
    case Te_Manager:
      stream << "PortManager@" << this;
      break;
    case Te_Proxy:
      stream << "PortProxy@" << this;
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
      stream << "<LockLocal@" << this << ">";
      break;
    case Te_Frame:
      stream << "<LockFrame@" << this << ">";
      break;
    case Te_Manager:
      stream << "<LockManager@" << this << ">";
      break;
    case Te_Proxy:
      stream << "<LockProxy@" << this << ">";
      break;
    default:         Assert(NO);
    }
    break;
  case Co_Builtin:
    ((Builtin *) this)->printLongStream(stream,depth,offset);
    break;
  case Co_Foreign_Pointer:
    ((ForeignPointer*)this)->printLongStream(stream,depth,offset); break;

  default:            Assert(NO);
  }
}

void ConstTerm::printStream(ostream &stream, int depth)
{
  switch (getType()) {
  case Co_BigInt:      ((BigInt *) this)->printStream(stream, depth);
    break;
  case Co_Abstraction: ((Abstraction *) this)->printStream(stream,depth);
    break;
  case Co_Object:      ((Object *) this)->printStream(stream,depth);
    break;
  case Co_Class:       ((ObjectClass *) this)->printStream(stream,depth);
    break;
  case Co_Cell:
    switch(((Tertiary *)this)->getTertType()){
    case Te_Local:
      stream << "CellLocal@" << this;
      break;
    case Te_Frame:
      stream << "CellFrame@" << this;
      break;
    case Te_Manager:
      stream << "CellManager@" << this;
      break;
    case Te_Proxy:
      stream << "CellProxy@" << this;
      break;
    default:
      Assert(NO);
    }
    break;
  case Co_Port:
    switch(((Tertiary *)this)->getTertType()){
    case Te_Local:
      stream << "PortLocal@" << this;
      break;
    case Te_Manager:
      stream << "PortManager@" << this;
      break;
    case Te_Proxy:
      stream << "PortProxy@" << this;
      break;
    default:
      Assert(NO);
    }
    break;
  case Co_Space:       ((Space *) this)->printStream(stream,depth);
    break;
  case Co_Chunk:       ((SChunk *) this)->printStream(stream,depth);
    break;
  case Co_Array:       ((OzArray *) this)->printStream(stream,depth);
    break;
  case Co_Dictionary:  ((OzDictionary *) this)->printStream(stream,depth);
    break;
  case Co_Lock:
    switch(((Tertiary *)this)->getTertType()){
    case Te_Local:
      stream << "<LockLocal@" << this << ">";
      break;
    case Te_Frame:
      stream << "<LockFrame@" << this << ">";
      break;
    case Te_Manager:
      stream << "<LockManager@" << this << ">";
      break;
    case Te_Proxy:
      stream << "<LockProxy@" << this << ">";
      break;
    default:         Assert(NO);
    }
    break;
  case Co_Builtin:     ((Builtin *) this)->printStream(stream,depth);
    break;
  case Co_Foreign_Pointer:
    ((ForeignPointer*)this)->printStream(stream,depth); break;
  default:             Assert(NO);
  }
}

void ForeignPointer::printStream(ostream &stream, int depth)
{
  stream << "foreign pointer: " << getPointer() << " at " << this << '.';
}

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

  if (isRoot()) {
    stream << "Root Space";
  } else {
    stream << "Space";
  }

  stream << "Board @" << this << " [";

  if (isInstalled())    stream << " Installed";
  if (isFailed())       stream << " Failed";
  if (isCommitted())    stream << " Committed";

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

  stream << indent(offset) << "Script: " << endl;
  script.printLongStream(stream,PRINT_DEPTH_DEC(depth),offset+2);

  if (isRoot())
    return;

  stream << indent(offset) << "Board:" << endl;
  getParentInternal()->printLongStream(stream,PRINT_DEPTH_DEC(depth),offset+2);
  stream << "Local propagator queue: " << localPropagatorQueue << endl;
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

void ThreadsPool::printThreads()
{
  cout << "Threads" << endl
       << "  running: ";
  _currentThread->printStream(cout,-1);
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

#ifdef LINKED_QUEUES
void ThreadQueue::printThreads()
{
  cout << " #" << size << " threads" << endl;
  ThreadQueueIterator iter(this);
  Thread*ptr;
  while ((ptr=iter.getNext())) {
    ptr->printStream(cout,-1);
    if (ptr==oz_currentThread())
      cout << " RUNNING";
    cout << endl;
  }
}
#else
void ThreadQueue::printThreads()
{
  int i = getSize();
  cout << " #" << i << " threads" << endl;

  for (; i; i--) {
    Thread *th = dequeue();
    th->printStream(cout,-1);
    if (th == oz_currentThread())
      cout << " RUNNING ";
    cout << endl;
    enqueue(th);
  }
}
#endif /* !LINKED_QUEUES */

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
  am.threadsPool.printThreads();
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

  default:
    stream << "(unknown type " << getThrType() << ")";
  }

  if ((getFlags ()) & T_ext)       stream << " ext";
  if ((getFlags ()) & T_tag)       stream << " tag";
  if ((getFlags ()) & T_lpq)       stream << " lpq";
  stream << " <";
  GETBOARD(this)->printStream(stream, PRINT_DEPTH_DEC(depth));
  stream << ">";
}

void Thread::printLongStream(ostream &stream, int depth, int offset)
{
  this->printStream(stream,depth);
  stream << endl;
  if (hasStack())
    item.threadBody->taskStack.printTaskStack(depth); //mm2: prints to stderr!
}

void Propagator::printStream(ostream &stream, int depth)
{
  stream << "Propagator: " << getPropagator()->toString();
}

void Propagator::printLongStream(ostream &stream, int depth, int)
{
  printStream(stream, depth);
}

void Suspension::printStream(ostream &stream, int depth)
{
  if (isThread()) {
    getThread()->printStream(stream, depth);
  } else {
    Assert(isPropagator());
    getPropagator()->printStream(stream, depth);
  }
}

void Suspension::printLongStream(ostream &stream, int depth, int offset)
{
  if (isThread()) {
    getThread()->printLongStream(stream, depth, offset);
  } else {
    Assert(isPropagator());
    getPropagator()->printLongStream(stream, depth, offset);
  }
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
  int n = getPred()->getGSize();
  if (offset == 0) {
    if (n > 0) {
      stream <<  "G Regs:";
      for (int i = 0; i < n; i++) {
        stream << " G[" << i << "]:\n";
        ozd_printLongStream(getG(i),stream,depth,offset+2);
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

void Builtin::printLongStream(ostream &stream, int depth, int offset)
{
  printStream(stream,depth);
  stream << endl;
  stream << indent(offset) << "gRegs: -" << endl;
}



void Space::printLongStream(ostream &stream, int depth, int offset)
{
  stream << indent(offset)
         << "Space@id" << this << endl
         << indent(offset)
         << " Board:"<<endl;
  solve->printLongStream(stream,depth,offset+2);
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
  GETBOARD(this)->printLongStream(stream,depth,offset);
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
  while (oz_isCons(ar)) {
    stream << indent(offset+2);
    TaggedRef feat = oz_head(ar);
    CHECK_DEREF(feat);
    ozd_printStream(feat,stream);
    ar = oz_tail(ar);
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
  stream << "<BigInt @" << this << ": " << toC(makeTaggedConst(this)) << ">";
}

#ifdef LINKED_QUEUES
void ThreadQueue::printStream(ostream &stream, int depth)
{
  if (isEmpty()) {
    stream << "Thread queue empty." << endl << flush;
  } else {
    stream << "Thread #" << size << endl << flush;
    int i = 0;
    Thread*ptr;
    ThreadQueueIterator iter(this);
    while ((ptr=iter.getNext())) {
      stream << "queue[" << i++ << "]=" << flush;
      ptr->printStream(stream,depth);
    }
  }
}
#else
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
#endif /* !LINKED_QUEUES */

void FDIntervals::printLong(ostream &stream, int idnt) const
{
  stream << endl << indent(idnt) << "high=" << endl;
  print(stream, idnt);
  for (int i = 0; i < high; i += 1) {
    stream << endl << indent(idnt)
           << "i_arr[" << i << "]@"
#if !defined(DEBUG_CHECK) || !defined(DEBUG_FD)
           << (const void*) &i_arr[i]
#endif
           << " left=" << i_arr[i].left << " right=" << i_arr[i].right;
  }
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
    OZ_error("unexpected case");
  }
}

// ----------------------------------------------------

#ifdef DEBUG_CHECK

void Board::printTree()
{
  Board *bb = this;

  int off=0;

  while (!oz_isRootBoard(bb)) {
    cout << indent(off);
    bb->printStream(cout,1);
    cout << endl;
    Assert(!bb->isCommitted());
    off++;
    cout << endl;
    off++;
    bb = bb->getParent();
  }
  cout << indent(off);
  bb->printStream(cout,1);
  cout << endl;
}

#endif

void LocalPropagationQueue::printStream(ostream &stream, int depth)
{
  int psize = size, phead = head;

  for (; psize; psize --) {
    stream << "lpqueue[" << phead << "]="
         << "@" << queue[phead].prop << endl;

    stream << "(" << endl;
    queue[phead].prop->printStream(stream,depth);
    stream << ")" << endl;

    phead = (phead + 1) & (maxsize - 1);
  }
}

void OrderedSuspList::printStream(ostream &stream, int depth)
{
  for (OrderedSuspList * p = this; p != NULL; p = p->getNext()) {
    OZ_Propagator * pr = p->_p->getPropagator();
    stream << "   " << pr->toString();
  }
}

// -----------------------------------------------------------------------
// Debug.print Builtins
// -----------------------------------------------------------------------

OZ_BI_define(BIdebugPrint,2,0)
{
  oz_declareIN(0,t);
  oz_declareIntIN(1,depth);
  ozd_printStream(t,cerr,depth);
  cerr << endl;
  flush(cerr);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIdebugPrintLong,2,0)
{
  oz_declareIN(0,t);
  oz_declareIntIN(1,depth);
  ozd_printLongStream(t,cerr,depth);
  return PROCEED;
} OZ_BI_end

#endif
