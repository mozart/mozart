
class MarshalInfo {
public:
  OZ_Term resources,saveTheseURLsToo, urlsFound, names,currentURL;
  MarshalInfo(OZ_Term ds, OZ_Term urls,OZ_Term current) { 
    resources = nil(); 
    names     = nil(); 
    currentURL= current;
    saveTheseURLsToo = deref(ds);
    urlsFound = literalEq(urls,NameUnit) ? NameUnit : nil();
  }
  void addRes(OZ_Term res) { resources = cons(res,resources); }
  void addURL(OZ_Term url) { 
    if (!literalEq(urlsFound,NameUnit) && !member(url,urlsFound)) {
      urlsFound = cons(url,urlsFound);
    }
  }
};


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
  MarshalInfo *info;

public:  
  Site *getSite(){return (Site*) NULL;}
  char *siteStringrep() {return "toFile";}
  void skipHeader();

  void setMarshalInfo(MarshalInfo *mi){info=mi;}

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

  int no_bufs(){
    int i=0;
    ByteBuffer *bb=first;
    while(bb!=NULL){
      i++;
      bb=bb->next;}
    return i;}

  void dumpByteBuffers();

      

  /* init */

  ByteStream():first(NULL),last(NULL),pos(NULL),type(BS_None){}
  void init(){type=BS_None;first=NULL;last=NULL;info=NULL;}

  /* marshal    beg:first->head()  pos=next free slot OR null */
                   /* INTERFACE  pos=first->head()  endpos= first free slot */
  void marshalBegin();

  void put(BYTE b){
    PD((MARSHAL_CT,"one character put %c",b));
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

  void marshalEnd(){
    Assert(type==BS_Marshal);
    PD((MARSHAL_BE,"marshal end"));
    endpos=pos;
    pos=first->head();
    if(endpos==NULL) {totlen +=BYTEBUFFER_SIZE;}
    else {totlen +=endpos-last->head();}
    type=BS_None;}

  /* unmarshal pos=first unread BYTE curpos last unread BYTE */

  void unmarshalBegin(){
    type=BS_Unmarshal;
    PD((MARSHAL_BE,"unmarshal begin"));
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
      PD((BUFFER,"bytebuffer dumped UnMarshal: %d",no_bufs()));
      pos=first->head();
      return ch;}
    PD((MARSHAL_CT,"one character got %c",*pos));
    return *pos++;}

  void unmarshalEnd(){
    Assert(type==BS_Unmarshal);    
    PD((MARSHAL_BE,"unmarshal end"));
    type=BS_None;
    Assert(pos==NULL);}

// new stuff

  void addRes(OZ_Term t){
    info->addRes(t);}

  void addURL(OZ_Term t){
    info->addURL(t);}

  Bool saveAnyway(OZ_Term t){
    if((literalEq(NameUnit,info->saveTheseURLsToo) ||
	member(t,info->saveTheseURLsToo))) {return NO;}
    return OK;}

  void marshaledProcHasNames(TaggedRef t){
    info->names=t;}

  Bool knownAsNewName(OZ_Term t){
    if(member(t,info->names)) {return OK;}
    return NO;}

  void gnameMark(GName* gname){gname->markURL(info->currentURL);}
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
  ByteBufferManager *byteBufM;
  ByteStreamManager *byteStreamM;

  CompBufferManager(){
    byteBufM = new ByteBufferManager();
    byteStreamM= new ByteStreamManager();}

  ByteStream *getByteStream();

  void freeByteStream(ByteStream *bs);
  ByteStream* newByteStream();
  void CompBufferManager::deleteByteStream(ByteStream* bs);

  ByteBuffer* getByteBuffer();
  void freeByteBuffer(ByteBuffer *bb);
  void dumpByteStream(ByteStream *);
};

CompBufferManager *bufferManager= new CompBufferManager();

MsgBuffer* getComponentMsgBuffer(){        // interface
  return bufferManager->getByteStream();}

/* **********************************************************************
 *                  BYTE STREAM
 * ********************************************************************** */

/* ByteBufferManger */

inline ByteBuffer* ByteBufferManager::newByteBuffer(){
  FreeListEntry *f=getOne();
  ByteBuffer *bb;
  if(f==NULL) {bb=new ByteBuffer();}
  else{GenCast(f,FreeListEntry*,bb,ByteBuffer*);}
  return bb;}

inline  void ByteBufferManager::deleteByteBuffer(ByteBuffer* bb){
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
  bs->ByteStream();
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
  PD((MARSHAL_BE,"marshal begin"));
  Assert(type==BS_None);
  Assert(first==NULL);
  Assert(last==NULL);
  first=getAnother();
  last=first;
  totlen= 0;
  type=BS_Marshal;
  pos=first->head()+tcpHeaderSize;}

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

void ByteStream::skipHeader(){get(); }

BYTE* ByteStream::initForRead(int &len){
  Assert(type==BS_None);
  PD((BUFFER,"bytebuffer alloc Read: %d",no_bufs()));
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
    PD((BUFFER,"bytebuffer alloc Read: %d",no_bufs()));
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
  Assert(within(curpos,last));}

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



