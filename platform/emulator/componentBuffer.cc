/*
 *  Authors:
 *    Author's name (Author's email address)
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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */



#define BYTEBUFFER_CUTOFF 100
#define BYTEBUFFER_SIZE   2048
#define BYTESTREAM_CUTOFF 40

class ByteBuffer{
  friend class CByteBufferManager;
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

class CByteBufferManager: public FreeListManager {
public:
  CByteBufferManager():FreeListManager(BYTEBUFFER_CUTOFF){}
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

inline
Bool isResource(OZ_Term t)
{
  if (oz_isFree(t) || oz_isFuture(t) || oz_isPort(t))
    return OK;
  return ozconf.perdioMinimal
    ? NO
    : oz_isObject(t) || oz_isLock(t) || oz_isCell(t);
}


class ByteStream: public MsgBuffer {
  friend class ByteStreamManager;
  friend class CompBufferManager;
  ByteBuffer *first;

  ByteBuffer *last;
  BYTE *pos;
  BYTE *curpos;
  BYTE *endpos;
  int totlen;  /* include header */
  int type;

  OZ_Term  resources;
  int perdioMajor, perdioMinor;

public:

  virtual Bool visit(OZ_Term val)
  {
    OZ_Term t = val;
    DEREF(t,_1,_2);
    if (isResource(t)) {
      resources = oz_cons(val,resources);
      return NO;
    }
    return OK;
  }
  OZ_Term getResources()    { return resources; }

  DSite *getSite() { return ((DSite*) NULL); }
  Bool isPersistentBuffer() { return OK; }

  char *siteStringrep() {return "toFile";}

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

  void removeFirst();
  void removeSingle();
  ByteBuffer* getAnother();
  void getSingle(){
    Assert(first==NULL);
    Assert(last==NULL);
    ByteBuffer *bb=getAnother();
    first=bb;
    last=bb;}

  /* marshal */
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
  /* write  pos=first to write  endpos= first free slot */

  void beginWrite(); /* putting end packet header */
  int getTotLen(){return totlen;}

  void sentFirst(){
    Assert(type==BS_Write);
    if(first==last){
      removeSingle();
      return;}
    removeFirst();
    pos=first->head();}

  int getWriteLen(){
    Assert(type==BS_Write);
    Assert(first!=NULL);
    if(first==last && endpos!=NULL) {return endpos-pos;}
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

  unsigned long crc()
  {
    unsigned long crc = init_crc();

    if(first==last){
      int len = (endpos!=NULL) ? (endpos-pos) : first->tail()-pos+1;
      return update_crc(crc,pos,len);
    } else {
      crc = update_crc(crc,pos,first->tail()-pos+1);
      ByteBuffer *bb=first->next;
      while(bb->next!=NULL){
        crc = update_crc(crc,bb->head(),BYTEBUFFER_SIZE);
        bb = bb->next;
      }
      Assert(bb==last);
      int len = (endpos==NULL) ? BYTEBUFFER_SIZE : endpos-last->head();
      return update_crc(crc,last->head(),len);
    }
  }

  void writeCheck(){
    Assert(type==BS_Write);
    Assert(first==NULL);
    Assert(last==NULL);
    return;}

  int no_bufs(){
    int i=0;
    ByteBuffer *bb=first;
    while(bb!=NULL){
      i++;
      bb=bb->next;}
    return i;}

  void dumpByteBuffers();



  /* init */

  virtual void init() {
    MsgBuffer::init();
    resources = oz_nil();
    type=BS_None;first=NULL;last=NULL;pos=NULL;
    perdioMajor = PERDIOMAJOR;
    perdioMinor = PERDIOMINOR;
  }
  ByteStream(){ init(); }

  void setVersion(int major, int minor) {
    perdioMajor = major;
    perdioMinor = minor;
  }

  virtual int getMinor() { return perdioMinor; }
  virtual void getVersion(int *major, int *minor) {
    *major = perdioMajor; *minor = perdioMinor; }


  /* marshal    beg:first->head()  pos=next free slot OR null */
                   /* INTERFACE  pos=first->head()  endpos= first free slot */
  void marshalBegin();

  void putNext(BYTE b){
    Assert(type==BS_Marshal);
    Assert(posMB==endMB+1);
    ByteBuffer *bb=getAnother();
    last->next=bb;
    last=bb;
    totlen += BYTEBUFFER_SIZE;
    posMB=bb->head();
    endMB=bb->tail();
    *posMB++=b;
    return;}

  void marshalEnd(){
    Assert(type==BS_Marshal);
    endpos=posMB;
    pos=first->head();
    if(endpos==NULL) {totlen +=BYTEBUFFER_SIZE;}
    else {totlen +=endpos-last->head();}
    type=BS_None;}

  /* unmarshal pos=first unread BYTE curpos last unread BYTE */

  void unmarshalBegin(){
    type=BS_Unmarshal;
    Assert(within(pos,first));
    if(first==last) {
      Assert(within(curpos,first));
      Assert(curpos>=pos);}
    // PB else {Assert(within(curpos,last));}
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
      markEnd();
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
    Assert(pos==NULL);}
};

class Exporter: public MsgBuffer {
  OZ_Term vars;

public:
  void marshalBegin()        { Assert(0); }
  void marshalEnd()          { Assert(0); }
  void unmarshalBegin()      { Assert(0); }
  void unmarshalEnd()        { Assert(0); }
  char* siteStringrep()      { Assert(0); return 0; }
  DSite* getSite()           { Assert(0); return 0; }
  virtual BYTE getNext()     { Assert(0); return 0; }
  virtual void putNext(BYTE) { Assert(0); }

  virtual Bool isPersistentBuffer() { return NO; }
  virtual Bool globalize()          { return NO; }

  Exporter() {
    posMB = endMB = 0; // so no data will be written nowhere
    MsgBuffer::init();
    vars = oz_nil();
  }

  OZ_Term getVars() { return vars; }

  virtual Bool visit(OZ_Term val)
  {
    OZ_Term t = val;
    DEREF(t,tPtr,_2);
    if (oz_isVariable(t))
      vars = oz_cons(val,vars);
    return OK;
  }
};


class ByteStreamManager: public FreeListManager {
public:
  ByteStreamManager():FreeListManager(BYTESTREAM_CUTOFF){}
  ByteStream *newByteStream();
  void deleteByteStream(ByteStream *);
};

class CompBufferManager {
private:
public:
  CByteBufferManager *byteBufM;
  ByteStreamManager *byteStreamM;

  CompBufferManager(){
    byteBufM = new CByteBufferManager();
    byteStreamM= new ByteStreamManager();}

  ByteStream *getByteStream();

  void freeByteStream(ByteStream *bs);
  ByteStream* newByteStream();
  void deleteByteStream(ByteStream* bs);

  ByteBuffer* getByteBuffer();
  void freeByteBuffer(ByteBuffer *bb);
  void dumpByteStream(ByteStream *);
};

CompBufferManager *bufferManager= new CompBufferManager();

//
// kost@ : interface methods. The only usage for it is SB-style
// exporting of variables;
MsgBuffer* getComponentMsgBuffer(){
  return (bufferManager->getByteStream());
}
void freeComponentMsgBuffer(MsgBuffer *buf) {
  bufferManager->freeByteStream((ByteStream *) buf);
}

/* **********************************************************************
 *                  BYTE STREAM
 * ********************************************************************** */

/* ByteBufferManger */

inline ByteBuffer* CByteBufferManager::newByteBuffer(){
  FreeListEntry *f=getOne();
  ByteBuffer *bb;
  if(f==NULL) {bb=new ByteBuffer();}
  else{GenCast(f,FreeListEntry*,bb,ByteBuffer*);}
  return bb;}

inline  void CByteBufferManager::deleteByteBuffer(ByteBuffer* bb){
  FreeListEntry *f;
  GenCast(bb,ByteBuffer*,f,FreeListEntry*);
  if(putOne(f)) return;
  delete bb;
  return;}


/* CompBufferManager */

inline ByteStream* ByteStreamManager::newByteStream(){
  FreeListEntry *f=getOne();
  ByteStream *bs;
  if(f==NULL) { return new ByteStream();}
  GenCast(f,FreeListEntry*,bs,ByteStream*);
  bs->init();
  return bs;}

inline  void ByteStreamManager::deleteByteStream(ByteStream* bs){
  FreeListEntry *f;
  GenCast(bs,ByteStream*,f,FreeListEntry*);
  if(putOne(f)) return;
  delete bs;
  return;}

/* ByteStream */

void ByteStream::removeFirst(){
  Assert(first!=last);
  ByteBuffer *bb=first;
  first=bb->next;
  bufferManager->freeByteBuffer(bb);}

void ByteStream::removeSingle(){
  Assert(first==last);
  bufferManager->freeByteBuffer(first);
  first=last=NULL;}

ByteBuffer *ByteStream::getAnother(){
  return(bufferManager->getByteBuffer());}

void ByteStream::marshalBegin(){
  Assert(type==BS_None);
  Assert(first==NULL);
  Assert(last==NULL);
  first=getAnother();
  last=first;
  totlen= 0;
  type=BS_Marshal;
  posMB=first->head()+tcpHeaderSize;
  endMB=first->tail();
  pos=NULL;
}

void ByteStream::dumpByteBuffers(){
  while(first!=last) {
    removeFirst();
  }
  removeSingle();
}

/* BufferManager */

ByteStream* CompBufferManager::getByteStream(){
  ByteStream *bs=byteStreamM->newByteStream();
  bs->init();
  return bs;}

void CompBufferManager::freeByteStream(ByteStream *bs){
  if(bs->first!=NULL){
    Assert(bs->first==bs->last);
    byteBufM->deleteByteBuffer(bs->last);}
  byteStreamM->deleteByteStream(bs);}

ByteBuffer* CompBufferManager::getByteBuffer(){
  ByteBuffer *bb=byteBufM->newByteBuffer();
  bb->init();
  return bb;}

void CompBufferManager::dumpByteStream(ByteStream *bs){
  bs->dumpByteBuffers();
  freeByteStream(bs);}

void CompBufferManager::freeByteBuffer(ByteBuffer* bb){
  byteBufM->deleteByteBuffer(bb);}


void ByteStream::beforeInterpret(int len){
  if(pos==NULL) {return;} /* read only header for close message*/
  Assert(type==BS_None);
  Assert(within(pos,first));
  Assert(len<=0);
  if(endpos==NULL){
    curpos=last->tail()+len;}
  else{
    curpos=endpos+len-1;}}


BYTE* ByteStream::initForRead(int &len){
  Assert(type==BS_None);
  pos=first->head();
  endpos=first->head();
  len=BYTEBUFFER_SIZE;
  type=BS_Read;
  return endpos;}

BYTE* ByteStream::beginRead(int &len){
  Assert(type==BS_None);
  type=BS_Read;
  if(endpos==NULL){
    if(pos==NULL){
      pos=first->head();
      endpos=first->head();
      len=BYTEBUFFER_SIZE;
      return endpos;}
    Assert(within(pos,first));
    ByteBuffer *bb=bufferManager->getByteBuffer();
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

void ByteStream::afterInterpret(){
  Assert(first==last);
  Assert(first!=NULL);
  pos=curpos+1;
  // PB Assert(within(curpos,last));
}

void ByteStream::afterRead(int len){
  Assert(type==BS_Read);
  Assert(len<=BYTEBUFFER_SIZE);
  Assert(availableSpace()>=len);
  Assert(within(endpos,last));
  type=BS_None;
  if(endpos+len-1==last->tail()){endpos=NULL;}
  else{endpos=endpos+len;}}

#define FILE_HEADER 0

inline BYTE *int2net(BYTE *buf,int i){
  for (int k=0; k<4; k++) {
    *buf++ = i & 0xFF;
    i = i>>8;}
  return buf;}

void ByteStream::beginWrite(){
  Assert(type==BS_None);
  Assert(first!=NULL);
  BYTE* thispos= first->head();
  *thispos++=FILE_HEADER;
  type=BS_Write;
  int2net(thispos,totlen);}
