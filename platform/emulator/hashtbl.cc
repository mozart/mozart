/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5261
  Author: tmueller
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(INTERFACE)
#pragma implementation "hashtbl.hh"
#endif

#include <math.h>
#include <malloc.h>
#include <unistd.h>
#include <stdio.h>

#include "types.hh"
#include "hashtbl.hh"

const int STEP=5;
const double MAXFULL=0.75; // The max. load of HashTable


inline Bool isPrime(int prime)
{
  if (prime%2 == 0) {
    return NO;
  }
  for(int i=3; i*i<=prime; i+=2) {
    if (prime%i == 0) {
      return NO;
    }
  }

  return OK;
}

int nextPrime(int prime)
{
  if (prime <= STEP) {
    prime = STEP+2;
  }
  if (prime%2 == 0) {
    prime++;
  }

  while(!isPrime(prime)) {
    prime += 2;
  }
  return prime;
}



HashTable::HashTable(HtKeyType typ, int s)
{
  tableSize = nextPrime(s);
  type = typ;
  counter = 0;
  percent = (int) (MAXFULL * tableSize);
  table = new HashNode[tableSize];
  for(int i=0; i<tableSize; i++) {
    table[i].setEmpty();
  }
}

HashTable::~HashTable()
{
  /* dispose hash table itself */
  delete [] table;
}



// M e t h o d s

inline int HashTable::hashFunc(intlong i) {
  return i % tableSize;
}

inline int HashTable::hashFunc(char *s) {
// 'hashfunc' is taken from 'Aho,Sethi,Ullman: Compilers ...', page 436
  char *p = s;
  unsigned h = 0, g;
  for(; *p; p++) {
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }
  return h % tableSize;
}

unsigned HashTable::memRequired(int valSize)
{
  unsigned mem = tableSize * sizeof(HashNode);
  for(int i = 0; i < tableSize; i++){
    HashNode *lnp = &table[i];
    if (! lnp->isEmpty()) {
      mem += valSize;
      if (type == HT_CHARKEY) {
        mem += strlen(lnp->key.fstr);
      }
    }
  }
  return mem;
}


void HashTable::resize()
{
  int oldSize = tableSize;
  tableSize = nextPrime(tableSize*2);
  counter = 0;
  percent = (int) (MAXFULL * tableSize);
  HashNode* neu = new HashNode[tableSize];
  HashNode* old = table;
  table = neu;
  int i;
  for (i=0; i<tableSize; i++)
    neu[i].setEmpty();
  if (type == HT_INTKEY) {
    for (i=0; i<oldSize; i++) {
      if (! old[i].isEmpty())
        htAdd(old[i].key.fint,old[i].value);
    }
  } else {
    for (i=0;i<oldSize;i++) {
      if (! old[i].isEmpty()) {
        htAdd(old[i].key.fstr,old[i].value,NO);
      }
    }
  }
  delete [] old;
}


inline int incKey(int key, int s)
{
  key += STEP;
  if (key >= s) {
    key -= s;
  }
  return key;
}


inline int HashTable::findIndex(char *s)
{
  int key = hashFunc(s);
  while (! table[key].isEmpty() && (strcmp(table[key].key.fstr,s)!=0)) {
    key = incKey(key,tableSize);
  }
  return key;
}

inline int HashTable::findIndex(intlong i)
{
  int key = hashFunc(i);
  while (! table[key].isEmpty() && table[key].key.fint != i) {
    key = incKey(key,tableSize);
  }
  return key;
}


Bool HashTable::htAdd(char *k, void *val, Bool duplicate)
{
  Assert(val!=htEmpty);

  if (counter > percent)
    resize();

  int key = findIndex(k);
  if (! table[key].isEmpty()) {     // already in there
    if (duplicate)
      free(table[key].key.fstr);
  } else {
    counter++;
  }

  table[key].key.fstr  = duplicate ? ozstrdup(k) : k;
  table[key].value = val;
  return OK;
}

Bool HashTable::htAdd(intlong k, void *val)
{
  Assert(val!=htEmpty);

  if (counter > percent)
    resize();

  int key = findIndex(k);
  if (table[key].isEmpty()) {     // already in there
    counter++;
  }

  table[key].key.fint  = k;
  table[key].value = val;
  return OK;
}


void *HashTable::htFind(char *s)
{
  int key = findIndex(s);
  return (table[key].isEmpty())
    ? htEmpty : table[key].value;
}

void *HashTable::htFind(intlong i)
{
  int key = findIndex(i);
  return (table[key].isEmpty())
    ? htEmpty : table[key].value;
}

int HashTable::lengthList(int i)
{
  int key;
  if (type == HT_CHARKEY)
    key = hashFunc(table[i].key.fstr);
  else
    key = hashFunc(table[i].key.fint);
  int ret = 1;
  while(key != i) {
    ret++;
    key = incKey(key,tableSize);
  }
  return ret;
}

void HashTable::print()
{
  if (type == HT_CHARKEY) {
    for(int i = 0; i < tableSize; i++) {
      if (! table[i].isEmpty()) {
        printf("table[%d] = <%s,0x%x>\n", i, table[i].key.fstr, table[i].value);
      }
    }
  } else {
    for(int i = 0; i < tableSize; i++) {
      if (!table[i].isEmpty()) {
        printf("table[%d] = <%d,0x%x>\n", i, table[i].key.fint, table[i].value);
      }
    }
  }
  printStatistic();
}

void HashTable::printStatistic()
{
  int maxx = 0, sum = 0, collpl = 0, coll = 0;
  for(int i = 0; i < tableSize; i++) {
    if (table[i].isEmpty())
      continue;
    int l = lengthList(i);
    maxx = maxx > l ? maxx : l;
    sum += l;
    coll  += l > 1 ? l - 1 : 0;
    collpl += l > 1 ? 1 : 0;
  }
  printf("\nHashtable-Statistics:\n");
  printf("\tmaximum bucket length     : %d\n", maxx);
  printf("\tnumber of collision places: %d\n", collpl);
  printf("\tnumber of collisions      : %d\n", coll);
  printf("\t%d table entries have been used for %d literals (%d%%)\n",
         tableSize, counter, counter*100/tableSize);
}
