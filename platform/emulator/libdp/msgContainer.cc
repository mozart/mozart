#include "msgContainer.hh"
#include "transObj.hh"

void MsgContainer::init(DSite *site) {
  flags=0;
  destination = site;
  creditSite = NULL;
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

void MsgContainer::setImplicitMessageCredit(DSite *s) {
  creditSite = s;
}

DSite* MsgContainer::getImplicitMessageCredit() {
  return creditSite;
}

DSite* MsgContainer::getDestination() { 
  return destination; 
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
