/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "cpi.hh"

//-----------------------------------------------------------------------------

void * OZ_FDIntVar::operator new(size_t s) 
{
  return CpiHeap.alloc(s);
}

void OZ_FDIntVar::operator delete(void * p, size_t s)
{
  // deliberately left empty
}

#ifdef __GNUC__
void * OZ_FDIntVar::operator new[](size_t s) 
{
  return CpiHeap.alloc(s);
}

void OZ_FDIntVar::operator delete[](void * p, size_t s)
{
  // deliberately left empty
}
#endif

void OZ_FDIntVar::ask(OZ_Term v) 
{
  Assert(isRef(v) || !isAnyVar(v));
  
  _DEREF(v, varPtr, vtag);
  var = v;

  if (isSmallInt(vtag)) {
    initial_size = dom.initSingleton(smallIntValue(v));
    domPtr = &dom;
    setSort(sgl_e);
  } else {
    Assert(isCVar(vtag));

    GenCVariable * cvar = tagged2CVar(v);

    if (cvar->getType() == BoolVariable) {
      initial_size = dom.initBool();
      domPtr = &dom;
      setSort(bool_e);
    } else {
      Assert(cvar->getType() == FDVariable);
	
      domPtr = &((GenFDVariable *) cvar)->getDom();
      initial_size = domPtr->getSize();
      setSort(int_e);
      
      Assert(initial_size > 1 && *domPtr != fd_bool);
    }
  } 
}

int OZ_FDIntVar::read(OZ_Term v) 
{
  Assert(isRef(v) || !isAnyVar(v));
  
  _DEREF(v, varPtr, vtag);
  var = v;

  if (isSmallInt(vtag)) {

    initial_size = dom.initSingleton(smallIntValue(v));
    initial_width = 0;
    domPtr = &dom;
    setSort(sgl_e);

  } else {
    Assert(isCVar(vtag));

    if (am.currentThread->isLocalThread()) {
    // local variable per definition

      setState(loc_e);
      GenCVariable * cvar = tagged2CVar(v);

      if (cvar->testReifiedFlag()) {
      // may already be a reified var; then the type is incorrect
	if (cvar->isBoolPatched()) goto local_bool; else goto local_fd; 
      } else if (cvar->getType() == BoolVariable) {	
      local_bool:

	setSort(bool_e);
	
	if (cvar->testStoreFlag()) {
	// may already be entered as store var
	  initial_size = 2;
	  domPtr = ((GenBoolVariable *) cvar)->getStorePatchBool();
	} else {
	// entered first time as store var
	  initial_size = dom.initBool();
	  domPtr = &dom;
	  ((GenBoolVariable *) cvar)->patchStoreBool(domPtr);
	}
	initial_width = 1;
      } else {
      local_fd:
	
	setSort(int_e);
	
	if (am.currentBoard->isRoot())
	  dom = ((GenFDVariable *) cvar)->getDom();
	domPtr = &((GenFDVariable *) cvar)->getDom();
	initial_size = domPtr->getSize();
	initial_width = ((OZ_FiniteDomainImpl * )domPtr)->getWidth();

	Assert(initial_size > 1 && *domPtr != fd_bool);
      }
      
    } else {
      // don't know before hand if local or global
      
      GenCVariable * cvar = tagged2CVar(v);
      setState(am.isLocalCVar(v) ? loc_e : glob_e);
      
      if (cvar->testReifiedFlag()) {
      // may already be a reified var; then the type is incorrect
	if (cvar->isBoolPatched()) goto global_bool; else goto global_fd; 
      } else if (cvar->getType() == BoolVariable) {
      global_bool:
	setSort(bool_e);
	
	if (cvar->testStoreFlag()) {
	// may already be entered as store var
	  domPtr = ((GenBoolVariable *) cvar)->getStorePatchBool();
	  initial_size = 2;
	} else {
	// entered first time as store var
	  domPtr = &dom;
	  ((GenBoolVariable *) cvar)->patchStoreBool(domPtr);
	  initial_size = dom.initBool();
	}
	initial_width = 1;

      } else {
      global_fd:
	
	if (isState(glob_e) || am.currentBoard->isRoot())
	  dom = ((GenFDVariable *) cvar)->getDom();
	domPtr = &((GenFDVariable *) cvar)->getDom();
	initial_size = domPtr->getSize();
	initial_width = ((OZ_FiniteDomainImpl *)domPtr)->getWidth();
	setSort(int_e);

	Assert(initial_size > 1 && *domPtr != fd_bool);
      }
    }
 
    setStoreFlag(v);
  }
  return domPtr->getSize();
}

int OZ_FDIntVar::readEncap(OZ_Term v) 
{
  Assert(isRef(v) || !isAnyVar(v));

  _DEREF(v, varPtr, vtag);
  var = v;

  if (isSmallInt(vtag)) {
    initial_size = dom.initSingleton(smallIntValue(v));
    initial_width = 0;
    domPtr = &dom;
    setState(loc_e);
    setSort(sgl_e);
  } else {
    Assert(isCVar(vtag));

    setState(encap_e);
    GenCVariable * cvar = tagged2CVar(v);
    
    if (cvar->testReifiedFlag()) {
    // var is already entered somewhere else
      setSort(cvar->isBoolPatched() ? bool_e : int_e);
      domPtr = cvar->getReifiedPatch();
      initial_size = domPtr->getSize();
      initial_width = ((OZ_FiniteDomainImpl *)domPtr)->getWidth();
    } else if (cvar->getType() == BoolVariable) {
    // bool var entered first time
      initial_size = dom.initBool();
      initial_width = 1;
      domPtr = &dom;
      setSort(bool_e);
      cvar->patchReified(domPtr, TRUE);
    } else {
    // fd var entered first time
      Assert(cvar->getType() == FDVariable);

      setSort(int_e);
      dom.initEmpty();
      dom = ((GenFDVariable *) cvar)->getDom();
      domPtr = &dom;
      initial_size = domPtr->getSize();
      initial_width = ((OZ_FiniteDomainImpl *)domPtr)->getWidth();

      cvar->patchReified(domPtr, FALSE);

      Assert(initial_size > 1 && *domPtr != fd_bool);
    }
    setReifiedFlag(v);
  }
  return domPtr->getSize();
}

#define CHECK_BOUNDS 							\
   (initial_width > ((OZ_FiniteDomainImpl *)domPtr)->getWidth() 	\
     ? fd_prop_bounds : fd_prop_any)

OZ_Boolean OZ_FDIntVar::tell(void)
{
  if (testReifiedFlag(var)) 
    unpatchReified(var, isSort(bool_e));
  
  if (!testResetStoreFlag(var)) {
    return OZ_FALSE;
  } else if(!isTouched()) {
    return OZ_TRUE;
  } else if (isSort(int_e)) { // finite domain variable
    if (*domPtr == fd_singl) {
      if (isState(loc_e)) {
	tagged2GenFDVar(var)->becomesSmallIntAndPropagate(varPtr);
      } else {
	int singl = domPtr->getSingleElem();
	*domPtr = dom;
	tagged2GenFDVar(var)->propagate(var, fd_prop_singl);
	am.doBindAndTrail(var, varPtr, OZ_int(singl));
      }
    } else if (*domPtr == fd_bool) {
      if (isState(loc_e)) {
	tagged2GenFDVar(var)->becomesBoolVarAndPropagate(varPtr);
      } else {
	*domPtr = dom;
	tagged2GenFDVar(var)->propagate(var, CHECK_BOUNDS);
	GenBoolVariable * newboolvar = new GenBoolVariable();
	OZ_Term * newtaggedboolvar = newTaggedCVar(newboolvar);
	am.doBindAndTrailAndIP(var, varPtr,
			       makeTaggedRef(newtaggedboolvar),
			       newboolvar, tagged2GenBoolVar(var));
      }
      return OZ_TRUE;
    } else {
      tagged2GenFDVar(var)->propagate(var, CHECK_BOUNDS);
      if (isState(glob_e)) {
	GenFDVariable * locfdvar = new GenFDVariable(*domPtr);
	OZ_Term * loctaggedfdvar = newTaggedCVar(locfdvar);
	*domPtr = dom;
	am.doBindAndTrailAndIP(var, varPtr,
			       makeTaggedRef(loctaggedfdvar),
			       locfdvar, tagged2GenFDVar(var));
      } 
      return OZ_TRUE;
    }
  } else {
    Assert(isSort(bool_e) && *domPtr == fd_singl); // boolean variable
    
    if (isState(loc_e)) {
      tagged2GenBoolVar(var)->becomesSmallIntAndPropagate(varPtr, *domPtr);
    } else {
      tagged2GenBoolVar(var)->propagate(var);
      am.doBindAndTrail(var, varPtr, OZ_int(domPtr->getSingleElem()));
    }
  }
  return OZ_FALSE;
}

void OZ_FDIntVar::fail(void)
{
  if (isSort(sgl_e)) 
    return;
  if (isState(encap_e)) {
    unpatchReified(var, isSort(bool_e));
    return;
  }

  // dont't change the order of the calls (side effects!)
  if (testResetStoreFlag(var) && isState(glob_e) && isSort(int_e)) {
    *domPtr = dom;
  } else if (am.currentBoard->isRoot()) {
    *domPtr = dom;
  }
}

// End of File
//-----------------------------------------------------------------------------


