/*
 *  Authors:
 *    Erik Klintskog(erik@sics.se)
 * 
 *  Contributors:
 * 
 *  Copyright:
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
 
#if defined(INTERFACE)
#pragma implementation "dss_msgLayerInterface.hh"
#endif


#include "dss_comService.hh"
#include "dssBase.hh"
#include "coordinator.hh"
#include "dss_msgLayerInterface.hh"
#include "dss_threads.hh"
#include <string.h>

namespace _dss_internal{
  
  void  
  DssMslClbk::m_noDestProxy2Proxy(MsgContainer *msgC, DSite *sender){
    MsgContainer *msgCret = m_getEnvironment()->a_msgnLayer->createAppSendMsgContainer();
    msgCret->pushIntVal(M_PROXY_PROXY_NODEST);
    msgCret->pushMsgC(msgC->reincarnate());
    sender->m_sendMsg(msgCret);
   }
  
  
  void  
  DssMslClbk::m_noDestProxy2Coord(MsgContainer *msgC, DSite *sender){
    MsgContainer *msgCret = m_getEnvironment()->a_msgnLayer->createAppSendMsgContainer();
    msgCret->pushIntVal(M_PROXY_COORD_NODEST);
    msgCret->pushMsgC(msgC->reincarnate());
    sender->m_sendMsg(msgCret);
  }


  void  
  DssMslClbk::m_noDestCoord2Proxy(MsgContainer *msgC, DSite *sender){
    MsgContainer *msgCret = m_getEnvironment()->a_msgnLayer->createAppSendMsgContainer();
    msgCret->pushIntVal(M_COORD_PROXY_NODEST);
    msgCret->pushMsgC(msgC->reincarnate());
    sender->m_sendMsg(msgCret);
  }

  void  
  DssMslClbk::m_noDestCoord2Coord(MsgContainer *msgC, DSite *sender){
    MsgContainer *msgCret = m_getEnvironment()->a_msgnLayer->createAppSendMsgContainer();
    msgCret->pushIntVal(M_COORD_COORD_NODEST);
    msgCret->pushMsgC(msgC->reincarnate());
    sender->m_sendMsg(msgCret);
  }
  
  void   
  DssMslClbk::m_MessageReceived(::MsgContainer* const msgC,DSite* const sender){
    MessageType mt = static_cast<MessageType>(msgC->popIntVal());
    dssLog(DLL_BEHAVIOR,"RECEIVE: %p, %d message from %p",
	   m_getEnvironment()->a_myDSite, mt, sender);
    switch (mt) {
    case M_PROXY_COORD_PROTOCOL:
      {
	NetIdentity ni = gf_popNetIdentity(msgC); 
	Coordinator *me = m_getEnvironment()->a_coordinatorTable->m_find(ni);
	if (me) 
	  me->m_receiveProtMsg(msgC,sender);
	else 
	  m_noDestProxy2Coord(msgC, sender);
	break;
      }
    case M_PROXY_PROXY_PROTOCOL:
    case M_COORD_PROXY_PROTOCOL:
      {
	NetIdentity ni = gf_popNetIdentity(msgC); 
	Proxy *pe   = m_getEnvironment()->a_proxyTable->m_find(ni);
	
	if (pe) pe->m_receiveProtMsg(msgC,sender);
	else {
	  if (mt == M_PROXY_PROXY_PROTOCOL) m_noDestProxy2Proxy(msgC, sender);
	  else m_noDestCoord2Proxy(msgC, sender);
	}
	break;
      }
      
    case M_PROXY_COORD_REF:
      {
	NetIdentity ni = gf_popNetIdentity(msgC); 
	Coordinator *me = m_getEnvironment()->a_coordinatorTable->m_find(ni);
	if (me) me->m_receiveRefMsg(msgC,sender);
	else 
	  m_noDestProxy2Coord(msgC, sender);
	break;
      }
    case M_COORD_PROXY_REF:
    case M_PROXY_PROXY_REF:
      {
	NetIdentity ni = gf_popNetIdentity(msgC); 
	Proxy *pe   = m_getEnvironment()->a_proxyTable->m_find(ni);
	if (pe) pe->m_receiveRefMsg(msgC,sender);
	else {
	  if (mt == M_PROXY_PROXY_REF) m_noDestProxy2Proxy(msgC, sender);
	  else m_noDestCoord2Proxy(msgC, sender);
	}
	break;
      }
    case M_PROXY_COORD_CNET:
    case M_COORD_COORD_CNET:
      {
	NetIdentity ni = gf_popNetIdentity(msgC); 
	Coordinator *me = m_getEnvironment()->a_coordinatorTable->m_find(ni);
	if(me) me->m_receiveAsMsg(msgC, sender);
	else {
	  if (mt == M_PROXY_COORD_CNET) m_noDestProxy2Coord(msgC, sender);
	  else m_noDestCoord2Coord(msgC, sender);
	}
	break;
      }
    case M_PROXY_PROXY_CNET:
    case M_COORD_PROXY_CNET:
      {
	NetIdentity ni = gf_popNetIdentity(msgC); 
	Proxy *pe   = m_getEnvironment()->a_proxyTable->m_find(ni);
	if(pe) pe->m_receiveAsMsg(msgC, sender);
	else {
	  if (mt == M_PROXY_PROXY_CNET) m_noDestProxy2Proxy(msgC, sender);
	  else m_noDestCoord2Proxy(msgC, sender);
	}
	break;
      }
    case M_PROXY_CNET:
      {
	NetIdentity ni = gf_popNetIdentity(msgC); 
	Proxy *pe   = m_getEnvironment()->a_proxyTable->m_find(ni);
	if(pe) pe->m_receiveAsMsg(msgC, sender);	
	break; 
      }
    case M_COORD_CNET:
      {
	NetIdentity ni = gf_popNetIdentity(msgC); 
	Coordinator *me = m_getEnvironment()->a_coordinatorTable->m_find(ni);
	if(me) me->m_receiveAsMsg(msgC, sender);
	break; 
      }
    case M_PROXY_PROXY_NODEST:
      { 
	MsgContainer * msg = msgC->popMsgC(); 
	msg->popIntVal();     // remove msl tag
	MessageType mtt = static_cast<MessageType>(msg->popIntVal());
	NetIdentity ni = gf_popNetIdentity(msg);
	Proxy *pe   = m_getEnvironment()->a_proxyTable->m_find(ni);
	if (pe) pe->m_noProxyAtDest(sender, mtt, msg);
	break; 
      }
    case M_COORD_PROXY_NODEST:
      { 
	MsgContainer * msg = msgC->popMsgC(); 
	msg->popIntVal();     // remove msl tag
	MessageType mtt = static_cast<MessageType>(msg->popIntVal());
	NetIdentity ni = gf_popNetIdentity(msg);
	Coordinator *me = m_getEnvironment()->a_coordinatorTable->m_find(ni);
	if(me) me->m_noProxyAtDest(sender, mtt, msg); 
	break; 
      }
    case M_PROXY_COORD_NODEST:
      { 
	MsgContainer * msg = msgC->popMsgC(); 
	msg->popIntVal();     // remove msl tag
	MessageType mtt = static_cast<MessageType>(msg->popIntVal());
	NetIdentity ni = gf_popNetIdentity(msg);
	Proxy *pe   = m_getEnvironment()->a_proxyTable->m_find(ni);
	if (pe) pe->m_noCoordAtDest(sender, mtt, msg);
	break; 
      }
    case M_COORD_COORD_NODEST:
      { 
	MsgContainer * msg = msgC->popMsgC(); 
	msg->popIntVal();     // remove msl tag
	MessageType mtt = static_cast<MessageType>(msg->popIntVal());
	NetIdentity ni = gf_popNetIdentity(msg);
	Coordinator *me = m_getEnvironment()->a_coordinatorTable->m_find(ni);
	if(me) me->m_noCoordAtDest(sender, mtt, msg); 
	break; 
      }
    default:
      m_getEnvironment()->a_map->GL_error("siteReceive: unknown message %d",mt);
      break;
    }

    dssLog(DLL_BEHAVIOR,"RECEIVE: Message handled (%d) from %x",mt,sender);
  }
  
  
  void
  DssMslClbk::m_stateChange(DSite* s, const FaultState& state){
    m_getEnvironment()->a_proxyTable->m_siteStateChange(s, state); 
    m_getEnvironment()->a_coordinatorTable->m_siteStateChange(s, state); 
  }
  
  void 
  DssMslClbk::m_unsentMessages(DSite* s, MsgContainer* msgs){
    MsgContainer *tmp;
    while(msgs!=NULL)
      {
	tmp = msgs->m_getNext(); 
	delete msgs; 
	msgs = tmp; 
      }
  }
  
  DssMslClbk::DssMslClbk(DSS_Environment *env):DSS_Environment_Base(env){
    ;
  }
  DssMslClbk::~DssMslClbk(){;}

  ExtDataContainerInterface*   
  DssMslClbk::m_createExtDataContainer(BYTE type){
    switch(type){
    case ADCT_PST:
      {
	return new PstContainer(m_getEnvironment()); 
      }
    case ADCT_PDC:
      {
	return new PstDataContainer(m_getEnvironment()); 
      }
    case ADCT_EBA:
      {
	return new EdcByteArea(NULL); 
      }
      
    default:
      dssError("Unknown appDataContainer type %d\n", type); 
    } 
    return NULL; 
}
    

    enum {
      ADCT_EMPTY,
      ADCT_FILLED
    };
    
  BYTE   PstContainer::getType(){
      return ADCT_PST; 
    }
    bool PstContainer:: marshal(DssWriteBuffer *bb){
      if(a_pstOut == NULL)
	{
	  bb->putByte(ADCT_EMPTY);
	  return true;
	}
      else
	{
	  Assert(a_pstOut!=reinterpret_cast<PstOutContainerInterface*>(0xbedda));
	  bb->putByte(ADCT_FILLED); 
	  return a_pstOut->marshal(bb); 
	  
	}
    }
    bool  PstContainer::unmarshal(DssReadBuffer *bb){
      BYTE type = bb->getByte(); 
      if(type == ADCT_EMPTY){
	a_pstIn = NULL; 
	return true; 
      }
      if (a_pstIn == NULL)
	a_pstIn = m_getEnvironment()->a_map->createPstInContainer();
      return a_pstIn->unmarshal(bb); 
    }
    // dispose pst in/out containers, and delete this one
    void  PstContainer::dispose(){
      if (a_pstIn) a_pstIn->dispose();
      if (a_pstOut) a_pstOut->dispose();
      delete this;
    }
    void  PstContainer::resetMarshaling(){
      if (a_pstOut) a_pstOut->resetMarshaling();
    }

    PstOutContainerInterface**  
    PstContainer::getPstOutContainerHandle(){
      a_pstOut = reinterpret_cast<PstOutContainerInterface*>(0xbedda);
      return &a_pstOut; 
    }
    
    PstContainer::PstContainer(DSS_Environment * env,PstOutContainerInterface* po):
      DSS_Environment_Base(env), a_pstOut(po), a_pstIn(NULL)
    {}
    
    PstContainer::PstContainer(DSS_Environment *env):
      DSS_Environment_Base(env), a_pstOut(NULL), a_pstIn(NULL)
    {}
  
    PstInContainerInterface*
    PstContainer::m_getPstIn(){
      if (a_pstIn) 
	return a_pstIn; 
      if (a_pstOut) {
	a_pstIn = a_pstOut->loopBack2In(); // keep track of it (for GC)
	return a_pstIn;
      }
      return NULL; 
    }

    PstOutContainerInterface*
    PstContainer::m_getPstOut(){
      return a_pstOut; 
    }



  void gf_pushPstOut(::MsgContainer* msg, PstOutContainerInterface* out){
    PstContainer* pc = new PstContainer(NULL, out); 
    msg->pushADC(pc); 
  }
  PstInContainerInterface* gf_popPstIn(::MsgContainer* msg){
    PstContainer* pc = static_cast<PstContainer*>(msg->popADC()); 
    return pc->m_getPstIn();
  }
  PstOutContainerInterface** gf_pushUnboundPstOut(::MsgContainer* msg){
    PstContainer* pc = new PstContainer(NULL); 
    msg->pushADC(pc); 
    return pc->getPstOutContainerHandle(); 
  }


  // ******************* PstDataContainer ************************
  // RefCntdBuffer* a_buffer; 
  // BYTE*          a_curr; 
  

  
  
  size_t
  InfiniteWriteBuffer::availableSpace() const {
    return 10000; // he, he, he
  }
  void 
  InfiniteWriteBuffer::writeToBuffer(const BYTE* ptr, size_t inlen){
    int availableSize = a_curBuf->a_end - a_cur; 
    int len = static_cast<int>(inlen);  
    if(availableSize < len){
      SimpleBlockBuffer *old = a_curBuf; 
      int size = a_curBuf->a_end - a_curBuf->a_vec; 
      int newSize = size + t_max(len + 1000 , size); 
      printf("creating new block size:%d\n",  newSize); 
      a_curBuf = new SimpleBlockBuffer(newSize); 
      int cpSize = a_cur - old->a_vec; 
      printf("copying data to new size:%d\n", cpSize); 
      memcpy(a_curBuf->a_vec,old->a_vec,cpSize);
      a_cur = a_curBuf->a_vec+cpSize; 
      delete old; 
      writeToBuffer(ptr,len); 
    }else{
      memcpy(a_cur,ptr,len);
      a_cur +=len;
    }
    
  }
  void 
  InfiniteWriteBuffer::putByte(const BYTE& b){
    writeToBuffer(&b, 1); 
  }
  
  InfiniteWriteBuffer::InfiniteWriteBuffer():a_curBuf(NULL), a_cur(NULL){
    a_curBuf = new SimpleBlockBuffer(1000); 
    a_cur = a_curBuf->a_vec; 
  }
  
  InfiniteWriteBuffer::~InfiniteWriteBuffer(){
    ;
  }
    
  SimpleBlockBuffer*
  InfiniteWriteBuffer::m_getBuffer(){
    printf("tot size %d\n", a_cur - a_curBuf->a_vec); 
    a_curBuf->a_end = a_cur; 
    return a_curBuf; 
  }
    
  
  class RefCntdBuffer{
    int a_refCnt; 
    static int allocated; 
    SimpleBlockBuffer* a_buf; 
    PstOutContainerInterface* a_out;
  public: 

    RefCntdBuffer(int sz):
      a_refCnt(1), a_buf(new SimpleBlockBuffer(sz)), a_out(NULL)
    {
      ;
    }

    RefCntdBuffer(PstOutContainerInterface**& out):
      a_refCnt(1), a_buf(NULL), a_out(reinterpret_cast<PstOutContainerInterface*>(0xbedda))
    {
      printf("Creating rcb:%p tot:%d\n", this, ++allocated); 
      out = &a_out; 
    }

    ~RefCntdBuffer(){
      printf("Deleteing rcb:%p tot:%d\n", this, --allocated); 
      if(a_buf) delete a_buf; 
      a_buf = NULL; 
    }
    
    
    SimpleBlockBuffer* m_getBuffer(){
      if(a_buf == NULL)
	{
	  InfiniteWriteBuffer *iw = new InfiniteWriteBuffer(); 
	  DebugCode(bool done =)
	    a_out->marshal(iw);
	  Assert(done); 
	  a_buf = iw->m_getBuffer();  
	  printf("marshalDone size:%d\n", a_buf->a_end- a_buf->a_vec); 
	  
	  delete iw; 
	}
      return a_buf; 
    }
    
    void m_addRef(){ a_refCnt++;}
    void m_delRef(){ if(--a_refCnt == 0) delete this;}
  };
  
  

  
  int RefCntdBuffer::allocated = 0; 
  
  

  
  class ReadBlockBuffer: public DssReadBuffer{
    BYTE*          a_cur; 
    BYTE*          a_end; 
  public: 
    ReadBlockBuffer(SimpleBlockBuffer* b):a_cur(b->a_vec), a_end(b->a_end){
      ;
    }
    ~ReadBlockBuffer(){;}

    virtual size_t availableData() const{
      return a_end - a_cur; 
    }
    virtual bool canRead(size_t len) const{
      Assert(a_end >= a_cur);
      return static_cast<size_t>(a_end - a_cur)>=len;
    }
    virtual void readFromBuffer(BYTE* ptr, size_t len){
      memcpy(ptr,a_cur,len);
    }
    virtual void commitRead(size_t read){
      a_cur += read; 
    }
    virtual const BYTE   getByte(){
      return *a_cur++;
    }
  };
  
  
  // Called when creating a completly new container
  PstDataContainer::PstDataContainer(DSS_Environment* env, PstOutContainerInterface** &pst):
    DSS_Environment_Base(env),a_cntdBuf(new RefCntdBuffer(pst)), a_cur(NULL){
  }

  // Called when creating a replica
  PstDataContainer::PstDataContainer(DSS_Environment* env, RefCntdBuffer* buf):
    DSS_Environment_Base(env),  a_cntdBuf(buf),a_cur(NULL){
    buf->m_addRef();
  }

  // Called when unmarshaling a container
  PstDataContainer::PstDataContainer(DSS_Environment* env): 
    DSS_Environment_Base(env), a_cntdBuf(NULL), a_cur(NULL){
    ;
  }
  
  PstDataContainer::~PstDataContainer(){
    if(a_cntdBuf)
      a_cntdBuf->m_delRef();
    a_cntdBuf = NULL; 
  }
  
  PstInContainerInterface* PstDataContainer::m_getPstIn(){
    // raph: this operation might leak memory (the returned
    // PstInContainerInterface will not be disposed).
    Assert(0);
    PstInContainerInterface* pstIn = m_getEnvironment()->a_map->createPstInContainer();
    ReadBlockBuffer rb(a_cntdBuf->m_getBuffer()); 
    pstIn->unmarshal(&rb);
    return pstIn; 
}
  
  PstDataContainer*      PstDataContainer::m_createReplica(){
    return new PstDataContainer(m_getEnvironment(), a_cntdBuf); 
  }

  BYTE  PstDataContainer::getType(){
    return ADCT_PDC;
  }

  bool PstDataContainer::marshal(DssWriteBuffer *bb){
    // We could potentially check the value of the pst pointer
    // if set to a dummy value, we could suspend serialization and 
    // continue when teh pst is completley filled in.... Coool!!! 
    SimpleBlockBuffer *buf = a_cntdBuf->m_getBuffer();
    if (a_cur == NULL){
      a_cur = buf->a_vec; 
      gf_MarshalNumber(bb, buf->a_end-buf->a_vec);
      printf("marshaling, RCB totSize %d\n", buf->a_end-buf->a_vec); 
    }
    
    int marshalSize =  t_min(bb->availableSpace() - 40, static_cast<size_t>(buf->a_end- a_cur));
    printf("marshaling, RCB blockSize %d\n", marshalSize); 
    gf_MarshalNumber(bb, marshalSize); 
    bb->writeToBuffer(a_cur, marshalSize); 
    a_cur+=marshalSize;
    return a_cur == buf->a_end;  
  }    
  
  bool PstDataContainer::unmarshal(DssReadBuffer *bb){
    if(a_cur == NULL){
      int size = gf_UnmarshalNumber(bb);
      printf("unmarshaling, RCB totSize %d\n", size); 
      a_cntdBuf = new RefCntdBuffer(size); 
      a_cur = a_cntdBuf->m_getBuffer()->a_vec; 
    }
    int marshalSize = gf_UnmarshalNumber(bb);
    printf("unmarshaling, RCB blockSize %d\n", marshalSize); 
    bb->readFromBuffer(a_cur, marshalSize); 
    bb->commitRead(marshalSize);
    a_cur += marshalSize; 
    return  (a_cur ==  a_cntdBuf->m_getBuffer()->a_end) ;
  }
  
  void PstDataContainer::dispose(){
    delete this; 
  }
  void PstDataContainer::resetMarshaling(){
    a_cur = NULL; 
  }


  // ********************** EdcByteArea **********************
  EdcByteArea::EdcByteArea(SimpleBlockBuffer *bb):a_buffer(bb), a_cur(NULL){;}
  
  DssReadBuffer *EdcByteArea::m_getReadBufInterface(){
    return new ReadBlockBuffer(a_buffer); 
  }
  
  BYTE  EdcByteArea::getType(){
    return ADCT_EBA;
  }

  bool EdcByteArea::marshal(DssWriteBuffer *bb){
    if (a_cur == NULL){
      a_cur = a_buffer->a_vec; 
      gf_MarshalNumber(bb, a_buffer->a_end - a_buffer->a_vec);
      printf("marshaling, EBA totSize %d\n", a_buffer->a_end - a_buffer->a_vec); 
    }
    size_t marshalSize = (bb->availableSpace() > 40 ?
			  t_min(bb->availableSpace() - 40,
				static_cast<size_t>(a_buffer->a_end- a_cur)) :
			  0);
    printf("marshaling, EBC blockSize %zu\n", marshalSize); 
    gf_MarshalNumber(bb, marshalSize); 
    bb->writeToBuffer(a_cur, marshalSize); 
    a_cur+=marshalSize;
    return a_cur == a_buffer->a_end;  
  }    
  bool EdcByteArea::unmarshal(DssReadBuffer *bb){
    
    if(a_cur == NULL){
      int size = gf_UnmarshalNumber(bb);
      printf("unmarshaling, EBC totSize %d\n", size); 
      a_buffer = new SimpleBlockBuffer(size); 
      a_cur = a_buffer->a_vec; 
    }
    int marshalSize = gf_UnmarshalNumber(bb);
    printf("unmarshaling, EBA blockSize %d\n", marshalSize); 
    bb->readFromBuffer(a_cur, marshalSize); 
    bb->commitRead(marshalSize);
    a_cur += marshalSize; 
    return  (a_cur ==  a_buffer->a_end) ;
  }
    
  void EdcByteArea::dispose(){
    delete a_buffer; 
  }
  void EdcByteArea::resetMarshaling(){
    a_cur = NULL; 
  }
  


  
  void gf_pushEBA(MsgContainer* msg, EdcByteArea* eba){
    msg->pushADC(eba);
  }
  
  EdcByteArea* gf_popEBA(MsgContainer* msg){
    return static_cast<EdcByteArea*>(msg->popADC());
  }

// ********************** PURE INTS *******************************

  void gf_createSndMsg(MsgContainer *msgC, 
		    int i1)
  {
    msgC->pushIntVal(i1);
  }
  void gf_createSndMsg(MsgContainer *msgC, 
		    int i1, int i2)
  {
    msgC->pushIntVal(i1);
    msgC->pushIntVal(i2);
  }
  void gf_createSndMsg(MsgContainer *msgC,  
		    int i1, int i2, int i3)
  {
    msgC->pushIntVal(i1);
    msgC->pushIntVal(i2);
    msgC->pushIntVal(i3);
  }


  // ********************* INTS & SITES ******************************

  void gf_createSndMsg(MsgContainer *msgC, 
		    DSite *s)
  {
    msgC->pushDSiteVal(s);
  } 

  void gf_createSndMsg(MsgContainer *msgC,  
		    int i1, DSite *s)
  {
    msgC->pushIntVal(i1);
    msgC->pushDSiteVal(s);
  } 
  void gf_createSndMsg(MsgContainer *msgC,  
		    int i1, int i2, DSite *s)
  {
    msgC->pushIntVal(i1);
    msgC->pushIntVal(i2);
    msgC->pushDSiteVal(s);
  } 
  void gf_createSndMsg(MsgContainer *msgC,  
		    int i1, int i2, int i3, DSite *s)
  {
    msgC->pushIntVal(i1);
    msgC->pushIntVal(i2);
    msgC->pushIntVal(i3);
    msgC->pushDSiteVal(s);
  } 

  // ********************** INTS & PSTS *******************************

  void gf_createSndMsg(MsgContainer *msgC, 
		    PstOutContainerInterface *pst)
  {
    gf_pushPstOut(msgC, pst);
  } 

  void gf_createSndMsg(MsgContainer *msgC,  
		    int i1, PstOutContainerInterface *pst)
  {
    msgC->pushIntVal(i1);
    gf_pushPstOut(msgC, pst);
  } 
  void gf_createSndMsg(MsgContainer *msgC,  
		    int i1, int i2, PstOutContainerInterface *pst)
  {
    msgC->pushIntVal(i1);
    msgC->pushIntVal(i2);
    gf_pushPstOut(msgC, pst);
  } 
  void gf_createSndMsg(MsgContainer *msgC,  
		    int i1, int i2, int i3, PstOutContainerInterface *pst)
  {
    msgC->pushIntVal(i1);
    msgC->pushIntVal(i2);
    msgC->pushIntVal(i3);
    gf_pushPstOut(msgC, pst);
  }
  // *********************** THR & Int *********************************
  
  void gf_createSndMsg(MsgContainer *msgC,  
		       GlobalThread* th){

    Assert(0);
    //msgC->pushThreadIdVal(th);
  }

  void gf_createSndMsg(MsgContainer *msgC,  
		       int i1, GlobalThread* th){

    msgC->pushIntVal(i1); 
    Assert(0);
    //msgC->pushThreadIdVal(th);
  }

  void gf_createSndMsg(MsgContainer *msgC,  
		       int i1, int i2, GlobalThread* th){

    msgC->pushIntVal(i1); 
    msgC->pushIntVal(i2); 
    Assert(0);
    //msgC->pushThreadIdVal(th);
  }
  


  // *********************** THR & PSTS *********************************
  
  void gf_createSndMsg(MsgContainer *msgC,  
		       GlobalThread* th, PstOutContainerInterface *pst){
    
    Assert(0);
    //msgC->pushThreadIdVal(th);
    gf_pushPstOut(msgC, pst);
  }
  
  // *********************** THR & PSTS & INT *********************************

  void gf_createSndMsg(MsgContainer *msgC,  
		       int i1, GlobalThread* th, PstOutContainerInterface *pst){
    msgC->pushIntVal(i1);
    Assert(0);
    //    msgC->pushThreadIdVal(th);
    gf_pushPstOut(msgC, pst);
  }


  void gf_createSndMsg(MsgContainer *msgC,  
		       int i1, int i2, GlobalThread* th, PstOutContainerInterface *pst){
    msgC->pushIntVal(i1);
    msgC->pushIntVal(i2);
    Assert(0);
    
    //msgC->pushThreadIdVal(th);
    gf_pushPstOut(msgC, pst);
  }

}


