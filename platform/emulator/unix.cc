/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl, schulte
  Last modified: $Date$ from $Author$
  Version: $Revision$
  */

#include "wsock.hh"

#include "oz.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef WINDOWS
#include <direct.h>
#else
#include <dirent.h>
#include <netdb.h>
#endif

#include "os.hh"

#if defined(LINUX) || defined(HPUX_700)
extern int h_errno;
#endif

#include <time.h>
#include <sys/stat.h>

#if !defined(OS2_I486) && !defined(WINDOWS)
#include <sys/param.h>
#include <sys/socket.h>
#ifndef LINUX
#include <sys/uio.h>
#endif
#include <sys/un.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <netinet/in.h>
#endif

#include <signal.h>

#ifndef WINDOWS
#include <sys/utsname.h>
#endif

#ifdef IRIX5_MIPS
#include <bstring.h>
#endif


extern "C" char *inet_ntoa(struct in_addr in);


#define max_vs_length 4096*4
#define vs_buff(VAR) char VAR[max_vs_length + 256];


//
// Argument handling
//

#define OZ_declareVsArg(ARG,VAR) \
 vs_buff(VAR); OZ_nonvarArg(ARG);                                     \
 { int len; OZ_Return status; OZ_Term rest, susp;                 \
   status = buffer_vs(OZ_getCArg(ARG), VAR, &len, &rest, &susp);      \
   if (status == SUSPEND) {                                           \
     if (OZ_isVariable(susp)) {                                       \
       return SUSPEND;                                                \
     } else {                                                         \
       return OZ_raise(OZ_mkTupleC("unix",1,OZ_mkTupleC("vs",2,"virtual string too long in arg",ARG+1))); \
     }                                                                \
   } else if (status == FAILED) {                                     \
     return FAILED;                                                   \
   }                                                                  \
   *(VAR+len) = '\0';                                                 \
 }

#define IsPair(s) (s[0]=='#' && s[1]=='\0')



// checking

inline int unixIsCons(OZ_Term list, OZ_Term *hd, OZ_Term *tl)
{
  if (!OZ_isCons(list)) {
    return 0;
  }
  *hd = OZ_head(list);
  *tl = OZ_tail(list);
  return 1;
}


// -------------------------------------------------
// errors
// -------------------------------------------------


#define WRAPCALL(CALL,RET,OUT) \
int RET;                                            \
while ((RET = CALL) < 0) {                          \
  if (errno != EINTR) { RETURN_UNIX_ERROR(OUT); } \
}


// -------------------------------------------------
// specification of returning
// -------------------------------------------------


// return upon runtime-failure
#define RETURN_ANY_ERROR(OUT,NUMBER,STRING,GROUP)             \
{  OZ_Term err_tuple = OZ_tupleC("error",3);                  \
   OZ_putArg(err_tuple, 0, OZ_int(NUMBER));              \
   OZ_putArg(err_tuple, 1, OZ_string(STRING));           \
   OZ_putArg(err_tuple, 2, OZ_string(GROUP));            \
   return OZ_unify(OUT, err_tuple);                           \
 }

// return upon unix-error
#define RETURN_UNIX_ERROR(OUT) \
{ RETURN_ANY_ERROR(OUT,errno,OZ_unixError(errno),"unix"); }


#if defined(ULTRIX_MIPS) || defined(OS2_I486)

#define RETURN_NET_ERROR(OUT) { RETURN_ANY_ERROR(OUT,0,"Host lookup failure.","host"); }

#else

static char* h_strerror(const int err) {
  switch (err) {
  case HOST_NOT_FOUND:
    return "No such host is known.";
  case TRY_AGAIN:
    return "Retry later again.";
  case NO_RECOVERY:
    return "Unexpected non-recoverable server failure.";
#if defined(SOLARIS_SPARC) || defined(LINUX)
  case NO_ADDRESS:
#endif
#if defined(SUNOS_SPARC)
  case NO_DATA:
#endif
    return "No internet address.";
  default:
    return "Hostname lookup failure.";
  }
}

#define RETURN_NET_ERROR(OUT) \
{ RETURN_ANY_ERROR(OUT,h_errno,h_strerror(errno),"host"); }

#endif


// return unable to perform service
#define RETURN_UNABLE(OUT) \
{ return OZ_unifyAtom(OUT,"unable"); }

// return suspension upon
#define RETURN_SUSPEND(OUT,LEN,VAR,REST)          \
{ OZ_Term susp_tuple = OZ_tupleC("suspend",3);    \
  OZ_putArg(susp_tuple,0,LEN);               \
  OZ_putArg(susp_tuple,1,VAR);               \
  OZ_putArg(susp_tuple,2,REST);              \
  return OZ_unify(OUT,susp_tuple);                 \
}

// return suspension upon
#define RETURN_UNABLE_REST(OUT,REST)              \
{ OZ_Term un_tuple = OZ_tupleC("unable", 1);      \
  OZ_putArg(un_tuple,0,REST);                \
  return OZ_unify(OUT,un_tuple);                  \
}



// -------------------------------------------------
// check file descriptors
// -------------------------------------------------

#define CHECK_READ(FD,OUT)                      \
{ int ret = osTestSelect(FD,SEL_READ);          \
  if (ret < 0)  { RETURN_UNIX_ERROR(OUT); }     \
  if (ret == 0) { RETURN_UNABLE(OUT); }         \
}

#define CHECK_WRITE(FD,OUT,REST)                        \
{ int ret = osTestSelect(FD,SEL_WRITE);                 \
  if (ret < 0)  { RETURN_UNIX_ERROR(OUT); }             \
  if (ret == 0) { RETURN_UNABLE_REST(OUT,REST); }       \
}



static OZ_Term openbuff2list(int len, const char *s, const OZ_Term tl)
{
  // gives back a list of length len which elments are taken from a C-string
  // the tail of the list is given by list
  OZ_Term prev, hd;

  if (len == 0)
    return tl;

  hd = OZ_tupleC("|", 2);
  OZ_putArg(hd, 0, OZ_int((unsigned char) *s++));
  prev = hd;

  while (--len) {
    OZ_Term next = OZ_tupleC("|", 2);

    OZ_putArg(next, 0, OZ_int((unsigned char) *s++));
    OZ_putArg(prev, 1, next);
    prev = next;
  }

  OZ_putArg(prev, 1, tl);
  return hd;
}


//
// Handling of virtual strings
//

inline OZ_Term buff2list(int len, const char *s)
{
  return openbuff2list(len, s, OZ_nil());
}



OZ_Return atom2buff(OZ_Term atom, char **write_buff, int *len,
                  OZ_Term *rest, OZ_Term *susp)
{
  char c;

  if (!OZ_isAtom(atom)) {
    OZ_warning("illegal atom in virtual string");
    return FAILED;
  }

  char *string = OZ_atomToC(atom);

  if (IsPair(string))
    return PROCEED;

  while ((c = *string) &&
         *len < max_vs_length) {
    **write_buff = c;
    (*write_buff)++;
    (*len)++;
    string++;
  }

  if (*len == max_vs_length && c) {
    *susp = OZ_string(string);
    *rest = *susp;
    return SUSPEND;
  }

  return PROCEED;
}


OZ_Return int2buff(OZ_Term ozint, char **write_buff, int *len,
                 OZ_Term *rest, OZ_Term *susp)
{
  char *string = OZ_intToCString(ozint);
  char c;

  char *help = string;
  while ((c = *help) &&
         *len < max_vs_length) {
    **write_buff = c;
    (*write_buff)++;
    (*len)++;
    help++;
  }

  if (*len == max_vs_length && c) {
    *susp = OZ_string(help);
    *rest = *susp;
    return SUSPEND;
  }

  return PROCEED;
}

OZ_Return float2buff(OZ_Term ozfloat, char **write_buff, int *len,
                   OZ_Term *rest, OZ_Term *susp)
{
  char *string = OZ_floatToCString(ozfloat);
  char c;

  char *help = string;
  while ((c = *help) &&
         *len < max_vs_length) {
    **write_buff = c;
    (*write_buff)++;
    (*len)++;
    help++;
  }

  if (*len == max_vs_length && c) {
    *susp = OZ_string(help);
    *rest = *susp;
    return SUSPEND;
  }

  return PROCEED;
}


OZ_Return list2buff(OZ_Term list, char **write_buff, int *len,
                  OZ_Term *rest, OZ_Term *susp)
{
  OZ_Term hd, tl;

  while (unixIsCons(list, &hd, &tl)) {
    if ((*len == max_vs_length) || OZ_isVariable(hd)) {
      *susp = hd;
      *rest = list;
      return SUSPEND;
    }

    int c;

    if (OZ_isInt(hd)) {
      c = OZ_intToC(hd);
      if ((c >= 0) && (c < 256)) {
        **write_buff = (unsigned char) c;
        (*write_buff)++;
        (*len)++;
        list = tl;
        continue;
      }
    }
    OZ_warning("skipping illegal list element in virtual string");
    return FAILED;
  }

  if (OZ_isVariable(list)) {
    *susp = list;
    *rest = list;
    return SUSPEND;
  }

  if (OZ_isNil(list))
    return PROCEED;


  OZ_warning("illegal virtual string");
  return FAILED;
}


static OZ_Return vs2buff(OZ_Term vs, char **write_buff, int *len,
                       OZ_Term *rest, OZ_Term *susp)
{
  if (OZ_isAtom(vs)) {
    return OZ_isNil(vs) ? PROCEED : atom2buff(vs, write_buff, len, rest, susp);
  }

  char *label = NULL;
  if (OZ_isTuple(vs) && (label = OZ_atomToC(OZ_label(vs)))) {
    int width = OZ_width(vs);
    if (IsPair(label) && width > 0) {
      OZ_Term arg_susp, arg_rest;

      for (int i=0; i<width; i++) {

        OZ_Return status = vs2buff(OZ_getArg(vs,i), write_buff, len,
                                 &arg_rest, &arg_susp);
        if (status == SUSPEND) {
          *susp = arg_susp;

          if (i==width-1) {
            *rest = arg_rest;
          } else {
            *rest = OZ_tupleC("#", (width - i));

            OZ_putArg(*rest, 0, arg_rest);
            i++;
            for (int j=1 ; i < width ; (j++, i++)) {
              OZ_putArg(*rest, j, OZ_getArg(vs, i));
            }
          }
          return SUSPEND;
        } else if (status == FAILED) {
          return FAILED;
        }

      }

      return PROCEED;

    } else if (label[0] == '|' && label[1] == '\0' && width == 2) {
      return list2buff(vs, write_buff, len, rest, susp);
    }
    OZ_warning("skipping illegal virtual string");
    return FAILED;
  }

  if (OZ_isInt(vs)) {
    return int2buff(vs, write_buff, len, rest, susp);
  }

  if (OZ_isFloat(vs)) {
    return float2buff(vs, write_buff, len, rest, susp);
  }

  if (OZ_isVariable(vs)) {
    *rest = vs;
    *susp = vs;
    return SUSPEND;
  }

  OZ_warning("skipping illegal virtual string");
  return FAILED;
}


inline OZ_Return buffer_vs(OZ_Term vs, char *write_buff, int *len,
                         OZ_Term *rest, OZ_Term *susp)
{
  *len        = 0;
  return vs2buff(vs, &write_buff, len, rest, susp);
}


// -------------------------------------------------
// unix IO
// -------------------------------------------------


OZ_C_ioproc_begin(unix_fileDesc,2)
{
  OZ_declareAtomArg( 0, OzFileDesc);
  OZ_declareArg(1, out);

  int desc;
  if (!strcmp(OzFileDesc,"STDIN_FILENO")) {
    desc=dup(STDIN_FILENO);
  } else if (!strcmp(OzFileDesc,"STDOUT_FILENO")) {
    desc=dup(STDOUT_FILENO);
  } else if (!strcmp(OzFileDesc,"STDERR_FILENO")) {
    desc=dup(STDERR_FILENO);
  } else {
    OZ_warning("fileDesc: illegal descriptor");
    return FAILED;
  }

  return OZ_unifyInt(out, desc);
}
OZ_C_proc_end


static OZ_Term readEntries(DIR *dp) {
  static struct dirent *dirp;
  OZ_Term dirEntry;
  if ((dirp = readdir(dp)) != NULL) {
    dirEntry = OZ_string(dirp->d_name);
    return OZ_cons(dirEntry, readEntries(dp));
  }
  else
    return OZ_nil();
}

OZ_C_proc_begin(unix_getDir,2)
{
  DIR *dp;
  OZ_Term dirValue;
  OZ_declareVsArg(0, path);
  OZ_declareArg(1, out);

  if ((dp = opendir(path)) == NULL)
    RETURN_UNIX_ERROR(out);

  dirValue = readEntries(dp);

  if (closedir(dp) < 0)
    RETURN_UNIX_ERROR(out);

  return OZ_unify(out, dirValue);
}
OZ_C_proc_end


OZ_C_proc_begin(unix_stat,2)
{
  struct stat buf;
  char *fileType;
  off_t fileSize;
  OZ_declareVsArg(0, filename);
  OZ_declareArg(1, out);

  if (stat(filename, &buf) < 0)
    RETURN_UNIX_ERROR(out);

  if      (S_ISREG(buf.st_mode))  fileType = "reg";
  else if (S_ISDIR(buf.st_mode))  fileType = "dir";
  else if (S_ISCHR(buf.st_mode))  fileType = "chr";
  else if (S_ISBLK(buf.st_mode))  fileType = "blk";
  else if (S_ISFIFO(buf.st_mode)) fileType = "fifo";
  else fileType = "unknown";

  fileSize = buf.st_size;

  OZ_Term pairlist=
    OZ_cons(OZ_pairAA("type",fileType),
            OZ_cons(OZ_pairAI("size",fileSize),
                    OZ_nil()));
  return OZ_unify(out,OZ_recordInit(OZ_atom("stat"),pairlist));
}
OZ_C_proc_end

#ifndef WINDOWS
OZ_C_proc_begin(unix_uName,1)
{
  OZ_declareArg(0, out);

  struct utsname buf;
  if (uname(&buf) < 0)
    RETURN_UNIX_ERROR(out);

  OZ_Term t1=OZ_pairAS("sysname",buf.sysname);
  OZ_Term t2=OZ_pairAS("nodename",buf.nodename);
  OZ_Term t3=OZ_pairAS("release",buf.release);
  OZ_Term t4=OZ_pairAS("version",buf.version);
  OZ_Term t5=OZ_pairAS("machine",buf.machine);

  OZ_Term pairlist=
    OZ_cons(t1,OZ_cons(t2,OZ_cons(t3,OZ_cons(t4,OZ_cons(t5,OZ_nil())))));
  return OZ_unify(out,OZ_recordInit(OZ_atom("utsname"),pairlist));
}
OZ_C_proc_end
#endif


OZ_C_proc_begin(unix_getCWD,1)
{
  OZ_declareArg(0, out);

  const int SIZE=256;
  char buf[SIZE];
  if (getcwd(buf,SIZE)) return OZ_unifyAtom(out,buf);
  if (errno != ERANGE) RETURN_UNIX_ERROR(out);

  int size=SIZE+SIZE;
  char *bigBuf;
  while (OK) {
    bigBuf=(char *) malloc(size);
    if (getcwd(bigBuf,size)) {
      OZ_Return ret=OZ_unifyAtom(out,buf);
      free(bigBuf);
      return ret;
    }
    if (errno != ERANGE) RETURN_UNIX_ERROR(out);
    free(bigBuf);
    size+=SIZE;
  }
}
OZ_C_proc_end

#ifdef WINDOWS
#define O_NOCTTY   0
#define O_NONBLOCK 0
#define O_SYNC     0
#endif

OZ_C_ioproc_begin(unix_open,4)
{
  OZ_declareVsArg(0, filename);
  OZ_declareArg(1, OzFlags);
  OZ_declareArg(2, OzMode);
  OZ_declareArg(3, out);

  // Compute flags from their textual representation

  int flags = 0;
  OZ_Term hd, tl;

  while (unixIsCons(OzFlags, &hd, &tl)) {

    if (OZ_isVariable(hd)) return SUSPEND;

    if (OZ_unifyAtom(hd,"O_RDONLY") == PROCEED) {
      flags |= O_RDONLY;
    } else if (OZ_unifyAtom(hd,"O_WRONLY"  ) == PROCEED) {
      flags |= O_WRONLY;
    } else if (OZ_unifyAtom(hd,"O_RDWR"    ) == PROCEED) {
      flags |= O_RDWR;
    } else if (OZ_unifyAtom(hd,"O_APPEND"  ) == PROCEED) {
      flags |= O_APPEND;
    } else if (OZ_unifyAtom(hd,"O_CREAT"   ) == PROCEED) {
      flags |= O_CREAT;
    } else if (OZ_unifyAtom(hd,"O_EXCL"    ) == PROCEED) {
      flags |= O_EXCL;
    } else if (OZ_unifyAtom(hd,"O_TRUNC"   ) == PROCEED) {
      flags |= O_TRUNC;
    } else if (OZ_unifyAtom(hd,"O_NOCTTY"  ) == PROCEED) {
      flags |= O_NOCTTY;
    } else if (OZ_unifyAtom(hd,"O_NONBLOCK") == PROCEED) {
      flags |= O_NONBLOCK;
    } else if (OZ_unifyAtom(hd,"O_SYNC"    ) == PROCEED) {
      flags |= O_SYNC;
    } else {
      OZ_warning("open: illegal flag");
      return FAILED;
    }

    OzFlags = tl;
  }

  if (OZ_isVariable(OzFlags)) {
    return SUSPEND;
  } else if (!OZ_isNil(OzFlags)) {
    return FAILED;
  }

  // Compute modes from their textual representation

  int mode = 0;
  while (unixIsCons(OzMode, &hd, &tl)) {

    if (OZ_isVariable(hd))
      return SUSPEND;
#ifdef OS2_I486
    OZ_warning("open: illegal mode");
    return FAILED;
#else
    if (OZ_unifyAtom(hd,"S_IRUSR") == PROCEED) { mode |= S_IRUSR; }
    else if (OZ_unifyAtom(hd,"S_IWUSR") == PROCEED) { mode |= S_IWUSR; }
    else if (OZ_unifyAtom(hd,"S_IXUSR") == PROCEED) { mode |= S_IXUSR; }
    else if (OZ_unifyAtom(hd,"S_IRGRP") == PROCEED) { mode |= S_IRGRP; }
    else if (OZ_unifyAtom(hd,"S_IWGRP") == PROCEED) { mode |= S_IWGRP; }
    else if (OZ_unifyAtom(hd,"S_IXGRP") == PROCEED) { mode |= S_IXGRP; }
    else if (OZ_unifyAtom(hd,"S_IROTH") == PROCEED) { mode |= S_IROTH; }
    else if (OZ_unifyAtom(hd,"S_IWOTH") == PROCEED) { mode |= S_IWOTH; }
    else if (OZ_unifyAtom(hd,"S_IXOTH") == PROCEED) { mode |= S_IXOTH; }
    else {
      OZ_warning("open: illegal mode");
      return FAILED;
    }
#endif
    OzMode = tl;
  }

  if (OZ_isVariable(OzMode)) {
    return SUSPEND;
  } else if (!OZ_isNil(OzMode)) {
    return FAILED;
  }

  WRAPCALL(open(filename, flags, mode),desc,out);

  return OZ_unifyInt(out,desc);
}
OZ_C_proc_end



OZ_C_ioproc_begin(unix_close,2)
{
  OZ_declareIntArg(0,fd);
  OZ_declareArg(1,out);

  WRAPCALL(close(fd),ret,out);

  return OZ_unifyInt(out,ret);
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_read,5)
{
  OZ_declareIntArg(0,fd);
  OZ_declareIntArg(1,maxx);
  OZ_declareArg(2, outHead);
  OZ_declareArg(3, outTail);
  OZ_declareArg(4, outN);

  CHECK_READ(fd,outN);

  char *buf = (char *) malloc(maxx+1);

  WRAPCALL(osread(fd, buf, maxx), ret, outN);

  OZ_Term hd = openbuff2list(ret, buf, outTail);

  free(buf);

  return ((OZ_unify(outHead, hd) == PROCEED)&&
          (OZ_unifyInt(outN,ret) == PROCEED)) ? PROCEED : FAILED;
}
OZ_C_proc_end



OZ_C_ioproc_begin(unix_write, 3)
{
  OZ_declareIntArg(0, fd);
  OZ_declareArg(1, vs);
  OZ_declareArg(2, out);

  CHECK_WRITE(fd,out,vs);

  int len;
  OZ_Return status;
  OZ_Term rest, susp;
  vs_buff(write_buff);

  status = buffer_vs(vs, write_buff, &len, &rest, &susp);

  if (status == FAILED)
    return FAILED;

  WRAPCALL(write(fd, write_buff, len), ret, out);

  if (len==ret && status != SUSPEND) {
    return OZ_unifyInt(out, len);
  }

  if (status != SUSPEND) {
    susp = OZ_nil();
    rest = susp;
  }

  if (len > ret) {
    OZ_Term rest_all = OZ_pair2(buff2list(len - ret, write_buff + ret),rest);

    RETURN_SUSPEND(out,OZ_int(ret),susp,rest_all);
  } else {
    RETURN_SUSPEND(out,OZ_int(ret),susp,rest);
  }
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_lSeek,4)
{
  OZ_declareIntArg(0, fd);
  OZ_declareIntArg(1, offset);
  OZ_declareAtomArg(2, OzWhence);
  OZ_declareArg(3, out);

  int whence;

  if (!strcmp(OzWhence,"SEEK_SET")) {
    whence=SEEK_SET;
  } else if (!strcmp(OzWhence,"SEEK_CUR")) {
    whence=SEEK_CUR;
  } else if (!strcmp(OzWhence,"SEEK_END")) {
    whence=SEEK_END;
  } else {
    OZ_warning("lSeek: illegal whence");
    return FAILED;
  }

  WRAPCALL(lseek(fd, offset, whence),ret,out);

  return OZ_unifyInt(out,ret);
}
OZ_C_proc_end


OZ_C_proc_begin(unix_readSelect,2)
{
  OZ_declareIntArg(0,fd);
  OZ_declareArg(1, out);

  WRAPCALL(osTestSelect(fd,SEL_READ),sel,out);

  if (sel == 0) {
    return OZ_readSelect(fd,OZ_int(0),out);
  }
  return OZ_unifyInt(out,0);
}
OZ_C_proc_end


OZ_C_proc_begin(unix_writeSelect,2)
{
  OZ_declareIntArg(0,fd);
  OZ_declareArg(1, out);

  WRAPCALL(osTestSelect(fd,SEL_WRITE),sel,out);

  if (sel == 0) {
    return OZ_writeSelect(fd,OZ_int(0),out);
  }
  return OZ_unifyInt(out,0);
}
OZ_C_proc_end



OZ_C_proc_begin(unix_deSelect,1)
{
  OZ_declareIntArg(0,fd);
  OZ_deSelect(fd);
  return PROCEED;
}
OZ_C_proc_end




// -------------------------------------------------
// sockets
// -------------------------------------------------

OZ_C_ioproc_begin(unix_socket,4)
{
  OZ_declareAtomArg(0, OzDomain);
  OZ_declareAtomArg(1, OzType);
  OZ_declareVsArg(2, OzProtocol);
  OZ_declareArg(3, out);

  int domain, type, protocol;

  // compute domain
  if (!strcmp(OzDomain,"PF_UNIX")) {
    domain = PF_UNIX;
  } else if (!strcmp(OzDomain,"PF_INET")) {
    domain = PF_INET;
  } else {
    OZ_warning("socket: unknown domain");
    return FAILED;
  }

  // compute type
  if (!strcmp(OzType,"SOCK_STREAM")) {
    type = SOCK_STREAM;
  } else if (!strcmp(OzType,"SOCK_DGRAM")) {
    type = SOCK_DGRAM;
  } else {
    OZ_warning("socket: unknown type");
    return FAILED;
  }

  // compute protocol
  if (*OzProtocol != '\0') {
    struct protoent *proto;

    proto = getprotobyname(OzProtocol);

    if (!proto) {
      OZ_warning("socket: unknown protocol");
      return FAILED;
    }

    protocol = proto->p_proto;
  } else if (*OzProtocol == '\0') {
    protocol = 0;
  } else {
    OZ_warning("socket: illegal protocol specification");
    return FAILED;
  }

  WRAPCALL(socket(domain, type, protocol), sock, out);

  return OZ_unifyInt(out, sock);
}
OZ_C_proc_end

OZ_C_ioproc_begin(unix_bindInet,3)
{
  OZ_declareIntArg(0,sock);
  OZ_declareIntArg(1,port);
  OZ_declareArg(2, out);

  struct sockaddr_in addr;

  memset((char *)&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons ((unsigned short) port);

  WRAPCALL(bind(sock,(struct sockaddr *)&addr,sizeof(struct
                                                     sockaddr_in)),ret,out);
  return OZ_unifyInt(out,ret);
}
OZ_C_proc_end



#ifndef OS2_I486
#ifndef WINDOWS
OZ_C_ioproc_begin(unix_bindUnix,3)
{
  OZ_declareIntArg(0,s);
  OZ_declareVsArg(1,path);
  OZ_declareArg(2, out);

  struct sockaddr_un addr;

  memset((char *)&addr, 0, sizeof (addr));
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, path);

  WRAPCALL(bind(s, (struct sockaddr *)&addr, sizeof(struct
                                                    sockaddr_un)),ret,out);
  return OZ_unifyInt(out,ret);
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_connectUnix,3)
{
  OZ_declareIntArg(0,s);
  OZ_declareVsArg(1,path);
  OZ_declareArg(2, out);

  struct sockaddr_un addr;

  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, path);

// critical region
  osBlockSignals();

  int ret;
  while ((ret = connect(s,(struct sockaddr *) &addr,
                        sizeof(struct sockaddr_un)))<0) {
    if (errno != EINTR) {
      osUnblockSignals();
      RETURN_UNIX_ERROR(out);
    }
  }

// end of critical region
  osUnblockSignals();

  return OZ_unifyInt(out,ret);
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_acceptUnix,3)
{
  OZ_declareIntArg(0, sock);
  OZ_declareArg(1, path);
  OZ_declareArg(2, out);

  struct sockaddr_un from;
  int fromlen = sizeof from;

  WRAPCALL(accept(sock,(struct sockaddr *)&from, &fromlen), fd, out);

  return (OZ_unify(path, OZ_string(from.sun_path)) == PROCEED
    && OZ_unifyInt(out, fd) == PROCEED) ? PROCEED: FAILED;
}
OZ_C_proc_end

#endif  /* WINDOWS */

OZ_C_ioproc_begin(unix_getSockName,2)
{
  OZ_declareIntArg(0,s);
  OZ_Term out = OZ_getCArg(1);

  struct sockaddr_in addr;
  int length = sizeof(addr);

  WRAPCALL(getsockname(s, (struct sockaddr *) &addr, &length), ret, out);

  return OZ_unifyInt(out,ntohs(addr.sin_port));
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_listen,3)
{
  OZ_declareIntArg(0, s);
  OZ_declareIntArg(1, n);
  OZ_declareArg(2, out);

  WRAPCALL(listen(s,n), ret, out);

  return OZ_unifyInt(out,ret);
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_connectInet,4)
{
  OZ_declareIntArg(0, s);
  OZ_declareVsArg(1, host);
  OZ_declareIntArg(2, port);
  OZ_declareArg(3, out);

  struct hostent *hostaddr;

  if ((hostaddr = gethostbyname(host)) == NULL) {
    RETURN_NET_ERROR(out);
  }

  struct sockaddr_in addr;
  memset((char *)&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  memcpy(&addr.sin_addr,hostaddr->h_addr_list[0],sizeof(addr.sin_addr));
  addr.sin_port = htons ((unsigned short) port);

// critical region
  osBlockSignals();

  int ret;
  while ((ret = connect(s,(struct sockaddr *) &addr,sizeof(addr)))<0) {
    if (errno != EINTR) {
      osUnblockSignals();
      RETURN_UNIX_ERROR(out);
    }
  }

// end of critical region
  osUnblockSignals();

  return OZ_unifyInt(out,ret);
}
OZ_C_proc_end



OZ_C_ioproc_begin(unix_acceptInet,4)
{
  OZ_declareIntArg(0, sock);
  OZ_declareArg(1, host);
  OZ_declareArg(2, port);
  OZ_declareArg(3, out);

  struct sockaddr_in from;
  int fromlen = sizeof from;

  WRAPCALL(accept(sock,(struct sockaddr *)&from, &fromlen),fd,out);

  struct hostent *gethost = gethostbyaddr((char *) &from.sin_addr,
                                          fromlen, AF_INET);
  if (gethost) {
    return (OZ_unifyInt(port, ntohs(from.sin_port)) == PROCEED
            && OZ_unify(host, OZ_string((char*)gethost->h_name)) == PROCEED
            && OZ_unifyInt(out, fd) == PROCEED) ? PROCEED : FAILED;
  } else {
    return (OZ_unifyInt(port, ntohs(from.sin_port)) == PROCEED
            && OZ_unify(host, OZ_string(inet_ntoa(from.sin_addr))) == PROCEED
            && OZ_unifyInt(out, fd) == PROCEED) ? PROCEED : FAILED;
  }
}
OZ_C_proc_end


static OZ_Return get_send_recv_flags(OZ_Term OzFlags, int * flags)
{
  OZ_Term hd, tl;

  *flags = 0;

  while (unixIsCons(OzFlags, &hd, &tl)) {

    if (OZ_isVariable(hd))
      return SUSPEND;

    if (OZ_unifyAtom(hd,"MSG_OOB") == PROCEED) {
      *flags |= MSG_OOB;
    } else if (OZ_unifyAtom(hd,"MSG_PEEK") == PROCEED) {
      *flags |= MSG_PEEK;
    } else {
      OZ_warning("send or receive: illegal flag");
      return FAILED;
    }

    OzFlags = tl;
  }

  if (OZ_isVariable(OzFlags))
    return SUSPEND;

  if (!(OZ_nil()))
    return FAILED;

  return PROCEED;
}


OZ_C_ioproc_begin(unix_send, 4)
{
  OZ_declareIntArg(0, sock);
  OZ_declareArg(1, vs);
  OZ_declareArg(2, OzFlags);
  OZ_declareArg(3, out);


  int flags;
  OZ_Return flagBool;

  if (!((flagBool = get_send_recv_flags(OzFlags,&flags)) == PROCEED))
      return flagBool;

  CHECK_WRITE(sock,out,vs);

  int len;
  OZ_Return status;
  OZ_Term rest, susp, from_buff, rest_all;
  vs_buff(write_buff);

  status = buffer_vs(vs, write_buff, &len, &rest, &susp);

  if (status == FAILED)
    return FAILED;

  WRAPCALL(send(sock, write_buff, len, flags), ret, out);

  if (len==ret && status != SUSPEND) {
    return OZ_unifyInt(out, len);
  }

  if (status != SUSPEND) {
    susp = OZ_nil();
    rest = susp;
  }

  if (len > ret) {
    from_buff = buff2list(len - ret, write_buff + ret);

    rest_all = OZ_pair2(from_buff,rest);

    RETURN_SUSPEND(out,OZ_int(ret),susp,rest_all);
  } else {
    RETURN_SUSPEND(out,OZ_int(ret),susp,rest);
  }

}
OZ_C_proc_end

OZ_C_ioproc_begin(unix_sendToInet, 6)
{
  OZ_declareIntArg(0, sock);
  OZ_declareArg(1, vs);
  OZ_declareArg(2, OzFlags);
  OZ_declareVsArg(3, host);
  OZ_declareIntArg(4, port);
  OZ_declareArg(5, out);

  int flags;
  OZ_Return flagBool;

  if (!((flagBool = get_send_recv_flags(OzFlags,&flags)) == PROCEED))
      return flagBool;

  CHECK_WRITE(sock,out,vs);

  struct hostent *hostaddr;

  if ((hostaddr = gethostbyname(host)) == NULL) {
    RETURN_NET_ERROR(out);
  }

  struct sockaddr_in addr;
  memset((char *)&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  memcpy(&addr.sin_addr, hostaddr->h_addr_list[0],
         sizeof(addr.sin_addr));
  addr.sin_port = htons ((unsigned short) port);


  int len;
  OZ_Return status;
  OZ_Term rest, susp;
  vs_buff(write_buff);

  status = buffer_vs(vs, write_buff, &len, &rest, &susp);

  if (status == FAILED)
    return FAILED;

  WRAPCALL(sendto(sock, write_buff, len, flags,
                  (struct sockaddr *) &addr, sizeof(addr)), ret, out);

  if (len==ret && status != SUSPEND) {
    return OZ_unifyInt(out, len);
  }

  if (status != SUSPEND) {
    susp = OZ_nil();
    rest = susp;
  }

  if (len > ret) {
    OZ_Term rest_all = OZ_pair2(buff2list(len - ret, write_buff + ret),rest);

    RETURN_SUSPEND(out,OZ_int(ret),susp,rest_all);
  } else {
    RETURN_SUSPEND(out,OZ_int(ret),susp,rest);
  }

}
OZ_C_proc_end

#ifndef WINDOWS
OZ_C_ioproc_begin(unix_sendToUnix, 5)
{
  OZ_declareIntArg(0, sock);
  OZ_declareArg(1, vs);
  OZ_declareArg(2, OzFlags);
  OZ_declareVsArg(3, path);
  OZ_declareArg(4, out);

  int flags;
  OZ_Return flagBool;

  if (!((flagBool = get_send_recv_flags(OzFlags,&flags)) == PROCEED))
      return flagBool;

  CHECK_WRITE(sock,out,vs);

  struct sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, path);

  int len;
  OZ_Return status;
  OZ_Term rest, susp;
  vs_buff(write_buff);

  status = buffer_vs(vs, write_buff, &len, &rest, &susp);

  if (status == FAILED)
    return FAILED;

  WRAPCALL(sendto(sock, write_buff, len, flags,
                  (struct sockaddr *) &addr, sizeof(addr)), ret, out);

  if (len==ret && status != SUSPEND) {
    return OZ_unifyInt(out, len);
  }

  if (status != SUSPEND) {
    susp = OZ_nil();
    rest = susp;
  }

  if (len > ret) {
    OZ_Term rest_all = OZ_pair2(buff2list(len - ret, write_buff + ret), rest);

    RETURN_SUSPEND(out,OZ_int(ret),susp,rest_all);
  } else {
    RETURN_SUSPEND(out,OZ_int(ret),susp,rest);
  }

}
OZ_C_proc_end
#endif  /* WINDOWS */


OZ_C_ioproc_begin(unix_shutDown, 3)
{
  OZ_declareIntArg(0,sock);
  OZ_declareIntArg(1,how);
  OZ_declareArg(2, out);

  WRAPCALL(shutdown(sock, how), ret, out);

  return OZ_unifyInt(out, ret);
}
OZ_C_proc_end



OZ_C_ioproc_begin(unix_receiveFromInet,8)
{
  OZ_declareIntArg(0,sock);
  OZ_declareIntArg(1,maxx);
  OZ_declareArg(2, OzFlags);
  OZ_declareArg(3, hd);
  OZ_declareArg(4, tl);
  OZ_declareArg(5, host);
  OZ_declareArg(6, port);
  OZ_declareArg(7, outN);

  int flags;
  OZ_Return flagBool;

  if (!((flagBool = get_send_recv_flags(OzFlags,&flags)) == PROCEED))
      return flagBool;

  CHECK_READ(sock,outN);

  char *buf = (char *) malloc(maxx+1);

  struct sockaddr_in from;
  int fromlen = sizeof from;

  WRAPCALL(recvfrom(sock, buf, maxx, flags,
                    (struct sockaddr*)&from, &fromlen),ret,outN);

  struct hostent *gethost = gethostbyaddr((char *) &from.sin_addr,
                                            fromlen, AF_INET);

  OZ_Term localhead = openbuff2list(ret, buf, tl);

  free(buf);

  return (OZ_unify(localhead, hd) == PROCEED
          && OZ_unifyInt(port, ntohs(from.sin_port)) == PROCEED
          && OZ_unify(host, OZ_string(gethost ?
                                        gethost->h_name :
                                        inet_ntoa(from.sin_addr))) == PROCEED
          && OZ_unifyInt(outN, ret) == PROCEED) ? PROCEED : FAILED;

}
OZ_C_proc_end

#ifndef WINDOWS
OZ_C_ioproc_begin(unix_receiveFromUnix,7)
{
  OZ_declareIntArg(0,sock);
  OZ_declareIntArg(1,maxx);
  OZ_declareArg(2, OzFlags);
  OZ_declareArg(3, hd);
  OZ_declareArg(4, tl);
  OZ_declareArg(5, path);
  OZ_declareArg(6, outN);

  int flags;
  OZ_Return flagBool;

  if (!((flagBool = get_send_recv_flags(OzFlags,&flags)) == PROCEED))
      return flagBool;

  CHECK_READ(sock,outN);

  char *buf = (char *) malloc(maxx+1);

  struct sockaddr_un from;
  int fromlen = sizeof from;

  WRAPCALL(recvfrom(sock, buf, maxx, flags,
                    (struct sockaddr*)&from, &fromlen),ret,outN);

  OZ_Term localhead = openbuff2list(ret, buf, tl);

  free(buf);

  return (OZ_unify(localhead, hd) == PROCEED
          && OZ_unify(path, OZ_string(from.sun_path)) == PROCEED
          && OZ_unifyInt(outN, ret) == PROCEED) ? PROCEED : FAILED;

}
OZ_C_proc_end

#endif   /* WINDOWS */
#endif   /* OS2 */


const int maxArgv = 100;
static char* argv[maxArgv];

OZ_C_ioproc_begin(unix_pipe,4)
{
  OZ_declareVsArg(0,s);
  OZ_declareArg(1, args);
  OZ_declareArg(2, rpid);
  OZ_declareArg(3, rwsock);

  OZ_Term hd, tl, argl;
  int argno = 0;

  argl=args;

  while (unixIsCons(argl, &hd, &tl)) {
    if (OZ_isVariable(hd)) return SUSPEND;
    argno++;
    argl = tl;
  }

  if (OZ_isVariable(argl))
    return SUSPEND;

  if (!OZ_isNil(argl))
    return FAILED;

  argl=args;

  if (argno+2 >= maxArgv) {
    OZ_warning("pipe: can only handle up to %d arguments, got: %d",maxArgv,argno+2);
    return FAILED;
  }
  argv[0] = s;
  argv[argno+1] = 0;

  argno = 1;

  while (unixIsCons(argl, &hd, &tl)) {
    int len;
    OZ_Return status;
    OZ_Term rest, susp;

    char *vsarg = (char *) malloc(max_vs_length + 256);

    status = buffer_vs(hd, vsarg, &len, &rest, &susp);

    if (status == SUSPEND) {
      free(vsarg);
      if (OZ_isVariable(susp)) {
        return SUSPEND;
      } else {
        OZ_warning("pipe: virtual string too long in arg 2");
        return FAILED;
      }
    } else if (status == FAILED) {
      free(vsarg);
      return FAILED;
    }
    *(vsarg+len) = '\0';

    argv[argno++] = vsarg;

    argl = tl;
  }

#ifdef WINDOWS
  int k;
  char buf[10000];
  buf[0] = '\0';
  for (k=0 ; k<argno; k++) {
    strcat(buf,argv[k]);
  }

  STARTUPINFO si;
  SECURITY_ATTRIBUTES sa;
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = TRUE;
  PROCESS_INFORMATION pinf;

  HANDLE saveout = GetStdHandle(STD_OUTPUT_HANDLE);
  HANDLE savein  = GetStdHandle(STD_INPUT_HANDLE);
  HANDLE rh1,wh1,rh2,wh2;
  if (!CreatePipe(&rh1,&wh1,&sa,0)  ||
      !CreatePipe(&rh2,&wh2,&sa,0)  ||
      !SetStdHandle(STD_OUTPUT_HANDLE,wh1) ||
      !SetStdHandle(STD_INPUT_HANDLE,rh2) ||
      !CreateProcess(NULL,buf,&sa,NULL,TRUE,DETACHED_PROCESS,
                     NULL,NULL,&si,&pinf)) {
    OZ_warning("createProcess failed: %d\n",GetLastError());
    return FAILED;
  }

  int pid = (int) pinf.hProcess;
  CloseHandle(wh1);
  CloseHandle(rh2);
  SetStdHandle(STD_OUTPUT_HANDLE,saveout);
  SetStdHandle(STD_INPUT_HANDLE,savein);

  int rsock = _hdopen((int)rh1,O_RDONLY|O_BINARY);
  int wsock = _hdopen((int)wh2,O_WRONLY|O_BINARY);
  if (rsock<0 || wsock<0) {
    perror("_hdopen");
    return FAILED;
  }

#else  /* !WINDOWS */

  int sv[2];
  WRAPCALL(socketpair(PF_UNIX,SOCK_STREAM,0,sv),ret,rpid);


  int pid =  fork();
  switch (pid) {
  case 0: // child
    {
      int i;
      for (i = 0; i < FD_SETSIZE; i++) {
        if (i != sv[1]) {
          close(i);
        }
      }
      dup(sv[1]);
      dup(sv[1]);
      dup(sv[1]);
      if (execvp(s,argv)  < 0) {
        RETURN_UNIX_ERROR(rpid);
      }
    }
    return FAILED;
  case -1:
    RETURN_UNIX_ERROR(rpid);
  default: // parent
    break;
  }
  close(sv[1]);

  int rsock = sv[0];
  /* we cann use the same descriptor for both reading and writing: */
  int wsock = rsock;
#endif

  int i;
  for (i=1 ; i<argno ; i++)
    free(argv[i]);

  addChildProc(pid);

  TaggedRef rw = OZ_pair2(OZ_int(rsock),OZ_int(wsock));
  return OZ_unifyInt(rpid,pid) == PROCEED
    && OZ_unify(rwsock,rw) == PROCEED ? PROCEED : FAILED;
}
OZ_C_proc_end

static OZ_Term mkAliasList(char **alias)
{
  OZ_Term ret = OZ_nil();
  while (*alias != 0) {
    ret = OZ_cons(OZ_string(*alias), ret);
    alias++;
  }
  return ret;
}

static OZ_Term mkAddressList(char **lstptr)
{
  OZ_Term ret = OZ_nil();
  while (*lstptr != NULL) {
    ret = OZ_cons(OZ_string(inet_ntoa(**((struct in_addr **) lstptr))),
                  ret);
    lstptr++;
  }
  return ret;
}

OZ_C_ioproc_begin(unix_getHostByName, 2)
{
  OZ_declareVsArg(0, name);
  OZ_declareArg(1, out);

  struct hostent *hostaddr;

  if ((hostaddr = gethostbyname(name)) == NULL) {
    RETURN_NET_ERROR(out);
  }

  OZ_Term t1=OZ_pairAS("name",hostaddr->h_name);
  OZ_Term t2=OZ_pairA("aliases",mkAliasList(hostaddr->h_aliases));
  OZ_Term t3=OZ_pairA("addrList",mkAddressList(hostaddr->h_addr_list));
  OZ_Term pairlist= OZ_cons(t1,OZ_cons(t2,OZ_cons(t3,OZ_nil())));

  return OZ_unify(out,OZ_recordInit(OZ_atom("hostent"),pairlist));
}
OZ_C_proc_end



// Misc stuff

OZ_C_ioproc_begin(unix_unlink, 2)
{
  OZ_declareVsArg(0,path);
  OZ_declareArg(1, out);

  WRAPCALL(unlink(path),ret,out);
  return OZ_unifyInt(out, ret);
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_system,2)
{
  OZ_declareVsArg(0, vs);
  OZ_declareArg(1, out);

  int ret = osSystem(vs);

  return OZ_unifyInt(out,ret);
}
OZ_C_proc_end

#ifndef WINDOWS
OZ_C_ioproc_begin(unix_wait,2)
{
  OZ_declareArg(0, rpid);
  OZ_declareArg(1, rstat);

  int status;
  int pid = waitpid(-1, &status, WNOHANG | WUNTRACED);

  return (OZ_unifyInt(rpid,pid) == PROCEED
          && OZ_unifyInt(rstat,status) == PROCEED) ? PROCEED : FAILED;
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_getServByName, 3)
{
  OZ_declareVsArg(0, name);
  OZ_declareVsArg(1, proto);
  OZ_Term out = OZ_getCArg(2);

  struct servent *serv;
  serv = getservbyname(name, proto);

  if (!serv)
    return OZ_unify(out, OZ_false());

  return OZ_unifyInt(out, ntohs(serv->s_port));
}
OZ_C_proc_end
#endif

#ifdef WINDOWS
/* ignore dir and prefix! */
#define tempnam(dir,prefix) tmpnam(NULL)
#endif

OZ_C_ioproc_begin(unix_tempName, 3)
{
  OZ_declareVsArg(0, directory);
  OZ_declareVsArg(1, prefix);
  OZ_declareArg(2, name);

  char *filename;

  if (strlen(prefix) > 5)
    return FAILED;

  if (!(filename = tempnam(directory, prefix))) {
    OZ_warning("tempName: no file name accessible");
    return FAILED;
  }
  filename = ozstrdup(filename);

  return OZ_unify(name, OZ_string(filename));
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_getEnv,2)
{
  OZ_declareVsArg(0, envVar);

  char *envValue;

  envValue = getenv(envVar);
  if (envValue == 0)
    return OZ_unify(OZ_getCArg(1),OZ_false());

  return OZ_unify(OZ_getCArg(1),OZ_string(envValue));
}
OZ_C_proc_end


/* putenv is NOT POSIX !!! */
OZ_C_ioproc_begin(unix_putEnv,2)
{
  OZ_declareVsArg(0, envVar);
  OZ_declareVsArg(1, envValue);

  char *buf = new char[strlen(envVar)+strlen(envValue)+2];
  sprintf(buf,"%s=%s",envVar,envValue);
  int ret = putenv(buf);
  if (ret != 0) {
    delete buf;
    return FAILED;
  }

  return PROCEED;
}
OZ_C_proc_end


static OZ_Term make_time(const struct tm* tim)
{
  OZ_Term t1=OZ_pairAI("sec",tim->tm_sec);
  OZ_Term t2=OZ_pairAI("min",tim->tm_min);
  OZ_Term t3=OZ_pairAI("hour",tim->tm_hour);
  OZ_Term t4=OZ_pairAI("mDay",tim->tm_mday);
  OZ_Term t5=OZ_pairAI("mon",tim->tm_mon);
  OZ_Term t6=OZ_pairAI("year",tim->tm_year);
  OZ_Term t7=OZ_pairAI("wDay",tim->tm_wday);
  OZ_Term t8=OZ_pairAI("yDay",tim->tm_yday);
  OZ_Term t9=OZ_pairAI("isDst",tim->tm_isdst);

  OZ_Term l1=OZ_cons(t6,OZ_cons(t7,OZ_cons(t8,OZ_cons(t9,OZ_nil()))));
  OZ_Term l2=OZ_cons(t1,OZ_cons(t2,OZ_cons(t3,OZ_cons(t4,OZ_cons(t5,l1)))));
  return OZ_recordInit(OZ_atom("time"),l2);
}

OZ_C_ioproc_begin(unix_gmTime, 1)
{
  OZ_Term out = OZ_getCArg(0);
  time_t timebuf;

  time(&timebuf);
  return OZ_unify(out,make_time(gmtime(&timebuf)));

}
OZ_C_proc_end

OZ_C_ioproc_begin(unix_localTime, 1)
{
  OZ_Term out = OZ_getCArg(0);
  time_t timebuf;

  time(&timebuf);
  return OZ_unify(out,make_time(localtime(&timebuf)));

}
OZ_C_proc_end

OZ_C_proc_begin(unix_rand, 1)
{
  OZ_Term out = OZ_getCArg(0);

  return OZ_unifyInt(out,rand());
}
OZ_C_proc_end

OZ_C_proc_begin(unix_srand, 1)
{
  OZ_declareIntArg(0, seed);

  if (seed) {
    srand((unsigned int) seed);
  } else {
    srand((unsigned int) time(NULL));
  }

  return PROCEED;
}
OZ_C_proc_end

#ifndef RAND_MAX
#ifdef SUNOS_SPARC
#define RAND_MAX ((1<<31)-1)
#else
#ifdef AIX3_RS6000
#   define RAND_MAX     32767
#else
... fill in RAND_MAX ...
#endif
#endif
#endif

OZ_C_proc_begin(unix_randLimits, 2)
{
  OZ_Term minn = OZ_getCArg(0);
  OZ_Term maxx = OZ_getCArg(1);

  if (OZ_unifyInt(minn,0) == FAILED) {
    return FAILED;
  }
  return OZ_unifyInt(maxx,RAND_MAX);
}
OZ_C_proc_end




#define NotAvail(Name,Arity,Fun)                                \
OZ_C_ioproc_begin(Fun,Arity)                                    \
{                                                               \
  OZ_warning("procedure %s not available under Windows",Name);  \
  return FAILED;                                                \
}                                                               \
OZ_C_proc_end


#ifdef WINDOWS
NotAvail("bindUnix",3,unix_bindUnix);
NotAvail("sendToUnix",5,unix_sendToUnix);
NotAvail("connectUnix",3,unix_connectUnix);
NotAvail("acceptUnix",3,unix_acceptUnix);
NotAvail("receiveFromUnix",7,unix_receiveFromUnix);
NotAvail("wait",2,unix_wait);
NotAvail("getServByName",3,unix_getServByName);
NotAvail("uName",1,unix_uName);
#endif

OZ_BIspec spec[] = {
  {"unix_getDir",2,unix_getDir},
  {"unix_stat",2,unix_stat},
  {"unix_getCWD",1,unix_getCWD},
  {"unix_open",4,unix_open},
  {"unix_fileDesc",2,unix_fileDesc},
  {"unix_close",2,unix_close},
  {"unix_write",3,unix_write},
  {"unix_read",5,unix_read},
  {"unix_lSeek",4,unix_lSeek},
  {"unix_unlink",2,unix_unlink},
  {"unix_readSelect",2,unix_readSelect},
  {"unix_writeSelect",2,unix_writeSelect},
  {"unix_deSelect",1,unix_deSelect},
  {"unix_system",2,unix_system},
  {"unix_getEnv",2,unix_getEnv},
  {"unix_putEnv",2,unix_putEnv},
  {"unix_gmTime",1,unix_gmTime},
  {"unix_localTime",1,unix_localTime},
  {"unix_srand",1,unix_srand},
  {"unix_rand",1,unix_rand},
  {"unix_randLimits",2,unix_randLimits},
  {"unix_socket",4,unix_socket},
  {"unix_bindInet",3,unix_bindInet},
  {"unix_listen",3,unix_listen},
  {"unix_connectInet",4,unix_connectInet},
  {"unix_acceptInet",4,unix_acceptInet},
  {"unix_shutDown",3,unix_shutDown},
  {"unix_send",4,unix_send},
  {"unix_sendToInet",6,unix_sendToInet},
  {"unix_receiveFromInet",8,unix_receiveFromInet},
  {"unix_getSockName",2,unix_getSockName},
  {"unix_getHostByName",2,unix_getHostByName},
  {"unix_bindUnix",3,unix_bindUnix},
  {"unix_sendToUnix",5,unix_sendToUnix},
  {"unix_connectUnix",3,unix_connectUnix},
  {"unix_acceptUnix",3,unix_acceptUnix},
  {"unix_receiveFromUnix",7,unix_receiveFromUnix},
  {"unix_pipe",4,unix_pipe},
  {"unix_tempName",3,unix_tempName},
  {"unix_wait",2,unix_wait},
  {"unix_getServByName",3,unix_getServByName},
  {"unix_uName",1,unix_uName},
  {0,0,0}
};

void BIinitUnix()
{
  OZ_addBISpec(spec);
}
