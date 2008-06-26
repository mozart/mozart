/*
 *  Authors:
 *    Per Brand, Konstantin Popov
 *
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 *    Boriss Mejias (bmc@info.ucl.ac.be)
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

// The following functions interface the distribution layer to the
// engine.  They implement operations that may imply distribution.
// The function pointers are assigned once the glue layer is loaded.

//
extern Bool (*dpReady)();



/************************* Entity operations **************************/

// We use the following convention:
//
//    OZ_Return (*distXXXYYY)(ZZZ, ...);
//
// XXX denotes the entity type, YYY denotes the operation, and ZZZ
// denotes a pointer or reference to the entity.  The operation's
// arguments are given next.  The returned value may indicate that:
//  - the operation just succeeded (PROCEED)
//  - the operation will be resumed later (BI_REPLACEBICALL)
//  - the operation suspended, typically because of a fault (SUSPEND)

// ports
extern OZ_Return (*distPortSend)(OzPort*, TaggedRef, TaggedRef);

// cells
extern OZ_Return (*distCellOp)(OperationTag, OzCell*, TaggedRef*, TaggedRef*);

// locks
extern OZ_Return (*distLockTake)(OzLock*, TaggedRef);
extern OZ_Return (*distLockRelease)(OzLock*, TaggedRef);

// objects
extern OZ_Return (*distObjectInvoke)(OzObject*, TaggedRef);
extern OZ_Return (*distObjectOp)(OperationTag, OzObject*,
                                 TaggedRef*, TaggedRef*);
extern OZ_Return (*distObjectStateOp)(OperationTag, ObjectState*,
                                      TaggedRef*, TaggedRef*);

// arrays
extern OZ_Return (*distArrayOp)(OperationTag, OzArray*,
                                TaggedRef*, TaggedRef*);

// dictionaries
extern OZ_Return (*distDictionaryOp)(OperationTag, OzDictionary*,
                                     TaggedRef*, TaggedRef*);

// distributed variables
extern OZ_Return (*distVarBind)(OzVariable*, TaggedRef*, TaggedRef);
extern OZ_Return (*distVarUnify)(OzVariable*, TaggedRef*,
                                 OzVariable*, TaggedRef*);
extern OZ_Return (*distVarMakeNeeded)(TaggedRef*);

// chunks
extern OZ_Return (*distChunkOp)(OperationTag, SChunk*,
                                TaggedRef*, TaggedRef*);

// classes
extern OZ_Return (*distClassGet)(OzClass*);

// procedures (Abstraction)
extern OZ_Return (*distProcedureCall)(Abstraction*, TaggedRef);



/******************** Garbage collection routines *********************/

// various phases of GC
extern void (*gCollectGlueStart)();
extern void (*gCollectGlueRoots)();
extern void (*gCollectGlueWeak)();
extern void (*gCollectGlueFinal)();

// mark a given mediator
class Mediator;
extern void (*gCollectMediator)(Mediator*);

// Note.  The class Mediator is defined in libdp; only the type
// Mediator is declared here.



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
