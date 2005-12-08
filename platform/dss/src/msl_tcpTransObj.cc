/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
 *    Erik Klintskog (erik@sics.se)
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
#pragma implementation "msl_tcpTransObj.hh"
#endif

#include "msl_tcpTransObj.hh"
#include "msl_comObj.hh"
#include "msl_msgContainer.hh"
#include "dss_comService.hh"
#include "mslBase.hh"
#include "msl_buffer.hh"
namespace _msl_internal{

  // const int HEADER  =11;
  namespace{    
    const int T_BYTE_DEF_SIZE=64000;
  }

  
#ifdef DEBUG_CHECK
  int TCPTransObj::a_allocated=0;
#endif  
  
  TCPTransObj::TCPTransObj(MsgnLayerEnv* env):
    TransObj(T_BYTE_DEF_SIZE,env),
    a_minSend(T_MIN_FOR_HEADER),
    a_readBuffer(NULL),
    a_writeBuffer(NULL),
    a_channel(NULL){
    BYTE* r = new BYTE[T_BYTE_DEF_SIZE];
    BYTE* w = new BYTE[T_BYTE_DEF_SIZE];
    a_readBuffer = new DssReadByteBuffer(r , T_BYTE_DEF_SIZE);
    a_writeBuffer= new DssWriteByteBuffer(w, T_BYTE_DEF_SIZE);
    
    DebugCode(a_allocated++;); // Count ALL TCP trans objects (for all Environments)
  }

  
  TCPTransObj::~TCPTransObj(){
    // The destructor of the buffers will delete them so...
    //
    a_readBuffer->dispose_buf();
    a_writeBuffer->dispose_buf();
    delete a_readBuffer;
    delete a_writeBuffer;

    DebugCode(a_allocated--;);
  }

  
  VirtualChannelInterface* TCPTransObj::m_closeConnection() {
    dssLog(DLL_DEBUG,"TRANSOBJ      (%p): closing down",this);
    VirtualChannelInterface *ans = a_channel;
    if(a_channel!=NULL) {
      a_mslEnv->a_ioFactory->registerRead(a_channel, false);  // Sometimes done twice!
      a_mslEnv->a_ioFactory->registerWrite(a_channel, false); // Sometimes done twice!
      a_mslEnv->a_ioFactory->setCallBackObj(a_channel, NULL); 
      a_channel = NULL;
    }
    return ans; 
  }

  void TCPTransObj::deliver() {
    Assert(a_channel !=NULL);
    a_mslEnv->a_ioFactory->registerWrite(a_channel, true);
  }

  void TCPTransObj::readyToReceive() {
    a_mslEnv->a_ioFactory->setCallBackObj(a_channel, this); 
    a_mslEnv->a_ioFactory->registerRead(a_channel, true);
  }


  void TCPTransObj::marshal(MsgCnt *msgC, int acknum) {
    bool cont=msgC->checkFlag(MSG_HAS_MARSHALCONT);
    a_writeBuffer->m_marshalBegin();

    dssLog(DLL_ALL,"TRANSOBJ      (%p): Marshal  ack:%d cont:%d",
	   this,
	   acknum,
	   msgC->checkFlag(MSG_HAS_MARSHALCONT)); 

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

    Assert(a_writeBuffer->availableSpace()>=1); // Room for trailer?
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

  void TCPTransObj::connectionLost(){
    a_comObj->connectionLost();
  }
  
  // Return 0 means invoke again, return 1 means done
  bool
  TCPTransObj::writeDataAvailable() {
    int len,ret;
    BYTE *pos;
    MsgCnt *msgC;
    int acknum;
    
    while (a_writeBuffer->m_getUnused()> (T_MIN_FOR_HEADER) &&
	   (msgC=a_comObj->getNextMsgCnt(acknum))!=NULL) 
      {
	marshal(msgC,acknum);
      }
    
    a_writeBuffer->m_transform();

    if (a_writeBuffer->m_getUsed()==0) {
      return true;
    }
    
    do
      {
      len = a_writeBuffer->m_getWriteParameters(pos);
      a_mslEnv->a_OSWriteCounter++;
      ret = a_mslEnv->a_ioFactory->writeData(a_channel, static_cast<void*>(pos),len);
      a_writeBuffer->m_hasWritten(ret);
      } 
    // If we are allowed to write what we want, there is possibly 
    // more space in the buffer..
    while(ret == len && a_writeBuffer->m_getUsed() >0);
    
    if (a_writeBuffer->m_getUsed() >0 || a_comObj->hasQueued())
      return false; 
    return true; 
  }

  unmarshalReturn TCPTransObj::unmarshal() {
    int acknum;
    int framesize;
    ControlFlag cf;
    ControlFlag cf2;
    MsgCnt *msgC;
    int msgnum;

  
    //ZACHARIAS:
    // - check what state we are in and require the buffer to be at
    //   least of size X bytes (should be done in readDataAvail below
    // - check that framesize is larger than MUSTREAD

    DebugCode(BYTE b=) a_readBuffer->m_getByte();              // Ctrl   1
    Assert(b==0xFF);
    acknum=a_readBuffer->m_getInt();      // Ack        4
    a_comObj->msgAcked(acknum);
    framesize=a_readBuffer->m_getInt();   // Framesize  4

    // ----------------------------------------- // Must read read
    if(a_readBuffer->m_canGet(framesize-MUSTREAD)) { // Can all be read?

      cf   = static_cast<ControlFlag>(a_readBuffer->m_getByte());          // CF

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
	cf2 = static_cast<ControlFlag>(a_readBuffer->m_getByte());
	a_readBuffer->m_commitReadOfData();
	if(cf2 == CF_CONT) {
	  Assert(msgC->checkFlag(MSG_HAS_UNMARSHALCONT));
	  a_comObj->msgPartlyReceived(msgC);
	}
	else {
	  Assert(cf2 == CF_FINAL);
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
  
  bool
  TCPTransObj::readDataAvailable() {  
    int ret,len;
    BYTE *pos;
    Assert(a_channel != NULL); 
    // Read
    
    len=a_readBuffer->m_getReadParameters(pos);  
    do {
      a_mslEnv->a_OSReadCounter++;
      ret = a_mslEnv->a_ioFactory->readData(a_channel, static_cast<void*>(pos) ,static_cast<unsigned int>(len));
      a_readBuffer->m_hasRead(ret);
    }
    // This might look funny, but I need to know if there is more 
    // place in the buffer, and if so read again. 
    while(ret == len && (len = a_readBuffer->m_getReadParameters(pos)) > 0);

    //printf("R-transforming\n");
    if(a_readBuffer->m_transform()){ // If the buffer is OK -> continue, otherwise close
      
      // Interpret (Allways interpret complete frames.)
      unmarshalReturn contin=U_MORE;
      
      a_readBuffer->m_unmarshalBegin();
      while(contin==U_MORE) {
	Assert(a_channel!=NULL);
	if(a_readBuffer->m_canGet(MUSTREAD))     // Includes previously read bytes
	  contin=unmarshal();
	else
	  break;
      }
      
      if(contin!=U_CLOSED)    // transobj could be passed on
	a_readBuffer->m_unmarshalEnd(); 
      
      return false;
    }
    else {
      a_comObj->m_closeErroneousConnection();
      return false;
    }
  }


  void
  TCPTransObj::m_EncryptReadTransport(BYTE* const key, const u32& keylen,
				      const u32& iv1,  const u32& iv2){
    //printf("TRANSPORT IS GOING RCRYPTO\n");
    DssCryptoReadByteBuffer* tmp = new DssCryptoReadByteBuffer(key, keylen, iv1, iv2, a_readBuffer);
    delete a_readBuffer;
    a_readBuffer = tmp;
  }

  void
  TCPTransObj::m_EncryptWriteTransport(BYTE* const key, const u32& keylen,
				       const u32& iv1,  const u32& iv2){    
    //printf("TRANSPORT IS GOING WCRYPTO\n");
    DssCryptoWriteByteBuffer* tmp = new DssCryptoWriteByteBuffer(key,keylen, iv1,iv2,a_writeBuffer);
    delete a_writeBuffer;
    a_writeBuffer = tmp;
  }



  
} //End namespace

