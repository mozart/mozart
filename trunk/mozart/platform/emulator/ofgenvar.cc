  
#if defined(INTERFACE)
#pragma implementation "ofgenvar.hh"
#endif

#include "am.hh"

#include "genvar.hh"
#include "ofgenvar.hh"

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
	    if (recordArity->find(feat)==(-1)) return TRUE;
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



//-------------------------------------------------------------------------
//                               for class GenOFSVariable
//-------------------------------------------------------------------------


// (Arguments are dereferenced)
Bool GenOFSVariable::unifyOFS(TaggedRef *vPtr, TaggedRef var,
			      TaggedRef *tPtr, TaggedRef term,
			      ByteCode *scp)
{
    TypeOfTerm tTag = tagTypeOf(term);
    TaggedRef bindInRecordCaseHack = term;
      
    switch (tTag) {
    case LITERAL:
      {
        // Literals have no features:
        if (getWidth()>0) return FALSE;

        // Get local/global flag:
        Bool vLoc=am.isLocalSVar(this);

        // Bind OFSVar to the Literal:
        if (vLoc) doBind(vPtr, term);
        else am.doBindAndTrail(var, vPtr, term);

        // Unify the labels:
        if (!am.unify(term,label,scp)) return FALSE;

        // Update the OFS suspensions:
        if (vLoc) am.addFeatOFSSuspensionList(var,suspList,makeTaggedNULL(),TRUE);

        // Propagate changes to the suspensions:
        // (this routine is actually GenCVariable::propagate)
        if (scp==0) propagate(var, suspList, pc_cv_unif);

        // Take care of linking suspensions
        if (!vLoc) {
            // Add a suspension to the OFSVariable if it is global:
            // Suspension* susp=new Suspension(am.currentBoard);
            // Assert(susp!=NULL);
            // addSuspension(susp);
        }
        return TRUE;
      }

    case LTUPLE:
      {
        // Get the LTuple corresponding to term:
        LTuple* termLTup=tagged2LTuple(term);

        // Get local/global flag:
        Bool vLoc=am.isLocalSVar(this);

        // Check that var features are subset of {1,2}
        TaggedRef arg1=getFeatureValue(makeTaggedSmallInt(1));
        TaggedRef arg2=getFeatureValue(makeTaggedSmallInt(2));
        if ((arg1!=makeTaggedNULL())+(arg2!=makeTaggedNULL()) != getWidth())
            return FALSE;

        // Take care of OFS suspensions:
        if (vLoc && am.hasOFSSuspension(suspList)) {
	    if (getWidth()<2) {
                // Calculate feature or list of features 'flist' that are
                // in LTUPLE and not in OFS.
                TaggedRef flist=AtomNil;
                if (!arg2) flist=cons(makeTaggedSmallInt(2),flist);
                if (!arg1) flist=cons(makeTaggedSmallInt(1),flist);
                // Add the extra features to S_ofs suspensions:
                am.addFeatOFSSuspensionList(var,suspList,flist,TRUE);
	    } else {
                am.addFeatOFSSuspensionList(var,suspList,makeTaggedNULL(),TRUE);
	    }
        }

        // Bind OFSVar to the LTuple:
        if (vLoc) doBind(vPtr, bindInRecordCaseHack);
        else am.doBindAndTrail(var, vPtr, bindInRecordCaseHack);

        // Unify the labels:
        if (!am.unify(AtomCons,label,scp)) return FALSE;

        // Unify corresponding feature values:
        if (arg1 && !am.unify(termLTup->getHead(),arg1,scp)) return FALSE;
        if (arg2 && !am.unify(termLTup->getTail(),arg2,scp)) return FALSE;

        // Propagate changes to the suspensions:
        // (this routine is actually GenCVariable::propagate)
        if (scp==0) propagate(var, suspList, pc_cv_unif);
        return TRUE;
      }

    case SRECORD:
    Record:
      {
        // For all features of var, term should contain the feature.
        // Unify the values of corresponding features.
        // If success, bind the var to the SRECORD (with local/global distinction).

        // Get the SRecord corresponding to term:
        SRecord* termSRec=tagged2SRecord(term);
        Assert(termSRec!=NULL);

        // Get local/global flag:
        Bool vLoc=am.isLocalSVar(this);
  
        // Check that all features of the OFSVar exist in the SRecord:
        // (During the check, calculate the list of feature pairs that correspond.)
        PairList* pairs;
        Bool success=dynamictable->srecordcheck(*termSRec, pairs);
        if (!success) { pairs->free(); return FALSE; }

        // Take care of OFS suspensions:
        if (vLoc && am.hasOFSSuspension(suspList)) {
	    if (termSRec->getWidth()>getWidth()) {
                // Calculate feature or list of features 'flist' that are in SRECORD
                // and not in OFS.
                TaggedRef flist = dynamictable->extraSRecFeatures(*termSRec);
                // Add the extra features to S_ofs suspensions:
                am.addFeatOFSSuspensionList(var,suspList,flist,TRUE);
	    } else {
                am.addFeatOFSSuspensionList(var,suspList,makeTaggedNULL(),TRUE);
	    }
        }

        // Bind OFSVar to the SRecord:
        if (vLoc) doBind(vPtr, bindInRecordCaseHack);
        else am.doBindAndTrail(var, vPtr, bindInRecordCaseHack);
  
        // Unify the labels:
        if (!am.unify(termSRec->getLabel(),label,scp)) 
            { pairs->free(); return FALSE; }

        // Unify corresponding feature values:
        PairList* p=pairs;
        TaggedRef t1, t2;
        while (p->getpair(t1, t2)) {
            Assert(!p->isempty());
            if (am.unify(t1, t2,scp)) {
                // Unification successful
            } else {
                // Unification failed
                success=FALSE;
                break;
            }
            p->nextpair();
        }
        Assert(!success || p->isempty());
        pairs->free();
        if (!success) return FALSE;
        // At this point, unification is successful

        // Propagate changes to the suspensions:
        // (this routine is actually GenCVariable::propagate)
        if (scp==0) propagate(var, suspList, pc_cv_unif);

        // Take care of linking suspensions
        if (!vLoc) {
            // Add a suspension to the OFSVariable if it is global:
            // Suspension* susp=new Suspension(am.currentBoard);
            // Assert(susp!=NULL);
            // addSuspension(susp);
        }
        return TRUE;
      }
  
    case CVAR:
      {
        if (tagged2CVar(term)->getType() != OFSVariable) return FALSE;
        Assert(term!=var);

        // Get the GenOFSVariable corresponding to term:
        GenOFSVariable* termVar=tagged2GenOFSVar(term);
        Assert(termVar!=NULL);

        // Get local/global flags:
        Bool vLoc=am.isLocalSVar(this);
        Bool tLoc=am.isLocalSVar(termVar);
  
        GenOFSVariable* newVar=NULL;
        GenOFSVariable* otherVar=NULL;
        TaggedRef* nvRefPtr=NULL;
        TaggedRef* otherPtr=NULL;
        long varWidth=getWidth();
        long termWidth=termVar->getWidth();
        if (vLoc && tLoc) {
            // Reuse the largest table (optimization to improve unification speed):
            if (varWidth>termWidth) {
                newVar=this;
                nvRefPtr=vPtr;
                otherVar=termVar; // otherVar must be smallest
		otherPtr=tPtr;
            } else {
		newVar=termVar;
		nvRefPtr=tPtr;
		otherVar=this; // otherVar must be smallest
		otherPtr=vPtr;
	    }
        } else if (vLoc && !tLoc) {
            // Reuse the var:
            newVar=this;
            nvRefPtr=vPtr;
            otherVar=termVar;
        } else if (!vLoc && tLoc) {
            // Reuse the term:
            newVar=termVar;
            nvRefPtr=tPtr;
            otherVar=this;
        } else if (!vLoc && !tLoc) {
            // Reuse the largest table (this improves unification speed):
            if (varWidth>termWidth) {
                // Make a local copy of the var's DynamicTable:
                DynamicTable* dt=dynamictable->copyDynamicTable();
                // Make a new GenOFSVariable with the new DynamicTable:
                newVar=new GenOFSVariable(*dt);
                nvRefPtr=newTaggedCVar(newVar);
                otherVar=termVar; // otherVar must be smallest
	    } else {
		// Same as above, but in opposite order:
		DynamicTable* dt=termVar->getTable()->copyDynamicTable();
                newVar=new GenOFSVariable(*dt);
                nvRefPtr=newTaggedCVar(newVar);
                otherVar=this; // otherVar must be smallest
	    }
        } else Assert(FALSE);
        Assert(nvRefPtr!=NULL);
        Assert(newVar!=NULL);
        Assert(otherVar!=NULL);

        // Take care of OFS suspensions, part 1/2 (before merging tables):
        Bool vOk=vLoc && am.hasOFSSuspension(suspList);
        TaggedRef vList = 0;
        if (vOk) {
            // Calculate the extra features in var:
            vList=termVar->dynamictable->extraFeatures(dynamictable);
        }
        Bool tOk=tLoc && am.hasOFSSuspension(termVar->suspList);
        TaggedRef tList = 0;
        if (tOk) {
            // Calculate the extra features in term:
            tList=dynamictable->extraFeatures(termVar->dynamictable);
        }

        // Merge otherVar's DynamicTable into newVar's DynamicTable.
        // (During the merge, calculate the list of feature pairs that correspond.)
        PairList* pairs;
        otherVar->dynamictable->merge(newVar->dynamictable, pairs);
        long mergeWidth=newVar->getWidth();

        // Take care of OFS suspensions, part 2/2 (after merging tables):
        if (vOk && (vList!=AtomNil /*mergeWidth>termWidth*/)) {
            // Add the extra features to S_ofs suspensions:
            am.addFeatOFSSuspensionList(var,suspList,vList,FALSE);
        }
        if (tOk && (tList!=AtomNil /*mergeWidth>varWidth*/)) {
            // Add the extra features to S_ofs suspensions:
            am.addFeatOFSSuspensionList(term,termVar->suspList,tList,FALSE);
        }
        
        // Bind both var and term to the (possibly reused) newVar:
        // Because of cycles, these bindings must be done _before_ the unification
	// If in glob/loc unification, the global is not constrained, then bind
	// the local to the global and relink the local's suspension list
        if (vLoc && tLoc) {
            // bind to var without trailing:
            doBind(otherPtr, makeTaggedRef(nvRefPtr));
        } else if (vLoc && !tLoc) {
            // Global term is constrained if result has more features than term:
            if (mergeWidth>termWidth)
                am.doBindAndTrailAndIP(term, tPtr, makeTaggedRef(vPtr),
				    newVar, otherVar);
            else
                doBind(vPtr, makeTaggedRef(tPtr));
        } else if (!vLoc && tLoc) {
            // Global var is constrained if result has more features than var:
	    if (mergeWidth>varWidth)
                am.doBindAndTrailAndIP(var, vPtr, makeTaggedRef(tPtr),
				    newVar, otherVar);
	    else
		doBind(tPtr, makeTaggedRef(vPtr));
        } else if (!vLoc && !tLoc) {
            // bind to new term with trailing:
            am.doBindAndTrailAndIP(var, vPtr, makeTaggedRef(nvRefPtr),
				newVar, this);
            am.doBindAndTrailAndIP(term, tPtr, makeTaggedRef(nvRefPtr),
				newVar, termVar);
        } else Assert(FALSE);

        // Unify the labels:
        if (!am.unify(termVar->label,label,scp)) 
            { pairs->free(); return FALSE; }
        // Must be literal or variable:
        TaggedRef tmp=label;
        DEREF(tmp,_1,_2);
	if (!isLiteral(tmp) && !isAnyVar(tmp))
            { pairs->free(); return FALSE; }
  
        // Unify the corresponding feature values in the two variables:
        // Return FALSE upon encountering the first failing unification
        // Return TRUE if all unifications succeed
        PairList* p=pairs;
        Bool success=TRUE;
        TaggedRef t1, t2;
        while (p->getpair(t1, t2)) {
            Assert(!p->isempty());
            if (am.unify(t1, t2, scp)) { // CAN ARGS BE _ANY_ TAGGEDREF* ?
                // Unification successful
            } else {
                // Unification failed
                success=FALSE;
                break;
            }
            p->nextpair();
        }
        Assert(!success || p->isempty());
        pairs->free();
        if (!success) return FALSE;
        // At this point, unification is successful
  
        // Propagate changes to the suspensions:
        // (this routine is actually GenCVariable::propagate)
	if (scp==0) {
	  propagate(var, suspList, pc_cv_unif);
	  termVar->propagate(term, termVar->suspList, pc_cv_unif);
	}

        // Take care of linking suspensions
        if (vLoc && tLoc) {
            otherVar->relinkSuspListTo(newVar);
        } else if (vLoc && !tLoc) {
	    if (mergeWidth>termWidth) {
                // Suspension* susp=new Suspension(am.currentBoard);
                // Assert(susp!=NULL);
                // termVar->addSuspension(susp);
	    } else {
		relinkSuspListTo(termVar);
	    }
        } else if (!vLoc && tLoc) {
	    if (mergeWidth>varWidth) {
                // Suspension* susp=new Suspension(am.currentBoard);
                // Assert(susp!=NULL);
                // addSuspension(susp);
	    } else {
		termVar->relinkSuspListTo(this);
	    }
        } else if (!vLoc && !tLoc) {
  	    if (scp==0) {
	      // Suspension* susp=new Suspension(am.currentBoard);
	      // Assert(susp!=NULL);
	      // termVar->addSuspension(susp);
	      // addSuspension(susp);
	    }
        } else Assert(FALSE);

        return TRUE;
      }
  
    default:
        // All other types fail when unified with an open feature structure
        // error("unexpected case in unifyOFS");
        return FALSE;
    }
}


// Return TRUE if OFS can't be constrained to l+tupleArity
Bool GenOFSVariable::disentailed(Literal *l, int tupleArity) {
    TaggedRef tmp=label;
    DEREF(tmp,_1,_2);
    if (isLiteral(tmp) && !literalEq(makeTaggedLiteral(l),tmp)) return TRUE;
    return (dynamictable->hasExtraFeatures(tupleArity));
}


// Return TRUE if OFS can't be constrained to l+recordArity
Bool GenOFSVariable::disentailed(Literal *l, Arity *recordArity) {
    TaggedRef tmp=label;
    DEREF(tmp,_1,_2);
    if (isLiteral(tmp) && !literalEq(makeTaggedLiteral(l),tmp)) return TRUE;
    return (dynamictable->hasExtraFeatures(recordArity));
}


Bool GenOFSVariable::valid(TaggedRef val)
{
    if (!isLiteral(val)) return FALSE;
    if (getWidth()>0) return FALSE;
    TaggedRef tmp=label;
    DEREF(tmp,_1,_2);
    if (isLiteral(tmp) && !literalEq(tmp,val)) return FALSE;
    return TRUE;
}


TaggedRef GenOFSVariable::getOpenArityList(TaggedRef* ftail)
{
    return dynamictable->getOpenArityList(ftail);
}

TaggedRef GenOFSVariable::getOpenArityList(TaggedRef* ftail, Board* hoome)
{
    return dynamictable->getOpenArityList(ftail,hoome);
}

TaggedRef GenOFSVariable::getArityList()
{
    return dynamictable->getArityList();
}



/**** Low-level utilities ****/
  
/* For eventual inlining (in similar manner to fdgenvar.icc): */
  
Bool cvarIsOFSvar(TaggedRef term)
{
    return (tagged2CVar(term)->getType() == OFSVariable);
}
  
Bool isGenOFSVar(TaggedRef term)
{
    GCDEBUG(term);
    return isCVar(term) && cvarIsOFSvar(term);
}
  
Bool isGenOFSVar(TaggedRef term, TypeOfTerm tag)
{
    GCDEBUG(term);
    return isCVar(tag) && cvarIsOFSvar(term);
}

GenOFSVariable* tagged2GenOFSVar(TaggedRef term)
{
    GCDEBUG(term);
#ifdef DEBUG_OFS
    if(isGenOFSVar(term) == NO)
        error("ofs variable expected");
#endif
    return (GenOFSVariable*) tagged2CVar(term);
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

// ---------------------------------------------------------------------
