/*
 *  Authors:
 *    Peter van Roy (pvr@info.ucl.ac.be)
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
  
#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_of.hh"
#endif

#include "var_of.hh"
#include "ofs-prop.hh"
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
    return (featureCmp(a, b) <= 0);
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

//-------------------------------------------------------------------------
//                               for class OzOFVariable
//-------------------------------------------------------------------------

#ifdef OUTLINE
#define inline
#endif

/* Add list of features to each OFS-marked suspension list 'flist' has
 * three possible values: a single feature (literal or integer), a
 * nonempty list of features, or NULL (no extra features).
 * 'determined'==TRUE iff the unify makes the OFS determined.  'var'
 * (which must be deref'ed) is used to make sure that features are
 * added only to variables that are indeed waiting for features. This
 * routine is inspired by am.checkSuspensionList, and must track all
 * changes to it.  */
void addFeatOFSSuspensionList(TaggedRef var,
			      SuspList * suspList,
			      TaggedRef flist,
			      Bool determ)
{
  while (suspList) {
    Suspendable * susp = suspList->getSuspendable();

    // The added condition ' || thr->isRunnable () ' is incorrect
    // since isPropagated means only that the thread is runnable
    if (susp->isDead()) {
      suspList = suspList->getNext();
      continue;
    }

    if (susp->isOFS()) {
      MonitorArityPropagator * prop = 
	(MonitorArityPropagator *) SuspToPropagator(susp)->getPropagator();

      Assert(sizeof(MonitorArityPropagator) == prop->sizeOf());

      // Only add features if var and fvar are the same:
      TaggedRef fvar=prop->X;
      DEREF(fvar,_1);
      if (var!=fvar) {
	suspList=suspList->getNext();
	continue;
      }
      // Only add features if the 'kill' variable is undetermined:
      TaggedRef killl=prop->K;
      DEREF(killl,_);
      Assert(!oz_isRef(killl));
      if (!oz_isVarOrRef(killl)) {
	suspList=suspList->getNext();
	continue;
      }

      // Add the feature or list to the diff. list in FH and FT:
      if (flist) {
	if (oz_isFeature(flist))
	  prop->FH = oz_cons(flist,prop->FH);
	else {
	  // flist must be a list
	  Assert(oz_isCons(flist));
	  TaggedRef tmplist=flist;
	  while (tmplist!=AtomNil) {
	    prop->FH = oz_cons(oz_head(tmplist),prop->FH);
	    tmplist=oz_tail(tmplist);
	  }
	}
      }
      if (determ) {
	// FS is det.: tail of list must be bound to nil: (always succeeds)
	// Do *not* use unification to do this binding!
	TaggedRef tl=prop->FT;
	DEREF(tl,tailPtr);
	Assert(!oz_isRef(tl));
	if (oz_isVar(tl)) {
	  OzVariable *ov = tagged2Var(tl);
	  oz_bindVar(ov, tailPtr, AtomNil);
	} else {
	  Assert(tl==AtomNil);
	}
      }
    }

    suspList = suspList->getNext();
  }
}


// Check if there exists an S_ofs (Open Feature Structure) suspension
// in the suspList (Used only for monitorArity)
static
Bool hasOFSSuspension(SuspList * suspList)
{
  while (suspList) {
    Suspendable * susp = suspList->getSuspendable();
    
    if (!susp->isDead() && susp->isOFS()) 
      return TRUE;

    suspList = suspList->getNext();
  }
  return FALSE;
}

//
static inline TaggedRef ofvTrail(TaggedRef *vPtr)
{
  return (*vPtr);
}
static inline void ofvRestore(TaggedRef *vPtr, TaggedRef val)
{
  *vPtr = val;
}

//
OZ_Return OzOFVariable::bind(TaggedRef *vPtr, TaggedRef term)
{
  Assert(!oz_isRef(term));
  //
  TaggedRef bindInRecordCaseHack = term;
  TaggedRef var                  = *vPtr;

  if (oz_isLiteral(term)) {
    // Literals have no features:
    if (getWidth()>0) return FALSE;
    
    // Get local/global flag:
    Bool vLoc=oz_isLocalVar(this);

    OZ_Term tv = ofvTrail(vPtr);
    // Bind OFSVar to the Literal:
    if (vLoc) DoBind(vPtr, term);
    else DoBindAndTrail(vPtr, term);
    
    // Unify the labels:
    OZ_Return ret;
    if ((ret = oz_unify(term,label)) != PROCEED) {
      if (ret != FAILED)
	ofvRestore(vPtr, tv);
      return (ret);
    }
    
    // Update the OFS suspensions:
    if (vLoc) addFeatOFSSuspensionList(var,suspList,makeTaggedNULL(),TRUE);
    
    // Propagate changes to the suspensions:
    // (this routine is actually OzVariable::propagate)
    propagate(suspList, pc_cv_unif);
    
    // Take care of linking suspensions
    if (!vLoc) {
      // Add a suspension to the OZ_VAR_OF if it is global:
      // Suspension* susp=new Suspension(am.currentBoard);
      // Assert(susp!=NULL);
      // addSuspension(susp);
    }
    return TRUE;
  }

  if (oz_isLTuple(term)) {
    // Get the LTuple corresponding to term:
    LTuple* termLTup=tagged2LTuple(term);
    
    // Get local/global flag:
    Bool vLoc=oz_isLocalVar(this);
    
    // Check that var features are subset of {1,2}
    TaggedRef arg1=getFeatureValue(makeTaggedSmallInt(1));
    TaggedRef arg2=getFeatureValue(makeTaggedSmallInt(2));
    if ((arg1!=makeTaggedNULL())+(arg2!=makeTaggedNULL()) != getWidth())
      return FALSE;
    
    // Take care of OFS suspensions:
    if (vLoc && hasOFSSuspension(suspList)) {
      if (getWidth()<2) {
	// Calculate feature or list of features 'flist' that are
	// in LTUPLE and not in OFS.
	TaggedRef flist=AtomNil;
	if (!arg2) flist=oz_cons(makeTaggedSmallInt(2),flist);
	if (!arg1) flist=oz_cons(makeTaggedSmallInt(1),flist);
	// Add the extra features to S_ofs suspensions:
	addFeatOFSSuspensionList(var,suspList,flist,TRUE);
      } else {
	addFeatOFSSuspensionList(var,suspList,makeTaggedNULL(),TRUE);
      }
    }

    OZ_Term tv = ofvTrail(vPtr);
    // Bind OFSVar to the LTuple:
    if (vLoc) DoBind(vPtr, bindInRecordCaseHack);
    else DoBindAndTrail(vPtr, bindInRecordCaseHack);
    
    // Unify the labels:
    OZ_Return ret;
    if ((ret = oz_unify(AtomCons,label)) != PROCEED) {
      if (ret != FAILED)
	ofvRestore(vPtr, tv);
      return (ret);
    }
    
    // Unify corresponding feature values:
    if (arg1 && (ret = oz_unify(termLTup->getHead(),arg1)) != PROCEED) {
      if (ret != FAILED)
	ofvRestore(vPtr, tv);
      return (ret);
    }
    if (arg2 && (ret = oz_unify(termLTup->getTail(),arg2)) != PROCEED) {
      if (ret != FAILED)
	ofvRestore(vPtr, tv);
      return (ret);
    }
    
    // Propagate changes to the suspensions:
    // (this routine is actually OzVariable::propagate)
    propagate(suspList, pc_cv_unif);
    return TRUE;
  }

  if (oz_isSRecord(term)) {
    // For all features of var, term should contain the feature.
    // Unify the values of corresponding features.
    // If success, bind the var to the SRECORD (with local/global distinction).

    // Get the SRecord corresponding to term:
    SRecord* termSRec=tagged2SRecord(term);
    Assert(termSRec!=NULL);

    // Get local/global flag:
    Bool vLoc=oz_isLocalVar(this);
  
    // Check that all features of the OFSVar exist in the SRecord:
    // (During the check, calculate the list of feature pairs that correspond.)
    PairList* pairs;
    Bool success=dynamictable->srecordcheck(*termSRec, pairs);
    if (!success) { pairs->free(); return FALSE; }

    // Take care of OFS suspensions:
    if (vLoc && hasOFSSuspension(suspList)) {
      if (termSRec->getWidth()>getWidth()) {
	// Calculate feature or list of features 'flist' that are in SRECORD
	// and not in OFS.
	TaggedRef flist = dynamictable->extraSRecFeatures(*termSRec);
	// Add the extra features to S_ofs suspensions:
	addFeatOFSSuspensionList(var,suspList,flist,TRUE);
      } else {
	addFeatOFSSuspensionList(var,suspList,makeTaggedNULL(),TRUE);
      }
    }

    OZ_Term tv = ofvTrail(vPtr);
    // Bind OFSVar to the SRecord:
    if (vLoc) DoBind(vPtr, bindInRecordCaseHack);
    else DoBindAndTrail(vPtr, bindInRecordCaseHack);
  
    // Unify the labels:
    OZ_Return ret;
    if ((ret = oz_unify(termSRec->getLabel(),label)) != PROCEED) {
      pairs->free();
      if (ret != FAILED)
	ofvRestore(vPtr, tv);
      return (ret);
    }

    // Unify corresponding feature values:
    PairList* p = pairs;
    TaggedRef t1, t2;
    while (p->getpair(t1, t2)) {
      Assert(!p->isempty());
      if ((ret = oz_unify(t1, t2)) != PROCEED) {
	DebugCode(success=FALSE);
	break;
      }
      p->nextpair();
    }
    Assert(!success || p->isempty());
    pairs->free();
    if (ret != PROCEED) {
      if (ret != FAILED)
	ofvRestore(vPtr, tv);
      return (ret);
    }

    // At this point, binding is successful

    // Propagate changes to the suspensions:
    // (this routine is actually OzVariable::propagate)
    propagate(suspList, pc_cv_unif);

    // Take care of linking suspensions
    if (!vLoc) {
      // Add a suspension to the OZ_VAR_OF if it is global:
      // Suspension* susp=new Suspension(am.currentBoard);
      // Assert(susp!=NULL);
      // addSuspension(susp);
    }
    return TRUE;
  }
  return FALSE;
}

// (Arguments are dereferenced)
OZ_Return OzOFVariable::unify(TaggedRef *vPtr, TaggedRef *tPtr)
{
  // var - var unification
  TaggedRef var  = *vPtr;
  TaggedRef term = *tPtr;
  OzVariable *cv = tagged2Var(term);
  if (cv->getType()!=OZ_VAR_OF) {
    return FALSE;
  }

  Assert(*tPtr != *vPtr);

  // Get the OzOFVariable corresponding to term:
  OzOFVariable* termVar = (OzOFVariable *)cv;
  Assert(termVar != NULL);

  // Get local/global flags:
  Bool vLoc = oz_isLocalVar(this);
  Bool tLoc = oz_isLocalVar(termVar);
  //  
  // kost@ : as far as i can comprehend this piece of code, 'newVar'
  // has two usages: (a) in the "both local" case it references the
  // remaining variable, and (b) in all the cases the "new" var acts
  // as a "template" for the new feature table: first, "new" var's
  // table is copied and then extended with the "other" var's one.
  // Positively speaking, the word "new" is somewhat misleading here..
  OzOFVariable *newVar;			DebugCode(newVar = NULL;);
  OzOFVariable *otherVar;		DebugCode(otherVar = NULL;);
  TaggedRef *nvRefPtr;			DebugCode(nvRefPtr = NULL;);
  TaggedRef *otherPtr;			DebugCode(otherPtr = NULL;);
  //
  long varWidth           = getWidth();
  long termWidth          = termVar->getWidth();
  DynamicTable * dt       = NULL;

  if (vLoc && tLoc) {
    // Reuse the largest table (optimization to improve unification speed):
    if (varWidth > termWidth) {
      DEBUG_CONSTRAIN_VAR(("varWidth > termWidth\n"));
      newVar   = this;
      dt       = newVar->getTable();
      nvRefPtr = vPtr;
      otherVar = termVar; // otherVar must be smallest
      otherPtr = tPtr;
    } else {
      DEBUG_CONSTRAIN_VAR(("! (varWidth > termWidth)\n"));
      newVar   = termVar;
      dt       = newVar->getTable();
      nvRefPtr = tPtr;
      otherVar = this; // otherVar must be smallest
      otherPtr = vPtr;
    }
  } else if (vLoc && !tLoc) {
    // Reuse the var:
    newVar   = this;
    dt       = newVar->getTable();
    // no need to set 'nvRefPtr';
    otherVar = termVar;
  } else if (!vLoc && tLoc) {
    // Reuse the term:
    newVar   = termVar;
    dt       = newVar->getTable();
    // no need to set 'nvRefPtr';
    otherVar = this;
  } else if (!vLoc && !tLoc) {
    // Reuse the largest table (this improves unification speed):
    if (varWidth > termWidth) {
      DEBUG_CONSTRAIN_VAR(("varWidth > termWidth\n"));
      // Make a local copy of the var's DynamicTable:
      newVar   = this;
      dt       = newVar->getTable()->copyDynamicTable();
      // no need to set 'nvRefPtr';
      otherVar = termVar; // otherVar must be smallest
    } else {
      DEBUG_CONSTRAIN_VAR(("! (varWidth > termWidth)\n"));
      // Same as above, but in opposite order:
      newVar   = termVar;
      dt       = newVar->getTable()->copyDynamicTable();
      // no need to set 'nvRefPtr';
      otherVar = this; // otherVar must be smallest
    }
  } else Assert(FALSE);
  Assert(newVar != NULL);
  Assert(otherVar != NULL);
  Assert(dt != NULL);

  // Take care of OFS suspensions, part 1/2 (before merging tables):
  Bool vOk        = vLoc && hasOFSSuspension(suspList);
  TaggedRef vList = 0;
  if (vOk) {
    // Calculate the extra features in var:
    vList = termVar->dynamictable->extraFeatures(dynamictable);
  }
  Bool tOk        = tLoc && hasOFSSuspension(termVar->suspList);
  TaggedRef tList = 0;
  if (tOk) {
    // Calculate the extra features in term:
    tList = dynamictable->extraFeatures(termVar->dynamictable);
  }

  // Merge otherVar's DynamicTable into newVar's DynamicTable.
  // (During the merge, calculate the list of feature pairs that correspond.)
  PairList * pairs;
  otherVar->dynamictable->merge(dt, pairs);
  long mergeWidth = dt->numelem;
  // kost@ : do *not* f$ck with the newVar's feature table here:
  //         (a) newVar can be global, 
  //         (b) newVar can be bound (in a "something is non-local" case);
  // newVar->dynamictable = dt;

  // Take care of OFS suspensions, part 2/2 (after merging tables):
  if (vOk && (vList != AtomNil /*mergeWidth>termWidth*/)) {
    // Add the extra features to S_ofs suspensions:
    addFeatOFSSuspensionList(var, suspList, vList, FALSE);
  }

  if (tOk && (tList != AtomNil /*mergeWidth>varWidth*/)) {
    // Add the extra features to S_ofs suspensions:
    addFeatOFSSuspensionList(term, termVar->suspList, tList, FALSE);
  }

  // Bind both var and term to the (possibly reused) newVar:
  // Because of cycles, these bindings must be done _before_ the unification
  // If in glob/loc unification, the global is not constrained, then bind
  // the local to the global and relink the local's suspension list
  //
  // kost@ : in addition, the binding is trailed, and restored later
  // in case of suspension. Note this is done even when the variable
  // is global;
  OZ_Term* trailedVarPtr;
  OZ_Term trailedVar;

  if (vLoc && tLoc) {
    DEBUG_CONSTRAIN_VAR(("vLoc && tLoc\n"));
    Assert(otherPtr); 
    Assert(nvRefPtr);
    // kost@ : extend the target variable now;
    newVar->dynamictable = dt;
    trailedVarPtr = otherPtr;
    trailedVar = ofvTrail(trailedVarPtr);

    //
    bindLocalVar(otherPtr, nvRefPtr);
  } else if (vLoc && !tLoc) {
    DEBUG_CONSTRAIN_VAR(("vLoc && !tLoc\n"));
    // Global term is constrained if result has more features than term:
    if (mergeWidth > termWidth) {
      DEBUG_CONSTRAIN_VAR(("constrainGlobalVar\n"));
      constrainGlobalVar(tPtr, dt);
    }
    //
    trailedVarPtr = vPtr;
    trailedVar = ofvTrail(trailedVarPtr);

    //
    bindLocalVar(vPtr, tPtr);
  } else if (!vLoc && tLoc) {
    DEBUG_CONSTRAIN_VAR(("!vLoc && tLoc\n"));
    // Global var is constrained if result has more features than var:
    if (mergeWidth > varWidth) {
      DEBUG_CONSTRAIN_VAR(("constrainGlobalVar\n"));
      constrainGlobalVar(vPtr, dt);
    }
    //
    trailedVarPtr = tPtr;
    trailedVar = ofvTrail(trailedVarPtr);

    //
    bindLocalVar(tPtr, vPtr);
  } else if (!vLoc && !tLoc) {
    DEBUG_CONSTRAIN_VAR(("!vLoc && !tLoc\n"));
    // bind to new term with trailing:
    if (mergeWidth > varWidth) {
      DEBUG_CONSTRAIN_VAR(("constrainGlobalVar\n"));
      constrainGlobalVar(vPtr, dt);
    }
    //
    trailedVarPtr = tPtr;
    trailedVar = ofvTrail(trailedVarPtr);

    //
    bindGlobalVar(tPtr, vPtr);
  } else Assert(FALSE);

  // Unify the labels:
  OZ_Return ret;
  if ((ret = oz_unify(termVar->label, label)) != PROCEED) {
    pairs->free();
    if (ret != FAILED)
      ofvRestore(trailedVarPtr, trailedVar);
    return (ret);
  }
  // Must be literal or variable:
  TaggedRef tmp = label;
  DEREF(tmp,_1);
  Assert(!oz_isRef(tmp));
  if (!oz_isLiteral(tmp) && !oz_isVarOrRef(tmp)) {
    pairs->free();
    return FALSE;
  }

  // Unify the corresponding feature values in the two variables:
  // Return FALSE upon encountering the first failing unification
  // Return TRUE if all unifications succeed
  PairList * p = pairs;
  TaggedRef t1, t2;
  while (p->getpair(t1, t2)) {
    Assert(!p->isempty());
    if ((ret = oz_unify(t1, t2)) != PROCEED)
      break;
    p->nextpair();
  }
  Assert(p->isempty() || ret != PROCEED);
  pairs->free();
  if (ret != PROCEED) {
    if (ret != FAILED)
      ofvRestore(trailedVarPtr, trailedVar);
    return (ret);
  }

  // At this point, unification is successful

  // Propagate changes to the suspensions:
  // (this routine is actually OzVariable::propagate)
  propagate(suspList, pc_cv_unif);
  termVar->propagate(termVar->suspList, pc_cv_unif);

  // Take care of linking suspensions
  if (vLoc && tLoc) {
    otherVar->relinkSuspListTo(newVar);
  } else if (vLoc && !tLoc) {
    if (mergeWidth > termWidth) {
      // Suspension* susp=new Suspension(am.currentBoard);
      // Assert(susp!=NULL);
      // termVar->addSuspension(susp);
    } else {
      relinkSuspListTo(termVar);
    }
  } else if (!vLoc && tLoc) {
    if (mergeWidth > varWidth) {
      // Suspension* susp=new Suspension(am.currentBoard);
      // Assert(susp!=NULL);
      // addSuspension(susp);
    } else {
      termVar->relinkSuspListTo(this);
    }
  } else if (!vLoc && !tLoc) {
    // Suspension* susp=new Suspension(am.currentBoard);
    // Assert(susp!=NULL);
    // termVar->addSuspension(susp);
    // addSuspension(susp);
  } else Assert(FALSE);

  return TRUE;
}

// Return TRUE if OFS can't be constrained to l+tupleArity
Bool OzOFVariable::disentailed(Literal *l, int tupleArity) {
    TaggedRef tmp=label;
    DEREF(tmp,_1);
    if (oz_isLiteral(tmp) && !oz_eq(makeTaggedLiteral(l),tmp)) return TRUE;
    return (dynamictable->hasExtraFeatures(tupleArity));
}


// Return TRUE if OFS can't be constrained to l+recordArity
Bool OzOFVariable::disentailed(Literal *l, Arity *recordArity) {
    TaggedRef tmp=label;
    DEREF(tmp,_1);
    if (oz_isLiteral(tmp) && !oz_eq(makeTaggedLiteral(l),tmp)) return TRUE;
    return (dynamictable->hasExtraFeatures(recordArity));
}


Bool OzOFVariable::valid(TaggedRef val)
{
    if (!oz_isLiteral(val)) return FALSE;
    if (getWidth()>0) return FALSE;
    TaggedRef tmp=label;
    DEREF(tmp,_1);
    if (oz_isLiteral(tmp) && !oz_eq(tmp,val)) return FALSE;
    return TRUE;
}

TaggedRef OzOFVariable::getOpenArityList(TaggedRef* ftail, Board* hoome)
{
    return dynamictable->getOpenArityList(ftail,hoome);
}

TaggedRef OzOFVariable::getArityList()
{
    return dynamictable->getArityList();
}

/*
 * Trailing support
 *
 */

OzVariable * OzOFVariable::copyForTrail(void) {
  return new OzOFVariable(label, 
			  dynamictable->copyDynamicTable(), 
			  oz_currentBoard());
}

void OzOFVariable::restoreFromCopy(OzOFVariable * c) {
  TaggedRef l      = c->label;
  DynamicTable * d = c->dynamictable;
  c->label        = label;
  c->dynamictable = dynamictable;
  label        = l;
  dynamictable = d;
}


// ---------------------------------------------------------------------
