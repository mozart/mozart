/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Denys Duchier (duchier@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#ifdef WINDOWS
#include "wsock.hh"
#else
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <netdb.h>
#include "runtime.hh"
#include "codearea.hh"
#include "indexing.hh"

#include "perdio.hh"
#include "perdio_debug.hh"  
#include "genvar.hh"
#include "perdiovar.hh"
#include "gc.hh"
#include "dictionary.hh"
#include "genhashtbl.hh"
#include "urlc.hh"
#include "marshaler.hh"
#include "comm.hh"
#include "msgbuffer.hh"

// ATTENTION
#define tcpHeaderSize 	7


#include "componentBuffer.cc"


// static OZ_Term url_map=0;
// static OZ_Term OZ_Cache_Path = 0;

/* ********************************************************************** */
/*                                                                        */
/* ********************************************************************** */

/* ********************************************************************** */
/*              BUILTINS                                                  */
/* ********************************************************************** */

// class ByteSource [Denys Duchier]
// something to marshal from

class ByteSource {
public:
  virtual OZ_Return maybeSkipHeader();
  virtual OZ_Return getBytes(BYTE*,int&,int&);
  virtual OZ_Return getTerm(OZ_Term);
  virtual OZ_Return makeByteStream(ByteStream*&);
  virtual char*     emptyMsg();
};

class ByteSourceFD : public ByteSource {
private:
  int fd;
  Bool del;
public:
  ByteSourceFD(int i,Bool d=FALSE):fd(i),del(d){}
  virtual ~ByteSourceFD(){ if (del) close(fd); }
  char* emptyMsg();
  OZ_Return maybeSkipHeader();
  OZ_Return getBytes(BYTE*,int&,int&);
};

class ByteSourceDatum : public ByteSource {
private:
  OZ_Datum dat;
  Bool     del;
  int      idx;
public:
  ByteSourceDatum():del(FALSE),idx(0){}
  ByteSourceDatum(OZ_Datum d,Bool b):dat(d),del(b),idx(0){}
  virtual ~ByteSourceDatum() {
    if (del) free(dat.data);
  }
  char* emptyMsg();
  OZ_Return maybeSkipHeader();
  OZ_Return getBytes(BYTE*,int&,int&);
};


// class ByteSink [Denys Duchier]
// something to marshal into

class ByteSink {
public:
  virtual OZ_Return putTerm(OZ_Term,OZ_Term);
  virtual OZ_Return putBytes(BYTE*,int);
  virtual OZ_Return allocateBytes(int);
  virtual OZ_Return maybeSaveHeader(ByteStream*);
};

OZ_Return ByteSink::maybeSaveHeader(ByteStream*){
   Assert(0);
   return PROCEED;}

class ByteSinkFD : public ByteSink {
private:
  int fd;
  Bool del;
public:
  ByteSinkFD(int f,Bool b=FALSE):fd(f),del(b){}
  ByteSinkFD():fd(0),del(FALSE){}
  virtual ~ByteSinkFD() { if (del) close(fd); }
  OZ_Return allocateBytes(int);
  OZ_Return putBytes(BYTE*,int);
};

class ByteSinkFile : public ByteSink {
private:
  char* filename;
  int fd;
public:
  ByteSinkFile(char*s):filename(s),fd(-1){};
  ByteSinkFile():filename(0),fd(-1){};
  virtual ~ByteSinkFile() { if (fd>=0) close(fd); }
  OZ_Return allocateBytes(int);
  OZ_Return putBytes(BYTE*,int);
};

class ByteSinkDatum : public ByteSink {
private:
  int      idx;
public:
  OZ_Datum dat;
  ByteSinkDatum():idx(0){ dat.size=0; dat.data=0; }
  OZ_Return allocateBytes(int);
  OZ_Return putBytes(BYTE*,int);
  OZ_Return maybeSaveHeader(ByteStream*);
};


// ===================================================================
// class ByteSink
// ===================================================================

OZ_Return
ByteSink::putBytes(BYTE*pos,int n)
{
  Assert(0);
  return FAILED;
}

OZ_Return
ByteSink::allocateBytes(int n)
{
  Assert(0);
  return FAILED;
}

OZ_Return
ByteSink::putTerm(OZ_Term in, OZ_Term resources)
{
  ByteStream* bs=bufferManager->getByteStream();
  MarshalInfo mi;
  bs->setMarshalInfo(&mi);
  marshal_M_FILE(bs,PERDIOVERSION,in);
  bs->beginWrite();
  bs->incPosAfterWrite(tcpHeaderSize);

  int total=bs->calcTotLen();
  allocateBytes(total);
  while (total) {
    Assert(total>0);
    int len=bs->getWriteLen();
    BYTE* pos=bs->getWritePos();
    total -= len;
    OZ_Return result = putBytes(pos,len);
    if (result!=PROCEED) {
      bufferManager->freeByteStream(bs);
      return result;
    }
    bs->sentFirst();
  }
  bs->writeCheck();
  bufferManager->freeByteStream(bs);

  return OZ_unify(resources,mi.resources) ? PROCEED : FAILED;
}

// ===================================================================
// class ByteSinkFD
// ===================================================================

OZ_Return
ByteSinkFD::allocateBytes(int n) { return PROCEED; }

OZ_Return
ByteSinkFD::putBytes(BYTE*pos,int len)
{
  if (oswrite(fd,pos,len)<0)
    return oz_raise(E_ERROR,OZ_atom("perdio"),"save",3,
		    oz_atom("write"),
		    oz_atom(OZ_unixError(errno)),
		    oz_int(fd));
  return PROCEED;
}

// ===================================================================
// class ByteSinkFile
// ===================================================================

OZ_Return
ByteSinkFile::allocateBytes(int n)
{
  fd = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0666);
  if (fd < 0)
    return oz_raise(E_ERROR,OZ_atom("perdio"),"save",3,
		    oz_atom("open"),
		    oz_atom(OZ_unixError(errno)),
		    oz_atom(filename));
  return PROCEED;
}

OZ_Return
ByteSinkFile::putBytes(BYTE*pos,int len)
{
  if (oswrite(fd,pos,len)<0)
    return oz_raise(E_ERROR,OZ_atom("perdio"),"save",3,
		    oz_atom("write"),
		    oz_atom(OZ_unixError(errno)),
		    oz_atom(filename));
  return PROCEED;
}

// ===================================================================
// class ByteSinkDatum
// ===================================================================

OZ_Return
ByteSinkDatum::allocateBytes(int n)
{
  dat.size = n;
  dat.data = (char*) malloc(n);
  if (dat.data==0)
    return oz_raise(E_ERROR,OZ_atom("perdio"),"save",3,
		    oz_atom("malloc"),
		    oz_atom(OZ_unixError(errno)),
		    oz_atom("datum"));
  return PROCEED;
}

OZ_Return
ByteSinkDatum::putBytes(BYTE*pos,int len)
{
  memcpy(&(dat.data[idx]),pos,len);
  idx += len;
  return PROCEED;
}

OZ_Return
ByteSinkDatum::maybeSaveHeader(ByteStream*)
{
	// saveHeader(bs)
  return PROCEED;
}

OZ_Return saveFile(OZ_Term in,char *filename,OZ_Term resources)
{
  ByteSinkFile sink(filename);
  return sink.putTerm(in,resources);
}

OZ_Return
saveDatum(OZ_Term in,OZ_Datum& dat,OZ_Term resources)
{
  ByteSinkDatum sink;
  OZ_Return result = sink.putTerm(in,resources);
  if (result==PROCEED) dat=sink.dat;
  else {
    if (sink.dat.data!=0) free(sink.dat.data);
  }
  return result;
}

OZ_C_proc_begin(BIsmartSave,3)
{
  OZ_declareArg(0,in);
  OZ_declareArg(2,resources);
  OZ_declareVirtualStringArg(1,filename);

  return saveFile(in,filename,resources);
}
OZ_C_proc_end

// int loadURL(TaggedRef url, OZ_Term out, Thread *th)
// {
//   Literal *lit = tagged2Literal(url);
//   Assert(lit->isAtom());
//   const char *s=lit->getPrintName();
//   return loadURL(s,out,th);
// }


// ===================================================================
// class ByteSource
// ===================================================================

OZ_Return
ByteSource::maybeSkipHeader()
{
  Assert(0);
  return FAILED;
}

OZ_Return
ByteSource::getBytes(BYTE*pos,int&max,int&got)
{
  Assert(0);
  return FAILED;
}

char*
ByteSource::emptyMsg()
{
  Assert(0);
  return "emptyByteSource";
}

OZ_Return
ByteSource::getTerm(OZ_Term out)
{
  OZ_Return result = maybeSkipHeader();
  if (result!=PROCEED) return result;
  ByteStream * stream;
   
  result = makeByteStream(stream);
  if (result!=PROCEED) return result;
  // EK fast fix

  stream->beforeInterpret(0);
  stream->unmarshalBegin();	

  char *versiongot = 0;
  OZ_Term val;

  if(stream->skipHeader() && unmarshal_SPEC(stream,versiongot,val)){
    stream->afterInterpret();    
    bufferManager->freeByteStream(stream);
    delete versiongot;    
    return oz_unify(val,out);}    
      
  bufferManager->dumpByteStream(stream);
  if (versiongot) {
    OZ_Term vergot = oz_atom(versiongot);
    delete versiongot;
    return oz_raise(E_ERROR,OZ_atom("perdio"),"load",3,
		    oz_atom("versionMismatch"),
		    oz_atom(PERDIOVERSION),
		    vergot);
  } else {
    return oz_raise(E_ERROR,OZ_atom("perdio"),"load",1,
		    oz_atom("notComponent"));
  }
}


OZ_Return
ByteSource::makeByteStream(ByteStream*& stream)
{
  stream = bufferManager->getByteStream();
  stream->getSingle();
  int max,got;
  int total = 0;
  BYTE *pos = stream->initForRead(max);
  while (TRUE) {
    OZ_Return result = getBytes(pos,max,got);
    if (result!=PROCEED) return result;
    total += got;
    stream->afterRead(got);
    if (got<max) break;
    pos = stream->beginRead(max);
  }
  if (total==0)
    return oz_raise(E_ERROR,OZ_atom("perdio"),"load",1,
		    oz_atom(emptyMsg()));
  return PROCEED;
}

// ===================================================================
// class ByteSourceFD
// ===================================================================

char*
ByteSourceFD::emptyMsg() { return "emptyFile"; }

OZ_Return
ByteSourceFD::maybeSkipHeader() {
  
  return PROCEED;
}

OZ_Return
ByteSourceFD::getBytes(BYTE*pos,int&max,int&got)
{
loop:
  got = osread(fd,pos,max);
  if (got < 0) {
    if (errno==EINTR) goto loop;
    return oz_raise(E_ERROR,OZ_atom("perdio"),"load",2,
		    oz_atom("read"),
		    oz_atom(OZ_unixError(errno)));
  }
  return PROCEED;
}

// ===================================================================
// class ByteSourceDatum
// ===================================================================

char*
ByteSourceDatum::emptyMsg() { return "emptyDatum"; }

OZ_Return
ByteSourceDatum::maybeSkipHeader() { return PROCEED; }

OZ_Return
ByteSourceDatum::getBytes(BYTE*pos,int&max,int&got)
{
  if (idx >= dat.size) {
    got = 0;
    return PROCEED;
  }
  got = dat.size - idx;
  if (got >= max) {
    got = max;
  }
  memcpy(pos,&(dat.data[idx]),got);
  idx += got;
  return PROCEED;
}

OZ_Return loadDatum(OZ_Datum dat,OZ_Term out)
{
  ByteSourceDatum src(dat,TRUE);
  return src.getTerm(out);
}

OZ_Return loadFD(int fd, OZ_Term out)
{
  ByteSourceFD src(fd,TRUE);
  return src.getTerm(out);
}

OZ_Return loadFile(char *filename,OZ_Term out)
{
  int fd = strcmp(filename,"-")==0 ? STDIN_FILENO : open(filename,O_RDONLY);
  if (fd < 0) {
    return oz_raise(E_ERROR,OZ_atom("perdio"),"load",3,
		    oz_atom("open"),
		    oz_atom(OZ_unixError(errno)),
		    oz_atom(filename));
  }
  return loadFD(fd,out);
}


char *newTempFile()
{
  char tn[L_tmpnam] = ""; // I like POSIX!
  tmpnam(tn);
  return ozstrdup(tn);  
}

enum URLAction { URL_LOCALIZE , URL_OPEN , URL_LOAD };

class PipeInfo {
public:
  int fd;
  int pid;
  char *file;
  char *url;
  TaggedRef thread, out;
  URLAction action;

  PipeInfo(int f, int p, char *tmpf, const char *u, TaggedRef o, Thread *t,
	   URLAction act):
    fd(f), pid(p), file(tmpf), out(o), action(act)
  {
    Assert(t);
    thread = makeTaggedConst(t);
    url = ozstrdup(u);
    OZ_protect(&thread); 
    OZ_protect(&out); 
  }
  ~PipeInfo() { 
    OZ_unprotect(&thread); 
    OZ_unprotect(&out); 
    delete url;
  }
};

#define ACTION_STRING(act)	\
((act==URL_LOCALIZE)?"localize":\
 (act==URL_OPEN    )?"open":	\
 (act==URL_LOAD    )?"load":	\
 "<unknown action>")

void doRaise(Thread *th, char *msg, const char *url,URLAction act)
{
  threadRaise(th,OZ_makeException(E_SYSTEM,oz_atom("url"),
				  ACTION_STRING(act),2,
				  oz_atom(msg),
				  oz_atom(url)),1);
}

int pipeHandler(int, void *arg)
{
  PipeInfo *pi = (PipeInfo *) arg;
  char retloc = 0;
  int n = osread(pi->fd,&retloc,sizeof(retloc));
  osclose(pi->fd);
 
  Thread *th = pi->thread ? tagged2Thread(pi->thread) : 0;

#ifndef WINDOWS
  int u = waitpid(pi->pid,NULL,0);
  if (u!=pi->pid) {
    doRaise(th,OZ_unixError(errno),pi->url,pi->action);
    return NO;
  }
#endif

  if (retloc!=URLC_OK) {
    doRaise(th,urlcStrerror(retloc),pi->url,pi->action);
    goto exit;
  }

  switch (pi->action) {
  case URL_LOCALIZE:
    // this is only called when the file is remote, therefore
    // a local copy will be made into the file
    pushUnify(th,pi->out,OZ_mkTupleC("new",1,oz_atom(pi->file)));
    break;
  case URL_OPEN:
    {
      int fd = osopen(pi->file,O_RDONLY,0);
      if (fd < 0) {
	doRaise(th,OZ_unixError(errno),pi->url,pi->action);
	goto exit;
      }
      unlink(pi->file);
      pushUnify(th,pi->out,OZ_int(fd));
      break;
    }
  case URL_LOAD:
    {
      int fd = osopen(pi->file, O_RDONLY,0);
      if (fd < 0) {
	doRaise(th,OZ_unixError(errno),pi->url,pi->action);
	goto exit;
      }
      OZ_Term other = oz_newVariable();
      OZ_Return aux = loadFD(fd,other);
      if (aux==RAISE) {
	threadRaise(th,am.getExceptionValue(),1);
	goto exit;
      }
      Assert(aux==PROCEED);
      unlink(pi->file);
      pushUnify(th,pi->out,other);
      break;
    }
  }
  oz_resumeFromNet(th);

exit:
  delete pi->file;
  delete pi;
  return OK;
}

#ifdef WINDOWS

class URLInfo {
public:
  char *tmpfile, *url;
  int fd;
  URLInfo(char *file, char *u, int f):
    tmpfile(ozstrdup(file)), url(ozstrdup(u)), fd(f) {}
  ~URLInfo() {
    delete tmpfile;
    delete url;
  }
};

unsigned __stdcall fetchThread(void *p)
{
  URLInfo *ui = (URLInfo *) p;
  int ret = localizeUrl(ui->url,ui->tmpfile);
  // message("fetchthread(%s,%s)=%d,fd=%d\n",ui->url,ui->tmpfile,ret,ui->fd);
  char buf[2];
  buf[0] = ret;
  buf[1] = '\n';
  oswrite(ui->fd,buf,2);
  osclose(ui->fd);
  delete ui;
  _endthreadex(1);
  return 1;
}

#endif



void getURL(const char *url, TaggedRef out, URLAction act, Thread *th)
{
  char *tmpfile = newTempFile();

#ifdef WINDOWS

  HANDLE rh,wh;
  CreatePipe(&rh,&wh,0,0);
  int wfd = _hdopen((int)wh,O_WRONLY|O_BINARY);
  int rfd = _hdopen((int)rh,O_RDONLY|O_BINARY);

  URLInfo *ui = new URLInfo(tmpfile,url,wfd);

  unsigned tid;
  HANDLE thrd = (HANDLE) _beginthreadex(NULL,0,&fetchThread,ui,0,&tid);
  if (thrd==NULL) {
    ozpwarning("getURL: start thread");
    return;
  }

  int pid = 0;

#else

  int fds[2];
  if (pipe(fds)<0) {
    perror("pipe");
    return;
  }

  pid_t pid = fork();
  switch(pid) {
  case 0: /* child */
    {
      osclose(fds[0]);
      char ret = (char) localizeUrl(url,tmpfile);
      oswrite(fds[1],&ret,sizeof(ret));
      exit(0);
    }
  case -1:
    perror("fork");
    return;
  default:
    break;
  }

  osclose(fds[1]);

  int rfd = fds[0];
#endif

  PipeInfo *pi = new PipeInfo(rfd,pid,tmpfile,url,out,th,act);
  oz_suspendOnNet(th);
  OZ_registerReadHandler(rfd,pipeHandler,pi);
}



// OZ_C_proc_begin(BIgetCachePath,1)
// {
//   return OZ_unify(OZ_getCArg(0),(OZ_Cache_Path==0)?OZ_unit():OZ_Cache_Path);
// }
// OZ_C_proc_end
// 
// OZ_C_proc_begin(BIsetCachePath,1)
// {
//   OZ_declareNonvarArg(0,cache);
//   if (!OZ_onToplevel())
//     return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("setCachePath"));
//   if (!OZ_isTuple(cache))
//     return OZ_typeError(0,"Tuple");
//   for(int i=OZ_width(cache);i>0;i--)
//     if (!OZ_isVirtualString(OZ_getArg(cache,i-1),0))
//       return OZ_typeError(0,"TupleOfVirtualStrings");
//   OZ_Cache_Path = cache;
//   return PROCEED;
// }
// OZ_C_proc_end

// static void
// init_cache_path()
// {
//   if (OZ_Cache_Path!=0) return;
//   extern int env_to_tuple(char*,OZ_Term*);
//   if (env_to_tuple("OZ_CACHE_PATH",&OZ_Cache_Path)==0) return;
// #define NAMESIZE 256
//   char buffer[NAMESIZE];
//   strcpy(buffer,ozconf.ozHome);
//   strcpy(buffer+strlen(ozconf.ozHome),"/cache");
//   OZ_Cache_Path = OZ_mkTuple(OZ_atom("cache"),1,OZ_atom(buffer));
// }
    
// int loadURL(const char *url0, OZ_Term out, Thread *th)
// {
//   if (ozconf.showLoad)
//     message("Loading %s\n",url0);
//   // we need to locally copy the url arg because it may point
//   // to the static area used the ...ToC interface.
// 
//   char urlbuf[NAMESIZE];
//   if (strlen(url0)>=NAMESIZE)
//     return OZ_raiseC("loadURL",2,OZ_atom("bufferOverflow"),
// 		     OZ_atom(url0));
//   strcpy(urlbuf,url0);
//   char* url = urlbuf;
// 
//   // perform translation through url_map:
//   // note that we leave currentURL untranslated in order to
//   // record the original symbolic dependency.  Only url is
//   // translated to obtain the actual location.
// 
//   if (url_map!=0) {
//     OZ_Term oldURL=oz_atom(url);
//     OZ_Term newURL;
//     int notTooMany = 100;
//     while ((newURL=OZ_subtree(url_map,oldURL))) {
//       if (!OZ_isAtom(newURL))
// 	return OZ_raiseC("loadURL",2,OZ_atom("badUrlInMap"),newURL);
//       oldURL=newURL;
//       if (!(notTooMany--))
// 	return OZ_raiseC("loadURL",1,OZ_atom("tooManyRemaps"));
//     }
//     const char *urlin = OZ_atomToC(oldURL);
//     if (strlen(urlin)>=NAMESIZE)
//       return OZ_raiseC("loadURL",2,OZ_atom("bufferOverflow"),oldURL);
//     strcpy(url,urlin);
//   }
// 
//   if (strchr(url,':')==NULL) { // no prefix --> local file name
//     return loadFile(url,out);
//   }
// 
//   // check local caches
//   if (OZ_Cache_Path==0) init_cache_path();
//   {
//     char buffer[NAMESIZE];
//     int idx = 0;
//     char *s = url;
//     if (strlen(s)>=NAMESIZE) goto fall_through;
//     while (*s!='\0' && *s!=':') buffer[idx++]=*s++;
//     if (s[0]!=':' || s[1]!='/' || s[2]!='/') goto fall_through;
//     s += 3;
//     buffer[idx++] = '/';
//     strcpy(buffer+idx,s);
//     extern int find_file(OZ_Term,char*,char*);
//     char path[NAMESIZE];
//     if (find_file(OZ_Cache_Path,buffer,path)==0) {
//       if (ozconf.showCacheLoad)
// 	message("Loading %s\n*** from cache %s\n",url,path);
//       return loadFile(path,out);
//     }
//   fall_through:;
//   }
// 
//   switch (url[0]) {
//   case 'f':
//     {
//       const char *prefix = "file:";
//       if (strncmp(url,prefix,strlen(prefix))!=0) goto bomb;
// 
//       char *filename = url+strlen(prefix);
//       return loadFile(filename,out);
//     }
//   }
// 
// bomb:
//   getURL(url,out,URL_LOAD,th);
//   return BI_PREEMPT;
// }

// URL_get is a primitive that performs an action on a url.
// the action maybe URL_LOCALIZE, URL_OPEN, or URL_LOAD.
// In the case or URL_LOCALIZE, the return value is either:
//	old(PATH)
//		if PATH is the pathname of the original
//		local file
//	new(PATH)
//		if PATH is the pathname of a new local copy
//		of the remote file

#include <ctype.h>
#include <unistd.h>

OZ_Return URL_get(const char*url,OZ_Term out,URLAction act)
{
#ifdef WINDOWS
      // check for WINDOWS style absolute pathname
  if (isalpha(url[0]) && url[1]==':' && (url[2]=='/' || url[2]=='\\')) {
    goto url_local;
  }
#endif
  if (strncmp(url,"file:/",6)==0) { url+=5; goto url_local; }
  {
    const char*s=url;
    while (isalnum(*s)) s++;
    if (*s==':') goto url_remote;
  }
url_local:
  switch (act) {
  case URL_LOCALIZE:
    {
      if (access(url,F_OK)<0) goto kaboom;
      return OZ_unify(out,OZ_mkTupleC("old",1,oz_atom(url)));
    }
  case URL_OPEN:
    {
      int fd = osopen(url,O_RDONLY,0);
      if (fd<0) goto kaboom;
      return OZ_unify(out,OZ_int(fd));
    }
  case URL_LOAD:
    {
      int fd = osopen(url,O_RDONLY,0);
      if (fd<0) goto kaboom;
      OZ_Term   val    = oz_newVariable();
      OZ_Return status = loadFD(fd,val);
      return (status==PROCEED)?OZ_unify(out,val):status;
    }
  default:
    Assert(0);
    return FAILED;
  }
url_remote:
  getURL(url,out,act,am.currentThread());
  return BI_PREEMPT;
kaboom:
  return oz_raise(E_SYSTEM,oz_atom("url"),ACTION_STRING(act),2,
		  oz_atom(OZ_unixError(errno)),
		  oz_atom(url));
}

OZ_C_proc_begin(BIurl_localize,2)
{
  OZ_declareVirtualStringArg(0,url);
  OZ_declareArg(1,out);
  return URL_get(url,out,URL_LOCALIZE);
}
OZ_C_proc_end

OZ_C_proc_begin(BIurl_open,2)
{
  OZ_declareVirtualStringArg(0,url);
  OZ_declareArg(1,out);
  return URL_get(url,out,URL_OPEN);
}
OZ_C_proc_end

OZ_C_proc_begin(BIurl_load,2)
{
  OZ_declareVirtualStringArg(0,url);
  OZ_declareArg(1,out);
  return URL_get(url,out,URL_LOAD);
}
OZ_C_proc_end

OZ_C_proc_begin(BIload,2)
{
  RefsArray args = allocateRefsArray(2);
  OZ_Term loader = registry_get(AtomLoad);
  args[0] = OZ_getCArg(0);
  args[1] = OZ_getCArg(1);
  Thread*tt=am.currentThread();
  if (loader) tt->pushCall(loader,args,2);
  else        tt->pushCFun(BIurl_load,args,2,OK);
  disposeRefsArray(args);
  return BI_REPLACEBICALL;
}
OZ_C_proc_end


// OZ_C_proc_begin(BIload,2)
// {
//   OZ_declareVirtualStringArg(0,url);
//   OZ_declareArg(1,out);
// 
//   return loadURL(url,out,am.currentThread());
// }
// OZ_C_proc_end

OZ_C_proc_begin(BIWget,2)
{
  OZ_declareVirtualStringArg(0,url);
  OZ_declareArg(1,out);

  getURL(url,out,URL_LOCALIZE,am.currentThread());

  return BI_PREEMPT;
}
OZ_C_proc_end


OZ_Return OZ_valueToDatum(OZ_Term t, OZ_Datum* d)
{
  return saveDatum(t,*d,OZ_nil());
}


OZ_Return OZ_datumToValue(OZ_Datum d,OZ_Term t)
{
  return loadDatum(d,t);
}

OZ_Term gatePort=0;

OZ_C_proc_begin(BIGateId,1)
{
  oz_declareArg(0,out);

  // ozgate://hostname:port/timestamp";
  static char url[100];
  ip_address a = mySite->getAddress();
  short unsigned int port = mySite->getPort();
  time_t stamp = mySite->getTimeStamp();

  // mm2: use inet_ntoa seems to be a better abstraction
  sprintf(url,"ozgate://%lu.%lu.%lu.%lu:%u/",
	  (a/(256*256*256))%256,
	  (a/(256*256))%256,
	  (a/256)%256,
	  a%256,
	  port
	  );
  char *s=url+strlen(url);
  while (stamp) {
    *s++ = 'a'+(stamp % 26);
    stamp = stamp/26;
  }
  *s++ = 0;
  return OZ_unifyAtom(out,url);
}
OZ_C_proc_end

OZ_C_proc_begin(BIOpenGate,1)
{
  oz_declareArg(0,stream);

  if (gatePort) return oz_raise(E_ERROR,E_SYSTEM,"gateAlreadyOpen",0);

  gatePort = oz_newPort(stream);
  OZ_protect(&gatePort);

  return PROCEED;
}
OZ_C_proc_end

OZ_C_proc_begin(BICloseGate,0)
{
  if (gatePort) {
    OZ_unprotect(&gatePort);
    gatePort = 0;
  }
  return PROCEED;
}
OZ_C_proc_end

extern int sendPort(OZ_Term port, OZ_Term val);

void sendGate(OZ_Term t) {
  if (gatePort) {
    sendPort(gatePort,t);
  }
}

OZ_C_proc_begin(BISendGate,2)
{
  oz_declareVirtualStringArg(0,url);
  oz_declareArg(1,val);

  if (strncmp(url,"ozgate://",9)!=0)
    goto bomb;
  url += 9;

  char *p;
  p = strchr(url,':');
  if (!p) goto bomb;
  *p = 0;

  struct hostent *hostaddr;
  hostaddr = gethostbyname(url);
  struct in_addr tmp;
  memcpy(&tmp,hostaddr->h_addr_list[0],sizeof(in_addr));
  ip_address addr;
  addr = ntohl(tmp.s_addr);
  url = p+1;

  int port;
  port = strtol(url,&p,10);
  if (*p!='/') goto bomb;
  url = p+1;

  time_t stamp;
  stamp=0;
  unsigned char *s;
  s = (unsigned char *)(url+strlen(url));
  while (s>(unsigned char *)url) {
    int c = *(--s)-'a';
    if (c<0||c>25) goto bomb;
    stamp = stamp*26+c;
  }

  Site *site;
  site = findSite(addr,port,stamp);

  if (!site) goto bomb;

  MsgBuffer *bs;
  bs = msgBufferManager->getMsgBuffer(site);
  marshal_M_SEND_GATE(bs,val);
  int ret;
  ret = site->sendTo(bs,M_SEND_GATE,0,0);
  // ignore ret;
  return PROCEED;

bomb:
  return oz_raise(E_ERROR,E_SYSTEM,"sendGate",2,OZ_getCArg(0),val);
}
OZ_C_proc_end



BIspec componentsSpec[] = {
  {"smartSave",    3, BIsmartSave, 0},
  {"load",         2, BIload, 0},

  //  {"getCachePath",1,BIgetCachePath,0},
  //  {"setCachePath",1,BIsetCachePath,0},

  {"Wget",         2, BIWget, 0},    

  {"GateId",       1, BIGateId},
  {"OpenGate",     1, BIOpenGate},
  {"CloseGate",    0, BICloseGate},
  {"SendGate",     2, BISendGate},

  {"Gate.id",       1, BIGateId},
  {"Gate.open",     1, BIOpenGate},
  {"Gate.close",    0, BICloseGate},
  {"Gate.send",     2, BISendGate},

  {"URL.localize", 2, BIurl_localize},
  {"URL.open",     2, BIurl_open},
  {"URL.load",     2, BIurl_load},
  {0,0,0,0}
};

void initComponents() {
  //  OZ_protect(&url_map);
  //  OZ_protect(&OZ_Cache_Path);
  BIaddSpec(componentsSpec);
}



