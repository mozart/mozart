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

//
OZ_Return objectExchangeStub(OzCell*,TaggedRef,TaggedRef,TaggedRef)
{
  OZD_error("'objectExchange' called without DP library?");
  return (PROCEED);
}
void cellOperationDoneStub(OzCell*,TaggedRef){
  OZD_error("'cellDoAccess' called without DP library?");
}

OZ_Return cellAtAccessStub(OzCell*,TaggedRef,TaggedRef)
{
  OZD_error("'cellAtAccess' called without DP library?");
  return (PROCEED);
}
OZ_Return cellAtExchangeStub(OzCell*,TaggedRef,TaggedRef)
{
  OZD_error("'cellAtExchange' called without DP library?");
  return (PROCEED);
}
OZ_Return cellAssignExchangeStub(OzCell*,TaggedRef,TaggedRef)
{
  OZD_error("'cellAssignExchange' called without DP library?");
  return (PROCEED);
}

// arrays
OZ_Return distArrayGetStub(OzArray*, TaggedRef, TaggedRef&) {
  OZD_error("'distArrayGet' called without DP library?");
  return PROCEED;
}
OZ_Return distArrayPutStub(OzArray*, TaggedRef, TaggedRef) {
  OZD_error("'distArrayPut' called without DP library?");
  return PROCEED;
}

// dictionaries
OZ_Return distDictionaryGetStub(OzDictionary*, TaggedRef, TaggedRef&) {
  OZD_error("'distDictionaryGet' called without DP library?");
  return PROCEED;
}
OZ_Return distDictionaryPutStub(OzDictionary*, TaggedRef, TaggedRef) {
  OZD_error("'distDictionaryPut' called without DP library?");
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

// interface for GC;
void gCollectMediatorStub(void* m) {
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

//
OZ_Return (*objectExchange)(OzCell*,TaggedRef,TaggedRef,TaggedRef)
  = objectExchangeStub;
OZ_Return (*cellAtAccess)(OzCell*,TaggedRef,TaggedRef)
  = cellAtAccessStub;
OZ_Return (*cellAtExchange)(OzCell*,TaggedRef,TaggedRef)
  = cellAtExchangeStub;
OZ_Return (*cellAssignExchange)(OzCell*,TaggedRef,TaggedRef)
  = cellAssignExchangeStub;

// arrays
OZ_Return (*distArrayGet)(OzArray*, TaggedRef, TaggedRef&)
  = distArrayGetStub;
OZ_Return (*distArrayPut)(OzArray*, TaggedRef, TaggedRef)
  = distArrayPutStub;

// dictionaries
OZ_Return (*distDictionaryGet)(OzDictionary*, TaggedRef, TaggedRef&)
  = distDictionaryGetStub;
OZ_Return (*distDictionaryPut)(OzDictionary*, TaggedRef, TaggedRef)
  = distDictionaryPutStub;

// distributed variables
OZ_Return (*distVarBind)(OzVariable*, TaggedRef*, TaggedRef)
  = distVarBindStub;
OZ_Return (*distVarUnify)(OzVariable*, TaggedRef*, OzVariable*, TaggedRef*)
  = distVarUnifyStub;
OZ_Return (*distVarMakeNeeded)(TaggedRef*)
  = distVarMakeNeededStub;

// experimental 
void (*cellOperationDone)(OzCell*,TaggedRef)
  =cellOperationDoneStub;

// garbage collection of a mediator
void (*gCollectMediator)(void*)
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


