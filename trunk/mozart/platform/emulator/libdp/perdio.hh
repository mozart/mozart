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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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
#include "msgbuffer.hh"
#include "msgType.hh"
#include "dsite.hh"

//
void initDP();

//
// Per said it must be here;
void SendTo(DSite *toS,MsgBuffer *bs,MessageType mt,DSite *sS,int sI);

//
// kost@ 26.3.98 : 'msgReceived()' is NOT a method of a site object.
// That's quite natural: we don't know who send us a message (of
// course, communication layer for remote site do know, but that's
// another story).
void msgReceived(MsgBuffer *);

//
OZ_Term getGatePort(DSite*);

// 
//
MsgBuffer* getRemoteMsgBuffer(DSite *);
void dumpRemoteMsgBuffer(MsgBuffer*);

class MsgBufferManager{
public:
  MsgBuffer* getMsgBuffer(DSite * s){
    Assert(s!=myDSite);
    Assert(s!=NULL);
    if(s->remoteComm()){
      return getRemoteMsgBuffer(s);}
    return (*getVirtualMsgBuffer)(s);}

  void dumpMsgBuffer(MsgBuffer *m){ // only for marshaled/write stuff (not read)
    DSite *s=m->getSite();
    if(s->remoteComm()){
      dumpRemoteMsgBuffer(m);}
    else{
      Assert(s!=NULL);
      (*dumpVirtualMsgBuffer)(m);}}
};

extern MsgBufferManager *msgBufferManager;

void globalizeTert(Tertiary *t);
GName *globalizeConst(ConstTerm *t, MsgBuffer *bs);

inline Bool SEND_SHORT(DSite* s){
  if(s->siteStatus()==PERM_SITE) {return OK;}
  return NO;}

DSite* getSiteFromTertiaryProxy(Tertiary*);


// MERGER CON OZ_Return raiseGeneric(char *msg, OZ_Term arg);
OZ_Return raiseGeneric(char *id,char *msg, OZ_Term arg);

void gcPendThread(PendThread **pt);

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
void gcBorrowTableUnusedFramesImpl();
void gcFrameToProxyImpl();
void gcPerdioFinalImpl();
void gcPerdioRootsImpl();
void dpExitImpl();

void SiteUnify(TaggedRef, TaggedRef);

Bool localizeTertiary(Tertiary*);
Bool isTertiaryPending(Tertiary*);
void dpExitWithTimer(unsigned int);

/* __PERDIOHH */
#endif 

