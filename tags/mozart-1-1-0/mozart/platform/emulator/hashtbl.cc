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
#pragma implementation "hashtbl.hh"
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

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



void HashTable::mkEmpty()
{
  counter = 0;
  percent = (int) (MAXFULL * tableSize);
  for(int i=0; i<tableSize; i++) {
    table[i].setEmpty();
  }
}

HashTable::HashTable(HtKeyType typ, int s)
{
  type = typ;
  tableSize = nextPrime(s);
  table = new HashNode[tableSize];
  mkEmpty();
}

HashTable::~HashTable() 
{
  /* dispose hash table itself */
  delete [] table;
}



// M e t h o d s

inline int HashTable::hashFunc(intlong i) {
  return ((unsigned) i) % tableSize;
}

inline int HashTable::hashFunc(const char *s) {
// 'hashfunc' is taken from 'Aho,Sethi,Ullman: Compilers ...', page 436
  const char *p = s;
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
	htAdd(old[i].key.fstr,old[i].value);
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


inline int HashTable::findIndex(const char *s)
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


void HashTable::htAdd(const char *k, void *val)
{
  Assert(val!=htEmpty);

  if (counter > percent)
    resize();
  
  int key = findIndex(k);
  if (table[key].isEmpty()) {
    counter++;
  }
  
  table[key].key.fstr = k;
  table[key].value = val;
}

void HashTable::htAdd(intlong k, void *val)
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
}


void *HashTable::htFind(const char *s)
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

#ifdef DEBUG_CHECK

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
	printf("table[%d] = <%s,0x%p>\n", i, table[i].key.fstr, table[i].value);
      }
    }
  } else {
    for(int i = 0; i < tableSize; i++) {
      if (!table[i].isEmpty()) {
	printf("table[%d] = <%ld,0x%p>\n", i, table[i].key.fint, table[i].value);
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

#endif

//
// above this do usual sequential reset;
const double DUMMYRESET = 0.33;

//
//
void HashTableFastReset::mkTable()
{
  counter = 0;
  percent = (int) (MAXFULL * tableSize);
  prev = (HashNodeLinked *) 0;
  table = new HashNodeLinked[tableSize];
}

HashTableFastReset::HashTableFastReset(int sz)
{
  tableSize = nextPrime(sz);
  mkTable();
}

HashTableFastReset::~HashTableFastReset() 
{
  delete [] table;
  DebugCode(prev = (HashNodeLinked *) -1);
}

inline int HashTableFastReset::findIndex(intlong i)
{
  int key = hashFunc(i);
  while (! table[key].isEmpty() && table[key].key.fint != i) {
    key = incKey(key,tableSize);
  }
  return key;
}

void HashTableFastReset::htAdd(intlong k, void *val)
{
  Assert(val != htEmpty);

  //
  if (counter > percent) resize();

  //
  int key = findIndex(k);
  if (table[key].isEmpty()) { // may be already in there;
    table[key].key.fint  = k;
    table[key].value = val;
    table[key].prev = prev;

    //
    prev = &table[key];
    counter++;
  }
}

void HashTableFastReset::mkEmpty()
{
  if (counter > (int) (DUMMYRESET * tableSize)) {
    for(int i = 0; i < tableSize; i++) {
      table[i].setEmpty();
    }
    prev = (HashNodeLinked *) 0;
  } else {
    while (prev) {
      HashNodeLinked *node = prev;
      prev = prev->prev;
      node->setEmpty();
    }
  }
  counter = 0;
}

void *HashTableFastReset::htFind(intlong i)
{
  int key = findIndex(i);
  return (table[key].isEmpty())
    ? htEmpty : table[key].value;
}

void HashTableFastReset::resize()
{
  int oldSize = tableSize;
  HashNodeLinked* old = table;    

  tableSize = nextPrime(tableSize*2);
  mkTable();

  //
  for (int i = 0; i < oldSize; i++) {
    if (! old[i].isEmpty()) 
      htAdd(old[i].key.fint, old[i].value);
  }

  //
  delete [] old;
}

#ifdef DEBUG_CHECK
void HashTableFastReset::print()
{
  for(int i = 0; i < tableSize; i++) {
    if (!table[i].isEmpty()) {
      printf("table[%d] = <%ld,0x%p>\n", i,
	     table[i].key.fint, table[i].value);
    }
  }
  printStatistics();
}

void HashTableFastReset::printStatistics()
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

int HashTableFastReset::lengthList(int i)
{
  int key = hashFunc(table[i].key.fint);
  int ret = 1;
  while (key != i) {
    ret++;
    key = incKey(key, tableSize);
  }
  return (ret);
}
#endif
