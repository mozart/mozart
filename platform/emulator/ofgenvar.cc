
#if defined(__GNUC__)
#pragma implementation "ofgenvar.hh"
#endif

#include "tagged.hh"
#include "term.hh"
#include "genvar.hh"

// (Arguments are dereferenced)
Bool GenOFSVariable::unifyOFS(TaggedRef *vPtr, TaggedRef var,  TypeOfTerm vTag,
                              TaggedRef *tPtr, TaggedRef term, TypeOfTerm tTag)
{
    switch (tTag) {
    case SRECORD:
      {
        // For all features of var, term should contain the feature.
        // Unify the values of corresponding features.
        // If success, bind the var to the SRECORD (with local/global distinction).

        // Get the SRecord corresponding to term:
        SRecord* termSRec=tagged2SRecord(term);
        Assert(termSRec!=NULL);

        // Check that the label is 'open':
        if (!sameAtom(termSRec->getLabel(),AtomOpen)) return FALSE;

        // Get local/global flag:
        Bool vLoc=isLocalVariable();

        // Check that all features of the OFSVar exist in the SRecord:
        // (During the check, calculate the list of feature pairs that correspond.)
        PairList* pairs;
        Bool success=dynamictable.srecordcheck(*termSRec, pairs);
        if (!success) { pairs->free(); return FALSE; }

        // Bind OFSVar to the SRecord:
        if (vLoc) doBind(vPtr, TaggedRef(tPtr));
        else doBindAndTrail(var, vPtr, TaggedRef(tPtr));

        // Unify corresponding feature values:
        PairList* p=pairs;
        TaggedRef t1, t2;
        while (p->getpair(t1, t2)) {
            Assert(!p->isempty());
            if (am.unify(t1, t2)) {
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
        propagate(var, suspList, makeTaggedRef(vPtr), pc_cv_unif);

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

        // Get the GenOFSVariable corresponding to term:
        GenOFSVariable* termVar=tagged2GenOFSVar(term);

        // Get local/global flags:
        Bool vLoc=isLocalVariable();
        Bool tLoc=termVar->isLocalVariable();

        GenOFSVariable* newVar=NULL;
        GenOFSVariable* otherVar=NULL;
        TaggedRef* nvRefPtr=NULL;
        if (vLoc && tLoc) {
            // Reuse the var:
            newVar=this;
            nvRefPtr=vPtr;
            otherVar=termVar;
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
            // Make a copy of the var's DynamicTable.
            DynamicTable* dt=new DynamicTable(dynamictable);
            // Make a new GenOFSVariable with the new DynamicTable:
            TaggedRef pn=tagged2CVar(var)->getName();
            newVar=new GenOFSVariable(*dt, pn);
            nvRefPtr=newTaggedCVar(newVar);
            otherVar=termVar;
        } else Assert(FALSE);
        Assert(nvRefPtr!=NULL);
        Assert(newVar!=NULL);
        Assert(otherVar!=NULL);

        // Merge otherVar's DynamicTable into newVar's DynamicTable.
        // (During the merge, calculate the list of feature pairs that correspond.)
        PairList* pairs;
        newVar->dynamictable.merge(otherVar->dynamictable, pairs);

        // Bind both var and term to the (possibly reused) newVar:
        // Because of cycles, these bindings must be done _before_ the unification
        if (vLoc && tLoc) {
            // bind to var without trailing:
            doBind(tPtr, makeTaggedRef(nvRefPtr));
        } else if (vLoc && !tLoc) {
            // bind to var with trailing:
            doBindAndTrail(term, tPtr, makeTaggedRef(nvRefPtr));
        } else if (!vLoc && tLoc) {
            // bind to term with trailing:
            doBindAndTrail(var, vPtr, makeTaggedRef(nvRefPtr));
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
            if (am.unify(t1, t2)) { // CAN ARGS BE _ANY_ TAGGEDREF* ?
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
        propagate(var, suspList, makeTaggedRef(nvRefPtr), pc_cv_unif);
        termVar->propagate(term, termVar->suspList, makeTaggedRef(nvRefPtr), pc_cv_unif);

        // Take care of linking suspensions
        if (vLoc && tLoc) {
            termVar->relinkSuspListTo(this);
        } else if (vLoc && !tLoc) {
            Suspension* susp=new Suspension(am.currentBoard);
            Assert(susp!=NULL);
            termVar->addSuspension(susp);
        } else if (!vLoc && tLoc) {
            Suspension* susp=new Suspension(am.currentBoard);
            Assert(susp!=NULL);
            addSuspension(susp);
        } else if (!vLoc && !tLoc) {
            Suspension* susp=new Suspension(am.currentBoard);
            Assert(susp!=NULL);
            termVar->addSuspension(susp);
            addSuspension(susp);
        } else Assert(FALSE);

        return TRUE;
      }

    default:
        // All other types fail when unified with an open feature structure
        error("unexpected case in unifyOFS");
        return FALSE;
    }
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
