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


#include "wsock.hh"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
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
#define tcpHeaderSize   7


#include "componentBuffer.cc"


static OZ_Term url_map=0;
static OZ_Term OZ_Cache_Path = 0;

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
  virtual OZ_Return getTerm(OZ_Term,OZ_Term);
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
  virtual OZ_Return putTerm(OZ_Term,OZ_Term,OZ_Term,OZ_Term,OZ_Term);
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
ByteSink::putTerm(OZ_Term in,
                  OZ_Term url,
                  OZ_Term dosave,
                  OZ_Term urls,
                  OZ_Term resources)
{
  ByteStream* bs=bufferManager->getByteStream();
  MarshalInfo mi(dosave,urls,url);
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

  if (!literalEq(deref(urls),NameUnit) && !OZ_unify(urls,mi.urlsFound))
    return FAILED;

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

OZ_Return saveFile(OZ_Term in,char *filename,OZ_Term url,
                   OZ_Term dosave, OZ_Term urls,
                   OZ_Term resources)
{
  ByteSinkFile sink(filename);
  return sink.putTerm(in,url,dosave,urls,resources);
}

OZ_Return
saveDatum(OZ_Term in,OZ_Datum& dat,OZ_Term url,
          OZ_Term dosave, OZ_Term urls,
          OZ_Term resources)
{
  ByteSinkDatum sink;
  OZ_Return result = sink.putTerm(in,url,dosave,urls,resources);
  if (result==PROCEED) dat=sink.dat;
  else {
    if (sink.dat.data!=0) free(sink.dat.data);
  }
  return result;
}

OZ_C_proc_begin(BIsmartSave,6)
{
  if(!perdioInit()) return FAILED;
  OZ_declareArg(0,in);
  OZ_declareNonvarArg(2,urlSave); urlSave = deref(urlSave);
  OZ_declareNonvarArg(3,dosave);
  OZ_declareArg(4,urls);
  OZ_declareArg(5,resources);

  OZ_Term url;
  if (literalEq(urlSave,NameUnit)) {
    url = urlSave;
  } else {
    OZ_declareVirtualStringArg(2,urlSaveAux);
    url=OZ_atom(urlSaveAux);
  }

  OZ_declareVirtualStringArg(1,filename);

  return saveFile(in,filename,url,dosave,urls,resources);
}
OZ_C_proc_end

int loadURL(TaggedRef url, OZ_Term out, OZ_Term triggerVar, Thread *th)
{
  Literal *lit = tagged2Literal(url);
  Assert(lit->isAtom());
  const char *s=lit->getPrintName();
  return loadURL(s,out,triggerVar,th);
}


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
ByteSource::getTerm(OZ_Term out,OZ_Term triggerVar)
{
  OZ_Return result = maybeSkipHeader();
  if (result!=PROCEED) return result;
  ByteStream * stream;

  result = makeByteStream(stream);
  if (result!=PROCEED) return result;
  // EK fast fix

  stream->beforeInterpret(0);
  stream->unmarshalBegin();

  char *versiongot = ozstrdup("unknown");
  OZ_Term val;

  if(stream->skipHeader() && unmarshal_SPEC(stream,versiongot,val)){
    stream->afterInterpret();
    bufferManager->freeByteStream(stream);
    delete versiongot;
    if (triggerVar && OZ_isVariable(triggerVar)) {
      return oz_raise(E_ERROR,OZ_atom("perdio"),"load",2,
                      oz_atom("novaluefound"),triggerVar);
    }
    return oz_unify(val,out);}

  bufferManager->dumpByteStream(stream);
  OZ_Term vergot = oz_atom(versiongot);
  delete versiongot;
  return oz_raise(E_ERROR,OZ_atom("perdio"),"load",3,
                  oz_atom("versionMismatch"),
                  oz_atom(PERDIOVERSION),
                  vergot);
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

OZ_Return loadDatum(OZ_Datum dat,OZ_Term out,OZ_Term triggerVar)
{
  ByteSourceDatum src(dat,TRUE);
  return src.getTerm(out,triggerVar);
}

OZ_Return loadFD(int fd, OZ_Term out,OZ_Term triggerVar)
{
  ByteSourceFD src(fd,TRUE);
  return src.getTerm(out,triggerVar);
}

OZ_Return loadFile(char *filename,OZ_Term out,OZ_Term triggerVar)
{
  int fd = strcmp(filename,"-")==0 ? STDIN_FILENO : open(filename,O_RDONLY);
  if (fd < 0) {
    return oz_raise(E_ERROR,OZ_atom("perdio"),"load",3,
                    oz_atom("open"),
                    oz_atom(OZ_unixError(errno)),
                    oz_atom(filename));
  }
  return loadFD(fd,out,triggerVar);
}

// -------------------------------------------------------------------
// URL Map Interface - Denys Duchier
//
// The idea is that there should be a record that maps urls to urls.
// {GetURLMap Map}
// {SetURLMap Map}
//
// loadURL effects this remapping before doing its actual job
// -------------------------------------------------------------------



OZ_C_proc_begin(BIperdioGetURLMap,1)
{
  if(!perdioInit()) return FAILED;
  return OZ_unify(OZ_getCArg(0),(url_map==0)?OZ_unit():url_map);
}
OZ_C_proc_end

OZ_C_proc_begin(BIperdioSetURLMap,1)
{
  if(!perdioInit()) return FAILED;
  OZ_Term map = OZ_getCArg(0);
  if (!OZ_onToplevel())
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("setURLMap"));
  if (!OZ_isRecord(map))
    return OZ_typeError(0,"Record");
  url_map = map;
  return PROCEED;
}
OZ_C_proc_end

char *newTempFile()
{
  char tn[L_tmpnam] = ""; // I like POSIX!
  tmpnam(tn);
  return ozstrdup(tn);
}


class PipeInfo {
public:
  int fd;
  int pid;
  char *file;
  char *url;
  TaggedRef thread, out,trigger;
  Bool load;

  PipeInfo(int f, int p, char *tmpf, char *u, TaggedRef o, Thread *t, TaggedRef var, Bool ld):
    fd(f), pid(p), file(tmpf), out(o), load(ld)
  {
    Assert(t);
    thread = makeTaggedConst(t);
    url = ozstrdup(u);
    OZ_protect(&thread);
    OZ_protect(&out);
    trigger=var;
    OZ_protect(&trigger);
  }
  ~PipeInfo() {
    OZ_unprotect(&thread);
    OZ_unprotect(&trigger);
    OZ_unprotect(&out);
    delete url;
  }
};


void doRaise(Thread *th, char *msg, char *url)
{
  threadRaise(th,
              OZ_mkTuple(E_ERROR,
                         1,
                         OZ_mkTupleC("perdio",
                                     3,
                                     oz_atom("load"),
                                     oz_atom(msg),
                                     oz_atom(url))));
}

int pipeHandler(int, PipeInfo *pi)
{
  char buf[2];
  int n = osread(pi->fd,buf,2);
  osclose(pi->fd);
  int retloc = buf[0];

  Thread *th = pi->thread ? tagged2Thread(pi->thread) : 0;

#ifndef WINDOWS
  int u = waitpid(pi->pid,NULL,0);
  if (u!=pi->pid) {
      doRaise(th,OZ_unixError(errno),pi->url);
    return NO;
  }
#endif

  if (retloc!=URLC_OK) {
    doRaise(th,urlcStrerror(retloc),pi->url);
    goto exit;
  }

  {
    OZ_Term other = oz_atom(pi->file);
    if (pi->load) {
      int fd = osopen(pi->file, O_RDONLY,0);
      if (fd < 0) {
        doRaise(th,OZ_unixError(errno),pi->url);
        goto exit;
      }

      other = oz_newVariable();
      OZ_Return aux = loadFD(fd,other,pi->trigger);
      if (aux==RAISE) {
        threadRaise(th, am.getExceptionValue());
        goto exit;
      }
      unlink(pi->file);
    }
    pushUnify(th,pi->out,other);
    oz_resumeFromNet(th);
  }

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



void getURL(char *url, TaggedRef out, TaggedRef trigger,Bool load, Thread *th)
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
      int ret = localizeUrl(url,tmpfile);
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

  PipeInfo *pi = new PipeInfo(rfd,pid,tmpfile,url,out,th,trigger,load);
  oz_suspendOnNet(th);
  OZ_registerReadHandler(rfd,pipeHandler,pi);
}



OZ_C_proc_begin(BIgetCachePath,1)
{
  if(!perdioInit()) return FAILED;
  return OZ_unify(OZ_getCArg(0),(OZ_Cache_Path==0)?OZ_unit():OZ_Cache_Path);
}
OZ_C_proc_end

OZ_C_proc_begin(BIsetCachePath,1)
{
  if(!perdioInit()) return FAILED;
  OZ_declareNonvarArg(0,cache);
  if (!OZ_onToplevel())
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("setCachePath"));
  if (!OZ_isTuple(cache))
    return OZ_typeError(0,"Tuple");
  for(int i=OZ_width(cache);i>0;i--)
    if (!OZ_isVirtualString(OZ_getArg(cache,i-1),0))
      return OZ_typeError(0,"TupleOfVirtualStrings");
  OZ_Cache_Path = cache;
  return PROCEED;
}
OZ_C_proc_end

static void
init_cache_path()
{
  if (OZ_Cache_Path!=0) return;
  extern int env_to_tuple(char*,OZ_Term*);
  if (env_to_tuple("OZ_CACHE_PATH",&OZ_Cache_Path)==0) return;
#define NAMESIZE 256
  char buffer[NAMESIZE];
  strcpy(buffer,ozconf.ozHome);
  strcpy(buffer+strlen(ozconf.ozHome),"/cache");
  OZ_Cache_Path = OZ_mkTuple(OZ_atom("cache"),1,OZ_atom(buffer));
}

int loadURL(const char *url0, OZ_Term out, OZ_Term triggerVar, Thread *th)
{
  if (ozconf.showLoad)
    message("Loading %s\n",url0);
  // we need to locally copy the url arg because it may point
  // to the static area used the ...ToC interface.

  char urlbuf[NAMESIZE];
  if (strlen(url0)>=NAMESIZE)
    return OZ_raiseC("loadURL",2,OZ_atom("bufferOverflow"),
                     OZ_atom(url0));
  strcpy(urlbuf,url0);
  char* url = urlbuf;

  // perform translation through url_map:
  // note that we leave currentURL untranslated in order to
  // record the original symbolic dependency.  Only url is
  // translated to obtain the actual location.

  currentURL=oz_atom(url);
  if (url_map!=0) {
    OZ_Term oldURL=currentURL;
    OZ_Term newURL;
    int notTooMany = 100;
    while ((newURL=OZ_subtree(url_map,oldURL))) {
      if (!OZ_isAtom(newURL))
        return OZ_raiseC("loadURL",2,OZ_atom("badUrlInMap"),newURL);
      oldURL=newURL;
      if (!(notTooMany--))
        return OZ_raiseC("loadURL",1,OZ_atom("tooManyRemaps"));
    }
    const char *urlin = OZ_atomToC(oldURL);
    if (strlen(urlin)>=NAMESIZE)
      return OZ_raiseC("loadURL",2,OZ_atom("bufferOverflow"),oldURL);
    strcpy(url,urlin);
  }

  if (strchr(url,':')==NULL) { // no prefix --> local file name
    //currentURL = oz_atom(url);
    return loadFile(url,out,triggerVar);
  }

  // check local caches
  if (OZ_Cache_Path==0) init_cache_path();
  {
    char buffer[NAMESIZE];
    int idx = 0;
    char *s = url;
    if (strlen(s)>=NAMESIZE) goto fall_through;
    while (*s!='\0' && *s!=':') buffer[idx++]=*s++;
    if (s[0]!=':' || s[1]!='/' || s[2]!='/') goto fall_through;
    s += 3;
    buffer[idx++] = '/';
    strcpy(buffer+idx,s);
    extern int find_file(OZ_Term,char*,char*);
    char path[NAMESIZE];
    if (find_file(OZ_Cache_Path,buffer,path)==0) {
      if (ozconf.showCacheLoad)
        message("Loading %s\n*** from cache %s\n",url,path);
      return loadFile(path,out,triggerVar);
    }
  fall_through:;
  }

  switch (url[0]) {
  case 'f':
    {
      const char *prefix = "file:";
      if (strncmp(url,prefix,strlen(prefix))!=0) goto bomb;

      //currentURL = oz_atom(url);
      char *filename = url+strlen(prefix);
      return loadFile(filename,out,triggerVar);
    }
  case 'o':
    {
      int OTI;
      if (strncmp(url,"ozp:",4)!=0) goto bomb;
      url+=4;
      char* rem;
      Site *s=stringToSite(url,rem);
      if(s==NULL) {
        Assert(0);   // ATTENTION
        goto bomb;}
      if(sscanf(rem,"%d",&OTI)!=1){
        Assert(0);   // ATTENTION
        goto bomb;}
      return oz_unify(out,makeBorrowRef(s,OTI));
    }
  }

bomb:
  getURL(url,out,triggerVar,OK,th);
  return BI_PREEMPT;
}


OZ_C_proc_begin(BIload,2)
{
  if(!perdioInit()) return FAILED;
  OZ_declareVirtualStringArg(0,url);
  OZ_declareArg(1,out);

  return loadURL(url,out,makeTaggedNULL(),am.currentThread());
}
OZ_C_proc_end


OZ_C_proc_begin(BIWget,2)
{
  if(!perdioInit()) return FAILED;
  OZ_declareVirtualStringArg(0,url);
  OZ_declareArg(1,out);

  getURL(url,out,NO,makeTaggedNULL(), am.currentThread());

  return BI_PREEMPT;
}
OZ_C_proc_end


OZ_Return OZ_valueToDatum(OZ_Term t, OZ_Datum* d)
{
  return saveDatum(t,*d,OZ_unit(),OZ_nil(),OZ_unit(),OZ_nil());
}


OZ_Return OZ_datumToValue(OZ_Datum d,OZ_Term t)
{
  return loadDatum(d,t,0);
}


OZ_C_proc_begin(BInewGate,2)
{
  OZ_declareArg(0,in);
  OZ_declareArg(1,out);

  int OTI=makeOwnerRef(in);
  static char url[100];
  sprintf(url,"ozp:%s %d",mySite->toString(),OTI);
  return oz_unifyAtom(out,url);
}
OZ_C_proc_end



BIspec componentsSpec[] = {
  {"smartSave",    6, BIsmartSave, 0},
  {"load",         2, BIload, 0},
  {"newGate",      2, BInewGate, 0},

  {"getURLMap",1,BIperdioGetURLMap,0},
  {"setURLMap",1,BIperdioSetURLMap,0},

  {"getCachePath",1,BIgetCachePath,0},
  {"setCachePath",1,BIsetCachePath,0},

  {"Wget",         2, BIWget, 0},

  {0,0,0,0}
};

void initComponents(){
  OZ_protect(&url_map);
  OZ_protect(&OZ_Cache_Path);
  BIaddSpec(componentsSpec);}
