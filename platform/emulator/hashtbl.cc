/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
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

#if defined(INTERFACE)
#pragma implementation "hashtbl.hh"
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "hashtbl.hh"

const double SHT_MAXLOAD = 0.75;

/*
inline Bool isPrime(int prime)
{
  if (prime%2 == 0)
    return NO;
  for (int i=3; i*i<=prime; i+=2)
    if (prime%i == 0)
      return NO;
  return OK;
}

//
#define MIN_PRIME       7
// kost@ : good enough for our purposes..
int nextPrime(int prime)
{
  if (prime < MIN_PRIME)
    prime = MIN_PRIME;
  if (prime%2 == 0)
    prime++;

  while(!isPrime(prime))
    prime += 2;
  return prime;
}
*/

//
// kost@ : use now the CRC32, which appears to be faster (and does not
// require the table size to be a prime (!));
/*
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
*/

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
    unsigned int key = hashFunc(hn->getKey());
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
  tableSize = 128;
  while (tableSize < s)
    tableSize = tableSize * 2;
  mask = tableSize - 1;
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
  while (strcmp((hn->getKey()), s) != 0) {
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
  tableSize = tableSize*2;
  mask = tableSize - 1;
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
        htAdd((hn->getKey()), hn->getValue());
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
  Assert(k != htEmpty);
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
               i, (hn->getKey()), (hn->getValue()));
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
        mem += strlen((hn->getKey()));
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
  const int totalBits = sizeof(unsigned int)*8;
  rsBits = totalBits - bits;
  slsBits = min(bits, rsBits);

  counter = 0;
  percent = (int) (AHT_MAXLOAD * tableSize);
  for (int i = tableSize; i--; )
    table[i].setEmpty();
  DebugCode(nsearch = 0;);
  DebugCode(tries = 0;);
  DebugCode(maxtries = 0;);
}

//
AddressHashTable::AddressHashTable(int sz)
{
  tableSize = 128;
  bits = 7;
  while (tableSize < sz) {
    tableSize = tableSize * 2;
    bits++;
  }
  table = new AHT_HashNode[tableSize];
  mkEmpty();
}

AddressHashTable::~AddressHashTable()
{
  /* dispose hash table itself */
  delete [] table;
}

// These functions are not really used - just for debugging;
inline
unsigned int AddressHashTable::primeHashFunc(void *i)
{
  Assert(sizeof(unsigned int)*8 == 32);
  return ((((unsigned int) i) * ((unsigned int) 0x9e6d5541)) >> rsBits);
}
inline
unsigned int AddressHashTable::incHashFunc(void *i)
{
  unsigned int m = ((unsigned int) i) * ((unsigned int) 0x9e6d5541);
  // has to be odd (in order the hash table covering be complete);
  return (((m << slsBits) >> rsBits) | 0x1);
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
  AHT_HashNode* old = table;

  tableSize = tableSize * 2;
  bits++;
  table = new AHT_HashNode[tableSize];
  mkEmpty();

  //
  for (int i = oldSize; i--; ) {
    if (! old[i].isEmpty())
      htAdd((old[i].getKey()), old[i].getValue());
  }

  //
  delete [] old;
}

//
void AddressHashTable::htAdd(void *k, void *val)
{
  if (counter > percent) resize();

  //
  Assert(k != htEmpty);
  Assert(val != htEmpty);
  unsigned int m = ((unsigned int) k) * ((unsigned int) 0x9e6d5541);
  unsigned int pkey = m >> rsBits;
  Assert(pkey == primeHashFunc(k));
  unsigned int ikey = 0;
  int key = (int) pkey;
  DebugCode(int step = 1;);

  //
  while (1) {
    if (table[key].isEmpty()) {
      // certainly not there;
      table[key].setKey(k);
      table[key].setValue(val);
      counter++;
      break;
    } else if (table[key].getKey() == k) {
      // already there;
      //DENYS: this assertion is ifdef'ed out because it turns out
      //that when building the opcodeTable, there are instructions that
      //share the same jump label.
#if !defined(THREADED)
      Assert(table[key].getValue() == val);
#endif
      break;
    } else {
      // next hop:
      if (ikey == 0) {
        ikey = ((m << slsBits) >> rsBits) | 0x1;
        Assert(ikey == incHashFunc(k));
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

void *AddressHashTable::htFind(void *k)
{
  unsigned int m = ((unsigned int) k) * ((unsigned int) 0x9e6d5541);
  unsigned int pkey = m >> rsBits;
  Assert(pkey == primeHashFunc(k));
  unsigned int ikey = 0;
  int key = (int) pkey;
  DebugCode(int step = 1;);

  //
  while (1) {
    if (table[key].isEmpty()) {
      // certainly not there;
      DebugCode(nsearch++;);
      DebugCode(tries += step);
      return (htEmpty);
    } else if (table[key].getKey() == k) {
      DebugCode(nsearch++;);
      DebugCode(tries += step);
      return (table[key].getValue());
    } else {
      // next hop:
      if (ikey == 0) {
        ikey = ((m << slsBits) >> rsBits) | 0x1;
        Assert(ikey == incHashFunc(k));
        Assert(ikey < tableSize);
      }
      key -= ikey;
      if (key < 0)
        key += tableSize;
      DebugCode(step++;);
    }
  }
  Assert(0);
}

#ifdef DEBUG_CHECK

void AddressHashTable::print()
{
  for(int i = 0; i < tableSize; i++) {
    if (!table[i].isEmpty())
      printf("table[%d] = <0x%x,0x%x>\n", i,
             (unsigned int) table[i].getKey(),
             (unsigned int) table[i].getValue());
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
      unsigned int pkey = primeHashFunc((table[i].getKey()));
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

// print statistics if on average there are more than that tries per search:
#define DEBUG_THRESHOLD         2

//
// Note: 'mkTable()' resets the 'pass';
void AddressHashTableO1Reset::mkTable()
{
  const int totalBits = sizeof(unsigned int)*8;
  // leave exactly 'bits':
  rsBits = totalBits - bits;
  // would like to drop all of the pkey's bits, but have to retain at
  // least 'bits':
  slsBits = min(bits, rsBits);

  //
  counter = 0;
  percent = (int) (AHTFR_MAXLOAD * tableSize);
  table = new AHT_HashNodeCnt[tableSize];
  // AHT_HashNodeCnt"s are initialized with cnt = 0, so make them unused:
  pass = 1;
  lastIndex = -1;
  DebugCode(lastKey = (void *) -1);
  DebugCode(nsearch = 0;);
  DebugCode(tries = 0;);
  DebugCode(maxtries = 0;);
  DebugCode(nsearchAcc = 0;);
  DebugCode(triesAcc = 0;);
}

//
void AddressHashTableO1Reset::mkEmpty()
{
  DebugCode(nsearchAcc += nsearch;);
  DebugCode(triesAcc += tries;);
  // DebugCode(printStatistics(DEBUG_THRESHOLD));
  pass++;
  if (pass == (unsigned int) 0xffffffff) {
    pass = 1;
    for (int i = tableSize; i--; )
      table[i].setCnt(0);
  }
  counter = 0;
  DebugCode(nsearch = 0;);
  DebugCode(tries = 0;);
  DebugCode(maxtries = 0;);
}

//
AddressHashTableO1Reset::AddressHashTableO1Reset(int sz)
{
  tableSize = 128;
  bits = 7;
  while (tableSize < sz) {
    tableSize = tableSize * 2;
    bits++;
  }
  mkTable();
}

AddressHashTableO1Reset::~AddressHashTableO1Reset()
{
  delete [] table;
}

//
// We're used to have also a division scheme.
// There, the hashing functions were:
/*
unsigned int AddressHashTableO1Reset::primeHashFunc(void *i)
{
  // multiplying by a prime number is supposedly better for dp_huge.
  // return ((((unsigned) i) * 397) % tableSize);
  return (((unsigned int) i) % tableSize);
}
unsigned int AddressHashTableO1Reset::incHashFunc(void *i)
{
  // return (1 + ((((unsigned int) i) * 617) % incStepMod));
  return (1 + (((unsigned int) i) % incStepMod));
}
*/
// and 'tableSize', 'incStepMod' where calculated as follows:
/*
  incStepMod = nextPrime(sz);
  tableSize = nextPrime(incStepMod+1);
*/
// The scheme worked pretty much the same in terms of collisions,
// but is much more computationally expensive.

// These functions are not really used - just for debugging;
inline
unsigned int AddressHashTableO1Reset::primeHashFunc(void *i)
{
  Assert(sizeof(unsigned int)*8 == 32);
  return ((((unsigned int) i) * ((unsigned int) 0x9e6d5541)) >> rsBits);
}
inline
unsigned int AddressHashTableO1Reset::incHashFunc(void *i)
{
  unsigned int m = ((unsigned int) i) * ((unsigned int) 0x9e6d5541);
  // has to be odd (in order the hash table covering be complete);
  return (((m << slsBits) >> rsBits) | 0x1);
}

//
void AddressHashTableO1Reset::htAdd(void *k, void *val)
{
  if (counter > percent) resize();

  //
  Assert(val != htEmpty);
  unsigned int m = ((unsigned int) k) * ((unsigned int) 0x9e6d5541);
  unsigned int pkey = m >> rsBits;
  Assert(pkey == primeHashFunc(k));
  unsigned int ikey = 0;
  int key = (int) pkey;
  DebugCode(int step = 1;);

  //
  while (1) {
    if (table[key].getCnt() < pass) {
      // certainly not there;
      table[key].setKey(k);
      table[key].setValue(val);
      table[key].setCnt(pass);
      counter++;
      break;
    } else if (table[key].getKey() == k) {
      // already there;
      Assert(table[key].getValue() == val);
      break;
    } else {
      // next hop:
      if (ikey == 0) {
        ikey = ((m << slsBits) >> rsBits) | 0x1;
        Assert(ikey == incHashFunc(k));
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

// bmc: adding code from Zacharias.
/*
  ZACHARIAS htAddSpecial is used to overwrite old entries in the
  addressHashTable. We want to reuse a key as it is assumed that the old value
  is obsolete

  This is used in the EngineTable in libdp
*/
void
AddressHashTableO1Reset::htAddOverWrite(void *k, void *val)
{
  if (counter > percent) resize();

  //
  Assert(val != htEmpty);
  unsigned int m = ((unsigned int) k) * ((unsigned int) 0x9e6d5541);
  unsigned int pkey = m >> rsBits;
  Assert(pkey == primeHashFunc(k));
  unsigned int ikey = 0;
  int key = (int) pkey;
  DebugCode(int step = 1;);

  //
  while (1) {
    if (table[key].getCnt() < pass) {
      // certainly not there;
      table[key].setKey(k);
      table[key].setValue(val);
      table[key].setCnt(pass);
      counter++;
      break;
    } else if (table[key].getKey() == k) {
      // already there;
      table[key].setValue(val); //OverWrite
      OZ_warning("HashTable reusing entry");
      break;
    } else {
      // next hop:
      if (ikey == 0) {
        ikey = ((m << slsBits) >> rsBits) | 0x1;
        Assert(ikey == incHashFunc(k));
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
void* AddressHashTableO1Reset::htFind(void *k)
{
  unsigned int m = ((unsigned int) k) * ((unsigned int) 0x9e6d5541);
  unsigned int pkey = m >> rsBits;
  Assert(pkey == primeHashFunc(k));
  unsigned int ikey = 0;
  int key = (int) pkey;
  DebugCode(int step = 1;);
  //
  while (1) {
    if (table[key].getCnt() < pass) {
      DebugCode(lastKey = k;);
      lastIndex = key;
      DebugCode(nsearch++;);
      DebugCode(tries += step);
      return (htEmpty);
    } else if (table[key].getKey() == k) {
      Assert(table[key].getCnt() == pass);
      DebugCode(lastKey = (void *) -1);
      DebugCode(lastIndex = -1);
      DebugCode(nsearch++;);
      DebugCode(tries += step);
      return (table[key].getValue());
    } else {
      if (ikey == 0) {
        ikey = ((m << slsBits) >> rsBits) | 0x1;
        Assert(ikey == incHashFunc(k));
        Assert(ikey < tableSize);
      }
      key -= ikey;
      if (key < 0)
        key += tableSize;
      DebugCode(step++;);
    }
  }
  Assert(0);
}

//
void AddressHashTableO1Reset::htAddLastNotFound(void *k, void *val)
{
  Assert(lastIndex != -1);
  Assert(k == lastKey);
  Assert(table[lastIndex].getCnt() < pass);

  //
  if (counter > percent) {
    resize();
    htAdd(k, val);
  } else {
    table[lastIndex].setKey(k);
    table[lastIndex].setValue(val);
    table[lastIndex].setCnt(pass);
    DebugCode(lastIndex = -1;);
    counter++;
  }
}

//
void AddressHashTableO1Reset::resize()
{
  int oldSize = tableSize;
  unsigned int oldPass = pass;
  AHT_HashNodeCnt* old = table;

  tableSize = tableSize * 2;
  bits++;
  mkTable();

  //
  for (int i = oldSize; i--; ) {
    if (old[i].getCnt() == oldPass)
      htAdd((old[i].getKey()), old[i].getValue());
  }

  //
  delete [] old;
}

//
#ifdef DEBUG_CHECK
void AddressHashTableO1Reset::print()
{
  for(int i = 0; i < tableSize; i++) {
    if (table[i].getCnt() == pass) {
      printf("table[%d] = <0x%x,0x%x>\n", i,
             (unsigned int) table[i].getKey(),
             (unsigned int) table[i].getValue());
    }
  }
  printStatistics();
}

void AddressHashTableO1Reset::printStatistics(int th)
{
  int misspl = 0;
  DebugCode(int sum = 0;);

  //
  for(int i = 0; i < tableSize; i++) {
    if (table[i].getCnt() == pass) {
      unsigned int pkey = primeHashFunc((table[i].getKey()));
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
