#if defined(INTERFACE)
#pragma implementation "msl_endRouter.hh"
#endif


#include "msl_endRouter.hh"
#include "msl_comObj.hh"
#include "msl_msgContainer.hh"
#include "msl_dct.hh"
#include "msl_interRouter.hh"
#include "msl_timers.hh"
#include "msl_dsite.hh"

//#include "dssBase.hh"

#include <stdlib.h>  // for rand()

namespace _msl_internal{

  const int BYTE_ByteBuffer_CUTOFF=200;
  const int HEADER  = 11;  // msg header length
  const int DAC_HEADER = 54; // DAC msg header length

  //! marshal size is with 64 bytes less in order to avoid splitting the msg  
  const int E_BYTE_DEF_SIZE= 4000-HEADER-DAC_HEADER;

  //marshaling

  //int  g_OSWriteCounter= 0;
  //int  g_OSReadCounter = 0;
  //int  g_ContCounter   = 0;


  class EndRouterDeliver: public Event{
  private:
    EndRouter* er; 

    EndRouterDeliver(const EndRouterDeliver&):er(NULL){}
    EndRouterDeliver& operator=(const EndRouterDeliver&){ return *this; }
  public: 
    EndRouterDeliver(EndRouter *e):er(e){}
    void event_execute(MsgnLayerEnv* env){
      if (er)
	{
	  er->writeHandler(); 
	  er = NULL; 
	}
    }
    void m_makeGCpreps(){;} // Make sure the endrouter is NOT removed
    virtual ~EndRouterDeliver(){}
  };
  
  EndRouter::EndRouter(MsgnLayerEnv* env):
    TransObj(E_BYTE_DEF_SIZE,env),
    a_readBuffer(NULL),a_writeBuffer(NULL), a_minSend(E_MIN_FOR_HEADER),
    a_succ(NULL), a_routeId(0), deliverEvent(NULL){
    
    a_readBuffer = new DssReadByteBuffer(new BYTE[E_BYTE_DEF_SIZE],  E_BYTE_DEF_SIZE);
    a_writeBuffer= new DssWriteByteBuffer(new BYTE[E_BYTE_DEF_SIZE], E_BYTE_DEF_SIZE);

    a_comObj=NULL;
  }
  

  EndRouter::~EndRouter(){
    a_readBuffer->dispose_buf();
    a_writeBuffer->dispose_buf();
    delete a_readBuffer;
    delete a_writeBuffer;
    
  }
  
  
  VirtualChannelInterface* EndRouter::m_closeConnection(){
    dssLog(DLL_DEBUG,"EndRouter closing down");
    return NULL;
  }


  void EndRouter::deliver() {
    //printf("deliver !!!\n");
    Assert(a_succ != NULL); //Assert: the route has already been established
    if (!deliverEvent){
      deliverEvent = new EndRouterDeliver(this); 
      a_mslEnv->m_appendImmediateEvent(deliverEvent); 
    }
  }
  
  
  void EndRouter::readyToReceive() { ;
  // do somethig else !!! like setting a boolean or something
  }


  void EndRouter::initRouteSetUp(DSite* dsVec[], int nrSites) {
    Site* succ = static_cast<Site*>(dsVec[0]);

    setSuccessor(succ->m_getComObj());
  
  
    a_routeId = a_mslEnv->a_routeIds++; 
    //printf("setup route:%d\n", a_routeId);
  
    // we register a route at this site in order to be able to terminate the route
    (a_mslEnv->a_interRouter)->registerRoute(a_comObj->getSite(), a_mslEnv->a_mySite, a_routeId, a_comObj->getSite(), a_succ->getSite());
  
    MsgCnt *msgC=new MsgCnt(C_SET_ROUTE ,true);
    msgC->pushSiteVal(a_comObj->getSite());     // dst site reference 
    msgC->pushSiteVal(a_mslEnv->a_mySite);  // src site reference 
    msgC->pushIntVal(a_routeId);    // route id

    msgC->pushIntVal(nrSites-1); // intermediary router

    // push the sites 
    for(int i = 1; i < nrSites; i++) 
      msgC->pushSiteVal(static_cast<Site*>(dsVec[i])); // intermediary site 

    // destroy the site array here
    delete dsVec;

    //printf("send C_SET_ROUTE\n");
    a_succ->m_send(msgC,MSG_PRIO_EAGER);
  }

  
  void EndRouter::marshal(MsgCnt *msgC, int acknum) {
    bool cont=msgC->checkFlag(MSG_HAS_MARSHALCONT);
    a_writeBuffer->m_marshalBegin();
  
    dssLog(DLL_ALL,"TRANSOBJ      (%p): Marshal ack:%d cont:%d",
	   this,
	   msgC->checkFlag(MSG_HAS_MARSHALCONT)); 

    //printf ("endRouter::marshal msgType:%s\n", mess_names[msgC->getMessageType()]);
  
    a_writeBuffer->m_putByte(0xFF);          // Ctrl
    a_writeBuffer->m_putInt(acknum);     // Ack
    a_writeBuffer->m_putInt(0xFFFFFFFF); // Placeholder for framesize
    if (cont) {
      a_writeBuffer->m_putByte(CF_CONT);     // CF
      Assert(msgC->getMsgNum()!=NO_MSG_NUM);
      a_writeBuffer->m_putInt(msgC->getMsgNum());
    } else {
      Assert(msgC->getMsgNum()==NO_MSG_NUM);
      a_writeBuffer->m_putByte(CF_FIRST);
    }
  
    msgC->m_serialize(a_writeBuffer, a_comObj->getSite(), a_mslEnv);
  
    if(msgC->checkFlag(MSG_HAS_MARSHALCONT)) {
      a_mslEnv->a_ContCounter++;
      dssLog(DLL_DEBUG,"TRANSOBJ      (%p): Marshal continuation %d\n",
	     this,a_mslEnv->a_ContCounter);
      a_comObj->msgPartlySent(msgC);
      a_writeBuffer->m_putByte(CF_CONT);
    }
    else {
      a_comObj->msgSent(msgC);
      a_writeBuffer->m_putByte(CF_FINAL);
    }
    a_writeBuffer->m_marshalEnd();           // Size will be written now
  }


  // ZACHARIAS

  //
  // Below seems inefficient, why don't we marshal everything at once??
  //

  void EndRouter::writeHandler() {
    int len;
    BYTE *pos;
    MsgCnt *msgC;
    int acknum;
    deliverEvent = NULL; 
    while (a_writeBuffer->m_getUnused() > (E_MIN_FOR_HEADER) &&
	   (msgC=a_comObj->getNextMsgCnt(acknum))!=NULL) {
      marshal(msgC,acknum);

      a_writeBuffer->m_transform();
      // !! be careful to read all the message at once
      len = a_writeBuffer->m_getWriteParameters(pos);
      Assert(len > 0);
      a_mslEnv->a_OSWriteCounter++;

      DssSimpleDacDct *dac   = new DssSimpleDacDct();
      dac->putData(pos,len);

      //printf("EndRouter::writeHandler len:%d\n", len);
      msgC = new MsgCnt(C_ROUTE, true);
    
      msgC->pushSiteVal(a_comObj->getSite());     // dst site reference 
      msgC->pushSiteVal(a_mslEnv->a_mySite);  // src site reference 
      msgC->pushIntVal(a_routeId);   // route id
      msgC->pushDctVal(dac);         // data area containing the msg to route
    
      //printf("send C_ROUTE\n");
  

      a_succ->m_send(msgC, MSG_PRIO_EAGER);
      a_writeBuffer->m_hasWritten(len);
    }
  }


  unmarshalReturn EndRouter::unmarshal() {
    BYTE b;
    int acknum;
    int framesize;
    int cf;
    MsgCnt *msgC;
    int msgnum;
    int t;

    b=a_readBuffer->m_getByte();              // Ctrl
    Assert(b==0xFF);
    acknum=a_readBuffer->m_getInt();      // Ack
    a_comObj->msgAcked(acknum);
    framesize=a_readBuffer->m_getInt();   // Framesize

    //printf ("framesize %d\n", framesize);
    // ----------------------------------------- // Must read read
    if(a_readBuffer->m_canGet(framesize-MUSTREAD)) { // Can all be read?
      cf=a_readBuffer->m_getByte();          // CF

      //printf("endRouter::unmarshal type:%d msgType:%s\n", type, mess_names[type]);

      //
      if (cf == CF_FIRST) {
	msgC = a_comObj->getMsgCnt();
	Assert(!msgC->checkFlag(MSG_HAS_UNMARSHALCONT));
      } else {
	Assert(cf == CF_CONT);
	msgnum = a_readBuffer->m_getInt();   // MsgNr
	msgC = a_comObj->getMsgCnt(msgnum);
      }
      Assert(msgC != NULL);

      // Unmarshal data
      a_readBuffer->m_setFrameSize(framesize-TRAILER); // How much can be unm.
      if (msgC->deserialize(a_readBuffer, a_comObj->getSite(), a_mslEnv)) {
	// Frame contents successfully unmarshaled.
	t=a_readBuffer->m_getByte();
	a_readBuffer->m_commitReadOfData();
	if(t==CF_CONT) {
	  Assert(msgC->checkFlag(MSG_HAS_UNMARSHALCONT));
	  a_comObj->msgPartlyReceived(msgC);
	}
	else {
	  Assert(t==CF_FINAL);
	  Assert(!msgC->checkFlag(MSG_HAS_UNMARSHALCONT));
	  if(!a_comObj->msgReceived(msgC))
	    return U_CLOSED;
	}
	return U_MORE;
      } else {
	// Contents somehow corrupted.
	// Since messages have to be delivered in order we cannot just
	// discard this frame. Using TCP something must be seriously wrong.
	// Break the connection by telling the comObj it was lost. The comObj can
	// then decide on further actions.
	a_comObj->connectionLost();
	return U_CLOSED;
      }
    }
    else
      return U_WAIT;                       // Wait for more data
  } 


  //
  // THIS IMPLEMENTATION DOES NOT WORK WHEN:
  //
  // since we have now flow control data area containers will just be
  // forgotten if they can't deliver the entire payload
  //
  // typical fix: a list of unhandled/unfinished dacs connected to the endrouter,
  // the problem is pobably similar for inter-router

  // !! should also treat the case when the connection is lost
  void EndRouter::readHandler(DssSimpleDacDct *dac) {
    unsigned int len, ret;
    BYTE *pos;
    //int ctr = 0; int bctr=0;
    // START:
    len = static_cast<unsigned int>(a_readBuffer->m_getReadParameters(pos));
    do{ 
      //printf("EndRouter::readHandler dlen:%d, len:%d\n",dlen, len);
      ret = dac->getData(pos,len);    // write max len data to pos
      a_readBuffer->m_hasRead(ret);   // update buffer
    }while(ret == len && (len = a_readBuffer->m_getReadParameters(pos)) > 0);

    if(a_readBuffer->m_transform()){ // If the buffer is OK -> continue, otherwise close
    
      // Interpret (Allways interpret complete frames.)
      unmarshalReturn contin=U_MORE;
    
      a_readBuffer->m_unmarshalBegin();
      while(contin==U_MORE) {
	if(a_readBuffer->m_canGet(MUSTREAD))     // Includes previously read bytes
	  contin=unmarshal();
	else
	  break;
      }
      if(contin!=U_CLOSED)    // transobj could be passed on
	a_readBuffer->m_unmarshalEnd();   
    } else {
      Assert(0);
      printf(" ERROR IN ROUTE\n");
      a_comObj->m_closeErroneousConnection();
    }
  }
  
  

    
  void EndRouter::routeSetUp(int routeId) {
    ;//printf("the route:%d routeId is set up\n", routeId); //! more to come ...
  }

  void
  EndRouter::m_EncryptReadTransport(BYTE* const key, const u32& keylen,
				    const u32& iv1,  const u32& iv2){
    //printf("ROUTE-TRANSPORT IS GOING RCRYPTO\n");
    DssCryptoReadByteBuffer* tmp = new DssCryptoReadByteBuffer(key, keylen, iv1, iv2, a_readBuffer);
    delete a_readBuffer;
    a_readBuffer = tmp;
  };

  void
  EndRouter::m_EncryptWriteTransport(BYTE* const key, const u32& keylen,
				     const u32& iv1,  const u32& iv2){    
    //printf("ROUTE-TRANSPORT IS GOING WCRYPTO\n");
    DssCryptoWriteByteBuffer* tmp = new DssCryptoWriteByteBuffer(key,keylen, iv1,iv2,a_writeBuffer);
    delete a_writeBuffer;
    a_writeBuffer = tmp;
  };

 


} //End namespace
