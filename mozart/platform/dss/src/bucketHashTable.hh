/*
 *  Authors:
 *    Erik Klintskog
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Erik Klintskog,2001
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

#ifndef __BUCKET_HH
#define __BUCKET_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"

// A generic hash table implementation, which handles collisions with
// chaining.  The nodes contain the elements, and can be linked to
// each other.  The class Node must have the following interface:
//
// class Node {
//   unsigned int hashCode();
//   bool         hashMatch(Key);
//   Node*&       hashSucc();
// };

#define BucketHashTable_MAXLOAD 0.75     // maximum load factor
#define BucketHashTable_MINLOAD 0.2      // minimum load factor
#define BucketHashTable_EXPAND  2        // expansion factor

template <typename Node>
class BucketHashTable {
private:
  Node** table;
  unsigned int size;         // size of table
  unsigned int minsize;      // minimal size of table
  unsigned int count;        // number of elements in table
  unsigned int maxcount;     // maximum count before resizing the table

  // don't use these
  BucketHashTable(const BucketHashTable&) {}
  BucketHashTable& operator=(const BucketHashTable&) { return *this; }

  void insertInTable(Node* const &n) {
    unsigned int i = n->hashCode() % size;
    n->hashSucc() = table[i];
    table[i] = n;
  }

  void init(unsigned int sz) {
    size = sz;
    maxcount = BucketHashTable_MAXLOAD * sz;
    table = new Node* [sz];
    while (sz--) table[sz] = NULL;
  }
  void resize(unsigned int sz) {
    Node** oldtable = table;
    unsigned int oldsize = size;
    init(sz);
    // now move all nodes from oldtable to the new table
    for (unsigned int i = 0; i < oldsize; i++) {
      Node* n = oldtable[i];
      while (n) {
	Node* m = n->hashSucc();
	insertInTable(n);
	n = m;
      }
    }
    delete [] oldtable;
  }
  void clear() {
    for (unsigned int i = 0; i < size; i++) {
      while (table[i]) {
	Node* n = table[i]->hashSucc();
	delete table[i];
	table[i] = n;
      }
    }
    count = 0;
  }

public:
  BucketHashTable(unsigned int sz) : minsize(sz), count(0) { init(sz); }
  ~BucketHashTable() { clear(); delete [] table; }

  unsigned int getSize() const { return size; }
  unsigned int getUsed() const { return count; }

  // only for decreasing table size (increase is automatic)
  void checkSize() {
    if (size >= minsize * BucketHashTable_EXPAND && count < BucketHashTable_MINLOAD * size)
      resize(size / BucketHashTable_EXPAND);
  }

  // find with a given key; return NULL if not found
  template <typename Key>
  Node* lookup(unsigned int hash, Key const &key) {
    for (Node* n = table[hash % size]; n; n = n->hashSucc()) {
      if (n->hashMatch(key)) return n;
    }
    return NULL;
  }
  // insert a node in the table
  void insert(Node* const &n) {
    if (count >= maxcount) resize(size * BucketHashTable_EXPAND);
    insertInTable(n);
    count++;
  }
  // remove n from the table (without deleting it)
  void remove(Node* const &n) {
    unsigned int i = n->hashCode() % size;
    if (!table[i]) return;
    if (table[i] == n) {   // n is first element in bucket
      table[i] = n->hashSucc();
      count--;
      return;
    }
    for (Node* p = table[i]; p; p = p->hashSucc()) {
      if (p->hashSucc() == n) {   // found predecessor of n in bucket
	p->hashSucc() = n->hashSucc();
	count--;
	return;
      }
    }
  }

  // used for traversing elements; return NULL when done
  Node* getFirst() {
    for (unsigned int i = 0; i < size; i++) {
      if (table[i]) return table[i];
    }
    return NULL;
  }
  Node* getNext(Node* const &n) {
    if (n->hashSucc()) return n->hashSucc();
    for (unsigned int i = n->hashCode() % size + 1; i < size; i++) {
      if (table[i]) return table[i];
    }
    return NULL;
  }
};

// a basic template for nodes
template <class Node>
class BucketHashNode {
private:
  Node* succ;
public:
  BucketHashNode() : succ(NULL) {}
  Node*& hashSucc() { return succ; }
};

#endif
