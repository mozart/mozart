/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl, schulte
  Last modified: $Date$ from $Author$
  Version: $Revision$
  */

#include "oz.h"
#include "sun-proto.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>

#if defined(LINUX_I486) || defined(HPUX_700)
extern int h_errno;
#endif

#include <time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <sys/uio.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <signal.h>

#ifdef IRIX5_MIPS
#include <bstring.h>
#endif


extern "C" char *inet_ntoa(struct in_addr in);
extern "C" char *ozStrerror(int errno);


#define max_vs_length 4096*4
#define vs_buff(VAR) char VAR[max_vs_length + 256];


//
// Argument handling
//

#define OZ_declareVsArg(FUN,ARG,VAR) \
 vs_buff(VAR); OZ_nonvarArg(ARG);                                     \
 { int len; OZ_Bool status; OZ_Term rest, susp;                       \
   status = buffer_vs(OZ_getCArg(ARG), VAR, &len, &rest, &susp);      \
   if (status == SUSPEND) {                                           \
     if (OZ_isVariable(susp)) {                                            \
       return SUSPEND;                                                \
     } else {                                                         \
       OZ_warning("%s: virtual string too long in arg %d",FUN,ARG+1); \
       return FAILED;                                                 \
     }                                                                \
   } else if (status == FAILED) {                                     \
     return FAILED;                                                   \
   }                                                                  \
   *(VAR+len) = '\0';                                                 \
 }

#define OZ_declareArg(ARG,VAR) \
     OZ_Term VAR = OZ_getCArg(ARG);

#define IsPair(s) (s[0]=='#' && s[1]=='\0')



static inline char *ozstrdup(char *s)
{
  char *ret = new char[strlen(s)+1];
  strcpy(ret,s);
  return ret;
}

// checking

inline int unixIsCons(OZ_Term cons, OZ_Term *head, OZ_Term *tail)
{
  if (!OZ_isCons(cons)) {
    return 0;
  }
  *head = OZ_head(cons);
  *tail = OZ_tail(cons);
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
#define RETURN_ANY_ERROR(OUT,NUMBER,STRING,GROUP)                    \
{  OZ_Term err_tuple = OZ_tupleC("error",3);                      \
   if (OZ_putArg(err_tuple, 1, OZ_CToInt(NUMBER)) &&              \
       OZ_putArg(err_tuple, 2, OZ_CToList(STRING)) &&               \
       OZ_putArg(err_tuple, 3, OZ_CToList(GROUP))) {                \
     return OZ_unify(OUT, err_tuple);                                \
   } else {                                                          \
     return FAILED;                                                  \
   }                                                                 \
 }

// return upon unix-error
#define RETURN_UNIX_ERROR(OUT) \
{ RETURN_ANY_ERROR(OUT,errno,ozStrerror(errno),"unix"); }


#if defined(SUNOS_SPARC) || defined(SOLARIS_SPARC) || defined(LINUX_I486) || defined(HPUX_700) || defined(IRIX5_MIPS)

static char* h_strerror(const int err) {
  switch (err) {
  case HOST_NOT_FOUND:
    return "No such host is known.";
  case TRY_AGAIN:
    return "Retry later again.";
  case NO_RECOVERY:
    return "Unexpected non-recoverable server failure.";
#if defined(SOLARIS_SPARC) || defined(LINUX_I486)
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

#ifdef ULTRIX_MIPS
#define RETURN_NET_ERROR(OUT) \
{ RETURN_ANY_ERROR(OUT,0,"Host lookup failure.","host"); }
#endif


// return unable to perform service
#define RETURN_UNABLE(OUT) \
{ return OZ_unifyString(OUT,"unable"); }

// return suspension upon
#define RETURN_SUSPEND(OUT,LEN,VAR,REST) \
{ OZ_Term susp_tuple = OZ_tupleC("suspend",3); \
  if (OZ_putArg(susp_tuple,1,LEN) &&              \
      OZ_putArg(susp_tuple,2,VAR) &&              \
      OZ_putArg(susp_tuple,3,REST)) {             \
    return OZ_unify(OUT,susp_tuple);              \
  } else {                                        \
    return FAILED;                                \
  }                                               \
}

// return suspension upon
#define RETURN_UNABLE_REST(OUT,REST) \
{ OZ_Term un_tuple = OZ_tupleC("unable", 1);   \
  if (OZ_putArg(un_tuple,1,REST)) {               \
    return OZ_unify(OUT,un_tuple);                \
  } else {                                        \
    return FAILED;                                \
  }                                               \
}



// -------------------------------------------------
// check file descriptors
// -------------------------------------------------



#define CHECK_READ(FD,OUT) \
{ fd_set fds;                                                    \
  struct timeval timeout;                                        \
  FD_ZERO(&fds); FD_SET(FD,&fds);                                \
  timeout.tv_sec=0; timeout.tv_usec=0;                           \
  WRAPCALL(ozSelect(FD+1, &fds, NULL, NULL, &timeout),ret,OUT);  \
  if (ret == 0) { RETURN_UNABLE(OUT); }                          \
}

#define CHECK_WRITE(FD,OUT,REST) \
{ fd_set fds;                                                    \
  struct timeval timeout;                                        \
  FD_ZERO(&fds); FD_SET(FD,&fds);                                \
  timeout.tv_sec=0; timeout.tv_usec=0;                           \
  WRAPCALL(ozSelect(FD+1, NULL, &fds, NULL, &timeout),ret,OUT);  \
  if (ret == 0) { RETURN_UNABLE_REST(OUT,REST); }                \
}




// -------------------------------------------------
// buffers to list of (unsigned char)
// -------------------------------------------------

static OZ_Term OZ_CToList(const char *s)
{
  // gives back a string as a list (an Oz string), i.e. scans the string
  // until the usual C-terminator '\0' is encountered
  OZ_Term head, prev, tail;

  if (*s == '\0')
    return OZ_nil();

  head = OZ_tupleC("|", 2);
  OZ_putArg(head, 1, OZ_CToInt((unsigned char) *s++));
  prev = head;
  tail = head;

  while (*s) {
    tail = OZ_tupleC("|", 2);
    OZ_putArg(tail, 1, OZ_CToInt((unsigned char) *s++));
    OZ_putArg(prev, 2, tail);
    prev = tail;
  }

  OZ_putArg(tail, 2, OZ_nil());
  return head;
}

static OZ_Term openbuff2list(int len, const char *s, const OZ_Term tail)
{
  // gives back a list of length len which elments are taken from a C-string
  // the tail of the list is given by list
  OZ_Term prev, head;

  if (len == 0)
    return tail;

  head = OZ_tupleC("|", 2);
  OZ_putArg(head, 1, OZ_CToInt((unsigned char) *s++));
  prev = head;

  while (--len) {
    OZ_Term next = OZ_tupleC("|", 2);

    OZ_putArg(next, 1, OZ_CToInt((unsigned char) *s++));
    OZ_putArg(prev, 2, next);
    prev = next;
  }

  OZ_putArg(prev, 2, tail);
  return head;
}


//
// Handling of virtual strings
//

inline OZ_Term buff2list(int len, const char *s)
{
  return openbuff2list(len, s, OZ_nil());
}



OZ_Bool atom2buff(OZ_Term atom, char **write_buff, int *len,
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
    *susp = OZ_CToList(string);
    *rest = *susp;
    return SUSPEND;
  }

  return PROCEED;
}


OZ_Bool int2buff(OZ_Term ozint, char **write_buff, int *len,
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
    *susp = OZ_CToList(help);
    *rest = *susp;
    OZ_free(string);
    return SUSPEND;
  }

  OZ_free(string);
  return PROCEED;
}

OZ_Bool float2buff(OZ_Term ozfloat, char **write_buff, int *len,
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
    *susp = OZ_CToList(help);
    *rest = *susp;
    OZ_free(string);
    return SUSPEND;
  }

  OZ_free(string);
  return PROCEED;
}


OZ_Bool list2buff(OZ_Term list, char **write_buff, int *len,
                         OZ_Term *rest, OZ_Term *susp)
{
  OZ_Term head, tail;

  while (unixIsCons(list, &head, &tail)) {
    if ((*len == max_vs_length) || OZ_isVariable(head)) {
      *susp = head;
      *rest = list;
      return SUSPEND;
    }

    int c;

    if (OZ_isInt(head)) {
      c = OZ_intToC(head);
      if ((c >= 0) && (c < 256)) {
        **write_buff = (unsigned char) c;
        (*write_buff)++;
        (*len)++;
        list = tail;
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


static OZ_Bool vs2buff(OZ_Term vs, char **write_buff, int *len,
                       OZ_Term *rest, OZ_Term *susp)
{
  static
  char *label;
  int width;

  if (OZ_isNil(vs)) {
    return PROCEED;
  } else if (OZ_isAtom(vs)) {
    return atom2buff(vs, write_buff, len, rest, susp);
  } else if (OZ_isInt(vs)) {
    return int2buff(vs, write_buff, len, rest, susp);
  } else if (OZ_isFloat(vs)) {
    return float2buff(vs, write_buff, len, rest, susp);
  } else if (OZ_isTuple(vs) && (label = OZ_atomToC(OZ_label(vs)))) {
    width = OZ_width(vs);
    if (IsPair(label) && width > 0) {
      int i,j;
      OZ_Term arg_susp, arg_rest;

      for (i=1; i<=width; i++) {

        OZ_Bool status = vs2buff(OZ_getArg(vs,i), write_buff, len,
                                 &arg_rest, &arg_susp);
        if (status == SUSPEND) {
          *susp = arg_susp;

          if (i==width) {
            *rest = arg_rest;
          } else {
            *rest = OZ_tupleC("#", (width - i) + 1);

            OZ_putArg(*rest, 1, arg_rest);
            i++;
            for (j=2 ; i <= width ; (j++, i++)) {
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
  } else if (OZ_isVariable(vs)) {
    *rest = vs;
    *susp = vs;
    return SUSPEND;
  }

  OZ_warning("skipping illegal virtual string");
  return FAILED;
}


inline OZ_Bool buffer_vs(OZ_Term vs, char *write_buff, int *len,
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
  OZ_declareStringArg("fileDesc", 0, OzFileDesc);
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


OZ_C_ioproc_begin(unix_open,4)
{
  OZ_declareVsArg("open", 0, filename);
  OZ_declareArg(1, OzFlags);
  OZ_declareArg(2, OzMode);
  OZ_declareArg(3, out);

  // Compute flags from their textual representation

  int flags = 0;
  OZ_Term head, tail;

  while (unixIsCons(OzFlags, &head, &tail)) {

    if (OZ_isVariable(head)) return SUSPEND;

    if (OZ_unifyString(head,"O_RDONLY") == PROCEED) {
      flags |= O_RDONLY;
    } else if (OZ_unifyString(head,"O_WRONLY"  ) == PROCEED) {
      flags |= O_WRONLY;
    } else if (OZ_unifyString(head,"O_RDWR"    ) == PROCEED) {
      flags |= O_RDWR;
    } else if (OZ_unifyString(head,"O_APPEND"  ) == PROCEED) {
      flags |= O_APPEND;
    } else if (OZ_unifyString(head,"O_CREAT"   ) == PROCEED) {
      flags |= O_CREAT;
    } else if (OZ_unifyString(head,"O_EXCL"    ) == PROCEED) {
      flags |= O_EXCL;
    } else if (OZ_unifyString(head,"O_TRUNC"   ) == PROCEED) {
      flags |= O_TRUNC;
    } else if (OZ_unifyString(head,"O_NOCTTY"  ) == PROCEED) {
      flags |= O_NOCTTY;
    } else if (OZ_unifyString(head,"O_NONBLOCK") == PROCEED) {
      flags |= O_NONBLOCK;
    } else if (OZ_unifyString(head,"O_SYNC"    ) == PROCEED) {
      flags |= O_SYNC;
    } else {
      OZ_warning("open: illegal flag");
      return FAILED;
    }

    OzFlags = tail;
  }

  if (OZ_isVariable(OzFlags)) {
    return SUSPEND;
  } else if (!OZ_isNil(OzFlags)) {
    return FAILED;
  }

  // Compute modes from their textual representation

  int mode = 0;
  while (unixIsCons(OzMode, &head, &tail)) {

    if (OZ_isVariable(head))
      return SUSPEND;

    if (OZ_unifyString(head,"S_IRUSR") == PROCEED) { mode |= S_IRUSR; }
    else if (OZ_unifyString(head,"S_IWUSR") == PROCEED) { mode |= S_IWUSR; }
    else if (OZ_unifyString(head,"S_IXUSR") == PROCEED) { mode |= S_IXUSR; }
    else if (OZ_unifyString(head,"S_IRGRP") == PROCEED) { mode |= S_IRGRP; }
    else if (OZ_unifyString(head,"S_IWGRP") == PROCEED) { mode |= S_IWGRP; }
    else if (OZ_unifyString(head,"S_IXGRP") == PROCEED) { mode |= S_IXGRP; }
    else if (OZ_unifyString(head,"S_IROTH") == PROCEED) { mode |= S_IROTH; }
    else if (OZ_unifyString(head,"S_IWOTH") == PROCEED) { mode |= S_IWOTH; }
    else if (OZ_unifyString(head,"S_IXOTH") == PROCEED) { mode |= S_IXOTH; }
    else {
      OZ_warning("open: illegal mode");
      return FAILED;
    }

    OzMode = tail;
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
  OZ_declareIntArg("close",0,fd);
  OZ_declareArg(1,out);

  WRAPCALL(close(fd),ret,out);

  return OZ_unifyInt(out,ret);
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_read,5)
{
  OZ_declareIntArg("read",0,fd);
  OZ_declareIntArg("read",1,max);
  OZ_declareArg(2, outHead);
  OZ_declareArg(3, outTail);
  OZ_declareArg(4, outN);


  CHECK_READ(fd,outN);

  char *buf = (char *) malloc(max+1);

  WRAPCALL(read(fd, buf, max), ret, outN);


  OZ_Term head = openbuff2list(ret, buf, outTail);

  free(buf);

  return ((OZ_unify(outHead, head) == PROCEED)&&
          (OZ_unifyInt(outN,ret) == PROCEED)) ? PROCEED : FAILED;
}
OZ_C_proc_end



OZ_C_ioproc_begin(unix_write, 3)
{
  OZ_declareIntArg("write", 0, fd);
  OZ_declareArg(1, vs);
  OZ_declareArg(2, out);

  CHECK_WRITE(fd,out,vs);

  int len;
  OZ_Bool status;
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
    OZ_Term rest_all = OZ_tupleC("#",2);

    OZ_putArg(rest_all, 1, buff2list(len - ret, write_buff + ret));
    OZ_putArg(rest_all, 2, rest);

    RETURN_SUSPEND(out,OZ_CToInt(ret),susp,rest_all);
  } else {
    RETURN_SUSPEND(out,OZ_CToInt(ret),susp,rest);
  }

}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_lSeek,4)
{
  OZ_declareIntArg("lSeek", 0, fd);
  OZ_declareIntArg("lSeek", 1, offset);
  OZ_declareStringArg("lSeek", 2, OzWhence);
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


OZ_C_proc_begin(unix_select,2)
{
  OZ_declareIntArg("select",0,fd);
  OZ_declareArg(1, out);

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd,&fds);

  // call select in non-blocked mode
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;  // wait 0 ms

  WRAPCALL(ozSelect(fd+1, &fds, NULL, NULL, &timeout), sel, out);

  if (sel == 0) {
    if (!OZ_select(fd)) {
      RETURN_ANY_ERROR(out,0,"Oz select failed","internal");
    }
    return OZ_unifyInt(out,0);
  }
  return OZ_unifyInt(out,1);
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_openIO,2) {
  OZ_declareIntArg("openIO",0,fd);
  OZ_declareArg(1,out);
  if (!OZ_openIO(fd)) {
    RETURN_ANY_ERROR(out,0,"Oz open IO failed","internal");
  }
  return OZ_unifyInt(out,0);
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_closeIO,2) {
  OZ_declareIntArg("closeIO",0,fd);
  OZ_declareArg(1,out);
  if (!OZ_closeIO(fd)) {
    RETURN_ANY_ERROR(out,0,"Oz close IO failed","internal");
  }
  return OZ_unifyInt(out,0);
}
OZ_C_proc_end



// -------------------------------------------------
// sockets
// -------------------------------------------------



OZ_C_ioproc_begin(unix_socket,4)
{
  OZ_declareStringArg("socket", 0, OzDomain);
  OZ_declareStringArg("socket", 1, OzType);
  OZ_declareVsArg("socket", 2, OzProtocol);
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
  OZ_declareIntArg("bindInet",0,sock);
  OZ_declareIntArg("bindInet",1,port);
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

OZ_C_ioproc_begin(unix_bindUnix,3)
{
  OZ_declareIntArg("bindUnix",0,s);
  OZ_declareVsArg("bindUnix",1,path);
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


OZ_C_ioproc_begin(unix_getSockName,2)
{
  OZ_declareIntArg("getSockName",0,s);
  OZ_Term out = OZ_getCArg(1);

  struct sockaddr_in addr;
  int length = sizeof(addr);

  WRAPCALL(getsockname(s, (struct sockaddr *) &addr, &length), ret, out);

  return OZ_unifyInt(out,ntohs(addr.sin_port));
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_listen,3)
{
  OZ_declareIntArg("listen", 0, s);
  OZ_declareIntArg("listen", 1, n);
  OZ_declareArg(2, out);

  WRAPCALL(listen(s,n), ret, out);

  return OZ_unifyInt(out,ret);
}
OZ_C_proc_end


static void blockSignals()
{
  sigset_t s,sOld;
  sigfillset(&s);
  sigprocmask(SIG_SETMASK,&s,&sOld);
  sigemptyset(&s);
  memcmp(&s,&sOld,sizeof(sigset_t));
}


static void unblockSignals()
{
  sigset_t s;
  sigemptyset(&s);
  sigprocmask(SIG_SETMASK,&s,NULL);
}


OZ_C_ioproc_begin(unix_connectInet,4)
{
  OZ_declareIntArg("connectInet", 0, s);
  OZ_declareVsArg("connectInet", 1, host);
  OZ_declareIntArg("connectInet", 2, port);
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
  blockSignals();

  int ret;
  while ((ret = connect(s,(struct sockaddr *) &addr,sizeof(addr)))<0) {
    if (errno != EINTR) {
      unblockSignals();
      RETURN_UNIX_ERROR(out);
    }
  }

// end of critical region
  unblockSignals();

  return OZ_unifyInt(out,ret);
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_connectUnix,3)
{
  OZ_declareIntArg("connectUnix",0,s);
  OZ_declareVsArg("connect",1,path);
  OZ_declareArg(2, out);

  struct sockaddr_un addr;

  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, path);

// critical region
  blockSignals();

  int ret;
  while ((ret = connect(s,(struct sockaddr *) &addr,
                        sizeof(struct sockaddr_un)))<0) {
    if (errno != EINTR) {
      unblockSignals();
      RETURN_UNIX_ERROR(out);
    }
  }

// end of critical region
  unblockSignals();

  return OZ_unifyInt(out,ret);
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_acceptInet,4)
{
  OZ_declareIntArg("acceptInet", 0, sock);
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
            && OZ_unify(host, OZ_CToList(gethost->h_name)) == PROCEED
            && OZ_unifyInt(out, fd) == PROCEED) ? PROCEED : FAILED;
  } else {
    return (OZ_unifyInt(port, ntohs(from.sin_port)) == PROCEED
            && OZ_unify(host, OZ_CToList(inet_ntoa(from.sin_addr))) == PROCEED
            && OZ_unifyInt(out, fd) == PROCEED) ? PROCEED : FAILED;
  }
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_acceptUnix,3)
{
  OZ_declareIntArg("acceptUnix", 0, sock);
  OZ_declareArg(1, path);
  OZ_declareArg(2, out);

  struct sockaddr_un from;
  int fromlen = sizeof from;

  WRAPCALL(accept(sock,(struct sockaddr *)&from, &fromlen), fd, out);

  return (OZ_unify(path, OZ_CToList(from.sun_path)) == PROCEED
    && OZ_unifyInt(out, fd) == PROCEED) ? PROCEED: FAILED;
}
OZ_C_proc_end



static OZ_Bool get_send_recv_flags(OZ_Term OzFlags, int * flags)
{
  OZ_Term head, tail;

  *flags = 0;

  while (unixIsCons(OzFlags, &head, &tail)) {

    if (OZ_isVariable(head))
      return SUSPEND;

    if (OZ_unifyString(head,"MSG_OOB") == PROCEED) {
      *flags |= MSG_OOB;
    } else if (OZ_unifyString(head,"MSG_PEEK") == PROCEED) {
      *flags |= MSG_PEEK;
    } else {
      OZ_warning("send or receive: illegal flag");
      return FAILED;
    }

    OzFlags = tail;
  }

  if (OZ_isVariable(OzFlags))
    return SUSPEND;

  if (!(OZ_nil(OzFlags)))
    return FAILED;

  return PROCEED;
}


OZ_C_ioproc_begin(unix_send, 4)
{
  OZ_declareIntArg("send", 0, sock);
  OZ_declareArg(1, vs);
  OZ_declareArg(2, OzFlags);
  OZ_declareArg(3, out);


  int flags;
  OZ_Bool flagBool;

  if (!((flagBool = get_send_recv_flags(OzFlags,&flags)) == PROCEED))
      return flagBool;

  CHECK_WRITE(sock,out,vs);

  int len;
  OZ_Bool status;
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

    rest_all = OZ_tupleC("#", 2);
    OZ_putArg(rest_all, 1, from_buff);
    OZ_putArg(rest_all, 2, rest);

    RETURN_SUSPEND(out,OZ_CToInt(ret),susp,rest_all);
  } else {
    RETURN_SUSPEND(out,OZ_CToInt(ret),susp,rest);
  }

}
OZ_C_proc_end

OZ_C_ioproc_begin(unix_sendToInet, 6)
{
  OZ_declareIntArg("sendToInet", 0, sock);
  OZ_declareArg(1, vs);
  OZ_declareArg(2, OzFlags);
  OZ_declareVsArg("sendToInet", 3, host);
  OZ_declareIntArg("sendToInet", 4, port);
  OZ_declareArg(5, out);

  int flags;
  OZ_Bool flagBool;

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
  OZ_Bool status;
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
    OZ_Term rest_all = OZ_tupleC("#",2);

    OZ_putArg(rest_all, 1, buff2list(len - ret, write_buff + ret));
    OZ_putArg(rest_all, 2, rest);

    RETURN_SUSPEND(out,OZ_CToInt(ret),susp,rest_all);
  } else {
    RETURN_SUSPEND(out,OZ_CToInt(ret),susp,rest);
  }

}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_sendToUnix, 5)
{
  OZ_declareIntArg("sendToUnix", 0, sock);
  OZ_declareArg(1, vs);
  OZ_declareArg(2, OzFlags);
  OZ_declareVsArg("sendToUnix", 3, path);
  OZ_declareArg(4, out);

  int flags;
  OZ_Bool flagBool;

  if (!((flagBool = get_send_recv_flags(OzFlags,&flags)) == PROCEED))
      return flagBool;

  CHECK_WRITE(sock,out,vs);

  struct sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, path);

  int len;
  OZ_Bool status;
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
    OZ_Term rest_all = OZ_tupleC("#",2);

    OZ_putArg(rest_all, 1, buff2list(len - ret, write_buff + ret));
    OZ_putArg(rest_all, 2, rest);

    RETURN_SUSPEND(out,OZ_CToInt(ret),susp,rest_all);
  } else {
    RETURN_SUSPEND(out,OZ_CToInt(ret),susp,rest);
  }

}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_shutDown, 3)
{
  OZ_declareIntArg("shutDown",0,sock);
  OZ_declareIntArg("shutDown",1,how);
  OZ_declareArg(2, out);

  WRAPCALL(shutdown(sock, how), ret, out);

  return OZ_unifyInt(out, ret);
}
OZ_C_proc_end



OZ_C_ioproc_begin(unix_receiveFromInet,8)
{
  OZ_declareIntArg("receiveFromInet",0,sock);
  OZ_declareIntArg("receiveFromInet",1,max);
  OZ_declareArg(2, OzFlags);
  OZ_declareArg(3, head);
  OZ_declareArg(4, tail);
  OZ_declareArg(5, host);
  OZ_declareArg(6, port);
  OZ_declareArg(7, outN);

  int flags;
  OZ_Bool flagBool;

  if (!((flagBool = get_send_recv_flags(OzFlags,&flags)) == PROCEED))
      return flagBool;

  CHECK_READ(sock,outN);

  char *buf = (char *) malloc(max+1);

  struct sockaddr_in from;
  int fromlen = sizeof from;

  WRAPCALL(recvfrom(sock, buf, max, flags,
                    (struct sockaddr*)&from, &fromlen),ret,outN);

  struct hostent *gethost = gethostbyaddr((char *) &from.sin_addr,
                                            fromlen, AF_INET);

  OZ_Term localhead = openbuff2list(ret, buf, tail);

  free(buf);

  return (OZ_unify(localhead, head) == PROCEED
          && OZ_unifyInt(port, ntohs(from.sin_port)) == PROCEED
          && OZ_unify(host, OZ_CToList(gethost ?
                                        gethost->h_name :
                                        inet_ntoa(from.sin_addr))) == PROCEED
          && OZ_unifyInt(outN, ret) == PROCEED) ? PROCEED : FAILED;

}
OZ_C_proc_end

OZ_C_ioproc_begin(unix_receiveFromUnix,7)
{
  OZ_declareIntArg("receiveFromUnix",0,sock);
  OZ_declareIntArg("receiveFromUnix",1,max);
  OZ_declareArg(2, OzFlags);
  OZ_declareArg(3, head);
  OZ_declareArg(4, tail);
  OZ_declareArg(5, path);
  OZ_declareArg(6, outN);

  int flags;
  OZ_Bool flagBool;

  if (!((flagBool = get_send_recv_flags(OzFlags,&flags)) == PROCEED))
      return flagBool;

  CHECK_READ(sock,outN);

  char *buf = (char *) malloc(max+1);

  struct sockaddr_un from;
  int fromlen = sizeof from;

  WRAPCALL(recvfrom(sock, buf, max, flags,
                    (struct sockaddr*)&from, &fromlen),ret,outN);

  OZ_Term localhead = openbuff2list(ret, buf, tail);

  free(buf);

  return (OZ_unify(localhead, head) == PROCEED
          && OZ_unify(path, OZ_CToList(from.sun_path)) == PROCEED
          && OZ_unifyInt(outN, ret) == PROCEED) ? PROCEED : FAILED;

}
OZ_C_proc_end




// Misc stuff

OZ_C_ioproc_begin(unix_unlink, 2)
{
  OZ_declareVsArg("unlink",0,path);
  OZ_declareArg(1, out);

  WRAPCALL(unlink(path),ret,out);
  return OZ_unifyInt(out, ret);
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_system,2)
{
  OZ_declareVsArg("system", 0, vs);
  OZ_declareArg(1, out);

  WRAPCALL(system(vs),ret,out);

  return OZ_unifyInt(out,ret);
}
OZ_C_proc_end

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

OZ_C_ioproc_begin(unix_pipe,4)
{
  OZ_declareVsArg("pipe",0,s);
  OZ_declareArg(1, args);
  OZ_declareArg(2, rpid);
  OZ_declareArg(3, rsock);

  OZ_Term head, tail, argl;
  int argno = 0;

  argl=args;

  while (unixIsCons(argl, &head, &tail)) {
    if (OZ_isVariable(head)) return SUSPEND;
    argno++;
    argl = tail;
  }

  if (OZ_isVariable(argl))
    return SUSPEND;

  if (!OZ_isNil(argl))
    return FAILED;

  argl=args;

  char* argv[argno+2];

  argv[0] = s;
  argv[argno+1] = 0;

  argno = 1;

  while (unixIsCons(argl, &head, &tail)) {
    int len;
    OZ_Bool status;
    OZ_Term rest, susp;

    char *vsarg = (char *) malloc(max_vs_length + 256);

    status = buffer_vs(head, vsarg, &len, &rest, &susp);

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

    argl = tail;
  }

  int sv[2];
  WRAPCALL(socketpair(PF_UNIX,SOCK_STREAM,0,sv),ret,rpid);


  int pid =  fork();
  switch (pid) {
  case 0: // child
    {
      int i;
      for (i = 0;
             i < FD_SETSIZE;
             i++)
        {
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
    break;
  case -1:
    RETURN_UNIX_ERROR(rpid);
  default: // parent
    close(sv[1]);

    int i;
    for (i=1 ; i<argno ; i++)
      free(argv[i]);

    return OZ_unifyInt(rpid,pid) == PROCEED
      && OZ_unifyInt(rsock,sv[0]) == PROCEED ? PROCEED : FAILED;
  }
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_getEnv,3)
{
  OZ_declareVsArg("getEnv", 0, envVar);
  OZ_nonvarArg(1);

  char *envValue;

  envValue = getenv(envVar);
  if (envValue == 0)
    return OZ_unify(OZ_getCArg(1),OZ_getCArg(2));

  return OZ_unify(OZ_getCArg(2),OZ_CToList(envValue));
}
OZ_C_proc_end

OZ_C_ioproc_begin(unix_putEnv,2)
{
  OZ_declareVsArg("putEnv", 0, envVar);
  OZ_declareVsArg("putEnv", 1, envValue);

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


OZ_C_ioproc_begin(unix_tempName, 3)
{
  OZ_declareVsArg("tempName", 0, directory);
  OZ_declareVsArg("tempName", 1, prefix);
  OZ_declareArg(2, name);

  char *filename;

  if (strlen(prefix) > 5)
    return FAILED;

  if (!(filename = tempnam(directory, prefix))) {
    OZ_warning("tempName: no file name accessible");
    return FAILED;
  }
  filename = ozstrdup(filename);

  return OZ_unify(name, OZ_CToList(filename));
}
OZ_C_proc_end


OZ_C_ioproc_begin(unix_getServByName, 4)
{
  OZ_declareVsArg("getServByName", 0, name);
  OZ_declareVsArg("getServByName", 1, proto);
  OZ_nonvarArg(2);
  OZ_Term out = OZ_getCArg(3);

  struct servent *serv;
  serv = getservbyname(name, proto);

  if (!serv)
    return OZ_unify(OZ_getCArg(2), out);

  return OZ_unifyInt(out, ntohs(serv->s_port));
}
OZ_C_proc_end



static OZ_Bool copy_time(const struct tm* time, OZ_Term *term)
{
  return (
          OZ_unifyInt(OZ_getRecordArgC(*term, "sec"), time->tm_sec) == PROCEED
          && OZ_unifyInt(OZ_getRecordArgC(*term, "min"), time->tm_min) == PROCEED
          && OZ_unifyInt(OZ_getRecordArgC(*term, "hour"), time->tm_hour) == PROCEED
          && OZ_unifyInt(OZ_getRecordArgC(*term, "mDay"), time->tm_mday) == PROCEED
          && OZ_unifyInt(OZ_getRecordArgC(*term, "mon"), time->tm_mon) == PROCEED
          && OZ_unifyInt(OZ_getRecordArgC(*term, "year"), time->tm_year) == PROCEED
          && OZ_unifyInt(OZ_getRecordArgC(*term, "wDay"), time->tm_wday) == PROCEED
          && OZ_unifyInt(OZ_getRecordArgC(*term, "yDay"), time->tm_yday) == PROCEED
          && OZ_unifyInt(OZ_getRecordArgC(*term, "isDst"), time->tm_isdst) == PROCEED
          ) ? PROCEED : FAILED;
}

OZ_C_ioproc_begin(unix_gmTime, 1)
{
  OZ_Term out = OZ_getCArg(0);
  time_t timebuf;

  time(&timebuf);
  return copy_time(gmtime(&timebuf), &out);

}
OZ_C_proc_end

OZ_C_ioproc_begin(unix_localTime, 1)
{
  OZ_Term out = OZ_getCArg(0);
  time_t timebuf;

  time(&timebuf);
  return copy_time(localtime(&timebuf), &out);

}
OZ_C_proc_end

static OZ_Term mkAliasList(char **alias)
{
  if (*alias == 0) {
    return OZ_nil();
  } else {
    return OZ_cons(OZ_CToList(*alias), mkAliasList(++alias));
  }
}

static OZ_Term mkAddressList(char **lstptr)
{
  if (*lstptr == 0) {
    return OZ_nil();
  } else {

#ifdef SUNOS_SPARC
    return OZ_cons(OZ_CToList(inet_ntoa(*(struct in_addr *)lstptr)),
                   mkAddressList(++lstptr));
#else
    return OZ_cons(OZ_CToList(inet_ntoa(*((struct in_addr *) *lstptr))),
                   mkAddressList(++lstptr));
#endif
  }
}

OZ_C_ioproc_begin(unix_getHostByName, 4)
{
  OZ_declareVsArg("getHostByName", 0, name);
  OZ_declareArg(1, offName);
  OZ_declareArg(2, aliases);
  OZ_declareArg(3, addresses);

  struct hostent *hostaddr;

  if ((hostaddr = gethostbyname(name)) == NULL) {
    RETURN_NET_ERROR(offName);
  }

  return (OZ_unify(offName,OZ_CToList(hostaddr->h_name)) == PROCEED &&
          OZ_unify(aliases,mkAliasList(hostaddr->h_aliases)) == PROCEED &&
          OZ_unify(addresses,mkAddressList(hostaddr->h_addr_list)) ==
          PROCEED)
    ? PROCEED : FAILED;
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
  OZ_declareIntArg("srand",0, seed);

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
... fill in RAND_MAX ...
#endif
#endif

OZ_C_proc_begin(unix_randLimits, 2)
{
  OZ_Term min = OZ_getCArg(0);
  OZ_Term max = OZ_getCArg(1);

  if (OZ_unifyInt(min,0) == FAILED) {
    return FAILED;
  }
  return OZ_unifyInt(max,RAND_MAX);
}
OZ_C_proc_end

void MyinitUnix()
{
  OZ_addBuiltin("unix_open",4,unix_open);
  OZ_addBuiltin("unix_fileDesc",2,unix_fileDesc);
  OZ_addBuiltin("unix_close",2,unix_close);
  OZ_addBuiltin("unix_write",3,unix_write);
  OZ_addBuiltin("unix_read",5,unix_read);
  OZ_addBuiltin("unix_lSeek",4,unix_lSeek);
  OZ_addBuiltin("unix_unlink",2,unix_unlink);
  OZ_addBuiltin("unix_socket",4,unix_socket);
  OZ_addBuiltin("unix_bindInet",3,unix_bindInet);
  OZ_addBuiltin("unix_bindUnix",3,unix_bindUnix);
  OZ_addBuiltin("unix_listen",3,unix_listen);
  OZ_addBuiltin("unix_connectInet",4,unix_connectInet);
  OZ_addBuiltin("unix_connectUnix",3,unix_connectUnix);
  OZ_addBuiltin("unix_acceptInet",4,unix_acceptInet);
  OZ_addBuiltin("unix_acceptUnix",3,unix_acceptUnix);
  OZ_addBuiltin("unix_shutDown",3,unix_shutDown);
  OZ_addBuiltin("unix_send",4,unix_send);
  OZ_addBuiltin("unix_sendToInet",6,unix_sendToInet);
  OZ_addBuiltin("unix_sendToUnix",5,unix_sendToUnix);
  OZ_addBuiltin("unix_receiveFromUnix",7,unix_receiveFromUnix);
  OZ_addBuiltin("unix_receiveFromInet",8,unix_receiveFromInet);
  OZ_addBuiltin("unix_getSockName",2,unix_getSockName);
  OZ_addBuiltin("unix_getServByName",4,unix_getServByName);
  OZ_addBuiltin("unix_select",2,unix_select);
  OZ_addBuiltin("unix_openIO",2,unix_openIO);
  OZ_addBuiltin("unix_closeIO",2,unix_closeIO);
  OZ_addBuiltin("unix_system",2,unix_system);
  OZ_addBuiltin("unix_wait",2,unix_wait);
  OZ_addBuiltin("unix_pipe",4,unix_pipe);
  OZ_addBuiltin("unix_getEnv",3,unix_getEnv);
  OZ_addBuiltin("unix_putEnv",2,unix_putEnv);
  OZ_addBuiltin("unix_tempName",3,unix_tempName);
  OZ_addBuiltin("unix_gmTime",1,unix_gmTime);
  OZ_addBuiltin("unix_localTime",1,unix_localTime);
  OZ_addBuiltin("unix_getHostByName",4,unix_getHostByName);
  OZ_addBuiltin("unix_srand",1,unix_srand);
  OZ_addBuiltin("unix_rand",1,unix_rand);
  OZ_addBuiltin("unix_randLimits",2,unix_randLimits);
}
