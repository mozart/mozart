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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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

#include "perdio.hh"
#include "comm.hh"

class MsgBuffer {
public:
  OZ_Term resources, nogoods;

  BYTE* posMB;
  BYTE* endMB;
  virtual void marshalBegin() = 0;
  virtual void marshalEnd()=0;
  virtual void unmarshalBegin()=0;
  virtual void unmarshalEnd()=0;
  virtual BYTE getNext()=0;
  virtual void putNext(BYTE)=0;

  void init() {
    resources = nil();
    nogoods   = nil();
  }
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


  virtual char* siteStringrep()=0;
  virtual Site* getSite()=0;                    // overrided for network/vsite comm
  virtual Bool isPersistentBuffer()=0;
  virtual void unmarshalReset()                 {} // only for network receovery

  void addRes(OZ_Term t)    { resources = cons(t,resources); }
  void addNogood(OZ_Term t) { nogoods = cons(t,nogoods); }
  OZ_Term getNoGoods() { return nogoods; }
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

#endif // __MSGBUFFER_HH
