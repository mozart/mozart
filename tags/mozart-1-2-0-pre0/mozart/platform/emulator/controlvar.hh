/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Michael Mehl (1997)
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

// internal interface to AMOZ

#ifndef RUNTIME_HH
#define RUNTIME_HH

#include "value.hh"
#include "unify.hh"

/* -----------------------------------------------------------------------
 * control variables
 * -----------------------------------------------------------------------*/

#define ControlVarNew(var,home)			\
OZ_Term var = oz_newVariable(home);		\
(void) oz_addSuspendVarList(var);

#define _controlVarUnify(var,val) oz_bind_global(var,val)

#define ControlVarResume(var)			\
_controlVarUnify(var,NameUnit)


#define ControlVarRaise(var,exc) 			\
_controlVarUnify(var,OZ_mkTuple(AtomException,1,exc))

#define ControlVarUnify(var,A,B) 			\
_controlVarUnify(var,OZ_mkTuple(AtomUnify,2,A,B))

#define ControlVarApply(var,P,Args)			\
_controlVarUnify(var,OZ_mkTuple(AtomApply,2,P,Args))

#define ControlVarApplyList(var,PairList)			\
_controlVarUnify(var,OZ_mkTuple(AtomApplyList,1,PairList))


OZ_Return suspendOnControlVar();

void suspendOnControlVar2();

#define SuspendOnControlVar			\
  return suspendOnControlVar();

#define SuspendOnControlVarReturnValue BI_REPLACEBICALL

/* -----------------------------------------------------------------------
 * MISC
 * -----------------------------------------------------------------------*/

/* -----------------------------------------------------------------------
 * TODO
 * -----------------------------------------------------------------------*/

/*

#define am DontUseAM
deref

*/

#endif
