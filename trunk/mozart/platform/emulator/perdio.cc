/*
  Perdio Project, DFKI & SICS,
  Universit"at des Saarlandes
  Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr, mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  network layer
  ------------------------------------------------------------------------
*/

#ifdef PERDIO

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "ip.hh"
#include "oz.h"
#include "am.hh"

#include "perdio_debug.cc"  

/**********************************************************************/
/**********************************************************************/
/*                        INITIAL                                     */
/**********************************************************************/
/**********************************************************************/



#define ProtocolObject Tertiary 
#define Proxy Tertiary



typedef long Credit;  /* TODO: full credit,long credit? */
typedef unsigned int Word;
typedef Word* AnyPtr;

class ByteStream;


/**********************************************************************/
/*                        NetAddress (not allocate only cast)         */
/**********************************************************************/

class NetAddress {       
public:
  /*  DummyClassConstruction(NetAddress)*/
  int site;
  int index;
  
  NetAddress(int s, int i) : site(s), index(i) {}

  void set(int s,int i) {site=s,index=i;}

  Bool same(NetAddress *na) { return na->site==site && na->index==index; }

  Bool isLocal() { return site==localSite; }
};


inline int reliableSend0(int,ByteStream *);
inline void marshallNumber(unsigned int,ByteStream *);
inline void marshallMySite(ByteStream* );
inline void marshallCredit(Credit,ByteStream *);

/* ********************************************************************** */
/*                  FreeListManager
/* ********************************************************************** */

class FreeListManager{
  AnyPtr free;
  int framesize;
  int frameno;

  void putBlock(Word* base){
    int i;
    for(i=0;i<frameno;i++){
      base[i*framesize] =  (Word) &base[(i+1)*framesize];}}

public:

  FreeListManager(int fs,int fn):
    framesize(fs),frameno(fn),free(NULL){}

  void putOne(AnyPtr p){
    *p =(Word)free;
    free=p;}

  AnyPtr getOne(){
    if(free==NULL) {
      AnyPtr base = (AnyPtr) malloc(framesize * frameno);
      putBlock(base);}
    AnyPtr ret=free;
    free= (AnyPtr)(*ret);
    return ret;}
};

/* ********************************************************************** */
/*                  BYTE STREAM
/* ********************************************************************** */

#define BSEOF (unsigned int) -1
#define DEFAULT_BYTE_STREAM_SIZE 1000

class ByteStream {
  char *array;
  int size;
  char *pos;
  int len;
public:
  ByteStream()
  {
    len = ipHeaderSize;
    size = DEFAULT_BYTE_STREAM_SIZE;
    array = new char[size];
    pos = array+len;
  }
  ByteStream(char *buf,int len) : len(len)
  {
    size=-1;
    array = buf;
    pos = buf;
  }
  ~ByteStream() { if (size>0) delete array; }

  void resize();

  void reset()  { pos = array; }

  unsigned int get() 
  {
    return pos>=array+len ? BSEOF : (unsigned int) (unsigned char) *pos++;
  }

  void endCheck(){
    return; /* TODO */
  }
       

  void put(char c)
  {
    Assert(size>0);
    if (pos>=array+size)
      resize();
    *pos++ = c;
    len++;
  }
  char *getPtr() { return array; }
  int getLen() { return len; }
};

/* ********************************************************************** */
/*                  PENDING MESSAGES STUFF
/* ********************************************************************** */

class PendEntry {
  int refCount;
  ByteStream *bs;
  BorrowEntry *back;
  int site;
public:           /* TODO overload new - delete with free list handlers */
  void send();
  PendEntry(ByteStream *bs1,int sd):
    bs(bs1),refCount(0),back(NULL),site(sd){};  

  PendEntry(ByteStream *bs1,int sd,BorrowEntry *b):
    bs(bs1),refCount(0),back(b),site(sd){};  

  void inc() {refCount++;}
  void dec() {refCount--;}
  BorrowEntry *getBack() {return back;}
  int getrefCount() {return refCount;}
  Bool isFIFO() {return back!=NULL;}
};

void PendEntry::send(){
  int ret=reliableSend0(site,bs);  /* TODO: delayed sending?? */
  Assert(ret==PROCEED); // TODO
}

class PendLink {
  Credit debt;
  PendEntry *pend;
public:
  PendLink *next;

  PendLink(Credit d,PendEntry *p):debt(d),next(NULL){}
  
  void setTag(){
    pend= (PendEntry *)(((unsigned int) pend) | 1);}

  Bool isTagged(){
    return ((unsigned int) pend & 1);}

  PendEntry *getPend(){
    return (PendEntry *)((unsigned int) pend & ~1);}

  Credit getDebt() {return debt;}
  void setDebt(Credit c) {debt=c;}
};

/* ********************************************************************** */
/*         TODO: make Real-Hash-Table (key any size)                      */   
/* ********************************************************************** */

class BLink {
  friend class GenHashTable;

  Word *key;
  BLink *next;
  void *entry;

public:
  BLink(Word *k,void *e):entry(e),next(NULL),key(k) {}
};

class GenHashTable {
  BLink *live;
  BLink *dead;
  int keysize;
private:
  ght_same(Word *,Word *);
public:
  GenHashTable(int sz):live(NULL),dead(NULL),keysize(sz){}
  Bool findEntry(Word*,void *&);
  void addEntry(Word*,void *);
  void subEntry(void*);
};

Bool GenHashTable::ght_same(Word *a,Word *b)
{
  int i;
  for(i=0;i<keysize;i++)
    {
      if(*a!=*b) return FALSE;
    }
  return TRUE;
}

void GenHashTable::addEntry(Word *key,void* entry)
{
  if(dead==NULL) 
    {
      BLink *bl;
      bl= new BLink(key,entry);
      bl->next=live;
      live=bl;
      return;
    }
  BLink *aux=dead;
  dead=aux->next;
  aux->next=live;
  live=aux;
  aux->entry=entry;
  return;
}

void GenHashTable::subEntry(void *entry)
{
  BLink *aux=live;
  if(aux->entry==entry)
    {
      live=live->next;
      dead->next=aux;
      dead=aux;
      return;
    }
  while(aux->next->entry!=entry) aux=aux->next;
  BLink *bl=aux->next;
  aux->next=bl->next;
  delete bl;
}

Bool GenHashTable::findEntry(Word *key,void *&entry)
{
  BLink *aux=live;    
  while(aux!=NULL)
    {
      if(ght_same(aux->key,key))
	{
	  entry=aux->entry;
	  aux=aux->next;
	  return TRUE;
	}
    }
  return FALSE;
}

/* ********************************************************************** */
/* ********************************************************************** */
/*                  OWNER TABLE STUFF                                     */
/* ********************************************************************** */
/* ********************************************************************** */

/* credit           infinity=  1?----? 
                    max=       01----1
                               .....
		    min=       0----01 
                    dead=      0-----0   Invariant: credit never 0 for live owner
*/


#define INFINITE_CREDIT          -1
#define START_CREDIT_SIZE        ((1<<31) - 1)
#define OWNER_GIVE_CREDIT_SIZE   ((1<<15))
#define BORROW_GIVE_CREDIT_SIZE  ((1<<7))
#define MIN_BORROW_CREDIT_SIZE   2
#define MAX_BORROW_CREDIT_SIZE   8 * OWNER_GIVE_CREDIT_SIZE
#define ASK_CREDIT_SIZE          OWNER_GIVE_CREDIT_SIZE

#define FREE_TAG                 0  

#define DEFAULT_OWNER_TABLE_SIZE   100
#define DEFAULT_BORROW_TABLE_SIZE  100

/* ********************************************************************** */
/*    OB_Entry - common to borrow and owner tables                        */
/* ********************************************************************** */

class OB_Entry {
protected:
  union
  {
    ProtocolObject *object;
    int nextfree;
  }entry;
  Credit credit;

  Bool isFree() {return(credit==FREE_TAG);}

  void makeFree(int next) {credit=FREE_TAG;entry.nextfree=next;}

  void setObject(ProtocolObject *o)  {
    Assert(credit==FREE_TAG);
    entry.object=o;}

  void setObjectCredit(ProtocolObject *o,Credit c)  {
    entry.object=o;
    credit=c;}

  int getNextFree(){
    Assert(credit==FREE_TAG);
    return entry.nextfree;  }

  void setCredit(Credit c) {credit=c;}

  void addToCredit(Credit c) {credit +=c;}

  void subFromCredit(Credit c) {credit -=c;}

  Credit getCredit(){Assert(credit!=FREE_TAG);return credit;}

public:
  ProtocolObject* getObject() {
    Assert(credit!=FREE_TAG);
    return entry.object; }
};
  
/* ********************************************************************** */


class OwnerEntry: public OB_Entry {
friend class OwnerTable;
public:
  void initOwner(ProtocolObject *o){
    setObjectCredit(o,START_CREDIT_SIZE);}

  Bool isFullCredit(){
    if(getCredit()==START_CREDIT_SIZE) return TRUE;return FALSE;}

  void returnCredit(Credit c) {
    if (credit != INFINITE_CREDIT) 
      addToCredit(c);}

  void returnOneCredit() {
    if (credit != INFINITE_CREDIT) 
      addToCredit(1);}

  Credit getSendCredit(){
    Credit c=getCredit();
    if(c < 2*OWNER_GIVE_CREDIT_SIZE) setCredit(INFINITE_CREDIT);
    if(c == INFINITE_CREDIT) return(OWNER_GIVE_CREDIT_SIZE);	
    subFromCredit(OWNER_GIVE_CREDIT_SIZE);
    return(OWNER_GIVE_CREDIT_SIZE);}

  Credit giveMoreCredit(){
    Credit c=getCredit();
    if(c < 2*ASK_CREDIT_SIZE) setCredit(INFINITE_CREDIT);
    if(c == INFINITE_CREDIT) return(ASK_CREDIT_SIZE);	
    subFromCredit(ASK_CREDIT_SIZE);
    return(ASK_CREDIT_SIZE);}
    
}; 

/* ********************************************************** */
/*                    OWNER TABLE                             */
/* ********************************************************** */

#define END_FREE -1

class OwnerTable {
  OwnerEntry* array;
  int size;
  int no_used;
  int nextfree;
  
  void init(int,int);
  void compactify();

public:
 
  OwnerEntry *getOwner(int i)  {return &array[i];}
  
  int getSize() {return size;}

  OwnerTable(int sz) {
    size = sz;
    array = (OwnerEntry*) malloc(size*sizeof(OwnerEntry));
    nextfree = END_FREE;
    no_used=0;
    init(0,sz);
  }

  void gcOwnerTable();

  void resize(int);

  int newOwner(Tertiary*);

  void freeOwnerEntry(int);
};

void OwnerTable::init(int beg,int end)
{
  int i=beg;
  while(i<end){
    array[i].makeFree(i+1);
    i++;}
  i--;
  array[i].makeFree(END_FREE);
  nextfree=beg;
}

void OwnerTable::compactify()
{
  size=size/2;
  array = (OwnerEntry*) realloc(array,size*sizeof(OwnerEntry));
  int *base= &nextfree;
  int i;
  for(i=0;i<=size;i++){
    if(array[i].isFree()) {
      *base=i;
      base=&array[i].entry.nextfree;}}
  return;
}

void OwnerTable::resize(int newsize){
  size = newsize;
  array = (OwnerEntry*) realloc(array,size*sizeof(OwnerEntry));
  init(size, newsize);  
  return;}

int OwnerTable::newOwner(ProtocolObject *obj){
  if(nextfree == END_FREE)
    resize((size*3)/2);
  int index=nextfree;
  nextfree=array[index].getNextFree();
  nextfree= array[index].entry.nextfree;
  OwnerEntry *oe= (OwnerEntry *)&(array[index]);
  oe->initOwner(obj);

  PERDIO_DEBUG1(TABLE,"owner insert: %d",index);
  no_used++;
  return index;}

void OwnerTable::freeOwnerEntry(int i){
  array[i].credit=FREE_TAG;
  array[i].entry.nextfree=nextfree;
  nextfree=i;
  no_used--;
  PERDIO_DEBUG1(TABLE,"owner delete %d",i);
  if((size>DEFAULT_OWNER_TABLE_SIZE) && (3 * no_used < size)) compactify();
  return;}

OwnerTable *ownerTable;

/* ********************************************************************** */
/* ********************************************************************** */
/*                  BORROW TABLE STUFF                                    */
/* ********************************************************************** */
/* ********************************************************************** */


class BorrowEntry: public OB_Entry {
friend class BorrowTable;
private:
  NetAddress netaddr;
  PendLink *pendLink;
  void inDebt(PendLink *);
  Credit pendLinkCredit(Credit c);
  void pendLinkHandle();

public:
  Bool isPending(){
    Assert(!isFree());
    return(pendLink!=NULL);}

  Bool gcCheck() {
    Assert(!isFree());
    Tertiary *tert=getObject();
    if(tert->isMarked()){
      tert->removeMark();
      return FALSE;}
    PERDIO_DEBUG(GC,"no mark found");
    return TRUE;}           // maybe garbage (only if pendLink==NULL); 

  void initBorrow(Credit c,int s,int i){
    Assert(!isFree());
    setCredit(c);
    setObject(NULL);
    pendLink=NULL;
    netaddr.set(s,i);
    return;}
      
  NetAddress* getNetAddress() {
    Assert(!isFree());
    return &netaddr;}
  
  void addPendLink(PendLink*);

  void freeBorrowEntry();

  void addCredit(Credit c){ 
    PERDIO_DEBUG3(CREDIT,"get <%d:%d> credit:%d",getNetAddress()->site,
		  getNetAddress()->index,c);
    Credit cur=getCredit();
    if(cur<MIN_BORROW_CREDIT_SIZE){
      setCredit(MIN_BORROW_CREDIT_SIZE);
      Credit back=pendLinkCredit(cur+c-MIN_BORROW_CREDIT_SIZE);
      pendLinkHandle();
      return;    }
    if(credit>MAX_BORROW_CREDIT_SIZE){
      giveBackCredit(credit-MAX_BORROW_CREDIT_SIZE);
      credit=MAX_BORROW_CREDIT_SIZE;}}
  
  Bool getOneAskCredit() {
    Credit c=getCredit();
    if(c==1) {
      PERDIO_DEBUG(CREDIT,"getOneAskCredit failed");
      return FALSE;
    }
    PERDIO_DEBUG(CREDIT,"getOneAskCredit OK");
    subFromCredit(1);
    return TRUE;}

  Bool getOneCredit() { 
    Credit c=getCredit();
    Assert(c>0);
    if(c <= MIN_BORROW_CREDIT_SIZE) {
      PERDIO_DEBUG(CREDIT,"getOneCredit failed");      
      return FALSE;}
    PERDIO_DEBUG(CREDIT,"getOneCredit OK");
    subFromCredit(1);
    return TRUE; }

  Bool getSmallCredit(Credit &c){
    Credit cur=getCredit();
    if(cur < 2 * MIN_BORROW_CREDIT_SIZE) return FALSE;
    if(cur >  2 * BORROW_GIVE_CREDIT_SIZE) c=BORROW_GIVE_CREDIT_SIZE;
    else{
      if(cur >= 2 * MIN_BORROW_CREDIT_SIZE) c=MIN_BORROW_CREDIT_SIZE;}
    PERDIO_DEBUG1(CREDIT,"give credit:%d",c);
    subFromCredit(c);
    return TRUE;}
  
  void inDebtMain(PendEntry *);
  void inDebtSec(Credit,PendEntry *);
  void moreCredit();

  void giveBackCredit(Credit c);
  Bool fifoCanSend(PendLink *,PendEntry *pe,Bool flag);
};

void BorrowEntry::inDebtMain(PendEntry *pe){
  PendLink *pl=new PendLink(1,pe);
  pl->setTag();
  inDebt(pl);}

void BorrowEntry::inDebtSec(Credit c,PendEntry *pe){
  PendLink *pl=new PendLink(c,pe);
  inDebt(pl);}

void BorrowEntry::inDebt(PendLink *pl){
  if(pendLink==NULL) {
    moreCredit();
    pendLink=pl;
    return;}
  PendLink* aux=pendLink;
  while(aux->next!=NULL) aux=aux->next;
  aux->next=pl;
  return;
}

Credit BorrowEntry::pendLinkCredit(Credit c){
  PendLink *pl= pendLink;
  while(c>0){
    Credit d= pl->getDebt(); 
    if(d>c) {return c;}
    pl->setDebt(0);
    c = c-d;
    (pl->getPend())->dec();
    pl=pl->next;
    if(pl==NULL) return c;
  }
  return c;}

Bool BorrowEntry::fifoCanSend(PendLink *cur,PendEntry *pe,Bool flag){
  if(!pe->isFIFO()) return TRUE;
  BorrowEntry *bb=pe->getBack();
  if(bb==this){
    if((cur->isTagged())&& (!flag)) return FALSE;
    return TRUE;}
  PendLink *pl=pendLink;
  while(TRUE){
    Assert(pl!=NULL);
    if(pl->getPend()==pe) return TRUE;
    if(pl->isTagged()) return FALSE;
    pl=pl->next; }}

void BorrowEntry::pendLinkHandle(){
  PendLink *cur=pendLink;
  PendLink **base=&pendLink;
  PendLink *aux;
  PendEntry *pe;
  Bool flag=TRUE; /* can send */
  Bool msgsent;

  while(TRUE){
    if(cur==NULL) {
      *base=cur;
      return;}
    if(cur->getDebt()!=0){
      *base=cur;
      moreCredit();
      return;}
    pe=cur->getPend();
    if((pe->getrefCount()==0)){
      if(fifoCanSend(cur,pe,flag)){
	PERDIO_DEBUG(DELAYED_MSG_SENT,"pendLinkHandle");
	pe->send();
	msgsent=TRUE;
	delete pe;}
      else{msgsent=FALSE;}}
    else{msgsent=FALSE;}
    if(cur->isTagged() && ((!flag) || (!msgsent))){
      flag=FALSE;
      base= &(cur->next);
      cur=cur->next;}
    else{
      aux=cur->next;
      PERDIO_DEBUG(PENDLINK,"removal");
      delete cur;
      cur=aux;}}
}


void BorrowEntry::moreCredit(){
  if(!getOneAskCredit()) return;
  ByteStream *bs= new ByteStream();
  bs->put(ASK_FOR_CREDIT);
  NetAddress *na = getNetAddress();
  int site = na->site;
  int index = na->index;
  marshallNumber(index,bs);
  marshallMySite(bs);
  PERDIO_DEBUG2(MSG_SENT,"ASK_FOR_CREDIT sd:%d,index:%d",site,index);
  int ret= reliableSend0(site,bs);
  Assert(ret==PROCEED); // TODO
  delete bs;}

void BorrowEntry::giveBackCredit(Credit c){
  ByteStream *bs= new ByteStream();
  bs->put(OWNER_CREDIT);
  NetAddress *na = getNetAddress();
  int site = na->site;
  int index = na->index;
  marshallNumber(index,bs);
  marshallCredit(credit,bs);
  PERDIO_DEBUG3(MSG_SENT,"OWNER_CREDIT sd:%d,index:%d,credit:%d",
		site,index,credit);
  int ret= reliableSend0(site,bs);
  Assert(ret==PROCEED); // TODO
  delete bs;}

void BorrowEntry::freeBorrowEntry(){
  giveBackCredit(credit);
  networkSiteDec((getNetAddress())->site);}

/* ********************************************************************** */
/*         BorrowTable                                                    */
/* ********************************************************************** */

class BorrowTable {           
  int no_used;
  BorrowEntry* array;
  int size;
  int nextfree;
  GenHashTable *hshtbl;

  void init(int,int);
  void compactify();

public:

  BorrowEntry *getBorrow(int i)  {return &array[i];}

  int ptr2Index(BorrowEntry *a) { return(a-array);}

  int getSize() {return size;}   

  BorrowTable(int sz)  {
    size= sz;
    array = (BorrowEntry*) malloc(size *sizeof(BorrowEntry));
    nextfree = END_FREE;
    init(0,sz);
    no_used=0;
    hshtbl = new GenHashTable(sizeof(NetAddress));  }

  void gcBorrowTable();
  
  BorrowEntry *find(NetAddress *na)  {
    void *tmp;
    if(hshtbl->findEntry((Word *)na,tmp)){
      PERDIO_DEBUG(LOOKUP,"borrow OK");
      return((BorrowEntry *) tmp);}
    PERDIO_DEBUG(LOOKUP,"borrow NO");
    return NULL;}

  void resize(int);

  int newBorrow(Credit,int,int);

  void maybeFreeBorrowEntry(int);
};

void BorrowTable::init(int beg,int end)
{
  int i=beg;
  while(i<end){
    array[i].entry.nextfree=i+1;
    array[i].credit=FREE_TAG;
    i++;}
  i--;
  array[i].entry.nextfree=nextfree;
  nextfree=beg;
}

void BorrowTable::compactify(){
  PERDIO_DEBUG2(TABLE,"compactify borrow oldsize:%d no_used:%d",size,no_used);
  size=size/2;
  array = (BorrowEntry*) realloc(array,size*sizeof(OwnerEntry));
  int *base= &nextfree;
  int i;
  for(i=0;i<=size;i++){
    if(array[i].isFree()) {
      *base=i;
      base=&array[i].entry.nextfree;}}
  return;
}

void BorrowTable::resize(int newsize){
  PERDIO_DEBUG2(TABLE,"resize borrow oldsize:%d no_used:%d",size,no_used);
  size = newsize;
  array = (BorrowEntry*) realloc(array,size*sizeof(BorrowEntry));
  init(size, newsize);  /* TODO: ask Ralph */
  return;}

int BorrowTable::newBorrow(Credit c,int sd,int off){
  if(nextfree == END_FREE)
    resize((size*3)/2);
  int index=nextfree;
  nextfree= array[index].entry.nextfree;
  BorrowEntry* oe = &(array[index]);
  oe->initBorrow(c,sd,off);
  no_used++;
  PERDIO_DEBUG1(TABLE,"borrow insert: %d",index);
  return index;}

void BorrowTable::maybeFreeBorrowEntry(int index){
  BorrowEntry *b = &(array[index]);
  if(b->isPending()) return; /* cannot remove as msgs pending on credit */
  array[index].credit=FREE_TAG;
  array[index].entry.nextfree=nextfree;
  nextfree=index;
  hshtbl->subEntry((void *) b); 
  b->freeBorrowEntry();
  array[index].entry.nextfree=nextfree;
  array[index].credit=FREE_TAG;
  nextfree=index;
  no_used--;
  PERDIO_DEBUG1(TABLE,"borrow delete: %d",index);
  if((size>DEFAULT_BORROW_TABLE_SIZE) && (3 * no_used < size)) compactify();
  return;}

BorrowTable *borrowTable;

/**********************************************************************/
/**********************************************************************/
/*                      GARBAGE COLLECTION
/**********************************************************************/
/**********************************************************************/

/* OBS: ---------- interface to gc.cc ----------*/

void gcOwnerTable() { ownerTable->gcOwnerTable();}
void gcBorrowTable(){ borrowTable->gcBorrowTable();}

void OwnerTable::gcOwnerTable()
{
  PERDIO_DEBUG(GC,"owner gc");
  int i;
  for(i=0;i<=size;i++)
    {
      OwnerEntry* o = ownerTable->getOwner(i);
      if(!(o->isFree()))
	{ 
	  if(o->isFullCredit()) 
	    {
	      Tertiary *te = o->entry.object;
	      switch(te->getType()){
	      case Co_Port:{
		Assert((te->getTertType())==Te_Manager);
		(tert2PortManager(te))->localize();
		break;}
	      default:
		Assert(0);
		break;
	      }
	      ownerTable->freeOwnerEntry(i);
	    }
	  else
	    {
	      o->entry.object = 
		(ProtocolObject*) o->entry.object->gcConstTerm();
	    }
	}
    }    
  return;
}

/* OBSERVE - this must done at the end of other gc */
void BorrowTable::gcBorrowTable()
{
  PERDIO_DEBUG(GC,"borrow gc");
  BorrowEntry *b;
  int i;
  for(i=0;i<=size;i++)
    {
      b=borrowTable->getBorrow(i);
      if(!(b->isFree())){
	if(b->gcCheck()) borrowTable->maybeFreeBorrowEntry(i);}
    }
}

/**********************************************************************/
/**********************************************************************/
/*                      GLOBALIZING                                   */
/**********************************************************************/
/**********************************************************************/

PortManager *PortLocal::globalize()
{ 
  PERDIO_DEBUG(GLOBALIZING,"globalizing port");
  Assert(sizeof(PortManager)==sizeof(PortLocal));
  PortManager *pm= (PortManager*) this;
  pm->setTertType(Te_Manager);
  int i = ownerTable->newOwner(pm);
  pm->setIndex(i);
  return pm;
}

/**********************************************************************/
/**********************************************************************/
/*                      LOCALIZING
/**********************************************************************/
/**********************************************************************/

PortLocal *PortManager::localize()
{ 
  PERDIO_DEBUG(GLOBALIZING,"localizing port");
  Assert(sizeof(PortManager)==sizeof(PortLocal));
  PortLocal *pl= (PortLocal*) this;
  Assert(!isMarked());
  pl->setTertType(Te_Local);
  pl->setBoard(am.rootBoard);
  return pl;
}

/**********************************************************************/
/**********************************************************************/
/*                            MARSHALLING STUFF                       */
/**********************************************************************/
/**********************************************************************/



typedef enum {SMALLINTTAG, BIGINTTAG, FLOATTAG, ATOMTAG, 
	      RECORDTAG, TUPLETAG, LISTTAG, REFTAG, 
	      OWNERTAG, BORROWTAG, 
	      PORTTAG} MarshallTag;

int unmarshallWithDest(char *buf, int len, OZ_Term *t);
void unmarshallNoDest(char *buf, int len, OZ_Term *t);

/**********************************************************************/
/*                Help-Classes for Marshalling                        */
/**********************************************************************/


void ByteStream::resize()
{
  PERDIO_DEBUG(AUXILLARY,"resizing bytestream");
  Assert(size>0);
  int oldsize = size;
  char *oldarray = array;
  char *oldpos = pos;
  size = (size*3)/2;
  array = new char[size];
  pos = array;
  for (char *s=oldarray; s<oldpos;) {
    *pos++ = *s++;
  }
  delete oldarray;
}

int refCounter = 0;

class RefTable {
  OZ_Term *array;
  int size;
public:
  RefTable()
  {
    size = 100;
    array = new OZ_Term[size];
  }
  OZ_Term get(int i)
  {
    Assert(i<size);
    return array[i];
  }
  void set(int pos, OZ_Term val)
  {
    if (pos>=size) 
      resize(pos);
    array[pos] = val;
  }
  void resize(int newsize)
  {
    int oldsize = size;
    OZ_Term  *oldarray = array;
    while(size <= newsize) {
      size = (size*3)/2;
    }
    array = new OZ_Term[size];
    for (int i=0; i<oldsize; i++) {
      array[i] = oldarray[i];
    }
    delete oldarray;
  }
};

RefTable *refTable;

class RefTrail: public Stack {
public:
  RefTrail() : Stack(200,Stack_WithMalloc) { }
  void trail(OZ_Term *t)
  {
    push(t);
    push(ToPointer(*t));
  }
  void unwind()
  {
    while(!isEmpty()) {
      OZ_Term oldval = ToInt32(pop());
      OZ_Term *loc = (OZ_Term*) pop();
      *loc = oldval;
    }
  }
};

RefTrail *refTrail;

class DebtRec:public Stack {
public:
  DebtRec():Stack(200,Stack_WithMalloc) {}

  void handler(PendEntry *pe){
    BorrowEntry *b;
    Assert(!isEmpty());
    while(!isEmpty()) {
      b=(BorrowEntry *) pop();
      b->inDebtSec(MIN_BORROW_CREDIT_SIZE,pe);}
    return;}
  
  void debtPush(int i){ 
    PERDIO_DEBUG1(DEBT,"push %d",i);
    push((void *)i);
  }
};


DebtRec* debtRec;


/**********************************************************************/
/*                 MARSHALLING/UNMARSHALLING  GROUND STRUCTURES       */
/**********************************************************************/

const int intSize = sizeof(int32); 

inline
void marshallNumber(unsigned int i, ByteStream *bs)
{
  for (int k=0; k<intSize; k++) {
    bs->put(i&0xFF);
    i = i>>8;}
}

inline
int unmarshallNumber(ByteStream *bs)
{
  unsigned int i1 = bs->get();
  unsigned int i2 = bs->get();
  unsigned int i3 = bs->get();
  unsigned int i4 = bs->get();
  return (int) (i1 + (i2<<8) + (i3<<16) + (i4<<24));
}

class DoubleConv {
public:
  union {
    int32 i[2];
    double d;
  } u;
};

inline
void marshallFloat(double d, ByteStream *bs)
{
  static DoubleConv dc;
  dc.u.d = d;
  marshallNumber(dc.u.i[0],bs);
  marshallNumber(dc.u.i[1],bs);
}

inline
double unmarshallFloat(ByteStream *bs)
{
  static DoubleConv dc;
  dc.u.i[0] = unmarshallNumber(bs);
  dc.u.i[1] = unmarshallNumber(bs);
  return dc.u.d;
}

inline
char *unmarshallString(ByteStream *bs)
{
  int i = unmarshallNumber(bs);
  char *ret = new char[i+1];
  for (int k=0; k<i; k++) {
    ret[k] = bs->get();
  }
  ret[i] = '\0';
  return ret;
}

inline
void marshallString(char *s, ByteStream *bs)
{
  marshallNumber(strlen(s),bs);
  while(*s) {
    bs->put(*s);
    s++;  }
}

/**********************************************************************/
/*            MARSHALLING/UNMARSHALLING NETWORK ADDRESSES             */
/**********************************************************************/

void unmarshallTerm(ByteStream*,OZ_Term*);

inline
void marshallCredit(Credit credit,ByteStream *bs){
  Assert(sizeof(Credit)==1);
  marshallNumber(credit,bs);}

inline
Credit unmarshallCredit(ByteStream *bs){
  Assert(sizeof(Credit)==1);
  return(unmarshallNumber(bs));}

inline
void marshallSite(int sd,ByteStream *bs){
  char *host;
  int port, timestamp;
  getSite(sd,host,port,timestamp);
  marshallString(host,bs);
  marshallNumber(port,bs);
  marshallNumber(timestamp,bs);}

inline
void marshallMySite(ByteStream *bs){
  marshallSite(localSite,bs);}

inline
void marshallNetAddress2(int site,int index,ByteStream *bs){
  marshallSite(site,bs);
  marshallNumber(index,bs);}

inline
void marshallNetAddress(NetAddress *a, ByteStream *bs){
  marshallNetAddress2(a->site,a->index,bs);}

inline
int unmarshallSiteId(ByteStream *bs){
  char *host = unmarshallString(bs);
  int port = unmarshallNumber(bs);
  int timestamp = unmarshallNumber(bs);
  return lookupSite(host,port,timestamp);}

void marshallOwned(int i,ByteStream *bs){
  PERDIO_DEBUG1(MARSHALL,"owned %d",i);
  OwnerEntry *o=ownerTable->getOwner(i);
  marshallNetAddress2(localSite,i,bs);
  marshallNumber(o->getSendCredit(),bs);}

void marshallBorrowed(int i,ByteStream *bs,DebtRec *dr){
  PERDIO_DEBUG1(MARSHALL,"borrowed %d",i);
  BorrowEntry *b=borrowTable->getBorrow(i);
  marshallNetAddress(b->getNetAddress(),bs);
  Credit cred;
  if(b->getSmallCredit(cred)) {
    marshallCredit(cred,bs);
    return;  }
  dr->debtPush(i);
  return;}

Bool unmarshallBorrow(ByteStream *bs,BorrowEntry *&b,int &ib){
  int sd = unmarshallSiteId(bs);
  int index = unmarshallNumber(bs);
  PERDIO_DEBUG2(UNMARSHALL,"borrowed site:%d index:%d",sd,index);
  Credit cred = unmarshallCredit(bs);
  if (sd==localSite) {
    error("should not happen");  }
  NetAddress na = NetAddress(sd,index); /* ASK RALPH */
  b = borrowTable->find(&na);
  if (b!=NULL) {
    PERDIO_DEBUG(UNMARSHALL,"borrowed hit");
    b->addCredit(cred);
    return TRUE;  }
  ib=borrowTable->newBorrow(cred,sd,index);
  PERDIO_DEBUG(UNMARSHALL,"borrowed miss");
  return FALSE;  }

void unmarshallNoDest(char *buf, int len, OZ_Term *t){
  ByteStream *bs = new ByteStream(buf,len);
  OZ_Term ret;
  refCounter = 0;
  unmarshallTerm(bs,t);
  bs->endCheck();
  delete bs;}

int unmarshallWithDest(char *buf, int len, OZ_Term *t){
  ByteStream *bs = new ByteStream(buf,len);
  OZ_Term ret;
  refCounter = 0;
  int dest = unmarshallNumber(bs);
  unmarshallTerm(bs,t);
  bs->endCheck();
  delete bs;
  return dest;}

/**********************************************************************/
/*                 MARSHALLING terms                                  */
/**********************************************************************/


#define CheckCycle(expr)			\
  {						\
    OZ_Term t = expr;				\
    if (!isRef(t) && tagTypeOf(t)==GCTAG) {	\
      PERDIO_DEBUG(MARSHALL,"circular");	\
      bs->put(REFTAG);				\
      marshallNumber(t>>tagSize,bs);		\
      return;					\
    }						\
  }

inline
void trailCycle(OZ_Term *t)
{					
  refTrail->trail(t);
  *t = (refCounter<<tagSize)|GCTAG;
  refCounter++;
}

void marshallTerm(OZ_Term t, ByteStream *bs, DebtRec *dr)
{
  OZ_Term *args;
  int argno;

loop:
  t = deref(t);
  switch(tagTypeOf(t)) {

  case SMALLINT:
    bs->put(SMALLINTTAG);
    marshallNumber(smallIntValue(t),bs);
    return;

  case OZFLOAT:
    bs->put(FLOATTAG);
    marshallFloat(tagged2Float(t)->getValue(),bs);
    return;

  case BIGINT:
    bs->put(BIGINTTAG);
    marshallString(toC(t),bs);
    return;

  case LITERAL:
    bs->put(ATOMTAG);
    marshallString(tagged2Literal(t)->getPrintName(),bs);
    return;

  case LTUPLE:
    {
      LTuple *l = tagged2LTuple(t);
      CheckCycle(*l->getRef());
      bs->put(LISTTAG);
      argno = 2;
      args  = l->getRef();
      goto processArgs;
    }

  case SRECORD:
    {
      SRecord *rec = tagged2SRecord(t);
      CheckCycle(*rec->getRef()); /* TODO mark instead of getRef ??*/
      if (rec->isTuple()) {
	bs->put(TUPLETAG);
	marshallNumber(rec->getTupleWidth(),bs);
      } else {
	bs->put(RECORDTAG);
	marshallTerm(rec->getArityList(),bs,dr);
      }
      marshallTerm(rec->getLabel(),bs,dr);
      argno = rec->getWidth();
      args  = rec->getRef();
      goto processArgs;
    }

  case OZCONST:
    {
      Tertiary *ptr = tagged2Tert(t);
      switch(ptr->getType()) {
      case Co_Port:
	{
	  bs->put(BORROWTAG);
	  if(isProxy(ptr)) {
	      PERDIO_DEBUG(MARSHALL,"port proxy");
	      PortProxy *pp=(PortProxy*) ptr;
	      marshallBorrowed(pp->getIndex(),bs,dr);
	      bs->put(PORTTAG);
	      return;}
	  PortManager *pm;
	  if(isLocal(ptr)){
	    PERDIO_DEBUG(MARSHALL,"port local");
	    pm=((PortLocal *)ptr)->globalize();}
	  else {
	    PERDIO_DEBUG(MARSHALL,"port manager");
	    pm= (PortManager *)ptr;
	    CheckCycle(*(pm->getRef()));}
	  trailCycle(pm->getRef());
	  marshallOwned(pm->getIndex(),bs);
	  bs->put(PORTTAG);
	  return;
	}	    
      default:
	warning("Cannot marshall generic %s",toC(t));
	marshallTerm(nil(),bs,dr);
	return;
      }
    }
    // no break here
  default:
    if (isAnyVar(t)) {
      warning("Cannot marshall variables");
    } else {
      warning("Cannot marshall %s",toC(t));
    }
    marshallTerm(nil(),bs,dr);
    return;
  }

processArgs:
  OZ_Term arg0 = *args;
  trailCycle(args);
  marshallTerm(arg0,bs,dr);
  args++;
  if (argno == 1) return;
  for(int i=1; i<argno-1; i++) {
    marshallTerm(*args,bs,dr);
    args++;
  }
  // tail recursion optimization
  t = *args;
  goto loop;
}



/**********************************************************************/
/*                 UNMARSHALLING terms                                */
/**********************************************************************/

void unmarshallTerm(ByteStream *bs, OZ_Term *ret)
{
  int argno;
loop:
  MarshallTag tag = (MarshallTag) bs->get();

  switch(tag) {

  case SMALLINTTAG: *ret = OZ_int(unmarshallNumber(bs)); return;
  case FLOATTAG:    *ret = OZ_float(unmarshallFloat(bs)); return;

  case ATOMTAG:     
    {
      char *aux = unmarshallString(bs);
      *ret = OZ_atom(aux);
      delete aux;
      return;
    }

  case BIGINTTAG:
    {
      char *aux = unmarshallString(bs);
      *ret = OZ_CStringToNumber(aux);
      delete aux;
      return;
    }

  case LISTTAG:
    {
      OZ_Term head, tail;
      argno = 2;
      LTuple *l = new LTuple();
      *ret = makeTaggedLTuple(l);
      refTable->set(refCounter++,*ret);
      ret = l->getRef();
      goto processArgs;
    }
  case TUPLETAG:
    {
      argno = unmarshallNumber(bs);
      TaggedRef label;
      unmarshallTerm(bs,&label);
      SRecord *rec = SRecord::newSRecord(label,argno);
      *ret = makeTaggedSRecord(rec);
      refTable->set(refCounter++,*ret);
      ret = rec->getRef();
      goto processArgs;      
    }

  case RECORDTAG:
    {
      TaggedRef arity;
      unmarshallTerm(bs,&arity);
      argno = length(arity);
      TaggedRef label;
      unmarshallTerm(bs,&label);
      SRecord *rec = SRecord::newSRecord(label,mkArity(arity));
      *ret = makeTaggedSRecord(rec);
      refTable->set(refCounter++,*ret);
      ret = rec->getRef();
      goto processArgs;      
    }

  case REFTAG:
    {
      PERDIO_DEBUG(UNMARSHALL,"circular");
      int i = unmarshallNumber(bs);
      *ret = refTable->get(i);
      return;
    }

  case BORROWTAG:
    {
      BorrowEntry *b;
      int index;

      if(unmarshallBorrow(bs,b,index)) 
	{
	  *ret=makeTaggedConst(b->getObject());
	  return;}
      tag = (MarshallTag) bs->get();
      switch(tag)
	{
	case PORTTAG:
	  {
	    PERDIO_DEBUG(UNMARSHALL,"port proxy");
	    PortProxy *pp=new PortProxy(index);
	    *ret= makeTaggedConst(pp);
	    return;}
	default:
    /* TODO - proper exception */
	  printf("unmarshall: unexpected tag: %d\n",tag);
	  *ret = nil();
	  return;}
    }
  case OWNERTAG:
  default:
    printf("unmarshall: unexpected tag: %d\n",tag); 
    *ret = nil();
    return;
  }

processArgs:
  for(int i=0; i<argno-1; i++) {
    unmarshallTerm(bs,ret++);
  }
  // tail recursion optimization
  goto loop;
}


/**********************************************************************
**********************************************************************
                            MAIN RECEIVE                            
SITESEND       term                                             
PORTSEND       index  term             implicit 1 credit        
GIVE_CREDIT    index  credit           
ASK_FOR_CREDIT index  b_n_addr         implicit 1 credit        
                                                                
    term :=      ....                                                
                 BORROWTAG netaddr d-term                           
	    
    d-term :=    PORTTAG

    netaddr:=    b_n_addr index credit
    b_n_addr:=   host port timestamp
            
            
**********************************************************************
**********************************************************************/

OZ_Term ozport=0;

void siteReceive(char *msg,int len)
{
  OZ_Term recvPort;

  switch (msg[0]) {
  case SITESEND:
    {
      PERDIO_DEBUG(MSG_RECEIVED,"SITESEND");
      OZ_Term t;
      unmarshallNoDest(msg+1,len-1,&t);
      if (!t) {
	if (ozconf.debugPerdio) {
	  printf("siteReceive: message SITESEND:");
	  printBytes(msg,len);
	}
	OZ_fail("siteReceive: SITESEND unmarshall failed\n");
      }
      if (ozconf.debugPerdio) {
	printf("siteReceive: SITESEND '%s'\n",OZ_toC(t,10,10));
      }
      sendPort(ozport,t);
      break;
    }
  case PORTSEND:    /* PORTSEND index term */
    {
      PERDIO_DEBUG(MSG_RECEIVED,"PORTSEND");
      OZ_Term t;
      int portIndex;
      portIndex=unmarshallWithDest(msg+1,len-1,&t);
      if (!t) {
	if (ozconf.debugPerdio) {
	  printf("siteReceive: message PORTSEND:");
	  printBytes(msg,len);
	}
	OZ_fail("siteReceive: PORTSEND unmarshall failed\n");
      }
      if (ozconf.debugPerdio) {
	printf("siteReceive: PORTSEND '%s'\n",OZ_toC(t,10,10));
      }
      OwnerEntry *oe=ownerTable->getOwner(portIndex);
      oe->returnOneCredit();
      Tertiary *tert= oe->getObject();
      if(!(tert->checkTertiary(Co_Port,Te_Manager)))
	{OZ_fail("siteReceive: PORTSEND not to port\n");}
      sendPort(makeTaggedConst(tert),t);
      break;
      }
  case ASK_FOR_CREDIT:
    {
      PERDIO_DEBUG(MSG_RECEIVED,"ASK_FOR_CREDIT");
      ByteStream *bs=new ByteStream(msg+1,len-1);
      int na_index=unmarshallNumber(bs);
      OwnerEntry *o;
      int rsite=unmarshallSiteId(bs);
      int rindex= unmarshallNumber(bs);
      bs->endCheck();
      o->returnOneCredit();
      delete bs;
      Credit c= o->giveMoreCredit();
      ByteStream *bs1=new ByteStream();
      bs1->put(BORROW_CREDIT);
      marshallNumber(rindex,bs1);
      marshallCredit(c,bs1);
      PERDIO_DEBUG1(MSG_SENT,"BORROW_CREDIT %d",c);
      int ret=reliableSend0(rsite,bs1);
      Assert(ret==PROCEED); // TODO
      delete bs1;
      break;
    }
  case OWNER_CREDIT:  
    {
      ByteStream *bs=new ByteStream(msg+1,len-1);
      int index=unmarshallNumber(bs);
      Credit c=unmarshallCredit(bs);
      PERDIO_DEBUG1(MSG_RECEIVED,"OWNER_CREDIT %d",c);
      bs->endCheck();
      (ownerTable->getOwner(index))->returnCredit(c);
      break;
    }
  case BORROW_CREDIT:  
    {
      ByteStream *bs=new ByteStream(msg+1,len-1);
      int bindex=unmarshallNumber(bs);
      Credit c=unmarshallCredit(bs);
      PERDIO_DEBUG1(MSG_RECEIVED,"BORROW_CREDIT %d",c);
      bs->endCheck();
      (borrowTable->getBorrow(bindex))->addCredit(c);
      break;
    }
  default:
    OZ_fail("siteReceive: unknown message %d\n",msg[0]);
    printf("\n--\n%s\n--\n",msg);
    break;
  }
}


/* ********************************************************************** */
/* ********************************************************************** */
/*              BUILTINS                                                  */
/* ********************************************************************** */
/* ********************************************************************** */

void domarshallTerm(OZ_Term t, ByteStream *bs)
{
  refCounter = 0;
  marshallTerm(t,bs,debtRec);
  refTrail->unwind();
}

inline
int reliableSend0(int sd, ByteStream *bs){
  int ret=reliableSend(sd,bs->getPtr(),bs->getLen());
  if (ret != 0) {
    return OZ_raiseC("ip",2,OZ_atom("send"),
		     OZ_atom(OZ_unixError(lastIpError())));
  }
  return PROCEED;
}


/* engine-interface */
int remoteSend(PortProxy *p, TaggedRef msg) {
  BorrowEntry *b= borrowTable->getBorrow(p->getIndex());
  NetAddress *na = b->getNetAddress();
  ByteStream *bs = new ByteStream();
  PendEntry *pe;
  int site = na->site;
  int index = na->index;

  if(!(b->getOneCredit())){
      PERDIO_DEBUG(DEBT_MAIN,"remoteSend");
      pe= new PendEntry(bs,site,b);
      b->inDebtMain(pe);}
  else pe=NULL;

  bs->put(PORTSEND);                    
  marshallNumber(index,bs);               
  domarshallTerm(msg,bs);
  if(pe==NULL){
    if(debtRec->isEmpty()){
      int ret = reliableSend0(site,bs);
      delete bs;
      return ret;}
    PERDIO_DEBUG(DEBT_SEC,"remoteSend");
    pe=new PendEntry(bs,site);
    debtRec->handler(pe);
    return PROCEED;}
  if(debtRec->isEmpty()){
    return PROCEED;}
  debtRec->handler(pe);  
  return PROCEED;
}

int sitesend(int sd,OZ_Term t){
  ByteStream *bs = new ByteStream();
  bs->put(SITESEND);
  domarshallTerm(t,bs);
  if(debtRec->isEmpty()){
    int ret = reliableSend0(sd,bs);
    delete bs;
    return ret;
  }
  PendEntry *pe;
  pe=new PendEntry(bs,sd);
  debtRec->handler(pe);
  return PROCEED;
}

/* ********************************************************************** */
/*              BUILTINS themselves                                       */
/* ********************************************************************** */

OZ_C_proc_begin(BIreliableSend,2)
{
  PERDIO_DEBUG(SEND_EMIT,"reliable send");
  OZ_declareIntArg(0,sd);
  OZ_declareArg(1,value);

  int ret = sitesend(sd,value);
  if (ret == PROCEED) {
    PERDIO_DEBUG(SEND_DONE,"reliable send");
  }
  return ret;
}
OZ_C_proc_end


OZ_C_proc_begin(BIunreliableSend,2)
{
  PERDIO_DEBUG(SEND_EMIT,"unreliable send");
  OZ_declareIntArg(0,sd);
  OZ_declareArg(1,value);

  ByteStream *bs=new ByteStream();
  bs->put(SITESEND);
  domarshallTerm(value,bs);
  if(debtRec->isEmpty()){
    int ret=unreliableSend(sd,bs->getPtr(),bs->getLen());
    delete bs;
    if(ret==0){
      PERDIO_DEBUG(SEND_DONE,"reliable send");
      return PROCEED;}
    return OZ_raiseC("unreliableSend",1);
  }
  PERDIO_DEBUG(DEBT,"unreliableSend");
  PendEntry *pe;
  pe=new PendEntry(bs,sd);
  debtRec->handler(pe);
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(BIstartSite,2)
{
  OZ_declareIntArg(0,p);
  OZ_declareArg(1,stream);

  if (ozport!=0) {
    return OZ_raise(OZ_mkTupleC("perdio",1,OZ_atom("site already started")));
  }
  ozport = makeTaggedConst(new PortWithStream(am.rootBoard, stream));
  if (ipInit(p,siteReceive) < 0) {
    ozport=0;
    return OZ_raiseC("ip",2,OZ_atom("ip init failed"),
		     OZ_atom(OZ_unixError(lastIpError())));
  }
  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIlookupSite,4)
{
  OZ_declareVirtualStringArg(0,host);
  OZ_declareIntArg(1,port);
  OZ_declareIntArg(2,timestamp);
  OZ_declareArg(3,out);

  return OZ_unifyInt(out,lookupSite(host,port,timestamp));
}
OZ_C_proc_end

BIspec perdioSpec[] = {
  {"reliableSend",   2, BIreliableSend, 0},
  {"unreliableSend", 2, BIunreliableSend, 0},
  {"startSite",      2, BIstartSite, 0},
  {"lookupSite",     4, BIlookupSite, 0},
  {0,0,0,0}
};


void BIinitPerdio()
{
  BIaddSpec(perdioSpec);
  
  refTable = new RefTable();
  refTrail = new RefTrail();
  ownerTable = new OwnerTable(DEFAULT_OWNER_TABLE_SIZE);
  borrowTable = new BorrowTable(DEFAULT_BORROW_TABLE_SIZE);
  debtRec= new DebtRec(); 
  /*  pendLinkManager = new FreeListManager(sizeof(PendLink));
  pendEntryManager = new FreeListManager(sizeof(PendEntry));*/
  /* TODO: gname-table */
#ifdef DEBUG_PERDIO
  dvset();
#endif
}

#endif
