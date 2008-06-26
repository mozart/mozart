/*
 *  Authors:
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

#ifndef __TRANSOBJ_HH
#define __TRANSOBJ_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "mslBase.hh"
#include "dss_comService.hh"

namespace _msl_internal{ //Start namespace

  class ComObj;
  class MsgnLayerEnv;

  class TransObj {
  protected:
    MsgnLayerEnv*        a_mslEnv;
    ComObj*              a_comObj;
    const int            a_bufferSize;

  private:
    TransObj(const TransObj&):a_mslEnv(NULL), a_comObj(NULL),
                              a_bufferSize(0){}
    TransObj& operator=(const TransObj&){ return *this; }

  public:
    virtual DssChannel* m_closeConnection() = 0;
    // ComObj keeps track of its deliver calls so that as long as
    // it has called deliver, and TransObj has not pulled all of its
    // messages with getNextMsgContainer, it will not say deliver again.
    virtual void deliver()=0;
    virtual void readyToReceive()=0;

    void setOwner(ComObj* com);
    int  getBufferSize() const ;

    TransObj(const int& s, MsgnLayerEnv* env);
    virtual TransMedium getTransportMedium() = 0;
    virtual void m_EncryptReadTransport(BYTE* const key, const u32& keylen,
                                        const u32& iv1,  const u32& iv2) = 0;
    virtual void m_EncryptWriteTransport(BYTE* const key, const u32& keylen,
                                         const u32& iv1,  const u32& iv2) = 0;
  };



  class DssReadByteBuffer;
  class DssWriteByteBuffer;

  // BufferedTransObj extends TransObj with buffers and marshaling
  class BufferedTransObj: public TransObj {
  protected:
    // Minimal size required in buffer to even consider marshaling
    static const int T_MIN_FOR_HEADER = 100;

    enum unmarshalReturn {
      U_MORE,
      U_WAIT,
      U_CLOSED
    };

    enum ControlFlag {
      CF_FIRST,
      CF_CONT,
      CF_FINAL
    };

    // raph: We use two layers of buffering when encryption of data is
    // required (see below).  Both layers provide the same API, but
    // play different roles.  For instance, messages are marshaled in
    // a_marshalBuffer, then the data is moved to a_writeBuffer by
    // encoding, then sent to the transport layer.  This explicit
    // architecture simplifies the implementation, and provides better
    // performance.
    //
    //    Dss <- a_unmarshalBuffer <-  a_readBuffer <- transport
    //
    //    Dss ->  a_marshalBuffer  -> a_writeBuffer -> transport
    //
    // When no encryption takes place, both layers coincide.  In other
    // words, a_unmarshalBuffer == a_readBuffer and a_marshalBuffer ==
    // a_writeBuffer.

    // the buffers used to read from the transport layer, and unmarshal.
    DssReadByteBuffer*  a_readBuffer;
    DssReadByteBuffer*  a_unmarshalBuffer;

    // the buffers used to write to the transport layer, and marshal.
    DssWriteByteBuffer* a_writeBuffer;
    DssWriteByteBuffer* a_marshalBuffer;

    // marshal/unmarshal Dss messages
    void marshal(MsgCnt *msgC, int acknum);
    unmarshalReturn unmarshal();

  private:
    // should not be used
    BufferedTransObj(const BufferedTransObj&)
      : TransObj(0,NULL), a_readBuffer(NULL), a_writeBuffer(NULL),
        a_unmarshalBuffer(NULL), a_marshalBuffer(NULL) {}
    BufferedTransObj& operator=(const BufferedTransObj&){ return *this; }

  public:
    BufferedTransObj(const int& sz, MsgnLayerEnv* env);
    virtual ~BufferedTransObj();

    virtual void m_EncryptReadTransport(BYTE* const key, const u32& keylen,
                                        const u32& iv1,  const u32& iv2);
    virtual void m_EncryptWriteTransport(BYTE* const key, const u32& keylen,
                                         const u32& iv1,  const u32& iv2);
  };

} //End namespace
#endif
