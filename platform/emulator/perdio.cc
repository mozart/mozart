/*
  Perdio Project, DFKI & SICS,
  Universit"at des Saarlandes
  Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr, mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  Log: $Log$
  Log: Revision 1.2  1996/07/26 15:17:43  mehl
  Log: perdio communication: see ~mehl/perdio.oz
  Log:

  network layer
  ------------------------------------------------------------------------
*/

#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include "ip.hh"
#include "perdio.hh"

#include "oz.h"
#include "am.hh"


extern OZ_Term unmarshal(char *buf, int len);

/*
 * a site is defined by a
 *   name: hostname.domainname
 *   port: udp/tcp port number
 *   time_stamp: creation time of the site
 *
 */
class Site {
public:
  char *hostname;
  int port;
  time_t time_stamp;
public:
  Site() {};
};

int mySocket;
Site *mySite;

int siteInit()
{
  udpInit();
  mySocket=createUdpPort();

  mySite             = new Site();
  mySite->hostname   = hostName();
  mySite->port       = portNumber(mySocket);
  mySite->time_stamp = time(0);
  return mySocket;
}

void siteReceive()
{
  char *msg;
  int len;
  if (!udpReceive(mySocket,msg,len)) {
    error("siteReceive: error\n");
    return;
  }
  if (len==0) {
    error("siteReceive: zero message\n");
    return;
  }

  switch (msg[0]) {
  case GSEND:
    {
      OZ_Term t = unmarshal(msg+1,len-1);
      printf("siteReceive: GSEND '%s'\n",OZ_toC(t,10,10));
    }
    break;
  default:
    printf("siteReceive: %d bytes\n",len);
    printf("\n--\n%s\n--\n",msg);
  }
}

int sendToSite(char *msg, int len, Site *site)
{
  return sendToPort(msg,len,site->hostname,site->port);
}

int sendToPort(char *msg, int len, char *hostname, int port)
{
  return udpSend(mySocket,msg,len,hostname,port);
}

int myPort() { return mySite->port; }


/*
 * Marshal
 */

#define BSEOF -1

class ByteStream {
  char *array;
  int size;
  char *pos;
  int len;
public:
  ByteStream() : len(0)
  {
    len = 0;
    size = 1000;
    array = new char[size];
    pos = array;
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
    return pos>=array+len ? BSEOF : *pos++;
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
  for (char *s=oldarray; s<pos;) {
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


#define CheckCycle(rec)				\
  {						\
    OZ_Term t = *rec->getRef();			\
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


typedef enum {SMALLINTTAG, BIGINTTAG, FLOATTAG, ATOMTAG, 
	      RECORDTAG, TUPLETAG, LISTTAG, REFTAG} MarshalTag;


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
      CheckCycle(l);
      bs->put(LISTTAG);
      argno = 2;
      args  = l->getRef();
      goto processArgs;
    }

  case SRECORD:
    {
      SRecord *rec = tagged2SRecord(t);
      CheckCycle(rec);
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
  default:
    warning("Cannot marshal %s",toC(t));
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

  default:
    error("Unexpected tag: %d",tag);
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

OZ_Term unmarshal(char *buf, int len)
{
  ByteStream *bs = new ByteStream(buf,len);
  OZ_Term ret;
  refCounter = 0;
  unmarshalTerm(bs,&ret);
  delete bs;
  return ret;
}

/*
 * TEST BUILTINS
 */

OZ_C_proc_begin(BImyPort,1)
{
  OZ_declareArg(0,out);

  return OZ_unifyInt(out,myPort());
}
OZ_C_proc_end

OZ_C_proc_begin(BIgSend,3)
{
  OZ_declareArg(0,value);
  OZ_declareAtomArg(1,host);
  OZ_declareIntArg(2,port);

  ByteStream *bs = new ByteStream();

  bs->put(GSEND);
  refCounter = 0;
  marshalTerm(value,bs);
  refTrail->unwind();

  int len = bs->getLen();
  int ret = sendToPort(bs->getPtr(),len,host,port);
  delete bs;
  if (ret == len) return PROCEED;
  if (ret < 0) {
    return FAILED;
  }
  return OZ_raise(OZ_atom("gSend"));
}
OZ_C_proc_end

OZ_C_proc_begin(BIeximportTerm,2)
{
  OZ_Term out = OZ_getCArg(0);
  ByteStream *bs = new ByteStream();

  refCounter = 0;
  marshalTerm(out,bs);
  bs->reset();
  refTrail->unwind();

  OZ_Term ret;
  refCounter = 0;
  unmarshalTerm(bs,&ret);

  delete bs;
  return OZ_unify(ret,OZ_getCArg(1));
}
OZ_C_proc_end


BIspec perdioSpec[] = {
  {"myPort",   1, BImyPort, 0},
  {"gSend",    3, BIgSend, 0},
  {"eximportTerm", 2, BIeximportTerm, 0},
  {0,0,0,0}
};


void BIinitPerdio()
{
  BIaddSpec(perdioSpec);
  
  refTable = new RefTable();
  refTrail = new RefTrail();
}

