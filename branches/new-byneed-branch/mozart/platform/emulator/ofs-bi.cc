/*
 *  Authors:
 *    Peter van Roy (pvr@info.ucl.ac.be)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Peter van Roy (1996)
 *    Tobias Mueller (1996)
 *    Christian Schulte, 1999
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
#pragma implementation "ofs-prop.hh"
#endif

#include "ofs-prop.hh"

/*********************************************************************
 * OF builtins
 *********************************************************************/

// global variable in cpi_expect.cc
extern Propagator * imposed_propagator;

// -----------------------------------------------------------------------
// propagators

void WidthPropagator::gCollect(void) {
  OZ_gCollectBlock(&rawrec, &rawrec, 2);
}

void WidthPropagator::sClone(void) {
  OZ_sCloneBlock(&rawrec, &rawrec, 2);
}

void MonitorArityPropagator::gCollect(void) {
  OZ_gCollectBlock(&X, &X, 3);
  if (FH)
    oz_gCollectTerm(FH,FH);
  if (FT)
    oz_gCollectTerm(FT,FT);
}
 
void MonitorArityPropagator::sClone(void) {
  OZ_sCloneBlock(&X, &X, 3);
  if (FH)
    OZ_sCloneBlock(&FH,&FH,1);
  if (FT)
    OZ_sCloneBlock(&FT,&FT,1);
}

/* -------------------------------------------------------------------------
 * OFS
* -------------------------------------------------------------------------*/

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
  DEREF(label,labelPtr);
  DEREF(t, tPtr);

  /* most probable case first */
  if (oz_isLiteral(label) && oz_isFree(t)) {
    DEREF(tNumFeats, nPtr);
    if (!oz_isSmallInt(tNumFeats)) oz_typeError(1,"Int");
    dt_index numFeats=tagged2SmallInt(tNumFeats);
    dt_index size=ceilPwrTwo((numFeats<=FILLLIMIT) ? numFeats
			     : (int)ceil((double)numFeats/FILLFACTOR));
    OzOFVariable *newofsvar=new OzOFVariable(label,size,oz_currentBoard());
    OZ_Return ok=oz_unify(makeTaggedRef(newTaggedVar(newofsvar)),
			  makeTaggedRef(tPtr));
    Assert(ok==PROCEED); // mm2
    return PROCEED;
  }

  switch (tagged2ltag(label)) {
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    oz_typeError(0,"Literal");
  case LTAG_LITERAL:
    break;
  case LTAG_VAR0:
  case LTAG_VAR1:
    switch (tagged2Var(label)->getType()) {
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

  Assert(oz_isLiteral(label));

  // Create record:
  switch (tagged2ltag(t)) {
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
    return oz_eq(label, AtomCons) ? PROCEED : FAILED;
  case LTAG_LITERAL:
    return oz_eq(label, t) ? PROCEED : FAILED;
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    return oz_eq(label, tagged2SRecord(t)->getLabel()) ? PROCEED : FAILED;
  case LTAG_VAR0:
  case LTAG_VAR1:
    if (tagged2Var(t)->getType()==OZ_VAR_OF) {
       OZ_Return ret=oz_unify(tagged2GenOFSVar(t)->getLabel(),label);
       tagged2GenOFSVar(t)->propagateOFS();
       return ret;
    }
    if (oz_isKindedVar(t)) {
      oz_typeError(3,"Record");
    } else {
      // Calculate initial size of hash table:
      DEREF(tNumFeats, nPtr);
      if (!oz_isSmallInt(tNumFeats)) oz_typeError(1,"Int");
      dt_index numFeats=tagged2SmallInt(tNumFeats);
      dt_index size =
	ceilPwrTwo((numFeats<=FILLLIMIT) ? numFeats
		   : (int)ceil((double)numFeats/FILLFACTOR));
      // Create newofsvar with unbound variable as label & given
      // initial size:
      OzOFVariable *newofsvar =
	new OzOFVariable(label,size,oz_currentBoard());
      // Unify newofsvar and term:
      Bool ok=oz_unify(makeTaggedRef(newTaggedVar(newofsvar)),
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
  DEREF(t, tPtr);
  DEREF(label,labelPtr);

  /* most probable case first */
  if (oz_isLiteral(label) && oz_isFree(t)) {
    OzOFVariable *newofsvar=new OzOFVariable(label,oz_currentBoard());
    Bool ok=oz_unify(makeTaggedRef(newTaggedVar(newofsvar)),
		     makeTaggedRef(tPtr));
    Assert(ok==PROCEED); // mm2
    return PROCEED;
  }

  switch (tagged2ltag(label)) {
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    oz_typeError(0,"Literal");
  case LTAG_LITERAL:
    break;
  case LTAG_VAR0:
  case LTAG_VAR1:
    switch (tagged2Var(label)->getType()) {
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

  Assert(oz_isLiteral(label));
  // Create record:
  switch (tagged2ltag(t)) {
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
    return oz_eq(label, AtomCons) ? PROCEED : FAILED;
  case LTAG_LITERAL:
    return oz_eq(label, t) ? PROCEED : FAILED;
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    return oz_eq(label, tagged2SRecord(t)->getLabel()) ? PROCEED : FAILED;
  case LTAG_VAR0:
  case LTAG_VAR1:
    if (tagged2Var(t)->getType()==OZ_VAR_OF) {
       OZ_Return ret=oz_unify(tagged2GenOFSVar(t)->getLabel(),label);
       tagged2GenOFSVar(t)->propagateOFS();
       return ret;
    }
    if (oz_isKindedVar(t)) {
      oz_typeError(0,"Record");
    } else {
      // Create newofsvar with unbound variable as label & given
      // initial size:
      OzOFVariable *newofsvar=new OzOFVariable(label,oz_currentBoard());
      // Unify newofsvar and term:
      Bool ok=oz_unify(makeTaggedRef(newTaggedVar(newofsvar)),
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
  DEREF(t, tPtr);
  switch (tagged2ltag(t)) {
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
  case LTAG_LITERAL:
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    OZ_RETURN(NameTrue);
  case LTAG_VAR0:
  case LTAG_VAR1:
    switch (tagged2Var(t)->getType()) {
    case OZ_VAR_OF:
      OZ_RETURN(NameTrue);
    case OZ_VAR_FD:
    case OZ_VAR_BOOL:
      OZ_RETURN(NameFalse);
    default:
      oz_suspendOnPtr(tPtr);
    }
    break;
  default:
    OZ_RETURN(NameFalse);
  }
} OZ_BI_end

/*
 * {RecordC.widthC X W} -- builtin that constrains number of features
 * of X to be equal to finite domain variable W.  Will constrain X to
 * a record and W to a finite domain.  This built-in installs a
 * WidthPropagator.
 */
// this really belongs in mozart_cpi.hh

OZ_BI_define(BIwidthC, 2, 0)
{
    OZ_EXPECTED_TYPE("record,finite domain");

    TaggedRef rawrec=OZ_in(0);
    TaggedRef rawwid=OZ_in(1);
    TaggedRef rec=rawrec;
    TaggedRef wid=rawwid;
    DEREF(rec, recPtr);
    DEREF(wid, widPtr);

    // Wait until first argument is a constrained record (OFS, SRECORD, LTUPLE, LITERAL):
    switch (tagged2ltag(rec)) {
    case LTAG_VAR0:
    case LTAG_VAR1:
      switch (tagged2Var(rec)->getType()) {
      case OZ_VAR_OF:
          break;
      case OZ_VAR_FD:
      case OZ_VAR_BOOL:
          oz_typeError(0,"Record");
      default:
          oz_suspendOn(rawrec);
      }
      break;
    case LTAG_SRECORD0:
    case LTAG_SRECORD1:
    case LTAG_LITERAL:
    case LTAG_LTUPLE0:
    case LTAG_LTUPLE1:
      break;
    default:
      oz_typeError(0,"Record");
    }

    // Ensure that second argument wid is a FD or integer:
    switch (tagged2ltag(wid)) {
    case LTAG_VAR0:
    case LTAG_VAR1:
      {
	OzVariable *widv = tagged2Var(wid);
	switch (oz_check_var_status(widv)) {
	case EVAR_STATUS_FREE: 
	  {
	    // Create new fdvar:
	    // Variable with maximal domain
	    OzFDVariable *fdvar = new OzFDVariable(oz_currentBoard());
	    // Unify fdvar and wid:
	    Bool ok = oz_unify(makeTaggedRef(newTaggedVar(fdvar)), rawwid);
	    Assert(ok == PROCEED);
	    break;
	  }

	case EVAR_STATUS_FUTURE:
	case EVAR_STATUS_FAILED:
	case EVAR_STATUS_DET:
	case EVAR_STATUS_UNKNOWN:
	  // kost@ : wait until the picture gets clear;
	  oz_suspendOn(rawwid);

	case EVAR_STATUS_KINDED:
	  if (widv->getType() != OZ_VAR_FD)
	    return FAILED;
	  // else somebody did already the job.
	  break;

	default:
	  Assert(0);
	}
	break;
      }

    case LTAG_CONST0:
    case LTAG_CONST1:
      if (!oz_isBigInt(wid)) return FAILED;
      break;
    case LTAG_SMALLINT:
        break;
    default:
        return FAILED;
    }

    // This completes the propagation abilities of widthC.  However, entailment is
    // still hard, so this rule will not be added now--we'll wait until people need it.
    //   // If propagator exists already on the variable, just unify the widths
    //   // Implements rule: width(x,w1)/\width(x,w2) -> width(x,w1)/\(w1=w2)
    //   if (recTag==VAR) {
    //       TaggedRef otherwid=am.getWidthSuspension((void*)BIpropWidth,rec);
    //       if (otherwid!=makeTaggedNULL()) {
    //           return (oz_unify(otherwid,rawwid) ? PROCEED : FAILED);
    //       }
    //   }

    OZ_Expect pe;
    OZ_EXPECT(pe, 0, expectRecordVar);
    OZ_EXPECT(pe, 1, expectIntVar);

    return pe.impose(new WidthPropagator(rawrec, rawwid)); // oz_args[0], oz_args[1]));
} OZ_BI_end

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
    DEREF(rec, recptr);
    DEREF(wid, widptr);

    switch (tagged2ltag(rec)) {
    case LTAG_SRECORD0:
    case LTAG_SRECORD1:
    case LTAG_LITERAL:
    case LTAG_LTUPLE0:
    case LTAG_LTUPLE1:
    {
        // Impose width constraint
      Assert(!oz_isRef(rec));
        recwidth = (oz_isSRecord(rec) ? tagged2SRecord(rec)->getWidth() :
		    (oz_isLTupleOrRef(rec) ? 2 : 0));
        if (isGenFDVar(wid)) {
            // OzFDVariable *fdwid=tagged2GenFDVar(wid);
            // res=fdwid->setSingleton(recwidth);
	  Bool res=oz_unify(makeTaggedSmallInt(recwidth),rawwid); // mm2
	  if (!res) { result = FAILED; break; }
        } else if (oz_isSmallInt(wid)) {
            int intwid=tagged2SmallInt(wid);
            if (recwidth!=intwid) { result = FAILED; break; }
        } else if (oz_isBigInt(wid)) {
            // BIGINT case: fail
            result = FAILED; break;
        } else {
	  OZD_error("unexpected wrong type for width in determined widthC");
        }
        result = PROCEED;
        break;
    }
    case LTAG_VAR0:
    case LTAG_VAR1:
    {
        Assert(tagged2Var(rec)->getType() == OZ_VAR_OF);
        // 1. Impose width constraint
        OzOFVariable *revar=tagged2GenOFSVar(rec);
        recwidth=revar->getWidth(); // current actual width of record
        if (isGenFDVar(wid)) {
            // Build fd with domain recwidth..fd_sup:
            OZ_FiniteDomain slice;
            slice.initRange(recwidth,fd_sup);
            OZ_FiniteDomain &dom = tagged2GenFDVar(wid)->getDom();
            if (dom.getSize() > (dom & slice).getSize()) { 
                OzFDVariable *fdcon=new OzFDVariable(slice,oz_currentBoard());
                Bool res=oz_unify(makeTaggedRef(newTaggedVar(fdcon)),rawwid); // mm2
                // No loc/glob handling: res=(fdwid>=recwidth);
                if (!res) { result = FAILED; break; }
            }
        } else if (oz_isSmallInt(wid)) {
            int intwid=tagged2SmallInt(wid);
            if (recwidth>intwid) { result = FAILED; break; }
        } else if (oz_isBigInt(wid)) {
            // BIGINT case: fail
            result = FAILED; break;
        } else {
	  OZD_error("unexpected wrong type for width in undetermined widthC");
        }
        // 2. Convert representation if necessary
        // 2a. Find size and value (latter is valid only if goodsize==TRUE):
        int goodsize,value;
        DEREF(wid,_3);
        if (isGenFDVar(wid)) {
            OzFDVariable *newfdwid=tagged2GenFDVar(wid);
            goodsize=(newfdwid->getDom().getSize())==1;
            value=newfdwid->getDom().getMinElem();
        } else if (oz_isSmallInt(wid)) {
            goodsize=TRUE;
            value=tagged2SmallInt(wid);
        } else {
            goodsize=FALSE;
	    value = 0; // make gcc quiet
        }
        // 2b. If size==1 and all features and label present, 
        //     then convert to SRECORD or LITERAL:
        if (goodsize && value==recwidth) {
            TaggedRef lbl=tagged2GenOFSVar(rec)->getLabel();
            DEREF(lbl,_4);
            if (oz_isLiteral(lbl)) {
                result = PROCEED;
                if (recwidth==0) {
                    // Convert to LITERAL:
		  Bool res=oz_unify(rawrec,lbl); // mm2
		  if (!res) OZD_error("unexpected failure of Literal conversion");
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

OZ_BI_define(BImonitorArity, 3, 0)
{
    OZ_EXPECTED_TYPE("any(record),any,any(list)");

    OZ_Term rec = OZ_in(0);
    OZ_Term kill = OZ_in(1);
    OZ_Term arity = OZ_in(2);

    OZ_Term tmpkill=OZ_in(1);
    DEREF(tmpkill,_1);
    Assert(!oz_isRef(tmpkill));
    Bool isKilled = !oz_isVarOrRef(tmpkill);

    OZ_Term tmprec=OZ_in(0);
    DEREF(tmprec,_2);
    switch (tagged2ltag(tmprec)) {
    case LTAG_LTUPLE0:
    case LTAG_LTUPLE1:
        return oz_unify(arity,makeTupleArityList(2));
    case LTAG_LITERAL:
        // *** arity is nil
        return oz_unify(arity,AtomNil);
    case LTAG_SRECORD0:
    case LTAG_SRECORD1:
        // *** arity is known set of features of the SRecord
        return oz_unify(arity,tagged2SRecord(tmprec)->getArityList());
    case LTAG_VAR0:
    case LTAG_VAR1:
        switch (tagged2Var(tmprec)->getType()) {
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
    tmprec=OZ_in(0);
    DEREF(tmprec,_3);

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

        TaggedRef v = oz_newVariable(home);
        OZ_Return r = 
	  pe.impose(new MonitorArityPropagator(rec,kill,feattail,v,v));
	imposed_propagator->setOFS();
	return r;
    }

    return PROCEED;
} OZ_BI_end

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
    DEREF(tmpkill,_2);
    Assert(!oz_isRef(tmpkill));
    Bool isKilled = !oz_isVarOrRef(tmpkill);

    TaggedRef tmptail=FT;
    DEREF(tmptail,_3);

    // Get featlist (a difference list stored in the arguments):
    TaggedRef fhead = FH;
    TaggedRef ftail = FT;

    if (tmptail!=AtomNil) {
        // The record is not determined, so reinitialize the featlist:
        // The home board of v(ar) must be taken from outside propFeat!
        // Get the home board for any new variables:
        //
        // kost@ : FT (tmptail) is encapsulated within the propagator, 
        //         so the following really holds:
        Assert(oz_isVar(tmptail));
        OzVariable *ov = tagged2Var(tmptail);
	Board *home = ov->getBoardInternal();
        TaggedRef v = oz_newVariable(home);
        FH = v;
        FT = v;
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




// X^Y=Z: add feature Y to open feature structure X (Tell operation).

OZ_BI_define(BIofsUpArrow, 2, 1) {
  TaggedRef term = OZ_in(0);
  TaggedRef fea  = OZ_in(1);

  DEREF(term, termPtr);
  DEREF(fea,  feaPtr);

  // optimize the most common case: adding or reading a feature
  Assert(!oz_isRef(term));
  if (oz_isVar(term) &&
      tagged2Var(term)->getType()==OZ_VAR_OF &&
      oz_isFeature(fea)) {
    OzOFVariable *ofsvar=tagged2GenOFSVar(term);
    
    TaggedRef t=ofsvar->getFeatureValue(fea);
    if (t!=makeTaggedNULL()) {
      // Feature exists
      OZ_RETURN(t);
    }
    
    if (oz_isCurrentBoard(GETBOARD(ofsvar))) {
      TaggedRef v = oz_newVariable();
      Bool ok=ofsvar->addFeatureValue(fea, v);
      Assert(ok);
      ofsvar->propagateOFS();
      OZ_RETURN(v);
    }
  }
  
  // Wait until Y is a feature:
  Assert(!oz_isRef(fea));
  if (oz_isVarOrRef(fea)) {
    if (tagged2Var(fea)->getType()==OZ_VAR_OF) {
      OzOFVariable *ofsvar=tagged2GenOFSVar(fea);
      if (ofsvar->getWidth()>0) 
	goto typeError2;
    }

    Assert(!oz_isRef(term));
    if (!oz_isVarOrRef(term) && !oz_isRecord(term)) 
      goto typeError2;
    
    oz_suspendOnPtr(feaPtr);
  }

  if (!oz_isFeature(fea)) 
    goto typeError2;
  
  // Add feature and return:
  Assert(term!=makeTaggedNULL());

  switch (tagged2ltag(term)) {
  case LTAG_VAR0:
  case LTAG_VAR1:

    if (tagged2Var(term)->getType() == OZ_VAR_OF) {
      OzOFVariable *ofsvar=tagged2GenOFSVar(term);
      TaggedRef t=ofsvar->getFeatureValue(fea);

      if (t!=makeTaggedNULL()) {
	// Feature exists
	OZ_RETURN(t);
      }

      // Feature does not yet exist
      // Add feature by (1) creating new ofsvar with one feature,
      // (2) unifying the new ofsvar with the old.

      TaggedRef v = oz_newVariable();
	
      if (oz_isCurrentBoard(GETBOARD(ofsvar))) {
	Bool ok=ofsvar->addFeatureValue(fea, v);
	Assert(ok);
	ofsvar->propagateOFS();
      } else {
	// Create newofsvar:
	OzOFVariable *newofsvar
	  =new OzOFVariable(oz_currentBoard());
	// Add feature to newofsvar:
	Bool ok1=newofsvar->addFeatureValue(fea, v);
	Assert(ok1);
	// Unify newofsvar and term (which is also an ofsvar):
	Bool ok2=oz_unify(makeTaggedRef(newTaggedVar(newofsvar)),
			  makeTaggedRef(termPtr));
	Assert(ok2==PROCEED); // mm2
      }

      OZ_RETURN(v);
    } else {
      // Create newofsvar:
      OzOFVariable *newofsvar=new OzOFVariable(oz_currentBoard());
      // Add feature to newofsvar:
      TaggedRef v = oz_newVariable();
      Bool ok1 = newofsvar->addFeatureValue(fea, v);
      Assert(ok1);
      
      // Unify newofsvar (ofs var) and term (free var):
      Bool ok2 =oz_unify(makeTaggedRef(newTaggedVar(newofsvar)),
			 makeTaggedRef(termPtr));
      Assert(ok2==PROCEED); // mm2
      OZ_RETURN(v);
    }
    
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
    {
      // Get the SRecord corresponding to term:
      SRecord* termSRec=makeRecord(term);
      
      TaggedRef t=termSRec->getFeature(fea);
      if (t!=makeTaggedNULL()) {
	OZ_RETURN(t);
      }
      return FAILED;
    }

  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
    {
      if (!oz_isSmallInt(fea)) return FAILED;
      int i2 = tagged2SmallInt(fea);
      switch (i2) {
      case 1:
	OZ_RETURN(tagged2LTuple(term)->getHead());
      case 2:
	OZ_RETURN(tagged2LTuple(term)->getTail());
      }
      return FAILED;
    }
    
  case LTAG_LITERAL:
    return FAILED;
    
  default:
    goto typeError1;
  }
 typeError1:
  oz_typeError(0,"Record");
 typeError2:
  oz_typeError(1,"Feature");
} OZ_BI_end



OZ_BI_define(BIhasLabel, 1, 1)
{
  oz_declareDerefIN(0,rec);
  // Wait for term to be a record with determined label:
  // Get the term's label, if it exists
  Assert(!oz_isRef(rec));
  if (oz_isVarOrRef(rec)) {
    if (isGenOFSVar(rec)) {
      TaggedRef thelabel=tagged2GenOFSVar(rec)->getLabel(); 
      DEREF(thelabel,lPtr);
      OZ_RETURN(oz_bool(!oz_isVar(thelabel)));
    }
    OZ_RETURN(NameFalse);
  }
  if (oz_isRecord(rec)) OZ_RETURN(NameTrue);
  oz_typeError(0,"Record");
} OZ_BI_end


/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modRecordC-if.cc"

#endif
