#include "oz_api.h"
#include "extension.hh"
#include "bytedata.hh"
#include "am.hh"
#include "os.hh"
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>

class FileDescriptor: public SituatedExtension {
public:
  int fd;
  OZ_Term read_lock;
  OZ_Term write_lock;
  FileDescriptor(int FD,OZ_Term RD,OZ_Term WR)
    :SituatedExtension(),fd(FD),read_lock(RD),write_lock(WR){}
  FileDescriptor(FileDescriptor&);
  // Situated Extension
  static int id;
  virtual int getIdV() { return id; }
  virtual OZ_Term typeV() { return OZ_atom("fileDescriptor"); }
  virtual void printStreamV(ostream &out,int depth = 10);
  virtual Extension* gcV();
  virtual void gcRecurseV();
  //
  void release();
  void close();
};

//
// Situated Extension
//

int FileDescriptor::id;

inline Bool oz_isFileDescriptor(OZ_Term t)
{
  return oz_isExtension(t) &&
    tagged2Extension(t)->getIdV()==FileDescriptor::id;
}

Bool OZ_isFileDescriptor(OZ_Term t)
{ return oz_isFileDescriptor(oz_deref(t)); }

inline FileDescriptor* tagged2FileDescriptor(OZ_Term t)
{
  Assert(oz_isFileDescriptor(t));
  return (FileDescriptor*) tagged2Extension(t);
}

FileDescriptor* OZ_toFileDescriptor(OZ_Term t)
{ return tagged2FileDescriptor(oz_deref(t)); }

void FileDescriptor::printStreamV(ostream &out,int depth = 10)
{
  out << "<fileDescriptor ";
  if (fd < 0) out << "[closed]";
  else out << fd;
  out << ">";
}

Extension* FileDescriptor::gcV()
{
  return new FileDescriptor(fd,read_lock,write_lock);
}

void FileDescriptor::gcRecurseV()
{
  OZ_collectHeapTerm(read_lock,read_lock);
  OZ_collectHeapTerm(write_lock,write_lock);
}

void FileDescriptor::close() {
  if (fd>=0) { ::close(fd); fd = -1; }
}

void FileDescriptor::release() { close(); }

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
    DEREF(t, t_ptr, t_tag);                             \
    if (oz_isVariable(t_tag)) {                         \
      am.addSuspendVarList(t_ptr);                      \
      return SUSPEND;                                   \
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
    DEREF(t, t_ptr, t_tag);                             \
    if (oz_isVariable(t_tag)) {                         \
      am.addSuspendVarList(t_ptr);                      \
      return SUSPEND;                                   \
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
  CHECK_READ(FD->fd);
  static char buffer[IOBUFMAX];
  WRAPCALL("read",osread(FD->fd,buffer,IOBUFMAX),ret);
  // return unit on EOF, a byte string otherwise
  OZ_RETURN((ret==0)?OZ_unit():OZ_mkByteString(buffer,ret));
} OZ_BI_end

OZ_BI_define(io_make,3,1)
{
  OZ_declareInt(0,FD);
  OZ_declareTerm(1,RD);
  OZ_declareTerm(2,WR);
  OZ_RETURN(makeTaggedConst(new FileDescriptor(FD,RD,WR)));
} OZ_BI_end

OZ_BI_define(io_close,1,0)
{
 OZ_declareFD(0,FD);
 FD->close();
 return PROCEED;
} OZ_BI_end

OZ_BI_define(io_release,1,0)
{
  OZ_declareFD(0,FD);
  FD->release();
  return PROCEED;
} OZ_BI_end

OZ_BI_define(io_readLock,1,1)
{
  OZ_declareFD(0,FD);
  OZ_RETURN(FD->read_lock);
} OZ_BI_end

OZ_BI_define(io_writeLock,1,1)
{
  OZ_declareFD(0,FD);
  OZ_RETURN(FD->write_lock);
} OZ_BI_end

OZ_BI_define(io_open,3,1)
{
  OZ_declareBitString(1,FLAGS   );
  OZ_declareInt(      2,MODE    );
  OZ_declareVS(       0,FILE,LEN);
  int    flags;
  if (FLAGS->get(0))
    flags = (FLAGS->get(1))?O_RDWR:O_RDONLY;
  else if (FLAGS->get(1)) flags = O_WRONLY;
  else
    return
      OZ_raiseErrorC("io",2,OZ_atom("open"),OZ_atom("noReadWrite"));
  if (FLAGS->get(2)) flags |= O_APPEND;
  if (FLAGS->get(3)) flags |= O_CREAT;
  if (FLAGS->get(4)) flags |= O_TRUNC;
  int fd;
  fd = open(FILE,flags,(mode_t)MODE);
  if (fd<0) RETURN_UNIX_ERROR("open");
  OZ_RETURN_INT(fd);
} OZ_BI_end

OZ_BI_define(io_socketpair,0,2)
{
  int sv[2];
  WRAPCALL("socketpair",socketpair(PF_UNIX,SOCK_STREAM,0,sv),ret);
  if (ret<0) RETURN_UNIX_ERROR("socketpair");
  OZ_out(0) = OZ_int(sv[0]);
  OZ_out(1) = OZ_int(sv[1]);
  return PROCEED;
} OZ_BI_end

OZ_BI_define(io_dup,1,1)
{
  OZ_declareFD(0,FD1);
  WRAPCALL("dup",dup(FD->fd),ret);
  if (ret<0) RETURN_UNIX_ERROR("dup");
  OZ_RETURN_INT(ret);
} OZ_BI_end

OZ_BI_define(io_fork,0,1)
{
  pid_t pid;
  pid = fork();
  if (pid<0) RETURN_UNIX_ERROR("fork");
  OZ_RETURN_INT(pid);
} OZ_BI_end

OZ_BI_define(io_execvp,2,0)
{
  OZ_expectVS(0,CMD,LEN);
  OZ_expectDetTerm(1,ARGS);
  int i = 1;
  argv[0] = strdup(CMD);
  while (!OZ_isNil(ARGS)) {
    argv[i] = strdup(OZ_virtualStringToC(OZ_head(ARGS),0));
    i++;
  }
  argv[i] = NULL;
  execvp(argv[0],argv);
  RETURN_UNIX_ERROR("execvp");  // should never return
} OZ_BI_end

OZ_BI_define(io_pipe,0,2)
{
  int sv[2];
  WRAPCALL("pipe",pipe(sv),ret);
  if (ret<0) RETURN_UNIX_ERROR("pipe");
  OZ_out(0) = OZ_int(sv[0]);
  OZ_out(1) = OZ_int(sv[1]);
  return PROCEED;
} OZ_BI_end

extern "C"
{
  OZ_C_proc_interface * oz_init_module(void)
  {
    static OZ_C_proc_interface i_table[] = {
      {"is"             ,1,1,io_is},
      {"write"          ,3,1,io_write},
      {"read"           ,1,1,io_read},
      {"make"           ,1,3,io_make},
      {"close"          ,1,0,io_close},
      {"release"        ,1,0,io_release},
      {"readLock"       ,1,1,io_readLock},
      {"writeLock"      ,1,1,io_writeLock},
      {"open"           ,3,1,io_open},
      {"socketpair"     ,0,2,io_socketpair},
      {"dup"            ,1,1,io_dup},
      {"fork"           ,0,1,io_fork},
      {"execvp"         ,2,0,io_execvp},
      {"pipe"           ,0,2,io_pipe},
      {0,0,0,0}
    };
    FileDescriptor::id = oz_newUniqueId();
    return i_table;
  }
} /* extern "C" */
