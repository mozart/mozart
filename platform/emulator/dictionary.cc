/*
 * FBPS Saarbr"ucken
 * Author: mehl
 * Last modified: $Date$ from $Author$
 * Version: $Revision$
 * State: $State$
 */

#if defined(INTERFACE)
#pragma implementation "dictionary.hh"
#endif

#include "am.hh"
#include "dictionary.hh"


// Return true iff argument is zero or a power of two
Bool isPwrTwo(dt_index s) {
    return (s & (s-1))==0;
    // while ((s&1)==0) s=(s>>1); return (s==1);
}

// Return zero or the least power of two greater or equal to s:
dt_index ceilPwrTwo(dt_index s) 
{
  int ret = 1;
  while (ret < s) {
    ret = ret+ret;
  }
  return ret;
}

//-------------------------------------------------------------------------
//                               for class DynamicTable
//-------------------------------------------------------------------------

// Create an initially empty dynamictable of size s 
DynamicTable* DynamicTable::newDynamicTable(dt_index s) {
    s = nextPowerOf2(s);
    size_t memSize = DTBlockSize(s);
    DynamicTable* ret = (DynamicTable *) freeListMalloc(memSize);
    Assert(ret!=NULL);
    ret->init(s);
    return ret;
}

// Initialize an elsewhere-allocated dynamictable of size s
void DynamicTable::init(dt_index s) {
    Assert(isPwrTwo(s));
    numelem=0;
    size=s;
    for (dt_index i=0; i<s; i++) {
        table[i].ident=makeTaggedNULL();
        table[i].value=makeTaggedNULL();
    }
}

// Return a copy of the current table that has size newSize and all contents
// of the current table.  The current table's contents MUST fit in the copy!
// Normally, the copy is rehashed except when newSize==size.  If
// optcopyflag==FALSE, then rehash also this case.
DynamicTable* DynamicTable::copyDynamicTable(dt_index newSize) {
    if (newSize==(dt_index)(-1L)) newSize=size;
    Assert(isPwrTwo(size));
    Assert(numelem<=fullFunc(size));
    Assert(numelem<=fullFunc(newSize));
    Assert(size!=(dt_index)(-1L));
    DynamicTable* ret;
    if (size==newSize) {
        // Optimize case where copy has same size as original:
        size_t memSize = DTBlockSize(size);
        ret = (DynamicTable *) freeListMalloc(memSize);
        ret->numelem=numelem;
        ret->size=size;
        for (dt_index i=0; i<ret->size; i++) ret->table[i]=table[i];
    } else {
        ret=newDynamicTable(newSize);
        Bool valid;
        for(dt_index i=0; i<size; i++) {
            if (table[i].value!=makeTaggedNULL()) {
                Assert(isFeature(table[i].ident));
                ret->insert(table[i].ident, table[i].value, &valid);
                Assert(valid);
            }
        }
    }
    return ret;
}

// Hash and rehash until: (1) the element is found, (2) a fully empty slot is
// found, or (3) the hash table has only filled slots and restricted empty slots
// and does not contain the element.  In first two cases, answer is valid and
// routine returns index of slot containing the element or a fully empty slot.
// Otherwise returns "invalidIndex".
// This hash routine works for completely full hash tables and hash tables with
// restricted empty slotes (i.e., in which elements have been removed by making
// their value NULL).
inline
dt_index DynamicTable::fullhash(TaggedRef id) 
{
  Assert(isPwrTwo(size));
  Assert(isFeature(id));
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


// Insert val at index id 
// If valid==FALSE then nothing has been done.
// Otherwise, return NULL if val is successfully inserted (id did not exist)
// or return the value of the pre-existing element if id already exists.
// ATTENTION: insert must only be done if the table has room for a new element.
// User should test for and increase size of hash table if it becomes too full.
TaggedRef DynamicTable::insert(TaggedRef id, TaggedRef val, Bool *valid) {
    Assert(isPwrTwo(size));
    Assert(isFeature(id));
    Assert(!fullTest());
    dt_index i=fullhash(id);
    if (i==invalidIndex) {
      *valid = FALSE;
      return makeTaggedNULL();
    }
    *valid = TRUE;
    Assert(i<size);
    if (table[i].value!=makeTaggedNULL()) {
        Assert(isFeature(table[i].ident));
        // Ident exists already; return value & don't insert
        return table[i].value;
    } else {
        // Ident doesn't exist; insert value
        numelem++;
        Assert(numelem<=fullFunc(size));
        table[i].ident=id;
        table[i].value=val;
        return makeTaggedNULL();
    }
}

// Look up val at index id
// Return val if it is found
// Return NULL if nothing is found
TaggedRef DynamicTable::lookup(TaggedRef id) {
    Assert(isPwrTwo(size));
    Assert(isFeature(id));
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
// Return TRUE if index id successfully updated, else FALSE
Bool DynamicTable::update(TaggedRef id, TaggedRef val) {
    Assert(isPwrTwo(size));
    Assert(isFeature(id));
    dt_index i=fullhash(id);
    Assert(i==invalidIndex || i<size);
    if (i!=invalidIndex && table[i].value!=makeTaggedNULL()) {
        Assert(isFeature(table[i].ident));
        // Ident exists; update value & return TRUE:
        table[i].value=val;
        return TRUE;
    } else {
        // Ident doesn't exist; return FALSE:
        return FALSE;
    }
}

// Destructively update index id with new value val even if id does 
// not have a value yet
// Return TRUE if index id successfully updated, else FALSE
Bool DynamicTable::add(TaggedRef id, TaggedRef val) 
{
  Assert(isPwrTwo(size));
  Assert(isFeature(id));
  dt_index i=fullhash(id);
  Assert(i==invalidIndex || i<size);
  if (i!=invalidIndex) {
    if (table[i].value==makeTaggedNULL()) {
      numelem++;
    }
    table[i].ident=id;
    table[i].value=val;
    return TRUE;
  } else {
    return FALSE;
  }
}

// Remove index id from table.  Reclaim memory: if the table becomes too sparse then
// return a smaller table that contains all its entries.  Otherwise, return same table.
DynamicTable *DynamicTable::remove(TaggedRef id) {
    Assert(isPwrTwo(size));
    Assert(isFeature(id));
    dt_index i=fullhash(id);
    Assert(i==invalidIndex || i<size);
    DynamicTable* ret=this;
    if (i!=invalidIndex && table[i].value!=makeTaggedNULL()) {
        // Remove the element
        numelem--;
        table[i].value=makeTaggedNULL();
        // Shrink table if it becomes too sparse
        if (numelem<=emptyFunc(size) && size>0) {
            Assert(numelem<=fullFunc(size>>1));
            ret=copyDynamicTable(size>>1);
        }
    }
    return ret;
}

// Return TRUE iff there are features in an external dynamictable that
// are not in the current dynamictable
// This routine is currently not needed
Bool DynamicTable::extraFeaturesIn(DynamicTable* dt) {
    Assert(isPwrTwo(size));
    for (dt_index i=0; i<dt->size; i++) {
        if (dt->table[i].value!=makeTaggedNULL()) {
            Assert(isFeature(dt->table[i].ident));
            Bool exists=lookup(dt->table[i].ident);
            if (!exists) return TRUE;
        }
    }
    return FALSE;
}

/* LATER OPT: move fullFunc out of inner loop */
// Merge the current dynamictable into an external dynamictable
// Return a pairlist containing all term pairs with the same feature
// The external dynamictable is resized if necessary
void DynamicTable::merge(DynamicTable* &dt, PairList* &pairs) {
    Assert(isPwrTwo(size));
    pairs=new PairList();
    Assert(pairs->isempty());
    Bool valid;
    for (dt_index i=0; i<size; i++) {
        if (table[i].value!=makeTaggedNULL()) {
            Assert(isFeature(table[i].ident));
            if (dt->fullTest()) resizeDynamicTable(dt);
            TaggedRef val=dt->insert(table[i].ident, table[i].value, &valid);
            if (!valid) {
                resizeDynamicTable(dt);
                val=dt->insert(table[i].ident, table[i].value, &valid);
            }
            Assert(valid);
            if  (val!=makeTaggedNULL()) {
                // Two terms have this feature; don't insert
                // Add the terms to the list of pairs:
                pairs->addpair(val, table[i].value);
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
Bool DynamicTable::srecordcheck(SRecord &sr, PairList* &pairs) {
    Assert(isPwrTwo(size));
    pairs=new PairList();
    Assert(pairs->isempty());
    for (dt_index i=0; i<size; i++) {
        if (table[i].value!=makeTaggedNULL()) {
    	    Assert(isFeature(table[i].ident));
    	    TaggedRef val=sr.getFeature(table[i].ident);
    	    if (val!=makeTaggedNULL()) {
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

// Return a sorted difference list of all the features currently in the dynamic table.
// The head is the return value and the tail is returned through an argument.
TaggedRef DynamicTable::getOpenArityList(TaggedRef* ftail)
{
    return getOpenArityList(ftail,am.currentBoard);
}

TaggedRef DynamicTable::getOpenArityList(TaggedRef* ftail, Board* home)
{
    TaggedRef thehead=makeTaggedRef(newTaggedUVar(home));
    TaggedRef thetail=thehead;
    thehead=getArityList(thetail);
    *ftail = thetail;
    return thehead;
}

// Return list of features in current table that are not in dt:
TaggedRef DynamicTable::extraFeatures(DynamicTable* &dt) {
    TaggedRef flist=AtomNil;
    for (dt_index i=0; i<size; i++) {
        TaggedRef feat=table[i].ident;
        TaggedRef val=table[i].value;
        if (val!=makeTaggedNULL() && !dt->lookup(feat)) {
            flist=makeTaggedLTuple(new LTuple(feat,flist));
        }
    }
    return flist;
}

// Return list of features in srecord that are not in current table:
TaggedRef DynamicTable::extraSRecFeatures(SRecord &sr) {
    TaggedRef flist=AtomNil;
    TaggedRef arity=sr.getArityList();
    while (isCons(arity)) {
        TaggedRef feat=head(arity);
        if (!lookup(feat)) {
            flist=cons(feat,flist);
        }
        arity=tail(arity);
    }
    return flist;
}

// Allocate & return sorted list containing all features:
// Takes optional tail as input argument.
TaggedRef DynamicTable::getArityList(TaggedRef tail) {
    TaggedRef arity=tail;
    if (numelem>0) {
        SRecord *stuple=SRecord::newSRecord(AtomNil,numelem);
        TaggedRef *arr=stuple->getRef();
        for (int ai=0,di=0; di<size; di++) {
            if (table[di].value!=makeTaggedNULL()) {
    	       Assert(isFeature(table[di].ident));
               arr[ai] = table[di].ident;
               ai++;
            }
        }
        inplace_quicksort(arr, arr+(numelem-1));
        for (int i=numelem-1; i>=0; i--) {
           arity=cons(arr[i],arity);
        }
    }
    return arity;
}

// Return _unsorted_ list containing all features:
TaggedRef DynamicTable::getKeys() 
{
  TaggedRef arity=AtomNil;
  for (int di=0; di<size; di++) {
    if (table[di].value!=makeTaggedNULL()) {
      Assert(isFeature(table[di].ident));
      arity=cons(table[di].ident,arity);
    }
  }
  return arity;
}


TaggedRef DynamicTable::getPairs() {
  TaggedRef arity=AtomNil;
  for (int di=0; di<size; di++) {
    if (table[di].value!=makeTaggedNULL()) {
      Assert(isFeature(table[di].ident));
      SRecord *sr = SRecord::newSRecord(AtomPair,2);
      sr->setArg(0, table[di].ident);
      sr->setArg(1, table[di].value);
      arity=cons(makeTaggedSRecord(sr),arity);
    }
  }
  return arity;
}


TaggedRef DynamicTable::getItems() 
{
  TaggedRef items=AtomNil;
  for (int di=0; di<size; di++) {
    if (table[di].value!=makeTaggedNULL()) {
      Assert(isFeature(table[di].ident));
      items=cons(table[di].value,items);
    }
  }
  return items;
}


// Return TRUE if current table has features that are not in arity argument
Bool DynamicTable::hasExtraFeatures(int tupleArity) {
    TaggedRef feat;
    if (tupleArity==0) return (numelem!=0);
    for (dt_index i=0; i<size; i++) {
	if (table[i].value!=makeTaggedNULL()) {
            feat=table[i].ident;
            if (!isSmallInt(feat)) return TRUE;
	    if (smallIntValue(feat)>tupleArity) return TRUE;
	}
    }
    return FALSE;
}


// Return TRUE if current table has features that are not in arity argument
Bool DynamicTable::hasExtraFeatures(Arity *recordArity) {
    TaggedRef feat;
    for (dt_index i=0; i<size; i++) {
	if (table[i].value!=makeTaggedNULL()) {
            feat=table[i].ident;
	    if (recordArity->lookupInternal(feat)==(-1)) return TRUE;
	}
    }
    return FALSE;
}


// Convert dynamic table to Literal, SRecord, or LTuple:
TaggedRef DynamicTable::toRecord(TaggedRef lbl)
{
    if (numelem==0)
        return lbl;
    else {
        TaggedRef alist=getArityList();
        Arity *arity=aritytable.find(alist);
        SRecord *newrec = SRecord::newSRecord(lbl,arity);
	for (dt_index i=0; i<size; i++) {
	    if (table[i].value!=makeTaggedNULL()) {
		Bool ok=newrec->setFeature(table[i].ident,table[i].value);
		Assert(ok);
            }
	}
        return newrec->normalize();
    }
}

/*
 * inplace quicksort using featureCmp
 */

// Swap TaggedRef array elements:
inline void inplace_swap(TaggedRef* a, TaggedRef* b) {
  register TaggedRef aux = *a;
  *a = *b;
  *b = aux;
}

// In-place sort of an array of TaggedRef:
void inplace_quicksort(TaggedRef* first, TaggedRef* last) {
  register TaggedRef* i;
  register TaggedRef* j;

  if (first >= last)
    return;
  for (i = first, j = last; ; j--) {
    while (i != j && featureCmp(*i, *j) <= 0)
      j--;
    if (i == j)
      break;
    inplace_swap(i, j);
    do
      i++;
    while (i != j && featureCmp(*i, *j) <= 0);
    if (i == j)
      break;
    inplace_swap(i, j);
  } // for
  inplace_quicksort(first, i-1);
  inplace_quicksort(i+1, last);
}

