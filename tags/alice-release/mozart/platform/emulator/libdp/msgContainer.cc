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

void MsgContainer::init(DSite *site)
{
  flags = 0;
  destination = site;
  next = NULL;
  cntrlVar = (OZ_Term) 0;
  msgNum=-1;
  //  sendTime=-1; AN!
  cont = (void *) 0;
  DebugCode(for (int i=0;i<MAX_NOF_FIELDS;i++) msgFields[i].arg=NULL;);
}

// includes MessageType-specific get_,put_,marshal_,unmarshal_,gcMsgC_
#include "msgContainer_marshal.cc"

MsgContainerManager::~MsgContainerManager()
{
  MsgContainer *msgC;
  FreeListEntry *f;
  int l=length();
  for(int i=0;i<l;i++) {
    f=getOne();
    Assert(f!=NULL);
    GenCast(f,FreeListEntry*,msgC,MsgContainer*);
    delete msgC;
  }
  Assert(length()==0);
}

MsgContainer *MsgContainerManager::newMsgContainer(DSite* site,int priority)
{
  MsgContainer *ret = newMsgContainer(site);
  //  printf("msgCprio %d\n",priority + 1);
  ret->setPriority(priority+1);
  //printf("prioSet %d\n",ret->getPriority());
  return ret;
}

MsgContainer *MsgContainerManager::newMsgContainer(DSite* site)
{
  MsgContainer *msgC = new MsgContainer();
  msgC->init(site);
  ++wc;
  msgC->setPriority(MSG_PRIO_MEDIUM);
  return msgC;
}

void MsgContainerManager::deleteMsgContainer(MsgContainer* msgC)
{
  if (msgC->cont!=0 && msgC->checkFlag(MSG_HAS_MARSHALCONT))
    msgC->transController->returnMarshaler((DPMarshaler *) msgC->cont);
  else if (msgC->cont!=0 && msgC->checkFlag(MSG_HAS_UNMARSHALCONT))
    msgC->transController->returnUnmarshaler((Builder *) msgC->cont);
  delete msgC;
}

void MsgContainerManager::deleteMsgContainer(MsgContainer* msgC,FaultCode fc) {
  if(msgC->getMessageType()<C_FIRST)
    msgC->destination->communicationProblem(msgC, fc);
  deleteMsgContainer(msgC);
}

int MsgContainerManager::getCTR(){ return wc;}

MsgContainerManager* msgContainerManager;
