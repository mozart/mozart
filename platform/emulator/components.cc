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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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
#include "dictionary.hh"
#include "urlc.hh"
#include "site.hh"
#include "mbuffer.hh"
#include "builtins.hh"
#include "os.hh"
#include "var_simple.hh"
#include "pickle.hh"
#include "componentBuffer.hh"

#ifndef WINDOWS
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#endif

#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>

#include "zlib.h"

/* ********************************************************************** */
/*              BUILTINS                                                  */
/* ********************************************************************** */

// class ByteSource [Denys Duchier]
// something to marshal from

class ByteSource {
public:
  virtual OZ_Return getBytes(BYTE*, int, int&) = 0;
  virtual char *getHeader() = 0;
  virtual Bool checkChecksum(crc_t) = 0;
  OZ_Return getTerm(OZ_Term, const char *compname, Bool);
  OZ_Return loadPickleBuffer(PickleBuffer*&, const char*);
};

class ByteSourceFD : public ByteSource {
private:
  gzFile fd;
  crc_t checkSum;
  char *header;
public:
  char *getHeader() {return header; }
  virtual ~ByteSourceFD() { free(header); gzclose(fd); }
  OZ_Return getBytes(BYTE*, int, int&);

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
    header = strdup(buf);
    free(buf);
    checkSum = 0;

    for (unsigned int k=0; k<sizeof(crc_t); k++) {
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
  virtual ~ByteSourceDatum() {}
  OZ_Return getBytes(BYTE*, int, int&);
  virtual Bool checkChecksum(crc_t i) { return OK; }
};


// class ByteSink [Denys Duchier]
// something to marshal into

class ByteSink {
public:
  OZ_Return putTerm(OZ_Term,char*,char*,unsigned int,Bool,Bool);
  virtual OZ_Return putBytes(BYTE*,int) = 0;
  virtual OZ_Return allocateBytes(int,char*,unsigned int,crc_t,Bool) = 0;
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
  OZ_Return allocateBytes(int,char*,unsigned int,crc_t,Bool);
  OZ_Return putBytes(BYTE*,int);
};

class ByteSinkDatum : public ByteSink {
private:
  int      idx;
public:
  OZ_Datum dat;
  ByteSinkDatum():idx(0){ dat.size=0; dat.data=0; }
  OZ_Return allocateBytes(int,char*,unsigned int,crc_t,Bool);
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
  return OZ_raiseDebug(makeGenericExc(id,msg,arg));
}

OZ_Return onlyFutures(OZ_Term l) {
  if (oz_isNil(l)) return PROCEED;
  while (oz_isCons(l)) {
    OZ_Term f=oz_head(l);
    if (!oz_isFuture(oz_deref(f))) {
      am.emptySuspendVarList();
      return PROCEED;
    }
    (void) oz_addSuspendVarList(f);
    l = oz_tail(l);
  }
  return SUSPEND;
}

OZ_Return
ByteSink::putTerm(OZ_Term in, char *filename, char *header,
                  unsigned int hlen,
                  Bool textmode, Bool cloneCells)
{
  OZ_Term resources, nogoods;
  int total, len;
  BYTE *pos;
  crc_t crc;

  //
  extractResources(in, cloneCells, resources, nogoods);
  OZ_Return ret = onlyFutures(resources);
  if (ret != PROCEED)
    return ret;

  //
  if (!oz_isNil(resources)) {
    return raiseGeneric("pickle:resources",
                        "Resources found during pickling",
                        oz_mklist(OZ_pairA("Resources", resources),
                                  OZ_pairA("Filename", oz_atom(filename))));
  }

  //
  if (!oz_isNil(nogoods)) {
    return raiseGeneric("pickle:nogoods",
                        "Non-exportables found during pickling",
                        oz_mklist(OZ_pairA("Resources", nogoods),
                                  OZ_pairA("Contained in", in)));
  }

  //
  PickleBuffer* pb = new PickleBuffer();
  if (textmode)
    pb->setTextmode();

  //
  pb->marshalBegin();
  marshalString(pb, MARSHALERVERSION);
  pickleTerm(in, pb, cloneCells);
  pb->marshalEnd();

  //
  pb->saveBegin();
  total = 0;

  // crc"ing;
  crc = init_crc();
  pos = pb->accessFirst(len);
  do {
    Assert(pos != (BYTE *) 0);
    total += len;
    crc = update_crc(crc, pos, len);
    pb->chunkDone();
    pos = pb->accessNext(len);
  } while (pos);

  // header - also open file if nec
  OZ_Term result = allocateBytes(total, header, hlen, crc, textmode);
  if (result!=PROCEED) {
    delete pb;
    return result;
  }

  //
  pos = pb->unlinkFirst(len);
  do {
    Assert(total > 0 && pos != (BYTE *) 0);
    total -= len;
    OZ_Return result = putBytes(pos,len);
    if (result != PROCEED) {
      do {
        pb->chunkWritten();
      } while (pb->unlinkNext(len));
      delete pb;
      return (result);
    }
    pb->chunkWritten();
    pos = pb->unlinkNext(len);
  } while (total);
  Assert(total == 0 && pos == (BYTE *) 0);

  //
  pb->saveEnd();
  delete pb;
  return (PROCEED);
}


// ===================================================================
// class ByteSinkFile
// ===================================================================

OZ_Return
ByteSinkFile::allocateBytes(int n,char * header, unsigned int hlen, crc_t crc, Bool txtmode)
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
    if (ossafewrite(fd,header,hlen) < 0 ||
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
ByteSinkDatum::allocateBytes(int n, char *, unsigned int, crc_t crc_ignored,
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
  OZ_Return result = sink.putTerm(in,"UNKNOWN FILENAME","",0,
                                  NO,ozconf.pickleCells);
  if (result==PROCEED) {
    dat=sink.dat;
  } else {
    if (sink.dat.data!=0) free(sink.dat.data);
  }
  return result;
}

OZ_Return
saveDatumWithCells(OZ_Term in,OZ_Datum& dat)
{
  ByteSinkDatum sink;
  OZ_Return result = sink.putTerm(in,"UNKNOWN FILENAME","",0,NO,OK);
  if (result==PROCEED) {
    dat=sink.dat;
  } else {
    if (sink.dat.data!=0) free(sink.dat.data);
  }
  return result;
}

static
OZ_Return saveIt(OZ_Term val, char *filename, char *header,
                 unsigned int hlen,
                 int compressionlevel, Bool textmode, Bool cloneCells)
{
  if (compressionlevel < 0 || compressionlevel > 9) {
    return raiseGeneric("save:compressionlevel",
                        "Save: compression level must be between 0 and 9",
                        oz_list(OZ_pairA("File",oz_atom(filename)),
                                OZ_pairAI("Compression level",compressionlevel),
                                0));
  }

  ByteSinkFile sink(filename,compressionlevel);
  OZ_Return ret = sink.putTerm(val,filename,header,hlen,textmode,cloneCells);
  if (ret!=PROCEED)
    unlink(filename);
  return ret;
}

OZ_BI_define(BIsave,2,0)
{
  OZ_declareTerm(0,in);
  OZ_declareVirtualString(1,filename);
  return saveIt(in,filename,"",0,0,NO,ozconf.pickleCells);
} OZ_BI_end


OZ_BI_define(BIsaveCompressed,3,0)
{
  OZ_declareTerm(0,in);
  OZ_declareVirtualString(1,filename);
  OZ_declareInt(2,complevel);
  return saveIt(in,filename,"",0,complevel,NO,ozconf.pickleCells);
} OZ_BI_end


OZ_BI_define(BIsaveWithHeader,4,0)
{
  OZ_declareTerm(0,value);
  OZ_expectDet(1);
  OZ_expectDet(2);
  OZ_declareInt(3,compressionlevel);

  OZ_declareVirtualString(1,filename);
  filename = strdup(filename);
  OZ_declareVS(2,header,len);

  OZ_Return ret = saveIt(value,filename,header,len,compressionlevel,
                         NO,ozconf.pickleCells);
  free(filename);
  return ret;
} OZ_BI_end


OZ_BI_define(BIsaveWithCells,4,0)
{
  OZ_declareTerm(0,value);
  OZ_expectDet(1);
  OZ_expectDet(2);
  OZ_declareInt(3,compressionlevel);

  OZ_declareVirtualString(1,filename);
  filename = strdup(filename);
  OZ_declareVS(2,header,len);

  OZ_Return ret = saveIt(value,filename,header,len,compressionlevel,NO,OK);
  free(filename);
  return ret;
} OZ_BI_end


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
  char * s = OZ_stringToC(header,0);
  aux = saveIt(res,"-",s,strlen(s),0,OK,ozconf.pickleCells);
  if (aux==RAISE) {
    fprintf(stderr,"Exception: %s\n",OZ_toC(am.getExceptionValue(),10,100));
    return NO;
  }
  return OK;
}


// ===================================================================
// class ByteSource
// ===================================================================

//
typedef enum {
  CLT_OK = 0,
  CLT_NOPICKLE,
  CLT_WRONGVERS,
  CLT_FORMATERR
} LoadTermRet;

static
LoadTermRet loadTerm(PickleBuffer *buf, char* &vers, OZ_Term &t)
{
  buf->unmarshalBegin();

  vers = unmarshalVersionString(buf);
  if (vers == 0)
    return (CLT_NOPICKLE);

  int major, minor;
  if (sscanf(vers,"%d#%d", &major, &minor) != 2)
    return (CLT_NOPICKLE);
  if (major != MARSHALERMAJOR || minor != MARSHALERMINOR)
    return (CLT_WRONGVERS);

  t = unpickleTerm(buf);

  buf->unmarshalEnd();

  return (CLT_OK);
}


//
// Map a marshaler version into a mozart version.
// The table must be sorted on marshaler numbers. It does not need to be
// complete but then it is not complete :-)
typedef struct {
  int major, minor;
  char ozversion[16];
} mv2ovTabType;
//
static mv2ovTabType mv2ovTab[] = {
  { 1, 5, "1.0.1" },
  { 2, 0, "1.1.0" },
  { 3, 0, "1.1.0" },
  { 3, 1, "1.1.1" },
  { 3, 2, "1.2.0" }
};

// returns a string to be used as "oz version %s";
// returned string must be deallocated;
char *mv2ov(char *pvs)
{
  int major, minor, num, i;
  char *buf = (char *) malloc(128);
  int pvn;

  //
  if (sscanf(pvs,"%d#%d",&major,&minor) != 2) {
    sprintf(buf, "cannot be determined");
    return (buf);
  }
  pvn = (int) (major << 16) | minor;

  //
  num = sizeof(mv2ovTab)/sizeof(mv2ovTabType);
  Assert(num);
  for (i = 0; ; i++) {
    int tpvn = (int) (mv2ovTab[i].major << 16) | mv2ovTab[i].minor;

    if (pvn == tpvn)
      sprintf(buf, "%s", mv2ovTab[i].ozversion);
    else if (pvn < tpvn)
      sprintf(buf, "earlier than %s(%d#%d)", mv2ovTab[i].ozversion,
              mv2ovTab[i].major, mv2ovTab[i].minor);
    else if (i == num-1)
      sprintf(buf, "later than %s(%d#%d)", mv2ovTab[i].ozversion,
              mv2ovTab[i].major, mv2ovTab[i].minor);
    else
      continue;
    break;
  }
  return (buf);
}

OZ_Return
ByteSource::getTerm(OZ_Term out, const char *compname, Bool wantHeader)
{
  PickleBuffer * buffer;
  char *versiongot = 0;
  OZ_Term val;
  LoadTermRet ret;

  OZ_Return result = loadPickleBuffer(buffer, compname);
  if (result != PROCEED) {
    return (result);
  }

  ret = loadTerm(buffer, versiongot, val);
  buffer->dropBuffers();
  delete buffer;

  switch (ret) {
  case CLT_OK:
    delete [] versiongot;
    if (wantHeader) {
      OZ_Return ret = oz_unify(out, oz_pair2(OZ_string(getHeader()), val));
      // kost@ : that's how it is used;
      Assert(ret == PROCEED);
      return (ret);
    } else {
      OZ_Return ret = oz_unify(out, val);
      // kost@ : cannot handle suspensions;
      Assert(ret == PROCEED || ret == FAILED);
      return (ret);
    }
    Assert(0);

  case CLT_NOPICKLE:
    return raiseGeneric("load:nonpickle", "Trying to load a non-pickle",
                        oz_cons(OZ_pairA("File",oz_atom(compname)),oz_nil()));
    Assert(0);

  case CLT_WRONGVERS:
    {
      OZ_Term vergot = oz_atom(versiongot);
      char *vs = mv2ov(versiongot);
      OZ_Term ozvergot = oz_atom(vs);
      char s1[80];
      sprintf(s1, "Pickle version %s corresponds to Oz version", versiongot);
      delete [] versiongot;
      delete vs;
      return raiseGeneric("load:versionmismatch",
                          "Version mismatch during loading of pickle",
                          oz_mklist(OZ_pairA("File",oz_atom(compname)),
                                    OZ_pairA("Expected",oz_atom(MARSHALERVERSION)),
                                    OZ_pairA("Got",vergot),
                                    OZ_pairA(s1,ozvergot)));
    }
    Assert(0);

  case CLT_FORMATERR:
    delete [] versiongot;
    return raiseGeneric("load:formaterr", "Error during unmarshaling",
                        oz_cons(OZ_pairA("File",oz_atom(compname)),oz_nil()));
    Assert(0);

  default:
    Assert(0);
    return (PROCEED);
  }
  Assert(0);
}

OZ_Return
ByteSource::loadPickleBuffer(PickleBuffer*& buffer, const char *filename)
{
  BYTE *pos;
  int max;
  int total = 0;
  //
  buffer = new PickleBuffer();
  crc_t crc = init_crc();

  //
  buffer->loadBegin();
  pos = buffer->allocateFirst(max);
  while (TRUE) {
    int got;
    OZ_Return result = getBytes(pos, max, got);
    if (result != PROCEED) {
      buffer->dropBuffers();
      delete buffer;
      DebugCode(buffer = (PickleBuffer *) 0;);
      return result;
    }
    total += got;
    crc = update_crc(crc, pos, got);

    //
    buffer->chunkRead(got);
    if (got < max)
      break;
    pos = buffer->allocateNext(max);
  }
  buffer->loadEnd();

  //
  if (total == 0) {
    buffer->dropBuffers();
    delete buffer;
    DebugCode(buffer = (PickleBuffer *) 0;);
    return (raiseGeneric("bytesource:empty",
                         "Magic header not found (not a pickle?)",
                         oz_cons(OZ_pairA("File",
                                          oz_atom(filename)), oz_nil())));
  }
  if (checkChecksum(crc) == NO) {
    buffer->dropBuffers();
    delete buffer;
    DebugCode(buffer = (PickleBuffer *) 0;);
    return (raiseGeneric("bytesource:crc","Checksum mismatch",
                         oz_cons(OZ_pairA("File",
                                          oz_atom(filename)), oz_nil())));
  }

  return (PROCEED);
}

// ===================================================================
// class ByteSourceFD
// ===================================================================


OZ_Return
ByteSourceFD::getBytes(BYTE *pos, int max, int &got)
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
ByteSourceDatum::getBytes(BYTE *pos, int max, int &got)
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

OZ_Return loadDatum(OZ_Datum dat, OZ_Term out)
{
  ByteSourceDatum src(dat);
  return src.getTerm(out, "filename unknown", NO);
}

OZ_Return loadFD(int fd, OZ_Term out, const char *compname)
{
  ByteSourceFD src(fd);
  return src.getTerm(out, compname, OK);
}


char *newTempFile()
{
#ifdef WINDOWS
  char tn[MAX_PATH] = ""; // Windows can return longer path names
#else
  char tn[L_tmpnam] = ""; // I like POSIX!
#endif
  ostmpnam(tn);
  return strdup(tn);
}

enum URLAction { URL_LOCALIZE , URL_OPEN , URL_LOAD };

class PipeInfo {
public:
  int fd;
  int pid;
  char *file;
  char *url;
  TaggedRef controlvar, out;
  URLAction action;

  PipeInfo(int f, int p, char *tmpf, const char *u, TaggedRef o, TaggedRef var,
           URLAction act):
    fd(f), pid(p), file(tmpf), out(o), action(act)
  {
    controlvar = var;;
    url = strdup(u);
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
  char *url;
  int fd;
  URLInfo(const char *file, const char *u, int f):
    tmpfile(strdup(file)), url(strdup(u)), fd(f) {}
  ~URLInfo() {
    delete tmpfile;
    delete url;
  }
};

DWORD __stdcall fetchThread(void *p)
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

  int sv[2];
  (void) ossocketpair(PF_UNIX,SOCK_STREAM,0,sv);

  int wfd = sv[0];
  int rfd = sv[1];

  URLInfo *ui = new URLInfo(tmpfile,url,wfd);

  DWORD tid;
  HANDLE thrd = CreateThread(NULL,0,&fetchThread,ui,0,&tid);
  if (thrd==NULL)
    return raiseGeneric("getURL:thread",
                        "getURL: start thread failed",
                        oz_cons(OZ_pairA("URL",oz_atom(url)),oz_nil()));
  CloseHandle(thrd);
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

inline unsigned char toHex(char c) {
  switch (c) {
  case '0': return 0;
  case '1': return 1;
  case '2': return 2;
  case '3': return 3;
  case '4': return 4;
  case '5': return 5;
  case '6': return 6;
  case '7': return 7;
  case '8': return 8;
  case '9': return 9;
  case 'a': case 'A': return 10;
  case 'b': case 'B': return 11;
  case 'c': case 'C': return 12;
  case 'd': case 'D': return 13;
  case 'e': case 'E': return 14;
  case 'f': case 'F': return 15;
  default:
    return 16;
  }
}

inline void urlDecode(const char*s1, char*s2) {
  while (*s1 != '\0') {
    unsigned char i1,i2;
    if (*s1 == '%' &&
        (i1 = toHex(s1[1]))<16 &&
        (i2 = toHex(s1[2]))<16) {
      *s2++ = (char) (i1*16 + i2);
      s1 += 3;
    }
    else
      *s2++ = *s1++;
  }
  *s2='\0';
}

#define Return(val) ret=val; goto exit;

OZ_Return URL_get(const char*url,OZ_Term& out,URLAction act)
{
  char *urlDecoded = new char [strlen(url)+1];
  char *urlDecStart = urlDecoded;
  urlDecode(url,urlDecoded);
  OZ_Return ret;
#ifdef WINDOWS
  // check for WINDOWS style absolute pathname
  if (isalpha(url[0]) && url[1]==':' && (url[2]=='/' || url[2]=='\\')) {
    goto url_local;
  }
#endif
  if (strncmp(url,"file:",5)==0) { urlDecoded+=5; goto url_local; }
  {
    const char*s=url;
    while (isalnum(*s)) s++;
    if (*s==':') goto url_remote;
  }
url_local:

  switch (act) {
  case URL_LOCALIZE:
    {
      if (access(urlDecoded,F_OK)<0) goto kaboom;
      out = OZ_mkTupleC("old",1,oz_atom(urlDecoded));
      Return(PROCEED);
    }
  case URL_OPEN:
    {
      int fd = osopen(urlDecoded,O_RDONLY,0);
      if (fd<0) goto kaboom;
      out = OZ_int(fd);
      Return(PROCEED);
    }
  case URL_LOAD:
    {
      int fd = osopen(urlDecoded,O_RDONLY,0);
      if (fd<0) goto kaboom;
      OZ_Term   val    = oz_newVariable();
      OZ_Return status = loadFD(fd,val,urlDecoded);
      if (status==PROCEED) out=val;
      Return(status);
    }
  default:
    Assert(0);
    Return(FAILED);
  }
url_remote:
  out = oz_newVariable();
  Return(getURL(url,out,act));
kaboom:
  Return(oz_raise(E_SYSTEM,oz_atom("url"),ACTION_STRING(act),2,
                  oz_atom(OZ_unixError(errno)),
                  oz_atom(url)));

 exit:
  delete [] urlDecStart;
  return ret;
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
    Assert(unifyret == PROCEED);
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
  am.prepareCall(loader,RefsArray::make(OZ_in(0),OZ_in(1)));
  return BI_REPLACEBICALL;
} OZ_BI_end


OZ_Return OZ_valueToDatum(OZ_Term t, OZ_Datum* d)
{
  return saveDatum(t,*d);
}

OZ_Return OZ_valueToDatumWithCells(OZ_Term t, OZ_Datum* d)
{
  return saveDatumWithCells(t,*d);
}


OZ_Return OZ_datumToValue(OZ_Datum d, OZ_Term t)
{
  return loadDatum(d, t);
}

OZ_BI_define(BIpicklePack, 1, 1) {
  OZ_declareTerm(0,term);
  OZ_Datum d;
  OZ_Return r = OZ_valueToDatum(term, &d);

  if (r != PROCEED)
    return r;

  r = OZ_mkByteString(d.data, d.size);
  free(d.data);

  OZ_RETURN(r);
} OZ_BI_end

OZ_BI_define(BIpicklePackWithCells, 1, 1) {
  OZ_declareTerm(0,term);
  OZ_Datum d;
  OZ_Return r = OZ_valueToDatumWithCells(term, &d);

  if (r != PROCEED)
    return r;

  r = OZ_mkByteString(d.data, d.size);
  free(d.data);

  OZ_RETURN(r);
} OZ_BI_end


OZ_BI_define(BIpickleUnpack, 2, 0) {
  OZ_declareVS(0,string,sz);
  OZ_declareTerm(1,out);
  //
  if (!(!OZ_isVariable(out) || oz_isFree(oz_deref(out))))
    return (OZ_typeError(1, "value or a free variable"));

  OZ_Datum d;
  d.data = string;
  d.size = sz;
  return OZ_datumToValue(d, out);
} OZ_BI_end

#ifdef DENYS_XML
typedef void (*marshalFun)(OZ_Term,MarshalerBuffer*);
OZ_Term toXML(OZ_Term t,marshalFun f)
{
  int len;
  BYTE *pos;
  PickleBuffer* pb = new PickleBuffer();

  //
  pb->marshalBegin();
  (*f)(t,pb);
  pb->marshalEnd();

  pb->saveBegin();
  int total=pb->calcTotLen();
  OZ_Term val = oz_nil();
  pos = pb->getFirst(len);
  do {
    val = OZ_pair2(val,OZ_mkByteString((char*)pos,len));
    pos = pb->getNext(len);
  }
  pb->saveEnd();
  delete pb;
  return val;
}
#endif

#if defined(PROFILE_MARSHALER)
//
#include "byteBuffer.hh"
#include "libdp/dpMarshaler.hh"

#define TestMError(PROC, MSG) \
  OZ_raiseErrorC("testM",3,OZ_atom(PROC),OZ_inAsList(),OZ_atom(MSG))

OZ_BI_define(BIemptyBuiltin, 0, 0)
{
  return PROCEED;
} OZ_BI_end

// Oz value + number of iterations;
OZ_BI_define(BItestMarshaler, 2, 0)
{
  OZ_declareDetTerm(0, val);
  OZ_declareInt(1, iterations);
  PickleBuffer* buf = new PickleBuffer();

  for (int i = iterations; i--; ) {
    OZ_Term resources, nogoods;

    extractResources(val, 0, resources, nogoods);

    if (!oz_isNil(resources))
      return (TestMError("testMarshaler", "resources found!"));
    if (!oz_isNil(nogoods))
      return (TestMError("testMarshaler", "nogoods found!"));

    buf->marshalBegin();
    pickleTerm(val, buf, 0);
    buf->marshalEnd();

    buf->saveBegin();
    int len;
    (void) buf->unlinkFirst(len);
    do {
      buf->chunkWritten();
    } while (buf->unlinkNext(len));
    buf->saveEnd();
  }

  delete buf;
  return PROCEED;
} OZ_BI_end

// Oz value + number of iterations.
// Note: first, the value is marshaled, which takes some time;
OZ_BI_define(BItestUnmarshaler, 2, 0)
{
  OZ_declareDetTerm(0, val);
  OZ_declareInt(1, iterations);

  OZ_Term resources, nogoods;

  extractResources(val, 0, resources, nogoods);

  if (!oz_isNil(resources))
    return (TestMError("testMarshaler", "resources found!"));
  if (!oz_isNil(nogoods))
    return (TestMError("testMarshaler", "nogoods found!"));

  PickleBuffer* buf = new PickleBuffer();
  buf->marshalBegin();
  pickleTerm(val, buf, 0);
  buf->marshalEnd();

  for (int i = iterations; i--; ) {
    buf->unmarshalBegin();
    (void) unpickleTerm(buf);
    buf->unmarshalEnd();
  }

  // bug in 1.2.0 : just 'delete buf' doesn't delete the whole buffer;
  buf->saveBegin();
  int len;
  (void) buf->unlinkFirst(len);
  do {
    buf->chunkWritten();
  } while (buf->unlinkNext(len));
  buf->saveEnd();
  delete buf;

  return PROCEED;
} OZ_BI_end

// Oz value, number of iterations, buffer size (kb) --> total bytes written;
OZ_BI_define(BItestDPMarshaler, 3, 1)
{
  OZ_declareDetTerm(0, val);
  OZ_declareInt(1, iterations);
  OZ_declareInt(2, bufferSize);

  const int bbSize = 1024*bufferSize;
  BYTE *bb = new BYTE[bbSize];
  int bytes = 0;

  DPMarshaler *dpm = (DPMarshaler *) new DPMarshaler;
  ByteBuffer* buf = new ByteBuffer();
  buf->init(bbSize, bb);

  for (int i = iterations; i--; ) {
    buf->reinit();
    buf->marshalBegin();
    DPMarshaler *dpmC;
    if ((dpmC = dpMarshalTerm(val, buf, dpm, (DSite *) 0))) {
      do {
        buf->marshalEnd();
        bytes += buf->getUsed();
        buf->reinit();
        buf->marshalBegin();
      } while ((dpmC = dpMarshalContTerm(buf, dpmC)));
    }
    buf->marshalEnd();
    bytes += buf->getUsed();
    dpm->reset();
  }

  delete buf;
  delete dpm;
  delete bb;
  OZ_RETURN_INT(bytes);
} OZ_BI_end

// Oz value, number of iterations, buffer size (in kb);
OZ_BI_define(BItestDPUnmarshaler, 3, 0)
{
  OZ_declareDetTerm(0, val);
  OZ_declareInt(1, iterations);
  OZ_declareInt(2, bufferSize);

  const int bbSize = 1024*bufferSize;
  BYTE *bb = new BYTE[bbSize];
  int bytes = 0;

  DPMarshaler *dpm = (DPMarshaler *) new DPMarshaler;
  ByteBuffer* buf = new ByteBuffer();
  buf->init(bbSize, bb);
  Builder *b = new Builder;

  buf->reinit();
  buf->marshalBegin();
  if (dpMarshalTerm(val, buf, dpm, (DSite *) 0)) {
    return (TestMError("testDPUnmarshaler", "too large value!"));
  }
  buf->marshalEnd();

  for (int i = iterations; i--; ) {
    buf->unmarshalBegin();
    (void) dpUnmarshalTerm(buf, b);
    buf->unmarshalEnd();
  }

  return (PROCEED);
} OZ_BI_end

/*
 * also add this to e.g. modPickle.spec:
    'emptyBuiltin'      => { in     => [],
                              out    => [],
                              BI     => BIemptyBuiltin},
    'testMarshaler'     => { in     => ['+value', '+int'],
                              out    => [],
                              BI     => BItestMarshaler},
    'testUnmarshaler'   => { in     => ['+value', '+int'],
                              out    => [],
                              BI     => BItestUnmarshaler},
    'testDPMarshaler'   => { in     => ['+value', '+int', '+int'],
                             out    => ['int'],
                             BI     => BItestDPMarshaler},
    'testDPUnmarshaler' => { in     => ['+value', '+int', '+int'],
                             out    => [],
                             BI     => BItestUnmarshaler},
 */

#endif // defined(PROFILE_MARSHALER)
