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
#include "perdio.hh"

#include "oz.h"
#include "am.hh"


/**********************************************************************/

#define ProtocolObject ConstTerm
#define Proxy          ConstTerm

class NetAddress {
public:
  int site;
  int index;
  NetAddress(int s, int i) : site(s), index(i) {}
  Bool same(int s, int i) { return s==site && i==index; }
  Bool isLocal() { return site==localSite; }
  int getCredit();
  void giveCredit(int c);
};

Bool isLocalAddress(NetAddress *na)
{
  return na->isLocal();
}

class BorrowerTableEntry {
public:
  long credit;
  Proxy *proxy;
  void giveCredit(int c) { credit += c; }
  int getCredit()
  {
    if (credit <= 2) {
      Assert(0);
    }
    
    int ret = credit/3;
    credit -= ret;
    Assert(credit>=0);
    return ret;
  }
};

class BorrowerTable {
  NetAddress *addr;
  BorrowerTableEntry *entry;
  BorrowerTable *next;
public:

  BorrowerTable(NetAddress *na, Proxy *p, BorrowerTable *nxt):
    next(nxt)
  {
    addr = na;
    entry = new BorrowerTableEntry();
    entry->proxy  = p;
    entry->credit = 0;
    
  }

  BorrowerTableEntry *find(int s, int index)
  {
    BorrowerTable *aux = this;
    while(aux) {
      if (aux->addr->same(s,index))
	return aux->entry;
      aux = aux->next;
    }
    return NULL;
  }
  BorrowerTable *add(NetAddress *na, Proxy *p)
  {
    return new BorrowerTable(na,p,this);
  }
  
};

extern BorrowerTable *borrowerTable;

const int amountCredit = 100000;

class OwnerTableEntry {
  long credit;
public:
  ProtocolObject *object;
  Proxy *proxy;

  void init(ProtocolObject *o, Proxy *p)
  {
    object = o;
    proxy  = p;
  }
  void giveCredit(int c) 
  {
    if ( credit != 1)
      credit += c; 
  }

  int getCredit()
  {
    if (credit == 1 || credit-amountCredit > 0) {
      /* overflow set credit=1 --> no GC anymore */
      credit = 1;
    }
    credit -= amountCredit;
    return amountCredit;
  }
};

class OwnerTable {
  OwnerTableEntry *array;
  int size;
  int nextfree;
public:
  OwnerTable(int sz) 
  {
    size = sz;
    array = (OwnerTableEntry*) malloc(size*sizeof(OwnerTableEntry));
    nextfree = 0;
  }

  void resize(int newsize)
  {
    size = newsize;
    array = (OwnerTableEntry*) realloc(array,size*sizeof(OwnerTableEntry));
  }

  OwnerTableEntry *get(int i)
  {
    return (i>=nextfree) ? (OwnerTableEntry *) NULL : array+i;
  }

  int newOTEntry(ProtocolObject *obj, Proxy *pr)
  {
    if (nextfree>=size) {
      resize((size*3)/2);
    }
   array[nextfree].init(obj,pr);
   return nextfree++;
  }
};

extern OwnerTable *ownerTable;

inline
int NetAddress::getCredit() 
{
  if (isLocal()) {
    return ownerTable->get(index)->getCredit();
  }
  BorrowerTableEntry *b = borrowerTable->find(site,index);
  Assert(b!=NULL);
  return b->getCredit();
}


inline
void NetAddress::giveCredit(int c) 
{
  if (isLocal()) {
    ownerTable->get(index)->giveCredit(c);
  } else {
    BorrowerTableEntry *b = borrowerTable->find(site,index);
    Assert(b!=NULL);
    b->giveCredit(c);
  }
}

/**********************************************************************/

typedef enum {SMALLINTTAG, BIGINTTAG, FLOATTAG, ATOMTAG, 
	      RECORDTAG, TUPLETAG, LISTTAG, REFTAG, PORTTAG} MarshalTag;


void unmarshalSend(char *buf, int len, int *port, OZ_Term *t);

OZ_Term ozport=0;
void siteReceive(char *msg,int len)
{
  OZ_Term recvPort;

  switch (msg[0]) {
  case GSEND:
    {
      OZ_Term t;
      int portIndex;
      unmarshalSend(msg+1,len-1,&portIndex,&t);
      if (portIndex==-1) {
	recvPort = ozport;
      } else {
	Port *p = (Port *) ownerTable->get(portIndex)->object;
	recvPort = makeTaggedConst(p);
      }
      if (!t) {
	if (ozconf.debugPerdio) {
	  printf("siteReceive: message GSEND:");
	  printBytes(msg,len);
	}
	OZ_fail("siteReceive: GSEND unmarshal failed\n");
      }

      if (ozconf.debugPerdio) {
	printf("siteReceive: GSEND '%s'\n",OZ_toC(t,10,10));
      }
      OZ_send(recvPort,t);
      break;
    }
  default:
    OZ_fail("siteReceive: unknown message %d\n",msg[0]);
    printf("\n--\n%s\n--\n",msg);
    break;
  }
}

/*
 * Marshal
 */

#define BSEOF (unsigned int) -1

class ByteStream {
  char *array;
  int size;
  char *pos;
  int len;
public:
  ByteStream()
  {
    len = ipHeaderSize;
    size = 1000;
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


void ByteStream::resize()
{
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


/**********************************************************************/

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

BorrowerTable *borrowerTable = NULL;
OwnerTable *ownerTable = NULL;

/**********************************************************************/

NetAddress *exportPort(Port *p)
{
  NetAddress *addr=p->getAddress();
  if (addr==NULL) {
    int index = ownerTable->newOTEntry(p,NULL);
    addr=new NetAddress(localSite,index);
    p->setAddress(addr);
  }
  return addr;
}

/**********************************************************************/

const int intSize = sizeof(int32); 

inline
void marshalNumber(unsigned int i, ByteStream *bs)
{
  for (int k=0; k<intSize; k++) {
    bs->put(i&0xFF);
    i = i>>8;
  }
}


inline
int unmarshalNumber(ByteStream *bs)
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
void marshalFloat(double d, ByteStream *bs)
{
  static DoubleConv dc;
  dc.u.d = d;
  marshalNumber(dc.u.i[0],bs);
  marshalNumber(dc.u.i[1],bs);
}


inline
double unmarshalFloat(ByteStream *bs)
{
  static DoubleConv dc;
  dc.u.i[0] = unmarshalNumber(bs);
  dc.u.i[1] = unmarshalNumber(bs);
  return dc.u.d;
}

inline
char *unmarshalString(ByteStream *bs)
{
  int i = unmarshalNumber(bs);
  char *ret = new char[i+1];
  for (int k=0; k<i; k++) {
    ret[k] = bs->get();
  }
  ret[i] = '\0';
  return ret;
}


inline
void marshalString(char *s, ByteStream *bs)
{
  marshalNumber(strlen(s),bs);
  while(*s) {
    bs->put(*s);
    s++;
  }
}


inline
void marshalNetAddress(NetAddress *a, ByteStream *bs)
{
  char *host;
  int port, timestamp;
  getSite(a->site,host,port,timestamp);
  marshalString(host,bs);
  marshalNumber(port,bs);
  marshalNumber(timestamp,bs);
  marshalNumber(a->index,bs);
}


inline
ProtocolObject *unmarshalNetAddress(MarshalTag t, ByteStream *bs)
{
  char *host = unmarshalString(bs);
  int port = unmarshalNumber(bs);
  int timestamp = unmarshalNumber(bs);
  int sd = lookupSite(host,port,timestamp);

  int index = unmarshalNumber(bs);

  if (sd==localSite) {
    return ownerTable->get(index)->object;
  }
  BorrowerTableEntry *b = borrowerTable->find(sd,index);
  if (b) {
    return b->proxy;
  }
  
  if (t==PORTTAG) {
    NetAddress *addr = new NetAddress(sd,index);
    Port *ret = new Port(addr);
    borrowerTable = borrowerTable->add(addr,ret);
    return ret;
  } 

  Assert(0);
  return NULL;
}


#define CheckCycle(expr)			\
  {						\
    OZ_Term t = expr;				\
    if (!isRef(t) && tagTypeOf(t)==GCTAG) {	\
      bs->put(REFTAG);				\
      marshalNumber(t>>tagSize,bs);		\
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




void marshalTerm(OZ_Term t, ByteStream *bs)
{
  OZ_Term *args;
  int argno;

loop:
  t = deref(t);
  switch(tagTypeOf(t)) {

  case SMALLINT:
    bs->put(SMALLINTTAG);
    marshalNumber(smallIntValue(t),bs);
    return;

  case OZFLOAT:
    bs->put(FLOATTAG);
    marshalFloat(tagged2Float(t)->getValue(),bs);
    return;

  case BIGINT:
    bs->put(BIGINTTAG);
    marshalString(toC(t),bs);
    return;

  case LITERAL:
    bs->put(ATOMTAG);
    marshalString(tagged2Literal(t)->getPrintName(),bs);
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
      CheckCycle(*rec->getRef());
      if (rec->isTuple()) {
	bs->put(TUPLETAG);
	marshalNumber(rec->getTupleWidth(),bs);
      } else {
	bs->put(RECORDTAG);
	marshalTerm(rec->getArityList(),bs);
      }
      marshalTerm(rec->getLabel(),bs);
      argno = rec->getWidth();
      args  = rec->getRef();
      goto processArgs;
    }

  case OZCONST:
    {
      if (isPort(t)) {
	Port *p = tagged2Port(t);
	CheckCycle(p->getStream());
	NetAddress *addr = exportPort(p);
	int credit = addr->getCredit();
	
	bs->put(PORTTAG);
	marshalNetAddress(addr,bs);
	marshalNumber(credit,bs);
	
	trailCycle(p->getStreamRef());
	return;
      }
    }
    // no break here
  default:
    if (isAnyVar(t)) {
      warning("Cannot marshal variables");
    } else {
      warning("Cannot marshal %s",toC(t));
    }
    marshalTerm(nil(),bs);
    return;
  }

processArgs:
  OZ_Term arg0 = *args;
  trailCycle(args);
  marshalTerm(arg0,bs);
  args++;
  if (argno == 1) return;
  for(int i=1; i<argno-1; i++) {
    marshalTerm(*args,bs);
    args++;
  }
  // tail recursion optimization
  t = *args;
  goto loop;
}

void unmarshalTerm(ByteStream *bs, OZ_Term *ret)
{
  int argno;
loop:
  MarshalTag tag = (MarshalTag) bs->get();

  switch(tag) {

  case SMALLINTTAG: *ret = OZ_int(unmarshalNumber(bs)); return;
  case FLOATTAG:    *ret = OZ_float(unmarshalFloat(bs)); return;

  case ATOMTAG:     
    {
      char *aux = unmarshalString(bs);
      *ret = OZ_atom(aux);
      delete aux;
      return;
    }

  case BIGINTTAG:
    {
      char *aux = unmarshalString(bs);
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
      argno = unmarshalNumber(bs);
      TaggedRef label;
      unmarshalTerm(bs,&label);
      SRecord *rec = SRecord::newSRecord(label,argno);
      *ret = makeTaggedSRecord(rec);
      refTable->set(refCounter++,*ret);
      ret = rec->getRef();
      goto processArgs;      
    }

  case RECORDTAG:
    {
      TaggedRef arity;
      unmarshalTerm(bs,&arity);
      argno = length(arity);
      TaggedRef label;
      unmarshalTerm(bs,&label);
      SRecord *rec = SRecord::newSRecord(label,mkArity(arity));
      *ret = makeTaggedSRecord(rec);
      refTable->set(refCounter++,*ret);
      ret = rec->getRef();
      goto processArgs;      
    }

  case REFTAG:
    {
      int i = unmarshalNumber(bs);
      *ret = refTable->get(i);
      return;
    }

  case PORTTAG:
    {
      Port *p = (Port *) unmarshalNetAddress(PORTTAG,bs);
      int credit = unmarshalNumber(bs);
      p->getAddress()->giveCredit(credit);
      *ret = makeTaggedConst(p);
      return;
    }
  default:
    printf("unmarshal: unexpected tag: %d\n",tag);
    *ret = nil();
    return;
  }

processArgs:
  for(int i=0; i<argno-1; i++) {
    unmarshalTerm(bs,ret++);
  }
  // tail recursion optimization
  goto loop;
}

void unmarshalSend(char *buf, int len, int *port, OZ_Term *t)
{
  ByteStream *bs = new ByteStream(buf,len);
  OZ_Term ret;
  refCounter = 0;
  *port = unmarshalNumber(bs);
  unmarshalTerm(bs,t);
  delete bs;
}

/*
 * -------------------------------------------------------------------------
 * BUILTINS
 * -------------------------------------------------------------------------
 */

void domarshalTerm(OZ_Term t, ByteStream *bs)
{
  refCounter = 0;
  marshalTerm(t,bs);
  refTrail->unwind();
}


int reliableSend(int sd, ByteStream *bs)
{
  return reliableSend(sd,bs->getPtr(),bs->getLen());
}


void remoteSend(Port *p, TaggedRef msg)
{
  int site = p->getAddress()->site;
  int index = p->getAddress()->index;

  ByteStream *bs = new ByteStream();
  bs->put(GSEND);
  marshalNumber(index,bs);
  domarshalTerm(msg,bs);
  
  int ret = reliableSend(site,bs);
  Assert(ret == 0);
  delete bs;
}


ByteStream *gsend(OZ_Term t)
{
  ByteStream *bs = new ByteStream();

  bs->put(GSEND);
  marshalNumber((unsigned int)-1,bs);
  domarshalTerm(t,bs);
  return bs;
}

OZ_C_proc_begin(BIreliableSend,2)
{
  OZ_declareIntArg(0,sd);
  OZ_declareArg(1,value);

  ByteStream *bs=gsend(value);
  int ret = reliableSend(sd,bs);
  delete bs;
  if (ret == 0) return PROCEED;

  return OZ_raise(OZ_mkTupleC("reliableSend",1,OZ_int(lastError())));
}
OZ_C_proc_end

OZ_C_proc_begin(BIunreliableSend,2)
{
  OZ_declareIntArg(0,sd);
  OZ_declareArg(1,value);

  ByteStream *bs=gsend(value);

  int len = bs->getLen();

  int ret = unreliableSend(sd,bs->getPtr(),len);
  delete bs;
  if (ret == 0) return PROCEED;

  return OZ_raise(OZ_mkTupleC("unreliableSend",1,OZ_int(lastError())));
}
OZ_C_proc_end


OZ_C_proc_begin(BIstartSite,2)
{
  OZ_declareIntArg(0,p);
  OZ_declareArg(1,stream);

  if (ozport!=0) {
    return OZ_raise(OZ_mkTupleC("startSite",1,OZ_atom("twice")));
  }

  ozport = makeTaggedConst(new Port(am.rootBoard, stream));

  if (ipInit(p,siteReceive) < 0) {
    ozport=0;
    return OZ_raise(OZ_mkTupleC("startSite",1,OZ_atom("ipInit")));
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
  ownerTable = new OwnerTable(100);
}

#endif
