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

const double SHT_MAXLOAD = 0.75;

#define MIN_PRIME	7

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

// kost@ : good enough for our purposes..
int nextPrime(int prime)
{
  if (prime < MIN_PRIME) {
    prime = MIN_PRIME;
  }
  if (prime%2 == 0) {
    prime++;
  }

  while(!isPrime(prime)) {
    prime += 2;
  }
  return prime;
}

//
inline 
unsigned int StringHashTable::hashFunc(const char *s)
{
  // 'hashfunc' is taken from 'Aho,Sethi,Ullman: Compilers ...',
  // page 436
  const char *p = s;
  unsigned h = 0, g;
  for(; *p; p++) {
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }
  return (h % tableSize);
}

//
SHT_HashNode* StringHashTable::getFirst()
{
  SHT_HashNode *f = table;
  for (; f < table+tableSize; f++)
    if (!f->isEmpty())
      return (f);
  return ((SHT_HashNode *) 0);
}

SHT_HashNode* StringHashTable::getNext(SHT_HashNode *hn)
{
  Assert(hn);
  SHT_HashNode *n = hn->getNext();
  if (n) {
    return (n);
  } else {
    unsigned int key = hashFunc(hn->getKey().fstr);
    hn = &table[key];
    for (hn++; hn < table+tableSize; hn++) {
      if (!hn->isEmpty())
	return (hn);
    }
    return ((SHT_HashNode *) 0);
  }
}

//
StringHashTable::StringHashTable(int s)
{
  tableSize = nextPrime(s);
  table = new SHT_HashNode[tableSize];
  mkEmpty();
}

StringHashTable::~StringHashTable() 
{
  for (int i = 0; i < tableSize; i++) {
    if (! table[i].isEmpty()) {
      SHT_HashNode* hn = &table[i];
      int num = 1;
      do {
	SHT_HashNode* sn = hn;
	hn = hn->getNext();
	if (num > 1) 
	  delete sn;
	num++;
      } while (hn);
    }
  }
  delete [] table;
}

//
void StringHashTable::mkEmpty()
{
  counter = 0;
  percent = (int) (SHT_MAXLOAD * tableSize);
  for(int i = 0; i < tableSize; i++)
    table[i].setEmpty();
}

static inline
SHT_HashNode* checkKey(SHT_HashNode *hn, const char *s)
{
  Assert(!(hn->isEmpty()));
  while (strcmp((hn->getKey()).fstr, s) != 0) {
    hn = hn->getNext();
    if (!hn)
      return ((SHT_HashNode* ) 0);
  }
  return (hn);
}

//
void StringHashTable::resize()
{
  int oldSize = tableSize;
  SHT_HashNode* old = table;    
  int i;

  //
  tableSize = nextPrime(tableSize*2);
  table = new SHT_HashNode[tableSize];
  counter = 0;
  percent = (int) (SHT_MAXLOAD * tableSize);

  //
  for (i = 0; i < tableSize; i++) 
    table[i].setEmpty();
  //
  for (i = 0; i < oldSize; i++) {
    if (! old[i].isEmpty()) {
      SHT_HashNode* hn = &old[i];
      int num = 1;
      do {
	htAdd((hn->getKey()).fstr, hn->getValue());
	SHT_HashNode* sn = hn;
	hn = hn->getNext();
	if (num > 1) 
	  delete sn;
	num++;
      } while (hn);
    }
  }

  //
  delete [] old;
}

//
void StringHashTable::htAdd(const char *k, void *val)
{
  Assert(val != htEmpty);

  if (counter > percent)
    resize();
  
  unsigned int key = hashFunc(k);
  SHT_HashNode* rhn = &table[key];
  if (rhn->isEmpty()) {
    rhn->setKey(k);
    rhn->setValue(val);
    rhn->setNext((SHT_HashNode *) 0);
    counter++;
  } else {
    SHT_HashNode* fhn;
    if ((fhn = checkKey(rhn, k)) == (SHT_HashNode *) 0) {
      fhn = new SHT_HashNode(k, val, rhn->getNext());
      rhn->setNext(fhn);
      counter++;
    } else {
      fhn->setValue(val);
    }
  }
}

void* StringHashTable::htFind(const char *s)
{
  SHT_HashNode *rhn = &table[hashFunc(s)];
  SHT_HashNode *fhn;
  if (rhn->isEmpty() ||
      (fhn = checkKey(rhn, s)) == (SHT_HashNode *) 0) {
    return (htEmpty);
  } else {
    return (fhn->getValue());
  }
}

//
int StringHashTable::lengthList(int i)
{
  SHT_HashNode* hn = &table[i];
  if (hn->isEmpty())
    return (0);

  int len = 0;
  while (hn) {
    len++;
    hn = hn->getNext();
  }
  return (len);
}

void StringHashTable::print()
{
  for(int i = 0; i < tableSize; i++) {
    if (! table[i].isEmpty()) {
      SHT_HashNode* hn = &table[i];
      do {
	printf("table[%d] = <%s,0x%p>\n",
	       i, (hn->getKey()).fstr, (hn->getValue()));
	hn = hn->getNext();
      } while (hn);
    }
  }
  printStatistic();
}

void StringHashTable::printStatistic()
{
  int maxx = 0, collpl = 0, coll = 0;
  DebugCode(int sum = 0;);
  for (int i = 0; i < tableSize; i++) {
    if (table[i].isEmpty())
      continue;
    int l = lengthList(i);
    maxx = maxx > l ? maxx : l;
    DebugCode(sum += l;);
    coll  += l > 1 ? l - 1 : 0;
    collpl += l > 1 ? 1 : 0;
  }
  Assert(sum == counter);
  printf("\nHashtable-Statistics:\n");
  printf("\tmaximum bucket length     : %d\n", maxx);
  printf("\tnumber of collision places: %d\n", collpl);
  printf("\tnumber of collisions      : %d\n", coll);
  printf("\t%d table entries have been used for %d literals (%d%%)\n", 
	 tableSize, counter, counter*100/tableSize);
}

//
unsigned StringHashTable::memRequired(int valSize)
{
  unsigned mem = tableSize * sizeof(SHT_HashNode);
  for (int i = 0; i < tableSize; i++) {
    if (! table[i].isEmpty()) {
      SHT_HashNode* hn = &table[i];
      int num = 1;
      do {
	mem += valSize;
	mem += strlen((hn->getKey()).fstr);
	if (num > 1)
	  mem += sizeof(SHT_HashNode);
	hn = hn->getNext();
	num++;
      } while (hn);
    }
  }
  return (mem);
}


//
const double AHT_MAXLOAD = 0.5;

//
void AddressHashTable::mkEmpty()
{
  counter = 0;
  percent = (int) (AHT_MAXLOAD * tableSize);
  for (int i = 0; i < tableSize; i++)
    table[i].setEmpty();
  DebugCode(nsearch = 0;);
  DebugCode(tries = 0;);
  DebugCode(maxtries = 0;);
}

//
AddressHashTable::AddressHashTable(int s)
{
  incStepMod = nextPrime(s);
  tableSize = nextPrime(incStepMod+1);
  table = new AHT_HashNode[tableSize];
  mkEmpty();
}

AddressHashTable::~AddressHashTable() 
{
  /* dispose hash table itself */
  delete [] table;
}

//
// kost@ : use now double hashing with the home-grown addition:
//         multiply the value we're hashing on with a prime number.
//         Without this modification we gain 2 orders of magnitude of
//         improvement over the "linear probing", and with it we get 3
//         orders (based on dp_huge).
//
inline
unsigned int AddressHashTable::primeHashFunc(intlong i)
{
  return ((((unsigned) i) * 397) % tableSize);
}
inline
unsigned int AddressHashTable::incHashFunc(intlong i)
{
  return (1 + ((((unsigned) i) * 617) % incStepMod));
}

inline
unsigned int AddressHashTable::getStepN(unsigned int pkey,
					unsigned int ikey, int i)
{
  return ((pkey + i*ikey) % tableSize);
}

//
unsigned AddressHashTable::memRequired(int valSize)
{
  unsigned mem = tableSize * sizeof(AHT_HashNode);
  mem += valSize * counter;
  return (mem);
}

//
void AddressHashTable::resize()
{
  int oldSize = tableSize;
  incStepMod = nextPrime(tableSize*2);
  tableSize = nextPrime(incStepMod+1);
  counter = 0;
  percent = (int) (AHT_MAXLOAD * tableSize);
  AHT_HashNode* neu = new AHT_HashNode[tableSize];
  AHT_HashNode* old = table;    
  table = neu;
  int i;
  for (i = 0; i < tableSize; i++) 
    neu[i].setEmpty();
  for (i = 0; i < oldSize; i++) {
    if (! old[i].isEmpty()) 
      htAdd((old[i].getKey()).fint, old[i].getValue());
  }
  delete [] old;
}

//
inline
unsigned int AddressHashTable::findIndex(intlong i)
{
  unsigned int pkey = primeHashFunc(i);
  unsigned int ikey = incHashFunc(i);
  unsigned int key = pkey;
  int step = 1;
  //
  while (! table[key].isEmpty() && (table[key].getKey()).fint != i)
    key = getStepN(pkey, ikey, step++);
  DebugCode(nsearch++;);
  DebugCode(tries += step);
  DebugCode(if (step > maxtries) { maxtries = step; });
  return (key);
}

//
void AddressHashTable::htAdd(intlong k, void *val)
{
  Assert(val != htEmpty);

  if (counter > percent)
    resize();

  unsigned int key = findIndex(k);
  if (table[key].isEmpty())
    counter++;
  table[key].setKey(k);
  table[key].setValue(val);
}

void *AddressHashTable::htFind(intlong i)
{
  unsigned int key = findIndex(i);
  return ((table[key].isEmpty())
	  ? htEmpty : table[key].getValue());
}

#ifdef DEBUG_CHECK

void AddressHashTable::print()
{
  for(int i = 0; i < tableSize; i++) {
    if (!table[i].isEmpty())
      printf("table[%d] = <%ld,0x%p>\n",
	     i, (table[i].getKey()).fint, table[i].getValue());
  }
  printStatistic();
}

void AddressHashTable::printStatistic()
{
  int misspl = 0;
  DebugCode(int sum = 0;);

  //
  for (int i = 0; i < tableSize; i++) {
    if (!table[i].isEmpty()) {
      unsigned int pkey = primeHashFunc((table[i].getKey()).fint);
      sum++;
      if (pkey != i)
	// that is, an alien entry took place here;
	misspl++;
    }
  }
  Assert(sum == counter);

  //
  printf("\nHashtable-Statistics:\n");
  printf("\tnumber of misplaced entries: %d\n", misspl);
  printf("\tnumber of searches:          %d\n", nsearch);
  printf("\tmaximal search tries:        %d\n", maxtries);
  printf("\taverage search tries:        %.3f\n", (double) tries/nsearch);
  printf("\t%d table entries have been used for %d literals (%d%%)\n", 
	 tableSize, counter, counter*100/tableSize);
}

#endif


//
const double AHTFR_MAXLOAD = 0.5;
// above this do usual sequential reset;
const double DUMMYRESET = 0.33;

// print statistics if on average there are more than tries per search:
#define DEBUG_THRESHOLD		2

//
//
void AddressHashTableFastReset::mkTable()
{
  counter = 0;
  percent = (int) (AHTFR_MAXLOAD * tableSize);
  prev = (AHT_HashNodeLinked *) 0;
  table = new AHT_HashNodeLinked[tableSize];
  DebugCode(nsearch = 0;);
  DebugCode(tries = 0;);
  DebugCode(maxtries = 0;);
  mkEmpty(TRUE);
}

//
AddressHashTableFastReset::AddressHashTableFastReset(int sz)
{
  incStepMod = nextPrime(sz);
  tableSize = nextPrime(incStepMod+1);
  mkTable();
}

AddressHashTableFastReset::~AddressHashTableFastReset() 
{
  delete [] table;
  DebugCode(prev = (AHT_HashNodeLinked *) -1);
}

//
inline
unsigned int AddressHashTableFastReset::primeHashFunc(intlong i)
{
  return ((((unsigned) i) * 397) % tableSize);
}
inline
unsigned int AddressHashTableFastReset::incHashFunc(intlong i)
{
  return (1 + ((((unsigned) i) * 617) % incStepMod));
}

inline
unsigned int AddressHashTableFastReset::getStepN(unsigned int pkey,
						 unsigned int ikey, int i)
{
  return ((pkey + i*ikey) % tableSize);
}

//
inline
unsigned int AddressHashTableFastReset::findIndex(intlong i)
{
  unsigned int pkey = primeHashFunc(i);
  unsigned int ikey = incHashFunc(i);
  unsigned int key = pkey;
  int step = 1;
  //
  while (! table[key].isEmpty() && (table[key].getKey()).fint != i)
    key = getStepN(pkey, ikey, step++);
  DebugCode(nsearch++;);
  DebugCode(tries += step);
  DebugCode(if (step > maxtries) { maxtries = step; });
  return (key);
}

void AddressHashTableFastReset::htAdd(intlong k, void *val)
{
  Assert(val != htEmpty);

  //
  if (counter > percent) resize();

  //
  unsigned int key = findIndex(k);
  if (table[key].isEmpty()) { // may be already in there;
    table[key].setKey(k);
    table[key].setValue(val);
    table[key].setPrev(prev);

    //
    prev = &table[key];
    counter++;
  } else {
    Assert(table[key].getValue() == val);
  }
}

//
void AddressHashTableFastReset::mkEmpty(Bool force)
{
  // DebugCode(printStatistics(DEBUG_THRESHOLD));
  if (force || counter > (int) (DUMMYRESET * tableSize)) {
    for (int i = 0; i < tableSize; i++) {
      table[i].setEmpty();
    }
    prev = (AHT_HashNodeLinked *) 0;
  } else {
    while (prev) {
      AHT_HashNodeLinked *node = prev;
      prev = prev->getPrev();
      node->setEmpty();
    }
  }
  counter = 0;
  DebugCode(nsearch = 0;);
  DebugCode(tries = 0;);
  DebugCode(maxtries = 0;);
}

//
void* AddressHashTableFastReset::htFind(intlong i)
{
  unsigned int key = findIndex(i);
  return ((table[key].isEmpty())
	  ? htEmpty : table[key].getValue());
}

//
void AddressHashTableFastReset::resize()
{
  int oldSize = tableSize;
  AHT_HashNodeLinked* old = table;    

  incStepMod = nextPrime(tableSize*2);
  tableSize = nextPrime(incStepMod+1);
  mkTable();

  //
  for (int i = 0; i < oldSize; i++) {
    if (! old[i].isEmpty()) 
      htAdd((old[i].getKey()).fint, old[i].getValue());
  }

  //
  delete [] old;
}

//
#ifdef DEBUG_CHECK
void AddressHashTableFastReset::print()
{
  for(int i = 0; i < tableSize; i++) {
    if (!table[i].isEmpty()) {
      printf("table[%d] = <%ld,0x%p>\n", i,
	     (table[i].getKey()).fint, table[i].getValue());
    }
  }
  printStatistics();
}

void AddressHashTableFastReset::printStatistics(int th)
{
  int misspl = 0;
  DebugCode(int sum = 0;);

  //
  for(int i = 0; i < tableSize; i++) {
    if (!table[i].isEmpty()) {
      unsigned int pkey = primeHashFunc((table[i].getKey()).fint);
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
