/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Erik Klintskog (erik@sics.se)
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

#include "value.hh" // For that darn Bool
#include "glue_ioFactory.hh"  
#include "os.hh"
#ifndef WINDOWS
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#endif
/************* I/O ************************/
Bool glue_io_read(int fd, void *arg);
Bool glue_io_write(int fd, void *arg);

class VirtualTcpChannel:public VirtualChannelInterface{
private:
  int a_fd; 
  bool a_connectionLost;  
public:
  VcCbkClassInterface *a_clbck;

private:
  ErrorClass classifyError() {
    switch(ossockerrno()) {
    case EINTR:
      printf("eintr\n"); 
      return EC_GO_AHEAD;
    
    case EHOSTUNREACH:
    case EAGAIN:
    case ETIMEDOUT:
      return EC_CONTINUE_LATER;
      
    case EINPROGRESS: 
    case EPIPE:
    case ECONNRESET:
    case EBADF:
      return EC_LOST;  // This connection is broken! The site might still be up.
    default: 
      return EC_LOST;
    }
  }
public:
  VirtualTcpChannel(int fd):a_fd(fd), a_clbck(NULL), a_connectionLost(false){;}
  
  ~VirtualTcpChannel(){
    ;
  }
  
  bool m_isConnectionLost()
  { 
    return a_connectionLost;
  }

  int m_readData( void *buf, const unsigned int& len){
    int ret = osread(a_fd,buf,len);
    if (ret<=0)
      {
	if (classifyError() == EC_LOST) {
    printf("the connection was lost reading data\n"); //bmc
	  a_connectionLost = true;
    }
	return 0; 
      }
    return ret; 
  }
  
  int m_writeData(void *buf, const unsigned int& len){
    int ret = oswrite(a_fd,buf,len);
    if (ret<0)
      {
	if (classifyError() == EC_LOST) {
    printf("connection lost when writting data\n"); //bmc
	  a_connectionLost = true;
   }
	return 0; 
      }
    return ret; 
  }
  
  void m_closeChannel(){
    osclose(a_fd);
    delete this; 
  }

  void m_registerRead(bool on){
    if(on)
      OZ_registerReadHandler(a_fd,glue_io_read ,this); 
    else
      OZ_unregisterRead(a_fd);
  }
  
  void m_registerWrite(bool on){
   if(on) 
     OZ_registerWriteHandler(a_fd,glue_io_write,this);
   else
     OZ_unregisterWrite(a_fd);
  }
  
  bool m_readInvoke(){
    // Check to see that we have a callback object, 
    // a null ptr indicates that no interest existst for the 
    // channel
    if(a_clbck == NULL || a_connectionLost)
      return false; 
    
    bool readMore = a_clbck->readDataAvailable();
    
    // After the call we check to see if the channel was lost 
    // while reading it. Note that we _dont_ tell the callback 
    // object while reading about the fact that teh cahnnel was 
    // lost. 
    if (a_connectionLost){
      a_clbck->connectionLost();
      return false; 
    }
    return readMore; 
  }
  
  bool m_writeInvoke(){
    // Check to see that we have a callback object, 
    // a null ptr indicates that no interest existst for the 
    // channel
    if(a_clbck == NULL || a_connectionLost)
      return false; 
    
    bool writeMore = a_clbck->writeDataAvailable();
    
    // After the call we check to see if the channel was lost 
    // while reading it. Note that we _dont_ tell the callback 
    // object while reading about the fact that teh cahnnel was 
    // lost. 
    if (a_connectionLost){
      a_clbck->connectionLost();
      return false; 
    }
    return writeMore; 
  }
  
  void m_setCallBackObj(VcCbkClassInterface* c) {
    a_clbck = c; 
  }
};


Bool glue_io_read(int fd, void *arg){
  return static_cast<VirtualTcpChannel*>(arg)->m_readInvoke();
}

Bool glue_io_write(int fd, void *arg){
  return static_cast<VirtualTcpChannel*>(arg)->m_writeInvoke();
}



GlueIoFactoryClass::GlueIoFactoryClass(){
  ;
}


VirtualChannelInterface*
GlueIoFactoryClass::channelFromFd(int fd){
  return new VirtualTcpChannel(fd);
}


// These two are not used in this impl. Connections are established
// using the primitives provided by Mozart

VirtualChannelInterface*
GlueIoFactoryClass::establishTCPchannel(int IP, int port, ChannelRequest *CR)
{
  printf("Here should code exist that requests a connection establishment\n");
  printf("and returns a channel. Not yet however\n");
  return NULL; 
}

void 
GlueIoFactoryClass::terminateTCPchannel(VirtualChannelInterface *vc)
{
  printf("Here should code exist that terminates a connection termination.\n");
  printf("Not yet however\n");
}
  
  // Communication
int  
GlueIoFactoryClass::readData(VirtualChannelInterface *vc, void *buf, 
			     const unsigned int& len)
{ 
  VirtualTcpChannel *vtc = static_cast<VirtualTcpChannel*>(vc);
  return vtc->m_readData(buf, len);  
}
int  
GlueIoFactoryClass::writeData(VirtualChannelInterface *vc, void *buf, 
			      const unsigned int& len)
{
  VirtualTcpChannel *vtc = static_cast<VirtualTcpChannel*>(vc);
  return vtc->m_writeData(buf, len); 
}
void 
GlueIoFactoryClass::registerRead(VirtualChannelInterface *vc, bool on){
  VirtualTcpChannel *vtc = static_cast<VirtualTcpChannel*>(vc);
  vtc->m_registerRead(on);
}
void 
GlueIoFactoryClass::registerWrite(VirtualChannelInterface *vc, bool on){
  VirtualTcpChannel *vtc = static_cast<VirtualTcpChannel*>(vc);
  vtc->m_registerWrite(on);
}
bool 
GlueIoFactoryClass::setCallBackObj(VirtualChannelInterface *vc, VcCbkClassInterface *vcc){
  VirtualTcpChannel *vtc = static_cast<VirtualTcpChannel*>(vc);
  if(vtc->m_isConnectionLost()) 
    return false;
  vtc->m_setCallBackObj(vcc);
  

} 

int   
GlueIoFactoryClass::setupTCPconnectPoint(int){
  printf("GlueIoFactoryClass::setupTCPconnectPoint -- not impl\n"); 
  return 0; 
}
