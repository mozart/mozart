#ifndef __GENOFSVAR__H__
#define __GENOFSVAR__H__

#if defined(__GNUC__)
#pragma interface
#endif

#include "genvar.hh"
#include "tagged.hh"
#include "term.hh"
#include "records.hh"
#include "mem.hh"

// TODO
// 1. Check that all TaggedRef's are dereferenced when they should be.

//-------------------------------------------------------------------------
//                           class PairList
//-------------------------------------------------------------------------

// A simple class implementing list of pairs of terms,
// used as a utility by DynamicTable and GenOFSVariable.


class Pair {
friend class PairList;

private:
  TaggedRef term1;
  TaggedRef term2;
  Pair* next;

  USEFREELISTMEMORY;

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
          freeListDispose(list, sizeof(*list));
          list=next;
      }
      freeListDispose(this, sizeof(*this)); // IS THIS REASONABLE?
  }
};


//-------------------------------------------------------------------------
//                               class DynamicTable
//-------------------------------------------------------------------------

// A class implementing a hash table that is automatically expanded when
// it becomes too full.  Several operations (merge, srecordcheck) are
// added to facilitate unification of GenOFSVariables, which are built on
// top of a DynamicTable.

typedef unsigned long dt_index;


class HashElement {
friend class DynamicTable;

private:
    TaggedRef ident;
    TaggedRef value;
};


// class DynamicTable uses:
//   TaggedRef SRecord::getFeature(Atom* feature)               (srecord.hh)
//   int Atom::hash()                                           (term.hh)
//   Atom* tagged2Atom(TaggedRef ref)                           (tagged.hh)
//   Bool isLiteral(TaggedRef term)                             (tagged.hh)
//   void* freeListMalloc(size_t chunk_size)                    (mem.hh)
//   void freeListDispose(void* addr, size_t chunk_size)        (mem.hh)
//   void *gcRealloc(void *ptr, size_t sz)                      (gc.cc)
//   void gcTagged(TaggedRef &fromTerm, TaggedRef &toTerm)      (gc.cc)

class DynamicTable {
friend class HashElement;

public:
    dt_index numelem;
    dt_index size;
    HashElement *table;

    USEHEAPMEMORY;

    // Copy the dynamictable from 'from' to 'to' space:
    void gc(void); // Definition in gc.cc

    // Create an initially empty dynamictable of size s (default 1)
    // (a future optimization: change the default to some more reasonable value)
    DynamicTable(dt_index s=1) {
        Assert(isPwrTwo(s));
        this->init(s);
    }

    // Initialize an elsewhere-allocated dynamictable
    void init(dt_index s=1) {
        Assert(isPwrTwo(s));
        numelem=0;
        size=s;
        table=(HashElement *)freeListMalloc(s*sizeof(HashElement));
        for (dt_index i=0; i<s; i++) table[i].ident=NULL;
    }

    // Create a copy of an existing dynamictable
    DynamicTable(DynamicTable &dt) {
        numelem=dt.numelem;
        size=dt.size;
        Assert(isPwrTwo(size));
        Assert(numelem<size);
        Assert(size>0);
        table=(HashElement *)freeListMalloc(size*sizeof(HashElement));
        Assert(table!=NULL);
        for (dt_index i=0; i<size; i++) table[i]=dt.table[i];
    }

    // Insert val at index id
    // Return NULL if val is successfully inserted (id did not exist)
    // Return the value of the pre-existing element if id already exists
    // Test for and increase size of hash table if it becomes too full
    TaggedRef insert(TaggedRef id, TaggedRef val) {
        Assert(isPwrTwo(size));
        Assert(isLiteral(id));
        overflowTest();
        dt_index i=fullhash(id);
        Assert(i<size);
        if (table[i].ident!=NULL) {
            Assert(isLiteral(table[i].ident));
            // Ident exists already; return value & don't insert
            return table[i].value;
        } else {
            // Ident doesn't exist; insert value
            numelem++;
            Assert(numelem<size);
            table[i].ident=id;
            table[i].value=val;
            return NULL;
        }
    }

    // Look up val at index id
    // Return val if it is found
    // Return NULL if nothing is found
    TaggedRef lookup(TaggedRef id) {
        Assert(isPwrTwo(size));
        Assert(isLiteral(id));
        dt_index i=fullhash(id);
        Assert(i<size);
        if (table[i].ident==id) {
            Assert(isLiteral(table[i].ident));
            // Val is found
            return table[i].value;
        } else {
            // Val is not found
            return NULL;
        }
    }

    // Merge an external dynamictable into the current dynamictable
    // Return a pairlist containing all term pairs with the same feature
    void merge(DynamicTable &dt, PairList* &pairs) {
        Assert(isPwrTwo(size));
        pairs=new PairList();
        Assert(pairs->isempty());
        for (dt_index i=0; i<dt.size; i++) {
            if (dt.table[i].ident!=NULL) {
                Assert(isLiteral(dt.table[i].ident));
                TaggedRef val=insert(dt.table[i].ident, dt.table[i].value);
                if  (val!=NULL) {
                    // Two terms have this feature; don't insert
                    // Add the terms to the list of pairs:
                    pairs->addpair(val, dt.table[i].value);
                    Assert(!pairs->isempty());
                } else {
                    // Element successfully inserted
                }
            }
        }
    }

    // Check an srecord against the current dynamictable
    // Return TRUE if all elements of dynamictable exist in srecord.
    // Return FALSE if there exists element of dynamictable that is not in srecord.
    // If TRUE, collect pairs of corresponding elements of dynamictable and srecord.
    // If FALSE, pair list contains a well-terminated but meaningless list.
    // Neither the srecord nor the dynamictable is modified.
    Bool srecordcheck(SRecord &sr, PairList* &pairs) {
        Assert(isPwrTwo(size));
        pairs=new PairList();
        Assert(pairs->isempty());
        for (dt_index i=0; i<size; i++) {
            if (table[i].ident!=NULL) {
                Assert(isLiteral(table[i].ident));
                TaggedRef val=sr.getFeature(table[i].ident);
                if (val!=NULL) {
                    // Feature found in srecord; add corresponding terms to list of pairs:
                    pairs->addpair(val, table[i].value);
                    Assert (!pairs->isempty());
                } else {
                    // Feature not found in srecord; failure of unification
                    return FALSE;
                }
            }
        }
        return TRUE;
    }

private:
    /**** Utilities ****/

    // Test for and double size of internal hash table if necessary
    void overflowTest() {
        Assert(isPwrTwo(size));
        if (numelem>=(size>>1)) { // numelem>=(size>>1)+(size>>2)
            Assert(isPwrTwo(size<<1));
            DynamicTable* dt=new DynamicTable(size<<1);
            for(dt_index i=0; i<size; i++) {
                if (table[i].ident!=NULL) {
                    Assert(isLiteral(table[i].ident));
                    dt->insert(table[i].ident, table[i].value);
                }
            }
            // Make current table a reference to dt's table:
            freeListDispose(table, size*sizeof(HashElement));
            numelem=dt->numelem;
            size=dt->size;
            Assert(numelem<size);
            table=dt->table;
            Assert(table!=NULL);
        }
    }

    // Hash and rehash until the element or an empty slot is found
    // Returns index of slot; the slot is empty or contains the element
    dt_index fullhash(TaggedRef id) {
        Assert(isPwrTwo(size));
        Assert(isLiteral(id));
        // Function 'hash' may eventually return the atom's seqNumber (see term.hh):
        dt_index i=(size-1) & ((dt_index) (tagged2Atom(id)->hash()));
        dt_index s=1;
        // Rehash if necessary using semi-quadratic probing (quadratic is not covering)
        while(table[i].ident!=NULL && table[i].ident!=id) {
            i+=s;
            i&=(size-1);
            s++;
        }
        return i;
    }

    // Return true iff argument is a power of two:
    Bool isPwrTwo(dt_index s)
    {
        Assert(s>0);
        while ((s&1)==0) s=(s>>1);
        return (s==1);
    }
};


//-------------------------------------------------------------------------
//                           class GenOFSVariable
//-------------------------------------------------------------------------


class GenOFSVariable: public GenCVariable {

private:
    // Attention: This is the _record itself_, not a pointer to it:
    DynamicTable dynamictable;

public:
    GenOFSVariable(DynamicTable &dt, TaggedRef pn = AtomVoid)
    : GenCVariable(OFSVariable, pn) {
        dynamictable=dt;
    }

    GenOFSVariable(TaggedRef pn = AtomVoid)
    : GenCVariable(OFSVariable, pn) {
        dynamictable.init();
    }

    // Methods relevant for term copying (gc and solve)
    void gc(void);
    size_t getSize(void) { return sizeof(GenOFSVariable); }

    Bool unifyOFS(TaggedRef *, TaggedRef, TypeOfTerm,
                  TaggedRef *, TaggedRef, TypeOfTerm);

    // Return the feature value if feature exists, return NULL if it doesn't exist
    TaggedRef getFeatureValue(TaggedRef feature) {
        Assert(isLiteral(feature));
        return dynamictable.lookup(feature);
    }

    // Add the feature and its value
    // If the feature already exists, do not insert anything
    // Return TRUE if feature successfully inserted, FALSE otherwise
    Bool addFeatureValue(TaggedRef feature, TaggedRef term) {
        Assert(isLiteral(feature));
        TaggedRef prev=dynamictable.insert(feature,term);
        // (a future optimization: a second suspList only waiting on features)
        propagate(makeTaggedCVar(this), suspList, makeTaggedCVar(this), pc_propagator);
        return (prev==NULL);
    }

    int getSuspListLength(void) {
        // see suspension.{hh,cc}:
        return suspList->length();
    }

    int getNumOfFeatures(void) {
        return (int) dynamictable.numelem;
    }

    // These procedures exist as well in the class GenFDVariable,
    // but they are not needed in GenOFSVariable:

    // void propagate (needs no redefinition from GenCVariable version)
    // void relinkSuspList (needs no redefinition from GenCVariable version)
    // void becomesSmallIntAndPropagate (meaningless for ofs)
    // Bool valid (meaningless for ofs)
    // void setDom (meaningless for ofs)
    // FiniteDomain &getDom (meaningless for ofs)
};


Bool isGenOFSVar(TaggedRef term);
Bool isGenOFSVar(TaggedRef term, TypeOfTerm tag);
GenOFSVariable *tagged2GenOFSVar(TaggedRef term);

#endif
