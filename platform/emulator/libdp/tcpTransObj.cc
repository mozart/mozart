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

// kost@ : does not look nice...
#define HEADER 11
#define TRAILER 1
#define MUSTREAD 9

// Minimal size available to even consider marshaling;
#define MIN_FOR_HEADER 64

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

void TCPTransObj::init()
{
  Assert(bufferSize > 0);
  comObj = NULL;
  fd = -1;
  readBuffer->reinit();
  writeBuffer->reinit();
}

void TCPTransObj::close() {
  close(TRUE);
}

void TCPTransObj::close(Bool isrunning) {
  PD((TCP_INTERFACE,"TCPTransObj closing down"));

//    fprintf(stdout, "close transObj %p (pid %d), fd=%d\n",
//        this, osgetpid(), fd);
//    fflush(stdout);

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
  Assert(fd != -1);
  Assert(writeBuffer != (ByteBuffer *) -1);
  OZ_registerWriteHandler(fd,tcpTransObj_writeHandler,(void *) this);
}

void TCPTransObj::readyToReceive() {
  Assert(fd != -1);
  Assert(readBuffer != (ByteBuffer *) -1);
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
  Bool cont=msgC->checkFlag(MSG_HAS_MARSHALCONT);
  writeBuffer->marshalBegin();

  PD((TCP_INTERFACE,"---marshal: %s ack:%d cont:%d",
      mess_names[msgC->getMessageType()],acknum,
      msgC->checkFlag(MSG_HAS_MARSHALCONT)));

  writeBuffer->put(0xFF);          // Ctrl
  writeBuffer->putInt(acknum);     // Ack
  writeBuffer->putInt(0xFFFFFFFF); // Placeholder for framesize
  writeBuffer->put(msgC->getMessageType());  // MessageType
  if (cont) {
    Assert(msgC->getMessageType()<C_FIRST);
    writeBuffer->put(CF_CONT);     // CF
    Assert(msgC->getMsgNum()!=NO_MSG_NUM);
    writeBuffer->putInt(msgC->getMsgNum());
  } else {
    Assert(msgC->getMsgNum()==NO_MSG_NUM);
    writeBuffer->put(CF_FIRST);
    mess_counter[msgC->getMessageType()].send();
  }

  writeBuffer->setSite(site);
  msgC->marshal(writeBuffer, tcptransController);

  Assert(writeBuffer->availableSpace()>=1); // Room for trailer?
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
int TCPTransObj::writeHandler(int fd)
{
  int totLen,len,ret;
  BYTE *pos;
  MsgContainer *msgC;
  int acknum;
  totLen = writeBuffer->getUsed();

  while (writeBuffer->availableSpace() > MIN_FOR_HEADER &&
         (msgC = comObj->getNextMsgContainer(acknum)) != NULL) {
    marshal(msgC, acknum);
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
    //    printf("Rwrite fd:%d bytes:%d len:%d\n",fd,ret,len);
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
    // If we didn't write all the data stop writing and
    // stay registered.
    if(ret<len) {
      return 0;
    }
  }
  // Everything is written from the buffer. If there are queued messages
  // continue write(i.e have the write handler registered)
  if (comObj->hasQueued())
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

    //
    if (cf == CF_FIRST) {
      msgC = comObj->getMsgContainer();
      msgC->setMessageType(type);
      Assert(!msgC->checkFlag(MSG_HAS_UNMARSHALCONT));
    } else {
      Assert(cf == CF_CONT);
      msgnum = readBuffer->getInt();   // MsgNr
      msgC = comObj->getMsgContainer(msgnum);
      Assert(type == msgC->getMessageType());
    }
    Assert(msgC != (MsgContainer *) 0);

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
    } else {
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
  while (TRUE) {
    globalOSReadCounter++;
    ret = osread(fd,pos,len);
    // printf("Read fd:%d bytes:%d len:%d this:%x\n",fd,ret,len,(int)this);
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

TransObj *TCPTransController::newTransObj()
{
  FreeListEntry *f = getOne();
  TCPTransObj *tcpTransObj;

  if (f == NULL) {
    tcpTransObj = new TCPTransObj();
  } else {
    tcpTransObj = new (f) TCPTransObj;
  }

  tcpTransObj->bufferSize = ozconf.dpBufferSize;
  tcpTransObj->readBuffer =
    byteBufferManager->getByteBuffer(tcpTransObj->bufferSize,
                                     new BYTE[tcpTransObj->bufferSize]);
  tcpTransObj->writeBuffer =
    byteBufferManager->getByteBuffer(tcpTransObj->bufferSize,
                                     new BYTE[tcpTransObj->bufferSize]);
  tcpTransObj->init();

  ++wc;
//    fprintf(stdout, "new transObj %p (pid %d), rb=%p, wb=%p\n",
//        tcpTransObj, osgetpid(),
//        tcpTransObj->readBuffer, tcpTransObj->writeBuffer);
//    fflush(stdout);
  return (tcpTransObj);
}

void TCPTransController::deleteTransObj(TransObj* transObj)
{
  TCPTransObj *tcpTransObj = (TCPTransObj *) transObj;
  delete [] byteBufferManager->deleteByteBuffer(tcpTransObj->readBuffer);
  delete [] byteBufferManager->deleteByteBuffer(tcpTransObj->writeBuffer);

  DebugCode(tcpTransObj->comObj = (ComObj *) -1;);
  DebugCode(tcpTransObj->bufferSize = -1;);
  DebugCode(tcpTransObj->fd = -1;);
  DebugCode(tcpTransObj->readBuffer = (ByteBuffer *) -1;);
  DebugCode(tcpTransObj->writeBuffer = (ByteBuffer *) -1;);

  FreeListEntry *f;
  --wc;
  f = (FreeListEntry*) transObj;
  if (!putOne(f))
    delete (TCPTransObj *) transObj;

//    fprintf(stdout, "deleted transObj %p (pid %d)\n",
//        tcpTransObj, osgetpid());
//    fflush(stdout);
}

void TCPTransObj::setBufferSize(int bufSizeIn)
{
  Assert(bufSizeIn != bufferSize);
  //
  delete [] byteBufferManager->deleteByteBuffer(readBuffer);
  delete [] byteBufferManager->deleteByteBuffer(writeBuffer);

  //
  bufferSize = bufSizeIn;
  readBuffer =
    byteBufferManager->getByteBuffer(bufferSize, new BYTE[bufferSize]);
  readBuffer->reinit();
  readBuffer->unmarshalBegin(); // 'continue', to be precise;
  writeBuffer =
    byteBufferManager->getByteBuffer(bufferSize, new BYTE[bufferSize]);
  writeBuffer->reinit();

//    fprintf(stdout, "changed buffer size for transObj %p (pid %d)\n",
//        this, osgetpid());
//    fflush(stdout);
}

int TCPTransController::getInfo(int &size)
{
  // size returned is not precise (there can be buffers of different
  // size);
  size = sizeof(TCPTransObj)+2*(ozconf.dpBufferSize);
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
