/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5261
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __HASHTABLEH
#define __HASHTABLEH

#ifdef INTERFACE
#pragma interface
#endif

#define htEmpty ((void*) -1L)

class HashNode;

typedef enum {INTTYPE = 0, CHARTYPE = 1} hashType;

typedef union {char *fstr; intlong fint; } HtKey;

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
  hashType type;
  HashNode * table;
  int hashFunc(intlong);
  int hashFunc(char *);
  int findIndex(intlong);
  int findIndex(char *);
  int lengthList(int i);
  void resize();

public:
  HashTable(hashType,int);
  ~HashTable();

  // return NO iff already in there and replace = NO
  Bool aadd(void *, char *, Bool replace = NO);
  Bool aadd(void *, intlong, Bool replace = NO);
  void *ffind(intlong);
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


