/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Peter van Roy (pvr@info.ucl.ac.be)
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Konstantin Popov, 2002
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

#ifndef __DICTIONARYHH
#define __DICTIONARYHH

#ifdef INTERFACE
#pragma interface
#endif

#include "value.hh"

//
// (Oz) Dictionaries
//

//
// Table sizes. These are primes stepped approximately 2/3 from each
// other, and choosen for best [division] performance (I have the
// utility that generated them).
//
#define HT_PRIMES_NUM		46
//
#define HT_PRIME0		1
#define HT_PRIME1		3
#define HT_PRIME2		5
#define HT_PRIME3		11
#define HT_PRIME4		23
#define HT_PRIME5		41
#define HT_PRIME6		71
#define HT_PRIME7		127
#define HT_PRIME8		191
#define HT_PRIME9		293
#define HT_PRIME10		461
#define HT_PRIME11		769
#define HT_PRIME12		1153
#define HT_PRIME13		1733
#define HT_PRIME14		2633
#define HT_PRIME15		4007
#define HT_PRIME16		6053
#define HT_PRIME17		9109
#define HT_PRIME18		13697
#define HT_PRIME19		20551
#define HT_PRIME20		30829
#define HT_PRIME21		46301
#define HT_PRIME22		69473
#define HT_PRIME23		104347
#define HT_PRIME24		156521
#define HT_PRIME25		234781
#define HT_PRIME26		352229
#define HT_PRIME27		528403
#define HT_PRIME28		792881
#define HT_PRIME29		1189637
#define HT_PRIME30		1784459
#define HT_PRIME31		2676727
#define HT_PRIME32		4015199
#define HT_PRIME33		6022873
#define HT_PRIME34		9034357
#define HT_PRIME35		13551589
#define HT_PRIME36		20327443
#define HT_PRIME37		30491239
#define HT_PRIME38		45736963
#define HT_PRIME39		68605463
#define HT_PRIME40		102908261
#define HT_PRIME41		154362469
#define HT_PRIME42		231543727
#define HT_PRIME43		347315603
#define HT_PRIME44		520973503
#define HT_PRIME45		781460413

//
// A DictNode holds either a key/value pair, or - in case of a
// collision - identifies a block of separately allocated DictNode"s;
class DictNode {
protected:
  OZ_Term key, value;

public:
  DictNode() : key(0) {
    DebugCode(value = (OZ_Term) -1;);
  }
  DictNode(OZ_Term keyIn, OZ_Term valueIn)
    : key(keyIn), value(valueIn) {}

  //
  void* operator new(size_t size) { Assert(0); return ((void *) 0); }
  void* operator new(size_t, void *place) { return (place); }

  //
  int isEmpty() { return (!key); }
  void setEmpty() {
    Assert(!isEmpty());
    key = (OZ_Term) 0;
    DebugCode(value = (OZ_Term) -1;);
  }
  void set(OZ_Term keyIn, OZ_Term valueIn) {
    Assert(isEmpty());
    key = keyIn;
    value = valueIn;
  }
  void setValue(OZ_Term valueIn) {
    Assert(!isEmpty());
    value = valueIn;
  }

  // If there is a collision, the DictNode in the hash table is
  // converted into a pointer to a chunk of separately allocated
  // DictNode"s. 'eptr' points at a cell after the array;
  void setSPtr(DictNode *sptr) {
    Assert(isRTAligned(sptr));
    key = ToInt32(sptr);
    DebugCode(value = (OZ_Term) -1;);
  }
  void setEPtr(DictNode *eptr) {
    Assert(isRTAligned(eptr));
    Assert(isPointer());
    value = ToInt32(eptr);
  }

  //
  int isPointer() { return (oz_isRef(key)); }
  DictNode *getDictNodeSPtr() {
    Assert(isPointer());
    return ((DictNode *) ToPointer(key));
  }
  DictNode *getDictNodeEPtr() {
    Assert(isPointer());
    return ((DictNode *) ToPointer(value));
  }

  //
  OZ_Term getKey() {
    Assert(!isPointer());
    return (key);
  }
  OZ_Term getValue() {
    Assert(!isPointer());
    return (value);
  }
};

//
extern unsigned int dictHTSizes[];


//
// DictHashTable is a custom one (unfortunately, I don't know how to
// generalize it). OZ_Term key/value pairs are kept directly in the
// table as long as there are no overflows. If any occurs, a
// contiguous block of DictNode"s is (re)allocated. The division
// hashing scheme is used (for the sake of perfection of hashing, in
// particular with consecutive integers as keys). Unfortunately, this
// is not as efficient to compute as the multiplication scheme (though
// on modern computers the difference is not as large as it used to
// be). Also, there is a method to copy a dictionary.
class DictHashTable {
  DictNode *table;
  // int tableSize;		// number of slots;
  int sizeIndex;		// in the 'dictHTSizes' array;
  int entries;			// number of allocated entries;
  // int slots;			// number of allocated slots;
  int maxEntries;		// boundaries for reallocation;
  // int maxSlots;

  //
private:
  void mkEmpty();
  void init(int size);
  void resize();

  //
  unsigned int hash(unsigned int m) {
    switch (sizeIndex) {
    case 0:	m = m % HT_PRIME0; break;
    case 1:	m = m % HT_PRIME1; break;
    case 2:	m = m % HT_PRIME2; break;
    case 3:	m = m % HT_PRIME3; break;
    case 4:	m = m % HT_PRIME4; break;
    case 5:	m = m % HT_PRIME5; break;
    case 6:	m = m % HT_PRIME6; break;
    case 7:	m = m % HT_PRIME7; break;
    case 8:	m = m % HT_PRIME8; break;
    case 9:	m = m % HT_PRIME9; break;
    case 10:	m = m % HT_PRIME10; break;
    case 11:	m = m % HT_PRIME11; break;
    case 12:	m = m % HT_PRIME12; break;
    case 13:	m = m % HT_PRIME13; break;
    case 14:	m = m % HT_PRIME14; break;
    case 15:	m = m % HT_PRIME15; break;
    case 16:	m = m % HT_PRIME16; break;
    case 17:	m = m % HT_PRIME17; break;
    case 18:	m = m % HT_PRIME18; break;
    case 19:	m = m % HT_PRIME19; break;
    case 20:	m = m % HT_PRIME20; break;
    case 21:	m = m % HT_PRIME21; break;
    case 22:	m = m % HT_PRIME22; break;
    case 23:	m = m % HT_PRIME23; break;
    case 24:	m = m % HT_PRIME24; break;
    case 25:	m = m % HT_PRIME25; break;
    case 26:	m = m % HT_PRIME26; break;
    case 27:	m = m % HT_PRIME27; break;
    case 28:	m = m % HT_PRIME28; break;
    case 29:	m = m % HT_PRIME29; break;
    case 30:	m = m % HT_PRIME30; break;
    case 31:	m = m % HT_PRIME31; break;
    case 32:	m = m % HT_PRIME32; break;
    case 33:	m = m % HT_PRIME33; break;
    case 34:	m = m % HT_PRIME34; break;
    case 35:	m = m % HT_PRIME35; break;
    case 36:	m = m % HT_PRIME36; break;
    case 37:	m = m % HT_PRIME37; break;
    case 38:	m = m % HT_PRIME38; break;
    case 39:	m = m % HT_PRIME39; break;
    case 40:	m = m % HT_PRIME40; break;
    case 41:	m = m % HT_PRIME41; break;
    case 42:	m = m % HT_PRIME42; break;
    case 43:	m = m % HT_PRIME43; break;
    case 44:	m = m % HT_PRIME44; break;
    case 45:	m = m % HT_PRIME45; break;
    }
    return (m);
  }

  //
  // Just inserts the entry into the table - keeps 'entries'(/'slots'),
  // does not bother with resizing, all nodes given are unique etc.,
  // and "cac"s" that entry;
  void gCollectDictEntry(DictNode *n);
  void sCloneDictEntry(DictNode *n);

  // Return sorted list (with given tail) containing all features;
  OZ_Term getArityList(OZ_Term tail = AtomNil);

  //
public:
  USEHEAPMEMORY;

  DictHashTable(int size) { init(size); }
  ~DictHashTable() {
    delete table;
    DebugCode(table = (DictNode *) -1;);
    DebugCode(sizeIndex = -1;);
    DebugCode(entries = maxEntries = -1;);
  }

  //
  void compactify();

  // returns '0' if nothing is found;
  OZ_Term htFind(OZ_Term key) {
    DictNode *np = &table[hash(featureHash(key))];

    //
    if (!np->isPointer()) {
      Assert(!np->isEmpty());
      if (featureEq(np->getKey(), key))
	return (np->getValue());
      else
	return ((OZ_Term) 0);
    } else if (!np->isEmpty()) {
      Assert(np->isPointer());
      // Actually, there are at least two elements;
      DictNode *sptr = np->getDictNodeSPtr();
      DictNode *eptr = np->getDictNodeEPtr();
      do {
	if (featureEq(sptr->getKey(), key))
	  return (sptr->getValue());
	sptr++;
      } while (sptr < eptr);
    }
    return ((OZ_Term) 0);
  }

  //
  // OZ_Term htFindOutline(OZ_Term key);

  // 'htAdd()' inserts a node, or updates an already existing one;
  void htAdd(OZ_Term key, OZ_Term value) {
    Assert(value != (OZ_Term) 0);	// '0' is reserved;
    //
    DictNode *np = &table[hash(featureHash(key))];

    //
    if (np->isEmpty()) {
      np->set(key, value);
      Assert(!np->isEmpty());
      //
      Assert(entries >= 0);
      entries++;
      // slots++;
      if (entries > maxEntries /* || slots > maxSlots */ )
	resize();
      //
      Assert(htFind(key));
      return;

      //
    } else {
      if (!np->isPointer()) {
	if (featureEq(np->getKey(), key)) {
	  np->setValue(value);
	  //
	  Assert(htFind(key));
	  return;

	  //
	} else {
	  // a fresh collision - allocate a new block of DictNode"s;
	  DictNode *newA =
	    (DictNode *) oz_heapMalloc(2 * sizeof(DictNode));
	  (void) new (newA) DictNode(*np);
	  np->setSPtr(newA++);
	  (void) new (newA++) DictNode(key, value);
	  np->setEPtr(newA);

	  //
	  Assert(entries >= 0);
	  entries++;
	  if (entries > maxEntries)
	    resize();
	  //
	  Assert(htFind(key));
	  return;
	}
      } else {
	// check the array;
	// Actually, there are at least two elements;
	DictNode *sptr = np->getDictNodeSPtr();
	DictNode *eptr = np->getDictNodeEPtr();
	do {
	  if (featureEq(sptr->getKey(), key)) {
	    sptr->setValue(value);
	    Assert(htFind(key));
	    return;
	  }
	  sptr++;
	} while (sptr < eptr);

	// not found - add another one;
	sptr = np->getDictNodeSPtr();
	DictNode *newA = 
	  (DictNode *) oz_heapMalloc((((char *) eptr) - ((char *) sptr)) +
				     sizeof(DictNode));
	//
	np->setSPtr(newA);
	// at least two elements are copied;
	(void) new (newA++) DictNode(*sptr++);
	do {
	  (void) new (newA++) DictNode(*sptr++);
	} while (sptr < eptr);
	(void) new (newA++) DictNode(key, value);
	np->setEPtr(newA);

	//
	Assert(entries >= 0);
	entries++;
	if (entries > maxEntries)
	  resize();
	//
	Assert(htFind(key));
	return;
      }
    }
    Assert(0);
  }

  // 'htReAdd()' inserts a new node without any size updates/checks;
  void htReAdd(OZ_Term key, OZ_Term value) {
    Assert(!htFind(key));
    Assert(value != (OZ_Term) 0);	// '0' is reserved;
    //
    DictNode *np = &table[hash(featureHash(key))];

    //
    if (np->isEmpty()) {
      np->set(key, value);
      Assert(!np->isEmpty());
      //
      Assert(htFind(key));
      return;

      //
    } else {
      if (!np->isPointer()) {
	// a fresh collision - allocate a new block of DictNode"s;
	DictNode *newA =
	  (DictNode *) oz_heapMalloc(2 * sizeof(DictNode));
	(void) new (newA) DictNode(*np);
	np->setSPtr(newA++);
	(void) new (newA++) DictNode(key, value);
	np->setEPtr(newA);
	//
	Assert(htFind(key));
	return;

	//
      } else {
	// check the array;
	// Actually, there are at least two elements;
	DictNode *sptr = np->getDictNodeSPtr();
	DictNode *eptr = np->getDictNodeEPtr();
	DictNode *newA = 
	  (DictNode *) oz_heapMalloc((((char *) eptr) - ((char *) sptr)) +
				     sizeof(DictNode));
	//
	np->setSPtr(newA);
	// at least two elements are copied;
	(void) new (newA++) DictNode(*sptr++);
	do {
	  (void) new (newA++) DictNode(*sptr++);
	} while (sptr < eptr);
	(void) new (newA++) DictNode(key, value);
	np->setEPtr(newA);
	//
	Assert(htFind(key));
	return;
      }
    }
    Assert(0);
  }


  // Returns the old value, or '0' if key was not found;
  OZ_Term htExchange(OZ_Term key, OZ_Term value, bool createFlag) {
    Assert(value != (OZ_Term) 0);
    //
    DictNode *np = &table[hash(featureHash(key))];
    OZ_Term oldValue;

    //
    if (np->isEmpty()) {
      if (createFlag) {
	np->set(key, value);
	Assert(!np->isEmpty());

	//
	Assert(entries >= 0);
	entries++;
	// slots++;
	if (entries > maxEntries /* || slots > maxSlots */ )
	  resize();
	//
	Assert(htFind(key));
      }
      return ((OZ_Term) 0);

      //
    } else {
      if (!np->isPointer()) {
	if (featureEq(np->getKey(), key)) {
	  OZ_Term oldValue = np->getValue();
	  np->setValue(value);
	  //
	  Assert(htFind(key));
	  return (oldValue);

	  //
	} else {
	  if (createFlag) {
	    // a new colision;
	    DictNode *newA =
	      (DictNode *) oz_heapMalloc(2 * sizeof(DictNode));
	    (void) new (newA) DictNode(*np);
	    np->setSPtr(newA++);
	    (void) new (newA++) DictNode(key, value);
	    np->setEPtr(newA);

	    //
	    Assert(entries >= 0);
	    entries++;
	    if (entries > maxEntries)
	      resize();
	    //
	    Assert(htFind(key));
	  }
	  return ((OZ_Term) 0);
	}

	//
      } else {
	// check the array;
	// Actually, there are at least two elements;
	DictNode *sptr = np->getDictNodeSPtr();
	DictNode *eptr = np->getDictNodeEPtr();
	do {
	  if (featureEq(sptr->getKey(), key)) {
	    OZ_Term oldValue = sptr->getValue();
	    sptr->setValue(value);
	    Assert(htFind(key));
	    return (oldValue);
	  }
	  sptr++;
	} while (sptr < eptr);

	if (createFlag) {
	  // not found - add another one;
	  sptr = np->getDictNodeSPtr();
	  DictNode *newA = 
	    (DictNode *) oz_heapMalloc((((char *) eptr) - ((char *) sptr)) +
				       sizeof(DictNode));
	  //
	  np->setSPtr(newA);
	  // at least two elements are copied;
	  (void) new (newA++) DictNode(*sptr++);
	  do {
	    (void) new (newA++) DictNode(*sptr++);
	  } while (sptr < eptr);
	  (void) new (newA++) DictNode(key, value);
	  np->setEPtr(newA);

	  //
	  Assert(entries >= 0);
	  entries++;
	  if (entries > maxEntries)
	    resize();
	  //
	  Assert(htFind(key));
	}
	return ((OZ_Term) 0);
      }
    }
    Assert(0);
  }

  // 'htDel()' does nothing if no node matches;
  void htDel(OZ_Term key) {
    DictNode *np = &table[hash(featureHash(key))];

    //
    if (!np->isPointer()) {
      Assert(!np->isEmpty());
      if (featureEq(np->getKey(), key)) {
	np->setEmpty();
	entries--;
	Assert(entries >= 0);
	// slots--;
      }
    } else if (!np->isEmpty()) {
      Assert(np->isPointer());

      // Actually, there are at least two elements;
      DictNode *sptr = np->getDictNodeSPtr();
      DictNode *eptr = np->getDictNodeEPtr();
      do {
	if (featureEq(sptr->getKey(), key)) {
	  // Now, 'sptr' points at the entry to be removed;
	  DictNode *fptr = np->getDictNodeSPtr();
	  int bytes = ((char *) eptr) - ((char *) fptr);
	  if (bytes > 2 * sizeof(DictNode)) {
	    DictNode *newA =
	      (DictNode *) oz_heapMalloc(bytes - sizeof(DictNode));
	    //
	    np->setSPtr(newA);
	    while (fptr < sptr) 
	      (void) new (newA++) DictNode(*fptr++);
	    Assert(fptr == sptr && fptr < eptr);
	    fptr++;
	    while (fptr < eptr)
	      (void) new (newA++) DictNode(*fptr++);
	    np->setEPtr(newA);

	    //
	  } else {
	    // only one entry left;
	    DictNode *fptr = np->getDictNodeSPtr();
	    if (fptr == sptr)
	      fptr++;
	    DebugCode(np->setEmpty());
	    np->set(fptr->getKey(), fptr->getValue());
	  }
	  entries--;
	  Assert(entries >= 0);
	  break;
	}

	//
	sptr++;
      } while (sptr < eptr);
    }
    Assert(!htFind(key));
  }

  // 
  int getSize() { return (dictHTSizes[sizeIndex]); }
  int getUsed() { Assert(entries >= 0); return (entries); }

  // Allocate & return _unsorted_ list containing all features:
  OZ_Term getKeys();
  // Allocate & return _unsorted_ pair list
  OZ_Term getPairs();
  // Allocate & return _unsorted_ list containing all values mapped to:
  OZ_Term getItems();
  // Convert table to Literal, SRecord or LTuple
  OZ_Term toRecord(OZ_Term lbl);

  //
  // Marshaling support: returns an array of DictNode"s that contain
  // all the dictionary's elements.
  DictNode* getPairsInArray();

  //
  DictHashTable *copy();

  //
  DictHashTable* gCollect(void);
  DictHashTable* sClone(void);
};
  

//
#define DictDefaultSize 5
//
#define DictSafeFlag  1

class OzDictionary: public ConstTermWithHome {
  friend void ConstTerm::gCollectConstRecurse(void);
  friend void ConstTerm::sCloneConstRecurse(void);
private:
  DictHashTable *table;
  int dictFlags;
  // Perdio: safe dictionaries (i.e. those used
  // within objects) can be marshalled.

private:
  OzDictionary(Board *b, DictHashTable *t, int flagsIn)
    : ConstTermWithHome(b, Co_Dictionary), dictFlags(flagsIn) {
    table = t->copy();
  }

public:
  NO_DEFAULT_CONSTRUCTORS(OzDictionary)

  void init(int sz) {
    table = new DictHashTable(sz);
    dictFlags = 0;
  }
  OzDictionary(Board *b, int sz = DictDefaultSize)
    : ConstTermWithHome(b, Co_Dictionary) {
    init(sz);
  }

  OZ_Term getArg(OZ_Term key) {
    return (table->htFind(key));
  }

  OZ_Term member(OZ_Term key) { 
    OZ_Term found = table->htFind(key);
    return oz_bool(found != (OZ_Term) 0);
  }

  void setArg(OZ_Term key, OZ_Term value) { 
    table->htAdd(key, value);
  }

  // createFlag indicates whether the entry should be created if it
  // does not already exist
  bool exchange(OZ_Term key, OZ_Term new_val, OZ_Term& old_val, bool createFlag) {
    return ((old_val = table->htExchange(key, new_val, createFlag))!=0);
  }

  void remove(OZ_Term key) { table->htDel(key); }
  void removeAll()  { init(DictDefaultSize); }

  OZ_Term keys()  { return table->getKeys(); }
  OZ_Term pairs() { return table->getPairs(); }
  OZ_Term items() { return table->getItems(); }
  OZ_Term toRecord(OZ_Term lbl) { return table->toRecord(lbl); }

  int getSize()     { return (table->getUsed()); }
  Bool isEmpty()    { return (table->getUsed() == 0); }

  OZ_Term clone(Board * b) {
    OzDictionary *aux = new OzDictionary(b, table, dictFlags);
    return (makeTaggedConst(aux));
  }

  //
  Bool isSafeDict() { return (dictFlags & DictSafeFlag); }
  void markSafe()   { dictFlags |= DictSafeFlag; }
  //
  DictNode* pairsInArray() { return (table->getPairsInArray()); }

  OZPRINT;
};


inline
Bool oz_isDictionary(OZ_Term term)
{
  return oz_isConst(term) && tagged2Const(term)->getType() == Co_Dictionary;
}

inline
OzDictionary *tagged2Dictionary(OZ_Term term)
{
  Assert(oz_isDictionary(term));
  return (OzDictionary *) tagged2Const(term);
}

#endif
