/*
 *  Authors:
 *    Peter van Roy (pvr@info.ucl.ac.be)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Peter van Roy (1996)
 *    Tobias Mueller (1996)
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

#include <math.h>
#include "var_of.hh"
#include "var_fd.hh"
#include "builtins.hh"
#include "fdomn.hh"

/*********************************************************************
 * OF builtins
 *********************************************************************/

// global variable in cpi_expect.cc
extern Propagator * imposed_propagator;

// -----------------------------------------------------------------------
// propagators

class WidthPropagator : public OZ_Propagator {
private:
  static OZ_PropagatorProfile profile;
protected:
  OZ_Term rawrec, rawwid;
public:
  WidthPropagator(OZ_Term r, OZ_Term w)
    : rawrec(r), rawwid(w) {}

  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_collectHeapTerm(rawrec,rawrec);
    OZ_collectHeapTerm(rawwid,rawwid);
  }
  virtual size_t sizeOf(void) { return sizeof(WidthPropagator); }
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {return &profile; }
  virtual OZ_Term getParameters(void) const { return OZ_nil(); }
};

class MonitorArityPropagator : public OZ_Propagator {
private:
  static OZ_PropagatorProfile profile;
protected:
  OZ_Term X, K, L, FH, FT;
public:
  MonitorArityPropagator(OZ_Term X1, OZ_Term K1, OZ_Term L1,
                         OZ_Term FH1, OZ_Term FT1)
    : X(X1), K(K1), L(L1), FH(FH1), FT(FT1) {}

  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_collectHeapTerm(X,X);
    OZ_collectHeapTerm(K,K);
    OZ_collectHeapTerm(L,L);
    OZ_collectHeapTerm(FH,FH);
    OZ_collectHeapTerm(FT,FT);
  }
  virtual size_t sizeOf(void) { return sizeof(MonitorArityPropagator); }
  virtual OZ_Return propagate(void);
  virtual OZ_PropagatorProfile * getProfile(void) const {return &profile; }
  virtual OZ_Term getParameters(void) const { return OZ_nil(); }

  TaggedRef getX(void) { return X; }
  TaggedRef getK(void) { return K; }
  TaggedRef getFH(void) { return FH; }
  TaggedRef getFT(void) { return FT; }
  void setFH(TaggedRef FH1) { FH=FH1; }
};



/* -------------------------------------------------------------------------
 * OFS
* -------------------------------------------------------------------------*/

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
    Suspension susp = suspList->getSuspension();

    // The added condition ' || thr->isRunnable () ' is incorrect
    // since isPropagated means only that the thread is runnable
    if (susp.isDead()) {
      suspList = suspList->getNext();
      continue;
    }

    if (susp.isPropagator() && susp.isOFSPropagator()) {
      MonitorArityPropagator * prop =
        (MonitorArityPropagator *) susp.getPropagator()->getPropagator();

      Assert(sizeof(MonitorArityPropagator) == prop->sizeOf());

      // Only add features if var and fvar are the same:
      TaggedRef fvar=prop->getX();
      DEREF(fvar,_1,_2);
      if (var!=fvar) {
        suspList=suspList->getNext();
        continue;
      }
      // Only add features if the 'kill' variable is undetermined:
      TaggedRef killl=prop->getK();
      DEREF(killl,_,killTag);
      if (!isVariableTag(killTag)) {
        suspList=suspList->getNext();
        continue;
      }

      // Add the feature or list to the diff. list in FH and FT:
      if (flist) {
        if (oz_isFeature(flist))
          prop->setFH(oz_cons(flist,prop->getFH()));
        else {
          // flist must be a list
          Assert(oz_isCons(flist));
          TaggedRef tmplist=flist;
          while (tmplist!=AtomNil) {
            prop->setFH(oz_cons(oz_head(tmplist),prop->getFH()));
            tmplist=oz_tail(tmplist);
          }
        }
      }
      if (determ) {
        // FS is det.: tail of list must be bound to nil: (always succeeds)
        // Do *not* use unification to do this binding!
        TaggedRef tl=prop->getFT();
        DEREF(tl,tailPtr,tailTag);
        switch (tailTag) {
        case LITERAL:
          Assert(tl==AtomNil);
          break;
        case UVAR:
          DoBind(tailPtr, AtomNil);
          break;
        default:
          Assert(FALSE);
        }
      }
    }

    suspList = suspList->getNext();
  }
}

/*
 * Constrain term to a record, with given label (wait until
 * determined), with an initial size sufficient for at least tNumFeats
 * features.  If term is already a record, do nothing.
 */

OZ_BI_define(BIsystemTellSize,3,0)
{
  TaggedRef label = OZ_in(0);
  TaggedRef tNumFeats = OZ_in(1);
  TaggedRef t = OZ_in(2);

  // Wait for label
  DEREF(label,labelPtr,labelTag);
  DEREF(t, tPtr, tag);

  /* most probable case first */
  if (isLiteralTag(labelTag) && oz_isFree(t)) {
    DEREF(tNumFeats, nPtr, ntag);
    if (!oz_isSmallInt(tNumFeats)) oz_typeError(1,"Int");
    dt_index numFeats=smallIntValue(tNumFeats);
    dt_index size=ceilPwrTwo((numFeats<=FILLLIMIT) ? numFeats
                             : (int)ceil((double)numFeats/FILLFACTOR));
    OzOFVariable *newofsvar=new OzOFVariable(label,size,oz_currentBoard());
    OZ_Return ok=oz_unify(makeTaggedRef(newTaggedCVar(newofsvar)),
                          makeTaggedRef(tPtr));
    Assert(ok==PROCEED); // mm2
    return PROCEED;
  }

  switch (labelTag) {
  case LTUPLE:
  case SRECORD:
    oz_typeError(0,"Literal");
  case LITERAL:
    break;
  case UVAR:
    // FUT
    oz_suspendOn (makeTaggedRef(labelPtr));
  case CVAR:
    switch (tagged2CVar(label)->getType()) {
    case OZ_VAR_OF:
      {
        OzOFVariable *ofsvar=tagged2GenOFSVar(label);
        if (ofsvar->getWidth()>0) return FAILED;
        oz_suspendOn (makeTaggedRef(labelPtr));
      }
    case OZ_VAR_FD:
    case OZ_VAR_BOOL:
      oz_typeError(0,"Literal");
    default:
      oz_suspendOn (makeTaggedRef(labelPtr));
    }
  default:
    oz_typeError(0,"Literal");
  }

  Assert(labelTag == LITERAL);

  // Create record:
  switch (tag) {
  case LTUPLE:
    return oz_eq(label, AtomCons) ? PROCEED : FAILED;
  case LITERAL:
    return oz_eq(label, t) ? PROCEED : FAILED;
  case SRECORD:
    return oz_eq(label, tagged2SRecord(t)->getLabel()) ? PROCEED : FAILED;
  case CVAR:
    if (tagged2CVar(t)->getType()==OZ_VAR_OF) {
       OZ_Return ret=oz_unify(tagged2GenOFSVar(t)->getLabel(),label);
       tagged2GenOFSVar(t)->propagateOFS();
       return ret;
    }
    if (oz_isKinded(t)) oz_typeError(3,"Record");
    // else fall through to creation case
  case UVAR:
    // FUT
    {
      // Calculate initial size of hash table:
      DEREF(tNumFeats, nPtr, ntag);
      if (!oz_isSmallInt(tNumFeats)) oz_typeError(1,"Int");
      dt_index numFeats=smallIntValue(tNumFeats);
      dt_index size=ceilPwrTwo((numFeats<=FILLLIMIT) ? numFeats
                                                     : (int)ceil((double)numFeats/FILLFACTOR));
      // Create newofsvar with unbound variable as label & given initial size:
      OzOFVariable *newofsvar
        =new OzOFVariable(label,size,oz_currentBoard());
      // Unify newofsvar and term:
      Bool ok=oz_unify(makeTaggedRef(newTaggedCVar(newofsvar)),
                       makeTaggedRef(tPtr));
      Assert(ok==PROCEED); // mm2
      return PROCEED;
    }
  default:
    return FAILED;
  }
} OZ_BI_end


// Constrain term to a record, with given label (wait until determined).
OZ_BI_define(BIrecordTell,2,0)
{
  TaggedRef label = OZ_in(0);
  TaggedRef t = OZ_in(1);

  // Wait for label
  DEREF(t, tPtr, tag);
  DEREF(label,labelPtr,labelTag);

  /* most probable case first */
  if (isLiteralTag(labelTag) && oz_isFree(t)) {
    OzOFVariable *newofsvar=new OzOFVariable(label,oz_currentBoard());
    Bool ok=oz_unify(makeTaggedRef(newTaggedCVar(newofsvar)),
                     makeTaggedRef(tPtr));
    Assert(ok==PROCEED); // mm2
    return PROCEED;
  }

  switch (labelTag) {
  case LTUPLE:
  case SRECORD:
    oz_typeError(0,"Literal");
  case LITERAL:
    break;
  case UVAR:
    // FUT
    oz_suspendOn (makeTaggedRef(labelPtr));
  case CVAR:
    switch (tagged2CVar(label)->getType()) {
    case OZ_VAR_OF:
      {
        OzOFVariable *ofsvar=tagged2GenOFSVar(label);
        if (ofsvar->getWidth()>0) return FAILED;
        oz_suspendOn (makeTaggedRef(labelPtr));
      }
    case OZ_VAR_FD:
    case OZ_VAR_BOOL:
      oz_typeError(0,"Literal");
    default:
      oz_suspendOn (makeTaggedRef(labelPtr));
    }
  default:
    oz_typeError(0,"Literal");
  }

  Assert(labelTag == LITERAL);
  // Create record:
  switch (tag) {
  case LTUPLE:
    return oz_eq(label, AtomCons) ? PROCEED : FAILED;
  case LITERAL:
    return oz_eq(label, t) ? PROCEED : FAILED;
  case SRECORD:
    return oz_eq(label, tagged2SRecord(t)->getLabel()) ? PROCEED : FAILED;
  case CVAR:
    if (tagged2CVar(t)->getType()==OZ_VAR_OF) {
       OZ_Return ret=oz_unify(tagged2GenOFSVar(t)->getLabel(),label);
       tagged2GenOFSVar(t)->propagateOFS();
       return ret;
    }
    if (oz_isKinded(t)) oz_typeError(0,"Record");
    // else fall through to creation case
  case UVAR:
    // FUT
    {
      // Create newofsvar with unbound variable as label & given initial size:
      OzOFVariable *newofsvar=new OzOFVariable(label,oz_currentBoard());
      // Unify newofsvar and term:
      Bool ok=oz_unify(makeTaggedRef(newTaggedCVar(newofsvar)),
                       makeTaggedRef(tPtr));
      Assert(ok==PROCEED); // mm2
      return PROCEED;
    }
  default:
    return FAILED;
  }
} OZ_BI_end


// Suspend until can determine whether term is a record or not.
// This routine extends isRecordInline to accept undetermined records.
OZ_BI_define(BIisRecordCB,1,1)
{
  OZ_Term t=OZ_in(0);
  DEREF(t, tPtr, tag);
  switch (tag) {
  case LTUPLE:
  case LITERAL:
  case SRECORD:
    OZ_RETURN(oz_true());
  case CVAR:
    switch (tagged2CVar(t)->getType()) {
    case OZ_VAR_OF:
      OZ_RETURN(oz_true());
    case OZ_VAR_FD:
    case OZ_VAR_BOOL:
      OZ_RETURN(oz_false());
    default:
      oz_suspendOnPtr(tPtr);
    }
    break;
  case UVAR:
    // FUT
    oz_suspendOnPtr(tPtr);
  default:
    OZ_RETURN(oz_false());
  }
} OZ_BI_end

/*
 * {RecordC.widthC X W} -- builtin that constrains number of features
 * of X to be equal to finite domain variable W.  Will constrain X to
 * a record and W to a finite domain.  This built-in installs a
 * WidthPropagator.
 */
// this really belongs in mozart_cpi.hh

OZ_C_proc_begin(BIwidthC, 2)
{
    OZ_EXPECTED_TYPE("record,finite domain");

    TaggedRef rawrec=OZ_getCArg(0);
    TaggedRef rawwid=OZ_getCArg(1);
    TaggedRef rec=OZ_getCArg(0);
    TaggedRef wid=OZ_getCArg(1);
    DEREF(rec, recPtr, recTag);
    DEREF(wid, widPtr, widTag);

    // Wait until first argument is a constrained record (OFS, SRECORD, LTUPLE, LITERAL):
    switch (recTag) {
    case UVAR:
      // FUT
      oz_suspendOn(rawrec);
    case CVAR:
      switch (tagged2CVar(rec)->getType()) {
      case OZ_VAR_OF:
          break;
      case OZ_VAR_FD:
      case OZ_VAR_BOOL:
          oz_typeError(0,"Record");
      default:
          oz_suspendOn(rawrec);
      }
      break;
    case SRECORD:
    case LITERAL:
    case LTUPLE:
      break;
    default:
      oz_typeError(0,"Record");
    }

    // Ensure that second argument wid is a FD or integer:
    switch (widTag) {
    case UVAR:
      // FUT
    {
        // Create new fdvar:
        OzFDVariable *fdvar=new OzFDVariable(oz_currentBoard()); // Variable with maximal domain
        // Unify fdvar and wid:
        Bool ok=oz_unify(makeTaggedRef(newTaggedCVar(fdvar)),rawwid);
        Assert(ok==PROCEED); // mm2
        break;
    }
    case CVAR:
      if (oz_isKinded(wid) && tagged2CVar(wid)->getType()!=OZ_VAR_FD)
        return FAILED;
      break;
    case OZCONST:
      if (!oz_isBigInt(wid)) return FAILED;
      break;
    case SMALLINT:
        break;
    default:
        return FAILED;
    }

    // This completes the propagation abilities of widthC.  However, entailment is
    // still hard, so this rule will not be added now--we'll wait until people need it.
    //   // If propagator exists already on the variable, just unify the widths
    //   // Implements rule: width(x,w1)/\width(x,w2) -> width(x,w1)/\(w1=w2)
    //   if (recTag==CVAR) {
    //       TaggedRef otherwid=am.getWidthSuspension((void*)BIpropWidth,rec);
    //       if (otherwid!=makeTaggedNULL()) {
    //           return (oz_unify(otherwid,rawwid) ? PROCEED : FAILED);
    //       }
    //   }

    OZ_Expect pe;
    OZ_EXPECT(pe, 0, expectRecordVar);
    OZ_EXPECT(pe, 1, expectIntVar);

    return pe.impose(new WidthPropagator(rawrec, rawwid)); // oz_args[0], oz_args[1]));
} OZ_C_proc_end

OZ_PropagatorProfile WidthPropagator::profile = "BIwidthC";

// Used by BIwidthC built-in
// {PropWidth X W} where X is OFS and W is FD width.
// Assume: rec is OFS or SRECORD or LITERAL.
// Assume: wid is FD or SMALLINT or BIGINT.
// This is the simplest most straightforward possible
// implementation and it can be optimized in many ways.
OZ_Return WidthPropagator::propagate(void)
{
    int recwidth;
    OZ_Return result = SLEEP;

    TaggedRef rec=rawrec;
    TaggedRef wid=rawwid;
    DEREF(rec, recptr, recTag);
    DEREF(wid, widptr, widTag);

    switch (recTag) {
    case SRECORD:
    case LITERAL:
    case LTUPLE:
    {
        // Impose width constraint
        recwidth=(recTag==SRECORD) ? tagged2SRecord(rec)->getWidth() :
                 ((recTag==LTUPLE) ? 2 : 0);
        if (isGenFDVar(wid)) {
            // OzFDVariable *fdwid=tagged2GenFDVar(wid);
            // res=fdwid->setSingleton(recwidth);
          Bool res=oz_unify(makeTaggedSmallInt(recwidth),rawwid); // mm2
          if (!res) { result = FAILED; break; }
        } else if (isSmallIntTag(widTag)) {
            int intwid=smallIntValue(wid);
            if (recwidth!=intwid) { result = FAILED; break; }
        } else if (oz_isBigInt(wid)) {
            // BIGINT case: fail
            result = FAILED; break;
        } else {
          OZ_error("unexpected wrong type for width in determined widthC");
        }
        result = PROCEED;
        break;
    }
    case CVAR:
    {
        Assert(tagged2CVar(rec)->getType() == OZ_VAR_OF);
        // 1. Impose width constraint
        OzOFVariable *recvar=tagged2GenOFSVar(rec);
        recwidth=recvar->getWidth(); // current actual width of record
        if (isGenFDVar(wid)) {
            // Build fd with domain recwidth..fd_sup:
            OZ_FiniteDomain slice;
            slice.initRange(recwidth,fd_sup);
            OZ_FiniteDomain &dom = tagged2GenFDVar(wid)->getDom();
            if (dom.getSize() > (dom & slice).getSize()) {
                OzFDVariable *fdcon=new OzFDVariable(slice,oz_currentBoard());
                Bool res=oz_unify(makeTaggedRef(newTaggedCVar(fdcon)),rawwid); // mm2
                // No loc/glob handling: res=(fdwid>=recwidth);
                if (!res) { result = FAILED; break; }
            }
        } else if (isSmallIntTag(widTag)) {
            int intwid=smallIntValue(wid);
            if (recwidth>intwid) { result = FAILED; break; }
        } else if (oz_isBigInt(wid)) {
            // BIGINT case: fail
            result = FAILED; break;
        } else {
          OZ_error("unexpected wrong type for width in undetermined widthC");
        }
        // 2. Convert representation if necessary
        // 2a. Find size and value (latter is valid only if goodsize==TRUE):
        int goodsize,value;
        DEREF(wid,_3,newwidTag);
        if (isGenFDVar(wid)) {
            OzFDVariable *newfdwid=tagged2GenFDVar(wid);
            goodsize=(newfdwid->getDom().getSize())==1;
            value=newfdwid->getDom().getMinElem();
        } else if (isSmallIntTag(newwidTag)) {
            goodsize=TRUE;
            value=smallIntValue(wid);
        } else {
            goodsize=FALSE;
            value = 0; // make gcc quiet
        }
        // 2b. If size==1 and all features and label present,
        //     then convert to SRECORD or LITERAL:
        if (goodsize && value==recwidth) {
            TaggedRef lbl=tagged2GenOFSVar(rec)->getLabel();
            DEREF(lbl,_4,lblTag);
            if (isLiteralTag(lblTag)) {
                result = PROCEED;
                if (recwidth==0) {
                    // Convert to LITERAL:
                  Bool res=oz_unify(rawrec,lbl); // mm2
                  if (!res) OZ_error("unexpected failure of Literal conversion");
                } else {
                    // Convert to SRECORD or LTUPLE:
                    // (Two efficiency problems: 1. Creates record & then unifies,
                    // instead of creating & only binding.  2. Rec->normalize()
                    // wastes the space of the original record.)
                    TaggedRef alist=tagged2GenOFSVar(rec)->getTable()->getArityList();
                    Arity *arity=aritytable.find(alist);
                    SRecord *newrec = SRecord::newSRecord(lbl,arity);
                    newrec->initArgs();
                    Bool res=oz_unify(rawrec,newrec->normalize()); // mm2
                    Assert(res);
                }
            }
        }
        break;
    }
    default:
      // mm2: type error ?
      OZ_warning("unexpected bad first argument to widthC");
      result=FAILED;
    }

    return (result);
}



// {RecordC.monitorArity X K L} -- builtin that tracks features added to OFS X
// in the list L.  Goes away if K is determined (if K is determined on first call,
// L is list of current features).  monitorArity imposes that X is a record (like
// RecordC) and hence fails if X is not a record.

OZ_C_proc_begin(BImonitorArity, 3)
{
    OZ_EXPECTED_TYPE("any(record),any,any(list)");

    OZ_Term rec = OZ_getCArg(0);
    OZ_Term kill = OZ_getCArg(1);
    OZ_Term arity = OZ_getCArg(2);

    OZ_Term tmpkill=OZ_getCArg(1);
    DEREF(tmpkill,_1,killTag);
    Bool isKilled = !isVariableTag(killTag);

    OZ_Term tmprec=OZ_getCArg(0);
    DEREF(tmprec,_2,recTag);
    switch (recTag) {
    case LTUPLE:
        return oz_unify(arity,makeTupleArityList(2));
    case LITERAL:
        // *** arity is nil
        return oz_unify(arity,AtomNil);
    case SRECORD:
        // *** arity is known set of features of the SRecord
        return oz_unify(arity,tagged2SRecord(tmprec)->getArityList());
    case UVAR:
      // FUT
        oz_suspendOn(rec);
    case CVAR:
        switch (tagged2CVar(tmprec)->getType()) {
        case OZ_VAR_OF:
            break;
        case OZ_VAR_FD:
        case OZ_VAR_BOOL:
            oz_typeError(0,"Record");
        default:
            oz_suspendOn(rec);
        }
        // *** arity is calculated from the OFS; see below
        break;
    default:
        oz_typeError(0,"Record");
    }
    tmprec=OZ_getCArg(0);
    DEREF(tmprec,_3,_4);

    // At this point, rec is OFS and tmprec is dereferenced and undetermined

    if (isKilled) {
        TaggedRef featlist;
        featlist=tagged2GenOFSVar(tmprec)->getArityList();

        return oz_unify(arity,featlist);
    } else {
        TaggedRef featlist;
        TaggedRef feattail;
        Board *home=am.currentBoard();
        featlist=tagged2GenOFSVar(tmprec)->getOpenArityList(&feattail,home);

        if (!oz_unify(featlist,arity)) return FAILED; // mm2

        OZ_Expect pe;
        OZ_EXPECT(pe, 0, expectRecordVar);
        OZ_EXPECT(pe, 1, expectVar);

        TaggedRef uvar=oz_newVar(home);
        OZ_Return r =
          pe.impose(new MonitorArityPropagator(rec,kill,feattail,uvar,uvar));
        imposed_propagator->markOFSPropagator();
        return r;
    }

    return PROCEED;
} OZ_C_proc_end

OZ_PropagatorProfile MonitorArityPropagator::profile = "BImonitorArity";

// The propagator for the built-in RecordC.monitorArity
// {PropFeat X K L FH FT} -- propagate features from X to the list L, and go
// away when K is determined.  L is closed when K is determined.  X is used to
// check in addFeatOFSSuspList that the suspension is waiting for the right
// variable.  FH and FT are a difference list that holds the features that
// have been added.
OZ_Return MonitorArityPropagator::propagate(void)
{
    // Check if killed:
    TaggedRef kill=K;
    TaggedRef tmpkill=kill;
    DEREF(tmpkill,_2,killTag);
    Bool isKilled = !isVariableTag(killTag);

    TaggedRef tmptail=FT;
    DEREF(tmptail,_3,_4);

    // Get featlist (a difference list stored in the arguments):
    TaggedRef fhead = FH;
    TaggedRef ftail = FT;

    if (tmptail!=AtomNil) {
        // The record is not determined, so reinitialize the featlist:
        // The home board of uvar must be taken from outside propFeat!
        // Get the home board for any new variables:
        Board* home=tagged2VarHome(tmptail);
        TaggedRef uvar=oz_newVar(home);
        FH=uvar;
        FT=uvar;
    } else {
        // Precaution for the GC?
        FH=makeTaggedNULL();
        FT=makeTaggedNULL();
    }

    // Add the features to L (the tail of the output list)
    TaggedRef arity=L;
    if (!oz_unify(fhead,arity)) return FAILED; // No further updating of the suspension // mm2
    L=ftail; // 'ftail' is the new L in the suspension

    if (tmptail!=AtomNil) {
        // The record is not determined, so the suspension is revived:
        if (!isKilled) return (SLEEP);
        else return oz_unify(ftail,AtomNil);
    }
    return PROCEED;
}





// Create new thread on suspension:
OZ_Return uparrowInlineNonBlocking(TaggedRef, TaggedRef, TaggedRef&);
OZ_DECLAREBI_USEINLINEFUN2(BIuparrowNonBlocking,uparrowInlineNonBlocking)

// Block current thread on suspension:
OZ_Return uparrowInlineBlocking(TaggedRef, TaggedRef, TaggedRef&);
OZ_DECLAREBI_USEINLINEFUN2(BIuparrowBlocking,uparrowInlineBlocking)

OZ_BI_define(BIuparrowBlockingWrapper,3,0)
{
  OZ_Term out;
  uparrowInlineBlocking(OZ_in(0),OZ_in(1),out);
  return oz_unify(OZ_in(2),out);
} OZ_BI_end



/*
 * NOTE: similar functions are dot, genericSet, uparrow
 */
// X^Y=Z: add feature Y to open feature structure X (Tell operation).
OZ_Return genericUparrowInline(TaggedRef term, TaggedRef fea, TaggedRef &out, Bool blocking)
{
    TaggedRef termOrig=term;
    TaggedRef feaOrig=fea;
    DEREF(term, termPtr, termTag);
    DEREF(fea,  feaPtr,  feaTag);

    // mm2
    // optimize the most common case: adding or reading a feature
    if (isCVar(termTag) &&
        tagged2CVar(term)->getType()==OZ_VAR_OF &&
        oz_isFeature(fea)) {
      OzOFVariable *ofsvar=tagged2GenOFSVar(term);

      TaggedRef t=ofsvar->getFeatureValue(fea);
      if (t!=makeTaggedNULL()) {
        // Feature exists
        out=t;
        return PROCEED;
      }

      if (oz_isCurrentBoard(GETBOARD(ofsvar))) {
        TaggedRef uvar=oz_newVariable();
        Bool ok=ofsvar->addFeatureValue(fea,uvar);
        Assert(ok);
        ofsvar->propagateOFS();
        out=uvar;
        return PROCEED;
      }
    }

    // Wait until Y is a feature:
    if (isVariableTag(feaTag)) {
      if (isCVar(feaTag) && tagged2CVar(fea)->getType()==OZ_VAR_OF) {
        OzOFVariable *ofsvar=tagged2GenOFSVar(fea);
        if (ofsvar->getWidth()>0) goto typeError2;
      }
      if (!oz_isVariable(term) && !oz_isRecord(term)) goto typeError2;

      if (blocking) {
        return SUSPEND;
      } else {
        if (oz_isFree(term)) {
          // Create newofsvar with unbound variable as label:
          OzOFVariable *newofsvar=new OzOFVariable(oz_currentBoard());
          // Unify newofsvar and term:
          Bool ok=oz_unify(makeTaggedRef(newTaggedCVar(newofsvar)),
                           makeTaggedRef(termPtr));
          Assert(ok==PROCEED); // mm2
          term=makeTaggedRef(termPtr);
          DEREF(term, termPtr2, tag2);
          termPtr=termPtr2;
          termTag=tag2;
        }
        // Create thread containing relational blocking version of uparrow:
        RefsArray x=allocateRefsArray(3, NO);
        out=oz_newVariable();
        x[0]=termOrig;
        x[1]=feaOrig;
        x[2]=out;
        OZ_Thread thr=OZ_makeSuspendedThread(BIuparrowBlockingWrapper,x,3);
        OZ_addThread(feaOrig,thr);
        return PROCEED;
      }
    }
    if (!oz_isFeature(fea)) goto typeError2;

    // Add feature and return:
    Assert(term!=makeTaggedNULL());
    switch (termTag) {
    case CVAR:
      if (tagged2CVar(term)->getType() == OZ_VAR_OF) {
        OzOFVariable *ofsvar=tagged2GenOFSVar(term);
        TaggedRef t=ofsvar->getFeatureValue(fea);
        if (t!=makeTaggedNULL()) {
            // Feature exists
            out=t;
        } else {
            // Feature does not yet exist
            // Add feature by (1) creating new ofsvar with one feature,
            // (2) unifying the new ofsvar with the old.
          if (oz_isCurrentBoard(GETBOARD(ofsvar))) {
                // Optimization:
                // If current board is same as ofsvar board then can add feature directly
                TaggedRef uvar=oz_newVariable();
                Bool ok=ofsvar->addFeatureValue(fea,uvar);
                Assert(ok);
                ofsvar->propagateOFS();
                out=uvar;
            } else {
                // Create newofsvar:
                OzOFVariable *newofsvar
                  =new OzOFVariable(oz_currentBoard());
                // Add feature to newofsvar:
                TaggedRef uvar=oz_newVariable();
                Bool ok1=newofsvar->addFeatureValue(fea,uvar);
                Assert(ok1);
                out=uvar;
                // Unify newofsvar and term (which is also an ofsvar):
                Bool ok2=oz_unify(makeTaggedRef(newTaggedCVar(newofsvar)),
                                  makeTaggedRef(termPtr));
                Assert(ok2==PROCEED); // mm2
            }
        }
        return PROCEED;
      }
      // else fall through
    case UVAR:
      // FUT
      {
        // Create newofsvar:
        OzOFVariable *newofsvar=new OzOFVariable(oz_currentBoard());
        // Add feature to newofsvar:
        TaggedRef uvar=oz_newVariable();
        Bool ok1=newofsvar->addFeatureValue(fea,uvar);
        Assert(ok1);
        out=uvar;
        // Unify newofsvar (CVAR) and term (SVAR or UVAR):
        Bool ok2=oz_unify(makeTaggedRef(newTaggedCVar(newofsvar)),
                          makeTaggedRef(termPtr));
        Assert(ok2==PROCEED); // mm2
        return PROCEED;
      }

    case SRECORD:
      {
        // Get the SRecord corresponding to term:
        SRecord* termSRec=makeRecord(term);

        TaggedRef t=termSRec->getFeature(fea);
        if (t!=makeTaggedNULL()) {
            out=t;
            return PROCEED;
        }
        return FAILED;
      }

    case LTUPLE:
      {
        if (!oz_isSmallInt(fea)) return FAILED;
        int i2 = smallIntValue(fea);
        switch (i2) {
        case 1:
          out=tagged2LTuple(term)->getHead();
          return PROCEED;
        case 2:
          out=tagged2LTuple(term)->getTail();
          return PROCEED;
        }
        return FAILED;
      }

    case LITERAL:
        return FAILED;

    default:
        goto typeError1;
    }
typeError1:
    oz_typeError(0,"Record");
typeError2:
    oz_typeError(1,"Feature");
}


OZ_Return uparrowInlineNonBlocking(TaggedRef term, TaggedRef fea,
                                   TaggedRef &out)
{
    return genericUparrowInline(term, fea, out, FALSE);
}

OZ_Return uparrowInlineBlocking(TaggedRef term, TaggedRef fea, TaggedRef &out)
{
    return genericUparrowInline(term, fea, out, TRUE);
}
