#include "tagged.hh"
#include "value.hh"
#include "mozart.h"
#include "extension.hh"
#include "zlib.h"

class ZlibIO : public OZ_Extension
{
public:
  static int id;
  gzFile zfile;

  ZlibIO(gzFile f) : zfile(f) {}

  virtual int getIdV() { return id; }
  virtual OZ_Term typeV() { return OZ_atom("zlibio"); }
  virtual OZ_Extension* gCollectV(void);
  virtual OZ_Extension* sCloneV(void) { Assert(0); return NULL; }
  virtual void gCollectRecurseV(void) {}
  virtual void sCloneRecurseV(void) {}
  virtual OZ_Term printV(int depth = 10);
  //
  void release();
  void close();

  const char * getErrorMsg()
  { int err; return gzerror(zfile,&err); }
};

int ZlibIO::id;

inline int oz_isZlibIO(OZ_Term t)
{
  t = oz_deref(t);
  return (oz_isExtension(t)) &&
    tagged2Extension(t)->getIdV()==ZlibIO::id;
}

inline ZlibIO* tagged2ZlibIO(OZ_Term t)
{
  Assert(oz_isZlibIO(t));
  return (ZlibIO*) tagged2Extension(oz_deref(t));
}

OZ_Extension* ZlibIO::gCollectV(void)
{
  return (new ZlibIO(zfile));
}

OZ_Term ZlibIO::printV(int)
{
  return typeV();
}

#define OZ_declareZlibIO(ARG,VAR) \
OZ_declareType(ARG,VAR,ZlibIO*,"zlibio",oz_isZlibIO,tagged2ZlibIO);

OZ_BI_define(zlibio_is,1,1)
{
  OZ_declareDetTerm(0,t);
  OZ_RETURN_BOOL(oz_isZlibIO(t));
}
OZ_BI_end

#define ZlibIOError(MSG) \
OZ_raiseErrorC("zlibio",1,OZ_atom(MSG))

#define CHECK_OPEN(t) \
if (t->zfile==0) { ZlibIOError("closed"); }

OZ_BI_define(zlibio_new,2,1)
{
  OZ_declareInt(0,fd);
  OZ_declareVirtualString(1,mode);
  gzFile zfile = gzdopen(fd,mode);
  if (zfile==NULL) { ZlibIOError("Z_MEM_ERROR"); }
  OZ_RETURN(OZ_extension(new ZlibIO(zfile)));
}
OZ_BI_end

OZ_BI_define(zlibio_close,1,0)
{
  OZ_declareZlibIO(0,t);
  if (t->zfile!=0)
    {
      if (gzclose(t->zfile)) { ZlibIOError(t->getErrorMsg()); }
      t->zfile = 0;
    }
  return PROCEED;
}
OZ_BI_end

static const int ZLIBIO_BUFFER_SIZE = 1024;

OZ_BI_define(zlibio_read_bytestring,2,2)
{
  OZ_declareZlibIO(0,t); CHECK_OPEN(t);
  OZ_declareInt(1,n);

  char buffer[ZLIBIO_BUFFER_SIZE];
  if (n>ZLIBIO_BUFFER_SIZE) n=ZLIBIO_BUFFER_SIZE;

  n = gzread(t->zfile,buffer,(unsigned int) n);
  if (n < 0) { ZlibIOError(t->getErrorMsg()); }
  OZ_out(0) = OZ_mkByteString(buffer,n);
  OZ_out(1) = oz_int(n);
  return PROCEED;
}
OZ_BI_end

OZ_BI_define(zlibio_read,2,3)
{
  OZ_declareZlibIO(0,t); CHECK_OPEN(t);
  OZ_declareInt(1,n);

  char buffer[ZLIBIO_BUFFER_SIZE];
  if (n>ZLIBIO_BUFFER_SIZE) n=ZLIBIO_BUFFER_SIZE;

  n = gzread(t->zfile,buffer,(unsigned int) n);
  if (n < 0) { ZlibIOError(t->getErrorMsg()); }

  OZ_Term tail = OZ_newVariable();
  OZ_Term head = oz_string(buffer,n,tail);
  OZ_out(0) = head;
  OZ_out(1) = tail;
  OZ_out(2) = oz_int(n);
  return PROCEED;
}
OZ_BI_end

#define max_vs_length 4096*4
#define vs_buff(VAR) char VAR[max_vs_length + 256];

#define NEW_RETURN_SUSPEND(LEN,VAR,REST)       \
{ OZ_Term susp_tuple = OZ_tupleC("suspend",3); \
  OZ_putArg(susp_tuple,0,LEN);                 \
  OZ_putArg(susp_tuple,1,VAR);                 \
  OZ_putArg(susp_tuple,2,REST);                \
  OZ_RETURN(susp_tuple);                       \
}

extern OZ_Return
OZ_buffer_vs(OZ_Term vs, char *write_buff, int *len,
             OZ_Term *rest, OZ_Term *susp);

OZ_BI_define(zlibio_write,2,1)
{
  OZ_declareZlibIO(0,t); CHECK_OPEN(t);
  OZ_declareDetTerm(1,vs);

  int len,n;
  OZ_Return status;
  OZ_Term rest,susp;
  vs_buff(write_buff);

  status = OZ_buffer_vs(vs, write_buff, &len, &rest, &susp);

  if (status != PROCEED && status != SUSPEND)
    return status;

  if (len==0) OZ_RETURN_INT(0);

  n = gzwrite(t->zfile,write_buff,len);

  if (n==0) {
    ZlibIOError(t->getErrorMsg());
  }
  else if (status==PROCEED) {
    if (n==len) {
      OZ_RETURN_INT(n);
    }
    else {
      Assert(len > n);
      NEW_RETURN_SUSPEND(oz_int(n), oz_nil(),
                         OZ_mkByteString(write_buff+n,len-n));
    }
  }
  else {
    Assert(status==SUSPEND);
    if (n==len) {
      NEW_RETURN_SUSPEND(oz_int(n),susp,rest);
    }
    else {
      NEW_RETURN_SUSPEND(oz_int(n), susp,
                         oz_pair2(OZ_mkByteString(write_buff+n,len-n),
                                  rest));
    }
  }
}
OZ_BI_end

OZ_BI_define(zlibio_flush,2,0)
{
  OZ_declareZlibIO(0,t); CHECK_OPEN(t);
  OZ_declareInt(1,n);
  switch (n) {
  case 0: n = Z_NO_FLUSH; break;
  case 1:
  case 2: n = Z_SYNC_FLUSH; break;
  case 3: n = Z_FULL_FLUSH; break;
  case 4: n = Z_FINISH; break;
  default:
    ZlibIOError("invalid flush value");
  }
  if (gzflush(t->zfile,n)==Z_OK)
    return PROCEED;
  ZlibIOError(t->getErrorMsg());
}
OZ_BI_end

#ifndef MODULES_LINK_STATIC
#include "modZlibIO-if.cc"
#endif
