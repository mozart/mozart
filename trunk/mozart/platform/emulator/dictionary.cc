/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Peter van Roy (pvr@info.ucl.ac.be)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
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
#pragma implementation "dictionary.hh"
#endif

#include "dictionary.hh"
#include "am.hh"
#include "ozostream.hh"
#include "builtins.hh"
#include "sort.hh"

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
    DynamicTable* ret = (DynamicTable *) oz_freeListMalloc(memSize);
    Assert(ret!=NULL);
    ret->init(s);
    return ret;
}

// Initialize an elsewhere-allocated dynamictable of size s
void DynamicTable::init(dt_index s) {
  Assert(isPwrTwo(s));
  numelem=0;
  size=s;
  for (dt_index i=s; i--; ) {
    table[i].ident=makeTaggedNULL();
    table[i].value=makeTaggedNULL();
  }
}

// Return a copy of the current table that has size newSize and all contents
// of the current table.  The current table's contents MUST fit in the copy!
// Normally, the copy is rehashed except when newSize==size.  If
// optcopyflag==FALSE, then rehash also this case.
DynamicTable* DynamicTable::copyDynamicTable(dt_index newSize) {
  if (newSize==(dt_index)(-1L)) 
    newSize=size;

  Assert(isPwrTwo(size));
  Assert(numelem<=fullFunc(size));
  Assert(numelem<=fullFunc(newSize));
  Assert(size!=(dt_index)(-1L));

  DynamicTable* ret;
  
  if (size==newSize) {
    // Optimize case where copy has same size as original:
    size_t memSize = DTBlockSize(size);
    ret = (DynamicTable *) oz_freeListMalloc(memSize);
    ret->numelem=numelem;
    ret->size=size;
    memcpy(ret->table, table, size * sizeof(table[0]));
  } else {
    ret=newDynamicTable(newSize);
    
    Bool valid;
    
    for(dt_index i=size; i--; ) {
      if (table[i].value!=makeTaggedNULL()) {
	Assert(oz_isFeature(table[i].ident));
	
	ret->insert(table[i].ident, table[i].value, &valid);
	Assert(valid);
      }
    }
  }
  return ret;
}



// Insert val at index id 
// If valid==FALSE then nothing has been done.
// Otherwise, return NULL if val is successfully inserted (id did not exist)
// or return the value of the pre-existing element if id already exists.
// ATTENTION: insert must only be done if the table has room for a new element.
// User should test for and increase size of hash table if it becomes too full.
TaggedRef DynamicTable::insert(TaggedRef id, TaggedRef val, Bool *valid) {
    Assert(isPwrTwo(size));
    Assert(oz_isFeature(id));
    Assert(!fullTest());
    dt_index i=fullhash(id);
    if (i==invalidIndex) {
      *valid = FALSE;
      return makeTaggedNULL();
    }
    *valid = TRUE;
    Assert(i<size);
    if (table[i].value!=makeTaggedNULL()) {
        Assert(oz_isFeature(table[i].ident));
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

// Destructively update index id with new value val, if index id already has a value
// Return TRUE if index id successfully updated, else FALSE
Bool DynamicTable::update(TaggedRef id, TaggedRef val) {
    Assert(isPwrTwo(size));
    Assert(oz_isFeature(id));
    dt_index i=fullhash(id);
    Assert(i==invalidIndex || i<size);
    if (i!=invalidIndex && table[i].value!=makeTaggedNULL()) {
        Assert(oz_isFeature(table[i].ident));
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
  Assert(oz_isFeature(id));
  dt_index i=fullhash(id);
  Assert(i==invalidIndex || i<size);
  if (i!=invalidIndex) {
    if (table[i].value==makeTaggedNULL()) {
      numelem++;
      table[i].ident=id;
    }
    Assert(featureEq(table[i].ident, id));
    table[i].value=val;
    return TRUE;
  } else {
    return FALSE;
  }
}

Bool DynamicTable::addCond(TaggedRef id, TaggedRef val) 
{
  Assert(isPwrTwo(size));
  Assert(oz_isFeature(id));
  dt_index i=fullhash(id);
  Assert(i==invalidIndex || i<size);
  if (i!=invalidIndex) {
    if (table[i].value==makeTaggedNULL()) {
      numelem++;
      table[i].value=val;
    }
    Assert(table[i].ident == id);
    return TRUE;
  } else {
    return FALSE;
  }
}

Bool DynamicTable::exchange(TaggedRef id, TaggedRef new_val, 
			    TaggedRef * old_val) {
  Assert(isPwrTwo(size));
  Assert(oz_isFeature(id));
  dt_index i=fullhash(id);
  Assert(i==invalidIndex || i<size);
  if (i!=invalidIndex) {
    TaggedRef ov = table[i].value;
    
    if (ov==makeTaggedNULL()) {
      numelem++;
      *old_val = makeTaggedNULL();
      table[i].ident = id;
    } else {
      *old_val = ov;
    }
    Assert(table[i].ident == id);
    table[i].value = new_val;
    return TRUE;
  } else {
    return FALSE;
  }
}

// Remove index id from table.  Reclaim memory: if the table becomes too sparse then
// return a smaller table that contains all its entries.  Otherwise, return same table.
DynamicTable *DynamicTable::remove(TaggedRef id) {
    Assert(isPwrTwo(size));
    Assert(oz_isFeature(id));
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

  for (dt_index i=dt->size; i--; ) {

    if (dt->table[i].value!=makeTaggedNULL()) {
      Assert(oz_isFeature(dt->table[i].ident));
      
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

      Assert(oz_isFeature(table[i].ident));

      if (dt->fullTest()) 
	resizeDynamicTable(dt);
      
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

  for (dt_index i=size; i--; ) {
    
    if (table[i].value!=makeTaggedNULL()) {
      Assert(oz_isFeature(table[i].ident));

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

TaggedRef DynamicTable::getOpenArityList(TaggedRef* ftail, Board* home)
{
    TaggedRef thehead = oz_newVariable(home);
    TaggedRef thetail = thehead;
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
  while (oz_isCons(arity)) {
    TaggedRef feat=oz_head(arity);
    if (!lookup(feat)) {
      flist=oz_cons(feat,flist);
    }
    arity=oz_tail(arity);
  }
  return flist;
}

class Order_TaggedRef_By_Feat {
public:
  Bool operator()(const TaggedRef& a, const TaggedRef& b) {
    return featureCmp(a, b) <= 0;
  }
};

// Allocate & return sorted list containing all features:
// Takes optional tail as input argument.
TaggedRef DynamicTable::getArityList(TaggedRef tail) {
  TaggedRef arity=tail;
  if (numelem>0) {
    // put elements into array
    NEW_TEMP_ARRAY(TaggedRef, arr, numelem);
    for (int ai=0,di=0; di<size; di++)
      if (table[di].value!=makeTaggedNULL()) {
	Assert(oz_isFeature(table[di].ident));
	arr[ai++] = table[di].ident;
      }
    Order_TaggedRef_By_Feat lt;
    fastsort(arr, numelem, lt);

    for (int i = numelem; i--; )
      arity=oz_cons(arr[i],arity);
  }
  return arity;
}

// Return _unsorted_ list containing all features:
TaggedRef DynamicTable::getKeys() 
{
  TaggedRef arity=AtomNil;
  for (int di=0; di<size; di++) {
    if (table[di].value!=makeTaggedNULL()) {
      Assert(oz_isFeature(table[di].ident));
      arity=oz_cons(table[di].ident,arity);
    }
  }
  return arity;
}


TaggedRef DynamicTable::getPairs() {
  TaggedRef arity=AtomNil;
  for (int di=0; di<size; di++) {
    if (table[di].value!=makeTaggedNULL()) {
      Assert(oz_isFeature(table[di].ident));
      SRecord *sr = SRecord::newSRecord(AtomPair,2);
      sr->setArg(0, table[di].ident);
      sr->setArg(1, table[di].value);
      arity=oz_cons(makeTaggedSRecord(sr),arity);
    }
  }
  return arity;
}


TaggedRef DynamicTable::getItems() 
{
  TaggedRef items=AtomNil;
  for (int di=0; di<size; di++) {
    if (table[di].value!=makeTaggedNULL()) {
      Assert(oz_isFeature(table[di].ident));
      items=oz_cons(table[di].value,items);
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
            if (!oz_isSmallInt(feat)) return TRUE;
	    if (tagged2SmallInt(feat)>tupleArity) return TRUE;
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
    TaggedRef alist=getArityList(oz_nil());
    Arity *arity=aritytable.find(alist);
    SRecord *newrec = SRecord::newSRecord(lbl,arity);
    for (dt_index i=size; i--; ) {
      if (table[i].value!=makeTaggedNULL()) {
	Bool ok=newrec->setFeature(table[i].ident,table[i].value);
	Assert(ok);
      }
    }
    return newrec->normalize();
  }
}

OZ_Term registry_get(OZ_Term k)
{
  OZ_Term v;
  return (tagged2Dictionary(system_registry)->getArg(k,v)==PROCEED)?v:0;
}

void registry_put(OZ_Term k,OZ_Term v)
{
  tagged2Dictionary(system_registry)->setArg(k,v);
}

ostream &DynamicTable::newprint(ostream &out, int depth)
{
  // Count Atoms & Names in dynamictable:
  OZ_Term tmplit,tmpval;
  dt_index di;
  long ai;
  long nAtomOrInt=0;
  long nName=0;
  for (di=0; di<size; di++) {
    tmplit=table[di].ident;
    tmpval=table[di].value;
    if (tmpval) { 
      if (oz_isAtom(tmplit)||oz_isInt(tmplit)) nAtomOrInt++; else nName++;
    }
  }
  // Allocate array on heap, put Atoms in array:
  OZ_Term *arr = new OZ_Term[nAtomOrInt+1]; // +1 since nAtomOrInt may be zero
  for (ai=0,di=0; di<size; di++) {
    tmplit=table[di].ident;
    tmpval=table[di].value;
    if (tmpval!=makeTaggedNULL() && (oz_isAtom(tmplit)||oz_isInt(tmplit)))
      arr[ai++]=tmplit;
  }
  // Sort the Atoms according to printName:
  Order_TaggedRef_By_Feat lt;
  fastsort(arr, nAtomOrInt, lt);

  // Output the Atoms first, in order:
  for (ai=0; ai<nAtomOrInt; ai++) {
    oz_printStream(arr[ai],out,0,0);
    out << ':';
    oz_printStream(lookup(arr[ai]),out,depth,0);
    out << ' ';
  }
  // Output the Names last, unordered:
  for (di=0; di<size; di++) {
    tmplit=table[di].ident;
    tmpval=table[di].value;
    if (tmpval!=makeTaggedNULL() && !(oz_isAtom(tmplit)||oz_isInt(tmplit))) {
      oz_printStream(tmplit,out,0,0);
      out << ':';
      oz_printStream(tmpval,out,depth,0);
      out << ' ';
    }
  }
  // Deallocate array:
  delete arr;
  return out;
}

/*
 * Builtins
 */

OZ_BI_define(BIdictionaryNew,0,1)
{
  OZ_RETURN(makeTaggedConst(new OzDictionary(oz_currentBoard())));
} OZ_BI_end

OZ_BI_define(BIdictionaryKeys,1,1)
{
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->keys());
} OZ_BI_end


OZ_BI_define(BIdictionaryMarkSafe,1,0)
{
  oz_declareDictionaryIN(0,dict);
  dict->markSafe();
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIdictionaryEntries,1,1)
{
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->pairs());
} OZ_BI_end


OZ_BI_define(BIdictionaryItems,1,1)
{
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->items());
} OZ_BI_end


OZ_BI_define(BIdictionaryClone,1,1)
{
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->clone(oz_currentBoard()));
} OZ_BI_end


OZ_BI_define(BIdictionaryToRecord,2,1) {
  oz_declareNonvarIN(0, label);

  if (!oz_isLiteral(label)) 
    oz_typeError(0, "Literal");

  oz_declareDictionaryIN(1,dict);

  OZ_RETURN(dict->toRecord(label));
} OZ_BI_end


OZ_BI_define(BIdictionaryIsEmpty,1,1) {
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->isEmpty() ? oz_true() : oz_false());
} OZ_BI_end


OZ_Return isDictionaryInline(TaggedRef t, TaggedRef &out)
{
  NONVAR( t, term);
  out = oz_bool(oz_isDictionary(term));
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEFUN1(BIisDictionary,isDictionaryInline)


#define GetDictAndKey(d,k,dict,key,checkboard)			\
  NONVAR(d,dictaux);						\
  NONVAR(k,key);						\
  if (!oz_isDictionary(dictaux)) { oz_typeError(0,"Dictionary"); }	\
  if (!oz_isFeature(key))        { oz_typeError(1,"feature"); }	\
  OzDictionary *dict = tagged2Dictionary(dictaux);		\
  if (checkboard) { CheckLocalBoard(dict,"dict"); }


OZ_Return dictionaryMemberInline(TaggedRef d, TaggedRef k, TaggedRef &out)
{
  GetDictAndKey(d,k,dict,key,NO);
  out = dict->member(key);
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEFUN2(BIdictionaryMember,dictionaryMemberInline)

OZ_Return dictionaryGetInline(TaggedRef d, TaggedRef k, TaggedRef &out)
{
  GetDictAndKey(d,k,dict,key,NO);
  if (dict->getArg(key,out) != PROCEED) {
    return oz_raise(E_SYSTEM,E_KERNEL,"dict",2,d,k);
  }
  return PROCEED;
}

OZ_DECLAREBI_USEINLINEFUN2(BIdictionaryGet,dictionaryGetInline)


OZ_Return dictionaryCondGetInline(TaggedRef d, TaggedRef k, TaggedRef deflt, TaggedRef &out)
{
  GetDictAndKey(d,k,dict,key,NO);
  if (dict->getArg(key,out) != PROCEED) {
    out = deflt;
  }
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEFUN3(BIdictionaryCondGet,dictionaryCondGetInline)

OZ_Return dictionaryPutInline(TaggedRef d, TaggedRef k, TaggedRef value)
{
  GetDictAndKey(d,k,dict,key,OK);
  dict->setArg(key,value);
  return PROCEED;
}

OZ_DECLAREBI_USEINLINEREL3(BIdictionaryPut,dictionaryPutInline)

OZ_Return dictionaryRemoveInline(TaggedRef d, TaggedRef k)
{
  GetDictAndKey(d,k,dict,key,OK);
  dict->remove(key);
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEREL2(BIdictionaryRemove,dictionaryRemoveInline)


OZ_BI_define(BIdictionaryRemoveAll,1,0)
{
  oz_declareNonvarIN(0,dict);
  if (!oz_isDictionary(dict)) {
    oz_typeError(0,"Dictionary");
  }

  tagged2Dictionary(dict)->removeAll();
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIdictionaryWaitOr,1,1)
{
  oz_declareNonvarIN(0,td);
  if (!oz_isDictionary(td)) {
    oz_typeError(0,"Dictionary");
  }

  OzDictionary * dict = tagged2Dictionary(td);

  TaggedRef arity = oz_deref(dict->keys());

  while (!oz_isNil(arity)) {
    TaggedRef f = oz_deref(oz_head(arity)); 
    TaggedRef v;
    dict->getArg(f,v);

    DEREF(v,vPtr);
    Assert(!oz_isRef(v));
    if (!oz_isVarOrRef(v)) {
      am.emptySuspendVarList();
      OZ_RETURN(f);
    }
    (void) am.addSuspendVarListInline(vPtr);
    arity=oz_deref(oz_tail(arity));
  }
  
  return SUSPEND;
  
} OZ_BI_end

