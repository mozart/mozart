  
#if defined(__GNUC__)
#pragma implementation "ofgenvar.hh"
#endif
  
#include "tagged.hh"
#include "term.hh"
#include "genvar.hh"
#include "misc.hh"

//-------------------------------------------------------------------------
//                               for class DynamicTable
//-------------------------------------------------------------------------

// Create an initially empty dynamictable of size s (default 1) 
DynamicTable* DynamicTable::newDynamicTable(dt_index s=1) {
    Assert(isPwrTwo(s));
    size_t memSize = sizeof(DynamicTable) + sizeof(HashElement)*(s-1);
    DynamicTable* ret = (DynamicTable *) heapMalloc(memSize);
    Assert(ret!=NULL);
    ret->init(s);
    return ret;
}

// Initialize an elsewhere-allocated dynamictable of size s
void DynamicTable::init(dt_index s=1) {
    Assert(isPwrTwo(s));
    numelem=0;
    size=s;
    for (dt_index i=0; i<s; i++) table[i].ident=makeTaggedNULL();
}

// Create a copy of an existing dynamictable
DynamicTable* DynamicTable::copyDynamicTable() {
    Assert(isPwrTwo(size));
    Assert(numelem<size);
    Assert(size>0);
    size_t memSize = sizeof(DynamicTable) + sizeof(HashElement)*(size-1);
    DynamicTable* ret = (DynamicTable *) heapMalloc(memSize);
    ret->numelem=numelem;
    ret->size=size;
    for (dt_index i=0; i<ret->size; i++) ret->table[i]=table[i];
    return ret;
}

// Test whether the current table has too little room for one new element:
// ATTENTION: Calls to insert should be preceded by fullTest.
// This test sets maximum fullness of table to 75%.
Bool DynamicTable::fullTest() {
    Assert(isPwrTwo(size));
    return (numelem>=((size>>1)+(size>>2)));
}

// Return a table that is double the size of the current table and
// that contains the same elements:
// ATTENTION: Should be called before insert if the table is full.
DynamicTable* DynamicTable::doubleDynamicTable() {
    Assert(isPwrTwo(size));
    int newSize=size<<1;
    DynamicTable* ret=newDynamicTable(newSize);
    for(dt_index i=0; i<size; i++) {
        if (table[i].ident!=makeTaggedNULL()) {
            Assert(isLiteral(table[i].ident));
            ret->insert(table[i].ident, table[i].value);
        }
    }
    return ret;
}

// Insert val at index id 
// Return NULL if val is successfully inserted (id did not exist) 
// Return the value of the pre-existing element if id already exists
// Test for and increase size of hash table if it becomes too full
// ATTENTION: insert must only be done if the table has room for a new element.
TaggedRef DynamicTable::insert(TaggedRef id, TaggedRef val) {
    Assert(isPwrTwo(size));
    Assert(isLiteral(id));
    Assert(!fullTest());
    dt_index i=fullhash(id);
    Assert(i<size);
    if (table[i].ident!=makeTaggedNULL()) {
        Assert(isLiteral(table[i].ident));
        // Ident exists already; return value & don't insert
        return table[i].value;
    } else {
        // Ident doesn't exist; insert value
        numelem++;
        Assert(numelem<size);
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
    Assert(isLiteral(id));
    dt_index i=fullhash(id);
    Assert(i<size);
    if (table[i].ident==id) {
        Assert(isLiteral(table[i].ident));
        // Val is found
        return table[i].value;
    } else {
        // Val is not found
        return makeTaggedNULL();
    }
}

// Return TRUE iff there are features in an external dynamictable that
// are not in the current dynamictable
Bool DynamicTable::extraFeaturesIn(DynamicTable* dt) {
    Assert(isPwrTwo(size));
    for (dt_index i=0; i<dt->size; i++) {
        if (dt->table[i].ident!=makeTaggedNULL()) {
    	Assert(isLiteral(dt->table[i].ident));
    	Bool exists=lookup(dt->table[i].ident);
    	if (!exists) return TRUE;
        }
    }
    return FALSE;
}

// Merge the current dynamictable into an external dynamictable
// Return a pairlist containing all term pairs with the same feature
// The external dynamictable is resized if necessary
void DynamicTable::merge(DynamicTable* &dt, PairList* &pairs) {
    Assert(isPwrTwo(size));
    pairs=new PairList();
    Assert(pairs->isempty());
    for (dt_index i=0; i<size; i++) {
        if (table[i].ident!=makeTaggedNULL()) {
    	    Assert(isLiteral(table[i].ident));
    	    if (dt->fullTest()) dt = dt->doubleDynamicTable();
            TaggedRef val=dt->insert(table[i].ident, table[i].value);
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
        if (table[i].ident!=makeTaggedNULL()) {
    	    Assert(isLiteral(table[i].ident));
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


//-------------------------------------------------------------------------
//                               for class GenOFSVariable
//-------------------------------------------------------------------------


// (Arguments are dereferenced)
Bool GenOFSVariable::unifyOFS(TaggedRef *vPtr, TaggedRef var,
			      TaggedRef *tPtr, TaggedRef term,
			      Bool prop)
{
    TypeOfTerm tTag = tagTypeOf(term);
  
    switch (tTag) {
    case LITERAL:
      {
        // Literals have no features:
        if (getWidth()>0) return FALSE;

        // Unify the labels:
        if (!am.unify(term,label,prop)) return FALSE;

        // At this point, unification is successful

        // Get local/global flag:
        Bool vLoc=(prop && isLocalVariable());

        // Bind OFSVar to the Literal:
        if (vLoc) doBind(vPtr, TaggedRef(term));
        else doBindAndTrail(var, vPtr, TaggedRef(term));

        // Propagate changes to the suspensions:
        // (this routine is actually GenCVariable::propagate)
        if (prop) propagate(var, suspList, makeTaggedRef(vPtr), pc_cv_unif);

        // Take care of linking suspensions
        if (!vLoc) {
            // Add a suspension to the OFSVariable if it is global:
            Suspension* susp=new Suspension(am.currentBoard);
            Assert(susp!=NULL);
            addSuspension(susp);
        }
        return TRUE;
      }

    case SRECORD:
      {
        // For all features of var, term should contain the feature.
        // Unify the values of corresponding features.
        // If success, bind the var to the SRECORD (with local/global distinction).

        // Get the SRecord corresponding to term:
        SRecord* termSRec=tagged2SRecord(term);
        Assert(termSRec!=NULL);
  
        // Unify the labels:
        if (!am.unify(termSRec->getLabel(),label,prop)) return FALSE;
        // Must be literal or variable:
        // TaggedRef tmp=label;
        // DEREF(tmp,_1,_2);
	// if (!isLiteral(tmp) && !isAnyVar(tmp)) return FALSE;

        // Get local/global flag:
        Bool vLoc=(prop && isLocalVariable());
  
        // Check that all features of the OFSVar exist in the SRecord:
        // (During the check, calculate the list of feature pairs that correspond.)
        PairList* pairs;
        Bool success=dynamictable->srecordcheck(*termSRec, pairs);
        if (!success) { pairs->free(); return FALSE; }

        // Bind OFSVar to the SRecord:
        if (vLoc) doBind(vPtr, TaggedRef(term));
        else doBindAndTrail(var, vPtr, TaggedRef(term));

        // Unify corresponding feature values:
        PairList* p=pairs;
        TaggedRef t1, t2;
        while (p->getpair(t1, t2)) {
            Assert(!p->isempty());
            if (am.unify(t1, t2,prop)) {
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
        if (prop) propagate(var, suspList, makeTaggedRef(vPtr), pc_cv_unif);

        // Take care of linking suspensions
        if (!vLoc) {
            // Add a suspension to the OFSVariable if it is global:
            Suspension* susp=new Suspension(am.currentBoard);
            Assert(susp!=NULL);
            addSuspension(susp);
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

        // Unify the labels:
        if (!am.unify(termVar->label,label,prop)) return FALSE;
        // Must be literal or variable:
        TaggedRef tmp=label;
        DEREF(tmp,_1,_2);
	if (!isLiteral(tmp) && !isAnyVar(tmp)) return FALSE;
  
        // Get local/global flags:
        Bool vLoc=(prop && isLocalVariable());
        Bool tLoc=(prop && termVar->isLocalVariable());
  
        GenOFSVariable* newVar=NULL;
        GenOFSVariable* otherVar=NULL;
        TaggedRef* nvRefPtr=NULL;
        TaggedRef* otherPtr=NULL;
        Bool globConstrained=TRUE;
        if (vLoc && tLoc) {
            // Reuse the largest table (optimization to improve unification speed):
            if (getWidth()>termVar->getWidth()) {
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
            globConstrained = otherVar->dynamictable->extraFeaturesIn(newVar->dynamictable);
        } else if (!vLoc && tLoc) {
            // Reuse the term:
            newVar=termVar;
            nvRefPtr=tPtr;
            otherVar=this;
            globConstrained = otherVar->dynamictable->extraFeaturesIn(newVar->dynamictable);
        } else if (!vLoc && !tLoc) {
            // Reuse the largest table (this improves unification speed):
            if (getWidth()>termVar->getWidth()) {
                // Make a local copy of the var's DynamicTable.
                DynamicTable* dt=dynamictable->copyDynamicTable();
                // Make a new GenOFSVariable with the new DynamicTable:
                TaggedRef pn=tagged2CVar(var)->getName();
                newVar=new GenOFSVariable(*dt, pn);
                nvRefPtr=newTaggedCVar(newVar);
                otherVar=termVar; // otherVar must be smallest
	    } else {
		// Same as above, but in opposite order:
		DynamicTable* dt=termVar->getTable()->copyDynamicTable();
                TaggedRef pn=tagged2CVar(term)->getName();
                newVar=new GenOFSVariable(*dt, pn);
                nvRefPtr=newTaggedCVar(newVar);
                otherVar=this; // otherVar must be smallest
	    }
        } else Assert(FALSE);
        Assert(nvRefPtr!=NULL);
        Assert(newVar!=NULL);
        Assert(otherVar!=NULL);

        // Merge otherVar's DynamicTable into newVar's DynamicTable.
        // (During the merge, calculate the list of feature pairs that correspond.)
        PairList* pairs;
        otherVar->dynamictable->merge(newVar->dynamictable, pairs);
        
        // Bind both var and term to the (possibly reused) newVar:
        // Because of cycles, these bindings must be done _before_ the unification
	// If in glob/loc unification, the global is not constrained, then bind
	// the local to the global and relink the local's suspension list
        if (vLoc && tLoc) {
            // bind to var without trailing:
            doBind(otherPtr, makeTaggedRef(nvRefPtr));
        } else if (vLoc && !tLoc) {
            if (globConstrained)
                doBindAndTrail(term, tPtr, makeTaggedRef(vPtr));
            else
                doBind(vPtr, makeTaggedRef(tPtr));
        } else if (!vLoc && tLoc) {
	    if (globConstrained)
                doBindAndTrail(var, vPtr, makeTaggedRef(tPtr));
	    else
		doBind(tPtr, makeTaggedRef(vPtr));
        } else if (!vLoc && !tLoc) {
            // bind to new term with trailing:
            doBindAndTrail(var, vPtr, makeTaggedRef(nvRefPtr));
            doBindAndTrail(term, tPtr, makeTaggedRef(nvRefPtr));
        } else Assert(FALSE);

        // Unify the corresponding feature values in the two variables:
        // Return FALSE upon encountering the first failing unification
        // Return TRUE if all unifications succeed
        PairList* p=pairs;
        Bool success=TRUE;
        TaggedRef t1, t2;
        while (p->getpair(t1, t2)) {
            Assert(!p->isempty());
            if (am.unify(t1, t2, prop)) { // CAN ARGS BE _ANY_ TAGGEDREF* ?
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
	if (prop) {
	  propagate(var, suspList, makeTaggedRef(nvRefPtr), pc_cv_unif);
	  termVar->propagate(term, termVar->suspList, makeTaggedRef(nvRefPtr),
			     pc_cv_unif);
	}

        // Take care of linking suspensions
        if (vLoc && tLoc) {
            otherVar->relinkSuspListTo(newVar);
        } else if (vLoc && !tLoc) {
	    if (globConstrained) {
                Suspension* susp=new Suspension(am.currentBoard);
                Assert(susp!=NULL);
                termVar->addSuspension(susp);
	    } else {
		relinkSuspListTo(termVar);
	    }
        } else if (!vLoc && tLoc) {
	    if (globConstrained) {
                Suspension* susp=new Suspension(am.currentBoard);
                Assert(susp!=NULL);
                addSuspension(susp);
	    } else {
		termVar->relinkSuspListTo(this);
	    }
        } else if (!vLoc && !tLoc) {
  	    if (prop) {
	      Suspension* susp=new Suspension(am.currentBoard);
	      Assert(susp!=NULL);
	      termVar->addSuspension(susp);
	      addSuspension(susp);
	    }
        } else Assert(FALSE);

        return TRUE;
      }
  
    default:
        // All other types fail when unified with an open feature structure
        error("unexpected case in unifyOFS");
        return FALSE;
    }
}


Bool GenOFSVariable::valid(TaggedRef val) {
    if (!isLiteral(val)) return FALSE;
    if (getWidth()>0) return FALSE;
    TaggedRef tmp=label;
    DEREF(tmp,_1,_2);
    if (isLiteral(tmp) && !sameLiteral(tmp,val)) return FALSE;
    return TRUE;
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
