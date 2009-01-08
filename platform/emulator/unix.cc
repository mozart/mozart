/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Contributors:
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 *
 *  Copyright:
 *    Michael Mehl, 1997
 *    Christian Schulte, 1997
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "wsock.hh"

#include "builtins.hh"
#include "am.hh"
#include "os.hh"
#include "var_base.hh"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#ifdef _MSC_VER
#include <direct.h>
#else
#include <dirent.h>
#endif

#ifndef WINDOWS
#include <netdb.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <netinet/tcp.h>
#endif


#if defined(LINUX) || defined(HPUX_700)
extern int h_errno;
#endif

#include <time.h>
#include <sys/stat.h>

// The following is an approximation under Windows.
#ifndef S_IRGRP
#define S_IRGRP _S_IREAD
#endif
#ifndef S_IWGRP
#define S_IWGRP _S_IWRITE
#endif
#ifndef S_IXGRP
#define S_IXGRP _S_IEXEC
#endif
#ifndef S_IROTH
#define S_IROTH _S_IREAD
#endif
#ifndef S_IWOTH
#define S_IWOTH _S_IWRITE
#endif
#ifndef S_IXOTH
#define S_IXOTH _S_IEXEC
#endif

#if !defined(OS2_I486) && !defined(WINDOWS)
#include <sys/param.h>
#include <sys/socket.h>
#if !defined(LINUX) && !defined(IRIX6)
#include <sys/uio.h>
#endif
#include <sys/un.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <netinet/in.h>
#endif

#include <signal.h>

#if !defined(WINDOWS) || defined(__CYGWIN32__)
#include <sys/wait.h>
#include <sys/utsname.h>
#endif


#ifdef IRIX
#include <bstring.h>
#endif

#ifndef WINDOWS
extern "C" char *inet_ntoa(struct in_addr in);
#endif

#define max_vs_length 4096*4
#define vs_buff(VAR) char VAR[max_vs_length + 256];

//
// Argument handling
//

#define OZ_BI_iodefine(Name,InArity,OutArity)                   \
OZ_BI_define(Name,InArity,OutArity)                             \
  if (!oz_onToplevel()) {                                       \
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,AtomIO);   \
  }
#define OZ_BI_ioend OZ_BI_end

#define OZ_declareVsIN(ARG,VAR)                                         \
 vs_buff(VAR); OZ_expectDet(ARG);                                       \
 { int len; OZ_Return status; OZ_Term rest, susp;                       \
   status = buffer_vs(OZ_in(ARG), VAR, &len, &rest, &susp);             \
   if (status == SUSPEND) {                                             \
     if (OZ_isVariable(susp)) {                                         \
        OZ_suspendOn(susp);                                             \
     } else {                                                           \
        return oz_raise(E_SYSTEM,E_SYSTEM,"limitInternal",1,            \
                         OZ_string("virtual string too long"));         \
     }                                                                  \
   } else if (status != PROCEED) {                                      \
     return status;                                                     \
   }                                                                    \
   *(VAR+len) = '\0';                                                   \
 }

#define DeclareAtomListIN(ARG,VAR)                                      \
OZ_Term VAR = OZ_in(ARG);                                               \
{ OZ_Term arg = VAR;                                                    \
  while (OZ_isCons(arg)) {                                              \
    TaggedRef a = OZ_head(arg);                                         \
    if (OZ_isVariable(a)) OZ_suspendOn(a);                              \
    if (!OZ_isAtom(a))    return OZ_typeError(ARG,"list(Atom)");        \
    arg = OZ_tail(arg);                                                 \
  }                                                                     \
  if (OZ_isVariable(arg)) OZ_suspendOn(arg);                            \
  if (!OZ_isNil(arg))     return OZ_typeError(ARG,"list(Atom)");        \
}

#define DeclareNonvarIN(ARG,VAR) \
  OZ_declareDetTerm(ARG,VAR)

#define IsPair(s) (s[0]=='#' && s[1]=='\0')



// checking

inline
int unixIsCons(OZ_Term list, OZ_Term *hd, OZ_Term *tl) {
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


#define WRAPCALL(f, CALL, RET)                          \
int RET;                                                \
while ((RET = CALL) < 0) {                              \
  if (ossockerrno() != EINTR) { RETURN_UNIX_ERROR(f); } \
}


// -------------------------------------------------
// specification of returning
// -------------------------------------------------

// Use this rather han OZ_unixError to get a platform independent errno
// interpretation.
// The string returned must be used immediately.
// The definition relies on the following errors being defined for all
// platforms directly or as done in wsock.hh
const char *errnoToString(int aErrno) {
  switch(aErrno) {
  case ECONNREFUSED:
    return "Connection refused";
  case EPIPE:
    return "Broken pipe";
  case EINPROGRESS:
    return "In progress";
  case EINTR:
    return "Interrupted";
  case EHOSTUNREACH:
    return "Host unreacheable";
  case EAGAIN:
    return "Try again";
  case ETIMEDOUT:
    return "Timed out";
  case ECONNRESET:
    return "Connection reset";
  case EBADF:
    return "Bad filedescriptor";

  default:
    return OZ_unixError(aErrno);
  }
}

int raiseUnixError(const char *f,int n, const char * e, const char * g) {
  return oz_raise(E_SYSTEM,E_OS, g, 3, OZ_string(f), OZ_int(n), OZ_string(e));
}

// return upon unix-error
#define RETURN_UNIX_ERROR(f) \
{ return raiseUnixError(f,ossockerrno(), errnoToString(ossockerrno()), "os"); }


#if defined(ULTRIX_MIPS) || defined(OS2_I486)

#define RETURN_NET_ERROR(f) \
{ return raiseUnixError(f, 0, "Host lookup failure.", "host"); }

#else

static const char* h_strerror(const int err) {
  switch (err) {
  case HOST_NOT_FOUND:
    return "No such host is known.";
  case TRY_AGAIN:
    return "Retry later again.";
  case NO_RECOVERY:
    return "Unexpected non-recoverable server failure.";
#if defined(SOLARIS) || defined(LINUX)
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

#define RETURN_NET_ERROR(f) \
{ return raiseUnixError(f, h_errno, h_strerror(h_errno), "host"); }


#endif


#define NEW_RETURN_SUSPEND(LEN,VAR,REST)       \
{ OZ_Term susp_tuple = OZ_tupleC("suspend",3); \
  OZ_putArg(susp_tuple,0,LEN);                 \
  OZ_putArg(susp_tuple,1,VAR);                 \
  OZ_putArg(susp_tuple,2,REST);                \
  OZ_RETURN(susp_tuple);                       \
}



// -------------------------------------------------
// check file descriptors
// -------------------------------------------------

#define CHECK_READ(FD)                                  \
{ int sel = osTestSelect(FD,SEL_READ);                  \
  if (sel < 0)  { RETURN_UNIX_ERROR("select"); }        \
  if (sel == 0) {                                       \
    TaggedRef t = oz_newVariable();                     \
    (void) OZ_readSelect(FD, NameUnit, t);              \
    DEREF(t, t_ptr);                                    \
    Assert(!oz_isRef(t));                               \
    if (oz_isVarOrRef(t)) {                             \
      return oz_addSuspendVarList(t_ptr);               \
    }                                                   \
  }                                                     \
}

#define CHECK_WRITE(FD)                                 \
{ int sel = osTestSelect(FD,SEL_WRITE);                 \
  if (sel < 0)  { RETURN_UNIX_ERROR("select"); }        \
  if (sel == 0) {                                       \
    TaggedRef t = oz_newVariable();                     \
    (void) OZ_writeSelect(FD, NameUnit, t);             \
    DEREF(t, t_ptr);                                    \
    Assert(!oz_isRef(t));                               \
    if (oz_isVarOrRef(t)) {                             \
      return oz_addSuspendVarList(t_ptr);               \
    }                                                   \
  }                                                     \
}

//
// Handling of virtual strings
//

inline
OZ_Term buff2list(const int len, const char *s)  {
  return oz_string(s, len, AtomNil);
}



OZ_Return atom2buff(OZ_Term atom, char **write_buff, int *len,
                    OZ_Term *rest, OZ_Term *susp)
{
  char c;

  if (!OZ_isAtom(atom)) {
    return OZ_typeError(-1,"VirtualString");
  }

  const char *string = OZ_atomToC(atom);

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
    *susp = OZ_string((OZ_CONST char*)string);
    *rest = *susp;
    return SUSPEND;
  }

  return PROCEED;
}

OZ_Return bytestring2buff(OZ_Term bs, char **write_buff, int *len,
                          OZ_Term *rest, OZ_Term *susp)
{
  if (!OZ_isByteString(bs))
    return OZ_typeError(-1,"ByteString");

  int n;
  char* s = OZ_vsToC(bs,&n);

  while (n>0 && *len<max_vs_length) {
    **write_buff = *s;
    (*write_buff)++;
    (*len)++; n--; s++;
  }

  if (*len==max_vs_length && n>0) {
    *susp = OZ_mkByteString(s,n);
    *rest = *susp;
    return SUSPEND;
  }

  return PROCEED;
}

OZ_Return int2buff(OZ_Term ozint, char **write_buff, int *len,
                   OZ_Term *rest, OZ_Term *susp)
{
  char *string = OZ_toC(ozint,0,0);
  if (*string == '~') *string='-';
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
  char *string = OZ_toC(ozfloat,0,0);
  for (char *p=string; *p; p++) {
    if (*p == '~') *p='-';
  }
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
    return OZ_typeError(-1,"VirtualString");
  }

  if (OZ_isVariable(list)) {
    *susp = list;
    *rest = list;
    return SUSPEND;
  }

  if (OZ_isNil(list))
    return PROCEED;


  return OZ_typeError(-1,"VirtualString");
}


static OZ_Return vs2buff(OZ_Term vs, char **write_buff, int *len,
                       OZ_Term *rest, OZ_Term *susp)
{
  if (OZ_isAtom(vs)) {
    return OZ_isNil(vs) ? PROCEED : atom2buff(vs, write_buff, len, rest, susp);
  }

  if (OZ_isByteString(vs))
    return bytestring2buff(vs,write_buff,len,rest,susp);

  const char *label = NULL;
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
        } else if (status != PROCEED) {
          return status;
        }

      }

      return PROCEED;

    } else if (label[0] == '|' && label[1] == '\0' && width == 2) {
      return list2buff(vs, write_buff, len, rest, susp);
    }
    return OZ_typeError(-1,"VirtualString");
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

  return OZ_typeError(-1,"VirtualString");
}


inline OZ_Return buffer_vs(OZ_Term vs, char *write_buff, int *len,
                         OZ_Term *rest, OZ_Term *susp)
{
  *len = 0;
  return vs2buff(vs, &write_buff, len, rest, susp);
}

OZ_Return OZ_buffer_vs(OZ_Term vs, char *write_buff, int *len,
                       OZ_Term *rest, OZ_Term *susp)
{ return buffer_vs(vs,write_buff,len,rest,susp); }

// -------------------------------------------------
// unix IO
// -------------------------------------------------

OZ_BI_iodefine(unix_fileDesc,1,1)
{
  OZ_declareAtom( 0, OzFileDesc);

  int desc;
  if (!strcmp(OzFileDesc,"STDIN_FILENO")) {
    desc=osdup(STDIN_FILENO);
  } else if (!strcmp(OzFileDesc,"STDOUT_FILENO")) {
    desc=osdup(STDOUT_FILENO);
  } else if (!strcmp(OzFileDesc,"STDERR_FILENO")) {
    desc=osdup(STDERR_FILENO);
  } else {
    return OZ_typeError(0,"enum(STDIN_FILENO STDOUT_FILENO STDERR_FILENO)");
  }

  OZ_RETURN_INT(desc);
} OZ_BI_ioend


static
OZ_Term readEntries(DIR *dp) {
  OZ_Term dirs = oz_nil();
  do {
  retry:
    // clear errno.  It looks like readdir doesn't
    // clear it when it successfully reaches the end.
    // this means that if an interrupt occurs while
    // attempting to read one entry, errno remains
    // set to EINTR and when reaching the end, readdir
    // just returns NULL and we loop for ever because
    // errno is still set to EINTR.
    errno=0;
    struct dirent * dirp = readdir(dp);
    if (dirp == NULL) {
      if (errno==EINTR) goto retry;
      return dirs;
    }
    dirs = oz_cons(OZ_string(dirp->d_name),dirs);
  } while(OK);
}

OZ_BI_iodefine(unix_getDir,1,1)
{
  DIR *dp;
  OZ_Term dirValue;
  OZ_declareVsIN(0, path);

 retry:
  if ((dp = opendir(path)) == NULL) {
    if (errno==EINTR) goto retry;
    RETURN_UNIX_ERROR("opendir");
  }

  dirValue = readEntries(dp);

  WRAPCALL("closedir", closedir(dp), __ret);

  OZ_RETURN(dirValue);
} OZ_BI_ioend


OZ_BI_iodefine(unix_stat,1,1)
{
  struct stat buf;
  const char *fileType;
  OZ_declareVsIN(0, filename);

 retry:
  if (stat(filename, &buf) < 0) {
    // EINTR may happen on some systems (SVR4)
    if (errno==EINTR) goto retry;
    RETURN_UNIX_ERROR("stat");
  }

  if      (S_ISREG(buf.st_mode))  fileType = "reg";
  else if (S_ISDIR(buf.st_mode))  fileType = "dir";
  else if (S_ISCHR(buf.st_mode))  fileType = "chr";
#ifndef WINDOWS
  else if (S_ISBLK(buf.st_mode))  fileType = "blk";
#endif
  else if (S_ISFIFO(buf.st_mode)) fileType = "fifo";
  else
    fileType = "unknown";

  OZ_MAKE_RECORD_S("stat",5,
                   {"type" OZ_COMMA "size" OZ_COMMA "mtime"
                      OZ_COMMA "ino" OZ_COMMA "dev"},
                   {oz_atom(fileType) OZ_COMMA
                      oz_int(buf.st_size) OZ_COMMA
                      oz_int(buf.st_mtime) OZ_COMMA
                      oz_int(buf.st_ino) OZ_COMMA
                      oz_int(buf.st_dev)}, r);

  OZ_RETURN(r);
} OZ_BI_ioend


OZ_BI_iodefine(unix_uName,0,1) {
#ifdef WINDOWS

  SYSTEM_INFO si;
  GetSystemInfo(&si);

  char machine[64];
  switch (si.wProcessorArchitecture) {
  case PROCESSOR_ARCHITECTURE_INTEL:
    sprintf(machine,"i%d86",si.wProcessorLevel);
    break;
  case PROCESSOR_ARCHITECTURE_MIPS:
    sprintf(machine,"mips");
    break;
  case PROCESSOR_ARCHITECTURE_ALPHA:
    sprintf(machine,"alpha");
    break;
  case PROCESSOR_ARCHITECTURE_PPC:
    sprintf(machine,"ppc");
    break;
#ifdef PROCESSOR_ARCHITECTURE_IA64
  case PROCESSOR_ARCHITECTURE_IA64:
    sprintf(machine,"ia64");
    break;
#endif
#ifdef PROCESSOR_ARCHITECTURE_AMD64
  case PROCESSOR_ARCHITECTURE_AMD64:
    sprintf(machine,"amd64");
    break;
#endif
  case PROCESSOR_ARCHITECTURE_UNKNOWN:
  default:
    sprintf(machine,"unknown");
    break;
  }

  OSVERSIONINFO vi;
  vi.dwOSVersionInfoSize = sizeof(vi);
  BOOL b = GetVersionEx(&vi);
  Assert(b==TRUE);

  char *sysname;
  switch (vi.dwPlatformId) {
  case VER_PLATFORM_WIN32s:
    sysname = "win32s";
    break;
  case VER_PLATFORM_WIN32_WINDOWS:
    sysname = "win32_windows";
    break;
  case VER_PLATFORM_WIN32_NT:
    sysname = "win32_nt";
    break;
  default:
    sysname = "win32_unknown";
    break;
  }

  char release[64];
  sprintf(release,"%ld.%ld",vi.dwMajorVersion,vi.dwMinorVersion);

  char version[196];
  if (vi.szCSDVersion[0] != '\0')
    sprintf(version,"Build %ld %s",vi.dwBuildNumber,vi.szCSDVersion);
  else
    sprintf(version,"Build %ld",vi.dwBuildNumber);

  OZ_MAKE_RECORD_S("utsname",5,
                   { "machine"   OZ_COMMA
                     "nodename"  OZ_COMMA
                     "release"   OZ_COMMA
                     "sysname"   OZ_COMMA
                     "version" },
                   { OZ_string(machine)            OZ_COMMA
                     OZ_string(oslocalhostname())  OZ_COMMA
                     OZ_string(release)            OZ_COMMA
                     OZ_string(sysname)            OZ_COMMA
                     OZ_string(version) }, r);

#else

  struct utsname buf;

  if (uname(&buf) < 0)
    RETURN_UNIX_ERROR("uname");

#if (defined(SUNOS_SPARC) || defined(LINUX)) && !defined(__FCC_VERSION)

  char dname[65];
  if (getdomainname(dname, 65)) {
    RETURN_UNIX_ERROR("getdomainname");
  }

  OZ_MAKE_RECORD_S("utsname", 6,
                   { "machine"    OZ_COMMA
                       "nodename" OZ_COMMA
                       "release"  OZ_COMMA
                       "sysname"  OZ_COMMA
                       "version"  OZ_COMMA
                       "domainname"},
                   { OZ_string(buf.machine)    OZ_COMMA
                       OZ_string(buf.nodename) OZ_COMMA
                       OZ_string(buf.release)  OZ_COMMA
                       OZ_string(buf.sysname)  OZ_COMMA
                       OZ_string(buf.version)  OZ_COMMA
                       OZ_string(dname)}, r);

#else

  OZ_MAKE_RECORD_S("utsname", 5,
                   { "machine"    OZ_COMMA
                       "nodename" OZ_COMMA
                       "release"  OZ_COMMA
                       "sysname"  OZ_COMMA
                       "version"},
                   { OZ_string(buf.machine)    OZ_COMMA
                       OZ_string(buf.nodename) OZ_COMMA
                       OZ_string(buf.release)  OZ_COMMA
                       OZ_string(buf.sysname)  OZ_COMMA
                       OZ_string(buf.version)}, r);

#endif

#endif

  OZ_RETURN(r);
} OZ_BI_ioend


OZ_BI_iodefine(unix_chDir,2,0)
{
  OZ_declareVsIN(0,dir);

  if (chdir(dir)) {
    RETURN_UNIX_ERROR("chdir");
  } else
    return PROCEED;
} OZ_BI_ioend

OZ_BI_define(unix_mkDir,2,0)
{
  OZ_declareVsIN(0, path);
  DeclareAtomListIN(1, OzMode);
  int mode = 0;
  OZ_Term hd, tl;

  while (unixIsCons(OzMode, &hd, &tl)) {
    if (OZ_isVariable(hd))
      return (SUSPEND);
#ifdef OS2_I486
    return OZ_typeError(2,"enum openMode");
#else
    if (OZ_eqAtom(hd,"S_IRUSR") == PROCEED) { mode |= S_IRUSR; }
    else if (OZ_eqAtom(hd,"S_IWUSR") == PROCEED) { mode |= S_IWUSR; }
    else if (OZ_eqAtom(hd,"S_IXUSR") == PROCEED) { mode |= S_IXUSR; }
    else if (OZ_eqAtom(hd,"S_IRGRP") == PROCEED) { mode |= S_IRGRP; }
    else if (OZ_eqAtom(hd,"S_IWGRP") == PROCEED) { mode |= S_IWGRP; }
    else if (OZ_eqAtom(hd,"S_IXGRP") == PROCEED) { mode |= S_IXGRP; }
    else if (OZ_eqAtom(hd,"S_IROTH") == PROCEED) { mode |= S_IROTH; }
    else if (OZ_eqAtom(hd,"S_IWOTH") == PROCEED) { mode |= S_IWOTH; }
    else if (OZ_eqAtom(hd,"S_IXOTH") == PROCEED) { mode |= S_IXOTH; }
    else
      return OZ_typeError(2,"enum openMode");
#endif
    OzMode = tl;
  }

  if (OZ_isVariable(OzMode)) {
    return SUSPEND;
  } else if (!OZ_isNil(OzMode)) {
    return OZ_typeError(2,"enum openMode");
  }

  if (
#if defined(WINDOWS)
      mkdir(path)
#else
      mkdir(path, mode)
#endif
      ) {
    RETURN_UNIX_ERROR("mkdir");
  } else {
    return (PROCEED);
  }
} OZ_BI_ioend

OZ_BI_iodefine(unix_getCWD,0,1)
{
  const int SIZE=256;
  char buf[SIZE];
 again:
  if (getcwd(buf,SIZE)) OZ_RETURN_ATOM(buf);
  if (errno == EINTR) goto again;
  if (errno != ERANGE) RETURN_UNIX_ERROR("getcwd");

  int size=SIZE+SIZE;
  char *bigBuf;
  while (OK) {
    bigBuf=(char *) malloc(size);
  retry:
    if (getcwd(bigBuf,size)) {
      OZ_Term res = oz_atom(bigBuf); // bug fix, was still using buf
      free(bigBuf);
      OZ_RETURN(res);
    }
    if (errno == EINTR) goto retry;
    if (errno != ERANGE) RETURN_UNIX_ERROR("getcwd");
    free(bigBuf);
    size+=SIZE;
  }
} OZ_BI_ioend

#ifndef O_SYNC
#define O_SYNC     0
#endif

#ifndef WINDOWS
#define O_BINARY 0
#define O_TEXT   0
#endif

OZ_BI_iodefine(unix_open,3,1)
{
  OZ_declareVsIN(0, filename);
  DeclareAtomListIN(1, OzFlags);
  DeclareAtomListIN(2, OzMode);

  // Compute flags from their textual representation

  int flags = 0;
  OZ_Term hd, tl;

  while (unixIsCons(OzFlags, &hd, &tl)) {

    if (OZ_isVariable(hd)) return SUSPEND;

    if (OZ_eqAtom(hd,"O_RDONLY") == PROCEED) {
      flags |= O_RDONLY;
    } else if (OZ_eqAtom(hd,"O_WRONLY"  ) == PROCEED) {
      flags |= O_WRONLY;
    } else if (OZ_eqAtom(hd,"O_RDWR"    ) == PROCEED) {
      flags |= O_RDWR;
    } else if (OZ_eqAtom(hd,"O_APPEND"  ) == PROCEED) {
      flags |= O_APPEND;
    } else if (OZ_eqAtom(hd,"O_CREAT"   ) == PROCEED) {
      flags |= O_CREAT;
    } else if (OZ_eqAtom(hd,"O_EXCL"    ) == PROCEED) {
      flags |= O_EXCL;
    } else if (OZ_eqAtom(hd,"O_TRUNC"   ) == PROCEED) {
      flags |= O_TRUNC;
    } else if (OZ_eqAtom(hd,"O_NOCTTY"  ) == PROCEED) {
      flags |= O_NOCTTY;
    } else if (OZ_eqAtom(hd,"O_NONBLOCK") == PROCEED) {
      flags |= O_NONBLOCK;
    } else if (OZ_eqAtom(hd,"O_SYNC"    ) == PROCEED) {
      flags |= O_SYNC;
    } else if (OZ_eqAtom(hd,"O_BINARY"  ) == PROCEED) {
      flags |= O_BINARY;
    } else if (OZ_eqAtom(hd,"O_TEXT"    ) == PROCEED) {
      flags |= O_TEXT;
    } else {
      return OZ_typeError(1,"enum openFlags");
    }

    OzFlags = tl;
  }

  if (OZ_isVariable(OzFlags)) {
    return SUSPEND;
  } else if (!OZ_isNil(OzFlags)) {
    return OZ_typeError(1,"enum openFlags");
  }

  // Compute modes from their textual representation

  int mode = 0;
  while (unixIsCons(OzMode, &hd, &tl)) {

    if (OZ_isVariable(hd))
      return SUSPEND;
#ifdef OS2_I486
    return OZ_typeError(2,"enum openMode");
#else
    if (OZ_eqAtom(hd,"S_IRUSR") == PROCEED) { mode |= S_IRUSR; }
    else if (OZ_eqAtom(hd,"S_IWUSR") == PROCEED) { mode |= S_IWUSR; }
    else if (OZ_eqAtom(hd,"S_IXUSR") == PROCEED) { mode |= S_IXUSR; }
    else if (OZ_eqAtom(hd,"S_IRGRP") == PROCEED) { mode |= S_IRGRP; }
    else if (OZ_eqAtom(hd,"S_IWGRP") == PROCEED) { mode |= S_IWGRP; }
    else if (OZ_eqAtom(hd,"S_IXGRP") == PROCEED) { mode |= S_IXGRP; }
    else if (OZ_eqAtom(hd,"S_IROTH") == PROCEED) { mode |= S_IROTH; }
    else if (OZ_eqAtom(hd,"S_IWOTH") == PROCEED) { mode |= S_IWOTH; }
    else if (OZ_eqAtom(hd,"S_IXOTH") == PROCEED) { mode |= S_IXOTH; }
    else
      return OZ_typeError(2,"enum openMode");
#endif
    OzMode = tl;
  }

  if (OZ_isVariable(OzMode)) {
    return SUSPEND;
  } else if (!OZ_isNil(OzMode)) {
    return OZ_typeError(2,"enum openMode");
  }

  WRAPCALL("open",osopen(filename, flags, mode),desc);

  OZ_RETURN_INT(desc);
} OZ_BI_ioend



OZ_BI_iodefine(unix_close,1,0)
{
  OZ_declareInt(0,fd);

  WRAPCALL("close",osclose(fd),ret);

  return PROCEED;
} OZ_BI_ioend


OZ_BI_iodefine(unix_read,5,0)
{
  OZ_declareInt(0,fd);
  OZ_declareInt(1,maxx);
  OZ_declareTerm(2, outHead);
  OZ_declareTerm(3, outTail);
  OZ_declareTerm(4, outN);

  if (!(!OZ_isVariable(outHead) || oz_isFree(oz_deref(outHead))))
    return (OZ_typeError(2, "value or a free variable"));
  if (!(!OZ_isVariable(outN) || oz_isFree(oz_deref(outN))))
    return (OZ_typeError(4, "value or a free variable"));

  CHECK_READ(fd);

  char *buf = (char *) malloc(maxx+1);

  WRAPCALL("read",osread(fd, buf, maxx), ret);

  OZ_Term hd = oz_string(buf, ret, outTail);

  free(buf);

  //
  OZ_Return ures;
  ures = oz_unify(outHead, hd);
  // kost@  : since 'outHead' is a free variable;
  Assert(ures == PROCEED || ures == FAILED);
  if (ures == FAILED) return (FAILED);
  ures = oz_unify(outN, oz_int(ret));
  Assert(ures == PROCEED || ures == FAILED);
  return (ures);
} OZ_BI_ioend



OZ_BI_iodefine(unix_write, 2,1)
{
  OZ_declareInt(0, fd);
  DeclareNonvarIN(1, vs);

  CHECK_WRITE(fd);

  {
    int len;
    OZ_Return status;
    OZ_Term rest, susp;
    vs_buff(write_buff);

    status = buffer_vs(vs, write_buff, &len, &rest, &susp);

    if (status != PROCEED && status != SUSPEND)
      return status;

    WRAPCALL("write",oswrite(fd, write_buff, len), ret);

    if (status == PROCEED) {
      if (len == ret) {
        OZ_RETURN_INT(len);
      } else {
        Assert(len > ret);
        NEW_RETURN_SUSPEND(OZ_int(ret), oz_nil(),
                           OZ_mkByteString(write_buff+ret,len-ret));
      }
    } else {

      Assert(status == SUSPEND);
      if (len == ret) {
        NEW_RETURN_SUSPEND(OZ_int(ret), susp, rest);
      } else {
        Assert(len > ret);
        NEW_RETURN_SUSPEND(OZ_int(ret), susp,
                           OZ_pair2(OZ_mkByteString(write_buff+ret,len-ret),
                                    rest));
      }
    }
  }
} OZ_BI_ioend


OZ_BI_iodefine(unix_lSeek,3,1) {
  OZ_declareInt(0, fd);
  OZ_declareInt(1, offset);
  OZ_declareAtom(2, OzWhence);

  int whence;

  if (!strcmp(OzWhence,"SEEK_SET")) {
    whence=SEEK_SET;
  } else if (!strcmp(OzWhence,"SEEK_CUR")) {
    whence=SEEK_CUR;
  } else if (!strcmp(OzWhence,"SEEK_END")) {
    whence=SEEK_END;
  } else {
    return OZ_typeError(2,"enum(SEEK_CUR SEEK_END)");
  }

  WRAPCALL("lseek",lseek(fd, offset, whence),ret);

  OZ_RETURN_INT(ret);
} OZ_BI_ioend


OZ_BI_iodefine(unix_readSelect, 1,0) {
  OZ_declareInt(0,fd);

  WRAPCALL("select",osTestSelect(fd,SEL_READ),sel);

  if (sel == 0) {
    TaggedRef t = oz_newVariable();

    (void) OZ_readSelect(fd, NameUnit, t);
    DEREF(t, t_ptr);

    Assert(!oz_isRef(t));
    if (oz_isVarOrRef(t)) {
      return oz_addSuspendVarList(t_ptr);
    }
  }

  return PROCEED;
} OZ_BI_ioend


OZ_BI_iodefine(unix_writeSelect,1,0) {
  OZ_declareInt(0,fd);

  WRAPCALL("select",osTestSelect(fd,SEL_WRITE),sel);

  if (sel == 0) {
    TaggedRef t = oz_newVariable();

    (void) OZ_writeSelect(fd, NameUnit, t);
    DEREF(t, t_ptr);

    Assert(!oz_isRef(t));
    if (oz_isVarOrRef(t)) {
      return oz_addSuspendVarList(t_ptr);
    }
  }

  return PROCEED;
} OZ_BI_ioend


OZ_BI_iodefine(unix_acceptSelect,1,0) {
  OZ_declareInt(0,fd);

  WRAPCALL("select",osTestSelect(fd,SEL_READ),sel);

  if (sel == 0) {

    TaggedRef t = oz_newVariable();

    (void) OZ_acceptSelect(fd, NameUnit, t);
    DEREF(t, t_ptr);

    Assert(!oz_isRef(t));
    if (oz_isVarOrRef(t)) {
      return oz_addSuspendVarList(t_ptr);
    }
  }

  return PROCEED;
} OZ_BI_ioend




OZ_BI_define(unix_deSelect,1,0) {
  OZ_declareInt(0,fd);
  OZ_deSelect(fd);
  return PROCEED;
} OZ_BI_end




// -------------------------------------------------
// sockets
// -------------------------------------------------
OZ_BI_iodefine(unix_socket,3,1)
{
  OZ_declareAtom(0, OzDomain);
  OZ_declareAtom(1, OzType);
  OZ_declareVsIN(2, OzProtocol);

  int domain, type, protocol;

  // compute domain
#ifndef WINDOWS
  if (!strcmp(OzDomain,"PF_UNIX")) {
    domain = PF_UNIX;
  } else
#endif
  if (!strcmp(OzDomain,"PF_INET")) {
    domain = PF_INET;
  } else {
    return OZ_typeError(0,"enum(PF_UNIX PF_INET)");
  }

  // compute type
  if (!strcmp(OzType,"SOCK_STREAM")) {
    type = SOCK_STREAM;
  } else if (!strcmp(OzType,"SOCK_DGRAM")) {
    type = SOCK_DGRAM;
  } else {
    return OZ_typeError(1,"enum(SOCK_STREAM SOCK_DGRAM)");
  }

  // compute protocol
  if (*OzProtocol != '\0') {
    struct protoent *proto;
    osBlockSignals();
    proto = getprotobyname(OzProtocol);
    osUnblockSignals();
    if (!proto) {
      return OZ_typeError(2,"enum protocol");
    }

    protocol = proto->p_proto;
  } else {
    protocol = 0;
  }

  WRAPCALL("socket",ossocket(domain, type, protocol), sock);

  OZ_RETURN_INT(sock);
} OZ_BI_ioend

OZ_BI_iodefine(unix_bindInet,2,0)
{
  OZ_declareInt(0,sock);
  OZ_declareInt(1,port);

  struct sockaddr_in addr;

  memset((char *)&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons ((unsigned short) port);

  WRAPCALL("bind",bind(sock,(struct sockaddr *)&addr,sizeof(struct
                                                     sockaddr_in)),ret);
  return PROCEED;
} OZ_BI_ioend


OZ_BI_define(unix_getSockName,1,1)
{
  OZ_declareInt(0,s);

  struct sockaddr_in addr;

#if __GLIBC__ == 2
  unsigned int length = sizeof(addr);
#else
  socklen_t length = sizeof(addr);
#endif

  WRAPCALL("getsockname",getsockname(s, (struct sockaddr *) &addr, &length), ret);

  OZ_RETURN_INT(ntohs(addr.sin_port));
} OZ_BI_end


OZ_BI_iodefine(unix_listen,2,0)
{
  OZ_declareInt(0, s);
  OZ_declareInt(1, n);

  WRAPCALL("listen",listen(s,n), ret);

  return PROCEED;
} OZ_BI_ioend

OZ_BI_define(unix_connectInet,3,0)
{
  OZ_declareInt(0, s);
  OZ_declareTerm(1, host);
  OZ_declareInt(2, port);

  struct sockaddr_in addr;

  if(OZ_isInt(host)) {
    addr.sin_addr.s_addr=htonl(OZ_intToC(host));
    addr.sin_family = AF_INET;
    addr.sin_port = htons ((unsigned short) port);
  }
  else if(OZ_isVirtualString(host,0)){
    struct hostent *hostaddr;
    if ((hostaddr = gethostbyname(OZ_virtualStringToC(host,0))) == NULL) {
      RETURN_NET_ERROR("gethostbyname");
    }

    memset((char *)&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr,hostaddr->h_addr_list[0],sizeof(addr.sin_addr));
    addr.sin_port = htons ((unsigned short) port);
  }
  else
    OZ_typeError(1,"VirtualString");

  int ret = osconnect(s,(struct sockaddr *) &addr,sizeof(addr));
  if (ret<0) {
    Assert(errno != EINTR);
    RETURN_UNIX_ERROR("connect");
  }

  return PROCEED;
} OZ_BI_end

#ifdef WINDOWS
/* connect under windows are always done in blocking mode */
void osSetNonBlocking(int fd, Bool onoff) {
  u_long dd = onoff;
  int ret = ioctlsocket(fd,FIONBIO,&dd);
  if (ret<0)
    message("ioctlsocket(%d,FIONBIO,%d) failed: %d\n",fd,onoff,ossockerrno());
}
#endif

OZ_BI_define(unix_connect_nonblocking,3,0)
{
  OZ_declareInt(0, s);
  OZ_declareTerm(1, host);
  OZ_declareInt(2, port);

  struct sockaddr_in addr;

  if(OZ_isInt(host)) {
    addr.sin_addr.s_addr=htonl(OZ_intToC(host));
    addr.sin_family = AF_INET;
    addr.sin_port = htons ((unsigned short) port);
  }
  else if(OZ_isVirtualString(host,0)){
    struct hostent *hostaddr;
    if ((hostaddr = gethostbyname(OZ_virtualStringToC(host,0))) == NULL) {
      RETURN_NET_ERROR("gethostbyname");
    }

    memset((char *)&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr,hostaddr->h_addr_list[0],sizeof(addr.sin_addr));
    addr.sin_port = htons ((unsigned short) port);
  }
  else
    OZ_typeError(1,"VirtualString");

  // Make the socket nonblocking, non nagle
  int one   =  1;
  if(setsockopt(s,IPPROTO_TCP,TCP_NODELAY,(char*) &one,sizeof(one))<0){
    RETURN_UNIX_ERROR("connectNonblocking");
  }

#ifndef WINDOWS
  fcntl(s,F_SETFL,O_NDELAY);
#endif

  //

  int ret = osconnect(s,(struct sockaddr *) &addr,sizeof(addr));
  if (ret<0) {
    Assert(errno != EINTR);
    RETURN_UNIX_ERROR("connectNonblocking");
  }
#ifdef WINDOWS
  else {
    // do this later on Windows otherwise it doesn't work
    osSetNonBlocking(s,OK);
  }
#endif

  return PROCEED;
} OZ_BI_end

OZ_BI_iodefine(unix_acceptInet,1,3)
{
  OZ_declareInt(0, sock);
  // OZ_out(0) == host
  // OZ_out(1) == port
  // OZ_out(2) == fd

  struct sockaddr_in from;
  int fromlen = sizeof from;

  WRAPCALL("accept",osaccept(sock,(struct sockaddr *)&from, &fromlen),fd);

  const char *host = inet_ntoa(from.sin_addr);
  if (strcmp(host,"127.0.0.1")==0) {  // this prevents network connections being
    host = "localhost";               // opened when working at home for example
  } else {
    struct hostent *gethost = gethostbyaddr((char *) &from.sin_addr,
                                            fromlen, AF_INET);
    if (gethost) {
      host = gethost->h_name;
    }
  }
  OZ_out(0) = OZ_string(host);
  OZ_out(1) = OZ_int(ntohs(from.sin_port));
  OZ_out(2) = OZ_int(fd);
  return PROCEED;
} OZ_BI_ioend

OZ_BI_iodefine(unix_accept_nonblocking,1,3)
{
  OZ_declareInt(0, sock);
  // OZ_out(0) == host
  // OZ_out(1) == port
  // OZ_out(2) == fd

  struct sockaddr_in from;
  int fromlen = sizeof from;

  WRAPCALL("accept",osaccept(sock,(struct sockaddr *)&from, &fromlen),fd);

  // Nonblocking
  int one=1;
  if(setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char*) &one,sizeof(one))<0){
    RETURN_UNIX_ERROR("acceptNonblocking");
  }

#ifdef WINDOWS
  osSetNonBlocking(fd,OK);
#else
  fcntl(fd,F_SETFL,O_NDELAY);
#endif
  //

  const char *host = inet_ntoa(from.sin_addr);
  if (strcmp(host,"127.0.0.1")==0) {  // this prevents network connections being
    host = "localhost";               // opened when working at home for example
  } else {
    osBlockSignals();
    struct hostent *gethost = gethostbyaddr((char *) &from.sin_addr,
                                            fromlen, AF_INET);
    osUnblockSignals();
    if (gethost) {
      host = gethost->h_name;
    }
  }
  OZ_out(0) = OZ_string(host);
  OZ_out(1) = OZ_int(ntohs(from.sin_port));
  OZ_out(2) = OZ_int(fd);
  return PROCEED;
} OZ_BI_ioend


/* This has the same functionality as unix_accept_nonblocking but with
   a small difference. That is, there is no name service query. The
   Host output parameter will be a string representing the IP of the
   end-host. */
OZ_BI_iodefine(unix_accept_nonblocking_noDnsLookup,1,3)
{
  OZ_declareInt(0, sock);
  // OZ_out(0) == host
  // OZ_out(1) == port
  // OZ_out(2) == fd

  struct sockaddr_in from;
  int fromlen = sizeof from;

  WRAPCALL("accept",osaccept(sock,(struct sockaddr *)&from, &fromlen),fd);

  // Nonblocking
  int one=1;
  if(setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char*) &one,sizeof(one))<0){
    RETURN_UNIX_ERROR("acceptNonblocking");
  }

#ifdef WINDOWS
  osSetNonBlocking(fd,OK);
#else
  fcntl(fd,F_SETFL,O_NDELAY);
#endif
  //

  char *host = inet_ntoa(from.sin_addr);

  OZ_out(0) = OZ_string(host);
  OZ_out(1) = OZ_int(ntohs(from.sin_port));
  OZ_out(2) = OZ_int(fd);
  return PROCEED;
} OZ_BI_ioend


static OZ_Return get_send_recv_flags(OZ_Term OzFlags, int * flags)
{
  OZ_Term hd, tl;

  *flags = 0;

  while (unixIsCons(OzFlags, &hd, &tl)) {

    if (OZ_isVariable(hd))
      return SUSPEND;

    if (OZ_eqAtom(hd,"MSG_OOB") == PROCEED) {
      *flags |= MSG_OOB;
    } else if (OZ_eqAtom(hd,"MSG_PEEK") == PROCEED) {
      *flags |= MSG_PEEK;
    } else {
      return OZ_typeError(-1,"enum(MSG_OOB MSG_PEEK)");
    }

    OzFlags = tl;
  }

  if (OZ_isVariable(OzFlags))
    return SUSPEND;

  if (!OZ_isNil(OzFlags)) {
    return OZ_typeError(-1,"enum(MSG_OOB MSG_PEEK)");
  }

  return PROCEED;
}


OZ_BI_iodefine(unix_send, 3,1)
{
  OZ_declareInt(0, sock);
  DeclareNonvarIN(1, vs);
  DeclareAtomListIN(2, OzFlags);


  int flags;
  OZ_Return flagBool;

  if (!((flagBool = get_send_recv_flags(OzFlags,&flags)) == PROCEED))
      return flagBool;

  CHECK_WRITE(sock);

  {
    int len;
    OZ_Return status;
    OZ_Term rest, susp, from_buff, rest_all;
    vs_buff(write_buff);

    status = buffer_vs(vs, write_buff, &len, &rest, &susp);

    if (status != PROCEED && status != SUSPEND)
      return status;

    WRAPCALL("send",send(sock, write_buff, len, flags), ret);

    if (len==ret && status != SUSPEND) {
      OZ_RETURN_INT(len);
    }

    if (status != SUSPEND) {
      susp = oz_nil();
      rest = susp;
    }

    if (len > ret) {
      from_buff = buff2list(len - ret, write_buff + ret);

      rest_all = OZ_pair2(from_buff,rest);

      NEW_RETURN_SUSPEND(OZ_int(ret),susp,rest_all);
    } else {
      NEW_RETURN_SUSPEND(OZ_int(ret),susp,rest);
    }

  }
} OZ_BI_ioend

OZ_BI_iodefine(unix_sendToInet, 5,1)
{
  OZ_declareInt(0, sock);
  DeclareNonvarIN(1, vs);
  DeclareAtomListIN(2, OzFlags);
  OZ_declareVsIN(3, host);
  OZ_declareInt(4, port);

  int flags;
  OZ_Return flagBool;

  if (!((flagBool = get_send_recv_flags(OzFlags,&flags)) == PROCEED))
      return flagBool;

  CHECK_WRITE(sock);

  {
    struct hostent *hostaddr;

    if ((hostaddr = gethostbyname(host)) == NULL) {
      RETURN_NET_ERROR("gethostbyname");
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

    if (status != PROCEED && status != SUSPEND)
      return status;

    WRAPCALL("sendto",sendto(sock, write_buff, len, flags,
                    (struct sockaddr *) &addr, sizeof(addr)), ret);

    if (len==ret && status != SUSPEND) {
      OZ_RETURN_INT(len);
    }

    if (status != SUSPEND) {
      susp = oz_nil();
      rest = susp;
    }

    if (len > ret) {
      OZ_Term rest_all = OZ_pair2(buff2list(len - ret, write_buff + ret),rest);

      NEW_RETURN_SUSPEND(OZ_int(ret),susp,rest_all);
    } else {
      NEW_RETURN_SUSPEND(OZ_int(ret),susp,rest);
    }
  }

} OZ_BI_ioend


OZ_BI_iodefine(unix_shutDown, 2,0)
{
  OZ_declareInt(0,sock);
  OZ_declareInt(1,how);

  WRAPCALL("shutdown",shutdown(sock, how), ret);

  return PROCEED;
} OZ_BI_ioend



OZ_BI_iodefine(unix_receiveFromInet,5,3)
{
  OZ_declareInt(0,sock);
  OZ_declareInt(1,maxx);
  DeclareAtomListIN(2, OzFlags);
  OZ_declareTerm(3, hd);
  OZ_declareTerm(4, tl);
  // OZ_out(0) == host
  // OZ_out(1) == port
  // OZ_out(2) == n

  if (!(!OZ_isVariable(hd) || oz_isFree(oz_deref(hd))))
    return (OZ_typeError(3, "value or a free variable"));

  int flags;
  OZ_Return flagBool;

  if (!((flagBool = get_send_recv_flags(OzFlags,&flags)) == PROCEED))
      return flagBool;

  CHECK_READ(sock);

  char *buf = (char *) malloc(maxx+1);

  struct sockaddr_in from;

#if __GLIBC__ == 2
  unsigned int fromlen = sizeof from;
#else
  socklen_t fromlen = sizeof from;
#endif

  WRAPCALL("recvfrom",recvfrom(sock, buf, maxx, flags,
                    (struct sockaddr*)&from, &fromlen),ret);

  struct hostent *gethost = gethostbyaddr((char *) &from.sin_addr,
                                            fromlen, AF_INET);

  OZ_Term localhead = oz_string(buf, ret, tl);

  free(buf);

  OZ_Return ures = oz_unify(localhead, hd);
  Assert(ures == PROCEED || ures == FAILED);
  if (ures == FAILED) return (FAILED);

  OZ_out(0) = OZ_string(gethost ?
                        (OZ_CONST char*) gethost->h_name :
                        (OZ_CONST char*) inet_ntoa(from.sin_addr));
  OZ_out(1) = OZ_int(ntohs(from.sin_port));
  OZ_out(2) = OZ_int(ret);
  return (PROCEED);
} OZ_BI_ioend


OZ_BI_iodefine(unix_receiveFromInetAnon,5,1)
{
  OZ_declareInt(0,sock);
  OZ_declareInt(1,maxx);
  DeclareAtomListIN(2, OzFlags);
  OZ_declareTerm(3, hd);
  OZ_declareTerm(4, tl);
  // OZ_out(0) == n

  if (!(!OZ_isVariable(hd) || oz_isFree(oz_deref(hd))))
    return (OZ_typeError(3, "value or a free variable"));

  int flags;
  OZ_Return flagBool;

  if (!((flagBool = get_send_recv_flags(OzFlags,&flags)) == PROCEED))
      return flagBool;

  CHECK_READ(sock);

  char *buf = (char *) malloc(maxx+1);

  struct sockaddr_in from;

#if __GLIBC__ == 2
  unsigned int fromlen = sizeof from;
#else
  socklen_t fromlen = sizeof from;
#endif

  WRAPCALL("recvfrom",recvfrom(sock, buf, maxx, flags,
                    (struct sockaddr*)&from, &fromlen),ret);

  OZ_Term localhead = oz_string(buf, ret, tl);

  free(buf);

  OZ_Return ures = oz_unify(localhead, hd);
  Assert(ures == PROCEED || ures == FAILED);
  if (ures == FAILED) return (FAILED);

  OZ_out(0) = OZ_int(ret);
  return (PROCEED);
} OZ_BI_ioend


const int maxArgv = 100;
static char* argv[maxArgv];

static OZ_Return enter_exec_args(char * s, OZ_Term args, int &argno) {
  OZ_Term hd, tl, argl;

  argno = 0;

  argl=args;

  while (unixIsCons(argl, &hd, &tl)) {
    OZ_Term var = (OZ_Term) 0;
    if (!OZ_isVirtualString(hd,&var)) {
      if (var) {
        OZ_suspendOnInternal(var);
        return SUSPEND;
      } else {
        return OZ_typeError(1, "list(VirtualString)");
      }
    }
    argno++;
    argl = tl;
  }

  if (OZ_isVariable(argl)) {
    OZ_suspendOnInternal(argl);
    return SUSPEND;
  }

  if (!OZ_isNil(argl))
    return OZ_typeError(1,"list(VirtualString)");

  argl=args;

  if (argno+2 >= maxArgv) {
    return oz_raise(E_SYSTEM,E_SYSTEM,"limitInternal",1,
                    OZ_string("too many arguments for pipe"));
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
      Assert(!OZ_isVariable(susp));
      return oz_raise(E_SYSTEM,E_SYSTEM,"limitInternal",1,
                   OZ_string("virtual string too long"));
    }
    Assert(status == PROCEED);
    *(vsarg+len) = '\0';

    argv[argno++] = vsarg;

    argl = tl;
  }

  return PROCEED;

}


OZ_BI_define(unix_pipe,2,2) {
  OZ_declareVsIN(0, s);
  OZ_declareTerm(1, args);
  int i;
  // OZ_out(0) == rpid
  // OZ_out(1) == rwsock

  int argno;
  OZ_Return status = enter_exec_args(s, args, argno);

  if (status != PROCEED)
    return status;

#ifdef WINDOWS
  int cmdlen = 0;
  for (i = 0; i < argno; i++)
    cmdlen += strlen(argv[i]) + 3;
  DECL_DYN_ARRAY(char, cmdline, cmdlen);
  char *p = cmdline;
  for (i = 0; i < argno; i++) {
    *p++ = '\"';
    *p = '\0';
    strcat(p,argv[i]);
    p += strlen(argv[i]);
    *p++ = '\"';
    if (i != argno - 1)
      *p++ = ' ';
    *p = '\0';
  }

  int sv[2];
  int aux = ossocketpair(PF_UNIX,SOCK_STREAM,0,sv);

  HANDLE rh0,wh0,rh1,wh1,wh2;
  {
    HANDLE wh0Tmp,rh1Tmp;

    SECURITY_ATTRIBUTES sa1;
    sa1.nLength = sizeof(sa1);
    sa1.lpSecurityDescriptor = NULL;
    sa1.bInheritHandle = TRUE;
    SECURITY_ATTRIBUTES sa2;
    sa2.nLength = sizeof(sa2);
    sa2.lpSecurityDescriptor = NULL;
    sa2.bInheritHandle = TRUE;

    if (!CreatePipe(&rh0,&wh0Tmp,&sa1,0)  ||
        !CreatePipe(&rh1Tmp,&wh1,&sa2,0)) {
      return raiseUnixError("CreatePipe",0,"Cannot create pipe.","os");
    }

    // The child must only inherit one side of each pipe.
    // Else the inherited handle will cause the pipe to remain open
    // even though we may have closed it, resulting in the child
    // never getting an EOF on it.
    if (!DuplicateHandle(GetCurrentProcess(),wh0Tmp,
                         GetCurrentProcess(),&wh0,0,
                         FALSE,DUPLICATE_SAME_ACCESS) ||
        !DuplicateHandle(GetCurrentProcess(),rh1Tmp,
                         GetCurrentProcess(),&rh1,0,
                         FALSE,DUPLICATE_SAME_ACCESS)) {
      return raiseUnixError("DuplicateHandle",0,
                            "Cannot duplicate handle.","os");
    }
    CloseHandle(wh0Tmp);
    CloseHandle(rh1Tmp);
  }

  // We need to duplicate the handle in case the child closes
  // either its output or its error handle.
  if (!DuplicateHandle(GetCurrentProcess(),wh1,
                       GetCurrentProcess(),&wh2,0,
                       TRUE,DUPLICATE_SAME_ACCESS)) {
    return raiseUnixError("DuplicateHandle",0,"Cannot duplicate handle.","os");
  }

  STARTUPINFO si;
  memset(&si,0,sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_FORCEOFFFEEDBACK|STARTF_USESTDHANDLES;
  si.hStdInput  = rh0;
  si.hStdOutput = wh1;
  si.hStdError  = wh2;

  PROCESS_INFORMATION pinf;
  if (!CreateProcess(NULL,cmdline,NULL,NULL,TRUE,0,NULL,NULL,&si,&pinf)) {
    return raiseUnixError("CreateProcess",0,"Cannot create process.","os");
  }

  int pid = pinf.dwProcessId;

  CloseHandle(rh0);
  CloseHandle(wh1);
  CloseHandle(wh2);
  CloseHandle(pinf.hProcess); //--** this is unsafe! keep open while pid used
  CloseHandle(pinf.hThread);

  // forward the handles to sockets so that we can do select() on them:
  createWriter(sv[1],wh0);
  createReader(sv[1],rh1); //--** sv[1] will be closed twice

#else  /* !WINDOWS */

  int sv[2];
  WRAPCALL("socketpair",socketpair(PF_UNIX,SOCK_STREAM,0,sv),ret);


  int pid =  fork();
  switch (pid) {
  case 0: // child
    {

#ifdef DEBUG_FORK_GROUP
      /*
       * create a new process group for child
       *   this allows to press Control-C when debugging the emulator
       */
      if (setsid() < 0) {
        fprintf(stderr,"setsid failed\n");
        exit(-1);
      }
#endif

      // the child process should not produce a core file -- otherwise
      // we get a problem if all core files are just named 'core', because
      // the emulator's core file gets overwritten immediately by wish's one...
      struct rlimit rlim;
      rlim.rlim_cur = 0;
      rlim.rlim_max = 0;
      if (setrlimit(RLIMIT_CORE, &rlim) < 0) {
        fprintf(stderr,"setrlimit failed\n");
        exit(-1);
      }

      for (i = 0; i < FD_SETSIZE; i++) {
        if (i != sv[1]) {
          close(i);
        }
      }
      osdup(sv[1]);
      osdup(sv[1]);
      osdup(sv[1]);
      if (execvp(s,argv)  < 0) {
        fprintf(stderr,"execvp failed\n");
        exit(-1);
      }
      printf("this should never happen\n");
      exit(-1);
    }
  case -1:
    RETURN_UNIX_ERROR("fork");
  default: // parent
    break;
  }
  close(sv[1]);

#endif

  int rsock = sv[0];
  /* we can use the same descriptor for both reading and writing: */
  int wsock = rsock;

  for (i=1 ; i<argno ; i++)
    free(argv[i]);

  addChildProc(pid);

  TaggedRef rw = OZ_pair2(OZ_int(rsock),OZ_int(wsock));
  OZ_out(0) = OZ_int(pid);
  OZ_out(1) = rw;
  return PROCEED;
} OZ_BI_end


OZ_BI_define(unix_exec,3,1){
  OZ_declareVsIN(0, s);
  OZ_declareTerm(1, args);
  OZ_declareBool(2, do_kill);
  int i;

  // OZ_out(0) == rpid

  int argno;
  OZ_Return status = enter_exec_args(s, args, argno);

  if (status != PROCEED)
    return status;

#ifdef WINDOWS
  int cmdlen = 0;
  for (i = 0; i < argno; i++)
    cmdlen += strlen(argv[i]) + 3;
  DECL_DYN_ARRAY(char, cmdline, cmdlen);
  char *p = cmdline;
  for (i = 0; i < argno; i++) {
    *p++ = '\"';
    *p = '\0';
    strcat(p,argv[i]);
    p += strlen(argv[i]);
    *p++ = '\"';
    if (i != argno - 1)
      *p++ = ' ';
    *p = '\0';
  }

  STARTUPINFO si;
  memset(&si,0,sizeof(si));
  si.cb = sizeof(si);
  if (do_kill) {
    si.dwFlags = STARTF_USESTDHANDLES;
    SetHandleInformation(GetStdHandle(STD_INPUT_HANDLE),
                         HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT);
    SetHandleInformation(GetStdHandle(STD_OUTPUT_HANDLE),
                         HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT);
    SetHandleInformation(GetStdHandle(STD_ERROR_HANDLE),
                         HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT);
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  }

  // If we don't want the child to be killed and we had OZPPID set,
  // then we save its value and unset it.  Otherwise, the buffer
  // contains an empty string.
  char ozppidbuf[100];
  if (!do_kill) {
    if (!GetEnvironmentVariable("OZPPID", ozppidbuf, sizeof(ozppidbuf))) {
      ozppidbuf[0] = '\0';
    } else {
      SetEnvironmentVariable("OZPPID", NULL);
      Assert(ozppidbuf[0] != '\0');
    }
  } else {
    ozppidbuf[0] = '\0';
  }

  PROCESS_INFORMATION pinf;
  if (!CreateProcess(NULL,cmdline,NULL,NULL,FALSE,
                     (do_kill ? 0 : DETACHED_PROCESS),
                     NULL,NULL,&si,&pinf)) {
    if (ozppidbuf[0] != '\0')
      SetEnvironmentVariable("OZPPID", ozppidbuf);
    return raiseUnixError("exec",0,"Cannot exec process.","os");
  }
  CloseHandle(pinf.hThread);
  CloseHandle(pinf.hProcess); //--** unsafe! keep open while pid used

  if (ozppidbuf[0] != '\0')
    SetEnvironmentVariable("OZPPID", ozppidbuf);

  int pid = pinf.dwProcessId;

#else  /* !WINDOWS */


  int pid =  fork();
  switch (pid) {
  case 0: // child
    {

#ifdef DEBUG_FORK_GROUP
      /*
       * create a new process group for child
       *   this allows to press Control-C when debugging the emulator
       */
      if (setsid() < 0) {
        // kost@ : raising an exception here makes no sense - that's
        // the child process...
        fprintf(stderr, "setsid failed\n");
        exit(-1);
      }
#endif

      // the child process should not produce a core file -- otherwise
      // we get a problem if all core files are just named 'core',
      // because the emulator's core file gets overwritten immediately
      // by wish's one...
      struct rlimit rlim;
      rlim.rlim_cur = 0;
      rlim.rlim_max = 0;
      if (setrlimit(RLIMIT_CORE, &rlim) < 0) {
        fprintf(stderr, "setrlimit failed\n");
        exit(-1);
      }

#ifdef DEBUG_CHECK
      // kost@ : leave 'std???' in place in debug mode since otherwise
      // one cannot see what forked sites are trying to say us.
      // However, this makes e.g. the 'detach' functionality of remote
      // servers non-working (but who wants it in debug mode anyway?)
      for (i = 3; i<FD_SETSIZE; i++)
        close(i);
#else
      if (do_kill) {
        for (i = 3; i<FD_SETSIZE; i++)
          close(i);
      } else {
        for (i = FD_SETSIZE; i--; )
          close(i);

        WRAPCALL("open",open("/dev/null", O_RDWR),dn);
        osdup(dn);              // stdout
        osdup(dn);              // stderr
      }
#endif

      int execRet;
      execRet = execvp(s, argv);
      Assert(execRet < 0);
      fprintf(stderr, "execvp failed\n");
      exit(-101);
    }

  case -1:
    RETURN_UNIX_ERROR("fork");

  default: // parent
    break;
  }

#endif

  for (i=1 ; i<argno ; i++)
    free(argv[i]);

  if (do_kill) {
    addChildProc(pid);
  }

  OZ_RETURN(OZ_int(pid));
} OZ_BI_end

static OZ_Term mkAliasList(char **alias)
{
  OZ_Term ret = oz_nil();
  while (*alias != 0) {
    ret = oz_cons(OZ_string(*alias), ret);
    alias++;
  }
  return ret;
}

static OZ_Term mkAddressList(char **lstptr)
{
  OZ_Term ret = oz_nil();
  while (*lstptr != NULL) {
    ret = oz_cons(OZ_string(osinet_ntoa(*lstptr)),ret);
    lstptr++;
  }
  return ret;
}

OZ_BI_iodefine(unix_getHostByName, 1,1)
{
  OZ_declareVsIN(0, name);

  struct hostent *hostaddr;

  if ((hostaddr = gethostbyname(name)) == NULL) {
    RETURN_NET_ERROR("gethostbyname");
  }

  OZ_MAKE_RECORD_S("hostent",3,
                   {"name" OZ_COMMA "aliases" OZ_COMMA "addrList"},
                   {OZ_string(hostaddr->h_name) OZ_COMMA
                      mkAliasList(hostaddr->h_aliases) OZ_COMMA
                      mkAddressList(hostaddr->h_addr_list)},
                   r);

  OZ_RETURN(r);
} OZ_BI_ioend


// Misc stuff

OZ_BI_iodefine(unix_rmDir,1,0) {
  OZ_declareVsIN(0,path);
  WRAPCALL("rmdir",rmdir(path),ret);
  return PROCEED;
} OZ_BI_ioend

OZ_BI_iodefine(unix_unlink, 1,0) {
  OZ_declareVsIN(0,path);

  WRAPCALL("unlink",unlink(path),ret);
  return PROCEED;
} OZ_BI_ioend


OZ_BI_iodefine(unix_system,1,1)
{
  OZ_declareVsIN(0, vs);

  int ret = osSystem(vs);

  OZ_RETURN_INT(ret);
} OZ_BI_ioend


OZ_BI_iodefine(unix_wait,0,2)
{
  // OZ_out(0) == rpid
  // OZ_out(1) == rstat

#if !defined(WINDOWS) || defined(__CYGWIN32__)
  int status;
  int pid = waitpid(-1, &status, WNOHANG | WUNTRACED);

  OZ_out(0) = OZ_int(pid);
  OZ_out(1) = OZ_int(status);
#endif

  return PROCEED;
} OZ_BI_ioend


OZ_BI_iodefine(unix_getServByName, 2,1)
{
  OZ_declareVsIN(0, name);
  OZ_declareVsIN(1, proto);

  struct servent *serv;
  serv = getservbyname(name, proto);

  if (!serv) OZ_RETURN(OZ_false());

  OZ_RETURN_INT(ntohs(serv->s_port));
} OZ_BI_ioend

OZ_BI_iodefine(unix_tmpnam,0,1) {
  char *filename;

  if (!(filename = ostmpnam(NULL))) {
    return raiseUnixError("tmpnam",0, "OS.tmpnam failed.", "os");
  }
  filename = strdup(filename);

  OZ_RETURN_STRING(filename);
} OZ_BI_ioend


OZ_BI_iodefine(unix_getEnv,1,1)
{
  OZ_declareVsIN(0, envVar);

  char *envValue;

  envValue = osgetenv(envVar);
  if (envValue == 0) OZ_RETURN(OZ_false());

  OZ_RETURN_STRING(envValue);
} OZ_BI_ioend

#ifdef __FCC_VERSION
extern "C" {
  int putenv(char *);
}
#endif


/* putenv is NOT POSIX !!! */
OZ_BI_iodefine(unix_putEnv,2,0)
{
  OZ_declareVsIN(0, envVar);
  OZ_declareVsIN(1, envValue);

#ifdef WINDOWS
  if (SetEnvironmentVariable(envVar,envValue) == FALSE) {
    return raiseUnixError("putenv", 0, "OS.putEnv failed.", "os");
  }
#else
  char *buf = new char[strlen(envVar)+strlen(envValue)+2];
  sprintf(buf,"%s=%s",envVar,envValue);
  int ret = putenv(buf);
  if (ret != 0) {
    delete buf;
    return raiseUnixError("putenv", 0, "OS.putEnv failed.", "os");
  }
#endif

  return PROCEED;
} OZ_BI_ioend


OZ_Term make_time(const struct tm* tim) {
  OZ_MAKE_RECORD_S("time",9,
                   {"hour" OZ_COMMA
                      "isDst" OZ_COMMA
                      "mDay" OZ_COMMA
                      "min" OZ_COMMA
                      "mon" OZ_COMMA
                      "sec" OZ_COMMA
                      "wDay" OZ_COMMA
                      "yDay" OZ_COMMA
                      "year"},
                   { oz_int(tim->tm_hour) OZ_COMMA
                       oz_int(tim->tm_isdst) OZ_COMMA
                       oz_int(tim->tm_mday) OZ_COMMA
                       oz_int(tim->tm_min) OZ_COMMA
                       oz_int(tim->tm_mon) OZ_COMMA
                       oz_int(tim->tm_sec) OZ_COMMA
                       oz_int(tim->tm_wday) OZ_COMMA
                       oz_int(tim->tm_yday) OZ_COMMA
                       oz_int(tim->tm_year) },r);

  return r;
}

OZ_BI_iodefine(unix_time, 0,1)
{
  OZ_RETURN_INT(time(0));
} OZ_BI_ioend

OZ_BI_iodefine(unix_gmTime,0,1)
{
  time_t timebuf;

  time(&timebuf);
  OZ_RETURN(make_time(gmtime(&timebuf)));

} OZ_BI_ioend

OZ_BI_iodefine(unix_localTime, 0,1)
{
  time_t timebuf;

  time(&timebuf);
  OZ_RETURN(make_time(localtime(&timebuf)));

} OZ_BI_ioend

OZ_BI_define(unix_rand, 0,1)
{
  OZ_RETURN_INT(rand());
} OZ_BI_end

OZ_BI_define(unix_srand, 1,0)
{
  OZ_declareInt(0, seed);

  if (seed) {
    srand((unsigned int) seed);
  } else {
    srand((unsigned int) time(NULL));
  }

  return PROCEED;
} OZ_BI_end

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

OZ_BI_define(unix_randLimits, 0,2)
{
  // OZ_out(0) == minn
  // OZ_out(1) == maxx

  OZ_out(0) = oz_int(0);
  OZ_out(1) = oz_int(RAND_MAX);
  return PROCEED;
} OZ_BI_end

#ifdef WALSER
OZ_BI_define(unix_random, 0,1)
{
#if defined(SOLARIS) || defined(SUNOS_SPARC) || defined(LINUX)
  OZ_RETURN_INT(random());
#else
  return oz_raise(E_SYSTEM,E_SYSTEM,"limitExternal",1,OZ_atom("OS.random"));
  // OZ_RETURN_INT(rand())
#endif
} OZ_BI_end


OZ_BI_define(unix_srandom, 1,0)
{
  OZ_declareInt(0, seed);

  if (!seed) { seed = time(NULL); }

#if defined(SOLARIS) || defined(SUNOS_SPARC) || defined(LINUX)
  srandom((unsigned int) seed);
#else
  return oz_raise(E_SYSTEM,E_SYSTEM,"limitExternal",1,OZ_atom("OS.srandom"));
  // srand((unsigned int) seed);
#endif

  return PROCEED;
} OZ_BI_end
#endif


#ifdef WINDOWS

OZ_BI_define(unix_getpwnam,1,1)
{
  return oz_raise(E_SYSTEM,E_SYSTEM,
                  "limitExternal",1,OZ_atom("OS.getpwnam"));
} OZ_BI_end

#else

#include <pwd.h>

#include <sys/types.h>

OZ_BI_define(unix_getpwnam,1,1)
{
  OZ_declareVirtualString(0,user);
retry:
  struct passwd *p = getpwnam(user);
  if (p==0) {
    if (errno==EINTR) goto retry;
    return raiseUnixError("getpwnam",errno,OZ_unixError(errno),"os");
  } else {
    OZ_MAKE_RECORD_S("passwd",5,
                     {"name" OZ_COMMA "uid" OZ_COMMA
                        "gid" OZ_COMMA "dir" OZ_COMMA
                        "shell" },
                     {oz_atom(p->pw_name) OZ_COMMA oz_int(p->pw_uid) OZ_COMMA
                        oz_int(p->pw_gid) OZ_COMMA oz_atom(p->pw_dir) OZ_COMMA
                        oz_atom(p->pw_shell)},
                     r);
    OZ_RETURN(r);
  }
} OZ_BI_end

#endif


OZ_BI_define(unix_signalHandler, 2,0)
{
  OZ_declareAtom(0,signo);
  OZ_declareDetTerm(1,handler);

  if (!(OZ_eq(handler,OZ_atom("ignore")) || OZ_eq(handler,OZ_atom("default")) ||
        OZ_isProcedure(handler) && oz_procedureArity(oz_deref(handler)) == 1)) {
    return OZ_typeError(1,"unary procedure or 'default' or 'ignore'");
  }

  if (osSignal(signo,handler))
    return PROCEED;

  return OZ_typeError(0,"signal name");
} OZ_BI_end


OZ_BI_define(unix_kill, 2,0)
{
  OZ_declareInt(0,pid);
  OZ_declareAtom(1,signo);
  OZ_RETURN_INT(oskill(pid,atomToSignal(signo)));
} OZ_BI_end




OZ_BI_define(unix_getpid,0,1) {
  OZ_RETURN(oz_int(getpid()));
} OZ_BI_end


OZ_BI_define(unix_setpgid,2,1)
{
  OZ_declareInt(0,pid);
  OZ_declareInt(1,pgid);
#ifdef HAVE_SETPGID
  int ret = setpgid(pid,pgid);
#else
  int ret = -1;
#endif
  OZ_RETURN(oz_int(ret));
} OZ_BI_end
