#include "builtins.hh"

#include "tcpTransObj.hh"
#include "comObj.hh"
#include "msgContainer.hh"
#include "byteBuffer.hh"

#include "os.hh"
#include <errno.h>

#define BYTE_DEF_SIZE 4096
#define MIN_FOR_HEADER 200// Minimal size available to even consider marshaling
static const int mustRead=9;

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

#ifdef WINDOWS
#define ETIMEDOUT    WSAETIMEDOUT
#define EHOSTUNREACH WSAEHOSTUNREACH
#endif

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
    return PERM;
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
  PD((TCP_INTERFACE,"TCPTransObj closing down"));
  //printf("cl on %x to %x\n",(int)this,(int)comObj);
//    printf("close %d %x %x %d\n", getpid(), (int)comObj, (int)this, fd);
//    comObj=NULL; // DEVEL
  if(fd!=-1) {
    OZ_unregisterRead(fd);  // Sometimes done twice!
    OZ_unregisterWrite(fd); // Sometimes done twice!
    osclose(fd);
    fd=-1;
  }
  // Must be last, this may be deleted or reused
  // implies that close may never be used twice!
  tcptransController->transObjFreed(comObj,this);
}

void TCPTransObj::deliver() {
  // ComObj responsible for not doing this unnecesarily, OS-call
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

void TCPTransObj::setUp(DSite *site,ComObj *comObj,OZ_Term settings) {
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
  tcptransController->addRunning(comObj);
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
  writeBuffer->putBegin();

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

  writeBuffer->fixsite=site;       // Fix because the marshaler uses site
  msgC->marshal(writeBuffer, tcptransController);

  if(msgC->checkFlag(MSG_HAS_MARSHALCONT)) {
    globalContCounter++;
    comObj->msgPartlySent(msgC);
    writeBuffer->put(CF_CONT);
  }
  else {
    comObj->msgSent(msgC);
    writeBuffer->put(CF_FINAL);
  }
  writeBuffer->putEnd();           // Size will be written now
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
//      printf("connectionLost writeHandler %d %d\n",
//             myDSite->getTimeStamp()->pid,site->getTimeStamp()->pid);
        comObj->connectionLost((void *) fd);
        // This object is now reused or removed. No fields may be altered
        return 0; // Don't unregister with 1 here since a new
                  // register might have been done in the mean time.
      }
    }

    writeBuffer->clearWrite(ret);
    if(ret<len) {
//        printf("writeHandler didn't write all (%d of %d)\n",ret,len);
      return 0;
    }
  }
  if (msgC!=NULL)
    return 0;
  else
    return 1;
}

inline unmarshalReturn TCPTransObj::unmarshal() {
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
  // -----------------------------------------
  if(readBuffer->canGet(framesize)) { // Can all be read (no commit yet)?
    b=readBuffer->get();           // MessageType
    GenCast(b,BYTE,type,MessageType);
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
    readBuffer->fixsite=site;
    // kost@ '!' handle error conditions!
    (void) msgC->unmarshal(readBuffer, tcptransController);

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
  else
    return U_WAIT;                       // Wait for more data
}

int TCPTransObj::readHandler(int fd) {
  //  printf("readHandler invoked\n");

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
//        printf("Error in read %d\n",ossockerrno());
      switch(classifyError()) {
      case GO_AHEAD:
        break;
      case CONTINUE_LATER:
        goto done_reading;
      case PERM:
        // Inform comObj which will close us
        // Here some information that was read could be lost AN
//  printf("connectionLost readHandler %d %d\n",myDSite->getTimeStamp()->pid,site->getTimeStamp()->pid);
        comObj->connectionLost((void *) fd);
        // This object is now reused or removed. No fields may be altered
        return 0; // Don't unregister with 1 here since a new
                  // register might have been done in the mean time.
      }
    }
    else {
      PD((TCP_INTERFACE,"Os-read %d",ret));
      readBuffer->hasRead(ret);

      if(ret==0) { // EOF
//      printf("connectionLost readHandler EOF %d %d %x %x\n",myDSite->getTimeStamp()->pid,site->getTimeStamp()->pid,comObj,this);
        comObj->connectionLost((void *) fd);
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

  readBuffer->getBegin();
  while(contin==U_MORE) {
    Assert(this->fd!=-1 && this->fd==fd);
    if(readBuffer->canGet(mustRead))     // Includes previously read bytes
      contin=unmarshal();
    else
      break;
  }
  if(contin!=U_CLOSED)    // transobj could be passed on
    readBuffer->getEnd();

  return 0;
}

class ByteBlockManager: public FreeListManager {
public:
  int wc;

  ByteBlockManager():FreeListManager(BYTE_ByteBuffer_CUTOFF){
    wc = 0;
  };
  BYTE * getByteBlock() {
    FreeListEntry *f=getOne();
    BYTE *bb;
    if(f==NULL) {
      bb=new BYTE[BYTE_DEF_SIZE];
    }
    else {
      GenCast(f,FreeListEntry*,bb,BYTE*);
    }
    ++wc;
    return bb;
  }
  void deleteByteBlock(BYTE* bb) {
    FreeListEntry *f;
    --wc;
    GenCast(bb,BYTE*,f,FreeListEntry*);
    if(putOne(f)) return;
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
    tcpTransObj->readBuffer = byteBufferManager->getByteBuffer(
                               BYTE_DEF_SIZE,byteBlockManager.getByteBlock());
    tcpTransObj->writeBuffer = byteBufferManager->getByteBuffer(
                               BYTE_DEF_SIZE,byteBlockManager.getByteBlock());
  }
  else {
    GenCast(f,FreeListEntry*,tcpTransObj,TCPTransObj*);
  }
  tcpTransObj->init();

  ++wc;
  return tcpTransObj;
}

void TCPTransController::deleteTransObj(TransObj* transObj) {
  FreeListEntry *f;
  --wc;
  GenCast((TCPTransObj *)transObj,TCPTransObj*,f,FreeListEntry*);
  if(putOne(f))
    return;

  byteBlockManager.deleteByteBlock(
  byteBufferManager->deleteByteBuffer(((TCPTransObj *) transObj)
                                      ->readBuffer));
  byteBlockManager.deleteByteBlock(
  byteBufferManager->deleteByteBuffer(((TCPTransObj *) transObj)
                                      ->writeBuffer));
  delete (TCPTransObj *) transObj;
  return;
}
