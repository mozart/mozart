/*
 *  Authors:
 *    Tobias Mueller <tmueller@ps.uni-sb.de>
 *    Ralf Scheidhauer <Ralf.Scheidhauer@ps.uni-sb.de>
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

#ifndef __HASHTABLEH
#define __HASHTABLEH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"

#define htEmpty ((void *) -1L)

class SHT_HashNode;

//
// The one with the paramount lookup efficiency (atom- and name- hash
// tables). That is, we don't really care how expensive is the
// "insert" operation, both in terms of speed and memory.

//
// The first key/value pair is stored in the table, and subsequent
// ones outside it;
class SHT_HashNode {
private:
  const char *key;
  void *value;
  SHT_HashNode *next;

public:
  void setEmpty() { key = (const char*) htEmpty; }
  Bool isEmpty()  { return (key == (const char*) htEmpty); }

  //
  void setKey(const char *sIn) { key = sIn; }
  void setValue(void *valueIn) { value = valueIn; }
  void setNext(SHT_HashNode *nextIn) { next = nextIn; }

  //
  SHT_HashNode() { setEmpty(); }
  SHT_HashNode(const char *s, void *valueIn, SHT_HashNode *nextIn)
    : value(valueIn), next(nextIn) {
    setKey(s);
    Assert(!isEmpty());
  }

  //
  const char* getKey() { return (key); }
  void* getValue() { return (value); }
  SHT_HashNode *getNext() { return (next); }
};

//
extern crc_t crc_table[256];

//
// kost@ : Now, with CRC32 hashing, the table size is the power of 2;
class StringHashTable {
protected:
  SHT_HashNode *table;          //
  int tableSize;                //
  unsigned int mask;            // hash mask;
  int counter;      // number of entries
  int percent;      // if more than percent is used, we reallocate

private:
  int lengthList(int i);

protected:
  unsigned int hashFunc(const char *p) {
    unsigned int hash = (unsigned int) 0xffffffff;
    for (; *p; p++)
      hash = crc_table[(hash ^ (*p)) & 0xff] ^ (hash >> 8);
    // this scheme performs quite well [for smaller examples] too:
    // for (; *p; p++)
    //   hash = hash = (hash<<7)^(hash>>25)^(*p);
    return (hash & mask);
  }
  void resize();

public:
  StringHashTable(int sz);
  ~StringHashTable();

  //
  int getSize() { return counter; }
  void mkEmpty();

  //
  void htAdd(const char *k, void *val);
  void *htFind(const char *);

  //
  void print();
  void printStatistic();
  unsigned memRequired(int valSize = 0);
  int getTblSize() { return (tableSize); }

  //
protected:
  SHT_HashNode *getFirst();
  SHT_HashNode *getNext(SHT_HashNode *hn);
};


//
// Compact one (does not allocate additional memory outside the table:
// "open addressing"). "delete" is not supported. Find/insert are
// supposed to be still reasonably fast..
//
class AHT_HashNode {
private:
  void *key;
  void *value;

public:
  void setEmpty() { key = (void *) htEmpty; }
  Bool isEmpty()  { return (key == (void *) htEmpty); }

  //
  AHT_HashNode() { setEmpty(); }

  //
  void setKey(void *iIn) { key = iIn; }
  void setValue(void *vIn) { value = vIn; }

  //
  void* getKey() { return (key); }
  void* getValue() { return (value); }
};

//
class AddressHashTable {
protected:
  AHT_HashNode* table;          //
  int tableSize;                //
  int counter;                  // number of entries
  int percent;                  // reallocate when percent > counter;
  //
  int bits;                     // pkey & skey;
  int rsBits;                   // right shift;
  int slsBits;                  // skey left shift;
  DebugCode(int nsearch;);      // number of searches;
  DebugCode(int tries;);        // accumulated;
  DebugCode(int maxtries;);

protected:
  unsigned int primeHashFunc(void*);
  unsigned int incHashFunc(void*);

  void resize();

public:
  AddressHashTable(int sz);
  ~AddressHashTable();

  //
  int getSize() { return counter; }
  void mkEmpty();

  //
  void htAdd(void *k, void *val);
  void *htFind(void *k);

  //
  DebugCode(void print(););
  DebugCode(void printStatistic(););
  unsigned memRequired(int valSize = 0);
  int getTblSize() { return (tableSize); }

  //
protected:
  AHT_HashNode *getNext(AHT_HashNode *hn) {
    for (hn++; hn < table+tableSize; hn++) {
      if (!hn->isEmpty())
        return (hn);
    }
    return ((AHT_HashNode *) 0);
  }
  AHT_HashNode *getFirst() { return (getNext(table-1)); }
};


//
// The 'AddressHashTableO1Reset' hash table performs the 'mkEmpty()'
// operation in constant time. This is supported by the 'cnt' field,
// which holds the current pass if the node is in use, and some
// smaller number otherwise:
class AHT_HashNodeCnt {
private:
  void *key;
  void *value;
  unsigned int cnt;

  //
public:
  AHT_HashNodeCnt() { cnt = 0; }

  unsigned int getCnt() { return (cnt); }
  void setCnt(unsigned int cntIn) { cnt = cntIn; }

  //
  void setKey(void *kIn) { key = kIn; }
  void setValue(void *vIn) { value = vIn; }

  //
  void* getKey() { return (key); }
  void* getValue() { return (value); }
};

//
class AddressHashTableO1Reset {
private:
  AHT_HashNodeCnt *table;
  int tableSize;
  int counter;                  // number of entries;
  int percent;                  // reallocate when percent > counter;
  //
  int bits;                     // pkey & skey;
  int rsBits;                   // right shift;
  int slsBits;                  // skey left shift;
  unsigned int pass;            // current pass;
  // (inits to 1 since AHT_HashNodeCnt inits it to 0);
  int lastIndex;
  DebugCode(void *lastKey;);
  DebugCode(int nsearch;);      // number of searches since last 'mkEmpty()';
  DebugCode(int tries;);        // accumulated over 'nsearch';
  DebugCode(int maxtries;);
  DebugCode(int nsearchAcc;);   // .. since object construction;
  DebugCode(int triesAcc;);     // accumulated over 'nsearchAcc';

  //
private:
  unsigned int primeHashFunc(void*);
  unsigned int incHashFunc(void*);

  void mkTable();
  void resize();

  //
public:
  AddressHashTableO1Reset(int sz);
  ~AddressHashTableO1Reset();

  //
  int getSize() { return (counter); }
  void htAdd(void *k, void *v);
  void htAddLastNotFound(void *k, void *v);
  void htAddOverWrite(void *k, void *val); //comments at the .cc-file
  void* htFind(void *k);
  void mkEmpty();

  //
  // for e.g. garbage collection:
  AHT_HashNodeCnt *getNext(AHT_HashNodeCnt *hn) {
    for (hn--; hn >= table; hn--) {
      if (hn->getCnt() == pass)
        return (hn);
    }
    return ((AHT_HashNodeCnt *) 0);
  }
  AHT_HashNodeCnt *getFirst() { return (getNext(table+tableSize)); }

  //
  DebugCode(void print(););
  DebugCode(void printStatistics(int th = 0););
};

//
// Generic template for tables with the 'delete' operation, and where
// dedicated nodes are (m-)allocated anyway. Notably, for the borrow
// (and possibly also owner) table. It is supposed to supersede the
// (in)famous 'GenHashTable'.
//
// Table keeps "nodes" that must be inherited from 'GenDistEntryNode'.
// Specific nodes must implement comparison methods, etc.
//
// The current implementation of 'GenDistEntryTable' is a bucket hash
// table-based one, with sorted lists. The hashing scheme is the
// multiplicative one (using the Knuth' terminology).
//

//
// 'GenDistEntryTable' uses methods of this class.
// Also, the linkage between nodes is provided;
template<class NODE>
class GenDistEntryNode {
private:
  NODE *next;

public:
  GenDistEntryNode() { DebugCode (next = (NODE *) -1;); }

  //
  NODE** getNextNodeRef() { return (&next); }
  NODE *getNext() { return(next); }
  void setNext(NODE *n) { next = n; }

  // Interface methods - must be implemented properly by specific node
  // types.
  // 'value4hash()' returns an unsigned integer for hashing;
  unsigned int value4hash() { Assert(0); return (0); }
  // Negative if less, 0 if equal, positive if larger than 'n';
  int compare(NODE *n) { Assert(0); return (0); }
};

//
template<class NODE>
class GenDistEntryTable {
private:
  // buckets are sorted in ascending order;
  NODE **table;
  int tableSize;                // number of cells (not the logarithm);
  int counter;                  // number of allocated entries;
  int percent;                  // reallocate when percent > counter;

  //
  // 'bits' is the number of bits in a hash value, and 'rsBits' is the
  // number of bits in the Right Shift to leave 'bits' in a hash
  // value;
  int bits;
  int rsBits;

  //
private:
  void mkEmpty();
  void init(int sizeAsPowerOf2);
  void resize();

  //
  unsigned int hash(unsigned int i) {
    // golden cut = 0.6180339887 = A/w
    // (0.61803398874989484820458683436563811772030917980576 ..),
    // there are 32bit integers w = 4294967296,
    // thus A = 2654435769 (11400714819323198485 for 64bit architecture).
    // Yet better, follow Knuth volume 3 section 6.4:
    //   0.25                < A/w < 0.29999999999999999
    //   0.33333333333333331 < A/w < 0.42857142857142855
    //   0.5714285714285714  < A/w < 0.66666666666666663
    //   0.69999999999999996 < A/w < 0.75
    // For MIX with radix=10 with 5 bytes word 'A' is suggested to be
    //   6125423371 = 61*10^4 + 25*10^3 + 42*10^2 + 33*10^1 + 71*10^0
    // For our radix=2 computers with 4 bytes let's take
    //   0x9e*256^3 + 0x41*256^2 + 0x93*256^1 + 0x55*256^0 = 0x9e419355
    // (2655097685 in decimal);

    // However, experiments show that if we consider a hash table of
    // 64k entries, with keys uniformly distributed between 4mb and
    // 5mb (which corresponds to 16mb of Oz heap), as well as
    // sequences of addresses with increments of 1byte, 16bytes, 32,
    // 64, ... 32768bytes, the following key outperforms the latter
    // one: 0x9e6d5541 (2657965377)

    return ((((unsigned int) i) * ((unsigned int) 0x9e6d5541)) >> rsBits);
  }

  //
public:
  GenDistEntryTable(int sizeAsPowerOf2) { init(sizeAsPowerOf2); }
  ~GenDistEntryTable() {
    delete[] table;
    DebugCode(table = (NODE **) -1;);
    DebugCode(tableSize = counter = percent = bits = rsBits = -1;);
  }

  //
  void compactify();

  // 'htAdd()' inserts any node - there may be equal nodes already in
  // there;
  // The 'newNode's 'next' field may be uninitialized (but will be set
  // properly, of course);
  void htAdd(NODE *newNode);
  // 'htDel()'/'htFind()' match nodes that are *equal* to the
  // specified one (but not necessarily the same one, of course);
  NODE* htFind(NODE *eqNode);
  // 'htDel()' does nothing if no node matches;
  void htDel(NODE *eqNode);

  //
  int getSize() { return (tableSize); }
  int getUsed() { return (counter); }

  // Sequential scavenging for garbage collection.  We assume that
  // nodes are not relocated, as well as are not added/deleted from
  // the table;
  NODE* getFirstNode(int i) { return (table[i]); }
  // Now, nodes can be removed too:
  NODE** getFirstNodeRef(int i) { return (&table[i]); }
  // delete the identified node from the table (note that a pointer to
  // a pointer is necessary), but not the node itself;
  void deleteNode(NODE *pn, NODE **ppn) {
    Assert(pn == *ppn);
    *ppn = pn->getNext();
    DebugCode(pn->setNext((NODE *) -1););
    counter--;
  }

  //
  DebugCode(void checkConsistency(););
};

//
// discrete ceiling of a logarithm base 2;
// log2ceiling(0) == 0 per definition;
inline int log2ceiling(int i) {
  Assert(i >= 0);
  int l = 0;
  if (i != 0) i--;
  while (i) {
    i = i >> 1;
    l++;
  }
  return (l);
}

#endif
