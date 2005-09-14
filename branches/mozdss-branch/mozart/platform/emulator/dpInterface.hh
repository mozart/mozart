/*
 *  Authors:
 *    Per Brand, Konstantin Popov
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Per Brand, Konstantin Popov 1998
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

#ifndef __DPINTERFACE_HH
#define __DPINTERFACE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"

#define SIZEOFPORTPROXY (4*sizeof(void*))

//
extern Bool (*isPerdioInitialized)();
 
// 
extern bool (*portSend)(OzPort *p, TaggedRef msg);
extern bool (*cellDoExchange)(OzCell*,TaggedRef&,TaggedRef);
extern bool (*cellDoAccess)(OzCell*,TaggedRef&);
extern OZ_Return (*cellAtAccess)(OzCell*,TaggedRef,TaggedRef);
extern OZ_Return (*cellAtExchange)(OzCell*,TaggedRef,TaggedRef);
extern OZ_Return (*cellAssignExchange)(OzCell*,TaggedRef,TaggedRef);
extern OZ_Return (*objectExchange) (OzCell*,TaggedRef,TaggedRef,TaggedRef);
// Experimental, just for testing the behavior of the GDS PROC_EXEC
extern void (*cellOperationDone)(OzCell*,TaggedRef);

// distributed variables
extern OZ_Return (*distVarBind)(OzVariable*, TaggedRef*, TaggedRef);
extern OZ_Return (*distVarUnify)(OzVariable*, TaggedRef*,
				 OzVariable*, TaggedRef*);
extern OZ_Return (*distVarMakeNeeded)(TaggedRef*);

extern bool (*lockDistLock)(Tertiary*, Thread *thr);
extern bool (*unlockDistLock)(Tertiary*);
//
extern bool (*distArrayGet)(OzArray*, TaggedRef, TaggedRef&);
extern bool (*distArrayPut)(OzArray*, TaggedRef, TaggedRef);

extern void (*gCollectMediator)(void *med);
//
//

//
extern void (*gCollectGlueStart)();
extern void (*gCollectGlueRoots)();
extern void (*gCollectGlueWeak)();
extern void (*gCollectGlueFinal)();

// exit hook;
extern void (*dpExit)();

// hook to make changing of tcpcache-size dynamic
extern void (*changeTCPLimit)();

// distribution handlers
extern Bool (*distHandlerInstall)(unsigned short,unsigned short,
				       Thread*,TaggedRef, TaggedRef);
extern Bool (*distHandlerDeInstall)(unsigned short,unsigned short,
				       Thread*,TaggedRef, TaggedRef);
#endif // __DPINTERFACE_HH


