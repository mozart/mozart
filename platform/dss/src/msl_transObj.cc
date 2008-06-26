/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
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
#pragma implementation "msl_transObj.hh"
#endif

#include "msl_transObj.hh"
#include "msl_comObj.hh"
#include "msl_msgContainer.hh"
#include "msl_buffer.hh"

namespace _msl_internal{

  /************************* TransObj *************************/

  TransObj::TransObj(const int& s, MsgnLayerEnv* env):
    a_mslEnv(env), a_comObj(NULL), a_bufferSize(s)
  {}

  void TransObj::setOwner(ComObj* com){  a_comObj = com; }
  int  TransObj::getBufferSize() const { return a_bufferSize; }



  /************************* BufferedTransObj *************************/

  BufferedTransObj::BufferedTransObj(const int& sz, MsgnLayerEnv* env) :
    TransObj(sz, env),
    a_readBuffer(new DssReadByteBuffer(sz)), a_unmarshalBuffer(NULL),
    a_writeBuffer(new DssWriteByteBuffer(sz)), a_marshalBuffer(NULL)
  {
    // no encryption by default
    a_unmarshalBuffer = a_readBuffer;
    a_marshalBuffer = a_writeBuffer;
  }

  BufferedTransObj::~BufferedTransObj() {
    delete a_readBuffer;
    delete a_writeBuffer;
    delete a_unmarshalBuffer;
    delete a_marshalBuffer;
  }



  /******************** marshal/unmarshal ********************/

  // useful sizes inside marshaled frames
  const int HEADER1 = 1 + SIZE_INT + SIZE_INT; // byte + acknum + framesize
  const int TRAILER = 1;                       // control flag

  void BufferedTransObj::marshal(MsgCnt *msgC, int acknum) {
    int framesize;
    BYTE* pos;

    dssLog(DLL_ALL,"TRANSOBJ      (%p): Marshal  ack:%d cont:%d",
           this, acknum, msgC->checkFlag(MSG_HAS_MARSHALCONT));

    framesize = a_marshalBuffer->getFree(); // max framesize
    Assert(framesize >= T_MIN_FOR_HEADER);

    a_marshalBuffer->m_putByte(0xFF);       // control byte
    a_marshalBuffer->m_putInt(acknum);      // ack number

    pos = a_marshalBuffer->getWritePos();   // where to put framesize
    a_marshalBuffer->m_putInt(0);           // placeholder for framesize

    // write control flag (CF_FIRST or CF_CONT)
    if (msgC->checkFlag(MSG_HAS_MARSHALCONT)) {
      Assert(msgC->getMsgNum() != NO_MSG_NUM);
      a_marshalBuffer->m_putByte(CF_CONT);
      a_marshalBuffer->m_putInt(msgC->getMsgNum());
    } else {
      Assert(msgC->getMsgNum() == NO_MSG_NUM);
      a_marshalBuffer->m_putByte(CF_FIRST);
    }

    // serialize message, keeping one byte for trailer
    a_marshalBuffer->reserveSpace(TRAILER);
    msgC->m_serialize(a_marshalBuffer, a_comObj->getSite(), a_mslEnv);
    a_marshalBuffer->resetReserve();

    // write trailer (CF_CONT or CF_FINAL)
    Assert(a_marshalBuffer->getFree() >= TRAILER);
    if (msgC->checkFlag(MSG_HAS_MARSHALCONT)) {
      a_mslEnv->a_ContCounter++;
      dssLog(DLL_DEBUG,"TRANSOBJ      (%p): Marshal continuation %d\n",
             this, a_mslEnv->a_ContCounter);
      a_comObj->msgPartlySent(msgC);
      a_marshalBuffer->m_putByte(CF_CONT);
    } else {
      a_comObj->msgSent(msgC);
      a_marshalBuffer->m_putByte(CF_FINAL);
    }

    // write framesize in placeholder now
    framesize -= a_marshalBuffer->getFree(); // real framesize
    a_marshalBuffer->m_putInt(pos, framesize);
  }

  BufferedTransObj::unmarshalReturn
  BufferedTransObj::unmarshal() {
    int framesize;
    ControlFlag cf;
    MsgCnt *msgC;

    // check whether we can read the header, wait for more if not.
    if (a_unmarshalBuffer->getUsed() < HEADER1) return U_WAIT;

    // read header
    BYTE b = a_unmarshalBuffer->m_getByte();     // control byte
    Assert(b == 0xFF);
    int acknum = a_unmarshalBuffer->m_getInt();      // ack number
    a_comObj->msgAcked(acknum);
    framesize = a_unmarshalBuffer->m_getInt();   // framesize

    // check whether we have a complete frame
    if (a_unmarshalBuffer->getUsed() < framesize - HEADER1) {
      a_unmarshalBuffer->m_commitRead(-HEADER1); // unread the header!
      return U_WAIT;                             // wait for more data
    }

    // it is easier to set the frame size now
    a_unmarshalBuffer->setFrameSize(framesize - HEADER1 - TRAILER);

    // read control flag (CF_FIRST or CF_CONT)
    cf = static_cast<ControlFlag>(a_unmarshalBuffer->m_getByte());
    if (cf == CF_FIRST) {
      msgC = a_comObj->getMsgCnt();
      Assert(!msgC->checkFlag(MSG_HAS_UNMARSHALCONT));
    } else {
      Assert(cf == CF_CONT);
      int msgnum = a_unmarshalBuffer->m_getInt(); // read message number
      msgC = a_comObj->getMsgCnt(msgnum);
    }
    Assert(msgC != NULL);

    // deserialize message
    if (msgC->deserialize(a_unmarshalBuffer, a_comObj->getSite(), a_mslEnv)) {
      // frame contents successfully deserialized, read trailer
      a_unmarshalBuffer->resetFrame();
      Assert(a_unmarshalBuffer->getUsed() >= TRAILER);
      cf = static_cast<ControlFlag>(a_unmarshalBuffer->m_getByte());
      if (cf == CF_CONT) {
        Assert(msgC->checkFlag(MSG_HAS_UNMARSHALCONT));
        a_comObj->msgPartlyReceived(msgC);
      } else {
        Assert(cf == CF_FINAL);
        Assert(!msgC->checkFlag(MSG_HAS_UNMARSHALCONT));
        if (!a_comObj->msgReceived(msgC)) return U_CLOSED;
      }

      // please read more
      return U_MORE;

    } else {
      // frame contents somehow corrupted.  Since messages have to
      // be delivered in order we cannot just discard this frame.
      // Something must be seriously wrong in the transport layer.
      // Break the connection by telling the comObj it was lost.
      // The comObj can then decide on further actions.
      a_comObj->connectionLost();
      return U_CLOSED;
    }
  }



  /******************** set crypto buffers ********************/

  void
  BufferedTransObj::m_EncryptReadTransport(BYTE* const key, const u32& keylen,
                                           const u32& iv1,  const u32& iv2) {
    Assert(a_readBuffer == a_unmarshalBuffer);
    a_readBuffer =
      new DssCryptoReadByteBuffer(key, keylen, iv1, iv2, a_unmarshalBuffer);
  }

  void
  BufferedTransObj::m_EncryptWriteTransport(BYTE* const key, const u32& keylen,
                                            const u32& iv1,  const u32& iv2) {
    Assert(a_writeBuffer == a_marshalBuffer);
    a_writeBuffer =
      new DssCryptoWriteByteBuffer(key, keylen, iv1, iv2, a_marshalBuffer);
  }

} //End namespace
