/*
 *  Authors:
 *    Tobias Mueller <tmueller@ps.uni-sb.de>
 *    Ralf Scheidhauer <Ralf.Scheidhauer@ps.uni-sb.de>
 *
 *  Contributors:
 *    Konstantin Popov <kost@sics.se>
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

#define htEmpty ((void*) -1L)

class HashNode;

/* keys of hashtables may be integers or strings */
typedef enum {HT_INTKEY = 0, HT_CHARKEY = 1} HtKeyType;

typedef union {const char *fstr; intlong fint; } HtKey;

class HashNode {
  public:
  HtKey key;
  void * value;
  void setEmpty() { key.fint = (intlong) htEmpty; }
  HashNode() : value(NULL)
  {
    setEmpty();
  };

  Bool isEmpty()  { return (key.fint == (intlong) htEmpty); }
};

class HashTable {
protected:
  int counter;      // number of entries
  int percent;      // if more than percent is used, we reallocate
  int tableSize;
  HtKeyType type;
  HashNode * table;
  int hashFunc(intlong);
  int hashFunc(const char *);
  int findIndex(intlong);
  int findIndex(const char *);
  int lengthList(int i);
  void resize();

public:
  HashTable(HtKeyType,int sz);
  ~HashTable();

  //
  int getSize() { return counter; }
  void htAdd(const char *k, void *val);
  void htAdd(intlong k, void *val);
  void *htFind(intlong);
  void *htFind(const char *);
  void mkEmpty();
  void print();
  void printStatistic();
  unsigned memRequired(int valSize = 0);
  int getTblSize(){return tableSize;}

  //
protected:
  HashNode *getNext(HashNode *hn) {
    for (hn++; hn < table+tableSize; hn++) {
      if (!hn->isEmpty())
        return (hn);
    }
    return ((HashNode *) 0);
  }
  HashNode *getFirst() { return (getNext(table-1)); }
};

//
// kost@ : A hash table with the O(n) (n=number of entries) reset
// time. Useful e.g. for marshaling. The idea is that a hash node
// keeps the number of a previously allocated node, so 'reset()'
// traverses those backwards (keep in mind also that there is no
// 'delete' operation). Unfortunately, i don't see any simple and
// efficient way to just extend the 'HashTable'. So, a lot of things
// are plain copied...

//
class HashNodeLinked {
public:
  HtKey key;
  void * value;
  HashNodeLinked *prev;

  //
public:
  void setEmpty() { key.fint = (intlong) htEmpty; }
  Bool isEmpty()  { return (key.fint == (intlong) htEmpty); }
  //
  HashNodeLinked() { setEmpty(); }
};

//
// Only 'intlong' keys are supported by now;
class HashTableFastReset {
private:
  int counter;      // number of entries
  int percent;      // if more than percent is used, we reallocate
  int tableSize;
  HashNodeLinked *table;
  HashNodeLinked *prev;

  //
private:
  int hashFunc(intlong i) { return (((unsigned) i) % tableSize); }
  int findIndex(intlong i);
  void mkTable();
  void resize();
  DebugCode(int lengthList(int););

  //
public:
  HashTableFastReset(int sz);
  ~HashTableFastReset();

  //
  int getSize() { return (counter); }
  void htAdd(intlong k, void *val);
  void *htFind(intlong k);
  void mkEmpty();

  //
  // for e.g. garbage collection:
  HashNodeLinked *getNext(HashNodeLinked *hn) {
    for (hn++; hn < table+tableSize; hn++) {
      if (!hn->isEmpty())
        return (hn);
    }
    return ((HashNodeLinked *) 0);
  }
  HashNodeLinked *getFirst() { return (getNext(table-1)); }

  //
  DebugCode(void print(););
  DebugCode(void printStatistics(););
};

#endif
