/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *    Zacharias El Banna (zeb@sics.se)
 * 
 *  Contributors:
 *    Anna Neiderud (annan@sics.se)
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

#include "dss_comService.hh"

#ifndef __GLUE_IOFACTORY_HH
#define __GLUE_IOFACTORY_HH

class GlueIoFactoryClass:public IoFactoryInterface{
public:
  GlueIoFactoryClass();

  // These two are not used in this impl. Connections are established
  // using the primitives provided by Mozart
  virtual VirtualChannelInterface* establishTCPchannel(int IP, int port, ChannelRequest *CR);
  virtual void terminateTCPchannel(VirtualChannelInterface *vc);
  
  // Communication
  virtual int  readData(VirtualChannelInterface *vc, void *buf, const unsigned int& len); 
  virtual int  writeData(VirtualChannelInterface *vc, void *buf, const unsigned int& len);
  virtual void registerRead(VirtualChannelInterface *vc, bool on); 
  virtual void registerWrite(VirtualChannelInterface *vc, bool on); 
  virtual bool setCallBackObj(VirtualChannelInterface *vc, VcCbkClassInterface *); 

  virtual int  setupTCPconnectPoint(int);
      
  // The method is used for the mozrt ComService that 
  // runns the whole connection establishment in oz, and
  // concequently establishes the connection at that level. 
  // I.e. extremly non-portable. 
  VirtualChannelInterface* channelFromFd(int fd);

};

// defined in engine_interface together with inits
extern GlueIoFactoryClass* glue_ioFactory;


#endif



