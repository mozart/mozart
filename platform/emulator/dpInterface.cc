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
bool portSendStub(Tertiary *p, TaggedRef msg)
{
  OZD_error("'portSend' called without DP library?");
  return true;
}
bool cellDoExchangeStub(Tertiary*,TaggedRef&,TaggedRef)
{
  OZD_error("'cellDoExchange' called without DP library?");
  return true;
}
OZ_Return objectExchangeStub(Tertiary*,TaggedRef,TaggedRef,TaggedRef)
{
  OZD_error("'objectExchange' called without DP library?");
  return (PROCEED);
}
bool cellDoAccessStub(Tertiary*,TaggedRef&)
{
  OZD_error("'cellDoAccess' called without DP library?");
  return true;
}
void cellOperationDoneStub(Tertiary*,TaggedRef){
  OZD_error("'cellDoAccess' called without DP library?");
}

OZ_Return cellAtAccessStub(Tertiary*,TaggedRef,TaggedRef)
{
  OZD_error("'cellAtAccess' called without DP library?");
  return (PROCEED);
}
OZ_Return cellAtExchangeStub(Tertiary*,TaggedRef,TaggedRef)
{
  OZD_error("'cellAtExchange' called without DP library?");
  return (PROCEED);
}
OZ_Return cellAssignExchangeStub(Tertiary*,TaggedRef,TaggedRef)
{
  OZD_error("'cellAssignExchange' called without DP library?");
  return (PROCEED);
}

// lock/unlock (interface) methods/their usage may be optimized
// further, e.g. inline cases when distributed locks are currently
// local;
// interface;
bool unlockDistLockStub(Tertiary *t)
{
  OZD_error("'unlockDistLock' called without DP library?");
  return false; 
}

bool lockDistLockStub(Tertiary *t, Thread *thr)
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

// interface for GC;
void gCollectProxyRecurseStub(ConstTerm *t, void* indx)
{
  OZD_error("'gCollectProxyRecurse' called without DP library?");
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
bool (*portSend)(Tertiary *p, TaggedRef msg)
  = portSendStub;
bool (*cellDoExchange)(Tertiary*,TaggedRef&,TaggedRef)
  = cellDoExchangeStub;
OZ_Return (*objectExchange)(Tertiary*,TaggedRef,TaggedRef,TaggedRef)
  = objectExchangeStub;
bool (*cellDoAccess)(Tertiary*,TaggedRef&)
  = cellDoAccessStub;
OZ_Return (*cellAtAccess)(Tertiary*,TaggedRef,TaggedRef)
  = cellAtAccessStub;
OZ_Return (*cellAtExchange)(Tertiary*,TaggedRef,TaggedRef)
  = cellAtExchangeStub;
OZ_Return (*cellAssignExchange)(Tertiary*,TaggedRef,TaggedRef)
  = cellAssignExchangeStub;

// experimental 
void (*cellOperationDone)(Tertiary*,TaggedRef)
  =cellOperationDoneStub;

// lock/unlock (interface) methods/their usage may be optimized
// further, e.g. inline cases when distributed locks are currently
// local;
bool (*lockDistLock)(Tertiary *t, Thread *thr)
  = lockDistLockStub;
bool (*unlockDistLock)(Tertiary *t)
  = unlockDistLockStub; 
bool (*distArrayGet)(OzArray*, TaggedRef, TaggedRef&)
  = distArrayGetStub;

bool (*distArrayPut)(OzArray*, TaggedRef, TaggedRef)
  =    distArrayPutStub;
//
void (*gCollectProxyRecurse)(ConstTerm *t, void*)
  = gCollectProxyRecurseStub;
//
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


