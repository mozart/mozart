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
#include "genhashtbl.hh"
#include "comm.hh" // For FaultCode
#include "table.hh"
#include "dpMarshaler.hh"

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
  FT_STRING,
  FT_SITE
} fieldType;

struct msgField {
  void *arg;
  fieldType ft;
};

//
class MsgContainer {
  friend MsgContainerManager;
private:
  MessageType mt;
  int flags;
  // kost@ : currently, MsgContainer can contain at most one OZ_Term.
  // Otherwise MsgTermSnapshot"s are to go into msgField"s;
  MsgTermSnapshot *msgTS;

  struct msgField msgFields[MAX_NOF_FIELDS];

  // For suspendable marshaler:
  // opaque argument for continuing marshaling message fields;
  void *cont;
  TransController *transController;

//  Var cntrlVar;
  int msgNum;
  int sendTime;

public:
  DSite *destination;
  MsgContainer *next;

  void init(DSite *site);

//    void setCntrl(Var cntrlVar);
//    Var getCntrlVar();

  void setMsgNum(int msgNum);
  int getMsgNum();

  int getSendTime();
  void setSendTime(int sendTime);

  MessageType getMessageType();
  void setMessageType(MessageType mt);

  void setFlag(int f); 
  int getFlags();
  Bool checkFlag(int f);
  void clearFlag(int f);

  DSite* getDestination(); 

  void takeSnapshot();
  void deleteSnapshot();

  void gcStart() { if (msgTS) mtsStartGC(msgTS); }
  void gcFinish() { if (msgTS) mtsFinishStartGC(msgTS); }

  // includes MessageType-specific get_,put_,marshal,unmarshal,gcMsgC
  #include "msgContainer_marshal.hh"
};

class MsgContainerManager: public FreeListManager {
public:
  MsgContainerManager():FreeListManager(1000){wc = 0;}
  int wc;

  MsgContainer*newMsgContainer(DSite* site);
  void deleteMsgContainer(MsgContainer* msgC);
  void deleteMsgContainer(MsgContainer* msgC,FaultCode fc);

  int getCTR();
}; 

extern MsgContainerManager *msgContainerManager;
#endif




