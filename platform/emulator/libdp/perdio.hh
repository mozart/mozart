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
 *    Organization or Person (Year(s))
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
#include "msgbuffer.hh"
#include "msgType.hh"
#include "dpBase.hh"
#include "dsite.hh"

//
Bool isPerdioInitialized();
void perdioInitLocal();

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
void initPerdio();

OZ_Term getGatePort(DSite*);

// 
//

void pendThreadRemoveFirst(PendThread **pt);
OZ_Return pendThreadAddToEnd(PendThread **pt,Thread *t, TaggedRef o, 
			     TaggedRef n, ExKind e, Board* home);

inline OZ_Return pendThreadAddToEnd(PendThread **pt,Thread *t, Board* home){
  return pendThreadAddToEnd(pt,t,0,0,NOEX,home);}

MsgBuffer* getRemoteMsgBuffer(DSite *);
void dumpRemoteMsgBuffer(MsgBuffer*);

class MsgBufferManager{
public:
  MsgBuffer* getMsgBuffer(DSite * s){
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
OZ_Return raiseGeneric(char *id,char *msg, OZ_Term arg);
void gcProxy(Tertiary*);
void gcManager(Tertiary*);

BorrowEntry *receiveAtBorrow(DSite*,int);
OwnerEntry *maybeReceiveAtOwner(DSite*,int);

#define NOT_IMPLEMENTED							\
  {									\
    OZ_warning("in file %s at line %d: not implemented - perdio",	\
	       __FILE__,__LINE__);					\
    Assert(0);								\
  }

/* __PERDIOHH */
#endif 
