#include "bytedata.hh"
#include "mozart.h"
#include "am.hh"
#include "os.hh"
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>

class FileDescriptor: public OZ_Extension {
public:
  int fd;
  FileDescriptor(int FD):OZ_Extension(),fd(FD){}
  FileDescriptor(FileDescriptor&);
  // Extension
  static int id;
  virtual int getIdV() { return id; }
  virtual OZ_Term typeV() { return OZ_atom("fileDescriptor"); }
  virtual OZ_Term printV(int depth = 10);
  virtual OZ_Extension* gCollectV(void);
  virtual OZ_Extension* sCloneV(void) { Assert(0); return NULL; }
  virtual void gCollectRecurseV(void) {}
  virtual void sCloneRecurseV(void) {}
  //
  void doFree();
  void doClose();
};

//
// Extension
//

int FileDescriptor::id;

inline Bool oz_isFileDescriptor(OZ_Term t)
{
  return OZ_isExtension(t) &&
    OZ_getExtension(t)->getIdV()==FileDescriptor::id;
}

Bool OZ_isFileDescriptor(OZ_Term t)
{ return oz_isFileDescriptor(oz_deref(t)); }

inline FileDescriptor* tagged2FileDescriptor(OZ_Term t)
{
  Assert(oz_isFileDescriptor(t));
  return (FileDescriptor*) OZ_getExtension(t);
}

FileDescriptor* OZ_toFileDescriptor(OZ_Term t)
{ return tagged2FileDescriptor(oz_deref(t)); }

OZ_Term FileDescriptor::printV(int)
{
  return OZ_pair2(OZ_atom("<fileDescriptor "),
                  OZ_pair2(fd < 0 ? OZ_atom("[closed]") : OZ_int(fd),
                           OZ_atom(">")));
}

OZ_Extension* FileDescriptor::gCollectV()
{
  return new FileDescriptor(fd);
}

void FileDescriptor::doClose() {
  if (fd>=0) {
    osClrWatchedFD(fd,SEL_READ);
    osClrWatchedFD(fd,SEL_WRITE);
    close(fd); fd = -1;
  }
}

void FileDescriptor::doFree() { doClose(); }

OZ_Term OZ_mkFileDescriptor(int fd) {
  // watching fd both for read and write may not always
  // be necessary - I may optimize this by supplying an
  // additional argument to be more specific when possible
  osWatchFD(fd,SEL_READ);
  osWatchFD(fd,SEL_WRITE);
  return OZ_extension(new FileDescriptor(fd));
}

//
// Builtins
//

#define OZ_declareFD(ARG,VAR) \
OZ_declareType(ARG,VAR,FileDescriptor*,"fileDescriptor", \
               OZ_isFileDescriptor,OZ_toFileDescriptor)

OZ_BI_define(io_is,1,1)
{
  OZ_declareDetTerm(0,t);
  OZ_RETURN_BOOL(OZ_isFileDescriptor(t));
} OZ_BI_end

extern int raiseUnixError(char *f,int n, char * e, char * g);

#define RETURN_UNIX_ERROR(f) \
{ return raiseUnixError(f,ossockerrno(), OZ_unixError(ossockerrno()), "os"); }

#define WRAPCALL(f, CALL, RET)                          \
int RET;                                                \
while ((RET = CALL) < 0) {                              \
  if (ossockerrno() != EINTR) { RETURN_UNIX_ERROR(f); } \
}

#define CHECK_WRITE(FD)                                 \
{ int sel = osTestSelect(FD,SEL_WRITE);                 \
  if (sel < 0)  { RETURN_UNIX_ERROR("select"); }        \
  if (sel == 0) {                                       \
    TaggedRef t = oz_newVariable();                     \
    (void) OZ_writeSelect(FD, NameUnit, t);             \
    DEREF(t, t_ptr);                                    \
    if (oz_isVar(t)) {                                  \
      return oz_addSuspendVarList(t_ptr);               \
    }                                                   \
  }                                                     \
}

OZ_BI_define(io_write,3,1)
{
  OZ_declareFD(        0,FD);
  OZ_declareByteString(1,BS);
  OZ_declareInt       (2,I );   // offset

  CHECK_WRITE(FD->fd);

  char * s = (char*) BS->getData();
  int    n = BS->getWidth();
  s += I;
  n -= I;
  // returns unit or the remaining offset into the byte string
  if (n>0) {
    WRAPCALL("write",oswrite(FD->fd,s,n),ret);
    if (n==ret) OZ_RETURN(OZ_unit());
    else OZ_RETURN_INT(I+ret);
  } else {
    OZ_RETURN(OZ_unit());
  }
} OZ_BI_end

#define CHECK_READ(FD)                                  \
{ int sel = osTestSelect(FD,SEL_READ);                  \
  if (sel < 0)  { RETURN_UNIX_ERROR("select"); }        \
  if (sel == 0) {                                       \
    TaggedRef t = oz_newVariable();                     \
    (void) OZ_readSelect(FD, NameUnit, t);              \
    DEREF(t, t_ptr);                                    \
    if (oz_isVar(t)) {                                  \
      return oz_addSuspendVarList(t_ptr);               \
    }                                                   \
  }                                                     \
}

#if SSIZE_MAX<4096
#define IOBUFMAX SSIZE_MAX
#else
#define IOBUFMAX 4096
#endif

OZ_BI_define(io_read,1,1)
{
  OZ_declareFD(0,FD);
  if (FD->fd < 0) OZ_RETURN(OZ_unit());
  CHECK_READ(FD->fd);
  static char buffer[IOBUFMAX];
  WRAPCALL("read",osread(FD->fd,buffer,IOBUFMAX),ret);
  // return unit on EOF, a byte string otherwise
  OZ_RETURN((ret==0)?OZ_unit():OZ_mkByteString(buffer,ret));
} OZ_BI_end

OZ_BI_define(io_make,3,1)
{
  OZ_declareInt(0,FD);
  OZ_RETURN(OZ_mkFileDescriptor(FD));
} OZ_BI_end

OZ_BI_define(io_close,1,0)
{
 OZ_declareFD(0,FD);
 FD->doClose();
 return PROCEED;
} OZ_BI_end

OZ_BI_define(io_free,1,0)
{
  OZ_declareFD(0,FD);
  FD->doFree();
  return PROCEED;
} OZ_BI_end

OZ_BI_define(io_open,3,1)
{
  OZ_declareBitString(1,FLAGS   );
  OZ_declareInt(      2,MODE    );
  OZ_declareVS(       0,FILE,LEN);
  int    flags;
  if (OZ_BitStringGet(FLAGS,0))
    flags = (OZ_BitStringGet(FLAGS,1))?O_RDWR:O_RDONLY;
  else if (OZ_BitStringGet(FLAGS,1)) flags = O_WRONLY;
  else
    return
      OZ_raiseErrorC("io",2,OZ_atom("open"),OZ_atom("noReadWrite"));
  if (OZ_BitStringGet(FLAGS,2)) flags |= O_APPEND;
  if (OZ_BitStringGet(FLAGS,3)) flags |= O_CREAT;
  if (OZ_BitStringGet(FLAGS,4)) flags |= O_TRUNC;
  int fd;
  fd = open(FILE,flags,(mode_t)MODE);
  if (fd<0) RETURN_UNIX_ERROR("open");
  OZ_RETURN(OZ_mkFileDescriptor(fd));
} OZ_BI_end

OZ_BI_define(io_socketpair,0,2)
{
  int sv[2];
  WRAPCALL("socketpair",socketpair(PF_UNIX,SOCK_STREAM,0,sv),ret);
  if (ret<0) RETURN_UNIX_ERROR("socketpair");
  OZ_out(0) = OZ_mkFileDescriptor(sv[0]);
  OZ_out(1) = OZ_mkFileDescriptor(sv[1]);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(io_dup,1,1)
{
  OZ_declareFD(0,FD);
  WRAPCALL("dup",dup(FD->fd),ret);
  if (ret<0) RETURN_UNIX_ERROR("dup");
  OZ_RETURN(OZ_mkFileDescriptor(ret));
} OZ_BI_end

OZ_BI_define(io_pipe,0,2)
{
  int sv[2];
  WRAPCALL("pipe",pipe(sv),ret);
  if (ret<0) RETURN_UNIX_ERROR("pipe");
  OZ_out(0) = OZ_mkFileDescriptor(sv[0]);
  OZ_out(1) = OZ_mkFileDescriptor(sv[1]);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(io_getfd,1,1)
{
  OZ_declareFD(0,FD);
  OZ_RETURN_INT(FD->fd);
} OZ_BI_end

OZ_BI_define(io_lseek,3,1)
{
  OZ_declareFD(0,FD);
  OZ_declareInt(1,OFF);
  OZ_declareInt(2,WHENCE);
  int whence;
  switch (WHENCE) {
  case 0: whence=SEEK_SET; break;
  case 1: whence=SEEK_CUR; break;
  case 2: whence=SEEK_END; break;
  default: whence=-1; break;
  }
  off_t point = lseek(FD->fd,OFF,whence);
  if (point<0) RETURN_UNIX_ERROR("lseek");
  OZ_RETURN_INT(point);
} OZ_BI_end

extern "C"
{
  OZ_C_proc_interface * oz_init_module(void)
  {
    static OZ_C_proc_interface i_table[] = {
      {"is"             ,1,1,io_is},
      {"write"          ,3,1,io_write},
      {"read"           ,1,1,io_read},
      {"make"           ,1,1,io_make},
      {"close"          ,1,0,io_close},
      {"free"           ,1,0,io_free},
      {"open"           ,3,1,io_open},
      {"socketpair"     ,0,2,io_socketpair},
      {"dup"            ,1,1,io_dup},
      {"pipe"           ,0,2,io_pipe},
      {"getfd"          ,1,1,io_getfd},
      {"lseek"          ,3,1,io_lseek},
      {0,0,0,0}
    };
    FileDescriptor::id = oz_newUniqueId();
    return i_table;
  }
} /* extern "C" */
