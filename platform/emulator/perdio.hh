/*
  Perdio Project, DFKI & SICS,
  Universit"at des Saarlandes
  Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
  SICS
  Box 1263, S-16428 Sweden, Phone (+46) 8 7521500
  Author: brand,scheidhr, mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __PERDIOHH
#define __PERDIOHH

#ifdef INTERFACE  
#pragma interface
#endif

#include "tagged.hh"
#include "genhashtbl.hh"
#include "perdio_debug.hh"
#include "runtime.hh"
#include "../include/config.h"


#define PERDIOVERSION    OZVERSION "#3"

/* magic marker for start of saved components */
#define PERDIOMAGICSTART       '\1'


/* TODO */
#define DummyClassConstruction(X) \
X();  \
~X(); \
(X&); 

OZ_Return remoteSend(Tertiary *p, char *biName, TaggedRef msg);
void portSend(Tertiary *p, TaggedRef msg);

void cellDoExchange(Tertiary*,TaggedRef,TaggedRef,Thread*);
void cellDoAccess(Tertiary*,TaggedRef);
TaggedRef cellGetContentsFast(Tertiary *c);

void networkSiteDec(int sd);

#define tert2PortManager(t)   ((PortManager*) t)
#define tert2PortLocal(t)     ((PortLocal*) t)
#define tert2PortProxy(t)     ((PortProxy*) t)

void gcOwnerTable();
void gcBorrowTable1();
void gcBorrowTable2();
void gcBorrowTable3();
void gcFrameToProxy();
void gcPortProxy(PortProxy* );
void gcPortManager(PortManager* );
void gcPendThread(PendThread **);

#define BYTEBUFFER_CUTOFF 100
#define BYTEBUFFER_SIZE 2048
#define BYTESTREAM_CUTOFF 40

class ByteBuffer{
  friend class ByteBufferManager;
  friend class ByteStream;
  friend class BufferManager;
  
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
  ByteBuffer *newByteBuffer();
  void deleteByteBuffer(ByteBuffer*);
};

enum ByteStreamType{
  BS_None,
  BS_Marshal,
  BS_Write,
  BS_Read,
  BS_Unmarshal
};

class ByteStream {
  friend class ByteStreamManager;
  friend class BufferManager;
  ByteBuffer *first; 

  ByteBuffer *last;
  BYTE *pos; 
  BYTE *curpos;
  BYTE *endpos; 
  int totlen;  /* include header */
  int type;

  int availableSpace(){
    Assert(last!=NULL);
    if(endpos==NULL) return 0;
    Assert(within(endpos,last));
    return last->tail()-endpos+1;}

  ByteBuffer *beforeLast(){
    Assert(first!=last);
    ByteBuffer *bb=first;
    while(bb->next!=last){bb=bb->next;}
    return bb;}

  Bool within(BYTE *p,ByteBuffer *bb){
    if(p<bb->head()) return FALSE;
    if(p>bb->tail()) return FALSE;  
    return TRUE;}

public:    
  /* init */

  void print() {
    printf("byte stream\n");
  }
  ByteStream():first(NULL),last(NULL),pos(NULL),type(BS_None){}

  void init(){type=BS_None;first=NULL;last=NULL;}

  /* basic */

  void removeFirst();
  void removeSingle();
  ByteBuffer* getAnother();
  void getSingle(){
    Assert(first==NULL);
    Assert(last==NULL);
    ByteBuffer *bb=getAnother();
    first=bb;
    last=bb;}

  /* marshall    beg:first->head()  pos=next free slot OR null */

  void marshalBegin();

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
    PD((MARSHALL_BE,"marshal end"));
    endpos=pos;
    pos=first->head();
    if(endpos==NULL) {totlen +=BYTEBUFFER_SIZE;}
    else {totlen +=endpos-last->head();}
    type=BS_None;}

  /* write  pos=first to write  endpos= first free slot */

  void beginWrite(); /* putting end packet header */

  int getTotLen(){return totlen;}  

  void sentFirst(){
    Assert(type==BS_Write);
    PD((BUFFER,"bytebuffer dumped sent: %d",no_bufs()));
    if(first==last){
      removeSingle();
      return;}
    removeFirst();
    pos=first->head();}
  
  int getWriteLen(){
    Assert(type==BS_Write);
    Assert(first!=NULL);
    if(first==last){
      if(endpos!=NULL) {return endpos-pos;}
      return first->tail()-pos+1;}
    return first->tail()-pos+1;}

  BYTE* getWritePos() {Assert(type==BS_Write);return pos;}

  void incPosAfterWrite(int i){
    Assert(type==BS_Write);
    Assert(within(pos,first));
    Assert(pos+i<=first->tail());
    pos +=i;}

  int calcTotLen(){
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

  void writeCheck(){
    Assert(type==BS_Write);
    Assert(first==NULL);
    Assert(last==NULL); 
    return;}
      

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
    PD((MARSHALL_BE,"unmarshal begin"));
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
      removeFirst();
      PD((BUFFER,"bytebuffer dumped UnMarshall: %d",no_bufs()));
      pos=first->head();
      return ch;}
    return *pos++;}

  void unmarshalEnd(){
    Assert(type==BS_Unmarshal);    
    PD((MARSHALL_BE,"unmarshal end"));
    type=BS_None;
    Assert(pos==NULL);}
  
  int no_bufs(){
    int i=0;
    ByteBuffer *bb=first;
    while(bb!=NULL){
      i++;
      bb=bb->next;}
    return i;}

  void dumpByteBuffers();
};

class ByteStreamManager: public FreeListManager {
public:
  ByteStreamManager():FreeListManager(BYTESTREAM_CUTOFF){}
  ByteStream *newByteStream();
  void deleteByteStream(ByteStream *);
};

class BufferManager{
public:
  ByteBufferManager *byteBufM;
  ByteStreamManager *byteStreamM;

  BufferManager(){
    byteBufM = new ByteBufferManager();
    byteStreamM= new ByteStreamManager();}

  ByteStream* getByteStream();
  ByteStream* getByteStreamMarshal();
  void freeByteStream(ByteStream *bs);
  ByteBuffer* getByteBuffer();
  void freeByteBuffer(ByteBuffer *bb);
  void dumpByteStream(ByteStream *bs);
};

class PendThread{
public:
  PendThread *next;
  Thread *thread;
  PendThread(Thread *th,PendThread *pt):thread(th),next(pt){}
  PendThread(Thread *th):thread(th),next(NULL){}
  PendThread(){thread=NULL;}
  USEFREELISTMEMORY;
  void dispose(){freeListDispose(this,sizeof(PendThread));}
};
  
extern BufferManager *bufferManager;

void gcGNameTable();
void gcGName(GName*);
void getCode(ProcProxy*);
GName *copyGName(GName*);
void deleteGName(GName*);

enum GNameType {
  GNT_NAME,
  GNT_PROC,
  GNT_CODE,
  GNT_CHUNK,
  GNT_OBJECT,
  GNT_CLASS
};

GName *newGName(TaggedRef t, GNameType gt);
GName *newGName(PrTabEntry *);

PrTabEntry *findCodeGName(GName *);

int loadURL(TaggedRef url, OZ_Term out);
int loadURL(char *url, OZ_Term out);
int perdioInit();

/* __PERDIOHH */
#endif 
