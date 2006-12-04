/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
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
#include "susp_queue.hh"
#include "sort.hh"

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
  return sl->getSuspendable()->isDead() ? NO : OK;
}

// returns OK if sl contains at least one alive suspension element
inline Bool isEffectiveList(SuspList* sl) {
  for (; sl != NULL; sl = sl->getNext())
    if (isEffectiveSusp(sl))
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

  switch(tagged2ltag(ref)) {
  case LTAG_VAR0:
  case LTAG_VAR1:
    stream << oz_varGetName(val);
    tagged2Var(ref)->printStream(stream, depth);
    break;
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    tagged2SRecord(ref)->printStream(stream,depth);
    break;
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
    tagged2LTuple(ref)->printStream(stream,depth);
    break;
  case LTAG_LITERAL:
    tagged2Literal(ref)->printStream(stream,depth);
    break;
  case LTAG_SMALLINT:
    stream << "<SmallInt @" << &ref << ": " << toC(ref) << ">";
    break;
  case LTAG_CONST0:
  case LTAG_CONST1:
    tagged2Const(ref)->printStream(stream,depth);
    break;
  default:
    stream << "<unknown tag " << (int) tagged2ltag(ref) << ">";
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

  switch(tagged2ltag(ref)) {
  case LTAG_VAR0:
  case LTAG_VAR1:
    stream << indent(offset) << oz_varGetName(val);
    tagged2Var(ref)->printLongStream(stream, depth, offset);
    break;
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    tagged2SRecord(ref)->printLongStream(stream,depth,offset);
    break;
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
    tagged2LTuple(ref)->printLongStream(stream,depth,offset);
    break;
  case LTAG_LITERAL:
    tagged2Literal(ref)->printLongStream(stream,depth,offset);
    break;
  case LTAG_SMALLINT:
    stream << indent(offset);
    ozd_printStream(val,stream,depth);
    stream << endl;
    break;
  case LTAG_CONST0:
  case LTAG_CONST1:
    tagged2Const(ref)->printLongStream(stream,depth,offset);
    break;
  default:
    stream << "unknown tag " << (int) tagged2ltag(ref) << endl;
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
    var2ExtVar(this)->printStreamV(stream,depth);
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
    var2ExtVar(this)->printLongStreamV(stream,depth,offset);
    break;

  default:
    stream << indent(offset) << " unknown type: " << (int) getType() << endl;
    break;
  }

} // printLongStream(OzVariable)

class Order_Taggedref_By_Feat {
public:
  Bool operator()(const TaggedRef& a, const TaggedRef& b) {
    return featureCmp(a,b) < 0;
  }
};

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
    Order_Taggedref_By_Feat lt;
    fastsort(arr, nAtomOrInt, lt);
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
      sl->getSuspendable()->printStream(stream);
      stream << endl;
    }
  } // for
}

void ConstTerm::printLongStream(ostream &stream, int depth, int offset)
{
  switch (getType()) {
  case Co_Float:
    ((Float *) this)->printLongStream(stream,depth,offset);
    break;
  case Co_Extension:
    {
      int n;
      char * s = OZ_virtualStringToC(const2Extension(this)->printV(depth),&n);
      stream << s;
      break;
    }
  case Co_BigInt:
    ((BigInt *) this)->printLongStream(stream, depth, offset);
    break;
  case Co_FSetValue:
    ((FSetValue *) (((ConstFSetValue *) this)->getValue()))->print(stream,depth,offset);
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

  default: 	      Assert(NO);
  }
}

void ConstTerm::printStream(ostream &stream, int depth)
{
  switch (getType()) {
  case Co_Float:
    ((Float *) this)->printStream(stream,depth);
    break;
  case Co_Extension:
    {
      int n;
      char * s = OZ_virtualStringToC(const2Extension(this)->printV(depth),&n);
      stream << s;
      break;
    }
  case Co_BigInt:      ((BigInt *) this)->printStream(stream, depth);
    break;
  case Co_FSetValue:
    ((FSetValue *) (((ConstFSetValue *) this)->getValue()))->print(stream,depth);
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

  if (isRoot()) 
    return;

  stream << indent(offset) << "Board:" << endl;
  getParentInternal()->printLongStream(stream,PRINT_DEPTH_DEC(depth),offset+2);
  stream << "Local propagator queue: " << &lpq << endl;
}

void ThreadsPool::printThreads() {
  cout << "Threads" << endl;

  if (!_q[ HI_PRIORITY].isEmpty()) {
    cout << "   prio = HI";
    _q[ HI_PRIORITY].print();
  }

  if (!_q[MID_PRIORITY].isEmpty()) {
    cout << "   prio = MID";
    _q[MID_PRIORITY].print();
  }

  if (!_q[LOW_PRIORITY].isEmpty()) {
    cout << "   prio = LOW";
    _q[LOW_PRIORITY].print();
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

  if (isDead()) {
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

  stream << " Tasks #" << taskStack->tasks();

  if (isExternal()) stream << " external";
  if (isTagged())   stream << " tagged";

  stream << " <";
  GETBOARD(this)->printStream(stream, PRINT_DEPTH_DEC(depth));
  stream << ">";
}

void Thread::printLongStream(ostream &stream, int depth, int offset)
{
  this->printStream(stream,depth);
  stream << endl;
  taskStack->printTaskStack(depth); //mm2: prints to stderr!
}

void Propagator::printStream(ostream &stream, int depth)
{
  stream << "Propagator: " << getPropagator()->toString();  
}

void Propagator::printLongStream(ostream &stream, int depth, int)
{
  printStream(stream, depth);
}

void Suspendable::printStream(ostream &stream, int depth)
{
  if (isThread()) {
    SuspToThread(this)->printStream(stream, depth);
  } else {
    SuspToPropagator(this)->printStream(stream, depth);
  }    
}

void Suspendable::printLongStream(ostream &stream, int depth, int offset)
{
  if (isThread()) {
    SuspToThread(this)->printLongStream(stream, depth, offset);
  } else {
    Assert(isPropagator());
    SuspToPropagator(this)->printLongStream(stream, depth, offset);
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
  switch (getKind()) {
  case K_XReg:
    c = 'X';
    break;
  case K_YReg:
    c = 'Y';
    break;
  case K_GReg:
  default:
    c = 'G';
    break;
  }
  stream << c << getIndex();
}

void PrTabEntry::printStream(ostream &stream, int depth) {
  stream << "<PrTabEntry: " << getPrintName()
	 << "/" << (int) p.arity <<  "PC: " << (void *) PC
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
  while (oz_isCons(ar)) {
    stream << indent(offset+2);
    TaggedRef feat = oz_head(ar);
    ozd_printStream(feat,stream);
    ar = oz_tail(ar);
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

void SuspStack::printStream(ostream &stream, int depth)
{
  if (isEmpty()) {
    stream << "Suspendable stack empty.\n";
  } else {
    stream << "Suspendable stack #" << getSize() << endl << flush;

    int i = 0;

    for (SuspList * sl = _head; sl != (SuspList *) NULL; 
	 sl = sl->getNext(), i+= 1) 
      {
	stream << "stack[" << i++ << "]=" << flush;
	sl->printStream(stream,depth);
      }
  }
}

void SuspQueue::printStream(ostream &stream, int depth)
{
  if (isEmpty()) {
    stream << "Suspendable queue empty.\n";
  } else {
    stream << "Suspendable queue #" << getSize() << endl << flush;

    int i = 0;

    for (SuspList * sl = last->getNext(); sl != last; sl = sl->getNext()) {
      stream << "queue[" << i++ << "]=" << flush;
      sl->printStream(stream,depth);
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
    OZD_error("unexpected case");
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
