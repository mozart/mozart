/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Copyright:
 *    Per Brand, 1998
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

#ifndef __PERDIOHH
#define __PERDIOHH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"
#include "dpBase.hh"
#include "dpInterface.hh"
#include "mbuffer.hh"
#include "msgType.hh"
#include "dsite.hh"
#include "byteBuffer.hh"

//
void initDP();

//
void send(MsgContainer *msgC,int priority);

//
// kost@ 26.3.98 : 'msgReceived()' is NOT a method of a site object.
// That's quite natural: we don't know who send us a message (of
// course, communication layer for remote site do know, but that's
// another story).
void msgReceived(MsgContainer *);

// Used by networklayer to do pinging.
void sendPing(DSite*);

//
OZ_Term getGatePort(DSite*);

//
void globalizeTert(Tertiary *t);

inline Bool SEND_SHORT(DSite* s){
  if(s->siteStatus()==PERM_SITE) {return OK;}
  return NO;}

DSite* getSiteFromTertiaryProxy(Tertiary*);


// MERGER CON OZ_Return raiseGeneric(char *msg, OZ_Term arg);
OZ_Return raiseGeneric(char *id,char *msg, OZ_Term arg);

void gCollectPendThread(PendThread **pt);

BorrowEntry *receiveAtBorrow(DSite*,int);
OwnerEntry *maybeReceiveAtOwner(DSite*,int);

void pendThreadRemoveFirst(PendThread **pt);

void pendThreadAddToEnd(PendThread **,TaggedRef,TaggedRef, ExKind);
void pendThreadAddToEnd(PendThread **);

void pendThreadAddDummyToEnd(PendThread **);
void pendThreadAddRAToEnd(PendThread **,DSite* , DSite*, int);
void pendThreadAddMoveToEnd(PendThread **);

void cellifyObject(Object*);

#define NOT_IMPLEMENTED						\
  {								\
    OZ_warning("in file %s at line %d: not implemented - perdio",	\
	    __FILE__,__LINE__);					\
    Assert(0);}							\
//
Bool isPerdioInitializedImpl();
void gcProxyRecurseImpl(Tertiary *t);
void gcManagerRecurseImpl(Tertiary *t);

//
void gcPerdioStartImpl();
void gcPerdioRootsImpl();
void gcBorrowTableUnusedFramesImpl();
void gcPerdioFinalImpl();

//
void dpExitImpl();

Bool localizeTertiary(Tertiary*);
Bool isTertiaryPending(Tertiary*);
void dpExitWithTimer(unsigned int);

//Stuff for setting values in Networklayer
void setIPAddress(int);
int  getIPAddress();
void setIPPort(int);
int getIPPort();
void setFirewallStatus(Bool);
Bool getFireWallStatus();
void setTransport(OZ_Term);
OZ_Term getTransport();

// ERIK 
// The pointers are set by InitIPconnection in dpMiscModule.cc and
// read by the connection stub and initIP in connection.cc.

extern OZ_Term defaultConnectionProcedure;
extern OZ_Term ConnectPortStream;
extern OZ_Term ConnectPort;

// Avoid name-conflict
inline void perdio_msgReceived(MsgContainer *msgC) {
  msgReceived(msgC);
}

// Message Statistics:
extern int  globalSendCounter;
extern int  globalRecCounter;
extern int  globalOSWriteCounter;
extern int  globalOSReadCounter;
extern int  globalContCounter;

// Logging
extern FILE *logfile;


/* __PERDIOHH */
#endif 

