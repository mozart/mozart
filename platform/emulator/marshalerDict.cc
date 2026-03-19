/*
 *  Authors:
 *    Kostja Popov <kost@sics.se>
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Konstantin Popov, 2001
 * 
 *  Last change:
 *    $Date$
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

#if defined(INTERFACE)
#pragma implementation "marshalerDict.cc"
#endif

#include "base.hh"
#include "marshalerDict.hh"
#if defined(DEBUG_CHECK)
#include "cac.hh"
#endif


//
const double AHTFR_MAXLOAD = 0.5;
const int TABLE_SIZE = 128;
const int TABLE_SIZE_BITS = 7;	// TABLE_SIZE == 2^TABLE_SIZE_BITS;

// print statistics if on average there are more than that tries per search:
#define DEBUG_THRESHOLD		2

// golden ratio hash constant (pointer-width)
#if __SIZEOF_POINTER__ == 8
#define GOLDEN_HASH_CONST ((ozword) 0x9e3779b97f4a7c15ULL)
#else
#define GOLDEN_HASH_CONST ((ozword) 0x9e3779b9)
#endif

//
// Note: 'mkTable()' resets the 'pass';
void MarshalerDict::mkTable()
{
  const int totalBits = sizeof(ozword)*8;
  // leave exactly 'bits':
  rsBits = totalBits - bits;
  // would like to drop all of the pkey's bits, but have to retain at
  // least 'bits':
  slsBits = min(bits, rsBits);

  //
  counter = 0;
  percent = (int) (AHTFR_MAXLOAD * tableSize);
  table = new MarshalerDict_Node[tableSize];
  // MarshalerDict_Node"s are initialized with cnt = 0, so make them unused:
  pass = 1;
  lastIndex = -1;
  DebugCode(lastKey = -1);
  DebugCode(nsearch = 0;);
  DebugCode(tries = 0;);
  DebugCode(maxtries = 0;);
  DebugCode(nsearchAcc = 0;);
  DebugCode(triesAcc = 0;);
}

//
void MarshalerDict::mkEmpty()
{
  DebugCode(nsearchAcc += nsearch;);
  DebugCode(triesAcc += tries;);
  // DebugCode(printStatistics(DEBUG_THRESHOLD));
  pass++;
  if (pass == (unsigned int) 0xffffffff) {
    pass = 1;
    for (int i = tableSize; i--; )
      table[i].reset();
  }
  counter = 0;
  index = 1;			// '0' is reserved;
  DebugCode(nsearch = 0;);
  DebugCode(tries = 0;);
  DebugCode(maxtries = 0;);
}

//
MarshalerDict::MarshalerDict(int sz)
{
  tableSize = TABLE_SIZE;
  bits = TABLE_SIZE_BITS;
  while (tableSize < sz) {
    tableSize = tableSize * 2;
    bits++;
  }
  index = 1;			// '0' is reserved;
  mkTable();
}

MarshalerDict::~MarshalerDict() 
{
  delete [] table;
}

// These functions are not really used - just for debugging;
inline
unsigned int MarshalerDict::primeHashFunc(OZ_Term i)
{
  // golden cut = 0.6180339887 = A/w
  // 32bit: w = 4294967296, A = 2654435769
  // 64bit: w = 18446744073709551616, A = 11400714819323198485
  return ((((ozword) i) * GOLDEN_HASH_CONST) >> rsBits);
}
inline
unsigned int MarshalerDict::incHashFunc(OZ_Term i)
{
  ozword m = ((ozword) i) * GOLDEN_HASH_CONST;
  return (((m << slsBits) >> rsBits) | 0x1); // has to be odd;
}

//
void MarshalerDict::recordNode(OZ_Term n, int ind)
{
  if (counter > percent) resize();

  //
  ozword m = ((ozword) n) * GOLDEN_HASH_CONST;
  unsigned int pkey = m >> rsBits;
  Assert(pkey == primeHashFunc(n));
  unsigned int ikey = 0;
  int key = (int) pkey;
  DebugCode(int step = 1;);

  //
  while (1) {
    MarshalerDict_Node *mdn = &table[key];
    if (mdn->getCnt() < pass) {
      mdn->setInd(n, ind, pass);
      counter++;
      break;
    } else {
      // Not supposed to be already there:
      Assert(mdn->getNode() != n);
      // next hop:
      if (ikey == 0) {
	ikey = ((m << slsBits) >> rsBits) | 0x1;
	Assert(ikey == incHashFunc(n));
	Assert(ikey < tableSize);
      }
      key -= ikey;
      if (key < 0)
	key += tableSize;
      DebugCode(step++;);
    }
  }
  DebugCode(nsearch++;);
  DebugCode(tries += step);
}

//
//
#define FindNode(AuxCodeNotFound)					\
  ozword m = ((ozword) n) * GOLDEN_HASH_CONST;				\
  unsigned int pkey = m >> rsBits;					\
  Assert(pkey == primeHashFunc(n));					\
  unsigned int ikey = 0;						\
  int key = (int) pkey;							\
  DebugCode(int step = 1;);						\
  while (1) {								\
    MarshalerDict_Node *mdn = &table[key];				\
    if (mdn->getCnt() < pass) {						\
      DebugCode(lastKey = n;);						\
      AuxCodeNotFound;							\
      DebugCode(nsearch++;);						\
      DebugCode(tries += step);						\
      return ((MarshalerDict_Node *) 0);				\
    } else if (mdn->getNode() == n) {					\
      Assert(mdn->getCnt() == pass);					\
      DebugCode(lastKey = (OZ_Term) -1);				\
      DebugCode(lastIndex = -1);					\
      DebugCode(nsearch++;);						\
      DebugCode(tries += step);						\
      return (mdn);							\
    } else {								\
      if (ikey == 0) {							\
	ikey = ((m << slsBits) >> rsBits) | 0x1;			\
	Assert(ikey == incHashFunc(n));					\
	Assert(ikey < tableSize);					\
      }									\
      key -= ikey;							\
      if (key < 0)							\
	key += tableSize;						\
      DebugCode(step++;);						\
    }									\
  }									\
  Assert(0);

//
MarshalerDict_Node* MarshalerDict::findNode(OZ_Term n)
{
  FindNode(lastIndex = key;);
}

//
MarshalerDict_Node* MarshalerDict::locateNode(OZ_Term n)
{
  FindNode(;);
}

//
void MarshalerDict::resize()
{
  int oldSize = tableSize;
  unsigned int oldPass = pass;
  MarshalerDict_Node* old = table;    

  tableSize = tableSize * 2;
  bits++; 
  mkTable();

  //
  for (int i = oldSize; i--; ) {
    MarshalerDict_Node *n = &old[i];
    if (n->getCnt() == oldPass)
      recordNode((n->getNode()), n->getAnyIndex());
  }

  //
  delete [] old;
}

//
struct GCMarshalerDictEntry {
  OZ_Term term;
  int index;
};

//
void MarshalerDict::gCollect()
{
  int asize = getSize();
  if (asize == 0)
    return;

  //
  GCMarshalerDictEntry *ta = new GCMarshalerDictEntry[asize];
  MarshalerDict_Node *n = getFirst();
  int i = 0;
  DebugCode(int realCnt = 0;);
  do {
    if (!n->firstTime()) {
      ta[i].term  = n->getNode();
      ta[i].index = n->getIndex();
      i++;
    }
    DebugCode(realCnt++;);
    n = getNext(n);
  } while (n);
  Assert(realCnt == asize);
  asize = i;

  //
  mkEmpty();

  //
  for (i = asize; i--; ) {
    OZ_Term t = ta[i].term;
    // 't' is either an (immediate) non-variable, a reference to a
    // variable (possibly GC"ed) which can be an OzValuePatch. Observe
    // that no direct variables or GC tags can be there;
    // 
    Assert(!oz_isMark(t));
    Assert(!oz_isVar(t));
#ifdef DEBUG_CHECK
    Bool isVar;
    if (oz_isRef(t)) {
      isVar = OK;
      Assert(oz_isVar(*tagged2Ref(t)) || isGCMarkedTerm(t));
    } else {
      isVar = NO;
    }
#endif

    // 
    oz_gCollectTerm(t, t);

    //
    // Now, the GC occasionaly adds (is free to!) references, so:
    DEREF(t, tp);
    Assert(!oz_isRef(t));
    if (oz_isVarOrRef(t))
      t = makeTaggedRef(tp);
    // ... otherwise just leave it dereferenced;
    Assert((isVar && oz_isRef(t)) || (!isVar && !oz_isRef(t)));

    //
    recordNode(t, ta[i].index);
  }

  //
  delete ta;
}

//
#ifdef DEBUG_CHECK
void MarshalerDict::print()
{
  for(int i = 0; i < tableSize; i++) {
    if (table[i].getCnt() == pass) {
      printf("table[%d] = <0x%x,%d>\n", i,
	     table[i].getNode(), table[i].index);
    }
  }
  printStatistics();
}

void MarshalerDict::printStatistics(int th)
{
  int misspl = 0;
  DebugCode(int sum = 0;);

  //
  for(int i = 0; i < tableSize; i++) {
    if (table[i].getCnt() == pass) {
      unsigned int pkey = primeHashFunc((table[i].getNode()));
      sum++;
      if (pkey != i)
	// that is, an alien entry took place here;
	misspl++;
    }
  }
  Assert(sum == counter);

  //
  if (nsearch > 0 && (int) tries/nsearch > th) {
    printf("\nHashtable-Statistics:\n");
    printf("\tnumber of misplaced entries: %d\n", misspl);
    printf("\tnumber of searches:          %d\n", nsearch);
    printf("\tmaximal search tries:        %d\n", maxtries);
    printf("\taverage search tries:        %.3f\n", (double) tries/nsearch);
    printf("\t%d table entries have been used for %d literals (%d%%)\n", 
	   tableSize, counter, counter*100/tableSize);
  }
}
#endif
