/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Peter van Roy (pvr@info.ucl.ac.be)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Ralf Scheidhauer, 1997
 *    Peter Van Roy, 1997
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
#include "board.hh"

// TODO
// 1. Check that all TaggedRef's are dereferenced when they should be.

//-------------------------------------------------------------------------
//                           class PairList 
//-------------------------------------------------------------------------

// A simple class implementing list of pairs of terms,
// used as a utility by DynamicTable and OzOFVariable.


class Pair {
friend class PairList;

private:
  TaggedRef term1;
  TaggedRef term2;
  Pair* next;

  USEFREELISTMEMORY;

  NO_DEFAULT_CONSTRUCTORS(Pair)
  Pair(TaggedRef t1, TaggedRef t2, Pair* list) {
     term1=t1;
     term2=t2;
     next=list;
  }
};


class PairList {
friend class Pair;

private:
  Pair* list;

public:
  NO_DEFAULT_CONSTRUCTORS2(PairList)
  PairList() { list=NULL; }

  USEFREELISTMEMORY;

  // Add a pair to the head of a list
  void addpair(TaggedRef term1, TaggedRef term2) {
      list=new Pair(term1, term2, list);
  }

  // Get the first pair of a list
  // (Return FALSE if the list is empty)
  Bool getpair(TaggedRef &t1, TaggedRef &t2) {
      if (list!=NULL) {
	  t1=list->term1;
	  t2=list->term2;
	  return TRUE;
      } else {
	  return FALSE;
      }
  }

  // Advance to next element
  // (Return FALSE if the advance cannot be done because the list is empty)
  Bool nextpair() {
      if (list!=NULL) {
	  list=list->next;
	  return TRUE;
      } else {
	  return FALSE;
      }
  }

  Bool isempty() {
      return (list==NULL);
  }

  // Free memory for all elements of the list and the head
  void free() {
      while (list) {
	  Pair* next=list->next;
          oz_freeListDispose(list, sizeof(*list));
	  list=next;
      }
      oz_freeListDispose(this, sizeof(*this)); // IS THIS REASONABLE?
  }
};


//-------------------------------------------------------------------------
//                               class DynamicTable
//-------------------------------------------------------------------------

// A class implementing a hash table with insertion/deletion operations that
// can be expanded when it becomes too full and reduced when it becomes too empty
// (recovering memory), while keeping amortized constant-time insertion/deletion.
// Several operations (merge, srecordcheck) are added to facilitate unification of
// OzOFVariables, which are built on top of a DynamicTable.

// Three possibilities for an entry:
// ident: fea value: val   filled entry
// ident: fea value: 0     empty entry (emptied by removeC--a restricted empty slot)
// ident: 0   value: 0     empty entry (empty from the start--a fully empty slot)
// With valid flag, full emptiness check is: table[i].value==makeTaggedNULL()
// Hash termination condition is: table[i].ident==makeTaggedNULL() || table[i].ident==id || s==0

// This implementation needs to be tuned for space & time.  Currently, in the worst-case,
// a hash table may be doubled in size if it contains only FILLFACTOR/2 entries, if all
// other entries are restricted empty slots.  The hash table will then be reduced in size
// at the first remove operation, at the price of a large transient memory use.  However,
// this method guarantees amortized constant-time access.  If, in a steady-state, there
// is balance between the add and remove operations, then no extra space is used on average.

typedef long dt_index;

const dt_index invalidIndex = -1L;

// Maximum size of completely full table
#define FILLLIMIT 4
// Maximum fill factor of bigger tables
// (if changing this, must also change ofgenvar.cc)
#define FILLFACTOR 0.75

// Return true iff argument is a power of two
extern Bool isPwrTwo(dt_index s);
// Round up to nearest power of two
extern dt_index ceilPwrTwo(dt_index s);

class HashElement {
friend class DynamicTable;

private:
    TaggedRef ident;
    TaggedRef value;
};


// Maximum number of elements in hash table:
/* Full/Max: 0/0, 1/1, 2/2, 4/4, 6/8, 12/16, 24/32 (limit:75%) */
/* !!! DOING +2 instead of +1 goes into INFINITE LOOP.  CHECK IT OUT! */
// #define fullFunc(size) (((size)+((size)>>1)+1)>>1)
#define fullFunc(size) ((size)<=FILLLIMIT?(size):( (size) - ((size)>>2) ))

// Fill factor at which the hash table is considered sparse enough to halve in size:
/* Empty/Max: 0/0, 0/1, 1/2, 2/4, 3/8, 6/16, ... (limit:37.5%) */
#define emptyFunc(size) (((size)+((size)>>1)+2)>>2)

// class DynamicTable uses:
//   TaggedRef SRecord::getFeature(Literal* feature)		(srecord.hh)
//   int Literal::hash()					(term.hh)
//   Literal* tagged2Literal(TaggedRef ref)			(tagged.hh)
//   Bool isLiteral(TaggedRef term)				(tagged.hh)
//   void* heapMalloc(size_t chunk_size)	 		(mem.hh)

class DynamicTable {
friend class HashElement;

public:
    dt_index numelem;
    dt_index size;
    HashElement table[1]; // 1 == placeholder.  Actual size set when allocated.

private:
  // Hash and rehash until: (1) the element is found, (2) a fully empty slot is
  // found, or (3) the hash table has only filled slots and restricted empty slots
  // and does not contain the element.  In first two cases, answer is valid and
  // routine returns index of slot containing the element or a fully empty slot.
  // Otherwise returns "invalidIndex".
  // This hash routine works for completely full hash tables and hash tables with
  // restricted empty slotes (i.e., in which elements have been removed by making
  // their value NULL).
  dt_index DynamicTable::fullhash(TaggedRef id) 
  {
    Assert(isPwrTwo(size));
    Assert(oz_isFeature(id));
    // Function 'hash' may eventually return the literal's seqNumber (see value.hh):
    if (size==0) { return invalidIndex; }
    dt_index size1=(size-1);
    dt_index i=size1 & ((dt_index) featureHash(id));
    dt_index s=size1;
    // Rehash if necessary using semi-quadratic probing (quadratic is not covering)
    // Theorem: semi-quadratic probing is covering in size steps (proof: PVR+JN)
    
    while(1) {
      if (table[i].ident==makeTaggedNULL() ||
	  featureEq(table[i].ident,id))
	return i;
      if (s==0)      
	return invalidIndex;
      i+=s;
      i&=size1;
      s--;
    }
  }

public:
  USEFREELISTMEMORY;
  OZPRINT;
  NO_DEFAULT_CONSTRUCTORS(DynamicTable)

  // iteration
  int getFirst() { return -1; }
  int getNext(int i) {
    i++;
    while (i<size) {
      if (table[i].value!=makeTaggedNULL())
	return i;
      i++;
    }
    return -1;
  }

  TaggedRef getKey(int i)   { return table[i].ident; }
  TaggedRef getValue(int i) { return table[i].value; }
  void clearValue(int i) { table[i].value=0; }

  ostream &newprint(ostream &, int depth);

  // Copy the dynamictable from 'from' to 'to' space:
  DynamicTable * gCollect(void);
  DynamicTable * sClone(void);
  

  // Create an initially empty dynamictable of size s
  static DynamicTable* newDynamicTable(dt_index s=4);

  // Initialize an elsewhere-allocated dynamictable of size s
  void init(dt_index s);

  // True if the hash table is considered full:
  // Test whether the current table has too little room for one new element:
  // ATTENTION: Calls to insert should be preceded by fullTest.
  Bool fullTest() {
    Assert(isPwrTwo(size));
    return (numelem>=fullFunc(size));
  }
  
  DynamicTable* copyDynamicTable(dt_index newSize=(dt_index)(-1L));
  
  // Return a table that is double the size of the current table and
  // that contains the same elements:
  // ATTENTION: Should be called before insert if the table is full.
  DynamicTable* doubleDynamicTable() {
    return copyDynamicTable(size?(size<<1):1);
  }

  static
  size_t DTBlockSize(int s)
  {
    return sizeof(DynamicTable) + sizeof(HashElement)*(s-1);
  }

    void dispose() { oz_freeListDispose(this, DTBlockSize(size)); } 
    // Insert val at index id 
    // If valid==FALSE then nothing has been done.
    // Otherwise, return NULL if val is successfully inserted (id did not exist) 
    // or return the value of the pre-existing element if id already exists.
    // ATTENTION: insert must only be done if the table has room for a new element.
    // User should test for and increase size of hash table if it becomes too full.
    TaggedRef insert(TaggedRef id, TaggedRef val, Bool *valid);

    // Look up val at index id
    // Return val if it is found
    // Return NULL if nothing is found
  TaggedRef lookup(TaggedRef id) {
    Assert(isPwrTwo(size));
    Assert(oz_isFeature(id));
    dt_index i=fullhash(id);
    Assert(i==invalidIndex || i<size);
    if (i!=invalidIndex &&
	table[i].value!=makeTaggedNULL() &&
	featureEq(table[i].ident,id)) {
      // Val is found
      return table[i].value;
    } else {
      // Val is not found
      return makeTaggedNULL();
    }
  }


    // Destructively update index id with new value val, if index id already has a value
    // Return TRUE if index id successfully updated, else FALSE if index id does not
    // exist in table
    Bool update(TaggedRef id, TaggedRef val);

  // Destructively update index id with new value val even if id does 
  // not have a value yet
  // Return TRUE if index id successfully updated, else FALSE
  Bool add(TaggedRef id, TaggedRef val);
  Bool addCond(TaggedRef id, TaggedRef val);
  Bool exchange(TaggedRef id, TaggedRef new_val, TaggedRef * old_val);

    // Remove index id from table.  To reclaim memory, if the table becomes too sparse then
    // return a smaller table that contains all its entries.  Otherwise, return same table.
    DynamicTable *remove(TaggedRef id);

    // Return TRUE iff there are features in an external dynamictable that
    // are not in the current dynamictable
    Bool extraFeaturesIn(DynamicTable* dt);

    // Merge the current dynamictable into an external dynamictable
    // Return a pairlist containing all term pairs with the same feature
    // The external dynamictable is resized if necessary
    void merge(DynamicTable* &dt, PairList* &pairs);

    // Check an srecord against the current dynamictable
    // Return TRUE if all elements of dynamictable exist in srecord.
    // Return FALSE if there exists element of dynamictable that is not in srecord.
    // If TRUE, collect pairs of corresponding elements of dynamictable and srecord.
    // If FALSE, pair list contains a well-terminated but meaningless list.
    // Neither the srecord nor the dynamictable is modified.
    Bool srecordcheck(SRecord &sr, PairList* &pairs);

    // Return a difference list of all the features currently in the dynamic table.
    // The head is the return value and the tail is returned through an argument.
    TaggedRef getOpenArityList(TaggedRef*,Board*);

    // Return list of features in current table that are not in dt
    TaggedRef extraFeatures(DynamicTable* &dt);

    // Return list of features in srecord that are not in current table
    TaggedRef extraSRecFeatures(SRecord &sr);

    // Return TRUE if current table has features that are not in arity argument:
    Bool hasExtraFeatures(int tupleArity);
    Bool hasExtraFeatures(Arity *recordArity);

    // Return sorted list (with given tail) containing all features
    TaggedRef getArityList(TaggedRef tail=AtomNil);

    // Allocate & return _unsorted_ list containing all features:
    TaggedRef getKeys();

    // Allocate & return _unsorted_ pair list
    TaggedRef getPairs();

    // Allocate & return _unsorted_ list containing all values mapped to:
    TaggedRef getItems();

    // Convert table to Literal, SRecord or LTuple
    TaggedRef toRecord(TaggedRef lbl);

};


inline
void resizeDynamicTable(DynamicTable *&dt) 
{
  DynamicTable *ret = dt->doubleDynamicTable();
  dt->dispose();
  dt = ret;
}


/*===================================================================
 * Dictionaries
 *=================================================================== */


#define DictDefaultSize 4

#define DictSafeFlag  1

class OzDictionary: public ConstTermWithHome {
  friend void ConstTerm::gCollectConstRecurse(void);
  friend void ConstTerm::sCloneConstRecurse(void);
private:
  DynamicTable *table;
  int dictFlags;
  // Perdio: safe dictionaries (i.e. those used
  // within objects) can be marshalled.

public:
  NO_DEFAULT_CONSTRUCTORS(OzDictionary)
  void init(int sz)
  {
    if (sz<0)
      sz = DictDefaultSize;
    table = DynamicTable::newDynamicTable(sz);
    dictFlags = 0;
  }
  OzDictionary(Board *b, int sz=DictDefaultSize) : ConstTermWithHome(b,Co_Dictionary) 
  {
    init(sz);
  }
  OzDictionary(Board *b, DynamicTable *t) : ConstTermWithHome(b,Co_Dictionary)
  {
    table = t->copyDynamicTable();
    dictFlags = 0;
  }

  OZ_Return getArg(TaggedRef key, TaggedRef &out) { 
    TaggedRef ret = table->lookup(key);
    if (ret == makeTaggedNULL())
      return FAILED;
    out = ret;
    return PROCEED;
  }

  TaggedRef member(TaggedRef key) { 
    TaggedRef found = table->lookup(key);
    return oz_bool(found != makeTaggedNULL());
  }

  void setArg(TaggedRef key, TaggedRef value) { 
    if (table->fullTest()) resizeDynamicTable(table);
    Bool valid=table->add(key,value);
    if (!valid) {
      resizeDynamicTable(table);
      valid = table->add(key,value);
    }
    Assert(valid);
  }

  void exchange(TaggedRef key, TaggedRef new_val, TaggedRef * old_val) { 
    if (table->fullTest()) 
      resizeDynamicTable(table);

    Bool valid=table->exchange(key,new_val,old_val);

    if (!valid) {
      resizeDynamicTable(table);
      valid = table->exchange(key,new_val,old_val);
    }

    Assert(valid);
  }

  void setCondArg(TaggedRef key, TaggedRef value) {
    if (table->fullTest()) 
      resizeDynamicTable(table);

    Bool valid=table->addCond(key,value);
    
    if (!valid) {
      resizeDynamicTable(table);
      valid = table->addCond(key,value);
    }
    Assert(valid);
  }

  void remove(TaggedRef key) { 
    DynamicTable *aux = table->remove(key); 
    if (aux!=table) {
      table->dispose();
      table = aux;
    }
  }
  void removeAll()           { table->dispose(); init(-1); }

  TaggedRef keys()  { return table->getKeys(); }
  TaggedRef pairs() { return table->getPairs(); }
  TaggedRef items() { return table->getItems(); }

  Bool isSafeDict() { return (dictFlags & DictSafeFlag); }
  void markSafe()   { dictFlags=DictSafeFlag; }

  int getSize()     { return table->numelem; }

  Bool isEmpty() {
    return (table->numelem == 0);
  }

  TaggedRef clone(Board * b) {
    OzDictionary *aux = new OzDictionary(b, table);
    aux->dictFlags = dictFlags;
    return makeTaggedConst(aux);
  }

  TaggedRef toRecord(TaggedRef lbl) { return table->toRecord(lbl); }

  // iteration
  int getFirst()            { return table->getFirst(); }
  int getNext(int i)        { return table->getNext(i); }
  TaggedRef getKey(int i)   { return table->getKey(i); }
  TaggedRef getValue(int i) { return table->getValue(i); }

  OZPRINT;
};


inline
Bool oz_isDictionary(TaggedRef term)
{
  return oz_isConst(term) && tagged2Const(term)->getType() == Co_Dictionary;
}

inline
OzDictionary *tagged2Dictionary(TaggedRef term)
{
  Assert(oz_isDictionary(term));
  return (OzDictionary *) tagged2Const(term);
}

#endif
