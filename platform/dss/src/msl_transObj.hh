/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
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
    virtual VirtualChannelInterface* m_closeConnection() = 0; 
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

} //End namespace
#endif
