/*
  PERDIO Project, DFKI & SICS,
  Universit"at des Saarlandes
  Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
  Author: brand,scheidhr, mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  abstract interface to the ip network
  ------------------------------------------------------------------------*/



#ifdef WINDOWS
#include "wsock.hh"
#else
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif
#include <sys/types.h>
#include <sys/utsname.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "os.hh"
#include "perdio.hh"
#include "marshaler.hh"
#include "comm.hh"
#include "msgbuffer.hh"
#include "perdio_debug.hh"





/* ************************************************************************ */
/*  SECTION ::  Enums & Defines                                            */
/* ************************************************************************ */

#define NETWORK_ERROR(Args) {error Args;}
#define IMPLEMENT(Args) {error Args;}


/* free list sizes */

#define SITE_CUTOFF    100
#define MESSAGE_CUTOFF 10
#define READ_CONNECTION_CUTOFF  30
#define WRITE_CONNECTION_CUTOFF  30
#define NO_MSG_NUM -1

#define TCP_MSG_SIGN_BITS 3
#define INT_IN_BYTES_LEN 6
#define CONNECTION REMOTE
#define REMOTESITE SITE

#define BYTEBUFFER_SIZE 2048
#define BYTEBUFFER_CUTOFF 100
#define NetMsgBuffer_CUTOFF 50

static Bool net_timer_set;


#include <netdb.h>



//EK
// When I understand how to instruct dependensies
// uncoment this.
//#include <net_errors.cc>


static const int netIntSize=4;
static const int msgNrSize =4;
static const int ansNrSize =4;
static const int tcpSimpleHeaderSize = 1+netIntSize;
static const int tcpHeaderSize=1+netIntSize+msgNrSize + ansNrSize;
static const int tcpConfirmSize=tcpHeaderSize+netIntSize;

enum InterfaceCode{
  INVALID_VIRTUAL_PORT,
  SITE_NAME_UNKNOWN,
  TIMESTAMP_CONFLICT,
  NET_RAN_OUT_OF_TRIES,
  NET_OK,
  SITE_UNKNOWN,
  NET_CRASH
};
enum tcpMessageType {
  TCP_CLOSE_REQUEST_FROM_WRITER = 0,
  TCP_CLOSE_REQUEST_FROM_READER,
  TCP_CLOSE_ACK_FROM_WRITER,
  TCP_CLOSE_ACK_FROM_READER,
  TCP_CONNECTION, // from READER
  TCP_PACKET,     // from WRITER
  TCP_MYSITE,     // from WRITER
  TCP_MSG_ACK_FROM_READER,
  TCP_MSG_ACK_REQUEST_FROM_WRITER,
  TCP_NONE 
};

enum ConnectionFlags{
  CLOSING=1,
  OPENING=2,
  WANTS_TO_OPEN=4,
  WANTS_TO_ACK_CLOSE=8,
  WANTS_TO_CLOSE=16,
  CURRENT=32,
  WRITE_QUEUE=64,
  CAN_CLOSE = 128};

enum ByteStreamType{
  BS_None,
  BS_Marshal,
  BS_Write,
  BS_Read,
  BS_Unmarshal};


#define REFERENCES     4 // ATTENTION

#define MAXTCPCACHE 2

enum ipReturn{
  IP_OK= 0,
  IP_BLOCK= -1,
  IP_NO_MORE_TRIES= -2,
  IP_TIMER= -3,
  IP_CLOSE= -4,
  IP_TIMER_EXCEPTION= -5,
  IP_NET_CRASH= -6,
  IP_TEMP_BLOCK= -7,
  IP_PERM_BLOCK= -8
};

#define LOST -1

#ifdef XXGNUWIN32
#define ntohl __ntohl
#define ntohs __ntohs
#endif

/* ************************************************************************ */
/*  SECTION ::  Forward declarations                                        */
/* ************************************************************************ */

class Connection;
class ReadConnection;
class WriteConnection;
class BufferManager;
class Message;
class IOQueue;
class WriteConnectionManager;
class ReadConnectionManager;
class NetMsgBufferManager;
class RemoteSiteManager;
class ByteBufferManager;
class MessageManager;
class TcpCache;
class TcpOpenMsgBuffer;

/*
extern void dvset(int);
*/

inline void  close_crashed_connection(int,Connection*);
ipReturn tcpCloseWriter(WriteConnection *);
int intifyUnique(BYTE *);
TcpOpenMsgBuffer *tcpOpenMsgBuffer;
/* ************************************************************************ */
/*  SECTION ::  Global Variables                                            */
/* ************************************************************************ */

WriteConnectionManager *writeConnectionManager;
ReadConnectionManager *readConnectionManager;
NetMsgBufferManager *netMsgBufferManager;
RemoteSiteManager *remoteSiteManager;
ByteBufferManager *byteBufferManager;
MessageManager *messageManager;
TcpCache *tcpCache;

/* ************************************************************************ */
/*  SECTION ::  Debugging                                                   */
/* ************************************************************************ */

void printBytes(BYTE *s, int len)
{
  for (int i = 0; i < len; i++) {
    if ((i % 16) == 0) printf("-> %d: ",i);
    printf(" %02x",*s++);
    if (((i+1) % 16) == 0) printf("\n");
  }
  printf("\n\n");
}


/* *****************************************************************
 * *****************************************************************
 * Site                     basic info on sites - common to gnames
   RemoteSiteManager              SINGLE
   MySiteInfo               SINGLE
 * *****************************************************************
 * *************************************************************** */


/* ------------------------------------------------------------------------
   Site - common to network layer and global name mechanism
   ----------------------------------------------------------------------- */

#define NO_TIMESTAMP ~1
#define NO_PORT 0xffff


/* ************************************************************************ */
/*  SECTION ::  Network MsgBuffer and friends                                       */
/* ************************************************************************ */

class ByteBuffer{
  friend class ByteBufferManager;
  friend class NetMsgBuffer;
  friend class NetMsgBufferManager;
  
protected:
  BYTE buf[BYTEBUFFER_SIZE];
  ByteBuffer *next;
  
public: 
  ByteBuffer(){}
  
  BYTE *head(){return buf;}
  BYTE *tail(){return buf+BYTEBUFFER_SIZE-1;}
  void init() {next=NULL;}
};

class ByteBufferManager: public FreeListManager {
public:
  ByteBufferManager():FreeListManager(BYTEBUFFER_CUTOFF){}

  ByteBuffer*newByteBuffer(){
    FreeListEntry *f=getOne();
    ByteBuffer *bb;
    if(f==NULL) {bb=new ByteBuffer();}
    else{GenCast(f,FreeListEntry*,bb,ByteBuffer*);}
    return bb;}
  
  void deleteByteBuffer(ByteBuffer* bb){
    FreeListEntry *f;
    GenCast(bb,ByteBuffer*,f,FreeListEntry*);
    if(putOne(f)) return;
    delete bb;
    return;}
}; 





class NetMsgBuffer:public MsgBuffer{
  friend class NetMsgBufferManager;
protected:
  ByteBuffer *first; 
  ByteBuffer *start;
  ByteBuffer *last;
  BYTE *pos; 
  BYTE *curpos;
  BYTE *endpos; 
  int totlen;  /* include header */
  int type;
  Site *site;
  RemoteSite *remotesite;
  int availableSpace();
  
  ByteBuffer *beforeLast(){
    Assert(first!=last);
    ByteBuffer *bb=first;
    while(bb->next!=last){bb=bb->next;}
    return bb;}

  Bool within(BYTE*,ByteBuffer*);
  
public:
  NetMsgBuffer(){}
  
  void init(){type=BS_None;first=NULL;start=NULL;last=NULL;
  site=NULL;remotesite=NULL;}
  void setSite(Site *s);
  Site* getSite();
  char *siteStringrep();
  int getTotLen();
  void removeFirst(){
    // EK 
    // should not be used
    Assert(first!=last);
    ByteBuffer *bb=first;
    first=bb->next;
    byteBufferManager->deleteByteBuffer(bb);}
  

  ByteBuffer* getAnother();

  void getSingle();

/* marshall    beg:first->head()  pos=next free slot OR null */
  
  

  void marshalBegin(){
    PD((MARSHAL_BE,"marshal begin"));
    Assert(type==BS_None);
    Assert(first==NULL);
    Assert(last==NULL);
    first=getAnother();
    last=first;
    totlen= 0;
    type=BS_Marshal;
    pos=first->head()+tcpHeaderSize;}

  void put(BYTE b){
    Assert(type==BS_Marshal);
    if(pos==NULL){
      PD((BUFFER,"bytebuffer alloc Marshal: %d",no_bufs()));
      ByteBuffer *bb=getAnother();
      last->next=bb;
      last=bb;
      totlen += BYTEBUFFER_SIZE;
      pos=bb->head();
      *pos++=b;
      return;}
    if(pos==last->tail()){
      *pos=b;
      pos=NULL;
      return;}
    *pos++=b;}

  /* INTERFACE  pos=first->head()  endpos= first free slot */

  void marshalEnd(){
    Assert(type==BS_Marshal);
    endpos=pos;
    pos=first->head();
    if(endpos==NULL) {totlen +=BYTEBUFFER_SIZE;}
    else {totlen +=endpos-last->head();}
    type=BS_None;}

  /* write  pos=first to write  endpos= first free slot */

  void beginWrite(RemoteSite*); /* putting end packet header */
  void PiggyBack();


  // EK
  //
  // Redo this implementation.

  void sentFirst();

  int getWriteLen();
  

  BYTE* getWritePos();

  void incPosAfterWrite(int i);

  int calcTotLen();

  void writeCheck();
      

  /* read  endpos=first free slotpos (if NULL then last is full)
           pos= first BYTE of unhandled message 

     after read endpos updated */
     
  
  BYTE* initForRead(int&);
  BYTE* beginRead(int&);
  void afterRead(int);

  /* interface pos=first BYTE curpos=last BYTE (may be in different buffers)
     between curpos and endpos BYTES in next message */

  void beforeInterpret(int);
  void afterInterpret();
  BYTE getInRead();
  int interLen(); /* used for debugging only */

  /* unmarshall pos=first unread BYTE curpos last unread BYTE */

  void unmarshalBegin(){
    type=BS_Unmarshal;
    Assert(within(pos,first));
    if(first==last) {
      Assert(within(curpos,first));
      Assert(curpos>=pos);}
    else {Assert(within(curpos,last));}}

  BYTE get(){
    Assert(type==BS_Unmarshal);
    Assert(pos!=NULL);
    if(pos==curpos){
      pos=NULL;
      Assert(first==last);
      return *curpos;}
    if(pos==first->tail()){
      BYTE ch;
      Assert(first!=last);
      ch=*pos;
      first=first->next;
      pos=first->head();
      return ch;}
    return *pos++;}

  void unmarshalEnd(){
    Assert(type==BS_Unmarshal);    
    type=BS_None;
    Assert(pos==NULL);}

  int no_bufs();

  void dumpByteBuffers(){
    while(first!=last) {
      ByteBuffer *bb=first;
      first=bb->next;
      byteBufferManager->deleteByteBuffer(bb);
    }
    byteBufferManager->deleteByteBuffer(first);
    first=last=NULL;
  }
};

void NetMsgBuffer::setSite(Site *s){
  site = s;}
Site* NetMsgBuffer::getSite(){
  return site;}
char* NetMsgBuffer::siteStringrep(){
  return site->stringrep();}
int NetMsgBuffer::getTotLen() {return totlen;}

BYTE* NetMsgBuffer::getWritePos(){
  Assert(type==BS_Write);return pos;}

int NetMsgBuffer::availableSpace(){
  Assert(last!=NULL);
  if(endpos==NULL) return 0;
  Assert(within(endpos,last));
  return last->tail()-endpos+1;}

Bool NetMsgBuffer::within(BYTE *p,ByteBuffer *bb){
  if(p<bb->head()) return FALSE;
  if(p>bb->tail()) return FALSE;  
  return TRUE;}

int NetMsgBuffer::no_bufs(){
  int i=0;
  ByteBuffer *bb=first;
  while(bb!=NULL){
    i++;
    bb=bb->next;}
  return i;}

void NetMsgBuffer::writeCheck(){
  Assert(type==BS_Write);
  Assert(first==NULL);
  Assert(last==NULL); 
  return;}

void NetMsgBuffer::sentFirst(){
  Assert(type==BS_Write);
  PD((BUFFER,"bytebuffer sent: %d",no_bufs()));
  if(first==last){
    first=last=NULL;
    return;}
  first=first->next;
  pos=first->head();}


int NetMsgBuffer::calcTotLen(){
  Assert(type==BS_Write);
  if(first==last){
    if(endpos!=NULL){return endpos-pos;}
    return first->tail()-pos+1;}
  else{
    int i=first->tail()-pos+1;
    ByteBuffer *bb=first->next;
    while(bb->next!=NULL){
      i+=BYTEBUFFER_SIZE;
      bb=bb->next;}
    Assert(bb==last);
    if(endpos==NULL){return i+BYTEBUFFER_SIZE;}
    return i+endpos-last->head();}}


void NetMsgBuffer::incPosAfterWrite(int i){
    Assert(type==BS_Write);
    Assert(within(pos,first));
    Assert(pos+i<=first->tail());
    pos +=i;}


void NetMsgBuffer::getSingle(){
  Assert(first==NULL);
  Assert(last==NULL);
  ByteBuffer *bb=getAnother();
  start = bb;
  first=bb;
  last=bb;}

int NetMsgBuffer::getWriteLen(){
  Assert(type==BS_Write);
  Assert(first!=NULL);
  if(first==last){
    if(endpos!=NULL) {return endpos-pos;}
    return first->tail()-pos+1;}
  return first->tail()-pos+1;}


class NetMsgBufferManager:public FreeListManager{
public:
  NetMsgBufferManager():FreeListManager(NetMsgBuffer_CUTOFF){}
  
  NetMsgBuffer*newNetMsgBuffer(Site *s){
    FreeListEntry *f=getOne();
    NetMsgBuffer *bb;
    if(f==NULL) {bb=new NetMsgBuffer();}
    else{GenCast(f,FreeListEntry*,bb,NetMsgBuffer*);}
    bb->setSite(s);
    return bb;}
  
  void deleteNetMsgBuffer(NetMsgBuffer* b){
    Assert(b->first == b->last);
    Assert(b->first == NULL);
    FreeListEntry *f;
    GenCast(b,NetMsgBuffer*,f,FreeListEntry*);
    if(putOne(f)) return;
    delete b;
    return;}
  
  void dumpNetMsgBuffer(NetMsgBuffer* nb) {
    nb->dumpByteBuffers();
    deleteNetMsgBuffer(nb);}
  
  NetMsgBuffer *getNetMsgBuffer(Site *s) {
    return newNetMsgBuffer(s);}
 
  void freeNetMsgBuffer(NetMsgBuffer* bs){
    deleteNetMsgBuffer(bs);}
};

ByteBuffer *NetMsgBuffer::getAnother(){
  return(byteBufferManager->newByteBuffer());}

class RemoteSite{
  friend class RemoteSiteManager;

  int sentMsgCtr;
  int recMsgCtr;
  int tmpMsgNum;
  SiteStatus status;
  int totalNrMsg;
  int totalMsgSize;
  int MsgMonitorSize;
  int MsgMonitorNr;
  void* MsgMonitorPtr;
  WriteConnection *writeConnection;
  ReadConnection *readConnection;
  Message *sentMsg;
  int  maxNrAck;
  int  maxSizeAck;
  int  recNrAck;
  int  recSizeAck;
public:
  Site* site;

protected:
  void init(Site*, int);
public:
  RemoteSite(): writeConnection(NULL),readConnection(NULL){} 
  
  int sendTo(NetMsgBuffer*,MessageType,Site*, int);
  void zeroReferences();
  void nonZeroReferences(){OZ_warning("We have pointers to us, soo?");}
  
  void writeConnectionRemoved();
  void readConnectionRemoved();
  
  SiteStatus siteStatus(){return status;}
  void sitePrmDwn();
  void siteTmpDwn();
  int incMsgCtr(){
    return ++sentMsgCtr; }
  int getMsgCtr(){
    return sentMsgCtr;}
  int getTmpMsgNum(){
    return tmpMsgNum++;}
  void receivedNewMsg(int Nr);
  int getRecMsgCtr();
  void storeSentMessage(NetMsgBuffer* bs);
  void ackReceived(int nr);
  time_t getTimeStamp();
  //Bool checkTimeStamp(time_t t);
  port_t getPort();
  ip_address getAddress();
  WriteConnection *getWriteConnection();
  ReadConnection *getReadConnection();
  void setWriteConnection(WriteConnection *r);
  void setReadConnection(ReadConnection *r);
  int discardUnsentMessage(int msgNum);
  int getQueueSize(int &noMsg) {Assert(0);return 0;} // ATTENTION
  MonitorReturn deMonitorQueue();
  void queueMessage(int size){
    Assert(totalNrMsg >=0 && totalMsgSize >=0);
    totalNrMsg++;
    totalMsgSize += size;
    PD((SITE,"New queue entry nr: %d size: %d",
	totalNrMsg,totalMsgSize));}
  void deQueueMessage(int size){
    totalNrMsg--;
    totalMsgSize -= size;
    Assert(totalNrMsg >=0 && totalMsgSize >= 0);
    /* 
       ATTENTION
       
    if (totalNrMsg <= MsgMonitorNr ||
	totalMsgSize <= MsgMonitorSize)
      queueSizeMsgs(totalNrMsg, totalMsgSize, MsgMonitorPtr);
      */
      PD((SITE,"Remove queue entry nr: %d size: %d",
	totalNrMsg,totalMsgSize));}
  void partlyDeQueueMessage(int size){
    Assert(totalNrMsg >0 && totalMsgSize >0 && size > 0);
    totalMsgSize -= size;
    Assert(totalMsgSize >= 0);
    PD((SITE,"Remove part of msg nr: %d totsize: %d size: %d",
	totalNrMsg,totalMsgSize, size));}

  MonitorReturn monitorQueueMsgs(int NoMess, int SizeMess, void *p);
  Bool deMonitorQueueMsg();
  int  getQueuesize(int &);
  void setMaxNrSizeAck(int mN, int mS);
  void receivedNewSize(int size);
  Site* getSite(){
    return site;}
};


/* ********************************************************************** 
   Message  
   ********************************************************************** */


class Message{
friend class IOQueue;
friend class MessageManager;
friend class RemoteSite;
friend class WriteConnection;
protected:
  Message *next;
  tcpMessageType type;
  NetMsgBuffer *bs;
  int remainder;
  int msgNum;
  Site *site;
  MessageType msgType;
  int storeIndx;
  
public:
  Message(){}
  void init(NetMsgBuffer *b, int n, 
	    Site *s, MessageType msg, int stI){
    next=NULL;
    bs=b;
    remainder=0;
    type=TCP_NONE;
    msgNum = n;
    site = s;
    msgType = msg;
    storeIndx = stI;}
  NetMsgBuffer* getMsgBuffer(){
    return bs;};
  void setType(tcpMessageType t){
    PD((MESSAGE,"set type %d",t));
    type=t;}
  tcpMessageType getType(){
    return type;}
  int getRemainder(){
    return remainder;} 
  void setRemainder(int i){
    PD((MESSAGE,"set remainder %d",i));
    remainder=i;}
  Message* getNext(){
    return next;}
  int getMsgNum(){
    return msgNum;}
  
};


class IOQueue {
private:
  Message *first;
  Message *last;
  void checkQueue();
public:
  IOQueue():first(NULL),last(NULL){}
  Message* getFirst();
  Message* removeFirst();
  Message* find(int msg){
    // EK
    // Rewrite, looks like shit...
    //
    Assert(((first==NULL) && (last==NULL))|| 
	   ((first!=NULL) && (last!=NULL)));
    Message *b=first;
    if (first == last)
      if (first != NULL && first -> msgNum == msg){
	first = last = NULL;
	return b;}
      else
	return NULL;
    if (first->msgNum == msg){
      first = first->next;
      return b;}
    while(b->next != NULL)
      if(b->next->msgNum == msg){
	Message *d = b->next;
	if(d == last){
	  last = b;
	  b->next = NULL; }
	else
	  b->next = d->next;
	return d;}
      else
	b = b->next;
    return NULL;}
  void enqueue(Message *m);
  void dequeue(Message *m);
};      


/* ********************************************************************** 
   Connection base-class
   ********************************************************************** */

class Connection {
public:
  int fd;               // LOST means no connection
  int flags;
  RemoteSite *remoteSite;
  
  Message* current;

  void init(RemoteSite *s);
  void setFlag(int f){flags |= f;}
  void clearFlag(int f){flags &= ~f;}
  Bool testFlag(int f){return f & flags;}


  Bool isWantsToAckClose(){
    return testFlag(WANTS_TO_ACK_CLOSE);}
  void clearWantsToAckClose(){ 
    Assert(testFlag(WANTS_TO_ACK_CLOSE));
    PD((CONNECTION,"clearWantsToAckClose site:%s",remoteSite->site->stringrep()));
    clearFlag(WANTS_TO_ACK_CLOSE);}


  
  Bool isOpening(){
    return testFlag(OPENING);}
  void setIncomplete(){
    Assert(!testFlag(CURRENT));
    PD((CONNECTION,"setIncomplete site:%s",remoteSite->site->stringrep()));   
    setFlag(CURRENT);}
  Bool isIncomplete(){
    return testFlag(CURRENT);}
  void setSite(RemoteSite *s){
    remoteSite=s;}
  void clearIncomplete(){
    Assert(testFlag(CURRENT));
    PD((CONNECTION,"clearIncomplete site:%s",remoteSite->site->stringrep()));    
    clearFlag(CURRENT);}
  void setClosing(){
    PD((CONNECTION,"setClosing site:%s",remoteSite->site->stringrep()));
    Assert(!testFlag(CLOSING));
    setFlag(CLOSING);}
  Bool isClosing(){
    return testFlag(CLOSING);}
  void clearClosing(){
    PD((CONNECTION,"clearClosing site:%s",remoteSite->site->stringrep()));
    Assert(testFlag(CLOSING));
    clearFlag(CLOSING);}
  Bool isWantsToOpen(){
    return testFlag(WANTS_TO_OPEN);} 
  
  /*
    EK can we compile this so remove it...
 
    time_t getTimeStamp(){
    return site->getTimeStamp();}
    void setTimeStamp(time_t t){
    Assert(site->getTimeStamp()==NO_TIMESTAMP);
    site->setTimeStamp(t);}
    */
  
  Bool isRemovable(){
    return flags==0;}
  RemoteSite *getRemoteSite(){
    return remoteSite;}
  void lostConnection(){
    fd=LOST;}
  void setFD(int f){
    fd=f;}
  int getFD(){
    return fd;}
  void addCurQueue(Message *m){
    PD((TCPQUEUE,"add writeCur m:%x r:%x",m,this));
    setIncomplete();
    current = m;}
  Message* getCurQueue(){
    Message *m = current;
    current = NULL;
    clearIncomplete();
    return m;
  }
};

class ReadConnection:public Connection{
  friend class ReadConnectionManager;
  friend class TcpCache;
protected:
  ReadConnection *next, *prev;
public:
  void informSiteRemove(){
    remoteSite->readConnectionRemoved();}
  
  void prmDwn();
  void informSiteAck(int m, int a, int s){
    PD((CONNECTION,"Ack received: %d waiting for: %d",
	a,remoteSite->getMsgCtr()));
    // EKT
    // This could be done in one method invokation,
    // They coulSreceivedNewMd allso be inlined...
    remoteSite->receivedNewMsg(m);
    remoteSite->ackReceived(a);
    remoteSite->receivedNewSize(s);}
  

   void addCurQueue(Message *m){
    PD((TCPQUEUE,"add readCur m:%x r:%x",m,this));
    setIncomplete();
    current = m;}

  void init(RemoteSite *s){
    next = prev = NULL;
    flags=0;
    remoteSite=s;}
  
  void disconnect(){
    Assert(next!=NULL);
    Assert(prev!=NULL);
    next->prev=prev;
    prev->next=next;}

  Bool canbeClosed(){
    if(isClosing()) return FALSE;
    if(isOpening()) return FALSE;

    return TRUE;}

};

class WriteConnection:public Connection{
  friend class WriteConnectionManager;
  friend class TcpCache;
protected:
  WriteConnection *next, *prev;
  BYTE intBuffer[INT_IN_BYTES_LEN];
  BYTE *bytePtr;
  int writeCtr;
  
  IOQueue writeQueue;
public:
  void tryToTakeDown(Bool askForAck){
    setCanClose();
    if(askForAck){
      return;}
    
    
  }
  void informSiteRemove(){
    remoteSite->writeConnectionRemoved();}
    
  void tmpDwn();
  Bool shouldSendFromUser(){
    if(isClosing()){PD((CONNECTION,"isClosing")); return FALSE;}
    if(isOpening()) {PD((CONNECTION,"isOpening")); return FALSE;}
    if(isWantsToAckClose()) {PD((CONNECTION,"isWantsToAckClose")); return FALSE;}
    if(isWantsToOpen()) {PD((CONNECTION,"isWantsToOpen")); return FALSE;}
    if(isIncomplete()) {PD((CONNECTION,"isIncompleteWrite")); return FALSE;}
    Assert(fd!=LOST);
    return TRUE;}

  void nowOpening();
  void setWantsToClose();
  void nowClosing();
  void setWantsToOpen();
  

  Bool goodCloseCand(){
    return (canbeClosed() && (!isWritePending()));}

  void opened(){
    Assert(testFlag(OPENING));
    PD((CONNECTION,"opened site:%s",remoteSite->site->stringrep()));
    clearFlag(OPENING);}
  
  Bool addByteToInt(BYTE msg){
    if((bytePtr - intBuffer) > INT_IN_BYTES_LEN)
      return false;
    *bytePtr++ = msg;
    if((bytePtr - intBuffer) == INT_IN_BYTES_LEN)
      {
	remoteSite->ackReceived(intifyUnique(intBuffer));
	return false;
      }
    return true;
  }
  void prmDwn();
  void addWriteQueue(Message* m,int msg){
    // EK fix this
    Assert(0);
  }
  void addWriteQueue(Message* m){
    setInWriteQueue();
    remoteSite->queueMessage(m->bs->getTotLen());
    writeQueue.dequeue(m);}
  Bool isInWriteQueue(){
    Assert(!testFlag(CURRENT));
    return testFlag(WRITE_QUEUE);}
  void setInWriteQueue(){
    writeCtr++;
    PD((CONNECTION,"setInWriteQueue s:%s ct:%d",remoteSite->site->stringrep(),writeCtr));
    setFlag(WRITE_QUEUE);}
  Message* getWriteQueue(){
    return writeQueue.removeFirst();}
  Bool isWantsToClose(){
    return testFlag(WANTS_TO_CLOSE);}
  Bool isOpening(){
    return testFlag(OPENING);}
  void incommingAck(){
    bytePtr = intBuffer;}
  Bool isWritePending(){
    return testFlag(WRITE_QUEUE|CURRENT);}
  void setWantsToAckClose(){
    Assert(!testFlag(WANTS_TO_ACK_CLOSE));    
    PD((CONNECTION,"setWantsToAckClose site:%s",remoteSite->site->stringrep()));
    setFlag(WANTS_TO_ACK_CLOSE);}
  
  void setCanClose(){
    // EK assertion?
    PD((CONNECTION,"You can now close site:%s",remoteSite->site->stringrep()));
    setFlag(CAN_CLOSE);}
  Bool isCanClose(){
    return testFlag(CAN_CLOSE);}
  void setOpening(){
    Assert(!testFlag(OPENING));
    PD((CONNECTION,"setOpening site:%s",remoteSite->site->stringrep()));
    setFlag(OPENING);}
  int discardUnsentMessage(int msgNum);
  Bool canbeClosed(){
    if(isClosing()) return FALSE;
    if(isOpening()) return FALSE;
    if(isWantsToAckClose()) return FALSE;
    // Ek is it writing? insert that here 
    return TRUE;}
  
  void init(RemoteSite *s){
    next = prev = NULL;
    flags=0;
    writeCtr=0;
    remoteSite=s;
    bytePtr = intBuffer + INT_IN_BYTES_LEN;
  }

  void disconnect(){
    Assert(next!=NULL);
    Assert(prev!=NULL);
    next->prev=prev;
    prev->next=next;}
};




void RemoteSite::zeroReferences(){
  if(writeConnection == NULL)
    return;
  
  if(writeConnection -> goodCloseCand())
    tcpCloseWriter(writeConnection);
  else
    writeConnection -> setCanClose();}
  
/* -------------------------------------------------------------------
            MySiteInfo
   ------------------------------------------------------------------- */

class MySiteInfo{
public:
  int        tcpFD;    // the file descriptor of the port
  RemoteSite       *site;
  int        maxNrAck;
  int        maxSizeAck;
}mySiteInfo;

/* -------------------------------------------------------------------
     RemoteSiteManager
   ------------------------------------------------------------------*/

class RemoteSiteManager: public FreeListManager{

  RemoteSite* newRemoteSite(){
    RemoteSite* s;
    FreeListEntry *f=getOne();
    if(f==NULL) {s=new RemoteSite();}
    else{GenCast(f,FreeListEntry*,s,RemoteSite*);}
    return s;}

  void deleteRemoteSite(RemoteSite *s){
    FreeListEntry *f;
    GenCast(s,RemoteSite*,f,FreeListEntry*);
    if(putOne(f)) {return;}
    delete s;
    return;}

public:
  RemoteSiteManager():FreeListManager(SITE_CUTOFF){}

  void freeRemoteSite(RemoteSite *s){ 
    deleteRemoteSite(s);}

  RemoteSite* allocRemoteSite(Site *s, int r){ 
    RemoteSite *rs=newRemoteSite();
    PD((SITE,"allocated a:%x ctr:%d rs:%x",s,r,rs));
    rs->init(s, r);
    return rs;}

};

void RemoteSite::writeConnectionRemoved(){
    writeConnection = NULL;
    if(readConnection == NULL){
      Assert(sentMsg == NULL);
      // EK insert this one when the real files
      // are inserted.
      // site->dumpRemoteSite(recMsgCtr);
      remoteSiteManager->freeRemoteSite(this);}}

void RemoteSite::readConnectionRemoved(){
    readConnection = NULL;
    if(writeConnection == NULL){
      Assert(sentMsg == NULL);
      // EK insert this one when the real files
      // are inserted.
      // site->dumpRemoteSite(recMsgCtr);
      remoteSiteManager->freeRemoteSite(this);}}
/* *******************************************************************
   *******************************************************************
            Connection    for all tcp connected-sites
	    ConnectionManager                                 SINGLE
	    TcpCache                                      SINGLE
   *******************************************************************
   ******************************************************************* */



/* *******************************************************************
            ConnectionManagers
   ******************************************************************* */

class ReadConnectionManager: public FreeListManager{
  void deleteConnection(ReadConnection *r){
    FreeListEntry *f;
    GenCast(r,ReadConnection*,f,FreeListEntry*);
    if(!putOne(f)) {delete r;}
    return;}
  ReadConnection* newConnection(RemoteSite *s,int fd0){
    FreeListEntry *f=getOne();
    ReadConnection *r;
    if(f==NULL) {r=new ReadConnection();}
    else {GenCast(f,FreeListEntry*,r,ReadConnection*);}
    r->init(s);
    r->fd=fd0;
    return r;}
public:
  ReadConnectionManager():FreeListManager(READ_CONNECTION_CUTOFF){} 
  
  void freeConnection(ReadConnection *r){ 
    PD((CONNECTION,"freed r:%x",r));
    Assert(r->isRemovable());
    RemoteSite *s=r->remoteSite;
    if(s!=NULL) {s->readConnectionRemoved();}
    deleteConnection(r);
    return;}

  ReadConnection *allocConnection(RemoteSite *s,int f){
    ReadConnection *r=newConnection(s,f);
    PD((CONNECTION,"allocated r:%x s:%x fd:%d",r,s,f));
    return r;}

};


class WriteConnectionManager: public FreeListManager{
  void deleteConnection(WriteConnection *r){
    FreeListEntry *f;
    GenCast(r,WriteConnection*,f,FreeListEntry*);
    if(!putOne(f)) {delete r;}
    return;}
  WriteConnection* newConnection(RemoteSite *s,int fd0){
    FreeListEntry *f=getOne();
    WriteConnection *r;
    if(f==NULL) {r=new WriteConnection();}
    else {GenCast(f,FreeListEntry*,r,WriteConnection*);}
    r->init(s);
    r->fd=fd0;
    return r;}
public:
  WriteConnectionManager():FreeListManager(WRITE_CONNECTION_CUTOFF){} 

  void freeConnection(WriteConnection *r){ 
    PD((CONNECTION,"freed r:%x",r));
    Assert(r->isRemovable());
    RemoteSite *s=r->remoteSite;
    if(s!=NULL) {s->writeConnectionRemoved();}
    deleteConnection(r);
    return;}

  WriteConnection *allocConnection(RemoteSite *s,int f){
    WriteConnection *r=newConnection(s,f);
    PD((CONNECTION,"allocated r:%x s:%x fd:%d",r,s,f));
    return r;}

};

/* TODO */

Bool noaccept() {NETWORK_ERROR(("not implemented"));return FALSE;}
Bool yesaccept() {NETWORK_ERROR(("not implemented"));return FALSE;}

/* *******************************************************************
            TcpCache
   ******************************************************************* */


ipReturn tcpWriteError()
{
  switch(errno){
  case EPIPE:{
    OZ_warning("Connection socket lost: EPIPE");
    return IP_PERM_BLOCK;}
  case ECONNRESET:{
    OZ_warning("Connection socket lost: ECONNRESET");
    return IP_PERM_BLOCK;}
  case ETIMEDOUT:{
    OZ_warning("Connection socket temp: ETIMEDOUT");
    return IP_TEMP_BLOCK;}
  default:{
    OZ_warning("Unhandled error: %d please inform erik@sics.se",
	       errno);
    Assert(0);
    return IP_TEMP_BLOCK;}}
  return IP_TEMP_BLOCK;
}

void tcpCloseReader(Connection *);


class TcpCache {
  ReadConnection* readHead; 
  ReadConnection* readTail;
  WriteConnection* writeHead;
  WriteConnection* writeTail;
  int size;
  int max_size;
    
  void destroyRead(ReadConnection *r){
    PD((TCPCACHE,"destroy r:%x",r));
    Assert(r->canbeClosed());
    tcpCloseReader(r);
    return;}

  void destroyWrite(WriteConnection *r){
    PD((TCPCACHE,"destroy r:%x",r));
    Assert(r->canbeClosed());
    tcpCloseWriter(r);
    return;}
    
  void addToReadFront(ReadConnection *r){
    if(readHead==NULL){
      Assert(readTail==NULL);
      r->next=NULL;
      r->prev=NULL;
      readHead=r;
      readTail=r;
      return;}
    r->next=readHead;
    r->prev=NULL;
    readHead->prev=r;
    readHead=r;}

  void addToWriteFront(WriteConnection *r){
    if(writeHead==NULL){
      Assert(writeTail==NULL);
      r->next=NULL;
      r->prev=NULL;
      writeHead=r;
      writeTail=r;
      return;}
    r->next=writeHead;
    r->prev=NULL;
    writeHead->prev=r;
    writeHead=r;}

public:
  Bool accept;

  TcpCache():size(0),accept(FALSE){ 
    readHead=NULL;
    writeHead=NULL;
    readTail=NULL;
    writeTail=NULL;
    max_size=MAXTCPCACHE;
    PD((TCPCACHE,"max_size:%d",max_size));}

  void nowAccept();

  void adjust(){
    if(size>max_size) close();
    if(size>max_size + 5){
      if(noaccept()) accept=FALSE;}
    if(size<max_size && !accept){
      if(yesaccept()) accept=TRUE;}}

  void close() {
    Assert(0);
    /*
    
    PD((TCPCACHE,"close"));
    Assert(0);
    Connection *r=writeTail;
    Assert(r!=NULL);
    Assert(r!=head);
    while(!r->goodCloseCand()){
      r=r->prev;
      if(r==NULL) break;}
    if(r!=NULL) {destroy(r);return;}
    if((r==NULL) && (size<max_size * 2)) return;
    r=tail;
    while(!r->canbeClosed()){
      r=r->prev;
      if(r==NULL) break;}
    if(r==NULL) return;
    destroy(r);
    */
  }

  void addWrite(WriteConnection *w) {
    PD((TCPCACHE,"cache add write r:%x",w));
    addToWriteFront(w);
    size++;
    adjust();}
  void addRead(ReadConnection *r) {
    PD((TCPCACHE,"cache add r:%x",r));
    addToReadFront(r);
    size++;
    adjust();}

  void touchRead(ReadConnection *r) {
    PD((TCPCACHE,"cache touch r:%x",r));
    if(readHead!=r){
      unlinkRead(r);
      addToReadFront(r);}
    adjust();}

  void touchWrite(WriteConnection *w) { 
    PD((TCPCACHE,"cache touch r:%x",w));
    if(writeHead!=w){
      unlinkWrite(w);
      addToWriteFront(w);}
    adjust();}

  void removeWrite(WriteConnection *w) {
    PD((TCPCACHE,"cache remove r:%x",w));
    unlinkWrite(w);
    writeConnectionManager->freeConnection(w);
    size--;}
  
  void removeRead(ReadConnection *r) {
    PD((TCPCACHE,"cache remove r:%x",r));
    unlinkRead(r);
    readConnectionManager->freeConnection(r);
    size--;}

  void unlinkRead(ReadConnection *r){
    Assert((readHead!=NULL && (readTail!=NULL)));
    PD((TCPCACHE,"cache unlink r:%x",r));
    if(readTail==r) {
      if(readHead==r){
	readTail=NULL;
	readHead=NULL;
	return;}
      readTail=r->prev;
      readTail->next=NULL;
      return;}
    if(readHead==r) {
      readHead=r->next;
      readHead->prev=NULL;
      return;}
    r->disconnect();}

  void unlinkWrite(WriteConnection *r){
    Assert((writeHead!=NULL && (writeTail!=NULL)));
    PD((TCPCACHE,"cache unlink r:%x",r));
    if(writeTail==r) {
      if(writeHead==r){
	writeTail=NULL;
	writeHead=NULL;
	return;}
      writeTail=r->prev;
      writeTail->next=NULL;
      return;}
    if(writeHead==r) {
      writeHead=r->next;
      writeHead->prev=NULL;
      return;}
    r->disconnect();}

  Bool has_place(){
    if(size<max_size) return TRUE;
    return FALSE;}
};



/* *****************************************************************
 * *****************************************************************
 *   Message              for partially sent/received msgs
     MessageManager        SINGLE
     IOQueue
     TcpQueues            SINGLE
 * *****************************************************************
 * *************************************************************** */


class MessageManager: public FreeListManager {
private:
  int Msgs;
  Message * newMessage(){
    FreeListEntry *f=getOne();
    Message *m;
    if(f==NULL) {m=new Message();}
    else {GenCast(f,FreeListEntry*,m,Message*);}
    return m;}

  void deleteMessage(Message *m){
    FreeListEntry *f;
    GenCast(m,Message*,f,FreeListEntry*);
    if(!putOne(f)) {delete m;}
    return;}

public:
  MessageManager():FreeListManager(MESSAGE_CUTOFF){Msgs = 0;};

  Message * allocMessage(NetMsgBuffer *bs, int msgNum = 0,
			 Site *s, MessageType b, int i){
    Message*m = newMessage();
    PD((MESSAGE,"allocate  nr:%d", --Msgs));
    m->init(bs,msgNum,s,b,i);
    return m;}

  Message * allocMessage(NetMsgBuffer *bs,tcpMessageType t,int rem){
    Message*m = newMessage();
    PD((MESSAGE,"allocate DANGER r:%x nr:%d", --Msgs));
    // EK
    //
    // When is this method invokated....
    // Dangerous. 
    //
    m->init(bs,0,NULL,M_LAST,0);
    m->setRemainder(rem);
    m->setType(t);
    return m;}
  
  void freeMessage(Message *m){
    PD((MESSAGE,"freed nr: %d", ++Msgs));
    deleteMessage(m);}

  void freeMessageAndMsgBuffer(Message *m){
    netMsgBufferManager->dumpNetMsgBuffer(m->bs);
    PD((MESSAGE,"BM freed nr:%d", ++Msgs));
    deleteMessage(m);}
  

};



int WriteConnection::discardUnsentMessage(int msgNum){
  if(isInWriteQueue()){
    Message *m  = writeQueue.find(msgNum);
    if (m != NULL){
      NetMsgBuffer *bs = m->bs;
	messageManager->freeMessage(m);
	return (int) bs;}}
  return MSG_SENT;}


void ReadConnection::prmDwn(){
  // EK what to do with the handler?
  osclose(fd);
  if(current!=NULL) 
    messageManager->freeMessage(current);
}
void WriteConnection::prmDwn(){
    Message *m;
     osclose(fd);
     //EK 
     // What to do with the close handler?
    if(current!=NULL) {
      remoteSite->site->communicationProblem(current->msgType, current->site,
			   current->storeIndx,COMM_FAULT_PERM_NOT_SENT,
			   (FaultInfo) current->bs);
      messageManager->freeMessage(current);}

    while((m = writeQueue.removeFirst()) && m != NULL) {
      remoteSite->site->communicationProblem(m->msgType, m->site, m->storeIndx,
			   COMM_FAULT_PERM_NOT_SENT,(FaultInfo) m->bs);
      messageManager->freeMessageAndMsgBuffer(m);}}

void WriteConnection::tmpDwn(){
    Message *m;
    if(current!=NULL) {
      remoteSite->site->communicationProblem(current->msgType, current->site,
			   current->storeIndx,COMM_FAULT_TEMP_NOT_SENT,(int) current->bs);}
    while((m = writeQueue.removeFirst()) && m != NULL) {
      remoteSite->site->communicationProblem(m->msgType, m->site, m->storeIndx,
			   COMM_FAULT_TEMP_NOT_SENT,(int) m->bs);;}}


inline void TcpCache::nowAccept(){
  Assert(accept==FALSE);
  accept=TRUE;}

/*
void Connection::clearInFastQueue(){
  writeCtr--;
  PD((CONNECTION,"clearInFastQueue s:%x ct:%d",site,writeCtr));
  if(findWriteFastQueue()!=NULL){
    Assert(writeCtr>0);
    return;}
  clearFlag(WRITE_FAST);}

  void Connection::clearInSlowQueue(){
writeCtr--;
  PD((CONNECTION,"clearInSlowQueue s:%x ct:%d",site,writeCtr));
  if(findWriteSlowQueue()!=NULL){ 
  Assert(writeCtr>0);
    return;}
    clearFlag(WRITE_SLOW);}
  
  */



/* *****************************************************************
 * *****************************************************************
 * methods from Site obj that handles messages...
 * *****************************************************************
 * *************************************************************** */

void RemoteSite::storeSentMessage(NetMsgBuffer* bs)
  {
    //EK 
    // maybe an own init for this type of message...
    int msg = getMsgCtr();
    queueMessage(bs->getTotLen());
    Message *m=
      messageManager->allocMessage(bs,msg,NULL,M_LAST,0);
    m->next = sentMsg;
    sentMsg = m;
  }
    
void RemoteSite::ackReceived(int nr)
  {
    int ctr = sentMsgCtr;
    PD((SITE,"Ack nr:%d received, total sent: %d",nr,ctr));
    Message *ptr = sentMsg;
    Message *old;

    Assert(ctr >= nr);
    if(ptr!=NULL){
      if(ctr == nr)
	sentMsg = NULL;
      else{
	while(ctr > nr){
	  old = ptr;
	  ptr = ptr->next;
	  if(ptr == NULL) break;
	  ctr = ptr -> msgNum;
	}
	    old->next = NULL;
	  }
	while(ptr!=NULL)
	  {
	    PD((REMOTESITE,"Removing from queue"));
	    old = ptr;
	    ptr=ptr->next;
	    deQueueMessage(old->getMsgBuffer()->getTotLen());
	    messageManager->freeMessageAndMsgBuffer(old);
	  }
      }
  }


int RemoteSite::discardUnsentMessage(int msgNum)
{
  //EKT
  //
  // This code may be compressed a lot.
  // With double pointers and so on....
  Message *m = sentMsg;
    if(m != NULL)
      {
	if(m->getMsgNum() == msgNum) 
	  {
	    NetMsgBuffer *bs = m->getMsgBuffer();
	    this->sentMsg = m->next;
	    messageManager->freeMessage(m);
	    return (int) bs;
	  }
	Message *ptr = m->next;
	while(ptr != NULL)
	  {
	    if(ptr->getMsgNum() == msgNum) 
	      {
		NetMsgBuffer *bs = ptr->getMsgBuffer();
		m->next = ptr->next;
		messageManager->freeMessage(ptr);
		return (int) bs;
	      }
	    m->next = ptr;
	    ptr = ptr->next;
	  }
      }
    return writeConnection->discardUnsentMessage(msgNum);
}


MonitorReturn RemoteSite::monitorQueueMsgs(int NoMess, int SizeMess, void* ptr)  {
  if(NoMess >= totalNrMsg)
    return NO_MSGS_THRESHOLD_REACHED;
  if(SizeMess >= totalMsgSize)
    return SIZE_THRESHOLD_REACHED;
  MsgMonitorNr = NoMess;
  MsgMonitorSize = SizeMess;
  MsgMonitorPtr = ptr;
  return MONITOR_OK;
}

MonitorReturn RemoteSite::deMonitorQueue(){
  if (MsgMonitorNr == -1 && MsgMonitorSize == -1)
    return NO_MONITOR_EXISTS;
  MsgMonitorNr = -1;
  MsgMonitorSize = -1;
  MsgMonitorPtr = NULL;
  return MONITOR_OK;
}

int RemoteSite::getQueuesize(int& NoMsg){
  NoMsg = totalNrMsg;
  return totalMsgSize;
}

/* *****************************************************************
            MsgBuffer operations
   *******************************************************************/

/* read       pos= first BYTE    endpos = first free slot or NULL
   interpret  pos= first BYTE curpos= last BYTE 
                              endpos= first free slot or NULL */

BYTE* NetMsgBuffer::initForRead(int &len){
  Assert(type==BS_None);
  PD((BUFFER,"bytebuffer alloc Read: %d",no_bufs()));
  pos=first->head();
  endpos=first->head();
  len=BYTEBUFFER_SIZE;
  type=BS_Read;
  return endpos;}

BYTE* NetMsgBuffer::beginRead(int &len){
  Assert(type==BS_None);
  type=BS_Read;
  if(endpos==NULL){
    if(pos==NULL){
      pos=first->head();
      endpos=first->head();
      len=BYTEBUFFER_SIZE;
      return endpos;}
    Assert(within(pos,first));    
    PD((BUFFER,"bytebuffer alloc Read: %d",no_bufs()));
    ByteBuffer *bb=byteBufferManager->newByteBuffer();
    last->next=bb;
    last=bb;
    len=BYTEBUFFER_SIZE;
    endpos=last->head();
    return endpos;}
  if(pos==NULL){pos=endpos;}
  else{Assert(within(pos,first));}
  len=last->tail()-endpos+1;
  Assert(len<BYTEBUFFER_SIZE);
  return endpos;}

void NetMsgBuffer::afterRead(int len){
  Assert(type==BS_Read);
  Assert(len<=BYTEBUFFER_SIZE);
  Assert(availableSpace()>=len);
  Assert(within(endpos,last));  
  type=BS_None;
  if(endpos+len-1==last->tail()){endpos=NULL;}
  else{endpos=endpos+len;}}

void NetMsgBuffer::beforeInterpret(int len){
  if(pos==NULL) {return;} /* read only header for close message*/
  Assert(type==BS_None);
  Assert(within(pos,first));
  Assert(len<=0);
  if(endpos==NULL){
    curpos=last->tail()+len;}
  else{
    curpos=endpos+len-1;}}

void NetMsgBuffer::afterInterpret(){
  Assert(first==last);
  Assert(first!=NULL);
  pos=curpos+1;
  Assert(within(curpos,last));}
    
BYTE NetMsgBuffer::getInRead(){
  Assert(within(pos,first));
  if(pos==first->tail()){
    BYTE ch=*pos;
    if(first==last){
      pos=NULL;
      return ch;}
    PD((BUFFER,"bytebuffer dumped read (SPEC):%d",no_bufs())); 
    byteBufferManager->deleteByteBuffer(first);
    first=last;
    pos=first->head();
    return ch;}
  return *pos++;}

int NetMsgBuffer::interLen(){
  if(first==last){return curpos-pos+1+tcpHeaderSize;}
  int i=first->tail()-pos+1+tcpHeaderSize;
  ByteBuffer *bb=first->next;
  while(bb!=last){bb=bb->next;i+=BYTEBUFFER_SIZE;}
  return i+curpos-last->head()+1;}
  
/* *****************************************************************
 * *****************************************************************
 * *****************************************************************
 * TCP
 *      Startup
 *         createTcpPort      network side of initializing global port
 *      Writer   
 *         tcpOpen            writer wants to open connection 
 *         tcpConnectionHandler writer before connection acknowledgement
 *         tcpCloseHandler    handles reader-initiated close connections
 *         tcpOpenCheck       tries to open connections for pending msgs
 *         tcpWriteHandler    handles write-continuations (prev blocked) 
 *         tcpSend            basic send 
 *         
 *      Reader   
 *         tcpReadHandler     handles reads 
 *         tcpAcceptHandler   handles writer-inititated opens
 *         
 * *****************************************************************
 * *****************************************************************
 * *************************************************************** */

int tcpConnectionHandler(int,void *);


  

inline BYTE *int2net(BYTE *buf,int i){ 
  for (int k=0; k<4; k++) { 
    *buf++ = i & 0xFF; 
    i = i>>8;}
  return buf;}

inline BYTE *short2net(BYTE *buf,short i){ 
  for (int k=0; k<2; k++) { 
    *buf++ = i & 0xFF; 
    i = i>>8;}
  return buf;}

inline int net2int(BYTE *buf){
  unsigned int i1 = *buf++;
  unsigned int i2 = *buf++;
  unsigned int i3 = *buf++;
  unsigned int i4 = *buf++;
  return (int) (i1 + (i2<<8) + (i3<<16) + (i4<<24));}

inline short net2short(BYTE *buf){
  unsigned int i1 = *buf++;
  unsigned int i2 = *buf++;
  return (short) (i1 + (i2<<8));} 


void NetMsgBuffer::beginWrite(RemoteSite *s)
{
  Assert(type==BS_None);
  Assert(first!=NULL);
  remotesite =  s;
  BYTE* thispos= first->head();
  *thispos++=TCP_PACKET;
  type=BS_Write;
  int2net(thispos,totlen);
}

void NetMsgBuffer::PiggyBack()
{
  BYTE* thispos= first->head() + 5;
  int2net(thispos, remotesite->incMsgCtr());
  thispos += 4;
  int2net(thispos, remotesite->getRecMsgCtr());
  PD((SITE,"Message nr: %d Ack nr: %d inserted"
      ,remotesite->getMsgCtr(), remotesite->getRecMsgCtr()));
  remotesite->storeSentMessage(this);
}

#define OZReadPortNumber  9000
#define OZStartUpTries    490
#ifdef WINDOWS
/* connect under windows takes very long */
#define OZConnectTries    10
#else
#define OZConnectTries    200
#endif
#define OZWritePortNumber 9500


/* *****************************************************************
   *****************************************************************
     STARTUP
   *****************************************************************
 * *************************************************************** */

inline Bool createTcpPort_RETRY(){
  if(ossockerrno()==ENOENT) return TRUE;
  if(ossockerrno()==EADDRINUSE) return FALSE;
  return FALSE;}
  

static ipReturn createTcpPort(int port,ip_address &ip,port_t &oport,int &fd) 
{
  PD((TCP,"Start createTcpPort: p:%d",port));
  int smalltries=OZConnectTries;
  int bigtries=OZStartUpTries;

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  struct utsname auname;

  if(uname(&auname)<0) {NETWORK_ERROR(("createTcpPort"));}
  struct hostent *hostaddr;
  hostaddr=gethostbyname(auname.nodename);
  struct in_addr tmp;
  memcpy(&tmp,hostaddr->h_addr_list[0],sizeof(in_addr));
  ip=ntohl(tmp.s_addr);

retry:
  if(bigtries<0){
    PD((WEIRD,"bind - ran out of tries"));
    return IP_TIMER;} // ATTENTION - major fault
  fd=ossocket(PF_INET,SOCK_STREAM,0);
  if (fd < 0) {
    if(errno==ENOBUFS) return IP_TIMER;
    NETWORK_ERROR(("system:socket %d\n",errno));}
  addr.sin_port = htons(port);

  if(bind(fd,(sockaddr *) &addr,sizeof(struct sockaddr_in))<0) {
    PD((WEIRD,"create TcpPort bind redo port:%d errno:%d",
		  port,errno));
    osclose(fd);
    smalltries--;
    if(createTcpPort_RETRY() && smalltries>0) goto retry;
    bigtries--;
    port++;
    smalltries=OZConnectTries;
    goto retry;}
  PD((TCP,"createTcpPort: got ip:%u port:%u",ip,port));
  if (listen(fd,5)<0) {NETWORK_ERROR(("listen %d\n",errno));}

  struct sockaddr_in addr1;
  int length = sizeof(addr1);
  if (getsockname(fd, (struct sockaddr *) &addr1, &length) < 0) {
    NETWORK_ERROR(("getsockname %d\n",errno));
  }
  oport=ntohs(addr1.sin_port);

  PD((TCP,"create TcpPort OK"));
  return IP_OK;
}

/* *****************************************************************
   *****************************************************************
          WRITER
   *****************************************************************
   *************************************************************** */

/* *****************************************************************
     tcpOpen
   *************************************************************** */


#define TCPOPENMSGBUFFER_SIZE 100
class TcpOpenMsgBuffer:public MsgBuffer{
  BYTE buffer[TCPOPENMSGBUFFER_SIZE];
  BYTE *pos;
  int size;
public:
  TcpOpenMsgBuffer(){
    pos = NULL;}
  void put(BYTE b){
    Assert(size < TCPOPENMSGBUFFER_SIZE); 
    *pos++ = b;
    size++;}
  void marshalBegin(){
    Assert(pos == NULL);
    pos = buffer + 5;
    size = 0;}
  void marshalEnd(){ pos = NULL;}
  void beginWrite(BYTE b){
    Assert(pos == NULL);
    pos = buffer;
    *pos++=b;
    int2net(pos,size);
    size+=5;
    pos = NULL;}
  BYTE get(){
    return *pos++;}
  void unmarshalBegin(){
    Assert(pos == NULL);
    pos = buffer;}
  void unmarshalEnd(){
    pos = NULL;}
  Site *getSite(){
    Assert(0);
    return NULL;}
  int getLen(){
    return size;}
  BYTE* getBuf(){
    return buffer;}
  char* siteStringrep(){
    Assert(0);
    return "Assert(0)";}
  BYTE readyForRead(int &len){
    BYTE b=get();
    len = net2int(pos);
    pos += 4;
    return b;}
};

#define USE_OLD_PORT -1

inline Bool tcpOpen_RETRY_Old(){
  return FALSE;}

inline Bool tcpOpen_RETRY_New(){
  if(ECONNREFUSED) return TRUE;
  return FALSE;}


static ipReturn tcpOpen(RemoteSite *remoteSite,WriteConnection *r) 
{
  port_t aport;
    
  aport=remoteSite->getPort();
  PD((TCP,"reopen s:%s",
      remoteSite->site->stringrep()));
  
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr=htonl(remoteSite->getAddress());
  addr.sin_port = htons(aport);
  
  int tries=OZConnectTries;
  int fd;
  
retry:
  
  fd=ossocket(PF_INET,SOCK_STREAM,0);
  if (fd < 0) {
    error("ip - should not happen");
    
    if(errno==ENOBUFS){
      PD((WEIRD,"tcpOpen set on timer",errno));
      return IP_TIMER;}
    NETWORK_ERROR(("system:socket %d",errno));}

  /*  URGENT
      if(setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,1,sizeof(int))<0)
    {NETWORK_ERROR(("setsockopt"));}*/
    

  // critical region  
  if(osconnect(fd,(struct sockaddr *) &addr,sizeof(addr))==0) {
    PD((TCP,"open success p:%d fd:%d",aport,fd));
    r->setFD(fd);
    r->setOpening(); 
    
    PD((TCP,"Sending My Site Message..%s",mySite->stringrep()));
    
    tcpOpenMsgBuffer->marshalBegin();
    marshalNumber(mySiteInfo.maxNrAck, tcpOpenMsgBuffer);
    marshalNumber(mySiteInfo.maxSizeAck, tcpOpenMsgBuffer);
    mySite->marshalSite(tcpOpenMsgBuffer);
    tcpOpenMsgBuffer->marshalEnd();
    tcpOpenMsgBuffer->beginWrite(TCP_MYSITE);
    int bufSize = tcpOpenMsgBuffer->getLen();
    BYTE *buf=tcpOpenMsgBuffer->getBuf();
    fcntl(fd,F_SETFL,O_NDELAY);
    int written = 0;
    
    while(TRUE){
      int ret=oswrite(fd,buf+written,bufSize-written);
      written += ret;
      if(written==bufSize) break;
      if(ret<=0 && ret!=EINTR) { 
	error("ip + should not happen 2");
	NETWORK_ERROR(("acceptHandler:write %d\n",errno));
      }
    }
    
    OZ_registerReadHandler(fd,tcpConnectionHandler,(void *)r);
    PD((OS,"register READ %d - tcpConnectionHandler",fd));
    return IP_OK;}

  
  if(tcpOpen_RETRY_Old()) 
    {osclose(fd);return IP_TIMER;}
  if (ossockerrno()==ECONNREFUSED) 
    { close_crashed_connection(fd,r); return IP_NET_CRASH; }
  NETWORK_ERROR(("connect %d\n",errno));
  osclose(fd);
  tries--;
  if(tries<=0){
    if(tcpOpen_RETRY_New()){
      PD((WEIRD,"tcp open set timer %d",errno));
      return IP_TIMER;}
    PD((WEIRD,"tcp open ran out of tries %d",errno));      
    return IP_NO_MORE_TRIES;}
  addr.sin_port = htons(aport);
  goto retry;
}

/* ********************************************************************** 
    tcpCloseHandler
   ********************************************************************** */

inline ipReturn writeI(int,BYTE*);

inline void tcpSetTimer(){
  if(net_timer_set) return;
  PD((TCP,"timer set"));
  net_timer_set=TRUE;
  
  /* URGENT
  setTimer(NETTIME,tcpOpenCheck);*/
  NETWORK_ERROR(("not implemented"));}

inline void tcpWantsToOpen(Connection *r){
  //  Message *m=messageManager->allocMessage(NULL,r);
  // r->addOpenQueue(m);
  //tcpSetTimer();
  Assert(0);
}
    
inline void close_write_connection(int fd,WriteConnection *r){
  PD((TCP,"close connection r:%x",r));
  osclose(fd);
  r->lostConnection();
  r->clearClosing();
  r->informSiteRemove();
  tcpCache->unlinkWrite(r);
  if(r->isWritePending()){
    tcpWantsToOpen(r);
    return;}
  writeConnectionManager->freeConnection(r);}

inline void close_read_connection(int fd,ReadConnection *r){
  PD((TCP,"close connection r:%x",r));
  osclose(fd);
  r->lostConnection();
  r->clearClosing();
  r->informSiteRemove();
  tcpCache->unlinkRead(r);
  readConnectionManager->freeConnection(r);}

inline void close_crashed_connection(int fd,Connection *r){
  PD((TCP,"close crashed connection r:%x",r));
  osclose(fd);
  r->remoteSite->sitePrmDwn();
}

inline ipReturn write_ack_close(int fd,Connection *r){
  PD((TCP,"write_ack_close r:%x",r));
  BYTE msg;
  msg=TCP_CLOSE_ACK_FROM_WRITER;
  ipReturn ret=writeI(fd,&msg);
  if(ret==IP_OK) return IP_OK;
  return IP_BLOCK;}

ipReturn tcpCloseWriter(WriteConnection *r){
  PD((TCP,"tcpCloseWriter r:%x",r));
  OZ_warning("Closing connection, no check for acks!!!!");
  Assert(r->canbeClosed());
  int fd=r->getFD();
  Assert(fd!=LOST);
  BYTE msg;
  msg=TCP_CLOSE_REQUEST_FROM_WRITER;
  ipReturn ret=writeI(fd,&msg);
  if(ret==IP_OK) {
    r->setClosing();
    return IP_OK;}
  return IP_BLOCK;}

/*********************************************************************** 
  tcpCloseHandler - associated with WRITER
  ********************************************************************** */

inline ipReturn readI(int,BYTE *);

static int tcpCloseHandler(int fd,void *r0){
  WriteConnection *r=(WriteConnection *)r0;
  BYTE msg;
  ipReturn ret;


close_handler_read:
  PD((TCP,"tcpCloseHandler invoked r:%x",r));
  ret=readI(fd,&msg);
  PD((TCP,"tcpCloseHandler read b:%d r:%d",msg, ret));


  // EK
  // I hope this will work. If The read is blocked 
  // The close handler just suspends and waits for the next time
  // it is invoked.
  if(ret==IP_BLOCK){
    PD((TCP,"Close-handler is blocked, do Nothing"));
    return 0;}
  if(ret!=IP_OK){  // crashed Connection site 
    PD((WEIRD,"crashed Connection site %s",r->remoteSite->site->stringrep()));
    close_crashed_connection(fd,r);
    return 0;}
  if(msg==TCP_MSG_ACK_FROM_READER){
    PD((TCP,"Incomming Ack"));
    r->incommingAck();
    // EK This might be stupid. Just do the 
    // Ack receive initialization and then suspend.
    // Next time the closehandler is invoked
    // should all info be in the buffer.
    goto close_handler_read;
  }
  if(msg==TCP_CLOSE_REQUEST_FROM_READER){
    if(r->isClosing()){ /* writer has precedence */
      PD((WEIRD,"tcpCloseHandler writer precedence"));
      return 0;}
    if(r->isOpening()){
      PD((WEIRD,"tcpCloseHandler is opening "));
      r->setWantsToAckClose();
      return 0;}
    if(r->isIncomplete()){
      PD((WEIRD,"tcpCloseHandler isIncompletWrite "));
      r->setWantsToAckClose();
      return 0;}
    if(write_ack_close(fd,r)==IP_BLOCK){
      PD((WEIRD,"tcpCloseHandler blocked 1"));
      r->setWantsToAckClose();
      return 0;}}
  if(msg==TCP_CLOSE_ACK_FROM_READER){
    // EK I'm very suspisious against this one...
    int ret=write_ack_close(fd,r);
    if(ret==IP_BLOCK){
      PD((WEIRD,"tcpCloseHandler blocked 2 "));
      r->setWantsToAckClose();
      return 0;}
    PD((TCP,"tcpCloseHandler closes connection"));
    close_write_connection(fd,r);
    return 0;}
  if(r->addByteToInt(msg))
    goto close_handler_read;
  PD((TCP,"Close Handler done..."));
  return 0;
}
/* ********************************************************************** 
    maybeWrite
   ********************************************************************** */

ipReturn tcpSend(int,NetMsgBuffer *,Bool);

ipReturn maybeWrite(int fd,WriteConnection *r){
  int ret;
  RemoteSite* site = r->remoteSite;
  
  PD((TCP,"maybeWrite r%x",r));
  if(r->isWantsToAckClose()){
    ret=write_ack_close(fd,r);
    if(ret==IP_BLOCK){
      PD((WEIRD,"maybeWrite blocked 1"));
      return IP_OK;}
    OZ_unregisterWrite(fd);
    PD((OS,"unregister WRITE fd:%d",fd));
    close_write_connection(fd,r);
    return IP_OK;}
  if(r->isWantsToClose()){
    ret=tcpCloseWriter(r);
    if(ret==IP_BLOCK){
      PD((WEIRD,"maybeWrite blocked 2"));
      return IP_OK;}
    return IP_OK;}
  Message *m;
  while(r->isInWriteQueue()){
    PD((TCP,"taking from write queue %x",r));
    m=r->getWriteQueue();
    Assert(m!=NULL);
    site->deQueueMessage(m->getMsgBuffer()->getTotLen());
    ret=tcpSend(r->getFD(),m->getMsgBuffer(),FALSE);
    if(ret<0){goto error_block;}
    messageManager->freeMessage(m);}
  
  tcpCache->touchWrite(r); /* QUES */
  return IP_OK;


error_block:
  switch(ret){
  case IP_BLOCK:{
    PD((WEIRD,"incomplete write %x",r));
    site->queueMessage(m->getMsgBuffer()->getTotLen());
    r->addCurQueue(m);
    return IP_BLOCK;}
  case IP_PERM_BLOCK:{
    close_crashed_connection(r->getFD(),r);
    messageManager->freeMessageAndMsgBuffer(m);
    return IP_PERM_BLOCK;}
  default:
    //EK
    // Not implemented yetgg
    OZ_warning("Tmp block during send, not handled...");
    return IP_BLOCK;
    
  }
}

/*  *****************************************************************
          tcpWriteHandler
   ***************************************************************** */

int tcpWriteHandler(int fd,void *r0){
  WriteConnection *r=(WriteConnection *)r0;
  Message *m;
  ipReturn ret;

  PD((TCP,"tcpWriteHandler invoked r:%x",r));
  if(!r->isIncomplete()){
    if(r->isWantsToAckClose()){
      ret=write_ack_close(fd,r);
      if(ret==IP_BLOCK){return IP_OK;}
      OZ_unregisterWrite(fd);
      PD((OS,"unregister WRITE fd:%d",fd));
      close_write_connection(fd,r);
      return 0;}
    if(r->isWantsToClose()){
      if(tcpCloseWriter(r)==IP_BLOCK)
	{NETWORK_ERROR(("tcpWriteHandler"));}
      return 0;}
    NETWORK_ERROR(("unregisterWrite does not work"));}
  m=r->getCurQueue();
  Assert(m!=NULL);
  ret=tcpSend(r->getFD(),m->getMsgBuffer(),TRUE);
  if(ret<0){
    Assert(ret==IP_BLOCK);
    PD((WEIRD,"tcpWriteHandler blocked r:%x",r));  
    r->addCurQueue(m);
    return 0;}

  PD((TCP,"tcpWriteHandler finished r:%x",r));  
  messageManager->freeMessage(m);
  ret=maybeWrite(fd,r);
  if(ret==IP_OK) {
    OZ_unregisterWrite(fd);
    PD((OS,"unregister WRITE fd:%d",fd));}
  return 0;
}

/*  *****************************************************************
          tcpSend
   ***************************************************************** */

ipReturn tcpSend(int fd,NetMsgBuffer *bs, Bool flag) 
{
  int total,len,ret;

  if(flag){
    total=bs->calcTotLen();
    PD((TCP,"tcpSend invoked rem:%d",total));}
  else{
    total=bs->getTotLen();
    PD((CONTENTS,"tot:%d",total));
    PD((TCP,"tcpSend invoked tot:%d",total));
    bs->PiggyBack();
  }
  
  while(total){
    Assert(total>0);
    len=bs->getWriteLen();
    BYTE* pos=bs->getWritePos();
    while(TRUE){
      ret=oswrite(fd,pos,len);
      if(ret>0) break;

      // EK check ret. The Assert will disapear in
      // non-debug code.
      Assert(ret<0);
      if(errno==EINTR) continue;
      if(!((errno==EWOULDBLOCK) || (errno==EAGAIN)))
	return tcpWriteError();
      break;}
    PD((WRITE,"wr:%d try:%d",ret,len));
    if(ret<len){
      if(ret>0){
	bs->incPosAfterWrite(ret);
	PD((TCP,"tcpSend blocked wrote %d of %d",ret,len));}
      return IP_BLOCK;}
    // EK
    // Redo:
    // The bytestream should not release its Bytebuffers until
    // a specific command is signaled. Then should the whole 
    // bytestream be released.
    bs->sentFirst();
    total -=len;}
  bs->writeCheck();
  PD((TCP,"tcpSend fin"));
  return IP_OK;
}


/* ********************************************************************** 
         tcpConnectionHandler
   ********************************************************************** */

int smallMustRead(int fd,BYTE *pos,int todo,int tries){
  PD((TCP,"smallMustRead invoked"));  
  while(TRUE){
    int ret=osread(fd,pos,todo);
    if(ret==todo) return 0;
    if(ret<0){
      // EK
      // Handle these errors. They can occur.
      // Inform Connection and Site.
      if(errno!=EINTR) return errno;
      tries--;
      if(tries<0) {return IP_NO_MORE_TRIES;}
    pos += ret;
    todo -= ret;}}}

#define CONNECTION_HANDLER_TRIES 50

int tcpConnectionHandler(int fd,void *r0){
  WriteConnection *r=(WriteConnection *)r0;
  int ret;
  int  bufSize = 9;

  BYTE *buf = new BYTE[bufSize];
  BYTE *pos=buf;
  int todo=bufSize;

  PD((TCP,"tcpConnectionHandler invoked r:%x",r));  
  ret=smallMustRead(fd,pos,bufSize,CONNECTION_HANDLER_TRIES);
  if(ret!=0){
    if(ret>0){
      // EK
      // Handle these errors. They can occur.
      // Inform Connection and Site.
     IMPLEMENT(("tcpConnectionHandler 2 %d\n",ret));
     delete buf;
     return 0;}
    NETWORK_ERROR(("tcpConnectionHandler 1 %d\n",ret));}

  pos = buf;
  if(*pos!=TCP_CONNECTION){
    NETWORK_ERROR(("connectionHandler received %c",buf[0]));}
  pos++;
  time_t timestamp=net2int(pos);
  int tcheck=r->remoteSite->site->checkTimeStamp(timestamp);
    if(tcheck!=0){
    if(tcheck<0) {
      NETWORK_ERROR(("impossible timestamp"));}
    delete buf;
    r->remoteSite->site->sitePermProblem();
    IMPLEMENT(("tcpConnectionHandler 2 %d\n",ret));}
    
  pos += netIntSize;
  unsigned int strngLen = net2int(pos);

  delete buf;
  buf = new BYTE[strngLen + 1];
  pos = buf;

  // EK 
  // Reads the fd for the string
  //
  
  ret=smallMustRead(fd,pos,strngLen,CONNECTION_HANDLER_TRIES);
  if(ret!=0){
    if(ret>0){
      // EK
      // Handle these errors. They can occur.
      // Inform Connection and Site.
     IMPLEMENT(("tcpConnectionHandler 4 %d\n",ret));
     delete buf;
     return 0;}
    NETWORK_ERROR(("tcpConnectionHandler 3 %d\n",ret));}

  if (strlen(PERDIOVERSION)!=strngLen ||
      strncmp(PERDIOVERSION,(char*)pos,strngLen)!=0) {
    buf[bufSize-1] = 0;
    OZ_warning("Perioversion conflict with site");
    r->remoteSite->sitePrmDwn();
    delete buf;
    return 0;
  }
  delete buf;
  r->opened();
  OZ_unregisterRead(fd);
  PD((OS,"unregister READ fd:%d",fd));
  // fcntl(fd,F_SETFL,O_NONBLOCK);
  fcntl(fd,F_SETFL,O_NDELAY);
  OZ_registerReadHandler(fd,tcpCloseHandler,(void *)r);
  PD((OS,"register READ - close fd:%d",fd));
  if(r->isWritePending()){
    int ret = maybeWrite(fd,r);
    if(ret==IP_BLOCK) {
      PD((WEIRD,"tcpConnectionHandler pending write blocked"));  
      PD((OS,"register WRITE - tcpWriteHandler fd:%d",fd));
      OZ_registerWriteHandler(fd,tcpWriteHandler,(void *)r);}}
  PD((TCP,"tcpConnectionHandler ord fin"));    
  return 0;
}

inline ipReturn writeI(int fd,BYTE *buf)
{
  int ret;
  while(TRUE){
    ret = oswrite(fd,buf,1);
    if(ret>0){
      Assert(ret==1);
      return IP_OK;}
    if (errno == EINTR){
      PD((WEIRD,"write interrupted"));}
    else {
      if((errno==EWOULDBLOCK) || (errno==EAGAIN)) {
	return IP_BLOCK;}
      NETWORK_ERROR(("writeI %d\n",errno));}}}

inline ipReturn readI(int fd,BYTE *buf)
{
  int ret;
  while(TRUE){
    ret = osread(fd,buf,1);
    if(ret>0){
      Assert(ret==1);
      return IP_OK;
    }
    if (errno == EINTR){
      PD((WEIRD,"readI interrupted"));
    } else {
      if((errno==EWOULDBLOCK) || (errno==EAGAIN)) {
	return IP_BLOCK;
      }
      NETWORK_ERROR(("readI %s\n",OZ_unixError(ossockerrno())));
      return IP_NET_CRASH;
    }
  }
}

/* *****************************************************************
   *****************************************************************
          READER
   *****************************************************************
   *************************************************************** */

#define EAGAIN_TRIES 100000

// EK
// This one seems to be litle dangerous
// If the send fails is nothing done....
void tcpCloseReader(ReadConnection *r){
  BYTE msg;
  int fd=r->getFD();
  Assert(fd!=LOST);
  ipReturn ret;
  PD((TCP,"tcpCloserReader r:%x",r));    
  msg=TCP_CLOSE_REQUEST_FROM_READER;
  ret=writeI(fd,&msg);
  if(ret==IP_BLOCK){
    NETWORK_ERROR(("tcpCloseReader:write %d\n",errno));}
  Assert(ret==IP_OK);
  r->setClosing();
  return;}



// EK
// WARNING
// The buffer must be of the right size. No check
// is done for overflow

void uniquefyInt(BYTE *buf, int Num)
{
  int ctr = 0;
  BYTE b;
  while(ctr < 33){
    b = Num & 255;
    b <<=  TCP_MSG_SIGN_BITS;
    *buf++ = b;
    Num >>= (8 - TCP_MSG_SIGN_BITS);
    ctr += 8 - TCP_MSG_SIGN_BITS;
  }
}

int intifyUnique(BYTE *buf){
  int ctr = 0;
  unsigned int i;
  int ans = 0;
  while(ctr < 33){
    i = *buf++;
    i >>= TCP_MSG_SIGN_BITS;
    ans += i << ctr;
    ctr += (8 - TCP_MSG_SIGN_BITS);}
  return ans;}



Bool tcpAckReader(ReadConnection *r, int ack){
  BYTE buffer[INT_IN_BYTES_LEN + 1];
  BYTE *buf = buffer;
  int fd=r->getFD();
  Assert(fd!=LOST);
  ipReturn ret;
  PD((TCP,"tcpAckReader r:%x a:%d",r, ack));    
  *buf=(BYTE) TCP_MSG_ACK_FROM_READER;
  uniquefyInt(buf + 1, ack);
  while((buf - buffer) < INT_IN_BYTES_LEN + 1){
    PD((TCP,"Writing byte: %d",*buf));
    ret=writeI(fd,buf++);
    if(ret!=IP_OK){
      //EK
      //
      // It is possible to recognize a failed site here.
      // the Connection could be invoked to handle this. 
      PD((TCP,"Write fail in Acking"));
      return false;}}
  return true;
}

inline
int tcpRead(int fd,BYTE *buf,int size,Bool &readAll)
{
  int ret;
  int no_tries=EAGAIN_TRIES;
  while(no_tries){
    ret = osread(fd,buf,size);
    if (ret > 0) {
      if(ret==size) {readAll=TRUE;}
      else {readAll=FALSE;}
      return ret;}
    if (ret==0) return -1;
    if (errno == EINTR){ 
      PD((WEIRD,"read interrupted"));
      no_tries--;
      continue;}
    if(errno==EWOULDBLOCK){
      Assert(0);
      return 0;}
    if(errno==EAGAIN){
      PD((WEIRD,"read EAGAIN"));
      no_tries--;
      continue;}
    break;}
  return ret;
}


inline ipReturn interpret(MsgBuffer *bs,tcpMessageType type){
  if(type==TCP_PACKET){
    PD((TCP,"interpret-packet"));      
    PD((TCP,"received TCP_PACKET"));
    bs->getSite()->msgReceived(bs);
    return IP_OK;}
  else{
    PD((TCP,"interpret - close"));      
    Assert(type==TCP_CLOSE_REQUEST_FROM_WRITER);
    PD((TCP,"received TCP_CLOSE_REQUEST_FROM_WRITER"));
    return IP_CLOSE;}
}


inline tcpMessageType getHeader(NetMsgBuffer *bs,int &len, int &msg, int &ans){
  BYTE buf[4];
  tcpMessageType type;
  BYTE b=bs->getInRead();
  GenCast(b,BYTE,type,tcpMessageType);
  buf[0]=bs->getInRead();
  buf[1]=bs->getInRead();
  buf[2]=bs->getInRead();
  buf[3]=bs->getInRead();
  len=net2int(buf);
  buf[0]=bs->getInRead();
  buf[1]=bs->getInRead();
  buf[2]=bs->getInRead();
  buf[3]=bs->getInRead();
  msg=net2int(buf);
  buf[0]=bs->getInRead();
  buf[1]=bs->getInRead();
  buf[2]=bs->getInRead();
  buf[3]=bs->getInRead();
  ans=net2int(buf);
  return type;}

// EKC
// Check this one out
// Uses the 5 arg allocMessage....

inline Message* newReadCur(Message *m,NetMsgBuffer *bs,
			   ReadConnection *r,
			   tcpMessageType type,int rem){
  if(m==NULL){
    PD((WEIRD,"newReadCur r:%x",r));
    Message *m=messageManager->allocMessage(bs,type,rem);
    r->addCurQueue(m);
    return m;}
  else{
    PD((WEIRD,"oldReadCur r:%x",r));
    m->setType(type);
    m->setRemainder(rem);
    return m;}
}
 
/*   ***************************************************************** */

static int tcpReadHandler(int fd,void *r0)
{  
  ReadConnection *r = (ReadConnection*) r0;
  tcpCache->touchRead(r);

  int ret,rem,len,msgNr,ansNr;
  int totLen;
  ipReturn ip;
  tcpMessageType type = TCP_NONE;
  Message *m;
  NetMsgBuffer *bs;
  BYTE *pos;
  Assert(fd==r->getFD());
  Bool readAll;
  Bool length_knownnown;
  
  if(r->isIncomplete()){
    m=r->getCurQueue();
    Assert(m!=NULL);
    bs=m->getMsgBuffer();
    rem=m->getRemainder();
    type=m->getType();
    PD((TCP,"readHandler incomplete r:%x rem:%d",r,rem));
    pos=bs->beginRead(len);}
  else{
    m=NULL;
    //EK
    rem=0-tcpHeaderSize;
    
    // ATTENTION 
    // Must insert the correct Site later....

    bs=netMsgBufferManager->getNetMsgBuffer(NULL);
    PD((TCP,"readHandler from scratch r:%x",r));
    bs->getSingle();
    pos=bs->initForRead(len);}

start:

  Assert(osTestSelect(fd,SEL_READ));
  ret=tcpRead(fd,pos,len,readAll);
  if (ret<0) {
    //EK
    // This one is noy handled right now....
    warning("Connection Site Has Crashed\n");
      PD((WEIRD,"readHandler sees crashed Connection site %s",r->remoteSite->site->stringrep()));
      if(m!=NULL){
	fprintf(stderr,"dumping incomplete read\n");
	Assert(r->isIncomplete());
	m = r->getCurQueue();
	messageManager->freeMessage(m);}
      
      // EK
      // check if this is right....

      netMsgBufferManager->freeNetMsgBuffer(bs);
      close_crashed_connection(fd,r);
      return 0;
  }

  PD((READ,"no:%d av:%d rem:%d",ret,len,rem));
  bs->afterRead(ret);
  
  while(TRUE){
    PD((READ,"WHILE no:%d av:%d rem:%d",ret,len,rem));
    if(rem<0){
      if(rem+ret>=0){
	type=getHeader(bs,len,msgNr,ansNr);
	totLen = len;
	PD((READ,"Header done no:%d av:%d rem:%d tcp:%d",ret,len,rem,tcpHeaderSize));
	rem=len-ret-tcpHeaderSize-rem;}
      else{
	rem +=ret;
	goto maybe_redo;}}
    else{
      rem -=ret;}
       
    if(rem>0){goto maybe_redo;}
    
    // EK 
    // Does Not check if messages is resending...
    r->informSiteAck(msgNr,ansNr,totLen);

    bs->beforeInterpret(rem);
    PD((CONTENTS,"interpret rem:%d len:%d",
		 rem,bs->interLen()));
    // EK this might be done in a nicer way...
    ip=interpret(bs,type);
    if(ip==IP_CLOSE){
      Assert(rem==0);
      if(m!=NULL){
	Assert(r->isIncomplete());
	messageManager->freeMessage(m);}
      // EK check this out...
      else{netMsgBufferManager->freeNetMsgBuffer(bs);}
      goto close;}
    bs->afterInterpret();    
    if(rem==0){goto fin;}
    ret=0-rem;
    rem=0-tcpHeaderSize;}

maybe_redo:  

  if(readAll && osTestSelect(fd,SEL_READ)){
    if(rem==0) {pos=bs->initForRead(len);}
    else {pos=bs->beginRead(len);}
    goto start;}

fin:
  if(rem==0){
    if(m==NULL){
      netMsgBufferManager->freeNetMsgBuffer(bs);
      return 0;}
    Assert(r->isIncomplete());
    messageManager->freeMessage(m);}
  newReadCur(m,bs,r,type,rem);
  return 0;

close:

  PD((TCP,"readHandler received close"));
  if(rem!=0) {NETWORK_ERROR(("readHandler gets bytes after Close"));}
  if(r->isClosing()){ /* writer has precedence */
    r->clearClosing();}
  BYTE msg=TCP_CLOSE_ACK_FROM_READER;
  while(TRUE){
    int ret=oswrite(fd,&msg,1);
    if(ret==1) break;
    if(ret<0){
      if((ret!=EINTR) && (ret!=EWOULDBLOCK) && (ret!=EAGAIN)){
	NETWORK_ERROR(("readHandler: write %d\n",errno));}}}
  close_read_connection(fd,r);
  return 0;
}


  
/* ********************************************************************** 
    tcpPreReadHandler
********************************************************************** */

#define PREREAD_HANDLER_TRIES 50
#define PREREAD_NO_BYTES 5

int tcpPreReadHandler(int fd,void *r0){
  ReadConnection *r=(ReadConnection *)r0;
  tcpOpenMsgBuffer->unmarshalBegin();
  BYTE *pos = tcpOpenMsgBuffer->getBuf();

  PD((TCP,"tcpPreReadHandler invoked r:%x",r));  
  int ret=smallMustRead(fd,pos,PREREAD_NO_BYTES,PREREAD_HANDLER_TRIES);
  if(ret!=0){
    if(ret<0)
      NETWORK_ERROR(("tcpPreReadHandler 1 %d\n",ret));  
    pos += ret;}
  pos +=5;
  int todo;
  BYTE header =  tcpOpenMsgBuffer->readyForRead(todo);
  if(header!=TCP_MYSITE){
    // EK
    // What to do here?
    // Garbage in the Accept handler pipe!!
    // flush it or?
    IMPLEMENT(("tcpPreReadHandler 5\n"));    
    return 0;}
  ret=smallMustRead(fd,pos,todo,PREREAD_HANDLER_TRIES);
  if(ret!=0){
    if(ret>0){
      // EK
      // Handle these errors. They can occur.
      // Inform Connection and Site.
     IMPLEMENT(("tcpPreReadHandler 4 %d\n",ret));
     return 0;}
    NETWORK_ERROR(("tcpPreReadHandler 3 %d\n",ret));}
  
  int maxNrAck = unmarshalNumber(tcpOpenMsgBuffer);
  int maxNrSize= unmarshalNumber(tcpOpenMsgBuffer);
  Site *site = unmarshalSite(tcpOpenMsgBuffer);
  tcpOpenMsgBuffer->unmarshalEnd();

  RemoteSite *remotesite = site->getRemoteSite();
  
  remotesite -> setMaxNrSizeAck(maxNrAck, maxNrSize);
  remotesite->setReadConnection(r);
  OZ_unregisterRead(fd);
  PD((OS,"unregister READ fd:%d",fd));
  PD((OS,"register READ - close fd:%d",fd));
  OZ_registerReadHandler(fd,tcpReadHandler,(void *)r);  
  return 0;
}

/* ********************************************************************** 
    acceptHandler
********************************************************************** */

static int acceptHandler(int fd,void *unused)
{
  PD((TCP,"acceptHandler: %d",fd));
  struct sockaddr_in from;
  int fromlen = sizeof(from);
  int newFD=osaccept(fd,(struct sockaddr *) &from, &fromlen);
  if (newFD < 0) {NETWORK_ERROR(("acceptHandler:accept %d\n",errno));}
  
  ip_address ip=from.sin_addr.s_addr;
  port_t port=from.sin_port;

  ReadConnection *r=readConnectionManager->
    allocConnection(NULL,newFD);

  int  bufSize = 9 + strlen(PERDIOVERSION) + 1;
  BYTE *buf = new BYTE[bufSize];
  BYTE *auxbuf = buf;

  *auxbuf = TCP_CONNECTION;
  auxbuf++;
  int2net(auxbuf,mySite->getTimeStamp());
  auxbuf += netIntSize;
  int2net(auxbuf, bufSize - 10);
  auxbuf += netIntSize;
  for (char *pv = PERDIOVERSION; *pv; pv++, auxbuf++) {
    *auxbuf = *pv;
  }
  *auxbuf++ = 0;
  Assert(auxbuf-buf == bufSize);

  // fcntl(newFD,F_SETFL,O_NONBLOCK);
  fcntl(newFD,F_SETFL,O_NDELAY);
  int written = 0;
  while(TRUE){
    int ret=oswrite(newFD,buf+written,bufSize-written);
    written += ret;
    if(written==bufSize) break;
    if(ret<=0 && ret!=EINTR) { 
      delete buf;
      error("ip + should not happen 2");
      NETWORK_ERROR(("acceptHandler:write %d\n",errno));
    }
  }

  delete buf;
  tcpCache->addRead(r);
  PD((TCP,"acceptHandler success r:%x",r)); 
  // EK should be optimized 
  // Check for MYSITE info in fd
  OZ_registerReadHandler(newFD,tcpPreReadHandler,(void *)r);
  PD((OS,"register READ- tcpPreReadHandler fd:%d",fd));
  return 0;
}


/*********************************************************

  METHODS

*******************************************************/



/**********************************
 *  class Site
 **********************************/

void RemoteSite::sitePrmDwn(){
  status = SITE_PERM;
  if(writeConnection!=NULL){
    writeConnection->prmDwn();
    writeConnectionManager->freeConnection(writeConnection);}
  if(readConnection!=NULL){
    readConnection->prmDwn();
    readConnectionManager->freeConnection(readConnection);}
  Message *m = sentMsg;
  sentMsg = NULL;
  while(m != NULL){
    site->communicationProblem(m->msgType, m->site, m->storeIndx,
			 COMM_FAULT_PERM_MAYBE_SENT,(FaultInfo) m->bs);
    Message *tmp = m;
    messageManager->freeMessageAndMsgBuffer(m);
    m = m->next;}
  //site->lostRemoteSite();
  remoteSiteManager->freeRemoteSite(this);
  
}
  void RemoteSite::siteTmpDwn(){
    status = SITE_TEMP; 
    writeConnection->tmpDwn();
    //Ek
    //release Connection
    Message *m = sentMsg;
    while(m != NULL){
      m->msgNum = getTmpMsgNum();
      site->communicationProblem(m->msgType, m->site, m->storeIndx,
			   COMM_FAULT_TEMP_MAYBE_SENT,(FaultInfo)m->msgNum);
      m = m->next;
    }
    
  }

void RemoteSite::init(Site* s, int msgCtr){
    writeConnection=NULL;
    readConnection=NULL;
    sentMsgCtr = 0;
    recMsgCtr = msgCtr;
    sentMsg = NULL;
    tmpMsgNum = 1;
    totalNrMsg = 0;
    totalMsgSize = 0;
    site = s;
}

void RemoteSite::setMaxNrSizeAck(int mA, int mS){
  PD((SITE,"Max acks: %d size: %d",mA, mS));
  maxNrAck = mA;
  maxSizeAck = mS;
  recNrAck = 0;
  recSizeAck = 0;
}

void RemoteSite::setWriteConnection(WriteConnection *r){
  Assert(writeConnection==NULL);
  writeConnection=r; 
  r->setSite(this);}

void RemoteSite::receivedNewMsg(int Nr){
  if (Nr > recMsgCtr || (Nr == 1 && recMsgCtr > Nr)){
    recMsgCtr = Nr;
    recNrAck ++;
    PD((SITE,"SiteObject msgrec: %d nr: %d lim: %d",
	recMsgCtr,recNrAck, maxNrAck));
    if (recNrAck >= maxNrAck && 
	tcpAckReader(getReadConnection(),Nr)){
      PD((SITE,"Ack, limit reached...."));
      recSizeAck = 0;
      recNrAck = 0;
    }}}

  
void RemoteSite::receivedNewSize(int size){
  recSizeAck +=  size;
  PD((SITE,"SizeReceived s: %d l: %d",recSizeAck
      ,maxSizeAck));
  if(recSizeAck >= maxSizeAck &&
     tcpAckReader(getReadConnection(),recMsgCtr)){
    PD((SITE,"Ack, limit reached...."));
    recSizeAck = 0;
    recNrAck = 0;
  }}

int RemoteSite::getRecMsgCtr(){ recNrAck = 0; 
recSizeAck = 0;
return recMsgCtr;}


void RemoteSite::setReadConnection(ReadConnection *r)
{
  Assert(readConnection==NULL);
  readConnection=r; 
  r->setSite(this); 
}
ReadConnection* RemoteSite::getReadConnection(){
  return readConnection;}

time_t RemoteSite::getTimeStamp(){return site->timestamp;} 

port_t RemoteSite::getPort(){return site->port;} 

ip_address RemoteSite::getAddress(){return site->address;} 

WriteConnection *RemoteSite::getWriteConnection(){
  return writeConnection;} 

/**********************************
 *  class IOQueue
 **********************************/


void IOQueue::checkQueue(){
  if(first==NULL) {Assert(last==NULL);return;}
  if(first==last) {Assert(first->next==NULL);return;}
  Message *b=first;
  while(b!=last){Assert(b!=NULL);b=b->next;}
  Assert(last->next==NULL);
  return;}

Message* IOQueue::getFirst(){return first;}
Message* IOQueue::removeFirst(){
  if(first == NULL){
    Assert(last == NULL);
    return NULL;}
  Message *t=first;
  first = first -> next;
  if(first == NULL)
    last = NULL;
  return t;
}
    
void IOQueue::enqueue(Message *m){ 
  // EKT
  // Expensive, shouyld be an Assertion
  checkQueue();
  Assert(((first==NULL) && (last==NULL))|| 
	 ((first!=NULL) && (last!=NULL)));
  if(last!=NULL){last->next=m;Assert(m->next==NULL);}
  else {Assert(first==NULL);first=m;}
    last=m;
    return;}



void IOQueue::dequeue(Message *m){
  Assert(((first==NULL) && (last==NULL))|| 
	 ((first!=NULL) && (last!=NULL)));
  if(first==m){
    first=m->next; 
    if(last==m) last=NULL;
    m->next=NULL;
    return;}
  Message *b=first;
  while(b->next!=m) {Assert(b!=NULL);b=b->next;}
  b->next=m->next;
  if(b->next==NULL) {Assert(m->next==NULL);last=b;}
  m->next=NULL;
    return;}

/**********************************
 *  class Connection
 **********************************/
/*

  void Connection::init(RemoteSite *s, Bool isR){
    next = prev = NULL;
    flags=0;
    writeCtr=0;
    remoteSite=s;
    isRdr = isR;
    bytePtr = intBuffer + INT_IN_BYTES_LEN;

  }
  */




  


  /* closing */

  void WriteConnection::setWantsToClose(){
    PD((CONNECTION,"setWantsToClose site:%s",remoteSite->site->stringrep()));
    Assert(!testFlag(WANTS_TO_CLOSE));
    Assert(!testFlag(CLOSING));
    setFlag(WANTS_TO_CLOSE);}
  void WriteConnection::nowClosing(){
    PD((CONNECTION,"nowClosing site:%s",remoteSite->site->stringrep()));
    clearFlag(WANTS_TO_CLOSE);
    Assert(!testFlag(CLOSING));
    setFlag(CLOSING);}

  void WriteConnection::setWantsToOpen(){
    PD((CONNECTION,"setWantsToOpen site:%s",remoteSite->site->stringrep()));
    Assert(!testFlag(WANTS_TO_OPEN));
    Assert(!testFlag(OPENING));
    setFlag(WANTS_TO_OPEN);}
  void WriteConnection::nowOpening(){
    Assert(testFlag(WANTS_TO_OPEN));
    PD((CONNECTION,"nowOpening site:%s",remoteSite->site->stringrep()));
    clearFlag(WANTS_TO_OPEN);
    setFlag(OPENING);}

/*  void WriteConnection::setOpening(){
    Assert(!testFlag(OPENING));
    PD((CONNECTION,"setOpening site:%x",site));
    setFlag(OPENING);}*/


  /* pending messages stuff */

/*
void Connection::setInFastQueue(){
    writeCtr++;
    Assert(!isReader());
    PD((CONNECTION,"setInFastQueue s:%x ct:%d",site,writeCtr));
    setFlag(WRITE_FAST);}
  
  Bool Connection::isInSlowQueue(){
    Assert(!testFlag(WRITE_CURRENT));
    Assert(!testFlag(WRITE_FAST));
    return testFlag(WRITE_SLOW);}
  void Connection::setInSlowQueue(){
    Assert(!isReader());
    writeCtr++;
    PD((CONNECTION,"setInSlowQueue s:%x ct:%d",site,writeCtr));
    setFlag(WRITE_SLOW);}
  
  Bool Connection::isWritePending(){
    if(testFlag(WRITE_SLOW|WRITE_FAST|WRITE_CURRENT)) return TRUE;
    return FALSE;}
    
void Connection::addOpenQueue(Message *m){
    PD((TCPQUEUE,"add open m:%x r:%x",m,this));
    setWantsToOpen();
    openQueue.enqueue(m);}

  void Connection::subOpenQueue(Message *m){
    PD((TCPQUEUE,"sub Open m:%x r:%x",m,this));
    nowOpening();
    openQueue.dequeue(m);}

*/


  
/* ********************************************************************** 
    ipInit               INVALID_VIRTUAL_PORT    NET_OK
                         NET_RAN_OUT_OF_TRIES
   ********************************************************************** */
  

InterfaceCode ipInit(){
  ip_address ip;
  port_t p;
  int tcpFD;
  

  PD((TCP_INTERFACE,"ipInit invoked"));
  ipReturn ret=createTcpPort(OZReadPortNumber,ip,p,tcpFD);
  if (ret<0){
    PD((WEIRD,"timer"));
    Assert(ret==IP_TIMER);
    return NET_RAN_OUT_OF_TRIES;}
  time_t timestamp=time(0);
  mySiteInfo.tcpFD=tcpFD;
  mySiteInfo.maxNrAck = 100;
  mySiteInfo.maxSizeAck = 40000;
  Assert(mySite==NULL);
  mySite=initMySite(ip,p,timestamp);
  Assert(mySite!=NULL);  
  OZ_registerAcceptHandler(tcpFD,acceptHandler,NULL);
  PD((OS,"register ACCEPT- acceptHandler fd:%d",tcpFD));
  tcpCache->nowAccept();
  return NET_OK;
}

/* ********************************************************************** 
    connectSite    (SITE_NAME_UNKNOWN      INVALID_VIRTUAL_PORT,
                    NET_RAN_OUT_OF_TRIES   NET_OK
   ********************************************************************** */

int RemoteSite::
sendTo(NetMsgBuffer *bs, MessageType msg,
       Site *storeS, int storeInd)
{
  int msgNum,ret;
  Message *m=messageManager->allocMessage(bs,NO_MSG_NUM,storeS,msg,storeInd);
  switch (siteStatus()){
  case SITE_TEMP:{
      goto tmpdwnsend2;}
  case SITE_PERM:{
      	return SITE_PERM;}
  case SITE_OK:{
    PD((SITE,"OK"));}}  
  bs->beginWrite(this);
  int fd;
  if(writeConnection==NULL){
    PD((TCP_INTERFACE,"try open %s",site->stringrep()));
    writeConnection=writeConnectionManager->allocConnection(this,LOST);
    fd=tcpOpen(this,writeConnection);
    if(fd==IP_TIMER ||fd==IP_NO_MORE_TRIES){
      goto tmpdwnsend;}
    if(fd==IP_NET_CRASH){ return SITE_PERM;}
    PD((TCP,"is reopened %s",site->stringrep()));
    tcpCache->addWrite(writeConnection);}
  else{
    if(!writeConnection->shouldSendFromUser())
      goto sendblock;
    tcpCache->touchWrite(writeConnection);}  
  
  fd=writeConnection->getFD();
  Assert(fd>0);
  switch(tcpSend(fd,bs,FALSE)){
	case IP_OK:{
	  PD((TCP_INTERFACE,"reliableSend- all sent"));
	  return ACCEPTED;}
	case IP_BLOCK:{
	  PD((TCP_INTERFACE,"reliableSend- part sent"));
	  OZ_registerWriteHandler(fd,tcpWriteHandler,(void *)this); 
	  writeConnection->addCurQueue(m);
	  return ACCEPTED;}
	case IP_TEMP_BLOCK:{
	    PD((TCP_INTERFACE,"reliableSend- tmp failed"));
	    siteTmpDwn();
	    msgNum = getTmpMsgNum();
	    // EK
	    // fixa en addCurQueue/2
	    writeConnection->addCurQueue(m);
	    return msgNum;}
	case IP_PERM_BLOCK:{
	    PD((TCP_INTERFACE,"reliableSend- perm failed"));
	    sitePrmDwn();
	    return PERM_NOT_SENT;}
	default:{
	  Assert(0); 
	  return PERM_NOT_SENT;}}
tmpdwnsend:
	siteTmpDwn();
	tcpWantsToOpen(writeConnection);
tmpdwnsend2:
	msgNum = getTmpMsgNum();
	writeConnection->addWriteQueue(m,msgNum);
	return msgNum;
sendblock:
  	Assert(0);
	return 0;
}

/**********************************************************************/
/*   SECTION :: exported to the protocol layer              */
/**********************************************************************/
RemoteSite* createRemoteSite(Site* site, int readCtr){
  return remoteSiteManager->allocRemoteSite(site, readCtr);}
void zeroRefsToRemote(RemoteSite *s){
  s->zeroReferences();}
void nonZeroRefsToRemote(RemoteSite *s){
  s->zeroReferences();} 
int sendTo_RemoteSite(RemoteSite* rs,MsgBuffer* bs,MessageType m,Site* s, int i){
  return rs->sendTo((NetMsgBuffer*)bs,m,s,i);}
int discardUnsentMessage_RemoteSite(RemoteSite* s,int msg){
  return s->discardUnsentMessage(msg);}
int getQueueStatus_RemoteSite(RemoteSite* s,int &noMsgs){
  return s->getQueueSize(noMsgs);}
SiteStatus siteStatus_RemoteSite(RemoteSite* s){
  return s->siteStatus();}
MonitorReturn monitorQueue_RemoteSite(RemoteSite* site,int size,int no_msgs,void* storePtr){
  return site->monitorQueueMsgs(no_msgs, size, storePtr);}
MonitorReturn demonitorQueue_RemoteSite(RemoteSite* site){
  return site->deMonitorQueue();}
ProbeReturn installProbe_RemoteSite(RemoteSite* site,ProbeType type,int frequency,void* storePtr){
  Assert(0);return PROBE_PERM;}
ProbeReturn deinstallProbe_RemoteSite(RemoteSite* site,ProbeType type){
  Assert(0);return PROBE_PERM;}
ProbeReturn probeStatus_RemoteSite(RemoteSite* site,ProbeType &pt,int &frequncey,void* &storePtr){
  Assert(0);return PROBE_PERM;}
GiveUpReturn giveUp_RemoteSite(RemoteSite* site){
  Assert(0);return GIVES_UP;}
void discoveryPerm_RemoteSite(RemoteSite* site){
  Assert(0);} 

void initNetwork(){
  writeConnectionManager = new WriteConnectionManager();
  readConnectionManager = new ReadConnectionManager();
  netMsgBufferManager = new NetMsgBufferManager();
  remoteSiteManager = new RemoteSiteManager();
  byteBufferManager = new ByteBufferManager();
  messageManager = new MessageManager();
  tcpCache = new TcpCache();
  tcpOpenMsgBuffer= new TcpOpenMsgBuffer();
(void)ipInit();}
  
MsgBuffer* getRemoteMsgBuffer(Site* s){ 
  return netMsgBufferManager->getNetMsgBuffer(s);}

  



