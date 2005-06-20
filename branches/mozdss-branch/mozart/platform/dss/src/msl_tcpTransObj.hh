/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
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

#ifndef __TCPTRANSOBJ_HH
#define __TCPTRANSOBJ_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "msl_transObj.hh"
#include "dss_comService.hh"
#include "mslBase.hh"

namespace _msl_internal{ //Start namespace 
  
  enum unmarshalReturn {
    U_MORE,
    U_WAIT,
    U_CLOSED
  };  

 
  class DssReadByteBuffer;
  class DssWriteByteBuffer;
  
  class TCPTransObj: public TransObj, public ::VcCbkClassInterface {
  private:

    static const int T_MIN_FOR_HEADER = 100;// Minimal size available to even consider marshaling

    enum ControlFlag{
      CF_FIRST,
      CF_CONT,
      CF_FINAL
    };

    int                  a_minSend;
    DssReadByteBuffer*   a_readBuffer;
    DssWriteByteBuffer*  a_writeBuffer;
    
    ::VirtualChannelInterface*    a_channel;
    
    
    void marshal(MsgCnt *msgC, int acknum);
    unmarshalReturn unmarshal();

    TCPTransObj(const TCPTransObj&):TransObj(0,NULL), a_minSend(0), a_readBuffer(NULL), a_writeBuffer(NULL), a_channel(NULL)
{}
    TCPTransObj& operator=(const TCPTransObj&){ return *this; }
    
  public:
#ifdef DEBUG_CHECK
    static int           a_allocated;
#endif
    
    TCPTransObj(MsgnLayerEnv* env);
    virtual ~TCPTransObj();
    
    virtual VirtualChannelInterface* m_closeConnection();
    virtual void deliver();
    virtual void readyToReceive();
    
    virtual void connectionLost();    
    virtual bool readDataAvailable(); 
    virtual bool writeDataAvailable();
  
    virtual TransMedium getTransportMedium() {return TM_TCP;}
    virtual void m_EncryptReadTransport(BYTE* const key, const u32& keylen,
					const u32& iv1,  const u32& iv2);
    virtual void m_EncryptWriteTransport(BYTE* const key, const u32& keylen,
					 const u32& iv1,  const u32& iv2);


    inline void setChannel(VirtualChannelInterface* ch){ a_channel = ch; }
    
  };

} //End namespace
#endif

