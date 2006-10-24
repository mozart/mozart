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

#ifndef __MSGCONTAINER_HH
#define __MSGCONTAINER_HH


#include "dpBase.hh"
#include "msgType.hh"
#include "memaux.hh"
#include "comm.hh" // For FaultCode
#include "table.hh"
#include "dpMarshaler.hh"
#include "controlvar.hh"


#define MAX_NOF_FIELDS 5

class ByteBuffer;
class MsgContainerManager;

enum MsgContainerFlags {
  MSG_HAS_MARSHALCONT = 2,
  MSG_HAS_UNMARSHALCONT = 4,
};

typedef enum {
  FT_NONE,
  FT_NUMBER,
  FT_CREDIT,
  FT_TERM,
  FT_FULLTOPTERM,
  FT_TERMCONT,
  FT_STRING,
  FT_SITE
} fieldType;

struct msgField {
  void *arg;
  fieldType ft;
};

//
class MsgContainer : public CppObjMemory {
  friend class MsgContainerManager;
public:
  // for placement argument
  void* operator new(size_t,void*p) { return p; }
  void* operator new(size_t n) { return ::new char[n]; }

private:
  MessageType mt;
  int flags;
  struct msgField msgFields[MAX_NOF_FIELDS];
  // marshaling/unmarshaling continuation, if any;
  void *cont;

  //
  TransController *transController;
  int msgNum;
  int def_priority;
  LongTime sendTime;
  OZ_Term cntrlVar;
public:
  DSite *destination;
  MsgContainer *next;
  
  void init(DSite *site);

  inline void setCntrlVar(OZ_Term c)
  {
    cntrlVar = c;
  }
  
  inline void bindCntrlVar()
  {
    if (cntrlVar)
      {
	ControlVarResume(cntrlVar);
	cntrlVar = (OZ_Term) 0; 
      }
  }
  
  inline void setMsgNum(int msgNum) {
    this->msgNum = msgNum;
  }

  inline int getMsgNum() {
    return msgNum;
  }

  inline void setPriority(int priority) {
    this->def_priority = priority;
  }

  inline int getPriority() {
    return def_priority;
  }

  inline void setSendTime(LongTime *sendTime) {
    this->sendTime = *sendTime;
  }

  inline LongTime * getSendTime() {
    return &sendTime;
  }

  void setMessageType(MessageType mt);
  MessageType getMessageType() { return (mt); }

  inline void setFlag(int flag) {
    flags |= flag;
  }

  inline int getFlags() {
    return flags;
  }

  inline Bool checkFlag(int flag) {
    return flags & flag;
  }

  inline void clearFlag(int flag) {
    flags &= ~flag;
  }

  inline DSite* getDestination() { 
    return destination; 
  }

  //
  void gcStart();
  void gcFinish();

  void resetMarshaling();

  // includes MessageType-specific get_,put_,marshal,unmarshal,gcMsgC
#include "msgContainer_marshal.hh"
};

class MsgContainerManager: public FreeListManager {
public:
  MsgContainerManager():FreeListManager(1000){wc = 0;}
  ~MsgContainerManager();

  int wc;

  MsgContainer*newMsgContainer(DSite* site);
  MsgContainer*newMsgContainer(DSite* site,int priority);
  void deleteMsgContainer(MsgContainer* msgC);
  void deleteMsgContainer(MsgContainer* msgC,FaultCode fc);

  int getCTR();
}; 

extern MsgContainerManager *msgContainerManager;
#endif




