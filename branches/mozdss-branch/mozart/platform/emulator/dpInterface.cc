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

#ifdef INTERFACE
#pragma interface "dpInterface.hh"
#endif

#include "base.hh"
#include "dpInterface.hh"
#include "value.hh"
#include "os.hh"

//
Bool isPerdioInitializedStub()
{
  return (NO);
}

// ports
OZ_Return distPortSendStub(OzPort *p, TaggedRef msg) {
  OZD_error("'distPortSend' called without DP library?");
  return PROCEED;
}

// cells
OZ_Return distCellAccessStub(OzCell*, TaggedRef&) {
  OZD_error("'distCellAccess' called without DP library?");
  return PROCEED;
}
OZ_Return distCellExchangeStub(OzCell*, TaggedRef&, TaggedRef) {
  OZD_error("'distCellExchange' called without DP library?");
  return PROCEED;
}

// locks
OZ_Return distLockTakeStub(OzLock*, TaggedRef) {
  OZD_error("'distLockTake' called without DP library?");
  return PROCEED;
}
OZ_Return distLockReleaseStub(OzLock*, TaggedRef) {
  OZD_error("'distLockRelease' called without DP library?");
  return PROCEED;
}

// objects
OZ_Return distObjectInvokeStub(OzObject*, TaggedRef) {
  OZD_error("'distObjectInvoke' called without DP library?");
  return PROCEED;
}
OZ_Return
distObjectOpStub(OperationTag, OzObject*, TaggedRef*, TaggedRef*) {
  OZD_error("'distObjectOp' called without DP library?");
  return PROCEED;
}
OZ_Return
distObjectStateOpStub(OperationTag, ObjectState*, TaggedRef*, TaggedRef*) {
  OZD_error("'distObjectStateOp' called without DP library?");
  return PROCEED;
}

// arrays
OZ_Return distArrayOpStub(OperationTag, OzArray*, TaggedRef*, TaggedRef*) {
  OZD_error("'distArrayOp' called without DP library?");
  return PROCEED;
}

// dictionaries
OZ_Return
distDictionaryOpStub(OperationTag, OzDictionary*, TaggedRef*, TaggedRef*) {
  OZD_error("'distDictionaryOp' called without DP library?");
  return PROCEED;
}

// distributed variables
OZ_Return distVarBindStub(OzVariable*, TaggedRef*, TaggedRef) {
  OZD_error("'distVarBind' called without DP library");
  return PROCEED;
}
OZ_Return distVarUnifyStub(OzVariable*, TaggedRef*, OzVariable*, TaggedRef*) {
  OZD_error("'distVarUnify' called without DP library");
  return PROCEED;
}
OZ_Return distVarMakeNeededStub(TaggedRef*) {
  OZD_error("'distVarMakeNeeded' called without DP library");
  return PROCEED;
}

// chunks
OZ_Return distChunkOpStub(OperationTag, SChunk*, TaggedRef*, TaggedRef*) {
  OZD_error("'distChunkOp' called without DP library?");
  return PROCEED;
}

// classes
OZ_Return distClassGetStub(OzClass*) {
  OZD_error("'distClassGet' called without DP library?");
  return PROCEED;
}

// procedures
OZ_Return distProcedureCallStub(Abstraction*, TaggedRef) {
  OZD_error("'distProcedureCall' called without DP library?");
  return PROCEED;
}

// interface for GC;
void gCollectMediatorStub(Mediator*) {
  OZD_error("'gCollectMediator' called without DP library?");
}

// stub for gCollectGlueXXX() functions: simply do nothing
void doNothingStub() {}

// exit hook;
void dpExitStub() {;}

// hook to make changing of tcpcache-size dynamic
void changeTCPLimitStub() {;}

Bool distHandlerInstallStub(unsigned short x,unsigned short y,
				 Thread* th,TaggedRef a,TaggedRef b){
  OZD_error("'distHandlerInstall' called without DP library?");  
  return PROCEED;}

Bool distHandlerDeInstallStub(unsigned short x,unsigned short y,
				   Thread* th,TaggedRef a,TaggedRef b){
  OZD_error("'distHandlerDeInstall' called without DP library?");  
  return PROCEED;}

//
// Link interface function pointers against stubs;

//
Bool (*isPerdioInitialized)() = isPerdioInitializedStub;

// ports
OZ_Return (*distPortSend)(OzPort*, TaggedRef)
  = distPortSendStub;

// cells
OZ_Return (*distCellAccess)(OzCell*, TaggedRef&)
  = distCellAccessStub;
OZ_Return (*distCellExchange)(OzCell*, TaggedRef&, TaggedRef)
  = distCellExchangeStub;

// locks
OZ_Return (*distLockTake)(OzLock*, TaggedRef)
  = distLockTakeStub;
OZ_Return (*distLockRelease)(OzLock*, TaggedRef)
  = distLockReleaseStub;

// objects
OZ_Return (*distObjectInvoke)(OzObject*, TaggedRef)
  = distObjectInvokeStub;
OZ_Return (*distObjectOp)(OperationTag, OzObject*, TaggedRef*, TaggedRef*)
  = distObjectOpStub;
OZ_Return (*distObjectStateOp)(OperationTag, ObjectState*,
			       TaggedRef*, TaggedRef*)
  = distObjectStateOpStub;

// arrays
OZ_Return (*distArrayOp)(OperationTag, OzArray*, TaggedRef*, TaggedRef*)
  = distArrayOpStub;

// dictionaries
OZ_Return (*distDictionaryOp)(OperationTag, OzDictionary*,
			      TaggedRef*, TaggedRef*)
  = distDictionaryOpStub;

// distributed variables
OZ_Return (*distVarBind)(OzVariable*, TaggedRef*, TaggedRef)
  = distVarBindStub;
OZ_Return (*distVarUnify)(OzVariable*, TaggedRef*, OzVariable*, TaggedRef*)
  = distVarUnifyStub;
OZ_Return (*distVarMakeNeeded)(TaggedRef*)
  = distVarMakeNeededStub;

// chunks
OZ_Return (*distChunkOp)(OperationTag, SChunk*, TaggedRef*, TaggedRef*)
  = distChunkOpStub;

// classes
OZ_Return (*distClassGet)(OzClass*)
  = distClassGetStub;

// procedures
OZ_Return (*distProcedureCall)(Abstraction*, TaggedRef)
  = distProcedureCallStub;

// garbage collection of a mediator
void (*gCollectMediator)(Mediator*)
  = gCollectMediatorStub;

// dss garbage collection steps
void (*gCollectGlueStart)()
  = doNothingStub;
void (*gCollectGlueRoots)()
  = doNothingStub;
void (*gCollectGlueWeak)()
  = doNothingStub;
void (*gCollectGlueFinal)()
  = doNothingStub;

// exit hook;
void (*dpExit)()
  = dpExitStub;

// hook to make changing of tcpcache-size dynamic
void (*changeTCPLimit)()
  = changeTCPLimitStub;

// distribution handlers

Bool (*distHandlerInstall)(unsigned short,unsigned short,Thread*,
				TaggedRef, TaggedRef)
  = distHandlerInstallStub;

Bool (*distHandlerDeInstall)(unsigned short,unsigned short,Thread*,
				  TaggedRef, TaggedRef)
  = distHandlerDeInstallStub;


