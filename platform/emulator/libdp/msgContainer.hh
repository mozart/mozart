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
  MSG_HAS_CREDIT = 1,
  MSG_HAS_MARSHALCONT = 2,
  MSG_HAS_UNMARSHALCONT = 4,
  MSG_WRITEMSG = 8,
  MSG_READMSG = 0x10,
  MSG_HAS_MSGNUM = 0x20,
  MSG_HAS_LARGEMSGNUM = 0x40
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
  DSite *creditSite;
  // kost@ : currently, MsgContainer can contain at most one OZ_Term.
  // Otherwise MsgTermSnapshot"s are to go into msgField"s;
  MsgTermSnapshot *msgTS;

protected: //AN Devel
  struct msgField msgFields[MAX_NOF_FIELDS];
private:
  // For suspendable marshaler:
  // opaque argument for continuing marshaling message fields;
  void *cont;
  TransController *transController;

//  Var cntrlVar;
  int msgNum;
  int sendTime;

public:
protected:
  DSite *destination;
public:
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

  DSite* getImplicitMessageCredit();
  void setImplicitMessageCredit(DSite* s);

  //
  DebugCode(DSite* getDestination() { return (destination); });

  //
  void takeSnapshot() {
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

  //
  void deleteSnapshot() {
    if (msgTS) {
      deleteTermSnapshot(msgTS);
      // should not be reused before 'init':
      DebugCode(msgTS = (MsgTermSnapshot *) -1);
    }
  }

  //
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




