#ifdef INTERFACE  
#pragma interface
#endif

class MsgBuffer{
public:
  virtual void marshalBegin() = 0;	
  virtual void marshalEnd()=0;
  virtual void unmarshalBegin()=0;
  virtual void unmarshalEnd()=0;
  virtual BYTE get()=0;	 
  virtual void put(BYTE)=0;
  virtual char* siteStringrep()=0;
  virtual Site* getSite()=0;                    // overrided for network/vsite comm
  virtual void addRes(OZ_Term)                  {} // only for load/save - noop for rest
  virtual void addURL(OZ_Term)                  {} // only for load/save - noop for rest
  virtual Bool saveAnyway(OZ_Term)              {return NO;} // only for load/save - returns NO for rest
  virtual void marshaledProcHasNames(TaggedRef) {} // only for load/save - noop for rest
  virtual void gnameMark(GName* gn)             {gn->markURL(0);} // only otherwise for load/save 
  virtual Bool knownAsNewName(OZ_Term)          {return NO;} // only for load/save - returns NO for rest
};

MsgBuffer* getComponentMsgBuffer();
MsgBuffer* getRemoteMsgBuffer(Site *);
MsgBuffer* getVirtualMsgBuffer(Site *);

class MsgBufferManager{
public:
  MsgBuffer* getMsgBuffer(Site * s){
    if(s){
      if(s->remoteComm()){
	return getRemoteMsgBuffer(s);}
      return getVirtualMsgBuffer(s);}
    return getComponentMsgBuffer();}
};

extern MsgBufferManager *msgBufferManager;


