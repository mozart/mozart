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
  sendTime=-1;
  cont = (void *) 0;
  DebugCode(for (int i=0;i<MAX_NOF_FIELDS;i++)
	       msgFields[i].arg=NULL;)
}

//  void MsgContainer::setCntrl(Var cntrlVar) {
//    this->cntrlVar = cntrlVar;
//  }

//  Var MsgContainer::getCntrlVar() {
//    return cntrlVar;
//  }

void MsgContainer::setMsgNum(int msgNum) {
  this->msgNum = msgNum;
}

int MsgContainer::getMsgNum() {
  return msgNum;
}

void MsgContainer::setSendTime(int sendTime) {
  this->sendTime = sendTime;
}

int MsgContainer::getSendTime() {
  return sendTime;
}

//  void MsgContainer::setNext(MsgContainer* next) {
//    this->next = next;
//  }

//  MsgContainer* MsgContainer::getNext() {
//    return next;
//  }

MessageType MsgContainer::getMessageType() {
  return mt;
}

void MsgContainer::setMessageType(MessageType mt) {
  this->mt=mt;
}

void MsgContainer::setFlag(int flag) {
  flags |= flag;
}

int MsgContainer::getFlags() {
  return flags;
}

Bool MsgContainer::checkFlag(int flag) {
  return flags & flag;
}

void MsgContainer::clearFlag(int flag) {
  flags &= ~flag;
}

DSite* MsgContainer::getDestination() { 
  return destination; 
}

void MsgContainer::takeSnapshot() {
  Assert(!checkFlag(MSG_HAS_MARSHALCONT));
  Assert(!checkFlag(MSG_HAS_UNMARSHALCONT));

  //
  for(int i = 0 ; i < MAX_NOF_FIELDS; i++) {
    switch(msgFields[i].ft) {
    case FT_FULLTOPTERM:
    case FT_TERM:
      {
	OZ_Term t = (OZ_Term) msgFields[i].arg;
	// currently at most one term per message:
	Assert(msgTS == (MsgTermSnapshot *) 0);
	msgTS = takeTermSnapshot(t, destination);
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

MsgContainer *MsgContainerManager::newMsgContainer(DSite* site) {
  FreeListEntry *f=getOne();
  MsgContainer *msgC;
  if(f==NULL) 
    msgC=new MsgContainer();
  else
    GenCast(f,FreeListEntry*,msgC,MsgContainer*);
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
  GenCast(msgC,MsgContainer*,f,FreeListEntry*);
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
