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

static const double GDT_MAXFULL = 1.0;
static const int GDT_IDEALFULL  = 2;
static const int GDT_MINFULL    = 4;
static const int minSize                = 128;
static const int minBits                = 7; // 2^7 = 128;

//
template<class NODE>
void GenDistEntryTable<NODE>::htAdd(NODE *newNode)
{
  NODE **np;

  if (counter > percent)
    resize();

  //
  np = getFirstNodeRef(hash(newNode->value4hash()));
  counter++;                    // going to insert anyway;

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
  for(int i = tableSize; i--; )
    table[i] = (NODE *) 0;
}

//
template<class NODE>
void GenDistEntryTable<NODE>::init(int sizeIn)
{
  tableSize = minSize;
  bits = minBits;
  while (tableSize < sizeIn) {
    tableSize = tableSize * 2;
    bits++;
  }
  table = new NODE*[tableSize];
  mkEmpty();
}

//
template<class NODE>
void GenDistEntryTable<NODE>::resize()
{
  int oldSize = tableSize;
  NODE** old = table;

  DebugCode(checkConsistency(););
  tableSize = tableSize * 2;
  bits++;
  table = new NODE*[tableSize];
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
  int idealSize = counter * GDT_IDEALFULL;
  int oldSize;
  NODE** old;
  DebugCode(int oldCounter;);

  DebugCode(checkConsistency(););
  if (counter >= tableSize / GDT_MINFULL || minSize >= idealSize)
    return;

  //
  oldSize = tableSize;
  DebugCode(oldCounter = counter;);
  old = table;
  // construct the table anew;
  init(idealSize);

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
