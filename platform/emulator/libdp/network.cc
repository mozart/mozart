/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
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

// abstract interface to the ip network

/* ***************************************************************************
***************************************************************************
                       ORGANIZATION

          1  Enums & Defs
          2  Forward declaration
          3  Global Variables
          4  Network MsgBuffer and friends
          5  RemoteSite
          6  Read and Write - Connection
          7  MySiteInfo
          8  Managers
          9  Small routines for RemoteSite
          10 TcpError
          11 TcpCache
          12 Exported to Perdio
          13 ????
          14 MessageManagers
          15 Methods
          16 Message storing
          17 Small Routines
          18 ReadStuff
          19 HandShaking Reader
          20 HandShaking Writer
          21 WriteHandler
          22 Ack-stuff
          23 Prm/Tmp Routines
          24 Closing
          25 RemoteSite Init
          26 IOQueue meths
          27 Probes
          28 Exports to Perdio
          29 Exported for debugging
          30 Transfer speed control

   **************************************************************************
   **************************************************************************/

#include "base.hh"
#include "dpBase.hh"

#include "wsock.hh"
#ifndef WINDOWS
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netdb.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "os.hh"
#include "pickle.hh"
#include "dpMarshaler.hh"
#include "msgbuffer.hh"
#include "dpDebug.hh"
#include "builtins.hh"
#include "genhashtbl.hh"
#include "comm.hh"
#include "dsite.hh"
#include "network.hh"
/* ************************************************************************ */
/*  SECTION 1:  Enums & Defines                                            */
/* ************************************************************************ */

#define NETWORK_ERROR(Args) {OZ_error Args;}
#define IMPLEMENT(Args) {OZ_error Args;}


/* free list sizes */

#define SITE_CUTOFF    100
#define MESSAGE_CUTOFF 1000
#define READ_CONNECTION_CUTOFF  30
#define WRITE_CONNECTION_CUTOFF  30
#define NO_MSG_NUM -1

#define TCP_MSG_SIGN_BITS   3
#define INT_IN_BYTES_LEN    7
#define CONNECTION REMOTE
#define REMOTESITE SITE

#define NetByteBuffer_SIZE     128
#define NetByteBuffer_CUTOFF   2000
#define NetMsgBuffer_CUTOFF 1000

#define OZReadPortNumber  9000
#define OZStartUpTries    490
#ifdef WINDOWS
/* connect under windows is done in blocking mode */
#define OZConnectTries    10
void osSetNonBlocking(int fd, Bool onoff)
{
  u_long dd = onoff;
  int ret = ioctlsocket(fd,FIONBIO,&dd);
  if (ret<0)
    message("ioctlsocket(%d,FIONBIO,%d) failed: %d\n",fd,ossockerrno(),onoff);
}

#else
#define OZConnectTries    200
#endif
#define OZWritePortNumber 9500



static const int netIntSize=4;
static const int msgNrSize =4;
static const int ansNrSize =4;
static const int tcpSimpleHeaderSize = 1+netIntSize;
static const int tcpHeaderSize=1+netIntSize+msgNrSize + ansNrSize;
static const int tcpConfirmSize=tcpHeaderSize+netIntSize;

enum tcpMessageType {
  TCP_CLOSE_REQUEST_FROM_WRITER = 0,
  TCP_CLOSE_REQUEST_FROM_READER,
  TCP_CLOSE_ACK_FROM_READER,
  TCP_MSG_ACK_FROM_READER,
  TCP_RESEND_MESSAGES,
  TCP_NO_CONNECTION, // from READER
  TCP_CONNECTION,    // from READER
  TCP_PACKET,        // from WRITER
  TCP_MYSITE,        // from WRITER
  TCP_MYSITE_ACK,
  TCP_PING_REQUEST,
  TCP_NONE

};

enum ConnectionFlags{
  CLOSING           = 1,        // has sent CLOSE request
  OPENING           = 2,        // has made connection
  WANTS_TO_CLOSE    = 4,        // is blocked on trying to send CLOSE
  CURRENT           = 8,        // incompleteRead or Write
  WRITE_QUEUE       = 16,       // has something in write queue
  CAN_CLOSE         = 32,       // Zero refs to site, but still something to send
  ACK_MSG_INCOMMING = 64,       // incomplete on back-channel
  WRITE_CON         = 128,      // connection type is WRITE
  MY_DWN            = 256,      // Closed by me
  REFERENCE         = 512,      // There are references to this site
  OK_PROBE          = 2048,     // Probe for Ok
  TMP_DWN           = 4096,     // Closed due to Tmp problem
  RC_READING        = 8192,     // ReadCon in midle of read
  RC_CRASHED        = 16384,    // ReadCon crashed during read
  HIS_DWN           = 32768,    // WriteCon closed by reader
  PROBE             = 65536     // WriteCon in probe list
};

enum ByteStreamType {
  BS_None,
  BS_Marshal,
  BS_Write,
  BS_Read,
  BS_Unmarshal};


#define MAXTCPCACHE 25

enum ipReturn{
  IP_OK              =   0,
  IP_BLOCK           =  -1,
  IP_NO_MORE_TRIES   =  -2,
  IP_TIMER           =  -3,
  IP_CLOSE           =  -4,
  IP_TIMER_EXCEPTION =  -5,
  IP_NET_CRASH       =  -6,
  IP_TEMP_BLOCK      =  -7,
  IP_PERM_BLOCK      =  -8,
  IP_GARBAGE         =  -9,
  IP_EOF             = -10
};

#define LOST -1

enum closeInitiator{
  MY_INITIATIVE,
  HIS_INITIATIVE,
  TMP_INITIATIVE
};

#define WKUPMYC 0
#define WKUPTMP 1
#define WKUPPRB 3

#define WKUPMYC_TIME 500
#define WKUPTMP_TIME 60000
#define WKUPPRB_TIME 3000
#define CLOSE_EXPIRETIME 540000

/* ************************************************************************ */
/*  SECTION 2:  Forward declarations                                        */
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
class NetByteBufferManager;
class MessageManager;
class TcpCache;
class TcpOpenMsgBuffer;
class NetMsgBuffer;

ipReturn tcpSend(int,Message *,Bool);
int intifyUnique(BYTE *);
TcpOpenMsgBuffer *tcpOpenMsgBuffer;
Bool tcpAckReader(ReadConnection*, int);
static ipReturn tcpOpen(RemoteSite *,WriteConnection *) ;
int tcpConnectionHandler(int,void *);

Bool incTimeSlice(unsigned long, void *v);
Bool checkIncTimeSlice(unsigned long, void* v);

static int tcpWriteHandler(int,void*);
static int tcpCloseHandler(int,void*);
int tcpPreReadHandler(int,void*);
static int tcpReadHandler(int,void*);
inline ipReturn writeI(int,BYTE*);
inline ipReturn readI(int,BYTE *);

/* ************************************************************************ */
/*  SECTION 3:  Global Variables                                            */
/* ************************************************************************ */

WriteConnectionManager *writeConnectionManager;
ReadConnectionManager *readConnectionManager;
NetMsgBufferManager *netMsgBufferManager;
RemoteSiteManager *remoteSiteManager;
NetByteBufferManager *netByteBufferManager;
MessageManager *messageManager;
TcpCache *tcpCache;
#ifdef SLOWNET
class TSCQueue;
TSCQueue *TSC;
#endif


class Message{
  friend class IOQueue;
  friend class MessageManager;
  friend class RemoteSite;
  friend class WriteConnection;
protected:
  Message *next;
  tcpMessageType type; // ??
  NetMsgBuffer *bs;
  int remainder;
  int msgNum;
  DSite *site;// not needed ??
  MessageType msgType;
  int storeIndx;

#ifdef SLOWNET
  int time;
#endif

public:
  Message(){}
  void init(NetMsgBuffer *b, int n,
            DSite *s, MessageType msg, int stI){
    next=NULL;
    bs=b;
    remainder=0;
    type=TCP_NONE;
    msgNum = n;
    site = s;
    msgType = msg;
    storeIndx = stI;
#ifdef SLOWNET
    time = 0;
#endif
  }
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
  void setMsgNum(int n){
    msgNum = n;}
  void resend();

  int getMsgType(){
    return (int)msgType;}

  DSite* getSite(){
    return site;}

  int getMsgIndex(){
    return storeIndx;
  }
#ifdef SLOWNET
  int getTime(){
    return time;}
  void setTime(int t){
    time = t;}
#endif

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
  Bool     queueEmpty(){
    Assert(((first==NULL) && (last==NULL))||
           ((first!=NULL) && (last!=NULL)));
    return first == NULL;}
  Message* find(int msg){
    Assert(((first==NULL) && (last==NULL))||((first!=NULL) && (last!=NULL)));
    Message *b=first,*d=NULL;
    while(b!=NULL && b->msgNum != msg)
      { d = b; b = b->next;}
    if(b == NULL) return NULL;
    if(b == last)
      last = d;
    if(b == first)
      first = b->next;
    else
      d->next = b->next;
    return b;}

  void enqueue(Message *m);
  void dequeue(Message *m);
  void addfirst(Message *m);
};

/**********************************************************************/
/*   SECTION 3b:  Transfer speed control                             */
/**********************************************************************/
#ifdef SLOWNET

int  TSC_LATENCY;
int TSC_TOTAL_A;
#define READ_NO_TIME        (0-1)
#define TSCQ_WRITE          0
#define TSCQ_READ           1
#define TSCQ_TIME_SLICE     10


class TSCQElement{
public:
  unsigned long t;
  void *c;
  int  type;
  TSCQElement *e;
  TSCQElement(unsigned long tt, int ty, void *cc, TSCQElement* ee){
    t = tt;
    c = cc;
    e = ee;
    type = ty;}
};

TSCQElement* newTSCQElement(){
  return (TSCQElement*) genFreeListManager->getOne_4();}

void freeTSCQElement(TSCQElement* e){
  genFreeListManager->putOne_4((FreeListEntry*) e);}


class TSCQueue{
private:
  int transferAmount;
  unsigned long time, dtime;
  TSCQElement *ptr, *last;
  int nrOfReads;
  Bool reAdd;
public:
  TSCQueue(){
    transferAmount     =  0;
    time = 0;
    dtime = 0;
    ptr = NULL;
    last = NULL;
    nrOfReads = 0;
    reAdd = FALSE;
  }

  void writing(int size){
    transferAmount += size;
    //   printf("writing %d tot:%d?n",size, writeAmount);
  }

  void reading(int size){
    transferAmount += size;}

  Bool netAmountFull(){
    return transferAmount > TSC_TOTAL_A;
  }

  Bool hasTask(unsigned long aTime){
    time = aTime;
    return  ptr && ptr->t <= aTime;}
  //(time + ++dtime * TSCQ_TIME_SLICE);}

  unsigned long getCurTime() {
    return time;}
  unsigned long getNewTime(){
    return time + TSC_LATENCY;}

  void addelement(TSCQElement* ele){
    if(ptr == NULL)
      ptr = ele;
    else
      last->e = ele;
    last = ele;}

  void addRead(void* ele){
    TSCQElement *TS = newTSCQElement();
    TS->TSCQElement::TSCQElement(getNewTime(),TSCQ_READ, ele, NULL);
    addelement(TS);}
  void addWrite(void *ele){
    TSCQElement *TS = newTSCQElement();
    TS->TSCQElement::TSCQElement(getNewTime(),TSCQ_WRITE, ele, NULL);
    addelement(TS);
  }
  void reAddWrite(){
    Assert(!reAdd);
    reAdd = TRUE;}

  void incTime(unsigned long);
};



#endif
/* ************************************************************************ */
/*  SECTION 4:  Network MsgBuffer and friends                                       */
/* ************************************************************************ */

class NetByteBuffer{
  friend class NetByteBufferManager;
  friend class NetMsgBuffer;
  friend class NetMsgBufferManager;

protected:
  BYTE buf[NetByteBuffer_SIZE];
  NetByteBuffer *next;

public:
  NetByteBuffer(){}

  BYTE *head(){return buf;}
  BYTE *tail(){return buf+NetByteBuffer_SIZE-1;}
  void init() {
    next=NULL;
#ifdef DEBUG_CHECK
    /*    for(int i=0;i<NetByteBuffer_SIZE;i++)
      buf[i]=0x44;
      Assert(*this->head()==0x44); */
#endif
  }
};

class NetByteBufferManager: public FreeListManager {
public:
  NetByteBufferManager():FreeListManager(NetByteBuffer_CUTOFF){wc = 0;}
  int wc;

  NetByteBuffer*newNetByteBuffer(){
    FreeListEntry *f=getOne();
    NetByteBuffer *bb;
    if(f==NULL) {bb=new NetByteBuffer();}
    else{GenCast(f,FreeListEntry*,bb,NetByteBuffer*);}
    bb->init();
    ++wc;
    return bb;}

  void deleteNetByteBuffer(NetByteBuffer* bb){
    FreeListEntry *f;
    --wc;
    GenCast(bb,NetByteBuffer*,f,FreeListEntry*);
    if(putOne(f)) return;
    delete bb;
    return;}
  int getCTR(){ return wc;}

};





class NetMsgBuffer:public MsgBuffer{
  friend class NetMsgBufferManager;
#ifdef SLOWNET
  friend ipReturn interpret(NetMsgBuffer *bs,tcpMessageType type, Bool ValidMsg);
#endif
protected:
  NetByteBuffer *first;
  NetByteBuffer *start;
  NetByteBuffer *stop;
  NetByteBuffer *last;
  BYTE *pos;
  BYTE *curpos;
  BYTE *endpos;
  int totlen;  /* include header */
  int type;
  DSite *site;
  int slownetTotLen;    // used for slownet communication


  RemoteSite *remotesite;
  int availableSpace();

  NetByteBuffer *beforeLast(){
    Assert(first!=last);
    NetByteBuffer *bb=first;
    while(bb->next!=last){bb=bb->next;}
    return bb;}

  Bool within(BYTE*,NetByteBuffer*);

public:
  NetMsgBuffer() {}

  //ATTENTION Hack Remove
  void startfixerik1(){
    NetMsgBuffer();}

  void setslownetTotLen(int s){slownetTotLen = s;}
  int getslownetTotLen(){return slownetTotLen;}


  void resend();
  void reset();
  void unmarshalReset();

  void init(){
    MsgBuffer::init();
    type=BS_None;first=NULL;start=NULL;last=NULL;
    site=NULL;remotesite=NULL;stop=NULL;}

  void init(DSite *s) {
    init();
    site=s;
  }

  void setSite(DSite *s){ site = s;}
  DSite* getSite(){ return site;}

  Bool isPersistentBuffer() { return NO; }

  RemoteSite* getRemoteSite(){
    return remotesite;}

  char *siteStringrep();
  int getTotLen();

  void removeFirst(){
    Assert(first!=last);
    NetByteBuffer *bb=first;
    first=bb->next;
    netByteBufferManager->deleteNetByteBuffer(bb);
  }

  void   constructMsg(RemoteSite*,tcpMessageType);

  void removeSingle(){
    Assert(first==last);
    Assert(first!=NULL);
    netByteBufferManager->deleteNetByteBuffer(first);
    last=first=NULL;}

  NetByteBuffer* getAnother();
  void getSingle();

  void marshalBegin(){
    PD((MARSHAL_BE,"marshal begin"));
    Assert(type==BS_None);
    Assert(first==NULL);
    Assert(last==NULL);
    first=getAnother();
    start=first;
    last=first;
    stop=first;
    totlen= 0;
    type=BS_Marshal;
    posMB=first->head()+tcpHeaderSize;
    //    Assert(*posMB == 0x44);
    endMB=first->tail();
    Assert(posMB<endMB);
    pos=NULL;}

  void putNext(BYTE b){
    Assert(type==BS_Marshal);
    Assert(posMB==endMB+1);
    PD((BUFFER,"NetByteBuffer alloc Marshal: %d",no_bufs()));
    NetByteBuffer *bb=getAnother();
    last->next=bb;
    last=bb;
    stop=bb;
    totlen += NetByteBuffer_SIZE;
    posMB=bb->head();
    endMB=bb->tail();
    // Assert(*posMB == 0x44);
    *posMB++=b;
    return;}

  void marshalEnd(){
    Assert(type==BS_Marshal);
    endpos=posMB;
    pos=first->head();
    if(endpos==NULL) {totlen +=NetByteBuffer_SIZE;}
    else {totlen +=endpos-last->head();}
    type=BS_None;}

  void beginWrite(RemoteSite*); /* putting end packet header */
  void PiggyBack(Message*);

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
    else {Assert(within(curpos,last));}
    posMB=pos;
    if(within(curpos,first)) {
      endMB=curpos;}
    else{
      endMB=first->tail();}
    pos=NULL;}

  BYTE getNext(){
    Assert(type==BS_Unmarshal);
    Assert(posMB!=NULL);
    if(posMB==curpos){
      posMB=NULL;
      Assert(first==last);
      return *curpos;}
    if(posMB==first->tail()){
      BYTE ch;
      Assert(first!=last);
      ch=*posMB;
      removeFirst();
      posMB=first->head();
      if(within(curpos,first)) {
        endMB=curpos;}
      else{
        endMB=first->tail();}
      return ch;}
    Assert(0);
    return 0;}

  void unmarshalEnd(){
    Assert(type==BS_Unmarshal);
    type=BS_None;
    Assert(posMB==NULL);}

  int no_bufs();

  void dumpNetByteBuffers(){
    /*if(type == BS_Write){
      Assert(start != NULL);
      while(start!=stop) {
        NetByteBuffer *bb=start;
        start=bb->next;
        netByteBufferManager->deleteNetByteBuffer(bb);
        }
      netByteBufferManager->deleteNetByteBuffer(start);
      start=stop=NULL;}
      else{*/
    Assert(first != NULL);
    while(first!=last) {
      NetByteBuffer *bb=first;
      first=bb->next;
      netByteBufferManager->deleteNetByteBuffer(bb);
    }
    netByteBufferManager->deleteNetByteBuffer(first);
    first=last=NULL; //}
  }
};


void Message::resend(){
  next = NULL;
  bs->resend();}
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

Bool NetMsgBuffer::within(BYTE *p,NetByteBuffer *bb){
  if(p<bb->head()) return FALSE;
  if(p>bb->tail()) return FALSE;
  return TRUE;}

int NetMsgBuffer::no_bufs(){
  int i=0;
  NetByteBuffer *bb=first;
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
  PD((BUFFER,"NetByteBuffer sent: %d",no_bufs()));
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
    NetByteBuffer *bb=first->next;
    while(bb->next!=NULL){
      i+=NetByteBuffer_SIZE;
      bb=bb->next;}
    Assert(bb==last);
    if(endpos==NULL){return i+NetByteBuffer_SIZE;}
    return i+endpos-last->head();}}


void NetMsgBuffer::incPosAfterWrite(int i){
    Assert(type==BS_Write);
    Assert(within(pos,first));
    Assert(pos+i<=first->tail());
    pos +=i;}


void NetMsgBuffer::getSingle(){
  Assert(first==NULL);
  Assert(last==NULL);
  NetByteBuffer *bb=getAnother();

  if(type == BS_Write){
    // EK
    // Must this be so?
    start = bb;stop = bb;}
  first=bb;
  last=bb;}

int NetMsgBuffer::getWriteLen(){
  Assert(type==BS_Write);
  Assert(first!=NULL);
  if(first==last){
    if(endpos!=NULL) {return endpos-pos;}
    return first->tail()-pos+1;}
  return first->tail()-pos+1;
}

#define RECYCLE  (NetByteBuffer*) 0x1


class NetMsgBufferManager:public FreeListManager{

  NetMsgBuffer*newNetMsgBuffer(DSite *s){
    FreeListEntry *f=getOne();
    NetMsgBuffer *bb;
    if(f==NULL) {bb=new NetMsgBuffer();}
    else{GenCast(f,FreeListEntry*,bb,NetMsgBuffer*);}
    bb->startfixerik1();
    bb->init(s);
    wc++;
    return bb;}

  void deleteNetMsgBuffer(NetMsgBuffer* b){
    Assert(b->first == b->last);
    Assert(b->first == NULL);
    DebugCode(b->start=RECYCLE;b->stop=RECYCLE;
    b->first=RECYCLE;b->last=RECYCLE;);

    wc--;
    FreeListEntry *f;
    GenCast(b,NetMsgBuffer*,f,FreeListEntry*);
    if(putOne(f)) return;
    delete b;}
public:
  NetMsgBufferManager():FreeListManager(NetMsgBuffer_CUTOFF){wc=0;}

  int wc; // for debug purposes

  void dumpNetMsgBuffer(NetMsgBuffer* nb) {
    Assert(nb->start != RECYCLE);
    Assert(nb->stop  != RECYCLE);
    Assert(nb->first != RECYCLE);
    Assert(nb->last  != RECYCLE);


    if(nb->start!=NULL && nb->first==NULL)
      nb->reset();
    if(nb->first!=NULL)
      nb->dumpNetByteBuffers();
    deleteNetMsgBuffer(nb);}

  NetMsgBuffer *getNetMsgBuffer(DSite *s) {
    return newNetMsgBuffer(s);}

  int getCTR(){ return wc;}
};

NetByteBuffer *NetMsgBuffer::getAnother(){
  NetByteBuffer* bb = netByteBufferManager->newNetByteBuffer();
  Assert(bb->next == NULL);
  /* Assert(*bb->tail() == 0x44);
     Assert(*bb->head() == 0x44); */
  return bb;}

/* *********************************************************************/
/*   SECTION 5: RemoteSite                                          */
/* *********************************************************************/


class RemoteSite{
  friend class RemoteSiteManager;

  int recMsgCtr;
  int tmpMsgNum;             // optimize away
  SiteStatus status;         // TEMP,PERM,OK

  // This is all monitor stuff.
  // should be encapsulated in an  object with manager
  int totalMsgSize;      // size of queued msgs
  int MsgMonitorSize;    // limits given by protocol-layer
  int treashFullCtr;

  WriteConnection *writeConnection;
  ReadConnection *readConnection;
  Message *sentMsg;      // non-acknowledge msgs
  IOQueue writeQueue; //Unsent Messages,
  int tSn;

  int nrOfSentMsgs;
  int nrOfRecMsgs;

public:
  DSite* site;

protected:
  void init(DSite*, int);
public:
  RemoteSite(): writeConnection(NULL),readConnection(NULL){}

  void incNOSM(){nrOfSentMsgs = (1 + nrOfSentMsgs) %  199802;}
  void incNORM(){nrOfRecMsgs = (1 + nrOfRecMsgs)  %  199802;}
  int  getNOSM(){
    int tmp =  nrOfSentMsgs;
    nrOfSentMsgs = 0;
    return tmp;}
  int  getNORM(){
    int tmp =  nrOfRecMsgs;
    nrOfRecMsgs = 0;
    return tmp;}

  void setAckStartNr(int nr);
  int resendAckQueue(Message *m);

  int readRecMsgCtr();
  void receivedNewAck(int);

  int sendTo(NetMsgBuffer*,MessageType,DSite*, int);
  void zeroReferences();

  void writeConnectionRemoved();
  void readConnectionRemoved();

  void addWriteQueue(Message* m,int msg){
    m->setMsgNum(msg);
    addWriteQueue(m);}

  void addWriteQueue(Message*);
  void addFirstWriteQueue(Message* m){
    writeQueue.addfirst(m);}

  Message* getWriteQueue();

  SiteStatus siteStatus(){return status;}
  void setSiteStatus(SiteStatus s){status = s;}

  void siteTmpDwn(closeInitiator);
  void sitePrmDwn();

  int getTmpMsgNum(){ return tmpMsgNum++;}

  Bool receivedNewMsg(int);
  int getRecMsgCtr();
  void clearRecMsgCtr(){recMsgCtr = 0;}

  void storeSentMessage(Message* bs);

  TimeStamp *getTimeStamp(){return &site->timestamp;}
  port_t getPort(){return site->port;}
  ip_address getAddress(){return site->address;}
  int getTmpSessionNr(){return tSn;}
  void incTmpSessionNr(){tSn = ++tSn % 199802;}

  WriteConnection *getWriteConnection(){return writeConnection;}
  ReadConnection *getReadConnection(){return readConnection;}

  void setWriteConnection(WriteConnection *r);
  void setReadConnection(ReadConnection *r);


  void queueMessage(int size){
    Assert(totalMsgSize >=0);
    totalMsgSize += size;
    PD((WRT_QUEUE,"New entry s:%d tS:%d tr:%d",size,totalMsgSize,MsgMonitorSize));}

  void deQueueMessage(int size){
    PD((WRT_QUEUE,"Rm entry s:%d tS:%d tr:%d",size,totalMsgSize,MsgMonitorSize));
    totalMsgSize -= size;
    if(totalMsgSize < 0) totalMsgSize = 0;
    Assert( totalMsgSize >= 0);
    if (totalMsgSize <= MsgMonitorSize){
      treashFullCtr = (1+treashFullCtr) % 100000;}}

  void monitorQueueMsgs(int SizeMess){
    Assert(MsgMonitorSize == -1);
    MsgMonitorSize=SizeMess;}
  void deMonitorQueue(){MsgMonitorSize = -1;}
  int  getQueueSize(){return totalMsgSize;}
  int  getTreashCtr(){return treashFullCtr;}
  DSite* getSite(){return site;}

  ProbeReturn installProbe(ProbeType);
  ProbeReturn deInstallProbe(ProbeType);

  void sendAck();
};


/***********************************************************************/
/* SECTION 6: Read and Write - Connection                                */
/***********************************************************************/

class Connection {
public:
  int fd;               // LOST means no connection
  int flags;
  RemoteSite *remoteSite;
  Message* current;
  Connection *next, *prev;

  void setFlag(int f){flags |= f;}
  void clearFlag(int f){flags &= ~f;}
  Bool testFlag(int f){return f & flags;}

  Bool isWriteCon(){return testFlag(WRITE_CON);}

  void setIncomplete(){
    Assert(!testFlag(CURRENT));
    PD((TCP_INTERFACE,"setIncomplete site:%s",remoteSite->site->stringrep()));
    setFlag(CURRENT);}
  Bool isIncomplete(){
    return testFlag(CURRENT);}
  void clearIncomplete(){
    Assert(testFlag(CURRENT));
    PD((TCP_INTERFACE,"clearIncomplete site:%s",remoteSite->site->stringrep()));
    clearFlag(CURRENT);}

  void setSite(RemoteSite *s){remoteSite=s;}

  void setClosing();
  Bool isClosing(){
    return testFlag(CLOSING);}
  void clearClosing(){
    PD((TCP_INTERFACE,"clearClosing site:%s",remoteSite->site->stringrep()));
    Assert(testFlag(CLOSING));
    clearFlag(CLOSING);}

  Bool isRemovable(){return flags==0;}

  RemoteSite *getRemoteSite(){return remoteSite;}

  void connectionLost(){remoteSite->site->discoveryPerm();}
  void connectionBlocked(){remoteSite->siteTmpDwn(TMP_INITIATIVE);}

  void clearFD(){fd=LOST;}
  void setFD(int f){fd=f;}
  int getFD(){ return fd;}

  void addCurQueue(Message *m){
    PD((TCPQUEUE,"add CurQueue m:%x r:%x",m,this));
    setIncomplete();
    current = m;}
  Message* getCurQueue(){
    Message *m = current;
    current = NULL;
    clearIncomplete();
    return m;}

  void disconnect(){
    Assert(next!=NULL);
    Assert(prev!=NULL);
    next->prev=prev;
    prev->next=next;
    next=NULL;
    prev=NULL;}

  Bool isOpening(){
    return testFlag(OPENING);}
  void setOpening(){
     Assert(!testFlag(OPENING));
     setFlag(OPENING);}
  void clearOpening(){
    Assert(testFlag(OPENING));
    clearFlag(OPENING);}


};

class ReadConnection:public Connection{
  friend class ReadConnectionManager;
  friend class TcpCache;

protected:
  int  maxSizeAck;
  int  recSizeAck;
#ifdef SLOWNET
  int  time;
#endif
public:


  void fastfixerik2()
  {Connection();}

  void receivedNewSize(int size){
    recSizeAck +=  size;
    PD((ACK_QUEUE,"SizeReceived s: %d l: %d",recSizeAck ,maxSizeAck));
    if(recSizeAck >= maxSizeAck)  sendAck();}

  void sendAck(){
    if(tcpAckReader(this,remoteSite->getRecMsgCtr())){
      PD((ACK_QUEUE,"Ack, limit reached...."));
      recSizeAck = 0;}}

  void messageSent(){recSizeAck=0;}

  void informSiteRemove(){
    remoteSite->readConnectionRemoved();}

  Bool resend();

  void setMaxSizeAck(int max){
    PD((SITE,"Max size: %d",max));
    maxSizeAck = max;
    recSizeAck = 0;}

  Bool informSiteAck(int, int);

  void init(RemoteSite *s){
    next = prev = NULL;
    flags=0;
    current = NULL;
    maxSizeAck = 0;
    recSizeAck = 0;
    remoteSite=s;
    current=NULL;
#ifdef SLOWNET
    time = READ_NO_TIME;
#endif

  }

#ifdef SLOWNET
  int getTime(){
    return time;}
  void setTime(int t){
    time = t;}
#endif


  Bool canbeClosed(){
    if(isClosing()) return FALSE;
    if(isOpening()) return FALSE;
    return TRUE;}

  Bool goodCloseCand(){
    return canbeClosed() && !isIncomplete();}

  void niceClose(){remoteSite->clearRecMsgCtr();}
  void close();
  void prmDwn();
  void closeConnection();

  void setReading(){setFlag(RC_READING);}
  Bool isReading(){return testFlag(RC_READING);}
  void clearReading(){clearFlag(RC_READING);}

  void setCrashed(){setFlag(RC_CRASHED);}
  Bool isCrashed(){return testFlag(RC_CRASHED);}
  void clearCrashed(){clearFlag(RC_CRASHED);}

};

class WriteConnection:public Connection{
  friend class WriteConnectionManager;
  friend class TcpCache;
protected:
  BYTE intBuffer[INT_IN_BYTES_LEN];
  BYTE *bytePtr;
  int sentMsgCtr;
  Message *sentMsg;      // non-acknowledge msgs
public:
  unsigned long expireTime;

  void fastfixerik3(){
    Connection();}

  void close(closeInitiator);
  void closeConnection();

  void informSiteRemove(){
    remoteSite->writeConnectionRemoved();}

  ipReturn open();

  Bool isReadReading(){
    ReadConnection *rr = remoteSite->getReadConnection();
    return (rr && rr->isReading()) ;}

  Bool shouldSendFromUser();

  void informSiteResendAckQueue();

  void storeSentMessage(Message* m);

  int getMsgCtr(){return sentMsgCtr;}
  int incMsgCtr(){return ++sentMsgCtr;}

  Bool checkAckQueue();

  void ackReceived(int);


  Bool goodCloseCand(){
    return (canbeClosed() && (!isWritePending()));}

  void opened(){
    Assert(isOpening());
    PD((TCP_INTERFACE,"opened site:%s",remoteSite->site->stringrep()));
    expireTime = 0;
    clearOpening();
    remoteSite->setSiteStatus(SITE_OK);
    remoteSite->incTmpSessionNr();
    if(isProbingOK()){
      remoteSite->site->probeFault(PROBE_OK);
      clearProbingOK();}}

  Bool addByteToInt(BYTE msg){
    if(!testFlag(ACK_MSG_INCOMMING))
      return FALSE;
    *bytePtr++ = msg;
    PD((TCP,"Reading byte: %d diff: %d tot: %d",
        msg, bytePtr - intBuffer, INT_IN_BYTES_LEN));
    if((bytePtr - intBuffer) == INT_IN_BYTES_LEN){
      clearFlag(ACK_MSG_INCOMMING);
      ackReceived(intifyUnique(intBuffer));
      return FALSE;}
    return TRUE; }

  void prmDwn();

  Bool isInWriteQueue(){
    Assert(!isIncomplete());
    return testFlag(WRITE_QUEUE);}
  void setInWriteQueue(){
    PD((TCP_INTERFACE,"setInWriteQueue s:%s",remoteSite->site->stringrep()));
    setFlag(WRITE_QUEUE);}
  void clearInWriteQueue(){
    PD((TCP_INTERFACE,"Removed from writequeue  s:%s",  remoteSite->site->stringrep()));
    clearFlag(WRITE_QUEUE);}


  Bool isProbingOK(){
    return testFlag(OK_PROBE);}
  void setProbingOK(){
    setFlag(OK_PROBE);}
  void clearProbingOK(){
    clearFlag(OK_PROBE);}
  Bool isProbing(){
    return testFlag(PROBE);}
  void setProbing(){
    setFlag(PROBE);}
  void clearProbing(){
    clearFlag(PROBE);}

  Bool hasReferences(){
    return testFlag(REFERENCE);}
  void setReference(){
    setFlag(REFERENCE);}
  void removeReference(){
    clearFlag(REFERENCE);}

  void setWantsToClose();
  void clearWantsToClose();
  Bool isWantsToClose(){
    return testFlag(WANTS_TO_CLOSE);}

  void incommingAck(){
    bytePtr = intBuffer;
    setFlag(ACK_MSG_INCOMMING);}
  Bool isWritePending(){
    return testFlag(WRITE_QUEUE|CURRENT);}

  // Zero references to Site. When no msgs are waiting
  // close the writeConnedtion.
  void setCanClose(){
    // EK assertion?
    PD((TCP_INTERFACE,"You can now close site:%s",remoteSite->site->stringrep()));
    setFlag(CAN_CLOSE);}
  Bool isCanClose(){
    return testFlag(CAN_CLOSE);}
  void clearCanClose(){
    Assert(isCanClose());
    clearFlag(CAN_CLOSE);}

  Bool canbeClosed(){
    if(isClosing()) return FALSE;
    if(isOpening()) return FALSE;
    // Ek is it writing? insert that here
    return TRUE;}

  Bool isTmpDwn(){
    return testFlag(TMP_DWN);}
  void setTmpDwn(){
    Assert(!isTmpDwn());
    setFlag(TMP_DWN);}
  void clearTmpDwn(){
    Assert(isTmpDwn());
    clearFlag(TMP_DWN);}
  Bool isAcked(){
    return sentMsg==NULL;}

  Bool isMyInitiative(){
    return testFlag(MY_DWN);}
  void setMyInitiative(){
    Assert(!isMyInitiative());
    setFlag(MY_DWN);}
  void clearMyInitiative(){
    Assert(isMyInitiative());
    clearFlag(MY_DWN);}

  Bool isHisInitiative(){
    return testFlag(HIS_DWN);}
  void setHisInitiative(){
    Assert(!isHisInitiative());
    setFlag(HIS_DWN);}
  void clearHisInitiative(){
    Assert(isHisInitiative());
    clearFlag(HIS_DWN);}

  void init(RemoteSite *s){
    next = prev = NULL;
    current = NULL;
    sentMsg = NULL;
    sentMsgCtr =0;
    flags=WRITE_CON;
    setReference();
    remoteSite=s;
    bytePtr = intBuffer + INT_IN_BYTES_LEN;}
};

/************************************************************/
/*  SECTION 7: MySiteInfo                                      */
/************************************************************/
class MySiteInfo{
public:
  int        tcpFD;    // the file descriptor of the port
  RemoteSite       *site;
  int        maxNrAck;
  int        maxSizeAck;
}mySiteInfo;

/************************************************************/
/* SECTION 8: Managers                                         */
/************************************************************/

class RemoteSiteManager: public FreeListManager{

  RemoteSite* newRemoteSite(){
    RemoteSite* s;
    FreeListEntry *f=getOne();
    if(f==NULL) {s=new RemoteSite();}
    else{GenCast(f,FreeListEntry*,s,RemoteSite*);}
    wc ++;
    return s;}

  void deleteRemoteSite(RemoteSite *s){
    FreeListEntry *f;
    wc--;
    GenCast(s,RemoteSite*,f,FreeListEntry*);
    if(putOne(f)) {return;}
    delete s;
      return;}

public:
  int wc;
  RemoteSiteManager():FreeListManager(SITE_CUTOFF){wc = 0;}

  void freeRemoteSite(RemoteSite *s){
    deleteRemoteSite(s);}

  RemoteSite* allocRemoteSite(DSite *s, int r){
    RemoteSite *rs=newRemoteSite();
    PD((SITE,"allocated a:%x ctr:%d rs:%x",s,r,rs));
    rs->init(s, r);
    return rs;}

  int getCTR(){return wc;}
};

class ReadConnectionManager: public FreeListManager{
  int wc;
  void deleteConnection(ReadConnection *r){
    FreeListEntry *f;
    GenCast(r,ReadConnection*,f,FreeListEntry*);
    if(!putOne(f)) {delete r;}
    wc--;
    return;}
  ReadConnection* newConnection(RemoteSite *s,int fd0){
    FreeListEntry *f=getOne();
    ReadConnection *r;
    if(f==NULL) {r=new ReadConnection();}
    else {GenCast(f,FreeListEntry*,r,ReadConnection*);}
    r->fastfixerik2();
    r->init(s);
    r->fd=fd0;
    wc++;
    return r;}
public:
  ReadConnectionManager():FreeListManager(READ_CONNECTION_CUTOFF){wc=0;}

  void freeConnection(ReadConnection *r){
    //Assert(r->isRemovable());
    deleteConnection(r);
    return;}

  ReadConnection *allocConnection(RemoteSite *s,int f){
    ReadConnection *r=newConnection(s,f);
    return r;}

  int getCTR(){return wc;}

};


class WriteConnectionManager: public FreeListManager{
  int wc;

  void deleteConnection(WriteConnection *r){
    FreeListEntry *f;
    GenCast(r,WriteConnection*,f,FreeListEntry*);
    if(!putOne(f)) {delete r;}
    wc--;
    return;}
  WriteConnection* newConnection(RemoteSite *s,int fd0){
    FreeListEntry *f=getOne();
    WriteConnection *r;
    if(f==NULL) {r=new WriteConnection();}
    else {GenCast(f,FreeListEntry*,r,WriteConnection*);}
    r->fastfixerik3();
    r->init(s);
    r->fd=fd0;
    wc++;
    return r;}
public:
  WriteConnectionManager():
    FreeListManager(WRITE_CONNECTION_CUTOFF){wc = 0;}

  void freeConnection(WriteConnection *r){
    r->clearFlag(WRITE_CON);
    deleteConnection(r);
    return;}

  WriteConnection *allocConnection(RemoteSite *s,int f){
    WriteConnection *r=newConnection(s,f);
    return r;}

  int getCTR(){return wc;}

 };

/************************************************************/
/* SECTION 9: small routines for RemoteSites                   */
/************************************************************/

void RemoteSite::zeroReferences(){
  return;
  PD((SITE,"Zero references to site %s",site->stringrep()));
  if(writeConnection == NULL)  return;
  writeConnection->removeReference();
  if(writeConnection->goodCloseCand() && !writeConnection->isTmpDwn() && !writeConnection->isProbing())
    writeConnection->closeConnection();
  else
    writeConnection -> setCanClose();}

void RemoteSite::writeConnectionRemoved(){
    writeConnection = NULL;
    if(readConnection == NULL){
      site->dumpRemoteSite(recMsgCtr);
      remoteSiteManager->freeRemoteSite(this);}}

void RemoteSite::readConnectionRemoved(){
  readConnection = NULL;
    if(writeConnection == NULL){
      site->dumpRemoteSite(recMsgCtr);
      remoteSiteManager->freeRemoteSite(this);}}


/************************************************************/
/* SECTION 10: tcpError                                         */
/************************************************************/


ipReturn tcpError()
{
switch(ossockerrno()){
  case EPIPE:{
    PD((TCP_HERROR,"Connection socket lost: EPIPE"));
    return IP_PERM_BLOCK;}
  case ECONNRESET:{
    PD((TCP_HERROR,"Connection socket lost: ECONNRESET"));
    return IP_PERM_BLOCK;}
  case EBADF:{
    PD((TCP_HERROR,"Connection socket lost: EBADF"));
    return IP_PERM_BLOCK;}
#ifndef WINDOWS
case ETIMEDOUT:{
  PD((TCP_HERROR,"Connection socket temp: ETIMEDOUT"));
  return IP_TEMP_BLOCK;}
#endif
default:{
  PD((TCP_HERROR,"Unhandled error: %d please inform erik@sics.se",
      ossockerrno()));
  DebugCode(fprintf(stderr,"default interpreted as perm:%d \n",ossockerrno()));
  return IP_PERM_BLOCK;}}
return IP_TEMP_BLOCK;
}
/************************************************************/
/* SECTION 11: TcpCache                                         */
/************************************************************/

class TcpCache {
  Connection* currentHead; // CurrentConnections
  Connection* currentTail; //
  Connection* closeHead;   // Connections that are closing
  Connection* closeTail;   //
  Connection* myHead;      // Writes that this site has taken down
  Connection* myTail;      //
  Connection* tmpHead;     // Connections that are tmp down
  Connection* tmpTail;     //
  Connection* probeHead;     // Connections that are tmp down
  Connection* probeTail;     //
  int current_size;
  int close_size;
  int open_size;
  int max_size;
  unsigned long tmpTime;
  unsigned long probeTime;
  unsigned long myTime;
  unsigned long curTime;

  void setMinTime(int time){
    int testTime = time + 1;
    if(time < min(min(tmpTime?WKUPTMP_TIME:testTime,
                      probeTime?WKUPPRB_TIME:testTime) ,
                  myTime?WKUPMYC_TIME:testTime)){
      //printf("SettingMinTime %d\n",time);
      am.setMinimalTaskInterval((void*)this, time);}
  }

  void newTmpDwn(){
    if(!tmpTime){
      setMinTime(WKUPTMP_TIME);
      tmpTime=curTime+WKUPTMP_TIME;}
  }

  void newProbe(){
    if(!probeTime){
      setMinTime(WKUPPRB_TIME);
    probeTime=curTime+WKUPPRB_TIME;}
  }
  void newMyDwn(){
    if(!myTime)   {
      setMinTime(WKUPMYC_TIME);
      myTime=curTime+WKUPMYC_TIME;}
  }


  void addToFront(Connection *r, Connection* &head, Connection* &tail){
    if(head==NULL){
      Assert(tail==NULL);
      r->next=NULL;
      r->prev=NULL;
      head=r;
      tail=r;
      Assert(r->prev==NULL);
      Assert(r->next==NULL);
      return;}
    r->next=head;
    r->prev=NULL;
    head->prev=r;
    head=r;
    Assert(r->prev==NULL);
    Assert(r->next!=NULL);
}

  void addToTail(Connection *r, Connection* &head, Connection* &tail){
    if(tail==NULL){
      Assert(head==NULL);
      r->next=NULL;
      r->prev=NULL;
      head=r;
      tail=r;
      return;}
    Assert(tail->next==NULL);
    r->next=NULL;
    r->prev=tail;
    tail->next=r;
    tail=r;}


  Connection* getLast(Connection* &head,Connection* &tail){
    // PD((TCPCACHE,"get last"));
    Connection *w = tail;
    if (head == tail)
      head = tail = NULL;
    else{
      tail = w->prev;
      w->prev = NULL;
      Assert(w->next==NULL);
      tail->next = NULL;}
    return w;}

  void unlink(Connection *r,Connection* &head, Connection* &tail){
    //    PD((TCPCACHE,"cache unlink r:%x",r));

    if(tail==r) {
      if(head==r){
        Assert(r->next==NULL);
        Assert(r->prev==NULL);
        tail=NULL;
        head=NULL;
        return;}
      Assert(r->next==NULL);
      Assert(r->prev!=NULL);
      tail=r->prev;
      tail->next=NULL;
      r->prev=NULL;
      return;}
    if(head==r) {
      Assert(r->prev==NULL);
      Assert(r->next!=NULL);
      head=r->next;
      head->prev=NULL;
      r->next=NULL;
      return;}
    Assert(r->next!=NULL);
    Assert(r->prev!=NULL);
    r->disconnect();}

  void decreaseConnections() {
    PD((TCPCACHE,"close"));
    WriteConnection* w;
    ReadConnection* r;
    Connection *c=currentTail;
    WriteConnection *cncls = NULL;
    while(c!=NULL){
      if(c->isWriteCon()){
        GenCast(c, Connection*, w,WriteConnection*);
        if(w->goodCloseCand()){
          w->closeConnection();
          //printf("WriteCon closed by me %d\n",(int)w);
          return;}
        if(cncls==NULL && w->canbeClosed())
          cncls = w;}
      else{
        GenCast(c, Connection*,  r, ReadConnection*);
        if(r->goodCloseCand()){
          //printf("ReadCon closed by me %d\n",(int)r);
          r->closeConnection();
          return;}}
    c = c->next;}
    if(open_size>max_size * 2 && cncls != NULL){
      //printf("Forced to close %d\n",(int)cncls);
      cncls->closeConnection();
      return;}
    //printf("NoOneToClose\n");
  }

public:
  Bool accept;
  Bool openCon;
  Bool probes;
  Bool shutDwn;


  int shutDwnTcpCacheProgres(){
    Connection *cu=currentHead, *cl = closeHead;
    int open = 0, closed = 0;
    while(cu!=NULL) {
      open++;
      cu = cu->next;}
    while(cl!=NULL) {
      closed++;
      cl = cl->next;}
    return open + closed;}

  int shutDwnTcpCache(){
    shutDwn = TRUE;
    closeConnections();
    return shutDwnTcpCacheProgres();}


  void closeConnections(){
    PD((TCPCACHE,"ClosingConnections fakingTmp"));
    Connection *c=currentHead, *cc;
    while(c!=NULL){
      //printf("closing %d %s\n",(int)c,myDSite->stringrep());
      cc = c->next;
      if(!c->isClosing())
        {
          if(c->testFlag(WRITE_CON))
            {
              if(shutDwn && !((WriteConnection*)c)->goodCloseCand())
                ((WriteConnection*)c)->setCanClose();
              else
                ((WriteConnection*)c)->closeConnection();
            }
          else
            {
              if(!c->isOpening()){
                ((ReadConnection*)c)->closeConnection();}
            }
        }
      c = cc;}
    openCon = FALSE;}


  void openConnections(){
    if(shutDwn) return;
    PD((TCPCACHE,"OpeningConnections %x",tmpHead));
    openCon = TRUE;
    while(openTmpBlockedConnection());}

  TcpCache():accept(FALSE){
    currentHead         = NULL;
    currentTail         = NULL;
    closeHead           = NULL;
    closeTail           = NULL;
    myHead              = NULL;
    myTail              = NULL;
    tmpHead             = NULL;
    tmpTail             = NULL;
    probeHead           = NULL;
    probeTail           = NULL;
    current_size        = 0;
    close_size          = 0;
    open_size           = 0;
    max_size            =MAXTCPCACHE;
    openCon             = TRUE;
    probes              = FALSE;
    shutDwn             = FALSE;
    tmpTime             = 0;
    myTime              = 0;
    probeTime           = 0;
    PD((TCPCACHE,"max_size:%d",max_size));}

  void nowAccept(){
    Assert(accept==FALSE);
    accept=TRUE;}

  Bool Accept(){return accept;}
  Bool CanOpen(){return openCon;}

  void adjust(){
    if(myHead!=NULL && open_size<max_size){
      openMyClosedConnection(0);
      return;}
    if(open_size>max_size){
      //printf("Size too big %d\n",open_size);
      decreaseConnections();}
    if ((open_size + close_size) > (2 * max_size)){
      //printf("Size way too large, not accepting %d %d\n",open_size, close_size);
      accept = FALSE;}
    else
      accept = TRUE;}

  Bool canAddWrite(){return open_size < max_size;}

  void add(Connection *w) {
    /*    printf("add r:%d \n cur: %d %d \n close: %d %d \n my: %d %d \n tmp: %d %d \n probe: %d %d\n",(int)w,
           (int) currentHead,
           (int)currentTail     ,
           (int)closeHead,
           (int)closeTail,
           (int)myHead,
           (int)myTail,
           (int)tmpHead,
           (int)tmpTail,
           (int)probeHead,
           (int)probeTail
           );
    */
    WriteConnection *ww;
    if(w->isOpening()){
      addToFront(w, currentHead, currentTail);
      open_size++;
      adjust();
      return;}
    if(w->isClosing()){
      //printf("New Closing %d write:%d\n",w->getFD(), w->isWriteCon());
      addToFront(w, closeHead, closeTail);
      close_size++;
      adjust();
      return;}
    Assert(w->isWriteCon());
    ww = ((WriteConnection*)w);
    if(ww->isMyInitiative() ||ww->isHisInitiative()){
      newMyDwn();
      addToFront(w, myHead, myTail);
      return;}
    if(ww->isTmpDwn()){
      if(openCon) newTmpDwn();
      addToFront(w, tmpHead, tmpTail);
      return;}
    if(ww->isProbing()){
      newProbe();
      addToFront(w, probeHead, probeTail);
      /*printf("Adding %d to probe %d %d\n",(int)ww,(int)probeHead,
             (int)probeTail);
      */
      return;}
    OZ_warning("Unknown type of connection");
    Assert(0);}

  void remove(Connection *w){
    if(w->isOpening()){
      //fprintf(stderr,"rmO\n");
      unlink(w, currentHead, currentTail);
      open_size--;
      adjust();
      return;}
    if(w->isClosing()){
      //fprintf(stderr,"rmC\n");
      unlink(w, closeHead, closeTail);
      close_size--;
      adjust();
      return;}
    if(w->isWriteCon()){
      if(((WriteConnection*)w)->isMyInitiative()
         ||((WriteConnection*)w)->isHisInitiative()){
        //fprintf(stderr,"rmM\n");
        unlink(w, myHead, myTail);
        return;}
      if(((WriteConnection*)w)->isTmpDwn()){
        //fprintf(stderr,"rmT\n");
        unlink(w, tmpHead, tmpTail);
        return;}
      if(((WriteConnection*)w)->isProbing()){
        //fprintf(stderr,"rmP\n");
        unlink(w, probeHead, probeTail);
        //printf("Removing %d from probe %d %d \n",(int)w,(int)probeHead,(int)probeTail);
        return;}

    }
    //fprintf(stderr,"rmCCC\n");
    unlink(w, currentHead, currentTail);
    open_size--;
    adjust();}


  void touch(Connection *r) {
  if(currentHead!=r){
      unlink(r, currentHead, currentTail);
      addToFront(r,currentHead, currentTail);}
    adjust();}

  Bool openTmpBlockedConnection();
  Bool openMyClosedConnection(unsigned long time);
  Bool startProbe();

  void wakeUp(unsigned long time);
  Bool checkWakeUp(unsigned long time);
};


/************************************************************/
/* SECTION 12a:  WakeUps of the TCP-cache                   */
/************************************************************/

void  TcpCache::wakeUp(unsigned long time){
  if (myTime && myTime<time)
    if( openMyClosedConnection(time))
      myTime += WKUPMYC_TIME;
    else {
      //printf("Nothingmore to check my\n");
      myTime = 0;}
  if (tmpTime && tmpTime<time)
    if(openTmpBlockedConnection())
      tmpTime += WKUPTMP_TIME;
    else
      tmpTime = 0;
  if (probeTime && probeTime<time){
    //printf("StartingProbes\n");
    if(startProbe())
      probeTime += WKUPPRB_TIME;
    else
      probeTime = 0;}
  if(!(probeTime|tmpTime|myTime)){
    //printf("Settingmin time to 0\n");
    am.setMinimalTaskInterval((void*)this,0);}
}

Bool TcpCache::checkWakeUp(unsigned long time){
  curTime = time;
  return ((myTime||probeTime||tmpTime)&&
          (myTime<time || tmpTime < time || probeTime< time));}

Bool wakeUpTcpCache(unsigned long time, void *v){
  tcpCache->wakeUp(time);
  return TRUE;}

Bool checkTcpCache(unsigned long time, void *v){
  return tcpCache->checkWakeUp(time);}

Bool TcpCache::openTmpBlockedConnection(){
  PD((TCPCACHE,"OpeningTmps %x %x",tmpHead, tmpTail));
  if(tmpHead!=NULL){
    WriteConnection *w = ((WriteConnection *) getLast(tmpHead, tmpTail));
    /*
      printf("Opening con closed by him %s\n",
           w->remoteSite->site->stringrep());
    */
    w->clearTmpDwn();
    w->open();}
  return tmpHead!=NULL;}

Bool TcpCache::openMyClosedConnection(unsigned long time){
  /*  if(time)
      printf("OpeningMys %d %d time:%d\n",(int)myHead, (int)myTail, (int) time);
  */
  WriteConnection *w = (WriteConnection *)myHead;
  while(time && w!= NULL){
    if(w->expireTime == 0)
      w->expireTime = time + CLOSE_EXPIRETIME;
    if(w->expireTime < time ){
      remove(w);
      if(w->isHisInitiative())
        w->clearHisInitiative();
      else
        w->clearMyInitiative();
      w->setTmpDwn();
      add(w);
      w->remoteSite->site->probeFault(PROBE_TEMP);}
    w = (WriteConnection*) w->next;}
  w = (WriteConnection *)myTail;
  while(w!=NULL && time==0 && w->isHisInitiative())
    w = (WriteConnection*) w->prev;
  if(w!=NULL){
    //fprintf(stderr,"Opening %d %d\n", (int)w, (int)w->isWriteCon());
    remove(w);
    if(w->isHisInitiative())
      w->clearHisInitiative();
    else
      w->clearMyInitiative();
    w->open();}
  return myHead!=NULL;}

Bool TcpCache::startProbe(){
  if(probeHead!=NULL){
    WriteConnection *w = ((WriteConnection *) getLast(probeHead, probeTail));
    /*printf("Starting from probe r:%d %d %d\n",(int)w,(int)probeHead,
           (int)probeTail);
    */
    w->clearProbing();
    if(canAddWrite())
      w->open();
    else{
      w->setMyInitiative();
      add(w); }
  }
  return probeHead!=NULL;
}



/************************************************************/
/* SECTION 12b:  Exported to Perdio                           */
/************************************************************/

int openclose(int Type){
  int state = 0;
  if(tcpCache->openCon) state = 1;
  if(Type){
    if(state) (void) tcpCache->closeConnections();
    else tcpCache->openConnections();}
  return state;}


/***********************************************************/
/* SECTION 13: ?????                                           */
/************************************************************/

void Connection::setClosing(){
  PD((TCP_INTERFACE,"setClosing site:%s",remoteSite->site->stringrep()));
  Assert(!testFlag(CLOSING));
  tcpCache->remove(this);
  setFlag(CLOSING);
  tcpCache->add(this);}

Bool WriteConnection::shouldSendFromUser(){
  if(isClosing()){PD((TCP_INTERFACE,"isClosing")); return FALSE;}
  if(isOpening()) {PD((TCP_INTERFACE,"isOpening")); return FALSE;}
  if(isIncomplete()) {PD((TCP_INTERFACE,"isIncompleteWrite")); return FALSE;}
  if(isInWriteQueue()) {PD((TCP_INTERFACE,"isInWriteQueue")); return FALSE;}
  if(isMyInitiative()) {PD((TCP_INTERFACE,"isMyInitiative")); return FALSE;}
  if(isHisInitiative()) {PD((TCP_INTERFACE,"isProbing")); return FALSE;}
  if(isProbing()) {
    //printf("Sending on probing con %d\n", (int) this);
    tcpCache->remove(this);
    clearProbing();
    setMyInitiative();
    tcpCache->add(this);
    return FALSE;}
  Assert(fd!=LOST);
  return TRUE;}

ipReturn WriteConnection::open(){
  PD((TCP_INTERFACE,"OpenConnection"));
  setOpening();
  tcpCache->add(this);
  return tcpOpen(remoteSite, this);}
/**********************************************************************/
/*   SECTION 14: MessageManagers                                           */
/**********************************************************************/

#define RECYCLE2  (NetMsgBuffer*) 0x1

class MessageManager: public FreeListManager {
private:
  int wc;
  int Msgs;

  Message * newMessage(){
    FreeListEntry *f=getOne();
    Message *m;
    if(f==NULL) {m=new Message();}
    else {GenCast(f,FreeListEntry*,m,Message*);}
    wc++;
    return m;}

  void deleteMessage(Message *m){
    FreeListEntry *f;
    DebugCode(m->bs = RECYCLE2;)
    GenCast(m,Message*,f,FreeListEntry*);
    if(!putOne(f)) {delete m;}
    wc--;
    return;}

public:
  MessageManager():FreeListManager(MESSAGE_CUTOFF){Msgs = 0; wc = 0;};

  Message * allocMessage(NetMsgBuffer *bs, int msgNum,
                         DSite *s, MessageType b, int i){
    Message*m = newMessage();
    PD((MESSAGE,"allocate  nr:%d", ++Msgs));
    m->init(bs,msgNum,s,b,i);
    return m;}

  Message * allocMessage(NetMsgBuffer *bs,tcpMessageType t,int rem){
    Message*m = newMessage();
    PD((MESSAGE,"allocate DANGER r:%x nr:%d", ++Msgs));
    m->init(bs,0,NULL,M_LAST,0);
    m->setRemainder(rem);
    m->setType(t);
    return m;}

  void freeMessage(Message *m){
    Assert(m->bs!=RECYCLE2);
    PD((MESSAGE,"freed nr: %d", --Msgs));
    deleteMessage(m);}

  void freeMessageAndMsgBuffer(Message *m){
    Assert(m->bs!=RECYCLE2);
    netMsgBufferManager->dumpNetMsgBuffer(m->bs);
    PD((MESSAGE,"BM freed nr:%d", --Msgs));
    deleteMessage(m);}

  int getCTR(){return wc;}
};


/**********************************************************************/
/*   SECTION 15: Methods                                            */
/**********************************************************************/


/********************************************************

      RemoteSite DBG

********************************************************/

Bool WriteConnection::checkAckQueue(){
  Message *m = sentMsg;
  if (m == NULL ) return FALSE;
  while (m != NULL){
    PD((ACK_QUEUE,"Ack queue ptr: %d nr: %d ctr: %d",
        m,m->msgNum,sentMsgCtr));
    m=m->next;}
  return TRUE;
}

/************************************************************/
/*  SECTION 16: Storing messages till ack received           */
/************************************************************/

void RemoteSite::storeSentMessage(Message* m) {
  PD((ACK_QUEUE,"Adding to ack queue  %d",m));
  Assert(writeConnection != NULL);
  writeConnection->storeSentMessage(m);}

void WriteConnection::storeSentMessage(Message* m) {
  m->next = sentMsg;
  sentMsg = m;
  PD((ACK_QUEUE,"*** QUEUE CHECKED %d***",checkAckQueue()));}


/* *****************************************************************
            MsgBuffer operations
   *******************************************************************/

/* read       pos= first BYTE    endpos = first free slot or NULL
   interpret  pos= first BYTE curpos= last BYTE
                              endpos= first free slot or NULL */

BYTE* NetMsgBuffer::initForRead(int &len){
  Assert(type==BS_None);
  PD((BUFFER,"NetByteBuffer alloc Read: %d",no_bufs()));
  pos=first->head();
  endpos=first->head();
  len=NetByteBuffer_SIZE;
  type=BS_Read;
  return endpos;}

BYTE* NetMsgBuffer::beginRead(int &len){
  Assert(type==BS_None);
  type=BS_Read;
  if(endpos==NULL){
    if(pos==NULL){
      pos=first->head();
      endpos=first->head();
      len=NetByteBuffer_SIZE;
      return endpos;}
    Assert(within(pos,first));
    PD((BUFFER,"NetByteBuffer alloc Read: %d",no_bufs()));
    NetByteBuffer *bb=netByteBufferManager->newNetByteBuffer();
    last->next=bb;
    last=bb;
    len=NetByteBuffer_SIZE;
    endpos=last->head();
    return endpos;}
  if(pos==NULL){pos=endpos;}
  else{Assert(within(pos,first));}
  len=last->tail()-endpos+1;
  Assert(len<NetByteBuffer_SIZE);
  return endpos;}

void NetMsgBuffer::afterRead(int len){
  Assert(type==BS_Read);
  Assert(len<=NetByteBuffer_SIZE);
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
    PD((BUFFER,"NetByteBuffer dumped read (SPEC):%d",no_bufs()));
    netByteBufferManager->deleteNetByteBuffer(first);
    first=last;
    pos=first->head();
    return ch;}
  return *pos++;}

int NetMsgBuffer::interLen(){
  if(first==last){return curpos-pos+1+tcpHeaderSize;}
  int i=first->tail()-pos+1+tcpHeaderSize;
  NetByteBuffer *bb=first->next;
  while(bb!=last){bb=bb->next;i+=NetByteBuffer_SIZE;}
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


void NetMsgBuffer::beginWrite(RemoteSite *s){
  Assert(type==BS_None);
  Assert(first!=NULL);
  remotesite =  s;
  BYTE* thispos= first->head();
  *thispos++=TCP_PACKET;
  type=BS_Write;
  int2net(thispos,totlen);}

void NetMsgBuffer::resend(){
  Assert(type == BS_Write);
  reset();
  pos = first->head();}

void NetMsgBuffer::reset(){
  Assert(start!=NULL && stop != NULL);
  first= start;
  last = stop;}

void NetMsgBuffer::unmarshalReset(){
  reset();
  pos = first->head() + tcpHeaderSize ;
  if(endpos==NULL) curpos=last->tail();
  else curpos= endpos-1;
}

void NetMsgBuffer::PiggyBack(Message* m)
{
  int msgCtr = remotesite->getWriteConnection()->incMsgCtr();
  BYTE* thispos= first->head() + 5;
  int2net(thispos, msgCtr);
  thispos += 4;
  int2net(thispos, remotesite->getRecMsgCtr());
  m->setMsgNum(msgCtr);
  remotesite->storeSentMessage(m);
  PD((ACK_QUEUE,"¤¤¤¤¤ Message nr: %d Ack nr: %d inserted ¤¤¤¤¤¤¤¤"
      ,msgCtr, remotesite->getRecMsgCtr()));
}

void NetMsgBuffer::constructMsg(RemoteSite* rs, tcpMessageType msg){
  type = BS_Write;
  first=getAnother();
  start=first;
  last=first;
  stop=first;
  remotesite = rs;
  pos=first->head();
  *pos=msg;
  pos++;
  int2net(pos,tcpHeaderSize);
  pos+=4;
  int2net(pos,remotesite->getWriteConnection()->getMsgCtr());

  pos+=4;
  int2net(pos,remotesite->getRecMsgCtr());
  pos+=4;
  endpos=pos;
  pos=first->head();
}


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

  char *nodename = oslocalhostname();
  if(nodename==0) {NETWORK_ERROR(("createTcpPort"));}
  struct hostent *hostaddr;
  hostaddr=gethostbyname(nodename);
  free(nodename);
  if (hostaddr==NULL) {
    nodename = "localhost";
    hostaddr=gethostbyname(nodename);
    OZ_warning("Unable to reach the net, using localhost instead\n");
  }


  struct in_addr tmp;
  memcpy(&tmp,hostaddr->h_addr_list[0],sizeof(in_addr));
  ip=ntohl(tmp.s_addr);
 retry:
  if(bigtries<0){
    PD((WEIRD,"bind - ran out of tries"));
    return IP_TIMER;} // ATTENTION - major fault
  fd=ossocket(PF_INET,SOCK_STREAM,0);
  if (fd < 0) {
    if(ossockerrno()==ENOBUFS) return IP_TIMER;
    if (ossockerrno() == EINTR) {
      bigtries--;
      goto retry;
    }
    NETWORK_ERROR(("system:socket %d\n",ossockerrno()));}
  addr.sin_port = htons(port);

  if(bind(fd,(sockaddr *) &addr,sizeof(struct sockaddr_in))<0) {
    PD((WEIRD,"create TcpPort bind redo port:%d ossockerrno():%d",
        port,ossockerrno()));
    osclose(fd);
    smalltries--;
    if(createTcpPort_RETRY() && smalltries>0) goto retry;
    bigtries--;
    port++;
    smalltries=OZConnectTries;
    goto retry;}
  PD((TCP,"createTcpPort: got ip:%u port:%u",ip,port));
  if (listen(fd,5)<0) {NETWORK_ERROR(("listen %d\n",ossockerrno()));}

  struct sockaddr_in addr1;
#if __GLIBC__ == 2
  unsigned int length = sizeof(addr1);
#else
  int length = sizeof(addr1);
#endif
  if (getsockname(fd, (struct sockaddr *) &addr1, &length) < 0) {
    NETWORK_ERROR(("getsockname %d\n",ossockerrno()));
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
    MsgBuffer::init();
    pos = NULL;}
  void marshalBegin(){
    posMB = buffer + 5;
    endMB = buffer + TCPOPENMSGBUFFER_SIZE;
    size = 0;}
  void marshalEnd(){ size = posMB - buffer  - 5;}
  void unmarshalMarshal(){}
  void beginWrite(BYTE b){
    pos = buffer;
    *pos++=b;
    int2net(pos,size);
    size += 5;
    pos = NULL;}
  void unmarshalBegin(){
    posMB = buffer;
    endMB = buffer + TCPOPENMSGBUFFER_SIZE;}
  void unmarshalEnd(){
    pos = NULL;}
  DSite *getSite(){
    Assert(0);
    return NULL;}
  Bool isPersistentBuffer() { return NO; }
  int getLen(){
    return size;}
  BYTE* getBuf(){
    return buffer;}
  char* siteStringrep(){
    Assert(0);
    return "Assert(0)";}
  BYTE readyForRead(int &len){
    BYTE b=get();
    len = net2int(posMB);
    posMB += 4;
    return b;}
  BYTE getNext(){
    Assert(0);
    return 0;}
  void putNext(BYTE b){
    Assert(0);}
};

#define USE_OLD_PORT -1

/*  *****************************************************************
          tcpSend
   ***************************************************************** */

ipReturn tcpSend(int fd,Message *m, Bool flag)
{
  int total,len,ret;
  NetMsgBuffer *bs = m->getMsgBuffer();
  if(flag){
    total=bs->calcTotLen();
    PD((TCP,"tcpSend invoked rem:%d",total));}
  else{
    total=bs->getTotLen();
    bs->PiggyBack(m);
#ifdef PERDIOLOGLOW
    printf("#!!!sendingnet(to:'%s' type:%d site:'%s' index:%d size:%d nr:%d)#&&&\n",
           bs->getSite()->stringrep(),
           m->getMsgType(),m->getSite()->stringrep(),
           m->getMsgIndex(),total, m->getMsgNum());

#endif

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
      if(ossockerrno()==EINTR) continue;
      if(!((ossockerrno()==EWOULDBLOCK) || (ossockerrno()==EAGAIN)))
        return tcpError();
      break;}
    PD((WRITE,"wr:%d try:%d error:%d",ret,len,ossockerrno()));
#ifdef SLOWNET
    TSC->writing(ret);
#endif
    if(ret<len){
      if(ret>0){
        bs->incPosAfterWrite(ret);
        PD((TCP,"tcpSend blocked wrote %d of %d",ret,len));}
      return IP_BLOCK;}
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
    if(ret == 0) return IP_CLOSE;
    if(ret<0){

#ifdef LINUX
      // EK
      // Linux returns a very strange errno when
      // connections are unaviable.
      // This is a hack to cover that case.
      // The strange number is 111
      if(ossockerrno() == 111)
        return IP_PERM_BLOCK;
#endif
      if(ossockerrno() == ECONNREFUSED ||
         ossockerrno() == EADDRNOTAVAIL)
        return IP_PERM_BLOCK;
      if(ossockerrno()!=EINTR) return IP_BLOCK;
      tries--;
      if(tries<0) {return IP_NO_MORE_TRIES;}
      pos += ret;
      todo -= ret;}}}

#define CONNECTION_HANDLER_TRIES 50


inline ipReturn writeI(int fd,BYTE *buf)
{
  int ret;
  while(TRUE){
    ret = oswrite(fd,buf,1);
    if(ret>0){
      Assert(ret==1);
      return IP_OK;}
    if (ossockerrno() == EINTR){
      PD((WEIRD,"write interrupted"));}
    else {
      if((ossockerrno()==EWOULDBLOCK) || (ossockerrno()==EAGAIN)) {
        return IP_BLOCK;}
      NETWORK_ERROR(("writeI %d\n",ossockerrno()));}}}

inline ipReturn readI(int fd,BYTE *buf)
{
  int ret;
  while(TRUE){
    ret = osread(fd,buf,1);
    if(ret>0){
      Assert(ret==1);
      return IP_OK;
    }

    if (ret==0) {
      PD((TCP,"Connection closed/lost %d",fd));
      return IP_EOF;
    }

    if (ossockerrno() == EINTR){
      PD((WEIRD,"readI interrupted"));
    } else {
      if((ossockerrno()==EWOULDBLOCK) || (ossockerrno()==EAGAIN)) {
        return IP_BLOCK;
      }
      PD((WEIRD,"readI discoversCRASH errno:%d",ossockerrno()));
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


/************************************************************/
/* SECTION 17: Div small routines                              */
/************************************************************/

void uniquefyInt(BYTE *buf, int Num){
  PD((TCP,"Intfy %d",Num));
  int ctr = 0;
  BYTE b;
  while(ctr < 33){
    b = Num & 255;
    b <<=  TCP_MSG_SIGN_BITS;
    *buf++ = b;
    PD((TCP,"Writing to buffer %d lim:33 act:%d", b, ctr));
    Num >>= (8 - TCP_MSG_SIGN_BITS);
    ctr += 8 - TCP_MSG_SIGN_BITS;}}

int intifyUnique(BYTE *buf){
  int ctr = 0;
  unsigned int i;
  int ans = 0;
  while(ctr < 33){
    i = *buf++;
    i >>= TCP_MSG_SIGN_BITS;
    ans += i << ctr;
    PD((TCP,"Reading from buffer %d ans:%d lim:33 act:%d", i,ans,ctr));
    ctr += (8 - TCP_MSG_SIGN_BITS);}
  PD((TCP,"Ack read: %d", ans));
  return ans;}

/************************************************************/
/* SECTION 18: ReadStuff                                        */
/************************************************************/

Bool tcpAckReader(ReadConnection *r, int ack){
  BYTE buffer[INT_IN_BYTES_LEN + 1];
  BYTE *buf = buffer;
  int fd=r->getFD();
  Assert(fd!=LOST);
  ipReturn ret;
  PD((TCP,"tcpAckReader r:%x a:%d",r, ack));
  *buf=(BYTE) TCP_MSG_ACK_FROM_READER;
  uniquefyInt(buf + 1, ack);
  return ossafewrite(fd,(char*)buf,INT_IN_BYTES_LEN+1) < 0 ? FALSE : TRUE;
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
    if (ossockerrno() == EINTR){
      PD((WEIRD,"read interrupted"));
      no_tries--;
      continue;}
    if(ossockerrno()==EAGAIN || ossockerrno()==EWOULDBLOCK){
      PD((WEIRD,"read EAGAIN"));
      no_tries--;
      continue;}
    break;}
  return ret;
}

inline
ipReturn interpret(NetMsgBuffer *bs,tcpMessageType type, Bool ValidMsg)
{
  switch(type){
  case TCP_PACKET:{
    if (ValidMsg) {
      PD((TCP,"interpret-packet"));
      PD((TCP,"received TCP_PACKET"));
#ifdef SLOWNET
      int totalLen = bs->getslownetTotLen();
      NetMsgBuffer *bsNew=netMsgBufferManager->getNetMsgBuffer(bs->getSite());
      bsNew->setslownetTotLen(totalLen);
      bsNew->marshalBegin();
      bs->unmarshalBegin();
      while(totalLen--){
        BYTE bb = bs->get();
        bsNew->put(bb);}
      bsNew->marshalEnd();
      bsNew->beforeInterpret(0);
      bsNew->pos = bsNew->pos + 13;
      TSC->addRead(bsNew);
      bs->unmarshalEnd();
#else
      bs->unmarshalBegin();
      msgReceived(bs);
      bs->unmarshalEnd();
#endif
    }
    //else printf("ThrowingAway!!!\n");
    return IP_OK;}
  case TCP_CLOSE_REQUEST_FROM_WRITER:{
    PD((TCP,"interpret - close"));
    return IP_CLOSE;}
  case TCP_PING_REQUEST:{
    PD((TCP,"interpret ping"));
    return IP_OK; }
  default:{
    // kost@ : the message is very informative ;->
    OZ_warning("Something is very wrong");
    Assert(0);
    return IP_GARBAGE;}
  }
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
    r->addCurQueue(m);  return m;}
  else{
    PD((WEIRD,"oldReadCur r:%x",r));
    m->setType(type);
    m->setRemainder(rem);
    r->addCurQueue(m);  return m;}


}

/*   ***************************************************************** */


#define PREREAD_HANDLER_TRIES 50
#define PREREAD_NO_BYTES 5


/************************************************************/
/* SECTION 19: Handshake Readin Site                            */
/************************************************************/

int accHbufSize = 9 + strlen(PERDIOVERSION);
BYTE *accHbuf = new BYTE[accHbufSize];

static int acceptHandler(int fd,void *unused)
{
  PD((TCP_INTERFACE,"acceptHandler: %d",fd));
  struct sockaddr_in from;
  int fromlen = sizeof(from);
  int newFD=osaccept(fd,(struct sockaddr *) &from, &fromlen);

  if (newFD < 0) {
    return 0;}

  if (!tcpCache->Accept()|| !tcpCache->CanOpen()){
    PD((TCP_INTERFACE,"Connection Refused"));
    osclose(newFD);
    return 0;}

  ip_address ip=from.sin_addr.s_addr;
  port_t port=from.sin_port;
  Bool accept = tcpCache->Accept();

  BYTE *auxbuf = accHbuf;

  *auxbuf = TCP_CONNECTION;
  auxbuf++;
  int2net(auxbuf,myDSite->getTimeStamp()->start);
  auxbuf += netIntSize;
  int2net(auxbuf, accHbufSize - 9);
  auxbuf += netIntSize;
  for (char *pv = PERDIOVERSION; *pv;) {
    PD((TCP,"Writing %c %d\n",*pv,auxbuf));
    *auxbuf++ = *pv++;}

  Assert(auxbuf-accHbuf == accHbufSize);
  // EK!!
  // fcntl(newFD,F_SETFL,O_NONBLOCK);
#ifdef WINDOWS
  osSetNonBlocking(newFD,OK);
#else
  fcntl(newFD,F_SETFL,O_NDELAY);
#endif
  int written = 0;
  while(TRUE){
    int ret=oswrite(newFD,accHbuf+written,accHbufSize-written);
    written += ret;
    if(written==accHbufSize) break;
    if(ret<=0 && ossockerrno()!=EINTR && ossockerrno()!=EWOULDBLOCK && ossockerrno()!=EAGAIN ) {
      OZ_warning("Error in OPening, we're closing");
      osclose(newFD);
      return 0;}}

  ReadConnection *r=readConnectionManager->allocConnection(NULL,newFD);
  r->setOpening();
  tcpCache->add(r);
  PD((TCP_CONNECTIONH,"acceptHandler success r:%x",r));
  OZ_registerReadHandler(newFD,tcpPreReadHandler,(void *)r);
  return 0; }

int tcpPreReadHandler(int fd,void *r0){
  ReadConnection *r=(ReadConnection *)r0, *old;
  tcpOpenMsgBuffer->unmarshalBegin();
  BYTE *pos = tcpOpenMsgBuffer->getBuf(),header;
  int ackStartNr,maxNrSize,todo;
  DSite *si;
  PD((TCP_CONNECTIONH,"tcpPreReadHandler invoked r:%x",r));
  if(smallMustRead(fd,pos,PREREAD_NO_BYTES,PREREAD_HANDLER_TRIES))
    goto tcpPreFailure;
  pos +=5;
  header =  tcpOpenMsgBuffer->readyForRead(todo);

  if(header!=TCP_MYSITE && header!=TCP_MYSITE_ACK)
    goto tcpPreFailure;
  if(smallMustRead(fd,pos,todo,PREREAD_HANDLER_TRIES))
    goto tcpPreFailure;

  ackStartNr = unmarshalNumber(tcpOpenMsgBuffer);
  maxNrSize= unmarshalNumber(tcpOpenMsgBuffer);
  si = unmarshalDSite(tcpOpenMsgBuffer);
  if(si->getRemoteSite() == NULL)
    goto tcpPreFailure;


  old = si->getRemoteSite()->getReadConnection();
  if(old!=NULL){old->close();}

  si->getRemoteSite()->setReadConnection(r);

  tcpOpenMsgBuffer->unmarshalEnd();

  if(header==TCP_MYSITE_ACK) if(!r->resend()) goto tcpPreFailure;

  r->setMaxSizeAck(maxNrSize);
  r->clearOpening();

  OZ_registerReadHandler(fd,tcpReadHandler,(void *)r);
  return 0;

tcpPreFailure:
  if(fd!=LOST) osclose(fd);
  tcpCache->remove(r);
  readConnectionManager->freeConnection(r);
  return 1;
}

static int tcpReadHandler(int fd,void *r0)
{
  PD((TCP,"tcpReadHandler Invoked"));

  ReadConnection *r = (ReadConnection*) r0;
  int ret,rem,len,msgNr,ansNr;
  int totLen;
  ipReturn ip;
  tcpMessageType type = TCP_NONE;
  Message *m;
  NetMsgBuffer *bs;
  BYTE *pos;
  Assert(fd==r->getFD());
  Bool readAll;

  if(r->isClosing()){
    BYTE buf;
    ipReturn ret = readI(fd, &buf);
    while(ret==IP_OK){
      ret = readI(fd, &buf);}
    if(ret==IP_EOF){
      r->niceClose();
      r->close();}
    return 0;}


  if(r->isIncomplete()){
    m=r->getCurQueue();
    Assert(m!=NULL);
    bs=m->getMsgBuffer();
    rem=m->getRemainder();
    type=m->getType();
    PD((TCP,"readHandler incomplete r:%x rem:%d",r,rem));
    pos=bs->beginRead(len);
    if (rem >= 0){
      msgNr = r->remoteSite->readRecMsgCtr() + 1;
      // ATTENTION
      totLen = rem;
    }
  }
  else{
    m=NULL;
    //EK
    rem=0-tcpHeaderSize;
    bs=netMsgBufferManager->getNetMsgBuffer(r->remoteSite->site);
    PD((TCP,"readHandler from scratch r:%x",r));
    bs->getSingle();
    pos=bs->initForRead(len);}

start:

  Assert(osTestSelect(fd,SEL_READ));
  ret=tcpRead(fd,pos,len,readAll);
  if (ret<0) {

    if(r->isClosing()){
      PD((TCP,"tcpReadHandler ses Close"));
      netMsgBufferManager->dumpNetMsgBuffer(bs);
      r->niceClose();
      r->close();
      return 0;}

    if(m!=NULL){
      //fprintf(stderr,"dumping incomplete read\n");
      Assert(r->isIncomplete());
      messageManager->freeMessage(m);}

    netMsgBufferManager->dumpNetMsgBuffer(bs);
    if(tcpError() == IP_TEMP_BLOCK)
      r->close();
    else{
      r->connectionLost();}
    return 0;}

  PD((READ,"no:%d av:%d rem:%d",ret,len,rem));
  bs->afterRead(ret);

  while(TRUE){
    PD((READ,"WHILE no:%d av:%d rem:%d",ret,len,rem));
    if(rem<0){
      if(rem+ret>=0){
        type=getHeader(bs,len,msgNr,ansNr);
        r->remoteSite->receivedNewAck(ansNr);
        totLen = len;
#ifdef SLOWNET
        bs->setslownetTotLen(totLen - tcpHeaderSize);
#endif
        PD((READ,"Header done no:%d av:%d rem:%d tcp:%d",ret,len,rem,tcpHeaderSize));
        rem=len-ret-tcpHeaderSize-rem;}
      else{
        rem +=ret;
        goto maybe_redo;}}
    else{
      rem -=ret;}
    if(rem>0){goto maybe_redo;}

    /* Must mark the readConnection. It is possible
       to discover that the site has crashed during interpret.
       EK
       */
    r->setReading();

    PD((CONTENTS," Informin: %d %d %d ",msgNr,ansNr,totLen));

    /***************************************************/
    /* ATTENTION                                       */
    /* EK                                              */
    /* For concistency, When to inform the remoteSite  */
    /* About the received Message.                     */
    /***************************************************/
    bs->beforeInterpret(rem);
    PD((CONTENTS,"interpret rem:%d len:%d",
                 rem,bs->interLen()));
#ifdef PERDIOLOGLOW
    printf("#!!!readingnet(sender:'%s' type:%d size:%d nr:%d)#&&&\n",
           r->remoteSite->site->stringrep(),
           type,totLen, msgNr);
#endif
    // EK this might be done in a nicer way...
    ip=interpret(bs,type,r->informSiteAck(msgNr,totLen));

    if(r->isCrashed()){
      r->clearReading();
      r->remoteSite->sitePrmDwn();
      return 0;}

    if(ip==IP_CLOSE){
      Assert(rem==0);
      if(m!=NULL){
        Assert(r->isIncomplete());
        messageManager->freeMessage(m);}
      netMsgBufferManager->dumpNetMsgBuffer(bs);
      goto close;}
    bs->afterInterpret();
    if(rem==0){goto fin;}
    ret=0-rem;
    rem=0-tcpHeaderSize;
  }

maybe_redo:
  if((!r->isCrashed()) && readAll && osTestSelect(fd,SEL_READ)){
    if(rem==0) {pos=bs->initForRead(len);}
    else {pos=bs->beginRead(len);}
    goto start;}

fin:
  // It is possible to discover that the site is gone
  // during interpret. If that has happened, take care of
  // it here.
  // Else clear reading.
  r->clearReading();
  if(r->isCrashed()){
    r->remoteSite->sitePrmDwn();
    return 0;}


  if(!r->isClosing())
    tcpCache->touch(r);
  if(rem==0){
    if(m==NULL){
      // EK check this out...
      // ATTENTION
      // Here we must dump all the NetByteBuffers, they should
      // have been removed dynamicly during marshaling.
      // ATTENTION
      netMsgBufferManager->dumpNetMsgBuffer(bs);
#ifdef SLOWNET
      // r->setTime(READ_NO_TIME);
#endif
      return 0;}
    Assert(!r->isIncomplete());
    messageManager->freeMessageAndMsgBuffer(m);
    return 0;}
  newReadCur(m,bs,r,type,rem);
  return 0;

close:
  if(r->isCrashed()){
    r->connectionLost();
    return 0;}
  r->clearReading();
  PD((TCP,"readHandler received close"));
  if(rem!=0) {NETWORK_ERROR(("readHandler gets bytes after Close"));}
  switch(type){
  case TCP_CLOSE_REQUEST_FROM_WRITER:{
    if(r->isClosing())
      return 0;
    r->setClosing();
    if(!tcpAckReader(r, r->remoteSite->getRecMsgCtr())){
      OZ_warning("During close, acks faild to be sent");
      OZ_warning("Inform erik");
      Assert(0);}

    BYTE msg=TCP_CLOSE_ACK_FROM_READER;
    while(TRUE){
      int ret=oswrite(fd,&msg,1);
      if(ret==1) break;
      if(ret<0){
        if((ret!=EINTR) && (ret!=EWOULDBLOCK) && (ret!=EAGAIN)){
          r->connectionLost();
          return 0;}}}
    return 0;}
  default:{
    Assert(0);
    return 0;}
  }
}



/************************************************************/
/* SECTION 20: Handshake Writing Site                           */
/************************************************************/

static ipReturn tcpOpen(RemoteSite *remoteSite,WriteConnection *r)
{
  port_t aport;

  aport=remoteSite->getPort();
  PD((TCP,"open s:%s",remoteSite->site->stringrep()));

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr=htonl(remoteSite->getAddress());
  addr.sin_port = htons(aport);

  int tries=OZConnectTries;
  int fd  = -1;
  int one = 1;

  if(!tcpCache->CanOpen()){goto  ipOpenNoAnswer;}

retry:
  PD((TCP,"Opening connection t:%d",tries));
  fd=ossocket(PF_INET,SOCK_STREAM,0);
  if (fd < 0) {
    if(ossockerrno()==ENOBUFS){goto  ipOpenNoAnswer;}
    //fprintf(stderr,"fd < 0 = %d  - interpreted as perm:%d \n",fd,ossockerrno());
        r->connectionLost();
    return IP_PERM_BLOCK;}

  if(setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char*) &one,sizeof(one))<0){
    goto  ipOpenNoAnswer;}

  // EK!!
#ifndef WINDOWS
  fcntl(fd,F_SETFL,O_NDELAY);
#endif
  if(osconnect(fd,(struct sockaddr *) &addr,sizeof(addr))==0
     || ossockerrno()==EINPROGRESS) {

#ifdef WINDOWS
    // do this later on Windows otherwise it doesn't work
    osSetNonBlocking(fd,OK);
#endif
    PD((TCP,"open success p:%d fd:%d",aport,fd));
    r->setFD(fd);
    OZ_registerReadHandler(fd,tcpConnectionHandler,(void *)r);
    PD((OS,"register READ %d - tcpConnectionHandler",fd));
    return IP_OK;}

  tries--;
  if(tries<=0){goto  ipOpenNoAnswer;}
  if((ossockerrno() == EADDRNOTAVAIL) || (ossockerrno() == ECONNREFUSED)){
    //fprintf(stderr,"cannot open - interpreted as perm:%d %d \n",EADDRNOTAVAIL,ossockerrno());
    r->connectionLost();
    return IP_PERM_BLOCK;}
  addr.sin_port = htons(aport);
  goto retry;

 ipOpenNoAnswer:
  tcpCache->remove(r);
  r->clearOpening();
  if(fd >= 0) osclose(fd);
  r->setHisInitiative();
  tcpCache->add(r);
  return IP_OK;
}


int tcpConnectionHandler(int fd,void *r0){
  WriteConnection *r=(WriteConnection *)r0;
  int ret,bufSize = 9, tcheck,todo=bufSize, written;
  BYTE *buf1 = new BYTE[bufSize], *pos, *buf2=NULL;
  unsigned int strngLen;
  time_t timestamp;
  char msgType;

  pos = buf1;

  PD((TCP,"tcpConnectionHandler invoked r:%x",r));
  ret = smallMustRead(fd,pos,bufSize,CONNECTION_HANDLER_TRIES);
  if(ret == IP_PERM_BLOCK) goto tcpConPermLost;
  if(ret == IP_CLOSE) {goto tcpConClosed;}
  if(ret){goto tcpConFailure;}

  pos = buf1;
  if(*pos!=TCP_CONNECTION)
    {goto tcpConFailure;}

  pos++;
  timestamp=net2int(pos);
  tcheck=r->remoteSite->site->checkTimeStamp(timestamp);
  if(tcheck!=0) goto tcpConPermLost;

  pos += netIntSize;
  strngLen = net2int(pos);


  buf2 = new BYTE[strngLen + 1];
  pos = buf2;

  ret=smallMustRead(fd,pos,strngLen,CONNECTION_HANDLER_TRIES);
  if(ret == IP_PERM_BLOCK) goto tcpConPermLost;
  if(ret){goto tcpConFailure;}


  if (strlen(PERDIOVERSION)!=strngLen ||
      strncmp(PERDIOVERSION,(char*)pos,strngLen)!=0) goto tcpConPermLost;

  PD((TCP,"Sending My Site Message..%s",myDSite->stringrep()));
  if(r->isAcked()) msgType = TCP_MYSITE;
  else msgType = TCP_MYSITE_ACK;

  tcpOpenMsgBuffer->marshalBegin();
  marshalNumber(r->getMsgCtr(), tcpOpenMsgBuffer);
  marshalNumber(mySiteInfo.maxSizeAck, tcpOpenMsgBuffer);
  myDSite->marshalDSite(tcpOpenMsgBuffer);
  tcpOpenMsgBuffer->marshalEnd();
  tcpOpenMsgBuffer->beginWrite(msgType);

  {
    bufSize = tcpOpenMsgBuffer->getLen();
    BYTE *otherbuf=tcpOpenMsgBuffer->getBuf();
    written = 0;

    while(TRUE){
      ret=oswrite(fd,otherbuf+written,bufSize-written);
      written += ret;
      if(written==bufSize) break;
      if(ret<=0 && ret!=EINTR && ossockerrno()!=EWOULDBLOCK && ossockerrno()!=EAGAIN )
        goto tcpConClosed;}
  }

  PD((OS,"unregister READ fd:%d",fd));
  // fcntl(fd,F_SETFL,O_NONBLOCK);
  // fcntl(fd,F_SETFL,O_NDELAY);
  OZ_registerReadHandler(fd,tcpCloseHandler,(void *)r);
  PD((OS,"register READ - close fd:%d",fd));

  if(r->isWritePending())
#ifndef SLOWNET
    OZ_registerWriteHandler(fd,tcpWriteHandler,(void *)r);
#else
  ;
#endif
  PD((TCP,"tcpConnectionHandler ord fin"));
  r->opened();
  delete buf1;
  if(buf2)delete buf2;
  if(r->isCanClose()){
    r->clearCanClose();
    r->closeConnection();}
  return 0;

 tcpConClosed:
  osclose(fd);
  delete buf1;
  if(buf2)delete buf2;
  r->close(HIS_INITIATIVE);
  return 1;

 tcpConFailure:
  osclose(fd);
  delete buf1;
  if(buf2)delete buf2;
  r->connectionBlocked();
  return 1;

tcpConPermLost:
  osclose(fd);
  delete buf1;
  if(buf2)delete buf2;
  r->connectionLost();
  return 1;
}



/************************************************************/
/* SECTION 21:  WriteHandlers                                    */
/************************************************************/

int tcpWriteHandler(int fd,void *r0){
  WriteConnection *r=(WriteConnection *)r0;
  Message *m;
  ipReturn ret;
#ifdef SLOWNET
  Bool hasSent = FALSE;
  if(r->isOpening()){
    TSC->reAddWrite();
    return 0;}
#endif
  RemoteSite* site = r->remoteSite;

  PD((TCP,"tcpWriteHandler invoked r:%x",r));
  if(r->isIncomplete()){
    m=r->getCurQueue();
#ifdef SLOWNET
    hasSent = TRUE;
#endif
    Assert(m!=NULL);
    Assert(r->getFD()==fd);
    ret=tcpSend(r->getFD(),m,TRUE);
    if(ret<0){
      goto writeHerrorBlock;
    }
    r->remoteSite->deQueueMessage(m->getMsgBuffer()->getTotLen());
    r->remoteSite->incNOSM();
  }

  if(r->isClosing()){
    messageManager->freeMessageAndMsgBuffer(m);
    return 1;}

  if(r->isWantsToClose()){
    r->closeConnection();
    return 0;}



  while(r->isInWriteQueue()){
#ifdef SLOWNET
    if(hasSent) break;
    hasSent = TRUE;
#endif
    PD((TCP,"taking from write queue %x",r));
    m=site->getWriteQueue();
    Assert(m!=NULL);
    ret=tcpSend(r->getFD(),m,FALSE);
    if(ret<0) goto writeHerrorBlock;
    r->remoteSite->deQueueMessage(m->getMsgBuffer()->getTotLen());
    r->remoteSite->incNOSM();
  }


  PD((TCP,"tcpWriteHandler finished r:%x",r));
#ifndef SLOWNET
  OZ_unregisterWrite(fd);
#endif
  PD((OS,"unregister WRITE fd:%d",fd));
  if(r->isCanClose()){
    r->clearCanClose();
    r->closeConnection();}
  else
    tcpCache->touch(r);
  return 0;

writeHerrorBlock:
  switch(ret){
  case IP_BLOCK:{
#ifdef SLOWNET
    TSC->reAddWrite();
#endif
    PD((WEIRD,"incomplete write %x",r));
    r->addCurQueue(m);
    tcpCache->touch(r);
    break;}
  case IP_PERM_BLOCK:{
    PD((WEIRD,"Site lost in maybeWrite %x",r));
    r->connectionLost();
    break;}
  default:{
    OZ_warning("Tmp block during send, not handled...");
    break;}}
  return 0;}



static int tcpCloseHandler(int fd,void *r0){
  WriteConnection *r=(WriteConnection *)r0;
  BYTE msg;
  ipReturn ret;
  PD((TCP,"tcpCloseHandler invoked r:%x",r));
  ret=readI(fd,&msg);
  PD((TCP,"tcpCloseHandler read b:%d r:%d e:%d",msg, ret,ossockerrno()));
close_handler_read:
  if(ret!=IP_OK){ // crashed Connection site
    PD((ERROR_DET,"crashed Connection site %s error:%d",r->remoteSite->site->stringrep(), ossockerrno()));
    if(ret==IP_EOF || tcpError()!=IP_TEMP_BLOCK)
      r->connectionLost();
    else
      r->connectionBlocked();
    return 0;}
  if(msg==TCP_MSG_ACK_FROM_READER){
    PD((TCP,"Incomming Ack"));
    r->incommingAck();
    ret=readI(fd,&msg);
    if(ret!=IP_BLOCK)
      goto close_handler_read;
    return 0;}
  if(msg==TCP_CLOSE_REQUEST_FROM_READER){
    PD((TCP,"ClsoeReq from reader"));
    if(!r->isClosing())
      r->setClosing();
    r->remoteSite->siteTmpDwn(HIS_INITIATIVE);
    return 0;}
  if(msg==TCP_CLOSE_ACK_FROM_READER){
    PD((TCP,"CloseAck form reader"));
    r->close(MY_INITIATIVE);
    return 0;}
  if(msg==TCP_RESEND_MESSAGES){
    PD((TCP,"Resend from reader"));
    r->informSiteResendAckQueue();}
  if(r->addByteToInt(msg)){
    ret=readI(fd,&msg);
    if(ret!=IP_BLOCK)
      goto close_handler_read;}
  PD((TCP,"Close Handler done..."));
  return 0;
}

/************************************************************/
/* SECTION 22:  Ack-stuff                                        */
/************************************************************/

Bool ReadConnection::informSiteAck(int m, int s){
  remoteSite->incNORM();
  if(!isClosing() && remoteSite->receivedNewMsg(m)){
    receivedNewSize(s);
    return TRUE;}
  return FALSE;}

void WriteConnection::informSiteResendAckQueue(){
  int resend = remoteSite->resendAckQueue(sentMsg);
  sentMsgCtr -=  resend;
  if(resend && !isInWriteQueue())
    setInWriteQueue();
  if(isInWriteQueue())
    OZ_registerWriteHandler(getFD(),tcpWriteHandler,(void *)this);
  sentMsg = NULL;}

int RemoteSite::resendAckQueue(Message  *ptr){
  Message *m = ptr;
  int nr = 0;
  while(ptr!=NULL){
    nr++;
    m = ptr;
    ptr = ptr->next;
    m->resend();
    writeQueue.addfirst(m);}
  return nr;}

Bool ReadConnection::resend(){
  PD((TCP_INTERFACE,"Resend messages"));
  int fd=getFD();
  Assert(fd!=LOST);
  BYTE msg = TCP_RESEND_MESSAGES;
  if(tcpAckReader(this,remoteSite->getRecMsgCtr()))
    return (writeI(fd,&msg) == IP_OK);
  return FALSE;}

void WriteConnection::ackReceived(int nr){
    int ctr = sentMsgCtr;
    Message *ptr = sentMsg;
    Message *old;
    if(ctr >= nr && ptr!=NULL){
      if(ctr == nr)
        sentMsg = NULL;
      else{
        while(ctr > nr){
          PD((ACK_QUEUE,"Stepping through ackQueue cur:%d stop%d",ctr, nr));
          old = ptr;
          ptr = ptr->next;
          if(ptr == NULL) break;
          ctr = ptr -> msgNum;}
        old->next = NULL;}
      while(ptr!=NULL) {
            old = ptr;
            ptr=ptr->next;
            // EK I dont know about this resend....
            messageManager->freeMessageAndMsgBuffer(old);}}}

Bool RemoteSite::receivedNewMsg(int Nr){
  PD((ACK_QUEUE,"MsgnumReceived new:%d old:%d",Nr,recMsgCtr));
  if (Nr  == recMsgCtr + 1){
    PD((ACK_QUEUE,"Message nr %d accepted",Nr));
    recMsgCtr = Nr;}
  else if(Nr > recMsgCtr){
    PD((ACK_QUEUE,"Message nr %d too large, old %d", Nr, recMsgCtr));
    readConnection->resend();
    return FALSE;}
  return TRUE;}

void RemoteSite::receivedNewAck(int a){
  if(writeConnection!= NULL) {
    PD((ACK_QUEUE,"Ack received: %d waiting for: %d",
        a,writeConnection->getMsgCtr()));
    writeConnection->ackReceived(a);}}

int RemoteSite::getRecMsgCtr(){
  PD((TCP_INTERFACE,"MsgnumGet %d",recMsgCtr));
  if(readConnection != NULL)
    readConnection->messageSent();
  return recMsgCtr;}
int RemoteSite::readRecMsgCtr(){
  return recMsgCtr;}



/************************************************************/
/* SECTION 23: Prm/Tmp-Down routines                        */
/************************************************************/

void RemoteSite::sitePrmDwn(){
  // printf("Site prm %d %d %s\n",(int)writeConnection, (int) readConnection,site->stringrep());
  status = SITE_PERM;
  if(readConnection!=NULL && readConnection->isReading()){
    readConnection->setCrashed();
    return;}

  if(writeConnection!=NULL){
    /* If we are in a Tmp state we must now remove the
       writeconnection from TC. I dont know about the LOST....
    */
    if(writeConnection->getFD() != LOST ||
       writeConnection->isMyInitiative()||
       writeConnection->isHisInitiative()||
       writeConnection->isProbing() ||
       writeConnection->isTmpDwn()||
       writeConnection->isOpening())
      writeConnection->prmDwn();
    writeConnectionManager->freeConnection(writeConnection);
    writeConnection = NULL;}

  if(readConnection!=NULL){
    readConnection->prmDwn();
    readConnectionManager->freeConnection(readConnection);
    readConnection=NULL;}
  Message* m;
  while((m = writeQueue.removeFirst()) && m != NULL) {
    site->communicationProblem(m->msgType, m->site, m->storeIndx,
                               COMM_FAULT_PERM_NOT_SENT,(FaultInfo) m->bs);
    //printf("RS wq %d\n",(int)m->bs);
    messageManager->freeMessageAndMsgBuffer(m);}
  remoteSiteManager->freeRemoteSite(this);}


void RemoteSite::siteTmpDwn(closeInitiator ci){
  if(status ==  SITE_OK && ci == TMP_INITIATIVE)
    status = SITE_TEMP;
  if(writeConnection!=NULL)
    writeConnection->close(ci);}

void ReadConnection::prmDwn(){
  //fprintf(stderr,"ReadConnection is taken down fd: %d\n",fd);
  osclose(fd);
  OZ_unregisterRead(fd);
  fd =  -1;
  tcpCache->remove(this);
  //EK possible mem leak
  if(isIncomplete())
    messageManager->freeMessageAndMsgBuffer(getCurQueue());}

void WriteConnection::prmDwn(){
  if(fd!=LOST)  osclose(fd);
  //fprintf(stderr,"WriteConDead %s %s \n",this->remoteSite->site->stringrep(),myDSite->stringrep());
  PD((TCP,"WriteConnection is taken down fd: %d %d",fd,getFD()));
  tcpCache->remove(this);
  remoteSite->site->probeFault(PROBE_PERM);

  Message *m = sentMsg;
  sentMsg = NULL;
  while(m != NULL){
    PD((ACK_QUEUE,"Emptying ackqueue m:%x bs: %x",m, m->bs));
    m->bs->resend();
    remoteSite->site->communicationProblem(m->msgType, m->site, m->storeIndx,
                                           COMM_FAULT_PERM_MAYBE_SENT,(FaultInfo) m->bs);
    Message *tmp = m;
    m = m->next;

    messageManager->freeMessageAndMsgBuffer(tmp);}
  if(fd!=LOST)
    OZ_unregisterRead(fd);

  if(!isWritePending())
    return;

  if(fd!=LOST) OZ_unregisterWrite(fd);

  if(isIncomplete()) {
    clearIncomplete();
    current = NULL;}
  if(isInWriteQueue())
    clearInWriteQueue();
}

/************************************************************/
/* SECTION 24: closing                                       */
/************************************************************/

void WriteConnection::setWantsToClose(){
  PD((TCP_INTERFACE,"setWantsToClose site:%s",remoteSite->site->stringrep()));
  Assert(!testFlag(WANTS_TO_CLOSE));
  Assert(!testFlag(CLOSING));
  setFlag(WANTS_TO_CLOSE);}

void WriteConnection::clearWantsToClose(){
  PD((TCP_INTERFACE,"nowClosing site:%s",remoteSite->site->stringrep()));
  clearFlag(WANTS_TO_CLOSE);
  Assert(!testFlag(CLOSING));}

void WriteConnection::closeConnection(){
  PD((TCP_INTERFACE,"tcpCloseWriter r:%x site: %s",this,remoteSite->site->stringrep()));
  int fd=getFD();
  Assert(fd!=LOST);
  if(isWantsToClose())
    clearWantsToClose();
  if(!isClosing()){
    if(isIncomplete() || isOpening()){
      setWantsToClose();
      return;}
    setClosing();}
  NetMsgBuffer* nb = netMsgBufferManager->
    getNetMsgBuffer(NULL);
  nb->constructMsg(remoteSite,TCP_CLOSE_REQUEST_FROM_WRITER);
  Message* m= messageManager->
    allocMessage(nb, NO_MSG_NUM, NULL,(MessageType) 0, 0);
  switch(isInWriteQueue()?IP_BLOCK:tcpSend(fd,m,TRUE)){
  case IP_OK:{
    PD((TCP,"CloseMsg sent"));
    messageManager->freeMessageAndMsgBuffer(m);
    return;}
  case IP_BLOCK:{
    PD((TCP,"CloseMsg Blocked"));
    addCurQueue(m);
    OZ_registerWriteHandler(fd,tcpWriteHandler,(void *)this);
    return;}
  case IP_PERM_BLOCK:{
    PD((TCP,"CloseMsg nSent sitecrash"));
    messageManager->freeMessageAndMsgBuffer(m);
    connectionLost();
    return;}
  default:  Assert(0);}}

void WriteConnection::close(closeInitiator type){
  //printf("close r:%x fd:%d type(%d:MY %d:HIS %d:TMP): %d \n",
  //   (int)this,fd, MY_INITIATIVE,HIS_INITIATIVE,TMP_INITIATIVE,type);
  if(fd >0)
    {OZ_unregisterRead(fd);
    OZ_unregisterWrite(fd);
    osclose(fd);}
  if(isIncomplete())
    getCurQueue();
  tcpCache->remove(this);
  if(isClosing()) clearClosing();
  if(isOpening())
    clearOpening();
  if(type == TMP_INITIATIVE && !isProbingOK()){
    remoteSite->site->probeFault(PROBE_TEMP);
    setProbingOK();}
  if(type!=TMP_INITIATIVE) sentMsgCtr=0;
  if(sentMsg!=NULL || isWritePending()) {
    //printf("Storing connection %d\n",(int)this);
    switch(type){
    case MY_INITIATIVE:
      setMyInitiative();
      break;
    case HIS_INITIATIVE:
      if(sentMsg!=NULL || isWritePending()){
        remoteSite->resendAckQueue(sentMsg);
        setInWriteQueue();}
      sentMsg = NULL;
      setHisInitiative();
      break;
    case TMP_INITIATIVE:
      setTmpDwn();
      break;
    default:
      OZ_warning("Unknown type of close reason %d",type);
      Assert(0);}

    clearFD();
    tcpCache->add(this);
    return;}
  if(hasReferences()){
    clearFD();
    setProbing();
    tcpCache->add(this);
    return;}

  PD((TCP_CONNECTIONH,"Discarding Connection"));
  informSiteRemove();
  clearFlag(WRITE_CON);
  writeConnectionManager->freeConnection(this);}

void ReadConnection::closeConnection(){
  BYTE msg;
  Assert(!isClosing());
  int fd=getFD();
  Assert(fd!=LOST);
  PD((TCP_CONNECTIONH,"tcpCloserReader r:%x",this));
  msg=TCP_CLOSE_REQUEST_FROM_READER;
  if(tcpAckReader(this,remoteSite->getRecMsgCtr())
     && fd != LOST && IP_OK ==writeI(fd,&msg)){
    setClosing();}

  return;}

void ReadConnection::close(){
  PD((TCP_CONNECTIONH,"close ReadConnection r:%x fd:%d",this,fd));
  if(fd!=LOST){
    OZ_unregisterRead(fd);
    osclose(fd);
    clearFD();}
  tcpCache->remove(this);
  informSiteRemove();
  if(isIncomplete())
    messageManager->freeMessageAndMsgBuffer(getCurQueue());
  readConnectionManager->freeConnection(this);}



/************************************************************/
/* SECTION 25: RemoteSite Init                              */
/************************************************************/

void RemoteSite::init(DSite* s, int msgCtr){
    writeConnection=NULL;
    readConnection=NULL;
    recMsgCtr = msgCtr;
    tmpMsgNum = 1;
    totalMsgSize = 0;
    MsgMonitorSize = -1;
    treashFullCtr = -1;
    site = s;
    tSn = 0;
    status = SITE_OK;
    nrOfSentMsgs = 0;
    nrOfRecMsgs = 0;
}

void RemoteSite::setWriteConnection(WriteConnection *r){
  Assert(writeConnection==NULL);
  writeConnection=r;
  r->setSite(this);}

void RemoteSite::setReadConnection(ReadConnection *r){
  Assert(readConnection==NULL);
  readConnection=r;
  r->setSite(this);}

/************************************************************/
/* SECTION 26: IOQueue methods                              */
/************************************************************/

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
  Assert(((first==NULL) && (last==NULL))||
         ((first!=NULL) && (last!=NULL)));
  if(last!=NULL){last->next=m;Assert(m->next==NULL);}
  else {Assert(first==NULL);first=m;}
  last=m;
  return;}
void IOQueue::addfirst(Message *m){
  Assert(((first==NULL) && (last==NULL))||
         ((first!=NULL) && (last!=NULL)));
  if(first!=NULL){
    Assert(m->next==NULL);
    m->next=first;}
  else {
    Assert(last==NULL);
    last=m;}
  first=m;
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

/************************************************************/
/* SECTION 27:probes                                           */
/************************************************************/

//
// This should be removed
//


ProbeReturn RemoteSite::installProbe(ProbeType pt){
  return PROBE_INSTALLED;}
/*if(siteStatus() == SITE_PERM)
    return PROBE_PERM;
  if(writeConnection==NULL){
    PD((TCP_INTERFACE,"try open %s",site->stringrep()));
    writeConnection=writeConnectionManager->allocConnection(this,LOST);
    if(writeConnection->open() == IP_PERM_BLOCK)
      return PROBE_PERM;
    PD((TCP,"is reopened %s",site->stringrep()));}
  writeConnection->installProbe(pt);
  if(siteStatus() == SITE_TEMP && pt==PROBE_TYPE_ALL)
    return PROBE_TEMP;
    return PROBE_INSTALLED;}*/

ProbeReturn RemoteSite::deInstallProbe(ProbeType pt){
  return PROBE_DEINSTALLED;}
/*  if(siteStatus() == SITE_PERM)
    return PROBE_DEINSTALLED;
  Assert(writeConnection!= NULL);
  writeConnection->deInstallProbe(pt);
  return PROBE_DEINSTALLED;}*/

void RemoteSite::addWriteQueue(Message* m){
  if(writeConnection)
    writeConnection->setInWriteQueue();
  else
    Assert(status == SITE_TEMP);
  writeQueue.enqueue(m);}

Message* RemoteSite::getWriteQueue(){
  Message* m = writeQueue.removeFirst();
  if(writeQueue.queueEmpty())
    writeConnection->clearInWriteQueue();
  return m;}

void RemoteSite::sendAck(){
  PD((SITE,"Ack Invoked from perdioLayer"));
  if(siteStatus() == SITE_PERM){return;}
  Assert(readConnection != NULL);
  readConnection->sendAck();}

int RemoteSite::
sendTo(NetMsgBuffer *bs, MessageType msg,
       DSite *storeS, int storeInd)
{
  int msgNum,ret;
  Message *m=messageManager->allocMessage(bs,NO_MSG_NUM,
storeS,msg,storeInd);


  switch (siteStatus()){
  case SITE_TEMP:{
    queueMessage(m->getMsgBuffer()->getTotLen());
    bs->beginWrite(this);
    goto tmpdwnsend2;}
  case SITE_PERM:{
    return PERM_NOT_SENT;}
  case SITE_OK:{
    queueMessage(m->getMsgBuffer()->getTotLen());}}
  bs->beginWrite(this);
  int fd;
  if(writeConnection==NULL){
    PD((TCP_INTERFACE,"try open %s",site->stringrep()));
    writeConnection=writeConnectionManager->allocConnection(this,LOST);
    fd = writeConnection->open();
    if(fd == IP_OK){
      PD((TCP_INTERFACE,"is opened %s",site->stringrep()));
      goto ipBlockSend;}
    /* This Code should be removed EK */
    if(fd==IP_TEMP_BLOCK){
      PD((TCP_INTERFACE,"is blocked opened %s",site->stringrep()));
      goto tmpdwnsend;}
    PD((TCP_HERROR,"is lost %s", site->stringrep()));
    return PERM_NOT_SENT;}
  else{
    if(!writeConnection->shouldSendFromUser())
      goto ipBlockSend;
    /*if(writeConnection->isReadReading()){
      OZ_registerWriteHandler(fd,tcpWriteHandler,
                              (void *)this->writeConnection);
      printf("sendT stoped by read\n");
      goto ipBlockSend;}
      */
    tcpCache->touch(writeConnection);}

#ifdef SLOWNET
  // In case of slownet, all msgs are put in the writeques.
  m->getMsgBuffer()->PiggyBack(m);
  writeConnection->addCurQueue(m);
  TSC->addWrite(this->writeConnection);
  return ACCEPTED;
#endif

  fd=writeConnection->getFD();
  Assert(fd>0);

  switch(tcpSend(fd,m,FALSE)){
  case IP_OK:{
    incNOSM();
    deQueueMessage(m->getMsgBuffer()->getTotLen());
    PD((TCP_INTERFACE,"reliableSend- all sent %d"));
    return ACCEPTED;}
  case IP_BLOCK:{
    PD((TCP_INTERFACE,"reliableSend- part sent"));
    OZ_registerWriteHandler(fd,tcpWriteHandler,
                            (void *)this->writeConnection);
    writeConnection->addCurQueue(m);
    return ACCEPTED;}
  case IP_TEMP_BLOCK:
    goto tmpdwnsend;
  case IP_PERM_BLOCK:{
    PD((TCP_INTERFACE,"reliableSend- perm failed"));
    site->discoveryPerm();
    // The message will be removed at network level.
    return ACCEPTED;}
  default:{
    Assert(0);
    return PERM_NOT_SENT;}}
tmpdwnsend:
#ifdef SLOWNET
  TSC->addWrite(this->writeConnection);
#endif
  PD((TCP_INTERFACE,"TMPDWNSEND1"));
  siteTmpDwn(TMP_INITIATIVE);
tmpdwnsend2:
  PD((TCP_INTERFACE,"TMPDWNSEND2"));
  msgNum = getTmpMsgNum();
  addWriteQueue(m);
  return msgNum;

ipBlockSend:
#ifdef SLOWNET
  TSC->addWrite(this->writeConnection);
#endif
  PD((TCP_INTERFACE,"sendTo IpBlock add to writeQueue %d",m));

  addWriteQueue(m);

  return ACCEPTED;
}

/**********************************************************************/
/*   SECTION 28: exported to the protocol layer              */
/**********************************************************************/

RemoteSite* createRemoteSite(DSite* site, int readCtr){
  RemoteSite *rSite = remoteSiteManager->allocRemoteSite(site, readCtr);
  return rSite;
}

void zeroRefsToRemote(RemoteSite *s){s->zeroReferences();}

int sendTo_RemoteSite(RemoteSite* rs,MsgBuffer* bs,MessageType m,DSite* s, int i){
  return rs->sendTo((NetMsgBuffer*)bs,m,s,i);}
void sendAck_RemoteSite(RemoteSite* rs){
  rs->sendAck();}

int discardUnsentMessage_RemoteSite(RemoteSite* s,int msg){
   return MSG_SENT;}
SiteStatus siteStatus_RemoteSite(RemoteSite* s){
  return s->siteStatus();}

void monitorQueue_RemoteSite(RemoteSite* site,int size){
  site->monitorQueueMsgs(size);}
void demonitorQueue_RemoteSite(RemoteSite* site){
  site->deMonitorQueue();}
int getTreashCtr_RemoteSite(RemoteSite* site){
  return site->getTreashCtr();}
int getQueueStatus_RemoteSite(RemoteSite* s){
  return s->getQueueSize();}


ProbeReturn installProbe_RemoteSite(RemoteSite* site,ProbeType pt, int frequency){
  return site->installProbe(pt);}
ProbeReturn deinstallProbe_RemoteSite(RemoteSite* site,ProbeType pt){
  return site->deInstallProbe(pt);}
ProbeReturn probeStatus_RemoteSite(RemoteSite* site,ProbeType &pt,int &frequncey,void* &storePtr){
  Assert(0);return PROBE_PERM;}


GiveUpReturn giveUp_RemoteSite(RemoteSite* site){
  Assert(0);return GIVES_UP;}


void discoveryPerm_RemoteSite(RemoteSite* site){
  site->sitePrmDwn();}

void siteAlive_RemoteSite(RemoteSite*) {}

int startNiceClose(){
  return tcpCache->shutDwnTcpCache();}
int niceCloseProgress(){
  return tcpCache->shutDwnTcpCacheProgres();}


void initNetwork()
{
  ip_address ip;
  port_t p;
  int tcpFD;
  PD((TCP_INTERFACE,"Init Network"));


  writeConnectionManager = new WriteConnectionManager();
  readConnectionManager = new ReadConnectionManager();
  netMsgBufferManager = new NetMsgBufferManager();
  remoteSiteManager = new RemoteSiteManager();
  netByteBufferManager = new NetByteBufferManager();
  messageManager = new MessageManager();
  tcpCache = new TcpCache();
  tcpOpenMsgBuffer= new TcpOpenMsgBuffer();

  ipReturn ret=createTcpPort(OZReadPortNumber,ip,p,tcpFD);
  if (ret<0){
    PD((WEIRD,"timer"));
    Assert(ret==IP_TIMER);
    return;}
  TimeStamp timestamp(time(0),osgetpid());
  mySiteInfo.tcpFD=tcpFD;
  mySiteInfo.maxNrAck = 100;
  mySiteInfo.maxSizeAck = 10000;
  Assert(myDSite==NULL);
  myDSite = makeMyDSite(ip, p, timestamp);
  Assert(myDSite!=NULL);
  OZ_registerAcceptHandler(tcpFD,acceptHandler,NULL);
  PD((OS,"register ACCEPT- acceptHandler fd:%d",tcpFD));
  tcpCache->nowAccept();  // can be removed ??

  if(!am.registerTask((void*) tcpCache, checkTcpCache, wakeUpTcpCache))
    OZ_error("Unable to register TCPCACHE task");


#ifdef SLOWNET
  TSC = new TSCQueue();
  if(!am.registerTask(NULL, checkIncTimeSlice, incTimeSlice))
    OZ_error("Unable to register TSC task");
  TSC_LATENCY = 300;
  TSC_TOTAL_A = 4000;
#endif
#ifdef PERDIOLOGLOW
  printf("#!!!mySite('%s')#&&&\n",myDSite->stringrep());
#endif
}

MsgBuffer* getRemoteMsgBuffer(DSite* s){
  return netMsgBufferManager->getNetMsgBuffer(s);}

void dumpRemoteMsgBuffer(MsgBuffer *m){
  netMsgBufferManager->dumpNetMsgBuffer((NetMsgBuffer*) m);}



/**********************************************************************/
/*   SECTION 29: exported for debugging                                */
/**********************************************************************/


int timeCtr = 0;
#define MY_INITIATED_CLOSE 5
#define HIS_INITIATED_CLOSE 10
#define PROBE 20


/**********************************************************************/
/*   SECTION 30: SLOWNET method, dependent of writeconnection     */
/**********************************************************************/
#ifdef SLOWNET
void TSCQueue::incTime(unsigned long aTime){

  if(transferAmount){
    int decrease = (int)((aTime - dtime) *  TSC_TOTAL_A / 1000);

    if(decrease){
      transferAmount -= decrease;
      if(transferAmount < 0) transferAmount  = 0;}}

  time =  dtime = aTime;
  //dtime * TSCQ_TIME_SLICE;

  TSCQElement *tptr = ptr;
  TSCQElement *tmpLast = last, *tmp;
  ptr = NULL;
  last = NULL;

  int ctr = 0;

  //  printf("slownet: incTime t:%d ta:%d tptr:%d tptrT:%d nf:%d\n",(int)time,
  // transferAmount,(int)tptr, (int)tptr->t,netAmountFull() );

  while(tptr && tptr->t < time && !netAmountFull()){
    if(tptr->type == TSCQ_WRITE){
      reAdd = FALSE;
      (void) tcpWriteHandler((int) -1 ,(void *) tptr->c);
      tmp = tptr->e;
      if(reAdd){
        tptr->e = NULL;
        addelement(tptr);}
      else{
        freeTSCQElement(tptr);}
      tptr = tmp;
    }
    else{
      NetMsgBuffer* nb = (NetMsgBuffer*)tptr->c;
      bs->unmarshalBegin();
      msgReceived(nb);
      bs->unmarshalEnd();
      reading(nb->getslownetTotLen());
      tmp = tptr->e;
      //netMsgBufferManager->dumpNetMsgBuffer(nb);
      freeTSCQElement(tptr);
      tptr = tmp;
    }
    ctr ++;
  }

  // printf("ctr: %d\n",ctr);

  if(tptr){
    addelement(tptr);
    last = tmpLast;}
}



/* ************************************************************************ */
/*  SECTION 42: Procerdures used for "Signals"                                            */
/* ************************************************************************ */

Bool incTimeSlice(unsigned long time, void *v){
  TSC->incTime(time);
  return TRUE;}

Bool checkIncTimeSlice(unsigned long time, void* v){
  return TSC->hasTask(time);}

#endif

/* ************************************************************************ */
/*  SECTION 43: DistPane-Info                                                    */
/* ************************************************************************ */

int getNetMsgBufferManagerInfo(int &size){
  size = sizeof(NetMsgBuffer);
  return netMsgBufferManager->getCTR();}

int getNetByteBufferManagerInfo(int &size){
  size = sizeof(NetByteBuffer);
  return netByteBufferManager->getCTR();}

int getWriteConnectionManagerInfo(int &size){
  size = sizeof(WriteConnection);
  return writeConnectionManager->getCTR();}

int getReadConnectionManagerInfo(int &size){
  size = sizeof(ReadConnection);
  return readConnectionManager->getCTR();}

int getMessageManagerInfo(int &size){
  size = sizeof(Message);
  return messageManager->getCTR();}

int getRemoteSiteManagerInfo(int &size){
  size = sizeof(RemoteSite);
  return remoteSiteManager->getCTR();}

int getNOSM_RemoteSite(RemoteSite* site){
  return site->getNOSM();}

int getNORM_RemoteSite(RemoteSite* site){
  return site->getNORM();}
