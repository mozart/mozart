/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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
#include "mslBase.hh"
#include "msl_buffer.hh"

namespace _msl_internal{

  const size_t BUFFER_SIZE = 1<<16;     // 64kb
  
#ifdef DEBUG_CHECK
  int TCPTransObj::a_allocated = 0;
#endif

  /******************** constructor/destructor ********************/
  
  TCPTransObj::TCPTransObj(MsgnLayerEnv* env) :
    BufferedTransObj(BUFFER_SIZE, env), a_channel(NULL)
  {
    // Count ALL TCP trans objects (for all Environments)
    DebugCode(a_allocated++;);
  }
  
  TCPTransObj::~TCPTransObj() {
    DebugCode(a_allocated--;);
  }


  
  /**************************** handling connection ********************/

  VirtualChannelInterface* TCPTransObj::m_closeConnection() {
    VirtualChannelInterface *ans = a_channel;

    dssLog(DLL_DEBUG,"TRANSOBJ      (%p): closing down",this);

    if (a_channel != NULL) {
      a_channel->registerRead(false);  // Sometimes done twice!
      a_channel->registerWrite(false); // Sometimes done twice!
      a_channel->setCallback(NULL); 
      a_channel = NULL;
    }
    return ans; 
  }

  void TCPTransObj::deliver() {
    Assert(a_channel != NULL);
    a_channel->registerWrite(true);
  }

  void TCPTransObj::readyToReceive() {
    a_channel->setCallback(this); 
    a_channel->registerRead(true);
  }

  void TCPTransObj::connectionLost() {
    a_comObj->connectionLost();
  }
  
  // return true if done, and false to be invoked again
  bool
  TCPTransObj::writeDataAvailable() {
    int len, ret, acknum;
    BYTE* pos;
    MsgCnt* msgC;

    // marshal as many messages as possible
    while (a_marshalBuffer->getFree() >= T_MIN_FOR_HEADER &&
	   (msgC = a_comObj->getNextMsgCnt(acknum))) {
      marshal(msgC, acknum);
    }

    // encode data to a_writeBuffer
    a_writeBuffer->encode();

    // check if something can be sent
    if (a_writeBuffer->getUsed() == 0) return true;

    // send to transport layer (loop twice if necessary)
    do {
      len = a_writeBuffer->getReadBlock(pos);
      a_mslEnv->a_OSWriteCounter++;
      ret = a_channel->write(pos, len);
      a_writeBuffer->m_commitRead(ret);
    } while (ret == len && a_writeBuffer->getUsed() > 0);

    // return true if everything was sent
    return (a_writeBuffer->getUsed() == 0 && !a_comObj->hasQueued());
  }

  // return false if done, and true if more has to be read
  bool
  TCPTransObj::readDataAvailable() {  
    int ret,len;
    BYTE *pos;

    Assert(a_channel != NULL); 
    Assert(a_readBuffer->getFree() > 0);

    // read from transport layer (loop twice if necessary)
    do {
      len = a_readBuffer->getWriteBlock(pos);
      a_mslEnv->a_OSReadCounter++;
      ret = a_channel->read(pos, len);
      a_readBuffer->m_commitWrite(ret);
    } while (ret == len && a_readBuffer->getFree() > 0);

    // decode data to a_unmarshalBuffer
    if (a_readBuffer->decode()) {
      // unmarshal as many messages as possible
      while (unmarshal() == U_MORE) {}

    } else {
      // decoding has failed, close this connection
      a_comObj->m_closeErroneousConnection();
    }

    // at this level, we never ask to be called again
    return false;
  }

} //End namespace
