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
#ifdef INTERFACE
#pragma interface
#endif

class MsgBuffer{
public:
  Bool unmarshallingOld;
  BYTE* posMB;
  BYTE* endMB;
  virtual void marshalBegin() = 0;
  virtual void marshalEnd()=0;
  virtual void unmarshalBegin()=0;
  virtual void unmarshalEnd()=0;
  virtual BYTE getNext()=0;
  virtual void putNext(BYTE)=0;

  void init() { unmarshallingOld=NO; }
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
  virtual void addRes(OZ_Term)                  {} // only for load/save - noop for rest
  virtual void addURL(OZ_Term)                  {} // only for load/save - noop for rest
  virtual Bool saveAnyway(OZ_Term)              {return NO;} // only for load/save - returns NO for rest
  virtual void marshaledProcHasNames(TaggedRef) {} // only for load/save - noop for rest
  virtual Bool knownAsNewName(OZ_Term)          {return NO;} // only for load/save - returns NO for rest
  virtual void unmarshalReset()                 {} // only for network receovery

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
