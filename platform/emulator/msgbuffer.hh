/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
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

#ifndef __MSGBUFFER_HH
#define __MSGBUFFER_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "tagged.hh"
#include "comm.hh"

#define MSGFLAG_TEXTMODE  0x1
#define MSGFLAG_OLDFORMAT 0x2

class MsgBuffer {
private:
  OZ_Term resources, nogoods;
  int flags;

  //
protected:
  BYTE* posMB;
  BYTE* endMB;

  virtual BYTE getNext()=0;
  virtual void putNext(BYTE)=0;

  //
public:
  virtual void marshalBegin() = 0;
  virtual void marshalEnd()=0;
  virtual void unmarshalBegin()=0;
  virtual void unmarshalEnd()=0;

  void init() {
    resources = oz_nil();
    nogoods   = oz_nil();
    flags     = 0;
  }

  void setTextmode() { flags |= MSGFLAG_TEXTMODE; }
  Bool textmode()    { return (flags&MSGFLAG_TEXTMODE); }

  void setOldFormat() { flags |= MSGFLAG_OLDFORMAT; }
  Bool oldFormat()    { return (flags&MSGFLAG_OLDFORMAT); }

  // NON-virtual!
  BYTE get(){
    if(posMB==endMB){
      return getNext();}
    PD((MARSHAL_CT,"one char got c:%d",*posMB));
    return *posMB++;}

  void put(BYTE b){
    if(posMB>endMB){
      putNext(b);
      return;}
    *posMB++=b;
    PD((MARSHAL_CT,"one char put c:%d p:%d",b,*posMB));
  }

  //
  virtual char* siteStringrep()=0;
  virtual Site* getSite()=0;                    // overrided for network/vsite comm
  virtual Bool isPersistentBuffer()=0;
  virtual void unmarshalReset()                 {} // only for network receovery

  void addRes(OZ_Term t)    { resources = oz_cons(t,resources); }
  OZ_Term getResources()    { return resources; }
  void addNogood(OZ_Term t) { nogoods = oz_cons(t,nogoods); }
  OZ_Term getNoGoods()      { return nogoods; }
};

MsgBuffer* getComponentMsgBuffer();
MsgBuffer* getRemoteMsgBuffer(Site *);
MsgBuffer* getVirtualMsgBuffer(Site *);
void dumpRemoteMsgBuffer(MsgBuffer*);
void dumpVirtualMsgBuffer(MsgBuffer*);

class MsgBufferManager{
public:
  MsgBuffer* getMsgBuffer(Site * s){
    if(s){
      if(s->remoteComm()){
        return getRemoteMsgBuffer(s);}
      return getVirtualMsgBuffer(s);}
    return getComponentMsgBuffer();}

  void dumpMsgBuffer(MsgBuffer *m){ // only for marshaled/write stuff (not read)
    Site *s=m->getSite();
    if(s->remoteComm()){
      dumpRemoteMsgBuffer(m);}
    else{
      Assert(s!=NULL);
      dumpVirtualMsgBuffer(m);}}
};

extern MsgBufferManager *msgBufferManager;

/* RS: have to GC the byte stream again !!!!!!!!!*/
#define CheckNogoods(val,bs,msg,Cleanup)                                \
  { OZ_Term nogoods = bs->getNoGoods();                                 \
    if (!oz_isNil(nogoods)) {                                           \
       Cleanup;                                                         \
       return raiseGeneric(msg,                                         \
                           oz_mklist(OZ_pairA("Resources",nogoods),     \
                                     OZ_pairA("Contained in",val)));    \
    }                                                                   \
  }

#endif // __MSGBUFFER_HH
