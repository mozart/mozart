/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 * 
 *  Copyright:
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

#include "msgContainer.hh"
#include "transObj.hh"

void MsgContainer::init(DSite *site) {
  flags=0;
  destination = site;
  msgTS = (MsgTermSnapshot *) 0;
  next = NULL;
  msgNum=-1;
  //  sendTime=-1; AN!
  cont = (void *) 0;
  DebugCode(for (int i=0;i<MAX_NOF_FIELDS;i++)
	       msgFields[i].arg=NULL;)
}

void MsgContainer::takeSnapshot() {
  Assert(!checkFlag(MSG_HAS_MARSHALCONT));
  Assert(!checkFlag(MSG_HAS_UNMARSHALCONT));

  //
  for(int i = 0 ; i < MAX_NOF_FIELDS; i++) {
    switch(msgFields[i].ft) {
    case FT_FULLTOPTERM:
      {
	OZ_Term t = (OZ_Term) msgFields[i].arg;
	// currently at most one term per message:
	Assert(msgTS == (MsgTermSnapshot *) 0);
	msgTS = takeTermSnapshot(t, destination, TRUE);
      }
      break;

    case FT_TERM:
      {
	OZ_Term t = (OZ_Term) msgFields[i].arg;
	// currently at most one term per message:
	Assert(msgTS == (MsgTermSnapshot *) 0);
	msgTS = takeTermSnapshot(t, destination, FALSE);
      }
      break;

    case FT_NUMBER:
    case FT_CREDIT:
    case FT_STRING:
    case FT_SITE:
    case FT_NONE:
      break;

    default:
      Assert(0);
      break;
    }
  }
}

void MsgContainer::deleteSnapshot() {
  if (msgTS) {
    deleteTermSnapshot(msgTS);
    // should not be reused before 'init':
    DebugCode(msgTS = (MsgTermSnapshot *) -1);
  }
}

// includes MessageType-specific get_,put_,marshal_,unmarshal_,gcMsgC_
#include "msgContainer_marshal.cc"

MsgContainerManager::~MsgContainerManager() {
  MsgContainer *msgC;
  FreeListEntry *f;
  int l=length();
  for(int i=0;i<l;i++) {
    f=getOne();
    Assert(f!=NULL);
    msgC = new (f) MsgContainer;
    delete msgC;
  }
  Assert(length()==0);
}

MsgContainer *MsgContainerManager::newMsgContainer(DSite* site) {
  FreeListEntry *f=getOne();
  MsgContainer *msgC;
  if(f==NULL) 
    msgC=new MsgContainer();
  else
    msgC = new (f) MsgContainer;
  msgC->init(site);
  ++wc;
  return msgC;
}

void MsgContainerManager::deleteMsgContainer(MsgContainer* msgC) {
  if(msgC->checkFlag(MSG_HAS_MARSHALCONT) && msgC->cont!=0)
    msgC->transController->returnMarshaler((DPMarshaler *) msgC->cont);
  else if(msgC->checkFlag(MSG_HAS_UNMARSHALCONT) && msgC->cont!=0)
    msgC->transController->returnUnmarshaler((Builder *) msgC->cont);
  msgC->deleteSnapshot();

  FreeListEntry *f;
  --wc;
  f = (FreeListEntry*)(void*) msgC;
  if(putOne(f)) return;
  delete msgC;
  return;
}

void MsgContainerManager::deleteMsgContainer(MsgContainer* msgC,FaultCode fc) {
  if(msgC->getMessageType()<C_FIRST)
    msgC->destination->communicationProblem(msgC, fc);
  deleteMsgContainer(msgC);
}

int MsgContainerManager::getCTR(){ return wc;}

MsgContainerManager* msgContainerManager;
