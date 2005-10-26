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

//
bool portSendStub(OzPort *p, TaggedRef msg)
{
  OZD_error("'portSend' called without DP library?");
  return true;
}
bool cellDoExchangeStub(OzCell*,TaggedRef&,TaggedRef)
{
  OZD_error("'cellDoExchange' called without DP library?");
  return true;
}
OZ_Return objectExchangeStub(OzCell*,TaggedRef,TaggedRef,TaggedRef)
{
  OZD_error("'objectExchange' called without DP library?");
  return (PROCEED);
}
bool cellDoAccessStub(OzCell*,TaggedRef&)
{
  OZD_error("'cellDoAccess' called without DP library?");
  return true;
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

// lock/unlock (interface) methods/their usage may be optimized
// further, e.g. inline cases when distributed locks are currently
// local;
// interface;
bool unlockDistLockStub(OzLock *l)
{
  OZD_error("'unlockDistLock' called without DP library?");
  return false; 
}

bool lockDistLockStub(OzLock *l, Thread *thr)
{
  OZD_error("'lockDistLock' called without DP library?");
  return false;
}
bool distArrayGetStub(OzArray*, TaggedRef, TaggedRef&){
  OZD_error("'distArrayGetStub' called without DP library?");
  return false; 
}
bool distArrayPutStub(OzArray*, TaggedRef, TaggedRef){
  OZD_error("'distArrayPutStub' called without DP library?");
  return false; 
}
bool distDictionaryGetStub(OzDictionary*, TaggedRef, TaggedRef&){
  OZD_error("'distDictionaryGetStub' called without DP library?");
  return false; 
}
bool distDictionaryPutStub(OzDictionary*, TaggedRef, TaggedRef){
  OZD_error("'distDictionaryPutStub' called without DP library?");
  return false; 
}

// interface for GC;
void gCollectMediatorStub(void* m)
{
  OZD_error("'gCollectMediator' called without DP library?");
}

//
// (Only) gCollect method - for cells & locks (because we have to know
// whether they are accessible locally or not);

//
void gCollectPerdioStartStub() {}
void gCollectPerdioFinalStub() {}
void gCollectBorrowTableUnusedFramesStub() {}
void gCollectPerdioRootsStub() {}

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

// 
bool (*portSend)(OzPort *p, TaggedRef msg)
  = portSendStub;
bool (*cellDoExchange)(OzCell*,TaggedRef&,TaggedRef)
  = cellDoExchangeStub;
OZ_Return (*objectExchange)(OzCell*,TaggedRef,TaggedRef,TaggedRef)
  = objectExchangeStub;
bool (*cellDoAccess)(OzCell*,TaggedRef&)
  = cellDoAccessStub;
OZ_Return (*cellAtAccess)(OzCell*,TaggedRef,TaggedRef)
  = cellAtAccessStub;
OZ_Return (*cellAtExchange)(OzCell*,TaggedRef,TaggedRef)
  = cellAtExchangeStub;
OZ_Return (*cellAssignExchange)(OzCell*,TaggedRef,TaggedRef)
  = cellAssignExchangeStub;

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

// lock/unlock (interface) methods/their usage may be optimized
// further, e.g. inline cases when distributed locks are currently
// local;
bool (*lockDistLock)(OzLock *l, Thread *thr)
  = lockDistLockStub;
bool (*unlockDistLock)(OzLock *l)
  = unlockDistLockStub; 
bool (*distArrayGet)(OzArray*, TaggedRef, TaggedRef&)
  = distArrayGetStub;
bool (*distArrayPut)(OzArray*, TaggedRef, TaggedRef)
  =    distArrayPutStub;
bool (*distDictionaryGet)(OzDictionary*, TaggedRef, TaggedRef&)
  = distDictionaryGetStub;
bool (*distDictionaryPut)(OzDictionary*, TaggedRef, TaggedRef)
  =    distDictionaryPutStub;

// garbage collection of a mediator
void (*gCollectMediator)(void*)
  = gCollectMediatorStub;

// dss garbage collection steps
void (*gCollectGlueStart)()
  = gCollectPerdioStartStub;
void (*gCollectGlueRoots)()
  = gCollectPerdioRootsStub;
void (*gCollectGlueWeak)()
  = gCollectBorrowTableUnusedFramesStub;
void (*gCollectGlueFinal)()
  = gCollectPerdioFinalStub;

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


