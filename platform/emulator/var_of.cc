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
      if (!oz_isVar(killl)) {
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

    // Bind OFSVar to the Literal:
    if (vLoc) DoBind(vPtr, term);
    else DoBindAndTrail(vPtr, term);

    // Unify the labels:
    if (!oz_unify(term,label)) return FALSE; // mm_u

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

    // Bind OFSVar to the LTuple:
    if (vLoc) DoBind(vPtr, bindInRecordCaseHack);
    else DoBindAndTrail(vPtr, bindInRecordCaseHack);

    // Unify the labels:
    if (!oz_unify(AtomCons,label)) return FALSE; // mm_u

    // Unify corresponding feature values:
    if (arg1 && !oz_unify(termLTup->getHead(),arg1)) return FALSE; // mm_u
    if (arg2 && !oz_unify(termLTup->getTail(),arg2)) return FALSE; // mm_u

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

    // Bind OFSVar to the SRecord:
    if (vLoc) DoBind(vPtr, bindInRecordCaseHack);
    else DoBindAndTrail(vPtr, bindInRecordCaseHack);

    // Unify the labels:
    if (!oz_unify(termSRec->getLabel(),label))  // mm_u
      { pairs->free(); return FALSE; }

    // Unify corresponding feature values:
    PairList* p=pairs;
    TaggedRef t1, t2;
    while (p->getpair(t1, t2)) {
      Assert(!p->isempty());
      if (oz_unify(t1, t2)) { // mm_u
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
  OzOFVariable * newVar   = NULL;
  OzOFVariable * otherVar = NULL;
  TaggedRef * nvRefPtr    = NULL;
  TaggedRef * otherPtr    = NULL;
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
    nvRefPtr = vPtr;
    otherVar = termVar;
  } else if (!vLoc && tLoc) {
    // Reuse the term:
    newVar   = termVar;
    dt       = newVar->getTable();
    nvRefPtr = tPtr;
    otherVar = this;
  } else if (!vLoc && !tLoc) {
    // Reuse the largest table (this improves unification speed):
    if (varWidth > termWidth) {
      DEBUG_CONSTRAIN_VAR(("varWidth > termWidth\n"));
      // Make a local copy of the var's DynamicTable:
      newVar   = this;
      dt       = newVar->getTable()->copyDynamicTable();
      nvRefPtr = newTaggedVar(newVar);
      otherVar = termVar; // otherVar must be smallest
    } else {
      DEBUG_CONSTRAIN_VAR(("! (varWidth > termWidth)\n"));
      // Same as above, but in opposite order:
      newVar   = termVar;
      dt       = newVar->getTable()->copyDynamicTable();
      nvRefPtr = newTaggedVar(newVar);
      otherVar = this; // otherVar must be smallest
    }
  } else Assert(FALSE);
  Assert(nvRefPtr != NULL);
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
  newVar->dynamictable = dt;

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
  if (vLoc && tLoc) {
    DEBUG_CONSTRAIN_VAR(("vLoc && tLoc\n"));
    Assert(otherPtr);
    // bind to var without trailing:
    bindLocalVar(otherPtr, nvRefPtr);
  } else if (vLoc && !tLoc) {
    DEBUG_CONSTRAIN_VAR(("vLoc && !tLoc\n"));
    // Global term is constrained if result has more features than term:
    if (mergeWidth > termWidth) {
      DEBUG_CONSTRAIN_VAR(("constrainGlobalVar\n"));
      constrainGlobalVar(tPtr, dt);
    }
    bindLocalVar(vPtr, tPtr);
  } else if (!vLoc && tLoc) {
    DEBUG_CONSTRAIN_VAR(("!vLoc && tLoc\n"));
    // Global var is constrained if result has more features than var:
    if (mergeWidth > varWidth) {
      DEBUG_CONSTRAIN_VAR(("constrainGlobalVar\n"));
      constrainGlobalVar(vPtr, dt);
    }
    bindLocalVar(tPtr, vPtr);
  } else if (!vLoc && !tLoc) {
    DEBUG_CONSTRAIN_VAR(("!vLoc && !tLoc\n"));
    // bind to new term with trailing:
    if (mergeWidth > varWidth) {
      DEBUG_CONSTRAIN_VAR(("constrainGlobalVar\n"));
      constrainGlobalVar(vPtr, dt);
    }
    bindGlobalVar(tPtr, vPtr);
  } else Assert(FALSE);

  // Unify the labels:
  if (!oz_unify(termVar->label, label)) {
    pairs->free();
    return FALSE;
  }
  // Must be literal or variable:
  TaggedRef tmp=label;
  DEREF(tmp,_1);
  if (!oz_isLiteral(tmp) && !oz_isVar(tmp)) {
    pairs->free();
    return FALSE;
  }

  // Unify the corresponding feature values in the two variables:
  // Return FALSE upon encountering the first failing unification
  // Return TRUE if all unifications succeed
  PairList * p = pairs;
  Bool success = TRUE;
  TaggedRef t1, t2;
  while (p->getpair(t1, t2)) {
    Assert(!p->isempty());
    if (oz_unify(t1, t2)) { // CAN ARGS BE _ANY_ TAGGEDREF* ?  // mm_u
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
  if (!success) {
    return FALSE;
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
