/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
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

#include "builtins.hh"

#include "tcpTransObj.hh"
#include "comObj.hh"
#include "msgContainer.hh"
#include "byteBuffer.hh"

#include "os.hh"
#include <errno.h>

#define BYTE_DEF_SIZE 4096
#define MIN_FOR_HEADER 200// Minimal size available to even consider marshaling

#define HEADER 11
#define TRAILER 1
#define MUSTREAD 9

#define CF_FIRST 0
#define CF_CONT 1
#define CF_FINAL 2

extern int  globalOSWriteCounter;
extern int  globalOSReadCounter;
extern int  globalContCounter;

static int tcpTransObj_writeHandler(int fd,void *o) {
  return ((TCPTransObj *) o)->writeHandler(fd);
}

static int tcpTransObj_readHandler(int fd,void *o) {
  return ((TCPTransObj *) o)->readHandler(fd);
}

enum ErrorClass {
  GO_AHEAD,
  CONTINUE_LATER,
  PERM              // This perm only means the transObj should give up
};

ErrorClass classifyError() {
  switch(ossockerrno()) {
  case EINTR:
    return GO_AHEAD;

  case EHOSTUNREACH:
  case EAGAIN:
  case EINPROGRESS: 
  case ETIMEDOUT:
    return CONTINUE_LATER;

  case EPIPE:
  case ECONNRESET:
  case EBADF:
    return PERM;  // This connection is broken! The site might still be up.
  default: 
    PD((TCP_INTERFACE,"Unhandled error %d",ossockerrno()));
    return PERM;
  }
}

void TCPTransObj::init() {
  minSend=200; // Whatever... Not well used yet
  comObj=NULL;
  bufferSize=BYTE_DEF_SIZE;
  readBuffer->reinit();
  writeBuffer->reinit();
  fd=-1;
}

void TCPTransObj::close() {
  close(TRUE);
}

void TCPTransObj::close(Bool isrunning) {
  PD((TCP_INTERFACE,"TCPTransObj closing down"));

  if(fd!=-1) {
    OZ_unregisterRead(fd);  // Sometimes done twice!
    OZ_unregisterWrite(fd); // Sometimes done twice!
    osclose(fd);
    fd=-1;
  }
  // Must be last, this may be deleted or reused
  // implies that close may never be used twice!
  tcptransController->transObjFreed(comObj,this,isrunning);
}

void TCPTransObj::deliver() {
  // ComObj responsible for not doing this unnecesarily, OS-call
  // kost@ : what OS call? I see none..
  Assert(fd!=-1);
  OZ_registerWriteHandler(fd,tcpTransObj_writeHandler,(void *) this);
}

void TCPTransObj::readyToReceive() {
  OZ_registerReadHandler(fd,tcpTransObj_readHandler,(void *) this);
}

void TCPTransObj::setSite(DSite *site) {
  this->site=site;
}

void TCPTransObj::setOwner(ComObj *comObj) {
  this->comObj=comObj;
}

OZ_Return TCPTransObj::setUp(DSite *site,ComObj *comObj,OZ_Term settings) {
  this->site=site;
  setOwner(comObj);

  SRecord *s = tagged2SRecord(settings);
  int index = s->getIndex(oz_atom("fd"));
  if (index>=0) { 
    OZ_Term t0 = s->getArg(index);
    NONVAR(t0,t);
    if(!oz_isInt(t))
      OZ_typeError(-1,"Int");
    int fd=oz_intToC(t);
    this->fd=fd;
  }
  //  tcptransController->addRunning(comObj); Doing so gives problems at
  // !comObj->handover in connection.cc
  return PROCEED;
}

Bool TCPTransObj::hasEmptyBuffers() {
  return readBuffer->isEmpty() && writeBuffer->isEmpty();
}

TransController *TCPTransObj::getTransController() {
  return tcptransController;
}

inline void TCPTransObj::marshal(MsgContainer *msgC, int acknum) {
  int num=msgC->getMsgNum();
  Bool cont=msgC->checkFlag(MSG_HAS_MARSHALCONT);
  writeBuffer->marshalBegin();

  PD((TCP_INTERFACE,"---marshal: %s nr:%d ack:%d cont:%d",
      mess_names[msgC->getMessageType()],num,acknum,
      msgC->checkFlag(MSG_HAS_MARSHALCONT))); 

  writeBuffer->put(0xFF);          // Ctrl
  writeBuffer->putInt(acknum);     // Ack
  writeBuffer->putInt(0xFFFFFFFF); // Placeholder for framesize
  writeBuffer->put(msgC->getMessageType());  // MessageType
  if(cont) {
    Assert(msgC->getMessageType()<C_FIRST);
    writeBuffer->put(CF_CONT);     // CF
    writeBuffer->putInt(num);
  }
  else
    writeBuffer->put(CF_FIRST);

  writeBuffer->setSite(site);
  msgC->marshal(writeBuffer, tcptransController);

  Assert(writeBuffer->availableSpace()>=0); // Room for trailer?
  if(msgC->checkFlag(MSG_HAS_MARSHALCONT)) {
    globalContCounter++;
    comObj->msgPartlySent(msgC);
    writeBuffer->put(CF_CONT);
  }
  else {
    comObj->msgSent(msgC);
    writeBuffer->put(CF_FINAL);
  }
  writeBuffer->marshalEnd();           // Size will be written now
}

// Return 0 means invoke again, return 1 means done
int TCPTransObj::writeHandler(int fd) {
  int totLen,len,ret;
  BYTE *pos;
  MsgContainer *msgC;
  int acknum;
  totLen = writeBuffer->getUsed();

  if (totLen < minSend) {
    while (writeBuffer->availableSpace()>MIN_FOR_HEADER &&
	   (msgC=comObj->getNextMsgContainer(acknum))!=NULL) {
      marshal(msgC,acknum);
    }
  }

  if (writeBuffer->getUsed()==0) {
    PD((TCP_INTERFACE,"writeHandler of %d invoked with no jobs\n",
	site!=NULL?site->getTimeStamp()->pid:0));
    //    Assert(0);  // Could get here legally at lost connection  
    return 1;
  }

  while((totLen=writeBuffer->getUsed())>0) {
    len = writeBuffer->getWriteParameters(pos);
    Assert(len>0);
    globalOSWriteCounter++;
    ret=oswrite(fd,pos,len);
    PD((TCP_INTERFACE,"Os-written %d",ret));
    
    if(ret<0) {
      writeBuffer->clearWrite(0);

      switch(classifyError()) {
      case GO_AHEAD:
	continue;
      case CONTINUE_LATER:
	return 0;
      case PERM:
	// Inform comObj wich will close us
	comObj->connectionLost();
	// This object is now reused or removed. No fields may be altered
	return 0; // Don't unregister with 1 here since a new
	// register might have been done in the mean time.
      }
    }

    writeBuffer->clearWrite(ret);
    if(ret<len) {
      return 0;
    }
  }
  if (msgC!=NULL)
    return 0;
  else
    return 1;
}

inline
unmarshalReturn TCPTransObj::unmarshal() {
  BYTE b;
  int acknum;
  int framesize;
  int cf;
  MessageType type;
  MsgContainer *msgC;
  int msgnum;
  int t;

  b=readBuffer->get();              // Ctrl
  Assert(b==0xFF);
  acknum=readBuffer->getInt();      // Ack
  comObj->msgAcked(acknum);
  framesize=readBuffer->getInt();   // Framesize
  // ----------------------------------------- // Must read read
  if(readBuffer->canGet(framesize-MUSTREAD)) { // Can all be read?
    b=readBuffer->get();           // MessageType
    type = (MessageType) b;
    cf=readBuffer->get();          // CF

    if(cf==CF_FIRST)
      msgC=comObj->getMsgContainer();
    else {
      Assert(cf==CF_CONT);
      msgnum=readBuffer->getInt();   // MsgNr
      msgC=comObj->getMsgContainer(msgnum);
    }

    Assert(msgC!=NULL);
    msgC->setMessageType(type);

    // Unmarshal data
    readBuffer->setSite(site);
    readBuffer->setFrameSize(framesize-TRAILER); // How much can be unm.
    if (msgC->unmarshal(readBuffer, tcptransController)) {
      // Frame contents successfully unmarshaled.
      t=readBuffer->get();
      readBuffer->getCommit();           // A full frame read.
      if(t==CF_CONT) {
	Assert(msgC->checkFlag(MSG_HAS_UNMARSHALCONT));
	Assert(type<C_FIRST);
	comObj->msgPartlyReceived(msgC);
      }
      else {
	Assert(t==CF_FINAL && !msgC->checkFlag(MSG_HAS_UNMARSHALCONT));
	if(!comObj->msgReceived(msgC))
	  return U_CLOSED;
      }
      return U_MORE;
    }
    else {
      // Contents somehow corrupted.
      // Since messages have to be delivered in order we cannot just
      // discard this frame. Using TCP something must be seriously wrong.
      // Break the connection by telling the comObj it was lost. The comObj can
      // then decide on further actions.
      comObj->connectionLost();
      return U_CLOSED;
    }
  }
  else
    return U_WAIT;                       // Wait for more data
}

int TCPTransObj::readHandler(int fd) {  
  int ret,len,msgNum;
  BYTE *pos;
  Assert(fd == this->fd);

  // Read
  len=readBuffer->getReadParameters(pos);
  Assert(osTestSelect(fd,SEL_READ) && len>0);
  while (TRUE) {
    globalOSReadCounter++;
    ret = osread(fd,pos,len);

    if (ret<0) {
      switch(classifyError()) {
      case GO_AHEAD:
	break;
      case CONTINUE_LATER:
	goto done_reading;
      case PERM:
	// Inform comObj which will close us
	// Here a couple of frames that were read could be lost
	comObj->connectionLost();
	// This object is now reused or removed. No fields may be altered
	return 0; // Don't unregister with 1 here since a new
	          // register might have been done in the mean time.
      } 
    }
    else {
      PD((TCP_INTERFACE,"Os-read %d",ret));
      readBuffer->hasRead(ret);

      if(ret==0) { // EOF
	comObj->connectionLost();
	// This object is now reused or removed. No fields may be altered
	return 0; // Don't unregister with 1 here since a new
	          // register might have been done in the mean time.
      }
      if(ret<len)    // Assume all available was read
	goto done_reading;

      len=readBuffer->getReadParameters(pos);      
      if(len==0)     // readBuffer is full
	goto done_reading;
    }
  }

 done_reading:
  // Interpret (Allways interpret complete frames.)
  unmarshalReturn contin=U_MORE;

  readBuffer->unmarshalBegin();
  while(contin==U_MORE) {
    Assert(this->fd!=-1 && this->fd==fd);
    if(readBuffer->canGet(MUSTREAD))     // Includes previously read bytes
      contin=unmarshal();
    else
      break;
  }
  if(contin!=U_CLOSED)    // transobj could be passed on
    readBuffer->unmarshalEnd(); 

  return 0;
}

class ByteBlockManager: public FreeListManager {
public:
  int wc;

  ByteBlockManager():FreeListManager(BYTE_ByteBuffer_CUTOFF){
    wc = 0;
  };
  ~ByteBlockManager() {
    BYTE *bb;
    FreeListEntry *f;
    int l=length();
    for(int i=0;i<l;i++) {
      f=getOne();
      Assert(f!=NULL);
      bb = (BYTE*) f;
      delete [] bb;
    }
    Assert(length()==0);
  }
  BYTE * getByteBlock() {
    FreeListEntry *f=getOne();
    BYTE *bb;
    if(f==NULL) {
      bb=new BYTE[BYTE_DEF_SIZE];
    }
    else {
      bb = (BYTE*) f;
    }
    ++wc;
    return bb;
  }
  void deleteByteBlock(BYTE* bb) {
    FreeListEntry *f;
    --wc;
    f = (FreeListEntry*) bb;
    if(!putOne(f))
      delete [] bb;
    return;
  }

  int getCTR(){ return wc;}
}; 

ByteBlockManager byteBlockManager;

TransObj *TCPTransController::newTransObj() {
  FreeListEntry *f=getOne();
  TCPTransObj *tcpTransObj;
  if(f==NULL) {
    tcpTransObj=new TCPTransObj();
  }
  else {
    tcpTransObj = new (f) TCPTransObj;
  }
  tcpTransObj->readBuffer = byteBufferManager->getByteBuffer(
			       BYTE_DEF_SIZE,byteBlockManager.getByteBlock());
  tcpTransObj->writeBuffer = byteBufferManager->getByteBuffer(
			       BYTE_DEF_SIZE,byteBlockManager.getByteBlock());
  tcpTransObj->init();

  ++wc;
  return tcpTransObj;
}
  
void TCPTransController::deleteTransObj(TransObj* transObj) {
  byteBlockManager.deleteByteBlock(
  byteBufferManager->deleteByteBuffer(((TCPTransObj *) transObj)
				      ->readBuffer));
  byteBlockManager.deleteByteBlock(
  byteBufferManager->deleteByteBuffer(((TCPTransObj *) transObj)
				      ->writeBuffer));
  FreeListEntry *f;
  --wc;
  f = (FreeListEntry*) transObj;
  if(putOne(f)) 
    return;

  delete (TCPTransObj *) transObj;
  return;
}

int TCPTransController::getInfo(int &size) {
  size = sizeof(TCPTransObj)+2*BYTE_DEF_SIZE;
  return getCTR();
}

TCPTransController::~TCPTransController() {
  TCPTransObj *tcpTransObj;
  FreeListEntry *f;
  int l=length();
  for(int i=0;i<l;i++) {
    f=getOne();
    Assert(f!=NULL);
    tcpTransObj = new (f) TCPTransObj;
    delete tcpTransObj;
  }
  Assert(length()==0);
}
