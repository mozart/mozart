#include <mozart.h>
#include <zlib.h>
#include <sys/types.h>
#include <unistd.h>

class ZFile : public OZ_Extension {
public:
  gzFile f;
  ZFile(const char* path,const char *mode)
    : OZ_Extension(), f(gzopen(path,mode)) {}
  ZFile(ZFile*o) : f(o->f) {}
  //
  static int id;
  virtual int getIdV() { return id; }
  virtual OZ_Term typeV() { return OZ_atom("zfile"); }
  virtual OZ_Extension* gCollectV(void);
  virtual OZ_Extension* sCloneV(void) { Assert(0); return NULL; }
  virtual void gCollectRecurseV(void) {}
  virtual void sCloneRecurseV(void) {}
  virtual OZ_Term printV(int depth = 10);
};

int ZFile::id;

OZ_Extension* ZFile::gCollectV(void)
{
  return new ZFile(this);
}

OZ_Term ZFile::printV(int depth = 10)
{
  return OZ_atom("<zfile>");
}

inline int oz_isZFile(OZ_Term t)
{
  t = OZ_deref(t);
  return OZ_isExtension(t) &&
    OZ_getExtension(t)->getIdV()==ZFile::id;
}

inline ZFile* tagged2ZFile(OZ_Term t)
{
  Assert(oz_isZFile(t));
  return (ZFile*) OZ_getExtension(OZ_deref(t));
}

#define ZFileError(PROC) \
OZ_raiseErrorC("zfile",2,OZ_atom(PROC),OZ_inAsList())

#define ZFileError2(PROC,MSG) \
OZ_raiseErrorC("zfile",3,OZ_atom(PROC),OZ_inAsList(),OZ_atom(MSG))

#include <string.h>

OZ_BI_define(zfile_open,2,1)
{
  OZ_declareVirtualString(1,m0);
  char m[100];
  strcpy(m,m0);
  OZ_declareVirtualString(0,s);
  ZFile * z = new ZFile(s,m);
  if (z->f == NULL)
    return ZFileError("zfile_open");
  OZ_RETURN(OZ_extension(z));
}
OZ_BI_end

#define OZ_declareZFile(ARG,VAR) \
OZ_declareType(ARG,VAR,ZFile*,"zfile",oz_isZFile,tagged2ZFile)

OZ_BI_define(zfile_read,2,1)
{
  OZ_declareZFile(0,z);
  OZ_declareInt(1,n);
  char buf[n];
  int m = gzread(z->f,buf,n);
  if (m < 0) return ZFileError("zfile_read");
  OZ_RETURN(OZ_mkByteString(buf,m));
}
OZ_BI_end

OZ_BI_define(zfile_write,2,1)
{
  OZ_declareZFile(0,z);
  OZ_declareVS(1,s,n);
  if (n==0) OZ_RETURN(OZ_unit());
  int m = gzwrite(z->f,s,n);
  if (m < n) OZ_RETURN(OZ_mkByteString(s+m,n-m));
  OZ_RETURN(OZ_unit());
}
OZ_BI_end

OZ_BI_define(zfile_full_flush,1,0)
{
  OZ_declareZFile(0,z);
  gzflush(z->f,Z_FULL_FLUSH);
  return PROCEED;
}
OZ_BI_end

OZ_BI_define(zfile_finish,1,0)
{
  OZ_declareZFile(0,z);
  gzflush(z->f,Z_FINISH);
  return PROCEED;
}
OZ_BI_end

OZ_BI_define(zfile_seek,3,0)
{
  OZ_declareZFile(0,z);
  OZ_declareInt(1,offset);
  OZ_declareInt(2,whence);
  if (whence==0) whence=SEEK_SET;
  else if (whence==1) whence=SEEK_CUR;
  else return ZFileError2("zfile_seek","whence");
  int r = gzseek(z->f,offset,whence);
  if (r < 0) return ZFileError("zfile_seek");
  return PROCEED;
}
OZ_BI_end

OZ_BI_define(zfile_tell,1,1)
{
  OZ_declareZFile(0,z);
  OZ_RETURN_INT(gztell(z->f));
}
OZ_BI_end

OZ_BI_define(zfile_close,1,0)
{
  OZ_declareZFile(0,z);
  gzclose(z->f);
  z->f = NULL;
  return PROCEED;
}
OZ_BI_end

OZ_BI_define(zfile_putc,2,0)
{
  OZ_declareZFile(0,z);
  OZ_declareInt(1,c);
  if (c < 0 || c > 255)
    return ZFileError2("zfile_putc","badChar");
  if (gzputc(z->f,c) < 0)
    return ZFileError("zfile_putc");
  return PROCEED;
}
OZ_BI_end

OZ_BI_define(zfile_getc,1,1)
{
  OZ_declareZFile(0,z);
  int c = gzgetc(z->f);
  if (c < 0) return ZFileError("zfile_getc");
  OZ_RETURN_INT(c);
}
OZ_BI_end

OZ_C_proc_interface * oz_init_module(void)
{
  static OZ_C_proc_interface table[] = {
    {"open",2,1,zfile_open},
    {"read",2,1,zfile_read},
    {"write",2,1,zfile_write},
    {"flush",1,0,zfile_full_flush},
    {"finish",1,0,zfile_finish},
    {"seek",3,0,zfile_seek},
    {"tell",1,1,zfile_tell},
    {"close",1,0,zfile_close},
    {"putc",2,0,zfile_putc},
    {"getc",1,1,zfile_getc},
    {0,0,0,0}
  };
  ZFile::id = oz_newUniqueId();
  return table;
};

char oz_module_name[] = "ZFile";
