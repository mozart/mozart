/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

//
// GenDistEntryTable's outline code.
// Must be instantiated explicitly;
// 

static const double GDT_MAXFULL	= 1.0;
static const int GDT_IDEALFULL	= 1; // logarithmic;
static const int GDT_MINFULL	= 2; // .. ditto.

//
template<class NODE>
void GenDistEntryTable<NODE>::htAdd(NODE *newNode)
{
  NODE **np;

  if (counter > percent)
    resize();

  //
  np = getFirstNodeRef(hash(newNode->value4hash()));
  counter++;			// going to insert anyway;

  //
  while (NODE *n = *np) {
    if (newNode->compare(n) <= 0) {
      // not greater than the rest - insert and bail out;
      newNode->setNext(n);
      *np = newNode;
      return;
    } else {
      np = n->getNextNodeRef();
    }
  }
  // 'np' still points to the last 'next' pointer cell;

  // either empty bucket or the newNode is larger than the rest;
  newNode->setNext((NODE *) 0);
  *np = newNode;
}

//
template<class NODE>
NODE* GenDistEntryTable<NODE>::htFind(NODE *eqNode)
{
  NODE **np = getFirstNodeRef(hash(eqNode->value4hash()));

  //
  while (NODE *n = *np) {
    if (eqNode->compare(n) == 0)
      return (n);
    else
      np = n->getNextNodeRef();
  }
  return ((NODE *) 0);
}

//
template<class NODE>
void GenDistEntryTable<NODE>::htDel(NODE *eqNode)
{
  NODE **np = getFirstNodeRef(hash(eqNode->value4hash()));

  //
  while (NODE *n = *np) {
    if (eqNode->compare(n) == 0) {
      *np = n->getNext();
      counter--;
      DebugCode(n->setNext((NODE *) -1););
      return;
    } else {
      np = n->getNextNodeRef();
    }
  }
}

//
//
template<class NODE>
void GenDistEntryTable<NODE>::mkEmpty()
{
  const int totalBits = sizeof(unsigned int)*8;
  rsBits = totalBits - bits;
  counter = 0;
  percent = (int) (GDT_MAXFULL * tableSize);
  table = new NODE*[tableSize];
  for (int i = tableSize; i--; )
    table[i] = (NODE *) 0;
}

//
template<class NODE>
void GenDistEntryTable<NODE>::init(int sizeAsPowerOf2)
{
  Assert(sizeAsPowerOf2 < sizeof(int)*8);
  // can even swallow "2^0" hash tables... if one really insists on :-)
  tableSize = 1 << sizeAsPowerOf2;
  bits = sizeAsPowerOf2;
  mkEmpty();
}

//
template<class NODE>
void GenDistEntryTable<NODE>::resize()
{
  int oldSize = tableSize;
  NODE** old = table;

  DebugCode(checkConsistency(););
  tableSize = tableSize << 1;
  bits++; 
  mkEmpty();

  //
  for (int i = oldSize; i--; ) {
    NODE *n = old[i];
    while (n != (NODE *) 0) {
      NODE *nn = n->getNext();
      htAdd(n);
      n = nn;
    }
  }

  //
  delete [] old;
  DebugCode(checkConsistency(););
}

//
template<class NODE>
void GenDistEntryTable<NODE>::compactify()
{
  // This constraint disallows 'compactify()' to attempt to compactify
  // an already compactified table;
  Assert(GDT_MINFULL > GDT_IDEALFULL);
  int idealSize = counter << GDT_IDEALFULL; // maybe zero;
  int oldSize, newSize;
  NODE** old;
  DebugCode(int oldCounter;);

  DebugCode(checkConsistency(););
  if (counter >= (tableSize >> GDT_MINFULL))
    return;

  //
  oldSize = tableSize;
  DebugCode(oldCounter = counter;);
  old = table;

  // construct the table anew;
  newSize = log2ceiling(idealSize);
  init(newSize);

  //
  for (int i = oldSize; i--; ) {
    NODE *n = old[i];
    while (n != (NODE *) 0) {
      NODE *nn = n->getNext();
      htAdd(n);
      n = nn;
    }
  }
  Assert(counter == oldCounter);

  //
  delete [] old;
  DebugCode(checkConsistency(););
}

//
#if defined(DEBUG_CHECK)
template<class NODE>
void GenDistEntryTable<NODE>::checkConsistency()
{
  for (int i = getSize(); i--; ) {
    NODE *n = getFirstNode(i);
    while (n)
      n = n->getNext();
  }
}
#endif
