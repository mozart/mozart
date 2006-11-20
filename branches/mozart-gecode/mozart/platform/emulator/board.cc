/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Kostja Popow, 1997-1999
 *    Michael Mehl, 1997-1999
 *    Christian Schulte, 1997-1999
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
#pragma implementation "board.hh"
#endif

#include "board.hh"
#include "thr_int.hh"
#include "prop_int.hh"
#include "builtins.hh"
#include "value.hh"
#include "var_base.hh"
#include "var_readonly.hh"
#include "var_opt.hh"
#include "GeVar.hh"
#include "os.hh"
#include "trail.hh"

#ifdef OUTLINE
#include "board.icc"
#endif

/*
 * Static members
 *
 */

Bool Board::_ignoreWakeUp = NO;

/*
 * Generic operations
 *
 */


void newRelink(OzVariable *globalVar,  OzVariable *localVar, Board *sb)
{
  //printf("newRelink\n");fflush(stdout);
  SuspList *sl = globalVar->getSuspList();
  SuspList *local = new SuspList((Suspendable *)NULL);
  SuspList *global = new SuspList((Suspendable *)NULL);
  
  for (;sl;sl=sl->getNext())
    {
      //printf("newRelink dentro del for\n");fflush(stdout);
      if (sb==sl->getSuspendable()->getBoardInternal())
	{
	  if (local->getSuspendable()==NULL)
	    localVar->setSuspList(new SuspList(sl->getSuspendable(),(SuspList *)NULL));
	  else	    
	    localVar->setSuspList(new SuspList(sl->getSuspendable(),local));      
	}
      else
	{
	  if (global->getSuspendable()==NULL)
	    globalVar->setSuspList(new SuspList(sl->getSuspendable(),(SuspList *)NULL));
	  else	    
	    globalVar->setSuspList(new SuspList(sl->getSuspendable(),global));      
	}

    }
  //localVar->setSuspList((SuspList *)NULL);
  //globalVar->setSuspList((SuspList *)NULL);
  //localVar->setSuspList(local);
  //globalVar->setSuspList(global);
  //printf("endRelink\n");fflush(stdout);
}


Board::Board() 
  : suspCount(0), dist(0),
    crt(0), suspList(0), nonMonoSuspList(0),
    status(taggedVoidValue), rootVar(taggedVoidValue),
    script(taggedVoidValue), parent(NULL), flags(BoTag_Root)
{
  //printf("BOARD: called constructor\n");fflush(stdout);
  Assert((int) OddGCStep == 0x0 && (int) EvenGCStep == (int) BoTag_EvenGC);
  optVar = makeTaggedVar(new OptVar(this));
  lpq.init();
  setGCStep(oz_getGCStep());
#ifdef BUILD_GECODE
  gespace = NULL;
#endif
}


Board::Board(Board * p) 
  : suspCount(0), dist(0),
    crt(0), suspList(0), nonMonoSuspList(0),
    script(taggedVoidValue), parent(p), flags(0)
{
  Assert(!p->isCommitted());
  status  = oz_newReadOnly(p);
  optVar = makeTaggedVar(new OptVar(this));
  rootVar = makeTaggedRef(newTaggedOptVar(optVar));
  setGCStep(oz_getGCStep());
  lpq.init();
#ifdef BUILD_GECODE
  if(p->gespace != NULL) {
    gespace = new GenericSpace(this);
    oz_newThreadInject(this)->pushCall(BI_PROP_GEC,NULL);
  } else {
    gespace = NULL;
  }
#endif

#ifdef CS_PROFILE
  orig_start  = (int32 *) NULL;
  copy_start  = (int32 *) NULL;
  copy_size   = 0;
#endif
}

TaggedRef Board::genSuspended(TaggedRef arg) {
  SRecord *stuple = SRecord::newSRecord(AtomSuspended, 1);
  stuple->setArg(0, arg);
  return makeTaggedSRecord(stuple);
}

void Board::bindStatus(TaggedRef t) {
  TaggedRef s = getStatus();
  DEREF(s, sPtr);
  if (oz_isReadOnly(s))
      oz_bindReadOnly(sPtr, t);      
}

void Board::clearStatus() {
  if (oz_isReadOnly(oz_deref(getStatus())))
    return;
  status = oz_newReadOnly(getParent());
}


/*
 * Propagator queue
 *
 */

void Board::wakeServeLPQ(void) {
  Assert(lpq.isEmpty());
  if (board_served == this)
    return;
  oz_newThreadInject(this)->pushCall(BI_PROP_LPQ, NULL);
}

inline
void Board::killServeLPQ(void) {
  board_served = NULL;
  lpq.reset();
}

Board * Board::board_served = NULL;

OZ_Return Board::scheduleLPQ(void) {
  Assert(!board_served);

  board_served = this;

  unsigned int starttime = 0;
	 
  if (ozconf.timeDetailed)
    starttime = osUserTime();
	
  while (!lpq.isEmpty() && !am.isSetSFlag()) {
    Propagator * prop = SuspToPropagator(lpq.dequeue());
    // this is needed by first-class propagators when they are
    // explicitly discarded. I am sorry that I have to destroy this
    // invariant
    // Assert(!prop->isDead());
    if (prop->isDead())
      continue;
    Propagator::setRunningPropagator(prop);

#ifdef COUNT_PROP_INVOCS
    extern int count_prop_invocs_max_runnable;
    extern int count_prop_invocs_min_runnable;
    extern double count_prop_invocs_sum_runnable;
    extern int count_prop_invocs_nb_smp_runnable;
    count_prop_invocs_max_runnable = max(count_prop_invocs_max_runnable,
					 lpq.getSize());
    count_prop_invocs_min_runnable = min(count_prop_invocs_min_runnable,
					 lpq.getSize());
    count_prop_invocs_sum_runnable += lpq.getSize()+1;
    count_prop_invocs_nb_smp_runnable += 1;
#endif
    switch (oz_runPropagator(prop)) {
    case SLEEP:
      oz_sleepPropagator(prop);
      break;
    case PROCEED:
      oz_closeDonePropagator(prop);
      break;
    case SCHEDULED:
      oz_preemptedPropagator(prop);
      break;
    case FAILED:
      
#ifdef NAME_PROPAGATORS
      // this is experimental: a top-level failure with set
      // property 'internal.propLocation',  
      if (am.isPropagatorLocation()) {
	if (!am.hf_raise_failure()) {
	  if (ozconf.errorDebug) 
	    am.setExceptionInfo(OZ_mkTupleC("apply",2,
					    OZ_atom((prop->getPropagator()->getProfile()->getPropagatorName())),
					    prop->getPropagator()->getParameters()));	
	  oz_sleepPropagator(prop);
	  prop->setFailed();
	  killServeLPQ();
	  return RAISE;
	}
      }
#endif

      if (ozconf.timeDetailed)
	ozstat.timeForPropagation.incf(osUserTime()-starttime);
	     
      // check for top-level and if not, prepare raising of an
      // exception (`hf_raise_failure()')
      if (am.hf_raise_failure()) {
	oz_closeDonePropagator(prop);
	killServeLPQ();
	return FAILED;
      }
      
      if (ozconf.errorDebug) 
	am.setExceptionInfo(OZ_mkTupleC("apply",2,
					OZ_atom((prop->getPropagator()->getProfile()->getPropagatorName())),
					prop->getPropagator()->getParameters()));	
      
      oz_closeDonePropagator(prop);
      killServeLPQ();
      return RAISE;
    }

    Assert(prop->isDead() || !prop->isRunnable());
    
  }
  
  if (ozconf.timeDetailed)
    ozstat.timeForPropagation.incf(osUserTime()-starttime);

  if (lpq.isEmpty()) {
    killServeLPQ();
    return PROCEED;
  } else {
    board_served = NULL;
    am.prepareCall(BI_PROP_LPQ, (RefsArray *) NULL);
    return BI_REPLACEBICALL;
  }

}

/*
 * Routines for checking external suspensions
 *
 */

void Board::checkExtSuspension(Suspendable * susp) {

  Board * varHome = derefBoard();

  Board * bb = oz_currentBoard();

  Bool wasFound = NO;
    
  while (bb != varHome) {
    Assert(!bb->isRoot() && !bb->isCommitted() && !bb->isFailed());
    
    bb->addSuspension(susp);
    wasFound = OK;
    
    bb = bb->getParent();
  }
  
  if (wasFound) 
    susp->setExternal();
  
}

static
Bool extParameters(TaggedRef list, Board * solve_board) {

  list = oz_deref(list);

  Assert(!oz_isRef(list));
  while (oz_isLTuple(list)) {
    TaggedRef h = oz_head(list);
    
    Bool found = FALSE;

    DEREF(h, hptr);
    Assert(!oz_isRef(h));
    if (oz_isVar(h)) {

      Assert(!oz_isOptVar(h));

      Board * home = GETBOARD(tagged2Var(h)); 
      Board * tmp  = solve_board;

      // from solve board go up to root; if you step over home 
      // then the variable is external otherwise it must be a local one
      do {
	tmp = tmp->getParent();

	if (tmp->isFailed())
	  return FALSE;
	
	if (tmp == home) { 
	  found = TRUE;
	  break;
	}
      } while (!tmp->isRoot());
      
    } else if (oz_isLTupleOrRef(h)) {
      found = extParameters(h, solve_board);
    }

    if (found) return TRUE;

    list = oz_tail(oz_deref(list));
    Assert(!oz_isRef(list));
  } // while
  return FALSE;
}


void Board::clearSuspList(Suspendable * killSusp) {
  Assert(!isRoot());
  
  SuspList * fsl = getSuspList();
  SuspList * tsl = (SuspList *) 0;

  while (fsl) {
    // Traverse suspension list and copy all valid suspensions
    Suspendable * susp = fsl->getSuspendable();

    fsl = fsl->dispose();

    if (susp->isDead() ||
	killSusp == susp ||
	(susp->isRunnable() && !susp->isPropagator())) {
      continue;
    }

    Board * bb = GETBOARD(susp);

    Bool isAlive = OK;
    
    // find suspensions, which occured in a failed nested search space
    while (1) {
      Assert(!bb->isCommitted() && !bb->isRoot());
      
      if (bb->isFailed()) {
	isAlive = NO;
	break;
      }
		     
      if (bb == this)
	break;

      bb = bb->getParent();
    }

    if (susp->isPropagator()) {
      
      if (isAlive) {
	// if propagator suspends on external variable then keep its
	// thread in the list to avoid stability
	if (extParameters(SuspToPropagator(susp)->getPropagator()->getParameters(), this)) {
	  tsl = new SuspList(susp, tsl);
	} 

      }

    } else {
      Assert(susp->isThread());
      
      if (isAlive) {
	tsl = new SuspList(susp, tsl);
      } else {
	oz_disposeThread(SuspToThread(susp));
      }
      
    }
  }

  setSuspList(tsl);

}


/*
 * Stability checking
 *
 */

void Board::checkStability(void) {
  Assert(!isRoot() && !isFailed() && !isCommitted());
  Assert(this == oz_currentBoard());

  crt--;
  
  Board * pb = getParent();
  
  if (isStable()) {

    pb->decRunnableThreads();

    if (getNonMono()) {
      scheduleNonMono();
    } else {
      Distributor * d = getDistributor();

      if (d) {
	int n = d->getAlternatives();
      
	if (n == 1) {
	  if (d->commit(this,1) == 0)
	    setDistributor(NULL);
	} else {
	  trail.popMark();
	  Assert(!oz_onToplevel() || trail.isEmptyChunk());
	  am.setCurrent(pb, pb->getOptVar());
	  bindStatus(genAlt(n));
	}
	Assert(!oz_onToplevel() || trail.isEmptyChunk());
      
      } else {
	// succeeded
	trail.popMark();
	Assert(!oz_onToplevel() || trail.isEmptyChunk());
	am.setCurrent(pb, pb->getOptVar());
	
	bindStatus(genSucceeded(getSuspCount() == 0));
	Assert(!oz_onToplevel() || trail.isEmptyChunk());
      }
    }

  } else {
    int n = crt;
    setScript(trail.unwind(this));
    Assert(!oz_onToplevel() || trail.isEmptyChunk());
    am.setCurrent(pb, pb->getOptVar());
  
    if (n == 0) {
      // No runnable threads: suspended      
      TaggedRef newVar = oz_newReadOnly(pb);
      
#ifdef BUILD_GECODE
      if (getGenericSpace(true)){
	OzVariable* nv = tagged2Var(oz_deref(newVar));
	TaggedRef oldVar = getStatus();
	DEREF(oldVar, oldVarPtr);
	// Transfer the suspensions in this space to the new variable
	SuspList** suspPtr = tagged2Var(oldVar)->getSuspListRef();
	SuspList* susp = *suspPtr;
	while (susp) {
	  if (susp->getSuspendable()->getBoardInternal() == this) {
	    nv->addSuspSVar(susp->getSuspendable());
	    *suspPtr = susp->getNext();     // drop susp from list
	  } else {
	    suspPtr = (*suspPtr)->getNextRef();
	  }
	  susp = *suspPtr;
	}
      }
#endif

      bindStatus(genSuspended(newVar));
      setStatus(newVar);
      pb->decRunnableThreads();
    }
    Assert(!oz_onToplevel() || trail.isEmptyChunk());    
  }
} 

void Board::fail(void) {
  Board * pb = getParent();

  Assert(!isRoot());
      
  setFailed();
      
  pb->decRunnableThreads();

  trail.unwindFailed();
      
  am.setCurrent(pb, pb->getOptVar());
      
  bindStatus(genFailed());
     
}

/*
 * Script installation
 *
 */

OZ_Return Board::installScript(Bool isMerging)
{
  //  cout<<"installScript"<<endl; fflush(stdout);
  TaggedRef xys = oz_deref(script);

  setScript(oz_nil());
  
  Assert(!oz_isRef(xys));
  while (oz_isLTuple(xys)) {
    TaggedRef xy = oz_deref(oz_head(xys));
    Assert(oz_isCons(xy));
    //global reference
    TaggedRef x = oz_head(xy);
    //local variable
    TaggedRef y = oz_tail(xy);

    xys = oz_deref(oz_tail(xys));
    Assert(!oz_isRef(xys));
    
    if (!isMerging) {
      /*
       * This is very important! Normally it is okay to not
       * wake during installation, since all wakeups will
       * happen anyway and otherwise no termination is achieved.
       *
       * However, if there is the possibility that a new speculative
       * is realized (e.g. <f(X),f(a)> with X global is in the script)
       * we _have_ to wake: that's okay, because the next time the
       * script will be simplified!
       *
       */
      if (!oz_isVarOrRef(oz_deref(x)) && !oz_isVarOrRef(oz_deref(y))) {
	//cout<<"MALDITA SEAAAAAAAAAAAA"<<endl; fflush(stdout);
	Board::ignoreWakeUp(NO);
      }
      else {
	//cout<<"RRRRRRRRRRRRRRRRR"<<endl; fflush(stdout);
	Board::ignoreWakeUp(OK);
      }
    }

    int res = PROCEED;

    if (oz_isGeVar(x)||oz_isGeVar(y))
      Board::ignoreWakeUp(NO);


    if (oz_isGeVar(x)) {
	GeVar *tmpVar = static_cast<GeVar*>(oz_getExtVar(oz_deref(x)));
      if (oz_isGeVar(y)) {

	GeVar *tmpVar2 = static_cast<GeVar*>(var2ExtVar(tagged2Var(oz_deref(y))));
	tmpVar->printDomain(); fflush(stdout);
	tmpVar2->printDomain();
	bool NOempty = tmpVar->intersect(y);
	tmpVar2->printDomain();
	////	bool NOempty = true;
	//	gespace->status();
	//	tmpVar->printDomain();
	res = NOempty? PROCEED: FAILED;
	//if (NOempty)
	//  newRelink(tagged2Var(oz_deref(x)),  tagged2Var(oz_deref(y)), this);
      }
      //Igual toca intersectar el valor local y con la variable global x
      else { 
	//PAra que se hace esto si al final hay un if donde se pregunta 
	//si y es entero y se llama a la unificacion
	Assert(oz_isInt(y));
	bool IS = tmpVar->In(y);
	res = IS? PROCEED: FAILED;
	//	res = oz_unify(x,y);
      }
      TaggedRef *xpt = tagged2Ref(x);
      //DEREF(x,xpp);
      //TaggedRef xpp = oz_deref(x);
      TaggedRef xaux = x;

      //No siempre es verdadero este invariante porque la variable local puede estar determinada
      //      Assert(oz_isVar(oz_deref(y)));

      trail.pushGeVariable(xpt,oz_deref(y));
      if(oz_isInt(y))  
	  res = oz_unify(x,y);		      
    } else {
      //      Board::ignoreWakeUp(NO); //Asi funciona¡¡¡¡¡¡ el ejemplo del hilo Wait
      res = oz_unify(x, y);
      //oz_var_dispose(tagged2Var(oz_deref(y)));
      //doBind(tagged2Ref(y),x);
    }

    Board::ignoreWakeUp(NO);

    if (res != PROCEED) {
      // NOTE: in case of a failure the remaining constraints in the
      //       script are discarded
      if (res == SUSPEND) {
	res = BI_REPLACEBICALL;
	am.prepareCall(BI_Unify,RefsArray::make(x,y));
      }
      if (res == BI_REPLACEBICALL) {
	while (oz_isLTuple(xys)) {
	  TaggedRef xy = oz_deref(oz_head(xys));
	  Assert(oz_isCons(xy));
	  TaggedRef x = oz_head(xy);
	  TaggedRef y = oz_tail(xy);
	  xys = oz_deref(oz_tail(xys));
	  am.prepareCall(BI_Unify,RefsArray::make(x,y));
	}
      }
      return res;
    }
    Assert(!oz_isRef(xys));
  }
  return PROCEED;
}




/*
 * Constraint installation
 *
 */


Bool Board::installDown(Board * frm) {

  if (frm == this)
    return OK;

  if (!getParent()->installDown(frm))
    return NO;

  am.setCurrent(this, optVar);
  trail.pushMark();

  OZ_Return ret = installScript(NO);

  if (ret != PROCEED) {
    Assert(ret==FAILED);
    fail();
    return NO;
  }

  return OK;

}


Bool Board::install(void) {
  // Tries to install "this".
  // If installation of a script fails, NO is returned and
  // the highest space for which installation is possible gets installed.
  // Otherwise, OK is returned.

  Board * frm = oz_currentBoard();

  Assert(!frm->isCommitted() && !this->isCommitted());

  if (frm == this)
    return OK;

  if (!isAlive())
    return NO;

  // Step 1: Mark all spaces including root as installed
  {
    Board * s;

    for (s = frm; !s->isRoot(); s=s->getParent()) {
      Assert(!s->hasMark());
      s->setMark();
    }
    Assert(!s->hasMark());
    s->setMark();
  }

  // Step 2: Find ancestor
  Board * ancestor = this;

  while (!ancestor->hasMark())
    ancestor = ancestor->getParent();

  // Step 3: Deinstall from "frm" to "ancestor", also purge marks
  {
    Board * s = frm;

    while (s != ancestor) {
      Assert(s->hasMark());
      s->unsetMark();
      s->setScript(trail.unwind(s));
      s=s->getParent();
      am.setCurrent(s, s->getOptVar());
    }

    am.setCurrent(ancestor, ancestor->getOptVar());

    // Purge remaining marks
    for ( ; !s->isRoot() ; s=s->getParent()) {
      Assert(s->hasMark());
      s->unsetMark();
    }
    Assert(s->hasMark());
    s->unsetMark();

  }

  // Step 4: Install from "ancestor" to "this"

  return installDown(ancestor);

}

/*
 * Before copying all spaces but the space to be copied get marked.
 * 
 * Important: even committed boards must be marked, since the globality
 * test does not do a dereference!
 *
 */

void Board::setGlobalMarks(void) {
  Board * b = this;

  while (!b->isRoot()) {
    b = b->getParentInternal(); 
    Assert(!b->hasMark());
    b->setMark();
  }
  
}

/*
 * Purge marks after copying
 */

void Board::unsetGlobalMarks(void) {
  Board * b = this;

  while (!b->isRoot()) {
    b = b->getParentInternal(); 
    Assert(b->hasMark());
    b->unsetMark();
  }

}
 
OZ_Return (*OZ_checkSituatedness)(Board *,TaggedRef *);


