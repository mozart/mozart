/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5261
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------

  exported variables/classes: class HashTable

  exported procedures: no
  ------------------------------------------------------------------------

  internal static variables: no

  internal procedures: no

  ------------------------------------------------------------------------
*/

#ifndef __HASHTABLEH
#define __HASHTABLEH

#ifdef __GNUC__
#pragma interface
#endif

#include <unistd.h>
#include <stdio.h>

#include "error.hh"
#include "types.hh"


#define htEmpty ((void*) -1)

class HashNode;

typedef enum hashType {INTTYPE = 0, CHARTYPE = 1};

typedef union {char *fstr; unsigned int fint; } HtKey;

class HashNode {
  public:
  HtKey key;
  void * value;
  HashNode() : value(NULL)
  {
    setEmpty();
  };

  Bool isEmpty()  { return (key.fint == (unsigned int) htEmpty); }
  void setEmpty() { key.fint = (unsigned int) htEmpty; }
};

class HashTable {
protected:
  int counter;      // number of entries
  int percent;      // if more than percent is used, we reallocate
  int tableSize;
  hashType type;
  HashNode * table;
  int hashFunc(int);
  int hashFunc(char *);
  int findIndex(int);
  int findIndex(char *);
  int lengthList(int i);
  void resize();

public:
  HashTable(hashType,int);
  ~HashTable();

  // return NO iff already in there and replace = NO
  Bool aadd(void *, char *, Bool replace = NO);
  Bool aadd(void *, int , Bool replace = NO);
  void *ffind(int);
  void *ffind(char *);
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
