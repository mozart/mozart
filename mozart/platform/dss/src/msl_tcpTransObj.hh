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

#ifndef __TCPTRANSOBJ_HH
#define __TCPTRANSOBJ_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "msl_transObj.hh"
#include "dss_comService.hh"
#include "mslBase.hh"

namespace _msl_internal{ //Start namespace

  class TCPTransObj: public BufferedTransObj, public ::DssChannelCallback {

  private:
    // raph: We extend the BufferedTransObj with a virtual channel.
    ::DssChannel* a_channel;

    // should not be used
    TCPTransObj(const TCPTransObj&)
      : BufferedTransObj(0,NULL), a_channel(NULL) {}
    TCPTransObj& operator=(const TCPTransObj&){ return *this; }
    
  public:
#ifdef DEBUG_CHECK
    static int           a_allocated;
#endif

    TCPTransObj(MsgnLayerEnv* env);
    virtual ~TCPTransObj();
    
    virtual DssChannel* m_closeConnection();
    virtual void deliver();
    virtual void readyToReceive();
    
    virtual void connectionLost();    
    virtual bool readDataAvailable(); 
    virtual bool writeDataAvailable();
  
    virtual TransMedium getTransportMedium() {return TM_TCP;}
    // m_EncryptReadTransport()  inherited
    // m_EncryptWriteTransport() inherited

    inline void setChannel(DssChannel* ch){ a_channel = ch; }
  };

} //End namespace
#endif

