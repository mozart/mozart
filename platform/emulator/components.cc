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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "conf.h"
#include "wsock.hh"
#include "codearea.hh"
#include "indexing.hh"
#include "var_base.hh"
#include "controlvar.hh"
#include "gc.hh"
#include "dictionary.hh"
#include "genhashtbl.hh"
#include "urlc.hh"
#include "marshaler.hh"
#include "site.hh"
#include "msgbuffer.hh"
#include "builtins.hh"
#include "os.hh"
#include "var_simple.hh"

#ifndef WINDOWS
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#endif
#ifndef __MINGW32__
#include <netdb.h>
#endif

#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "newmarshaler.hh"

#include "zlib.h"


// ATTENTION
// kost@: PER-LOOK what it has to do here?!!
#define tcpHeaderSize   7

static int checkzlibversion()
{
  if (strcmp(ZLIB_VERSION,zlibVersion()) != 0) {
    OZ_error("version mismatch in zlib version: header (%s) != library(%s)\n",
             ZLIB_VERSION,zlibVersion());
  }

  if (strncmp(ZLIB_VERSION,"1.1",3) != 0) {
    OZ_error("zlib version 1.1.x needed: got %s\n",ZLIB_VERSION);
  }

  return 1;
}

static int zlibDummy = checkzlibversion();



#include "componentBuffer.cc"

/* ********************************************************************** */
/*              BUILTINS                                                  */
/* ********************************************************************** */

// class ByteSource [Denys Duchier]
// something to marshal from

class ByteSource {
public:
  virtual OZ_Return getBytes(BYTE*,int&,int&) = 0;
  virtual char *getHeader() = 0;
  virtual Bool checkChecksum(crc_t) = 0;
  OZ_Return getTerm(OZ_Term, const char *compname,Bool);
  OZ_Return makeByteStream(ByteStream*&, const char*);
};

class ByteSourceFD : public ByteSource {
private:
  gzFile fd;
  crc_t checkSum;
  char *header;
public:
  char *getHeader() {return header; }
  virtual ~ByteSourceFD() { free(header); gzclose(fd); }
  OZ_Return getBytes(BYTE*,int&,int&);

  virtual Bool checkChecksum(crc_t i) { return checkSum==i; }

  ByteSourceFD(int i)
  {
    int bufsz = 10;
    char *buf = (char *) malloc(bufsz);
    int j = 0;
    int sysheaderread = 0;
    while(1) {
      if (j>=bufsz) {
        bufsz *= 2;
        buf = (char *) realloc(buf,bufsz);
      }

      if (osread(i,&buf[j],1)<=0)
        break;

      /* check whether we got syslet header 3 times in sequence */
      if (buf[j]==SYSLETHEADER) {
        if (++sysheaderread==3) {
          j -= 2;
          break;
        }
      } else {
        sysheaderread = 0;
      }

      j++;
    }

    buf[j] = 0;
    header = ozstrdup(buf);
    free(buf);
    checkSum = 0;

    for (int k=0; k<sizeof(crc_t); k++) {
      unsigned char c = 0;
      osread(i,&c,1);
      checkSum |=  c<<(k*8);
    }

    fd = gzdopen(i,"rb");
  }
};


class ByteSourceDatum : public ByteSource {
private:
  OZ_Datum dat;
  int      idx;
public:
  char *getHeader() {return ""; }
  ByteSourceDatum(OZ_Datum d):dat(d),idx(0) {}
  virtual ~ByteSourceDatum() { free(dat.data); }
  OZ_Return getBytes(BYTE*,int&,int&);
  virtual Bool checkChecksum(crc_t i) { return OK; }
};


// class ByteSink [Denys Duchier]
// something to marshal into

class ByteSink {
public:
  OZ_Return putTerm(OZ_Term,char*,char*,Bool text);
  virtual OZ_Return putBytes(BYTE*,int) = 0;
  virtual OZ_Return allocateBytes(int,char*,crc_t,Bool) = 0;
};


class ByteSinkFile : public ByteSink {
private:
  int fd;
  gzFile zfd;
  char *filename;
  int compressionlevel;
public:
  ByteSinkFile(char *s,int lvl): filename(s),fd(-1),zfd(0), compressionlevel(lvl) {}
  virtual ~ByteSinkFile() {
    if (zfd!=0) { gzclose(zfd); return; }
    if (fd!=-1) close(fd);
  }
  OZ_Return allocateBytes(int,char*,crc_t,Bool);
  OZ_Return putBytes(BYTE*,int);
};

class ByteSinkDatum : public ByteSink {
private:
  int      idx;
public:
  OZ_Datum dat;
  ByteSinkDatum():idx(0){ dat.size=0; dat.data=0; }
  OZ_Return allocateBytes(int,char*,crc_t,Bool);
  OZ_Return putBytes(BYTE*,int);
};


// ===================================================================
// class ByteSink
// ===================================================================

OZ_Term makeGenericExc(char *id,char *msg, OZ_Term arg)
{
  return OZ_makeException(E_ERROR,OZ_atom("dp"),"generic",
                          3,oz_atom(id),oz_atom(msg),arg);
}

OZ_Return raiseGeneric(char *id, char *msg, OZ_Term arg)
{
  return OZ_raise(makeGenericExc(id,msg,arg));
}


Bool newMarshaler = NO;

void marshalTermRT0(OZ_Term t, MsgBuffer *bs)
{
  if (newMarshaler)
    newMarshalTerm(t,bs);
  else
    marshalTermRT(t,bs);
}


void saveTerm(ByteStream* buf,TaggedRef t) {
  buf->marshalBegin();
  char *version  =  newMarshaler ? "2#0" : PERDIOVERSION;
  marshalString(version, buf);
  marshalTermRT0(t, buf);
  buf->marshalEnd();
  return;
}

OZ_Return onlyFutures(OZ_Term l) {
  if (oz_isNil(l)) return PROCEED;
  while (oz_isCons(l)) {
    OZ_Term f=oz_head(l);
    if (!oz_isFuture(oz_deref(f))) {
      am.emptySuspendVarList();
      return PROCEED;
    }
    am.addSuspendVarList(f);
    l = oz_tail(l);
  }
  return SUSPEND;
}

OZ_Return
ByteSink::putTerm(OZ_Term in, char *filename, char *header, Bool textmode)
{
  ByteStream* bs=bufferManager->getByteStream();
  if (textmode)
    bs->setTextmode();
  saveTerm(bs,in);

  OZ_Return ret=onlyFutures(bs->getResources());
  if (ret != PROCEED) {
    bufferManager->dumpByteStream(bs);
    return ret;
  }

  CheckNogoods(in,bs,"save:nogoods","Non-exportables found during save",
               bufferManager->dumpByteStream(bs));

  bs->beginWrite();
  bs->incPosAfterWrite(tcpHeaderSize);

  int total=bs->calcTotLen();
  allocateBytes(total,header,bs->crc(),textmode);

  while (total) {
    Assert(total>0);
    int len=bs->getWriteLen();
    BYTE* pos=bs->getWritePos();
    total -= len;
    OZ_Return result = putBytes(pos,len);
    if (result!=PROCEED) {
      bufferManager->dumpByteStream(bs);
      return result;
    }
    bs->sentFirst();
  }
  bs->writeCheck();
  TaggedRef res = bs->getResources();
  bufferManager->freeByteStream(bs);

  //  return oz_unify(resources,bs->resources);
  if (!oz_isNil(res)) {
    return raiseGeneric("save:resources",
                        "Resources found during save",
                        oz_mklist(OZ_pairA("Resources",res),
                                  OZ_pairA("Filename",oz_atom(filename))));
  }

  return PROCEED;
}


// ===================================================================
// class ByteSinkFile
// ===================================================================

OZ_Return
ByteSinkFile::allocateBytes(int n,char *header, crc_t crc, Bool txtmode)
{
  fd = strcmp(filename,"-")==0 ? STDOUT_FILENO
                               : open(filename,O_WRONLY|O_CREAT|O_TRUNC,0666);
  if (fd < 0)
    return raiseGeneric("save:open",
                        "Open failed during save",
                        oz_mklist(OZ_pairA("File",oz_atom(filename)),
                                  OZ_pairA("Error",oz_atom(OZ_unixError(errno)))));

  if (!txtmode) {
    /* write syslet header uncompressed */
    int headerSize;
    char *sysletheader = makeHeader(crc,&headerSize);
    if (ossafewrite(fd,header,strlen(header)) < 0 ||
        ossafewrite(fd,sysletheader,headerSize) < 0) {
      return raiseGeneric("save:write",
                          "Write failed during save",
                          oz_mklist(OZ_pairA("File",oz_atom(filename)),
                                    OZ_pairA("Error",oz_atom(OZ_unixError(errno)))));
    }
  }

  /* gzdopen always spits out the gzip header */
  if (compressionlevel>0) {
    char buf[10];
    sprintf(buf,"w%d",compressionlevel);
    zfd = gzdopen(fd,buf);
    Assert(zfd);
  }

  return PROCEED;
}

OZ_Return
ByteSinkFile::putBytes(BYTE*pos,int len)
{
 loop:
  if (compressionlevel==0 && ossafewrite(fd,(char*)pos,len)<0 ||
      compressionlevel>0 && gzwrite(zfd,pos,len)<0) {
    if (errno != EINTR)
      return raiseGeneric("save:write",
                          "Write failed during save",
                          oz_mklist(OZ_pairA("File",oz_atom(filename)),
                                    OZ_pairA("Error",oz_atom(OZ_unixError(errno)))));
    goto loop;
  }
  return PROCEED;
}

// ===================================================================
// class ByteSinkDatum
// ===================================================================

OZ_Return
ByteSinkDatum::allocateBytes(int n, char *ignored, crc_t crc_ignored,
                             Bool txtmode_ignored)
{
  dat.size = n;
  dat.data = (char*) malloc(n);
  if (dat.data==0)
    return raiseGeneric("save:malloc",
                        "Malloc failed during save",
                        oz_cons(OZ_pairA("Error",oz_atom(OZ_unixError(errno))),
                                oz_nil()));
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
saveDatum(OZ_Term in,OZ_Datum& dat)
{
  ByteSinkDatum sink;
  OZ_Return result = sink.putTerm(in,"filename unknown","",NO);
  if (result==PROCEED) {
    dat=sink.dat;
  } else {
    if (sink.dat.data!=0) free(sink.dat.data);
  }
  return result;
}

static
OZ_Return saveIt(OZ_Term val, char *filename, char *header,
                 int compressionlevel, Bool textmode)
{
  if (compressionlevel < 0 || compressionlevel > 9) {
    return raiseGeneric("save:compressionlevel",
                        "Save: compression level must be between 0 and 9",
                        oz_list(OZ_pairA("File",oz_atom(filename)),
                                OZ_pairAI("Compression level",compressionlevel),
                                0));
  }

  ByteSinkFile sink(filename,compressionlevel);
  OZ_Return ret = sink.putTerm(val,filename,header,textmode);
  if (ret!=PROCEED)
    unlink(filename);
  return ret;
}

OZ_BI_define(BIsave,2,0)
{
  OZ_declareTerm(0,in);
  OZ_declareVirtualString(1,filename);
  return saveIt(in,filename,"",0,NO);
} OZ_BI_end


OZ_BI_define(BInewMarshaler,1,0)
{
  OZ_declareInt(0,nm);
  newMarshaler = nm;
  return PROCEED;
} OZ_BI_end



OZ_BI_define(BIsaveCompressed,3,0)
{
  OZ_declareTerm(0,in);
  OZ_declareVirtualString(1,filename);
  OZ_declareInt(2,complevel);
  return saveIt(in,filename,"",complevel,NO);
} OZ_BI_end


OZ_BI_define(BIsaveWithHeader,4,0)
{
  OZ_declareTerm(0,value);
  OZ_expectDet(1);
  OZ_expectDet(2);
  OZ_declareInt(3,compressionlevel);

  OZ_declareVirtualString(1,filename);
  filename = ozstrdup(filename);
  OZ_declareVirtualString(2,header);

  OZ_Return ret = saveIt(value,filename,header,compressionlevel,NO);
  free(filename);
  return ret;
} OZ_BI_end


#ifdef PICKLE2TEXTHACK
OZ_Return loadFD(int fd, OZ_Term out, const char *compname);
Bool pickle2text()
{
  OZ_Term res    =   oz_newVariable();
  OZ_Term header =   oz_newVariable();
  OZ_Return aux = loadFD(STDIN_FILENO,oz_pair2(header,res),"-");
  if (aux==RAISE) {
    fprintf(stderr,"Exception: %s\n",OZ_toC(am.getExceptionValue(),10,100));
    return NO;
  }
  aux = saveIt(res,"-",OZ_stringToC(header,0),0,OK);
  if (aux==RAISE) {
    fprintf(stderr,"Exception: %s\n",OZ_toC(am.getExceptionValue(),10,100));
    return NO;
  }
  return OK;
}

#endif

OZ_Return export(OZ_Term t)
{
  if (ozconf.perdioMinimal) {
    Exporter bs;
    marshalTermRT0(t,&bs);
    CheckNogoods(t,(&bs),"export:nogoods","Non-exportables found during export",);

    OZ_Term vars = bs.getVars();
    while (!oz_isNil(vars)) {
      OZ_Term t = oz_head(vars);
      DEREF(t,tPtr,_2);
      oz_getVar(tPtr)->markExported();
      vars = oz_tail(vars);
    }

  }
  return PROCEED;
}

OZ_BI_define(BIexport,1,0)
{
  OZ_declareTerm(0,in);

  return export(in);
} OZ_BI_end

// ===================================================================
// pb2 used for perdio but put here as needs componentBuffer.cc data
// structures - to be removed when marshaler rebuilt
// ===================================================================

OZ_Term digOutVars(OZ_Term t)
{
  int cached=ozconf.perdioMinimal;
  ozconf.perdioMinimal=TRUE;
  Exporter bs;
  marshalTermRT0(t,&bs);
  OZ_Term vars=bs.getVars();
  ozconf.perdioMinimal=cached;
  return vars;
}

// ===================================================================
// class ByteSource
// ===================================================================

static
Bool loadTerm(ByteStream *buf,char* &vers,OZ_Term &t)
{
  refTable->reset();
  Assert(refTrail->isEmpty());
  vers = unmarshalVersionString(buf);

  if (vers==0)
    return NO;

  int major,minor;
  if (sscanf(vers,"%d#%d",&major,&minor) != 2) {
    return NO;
  }

  Bool newFormat = NO;

  if (major!=PERDIOMAJOR || minor > PERDIOMINOR) {
    if (major==2 && minor==0)
      newFormat = OK;
    else
      return NO;
  }

  buf->setVersion(major,minor);

  t = newFormat ? newUnmarshalTerm(buf) : unmarshalTerm(buf);
  buf->unmarshalEnd();
  refTrail->unwind();
  return OK;
}


OZ_Return
ByteSource::getTerm(OZ_Term out, const char *compname, Bool wantHeader)
{
  ByteStream * stream;

  OZ_Return result = makeByteStream(stream,compname);
  if (result!=PROCEED) return result;
  // EK fast fix

  stream->beforeInterpret(0);
  stream->unmarshalBegin();

  char *versiongot = 0;
  OZ_Term val;

  if(loadTerm(stream,versiongot,val)){
    stream->afterInterpret();
    bufferManager->dumpByteStream(stream);
    delete versiongot;
    return wantHeader ? oz_unify(out,oz_pair2(OZ_string(getHeader()),val)) // mm_u
                      : oz_unify(out,val);
  }

  bufferManager->dumpByteStream(stream);
  if (versiongot) {
    OZ_Term vergot = oz_atom(versiongot);
    delete versiongot;
    return raiseGeneric("load:versionmismatch",
                        "Version mismatch during loading of pickle",
                        oz_mklist(OZ_pairA("File",oz_atom(compname)),
                                  OZ_pairA("Expected",oz_atom(PERDIOVERSION)),
                                  OZ_pairA("Got",vergot)));
  } else {
    return raiseGeneric("load:nonpickle",
                        "Trying to load non-pickle",
                        oz_cons(OZ_pairA("File",oz_atom(compname)),oz_nil()));
  }
}


OZ_Return
ByteSource::makeByteStream(ByteStream*& stream, const char *filename)
{
  stream = bufferManager->getByteStream();
  stream->getSingle();
  int max,got;
  int total = 0;
  BYTE *pos = stream->initForRead(max);
  crc_t crc = init_crc();
  while (TRUE) {
    OZ_Return result = getBytes(pos,max,got);
    if (result!=PROCEED) return result;
    crc = update_crc(crc,pos,got);
    total += got;
    stream->afterRead(got);
    if (got<max) break;
    pos = stream->beginRead(max);
  }
  if (total==0)
    return raiseGeneric("bytesource:empty",
                        "Magic header not found (not a pickle?)",
                        oz_cons(OZ_pairA("File",oz_atom(filename)),oz_nil()));

  if (checkChecksum(crc)==NO)
    return raiseGeneric("bytesource:crc","Checksum mismatch",
                        oz_cons(OZ_pairA("File",oz_atom(filename)),oz_nil()));

  return PROCEED;
}

// ===================================================================
// class ByteSourceFD
// ===================================================================


OZ_Return
ByteSourceFD::getBytes(BYTE*pos,int&max,int&got)
{
loop:
  got = gzread(fd,pos,max);
  if (got < 0) {
    if (errno==EINTR) goto loop;
    int errnum;
    const char* msg;
    msg = gzerror(fd,&errnum);
    if (errnum==Z_ERRNO) msg=OZ_unixError(errno);
    return raiseGeneric("load:read",
                        "Read error during load",
                        oz_cons(OZ_pairA("Error",oz_atom(msg)),
                                oz_nil()));
  }
  return PROCEED;
}

// ===================================================================
// class ByteSourceDatum
// ===================================================================

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
  ByteSourceDatum src(dat);
  return src.getTerm(out,"filename unknown",NO);
}

OZ_Return loadFD(int fd, OZ_Term out, const char *compname)
{
  ByteSourceFD src(fd);
  return src.getTerm(out,compname,OK);
}


char *newTempFile()
{
  char tn[L_tmpnam] = ""; // I like POSIX!
  ostmpnam(tn);
  return ozstrdup(tn);
}

enum URLAction { URL_LOCALIZE , URL_OPEN , URL_LOAD };

class PipeInfo {
public:
  int fd;
  int pid;
  const char *file;
  char *url;
  TaggedRef controlvar, out;
  URLAction action;

  PipeInfo(int f, int p, const char *tmpf, const char *u, TaggedRef o, TaggedRef var,
           URLAction act):
    fd(f), pid(p), file(tmpf), out(o), action(act)
  {
    controlvar = var;;
    url = ozstrdup(u);
    OZ_protect(&controlvar);
    OZ_protect(&out);
  }
  ~PipeInfo() {
    OZ_unprotect(&controlvar);
    OZ_unprotect(&out);
    delete url;
  }
};

#define ACTION_STRING(act)      \
((act==URL_LOCALIZE)?"localize":\
 (act==URL_OPEN    )?"open":    \
 (act==URL_LOAD    )?"load":    \
 "<unknown action>")


static
void doRaise(TaggedRef controlvar, char *msg, const char *url,URLAction act)
{
  ControlVarRaise(controlvar,
                  makeGenericExc("URLhandler",
                                 "Error in URL handler",
                                 oz_mklist(OZ_pairA("Message",oz_atom(msg)),
                                           OZ_pairA("Action",oz_atom(ACTION_STRING(act))),
                                           OZ_pairA("URL",oz_atom(url)))));
}

static
int pipeHandler(int, void *arg)
{
  PipeInfo *pi = (PipeInfo *) arg;
  char retloc = 0;
  int n = osread(pi->fd,&retloc,sizeof(retloc));
  osclose(pi->fd);

  TaggedRef controlvar = pi->controlvar;

#ifndef WINDOWS
  int u = waitpid(pi->pid,NULL,0);
  if (u!=pi->pid) {
    doRaise(controlvar,OZ_unixError(errno),pi->url,pi->action);
    // mm2: cleanup missing?
    return NO;
  }
#endif

  if (retloc!=URLC_OK) {
    doRaise(controlvar,urlcStrerror(retloc),pi->url,pi->action);
  } else {

    switch (pi->action) {
    case URL_LOCALIZE:
      // this is only called when the file is remote, therefore
      // a local copy will be made into the file
      ControlVarUnify(controlvar,pi->out,
                      OZ_mkTupleC("new",1,oz_atom(pi->file)));
      break;
    case URL_OPEN:
      {
        int fd = osopen(pi->file,O_RDONLY,0);
        if (fd < 0) {
          doRaise(controlvar,OZ_unixError(errno),pi->url,pi->action);
        } else {
          unlink(pi->file);
          ControlVarUnify(controlvar,pi->out,OZ_int(fd));
        }
        break;
      }
    case URL_LOAD:
      {
        int fd = osopen(pi->file, O_RDONLY,0);
        if (fd < 0) {
          doRaise(controlvar,OZ_unixError(errno),pi->url,pi->action);
        } else {
          OZ_Term other = oz_newVariable();
          OZ_Return aux = loadFD(fd,other,pi->file);
          if (aux==RAISE) {
            ControlVarRaise(controlvar,am.getExceptionValue());
          } else {
            Assert(aux==PROCEED); // mm_u
            unlink(pi->file);
            ControlVarUnify(controlvar,pi->out,other);
          }
        }
        break;
      }
    }
  }

  delete pi->file;
  delete pi;
  return OK;
}

#ifdef WINDOWS

class URLInfo {
public:
  char *tmpfile;
  const char *url;
  int fd;
  URLInfo(const char *file, const char *u, int f):
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
  ossafewrite(ui->fd,buf,2);
  osclose(ui->fd);
  delete ui;
  ExitThread(1);
  return 1;
}

#endif



static
OZ_Return getURL(const char *url, TaggedRef out, URLAction act)
{
  char *tmpfile = newTempFile();

#ifdef WINDOWS

  HANDLE rh,wh;
  CreatePipe(&rh,&wh,0,0);
  int wfd = _hdopen((int)wh,O_WRONLY|O_BINARY);
  int rfd = _hdopen((int)rh,O_RDONLY|O_BINARY);

  URLInfo *ui = new URLInfo(tmpfile,url,wfd);

  unsigned tid;
  HANDLE thrd = CreateThread(NULL,0,&fetchThread,ui,0,&tid);
  if (thrd==NULL)
    return raiseGeneric("getURL:thread",
                        "getURL: start thread failed",
                        oz_cons(OZ_pairA("URL",oz_atom(url)),oz_nil()));
  int pid = 0;

#else

  int fds[2];
  if (pipe(fds)<0) {
    return raiseGeneric("getURL:pipe",
                        "getURL: system call 'pipe' failed",
                        oz_cons(OZ_pairA("URL",oz_atom(url)),oz_nil()));
  }

  pid_t pid = fork();
  switch(pid) {
  case 0: /* child */
    {
      osclose(fds[0]);
      char ret = (char) localizeUrl(url,tmpfile);
      ossafewrite(fds[1],&ret,sizeof(ret));
      exit(0);
    }
  case -1:
    return raiseGeneric("getURL:fork",
                        "getURL: system call 'fork' failed",
                        oz_cons(OZ_pairA("URL",oz_atom(url)),oz_nil()));
  default:
    break;
  }

  osclose(fds[1]);

  int rfd = fds[0];
#endif

  ControlVarNew(var,am.currentBoard());
  PipeInfo *pi = new PipeInfo(rfd,pid,tmpfile,url,out,var,act);
  OZ_registerReadHandler(rfd,pipeHandler,pi);
  SuspendOnControlVar;
}


// URL_get is a primitive that performs an action on a url.
// the action maybe URL_LOCALIZE, URL_OPEN, or URL_LOAD.
// In the case or URL_LOCALIZE, the return value is either:
//      old(PATH)
//              if PATH is the pathname of the original
//              local file
//      new(PATH)
//              if PATH is the pathname of a new local copy
//              of the remote file

#include <ctype.h>
#include <unistd.h>

OZ_Return URL_get(const char*url,OZ_Term& out,URLAction act)
{
#ifdef WINDOWS
  // check for WINDOWS style absolute pathname
  if (isalpha(url[0]) && url[1]==':' && (url[2]=='/' || url[2]=='\\')) {
    goto url_local;
  }
#endif
  if (strncmp(url,"file:",5)==0) { url+=5; goto url_local; }
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
      out = OZ_mkTupleC("old",1,oz_atom(url));
      return PROCEED;
    }
  case URL_OPEN:
    {
      int fd = osopen(url,O_RDONLY,0);
      if (fd<0) goto kaboom;
      out = OZ_int(fd);
      return PROCEED;
    }
  case URL_LOAD:
    {
      int fd = osopen(url,O_RDONLY,0);
      if (fd<0) goto kaboom;
      OZ_Term   val    = oz_newVariable();
      OZ_Return status = loadFD(fd,val,url);
      if (status==PROCEED) out=val;
      return status;
    }
  default:
    Assert(0);
    return FAILED;
  }
url_remote:
  out = oz_newVariable();
  return getURL(url,out,act);
kaboom:
  return oz_raise(E_SYSTEM,oz_atom("url"),ACTION_STRING(act),2,
                  oz_atom(OZ_unixError(errno)),
                  oz_atom(url));
}

OZ_BI_define(BIurl_localize,1,1)
{
  OZ_declareVirtualString(0,url);
  return URL_get(url,OZ_out(0),URL_LOCALIZE);
} OZ_BI_end

OZ_BI_define(BIurl_open,1,1)
{
  OZ_declareVirtualString(0,url);
  return URL_get(url,OZ_out(0),URL_OPEN);
} OZ_BI_end

OZ_BI_define(BIurl_load,1,1)
{
  OZ_declareVirtualString(0,url);
  OZ_Term aux = 0;
  OZ_Return ret = URL_get(url,aux,URL_LOAD);
  if (aux != 0) {
    OZ_Term aux2 = oz_newVariable();
    OZ_Return unifyret = OZ_unify(oz_pair2(oz_newVariable(),aux2),aux);
    Assert(unifyret==PROCEED);
    OZ_result(aux2);
  }

  return ret;
} OZ_BI_end

OZ_BI_define(BIloadWithHeader,1,1)
{
  OZ_declareVirtualString(0,url);
  return URL_get(url,OZ_out(0),URL_LOAD);
} OZ_BI_end

OZ_BI_define(BIload,2,0)
{
  OZ_Term loader = registry_get(AtomLoad);
  if (loader==0)
    loader = BI_url_load;
  am.prepareCall(loader,OZ_in(0),OZ_in(1));
  return BI_REPLACEBICALL;
} OZ_BI_end


OZ_Return OZ_valueToDatum(OZ_Term t, OZ_Datum* d)
{
  return saveDatum(t,*d);
}


OZ_Return OZ_datumToValue(OZ_Datum d,OZ_Term t)
{
  return loadDatum(d,t);
}
