/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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

#include "types.hh"

#define htEmpty ((void*) -1L)

class HashNode;

/* keys of hashtables may be integers or strings */
typedef enum {HT_INTKEY = 0, HT_CHARKEY = 1} HtKeyType;

typedef union {const char *fstr; intlong fint; } HtKey;

class HashNode {
  public:
  HtKey key;
  void * value;
  HashNode() : value(NULL)
  {
    setEmpty();
  };

  Bool isEmpty()  { return (key.fint == (intlong) htEmpty); }
  void setEmpty() { key.fint = (intlong) htEmpty; }
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

  void htAdd(const char *k, void *val);
  void htAdd(intlong k, void *val);
  void *htFind(intlong);
  void *htFind(const char *);
  void print();
  void printStatistic();
  unsigned memRequired(int valSize = 0);
  int getTblSize(){return tableSize;}
  
  HashNode *getNext(HashNode *hn)
  {
    hn++;
    
    for (; hn < table+tableSize; hn++) {
      if (!hn->isEmpty())
	return hn;
    }
    
    return NULL;
  }
  
  HashNode *getFirst()
  {
    return getNext(table-1);
  }
};

#endif


